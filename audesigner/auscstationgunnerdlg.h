//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscstationgunnerdlg.h,v 1.2 2022/02/19 18:22:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScStationGunnerDlg_H
#define AuScStationGunnerDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;
class QPushButton;
class QRadioButton;

class AuScStationGunnerDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QComboBox*							m_Modules;
	QPushButton*						m_PickLocation;
	QRadioButton*						m_SitStationary;
	QRadioButton*						m_SitSwivel;
	//QRadioButton*						m_SitSwivel3D;
	QRadioButton*						m_Stand;
	QRadioButton*						m_StandStationary;

public:
	AuScStationGunnerDlg(QWidget* parent=0);
	virtual ~AuScStationGunnerDlg();

	COMMODITYID Module() const;

	//AuScTurret::GUNNER_POSITION
	quint8 Position() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScStationGunnerDlg_H
