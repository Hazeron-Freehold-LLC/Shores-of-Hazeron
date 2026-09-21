//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumissionorderwidget.cpp,v 1.28 2022/03/10 22:26:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumissionorderwidget.h"
//
#include "aucity.h"
#include "auapp.h"
#include "auclientglobal.h"
#include "aucommodityfilter.h"
#include "auframe.h"
#include "autablewidgetitem.h"
#include "auview.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldring.h"
#include "dewaypoint.h"
#include "dgdata.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTableWidget>
#include <QPushButton>

// Helper class used for the commodity list.
struct CommodityEntry
{
	COMMODITYID			m_CID;
	QString				m_IconName;
};

// Helper class used for the cities list.
struct CityEntry
{
	double				m_Distance;		// Straight-line distance to the city in feet.
	QString				m_Info;			// location description
	QString				m_Name;
	POLITICAL_STANCE	m_Stance;
};

// Helper class used for the planets list.
struct PlanetOrStarEntry
{
	double				m_Distance;		// Straight-line distance to the planet or star in feet.
	QString				m_Info;			// location description
	bool				m_IsStar;		// true if this is a star
	QString				m_Name;
	double				m_SortOrder;
};

// Helper class used for the stations list.
struct StationEntry
{
	double				m_Distance;		// Straight-line distance to the station in feet.
	QString				m_Info;			// location description
	QString				m_Name;
	POLITICAL_STANCE	m_Stance;
};

// Helper class used for the targets list.
struct TargetEntry
{
	AuId				m_Id;			// target atom id
	double				m_Distance;		// Straight-line distance to the target in feet.
	QString				m_Info;			// target description
	POLITICAL_STANCE	m_Stance;
};

