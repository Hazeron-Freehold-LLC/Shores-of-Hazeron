//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augawithdraw.cpp,v 1.4 2022/02/19 18:10:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augawithdraw.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#define c_MinFundsTransfer				100						// Minimum amount of money that can be transferred.

AuGAWithdraw::AuGAWithdraw(const AuId& opBd, QWidget* parent) : AuDialog(parent)
{
	m_CapitolOpId = opBd;
	setAuDlgIcon(QIcon(":/data/c_Money.png"));
	setAuDlgTitle("Withdraw Funds From Bank System");

	double accountBalance = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		accountBalance = person->AccountBalance();
	pView->UnlockSceneAtom();

	QString minFunds = QString("%L1%2").arg(c_MinFundsTransfer).arg(g_Au.CurrencySymbol());

	QLabel* lb0 = new QLabel(tr("<html>The minimum amount that can be withdrawn is %L1. "
				"How much of the bank system will you withdraw into your personal account?</html>").arg(minFunds));
	lb0->setWordWrap(true);

	QLabel* lb1 = new QLabel(tr("Your personal account balance is %L1%2.").arg(accountBalance,0,'f',0).arg(g_Au.CurrencySymbol()));

	m_Amount = new QLineEdit;

	QPushButton* pb0 = new QPushButton("Withdraw Funds");
	connect(pb0, SIGNAL(clicked()), SLOT(WithdrawFunds()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lb0, row++,0);
	row++;
	lay->addWidget(lb1, row++,0);
	row++;
	lay->addWidget(m_Amount, row++,0);
	row++;
	lay->addWidget(pb0, row++,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGAWithdraw::~AuGAWithdraw()
{
}

void AuGAWithdraw::WithdrawFunds()
{
	// Determine the amount to be withdrawn.
	double amount = QVariant(m_Amount->text()).toDouble();

	// Insure the minimum amount is withdrawn.
	if(amount < c_MinFundsTransfer)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("The withdrawal amount must be at least %L1%2.").arg(c_MinFundsTransfer).arg(g_Au.CurrencySymbol()),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	auApp->BankGAWithdraw(amount, m_CapitolOpId);
	accept();
}

