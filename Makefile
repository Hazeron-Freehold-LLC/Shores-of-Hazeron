# Identify target names that are not to be confused with files or folders with the same name.
.PHONY: all client servers cgi relink report reqt clean auacctemail auacctexpire auacctnew auacctpassword auacctsendpassword auloginserver aumailclient aumailserver aulauncher ausceneclient ausceneserver auscript auserverstatus auservermon auchatclient

all: client servers cgi report auviewer

# Native macOS scene-client build; existing platform targets are unchanged.
.PHONY: mac-client mac-package
MAC_JOBS ?= 4
mac-client:
	python3 tools/build-macos.py --jobs $(MAC_JOBS)

mac-package:
	python3 tools/package-macos.py

client: ausceneclient aulauncher aumailclient auservermon auscript auchatclient

servers: ausceneserver auloginserver aumailserver report

cgi: auacctemail auacctexpire auacctnew auacctpassword auacctsendpassword auserverstatus

report: auempirereport

story: auscript

auglver:
	@if [ "`pwd | grep gl41`" != "" ] ; then \
		echo '#define AUGL41' > augl/.d/AuGLVer.h ; \
	else \
		echo '#define AUGL33' > augl/.d/AuGLVer.h ; \
	fi ;

rebuild:
	# Removing a few libraries and executables so they have to be archived or linked over again.
	rm -f auchatclient/AuChatClient_debug
	rm -f ausceneclient/AuSceneClient_debug
	rm -f ausceneserver/AuSceneServer_debug
	rm -f auservermon/AuServerMon_debug
	rm -f auviewer/AuViewer_debug
	rm -f debug/libAuAtom*
	rm -f debug/libAuClient*
	rm -f debug/libAuData*
	rm -f debug/libAuDesigner*
	rm -f debug/libAuGL*
	rm -f debug/libAuGlobal*
	rm -f debug/libAuLoginClient*
	rm -f debug/libAuMail*
	rm -f debug/libAuMath*
	rm -f debug/libAuUtil*

relink: rebuild
	# Removing libraries and executables so they have to be archived or linked over again.
	rm -f auacctemail/AuAcctEmail*
	rm -f auacctexpire/AuAcctExpire*
	rm -f auacctnew/AuAcctNew*
	rm -f auacctpassword/AuAcctPassword*
	rm -f auacctsendpassword/AuAcctSendPassword*
	rm -f auchatclient/AuChatClient_debug
	rm -f auempirereport/AuEmpireReport_debug
	rm -f auloginserver/AuLoginServer_debug
	rm -f aumailclient/AuMailClient_debug
	rm -f aumailserver/AuMailServer_debug
	rm -f aulauncher/AuLauncher_debug
	rm -f auscript/AuScript_debug
	rm -f auserverstatus/AuServerStatus_debug
	rm -f debug/libAuAtomRes*
	rm -f debug/libAuBodyParts*
	rm -f debug/libAuClient*
	rm -f debug/libAuModel*
	rm -f debug/libAuPowerArmor*
	rm -f debug/libAuShipTex0*
	rm -f debug/libAuScTex1*
	rm -f debug/libAuScTex2*
	rm -f debug/libAuScTex3*
	rm -f debug/libAuScTex4*
	rm -f debug/libAuScTex5*
	rm -f debug/libAuStoryRes*
	rm -f debug/libAuTexture*

reqt:
	# Recreating all project Makefiles whether they need it or not.
	@reqt debug auacctemail ; \
	reqt debug auacctexpire ; \
	reqt debug auacctnew ; \
	reqt debug auacctpassword ; \
	reqt debug auacctsendpassword ; \
	reqt debug auatom ; \
	reqt debug auatomres ; \
	reqt debug auatomr1 ; \
	reqt debug auatomr2 ; \
	reqt debug auatomr3 ; \
	reqt debug auatomr4 ; \
	reqt debug auatomr5 ; \
	reqt debug auatomr6 ; \
	reqt debug aubodyparts0 ; \
	reqt debug aubodyparts1 ; \
	reqt debug aubodyparts2 ; \
	reqt debug aubodyparts3 ; \
	reqt debug aubodyparts4 ; \
	reqt debug aubodyparts5 ; \
	reqt debug aubodyparts6 ; \
	reqt debug aubodyparts7 ; \
	reqt debug aubodypartsres ; \
	reqt debug auchatclient ; \
	reqt debug auclient ; \
	reqt debug auclientmailres ; \
	reqt debug auclientres ; \
	reqt debug auclientres1 ; \
	reqt debug audata ; \
	reqt debug audesigner ; \
	reqt debug auempirereport ; \
	reqt debug augl ; \
	reqt debug auglobal ; \
	reqt debug aulauncher ; \
	reqt debug auloginclient ; \
	reqt debug auloginserver ; \
	reqt debug aumail ; \
	reqt debug aumailclient ; \
	reqt debug aumailserver ; \
	reqt debug aumath ; \
	reqt debug aumodel1 ; \
	reqt debug aumodel2 ; \
	reqt debug aumodel3 ; \
	reqt debug aumodel4 ; \
	reqt debug aumodel5 ; \
	reqt debug aumodel6 ; \
	reqt debug aumodel7 ; \
	reqt debug aumodel8 ; \
	reqt debug aumodel9 ; \
	reqt debug aumodel10 ; \
	reqt debug aumodelres ; \
	reqt debug aupowerarmor ; \
	reqt debug ausceneclient ; \
	reqt debug ausceneserver ; \
	reqt debug auscript ; \
	reqt debug ausctex1 ; \
	reqt debug ausctex2 ; \
	reqt debug ausctex3 ; \
	reqt debug ausctex4 ; \
	reqt debug ausctex5 ; \
	reqt debug auservermon ; \
	reqt debug auserverstatus ; \
	reqt debug aushiptex0 ; \
	reqt debug austoryres ; \
	reqt debug autexture ; \
	reqt debug auutil ; \
	reqt debug auviewer ;

