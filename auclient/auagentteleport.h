//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentteleport.h,v 1.9 2022/02/19 17:49:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentTeleport_H
#define AuAgentTeleport_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuAgentTeleport : public QWidget
{
	Q_OBJECT;
private:
	bool		m_DataValid;					// true if char and cities lists are good, false to refill on next setVisible
	AuId		m_GoToId;
	bool		m_IsManualGoTo;					// true if m_GoToId is an id manually entered by the user
	AuId		m_SpawnCity;
	QString		m_SpawnCityName;
	AuId		m_SpawnShip;
	QString		m_SpawnShipName;
	AuId		m_SummonId;

	// Widgets
	QComboBox*							m_Account;
	QComboBox*							m_Avatars;
	QComboBox*							m_Cities;
	QPushButton*						m_GoTo;
	QPushButton*						m_GoToGalaxy;
	QPushButton*						m_GoToGalaxyWithRide;
	QPushButton*						m_GoToLongLat;
	QPushButton*						m_GoToLongLatWithRide;
	QPushButton*						m_GoToWithRide;
	QPushButton*						m_LimboRescue;
	QComboBox*							m_SceneAtoms;
	QPushButton*						m_Summon;
	QPushButton*						m_SummonWithRide;
	QLineEdit*							leatom;

public:
	AuAgentTeleport(QWidget* parent=0);
	virtual ~AuAgentTeleport();

	// 'destType' is 0=character, 1=city, 2=unknown
	void SetGoToDestination(const AuId& id, const QString& name, int destType=2);

	// Sends the AdminGoPacket.
	static void GoToDestination(const AuId& goToId, bool rideToo, bool limboRescue);

signals:
	void AgentActionComplete();
	void EmpireDataComplete();
	void EmpireDataRequested();

public slots:
	void EmpireDataInvalid();
	virtual void setVisible(bool visible);
	void VisibilityRestored();

protected slots:
	void AccountSelected(int index);
	void AvatarSelected(int index);
	void AvatarSelected(AuId charId, QString charName);
	void CitySelected(int index);
	void ClearAtomId();
	void GoToDestination();
	void GoToDestinationWithRide();
	void GoToGalaxy();
	void GoToGalaxyWithRide();
	void GoToLongLat();
	void GoToLongLatWithRide();
	void IdChanged(const QString& idText);
	void LimboRescue();
	void RefreshEmpires();
	void RefreshSceneAtoms();
	void SceneAtomSelected(int index);
	void SummonPerson();
	void SummonPersonWithRide();

protected:
	void RefreshData();
	void SummonPerson(const AuId& summonId, bool rideToo);
};

#endif // AuAgentTeleport_H
