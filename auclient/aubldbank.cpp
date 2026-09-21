//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbank.cpp,v 1.5 2022/02/19 18:08:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldbank.h"
//
#include "auapp.h"
#include "aubankloan.h"
#include "aubankpay.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "augadeposit.h"
#include "augawithdraw.h"
#include "auview.h"
#include "daperson.h"
#include "daworld.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldBank::AuBldBank(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	// Government Account
	QGroupBox* gaGroup = new QGroupBox(tr("Bank System"));
	{
		QLabel* gaLabel = new QLabel(tr("<html>The bank system controls all money in the city. "
					"Only a person in the chain of command of the city may access the bank system."
					"</html>"));
		gaLabel->setWordWrap(true);

		QPushButton* reqBalButton = new QPushButton(tr("Request Balance"));
		reqBalButton->setToolTip("<html><b>Request Balance</b><br>"
				"Requests the current balance of the bank system. "
				"The balance is reported on the Government comm channel.</html>");
		connect(reqBalButton, SIGNAL(clicked()), SLOT(GARequestBalance()));

		QPushButton* depositButton = new QPushButton(tr("Deposit Funds"));
		depositButton->setToolTip("<html><b>Deposit Funds</b><br>"
				"Deposits funds into the bank system. "
				"Funds are transferred from your personal account. "
				"The result of the transaction is sent on the Government comm channel.</html>");
		connect(depositButton, SIGNAL(clicked()), SLOT(GADepositFunds()));

		QPushButton* withdrawButton = new QPushButton(tr("Withdraw Funds"));
		withdrawButton->setToolTip("<html><b>Withdraw Funds</b><br>"
				"Withdraws funds from the bank system. "
				"Funds are transferred to your personal account. "
				"The result of the transaction is sent on the Government comm channel.</html>");
		connect(withdrawButton, SIGNAL(clicked()), SLOT(GAWithdrawFunds()));

		QGridLayout* galay = new QGridLayout;
		int row=0;
		galay->addWidget(gaLabel, row,0, 1,3);
		row++;
		galay->addWidget(reqBalButton, row,0, 1,1);
		galay->addWidget(depositButton, row,1, 1,1);
		galay->addWidget(withdrawButton, row,2, 1,1);
		row++;
		galay->setMargin(12);
		galay->setSpacing(2);
		gaGroup->setLayout(galay);
	}

	QGroupBox* gdGroup = new QGroupBox(tr("Government Debt"));
	{
		m_PlayerOwedEdit = new QLineEdit();
		m_PlayerOwedEdit->setEnabled(false);
		m_PlayerOwedEdit->setAlignment(Qt::AlignRight);
		m_PlayerOwedEdit->setToolTip(tr("<html><b>Government Debt</b><br>"
					"Government debt is the amount of money owed a player by their empire; it accrues in various ways. "
					"Empire rewards are credited to a player's government debt account when the reward is earned. "
					"Cities will sometimes offer to purchase crucial supplies with government debt; "
					"those purchases are credited to a player's government debt account.<br><br>"
					"Government debt is canceled when a player's empire changes for any reason."
					"</html>"));

		QPushButton* pbClaim = new QPushButton(tr("Redeem Government Debt"));
		pbClaim->setToolTip(tr("<html><b>Redeem Government Debt</b><br>"
					"Government debt is collected at banks of your empire. "
					"Stand within the area of a bank building and press this button to request payment for all outstanding "
					"government debt. "
					"The bank will pay as much of the debt as possible from the bank system funds it has available."
					"</html>"));
		connect(pbClaim, SIGNAL(clicked()), SLOT(ClaimRewards()));

		QGridLayout* gdlay = new QGridLayout;
		int row=0;
		gdlay->addWidget(m_PlayerOwedEdit, row,0, 1,1);
		gdlay->addWidget(pbClaim, row,1, 1,1);
		row++;
		gdlay->setMargin(12);
		gdlay->setSpacing(2);
		gdGroup->setLayout(gdlay);
	}

	QGroupBox* lGroup = new QGroupBox(tr("Loans"));
	{
		m_PlayerLoansEdit = new QLineEdit();
		m_PlayerLoansEdit->setEnabled(false);
		m_PlayerLoansEdit->setAlignment(Qt::AlignRight);
		m_PlayerLoansEdit->setToolTip(tr("<html><b>Loan Debt</b><br>"
					"Loan debt is the amount of money a player owes to the bank. "
					"Loan debt can be repaid at any bank."
					"</html>"));

		QPushButton* pbBorrow = new QPushButton(tr("Request Loan"));
		pbBorrow->setToolTip(tr("<html><b>Request Loan</b><br>"
					"Borrows money from the bank."
					"</html>"));
		connect(pbBorrow, SIGNAL(clicked()), SLOT(RequestLoan()));

		QPushButton* pbPay = new QPushButton(tr("Pay Debt"));
		pbPay->setToolTip(tr("<html><b>Pay Debt</b><br>"
					"Pays money toward your debt. "
					"Your debt can be repaid at any bank."
					"</html>"));
		connect(pbPay, SIGNAL(clicked()), SLOT(PayDebt()));

		QGridLayout* llay = new QGridLayout;
		int row=0;
		llay->addWidget(m_PlayerLoansEdit, row,0, 1,1);
		llay->addWidget(pbBorrow, row,1, 1,1);
		llay->addWidget(pbPay, row,2, 1,1);
		row++;
		llay->setMargin(12);
		llay->setSpacing(2);
		lGroup->setLayout(llay);
	}

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lGroup, row++,0);
	lay->addWidget(gaGroup, row++,0);
	lay->addWidget(gdGroup, row++,0);

	lay->setRowStretch(row++,1);

	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldBank::~AuBldBank()
{
}

