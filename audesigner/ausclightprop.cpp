//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausclightprop.cpp,v 1.5 2022/02/19 18:25:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausclightprop.h"
//
#include "auglobal.h"
#include "aupushbutton.h"
#include "aulightcolorwidget.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

AuScLightProp::AuScLightProp(bool isOmni, const QColor& color, float attenMeters, float innerAngle, float outerAngle,
		QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Light Bulb Properties"));
	setAuDlgIcon(QIcon(":data/Light.png"));

	m_OmniRadio = new QRadioButton(tr("Omnidirectional"), this);
	m_OmniRadio->setChecked(isOmni);
	m_OmniRadio->setToolTip(tr("<html>"
				"Light is emitted uniformly in all directions. "
				"</html>"));
	connect(m_OmniRadio, SIGNAL(toggled(bool)), SLOT(RadioToggled(bool)));

	m_SpotRadio = new QRadioButton(tr("Spot Light"), this);
	m_SpotRadio->setChecked(!isOmni);
	m_SpotRadio->setToolTip(tr("<html>"
				"Light is restricted to a cone, emitted in a direction. "
				"</html>"));
	connect(m_SpotRadio, SIGNAL(toggled(bool)), SLOT(RadioToggled(bool)));

	QLabel* attenLb = new QLabel(tr("Attenuation Distance"), this);

	m_Atten = new QLineEdit(QString("%1").arg(attenMeters), this);
	m_Atten->setToolTip(tr("<html>"
				"Attenuation distance, in the range of 1 to 250 meters. "
				"</html>"));
	connect(m_Atten, SIGNAL(textChanged(const QString&)), SLOT(AttenChanged(const QString&)));

	QLabel* innerLb = new QLabel(tr("Inner Cone Angle"), this);

	if(isOmni)
		m_InnerCone = new QLineEdit(this);
	else
		m_InnerCone = new QLineEdit(QString("%1").arg(innerAngle), this);
	m_InnerCone->setToolTip(tr("<html>"
				"Inner cone angle, in the range of 0 to 180 degrees. "
				"The inner cone angle must be equal or less than the outer cone angle. "
				"</html>"));
	connect(m_InnerCone, SIGNAL(textChanged(const QString&)), SLOT(InnerConeChanged(const QString&)));

	QLabel* outerLb = new QLabel(tr("Outer Cone Angle"), this);

	if(isOmni)
		m_OuterCone = new QLineEdit(this);
	else
		m_OuterCone = new QLineEdit(QString("%1").arg(outerAngle), this);
	m_OuterCone->setToolTip(tr("<html>"
				"Outer cone angle, in the range of 0 to 180 degrees. "
				"The outer cone angle must equal or exceed the inner cone angle. "
				"</html>"));
	connect(m_OuterCone, SIGNAL(textChanged(const QString&)), SLOT(OuterConeChanged(const QString&)));

	QLabel* colorLb = new QLabel(tr("Light Color"), this);

	m_Color = new AuLightColorWidget(this);
	m_Color->setToolTip(tr("<html>"
				"Light intensity drops off with distance from the light. "
				"Light color is the illumination applied to objects at the attenuation distance away from the light. "
				"</html>"));
	m_Color->SetLightColor(color);

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setToolTip(tr("<html>"
				"Accepts the light bulb properties. "
				"</html>"));
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(accept()));

	m_DefaultSpacecraftLight = new QPushButton(tr("Spacecraft Light"), this);
	m_DefaultSpacecraftLight->setToolTip(tr("<html>"
				"Configures settings for a spacecraft headlight. "
				"Light color is carbon arc light. "
				"</html>"));
	connect(m_DefaultSpacecraftLight, SIGNAL(clicked()), SLOT(DefaultSpacecraftLight()));

	m_DefaultStreetLight = new QPushButton(tr("Street Light"), this);
	m_DefaultStreetLight->setToolTip(tr("<html>"
				"Configures settings for a street light. "
				"Light color is sodium vapor light. "
				"</html>"));
	connect(m_DefaultStreetLight, SIGNAL(clicked()), SLOT(DefaultStreetLight()));

	QLabel* figLb = new QLabel(tr("<html>"
				"<p>"
				"Intensity drops off with distance from the light. "
				"Intensity is 1 at the <i>attenuation distance</i>. "
				"That is the distance where the light's illumination equals its color property. "
				"</p>"
				"<p>"
				"Spot lights are described by two cones. "
				"Intensity is at its maximum inside the <i>inner cone</i>. "
				"Intensity drops off to zero from the limit of the inner cone to the limit of the <i>outer cone</i>. "
				"</p>"
				"<p>"
				"Angles are based on the spot light's facing direction. "
				"0%1 is a laser pointer. "
				"90%1 is a hemisphere. "
				"180%1 is omnidirectional. "
				"</p>"
				"<p>"
				"An omnidirectional light is a spot light with both cone angles at 180%1. "
				"</p>"
				"</html>").arg(AuGlobal::DegreeSymbol()), this);
	figLb->setWordWrap(true);
	figLb->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

	QLabel* fig = new QLabel(this);
	fig->setPixmap(QPixmap(":data/SpotLightCones.png"));

	QGridLayout* play = new QGridLayout;
	int row=0;
	play->addWidget(attenLb, row,0);
	play->addWidget(m_Atten, row,1);
	row++;
	play->addWidget(innerLb, row,0);
	play->addWidget(m_InnerCone, row,1);
	row++;
	play->addWidget(outerLb, row,0);
	play->addWidget(m_OuterCone, row,1);
	row++;
	play->addWidget(colorLb, row,0);
	play->addWidget(m_Color, row,1);
	row++;
	play->addWidget(fig, row,0);
	play->addWidget(figLb, row,1);
	row++;

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_DefaultSpacecraftLight);
	blay->addWidget(m_DefaultStreetLight);
	blay->addStretch();
	blay->addWidget(m_OkButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_OmniRadio);
	lay->addWidget(m_SpotRadio);
	lay->addLayout(play);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuScLightProp::~AuScLightProp()
{
}