// Helper class used for the systems list.
struct SystemEntry
{
	QString				m_Catalog;
	double				m_Distance;		// Straight-line distance in parsecs
	AuPoint				m_GalaxyLoc;
	int					m_Jumps;		// number of wormhole jumps to get there
	QString				m_Name;
	QString				m_Route;		// Description of the route.
	QString				m_Sector;		// Sector name.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuHarvestOpWidget::AuHarvestOpWidget(const QString& title, COMMODITYID module, COMMODITYID harvestCID, quint8 amountType,
		quint8 amountPercent, quint32 amountQty, QWidget* parent) : QGroupBox(title, parent)
{
	m_Commodities = new QComboBox(this);
	if(module == CID_AtmosphereCondenserBayModule)
	{
		for(int ci=0; ci<HarvestAtmosphereCount(); ci++)
		{
			COMMODITYID cid = HarvestAtmosphereCommodity(ci);
			m_Commodities->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
	}
	else if(module == CID_ParticleCollectorBayModule)
	{
		for(int ci=0; ci<HarvestParticleCount(); ci++)
		{
			COMMODITYID cid = HarvestParticleCommodity(ci);
			m_Commodities->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
	}
	else if(module == CID_ProspectorBayModule)
	{
		for(int ci=0; ci<HarvestProspectorCount(); ci++)
		{
			COMMODITYID cid = HarvestProspectorCommodity(ci);
			m_Commodities->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
	}
	else if(module == CID_SiphonBayModule)
	{
		for(int ci=0; ci<HarvestSiphonCount(); ci++)
		{
			COMMODITYID cid = HarvestSiphonCommodity(ci);
			m_Commodities->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
	}
	else if(module == CID_WildcatterBayModule)
	{
		for(int ci=0; ci<HarvestWildcatterCount(); ci++)
		{
			COMMODITYID cid = HarvestWildcatterCommodity(ci);
			m_Commodities->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
	}
	int defidx = m_Commodities->findData(harvestCID);
	if(defidx >= 0)
		m_Commodities->setCurrentIndex(defidx);
	if(module == CID_ParticleCollectorBayModule)
		m_Commodities->setToolTip(tr("<html>Select a commodity to harvest using %1. "
					"Preon type is irrelevent; "
					"star type determines the type of preons harvested.</html>").arg(g_Au.CommodityName(module)));
	else
		m_Commodities->setToolTip(tr("<html>Select a commodity to harvest using %1.</html>").arg(g_Au.CommodityName(module)));

	m_PercentRadio = new QRadioButton(tr("Percent of Hold Space"), this);
	m_PercentRadio->setToolTip(tr("<html>Harvest quantity is specified as a percentage of the total hold or fuel cell "
				"space. "
				"The operator will attempt to harvest commodities until the percentage of hold space is filled, "
				"fuel cell when harvesting hydrogen."
				"</html>"));
	m_PercentRadio->setChecked(amountType == 0);
	connect(m_PercentRadio, SIGNAL(clicked()), SLOT(EnableControls()));

	m_PercentLabel = new QLabel(tr("%1%").arg(amountPercent), this);

	m_PercentSlider = new QSlider(Qt::Horizontal, this);
	m_PercentSlider->setRange(1, 100);
	m_PercentSlider->setSingleStep(1);
	m_PercentSlider->setPageStep(5);
	m_PercentSlider->setValue(amountPercent);
	connect(m_PercentSlider, SIGNAL(valueChanged(int)), SLOT(PercentChanged(int)));

	m_AmountRadio = new QRadioButton(tr("Actual Quantity"), this);
	m_AmountRadio->setToolTip(tr("<html>Harvest quantity is specified as an actual number of units. "
				"The operator will attempt to harvest commodities until the quantity of hold space is filled, "
				"fuel cell when harvesting hydrogen."
				"</html>"));
	m_AmountRadio->setChecked(amountType == 1);
	connect(m_AmountRadio, SIGNAL(clicked()), SLOT(EnableControls()));

	m_AmountLabel = new QLabel(tr("Units"), this);

	m_AmountEdit = new QLineEdit(QVariant(amountQty).toString(), this);
	m_AmountEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_IdleRadio = new QRadioButton(tr("Remain Idle"), this);
	m_IdleRadio->setToolTip(tr("<html>Harvester remains idle. "
				"No harvesting actions are performed. "
				"</html>"));
	m_IdleRadio->setChecked(amountType == 2);
	connect(m_IdleRadio, SIGNAL(clicked()), SLOT(EnableControls()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_Commodities, row,0, 1,3);
	row++;
	lay->addWidget(m_PercentRadio, row,0, 1,3);
	row++;
	lay->addWidget(m_PercentSlider, row,1, 1,1);
	lay->addWidget(m_PercentLabel, row,2, 1,1);
	row++;
	lay->addWidget(m_AmountRadio, row,0, 1,3);
	row++;
	lay->addWidget(m_AmountEdit, row,1, 1,1);
	lay->addWidget(m_AmountLabel, row,2, 1,1);
	row++;
	lay->addWidget(m_IdleRadio, row,0, 1,3);
	lay->setColumnMinimumWidth(0, 40);
	lay->setSpacing(2);
	lay->setMargin(12);
	setLayout(lay);

	EnableControls();
}

void AuHarvestOpWidget::HarvestOp(AuHarvestOp& op)
{
	COMMODITYID cid = (COMMODITYID)m_Commodities->itemData(m_Commodities->currentIndex()).toInt();

	quint32 amount;
	quint8 amountType;
	if(m_PercentRadio->isChecked())
	{
		amount = m_PercentSlider->value();
		amountType = 0;	// percent
	}
	else if(m_AmountRadio->isChecked())
	{
		amount = m_AmountEdit->text().toUInt();
		amountType = 1;	// actual
	}
	else
	{
		amount = 0;
		amountType = 2;	// idle
	}

	op = AuHarvestOp(cid, amount, amountType);
}

void AuHarvestOpWidget::EnableControls()
{
	if(m_PercentRadio->isChecked())
	{
		m_AmountEdit->setEnabled(false);
		m_PercentSlider->setEnabled(true);
	}
	else if(m_AmountRadio->isChecked())
	{
		m_AmountEdit->setEnabled(true);
		m_PercentSlider->setEnabled(false);
	}
	else
	{
		m_AmountEdit->setEnabled(false);
		m_PercentSlider->setEnabled(false);
	}
}

void AuHarvestOpWidget::PercentChanged(int value)
{
	m_PercentLabel->setText(tr("%1%").arg(value));
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderWidget::AuMissionOrderWidget(const AuId& authorization, const AuId& solarSystem, const AuSensorBlipList& blips,
		const AuPoint& galaxyCoord, const AuPoint& sectorCoord, const AuPoint& systemCoord, const AuMissionOrder* order,
		QWidget* parent) : QWidget(parent), m_Ether("AuMissionOrderWidget::AuMissionOrderWidget")
{
	//qDebug("AuMissionOrderWidget system %d", solarSystem.GetID());
	// Initialize default values before getting values from 'order'.
	m_AmountBuyPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountBuyPercent", 50);
	m_AmountBuyType = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountBuyType", 0);
	m_AmountBuyUnits = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountBuyUnits", 0);
	m_AmountSellPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountSellPercent", 100);
	m_AmountSellType = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountSellType", 0);
	m_AmountSellUnits = g_Au.Settings().readNumEntry("MissionOrderWidget/AmountSellUnits", 0);
	m_Authorization = authorization;
	m_BayWhich = 256;
	m_Category = (COMMODITYFILTER)g_Au.Settings().readNumEntry("MissionOrderWidget/Category", FILTER_All);
	m_CatBuyType = g_Au.Settings().readNumEntry("MissionOrderWidget/CategoryBuyType", 0);
	m_CatSellType = g_Au.Settings().readNumEntry("MissionOrderWidget/CategorySellType", 0);
	m_CIDAtmo = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommodityAtmo", CID_Hydrogen);
	m_CIDBuy = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/Commodity", CID_Hydrogen);	// Hint how to get fuel.
	m_CIDMin = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommodityMin", CID_Lumenite);
	m_CIDPart = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommodityPart", CID_AntifluxParticles);
	m_CIDSea = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommoditySea", CID_Water);
	m_CIDTransfer = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommodityTransfer", CID_Hydrogen);
	m_CIDWell = (COMMODITYID)g_Au.Settings().readNumEntry("MissionOrderWidget/CommodityWell", CID_CrudeOil);
	m_ContentValid = false;
	m_Count = g_Au.Settings().readNumEntry("MissionOrderWidget/Count", 0);
	m_GalaxyCoord = galaxyCoord;
	m_GoToRelative = g_Au.Settings().readNumEntry("MissionOrderWidget/GoToRelative", 0);
	if(m_GoToRelative > 2)
		m_GoToRelative = 0;
	switch(m_GoToRelative)
	{
	case 0:
		m_GoTo = galaxyCoord;
		break;
	case 1:
		m_GoTo = sectorCoord;
		break;
	case 2:
		m_GoTo = systemCoord;
		break;
	}
	m_HarvestFuelThreshold = g_Au.Settings().readNumEntry("MissionOrderWidget/HarvestFuelThreshold", 10);	// percent
	m_HarvestFuelThresholdByM = g_Au.Settings().readNumEntry("MissionOrderWidget/HarvestFuelThresholdByM", 10);	// percent
	m_HoverAlt = g_Au.Settings().readNumEntry("MissionOrderWidget/HoverMeters", 50);	// meters
	m_IsDoOnce = false;
	m_IsMailCoC = false;
	m_LightStateHeadlight = g_Au.Settings().readBoolEntry("MissionOrderWidget/LightHeadlight", true);
	m_LightStateNavLight = g_Au.Settings().readBoolEntry("MissionOrderWidget/LightNavLight", true);
	m_LightStateRotatingBeacon = g_Au.Settings().readBoolEntry("MissionOrderWidget/LightRotatingBeacon", true);
	m_LightStateStrobe = g_Au.Settings().readBoolEntry("MissionOrderWidget/LightStrobe", true);
	m_MarginBuyCost = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginBuyCost", 0);
	m_MarginBuyPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginBuyPercent", 100);
	m_MarginBuyType = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginBuyType", 2);
	m_MarginSellCost = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginSellCost", 0);
	m_MarginSellPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginSellPercent", 100);
	m_MarginSellType = g_Au.Settings().readNumEntry("MissionOrderWidget/MarginSellType", 2);
	m_ApplyQ = g_Au.Settings().readNumEntry("MissionOrderWidget/ApplyQ", 0);
	m_MinQ = g_Au.Settings().readNumEntry("MissionOrderWidget/MinQ", 1);
	m_Minutes = g_Au.Settings().readNumEntry("MissionOrderWidget/Minutes", 15);
	m_Order = AuMissionOrder::Invalid;		// Init to invalid.
	m_OrderSystemId = solarSystem;			// May be null to use person's current system.
	m_PilotUpPolicy = (PILOT_UP_POLICY)g_Au.Settings().readNumEntry("MissionOrderWidget/PilotUpPolicy", UpAwayFromGravity);
	if(m_PilotUpPolicy >= PILOT_UP_POLICY_Count)
		m_PilotUpPolicy = UpAwayFromGravity;
	m_Posture = (CITIZEN_POSTURE)g_Au.Settings().readNumEntry("MissionOrderWidget/Posture", POSTURE_Offensive);
	if(m_Posture < POSTURE_Offensive || m_Posture > POSTURE_HoldFire)
		m_Posture = POSTURE_Offensive;
	m_PreboardTime = g_Au.Settings().readNumEntry("MissionOrderWidget/PreboardMinutes", 3);	// minutes
	m_ReportChannel = g_Au.Settings().readEntry("MissionOrderWidget/ReportChannel", "Hail");
	m_ReportRange = (AuCommStdChannel::RANGE)g_Au.Settings().readNumEntry("MissionOrderWidget/ReportRange",
			AuCommStdChannel::SolarSystem);
	m_ReportWhat = 0;
	m_ScannedSystem = 0;
	m_SensorBlips = blips;
	m_SectorCoord = sectorCoord;
	m_ShieldFuelPolicy = (SHIELDFUEL_POLICY)g_Au.Settings().readNumEntry("MissionOrderWidget/ShieldFuelPolicy",
			ShieldUpUnlessNoFuel);
	if(m_ShieldFuelPolicy < ShieldUpUnlessNoFuel || m_ShieldFuelPolicy > ShieldUpIgnoreFuel)
		m_ShieldFuelPolicy = ShieldUpUnlessNoFuel;
	m_ShieldPolicy = (SHIELD_POLICY)g_Au.Settings().readNumEntry("MissionOrderWidget/ShieldPolicy", ShieldUp);
	if(m_ShieldPolicy < ShieldUp || m_ShieldPolicy > ShieldUpIfEnemy)
		m_ShieldPolicy = ShieldUp;
	m_SystemCoord = systemCoord;
	m_Transferee = g_Au.Settings().readEntry("MissionOrderWidget/Transferee", "");
	m_TransferMoneyFromCount = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyFromCount", 0.0);
	m_TransferMoneyFromPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyFromPercent", 0);
	m_TransferMoneyFromType = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyFromType", 0);
	m_TransferMoneyToCount = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyToCount", 0.0);
	m_TransferMoneyToPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyToPercent", 0);
	m_TransferMoneyToType = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferMoneyToType", 0);
	m_TransferPeopleFromBerth = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleFromBerth", 0);
	m_TransferPeopleFromCount = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleFromCount", 0);
	m_TransferPeopleFromPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleFromPercent", 0);
	m_TransferPeopleFromType = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleFromType", 0);
	m_TransferPeopleToBerth = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleToBerth", 0);
	m_TransferPeopleToCount = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleToCount", 0);
	m_TransferPeopleToPercent = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleToPercent", 0);
	m_TransferPeopleToType = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferPeopleToType", 0);
	m_TransferTarget = g_Au.Settings().readNumEntry("MissionOrderWidget/TransferTarget", 0);

	// Set defaults based on 'order', if any.
	GetValuesFromOrder(order);

	// Make a map of the commodities so they are sorted alphabetically.
	QMap<QString, CommodityEntry> cids;
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); it++)
	{
		if((*it).m_id != CID_Electricity && !(*it).CanBeCargo())
			continue;	// Don't list things that can't be carried, except electricity.
		if((*it).IsObsolete())
			continue;	// Skip obsolete commodities.

		CommodityEntry cident;
		cident.m_IconName = (*it).BitmapName();
		cident.m_CID = (COMMODITYID)(*it).m_id;
		cids.insert((*it).m_name, cident);
	}

	m_Stack = new QStackedWidget(this);
	int pageIndex = 0;
	{
		// This is the monolithic list of controls from the old days, before using stacked widgets.
		m_DoOnce = new QCheckBox(tr("Do-Once Order"), this);
		m_DoOnce->setCheckState(m_IsDoOnce ? Qt::Checked : Qt::Unchecked);
		m_DoOnce->setToolTip(tr("<html>When checked, the order is only executed once, even if the mission is cyclic. "
					"This is useful when a temporary order is desired that does not permanently alter a spacecraft's mission "
					"programming.</html>"));

		m_MailCoC = new QCheckBox(tr("Report to Entire Chain-of-Command"), this);
		m_MailCoC->setCheckState(m_IsMailCoC ? Qt::Checked : Qt::Unchecked);
		m_MailCoC->setToolTip(tr("<html>When checked, the report is sent to the entire chain-of-command of the spacecraft, "
					"including the captain.<br>"
					"When unchecked, the report is only sent to the captain of the spacecraft.</html>"));

		// Radio controls 
		QVBoxLayout* systemRadios = new QVBoxLayout;
		{
			m_ShowAll = new QRadioButton("All Systems", this);
			m_ShowAll->setToolTip(tr("<html>All systems in the current sector and adjacent sectors are shown.</html>"));
			connect(m_ShowAll, SIGNAL(clicked()), SLOT(OrderSelected()));

			m_ShowNamed = new QRadioButton("Named Systems", this);
			m_ShowNamed->setToolTip(tr("<html>Only systems in the current sector and adjacent sectors that have names are "
						"shown.</html>"));
			m_ShowNamed->setChecked(true);		// This is the default.
			connect(m_ShowNamed, SIGNAL(clicked()), SLOT(OrderSelected()));

			systemRadios->addWidget(m_ShowAll);
			systemRadios->addWidget(m_ShowNamed);
			systemRadios->setSpacing(2);
			systemRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_ShowAll);
			bg->addButton(m_ShowNamed);
		}

		m_ShipNameLabel = new QLabel(tr("Spacecraft Name"), this);

		m_ShipNameEdit = new QLineEdit("", this);
		connect(m_ShipNameEdit, SIGNAL(textEdited(const QString&)), SLOT(ShipNameChanged(const QString&)));

		m_DurWaitLabel = new QLabel(tr("Wait Time"), this);

		m_DurPatrolLabel = new QLabel(tr("Patrol Time"), this);

		m_DurPreboardLabel = new QLabel(tr("Preboarding Announcement Time"), this);
		m_DurPreboardLabel->setToolTip(tr("<html>A preboarding announcement gives players in the area time to take "
					"a berth before those berths are filled with non-player citizens.</html>"));

		m_MinutesSpin = new QSpinBox(this);
		m_MinutesSpin->setMinimum(0);
		m_MinutesSpin->setMaximum(0xffff);
		m_MinutesSpin->setValue(m_Minutes);
		connect(m_MinutesSpin, SIGNAL(valueChanged(int)), SLOT(PatrolTimeChanged(int)));

		m_MinutesLabel = new QLabel(tr("Minutes Duration"), this);

		m_MinutesPreboardSpin = new QSpinBox(this);
		m_MinutesPreboardSpin->setMinimum(0);
		m_MinutesPreboardSpin->setMaximum(0xffff);
		m_MinutesPreboardSpin->setValue(m_PreboardTime);
		m_MinutesPreboardSpin->setToolTip(tr("<html>Pre-boarding announcement time in minutes. "
					"Zero results in the least time.</html>"));
		connect(m_MinutesPreboardSpin, SIGNAL(valueChanged(int)), SLOT(PreBoardTimeChanged(int)));

		m_MinutesPreboardLabel = new QLabel(tr("Minutes Prior to Boarding"), this);

		m_HoverAltLabel = new QLabel(tr("Hover Altitude"), this);

		m_MetersLabel = new QLabel(tr("Meters"), this);

		m_HoverAltSpin = new QSpinBox(this);
		m_HoverAltSpin->setMinimum(1);
		m_HoverAltSpin->setMaximum(0xffff);
		m_HoverAltSpin->setSingleStep(10);
		m_HoverAltSpin->setValue(m_HoverAlt);
		m_HoverAltSpin->setToolTip(tr("<html>Altitude to hover above the site in meters.</html>"));
		connect(m_HoverAltSpin, SIGNAL(valueChanged(int)), SLOT(HoverAltitudeChanged(int)));

		m_SellMarginSlider = new QSlider(Qt::Horizontal, this);
		m_SellMarginSlider->setRange(1, 250);
		m_SellMarginSlider->setSingleStep(1);
		m_SellMarginSlider->setPageStep(5);
		m_SellMarginSlider->setValue(m_MarginSellPercent);
		m_SellMarginSlider->setToolTip(tr("<html>Lowest offer to accept when selling, "
					"as a percentage of nominal commodity price.</html>"));
		connect(m_SellMarginSlider, SIGNAL(valueChanged(int)), SLOT(MarginSellChanged(int)));

		m_SellMarginLabel = new QLabel(this);
		m_SellMarginLabel->setText(tr("Accept No Less Than %1% of Nominal Price").arg(m_MarginSellPercent));

		m_TransferAuthorizationLabel = new QLabel(tr("<html><b>TRANSFER AUTHORITY</b><br><small>"
					"Transfer requires permission from the other spacecraft. "
					"Permission is granted if the other spacecraft is in the chain of command of the officer executing this order "
					"or the avatar who issued this order. "
					"The avatar's id is attached to the order for authorization. "
					"Transfer authorization for this order is <i>%1</i>."
					"</small></html>").arg(m_Authorization.StringId()), this);
		m_TransferAuthorizationLabel->setWordWrap(true);

		m_CoordsLabel = new QLabel(tr("Longitude and Latitude"), this);

		m_CoordsEdit = new QLineEdit("", this);
		m_CoordsEdit->setToolTip(tr("<html>Longitude and latitude location. Degree symbols optional. e.g. '123.4E 32.1N.</html>"));
		connect(m_CoordsEdit, SIGNAL(textEdited(const QString&)), SLOT(LongLatChanged(const QString&)));

		m_BuyCargoCountLabel = new QLabel(tr("Cargo to Buy"), this);

		m_SellCargoCountLabel = new QLabel(tr("Cargo to Sell"), this);

		m_CryoBerthsLoadCountLabel = new QLabel(tr("Cryo Travellers to Load"), this);

		m_PassengerLoadCountLabel = new QLabel(tr("Passengers to Load"), this);

		// Transfer target for cargo, money or people.
		QGridLayout* transferTargetLayout = new QGridLayout;
		{
			m_TransferFromLabel = new QLabel(tr("Transfer from Spacecraft"), this);

			m_TransferToLabel = new QLabel(tr("Transfer to Spacecraft"), this);

			m_TransferStation = new QRadioButton("Station", this);
			m_TransferStation->setToolTip(tr("<html>Transfer target is a station.<br>"
						"The spacecraft carrying out the order must be in trade range of a station.<br>"
						"To permit the transfer, the other spacecraft must be in the chain of command of the avatar with id <i>%1</i> "
						"or the officer executing the order."
						"</html>").arg(m_Authorization.StringId()));
			m_TransferStation->setChecked(m_TransferTarget == 0);
			connect(m_TransferStation, SIGNAL(clicked()), SLOT(TransferStation()));

			m_TransferNamedShip = new QRadioButton("Named Spacecraft", this);
			m_TransferNamedShip->setToolTip(tr("<html>Transfer target is a spacecraft, identified by name.<br>"
						"The spacecraft carrying out the order must be in trade range of the other spacecraft.<br>"
						"To permit the transfer, the other spacecraft must be in the chain of command of the avatar with id <i>%1</i> "
						"or the officer executing the order."
						"</html>").arg(m_Authorization.StringId()));
			m_TransferNamedShip->setChecked(m_TransferTarget == 1);
			connect(m_TransferNamedShip, SIGNAL(clicked()), SLOT(TransferNamedShip()));

			m_TransfereeEdit = new QLineEdit(m_Transferee, this);
			connect(m_TransfereeEdit, SIGNAL(textEdited(const QString&)), SLOT(TransfereeChanged(const QString&)));

			int row=0;
			transferTargetLayout->addWidget(m_TransferFromLabel, row,0, 1,2);
			row++;
			transferTargetLayout->addWidget(m_TransferToLabel, row,0, 1,2);
			row++;
			transferTargetLayout->addWidget(m_TransferStation, row,0, 1,2);
			row++;
			transferTargetLayout->addWidget(m_TransferNamedShip, row,0, 1,1);
			transferTargetLayout->addWidget(m_TransfereeEdit, row,1, 1,1);
			transferTargetLayout->setSpacing(2);
			transferTargetLayout->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_TransferStation);
			bg->addButton(m_TransferNamedShip);
		}

		m_TransferCargoCountLabel = new QLabel(tr("Cargo to Transfer"), this);

		m_TroopCountLabel = new QLabel(tr("Troops to Load"), this);

		m_CrewCountLabel = new QLabel(tr("Crew Members to Load"), this);

		m_CryoBerthsUnloadCountLabel = new QLabel(tr("Cryo Travellers to Unload"), this);

		m_PassengerUnloadCountLabel = new QLabel(tr("Passengers to Unload"), this);

		m_CountSpin = new QSpinBox(this);
		m_CountSpin->setMinimum(0);
		m_CountSpin->setMaximum(1000000);
		m_CountSpin->setValue(m_Count);
		connect(m_CountSpin, SIGNAL(valueChanged(int)), SLOT(CountChanged(int)));

		m_CommoditiesTransfer = new QComboBox(this);
		for(QMap<QString, CommodityEntry>::const_iterator ci = cids.cbegin(); ci != cids.cend(); ci++)
		{
			QIcon icon((*ci).m_IconName);
			m_CommoditiesTransfer->addItem(icon, ci.key(), (*ci).m_CID);
		}
		int defidx = m_CommoditiesTransfer->findData(m_CIDTransfer);
		m_CommoditiesTransfer->setCurrentIndex(defidx);
		m_CommoditiesTransfer->setToolTip(tr("<html>Select a commodity to transfer.</html>"));
		connect(m_CommoditiesTransfer, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedTransfer(int)));

		m_CommoditiesBuy = new QComboBox(this);
		for(QMap<QString, CommodityEntry>::const_iterator ci = cids.cbegin(); ci != cids.cend(); ci++)
		{
			if(DbCommodity::IsPatent((*ci).m_CID))
				continue;	// Don't list patents in the list of things to buy.
			QIcon icon((*ci).m_IconName);
			m_CommoditiesBuy->addItem(icon, ci.key(), (*ci).m_CID);
		}
		defidx = m_CommoditiesBuy->findData(m_CIDBuy);
		m_CommoditiesBuy->setCurrentIndex(defidx);
		m_CommoditiesBuy->setToolTip(tr("<html>Select a commodity.</html>"));
		connect(m_CommoditiesBuy, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedBuy(int)));

		m_CommoditiesAtmo = new QComboBox(this);
		for(int ci=0; ci<HarvestAtmosphereCount(); ci++)
		{
			COMMODITYID cid = HarvestAtmosphereCommodity(ci);
			m_CommoditiesAtmo->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
		defidx = m_CommoditiesAtmo->findData(m_CIDAtmo);
		m_CommoditiesAtmo->setCurrentIndex(defidx);
		m_CommoditiesAtmo->setToolTip(tr("<html>Select a commodity to harvest using atmosphere condenser bay.</html>"));
		connect(m_CommoditiesAtmo, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedAtmo(int)));

		m_CommoditiesMin = new QComboBox(this);
		for(int ci=0; ci<HarvestProspectorCount(); ci++)
		{
			COMMODITYID cid = HarvestProspectorCommodity(ci);
			m_CommoditiesMin->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
		defidx = m_CommoditiesMin->findData(m_CIDMin);
		m_CommoditiesMin->setCurrentIndex(defidx);
		m_CommoditiesMin->setToolTip(tr("<html>Select a commodity to harvest using prospector bay.</html>"));
		connect(m_CommoditiesMin, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedMin(int)));

		m_CommoditiesPart = new QComboBox(this);
		for(int ci=0; ci<HarvestParticleCount(); ci++)
		{
			COMMODITYID cid = HarvestParticleCommodity(ci);
			m_CommoditiesPart->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
		defidx = m_CommoditiesPart->findData(m_CIDPart);
		m_CommoditiesPart->setCurrentIndex(defidx);
		m_CommoditiesPart->setToolTip(tr("<html>Select a commodity to harvest using particle collector bay. "
					"Type of preon is irrelevant because the type of the star determines the type of preon collected.</html>"));
		connect(m_CommoditiesPart, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedPart(int)));

		m_CommoditiesSea = new QComboBox(this);
		for(int ci=0; ci<HarvestSiphonCount(); ci++)
		{
			COMMODITYID cid = HarvestSiphonCommodity(ci);
			m_CommoditiesSea->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
		defidx = m_CommoditiesSea->findData(m_CIDSea);
		m_CommoditiesSea->setCurrentIndex(defidx);
		m_CommoditiesSea->setToolTip(tr("<html>Select a commodity to harvest using siphon bay.</html>"));
		connect(m_CommoditiesSea, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedSea(int)));

		m_CommoditiesWell = new QComboBox(this);
		for(int ci=0; ci<HarvestWildcatterCount(); ci++)
		{
			COMMODITYID cid = HarvestWildcatterCommodity(ci);
			m_CommoditiesWell->addItem(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid), cid);
		}
		defidx = m_CommoditiesWell->findData(m_CIDWell);
		m_CommoditiesWell->setCurrentIndex(defidx);
		m_CommoditiesWell->setToolTip(tr("<html>Select a commodity to harvest using wildcatter bay.</html>"));
		connect(m_CommoditiesWell, SIGNAL(currentIndexChanged(int)), SLOT(CommodityChangedWell(int)));

		m_Bays = new QComboBox(this);
		m_Bays->setToolTip(tr("<html>Select equipment bay to configure. "
					"'All Bays' configures harvester bays appropriate to the commodity harvested. "
					"Other bays are not affected.</html>"));
		QIcon iconBay(":data/dp_WeaponBay.png");
		for(int bay=255; bay>=0; bay--)
			m_Bays->addItem(iconBay, tr("Bay %1").arg(bay+1), bay);
		m_Bays->addItem(iconBay, tr("All Bays"), 256);	// 256 is not a valid bay room number, which is a quint8 in the ship design.
		defidx = m_Bays->findData(m_BayWhich);
		m_Bays->setCurrentIndex(defidx);
		connect(m_Bays, SIGNAL(currentIndexChanged(int)), SLOT(BayChanged(int)));

		m_Postures = new QComboBox(this);
		m_Postures->addItem(QIcon(PostureIcon(POSTURE_Berserk)),
				tr("Berserk - Pursue and attack targets in sensor range until red alert."), POSTURE_Berserk);
		m_Postures->addItem(QIcon(PostureIcon(POSTURE_Offensive)),
				tr("Offensive - Pursue and attack targets in sensor range until yellow alert."), POSTURE_Offensive);
		m_Postures->addItem(QIcon(PostureIcon(POSTURE_DefendArea)),
				tr("Defend Area - Attack targets in weapon range until yellow alert."), POSTURE_DefendArea);
		m_Postures->addItem(QIcon(PostureIcon(POSTURE_DefendSelf)),
				tr("Defend Self - Return attacks from targets in weapon range until yellow alert."), POSTURE_DefendSelf);
		m_Postures->addItem(QIcon(PostureIcon(POSTURE_HoldFire)),
				tr("Hold Fire - Do not attack any targets."), POSTURE_HoldFire);
		defidx = m_Postures->findData(m_Posture);
		m_Postures->setCurrentIndex(defidx);
		m_Postures->setToolTip(tr("<html>Select a posture.</html>"));
		connect(m_Postures, SIGNAL(currentIndexChanged(int)), SLOT(PostureChanged(int)));

		// We can only show the cities list if the system context is the same as the scene atom's current solar system.
		{
			m_Airports = new QTableWidget(this);
			m_Airports->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Airports->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Airports->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Airports->setShowGrid(false);
			QStringList hls;
			hls << tr("Airport") << tr("Stance") << tr("Distance") << tr("Info");
			m_Airports->setColumnCount(hls.count());
			m_Airports->setHorizontalHeaderLabels(hls);
			m_Airports->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Airports->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Airports->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Airports->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
			m_Airports->setColumnWidth(0, 150);
			m_Airports->setColumnWidth(1, 50);
			m_Airports->setColumnWidth(2, 80);
			QHeaderView* vhdr = m_Airports->verticalHeader();
			vhdr->hide();
			connect(m_Airports, SIGNAL(itemSelectionChanged()), SLOT(AirportChanged()));
			m_Airports->setToolTip(tr("<html>Select an airport in your area.</html>"));
		
			// Cities table is filled on demand because the scene atom's location can change.
		}

		// We can only show the cities list if the system context is the same as the scene atom's current solar system.
		{
			m_Cities = new QTableWidget(this);
			m_Cities->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Cities->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Cities->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Cities->setShowGrid(false);
			QStringList hls;
			hls << tr("City") << tr("Stance") << tr("Distance") << tr("Info");
			m_Cities->setColumnCount(hls.count());
			m_Cities->setHorizontalHeaderLabels(hls);
			m_Cities->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Cities->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Cities->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Cities->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
			m_Cities->setColumnWidth(0, 150);
			m_Cities->setColumnWidth(1, 50);
			m_Cities->setColumnWidth(2, 80);
			QHeaderView* vhdr = m_Cities->verticalHeader();
			vhdr->hide();
			connect(m_Cities, SIGNAL(itemSelectionChanged()), SLOT(CityChanged()));
			m_Cities->setToolTip(tr("<html>Select a city in your area.</html>"));
		
			// Cities table is filled on demand because the scene atom's location can change.
		}

		{
			m_PlanetsAndStars = new QTableWidget(this);
			m_PlanetsAndStars->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_PlanetsAndStars->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_PlanetsAndStars->setSelectionMode(QAbstractItemView::SingleSelection);
			m_PlanetsAndStars->setShowGrid(false);
			QStringList hls;
			hls << tr("Planet or Star") << tr("Distance") << tr("Info");
			m_PlanetsAndStars->setColumnCount(hls.count());
			m_PlanetsAndStars->setHorizontalHeaderLabels(hls);
			m_PlanetsAndStars->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_PlanetsAndStars->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_PlanetsAndStars->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			m_PlanetsAndStars->setColumnWidth(0, 150);
			m_PlanetsAndStars->setColumnWidth(1, 80);
			QHeaderView* vhdr = m_PlanetsAndStars->verticalHeader();
			vhdr->hide();
			connect(m_PlanetsAndStars, SIGNAL(itemSelectionChanged()), SLOT(PlanetChanged()));
			m_PlanetsAndStars->setToolTip(tr("<html>Select a planet or star.</html>"));

			// Planets table is filled on demand because the scene atom's location can change.
		}

		{
			m_PlanetsMin = new QTableWidget(this);
			m_PlanetsMin->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_PlanetsMin->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_PlanetsMin->setSelectionMode(QAbstractItemView::SingleSelection);
			m_PlanetsMin->setShowGrid(false);
			QStringList hls;
			hls << tr("World") << tr("Distance") << tr("Info");
			m_PlanetsMin->setColumnCount(hls.count());
			m_PlanetsMin->setHorizontalHeaderLabels(hls);
			m_PlanetsMin->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_PlanetsMin->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_PlanetsMin->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			m_PlanetsMin->setColumnWidth(0, 150);
			m_PlanetsMin->setColumnWidth(1, 80);
			QHeaderView* vhdr = m_PlanetsMin->verticalHeader();
			vhdr->hide();
			connect(m_PlanetsMin, SIGNAL(itemSelectionChanged()), SLOT(PlanetChangedMin()));
			m_PlanetsMin->setToolTip(tr("<html>Select a planet, moon, ringworld or ring from which to collect minerals.</html>"));

			// Planets table is filled on demand because the scene atom's location can change.
		}

		{
			m_PlanetsSea = new QTableWidget(this);
			m_PlanetsSea->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_PlanetsSea->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_PlanetsSea->setSelectionMode(QAbstractItemView::SingleSelection);
			m_PlanetsSea->setShowGrid(false);
			QStringList hls;
			hls << tr("Planet") << tr("Distance") << tr("Info");
			m_PlanetsSea->setColumnCount(hls.count());
			m_PlanetsSea->setHorizontalHeaderLabels(hls);
			m_PlanetsSea->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_PlanetsSea->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_PlanetsSea->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			m_PlanetsSea->setColumnWidth(0, 150);
			m_PlanetsSea->setColumnWidth(1, 80);
			QHeaderView* vhdr = m_PlanetsSea->verticalHeader();
			vhdr->hide();
			connect(m_PlanetsSea, SIGNAL(itemSelectionChanged()), SLOT(PlanetChangedSea()));
			m_PlanetsSea->setToolTip(tr("<html>Select a planet, moon or ringworld from which to collect sea resources.</html>"));

			// Planets table is filled on demand because the scene atom's location can change.
		}

		{
			m_PlanetsWell = new QTableWidget(this);
			m_PlanetsWell->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_PlanetsWell->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_PlanetsWell->setSelectionMode(QAbstractItemView::SingleSelection);
			m_PlanetsWell->setShowGrid(false);
			QStringList hls;
			hls << tr("World") << tr("Distance") << tr("Info");
			m_PlanetsWell->setColumnCount(hls.count());
			m_PlanetsWell->setHorizontalHeaderLabels(hls);
			m_PlanetsWell->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_PlanetsWell->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_PlanetsWell->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			m_PlanetsWell->setColumnWidth(0, 150);
			m_PlanetsWell->setColumnWidth(1, 80);
			QHeaderView* vhdr = m_PlanetsWell->verticalHeader();
			vhdr->hide();
			connect(m_PlanetsWell, SIGNAL(itemSelectionChanged()), SLOT(PlanetChangedWell()));
			m_PlanetsWell->setToolTip(tr("<html>Select a planet, moon or ringworld from which to collect well resources.</html>"));

			// Planets table is filled on demand because the scene atom's location can change.
		}

		{
			m_PlanetsPart = new QTableWidget(this);
			m_PlanetsPart->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_PlanetsPart->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_PlanetsPart->setSelectionMode(QAbstractItemView::SingleSelection);
			m_PlanetsPart->setShowGrid(false);
			QStringList hls;
			hls << tr("Sun, Ring or Gas Giant") << tr("Distance") << tr("Info");
			m_PlanetsPart->setColumnCount(hls.count());
			m_PlanetsPart->setHorizontalHeaderLabels(hls);
			m_PlanetsPart->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_PlanetsPart->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_PlanetsPart->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
			m_PlanetsPart->setColumnWidth(0, 150);
			m_PlanetsPart->setColumnWidth(1, 80);
			QHeaderView* vhdr = m_PlanetsPart->verticalHeader();
			vhdr->hide();
			connect(m_PlanetsPart, SIGNAL(itemSelectionChanged()), SLOT(PlanetChangedPart()));
			m_PlanetsPart->setToolTip(tr("<html>Select a sun, ring or gas giant from which to collect particles.</html>"));

			// Planets table is filled on demand because the scene atom's location can change.
		}

		{
			m_Stations = new QTableWidget(this);
			m_Stations->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Stations->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Stations->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Stations->setShowGrid(false);
			QStringList hls;
			hls << tr("Station") << tr("Stance") << tr("Distance") << tr("Info");
			m_Stations->setColumnCount(hls.count());
			m_Stations->setHorizontalHeaderLabels(hls);
			m_Stations->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Stations->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Stations->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Stations->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
			m_Stations->setColumnWidth(0, 150);
			m_Stations->setColumnWidth(1, 50);
			m_Stations->setColumnWidth(2, 80);
			QHeaderView* vhdr = m_Stations->verticalHeader();
			vhdr->hide();
			connect(m_Stations, SIGNAL(itemSelectionChanged()), SLOT(StationChanged()));
			m_Stations->setToolTip(tr("<html>Select a station in your area.</html>"));

			// Stations table is filled on demand because the scene atom's location can change.
		}

		{
			m_Targets = new QTableWidget(this);
			m_Targets->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Targets->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Targets->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Targets->setShowGrid(false);
			QStringList hls;
			hls << tr("Unit") << tr("Stance") << tr("Distance") << tr("Info");
			m_Targets->setColumnCount(hls.count());
			m_Targets->setHorizontalHeaderLabels(hls);
			m_Targets->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Targets->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Targets->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Targets->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
			m_Targets->setColumnWidth(0, 150);
			m_Targets->setColumnWidth(1, 50);
			m_Targets->setColumnWidth(2, 80);
			QHeaderView* vhdr = m_Targets->verticalHeader();
			vhdr->hide();
			connect(m_Targets, SIGNAL(itemSelectionChanged()), SLOT(TargetChanged()));
			m_Targets->setToolTip(tr("<html>Select a unit.</html>"));

			// Targets table is filled on demand because the scene atom's location can change.
		}

		// Table of systems with known routes.
		{
			m_Routes = new QTableWidget(this);
			m_Routes->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Routes->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Routes->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Routes->setShowGrid(false);
			QStringList hls;
			hls << tr("Solar System") << tr("Catalog") << tr("Sector") << tr("Range") << tr("Jumps") << tr("Route");
			m_Routes->setColumnCount(hls.count());
			m_Routes->setHorizontalHeaderLabels(hls);
			m_Routes->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Routes->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Routes->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Routes->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
			m_Routes->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
			m_Routes->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
			m_Routes->setColumnWidth(0, 150);
			m_Routes->setColumnWidth(1, 100);
			m_Routes->setColumnWidth(2, 80);
			m_Routes->setColumnWidth(3, 60);
			m_Routes->setColumnWidth(4, 50);
			QHeaderView* vhdr = m_Routes->verticalHeader();
			vhdr->hide();
			connect(m_Routes, SIGNAL(itemSelectionChanged()), SLOT(RouteSystemChanged()));
			m_Routes->setToolTip(tr("<html>Select a solar system.</html>"));
		
			// Systems table is filled on demand because the scene atom's location can change.
		}

		// Table of all systems without route information.
		{
			m_Systems = new QTableWidget(this);
			m_Systems->setEditTriggers(QAbstractItemView::NoEditTriggers);
			m_Systems->setSelectionBehavior(QAbstractItemView::SelectRows);
			m_Systems->setSelectionMode(QAbstractItemView::SingleSelection);
			m_Systems->setShowGrid(false);
			QStringList hls;
			hls << tr("Solar System") << tr("Catalog") << tr("Sector") << tr("Range");
			m_Systems->setColumnCount(hls.count());
			m_Systems->setHorizontalHeaderLabels(hls);
			m_Systems->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
			m_Systems->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
			m_Systems->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
			m_Systems->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
			m_Systems->setColumnWidth(0, 150);
			m_Systems->setColumnWidth(1, 100);
			m_Systems->setColumnWidth(2, 80);
			QHeaderView* vhdr = m_Systems->verticalHeader();
			vhdr->hide();
			connect(m_Systems, SIGNAL(itemSelectionChanged()), SLOT(SystemChanged()));
			m_Systems->setToolTip(tr("<html>Select a solar system.</html>"));
		
			// Systems table is filled on demand because the scene atom's location can change.
		}

		m_CommodityFilter = new AuCommodityFilter;
		m_CommodityFilter->SetFilter(m_Category);
		connect(m_CommodityFilter, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(CategoryChanged(COMMODITYFILTER)));

		QVBoxLayout* minQRadios = new QVBoxLayout;
		{
			m_MinQLabel = new QLabel(tr("Minimum Replacement Part Quality"), this);

			m_ApplySystemQRadio = new QRadioButton(tr("System Module Quality"), this);
			m_ApplySystemQRadio->setToolTip(tr("<html>When repairs are performed by an officer, the lowest replacement part "
						"quality will be system module quality. "
						"<br>"
						"%1 Officer specifies minimum part quality when performing repairs."
						"<br>"
						"%1 Officer repairs systems one by one, to prevent degradation of module quality."
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_ApplySystemQRadio->setChecked(m_ApplyQ == 0);
			connect(m_ApplySystemQRadio, SIGNAL(clicked()), SLOT(ApplyQSelected()));

			m_ApplyShipQRadio = new QRadioButton(tr("Spacecraft Manufacture Quality"), this);
			m_ApplyShipQRadio->setToolTip(tr("<html>When repairs are performed by an officer, the lowest replacement part quality "
						"will be the manufacture quality of the spacecraft. "
						"<br>"
						"%1 Officer specifies minimum part quality when performing repairs."
						"<br>"
						"%1 <b>The quality of a system's module is reduced to the lowest quality part installed into the "
						"system.</b>"
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_ApplyShipQRadio->setChecked(m_ApplyQ == 1);
			connect(m_ApplyShipQRadio, SIGNAL(clicked()), SLOT(ApplyQSelected()));

			m_ApplyMinQRadio = new QRadioButton(tr("Specified Minimum Quality"), this);
			m_ApplyMinQRadio->setToolTip(tr("<html>When repairs are performed by an officer, the lowest replacement part quality "
						"will be the value specified. "
						"<br>"
						"%1 Officer specifies minimum part quality when performing repairs."
						"<br>"
						"%1 <b>The quality of a system's module is reduced to the lowest quality part installed into the "
						"system.</b>"
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_ApplyMinQRadio->setChecked(m_ApplyQ == 2);
			connect(m_ApplyMinQRadio, SIGNAL(clicked()), SLOT(ApplyQSelected()));

			m_MinQSpin = new QSpinBox(this);
			m_MinQSpin->setMinimum(1);
			m_MinQSpin->setMaximum(255);
			m_MinQSpin->setValue(m_MinQ);
			m_MinQSpin->setToolTip(tr("<html>"
						"Minimum replacement part quality."
						"</html>"));
			connect(m_MinQSpin, SIGNAL(valueChanged(int)), SLOT(MinQChanged(int)));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_ApplyShipQRadio);
			bg->addButton(m_ApplyMinQRadio);
			bg->addButton(m_ApplySystemQRadio);

			QHBoxLayout* slay = new QHBoxLayout;
			slay->addStretch(1);
			slay->addWidget(m_MinQSpin);
			slay->addStretch(3);

			minQRadios->addWidget(m_MinQLabel);
			minQRadios->addWidget(m_ApplySystemQRadio);
			minQRadios->addWidget(m_ApplyShipQRadio);
			minQRadios->addWidget(m_ApplyMinQRadio);
			minQRadios->addLayout(slay);
			minQRadios->setSpacing(2);
			minQRadios->setMargin(0);
		}

		QVBoxLayout* coordRadios = new QVBoxLayout;
		{
			m_GoToGalaxy = new QRadioButton(tr("Galaxy"), this);
			m_GoToGalaxy->setToolTip(tr("<html>Coordinate is specified relative to the galaxy. "
						"The coordinate system origin is the center of the current galaxy; "
						"the XY plane is parallel to the galactic plane.<br> "
						"%1 Positive X is east and negative X is west in the galactic plane.<br>"
						"%1 Positive Y is north and negative Y is south in the galactic plane.<br>"
						"%1 Positive Z is above and negative Z is below the galactic plane.<br>"
						"%1 The unit of measure is parsecs (pc).<br>"
						"%1 Not valid in arena."
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_GoToGalaxy->setChecked(m_GoToRelative == 0);
			connect(m_GoToGalaxy, SIGNAL(clicked()), SLOT(GoToSelected()));

			m_GoToSector = new QRadioButton(tr("Sector"), this);
			m_GoToSector->setToolTip(tr("<html>Coordinate is specified relative to the sector. "
						"The coordinate system origin is the center of the current sector; "
						"the XY plane is parallel to the galactic plane.<br> "
						"%1 Positive X is east and negative X is west in the galactic plane.<br>"
						"%1 Positive Y is north and negative Y is south in the galactic plane.<br>"
						"%1 Positive Z is above and negative Z is below the galactic plane.<br>"
						"%1 The unit of measure is parsecs (pc).<br>"
						"%1 Not valid in arena."
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_GoToSector->setChecked(m_GoToRelative == 1);
			connect(m_GoToSector, SIGNAL(clicked()), SLOT(GoToSelected()));

			m_GoToSystem = new QRadioButton(tr("Solar System"), this);
			m_GoToSystem->setToolTip(tr("<html>Coordinate is specified relative to the solar system. "
						"The coordinate system origin is at the center of the primary star; "
						"the XY plane is parallel to the plane of the ecliptic.<br>"
						"%1 Positive X is east and negative X is west in the ecliptic plane.<br>"
						"%1 Positive Y is north and negative Y is south in the ecliptic plane.<br>"
						"%1 Positive Z is above and negative Z is below the ecliptic plane.<br>"
						"%1 The unit of measure is astronomical units (au).<br>"
						"%1 Arena radius is .125au."
						"</html>").arg(g_Au.MiddleDotSymbol()));
			m_GoToSystem->setChecked(m_GoToRelative == 2);
			connect(m_GoToSystem, SIGNAL(clicked()), SLOT(GoToSelected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_GoToGalaxy);
			bg->addButton(m_GoToSector);
			bg->addButton(m_GoToSystem);

			coordRadios->addWidget(m_GoToGalaxy);
			coordRadios->addWidget(m_GoToSector);
			coordRadios->addWidget(m_GoToSystem);
			coordRadios->setSpacing(2);
			coordRadios->setMargin(0);
		}

		QHBoxLayout* coords = new QHBoxLayout;
		{
			m_GoToCoordLabel = new QLabel(tr("Coordinate"), this);

			QString gotoCoord = QString("%1, %2, %3")
					.arg(m_GoTo.x, 0,'f',4)
					.arg(m_GoTo.y, 0,'f',4)
					.arg(m_GoTo.z, 0,'f',4);
			m_GoToCoord = new QLineEdit(gotoCoord, this);
			m_GoToCoord->setToolTip(tr("<html>Enter the Cartesian <b>X,Y,Z</b> coordinate.<br>"
						"%1 <b>Galaxy</b> or <b>Sector</b> unit of measure is parsecs (pc).<br>"
						"%1 <b>Solar System</b> unit of measure is astronomical units (au). Arena radius is .125au."
						"</html>").arg(g_Au.MiddleDotSymbol()));
			connect(m_GoToCoord, SIGNAL(textEdited(const QString&)), SLOT(GoToCoordChanged(const QString&)));

			coords->addWidget(m_GoToCoordLabel);
			coords->addWidget(m_GoToCoord);
			coords->setSpacing(2);
			coords->setMargin(0);
		}

		QVBoxLayout* lightHeadlightRadios = new QVBoxLayout;
		{
			m_LightHeadlightLabel = new QLabel(tr("Headlight"), this);

			m_LightHeadlightOff = new QRadioButton(tr("Off"), this);
			m_LightHeadlightOff->setToolTip(tr("<html>Headlight will be switched off."
						"</html>"));
			m_LightHeadlightOff->setChecked(m_LightStateHeadlight == false);
			connect(m_LightHeadlightOff, SIGNAL(clicked()), SLOT(LightHeadlightOffSelected()));

			m_LightHeadlightOn = new QRadioButton(tr("On"), this);
			m_LightHeadlightOn->setToolTip(tr("<html>Headlight will be switched on."
						"</html>"));
			m_LightHeadlightOn->setChecked(m_LightStateHeadlight == true);
			connect(m_LightHeadlightOn, SIGNAL(clicked()), SLOT(LightHeadlightOnSelected()));

			lightHeadlightRadios->addWidget(m_LightHeadlightLabel);
			lightHeadlightRadios->addWidget(m_LightHeadlightOff);
			lightHeadlightRadios->addWidget(m_LightHeadlightOn);
			lightHeadlightRadios->setSpacing(2);
			lightHeadlightRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_LightHeadlightOff);
			bg->addButton(m_LightHeadlightOn);
		}

		QVBoxLayout* lightNavLightRadios = new QVBoxLayout;
		{
			m_LightNavLightLabel = new QLabel(tr("Navigation Lights"), this);

			m_LightNavLightOff = new QRadioButton(tr("Off"), this);
			m_LightNavLightOff->setToolTip(tr("<html>Nav lights will be switched off."
						"</html>"));
			m_LightNavLightOff->setChecked(m_LightStateNavLight == false);
			connect(m_LightNavLightOff, SIGNAL(clicked()), SLOT(LightNavLightOffSelected()));

			m_LightNavLightOn = new QRadioButton(tr("On"), this);
			m_LightNavLightOn->setToolTip(tr("<html>Nav lights will be switched on."
						"</html>"));
			m_LightNavLightOn->setChecked(m_LightStateNavLight == true);
			connect(m_LightNavLightOn, SIGNAL(clicked()), SLOT(LightNavLightOnSelected()));

			lightNavLightRadios->addWidget(m_LightNavLightLabel);
			lightNavLightRadios->addWidget(m_LightNavLightOff);
			lightNavLightRadios->addWidget(m_LightNavLightOn);
			lightNavLightRadios->setSpacing(2);
			lightNavLightRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_LightNavLightOff);
			bg->addButton(m_LightNavLightOn);
		}

		QVBoxLayout* lightRotatingBeaconRadios = new QVBoxLayout;
		{
			m_LightRotatingBeaconLabel = new QLabel(tr("Rotating Beacons"), this);

			m_LightRotatingBeaconOff = new QRadioButton(tr("Off"), this);
			m_LightRotatingBeaconOff->setToolTip(tr("<html>Rotating beacons will be switched off."
						"</html>"));
			m_LightRotatingBeaconOff->setChecked(m_LightStateRotatingBeacon == false);
			connect(m_LightRotatingBeaconOff, SIGNAL(clicked()), SLOT(LightRotatingBeaconOffSelected()));

			m_LightRotatingBeaconOn = new QRadioButton(tr("On"), this);
			m_LightRotatingBeaconOn->setToolTip(tr("<html>Rotating beacons will be switched on."
						"</html>"));
			m_LightRotatingBeaconOn->setChecked(m_LightStateRotatingBeacon == true);
			connect(m_LightRotatingBeaconOn, SIGNAL(clicked()), SLOT(LightRotatingBeaconOnSelected()));

			lightRotatingBeaconRadios->addWidget(m_LightRotatingBeaconLabel);
			lightRotatingBeaconRadios->addWidget(m_LightRotatingBeaconOff);
			lightRotatingBeaconRadios->addWidget(m_LightRotatingBeaconOn);
			lightRotatingBeaconRadios->setSpacing(2);
			lightRotatingBeaconRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_LightRotatingBeaconOff);
			bg->addButton(m_LightRotatingBeaconOn);
		}

		QVBoxLayout* lightStrobeRadios = new QVBoxLayout;
		{
			m_LightStrobeLabel = new QLabel(tr("Strobes"), this);

			m_LightStrobeOff = new QRadioButton(tr("Off"), this);
			m_LightStrobeOff->setToolTip(tr("<html>Strobes will be switched off."
						"</html>"));
			m_LightStrobeOff->setChecked(m_LightStateStrobe == false);
			connect(m_LightStrobeOff, SIGNAL(clicked()), SLOT(LightStrobeOffSelected()));

			m_LightStrobeOn = new QRadioButton(tr("On"), this);
			m_LightStrobeOn->setToolTip(tr("<html>Strobes will be switched on."
						"</html>"));
			m_LightStrobeOn->setChecked(m_LightStateStrobe == true);
			connect(m_LightStrobeOn, SIGNAL(clicked()), SLOT(LightStrobeOnSelected()));

			lightStrobeRadios->addWidget(m_LightStrobeLabel);
			lightStrobeRadios->addWidget(m_LightStrobeOff);
			lightStrobeRadios->addWidget(m_LightStrobeOn);
			lightStrobeRadios->setSpacing(2);
			lightStrobeRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_LightStrobeOff);
			bg->addButton(m_LightStrobeOn);
		}

		m_PilotUpPolicies = new QComboBox(this);
		m_PilotUpPolicies->addItem(tr("Fly ship upright at all times."), UpAwayFromGravity);
		m_PilotUpPolicies->addItem(tr("Fly ship inverted in space."), UpTowardGravity);
		m_PilotUpPolicies->addItem(tr("Fly ship port side to gravity well."), PortTowardGravity);
		m_PilotUpPolicies->addItem(tr("Fly ship starboard side to gravity well."), StarboardTowardGravity);
		defidx = m_PilotUpPolicies->findData(m_PilotUpPolicy);
		m_PilotUpPolicies->setCurrentIndex(defidx);
		m_PilotUpPolicies->setToolTip(tr("<html><b>Ship Orientation Policy</b><br>"
					"This policy tells the pilot which direction to orient the up direction of the ship. "
					"This policy only applies when the ship is outside the <i>atmosphere range</i> of the nearest world, "
					"regardless of whether the world actually has an atmosphere.</html>"));
		connect(m_PilotUpPolicies, SIGNAL(currentIndexChanged(int)), SLOT(PilotUpPolicyChanged(int)));

		m_ShieldPolicies = new QComboBox(this);
		m_ShieldPolicies->addItem(tr("Shields up at all times."), ShieldUp);
		m_ShieldPolicies->addItem(tr("Shields down at all times."), ShieldDown);
		m_ShieldPolicies->addItem(tr("Shields down unless any contacts are detected."), ShieldUpIfAnyone);
		m_ShieldPolicies->addItem(tr("Shields down unless non-empire contacts are detected."), ShieldUpIfForeign);
		m_ShieldPolicies->addItem(tr("Shields down unless neutral or enemy contacts are detected."), ShieldUpIfNeutral);
		m_ShieldPolicies->addItem(tr("Shields down unless enemy contacts are detected."), ShieldUpIfEnemy);
		defidx = m_ShieldPolicies->findData(m_ShieldPolicy);
		m_ShieldPolicies->setCurrentIndex(defidx);
		m_ShieldPolicies->setToolTip(tr("<html>Select a shield policy. "
					"The shield operator prefers to keep the shields down to save power. "
					"This policy tells the shield operator what conditions require the shields to be raised."
					"</html>"));
		connect(m_ShieldPolicies, SIGNAL(currentIndexChanged(int)), SLOT(ShieldPolicyChanged(int)));

		m_ShieldFuelPolicies = new QComboBox(this);
		m_ShieldFuelPolicies->addItem(tr("Ignore fuel status of ship when raising shields."), ShieldUpIgnoreFuel);
		m_ShieldFuelPolicies->addItem(tr("Do not raise shields when ship has no fuel."), ShieldUpUnlessNoFuel);
		defidx = m_ShieldFuelPolicies->findData(m_ShieldFuelPolicy);
		m_ShieldFuelPolicies->setCurrentIndex(defidx);
		m_ShieldFuelPolicies->setToolTip(tr("<html>Select a shield fuel policy.</html>"));
		connect(m_ShieldFuelPolicies, SIGNAL(currentIndexChanged(int)), SLOT(ShieldFuelPolicyChanged(int)));

		m_Smashie = new QWidget(this);
		m_Smashie->setMinimumSize(QSize(0,0));
		m_Smashie->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

		QHBoxLayout* amountBuyRadios = new QHBoxLayout;
		{
			m_AmountBuyLabel = new QLabel(tr("Manifest Quantity"), this);

			m_AmountTransferFromLabel = new QLabel(tr("Transfer Quantity"), this);

			m_AmountBuyPercentRadio = new QRadioButton(tr("Percent of Hold Space"), this);
			m_AmountBuyPercentRadio->setToolTip(tr("<html>Manifest quantity is specified as a percentage of the total hold space. "
						"The officer will attempt to purchase commodities until the percentage of hold space is filled."
						"</html>"));
			m_AmountBuyPercentRadio->setChecked(m_AmountBuyType == 0);
			connect(m_AmountBuyPercentRadio, SIGNAL(clicked()), SLOT(AmountBuy0Selected()));

			m_AmountBuyActualRadio = new QRadioButton(tr("Actual Quantity"), this);
			m_AmountBuyActualRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of units. "
						"The officer will attempt to purchase commodities until the purchase quantity is filled."
						"</html>"));
			m_AmountBuyActualRadio->setChecked(m_AmountBuyType == 1);
			connect(m_AmountBuyActualRadio, SIGNAL(clicked()), SLOT(AmountBuy1Selected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_AmountBuyPercentRadio);
			bg->addButton(m_AmountBuyActualRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_AmountBuyLabel);
			radioLay->addWidget(m_AmountTransferFromLabel);
			radioLay->addWidget(m_AmountBuyPercentRadio);
			radioLay->addWidget(m_AmountBuyActualRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			m_BuyCargoAmountLabel = new QLabel(tr("Total Units to Acquire"), this);

			m_BuyCargoAmountEdit = new QLineEdit(QVariant(m_AmountBuyUnits).toString(), this);
			connect(m_BuyCargoAmountEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoBuyAmountChanged(const QString&)));

			m_BuyCargoSlider = new QSlider(Qt::Horizontal, this);
			m_BuyCargoSlider->setRange(1, 100);
			m_BuyCargoSlider->setSingleStep(1);
			m_BuyCargoSlider->setPageStep(5);
			m_BuyCargoSlider->setValue(m_AmountBuyPercent);
			connect(m_BuyCargoSlider, SIGNAL(valueChanged(int)), SLOT(CargoBuyPercentChanged(int)));

			m_BuyCargoLabel = new QLabel(tr("Fill Up To %1% of Space").arg(m_AmountBuyPercent), this);

			m_TransferCargoFromAmountLabel = new QLabel(tr("Total Units to Acquire"), this);

			m_TransferCargoFromLabel = new QLabel(tr("Transfer Up To %1% of Their Cargo").arg(m_AmountBuyPercent), this);

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_BuyCargoLabel);
			optionsLay->addWidget(m_TransferCargoFromLabel);
			optionsLay->addWidget(m_BuyCargoSlider);
			optionsLay->addWidget(m_BuyCargoAmountLabel);
			optionsLay->addWidget(m_TransferCargoFromAmountLabel);
			optionsLay->addWidget(m_BuyCargoAmountEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			amountBuyRadios->addLayout(radioLay);
			amountBuyRadios->addLayout(optionsLay);
			amountBuyRadios->setSpacing(2);
			amountBuyRadios->setMargin(0);
		}

		QHBoxLayout* amountSellRadios = new QHBoxLayout;
		{
			m_AmountSellLabel = new QLabel(tr("Sell Quantity"), this);

			m_AmountTransferToLabel = new QLabel(tr("Transfer Quantity"), this);

			m_AmountSellPercentRadio = new QRadioButton(tr("Percent of Hold Space"), this);
			m_AmountSellPercentRadio->setToolTip(tr("<html>Sell quantity is specified as a percentage of the total hold space. "
						"The officer will attempt to sell up to this number of commodities."
						"</html>"));
			m_AmountSellPercentRadio->setChecked(m_AmountSellType == 0);
			connect(m_AmountSellPercentRadio, SIGNAL(clicked()), SLOT(AmountSell0Selected()));

			m_AmountSellActualRadio = new QRadioButton(tr("Actual Quantity"), this);
			m_AmountSellActualRadio->setToolTip(tr("<html>Sell quantity is specified as an actual number of units. "
						"The officer will attempt to sell up to this number of commodities."
						"</html>"));
			m_AmountSellActualRadio->setChecked(m_AmountSellType == 1);
			connect(m_AmountSellActualRadio, SIGNAL(clicked()), SLOT(AmountSell1Selected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_AmountSellPercentRadio);
			bg->addButton(m_AmountSellActualRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_AmountSellLabel);
			radioLay->addWidget(m_AmountTransferToLabel);
			radioLay->addWidget(m_AmountSellPercentRadio);
			radioLay->addWidget(m_AmountSellActualRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			m_SellCargoAmountLabel = new QLabel(tr("Units to Sell"), this);

			m_SellCargoAmountEdit = new QLineEdit(QVariant(m_AmountSellUnits).toString(), this);
			connect(m_SellCargoAmountEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoSellAmountChanged(const QString&)));

			m_SellCargoSlider = new QSlider(Qt::Horizontal, this);
			m_SellCargoSlider->setRange(1, 100);
			m_SellCargoSlider->setSingleStep(1);
			m_SellCargoSlider->setPageStep(5);
			m_SellCargoSlider->setValue(m_AmountSellPercent);
			connect(m_SellCargoSlider, SIGNAL(valueChanged(int)), SLOT(CargoSellPercentChanged(int)));

			m_SellCargoLabel = new QLabel(tr("Sell Up To %1% of Space").arg(m_AmountSellPercent), this);

			m_TransferCargoToAmountLabel = new QLabel(tr("Units to Transfer"), this);

			m_TransferCargoToLabel = new QLabel(tr("Transfer Up To %1% of Our Cargo").arg(m_AmountSellPercent), this);

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_SellCargoLabel);
			optionsLay->addWidget(m_TransferCargoToLabel);
			optionsLay->addWidget(m_SellCargoSlider);
			optionsLay->addWidget(m_SellCargoAmountLabel);
			optionsLay->addWidget(m_TransferCargoToAmountLabel);
			optionsLay->addWidget(m_SellCargoAmountEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			amountSellRadios->addLayout(radioLay);
			amountSellRadios->addLayout(optionsLay);
			amountSellRadios->setSpacing(2);
			amountSellRadios->setMargin(0);
		}

		QHBoxLayout* categoryBuyRadios = new QHBoxLayout;
		{
			m_CatBuyLabel = new QLabel(tr("Manifest Quantity"), this);

			m_CatTransferFromLabel = new QLabel(tr("Transfer Quantity"), this);

			m_CatBuyPercentRadio = new QRadioButton(tr("Percent of Hold Space"), this);
			m_CatBuyPercentRadio->setToolTip(tr("<html>Manifest quantity is specified as a percentage of the total hold space. "
						"The officer will attempt to purchase commodities until the percentage of hold space is filled."
						"</html>"));
			m_CatBuyPercentRadio->setChecked(m_CatBuyType == 0);
			connect(m_CatBuyPercentRadio, SIGNAL(clicked()), SLOT(CatBuy0Selected()));

			m_CatBuyActualRadio = new QRadioButton(tr("Quantity of Hold Space"), this);
			m_CatBuyActualRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of units. "
						"The officer will attempt to purchase commodities until the purchase quantity is filled."
						"</html>"));
			m_CatBuyActualRadio->setChecked(m_CatBuyType == 1);
			connect(m_CatBuyActualRadio, SIGNAL(clicked()), SLOT(CatBuy1Selected()));

			m_CatBuyEachRadio = new QRadioButton(tr("Quantity of Each Kind"), this);
			m_CatBuyEachRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of each kind of commodity "
						"in the category. "
						"The officer will attempt to purchase commodities until the hold contains this quantity of each kind of "
						"commodity in the category. "
						"</html>"));
			m_CatBuyEachRadio->setChecked(m_CatBuyType == 2);
			connect(m_CatBuyEachRadio, SIGNAL(clicked()), SLOT(CatBuy2Selected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_CatBuyPercentRadio);
			bg->addButton(m_CatBuyActualRadio);
			bg->addButton(m_CatBuyEachRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_CatBuyLabel);
			radioLay->addWidget(m_CatTransferFromLabel);
			radioLay->addWidget(m_CatBuyPercentRadio);
			radioLay->addWidget(m_CatBuyActualRadio);
			radioLay->addWidget(m_CatBuyEachRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			m_CatBuyAmountLabel = new QLabel(tr("Total Units to Acquire"), this);

			m_CatBuyAmountEdit = new QLineEdit(QVariant(m_AmountBuyUnits).toString(), this);
			connect(m_CatBuyAmountEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoBuyAmountChanged(const QString&)));

			m_CatBuySlider = new QSlider(Qt::Horizontal, this);
			m_CatBuySlider->setRange(1, 100);
			m_CatBuySlider->setSingleStep(1);
			m_CatBuySlider->setPageStep(5);
			m_CatBuySlider->setValue(m_AmountBuyPercent);
			connect(m_CatBuySlider, SIGNAL(valueChanged(int)), SLOT(CargoBuyPercentChanged(int)));

			m_CatBuyPercentLabel = new QLabel(tr("Fill Up To %1% of Space").arg(m_AmountBuyPercent), this);

			m_CatTransferFromAmountLabel = new QLabel(tr("Total Units to Acquire"), this);

			m_CatTransferFromPercentLabel = new QLabel(tr("Transfer Up To %1% of Their Cargo").arg(m_AmountBuyPercent), this);

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_CatBuyPercentLabel);
			optionsLay->addWidget(m_CatTransferFromPercentLabel);
			optionsLay->addWidget(m_CatBuySlider);
			optionsLay->addWidget(m_CatBuyAmountLabel);
			optionsLay->addWidget(m_CatTransferFromAmountLabel);
			optionsLay->addWidget(m_CatBuyAmountEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			categoryBuyRadios->addLayout(radioLay);
			categoryBuyRadios->addLayout(optionsLay);
			categoryBuyRadios->setSpacing(2);
			categoryBuyRadios->setMargin(0);
		}

		QHBoxLayout* categorySellRadios = new QHBoxLayout;
		{
			m_CatSellLabel = new QLabel(tr("Sell Quantity"), this);

			m_CatTransferToLabel = new QLabel(tr("Transfer Quantity"), this);

			m_CatSellPercentRadio = new QRadioButton(tr("Percent of Hold Space"), this);
			m_CatSellPercentRadio->setToolTip(tr("<html>Sell quantity is specified as a percentage of the total hold space. "
						"The officer will attempt to sell up to this number of commodities."
						"</html>"));
			m_CatSellPercentRadio->setChecked(m_CatSellType == 0);
			connect(m_CatSellPercentRadio, SIGNAL(clicked()), SLOT(CatSell0Selected()));

			m_CatSellActualRadio = new QRadioButton(tr("Quantity of Hold Space"), this);
			m_CatSellActualRadio->setToolTip(tr("<html>Sell quantity is specified as an actual number of units. "
						"The officer will attempt to sell up to this number of each kind of commodity in the category."
						"</html>"));
			m_CatSellActualRadio->setChecked(m_CatSellType == 1);
			connect(m_CatSellActualRadio, SIGNAL(clicked()), SLOT(CatSell1Selected()));

			// This option is only valid for TransferTo order.
			m_CatSellEachRadio = new QRadioButton(tr("Quantity of Each Kind"), this);
			m_CatSellEachRadio->setToolTip(tr("<html>Transfer quantity is specified as an actual number of each kind of commodity "
						"in the category. "
						"The officer will attempt to transfer up to this number of each kind of commodity in the category."
						"</html>"));
			m_CatSellEachRadio->setChecked(m_CatSellType == 2);
			connect(m_CatSellEachRadio, SIGNAL(clicked()), SLOT(CatSell2Selected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_CatSellPercentRadio);
			bg->addButton(m_CatSellActualRadio);
			bg->addButton(m_CatSellEachRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_CatSellLabel);
			radioLay->addWidget(m_CatTransferToLabel);
			radioLay->addWidget(m_CatSellPercentRadio);
			radioLay->addWidget(m_CatSellActualRadio);
			radioLay->addWidget(m_CatSellEachRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			m_CatSellAmountLabel = new QLabel(tr("Units to Sell"), this);

			m_CatSellAmountEdit = new QLineEdit(QVariant(m_AmountSellUnits).toString(), this);
			connect(m_CatSellAmountEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoSellAmountChanged(const QString&)));

			m_CatSellSlider = new QSlider(Qt::Horizontal, this);
			m_CatSellSlider->setRange(1, 100);
			m_CatSellSlider->setSingleStep(1);
			m_CatSellSlider->setPageStep(5);
			m_CatSellSlider->setValue(m_AmountSellPercent);
			connect(m_CatSellSlider, SIGNAL(valueChanged(int)), SLOT(CargoSellPercentChanged(int)));

			m_CatSellPercentLabel = new QLabel(tr("Sell Up To %1% of Space").arg(m_AmountSellPercent), this);

			m_CatTransferToAmountLabel = new QLabel(tr("Units to Transfer"), this);

			m_CatTransferToPercentLabel = new QLabel(tr("Transfer Up To %1% of Our Cargo").arg(m_AmountSellPercent), this);

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_CatSellPercentLabel);
			optionsLay->addWidget(m_CatTransferToPercentLabel);
			optionsLay->addWidget(m_CatSellSlider);
			optionsLay->addWidget(m_CatSellAmountLabel);
			optionsLay->addWidget(m_CatTransferToAmountLabel);
			optionsLay->addWidget(m_CatSellAmountEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			categorySellRadios->addLayout(radioLay);
			categorySellRadios->addLayout(optionsLay);
			categorySellRadios->setSpacing(2);
			categorySellRadios->setMargin(0);
		}

		QGridLayout* transferMoneyFromRadios = new QGridLayout;
		{
			m_TransferMoneyFromLabel = new QLabel(tr("Amount of Money to Take"), this);

			m_TransferMoneyFromPercentRadio = new QRadioButton(tr("Percent of Their Money"), this);
			m_TransferMoneyFromPercentRadio->setToolTip(tr("<html>Amount of money to take is specified as a percentage of the other "
						"spacecraft's account."
						"</html>"));
			m_TransferMoneyFromPercentRadio->setChecked(m_TransferMoneyFromType == 0);
			connect(m_TransferMoneyFromPercentRadio, SIGNAL(clicked()), SLOT(TransferMoneyFromPercent()));

			m_TransferMoneyFromPercentSlider = new QSlider(Qt::Horizontal, this);
			m_TransferMoneyFromPercentSlider->setRange(1, 100);
			m_TransferMoneyFromPercentSlider->setSingleStep(1);
			m_TransferMoneyFromPercentSlider->setPageStep(5);
			m_TransferMoneyFromPercentSlider->setValue(m_TransferMoneyFromPercent);
			connect(m_TransferMoneyFromPercentSlider, SIGNAL(valueChanged(int)), SLOT(TransferMoneyFromPercentChanged(int)));

			m_TransferMoneyFromPercentLabel = new QLabel(this);
			m_TransferMoneyFromPercentLabel->setText(QString("%1%").arg(m_TransferMoneyFromPercent));

			m_TransferMoneyFromAmountRadio = new QRadioButton(tr("Actual Amount"), this);
			m_TransferMoneyFromAmountRadio->setToolTip(tr("<html>Amount of money to take from the other spacecraft's account "
						"is specified as a number of cronos."
						"</html>"));
			m_TransferMoneyFromAmountRadio->setChecked(m_TransferMoneyFromType == 1);
			connect(m_TransferMoneyFromAmountRadio, SIGNAL(clicked()), SLOT(TransferMoneyFromAmount()));

			m_TransferMoneyFromAmountEdit = new QLineEdit(QVariant(m_TransferMoneyFromCount).toString(), this);

			int row=0;
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromLabel, row,0, 1,3);
			row++;
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromPercentRadio, row,0, 1,1);
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromPercentSlider, row,1, 1,1);
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromPercentLabel, row,2, 1,1);
			row++;
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromAmountRadio, row,0, 1,1);
			transferMoneyFromRadios->addWidget(m_TransferMoneyFromAmountEdit, row,1, 1,1);
			transferMoneyFromRadios->setSpacing(2);
			transferMoneyFromRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_TransferMoneyFromPercentRadio);
			bg->addButton(m_TransferMoneyFromAmountRadio);
		}

		QGridLayout* transferMoneyToRadios = new QGridLayout;
		{
			m_TransferMoneyToLabel = new QLabel(tr("Amount of Money to Give"), this);

			m_TransferMoneyToPercentSlider = new QSlider(Qt::Horizontal, this);
			m_TransferMoneyToPercentSlider->setRange(1, 100);
			m_TransferMoneyToPercentSlider->setSingleStep(1);
			m_TransferMoneyToPercentSlider->setPageStep(5);
			m_TransferMoneyToPercentSlider->setValue(m_TransferMoneyToPercent);
			connect(m_TransferMoneyToPercentSlider, SIGNAL(valueChanged(int)), SLOT(TransferMoneyToPercentChanged(int)));

			m_TransferMoneyToPercentLabel = new QLabel(this);
			m_TransferMoneyToPercentLabel->setText(QString("%1%").arg(m_TransferMoneyToPercent));

			m_TransferMoneyToPercentRadio = new QRadioButton(tr("Percent of Our Money"), this);
			m_TransferMoneyToPercentRadio->setToolTip(tr("<html>Amount of money to give is specified as a percentage of the "
						"mission spacecraft's account."
						"</html>"));
			m_TransferMoneyToPercentRadio->setChecked(m_TransferMoneyToType == 0);
			connect(m_TransferMoneyToPercentRadio, SIGNAL(clicked()), SLOT(TransferMoneyToPercent()));

			m_TransferMoneyToAmountRadio = new QRadioButton(tr("Actual Amount"), this);
			m_TransferMoneyToAmountRadio->setToolTip(tr("<html>Amount of money to give is specified as a number of cronos from the "
						"mission spacecraft's account."
						"</html>"));
			m_TransferMoneyToAmountRadio->setChecked(m_TransferMoneyToType == 1);
			connect(m_TransferMoneyToAmountRadio, SIGNAL(clicked()), SLOT(TransferMoneyToAmount()));

			m_TransferMoneyToAmountEdit = new QLineEdit(QVariant(m_TransferMoneyToCount).toString(), this);

			int row=0;
			transferMoneyToRadios->addWidget(m_TransferMoneyToLabel, row,0, 1,3);
			row++;
			transferMoneyToRadios->addWidget(m_TransferMoneyToPercentRadio, row,0, 1,1);
			transferMoneyToRadios->addWidget(m_TransferMoneyToPercentSlider, row,1, 1,1);
			transferMoneyToRadios->addWidget(m_TransferMoneyToPercentLabel, row,2, 1,1);
			row++;
			transferMoneyToRadios->addWidget(m_TransferMoneyToAmountRadio, row,0, 1,1);
			transferMoneyToRadios->addWidget(m_TransferMoneyToAmountEdit, row,1, 1,1);
			transferMoneyToRadios->setSpacing(2);
			transferMoneyToRadios->setMargin(0);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_TransferMoneyToPercentRadio);
			bg->addButton(m_TransferMoneyToAmountRadio);
		}

		QGridLayout* transferPeopleFromRadios = new QGridLayout;
		{
			m_TransferPeopleFromLabel = new QLabel(tr("Number of People to Board"), this);

			m_TransferPeopleFromBerthLabel = new QLabel(tr("Berth Type to Board"), this);

			m_TransferPeopleFromOfficersRadio = new QRadioButton("Officers", this);
			m_TransferPeopleFromOfficersRadio->setToolTip(tr("<html>Officers will be transferred from the other spacecraft."
						"</html>"));
			m_TransferPeopleFromOfficersRadio->setChecked(m_TransferPeopleFromBerth == 0);
			connect(m_TransferPeopleFromOfficersRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromOfficers()));

			m_TransferPeopleFromCrewRadio = new QRadioButton("Crew", this);
			m_TransferPeopleFromCrewRadio->setToolTip(tr("<html>Crew will be transferred from the other spacecraft."
						"</html>"));
			m_TransferPeopleFromCrewRadio->setChecked(m_TransferPeopleFromBerth == 1);
			connect(m_TransferPeopleFromCrewRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromCrew()));

			m_TransferPeopleFromTroopsRadio = new QRadioButton("Troops", this);
			m_TransferPeopleFromTroopsRadio->setToolTip(tr("<html>Troops will be transferred from the other spacecraft."
						"</html>"));
			m_TransferPeopleFromTroopsRadio->setChecked(m_TransferPeopleFromBerth == 2);
			connect(m_TransferPeopleFromTroopsRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromTroops()));

			m_TransferPeopleFromPassengersRadio = new QRadioButton("Passengers", this);
			m_TransferPeopleFromPassengersRadio->setToolTip(tr("<html>Passengers will be transferred from the other spacecraft."
						"</html>"));
			m_TransferPeopleFromPassengersRadio->setChecked(m_TransferPeopleFromBerth == 3);
			connect(m_TransferPeopleFromPassengersRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromPassengers()));

			QButtonGroup* bg2 = new QButtonGroup(this);
			bg2->addButton(m_TransferPeopleFromOfficersRadio);
			bg2->addButton(m_TransferPeopleFromCrewRadio);
			bg2->addButton(m_TransferPeopleFromTroopsRadio);
			bg2->addButton(m_TransferPeopleFromPassengersRadio);

			m_TransferPeopleFromPercentRadio = new QRadioButton(tr("Percent of People Available"), this);
			m_TransferPeopleFromPercentRadio->setToolTip(tr("<html>Number of people to board is specified as a percentage of the "
						"people aboard the other spacecraft."
						"</html>"));
			m_TransferPeopleFromPercentRadio->setChecked(m_TransferPeopleFromType == 0);
			connect(m_TransferPeopleFromPercentRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromPercent()));

			m_TransferPeopleFromPercentSlider = new QSlider(Qt::Horizontal, this);
			m_TransferPeopleFromPercentSlider->setRange(1, 100);
			m_TransferPeopleFromPercentSlider->setSingleStep(1);
			m_TransferPeopleFromPercentSlider->setPageStep(5);
			m_TransferPeopleFromPercentSlider->setValue(m_TransferPeopleFromPercent);
			connect(m_TransferPeopleFromPercentSlider, SIGNAL(valueChanged(int)), SLOT(TransferPeopleFromPercentChanged(int)));

			m_TransferPeopleFromPercentLabel = new QLabel(this);
			m_TransferPeopleFromPercentLabel->setText(QString("%1%").arg(m_TransferPeopleFromPercent));

			m_TransferPeopleFromAmountRadio = new QRadioButton(tr("Actual Number of People"), this);
			m_TransferPeopleFromAmountRadio->setToolTip(tr("<html>Actual number of people to board from the other spacecraft is "
						"specified."
						"</html>"));
			m_TransferPeopleFromAmountRadio->setChecked(m_TransferPeopleFromType == 1);
			connect(m_TransferPeopleFromAmountRadio, SIGNAL(clicked()), SLOT(TransferPeopleFromAmount()));

			m_TransferPeopleFromAmountEdit = new QLineEdit(QVariant(m_TransferPeopleFromCount).toString(), this);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_TransferPeopleFromPercentRadio);
			bg->addButton(m_TransferPeopleFromAmountRadio);

			int row=0;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromBerthLabel, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromOfficersRadio, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromCrewRadio, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromTroopsRadio, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromPassengersRadio, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromLabel, row,0, 1,3);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromPercentRadio, row,0, 1,1);
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromPercentSlider, row,1, 1,1);
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromPercentLabel, row,2, 1,1);
			row++;
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromAmountRadio, row,0, 1,1);
			transferPeopleFromRadios->addWidget(m_TransferPeopleFromAmountEdit, row,1, 1,1);
			transferPeopleFromRadios->setSpacing(2);
			transferPeopleFromRadios->setMargin(0);
		}

		QGridLayout* transferPeopleToRadios = new QGridLayout;
		{
			m_TransferPeopleToLabel = new QLabel(tr("Number of People to Debark"), this);

			m_TransferPeopleToBerthLabel = new QLabel(tr("Berth Type to Debark"), this);

			m_TransferPeopleToOfficersRadio = new QRadioButton("Officers", this);
			m_TransferPeopleToOfficersRadio->setToolTip(tr("<html>Officers will be transferred to the other spacecraft."
						"</html>"));
			m_TransferPeopleToOfficersRadio->setChecked(m_TransferPeopleToBerth == 0);
			connect(m_TransferPeopleToOfficersRadio, SIGNAL(clicked()), SLOT(TransferPeopleToOfficers()));

			m_TransferPeopleToCrewRadio = new QRadioButton("Crew", this);
			m_TransferPeopleToCrewRadio->setToolTip(tr("<html>Crew will be transferred to the other spacecraft."
						"</html>"));
			m_TransferPeopleToCrewRadio->setChecked(m_TransferPeopleToBerth == 1);
			connect(m_TransferPeopleToCrewRadio, SIGNAL(clicked()), SLOT(TransferPeopleToCrew()));

			m_TransferPeopleToTroopsRadio = new QRadioButton("Troops", this);
			m_TransferPeopleToTroopsRadio->setToolTip(tr("<html>Troops will be transferred to the other spacecraft."
						"</html>"));
			m_TransferPeopleToTroopsRadio->setChecked(m_TransferPeopleToBerth == 2);
			connect(m_TransferPeopleToTroopsRadio, SIGNAL(clicked()), SLOT(TransferPeopleToTroops()));

			m_TransferPeopleToPassengersRadio = new QRadioButton("Passengers", this);
			m_TransferPeopleToPassengersRadio->setToolTip(tr("<html>Passengers will be transferred to the other spacecraft."
						"</html>"));
			m_TransferPeopleToPassengersRadio->setChecked(m_TransferPeopleToBerth == 3);
			connect(m_TransferPeopleToPassengersRadio, SIGNAL(clicked()), SLOT(TransferPeopleToPassengers()));

			QButtonGroup* bg2 = new QButtonGroup(this);
			bg2->addButton(m_TransferPeopleToOfficersRadio);
			bg2->addButton(m_TransferPeopleToCrewRadio);
			bg2->addButton(m_TransferPeopleToTroopsRadio);
			bg2->addButton(m_TransferPeopleToPassengersRadio);

			m_TransferPeopleToPercentRadio = new QRadioButton(tr("Percent of People Available"), this);
			m_TransferPeopleToPercentRadio->setToolTip(tr("<html>Number of people to debark is specified as a percentage of the "
						"people aboard the mission spacecraft."
						"</html>"));
			m_TransferPeopleToPercentRadio->setChecked(m_TransferPeopleToType == 0);
			connect(m_TransferPeopleToPercentRadio, SIGNAL(clicked()), SLOT(TransferPeopleToPercent()));

			m_TransferPeopleToPercentSlider = new QSlider(Qt::Horizontal, this);
			m_TransferPeopleToPercentSlider->setRange(1, 100);
			m_TransferPeopleToPercentSlider->setSingleStep(1);
			m_TransferPeopleToPercentSlider->setPageStep(5);
			m_TransferPeopleToPercentSlider->setValue(m_TransferPeopleToPercent);
			connect(m_TransferPeopleToPercentSlider, SIGNAL(valueChanged(int)), SLOT(TransferPeopleToPercentChanged(int)));

			m_TransferPeopleToPercentLabel = new QLabel(this);
			m_TransferPeopleToPercentLabel->setText(QString("%1%").arg(m_TransferPeopleToPercent));

			m_TransferPeopleToAmountRadio = new QRadioButton(tr("Actual Number of People"), this);
			m_TransferPeopleToAmountRadio->setToolTip(tr("<html>Actual number of people to debark to the other spacecraft is specified."
						"</html>"));
			m_TransferPeopleToAmountRadio->setChecked(m_TransferPeopleToType == 1);
			connect(m_TransferPeopleToAmountRadio, SIGNAL(clicked()), SLOT(TransferPeopleToAmount()));

			m_TransferPeopleToAmountEdit = new QLineEdit(QVariant(m_TransferPeopleToCount).toString(), this);

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_TransferPeopleToPercentRadio);
			bg->addButton(m_TransferPeopleToAmountRadio);

			int row=0;
			transferPeopleToRadios->addWidget(m_TransferPeopleToBerthLabel, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToOfficersRadio, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToCrewRadio, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToTroopsRadio, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToPassengersRadio, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToLabel, row,0, 1,3);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToPercentRadio, row,0, 1,1);
			transferPeopleToRadios->addWidget(m_TransferPeopleToPercentSlider, row,1, 1,1);
			transferPeopleToRadios->addWidget(m_TransferPeopleToPercentLabel, row,2, 1,1);
			row++;
			transferPeopleToRadios->addWidget(m_TransferPeopleToAmountRadio, row,0, 1,1);
			transferPeopleToRadios->addWidget(m_TransferPeopleToAmountEdit, row,1, 1,1);
			transferPeopleToRadios->setSpacing(2);
			transferPeopleToRadios->setMargin(0);
		}

		QHBoxLayout* marginBuyRadios = new QHBoxLayout;
		{
			m_MarginBuyLabel = new QLabel(tr("Purchase Price Maximum"), this);

			m_MarginBuyPercentRadio = new QRadioButton(tr("Percent of Nominal Price"), this);
			m_MarginBuyPercentRadio->setToolTip(tr("<html>Maximum purchase price is specified as a percentage of the nominal "
						"price of the commodity, based on its quality."
						"</html>"));
			m_MarginBuyPercentRadio->setChecked(m_MarginBuyType == 0);
			connect(m_MarginBuyPercentRadio, SIGNAL(clicked()), SLOT(MarginBuy0Selected()));

			m_MarginBuyActualRadio = new QRadioButton(tr("Actual Price"), this);
			m_MarginBuyActualRadio->setToolTip(tr("<html>Maximum purchase price is specified as the actual maximum amount to pay "
						"for the commodity."
						"</html>"));
			m_MarginBuyActualRadio->setChecked(m_MarginBuyType == 1);
			connect(m_MarginBuyActualRadio, SIGNAL(clicked()), SLOT(MarginBuy1Selected()));

			m_MarginBuyAnyPriceRadio = new QRadioButton("Any Price", this);
			m_MarginBuyAnyPriceRadio->setToolTip(tr("<html>Purchase price maximum is not limited. "
						"Pay any price to acquire the commodity."
						"</html>"));
			m_MarginBuyAnyPriceRadio->setChecked(m_MarginBuyType == 2);
			connect(m_MarginBuyAnyPriceRadio, SIGNAL(clicked()), SLOT(MarginBuy2Selected()));

			m_BuyMarginSlider = new QSlider(Qt::Horizontal, this);
			m_BuyMarginSlider->setRange(1, 250);
			m_BuyMarginSlider->setSingleStep(1);
			m_BuyMarginSlider->setPageStep(5);
			m_BuyMarginSlider->setValue(m_MarginBuyPercent);
			m_BuyMarginSlider->setToolTip(tr("<html>Highest price to pay when buying, as a percentage of nominal commodity price."
						"</html>"));
			connect(m_BuyMarginSlider, SIGNAL(valueChanged(int)), SLOT(MarginBuyChanged(int)));

			m_BuyMarginLabel = new QLabel(this);
			m_BuyMarginLabel->setText(tr("Pay No More Than %1% of Nominal Price").arg(m_MarginBuyPercent));

			m_BuyCargoCostLabel = new QLabel(tr("Pay No More Than"), this);

			m_BuyCargoCostEdit = new QLineEdit(QVariant(m_MarginBuyCost).toString(), this);
			connect(m_BuyCargoCostEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoBuyCostChanged(const QString&)));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_MarginBuyPercentRadio);
			bg->addButton(m_MarginBuyActualRadio);
			bg->addButton(m_MarginBuyAnyPriceRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_MarginBuyLabel);
			radioLay->addWidget(m_MarginBuyPercentRadio);
			radioLay->addWidget(m_MarginBuyActualRadio);
			radioLay->addWidget(m_MarginBuyAnyPriceRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_BuyMarginLabel);
			optionsLay->addWidget(m_BuyMarginSlider);
			optionsLay->addWidget(m_BuyCargoCostLabel);
			optionsLay->addWidget(m_BuyCargoCostEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			marginBuyRadios->addLayout(radioLay);
			marginBuyRadios->addLayout(optionsLay);
			marginBuyRadios->setSpacing(2);
			marginBuyRadios->setMargin(0);
		}

		QHBoxLayout* marginSellRadios = new QHBoxLayout;
		{
			m_MarginSellLabel = new QLabel(tr("Sale Price Minimum"), this);

			m_MarginSellPercentRadio = new QRadioButton(tr("Percent of Nominal Price"), this);
			m_MarginSellPercentRadio->setToolTip(tr("<html>Minimum purchase price is specified as a percentage of the nominal "
						"price of the commodity, based on its quality."
						"</html>"));
			m_MarginSellPercentRadio->setChecked(m_MarginSellType == 0);
			connect(m_MarginSellPercentRadio, SIGNAL(clicked()), SLOT(MarginSell0Selected()));

			m_MarginSellActualRadio = new QRadioButton(tr("Actual Price"), this);
			m_MarginSellActualRadio->setToolTip(tr("<html>Minimum purchase price is specified as the actual minimum amount to accept "
						"for the commodity."
						"</html>"));
			m_MarginSellActualRadio->setChecked(m_MarginSellType == 1);
			connect(m_MarginSellActualRadio, SIGNAL(clicked()), SLOT(MarginSell1Selected()));

			m_MarginSellAnyPriceRadio = new QRadioButton("Any Price", this);
			m_MarginSellAnyPriceRadio->setToolTip(tr("<html>Purchase price minimum is not limited. "
						"Accept any offer to unload the commodity."
						"</html>"));
			m_MarginSellAnyPriceRadio->setChecked(m_MarginSellType == 2);
			connect(m_MarginSellAnyPriceRadio, SIGNAL(clicked()), SLOT(MarginSell2Selected()));

			QButtonGroup* bg = new QButtonGroup(this);
			bg->addButton(m_MarginSellPercentRadio);
			bg->addButton(m_MarginSellActualRadio);
			bg->addButton(m_MarginSellAnyPriceRadio);

			QVBoxLayout* radioLay = new QVBoxLayout;
			radioLay->addWidget(m_MarginSellLabel);
			radioLay->addWidget(m_MarginSellPercentRadio);
			radioLay->addWidget(m_MarginSellActualRadio);
			radioLay->addWidget(m_MarginSellAnyPriceRadio);
			radioLay->setSpacing(2);
			radioLay->setMargin(0);

			m_SellCargoCostLabel = new QLabel(tr("Accept No Less Than"), this);

			m_SellCargoCostEdit = new QLineEdit(QVariant(m_MarginSellCost).toString(), this);
			connect(m_SellCargoCostEdit, SIGNAL(textEdited(const QString&)), SLOT(CargoSellCostChanged(const QString&)));

			QVBoxLayout* optionsLay = new QVBoxLayout;
			optionsLay->addWidget(m_SellMarginLabel);
			optionsLay->addWidget(m_SellMarginSlider);
			optionsLay->addWidget(m_SellCargoCostLabel);
			optionsLay->addWidget(m_SellCargoCostEdit);
			optionsLay->setSpacing(2);
			optionsLay->setMargin(0);

			marginSellRadios->addLayout(radioLay);
			marginSellRadios->addLayout(optionsLay);
			marginSellRadios->setSpacing(2);
			marginSellRadios->setMargin(0);
		}

		QHBoxLayout* minlay = new QHBoxLayout;
		{
			minlay->addWidget(m_MinutesSpin);
			minlay->addWidget(m_MinutesLabel);
		}

		QHBoxLayout* minpreblay = new QHBoxLayout;
		{
			minpreblay->addWidget(m_MinutesPreboardSpin);
			minpreblay->addWidget(m_MinutesPreboardLabel);
		}

		QHBoxLayout* hoverlay = new QHBoxLayout;
		{
			hoverlay->addWidget(m_HoverAltSpin);
			hoverlay->addWidget(m_MetersLabel);
		}

		QSet<QWidget*> showit;				// Hide all the widgets.
		SetControlVisibility(showit, false);

		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(m_DoOnce);
		lay->addLayout(transferTargetLayout);
		lay->addWidget(m_MailCoC);
		lay->addWidget(m_BuyCargoCountLabel);
		lay->addWidget(m_SellCargoCountLabel);
		lay->addWidget(m_TransferCargoCountLabel);
		lay->addWidget(m_CommodityFilter);
		lay->addWidget(m_CommoditiesAtmo);
		lay->addWidget(m_CommoditiesBuy);
		lay->addWidget(m_CommoditiesMin);
		lay->addWidget(m_CommoditiesPart);
		lay->addWidget(m_CommoditiesSea);
		lay->addWidget(m_CommoditiesTransfer);
		lay->addWidget(m_CommoditiesWell);
		lay->addWidget(m_Bays);
		lay->addWidget(m_Postures);
		lay->addLayout(amountBuyRadios);
		lay->addLayout(amountSellRadios);
		lay->addLayout(categoryBuyRadios);
		lay->addLayout(categorySellRadios);
		lay->addWidget(m_TroopCountLabel);
		lay->addWidget(m_CrewCountLabel);
		lay->addWidget(m_CryoBerthsLoadCountLabel);
		lay->addWidget(m_CryoBerthsUnloadCountLabel);
		lay->addWidget(m_PassengerLoadCountLabel);
		lay->addWidget(m_PassengerUnloadCountLabel);
		lay->addWidget(m_CountSpin);
		lay->addWidget(m_DurWaitLabel);
		lay->addWidget(m_DurPatrolLabel);
		lay->addWidget(m_DurPreboardLabel);
		lay->addLayout(minlay);
		lay->addLayout(minpreblay);
		lay->addWidget(m_HoverAltLabel);
		lay->addLayout(hoverlay);
		lay->addWidget(m_CoordsLabel);
		lay->addWidget(m_CoordsEdit);
		lay->addWidget(m_Airports);
		lay->addWidget(m_Cities);
		lay->addWidget(m_PlanetsAndStars);
		lay->addWidget(m_PlanetsMin);
		lay->addWidget(m_PlanetsPart);
		lay->addWidget(m_PlanetsSea);
		lay->addWidget(m_PlanetsWell);
		lay->addWidget(m_Stations);
		lay->addWidget(m_Targets);
		lay->addLayout(systemRadios);
		lay->addWidget(m_Routes);
		lay->addWidget(m_Systems);
		lay->addLayout(marginBuyRadios);
		lay->addLayout(marginSellRadios);
		lay->addLayout(transferMoneyFromRadios);
		lay->addLayout(transferMoneyToRadios);
		lay->addLayout(transferPeopleFromRadios);
		lay->addLayout(transferPeopleToRadios);
		lay->addWidget(m_TransferAuthorizationLabel);
		lay->addWidget(m_ShipNameLabel);
		lay->addWidget(m_ShipNameEdit);
		lay->addLayout(coordRadios);
		lay->addLayout(coords);
		lay->addLayout(lightHeadlightRadios);
		lay->addLayout(lightNavLightRadios);
		lay->addLayout(lightRotatingBeaconRadios);
		lay->addLayout(lightStrobeRadios);
		lay->addWidget(m_PilotUpPolicies);
		lay->addWidget(m_ShieldPolicies);
		lay->addWidget(m_ShieldFuelPolicies);
		lay->addLayout(minQRadios);
		// Add new widgets above the smashie widget.
		lay->addWidget(m_Smashie);
		lay->setMargin(2);
		lay->setSpacing(2);
		lay->setSizeConstraint(QLayout::SetMinAndMaxSize);	// Insure size update when dynamically changed.

		QWidget* pageOpt = new QWidget(this);
		pageOpt->setLayout(lay);

		m_Stack->addWidget(pageOpt);
		pageIndex++;
	}

	// Create the stacked page of widgets for configuring the harvester operations order by bay.
	{
		m_HarvestDoOnce = new QCheckBox(tr("Do-Once Order"), this);
		m_HarvestDoOnce->setCheckState(m_IsDoOnce ? Qt::Checked : Qt::Unchecked);
		m_HarvestDoOnce->setToolTip(tr("<html>When checked, the order is only executed once, even if the mission is cyclic. "
					"This is useful when a temporary order is desired that does not permanently alter a spacecraft's mission "
					"programming.</html>"));

		// Planets and stars table is filled on demand because the scene atom's location can change.
		m_HarvestPlanetsAndStars = new QTableWidget(this);
		m_HarvestPlanetsAndStars->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_HarvestPlanetsAndStars->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_HarvestPlanetsAndStars->setSelectionMode(QAbstractItemView::SingleSelection);
		m_HarvestPlanetsAndStars->setShowGrid(false);
		QStringList hls;
		hls << tr("Planet or Star to Harvest") << tr("Distance") << tr("Info");
		m_HarvestPlanetsAndStars->setColumnCount(hls.count());
		m_HarvestPlanetsAndStars->setHorizontalHeaderLabels(hls);
		m_HarvestPlanetsAndStars->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
		m_HarvestPlanetsAndStars->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
		m_HarvestPlanetsAndStars->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		m_HarvestPlanetsAndStars->setColumnWidth(0, 150);
		m_HarvestPlanetsAndStars->setColumnWidth(1, 80);
		QHeaderView* vhdr = m_HarvestPlanetsAndStars->verticalHeader();
		vhdr->hide();
		connect(m_HarvestPlanetsAndStars, SIGNAL(itemSelectionChanged()), SLOT(HarvestPlanetChanged()));
		m_HarvestPlanetsAndStars->setToolTip(tr("<html>Select a planet or star from which to harvest resources.</html>"));

		m_HarvestFuelThresholdLabel =
			new QLabel(tr("Stop harvesting if fuel level is %1% or below.").arg(m_HarvestFuelThreshold), this);

		m_HarvestFuelThresholdSlider = new QSlider(Qt::Horizontal, this);
		m_HarvestFuelThresholdSlider->setRange(0,100);
		m_HarvestFuelThresholdSlider->setValue(m_HarvestFuelThreshold);
		m_HarvestFuelThresholdSlider->setToolTip(tr("<html>"
					"Harvest operations are stopped if the fuel level is at or below the threshold. "
					"</html>"));
		connect(m_HarvestFuelThresholdSlider, SIGNAL(valueChanged(int)), SLOT(HarvestFuelThresholdChanged(int)));

		QLabel* lbop = new QLabel(tr("Harvester Operations by Bay"), this);

		m_HarvestOpLay = new QVBoxLayout;
		m_HarvestOpLay->addWidget(lbop);
		m_HarvestOpLay->setSizeConstraint(QLayout::SetMinAndMaxSize);	// Insure size update when dynamically changed.

		QWidget* opPar = new QWidget(this);
		opPar->setLayout(m_HarvestOpLay);

		QScrollArea* opScroll = new QScrollArea(this);
		opScroll->setWidget(opPar);

		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(m_HarvestDoOnce);
		lay->addWidget(m_HarvestPlanetsAndStars);
		lay->addWidget(m_HarvestFuelThresholdLabel);
		lay->addWidget(m_HarvestFuelThresholdSlider);
		lay->addWidget(opScroll);
		lay->setMargin(2);
		lay->setSpacing(2);

		QWidget* pageOpt = new QWidget(this);
		pageOpt->setLayout(lay);

		m_Stack->addWidget(pageOpt);
		m_HarvestByBayPage = pageIndex++;
	}

	// Create the stacked page of widgets for configuring the harvester operations order by module.
	{
		m_HarvestDoOnceByM = new QCheckBox(tr("Do-Once Order"), this);
		m_HarvestDoOnceByM->setCheckState(m_IsDoOnce ? Qt::Checked : Qt::Unchecked);
		m_HarvestDoOnceByM->setToolTip(tr("<html>When checked, the order is only executed once, even if the mission is cyclic. "
					"This is useful when a temporary order is desired that does not permanently alter a spacecraft's mission "
					"programming.</html>"));

		// Planets and stars table is filled on demand because the scene atom's location can change.
		m_HarvestPlanetsAndStarsByM = new QTableWidget(this);
		m_HarvestPlanetsAndStarsByM->setEditTriggers(QAbstractItemView::NoEditTriggers);
		m_HarvestPlanetsAndStarsByM->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_HarvestPlanetsAndStarsByM->setSelectionMode(QAbstractItemView::SingleSelection);
		m_HarvestPlanetsAndStarsByM->setShowGrid(false);
		QStringList hls;
		hls << tr("Planet or Star to Harvest") << tr("Distance") << tr("Info");
		m_HarvestPlanetsAndStarsByM->setColumnCount(hls.count());
		m_HarvestPlanetsAndStarsByM->setHorizontalHeaderLabels(hls);
		m_HarvestPlanetsAndStarsByM->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
		m_HarvestPlanetsAndStarsByM->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
		m_HarvestPlanetsAndStarsByM->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
		m_HarvestPlanetsAndStarsByM->setColumnWidth(0, 150);
		m_HarvestPlanetsAndStarsByM->setColumnWidth(1, 80);
		QHeaderView* vhdr = m_HarvestPlanetsAndStarsByM->verticalHeader();
		vhdr->hide();
		connect(m_HarvestPlanetsAndStarsByM, SIGNAL(itemSelectionChanged()), SLOT(HarvestPlanetChangedByM()));
		m_HarvestPlanetsAndStarsByM->setToolTip(tr("<html>Select a planet or star from which to harvest resources.</html>"));

		m_HarvestFuelThresholdLabelByM =
			new QLabel(tr("Stop harvesting if fuel level is %1% or below.").arg(m_HarvestFuelThresholdByM), this);

		m_HarvestFuelThresholdSliderByM = new QSlider(Qt::Horizontal, this);
		m_HarvestFuelThresholdSliderByM->setRange(0,100);
		m_HarvestFuelThresholdSliderByM->setValue(m_HarvestFuelThresholdByM);
		m_HarvestFuelThresholdSliderByM->setToolTip(tr("<html>"
					"Harvest operations are stopped if the fuel level is at or below the threshold. "
					"</html>"));
		connect(m_HarvestFuelThresholdSliderByM, SIGNAL(valueChanged(int)), SLOT(HarvestFuelThresholdByMChanged(int)));

		AuHarvestOpWidget* opAt;
		AuHarvestOps::const_iterator oi = m_HarvestOpsByM.find(CID_AtmosphereCondenserBayModule);
		if(oi == m_HarvestOpsByM.cend())
			opAt = new AuHarvestOpWidget(g_Au.CommodityName(CID_AtmosphereCondenserBayModule), CID_AtmosphereCondenserBayModule,
					CID_Invalid, 2, 100, 0, this);
		else
			opAt = new AuHarvestOpWidget(g_Au.CommodityName(CID_AtmosphereCondenserBayModule), CID_AtmosphereCondenserBayModule,
					(*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(), this);
		m_HarvestByModule.insert(CID_AtmosphereCondenserBayModule, opAt);

		AuHarvestOpWidget* opPa;
		oi = m_HarvestOpsByM.find(CID_ParticleCollectorBayModule);
		if(oi == m_HarvestOpsByM.cend())
			opPa = new AuHarvestOpWidget(g_Au.CommodityName(CID_ParticleCollectorBayModule), CID_ParticleCollectorBayModule,
					CID_Invalid, 2, 100, 0, this);
		else
			opPa = new AuHarvestOpWidget(g_Au.CommodityName(CID_ParticleCollectorBayModule), CID_ParticleCollectorBayModule,
					(*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(), this);
		m_HarvestByModule.insert(CID_ParticleCollectorBayModule, opPa);

		AuHarvestOpWidget* opPr;
		oi = m_HarvestOpsByM.find(CID_ProspectorBayModule);
		if(oi == m_HarvestOpsByM.cend())
			opPr = new AuHarvestOpWidget(g_Au.CommodityName(CID_ProspectorBayModule), CID_ProspectorBayModule,
					CID_Invalid, 2, 100, 0, this);
		else
			opPr = new AuHarvestOpWidget(g_Au.CommodityName(CID_ProspectorBayModule), CID_ProspectorBayModule,
					(*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(), this);
		m_HarvestByModule.insert(CID_ProspectorBayModule, opPr);

		AuHarvestOpWidget* opSi;
		oi = m_HarvestOpsByM.find(CID_SiphonBayModule);
		if(oi == m_HarvestOpsByM.cend())
			opSi = new AuHarvestOpWidget(g_Au.CommodityName(CID_SiphonBayModule), CID_SiphonBayModule,
					CID_Invalid, 2, 100, 0, this);
		else
			opSi = new AuHarvestOpWidget(g_Au.CommodityName(CID_SiphonBayModule), CID_SiphonBayModule,
					(*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(), this);
		m_HarvestByModule.insert(CID_SiphonBayModule, opSi);

		AuHarvestOpWidget* opWi;
		oi = m_HarvestOpsByM.find(CID_WildcatterBayModule);
		if(oi == m_HarvestOpsByM.cend())
			opWi = new AuHarvestOpWidget(g_Au.CommodityName(CID_WildcatterBayModule), CID_WildcatterBayModule,
					CID_Invalid, 2, 100, 0, this);
		else
			opWi = new AuHarvestOpWidget(g_Au.CommodityName(CID_WildcatterBayModule), CID_WildcatterBayModule,
					(*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(), this);
		m_HarvestByModule.insert(CID_WildcatterBayModule, opWi);

		QLabel* lbop = new QLabel(tr("Harvester Operations by Module"), this);

		QVBoxLayout* opLay = new QVBoxLayout;
		opLay->addWidget(lbop);
		opLay->addWidget(opAt);
		opLay->addWidget(opPa);
		opLay->addWidget(opPr);
		opLay->addWidget(opSi);
		opLay->addWidget(opWi);
		opLay->setSizeConstraint(QLayout::SetMinAndMaxSize);	// Insure size update when dynamically changed.

		QWidget* opPar = new QWidget(this);
		opPar->setLayout(opLay);

		QScrollArea* opScroll = new QScrollArea(this);
		opScroll->setWidget(opPar);

		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(m_HarvestDoOnceByM);
		lay->addWidget(m_HarvestPlanetsAndStarsByM);
		lay->addWidget(m_HarvestFuelThresholdLabelByM);
		lay->addWidget(m_HarvestFuelThresholdSliderByM);
		lay->addWidget(opScroll);
		lay->setMargin(2);
		lay->setSpacing(2);

		QWidget* pageOpt = new QWidget(this);
		pageOpt->setLayout(lay);

		m_Stack->addWidget(pageOpt);
		m_HarvestByModulePage = pageIndex++;
	}

	// Create the stacked page of widgets for configuring the report order.
	{
		m_ReportDoOnce = new QCheckBox(tr("Do-Once Order"), this);
		m_ReportDoOnce->setCheckState(m_IsDoOnce ? Qt::Checked : Qt::Unchecked);
		m_ReportDoOnce->setToolTip(tr("<html>When checked, the order is only executed once, even if the mission is cyclic. "
					"This is useful when a temporary order is desired that does not permanently alter a spacecraft's mission "
					"programming.</html>"));

		m_ReportChannelCombo = new QComboBox(this);
		m_ReportChannelCombo->setEditable(true);
		m_ReportChannelCombo->setToolTip(tr("<html>"
					"Comm channel on which to transmit the report. "
					"Officers are restricted from broadcasting on the Galactic channel and a few others. "
					"The order is rejected if a restricted channel is chosen. "
					"</html>"));
		QStringList channelNames;
		AuCommStdChannel::ListOfficerReportChannels(channelNames);
		QMap<QString, bool> sortedNames;
		for(QStringList::const_iterator ci = channelNames.cbegin(); ci != channelNames.cend(); ci++)
			sortedNames.insert(*ci, true);

		for(QMap<QString, bool>::const_iterator ci = sortedNames.cbegin(); ci != sortedNames.cend(); ci++)
			m_ReportChannelCombo->addItem(ci.key());
		m_ReportChannelCombo->setCurrentIndex(m_ReportChannelCombo->findText(m_ReportChannel));
		connect(m_ReportChannelCombo, SIGNAL(currentTextChanged(const QString&)), SLOT(ReportChannelChanged(const QString&)));

		m_ReportRangeCombo = new QComboBox(this);
		m_ReportRangeCombo->setToolTip(tr("<html>"
					"Range to transmit the report. "
					"</html>"));
		for(int r = AuCommStdChannel::Close; r < AuCommStdChannel::RANGE_Count; r++)
			m_ReportRangeCombo->addItem(AuCommStdChannel::RangeName((AuCommStdChannel::RANGE)r), r);
		m_ReportRangeCombo->setCurrentIndex(m_ReportRangeCombo->findData(m_ReportRange));
		connect(m_ReportRangeCombo, SIGNAL(currentIndexChanged(int)), SLOT(ReportRangeChanged(int)));

		m_ReportSayEdit = new QLineEdit(m_ReportSay, this);
		m_ReportSayEdit->setToolTip(tr("<html>"
					"Statement to be said at the beginning of the report. "
					"If left blank, the statement is, '<i>Shipname</i> reports as ordered.'"
					"</html>"));

		m_ReportPosition = new QCheckBox(tr("Report Position"), this);
		m_ReportPosition->setCheckState(m_ReportWhat & 0x0001 ? Qt::Checked : Qt::Unchecked);
		m_ReportPosition->setToolTip(tr("<html>"
					"Includes position information in the report."
					"</html>"));

		m_ReportCrew = new QCheckBox(tr("Report Crew Status"), this);
		m_ReportCrew->setCheckState(m_ReportWhat & 0x0002 ? Qt::Checked : Qt::Unchecked);
		m_ReportCrew->setToolTip(tr("<html>"
					"Includes crew status in the report."
					"A summary of their health is provided if the ship has a functioning sick bay. "
					"</html>"));

		m_ReportFuel = new QCheckBox(tr("Report Fuel Status"), this);
		m_ReportFuel->setCheckState(m_ReportWhat & 0x0004 ? Qt::Checked : Qt::Unchecked);
		m_ReportFuel->setToolTip(tr("<html>"
					"Includes fuel status in the report."
					"</html>"));

		m_ReportCargo = new QCheckBox(tr("Report Cargo Status"), this);
		m_ReportCargo->setCheckState(m_ReportWhat & 0x0008 ? Qt::Checked : Qt::Unchecked);
		m_ReportCargo->setToolTip(tr("<html>"
					"Includes cargo loaded vs hold capacity in the report. "
					"Does not list all things in the hold. "
					"</html>"));

		m_ReportDamage = new QCheckBox(tr("Report Damage Summary"), this);
		m_ReportDamage->setCheckState(m_ReportWhat & 0x0010 ? Qt::Checked : Qt::Unchecked);
		m_ReportDamage->setToolTip(tr("<html>"
					"Includes current spacecraft health summary. "
					"Does not provide a detailed report; use the <i>Damage Report</i> order to get more information. "
					"</html>"));

		m_ReportSensor = new QCheckBox(tr("Report Sensor Summary"), this);
		m_ReportSensor->setCheckState(m_ReportWhat & 0x0020 ? Qt::Checked : Qt::Unchecked);
		m_ReportSensor->setToolTip(tr("<html>"
					"Includes a summary of current sensor contacts. "
					"</html>"));

		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(m_ReportDoOnce);
		lay->addWidget(m_ReportChannelCombo);
		lay->addWidget(m_ReportRangeCombo);
		lay->addWidget(m_ReportSayEdit);
		lay->addWidget(m_ReportPosition);
		lay->addWidget(m_ReportCrew);
		lay->addWidget(m_ReportFuel);
		lay->addWidget(m_ReportCargo);
		lay->addWidget(m_ReportDamage);
		lay->addWidget(m_ReportSensor);
		lay->addStretch();
		lay->setMargin(2);
		lay->setSpacing(2);

		QWidget* pageOpt = new QWidget(this);
		pageOpt->setLayout(lay);

		m_Stack->addWidget(pageOpt);
		m_ReportPage = pageIndex++;
	}

	// Orders list is created last so that any signals emitted by selecting an order can find the widgets they need.
	m_Orders = new QListWidget(this);
	m_Orders->setToolTip(tr("<html>Select a mission order.</html>"));
	connect(m_Orders, SIGNAL(itemSelectionChanged()), SLOT(OrderSelected()));
	{
		AuMissionOrderGoToCoordinate oGoToCoordinate;
		oGoToCoordinate.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToCoordinate);

		AuMissionOrderGoToSystemDirect oGoToSystemDirect;
		oGoToSystemDirect.AddListWidgetEntry(m_Orders,
				m_Order == AuMissionOrder::GoToSystemDirect || m_Order == AuMissionOrder::GoToSystemDirect_Deprecated);

		AuMissionOrderGoToSystemViaWormholes oGoToSystemViaWormholes;
		oGoToSystemViaWormholes.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToSystemViaWormholes);

		AuMissionOrderAttack oAttack;
		oAttack.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Attack);

		AuMissionOrderPlunder oPlunder;
		oPlunder.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Plunder);

		AuMissionOrderFollow oFollow;
		oFollow.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Follow);

		AuMissionOrderGoToHoverSite oGoToHoverSite;
		oGoToHoverSite.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToHoverSite);

		AuMissionOrderGoToCity oGoToCity;
		oGoToCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToCity);

		//AuMissionOrderGoToBestCity oGoToBestCity;
		//oGoToBestCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToBestCity);

		AuMissionOrderGoToClosestCity oGoToClosestCity;
		oGoToClosestCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToClosestCity);

		AuMissionOrderGoToCityAnywhere oGoToCityAnywhere;
		oGoToCityAnywhere.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToCityAnywhere);

		AuMissionOrderGoToLandingSite oGoToLandingSite;
		oGoToLandingSite.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToLandingSite);

		AuMissionOrderGoToLand oGoToLand;
		oGoToLand.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToLand);

		AuMissionOrderGoToStation oGoToStation;
		oGoToStation.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToStation);

		//AuMissionOrderGoToBestStation oGoToBestStation;
		//oGoToBestStation.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToBestStation);

		//AuMissionOrderGoToBestStationOrCity oGoToBestStationOrCity;
		//oGoToBestStationOrCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToBestStationOrCity);

		AuMissionOrderGoToClosestStation oGoToClosestStation;
		oGoToClosestStation.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToClosestStation);

		AuMissionOrderGoToClosestStationOrCity oGoToClosestStationOrCity;
		oGoToClosestStationOrCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToClosestStationOrCity);

		AuMissionOrderGoToOrbit oGoToOrbit;
		oGoToOrbit.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToOrbit);

		AuMissionOrderGoToOrbitBestPlanet oGoToOrbitBestPlanet;
		oGoToOrbitBestPlanet.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToOrbitBestPlanet);

		AuMissionOrderGoToOrbitCity oGoToOrbitCity;
		oGoToOrbitCity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToOrbitCity);

		AuMissionOrderGoToOrbitCoords oGoToOrbitCoords;
		oGoToOrbitCoords.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::GoToOrbitCoords);

		AuMissionOrderPatrolCities oPatrolCities;
		oPatrolCities.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PatrolCities);

		AuMissionOrderPatrolSystem oPatrolSystem;
		oPatrolSystem.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PatrolSystem);

		AuMissionOrderPatrolStations oPatrolStations;
		oPatrolStations.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PatrolStations);

		AuMissionOrderPatrolWormholes oPatrolWormholes;
		oPatrolWormholes.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PatrolWormholes);

		AuMissionOrderHarvest oHarvest;
		oHarvest.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Harvest);

		AuMissionOrderHide oHide;
		oHide.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Hide);

		AuMissionOrderShieldsDown oShieldsDown;
		oShieldsDown.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ShieldsDown);

		AuMissionOrderStandDown oStandDown;
		oStandDown.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::StandDown);

		AuMissionOrderStay oStay;
		oStay.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Stay);

		AuMissionOrderCryoBerthsUnload oCryoBerthsUnload;
		oCryoBerthsUnload.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CryoBerthsUnload);

		AuMissionOrderPassengersUnload oPassengersUnload;
		oPassengersUnload.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PassengersUnload);

		AuMissionOrderCryoBerthsLoad oCryoBerthsLoad;
		oCryoBerthsLoad.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CryoBerthsLoad);

		AuMissionOrderPassengersLoad oPassengersLoad;
		oPassengersLoad.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PassengersLoad);

		AuMissionOrderCrewLoad oCrewLoad;
		oCrewLoad.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CrewLoad);

		AuMissionOrderTroopsLoad oTroopsLoad;
		oTroopsLoad.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TroopsLoad);

		AuMissionOrderCargoSellCategory oCargoSellCategory;
		oCargoSellCategory.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CargoSellCategory);

		AuMissionOrderCargoSellCommodity oCargoSellCommodity;
		oCargoSellCommodity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CargoSellCommodity);

		AuMissionOrderCargoBuyCategory oCargoBuyCategory;
		oCargoBuyCategory.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CargoBuyCategory);

		AuMissionOrderCargoBuyCommodity oCargoBuyCommodity;
		oCargoBuyCommodity.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::CargoBuyCommodity);

		AuMissionOrderTransferCargoCategoryTo oTransferCargoCategoryTo;
		oTransferCargoCategoryTo.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferCargoCategoryTo);

		AuMissionOrderTransferCargoTo oTransferCargoTo;
		oTransferCargoTo.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferCargoTo);

		AuMissionOrderTransferCargoCategoryFrom oTransferCargoCategoryFrom;
		oTransferCargoCategoryFrom.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferCargoCategoryFrom);

		AuMissionOrderTransferCargoFrom oTransferCargoFrom;
		oTransferCargoFrom.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferCargoFrom);

		AuMissionOrderTransferMoneyTo oTransferMoneyTo;
		oTransferMoneyTo.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferMoneyTo);

		AuMissionOrderTransferMoneyFrom oTransferMoneyFrom;
		oTransferMoneyFrom.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferMoneyFrom);

		AuMissionOrderTransferPeopleTo oTransferPeopleTo;
		oTransferPeopleTo.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferPeopleTo);

		AuMissionOrderTransferPeopleFrom oTransferPeopleFrom;
		oTransferPeopleFrom.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::TransferPeopleFrom);

		AuMissionOrderExterminateRats oExterminate;
		oExterminate.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ExterminateRats);

		AuMissionOrderRepair oRepair;
		oRepair.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Repair);

		AuMissionOrderDamageReport oDamageReport;
		oDamageReport.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::DamageReport);

		//AuMissionOrderScanSurveyPlanet oScanSurveyPlanet;
		//oScanSurveyPlanet.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ScanSurveyPlanet);

		AuMissionOrderScanSurveySystem oScanSurveySystem;
		oScanSurveySystem.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ScanSurveySystem);

		AuMissionOrderScanTactical oScanTactical;
		oScanTactical.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ScanTactical);

		AuMissionOrderReport oReport;
		oReport.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Report);

		AuMissionOrderHarvestOperationByBay oHarvestOperationByBay;
		oHarvestOperationByBay.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestOperationByBay);

		AuMissionOrderHarvestOperationByModule oHarvestOperationByModule;
		oHarvestOperationByModule.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestOperationByModule);

		AuMissionOrderHarvestAtmosphere oHarvestAtmosphere;
		oHarvestAtmosphere.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestAtmosphere);

		AuMissionOrderHarvestMinerals oHarvestMinerals;
		oHarvestMinerals.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestMinerals);

		AuMissionOrderHarvestParticles oHarvestParticles;
		oHarvestParticles.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestParticles);

		AuMissionOrderHarvestSea oHarvestSea;
		oHarvestSea.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestSea);

		AuMissionOrderHarvestWell oHarvestWell;
		oHarvestWell.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HarvestWell);

		AuMissionOrderPosture oPosture;
		oPosture.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Posture);

		AuMissionOrderChristen oChristen;
		oChristen.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Christen);

		AuMissionOrderHoldMission oHoldMission;
		oHoldMission.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::HoldMission);

		AuMissionOrderLightState oLightState;
		oLightState.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::LightState);

		AuMissionOrderPilotPolicy oPilotPolicy;
		oPilotPolicy.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::PilotPolicy);

		AuMissionOrderShieldPolicy oShieldPolicy;
		oShieldPolicy.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::ShieldPolicy);

		AuMissionOrderScuttle oScuttle;
		oScuttle.AddListWidgetEntry(m_Orders, m_Order == AuMissionOrder::Scuttle);
	}

	// Assemble the widgets into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Horizontal);
	m_Splitter->addWidget(m_Orders);
	m_Splitter->addWidget(m_Stack);
	m_Splitter->setCollapsible(0, false);						// Prevent starmap from collapsing to 0. People lose it.
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QGridLayout* laySplit = new QGridLayout;
	laySplit->addWidget(m_Splitter, 0,0);
	laySplit->setMargin(0);
	laySplit->setSpacing(0);
	setLayout(laySplit);

	// Restore splitter state.
	QByteArray splitterState;
	g_Au.Settings().readEntry("MissionOrderWidget/Splitter", splitterState);
	if(!splitterState.isEmpty())
		m_Splitter->restoreState(splitterState);

	// Show/hide appropriate controls.
	OrderSelected();
}

