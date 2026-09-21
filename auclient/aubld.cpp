//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubld.cpp,v 1.7 2022/02/19 18:09:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubld.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "aubldarena.h"
#include "aubldbank.h"
#include "aubldbroker.h"
#include "aubldbuilding.h"
#include "aubldcapitol.h"
#include "aubldconstruct.h"
#include "aublddesignstudio.h"
#include "aubldfarm.h"
#include "aubldhangar.h"
#include "aubldhq.h"
#include "aubldinfo.h"
#include "aubldmanufacture.h"
#include "aubldrepairshop.h"
#include "aubldshed.h"
#include "aubldspacecraftfactory.h"
#include "aubldstoragetank.h"
#include "aubldstore.h"
#include "aubldwarehouse.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include <dgdata.h>
#include <QIcon>
#include <QStackedLayout>
#include <QTabBar>
#include <QTimer>

void AuBuildingDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleBuildingUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBld::AuBld(QWidget* parent) : QWidget(parent)
{
	m_Mode = Init;
	m_timer = 0;

	m_Pages = new QStackedLayout();

	// Create a tab bar for each MODE we will need.
	// Every mode has more than one page therefore every mode needs a tab bar.

	// Arena MODE
	arenatabs = new QTabBar(this);
	arenatabs->hide();
	connect(arenatabs, SIGNAL(currentChanged(int)), SLOT(ArenaTabChanged(int)));

	// Bank MODE
	banktabs = new QTabBar(this);
	banktabs->hide();
	connect(banktabs, SIGNAL(currentChanged(int)), SLOT(BankTabChanged(int)));

	// Broker MODE
	brokertabs = new QTabBar(this);
	brokertabs->hide();
	connect(brokertabs, SIGNAL(currentChanged(int)), SLOT(BrokerTabChanged(int)));

	// Building MODE
	bldtabs = new QTabBar(this);
	bldtabs->hide();
	connect(bldtabs, SIGNAL(currentChanged(int)), SLOT(BuildingTabChanged(int)));

	// Capitol MODE
	captabs = new QTabBar(this);
	captabs->hide();
	connect(captabs, SIGNAL(currentChanged(int)), SLOT(CapitolTabChanged(int)));

	// Construction MODE
	contabs = new QTabBar(this);
	contabs->hide();
	connect(contabs, SIGNAL(currentChanged(int)), SLOT(ConstructionTabChanged(int)));

	// DesignStudio MODE
	designtabs = new QTabBar(this);
	designtabs->hide();
	connect(designtabs, SIGNAL(currentChanged(int)), SLOT(DesignTabChanged(int)));

	// Farm MODE
	farmtabs = new QTabBar(this);
	farmtabs->hide();
	connect(farmtabs, SIGNAL(currentChanged(int)), SLOT(FarmTabChanged(int)));

	// Hangar MODE
	hangartabs = new QTabBar(this);
	hangartabs->hide();
	connect(hangartabs, SIGNAL(currentChanged(int)), SLOT(HangarTabChanged(int)));

	// HQ MODE
	hqtabs = new QTabBar(this);
	hqtabs->hide();
	connect(hqtabs, SIGNAL(currentChanged(int)), SLOT(HQTabChanged(int)));

	// Manufacture MODE
	mfgtabs = new QTabBar(this);
	mfgtabs->hide();
	connect(mfgtabs, SIGNAL(currentChanged(int)), SLOT(ManufacturingTabChanged(int)));

	// NoStore MODE
	nostoretabs = new QTabBar(this);
	nostoretabs->hide();
	connect(nostoretabs, SIGNAL(currentChanged(int)), SLOT(NoStoreTabChanged(int)));

	// RepairShop MODE
	reptabs = new QTabBar(this);
	reptabs->hide();
	connect(reptabs, SIGNAL(currentChanged(int)), SLOT(RepairShopTabChanged(int)));

	// Shed MODE
	shedtabs = new QTabBar(this);
	shedtabs->hide();
	connect(shedtabs, SIGNAL(currentChanged(int)), SLOT(ShedTabChanged(int)));

	// SpacecraftFactory MODE
	scfacttabs = new QTabBar(this);
	scfacttabs->hide();
	connect(scfacttabs, SIGNAL(currentChanged(int)), SLOT(SpacecraftFactoryTabChanged(int)));

	// StorageTank MODE
	stanktabs = new QTabBar(this);
	stanktabs->hide();
	connect(stanktabs, SIGNAL(currentChanged(int)), SLOT(StorageTankTabChanged(int)));

	// StreetLight MODE
	streetlighttabs = new QTabBar(this);
	streetlighttabs->hide();
	connect(streetlighttabs, SIGNAL(currentChanged(int)), SLOT(StreetLightTabChanged(int)));

	// Warehouse MODE
	waretabs = new QTabBar(this);
	waretabs->hide();
	connect(waretabs, SIGNAL(currentChanged(int)), SLOT(WarehouseTabChanged(int)));

	// All the tab bars are placed in the same spot. The one we need based on MODE is the only one visible at any given time.
	QGridLayout* lay = new QGridLayout;
	lay->addWidget(arenatabs, 0,0);
	lay->addWidget(banktabs, 0,0);
	lay->addWidget(bldtabs, 0,0);
	lay->addWidget(brokertabs, 0,0);
	lay->addWidget(captabs, 0,0);
	lay->addWidget(contabs, 0,0);
	lay->addWidget(designtabs, 0,0);
	lay->addWidget(farmtabs, 0,0);
	lay->addWidget(hangartabs, 0,0);
	lay->addWidget(hqtabs, 0,0);
	lay->addWidget(mfgtabs, 0,0);
	lay->addWidget(nostoretabs, 0,0);
	lay->addWidget(reptabs, 0,0);
	lay->addWidget(scfacttabs, 0,0);
	lay->addWidget(shedtabs, 0,0);
	lay->addWidget(stanktabs, 0,0);
	lay->addWidget(streetlighttabs, 0,0);
	lay->addWidget(waretabs, 0,0);
	lay->addLayout(m_Pages, 1,0);	// The stack of pages that will be accessed by the tabs.
	lay->setMargin(2);
	lay->setSpacing(0);
	setLayout(lay);

	// Create one of each kind of page needed by the MODEs.
	// All pages are not needed by all MODEs.
	// Each page is added to the particular tab bars that need them, according to their MODE.

	// NOTE: The order these pages are added affects their page index. This is hard coded into the NNNTabChanged functions.

	// Construction page is always alone because Bd buildings cannot do anything else while under construction.
	contabs->addTab(QIcon(":data/ConstructionSign.png"), tr("Construction"));
	AuBldConstruct* laborCon = new AuBldConstruct(this);
	m_ConstructionPage = m_Pages->addWidget(laborCon);

	// Arena tab
	arenatabs->addTab(QIcon(":data/bArena.png"), tr("Arena"));
	AuBldArena* laborArena = new AuBldArena(this);
	m_ArenaPage = m_Pages->addWidget(laborArena);

	// Bank tab
	banktabs->addTab(QIcon(":data/bBank.png"), tr("Bank"));
	AuBldBank* laborBank = new AuBldBank(this);
	m_BankPage = m_Pages->addWidget(laborBank);

	// Broker tab
	brokertabs->addTab(QIcon(":data/bBroker.png"), tr("Broker"));
	m_BldBroker = new AuBldBroker(this);
	m_BrokerPage = m_Pages->addWidget(m_BldBroker);

	// Capitol tab
	captabs->addTab(QIcon(":data/bCapitol.png"), tr("Capitol"));
	m_BldCapitol = new AuBldCapitol(this);
	m_CapitolPage = m_Pages->addWidget(m_BldCapitol);

	// Design Studio tab
	designtabs->addTab(QIcon(":data/bDesignStudio.png"), tr("Design Studio"));
	AuBldDesignStudio* laborDesignStudio = new AuBldDesignStudio(this);
	m_DesignPage = m_Pages->addWidget(laborDesignStudio);

	// Farm tab
	farmtabs->addTab(QIcon(":data/bFarm.png"), tr("Farm"));
	AuBldFarm* laborFarm = new AuBldFarm(this);
	m_FarmPage = m_Pages->addWidget(laborFarm);

	// Hangar tab
	hangartabs->addTab(QIcon(":data/bHangar.png"), tr("Hangar"));
	AuBldHangar* laborHangar = new AuBldHangar(this);
	m_HangarPage = m_Pages->addWidget(laborHangar);

	// HQ tab
	hqtabs->addTab(QIcon(":data/bBaseHeadquarters.png"), tr("Military HQ"));
	AuBldHQ* laborHQ = new AuBldHQ(this);
	m_HQPage = m_Pages->addWidget(laborHQ);

	// Repair Shop tab
	reptabs->addTab(QIcon(":data/bAirportRepairShop.png"), tr("Repair Shop"));
	AuBldRepairShop* laborRepairShop = new AuBldRepairShop(this);
	m_RepairShopPage = m_Pages->addWidget(laborRepairShop);

	// Shed tab
	shedtabs->addTab(QIcon(":data/bShed.png"), tr("Shed"));
	AuBldShed* laborShed = new AuBldShed(this);
	m_ShedPage = m_Pages->addWidget(laborShed);

	// SpacecraftFactory tab
	scfacttabs->addTab(QIcon(":data/bSpacecraftFactory.png"), tr("Spacecraft Factory"));
	AuBldSpacecraftFactory* laborScFact = new AuBldSpacecraftFactory(this);
	m_ScFactPage = m_Pages->addWidget(laborScFact);

	// StorageTank tab
	stanktabs->addTab(QIcon(":data/bStorageTank.png"), tr("Storage Tank"));
	AuBldStorageTank* laborStorageTank = new AuBldStorageTank(this);
	m_StorageTankPage = m_Pages->addWidget(laborStorageTank);

	// Warehouse tab
	waretabs->addTab(QIcon(":data/bWarehouse.png"), tr("Warehouse"));
	AuBldWarehouse* laborWarehouse = new AuBldWarehouse(this);
	m_WarehousePage = m_Pages->addWidget(laborWarehouse);

	// Manufacture tab
	// Not needed by buildings that do not manufacture anything.
	banktabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));	// bullion
	mfgtabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));	// most commodity production
	reptabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));	// modules
	farmtabs->addTab(QIcon(":data/HammerAndSickle.png"), tr("Manufacture"));	// food
	AuBldManufacture* laborMfg = new AuBldManufacture(this);
	m_ManufacturePage = m_Pages->addWidget(laborMfg);

	// Store tab provides access to the store at most buildings that have one.
	arenatabs->addTab(QIcon(":data/City.png"), tr("Store"));
	bldtabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	brokertabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	designtabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	farmtabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	mfgtabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	reptabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	scfacttabs->addTab(QIcon(":data/CommTrade.png"), tr("Store"));
	AuBldStore* laborStore = new AuBldStore(this);
	m_StorePage = m_Pages->addWidget(laborStore);

	// Building page provides information about the building itself. It is useful at all buildings.
	arenatabs->addTab(QIcon(":data/City.png"), tr("Building"));
	banktabs->addTab(QIcon(":data/City.png"), tr("Building"));
	bldtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	brokertabs->addTab(QIcon(":data/City.png"), tr("Building"));
	captabs->addTab(QIcon(":data/City.png"), tr("Building"));
	contabs->addTab(QIcon(":data/City.png"), tr("Building"));
	designtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	farmtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	hangartabs->addTab(QIcon(":data/City.png"), tr("Building"));
	hqtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	mfgtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	nostoretabs->addTab(QIcon(":data/City.png"), tr("Building"));
	reptabs->addTab(QIcon(":data/City.png"), tr("Building"));
	scfacttabs->addTab(QIcon(":data/City.png"), tr("Building"));
	shedtabs->addTab(QIcon(":data/City.png"), tr("Building"));
	stanktabs->addTab(QIcon(":data/City.png"), tr("Building"));
	streetlighttabs->addTab(QIcon(":data/City.png"), tr("Building"));
	waretabs->addTab(QIcon(":data/City.png"), tr("Building"));
	AuBldBuilding* bldBuilding = new AuBldBuilding(this);
	m_BuildingPage = m_Pages->addWidget(bldBuilding);

	// Info page provides information about the building window. It is useful at all buildings.
	arenatabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	banktabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	bldtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	brokertabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	captabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	contabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	designtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	farmtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	hangartabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	hqtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	mfgtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	nostoretabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	reptabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	scfacttabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	shedtabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	stanktabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	streetlighttabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	waretabs->addTab(QIcon(":data/OrderRead.png"), tr("Info"));
	AuBldInfo* bldInfo = new AuBldInfo(false, this);
	m_InfoPage = m_Pages->addWidget(bldInfo);
}

