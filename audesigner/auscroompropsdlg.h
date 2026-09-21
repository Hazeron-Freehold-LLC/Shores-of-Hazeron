//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscroompropsdlg.h,v 1.2 2022/02/19 18:22:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScRoomPropsDlg_H
#define AuScRoomPropsDlg_H

#include "audef.h"
#include "audialog.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

class AuScRoomPropsDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QCheckBox*							m_IsSunlit;
	QComboBox*							m_InitLight;

public:
	AuScRoomPropsDlg(QWidget* parent=0);
	virtual ~AuScRoomPropsDlg();

	INTERIOR_LIGHT InitialLighting() const;

	bool IsSunlit() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoProps();
};

#endif // AuScRoomPropsDlg_H
