//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augleffects.h,v 1.166 2022/04/15 20:33:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGLEffects_H
#define AuGLEffects_H

#include "audef.h"
#include "auelapsedtimer.h"
#include "augllight.h"
#include "auglsl.h"
#include "auid.h"
#include "aupoint.h"
#include <QColor>
#include <QFile>
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QString>

using namespace std;
#include <unordered_map>

#define STATIC_TEXTURE_ID	1	// Common id for textures that must remain resident until the application exits.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Used as indexes in AuGLEffects::RenderItemEffect. Changes here should be reflected there.
// Sync with WEAPONTYPE and WEAPONEFFECT
enum ITEMEFFECT
{
	IE_Acid,
	IE_Suffocate,
	IE_Stun,
	IE_Poison,
	IE_Radiation,
	IE_Disease,
	IE_Electric,
	IE_Bludgeon,
	IE_Cold,
	IE_Crush,
	IE_Sonic,
	IE_Heat,
	IE_Pierce,
	IE_Implant,
	IE_Slash,
	IE_Food,
	IE_Liquor,
	IE_Revive,
	IE_CureDisease,
	IE_CurePoison,
	IE_Heal,
	IE_RepairMetal,
	IE_RepairMagmium,
	IE_RepairVulcium,
	IE_RepairAdamantine,
	IE_RepairModule,
	IE_RepairElectronicPart,
	IE_RepairMechanicalPart,
	IE_Butcher,
	IE_Dig,
	IE_Light,
	IE_Visor,	// helmet opens and closes
	IE_UpgradeModule,				// Item upgrades a SYSTEM of a spacecraft.
	IE_SampleDNA,
	IE_PowerPlantOn,				// Used for a fighter hud indicator. Not really an item effect.
	IE_ReticleBox,					// Used for a target lock indicator. Not really an item effect.
	IE_ReticleDiamond,				// Used for a target lock indicator. Not really an item effect.
	IE_PoliticalStance,				// Used for an on screen indicator. Not really an item effect.
	IE_System_CommandAndControl,	// Used to display a SYSTEM icon.
	IE_System_Fuel,					// Used to display a SYSTEM icon.
	IE_System_ManeuverDrive,		// Used to display a SYSTEM icon.
	IE_System_Hold,					// Used to display a SYSTEM icon.
	IE_System_Hull,					// Used to display a SYSTEM icon.
	IE_System_VehicleLaunch,		// Used to display a SYSTEM icon.
	IE_System_LifeSupport,			// Used to display a SYSTEM icon.
	IE_System_PowerPlant,			// Used to display a SYSTEM icon.
	IE_System_Sensor,				// Used to display a SYSTEM icon.
	IE_System_Shield,				// Used to display a SYSTEM icon.
	IE_System_SurgeryUnit,			// Used to display a SYSTEM icon.
	IE_System_Transporter,			// Used to display a SYSTEM icon.
	IE_System_Turret,				// Used to display a SYSTEM icon.
	IE_System_Weapon,				// Used to display a SYSTEM icon.
	IE_System_FTLWormholeDrive,		// Used to display a SYSTEM icon of a wormhole FTL drive.
	IE_System_Capacitor,			// Used to display a SYSTEM icon.
	IE_System_VehicleLanding,		// Used to display a SYSTEM icon.
	IE_System_Door,					// Used to display a SYSTEM icon of a door.
	IE_System_FTLWarpDrive,			// Used to display a SYSTEM icon of a warp FTL drive.
	IE_System_Berth,				// Used to display a SYSTEM icon of a berth.
	IE_SpecWeaponNotReady,			// Used to display icon for special weapon not ready.
	IE_WM_Area,						// Used to display a weapon mode icon.
	IE_WM_Contact,					// Used to display a weapon mode icon.
	IE_WM_Detonator,				// Used to display a weapon mode icon.
	IE_WM_Fuse,						// Used to display a weapon mode icon.
	IE_WM_Projectile,				// Used to display a weapon mode icon.
	IE_WM_Psionic,					// Used to display a weapon mode icon.
	IE_Decontaminate,
	IE_MotherShipReticle,			// Used for mother ship reticle in vehicle.
	IE_StallWarning,
	IE_DisassembleModule,
};

// Delivery mode of weapons.
enum WEAPONMODE
{
	WM_Not = 0,			// Not a weapon.
	WM_Area = 1,
	WM_Contact = 2,
	WM_Detonator = 3,	// Weapon is remotely detonated after placement.
	WM_Fuse = 4,		// Weapon detonates a short time after placement.
	WM_Projectile = 5,
	WM_Psionic = 6,
	WM_Internal = 7,
};

// Weapon effect decribes the type of damage inflicted by a weapon.
// Weapon effect is also used to describe the vulnerability type of armor.
// Some weapon effects only apply to weapon damage or armor vulnerability but
// not both.
enum WEAPONEFFECT
{
	// NOTE: Weapon effects 0..7 MUST match AuFloraDNA::CHEMICALs.
	//       AuFloraDNA::Chemical is only 3 bits wide.
	//       These are the chemical defenses found naturally in plants.
	// NOTE: Weapon effects 0..15 MUST match DhDNA::WEAPONTYPEs...and ITEMEFFECTs
	//       DhDNA::WeaponType is only 4 bits wide.
	//       These are the weapon types found naturally in creatures.
	WE_None,
	WE_Acid,
	WE_Suffocate,
	WE_Stun,		// 3, Hit points inflicted become time paralyzed, in seconds?
	WE_Poison,		// 4
	WE_Radiation,	// 5
	WE_Disease,		// 6
	WE_Electric, 
	WE_Bludgeon,	// 8
	WE_Cold,		// 9
	WE_Crush,		// 10
	WE_Sonic,		// 11
	WE_Heat,		// 12
	WE_Pierce,		// 13
	WE_Implant,
	WE_Slash,		// 15
	// Remaining weapon types are not used by creatures at all.
	WE_Generic,			// shield vulnerability category
	WE_Unused,
	WE_Bleeding,		// Used when creature is dying but not dead.
	WE_Starving,		// Used when creature takes damage due to starvation.
	WE_Corrosion,		// Used when vehicle or spacecraft takes damage due to environmental corrosion.
	WE_Audible,			// Used when depicting the source of an audible message.
	WE_Invalid,
};

class AUGL_DLL WEAPONEFFECT_Hash
{
public:
	inline size_t operator()(const WEAPONEFFECT& o) const
	{
		return hash<unsigned int>()(o);
	}
};

