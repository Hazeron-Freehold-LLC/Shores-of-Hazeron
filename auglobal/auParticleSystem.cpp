//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auParticleSystem.cpp,v 1.45 2022/02/19 18:33:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auParticleSystem.h"
#include "auglobal.h"
#include "augleffects.h"

AuParticleSystem::AuParticleSystem()
{
	m_pActiveList		= NULL; // Head node of particles that are active
	m_pFreeList			= NULL; // Head node of particles that are inactive and waiting to be recycled.
	m_lActiveCount		= 0;
	m_fCurrentTime		= 0.0f;
	m_fLastUpdate		= 0.0f;
	m_fInitTime			= 0.0f;
	m_textureID			= 0;
	m_lMaxParticles		= 1;
	m_lNumToRelease		= 1;
	m_fReleaseInterval	= 1.0f;
	m_fLifeCycle		= 1.0f;
	m_fEndReleaseTime	= 0.0f;
	m_fSize				= 1.0f;
	m_clrColor			= QColor(255,255,255);
	m_vPosition			= AuPoint(0.0f,0.0f,0.0f);
	m_vVelocity			= AuPoint(0.0f,0.0f,0.0f);
	m_fVelocityVar		= 1.0f;
	m_fPositionVar		= 1.0f;

    m_fMaxPointSize = 0;	// Max point size allowed for sprites
}

AuParticleSystem::~AuParticleSystem()
{
	// NOTE: Particle system not responsible for cleanup of textures it was binding to.  That is
	// the job of the caller who created the particle system to do.
	while(m_pActiveList)
	{
		auParticle* pParticle = m_pActiveList;
		m_pActiveList = pParticle->m_pNext;
		delete pParticle;
	}
	m_pActiveList = NULL;

	while(m_pFreeList)
	{
		auParticle *pParticle = m_pFreeList;
		m_pFreeList = pParticle->m_pNext;
		delete pParticle;
	}
	m_pFreeList = NULL;
}

int AuParticleSystem::Init(AuGLEffects& re)
{
	// Get max point size
	float range[2];
	re.glGetFloatv(GL_POINT_SIZE_RANGE, range);
	m_fMaxPointSize = range[1];

	//if(m_fMaxPointSize > 0)
		//re.glPointSize(m_fMaxPointSize);

	return 1;
}

void AuParticleSystem::MoveParticle(auParticle* /*pParticle*/)
{

}

void AuParticleSystem::InitParticle(auParticle* /*pParticle*/)
{

}

bool AuParticleSystem::HasPassedReleaseTime()
{
	if(m_fEndReleaseTime == 0)
		return false;	// Particle system never stops emitting
	else if(m_fCurrentTime-m_fInitTime <= m_fEndReleaseTime)
		return false;	// Release time not yet reached keep emitting

	return true;		// Particle system should stop emitting
}

bool AuParticleSystem::HasParticleExpired(const auParticle* pParticle)
{
	if((m_fCurrentTime - pParticle->m_fInitTime) >= m_fLifeCycle)
		return true;
	return false;
}

// Generate a random unit vector.
AuPoint AuParticleSystem::GetRandomVector()
{
	AuPoint vVector;
	vVector.x = g_Au.Dice().Roll(1,2001,-1001);	// Generate a random number from -1000 to 1000.
	vVector.y = g_Au.Dice().Roll(1,2001,-1001);	// Generate a random number from -1000 to 1000.
	vVector.z = g_Au.Dice().Roll(1,2001,-1001);	// Generate a random number from -1000 to 1000.
	vVector.Unit();
	return vVector;
}

// Generate a random unit vector.
AuPoint AuParticleSystem::GetRandomVector2D()
{
	AuPoint vVector;
	vVector.x = g_Au.Dice().Roll(1,2001,-1001);	// Generate a random number from -1000 to 1000.
	vVector.y = g_Au.Dice().Roll(1,2001,-1001);	// Generate a random number from -1000 to 1000.
	vVector.Unit_2D();
	return vVector;
}

void AuParticleSystem::RestartParticleSystem()
{
	auParticle	*pParticle;
	auParticle	**ppParticle;

	ppParticle = &m_pActiveList; // Start at the head of the active list

	while(*ppParticle)
	{
		pParticle = *ppParticle; // Set a pointer to the head

		// Put the particle back on the free list...
		*ppParticle = pParticle->m_pNext;
		pParticle->m_pNext = m_pFreeList;
		m_pFreeList = pParticle;
		--m_lActiveCount;
	}
}

