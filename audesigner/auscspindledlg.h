//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscspindledlg.h,v 1.2 2022/02/19 18:22:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScSpindleDlg_H
#define AuScSpindleDlg_H

#include "audialog.h"

class QRadioButton;
class QSpinBox;

class AuScSpindleDlg : public AuDialog
{
	Q_OBJECT;
private:
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::TypeStatic.

	// Widgets
	QRadioButton*						m_XAxis;
	QRadioButton*						m_YAxis;
	QRadioButton*						m_ZAxis;
	QSpinBox*							m_StepsSpin;

public:
	AuScSpindleDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScSpindleDlg();

	// 0=x, 1=y, 2=z
	int Axis() const;

	quint16 Steps() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoStepsEntered();
};

#endif // AuScSpindleDlg_H
