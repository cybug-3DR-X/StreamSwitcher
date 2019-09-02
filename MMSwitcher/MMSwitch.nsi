; MMSwitch.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The instalelr simply 
; prompts the user asking them where to install, and drops of notepad.exe
; there. If your Windows directory is not C:\windows, change it below.
;

; The name of the installer
Name "Morgan Stream Switcher v0.99"

LicenseText "Morgan Stream Switcher"
LicenseData "MMSwitch.txt"
;EnabledBitmap bitmap1.bmp
;DisabledBitmap bitmap2.bmp
CheckBitmap ".\bitmap.bmp"
Icon "main.ico"
;BGGradient
BrandingText " "

; The file to write
OutFile "MMSwitch.exe"


; The default installation directory
InstallDir "$PROGRAMFILES\Morgan\mmswitch"
;InstallDir $SYSDIR

; uninstall stuff
;UninstallText "This will Morgan Stream Switcher. Hit next to continue."
;UninstallExeName "uninst.exe"
UninstallIcon "uninst.ico"

; The text to prompt the user to enter a directory
DirText "This will install Morgan Stream Switcher on your computer. Choose a directory" "" ""

Section ""
; write reg crap
WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\Morgan\mmswitch "Install_Dir" "$INSTDIR"
; write uninstall strings
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\mmswitch" "DisplayName" "Morgan Stream Switcher"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\mmswitch" "UninstallString" '"$INSTDIR\uninst.exe"'
SectionEnd

; The stuff to install
Section "Morgan Stream Switcher"
; Set output path to the installation directory.
SetOutPath $INSTDIR
WriteUninstaller "uninst.exe"
; Put file there
File ".\MMSwitch.ax"
File "..\MMSwitch\Release\MMSwitch.dll"
File "..\MMAVILng\Release\MMAVILng.exe"
File ".\doc.ico"
File ".\web.ico"
CreateDirectory "$SMPROGRAMS\Morgan Stream Switcher"
CreateShortCut "$SMPROGRAMS\Morgan Stream Switcher\Morgan Stream Switcher.lnk" "http:\\www.morgan-multimedia.com/mmswitch/" "" "$INSTDIR\doc.ico"
CreateShortCut "$SMPROGRAMS\Morgan Stream Switcher\Morgan Multimedia web site.lnk" "http:\\www.morgan-multimedia.com/" "" "$INSTDIR\web.ico"
CreateShortCut "$SMPROGRAMS\Morgan Stream Switcher\Uninstall.lnk" "$INSTDIR\uninst.exe"
CopyFiles "$INSTDIR\MMSwitch.ax" "$SYSDIR\" 58
CopyFiles "$INSTDIR\MMSwitch.dll" "$SYSDIR\" 76
CopyFiles "$INSTDIR\MMAVILng.exe" "$SYSDIR\" 40
RegDLL "$SYSDIR\MMSwitch.ax"
RegDLL "$SYSDIR\MMSwitch.dll"
SectionEnd

; special uninstall section.
Section "Uninstall"
DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\mmswitch"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Morgan\mmswitch"
Delete "$SMPROGRAMS\Morgan Stream Switcher\*.*"
RMDir "$SMPROGRAMS\Morgan Stream Switcher"
UnRegDLL "$SYSDIR\MMSwitch.dll"
Delete "$SYSDIR\MMSwitch.dll"
UnRegDLL "$SYSDIR\MMSwitch.ax"
Delete "$SYSDIR\MMSwitch.ax"
Delete "$INSTDIR\*.*"
RMDir "$INSTDIR"
SectionEnd
; eof
