//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccesshold.cpp,v 1.13 2022/05/04 21:20:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auaccesshold.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auglobal.h"
#include "auview.h"
#include "daatom.h"
#include "daperson.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer>

void AuCargoHoldDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAccessHold::AuAccessHold(QWidget* parent) : QWidget(parent)
{
	m_Mode = Hold;

	// Place controls for listing gear.
	QLabel* lbinv = new QLabel(tr("Personal Gear"), this);

	m_GearFilter = FILTER_All;
	cfilt = new AuCommodityFilter;
	connect(cfilt, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(FilterGear(COMMODITYFILTER)));

	lwGear = new QListWidget(this);
	lwGear->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwGear->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwGear->setSelectionMode(QAbstractItemView::ExtendedSelection);
	lwGear->setSortingEnabled(true);
	connect(lwGear, SIGNAL(itemSelectionChanged()), SLOT(GearSelected()));
	connect(lwGear, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(GearDoubleClicked(QListWidgetItem*)));

	invdesc = new QTextBrowser(this);
	invdesc->setReadOnly(true);

	QHBoxLayout* invlay = new QHBoxLayout;
	invlay->addWidget(lwGear);
	invlay->addWidget(invdesc);

	// Place controls for transferring selected items.
	QImage image(":/data/hazBlackSliderArrow.png");
	QSize size(16, 16);
	QImage scaledImage = image.scaled( size, Qt::IgnoreAspectRatio);
	QPixmap pix = QPixmap::fromImage(scaledImage);

	pbtake = new QPushButton(tr("Take Item From Hold"));
	pbtake->setIcon(QIcon(pix));
	connect(pbtake, SIGNAL(clicked()), SLOT(TakeFromCargo()));

	QTransform arrowRotate;
	arrowRotate.rotate(180);
	scaledImage = scaledImage.transformed(arrowRotate,
			Qt::FastTransformation);
	pix = QPixmap::fromImage(scaledImage);

	pbput = new QPushButton(tr("Put Item Into Hold"));
	pbput->setIcon(QIcon(pix));
	connect(pbput, SIGNAL(clicked()), SLOT(PutIntoCargo()));

	// Place controls for listing ship's cargo.
	lbcargo = new QLabel(tr("Cargo Hold"), this);

	// Cargo aboard
	lwcargo = new QListWidget(this);
	lwcargo->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwcargo->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwcargo->setSelectionMode(QAbstractItemView::SingleSelection);
	lwcargo->setSortingEnabled(true);
	connect(lwcargo, SIGNAL(itemSelectionChanged()), SLOT(CargoSelected()));
	connect(lwcargo, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(CargoDoubleClicked(QListWidgetItem*)));

	cargodesc = new QTextBrowser(this);
	cargodesc->setReadOnly(true);

	QHBoxLayout* cargolay = new QHBoxLayout;
	cargolay->addWidget(lwcargo);
	cargolay->addWidget(cargodesc);

	// Cargo Space Status
	lbct = new QLabel(tr("Cargo Hold"), this);
	lbct->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_FillCargo = new QProgressBar(this);
	m_FillCargo->setFormat(tr("%v of %m"));

	lbnot = new QLabel(tr("<html>"
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
	lbnot->setWordWrap(true);
	lbnot->hide();

	QGridLayout* tlay = new QGridLayout();
	tlay->addWidget(pbtake, 0,0);
	tlay->addWidget(pbput, 1,0);
	tlay->addWidget(cfilt, 0,1, 2,1);

	QGridLayout* clay = new QGridLayout();
	clay->addWidget(lbct, 0,0);
	clay->addWidget(m_FillCargo, 0,1);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbinv);
	lay->addLayout(invlay);
	lay->addLayout(tlay);
	lay->addWidget(lbcargo);
	lay->addLayout(cargolay);
	lay->addLayout(clay);
	lay->addWidget(lbnot);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	m_timer = 0;

	// Set initial control states until timer hits.
	EnableControls();
}

AuAccessHold::~AuAccessHold()
{
}

// Update contents of dialog that might change while the dialog is up, such as
// performing the transactions that this dialog offers.
void AuAccessHold::UpdateContents()
{
	// Get list controls and selected item info for reselect after update.
	QByteArray selGearCode;
	QList<QListWidgetItem*> sel = lwGear->selectedItems();
	if(!sel.isEmpty())
		selGearCode = sel[0]->data(Qt::UserRole).toByteArray();

	AuItem* selItemClone = 0;
	sel = lwcargo->selectedItems();
	if(!sel.isEmpty())
		selItemClone = AuItem::Unpack(sel[0]->data(Qt::UserRole).toByteArray());

	// List all cargo.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		MODE accessMode = Invalid;
		if(person->IsInsideSpacecraft())
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				// Cargo can only be accessed from certain room types.
				ROOMTYPE inRoomType = person->ShipRoomType();
				switch(inRoomType)
				{
				case ROOM_None:
				case ROOM_Hull:
				case ROOM_Bridge:
				case ROOM_Corridor:
				case ROOM_Fuel:
				case ROOM_GravityWell:
				case ROOM_OpenToDeckBelow:
				case ROOM_SensorRoom:
				case ROOM_ShieldRoom:
				case ROOM_TransporterRoom:
				case ROOM_WeaponBay:
				case ROOM_HullLargeDoor:
				case ROOM_HullOpening:
				case ROOM_HullWallDoor:
				case ROOM_HullWallTurret:
				case ROOM_RoofGable:
				case ROOM_RoofHip:
				case ROOM_RoofRound:
					// These are not valid rooms in which to interact with cargo.
					break;

				case ROOM_Barracks:
					if(person->IsInBerth())						// Troop is in their berth.
						accessMode = SeaChest;
					else
						accessMode = Armory;
					break;

				case ROOM_Brig:
					accessMode = Feeder;
					break;
				case ROOM_Hold:
					accessMode = Hold;
					break;
				case ROOM_EngineRoom:
					accessMode = FuelCoupling;
					break;
				case ROOM_Galley:
					accessMode = Pantry;
					break;
				case ROOM_Lounge:
					accessMode = Bar;
					break;
				case ROOM_SickBay:
					accessMode = Pharmacy;
					break;

				case ROOM_CaptainsCabin:
				case ROOM_CrewCabin:
					if(person->IsInBerth())						// Officer|crew is in their berth.
						accessMode = SeaChest;
					else
						accessMode = Wardrobe;
					break;

				case ROOM_Stateroom:
					if(person->IsInBerth() &&														// Person is in their berth AND
							(ship->CanPassengersAccessSeaChest() ||									// policy allows chest access OR
							 !AuRosterEntry::IsPassenger(ship->BerthTypeOccupied(person->Id()))))	// it's not a passenger berth.
						accessMode = SeaChest;
					else
						accessMode = Wardrobe;
					break;

				case ROOM_VehicleBay:
					accessMode = Garage;
					break;
				}
			}
		}
		else if(!person->IsAboardVehicle() &&
				!person->IsInSpacecraft() &&
				person->InIndustry() != IID_None &&
				!person->SpawnCity().IsNull() && 
				person->SpawnCity() == person->BdId() &&
				FuzzyEquals(person->InIndustryLL().Longitude(), person->SpawnDevLL().Longitude()) &&
				FuzzyEquals(person->InIndustryLL().Latitude(), person->SpawnDevLL().Latitude()))
		{
			accessMode = SeaChest;
		}

		if(m_Mode != accessMode)
		{
			m_Mode = accessMode;

			// Reconfigure controls for this mode.
			QString titleLabel;
			QString accessLabel;
			QString takeLabel;
			QString putLabel;
			QString totLabel = tr("Total Cargo Hold");
			QString usedLabel = tr("Used Cargo Hold");
			QString emptyLabel = tr("Empty Cargo Hold");
			QString invTip, cargoTip;
			switch(m_Mode)
			{
			case Invalid:
				titleLabel = tr("Cargo");
				accessLabel = "";
				invTip = tr("<html>Items in your gear that can be stored.</html>");
				cargoTip = tr("<html>Items can be taken from storage.</html>");
				break;
			case Armory:
				titleLabel = tr("Barracks");
				accessLabel = tr("Armory");
				takeLabel = tr("Take Item From Armory");
				putLabel = tr("Put Item Into Armory");
				invTip = tr("<html>Items in your gear that can be put into the armory.</html>");
				cargoTip = tr("<html>Items that can be taken from the armory.</html>");
				break;
			case Feeder:
				titleLabel = tr("Brig");
				accessLabel = tr("Feeder");
				takeLabel = tr("Take Item From Feeder");
				putLabel = tr("Put Item Into Feeder");
				invTip = tr("<html>Items in your gear that can be put into the feeder.</html>");
				cargoTip = tr("<html>Items that can be taken from the feeder.</html>");
				break;
			case FuelCoupling:
				titleLabel = tr("Engine Room");
				accessLabel = tr("Fuel Coupling");
				takeLabel = tr("Take Fuel From Coupling");
				putLabel = tr("Put Fuel Into Coupling");
				totLabel = tr("Total Fuel Cells");
				usedLabel = tr("Used Fuel Cells");
				emptyLabel = tr("Empty Fuel Cells");
				invTip = tr("<html>Items in your gear that can be put into the fuel coupling.</html>");
				cargoTip = tr("<html>Items that can be taken from the fuel coupling.</html>");
				break;
			case Hold:
				titleLabel = tr("Hold");
				accessLabel = tr("Cargo Hold");
				takeLabel = tr("Take Item From Hold");
				putLabel = tr("Put Item Into Hold");
				invTip = tr("<html>Items in your gear that can be put into the hold.</html>");
				cargoTip = tr("<html>Items that can be taken from the hold.</html>");
				break;
			case Bar:
				titleLabel = tr("Lounge");
				accessLabel = tr("Bar");
				takeLabel = tr("Take Item From Bar");
				putLabel = tr("Put Item Into Bar");
				invTip = tr("<html>Items in your gear that can be put into the bar.</html>");
				cargoTip = tr("<html>Items that can be taken from the bar.</html>");
				break;
			case Pantry:
				titleLabel = tr("Galley");
				accessLabel = tr("Pantry");
				takeLabel = tr("Take Item From Pantry");
				putLabel = tr("Put Item Into Pantry");
				invTip = tr("<html>Items in your gear that can be put into the pantry.</html>");
				cargoTip = tr("<html>Items that can be taken from the pantry.</html>");
				break;
			case Pharmacy:
				titleLabel = tr("Sick Bay");
				accessLabel = tr("Pharmacy");
				takeLabel = tr("Take Item From Pharmacy");
				putLabel = tr("Put Item Into Pharmacy");
				invTip = tr("<html>Items in your gear that can be put into the pharmacy.</html>");
				cargoTip = tr("<html>Items that can be taken from the pharmacy.</html>");
				break;
			case SeaChest:
				titleLabel = tr("Berth");
				accessLabel = tr("Sea Chest");
				takeLabel = tr("Take Item From Sea Chest");
				putLabel = tr("Put Item Into Sea Chest");
				totLabel = tr("Total Sea Chest");
				usedLabel = tr("Used Sea Chest");
				emptyLabel = tr("Empty Sea Chest");
				invTip = tr("<html>Items in your gear that can be put into your sea chest.</html>");
				cargoTip = tr("<html>Items that can be taken from your sea chest.</html>");
				break;
			case Wardrobe:
				titleLabel = tr("Cabin");
				accessLabel = tr("Wardrobe");
				takeLabel = tr("Take Item From Wardrobe");
				putLabel = tr("Put Item Into Wardrobe");
				invTip = tr("<html>Items in your gear that can be put into the wardrobe.</html>");
				cargoTip = tr("<html>Items that can be taken from the wardrobe.</html>");
				break;
			case Garage:
				titleLabel = tr("Vehicle Bay");
				accessLabel = tr("Garage");
				takeLabel = tr("Take Item From Garage");
				putLabel = tr("Put Item Into Garage");
				invTip = tr("<html>Items in your gear that can be put into the garage.</html>");
				cargoTip = tr("<html>Items that can be taken from the garage.</html>");
				break;
			}
			lwGear->setToolTip(invTip);
			if(m_Mode == Invalid)
			{
				lbnot->show();
				pbput->hide();
				pbtake->hide();
				lbct->hide();
				m_FillCargo->hide();
				lbcargo->hide();
				cargodesc->hide();
				lwcargo->hide();
				cfilt->hide();
			}
			else
			{
				lbcargo->setText(accessLabel);
				lbct->setText(totLabel);
				lwcargo->setToolTip(cargoTip);
				pbput->setText(putLabel);
				pbtake->setText(takeLabel);

				lbnot->hide();
				pbput->show();
				pbtake->show();
				lbct->show();
				m_FillCargo->show();
				lbcargo->show();
				cargodesc->show();
				lwcargo->show();
				cfilt->show();
			}
		}

		// Get a list of the commodities that fit into the current filter.
		QList<COMMODITYID> cidFilter;
		g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_GearFilter);

		// Insure we have the empire and specie data loaded for all items in gear, including items in containers.
		const AuGear* inven = person->LockGearForRead();
		auApp->QueryEmpireAndSpecie(*inven);

		// Show gear contents appropriate to person's location.
		// Check for changes before clearing and refilling.
		AuItemHash gear;
		for(AuGear::const_iterator it = inven->begin(); it != inven->end(); it++)
		{
			// Filter gear items based on access mode.
			bool useitem = true;
			switch(m_Mode)
			{
			case Invalid:
				break;
			case Armory:
				if(!(*it).Item().IsArmoryItem())
					useitem = false;	// Skip non-weapons and non-armor in the armory.
				break;
			case Feeder:
				if(!(*it).Item().IsFood())
					useitem = false;	// Only food and water in brig.
				break;
			case FuelCoupling:
				if((*it).Item().CommodityId() != CID_Hydrogen)
					useitem = false;	// Skip non-hydrogen at fuel coupling.
				break;
			case Hold:
				// Everything except hydrogen is accessible in the hold.
				if((*it).Item().CommodityId() == CID_Hydrogen)
					useitem = false;
				break;
			case Bar:
				if(!(*it).Item().IsCantinaItem())
					useitem = false;	// Skip non-Cantina items in bar.
				break;
			case Pantry:
				if(!(*it).Item().IsPantryItem())
					useitem = false;	// Skip non-pantry items in pantry.
				break;
			case Pharmacy:
				if(!(*it).Item().IsMedical())
					useitem = false;	// Skip non-pharmacy items in the pharm
				break;
			case SeaChest:
				// Everything is accessible in the sea chest.
				break;
			case Wardrobe:
				if(!(*it).Item().IsWardrobeItem())
					useitem = false;	// Skip non-wardrobe items in the wardrobe
				break;
			case Garage:
				if(!(*it).Item().IsGarageItem())
					useitem = false;	// Skip non-garage items in the garage
				break;
			}

			// Apply category filter to list.
			if(useitem)
				useitem = cidFilter.contains((*it).Item().CommodityId());

			// Do not list non-empty containers.
			if(m_Mode != SeaChest && (*it).Item().Content() && !(*it).Item().Content()->empty())
				useitem = false;

			if(useitem)
				gear.insert(pair<COMMODITYID, AuItem*>((*it).Item().CommodityId(), (*it).Item().Clone()));

			if((*it).Item().Content())
			{
				const AuItemContent* cont = (*it).Item().Content();
				for(AuItemContent::const_iterator cit = cont->begin(); cit != cont->end(); cit++)
				{
					// Filter gear items based on access mode.
					switch(m_Mode)
					{
					case Invalid:
						break;
					case Armory:
						if(!(*cit).second->IsArmoryItem())
							continue;	// Skip non-weapons and non-armor in the armory.
						break;
					case Feeder:
						if(!(*cit).second->IsFood())
							continue;	// Only food and water in brig.
						break;
					case FuelCoupling:
						if((*cit).second->CommodityId() != CID_Hydrogen)
							continue;	// Skip non-hydrogen at fuel coupling.
						break;
					case Hold:
						// Everything except hydrogen is accessible in the hold.
						if((*cit).second->CommodityId() == CID_Hydrogen)
							continue;
						break;
					case Bar:
						if(!(*cit).second->IsCantinaItem())
							continue;	// Skip non-Cantina items in bar.
						break;
					case Pantry:
						if(!(*cit).second->IsPantryItem())
							continue;	// Skip non-pantry items in pantry.
						break;
					case Pharmacy:
						if(!(*cit).second->IsMedical())
							continue;	// Skip non-pharmacy items in the pharm.
						break;
					case SeaChest:
						// Everything is accessible in the sea chest.
						break;
					case Wardrobe:
						if(!(*cit).second->IsWardrobeItem())
							continue;	// Skip non-Wardrobe items in the Wardrobe.
						break;
					case Garage:
						if(!(*cit).second->IsGarageItem())
							continue;	// Skip non-Garage items in the Garage.
						break;
					}

					// Apply category filter to list.
					if(!cidFilter.contains((*cit).second->CommodityId()))
						continue;

					// Do not list non-empty containers.
					if(m_Mode != SeaChest && (*cit).second->Content() && !(*cit).second->Content()->empty())
						continue;

					gear.insert(pair<COMMODITYID, AuItem*>((*cit).second->CommodityId(), (*cit).second->Clone()));
				}
			}
		}
		person->UnlockGear();

		if(gear != m_GearShown)
		{
			// Something in gear changed.
			m_GearShown.Clear();
			for(AuItemHash::const_iterator ii = gear.begin(); ii != gear.end(); ii++)
				m_GearShown.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));

			// List gear items relevant to the access mode.
			lwGear->clear();
			for(AuItemHash::const_iterator ii = m_GearShown.begin(); ii != m_GearShown.end(); ii++)
			{
				QString text = (*ii).second->ListDesc();
				QByteArray code = (*ii).second->Pack();

				QPixmap px32 = QPixmap((*ii).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				QIcon icon(px32);

				QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwGear);
				lwi->setData(Qt::UserRole, code);
				lwi->setToolTip((*ii).second->HtmlToolTip());

				if(code == selGearCode)
				{
					lwi->setSelected(true);
					selGearCode = "";	// so we don't match another
				}
			}
		}

		if(m_Mode == SeaChest)
		{
			// Show available sea chest items.
			// Check for changes before clearing and refilling.
			AuItemHash cargo;

			// Cargo aboard
			// Get a list of the commodities that fit into the current filter.
			QList<COMMODITYID> cidFilter;
			g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_GearFilter);

			// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
			const AuItemHash& cgo = person->LockSeaChestForRead();
			auApp->QueryEmpireAndSpecie(cgo);

			for(AuItemHash::const_iterator it = cgo.begin(); it != cgo.end(); it++)
			{
				// Apply filter to list.
				if(!cidFilter.contains((*it).second->CommodityId()))
					continue;

				cargo.insert(pair<COMMODITYID, AuItem*>((*it).second->CommodityId(), (*it).second->Clone()));
			}
			person->UnlockSeaChest();

			// Cargo Usage Status
			if(cargo != m_CargoShown)
			{
				// Something in cargo changed.
				m_CargoShown.Clear();
				for(AuItemHash::const_iterator ii = cargo.begin(); ii != cargo.end(); ii++)
					m_CargoShown.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));

				// List person's sea chest items that are relevant to the access mode.
				lwcargo->clear();
				for(AuItemHash::const_iterator ii = m_CargoShown.begin(); ii != m_CargoShown.end(); ii++)
				{
					QString text = (*ii).second->ListDesc();
					QByteArray code = (*ii).second->Pack();

					QPixmap px32 = QPixmap((*ii).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					QIcon icon(px32);

					QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwcargo);
					lwi->setData(Qt::UserRole, code);
					lwi->setToolTip((*ii).second->HtmlToolTip());

					if(selItemClone && selItemClone->IsItem(*(*ii).second))
					{
						lwi->setSelected(true);
						delete selItemClone;
						selItemClone = 0;	// so we don't match another
					}
				}

				// Cargo Space Status
				int cargoTotal = person->SeaChestCapacity();
				int cargoUsed = person->LockSeaChestForRead().TotalUnits();
				person->UnlockSeaChest();
				m_FillCargo->setRange(0, cargoTotal);
				m_FillCargo->setValue(cargoUsed);
			}
		}
		else if(m_Mode != Invalid)
		{
			// Show available cargo items.
			// Check for changes before clearing and refilling.
			AuItemHash cargo;
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				if(m_Mode == FuelCoupling || m_Mode == Garage)
				{
					if(m_GearFilter == FILTER_All || m_GearFilter == FILTER_Fuel)
					{
						// List Hydrogen fuel in fuel cells.
						int fuelQty = (int)ship->FuelQuantity();
						if(fuelQty)
						{
							unsigned int fuelQual = ship->FuelQuality();

							AuItem* item = AuItem::Create(CID_Hydrogen, fuelQual, ship->EmpireId(), fuelQty);
							if(item)
								cargo.insert(pair<COMMODITYID, AuItem*>(CID_Hydrogen, item));
						}
					}
				}

				if(m_Mode != FuelCoupling)
				{
					// Cargo aboard
					// Get a list of the commodities that fit into the current filter.
					QList<COMMODITYID> cidFilter;
					g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_GearFilter);

					// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
					const AuItemHash& cgo = ship->LockCargoForRead();
					auApp->QueryEmpireAndSpecie(cgo);

					for(AuItemHash::const_iterator it = cgo.begin(); it != cgo.end(); it++)
					{
						// Filter cargo hold items based on access mode.
						switch(m_Mode)
						{
						case Invalid:
							break;
						case Armory:
							if(!(*it).second->IsArmoryItem())
								continue;	// Skip non-weapons and non-armor in the armory.
							break;
						case Feeder:
							if(!(*it).second->IsFood())
								continue;	// Only food and water in brig.
							break;
						case FuelCoupling:
							if((*it).second->CommodityId() != CID_Hydrogen)
								continue;	// Skip non-hydrogen at fuel coupling.
							break;
						case Hold:
							// Everything except hydrogen is accessible in the hold.
							if((*it).second->CommodityId() == CID_Hydrogen)
								continue;
							break;
						case Bar:
							if(!(*it).second->IsCantinaItem())
								continue;	// Skip non-Cantina items in bar.
							break;
						case Pantry:
							if(!(*it).second->IsPantryItem())
								continue;	// Skip non-pantry items in pantry.
							break;
						case Pharmacy:
							if(!(*it).second->IsMedical())
								continue;	// Skip non-pharmacy items in pharmacy.
							break;
						case SeaChest:
							// Everything is accessible in the sea chest.
							break;
						case Wardrobe:
							if(!(*it).second->IsWardrobeItem())
								continue;	// Skip non-wardrobe items in wardrobe.
							break;
						case Garage:
							if(!(*it).second->IsGarageItem())
								continue;	// Skip non-Garage items in Garage.
							break;
						}

						// Apply filter to list.
						if(!cidFilter.contains((*it).second->CommodityId()))
							continue;

						cargo.insert(pair<COMMODITYID, AuItem*>((*it).second->CommodityId(), (*it).second->Clone()));
					}
					ship->UnlockCargo();
				}
			}

			if(cargo != m_CargoShown)
			{
				// Something in cargo changed.
				m_CargoShown.Clear();
				for(AuItemHash::const_iterator ii = cargo.begin(); ii != cargo.end(); ii++)
					m_CargoShown.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));

				// List ship's cargo items that are relevant to the access mode.
				lwcargo->clear();
				if(ship)
				{
					if(m_Mode == FuelCoupling || m_Mode == Garage)
					{
						if(m_GearFilter == FILTER_All || m_GearFilter == FILTER_Fuel)
						{
							// List Hydrogen fuel in fuel cells.
							int fuelQty = (int)ship->FuelQuantity();
							if(fuelQty)
							{
								QString name = g_Au.CommodityName(CID_Hydrogen);

								QString madeIn;
								if(ship->EmpireId())
								{
									QString empireName;
									const DgData& gd = g_Au.LockDataForRead();
									gd.EmpireName(ship->EmpireId(), empireName);
									g_Au.UnlockData();
									madeIn = tr(" Made in %1").arg(empireName);
								}

								unsigned int fuelQual = ship->FuelQuality();
								// Format example: 'Hydrogen, Poor Q94, 12 units'
								QString text = QObject::tr("%1 %2 Q%3%5, %L4 units in fuel cells")
									.arg(name).arg(DbCommodity::QualityDesc(fuelQual))
									.arg(fuelQual).arg(fuelQty).arg(madeIn);

								AuItem* item = AuItem::Create(CID_Hydrogen, fuelQual, ship->EmpireId());
								if(item)
								{
									QByteArray code = item->Pack();

									QPixmap px32 = QPixmap(item->BitmapName()).scaled(32,32, Qt::KeepAspectRatio,
											Qt::SmoothTransformation);
									QIcon icon(px32);

									QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwcargo);
									lwi->setData(Qt::UserRole, code);
									lwi->setToolTip(item->HtmlToolTip());

									if(selItemClone && selItemClone->IsItem(*item))
									{
										lwi->setSelected(true);
										delete selItemClone;
										selItemClone = 0;	// so we don't match another
									}

									delete item;
								}
							}
						}

						int fuelTotal = (int)ship->FuelCapacity();
						int fuelUsed = (int)ceil(ship->FuelQuantity());
						m_FillCargo->setRange(0, fuelTotal);
						m_FillCargo->setValue(fuelUsed);
					}

					if(m_Mode != FuelCoupling)
					{
						// Cargo aboard
						for(AuItemHash::const_iterator ci = m_CargoShown.begin(); ci != m_CargoShown.end(); ci++)
						{
							QString text = (*ci).second->ListDesc();
							QByteArray code = (*ci).second->Pack();

							QPixmap px32 = QPixmap((*ci).second->BitmapName()).scaled(32,32, Qt::KeepAspectRatio,
									Qt::SmoothTransformation);
							QIcon icon(px32);

							QListWidgetItem* lwi = new QListWidgetItem(icon, text, lwcargo);
							lwi->setData(Qt::UserRole, code);
							lwi->setToolTip((*ci).second->HtmlToolTip());

							if(selItemClone && selItemClone->IsItem(*(*ci).second))
							{
								lwi->setSelected(true);
								delete selItemClone;
								selItemClone = 0;	// so we don't match another
							}
						}

						// Cargo Space Status
						int cargoTotal = ship->CargoCapacityVolume();
						int cargoUsed = ship->CargoTotalVolume();
						m_FillCargo->setRange(0, cargoTotal);
						m_FillCargo->setValue(cargoUsed);
					}
				}
			}
		}
	}
	pView->UnlockSceneAtom();
	EnableControls();

	if(selItemClone)
		delete selItemClone;
}

