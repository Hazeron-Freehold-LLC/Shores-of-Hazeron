//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aueffect.cpp,v 1.90 2022/02/19 17:39:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aueffect.h"

#include "auatomrenderinfo.h"
#include "audef.h"
#include "auglobal.h"
#include "augleffects.h"
#include "aunoise.h"
#include "ausound.h"
#include "psexplodeacid.h"
#include "psexplodebludgeon.h"
#include "psexplodecold.h"
#include "psexplodecrush.h"
#include "psexplodedisease.h"
#include "psexplodeelectric.h"
#include "psexplodeheat.h"
#include "psexplodepierce.h"
#include "psexplodepoison.h"
#include "psexploderadiation.h"
#include "psexplodeslash.h"
#include "psexplodesonic.h"
#include "psexplodestun.h"
#include "psexplodesuffocate.h"
#include "psweaponeffect.h"

#define c_MarkWithTurrets 0				// Set to 1 to cause turret hits to remain in the scene indefinitely.

AuEffect::~AuEffect()
{
	if(m_ParticleEffect)
	{
		delete m_ParticleEffect;
		m_ParticleEffect = 0;
	}
	if(m_ParticleEffect2)
	{
		delete m_ParticleEffect2;
		m_ParticleEffect2 = 0;
	}
}

void AuEffect::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(!m_OriginLoc.IsZero())
		hasWhat |= 0x01;
	if(m_Sound != 0/*SOUND_None*/)
		hasWhat |= 0x02;
	if(m_Visual != VISUAL_None)
		hasWhat |= 0x04;
	if(!m_Rotation.IsZero())
		hasWhat |= 0x08;
	if(!m_TargetLoc.IsZero())
		hasWhat |= 0x10;
	if(!m_Vector.IsZero())
		hasWhat |= 0x20;
	if(m_Effect1 != WE_None)
		hasWhat |= 0x40;
	if(m_Effect2 != WE_None)
		hasWhat |= 0x80;

	out << hasWhat;

	// Effects don't live long enough to need the entire 8 bytes of the time value. The bottom 16 bits give us a maximum lifespan
	// of 65,535ms. Most effects last a few seconds at most.
	m_Start.Tx16(out);

	if(hasWhat & 0x01)
	{
		out << (float)m_OriginLoc.x;
		out << (float)m_OriginLoc.y;
		out << (float)m_OriginLoc.z;
	}

	if(hasWhat & 0x02)
	{
		out << (quint8)m_Sound;
		out << m_Gain;
		out << m_Pitch;
	}

	if(hasWhat & 0x04)
	{
		quint8 vhit = m_Visual;
		if(m_ShieldHit)
			vhit |= 0x80;
		out << vhit;
	}

	if(hasWhat & 0x08)
	{
		out << (float)m_Rotation.x;
		out << (float)m_Rotation.y;
		out << (float)m_Rotation.z;
	}

	if(hasWhat & 0x10)
	{
		out << (float)m_TargetLoc.x;
		out << (float)m_TargetLoc.y;
		out << (float)m_TargetLoc.z;
	}

	if(hasWhat & 0x20)
	{
		out << (float)m_Vector.x;
		out << (float)m_Vector.y;
		out << (float)m_Vector.z;
	}

	if(hasWhat & 0x40)
	{
		out << m_Effect1;
		out << m_Radius1;
	}

	if(hasWhat & 0x80)
	{
		out << m_Effect2;
		out << m_Radius2;
	}

	// m_SecOff not sent.
	// m_IntersectLoc calculated once on the client.
}

