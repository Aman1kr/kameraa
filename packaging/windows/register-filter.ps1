# Registers or unregisters the Kameraa DirectShow virtual camera filter DLL.
# Run as Administrator. See docs/05-PLATFORM-INTEGRATION.md.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File register-filter.ps1 [-Unregister] [-DllPath <path>]

param(
    [switch]$Unregister,
    [string]$DllPath = "$PSScriptRoot\kameraa_vcam.dll"
)

$ErrorActionPreference = 'Stop'

function Assert-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($id)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw "This script must be run as Administrator."
    }
}

Assert-Admin

if (-not (Test-Path $DllPath)) {
    throw "Filter DLL not found: $DllPath. Build the virtualcam target first."
}

if ($Unregister) {
    Write-Host "Unregistering $DllPath ..."
    & regsvr32.exe /u /s $DllPath
    Write-Host "Unregistered. Restart your video apps so they re-scan cameras."
} else {
    Write-Host "Registering $DllPath ..."
    & regsvr32.exe /s $DllPath
    Write-Host "Registered 'Kameraa Virtual Camera'. Restart your video apps."
}
