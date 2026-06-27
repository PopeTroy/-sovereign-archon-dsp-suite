[Setup]
AppName=UESP Sovereign Archon Bundle
AppVersion=6.0.0
DefaultDirName={commoncf}\VST3\UESP_Sovereign
DefaultGroupName=UESP Sovereign Audio
OutputBaseFilename=Sovereign-Archon-Bundle-Windows-Installer
Compression=lzma2/ultra
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Files]
Source: "..\build\plugins\SovereignCompressor\Release\VST3\SovereignCompressor.vst3\*"; DestDir: "{app}\SovereignCompressor.vst3"; Flags: ignoreversion recursesubdirs
Source: "..\build\plugins\SovereignSaturation\Release\VST3\SovereignSaturation.vst3\*"; DestDir: "{app}\SovereignSaturation.vst3"; Flags: ignoreversion recursesubdirs
Source: "..\build\plugins\SovereignReverb\Release\VST3\SovereignReverb.vst3\*"; DestDir: "{app}\SovereignReverb.vst3"; Flags: ignoreversion recursesubdirs
Source: "..\build\plugins\SovereignDelay\Release\VST3\SovereignDelay.vst3\*"; DestDir: "{app}\SovereignDelay.vst3"; Flags: ignoreversion recursesubdirs
Source: "..\build\plugins\SovereignEqualizer\Release\VST3\SovereignEqualizer.vst3\*"; DestDir: "{app}\SovereignEqualizer.vst3"; Flags: ignoreversion recursesubdirs
Source: "..\build\plugins\SovereignMaximizer\Release\VST3\SovereignMaximizer.vst3\*"; DestDir: "{app}\SovereignMaximizer.vst3"; Flags: ignoreversion recursesubdirs

[Messages]
WelcomeLabel2=This master installer drops the entire UESP Sovereign Archon processing suite (Compressor, Saturation, Reverb, Delay, Equalizer, and Maximizer) natively inside your production software rails.
