//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psnebula.h,v 1.3 2022/02/19 18:32:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsNebula_H
#define PsNebula_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsNebula : public AuParticleSystem
{
	float								m_Density;

public:
	PsNebula();
	virtual ~PsNebula();

	// Overrides
	virtual void InitParticle(auParticle* pParticle);
	virtual bool HasParticleExpired(const auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

	inline void SetDensity(float density)
	{
		m_Density = density;
	}

	void Transform(const AuMatrix4x4& m);
};

#endif	// PsNebula_H
