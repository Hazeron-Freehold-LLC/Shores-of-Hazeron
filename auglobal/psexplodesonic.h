//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodesonic.h,v 1.6 2022/02/19 18:32:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeSonic_H
#define PsExplodeSonic_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeSonic : public AuParticleSystem
{
public:
	PsExplodeSonic(float radius, float lifetime, float gain, GLuint texId);
	virtual ~PsExplodeSonic();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeSonic_H
