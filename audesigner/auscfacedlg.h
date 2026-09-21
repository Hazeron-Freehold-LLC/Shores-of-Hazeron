//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscfacedlg.h,v 1.2 2022/02/19 18:22:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScFaceDlg_H
#define AuScFaceDlg_H

#include "audialog.h"

class QLineEdit;
class QSpinBox;

class AuScFaceDlg : public AuDialog
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
	AuScFaceDlg(QWidget* parent=0);
	virtual ~AuScFaceDlg();

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

#endif // AuScFaceDlg_H