void AuAccessHold::EnableControls()
{
	// Cargo
	bool cargoSelected = false;
	QList<QListWidgetItem*> sel = lwcargo->selectedItems();
	if(!sel.isEmpty())
		cargoSelected = true;
	pbtake->setEnabled(cargoSelected);

	QString cargotext;
	QListWidgetItem* curcargo = lwcargo->currentItem();
	if(curcargo)
	{
		AuItem* item = AuItem::Unpack(curcargo->data(Qt::UserRole).toByteArray());
		if(item)
		{
			QSize packSize = item->BitmapSizeToFit(100);
			cargotext = QString("<html>"
					"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
					"</html>")
				.arg(item->BitmapName())
				.arg(item->HtmlDesc())
				.arg(packSize.width())
				.arg(packSize.height());
			delete item;
		}
	}
	cargodesc->setHtml(cargotext);

	// Gear
	bool gearSelected = false;
	sel = lwGear->selectedItems();
	if(!sel.isEmpty())
		gearSelected = true;
	pbput->setEnabled(gearSelected);

	QString invtext;
	QListWidgetItem* curinv = lwGear->currentItem();
	if(curinv)
	{
		AuItem* item = AuItem::Unpack(curinv->data(Qt::UserRole).toByteArray());
		if(item)
		{
			QSize packSize = item->BitmapSize(24, 8);
			invtext = QString("<html>"
					"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
					"</html>")
				.arg(item->BitmapName())
				.arg(item->HtmlDesc())
				.arg(packSize.width())
				.arg(packSize.height());
			delete item;
		}
	}
	invdesc->setHtml(invtext);
}

