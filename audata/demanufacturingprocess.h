//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: demanufacturingprocess.h,v 1.134 2022/02/19 18:19:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DeManufacturingProcess_H
#define DeManufacturingProcess_H

#include "audata.h"
#include "auid.h"
#include "auspecies.h"
#include "autime.h"
#include "auzoneresource.h"
#include "dbindustry.h"
#include "deprocesscomponent.h"
#include <QDataStream>

using namespace std;
#include <unordered_map>

#define LABOR_PER_SEC					8						// Amount of labor per second applied to manufacturing processes.

class AuItem;
class AuItemHash;
class AuKnownPatents;
class AuScDesign;

class AUDATA_DLL DeManufacturingProcess
{
friend class AuFixDb;
friend class DeManufacturingProcessArray;
private:
	DeProcessComponentArray	m_aComponents;	// Components that affect this manufacturing process.
	bool		m_DNAIsOutput;				// true if the output of this process can store DNA.
	bool		m_DNAIsSentient;			// DNA must be distinguished between animal and sentient.
	AuSpecies	m_DNASpecies;				// Some processes carry DNA with them.
	quint16		m_idProcess;				// Id of process.
	qint32		m_LastQuantity;				// Quantity of last commodity produced. Negative values are error codes.
											// 1+ = count manufactured
											// 0 = none
											// -1 = No empty spawn location.
											// -2 = No animal.
											// -3 = Inventory full.
											// -4 = Ship design not ok to manufacture.
											// -5 = Ship design not analyzed.
											// -11 = No slab found.
											// -12 = No unobstructed slab found.
											// -13 = Spawn location not on the world.
											// -21 = blocked by unanalyzed ship
											// -22 = Cannot spawn another station in orbit or replace an existing station.
											// -23 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
											// -24 = Orbit spawn location could not be found.
											// -25 = No station at which to spawn.
											// -26 = Invention prototype failed.
											// -40 = Cannot spawn ship with no empire.
											// -99 = Unimplemented spawn object
	quint32		m_lLaborNeeded;				// Remaining labor needed to complete running this process.
	qint32		m_lMPID;					// Current manufacturing process Id.
											// m_lMPID is the -designId if a spacecraft.
											// m_lMPID is 0 when the "None" option is selected in a spacecraft factory.
	qint32		m_lProductionBoost;			// Production modifier when GO pushed.
	quint32		m_QtyToProduce;				// Set when run, based on workers enabled at the time.
	quint8		m_QualityReq;				// Miniumum quality required to run this process.
	quint8		m_QualitySet;				// miniumum quality set by the user. Must be >= m_QualityReq.
	QString		m_sCName;					// Name of commodity or ship design
	quint32		m_ShopsEnabled;				// Number of shops assigned to operate this process at a building.
	quint32		m_StockLimit;				// Amount to stock before halting production. 0 = No limit.
	AuTime		m_tmComplete;				// Time this process will finish. Null when not running.
	AuTime		m_tmLastWork;				// For doling work to this process.
	quint8		m_ucLastQuality;			// Quality of last commodity produced.
	quint32		m_WorkersPresent;			// Number of workers present to operate this process at a building.

	// Transient data that is loaded from the database rather than serializing.
	qint32		m_lCID;						// Commodity produced by process, -designId if a spacecraft.
	quint32		m_lProcessLabor;			// Labor overhead needed each time process is run.
	quint32		m_lQtyProduced;				// Quantity produced by invoking process.

public:
	inline DeManufacturingProcess(quint16 idProcess)
	{
		m_DNAIsOutput = false;
		m_DNAIsSentient = false;
		m_idProcess = idProcess;
		m_ShopsEnabled = 1;
		m_StockLimit = 0;
		m_WorkersPresent = 0;
		m_QtyToProduce = 0;
		m_lCID = 0;
		m_lLaborNeeded = 0;
		m_lMPID = 0;
		m_lProcessLabor = 0;
		m_lProductionBoost = 0;
		m_lQtyProduced = 0;
		m_QualityReq = 1;
		m_QualitySet = 1;
		m_ucLastQuality = 0;
		m_LastQuantity = 0;
	}

