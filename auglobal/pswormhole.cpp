//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pswormhole.cpp,v 1.20 2022/02/19 18:34:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "pswormhole.h"
//
#include "augleffects.h"

PsWormhole::PsWormhole() : AuParticleSystem()
{
	m_bOrigin = false;
}

PsWormhole::~PsWormhole()
{

}

void PsWormhole::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	AuPoint vRandomVec = GetRandomVector();
	if(GetVelocityVar() != 0.0f)
		pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
	pParticle->m_vCurVel.x = fabs(vRandomVec.x*2*PIf);
	pParticle->m_fInitTime  = GetCurTime();
}

void PsWormhole::MoveParticle(auParticle* pParticle)
{
	// Calculate position based off time
	float fTimePassed = GetCurTime() - pParticle->m_fInitTime;
	if(m_bOrigin)	// 1 = origin (blue particles in)
		fTimePassed = (GetLifeCycle() - fTimePassed);
	float fTime = fTimePassed/GetLifeCycle();
	if(m_bOrigin)
	{
		if(fTime < .05f)
			fTime = .05f;
	}
	else
	{
		if(fTime < .025f)
			fTime = .025f;
	}

	// The number multiplied by cos(fTimePassed) needs to be changed when the scale of things changes. See auwormhole.cpp.
	pParticle->m_vCurPos.x = cos(pParticle->m_vCurVel.x) * fTime * GetScale();
	pParticle->m_vCurPos.y = sin(pParticle->m_vCurVel.x) * fTime * GetScale();
	pParticle->m_vCurPos.z = 1000 * cos(fTimePassed) / fTime;
}

void PsWormhole::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(GetColor());
	re.GLSL().SpriteSizeShader().SetSize(GetSize() * scale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	//AuParticleSystem::Render(projection, model, re);
	{
		// Compute billboard vertices's on the GPU using ARB_point_sprites
		auParticle	*pParticle = GetActiveList();

		// Render point sprites
		re.glEnable(GL_PROGRAM_POINT_SIZE); 
		//re.glPointSize(GetSize() * scale);

		// Compile a list of vertex locations.
		QList<AuPointAlpha> verts;
		while(pParticle)
		{
			if(pParticle->m_vCurPos.z < 9000)
			{
				verts << AuPointAlpha(pParticle->m_vCurPos.x,
							pParticle->m_vCurPos.y,
							pParticle->m_vCurPos.z, 1);	// w is the vertex alpha
			}
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

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
