<#
.SYNOPSIS
  Deploys Qt runtime next to kameraa.exe and builds the Inno Setup installer.

.DESCRIPTION
  Run on Windows after scripts\build-windows.ps1. Uses windeployqt to gather Qt
  DLLs/plugins into a deploy\ folder, copies the models manifest and the virtual
  camera DLL (if built), then invokes Inno Setup (iscc) to produce the installer.

  Code signing is a separate step (requires a certificate) — see
  docs/10-BUILD-AND-PACKAGING.md.
#>
param(
    [string]$Config = "Release",
    [string]$Version = "1.0.0"
)
$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $repo

$exe = Join-Path $repo "build\windows\src\app\$Config\kameraa.exe"
if (-not (Test-Path $exe)) { throw "kameraa.exe not found. Run scripts\build-windows.ps1 first." }

$deploy = Join-Path $repo "deploy"
if (Test-Path $deploy) { Remove-Item $deploy -Recurse -Force }
New-Item -ItemType Directory -Path $deploy | Out-Null
Copy-Item $exe $deploy

# Gather Qt runtime.
$windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($windeployqt) {
    & $windeployqt.Source --release --no-translations --dir $deploy $exe
} else {
    Write-Warning "windeployqt not found on PATH; Qt DLLs will not be bundled."
}

# Models manifest (model binaries are added/downloaded by the user).
New-Item -ItemType Directory -Path (Join-Path $deploy "models") -Force | Out-Null
Copy-Item (Join-Path $repo "models\manifest.json") (Join-Path $deploy "models") -Force

# Virtual camera filter DLL (optional — only if the virtualcam target was built).
$dll = Join-Path $repo "build\windows\virtualcam\$Config\kameraa_vcam.dll"
if (Test-Path $dll) { Copy-Item $dll $deploy } else {
    Write-Warning "kameraa_vcam.dll not found; installer will lack the virtual camera. Build with -DKAMERAA_BUILD_DSHOW_FILTER=ON."
}

# Build the installer.
$iscc = Get-Command iscc -ErrorAction SilentlyContinue
if ($iscc) {
    & $iscc.Source "/DAppVersion=$Version" (Join-Path $repo "packaging\windows\kameraa.iss")
    Write-Host "Installer built in packaging\windows\Output (or configured OutputDir)." -ForegroundColor Green
} else {
    Write-Warning "Inno Setup (iscc) not found. Install it, then run: iscc packaging\windows\kameraa.iss"
}
