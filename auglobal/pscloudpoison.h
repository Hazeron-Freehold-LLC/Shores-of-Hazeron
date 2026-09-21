//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pscloudpoison.h,v 1.4 2022/02/19 18:31:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsCloudPoison_H
#define PsCloudPoison_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsCloudPoison : public AuParticleSystem
{
public:
	PsCloudPoison(float radius, float gain, GLuint texId);
	virtual ~PsCloudPoison();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsCloudPoison_H
