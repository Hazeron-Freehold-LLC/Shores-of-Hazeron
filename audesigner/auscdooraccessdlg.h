//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdooraccessdlg.h,v 1.4 2022/02/19 18:22:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDoorAccessDlg_H
#define AuScDoorAccessDlg_H

#include "audef.h"
#include "audialog.h"

class QCheckBox;

class AuScDoorAccessDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QCheckBox*							m_InitLocked;
	QCheckBox*							m_IsAuto;
	QCheckBox*							m_RemoteSpacecraft;
	QCheckBox*							m_RemoteVehicle;

public:
	AuScDoorAccessDlg(Qt::CheckState isAuto, Qt::CheckState initLock, Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle,
			QWidget* parent=0);
	virtual ~AuScDoorAccessDlg();

	Qt::CheckState IsAutomatic() const;

	Qt::CheckState InitLocked() const;

	Qt::CheckState RemoteSpacecraft() const;

	Qt::CheckState RemoteVehicle() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoIt();
};

#endif // AuScDoorAccessDlg_H
