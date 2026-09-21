//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodepoison.h,v 1.5 2022/02/19 18:32:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodePoison_H
#define PsExplodePoison_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodePoison : public AuParticleSystem
{
public:
	PsExplodePoison(float radius, float gain, GLuint texId);
	virtual ~PsExplodePoison();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodePoison_H
