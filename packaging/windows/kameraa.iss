; Inno Setup script for the Kameraa Windows installer.
; Produces Kameraa-Setup-<version>.exe from a prepared 'deploy' folder
; (kameraa.exe + Qt runtime via windeployqt + models\manifest.json, and
; optionally the virtual-camera filter DLL).
;
; Build locally: release\windows\build-installer.ps1
; Build in CI:   .github/workflows/windows-installer.yml

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#define AppName "Kameraa"
#define AppPublisher "Kameraa"

[Setup]
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
OutputDir=Output
OutputBaseFilename=Kameraa-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
WizardStyle=modern

[Files]
; Everything windeployqt produced (app exe + Qt DLLs/plugins + models), except the
; virtual-camera DLL which needs explicit COM registration below.
Source: "..\..\deploy\*"; DestDir: "{app}"; Excludes: "kameraa_vcam.dll"; \
    Flags: ignoreversion recursesubdirs createallsubdirs
; Virtual camera filter DLL (optional): registered via DllRegisterServer if present.
Source: "..\..\deploy\kameraa_vcam.dll"; DestDir: "{app}"; \
    Flags: ignoreversion regserver skipifsourcedoesntexist

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\kameraa.exe"
Name: "{commondesktop}\{#AppName}"; Filename: "{app}\kameraa.exe"

[Run]
Filename: "{app}\kameraa.exe"; Description: "Launch Kameraa"; \
    Flags: nowait postinstall skipifsilent

; On uninstall, Inno automatically unregisters the regserver DLL.
