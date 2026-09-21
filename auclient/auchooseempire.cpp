//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchooseempire.cpp,v 1.2 2022/02/19 18:10:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auchooseempire.h"
//
#include <dgdata.h>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

AuChooseEmpire::AuChooseEmpire(const QString& caption, QWidget* parent) : AuDialog(parent)
{
	m_EmpireId = 0;

	setAuDlgTitle(tr("Choose Empire"));
	setAuDlgIcon(QIcon(":data/GovPolicy.png"));

	QLabel* lbico = new QLabel(this);
	lbico->setPixmap(QPixmap(":data/SurrenderFlag.png"));

	QLabel* lb = new QLabel(caption, this);
	lb->setWordWrap(true);

	m_Empires = new QComboBox(this);
	const DgData& gd = g_Au.LockDataForRead();
	gd.ListAllEmpires(*m_Empires);
	g_Au.UnlockData();
	m_Empires->setCurrentIndex(-1);
	//m_Empires->setToolTip(tr("<html>* denotes pirate empire.</html>"));
	connect(m_Empires, SIGNAL(currentIndexChanged(int)), SLOT(EmpireSelected(int)));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>Choose the selected empire.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QVBoxLayout* layico = new QVBoxLayout;
	layico->addWidget(lbico);
	layico->addStretch();

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(layico, row,0);
	lay->addWidget(lb, row,1, 1,2);
	row++;
	lay->addWidget(m_Empires, row,1, 1,2);
	row++;
	lay->addWidget(pbok, row,2);
	lay->setSpacing(2);
	lay->setMargin(2);

	setAuDlgLayout(lay);
}

AuChooseEmpire::~AuChooseEmpire()
{
}

void AuChooseEmpire::EmpireSelected(int index)
{
	m_EmpireId = m_Empires->itemData(index).toInt();
}
