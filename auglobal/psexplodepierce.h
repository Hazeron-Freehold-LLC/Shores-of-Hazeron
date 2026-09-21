//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psexplodepierce.h,v 1.6 2022/02/19 18:32:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsExplodePierce_H
#define PsExplodePierce_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsExplodePierce : public AuParticleSystem
{
private:
	float								m_Radius;

public:
	PsExplodePierce(float radius, float gain, GLuint texId);
	virtual ~PsExplodePierce();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsExplodePierce_H
