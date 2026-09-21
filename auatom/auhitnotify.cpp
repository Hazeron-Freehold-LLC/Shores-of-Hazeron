//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhitnotify.cpp,v 1.33 2022/04/13 14:40:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auhitnotify.h"
//
#include "auatomrenderinfo.h"
#include "auglobal.h"
#include "daatom.h"

static AuGLVAO s_HitArmorVAO;
static AuGLVAO s_HitBodyVAO;
static AuGLVAO s_HitInternalVAO;

AuHitNotify::AuHitNotify()
{
	m_HitsAbsorbed1 = 0;
	m_HitsAbsorbed2 = 0;
	m_HitsDamage1 = 0;
	m_HitsDamage2 = 0;
	m_HitEffect1 = WE_None;
	m_HitEffect2 = WE_None;
	m_StopWatch.start();
}

AuHitNotify::AuHitNotify(const AuPoint& fromDir, WEAPONEFFECT effect1, qint32 hitsDamage1, qint32 hitsAbsorbed1,
		WEAPONEFFECT effect2, qint32 hitsDamage2, qint32 hitsAbsorbed2)
{
	m_HitDirection = fromDir;
	m_HitEffect1 = effect1;
	m_HitEffect2 = effect2;
	m_HitsDamage1 = hitsDamage1;
	m_HitsAbsorbed1 = hitsAbsorbed1;
	m_HitsDamage2 = hitsDamage2;
	m_HitsAbsorbed2 = hitsAbsorbed2;
	m_StopWatch.start();
}

AuHitNotify::AuHitNotify(const AuHitNotify& c)
{
	m_HitDirection = c.m_HitDirection;
	m_HitEffect1 = c.m_HitEffect1;
	m_HitEffect2 = c.m_HitEffect2;
	m_HitsAbsorbed1 = c.m_HitsAbsorbed1; 
	m_HitsAbsorbed2 = c.m_HitsAbsorbed2;
	m_HitsDamage1 = c.m_HitsDamage1;
	m_HitsDamage2 = c.m_HitsDamage2;
	m_StopWatch = c.m_StopWatch; 
}

const AuHitNotify& AuHitNotify::operator=(const AuHitNotify& c)
{
	m_HitDirection = c.m_HitDirection;
	m_HitEffect1 = c.m_HitEffect1;
	m_HitEffect2 = c.m_HitEffect2;
	m_HitsAbsorbed1 = c.m_HitsAbsorbed1; 
	m_HitsAbsorbed2 = c.m_HitsAbsorbed2;
	m_HitsDamage1 = c.m_HitsDamage1;
	m_HitsDamage2 = c.m_HitsDamage2;
	m_StopWatch = c.m_StopWatch; 

	return *this;
}
	
void AuHitNotify::operator<<(QDataStream& in)
{
	float x, y, z;
	in >> x;
	in >> y;
	in >> z;
	m_HitDirection = AuPoint(x,y,z);

	quint8 ui8;
	in >> ui8;
	m_HitEffect1 = (WEAPONEFFECT)ui8;
	if(m_HitEffect1 != WE_None)
	{
		in >> m_HitsDamage1;
		in >> m_HitsAbsorbed1;
	}

	in >> ui8;
	m_HitEffect2 = (WEAPONEFFECT)ui8;
	if(m_HitEffect2 != WE_None)
	{
		in >> m_HitsDamage2;
		in >> m_HitsAbsorbed2;
	}

	//RestartTime();
}

void AuHitNotify::operator>>(QDataStream& out) const
{
	out << (float)m_HitDirection.x;
	out << (float)m_HitDirection.y;
	out << (float)m_HitDirection.z;

	out << (quint8)m_HitEffect1;
	if(m_HitEffect1 != WE_None)
	{
		out << m_HitsDamage1;
		out << m_HitsAbsorbed1;
	}

	out << (quint8)m_HitEffect2;
	if(m_HitEffect2 != WE_None)
	{
		out << m_HitsDamage2;
		out << m_HitsAbsorbed2;
	}
}

