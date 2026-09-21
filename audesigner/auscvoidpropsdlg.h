//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscvoidpropsdlg.h,v 1.2 2022/02/19 18:22:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScVoidPropsDlg_H
#define AuScVoidPropsDlg_H

#include "audef.h"
#include "audialog.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

class AuScVoidPropsDlg : public AuDialog
{
	Q_OBJECT;
public:
	enum MODE
	{
		UpZ,
		PickVector,
		UpZGrid
	};

private:
	MODE								m_Mode;

	// Widgets
	QComboBox*							m_GravityType;
	QComboBox*							m_HoldAccess;
	QLineEdit*							m_HeightEdit;
	QCheckBox*							m_PersonnelCrew;
	QCheckBox*							m_PersonnelOfficers;
	QCheckBox*							m_PersonnelPassengers;
	QCheckBox*							m_PersonnelTroops;
	QPushButton*						m_PickDir;

public:
	AuScVoidPropsDlg(QWidget* parent=0);
	virtual ~AuScVoidPropsDlg();

	GRAVITY GravityType() const;

	HOLD_ACCESS HoldAccess() const;

	// Valid on accept.
	MODE Mode() const
	{
		return m_Mode;
	}

	quint8 PersonnelAccess() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void CrewChanged(int state);
	void DoUpZ();
	void DoUpZGrid();
	void DoPickVector();
	void EnableControls();
	void OfficersChanged(int state);
	void PassengersChanged(int state);
	void TroopsChanged(int state);
};

#endif // AuScVoidPropsDlg_H
