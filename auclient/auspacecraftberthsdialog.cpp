//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftberthsdialog.cpp,v 1.3 2022/02/19 18:16:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftberthsdialog.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auspacecraftberths.h"
#include <QSplitter>
#include <QVBoxLayout>

AuSpacecraftBerthsDialog::AuSpacecraftBerthsDialog(QWidget* parent) : AuDialog(parent)
{
	AuFrame* frame = auApp->Frame();

	setAuDlgTitle(tr("Spacecraft Berths"));
	setAuDlgIcon(QIcon(":data/CommCrew.png"));

	m_Berths = new AuSpacecraftBerths(this);
	connect(m_Berths, SIGNAL(KickBerthOccupant(AuId)), frame, SLOT(KickBerthOccupant(AuId)));
	connect(m_Berths, SIGNAL(ReconcileRoster()), frame, SLOT(ReconcileRoster()));
	connect(m_Berths, SIGNAL(StowawaysKicked()), frame, SLOT(KickStowaways()));
	connect(m_Berths->m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Berths);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSpacecraftBerthsDialog::~AuSpacecraftBerthsDialog()
{
}

void AuSpacecraftBerthsDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuSpacecraftBerthsDialog::SplitterMoved(int /*pos*/, int /*index*/)
{
	// Save splitter state.
	QByteArray splitterState = m_Berths->m_Splitter->saveState();
	g_Au.Settings().readEntry("Widget/SpacecraftBerthsDialog/Splitter", splitterState);
}

void AuSpacecraftBerthsDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SpacecraftBerthsDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SpacecraftBerthsDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SpacecraftBerthsDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SpacecraftBerthsDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	QByteArray splitterState;
	g_Au.Settings().readEntry("Widget/SpacecraftBerthsDialog/Splitter", splitterState);
	if(!splitterState.isEmpty())
		m_Berths->m_Splitter->restoreState(splitterState);
}

void AuSpacecraftBerthsDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SpacecraftBerthsDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SpacecraftBerthsDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SpacecraftBerthsDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SpacecraftBerthsDialog/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/SpacecraftBerthsDialog/Splitter", m_Berths->m_Splitter->saveState());
}
