//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pslandingsystem.h,v 1.2 2022/02/19 18:32:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsLandingSystem_H
#define PsLandingSystem_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsLandingSystem : public AuParticleSystem
{
private:
	int									m_Number;				// Landing system identifier.
	float								m_SegScale;				// Scale of beam segments vs particle size returned from GetSize().
	bool								m_Switch;				// use this to alternate draw order

public:
	PsLandingSystem(GLuint texIdSpot, GLuint texIdStreak);
	~PsLandingSystem();

	inline void SetNumber(int n)
	{
		m_Number = n;
	}

	// Overrides
	virtual bool HasParticleExpired(const auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void MoveParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	//PsLandingSystem_H
