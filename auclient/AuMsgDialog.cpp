//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMsgDialog.cpp,v 1.4 2022/02/19 18:16:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuMsgDialog.h"
//
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>

AuMsgDialog::AuMsgDialog(QWidget* parent, QString sMsg, bool bTitle) : AuDialog(parent, bTitle)
{
	m_Mode = 0;
	m_Step = 0;
	QGroupBox* gb = new QGroupBox(this);

	m_MsgLabel = new QLabel(sMsg, this);
	m_MsgLabel->setAlignment(Qt::AlignCenter);

	m_IconLabel = new QLabel(this);

	QHBoxLayout* glay = new QHBoxLayout;
	glay->addStretch();
	glay->addWidget(m_IconLabel);
	glay->addWidget(m_MsgLabel);
	glay->addStretch();
	glay->setSpacing(2);
	glay->setMargin(12);
	gb->setLayout(glay);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(gb);
	lay->setMargin(4);
	lay->setSpacing(0);
	setAuDlgLayout(lay);
	//setWindowOpacity(.5);
}

AuMsgDialog::~AuMsgDialog()
{
}

void AuMsgDialog::SetMessage(int mode, QString sMsg)
{
	m_Mode = mode;
	m_MsgLabel->setText(sMsg);
	m_Step = 0;
	StepNext();
}

void AuMsgDialog::StepNext()
{
	m_Step++;
	if(m_Step > 4)
		m_Step = 1;
	if(m_Mode == 0)
	{
		QString icon = QString(":data/Login%1.png").arg(m_Step);
		m_IconLabel->setPixmap(QPixmap(icon));
	}
	else
	{
		QString icon = QString(":data/EnterHazeron%1.png").arg(m_Step);
		m_IconLabel->setPixmap(QPixmap(icon));
	}
}
