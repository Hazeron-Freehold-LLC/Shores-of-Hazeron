//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexploderadiation.h,v 1.5 2022/02/19 18:32:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeRadiation_H
#define PsExplodeRadiation_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeRadiation : public AuParticleSystem
{
public:
	PsExplodeRadiation(float radius, float gain, GLuint texId, GLuint texIdGlow);
	virtual ~PsExplodeRadiation();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeRadiation_H
