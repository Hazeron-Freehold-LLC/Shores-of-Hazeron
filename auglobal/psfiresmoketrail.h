//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psfiresmoketrail.h,v 1.6 2022/02/19 18:32:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PSFIRESMOKETRAIL_H
#define PSFIRESMOKETRAIL_H

#include "psenginetrail.h"
#include <aumatrix4x4.h>

class AUGLOBAL_DLL PsFireSmokeTrail : public PsEngineTrail
{
public:
	PsFireSmokeTrail();
	~PsFireSmokeTrail();

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	//PSFIRESMOKETRAIL_H
