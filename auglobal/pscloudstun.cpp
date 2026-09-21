//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pscloudstun.cpp,v 1.15 2022/02/19 18:33:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "pscloudstun.h"
//
#include "augleffects.h"

PsCloudStun::PsCloudStun(float radius, float gain, GLuint texId) : AuParticleSystem()
{
	Q_UNUSED(gain);
	SetMaxParticles(50);
	SetLifeCycle(.4f);
	SetVelocityVar(radius/.4f / 2);
	SetNumToRelease(1);
	SetReleaseInterval(0.1f);
	QColor col(62, 25, 172);
	SetColor(col);
	//SetSize(((sqrt(gain) + 10) * 10) / 5);
	SetSize(radius);
	SetTextureID(texId);
}

PsCloudStun::~PsCloudStun()
{

}

void PsCloudStun::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsCloudStun::InitParticle(auParticle* pParticle)
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

void PsCloudStun::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(GetColor());

	// Draw field effect
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	re.glEnable(GL_PROGRAM_POINT_SIZE); 
	re.glDisable(GL_CULL_FACE);

	AuPoint emitPos = GetPosition();

	float size = (GetSize() * 200.f * scale) + (GetSize() * 20.f * fabsf(sinf(GetCurTime() * 20.f)));
	//re.glPointSize(size * c_UnitScale);
	re.GLSL().SpriteSizeShader().SetSize(size *c_UnitScale);

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
