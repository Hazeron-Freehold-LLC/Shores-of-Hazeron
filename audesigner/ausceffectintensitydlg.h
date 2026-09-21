//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceffectintensitydlg.h,v 1.2 2022/02/19 18:22:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScEffectIntensityDlg_H
#define AuScEffectIntensityDlg_H

#include "audialog.h"

class QLineEdit;

class AuScEffectIntensityDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_Intensity;

public:
	AuScEffectIntensityDlg(QWidget* parent=0);
	virtual ~AuScEffectIntensityDlg();

	float Intensity() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoIt();
};

#endif // AuScEffectIntensityDlg_H
