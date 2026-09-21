@echo off
qmake -project -t lib "TARGET = AuData" "VERSION = 7.0" "TEMPLATE = lib" "CONFIG += staticlib qt thread" "QT += opengl network sql" "INCLUDEPATH = ../auutil ../auatom ../auglobal ../auloginserver ../aumailserver ../ausceneserver ../augl ../aumath %SEIDEV%\OPENAL~1.1SD\include" "DESTDIR = ../Release" "DEFINES += AUDATA" "QMAKE_CXXFLAGS += /MP"
qmake "LIBS += ../release/auglobal.lib ../release/auutil.lib ../release/aumath.lib ../release/augl.lib %SEIDEV%\OPENAL~1.1SD\libs\%AUDEVOPENAL%\OpenAL32.lib %SEIDEV%\SDL2-2.0.3\VisualC\SDL\%AUDEVSDL%\Release\SDL2.lib"