AuBld::~AuBld()
{
}

void AuBld::showEvent(QShowEvent* event)
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

void AuBld::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();
}


QSize AuBld::sizeHint() const
{
	return QSize(700,512);
}

void AuBld::BrokerShipment(AuShipment shipment)
{
	m_BldBroker->Shipment(shipment);
	m_BldCapitol->Shipment(shipment);
}

void AuBld::BrokerShipmentsClear()
{
	m_BldBroker->ShipmentsClear();
	m_BldCapitol->ShipmentsClear();
}

void AuBld::BrokerTradeAgreement(AuTradeAgreement ta)
{
	m_BldBroker->TradeAgreement(ta);
	m_BldCapitol->TradeAgreement(ta);
}

void AuBld::BrokerTradeAgreementsClear()
{
	m_BldBroker->TradeAgreementsClear();
	m_BldCapitol->TradeAgreementsClear();
}

void AuBld::BuildingTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::NoStoreTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::ManufacturingTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::ArenaTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ArenaPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::BankTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BankPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::BrokerTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BrokerPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::ConstructionTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ConstructionPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::HangarTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_HangarPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::HQTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_HQPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::CapitolTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_CapitolPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::DesignTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_DesignPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::FarmTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_FarmPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 4:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::ShedTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ShedPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::SpacecraftFactoryTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_ScFactPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::StorageTankTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_StorageTankPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::StreetLightTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::WarehouseTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_WarehousePage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::RepairShopTabChanged(int idx)
{
	switch(idx)
	{
	case 0:
		m_Pages->setCurrentIndex(m_RepairShopPage);
		break;
	case 1:
		m_Pages->setCurrentIndex(m_ManufacturePage);
		break;
	case 2:
		m_Pages->setCurrentIndex(m_StorePage);
		break;
	case 3:
		m_Pages->setCurrentIndex(m_BuildingPage);
		break;
	case 4:
		m_Pages->setCurrentIndex(m_InfoPage);
		break;
	}
}