	inline DeManufacturingProcess()
	{
		m_DNAIsOutput = false;
		m_DNAIsSentient = false;
		m_idProcess = 0;
		m_ShopsEnabled = 1;
		m_StockLimit = 0;
		m_WorkersPresent = 0;
		m_QtyToProduce = 0;
		m_lCID = 0;
		m_lLaborNeeded = 0;
		m_lMPID = 0;
		m_lProcessLabor = 0;
		m_lProductionBoost = 0;
		m_lQtyProduced = 0;
		m_QualityReq = 1;
		m_QualitySet = 1;
		m_ucLastQuality = 0;
		m_LastQuantity = 0;
	}

	inline DeManufacturingProcess(const DeManufacturingProcess &c) : m_aComponents(c.m_aComponents)
	{
		m_DNAIsOutput = c.m_DNAIsOutput;
		m_DNAIsSentient = c.m_DNAIsSentient;
		m_DNASpecies = c.m_DNASpecies;
		m_idProcess = c.m_idProcess;
		m_ShopsEnabled = c.m_ShopsEnabled;
		m_StockLimit = c.m_StockLimit;
		m_WorkersPresent = c.m_WorkersPresent;
		m_QtyToProduce = c.m_QtyToProduce;
		m_lCID = c.m_lCID;
		m_lLaborNeeded = c.m_lLaborNeeded;
		m_lMPID = c.m_lMPID;
		m_lProcessLabor = c.m_lProcessLabor;
		m_lProductionBoost = c.m_lProductionBoost;
		m_lQtyProduced = c.m_lQtyProduced;
		m_sCName = c.m_sCName;
		m_QualityReq = c.m_QualityReq;
		m_QualitySet = c.m_QualitySet;
		m_tmComplete = c.m_tmComplete;
		m_tmLastWork = c.m_tmLastWork;
		m_ucLastQuality = c.m_ucLastQuality;
		m_LastQuantity = c.m_LastQuantity;
	}

	inline DeManufacturingProcess& operator=(const DeManufacturingProcess &c)
	{
		m_aComponents = c.m_aComponents;
		m_DNAIsOutput = c.m_DNAIsOutput;
		m_DNAIsSentient = c.m_DNAIsSentient;
		m_DNASpecies = c.m_DNASpecies;
		m_idProcess = c.m_idProcess;
		m_ShopsEnabled = c.m_ShopsEnabled;
		m_StockLimit = c.m_StockLimit;
		m_WorkersPresent = c.m_WorkersPresent;
		m_QtyToProduce = c.m_QtyToProduce;
		m_lCID = c.m_lCID;
		m_lLaborNeeded = c.m_lLaborNeeded;
		m_lMPID = c.m_lMPID;
		m_lProcessLabor = c.m_lProcessLabor;
		m_lProductionBoost = c.m_lProductionBoost;
		m_lQtyProduced = c.m_lQtyProduced;
		m_sCName = c.m_sCName;
		m_QualityReq = c.m_QualityReq;
		m_QualitySet = c.m_QualitySet;
		m_tmComplete = c.m_tmComplete;
		m_tmLastWork = c.m_tmLastWork;
		m_ucLastQuality = c.m_ucLastQuality;
		m_LastQuantity = c.m_LastQuantity;
		return *this;
	}

	// Returns the actual manufacturing cost based on a quality-based cost
	// for all components that have been fetched.
	// Assumes all commodities have been satisfied; does not verify that.
	float ActualCost() const;

	// Only called using the item contained in a DaItem atom, or in the contents of a DaItem atom's container.
	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	void ApplyFetchedMaterial(AuItem& ci, quint8 elecSubQ, quint8 waterSubQ);

	// Returns true if anything changes.
	inline bool ApplyNaturalResources(const AuZoneResource &res)
	{
		bool anyChange;
		m_lLaborNeeded += m_aComponents.ApplyNaturalManufacturingResources(res, anyChange, m_WorkersPresent);
		return anyChange;
	}