// Names that correspond to each WEAPONEFFECT.
inline QString EffectName(WEAPONEFFECT effect)
{
	QString effectNames[] =
	{
		QObject::tr("No"),				// Abbreviations are for weapon damage in item names, e.g. rockets and missiles.
		QObject::tr("Acid"),			// Ac
		QObject::tr("Suffocation"),		// Su
		QObject::tr("Stun"),			// St
		QObject::tr("Poison"),			// Po
		QObject::tr("Radiation"),		// Ra
		QObject::tr("Disease"),			// Di
		QObject::tr("Electrical"),		// El
		QObject::tr("Bludgeon"),		// Bl
		QObject::tr("Freezing"),		// Fr
		QObject::tr("Crushing"),		// Cr
		QObject::tr("Sonic"),			// So
		QObject::tr("Burning"),			// Bu
		QObject::tr("Piercing"),		// Pi
		QObject::tr("Implant"),
		QObject::tr("Slashing"),		// Sl
		QObject::tr("Generic"),
		QObject::tr("Unused"),
		QObject::tr("Bleeding"),
		QObject::tr("Starvation"),
		QObject::tr("Corrosion"),

		// Insert new effect names above this line.
		// Add extra entries in case I add weapon effects on the server. This way the current clients won't crash as a result.
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
		QObject::tr("Unknown"),
	};
	return effectNames[effect];
}

// Returns true for weapon effects that are considered energy effects for armor absorption purposes.
inline bool IsEnergyEffect(WEAPONEFFECT effect)
{
	switch(effect)
	{
	default:
		return false;
	case WE_Cold:
	case WE_Electric:
	case WE_Heat:
	case WE_Radiation:
		return true;
	}
}

// Returns true for weapon effects that are considered kinetic effects for armor absorption purposes.
inline bool IsKineticEffect(WEAPONEFFECT effect)
{
	switch(effect)
	{
	default:
		return false;
	case WE_Acid:
	case WE_Bludgeon:
	case WE_Crush:
	case WE_Implant:
	case WE_Pierce:
	case WE_Slash:
	case WE_Sonic:
		return true;
	}
}

