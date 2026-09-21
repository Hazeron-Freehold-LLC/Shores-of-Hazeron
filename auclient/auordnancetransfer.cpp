//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auordnancetransfer.cpp,v 1.15 2022/05/04 21:17:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auordnancetransfer.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "davessel.h"
#include "davehicle.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>

void AuCargoOrdnanceDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoOrdnanceUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuOrdnanceTransfer::AuOrdnanceTransfer(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;
	m_WeaponCID = CID_Invalid;
	m_WeaponUnits = 0;

	m_Desc = new QLabel(tr("<html>"
				"<p>Ordnance is the ammunition, bombs, rockets, and missiles fired by kinetic weapons. "
				"Ordnance for energy weapons is represented by lenses, e.g. heat ray lens, disruptor beam lens, etc.</p>"
				"<p>Use this window to view and load ordnance while occupying a vehicle, turret or weapon station. "
				"Ordnance is shown that is currently loaded or selected into the weapon. "
				"In addition, ordnance is shown that can be loaded into the weapon from available stores.</p>"
				"<p>Weapons that use ordnance include: turrets aboard spacecraft, turrets in guard towers, "
				"weapon bays aboard spacecraft, weapon bases, and weapons integrated into combat vehicles.</p>"
				"<p>Spacecraft weapons and turrets load ordnance from the ship's hold. "
				"Turrets load ordnance immediately. "
				"Weapon bays simply select the ordnance that will be used when the weapon is fired.</p>"
				"<p>Weapon bases and guard towers load ordnance from the city's stores. "
				"Guard tower turrets load ordnance immediately. "
				"Weapon bases simply select the ordnance that will be used when the weapon is fired.</p>"
				"<p>Vehicle weapons can load ordnance from several different places.<br>"
				"%1 When in a vehicle bay, ordnance is loaded from the hold.<br>"
				"%1 When on a road slab, ordnance is loaded from city stores.<br>"
				"%1 When in trade range of a station, ordnance is loaded from the station's hold.</p>"
				"<p>Spacecraft ordnance is loaded like cargo, by purchasing it from cities. "
				"Fleet spacecraft can purchase ordnance for free at empire cities.</p>"
				"<p>Export policies apply when loading ordnance.</p>"
				"</html>").arg(g_Au.MiddleDotSymbol()),this);
	m_Desc->setWordWrap(true);
	m_Desc->hide();

	m_MyPic = new QLabel(this);
	m_MyName = new QLabel(this);

	m_GunPic = new QLabel(this);
	m_GunPic->setPixmap(QPixmap(":data/c_HeavyWeapon.png"));
	m_GunPic->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	m_MyGunPic = new QLabel(this);
	m_MyGunPic->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

	m_MyGunDesc = new QLabel(tr("Empty"), this);

	m_Ord0Pic = new QLabel(this);
	m_Ord0Pic->setPixmap(QPixmap(":data/WeaponRack.png"));
	m_Ord0Pic->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	m_MyOrd0Pic = new QLabel(this);
	m_MyOrd0Pic->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

	m_MyOrd0Desc = new QLabel(tr("Empty"), this);

	m_Ord1Pic = new QLabel(this);
	m_Ord1Pic->setPixmap(QPixmap(":data/WeaponRack.png"));
	m_Ord1Pic->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	m_MyOrd1Pic = new QLabel(this);
	m_MyOrd1Pic->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

	m_MyOrd1Desc = new QLabel(tr("Empty"), this);

	QGridLayout* mylay = new QGridLayout();
	int myrow=0;
	mylay->addWidget(m_Desc, myrow,0, 1,3);
	myrow++;
	mylay->addWidget(m_MyPic, myrow,0, 1,3);
	myrow++;
	mylay->addWidget(m_MyName, myrow,0, 1,3);
	myrow++;
	mylay->addWidget(m_GunPic, myrow,0);
	mylay->addWidget(m_MyGunPic, myrow,1);
	mylay->addWidget(m_MyGunDesc, myrow,2);
	myrow++;
	mylay->addWidget(m_Ord0Pic, myrow,0);
	mylay->addWidget(m_MyOrd0Pic, myrow,1);
	mylay->addWidget(m_MyOrd0Desc, myrow,2);
	myrow++;
	mylay->addWidget(m_Ord1Pic, myrow,0);
	mylay->addWidget(m_MyOrd1Pic, myrow,1);
	mylay->addWidget(m_MyOrd1Desc, myrow,2);
	myrow++;
	mylay->setRowStretch(myrow, 1);
	mylay->setColumnMinimumWidth(0,20);
	mylay->setColumnMinimumWidth(1,20);
	mylay->setColumnStretch(2, 1);

	m_OtherPic = new QLabel(this);
	m_OtherPic->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_OtherDesc = new QLabel(this);
	m_OtherDesc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_OrdnanceTable = new QTableWidget(0, 3, this);
	m_OrdnanceTable->setToolTip(tr("<html>Ammunition available for loading.</html>"));
	m_OrdnanceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_OrdnanceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
	m_OrdnanceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
	QStringList headerLabels;
	headerLabels << "Ammunition";
	headerLabels << "Rack";
	headerLabels << "Load";
	m_OrdnanceTable->setHorizontalHeaderLabels(headerLabels);
	m_OrdnanceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_OrdnanceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_OrdnanceTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_OrdnanceTable->setShowGrid(false);
	m_OrdnanceTable->setSortingEnabled(true);
	m_OrdnanceTable->sortByColumn(0, Qt::AscendingOrder);
	m_OrdnanceTable->verticalHeader()->hide();
	m_OrdnanceTable->verticalHeader()->setDefaultSectionSize(18);
	m_OrdnanceTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	m_OrdnanceTable->setColumnWidth(1, 40);
	m_OrdnanceTable->setColumnWidth(2, 50);
	connect(m_OrdnanceTable, SIGNAL(itemSelectionChanged()), SLOT(OrdnanceSelected()));

	m_OrdnanceDesc = new QLabel(this);

	m_OrdnancePic = new QLabel(this);

	m_LoadGun = new QPushButton(QIcon(":data/c_HeavyWeapon.png"), tr("%1 Load Guns").arg(g_Au.DoubleAngleLeftSymbol()), this);
	m_LoadGun->setToolTip(tr("<html>Loads the selected ammunition into the guns of your vehicle or the turret you are operating. "
				"One unit of the ammunition is removed from the inventory of the loader.<br>"
				"Ammunition replaced is returned to stock if unfired or an energy weapon."
				"</html>"));
	m_LoadGun->hide();
	connect(m_LoadGun, SIGNAL(clicked()), SLOT(LoadSelectedToGun()));

	m_SelectAmmo = new QPushButton(QIcon(":data/dp_WeaponBay.png"), tr("%1 Select Ammunition").arg(g_Au.DoubleAngleLeftSymbol()),
			this);
	m_SelectAmmo->setToolTip(tr("<html>Chooses the selected ammunition for the weapon bay you are operating. "
				"</html>"));
	m_SelectAmmo->hide();
	connect(m_SelectAmmo, SIGNAL(clicked()), SLOT(LoadSelectedToGun()));

	m_LoadOrd0 = new QPushButton(QIcon(":data/WeaponRack.png"), tr("%1 Load Rack 1").arg(g_Au.DoubleAngleLeftSymbol()), this);
	m_LoadOrd0->setToolTip(tr("<html>Loads the selected ammunition onto weapon rack 1 of your vehicle. "
				"One unit of the ammunition is removed from the inventory of the loader.<br>"
				"Ammunition replaced is returned to stock if unfired or an energy weapon."
				"</html>"));
	m_LoadOrd0->hide();
	connect(m_LoadOrd0, SIGNAL(clicked()), SLOT(LoadSelectedToOrd0()));

	m_LoadOrd1 = new QPushButton(QIcon(":data/WeaponRack.png"), tr("%1 Load Rack 2").arg(g_Au.DoubleAngleLeftSymbol()), this);
	m_LoadOrd1->setToolTip(tr("<html>Loads the selected ammunition onto weapon rack 2 of your vehicle. "
				"One unit of the ammunition is removed from the inventory of the loader.<br>"
				"Ammunition replaced is returned to stock if unfired or an energy weapon."
				"</html>"));
	m_LoadOrd1->hide();
	connect(m_LoadOrd1, SIGNAL(clicked()), SLOT(LoadSelectedToOrd1()));

	QGridLayout* olay = new QGridLayout();
	int orow=0;
	olay->addWidget(m_OtherPic, orow,1);
	orow++;
	olay->addWidget(m_OtherDesc, orow,0, 1,2);
	orow++;
	olay->addWidget(m_OrdnanceTable, orow,0, 1,2);
	orow++;
	olay->addWidget(m_LoadGun, orow,0);
	olay->addWidget(m_OrdnancePic, orow,1);
	orow++;
	olay->addWidget(m_LoadOrd0, orow,0);
	olay->addWidget(m_OrdnanceDesc, orow,1, 4,1);
	orow++;
	olay->addWidget(m_LoadOrd1, orow,0);
	orow++;
	olay->addWidget(m_SelectAmmo, orow,0);
	olay->setColumnStretch(1,1);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(mylay, row,0);
	lay->addLayout(olay, row,1);
	row++;
	lay->setColumnStretch(1, 1);
	lay->setMargin(0);
	lay->setSpacing(2);
	setLayout(lay);
}

