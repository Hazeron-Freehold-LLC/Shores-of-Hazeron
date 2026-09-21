//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodeslash.h,v 1.5 2022/02/19 18:32:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodeSlash_H
#define PsExplodeSlash_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodeSlash : public AuParticleSystem
{
private:
	float								m_Radius;

public:
	PsExplodeSlash(float radius, float gain, GLuint texId);
	virtual ~PsExplodeSlash();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodeSlash_H
