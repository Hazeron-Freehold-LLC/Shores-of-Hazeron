//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentempire.h,v 1.13 2022/02/19 17:49:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentEmpire_H
#define AuAgentEmpire_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "dbindustry.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;

class AUCLIENT_DLL AuAgentEmpire : public QWidget
{
	Q_OBJECT;
private:
	bool		m_DataValid;					// true if char and cities lists are good, false to refill on next setVisible

	// Widgets
	QComboBox*							m_EmpireCitizens;
	QLabel*								m_EmpireName;
	QLabel*								m_EmpireRole;
	QComboBox*							m_Empires;
	QLabel*								m_Flag;

public:
	AuAgentEmpire(QWidget* parent=0);
	virtual ~AuAgentEmpire();

signals:
	void EmpireDataComplete();
	void EmpireDataRequested();

public slots:
	void EmpireDataInvalid();
	virtual void setVisible(bool visible);
	void VisibilityRestored();

protected slots:
	void EmpireChgEmperor();
	void EmpireChgFlag();
	void EmpireChgName();
	void EmpireChgRole();
	void EmpireSelected(int index);
	void RefreshEmpires();
	void PirateSetBuilding();
	void PirateSetExplorerShip();
	void PirateSetFreighterShip();
	void PirateSetMilitaryShip();
	void PirateSetSpaceStation();

protected:
	void PirateSetIndustryBuilding(quint32 empireId, INDUSTRYID industry, quint32 blueprint);
	void PirateSetSpacecraft(int scrole);
	void RefreshData();
};

#endif // AuAgentEmpire_H
