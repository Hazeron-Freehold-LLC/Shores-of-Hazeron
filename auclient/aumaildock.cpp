//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumaildock.cpp,v 1.5 2022/02/19 18:16:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumaildock.h"
//
#include "auapp.h"
#include "aumail.h"
#include "auframe.h"
#include "auview.h"

void AuMailDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleMailUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMailDock::AuMailDock(const AuId& selId, QWidget* parent) : AuDockWidget(":data/ToggleMail.png", tr("<b>Mail</b>"), tr("Mail"),
		parent)
{
	m_Mail = new AuMail(this);
	setWidget(m_Mail);
	connect(m_Mail, SIGNAL(AlertMessageReceived(AuMailMsg)), SIGNAL(AlertMessageReceived(AuMailMsg)));
	connect(m_Mail, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)), SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)));
	connect(m_Mail, SIGNAL(GoDirectToCoord(AuPoint)), SIGNAL(GoDirectToCoord(AuPoint)));
	connect(m_Mail, SIGNAL(GoLandAtCity(AuId, QString)), SIGNAL(GoLandAtCity(AuId, QString)));
	connect(m_Mail, SIGNAL(GoOrbit(AuId, QString)), SIGNAL(GoOrbit(AuId, QString)));
	connect(m_Mail, SIGNAL(GoOrbitCity(AuId, QString)), SIGNAL(GoOrbitCity(AuId, QString)));
	connect(m_Mail, SIGNAL(GoViaWormholes(AuId, QString)), SIGNAL(GoViaWormholes(AuId, QString)));
	connect(m_Mail, SIGNAL(UpdateSquelch(AuId, bool)), SIGNAL(UpdateSquelch(AuId, bool)));
	connect(m_Mail, SIGNAL(UpdateSquelches(AuIdList, bool)), SIGNAL(UpdateSquelches(AuIdList, bool)));

	int dockSides = g_Au.Settings().readNumEntry("Dock/MailAreas", (int)allowedAreas());
	setAllowedAreas((Qt::DockWidgetAreas)dockSides);

	if(!selId.IsNull())
		m_Mail->SetRecipient(selId);
}

void AuMailDock::SetRecipient(const AuId& personId)
{
	m_Mail->SetRecipient(personId);
}

void AuMailDock::StopThreads()
{
	m_Mail->StopThreads();
}

void AuMailDock::AvatarsUpdated()
{
	m_Mail->AvatarsUpdated();
}