inline bool IsMetabolicEffect(WEAPONEFFECT effect)
{
	switch(effect)
	{
	default:
		return false;
	case WE_Suffocate:
	case WE_Stun:
	case WE_Poison:
	case WE_Disease:
	case WE_Bleeding:
	case WE_Starving:
	case WE_Corrosion:											// Metabolic for a ship or vehicle, in a sense. Similar to bleeding.
		return true;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFog
{
public:
	float								r;						// 0..d_FogColorMax
	float								g;						// 0..d_FogColorMax
	float								b;						// 0..d_FogColorMax
	float								density;				// 0..1. For exponential fog calculation. 0=no fog, 1=max fog.

public:
	AuFog(float red, float green, float blue, float fogDensity)
	{
		r = red;
		g = green;
		b = blue;
		density = fogDensity;

		// Colors are clamped to the range [0..d_FogColorMax]
		if(r < 0)
			r = 0;
		else if (r > d_FogColorMax)
			r = d_FogColorMax;
		if(g < 0)
			g = 0;
		else if (g > d_FogColorMax)
			g = d_FogColorMax;
		if(b < 0)
			b = 0;
		else if (b > d_FogColorMax)
			b = d_FogColorMax;

		// Density is clamped to the range [0..1]
		if(density < 0)
			density = 0;
		else if (density > 1)
			density = 1;
	}

	AuFog()
	{
		r = 0;
		g = 0;
		b = 0;
		density = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGLTextItem
{
public:
	int									m_Height;				// In pixels.
	AuElapsedTimer						m_LastUsed;				// The last time this text item was rendered.
	QString								m_TexName;				// The name used when creating the texture.
	GLuint								m_Texture;				// OpenGL texture id
	int									m_Width;				// In pixels.

public:
	AuGLTextItem()
	{
		m_Height = 0;
		m_Texture = 0;
		m_Width = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLRGB
{
private:
    unsigned char rgbRed;
    unsigned char rgbGreen;
  	unsigned char rgbBlue;

public:
	inline void SetColor(const QRgb &col)
	{
		rgbRed = qRed(col);
		rgbGreen = qGreen(col);
		rgbBlue = qBlue(col);
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLRGBA
{
private:
    unsigned char rgbRed;
    unsigned char rgbGreen;
  	unsigned char rgbBlue;
	unsigned char rgbAlpha;

public:
	inline void SetColor(const QRgb &col)
	{
		rgbRed = qRed(col);
		rgbGreen = qGreen(col);
		rgbBlue = qBlue(col);
		rgbAlpha = qAlpha(col);
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLRGBAf
{
private:
    float rgbRed;
    float rgbGreen;
  	float rgbBlue;
	float rgbAlpha;

public:
	inline void SetColor(float r, float g, float b, float a)
	{
		rgbRed = r;
		rgbGreen = g;
		rgbBlue = b;
		rgbAlpha = a;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLSun
{
public:
	float	m_Attenuation;
	float	m_ColorR;	// range 0..1
	float	m_ColorG;	// range 0..1
	float	m_ColorB;	// range 0..1
	float	m_ColorA;	// range 0..1
	float	m_DiskRadius;
	bool	m_Enabled;
	float	m_LocX;
	float	m_LocY;
	float	m_LocZ;
	AuPoint	m_XVec;
	AuPoint	m_YVec;
	float	m_ShadowAngle;	// angle of shadow squares rotation
	int		m_ShadowCount;	// number of shadow squares in the circle, 0 disables ringworld shadow effect

public:
	inline AuGLSun(float attenuation, float r, float g, float b, float a, float locX, float locY, float locZ, float diskRadius,
			const AuPoint& xvec, const AuPoint& yvec, float shadowAngle, int shadowCount)
	{
		m_Attenuation = attenuation;
		m_ColorR = r;
		m_ColorG = g;
		m_ColorB = b;
		m_ColorA = a;
		m_Enabled = true;
		m_LocX = locX;
		m_LocY = locY;
		m_LocZ = locZ;
		m_DiskRadius = diskRadius;
		m_XVec = xvec;
		m_YVec = yvec;
		m_ShadowAngle = shadowAngle;
		m_ShadowCount = shadowCount;
	}

	inline AuGLSun(float attenuation, const QColor& col, const AuPoint& loc, float diskRadius, const AuPoint& xvec, const AuPoint& yvec,
			float shadowAngle, int shadowCount)
	{
		m_Attenuation = attenuation;
		m_ColorR = col.redF();
		m_ColorG = col.greenF();
		m_ColorB = col.blueF();
		m_ColorA = col.alphaF();
		m_Enabled = true;
		m_LocX = (float)loc.x;
		m_LocY = (float)loc.y;
		m_LocZ = (float)loc.z;
		m_DiskRadius = diskRadius;
		m_XVec = xvec;
		m_YVec = yvec;
		m_ShadowAngle = shadowAngle;
		m_ShadowCount = shadowCount;
	}

	inline AuGLSun(float attenuation, const QColor& col, const AuPoint& loc, float diskRadius)
	{
		m_Attenuation = attenuation;
		m_ColorR = col.redF();
		m_ColorG = col.greenF();
		m_ColorB = col.blueF();
		m_ColorA = col.alphaF();
		m_Enabled = true;
		m_LocX = (float)loc.x;
		m_LocY = (float)loc.y;
		m_LocZ = (float)loc.z;
		m_DiskRadius = diskRadius;
		m_XVec = AuPoint(1,0,0);
		m_YVec = AuPoint(0,1,0);
		m_ShadowAngle = 0;
		m_ShadowCount = 0;
	}

	inline AuGLSun()
	{
		m_Attenuation = 0;
		m_ColorR = 0;
		m_ColorG = 0;
		m_ColorB = 0;
		m_ColorA = 0;
		m_Enabled = false;
		m_LocX = 0;
		m_LocY = 0;
		m_LocZ = 0;
		m_DiskRadius = 0;
		m_XVec = AuPoint(1,0,0);
		m_YVec = AuPoint(0,1,0);
		m_ShadowAngle = 0;
		m_ShadowCount = 0;
	}

public:
	// Calculates attenuation when distances have been scaled. See DaWorldRingSection::RenderHullStatue and other Statue rendering
	// code for examples of this.
	inline float Attenuation(float scale) const
	{
		float attenuation = sqrtf(1.f/m_Attenuation);
		attenuation /= scale;
		return 1.f/(attenuation*attenuation);
	}

	// 'atmDensity' is expected to be in the range of 0 to 1.
	QColor SunsetColor(const QColor& atmColor, float atmDensity) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuTextureIdHash
{
public:
	inline size_t operator()(const unsigned int& o) const
	{
		return hash<unsigned int>()(o);
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGL_DLL AuGLTexture
{
friend class AuGLEffects;

private:
	GLuint								m_GlName;	// OpenGL assigned name of this texture, 0 if not assigned
	QString								m_sId;		// A unique id for this texture when it's stored by AuGLEffects.
	unordered_map<unsigned int, unsigned int, AuTextureIdHash>	m_Users;	// Ids of users of this texture.

private:
	// Only used by AuGLEffects
	inline AuGLTexture(unsigned int uiUserId, const QString &sTextureId)
	{
		m_GlName = 0;
		m_sId = sTextureId;
		AddUser(uiUserId);
	}

public:
	inline AuGLTexture()
	{
		m_GlName = 0;
	}

	inline ~AuGLTexture()
	{
		// We don't seem to have any way to know when our window is going to be destroyed, in order to clean up our GL textures.
		// We must assume GL takes care of that.
		//if(m_GlName)
			//qDebug() << "AuGLTexture destructed with id" << m_GlName << m_sId << "users" << m_Users.count();

		// There is no guarantee that a GL context will be current when this object is destroyed.
		//Delete();
	}

	// Cache Management
	inline void AddUser(unsigned int uid)
	{
		m_Users.insert(pair<unsigned int, unsigned int>(uid, 1));
	}

	inline bool FindUser(unsigned int uid) const
	{
		return m_Users.find(uid) != m_Users.end();
	}

	// Returns TRUE if the texture is no longer in use after removing the user.
	inline bool RemoveUser(unsigned int uid)
	{
		unordered_map<unsigned int, unsigned int, AuTextureIdHash>::iterator ui = m_Users.find(uid);
		if(ui != m_Users.end())
			m_Users.erase(ui);
		return m_Users.empty();
	}

	inline unsigned int UserCount() const
	{
		return (unsigned int)m_Users.size();
	}

	inline const QString &Id() const
	{
		return m_sId;
	}

	// Queries
	inline GLuint GlName() const
	{
		return m_GlName;
	}
#if !defined(QT_NO_DEBUG)
	inline static void OutputGlError(const QString& label)
	{
		GLenum errorno = glGetError();
		if(errorno != GL_NO_ERROR)
			qWarning() << label << "AuGLTexture OpenGL error:" << errorno;
	}
#else
	inline static void OutputGlError(const QString&)
	{}
#endif

	void ReplaceSubImage(AuOpenGLFunctions& GLf, QImage &bm, unsigned int xpos, unsigned int ypos);

	// Creates this texture from a QImage.
	static bool CreateFromBitmap(AuOpenGLFunctions& GLf, const QImage &bm, bool mipMap);

private:
	// expects a compact array of floats in sets of three
	static bool CreateFromFloats(AuOpenGLFunctions& GLf, const float* data, unsigned int width, unsigned int height,
			bool mipMap, GLenum format);

	// Creates this texture from a buffer containing RGB values.
	// See glReadPixels and glTexImage2D for valid format values.
	//
	// The GL device must be current when calling this function.
	static bool CreateFromRGB(AuOpenGLFunctions& GLf, const unsigned char *rgbdata, int xsize, int ysize, GLenum format,
			GLenum dataType, bool mipMap);

	// Only used by AuGLEffects. Names and binds a texture before creating it
	// in the cache.
	void NameAndBind(AuOpenGLFunctions& GLf, bool mipMap, GLint magFilter=GL_LINEAR, GLint minFilter=GL_LINEAR);

	// Delete this texture if it has been created.
	void Delete(AuOpenGLFunctions& GLf);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Converts a screen pixel location (0,0 upper left. +X right. +Y down to screen size in pixels) to an
// OpenGL viewport location (0,0 center. -1,-1 to 1,1, +X right, +Y up).
inline QPointF PixelToViewport(const QWidget* w, const QPoint& pixel)
{
	float xpos = ((float)pixel.x() / (float)w->width() - .5f) * 2.f;		// -1 left to 1 right.
	float ypos = (.5f - (float)pixel.y() / (float)w->height()) * 2.f;		// -1 bottom to 1 top
	return QPointF(xpos, ypos);
}

// Performs the reverse of PixelToViewport.
inline QPoint ViewportToPixel(const QWidget* w, const QPointF& viewPoint)
{
	int x = w->width()/2 + (viewPoint.x() * w->width()/2);
	int y = w->height()/2 - (viewPoint.y() * w->height()/2);
	return QPoint(x, y);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuGLEffects - Manages AuGLTexture objects and GLSL shaders.

class AUGL_DLL AuGLEffects : public AuOpenGLFunctions
{
public:
private:
	GLuint								m_HUDItemsTexture;		// This one gets requested a lot.
	QVector<AuGLTexture*>				m_Textures;
	QMutex								m_muTextures;

	// GLSL
	AuGLVAO								m_BarVAO;				// For rendering horizontal status bars: health, damage, etc.
	AuGLVAO								m_CrosshairBar;			// For rendering crosshair.
	AuGLVAO								m_CrosshairDot;			// For rendering crosshair.
	AuGLSL								m_GLSL;					// Contains all GLSL shaders.
	AuGLVAO								m_HeadingVAO;			// For rendering heading tape on HUD.
	AuGLVAO								m_HUDBaseVAO;			// For rendering HUD.
	AuGLVAO								m_HUDEnemyVAO;			// For rendering HUD.
	AuGLVAO								m_HUDGunVAO;			// For rendering APC HUD.
	AuGLVAO								m_HUDHoldVAO;			// For rendering HUD.
	AuGLVAO								m_HUDLadderVAO;			// For rendering HUD.
	AuGLVAO								m_HUDStressVAO;			// For rendering HUD.
	AuGLVAO								m_HUDThrottleVAO;		// For rendering HUD.
	AuGLVAO								m_ItemBarVAO;			// For rendering item condition.
	AuGLVAO								m_JumpVAO;				// For rendering jump spring.
	AuGLVAO								m_LocationDot;			// For rendering location.
	AuGLVAO								m_LocationCircle;		// For rendering location.
	AuGLVAO								m_MoveVAO;				// For rendering move power bar.
	AuGLVAO								m_NumberVAO;			// For rendering numbers.
	AuGLVAO								m_OffsetVAO;			// For rendering weapon effects and +/- sign on numbers.
	AuGLVAO								m_NWVAO;				// For rendering longitude and latitude numbers.
	AuGLVAO								m_SailVAO;				// For rendering sail.
	AuGLVAO								m_SEVAO;				// For rendering longitude and latitude numbers.
	AuGLVAO								m_ShieldVAO;			// For rendering shield bubbles.
	AuGLVAO								m_SpeedoGaugeVAO;		// For rendering speedometer.
	AuGLVAO								m_SpeedoNeedleVAO;		// For rendering speedometer.
	AuGLVAO								m_SpriteVAO;			// For rendering sprites.
	AuGLVAO								m_OrientHeadVAO;
	AuGLVAO								m_OrientPointerVAO;

	// Ambient light stack
	QColor			m_AmbientLight;		// Current ambient light setting.

	// Fog stack
	AuFog			m_Fog;				// Current fog setting.

	// Sun stack
	AuGLSun			m_Suns[MAX_SUNS];		// Current suns.
	AuGLLight		m_Lights[MAX_POINT_LIGHTS];	// Current Lights

	// Eye position
	AuPoint			m_EyePos;		// Point where we are rendering the scene from

	QMap<QFont, QMap<QString, AuGLTextItem> >	m_StringTextures;

public:
	AuGLEffects();

	~AuGLEffects();

	// Returns 'col1' blended with 'col2' by 'blend' ammount. 'blend' is in the range of 0 to 1.
	// 'blend' <= 0 returns col1
	// 'blend' >= 1 returns col2
	static QColor Blend(const QColor& col1, const QColor& col2, float blend);

	// Call this periodically to delete render text items that have expired.
	void DecayRenderText();

	GLuint HUDItemsTexture();

	// Returns the aspect ratio for rendering the scene, essentially max(WindowAspect, .25).
	// Our typical perspective projection has a 90 degree field of view.
	// We account for aspect ratio so the field of view remains constant vertically; horizontal field of view varies with
	// window aspect ratio. We don't allow less than .25 aspect because the resulting horizontal field of view is greater
	// than 180 degrees, which makes things clip too much in tests for being on the screen, based on Z >= 0. Instead, we will
	// stretch the scene using aspect ratio, rather than allow a field of view greater than 180.
	static double SceneAspectRatio(const QRect& windowRect);

	// Returns the aspect ratio of 'windowRect'.
	// Returns 1 if 'windowRect' has zero width or height.
	inline static double WindowAspectRatio(const QRect& windowRect)
	{
		int w = windowRect.width();
		if(w <= 0)
			return 1;
		int h = windowRect.height();
		if(h <= 0)
			return 1;
		return (double)h / (double)w;
	}

	// 'desiredFoVY' is the Y field of view angle wanted, in radians.
	// Returns a field of view Y angle for the perspective transformation that maintains a 1:1 image aspect ratio, regardless of
	// the aspect ratio of the window.
	// Return angle may be less than 'desiredFoVY' to prevent the X field of view from exceeding 180 degrees, based on 'aspect'.
	static double WindowFieldOfViewY(double aspect, double desiredFoVY);

	inline static double WindowFieldOfViewY(const QRect& windowRect, double desiredFoVY)
	{
		return WindowFieldOfViewY(WindowAspectRatio(windowRect), desiredFoVY);
	}

	inline const AuPoint& EyeLocation() const
	{
		return m_EyePos;
	}

	// Get/create a render text item.
	// OpenGL context must be current.
	void GetRenderTextItem(AuGLTextItem& textItem, const QString& str, const QFont& font, int flags, int wrapWidth=1);

	// Fills 'glsupport' with names of OpenGL versions that are supported by the current driver.
	static void OpenGLVersions(QStringList& glsupport);

	// Used by aircraft and spacecraft helm.
	// 'alt', 'speed', 'vspeed' and 'lspeed' are displayed as number without units. They are unitless.
	// 'pitch', 'roll', 'heading', 'longitude' and 'latitude' are in radians.
	// This function sets up it's own shaders.
	void RenderAircraftHud(const QRect& clientRect, float xpos, float ypos, float alt, float pitch, float roll, float heading,
			float speed, float vspeed, float lspeed, float longitude, float latitude, int hudIntensity, float hullStress,
			int throttle, int etesecs, bool inEnemyRange, bool isHolding);

	// Used by APC.
	// 'longitude' and 'latitude' are in radians.
	// This function sets up it's own shaders.
	void RenderAPCHud(const QRect& clientRect, float xpos, float ypos, float heading, float longitude, float latitude,
			int hudIntensity, bool inEnemyRange);

	// Renders a horizontal status bar from the HudItems.png texture.
	// 'value' is clamped to the range 0 to 1.
	// 'texoffx' and 'texoffy' identify the texture offset to the bar in HudItems.png, 0-based, in pixels.
	// 'whichBar' 0=vehicle health/wrench, 1=fuel/gas pump, 2=body health/heart, 3=stamina/lungs, 4=hunger/stomach
	// This function sets up it's own shaders.
	void RenderBar(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value, int whichBar,
			float rleft, float gleft, float bleft, float rright, float gright, float bright, float alpha=1.f);

	// 'targetLevel' is clamped to the range 0 to 1.
	// This function sets up it's own shaders.
	void RenderCrosshair(const QRect& clientRect, float xpos, float ypos, float targetLevel,
			float r, float g, float b, float a=1.f);

	// 'value' is clamped to the range 0 to 1.
	// This function sets up it's own shaders.
	void RenderItemConditionBar(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
			float rbottom, float gbottom, float bbottom, float rtop, float gtop, float btop, float alpha);

	// This function sets up it's own shaders.
	void RenderItemEffect(const QRect& clientRect, ITEMEFFECT effect, float left, float bottom, float size,
			float r, float g, float b, float a=1.f);

	// Expects TexOffset shader to be selected and configured somewhat.
	void RenderItemEffect(const AuMatrix4x4& projection, ITEMEFFECT effect, float left, float bottom, float size);

	// 'value' is clamped to the range 0 to 1.
	// This function sets up it's own shaders.
	void RenderJumpSpring(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
			float r, float g, float b, float a);

	// Renders pulsating circle at screen position.
	// This function sets up it's own shaders.
	void RenderLocation(const QRect& clientRect, float xpos, float ypos, float size, float r, float g, float b, float a=1.f);

	// 'value' is clamped to the range 0 to 1.
	// This function sets up it's own shaders.
	void RenderMovePower(const QRect& clientRect, float left, float bottom, float barwid, float barht, float value,
			float r, float g, float b, float a);

	// 'justify' 0=left, 1=center, 2=right
	// 'sign' is 0=minus when negative, 1=plus and minus always, 2=NS for latitude, 3=EW for longitude
	// This function sets up it's own shaders.
	float RenderNumber(const QRect& clientRect, int value, float left, float base, float ht, int justify, int sign,
			float r, float g, float b, float a=1.f);

	// This version of RenderNumber expects the TexOffset shader to be selected and configured before the call.
	// 'justify' 0=left, 1=center, 2=right
	// 'sign' is 0=minus when negative, 1=plus and minus always, 2=NS for latitude, 3=EW for longitude
	float RenderNumber2(const AuMatrix4x4& projection, int value, float left, float base, float ht, int justify, int sign);

	void RenderOrientationIndicator(const QRect& clientRect, float xpos, float ypos, float zRot, float size,
			float r, float g, float b, float a=1.f);

	// 'sizeWid' is based on sail setting/throttle, can be negative for left facing sail.
	// This function sets up it's own shaders.
	void RenderSail(const QRect& clientRect, float xpos, float ypos, float sizeWid, float r, float g, float b, float a=1.f);

	// 'xxScram' bools indicate that a side is scrambled.
	void RenderShieldBubble(const AuMatrix4x4& projection, const AuMatrix4x4& model, float sizeX, float sizeY, float sizeZ,
			float front, bool frontScram, float bottom, bool bottomScram, float left, bool leftScram, float back, bool backScram,
			float top, bool topScram, float right, bool rightScram, float texOffset, float divisor,
			float hitPosArray[MAX_SHIELD_HIT_EFFECTS*4], float hitDurArray[MAX_SHIELD_HIT_EFFECTS], bool hasShieldHits);

	// 'speedMph' is clamped into the range 5mph to 85mph, which is the limits of the gauge.
	// This function sets up it's own shaders.
	void RenderSpeedometer(const QRect& clientRect, float xpos, float ypos, float size, float speedMph, float r, float g,
			float b, float a);

	// This function sets up it's own shaders.
	void RenderSprite(const QRect& clientRect, float xpos, float ypos, float sizeX, float sizeY, GLuint texture,
			float r, float g, float b, float a=1.f);

	// 'system' matches the enum SYSTEM.
	// This function sets up it's own shaders.
	inline void RenderSystemIcon(const QRect& clientRect, int system, float left, float bottom, float size,
			float r, float g, float b, float a=1.f)
	{
		// NOTE: SYS_FTLDrive corresponds to IE_System_FTLWormholeDrive
		//       SYS_Count corresponds to IE_System_Door
		//       SYS_Count+1 corresponds to IE_System_FTLWarpDrive.
		//       SYS_Count+2 corresponds to IE_System_Berth.
		ITEMEFFECT effect = (ITEMEFFECT)(IE_System_CommandAndControl + system);
		Q_ASSERT(effect >= IE_System_CommandAndControl && effect <= IE_System_Berth);
		RenderItemEffect(clientRect, effect, left, bottom, size, r,g,b,a);
	}

	// Render text at a pixel position on the screen using OpenGL.
	// 'x' and 'y' are pixel positions on the screen, with 0,0 at upper left of screen.
	// 'str' is the string to display.
	// 'font' is the font to use.
	// 'viewX' and 'viewY' are the pixel dimensions of the viewport.
	// 'flags' are those documented in QPainter::drawText. 'wrapWidth' in pixels is used when Qt::TextWordWrap is specified; it may
	//   also affect Qt::AlignJustify.
	// NOTE: glDepthMask and GL_DEPTH_TEST are disabled while rendering the text.
	// NOTE: With no blending enabled, the text appears on a solid black background rectangle.
	// NOTE: Qt::TextWordWrap and Qt::AlignLeft together either conflict or invoke logic I don't understand because sometimes the
	//   lines don't wordwrap. Qt::TextWordWrap alone aligns to the left by default and seems to work properly.
	// Returns the pixel height of the text item.
	int RenderText(int x, int y, const QString& str, const QFont& font, const QColor& color,
			int viewX, int viewY, int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, int wrapWidth=1);

	// Render text at a model space position on the screen using OpenGL.
	// 'loc' is a model space position on the screen.
	// 'str' is the string to display.
	// 'font' is the font to use.
	// 'viewY' is the pixel height of the viewport.
	// 'flags' are those documented in QPainter::drawText. 'wrapWidth' in pixels is used when Qt::TextWordWrap is specified; it may
	//   also affect Qt::AlignJustify.
	// NOTE: With no blending enabled, the text appears on a solid black background rectangle.
	// NOTE: Qt::TextWordWrap and Qt::AlignLeft together either conflict or invoke logic I don't understand because sometimes the
	//   lines don't wordwrap. Qt::TextWordWrap alone aligns to the left by default and seems to work properly.
	void RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, const AuPoint& loc, const QString& str,
			const QFont& font, const QColor& color, int viewY, int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine,
			int wrapWidth=1);

	// Render text into a quad 'll', 'ul', 'ur', 'lr'.
	void RenderText(const AuMatrix4x4& projection, const AuMatrix4x4& model, const AuPoint& ll, const AuPoint& ul,
			const AuPoint& ur, const AuPoint& lr, const QString& str, const QFont& font, const QColor& color);

	// Same as previous, except that the TextureShader is already selected and configured with projection matrix .
	// 'model' is the model matrix that places the text where it is wanted. It is not actually the matrix that is set in the shader.
	void RenderText_Shader(const AuMatrix4x4& projection, const AuMatrix4x4& model, const QString& str, const QFont& font,
			const QColor& color, int viewY, int flags=Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine, int wrapWidth=1);

	// Expects TexOffset shader to be selected and configured somewhat.
	inline void RenderWeaponEffect(const AuMatrix4x4& projection, WEAPONEFFECT effect, float left, float bottom, float size)
	{
		Q_ASSERT(effect != WE_None);
		RenderItemEffect(projection, (ITEMEFFECT)(effect - 1), left, bottom, size);
	}

	// This function sets up it's own shaders.
	void RenderWeaponEffect(const QRect& clientRect, WEAPONEFFECT effect, float left, float bottom, float size,
			float r, float g, float b, float a=1.f);

	// Expects TexOffset shader to be selected and configured somewhat.
	inline void RenderWeaponMode(const AuMatrix4x4& projection, WEAPONMODE mode, float left, float bottom, float size)
	{
		Q_ASSERT(mode > WM_Not && mode <= WM_Psionic);
		RenderItemEffect(projection, (ITEMEFFECT)(IE_WM_Area + mode - 1), left, bottom, size);
	}

	// This function sets up it's own shaders.
	void RenderWeaponMode(const QRect& clientRect, WEAPONMODE mode, float left, float bottom, float size,
			float r, float g, float b, float a=1.f);

	// Changes the user id of ALL texture objects currently in use by 'uiOldId'.
	// 'uiNewId' is the new id that is assigned in its place.
	inline void ChangeUserId(unsigned int uiOldId, unsigned int uiNewId)
	{
		m_muTextures.lock();
		for(QVector<AuGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); it++)
		{
			if((*it)->FindUser(uiOldId))
			{
				// The old user id is using the texture object.
				// Replace the old user id with the new user id.
				(*it)->RemoveUser(uiOldId);
				(*it)->AddUser(uiNewId);
			}
		}
		m_muTextures.unlock();
	}

	// 'clampEdges' 0=default, 1=GL_CLAMP_TO_EDGE, 2=GL_CLAMP to black
	GLuint CreateTextureFromBitmap(unsigned int uiUserId, const QString &sTextureId, const QImage &bm,
			GLint magFilter=GL_LINEAR, GLint minFilter=GL_LINEAR, bool mipMap=false, int clampEdges=0);

	GLuint CreateTextureFromBitmap(unsigned int uiUserId, const QString &sTextureId, const QString &sFileName,
			GLint magFilter=GL_LINEAR, GLint minFilter=GL_LINEAR, bool mipMap=false, int clampEdges=0);

	// Create a texture map from an array of floating point triplets.
	GLuint CreateTextureFromFloats(unsigned int uiUserId, const QString &sTextureId, const float* data, unsigned int width,
			unsigned int height, bool mipMap=false, int clampEdges=0, GLenum format=GL_RGB);

	void DeleteTexture(unsigned int uiUserId, const QString &sTextureId);

	// ALL textures used by 'uiUserId'.
	void DeleteTextures(unsigned int uiUserId);

	inline AuGLTexture *FindTexture(const QString &sTextureId)
	{
		m_muTextures.lock();
		for(QVector<AuGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); it++)
			if((*it)->Id() == sTextureId)
			{
				m_muTextures.unlock();
				return *it;
			}
		m_muTextures.unlock();
		return 0;
	}

	inline AuGLTexture *FindTexture(GLuint texId)
	{
		m_muTextures.lock();
		for(QVector<AuGLTexture*>::iterator it = m_Textures.begin(); it != m_Textures.end(); it++)
			if((*it)->GlName() == texId)
			{
				m_muTextures.unlock();
				return *it;
			}
		m_muTextures.unlock();
		return 0;
	}

	inline void ReplaceSubImage(const QString &sTextureId, QImage &bm,
			unsigned int xpos, unsigned int ypos)
	{
		// Find the existing texture object using the texture id.
		AuGLTexture *pTex = FindTexture(sTextureId);
		if(pTex)
			pTex->ReplaceSubImage(*this, bm, xpos, ypos);
	}

	// Claims an existing texture for use by 'uiUserId'.
	// The Gl name of the texture object is returned, if found.
	// Zero is returned if the texture does not exist.
	inline GLuint UseTexture(unsigned int uiUserId, const QString &sTextureId)
	{
		// Find the existing texture object using the texture id.
		AuGLTexture *pTex = FindTexture(sTextureId);
		if(!pTex)
			return 0;	// No texture object with that id exists.

		// Add the user to the texture object.
		pTex->AddUser(uiUserId);

		// Return the Gl name of the texture object.
		return pTex->GlName();
	}

	// Claims an existing texture for use by 'uiUserId', using the Gl name
	// of the texture object. Au3ds uses this.
	inline void UseTexture(unsigned int uiUserId, GLuint texId)
	{
		// Find the existing texture object using the texture id.
		AuGLTexture *pTex = FindTexture(texId);
		if(!pTex)
			return;	// No texture object with that id exists.

		// Add the user to the texture object.
		pTex->AddUser(uiUserId);
	}

	// Create a texture map from a bitmap.
	// The result is NOT stored in the cache.
	inline void CreateTextureFromBitmap(const QImage& bmTexture, bool mipMap)
	{
		AuGLTexture tex;
		tex.CreateFromBitmap(*this, bmTexture, mipMap);
	}

	// Create a texture map from an array of floating point triplets, or quads, depending on format.
	// The result is NOT stored in the cache.
	inline void CreateTextureFromFloats(const float* data, unsigned int width, unsigned int height, GLenum format=GL_RGB)
	{
		AuGLTexture tex;
		tex.CreateFromFloats(*this, data, width, height, false, format);
	}

	// Shaders
	inline void CurrentFog(float& r, float& g, float& b, float& density) const
	{
		r = m_Fog.r;
		g = m_Fog.g;
		b = m_Fog.b;
		density = m_Fog.density;
	}

	inline bool HasFog() const
	{
		return m_Fog.density > 0;
	}

	inline void CurrentAmbientLight(float& r, float& g, float& b, float& a) const
	{
		r = m_AmbientLight.redF();
		g = m_AmbientLight.greenF();
		b = m_AmbientLight.blueF();
		a = m_AmbientLight.alphaF();
	}

#if !defined(QT_NO_DEBUG)
	inline void DebugLights()
	{
		int suns = 0;
		for(int isun=0; isun<MAX_SUNS; isun++)
			if(m_Suns[isun].m_Enabled)
				suns++;

		qDebug("AuGLEffects::DebugLights %d suns enabled, ambient %d,%d,%d",
				suns,
				m_AmbientLight.red(),
				m_AmbientLight.green(),
				m_AmbientLight.blue());
	}
#endif

	inline void DeselectShaders()
	{
		GLSL().DeselectProgram();
	}

	inline AuGLSL& GLSL()
	{
		return m_GLSL;
	}

	// This gets configured during init and compiled into the shaders.
	inline unsigned int MaxLights() const
	{
		return m_GLSL.MaxLights();
	}

	// Disables all lights that were created by a particular 'atomId'.
	// Skips light from effects.
	inline void DisableAtomLights(const AuId& atomId)
	{
		for(unsigned int ilight=0; ilight<MaxLights(); ilight++)
			if(m_Lights[ilight].IsAtomLight(atomId))
				m_Lights[ilight].m_bEnabled = false;
		GLSL().InvalidateLights();
	}

	inline void DisableLights()				// all lights without changing settings
	{
		for(unsigned int ilight=0; ilight<MaxLights(); ilight++)
			m_Lights[ilight].m_bEnabled = false;
	}

	inline void DisableSuns()					// all suns without changing settings
	{
		for(int isun=0; isun<MAX_SUNS; isun++)
			m_Suns[isun].m_Enabled = false;
		GLSL().InvalidateLights();
	}

	inline void DisableSuns(QList<int>& sunsDisabled)	// all suns without changing settings. Fills list of those disabled.
	{
		for(int isun=0; isun<MAX_SUNS; isun++)
			if(m_Suns[isun].m_Enabled)
			{
				m_Suns[isun].m_Enabled = false;
				sunsDisabled.append(isun);
			}
		GLSL().InvalidateLights();
	}

	// Enables all lights that were created by a particular 'atomId'.
	// Skips light from effects.
	inline void EnableAtomLights(const AuId& atomId)
	{
		for(unsigned int ilight=0; ilight<MaxLights(); ilight++)
			if(m_Lights[ilight].IsAtomLight(atomId))
				m_Lights[ilight].m_bEnabled = true;
		GLSL().InvalidateLights();
	}

	inline void EnableSuns(const QList<int>& sunsToEnable)		// with current settings
	{
		for(QList<int>::const_iterator si = sunsToEnable.cbegin(); si != sunsToEnable.cend(); si++)
			m_Suns[*si].m_Enabled = true;
		GLSL().InvalidateSuns();
	}

	// Must be called while GL context is current.
	// 'maxLights' is the maximum number of point light sources.
	void InitializeGL(unsigned int maxLights);

	inline void SetAmbientLight(QColor& ambient)
	{
		float r = ambient.redF();
		float g = ambient.greenF();
		float b = ambient.blueF();
		float a = ambient.alphaF();
		SetAmbientLight(r,g,b,a);
	}

	// rgb values are [0..1]
	inline void SetFog(float r, float g, float b, float density)
	{
		m_Fog = AuFog(r,g,b, density);
	}

	// rgb values are [0..1]
	inline void SetAmbientLight(float r, float g, float b, float a)
	{
		// Colors need to be clamped to the range [0..1]
		if(r < 0)
			r = 0;
		else if (r > 1.0)
			r = 1.0;
		if(g < 0)
			g = 0;
		else if (g > 1.0)
			g = 1.0;
		if(b < 0)
			b = 0;
		else if (b > 1.0)
			b = 1.0;
		if(a < 0)
			a = 0;
		else if (a > 1.0)
			a = 1.0;
		//qDebug("AuGLEffects::SetAmbientLight rgba %g,%g,%g,%g", r,g,b,a);

		m_AmbientLight.setRgbF(r, g, b, a);
	}

	// also enables the light.
	inline void SetLight(int ilight, float kL, float kQ, const QColor& color, float locX, float locY, float locZ)
	{
		SetSpotLight(ilight, kL,kQ, color, locX,locY,locZ, -2.f, -2.f, 0,1,0);
	}

	void SetEyePos(const AuPoint& pEyePos);

	// enables the light
	inline void SetSpotLight(int ilight, float kL, float kQ, const QColor& color, float locX, float locY, float locZ,
			float cosOuterCone, float cosInnerCone,	double dirX, double dirY, double dirZ)
	{
		// Colors need to be expressed in the range [0..1]
		float r = color.redF();
		float g = color.greenF();
		float b = color.blueF();
		float a = color.alphaF();

		// AuGLLight constructor creates it with 'enabled' set true.
		m_Lights[ilight] = AuGLLight(kL,kQ, r,g,b,a, locX,locY,locZ, cosOuterCone, cosInnerCone, dirX, dirY, dirZ);
	}

	inline void SetLight(int ilight, const AuGLLight& light)
	{
		m_Lights[ilight] = light;
	}

	// enables the light
	inline void SetSun(int isun, float attenuation, const QColor& color, float locX, float locY, float locZ, float diskRadius,
			const AuPoint& xvec=AuPoint(1,0), const AuPoint& yvec=AuPoint(0,1), float shadowAngle=0, int shadowCount=0)
	{
		// Colors need to be expressed in the range [0..1]
		float r = color.redF();
		float g = color.greenF();
		float b = color.blueF();
		float a = color.alphaF();

		// AuGLSun constructor creates it with 'enabled' set true.
		m_Suns[isun] = AuGLSun(attenuation, r,g,b,a, locX,locY,locZ, diskRadius, xvec, yvec, shadowAngle, shadowCount);
	}

	inline void SetSun(int isun, const AuGLSun& sun)
	{
		m_Suns[isun] = sun;
	}

	inline void SetSunShadowAngle(int isun, float angle)
	{
		m_Suns[isun].m_ShadowAngle = angle;
		//qDebug("AuGLEffects::SetSun %d enabled", isun);
	}

	const AuGLLight& Light(int lightIndex) const
	{
		return m_Lights[lightIndex];
	}

	const AuGLSun& Sun(int sunIndex) const
	{
		return m_Suns[sunIndex];
	}

	/*
	// Turns off suns that are over the horizon from the 'vup' vector.
	// 'sunModulation' is multiplied by the current color of affected suns.
	inline void SunHorizonEffect(const AuPoint& vup, float sunModulation)
	{
		for(int isun=0; isun<MAX_SUNS; isun++)
		{
			if(m_Suns[isun].m_Enabled)
			{
				// Account for radius of solar disk.
				AuPoint vsun(m_Suns[isun].m_LocX, m_Suns[isun].m_LocY, m_Suns[isun].m_LocZ);
				float distSun = vsun.Length();
				if(distSun != 0 && distSun != 1.0)
					vsun /= distSun;
				float rDot = vsun.Dot(vup);
				float diskang = atan(m_Suns[isun].m_DiskRadius/distSun);
				float sunang = acos(rDot);
				float rd1 = cos(sunang + diskang);
				float rd2 = cos(sunang - diskang);
				if(rd1 > rDot)
					rDot = rd1;
				else if(rd2 > rDot)
					rDot = rd2;
				if(rDot < 0)
					ModulateSun(isun, sunModulation);
			}
		}
	}
	*/
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Texture Library
// Provides a simple library of our textures that are useful for designing spacecraft, buildings, etc.

class AUGL_DLL AuTextureLibItem
{
public:
	enum EFFECT_INDEX
	{
		EI_None,
		EI_Capacitor1,
		EI_Capacitor2,
		EI_Capacitor3,
		EI_Capacitor4,
		EI_Capacitor5,
		EI_Capacitor6,
		EI_FTLDrive1,
		EI_FTLDrive2,
		EI_FTLDrive3,
		EI_FTLDrive4,
		EI_FTLDrive5,
		EI_FTLDrive6,
		EI_FTLDriveLow1,
		EI_FTLDriveLow2,
		EI_FTLDriveLow3,
		EI_FTLDriveLow4,
		EI_FTLDriveLow5,
		EI_FTLDriveLow6,
		EI_HullWarp1,
		EI_HullWarp2,
		EI_HullWarp3,
		EI_HullWarp4,
		EI_HullWarp5,
		EI_HullWarp6,
		EI_HullWormPos1,
		EI_HullWormPos2,
		EI_HullWormPos3,
		EI_HullWormPos4,
		EI_HullWormPos5,
		EI_HullWormPos6,
		EI_LifeSupport1,
		EI_LifeSupport2,
		EI_LifeSupport3,
		EI_LifeSupport4,
		EI_LifeSupport5,
		EI_LifeSupport6,
		EI_ManeuverDrive1,
		EI_ManeuverDrive2,
		EI_ManeuverDrive3,
		EI_ManeuverDrive4,
		EI_ManeuverDrive5,
		EI_ManeuverDrive6,
		EI_ManeuverDriveLow1,
		EI_ManeuverDriveLow2,
		EI_ManeuverDriveLow3,
		EI_ManeuverDriveLow4,
		EI_ManeuverDriveLow5,
		EI_ManeuverDriveLow6,
		EI_PowerPlant1,
		EI_PowerPlant2,
		EI_PowerPlant3,
		EI_PowerPlant4,
		EI_PowerPlant5,
		EI_PowerPlant6,
		EI_Sensor1,
		EI_Sensor2,
		EI_Sensor3,
		EI_Sensor4,
		EI_Sensor5,
		EI_Sensor6,
		EI_Shield1,
		EI_Shield2,
		EI_Shield3,
		EI_Shield4,
		EI_Shield5,
		EI_Shield6,
		EI_Transporter1,
		EI_Transporter2,
		EI_Transporter3,
		EI_Transporter4,
		EI_Transporter5,
		EI_Transporter6,
		EI_TransporterLow1,
		EI_TransporterLow2,
		EI_TransporterLow3,
		EI_TransporterLow4,
		EI_TransporterLow5,
		EI_TransporterLow6,
		EI_Weapon1,
		EI_Weapon2,
		EI_Weapon3,
		EI_Weapon4,
		EI_Weapon5,
		EI_Weapon6,
		EI_HullWormNeg1,
		EI_HullWormNeg2,
		EI_HullWormNeg3,
		EI_HullWormNeg4,
		EI_HullWormNeg5,
		EI_HullWormNeg6,
		EI_HullWormNeu1,
		EI_HullWormNeu2,
		EI_HullWormNeu3,
		EI_HullWormNeu4,
		EI_HullWormNeu5,
		EI_HullWormNeu6,
		EI_Damage1,
		EI_Damage2,
		EI_Damage3,
		EI_Damage4,
		EI_Damage5,
		EI_Damage6,
		EI_Damage7,
		EI_Damage8,
		EI_Damage9,
		EI_Count
	};

private:
	quint16								m_Id;
	QString								m_Name;
	QString								m_File;

public:	// Constructors
	AuTextureLibItem(quint16 id, const QString& name, const QString& fileName)
	{
		m_Id = id;
		m_Name = name;
		m_File = fileName;
	}

	AuTextureLibItem(const AuTextureLibItem& c)
	{
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_File = c.m_File;
	}

public:	// Methods
	// Gets/loads an effect texture. 'frame' must be in the range 0 to 5.
	// OpenGL context must be current when calling this function.
	static GLuint EffectTexture(AuGLEffects& re, SURFACE_EFFECT effect, int frame);

	static GLuint EffectTexture(AuGLEffects& re, EFFECT_INDEX effIdx);

	static const QString& EffectTextureFile(EFFECT_INDEX libTexId);

	static const QString& EffectTextureFile(SURFACE_EFFECT effect, int frame);

	static AuTextureLibItem& EffectTextureItem(EFFECT_INDEX effIdx);

	// Effect textures are in groups for animation.
	// Returns a typical texture lib item suitable for a sample of the effect in an inanimate control.
	static AuTextureLibItem& EffectTextureItem(SURFACE_EFFECT effect);

	// 'frame' must be in the range 0 to 5.
	static AuTextureLibItem& EffectTextureItem(SURFACE_EFFECT effect, int frame);

	inline const QString& FileName() const
	{
		return m_File;
	}

	inline quint16 Id() const
	{
		return m_Id;
	}

	// Default palette for buildings.
	static void InitDefaultBdPalette(quint16 texturePalette[SCTEXI_Count]);

	// Default palette for spacecraft.
	static void InitDefaultScPalette(quint16 texturePalette[SCTEXI_Count]);

	// Gets/loads a texture.
	// OpenGL context must be current when calling this function.
	static GLuint LibraryTexture(AuGLEffects& re, quint16 libTexId);

	// Returns the number of textures available in the library.
	static quint16 LibraryTextureCount();

	static const QString& LibraryTextureFile(quint16 libTexId);

	static AuTextureLibItem& LibraryTextureItem(quint16 libTexId);

	// Human readable name for user interfaces.
	static const QString& LibraryTextureName(quint16 libTexId);

	// Returns the library texture id for a adamantine hull.
	static quint16 LibraryTextureHullAdamantine();

	// Returns the library texture id for a lumber building.
	static quint16 LibraryTextureHullLumber();

	// Returns the library texture id for a magmium hull.
	static quint16 LibraryTextureHullMagmium();

	// Returns the library texture id for a metal hull.
	static quint16 LibraryTextureHullMetal();

	// Returns the library texture id for a stone building.
	static quint16 LibraryTextureHullStone();

	// Returns the library texture id for a vulcium hull.
	static quint16 LibraryTextureHullVulcium();

	// Returns the library texture id for a wood building.
	static quint16 LibraryTextureHullWood();

	// Returns the library texture id for the background of the person's body stats.
	static quint16 LibraryTextureStatsBody();

	// Returns the library texture id for the background of the person's vehicle stats.
	static quint16 LibraryTextureStatsVehicle();

	inline const QString& Name() const
	{
		return m_Name;
	}
};

#endif // AuGLEffects_H
