//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodeheat.cpp,v 1.15 2022/02/19 18:33:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodeheat.h"
//
#include "augleffects.h"

PsExplodeHeat::PsExplodeHeat(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	Q_UNUSED(gain);
	SetMaxParticles(250);
	SetLifeCycle(2.f);
	SetVelocityVar(radius * .67f);
	SetNumToRelease(100);
	SetReleaseInterval(0.15f);
	QColor colFire;
	colFire.setRgbF(1, .25, .15);
	SetColor(colFire);
	SetSize(radius);
	SetTextureID(texId);
	SetEndReleaseTime(.9f);
}

PsExplodeHeat::~PsExplodeHeat()
{

}

void PsExplodeHeat::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsExplodeHeat::InitParticle(auParticle* pParticle)
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

void PsExplodeHeat::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	//qDebug() << "PsExplodeHeat::Render" << GetSize();
	float colAlpha;
	if(GetCurTime() <= 1.5f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 1.5f)/.5f;
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
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*100 *c_UnitScale * scale);

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
