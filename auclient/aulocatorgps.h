//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorgps.h,v 1.7 2022/02/19 17:58:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLocatorGPS_H
#define AuLocatorGPS_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QWidget>

class QLineEdit;
class QTimer;

class AUCLIENT_DLL AuLocatorGPS : public QWidget
{
	Q_OBJECT;
private:
	QTimer*			m_timer;

	// Widgets
	QLineEdit*		m_GalaxyPos;
	QLineEdit*		m_SectorPos;
	QLineEdit*		m_SystemPos;

public:
	AuLocatorGPS(QWidget* parent=0);
	virtual ~AuLocatorGPS();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorGPSDock : public AuDockWidget
{
public:
	AuLocatorGPS*						m_Content;

public:
	AuLocatorGPSDock(QWidget* parent) : AuDockWidget(":data/SensorScan.png", tr("<b>GPS</b>"), tr("GPS"), parent)
	{
		m_Content = new AuLocatorGPS(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorGPSAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorGPSAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuLocatorGPS_H
