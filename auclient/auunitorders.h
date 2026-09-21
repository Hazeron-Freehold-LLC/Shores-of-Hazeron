//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunitorders.h,v 1.10 2022/02/19 18:02:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuUnitOrders_H
#define AuUnitOrders_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auid.h"
#include <QTableWidget>
#include <QWidget>

class AuViewWidget;
class QComboBox;
class QTimer;

class AuUnitTable : public QTableWidget
{
	Q_OBJECT;
public:
	AuUnitTable(QWidget* parent);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* e);

signals:
	void UnitRightClicked(AuId unitId);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuUnitOrders : public QWidget
{
	Q_OBJECT;
	QComboBox*			m_Channels;		// Channel on which orders are transmitted.
	QTimer*				m_timer;		// UpdateContents timer
	AuUnitTable*		twcitizens;

public:
	AuUnitOrders(QWidget* parent=0);
	virtual ~AuUnitOrders();

	QString OrderChannel() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	void RestoreSettings();
	void SaveSettings();
	virtual void showEvent(QShowEvent* event);
	virtual QSize sizeHint() const;

protected slots:
	void AmmoClicked(int unitId);
	void AmmoCountChanged(int count, int unitId);
	void ArmorClicked(int unitId);
	void AttackTarget();
	void BattleStations();
	void BerserkPosture();
	void CloseDoor();
	void ClothingClicked(int unitId);
	void ComeHere();
	void DefendAreaPosture();
	void DefendSelfPosture();
	void DeselectAllUnits();
	void DropRider();
	void EnableControls();
	void EnterSurgery();
	void FollowMe();
	void FollowTarget();
	void Garrison();
	void GetIn();
	void GetItem();
	void GetOut();
	void GoToRoom();
	void GoToTarget();
	void HelmetClicked(int unitId);
	void HoldFirePosture();
	void MountMe();
	void MountTarget();
	void OffensivePosture();
	void OpenDoor();
	void ReleaseCommand();
	void SelectCrew();
	void SelectOfficers();
	void SelectPassengers();
	void SelectTroops();
	void SelectType(int npcRole);	// 'npcRole' is an NPC_ROLE
	void SelectionChanged();
	void Stay();
	void TakeCommand();
	void UnitRightClicked(AuId unitId);
	void UpdateContents();
	void ViewResume(AuId unitId);
	void WeaponClicked(int unitId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuUnitOrdersDock : public AuDockWidget
{
public:
	AuUnitOrders*								m_UnitOrders;

public:
	AuUnitOrdersDock(QWidget* parent);

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas);

	virtual void UpdateTitleShortcut();
};

#endif // AuUnitOrders_H