AuOrdnanceTransfer::~AuOrdnanceTransfer()
{
}

void AuOrdnanceTransfer::showEvent(QShowEvent* event)
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

void AuOrdnanceTransfer::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		m_LoaderId = AuId(0);	// So we'll get fresh ordnance when this becomes visible again.
		return;
	}

	// Lock the root and find the scene atom.
	COMMODITYID oldWeaponCID = m_WeaponCID;
	quint32 oldWeaponUnits = m_WeaponUnits;
	m_WeaponCID = CID_Invalid;
	AuId loaderId;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// Vehicle, ship, and city weapons use ordnance.
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
		const DaVessel* ship = dynamic_cast<const DaVessel*>(person->Parent());
		if(vehicle)
		{
			m_GunPic->setPixmap(QPixmap(":data/c_HeavyWeapon.png"));
			m_MyName->setText(vehicle->Name());

			// Only certain vehicles can carry ordnance.
			switch(vehicle->CommodityId())
			{
			case CID_SpaceFighter:
				m_MyPic->setPixmap(QPixmap(":data/FighterF1Pic.png"));
				m_WeaponCID = vehicle->CommodityId();
				break;
			case CID_AdvSpaceFighter:
				m_MyPic->setPixmap(QPixmap(":data/FighterF2Pic.png"));
				m_WeaponCID = vehicle->CommodityId();
				break;
			case CID_SpaceTransport:
				m_MyPic->setPixmap(QPixmap(":data/BomberPic.png"));
				m_WeaponCID = vehicle->CommodityId();
				break;
			case CID_APC:
				m_MyPic->setPixmap(QPixmap(":data/APCPic.png"));
				m_WeaponCID = vehicle->CommodityId();
				break;
			default:
				break;
			}

			if(m_WeaponCID != CID_Invalid)
			{
				m_MyGunPic->setPixmap(QPixmap(vehicle->WeaponLoadPic(DaVehicle::RACK_Gun)));
				m_MyGunPic->setToolTip(vehicle->WeaponLoadDesc(DaVehicle::RACK_Gun));
				m_MyGunDesc->setText(vehicle->WeaponLoadListName(DaVehicle::RACK_Gun));

				m_MyOrd0Pic->setPixmap(QPixmap(vehicle->WeaponLoadPic(DaVehicle::RACK_Ord0)));
				m_MyOrd0Pic->setToolTip(vehicle->WeaponLoadDesc(DaVehicle::RACK_Ord0));
				m_MyOrd0Desc->setText(vehicle->WeaponLoadListName(DaVehicle::RACK_Ord0));

				m_MyOrd1Pic->setPixmap(QPixmap(vehicle->WeaponLoadPic(DaVehicle::RACK_Ord1)));
				m_MyOrd1Pic->setToolTip(vehicle->WeaponLoadDesc(DaVehicle::RACK_Ord1));
				m_MyOrd1Desc->setText(vehicle->WeaponLoadListName(DaVehicle::RACK_Ord1));
			}

			// Only certain locations can provide ordnance.
			INDUSTRYID iid = vehicle->InIndustry();
			if(DbIndustry::CanSupplyOrdnance(iid))
			{
				const DaWorld* world = vehicle->FindParentPlanetInEnv();
				if(world)
				{
					if(!vehicle->BdId().IsNull())
					{
						const DaBd* bd = world->FindBd(vehicle->BdId());
						if(bd && PoliticalStance(bd->EmpireId(), vehicle->EmpireId()) != POL_Enemy)
						{
							// Vehicle is at a bd and it's not an enemy.
							m_OtherPic->setPixmap(QPixmap(":data/City.png"));
							m_OtherDesc->setText(bd->Name());
							loaderId = bd->Id();
						}
					}
				}
			}
			else if(vehicle->IsInSpacecraft() && vehicle->ShipRoomType() == ROOM_VehicleBay)
			{
				const DaVessel* ship = vehicle->FindParentVesselInEnv();
				if(ship)
				{
					if(PoliticalStance(ship->EmpireId(), vehicle->EmpireId()) != POL_Enemy)
					{
						// Vehicle is in the flight deck and it's not an enemy.
						if(ship->IsSpaceStation())
							m_OtherPic->setPixmap(QPixmap(":data/CommStation.png"));
						else
							m_OtherPic->setPixmap(QPixmap(":data/c_Spacecraft.png"));
						m_OtherDesc->setText(ship->NameWithEmpire());
						loaderId = ship->Id();
					}
				}
			}
			else
			{
				bool hasStation = false;
				DaVessel* station = vehicle->StationInTradeRange(vehicle->RoughRadius(), hasStation);
				if(station && PoliticalStance(station->EmpireId(), vehicle->EmpireId()) != POL_Enemy)
				{
					// Vehicle is in trade range of a station and it's not an enemy.
					m_OtherPic->setPixmap(QPixmap(":data/CommStation.png"));
					m_OtherDesc->setText(station->NameWithEmpire());
					loaderId = station->Id();
				}
			}
		}
		else if(ship && (person->IsTurretPose() || person->IsFireControlStationPose()))
		{
			// Ship's hold provides ammunition.
			if(ship->IsSpaceStation())
				m_OtherPic->setPixmap(QPixmap(":data/CommStation.png"));
			else
				m_OtherPic->setPixmap(QPixmap(":data/c_Spacecraft.png"));
			m_OtherDesc->setText(ship->NameWithEmpire());
			loaderId = ship->Id();

			if(person->IsTurretPose())
			{
				// The scene atom is in a turret.
				m_MyPic->setPixmap(QPixmap(":data/ShipTurretPic.png"));
				m_GunPic->setPixmap(QPixmap(":data/c_HeavyWeapon.png"));
				m_WeaponCID = ship->SystemModule(SYS_Turret, person->StationId());
				m_MyName->setText(tr("Turret<br>%1").arg(g_Au.CommodityName(m_WeaponCID)));

				if(person->Ordnance())
				{
					m_MyGunPic->setPixmap(QPixmap(person->Ordnance()->BitmapName()));
					m_MyGunPic->setToolTip(person->Ordnance()->HtmlToolTip());
					m_MyGunDesc->setText(person->Ordnance()->ListName());
				}
				else
				{
					m_MyGunPic->setPixmap(QPixmap(""));
					m_MyGunPic->setToolTip(tr("No ammunition loaded."));
					m_MyGunDesc->setText(tr("Empty"));
				}
			}
			else if(person->IsFireControlStationPose())
			{
				COMMODITYID bayCID = ship->SystemModule(SYS_Weapon, person->ControlId());
				if(DbCommodity::IsBayWeaponModule(bayCID))
				{
					// The scene atom is in fire control console.
					m_MyPic->setPixmap(QPixmap(":data/ShipBayWeaponPic.png"));
					m_GunPic->setPixmap(QPixmap(":data/dp_WeaponBay.png"));
					m_WeaponCID = bayCID;
					m_WeaponUnits = ship->BayWeaponUnits(person->ControlId());
					m_MyName->setText(tr("Bay %1<br>%2").arg(person->ControlId()+1).arg(g_Au.CommodityName(bayCID)));

					if(person->Ordnance())
					{
						m_MyGunPic->setPixmap(QPixmap(person->Ordnance()->BitmapName()));
						m_MyGunPic->setToolTip(person->Ordnance()->HtmlToolTip());
						m_MyGunDesc->setText(person->Ordnance()->ListName());
					}
					else
					{
						m_MyGunPic->setPixmap(QPixmap(""));
						m_MyGunPic->setToolTip(tr("No ammunition loaded."));
						m_MyGunDesc->setText(tr("Empty"));
					}
				}
			}
		}
	}
	pView->UnlockSceneAtom();

	bool showDesc = true;	// True to show generic tips. Set false if we display anything useful.

	bool vehicleOk = m_WeaponCID != CID_Invalid;
	bool ordOk = vehicleOk && !DbCommodity::IsTurretModule(m_WeaponCID) && !DbCommodity::IsBayWeaponModule(m_WeaponCID);
	if(vehicleOk || ordOk)
		showDesc = false;
	m_GunPic->setVisible(vehicleOk);
	m_MyGunDesc->setVisible(vehicleOk);
	m_MyGunPic->setVisible(vehicleOk);
	m_MyOrd0Desc->setVisible(ordOk);
	m_MyOrd0Pic->setVisible(ordOk);
	m_MyOrd1Desc->setVisible(ordOk);
	m_MyOrd1Pic->setVisible(ordOk);
	m_MyPic->setVisible(vehicleOk);
	m_MyName->setVisible(vehicleOk);
	m_Ord0Pic->setVisible(ordOk);
	m_Ord1Pic->setVisible(ordOk);

	bool locationOk = !loaderId.IsNull();
	m_OtherDesc->setVisible(locationOk);
	m_OtherPic->setVisible(locationOk);
	m_OrdnanceTable->setVisible(locationOk);
	if(locationOk)
	{
		OrdnanceSelected();
		showDesc = false;
	}
	else
	{
		bool loadOrdOk = locationOk && !DbCommodity::IsTurretModule(m_WeaponCID) && !DbCommodity::IsBayWeaponModule(m_WeaponCID);
		if(DbCommodity::IsBayWeaponModule(m_WeaponCID))
		{
			m_LoadGun->setVisible(false);
			m_SelectAmmo->setVisible(locationOk);
		}
		else
		{
			m_LoadGun->setVisible(locationOk);
			m_SelectAmmo->setVisible(false);
		}
		m_LoadOrd0->setVisible(loadOrdOk);
		m_LoadOrd1->setVisible(loadOrdOk);
		m_OrdnanceDesc->setVisible(locationOk);
		m_OrdnancePic->setVisible(locationOk);
		if(loadOrdOk || locationOk)
			showDesc = false;
	}

	if(vehicleOk && locationOk)
	{
		if(m_LoaderId != loaderId || m_WeaponCID != oldWeaponCID || m_WeaponUnits != oldWeaponUnits)
		{
			// The loader id has changed, or perhaps the kind of bay weapon they are operating. Request fresh ordnance.
			m_LoaderId = loaderId;
			RequestOrdnance();
		}
		showDesc = false;
	}

	m_Desc->setVisible(showDesc);
}

