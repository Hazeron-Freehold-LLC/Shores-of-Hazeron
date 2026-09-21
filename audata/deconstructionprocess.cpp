//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deconstructionprocess.cpp,v 1.70 2022/02/19 18:20:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "deconstructionprocess.h"
//
#include "aubddesign.h"
#include "auglobal.h"
#include "auitem.h"
#include "auzoneresource.h"
#include "dgdata.h"

#define LABOR_PER_SEC					4						// Amount of labor per second applied to construction processes.

DeConstructionProcess::DeConstructionProcess(CONSTRUCTIONPROCESSID lCPID, quint32 sitePrepLabor, quint32 paveAmount)
{
	m_DesignId = 0;
	m_lCPID = lCPID;
	m_lLaborNeeded = sitePrepLabor;
	m_MPID = 0;
	m_WorkerOk = false;
	InitGeo(paveAmount);
}

DeConstructionProcess::DeConstructionProcess()
{
	m_DesignId = 0;
	m_ePhase = CP_Stakes;
	m_lCPID = CPID_Building;
	m_lLaborNeeded = 10;
	m_MPID = 0;
	m_WorkerOk = true;
}

DeConstructionProcess::DeConstructionProcess(const DeConstructionProcess &c) : m_aComponents(c.m_aComponents)
{
	m_DesignId = c.m_DesignId;
	m_ePhase = c.m_ePhase;
	m_lCPID = c.m_lCPID;
	m_lLaborNeeded = c.m_lLaborNeeded;
	m_MPID = c.m_MPID;
	m_WorkerOk = c.m_WorkerOk;
}

const DeConstructionProcess& DeConstructionProcess::operator=(const DeConstructionProcess& c)
{
	m_aComponents = c.m_aComponents;
	m_DesignId = c.m_DesignId;
	m_ePhase = c.m_ePhase;
	m_lCPID = c.m_lCPID;
	m_lLaborNeeded = c.m_lLaborNeeded;
	m_MPID = c.m_MPID;
	m_WorkerOk = c.m_WorkerOk;
	return *this;
}

DeConstructionProcess::~DeConstructionProcess()
{

}

// This function is only ever called on the contents of DaItem objects lying about, not on city inventories.
void DeConstructionProcess::ApplyFetchedMaterial(AuItem& ci)
{
	// No construction process uses sunlight so no need to enable substitution.
	AddLabor(m_aComponents.ApplyInventoryMaterial(ci, 0, 0, 0));
}

void DeConstructionProcess::ApplyNaturalResources(const AuZoneResource& res)
{
	AddLabor(m_aComponents.ApplyNaturalConstructionResources(res));
}

// Returns amount of labor actually applied, if any was needed.
quint32 DeConstructionProcess::ApplyConstructionLabor(quint32 labor)
{
	if(m_lLaborNeeded <= labor)
	{
		labor = m_lLaborNeeded;
		m_lLaborNeeded = 0;
	}
	else
		m_lLaborNeeded -= labor;
	return labor;
}

// true if all labor and materials have been applied.
bool DeConstructionProcess::IsComplete() const
{
	if (m_lLaborNeeded > 0)
		return false;			// Not done if labor is still needed.

	if (!m_aComponents.IsCompleted())
		return false;			// Not done if all components have not been applied.

	// All labor and components must have been applied so this process is completed.
	return true;
}

void DeConstructionProcess::InitGeo(quint32 paveAmount)
{
	m_ePhase = CP_Stakes;
	m_aComponents.clear();

	// This information used to come from a database table but it was reduced to these few remaining road building processes.
	switch(m_lCPID)
	{
	case CPID_AsphaltRoad:
		m_aComponents.AddRequirement(CID_CrudeOil, CE_Required, paveAmount);
		break;
	case CPID_ConcreteRoad:
		m_aComponents.AddRequirement(CID_Stone, CE_Required, paveAmount);
		break;
	case CPID_Building:
	case CPID_Clearing:
	case CPID_Defoliant:
	case CPID_DirtRoad:
	case CPID_FenceBrickWall:
	case CPID_FenceChainLink:
	case CPID_FenceChainLinkRazorWire:
	case CPID_FenceCustom:
	case CPID_FencePosts:
	case CPID_FencePostsT:
	case CPID_FenceRange:
	case CPID_FenceRangeT:
	case CPID_FenceSandBagWall:
	case CPID_FenceSplitRail:
	case CPID_FenceStoneWall:
	case CPID_FenceWire:
	case CPID_FenceWireT:
	case CPID_FenceWoodYard:
	case CPID_Irrigation:
	case CPID_RoadGrade:
		break;
	}
	m_aComponents.AddRequirement(CID_Computer, CE_DecreasesTime, 0);
	m_aComponents.AddRequirement(CID_Electricity, CE_DecreasesTime, 1);
	m_aComponents.AddRequirement(CID_SUV, CE_DecreasesTime, 0);
	m_aComponents.AddRequirement(CID_Shovel, CE_DecreasesTime, 0);
}

