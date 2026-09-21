//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemaccesscode.cpp,v 1.3 2022/02/19 18:17:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemaccesscode.h"
//
#include "auglobal.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuSystemAccessCode::AuSystemAccessCode(const QString& title, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(title);
	setAuDlgIcon(QIcon(":data/Padlock.png"));

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

	QGroupBox* group = new QGroupBox(tr("Authorized Personnel Only"));
	{
		QLabel* lblock = new QLabel(this);
		lblock->setPixmap(QPixmap(":data/Padlock.png"));

		m_DigitHundreds = new QSpinBox(this);
		m_DigitHundreds->setRange(0,9);
		m_DigitHundreds->setValue(hundreds);

		m_DigitTens = new QSpinBox(this);
		m_DigitTens->setRange(0,9);
		m_DigitTens->setValue(tens);

		m_DigitOnes = new QSpinBox(this);
		m_DigitOnes->setRange(0,9);
		m_DigitOnes->setValue(ones);

		QPushButton* okbutton = new QPushButton(tr("Login"), this);
		okbutton->setAutoDefault(true);
		okbutton->setDefault(true);
		okbutton->setToolTip(tr("Logs in to the system using the access code entered."));
		connect(okbutton, SIGNAL(clicked()), SLOT(accept()));

		QHBoxLayout* glay = new QHBoxLayout;
		glay->addWidget(lblock);
		glay->addStretch();
		glay->addWidget(m_DigitHundreds);
		glay->addWidget(m_DigitTens);
		glay->addWidget(m_DigitOnes);
		glay->addStretch();
		glay->addWidget(okbutton);
		glay->setMargin(12);
		glay->setSpacing(2);
		group->setLayout(glay);
	}

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(group);
	lay->setSpacing(2);
	lay->setMargin(2);
	setAuDlgLayout(lay);
}

AuSystemAccessCode::~AuSystemAccessCode()
{
}

void AuSystemAccessCode::setVisible(bool visible)
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

void AuSystemAccessCode::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SystemAccessCode/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSystemAccessCode::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SystemAccessCode/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/SystemAccessCode/LastEntry", AccessCode());
}

quint16 AuSystemAccessCode::AccessCode() const
{
	int hundreds = m_DigitHundreds->value();
	int tens = m_DigitTens->value();
	int ones = m_DigitOnes->value();
	quint16 sac = hundreds * 100 + tens * 10 + ones;
	return sac;
}
