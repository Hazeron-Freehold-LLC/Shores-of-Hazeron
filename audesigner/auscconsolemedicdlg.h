//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscconsolemedicdlg.h,v 1.2 2022/02/19 18:22:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScConsoleMedicDlg_H
#define AuScConsoleMedicDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;
class QPushButton;
class QRadioButton;

class AuScConsoleMedicDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QComboBox*							m_Modules;
	QPushButton*						m_PickLocation;
	QRadioButton*						m_PatientProne;
	QRadioButton*						m_PatientSitStationary;
	QRadioButton*						m_PatientSitSwivel;
	QRadioButton*						m_PatientStand;
	QRadioButton*						m_PatientStandStationary;

public:
	AuScConsoleMedicDlg(QWidget* parent=0);
	virtual ~AuScConsoleMedicDlg();

	COMMODITYID Module() const;

	//AuScConsoleMedic::PATIENT_POSITION
	quint8 PatientPosition() const;

public slots:
	virtual void setVisible(bool visible);
	void EnableControls();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScConsoleMedicDlg_H