void AuOrdnanceTransfer::RequestOrdnance()
{
	auApp->OrdnanceRequest();
}

void AuOrdnanceTransfer::OrdnanceReceived(const QByteArray& ba)
{
	// Extract the ordnance from the byte array.
	AuItemHash ord;
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	in >> ord;

	// List ammunition.
	m_OrdnanceTable->clearContents();
	m_OrdnanceTable->setSortingEnabled(false);
	for(AuItemHash::const_iterator ii = ord.begin(); ii != ord.end(); ii++)
	{
		// Filter ammo that cannot be loaded by the player's current weapon.
		if(!DbCommodity::IsAmmunitionForWeapon(m_WeaponCID, (*ii).second->CommodityId()))
			continue;

		m_OrdnanceTable->insertRow(0);

		int col=0;
		QString text = (*ii).second->ListDesc();
		QIcon icon((*ii).second->BitmapName());
		QTableWidgetItem* twname = new QTableWidgetItem(icon, text);
		twname->setData(Qt::UserRole, (*ii).second->Pack());
		twname->setToolTip(tr("Brief description of ammunition available."));
		m_OrdnanceTable->setItem(0,col++, twname);

		QString rack;
		QString tip;
		QString rounds;
		QString rtip;
		int roundsCount;
		switch(m_WeaponCID)
		{
		case CID_BallisticWeaponBayModule:
			// Selecting ammo for a bay weapon.
			rack = ":data/dp_WeaponBay.png";
			tip = tr("Selects ammunition to be fired by %1.").arg(g_Au.CommodityName(m_WeaponCID));
			rtip = tr("%1 ammunition units needed to fire full barrage.")
				.arg(BayWeaponCannons(m_WeaponUnits));
			rounds = QString("%L1").arg(BayWeaponCannons(m_WeaponUnits));
			break;

		case CID_EnergyWeaponBayModule:
			// Selecting ammo for a bay weapon.
			rack = ":data/dp_WeaponBay.png";
			tip = tr("Selects energy weapon unit to be fired by %1.").arg(g_Au.CommodityName(m_WeaponCID));
			rtip = tr("%1 ammunition units needed to fire full bundle.")
				.arg(BayWeaponProjectors(m_WeaponUnits));
			rounds = QString("%L1").arg(BayWeaponProjectors(m_WeaponUnits));
			break;

		case CID_WeaponDeploymentBayModule:
			// Selecting ammo for a bay weapon.
			rack = ":data/dp_WeaponBay.png";
			tip = tr("Selects ammunition to be deployed by %1.").arg(g_Au.CommodityName(m_WeaponCID));
			rtip = tr("%1 ammunition units needed to drop full load.")
				.arg(BayWeaponEjectorRacks(m_WeaponUnits));
			rounds = QString("%L1").arg(BayWeaponEjectorRacks(m_WeaponUnits));
			break;

		case CID_WeaponLauncherBayModule:
			// Selecting ammo for a bay weapon.
			rack = ":data/dp_WeaponBay.png";
			tip = tr("Selects ammunition to be launched by %1.").arg(g_Au.CommodityName(m_WeaponCID));
			rtip = tr("%1 ammunition units needed to launch full volley.")
				.arg(BayWeaponLaunchers(m_WeaponUnits));
			rounds = QString("%L1").arg(BayWeaponLaunchers(m_WeaponUnits));
			break;

		case CID_BallisticWeaponTurretModule:
		case CID_WeaponLauncherTurretModule:
			// Loading a turret.
			rack = ":data/c_HeavyWeapon.png";
			tip = tr("Loads into turret guns.");
			roundsCount = DbCommodity::MaxTurretRounds((*ii).second->CommodityId());
			rtip = tr("Rounds count when loaded is %L1. Reload consumes one unit of ammunition.").arg(roundsCount);
			rounds = QString("%L1").arg(roundsCount);
			break;

		case CID_EnergyWeaponTurretModule:
			// Loading a turret.
			rack = ":data/c_HeavyWeapon.png";
			tip = tr("Loads into turret guns.");
			roundsCount = DbCommodity::MaxTurretRounds((*ii).second->CommodityId());
			rtip = tr("Energy weapon rounds count is %L1. Reload consumes one unit of energy.").arg(roundsCount);
			rounds = QString("%L1e").arg(roundsCount);
			break;

		default:
			// Vehicle has guns and weapon racks.
			switch(DaVehicle::WeaponRack((*ii).second->CommodityId()))
			{
			case DaVehicle::RACK_None:
				break;
			case DaVehicle::RACK_Gun:
				rack = ":data/c_HeavyWeapon.png";
				tip = tr("Loads into vehicle guns.");
				break;
			case DaVehicle::RACK_Ord0:
			case DaVehicle::RACK_Ord1:
				rack = ":data/WeaponRack.png";
				tip = tr("Loads onto vehicle weapon rack.");
				break;
			}

			roundsCount = DaVehicle::MaxWeaponRounds(m_WeaponCID, (*ii).second->CommodityId());
			if(DbCommodity::IsEnergyWeapon((*ii).second->CommodityId()))
			{
				rtip = tr("Energy weapon rounds count is %L1. Reload consumes one unit of fuel.").arg(roundsCount);
				rounds = QString("%L1e").arg(roundsCount);
			}
			else
			{
				rtip = tr("Rounds count when loaded is %L1.").arg(roundsCount);
				rounds = QString("%L1").arg(roundsCount);
			}
		}

		QTableWidgetItem* twloadat = new QTableWidgetItem(QIcon(rack), "");
		twloadat->setToolTip(tip);
		m_OrdnanceTable->setItem(0,col++, twloadat);

		QTableWidgetItem* twrds = new QTableWidgetItem(rounds);
		twrds->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
		twrds->setToolTip(rtip);
		m_OrdnanceTable->setItem(0,col++, twrds);
	}
	m_OrdnanceTable->setSortingEnabled(true);
}

