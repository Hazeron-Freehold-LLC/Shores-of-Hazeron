//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodepierce.cpp,v 1.22 2022/02/19 18:33:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodepierce.h"
//
#include "augleffects.h"

PsExplodePierce::PsExplodePierce(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	m_Radius = radius*1.25f;

	if(radius < 6)
		SetMaxParticles(10);
	else
		SetMaxParticles(20);
	SetLifeCycle(1.5f);
	SetVelocityVar(m_Radius*4.f);
	SetNumToRelease(20);
	SetReleaseInterval(0.0f);
	QColor col(255, 20, 15);
	SetColor(col);
	SetSize(sqrtf(gain) * 10 + 10);
	SetTextureID(texId);
	SetEndReleaseTime(.5f);
}

PsExplodePierce::~PsExplodePierce()
{

}

void PsExplodePierce::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
	float len = pParticle->m_vCurPos.Length();
	if(len > m_Radius)
		pParticle->m_vCurPos *= m_Radius/len;
}

void PsExplodePierce::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	AuPoint vRandomVec = GetRandomVector();
	if(GetVelocityVar() != 0.0f)
		pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
	pParticle->m_vCurPos = GetPosition() + vRandomVec/12.;
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2();
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsExplodePierce::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle* pParticle = GetActiveList();
	if(!pParticle)
		return;

	float colAlpha;
	if(GetCurTime() <= 1.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 1.f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(GetColor().redF(), GetColor().greenF(), GetColor().blueF(), colAlpha);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

	// Specify point sprite texture coordinate replacement mode for each texture unit
	re.glEnable(GL_PROGRAM_POINT_SIZE); 

	// Draw each particle as a stream from tip to core, getting larger as we go, for a sharp pointy spear effect.
	float curlen = pParticle->m_vCurPos.Length();
	int sizes = (int)ceilf(curlen / 1.5f);
	float sizeIncrease;
	if(sizes < 10)
	{
		sizes = 10;
		sizeIncrease = 1.04f;
	}
	else
	{
		if(sizes > 40)
			sizes = 40;
		sizeIncrease = 1.1f;
	}
	float radiusDec = 1.f / (float)sizes;
	float ptSize = GetSize();
	float ptRadius = 1.f;
	for(int size=0; size<sizes; size++)
	{
		// Adjust point size
		re.GLSL().SpriteSizeShader().SetSize(ptSize*10 *c_UnitScale * scale);

		// Render point sprites
		//re.glPointSize(ptSize * scale);

		// Compile a list of vertex locations.
		QList<AuPointAlpha> verts;
		auParticle* pParticle = GetActiveList();
		while(pParticle)
		{
			AuPoint loc = pParticle->m_vCurPos;
			loc *= ptRadius;
			verts << AuPointAlpha(loc.x, loc.y, loc.z, 1);
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

		ptSize *= sizeIncrease;			// Each pass will get larger.
		ptRadius -= radiusDec;
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 

	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}
