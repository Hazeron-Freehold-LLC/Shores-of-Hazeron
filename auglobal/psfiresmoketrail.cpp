//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psfiresmoketrail.cpp,v 1.20 2022/02/19 18:34:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psfiresmoketrail.h"
//
#include "augleffects.h"

PsFireSmokeTrail::PsFireSmokeTrail() : PsEngineTrail()
{
}

PsFireSmokeTrail::~PsFireSmokeTrail()
{
}

void PsFireSmokeTrail::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsFireSmokeTrail::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity()*2;
	AuPoint vRandomVec = GetRandomVector();
	if(GetVelocity().y)
	{
		pParticle->m_vCurVel.x = vRandomVec.x * GetVelocityVar();
		pParticle->m_vCurVel.y *= fabs(vRandomVec.y);
		pParticle->m_vCurVel.z = vRandomVec.z * GetVelocityVar();
	}
	else
	{
		pParticle->m_vCurVel.x = vRandomVec.x * GetVelocityVar();
		pParticle->m_vCurVel.y = vRandomVec.z * GetVelocityVar();
		pParticle->m_vCurVel.z *= fabs(vRandomVec.y);
	}
	pParticle->m_vCurPos = GetPosition();
	pParticle->m_fInitTime = GetCurTime();
}

void PsFireSmokeTrail::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle	*pParticle = GetActiveList();

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	if(GetVelocity().y)
	{
		float size = GetSize() * fabs(GetVelocity().y)/2.;
		if(size < GetSize()*15)
			size = GetSize()*15;
		size *= scale;
		re.GLSL().SpriteSizeShader().SetSize(size);
	}
	else
	{
		float size = GetSize() * fabs(GetVelocity().z)/2.;
		if(size < GetSize()*20)
			size = GetSize()*20;
		size *= scale;
		re.GLSL().SpriteSizeShader().SetSize(size);
	}
	re.GLSL().SpriteSizeShader().SetColor(m_clrColor.redF(), m_clrColor.greenF(), m_clrColor.blueF(), 1);

	// Render point sprites
	re.glEnable(GL_PROGRAM_POINT_SIZE);

	// Compile a list of vertex locations.
	QList<AuPointAlpha> verts;
	while(pParticle)
	{
		// Modify color over time
		//float fMod = ((GetCurTime() - pParticle->m_fInitTime)/GetLifeCycle())/2.0;
		//re.GLSL().SpriteSizeShader().SetColor(m_clrColor.redF()*fMod, m_clrColor.greenF()*fMod, m_clrColor.blueF()*fMod, 1);
		verts << AuPointAlpha(pParticle->m_vCurPos.x, pParticle->m_vCurPos.y, pParticle->m_vCurPos.z, 1);
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

	re.GLSL().SpriteSizeShader().Deselect();
}
