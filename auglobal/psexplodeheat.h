//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodeheat.h,v 1.6 2022/02/19 18:32:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeHeat_H
#define PsExplodeHeat_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeHeat : public AuParticleSystem
{
public:
	PsExplodeHeat(float radius, float gain, GLuint texId);
	virtual ~PsExplodeHeat();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeHeat_H
