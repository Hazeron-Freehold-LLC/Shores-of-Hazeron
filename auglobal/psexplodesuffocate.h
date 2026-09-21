//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodesuffocate.h,v 1.5 2022/02/19 18:32:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeSuffocate_H
#define PsExplodeSuffocate_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeSuffocate : public AuParticleSystem
{
public:
	PsExplodeSuffocate(float radius, float gain, GLuint texId);
	virtual ~PsExplodeSuffocate();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeSuffocate_H
