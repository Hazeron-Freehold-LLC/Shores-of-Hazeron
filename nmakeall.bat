@echo off
echo Au nmakeall.bat Started
time /t

rem Configure environment variables.
call auenv.bat
if "%AUARCH%" == "" (
	exit /B
)

rem Build base level libraries.
echo ==================== auutil ====================
cd auutil
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumath ====================
cd ..\aumath
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== augl ====================
cd ..\augl
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auglobal ====================
cd ..\auglobal
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== audata ====================
cd ..\audata
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auloginclient ====================
cd ..\auloginclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumail ====================
cd ..\aumail
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatom ====================
cd ..\auatom
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auclient ====================
cd ..\auclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== audesigner ====================
cd ..\audesigner
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

echo ==================== austoryres ====================
cd ..\austoryres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build client resources libraries, which is a bunch of libs linked into a big dll.
echo ==================== auclientmailres ====================
cd ..\auclientmailres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auclientres1 ====================
cd ..\auclientres1
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auclientres ====================
cd ..\auclientres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build atom resources library, which is a bunch of libs linked into a big dll.
echo ==================== auatomr1 ====================
cd ..\auatomr1
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomr2 ====================
cd ..\auatomr2
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomr3 ====================
cd ..\auatomr3
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomr4 ====================
cd ..\auatomr4
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomr5 ====================
cd ..\auatomr5
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomr6 ====================
cd ..\auatomr6
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auatomres ====================
cd ..\auatomres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build body parts resources library, which is a bunch of libs linked into a big dll.
echo ==================== aubodyparts0 ====================
cd ..\aubodyparts0
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts1 ====================
cd ..\aubodyparts1
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts2 ====================
cd ..\aubodyparts2
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts3 ====================
cd ..\aubodyparts3
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts4 ====================
cd ..\aubodyparts4
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts5 ====================
cd ..\aubodyparts5
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts6 ====================
cd ..\aubodyparts6
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodyparts7 ====================
cd ..\aubodyparts7
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aupowerarmor ====================
cd ..\aupowerarmor
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aubodypartsres ====================
cd ..\aubodypartsres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build model resources library, which is a bunch of libs linked into a big dll.
echo ==================== aumodel1 ====================
cd ..\aumodel1
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel2 ====================
cd ..\aumodel2
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel3 ====================
cd ..\aumodel3
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel4 ====================
cd ..\aumodel4
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel5 ====================
cd ..\aumodel5
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel6 ====================
cd ..\aumodel6
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel7 ====================
cd ..\aumodel7
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel8 ====================
cd ..\aumodel8
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel9 ====================
cd ..\aumodel9
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodel10 ====================
cd ..\aumodel10
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumodelres ====================
cd ..\aumodelres
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build texture resources library, which is a bunch of libs linked into a big dll.
echo ==================== ausctex1 ====================
cd ..\ausctex1
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== ausctex2 ====================
cd ..\ausctex2
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== ausctex3 ====================
cd ..\ausctex3
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== ausctex4 ====================
cd ..\ausctex4
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== ausctex5 ====================
cd ..\ausctex5
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aushiptex0 ====================
cd ..\aushiptex0
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== autexture ====================
cd ..\autexture
call reqtr.bat
nmake /NOLOGO /f Makefile.Release

rem Build client programs.
echo ==================== ausceneclient ====================
cd ..\ausceneclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== auchatclient ====================
cd ..\auchatclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aulauncher ====================
cd ..\aulauncher
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== aumailclient ====================
cd ..\aumailclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
echo ==================== austeamclient ====================
cd ..\austeamclient
call reqtr.bat
nmake /NOLOGO /f Makefile.Release
rem echo ==================== auscript ====================
rem cd ..\auscript
rem call reqtr.bat
rem nmake /NOLOGO /f Makefile.Release
cd ..

rem Build launcher distribution.
echo Copying launcher output to ShoresOfHazeron folder.
mkdir ..\ShoresOfHazeron
echo aulauncher.exe
copy /B /Y release\aulauncher.exe "..\ShoresOfHazeron\Hazeron Launcher.exe"
echo %AUARCH% > ..\ShoresOfHazeron\data.bin
cd ..\ShoresOfHazeron
windeployqt "Hazeron Launcher.exe"

