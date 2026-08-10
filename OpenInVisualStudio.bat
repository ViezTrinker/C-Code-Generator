@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
   echo Visual Studio wurde nicht gefunden ^(vswhere fehlt^).
   echo Bitte Visual Studio 2022/2026 mit C++-Workload installieren.
   pause
   exit /b 1
)

set "DEVENV="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property productPath`) do (
   set "DEVENV=%%I"
)

if not defined DEVENV (
   for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -property productPath`) do (
      set "DEVENV=%%I"
   )
)

if not defined DEVENV (
   echo Keine Visual-Studio-Installation gefunden.
   pause
   exit /b 1
)

if not exist "%DEVENV%" (
   echo devenv.exe nicht gefunden:
   echo %DEVENV%
   pause
   exit /b 1
)

echo Oeffne Ordner in Visual Studio:
echo %CD%
start "" "%DEVENV%" "%CD%"
exit /b 0
