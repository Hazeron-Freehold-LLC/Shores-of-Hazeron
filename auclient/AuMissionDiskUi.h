//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionDiskUi.h,v 1.6 2022/02/19 17:58:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMissionDiskUi_H
#define AuMissionDiskUi_H

#include "auclientlib.h"
#include <audialog.h>
#include <aumission.h>

class QComboBox;
class QPushButton;

class AUCLIENT_DLL AuMissionDiskUi : public AuDialog
{
	Q_OBJECT;
	AuMission				m_Mission;
	QComboBox*				m_MissionDisks;
	QString					m_MissionName;
	QPushButton*			m_Ok;

public:
	AuMissionDiskUi(bool isNew, bool listShipMission, QWidget* parent=0);
	~AuMissionDiskUi();

	const AuMission& Mission() const
	{
		return m_Mission;
	}

	const QString& MissionName() const
	{
		return m_MissionName;
	}

protected slots:
	void MissionChanged(int index);
	void OkPressed();
};

#endif // AuMissionDiskUi_H
