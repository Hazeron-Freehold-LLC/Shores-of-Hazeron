//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuSendFunds.cpp,v 1.7 2022/02/19 18:16:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuSendFunds.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#define c_MinFundsTransfer		1000		// Minimum amount of money that can be transferred.

AuSendFunds::AuSendFunds(const QString& titleText, const QString& toName, const AuId& toId, QWidget* parent) : AuDialog(parent)
{
	m_ToId = toId;
	m_ToName = toName;

	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	setAuDlgIcon(QIcon(":/data/c_Money.png"));
	if(titleText.isEmpty())
		setAuDlgTitle("Transfer Funds");
	else
		setAuDlgTitle(titleText);

	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		m_AccountBalance = person->AccountBalance();
	else
		m_AccountBalance = 0;
	pView->UnlockSceneAtom();

	QString minFunds = QString("%L1%2").arg(c_MinFundsTransfer).arg(g_Au.CurrencySymbol());
	QString sFunds = QString("%L1%2").arg(m_AccountBalance,0,'f',0).arg(g_Au.CurrencySymbol());
	QLabel* lb0;
	if(isAgent)
		lb0 = new QLabel(tr("<html>"
					"<p>Send funds to the sender of a message.</p>"
					"<p>Money can be sent to players and spacecraft <u>in your current galaxy</u>.</p>"
					"<p>The minimum amount that can be transferred is %1.</p>"
					"<p>How much of your personal funds will you send?</p>"
					"<p>Your personal account balance is %2.</p>"
					"<p><b>When agent power is enabled, the amount is not limited to your account balance and "
					"the money is not deducted from your account.</b></p>"
					"</html>").arg(minFunds).arg(sFunds));
	else
		lb0 = new QLabel(tr("<html>"
					"<p>Send funds to the sender of a message.</p>"
					"<p>Money can be sent to players and spacecraft <u>in your current galaxy</u>.</p>"
					"<p>The minimum amount that can be transferred is %1.</p>"
					"<p>How much of your personal funds will you send?</p>"
					"<p>Your personal account balance is %2.</p>"
					"</html>").arg(minFunds).arg(sFunds));
	lb0->setWordWrap(true);

	m_AmountEdit = new QLineEdit;
	connect(m_AmountEdit, SIGNAL(textChanged(const QString&)), SLOT(AmountChanged(const QString&)));

	m_SendButton = new QPushButton("Send the Funds");
	m_SendButton->setDefault(true);
	m_SendButton->setAutoDefault(true);
	m_SendButton->setEnabled(false);
	connect(m_SendButton, SIGNAL(clicked()), SLOT(SendFunds()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lb0, row++,0);
	row++;
	lay->addWidget(m_AmountEdit, row++,0);
	row++;
	lay->addWidget(m_SendButton, row++,0);
	setAuDlgLayout(lay);
}

AuSendFunds::~AuSendFunds()
{
}

void AuSendFunds::SendFunds()
{
	double amount = QVariant(m_AmountEdit->text()).toDouble();
	if(amount >= c_MinFundsTransfer)
	{
		if(auApp->SendFunds(m_ToName, m_ToId, amount))
			accept();
		else
			reject();
	}
}

void AuSendFunds::AmountChanged(const QString& amt)
{
	// People are limited to sending their account balance.
	double amount = QVariant(amt).toDouble();
	bool ok = amount >= c_MinFundsTransfer && amount <= m_AccountBalance;

	// Agents are not limited to sending their account balance.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	if(isAgent && amount >= c_MinFundsTransfer)
		ok = true;
	g_Au.UnlockData();

	m_SendButton->setEnabled(ok);
}

