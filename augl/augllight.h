//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augllight.h,v 1.11 2022/02/19 18:29:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGLLight_H
#define AuGLLight_H

#include "augl.h"
#include "auid.h"
#include "aupoint.h"

class AUGL_DLL AuGLLight
{
public:
	enum ROLE
	{
		Unknown,
		ScenePerson,					// The person for whom the scene is rendered.
		SceneVehicle,					// Light of vehicle occupied by the scene person.
		SceneSpacecraft,				// Light of spacecraft occupied by the scene person.
		Person,							// A person's light other than the scene person.
		Vehicle,						// A vehicle light other than the scene person's vehicle.
		Spacecraft,						// A spacecraft light other than the scene person's spacecraft.
		Item,							// An item's light.
		HengeFire,						// The fire at a henge.
		Effect,							// Special effect: muzzle flash, explosion, etc.
	};

public:
	AuId								m_AtomId;				// Atom id who created the light.
	bool								m_bEnabled;
	float								m_kL;					// Linear attenuation
	float								m_kQ;					// Quadratic attenuation
	float								m_ColorR;				// range 0..1
	float								m_ColorG;				// range 0..1
	float								m_ColorB;				// range 0..1
	float								m_ColorA;				// range 0..1
	AuPoint								m_Position;
	float								m_cosOuter;
	float								m_cosInner;
	AuPoint								m_Direction;
	ROLE								m_Role;					// 0=scene atom, 1=scene atom's vehicle, 2=scene atom's spacecraft,
																// 3=creature, 4=vehicle, 5=spacecraft, 6=building, 7=item
public:
	inline AuGLLight(float kL, float kQ, float r, float g, float b, float a, float locX, float locY, float locZ,
		float cosOuter, float cosInner, float dirX, float dirY, float dirZ)
	{
		m_kL = kL;
		m_kQ = kQ;
		m_ColorR = r;
		m_ColorG = g;
		m_ColorB = b;
		m_ColorA = a;
		m_bEnabled = true;
		m_Position = AuPoint(locX, locY, locZ);
		m_cosOuter = cosOuter;
		m_cosInner = cosInner;
		m_Direction = AuPoint(dirX, dirY, dirZ);
		m_Role = Unknown;
	}

	inline AuGLLight(const AuId& atomId, ROLE role, float kL, float kQ, const QColor& color, const AuPoint& loc, float cosOuter,
			float cosInner, const AuPoint& dir)
	{
		m_AtomId = atomId;
		m_kL = kL;
		m_kQ = kQ;
		m_ColorR = color.redF();
		m_ColorG = color.greenF();
		m_ColorB = color.blueF();
		m_ColorA = color.alphaF();
		m_bEnabled = true;
		m_Position = loc;
		m_cosOuter = cosOuter;
		m_cosInner = cosInner;
		m_Direction = dir;
		m_Role = role;
	}

	inline AuGLLight(const AuId& atomId, ROLE role, float kL, float kQ, const QColor& color, const AuPoint& loc)
	{
		m_AtomId = atomId;
		m_kL = kL;
		m_kQ = kQ;
		m_ColorR = color.redF();
		m_ColorG = color.greenF();
		m_ColorB = color.blueF();
		m_ColorA = color.alphaF();
		m_bEnabled = true;
		m_Position = loc;
		m_cosOuter = -2;
		m_cosInner = -2;
		m_Direction = AuPoint(0,1,0);
		m_Role = role;
	}

	inline AuGLLight()
	{
		m_kL = 0;
		m_kQ = 0;
		m_ColorR = 0;
		m_ColorG = 0;
		m_ColorB = 0;
		m_ColorA = 0;
		m_bEnabled = false;
		m_Position = AuPoint(0,0);
		m_cosOuter = 0;
		m_cosInner = 0;
		m_Role = Unknown;
	}

public:
	inline const AuId& AtomId() const
	{
		return m_AtomId;
	}

	inline double EyeDistance() const
	{
		return m_Position.Length();
	}

	inline bool IsAtomLight(const AuId& atom) const
	{
		switch(m_Role)
		{
		case ScenePerson:
		case SceneVehicle:
		case SceneSpacecraft:
		case Person:
		case Vehicle:
		case Spacecraft:
		case Item:
			return m_AtomId == atom;
		case Unknown:
		case HengeFire:
		case Effect:
			break;
		}
		return false;
	}

