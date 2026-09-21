@echo off
qmake -project -t lib "TARGET = AuAtomR2" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "DESTDIR = ../Release"
qmake
