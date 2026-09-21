//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldcapitol.h,v 1.7 2022/02/19 17:51:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldCapitol_H
#define AuBldCapitol_H

#include "auclientlib.h"
#include "auglobell.h"
#include "auid.h"
#include "aushipment.h"
#include <QWidget>

class AuBldBank;
class AuBldBroker;
class AuBldBuilding;
class AuBldConstruct;
class AuBldInfo;
class AuBldManufacture;
class AuBldShed;
class AuBldSpacecraftFactory;
class AuBldStorageTank;
class AuBldWarehouse;
class QLabel;
class QPushButton;
class QStackedLayout;
class QTabBar;
class QTableWidget;
class QToolButton;

class AUCLIENT_DLL AuBldCapitol : public QWidget
{
	Q_OBJECT;
private:
	enum MODE
	{
		Init,			// Mode not yet established.
		Info,			// Person is not in a zone with any specific UI available.
		Bank,
		Broker,
		Building,		// A building that does not manufacture anything.
		Construction,	// Person is in construction zone.
		Manufacture,	// Person is in manufacturing zone.
		Shed,
		SpacecraftFactory,
		StorageTank,
		Warehouse,
	};

	MODE		m_Mode;

	// page indices in stacked layout
	AuId		m_CapitolId;			// Non-zero only if scene atom has governing authority over the city, to request reports.
	int			m_BankPage;
	int			m_BrokerPage;
	int			m_BuildingPage;
	int			m_ConstructionPage;
	int			m_InfoPage;
	int			m_ManufacturePage;
	int			m_ScFactPage;
	int			m_ShedPage;
	int			m_StorageTankPage;
	int			m_WarehousePage;
	AuId		m_SelectedId;
	AuGlobeLLF	m_SelectedLL;
	QString		m_SelectedName;

	// Widgets
	QTabBar*							banktabs;
	QTabBar*							bldtabs;
	QTabBar*							brokertabs;
	QTabBar*							contabs;
	QTabBar*							mfgtabs;
	QTabBar*							scfacttabs;
	QTabBar*							shedtabs;
	QTabBar*							stanktabs;
	QTabBar*							waretabs;
	AuBldBank*							m_BldBank;
	AuBldBroker*						m_BldBroker;
	AuBldBuilding*						m_BldBuilding;
	AuBldConstruct*						m_BldConstruct;
	AuBldInfo*							m_BldInfo;
	AuBldManufacture*					m_BldManufacture;
	AuBldShed*							m_BldShed;
	AuBldSpacecraftFactory*				m_BldScFact;
	AuBldStorageTank*					m_BldStorageTank;
	AuBldWarehouse*						m_BldWarehouse;
	QTableWidget*						m_Buildings;
	QPushButton*						m_CityName;
	QLabel*								m_Info;
	QLabel*								m_WorldName;
	QToolButton*						m_DeclareEmpireCapital;
	QToolButton*						m_DeclareGalaxyCapital;
	QToolButton*						m_DeclareSectorCapital;
	QToolButton*						m_DemolishButton;
	QStackedLayout*						m_Pages;
	QToolButton*						m_Report;

public:
	AuBldCapitol(QWidget* parent=0);
	virtual ~AuBldCapitol();

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

public slots:
	void BuildingChanged();
	void Shipment(AuShipment shipment);
	void ShipmentsClear();
	void TradeAgreement(AuTradeAgreement ta);
	void TradeAgreementsClear();

protected slots:
	void BankTabChanged(int idx);
	void BrokerTabChanged(int idx);
	void BuildingTabChanged(int idx);
	void ConstructionTabChanged(int idx);
	void DeclareEmpireCapital();
	void DeclareGalaxyCapital();
	void DeclareSectorCapital();
	void Demolish();
	void ListBuildings();
	void ManufacturingTabChanged(int idx);
	void RenameCity();
	void ReportCityLast();
	void ShedTabChanged(int idx);
	void SpacecraftFactoryTabChanged(int idx);
	void StorageTankTabChanged(int idx);
	void WarehouseTabChanged(int idx);
};

#endif // AuBldCapitol_H