AuMissionOrderWidget::~AuMissionOrderWidget()
{
}

void AuMissionOrderWidget::GetValuesFromOrder(const AuMissionOrder* order)
{
	if(!order)
		return;

	m_IsDoOnce = order->IsDoOnce();

	m_Order = order->Type();
	switch(m_Order)
	{
	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
	case AuMissionOrder::ExterminateRats:
	case AuMissionOrder::GoToBestCity:
	case AuMissionOrder::GoToBestStation:
	case AuMissionOrder::GoToBestStationOrCity:
	case AuMissionOrder::GoToClosestCity:
	case AuMissionOrder::GoToClosestStation:
	case AuMissionOrder::GoToClosestStationOrCity:
	case AuMissionOrder::GoToOrbitBestPlanet:
	case AuMissionOrder::HoldMission:
		break;

	case AuMissionOrder::Report:
		m_ReportChannel = order->CommChannel();
		m_ReportRange = order->CommRange();
		m_ReportWhat = order->ReportWhat();
		m_ReportSay = order->SayText();
		break;

	case AuMissionOrder::TransferCargoCategoryFrom:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_Category = order->CommodityFilter();

		m_CatBuyType = order->AmountType();
		if(m_CatBuyType == 0)
			m_AmountBuyPercent = order->Amount();
		else
			m_AmountBuyUnits = order->Amount();
		break;

	case AuMissionOrder::TransferCargoCategoryTo:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_Category = order->CommodityFilter();

		m_CatSellType = order->AmountType();
		if(m_CatSellType == 0)
			m_AmountSellPercent = order->Amount();
		else
			m_AmountSellUnits = order->Amount();
		break;

	case AuMissionOrder::TransferCargoFrom:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_CIDTransfer = order->CommodityId();

		m_AmountBuyType = order->AmountType();
		if(m_AmountBuyType == 0)
			m_AmountBuyPercent = order->Amount();
		else
			m_AmountBuyUnits = order->Amount();
		break;

	case AuMissionOrder::TransferCargoTo:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_CIDTransfer = order->CommodityId();

		m_AmountSellType = order->AmountType();
		if(m_AmountSellType == 0)
			m_AmountSellPercent = order->Amount();
		else
			m_AmountSellUnits = order->Amount();
		break;

	case AuMissionOrder::TransferMoneyFrom:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_TransferMoneyFromType = order->AmountType();
		if(m_TransferMoneyFromType == 0)
			m_TransferMoneyFromPercent = order->Amount();
		else
			m_TransferMoneyFromCount = order->AmountOfMoney();
		break;

	case AuMissionOrder::TransferMoneyTo:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_TransferMoneyToType = order->AmountType();
		if(m_TransferMoneyToType == 0)
			m_TransferMoneyToPercent = order->Amount();
		else
			m_TransferMoneyToCount = order->AmountOfMoney();
		break;

	case AuMissionOrder::TransferPeopleFrom:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_TransferPeopleFromType = order->AmountType();
		m_TransferPeopleFromBerth = order->PeopleType();;
		if(m_TransferPeopleFromType == 0)
			m_TransferPeopleFromPercent = order->Amount();
		else
			m_TransferPeopleFromCount = order->Amount();
		break;

	case AuMissionOrder::TransferPeopleTo:
		m_Transferee = order->TargetName();
		m_TransferTarget = m_Transferee.isEmpty() ? 0 : 1;

		m_TransferPeopleToType = order->AmountType();
		m_TransferPeopleToBerth = order->PeopleType();;
		if(m_TransferPeopleToType == 0)
			m_TransferPeopleToPercent = order->Amount();
		else
			m_TransferPeopleToCount = order->Amount();
		break;

	case AuMissionOrder::Attack:
	case AuMissionOrder::Follow:
	case AuMissionOrder::Plunder:
		m_TargetId = order->TargetId();
		m_TargetName = order->TargetName();
		break;

	case AuMissionOrder::CargoBuyCategory:
		m_Category = order->CommodityFilter();

		m_CatBuyType = order->AmountType();
		if(m_CatBuyType == 0)
			m_AmountBuyPercent = order->Amount();
		else
			m_AmountBuyUnits = order->Amount();

		m_MarginBuyType = order->PriceMarginType();
		if(m_MarginBuyType == 0)
			m_MarginBuyPercent = order->PriceMarginValue();
		else if(m_MarginBuyType == 1)
			m_MarginBuyCost = order->PriceMarginValue();
		break;

	case AuMissionOrder::CargoBuyCommodity:
		m_CIDBuy = order->CommodityId();

		m_AmountBuyType = order->AmountType();
		if(m_AmountBuyType == 0)
			m_AmountBuyPercent = order->Amount();
		else
			m_AmountBuyUnits = order->Amount();

		m_MarginBuyType = order->PriceMarginType();
		if(m_MarginBuyType == 0)
			m_MarginBuyPercent = order->PriceMarginValue();
		else if(m_MarginBuyType == 1)
			m_MarginBuyCost = order->PriceMarginValue();
		break;

	case AuMissionOrder::CargoSellCategory:
		m_Category = order->CommodityFilter();

		m_CatSellType = order->AmountType();
		if(m_CatSellType == 0)
			m_AmountSellPercent = order->Amount();
		else
			m_AmountSellUnits = order->Amount();

		m_MarginSellType = order->PriceMarginType();
		if(m_MarginSellType == 0)
			m_MarginSellPercent = order->PriceMarginValue();
		else if(m_MarginSellType == 1)
			m_MarginSellCost = order->PriceMarginValue();
		break;

	case AuMissionOrder::CargoSellCommodity:
		m_CIDBuy = order->CommodityId();

		m_AmountSellType = order->AmountType();
		if(m_AmountSellType == 0)
			m_AmountSellPercent = order->Amount();
		else
			m_AmountSellUnits = order->Amount();

		m_MarginSellType = order->PriceMarginType();
		if(m_MarginSellType == 0)
			m_MarginSellPercent = order->PriceMarginValue();
		else if(m_MarginSellType == 1)
			m_MarginSellCost = order->PriceMarginValue();
		break;

	case AuMissionOrder::Christen:
		m_ShipName = order->ShipName().trimmed();
		break;

	case AuMissionOrder::CrewLoad:
	case AuMissionOrder::PassengersLoad:
	case AuMissionOrder::TroopsLoad:
		m_Count = order->HeadCount();
		m_PreboardTime = order->PreboardingAnnouncementMinutes();
		break;

	case AuMissionOrder::Repair:
		m_MinQ = order->MinQ();
		m_ApplyQ = order->ApplyMinQ();
		break;

	case AuMissionOrder::DamageReport:
	case AuMissionOrder::ScanSurveyPlanet:
	case AuMissionOrder::ScanSurveySystem:
	case AuMissionOrder::ScanTactical:
		m_IsMailCoC = order->ReportToCommand();
		break;

	case AuMissionOrder::GoToCity:
		m_AirportId = order->DestinationId();
		m_AirportName = order->DestinationName();
		break;

	case AuMissionOrder::GoToCityAnywhere:
	case AuMissionOrder::GoToOrbitCity:
		m_CityId = order->DestinationId();
		m_CityName = order->DestinationName();
		break;

	case AuMissionOrder::GoToCoordinate:
		m_GoToRelative = order->DestinationContext();
		m_GoTo = order->DestinationCoordinate();
		break;

	case AuMissionOrder::HarvestOperationByBay:
		m_HarvestPlanetId = order->HarvestId();
		m_HarvestPlanetName = order->HarvestName();
		m_HarvestFuelThreshold = order->FuelThreshold();
		m_HarvestOps = order->HarvestOps();
		break;

	case AuMissionOrder::HarvestOperationByModule:
		m_HarvestPlanetIdByM = order->HarvestId();
		m_HarvestPlanetNameByM = order->HarvestName();
		m_HarvestFuelThresholdByM = order->FuelThreshold();
		m_HarvestOpsByM = order->HarvestOps();
		break;

	case AuMissionOrder::GoToHoverSite:
		m_PlanetId = order->DestinationId();
		m_PlanetName = order->DestinationName();
		m_PlanetLongLat = order->GlobeLL().ToString();
		m_HoverAlt = order->Altitude();
		break;

	case AuMissionOrder::GoToLand:
	case AuMissionOrder::GoToOrbit:
		m_PlanetId = order->DestinationId();
		m_PlanetName = order->DestinationName();
		break;

	case AuMissionOrder::GoToLandingSite:
	case AuMissionOrder::GoToOrbitCoords:
		m_PlanetId = order->DestinationId();
		m_PlanetName = order->DestinationName();
		m_PlanetLongLat = order->GlobeLL().ToString();
		break;

	case AuMissionOrder::GoToStation:
		m_StationId = order->DestinationId();
		m_StationName = order->DestinationName();
		break;

	case AuMissionOrder::GoToSystemDirect_Deprecated:
		m_Order = AuMissionOrder::GoToSystemDirect;
		[[fallthrough]];
	case AuMissionOrder::GoToSystemDirect:
		m_SystemGalaxyLoc = order->DestinationCoordinate();
		m_SystemId = order->DestinationId();
		m_SystemName = order->DestinationName();
		break;

	case AuMissionOrder::GoToSystemViaWormholes:
		m_RouteSystemId = order->DestinationId();
		m_RouteSystemName = order->DestinationName();
		break;

	case AuMissionOrder::Harvest:
	case AuMissionOrder::Hide:
	case AuMissionOrder::PatrolCities:
	case AuMissionOrder::PatrolStations:
	case AuMissionOrder::PatrolSystem:
	case AuMissionOrder::PatrolWormholes:
	case AuMissionOrder::Scuttle:
	case AuMissionOrder::ShieldsDown:
	case AuMissionOrder::StandDown:
	case AuMissionOrder::Stay:
		m_Minutes = order->Minutes();
		break;

	case AuMissionOrder::HarvestAtmosphere:
		m_CIDAtmo = order->CommodityId();
		if(order->AllBays())
			m_BayWhich = 256;
		else
			m_BayWhich = order->WhichBay();
		m_PlanetId = order->HarvestId();
		m_PlanetName = order->HarvestName();
		break;

	case AuMissionOrder::HarvestMinerals:
		m_CIDMin = order->CommodityId();
		if(order->AllBays())
			m_BayWhich = 256;
		else
			m_BayWhich = order->WhichBay();
		m_PlanetId = order->HarvestId();
		m_PlanetName = order->HarvestName();
		break;

	case AuMissionOrder::HarvestParticles:
		m_CIDPart = order->CommodityId();
		if(order->AllBays())
			m_BayWhich = 256;
		else
			m_BayWhich = order->WhichBay();
		m_PlanetId = order->HarvestId();
		m_PlanetName = order->HarvestName();
		break;

	case AuMissionOrder::HarvestSea:
		m_CIDSea = order->CommodityId();
		if(order->AllBays())
			m_BayWhich = 256;
		else
			m_BayWhich = order->WhichBay();
		m_PlanetId = order->HarvestId();
		m_PlanetName = order->HarvestName();
		break;

	case AuMissionOrder::HarvestWell:
		m_CIDWell = order->CommodityId();
		if(order->AllBays())
			m_BayWhich = 256;
		else
			m_BayWhich = order->WhichBay();
		m_PlanetId = order->HarvestId();
		m_PlanetName = order->HarvestName();
		break;

	case AuMissionOrder::LightState:
		m_LightStateHeadlight = order->HeadlightOn();
		m_LightStateNavLight = order->NavLightOn();
		m_LightStateRotatingBeacon = order->RotatingBeaconOn();
		m_LightStateStrobe = order->StrobeOn();
		break;

	case AuMissionOrder::PassengersUnload:
	case AuMissionOrder::CryoBerthsLoad:
	case AuMissionOrder::CryoBerthsUnload:
		m_Count = order->HeadCount();
		break;

	case AuMissionOrder::Posture:
		m_Posture = order->CitizenPosture();
		break;

	case AuMissionOrder::PilotPolicy:
		m_PilotUpPolicy = order->PilotUpPolicy();
		break;

	case AuMissionOrder::ShieldPolicy:
		m_ShieldFuelPolicy = order->ShipShieldFuelPolicy();
		m_ShieldPolicy = order->ShipShieldPolicy();
		break;
	}
}