void AuOrdnanceTransfer::OrdnanceSelected()
{
	QList<QTableWidgetItem*> selitems = m_OrdnanceTable->selectedItems();
	if(selitems.isEmpty())
	{
		m_LoadGun->hide();
		m_SelectAmmo->hide();
		m_LoadOrd0->hide();
		m_LoadOrd1->hide();
		m_OrdnanceDesc->hide();
		m_OrdnancePic->hide();
	}
	else
	{
		AuItem* item = AuItem::Unpack(selitems.first()->data(Qt::UserRole).toByteArray());
		if(item)
		{
			m_OrdnanceDesc->setText(item->HtmlDesc());
			m_OrdnancePic->setPixmap(item->BitmapName());

			m_OrdnanceDesc->show();
			m_OrdnancePic->show();
		}
		else
		{
			m_OrdnanceDesc->hide();
			m_OrdnancePic->hide();
		}

		if(DbCommodity::IsTurretModule(m_WeaponCID))
		{
			m_LoadGun->show();
			m_SelectAmmo->hide();
			m_LoadOrd0->hide();
			m_LoadOrd1->hide();
		}
		else if(DbCommodity::IsBayWeaponModule(m_WeaponCID))
		{
			m_LoadGun->hide();
			m_SelectAmmo->show();
			m_LoadOrd0->hide();
			m_LoadOrd1->hide();
		}
		else
		{
			m_SelectAmmo->hide();
			switch(DaVehicle::WeaponRack(item->CommodityId()))
			{
			case DaVehicle::RACK_None:
				m_LoadGun->hide();
				m_LoadOrd0->hide();
				m_LoadOrd1->hide();
				break;
			case DaVehicle::RACK_Gun:
				m_LoadGun->show();
				m_LoadOrd0->hide();
				m_LoadOrd1->hide();
				break;
			case DaVehicle::RACK_Ord0:
			case DaVehicle::RACK_Ord1:
				m_LoadGun->hide();
				m_LoadOrd0->show();
				m_LoadOrd1->show();
				break;
			}
		}

		delete item;
	}
}

void AuOrdnanceTransfer::LoadSelectedToGun()
{
	LoadSelectedOrdnance(DaVehicle::RACK_Gun);
}

void AuOrdnanceTransfer::LoadSelectedToOrd0()
{
	LoadSelectedOrdnance(DaVehicle::RACK_Ord0);
}

void AuOrdnanceTransfer::LoadSelectedToOrd1()
{
	LoadSelectedOrdnance(DaVehicle::RACK_Ord1);
}

void AuOrdnanceTransfer::LoadSelectedOrdnance(DaVehicle::WEAPON_RACK rack)
{
	if(rack == DaVehicle::RACK_None)
		return;

	QList<QTableWidgetItem*> selitems = m_OrdnanceTable->selectedItems();
	if(selitems.isEmpty())
		return;

	AuItem* item = AuItem::Unpack(selitems.first()->data(Qt::UserRole).toByteArray());
	if(!item)
		return;

	auApp->OrdnanceLoad(rack, *item);
	delete item;
}

QSize AuOrdnanceTransfer::sizeHint() const
{
	return QSize(800,700);
}