reqtcgi:
	# Recreating all project Makefiles needed to build cgi, whether they need it or not.
	@reqt debug auacctemail ; \
	reqt debug auacctexpire ; \
	reqt debug auacctnew ; \
	reqt debug auacctpassword ; \
	reqt debug auacctsendpassword ; \
	reqt debug auserverstatus ; \
	reqt debug auutil ;

reqtservers:
	# Recreating all project Makefiles needed to build servers, whether they need it or not.
	@reqt debug auatom ; \
	reqt debug auatomr1 ; \
	reqt debug auatomr2 ; \
	reqt debug auatomr3 ; \
	reqt debug auatomr4 ; \
	reqt debug auatomr5 ; \
	reqt debug auatomr6 ; \
	reqt debug auatomres ; \
	reqt debug aubodyparts0 ; \
	reqt debug aubodyparts1 ; \
	reqt debug aubodyparts2 ; \
	reqt debug aubodyparts3 ; \
	reqt debug aubodyparts4 ; \
	reqt debug aubodyparts5 ; \
	reqt debug aubodyparts6 ; \
	reqt debug aubodyparts7 ; \
	reqt debug aubodypartsres ; \
	reqt debug audata ; \
	reqt debug auempirereport ; \
	reqt debug augl ; \
	reqt debug auglobal ; \
	reqt debug auloginclient ; \
	reqt debug auloginserver ; \
	reqt debug aumail ; \
	reqt debug aumailserver ; \
	reqt debug aumath ; \
	reqt debug aumodel1 ; \
	reqt debug aumodel10 ; \
	reqt debug aumodel2 ; \
	reqt debug aumodel3 ; \
	reqt debug aumodel4 ; \
	reqt debug aumodel5 ; \
	reqt debug aumodel6 ; \
	reqt debug aumodel7 ; \
	reqt debug aumodel8 ; \
	reqt debug aumodel9 ; \
	reqt debug aumodelres ; \
	reqt debug aupowerarmor ; \
	reqt debug ausceneserver ; \
	reqt debug austoryres ; \
	reqt debug auutil ;

clean: relink
	# Cleaning all projects.
	$(MAKE) -C auacctemail clean
	$(MAKE) -C auacctexpire clean
	$(MAKE) -C auacctnew clean
	$(MAKE) -C auacctpassword clean
	$(MAKE) -C auacctsendpassword clean
	$(MAKE) -C auatom clean
	$(MAKE) -C auatomres clean
	$(MAKE) -C auatomr1 clean
	$(MAKE) -C auatomr2 clean
	$(MAKE) -C auatomr3 clean
	$(MAKE) -C auatomr4 clean
	$(MAKE) -C auatomr5 clean
	$(MAKE) -C auatomr6 clean
	$(MAKE) -C aubodyparts0 clean
	$(MAKE) -C aubodyparts1 clean
	$(MAKE) -C aubodyparts2 clean
	$(MAKE) -C aubodyparts3 clean
	$(MAKE) -C aubodyparts4 clean
	$(MAKE) -C aubodyparts5 clean
	$(MAKE) -C aubodyparts6 clean
	$(MAKE) -C aubodyparts7 clean
	$(MAKE) -C aubodypartsres clean
	$(MAKE) -C auchatclient clean
	$(MAKE) -C auclient clean
	$(MAKE) -C auclientmailres clean
	$(MAKE) -C auclientres clean
	$(MAKE) -C auclientres1 clean
	$(MAKE) -C audata clean
	$(MAKE) -C audesigner clean
	$(MAKE) -C auempirereport clean
	$(MAKE) -C augl clean
	$(MAKE) -C auglobal clean
	$(MAKE) -C aulauncher clean
	$(MAKE) -C auloginclient clean
	$(MAKE) -C auloginserver clean
	$(MAKE) -C aumail clean
	$(MAKE) -C aumailclient clean
	$(MAKE) -C aumailserver clean
	$(MAKE) -C aumath clean
	$(MAKE) -C aumodel1 clean
	$(MAKE) -C aumodel2 clean
	$(MAKE) -C aumodel3 clean
	$(MAKE) -C aumodel4 clean
	$(MAKE) -C aumodel5 clean
	$(MAKE) -C aumodel6 clean
	$(MAKE) -C aumodel7 clean
	$(MAKE) -C aumodel8 clean
	$(MAKE) -C aumodel9 clean
	$(MAKE) -C aumodel10 clean
	$(MAKE) -C aumodelres clean
	$(MAKE) -C aupowerarmor clean
	$(MAKE) -C ausceneclient clean
	$(MAKE) -C ausceneserver clean
	$(MAKE) -C auscript clean
	$(MAKE) -C ausctex1 clean
	$(MAKE) -C ausctex2 clean
	$(MAKE) -C ausctex3 clean
	$(MAKE) -C ausctex4 clean
	$(MAKE) -C ausctex5 clean
	$(MAKE) -C auservermon clean
	$(MAKE) -C auserverstatus clean
	$(MAKE) -C aushiptex0 clean
	$(MAKE) -C austoryres clean
	$(MAKE) -C autexture clean
	$(MAKE) -C auutil clean
	$(MAKE) -C auviewer clean

