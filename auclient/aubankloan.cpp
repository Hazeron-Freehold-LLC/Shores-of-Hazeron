//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubankloan.cpp,v 1.4 2022/02/19 18:05:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubankloan.h"
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

AuBankLoan::AuBankLoan(const AuId& opBd, QWidget* parent) : AuDialog(parent)
{
	m_CapitolOpId = opBd;
	setAuDlgIcon(QIcon(":data/bBank.png"));
	setAuDlgTitle("Bank Loan");

	double accountBalance = 0;
	double creditLimit = 0;
	double loanBalance = 0;
	int rep = 0;
	quint32 empireId = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		empireId = person->EmpireId();
		rep = person->Reputation();
		accountBalance = person->AccountBalance();
		creditLimit = person->CreditLimit();
		loanBalance = person->LoanBalance();
	}
	pView->UnlockSceneAtom();

	double availCredit = creditLimit - loanBalance;

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
	QLabel* lb3;
	QPushButton* pb0;
	if(!empireId)
	{
		lb0 = new QLabel(tr("<html>Your lack of any reputation or standing in any empire prevents us from doing business with you. "
					"Perhaps another time.</html>"));

		lb3 = 0;

		m_Amount = 0;

		pb0 = new QPushButton("Ok");
		connect(pb0, SIGNAL(clicked()), SLOT(accept()));
	}
	else if(creditLimit > 0)
	{
		lb0 = new QLabel(tr("<html>With your reputation of %L1, your credit limit is %L2%3.</html>")
				.arg(rep)
				.arg(creditLimit,0,'f',0)
				.arg(g_Au.CurrencySymbol()));

		if(availCredit > 0)
		{
			lb3 = new QLabel(tr("<html>Your available credit is %L2%3. "
						"We will loan money to you at 0% interest. "
						"You must repay this loan when you are able. "
						"How much will we be lending you today?</html>")
				.arg(availCredit,0,'f',0)
				.arg(g_Au.CurrencySymbol()));
			lb3->setWordWrap(true);

			QString ac = QString("%1").arg(round(availCredit), 0,'f',0);
			m_Amount = new QLineEdit(ac);
			m_Amount->setAlignment(Qt::AlignRight);

			pb0 = new QPushButton("Request Loan");
			connect(pb0, SIGNAL(clicked()), SLOT(BorrowFunds()));
		}
		else
		{
			lb3 = new QLabel(tr("<html>You have no available credit. "
						"While we appreciate your business, now is not a good time.</html>"));
			lb3->setWordWrap(true);

			m_Amount = 0;

			pb0 = new QPushButton("Ok");
			connect(pb0, SIGNAL(clicked()), SLOT(accept()));
		}
	}
	else
	{
		lb0 = new QLabel(tr("<html>Your negative reputation prevents us from doing business with you. "
					"Perhaps another time.</html>"));

		lb3 = 0;

		m_Amount = 0;

		pb0 = new QPushButton("Ok");
		connect(pb0, SIGNAL(clicked()), SLOT(accept()));
	}
	lb0->setWordWrap(true);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbicon, row,0, 2,1);
	lay->addWidget(lb1, row,1);
	row++;
	lay->addWidget(lb2, row,1);
	row++;
	lay->addWidget(lb0, row,0, 1,2);
	row++;
	if(lb3)
	{
		lay->addWidget(lb3, row,0, 1,2);
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

AuBankLoan::~AuBankLoan()
{
}

QSize AuBankLoan::sizeHint() const
{
	return QSize(500, 250);
}

void AuBankLoan::BorrowFunds()
{
	// Determine the amount to be deposited.
	double amount = QVariant(m_Amount->text()).toDouble();

	double creditLimit = 0;
	double loanBalance = 0;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		creditLimit = person->CreditLimit();
		loanBalance = person->LoanBalance();
	}
	pView->UnlockSceneAtom();

	double availCredit = creditLimit - loanBalance;
	if(amount < 1)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("You cannot borrow less than 1%1.").arg(g_Au.CurrencySymbol()),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	if(availCredit <= 0)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("You have no credit available to borrow any money."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	if(amount > availCredit)
	{
		QMessageBox mb(g_Au.ApplicationName(),
				tr("You have insufficient credit available to borrow that much money."),
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, this);
		auApp->Frame()->ExecMessageBox(mb);
		return;
	}

	// Borrow the money.
	auApp->BankBorrow(amount, m_CapitolOpId);
	accept();
}
