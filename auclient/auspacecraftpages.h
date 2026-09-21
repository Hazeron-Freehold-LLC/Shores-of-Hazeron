//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftpages.h,v 1.12 2022/02/19 18:01:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraft_H
#define AuSpacecraft_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "aushipment.h"
#include "autabcolumn.h"

class AuDamageReport;
class AuMissionTool;
class AuScExchange;
class AuSpacecraftAccount;
class AuSpacecraftBerths;
class AuSpacecraftBounties;
class AuSpacecraftPolicy;
class AuSpacecraftShipments;
class AuSpacecraftSpecs;
class QStackedLayout;

class AUCLIENT_DLL AuSpacecraftTabs : public AuTabColumn
{
	Q_OBJECT;
public:
	AuSpacecraftTabs(QWidget *parent=0);
	~AuSpacecraftTabs();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftPages : public QWidget
{
friend class AuFrame;
	Q_OBJECT;
private:
	// Widgets
	QStackedLayout*						m_Pages;
	AuSpacecraftAccount*				m_ShipAccount;
	AuSpacecraftBerths*					m_ShipBerths;
	AuSpacecraftBounties*				m_ShipBounties;
	AuDamageReport*						m_ShipDamage;
	AuScExchange*						m_ShipDesignExchange;
	AuMissionTool*						m_ShipMission;
	AuSpacecraftPolicy*					m_ShipPolicy;
	AuSpacecraftShipments*				m_ShipShipments;
	AuSpacecraftSpecs*					m_ShipSpecs;

public:
	AuSpacecraftPages(bool freeAvatar, QWidget* parent=0);
	~AuSpacecraftPages();

	void BrokerShipment(AuShipment shipment);

	void BrokerShipmentsClear();

	AuMissionTool* ShipMission()
	{
		return m_ShipMission;
	}

	virtual QSize sizeHint() const;

protected slots:
	void CurrentTabChanged(int idx);

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftDock : public AuDockWidget
{
public:
	AuSpacecraftPages*					m_Content;

public:
	AuSpacecraftDock(QWidget* parent) : AuDockWidget(":data/GovSpacecraft.png", tr("<b>Spacecraft</b>"), tr("Spacecraft"),
			parent)
	{
		m_Content = new AuSpacecraftPages(false, this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftDockFree : public AuDockWidget
{
public:
	AuSpacecraftPages*					m_Content;

public:
	AuSpacecraftDockFree(QWidget* parent) : AuDockWidget(":data/GovSpacecraft.png", tr("<b>Spacecraft</b>"), tr("Spacecraft"),
			parent)
	{
		SetWikiTopic("Commanding a Spacecraft");
		m_Content = new AuSpacecraftPages(true, this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraft_H
