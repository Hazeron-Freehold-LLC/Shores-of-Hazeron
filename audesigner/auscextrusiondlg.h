//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscextrusiondlg.h,v 1.3 2022/02/19 18:22:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScExtrusionDlg_H
#define AuScExtrusionDlg_H

#include "audialog.h"

class QLineEdit;
class QSpinBox;

class AuScExtrusionDlg : public AuDialog
{
	Q_OBJECT;
public:
	enum MODE
	{
		HeightEntered,
		PickHeight,
		PickVector
	};

private:
	MODE								m_Mode;					// 0=height entered, 1=pick height, 2=pick vector
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::Type

	// Widgets
	QLineEdit*							m_HeightEdit;
	QSpinBox*							m_StepsSpin;

public:
	// 'partType' is a AuScPart::PART_TYPE
	AuScExtrusionDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScExtrusionDlg();

	double Height() const;

	// Valid on accept.
	inline MODE Mode() const
	{
		return m_Mode;
	}

	int Steps() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoHeightEntered();
	void DoPickHeight();
	void DoPickVector();
};

#endif // AuScExtrusionDlg_H
