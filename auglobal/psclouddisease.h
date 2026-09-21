//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psclouddisease.h,v 1.4 2022/02/19 18:31:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsCloudDisease_H
#define PsCloudDisease_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsCloudDisease : public AuParticleSystem
{
public:
	PsCloudDisease(float radius, float gain, GLuint texId);
	virtual ~PsCloudDisease();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsCloudDisease_H
