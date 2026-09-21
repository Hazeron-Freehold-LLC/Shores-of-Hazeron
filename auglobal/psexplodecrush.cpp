//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodecrush.cpp,v 1.12 2022/02/19 18:33:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodecrush.h"
//
#include "augleffects.h"

PsExplodeCrush::PsExplodeCrush(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	m_Radius = radius * 2.5;

	SetMaxParticles(50);
	SetLifeCycle(1.5f);
	SetVelocityVar(m_Radius);
	SetNumToRelease(50);
	SetReleaseInterval(0.0f);
	QColor col(155, 1, 1);
	SetColor(col);
	SetSize((sqrt(gain) + 5) * 25);
	SetTextureID(texId);
	SetEndReleaseTime(.9f);				// Prevent additional waves of particles.
}

PsExplodeCrush::~PsExplodeCrush()
{

}

void PsExplodeCrush::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	if(GetCurTime() < .5f)
		pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
	else if(GetCurTime() < .6f)
		pParticle->m_vCurPos.Unit(m_Radius/2.f);
	else
		pParticle->m_vCurPos -= pParticle->m_vCurVel * GetElapsedTime() * 1.25;
}

void PsExplodeCrush::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	AuPoint vRandomVec = GetRandomVector();
	if(GetVelocityVar() != 0.0f)
		pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
	pParticle->m_vCurPos = GetPosition() + GetRandomVector()/2;
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2();
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsExplodeCrush::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	float colAlpha;
	if(GetCurTime() <= 1.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 1.f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}

	// Turn redder as time passes.
	float colScale = GetCurTime();
	if(colScale == 0)
		return;
	if(colScale > 1.f)
		colScale = 1.f;
	QColor col(GetColor());
	col.setHsvF(0, colScale, 61.f/100.f);
	col.setAlphaF(colAlpha);
	SetColor(col);

	float sizeTime = GetSize()*10;
	sizeTime *= colScale * 3.f;

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(col);
	re.GLSL().SpriteSizeShader().SetSize(sizeTime *c_UnitScale * scale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

	AuParticleSystem::Render(projection, model, re, scale);

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
