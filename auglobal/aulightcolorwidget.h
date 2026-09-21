//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulightcolorwidget.h,v 1.3 2022/02/19 18:31:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLightColorWidget_H
#define AuLightColorWidget_H

#include <QWidget>

class AuColorButton;
class QComboBox;

class AuLightColorWidget : public QWidget
{
	Q_OBJECT;
	AuColorButton*						m_Color;
	QComboBox*							m_ColorPresets;

public:
	AuLightColorWidget(QWidget* parent=0);
	virtual ~AuLightColorWidget();

	QColor LightColor() const;

public slots:
	void SetLightColor(const QColor& color);

protected slots:
	void ColorChanged(QColor color);
	void ColorPresetChanged(int index);

signals:
	void LightColorChanged(QColor color);
};

#endif // AuLightColorWidget_H