	// Calculates linear attenuation when distances have been scaled. See DaWorldRingSection::RenderHullStatue and other Statue
	// rendering code for examples of this.
	inline float kL(float scale) const
	{
		float attenuation = sqrtf(1.f/m_kL);
		attenuation /= scale;
		return 1.f/(attenuation*attenuation);
	}

	// Calculates quadratic attenuation when distances have been scaled. See DaWorldRingSection::RenderHullStatue and other Statue
	// rendering code for examples of this.
	inline float kQ(float scale) const
	{
		float attenuation = sqrtf(1.f/m_kQ);
		attenuation /= scale;
		return 1.f/(attenuation*attenuation);
	}

	// Brightness of darkness inside somewhere that might have little bits of light, such as a spacecraft or building.
	// Slightly brighter than AmbientDarkness.
	inline static float AmbientDarkInterior()
	{
		return 12.f/256.f;
	}

	// Brightness of natural darkness. Let's avoid pure black zeros.
	inline static float AmbientDarkness()
	{
		return 8.f/256.f;
	}

	// Ambient light used on person when lamp is on.
	inline static float AmbientLitBackwash()
	{
		return .7f;
	}

	// Brightness of interior ambient lighting of buildings with lights on.
	inline static float AmbientLitBuilding()
	{
		return .75f;
	}

	// Ambience added to starlight in lit city.
	inline static float AmbientLitCity()
	{
		return .15f;
	}

	// Brightness of interior ambient lighting of space ships.
	inline static float AmbientLitInterior()
	{
		return .7f;
	}

	// Brightness of interior ambient lighting of space ships in dim light mode.
	inline static float AmbientLitInteriorDim()
	{
		return .2f;
	}

	// Returns the color of 'light', which is an index.
	static const QColor& Color(int light);

	static const QColor& ColorCandle();

	static const QColor& ColorCarbonArc();

	static const QColor& ColorFluorescentTubeCoolWhite();

	static const QColor& ColorFluorescentTubeDaylight();

	static const QColor& ColorHalogen();

	static const QColor& ColorHighPressureSodium();

	// 'k' is clamped to the range 1000k to 40000k.
	static QColor ColorKelvin(float k);

	static const QColor& ColorLowPressureSodium();

	static const QColor& ColorMercuryVapor();

	// Returns the default sodium vapor light color, either high or low pressure. Whichever I decide.
	static const QColor& ColorSodiumVapor();

	static const QColor& ColorTorch();

	static const QColor& ColorTungsten40W();

	static const QColor& ColorTungsten60W();

	static const QColor& ColorTungsten100W();

	static const QColor& ColorXenonStrobe();

	// Returns the number of defined lights.
	static int Count();

	// Brightness of ambient starlight at maximum galactic star density.
	inline static float MaxAmbientStarlight()
	{
		return .25f;
	}

	// Returns the name of 'light', which is an index.
	static const QString& Name(int light);

