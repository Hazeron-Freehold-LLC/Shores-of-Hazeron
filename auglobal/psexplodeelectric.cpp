//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodeelectric.cpp,v 1.17 2022/02/19 18:33:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodeelectric.h"
//
#include "augleffects.h"
#include "auglobal.h"

PsExplodeElectric::PsExplodeElectric(float radius, float gain, GLuint texId, GLuint texIdGlow) : AuParticleSystem()
{
	SetMaxParticles(100);
	SetLifeCycle(.5f);
	SetVelocityVar(radius);
	SetNumToRelease(2);
	SetReleaseInterval(0.1f);
	QColor col(255,255,255);
	SetColor(col);
	SetSize((sqrt(gain) + 15) * 10);
	SetTextureID(texId);
	SetTextureID2(texIdGlow);
	SetEndReleaseTime(.9f);				// Prevent additional waves of particles.
}

PsExplodeElectric::~PsExplodeElectric()
{

}

void PsExplodeElectric::MoveParticle(auParticle* /*pParticle*/)
{
	// Update position with respect to velocity
	//pParticle->m_vOldPos = pParticle->m_vCurPos;
	//pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
	//float lengthMod = g_Au.Dice().Roll(1,100,-50)/100.f + 1.f;
	//pParticle->m_vCurPos = GetPosition() + GetRandomVector() * (GetVelocityVar() * lengthMod);
}

void PsExplodeElectric::InitParticle(auParticle* pParticle)
{
	//pParticle->m_vCurVel = GetVelocity();
	//AuPoint vRandomVec = GetRandomVector();
	//if(GetVelocityVar() != 0.0f)
		//pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
	pParticle->m_vCurPos = GetPosition() + GetRandomVector() * (GetVelocityVar() * 1.5f);
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2();
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsExplodeElectric::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	//qDebug() << "PsExplodeElectric::Render" << GetSize();
	float colAlpha;
	if(GetCurTime() <= 1.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - 1.f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}

	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle* pParticle = GetActiveList();
	if(!pParticle)
		return;

	// Create a list of triangles to draw the explosion.
	QList<AuTriTex> tris;
	AuPoint emitPos = GetPosition();
	AuPoint vNormal = model.UnProjectVector(AuPoint(0,0,-1));		// Give me a vector that will project into a 0,0,-1 normal.
	AuPoint vPoint0, vPoint1, vPoint2, vPoint3;
	float width = GetSize() / 10 * c_UnitScale * scale;					// 1/2 the width of the billboard sprite
	while(pParticle)
	{
		auParticle *pNext = pParticle->m_pNext;
		if(pNext)
		{
			AuPoint vNext(pNext->m_vCurPos);					// In the space of the host's parent atom, to prevent wigglies.

			AuPoint vDir = emitPos - vNext;
			AuPoint vPoint = vNormal.Cross(vDir);
			vPoint.Unit();
			if(vPoint.y < 0)
				vPoint = -vPoint;

			vPoint0 = emitPos - vPoint*width;
			vPoint1 = emitPos + vPoint*width;
			vPoint2 = vNext + vPoint*width;
			vPoint3 = vNext - vPoint*width;

			AuTriTex t0;
			t0.m_V0 = AuPointTex(vPoint0, 1,0);
			t0.m_V1 = AuPointTex(vPoint1, 0,0);
			t0.m_V2 = AuPointTex(vPoint2, 0,1);
			tris.append(t0);

			AuTriTex t1;
			t0.m_V0 = AuPointTex(vPoint2, 0,1);
			t0.m_V1 = AuPointTex(vPoint3, 1,1);
			t0.m_V2 = AuPointTex(vPoint0, 1,0);
			tris.append(t1);
		}
		pParticle = pParticle->m_pNext;
	}
	if(tris.isEmpty())
		return;

	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glDisable(GL_CULL_FACE);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Render the trail.
	if(!tris.isEmpty())
	{
		// Create a VAO of the trail triangles.
		AuGLVAO vao;
		vao.CreateForTexture(re, tris, GL_STREAM_DRAW);

		// Render the VAO.
		re.GLSL().TextureShader().Select();
		re.GLSL().TextureShader().SetProjectionModelView(projection * model);
		re.GLSL().TextureShader().SetTexture(GetTextureID());
		re.GLSL().TextureShader().SetColor(m_clrColor.redF(), m_clrColor.greenF(), m_clrColor.blueF(), colAlpha);

		vao.SelectGL();
		vao.DrawGL();
		vao.DeselectGL();
		vao.DeleteGL();

		re.GLSL().TextureShader().Deselect();
	}

	// Draw glow effect
	int effectPulses = g_Au.Dice().Roll(1,3);
	if(effectPulses > 0)
	{
		// Draw glow effect
		re.GLSL().SpriteSizeShader().Select();
		re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
		re.GLSL().SpriteSizeShader().SetTexture(GetTextureID2());
		re.GLSL().SpriteSizeShader().SetColor(125.f/255.f, 118.f/255.f, 196.f/255.f, colAlpha);
		re.GLSL().SpriteSizeShader().SetSize(GetSize()*50 *c_UnitScale * scale);

		// Render point sprites
		re.glEnable(GL_PROGRAM_POINT_SIZE); 
		//re.glPointSize(GetSize() * scale);

		// Compile a list of vertex locations.
		QList<AuPointAlpha> verts;
		for(int i=0; i<effectPulses; i++)
			verts << AuPointAlpha(emitPos.x, emitPos.y, emitPos.z, 1);

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		vao.SelectGL();
		vao.DrawGL(GL_POINTS);
		vao.DeselectGL();
		vao.DeleteGL();

		re.glDisable(GL_PROGRAM_POINT_SIZE); 

		re.GLSL().SpriteSizeShader().Deselect();
	}

	re.glDisable(GL_BLEND);
	re.glEnable(GL_CULL_FACE);
	re.glDepthMask(GL_TRUE);
}
