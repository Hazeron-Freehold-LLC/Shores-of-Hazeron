//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhaircolorwidget.h,v 1.2 2022/02/19 18:19:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuHairColorWidget_H
#define AuHairColorWidget_H

#include <QWidget>

class AuColorButton;
class QComboBox;

class AuHairColorWidget : public QWidget
{
	Q_OBJECT;
	AuColorButton*						m_Color;
	QComboBox*							m_ColorPresets;

public:
	AuHairColorWidget(QWidget* parent=0);
	virtual ~AuHairColorWidget();

	QColor HairColor() const;

public slots:
	void SetHairColor(const QColor& color);

protected slots:
	void ColorChanged(QColor color);
	void ColorPresetChanged(int index);

signals:
	void HairColorChanged(QColor color);
};

#endif // AuHairColorWidget_H