cleanservers: relink
	# Cleaning all projects.
	$(MAKE) -C auatom clean
	$(MAKE) -C auatomr1 clean
	$(MAKE) -C auatomr2 clean
	$(MAKE) -C auatomr3 clean
	$(MAKE) -C auatomr4 clean
	$(MAKE) -C auatomr5 clean
	$(MAKE) -C auatomr6 clean
	$(MAKE) -C auatomres clean
	$(MAKE) -C aubodyparts0 clean
	$(MAKE) -C aubodyparts1 clean
	$(MAKE) -C aubodyparts2 clean
	$(MAKE) -C aubodyparts3 clean
	$(MAKE) -C aubodyparts4 clean
	$(MAKE) -C aubodyparts5 clean
	$(MAKE) -C aubodyparts6 clean
	$(MAKE) -C aubodyparts7 clean
	$(MAKE) -C aubodypartsres clean
	$(MAKE) -C audata clean
	$(MAKE) -C auempirereport clean
	$(MAKE) -C augl clean
	$(MAKE) -C auglobal clean
	$(MAKE) -C auloginclient clean
	$(MAKE) -C auloginserver clean
	$(MAKE) -C aumail clean
	$(MAKE) -C aumailserver clean
	$(MAKE) -C aumath clean
	$(MAKE) -C aumodel1 clean
	$(MAKE) -C aumodel10 clean
	$(MAKE) -C aumodel2 clean
	$(MAKE) -C aumodel3 clean
	$(MAKE) -C aumodel4 clean
	$(MAKE) -C aumodel5 clean
	$(MAKE) -C aumodel6 clean
	$(MAKE) -C aumodel7 clean
	$(MAKE) -C aumodel8 clean
	$(MAKE) -C aumodel9 clean
	$(MAKE) -C aumodelres clean
	$(MAKE) -C aupowerarmor clean
	$(MAKE) -C ausceneserver clean
	$(MAKE) -C austoryres clean
	$(MAKE) -C auutil clean

cleancode: relink
	# Cleaning only those projects that have the primary code in them. Omits cgis, models and body parts data.
	$(MAKE) -C auatom clean
	$(MAKE) -C auchatclient clean
	$(MAKE) -C audata clean
	$(MAKE) -C auempirereport clean
	$(MAKE) -C augl clean
	$(MAKE) -C auglobal clean
	$(MAKE) -C aulauncher clean
	$(MAKE) -C auloginclient clean
	$(MAKE) -C auloginserver clean
	$(MAKE) -C aumail clean
	$(MAKE) -C aumailclient clean
	$(MAKE) -C aumailserver clean
	$(MAKE) -C ausceneclient clean
	$(MAKE) -C ausceneserver clean
	$(MAKE) -C auscript clean
	$(MAKE) -C auserverstatus clean
	$(MAKE) -C auutil clean
	$(MAKE) -C auviewer clean

cleanservercode: relink
	# Cleaning only those projects that have the primary code in them. Omits cgis, models and body parts data.
	$(MAKE) -C auatom clean
	$(MAKE) -C audata clean
	$(MAKE) -C auempirereport clean
	$(MAKE) -C augl clean
	$(MAKE) -C auglobal clean
	$(MAKE) -C auloginclient clean
	$(MAKE) -C auloginserver clean
	$(MAKE) -C aumail clean
	$(MAKE) -C aumailserver clean
	$(MAKE) -C ausceneserver clean
	$(MAKE) -C auserverstatus clean
	$(MAKE) -C auutil clean

auacctemail: auacctemail/AuAcctEmail

auacctexpire: auacctexpire/AuAcctExpire

auacctnew: auacctnew/AuAcctNew

auacctpassword: auacctpassword/AuAcctPassword

auacctsendpassword: auacctsendpassword/AuAcctSendPassword

auempirereport: auempirereport/AuEmpireReport_debug

auloginserver: auloginserver/AuLoginServer_debug

aumailclient: aumailclient/AuMailClient_debug

aumailserver: aumailserver/AuMailServer_debug

aulauncher: aulauncher/AuLauncher_debug

ausceneclient: ausceneclient/AuSceneClient_debug

auviewer: auviewer/AuViewer_debug

auchatclient: auchatclient/AuChatClient_debug

ausceneserver: ausceneserver/AuSceneServer_debug

auscript: auscript/AuScript_debug

auservermon: auservermon/AuServerMon_debug

auserverstatus: auserverstatus/AuServerStatus_debug

auacctemail/AuAcctEmail: auacctemail/Makefile debug/libAuUtil_debug.a
	# Building auacctemail
	$(MAKE) -C auacctemail

