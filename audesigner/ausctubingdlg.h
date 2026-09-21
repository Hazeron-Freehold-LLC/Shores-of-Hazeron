//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctubingdlg.h,v 1.2 2022/02/19 18:22:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScTubingDlg_H
#define AuScTubingDlg_H

#include "audialog.h"

class QLineEdit;
class QSpinBox;;

class AuScTubingDlg : public AuDialog
{
	Q_OBJECT;
public:
	enum MODE
	{
		DimensionsEntered,
		DrawShape
	};

private:
	MODE								m_Mode;					// MODE
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::Type

	// Widgets
	QLineEdit*							m_DiameterEdit;
	QSpinBox*							m_SidesSpin;

public:
	AuScTubingDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScTubingDlg();

	double Diameter() const;

	// Valid on accept.
	MODE Mode() const
	{
		return m_Mode;
	}

	int Sides() const;

	bool SmoothSides() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoDimensionsEntered();
	void DoDrawShape();
};

#endif // AuScTubingDlg_H
