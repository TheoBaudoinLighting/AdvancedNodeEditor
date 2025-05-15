function Sanitize-DotName {
    param (
        [string]$name
    )
    
    # Replace problematic characters with underscores
    # Need to be especially careful with characters that have special meaning in DOT syntax
    $sanitized = $name -replace '[<>:"|{}\\;,]', '_'  # These characters cause problems in DOT
    $sanitized = $sanitized -replace '[-]', '_'        # Replace hyphens with underscores
    $sanitized = $sanitized -replace '[^a-zA-Z0-9_./]', '_'  # Replace any other non-alphanumeric chars
    
    # If the name starts with a number or period, prefix with underscore
    if ($sanitized -match '^[0-9.]') {
        $sanitized = "_" + $sanitized
    }
    
    # Ensure there are no periods (except for file extensions)
    # This keeps extensions but replaces other periods
    if ($sanitized -match '\.[a-zA-Z0-9]+$') {
        $extension = $sanitized -replace '.*(\.[a-zA-Z0-9]+)$', '$1'
        $baseName = $sanitized -replace '(.*)\.[a-zA-Z0-9]+$', '$1'
        $baseName = $baseName -replace '\.', '_'
        $sanitized = $baseName + $extension
    } else {
        $sanitized = $sanitized -replace '\.', '_'
    }
    return $sanitized
}


function Get-CppFiles {
    param (
        [string]$srcDir,
        [string]$includeDir
    )
    
    $cppFiles = @()
    if (Test-Path $srcDir) {
        $cppFiles += Get-ChildItem -Path $srcDir -Recurse -Include "*.cpp", "*.h", "*.hpp"
    }
    if (Test-Path $includeDir) {
        $cppFiles += Get-ChildItem -Path $includeDir -Recurse -Include "*.h", "*.hpp"
    }
    
    return $cppFiles
}

function Generate-FileInclusionGraph {
    param (
        [string]$srcDir,
        [string]$includeDir
    )
    
    Write-Host "Generating file inclusion graph..."
    $cppFiles = Get-CppFiles -srcDir $srcDir -includeDir $includeDir
    
    $dotContent = "digraph FileInclusions {`n"
    $dotContent += "  rankdir=LR;`n"
    $dotContent += "  node [shape=box, style=filled, fillcolor=lightblue];`n"
    
    $fileNodes = @{}
    foreach ($file in $cppFiles) {
        $relativePath = $file.FullName.Replace("$((Get-Location).Path)\", "")
        $relativePathNode = Sanitize-DotName($relativePath.Replace("\", "/"))
        $fileNodes[$file.Name] = $relativePathNode
        $dotContent += "  `"$relativePathNode`" [label=`"$($file.Name)`"];`n"
    }
    
    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName
        $relativePathSource = Sanitize-DotName($file.FullName.Replace("$((Get-Location).Path)\", "").Replace("\", "/"))
        
        foreach ($line in $content) {
            if ($line -match '#include\s+[<"]([^>"]+)[>"]') {
                $includedFile = $matches[1]
                $includedFileName = [System.IO.Path]::GetFileName($includedFile)
                
                if ($fileNodes.ContainsKey($includedFileName)) {
                    # Inverted direction: included file -> including file
                    $dotContent += "  `"$($fileNodes[$includedFileName])`" -> `"$relativePathSource`";`n"
                }
            }
        }
    }
    
    $dotContent += "}`n"
    
    $outputFile = "file_inclusions.dot"
    $dotContent | Out-File -FilePath $outputFile -Encoding utf8
    
    Write-Host "File inclusion graph generated in $outputFile"
    Write-Host "To generate an image, run: dot -Tpng $outputFile -o file_inclusions.png"
}

