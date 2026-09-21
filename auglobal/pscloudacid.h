//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pscloudacid.h,v 1.4 2022/02/19 18:31:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PsCloudAcid_H
#define PsCloudAcid_H

#include "auParticleSystem.h"

class AUGLOBAL_DLL PsCloudAcid : public AuParticleSystem
{
public:
	PsCloudAcid(float radius, float gain, GLuint texId);
	virtual ~PsCloudAcid();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	// PsCloudAcid_H