void AuEffect::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	m_Start.Rx16(in, g_Au.ServerTimeOffset());
	//qDebug() << "AuEffect::operator<< start" << m_Start << "now" << AuTime::Now();

	float x,y,z;
	if(hasWhat & 0x01)
	{
		in >> x;
		in >> y;
		in >> z;
		m_OriginLoc = AuPoint(x,y,z);
	}

	quint8 ui8;
	if(hasWhat & 0x02)
	{
		in >> ui8;
		m_Sound = ui8;
		in >> m_Gain;
		in >> m_Pitch;
	}

	if(hasWhat & 0x04)
	{
		in >> ui8;
		m_Visual = (VISUAL_EFFECT)(ui8 & 0x7f);
		m_ShieldHit = ui8 & 0x80;
	}

	if(hasWhat & 0x08)
	{
		in >> x;
		in >> y;
		in >> z;
		m_Rotation = AuPoint(x,y,z);
	}

	if(hasWhat & 0x10)
	{
		in >> x;
		in >> y;
		in >> z;
		m_TargetLoc = AuPoint(x,y,z);
	}

	if(hasWhat & 0x20)
	{
		in >> x;
		in >> y;
		in >> z;
		m_Vector = AuPoint(x,y,z);
	}

	if(hasWhat & 0x40)
	{
		in >> m_Effect1;
		in >> m_Radius1;
	}
	else
	{
		m_Effect1 = WE_None;
		m_Radius1 = 0;
	}

	if(hasWhat & 0x80)
	{
		in >> m_Effect2;
		in >> m_Radius2;
	}
	else
	{
		m_Effect2 = WE_None;
		m_Radius2 = 0;
	}

	// m_SecOff not sent.
	// m_IntersectLoc calculated once on the client.
}

bool AuEffect::Animate(AuAtomRenderInfo& ari)
{
	// Do the same tests the server does when aging effects.
	if(!Tick())
		return false;

	if(!m_Model)
	{
		switch(m_Visual)
		{
		case VISUAL_None:
			break;

		case VISUAL_HitAcid:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Acid, ExplosionRadius1());
			break;
		case VISUAL_HitCold:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Cold, ExplosionRadius1());
			break;
		case VISUAL_HitElectric:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Electric, ExplosionRadius1());
			break;
		case VISUAL_HitFire:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Heat, ExplosionRadius1());
			break;
		case VISUAL_HitRadiation:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Radiation, ExplosionRadius1());
			break;
		case VISUAL_HitDisease:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Disease, ExplosionRadius1());
			break;
		case VISUAL_HitGasping:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Suffocate, ExplosionRadius1());
			break;
		case VISUAL_HitParalysis:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Stun, ExplosionRadius1());
			break;
		case VISUAL_HitPoison:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Poison, ExplosionRadius1());
			break;
		case VISUAL_HitSonic:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Sonic, ExplosionRadius1());
			break;

		case VISUAL_HitBludgeon:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Bludgeon, ExplosionRadius1());
			break;

		case VISUAL_HitPierce:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Pierce, ExplosionRadius1());
			break;

		case VISUAL_HitSlash:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Slash, ExplosionRadius1());
			break;

		case VISUAL_ExplodeSpacecraft:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Heat, ExplosionRadius1());
			if(!m_ParticleEffect2)
				m_ParticleEffect2 = CreateExplosion(*ari.re, WE_Radiation, ExplosionRadius2());
			break;

		case VISUAL_ExplodeVehicle:
			if(!m_ParticleEffect)
				m_ParticleEffect = CreateExplosion(*ari.re, WE_Heat, ExplosionRadius1());
			if(!m_ParticleEffect2)
				m_ParticleEffect2 = CreateExplosion(*ari.re, WE_Bludgeon, ExplosionRadius2());
			break;

		case VISUAL_Explosion:
			if(!m_ParticleEffect && ExplosionEffect1() != WE_None)
				m_ParticleEffect = CreateExplosion(*ari.re, ExplosionEffect1(), ExplosionRadius1());
			if(!m_ParticleEffect2 && ExplosionEffect2() != WE_None)
				m_ParticleEffect2 = CreateExplosion(*ari.re, ExplosionEffect2(), ExplosionRadius2());
			break;

		case VISUAL_HitBludgeonBlood:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mBlood.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBlood", ":data/Blood.png");
			break;
		case VISUAL_HitBludgeonBug:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mBlood.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBugGuts", ":data/BugGuts.png");
			break;

		case VISUAL_HitPierceBlood:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mHitPierce.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBlood", ":data/Blood.png");
			break;
		case VISUAL_HitPierceBug:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mHitPierce.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBugGuts", ":data/BugGuts.png");
			break;
		case VISUAL_HitSeed:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mHitSeed.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBloodSeed", ":data/BloodSeed.png");
			break;
		case VISUAL_HitSlashBlood:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mHitSlash.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBlood", ":data/Blood.png");
			break;
		case VISUAL_HitSlashBug:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mHitSlash.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VBugGuts", ":data/BugGuts.png");
			break;

		case VISUAL_FireEnergyBeam:
			if(!m_ParticleEffect)
			{
				// Select texture based on m_Effect1
				QString energyBeamTextures[WE_Invalid] =
				{
					"",											// WE_None
					"",											// WE_Acid
					"",											// WE_Suffocate
					":data/BeamStun.png",						// WE_Stun
					"",											// WE_Poison
					":data/BeamRadiation.png",					// WE_Radiation
					"",											// WE_Disease
					":data/BeamElectric.png",					// WE_Electric
					":data/BeamBludgeon.png",					// WE_Bludgeon
					":data/BeamCold.png",						// WE_Cold
					":data/BeamCrush.png",						// WE_Crush
					":data/BeamSonic.png",						// WE_Sonic
					":data/BeamHeat.png",						// WE_Heat
					":data/BeamPierce.png",						// WE_Pierce
					"",											// WE_Implant
					":data/BeamSlash.png",						// WE_Slash
				};
				QString texname = QString("VFire%1").arg(m_Effect1);
				GLuint texId = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, texname, energyBeamTextures[m_Effect1]);
				GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

				// Create the engine trail emitter.
				PsWeaponEffect* weaponEffect = new PsWeaponEffect;
				weaponEffect->SetMaxParticles(100);
				weaponEffect->SetLifeCycle(1.0f);
				//weaponEffect->SetVelocityVar(0.0f);
				weaponEffect->SetNumToRelease(1);
				weaponEffect->SetReleaseInterval(0.0f);
				if(m_Effect1 == WE_Sonic)
					weaponEffect->SetSegmentScale(1.f/50.f);		// 1/100 is the default seg scale. Sonic better wider.
				weaponEffect->SetSize(sqrtf(Gain()) * 100.0);
				weaponEffect->SetTextureID(texId);
				weaponEffect->SetTextureID2(texIdGlow);
				weaponEffect->SetVelocity((m_TargetLoc-m_OriginLoc) * 2.0);
				weaponEffect->SetLength(m_TargetLoc.Distance(m_OriginLoc));	// my own length so I don't go past the target
				weaponEffect->SetTargetLoc(m_TargetLoc);
				m_ParticleEffect = weaponEffect;
			}
			break;

		case VISUAL_FireCannon:
		case VISUAL_FireMachineGun:
		case VISUAL_FirePistol:
		case VISUAL_FireRifle:
		case VISUAL_FireShotgun:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mFirePistol.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VFirePistol", ":data/MuzzleFlash.png");
			break;

		case VISUAL_KnifeSwish:
			// Create the 3ds model of the effect.
			m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.CreateModelGLSL(":data/mKnifeSwish.3dsX", true, c_UnitScale));

			// Load an optional texture to override the one that comes with
			// the 3ds model. Otherwise, set m_Texture to 0.
			m_Texture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VKnifeSwish", ":data/KnifeSwish.png");
			break;
		}
	}

	if(m_ParticleEffect)
		m_ParticleEffect->Update();
	if(m_ParticleEffect2)
		m_ParticleEffect2->Update();

	// This effect is still active.
	return true;
}

