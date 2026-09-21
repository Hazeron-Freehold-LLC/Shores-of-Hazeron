//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psentity.cpp,v 1.3 2022/02/19 18:33:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "psentity.h"
//

PsEntity::PsEntity() : AuParticleSystem()
{
}

PsEntity::~PsEntity()
{

}

bool PsEntity::HasParticleExpired(const auParticle* pParticle)
{
	float fTimePassed  = GetCurTime() - pParticle->m_fInitTime;
	if(fTimePassed >= GetLifeCycle() ||
			(pParticle->m_vOldPos.x < 0 && pParticle->m_vCurPos.x > 0) ||	// check x sign change
			(pParticle->m_vOldPos.x > 0 && pParticle->m_vCurPos.x < 0) ||
			(pParticle->m_vOldPos.y < 0 && pParticle->m_vCurPos.y > 0) ||	// check y sign change
			(pParticle->m_vOldPos.y > 0 && pParticle->m_vCurPos.y < 0) ||
			(pParticle->m_vOldPos.z < 0 && pParticle->m_vCurPos.z > 0) ||	// check z sign change
			(pParticle->m_vOldPos.z > 0 && pParticle->m_vCurPos.z < 0))
		return true;
	return false;
}

void PsEntity::MoveParticle(auParticle* pParticle)
{
	// Update position with respect to velocity
	pParticle->m_vOldPos = pParticle->m_vCurPos;
	pParticle->m_vCurPos += pParticle->m_vCurVel * GetElapsedTime();
}

void PsEntity::InitParticle(auParticle* pParticle)
{
	pParticle->m_vCurVel = GetVelocity();
	AuPoint vRandomVec = GetRandomVector();
	if(GetVelocityVar() != 0.0f)
	{
		pParticle->m_vCurVel += vRandomVec * GetVelocityVar();
		vRandomVec = GetRandomVector() * GetVelocityVar();
	}
	pParticle->m_vCurPos = GetPosition() + vRandomVec/2;
	pParticle->m_InitDist	= pParticle->m_vCurPos.Length2() + 1;	// Particle is killed when it moves farther away.
	pParticle->m_fInitTime  = GetCurTime();
	pParticle->m_vOldPos	= pParticle->m_vCurPos;
}
