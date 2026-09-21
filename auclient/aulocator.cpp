//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocator.cpp,v 1.5 2022/02/19 18:16:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulocator.h"
//
#include <auglobal.h>
#include "aulocatorgps.h"
#include "aulocatormap.h"
#include "aulocatorpos.h"
#include "aulocatorstarmap.h"
#include "ausystemscan.h"
#include "auapp.h"
#include "auframe.h"
#include <QGridLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QStackedLayout>
#include <QTabBar>

void AuLocatorDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocator::AuLocator(QWidget* parent) : QWidget(parent)
{
	m_Tabs = new QTabBar(this);

	m_Pos = new AuLocatorPos(this);

	QIcon posIcon(":data/Locator.png");
	int itab = m_Tabs->insertTab(0, posIcon, "");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Compass</b><br>"
				"Heading and location information."
				"</html>"));

	m_WorldMap = new AuLocatorMap(this);

	QIcon mapIcon(":data/Map.png");
	itab = m_Tabs->insertTab(1, mapIcon, "");
	m_Tabs->setTabToolTip(itab, QString("<html><b>World Map</b><br>"
				"Map of the world, when you are at a world.<br>"
				"%1 Double-click the map to center on a location.<br>"
				"%1 Double-click your flashing icon or the map margin to center on your location.<br>"
				"%1 Mouse wheel or drag the magnifier to change magnification.<br>"
				"%1 Click/drag to move the map.<br>"
				"%1 Right-click for options."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	m_SystemScan = new AuSystemScan(true, this);

	QIcon sysIcon(":data/RangeSystem.png");
	itab = m_Tabs->insertTab(2, sysIcon, "");
	m_Tabs->setTabToolTip(itab, QString("<html><p><b>System Map</b><br>"
				"Shows a map of your current solar system. "
				"Data is limited if the system has not been surveyed by your empire. "
				"</p>"
				"<p>"
				"To survey a system, do one of these things.<br>"
				"%1 View detailed information about a sun or world on a sensor console.<br>"
				"%1 Order crew to conduct a <i>Solar System Survey Scan</i>.<br>"
				"%1 Build a city that has an observatory and a radar.<br>"
				"</p>"
				"</html>").arg(g_Au.MiddleDotSymbol()));

	m_StarMap = new AuLocatorStarMap(this);

	QIcon empireIcon(":data/RangeGalaxy.png");
	itab = m_Tabs->insertTab(3, empireIcon, "");
	m_Tabs->setTabToolTip(itab, QString("<html><b>Star Map</b><br>"
				"Map of the stars known to your empire.<br>"
				"%1 Click a place to select or deselect.<br>"
				"%1 Double-click a place to center on its location.<br>"
				"%1 Mouse wheel or drag the magnifier to change distance from eye to look point. "
				"Shift key reduces wheel zoom increment.<br>"
				"%1 Click/drag to look around.<br>"
				"%1 W moves the look point forward.<br>"
				"%1 S moves the look point backward.<br>"
				"%1 A moves the look point left.<br>"
				"%1 D moves the look point right.<br>"
				"%1 C and Z rotate around the look point."
				"%1 Right-click for options."
				"</html>").arg(g_Au.MiddleDotSymbol()));

	m_GPS = new AuLocatorGPS(this);

	QIcon gpsIcon(":data/SensorScan.png");
	itab = m_Tabs->insertTab(4, gpsIcon, "");
	m_Tabs->setTabToolTip(itab, QString("<html><b>GPS</b><br>"
				"Galactic positioning system coordinates."
				"</html>"));

	// Do this after signal/slot connections so the compass size updates correctly.
	m_Pages = new QStackedLayout;
	m_Pages->insertWidget(0, m_Pos);
	m_Pages->insertWidget(1, m_WorldMap);
	m_Pages->insertWidget(2, m_SystemScan);
	m_Pages->insertWidget(3, m_StarMap);
	m_Pages->insertWidget(4, m_GPS);
	connect(m_Tabs, SIGNAL(currentChanged(int)), m_Pages, SLOT(setCurrentIndex(int)));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Tabs);
	lay->addLayout(m_Pages);
	lay->setMargin(2);
	lay->setSpacing(0);
	setLayout(lay);
}

AuLocator::~AuLocator()
{
}

QSize AuLocator::sizeHint() const
{
	return QSize(800,230);
}

void AuLocator::RestoreSettings()
{
	/*
	float scale = g_Au.Settings().readNumEntry("Widget/Locator/Scale", m_WorldMap->MapScale());
	m_WorldMap->SetMapScale(scale);
	int compx = g_Au.Settings().readNumEntry("Widget/Locator/CompassX", m_WorldMap->CompassLoc().x());
	int compy = g_Au.Settings().readNumEntry("Widget/Locator/CompassY", m_WorldMap->CompassLoc().y());
	m_WorldMap->SetCompassLoc(QPoint(compx, compy));
	*/

	m_StarMap->RestoreSettings();
}

void AuLocator::SaveSettings()
{
	/*
	g_Au.Settings().writeEntry("Widget/Locator/Scale", m_WorldMap->MapScale());
	g_Au.Settings().writeEntry("Widget/Locator/CompassX", m_WorldMap->CompassLoc().x());
	g_Au.Settings().writeEntry("Widget/Locator/CompassY", m_WorldMap->CompassLoc().y());
	*/

	m_StarMap->SaveSettings();
}

void AuLocator::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	RestoreSettings();
}

void AuLocator::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	SaveSettings();
}

void AuLocator::EmpireDataChanged()
{
	if(m_StarMap)
		m_StarMap->EmpireDataChanged();
}

void AuLocator::StarMapErased()
{
	if(m_StarMap)
		m_StarMap->StarMapErased();
}

void AuLocator::StarMapSectorAdded(const AuId& sectorId)
{
	if(m_StarMap)
		m_StarMap->StarMapSectorAdded(sectorId);
}

void AuLocator::StarMapShowCities(const AuIdList& cityIds)
{
	if(m_StarMap)
		m_StarMap->StarMapShowCities(cityIds);
}

void AuLocator::StarMapShowGalaxyLocation(const QString& galaxyName, const AuPoint& loc)
{
	if(m_StarMap)
		m_StarMap->StarMapShowGalaxyLocation(galaxyName, loc);
}

void AuLocator::StarMapUpdateAllComplete()
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateAllComplete();
}

void AuLocator::StarMapUpdateGalaxyComplete(GALAXY galaxy)
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateGalaxyComplete(galaxy);
}

void AuLocator::StarMapUpdateSectorComplete(GALAXY galaxy, const AuIdList& sectorIds)
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateSectorComplete(galaxy, sectorIds);
}

void AuLocator::StarMapUpdateAllStarted()
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateAllStarted();
}

void AuLocator::StarMapUpdateGalaxyStarted(GALAXY galaxy)
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateGalaxyStarted(galaxy);
}

void AuLocator::StarMapUpdateSectorStarted(const AuId& sectorId, unsigned int range)
{
	if(m_StarMap)
		m_StarMap->StarMapUpdateSectorStarted(sectorId, range);
}