void AuBldBank::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuBldBank::TimerUpdate()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Lock the root and find the scene atom.
	if(m_PlayerOwedEdit)
	{
		AuView* pView = auApp->Frame()->View();
		DaPerson* person;
		pView->LockSceneAtom(&person);
		double govtDebt;
		double loanDebt;
		if(person)
		{
			govtDebt = person->GetReward();
			loanDebt = person->LoanBalance();
		}
		else
		{
			govtDebt = 0;
			loanDebt = 0;
		}
		pView->UnlockSceneAtom();
		m_PlayerLoansEdit->setText(QString("%L1%2").arg(loanDebt, 0,'f',0).arg(g_Au.CurrencySymbol()));
		m_PlayerOwedEdit->setText(QString("%L1%2").arg(govtDebt, 0,'f',0).arg(g_Au.CurrencySymbol()));
	}
}


void AuBldBank::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}

void AuBldBank::ClaimRewards()
{
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && !person->BdId().IsNull())
	{
		if(person->InCityEmpire() != person->EmpireId())
		{
			// Tell the person why they could not claim their reward
			QString sText(tr("I must go to a bank of my empire to claim rewards."));
			AuCommThought msg(sText, person->Id(), person->Name());
			auApp->SendChatMessage(msg);
		}
		else if(person->GetReward() > 0)
			auApp->ClaimReward(m_CapitolOpId);
	}
	pView->UnlockSceneAtom();
}

void AuBldBank::GADepositFunds()
{
	AuFrame* pFrame = auApp->Frame();
	AuGADeposit depositDlg(m_CapitolOpId, pFrame);
	auApp->Frame()->ExecDialog(depositDlg);
}

void AuBldBank::GARequestBalance()
{
	auApp->BankGARequestBalance(m_CapitolOpId);
}

void AuBldBank::GAWithdrawFunds()
{
	AuFrame* pFrame = auApp->Frame();
	AuGAWithdraw withdrawDlg(m_CapitolOpId, pFrame);
	auApp->Frame()->ExecDialog(withdrawDlg);
}

void AuBldBank::RequestLoan()
{
	AuFrame* pFrame = auApp->Frame();
	AuBankLoan loanDlg(m_CapitolOpId, pFrame);
	auApp->Frame()->ExecDialog(loanDlg);
}

void AuBldBank::PayDebt()
{
	AuFrame* pFrame = auApp->Frame();
	AuBankPay payDlg(m_CapitolOpId, pFrame);
	auApp->Frame()->ExecDialog(payDlg);
}
