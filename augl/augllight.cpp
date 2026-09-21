//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augllight.cpp,v 1.7 2022/02/19 18:29:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augllight.h"
//

class AuGLLightDef
{
public:
	QColor								m_Color;
	QString								m_Name;

public:
	AuGLLightDef(const QString& name, const QColor& color)
	{
		m_Color = color;
		m_Name = name;
	}
};

// These are ordered more-or-less according to Kelvin value.
// WARNING: Changes to this table may require updates to hard coded indexes in ::Color functions.
AuGLLightDef s_Lights[] = {
	AuGLLightDef(QObject::tr("Match"), QColor(255,140,39)),	// 1700K
	AuGLLightDef(QObject::tr("Candle"), QColor(255,149,44)),	// 1900K
	AuGLLightDef(QObject::tr("Torch"), QColor(255,151,45)),
	AuGLLightDef(QObject::tr("Low Pressure Sodium Lamp"), QColor(255,160,75)),	// 2000K
	AuGLLightDef(QObject::tr("High Pressure Sodium Lamp"), QColor(255,180,112)),	// 2300K
	AuGLLightDef(QObject::tr("Tungsten Lamp 40W"), QColor(255,200,140)),	// 2600K
	AuGLLightDef(QObject::tr("Tungsten Lamp 60W"), QColor(255,205,150)),	// 2700K
	AuGLLightDef(QObject::tr("Tungsten Lamp 100W"), QColor(255,219,172)),	// 2850K
	AuGLLightDef(QObject::tr("Fluorescent Tube Warm White"), QColor(255,238,203)),	// 3000K
	AuGLLightDef(QObject::tr("Halogen Lamp"), QColor(255,248,224)),	// 3200K
	AuGLLightDef(QObject::tr("Fluorescent Tube Cool White"), QColor(255,249,230)),	// 4000K
	AuGLLightDef(QObject::tr("White LED"), QColor(255,250,240)),	// 4500K
	AuGLLightDef(QObject::tr("Carbon Arc"), QColor(255,250,244)),
	AuGLLightDef(QObject::tr("Xenon Strobe"), QColor(255,253,250)),	// 5500K
	AuGLLightDef(QObject::tr("Fluorescent Tube Daylight"), QColor(249,251,255)),	// 6500K
	AuGLLightDef(QObject::tr("Metal Halide"), QColor(242,252,255)),
	AuGLLightDef(QObject::tr("Mercury Vapor"), QColor(216,247,255)),
	AuGLLightDef(QObject::tr("Fluorescent Tube Skywhite"), QColor(213,230,255)),	// 8000K
};
int s_LightCount = sizeof(s_Lights) / sizeof(AuGLLightDef);

//static
const QColor& AuGLLight::ColorCandle()
{
	return s_Lights[1].m_Color;
}

//static
const QColor& AuGLLight::ColorCarbonArc()
{
	return s_Lights[12].m_Color;
}

//static
const QColor& AuGLLight::ColorFluorescentTubeCoolWhite()
{
	return s_Lights[10].m_Color;
}

//static
const QColor& AuGLLight::ColorFluorescentTubeDaylight()
{
	return s_Lights[14].m_Color;
}

//static
const QColor& AuGLLight::ColorHalogen()
{
	return s_Lights[9].m_Color;
}

//static
const QColor& AuGLLight::ColorHighPressureSodium()
{
	return s_Lights[4].m_Color;
}

//static
// https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
QColor AuGLLight::ColorKelvin(float k)
{
	// Temperature must fall between 1000 and 40000 degrees kelvin
	float t = k / 100;
	if(t < 10)
		t = 10;
	else if(t > 400)
		t = 400;

	// Calculate Red
	float r;
	if(t <= 66)
		r = 255;
	else
	{
		r = t - 60;
		r = 329.698727446f * powf(r, -0.1332047592f);
		if(r < 0)
			r = 0;
		else if(r > 255)
			r = 255;
	}

	// Calculate Green
	float g;
	if(t <= 66)
	{
		g = t;
		g = 99.4708025861f * logf(g) - 161.1195681661f;
		if(g < 0)
			g = 0;
		else if(g > 255)
			g = 255;
	}
	else
	{
		g = t - 60;
		g = 288.1221695283f * powf(g, -0.0755148492f);
		if(g < 0)
			g = 0;
		else if(g > 255)
			g = 255;
	}

	// Calculate Blue
	float b;
	if(t >= 66)
	{
		b = 255;
	}
	else
	{
		if(t <= 19)
			b = 0;
		else
		{
			b = t - 10;
			b = 138.5177312231f * logf(b) - 305.0447927307f;
			if(b < 0)
				b = 0;
			else if(b > 255)
				b = 255;
		}
	}

	int ir = (int)roundf(r);
	int ig = (int)roundf(g);
	int ib = (int)roundf(b);
	return QColor(ir,ig,ib);
}

//static
const QColor& AuGLLight::ColorLowPressureSodium()
{
	return s_Lights[3].m_Color;
}

//static
const QColor& AuGLLight::ColorMercuryVapor()
{
	return s_Lights[16].m_Color;
}

//static
const QColor& AuGLLight::ColorSodiumVapor()
{
	return s_Lights[4].m_Color;	// high pressure sodium
}

//static
const QColor& AuGLLight::ColorTungsten40W()
{
	return s_Lights[5].m_Color;
}

//static
const QColor& AuGLLight::ColorTungsten60W()
{
	return s_Lights[6].m_Color;
}

//static
const QColor& AuGLLight::ColorTungsten100W()
{
	return s_Lights[7].m_Color;
}

//static
const QColor& AuGLLight::ColorTorch()
{
	return s_Lights[2].m_Color;
}

//static
const QColor& AuGLLight::ColorXenonStrobe()
{
	return s_Lights[13].m_Color;
}

//static
const QColor& AuGLLight::Color(int light)
{
	if(light < 0 || light >= s_LightCount)
		light = 0;
	return s_Lights[light].m_Color;
}

//static
int AuGLLight::Count()
{
	return s_LightCount;
}

//static
const QString& AuGLLight::Name(int light)
{
	if(light < 0 || light >= s_LightCount)
		light = 0;
	return s_Lights[light].m_Name;
}
