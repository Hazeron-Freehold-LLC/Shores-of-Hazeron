//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aueffect.h,v 1.60 2022/04/15 20:45:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEffect_H
#define AuEffect_H

#include "au3dsglsl.h"
#include "auatom.h"
#include "augleffects.h"
#include "autime.h"
#include <QList>

class AuAtomRenderInfo;
class AuGLEffects;
class AuParticleSystem;

enum VISUAL_EFFECT
{
	VISUAL_None,
	VISUAL_ExplodeSpacecraft,
	VISUAL_ExplodeVehicle,
	VISUAL_Explosion,
	VISUAL_FireCannon,
	VISUAL_FireEnergyBeam,
	VISUAL_FireMachineGun,
	VISUAL_FirePistol,
	VISUAL_FireRifle,
	VISUAL_FireShotgun,
	VISUAL_HitAcid,
	VISUAL_HitBludgeon,
	VISUAL_HitBludgeonBlood,
	VISUAL_HitBludgeonBug,
	VISUAL_HitCold,
	VISUAL_HitDisease,
	VISUAL_HitElectric,
	VISUAL_HitFire,
	VISUAL_HitGasping,
	VISUAL_HitParalysis,
	VISUAL_HitPierce,
	VISUAL_HitPierceBlood,
	VISUAL_HitPierceBug,
	VISUAL_HitPoison,
	VISUAL_HitRadiation,
	VISUAL_HitSeed,
	VISUAL_HitSlash,
	VISUAL_HitSlashBlood,
	VISUAL_HitSlashBug,
	VISUAL_HitSonic, 
	VISUAL_KnifeSwish,
	// Last item must not exceed 0x7f or changes to streaming operators are required.
};

class AUATOM_DLL AuEffect
{
private:
	float			m_Gain;				// Strength of effect. Sound, energy, etc.
	AuPoint			m_IntersectLoc;		// Relative to parent atom. For collision effects. (shield hits)
	AuPoint			m_OriginLoc;		// Origin of the effect. Relative to parent atom.
	float			m_Pitch;			// Valid when m_Sound != SOUND_None.
	AuPoint			m_Rotation;			// The rotation.
	float			m_SecOff;			// Seconds offset since effect started.
	int				m_Sound;			// SOUND_EFFECT. May be SOUND_None.
	bool			m_ShieldHit;		// True if this is a hit on a shield.
	AuTime			m_Start;			// UTC time this effect was initiated on server.
	AuPoint			m_TargetLoc;		// Other end of effect. Relative to parent atom.
	AuPoint			m_Vector;			// Movement vector, relative to the parent atom.
	VISUAL_EFFECT	m_Visual;			// Visual effect. May be VISUAL_None.

	// Weapon Explosion Effect
	quint8			m_Effect1;			// Also used for beam weapon effects.
	float			m_Radius1;
	quint8			m_Effect2;
	float			m_Radius2;

	// OpenGL
	Au3dsGLSLModelInstance*	m_Model;
	GLuint				m_Texture;
	AuParticleSystem*	m_ParticleEffect;
	AuParticleSystem*	m_ParticleEffect2;

public:
	inline AuEffect(const AuPoint& loc, const AuPoint& rot, const AuPoint& vector, int sound=0, float gain=1,
			float pitch=1, VISUAL_EFFECT visual=VISUAL_None)
	{
		m_Gain = gain;
		m_IntersectLoc = AuPoint(0,0,0);
		m_OriginLoc = loc;
		m_Pitch = pitch;
		m_SecOff = 0;
		m_ShieldHit = false;
		m_Sound = sound;
		m_Vector = vector;
		m_Visual = visual;
		SetRotation(rot);	// Energy beam ignores rotation. Does not set it here.

		m_Effect1 = WE_None;
		m_Radius1 = 0;
		m_Effect2 = WE_None;
		m_Radius2 = 0;

		m_Model = 0;
		m_Texture = 0;
		m_ParticleEffect = 0;
		m_ParticleEffect2 = 0;
	}
	inline AuEffect()
	{
		m_Gain = 1;
		m_Pitch = 1;
		m_SecOff = 0;
		m_ShieldHit = false;
		m_Sound = 0;	//SOUND_None;
		m_Visual = VISUAL_None;

		m_Effect1 = WE_None;
		m_Radius1 = 0;
		m_Effect2 = WE_None;
		m_Radius2 = 0;

		m_Model = 0;
		m_Texture = 0;
		m_ParticleEffect = 0;
		m_ParticleEffect2 = 0;
	}
	virtual ~AuEffect();

