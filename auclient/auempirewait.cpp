//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempirewait.cpp,v 1.5 2022/02/19 18:10:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auempirewait.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

AuEmpireWait::AuEmpireWait(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Agent.png"));
	setAuDlgTitle("Empire Data Requested");

	m_EmpiresKnown = 0;

	QLabel* lb0 = new QLabel(tr("The complete list of empires was requested from the server. Here is what I have so far."));

	m_lbProgress = new QLabel("Waiting for empire records.");
	m_lbProgress->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb0);
	lay->addWidget(m_lbProgress);
	setAuDlgLayout(lay);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(TimerUpdate()));
	timer->start(50);
}

AuEmpireWait::~AuEmpireWait()
{
}

void AuEmpireWait::TimerUpdate()
{
	const DgData& gd = g_Au.LockDataForRead();
	int empires = (int)gd.m_gEmpires.size();
	g_Au.UnlockData();
	if(empires != m_EmpiresKnown)
	{
		m_EmpiresKnown = empires;
		m_lbProgress->setText(tr("%1 empire records received.").arg(m_EmpiresKnown));
	}
}
