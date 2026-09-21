//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientapp.cpp,v 1.15 2022/02/19 18:39:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientapp.h"
//
#include "auclientglobal.h"
#include "auframe.h"
#include "dgdata.h"

AuClientApp::AuClientApp(int& argc, char** argv) : AuDesignerApp(argc, argv, "Shores of Hazeron", "Shores of Hazeron", false)
{
	//g_Au.m_LogMessages |= LOG_Story;
	//g_Au.m_LogMessages |= LOG_Spawn;
	//g_Au.m_LogMessages |= LOG_Etherium;
}

AuClientApp::~AuClientApp()
{
}

QString AuClientApp::Icon128() const
{
	return QString(":data/SoHLogo128.png");
}

QString AuClientApp::Icon32() const
{
	return QString(":data/SoHLogo32.png");
}

QString AuClientApp::Slogan() const
{
	return QString("The MMO Sandbox Universe");
}
