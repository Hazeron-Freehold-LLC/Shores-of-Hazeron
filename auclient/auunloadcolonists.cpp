//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunloadcolonists.cpp,v 1.2 2022/02/19 18:17:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auunloadcolonists.h"
//
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>

AuUnloadColonists::AuUnloadColonists(unsigned int cryo, unsigned int passengers, unsigned int troops, bool isInVehicle,
		QWidget* parent) : AuDialog(parent)
{
	m_Cryo = cryo;
	m_Passengers = passengers;
	m_Troops = troops;
	if((m_Cryo || m_Passengers) && m_Troops)
		setAuDlgTitle("Debark Passengers or Troops");
	else if(m_Cryo || m_Passengers)
		setAuDlgTitle("Debark Passengers");
	else
		setAuDlgTitle("Debark Troops");
	setAuDlgIcon(QIcon(":/data/MsgUnloadColonists.png"));

	// Layout the controls.
	QGridLayout* lay = new QGridLayout();
	int row = 0;

	if(m_Passengers)
	{
		QLabel* lb0 = new QLabel(tr("%1 passengers are aboard.").arg(QVariant(m_Passengers).toString()));

		QLabel* lb1;
		if(isInVehicle)
			lb1 = new QLabel(tr("How many passengers will debark the vehicle?"));
		else
			lb1 = new QLabel(tr("How many passengers will debark the ship?"));

		QLineEdit* le = new QLineEdit(QVariant(m_Passengers).toString());
		le->setObjectName("leP");
		le->selectAll();
		connect(le, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		QPushButton* pbok = new QPushButton(QIcon(":data/Passenger.png"), tr("Debark Passengers"));
		pbok->setAutoDefault(true);
		pbok->setDefault(true);
		pbok->setObjectName("pbofferP");
		connect(pbok, SIGNAL(clicked()), SLOT(OfferPassengersPressed()));
		if(isInVehicle)
			pbok->setToolTip(tr("Unload passengers from the vehicle."));
		else
			pbok->setToolTip(tr("Unload passengers from the ship."));

		lay->addWidget(lb0, row, 0, 1,3);
		row++;
		lay->addWidget(lb1, row, 0);
		lay->addWidget(le, row, 1);
		lay->addWidget(pbok, row, 2);
		row++;
	}

	if(m_Troops)
	{
		QLabel* lb0 = new QLabel(tr("%1 troops are aboard.").arg(QVariant(m_Troops).toString()));

		QLabel* lb1;
		if(isInVehicle)
			lb1 = new QLabel(tr("How many troops will debark the vehicle?"));
		else
			lb1 = new QLabel(tr("How many troops will debark the ship?"));

		QLineEdit* le = new QLineEdit(QVariant(m_Troops).toString());
		le->setObjectName("leT");
		connect(le, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		QPushButton* pbok = new QPushButton(QIcon(":data/Troop.png"), tr("Debark Troops"));
		pbok->setObjectName("pbofferT");
		connect(pbok, SIGNAL(clicked()), SLOT(OfferTroopsPressed()));
		if(isInVehicle)
			pbok->setToolTip(tr("Unload troops from the vehicle."));
		else
			pbok->setToolTip(tr("Unload troops from the ship."));

		lay->addWidget(lb0, row, 0, 1,3);
		row++;
		lay->addWidget(lb1, row, 0);
		lay->addWidget(le, row, 1);
		lay->addWidget(pbok, row, 2);
		row++;
	}

	if(m_Cryo && !isInVehicle)
	{
		QLabel* lb0 = new QLabel(tr("%L1 cryo travellers are aboard.").arg(m_Cryo));

		QLabel* lb1 = new QLabel(tr("How many cryo travellers will debark the ship?"));

		QLineEdit* le = new QLineEdit(QVariant(m_Cryo).toString());
		le->setObjectName("leCryo");
		le->selectAll();
		connect(le, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		QPushButton* pbok = new QPushButton(QIcon(":data/CryoPassenger.png"), tr("Debark Cryo Travellers"));
		pbok->setObjectName("pbofferCryo");
		connect(pbok, SIGNAL(clicked()), SLOT(OfferCryoPressed()));
		pbok->setToolTip(tr("Unload cryo travellers from the ship."));

		lay->addWidget(lb0, row, 0, 1,3);
		row++;
		lay->addWidget(lb1, row, 0);
		lay->addWidget(le, row, 1);
		lay->addWidget(pbok, row, 2);
		row++;
	}

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuUnloadColonists::~AuUnloadColonists()
{
}

void AuUnloadColonists::EnableControls()
{
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

void AuUnloadColonists::OfferCryoPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leCryo");
	if(le)
	{
		m_Cryo = QVariant(le->text()).toInt();
		m_Passengers = 0;
		m_Troops = 0;
	}
	accept();
}

void AuUnloadColonists::OfferPassengersPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leP");
	if(le)
	{
		m_Cryo = 0;
		m_Passengers = QVariant(le->text()).toInt();
		m_Troops = 0;
	}
	accept();
}

void AuUnloadColonists::OfferTroopsPressed()
{
	QLineEdit* le = findChild<QLineEdit*>("leT");
	if(le)
	{
		m_Cryo = 0;
		m_Passengers = 0;
		m_Troops = QVariant(le->text()).toInt();
	}
	accept();
}
