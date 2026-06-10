#pragma once

#ifndef NODE_EDITOR_LOG_TRACE
#define NODE_EDITOR_LOG_TRACE(...) do {} while (false)
#endif

#ifndef NODE_EDITOR_LOG_DEBUG
#define NODE_EDITOR_LOG_DEBUG(...) do {} while (false)
#endif

#ifndef NODE_EDITOR_LOG_INFO
#define NODE_EDITOR_LOG_INFO(...) do {} while (false)
#endif

#ifndef NODE_EDITOR_LOG_WARN
#define NODE_EDITOR_LOG_WARN(...) do {} while (false)
#endif

#ifndef NODE_EDITOR_LOG_ERROR
#define NODE_EDITOR_LOG_ERROR(...) do {} while (false)
#endif

#ifndef LOG_TRACE
#define LOG_TRACE(...) NODE_EDITOR_LOG_TRACE(__VA_ARGS__)
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG(...) NODE_EDITOR_LOG_DEBUG(__VA_ARGS__)
#endif

#ifndef LOG_INFO
#define LOG_INFO(...) NODE_EDITOR_LOG_INFO(__VA_ARGS__)
#endif

#ifndef LOG_WARN
#define LOG_WARN(...) NODE_EDITOR_LOG_WARN(__VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(...) NODE_EDITOR_LOG_ERROR(__VA_ARGS__)
#endif
