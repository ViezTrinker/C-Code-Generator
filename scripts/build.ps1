#Requires -Version 5.1
<#
.SYNOPSIS
  Fetches optional local SFML sources into third_party/sfml and builds the project.

.DESCRIPTION
  - nlohmann_json is already vendored under third_party/nlohmann/
  - SFML is downloaded into third_party/sfml on first run (or CMake FetchContent does it)
  - Then configures and builds with CMake

.PARAMETER Configuration
  Debug or Release (default: Release)

.PARAMETER Generator
  Optional CMake generator. Default: Ninja if available, else Visual Studio 17 2022.

.PARAMETER SkipFetch
  Skip downloading SFML into third_party (CMake may still FetchContent).
#>
param(
   [ValidateSet("Debug", "Release")]
   [string]$Configuration = "Release",
   [string]$Generator = "",
   [switch]$SkipFetch
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $RepoRoot

function Test-CommandExists([string]$Name)
{
   return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

if (-not (Test-CommandExists "cmake"))
{
   throw "cmake not found on PATH. Install CMake or open a Visual Studio Developer PowerShell."
}

$SfmlVersion = "3.0.2"
$SfmlDir = Join-Path $RepoRoot "third_party\sfml"
$SfmlMarker = Join-Path $SfmlDir "CMakeLists.txt"

if ((-not $SkipFetch) -and (-not (Test-Path $SfmlMarker)))
{
   Write-Host "Downloading SFML $SfmlVersion into third_party/sfml ..."
   $TempZip = Join-Path $env:TEMP "SFML-$SfmlVersion.zip"
   $TempExtract = Join-Path $env:TEMP "SFML-$SfmlVersion-extract"
   $Url = "https://github.com/SFML/SFML/archive/refs/tags/$SfmlVersion.zip"

   Invoke-WebRequest -Uri $Url -OutFile $TempZip
   if (Test-Path $TempExtract)
   {
      Remove-Item -Recurse -Force $TempExtract
   }
   Expand-Archive -Path $TempZip -DestinationPath $TempExtract -Force

   $Extracted = Join-Path $TempExtract "SFML-$SfmlVersion"
   if (-not (Test-Path $Extracted))
   {
      throw "Unexpected SFML archive layout under $TempExtract"
   }

   New-Item -ItemType Directory -Force -Path (Split-Path $SfmlDir) | Out-Null
   if (Test-Path $SfmlDir)
   {
      Remove-Item -Recurse -Force $SfmlDir
   }
   Move-Item $Extracted $SfmlDir
   Write-Host "SFML ready at $SfmlDir"
}

$BuildDir = Join-Path $RepoRoot "build"
$CMakeArgs = @(
   "-S", $RepoRoot,
   "-B", $BuildDir,
   "-DCMAKE_BUILD_TYPE=$Configuration"
)

if ($Generator -ne "")
{
   $CMakeArgs += @("-G", $Generator)
   if ($Generator -like "Visual Studio*")
   {
      $CMakeArgs += @("-A", "x64")
      # Multi-config generators ignore CMAKE_BUILD_TYPE at configure time.
      $CMakeArgs = $CMakeArgs | Where-Object { $_ -notlike "-DCMAKE_BUILD_TYPE=*" }
   }
}
elseif (Test-CommandExists "ninja")
{
   $CMakeArgs += @("-G", "Ninja")
}
elseif (Test-CommandExists "cl")
{
   $CMakeArgs += @("-G", "Visual Studio 17 2022", "-A", "x64")
   $CMakeArgs = $CMakeArgs | Where-Object { $_ -notlike "-DCMAKE_BUILD_TYPE=*" }
}

Write-Host "Configuring: cmake $($CMakeArgs -join ' ')"
& cmake @CMakeArgs
if ($LASTEXITCODE -ne 0)
{
   throw "CMake configure failed"
}

Write-Host "Building ($Configuration) ..."
if (($Generator -like "Visual Studio*") -or ((-not $Generator) -and (Test-Path (Join-Path $BuildDir "c_code_generator.sln"))))
{
   & cmake --build $BuildDir --config $Configuration
}
else
{
   & cmake --build $BuildDir
}
if ($LASTEXITCODE -ne 0)
{
   throw "CMake build failed"
}

$ExeCandidates = @(
   (Join-Path $BuildDir "c_code_generator.exe"),
   (Join-Path $BuildDir "$Configuration\c_code_generator.exe"),
   (Join-Path $BuildDir "Release\c_code_generator.exe"),
   (Join-Path $BuildDir "Debug\c_code_generator.exe")
)

$ExePath = $ExeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if ($null -eq $ExePath)
{
   Write-Host "Build finished, but executable path was not detected automatically."
}
else
{
   Write-Host "Build OK: $ExePath"
   Write-Host "Run: `"$ExePath`""
   Write-Host "Self-test: `"$ExePath`" --self-test"
}
