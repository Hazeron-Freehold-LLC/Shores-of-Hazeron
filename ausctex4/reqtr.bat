@echo off
qmake -project -t lib "TARGET = AuScTex4" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "DESTDIR = ../Release" "QMAKE_CXXFLAGS_RELEASE += -Zm1500"
qmake
