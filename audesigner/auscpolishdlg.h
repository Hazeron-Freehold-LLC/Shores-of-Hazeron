//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpolishdlg.h,v 1.2 2022/02/19 18:22:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPolishDlg_H
#define AuScPolishDlg_H

#include "audialog.h"

class QLineEdit;
class QSlider;

class AuScPolishDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSlider*							m_ReflectSlider;
	QLabel*								m_ReflectValue;
	QLineEdit*							m_ShineEdit;

public:
	AuScPolishDlg(QWidget* parent=0);
	virtual ~AuScPolishDlg();

	float Reflectivity() const;

	float Shine() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoMatte();
	void DoPolish();
	void ReflectChanged(int value);
};

#endif // AuScPolishDlg_H
