//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftaccount.cpp,v 1.10 2022/05/04 21:47:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftaccount.h"
//
#include "auchristendialog.h"
#include "auapp.h"
#include <auglobal.h>
#include "auframe.h"
#include <auroster.h>
#include "auview.h"
#include <dagalaxy.h>
#include <daperson.h>
#include <dasector.h>
#include <dgdata.h>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>

void AuSpacecraftAccountDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftAccountUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftAccount::AuSpacecraftAccount(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;

	QLabel* lbSendFunds = new QLabel(tr("Transfer Amount"));
	QLabel* lbShipFunds = new QLabel(tr("Ship Account"));
	QLabel* lbPlayerFunds = new QLabel(tr("Your Account"));
	QLabel* lbShipTaxFunds = new QLabel(tr("Ship Tax Account"));

	lefunds = new QLineEdit();
	lefunds->setToolTip(tr("Funds to deposit or withdraw from the ship's account."));
	lefunds->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	float defTx = 1000.;	// Default amount to show in transfer funds box.
	QString defTrans = QString("%1").arg(defTx);
	lefunds->setText(defTrans);
	connect(lefunds, SIGNAL(textChanged(const QString&)), SLOT(UpdateText()));

	leshipfunds = new QLineEdit();
	leshipfunds->setToolTip(tr("<b>Ship Account Balance</b><br>"
				"Account balance of the ship you are aboard. "
				"Ship's account is used for purchasing cargo, fuel and repairs when trading with cities and other spacecraft.<br>"
				"Only a person in the ship's chain-of-command may view the ship's account balance."));
	leshipfunds->setEnabled(false);
	leshipfunds->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	leshiptaxfunds = new QLineEdit();
	leshiptaxfunds->setToolTip(tr("<html><b>Fleet Tax Account Balance</b><br>"
				"Fleet tax is only collected by fleet spacecraft, not by company spacecraft. "
				"Fleet tax is collected like sales tax on all cargo sales and passenger transport fees. "
				"The tax collected is stored in the ship's fleet tax account.<br>"
				"%1 The fleet tax balance is transferred to a bank at the next empire city that is hailed that has a bank.<br>"
				"%1 Only a person in the ship's chain-of-command may view the fleet tax account balance."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	leshiptaxfunds->setEnabled(false);
	leshiptaxfunds->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	leplayerfunds = new QLineEdit();
	leplayerfunds->setToolTip(tr("Your account balance."));
	leplayerfunds->setEnabled(false);
	leplayerfunds->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QImage image(":data/hazBlackSliderArrow.png");
	QSize size(16, 16);
	QImage scaledImage = image.scaled( size, Qt::IgnoreAspectRatio);
	QPixmap pix = QPixmap::fromImage(scaledImage);

	pbdeposit = new QPushButton(tr("Deposit Funds"));
	pbdeposit->setIcon(QIcon(pix));
	pbdeposit->setToolTip(tr("<b>Deposit Funds</b><br>"
				"Transfers funds to the ship's account from your personal account.<br>"
				"Any player may deposit funds. You do not have to be in the ship's chain-of-command."));
	connect(pbdeposit, SIGNAL(clicked()), SLOT(SendFunds()));

	QTransform arrowRotate;
	arrowRotate.rotate(180);
	scaledImage = scaledImage.transformed( arrowRotate, Qt::FastTransformation);
	pix = QPixmap::fromImage(scaledImage);

	pbwithdraw = new QPushButton(tr("Withdraw Funds"));
	pbwithdraw->setIcon(QIcon(pix));
	pbwithdraw->setToolTip(tr("<b>Withdraw Funds</b><br>"
				"Transfers funds from the ship's account to your personal account.<br>"
				"Only a person in the ship's chain-of-command may withdraw funds."));
	connect(pbwithdraw, SIGNAL(clicked()), SLOT(ReceiveFunds()));

	// Funds transfer buttons go in a grid.
	int row = 0;
	QGridLayout* flay = new QGridLayout;
	flay->addWidget(lbShipTaxFunds, row,0);
	flay->addWidget(leshiptaxfunds, row,1);
	row++;
	flay->addWidget(lbShipFunds, row,0);
	flay->addWidget(leshipfunds, row,1);
	row++;
	flay->addWidget(lbSendFunds, row,0, 2,1);
	flay->addWidget(lefunds, row,1, 2,1);
	flay->addWidget(pbdeposit, row,2);
	row++;
	flay->addWidget(pbwithdraw, row,2);
	row++;
	flay->addWidget(lbPlayerFunds, row,0);
	flay->addWidget(leplayerfunds, row,1);
	row++;
	flay->setRowStretch(row,1);
	flay->setMargin(0);
	flay->setSpacing(2);

	QWidget* wf = new QWidget();
	wf->setLayout(flay);

	QScrollArea* scroll = new QScrollArea;
	scroll->setWidget(wf);
	scroll->setWidgetResizable(true);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(scroll);
	lay->setMargin(0);
	lay->setSpacing(2);
	setLayout(lay);

	connect(this, SIGNAL(SendShipFunds(double, AuId)), auApp, SLOT(SendShipFunds(double, AuId)));
	connect(this, SIGNAL(ReceiveShipFunds(double, AuId)), auApp, SLOT(ReceiveShipFunds(double, AuId)));
}

AuSpacecraftAccount::~AuSpacecraftAccount()
{
}

void AuSpacecraftAccount::UpdateText()
{
	// Update Funds to send
	double count = QVariant(lefunds->text()).toDouble();
	if(count <= 0 && lefunds->text().count() > 15)
		pbdeposit->setEnabled(false);
	else
		pbdeposit->setEnabled(true);
}

void AuSpacecraftAccount::EnableControls()
{
	// Determine if the person is in the chain-of-command of the ship.
	AuView* pView = auApp->Frame()->View();
	bool inCommand = pView->IsPersonInCommand();		// also true for admins

	// Enable/disable controls.
	pbwithdraw->setEnabled(inCommand);
}

void AuSpacecraftAccount::SendFunds()
{
	double amount = lefunds->text().toDouble();
	if(amount > 0)
	{
		AuId shipId;
		bool emitit = false;
		AuView* pView = auApp->Frame()->View();
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
		{
			const DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
			if(ship)
			{
				shipId = ship->Id();
				emitit = true;
			}
		}
		pView->UnlockSceneAtom();
		if(emitit)
			emit SendShipFunds(amount, shipId);
		else
			QMessageBox::warning(this, g_Au.ApplicationName(),
					tr("You are not aboard a spacecraft. Only a person aboard a ship can\ndeposit funds."),
					QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	}
}

void AuSpacecraftAccount::ReceiveFunds()
{
	double amount = lefunds->text().toDouble();
	if(amount > 0)
	{
		AuId shipId;
		bool emitit = false;
		bool isOfficer;
		AuView* pView = auApp->Frame()->View();
		if(pView->IsPersonInCommand())
		{
			isOfficer = true;
			DaPerson* person;
			pView->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
				if(ship)
				{
					shipId = ship->Id();
					emitit = true;
				}
			}
			pView->UnlockSceneAtom();
		}
		else
			isOfficer = false;

		if(emitit)
			emit ReceiveShipFunds(amount, shipId);
		else
		{
			// Could not withdraw the funds.
			if(!isOfficer)
				QMessageBox::warning(this, g_Au.ApplicationName(),
						tr("You are not in the ship's chain-of-command. "
							"Only a person in the ship's chain-of-command can withdraw funds."),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
			else
				QMessageBox::warning(this, g_Au.ApplicationName(),
						tr("You are not aboard a spacecraft. "
							"Only a person a the ship's chain-of-command can withdraw funds."),
						QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void AuSpacecraftAccount::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuSpacecraftAccount::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// We must use the scene atom because the root ship atom never does the extra work of analyzing its design.
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// List player's current funds available
		if(person->AccountBalance() > 0)
			leplayerfunds->setText(QString("%L1").arg(person->AccountBalance(),0,'f',0) + g_Au.CurrencySymbol());
		else
			leplayerfunds->setText("0");

		// List items that relate to the ship the person is aboard.
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// List ship's current funds available to officers
			if(pView->IsPersonInCommand())
			{
				leshipfunds->setText(QString("%L1%2").arg(ship->AccountBalance(),0,'f',0).arg(g_Au.CurrencySymbol()));
				leshiptaxfunds->setText(QString("%L1%2").arg(ship->AccountBalanceSubsidyTax(),0,'f',0).arg(g_Au.CurrencySymbol()));
			}
			else
			{
				leshipfunds->setText(g_Au.MiddleDotSymbol());
				leshiptaxfunds->setText(g_Au.MiddleDotSymbol());
			}
		}
		else
		{
			// Not aboard a spacecraft.
			leshipfunds->setText("");
			leshiptaxfunds->setText("");
		}

	}
	pView->UnlockSceneAtom();
	EnableControls();
}

QSize AuSpacecraftAccount::sizeHint() const
{
	return QSize(500,250);
}

