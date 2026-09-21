//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftpolicy.h,v 1.9 2022/02/19 18:01:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftPolicy_H
#define AuSpacecraftPolicy_H

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

class AUCLIENT_DLL AuSpacecraftPolicy : public QWidget
{
	Q_OBJECT;
private:
	AuId			m_PersonId;		// Id of the person.
	QString			m_PersonName;
	GALAXY			m_PersonGalaxy;
	AuId			m_PersonSector;
	AuId			m_ShipId;		// Id of the person's ship.
	QTimer*			m_timer;		// For updates.

	// Widgets
	QCheckBox*		ckcrewopen;
	QCheckBox*		ckpassopen;
	QCheckBox*		ckpassSeaChest;
	QCheckBox*		cktroopopen;
	QLabel*			lbshipowner;
	QComboBox*		m_AccessPolicy;
	QComboBox*		m_AlertCondition;
	QCheckBox*		m_AutoFetchVehicles;
	QComboBox*		m_PilotUpPolicies;
	QComboBox*		m_ShieldFuelPolicies;
	QComboBox*		m_ShieldPolicies;
	QPushButton*	pbclaimtitle;
	QPushButton*	pbcopytitle;
	QPushButton*	pbfreetitle;
	QPushButton*	pchristen;

public:
	AuSpacecraftPolicy(QWidget* parent);
	virtual ~AuSpacecraftPolicy();

signals:
	void AcceptCrew(bool accept);
	void AcceptPassengers(bool accept);
	void AcceptTroops(bool accept);
	void AccessPolicy(int policy);
	void AlertCondition(int condition);
	void AllowPassengerSeaChests(bool allow);
	void AutoFetchVehicles(bool autoFetch);
	void PilotUpPolicy(int policy);
	void ShieldFuelPolicy(int policy);
	void ShieldUpPolicy(int policy);

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void AccessPolicyChanged(int index);
	void AlertConditionChanged(int index);
	void AutoFetchVehiclesChecked(int state);
	void Christen();
	void ClaimTitle();
	void CopyTitle();
	void CrewChecked(int state);
	void EnableControls();
	void FreeTitle();
	void PassengerChecked(int state);
	void PassengerSeaChestChecked(int state);
	void PilotUpPolicyChanged(int index);
	void ShieldFuelPolicyChanged(int index);
	void ShieldPolicyChanged(int index);
	void TroopChecked(int state);
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftPolicyDock : public AuDockWidget
{
public:
	AuSpacecraftPolicy*					m_Content;

public:
	AuSpacecraftPolicyDock(QWidget* parent) : AuDockWidget(":data/Captain.png", tr("<b>Spacecraft Policy</b>"),
			tr("Policy"), parent)
	{
		m_Content = new AuSpacecraftPolicy(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftPolicyAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftPolicyAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftPolicy_H