	inline AuEffect(const AuEffect& c)
	{
		m_Gain = c.m_Gain;
		m_IntersectLoc = c.m_IntersectLoc;
		m_OriginLoc = c.m_OriginLoc;
		m_Pitch = c.m_Pitch;
		m_Rotation = c.m_Rotation;
		m_SecOff = c.m_SecOff;
		m_ShieldHit = c.m_ShieldHit;
		m_Sound = c.m_Sound;
		m_Start = c.m_Start;
		m_TargetLoc = c.m_TargetLoc;
		m_Vector = c.m_Vector;
		m_Visual = c.m_Visual;

		m_Effect1 = c.m_Effect1;
		m_Radius1 = c.m_Radius1;
		m_Effect2 = c.m_Effect2;
		m_Radius2 = c.m_Radius2;

		m_Model = 0;
		m_Texture = 0;
		m_ParticleEffect = 0;
		m_ParticleEffect2 = 0;
	}

	inline const AuEffect& operator=(const AuEffect& c)
	{
		m_Gain = c.m_Gain;
		m_IntersectLoc = c.m_IntersectLoc;
		m_OriginLoc = c.m_OriginLoc;
		m_Pitch = c.m_Pitch;
		m_Rotation = c.m_Rotation;
		m_SecOff = c.m_SecOff;
		m_ShieldHit = c.m_ShieldHit;
		m_Sound = c.m_Sound;
		m_Start = c.m_Start;
		m_TargetLoc = c.m_TargetLoc;
		m_Vector = c.m_Vector;
		m_Visual = c.m_Visual;

		m_Effect1 = c.m_Effect1;
		m_Radius1 = c.m_Radius1;
		m_Effect2 = c.m_Effect2;
		m_Radius2 = c.m_Radius2;

		m_Model = 0;
		m_Texture = 0;
		m_ParticleEffect = 0;
		m_ParticleEffect2 = 0;

		return *this;
	}

	// Returns false when this effect can be removed from the queue.
	bool Animate(AuAtomRenderInfo& ari);

	// Returns alpha blend factor for this effect.
	inline float Alpha() const
	{
		// Stay at full alpha for the first 50ms.
		const float leadTime = .05f;	// 50ms lead time at full alpha
		if(m_SecOff <= leadTime)
			return 1;
		else if(m_SecOff < VisualDuration())
		{
			// Calculate a linear dropoff from the end of the lead time
			// to the end of the visual duration time, in the range [1..0].
			float dropDur = VisualDuration() - leadTime;	// Dropoff duration.
			return (dropDur - (m_SecOff - leadTime))/dropDur;
		}
		return 0;
	}

	inline void ClearSound()
	{
		m_Sound = 0;	//SOUND_None;
	}

	inline void ClearVisual()
	{
		m_Visual = VISUAL_None;
	}

	// Returns the lifetime of this effect in seconds.
	inline float Lifetime() const
	{
		float visualDuration = VisualDuration();
		if(visualDuration > 0.)
			return visualDuration;

		// Must allow time for sound-only effects to be sent, e.g. SOUND_BombRelease.
		if(m_Sound != 0/*SOUND_None*/)
			return 1.f;

		return 0.f;
	}

	inline WEAPONEFFECT EnergyBeamEffect() const
	{
		return (WEAPONEFFECT)m_Effect1;
	}

	inline WEAPONEFFECT ExplosionEffect1() const
	{
		return (WEAPONEFFECT)m_Effect1;
	}