void AuMissionOrderWidget::EnableControls()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	// Show/hide controls appropriate to the selected order.
	bool contentValid = true;			// validity of contents

	// We can't hide everything then unhide the ones we want because it breaks focus.
	// Instead, we will make a set of the controls that need to be visible, called 'showit'. Then we'll show/hide all controls
	// based on their presense in the showit set of widgets.
	QSet<QWidget*> showit;				// Set of widgets that should be visible.
	showit.insert(m_Smashie);			// Almost always needed. The exceptions will remove it from the set.
	showit.insert(m_DoOnce);			// Almost always needed. The exceptions will remove it from the set.
	int pageIndex = 0;
	switch(m_Order)
	{
	case AuMissionOrder::HarvestOperationByBay:
		showit.clear();					// Page has all the controls it needs.
		pageIndex = m_HarvestByBayPage;
		break;

	case AuMissionOrder::HarvestOperationByModule:
		showit.clear();					// Page has all the controls it needs.
		pageIndex = m_HarvestByModulePage;
		break;

	case AuMissionOrder::Report:
		showit.clear();					// Page has all the controls it needs.
		pageIndex = m_ReportPage;
		break;

	case AuMissionOrder::TransferCargoCategoryFrom:
		showit.insert(m_TransferFromLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_CommodityFilter);
		showit.insert(m_TransferCargoCountLabel);

		if(m_Category == FILTER_SpacecraftDesigns || m_Category == FILTER_BuildingDesigns)
			contentValid = false;	// AI can't really buy/sell these.

		showit.insert(m_CatBuyActualRadio);
		showit.insert(m_CatBuyEachRadio);
		showit.insert(m_CatBuyPercentRadio);
		showit.insert(m_CatTransferFromLabel);

		m_CatBuyPercentRadio->setText(tr("Percent of Their Cargo"));
		m_CatBuyPercentRadio->setToolTip(tr("<html>Transfer quantity is specified as a percentage of the total amount of the "
					"commodity aboard the other spacecraft. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));
		m_CatBuyActualRadio->setToolTip(tr("<html>Transfer quantity is specified as an actual number of units. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));
		m_CatBuyEachRadio->setToolTip(tr("<html>Transfer quantity is specified as an actual number of each kind of commodity "
					"in the category. "
					"The officer will attempt to transfer up to this number of each kind of commodity in the category. "
					"</html>"));

		if(m_CatBuyType == 0)
		{
			showit.insert(m_CatTransferFromPercentLabel);
			showit.insert(m_CatBuySlider);
			m_CatBuySlider->setToolTip(tr("<html>Transfer commodity amount, as a percentage of gross hold space, "
						"fuel cells when transferring hydrogen, and capacitor when transferring electricity. "
						"Specifies a limit on space allocated to the commodity; does not specify an amount to transfer.</html>"));
		}
		else
		{
			showit.insert(m_CatBuyAmountEdit);
			showit.insert(m_CatTransferFromAmountLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferCargoCategoryTo:
		showit.insert(m_TransferToLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_CommodityFilter);
		showit.insert(m_TransferCargoCountLabel);

		if(m_Category == FILTER_SpacecraftDesigns || m_Category == FILTER_BuildingDesigns)
			contentValid = false;	// AI can't really buy/sell these.

		showit.insert(m_CatSellActualRadio);
		showit.insert(m_CatSellEachRadio);
		showit.insert(m_CatSellPercentRadio);
		showit.insert(m_CatTransferToLabel);

		m_CatSellPercentRadio->setText(tr("Percent of Our Cargo"));
		m_CatSellPercentRadio->setToolTip(tr("<html>Transfer quantity is specified as a percentage of the total amount of the "
					"commodity aboard the mission spacecraft. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));
		m_CatSellActualRadio->setText(tr("Quantity of Hold Space"));
		m_CatSellActualRadio->setToolTip(tr("<html>Transfer quantity is specified as a number of total units. "
					"The officer will attempt to transfer up to this total number of commodities."
					"</html>"));

		if(m_CatSellType == 0)
		{
			showit.insert(m_CatTransferToPercentLabel);
			showit.insert(m_CatSellSlider);
			m_CatSellSlider->setToolTip(tr("<html>Transfer commodity amount, as a percentage of gross hold space, "
						"fuel cells when transferring hydrogen, and capacitor when transferring electricity. "
						"Specifies a maximum amount to transfer.</html>"));
		}
		else
		{
			showit.insert(m_CatSellAmountEdit);
			showit.insert(m_CatTransferToAmountLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferCargoFrom:
		showit.insert(m_TransferFromLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_CommoditiesTransfer);
		showit.insert(m_TransferCargoCountLabel);

		if(m_CIDTransfer == CID_Invalid)
			contentValid = false;

		showit.insert(m_AmountBuyActualRadio);
		showit.insert(m_AmountTransferFromLabel);
		showit.insert(m_AmountBuyPercentRadio);

		m_AmountBuyPercentRadio->setText(tr("Percent of Their Cargo"));
		m_AmountBuyPercentRadio->setToolTip(tr("<html>Transfer quantity is specified as a percentage of the total amount of the "
					"commodity aboard the other spacecraft. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));
		m_AmountBuyActualRadio->setToolTip(tr("<html>Transfer quantity is specified as an actual number of units. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));

		if(m_AmountBuyType == 0)
		{
			showit.insert(m_TransferCargoFromLabel);
			showit.insert(m_BuyCargoSlider);
			m_BuyCargoSlider->setToolTip(tr("<html>Transfer commodity amount, as a percentage of gross hold space, "
						"fuel cells when transferring hydrogen, and capacitor when transferring electricity. "
						"Specifies a limit on space allocated to the commodity; does not specify an amount to transfer.</html>"));
		}
		else
		{
			showit.insert(m_BuyCargoAmountEdit);
			showit.insert(m_TransferCargoFromAmountLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferCargoTo:
		showit.insert(m_TransferToLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_CommoditiesTransfer);
		showit.insert(m_TransferCargoCountLabel);

		if(m_CIDTransfer == CID_Invalid)
			contentValid = false;

		showit.insert(m_AmountSellActualRadio);
		showit.insert(m_AmountTransferToLabel);
		showit.insert(m_AmountSellPercentRadio);

		m_AmountSellPercentRadio->setText(tr("Percent of Our Cargo"));
		m_AmountSellPercentRadio->setToolTip(tr("<html>Transfer quantity is specified as a percentage of the total amount of the "
					"commodity aboard the mission spacecraft. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));
		m_AmountSellActualRadio->setToolTip(tr("<html>Transfer quantity is specified as an actual number of units. "
					"The officer will attempt to transfer up to this number of commodities."
					"</html>"));

		if(m_AmountSellType == 0)
		{
			showit.insert(m_TransferCargoToLabel);
			showit.insert(m_SellCargoSlider);
			m_SellCargoSlider->setToolTip(tr("<html>Transfer commodity amount, as a percentage of gross hold space, "
						"fuel cells when transferring hydrogen, and capacitor when transferring electricity. "
						"Specifies a maximum amount to transfer.</html>"));
		}
		else
		{
			showit.insert(m_SellCargoAmountEdit);
			showit.insert(m_TransferCargoToAmountLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferMoneyFrom:
		showit.insert(m_TransferFromLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_TransferMoneyFromLabel);
		showit.insert(m_TransferMoneyFromAmountRadio);
		if(m_TransferMoneyFromAmountRadio->isChecked())
			showit.insert(m_TransferMoneyFromAmountEdit);
		showit.insert(m_TransferMoneyFromPercentRadio);
		if(m_TransferMoneyFromPercentRadio->isChecked())
		{
			showit.insert(m_TransferMoneyFromPercentSlider);
			showit.insert(m_TransferMoneyFromPercentLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferMoneyTo:
		showit.insert(m_TransferToLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_TransferMoneyToLabel);
		showit.insert(m_TransferMoneyToAmountRadio);
		if(m_TransferMoneyToAmountRadio->isChecked())
			showit.insert(m_TransferMoneyToAmountEdit);
		showit.insert(m_TransferMoneyToPercentRadio);
		if(m_TransferMoneyToPercentRadio->isChecked())
		{
			showit.insert(m_TransferMoneyToPercentSlider);
			showit.insert(m_TransferMoneyToPercentLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferPeopleFrom:
		showit.insert(m_TransferFromLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_TransferPeopleFromBerthLabel);
		showit.insert(m_TransferPeopleFromOfficersRadio);
		showit.insert(m_TransferPeopleFromCrewRadio);
		showit.insert(m_TransferPeopleFromTroopsRadio);
		showit.insert(m_TransferPeopleFromPassengersRadio);

		showit.insert(m_TransferPeopleFromLabel);
		showit.insert(m_TransferPeopleFromBerthLabel);
		showit.insert(m_TransferPeopleFromAmountRadio);
		if(m_TransferPeopleFromAmountRadio->isChecked())
			showit.insert(m_TransferPeopleFromAmountEdit);
		showit.insert(m_TransferPeopleFromPercentRadio);
		if(m_TransferPeopleFromPercentRadio->isChecked())
		{
			showit.insert(m_TransferPeopleFromPercentSlider);
			showit.insert(m_TransferPeopleFromPercentLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::TransferPeopleTo:
		showit.insert(m_TransferToLabel);
		showit.insert(m_TransferStation);
		showit.insert(m_TransferNamedShip);
		if(m_TransferTarget)
			showit.insert(m_TransfereeEdit);

		showit.insert(m_TransferPeopleToBerthLabel);
		showit.insert(m_TransferPeopleToOfficersRadio);
		showit.insert(m_TransferPeopleToCrewRadio);
		showit.insert(m_TransferPeopleToTroopsRadio);
		showit.insert(m_TransferPeopleToPassengersRadio);

		showit.insert(m_TransferPeopleToLabel);
		showit.insert(m_TransferPeopleToBerthLabel);
		showit.insert(m_TransferPeopleToAmountRadio);
		if(m_TransferPeopleToAmountRadio->isChecked())
			showit.insert(m_TransferPeopleToAmountEdit);
		showit.insert(m_TransferPeopleToPercentRadio);
		if(m_TransferPeopleToPercentRadio->isChecked())
		{
			showit.insert(m_TransferPeopleToPercentSlider);
			showit.insert(m_TransferPeopleToPercentLabel);
		}

		showit.insert(m_TransferAuthorizationLabel);
		break;

	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
#if defined(QT_NODEBUG)
	default:							// No debug default to see compiler warnings if one is omitted. Release wants this default.
#endif
		showit.remove(m_DoOnce);		// No order selected.
		contentValid = false;
	case AuMissionOrder::ExterminateRats:
	case AuMissionOrder::GoToBestCity:
	case AuMissionOrder::GoToBestStation:
	case AuMissionOrder::GoToBestStationOrCity:
	case AuMissionOrder::GoToClosestCity:
	case AuMissionOrder::GoToClosestStation:
	case AuMissionOrder::GoToClosestStationOrCity:
	case AuMissionOrder::GoToOrbitBestPlanet:
	case AuMissionOrder::HoldMission:
		// All controls hidden.
		break;

	case AuMissionOrder::Repair:
		showit.insert(m_ApplyShipQRadio);
		showit.insert(m_ApplyMinQRadio);
		showit.insert(m_ApplySystemQRadio);
		showit.insert(m_MinQLabel);
		showit.insert(m_MinQSpin);
		break;

	case AuMissionOrder::LightState:
		showit.insert(m_LightHeadlightLabel);
		showit.insert(m_LightHeadlightOff);
		showit.insert(m_LightHeadlightOn);
		showit.insert(m_LightNavLightLabel);
		showit.insert(m_LightNavLightOff);
		showit.insert(m_LightNavLightOn);
		showit.insert(m_LightRotatingBeaconLabel);
		showit.insert(m_LightRotatingBeaconOff);
		showit.insert(m_LightRotatingBeaconOn);
		showit.insert(m_LightStrobeLabel);
		showit.insert(m_LightStrobeOff);
		showit.insert(m_LightStrobeOn);
		break;

	case AuMissionOrder::PilotPolicy:
		showit.insert(m_PilotUpPolicies);
		break;

	case AuMissionOrder::ShieldPolicy:
		showit.insert(m_ShieldPolicies);
		showit.insert(m_ShieldFuelPolicies);
		break;

	case AuMissionOrder::GoToCoordinate:
		showit.insert(m_GoToCoordLabel);
		showit.insert(m_GoToCoord);
		showit.insert(m_GoToGalaxy);
		showit.insert(m_GoToSector);
		showit.insert(m_GoToSystem);

		switch(m_GoToRelative)
		{
		case 0:
			// Crown of Othon galaxy has the largest radius. Each sector is 10 parsecs.
			if(fabs(m_GoTo.x) > DaGalaxy::Radius(CrownOfOthon))
				contentValid = false;
			if(fabs(m_GoTo.y) > DaGalaxy::Radius(CrownOfOthon))
				contentValid = false;
			if(fabs(m_GoTo.z) > DaGalaxy::Radius(CrownOfOthon)/3.875)	// Thickness factor from DaSector::StarDensity.
				contentValid = false;
			break;
		case 1:
			// Sector is 10x10x10 parsecs.
			if(fabs(m_GoTo.x) >= 5)
				contentValid = false;
			if(fabs(m_GoTo.y) >= 5)
				contentValid = false;
			if(fabs(m_GoTo.z) >= 5)
				contentValid = false;
			break;
		case 2:
			// Solar system coordinates are expressed in au.
			if(fabs(m_GoTo.x)*FeetPerAu() > FeetPerParsec())
				contentValid = false;
			if(fabs(m_GoTo.y)*FeetPerAu() > FeetPerParsec())
				contentValid = false;
			if(fabs(m_GoTo.z)*FeetPerAu() > FeetPerParsec())
				contentValid = false;
			break;
		default:
			contentValid = false;
			break;
		}
		break;

	case AuMissionOrder::Christen:
		showit.insert(m_ShipNameLabel);
		showit.insert(m_ShipNameEdit);

		contentValid = IsNameValid(m_ShipName);
		break;

	case AuMissionOrder::Harvest:
		showit.insert(m_DurWaitLabel);
		showit.insert(m_MinutesSpin);
		showit.insert(m_MinutesLabel);

		m_MinutesSpin->setToolTip(tr("<html>Harvest time in minutes. 0 results in endless harvest.</html>"));
		m_MinutesLabel->setText(tr("Minutes Duration"));
		
		if(m_Minutes < 0 || m_Minutes > 0xffff)
			contentValid = false;
		break;

	case AuMissionOrder::Scuttle:
		showit.insert(m_DurWaitLabel);
		showit.insert(m_MinutesSpin);
		showit.insert(m_MinutesLabel);

		if(isAgent)
			m_MinutesSpin->setToolTip(tr("<html>Seconds before self destruct. "
						"A three minute minimum is imposed before self destructing. "
						"<br><b>Agent Power</b> No minimum time when agent power is enabled. "
						"0 results in immediate scuttle. "
						"</html>"));
		else
			m_MinutesSpin->setToolTip(tr("<html>Seconds before self destruct. "
						"A three minute minimum (180 sec.) is imposed before self destructing. "
						"</html>"));
		m_MinutesLabel->setText(tr("Seconds Before Self Destruct"));
		
		if(m_Minutes < 0 || m_Minutes > 0xffff)
			contentValid = false;
		break;

	case AuMissionOrder::ShieldsDown:
		showit.insert(m_DurWaitLabel);
		showit.insert(m_MinutesSpin);
		showit.insert(m_MinutesLabel);

		m_MinutesSpin->setToolTip(tr("<html>Shield down time in seconds. 0 results in endless shield down.</html>"));
		m_MinutesLabel->setText(tr("Seconds Duration"));
		
		if(m_Minutes < 0 || m_Minutes > 0xffff)
			contentValid = false;
		break;

	case AuMissionOrder::Hide:
	case AuMissionOrder::StandDown:
	case AuMissionOrder::Stay:
		showit.insert(m_DurWaitLabel);
		showit.insert(m_MinutesSpin);
		showit.insert(m_MinutesLabel);

		m_MinutesSpin->setToolTip(tr("<html>Wait time in minutes. 0 results in endless wait.</html>"));
		m_MinutesLabel->setText(tr("Minutes Duration"));
		
		if(m_Minutes < 0 || m_Minutes > 0xffff)
			contentValid = false;
		break;

	case AuMissionOrder::CargoBuyCategory:
		showit.insert(m_BuyCargoCountLabel);
		showit.insert(m_CommodityFilter);

		if(m_Category == FILTER_SpacecraftDesigns || m_Category == FILTER_BuildingDesigns)
			contentValid = false;	// AI can't really buy/sell these.

		showit.insert(m_CatBuyActualRadio);
		showit.insert(m_CatBuyEachRadio);
		showit.insert(m_CatBuyLabel);
		showit.insert(m_CatBuyPercentRadio);
		showit.insert(m_MarginBuyActualRadio);
		showit.insert(m_MarginBuyAnyPriceRadio);
		showit.insert(m_MarginBuyLabel);
		showit.insert(m_MarginBuyPercentRadio);

		m_CatBuyPercentRadio->setText(tr("Percent of Hold Space"));
		m_CatBuyPercentRadio->setToolTip(tr("<html>Manifest quantity is specified as a percentage of the total hold space. "
					"The officer will attempt to purchase commodities until the percentage of hold space is filled."
					"</html>"));
		m_CatBuyActualRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of units. "
					"The officer will attempt to purchase commodities until the purchase quantity is filled."
					"</html>"));
		m_CatBuyEachRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of each kind of commodity "
					"in the category. "
					"The officer will attempt to purchase commodities until the hold contains this quantity of each kind of "
					"commodity in the category. "
					"</html>"));

		if(m_MarginBuyType == 0)
		{
			showit.insert(m_BuyMarginSlider);
			showit.insert(m_BuyMarginLabel);
		}
		else if(m_MarginBuyType == 1)
		{
			showit.insert(m_BuyCargoCostEdit);
			showit.insert(m_BuyCargoCostLabel);
		}

		if(m_CatBuyType == 0)
		{
			showit.insert(m_CatBuyPercentLabel);
			showit.insert(m_CatBuySlider);
			m_CatBuySlider->setToolTip(tr("<html>Buy commodity category amount, as a percentage of gross hold space. "
						"Specifies a limit on hold space allocated to the category; does not specify an amount to buy. "
						"The space is divided by all commodities in the category to determine how many of each to buy, at least 1. "
						"Will not buy hydrogen or electricity when buying a category of cargo.</html>"));
		}
		else
		{
			showit.insert(m_CatBuyAmountEdit);
			showit.insert(m_CatBuyAmountLabel);
		}
		break;
	case AuMissionOrder::CargoSellCategory:
		showit.insert(m_SellCargoCountLabel);
		showit.insert(m_CommodityFilter);

		if(m_Category == FILTER_SpacecraftDesigns || m_Category == FILTER_BuildingDesigns)
			contentValid = false;	// AI can't really buy/sell these.

		showit.insert(m_CatSellActualRadio);
		showit.insert(m_CatSellLabel);
		showit.insert(m_CatSellPercentRadio);
		showit.insert(m_MarginSellActualRadio);
		showit.insert(m_MarginSellAnyPriceRadio);
		showit.insert(m_MarginSellLabel);
		showit.insert(m_MarginSellPercentRadio);

		if(m_MarginSellType == 0)
		{
			showit.insert(m_SellMarginSlider);
			showit.insert(m_SellMarginLabel);
		}
		else if(m_MarginSellType == 1)
		{
			showit.insert(m_SellCargoCostEdit);
			showit.insert(m_SellCargoCostLabel);
		}

		m_CatSellPercentRadio->setText(tr("Percent of Hold Space"));
		m_CatSellPercentRadio->setToolTip(tr("<html>Sell quantity is specified as a percentage of the total hold space. "
					"The officer will attempt to sell up to this number of commodities."
					"</html>"));
		m_CatSellActualRadio->setText(tr("Quantity of Each Kind"));
		m_CatSellActualRadio->setToolTip(tr("<html>Sell quantity is specified as an actual number of units. "
					"The officer will attempt to sell up to this number of each kind of commodity in the category."
					"</html>"));

		if(m_CatSellType == 0)
		{
			showit.insert(m_CatSellPercentLabel);
			showit.insert(m_CatSellSlider);
			m_CatSellSlider->setToolTip(tr("<html>Sell commodity category amount, as a percentage of gross hold space. "
						"Specifies a maximum amount to sell. "
						"Will not sell hydrogen or electricity when selling a category of cargo.</html>"));
		}
		else
		{
			showit.insert(m_CatSellAmountEdit);
			showit.insert(m_CatSellAmountLabel);
		}
		break;
	case AuMissionOrder::CargoBuyCommodity:
		showit.insert(m_CommoditiesBuy);
		showit.insert(m_BuyCargoCountLabel);

		if(m_CIDBuy == CID_Invalid)
			contentValid = false;

		showit.insert(m_AmountBuyActualRadio);
		showit.insert(m_AmountBuyLabel);
		showit.insert(m_AmountBuyPercentRadio);
		showit.insert(m_MarginBuyActualRadio);
		showit.insert(m_MarginBuyAnyPriceRadio);
		showit.insert(m_MarginBuyLabel);
		showit.insert(m_MarginBuyPercentRadio);

		if(m_MarginBuyType == 0)
		{
			showit.insert(m_BuyMarginSlider);
			showit.insert(m_BuyMarginLabel);
		}
		else if(m_MarginBuyType == 1)
		{
			showit.insert(m_BuyCargoCostEdit);
			showit.insert(m_BuyCargoCostLabel);
		}

		m_AmountBuyPercentRadio->setText(tr("Percent of Hold Space"));
		m_AmountBuyPercentRadio->setToolTip(tr("<html>Manifest quantity is specified as a percentage of the total hold space. "
					"The officer will attempt to purchase commodities until the percentage of hold space is filled."
					"</html>"));
		m_AmountBuyActualRadio->setToolTip(tr("<html>Manifest quantity is specified as an actual number of units. "
					"The officer will attempt to purchase commodities until the purchase quantity is filled."
					"</html>"));

		if(m_AmountBuyType == 0)
		{
			showit.insert(m_BuyCargoLabel);
			showit.insert(m_BuyCargoSlider);
			m_BuyCargoSlider->setToolTip(tr("<html>Buy commodity amount, as a percentage of gross hold space, "
						"fuel cells when buying hydrogen, and capacitor when buying electricity. "
						"Specifies a limit on space allocated to the commodity; does not specify an amount to buy.</html>"));
		}
		else
		{
			showit.insert(m_BuyCargoAmountEdit);
			showit.insert(m_BuyCargoAmountLabel);
		}
		break;
	case AuMissionOrder::CargoSellCommodity:
		showit.insert(m_CommoditiesBuy);
		showit.insert(m_SellCargoCountLabel);

		if(m_CIDBuy == CID_Invalid)
			contentValid = false;

		showit.insert(m_AmountSellActualRadio);
		showit.insert(m_AmountSellLabel);
		showit.insert(m_AmountSellPercentRadio);
		showit.insert(m_MarginSellActualRadio);
		showit.insert(m_MarginSellAnyPriceRadio);
		showit.insert(m_MarginSellLabel);
		showit.insert(m_MarginSellPercentRadio);

		if(m_MarginSellType == 0)
		{
			showit.insert(m_SellMarginSlider);
			showit.insert(m_SellMarginLabel);
		}
		else if(m_MarginSellType == 1)
		{
			showit.insert(m_SellCargoCostEdit);
			showit.insert(m_SellCargoCostLabel);
		}

		m_AmountSellPercentRadio->setText(tr("Percent of Hold Space"));
		m_AmountSellPercentRadio->setToolTip(tr("<html>Sell quantity is specified as a percentage of the total hold space. "
					"The officer will attempt to sell up to this number of commodities."
					"</html>"));
		m_AmountSellActualRadio->setToolTip(tr("<html>Sell quantity is specified as an actual number of units. "
					"The officer will attempt to sell up to this number of commodities."
					"</html>"));

		if(m_AmountSellType == 0)
		{
			showit.insert(m_SellCargoLabel);
			showit.insert(m_SellCargoSlider);
			m_SellCargoSlider->setToolTip(tr("<html>Sell commodity amount, as a percentage of gross hold space, "
						"fuel cells when selling hydrogen, and capacitor when selling electricity. "
						"Specifies a maximum amount to sell.</html>"));
		}
		else
		{
			showit.insert(m_SellCargoAmountEdit);
			showit.insert(m_SellCargoAmountLabel);
		}
		break;
	case AuMissionOrder::GoToCity:
		showit.insert(m_Airports);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_AirportId.IsNull())
			contentValid = false;	// must select an airport
		break;

	case AuMissionOrder::GoToCityAnywhere:
	case AuMissionOrder::GoToOrbitCity:
		showit.insert(m_Cities);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_CityId.IsNull())
			contentValid = false;	// must select a city
		break;
	case AuMissionOrder::GoToStation:
		showit.insert(m_Stations);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_StationId.IsNull())
			contentValid = false;	// must select a city
		break;

	case AuMissionOrder::GoToSystemDirect:
	case AuMissionOrder::GoToSystemDirect_Deprecated:
		showit.insert(m_ShowAll);
		showit.insert(m_ShowNamed);
		showit.insert(m_Systems);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_SystemId.IsNull())
			contentValid = false;	// must select a solar system
		break;
	case AuMissionOrder::GoToSystemViaWormholes:
		showit.insert(m_ShowAll);
		showit.insert(m_ShowNamed);
		showit.insert(m_Routes);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_RouteSystemId.IsNull())
			contentValid = false;	// must select a solar system
		break;
	case AuMissionOrder::TroopsLoad:
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of troops to load. Zero loads all possible troops.</html>"));
		showit.insert(m_MinutesPreboardLabel);
		showit.insert(m_MinutesPreboardSpin);
		showit.insert(m_DurPreboardLabel);
		showit.insert(m_TroopCountLabel);

		if(m_PreboardTime < 0 || m_PreboardTime > 0xffff)
			contentValid = false;
		break;
	case AuMissionOrder::CrewLoad:
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of crewmen to load. Zero loads all possible crewmen.</html>"));
		showit.insert(m_MinutesPreboardLabel);
		showit.insert(m_MinutesPreboardSpin);
		showit.insert(m_DurPreboardLabel);
		showit.insert(m_CrewCountLabel);

		if(m_PreboardTime < 0 || m_PreboardTime > 0xffff)
			contentValid = false;
		break;
	case AuMissionOrder::PassengersLoad:
		showit.insert(m_PassengerLoadCountLabel);
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of passengers to load. Zero loads all possible passengers.</html>"));
		showit.insert(m_MinutesPreboardLabel);
		showit.insert(m_MinutesPreboardSpin);
		showit.insert(m_DurPreboardLabel);

		if(m_PreboardTime < 0 || m_PreboardTime > 0xffff)
			contentValid = false;
		break;
	case AuMissionOrder::PassengersUnload:
		showit.insert(m_PassengerUnloadCountLabel);
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of passengers to unload. Zero unloads all possible passengers.</html>"));
		break;
	case AuMissionOrder::CryoBerthsLoad:
		showit.insert(m_CryoBerthsLoadCountLabel);
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of cryo travellers to load. Zero loads all possible cryo travellers.</html>"));
		break;
	case AuMissionOrder::CryoBerthsUnload:
		showit.insert(m_CryoBerthsUnloadCountLabel);
		showit.insert(m_CountSpin);
		m_CountSpin->setToolTip(tr("<html>Number of cryo travellers to unload. Zero unloads all possible cryo travellers.</html>"));
		break;
	case AuMissionOrder::PatrolCities:
	case AuMissionOrder::PatrolStations:
	case AuMissionOrder::PatrolSystem:
	case AuMissionOrder::PatrolWormholes:
		showit.insert(m_DurPatrolLabel);
		showit.insert(m_MinutesLabel);
		m_MinutesLabel->setText(tr("Minutes To Patrol"));
		showit.insert(m_MinutesSpin);
		m_MinutesSpin->setToolTip(tr("<html>Patrol duration in minutes. 0 results in endless patrol.</html>"));

		if(m_Minutes < 0 || m_Minutes > 0xffff)
			contentValid = false;
		break;
	case AuMissionOrder::Attack:
	case AuMissionOrder::Plunder:
	case AuMissionOrder::Follow:
		showit.insert(m_Targets);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.

		if(m_TargetId.IsNull())
			contentValid = false;	// must select an atom to follow
		break;

	case AuMissionOrder::Posture:
		showit.insert(m_Postures);
		break;

	case AuMissionOrder::DamageReport:
	case AuMissionOrder::ScanSurveyPlanet:
	case AuMissionOrder::ScanSurveySystem:
	case AuMissionOrder::ScanTactical:
		showit.insert(m_MailCoC);
		break;

	case AuMissionOrder::GoToHoverSite:
		showit.insert(m_PlanetsAndStars);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		showit.insert(m_CoordsLabel);
		showit.insert(m_CoordsEdit);
		showit.insert(m_HoverAltLabel);
		showit.insert(m_MetersLabel);
		showit.insert(m_HoverAltSpin);

		// Test the long lat string.
		if(m_HoverAlt <= 0 || m_HoverAlt > 0xffff || !IsLongLatStringValid())
			contentValid = false;
		break;

	case AuMissionOrder::GoToLandingSite:
	case AuMissionOrder::GoToOrbitCoords:
		showit.insert(m_PlanetsAndStars);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		showit.insert(m_CoordsLabel);
		showit.insert(m_CoordsEdit);

		// Test the long lat string.
		if(!IsLongLatStringValid())
			contentValid = false;
		break;

	case AuMissionOrder::GoToLand:
	case AuMissionOrder::GoToOrbit:
		showit.insert(m_PlanetsAndStars);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;
	case AuMissionOrder::HarvestAtmosphere:
		showit.insert(m_Bays);
		showit.insert(m_CommoditiesAtmo);
		showit.insert(m_PlanetsAndStars);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;

	case AuMissionOrder::HarvestMinerals:
		showit.insert(m_Bays);
		showit.insert(m_CommoditiesMin);
		showit.insert(m_PlanetsMin);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;

	case AuMissionOrder::HarvestSea:
		showit.insert(m_Bays);
		showit.insert(m_CommoditiesSea);
		showit.insert(m_PlanetsSea);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;

	case AuMissionOrder::HarvestWell:
		showit.insert(m_Bays);
		showit.insert(m_CommoditiesWell);
		showit.insert(m_PlanetsWell);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;

	case AuMissionOrder::HarvestParticles:
		showit.insert(m_Bays);
		showit.insert(m_CommoditiesPart);
		showit.insert(m_PlanetsPart);
		showit.remove(m_Smashie);		// Smashie not needed. In fact, if left visible it wants half the space.
		break;
	}

	// Hide controls, then show them.
	SetControlVisibility(showit, false);
	SetControlVisibility(showit, true);

	m_Stack->setCurrentIndex(pageIndex);

	if(contentValid != m_ContentValid)
	{
		m_ContentValid = contentValid;
		emit ValidityChanged();
	}

	m_MinQSpin->setEnabled(m_ApplyMinQRadio->isChecked());
}

void AuMissionOrderWidget::SetControlVisibility(const QSet<QWidget*>& visibleControls, bool show)
{
	//qDebug() << "AuMissionOrderWidget::SetControlVisibility" << visibleControls.size() << (show?"show visible":"hide invisible");
	if(visibleControls.contains(m_Airports) == show)
		m_Airports->setVisible(show);

	if(visibleControls.contains(m_AmountBuyActualRadio) == show)
		m_AmountBuyActualRadio->setVisible(show);
	if(visibleControls.contains(m_AmountBuyLabel) == show)
		m_AmountBuyLabel->setVisible(show);
	if(visibleControls.contains(m_AmountBuyPercentRadio) == show)
		m_AmountBuyPercentRadio->setVisible(show);
	if(visibleControls.contains(m_AmountSellActualRadio) == show)
		m_AmountSellActualRadio->setVisible(show);
	if(visibleControls.contains(m_AmountSellLabel) == show)
		m_AmountSellLabel->setVisible(show);
	if(visibleControls.contains(m_AmountSellPercentRadio) == show)
		m_AmountSellPercentRadio->setVisible(show);
	if(visibleControls.contains(m_AmountTransferFromLabel) == show)
		m_AmountTransferFromLabel->setVisible(show);
	if(visibleControls.contains(m_AmountTransferToLabel) == show)
		m_AmountTransferToLabel->setVisible(show);

	if(visibleControls.contains(m_Bays) == show)
		m_Bays->setVisible(show);

	if(visibleControls.contains(m_BuyCargoAmountEdit) == show)
		m_BuyCargoAmountEdit->setVisible(show);
	if(visibleControls.contains(m_BuyCargoAmountLabel) == show)
		m_BuyCargoAmountLabel->setVisible(show);
	if(visibleControls.contains(m_BuyCargoCostEdit) == show)
		m_BuyCargoCostEdit->setVisible(show);
	if(visibleControls.contains(m_BuyCargoCostLabel) == show)
		m_BuyCargoCostLabel->setVisible(show);
	if(visibleControls.contains(m_BuyCargoCountLabel) == show)
		m_BuyCargoCountLabel->setVisible(show);
	if(visibleControls.contains(m_BuyCargoLabel) == show)
		m_BuyCargoLabel->setVisible(show);
	if(visibleControls.contains(m_BuyCargoSlider) == show)
		m_BuyCargoSlider->setVisible(show);

	if(visibleControls.contains(m_BuyMarginLabel) == show)
		m_BuyMarginLabel->setVisible(show);
	if(visibleControls.contains(m_BuyMarginSlider) == show)
		m_BuyMarginSlider->setVisible(show);

	if(visibleControls.contains(m_CatBuyActualRadio) == show)
		m_CatBuyActualRadio->setVisible(show);
	if(visibleControls.contains(m_CatBuyAmountEdit) == show)
		m_CatBuyAmountEdit->setVisible(show);
	if(visibleControls.contains(m_CatBuyAmountLabel) == show)
		m_CatBuyAmountLabel->setVisible(show);
	if(visibleControls.contains(m_CatBuyEachRadio) == show)
		m_CatBuyEachRadio->setVisible(show);
	if(visibleControls.contains(m_CatBuyLabel) == show)
		m_CatBuyLabel->setVisible(show);
	if(visibleControls.contains(m_CatBuyPercentLabel) == show)
		m_CatBuyPercentLabel->setVisible(show);
	if(visibleControls.contains(m_CatBuyPercentRadio) == show)
		m_CatBuyPercentRadio->setVisible(show);
	if(visibleControls.contains(m_CatBuySlider) == show)
		m_CatBuySlider->setVisible(show);

	if(visibleControls.contains(m_CatSellActualRadio) == show)
		m_CatSellActualRadio->setVisible(show);
	if(visibleControls.contains(m_CatSellAmountEdit) == show)
		m_CatSellAmountEdit->setVisible(show);
	if(visibleControls.contains(m_CatSellAmountLabel) == show)
		m_CatSellAmountLabel->setVisible(show);
	if(visibleControls.contains(m_CatSellEachRadio) == show)
		m_CatSellEachRadio->setVisible(show);
	if(visibleControls.contains(m_CatSellLabel) == show)
		m_CatSellLabel->setVisible(show);
	if(visibleControls.contains(m_CatSellPercentLabel) == show)
		m_CatSellPercentLabel->setVisible(show);
	if(visibleControls.contains(m_CatSellPercentRadio) == show)
		m_CatSellPercentRadio->setVisible(show);
	if(visibleControls.contains(m_CatSellSlider) == show)
		m_CatSellSlider->setVisible(show);

	if(visibleControls.contains(m_CatTransferFromAmountLabel) == show)
		m_CatTransferFromAmountLabel->setVisible(show);
	if(visibleControls.contains(m_CatTransferFromLabel) == show)
		m_CatTransferFromLabel->setVisible(show);
	if(visibleControls.contains(m_CatTransferFromPercentLabel) == show)
		m_CatTransferFromPercentLabel->setVisible(show);
	if(visibleControls.contains(m_CatTransferToAmountLabel) == show)
		m_CatTransferToAmountLabel->setVisible(show);
	if(visibleControls.contains(m_CatTransferToLabel) == show)
		m_CatTransferToLabel->setVisible(show);
	if(visibleControls.contains(m_CatTransferToPercentLabel) == show)
		m_CatTransferToPercentLabel->setVisible(show);

	if(visibleControls.contains(m_Cities) == show)
		m_Cities->setVisible(show);

	if(visibleControls.contains(m_CommoditiesTransfer) == show)
		m_CommoditiesTransfer->setVisible(show);
	if(visibleControls.contains(m_CommoditiesAtmo) == show)
		m_CommoditiesAtmo->setVisible(show);
	if(visibleControls.contains(m_CommoditiesBuy) == show)
		m_CommoditiesBuy->setVisible(show);
	if(visibleControls.contains(m_CommoditiesMin) == show)
		m_CommoditiesMin->setVisible(show);
	if(visibleControls.contains(m_CommoditiesPart) == show)
		m_CommoditiesPart->setVisible(show);
	if(visibleControls.contains(m_CommoditiesSea) == show)
		m_CommoditiesSea->setVisible(show);
	if(visibleControls.contains(m_CommoditiesWell) == show)
		m_CommoditiesWell->setVisible(show);
	if(visibleControls.contains(m_CommodityFilter) == show)
		m_CommodityFilter->setVisible(show);

	if(visibleControls.contains(m_CoordsEdit) == show)
		m_CoordsEdit->setVisible(show);
	if(visibleControls.contains(m_CoordsLabel) == show)
		m_CoordsLabel->setVisible(show);

	if(visibleControls.contains(m_CountSpin) == show)
		m_CountSpin->setVisible(show);

	if(visibleControls.contains(m_CrewCountLabel) == show)
		m_CrewCountLabel->setVisible(show);

	if(visibleControls.contains(m_CryoBerthsLoadCountLabel) == show)
		m_CryoBerthsLoadCountLabel->setVisible(show);
	if(visibleControls.contains(m_CryoBerthsUnloadCountLabel) == show)
		m_CryoBerthsUnloadCountLabel->setVisible(show);

	if(visibleControls.contains(m_DoOnce) == show)
		m_DoOnce->setVisible(show);

	if(visibleControls.contains(m_DurPatrolLabel) == show)
		m_DurPatrolLabel->setVisible(show);
	if(visibleControls.contains(m_DurPreboardLabel) == show)
		m_DurPreboardLabel->setVisible(show);
	if(visibleControls.contains(m_DurWaitLabel) == show)
		m_DurWaitLabel->setVisible(show);

	if(visibleControls.contains(m_GoToCoord) == show)
		m_GoToCoord->setVisible(show);
	if(visibleControls.contains(m_GoToCoordLabel) == show)
		m_GoToCoordLabel->setVisible(show);
	if(visibleControls.contains(m_GoToGalaxy) == show)
		m_GoToGalaxy->setVisible(show);
	if(visibleControls.contains(m_GoToSector) == show)
		m_GoToSector->setVisible(show);
	if(visibleControls.contains(m_GoToSystem) == show)
		m_GoToSystem->setVisible(show);

	if(visibleControls.contains(m_ApplyShipQRadio) == show)
		m_ApplyShipQRadio->setVisible(show);
	if(visibleControls.contains(m_ApplyMinQRadio) == show)
		m_ApplyMinQRadio->setVisible(show);
	if(visibleControls.contains(m_ApplySystemQRadio) == show)
		m_ApplySystemQRadio->setVisible(show);
	if(visibleControls.contains(m_MinQLabel) == show)
		m_MinQLabel->setVisible(show);
	if(visibleControls.contains(m_MinQSpin) == show)
		m_MinQSpin->setVisible(show);

	if(visibleControls.contains(m_HoverAltLabel) == show)
		m_HoverAltLabel->setVisible(show);
	if(visibleControls.contains(m_HoverAltSpin) == show)
		m_HoverAltSpin->setVisible(show);

	if(visibleControls.contains(m_LightHeadlightLabel) == show)
		m_LightHeadlightLabel->setVisible(show);
	if(visibleControls.contains(m_LightHeadlightOff) == show)
		m_LightHeadlightOff->setVisible(show);
	if(visibleControls.contains(m_LightHeadlightOn) == show)
		m_LightHeadlightOn->setVisible(show);
	if(visibleControls.contains(m_LightNavLightLabel) == show)
		m_LightNavLightLabel->setVisible(show);
	if(visibleControls.contains(m_LightNavLightOff) == show)
		m_LightNavLightOff->setVisible(show);
	if(visibleControls.contains(m_LightNavLightOn) == show)
		m_LightNavLightOn->setVisible(show);
	if(visibleControls.contains(m_LightRotatingBeaconLabel) == show)
		m_LightRotatingBeaconLabel->setVisible(show);
	if(visibleControls.contains(m_LightRotatingBeaconOff) == show)
		m_LightRotatingBeaconOff->setVisible(show);
	if(visibleControls.contains(m_LightRotatingBeaconOn) == show)
		m_LightRotatingBeaconOn->setVisible(show);
	if(visibleControls.contains(m_LightStrobeLabel) == show)
		m_LightStrobeLabel->setVisible(show);
	if(visibleControls.contains(m_LightStrobeOff) == show)
		m_LightStrobeOff->setVisible(show);
	if(visibleControls.contains(m_LightStrobeOn) == show)
		m_LightStrobeOn->setVisible(show);

	if(visibleControls.contains(m_MailCoC) == show)
		m_MailCoC->setVisible(show);

	if(visibleControls.contains(m_MarginBuyActualRadio) == show)
		m_MarginBuyActualRadio->setVisible(show);
	if(visibleControls.contains(m_MarginBuyAnyPriceRadio) == show)
		m_MarginBuyAnyPriceRadio->setVisible(show);
	if(visibleControls.contains(m_MarginBuyLabel) == show)
		m_MarginBuyLabel->setVisible(show);
	if(visibleControls.contains(m_MarginBuyPercentRadio) == show)
		m_MarginBuyPercentRadio->setVisible(show);
	if(visibleControls.contains(m_MarginSellActualRadio) == show)
		m_MarginSellActualRadio->setVisible(show);
	if(visibleControls.contains(m_MarginSellAnyPriceRadio) == show)
		m_MarginSellAnyPriceRadio->setVisible(show);
	if(visibleControls.contains(m_MarginSellLabel) == show)
		m_MarginSellLabel->setVisible(show);
	if(visibleControls.contains(m_MarginSellPercentRadio) == show)
		m_MarginSellPercentRadio->setVisible(show);

	if(visibleControls.contains(m_MetersLabel) == show)
		m_MetersLabel->setVisible(show);

	if(visibleControls.contains(m_MinutesLabel) == show)
		m_MinutesLabel->setVisible(show);
	if(visibleControls.contains(m_MinutesPreboardLabel) == show)
		m_MinutesPreboardLabel->setVisible(show);
	if(visibleControls.contains(m_MinutesPreboardSpin) == show)
		m_MinutesPreboardSpin->setVisible(show);
	if(visibleControls.contains(m_MinutesSpin) == show)
		m_MinutesSpin->setVisible(show);

	if(visibleControls.contains(m_PassengerLoadCountLabel) == show)
		m_PassengerLoadCountLabel->setVisible(show);
	if(visibleControls.contains(m_PassengerUnloadCountLabel) == show)
		m_PassengerUnloadCountLabel->setVisible(show);

	if(visibleControls.contains(m_PilotUpPolicies) == show)
		m_PilotUpPolicies->setVisible(show);

	if(visibleControls.contains(m_PlanetsAndStars) == show)
		m_PlanetsAndStars->setVisible(show);

	if(visibleControls.contains(m_PlanetsMin) == show)
		m_PlanetsMin->setVisible(show);
	if(visibleControls.contains(m_PlanetsPart) == show)
		m_PlanetsPart->setVisible(show);
	if(visibleControls.contains(m_PlanetsSea) == show)
		m_PlanetsSea->setVisible(show);
	if(visibleControls.contains(m_PlanetsWell) == show)
		m_PlanetsWell->setVisible(show);

	if(visibleControls.contains(m_Postures) == show)
		m_Postures->setVisible(show);

	if(visibleControls.contains(m_Routes) == show)
		m_Routes->setVisible(show);

	if(visibleControls.contains(m_SellCargoAmountEdit) == show)
		m_SellCargoAmountEdit->setVisible(show);
	if(visibleControls.contains(m_SellCargoAmountLabel) == show)
		m_SellCargoAmountLabel->setVisible(show);
	if(visibleControls.contains(m_SellCargoCostEdit) == show)
		m_SellCargoCostEdit->setVisible(show);
	if(visibleControls.contains(m_SellCargoCostLabel) == show)
		m_SellCargoCostLabel->setVisible(show);
	if(visibleControls.contains(m_SellCargoCountLabel) == show)
		m_SellCargoCountLabel->setVisible(show);
	if(visibleControls.contains(m_SellCargoLabel) == show)
		m_SellCargoLabel->setVisible(show);
	if(visibleControls.contains(m_SellCargoSlider) == show)
		m_SellCargoSlider->setVisible(show);
	if(visibleControls.contains(m_SellMarginLabel) == show)
		m_SellMarginLabel->setVisible(show);
	if(visibleControls.contains(m_SellMarginSlider) == show)
		m_SellMarginSlider->setVisible(show);

	if(visibleControls.contains(m_ShieldFuelPolicies) == show)
		m_ShieldFuelPolicies->setVisible(show);
	if(visibleControls.contains(m_ShieldPolicies) == show)
		m_ShieldPolicies->setVisible(show);

	if(visibleControls.contains(m_ShipNameEdit) == show)
		m_ShipNameEdit->setVisible(show);
	if(visibleControls.contains(m_ShipNameLabel) == show)
		m_ShipNameLabel->setVisible(show);

	if(visibleControls.contains(m_ShowAll) == show)
		m_ShowAll->setVisible(show);
	if(visibleControls.contains(m_ShowNamed) == show)
		m_ShowNamed->setVisible(show);

	if(visibleControls.contains(m_Smashie) == show)
		m_Smashie->setVisible(show);

	if(visibleControls.contains(m_Stations) == show)
		m_Stations->setVisible(show);

	if(visibleControls.contains(m_Systems) == show)
		m_Systems->setVisible(show);

	if(visibleControls.contains(m_Targets) == show)
		m_Targets->setVisible(show);

	if(visibleControls.contains(m_TransferAuthorizationLabel) == show)
		m_TransferAuthorizationLabel->setVisible(show);
	if(visibleControls.contains(m_TransferCargoCountLabel) == show)
		m_TransferCargoCountLabel->setVisible(show);
	if(visibleControls.contains(m_TransferCargoFromAmountLabel) == show)
		m_TransferCargoFromAmountLabel->setVisible(show);
	if(visibleControls.contains(m_TransferCargoFromLabel) == show)
		m_TransferCargoFromLabel->setVisible(show);
	if(visibleControls.contains(m_TransferCargoToAmountLabel) == show)
		m_TransferCargoToAmountLabel->setVisible(show);
	if(visibleControls.contains(m_TransferCargoToLabel) == show)
		m_TransferCargoToLabel->setVisible(show);
	if(visibleControls.contains(m_TransfereeEdit) == show)
		m_TransfereeEdit->setVisible(show);
	if(visibleControls.contains(m_TransferFromLabel) == show)
		m_TransferFromLabel->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromAmountEdit) == show)
		m_TransferMoneyFromAmountEdit->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromAmountRadio) == show)
		m_TransferMoneyFromAmountRadio->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromLabel) == show)
		m_TransferMoneyFromLabel->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromPercentLabel) == show)
		m_TransferMoneyFromPercentLabel->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromPercentRadio) == show)
		m_TransferMoneyFromPercentRadio->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyFromPercentSlider) == show)
		m_TransferMoneyFromPercentSlider->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToAmountEdit) == show)
		m_TransferMoneyToAmountEdit->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToAmountRadio) == show)
		m_TransferMoneyToAmountRadio->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToLabel) == show)
		m_TransferMoneyToLabel->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToPercentLabel) == show)
		m_TransferMoneyToPercentLabel->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToPercentRadio) == show)
		m_TransferMoneyToPercentRadio->setVisible(show);
	if(visibleControls.contains(m_TransferMoneyToPercentSlider) == show)
		m_TransferMoneyToPercentSlider->setVisible(show);
	if(visibleControls.contains(m_TransferNamedShip) == show)
		m_TransferNamedShip->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromAmountEdit) == show)
		m_TransferPeopleFromAmountEdit->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromAmountRadio) == show)
		m_TransferPeopleFromAmountRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromBerthLabel) == show)
		m_TransferPeopleFromBerthLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromCrewRadio) == show)
		m_TransferPeopleFromCrewRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromLabel) == show)
		m_TransferPeopleFromLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromOfficersRadio) == show)
		m_TransferPeopleFromOfficersRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromPassengersRadio) == show)
		m_TransferPeopleFromPassengersRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromPercentLabel) == show)
		m_TransferPeopleFromPercentLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromPercentRadio) == show)
		m_TransferPeopleFromPercentRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromPercentSlider) == show)
		m_TransferPeopleFromPercentSlider->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleFromTroopsRadio) == show)
		m_TransferPeopleFromTroopsRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToAmountEdit) == show)
		m_TransferPeopleToAmountEdit->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToAmountRadio) == show)
		m_TransferPeopleToAmountRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToBerthLabel) == show)
		m_TransferPeopleToBerthLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToCrewRadio) == show)
		m_TransferPeopleToCrewRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToLabel) == show)
		m_TransferPeopleToLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToOfficersRadio) == show)
		m_TransferPeopleToOfficersRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToPassengersRadio) == show)
		m_TransferPeopleToPassengersRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToPercentLabel) == show)
		m_TransferPeopleToPercentLabel->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToPercentRadio) == show)
		m_TransferPeopleToPercentRadio->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToPercentSlider) == show)
		m_TransferPeopleToPercentSlider->setVisible(show);
	if(visibleControls.contains(m_TransferPeopleToTroopsRadio) == show)
		m_TransferPeopleToTroopsRadio->setVisible(show);
	if(visibleControls.contains(m_TransferStation) == show)
		m_TransferStation->setVisible(show);
	if(visibleControls.contains(m_TransferToLabel) == show)
		m_TransferToLabel->setVisible(show);

	if(visibleControls.contains(m_TroopCountLabel) == show)
		m_TroopCountLabel->setVisible(show);
}