const char* AuHitNotify::TextureFileEndoskeletal(WEAPONEFFECT effect) const
{
	const char* effectTex[] =
	{
		":data/HitMiss.png",			// WE_None
		":data/Acid.png",				// WE_Acid
		":data/Suffocate.png",			// WE_Suffocate
		":data/Suffocate.png",			// WE_Stun
		":data/Poison.png",				// WE_Poison
		":data/Radiation.png",			// WE_Radiation
		":data/Disease.png",			// WE_Disease
		":data/Lightning.png",			// WE_Electric
		":data/Blood.png",				// WE_Bludgeon
		":data/Freeze.png",				// WE_Cold
		":data/Blood.png",				// WE_Crush
		":data/Sonic.png",				// WE_Sonic
		":data/Heat.png",				// WE_Heat
		":data/Blood.png",				// WE_Pierce
		":data/Blood.png",				// WE_Implant
		":data/Blood.png",				// WE_Slash
		":data/Lightning.png",			// WE_Generic
		":data/Blood.png",				// WE_Unused
		":data/Blood.png",				// WE_Bleeding
		":data/Starving.png",			// WE_Starving
		":data/Acid.png",				// WE_Corrosion
		":data/stunsprite.png",			// WE_Audible
	};
	int effectTexCount = sizeof(effectTex) / sizeof(const char*);
	if(effect < effectTexCount)
		return effectTex[effect];
	return effectTex[WE_Bleeding];
}

const char* AuHitNotify::TextureFileExoskeletal(WEAPONEFFECT effect) const
{
	const char* effectTex[] =
	{
		":data/HitMiss.png",			// WE_None
		":data/Acid.png",				// WE_Acid
		":data/Suffocate.png",			// WE_Suffocate
		":data/Suffocate.png",			// WE_Stun
		":data/Poison.png",				// WE_Poison
		":data/Radiation.png",			// WE_Radiation
		":data/Disease.png",			// WE_Disease
		":data/Lightning.png",			// WE_Electric
		":data/BugGuts.png",			// WE_Bludgeon
		":data/Freeze.png",				// WE_Cold
		":data/BugGuts.png",			// WE_Crush
		":data/Sonic.png",				// WE_Sonic
		":data/Heat.png",				// WE_Heat
		":data/BugGuts.png",			// WE_Pierce
		":data/BugGuts.png",			// WE_Implant
		":data/BugGuts.png",			// WE_Slash
		":data/Lightning.png",			// WE_Generic
		":data/BugGuts.png",			// WE_Unused
		":data/BugGuts.png",			// WE_Bleeding
		":data/Starving.png",			// WE_Starving
		":data/Acid.png",				// WE_Corrosion
		":data/stunsprite.png",			// WE_Audible
	};
	int effectTexCount = sizeof(effectTex) / sizeof(const char*);
	if(effect < effectTexCount)
		return effectTex[effect];
	return effectTex[WE_Bleeding];
}

// This function is for setting the radius of the sphere on screen for different shells of notifications 
float AuHitNotify::DomeRadius(HITTYPE type) const
{
	switch(type)
	{
	case HIT_Body:
		return 0.6f;
	case HIT_BodyArmor:
		return 0.69f;
	case HIT_Internal:
		return 0;
	}
	return 10;
}

float AuHitNotify::ZRotation(WEAPONEFFECT effect) const
{
	switch(effect)
	{
	case WE_None:
	case WE_Unused:
	case WE_Audible:
	case WE_Invalid:
	case WE_Generic:
	case WE_Starving:
	case WE_Bleeding:
	case WE_Stun:	
	case WE_Corrosion:
	case WE_Crush:
	case WE_Implant:
	case WE_Slash:
	case WE_Bludgeon:
	case WE_Pierce:
	case WE_Acid:
	case WE_Poison:	
	case WE_Cold:
	case WE_Heat:
		break;

	case WE_Disease: 
	case WE_Suffocate:
		return (float)m_StopWatch.elapsed() / 2000.f * PIf/3;

	case WE_Sonic:
		// Flips it every frame so it looks like it is moving.
		return (m_StopWatch.elapsed() % 2) * PIf;

	case WE_Radiation: 
		return g_Au.Dice().Roll(1,360) * DEG2RADf;

	case WE_Electric:
		return g_Au.Dice().Roll(1,11,-6) * DEG2RADf;
	}
	return 0;
}

