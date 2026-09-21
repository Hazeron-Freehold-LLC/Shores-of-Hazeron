//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodecold.cpp,v 1.13 2022/02/19 18:33:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodecold.h"
//
#include "augleffects.h"

PsExplodeCold::PsExplodeCold(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(500);
	SetLifeCycle(1.5f);
	SetVelocityVar(radius*2);
	SetNumToRelease(500);
	SetReleaseInterval(0.f);
	QColor col(167, 246, 255);
	SetColor(col);
	SetSize((sqrt(gain) + 5) * 10);
	SetTextureID(texId);
	SetEndReleaseTime(1.f);
}

PsExplodeCold::~PsExplodeCold()
{

}

void PsExplodeCold::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	if(GetCurTime() <= 1.f)
	{
		pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
		double len = pParticle->m_vCurPos.Length();
		double radius = GetVelocityVar()/3;
		if(len > radius)
			pParticle->m_vCurPos *= radius/len;
	}
	else
		pParticle->m_vCurPos += pParticle->m_vCurVel * (powf(10.f, GetElapsedTime())/3.f);
}

void PsExplodeCold::InitParticle(auParticle* pParticle)
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

void PsExplodeCold::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	//qDebug() << "PsExplodeCold::Render" << GetSize();
	float colAlpha;
	if(GetCurTime() <= 1.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 1.f)/.5f;
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
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

	AuParticleSystem::Render(projection, model, re, scale);

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
