//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pscloudstun.h,v 1.4 2022/02/19 18:31:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsCloudStun_H
#define PsCloudStun_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsCloudStun : public AuParticleSystem
{
public:
	PsCloudStun(float radius, float gain, GLuint texId);
	virtual ~PsCloudStun();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsCloudStun_H