function Generate-ClassNamespaceGraph {
    param (
        [string]$srcDir,
        [string]$includeDir
    )
    
    Write-Host "Generating class and namespace graph..."
    $cppFiles = Get-CppFiles -srcDir $srcDir -includeDir $includeDir
    
    $namespaces = @{}
    $classes = @{}
    $relations = @()
    
    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName -Raw
        $relativePath = $file.FullName.Replace("$((Get-Location).Path)\", "").Replace("\", "/")
        
        $namespaceMatches = [regex]::Matches($content, 'namespace\s+(\w+)')
        foreach ($match in $namespaceMatches) {
            $namespace = $match.Groups[1].Value
            if (-not $namespaces.ContainsKey($namespace)) {
                $namespaces[$namespace] = @()
            }
            if (-not $namespaces[$namespace].Contains($relativePath)) {
                $namespaces[$namespace] += $relativePath
            }
        }
        
        $classMatches = [regex]::Matches($content, 'class\s+(\w+)(?:\s*:\s*(?:public|protected|private)\s+(\w+))?')
        foreach ($match in $classMatches) {
            $class = $match.Groups[1].Value
            if (-not $classes.ContainsKey($class)) {
                $classes[$class] = @{
                    File = $relativePath
                    Namespace = ""
                }
                
                $nsStart = 0
                $namespaceMatches | ForEach-Object {
                    if ($_.Index -lt $match.Index -and $_.Index -gt $nsStart) {
                        $nsStart = $_.Index
                        $classes[$class].Namespace = $_.Groups[1].Value
                    }
                }
            }
            
            if ($match.Groups[2].Success) {
                $baseClass = $match.Groups[2].Value
                $relations += @{
                    Source = $baseClass  # Inverted: base -> derived
                    Target = $class
                    Type = "inherits"
                }
            }
        }
        
        foreach ($class in $classes.Keys) {
            if ($classes[$class].File -eq $relativePath) {
                $classPattern = "class\s+$class\b"
                $classMatch = [regex]::Match($content, $classPattern)
                if ($classMatch.Success) {
                    $classEnd = $content.IndexOf("};", $classMatch.Index)
                    if ($classEnd -gt 0) {
                        $classBody = $content.Substring($classMatch.Index, $classEnd - $classMatch.Index)
                        foreach ($otherClass in $classes.Keys) {
                            if ($class -ne $otherClass -and $classBody -match "\b$otherClass\b") {
                                $relations += @{
                                    Source = $otherClass  # Inverted: used -> user
                                    Target = $class
                                    Type = "uses"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    $dotContent = "digraph ClassNamespaces {`n"
    $dotContent += "  rankdir=LR;`n"
    $dotContent += "  node [shape=box, style=filled, fillcolor=lightblue];`n"
    
    $nsCount = 0
    foreach ($ns in $namespaces.Keys) {
        $nsCount++
        $dotContent += "  subgraph `"cluster_$nsCount`" {`n"
        $dotContent += "    label=`"namespace $ns`";`n"
        $dotContent += "    style=filled;`n"
        $dotContent += "    color=lightgrey;`n"
        
        $classesInNs = $classes.GetEnumerator() | Where-Object { $_.Value.Namespace -eq $ns }
        foreach ($classEntry in $classesInNs) {
            $safeKey = Sanitize-DotName($classEntry.Key)
            $dotContent += "    `"$safeKey`" [label=`"$($classEntry.Key)`"];`n"
        }
        
        $dotContent += "  }`n"
    }
    
    $orphanClasses = $classes.GetEnumerator() | Where-Object { $_.Value.Namespace -eq "" }
    foreach ($classEntry in $orphanClasses) {
        $safeKey = Sanitize-DotName($classEntry.Key)
        $dotContent += "  `"$safeKey`" [label=`"$($classEntry.Key)`"];`n"
    }
    
    foreach ($rel in $relations) {
        $style = if ($rel.Type -eq "inherits") { "solid" } else { "dashed" }
        $safeSource = Sanitize-DotName($rel.Source)
        $safeTarget = Sanitize-DotName($rel.Target)
        $dotContent += "  `"$safeSource`" -> `"$safeTarget`" [style=$style];`n"
    }
    
    $dotContent += "}`n"
    
    $outputFile = "class_namespaces.dot"
    $dotContent | Out-File -FilePath $outputFile -Encoding utf8
    
    Write-Host "Class and namespace graph generated in $outputFile"
    Write-Host "To generate an image, run: dot -Tpng $outputFile -o class_namespaces.png"
}

function Generate-CMakeGraph {
    param (
        [string]$projectRoot
    )
    
    Write-Host "Generating CMake targets graph..."
    $cmakeFiles = Get-ChildItem -Path $projectRoot -Recurse -Filter "CMakeLists.txt"
    
    if ($cmakeFiles.Count -eq 0) {
        Write-Host "No CMakeLists.txt files found in the project."
        return
    }
    
    $targets = @{}
    $dependencies = @()
    $filesByTarget = @{}
    
    foreach ($file in $cmakeFiles) {
        $content = Get-Content $file.FullName
        $directoryName = Split-Path -Path $file.Directory -Leaf
        $directoryPath = Split-Path -Path $file.FullName -Parent
        
        $currentTarget = ""
        $projectName = ""
        
        for ($i = 0; $i -lt $content.Count; $i++) {
            $line = $content[$i]
            
            if ($line -match 'project\s*\(\s*(\w+)') {
                $projectName = $matches[1]
            }
            
            if ($line -match '(add_executable|add_library)\s*\(\s*(\w+)') {
                $targetType = $matches[1]
                $targetName = $matches[2]
                $currentTarget = $targetName
                
                $targets[$targetName] = @{
                    Type = $targetType
                    Directory = $directoryName
                    Project = $projectName
                }
                
                $filesByTarget[$targetName] = @()
                
                $j = $i
                $multiline = $line
                while ($j -lt $content.Count -and -not ($multiline -match '\)')) {
                    $j++
                    if ($j -lt $content.Count) {
                        $multiline += " " + $content[$j]
                    }
                }
                
                $sourceMatches = [regex]::Matches($multiline, '[\w\.]+\.(cpp|h|hpp)')
                foreach ($sourceMatch in $sourceMatches) {
                    $sourceFile = $sourceMatch.Value
                    $filesByTarget[$targetName] += $sourceFile
                }
            }
            
            if ($line -match 'target_link_libraries\s*\(\s*(\w+)\s+(.+)') {
                $targetName = $matches[1]
                $depsLine = $matches[2]
                
                $j = $i
                while ($j -lt $content.Count -and -not ($depsLine -match '\)')) {
                    $j++
                    if ($j -lt $content.Count) {
                        $depsLine += " " + $content[$j]
                    }
                }
                
                $depsLine = $depsLine -replace '\)', ''
                $deps = $depsLine -split '\s+'
                
                foreach ($dep in $deps) {
                    $dep = $dep.Trim()
                    if ($dep -and $dep -ne '' -and $dep -notmatch '^\${' -and $dep -notmatch '^-') {
                        $dependencies += @{
                            Source = $dep      # Inverted: dependency -> dependent
                            Target = $targetName
                        }
                    }
                }
            }
        }
    }
    
    $dotContent = "digraph CMakeTargets {`n"
    $dotContent += "  rankdir=LR;`n"
    $dotContent += "  node [shape=box, style=filled, fillcolor=lightblue];`n"
    
    foreach ($target in $targets.Keys) {
        $shape = if ($targets[$target].Type -eq "add_executable") { "box" } else { "ellipse" }
        $label = "$target\n($($targets[$target].Type))"
        $safeTarget = Sanitize-DotName($target)
        $dotContent += "  `"$safeTarget`" [label=`"$label`", shape=$shape];`n"
    }
    
    # Get all projects with safe access to Project property
    $projects = @()
    foreach ($target in $targets.Values) {
        if ($target.PSObject.Properties.Name -contains "Project" -and $target.Project) {
            if (-not $projects -contains $target.Project) {
                $projects += $target.Project
            }
        }
    }
    
    foreach ($project in $projects) {
        $dotContent += "  subgraph `"cluster_$project`" {`n"
        $dotContent += "    label=`"Project: $project`";`n"
        $dotContent += "    style=filled;`n"
        $dotContent += "    color=lightgrey;`n"
        
        foreach ($target in $targets.Keys) {
            if ($targets[$target].PSObject.Properties.Name -contains "Project" -and 
                $targets[$target].Project -eq $project) {
                $safeTarget = Sanitize-DotName($target)
                $dotContent += "    `"$safeTarget`";`n"
            }
        }
        
        $dotContent += "  }`n"
    }
    
    # Add all edges with safe identifiers
    foreach ($dep in $dependencies) {
        $safeSource = Sanitize-DotName($dep.Source)
        $safeTarget = Sanitize-DotName($dep.Target)
        $dotContent += "  `"$safeSource`" -> `"$safeTarget`";`n"
    }
    
    $dotContent += "}`n"
    
    $outputFile = "cmake_targets.dot"
    $dotContent | Out-File -FilePath $outputFile -Encoding utf8
    
    Write-Host "CMake targets graph generated in $outputFile"
    Write-Host "To generate an image, run: dot -Tpng $outputFile -o cmake_targets.png"
}


function Generate-CircularInclusionGraph {
    param (
        [string]$srcDir,
        [string]$includeDir
    )
    
    Write-Host "Detecting circular inclusions..."
    $cppFiles = Get-CppFiles -srcDir $srcDir -includeDir $includeDir
    
    # Build inclusion graph
    $inclusionGraph = @{}
    $filePathMap = @{}
    
    foreach ($file in $cppFiles) {
        $relativePath = $file.FullName.Replace("$((Get-Location).Path)\", "")
        $relativePathNode = Sanitize-DotName($relativePath.Replace("\", "/"))
        $filePathMap[$file.Name] = $relativePathNode
        $inclusionGraph[$relativePathNode] = @()
    }
    
    foreach ($file in $cppFiles) {
        $content = Get-Content $file.FullName
        $relativePathSource = Sanitize-DotName($file.FullName.Replace("$((Get-Location).Path)\", "").Replace("\", "/"))
        
        foreach ($line in $content) {
            if ($line -match '#include\s+[<"]([^>"]+)[>"]') {
                $includedFile = $matches[1]
                $includedFileName = [System.IO.Path]::GetFileName($includedFile)
                
                if ($filePathMap.ContainsKey($includedFileName)) {
                    $inclusionGraph[$relativePathSource] += $filePathMap[$includedFileName]
                }
            }
        }
    }
    
    # Detect cycles
    $visited = @{}
    $recursionStack = @{}
    $cycles = @()
    
    function DFS-Cycle {
        param (
            [string]$node,
            [System.Collections.ArrayList]$path
        )
        
        # Mark the current node as visited and add to recursion stack
        $visited[$node] = $true
        $recursionStack[$node] = $true
        
        $path.Add($node) | Out-Null
        
        # Visit all adjacent vertices
        foreach ($neighbor in $inclusionGraph[$node]) {
            # If not visited, recursively check
            if (-not $visited.ContainsKey($neighbor) -or -not $visited[$neighbor]) {
                DFS-Cycle -node $neighbor -path $path
            }
            # If already in recursion stack, we found a cycle
            elseif ($recursionStack.ContainsKey($neighbor) -and $recursionStack[$neighbor]) {
                # Find the start of the cycle in the path
                $cycleStartIndex = $path.IndexOf($neighbor)
                if ($cycleStartIndex -ge 0) {
                    $cycle = $path.GetRange($cycleStartIndex, $path.Count - $cycleStartIndex)
                    $cycle.Add($neighbor) | Out-Null
                    $cycles += , $cycle
                }
            }
        }
        
        # Remove the node from the recursion stack and path
        $recursionStack[$node] = $false
        $path.RemoveAt($path.Count - 1)
    }
    
    # Check for cycles in each connected component
    foreach ($node in $inclusionGraph.Keys) {
        if (-not $visited.ContainsKey($node) -or -not $visited[$node]) {
            $path = New-Object System.Collections.ArrayList
            DFS-Cycle -node $node -path $path
        }
    }
    
    # Generate DOT file for circular inclusions
    $dotContent = "digraph CircularInclusions {`n"
    $dotContent += "  rankdir=LR;`n"
    $dotContent += "  node [shape=box, style=filled, fillcolor=lightblue];`n"
    
    # Add nodes
    foreach ($node in $inclusionGraph.Keys) {
        $label = [System.IO.Path]::GetFileName($node)
        $safeNode = Sanitize-DotName($node)
        $dotContent += "  `"$safeNode`" [label=`"$label`"];`n"
    }
    
    # Highlight circular inclusions
    $highlightedEdges = @{}
    
    if ($cycles.Count -gt 0) {
        Write-Host "Found $($cycles.Count) circular inclusions:"
        
        foreach ($cycle in $cycles) {
            Write-Host "Circular inclusion detected: " -NoNewline
            
            for ($i = 0; $i -lt $cycle.Count - 1; $i++) {
                $source = $cycle[$i]
                $target = $cycle[$i + 1]
                $safeSource = Sanitize-DotName($source)
                $safeTarget = Sanitize-DotName($target)
                
                Write-Host "$([System.IO.Path]::GetFileName($source)) -> " -NoNewline
                
                $edgeKey = "$safeSource -> $safeTarget"
                $highlightedEdges[$edgeKey] = $true
            }
            
            # Close the cycle
            $lastSource = $cycle[$cycle.Count - 1]
            $lastTarget = $cycle[0]
            $safeLastSource = Sanitize-DotName($lastSource)
            $safeLastTarget = Sanitize-DotName($lastTarget)
            
            Write-Host "$([System.IO.Path]::GetFileName($lastSource)) -> $([System.IO.Path]::GetFileName($lastTarget))"
            
            $edgeKey = "$safeLastSource -> $safeLastTarget"
            $highlightedEdges[$edgeKey] = $true
        }
    } else {
        Write-Host "No circular inclusions detected."
    }
    
    # Add all edges to the graph
    foreach ($node in $inclusionGraph.Keys) {
        $safeNode = Sanitize-DotName($node)
        foreach ($neighbor in $inclusionGraph[$node]) {
            $safeNeighbor = Sanitize-DotName($neighbor)
            $edgeKey = "$safeNode -> $safeNeighbor"
            if ($highlightedEdges.ContainsKey($edgeKey)) {
                $dotContent += "  `"$safeNode`" -> `"$safeNeighbor`" [color=red, penwidth=2.0];`n"
            } else {
                $dotContent += "  `"$safeNode`" -> `"$safeNeighbor`";`n"
            }
        }
    }
    
    $dotContent += "}`n"
    
    $outputFile = "circular_inclusions.dot"
    $dotContent | Out-File -FilePath $outputFile -Encoding utf8
    
    Write-Host "Circular inclusions graph generated in $outputFile"
    Write-Host "To generate an image, run: dot -Tpng $outputFile -o circular_inclusions.png"
}


function Analyze-CppProject {
    param (
        [Parameter(Mandatory=$true)]
        [ValidateSet(1, 2, 3, 4)]
        [int]$Mode,
        
        [string]$CustomSrcDir,
        
        [string]$CustomIncludeDir
    )
    
    $projectRoot = Get-Location
    $srcDir = Join-Path $projectRoot "src"
    $dotSrcDir = Join-Path $projectRoot ".src"
    $includeDir = Join-Path $projectRoot "include"
    
    # Use custom directories if provided
    if ($CustomSrcDir) {
        if (Test-Path $CustomSrcDir) {
            $srcDir = $CustomSrcDir
            Write-Host "Using custom source directory: $CustomSrcDir"
        } else {
            Write-Host "Warning: Custom source directory '$CustomSrcDir' not found."
        }
    } else {
        # Check if .src exists
        if (Test-Path $dotSrcDir) {
            $srcDir = $dotSrcDir
            Write-Host "Found .src directory"
        }
    }
    
    if ($CustomIncludeDir) {
        if (Test-Path $CustomIncludeDir) {
            $includeDir = $CustomIncludeDir
            Write-Host "Using custom include directory: $CustomIncludeDir"
        } else {
            Write-Host "Warning: Custom include directory '$CustomIncludeDir' not found."
        }
    }
    
    if (-not (Test-Path $srcDir) -and -not (Test-Path $includeDir)) {
        Write-Host "Warning: Neither source nor include directories exist in this location."
        Write-Host "The script will search for files in the current directory."
        $srcDir = $projectRoot
        $includeDir = $projectRoot
    }
    
    switch ($Mode) {
        1 { Generate-FileInclusionGraph $srcDir $includeDir }
        2 { Generate-ClassNamespaceGraph $srcDir $includeDir }
        3 { Generate-CMakeGraph $projectRoot }
        4 { Generate-CircularInclusionGraph $srcDir $includeDir }
        default { Write-Error "Unsupported mode" }
    }
}

function Show-Help {
    Write-Host "CppProjectMapper - Graph Generation Tool for C++ Projects"
    Write-Host "Usage: .\CppProjectMapper.ps1 <Mode> [<CustomSrcDir> [<CustomIncludeDir>]]"
    Write-Host ""
    Write-Host "Available modes:"
    Write-Host "  1 - File Inclusion Mapping"
    Write-Host "      Generates a graph showing which files include which other files."
    Write-Host ""
    Write-Host "  2 - Class and Namespace Mapping"
    Write-Host "      Generates a graph showing relationships between classes and their organization in namespaces."
    Write-Host ""
    Write-Host "  3 - CMakeLists Configuration Mapping"
    Write-Host "      Generates a graph showing CMake targets and their dependencies."
    Write-Host ""
    Write-Host "  4 - Circular Inclusion Detection"
    Write-Host "      Detects and visualizes circular dependencies between header files."
    Write-Host ""
    Write-Host "Optional parameters:"
    Write-Host "  <CustomSrcDir>      - Custom source directory path (instead of src/.src)"
    Write-Host "  <CustomIncludeDir>  - Custom include directory path (instead of include)"
    Write-Host ""
    Write-Host "The script should be executed at the root of the project. By default, it looks for"
    Write-Host "src/.src and/or include directories, but you can specify custom directories."
    Write-Host ""
    Write-Host "Example usage:"
    Write-Host "  1. Basic usage with default directories:"
    Write-Host "     .\CppProjectMapper.ps1 2"
    Write-Host ""
    Write-Host "  2. With custom source directory:"
    Write-Host "     .\CppProjectMapper.ps1 2 F:\Code\MyProject\source"
    Write-Host ""
    Write-Host "  3. With custom source and include directories:"
    Write-Host "     .\CppProjectMapper.ps1 2 F:\Code\MyProject\source F:\Code\MyProject\headers"
}

function Main {
    param (
        [Parameter(Position=0)]
        [string]$ModeArg,
        
        [Parameter(Position=1)]
        [string]$CustomSrcDir,
        
        [Parameter(Position=2)]
        [string]$CustomIncludeDir
    )
    
    if ($ModeArg -eq "" -or $ModeArg -eq "-h" -or $ModeArg -eq "--help") {
        Show-Help
        return
    }
    
    $Mode = 0
    if (-not [int]::TryParse($ModeArg, [ref]$Mode) -or $Mode -lt 1 -or $Mode -gt 4) {
        Write-Host "Invalid mode: $ModeArg"
        Show-Help
        return
    }
    
    $params = @{
        Mode = $Mode
    }
    
    if ($CustomSrcDir) {
        $params.CustomSrcDir = $CustomSrcDir
    }
    
    if ($CustomIncludeDir) {
        $params.CustomIncludeDir = $CustomIncludeDir
    }
    
    Analyze-CppProject @params
}

# Execute main function with arguments
Main $args[0] $args[1] $args[2]