//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentship.h,v 1.7 2022/02/19 17:49:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentShip_H
#define AuAgentShip_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class AUCLIENT_DLL AuAgentShip : public QWidget
{
	Q_OBJECT;
private:
	// Widgets

public:
	AuAgentShip(QWidget* parent=0);
	virtual ~AuAgentShip();

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void ClearRatDNA();
	void DrainCapacitor();
	void FillCapacitor();
	void LoadCrew();
	void LoadOfficer();
	void LoadPassengers();
	void LoadTroops();
	void PirateEquip();
	void RecoverShip();
	void Refuel();
	void Repair();
	void ScuttleShip();
	void ToggleOwnership();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuAgentScuttle : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_Reason;

public:
	AuAgentScuttle(QWidget* parent=0);
	virtual ~AuAgentScuttle();

	QString Reason() const;
};

#endif // AuAgentShip_H
