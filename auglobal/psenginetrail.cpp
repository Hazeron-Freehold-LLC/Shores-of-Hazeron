//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psenginetrail.cpp,v 1.30 2022/02/19 18:33:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psenginetrail.h"
//
#include "augleffects.h"

PsEngineTrail::PsEngineTrail() : AuParticleSystem()
{
	m_SegScale = 1.f/900.f;
}

PsEngineTrail::~PsEngineTrail()
{

}

void PsEngineTrail::SetParent(const AuId& parentId, const AuMatrix4x4& parentT)
{
	if(m_idParent != parentId)
	{
		// Must recalc particle locations so they don't spaz out.
		AuMatrix4x4 t = parentT * m_tParent.Inverse();
		auParticle* pParticle = GetActiveList();
		while(pParticle)
		{
			pParticle->m_vCurPos = t.Project(pParticle->m_vCurPos);
			//pParticle->m_vCurPos = m_tParent.UnProject(pParticle->m_vCurPos);	// Convert to be relative to ship.
			//pParticle->m_vCurPos = parentT.Project(pParticle->m_vCurPos);		// Convert be relative to nu parent.
			pParticle = pParticle->m_pNext;
		}

		m_idParent = parentId;
	}
	m_tParent = parentT;
}

void PsEngineTrail::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vCurPos += GetVelocity() * GetElapsedTime();
}

void PsEngineTrail::InitParticle(auParticle* pParticle)
{
	pParticle->m_fInitTime = GetCurTime();
	pParticle->m_vCurPos = m_tParent.Project(GetPosition());	// Particle locs stored in space of parent.
	pParticle->m_vOldPos = pParticle->m_vCurPos;
}

void PsEngineTrail::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle	*pParticle = GetActiveList();
	int iRendered = 0;
	AuPoint emitPos = GetPosition();

	// Create a list of triangles to draw the engine trail.
	QList<AuTriTex> tris;
	AuPoint vPrev = emitPos;									// First particle is placed exactly at the emission point.
	AuPoint vPoint0, vPoint1;
	AuPoint vPrev0, vPrev1;
	AuPoint vNormal = model.UnProjectVector(AuPoint(0,0,-1));	// Give me a vector that will project into a 0,0,-1 normal.
	bool bSwitch = false;
	bool havePrev = false;
	while(pParticle)
	{
		auParticle *pNext = pParticle->m_pNext;
		if(pNext)
		{
			AuPoint vNext = m_tParent.UnProject(pNext->m_vCurPos);	// In the space of the host's parent atom, to prevent wigglies.
			double dDist2 = vPrev.Distance2(vNext);	// Squared distance to next vertex.
			if(dDist2 <= .5*.5)
			{
				pParticle = pParticle->m_pNext;
				continue;
			}
			iRendered++;

			// First Segment
			float fElapsed = 1-(GetCurTime() - pParticle->m_fInitTime)/GetLifeCycle();
			float fNewSize = GetSize() * m_SegScale * fElapsed/1 * scale;

			// Direction vector
			AuPoint vDir = vPrev - vNext;
			AuPoint vPoint = vNormal.Cross(vDir);
			vPoint.Unit();
			if(vPoint.y < 0)
				vPoint = -vPoint;

			vPoint0 = vPrev + (-vPoint)*fNewSize;
			vPoint1 = vPrev + vPoint*fNewSize;

			if(havePrev)
			{
				// Make triangles.
				if(bSwitch)
				{
					AuTriTex t0;
					t0.m_V0 = AuPointTex(vPrev0, 0,0);
					t0.m_V1 = AuPointTex(vPrev1, 0,1);
					t0.m_V2 = AuPointTex(vPoint1, 1,1);
					tris.append(t0);

					AuTriTex t1;
					t1.m_V0 = AuPointTex(vPoint1, 1,1);
					t1.m_V1 = AuPointTex(vPoint0, 1,0);
					t1.m_V2 = AuPointTex(vPrev0, 0,0);
					tris.append(t1);
				}
				else
				{
					AuTriTex t0;
					t0.m_V0 = AuPointTex(vPrev0, 1,0);
					t0.m_V1 = AuPointTex(vPrev1, 1,1);
					t0.m_V2 = AuPointTex(vPoint1, 0,1);
					tris.append(t0);

					AuTriTex t1;
					t1.m_V0 = AuPointTex(vPoint1, 0,1);
					t1.m_V1 = AuPointTex(vPoint0, 0,0);
					t1.m_V2 = AuPointTex(vPrev0, 1,0);
					tris.append(t1);
				}
				bSwitch = !bSwitch;
			}

			vPrev = vNext;
			vPrev0 = vPoint0;
			vPrev1 = vPoint1;
			havePrev = true;
		}
		pParticle = pParticle->m_pNext;
	}

	re.glDisable(GL_CULL_FACE);

	// Render the engine trail.
	if(!tris.isEmpty())
	{
		// Create a VAO of the engine trail triangles.
		AuGLVAO vao;
		vao.CreateForTexture(re, tris, GL_STREAM_DRAW);

		if(vao.VAO())
		{
			// Render the VAO.
			re.GLSL().TextureShader().Select();
			re.GLSL().TextureShader().SetProjectionModelView(projection * model * m_tParent);
			re.GLSL().TextureShader().SetTexture(GetTextureID());
			re.GLSL().TextureShader().SetColor(m_clrColor.redF(), m_clrColor.greenF(), m_clrColor.blueF(), 1);

			vao.SelectGL();
			vao.DrawGL();
			vao.DeselectGL();
			vao.DeleteGL();

			re.GLSL().TextureShader().Deselect();
		}
	}

	// Draw glow effect
	if(GetActiveList())
	{
		re.GLSL().SpriteSizeShader().Select();
		re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model * m_tParent);
		re.GLSL().SpriteSizeShader().SetTexture(GetTextureID2());
		re.GLSL().SpriteSizeShader().SetSize(GetSize() * scale);

		// Render point sprites
		re.glEnable(GL_PROGRAM_POINT_SIZE); 
		//re.glPointSize(GetSize() * scale);

		// Draw Engine effect
		iRendered = MAX(iRendered, (int)GetMaxParticles()/5);
		float fColorMod = (float)iRendered/(float)GetMaxParticles();
		fColorMod *= 2.0f;
		re.GLSL().SpriteSizeShader().SetColor(m_clrColor.redF()*fColorMod, m_clrColor.greenF()*fColorMod,
				m_clrColor.blueF()*fColorMod, 1);

		// Compile a list of vertex locations.
		QList<AuPointAlpha> verts;
		for(int i=0; i<3; i++)
			verts << AuPointAlpha(emitPos.x, emitPos.y, emitPos.z, 1);

		// Make a VAO of the vertex locations.
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

		re.glDisable(GL_PROGRAM_POINT_SIZE); 

		re.GLSL().SpriteSizeShader().Deselect();
	}

	re.glEnable(GL_CULL_FACE);
}
