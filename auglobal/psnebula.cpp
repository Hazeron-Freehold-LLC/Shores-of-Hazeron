//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psnebula.cpp,v 1.11 2022/02/19 18:34:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psnebula.h"
//
#include "audef.h"
#include "augleffects.h"
#include "auglobal.h"

PsNebula::PsNebula() : AuParticleSystem()
{
	m_Density = 0;
	float size = FeetPerSolarSystem();

	SetLifeCycle(20.f);
	SetReleaseInterval(.2f);
	SetSize(size);
}

PsNebula::~PsNebula()
{

}

bool PsNebula::HasParticleExpired(const auParticle* pParticle)
{
	float fTimePassed  = GetCurTime() - pParticle->m_fInitTime;
	if(fTimePassed >= GetLifeCycle())
		return true;

	// Check for distant particle moving away.
	double maxdist = FeetPerSolarSystem()/10;
	double curdist2 = pParticle->m_vCurPos.Length2();
	if(curdist2 > maxdist*maxdist && pParticle->m_vOldPos.Length2() < curdist2)
		return true;	// Far away particle is moving away.

	return false;
}

void PsNebula::InitParticle(auParticle* pParticle)
{
	double mindist = 1000 * FeetPerMeter();
	double maxdist = FeetPerSolarSystem()/10;
	double initDist = g_Au.Dice().Roll(1, maxdist-mindist, mindist);
	pParticle->m_vCurPos = GetPosition() + GetRandomVector() * initDist + GetVelocity() * 2;	// seconds out ahead
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2() + 1;	// Particle is killed when it moves farther away.
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsNebula::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(GetColor());
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 * scale);

	// Render the nebula particle effect.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if(Update())
	{
		// Compute billboard vertices's on the GPU using ARB_point_sprites
		auParticle* pParticle = GetActiveList();

		// Render point sprites
		re.glEnable(GL_PROGRAM_POINT_SIZE); 
		//re.glPointSize(GetSize() * scale);

		// Compile a list of vertex locations of the particles.
		unsigned int particles = 0;
		QList<AuPointAlpha> verts;
		while(pParticle)
		{
			double maxdist = GetSize()/10;
			float partAlpha = (float)(1. - pParticle->m_vCurPos.Length() / maxdist);
			if(partAlpha > 0)
			{
				float elapsed = GetCurTime() - pParticle->m_fInitTime;	// Time the particle has been alive.
				if(elapsed < 3.f && partAlpha > 0)
				{
					// New particle inside the maxdist visibility radius fades in.
					float timeAlpha = elapsed / 3.f;
					partAlpha *= timeAlpha;
				}
				else
				{
					float fade = GetLifeCycle() - 5.f;						// Fade out in last seconds.
					if(elapsed > fade)
					{
						float timeAlpha = (1.f - (elapsed - fade) / (GetLifeCycle() - fade));
						partAlpha *= timeAlpha;
					}
				}
			}

			verts << AuPointAlpha(pParticle->m_vCurPos, partAlpha);
			pParticle = pParticle->m_pNext;
			particles++;
		}

		if(GetVelocity().IsZero())
		{
			// Not moving.
			int maxParticles = (int)(10 + 30 * m_Density);
			SetMaxParticles(maxParticles);
			if(particles < 10)
				SetNumToRelease(10 - particles);
			else
				SetNumToRelease(1);
		}
		else
		{
			// Moving
			int maxParticles = (int)(20 + 80 * m_Density);
			SetMaxParticles(maxParticles);
			SetNumToRelease(3);
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
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void PsNebula::Transform(const AuMatrix4x4& m)
{
	auParticle* pParticle = GetActiveList();
	while(pParticle)
	{
		pParticle->m_vOldPos = pParticle->m_vCurPos;
		pParticle->m_vCurPos = m.Project(pParticle->m_vCurPos);
		pParticle->m_InitDist = pParticle->m_vCurPos.Length2() + 1;	// Particle is killed when it moves farther away.
		pParticle = pParticle->m_pNext;
	}
}