float AuHitNotify::SplatSize(HITTYPE type, int which) const
{
	float size;
	switch(type)
	{
	case HIT_Internal:
		break;

	case HIT_Body:
		if(which == 1)
			size = (m_HitsDamage1/50.f) * .75f + 0.25f;
		else
			size = (m_HitsDamage2/50.f) * .75f + 0.25f;
		if(size > 1)
			return 1;
		return size;

	case HIT_BodyArmor:
		if(which == 1)
			size = (m_HitsAbsorbed1/50.f) * .75f + 0.25f;
		else if(which == 2)
			size = (m_HitsAbsorbed2/50.f) * .75f + 0.25f;
		else
			size = ((m_HitsAbsorbed1 + m_HitsAbsorbed2)/50.f) * .75f + 0.25f;
		if(size > 1)
			return 1;
		return size;
	}
	return 0;
}

void AuHitNotify::DrawHit(AuAtomRenderInfo& ari, float tilt, bool isExoskeletal)
{
	// Insure we have GL model display lists.
	if(!s_HitArmorVAO.VAO())
		s_HitArmorVAO = CreateModel(*ari.re, HIT_BodyArmor);

	if(!s_HitBodyVAO.VAO())
		s_HitBodyVAO = CreateModel(*ari.re, HIT_Body);

	if(!s_HitInternalVAO.VAO())
		s_HitInternalVAO = CreateModel(*ari.re, HIT_Internal);

	// Configure rendering parameters.
	ari.re->glDepthMask(false);				// Do not modify the Z buffer.
	ari.re->glDisable(GL_DEPTH_TEST);		// Do not care about what is already in the Z buffer.
	ari.re->glEnable(GL_BLEND);

	//Set up rendering functions.
	float alpha = Alpha();										// Alpha decays over time. Applies to all shells of this hit effect.

	if(HasDirection())
	{
		// Set up model view matrix to orient the hit effect at the center of the screen.
		double aspect = AuGLEffects::WindowAspectRatio(ari.glw->rect());
		double fovy = AuGLEffects::WindowFieldOfViewY(aspect, g_Au.FieldOfViewY());
		AuMatrix4x4 projection;
		projection.Perspective(fovy, aspect, 1, 10000);

		AuMatrix4x4 transform;
		transform.Translate(AuPoint(0,0,-2.1));					// Move the effect away from the perspective near clip plane. 
		transform.RotateX(-PI/2-tilt);							// Tilt -90 degrees and account for head tilt.
		transform *= AuPoint(0,0,1).GetAlignmentTransform(m_HitDirection);	// Align 'up' of dome with hit direction vector.
		//qDebug() << "AuHitNotify::DrawHit hitDir" << m_HitDirection << m_HitsAbsorbed1 << m_HitsAbsorbed2;

		ari.re->GLSL().SplatShader().Select();

		// Render hit shells from back to front.
		float shieldAlpha = alpha / 3;							// Make shield fade faster than body hits.

		// Configure for rendering insides of shells.
		ari.re->glFrontFace(GL_CCW);

		// Render armor absorb shell inside, which is the farthest away.
		if(m_HitsAbsorbed1 || m_HitsAbsorbed2)
		{
			RenderShield(projection, transform, *ari.re, SplatSize(HIT_BodyArmor, 3), .5, shieldAlpha);
			if(m_HitsAbsorbed1)
				RenderShell(projection, transform, *ari.re, HIT_BodyArmor, m_HitEffect1, m_HitsAbsorbed1,
						SplatSize(HIT_BodyArmor, 1), .5f, shieldAlpha, isExoskeletal);
			if(m_HitsAbsorbed2)
				RenderShell(projection, transform, *ari.re, HIT_BodyArmor, m_HitEffect2, m_HitsAbsorbed2,
						SplatSize(HIT_BodyArmor, 2), .5f, shieldAlpha, isExoskeletal);
		}

		// Render the body hit shell insides, which are the next closest.
		if(m_HitEffect1 != WE_None)
			RenderShell(projection, transform, *ari.re, HIT_Body, m_HitEffect1, m_HitsDamage1, SplatSize(HIT_Body, 1), .5f, alpha,
					isExoskeletal);
		if(m_HitEffect2 != WE_None)
			RenderShell(projection, transform, *ari.re, HIT_Body, m_HitEffect2, m_HitsDamage2, SplatSize(HIT_Body, 2), .5f, alpha,
					isExoskeletal);

		// Configure for rendering outside of shells.
		ari.re->glFrontFace(GL_CW);

		// Render the body hit shell outsides, which are the next closest.
		if(m_HitEffect1 != WE_None)
			RenderShell(projection, transform, *ari.re, HIT_Body, m_HitEffect1, m_HitsDamage1, SplatSize(HIT_Body, 1), 1, alpha,
					isExoskeletal);
		if(m_HitEffect2 != WE_None)
			RenderShell(projection, transform, *ari.re, HIT_Body, m_HitEffect2, m_HitsDamage2, SplatSize(HIT_Body, 2), 1, alpha,
					isExoskeletal);

		// Render armor absorb shell outside, which is the closet.
		if(m_HitsAbsorbed1 || m_HitsAbsorbed2)
		{
			RenderShield(projection, transform, *ari.re, SplatSize(HIT_BodyArmor, 3), 1, shieldAlpha);
			if(m_HitsAbsorbed1)
				RenderShell(projection, transform, *ari.re, HIT_BodyArmor, m_HitEffect1, m_HitsAbsorbed1,
						SplatSize(HIT_BodyArmor, 1), 1, shieldAlpha, isExoskeletal);
			if(m_HitsAbsorbed2)
				RenderShell(projection, transform, *ari.re, HIT_BodyArmor, m_HitEffect2, m_HitsAbsorbed2,
						SplatSize(HIT_BodyArmor, 2), 1, shieldAlpha, isExoskeletal);
		}

		ari.re->GLSL().SplatShader().Deselect();
	}
	else if(HasHitPoints())
	{
		// No Direction but has hit points...Internal Damage
		// Texture shader
		// Configure projection to draw to unit screen.
		ari.re->GLSL().TextureShader().Select();
		ari.re->GLSL().TextureShader().SetProjectionModelView(AuMatrix4x4());

		// Texture parameters
		QString sid = QString("HE%1%2").arg(m_HitEffect1).arg(isExoskeletal);
		GLuint glTexture;
		if(isExoskeletal)
			glTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileExoskeletal(m_HitEffect1));
		else
			glTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileEndoskeletal(m_HitEffect1));
		ari.re->GLSL().TextureShader().SetTexture(glTexture);
		ari.re->GLSL().TextureShader().SetColor(1,1,1, alpha);

		// Rendering parameters.
		SetBlendFunc(*ari.re, m_HitEffect1);

		// Draw
		s_HitInternalVAO.SelectGL();
		s_HitInternalVAO.DrawGL();
		s_HitInternalVAO.DeselectGL();

		//Clean up
		ari.re->GLSL().TextureShader().Deselect();
	}

	// Restore preferred default rendering parameters.
	ari.re->glDisable(GL_BLEND);
	ari.re->glEnable(GL_DEPTH_TEST);
	ari.re->glDepthMask(true);

	// Clear this effect when its time has expired.
	Decay();
}

