//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftbounties.h,v 1.8 2022/05/04 21:48:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftBounties_H
#define AuSpacecraftBounties_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "auroster.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuSpacecraftBounties : public QWidget
{
	Q_OBJECT;
private:
	QTimer*			m_timer;		// For updates.

	// Widgets
	QTableWidget*	twbounties;

public:
	AuSpacecraftBounties(QWidget* parent);
	virtual ~AuSpacecraftBounties();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftBountiesDock : public AuDockWidget
{
public:
	AuSpacecraftBounties*				m_Content;

public:
	AuSpacecraftBountiesDock(QWidget* parent) : AuDockWidget(":data/JobBountyHunter.png", tr("<b>Spacecraft Bounties</b>"),
			tr("Bounties"), parent)
	{
		m_Content = new AuSpacecraftBounties(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftBountiesAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftBountiesAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftBounties_H
