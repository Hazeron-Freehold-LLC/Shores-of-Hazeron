//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: pswormhole.h,v 1.7 2022/02/19 18:32:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PSWORMHOLE_H
#define PSWORMHOLE_H

#include "aumatrix4x4.h"
#include "auParticleSystem.h"

class AUGLOBAL_DLL PsWormhole : public AuParticleSystem
{
public:
	PsWormhole();
	~PsWormhole();

	// Inline functions
	void SetOrigin(bool bOrigin)
		{ m_bOrigin = bOrigin; }
	bool GetOrigin()
		{ return m_bOrigin; }
	void SetScale(float fScale)
		{ m_fScale = fScale; }
	float GetScale()
		{ return m_fScale; }
	void SetHeight(float fHeight)
		{ m_fHeight = fHeight; }
	float GetHeight()
		{ return m_fHeight; }

	// Overrides
	virtual void InitParticle(auParticle* pParticle);
	virtual void MoveParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

private:
	bool		m_bOrigin;			// True if origin, false if destination
	float		m_fScale;
	float		m_fHeight;
};

#endif	//PSWORMHOLE_H