void DeConstructionProcess::Halt()
{
	if (m_ePhase == CP_Stakes)
		m_ePhase = CP_HaltedStakes;
	else if (m_ePhase == CP_UnderConstruction)
		m_ePhase = CP_Halted;
}

void DeConstructionProcess::Resume()
{
	if (m_ePhase == CP_HaltedStakes)
		m_ePhase = CP_Stakes;
	else if (m_ePhase == CP_Halted)
		m_ePhase = CP_UnderConstruction;
}

void DeConstructionProcess::operator>>(QDataStream& out) const
{
	quint8 cpid = m_lCPID;	// Highest known CPID is 83
	if(m_DesignId)
		cpid |= 0x80;
	out << cpid;

	quint8 phaseBits = m_ePhase;	// b0-b1 = phase
	phaseBits |= 0x80;				// b7 = 32-bit labor, was 16 bits prior
	out << phaseBits;
	out << m_lLaborNeeded;
	m_aComponents >> out;
	out << (quint8)0;				// Used to be number of levels to construct.
	out << m_MPID;

	quint8 switches = 0;
	if(m_WorkerOk)
		switches |= 0x01;
	out << switches;

	if(cpid & 0x80)
		out << m_DesignId;
}

void DeConstructionProcess::operator<<(QDataStream& in)
{
	quint8 cpid;
	in >> cpid;
	m_lCPID = (CONSTRUCTIONPROCESSID)(cpid & 0x7f);
	quint8 phaseBits;
	in >> phaseBits;
	m_ePhase = (CONPHASE)(phaseBits & 0x03);	// b0-b1 = phase
	if(phaseBits & 0x80)			// b7 = 32-bit labor, was 16 bits prior
		in >> m_lLaborNeeded;
	else
	{
		// Read labor needed as a 16 bit value.
		quint16 u16;
		in >> u16;
		m_lLaborNeeded = u16;
	}
	m_aComponents << in;
	quint8 ui8;
	in >> ui8;						// Used to be number of levels to construct.
	in >> m_MPID;

	quint8 switches;
	in >> switches;
	m_WorkerOk = switches & 0x01;

	if(cpid & 0x80)
		in >> m_DesignId;
	else
		m_DesignId = 0;
}

void DeConstructionProcess::Tx(QDataStream& out) const
{
	quint8 cpid = m_lCPID;	// Highest known CPID is 83
	if(m_DesignId)
		cpid |= 0x80;
	out << cpid;

	quint8 phaseBits = m_ePhase;	// b0-b1 = phase
	phaseBits |= 0x80;				// b7 = 32-bit labor, was 16 bits prior
	out << phaseBits;
	out << m_lLaborNeeded;
	m_aComponents.Tx(out);
	out << m_MPID;

	quint8 switches = 0;
	if(m_WorkerOk)
		switches |= 0x01;
	out << switches;

	if(cpid & 0x80)
		out << m_DesignId;
}

void DeConstructionProcess::Rx(QDataStream& in)
{
	quint8 cpid;
	in >> cpid;
	m_lCPID = (CONSTRUCTIONPROCESSID)(cpid & 0x7f);
	quint8 phaseBits;
	in >> phaseBits;
	m_ePhase = (CONPHASE)(phaseBits & 0x03);	// b0-b1 = phase
	if(phaseBits & 0x80)		// b7 = 32-bit labor, was 16 bits prior
		in >> m_lLaborNeeded;
	else
	{
		// Read labor needed as a 16 bit value.
		quint16 u16;
		in >> u16;
		m_lLaborNeeded = u16;
	}
	m_aComponents.Rx(in);
	in >> m_MPID;

	quint8 switches;
	in >> switches;
	m_WorkerOk = switches & 0x01;

	if(cpid & 0x80)
		in >> m_DesignId;
	else
		m_DesignId = 0;
}

