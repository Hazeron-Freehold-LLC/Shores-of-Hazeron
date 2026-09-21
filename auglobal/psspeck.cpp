//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psspeck.cpp,v 1.12 2022/02/19 18:34:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psspeck.h"
//
#include "audef.h"
#include "augleffects.h"
#include "auglobal.h"

#define d_MinDist						(FeetPerMoonOrbit()*2)
#define d_MaxDist						(FeetPerOrbit())

PsSpeck::PsSpeck() : AuParticleSystem()
{
	m_Density = 0;

	SetLifeCycle(5.f);
	SetMaxParticles(10000);
	SetReleaseInterval(.5f);
	SetSize(6000.);
}

PsSpeck::~PsSpeck()
{

}

bool PsSpeck::HasParticleExpired(const auParticle* pParticle)
{
	float fTimePassed  = GetCurTime() - pParticle->m_fInitTime;
	if(fTimePassed >= GetLifeCycle())
		return true;

	double olddist2 = pParticle->m_vOldPos.Length2();
	double curdist2 = pParticle->m_vCurPos.Length2();
	if(curdist2 > olddist2)
	{
		// Particle is moving away.
		if(curdist2 > d_MaxDist/2*d_MaxDist/2)
			return true;	// Far away particle moving away.
	}
	else
	{
		// Particle is approaching or the same.
		if(curdist2 < d_MinDist/2*d_MinDist/2)
			return true;	// Particle is too close.
	}
	return false;
}

void PsSpeck::InitParticle(auParticle* pParticle)
{
	double initDist = g_Au.Dice().Roll(1, d_MaxDist-d_MinDist, d_MinDist);
	pParticle->m_vCurPos = GetPosition() + GetRandomVector() * initDist + GetVelocity() * 2;	// seconds out ahead
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2() + 1;	// Particle is killed when it moves farther away.
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsSpeck::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
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
		QList<AuPointAlpha> verts;
		while(pParticle)
		{
			float partAlpha = (float)(1. - pParticle->m_vCurPos.Length() / d_MaxDist);
			if(partAlpha > 0)
			{
				float elapsed = GetCurTime() - pParticle->m_fInitTime;	// Time the particle has been alive.
				if(elapsed < .5f && partAlpha > 0)
				{
					// New particle inside the maxdist visibility radius fades in.
					float timeAlpha = elapsed / .5f;
					partAlpha *= timeAlpha;
				}
				else
				{
					float fade = GetLifeCycle() - .5f;						// Fade out in last milliseconds.
					if(elapsed > fade)
					{
						float timeAlpha = (1.f - (elapsed - fade) / (GetLifeCycle() - fade));
						partAlpha *= timeAlpha;
					}
				}
			}

			AuPoint eyeSpaceLoc = model.Project(pParticle->m_vCurPos);
			if(eyeSpaceLoc.z < -d_MinDist/2)
				verts << AuPointAlpha(pParticle->m_vCurPos, partAlpha);
			pParticle = pParticle->m_pNext;
		}

		if(GetVelocity().IsZero())
			SetNumToRelease(0);	// Not moving.
		else
			SetNumToRelease(500 * m_Density);	// Moving

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

void PsSpeck::Transform(const AuMatrix4x4& m)
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
