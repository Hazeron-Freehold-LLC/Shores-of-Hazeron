//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auNebula.h,v 1.25 2022/02/19 17:32:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUNEBULA_H
#define AUNEBULA_H

#include "auatom.h"
#include <aupoint.h>
#include <augleffects.h>
#include <audice.h>
#include <augl.h>
#include <QVector>

class AuAtomRenderInfo;

struct galParticle
{
	AuPoint pos;		// Position of Particle
};

class AUATOM_DLL auNebula  
{
	// Constructors / Destructor
public:
	auNebula();
	auNebula(int iParticles, int iStars, float fA, float fB, float fC, float fD,
		float fParticleSize, int iColor, bool m_bReverseColor, AuPoint center, int seed, int iType,
		const QColor& galaxyColor);
	virtual ~auNebula();
protected:
	auNebula(const auNebula &rhs);		// Copy constructor.

	struct nebParticle
	{
		AuPoint pos;		// Position of Particle
		float r, g, b;	// Color of this particle
		float speed;	// Speed of Particle
		float angle;	// Angle formed between this particle and its previous and next particle
	};

	// Variables
public:
	double								m_CoreRadPc;	// Radius of the galaxy core in pc.
	double								m_GalaxyRadPc;	// Radius of the galaxy in pc.
	AuGLVAO								m_SkyBoxVAO;
	GLuint								m_img;
	QVector<nebParticle>				m_vNebula;
	QVector<galParticle>				m_vStars;		// These stars depict the stars in our Galaxy and the blue aura clouds.
	// These stars depict the rest of the stars in the Universe we can faintly see 
	QVector<AuPoint>					m_vUniStars;
	int		m_iParticles;		// Number of particles to generate nebula
	int		m_iStars;			// Number of Stars to generate with nebula
	float	m_fParticleSize;	// The particle size
	int		m_iColor;			// The color scheme of the nebula
	bool	m_bReverseColor;	// True if reversing the color scheme for the nebula
	// Variables that modify the generation algorithm of the nebula
	float	m_fA;
	float	m_fB;
	float	m_fC;
	float	m_fD;
	int		m_Seed;
	// Variables that are calculated
	float	m_fMaxAngle;		// Used in the color of the nebula
	float	m_fMaxSpeed;		// Used in the color of the nebula
	AuPoint	m_Dimensions;		// Max dimensions of nebula
	AuPoint	m_Center;			// Nebula center, these are sector coordinates
	AuDice	m_Dice;				// The dice
	int		m_Type;				// 0 - 6	0 - More Swirly, 6 - More Cloudy
	float	m_fTotalMax;		// Determined by m_Type
	QColor								m_GalaxyColor;			// Color of the galactic cloud.

	// Variables used to paint the nebula cloud effect associated with my nebula, if any.
	QColor								m_CloudColor;			// Dominant color of nebula. QColor(0,0,0) if none.
	float								m_CloudDensity;			// Density of smoke cloud.
	int									m_CloudTexture;			// Texture index to use for cloud effect, not a GL texture id.

	// Functions
public:
	void CreateNebula();
	void CalculateColor();

	void DeleteDeviceObjects(AuOpenGLFunctions& GLf);

	// Returns the color of the nebula cloud.
	inline QColor CloudColor() const
	{
		return m_CloudColor;
	}

	// Returns the density of the nebula cloud, in the range 0 to 1.
	inline float CloudDensity() const
	{
		return m_CloudDensity;
	}

	inline int CloudTexture() const
	{
		return m_CloudTexture;
	}

	// 'paneSize' should be a power of 2, e.g. 1024
	// 'mipMap' should be false, based on what I have observed.
	void CreateSkyBox(AuAtomRenderInfo& ari, const AuPoint& pos, unsigned int paneSize, bool mipMap);
	void RenderSkyBox(AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model, float opacity);
	void ChooseOrder();
	void Swap(float num1, float num2);
	AuPoint GetRandomVector();

	inline void SetCloudDensity(float d)
	{
		m_CloudDensity = d;
	}

	inline void SetCloudTexture(int t)
	{
		m_CloudTexture = t;
	}

private:
	void RenderNebula(double distance, AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model, GLuint texture);
	void RenderStars(float starDotSize, AuGLEffects& re, const AuMatrix4x4& projection, const AuMatrix4x4& model, GLuint texture,
			const AuPoint& pos);
};

#endif	// AUNEBULA_H

