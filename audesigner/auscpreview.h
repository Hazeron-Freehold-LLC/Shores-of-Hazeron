//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpreview.h,v 1.3 2022/02/19 18:22:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPreview_H
#define AuScPreview_H

#include "audialog.h"

class QLabel;

class AuScPreview : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QLabel*								m_RotAngle;

public:
	AuScPreview(QWidget* parent=0);
	virtual ~AuScPreview();

public slots:
	void CancelClicked();
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void TimeOfDaySliderChanged(int sliderVal);

signals:
	void TimeOfDayChanged(float angle);
};

#endif // AuScPreview_H