AuMissionOrder* AuMissionOrderWidget::Order() const
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	// Overridden by harvester operations, which have their own control for do once.
	AuMissionOrder::EXECTYPE execType = m_DoOnce->checkState() == Qt::Checked ? AuMissionOrder::DoOnce : AuMissionOrder::Sequential;

	switch(m_Order)
	{
	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
		break;

	case AuMissionOrder::HarvestOperationByBay:
		{
			AuMissionOrder::EXECTYPE execType =
				m_HarvestDoOnce->checkState() == Qt::Checked ? AuMissionOrder::DoOnce : AuMissionOrder::Sequential;
			AuHarvestOps ops;
			HarvestOps(ops);
			return new AuMissionOrderHarvestOperationByBay(m_HarvestPlanetId,
					m_HarvestPlanetName,
					m_HarvestFuelThreshold,
					ops,
					execType);
		}

	case AuMissionOrder::HarvestOperationByModule:
		{
			AuMissionOrder::EXECTYPE execType =
				m_HarvestDoOnceByM->checkState() == Qt::Checked ? AuMissionOrder::DoOnce : AuMissionOrder::Sequential;
			AuHarvestOps ops;
			HarvestOps(ops);
			return new AuMissionOrderHarvestOperationByModule(m_HarvestPlanetIdByM,
					m_HarvestPlanetNameByM,
					m_HarvestFuelThresholdByM,
					ops,
					execType);
		}

	case AuMissionOrder::Report:
		if(!m_ReportChannel.isEmpty())
		{
			AuMissionOrder::EXECTYPE execType =
				m_ReportDoOnce->checkState() == Qt::Checked ? AuMissionOrder::DoOnce : AuMissionOrder::Sequential;

			quint16 reportWhat = 0;
			if(m_ReportPosition->isChecked())
				reportWhat |= 0x0001;
			if(m_ReportCrew->isChecked())
				reportWhat |= 0x0002;
			if(m_ReportFuel->isChecked())
				reportWhat |= 0x0004;
			if(m_ReportCargo->isChecked())
				reportWhat |= 0x0008;
			if(m_ReportDamage->isChecked())
				reportWhat |= 0x0010;
			if(m_ReportSensor->isChecked())
				reportWhat |= 0x0020;

			QString reportSay = m_ReportSayEdit->text().trimmed();

			AuCommStdChannel::RANGE range = m_ReportRange;
			if(AuCommStdChannel::IsHailChannel(m_ReportChannel) || AuCommStdChannel::IsTradeChannel(m_ReportChannel))
			{
				if(range > AuCommStdChannel::SolarSystem)
					range = AuCommStdChannel::SolarSystem;
			}
			else if(!AuCommStdChannel::ChannelHasRange(m_ReportChannel))
				range = AuCommStdChannel::DefaultRange(m_ReportChannel);

			return new AuMissionOrderReport(reportWhat, reportSay, m_ReportChannel, range, execType);
		}
		break;

	case AuMissionOrder::Attack:
		return new AuMissionOrderAttack(m_TargetId, m_TargetName, execType);

	case AuMissionOrder::TransferCargoCategoryFrom:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint32 amount;
			if(m_CatBuyType == 0)
				amount = m_AmountBuyPercent;
			else
				amount = m_AmountBuyUnits;

			return new AuMissionOrderTransferCargoCategoryFrom(transferee, m_Authorization, m_Category, amount, m_CatBuyType,
					execType);
		}

	case AuMissionOrder::TransferCargoCategoryTo:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint32 amount;
			if(m_CatSellType == 0)
				amount = m_AmountSellPercent;
			else
				amount = m_AmountSellUnits;

			return new AuMissionOrderTransferCargoCategoryTo(transferee, m_Authorization, m_Category, amount, m_CatSellType,
					execType);
		}

	case AuMissionOrder::TransferCargoFrom:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint32 amount;
			if(m_AmountBuyType == 0)
				amount = m_AmountBuyPercent;
			else
				amount = m_AmountBuyUnits;

			return new AuMissionOrderTransferCargoFrom(transferee, m_Authorization, m_CIDTransfer, amount, m_AmountBuyType, execType);
		}

	case AuMissionOrder::TransferCargoTo:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint32 amount;
			if(m_AmountSellType == 0)
				amount = m_AmountSellPercent;
			else
				amount = m_AmountSellUnits;

			return new AuMissionOrderTransferCargoTo(transferee, m_Authorization, m_CIDTransfer, amount, m_AmountSellType, execType);
		}

	case AuMissionOrder::TransferMoneyFrom:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint8 amountType;
			double amount;
			if(m_TransferMoneyFromPercentRadio->isChecked())
			{
				amount = m_TransferMoneyFromPercentSlider->value();
				amountType = 0;	// percent
			}
			else
			{
				amount = QVariant(m_TransferMoneyFromAmountEdit->text()).toDouble();
				amountType = 1;	// amount
			}
			if(amount <= 0)
				break;

			return new AuMissionOrderTransferMoneyFrom(transferee, m_Authorization, amount, amountType, execType);
		}

	case AuMissionOrder::TransferMoneyTo:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint8 amountType;
			double amount;
			if(m_TransferMoneyToPercentRadio->isChecked())
			{
				amount = m_TransferMoneyToPercentSlider->value();
				amountType = 0;	// percent
			}
			else
			{
				amount = QVariant(m_TransferMoneyToAmountEdit->text()).toDouble();
				amountType = 1;	// amount
			}
			if(amount <= 0)
				break;

			return new AuMissionOrderTransferMoneyTo(transferee, m_Authorization, amount, amountType, execType);
		}

	case AuMissionOrder::TransferPeopleFrom:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint8 berthType;
			if(m_TransferPeopleFromOfficersRadio->isChecked())
				berthType = 0;
			else if(m_TransferPeopleFromCrewRadio->isChecked())
				berthType = 1;
			else if(m_TransferPeopleFromTroopsRadio->isChecked())
				berthType = 2;
			else if(m_TransferPeopleFromPassengersRadio->isChecked())
				berthType = 3;
			else
				break;

			quint8 amountType;
			quint16 amount;
			if(m_TransferPeopleFromPercentRadio->isChecked())
			{
				amount = m_TransferPeopleFromPercentSlider->value();
				amountType = 0;	// percent
			}
			else
			{
				amount = QVariant(m_TransferPeopleFromAmountEdit->text()).toUInt();
				amountType = 1;	// amount
			}
			if(amount <= 0)
				break;

			return new AuMissionOrderTransferPeopleFrom(transferee, m_Authorization, berthType, amount, amountType, execType);
		}

	case AuMissionOrder::TransferPeopleTo:
		{
			QString transferee;				// blank transferee implies station to the order
			if(m_TransferTarget)
				transferee = m_Transferee;

			quint8 berthType;
			if(m_TransferPeopleToOfficersRadio->isChecked())
				berthType = 0;
			else if(m_TransferPeopleToCrewRadio->isChecked())
				berthType = 1;
			else if(m_TransferPeopleToTroopsRadio->isChecked())
				berthType = 2;
			else if(m_TransferPeopleToPassengersRadio->isChecked())
				berthType = 3;
			else
				break;

			quint8 amountType;
			quint16 amount;
			if(m_TransferPeopleToPercentRadio->isChecked())
			{
				amount = m_TransferPeopleToPercentSlider->value();
				amountType = 0;	// percent
			}
			else
			{
				amount = QVariant(m_TransferPeopleToAmountEdit->text()).toUInt();
				amountType = 1;	// amount
			}
			if(amount <= 0)
				break;

			return new AuMissionOrderTransferPeopleTo(transferee, m_Authorization, berthType, amount, amountType, execType);
		}

	case AuMissionOrder::CargoBuyCategory:
		{
			quint32 amount;
			if(m_CatBuyType == 0)
				amount = m_AmountBuyPercent;
			else
				amount = m_AmountBuyUnits;

			quint32 margin;
			if(m_MarginBuyType == 0)
				margin = m_MarginBuyPercent;
			else if(m_MarginBuyType == 1)
				margin = m_MarginBuyCost;
			else
				margin = 0;

			return new AuMissionOrderCargoBuyCategory(m_Category, amount, m_CatBuyType, margin, m_MarginBuyType, execType);
		}

	case AuMissionOrder::CargoBuyCommodity:
		{
			quint32 amount;
			if(m_AmountBuyType == 0)
				amount = m_AmountBuyPercent;
			else
				amount = m_AmountBuyUnits;

			quint32 margin;
			if(m_MarginBuyType == 0)
				margin = m_MarginBuyPercent;
			else if(m_MarginBuyType == 1)
				margin = m_MarginBuyCost;
			else
				margin = 0;

			return new AuMissionOrderCargoBuyCommodity(m_CIDBuy, amount, m_AmountBuyType, margin, m_MarginBuyType, execType);
		}

	case AuMissionOrder::ExterminateRats:
		return new AuMissionOrderExterminateRats(execType);

	case AuMissionOrder::CargoSellCategory:
		{
			quint32 amount;
			if(m_CatSellType == 0)
				amount = m_AmountSellPercent;
			else
				amount = m_AmountSellUnits;

			quint32 margin;
			if(m_MarginSellType == 0)
				margin = m_MarginSellPercent;
			else if(m_MarginSellType == 1)
				margin = m_MarginSellCost;
			else
				margin = 0;

			quint8 sellType;
			if(m_CatSellType == 0)
				sellType = 0;
			else
				sellType = 1;

			return new AuMissionOrderCargoSellCategory(m_Category, amount, sellType, margin, m_MarginSellType, execType);
		}

	case AuMissionOrder::CargoSellCommodity:
		{
			quint32 amount;
			if(m_AmountSellType == 0)
				amount = m_AmountSellPercent;
			else
				amount = m_AmountSellUnits;

			quint32 margin;
			if(m_MarginSellType == 0)
				margin = m_MarginSellPercent;
			else if(m_MarginSellType == 1)
				margin = m_MarginSellCost;
			else
				margin = 0;

			return new AuMissionOrderCargoSellCommodity(m_CIDBuy, amount, m_AmountSellType, margin, m_MarginSellType, execType);
		}

	case AuMissionOrder::Christen:
		if(IsNameValid(m_ShipName))
			return new AuMissionOrderChristen(m_ShipName, execType);
		break;

	case AuMissionOrder::CrewLoad:
		if(m_PreboardTime >= 0 && m_PreboardTime <= 0xff)
			return new AuMissionOrderCrewLoad(m_Count <= 0xff ? m_Count : 0, (quint8)m_PreboardTime, execType);
		break;

	case AuMissionOrder::DamageReport:
		return new AuMissionOrderDamageReport(m_MailCoC->checkState() == Qt::Checked ? 1 : 0, execType);

	case AuMissionOrder::Follow:
		return new AuMissionOrderFollow(m_TargetId, m_TargetName, execType);

	case AuMissionOrder::GoToBestCity:
		return new AuMissionOrderGoToBestCity(execType);

	case AuMissionOrder::GoToBestStation:
		return new AuMissionOrderGoToBestStation(execType);

	case AuMissionOrder::GoToBestStationOrCity:
		return new AuMissionOrderGoToBestStationOrCity(execType);

	case AuMissionOrder::GoToCity:
		if(!m_AirportId.IsNull())
			return new AuMissionOrderGoToCity(m_AirportId, m_AirportName, execType);
		break;

	case AuMissionOrder::GoToCityAnywhere:
		if(!m_CityId.IsNull())
			return new AuMissionOrderGoToCityAnywhere(m_CityId, m_CityName, execType);
		break;

	case AuMissionOrder::GoToClosestCity:
		return new AuMissionOrderGoToClosestCity(execType);

	case AuMissionOrder::GoToClosestStation:
		return new AuMissionOrderGoToClosestStation(execType);

	case AuMissionOrder::GoToClosestStationOrCity:
		return new AuMissionOrderGoToClosestStationOrCity(execType);

	case AuMissionOrder::GoToCoordinate:
		return new AuMissionOrderGoToCoordinate((AuMissionOrderGoToCoordinate::CONTEXT)m_GoToRelative, m_GoTo, execType);

	case AuMissionOrder::GoToHoverSite:
		if(m_HoverAlt >= 1 && m_HoverAlt <= 0xffff)
		{
			AuGlobeLLF globeLL;
			if(globeLL.ParseLongitudeLatitude(m_PlanetLongLat))
				return new AuMissionOrderGoToHoverSite(m_PlanetId, m_PlanetName, globeLL, (quint16)m_HoverAlt, execType);
		}
		break;

	case AuMissionOrder::GoToLand:
		return new AuMissionOrderGoToLand(m_PlanetId, m_PlanetName, execType);

	case AuMissionOrder::GoToLandingSite:
		{
			AuGlobeLLF globeLL;
			if(globeLL.ParseLongitudeLatitude(m_PlanetLongLat))
				return new AuMissionOrderGoToLandingSite(m_PlanetId, m_PlanetName, globeLL, execType);
		}
		break;

	case AuMissionOrder::GoToOrbit:
		return new AuMissionOrderGoToOrbit(m_PlanetId, m_PlanetName, execType);

	case AuMissionOrder::GoToOrbitBestPlanet:
		return new AuMissionOrderGoToOrbitBestPlanet(execType);

	case AuMissionOrder::GoToOrbitCity:
		return new AuMissionOrderGoToOrbitCity(m_CityId, m_CityName, execType);

	case AuMissionOrder::GoToOrbitCoords:
		{
			AuGlobeLLF globeLL;
			if(globeLL.ParseLongitudeLatitude(m_PlanetLongLat))
				return new AuMissionOrderGoToOrbitCoords(m_PlanetId, m_PlanetName, globeLL, execType);
		}
		break;

	case AuMissionOrder::GoToStation:
		if(!m_StationId.IsNull())
			return new AuMissionOrderGoToStation(m_StationId, m_StationName, execType);
		break;

	case AuMissionOrder::GoToSystemDirect:
	case AuMissionOrder::GoToSystemDirect_Deprecated:
		return new AuMissionOrderGoToSystemDirect(m_SystemId, m_SystemName, m_SystemGalaxyLoc, execType);

	case AuMissionOrder::GoToSystemViaWormholes:
		return new AuMissionOrderGoToSystemViaWormholes(m_RouteSystemId, m_RouteSystemName, execType);

	case AuMissionOrder::Harvest:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderHarvest((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::HarvestAtmosphere:
		if(m_BayWhich >= 0 && m_BayWhich <= 0xff)
			return new AuMissionOrderHarvestAtmosphere(m_CIDAtmo, false, m_BayWhich, m_PlanetId, m_PlanetName, execType);
		else
			return new AuMissionOrderHarvestAtmosphere(m_CIDAtmo, true, 0, m_PlanetId, m_PlanetName, execType);

	case AuMissionOrder::HarvestMinerals:
		if(m_BayWhich >= 0 && m_BayWhich <= 0xff)
			return new AuMissionOrderHarvestMinerals(m_CIDMin, false, m_BayWhich, m_HarvestMinId, m_HarvestMinName, execType);
		else
			return new AuMissionOrderHarvestMinerals(m_CIDMin, true, 0, m_HarvestMinId, m_HarvestMinName, execType);

	case AuMissionOrder::HarvestParticles:
		if(m_BayWhich >= 0 && m_BayWhich <= 0xff)
			return new AuMissionOrderHarvestParticles(m_CIDPart, false, m_BayWhich, m_HarvestPartId, m_HarvestPartName, execType);
		else
			return new AuMissionOrderHarvestParticles(m_CIDPart, true, 0, m_HarvestPartId, m_HarvestPartName, execType);

	case AuMissionOrder::HarvestSea:
		if(m_BayWhich >= 0 && m_BayWhich <= 0xff)
			return new AuMissionOrderHarvestSea(m_CIDSea, false, m_BayWhich, m_HarvestSeaId, m_HarvestSeaName, execType);
		else
			return new AuMissionOrderHarvestSea(m_CIDSea, true, 0, m_HarvestSeaId, m_HarvestSeaName, execType);

	case AuMissionOrder::HarvestWell:
		if(m_BayWhich >= 0 && m_BayWhich <= 0xff)
			return new AuMissionOrderHarvestWell(m_CIDWell, false, m_BayWhich, m_HarvestWellId, m_HarvestWellName, execType);
		else
			return new AuMissionOrderHarvestWell(m_CIDWell, true, 0, m_HarvestWellId, m_HarvestWellName, execType);

	case AuMissionOrder::Hide:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderHide((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::HoldMission:
		return new AuMissionOrderHoldMission(execType);

	case AuMissionOrder::LightState:
		return new AuMissionOrderLightState(m_LightStateHeadlight, m_LightStateNavLight, m_LightStateRotatingBeacon,
				m_LightStateStrobe, execType);

	case AuMissionOrder::CryoBerthsLoad:
		return new AuMissionOrderCryoBerthsLoad(m_Count, execType);

	case AuMissionOrder::CryoBerthsUnload:
		return new AuMissionOrderCryoBerthsUnload(m_Count, execType);

	case AuMissionOrder::PassengersLoad:
		if(m_PreboardTime >= 0 && m_PreboardTime <= 0xff)
			return new AuMissionOrderPassengersLoad(m_Count <= 0xff ? m_Count : 0, (quint8)m_PreboardTime, execType);
		break;

	case AuMissionOrder::PassengersUnload:
		return new AuMissionOrderPassengersUnload(m_Count <= 0xff ? m_Count : 0, execType);

	case AuMissionOrder::PatrolCities:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderPatrolCities((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::PatrolStations:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderPatrolStations((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::PatrolSystem:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderPatrolSystem((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::PatrolWormholes:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderPatrolWormholes((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::Plunder:
		return new AuMissionOrderPlunder(m_TargetId, m_TargetName, execType);

	case AuMissionOrder::Posture:
		return new AuMissionOrderPosture(m_Posture, execType);

	case AuMissionOrder::Repair:
		return new AuMissionOrderRepair(m_ApplyQ, m_MinQ, execType);

	case AuMissionOrder::ScanSurveyPlanet:
		return new AuMissionOrderScanSurveyPlanet(m_MailCoC->checkState() == Qt::Checked ? 1 : 0, execType);

	case AuMissionOrder::ScanSurveySystem:
		return new AuMissionOrderScanSurveySystem(m_MailCoC->checkState() == Qt::Checked ? 1 : 0, execType);

	case AuMissionOrder::ScanTactical:
		return new AuMissionOrderScanTactical(m_MailCoC->checkState() == Qt::Checked ? 1 : 0, execType);

	case AuMissionOrder::Scuttle:
		// m_Minutes is actually seconds, not minutes.
		if(m_Minutes < 0 || m_Minutes > 0xffff)
			break;

		if(isAgent)
			return new AuMissionOrderScuttle((quint16)m_Minutes, execType);

		// Impose a 3 minute minimum.
		if(m_Minutes <= 3 * 60)
			return new AuMissionOrderScuttle(3 * 60, execType);
		else
			return new AuMissionOrderScuttle((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::PilotPolicy:
		return new AuMissionOrderPilotPolicy(m_PilotUpPolicy, execType);

	case AuMissionOrder::ShieldPolicy:
		return new AuMissionOrderShieldPolicy(m_ShieldPolicy, m_ShieldFuelPolicy, execType);

	case AuMissionOrder::ShieldsDown:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderShieldsDown((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::StandDown:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderStandDown((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::Stay:
		if(m_Minutes >= 0 && m_Minutes <= 0xffff)
			return new AuMissionOrderStay((quint16)m_Minutes, execType);
		break;

	case AuMissionOrder::TroopsLoad:
		if(m_PreboardTime >= 0 && m_PreboardTime <= 0xff)
			return new AuMissionOrderTroopsLoad(m_Count <= 0xff ? m_Count : 0, (quint8)m_PreboardTime, execType);
		break;
	}

	// Return an invalid order.
	return 0;
}

void AuMissionOrderWidget::OrderSelected()
{
	// Get type so it can be retrieved after the widget is gone.
	QItemSelectionModel* sel = m_Orders->selectionModel();
	if(sel && sel->hasSelection())
	{
		QModelIndexList selRowIdxs = sel->selectedRows();
		Q_ASSERT(selRowIdxs.count());
		QModelIndex selIdx = selRowIdxs.first();
		m_Order = (AuMissionOrder::TYPE)selIdx.data(Qt::UserRole).toInt();
	}

	if(m_Order == AuMissionOrder::HarvestOperationByBay)
	{
		// Do this every time because the person's scene may have changed.
		QHash<AuId, PlanetOrStarEntry> planets;		// in current system
		QMap<quint16, COMMODITYID>	shipBays;		// Bays on current ship. Key = which id, value = module.
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaVessel* ship = person->FindParentVesselInEnv();
				const DaUnit* refUnit = ship;
				if(!refUnit)
					refUnit = person;

				// Get bay information from current ship, for harvest orders.
				if(ship)
				{
					// Build a list of the bays with their modules.
					QList<quint16> bayIds;
					ship->WeaponBayIds(bayIds);
					for(QList<quint16>::const_iterator bi = bayIds.cbegin(); bi != bayIds.cend(); bi++)
						shipBays.insert(*bi, ship->SystemModule(SYS_Weapon, *bi));
				}

				if(refUnit)
				{
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// List planets and stars in the scene atom's current solar system.
					// We can only show the planets and stars if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, system);

								// Calcualate a sort order for the planet. We would like each star to be followed by its
								// planets, each of them followed by its moons. Name sorting does not work for this.
								const DaStar* star = planet->FindParentStarInEnv();
								double sortOrder = star->StarNumber() * 1000000;
								const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
								if(parPlanet)
								{
									sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
									sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
								}
								else
									sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planent.m_SortOrder = sortOrder;
								planets.insert(planet->Id(), planent);
							}
							else
							{
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, system);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}
					else if(system && system->IsExploredBy(refUnit->EmpireId()))
					{
						// Gen the system like the system scan to list the planets.
						if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
						{
							// Scanned system changed.
							delete m_ScannedSystem;
							m_ScannedSystem = 0;
						}
						if(!m_ScannedSystem)
						{
							m_Ether.DeleteEther();
							m_ScannedSystem = new DaSolarSystem;
							m_ScannedSystem->SetId(system->Id());
							m_Ether.AddAtom(m_ScannedSystem);

							float starLight = -1.f;
							m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
									system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
						}

						AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

						DaAtomList atoms;
						m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

								// Calcualate a sort order for the planet. We would like each star to be followed by its
								// planets, each of them followed by its moons. Name sorting does not work for this.
								const DaStar* star = planet->FindParentStarInEnv();
								double sortOrder = star->StarNumber() * 1000000;
								const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
								if(parPlanet)
								{
									sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
									sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
								}
								else
									sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planent.m_SortOrder = sortOrder;
								planets.insert(planet->Id(), planent);
							}
							else
							{
								// GoToOrbit and GoToOrbitCoords orders list stars too.
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}

					// List planets using the global empires data.
					// Empire records know what planets have cities that belong to them.
					const DgData& gd = g_Au.LockDataForRead();
					{
						DgEmpire empire;
						empire.m_id = refUnit->EmpireId();
						if(gd.Empire(empire))
						{
							// Found my empire record.
							// List the planets that host cities in the order system.
							double sortOrder = 0;
							for(AuCityHash::const_iterator eci = empire.m_Cities->cbegin(); eci != empire.m_Cities->cend(); eci++)
								if((*eci).second.SystemId() == m_OrderSystemId)
									if(!planets.contains((*eci).second.PlanetId()))
									{
										PlanetOrStarEntry planent;
										planent.m_Name = (*eci).second.PlanetName();
										planent.m_Info = tr("Has Empire City: %1").arg((*eci).second.CityName());
										planent.m_Distance = 0;
										planent.m_IsStar = false;
										planent.m_SortOrder = sortOrder++;
										planets.insert((*eci).second.PlanetId(), planent);
									}
						}
					}
					g_Au.UnlockData();
				}
			}
			view->UnlockSceneAtom();
		}

		// Fill planets table.
		m_HarvestPlanetsAndStars->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
		m_HarvestPlanetsAndStars->setRowCount(planets.count());
		QIcon iconW(":data/World.png");
		QIcon iconS(":data/c_Sunlight.png");
		int trow=0;
		for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
		{
			int col=0;
			QTableWidgetItem* twi;
			if((*si).m_IsStar)
				twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconS, (*si).m_Name);
			else
				twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconW, (*si).m_Name);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStars->setItem(trow, col++, twi);

			twi = new AuTableWidgetDistanceItem((*si).m_Distance);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStars->setItem(trow, col++, twi);

			twi = new QTableWidgetItem((*si).m_Info);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStars->setItem(trow, col++, twi);

			m_HarvestPlanetsAndStars->setRowHeight(trow, 18);
		}
		m_HarvestPlanetsAndStars->sortByColumn(0, Qt::AscendingOrder);
		m_HarvestPlanetsAndStars->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
		m_HarvestPlanetsAndStars->resizeColumnsToContents();

		// Update harvest operation widgets. We need one per actual bay and no extras.
		// Show or hide the ones we already have, depending on if we want them right now.
		for(QMap<quint16, AuHarvestOpWidget*>::iterator hwi = m_HarvestByBay.begin(); hwi != m_HarvestByBay.end(); hwi++)
		{
			if(shipBays.contains(hwi.key()))
				(*hwi)->show();
			else
				(*hwi)->hide();
		}
		// Create harvester op widgets that we are missing.
		for(QMap<quint16, COMMODITYID>::const_iterator bi = shipBays.cbegin(); bi != shipBays.cend(); bi++)
		{
			if(m_HarvestByBay.contains(bi.key()))
				continue;

			// Create a new harvest op widget.
			AuHarvestOpWidget* opwid;
			AuHarvestOps::const_iterator oi = m_HarvestOps.find(bi.key());
			QString title = tr("Bay %1, %2").arg(bi.key()).arg(g_Au.CommodityName(*bi));
			if(oi == m_HarvestOps.cend())
				opwid = new AuHarvestOpWidget(title, *bi, CID_Invalid, 2, 100, 0, this);
			else
				opwid = new AuHarvestOpWidget(title, *bi, (*oi).CommodityId(), (*oi).AmountType(), (*oi).Amount(), (*oi).Amount(),
						this);
			m_HarvestOpLay->addWidget(opwid);
			m_HarvestByBay.insert(bi.key(), opwid);
		}
	}
	else if(m_Order == AuMissionOrder::HarvestOperationByModule)
	{
		// Do this every time because the person's scene may have changed.
		QHash<AuId, PlanetOrStarEntry> planets;		// in current system
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// List planets and stars in the scene atom's current solar system.
					// We can only show the planets and stars if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, system);

								// Calcualate a sort order for the planet. We would like each star to be followed by its
								// planets, each of them followed by its moons. Name sorting does not work for this.
								const DaStar* star = planet->FindParentStarInEnv();
								double sortOrder = star->StarNumber() * 1000000;
								const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
								if(parPlanet)
								{
									sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
									sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
								}
								else
									sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planent.m_SortOrder = sortOrder;
								planets.insert(planet->Id(), planent);
							}
							else
							{
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, system);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}
					else if(system && system->IsExploredBy(refUnit->EmpireId()))
					{
						// Gen the system like the system scan to list the planets.
						if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
						{
							// Scanned system changed.
							delete m_ScannedSystem;
							m_ScannedSystem = 0;
						}
						if(!m_ScannedSystem)
						{
							m_Ether.DeleteEther();
							m_ScannedSystem = new DaSolarSystem;
							m_ScannedSystem->SetId(system->Id());
							m_Ether.AddAtom(m_ScannedSystem);

							float starLight = -1.f;
							m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
									system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
						}

						AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

						DaAtomList atoms;
						m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

								// Calcualate a sort order for the planet. We would like each star to be followed by its
								// planets, each of them followed by its moons. Name sorting does not work for this.
								const DaStar* star = planet->FindParentStarInEnv();
								double sortOrder = star->StarNumber() * 1000000;
								const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
								if(parPlanet)
								{
									sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
									sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
								}
								else
									sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planent.m_SortOrder = sortOrder;
								planets.insert(planet->Id(), planent);
							}
							else
							{
								// GoToOrbit and GoToOrbitCoords orders list stars too.
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}

					// List planets using the global empires data.
					// Empire records know what planets have cities that belong to them.
					const DgData& gd = g_Au.LockDataForRead();
					{
						DgEmpire empire;
						empire.m_id = refUnit->EmpireId();
						if(gd.Empire(empire))
						{
							// Found my empire record.
							// List the planets that host cities in the order system.
							double sortOrder = 0;
							for(AuCityHash::const_iterator eci = empire.m_Cities->cbegin(); eci != empire.m_Cities->cend(); eci++)
								if((*eci).second.SystemId() == m_OrderSystemId)
									if(!planets.contains((*eci).second.PlanetId()))
									{
										PlanetOrStarEntry planent;
										planent.m_Name = (*eci).second.PlanetName();
										planent.m_Info = tr("Has Empire City: %1").arg((*eci).second.CityName());
										planent.m_Distance = 0;
										planent.m_IsStar = false;
										planent.m_SortOrder = sortOrder++;
										planets.insert((*eci).second.PlanetId(), planent);
									}
						}
					}
					g_Au.UnlockData();
				}
			}
			view->UnlockSceneAtom();
		}

		// Fill planets table.
		m_HarvestPlanetsAndStarsByM->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
		m_HarvestPlanetsAndStarsByM->setRowCount(planets.count());
		QIcon iconW(":data/World.png");
		QIcon iconS(":data/c_Sunlight.png");
		int trow=0;
		for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
		{
			int col=0;
			QTableWidgetItem* twi;
			if((*si).m_IsStar)
				twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconS, (*si).m_Name);
			else
				twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconW, (*si).m_Name);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStarsByM->setItem(trow, col++, twi);

			twi = new AuTableWidgetDistanceItem((*si).m_Distance);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStarsByM->setItem(trow, col++, twi);

			twi = new QTableWidgetItem((*si).m_Info);
			twi->setData(Qt::UserRole, si.key().GetID());
			m_HarvestPlanetsAndStarsByM->setItem(trow, col++, twi);

			m_HarvestPlanetsAndStarsByM->setRowHeight(trow, 18);
		}
		m_HarvestPlanetsAndStarsByM->sortByColumn(0, Qt::AscendingOrder);
		m_HarvestPlanetsAndStarsByM->setSortingEnabled(true);	// enabled after table is filled otherwise it messes with the fill
		m_HarvestPlanetsAndStarsByM->resizeColumnsToContents();
	}
	else
	{
		// Wait until the Follow or Attack order is selected before populating the targets table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::Follow || m_Order == AuMissionOrder::Attack || m_Order == AuMissionOrder::Plunder)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QMultiMap<QString, TargetEntry> targets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaAtom* env = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						env = person->Ether()->FindNormalAtom(orderSystemId);
					if(!env)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						env = person->FindEnvironmentAtom();
						if(env)
							orderSystemId = env->Id();
					}

					// We can only show the targets lists if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(env && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, env);

						// List targets in the solar system.
						QMap<AuId, QString> listed;	// Atoms listed and the name key used.
						DaAtomList atoms;
						env->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaVessel* sysShip = dynamic_cast<const DaVessel*>(*ai);
							if(sysShip)
							{
								// Ships are targets that can be followed or attacked.
								if(sysShip->IsInsideSpacecraft())
									continue;	// Do not target units that are inside spacecraft.
								AuPoint targSysLoc;
								sysShip->GetLocationRelativeToParent(targSysLoc, env);

								TargetEntry targent;
								targent.m_Id = sysShip->Id();
								targent.m_Info = sysShip->SpotDescription(person->EmpireId());
								targent.m_Stance = PoliticalStance(person->EmpireId(), sysShip->EmpireId());
								targent.m_Distance = targSysLoc.Distance(personSysLoc);
								targets.insert(sysShip->Name(), targent);

								listed.insert(sysShip->Id(), sysShip->Name());
								continue;
							}

							const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(*ai);
							if(vehicle)
							{
								if(vehicle->IsFlying())
								{
									// Vehicles that are flying can be followed.
									if(vehicle->IsInsideSpacecraft())
										continue;	// Do not target units that are inside spacecraft.
									AuPoint targSysLoc;
									vehicle->GetLocationRelativeToParent(targSysLoc, env);

									TargetEntry targent;
									targent.m_Id = vehicle->Id();
									targent.m_Info = vehicle->SpotDescription(person->EmpireId());
									targent.m_Stance = PoliticalStance(person->EmpireId(), vehicle->EmpireId());
									targent.m_Distance = targSysLoc.Distance(personSysLoc);
									targets.insert(vehicle->Name(), targent);

									listed.insert(vehicle->Id(), vehicle->Name());
								}
								continue;
							}

							const DaAsteroid* roid = dynamic_cast<const DaAsteroid*>(*ai);
							if(roid)
							{
								// Asteroids are targets that can be followed or attacked.
								if(roid->IsAwaitingRelease())
									continue;	// Do not target asteroids that are dormant.
								AuPoint targSysLoc;
								roid->GetLocationRelativeToParent(targSysLoc, env);

								TargetEntry targent;
								targent.m_Id = roid->Id();
								targent.m_Info = roid->SpotDescription(person->EmpireId());
								targent.m_Stance = POL_Enemy;	//PoliticalStance(person->EmpireId(), roid->EmpireId());
								targent.m_Distance = targSysLoc.Distance(personSysLoc);
								targets.insert(roid->Name(), targent);

								listed.insert(roid->Id(), roid->Name());
								continue;
							}
						}

						// Check ship sensor blips. There might be something we haven't listed.
						const DaVessel* ship = person->FindParentVesselInEnv();
						if(ship)
						{
							// Sensor blips on the client mostly only exist on the person's console.
							// If they don't have a console, the best we can hope for is the locked-on blip, which will be in the
							// ship's sensor blip array.
							for(AuSensorBlipList::const_iterator bi = m_SensorBlips.begin(); bi != m_SensorBlips.end(); bi++)
							{
								if((*bi)->IsNewSpacecraft() || (*bi)->IsNewVehicle())
									continue;	// Don't list "new" spacecraft and vehicles.

								if((*bi)->IsVehicle() || (*bi)->IsSpacecraft() || (*bi)->IsAsteroid())
								{
									// Targets that can be followed or attacked.
									if(listed.contains((*bi)->AtomId()))
									{
										// Add extra sensor blip information to the location description.
										QMultiMap<QString, TargetEntry>::iterator ti = targets.find((*bi)->ScreenName());
										if(ti != targets.end())
											(*ti).m_Info += ", " + (*bi)->HtmlDesc(env);
									}
									else
									{
										TargetEntry targent;
										targent.m_Id = (*bi)->AtomId();
										targent.m_Info = (*bi)->HtmlDesc(env);
										targent.m_Stance = (*bi)->PoliticalStance();
										targent.m_Distance = (*bi)->Range(ship->EnvLocation());
										targets.insert((*bi)->ScreenName(), targent);
									}
								}
							}
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill targets table.
			m_Targets->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Targets->setRowCount(targets.count());
			QIcon icon(":data/dp_Sensor.png");
			int trow=0;
			for(QMultiMap<QString, TargetEntry>::const_iterator si = targets.cbegin(); si != targets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, si.key());
				twi->setData(Qt::UserRole, (*si).m_Id.GetID());
				m_Targets->setItem(trow, col++, twi);

				switch((*si).m_Stance)
				{
				case POL_Friend:
					twi = new QTableWidgetItem(tr("Friend"));
					break;
				case POL_Neutral:
					twi = new QTableWidgetItem(tr("Neutral"));
					break;
				case POL_Enemy:
					twi = new QTableWidgetItem(tr("Enemy"));
					break;
				case POL_Empire:
					twi = new QTableWidgetItem(tr("Empire"));
					break;
				case POL_Vassal:
					twi = new QTableWidgetItem(tr("Overlord"));	// How vassal perceives target.
					break;
				}
				twi->setForeground(PoliticalStanceColor((*si).m_Stance, false));
				twi->setData(Qt::UserRole, (*si).m_Id.GetID());
				m_Targets->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, (*si).m_Id.GetID());
				m_Targets->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, (*si).m_Id.GetID());
				m_Targets->setItem(trow, col++, twi);

				m_Targets->setRowHeight(trow, 18);
			}
			m_Targets->sortByColumn(0, Qt::AscendingOrder);
			m_Targets->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Targets->resizeColumnsToContents();
		}

		// Wait until a harvest order is selected to populate the bay id combo box.
		if(m_Order == AuMissionOrder::HarvestAtmosphere)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QList<quint16> bays;	// 'which' ids of atmosphere condenser bays
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					ship->WeaponBaysWithModule(bays, CID_AtmosphereCondenserBayModule);
			}
			view->UnlockSceneAtom();
		}
		else if(m_Order == AuMissionOrder::HarvestMinerals)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QList<quint16> bays;	// 'which' ids of prospector bays
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					ship->WeaponBaysWithModule(bays, CID_ProspectorBayModule);
			}
			view->UnlockSceneAtom();
		}
		else if(m_Order == AuMissionOrder::HarvestSea)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QList<quint16> bays;	// 'which' ids of siphon bays
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					ship->WeaponBaysWithModule(bays, CID_SiphonBayModule);
			}
			view->UnlockSceneAtom();
		}
		else if(m_Order == AuMissionOrder::HarvestWell)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QList<quint16> bays;	// 'which' ids of wildcatter bays
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					ship->WeaponBaysWithModule(bays, CID_WildcatterBayModule);
			}
			view->UnlockSceneAtom();
		}
		else if(m_Order == AuMissionOrder::HarvestParticles)
		{
			// Make maps of the data entries so they are sorted alphabetically.
			QList<quint16> bays;	// 'which' ids of particle collector bays
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
					ship->WeaponBaysWithModule(bays, CID_ParticleCollectorBayModule);
			}
			view->UnlockSceneAtom();
		}

		// Wait until the GoToStation order is selected before populating the stations table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::GoToStation)
		{
			// Make a map of the station entries so they are sorted alphabetically.
			QHash<AuId, StationEntry> stations;	// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// We can only show the targets list if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						// List neutral or stations in the solar system.
						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaVessel* sysShip = dynamic_cast<const DaVessel*>(*ai);
							if(sysShip)
							{
								if(sysShip->IsSpaceStation())
								{
									// Test political stance.
									POLITICAL_STANCE stance = PoliticalStance(person->EmpireId(), sysShip->EmpireId());
									if(stance == POL_Enemy)
										continue;

									AuPoint statSysLoc;
									sysShip->GetLocationRelativeToParent(statSysLoc, system);

									StationEntry statent;
									statent.m_Name = sysShip->Name();
									const DaWorld* statPlanet = sysShip->FindParentPlanetInEnv();
									if(statPlanet)
										statent.m_Info = statPlanet->Name();
									else
										statent.m_Info = "System";
									statent.m_Stance = stance;
									statent.m_Distance = statSysLoc.Distance(personSysLoc);
									stations.insert(sysShip->Id(), statent);
								}
							}
						}

						// Check ship sensor blips. There might be something we haven't listed.
						const DaVessel* ship = person->FindParentVesselInEnv();
						if(ship)
						{
							// Sensor blips on the client mostly only exists on the person's console.
							// If they don't have a console, the best we can hope for is the locked-on blip, which will be in the ship's
							// sensor blip array.
							for(AuSensorBlipList::const_iterator bi = m_SensorBlips.begin(); bi != m_SensorBlips.end(); bi++)
							{
								if((*bi)->IsSpaceStation())
								{
									// Stations
									if(stations.contains((*bi)->AtomId()))
									{
										// Add extra sensor blip information to the location description.
										stations[(*bi)->AtomId()].m_Info += ", " + (*bi)->HtmlDesc(system);
									}
									else
									{
										StationEntry statent;
										statent.m_Name = (*bi)->ScreenName();
										statent.m_Info = (*bi)->HtmlDesc(system);
										statent.m_Stance = (*bi)->PoliticalStance();
										statent.m_Distance = (*bi)->Range(ship->EnvLocation());
										stations.insert((*bi)->AtomId(), statent);
									}
								}
							}
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill stations table.
			m_Stations->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Stations->setRowCount(stations.count());
			QIcon icon(":data/CommStation.png");
			int trow=0;
			for(QHash<AuId, StationEntry>::const_iterator si = stations.cbegin(); si != stations.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Stations->setItem(trow, col++, twi);

				switch((*si).m_Stance)
				{
				case POL_Friend:
					twi = new QTableWidgetItem(tr("Friend"));
					break;
				case POL_Neutral:
					twi = new QTableWidgetItem(tr("Neutral"));
					break;
				case POL_Enemy:
					twi = new QTableWidgetItem(tr("Enemy"));
					break;
				case POL_Empire:
					twi = new QTableWidgetItem(tr("Empire"));
					break;
				case POL_Vassal:
					twi = new QTableWidgetItem(tr("Overlord"));	// How vassal perceives target.
					break;
				}
				twi->setForeground(PoliticalStanceColor((*si).m_Stance, false));
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Stations->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Stations->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Stations->setItem(trow, col++, twi);

				m_Stations->setRowHeight(trow, 18);
			}
			m_Stations->sortByColumn(0, Qt::AscendingOrder);
			m_Stations->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Stations->resizeColumnsToContents();
		}

		// Wait until the GoToLandingSite, GoToOrbit, or GoToHoverSite order is selected before populating the planets table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::GoToLandingSite || m_Order == AuMissionOrder::GoToOrbit ||
					m_Order == AuMissionOrder::GoToHoverSite || m_Order == AuMissionOrder::GoToLand ||
					m_Order == AuMissionOrder::GoToOrbitCoords || m_Order == AuMissionOrder::HarvestAtmosphere ||
					m_Order == AuMissionOrder::HarvestMinerals || m_Order == AuMissionOrder::HarvestSea ||
					m_Order == AuMissionOrder::HarvestWell)
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			QHash<AuId, PlanetOrStarEntry> planets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaWorld* refPlanet = refUnit->FindParentPlanetInEnv();
					if(refPlanet)
					{
						// Get current long lat of refUnit.
						AuPoint refLoc;
						refUnit->GetLocationRelativeToParent(refLoc, refPlanet);
						const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(refPlanet);
						if(llworld)
						{
							AuGlobeLLF ll;
							if(llworld->LLOfLoc(ll, refLoc))
								m_PlanetLongLat = ll.ToString();
						}
					}
					else
					{
						const DaStar* refStar = refUnit->FindParentStarInEnv();
						if(refStar)
						{
							// Get current long lat of refUnit.
							AuPoint refLoc;
							refUnit->GetLocationRelativeToParent(refLoc, refStar);
							AuGlobeLLF ll;
							ll.SetFromGlobeLocation(refLoc);
							m_PlanetLongLat = ll.ToString();
						}
					}

					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// List planets and stars in the scene atom's current solar system.
					// We can only show the planets and stars if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						// List neutral or better cities and stations in the solar system.
						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								// Rings are only listed with certain orders.
								if(!dynamic_cast<const DaWorldRing*>(planet) ||
										m_Order == AuMissionOrder::GoToOrbit || m_Order == AuMissionOrder::GoToOrbitCoords)
								{
									AuPoint planSysLoc;
									planet->GetLocationRelativeToParent(planSysLoc, system);

									// Calcualate a sort order for the planet. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									const DaStar* star = planet->FindParentStarInEnv();
									double sortOrder = star->StarNumber() * 1000000;
									const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
									if(parPlanet)
									{
										sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
										sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
									}
									else
										sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

									PlanetOrStarEntry planent;
									planent.m_Name = planet->Name();
									planent.m_Info = planet->Description();
									planent.m_Distance = planSysLoc.Distance(personSysLoc);
									planent.m_IsStar = false;
									planent.m_SortOrder = sortOrder;
									planets.insert(planet->Id(), planent);
								}
							}
							else if(m_Order == AuMissionOrder::GoToOrbit || m_Order == AuMissionOrder::GoToOrbitCoords)
							{
								// GoToOrbit and GoToOrbitCoords orders list stars too.
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, system);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}
					else if(system && system->IsExploredBy(refUnit->EmpireId()))
					{
						// Gen the system like the system scan to list the planets.
						if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
						{
							// Scanned system changed.
							delete m_ScannedSystem;
							m_ScannedSystem = 0;
						}
						if(!m_ScannedSystem)
						{
							m_Ether.DeleteEther();
							m_ScannedSystem = new DaSolarSystem;
							m_ScannedSystem->SetId(system->Id());
							m_Ether.AddAtom(m_ScannedSystem);

							float starLight = -1.f;
							m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
									system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
						}

						AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

						// List neutral or better cities and stations in the solar system.
						DaAtomList atoms;
						m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
							if(planet)
							{
								// Rings are only listed with certain orders.
								if(!dynamic_cast<const DaWorldRing*>(planet) ||
										m_Order == AuMissionOrder::GoToOrbit || m_Order == AuMissionOrder::GoToOrbitCoords)
								{
									AuPoint planSysLoc;
									planet->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

									// Calcualate a sort order for the planet. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									const DaStar* star = planet->FindParentStarInEnv();
									double sortOrder = star->StarNumber() * 1000000;
									const DaWorld* parPlanet = planet->Parent()->FindParentPlanetInEnv();
									if(parPlanet)
									{
										sortOrder += parPlanet->OrbitRadius() / FeetPerOrbit() * 10000;
										sortOrder += planet->Location().Length()/FeetPerMoonOrbit();
									}
									else
										sortOrder += planet->OrbitRadius() / FeetPerOrbit() * 10000;

									PlanetOrStarEntry planent;
									planent.m_Name = planet->Name();
									planent.m_Info = planet->Description();
									planent.m_Distance = planSysLoc.Distance(personSysLoc);
									planent.m_IsStar = false;
									planent.m_SortOrder = sortOrder;
									planets.insert(planet->Id(), planent);
								}
							}
							else if(m_Order == AuMissionOrder::GoToOrbit || m_Order == AuMissionOrder::GoToOrbitCoords)
							{
								// GoToOrbit and GoToOrbitCoords orders list stars too.
								const DaStar* star = dynamic_cast<const DaStar*>(*ai);
								if(star)
								{
									AuPoint planSysLoc;
									star->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

									// Calcualate a sort order for the star. We would like each star to be followed by its
									// planets, each of them followed by its moons. Name sorting does not work for this.
									double sortOrder = star->StarNumber() * 1000000;

									PlanetOrStarEntry starent;
									starent.m_Name = star->Name();
									starent.m_Info = star->Description();
									starent.m_Distance = planSysLoc.Distance(personSysLoc);
									starent.m_IsStar = true;
									starent.m_SortOrder = sortOrder;
									planets.insert(star->Id(), starent);
								}
							}
						}
					}

					// List planets using the global empires data.
					// Empire records know what planets have cities that belong to them.
					const DgData& gd = g_Au.LockDataForRead();
					{
						DgEmpire empire;
						empire.m_id = refUnit->EmpireId();
						if(gd.Empire(empire))
						{
							// Found my empire record.
							// List the planets that host cities in the order system.
							double sortOrder = 0;
							for(AuCityHash::const_iterator eci = empire.m_Cities->cbegin(); eci != empire.m_Cities->cend(); eci++)
								if((*eci).second.SystemId() == m_OrderSystemId)
									if(!planets.contains((*eci).second.PlanetId()))
									{
										PlanetOrStarEntry planent;
										planent.m_Name = (*eci).second.PlanetName();
										planent.m_Info = tr("Has Empire City: %1").arg((*eci).second.CityName());
										planent.m_Distance = 0;
										planent.m_IsStar = false;
										planent.m_SortOrder = sortOrder++;
										planets.insert((*eci).second.PlanetId(), planent);
									}
						}
					}
					g_Au.UnlockData();
				}
			}
			view->UnlockSceneAtom();

			// Fill long lat control with person's current location.
			m_CoordsEdit->setText(m_PlanetLongLat);

			// Fill planets table.
			m_PlanetsAndStars->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_PlanetsAndStars->setRowCount(planets.count());
			QIcon iconW(":data/World.png");
			QIcon iconS(":data/c_Sunlight.png");
			int trow=0;
			for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi;
				if((*si).m_IsStar)
					twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconS, (*si).m_Name);
				else
					twi = new AuTableWidgetSortedItem((*si).m_SortOrder, iconW, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsAndStars->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsAndStars->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsAndStars->setItem(trow, col++, twi);

				m_PlanetsAndStars->setRowHeight(trow, 18);
			}
			m_PlanetsAndStars->sortByColumn(0, Qt::AscendingOrder);
			m_PlanetsAndStars->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_PlanetsAndStars->resizeColumnsToContents();
		}

		// Wait until the HarvestMinerals order is selected before populating the rings table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::HarvestMinerals)
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			QHash<AuId, PlanetOrStarEntry> planets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				const DaAtom* sceneSystem = person->FindEnvironmentAtom();
				if(sceneSystem)
					sceneSystemId = sceneSystem->Id();

				const DaSolarSystem* system = 0;
				AuId orderSystemId(m_OrderSystemId);
				if(!orderSystemId.IsNull())
					system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
				if(!system)
				{
					// Use person's current system as origin if a null or invalid system id was provided.
					system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
					if(system)
						orderSystemId = system->Id();
				}

				// List rings in the scene atom's current solar system.
				// We can only show the rings if the solar system context of this order is the same as the
				// scene atom's current solar system.
				if(system && sceneSystemId == orderSystemId)
				{
					AuPoint personSysLoc;
					person->GetLocationRelativeToParent(personSysLoc, system);

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					system->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(*ai);	// ring
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets, ringworlds and rings are listed.
						if(ring || world)
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, system);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
				else if(system && system->IsExploredBy(refUnit->EmpireId()))
				{
					// Gen the system like the system scan to list the planets.
					if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
					{
						// Scanned system changed.
						delete m_ScannedSystem;
						m_ScannedSystem = 0;
					}
					if(!m_ScannedSystem)
					{
						m_Ether.DeleteEther();
						m_ScannedSystem = new DaSolarSystem;
						m_ScannedSystem->SetId(system->Id());
						m_Ether.AddAtom(m_ScannedSystem);

						float starLight = -1.f;
						m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
								system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
					}

					AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(*ai);	// ring
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets, ringworlds and rings are listed.
						if(ring || world)
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill planets table.
			m_PlanetsMin->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_PlanetsMin->setRowCount(planets.count());
			QIcon icon(":data/World.png");
			int trow=0;
			for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsMin->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsMin->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsMin->setItem(trow, col++, twi);

				m_PlanetsMin->setRowHeight(trow, 18);
			}
			m_PlanetsMin->sortByColumn(0, Qt::AscendingOrder);
			m_PlanetsMin->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_PlanetsMin->resizeColumnsToContents();
		}

		// Wait until the HarvestSea order is selected before populating the rings table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::HarvestSea)
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			QHash<AuId, PlanetOrStarEntry> planets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				const DaAtom* sceneSystem = person->FindEnvironmentAtom();
				if(sceneSystem)
					sceneSystemId = sceneSystem->Id();

				const DaSolarSystem* system = 0;
				AuId orderSystemId(m_OrderSystemId);
				if(!orderSystemId.IsNull())
					system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
				if(!system)
				{
					// Use person's current system as origin if a null or invalid system id was provided.
					system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
					if(system)
						orderSystemId = system->Id();
				}

				// List rings in the scene atom's current solar system.
				// We can only show the rings if the solar system context of this order is the same as the
				// scene atom's current solar system.
				if(system && sceneSystemId == orderSystemId)
				{
					AuPoint personSysLoc;
					person->GetLocationRelativeToParent(personSysLoc, system);

					// List planets and ringworlds with seas.
					DaAtomList atoms;
					system->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets and ringworlds with seas are listed.
						if(world && world->Hydrographics())
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, system);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
				else if(system && system->IsExploredBy(refUnit->EmpireId()))
				{
					// Gen the system like the system scan to list the planets.
					if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
					{
						// Scanned system changed.
						delete m_ScannedSystem;
						m_ScannedSystem = 0;
					}
					if(!m_ScannedSystem)
					{
						m_Ether.DeleteEther();
						m_ScannedSystem = new DaSolarSystem;
						m_ScannedSystem->SetId(system->Id());
						m_Ether.AddAtom(m_ScannedSystem);

						float starLight = -1.f;
						m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
								system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
					}

					AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets and ringworlds with seas are listed.
						if(world && world->Hydrographics())
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill planets table.
			m_PlanetsSea->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_PlanetsSea->setRowCount(planets.count());
			QIcon icon(":data/World.png");
			int trow=0;
			for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsSea->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsSea->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsSea->setItem(trow, col++, twi);

				m_PlanetsSea->setRowHeight(trow, 18);
			}
			m_PlanetsSea->sortByColumn(0, Qt::AscendingOrder);
			m_PlanetsSea->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_PlanetsSea->resizeColumnsToContents();
		}

		// Wait until the HarvestWell order is selected before populating the rings table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::HarvestWell)
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			QHash<AuId, PlanetOrStarEntry> planets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				const DaAtom* sceneSystem = person->FindEnvironmentAtom();
				if(sceneSystem)
					sceneSystemId = sceneSystem->Id();

				const DaSolarSystem* system = 0;
				AuId orderSystemId(m_OrderSystemId);
				if(!orderSystemId.IsNull())
					system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
				if(!system)
				{
					// Use person's current system as origin if a null or invalid system id was provided.
					system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
					if(system)
						orderSystemId = system->Id();
				}

				// List rings in the scene atom's current solar system.
				// We can only show the rings if the solar system context of this order is the same as the
				// scene atom's current solar system.
				if(system && sceneSystemId == orderSystemId)
				{
					AuPoint personSysLoc;
					person->GetLocationRelativeToParent(personSysLoc, system);

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					system->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets, ringworlds and rings are listed.
						if(world)
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, system);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
				else if(system && system->IsExploredBy(refUnit->EmpireId()))
				{
					// Gen the system like the system scan to list the planets.
					if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
					{
						// Scanned system changed.
						delete m_ScannedSystem;
						m_ScannedSystem = 0;
					}
					if(!m_ScannedSystem)
					{
						m_Ether.DeleteEther();
						m_ScannedSystem = new DaSolarSystem;
						m_ScannedSystem->SetId(system->Id());
						m_Ether.AddAtom(m_ScannedSystem);

						float starLight = -1.f;
						m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
								system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
					}

					AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(*ai);		// planet, moon or ringworld

						// Only planets, ringworlds and rings are listed.
						if(world)
						{
							AuPoint planSysLoc;
							(*ai)->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

							PlanetOrStarEntry planent;
							planent.m_Name = (*ai)->Name();
							planent.m_Info = (*ai)->Description();
							planent.m_Distance = planSysLoc.Distance(personSysLoc);
							planent.m_IsStar = false;
							planets.insert((*ai)->Id(), planent);
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill planets table.
			m_PlanetsWell->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_PlanetsWell->setRowCount(planets.count());
			QIcon icon(":data/World.png");
			int trow=0;
			for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsWell->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsWell->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsWell->setItem(trow, col++, twi);

				m_PlanetsWell->setRowHeight(trow, 18);
			}
			m_PlanetsWell->sortByColumn(0, Qt::AscendingOrder);
			m_PlanetsWell->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_PlanetsWell->resizeColumnsToContents();
		}

		// Wait until the HarvestParticles order is selected before populating the suns, rings and gas giants table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::HarvestParticles)
		{
			// Make a map of the planet data entries so they are sorted alphabetically.
			QHash<AuId, PlanetOrStarEntry> planets;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				const DaAtom* sceneSystem = person->FindEnvironmentAtom();
				if(sceneSystem)
					sceneSystemId = sceneSystem->Id();

				const DaSolarSystem* system = 0;
				AuId orderSystemId(m_OrderSystemId);
				if(!orderSystemId.IsNull())
					system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
				if(!system)
				{
					// Use person's current system as origin if a null or invalid system id was provided.
					system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
					if(system)
						orderSystemId = system->Id();
				}

				// List rings in the scene atom's current solar system.
				// We can only show the rings if the solar system context of this order is the same as the
				// scene atom's current solar system.
				QMap<AuId, QString> listed;	// Atoms listed and the name key used.
				if(system && sceneSystemId == orderSystemId)
				{
					AuPoint personSysLoc;
					person->GetLocationRelativeToParent(personSysLoc, system);

					DaAtomList atoms;
					system->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
						if(planet)
						{
							// Rings and gas giants can be harvested for particles.
							const DaWorldGasGiant* gworld = dynamic_cast<const DaWorldGasGiant*>(planet);
							const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(planet);
							if(gworld || ring)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, system);

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planets.insert(planet->Id(), planent);
							}
						}
						else
						{
							const DaStar* star = dynamic_cast<const DaStar*>(*ai);
							if(star)
							{
								// Suns can be harvested for particles.
								AuPoint planSysLoc;
								star->GetLocationRelativeToParent(planSysLoc, system);

								PlanetOrStarEntry starent;
								starent.m_Name = star->Name();
								starent.m_Info = "";
								starent.m_Distance = planSysLoc.Distance(personSysLoc);
								starent.m_IsStar = true;
								planets.insert(star->Id(), starent);
							}
						}
					}
				}
				else if(system && system->IsExploredBy(refUnit->EmpireId()))
				{
					// Gen the system like the system scan to list the planets.
					if(m_ScannedSystem && (!m_ScannedSystem->Is(system->Id()) || m_ScannedSystem->Name() != system->Name()))
					{
						// Scanned system changed.
						delete m_ScannedSystem;
						m_ScannedSystem = 0;
					}
					if(!m_ScannedSystem)
					{
						m_Ether.DeleteEther();
						m_ScannedSystem = new DaSolarSystem;
						m_ScannedSystem->SetId(system->Id());
						m_Ether.AddAtom(m_ScannedSystem);

						float starLight = -1.f;
						m_ScannedSystem->PrepareForResourceScan(system->CatalogName(), system->Name(), refUnit->Galaxy(), system->GenHab(),
								system->GenSeed(), system->IsCoreBlackHole(), system->Transform(), starLight);
					}

					AuPoint personSysLoc;	// Default 0,0,0 will produce distances from system center.

					// List planets, ringworlds and rings.
					DaAtomList atoms;
					m_ScannedSystem->ListNormalSubatomsRecursive(atoms);
					for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
					{
						const DaWorld* planet = dynamic_cast<const DaWorld*>(*ai);
						if(planet)
						{
							// Rings and gas giants can be harvested for particles.
							const DaWorldGasGiant* gworld = dynamic_cast<const DaWorldGasGiant*>(planet);
							const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(planet);
							if(gworld || ring)
							{
								AuPoint planSysLoc;
								planet->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

								PlanetOrStarEntry planent;
								planent.m_Name = planet->Name();
								planent.m_Info = planet->Description();
								planent.m_Distance = planSysLoc.Distance(personSysLoc);
								planent.m_IsStar = false;
								planets.insert(planet->Id(), planent);
							}
						}
						else
						{
							const DaStar* star = dynamic_cast<const DaStar*>(*ai);
							if(star)
							{
								// Suns can be harvested for particles.
								AuPoint planSysLoc;
								star->GetLocationRelativeToParent(planSysLoc, m_ScannedSystem);

								PlanetOrStarEntry starent;
								starent.m_Name = star->Name();
								starent.m_Info = "";
								starent.m_Distance = planSysLoc.Distance(personSysLoc);
								starent.m_IsStar = true;
								planets.insert(star->Id(), starent);
							}
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill rings, gas giants and suns table.
			m_PlanetsPart->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_PlanetsPart->setRowCount(planets.count());
			QIcon iconW(":data/World.png");
			QIcon iconS(":data/c_Sunlight.png");
			int trow=0;
			for(QHash<AuId, PlanetOrStarEntry>::const_iterator si = planets.cbegin(); si != planets.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi;
				if((*si).m_IsStar)
					twi = new QTableWidgetItem(iconS, (*si).m_Name);
				else
					twi = new QTableWidgetItem(iconW, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsPart->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsPart->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_PlanetsPart->setItem(trow, col++, twi);

				m_PlanetsPart->setRowHeight(trow, 18);
			}
			m_PlanetsPart->sortByColumn(0, Qt::AscendingOrder);
			m_PlanetsPart->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_PlanetsPart->resizeColumnsToContents();
		}

		// Wait until the GoToCity order is selected before populating the cities table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::GoToCity)
		{
			// Make a map of the city entries so they are sorted alphabetically.
			// This order is specifically looking for airports.
			QHash<AuId, CityEntry> cities;		// in current system
			AuId sceneSystemId;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// List airports in the scene atom's current solar system.
					// We can only show city atoms in the list if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						// List neutral or better cities in the solar system.
						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaBd* bd = dynamic_cast<const DaBd*>(*ai);
							if(bd && (bd->IndustryId() == IID_AirportTerminal || bd->IndustryId() == IID_BaseAirTerminal))
							{
								AuPoint fieldSysLoc;
								bd->GetLocationRelativeToParent(fieldSysLoc, system);

								CityEntry citent;
								citent.m_Name = bd->CityName() + " " + bd->Name();
								citent.m_Info = tr("%1 on %2").arg(bd->Name()).arg(bd->FindParentPlanetInEnv()->Name());
								citent.m_Stance = PoliticalStance(person->EmpireId(), bd->EmpireId());
								citent.m_Distance = fieldSysLoc.Distance(personSysLoc);
								cities.insert(bd->Id(), citent);
							}
						}

						// Check ship sensor blips. There might be something we haven't listed.
						const DaVessel* ship = person->FindParentVesselInEnv();
						if(ship)
						{
							// Sensor blips on the client mostly only exists on the person's console.
							// If they don't have a console, the best we can hope for is the locked-on blip, which will be in the
							// ship's sensor blip array.
							for(AuSensorBlipList::const_iterator bi = m_SensorBlips.begin(); bi != m_SensorBlips.end(); bi++)
							{
								if((*bi)->IsCity())
								{
									// Cities
									if(cities.contains((*bi)->AtomId()))
									{
										// Add extra sensor blip information to the location description.
										cities[(*bi)->AtomId()].m_Info += ", " + (*bi)->HtmlDesc(system);
									}
									else
									{
										CityEntry citent;
										citent.m_Name = (*bi)->ScreenName();
										citent.m_Info = (*bi)->HtmlDesc(system);
										citent.m_Stance = (*bi)->PoliticalStance();
										citent.m_Distance = (*bi)->Range(ship->EnvLocation());
										cities.insert((*bi)->AtomId(), citent);
									}
								}
							}
						}
					}

					// List airports using the star map data.
					{
						AuIdList cityIds;
						QStringList cityNames;
						QList<quint32> cityEmpires;
						g_AuClient.StarMapListAirportsInSystem(orderSystemId, cityIds, cityNames, cityEmpires);
						if(!cityIds.isEmpty())
						{
							AuIdList::const_iterator ii = cityIds.cbegin();
							QStringList::const_iterator ni = cityNames.cbegin();
							QList<quint32>::const_iterator ei = cityEmpires.cbegin();
							while(ii != cityIds.cend())
							{
								if(!cities.contains(*ii))
								{
									CityEntry citent;
									citent.m_Name = *ni;
									citent.m_Info = tr("Star Map City with Airport");
									citent.m_Stance = PoliticalStance(refUnit->EmpireId(), *ei);
									citent.m_Distance = 0;
									cities.insert(*ii, citent);
								}
								ii++;
								ni++;
								ei++;
							}
						}
					}

					// Empire records know what airports belong to them.
					const DgData& gd = g_Au.LockDataForRead();
					{
						DgEmpire empire;
						empire.m_id = refUnit->EmpireId();
						if(gd.Empire(empire))
						{
							// Found my empire record.
							// List the cities in the order system.
							for(AuCityHash::const_iterator eci = empire.m_Cities->cbegin(); eci != empire.m_Cities->cend(); eci++)
								if((*eci).second.SystemId() == m_OrderSystemId)
									if(!cities.contains((*eci).first))
									{
										if(!(*eci).second.Facilities().HasAirport() && !(*eci).second.Facilities().HasAirBase())
											continue;
										CityEntry citent;
										citent.m_Name = (*eci).second.CityName();
										citent.m_Info = tr("Empire City with Airport on %1").arg((*eci).second.PlanetName());
										citent.m_Stance = POL_Empire;
										citent.m_Distance = 0;
										cities.insert((*eci).first, citent);
									}
						}
					}
					g_Au.UnlockData();
				}
			}
			view->UnlockSceneAtom();

			// Fill cities table.
			m_Airports->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Airports->setRowCount(cities.count());
			QIcon icon(":data/City.png");
			int trow=0;
			for(QHash<AuId, CityEntry>::const_iterator si = cities.cbegin(); si != cities.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Airports->setItem(trow, col++, twi);

				switch((*si).m_Stance)
				{
				case POL_Friend:
					twi = new QTableWidgetItem(tr("Friend"));
					break;
				case POL_Neutral:
					twi = new QTableWidgetItem(tr("Neutral"));
					break;
				case POL_Enemy:
					twi = new QTableWidgetItem(tr("Enemy"));
					break;
				case POL_Empire:
					twi = new QTableWidgetItem(tr("Empire"));
					break;
				case POL_Vassal:
					twi = new QTableWidgetItem(tr("Overlord"));	// How vassal perceives target.
					break;
				}
				twi->setForeground(PoliticalStanceColor((*si).m_Stance, false));
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Airports->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Airports->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Airports->setItem(trow, col++, twi);

				m_Airports->setRowHeight(trow, 18);
			}
			m_Airports->sortByColumn(0, Qt::AscendingOrder);
			m_Airports->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Airports->resizeColumnsToContents();
		}

		// Wait until the GoToCity order is selected before populating the cities table.
		// Do this every time because the person's scene may have changed.
		if(m_Order == AuMissionOrder::GoToCityAnywhere || m_Order == AuMissionOrder::GoToOrbitCity)
		{
			// Make a map of the city entries so they are sorted alphabetically.
			QHash<AuId, CityEntry> cities;		// in current system
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					AuId sceneSystemId;
					const DaAtom* sceneSystem = person->FindEnvironmentAtom();
					if(sceneSystem)
						sceneSystemId = sceneSystem->Id();

					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}

					// List cities in the scene atom's current solar system.
					// We can only show city atoms in the list if the solar system context of this order is the same as the
					// scene atom's current solar system.
					if(system && sceneSystemId == orderSystemId)
					{
						AuPoint personSysLoc;
						person->GetLocationRelativeToParent(personSysLoc, system);

						// List neutral or better cities in the solar system.
						DaAtomList atoms;
						system->ListNormalSubatomsRecursive(atoms);
						for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
						{
							const DaBd* bd = dynamic_cast<const DaBd*>(*ai);
							if(bd && (bd->IndustryId() == IID_Capitol || bd->IndustryId() == IID_BaseHeadquarters))
							{
								// Found a city capitol or a base hq.
								AuPoint fieldSysLoc;
								bd->GetLocationRelativeToParent(fieldSysLoc, system);

								CityEntry citent;
								citent.m_Name = bd->CityName();
								if(bd->IndustryId() == IID_Capitol)
									citent.m_Info = tr("City on %2").arg(bd->FindParentPlanetInEnv()->Name());
								else
									citent.m_Info = tr("Military Base on %2").arg(bd->FindParentPlanetInEnv()->Name());
								citent.m_Stance = PoliticalStance(person->EmpireId(), bd->EmpireId());
								citent.m_Distance = fieldSysLoc.Distance(personSysLoc);
								cities.insert(bd->Id(), citent);
							}
						}

						// Check ship sensor blips. There might be something we haven't listed.
						const DaVessel* ship = person->FindParentVesselInEnv();
						if(ship)
						{
							// Sensor blips on the client mostly only exists on the person's console.
							// If they don't have a console, the best we can hope for is the locked-on blip, which will be in the
							// ship's sensor blip array.
							for(AuSensorBlipList::const_iterator bi = m_SensorBlips.begin(); bi != m_SensorBlips.end(); bi++)
							{
								if((*bi)->IsCity())
								{
									// Cities
									if(cities.contains((*bi)->AtomId()))
									{
										// Add extra sensor blip information to the location description.
										cities[(*bi)->AtomId()].m_Info += ", " + (*bi)->HtmlDesc(system);
									}
									else
									{
										CityEntry citent;
										citent.m_Name = (*bi)->ScreenName();
										citent.m_Info = (*bi)->HtmlDesc(system);
										citent.m_Stance = (*bi)->PoliticalStance();
										citent.m_Distance = (*bi)->Range(ship->EnvLocation());
										cities.insert((*bi)->AtomId(), citent);
									}
								}
							}
						}
					}

					// List cities using the star map data.
					{
						AuIdList cityIds;
						QStringList cityNames;
						QList<quint32> cityEmpires;
						g_AuClient.StarMapListCitiesInSystem(orderSystemId, cityIds, cityNames, cityEmpires);
						if(!cityIds.isEmpty())
						{
							AuIdList::const_iterator ii = cityIds.cbegin();
							QStringList::const_iterator ni = cityNames.cbegin();
							QList<quint32>::const_iterator ei = cityEmpires.cbegin();
							while(ii != cityIds.cend())
							{
								if(!cities.contains(*ii))
								{
									CityEntry citent;
									citent.m_Name = *ni;
									citent.m_Info = tr("Star Map City");
									citent.m_Stance = PoliticalStance(refUnit->EmpireId(), *ei);
									citent.m_Distance = 0;
									cities.insert(*ii, citent);
								}
								ii++;
								ni++;
								ei++;
							}
						}
					}

					// Empire records know what cities belong to them.
					const DgData& gd = g_Au.LockDataForRead();
					{
						DgEmpire empire;
						empire.m_id = refUnit->EmpireId();
						if(gd.Empire(empire))
						{
							// Found my empire record.
							// List the cities in the order system.
							for(AuCityHash::const_iterator eci = empire.m_Cities->cbegin(); eci != empire.m_Cities->cend(); eci++)
								if((*eci).second.SystemId() == m_OrderSystemId)
									if(!cities.contains((*eci).first))
									{
										CityEntry citent;
										citent.m_Name = (*eci).second.CityName();
										citent.m_Info = tr("Empire City on %1").arg((*eci).second.PlanetName());
										citent.m_Stance = POL_Empire;
										citent.m_Distance = 0;
										cities.insert((*eci).first, citent);
									}
						}
					}
					g_Au.UnlockData();
				}
			}
			view->UnlockSceneAtom();

			// Fill cities table.
			m_Cities->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Cities->setRowCount(cities.count());
			QIcon icon(":data/City.png");
			int trow=0;
			for(QHash<AuId, CityEntry>::const_iterator si = cities.cbegin(); si != cities.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Cities->setItem(trow, col++, twi);

				switch((*si).m_Stance)
				{
				case POL_Friend:
					twi = new QTableWidgetItem(tr("Friend"));
					break;
				case POL_Neutral:
					twi = new QTableWidgetItem(tr("Neutral"));
					break;
				case POL_Enemy:
					twi = new QTableWidgetItem(tr("Enemy"));
					break;
				case POL_Empire:
					twi = new QTableWidgetItem(tr("Empire"));
					break;
				case POL_Vassal:
					twi = new QTableWidgetItem(tr("Overlord"));	// How vassal perceives target.
					break;
				}
				twi->setForeground(PoliticalStanceColor((*si).m_Stance, false));
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Cities->setItem(trow, col++, twi);

				twi = new AuTableWidgetDistanceItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Cities->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Info);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Cities->setItem(trow, col++, twi);

				m_Cities->setRowHeight(trow, 18);
			}
			m_Cities->sortByColumn(0, Qt::AscendingOrder);
			m_Cities->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Cities->resizeColumnsToContents();
		}

		// Wait until the GoToSystem order is selected before populating the system routes.
		// Do this every time because the person's current system may have changed.
		if(m_Order == AuMissionOrder::GoToSystemViaWormholes)
		{
			// Make a map of the system routes so they are sorted alphabetically.
			QHash<AuId, SystemEntry> systems;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}
					if(system)
					{
						// 'system' is the solar system of origin for routing purposes.
						const DaSector* sector = system->FindParentSector();	// The sector that contains 'system'.

						// Get a list of all the systems in the 3x3x3 cube of sectors centered on 'sector'.
						DqAtomLocArray aSystems;
						sector->ListSolarSystemLocations(aSystems);
						for(int il=0; il<aSystems.count(); il++)
						{
							DqAtomLoc& al = aSystems[il];

							const DaSector* sysSector = al.m_pA->FindParentSector();

							SystemEntry sysent;
							sysent.m_Name = al.m_pA->Name();
							sysent.m_Catalog = ((const DaSolarSystem*)al.m_pA)->CatalogName();
							sysent.m_GalaxyLoc = ((const DaSolarSystem*)al.m_pA)->GalaxyLoc();
							sysent.m_Sector = sysSector->Name() + ' ' + sysSector->Description();
							if(al.m_pA->Is(orderSystemId))
							{
								// My current/reference system.
								sysent.m_Distance = 0;
								sysent.m_Jumps = 0;
								sysent.m_Route = tr("Current Solar System");
							}
							else
							{
								// Not my current/reference system.
								AuPoint starloc = al.m_fLocation - system->Location();	// in parsecs (up to about 25 away)
								sysent.m_Distance = starloc.Length();

								// Check ship wormhole ability.
								const DaVessel* ship = person->FindParentVesselInEnv();
								bool negativeOk = ship ? ship->CanEnterNegativeWormhole() : false;
								bool neutralOk = ship ? ship->CanEnterNeutralWormhole() : false;

								DeWaypointRoute route;
								if(refUnit->AddWaypointRouteTo(route, orderSystemId, al.m_pA->Id(), AuPoint(0,0), negativeOk,
											neutralOk))
								{
									// There is a route to the system.
									Q_ASSERT(route.count() != 1);		// No 1-waypoint routes expected.
									sysent.m_Jumps = route.count()-1;	// First entry is always the origin system.

									// Show the route.
									for(DeWaypointRoute::const_iterator ri = route.cbegin()+1; ri != route.cend(); ri++)
									{
										// Locate the destination of the waypoint.
										if((*ri).Destination().IsNull())
											continue;
										for(int idl=0; idl<aSystems.count(); idl++)
										{
											DqAtomLoc& dl = aSystems[idl];
											if(dl.m_pA->Is((*ri).Destination()))
											{
												if(!sysent.m_Route.isEmpty())
													sysent.m_Route += " ";
												sysent.m_Route += g_Au.DoubleAngleRightSymbol() + " " + dl.m_pA->Name();
												break;
											}
										}
									}
								}
								else
								{
									// There is no route to the system.
									sysent.m_Jumps = 0;
									sysent.m_Route = "";
								}
							}

							if(m_ShowAll->isChecked() && !sysent.m_Route.isEmpty())
								systems.insert(al.m_pA->Id(), sysent);
							else if(m_ShowNamed->isChecked() && ((const DaSolarSystem*)al.m_pA)->HasName() &&
									!sysent.m_Route.isEmpty())
								systems.insert(al.m_pA->Id(), sysent);
						}

						// Intergalactic wormholes.
						if(system->HasBlackHole())
						{
							const AuWormholeArray& worms = system->LockWormholes();
							for(AuWormholeArray::const_iterator wi = worms.cbegin(); wi != worms.cend(); wi++)
							{
								if((*wi).Polarity() == AuWormhole::Intergalactic)
								{
									GALAXY destGal = DaSolarSystem::WormholeDestGalaxy(system->Galaxy(),
											(*wi).SectorVecToDestination());

									SystemEntry sysent;
									sysent.m_Name = ::GalaxyName(destGal);
									sysent.m_Catalog = "A'A'A'a";
									sysent.m_GalaxyLoc = AuPoint(0,0,0);
									sysent.m_Sector = sector->Description();
									sysent.m_Distance = 0;
									sysent.m_Jumps = 1;
									sysent.m_Route = tr("Intergalactic Wormhole");
									systems.insert(AuId::GalaxyId(destGal), sysent);
								}
								else if((*wi).Polarity() == AuWormhole::IntergalacticOneWay)
								{
									GALAXY destGal = DaSolarSystem::WormholeDestGalaxy(system->Galaxy(),
											(*wi).SectorVecToDestination());

									SystemEntry sysent;
									sysent.m_Name = ::GalaxyName(destGal);
									sysent.m_Catalog = "Unknown";
									sysent.m_GalaxyLoc = system->GalaxyLoc();
									sysent.m_Sector = sector->Description();
									sysent.m_Distance = 0;
									sysent.m_Jumps = 1;
									sysent.m_Route = tr("One Way Intergalactic Wormhole");
									systems.insert(AuId::GalaxyId(destGal), sysent);
								}
							}
							system->UnlockWormholes();
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill systems with wormhole routes table.
			m_Routes->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Routes->setRowCount(systems.count());
			QIcon icon(":data/SolarSystem.png");
			int trow=0;
			for(QHash<AuId, SystemEntry>::const_iterator si = systems.cbegin(); si != systems.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Catalog);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Sector);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				twi = new AuTableWidgetParsecItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				twi = new AuTableWidgetIntegerItem((*si).m_Jumps);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Route);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Routes->setItem(trow, col++, twi);

				m_Routes->setRowHeight(trow, 18);
			}
			m_Routes->sortByColumn(0, Qt::AscendingOrder);
			m_Routes->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Routes->resizeColumnsToContents();
		}

		// Wait until the GoToSystem order is selected before populating the system routes.
		// Do this every time because the person's current system may have changed.
		if(m_Order == AuMissionOrder::GoToSystemDirect)
		{
			// Make a hash of the system entries. Key is system id.
			QHash<AuId, SystemEntry> systems;
			AuView* view = auApp->Frame()->View();
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				// Determine which unit to reference.
				const DaUnit* refUnit = person->FindParentVesselInEnv();
				if(!refUnit)
					refUnit = person;

				if(refUnit)
				{
					const DaSolarSystem* system = 0;
					AuId orderSystemId(m_OrderSystemId);
					if(!orderSystemId.IsNull())
						system = dynamic_cast<const DaSolarSystem*>(person->Ether()->FindNormalAtom(orderSystemId));
					if(!system)
					{
						// Use person's current system as origin if a null or invalid system id was provided.
						system = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
						if(system)
							orderSystemId = system->Id();
					}
					if(system)
					{
						// 'system' is the solar system of origin for routing purposes.
						const DaSector* sector = system->FindParentSector();	// The sector that contains 'system'.

						// Get a list of all the systems in the 3x3x3 cube of sectors centered on 'sector'.
						DqAtomLocArray aSystems;
						sector->ListSolarSystemLocations(aSystems);
						for(int il=0; il<aSystems.count(); il++)
						{
							DqAtomLoc& al = aSystems[il];

							const DaSector* sysSector = al.m_pA->FindParentSector();

							SystemEntry sysent;
							sysent.m_Name = al.m_pA->Name();
							sysent.m_Catalog = ((const DaSolarSystem*)al.m_pA)->CatalogName();
							sysent.m_GalaxyLoc = ((const DaSolarSystem*)al.m_pA)->GalaxyLoc();
							sysent.m_Sector = sysSector->Name() + ' ' + sysSector->Description();
							if(al.m_pA->Is(orderSystemId))
							{
								// My current/reference system.
								sysent.m_Distance = 0;
								sysent.m_Jumps = 0;
								sysent.m_Route = tr("Current Solar System");
							}
							else
							{
								// Not my current/reference system.
								AuPoint starloc = al.m_fLocation - system->Location();	// in parsecs (up to about 25 away)
								sysent.m_Distance = starloc.Length();
								sysent.m_Jumps = 0;
								sysent.m_Route = "";
							}

							if(m_ShowAll->isChecked())
								systems.insert(al.m_pA->Id(), sysent);
							else if(m_ShowNamed->isChecked() && ((const DaSolarSystem*)al.m_pA)->HasName())
								systems.insert(al.m_pA->Id(), sysent);
						}
					}
				}
			}
			view->UnlockSceneAtom();

			// Fill systems table.
			m_Systems->setSortingEnabled(false);	// disabled while filling the table or it messes with the fill.
			m_Systems->setRowCount(systems.count());
			QIcon icon(":data/SolarSystem.png");
			int trow=0;
			for(QHash<AuId, SystemEntry>::const_iterator si = systems.cbegin(); si != systems.cend(); si++, trow++)
			{
				int col=0;
				QTableWidgetItem* twi = new QTableWidgetItem(icon, (*si).m_Name);
				twi->setData(Qt::UserRole, si.key().GetID());
				twi->setData(Qt::UserRole+1, (*si).m_GalaxyLoc.x);
				twi->setData(Qt::UserRole+2, (*si).m_GalaxyLoc.y);
				twi->setData(Qt::UserRole+3, (*si).m_GalaxyLoc.z);
				m_Systems->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Catalog);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Systems->setItem(trow, col++, twi);

				twi = new QTableWidgetItem((*si).m_Sector);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Systems->setItem(trow, col++, twi);

				twi = new AuTableWidgetParsecItem((*si).m_Distance);
				twi->setData(Qt::UserRole, si.key().GetID());
				m_Systems->setItem(trow, col++, twi);

				m_Systems->setRowHeight(trow, 18);
			}
			m_Systems->sortByColumn(0, Qt::AscendingOrder);
			m_Systems->setSortingEnabled(true);	// enabled after the table is filled otherwise it messes with the fill
			m_Systems->resizeColumnsToContents();
		}
	}

	EnableControls();
}

