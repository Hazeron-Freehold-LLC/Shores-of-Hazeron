//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioaccount.cpp,v 1.16 2022/02/19 18:08:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubioaccount.h"
//
#include "auapp.h"
#include "audef.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

AuBioAccount::AuBioAccount(QWidget *parent) : QWidget(parent)
{
	m_Timer = 0;

	lbIcon = new QLabel(this);

	lbName = new QLabel(this);

	lbStatus = new QLabel(this);

	QVBoxLayout* iconlay = new QVBoxLayout;
	iconlay->addWidget(lbIcon);
	iconlay->addStretch();

	QVBoxLayout* infolay = new QVBoxLayout;
	infolay->addWidget(lbName);
	infolay->addWidget(lbStatus);
	infolay->addStretch(3);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addLayout(iconlay);
	lay->addLayout(infolay,1);
	lay->addStretch(1);
	setLayout(lay);
}

AuBioAccount::~AuBioAccount()
{
}

void AuBioAccount::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
		UpdateContents();	// Don't wait for the first one.
	}
	if(!m_Timer->isActive())
		m_Timer->start(30000);
}

void AuBioAccount::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	const DgData& gd = g_Au.LockDataForRead();
	ACCOUNT acct = gd.ClientAccountType();
	QString acctName = gd.GetAcctName();
	g_Au.UnlockData();

	QString acctIcon = AcctTypeIcon(acct);
	QString acctType = AcctTypeName(acct);

	lbIcon->setPixmap(QPixmap(acctIcon).scaled(64,128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	lbName->setText(tr("Name <b>%1</b>").arg(acctName));

	lbStatus->setText(tr("Status <b>%1</b>").arg(acctType));
}