rem Build game package files for download by launcher.
cd ..\au
echo Copying package output to %AUARCH% folder.
mkdir ..\%AUARCH%
mkdir ..\%AUARCH%\tex
mkdir ..\%AUARCH%\Blueprints
mkdir ..\%AUARCH%\Blueprints\Buildings
mkdir ..\%AUARCH%\Blueprints\Spacecraft
echo Building Blueprints
copy /B /Y Blueprints\Buildings\*.SoH ..\%AUARCH%\Blueprints\Buildings
echo Spacecraft Blueprints
copy /B /Y Blueprints\Spacecraft\*.SoH ..\%AUARCH%\Blueprints\Spacecraft
echo sfx
copy /B /Y auglobal\sfx\sfx.har ..\%AUARCH%
echo tex
copy /B /Y autexture\data\*.png ..\%AUARCH%\tex
echo SDL2.dll
copy /B /Y "%SEIDEV%\SDL2-2.0.3\VisualC\SDL\%AUDEVSDL%\Release\SDL2.dll" ..\%AUARCH%
echo GD
copy /B /Y GD ..\%AUARCH%
echo aumailclient.exe
copy /B /Y release\aumailclient.exe "..\%AUARCH%\Hazeron Mail.exe"
echo ausceneclient.exe
copy /B /Y release\ausceneclient.exe "..\%AUARCH%\Shores of Hazeron.exe"
echo auchatclient.exe
copy /B /Y release\auchatclient.exe "..\%AUARCH%\Hazeron Commander.exe"
rem copy /B /Y release\auscript.exe ..\%AUARCH%\AuScript.exe
rem copy /B /Y auscript\AuScript.bin ..\%AUARCH%\AuScript.bin
rem copy /B /Y auscript\AuScript.txt ..\%AUARCH%\AuScript.txt
echo auatomres7.dll
copy /B /Y release\auatomres7.dll ..\%AUARCH%\AuAtomRes7.dll
echo aubodypartsres7.dll
copy /B /Y release\aubodypartsres7.dll ..\%AUARCH%\AuBodyPartsRes7.dll
echo auclientmailres7.dll
copy /B /Y release\auclientmailres7.dll ..\%AUARCH%\AuClientMailRes7.dll
echo auclientres7.dll
copy /B /Y release\auclientres7.dll ..\%AUARCH%\AuClientRes7.dll
echo aumodelres7.dll
copy /B /Y release\aumodelres7.dll ..\%AUARCH%\AuModelRes7.dll
echo austoryres7.dll
copy /B /Y release\austoryres7.dll ..\%AUARCH%\AuStoryRes7.dll
echo autexture7.dll
copy /B /Y release\autexture7.dll ..\%AUARCH%\AuTexture7.dll
cd ..\%AUARCH%
windeployqt --sql "Shores of Hazeron.exe"
windeployqt "Hazeron Mail.exe"
windeployqt --sql "Hazeron Commander.exe"

rem Build game package files for Steam.
cd ..\au
echo Copying package output to %AUARCHSTEAM% folder.
mkdir ..\%AUARCHSTEAM%
mkdir ..\%AUARCHSTEAM%\tex
mkdir ..\%AUARCHSTEAM%\Blueprints
mkdir ..\%AUARCHSTEAM%\Blueprints\Buildings
mkdir ..\%AUARCHSTEAM%\Blueprints\Spacecraft
echo Building Blueprints
copy /B /Y Blueprints\Buildings\*.SoH ..\%AUARCHSTEAM%\Blueprints\Buildings
echo Spacecraft Blueprints
copy /B /Y Blueprints\Spacecraft\*.SoH ..\%AUARCHSTEAM%\Blueprints\Spacecraft
echo sfx
copy /B /Y auglobal\sfx\sfx.har ..\%AUARCHSTEAM%
echo tex
copy /B /Y autexture\data\*.png ..\%AUARCHSTEAM%\tex
echo SDL2.dll
copy /B /Y "%SEIDEV%\SDL2-2.0.3\VisualC\SDL\%AUDEVSDL%\Release\SDL2.dll" ..\%AUARCHSTEAM%
echo Steam API DLL
copy /B /Y "%SEIDEV%\valve\%AUDEVSTEAM%.dll" ..\%AUARCHSTEAM%
echo GD
copy /B /Y GD ..\%AUARCHSTEAM%
echo GS
copy /B /Y GS ..\%AUARCHSTEAM%
rem echo aumailclient.exe
rem copy /B /Y release\aumailclient.exe "..\%AUARCHSTEAM%\Hazeron Mail.exe"
echo austeamclient.exe
copy /B /Y release\austeamclient.exe "..\%AUARCHSTEAM%\Hazeron Starship.exe"
rem echo auchatclient.exe
rem copy /B /Y release\auchatclient.exe "..\%AUARCHSTEAM%\Hazeron Commander.exe"
rem copy /B /Y release\auscript.exe ..\%AUARCHSTEAM%\AuScript.exe
rem copy /B /Y auscript\AuScript.bin ..\%AUARCHSTEAM%\AuScript.bin
rem copy /B /Y auscript\AuScript.txt ..\%AUARCHSTEAM%\AuScript.txt
echo auatomres7.dll
copy /B /Y release\auatomres7.dll ..\%AUARCHSTEAM%\AuAtomRes7.dll
echo aubodypartsres7.dll
copy /B /Y release\aubodypartsres7.dll ..\%AUARCHSTEAM%\AuBodyPartsRes7.dll
echo auclientmailres7.dll
copy /B /Y release\auclientmailres7.dll ..\%AUARCHSTEAM%\AuClientMailRes7.dll
echo auclientres7.dll
copy /B /Y release\auclientres7.dll ..\%AUARCHSTEAM%\AuClientRes7.dll
echo aumodelres7.dll
copy /B /Y release\aumodelres7.dll ..\%AUARCHSTEAM%\AuModelRes7.dll
echo austoryres7.dll
copy /B /Y release\austoryres7.dll ..\%AUARCHSTEAM%\AuStoryRes7.dll
echo autexture7.dll
copy /B /Y release\autexture7.dll ..\%AUARCHSTEAM%\AuTexture7.dll
cd ..\%AUARCHSTEAM%
windeployqt --sql "Hazeron Starship.exe"

rem Wave goodbye.
cd ..\au
echo Au nmakeall.bat Done
time /t
