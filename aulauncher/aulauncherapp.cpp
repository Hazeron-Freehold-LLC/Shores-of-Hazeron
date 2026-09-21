//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulauncherapp.cpp,v 1.14 2022/02/19 18:34:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulauncherapp.h"
//
#include "austylesheet.h"
#include <QPalette>

AuLauncherApp::AuLauncherApp(int& argc, char** argv) : QApplication(argc, argv)
{
	AuStyleSheet::SetStyle(false);
}

AuLauncherApp::~AuLauncherApp()
{
}