void AuScLightProp::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuScLightProp::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/LightProp/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/LightProp/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/LightProp/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/LightProp/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScLightProp::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/LightProp/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/LightProp/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/LightProp/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/LightProp/SizeY",sz.height());
}

void AuScLightProp::EnableControls()
{
	bool okOk = true;
	if(AttenuationDistance() < 1 || AttenuationDistance() > 250)
		okOk = false;	// Attenuation distance must 1 to 250m.

	if(LightColor() == QColor(0,0,0))
		okOk = false;	// No black lights.

	bool isSpot = IsSpotlight();
	m_InnerCone->setEnabled(isSpot);
	m_OuterCone->setEnabled(isSpot);

	if(isSpot)
	{
		// Verify spotlight properties.
		float innerCone = InnerConeAngle();
		float outerCone = OuterConeAngle();
		if(innerCone > outerCone)
			okOk = false;
		else if(fabs(innerCone) > 180)
			okOk = false;
		else if(fabs(outerCone) > 180)
			okOk = false;
	}

	m_OkButton->setEnabled(okOk);
}

void AuScLightProp::RadioToggled(bool checked)
{
	Q_UNUSED(checked);
	EnableControls();
}

float AuScLightProp::AttenuationDistance() const
{
	return m_Atten->text().toFloat();
}

float AuScLightProp::InnerConeAngle() const
{
	if(m_InnerCone->text().isEmpty())
		return 1000;	// invalid
	return m_InnerCone->text().toFloat();
}

bool AuScLightProp::IsOmnidirectional() const
{
	return m_OmniRadio->isChecked();
}

bool AuScLightProp::IsSpotlight() const
{
	return m_SpotRadio->isChecked();
}

QColor AuScLightProp::LightColor() const
{
	return m_Color->LightColor();
}

float AuScLightProp::OuterConeAngle() const
{
	if(m_OuterCone->text().isEmpty())
		return 1000;	// invalid
	return m_OuterCone->text().toFloat();
}

void AuScLightProp::AttenChanged(const QString& text)
{
	Q_UNUSED(text);
	EnableControls();
}

void AuScLightProp::InnerConeChanged(const QString& text)
{
	Q_UNUSED(text);
	EnableControls();
}

void AuScLightProp::OuterConeChanged(const QString& text)
{
	Q_UNUSED(text);
	EnableControls();
}

void AuScLightProp::SetAttenuationDistance(float attenMeters)
{
	m_Atten->setText(tr("%1").arg(attenMeters));
}

void AuScLightProp::SetInnerConeAngle(float angleDegrees)
{
	m_InnerCone->setText(tr("%1").arg(angleDegrees));
}

void AuScLightProp::SetLightColor(const QColor& color)
{
	m_Color->SetLightColor(color);
}

void AuScLightProp::SetOmnidirectional()
{
	m_OmniRadio->setChecked(true);
}

void AuScLightProp::SetOuterConeAngle(float angleDegrees)
{
	m_OuterCone->setText(tr("%1").arg(angleDegrees));
}

void AuScLightProp::SetSpotlight()
{
	m_SpotRadio->setChecked(true);
}

void AuScLightProp::DefaultSpacecraftLight()
{
	SetSpotlight();
	SetLightColor(AuGLLight::ColorCarbonArc());
	SetAttenuationDistance(200);
	SetInnerConeAngle(20);
	SetOuterConeAngle(60);
}

void AuScLightProp::DefaultStreetLight()
{
	SetSpotlight();
	SetLightColor(AuGLLight::ColorSodiumVapor());
	SetAttenuationDistance(10);
	SetInnerConeAngle(30);
	SetOuterConeAngle(80);
}

