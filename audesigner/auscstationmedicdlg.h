//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscstationmedicdlg.h,v 1.2 2022/02/19 18:22:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScStationMedicDlg_H
#define AuScStationMedicDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;
class QPushButton;
class QRadioButton;

class AuScStationMedicDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QRadioButton*						m_MedicSitStationary;
	QRadioButton*						m_MedicSitSwivel;
	QRadioButton*						m_MedicStand;
	QRadioButton*						m_MedicStandStationary;
	QComboBox*							m_Modules;
	QPushButton*						m_PickLocation;
	QRadioButton*						m_PatientProne;
	QRadioButton*						m_PatientSitStationary;
	QRadioButton*						m_PatientSitSwivel;
	QRadioButton*						m_PatientStand;
	QRadioButton*						m_PatientStandStationary;

public:
	AuScStationMedicDlg(QWidget* parent=0);
	virtual ~AuScStationMedicDlg();

	//AuScStation::OPERATOR_POSITION
	quint8 MedicPosition() const;

	COMMODITYID Module() const;

	//AuScStationMedic::PATIENT_POSITION
	quint8 PatientPosition() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScStationMedicDlg_H
