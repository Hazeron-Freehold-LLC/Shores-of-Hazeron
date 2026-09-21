//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psfireelectric.h,v 1.4 2022/02/19 18:32:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsFireElectric_H
#define PsFireElectric_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsFireElectric : public AuParticleSystem
{
	float								m_HaloSize;

public:
	// radius is how far the lightning shoots from center.
	// width is how wide the lightning is.
	// haloSize is the size of the big glare spot.
	PsFireElectric(float radius, float width, float haloSize, GLuint texId, GLuint texIdGlow);
	virtual ~PsFireElectric();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsFireElectric_H
