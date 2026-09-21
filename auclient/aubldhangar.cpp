//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldhangar.cpp,v 1.5 2022/02/19 18:09:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldhangar.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daatomlist.h"
#include <daperson.h>
#include <daworld.h>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldHangar::AuBldHangar(QWidget* parent) : QWidget(parent)
{
	setObjectName("BldHangar");
	m_StoredShipsInfo = new QLineEdit(tr("Waiting for hangar storage status..."), this);
	m_StoredShipsInfo->setReadOnly(true);
	m_StoredShipsInfo->setToolTip(tr("<html>Information about all spacecraft stored in this hangar.</html>"));

	m_StoredShipInfo = new QLineEdit(tr("Waiting for spacecraft storage status..."), this);
	m_StoredShipInfo->setReadOnly(true);
	m_StoredShipInfo->setToolTip(tr("<html>Information about the spacecraft you stored in this hangar.</html>"));

	m_Launch = new QPushButton(tr("Launch"));
	m_Launch->setToolTip(tr("<html><b>Launch</b><br>"
				"Launches the spacecraft that was mothballed in this hangar by you.<br>"
				"%1 Building empire must not be enemy toward avatar's empire.<br>"
				"%1 Spacecraft will launch into orbit above this hangar."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_Launch, SIGNAL(clicked()), SLOT(Launch()));
	m_Launch->setEnabled(false);	// Start disabled until we get good info.

	m_TakeCommand = new QCheckBox(tr("Take Captain's Berth"), this);
	m_TakeCommand->setToolTip(tr("<html><b>Take Captain's Berth</b><br>"
				"When checked, the Captain's berth is assigned to your avatar when the spacecraft is launched. "
				"Then your avatar can recall home, to board the spacecraft after it launches."
				"</html>"));
	m_TakeCommand->setChecked(true);
	m_TakeCommand->setEnabled(false);

	m_Mothball = new QPushButton(tr("Mothball"));
	m_Mothball->setToolTip(tr("<html><b>Mothball</b><br>"
				"Stores a spacecraft into your private space in this hangar. "
				"Select a nearby spacecraft for mothball storage using the combo box.<br>"
				"%1 Building empire must not be enemy toward avatar's empire.<br>"
				"%1 Each avatar can store one spacecraft per hangar.<br>"
				"%1 Any number of avatars can store spacecraft at the same hangar.<br>"
				"%1 Spacecraft must be on the ground.<br>"
				"%1 Spacecraft must be rigged for repairs.<br>"
				"%1 Spacecraft must be empty of all players and officers. Crew and troops may remain aboard.<br>"
				"%1 You must be in the ship's chain-of-command or the captain's berth must be available to you.<br>"
				"%1 <b>Mothballed spacecraft are lost if their hangar is destroyed. "
				"Battle damage does not harm the ships stored in the hanger.</b>"
				"</html").arg(g_Au.MiddleDotSymbol()));
	connect(m_Mothball, SIGNAL(clicked()), SLOT(Mothball()));

	m_NearbyShips = new QComboBox(this);
	m_NearbyShips->setToolTip(tr("<html><b>Spacecraft Nearby</b><br>"
				"Lists spacecraft nearby. "
				"Select a spacecraft for mothball storage.<br>"
				"%1 Nearby spacecraft are listed regardless of your authority to mothball them."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	// Fill combo box with all spacecraft nearby.
	ListNearbyShips();

	// Build the layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_StoredShipsInfo, row++,0, 1,2);

	lay->addWidget(m_StoredShipInfo, row++,0, 1,2);

	lay->addWidget(m_TakeCommand, row,0);
	lay->addWidget(m_Launch, row,1);
	row++;

	lay->addWidget(m_NearbyShips, row++,0, 1,2);

	lay->addWidget(m_Mothball, row++,1);

	lay->setRowStretch(row++,1);

	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldHangar::~AuBldHangar()
{
}

void AuBldHangar::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	Query();
	ListNearbyShips();
}

void AuBldHangar::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widget now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QGridLayout* lay = (QGridLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
	
	m_StoredShipsInfo->setText("");
	m_StoredShipInfo->setText("");
}

void AuBldHangar::ListNearbyShips()
{
	m_NearbyShips->clear();

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			DaAtomList ships;
			planet->FindVesselsRecursive(ships);

			for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
			{
				const DaVessel* ship = dynamic_cast<const DaVessel*>(*si);
				if(!ship)
					continue;
				if(ship->IsSpaceStation())
					continue;
				if(!ship->IsOnGround())
					continue;
				if(ship->InDesignEnvironment())
					continue;

				QString desc = ship->NameWithEmpire();
				m_NearbyShips->addItem(QIcon(":data/c_Spacecraft.png"), desc, QVariant(ship->Id().GetID()));
			}
		}
	}
	pView->UnlockSceneAtom();

	if(m_NearbyShips->currentIndex() < 0)
		m_Mothball->setEnabled(false);
	else
		m_Mothball->setEnabled(true);
}

void AuBldHangar::Query()
{
	auApp->HangarQuery();
}

void AuBldHangar::Launch()
{
	bool takeCommand = m_TakeCommand->isChecked();
	auApp->HangarLaunch(takeCommand);
}

void AuBldHangar::Mothball()
{
	int curIdx = m_NearbyShips->currentIndex();
	if(curIdx < 0)
		return;

	AuId shipId = AuId(m_NearbyShips->itemData(curIdx).toUInt());
	auApp->HangarMothball(shipId);
}

void AuBldHangar::QueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar)
{
	if(shipsInHangar)
		m_StoredShipsInfo->setText(tr("%1 spacecraft stored in hangar.").arg(shipsInHangar));
	else
		m_StoredShipsInfo->setText(tr("No spacecraft stored in hangar."));

	if(shipDesc.isEmpty())
	{
		m_StoredShipInfo->setText(tr("No spacecraft stored by you."));
		m_Launch->setEnabled(false);
		m_TakeCommand->setEnabled(false);
	}
	else
	{
		m_StoredShipInfo->setText(tr("%1 %2").arg(shipDesc).arg(shipDesign));
		m_Launch->setEnabled(true);
		m_TakeCommand->setEnabled(true);
	}
}