auacctexpire/AuAcctExpire: auacctexpire/Makefile debug/libAuUtil_debug.a
	# Building auacctexpire
	$(MAKE) -C auacctexpire

auacctnew/AuAcctNew: auacctnew/Makefile debug/libAuUtil_debug.a
	# Building auacctnew
	$(MAKE) -C auacctnew

auacctpassword/AuAcctPassword: auacctpassword/Makefile debug/libAuUtil_debug.a
	# Building auacctpassword
	$(MAKE) -C auacctpassword

auacctsendpassword/AuAcctSendPassword: auacctsendpassword/Makefile debug/libAuUtil_debug.a
	# Building auacctsendpassword
	$(MAKE) -C auacctsendpassword

auempirereport/AuEmpireReport_debug: auempirereport/Makefile debug/libAuAtom_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuLoginClient_debug.a debug/libAuMail_debug.a debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuGL_debug.a
	# Building auempirereport
	$(MAKE) -C auempirereport

debug/libAuAtom_debug.a: auatom/Makefile debug/libAuUtil_debug.a debug/libAuMath_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a debug/libAuData_debug.a debug/libAuStoryRes_debug.so.7
	# Building auatom
	$(MAKE) -C auatom

debug/libAuAtomR1_debug.a: auatomr1/Makefile
	# Building auatomr1
	$(MAKE) -C auatomr1

debug/libAuAtomR2_debug.a: auatomr2/Makefile
	# Building auatomr2
	$(MAKE) -C auatomr2

debug/libAuAtomR3_debug.a: auatomr3/Makefile
	# Building auatomr3
	$(MAKE) -C auatomr3

debug/libAuAtomR4_debug.a: auatomr4/Makefile
	# Building auatomr4
	$(MAKE) -C auatomr4

debug/libAuAtomR5_debug.a: auatomr5/Makefile
	# Building auatomr5
	$(MAKE) -C auatomr5

debug/libAuAtomR6_debug.a: auatomr6/Makefile
	# Building auatomr6
	$(MAKE) -C auatomr6

debug/libAuAtomRes_debug.so.7: auatomres/Makefile debug/libAuAtomR1_debug.a debug/libAuAtomR2_debug.a debug/libAuAtomR3_debug.a debug/libAuAtomR4_debug.a debug/libAuAtomR5_debug.a debug/libAuAtomR6_debug.a
	# Building auatomres
	$(MAKE) -C auatomres

debug/libAuClient_debug.a: auclient/Makefile
	# Building auclient
	$(MAKE) -C auclient

debug/libAuDesigner_debug.a: audesigner/Makefile debug/libAuClient_debug.a
	# Building audesigner
	$(MAKE) -C audesigner

debug/libAuClientRes_debug.so.7: auclientres/Makefile debug/libAuClientRes1_debug.a
	# Building auclientres
	$(MAKE) -C auclientres

debug/libAuClientRes1_debug.a: auclientres1/Makefile
	# Building auclientres1
	$(MAKE) -C auclientres1

debug/libAuClientMailRes_debug.so.7: auclientmailres/Makefile
	# Building auclientmailres
	$(MAKE) -C auclientmailres

debug/libAuStoryRes_debug.so.7:
	# Building austoryres
	$(MAKE) -C austoryres

debug/libAuTexture_debug.so.7: autexture/Makefile debug/libAuShipTex0_debug.a debug/libAuScTex1_debug.a debug/libAuScTex2_debug.a debug/libAuScTex3_debug.a debug/libAuScTex4_debug.a debug/libAuScTex5_debug.a
	# Building autexture
	$(MAKE) -C autexture

debug/libAuData_debug.a: audata/Makefile debug/libAuUtil_debug.a debug/libAuMath_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a
	# Building audata
	$(MAKE) -C audata

debug/libAuGlobal_debug.a: auglobal/Makefile debug/libAuUtil_debug.a debug/libAuMath_debug.a debug/libAuGL_debug.a sfx
	# Building auglobal
	$(MAKE) -C auglobal

augl/.d/AuGLVer.h:
	@if [ "`uname -n | grep Capella`" != "" ] || [ "`uname -n | grep MacBook`" != "" ] ; then \
		if [ "`pwd | grep gl41`" != "" ] ; then \
			echo '#define AUGL41' > augl/.d/AuGLVer.h ; \
		else \
			echo '#define AUGL33' > augl/.d/AuGLVer.h ; \
		fi ; \
	else \
		echo '#define AUGL32' > augl/.d/AuGLVer.h ; \
	fi ;

debug/libAuGL_debug.a: augl/Makefile augl/.d/AuGLVer.h
	# Building augl
	$(MAKE) -C augl

debug/libAuMath_debug.a: aumath/Makefile
	# Building aumath
	$(MAKE) -C aumath

debug/libAuUtil_debug.a: auutil/Makefile
	# Building auutil
	$(MAKE) -C auutil

debug/libAuLoginClient_debug.a: auloginclient/Makefile debug/libAuUtil_debug.a debug/libAuMath_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a
	# Building auloginclient
	$(MAKE) -C auloginclient

auloginserver/AuLoginServer_debug: auloginserver/Makefile debug/libAuData_debug.a debug/libAuUtil_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a debug/libAuMath_debug.a 
	# Building auloginserver
	$(MAKE) -C auloginserver