	/*
	// Light colors courtesy of http://planetpixelemporium.com/tutorialpages/light.html.
	//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+
	// Black Body Illuminants
	// Candle
	inline static int CandleR() { return (255); }
	inline static int CandleG() { return (147); }
	inline static int CandleB() { return (41); }
	inline static float CandleRf() { return ((float)CandleR()/255.f); }
	inline static float CandleGf() { return ((float)CandleG()/255.f); }
	inline static float CandleBf() { return ((float)CandleB()/255.f); }
	inline static QColor CandleColor() { return QColor(CandleR(), CandleG(), CandleB()); }

	// Tungsten40w
	inline static int Tungsten40wR() { return (255); }
	inline static int Tungsten40wG() { return (197); }
	inline static int Tungsten40wB() { return (143); }
	inline static float Tungsten40wRf() { return ((float)Tungsten40wR()/255.f); }
	inline static float Tungsten40wGf() { return ((float)Tungsten40wG()/255.f); }
	inline static float Tungsten40wBf() { return ((float)Tungsten40wB()/255.f); }
	inline static QColor Tungsten40wColor() { return QColor(Tungsten40wR(), Tungsten40wG(), Tungsten40wB()); }

	// Tungsten100w
	inline static int Tungsten100wR() { return (255); }
	inline static int Tungsten100wG() { return (214); }
	inline static int Tungsten100wB() { return (170); }
	inline static float Tungsten100wRf() { return ((float)Tungsten100wR()/255.f); }
	inline static float Tungsten100wGf() { return ((float)Tungsten100wG()/255.f); }
	inline static float Tungsten100wBf() { return ((float)Tungsten100wB()/255.f); }
	inline static QColor Tungsten100wColor() { return QColor(Tungsten100wR(), Tungsten100wG(), Tungsten100wB()); }

	// Halogen
	inline static int HalogenR() { return (255); }
	inline static int HalogenG() { return (241); }
	inline static int HalogenB() { return (224); }
	inline static float HalogenRf() { return ((float)HalogenR()/255.f); }
	inline static float HalogenGf() { return ((float)HalogenG()/255.f); }
	inline static float HalogenBf() { return ((float)HalogenB()/255.f); }
	inline static QColor HalogenColor() { return QColor(HalogenR(), HalogenG(), HalogenB()); }

	// Carbon Arc
	inline static int CarbonArcR() { return (255); }
	inline static int CarbonArcG() { return (250); }
	inline static int CarbonArcB() { return (244); }
	inline static float CarbonArcRf() { return ((float)CarbonArcR()/255.f); }
	inline static float CarbonArcGf() { return ((float)CarbonArcG()/255.f); }
	inline static float CarbonArcBf() { return ((float)CarbonArcB()/255.f); }
	inline static QColor CarbonArcColor() { return QColor(CarbonArcR(), CarbonArcG(), CarbonArcB()); }

	//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+
	// Fluorescent Lights
	// Warm Fluorescent
	inline static int WarmFluorescentR() { return (255); }
	inline static int WarmFluorescentG() { return (244); }
	inline static int WarmFluorescentB() { return (229); }
	inline static float WarmFluorescentRf() { return ((float)WarmFluorescentR()/255.f); }
	inline static float WarmFluorescentGf() { return ((float)WarmFluorescentG()/255.f); }
	inline static float WarmFluorescentBf() { return ((float)WarmFluorescentB()/255.f); }
	inline static QColor WarmFluorescentColor() { return QColor(WarmFluorescentR(), WarmFluorescentG(), WarmFluorescentB()); }

	// Standard Fluorescent
	inline static int StandardFluorescentR() { return (244); }
	inline static int StandardFluorescentG() { return (255); }
	inline static int StandardFluorescentB() { return (250); }
	inline static float StandardFluorescentRf() { return ((float)StandardFluorescentR()/255.f); }
	inline static float StandardFluorescentGf() { return ((float)StandardFluorescentG()/255.f); }
	inline static float StandardFluorescentBf() { return ((float)StandardFluorescentB()/255.f); }
	inline static QColor StandardFluorescentColor() { return QColor(StandardFluorescentR(), StandardFluorescentG(),
			StandardFluorescentB()); }

	// Cool White Fluorescent
	inline static int CoolWhiteFluorescentR() { return (212); }
	inline static int CoolWhiteFluorescentG() { return (235); }
	inline static int CoolWhiteFluorescentB() { return (255); }
	inline static float CoolWhiteFluorescentRf() { return ((float)CoolWhiteFluorescentR()/255.f); }
	inline static float CoolWhiteFluorescentGf() { return ((float)CoolWhiteFluorescentG()/255.f); }
	inline static float CoolWhiteFluorescentBf() { return ((float)CoolWhiteFluorescentB()/255.f); }
	inline static QColor CoolWhiteFluorescentColor() { return QColor(CoolWhiteFluorescentR(), CoolWhiteFluorescentG(),
			CoolWhiteFluorescentB()); }

	// Full Spectrum Fluorescent
	inline static int FullSpectrumFluorescentR() { return (255); }
	inline static int FullSpectrumFluorescentG() { return (244); }
	inline static int FullSpectrumFluorescentB() { return (242); }
	inline static float FullSpectrumFluorescentRf() { return ((float)FullSpectrumFluorescentR()/255.f); }
	inline static float FullSpectrumFluorescentGf() { return ((float)FullSpectrumFluorescentG()/255.f); }
	inline static float FullSpectrumFluorescentBf() { return ((float)FullSpectrumFluorescentB()/255.f); }
	inline static QColor FullSpectrumFluorescentColor() { return QColor(FullSpectrumFluorescentR(), FullSpectrumFluorescentG(),
			FullSpectrumFluorescentB()); }

	// Grow Light Fluorescent
	inline static int GrowLightFluorescentR() { return (255); }
	inline static int GrowLightFluorescentG() { return (239); }
	inline static int GrowLightFluorescentB() { return (247); }
	inline static float GrowLightFluorescentRf() { return ((float)GrowLightFluorescentR()/255.f); }
	inline static float GrowLightFluorescentGf() { return ((float)GrowLightFluorescentG()/255.f); }
	inline static float GrowLightFluorescentBf() { return ((float)GrowLightFluorescentB()/255.f); }
	inline static QColor GrowLightFluorescentColor() { return QColor(GrowLightFluorescentR(), GrowLightFluorescentG(),
			GrowLightFluorescentB()); }

	// Black Light Fluorescent
	inline static int BlackLightFluorescentR() { return (167); }
	inline static int BlackLightFluorescentG() { return (0); }
	inline static int BlackLightFluorescentB() { return (255); }
	inline static float BlackLightFluorescentRf() { return ((float)BlackLightFluorescentR()/255.f); }
	inline static float BlackLightFluorescentGf() { return ((float)BlackLightFluorescentG()/255.f); }
	inline static float BlackLightFluorescentBf() { return ((float)BlackLightFluorescentB()/255.f); }
	inline static QColor BlackLightFluorescentColor() { return QColor(BlackLightFluorescentR(), BlackLightFluorescentG(),
			BlackLightFluorescentB()); }

	//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+
	// Gaseous Lights
	// Mercury Vapor
	inline static int MercuryVaporR() { return (216); }
	inline static int MercuryVaporG() { return (247); }
	inline static int MercuryVaporB() { return (255); }
	inline static float MercuryVaporRf() { return ((float)MercuryVaporR()/255.f); }
	inline static float MercuryVaporGf() { return ((float)MercuryVaporG()/255.f); }
	inline static float MercuryVaporBf() { return ((float)MercuryVaporB()/255.f); }
	inline static QColor MercuryVaporColor() { return QColor(MercuryVaporR(), MercuryVaporG(), MercuryVaporB()); }

	// Sodium Vapor
	inline static int SodiumVaporR() { return (255); }
	inline static int SodiumVaporG() { return (209); }
	inline static int SodiumVaporB() { return (178); }
	inline static float SodiumVaporRf() { return ((float)SodiumVaporR()/255.f); }
	inline static float SodiumVaporGf() { return ((float)SodiumVaporG()/255.f); }
	inline static float SodiumVaporBf() { return ((float)SodiumVaporB()/255.f); }
	inline static QColor SodiumVaporColor() { return QColor(SodiumVaporR(), SodiumVaporG(), SodiumVaporB()); }

	// Metal Halide
	inline static int MetalHalideR() { return (242); }
	inline static int MetalHalideG() { return (252); }
	inline static int MetalHalideB() { return (255); }
	inline static float MetalHalideRf() { return ((float)MetalHalideR()/255.f); }
	inline static float MetalHalideGf() { return ((float)MetalHalideG()/255.f); }
	inline static float MetalHalideBf() { return ((float)MetalHalideB()/255.f); }
	inline static QColor MetalHalideColor() { return QColor(MetalHalideR(), MetalHalideG(), MetalHalideB()); }

	// High Pressure Sodium
	inline static int HighPressureSodiumR() { return (255); }
	inline static int HighPressureSodiumG() { return (183); }
	inline static int HighPressureSodiumB() { return (76); }
	inline static float HighPressureSodiumRf() { return ((float)HighPressureSodiumR()/255.f); }
	inline static float HighPressureSodiumGf() { return ((float)HighPressureSodiumG()/255.f); }
	inline static float HighPressureSodiumBf() { return ((float)HighPressureSodiumB()/255.f); }
	inline static QColor HighPressureSodiumColor() { return QColor(HighPressureSodiumR(), HighPressureSodiumG(),
			HighPressureSodiumB()); }
	*/
};

#endif // AuGLLight_H
