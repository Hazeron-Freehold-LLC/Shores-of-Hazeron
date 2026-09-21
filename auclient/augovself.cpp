//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovself.cpp,v 1.2 2022/02/19 18:16:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovself.h"
//
#include <auglobal.h>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>

AuGovSelf::AuGovSelf(const QString& name, QWidget* parent) : QWidget(parent)
{
	setObjectName(name);

	QLabel* lbactive = new QLabel(tr("Transparency of Active Window"), this);
	lbactive->setAlignment(Qt::AlignCenter);

	QLabel* lbinactive = new QLabel(tr("Transparency of Inactive Windows"), this);
	lbinactive->setAlignment(Qt::AlignCenter);

	QSlider* slactive = new QSlider(Qt::Horizontal, this);
	slactive->setRange(0, 95);
	slactive->setSingleStep(5);
	slactive->setPageStep(5);
	int actTrans = 100 - g_Au.WindowOpacity(true);
	slactive->setValue(actTrans);
	connect(slactive, SIGNAL(valueChanged(int)), SLOT(SetActiveTrans(int)));

	QSlider* slinactive = new QSlider(Qt::Horizontal, this);
	slinactive->setRange(0, 95);
	slinactive->setSingleStep(5);
	slinactive->setPageStep(5);
	int inactTrans = 100 - g_Au.WindowOpacity(false);
	slinactive->setValue(inactTrans);
	connect(slinactive, SIGNAL(valueChanged(int)), SLOT(SetInactiveTrans(int)));

	QLabel* lbless0 = new QLabel(tr("Less Transparent"), this);

	QLabel* lbless1 = new QLabel(tr("Less Transparent"), this);

	QLabel* lbmore0 = new QLabel(tr("More Transparent"), this);
	lbmore0->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbmore1 = new QLabel(tr("More Transparent"), this);
	lbmore1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QWidget* spacer = new QWidget(this);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbactive, row,0, 1,2);
	row++;
	lay->addWidget(slactive, row,0, 1,2);
	row++;
	lay->addWidget(lbless0, row,0, 1,1);
	lay->addWidget(lbmore0, row,1, 1,1);
	row++;
	lay->addWidget(lbinactive, row,0, 1,2);
	row++;
	lay->addWidget(slinactive, row,0, 1,2);
	row++;
	lay->addWidget(lbless1, row,0, 1,1);
	lay->addWidget(lbmore1, row,1, 1,1);
	row++;
	lay->addWidget(spacer, row,0);
	lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuGovSelf::SetActiveTrans(int value)
{
	g_Au.SetWindowOpacity(true, 100 - value);
}

void AuGovSelf::SetInactiveTrans(int value)
{
	g_Au.SetWindowOpacity(false, 100 - value);
}

