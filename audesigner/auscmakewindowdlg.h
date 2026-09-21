//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakewindowdlg.h,v 1.2 2022/02/19 18:22:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScMakeWindowDlg_H
#define AuScMakeWindowDlg_H

#include "audialog.h"

class QSlider;

class AuScMakeWindowDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSlider*							m_TransparencySlider;
	QLabel*								m_TransparencyValue;

public:
	AuScMakeWindowDlg(QWidget* parent=0);
	virtual ~AuScMakeWindowDlg();

	// Returns 0=invisible to 1=translucent
	float Opacity() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoTranslucent();
	void DoTransparent();
	void TransparencyChanged(int value);
};

#endif // AuScMakeWindowDlg_H
