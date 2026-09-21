//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloadcolonists.cpp,v 1.2 2022/02/19 18:16:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auloadcolonists.h"
//
#include "auclientglobal.h"
#include <auroster.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>

AuLoadColonists::AuLoadColonists(unsigned int officers, unsigned int crew, unsigned int cryo, unsigned int passengers,
		unsigned int troops, bool isInVehicle, QWidget* parent) : AuDialog(parent)
{
	m_Crew = crew;
	m_Cryo = cryo;
	m_Officers = officers;
	m_Passengers = passengers;
	m_Troops = troops;

	setAuDlgTitle("Boarding");
	setAuDlgIcon(QIcon(":data/MsgLoadColonists.png"));

	// Layout the controls.
	QGridLayout* lay = new QGridLayout();
	int row = 0;

	if(!isInVehicle)
	{
		// Officers, Crew, and cryo berths can only be loaded into a spacecraft.
		// Officers, Crew, and cryo berths can not be loaded into a vehicle.
		if(m_Officers)
		{
			QLabel* lbO0 = new QLabel(tr("%1 officer berths are available.").arg(m_Officers), this);
			QLabel* lbO1 = new QLabel(tr("How many berths will you offer to officers?"), this);
			QPushButton* pbofferO = new QPushButton(QIcon(AuRosterEntry::BerthIcon(AuRosterEntry::OfficerHuman)),
					tr("Offer Officer Berths"), this);
			pbofferO->setToolTip(tr("Offer officer berths to officers on the current channel. "
						"A 2000%1 sign-on bonus is paid to each new officer from the ship's fund.<br>"
						"A 2000%1 wage is paid once per real day to each officer from the ship's fund.")
					.arg(g_Au.CurrencySymbol()));

			QLineEdit* leO = new QLineEdit(QVariant(m_Officers).toString(), this);
			leO->setObjectName("leO");
			leO->selectAll();
			connect(leO, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

			pbofferO->setAutoDefault(true);
			pbofferO->setDefault(true);
			pbofferO->setObjectName("pbofferO");
			connect(pbofferO, SIGNAL(clicked()), SLOT(OfferOfficersPressed()));

			lay->addWidget(lbO0, row, 0, 1,3);
			row++;
			lay->addWidget(lbO1, row, 0);
			lay->addWidget(leO, row, 1);
			lay->addWidget(pbofferO, row, 2);
			row++;
		}

		if(m_Crew)
		{
			QLabel* lbC0 = new QLabel(tr("%1 crew berths are available.").arg(QVariant(m_Crew).toString()), this);
			QLabel* lbC1 = new QLabel(tr("How many berths will you offer to crew?"), this);
			QPushButton* pbofferC = new QPushButton(QIcon(AuRosterEntry::BerthIcon(AuRosterEntry::Crew)),
					tr("Offer Crew Berths"), this);
			pbofferC->setToolTip(tr("Offer crew berths to crew on the current channel. "
						"A 1000%1 sign-on bonus is paid to each new crew member from the ship's fund.<br>"
						"A 1000%1 wage is paid once per real day to each crew member from the ship's fund.")
					.arg(g_Au.CurrencySymbol()));

			QLineEdit* leC = new QLineEdit(QVariant(m_Crew).toString(), this);
			leC->setObjectName("leC");
			leC->selectAll();
			connect(leC, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

			pbofferC->setAutoDefault(true);
			pbofferC->setDefault(true);
			pbofferC->setObjectName("pbofferC");
			connect(pbofferC, SIGNAL(clicked()), SLOT(OfferCrewPressed()));

			lay->addWidget(lbC0, row, 0, 1,3);
			row++;
			lay->addWidget(lbC1, row, 0);
			lay->addWidget(leC, row, 1);
			lay->addWidget(pbofferC, row, 2);
			row++;
		}
	}

	if(m_Passengers)
	{
		QLabel* lbP0;
		QLabel* lbP1;
		QPushButton* pbofferP;
		if(isInVehicle)
		{
			lbP0 = new QLabel(tr("%1 empty seats are available.").arg(QVariant(m_Passengers).toString()), this);
			lbP1 = new QLabel(tr("How many seats will you offer to passengers?"), this);
			pbofferP = new QPushButton(QIcon(AuRosterEntry::OccupantIcon(AuRosterEntry::PassengerHuman)), tr("Offer Seats"), this);
			pbofferP->setToolTip(tr("Offer empty seats to passengers on the current channel."));
		}
		else
		{
			lbP0 = new QLabel(tr("%1 stateroom berths are available.").arg(QVariant(m_Passengers).toString()), this);
			lbP1 = new QLabel(tr("How many berths will you offer to passengers?"), this);
			pbofferP = new QPushButton(QIcon(AuRosterEntry::BerthIcon(AuRosterEntry::PassengerHuman)),
					tr("Offer Stateroom Berths"), this);
			pbofferP->setToolTip(tr("Offer stateroom berths to passengers on the current channel."));
		}

		QLineEdit* leP = new QLineEdit(QVariant(m_Passengers).toString(), this);
		leP->setObjectName("leP");
		leP->selectAll();
		connect(leP, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		pbofferP->setAutoDefault(true);
		pbofferP->setDefault(true);
		pbofferP->setObjectName("pbofferP");
		connect(pbofferP, SIGNAL(clicked()), SLOT(OfferPassengersPressed()));

		lay->addWidget(lbP0, row, 0, 1,3);
		row++;
		lay->addWidget(lbP1, row, 0);
		lay->addWidget(leP, row, 1);
		lay->addWidget(pbofferP, row, 2);
		row++;
	}

	if(m_Troops)
	{
		QLabel* lbT0;
		QLabel* lbT1;
		QPushButton* pbofferT;
		if(isInVehicle)
		{
			lbT0 = new QLabel(tr("%1 empty seats are available.").arg(QVariant(m_Troops).toString()), this);
			lbT1 = new QLabel(tr("How many seats will you offer to troops?"), this);
			pbofferT = new QPushButton(QIcon(AuRosterEntry::OccupantIcon(AuRosterEntry::Troop)), tr("Offer Seats"), this);
			pbofferT->setToolTip(tr("Offer empty seats to troops on the current channel.<br>"
						"A 1000%1 sign-on bonus is paid to each new troop from the ship's fund.<br>"
						"A 1000%1 wage is paid once per real day to each troop from the ship's fund.")
					.arg(g_Au.CurrencySymbol()));
		}
		else
		{
			lbT0 = new QLabel(tr("%1 troop berths are available.").arg(QVariant(m_Troops).toString()), this);
			lbT1 = new QLabel(tr("How many berths will you offer to troops?"), this);
			pbofferT = new QPushButton(QIcon(AuRosterEntry::BerthIcon(AuRosterEntry::Troop)), tr("Offer Troop Berths"), this);
			pbofferT->setToolTip(tr("Offer troop berths to troops on the current channel.<br>"
						"A 1000%1 sign-on bonus is paid to each new troop from the ship's fund.<br>"
						"A 1000%1 wage is paid once per real day to each troop from the ship's fund.")
					.arg(g_Au.CurrencySymbol()));
		}

		QLineEdit* leT = new QLineEdit(QVariant(m_Troops).toString(), this);
		leT->setObjectName("leT");
		connect(leT, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		pbofferT->setObjectName("pbofferT");
		connect(pbofferT, SIGNAL(clicked()), SLOT(OfferTroopsPressed()));

		lay->addWidget(lbT0, row, 0, 1,3);
		row++;
		lay->addWidget(lbT1, row, 0);
		lay->addWidget(leT, row, 1);
		lay->addWidget(pbofferT, row, 2);
		row++;
	}

	if(m_Cryo)
	{
		QLabel* lbC0 = new QLabel(tr("%1 cryo berths are available.").arg(QVariant(m_Cryo).toString()), this);
		QLabel* lbC1 = new QLabel(tr("How many berths will you offer to cryo travellers?"), this);
		QPushButton* pbofferC = new QPushButton(QIcon(":data/CryoPassenger.png"), tr("Offer Cryo Berths"), this);
		pbofferC->setToolTip(tr("Offer cryo berths to cryo travellers on the current channel."));

		QLineEdit* leC = new QLineEdit(QVariant(m_Cryo).toString(), this);
		leC->setObjectName("leCryo");
		leC->selectAll();
		connect(leC, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		pbofferC->setAutoDefault(true);
		pbofferC->setDefault(true);
		pbofferC->setObjectName("pbofferCryo");
		connect(pbofferC, SIGNAL(clicked()), SLOT(OfferCryoPressed()));

		lay->addWidget(lbC0, row, 0, 1,3);
		row++;
		lay->addWidget(lbC1, row, 0);
		lay->addWidget(leC, row, 1);
		lay->addWidget(pbofferC, row, 2);
		row++;
	}

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuLoadColonists::~AuLoadColonists()
{
}

void AuLoadColonists::EnableControls()
{
	{
		unsigned int count = 0;
		QLineEdit* le = findChild<QLineEdit*>("leO");
		if(le)
			count = QVariant(le->text()).toInt();

		bool enableOk = count > 0 && count <= m_Officers;
		QPushButton* pbok = findChild<QPushButton*>("pbofferO");
		if(pbok)
			pbok->setEnabled(enableOk);
	}
	{
		unsigned int count = 0;
		QLineEdit* le = findChild<QLineEdit*>("leC");
		if(le)
			count = QVariant(le->text()).toInt();

		bool enableOk = count > 0 && count <= m_Crew;
		QPushButton* pbok = findChild<QPushButton*>("pbofferC");
		if(pbok)
			pbok->setEnabled(enableOk);
	}
	{
		unsigned int count = 0;
		QLineEdit* le = findChild<QLineEdit*>("leCryo");
		if(le)
			count = QVariant(le->text()).toInt();

		bool enableOk = count > 0 && count <= m_Cryo;
		QPushButton* pbok = findChild<QPushButton*>("pbofferCryo");
		if(pbok)
			pbok->setEnabled(enableOk);
	}
	{
		unsigned int count = 0;
		QLineEdit* le = findChild<QLineEdit*>("leP");
		if(le)
			count = QVariant(le->text()).toInt();

		bool enableOk = count > 0 && count <= m_Passengers;
		QPushButton* pbok = findChild<QPushButton*>("pbofferP");
		if(pbok)
			pbok->setEnabled(enableOk);
	}
	{
		unsigned int count = 0;
		QLineEdit* le = findChild<QLineEdit*>("leT");
		if(le)
			count = QVariant(le->text()).toInt();

		bool enableOk = count > 0 && count <= m_Troops;
		QPushButton* pbok = findChild<QPushButton*>("pbofferT");
		if(pbok)
			pbok->setEnabled(enableOk);
	}
}

void AuLoadColonists::OfferCrewPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leC");
	if(le)
	{
		m_Crew = QVariant(le->text()).toInt();
		m_Cryo = 0;
		m_Officers = 0;
		m_Passengers = 0;
		m_Troops = 0;
	}
	accept();
}

void AuLoadColonists::OfferCryoPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leCryo");
	if(le)
	{
		m_Crew = 0;
		m_Cryo = QVariant(le->text()).toInt();
		m_Officers = 0;
		m_Passengers = 0;
		m_Troops = 0;
	}
	accept();
}

void AuLoadColonists::OfferOfficersPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leO");
	if(le)
	{
		m_Crew = 0;
		m_Cryo = 0;
		m_Officers = QVariant(le->text()).toInt();
		m_Passengers = 0;
		m_Troops = 0;
	}
	accept();
}

void AuLoadColonists::OfferPassengersPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leP");
	if(le)
	{
		m_Crew = 0;
		m_Cryo = 0;
		m_Officers = 0;
		m_Passengers = QVariant(le->text()).toInt();
		m_Troops = 0;
	}
	accept();
}

void AuLoadColonists::OfferTroopsPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leT");
	if(le)
	{
		m_Crew = 0;
		m_Cryo = 0;
		m_Officers = 0;
		m_Passengers = 0;
		m_Troops = QVariant(le->text()).toInt();
	}
	accept();
}