void AuHitNotify::RenderShell(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, HITTYPE hitType,
		WEAPONEFFECT effect, qint32 hits, float splatSize, float intensity, float alpha, bool isExoskeletal) const
{
	// Texture parameters
	GLuint glTexture;
	float zrot;
	if(hits)
	{
		// Use texture of weapon effect.
		QString sid = QString("HE%1%2").arg(effect).arg(isExoskeletal);
		if(isExoskeletal)
			glTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileExoskeletal(effect));
		else
			glTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileEndoskeletal(effect));
		zrot = ZRotation(effect);
	}
	else
	{
		// Ping texture only, not the actual effect.
		QString sid = QString("HE%1%2").arg(WE_None).arg(isExoskeletal);
		if(isExoskeletal)
			glTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileExoskeletal(WE_None));
		else
			glTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, TextureFileEndoskeletal(WE_None));
		zrot = 0;
	}
	re.GLSL().SplatShader().SetTexture(glTexture);

	// Model projection.
	if(zrot != 0)
	{
		AuMatrix4x4 rot(model);
		rot.RotateZ(zrot);
		re.GLSL().SplatShader().SetProjectionModelView(projection * rot);
	}
	else
		re.GLSL().SplatShader().SetProjectionModelView(projection * model);

	// Rendering parameters.
	re.GLSL().SplatShader().SetSplatScale(splatSize);	// .5 makes the splat texture appear to be half as big as 1.
	re.GLSL().SplatShader().SetColor(intensity, intensity, intensity, alpha);
	SetBlendFunc(re, effect);

	// Draw dome model
	if(hitType == HIT_Body)
	{
		s_HitBodyVAO.SelectGL();
		s_HitBodyVAO.DrawGL();
		s_HitBodyVAO.DeselectGL();
	}
	else
	{
		s_HitArmorVAO.SelectGL();
		s_HitArmorVAO.DrawGL();
		s_HitArmorVAO.DeselectGL();
	}
}