void DeConstructionProcess::ChangeMaterial(COMMODITYID idCommodity, COMMODITYID idNewCommodity)
{
	m_aComponents.ChangeMaterial(idCommodity, idNewCommodity);
}

quint32 DeConstructionProcess::DemandTally(const AuItem& item) const
{
	// Halted construction does not represent a demand for materials.
	if(IsHalted())
		return 0;

	// Construction does not necessarily represent a recurring need so the components
	// array demand is sufficient. See DeManufacturingProcess::DemandTally.
	return m_aComponents.ConstructionDemandTally(item);
}

bool DeConstructionProcess::IsRequired(COMMODITYID cid) const
{
	if(IsHalted())
		return false;
	return m_aComponents.IsRequired(cid);
}

// returns true if any materials are taken from the inventory
bool DeConstructionProcess::FetchMaterials(AuItemHash& inventory, AuItemHash& tools)
{
	bool anyTaken = false;
	bool anyTakenThisPass;
	do
	{
		anyTakenThisPass = false;
		for(DeProcessComponentArray::iterator pci = m_aComponents.begin(); pci != m_aComponents.end(); pci++)
			if(!(*pci).IsSatisfied())
			{
				if((*pci).QtyConsumed() == 0)
				{
					// Item is needed that does not get consumed. Locate one in our tools list.
					AuItemHash::iterator toolI = tools.FindBest((*pci).CommodityId());
					if(toolI == tools.end())
					{
						// Tool was not found. Try taking one from the inventory.
						// Locate the best tools so they don't wear down as fast.
						AuItemHash::iterator invI = inventory.FindBest((*pci).CommodityId());
						if(invI != inventory.end())
						{
							toolI = tools.AddItemCount(*(*invI).second, 1);
							inventory.TakeItem(*(*invI).second, 1);
						}
					}

					if(toolI != tools.end())
					{
						// We have the tool. Apply it to the process components.
						quint32 oldcount = (*toolI).second->Count();
						AddLabor(m_aComponents.ApplyInventoryMaterial(pci, *(*toolI).second, 0, 0, 0));
						if((*toolI).second->Count() == 0)
						{
							tools.Erase(toolI);
							anyTakenThisPass = true;
							anyTaken = true;
						}
						else if((*toolI).second->Count() != oldcount)
						{
							anyTakenThisPass = true;
							anyTaken = true;
						}
					}
				}
				else
				{
					// The quality of the building materials does not currently affect the finished building so fetch the worst
					// quality of materials to save the good stuff for manufacturing processes.
					//if((*pci).CommodityId() == CID_Stone)
						//qDebug() << "DeConstructionProcess::FetchMaterials Stone" << (*pci).CommodityId();
					AuItemHash::iterator invI = inventory.FindWorst((*pci).CommodityId());
					if(invI != inventory.end())
					{
						quint32 oldcount = (*invI).second->Count();
						AddLabor(m_aComponents.ApplyInventoryMaterial(pci, *(*invI).second, 0, 0, 0));
						//if((*pci).CommodityId() == CID_Stone)
							//qDebug() << "DeConstructionProcess::FetchMaterials Stone found" << oldcount << (*invI).second->Count();
						if((*invI).second->Count() == 0)
						{
							inventory.Erase(invI);
							anyTakenThisPass = true;
							anyTaken = true;
						}
						else if((*invI).second->Count() != oldcount)
						{
							anyTakenThisPass = true;
							anyTaken = true;
						}
					}
				}
			}
	}
	while(anyTakenThisPass);

	return anyTaken;
}

// Satisfy any needs for vehicles from the ones listed. These are the vehicles available to the process. Vehicles are never
// consumes; they just modify processes.
bool DeConstructionProcess::FetchVehicles(const QList<COMMODITYID>& vehiclesAvailable)
{
	bool anyChange = false;
	for(DeProcessComponentArray::iterator pci = m_aComponents.begin(); pci != m_aComponents.end(); pci++)
		if(!(*pci).IsSatisfied() && DbCommodity::IsVehicle((*pci).CommodityId()))
			if((*pci).SupplyVehicle(vehiclesAvailable))
				anyChange = true;
	return anyChange;
}

