<#
.SYNOPSIS
  Uninstalls the build toolchain that was installed for Kameraa (run as Administrator).

.DESCRIPTION
  Removes Visual Studio 2022 Build Tools, CMake, Inno Setup, and (optionally) Git
  via winget. Git is kept by default since you may use it elsewhere — pass
  -RemoveGit to remove it too.

.EXAMPLE
  # In an ADMIN PowerShell:
  powershell -ExecutionPolicy Bypass -File scripts\uninstall-windows-toolchain.ps1
  powershell -ExecutionPolicy Bypass -File scripts\uninstall-windows-toolchain.ps1 -RemoveGit
#>
param([switch]$RemoveGit)

$ErrorActionPreference = 'Continue'

function Assert-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $p = New-Object Security.Principal.WindowsPrincipal($id)
    if (-not $p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "Run this in an Administrator PowerShell (right-click > Run as administrator)."
    }
}

function Remove-Pkg($id) {
    Write-Host "Uninstalling $id ..." -ForegroundColor Cyan
    winget uninstall --id $id -e --accept-source-agreements --silent
}

Assert-Admin

Remove-Pkg 'Microsoft.VisualStudio.2022.BuildTools'   # the large one (several GB)
Remove-Pkg 'Microsoft.VisualStudio.Installer'         # VS Installer + Package Cache
Remove-Pkg 'Kitware.CMake'
Remove-Pkg 'JRSoftware.InnoSetup'                      # no-op if it wasn't installed
if ($RemoveGit) { Remove-Pkg 'Git.Git' }

# Remove any now-orphaned Visual Studio leftover folders.
$leftovers = @(
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer"
)
foreach ($p in $leftovers) {
    if (Test-Path $p) {
        Write-Host "Removing leftover: $p" -ForegroundColor Cyan
        Remove-Item $p -Recurse -Force -ErrorAction SilentlyContinue
    }
}

Write-Host ""
Write-Host "Toolchain uninstall finished." -ForegroundColor Green
if (-not $RemoveGit) { Write-Host "Git was kept. Re-run with -RemoveGit to remove it too." -ForegroundColor Yellow }
