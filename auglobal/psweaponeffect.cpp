//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psweaponeffect.cpp,v 1.27 2022/02/19 18:34:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psweaponeffect.h"
//
#include "augleffects.h"

PsWeaponEffect::PsWeaponEffect() : AuParticleSystem()
{
	m_SegScale = 1.f/100.f;
	m_Switch = false;
	m_Order = -1;
}

PsWeaponEffect::~PsWeaponEffect()
{

}

void PsWeaponEffect::SetParent(const AuId& parentId, const AuMatrix4x4& parentT)
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

void PsWeaponEffect::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vCurPos += (GetVelocity()) * 0.05;// GetElapsedTime();
}

void PsWeaponEffect::InitParticle(auParticle* pParticle)
{
	//pParticle->m_vCurVel = GetVelocity();// * 10.;
	pParticle->m_fInitTime = GetCurTime();
	pParticle->m_vCurPos = m_tParent.Project(GetPosition());	// Particle locs stored in space of parent.
	pParticle->m_vOldPos = pParticle->m_vCurPos;

	if(m_Order == 3)
	{
		m_Switch = !m_Switch;
		m_Order = 0;
	}
	else
		m_Order++;
	pParticle->m_Order = m_Order;
	pParticle->m_Switch = m_Switch;
}

void PsWeaponEffect::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale)
{
	// Compute billboard vertices's on the GPU using ARB_point_sprites
	auParticle	*pParticle = GetActiveList();

	// Create a list of triangles to draw the trail.
	AuPoint emitPos = m_tParent.Project(GetPosition());
	QList<AuTriTex> tris;
	if(scale == 1.)
	{
		AuPoint vPrev = emitPos;									// First particle is placed exactly at the emission point.
		AuPoint vPoint0, vPoint1;
		AuPoint vPrev0, vPrev1;
		AuPoint vNormal = model.UnProjectVector(AuPoint(0,0,-1));	// Give me a vector that will project into a 0,0,-1 normal.
		vNormal.Unit();
		bool bDone = false;
		bool havePrev = false;
		const float himaxx = 128;	// width of the texture
		const float himaxy = 64;	// height of the texture
		float texCoord0=0.0;	//, texCoord1=0.0;
		float texCoord0Prev=0.0;	//, texCoord1=0.0;
		while(pParticle)
		{
			auParticle *pNext = pParticle->m_pNext;
			if(pNext)
			{
				if(vPrev.Distance(GetPosition()) > m_Length)
				{
					vPrev = m_TargetLoc;
					bDone = true;
				}

				AuPoint vNext(pNext->m_vCurPos);			// In the space of the host's parent atom, to prevent wigglies.
				double dDist2 = vPrev.Distance2(vNext);	// Squared distance to next vertex.
				if(dDist2 <= .5*.5)
				{
					pParticle = pParticle->m_pNext;
					continue;
				}

				// First Segment
				float fElapsed = 1-(GetCurTime() - pParticle->m_fInitTime)/GetLifeCycle();
				float fNewSize = GetSize() * m_SegScale * fElapsed * scale;

				// Direction vector
				AuPoint vDir = vPrev - vNext;
				vDir.Unit();
				AuPoint vPoint = vNormal.Cross(vDir);
				vPoint.Unit();
				if(vPoint.y < 0)
					vPoint = -vPoint;

				vPoint *= fNewSize;
				vPoint0 = vPrev - vPoint;
				vPoint1 = vPrev + vPoint;

				// Set up cycling texture coordinates
				// these are all the X-Coordinates
				switch(pParticle->m_Order)
				{
				case 0:
					texCoord0 = pParticle->m_Switch ? 127.5f/himaxx : 0.5f/himaxx;
					//texCoord1 = pParticle->m_Switch ? 0.75f : 0.25f;
					break;
				case 1:
					texCoord0 = pParticle->m_Switch ? 0.75f : 0.25f;
					//texCoord1 = pParticle->m_Switch ? 0.5f : 0.5f;
					break;
				case 2:
					texCoord0 = pParticle->m_Switch ? 0.5f : 0.5f;
					//texCoord1 = pParticle->m_Switch ? 0.25f : 0.75f;
					break;
				case 3:
					texCoord0 = pParticle->m_Switch ? 0.25f : 0.75f;
					//texCoord1 = pParticle->m_Switch ? 0.5f/himaxx : 127.5f/himaxx;
					break;
				default:
					Q_ASSERT(false);
					break;
				}

				if(bDone)
					break;

				if(havePrev)
				{
					// Make triangles.
					float tt0 = 0.5f/himaxy;
					float tt1 = 63.5f/himaxy;

					AuTriTex t0;
					t0.m_V0 = AuPointTex(vPrev0, texCoord0Prev,tt0);
					t0.m_V1 = AuPointTex(vPrev1, texCoord0Prev,tt1);
					t0.m_V2 = AuPointTex(vPoint1, texCoord0,tt1);
					tris.append(t0);

					AuTriTex t1;
					t1.m_V0 = AuPointTex(vPoint1, texCoord0,tt1);
					t1.m_V1 = AuPointTex(vPoint0, texCoord0,tt0);
					t1.m_V2 = AuPointTex(vPrev0, texCoord0Prev,tt0);
					tris.append(t1);
				}

				vPrev = vNext;
				vPrev0 = vPoint0;
				vPrev1 = vPoint1;
				texCoord0Prev = texCoord0;
				havePrev = true;
			}
			pParticle = pParticle->m_pNext;
		}
	}

	re.glDisable(GL_CULL_FACE);
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Render the trail.
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
			re.GLSL().TextureShader().SetColor(m_clrColor.redF(), m_clrColor.greenF(), m_clrColor.blueF(), 1);

			vao.SelectGL();
			vao.DrawGL();
			vao.DeselectGL();
			vao.DeleteGL();

			re.GLSL().TextureShader().Deselect();
		}
	}

	// Draw glow effect
	pParticle = GetActiveList();
	if(pParticle)
	{
		re.GLSL().SpriteSizeShader().Select();
		re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
		re.GLSL().SpriteSizeShader().SetTexture(GetTextureID2());
		re.GLSL().SpriteSizeShader().SetColor(.35f, .35f, 1, 1);
		re.GLSL().SpriteSizeShader().SetSize(GetSize()*10 * scale);

		// Render point sprites
		re.glEnable(GL_PROGRAM_POINT_SIZE); 
		//re.glPointSize(GetSize() * scale);

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

	re.glDisable(GL_BLEND);
	re.glEnable(GL_CULL_FACE);
}
