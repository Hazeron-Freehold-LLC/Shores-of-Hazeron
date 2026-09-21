//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftberths.h,v 1.10 2022/05/04 21:07:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftBerths_H
#define AuSpacecraftBerths_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "auroster.h"
#include "auspecies.h"
#include <QWidget>

class AuIconButton;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSplitter;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuSpacecraftBerths : public QWidget
{
friend class AuSpacecraftBerthsDialog;
	Q_OBJECT;
private:
	quint32			m_CrewBerths;
	AuId			m_PersonId;		// Id of the person.
	QString			m_PersonName;
	GALAXY			m_PersonGalaxy;
	AuId			m_PersonSector;
	quint32			m_PassengerBerths;
	QTimer*			m_timer;		// For updates.
	quint32			m_TroopBerths;

	// Widgets
	QLabel*			lbcryo;
	QListWidget*	lwavail;
	QSplitter*		m_Splitter;
	AuIconButton*	pbrevive;
	AuIconButton*	pbkick;
	AuIconButton*	pbkickcryo;
	AuIconButton*	pbrecon;
	AuIconButton*	pbrequest;
	QTableWidget*	twcryo;
	QTableWidget*	twroster;

public:
	AuSpacecraftBerths(QWidget* parent);
	virtual ~AuSpacecraftBerths();

	virtual QSize sizeHint() const;

signals:
	void KickBerthOccupant(AuId occupant);
	void KickCryoBerthOccupants(AuSpecies species, int count);
	void ReconcileRoster();
	void ReviveCryoBerthOccupant(AuSpecies species, quint8 role);	// 'role' is 0=none, 1=crew, 2=troop, 3=passenger.
	void SpeciesRequested(AuSpecies species);
	void StowawaysKicked();

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void EnableControls();
	void KickCryoOccupant();
	void KickOccupant();
	void Reconcile();
	void RequestSlot();
	void ReviveCryoOccupant();
	void SplitterMoved(int pos, int index);
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftRosterDock : public AuDockWidget
{
public:
	AuSpacecraftBerths*				m_Content;

public:
	AuSpacecraftRosterDock(QWidget* parent) : AuDockWidget(":data/CommCrew.png", tr("<b>Spacecraft Roster</b>"),
			tr("Roster"), parent)
	{
		m_Content = new AuSpacecraftBerths(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftRosterAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftRosterAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftBerths_H
