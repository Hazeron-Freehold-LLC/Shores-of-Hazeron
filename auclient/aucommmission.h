//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommmission.h,v 1.8 2022/04/15 17:02:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommMission_H
#define AuCommMission_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "ausensorblip.h"

class AuCommChannel;
class AuMissionOrderWidget;
class AuIconButton;
class QTimer;

class AUCLIENT_DLL AuCommMission : public AuDialog
{
	Q_OBJECT;
	AuIconButton*				m_AddOrder;
	AuIconButton*				m_InsertOrder;
	AuIconButton*				m_LoadMission;
	AuMissionOrderWidget*		m_MO;
	AuIconButton*				m_NewOrder;
	AuId						m_ToId;
	quint32						m_PersonEmpire;

	AuSensorBlipList					m_SensorBlips;
	AuElapsedTimer						m_SensorBlipTime;		// Restarted when sensor blips arrive.
	AuIconButton*						m_SensorContacts;

	QTimer*								m_timer;				// For updates.

public:
	AuCommMission(const AuId& authorization, const QString& toName, const AuId& toId, const AuId& toSystemId,
			AuCommChannel* parent=0);
	virtual ~AuCommMission();

	void SensorContactsReceived(QByteArray bablips);

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;

protected slots:
	void ActivateMission();
	void AddMissionOrder();
	void CycleMission();
	void DeactivateMission();
	void EnableControls();
	void InsertMissionOrder();
	void LoadMission();
	void NewMission();
	void NoCycleMission();
	void RequestSensorContacts();
	void SendMission();
	void ShowMission();
	void UpdateContents();
};

#endif // AuCommMission_H
