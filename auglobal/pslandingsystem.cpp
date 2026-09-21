//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pslandingsystem.cpp,v 1.13 2022/02/19 18:34:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "pslandingsystem.h"
//
#include "audef.h"
#include "augleffects.h"
#include "auglobal.h"

PsLandingSystem::PsLandingSystem(GLuint texIdSpot, GLuint texIdStreak) : AuParticleSystem()
{
	m_SegScale = .5f/100.f;
	m_Switch = false;
	SetMaxParticles(1);
	SetNumToRelease(1);
	SetReleaseInterval(1.0f);
	SetSize((float)(100*FeetPerMeter() * c_UnitScale));	// Size of dots.
	SetTextureID(texIdStreak);
	SetTextureID2(texIdSpot);
	SetVelocity(AuPoint(0, -SpacecraftRecoveryLength(),0));
}

PsLandingSystem::~PsLandingSystem()
{
}

bool PsLandingSystem::HasParticleExpired(const auParticle* pParticle)
{
	return pParticle->m_vCurPos.Dot(GetVelocity()) >= 0;
}

void PsLandingSystem::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsLandingSystem::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	pParticle->m_vCurPos = GetPosition() + -GetVelocity();
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

void PsLandingSystem::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	// Create a list of triangles to draw the trail.
	AuPoint emitPos;
	AuPoint endPos(-GetVelocity());

	// Calculate quad points, billboarded flat to the screen.
	AuPoint vNormal = model.UnProjectVector(AuPoint(0,0,-1));		// Give me a vector that will project into a 0,0,-1 normal.
	vNormal.Unit();
	AuPoint vDir(0,1,0);
	AuPoint vPoint = vNormal.Cross(vDir);
	vPoint.Unit();
	if(vPoint.y < 0)
		vPoint = -vPoint;

	float fNewSize = GetSize() * m_SegScale * scale;
	vPoint *= fNewSize;
	AuPoint vPrev0 = emitPos - vPoint;
	AuPoint vPrev1 = emitPos + vPoint;
	AuPoint vPoint0 = endPos - vPoint;
	AuPoint vPoint1 = endPos + vPoint;

	// Make triangles.
	//const float himaxx = 128;	// width of the texture
	const float himaxy = 64;	// height of the texture
	const float tt0 = 0.5f/himaxy;
	const float tt1 = 63.5f/himaxy;
	QList<AuTriTex> tris;

	AuTriTex t0;
	t0.m_V0 = AuPointTex(vPrev0, 0,tt0);
	t0.m_V1 = AuPointTex(vPrev1, 0,tt1);
	t0.m_V2 = AuPointTex(vPoint1, 1,tt1);
	tris.append(t0);

	AuTriTex t1;
	t1.m_V0 = AuPointTex(vPoint1, 1,tt1);
	t1.m_V1 = AuPointTex(vPoint0, 1,tt0);
	t1.m_V2 = AuPointTex(vPrev0, 0,tt0);
	tris.append(t1);

	re.glDisable(GL_CULL_FACE);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Runway number.
	{
		double bot = SpacecraftRecoveryLength() - 10*FeetPerMeter();
		double top = SpacecraftRecoveryLength() - SpacecraftRecoveryWidth()/2 - 10*FeetPerMeter();
		AuPoint ll(SpacecraftRecoveryWidth()/2 - FeetPerMeter(), bot, 1);
		AuPoint ul(SpacecraftRecoveryWidth()/2 - FeetPerMeter(), top, 1);
		AuPoint ur(-(SpacecraftRecoveryWidth()/2 - FeetPerMeter()), top, FeetPerMeter());
		AuPoint lr(-(SpacecraftRecoveryWidth()/2 - FeetPerMeter()), bot, FeetPerMeter());
		QString str = QString("%1").arg(m_Number);
		re.RenderText(projection, model, ll, ul, ur, lr, str, QFont(g_Au.TypefaceSans(),24), QColor(255,255,255));
	}

	// Render the center line beam.
	if(!tris.isEmpty())
	{
		// Create a VAO of the trail triangles.
		AuGLVAO vao;
		vao.CreateForTexture(re, tris, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Render the VAO.
			re.GLSL().TextureShader().Select();
			re.GLSL().TextureShader().SetProjectionModelView(projection * model);
			re.GLSL().TextureShader().SetTexture(GetTextureID());
			re.GLSL().TextureShader().SetColor(1,1,1,.5f);

			vao.SelectGL();
			vao.DrawGL();
			vao.DeselectGL();
			vao.DeleteGL();

			re.GLSL().TextureShader().Deselect();
		}
	}

	// Render point sprites
	re.glEnable(GL_PROGRAM_POINT_SIZE); 

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(GetTextureID2());

	// Draw blue edge lights.
	{
		re.GLSL().SpriteSizeShader().SetColor(.15f, .15f, 1, 1);
		re.GLSL().SpriteSizeShader().SetSize(GetSize()*5 * scale);
		QList<AuPointAlpha> everts;
		AuPoint stl = emitPos - AuPoint(SpacecraftRecoveryWidth()/2, 0, 0);
		AuPoint str = emitPos + AuPoint(SpacecraftRecoveryWidth()/2, 0, 0);
		QList<AuPointAlpha> verts;
		int lights = SpacecraftRecoveryLength() / (10*FeetPerMeter());
		for(int e=0; e<=lights; e++)
		{
			double frac = (double)e/lights;
			AuPoint posl = stl - GetVelocity() * frac;
			AuPoint posr = str - GetVelocity() * frac;
			verts << AuPointAlpha(posl.x, posl.y, posl.z, 1);
			verts << AuPointAlpha(posr.x, posr.y, posr.z, 1);
		}
		AuGLVAO vao;
		vao.CreateForSprites(re, verts, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Render white moving sprites.
	{
		re.GLSL().SpriteSizeShader().SetColor(1, 1, 1, 1);
		re.GLSL().SpriteSizeShader().SetSize(GetSize()*20 * scale);
		AuParticleSystem::Render(projection, model, re, scale);
		re.glEnable(GL_PROGRAM_POINT_SIZE);		// Re enable after AuParticleSystem::Render shut it off.
	}

	// Draw glow effect at emitter and far end.
	{
		re.GLSL().SpriteSizeShader().SetColor(1, 1, 1, 1);
		re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 * scale);
		QList<AuPointAlpha> verts;
		for(int i=0; i<3; i++)
			verts << AuPointAlpha(emitPos.x, emitPos.y, emitPos.z, 1);
		verts << AuPointAlpha(endPos.x, endPos.y, endPos.z, 1);
		AuGLVAO vao;
		vao.CreateForSprites(re, verts, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_POINTS);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	re.glDisable(GL_PROGRAM_POINT_SIZE); 

	re.GLSL().SpriteSizeShader().Deselect();

	re.glDisable(GL_BLEND);
	re.glEnable(GL_CULL_FACE);
}
