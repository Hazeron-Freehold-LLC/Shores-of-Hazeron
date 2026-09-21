//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodebludgeon.cpp,v 1.15 2022/02/19 18:33:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodebludgeon.h"
//
#include "augleffects.h"

PsExplodeBludgeon::PsExplodeBludgeon(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(500);
	SetLifeCycle(1.f);
	SetVelocityVar(radius*2);
	SetNumToRelease(500);
	SetReleaseInterval(0.0f);
	QColor col(228, 171, 75);
	SetColor(col);
	SetSize((sqrt(gain) + 5) * 15);
	SetTextureID(texId);
	SetEndReleaseTime(.4f);
}

PsExplodeBludgeon::~PsExplodeBludgeon()
{

}

void PsExplodeBludgeon::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsExplodeBludgeon::InitParticle(auParticle* pParticle)
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

void PsExplodeBludgeon::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	// Fade away at the end.
	float colAlpha;
	if(GetCurTime() <= .5f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - .5f)/.5f;
		if(colAlpha <= 0)
			return;
	}
	QColor col(GetColor());
	col.setAlphaF(colAlpha);
	SetColor(col);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(col);
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 * colAlpha *c_UnitScale * scale);

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