	inline float ExplosionRadius1() const
	{
		switch(m_Visual)
		{
		case VISUAL_HitAcid:
		case VISUAL_HitCold:
		case VISUAL_HitElectric:
		case VISUAL_HitFire:
		case VISUAL_HitRadiation:
		case VISUAL_HitDisease:
		case VISUAL_HitGasping:
		case VISUAL_HitParalysis:
		case VISUAL_HitPoison:
		case VISUAL_HitSonic:
			return Gain() * 2;

		case VISUAL_HitBludgeon:
		case VISUAL_HitPierce:
		case VISUAL_HitSlash:
			return 1;

		case VISUAL_ExplodeSpacecraft:
			return sqrtf(Gain() + 15) * 10;

		case VISUAL_ExplodeVehicle:
			return Gain() * 2;

		case VISUAL_Explosion:
			return m_Radius1;

		default:
			break;
		}
		return 0;
	}

	inline WEAPONEFFECT ExplosionEffect2() const
	{
		return (WEAPONEFFECT)m_Effect2;
	}

	inline float ExplosionRadius2() const
	{
		switch(m_Visual)
		{
		case VISUAL_ExplodeSpacecraft:
			return sqrtf(Gain() + 15) * 10;

		case VISUAL_ExplodeVehicle:
			return Gain() * 2;

		case VISUAL_Explosion:
			return m_Radius2;

		default:
			break;
		}
		return 0;
	}

	void FindShieldIntersection(const AuPoint& rOrigin, const AuPoint& rHitPos, const AuPoint& eRadius);

	inline float Gain() const
	{
		return m_Gain;
	}

	void GetRenderLights(const AuMatrix4x4& model, QList<AuGLLight>& lights) const;

	inline bool HasTargetLoc() const
	{
		return !m_TargetLoc.IsZero();
	}

	inline bool IsHitEffect() const
	{
		switch(m_Visual)
		{
		case VISUAL_None:
		case VISUAL_ExplodeSpacecraft:
		case VISUAL_ExplodeVehicle:
		case VISUAL_Explosion:
		case VISUAL_FireCannon:
		case VISUAL_FireEnergyBeam:
		case VISUAL_FireMachineGun:
		case VISUAL_FirePistol:
		case VISUAL_FireRifle:
		case VISUAL_FireShotgun:
		case VISUAL_KnifeSwish:
			break;
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
			return true;
		}
		return false;
	}

	inline bool IsNew(const AuTime& sinceWhen) const
	{
		return m_Start >= sinceWhen ? true : false;
	}

	inline bool IsShieldHit() const
	{
		return m_ShieldHit;
	}

	// Returns origin location adjusted for movement over time.
	// This movement over time is not part of the effect itself. It occurs when a single DaEffect atom must manage multiple
	// different effects, such as explosions, that are moving because their various parent atoms were moving when destroyed.
	inline AuPoint OriginLoc() const
	{
		if(m_Vector == AuPoint(0,0) || m_SecOff < 0)
			return m_OriginLoc;
		return m_OriginLoc + m_Vector * m_SecOff;
	}

	inline Au3dsGLSLModelInstance* Model() const
	{
		return m_Model;
	}

	inline AuParticleSystem* ParticleEffect() const
	{
		return m_ParticleEffect;
	}

	inline AuParticleSystem* ParticleEffect2() const
	{
		return m_ParticleEffect2;
	}

	inline bool operator==(const AuEffect& c) const
	{
		if(m_Start != c.m_Start)
			return false;
		if(m_Visual != c.m_Visual)
			return false;
		if(m_ShieldHit != c.m_ShieldHit)
			return false;
		if(m_Sound != c.m_Sound)
			return false;
		if(m_OriginLoc != c.m_OriginLoc)
			return false;
		if(m_Gain != c.m_Gain)
			return false;
		if(m_Pitch != c.m_Pitch)
			return false;
		return true;
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline float Pitch() const
	{
		// Pitch is only valid when a sound effect has been specified.
		Q_ASSERT(m_Sound != 0/*SOUND_None*/);
		return m_Pitch;
	}

	// Reference distance used for sound attenuation.
	// From OpenAL documentation on the default Inverse Distance Rolloff Clamped (AL_INVERSE_DISTANCE_CLAMPED) model.
	//   distance = max(distance, AL_REFERENCE_DISTANCE)
	//   distance = min(distance, AL_MAX_DISTANCE)
	//   gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE))
	float ReferenceDistance() const;

	void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientR, float ambientG,
			float ambientB, float fogDensity, float fogR, float fogG, float fogB, float effectScale);

