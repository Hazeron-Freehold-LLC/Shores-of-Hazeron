@echo off
qmake -project -t lib "TARGET = AuBodyPartsRes" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUBODYPARTSRES" "RC_FILE = AuBodyPartsRes.rc"
qmake "LIBS += ../release/AuBodyParts0.lib ../release/AuBodyParts1.lib ../release/AuBodyParts2.lib ../release/AuBodyParts3.lib ../release/AuBodyParts4.lib ../release/AuBodyParts5.lib ../release/AuBodyParts6.lib ../release/AuBodyParts7.lib ../release/AuPowerArmor.lib"
