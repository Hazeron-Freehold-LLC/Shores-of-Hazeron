//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausclightlensprop.h,v 1.3 2022/02/19 18:22:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScLightLensProp_H
#define AuScLightLensProp_H

#include "audialog.h"

class AuLightColorWidget;
class QComboBox;
class QPushButton;

class AuScLightLensProp : public AuDialog
{
	Q_OBJECT;

	// Widgets
	AuLightColorWidget*					m_Color;
	QPushButton*						m_DefaultSpacecraftLight;
	QPushButton*						m_DefaultStreetLight;
	QPushButton*						m_OkButton;

public:
	AuScLightLensProp(const QColor& defaultColor, QWidget* parent=0);
	virtual ~AuScLightLensProp();

	QColor LightColor() const;

	void SetLightColor(const QColor& color);

public slots:
	void DefaultSpacecraftLight();
	void DefaultStreetLight();
	void EnableControls();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScLightLensProp_H
