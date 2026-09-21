//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemaccesscodenew.cpp,v 1.3 2022/02/19 18:17:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemaccesscodenew.h"
//
#include "auglobal.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuSystemAccessCodeNew::AuSystemAccessCodeNew(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Change System Access Code"));
	setAuDlgIcon(QIcon(":data/Padlock.png"));

	m_CodeDisabled = false;

	quint16 lastcode = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/LastEntry", 1000);
	int ones, tens, hundreds;
	if(lastcode < 1000)
	{
		ones = lastcode;
		hundreds = ones / 100;
		ones -= hundreds * 100;
		tens = ones / 10;
		ones -= tens * 10;
	}
	else
	{
		ones = 0;
		tens = 0;
		hundreds = 0;
	}

	QLabel* lblock = new QLabel(this);
	lblock->setPixmap(QPixmap(":data/Padlock.png"));

	QLabel* lbdo = new QLabel(tr("Select a new access code for critical systems."), this);

	m_DigitHundreds = new QSpinBox(this);
	m_DigitHundreds->setRange(0,9);
	m_DigitHundreds->setValue(hundreds);

	m_DigitTens = new QSpinBox(this);
	m_DigitTens->setRange(0,9);
	m_DigitTens->setValue(tens);

	m_DigitOnes = new QSpinBox(this);
	m_DigitOnes->setRange(0,9);
	m_DigitOnes->setValue(ones);

	QPushButton* randomsac = new QPushButton(tr("Randomize"), this);
	randomsac->setToolTip(tr("<html>"
				"Selects random access code values."
				"</htmlm>"));
	connect(randomsac, SIGNAL(clicked()), SLOT(Randomize()));

	QPushButton* disablesac = new QPushButton(tr("Disable System Access Code Security"), this);
	disablesac->setToolTip(tr("<html>"
				"Disables system access code security. "
				"Critical systems will not require login. "
				"<b>This is extremely unsecure.</b>"
				"</htmlm>"));
	connect(disablesac, SIGNAL(clicked()), SLOT(DisableCode()));

	QPushButton* ok = new QPushButton(tr("Change System Access Code"), this);
	ok->setAutoDefault(true);
	ok->setDefault(true);
	ok->setToolTip(tr("Assigns a new access code for logging into critical systems."));
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* dlay = new QHBoxLayout;
	dlay->addStretch();
	dlay->addWidget(m_DigitHundreds);
	dlay->addWidget(m_DigitTens);
	dlay->addWidget(m_DigitOnes);
	dlay->addWidget(randomsac);
	dlay->addStretch();

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lblock, row,0, 3,1);
	lay->addWidget(lbdo, row,1, 1,3);
	row++;
	lay->addLayout(dlay, row,1, 1,3);
	row++;
	lay->addWidget(disablesac, row,1, 1,3);
	row++;
	lay->addWidget(ok, row,1, 1,3);
	lay->setSpacing(2);
	lay->setMargin(2);
	setAuDlgLayout(lay);
}

AuSystemAccessCodeNew::~AuSystemAccessCodeNew()
{
}

void AuSystemAccessCodeNew::setVisible(bool visible)
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

void AuSystemAccessCodeNew::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemAccessCodeNew/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemAccessCodeNew/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemAccessCodeNew/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemAccessCodeNew/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSystemAccessCodeNew::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemAccessCodeNew/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemAccessCodeNew/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemAccessCodeNew/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemAccessCodeNew/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/SystemAccessCode/LastEntry", AccessCode());
}

quint16 AuSystemAccessCodeNew::AccessCode() const
{
	if(m_CodeDisabled)
		return 1000;

	int hundreds = m_DigitHundreds->value();
	int tens = m_DigitTens->value();
	int ones = m_DigitOnes->value();
	quint16 sac = hundreds * 100 + tens * 10 + ones;
	return sac;
}

void AuSystemAccessCodeNew::DisableCode()
{
	m_CodeDisabled = true;
	accept();
}

void AuSystemAccessCodeNew::Randomize()
{
	quint16 newsac = g_Au.Dice().Roll(1,1000,-1);

	int ones, tens, hundreds;
	ones = newsac;
	hundreds = ones / 100;
	ones -= hundreds * 100;
	tens = ones / 10;
	ones -= tens * 10;

	m_DigitHundreds->setValue(hundreds);
	m_DigitTens->setValue(tens);
	m_DigitOnes->setValue(ones);
}