void AuMissionOrderWidget::CategoryChanged(COMMODITYFILTER filter)
{
	m_Category = filter;
	g_Au.Settings().writeEntry("MissionOrderWidget/Category", m_Category);
	EnableControls();
}

void AuMissionOrderWidget::BayChanged(int index)
{
	m_BayWhich = m_Bays->itemData(index).toInt();
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedTransfer(int index)
{
	m_CIDTransfer = (COMMODITYID)m_CommoditiesTransfer->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommodityTransfer", m_CIDTransfer);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedAtmo(int index)
{
	m_CIDAtmo = (COMMODITYID)m_CommoditiesAtmo->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommodityAtmo", m_CIDAtmo);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedBuy(int index)
{
	m_CIDBuy = (COMMODITYID)m_CommoditiesBuy->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/Commodity", m_CIDBuy);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedMin(int index)
{
	m_CIDMin = (COMMODITYID)m_CommoditiesMin->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommodityMin", m_CIDMin);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedPart(int index)
{
	m_CIDPart = (COMMODITYID)m_CommoditiesPart->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommodityPart", m_CIDPart);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedSea(int index)
{
	m_CIDSea = (COMMODITYID)m_CommoditiesSea->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommoditySea", m_CIDSea);
	EnableControls();
}

void AuMissionOrderWidget::CommodityChangedWell(int index)
{
	m_CIDWell = (COMMODITYID)m_CommoditiesWell->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/CommodityWell", m_CIDWell);
	EnableControls();
}

void AuMissionOrderWidget::PostureChanged(int index)
{
	m_Posture = (CITIZEN_POSTURE)m_Postures->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/Posture", m_Posture);
	EnableControls();
}

void AuMissionOrderWidget::RouteSystemChanged()
{
	m_RouteSystemId = AuId(0);
	m_RouteSystemName = "";
	QItemSelectionModel* ism = m_Routes->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Routes->item((*si).row(), 0);
				if(item)
				{
					m_RouteSystemId = AuId(item->data(Qt::UserRole).toInt());
					m_RouteSystemName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::SystemChanged()
{
	m_SystemId = AuId(0);
	m_SystemName = "";
	m_SystemGalaxyLoc = AuPoint(0,0);
	QItemSelectionModel* ism = m_Systems->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Systems->item((*si).row(), 0);
				if(item)
				{
					m_SystemId = AuId(item->data(Qt::UserRole).toInt());
					m_SystemName = item->text();
					m_SystemGalaxyLoc.x = item->data(Qt::UserRole+1).toDouble();
					m_SystemGalaxyLoc.y = item->data(Qt::UserRole+2).toDouble();
					m_SystemGalaxyLoc.z = item->data(Qt::UserRole+3).toDouble();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::AirportChanged()
{
	m_AirportId = AuId(0);
	m_AirportName = "";
	QItemSelectionModel* ism = m_Airports->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Airports->item((*si).row(), 0);
				if(item)
				{
					m_AirportId = AuId(item->data(Qt::UserRole).toInt());
					m_AirportName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::CityChanged()
{
	m_CityId = AuId(0);
	m_CityName = "";
	QItemSelectionModel* ism = m_Cities->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Cities->item((*si).row(), 0);
				if(item)
				{
					m_CityId = AuId(item->data(Qt::UserRole).toInt());
					m_CityName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::TargetChanged()
{
	m_TargetId = AuId(0);
	m_TargetName = "";
	QItemSelectionModel* ism = m_Targets->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Targets->item((*si).row(), 0);
				if(item)
				{
					m_TargetId = AuId(item->data(Qt::UserRole).toInt());
					m_TargetName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::HarvestPlanetChanged()
{
	m_HarvestPlanetId = AuId(0);
	m_HarvestPlanetName = "";
	QItemSelectionModel* ism = m_HarvestPlanetsAndStars->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_HarvestPlanetsAndStars->item((*si).row(), 0);
				if(item)
				{
					m_HarvestPlanetId = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestPlanetName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::HarvestPlanetChangedByM()
{
	m_HarvestPlanetIdByM = AuId(0);
	m_HarvestPlanetNameByM = "";
	QItemSelectionModel* ism = m_HarvestPlanetsAndStars->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_HarvestPlanetsAndStars->item((*si).row(), 0);
				if(item)
				{
					m_HarvestPlanetIdByM = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestPlanetNameByM = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::PlanetChanged()
{
	m_PlanetId = AuId(0);
	m_PlanetName = "";
	QItemSelectionModel* ism = m_PlanetsAndStars->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_PlanetsAndStars->item((*si).row(), 0);
				if(item)
				{
					m_PlanetId = AuId(item->data(Qt::UserRole).toInt());
					m_PlanetName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::PlanetChangedMin()
{
	m_HarvestMinId = AuId(0);
	m_HarvestMinName = "";
	QItemSelectionModel* ism = m_PlanetsMin->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_PlanetsMin->item((*si).row(), 0);
				if(item)
				{
					m_HarvestMinId = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestMinName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::PlanetChangedSea()
{
	m_HarvestSeaId = AuId(0);
	m_HarvestSeaName = "";
	QItemSelectionModel* ism = m_PlanetsSea->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_PlanetsSea->item((*si).row(), 0);
				if(item)
				{
					m_HarvestSeaId = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestSeaName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::PlanetChangedWell()
{
	m_HarvestWellId = AuId(0);
	m_HarvestWellName = "";
	QItemSelectionModel* ism = m_PlanetsWell->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_PlanetsWell->item((*si).row(), 0);
				if(item)
				{
					m_HarvestWellId = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestWellName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::PlanetChangedPart()
{
	m_HarvestPartId = AuId(0);
	m_HarvestPartName = "";
	QItemSelectionModel* ism = m_PlanetsPart->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_PlanetsPart->item((*si).row(), 0);
				if(item)
				{
					m_HarvestPartId = AuId(item->data(Qt::UserRole).toInt());
					m_HarvestPartName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::StationChanged()
{
	m_StationId = AuId(0);
	m_StationName = "";
	QItemSelectionModel* ism = m_Stations->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				// atom id and name are stored on column 0
				QTableWidgetItem* item = m_Stations->item((*si).row(), 0);
				if(item)
				{
					m_StationId = AuId(item->data(Qt::UserRole).toInt());
					m_StationName = item->text();
				}
				break;
			}
	}
	EnableControls();
}

void AuMissionOrderWidget::CountChanged(int count)
{
	m_Count = count;
	g_Au.Settings().writeEntry("MissionOrderWidget/Count", m_Count);
	EnableControls();
}

void AuMissionOrderWidget::ShipNameChanged(const QString& text)
{
	m_ShipName = text.trimmed();
	EnableControls();
}

void AuMissionOrderWidget::TransfereeChanged(const QString& text)
{
	m_Transferee = text;
	g_Au.Settings().writeEntry("MissionOrderWidget/Transferee", m_Transferee);
	EnableControls();
}

void AuMissionOrderWidget::MinQChanged(int v)
{
	m_MinQ = v;
	g_Au.Settings().writeEntry("MissionOrderWidget/MinQ", m_MinQ);
	EnableControls();
}

void AuMissionOrderWidget::PatrolTimeChanged(int minutes)
{
	m_Minutes = minutes;
	g_Au.Settings().writeEntry("MissionOrderWidget/Minutes", m_Minutes);
	EnableControls();
}

void AuMissionOrderWidget::PreBoardTimeChanged(int minutes)
{
	m_PreboardTime = minutes;
	g_Au.Settings().writeEntry("MissionOrderWidget/PreboardMinutes", m_PreboardTime);
	EnableControls();
}

void AuMissionOrderWidget::HoverAltitudeChanged(int altitude)
{
	m_HoverAlt = altitude;
	g_Au.Settings().writeEntry("MissionOrderWidget/HoverMeters", m_HoverAlt);
	EnableControls();
}

void AuMissionOrderWidget::LongLatChanged(const QString& text)
{
	m_PlanetLongLat = text;
	EnableControls();
}

QSize AuMissionOrderWidget::sizeHint() const
{
	return QSize(400,200);
}

void AuMissionOrderWidget::MarginBuyChanged(int margin)
{
	m_MarginBuyPercent = margin;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginBuyPercent", m_MarginBuyPercent);
	m_BuyMarginLabel->setText(tr("Pay No More Than %1% of Nominal Price").arg(m_MarginBuyPercent));
	EnableControls();
}

void AuMissionOrderWidget::MarginSellChanged(int margin)
{
	m_MarginSellPercent = margin;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginSellPercent", m_MarginSellPercent);
	m_SellMarginLabel->setText(tr("Accept No Less Than %1% of Nominal Price").arg(m_MarginSellPercent));
	EnableControls();
}

void AuMissionOrderWidget::CargoBuyPercentChanged(int percent)
{
	m_AmountBuyPercent = percent;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountBuyPercent", m_AmountBuyPercent);
	m_BuyCargoLabel->setText(tr("Fill Up To %1% of Space").arg(m_AmountBuyPercent));
	m_CatBuyPercentLabel->setText(tr("Fill Up To %1% of Space").arg(m_AmountBuyPercent));
	m_TransferCargoFromLabel->setText(tr("Transfer Up To %1% of Their Cargo").arg(m_AmountBuyPercent));
	m_CatTransferFromPercentLabel->setText(tr("Transfer Up To %1% of Their Cargo").arg(m_AmountBuyPercent));
	EnableControls();
}

void AuMissionOrderWidget::CargoSellPercentChanged(int percent)
{
	m_AmountSellPercent = percent;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountSellPercent", m_AmountSellPercent);
	m_SellCargoLabel->setText(tr("Sell Up To %1% of Space").arg(m_AmountSellPercent));
	m_CatSellPercentLabel->setText(tr("Sell Up To %1% of Space").arg(m_AmountSellPercent));
	m_TransferCargoToLabel->setText(tr("Transfer Up To %1% of Our Cargo").arg(m_AmountSellPercent));
	m_CatTransferToPercentLabel->setText(tr("Transfer Up To %1% of Our Cargo").arg(m_AmountSellPercent));
	EnableControls();
}

bool AuMissionOrderWidget::IsLongLatStringValid() const
{
	return AuGlobeLL::IsLongLatStringValid(m_PlanetLongLat);
}

void AuMissionOrderWidget::GoToCoordChanged(const QString& text)
{
	m_GoTo = AuPoint(text);
	g_Au.Settings().writeEntry("MissionOrderWidget/GoToX", m_GoTo.x);
	g_Au.Settings().writeEntry("MissionOrderWidget/GoToY", m_GoTo.y);
	g_Au.Settings().writeEntry("MissionOrderWidget/GoToZ", m_GoTo.z);
	EnableControls();
}

void AuMissionOrderWidget::ApplyQSelected()
{
	if(m_ApplyMinQRadio->isChecked())
		m_ApplyQ = 2;
	else if(m_ApplySystemQRadio->isChecked())
		m_ApplyQ = 1;
	else
		m_ApplyQ = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/ApplyQ", m_ApplyQ);

	EnableControls();
}

void AuMissionOrderWidget::GoToSelected()
{
	if(m_GoToSystem->isChecked())
	{
		m_GoToRelative = 2;
		m_GoTo = m_SystemCoord;
	}
	else if(m_GoToSector->isChecked())
	{
		m_GoToRelative = 1;
		m_GoTo = m_SectorCoord;
	}
	else
	{
		m_GoToRelative = 0;
		m_GoTo = m_GalaxyCoord;
	}
	g_Au.Settings().writeEntry("MissionOrderWidget/GoToRelative", m_GoToRelative);

	QString gotoCoord = QString("%1, %2, %3")
			.arg(m_GoTo.x, 0,'f',4)
			.arg(m_GoTo.y, 0,'f',4)
			.arg(m_GoTo.z, 0,'f',4);
	m_GoToCoord->setText(gotoCoord);

	EnableControls();
}

void AuMissionOrderWidget::LightHeadlightOffSelected()
{
	m_LightStateHeadlight = false;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightHeadlight", m_LightStateHeadlight);
	EnableControls();
}

void AuMissionOrderWidget::LightHeadlightOnSelected()
{
	m_LightStateHeadlight = true;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightHeadlight", m_LightStateHeadlight);
	EnableControls();
}

void AuMissionOrderWidget::LightNavLightOffSelected()
{
	m_LightStateNavLight = false;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightNavLight", m_LightStateNavLight);
	EnableControls();
}

void AuMissionOrderWidget::LightNavLightOnSelected()
{
	m_LightStateNavLight = true;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightNavLight", m_LightStateNavLight);
	EnableControls();
}

void AuMissionOrderWidget::LightRotatingBeaconOffSelected()
{
	m_LightStateRotatingBeacon = false;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightRotatingBeacon", m_LightStateRotatingBeacon);
	EnableControls();
}

void AuMissionOrderWidget::LightRotatingBeaconOnSelected()
{
	m_LightStateRotatingBeacon = true;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightRotatingBeacon", m_LightStateRotatingBeacon);
	EnableControls();
}

void AuMissionOrderWidget::LightStrobeOffSelected()
{
	m_LightStateStrobe = false;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightStrobe", m_LightStateStrobe);
	EnableControls();
}

void AuMissionOrderWidget::LightStrobeOnSelected()
{
	m_LightStateStrobe = true;
	g_Au.Settings().writeEntry("MissionOrderWidget/LightStrobe", m_LightStateStrobe);
	EnableControls();
}

void AuMissionOrderWidget::TransferNamedShip()
{
	m_TransferTarget = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferTarget", m_TransferTarget);
	EnableControls();
}

void AuMissionOrderWidget::TransferStation()
{
	m_TransferTarget = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferTarget", m_TransferTarget);
	EnableControls();
}

void AuMissionOrderWidget::AmountBuy0Selected()
{
	m_AmountBuyType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountBuyType", m_AmountBuyType);
	EnableControls();
}

void AuMissionOrderWidget::AmountBuy1Selected()
{
	m_AmountBuyType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountBuyType", m_AmountBuyType);
	EnableControls();
}

void AuMissionOrderWidget::AmountSell0Selected()
{
	m_AmountSellType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountSellType", m_AmountSellType);
	EnableControls();
}

void AuMissionOrderWidget::AmountSell1Selected()
{
	m_AmountSellType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountSellType", m_AmountSellType);
	EnableControls();
}

void AuMissionOrderWidget::CatBuy0Selected()
{
	m_CatBuyType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategoryBuyType", m_CatBuyType);
	EnableControls();
}

void AuMissionOrderWidget::CatBuy1Selected()
{
	m_CatBuyType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategoryBuyType", m_CatBuyType);
	EnableControls();
}

void AuMissionOrderWidget::CatBuy2Selected()
{
	m_CatBuyType = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategoryBuyType", m_CatBuyType);
	EnableControls();
}

void AuMissionOrderWidget::CatSell0Selected()
{
	m_CatSellType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategorySellType", m_CatSellType);
	EnableControls();
}

void AuMissionOrderWidget::CatSell1Selected()
{
	m_CatSellType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategorySellType", m_CatSellType);
	EnableControls();
}

void AuMissionOrderWidget::CatSell2Selected()
{
	m_CatSellType = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/CategorySellType", m_CatSellType);
	EnableControls();
}

void AuMissionOrderWidget::MarginBuy0Selected()
{
	m_MarginBuyType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginBuyType", m_MarginBuyType);
	EnableControls();
}

void AuMissionOrderWidget::MarginBuy1Selected()
{
	m_MarginBuyType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginBuyType", m_MarginBuyType);
	EnableControls();
}

void AuMissionOrderWidget::MarginBuy2Selected()
{
	m_MarginBuyType = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginBuyType", m_MarginBuyType);
	EnableControls();
}

void AuMissionOrderWidget::MarginSell0Selected()
{
	m_MarginSellType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginSellType", m_MarginSellType);
	EnableControls();
}

void AuMissionOrderWidget::MarginSell1Selected()
{
	m_MarginSellType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginSellType", m_MarginSellType);
	EnableControls();
}

void AuMissionOrderWidget::MarginSell2Selected()
{
	m_MarginSellType = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginSellType", m_MarginSellType);
	EnableControls();
}

void AuMissionOrderWidget::PilotUpPolicyChanged(int index)
{
	if(index < 0)
		return;

	m_PilotUpPolicy = (PILOT_UP_POLICY)m_PilotUpPolicies->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/PilotUpPolicy", m_PilotUpPolicy);
	EnableControls();
}

void AuMissionOrderWidget::ShieldPolicyChanged(int index)
{
	if(index < 0)
		return;

	m_ShieldPolicy = (SHIELD_POLICY)m_ShieldPolicies->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/ShieldPolicy", m_ShieldPolicy);
	EnableControls();
}

void AuMissionOrderWidget::ShieldFuelPolicyChanged(int index)
{
	if(index < 0)
		return;

	m_ShieldFuelPolicy = (SHIELDFUEL_POLICY)m_ShieldFuelPolicies->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/ShieldFuelPolicy", m_ShieldFuelPolicy);
	EnableControls();
}

void AuMissionOrderWidget::SplitterMoved(int /*pos*/, int /*index*/)
{
	// Save splitter state.
	QByteArray splitterState = m_Splitter->saveState();
	g_Au.Settings().writeEntry("MissionOrderWidget/Splitter", splitterState);
}

void AuMissionOrderWidget::CargoBuyCostChanged(const QString& costText)
{
	m_MarginBuyCost = QVariant(costText).toUInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginBuyCost", m_MarginBuyCost);
	EnableControls();
}

void AuMissionOrderWidget::CargoSellCostChanged(const QString& costText)
{
	m_MarginSellCost = QVariant(costText).toUInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/MarginSellCost", m_MarginSellCost);
	EnableControls();
}

void AuMissionOrderWidget::CargoBuyAmountChanged(const QString& costText)
{
	m_AmountBuyUnits = QVariant(costText).toUInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountBuyUnits", m_AmountBuyUnits);
	EnableControls();
}

void AuMissionOrderWidget::CargoSellAmountChanged(const QString& costText)
{
	m_AmountSellUnits = QVariant(costText).toUInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/AmountSellUnits", m_AmountSellUnits);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyFromPercentChanged(int percent)
{
	m_TransferMoneyFromPercentLabel->setText(QString("%1%").arg(percent));
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyFromPercent", percent);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyToPercentChanged(int percent)
{
	m_TransferMoneyToPercentLabel->setText(QString("%1%").arg(percent));
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyToPercent", percent);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyFromPercent()
{
	m_TransferMoneyFromType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyFromType", m_TransferMoneyFromType);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyFromAmount()
{
	m_TransferMoneyFromType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyFromType", m_TransferMoneyFromType);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyToPercent()
{
	m_TransferMoneyToType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyToType", m_TransferMoneyToType);
	EnableControls();
}

void AuMissionOrderWidget::TransferMoneyToAmount()
{
	m_TransferMoneyToType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferMoneyToType", m_TransferMoneyToType);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromPercentChanged(int percent)
{
	m_TransferPeopleFromPercentLabel->setText(QString("%1%").arg(percent));
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromPercent", percent);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToPercentChanged(int percent)
{
	m_TransferPeopleToPercentLabel->setText(QString("%1%").arg(percent));
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToPercent", percent);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromPercent()
{
	m_TransferPeopleFromType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromType", m_TransferPeopleFromType);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromAmount()
{
	m_TransferPeopleFromType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromType", m_TransferPeopleFromType);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToPercent()
{
	m_TransferPeopleToType = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToType", m_TransferPeopleToType);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToAmount()
{
	m_TransferPeopleToType = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToType", m_TransferPeopleToType);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromOfficers()
{
	m_TransferPeopleFromBerth = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromBerth", m_TransferPeopleFromBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromCrew()
{
	m_TransferPeopleFromBerth = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromBerth", m_TransferPeopleFromBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromTroops()
{
	m_TransferPeopleFromBerth = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromBerth", m_TransferPeopleFromBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleFromPassengers()
{
	m_TransferPeopleFromBerth = 3;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleFromBerth", m_TransferPeopleFromBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToOfficers()
{
	m_TransferPeopleToBerth = 0;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToBerth", m_TransferPeopleToBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToCrew()
{
	m_TransferPeopleToBerth = 1;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToBerth", m_TransferPeopleToBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToTroops()
{
	m_TransferPeopleToBerth = 2;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToBerth", m_TransferPeopleToBerth);
	EnableControls();
}

void AuMissionOrderWidget::TransferPeopleToPassengers()
{
	m_TransferPeopleToBerth = 3;
	g_Au.Settings().writeEntry("MissionOrderWidget/TransferPeopleToBerth", m_TransferPeopleToBerth);
	EnableControls();
}

void AuMissionOrderWidget::HarvestFuelThresholdChanged(int t)
{
	m_HarvestFuelThreshold = t;
	g_Au.Settings().writeEntry("MissionOrderWidget/HarvestFuelThreshold", m_HarvestFuelThreshold);
	m_HarvestFuelThresholdLabel->setText(tr("Stop harvesting if fuel level is %1% or below.").arg(m_HarvestFuelThreshold));
	EnableControls();
}

void AuMissionOrderWidget::HarvestFuelThresholdByMChanged(int t)
{
	m_HarvestFuelThresholdByM = t;
	g_Au.Settings().writeEntry("MissionOrderWidget/HarvestFuelThresholdByM", m_HarvestFuelThresholdByM);
	m_HarvestFuelThresholdLabelByM->setText(tr("Stop harvesting if fuel level is %1% or below.").arg(m_HarvestFuelThresholdByM));
	EnableControls();
}

void AuMissionOrderWidget::HarvestOps(AuHarvestOps& ops) const
{
	ops.clear();
	if(m_Order == AuMissionOrder::HarvestOperationByBay)
	{
		// The key here is the bay which id.
		for(QMap<quint16, AuHarvestOpWidget*>::const_iterator hi = m_HarvestByBay.cbegin(); hi != m_HarvestByBay.cend(); hi++)
		{
			if((*hi)->isHidden())
				continue;

			AuHarvestOp op;
			(*hi)->HarvestOp(op);

			ops.insert(hi.key(), op);
		}
	}
	else if(m_Order == AuMissionOrder::HarvestOperationByModule)
	{
		// The key here is the module cid.
		for(QMap<quint16, AuHarvestOpWidget*>::const_iterator hi = m_HarvestByModule.cbegin(); hi != m_HarvestByModule.cend(); hi++)
		{
			if((*hi)->isHidden())
				continue;

			AuHarvestOp op;
			(*hi)->HarvestOp(op);

			ops.insert(hi.key(), op);
		}
	}
}

void AuMissionOrderWidget::ReportChannelChanged(const QString& text)
{
	m_ReportChannel = text;
	g_Au.Settings().writeEntry("MissionOrderWidget/ReportChannel", m_ReportChannel);
	EnableControls();
}

void AuMissionOrderWidget::ReportRangeChanged(int index)
{
	m_ReportRange = (AuCommStdChannel::RANGE)m_ReportRangeCombo->itemData(index).toInt();
	g_Au.Settings().writeEntry("MissionOrderWidget/ReportRange", m_ReportRange);
	EnableControls();
}