void AuHitNotify::RenderShield(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float splatSize,
		float intensity, float alpha) const
{
	// Model projection.
	re.GLSL().SplatShader().SetProjectionModelView(projection * model);

	// Texture parameters
	GLuint glTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "HitShield", ":data/Shield.png");
	re.GLSL().SplatShader().SetTexture(glTexture);

	// Rendering parameters.
	re.GLSL().SplatShader().SetSplatScale(splatSize);	// .5 makes the splat texture appear to be half as big as 1.
	re.GLSL().SplatShader().SetColor(intensity, intensity, intensity, alpha);
	SetBlendFunc(re, WE_None);

	// Draw dome model
	s_HitArmorVAO.SelectGL();
	s_HitArmorVAO.DrawGL();
	s_HitArmorVAO.DeselectGL();
}

AuGLVAO AuHitNotify::CreateModel(AuGLEffects& re, AuHitNotify::HITTYPE type) const
{
	AuGLVAO vao;
	switch(type)
	{
	case HIT_Internal:
		{
			// Top
			AuTriTex trit0;
			trit0.m_V0 = AuPointTex(-1, 1, 0, 0, 0.5f);
			trit0.m_V1 = AuPointTex(1, 1, 0, 1, 0.5f);
			trit0.m_V2 = AuPointTex(0.8f, 0.8f, 0, 1, 0);
			AuTriTex trit1;
			trit1.m_V0 = AuPointTex(0.8f, 0.8f, 0, 1, 0);
			trit1.m_V1 = AuPointTex(-0.8f, 0.8f, 0, 0, 0);
			trit1.m_V2 = AuPointTex(-1, 1, 0, 0, 0.5f);

			// Right
			AuTriTex trir0;
			trir0.m_V0 = AuPointTex(1, 1, 0, 0.5f, 1);
			trir0.m_V1 = AuPointTex(1, -1, 0, 0.5f, 0);
			trir0.m_V2 = AuPointTex(0.8f, -0.8f, 0, 0, 0);
			AuTriTex trir1;
			trir1.m_V0 = AuPointTex(0.8f, -0.8f, 0, 0, 0);
			trir1.m_V1 = AuPointTex(0.8f, 0.8f, 0, 0, 1);
			trir1.m_V2 = AuPointTex(1, 1, 0, 0.5f, 1);

			// Bottom
			AuTriTex trib0;
			trib0.m_V0 = AuPointTex(1, -1, 0, 1, 0.5f);
			trib0.m_V1 = AuPointTex(-1, -1, 0, 0, 0.5f);
			trib0.m_V2 = AuPointTex(-0.8f, -0.8f,  0, 0, 1);
			AuTriTex trib1;
			trib1.m_V0 = AuPointTex(-0.8f, -0.8f,  0, 0, 1);
			trib1.m_V1 = AuPointTex(0.8f, -0.8f, 0, 1, 1);
			trib1.m_V2 = AuPointTex(1, -1, 0, 1, 0.5f);

			// Left
			AuTriTex tril0;
			tril0.m_V0 = AuPointTex(-1, -1, 0, 0.5f, 1);
			tril0.m_V1 = AuPointTex(-1, 1, 0, 0.5f, 0);
			tril0.m_V2 = AuPointTex(-0.8f, 0.8f, 0, 1, 0);
			AuTriTex tril1;
			tril1.m_V0 = AuPointTex(-0.8f, 0.8f, 0, 1, 0);
			tril1.m_V1 = AuPointTex(-0.8f, -0.8f, 0, 1, 1);
			tril1.m_V2 = AuPointTex(-1, -1, 0, 0.5f, 1);

			QList<AuTriTex> tris;
			tris << trit0 << trit1 << trir0 << trir1 << trib0 << trib1 << tril0 << tril1;

			vao.CreateForTexture(re, tris, GL_STATIC_DRAW);
		}
		break;

	case HIT_Body:
	case HIT_BodyArmor:
		{
			const int londivs = 20;
			const int latdivs = 10;
			
			float radius = DomeRadius(type);
				
			float latdiv = (PIf/latdivs);		
			float londiv = (PIf*2.0f/londivs);
			float texScale = 1.f / cosf(PIf/2 - latdiv * 3);		// Range is 0 to 1 for fsplat shader.

			QList<AuTriTex> tris;
			for(int lat=0; lat < 3; lat++)
			{
				float latang = (PIf/2.0f - latdiv * lat);
				float latang2 = (PIf/2.0f - latdiv * (lat+1));
				float latcos = cosf(latang);
				float latcos2 = cosf(latang2);
				float latsin = sinf(latang);
				float latsin2 = sinf(latang2);
	
				for(int lon=0; lon<londivs; lon++)
				{
					float rangle = lon*londiv;
					float rx = cosf(rangle);
					float ry = sinf(rangle);

					float rnext = (lon+1)*londiv;
					float rx2 = cosf(rnext);
					float ry2 = sinf(rnext);
					
					//Set up Points
					AuPoint v1(rx*latcos, ry*latcos, latsin);		// UL corner of quad
					v1 *= radius;
					float t1x = texScale*rx*latcos;
					float t1y = texScale*ry*latcos;

					AuPoint v2(rx2*latcos, ry2*latcos, latsin);		// UR corner of quad
					v2 *= radius;
					float t2x = texScale*rx2*latcos;
					float t2y = texScale*ry2*latcos;

					AuPoint v3(rx2*latcos2, ry2*latcos2, latsin2);	// LR corner of quad
					v3 *= radius;
					float t3x = texScale*rx2*latcos2;
					float t3y = texScale*ry2*latcos2;

					AuPoint v4(rx*latcos2, ry*latcos2, latsin2);	// LL corner of quad
					v4 *= radius;
					float t4x = texScale*rx*latcos2;
					float t4y = texScale*ry*latcos2;

					AuTriTex tri0;
					tri0.m_V0 = AuPointTex(v4, t4x, t4y);
					tri0.m_V1 = AuPointTex(v1, t1x, t1y);
					tri0.m_V2 = AuPointTex(v2, t2x, t2y);

					AuTriTex tri1;
					tri1.m_V0 = AuPointTex(v2, t2x, t2y);
					tri1.m_V1 = AuPointTex(v3, t3x, t3y);
					tri1.m_V2 = AuPointTex(v4, t4x, t4y);

					tris << tri0 << tri1;
				}
			}
			vao.CreateForTexture(re, tris, GL_STATIC_DRAW);
		}
		break;
	}
	return vao;
}

