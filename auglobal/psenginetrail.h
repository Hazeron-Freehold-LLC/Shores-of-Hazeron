//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psenginetrail.h,v 1.13 2022/02/19 18:32:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PSENGINETRAIL_H
#define PSENGINETRAIL_H

#include "augloballib.h"
#include <auid.h>
#include "auParticleSystem.h"
#include <aumatrix4x4.h>

class AUGLOBAL_DLL PsEngineTrail : public AuParticleSystem
{
public:
	PsEngineTrail();
	~PsEngineTrail();

	// 'parentId' is the atom id of the parent atom of the atom that is generating this contrail.
	// 'parentT' is the atom transform of the atom generating this contrail relative to parentId.
	void SetParent(const AuId& parentId, const AuMatrix4x4& parentT);

	inline void SetSegmentScale(float scale)
	{
		m_SegScale = scale;
	}

	// Overrides
	virtual void InitParticle(auParticle* pParticle);
	virtual void MoveParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

private:
	AuId		m_idParent;			// Id of parent atom.
	float		m_SegScale;			// Scale of trail segments vs particle size returned from GetSize().
	AuMatrix4x4	m_tParent;			// Transformation matrix for the parent of this effect
};

#endif	//PSENGINETRAIL_H
