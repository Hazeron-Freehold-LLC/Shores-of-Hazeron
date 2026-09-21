//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodebludgeon.h,v 1.5 2022/02/19 18:32:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeBludgeon_H
#define PsExplodeBludgeon_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeBludgeon : public AuParticleSystem
{
public:
	PsExplodeBludgeon(float radius, float gain, GLuint texId);
	virtual ~PsExplodeBludgeon();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeBludgeon_H