	// Rolloff factor used for sound attenuation.
	// From OpenAL documentation on the default Inverse Distance Rolloff Clamped (AL_INVERSE_DISTANCE_CLAMPED) model.
	//   distance = max(distance, AL_REFERENCE_DISTANCE)
	//   distance = min(distance, AL_MAX_DISTANCE)
	//   gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance – AL_REFERENCE_DISTANCE))
	float RolloffFactor() const;

	AuPoint Rotation() const;

	// Scale factor applied to base model for visual rendering.
	float Scale() const;

	// Distance at which this effect appears in someone's scene.
	bool IsInScene(const AuPoint& sceneLoc) const;

	inline float SecondsOffset() const
	{
		return m_SecOff;
	}

	inline void SetRotation(const AuPoint& rot)
	{
		// Energy beam does not want rotation to be set.
		if(m_Visual != VISUAL_FireEnergyBeam)
			m_Rotation = rot;
	}

	inline void SetShieldHit(bool shieldHit)
	{
		m_ShieldHit = shieldHit;
	}

	inline void SetTargetLoc(const AuPoint& loc)
	{
		// Only energy beam needs target location to be set.
		if(m_Visual == VISUAL_FireEnergyBeam)
			m_TargetLoc = loc;
	}

	inline void SetStartTime()
	{
		m_Start.Set();
	}

	inline void SetVisual(VISUAL_EFFECT visual)
	{
		m_Visual = visual;
	}

	// Setup for VISUAL_EnergyBeam
	inline void SetEnergyBeamEffect(WEAPONEFFECT effect)
	{
		if(m_Visual == VISUAL_FireEnergyBeam)
			m_Effect1 = effect;
	}

	// Setup for VISUAL_Explosion
	inline void SetExplosionEffect1(WEAPONEFFECT effect, float radius)
	{
		m_Effect1 = effect;
		m_Radius1 = radius;
	}

	// Setup for VISUAL_Explosion
	inline void SetExplosionEffect2(WEAPONEFFECT effect, float radius)
	{
		m_Effect2 = effect;
		m_Radius2 = radius;
	}

	inline int Sound() const
	{
		return m_Sound;
	}

	inline const AuTime& StartTime() const
	{
		return m_Start;
	}

	// Returns target location adjusted for movement over time.
	inline AuPoint TargetLoc() const
	{
		if(m_Vector == AuPoint(0,0) || m_SecOff < 0)
			return m_TargetLoc;
		return m_TargetLoc + m_Vector * m_SecOff;
	}

	inline const AuPoint& IntersectLoc() const
	{
		return m_IntersectLoc;
	}

	// Returns id of texture that should be used to render this effect.
	// May be zero but don't ever expect it to be.
	GLuint Texture() const;

	inline float TextureOffsetX() const
	{
		// Stay at full alpha for the first 50ms.
		const float leadTime = .05f;	// 50ms lead time at full alpha
		if(m_SecOff <= leadTime)
			return 0;
		else if(m_SecOff < VisualDuration())
		{
			// Calculate a linear offset from the end of the lead time to the end of the visual duration time, in the range [0..1].
			float offDur = VisualDuration() - leadTime;	// Offset duration.
			return (m_SecOff - leadTime)/offDur;
		}
		return 1;
	}

	// Returns false when this effect can be removed from the queue.
	bool Tick();

	inline VISUAL_EFFECT Visual() const
	{
		return m_Visual;
	}