float AuHitNotify::Alpha() const
{
	float alpha = 1.f - (float)m_StopWatch.elapsed() / 2000.f;
	if(alpha > .8f)
		return .8f;
	return alpha;
}

void AuHitNotify::Decay()
{
	// Checks to see if the time of the effect is over.
	if(IsReadyToDecay())
		ClearHit();
}

bool AuHitNotify::HasDirection() const
{
	return m_HitDirection != AuPoint(0,0);
}

bool AuHitNotify::HasHitPoints() const
{
	return (m_HitsDamage1 > 0 || m_HitsAbsorbed1 > 0);
}

void AuHitNotify::SetBlendFunc(AuGLEffects& re, WEAPONEFFECT effect) const
{
	switch(effect)
	{
	case WE_Generic:
		break;
	case WE_None:
	case WE_Unused:
	case WE_Invalid:
	case WE_Acid:
	case WE_Bleeding:
	case WE_Bludgeon:
	case WE_Cold:
	case WE_Corrosion:
	case WE_Crush:
	case WE_Disease: 
	case WE_Implant:
	case WE_Pierce:
	case WE_Poison:	
	case WE_Starving:
	case WE_Stun:	
	case WE_Suffocate:
	case WE_Slash:
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Combination blend.
		break;

	case WE_Audible:
	case WE_Electric:
	case WE_Heat:
	case WE_Radiation: 
	case WE_Sonic:
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend.
		break;
	}
}

void AuHitNotify::RotateZ(float angle)
{
	m_HitDirection = m_HitDirection.RotateZ(angle);
}
