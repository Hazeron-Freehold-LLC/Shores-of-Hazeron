//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorpos.h,v 1.9 2022/02/19 17:58:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLocatorPos_H
#define AuLocatorPos_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QWidget>

class AuCompassWidget;
class QLabel;
class QLineEdit;
class QTimer;

class AUCLIENT_DLL AuLocatorPos : public QWidget
{
	Q_OBJECT;
private:
	QTimer*								m_Timer;

	// Widgets
	QLabel*								m_City;
	QLabel*								m_CityIcon;
	AuCompassWidget*					m_Compass;
	QLabel*								m_Galaxy;
	QLabel*								m_GalaxyIcon;
	QLineEdit*							m_LongLat;
	QLabel*								m_Sector;
	QLabel*								m_SectorIcon;
	QLabel*								m_Ship;
	QLabel*								m_ShipIcon;
	QLabel*								m_System;
	QLabel*								m_SystemIcon;
	QLabel*								m_Weather;
	QLabel*								m_WeatherIcon;
	QLabel*								m_World;
	QLabel*								m_WorldIcon;

public:
	AuLocatorPos(QWidget* parent=0);
	virtual ~AuLocatorPos();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorCompassDock : public AuDockWidget
{
public:
	AuLocatorPos*						m_Content;

public:
	AuLocatorCompassDock(QWidget* parent) : AuDockWidget(":data/Locator.png", tr("<b>Compass</b>"), tr("Compass"), parent)
	{
		m_Content = new AuLocatorPos(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorCompassAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorCompassAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuLocatorPos_H