debug/libAuMail_debug.a: aumail/Makefile debug/libAuUtil_debug.a debug/libAuMath_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a debug/libAuLoginClient_debug.a 
	# Building aumail
	$(MAKE) -C aumail

aumailclient/AuMailClient_debug: aumailclient/Makefile debug/libAuUtil_debug.a debug/libAuData_debug.a debug/libAuGL_debug.a debug/libAuMath_debug.a debug/libAuLoginClient_debug.a debug/libAuGlobal_debug.a debug/libAuMail_debug.a debug/libAuClientMailRes_debug.so.7
	# Building aumailclient
	$(MAKE) -C aumailclient

aumailserver/AuMailServer_debug: aumailserver/Makefile debug/libAuUtil_debug.a debug/libAuGlobal_debug.a debug/libAuGL_debug.a debug/libAuMath_debug.a 
	# Building aumailserver
	$(MAKE) -C aumailserver

auservermon/AuServerMon_debug: auservermon/Makefile debug/libAuAtom_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuLoginClient_debug.a debug/libAuMail_debug.a debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuGL_debug.a debug/libAuClient_debug.a debug/libAuClientRes_debug.so.7 debug/libAuClientMailRes_debug.so.7
	# Building auservermon
	$(MAKE) -C auservermon

debug/libAuShipTex0_debug.a: aushiptex0/Makefile
	# Building aushiptex0
	$(MAKE) -C aushiptex0

debug/libAuScTex1_debug.a: ausctex1/Makefile
	# Building ausctex1
	$(MAKE) -C ausctex1

debug/libAuScTex2_debug.a: ausctex2/Makefile
	# Building ausctex2
	$(MAKE) -C ausctex2

debug/libAuScTex3_debug.a: ausctex3/Makefile
	# Building ausctex3
	$(MAKE) -C ausctex3

debug/libAuScTex4_debug.a: ausctex4/Makefile
	# Building ausctex4
	$(MAKE) -C ausctex4

debug/libAuScTex5_debug.a: ausctex5/Makefile
	# Building ausctex5
	$(MAKE) -C ausctex5

debug/libAuModelRes_debug.so.7: aumodelres/Makefile debug/libAuModel1_debug.a debug/libAuModel2_debug.a debug/libAuModel3_debug.a debug/libAuModel4_debug.a debug/libAuModel5_debug.a debug/libAuModel6_debug.a debug/libAuModel7_debug.a debug/libAuModel8_debug.a debug/libAuModel9_debug.a debug/libAuModel10_debug.a
	# Building aumodelres
	$(MAKE) -C aumodelres

debug/libAuModel1_debug.a: aumodel1/Makefile
	# Building aumodel
	$(MAKE) -C aumodel1

debug/libAuModel2_debug.a: aumodel2/Makefile
	# Building aumodel2
	$(MAKE) -C aumodel2

debug/libAuModel3_debug.a: aumodel3/Makefile
	# Building aumodel3
	$(MAKE) -C aumodel3

debug/libAuModel4_debug.a: aumodel4/Makefile
	# Building aumodel4
	$(MAKE) -C aumodel4

debug/libAuModel5_debug.a: aumodel5/Makefile
	# Building aumodel5
	$(MAKE) -C aumodel5

debug/libAuModel6_debug.a: aumodel6/Makefile
	# Building aumodel6
	$(MAKE) -C aumodel6

debug/libAuModel7_debug.a: aumodel7/Makefile
	# Building aumodel7
	$(MAKE) -C aumodel7

debug/libAuModel8_debug.a: aumodel8/Makefile
	# Building aumodel8
	$(MAKE) -C aumodel8

debug/libAuModel9_debug.a: aumodel9/Makefile
	# Building aumodel9
	$(MAKE) -C aumodel9

debug/libAuModel10_debug.a: aumodel10/Makefile
	# Building aumodel10
	$(MAKE) -C aumodel10

debug/libAuBodyPartsRes_debug.so.7: aubodypartsres/Makefile debug/libAuBodyParts0_debug.a debug/libAuBodyParts1_debug.a debug/libAuBodyParts2_debug.a debug/libAuBodyParts3_debug.a debug/libAuBodyParts4_debug.a debug/libAuBodyParts5_debug.a debug/libAuBodyParts6_debug.a debug/libAuBodyParts7_debug.a debug/libAuPowerArmor_debug.a
	# Building aubodypartsres
	$(MAKE) -C aubodypartsres

debug/libAuBodyParts0_debug.a: aubodyparts0/Makefile
	# Building aubodyparts0
	$(MAKE) -C aubodyparts0

debug/libAuBodyParts1_debug.a: aubodyparts1/Makefile
	# Building aubodyparts1
	$(MAKE) -C aubodyparts1

debug/libAuBodyParts2_debug.a: aubodyparts2/Makefile
	# Building aubodyparts2
	$(MAKE) -C aubodyparts2

debug/libAuBodyParts3_debug.a: aubodyparts3/Makefile
	# Building aubodyparts3
	$(MAKE) -C aubodyparts3

debug/libAuBodyParts4_debug.a: aubodyparts4/Makefile
	# Building aubodyparts4
	$(MAKE) -C aubodyparts4

