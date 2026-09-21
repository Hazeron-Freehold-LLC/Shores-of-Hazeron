//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubld.h,v 1.10 2022/02/19 17:51:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBld_H
#define AuBld_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "aushipment.h"

class AuBldBroker;
class AuBldCapitol;
class QStackedLayout;
class QTabBar;
class QTimer;

class AUCLIENT_DLL AuBld : public QWidget
{
	Q_OBJECT;

	enum MODE
	{
		Init,			// Mode not yet established.
		Arena,			// Person is in arena.
		Bank,			// Bank.
		Broker,			// Broker.
		Building,		// A building that does not manufacture anything.
		Capitol,		// Person is in capitol building.
		Construction,	// Person is in construction zone.
		DesignStudio,	// Person is in design studio.
		Farm,			// Farm
		Hangar,			// Spacecraft hangar.
		HQ,				// Person is in base HQ building.
		Info,			// Person is not in a zone with any specific UI available.
		Manufacture,	// Person is in manufacturing zone.
		NoStore,		// Only Building and Info tabs, for buildings that cannot have a store.
		RepairShop,		// Airport Repair Shop.
		Shed,
		SpacecraftFactory,
		StorageTank,
		Warehouse,
		StreetLight,
	};

	MODE		m_Mode;
	QTimer*		m_timer;

	// Page indices in m_Pages stacked layout.
	int			m_ArenaPage;
	int			m_BankPage;
	int			m_BrokerPage;
	int			m_BuildingPage;
	int			m_CapitolPage;
	int			m_ConstructionPage;
	int			m_DesignPage;
	int			m_FarmPage;
	int			m_HangarPage;
	int			m_HQPage;
	int			m_InfoPage;
	int			m_ManufacturePage;
	int			m_RepairShopPage;
	int			m_ScFactPage;
	int			m_ShedPage;
	int			m_StorageTankPage;
	int			m_StorePage;
	int			m_WarehousePage;

	// Widgets
	QTabBar*							arenatabs;
	QTabBar*							banktabs;
	QTabBar*							bldtabs;
	QTabBar*							brokertabs;
	QTabBar*							captabs;
	QTabBar*							contabs;
	QTabBar*							designtabs;
	QTabBar*							farmtabs;
	QTabBar*							hangartabs;
	QTabBar*							hqtabs;
	QTabBar*							mfgtabs;
	QTabBar*							nostoretabs;
	QTabBar*							reptabs;
	QTabBar*							scfacttabs;
	QTabBar*							shedtabs;
	QTabBar*							stanktabs;
	QTabBar*							streetlighttabs;
	QTabBar*							waretabs;

	AuBldBroker*						m_BldBroker;
	AuBldCapitol*						m_BldCapitol;
	QStackedLayout*						m_Pages;

public:
	AuBld(QWidget* parent=0);
	~AuBld();

	void BrokerShipment(AuShipment shipment);

	void BrokerShipmentsClear();

	void BrokerTradeAgreement(AuTradeAgreement ta);

	void BrokerTradeAgreementsClear();

	virtual QSize sizeHint() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void ArenaTabChanged(int idx);
	void BankTabChanged(int idx);
	void BrokerTabChanged(int idx);
	void BuildingTabChanged(int idx);
	void CapitolTabChanged(int idx);
	void ConstructionTabChanged(int idx);
	void DesignTabChanged(int idx);
	void FarmTabChanged(int idx);
	void HangarTabChanged(int idx);
	void HQTabChanged(int idx);
	void ManufacturingTabChanged(int idx);
	void NoStoreTabChanged(int idx);
	void RepairShopTabChanged(int idx);
	void ShedTabChanged(int idx);
	void SpacecraftFactoryTabChanged(int idx);
	void StorageTankTabChanged(int idx);
	void StreetLightTabChanged(int idx);
	void UpdateContents();
	void WarehouseTabChanged(int idx);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBuildingDock : public AuDockWidget
{
public:
	AuBld*							m_Labor;

public:
	AuBuildingDock(QWidget* parent) : AuDockWidget(":data/City.png", tr("<b>Building</b>"), tr("Building"), parent)
	{
		SetWikiTopic("Category:Colonization");
		m_Labor = new AuBld(this);
		setWidget(m_Labor);

		int dockSides = g_Au.Settings().readNumEntry("Dock/BuildingAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/BuildingAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuBld_H
