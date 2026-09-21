@echo off
qmake -project -t lib "TARGET = AuShipTex0" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "DESTDIR = ../Release"
qmake
