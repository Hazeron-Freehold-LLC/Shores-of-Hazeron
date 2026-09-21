//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscopacitydlg.h,v 1.2 2022/02/19 18:22:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScOpacityDlg_H
#define AuScOpacityDlg_H

#include "audialog.h"

class QSlider;

class AuScOpacityDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSlider*							m_TransparencySlider;
	QLabel*								m_TransparencyValue;

public:
	AuScOpacityDlg(QWidget* parent=0);
	virtual ~AuScOpacityDlg();

	// Returns 0=invisible to 1=opaque
	float Opacity() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoOpaque();
	void DoTransparent();
	void TransparencyChanged(int value);
};

#endif // AuScOpacityDlg_H
