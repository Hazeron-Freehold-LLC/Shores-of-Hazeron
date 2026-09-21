//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubankpay.cpp,v 1.4 2022/02/19 18:05:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubankpay.h"
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

AuBankPay::AuBankPay(const AuId& opBd, QWidget* parent) : AuDialog(parent)
{
	m_CapitolOpId = opBd;
	setAuDlgIcon(QIcon(":data/bBank.png"));
	setAuDlgTitle("Bank Payment");

	double accountBalance = 0;
	double loanBalance = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		accountBalance = person->AccountBalance();
		loanBalance = person->LoanBalance();
	}
	pView->UnlockSceneAtom();
	double canPay = loanBalance;
	if(canPay > accountBalance)
		canPay = accountBalance;

	QLabel* lbicon = new QLabel;
	lbicon->setPixmap(QPixmap(":data/bBank.png"));

	QLabel* lb1;
	if(accountBalance == 0)
		lb1 = new QLabel(tr("You have no money."));
	else
		lb1 = new QLabel(tr("You have %L1%2 in cash.").arg(accountBalance,0,'f',0).arg(g_Au.CurrencySymbol()));

	QLabel* lb2;
	if(loanBalance == 0)
		lb2 = new QLabel(tr("You have no loan debt."));
	else
		lb2 = new QLabel(tr("You owe the bank %L1%2.").arg(loanBalance,0,'f',0).arg(g_Au.CurrencySymbol()));

	QLabel* lb0;
	QPushButton* pb0;
	if(canPay > 0)
	{
		lb0 = new QLabel(tr("<html>How much will you be paying toward your loan today?</html>"));
		lb0->setWordWrap(true);

		QString ac = QString("%1").arg(round(canPay), 0,'f',0);
		m_Amount = new QLineEdit(ac);
		m_Amount->setAlignment(Qt::AlignRight);

		pb0 = new QPushButton("Make Payment");
		connect(pb0, SIGNAL(clicked()), SLOT(MakePayment()));
	}
	else
	{
		lb0 = 0;
		m_Amount = 0;

		pb0 = new QPushButton("Ok");
		connect(pb0, SIGNAL(clicked()), SLOT(accept()));
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbicon, row,0, 2,1);
	lay->addWidget(lb1, row,1);
	row++;
	lay->addWidget(lb2, row,1);
	row++;
	if(lb0)
	{
		lay->addWidget(lb0, row,0, 1,2);
		row++;
	}
	if(m_Amount)
	{
		lay->addWidget(m_Amount, row,0, 1,2);
		row++;
	}
	lay->addWidget(pb0, row,1);
	row++;
	lay->setColumnStretch(1,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBankPay::~AuBankPay()
{
}

QSize AuBankPay::sizeHint() const
{
	return QSize(250,100);
}

void AuBankPay::MakePayment()
{
	// Determine the amount to be deposited.
	double amount = QVariant(m_Amount->text()).toDouble();

	double accountBalance = 0;
	double loanBalance = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		accountBalance = person->AccountBalance();
		loanBalance = person->LoanBalance();
	}
	pView->UnlockSceneAtom();

	if(amount > accountBalance)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("You do not have that much money."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	if(amount > loanBalance)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("You do not owe that much money to the bank."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	auApp->BankPay(amount, m_CapitolOpId);
	accept();
}