int AuParticleSystem::Update()
{
	int result = 1;
	m_fElapsedTime = 0;
	if(m_Timer.isValid())
		m_fElapsedTime = (float)m_Timer.elapsed()*(.001f);
	m_Timer.restart();	// Restart timer

	auParticle	*pParticle;
	auParticle	**ppParticle;

	// Update our particle system timer
	m_fCurrentTime += m_fElapsedTime;

	// Start at the head of the active list
	ppParticle = &m_pActiveList;
	while(*ppParticle)
	{
		pParticle = *ppParticle;

		if(HasParticleExpired(pParticle))
		{
			// Time is up, put the particle back on the free list...
			*ppParticle = pParticle->m_pNext;
			pParticle->m_pNext = m_pFreeList;
			m_pFreeList = pParticle;
			--m_lActiveCount;
		}
		else
		{
			// Still active, calculate nu position
			MoveParticle(pParticle);

			ppParticle = &pParticle->m_pNext;	// Go to next in the list
		}
	}

	//-------------------------------------------------------------------------
	// Emit nu particles in accordance to the flow rate...
	// 
	// NOTE: The system operates with a finite number of particles.
	//	   New particles will be created until the max amount has
	//	   been reached, after that, only old particles that have
	//	   been recycled can be reinitialized and used again.
	//-------------------------------------------------------------------------
	if(m_fCurrentTime - m_fLastUpdate > m_fReleaseInterval && !HasPassedReleaseTime())
	{
		// Reset update timing
		m_fLastUpdate = m_fCurrentTime;
	
		// Emit nu particles at specified flow rate...
		for(unsigned long i = 0; i < m_lNumToRelease; ++i)
		{
			if(m_lActiveCount < m_lMaxParticles)
			{
				// Do we have any free particles to put back to work?
				if(m_pFreeList)
				{
					// If so, hand over the first free one to be reused.
					pParticle = m_pFreeList;
					// Then make the next free particle in the list next to go!
					m_pFreeList = pParticle->m_pNext;
				}
				else
				{
					// There are no free particles to recycle...
					// We'll have to create a nu one from scratch!
					if(NULL == (pParticle = new auParticle))
					{
						result = 0;
						break;
					}
				}

				pParticle->m_pNext = m_pActiveList; // Make it the nu head
				m_pActiveList = pParticle;
				
				// Set the attributes for our nu particle
				InitParticle(pParticle);
				
				++m_lActiveCount;
			}
		}
	}
	
	return result;
}

void AuParticleSystem::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Q_UNUSED(projection);
	Q_UNUSED(model);
	Q_UNUSED(scale);
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle	*pParticle = m_pActiveList;

	// Render point sprites
	re.glEnable(GL_PROGRAM_POINT_SIZE); 
	//re.glPointSize(m_fSize * scale);

	// Everybody who calls this uses the SpriteSizeShader.
	// Build vertex data for that shader.
	/*
	re.glBegin(GL_POINTS);
	{
		// Render each particle
		while(pParticle)
		{
			re.glNormal3d(pParticle->m_vNorm.x,
				pParticle->m_vNorm.y,
				pParticle->m_vNorm.z);
			re.glVertex4d(pParticle->m_vCurPos.x,
						pParticle->m_vCurPos.y,
						pParticle->m_vCurPos.z, 1);	// w is the vertex alpha
			pParticle = pParticle->m_pNext;
		}
	}
	re.glEnd();
	*/

	// Compile a list of vertex locations of the particles.
	QList<AuPointAlpha> verts;
	while(pParticle)
	{
		verts << AuPointAlpha(pParticle->m_vCurPos);
		pParticle = pParticle->m_pNext;
	}

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_POINTS);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 
}

void AuParticleSystem::RenderCloud(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Q_UNUSED(projection);
	Q_UNUSED(model);
	Q_UNUSED(scale);
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle	*pParticle = GetActiveList();

	// Render point sprites
	re.glEnable(GL_PROGRAM_POINT_SIZE); 
	//re.glPointSize(GetSize() * scale);
	/*
	re.glBegin(GL_POINTS);
	{
		// Render each particle
		while(pParticle)
		{
			float partAlpha = (1.f - (GetCurTime() - pParticle->m_fInitTime) / GetLifeCycle()) / 2.f;

			re.glNormal3d(pParticle->m_vNorm.x,
				pParticle->m_vNorm.y,
				pParticle->m_vNorm.z);
			re.glVertex4d(pParticle->m_vCurPos.x,
						pParticle->m_vCurPos.y,
						pParticle->m_vCurPos.z,
						partAlpha);
			pParticle = pParticle->m_pNext;
		}
	}
	re.glEnd();
	*/

	// Compile a list of vertex locations of the particles.
	QList<AuPointAlpha> verts;
	while(pParticle)
	{
		float partAlpha = (1.f - (GetCurTime() - pParticle->m_fInitTime) / GetLifeCycle()) / 2.f;
		verts << AuPointAlpha(pParticle->m_vCurPos, partAlpha);
		pParticle = pParticle->m_pNext;
	}

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	if(vao.VAO())
	{
		vao.SelectGL();
		vao.DrawGL(GL_POINTS);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 
}

void AuParticleSystem::FreeParticles(int howMany)
{
	while(howMany--)
		FreeLast();
}

void AuParticleSystem::FreeLast()
{
	// Find the oldest particle.
	auParticle* pParticle = m_pActiveList;
	auParticle* oldestParticle = 0;
	float oldestTime = 0;
	while(pParticle)
	{
		if(!oldestParticle || pParticle->m_fInitTime < oldestTime)
		{
			oldestParticle = pParticle;
			oldestTime = pParticle->m_fInitTime;
		}
		pParticle = pParticle->m_pNext;
	}

	if(!oldestParticle)
		return;

	// Remove the oldest particle.
	auParticle** ppParticle = &m_pActiveList;
	while(*ppParticle)
	{
		pParticle = *ppParticle;

		if(pParticle == oldestParticle)
		{
			// Put the particle back on the free list...
			*ppParticle = pParticle->m_pNext;
			pParticle->m_pNext = m_pFreeList;
			m_pFreeList = pParticle;
			--m_lActiveCount;
			break;
		}
		else
			ppParticle = &pParticle->m_pNext;	// Go to next in the list
	}
}
