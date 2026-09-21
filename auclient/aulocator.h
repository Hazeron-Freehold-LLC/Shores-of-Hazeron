//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocator.h,v 1.9 2022/02/19 17:58:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AULOCATOR_H
#define AULOCATOR_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"

class AuId;
class AuLocatorGPS;
class AuLocatorMap;
class AuLocatorPos;
class AuLocatorStarMap;
class AuSystemScan;
class QSpinBox;
class QStackedLayout;
class QTabBar;

class AUCLIENT_DLL AuLocator : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	AuLocatorGPS*			m_GPS;
	QStackedLayout* 		m_Pages;
	AuLocatorPos*			m_Pos;
	AuLocatorStarMap*		m_StarMap;
	AuSystemScan*			m_SystemScan;
	QTabBar*				m_Tabs;
	AuLocatorMap*			m_WorldMap;

public:
	AuLocator(QWidget* parent=0);
	~AuLocator();

	virtual QSize sizeHint() const;

	void StarMapErased();

	void StarMapSectorAdded(const AuId& sectorId);

	void StarMapShowCities(const AuIdList& cityIds);

	void StarMapShowGalaxyLocation(const QString& galaxyName, const AuPoint& loc);

	void StarMapUpdateAllComplete();

	void StarMapUpdateGalaxyComplete(GALAXY galaxy);

	void StarMapUpdateSectorComplete(GALAXY galaxy, const AuIdList& sectorIds);

	void StarMapUpdateAllStarted();

	void StarMapUpdateGalaxyStarted(GALAXY galaxy);

	void StarMapUpdateSectorStarted(const AuId& sector, unsigned int range);

public slots:
	void EmpireDataChanged();

protected:
	virtual void hideEvent(QHideEvent* event);
	void RestoreSettings();
	void SaveSettings();
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorDock : public AuDockWidget
{
public:
	AuLocator*							m_Locator;

public:
	AuLocatorDock(QWidget* parent) : AuDockWidget(":data/Locator.png", tr("<b>Locator</b>"), tr("Locator"), parent)
	{
		SetWikiTopic("Locator");
		m_Locator = new AuLocator(this);
		setWidget(m_Locator);

		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AULOCATOR_H