	inline bool CanRun() const
	{
		// Can't run a process that is already running.
		if(IsRunning())
			return false;

		// Can't run a process with no manufacturing process id.
		// This is probably the "None" option in a spacecraft factory.
		if(m_lMPID == 0)
			return false;

		// Check to see if all required components are present.
		if(!m_aComponents.IsCompleted())
		{
			//qDebug("DeManufacturingProcess::CanRun - Components incomplete "
					//"to produce CID %d Process %d", m_lCID, m_idProcess);
			return false;
		}
		return true;
	}

	// Used by AuScDesign to prevent this process from being run when it represents a spacecraft design that is not final.
	inline void ClearManufacturingProcessId()
	{
		m_lMPID = 0;
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_lCID;
	}

	inline const QString &CommodityName() const
	{
		return m_sCName;
	}

	inline const DeProcessComponentArray &Components() const
	{
		return m_aComponents;
	}

	inline DeProcessComponentArray &Components()
	{
		return m_aComponents;
	}

	inline void ConsumeComponents()
	{
		m_aComponents.ConsumeAppliedInventory();
	}

	void DemandTally(const AuItem& item, quint32& needCount, quint32& produceCount) const;

	// Can be null.
	inline const AuSpecies& DNASpecies() const
	{
		return m_DNASpecies;
	}

	inline bool DNAIsOutput() const
	{
		return m_DNAIsOutput;
	}

	inline bool DNAIsSentient() const
	{
		return m_DNAIsSentient;
	}

	inline bool DNAIsViable() const
	{
		switch(CommodityId())
		{
		default:
			// Not many things keep their DNA when manufactured.
			break;

		// These items contain animal DNA.
		case CID_Animal:
		case CID_AnimalMeat:
		case CID_Brain:
		case CID_Bone:					// NOTE: This creates a bone with viable DNA from a head with unviable DNA.
		case CID_Cheese:
		//case CID_DNASample:			// Cannot be manufactured.
		case CID_Eggs:
		case CID_Fish:
		case CID_FishMeat:
		case CID_Gills:
		//case CID_Head:				// Cannot be manufactured.
		case CID_Heart:
		//case CID_Leather:				// DNA lost in the process.
		//case CID_LeatherBodyArmor:	// DNA lost in the process.
		//case CID_LeatherClothes:		// DNA lost in the process.
		//case CID_LeatherHelmet:		// DNA lost in the process.
		case CID_Lungs:
		case CID_Milk:
		case CID_Stomach:
			return !m_DNASpecies.IsNull();
		}
		return false;
	}

	void SetShopsEnabled(quint32 shops);

	void SetStockLimit(quint32 stockLimit);

	void SetWorkersPresent(quint32 workers);

	// Returns the estimated manufacturing cost based on a quality-based cost for all components.
	float EstimatedCost(quint8 quality) const;

	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	// 'power' is the amount of electricity units available that are not stored in 'inventory'. It is used first. It is updated if
	// used.
	// returns true if any materials are taken from the inventory or tools or power.
	bool FetchMaterials(AuItemHash& inventory, AuItemHash& tools, const AuKnownPatents& patents, quint8 elecSubQ, quint8 waterSubQ,
			quint32& power);

	// Satisfy any needs for vehicles from the ones listed. These are the vehicles available to the process. Vehicles are never
	// consumes; they just modify processes.
	// Returns true if this process changes.
	bool FetchVehicles(const QList<COMMODITYID>& vehiclesAvailable);

	// Sets m_ucLastQuality, which can be queried with LastOutputQuality()
	// Sets m_LastQuantity, which can be queried with LastOutputQuantity()
	void FinishOutput(const AuZoneResource& naturalResourcesPresent, const AuKnownPatents& patents);

	// Quality of fuel component added.
	inline quint8 FuelQuality() const
	{
		return m_aComponents.FuelQuality();
	}

	// Return true if this process is manufacturing something that is critical to the survival of the city.
	bool IsCriticalToSurvival(bool planetCanHaveFauna) const;

	inline void IncreaseLabor(quint32 labor)
	{
		m_lLaborNeeded += labor;
	}

