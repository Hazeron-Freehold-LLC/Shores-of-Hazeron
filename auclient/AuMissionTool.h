//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionTool.h,v 1.12 2022/04/15 17:00:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMissionTool_H
#define AuMissionTool_H

#include "auclientlib.h"
#include "audialog.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "aumission.h"
#include "ausensorblip.h"

class AuIconButton;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuMissionTool : public QWidget
{
	Q_OBJECT;
private:
	AuMission		m_Mission;		// Copy of the mission that is currently displayed.
	QTimer*			m_timer;		// For updates.
	AuId			m_SystemId;		// System id at last timer update. Used to trigger request for data from server.
	AuSensorBlipList					m_SensorBlips;
	AuElapsedTimer						m_SensorBlipTime;	// Restarted when sensor blips arrive.

	// Widgets
	AuIconButton*	pbactivate;
	AuIconButton*	pbcycle;
	AuIconButton*	pbdeactivate;
	AuIconButton*	pbnocycle;
	AuIconButton*	pbsensors;
	AuIconButton*	pmadd;
	AuIconButton*	pmadddisk;
	AuIconButton*	pmclear;
	AuIconButton*	pmdelete;
	AuIconButton*	pminsert;
	AuIconButton*	pmnew;
	AuIconButton*	pmprop;
	AuIconButton*	tbburn;
	AuIconButton*	tbload;
	AuIconButton*	tborderfin;
	QTableWidget*	twmission;

public:
	AuMissionTool(QWidget* parent);
	virtual ~AuMissionTool();

	void SensorContactsReceived(QByteArray bablips);

protected:
	virtual void showEvent(QShowEvent* event);
	virtual QSize sizeHint() const;

protected slots:
	void EnableControls();
	void MissionActivate();
	void MissionAdd();
	void MissionAddMission();
	void MissionBurnCD();
	void MissionClear();
	void MissionCycle();
	void MissionDeactivate();
	void MissionDelete();
	void MissionInsert();
	void MissionLoad();
	void MissionNew();
	void MissionNoCycle();
	void MissionOrderComplete();
	void MissionOrderDoubleClicked(QTableWidgetItem* orderItem);
	void MissionOrderProperties();
	void RequestSensorContacts();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftMissionDock : public AuDockWidget
{
public:
	AuMissionTool*						m_Mission;

public:
	AuSpacecraftMissionDock(QWidget* parent) : AuDockWidget(":data/OrderTakeCommand.png", tr("<b>Spacecraft Mission</b>"),
			tr("Mission"), parent)
	{
		SetWikiTopic("Commanding a Spacecraft");
		m_Mission = new AuMissionTool(this);
		setWidget(m_Mission);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftMissionAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftMissionAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuMissionDialog : public AuDialog
{
	Q_OBJECT;
	AuMissionTool*						m_Mission;

public:
	AuMissionDialog(QWidget* parent=0);
	virtual ~AuMissionDialog();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuMissionTool_H
