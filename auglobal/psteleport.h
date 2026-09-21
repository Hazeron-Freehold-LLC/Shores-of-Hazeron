//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psteleport.h,v 1.5 2022/02/19 18:32:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PSTELEPORT_H
#define PSTELEPORT_H

#include "auParticleSystem.h"
#include <aumatrix4x4.h>

class AUGLOBAL_DLL PsTeleport : public AuParticleSystem
{
public:
	PsTeleport();
	virtual ~PsTeleport();

	inline void SetOrigin(bool bOrigin)
	{
		m_bOrigin = bOrigin;
	}

	void SetSpread(float vertical, float lateral)
	{
		m_SpreadLat = lateral;
		m_SpreadLat2 = m_SpreadLat * m_SpreadLat;
		m_SpreadVert = vertical;
	}

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual bool HasParticleExpired(const auParticle* pParticle);
private:
	bool		m_bOrigin;			// True if origin, false if destination
	float		m_SpreadLat;		// Lateral limit of spread of particles.
	float		m_SpreadLat2;		// Squared lateral limit of spread of particles.
	float		m_SpreadVert;		// Vertical limit of spread of particles.
};

#endif	//PSTELEPORT_H
