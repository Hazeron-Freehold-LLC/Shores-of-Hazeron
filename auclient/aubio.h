//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubio.h,v 1.13 2022/02/19 17:51:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioPages_H
#define AuBioPages_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"

class AuBioAccount;
class AuBioAvatar;
class AuBioBounties;
class AuBioJobs;
class AuBioBiology;
class AuBioDomain;
class AuBioSkills;
class QStackedLayout;
class QTabBar;

class AUCLIENT_DLL AuBioPages : public QWidget
{
	Q_OBJECT;
public:
	AuBioAccount*			m_AccountPage;
	AuBioAvatar*			m_AvatarPage;
	AuBioBiology*			m_BiologyPage;
	AuBioBounties*			m_BountiesPage;
	AuBioDomain*			m_DomainPage;
	AuBioJobs*				m_JobsPage;
	AuBioSkills*			m_SkillsPage;

private:
	// Widgets
	QStackedLayout*			m_Pages;
	QTabBar*				m_Tabs;

public:
	AuBioPages(QWidget* parent=0);
	virtual ~AuBioPages();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBioDock : public AuDockWidget
{
public:
	AuBioPages*						m_Bio;

public:
	AuBioDock(QWidget* parent) : AuDockWidget(":data/c_VacSuitHelmet.png", tr("<b>Bio</b>"), tr("Bio"), parent)
	{
		SetWikiTopic("Bio");
		m_Bio = new AuBioPages(this);
		setWidget(m_Bio);

		int dockSides = g_Au.Settings().readNumEntry("Dock/BioAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/BioAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuBioPages_H
