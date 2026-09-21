//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscspherelatlongdlg.h,v 1.2 2022/02/19 18:22:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScSphereLatLongDlg_H
#define AuScSphereLatLongDlg_H

#include "audialog.h"

class QSpinBox;

class AuScSphereLatLongDlg : public AuDialog
{
	Q_OBJECT;
private:
	//AuScPart::PART_TYPE
	quint8								m_PartType;				// AuScPart::TypeStatic.

	// Widgets
	QSpinBox*							m_LatStepsSpin;
	QSpinBox*							m_LonStepsSpin;

public:
	AuScSphereLatLongDlg(quint8 partType, QWidget* parent=0);
	virtual ~AuScSphereLatLongDlg();

	quint16 LatitudeSteps() const;

	quint16 LongitudeSteps() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void OkPressed();
};

#endif // AuScSphereLatLongDlg_H
