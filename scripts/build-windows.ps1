<#
.SYNOPSIS
  Configure and build Kameraa on Windows (MSVC) into build/windows.

.DESCRIPTION
  Bootstraps vcpkg (if needed), then configures and builds with the "windows"
  CMake preset. Requires: Visual Studio 2022 (C++), CMake, Git.

.PARAMETER VcpkgRoot
  Path to an existing vcpkg checkout. If omitted, uses $env:VCPKG_ROOT or clones
  one into .vcpkg.

.PARAMETER Tests
  Also build unit tests (adds the vcpkg 'tests' feature + GoogleTest).

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1 -Tests
#>
param(
    [string]$VcpkgRoot = $env:VCPKG_ROOT,
    [switch]$Tests,
    [switch]$Onnx
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

function Require($name) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        throw "'$name' not found on PATH. Install it and retry. See docs/10-BUILD-AND-PACKAGING.md."
    }
}
Require cmake
Require git

# Bootstrap vcpkg if no root was supplied.
if (-not $VcpkgRoot) {
    $VcpkgRoot = Join-Path $repo ".vcpkg"
    if (-not (Test-Path (Join-Path $VcpkgRoot "vcpkg.exe"))) {
        if (-not (Test-Path $VcpkgRoot)) {
            git clone https://github.com/microsoft/vcpkg $VcpkgRoot
        }
        & (Join-Path $VcpkgRoot "bootstrap-vcpkg.bat")
    }
}
$toolchain = Join-Path $VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
Write-Host "Using vcpkg toolchain: $toolchain"

$features = @()
if ($Tests) { $features += "tests" }
$manifestFeatures = if ($features) { "-DVCPKG_MANIFEST_FEATURES=$($features -join ';')" } else { "" }

$cmakeArgs = @(
    "--preset", "windows",
    "-DCMAKE_TOOLCHAIN_FILE=$toolchain"
)
if ($Tests) { $cmakeArgs += "-DKAMERAA_BUILD_TESTS=ON" }
if ($Onnx)  { $cmakeArgs += "-DKAMERAA_ENABLE_ONNX=ON" }
if ($manifestFeatures) { $cmakeArgs += $manifestFeatures }

Write-Host "Configuring..." -ForegroundColor Cyan
cmake @cmakeArgs

Write-Host "Building (Release)..." -ForegroundColor Cyan
cmake --build --preset windows --config Release

if ($Tests) {
    Write-Host "Running tests..." -ForegroundColor Cyan
    ctest --test-dir build/windows -C Release --output-on-failure
}

Write-Host "Done. Binaries in build/windows/src/app/Release." -ForegroundColor Green
