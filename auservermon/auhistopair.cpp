//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhistopair.cpp,v 1.9 2022/02/19 18:40:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auhistopair.h"
//
#include "austatusatoms.h"
#include "austatushistogram.h"
#include <QTimer>
#include <QGridLayout>

AuHistoPair::AuHistoPair(const QList<AuStatusReport>* status, QWidget* parent) : QWidget(parent)
{
	m_StatusAtoms = new AuStatusAtoms(status);
	connect(this, SIGNAL(TMinus(int)), m_StatusAtoms, SLOT(SetTMinus(int)));
	connect(m_StatusAtoms, SIGNAL(SystemRightClicked(AuId)), this, SIGNAL(SystemRightClicked(AuId)));

	m_StatusHistoAvg = new AuStatusHistogram(status, 0);
	connect(m_StatusHistoAvg, SIGNAL(TMinus(int)), SIGNAL(TMinus(int)));
	connect(this, SIGNAL(TMinus(int)), m_StatusHistoAvg, SLOT(SetTMinus(int)));

	m_StatusHistoCycle = new AuStatusHistogram(status, 2);
	connect(m_StatusHistoCycle, SIGNAL(TMinus(int)), SIGNAL(TMinus(int)));
	connect(this, SIGNAL(TMinus(int)), m_StatusHistoCycle, SLOT(SetTMinus(int)));

	m_StatusHistoMax = new AuStatusHistogram(status, 1);
	connect(m_StatusHistoMax, SIGNAL(TMinus(int)), SIGNAL(TMinus(int)));
	connect(this, SIGNAL(TMinus(int)), m_StatusHistoMax, SLOT(SetTMinus(int)));

	// Let them notify each other and the atom status widget.
	connect(m_StatusHistoAvg, SIGNAL(TMinus(int)), m_StatusHistoCycle, SLOT(SetTMinus(int)));
	connect(m_StatusHistoAvg, SIGNAL(TMinus(int)), m_StatusHistoMax, SLOT(SetTMinus(int)));
	connect(m_StatusHistoAvg, SIGNAL(TMinus(int)), m_StatusAtoms, SLOT(SetTMinus(int)));

	connect(m_StatusHistoCycle, SIGNAL(TMinus(int)), m_StatusHistoAvg, SLOT(SetTMinus(int)));
	connect(m_StatusHistoCycle, SIGNAL(TMinus(int)), m_StatusHistoMax, SLOT(SetTMinus(int)));
	connect(m_StatusHistoCycle, SIGNAL(TMinus(int)), m_StatusAtoms, SLOT(SetTMinus(int)));

	connect(m_StatusHistoMax, SIGNAL(TMinus(int)), m_StatusHistoAvg, SLOT(SetTMinus(int)));
	connect(m_StatusHistoMax, SIGNAL(TMinus(int)), m_StatusHistoCycle, SLOT(SetTMinus(int)));
	connect(m_StatusHistoMax, SIGNAL(TMinus(int)), m_StatusAtoms, SLOT(SetTMinus(int)));

	QGridLayout* mainLay = new QGridLayout;
	{
		mainLay->addWidget(m_StatusHistoCycle, 0,0);
		mainLay->addWidget(m_StatusHistoAvg, 0,1);
		mainLay->addWidget(m_StatusHistoMax, 0,2);
		mainLay->addWidget(m_StatusAtoms, 1,0, 1,3);
		mainLay->setMargin(0);
		mainLay->setSpacing(2);
		setLayout(mainLay);
	}

	// Get an update timer started.
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	timer->start(500);
}

AuHistoPair::~AuHistoPair()
{
}

void AuHistoPair::TimerUpdate()
{
	// Redraw the histograms.
	m_StatusAtoms->update();
	m_StatusHistoAvg->update();
	m_StatusHistoCycle->update();
	m_StatusHistoMax->update();
}

void AuHistoPair::SetTMinus(int ms)
{
	m_StatusAtoms->SetTMinus(ms);
	m_StatusHistoAvg->SetTMinus(ms);
	m_StatusHistoCycle->SetTMinus(ms);
	m_StatusHistoMax->SetTMinus(ms);
}