// Returns false when this effect can be removed from the queue.
// NOTE: This Tick function is called on the server AND during Animate on the client.
bool AuEffect::Tick()
{
	// Test to see if the effect is no longer active.
	if(m_Visual == VISUAL_None && m_Sound == 0/*SOUND_None*/)
		return false;	// No effects left to initiate.

	// Calculate seconds offset to start effect.
	m_SecOff = (float)m_Start.MSecsTo(g_Au.CoordinatedTime()) / 1000.f;
	if(m_SecOff < 0)
	{
		//qDebug() << "AuEffect::Tick" << m_Visual << "secOff" << m_SecOff;
		return false;	// 24hr wraparound occurred. Skip this one.
	}

	// Stop some visual effects after a prescribed time.
	// Determine if this effects has gotten too old.
#if(c_MarkWithTurrets == 0)
	if(m_SecOff > Lifetime())
	{
		//qDebug() << "AuEffect::Tick" << m_Visual << m_SecOff << "lifeTime" << Lifetime();
		return false;
	}
#endif

	// This effect is still active.
	return true;
}

void AuEffect::FindShieldIntersection(const AuPoint& rOrigin, const AuPoint& rHitPos, const AuPoint& eRadius)
{
	if(!m_IntersectLoc.IsZero())
		return;
	AuPoint rDir = rHitPos-rOrigin;
	double x2 = eRadius.x*eRadius.x;
	double y2 = eRadius.y*eRadius.y;
	double z2 = eRadius.z*eRadius.z;

	double a = ((rDir.x*rDir.x)/(x2))
			 + ((rDir.y*rDir.y)/(y2))
			 + ((rDir.z*rDir.z)/(z2));
	double b = ((2*rOrigin.x*rDir.x)/(x2))
			 + ((2*rOrigin.y*rDir.y)/(y2))
			 + ((2*rOrigin.z*rDir.z)/(z2));
	double c = ((rOrigin.x*rOrigin.x)/(x2))
			 + ((rOrigin.y*rOrigin.y)/(y2))
			 + ((rOrigin.z*rOrigin.z)/(z2))
			 - 1;

	double d = ((b*b)-(4*a*c));
	if(d < 0)
		return;
	else
		d = sqrt(d);
	// we want the result closest to the origin of the ray
	double hit = (-b + d)/(2*a);
	double hitsecond = (-b - d)/(2*a);
	double result = hitsecond;
	if(hit < hitsecond)
		result = hit;
	m_IntersectLoc = rOrigin + rDir*result;
}

