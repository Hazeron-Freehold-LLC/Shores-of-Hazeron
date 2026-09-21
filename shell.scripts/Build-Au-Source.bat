@echo off
echo Configuring Visual Studio 14.0 Build Environment
echo ++++++++++++++++++++++++++++++++++++++++++++++++
if "%AUARCH%" == "soh-win64" (
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
) else (
	call "C:\Program Files\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
)

echo Building
echo ++++++++
c:
cd %SEIDEV%\au
echo Deleting files from release folder.
del /f /q release\*
call nmakeall.bat
