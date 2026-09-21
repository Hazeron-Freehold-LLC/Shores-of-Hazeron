//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deconstructionprocess.h,v 1.46 2022/02/19 18:19:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DeConstructionProcess_H
#define DeConstructionProcess_H

#include "audata.h"
#include "deprocesscomponent.h"
#include <QDataStream>

class AuBdDesign;
class AuItem;
class AuItemHash;
class AuZoneResource;

// Construction processes used to relate to a record in the database, hence the odd spread of numbers. Do not change them because
// they get stored somewhere.
enum CONSTRUCTIONPROCESSID
{
	CPID_Building = 0,				// Most construction processes make a building.
	CPID_DirtRoad = 2,				// Geo road
	CPID_AsphaltRoad = 3,			// Geo road
	CPID_ConcreteRoad = 4,			// Geo road
	CPID_RoadGrade = 5,				// Geo road
	CPID_FencePosts = 10,			// Fence bd. Processes correspond to AuScFence::STYLE.
	CPID_FencePostsT,				// Fence bd
	CPID_FenceWire,					// Fence bd
	CPID_FenceWireT,				// Fence bd
	CPID_FenceRange,				// Fence bd
	CPID_FenceRangeT,				// Fence bd
	CPID_FenceChainLink,			// Fence bd
	CPID_FenceChainLinkRazorWire,	// Fence bd
	CPID_FenceSplitRail,			// Fence bd
	CPID_FenceWoodYard,				// Fence bd
	CPID_FenceSandBagWall,			// Fence bd
	CPID_FenceBrickWall,			// Fence bd
	CPID_FenceStoneWall,			// Fence bd
	CPID_FenceCustom,				// Fence bd using custom fence blueprint.
	CPID_Irrigation = 74,			// Geo irrigation
	CPID_Defoliant = 80,			// Geo defoliant
	CPID_Clearing = 81,				// Geo clearing
};

// This class contains information needed during the construction of a building.
class AUDATA_DLL DeConstructionProcess  
{
public:
	enum CONPHASE		// Phases of construction.
	{
		CP_Stakes,						// Stakes only. Construction hasn't started yet.
		CP_UnderConstruction,			// Construction started.
		CP_Halted,						// Construction halted after starting.
		CP_HaltedStakes,				// Construction halted before starting.
	};

private:
	quint32					m_DesignId;		// Bd design id, when m_lCPID == 0.
	CONPHASE				m_ePhase;		// Phase of construction.
	CONSTRUCTIONPROCESSID	m_lCPID;		// Construction process Id from database.
	quint32					m_lLaborNeeded;	// Labor needed to finish construction.
	DeProcessComponentArray	m_aComponents;	// Components that affect this construction process.
	quint16					m_MPID;			// Manufacturing process to make when construction is complete
	bool					m_WorkerOk;		// true to enable worker in manufacturing process when construction is complete

public:	// Constructors
	DeConstructionProcess();
	DeConstructionProcess(CONSTRUCTIONPROCESSID lCPID, quint32 sitePrepLabor, quint32 paveAmount);	// Geo road
	DeConstructionProcess(const DeConstructionProcess &c);
	virtual ~DeConstructionProcess();

public:	// Operators
	const DeConstructionProcess& operator=(const DeConstructionProcess& c);

	inline bool operator==(const DeConstructionProcess& c) const
	{
		if(m_DesignId != c.m_DesignId)
			return false;
		if(m_ePhase != c.m_ePhase)
			return false;
		if(m_lCPID != c.m_lCPID)
			return false;
		if(m_lLaborNeeded != c.m_lLaborNeeded)
			return false;
		if(m_MPID != c.m_MPID)
			return false;
		if(m_WorkerOk != c.m_WorkerOk)
			return false;
		if(m_aComponents != c.m_aComponents)
			return false;
		return true;
	}