AuPoint AuEffect::Rotation() const
{
	switch(Visual())
	{
	case VISUAL_FireEnergyBeam:
		// Beam weapon effect does not want rotation.
		return AuPoint(0,0,0);

	case VISUAL_ExplodeSpacecraft:
	case VISUAL_ExplodeVehicle:
	case VISUAL_Explosion:
	case VISUAL_HitAcid:
	case VISUAL_HitBludgeon:
	case VISUAL_HitBludgeonBlood:
	case VISUAL_HitBludgeonBug:
	case VISUAL_HitCold:
	case VISUAL_HitDisease:
	case VISUAL_HitElectric:
	case VISUAL_HitFire:
	case VISUAL_HitGasping:
	case VISUAL_HitParalysis:
	case VISUAL_HitPierce:
	case VISUAL_HitPierceBlood:
	case VISUAL_HitPierceBug:
	case VISUAL_HitPoison:
	case VISUAL_HitRadiation:
	case VISUAL_HitSeed:
	case VISUAL_HitSlash:
	case VISUAL_HitSlashBlood:
	case VISUAL_HitSlashBug:
	case VISUAL_HitSonic: 
	case VISUAL_KnifeSwish:
	case VISUAL_None:
		break;

	case VISUAL_FireCannon:
	case VISUAL_FireMachineGun:
	case VISUAL_FirePistol:
	case VISUAL_FireRifle:
	case VISUAL_FireShotgun:
		{
			// Rotate 45 degrees on its firing direction axis, which is the Y axis.
			AuMatrix4x4 rot;
			rot.Rotate(m_Rotation);
			rot.RotateY(45*DEG2RADf);
			return rot.GetRotation();
		}
		break;
	}

	// Default return is the raw rotation value.
	return m_Rotation;
}

// Scale factor applied to base model for visual rendering.
float AuEffect::Scale() const
{
	switch(Visual())
	{
	case VISUAL_None:
	case VISUAL_ExplodeSpacecraft:
	case VISUAL_ExplodeVehicle:
	case VISUAL_Explosion:
	case VISUAL_FireEnergyBeam:
	case VISUAL_HitAcid:
	case VISUAL_HitCold:
	case VISUAL_HitDisease:
	case VISUAL_HitElectric:
	case VISUAL_HitFire:
	case VISUAL_HitGasping:
	case VISUAL_HitParalysis:
	case VISUAL_HitPoison:
	case VISUAL_HitRadiation:
	case VISUAL_HitSonic: 
	case VISUAL_HitBludgeon:
	case VISUAL_HitPierce:
	case VISUAL_HitSlash:
		// These effects do not scale a model.
		return 0;

	case VISUAL_HitBludgeonBlood:
	case VISUAL_HitBludgeonBug:
	case VISUAL_HitPierceBlood:
	case VISUAL_HitPierceBug:
	case VISUAL_HitSlashBlood:
	case VISUAL_HitSlashBug:
		// Grow until gone.
		return Gain() * (SecondsOffset()+1)*5;

	case VISUAL_HitSeed:
		return (Gain() + 1) * 3;

	case VISUAL_FireCannon:
		return Gain();

	case VISUAL_FireMachineGun:
		return 2;

	case VISUAL_FirePistol:
	case VISUAL_FireRifle:
	case VISUAL_FireShotgun:
	case VISUAL_KnifeSwish:
		return 1;
	}
	return 0;
}

