//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodestun.cpp,v 1.19 2022/02/19 18:34:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodestun.h"
//
#include "augleffects.h"

PsExplodeStun::PsExplodeStun(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(50);
	SetLifeCycle(.4f);
	SetVelocityVar(radius/.4f);
	SetNumToRelease(50);
	SetReleaseInterval(0.0f);
	QColor col(62, 25, 172);
	SetColor(col);
	SetSize((sqrt(gain) + 10) * 10);
	SetTextureID(texId);
	SetEndReleaseTime(.3f);				// Prevent additional waves of particles.
}

PsExplodeStun::~PsExplodeStun()
{

}

void PsExplodeStun::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsExplodeStun::InitParticle(auParticle* pParticle)
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

void PsExplodeStun::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	if(GetCurTime() == 0)
		return;

	//qDebug() << "PsExplodeSonic::Render" << GetSize();
	float colAlpha;
	if(GetCurTime() <= .5f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - .5f)/.5f;
		if(colAlpha < 0)
			colAlpha = 0;
	}
	QColor col = GetColor();
	col.setAlphaF(colAlpha);
	SetColor(col);

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(GetColor());
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*100 *c_UnitScale * scale);

	re.glDisable(GL_CULL_FACE);

	// Draw field effect
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	AuParticleSystem::Render(projection, model, re, scale);

	re.glEnable(GL_PROGRAM_POINT_SIZE); 
	re.glDisable(GL_CULL_FACE);

	AuPoint emitPos = GetPosition();

	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID2());

	float size = GetSize() * 300 * GetCurTime() + 1;
	//re.glPointSize(size * c_UnitScale * scale);
	re.GLSL().SpriteSizeShader().SetSize(size *c_UnitScale * scale);

	// Compile a list of vertex locations.
	QList<AuPointAlpha> verts;
	verts << AuPointAlpha(emitPos.x, emitPos.y, emitPos.z, 1);

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_POINTS);
	vao.DeselectGL();
	vao.DeleteGL();

	re.glEnable(GL_CULL_FACE);
	re.glDisable(GL_PROGRAM_POINT_SIZE); 

	re.glDisable(GL_BLEND);
	re.glDepthMask(GL_TRUE);

	re.GLSL().SpriteSizeShader().Deselect();
}
