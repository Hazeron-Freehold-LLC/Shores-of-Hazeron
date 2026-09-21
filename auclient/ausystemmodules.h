//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemmodules.h,v 1.7 2022/02/19 18:02:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemModules_H
#define AuSystemModules_H

#include "auclientlib.h"
#include "audialog.h"
#include "dbcommodity.h"

using namespace std;
#include <unordered_map>

class AuComboBoxValue;
class QComboBox;

class AUCLIENT_DLL AuSystemModules : public AuDialog
{
	Q_OBJECT;
public:
	// Values
	quint16					m_CnCModule;
	quint16					m_FTLDriveModule;
	quint16					m_HullModule;
	quint16					m_LifeSupportModule;
	quint16					m_ManeuverDriveModule;
	quint16					m_PowerPlantModule;
	quint16					m_SensorModule;
	quint16					m_ShieldModule;
	unordered_map<quint16, quint16>	m_BayModules;
	unordered_map<quint16, quint16>	m_TransporterModules;

private:
	// Widgets
	QComboBox*		m_CnC;
	QComboBox*		m_FTLDrive;
	QComboBox*		m_Hull;
	QComboBox*		m_LifeSupport;
	QComboBox*		m_ManeuverDrive;
	QComboBox*		m_PowerPlant;
	QComboBox*		m_Shield;
	QComboBox*		m_Sensor;
	unordered_map<quint16, AuComboBoxValue*>	m_Bays;
	unordered_map<quint16, AuComboBoxValue*>	m_Transporters;

public:
	AuSystemModules(bool isBuilding, int lowestDeck, quint16 cncModule, quint16 ftlDriveModule, quint16 hullModule,
			quint16 lifeSupportModule, quint16 maneuverDriveModule, quint16 powerPlantModule, quint16 sensorModule,
			quint16 shieldModule, const unordered_map<quint16, quint16>& bayModules,
			const unordered_map<quint16, quint16>& transporterModules, QWidget* parent=0);
	virtual ~AuSystemModules();

private slots:
	void BayModuleChanged(int, int);
	void CnCModuleChanged(int);
	void FTLDriveModuleChanged(int);
	void HullModuleChanged(int);
	void LifeSupportModuleChanged(int);
	void ManeuverDriveModuleChanged(int);
	void PowerPlantModuleChanged(int);
	void SensorModuleChanged(int);
	void ShieldModuleChanged(int);
	void TransporterModuleChanged(int, int);

private:
	void ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID, bool isBuilding);
};

#endif // AuSystemModules_H
