//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovern.h,v 1.9 2022/02/19 17:57:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovern_H
#define AuGovern_H

#include "auclientlib.h"
#include <audockwidget.h>
#include <auglobal.h>

class AuEmpireCitizens;
class AuGovAnnouncements;
class AuGovCities;
class AuGovCommerce;
class AuGovDiplomacy;
class AuGovPatents;
class AuGovPolicy;
class AuGovTaxes;
class QStackedLayout;

class AUCLIENT_DLL AuGovern : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	AuGovAnnouncements*					m_Announcements;
	AuEmpireCitizens*					m_Citizens;
	AuGovCommerce*						m_Commerce;
	AuGovDiplomacy*						m_Diplomacy;
	QStackedLayout*						m_Pages;
	AuGovCities* 						m_Places;
	AuGovPatents*						m_Patents;
	AuGovPolicy*						m_Policy;
	AuGovTaxes*							m_Taxes;

public:
	AuGovern(QWidget* parent=0);
	~AuGovern();

	virtual QSize sizeHint() const;

public slots:
	void UpdateDatabase();

protected slots:
	void CurrentTabChanged(int idx);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovernDock : public AuDockWidget
{
public:
	AuGovern*								m_Govern;

public:
	AuGovernDock(QWidget* parent) : AuDockWidget(":data/CommGovernment.png", tr("<b>Governance</b>"), tr("Govern"), parent)
	{
		SetWikiTopic("Governance");
		m_Govern = new AuGovern(this);
		setWidget(m_Govern);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernanceAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernanceAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovern_H
