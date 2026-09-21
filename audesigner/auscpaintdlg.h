//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpaintdlg.h,v 1.2 2022/02/19 18:22:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPaintDlg_H
#define AuScPaintDlg_H

#include "audialog.h"

class AuColorButton;
class QComboBox;
class QRadioButton;
class QSlider;

class AuScPaintDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuColorButton*						m_Color;
	QSlider*							m_TransparencySlider;
	QLabel*								m_TransparencyValue;
	QRadioButton*						m_RadioColor;
	QRadioButton*						m_RadioTheme;
	QComboBox*							m_ThemeCombo;

public:
	AuScPaintDlg(const QList<QColor>& paintInUse, QWidget* parent=0);
	virtual ~AuScPaintDlg();

	quint8 Opacity() const;

	// Returns the paint color, with alpha set to the opacity.
	QColor Paint() const;

	//AuScFace::COLOR_THEME
	quint8 Theme() const;

	bool UseTheme() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoNoPaint();
	void DoPaint();
	void TransparencyChanged(int value);
};

#endif // AuScPaintDlg_H
