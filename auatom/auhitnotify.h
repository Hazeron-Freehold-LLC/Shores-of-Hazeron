//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhitnotify.h,v 1.17 2022/04/15 16:54:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuHitNotify_H
#define AuHitNotify_H

#include "auelapsedtimer.h"
#include "augleffects.h"
#include "aupoint.h"

class AuAtomRenderInfo;

class AuHitNotify 
{
	enum HITTYPE
	{
		HIT_Internal,
		HIT_Body,
		HIT_BodyArmor
	};

private:
	AuPoint			m_HitDirection;		
	WEAPONEFFECT		m_HitEffect1;
	WEAPONEFFECT		m_HitEffect2;
	qint32			m_HitsAbsorbed1;
	qint32			m_HitsAbsorbed2;
	qint32			m_HitsDamage1;
	qint32			m_HitsDamage2;
	AuElapsedTimer		m_StopWatch;
	
public:
	AuHitNotify();
	AuHitNotify(const AuPoint& fromDir, WEAPONEFFECT effect1, qint32 hitsDamage1, qint32 hitsAbsorbed1,
			WEAPONEFFECT effect2, qint32 hitsDamage2, qint32 hitsAbsorbed2);
	AuHitNotify(const AuHitNotify& c);

	const AuHitNotify& operator=(const AuHitNotify& c);

	inline void ClearHit()
	{
		m_HitEffect1 = WE_None;
	}

	void DrawHit(AuAtomRenderInfo& ari, float hit, bool isExoskeletal);

	inline const AuPoint& HitDirection() const
	{
		return m_HitDirection;
	}

	inline bool IsHit() const
	{
		return m_HitEffect1 != WE_None;
	}

	inline bool IsReadyToDecay() const
	{
		return m_StopWatch.elapsed() > 2000;
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline void RestartTime()
	{
		m_StopWatch.restart();
	}

	void RotateZ(float angle);

	inline void SetHitDirection(const AuPoint& hitDir)
	{
		m_HitDirection = hitDir;
	}

private:
	float Alpha() const;

	AuGLVAO CreateModel(AuGLEffects& re, HITTYPE type) const;

	void Decay();

	float DomeRadius(HITTYPE type) const;

	bool HasDirection() const;

	bool HasHitPoints() const;

	void RenderShell(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, HITTYPE hitType,
			WEAPONEFFECT effect, qint32 hits, float splatSize, float intensity, float alpha, bool isExoskeletal) const;

	void RenderShield(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float splatSize, float intensity,
			float alpha) const;

	void SetBlendFunc(AuGLEffects& re, WEAPONEFFECT effect) const;

	float SplatSize(HITTYPE type, int which) const;

	const char* TextureFileEndoskeletal(WEAPONEFFECT effect) const;

	const char* TextureFileExoskeletal(WEAPONEFFECT effect) const;

	float ZRotation(WEAPONEFFECT effect) const;
};

#endif // AuHitNotify_H