	inline bool operator!=(const DeConstructionProcess& c) const
	{
		return !operator==(c);
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:	// Methods
	inline void AddLabor(quint32 labor)
	{
		if(!labor)
			return;
		quint32 nulabor = m_lLaborNeeded + labor;
		if(nulabor > m_lLaborNeeded)
			m_lLaborNeeded = nulabor;		// labor did not roll over
		else
			m_lLaborNeeded = 0xffffffff;	// labor rolled over so set it to the max 32 bit unsigned int
	}

	quint32 ApplyConstructionLabor(quint32 labor);	// Returns amount of labor actually applied, if any was needed.

	// Only called using the item contained in a DaItem atom, or in the contents of a DaItem atom's container.
	void ApplyFetchedMaterial(AuItem& ci);

	void ApplyNaturalResources(const AuZoneResource& res);

	void ChangeMaterial(COMMODITYID idCommodity, COMMODITYID idNewCommodity);

	// Called by designer after InitBd on unfinal design, to insure this process is not valid for construction.
	inline void ClearDesignId()
	{
		m_DesignId = 0;
	}

	inline DeProcessComponentArray& Components()
	{
		return m_aComponents;
	}

	inline const DeProcessComponentArray& Components() const
	{
		return m_aComponents;
	}

	inline CONSTRUCTIONPROCESSID ConstructionProcessId() const
	{
		return m_lCPID;
	}

	quint32 DemandTally(const AuItem& item) const;

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	// returns true if any materials are taken from the inventory or tools
	bool FetchMaterials(AuItemHash& inventory, AuItemHash& tools);

	// Satisfy any needs for vehicles from the ones listed. These are the vehicles available to the process. Vehicles are never
	// consumes; they just modify processes.
	// Returns true if this process changes.
	bool FetchVehicles(const QList<COMMODITYID>& vehiclesAvailable);

	void Halt();

	void InitBd(const AuBdDesign& design);

	void InitGeo(quint32 paveAmount);

	bool IsComplete() const;	// true if all labor and materials have been applied.

	inline bool IsHalted() const
	{
		return m_ePhase == CP_Halted || m_ePhase == CP_HaltedStakes;
	}

	// Returns true if 'cid' is required by this process, whether or not it is consumed by the process or if the total demand is
	// currently satisfied.
	bool IsRequired(COMMODITYID cid) const;

	inline quint32 LaborNeeded() const
	{
		return m_lLaborNeeded;
	}

	inline quint16 ManufacturingProcessId() const
	{
		return m_MPID;
	}

	inline bool ManufacturingProcessWorkerEnabled() const
	{
		return m_WorkerOk;
	}

	quint64 MemoryUsage() const;

	inline bool NeedsComponents() const	// True if materials are needed to start.
	{
		return !m_aComponents.IsCompleted();
	}

	// Returns an estimate of the level of completion of this process.
	unsigned int PercentComplete() const;

	// Not valid until construction is complete.
	quint8 QualityOfConstruction() const;

	void Resume();	

	void Rx(QDataStream& in);

	// Time to run based on standard unboosted labor rate and all labor needed when all commodities are fetched.
	unsigned int SecondsEstimated() const;

	inline void SetLabor(quint32 labor)
	{
		m_lLaborNeeded = labor;
	}

	inline void SetMPID(quint16 mpid)
	{
		m_MPID = mpid;
	}

	inline void SetWorkerOk(bool workerOk)
	{
		m_WorkerOk = workerOk;
	}

	// Returns true if components change.
	inline bool SupplyAllPower()
	{
		return m_aComponents.SupplyAllPower(0);
	}

	void Tx(QDataStream& out) const;

	// Returns true if any component is not satisfied, even optional components.
	inline bool WantsAnyComponent() const
	{
		return m_aComponents.WantsAnyComponent();
	}
};

inline QDataStream& operator<<(QDataStream& out, const DeConstructionProcess& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeConstructionProcess& o)
{
	o << in;
	return in;
}

#endif // DeConstructionProcess_H
