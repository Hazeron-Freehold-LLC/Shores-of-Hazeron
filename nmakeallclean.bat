@echo off
echo Au nmakeallclean.bat Started
time /t
echo ==================== aumath ====================
cd aumath
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auutil ====================
cd auutil
nmake /NOLOGO /f Makefile.Release clean
echo ==================== augl ====================
cd ..\augl
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auglobal ====================
cd ..\auglobal
nmake /NOLOGO /f Makefile.Release clean
echo ==================== audata ====================
cd ..\audata
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auloginclient ====================
cd ..\auloginclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumail ====================
cd ..\aumail
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumailclient ====================
cd ..\aumailclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatom ====================
cd ..\auatom
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auclientres1 ====================
cd ..\auclientres1
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auclientres ====================
cd ..\auclientres
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auclientmailres ====================
cd ..\auclientmailres
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auclient ====================
cd ..\auclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== audesigner ====================
cd ..\audesigner
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr1 ====================
cd ..\auatomr1
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr2 ====================
cd ..\auatomr2
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr3 ====================
cd ..\auatomr3
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr4 ====================
cd ..\auatomr4
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr5 ====================
cd ..\auatomr5
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomr6 ====================
cd ..\auatomr6
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auatomres ====================
cd ..\auatomres
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel1 ====================
cd ..\aumodel1
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel2 ====================
cd ..\aumodel2
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel3 ====================
cd ..\aumodel3
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel4 ====================
cd ..\aumodel4
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel5 ====================
cd ..\aumodel5
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel6 ====================
cd ..\aumodel6
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel7 ====================
cd ..\aumodel7
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel8 ====================
cd ..\aumodel8
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel9 ====================
cd ..\aumodel9
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodel10 ====================
cd ..\aumodel10
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aumodelres ====================
cd ..\aumodelres
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts0 ====================
cd ..\aubodyparts0
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts1 ====================
cd ..\aubodyparts1
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts2 ====================
cd ..\aubodyparts2
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts3 ====================
cd ..\aubodyparts3
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts4 ====================
cd ..\aubodyparts4
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts5 ====================
cd ..\aubodyparts5
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts6 ====================
cd ..\aubodyparts6
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodyparts7 ====================
cd ..\aubodyparts7
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aubodypartsres ====================
cd ..\aubodypartsres
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aupowerarmor ====================
cd ..\aupowerarmor
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aushiptex0 ====================
cd ..\aushiptex0
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausctex1 ====================
cd ..\ausctex1
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausctex2 ====================
cd ..\ausctex2
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausctex3 ====================
cd ..\ausctex3
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausctex4 ====================
cd ..\ausctex4
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausctex5 ====================
cd ..\ausctex5
nmake /NOLOGO /f Makefile.Release clean
echo ==================== aulauncher ====================
cd ..\aulauncher
nmake /NOLOGO /f Makefile.Release clean
echo ==================== ausceneclient ====================
cd ..\ausceneclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== austeamclient ====================
cd ..\austeamclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auchatclient ====================
cd ..\auchatclient
nmake /NOLOGO /f Makefile.Release clean
echo ==================== auscript ====================
cd ..\auscript
nmake /NOLOGO /f Makefile.Release clean
echo ==================== release ====================
cd ..\Release
del *.lib
del *.exe
cd ..
echo Au nmakeallclean.bat Done
time /t