debug/libAuBodyParts5_debug.a: aubodyparts5/Makefile
	# Building aubodyparts5
	$(MAKE) -C aubodyparts5

debug/libAuBodyParts6_debug.a: aubodyparts6/Makefile
	# Building aubodyparts6
	$(MAKE) -C aubodyparts6

debug/libAuBodyParts7_debug.a: aubodyparts7/Makefile
	# Building aubodyparts7
	$(MAKE) -C aubodyparts7

debug/libAuPowerArmor_debug.a: aupowerarmor/Makefile
	# Building aupowerarmor
	$(MAKE) -C aupowerarmor

aulauncher/AuLauncher_debug: aulauncher/Makefile debug/libAuUtil_debug.a
	# Building aulauncher
	$(MAKE) -C aulauncher

auviewer/AuViewer_debug: auviewer/Makefile debug/libAuUtil_debug.a debug/libAuGL_debug.a debug/libAuAtom_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuMath_debug.a debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuLoginClient_debug.a debug/libAuMail_debug.a debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuClient_debug.a debug/libAuTexture_debug.so.7 debug/libAuDesigner_debug.a debug/libAuClientRes_debug.so.7 debug/libAuClientMailRes_debug.so.7
	# Building auviewer
	$(MAKE) -C auviewer

ausceneclient/AuSceneClient_debug: ausceneclient/Makefile debug/libAuUtil_debug.a debug/libAuGL_debug.a debug/libAuAtom_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuMath_debug.a debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuLoginClient_debug.a debug/libAuMail_debug.a debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuClient_debug.a debug/libAuTexture_debug.so.7 debug/libAuDesigner_debug.a debug/libAuClientRes_debug.so.7 debug/libAuClientMailRes_debug.so.7 debug/libAuStoryRes_debug.so.7
	# Building ausceneclient
	$(MAKE) -C ausceneclient

auchatclient/AuChatClient_debug: auchatclient/Makefile debug/libAuUtil_debug.a debug/libAuGL_debug.a debug/libAuAtom_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuMath_debug.a debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuLoginClient_debug.a debug/libAuMail_debug.a debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuClient_debug.a debug/libAuTexture_debug.so.7  debug/libAuClientRes_debug.so.7 debug/libAuClientMailRes_debug.so.7
	# Building auchatclient
	$(MAKE) -C auchatclient

ausceneserver/AuSceneServer_debug: ausceneserver/Makefile debug/libAuUtil_debug.a debug/libAuGL_debug.a debug/libAuAtom_debug.a debug/libAuMath_debug.a debug/libAuLoginClient_debug.a debug/libAuData_debug.a debug/libAuGlobal_debug.a debug/libAuMail_debug.a debug/libAuAtomRes_debug.so.7 debug/libAuModelRes_debug.so.7 debug/libAuBodyPartsRes_debug.so.7 debug/libAuStoryRes_debug.so.7
	# Building ausceneserver
	$(MAKE) -C ausceneserver

auscript/AuScript_debug: auscript/Makefile debug/libAuUtil_debug.a debug/libAuAtom_debug.a debug/libAuGL_debug.a debug/libAuMath_debug.a debug/libAuGlobal_debug.a debug/libAuData_debug.a
	# Building auscript
	$(MAKE) -C auscript

auserverstatus/AuServerStatus_debug: auserverstatus/Makefile debug/libAuUtil_debug.a
	# Building auserverstatus
	$(MAKE) -C auserverstatus

auacctemail/Makefile: auacctemail/reqt.opt
	reqt debug auacctemail

auacctexpire/Makefile: auacctexpire/reqt.opt
	reqt debug auacctexpire

auacctnew/Makefile: auacctnew/reqt.opt
	reqt debug auacctnew

auacctpassword/Makefile: auacctpassword/reqt.opt
	reqt debug auacctpassword

auacctsendpassword/Makefile: auacctsendpassword/reqt.opt
	reqt debug auacctsendpassword

auatom/Makefile: auatom/reqt.opt
	reqt debug auatom

auatomr1/Makefile: auatomr1/reqt.opt
	reqt debug auatomr1

auatomr2/Makefile: auatomr2/reqt.opt
	reqt debug auatomr2

auatomr3/Makefile: auatomr3/reqt.opt
	reqt debug auatomr3

auatomr4/Makefile: auatomr4/reqt.opt
	reqt debug auatomr4

auatomr5/Makefile: auatomr5/reqt.opt
	reqt debug auatomr5

auatomr6/Makefile: auatomr6/reqt.opt
	reqt debug auatomr6

auatomres/Makefile: auatomres/reqt.opt
	reqt debug auatomres

auclientres/Makefile: auclientres/reqt.opt
	reqt debug auclientres

auclientres1/Makefile: auclientres1/reqt.opt
	reqt debug auclientres1

auclientmailres/Makefile: auclientmailres/reqt.opt
	reqt debug auclientmailres

auclient/Makefile: auclient/reqt.opt
	reqt debug auclient

audesigner/Makefile: audesigner/reqt.opt
	reqt debug audesigner

austoryres/Makefile: austoryres/reqt.opt
	reqt debug austoryres

autexture/Makefile: autexture/reqt.opt
	reqt debug autexture

audata/Makefile: audata/reqt.opt
	reqt debug audata