void AuEffect::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientR, float ambientG,
		float ambientB, float fogDensity, float fogR, float fogG, float fogB, float effectScale)
{
	if(Visual() == VISUAL_None)
		return;	// nothing to render

	if(m_ParticleEffect)
	{
		// Render Scene
		re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

		AuMatrix4x4 tThis;
		tThis.Translate(OriginLoc());
		tThis.Rotate(Rotation());
		tThis = model * tThis;

		m_ParticleEffect->Render(projection, tThis, re, effectScale);
		if(m_ParticleEffect2)
			m_ParticleEffect2->Render(projection, tThis, re, effectScale);

		re.glDisable(GL_BLEND);
		re.glDepthMask(GL_TRUE);
	}
	else if(Model())
	{
		// Visual Effect
		// Configure appropriate blending based on the effect.
		switch(m_Visual)
		{
		case VISUAL_None:
		case VISUAL_HitAcid:
		case VISUAL_HitDisease:
		case VISUAL_HitPoison:
		case VISUAL_HitSeed:
		case VISUAL_HitSonic:
			// No blending.
			re.glDisable(GL_BLEND);
			break;
		case VISUAL_HitBludgeon:
		case VISUAL_HitBludgeonBlood:
		case VISUAL_HitBludgeonBug:
		case VISUAL_HitCold:
		case VISUAL_HitGasping:
		case VISUAL_HitParalysis:
		case VISUAL_HitPierce:
		case VISUAL_HitPierceBlood:
		case VISUAL_HitPierceBug:
		case VISUAL_HitSlash:
		case VISUAL_HitSlashBlood:
		case VISUAL_HitSlashBug:
		case VISUAL_KnifeSwish:
			// Regular blending.
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case VISUAL_ExplodeSpacecraft:
		case VISUAL_ExplodeVehicle:
		case VISUAL_FireCannon:
		case VISUAL_FireMachineGun:
		case VISUAL_FirePistol:
		case VISUAL_FireRifle:
		case VISUAL_FireShotgun:
		case VISUAL_HitElectric:
		case VISUAL_HitFire:
		case VISUAL_HitRadiation:
		case VISUAL_Explosion:
		case VISUAL_FireEnergyBeam:
			// Additive blending
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		}

		// Some effects work better without the depth mask.
		re.glDepthMask(GL_FALSE);
		re.glDisable(GL_CULL_FACE);

		if(Visual() == VISUAL_FireCannon ||
				Visual() == VISUAL_FireMachineGun ||
				Visual() == VISUAL_FireRifle)
		{
			// Multiply by current transformation
			AuMatrix4x4 tThis;
			tThis.Translate(OriginLoc());
			tThis.Rotate(Rotation());
#if(c_MarkWithTurrets == 0)
			float scl = Scale();
			if(scl != 1)
				tThis.Scale(scl, scl, scl);
#endif
			tThis = model * tThis;
			scl /= 4;	// Scale of side flashes.

			m_Model->SetTexture(Texture());	// Override model texture.
			if(fogDensity > 0)
			{
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, Alpha());

				// Draw flash suppressor pattern.
				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateX(PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, Alpha());

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateX(-PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, Alpha());

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateZ(PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, Alpha());

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateZ(-PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, Alpha());
			}
			else
			{
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, Alpha(),
						false);

				// Draw flash suppressor pattern.
				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateX(PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, Alpha(),
						false);

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateX(-PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, Alpha(),
						false);

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateZ(PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, Alpha(),
						false);

				tThis.LoadIdentity();
				tThis.Translate(OriginLoc());
				tThis.Rotate(Rotation());
				tThis.RotateZ(-PI/2);
#if(c_MarkWithTurrets == 0)
				tThis.Scale(scl, scl, scl);
#endif
				tThis = model * tThis;
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, Alpha(),
						false);
			}
		}
		else
		{
			// Multiply by current transformation
			AuMatrix4x4 tThis;
			tThis.Translate(OriginLoc());
			tThis.Rotate(Rotation());
#if(c_MarkWithTurrets == 0)
			const float scl = Scale();
			if(scl != 1)
				tThis.Scale(scl, scl, scl);
#endif
			tThis = model * tThis;

			float alpha = Alpha();

			m_Model->SetTexture(Texture());	// Override model texture.
			if(fogDensity > 0)
				m_Model->RenderGLSLModelTexFog(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false,
						fogDensity,fogR,fogG,fogB, alpha, false);
			else
				m_Model->RenderGLSLModelTex(projection, tThis, re, ambientR,ambientG,ambientB, AuPoint(0,0,1), false, alpha, false);
		}

		re.glEnable(GL_CULL_FACE);
		re.glDisable(GL_BLEND);
		re.glDepthMask(GL_TRUE);
	}
}

