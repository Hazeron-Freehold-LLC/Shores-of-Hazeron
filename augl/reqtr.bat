@echo off
qmake -project -t lib "TARGET = AuGL" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "QT += opengl" "DESTDIR = ..\release" "INCLUDEPATH = ..\auutil ..\aumath" "DEFINES += AUGL"
qmake "LIBS += -lOpenGL32 ../release/auutil.lib ../release/aumath.lib %SEIDEV%\SDL2-2.0.3\VisualC\SDL\%AUDEVSDL%\Release\SDL2.lib"