auempirereport/Makefile: auempirereport/reqt.opt
	reqt debug auempirereport

augl/Makefile: augl/reqt.opt
	reqt debug augl

auglobal/Makefile: auglobal/reqt.opt
	reqt debug auglobal

auloginclient/Makefile: auloginclient/reqt.opt
	reqt debug auloginclient

auloginserver/Makefile: auloginserver/reqt.opt debug/libAuUtil_debug.a debug/libAuGlobal_debug.a debug/libAuMath_debug.a
	reqt debug auloginserver

aumail/Makefile: aumail/reqt.opt
	reqt debug aumail

aumailclient/Makefile: aumailclient/reqt.opt
	reqt debug aumailclient

aumailserver/Makefile: aumailserver/reqt.opt
	reqt debug aumailserver

aumath/Makefile: aumath/reqt.opt
	reqt debug aumath

auutil/Makefile: auutil/reqt.opt
	reqt debug auutil

auservermon/Makefile: auservermon/reqt.opt
	reqt debug auservermon

aushiptex0/Makefile: aushiptex0/reqt.opt
	reqt debug aushiptex0

ausctex1/Makefile: ausctex1/reqt.opt
	reqt debug ausctex1

ausctex2/Makefile: ausctex2/reqt.opt
	reqt debug ausctex2

ausctex3/Makefile: ausctex3/reqt.opt
	reqt debug ausctex3

ausctex4/Makefile: ausctex4/reqt.opt
	reqt debug ausctex4

ausctex5/Makefile: ausctex5/reqt.opt
	reqt debug ausctex5

aumodelres/Makefile: aumodelres/reqt.opt
	reqt debug aumodelres

aumodel1/Makefile: aumodel1/reqt.opt
	reqt debug aumodel1

aumodel2/Makefile: aumodel2/reqt.opt
	reqt debug aumodel2

aumodel3/Makefile: aumodel3/reqt.opt
	reqt debug aumodel3

aumodel4/Makefile: aumodel4/reqt.opt
	reqt debug aumodel4

aumodel5/Makefile: aumodel5/reqt.opt
	reqt debug aumodel5

aumodel6/Makefile: aumodel6/reqt.opt
	reqt debug aumodel6

aumodel7/Makefile: aumodel7/reqt.opt
	reqt debug aumodel7

aumodel8/Makefile: aumodel8/reqt.opt
	reqt debug aumodel8

aumodel9/Makefile: aumodel9/reqt.opt
	reqt debug aumodel9

aumodel10/Makefile: aumodel10/reqt.opt
	reqt debug aumodel10

aubodypartsres/Makefile: aubodypartsres/reqt.opt
	reqt debug aubodypartsres

aubodyparts0/Makefile: aubodyparts0/reqt.opt
	reqt debug aubodyparts0

aubodyparts1/Makefile: aubodyparts1/reqt.opt
	reqt debug aubodyparts1

aubodyparts2/Makefile: aubodyparts2/reqt.opt
	reqt debug aubodyparts2

aubodyparts3/Makefile: aubodyparts3/reqt.opt
	reqt debug aubodyparts3

aubodyparts4/Makefile: aubodyparts4/reqt.opt
	reqt debug aubodyparts4

aubodyparts5/Makefile: aubodyparts5/reqt.opt
	reqt debug aubodyparts5

aubodyparts6/Makefile: aubodyparts6/reqt.opt
	reqt debug aubodyparts6

aubodyparts7/Makefile: aubodyparts7/reqt.opt
	reqt debug aubodyparts7

aupowerarmor/Makefile: aupowerarmor/reqt.opt
	reqt debug aupowerarmor

aulauncher/Makefile: aulauncher/reqt.opt
	reqt debug aulauncher

auviewer/Makefile: auviewer/reqt.opt
	reqt debug auviewer

ausceneclient/Makefile: ausceneclient/reqt.opt
	reqt debug ausceneclient

auchatclient/Makefile: auchatclient/reqt.opt
	reqt debug auchatclient

ausceneserver/Makefile: ausceneserver/reqt.opt
	reqt debug ausceneserver

auscript/Makefile: auscript/reqt.opt
	reqt debug auscript

auserverstatus/Makefile: auserverstatus/reqt.opt
	reqt debug auserverstatus

sfx: auglobal/sfx/sfx.har

