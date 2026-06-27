[Setup]
AppName=UESP Sovereign Archon Suite
AppVersion=6.0.0
DefaultDirName={commoncf}\VST3\UESP_Sovereign
DefaultGroupName=UESP Sovereign Audio
OutputBaseFilename=Sovereign-Archon-Bundle-Windows-Installer
Compression=lzma2/ultra
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
; Grabs your unified dynamic processor library file cleanly out of the build tree artifacts directory
Source: "..\build\SovereignArchonSuite_artefacts\Release\VST3\SovereignArchonSuite.vst3\*"; DestDir: "{app}\SovereignArchonSuite.vst3"; Flags: ignoreversion recursesubdirs

[Messages]
WelcomeLabel2=This master installer initializes the entire UESP Sovereign Archon 6-in-1 processing engine matrix cleanly inside your workstation infrastructure rails.
