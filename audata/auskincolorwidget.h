//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auskincolorwidget.h,v 1.2 2022/02/19 18:19:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSkinColorWidget_H
#define AuSkinColorWidget_H

#include <QWidget>

class AuColorButton;
class QComboBox;

class AuSkinColorWidget : public QWidget
{
	Q_OBJECT;
	AuColorButton*						m_Color;
	QComboBox*							m_ColorPresets;

public:
	AuSkinColorWidget(QWidget* parent=0);
	virtual ~AuSkinColorWidget();

	QColor SkinColor() const;

public slots:
	void SetSkinColor(const QColor& color);

protected slots:
	void ColorChanged(QColor color);
	void ColorPresetChanged(int index);

signals:
	void SkinColorChanged(QColor color);
};

#endif // AuSkinColorWidget_H