	// Seconds duration of this visual effect.
	inline float VisualDuration() const
	{
		switch(m_Visual)
		{
		case VISUAL_None:
			break;
		case VISUAL_FireMachineGun:
			return 3.f;
		case VISUAL_FirePistol:
		case VISUAL_FireRifle:
		case VISUAL_FireShotgun:
		case VISUAL_KnifeSwish:
			return .25f;
		case VISUAL_FireCannon:
			return 1.5f;
		case VISUAL_FireEnergyBeam:
			return 1.f;
		case VISUAL_ExplodeSpacecraft:
		case VISUAL_ExplodeVehicle:
		case VISUAL_HitAcid:
		case VISUAL_HitBludgeon:
		case VISUAL_HitCold:
		case VISUAL_HitDisease:
		case VISUAL_HitElectric:
		case VISUAL_HitFire:
		case VISUAL_HitGasping:
		case VISUAL_HitParalysis:
		case VISUAL_HitPierce:
		case VISUAL_HitPoison:
		case VISUAL_HitRadiation:
		case VISUAL_HitSlash:
		case VISUAL_HitSonic:
			return 2.5f;

		case VISUAL_Explosion:
			// Big explosions last a little longer.
			if(m_Radius1 > 25000.f && m_Radius1 > m_Radius2)
				return m_Radius1/10000.f;
			else if(m_Radius2 > 25000.f)
				return m_Radius2/10000.f;
			else
				return 2.5f;

		case VISUAL_HitBludgeonBlood:
		case VISUAL_HitBludgeonBug:
		case VISUAL_HitPierceBlood:
		case VISUAL_HitPierceBug:
		case VISUAL_HitSlashBlood:
		case VISUAL_HitSlashBug:
			return .6f;

		case VISUAL_HitSeed:
			return .75f;
		}
		return 0;
	}

protected:
	AuParticleSystem* CreateExplosion(AuGLEffects& re, WEAPONEFFECT effect, float radius);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuEffectArray
{
public:
	QList<AuEffect>						m_Effects;

public:
	// Adds an effect if it doesn't appear to already be in this array.
	inline bool AddIfUnique(const AuEffect& effect)
	{
		for(QList<AuEffect>::const_iterator it = m_Effects.cbegin(); it != m_Effects.cend(); it++)
			if((*it) == effect)
				return false;	// Already a matching effect in this array.
		m_Effects.append(effect);
		return true;
	}

	// Returns true if any are added.
	inline bool AddNewer(const AuEffectArray& effects, const AuTime& sinceWhen)
	{
		bool anyAdded = false;
		for(QList<AuEffect>::const_iterator it = effects.m_Effects.cbegin(); it != effects.m_Effects.cend(); it++)
			if((*it).IsNew(sinceWhen))
				if(AddIfUnique(*it))
					anyAdded = true;
		return anyAdded;
	}

	inline void AddUnique(const AuEffectArray& effects)
	{
		for(QList<AuEffect>::const_iterator it = effects.m_Effects.cbegin(); it != effects.m_Effects.cend(); it++)
			AddIfUnique(*it);
	}

	inline void Animate(AuAtomRenderInfo& ari)
	{
		QList<AuEffect>::iterator it = m_Effects.begin();
		while(it != m_Effects.end())
		{
			if(!(*it).Animate(ari))
				it = m_Effects.erase(it);
			else
				it++;
		}
	}

	inline int CountNew(const AuTime& sinceWhen) const
	{
		int nucount = 0;
		for(QList<AuEffect>::const_iterator it = m_Effects.cbegin(); it != m_Effects.cend(); it++)
			if((*it).IsNew(sinceWhen))
				nucount++;
		return nucount;
	}

	void GetRenderLights(const AuMatrix4x4& model, QList<AuGLLight>& lights) const;

	inline bool HasSoundEffect(int sound) const
	{
		for(QList<AuEffect>::const_iterator it = m_Effects.cbegin(); it != m_Effects.cend(); it++)
			if((*it).Sound() == sound)
				return true;			// Found effect return result
		return false;
	}

	inline bool HasVisualEffect(VISUAL_EFFECT visual) const
	{
		for(QList<AuEffect>::const_iterator it = m_Effects.cbegin(); it != m_Effects.cend(); it++)
			if((*it).Visual() == visual)
				return true;			// Found effect return result
		return false;
	}

	quint64 MemoryUsage() const;

	// Server's version of animate doesn't involve rendering.
	inline void Tick()
	{
		QList<AuEffect>::iterator it = m_Effects.begin();
		while(it != m_Effects.end())
		{
			if(!(*it).Tick())
				it = m_Effects.erase(it);
			else
				it++;
		}
	}
};

#endif // AuEffect_H
