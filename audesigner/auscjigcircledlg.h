//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscjigcircledlg.h,v 1.2 2022/02/19 18:22:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScJigCircleDlg_H
#define AuScJigCircleDlg_H

#include "audialog.h"

class QLineEdit;
class QSpinBox;

class AuScJigCircleDlg : public AuDialog
{
	Q_OBJECT;
public:
	enum MODE
	{
		RadiusEntered,
		PickRadius
	};

private:
	MODE								m_Mode;					// 0=height entered, 1=pick height, 2=pick vector

	// Widgets
	QLineEdit*							m_RadiusEdit;
	QSpinBox*							m_StepsSpin;

public:
	AuScJigCircleDlg(QWidget* parent=0);
	virtual ~AuScJigCircleDlg();

	// Valid on accept.
	MODE Mode() const
	{
		return m_Mode;
	}

	double Radius() const;

	quint16 Steps() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoRadiusEntered();
	void DoPickRadius();
};

#endif // AuScJigCircleDlg_H
