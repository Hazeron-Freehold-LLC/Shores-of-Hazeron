//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucargodialog.cpp,v 1.5 2022/02/19 18:10:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucargodialog.h"
//
#include "auaccesshold.h"
#include "aucargojettison.h"
#include "aucargotransfer.h"
#include "auapp.h"
#include "auframe.h"
#include "auordnancetransfer.h"
#include <auglobal.h>
#include <QGridLayout>
#include <QIcon>
#include <QStackedLayout>
#include <QTabBar>

void AuCargoDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCargoDockFree::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCargoDialog::AuCargoDialog(bool freeAvatar, QWidget* parent) : QWidget(parent)
{
	Q_UNUSED(freeAvatar);
	m_Pages = new QStackedLayout;

	m_Tabs = new QTabBar(this);
	connect(m_Tabs, SIGNAL(currentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	int pageIndex = 0;

	//if(!freeAvatar)
	//{
		AuCargoJettison* jettPage = new AuCargoJettison(this);
		m_Pages->insertWidget(pageIndex, jettPage);

		QIcon jettIcon(":data/c_Crate.png");
		int itab = m_Tabs->insertTab(pageIndex, jettIcon, "Manifest");
		m_Tabs->setTabToolTip(itab, tr("<html><b>Manifest</b><br>"
					"Shows a manifest of cargo in the hold of the ship. "
					"</html>"));

		pageIndex++;
	//}

	AuAccessHold* holdPage = new AuAccessHold(this);
	m_Pages->insertWidget(pageIndex, holdPage);

	QIcon holdIcon(":data/Shelf.png");
	itab = m_Tabs->insertTab(pageIndex, holdIcon, "Hold");
	m_Tabs->setTabToolTip(itab, QString("<html>"
				"<b>Cargo Hold and Fuel Cells</b><br>"
				"Move to a room aboard a spacecraft to transfer items between your gear and the cargo hold or fuel cells. "
				"Different kinds of rooms enable access to specific items.<br>"
				"%1 <b>Barracks</b> accesses the <i>Armory</i> for weapons and ammunition.<br>"
				"%1 <b>Brig</b> accesses the <i>Feeder</i> for food and water.<br>"
				"%1 <b>Engine Room</b> accesses the <i>Fuel Coupling</i> for hydrogen fuel.<br>"
				"%1 <b>Galley</b> accesses the <i>Pantry</i> for food and drinks.<br>"
				"%1 <b>Hold</b> accesses the <i>Cargo Hold</i> for all things in the hold.<br>"
				"%1 <b>Lounge</b> accesses the <i>Bar</i> for food and drinks.<br>"
				"%1 <b>Sick Bay</b> accesses the <i>Pharmacy</i> for medical supplies.<br>"
				"%1 <b>Stateroom</b>, <b>Crew Cabin</b> and <b>Officer Cabin</b> accesses the <i>Wardrobe</i> for "
					"clothing and environment suits.<br>"
				"%1 <b>Vehicle Bay</b> accesses the <i>Garage</i> for tools, fuel and ammo."
				"<hr>"
				"<b>Sea Chest</b><br>"
				"Enter your berth or stand at your home building site to transfer items between your gear and your sea chest.<br>"
				"%1 Sea chest can hold up to 100 items.<br>"
				"%1 Sea chest contents are never lost.<br>"
				"%1 Ship policy may prohibit access to sea chests by <i>passengers</i>.<br>"
				"%1 You have a different sea chest in each zone: conflict or neutral."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	pageIndex++;

	m_CargoPage = new AuCargoTransfer(this);
	m_Pages->insertWidget(pageIndex, m_CargoPage);

	QIcon xferIcon(":data/c_Spacecraft.png");
	itab = m_Tabs->insertTab(pageIndex, xferIcon, "Transfer");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Transfer</b><br>"
			"Transfers money, citizens and cargo between the spacecraft you are aboard and another "
			"spacecraft.<br>"
			"%1 You must be in the chain-of-command of the spacecraft you are aboard.<br>"
			"%1 The other spacecraft must be within trading range of your spacecraft.<br>"
			"%1 You must have authority to transfer cargo from the other spacecraft."
			"</html>").arg(g_Au.MiddleDotSymbol()));

	pageIndex++;

	m_OrdPage = new AuOrdnanceTransfer(this);
	m_Pages->insertWidget(pageIndex, m_OrdPage);

	QIcon ordIcon(":data/c_HeavyWeapon.png");
	itab = m_Tabs->insertTab(pageIndex, ordIcon, "Ordnance");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Ordnance</b><br>"
			"Loads ammunition into the vehicle you occupy.<br>"
			"%1 Ammunition may be loaded into a turret of a spacecraft.<br>"
			"%1 Turrets load ammunition from the hold of the spacecraft.<br>"
			"%1 Ammunition may be loaded into a vehicle that has weapons.<br>"
			"%1 Vehicles must be in the flight deck of a spacecraft, on a slab in a city, or in the trade zone of a station.<br>"
			"%1 Export policies affect the ordnance choices available."
			"</html>").arg(g_Au.MiddleDotSymbol()));

	QGridLayout* lay = new QGridLayout();
	lay->addWidget(m_Tabs, 0, 0);
	lay->addLayout(m_Pages, 1, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuCargoDialog::~AuCargoDialog()
{
}

void AuCargoDialog::OrdnanceReceived(const QByteArray& ba)
{
	if(m_OrdPage)
		m_OrdPage->OrdnanceReceived(ba);
}

void AuCargoDialog::TransferCitizensAvailable(const QByteArray& ba)
{
	if(m_CargoPage)
		m_CargoPage->TransferCitizensAvailable(ba);
}