AuParticleSystem* AuEffect::CreateExplosion(AuGLEffects& re, WEAPONEFFECT effect, float radius)
{
	//qDebug() << "AuEffect::CreateExplosion effect" << effect << "radius" << radius << "ft";
	switch(effect)
	{
	case WE_Acid:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WAc", ":data/acidsprite.png");
			PsExplodeAcid* explosion = new PsExplodeAcid(radius, Gain(), texId);
			return explosion;
		}
	case WE_Bludgeon:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodeBludgeon* explosion = new PsExplodeBludgeon(radius, Gain(), texId);
			return explosion;
		}
	case WE_Cold:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WCo", ":data/SnowFlake.png");
			PsExplodeCold* explosion = new PsExplodeCold(radius, Gain(), texId);
			return explosion;
		}
	case WE_Crush:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodeCrush* explosion = new PsExplodeCrush(radius, Gain(), texId);
			return explosion;
		}
	case WE_Disease:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WDi", ":data/diseasesprite.png");
			PsExplodeDisease* explosion = new PsExplodeDisease(radius, Gain(), texId);
			return explosion;
		}
	case WE_Electric:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WEl", ":data/electricsprite.png");
			GLuint texIdGlow = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodeElectric* explosion = new PsExplodeElectric(radius, Gain(), texId, texIdGlow);
			return explosion;
		}
	case WE_Heat:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodeHeat* explosion = new PsExplodeHeat(radius, Gain(), texId);
			return explosion;
		}
	case WE_Pierce:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodePierce* explosion = new PsExplodePierce(radius, Gain(), texId);
			return explosion;
		}
	case WE_Poison:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WPo", ":data/poisonsprite.png");
			PsExplodePoison* explosion = new PsExplodePoison(radius, Gain(), texId);
			return explosion;
		}
	case WE_Radiation:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WEl", ":data/electricsprite.png");
			GLuint texIdGlow = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WRa", ":data/radiationsprite.png");
			PsExplodeRadiation* explosion = new PsExplodeRadiation(radius, Gain(), texId, texIdGlow);
			return explosion;
		}
	case WE_Slash:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
			PsExplodeSlash* explosion = new PsExplodeSlash(radius, Gain(), texId);
			return explosion;
		}
	case WE_Sonic:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WSo", ":data/sonicsprite.png");
			PsExplodeSonic* explosion;
			if(radius <= 25000.f)
				explosion = new PsExplodeSonic(radius, 2.5f, Gain(), texId);
			else
				explosion = new PsExplodeSonic(radius, radius / 10000.f, Gain(), texId);
			return explosion;
		}
	case WE_Stun:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WSt", ":data/stunsprite.png");
			PsExplodeStun* explosion = new PsExplodeStun(radius, Gain(), texId);
			return explosion;
		}
	case WE_Suffocate:
		{
			GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WPo", ":data/poisonsprite.png");
			PsExplodeSuffocate* explosion = new PsExplodeSuffocate(radius, Gain(), texId);
			return explosion;
		}
	case WE_None:
	case WE_Corrosion:
	case WE_Audible:
	default:
		// These are not valid damage effects for explosions.
		Q_ASSERT(false);
		return 0;
	}
}

bool AuEffect::IsInScene(const AuPoint& sceneLoc) const
{
	double sightDist2 = ExplosionRadius1();	// Squared distance at which this effect can be seen in the view.
	if(sightDist2 == 0)
		sightDist2 = 1;
	sightDist2 *= 1000;
	sightDist2 *= sightDist2;	// squared
	//qDebug() << "AuEffect::IsInScene rads" << m_Radius1 << m_Radius2
		//<< "sightDist" << sqrt(sightDist2) << "dist" << OriginLoc().Distance(sceneLoc) << "hasTarg" << HasTargetLoc();

	if(OriginLoc().Distance2(sceneLoc) < sightDist2)
		return true;

	if(HasTargetLoc() && TargetLoc().Distance2(sceneLoc) < sightDist2)
		return true;

	return false;
}

