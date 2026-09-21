//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodedisease.cpp,v 1.14 2022/02/19 18:33:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodedisease.h"
//
#include "augleffects.h"

PsExplodeDisease::PsExplodeDisease(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(100);
	SetLifeCycle(2.5f);
	SetVelocityVar(radius * .65f);
	SetNumToRelease(4);
	SetReleaseInterval(0.1f);
	QColor col(128,128,128);
	SetColor(col);
	SetSize((sqrt(gain) + 5) * 10);
	SetTextureID(texId);
}

PsExplodeDisease::~PsExplodeDisease()
{

}

void PsExplodeDisease::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
	pParticle->m_vCurPos += GetRandomVector() * (GetVelocityVar() * GetElapsedTime());
}

void PsExplodeDisease::InitParticle(auParticle* pParticle)
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

void PsExplodeDisease::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	// Fade away at the end.
	float colAlpha;
	if(GetCurTime() <= 2.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 2.f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}
	QColor col(GetColor());
	col.setAlphaF(colAlpha);
	SetColor(col);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(col);
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 *c_UnitScale * scale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	AuParticleSystem::Render(projection, model, re, scale);

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
