//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesignbase.h,v 1.24 2022/05/17 15:57:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaDesignBase_H
#define DaDesignBase_H

#include "daatom.h"

class QStringList;

class AUATOM_DLL DaDesignBase : public DaAtom  
{
public:
	enum DESIGNMODE
	{
		InvalidMode,
		DESIGN_Unknown,					// Initial state on entry to the new design environment. User must select a mode.
		DESIGN_Spacecraft,				// Spacecraft designer.
		DESIGN_Building,				// Building designer.
		DESIGN_Vehicle,					// Vehicle designer.
		DESIGN_Item,					// Item designer.
	};

private:
	qint8								m_DesignMode;			// DESIGNMODE
	quint16								m_StudioId;				// Panel Id that created this design studio.

public:
	DaDesignBase(quint16 studioId, DESIGNMODE mode);
	DaDesignBase();
	virtual ~DaDesignBase();

protected:
	DaDesignBase(const DaDesignBase &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual bool CanRenderEnvironment() const
	{
		return true;
	}

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	// The design id loaded/saved last.
	// Not the live design id of the ship or building being edited.
	virtual quint32 DbDesignId() const = 0;

	inline DESIGNMODE DesignMode() const
	{
		return (DESIGNMODE)m_DesignMode;
	}

	virtual QString DesignArchitect() const = 0;

	virtual QString DesignName() const = 0;

	virtual QString EntryPassword() const = 0;

	virtual DaBd* FindParentBdInEnv();

	virtual const DaBd* FindParentBdInEnv() const;

	virtual DaCapsule* FindParentCapsuleInEnv();

	virtual const DaCapsule* FindParentCapsuleInEnv() const;

	virtual DaDesignBase* FindParentDesignBaseInEnv();

	virtual const DaDesignBase* FindParentDesignBaseInEnv() const;

	virtual DaDesigner* FindParentDesignerInEnv();

	virtual const DaDesigner* FindParentDesignerInEnv() const;

	virtual DaWorld* FindParentPlanetInEnv();

	virtual const DaWorld* FindParentPlanetInEnv() const;

	virtual DaSolarSystem* FindParentSolarSystemInEnv();

	virtual const DaSolarSystem* FindParentSolarSystemInEnv() const;

	virtual DaStar* FindParentStarInEnv();

	virtual const DaStar* FindParentStarInEnv() const;

	virtual DaVessel* FindParentVesselInEnv();

	virtual const DaVessel* FindParentVesselInEnv() const;

	virtual DaWorldLL* FindParentWorldInEnv();

	virtual const DaWorldLL* FindParentWorldInEnv() const;

	virtual void GetAtomEnvironment(AuQueryEnvironment& qe, const DaAtom* atom) const = 0;

	virtual double GetDesignPlaneGridSpacing(const AuId& idArchitect) const = 0;

	virtual AuMatrix4x4 GetDesignPlaneTransform(const AuId& idArchitect) const = 0;

	// Collects lights from all scene atoms with a light that is on.
	// 'person' is the person for whom the scene is rendered.
	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	virtual short int GridOnBits(const AuId& idArchitect) const = 0;

	virtual void InitDesignPlane(const AuId& idArchitect) = 0;

	virtual bool IsDesignModified() const = 0;

	virtual bool IsEthereal() const
	{
		return true;
	}

	virtual bool IsGravityOn(const AuId& idArchitect) const = 0;

	// Instance universe's prevent some top-down recursive searches from entering the instance.
	virtual bool IsInstanceUniverse() const
	{
		return true;
	}

	virtual bool IsPreview() const
	{
		return false;
	}

	virtual bool IsRenderSolid(const AuId& idArchitect) const = 0;

	virtual bool IsTicker() const;

	virtual quint64 MemoryUsage() const;

	virtual AuId Owner() const = 0;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Scanners do not detect anything inside a design studio.
	virtual void SensorScan(AuSensorBlipList& /*blips*/, const AuPoint& /*scannerLoc*/, const AuId& /*scannerId*/,
			quint32 /*scannerEmpire*/, int scannerDomain, const AuId& scannerWorld,
			float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int /*scanContent*/,
			const AuId& /*lockedBlip*/, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& /*stanceCache*/) const
	{
		Q_UNUSED(scannerDomain);
		Q_UNUSED(scannerWorld);
	}

	void SetupLights(DaPerson* sceneAtom, const AuMatrix4x4& model, AuGLEffects& re);

	inline quint16 StudioId() const
	{
		return m_StudioId;
	}

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, 
			const DaPerson& sceneAtom);

protected:
	inline void SetDesignMode(DESIGNMODE designMode)
	{
		m_DesignMode = designMode;
	}

	inline void SetStudioId(quint16 studioId)
	{
		m_StudioId = studioId;
	}
};

#endif // DaDesignBase_H
