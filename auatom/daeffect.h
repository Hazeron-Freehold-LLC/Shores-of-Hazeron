//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daeffect.h,v 1.47 2022/04/14 14:32:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DaEffect
// - This atom is not visible on its own; it merely acts as a vessel for sound
// and visual effects, which need to be attached to an atom. Sometimes effects
// must exist in the environment on their own, such as smoke clouds and
// explosions, independent of the atoms that make up the scene.
// - For examples of usage, look at what happens when a vehicle or spacecraft is
// destroyed.
// - Effect atoms usually don't live more than a few seconds so they are not
// stored in the database.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#ifndef DaEffect_H
#define DaEffect_H

#include "daatom.h"

class AUATOM_DLL DaEffect : public DaAtom  
{
public:
	DaEffect();
	virtual ~DaEffect();

protected:
	DaEffect(const DaEffect &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual DaAtom *Clone() const;

	// Return true for atoms that require QueryEnvironment information.
	// Atoms that can have a movement vector must return true.
	virtual bool DoesQueryEnvironment() const
	{
		return true;
	}

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	virtual bool IsEthereal() const
	{
		return true;
	}

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsVisibleNearWorld() const
	{
		return true;
	}

	virtual QString Name() const
	{
		return "Sound/Visual Effect";
	}

	// Don't bother scanning subatoms.
	virtual void SensorScan(AuSensorBlipList& /*blips*/, const AuPoint& /*scannerLoc*/, const AuId& /*scannerId*/,
			quint32 /*scannerEmpire*/, int scannerDomain, const AuId& scannerWorld,
			float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int /*scanContent*/,
			const AuId& /*lockedBlip*/, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& /*stanceCache*/) const
	{
		Q_UNUSED(scannerDomain);
		Q_UNUSED(scannerWorld);
	}

	// This atom is not stored in the database.
	virtual void SetModified(SET_MODIFIED_BITS notifyBits=NOTIFY_All)
	{
		notifyBits = (SET_MODIFIED_BITS)(notifyBits & ~NOTIFY_Database);		// Turn off database bit.
		DaAtom::SetModified(notifyBits);
	}

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual ATOM_TYPE AtomType() const;
};

#endif // DaEffect_H
