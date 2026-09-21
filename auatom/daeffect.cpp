//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daeffect.cpp,v 1.36 2022/04/14 14:32:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daeffect.h"
//
#include "daperson.h"

DaEffect::DaEffect()
{
}

DaEffect::~DaEffect()
{

}

DaEffect::DaEffect(const DaEffect& rhs) : DaAtom(rhs)
{
}

DaAtom* DaEffect::Clone() const
{
	return new DaEffect(*this);
}

void DaEffect::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
}

void DaEffect::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
}

bool DaEffect::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	if(!IsNormal())
		return false;
	Q_UNUSED(sceneAtom);
	m_EffectsLock.lockForRead();
	bool e = m_Effects && !m_Effects->m_Effects.isEmpty();
	if(!e)
	{
		//qDebug() << "DaEffect::IsInScene" << *this << "has no effects";
		m_EffectsLock.unlock();
		return false;
	}

	// Calculate scene atom location in my coordinate space.
	AuPoint sceneLoc = EnvTransform().UnProject(cameraEnvLoc);

	// Determine if any of my effects is close to the scene atom.
	for(QList<AuEffect>::const_iterator ei = m_Effects->m_Effects.constBegin(); ei != m_Effects->m_Effects.constEnd(); ei++)
		if((*ei).IsInScene(sceneLoc))
		{
			m_EffectsLock.unlock();
			return true;
		}
	m_EffectsLock.unlock();

	//qDebug() << "DaEffect::IsInScene" << *this << "has no effects close to scene atom";
	return false;
}

bool DaEffect::IsTicker() const
{
	if(IsDead())
		return false;
	return true;
}

void DaEffect::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// This atom dies when all its effects are gone.
	if(IsNormal())
	{
		m_EffectsLock.lockForRead();
		AuTime effChg(m_tmEffectsChange);
		bool k = !m_Effects || m_Effects->m_Effects.isEmpty();
		m_EffectsLock.unlock();
		if(k && !effChg.IsNull() && effChg.MSecsTo(AuTime::Now()) > 2000)
		{
			// My effects array is empty and I was last used long enough ago that I can die now.
			//qDebug() << "DaEffect::Tick" << Id() << Name() << "no more effects. DYING!";
			Kill();
		}
	}

	// Perform base-level tick processing. That decays the remaining effects over time. When they are gone, this atom can die.
	DaAtom::Tick(last, now);
}

void DaEffect::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	Q_UNUSED(person);
	Q_UNUSED(suns);
	// Calculate scene atom location in my coordinate space.
	m_EffectsLock.lockForRead();
	if(m_Effects)
		m_Effects->GetRenderLights(model, lights);
	m_EffectsLock.unlock();
}

DaAtom::ATOM_TYPE DaEffect::AtomType() const
{
	return ATOM_Effect;
}

