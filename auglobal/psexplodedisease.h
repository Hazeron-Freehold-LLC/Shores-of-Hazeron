//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodedisease.h,v 1.5 2022/02/19 18:32:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeDisease_H
#define PsExplodeDisease_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeDisease : public AuParticleSystem
{
public:
	PsExplodeDisease(float radius, float gain, GLuint texId);
	virtual ~PsExplodeDisease();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeDisease_H
