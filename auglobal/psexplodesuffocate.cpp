//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodesuffocate.cpp,v 1.13 2022/02/19 18:34:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodesuffocate.h"
//
#include "augleffects.h"

PsExplodeSuffocate::PsExplodeSuffocate(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(20);
	SetLifeCycle(2.5f);
	SetVelocityVar(radius/2.f);
	SetNumToRelease(20);
	SetReleaseInterval(0.f);
	QColor col(172, 99, 39);
	SetColor(col);
	SetSize((sqrt(gain) + 10) * 100);
	SetTextureID(texId);
	SetEndReleaseTime(1.f);
}

PsExplodeSuffocate::~PsExplodeSuffocate()
{

}

void PsExplodeSuffocate::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsExplodeSuffocate::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurPos = GetPosition() + GetRandomVector() * (GetVelocityVar()*2.5f);
	pParticle->m_vCurVel = GetPosition() - pParticle->m_vCurPos;				// Toward position.
	pParticle->m_vCurVel.Unit(GetVelocityVar());
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2();
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsExplodeSuffocate::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	//qDebug() << "PsExplodeSuffocate::Render" << GetSize();
	Init(re);
	float colAlpha;
	if(GetCurTime() < .5f)
		colAlpha = GetCurTime() / .5f;
	else if(GetCurTime() <= 2.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 2.f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}
	QColor col(GetColor());
	col.setAlphaF(colAlpha/10.f);
	SetColor(col);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(col);
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 *c_UnitScale * scale);

	// Render the model of this atom.
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	AuParticleSystem::Render(projection, model, re, scale);

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
