//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscselfilluminationdlg.h,v 1.2 2022/02/19 18:22:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScSelfIlluminationDlg_H
#define AuScSelfIlluminationDlg_H

#include "audialog.h"

class QLineEdit;
class QSlider;

class AuScSelfIlluminationDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSlider*							m_SelfIllumSlider;
	QLabel*								m_SelfIllumValue;

public:
	AuScSelfIlluminationDlg(QWidget* parent=0);
	virtual ~AuScSelfIlluminationDlg();

	float SelfIllumination() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoNoSelfIllumination();
	void DoSelfIllumination();
	void SelfIlluminationChanged(int value);
};

#endif // AuScSelfIlluminationDlg_H