auglobal/sfx/sfx.har : auglobal/sfx/Lava.wav \
		auglobal/sfx/FootstepsPavement.wav \
		auglobal/sfx/DoorCargoOpen.wav \
		auglobal/sfx/WingFlap.wav \
		auglobal/sfx/HitElectric.wav \
		auglobal/sfx/Bleep4.wav \
		auglobal/sfx/RingworldStargate.wav \
		auglobal/sfx/DoorHatchClose.wav \
		auglobal/sfx/WindPortal.wav \
		auglobal/sfx/ExplodeVehicle.wav \
		auglobal/sfx/HitSeed.wav \
		auglobal/sfx/Bleep7.wav \
		auglobal/sfx/HitParalysis.wav \
		auglobal/sfx/Chat4.wav \
		auglobal/sfx/DragWoodFloor.wav \
		auglobal/sfx/HitPierce.wav \
		auglobal/sfx/FootstepsRock.wav \
		auglobal/sfx/ServoStopped.wav \
		auglobal/sfx/LaserCannon.wav \
		auglobal/sfx/Submarine.wav \
		auglobal/sfx/DoorExtOpen.wav \
		auglobal/sfx/TorchFlame.wav \
		auglobal/sfx/FireRifle.wav \
		auglobal/sfx/AlertYellow.wav \
		auglobal/sfx/AlertRed.wav \
		auglobal/sfx/DirtBikeIdle.wav \
		auglobal/sfx/HitRadiation.wav \
		auglobal/sfx/FootstepsMetal.wav \
		auglobal/sfx/HitCannon.wav \
		auglobal/sfx/DragRock.wav \
		auglobal/sfx/HelicopterEngine.wav \
		auglobal/sfx/DragForest.wav \
		auglobal/sfx/RocketBlastInt.wav \
		auglobal/sfx/KnifeSwish.wav \
		auglobal/sfx/BreathingScuba.wav \
		auglobal/sfx/DoorExtClose.wav \
		auglobal/sfx/EatingMeat.wav \
		auglobal/sfx/Dig.wav \
		auglobal/sfx/LaserPulse.wav \
		auglobal/sfx/DragHardFloor.wav \
		auglobal/sfx/FireMachineGun.wav \
		auglobal/sfx/DragSavanna.wav \
		auglobal/sfx/HitSlash.wav \
		auglobal/sfx/FireCannon.wav \
		auglobal/sfx/DragSnow.wav \
		auglobal/sfx/ShroudOfGlory.wav \
		auglobal/sfx/Bleep8.wav \
		auglobal/sfx/RocketBlastExt.wav \
		auglobal/sfx/BombRelease.wav \
		auglobal/sfx/HitSonic.wav \
		auglobal/sfx/Bleep1.wav \
		auglobal/sfx/SnipWire.wav \
		auglobal/sfx/KnifeHitNoDamage.wav \
		auglobal/sfx/FireShotgun.wav \
		auglobal/sfx/NativeDrums.wav \
		auglobal/sfx/FootstepsSnow.wav \
		auglobal/sfx/KnifeHitCritical.wav \
		auglobal/sfx/AlertGreen.wav \
		auglobal/sfx/FireHandgun.wav \
		auglobal/sfx/Chat1.wav \
		auglobal/sfx/DoorIntOpen.wav \
		auglobal/sfx/SUVIdle.wav \
		auglobal/sfx/HitBludgeon.wav \
		auglobal/sfx/HitDisease.wav \
		auglobal/sfx/BreathingInMask.wav \
		auglobal/sfx/Chat3.wav \
		auglobal/sfx/HitFire.wav \
		auglobal/sfx/FootstepsWoodFloor.wav \
		auglobal/sfx/HitPierceNoDamage.wav \
		auglobal/sfx/BuddyOnLine.wav \
		auglobal/sfx/HitPierceCritical.wav \
		auglobal/sfx/DragSand.wav \
		auglobal/sfx/DragPavement.wav \
		auglobal/sfx/FootstepsGravel.wav \
		auglobal/sfx/FootstepsHardFloor.wav \
		auglobal/sfx/DirtBikeRPM2000.wav \
		auglobal/sfx/HitGasping.wav \
		auglobal/sfx/Chat2.wav \
		auglobal/sfx/KnifeHit.wav \
		auglobal/sfx/HitExplosion.wav \
		auglobal/sfx/EatingPlants.wav \
		auglobal/sfx/CommChannel.wav \
		auglobal/sfx/ChatSOS.wav \
		auglobal/sfx/Bleep6.wav \
		auglobal/sfx/PlantFlag.wav \
		auglobal/sfx/DoorIntClose.wav \
		auglobal/sfx/Bleep5.wav \
		auglobal/sfx/FootstepsForest.wav \
		auglobal/sfx/SpaceShipRumble.wav \
		auglobal/sfx/TractorBeam.wav \
		auglobal/sfx/FootstepsWater.wav \
		auglobal/sfx/GasLamp.wav \
		auglobal/sfx/Harvest.wav \
		auglobal/sfx/DragMetal.wav \
		auglobal/sfx/ServoWeak.wav \
		auglobal/sfx/Rain.wav \
		auglobal/sfx/CracklingFire.wav \
		auglobal/sfx/Telemetry.wav \
		auglobal/sfx/DoorCargoClose.wav \
		auglobal/sfx/HitSleep.wav \
		auglobal/sfx/ExplodeSpacecraft.wav \
		auglobal/sfx/HitPoison.wav \
		auglobal/sfx/LaserBeam.wav \
		auglobal/sfx/DragWater.wav \
		auglobal/sfx/FireAK47.wav \
		auglobal/sfx/HitCold.wav \
		auglobal/sfx/FootstepsSavanna.wav \
		auglobal/sfx/Bleep3.wav \
		auglobal/sfx/WindWhistling.wav \
		auglobal/sfx/HitAcid.wav \
		auglobal/sfx/Whizzer.wav \
		auglobal/sfx/DoorHatchOpen.wav \
		auglobal/sfx/Bleep2.wav \
		auglobal/sfx/ColderThanIce.wav \
		auglobal/sfx/SailBoat.wav
	@cd auglobal/sfx ; \
	hashpack -c sfx.har *.wav

