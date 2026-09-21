@echo off
qmake -project -t lib "TARGET = AuTexture" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += dll qt thread" "DESTDIR = ../Release" "DEFINES += AUTEXTURE"
qmake "LIBS += ../release/AuShipTex0.lib ../release/AuScTex1.lib ../release/AuScTex2.lib ../release/AuScTex3.lib ../release/AuScTex4.lib ../release/AuScTex5.lib"
