//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldhq.h,v 1.6 2022/02/19 17:52:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldHQ_H
#define AuBldHQ_H

#include "auclientlib.h"
#include "auglobell.h"
#include "auid.h"
#include <QWidget>

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

class AUCLIENT_DLL AuBldHQ : public QWidget
{
	Q_OBJECT;
private:
	enum MODE
	{
		Init,			// Mode not yet established.
		Info,			// Person is not in a zone with any specific UI available.
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
	QTabBar*							bldtabs;
	QTabBar*							contabs;
	QTabBar*							mfgtabs;
	QTabBar*							scfacttabs;
	QTabBar*							shedtabs;
	QTabBar*							stanktabs;
	QTabBar*							waretabs;
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
	QLabel*								m_WorldName;
	QToolButton*						m_DemolishButton;
	QStackedLayout*						m_Pages;
	QToolButton*						m_Report;

public:
	AuBldHQ(QWidget* parent=0);
	virtual ~AuBldHQ();

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

public slots:
	void BuildingChanged();

protected slots:
	void BuildingTabChanged(int idx);
	void ConstructionTabChanged(int idx);
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

#endif // AuBldHQ_H
