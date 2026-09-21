//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auapp.cpp,v 1.15 2022/02/19 18:57:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auapp.h"
//
#include "auframe.h"
#include "austylesheet.h"
#include "auview.h"
#include "auviewerglobal.h"
#include "dgdata.h"
#include <signal.h>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSysInfo>
#if defined(Q_OS_WIN)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

AuApp::AuApp(int& argc, char** argv) : QApplication(argc, argv)
{
	m_Data = new DgData;
	m_Frame = 0;
	m_Etherium = new AuEther("AuApp");

	gp_AuGlobal = new AuViewerGlobal("Hazeron Viewer", "Hazeron Freehold", m_Data, m_Etherium);

	// Create cache for 3ds models.
	m_ModelCache = new AuGLModelCache;
	g_Au.SetModelCache(m_ModelCache);

	// Set style
	SetStyle();

	// Initialize the Simple Directmedia Layer (SDL).
	if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
		qWarning() << "error: SDL failed to initialize. Simple Directmedia Layer will not be available.";

	g_Au.m_LogMessages |= LOG_Database;
	int err = m_Data->LoadBasicGameData();
	if(err)
		qWarning() << "error: AuApp::AuApp failed to load basic game data. error code" << err;
}

AuApp::~AuApp()
{
	// Shut down SDL.
	SDL_Quit();

	// Delete global object.
	if(gp_AuGlobal)
		delete gp_AuGlobal;

	// Delete data and root atom, which were used by gp_AuGlobal.
	if(m_Data)
		delete m_Data;

	if(m_Etherium)
		delete m_Etherium;

	if(m_ModelCache)
		delete m_ModelCache;
}

#if !defined(Q_OS_WIN)
void AuApp::HandleSignal(int value)
#else
void AuApp::HandleSignal(int)
#endif
{
#if !defined(Q_OS_WIN)
	if(value == SIGPIPE)
		return;			// Ignore broken pipe signal.
#endif

	closeAllWindows();	// Exit cleanly, letting things destruct properly.
}

void AuApp::SetStyle()
{
	AuStyleSheet::SetStyle(false);
}
