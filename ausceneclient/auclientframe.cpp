//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientframe.cpp,v 1.16 2022/02/19 18:39:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientframe.h"
//
#include "auapp.h"
#include "auclientview.h"
#include "auintro.h"

AuClientFrame::AuClientFrame(QWidget *parent) : AuDesignerFrame(parent)
{
	// Create intro screen.
	m_Intro = new AuIntro();
	connect(m_Intro, SIGNAL(LoginOk()), auApp, SLOT(LoginSucceeded()));
	connect(m_Intro, SIGNAL(AvatarLogin(AuId)), auApp, SLOT(StartSceneCommunications(AuId)));
#if defined(INTRO_SOLO_TOO)
	connect(m_Intro, SIGNAL(AvatarSolo(AuId)), auApp, SLOT(StartSolitaire(AuId)));
#endif
	connect(m_Intro, SIGNAL(HelpAbout()), SLOT(HelpAbout()));
	connect(m_Intro, SIGNAL(LaunchDesigner()), SLOT(LaunchDesigner()));
	connect(m_Intro, SIGNAL(ToggleSettingsUI()), SLOT(ToggleSettingsUi()));
	connect(m_Intro, SIGNAL(ToggleFullScreen()), SLOT(ToggleFullScreen()));
	connect(m_Intro, SIGNAL(ExitGame()), auApp, SLOT(closeAllWindows()));
	setCentralWidget(m_Intro);
}

AuClientFrame::~AuClientFrame()
{
}

AuDesignerView* AuClientFrame::CreateViewWidget()
{
	//qDebug() << "AuClientFrame::CreateViewWidget";
	AuClientView* view = new AuClientView(this);
	connect(view, SIGNAL(SceneContextChanged(int)), SLOT(SceneContextChanged(int)));
	connect(view, SIGNAL(SceneAtomChanged()), SLOT(SceneAtomChanged()));
	connect(view, SIGNAL(SelectGame()), SLOT(SelectGame()), Qt::QueuedConnection);
	connect(view, SIGNAL(ToggleScreenReady()), SLOT(ToggleFullScreen()), Qt::QueuedConnection);
	connect(view, SIGNAL(DesignerSelectionChanged()), SLOT(DesignerEnableControls()));
	connect(view, SIGNAL(DesignerPreviewChanged()), SLOT(DesignerEnableControls()));

	// Replace the splash screen with the game UI.
	setCentralWidget(view);

	// Close the splash screen widget now that it is no longer the central widget.
	if(m_Intro)
	{
		m_Intro->close();
		m_Intro = 0;
	}
	return view;
}

