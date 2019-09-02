; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The instalelr simply 
; prompts the user asking them where to install, and drops of notepad.exe
; there. If your Windows directory is not C:\windows, change it below.
;

; The name of the installer
Name "Example1"

; The file to write
OutFile "example1.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Example1

; The text to prompt the user to enter a directory
DirText "This will install the very simple example1 on your computer. Choose a directory"

; The stuff to install
Section "ThisNameIsIgnoredSoWhyBother?"
; Set output path to the installation directory.
SetOutPath $INSTDIR
; Put file there
File "C:\windows\notepad.exe"

; eof
