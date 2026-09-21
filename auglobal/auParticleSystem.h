//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auParticleSystem.h,v 1.25 2022/04/15 16:34:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// This class is heavily derived from Kevin Harris's work on ogl_optimized_billboards.
// The original work has been modified to allow compliance with Linux machines.
// This will be version 1 of our particle system engine.  Further research into
// frame buffer objects and the ping-pong method should dramatically improve
// our particle system as this technique updates on the GPU rather than
// our current implementation that is all on the CPU.  This would allow us
// to update and render more particles on the screen
#ifndef AuParticleSystem_H
#define AuParticleSystem_H

#include "auelapsedtimer.h"
#include "augloballib.h"
#include "aupoint.h"

class AuGLEffects;

#define c_UnitScale						1						// Introduced by GW. Here to save me from editing a bunch of code.

// Particle object unique parameters
struct auParticle
{
	AuPoint		m_vCurPos;		// Current position of particle
	AuPoint		m_vCurVel;		// Current velocity of particle
	AuPoint		m_vNorm;		// Normal of vertex point
	AuPoint		m_vOldPos;
	float		m_fInitTime;	// Time of creation of particle
	float		m_InitDist;		// Viewer distance squared when position initialized.
	bool		m_Switch;		// T/F to switch texture coordinate direction
	int			m_Order;		// texture coordinate order for spanning a texture across multiple quads

	auParticle	*m_pNext;		// Next particle in list
};

// ParticleSystem class
class AUGLOBAL_DLL AuParticleSystem
{

public:

	// Constructor / Destructor
	AuParticleSystem();
	virtual ~AuParticleSystem();

	// Free the last particle.
	void FreeLast();

	// Frees the last 'howMany' particles.
	void FreeParticles(int howMany);

	// Inline functions
	void SetMaxParticles(unsigned long lMaxParticles)	// Sets maximum number of particles allowed on the system
		{ m_lMaxParticles = lMaxParticles; }
	unsigned long GetMaxParticles()						// Gets maximum number of particles allowed on the system
		{ return m_lMaxParticles; }

	void SetNumToRelease(unsigned long lNumToRelease)	// Sets number of particles to release
		{ m_lNumToRelease = lNumToRelease; }
	unsigned long GetNumToRelease()						// Gets number of particles to release
		{ return m_lNumToRelease; }

	void SetReleaseInterval(float fReleaseInterval)	// Sets release interval
		{ m_fReleaseInterval = fReleaseInterval; }
	float GetReleaseInterval()						// Gets release interval
		{ return m_fReleaseInterval; }

	void SetLifeCycle(float fLifeCycle)			// Sets life cycle of nu particles
		{ m_fLifeCycle = fLifeCycle; }
	float GetLifeCycle()						// Gets life cycle of nu particles
		{ return m_fLifeCycle; }

	void SetEndReleaseTime(float fTime)
		{ m_fEndReleaseTime = fTime; }

	void SetSize(float fSize)					// Sets point size to use for sprites
		{ m_fSize = fSize; }
	float GetSize()								// Gets current point size for sprites
		{ return m_fSize; }
	float GetMaxPointSize()						// Gets max point size for sprites
		{ return m_fMaxPointSize; }

	void SetColor(const QColor& clrColor)				// Sets color
		{ m_clrColor = clrColor; }
	const QColor& GetColor()							// Gets color
		{ return m_clrColor; }

	void SetPosition(const AuPoint& vPosition)			// Sets position of particle system
		{ m_vPosition = vPosition; }
	const AuPoint& GetPosition()						// Gets position of particle system
		{ return m_vPosition; }

	void SetVelocity(const AuPoint& vVelocity)			// Sets velocity vector
		{ m_vVelocity = vVelocity; }
	const AuPoint& GetVelocity()						// Gets velocity vector
		{ return m_vVelocity; }

	void SetVelocityVar(float fVelocityVar)	// Sets velocity variant to apply to particles
		{ m_fVelocityVar = fVelocityVar; }
	float GetVelocityVar()					// Gets velocity variant to apply to particles
		{ return m_fVelocityVar; }

	void SetPositionVar(float fPositionVar)	// Sets the position variant to apply to particles
		{ m_fPositionVar = fPositionVar; }
	float GetPositionVar()					// Gets the position variant to apply to particles
		{ return m_fPositionVar; }

	int GetParticleCount()					// Gets current active particle count on the system
		{ return m_lActiveCount; }

	float GetElapsedTime()
		{ return m_fElapsedTime; }
	float GetCurTime()
		{ return m_fCurrentTime; }

	void SetTextureID(GLuint textureId)		// Sets texture id used to render the particles
		{ m_textureID = textureId; }
	void SetTextureID2(GLuint textureId)	// Sets texture id used to render the particles
		{ m_textureID2 = textureId; }
	GLuint GetTextureID()
		{ return m_textureID; }
	GLuint GetTextureID2()
		{ return m_textureID2; }

	auParticle* GetActiveList()
		{ return m_pActiveList; }
		
	int Init(AuGLEffects& re);								// Initialize particle system
	static AuPoint GetRandomVector();		// Creates a random unit vector
	static AuPoint GetRandomVector2D();		// Creates a random unit vector
	void RestartParticleSystem();			// Will reset all particles to inactive

	// Virtual overrides
	virtual bool HasParticleExpired(const auParticle* pParticle);
	virtual bool HasPassedReleaseTime();
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);

	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

	// Particles fade out with alpha as they reach their radius.
	void RenderCloud(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

	virtual int Update();							// Updates particles and creates nu ones.

public:
	// Public variables
	QColor			m_clrColor;			// Color

private:
	auParticle		*m_pActiveList;		// Active particle list
    auParticle		*m_pFreeList;		// Inactive particle list
	unsigned long	m_lActiveCount;		// Number of active particles
	unsigned long	m_lMaxParticles;	// Max number of active particles allowed
	unsigned long	m_lNumToRelease;	// Max number of inactive particles to become active in one pass
	AuPoint			m_vPosition;		// Position particle system which is the emit location of each particle
	AuPoint			m_vVelocity;		// Velocity vector in units(feet) per second.
    float			m_fCurrentTime;		// Current time for this pass, in seconds
	float			m_fElapsedTime;		// Elapsed time between updates, in seconds
    float			m_fLastUpdate;		// Time of last update pass, in seconds
	float			m_fInitTime;		// Time particle system started, in seconds
	float			m_fMaxPointSize;	// Max point size allowed for sprites
	float			m_fReleaseInterval;	// Time between release of particles is allowed, in seconds
	float			m_fEndReleaseTime;	// Time after start to stop emitting particles, in seconds
	float			m_fLifeCycle;		// Life cycle of particle
	float			m_fSize;			// Size
	float			m_fVelocityVar;		// Velocity variant to apply
	float			m_fPositionVar;		// Position offset to apply
    GLuint			m_textureID;		// Texture id to bind particles to
	GLuint			m_textureID2;		// Second Texture
	AuElapsedTimer	m_Timer;			// Timer used to keep track of elapsed time between updates
};

#endif	// AuParticleSystem_H
