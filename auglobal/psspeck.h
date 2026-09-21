//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psspeck.h,v 1.3 2022/02/19 18:32:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsSpeck_H
#define PsSpeck_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsSpeck : public AuParticleSystem
{
	float								m_Density;

public:
	PsSpeck();
	virtual ~PsSpeck();

	// Overrides
	virtual void InitParticle(auParticle* pParticle);
	virtual bool HasParticleExpired(const auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

	inline void SetDensity(float density)
	{
		if(density > 1.f)
			m_Density = 1.f;
		else
			m_Density = density;
	}

	void Transform(const AuMatrix4x4& m);
};

#endif	// PsSpeck_H