void AuAccessHold::CargoSelected()
{
	// Clear selection from the gear list.
	lwGear->blockSignals(true);
	lwGear->clearSelection();
	lwGear->blockSignals(false);

	EnableControls();
}

void AuAccessHold::GearSelected()
{
	// Clear selection from the cargo list.
	lwcargo->blockSignals(true);
	lwcargo->clearSelection();
	lwcargo->blockSignals(false);

	EnableControls();
}

void AuAccessHold::PutIntoCargo()
{
	// Look for the selected gear item.
	QList<QListWidgetItem*> sel = lwGear->selectedItems();
	if(!sel.isEmpty())
	{
		for(QList<QListWidgetItem*>::iterator si = sel.begin(); si != sel.end(); si++)
		{
			AuItem* item = AuItem::Unpack((*si)->data(Qt::UserRole).toByteArray());
			if(item)
			{
				auApp->PutIntoCargo(*item);
				delete item;
			}
		}
	}
}

void AuAccessHold::TakeFromCargo()
{
	QList<QListWidgetItem*> sel = lwcargo->selectedItems();
	if(!sel.isEmpty())
	{
		AuItem* item = AuItem::Unpack(sel[0]->data(Qt::UserRole).toByteArray());
		if(item)
		{
			auApp->GetFromCargo(*item);
			delete item;
		}
	}
}

void AuAccessHold::showEvent(QShowEvent* event)
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

void AuAccessHold::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();
}

void AuAccessHold::FilterGear(COMMODITYFILTER filter)
{
	m_GearFilter = filter;
	UpdateContents();
}

void AuAccessHold::CargoDoubleClicked(QListWidgetItem* /*item*/)
{
	TakeFromCargo();
}

void AuAccessHold::GearDoubleClicked(QListWidgetItem* /*item*/)
{
	PutIntoCargo();
}

void AuAccessHold::keyPressEvent(QKeyEvent* event)
{
	if(event->key() != Qt::Key_Escape)
		QWidget::keyPressEvent(event);
}

QSize AuAccessHold::sizeHint() const
{
	return QSize(800,600);
}

