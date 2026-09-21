//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausclightprop.h,v 1.3 2022/02/19 18:22:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScLightProp_H
#define AuScLightProp_H

#include "audialog.h"

class AuLightColorWidget;
class QComboBox;
class QLineEdit;
class QPushButton;
class QRadioButton;

class AuScLightProp : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QLineEdit*							m_Atten;
	AuLightColorWidget*					m_Color;
	QPushButton*						m_DefaultSpacecraftLight;
	QPushButton*						m_DefaultStreetLight;
	QLineEdit*							m_InnerCone;
	QPushButton*						m_OkButton;
	QRadioButton*						m_OmniRadio;
	QLineEdit*							m_OuterCone;
	QRadioButton*						m_SpotRadio;

public:
	AuScLightProp(bool isOmni, const QColor& color, float attenMeters, float innerAngle, float outerAngle, QWidget* parent=0);
	virtual ~AuScLightProp();

	float AttenuationDistance() const;

	float InnerConeAngle() const;

	bool IsOmnidirectional() const;

	bool IsSpotlight() const;

	QColor LightColor() const;

	float OuterConeAngle() const;

	void SetAttenuationDistance(float attenMeters);

	void SetInnerConeAngle(float angleDegrees);

	void SetOmnidirectional();

	void SetOuterConeAngle(float angleDegrees);

	void SetSpotlight();

public slots:
	void AttenChanged(const QString& text);
	void DefaultSpacecraftLight();
	void DefaultStreetLight();
	void EnableControls();
	void InnerConeChanged(const QString& text);
	void OuterConeChanged(const QString& text);
	void RadioToggled(bool checked);
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	void SetLightColor(const QColor& color);
};

#endif // AuScLightProp_H
