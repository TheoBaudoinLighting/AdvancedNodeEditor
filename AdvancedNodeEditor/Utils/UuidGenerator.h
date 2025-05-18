#pragma once

#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <mutex>
#include <chrono>
#include <atomic>

namespace NodeEditorCore {
    class Uuid {
    public:
        using value_type = std::array<uint8_t, 16>;

        Uuid() noexcept : m_data{} {
        }

        explicit Uuid(const value_type &data) noexcept : m_data(data) {
        }

        explicit Uuid(const std::string &str) {
            if (str.size() != 36 || str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-') {
                throw std::invalid_argument("Invalid UUID format");
            }

            std::string hexStr = str.substr(0, 8) + str.substr(9, 4) + str.substr(14, 4) +
                                 str.substr(19, 4) + str.substr(24);

            for (size_t i = 0; i < 16; ++i) {
                std::string byteStr = hexStr.substr(i * 2, 2);
                try {
                    m_data[i] = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
                } catch (const std::exception &) {
                    throw std::invalid_argument("UUID contains non-hexadecimal characters");
                }
            }
        }

        uint8_t getVersion() const noexcept {
            return (m_data[6] >> 4) & 0x0F;
        }

        void setVersion(uint8_t version) noexcept {
            m_data[6] = (m_data[6] & 0x0F) | ((version & 0x0F) << 4);
        }

        void setVariant() noexcept {
            m_data[8] = (m_data[8] & 0x3F) | 0x80;
        }

        std::string toString() const {
            std::stringstream ss;
            ss << std::hex << std::setfill('0');

            for (size_t i = 0; i < 16; ++i) {
                if (i == 4 || i == 6 || i == 8 || i == 10) {
                    ss << '-';
                }
                ss << std::setw(2) << static_cast<int>(m_data[i]);
            }

            return ss.str();
        }

        const value_type &getData() const noexcept {
            return m_data;
        }

        bool isNil() const noexcept {
            for (auto byte: m_data) {
                if (byte != 0) return false;
            }
            return true;
        }

        bool operator==(const Uuid &other) const noexcept {
            return m_data == other.m_data;
        }

        bool operator!=(const Uuid &other) const noexcept {
            return !(*this == other);
        }

        bool operator<(const Uuid &other) const noexcept {
            return m_data < other.m_data;
        }

        explicit operator std::string() const {
            return toString();
        }

        explicit operator bool() const noexcept {
            return !isNil();
        }

        int32_t toId() const noexcept {
            std::hash<std::string> hasher;
            size_t hashValue = hasher(toString());
            return static_cast<int32_t>(hashValue & 0x7FFFFFFF);
        }

    private:
        value_type m_data;
    };

    class UuidGenerator {
    public:
        static UuidGenerator &getInstance() {
            static UuidGenerator instance;
            return instance;
        }

        Uuid generateV4() {
            std::lock_guard<std::mutex> lock(m_mutex);

            Uuid::value_type data;
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] = static_cast<uint8_t>(m_distribution(m_rng));
            }

            Uuid uuid(data);
            uuid.setVersion(4);
            uuid.setVariant();

            if (m_lastGenerated == uuid) {
                return generateV4();
            }

            m_lastGenerated = uuid;
            m_count++;

            return uuid;
        }

        Uuid generateV1() {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto now = std::chrono::high_resolution_clock::now();
            auto epochNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count();

            constexpr uint64_t NANOS_PER_TICK = 100;
            constexpr uint64_t UNIX_EPOCH_TO_UUID_EPOCH = 0x01B21DD213814000;
            uint64_t timestamp = (epochNanos / NANOS_PER_TICK) + UNIX_EPOCH_TO_UUID_EPOCH;

            Uuid::value_type data;

            data[0] = static_cast<uint8_t>((timestamp >> 24) & 0xFF);
            data[1] = static_cast<uint8_t>((timestamp >> 16) & 0xFF);
            data[2] = static_cast<uint8_t>((timestamp >> 8) & 0xFF);
            data[3] = static_cast<uint8_t>(timestamp & 0xFF);

            data[4] = static_cast<uint8_t>((timestamp >> 40) & 0xFF);
            data[5] = static_cast<uint8_t>((timestamp >> 32) & 0xFF);

            data[6] = static_cast<uint8_t>((timestamp >> 56) & 0x0F);
            data[7] = static_cast<uint8_t>((timestamp >> 48) & 0xFF);

            uint16_t clockSeq = m_sequenceCounter++;
            data[8] = static_cast<uint8_t>((clockSeq >> 8) & 0xFF);
            data[9] = static_cast<uint8_t>(clockSeq & 0xFF);

            for (int i = 0; i < 6; ++i) {
                data[10 + i] = static_cast<uint8_t>(m_distribution(m_rng));
            }

            Uuid uuid(data);
            uuid.setVersion(1);
            uuid.setVariant();

            return uuid;
        }

        uint64_t getGenerationCount() const {
            return m_count;
        }

        void reseed(uint64_t seed = 0) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (seed == 0) {
                std::random_device rd;
                seed = static_cast<uint64_t>(rd()) << 32 | rd();
            }
            m_rng.seed(seed);
        }

    private:
        UuidGenerator() {
            std::random_device rd;
            m_rng.seed(static_cast<uint64_t>(rd()) << 32 | rd());
        }

        UuidGenerator(const UuidGenerator &) = delete;

        UuidGenerator &operator=(const UuidGenerator &) = delete;

        std::mt19937_64 m_rng;
        std::uniform_int_distribution<unsigned int> m_distribution{0, 255};
        std::mutex m_mutex;
        Uuid m_lastGenerated;
        std::atomic<uint64_t> m_count{0};
        std::atomic<uint16_t> m_sequenceCounter{0};
    };
}

namespace std {
    template<>
    struct hash<NodeEditorCore::Uuid> {
        std::size_t operator()(const NodeEditorCore::Uuid &uuid) const {
            const auto &data = uuid.getData();
            std::size_t h = 0;

            for (auto byte: data) {
                h = h * 31 + byte;
            }

            return h;
        }
    };
}
