//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodesonic.cpp,v 1.19 2022/02/19 18:34:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psexplodesonic.h"
//
#include "augleffects.h"

PsExplodeSonic::PsExplodeSonic(float radius, float lifetime, float gain, GLuint texId) : AuParticleSystem()
{
	SetMaxParticles(0);
	SetLifeCycle(lifetime);
	SetVelocityVar(radius);
	SetNumToRelease(0);
	SetReleaseInterval(0.f);
	QColor col(1, 1, 1);
	SetColor(col);
	SetSize((sqrt(gain) + 5) * 10);
	SetTextureID(texId);
	SetEndReleaseTime(.9f);				// Prevent additional waves of particles.
}

PsExplodeSonic::~PsExplodeSonic()
{

}

void PsExplodeSonic::MoveParticle(auParticle* /*pParticle*/)
{
	// Update position with respect to velocity
	//pParticle->m_vOldPos = pParticle->m_vCurPos;
	//pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
	//float lengthMod = g_Au.Dice().Roll(1,100,-50)/100.f + 1.f;
	//pParticle->m_vCurPos = GetPosition() + GetRandomVector() * (GetVelocityVar() * lengthMod);
}

void PsExplodeSonic::InitParticle(auParticle* pParticle)
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

void PsExplodeSonic::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	Init(re);
	//qDebug() << "PsExplodeSonic::Render" << GetSize();
	float colAlpha;
	if(GetCurTime() <= GetLifeCycle() - 1.f)
		colAlpha = 1.f;
	else
	{
		colAlpha = 1.f - (GetCurTime() - (GetLifeCycle() - 1.f));
		if(colAlpha < 0)
			colAlpha = 0;
	}

	// Render point sprites
	re.glEnable(GL_PROGRAM_POINT_SIZE); 
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	re.glDisable(GL_CULL_FACE);

	// Draw glow effect
	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID());
	re.GLSL().SpriteSizeShader().SetColor(198.f/255.f, 2.f/255.f, 255.f/255.f, colAlpha/3.f);
	re.GLSL().SpriteSizeShader().SetSize(GetSize()*500 *c_UnitScale * scale);

	AuPoint emitPos = GetPosition();

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

	// Draw an overlay that grows to maximum radius in one second and keeps on going.
	float radius = GetVelocityVar() * 500 * GetCurTime()/GetLifeCycle() * c_UnitScale * scale;
	re.GLSL().SpriteSizeShader().SetSize(radius);
	verts.clear();
	verts << AuPointAlpha(emitPos.x, emitPos.y, emitPos.z, 1.);

	// Make a VAO of the vertex locations.
	vao.CreateForSprites(re, verts, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_POINTS);
	vao.DeselectGL();
	vao.DeleteGL();

	re.glEnable(GL_CULL_FACE);
	re.glDisable(GL_BLEND);
	re.glDepthMask(GL_TRUE);

	re.glDisable(GL_PROGRAM_POINT_SIZE); 

	re.GLSL().SpriteSizeShader().Deselect();
}
