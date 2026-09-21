//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausclightlensprop.cpp,v 1.5 2022/02/19 18:25:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausclightlensprop.h"
//
#include "auglobal.h"
#include "aulightcolorwidget.h"
#include "aupushbutton.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

AuScLightLensProp::AuScLightLensProp(const QColor& defaultColor, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Light Lens Properties"));
	setAuDlgIcon(QIcon(":data/ScHeadlight.png"));

	QLabel* colorLb = new QLabel(tr("Light Color"), this);

	m_Color = new AuLightColorWidget(this);
	m_Color->setToolTip(tr("<html>"
				"Light color is the color of the lens when the light is on. "
				"When used with a light bulb, the lens color should match the light color. "
				"</html>"));
	m_Color->SetLightColor(defaultColor);

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setToolTip(tr("<html>"
				"Accepts the light lens properties. "
				"</html>"));
	m_OkButton->setAutoDefault(true);
	m_OkButton->setDefault(true);
	connect(m_OkButton, SIGNAL(clicked()), SLOT(accept()));

	m_DefaultSpacecraftLight = new QPushButton(tr("Spacecraft Light"), this);
	m_DefaultSpacecraftLight->setToolTip(tr("<html>"
				"Selects carbon arc light color, typical for a spacecraft headlight. "
				"</html>"));
	connect(m_DefaultSpacecraftLight, SIGNAL(clicked()), SLOT(DefaultSpacecraftLight()));

	m_DefaultStreetLight = new QPushButton(tr("Street Light"), this);
	m_DefaultStreetLight->setToolTip(tr("<html>"
				"Selects sodium vapor light color, typical for a street light. "
				"</html>"));
	connect(m_DefaultStreetLight, SIGNAL(clicked()), SLOT(DefaultStreetLight()));

	QGridLayout* play = new QGridLayout;
	int row=0;
	play->addWidget(colorLb, row,0);
	play->addWidget(m_Color, row,1);
	row++;

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_DefaultSpacecraftLight);
	blay->addWidget(m_DefaultStreetLight);
	blay->addStretch();
	blay->addWidget(m_OkButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(play);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuScLightLensProp::~AuScLightLensProp()
{
}

void AuScLightLensProp::setVisible(bool visible)
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

void AuScLightLensProp::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/LightLensProp/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/LightLensProp/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/LightLensProp/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/LightLensProp/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScLightLensProp::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/LightLensProp/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/LightLensProp/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/LightLensProp/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/LightLensProp/SizeY",sz.height());
}

void AuScLightLensProp::EnableControls()
{
	bool okOk = true;
	if(LightColor() == QColor(0,0,0))
		okOk = false;	// No black lights.
	m_OkButton->setEnabled(okOk);
}

QColor AuScLightLensProp::LightColor() const
{
	return m_Color->LightColor();
}

void AuScLightLensProp::SetLightColor(const QColor& color)
{
	m_Color->SetLightColor(color);
}

void AuScLightLensProp::DefaultSpacecraftLight()
{
	SetLightColor(AuGLLight::ColorCarbonArc());
}

void AuScLightLensProp::DefaultStreetLight()
{
	SetLightColor(AuGLLight::ColorSodiumVapor());
}

