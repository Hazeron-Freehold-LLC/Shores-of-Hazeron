//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psteleport.cpp,v 1.4 2022/02/19 18:34:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psteleport.h"

PsTeleport::PsTeleport() : AuParticleSystem()
{
	m_bOrigin = false;
	m_SpreadLat = 1.3f;
	m_SpreadLat2 = m_SpreadLat * m_SpreadLat;
	m_SpreadVert = 3.9f;
}

PsTeleport::~PsTeleport()
{

}

bool PsTeleport::HasParticleExpired(const auParticle* pParticle)
{
	float fTimePassed  = GetCurTime() - pParticle->m_fInitTime;
	if(!m_bOrigin && (fTimePassed >= GetLifeCycle() ||
		pParticle->m_vCurPos.z <= GetPosition().z-m_SpreadVert ||
		pParticle->m_vCurPos.z >= GetPosition().z+m_SpreadVert ||
		pParticle->m_vCurPos.Length2_2D() >= m_SpreadLat2))
		return true;
	else if(m_bOrigin && (fTimePassed >= GetLifeCycle() ||
		(pParticle->m_vOldPos.x < 0 && pParticle->m_vCurPos.x > 0) ||	// check x sign change
		(pParticle->m_vOldPos.x > 0 && pParticle->m_vCurPos.x < 0) ||
		(pParticle->m_vOldPos.y < 0 && pParticle->m_vCurPos.y > 0) ||	// check y sign change
		(pParticle->m_vOldPos.y > 0 && pParticle->m_vCurPos.y < 0) ||
		(pParticle->m_vOldPos.z < 0 && pParticle->m_vCurPos.z > 0) ||	// check z sign change
		(pParticle->m_vOldPos.z > 0 && pParticle->m_vCurPos.z < 0)))
		return true;
	return false;
}

void PsTeleport::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsTeleport::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	AuPoint vRandomVec = GetRandomVector();
	if(m_bOrigin)
	{
		AuPoint curPos = vRandomVec*10;
		vRandomVec.x *= m_SpreadLat;
		vRandomVec.y *= m_SpreadLat;
		vRandomVec.z *= m_SpreadVert;
		curPos = vRandomVec;
		vRandomVec.Unit();
		pParticle->m_vCurPos = curPos+GetPosition();
		if(GetVelocityVar() != 0.0f)
			pParticle->m_vCurVel -= vRandomVec * GetVelocityVar();
	}
	else
	{
		if(GetVelocityVar() != 0.0f)
			pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
		pParticle->m_vCurPos = GetPosition() + GetRandomVector()/2;
	}
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2() + 1;	// Particle is killed when it moves farther away.
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}

