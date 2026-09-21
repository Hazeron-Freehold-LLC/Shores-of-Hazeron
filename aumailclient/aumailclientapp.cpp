//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailclientapp.cpp,v 1.27 2022/04/07 22:03:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailclientapp.h"
//
#include "auglobal.h"
#include "aulogincomm.h"
#include "austylesheet.h"
#include <dgdata.h>

AuMailClientApp::AuMailClientApp(int& argc, char** argv) :
	QApplication(argc, argv)
{
	// Create 'client mode' global data object independent of app object.
	Q_ASSERT(gp_AuGlobal == 0);
	m_Data = new DgData;
	// The second parameter is the application name used in the settings object, so my settings are syncronous with the mail
	// UI inside the Shores of Hazeron game app.
	gp_AuGlobal = new AuGlobal("Hazeron Mail", "Shores of Hazeron", "Hazeron Freehold", AuGlobal::MailClient, m_Data);
	
	// Parse command line.
	QString altMaster;
	bool setStyle = true;
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.constBegin(); it != args.constEnd(); it++)
	{
		if((*it).left(2) == "-a")
			altMaster = (*it).mid(2);
		else if(*it == "-nostyle")
			setStyle = false;
	}

	// Set style
	AuStyleSheet::SetStyle(!setStyle);

	// Set alternate servers.
	if(!altMaster.isEmpty())
	{
		// Set 'all' first so individuals will override the result appropriately.
		g_Au.SetLoginMaster(altMaster);
		g_Au.SetMailMaster(altMaster);
		g_Au.SetSceneMaster(altMaster);
	}
}

AuMailClientApp::~AuMailClientApp()
{
	// Notify login server that we are logging out. This is a formality; it's not really essential.
	AuLoginComm login;
	login.Logout();

	// Delete global object.
	if(gp_AuGlobal)
	{
		delete gp_AuGlobal;
		gp_AuGlobal = 0;
	}

	// Delete data used by gp_AuGlobal.
	if(m_Data)
	{
		delete m_Data;
		m_Data = 0;
	}
}