void DeConstructionProcess::InitBd(const AuBdDesign& design)
{
	m_DesignId = design.Id();
	m_ePhase = CP_Stakes;
	m_lCPID = CPID_Building;
	m_lLaborNeeded = 0;
	m_aComponents.clear();
	m_MPID = 0;
	m_WorkerOk = false;

	// List components required by the design.
	design.ListComponents(m_aComponents);
	
	if((quint32)(design.FieldArea()*SquareMetersPerSquareFoot()) >= design.AreaFirstField())
		m_aComponents.AddRequirement(CID_Shovel, CE_DecreasesTime, 0);	// Shovels help to prepare fields.
	if(design.ShopVolume() >= design.VolumeFirstShop() || design.PowerPlantVolume() >= design.VolumeFirstGenerator())
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);	// Buildings that manufacture things require tools to build.

	// Add tools needed for various components.
	// These are done last so they won't affect existing components of the same type.
	if(m_aComponents.IsAnyModuleRequired())
		m_aComponents.AddRequirement(CID_FittingWrench, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Adamantine))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Computer))
		m_aComponents.AddRequirement(CID_ScrewDriver, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_ElectronicPart))
		m_aComponents.AddRequirement(CID_SolderIron, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_IoTube))
		m_aComponents.AddRequirement(CID_Clamp, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Magmium))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_MechanicalPart))
		m_aComponents.AddRequirement(CID_Wrench, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Metal))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Plastic))
		m_aComponents.AddRequirement(CID_GlueGun, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Textiles))
		m_aComponents.AddRequirement(CID_SewingNeedle, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Vulcium))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);

	// Add some things that are helpful.
	if(!m_aComponents.IsComponent(CID_Electricity))
		m_aComponents.AddRequirement(CID_Electricity, CE_DecreasesTime, 1);
	if(!m_aComponents.IsComponent(CID_Computer))
		m_aComponents.AddRequirement(CID_Computer, CE_DecreasesTime, 0);
	if(!m_aComponents.IsComponent(CID_Hammer))
		m_aComponents.AddRequirement(CID_Hammer, CE_DecreasesTime, 0);	// Hammer can always help building construction.
	if(!m_aComponents.IsComponent(CID_Shovel))
		m_aComponents.AddRequirement(CID_Shovel, CE_DecreasesTime, 0);	// Shovel can always help building construction.
	if(!m_aComponents.IsComponent(CID_SUV))
		m_aComponents.AddRequirement(CID_SUV, CE_DecreasesTime, 0);

	m_lLaborNeeded = 10;
}

unsigned int DeConstructionProcess::SecondsEstimated() const
{
	unsigned int compLabor = m_lLaborNeeded;
	for(DeProcessComponentArray::const_iterator pci = m_aComponents.cbegin(); pci != m_aComponents.cend(); pci++)
		if((*pci).IsRequired() && (*pci).QtyConsumed())
			compLabor += (*pci).QtyConsumed();

	unsigned int secsToRun = (compLabor + LABOR_PER_SEC - 1)/LABOR_PER_SEC;
	if(!secsToRun)
		secsToRun = 1;
	return secsToRun;
}

unsigned int DeConstructionProcess::PercentComplete() const
{
	unsigned int totalApplied = 0;
	unsigned int totalNeeded = 1;	// Avoids divide by zero. Avoids returning 100% since this is called when under construction.
	for(DeProcessComponentArray::const_iterator pci = m_aComponents.cbegin(); pci != m_aComponents.cend(); pci++)
		if((*pci).IsRequired() && (*pci).QtyConsumed())
		{
			totalApplied += (*pci).QtyApplied();
			totalNeeded += (*pci).QtyConsumed();
		}
	unsigned int percComplete = Percent(totalApplied, totalNeeded);
	if(percComplete >= 100)
		return 100;
	return percComplete;
}

quint64 DeConstructionProcess::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_DesignId);
	mem += sizeof(m_ePhase);
	mem += sizeof(m_lCPID);
	mem += sizeof(m_lLaborNeeded);
	mem += sizeof(DeProcessComponent) * m_aComponents.capacity();
	mem += sizeof(m_MPID);
	mem += sizeof(m_WorkerOk);
	return mem;
}

quint8 DeConstructionProcess::QualityOfConstruction() const
{
	return m_aComponents.LowestQualityApplied();
}