void AuEffect::GetRenderLights(const AuMatrix4x4& model, QList<AuGLLight>& lights) const
{
	QColor lightColor;
	float lightRadius = 0;
	float lightAlpha = 0;
	switch(Visual())
	{
	case VISUAL_None:
	case VISUAL_HitAcid:
	case VISUAL_HitBludgeon:
	case VISUAL_HitBludgeonBlood:
	case VISUAL_HitBludgeonBug:
	case VISUAL_HitCold:
	case VISUAL_HitDisease:
	case VISUAL_HitGasping:
	case VISUAL_HitParalysis:
	case VISUAL_HitPierce:
	case VISUAL_HitPierceBlood:
	case VISUAL_HitPierceBug:
	case VISUAL_HitPoison:
	case VISUAL_HitSeed:
	case VISUAL_HitSlash:
	case VISUAL_HitSlashBlood:
	case VISUAL_HitSlashBug:
	case VISUAL_HitSonic: 
	case VISUAL_KnifeSwish:
		// These effects do not emit light.
		return;
	case VISUAL_ExplodeSpacecraft:
		lightAlpha = Alpha();
		lightColor = QColor(157,255,193);
		lightRadius = ExplosionRadius1();
		break;
	case VISUAL_ExplodeVehicle:
	case VISUAL_HitFire:
		lightAlpha = Alpha();
		lightColor = QColor(255,254,226);
		lightRadius = ExplosionRadius1();
		break;
	case VISUAL_Explosion:
		lightAlpha = Alpha();
		lightColor = QColor(255,254,226);
		if(ExplosionRadius1() > ExplosionRadius2())
			lightRadius = ExplosionRadius1();
		else
			lightRadius = ExplosionRadius2();
		break;
	case VISUAL_FireEnergyBeam:
		lightAlpha = Alpha();
		lightColor = QColor(191,184,255);
		lightRadius = 5 * Gain();
		break;
	case VISUAL_HitRadiation:
		lightAlpha = Alpha();
		lightColor = QColor(157,255,193);
		lightRadius = ExplosionRadius1();
		break;
	case VISUAL_HitElectric:
		lightAlpha = Alpha();
		lightColor = QColor(191,184,255);
		lightRadius = ExplosionRadius1();
		break;
	case VISUAL_FireCannon:
	case VISUAL_FireMachineGun:
	case VISUAL_FirePistol:
	case VISUAL_FireRifle:
	case VISUAL_FireShotgun:
		lightAlpha = Alpha();
		lightColor = QColor(255,254,226);
		lightRadius = 5 * Gain();
		break;
	}
	if(lightAlpha == 0)
		return;
	lightRadius *= 5;
	lightRadius = AuNoiseF::Cosine_Interpolate(0, lightRadius, lightAlpha);	// Light radius drops off with time.
	//qDebug() << "effect" << Visual() << "light radius" << lightRadius;
	float kQ = 1.f / (lightRadius * lightRadius);				// Quadratic attenuation. 1/lightRadiusSquared
	float kL = .004f;											// Linear attenuation. A higher number produces quicker falloff.
	// So far, effect lights do not need an id because of how the id is used.
	lights.append(AuGLLight(AuId(), AuGLLight::Effect, kL, kQ, lightColor, model.Project(OriginLoc())));
}

GLuint AuEffect::Texture() const
{
	if(m_Texture)
		return m_Texture;	// This texture overrides the one in the model.
	else if(Model())
		return Model()->Texture();
	return 0;
}

float AuEffect::ReferenceDistance() const
{
	return AuSound::SoundReferenceDistance((SOUND_EFFECT)Sound());
}

float AuEffect::RolloffFactor() const
{
	return AuSound::SoundRolloffFactor((SOUND_EFFECT)Sound());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuEffectArray::GetRenderLights(const AuMatrix4x4& model, QList<AuGLLight>& lights) const
{
	for(QList<AuEffect>::const_iterator ei = m_Effects.constBegin(); ei != m_Effects.constEnd(); ei++)
		(*ei).GetRenderLights(model, lights);
}

quint64 AuEffectArray::MemoryUsage() const
{
	quint64 mem = sizeof(AuEffectArray);
	for(QList<AuEffect>::const_iterator ei = m_Effects.constBegin(); ei != m_Effects.constEnd(); ei++)
		mem += sizeof(AuEffect);
	return mem;
}

