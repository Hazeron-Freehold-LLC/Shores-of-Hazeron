//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpreview.h,v 1.3 2022/02/19 18:21:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdPreview_H
#define AuBdPreview_H

#include "audialog.h"

class QLabel;

class AuBdPreview : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QLabel*								m_RotAngle;

public:
	AuBdPreview(bool hasPower, QWidget* parent=0);
	virtual ~AuBdPreview();

public slots:
	void CancelClicked();
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void HasPowerStateChanged(int checkState);
	void TimeOfDaySliderChanged(int sliderVal);

signals:
	void HasPowerChanged(bool hasPower);
	void TimeOfDayChanged(float angle);
};

#endif // AuBdPreview_H
