@echo off
echo Setting environment for building Au.
if "%SEIDEV%" == "" (
	echo Error: SEIDEV not defined. Typically set to C:\dev.
	exit /B
)

if "%AUARCH%" == "soh-win64" (
	rem Configure for 64-bit Windows build.
	set "AUARCHSTEAM=hss-win64"
	rem The OpenAL binaries subfolder.
	set "AUDEVOPENAL=Win64"
	rem The SDL binaries subfolder.
	set "AUDEVSDL=x64"
	rem The Steam library (both .lib and .dll), relative to the valve folder.
	set "AUDEVSTEAM=redistributable_bin\win64\steam_api64"
	rem This is where all the Au source code goes.
	set "AUDIR=%SEIDEV%\au"
	rem For larger than 2Gb memory addressing on Win32.
	set "AULARGEADDRESSAWARE="
) else if "%AUARCH%" == "soh-win32" (
	rem Configure for 32-bit Windows build.
	set "AUARCHSTEAM=hss-win32"
	rem The OpenAL binaries subfolder.
	set "AUDEVOPENAL=Win32"
	rem The SDL binaries subfolder.
	set "AUDEVSDL=Win32"
	rem The Steam library (both .lib and .dll), relative to the valve folder.
	set "AUDEVSTEAM=redistributable_bin\steam_api"
	rem This is where all the Au source code goes.
	set "AUDIR=%SEIDEV%\au"
	rem For larger than 2Gb memory addressing on Win32.
	set "AULARGEADDRESSAWARE=/LARGEADDRESSAWARE"
) else (
	echo Error: AUARCH not defined. Must be soh-win64.
	exit /B
)
