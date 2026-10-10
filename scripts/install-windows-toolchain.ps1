<#
.SYNOPSIS
  Installs the Windows build toolchain for Kameraa (run as Administrator).

.DESCRIPTION
  Installs Git, CMake, Visual Studio 2022 Build Tools (C++ workload) and Inno Setup
  via winget. After this finishes, open a NEW terminal and run:
    scripts\build-windows.ps1
  then:
    release\windows\build-installer.ps1
  to produce the installer in packaging\windows\Output.

  NOTE: Qt6 and OpenCV are then built from source by vcpkg on first build, which
  can take a long time and several GB of disk.

.EXAMPLE
  # In an ADMIN PowerShell:
  powershell -ExecutionPolicy Bypass -File scripts\install-windows-toolchain.ps1
#>
$ErrorActionPreference = 'Stop'

function Assert-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $p = New-Object Security.Principal.WindowsPrincipal($id)
    if (-not $p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "Run this script in an Administrator PowerShell (right-click > Run as administrator)."
    }
}

function Install-Pkg($id, $extra = @()) {
    Write-Host "Installing $id ..." -ForegroundColor Cyan
    $args = @('install', '--id', $id, '-e', '--source', 'winget',
              '--accept-package-agreements', '--accept-source-agreements') + $extra
    winget @args
    if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne -1978335189) {
        # -1978335189 = already installed / no applicable update
        Write-Warning "winget returned $LASTEXITCODE for $id (continuing)."
    }
}

Assert-Admin

Install-Pkg 'Git.Git'
Install-Pkg 'Kitware.CMake'
Install-Pkg 'JRSoftware.InnoSetup'

# Visual Studio 2022 Build Tools with the C++ (Desktop) workload.
Write-Host "Installing Visual Studio 2022 Build Tools (C++)..." -ForegroundColor Cyan
winget install --id Microsoft.VisualStudio.2022.BuildTools -e --source winget `
    --accept-package-agreements --accept-source-agreements `
    --override "--quiet --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

Write-Host ""
Write-Host "Toolchain install finished." -ForegroundColor Green
Write-Host "Open a NEW terminal, then run:" -ForegroundColor Green
Write-Host "  scripts\build-windows.ps1" -ForegroundColor Yellow
Write-Host "  release\windows\build-installer.ps1" -ForegroundColor Yellow
