//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausolodesignerdlg.h,v 1.7 2022/02/19 18:23:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSoloDesignerDlg_H
#define AuSoloDesignerDlg_H

#include "audef.h"
#include "audialog.h"
#include "audna.h"

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class AuSoloDesignerDlg : public AuDialog
{
	Q_OBJECT;
	AuDNA								m_DNA;

	// Widgets
	QPushButton*						m_DNAButton;
	QComboBox*							m_Gender;
	QComboBox*							m_Hand;
	QLineEdit*							m_Name;

public:
	AuSoloDesignerDlg(QWidget* parent=0);
	virtual ~AuSoloDesignerDlg();

	AuDNA DNA() const;

	GENDER Gender() const;

	HANDEDNESS Handedness() const;

	QString Name() const;

public slots:
	void DNAClicked();
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSoloDesignerDlg_H