void AuBld::UpdateContents()
{
	// Determine what mode I should be in based on the terrain location of the scene atom.
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleBuildingUI);
	QString dockTitleText;
	MODE modeNeeded = Info;	// Set to the mode needed by the scene atom. Info is always available.
	AuDockWidget* dockWidget = qobject_cast<AuDockWidget*>(parentWidget());
	if(dockWidget)
	{
		// This will change to show the industry name at the scene atom's location.
		if(control.isEmpty())
			dockTitleText = dockWidget->OriginalTitle();
		else
			dockTitleText = dockWidget->OriginalTitle() + " (" + control + ")";
	}

	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		// Scene atom is in a developed terrain tile.
		// Find the terrain patch so I cen see if there are any manufacturing processes in this tile.
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(person->FindParentPlanetInEnv());
		if(llworld)
		{
			if(person->IsAtGeo())
			{
				if(dockWidget)
				{
					const DgData& gd = g_Au.LockDataForRead();
					dockTitleText += QString(" %1").arg(gd.IndustryName(person->InIndustry()));
					g_Au.UnlockData();
					dockWidget->SetTitle(dockTitleText);
				}

				DeConstructionProcess cp;
				if(llworld->GeoUnderConstruction(person->InIndustryLL(), cp))
					modeNeeded = Construction;	// The geo is under construction.
			}
			else
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(llworld->FindIdRecursive(person->BdId()));
				if(bd)
				{
					if(dockWidget)
					{
						const DgData& gd = g_Au.LockDataForRead();
						dockTitleText += QString(" %1")
							.arg(gd.IndustryName(bd->IndustryId()));
						g_Au.UnlockData();
						dockWidget->SetTitle(dockTitleText);
					}

					switch(bd->IndustryId())
					{
					case IID_AirportRepairShop:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = RepairShop;
						break;
					case IID_Arena:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Arena;
						break;
					case IID_Bank:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Bank;
						break;
					case IID_BaseHeadquarters:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = HQ;
						break;
					case IID_Broker:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Broker;
						break;
					case IID_Capitol:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Capitol;
						break;
					case IID_DesignStudio:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = DesignStudio;
						break;
					case IID_Farm:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Farm;
						break;
					case IID_Hangar:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Hangar;
						break;
					case IID_Shed:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Shed;
						break;
					case IID_SpacecraftFactory:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = SpacecraftFactory;
						break;
					case IID_StorageTank:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = StorageTank;
						break;
					case IID_StreetLight:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = StreetLight;
						break;
					case IID_Warehouse:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else
							modeNeeded = Warehouse;
						break;
					default:
						if(bd->IsUnderConstruction())
							modeNeeded = Construction;
						else if(DbIndustry::DoesManufacture(bd->IndustryId()))
							modeNeeded = Manufacture;
						else if(bd->Stores())
							modeNeeded = Building;	// Building has a store.
						else
							modeNeeded = NoStore;	// Building does not have a store.
						break;
					}
				}
			}
		}
	}
	else if(dockWidget)
		dockWidget->SetTitle(dockTitleText);
	pView->UnlockSceneAtom();

	// Do nothing if the mode hasn't changed.
	if(modeNeeded == m_Mode)
		return;

	int selPage = m_Pages->currentIndex();
	int selTabIndex = 0;
	QTabBar* showTabs = 0;
	switch(modeNeeded)
	{
	case Init:
	case Info:
		selPage = m_InfoPage;
		break;
	case Arena:
		showTabs = arenatabs;
		if(selPage == m_StorePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ArenaPage;
			selTabIndex = 0;
		}
		break;
	case Bank:
		showTabs = banktabs;
		if(selPage == m_ManufacturePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_BankPage;
			selTabIndex = 0;
		}
		break;
	case Broker:
		showTabs = brokertabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_StorePage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_BrokerPage;
			selTabIndex = 0;
		}
		break;
	case Building:
		showTabs = bldtabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_StorePage;
			selTabIndex = 0;
		}
		break;
	case Capitol:
		showTabs = captabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_CapitolPage;
			selTabIndex = 0;
		}
		break;
	case Construction:
		selPage = m_ConstructionPage;
		showTabs = contabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ConstructionPage;
			selTabIndex = 0;
		}
		break;
	case DesignStudio:
		showTabs = designtabs;
		if(selPage == m_StorePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_DesignPage;
			selTabIndex = 0;
		}
		break;
	case Farm:
		showTabs = farmtabs;
		if(selPage == m_ManufacturePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_StorePage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 3;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_FarmPage;
			selTabIndex = 0;
		}
		break;
	case Hangar:
		showTabs = hangartabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_HangarPage;
			selTabIndex = 0;
		}
		break;
	case HQ:
		showTabs = hqtabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_HQPage;
			selTabIndex = 0;
		}
		break;
	case Manufacture:
		showTabs = mfgtabs;
		if(selPage == m_StorePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ManufacturePage;
			selTabIndex = 0;
		}
		break;
	case NoStore:
		showTabs = nostoretabs;
		selPage = m_BuildingPage;
		selTabIndex = 0;
		break;
	case RepairShop:
		showTabs = reptabs;
		if(selPage == m_ManufacturePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_StorePage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 3;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_RepairShopPage;
			selTabIndex = 0;
		}
		break;
	case Shed:
		showTabs = shedtabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ShedPage;
			selTabIndex = 0;
		}
		break;
	case SpacecraftFactory:
		showTabs = scfacttabs;
		if(selPage == m_StorePage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else if(selPage == m_BuildingPage)
			selTabIndex = 2;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_ScFactPage;
			selTabIndex = 0;
		}
		break;
	case StorageTank:
		showTabs = stanktabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_StorageTankPage;
			selTabIndex = 0;
		}
		break;
	case StreetLight:
		showTabs = streetlighttabs;
		selPage = m_BuildingPage;
		selTabIndex = 0;
		break;
	case Warehouse:
		showTabs = waretabs;
		if(selPage == m_BuildingPage)
			selTabIndex = 1;	// Current page is valid so select the appropriate tab for it.
		else
		{
			selPage = m_WarehousePage;
			selTabIndex = 0;
		}
		break;
	}

	if(arenatabs != showTabs)
		arenatabs->hide();
	if(banktabs != showTabs)
		banktabs->hide();
	if(bldtabs != showTabs)
		bldtabs->hide();
	if(brokertabs != showTabs)
		brokertabs->hide();
	if(captabs != showTabs)
		captabs->hide();
	if(contabs != showTabs)
		contabs->hide();
	if(designtabs != showTabs)
		designtabs->hide();
	if(farmtabs != showTabs)
		farmtabs->hide();
	if(hangartabs != showTabs)
		hangartabs->hide();
	if(hqtabs != showTabs)
		hqtabs->hide();
	if(mfgtabs != showTabs)
		mfgtabs->hide();
	if(nostoretabs != showTabs)
		nostoretabs->hide();
	if(reptabs != showTabs)
		reptabs->hide();
	if(scfacttabs != showTabs)
		scfacttabs->hide();
	if(shedtabs != showTabs)
		shedtabs->hide();
	if(stanktabs != showTabs)
		stanktabs->hide();
	if(streetlighttabs != showTabs)
		streetlighttabs->hide();
	if(waretabs != showTabs)
		waretabs->hide();
	if(showTabs)
	{
		showTabs->show();
		showTabs->setCurrentIndex(selTabIndex);
	}
	m_Pages->setCurrentIndex(selPage);
	m_Mode = modeNeeded;
}