	void InitIndustry(INDUSTRYID lIID, const AuKnownPatents& patents);

	// Init spacecraft manufacturing process for AuScDesign design.
	// 'designQ' is the minimum quality required to manufacture the design.
	bool InitSc(quint32 designId, quint8 designQ);

	// 'designQ' is the minimum quality required to manufacture the design.
	void InitSc(const AuScDesign& design, quint8 designQ);

	inline bool IsMakingUnit() const
	{
		// Negative manufacturing process id indicates making a spacecraft.
		if(m_lMPID < 0)
			return true;
		if(m_lMPID == 0)
			return false;	// Not making anything.

		// Note that CID_Animal is not considered to be making a unit because it doesn't make one every time.
		switch(CommodityId())
		{
		case CID_DirtBike:
		case CID_SailBoat:
		case CID_Helicopter:
		case CID_SpaceFighter:
		case CID_AdvSpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		case CID_FarmMachinery:
		case CID_MineMachinery:
		case CID_CommercialShip:
		case CID_NavalShip:
		case CID_Submarine:
		case CID_SUV:
		case CID_Tank:
		case CID_APC:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsRnD() const
	{
		return DbCommodity::IsPatent(CommodityId());
	}

	inline bool IsRunning() const
	{
		return !m_tmComplete.IsNull();
	}

	quint64 MemoryUsage() const;

	inline quint32 ShopsEnabled() const
	{
		return m_ShopsEnabled;
	}

	inline quint32 StockLimit() const
	{
		return m_StockLimit;
	}

	inline quint32 WorkersPresent() const
	{
		return m_WorkersPresent;
	}

	bool IsValid() const;

	inline quint32 LaborNeeded() const
	{
		return m_lLaborNeeded;
	}

	inline quint8 LastOutputQuality() const
	{
		return m_ucLastQuality;
	}

	// Quantity of last commodity produced. Negative values are error codes. See above doc on m_LastQuantity.
	inline qint32 LastOutputQuantity() const
	{
		return m_LastQuantity;
	}

	inline quint8 MinimumQRequired() const
	{
		return m_QualityReq;
	}

	inline quint8 MinimumQSet() const
	{
		return m_QualitySet;
	}

	inline const AuTime& LastWorked() const
	{
		return m_tmLastWork;
	}

	inline qint32 ManufacturingProcessId() const
	{
		return m_lMPID;
	}

	inline bool NeedsComponents() const	// True if materials are needed to start.
	{
		return !m_aComponents.IsCompleted();
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline quint16 ProcessId() const
	{
		return m_idProcess;
	}

	inline void Reset()
	{
		ConsumeComponents();

		m_tmComplete.SetNull();

		// Reset labor so panel shows base labor needed.
		m_lLaborNeeded = m_lProcessLabor;
		m_QtyToProduce = 0;
	}

	void Run(const AuTime& start, unsigned int laborBoost, int productionBoost);

	void RunHandCraft(const AuKnownPatents& patents);

	// Time to run based on standard unboosted labor rate and current labor needed, which grows as commodities are fetched.
	unsigned int SecondsAccumulated() const;

	// Time to run based on standard unboosted labor rate and all labor needed when all commodities are fetched.
	unsigned int SecondsEstimated() const;

	// Uses UTC time sync'd with the server for best accuracy.
	unsigned int SecondsRemaining() const;

	// Specifically for 'handcrafting', not 'manufacturing'.
	// The resulting process consumption and output are not multipled by a manufacturing multiplier.
	inline void SetHandCraftingProcessId(qint32 mpid)
	{
		m_lMPID = mpid;
		InitDbData(false);
	}

	// 'lastQty' is an error code when negative. See above doc on m_LastQuantity.
	inline void SetLastOutput(qint32 lastQty, quint8 quality)
	{
		m_LastQuantity = lastQty;
		m_ucLastQuality = quality;
	}

	// Specifically for 'manufacturing', not 'handcrafting'.
	// The resulting process consumption and output are multipled by a bulk manufacturing multiplier.
	inline void SetManufacturingProcessId(qint32 mpid)
	{
		m_lMPID = mpid;
		InitDbData(true);
	}

	inline void SetMinimumQ(quint8 quality)
	{
		Q_ASSERT(quality);
		if(quality >= m_QualityReq)
			m_QualitySet = quality;
	}

	void SetSpecies(const AuSpecies& species, bool isSentient);

	// 'elecSubQ' is non-zero if it is permitted to substitute electricity for the sunlight requirement.
	// This is done in farming processes to simulate the effect of having artificial lighting.
	// Returns true if components change.
	inline bool SupplyAllPower(quint8 elecSubQ)
	{
		return m_aComponents.SupplyAllPower(elecSubQ);
	}

	// This might seem odd but cities pass us their account balance, which is a double.
	// No process uses billions of cronos so the return result can be an int.
	inline quint32 SupplyMoney(double money)
	{
		Q_ASSERT(money >= 0);
		quint32 moneyTaken = m_aComponents.SupplyMoney(money);
		//m_lLaborNeeded += moneyTaken * LABOR_PER_SEC;	// Add 1 second per crono applied.
		return moneyTaken;
	}

	inline bool Tick()
	{
		Q_ASSERT(IsRunning());	// Check this before calling.
		if(AuTime::Now() < m_tmComplete)
			return false;		// Process is not complete yet.

		// This process is complete.
		m_tmComplete.SetNull();

		// Reset labor so panel shows base labor needed.
		m_lLaborNeeded = m_lProcessLabor;
		return true;
	}

	inline void Worked(const AuTime& when)
	{
		m_tmLastWork = when;
	}

protected:
	void InitDbData(bool doManufMult);

	inline qint32 QtyProduced() const
	{
		// Quantity produced by running this process.
		if(m_lProductionBoost > 0)
		{
			// Production boost is treated as 1=10%, min +1.
			int iqty = m_QtyToProduce;
			int prodinc = m_QtyToProduce * m_lProductionBoost / 10;
			if(prodinc < m_lProductionBoost)
				iqty += m_lProductionBoost;
			else
				iqty += prodinc;
			if(iqty < 0)
				return iqty;
			return iqty;
		}
		else
			return (qint32)m_QtyToProduce;
	}
};

inline QDataStream& operator<<(QDataStream& out, const DeManufacturingProcess& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeManufacturingProcess& o)
{
	o << in;
	return in;
}

// I'm not sure why this is needed. The build started failing when I added AUDATA_DLL to export the class. The error
// indicated that it couldn't find a qHash function for a the class, hmm. - D.H.H Wed Dec 5, 2012
inline unsigned int qHash(const DeManufacturingProcess& key)
{
	Q_ASSERT(false);					// Not a unique hash result. Does it need to be unique?
	return key.ManufacturingProcessId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DeManufacturingProcessArray : public list<DeManufacturingProcess>
{
public:	// Operators
	AUDATA_DLL void operator<<(QDataStream& in);

	AUDATA_DLL void operator>>(QDataStream& out) const;

public:	// Methods
	AUDATA_DLL quint16 AvailableProcessId() const;

	AUDATA_DLL bool DeleteMPID(quint16 mpid);

	AUDATA_DLL iterator FindMPID(quint16 mpid);

	AUDATA_DLL DeManufacturingProcess& FindProcess(int index);

	AUDATA_DLL const DeManufacturingProcess& FindProcess(int index) const;

	AUDATA_DLL quint64 MemoryUsage() const;

	// Changes the index of a process in the list.
	AUDATA_DLL void MoveProcess(int fromListIndex, int toListIndex);

	// Returns -1 if 'mpid' is not found.
	AUDATA_DLL int MPIDIndex(quint16 mpid) const;

	AUDATA_DLL quint32 ShopsEnabled() const;

	AUDATA_DLL quint32 WorkersPresent() const;
};

inline QDataStream& operator<<(QDataStream& out, const DeManufacturingProcessArray& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeManufacturingProcessArray& o)
{
	o << in;
	return in;
}

#endif // DeManufacturingProcess_H
