//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pslaunchsystem.h,v 1.2 2022/02/19 18:32:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsLaunchSystem_H
#define PsLaunchSystem_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsLaunchSystem : public AuParticleSystem
{
private:
	float								m_SegScale;				// Scale of beam segments vs particle size returned from GetSize().
	bool								m_Switch;				// use this to alternate draw order

public:
	PsLaunchSystem(GLuint texIdSpot, GLuint texIdStreak);
	~PsLaunchSystem();

	// Overrides
	virtual bool HasParticleExpired(const auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void MoveParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	//PsLaunchSystem_H
