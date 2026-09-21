//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: demanufacturingprocess.cpp,v 1.179 2022/05/06 18:31:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "demanufacturingprocess.h"
//
#include "audna.h"
#include "auglobal.h"
#include "auitem.h"
#include "auknownpatents.h"
#include "auscdesign.h"
#include "auscreport.h"
#include "dgdata.h"
#include "dhdna.h"

#define c_ManufMult		10		// Multiplication factor for DbManufacturingProcess amounts, for bulk manufacturing. The db
								// amounts reflect one-off quantities as needed for hand-crafting.

// Initializes this process for an industry.
// Selects the first manufacturing process in the database.
// m_lMPID will be set to zero if the industry has no manufacturing processes,
// including those like spacecraft factories that create custom manufacturing
// processes based on a spacecraft design.
// However, most (all?) industries now have at least a patent process.
void DeManufacturingProcess::InitIndustry(INDUSTRYID lIID, const AuKnownPatents& patents)
{
	m_lMPID = 0;
	quint32 patentmpid = 0;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbManufacturingProcessArray::const_iterator it = gd.m_bManufacturingProcessArray.cbegin();
			it != gd.m_bManufacturingProcessArray.cend(); it++)
	{
		if((*it).m_idi != lIID)
			continue;					// The process cannot be run by the industry.
		// Insure the patent is available to run the process.
		if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*it).m_id, patents))
			continue;

		// The patent is available.
		// Remember the first process id, in case there is no specified default.
		if((*it).m_isDefault)
		{
			// Quit if we find the default process id.
			m_lMPID = (*it).m_id;
			break;
		}
		else
		{
			// Remember the first mpid we can run, both patent and non-patent.
			if(DbCommodity::IsPatent((*it).CommodityId()))
			{
				if(!patentmpid)
					patentmpid = (*it).m_id;
			}
			else
			{
				if(!m_lMPID)
					m_lMPID = (*it).m_id;
			}
		}
	}
	g_Au.UnlockData();
	// If we did not find a non-patent process, select the first patent process we found, if any.
	if(!m_lMPID)
		m_lMPID = patentmpid;
	InitDbData(true);
}

bool DeManufacturingProcess::InitSc(quint32 designId, quint8 designQ)
{
	Q_ASSERT(designQ);
	//qDebug("DeManufacturingProcess::InitSc design %d", designId);
	// Init process values for safe return on failure.
	m_aComponents.clear();
	m_lMPID = 0;
	m_lCID = 0;
	m_lLaborNeeded = 0;
	m_lProcessLabor = 0;
	m_lQtyProduced = 0;
	m_sCName = "";
	m_QualityReq = designQ;
	m_QualitySet = m_QualityReq;
	m_tmComplete.SetNull();
	m_ucLastQuality = 0;			// Quality of last commodity produced.
	m_LastQuantity = 0;				// Quantity of last commodity produced. Negative values are error codes.
	m_DNAIsOutput = false;
	m_DNAIsSentient = false;
	m_DNASpecies = AuSpecies();

	if(designId == 0)
		return true;	// We get here when the "None" option is selected on the spacecraft manufacture dialog.

	// Determine the process needed to manufacture the spacecraft design.
	AuScDesign design;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesign(designId, design);
	g_Au.UnlockScDesigns();
	if(!hasDesign)
	{
		//qDebug() << "DeManufacturingProcess::InitSc design not loaded" << designId;
		return false;	// Failed to load ship design.
	}

	InitSc(design, designQ);
	return true;
}

void DeManufacturingProcess::InitSc(const AuScDesign& design, quint8 designQ)
{
	Q_ASSERT(designQ);
	//qDebug("DeManufacturingProcess::InitSc design %d", designId);
	// Init process values for safe return on failure.
	m_aComponents.clear();
	m_lMPID = 0;
	m_lCID = 0;
	m_lLaborNeeded = 0;
	m_lProcessLabor = 0;
	m_lQtyProduced = 0;
	m_sCName = "";
	m_QualityReq = designQ;
	m_QualitySet = m_QualityReq;
	m_tmComplete.SetNull();
	m_ucLastQuality = 0;			// Quality of last commodity produced.
	m_LastQuantity = 0;				// Quantity of last commodity produced. Negative values are error codes.
	m_DNAIsOutput = false;
	m_DNAIsSentient = false;
	m_DNASpecies = AuSpecies();

	m_sCName = QObject::tr("%1 by %2").arg(design.Name()).arg(design.ArchitectName());

	// List components required by the design.
	design.ListComponents(m_aComponents);
	
	// Add tools needed for various components.
	// These are done last so they won't affect existing components of the same type.
	m_aComponents.AddRequirement(CID_Electricity, CE_Required, 0);
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
	if(m_aComponents.IsRequired(CID_Gold))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Plastic))
		m_aComponents.AddRequirement(CID_GlueGun, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Textiles))
		m_aComponents.AddRequirement(CID_SewingNeedle, CE_Required, 0);
	if(m_aComponents.IsRequired(CID_Vulcium))
		m_aComponents.AddRequirement(CID_Hammer, CE_Required, 0);

	m_lMPID = -(int)design.Id();	// Store neg to distinguish design id from commodity
	m_lCID = m_lMPID;		// For the manufacturing console.
	m_lProcessLabor = 20;
	m_lLaborNeeded = m_lProcessLabor;
	m_lQtyProduced = 1;
	//qDebug("DeManufacturingProcess::InitSc design %d %s initialized",
			//designId, (const char*)design.m_name.toAscii());
}

// Loads process values for m_lMPID.
void DeManufacturingProcess::InitDbData(bool doManufMult)
{
	// Init process values for safe return on failure.
	m_aComponents.clear();
	m_lCID = 0;
	m_lLaborNeeded = 0;
	m_lProcessLabor = 0;
	m_lQtyProduced = 0;
	m_sCName = "";
	m_QualityReq = 1;
	m_QualitySet = m_QualityReq;
	m_tmComplete.SetNull();
	m_ucLastQuality = 0;			// Quality of last commodity produced.
	m_LastQuantity = 0;				// Quantity of last commodity produced. Negative values are error codes.
	m_DNAIsOutput = false;
	m_DNAIsSentient = false;
	m_DNASpecies = AuSpecies();
	if(m_lMPID <= 0)
		return;	// A spacecraft design or "None" is selected.

	const DgData& gd = g_Au.LockDataForRead();
	DbManufacturingProcess bMan;
	if(gd.ManufacturingProcess(m_lMPID, bMan))
	{
		m_lCID = bMan.m_idc;
		m_sCName = g_Au.CommodityName(CommodityId());

		// Processes that produce commodities that store DNA will collect DNA from components that are consumed by the process.
		m_DNAIsOutput = AuItem::ItemType(CommodityId()) == AuItemDNA::ItemType();

		// Some processes do not receive the manufacturing quantity multiplier, even when it is requested.
		// Tech, patent and non-cargo items such as vehicles are not manufactured in bulk quantities.
		m_lProcessLabor = bMan.m_labor;
		m_lQtyProduced = bMan.m_produces;

		// Apply bulk manufacturing multiplier.
		bool doMultiple;
		if(doManufMult && (CommodityId() == CID_Electricity || CommodityId() == CID_Money ||
				((g_Au.Commodity(CommodityId()).m_combits & CB_Cargo) &&
				 !DbCommodity::IsPatent(CommodityId()))))
		{
			// This process is subject to the bulk manufacturing multiplier.
			m_lProcessLabor *= c_ManufMult;
			m_lQtyProduced *= c_ManufMult;
			doMultiple = true;
		}
		else
		{
			// Do not do bulk manufacturing.
			doMultiple = false;
		}
		m_lLaborNeeded = m_lProcessLabor;

		// Process components.
		DbManufacturingComponentArray::const_iterator it;
		for(it = gd.m_bManufacturingComponentArray.cbegin(); it != gd.m_bManufacturingComponentArray.cend(); ++it)
			if((*it).m_idmp == m_lMPID)
			{
				// NOTE: To see how the cost of Tech is adjusted based on quality, look at the SetBuildingQ function.
				if((COMMODITYID)(*it).m_idc == CID_Money)
				{
					// When smelting gold from money, we must consume the same amount of pure money gold as the corresponding
					// bullion from gold process produces. The process data is built around the QualityBaseline of 87.
					// Perhaps the quality baseline should be 255?
					if(CommodityId() == CID_Gold)
						m_aComponents.AddRequirement((COMMODITYID)(*it).m_idc, (COMPEFFECT)(*it).m_ide,
								(*it).m_consumed * QualityBoostF(255));
					else
						m_aComponents.AddRequirement((COMMODITYID)(*it).m_idc, (COMPEFFECT)(*it).m_ide, (*it).m_consumed);
				}
				else if(doMultiple && (*it).IsRequired() && (*it).m_consumed)
					m_aComponents.AddRequirement((COMMODITYID)(*it).m_idc, (COMPEFFECT)(*it).m_ide, (*it).m_consumed*c_ManufMult);
				else
					m_aComponents.AddRequirement((COMMODITYID)(*it).m_idc, (COMPEFFECT)(*it).m_ide, (*it).m_consumed);
			}
	}
	g_Au.UnlockData();
}

void DeManufacturingProcess::operator>>(QDataStream& out) const
{
	out << AuId(0);			// m_TerId;
	out << QPointF(0,0);	// m_TileLL;
	out << m_idProcess;

	quint8 bits = 0x80;				// b7 indicates we have gone to 32 bit quantities, for multiples.
	if(m_ShopsEnabled == 1)			// Used to be a bool. We'll treat it as a 1 at the other side.
		bits |= 0x01;
	if(m_DNAIsOutput)
		bits |= 0x02;
	if(m_DNAIsSentient)
		bits |= 0x04;
	if(m_QualityReq != m_QualitySet)
		bits |= 0x08;
	if(m_ShopsEnabled > 1 || m_StockLimit)
		bits |= 0x10;
	if(m_QtyToProduce)
		bits |= 0x20;
	if(m_WorkersPresent)
		bits |= 0x40;

	out << bits;
	out << m_lMPID;
	out << m_lLaborNeeded;
	out << m_sCName;
	out << m_tmComplete.UTC();
	out << m_ucLastQuality;
	out << m_LastQuantity;
	m_aComponents >> out;
	out << m_QualityReq;
	if(bits & 0x08)
		out << m_QualitySet;
	quint8 ver = 3;
	out << ver;
	out << m_lProductionBoost;	// Became 32 bits in ver 1
	out << m_lCID;
	out << m_lProcessLabor;
	out << m_lQtyProduced;
	out << m_tmLastWork.UTC();

	if(bits & 0x02)
		out << m_DNASpecies;

	if(bits & 0x10)
	{
		out << m_ShopsEnabled;
		out << m_StockLimit;	// Added in ver 3.
	}
	if(bits & 0x20)
		out << m_QtyToProduce;
	if(bits & 0x40)
		out << m_WorkersPresent;
}

void DeManufacturingProcess::operator<<(QDataStream& in)
{
	//in >> m_TerId;
	//in >> m_TileLL;
	AuId terId;
	QPointF tileLL;
	in >> terId;
	in >> tileLL;

	in >> m_idProcess;
	quint8 bits;
	in >> bits;
	m_ShopsEnabled = (bits & 0x01)?1:0;
	in >> m_lMPID;
	in >> m_lLaborNeeded;
	in >> m_sCName;

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_tmComplete = tm;

	in >> m_ucLastQuality;
	in >> m_LastQuantity;
	m_aComponents << in;
	in >> m_QualityReq;
	if(!m_QualityReq)
		m_QualityReq = 1;
	if(bits & 0x08)
		in >> m_QualitySet;
	else
		m_QualitySet = m_QualityReq;
	if(!m_QualitySet)
		m_QualitySet = 1;
	quint8 ver;
	in >> ver;
	if(ver < 1)
	{
		qint8 pb;
		in >> pb;
		m_lProductionBoost = pb;
	}
	else
		in >> m_lProductionBoost;
	in >> m_lCID;
	if(m_lCID == CID_ComputerObsolete)
		m_lCID = CID_Computer;
	if(bits & 0x80)
	{
		in >> m_lProcessLabor;
		in >> m_lQtyProduced;
	}
	else
	{
		// Quantities were 16 bit at one time, too small for multiples.
		quint16 procLab, qtyProd;
		in >> procLab;
		in >> qtyProd;
		m_lProcessLabor = procLab;
		m_lQtyProduced = qtyProd;
	}

	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_tmLastWork = tm;

	m_DNAIsOutput = (bits & 0x02) ? true : false;
	m_DNAIsSentient = (bits & 0x04) ? true : false;
	if(m_DNAIsOutput)
	{
		if(ver >= 2)
			in >> m_DNASpecies;
		else
		{
			DhDNA dna3;
			in >> dna3;
			m_DNASpecies = dna3.Species();
		}
	}

	if(bits & 0x10)
	{
		in >> m_ShopsEnabled;
		if(ver >= 3)
			in >> m_StockLimit;
	}
	if(bits & 0x20)
		in >> m_QtyToProduce;
	if(bits & 0x40)
		in >> m_WorkersPresent;

	if(IsRunning() && !m_aComponents.IsCompleted())
		qDebug() << "DeManufacturingProcess::operator<< error: running but components incomplete";
}

void DeManufacturingProcess::FinishOutput(const AuZoneResource& naturalResourcesPresent, const AuKnownPatents& patents)
{
	if(CommodityId() == CID_Electricity)
	{
		// Electricity is always assumed to be of perfect quality and the highest quality since it is pure energy.
		m_ucLastQuality = 255;
	}
	else
	{
		quint32 lval = 0;
		quint32 lcount = 0;
		quint8 natQ = 0;
		for(DeProcessComponentArray::const_iterator pc = m_aComponents.cbegin(); pc != m_aComponents.cend(); ++pc)
		{
			// Components that are not required do not affect the quality of the result.
			if(!(*pc).IsRequired())
				continue;
			if((*pc).CommodityId() == CID_Electricity)
				continue;	// Quality of power does not affect the quality of the result.
			if((*pc).CommodityId() == CID_Money && CommodityId() != CID_Gold)
				continue;	// Quality of money does not affect the quality of the result, except when smelting money into gold.
			if((*pc).QtyConsumed())
			{
				// Consumed components directly affect the quality of the result.
				if((*pc).Quality() > 0 && (*pc).QtyApplied() > 0)
				{
					lval += ((*pc).Quality() * (*pc).QtyApplied());
					lcount += (*pc).QtyApplied();
				}
			}
			else
			{
				// Some patent processes do not consume anything but they require natural resources. We will get the result quality
				// from the required natural resources. This will be used only if nothing else gives us the quality.
				quint8 byResQ = naturalResourcesPresent.Quality((*pc).CommodityId());
				if(byResQ)
					natQ = byResQ;
			}
		}

		// Nature affects output quality of some resources. This call returns the appropriate resource quality produced for some
		// patents for refining and extracting natural resources. i.e. Air (from atmo), Cryozine, Hydrogen (from atmo), Ioplasma,
		// Magmex, Myrathane and Flomentum.
		unsigned char byResQ = naturalResourcesPresent.Quality(CommodityId());
		if(byResQ)
		{
			const DgData& gd = g_Au.LockDataForRead();
			if(gd.IsManufacturingProcessNatural(m_lMPID))
			{
				// A natural resource was produced.
				lval += byResQ;
				lcount++;
			}
			g_Au.UnlockData();
		}

		if(lcount)
		{
			// Calculate average quality of all components.
			if(lcount > 1)
				lval /= lcount;
		}
		else
		{
			// Nothing was consumed, other than money or electricity. Patent development process?
			if(byResQ)
				lval = byResQ;	// Quality of output is the quality of natural resource that is produced.
			else
				lval = natQ;	// Quality of output is the quality of the required natural resource component.
		}

		// Patents limit the output quality, except for patent development processes.
		if(!DbCommodity::IsPatent(CommodityId()))
		{
			const DgData& gd = g_Au.LockDataForRead();
			quint8 patentQ = gd.ManufacturingProcessPatentQ(m_lMPID, patents);
			g_Au.UnlockData();
			if(lval > patentQ)
				lval = patentQ;
		}
		if(!lval)
			lval = 1;
		Q_ASSERT(lval <= 255);
		m_ucLastQuality = (quint8)lval;
	}
	m_LastQuantity = QtyProduced();
}

// Returns the estimated manufacturing cost based on a quality-based cost for all components.
float DeManufacturingProcess::EstimatedCost(quint8 quality) const
{
	float estCost = 0;
	for(DeProcessComponentArray::const_iterator pc = m_aComponents.cbegin(); pc != m_aComponents.cend(); pc++)
	{
		if((*pc).QtyConsumed())
		{
			// Get the cost of the commodity at the quality specified.
			float cost = g_Au.CommodityQualityCost((*pc).CommodityId(), quality);

			// Cost is increased to account for the labor added per unit.
			estCost += (cost + LABOR_RATE) * (*pc).QtyConsumed();
		}
	}

	// Round estimated cost to 1000 cronos.
	estCost /= 1000;
	estCost = round(estCost);
	estCost *= 1000;
	return estCost;
}

// Returns the actual manufacturing cost based on a quality-based cost
// for all components that have been fetched.
// Assumes all commodities have been satisfied; does not verify that.
float DeManufacturingProcess::ActualCost() const
{
	float actCost = 0;
	if(ManufacturingProcessId())
	{
		// The process looks valid. List the components.
		for(DeProcessComponentArray::const_iterator pc = m_aComponents.cbegin(); pc != m_aComponents.cend(); pc++)
		{
			if((*pc).QtyConsumed())
			{
				// Get the cost of the commodity at the quality specified.
				float cost = (*pc).Cost();

				// Cost is increased to account for the labor added per unit.
				actCost += (cost + LABOR_RATE) * (*pc).QtyConsumed();
			}
		}

		// Round actual cost to 1000 cronos.
		actCost /= 1000;
		actCost = round(actCost);
		actCost *= 1000;
	}
	return actCost;
}

// This function is only ever called on the contents of DaItem objects lying about, not on city inventories.
// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
void DeManufacturingProcess::ApplyFetchedMaterial(AuItem& ci, quint8 elecSubQ, quint8 waterSubQ)
{
	quint32 oldCount = ci.Count();
	m_lLaborNeeded += m_aComponents.ApplyInventoryMaterial(ci, elecSubQ, waterSubQ, m_QualitySet);

	// Gather DNA from items that get consumed, if this process outputs a commodity that stores DNA.
	if(m_DNAIsOutput && oldCount != ci.Count())
	{
		// Some of the item was consumed. Pick up its DNA if it has any.
		if(ci.DNA())
			SetSpecies(ci.DNASpecies(), ci.DNAIsSentient());
	}
}

// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
// returns true if any materials are taken from the inventory
bool DeManufacturingProcess::FetchMaterials(AuItemHash& inventory, AuItemHash& tools, const AuKnownPatents& patents,
		quint8 elecSubQ, quint8 waterSubQ, quint32& power)
{
	AuItem electricity(CID_Electricity, 255, 0, power);
	bool anyTaken;
	do
	{
		anyTaken = false;
		for(DeProcessComponentArray::iterator pci = m_aComponents.begin(); pci != m_aComponents.end(); pci++)
			if(!(*pci).IsSatisfied())
			{
				if(DbCommodity::IsPatent((*pci).CommodityId()))
				{
					// Patents are found in a separate list.
					quint8 patentQ = patents.PatentQuality((*pci).CommodityId());
					if(patentQ && patentQ >= m_QualitySet)
						(*pci).SetQtyApplied(1);
				}
				else if(electricity.Count() &&
						(((*pci).CommodityId() == CID_Sunlight && elecSubQ) || (*pci).CommodityId() == CID_Electricity))
				{
					// Use electricity from the power passed in. When that happens, it does not exist in the inventory.
					m_lLaborNeeded += m_aComponents.ApplyInventoryMaterial(pci, electricity, elecSubQ, waterSubQ, m_QualitySet);
					anyTaken = true;
				}
				else if((*pci).QtyConsumed() == 0)
				{
					// Item is needed that does not get consumed. Locate one in our tools list.
					AuItemHash::iterator toolI;
					if((*pci).CommodityId() == CID_Sunlight && elecSubQ)
						toolI = tools.FindBest(CID_Electricity);		// Supply electricity for the sunlight component.
					else if((*pci).CommodityId() == CID_Hydrographics && waterSubQ)
						toolI = tools.FindBest(CID_Water);				// Supply water for the hydrographics component.
					else
						toolI = tools.FindBest((*pci).CommodityId());
					if(toolI == tools.end())
					{
						// Tool was not found. Try taking one from the inventory.
						// Locate the best tools so they don't wear down as fast.
						AuItemHash::iterator invI;
						if((*pci).CommodityId() == CID_Sunlight && elecSubQ)
							invI = inventory.FindBest(CID_Electricity);		// Supply electricity for the sunlight component.
						else if((*pci).CommodityId() == CID_Hydrographics && waterSubQ)
							invI = inventory.FindBest(CID_Water);			// Supply water for the hydrographics component.
						else
							invI = inventory.FindBest((*pci).CommodityId());
						if(invI != inventory.end())
						{
							toolI = tools.AddItemCount(*(*invI).second, 1);
							inventory.TakeItem(*(*invI).second, 1);
							anyTaken = true;
						}
					}

					if(toolI != tools.end())
					{
						// We have the tool. Apply it to the process components.
						quint32 oldcount = (*toolI).second->Count();
						m_lLaborNeeded += m_aComponents.ApplyInventoryMaterial(pci, *(*toolI).second, elecSubQ, waterSubQ,
								m_QualitySet);
						if((*toolI).second->Count() == 0)
						{
							tools.Erase(toolI);
							anyTaken = true;
						}
						else if((*toolI).second->Count() != oldcount)
							anyTaken = true;
					}
				}
				else
				{
					// Sunlight is not stored in any inventory but sometimes electricity can be substituted, simulating the use of
					// electric lighting at farms for instance.
					AuItemHash::iterator invI;
					if((*pci).CommodityId() == CID_Sunlight && elecSubQ)
						invI = inventory.FindBest(CID_Electricity);		// Supply electricity for the sunlight component.
					else if((*pci).CommodityId() == CID_Hydrographics && waterSubQ)
						invI = inventory.FindBest(CID_Water);			// Supply water for the hydrographics component.
					else
						invI = inventory.FindBest((*pci).CommodityId());
					if(invI != inventory.end())
					{
						quint32 oldcount = (*invI).second->Count();
						m_lLaborNeeded += m_aComponents.ApplyInventoryMaterial(pci, *(*invI).second, elecSubQ, waterSubQ,
								m_QualitySet);

						// Gather DNA from items that get consumed, if this process outputs a commodity that stores DNA.
						if(m_DNAIsOutput && oldcount != (*invI).second->Count())
						{
							// Some of the item was consumed. Pick up its DNA if it has any.
							if((*invI).second->DNA())
								SetSpecies((*invI).second->DNASpecies(), (*invI).second->DNAIsSentient());
						}

						if((*invI).second->Count() == 0)
						{
							inventory.Erase(invI);
							anyTaken = true;
						}
						else if((*invI).second->Count() != oldcount)
							anyTaken = true;
					}
				}
			}
	}
	while(anyTaken);
	if(power && anyTaken)
		power = electricity.Count();
	return anyTaken;
}

// Satisfy any needs for vehicles from the ones listed. These are the vehicles available to the process. Vehicles are never
// consumes; they just modify processes.
bool DeManufacturingProcess::FetchVehicles(const QList<COMMODITYID>& vehiclesAvailable)
{
	bool anyChange = false;
	for(DeProcessComponentArray::iterator pci = m_aComponents.begin(); pci != m_aComponents.end(); pci++)
		if(!(*pci).IsSatisfied() && DbCommodity::IsVehicle((*pci).CommodityId()))
			if((*pci).SupplyVehicle(vehiclesAvailable))
				anyChange = true;
	return anyChange;
}

bool DeManufacturingProcess::IsValid() const
{
	if(!m_lMPID)
	{
		qDebug() << "DeManufacturingProcess::IsValid CID" << m_lCID << "MPID ZERO" << m_lMPID;
		return false;	// Every process must have a manufacturing process id. Spacecraft factories use negative of design id.
	}

	if(!m_idProcess)
	{
		qDebug() << "DeManufacturingProcess::IsValid CID" << m_lCID << "MPID" << m_lMPID << "processId ZERO";
		return false;	// Every process must have a non-zero process id.
	}

	if(m_aComponents.isEmpty())
	{
		qDebug() << "DeManufacturingProcess::IsValid CID" << m_lCID << "MPID" << m_lMPID << "processId" << m_idProcess <<
			"components EMPTY";
		return false;	// Every process has at least one component.
	}

	if(m_lMPID < 0)
	{
		// Every spacecraft design process consumes at least one required commodity.
		// We can't really test all components comprehensively because the spacecraft design has to be loaded to do that.
		// When this function gets called at startup, the spacecraft design may not be loaded.
		if(!m_aComponents.DoesConsumeCommodity())
		{
			// The process is obviously broken.
			qDebug() << "DeManufacturingProcess::IsValid CID" << m_lCID << "MPID" << m_lMPID << "processId" << m_idProcess <<
				"components" << m_aComponents.size() << "consumes ZERO commodities";
			return false;
		}
	}
	else
	{
		// All other processes must match a corresponding manufacturing process in the database.
		DeManufacturingProcess cmp;
		cmp.SetManufacturingProcessId(m_lMPID);
		if(cmp.m_lCID != m_lCID)
		{
			qDebug() << "DeManufacturingProcess::IsValid CID" << m_lCID << "MPID" << m_lMPID << "processId" << m_idProcess <<
				"components" << m_aComponents.size() << "mfg proc NOT FOUND";
			return false;
		}

		// We found the manufacturing process. Compare the components.
		/* I added a patent requirement to a lot of processes. That change made this code destroy a lot of essential processes.
		if(cmp.Components().count() != m_aComponents.count())
		{
			qDebug("DeManufacturingProcess::IsValid terId %d CID %d MPID %d m_idProcess %d cx %g, cy %g components %d "
			return false;
		}

		// Compare the individual components.
		for(DeProcessComponentArray::const_iterator it = cmp.Components().cbegin(); it != cmp.Components().cend(); it++)
		{
			// Found a component prescribed for this process. Is it in my components array?
			bool foundComponent = false;
			for(DeProcessComponentArray::const_iterator pc = m_aComponents.cbegin(); pc != m_aComponents.cend(); ++pc)
			{
				if((*pc).CommodityId() == (*it).CommodityId() &&		// Commodity Id
						//(*pc).QtyConsumed() == (*it).m_consumed &&		// Amount Consumed
						(*pc).Effect() == (*it).Effect())				// Effect On Process
				{
					foundComponent = true;
					break;
				}
			}
			if(!foundComponent)
			{
				// A component of our process is missing.
				qDebug("DeManufacturingProcess::IsValid terId %d CID %d MPID %d m_idProcess %d cx %g, cy %g components %d "
				return false;
			}
		}
		*/
	}

	return true;
}

// Return true if this process is manufacturing something that is critical to the survival of the city.
bool DeManufacturingProcess::IsCriticalToSurvival(bool planetCanHaveFauna) const
{
	// Negative commodity id indicates a spacecraft manufacturing process. It is not critical to survival.
	if(CommodityId() <= 0)
		return false;

	if(!planetCanHaveFauna)
	{
		// Air is critical when a planet cannot support animals.
		// Electricity is critical in harsh environments to manufacture air from water.
		if(CommodityId() == CID_Electricity)
			return true;

		// Air and water, because water can be used to make air at a refinery.
		else if(CommodityId() == CID_Air || CommodityId() == CID_Water)
			return true;
	}

	// Food is always critical.
	// Food with real food nutrition value, i.e. not "diet".
	const DbCommodity& commodity = g_Au.Commodity(CommodityId());
	if(commodity.m_food && commodity.IsFood())
		return true;

	// Manufacture is not critical to the survival of the city.
	return false;
}

void DeManufacturingProcess::DemandTally(const AuItem& item, quint32& needCount, quint32& produceCount) const
{
	if(m_ShopsEnabled)
	{
		// Manufacturing represents a recurring need so the components array demand and production count are multiplied.
		// See DeConstructionProcess::DemandTally.
		needCount += m_aComponents.ManufacturingDemandTally(item, m_QualitySet) * m_ShopsEnabled;

		// The output of production needs to be magnified so it has the desired effect on the demand calculation in
		// AuItem::Demand. Otherwise, it's too insignificant once the population grows. This complements the 10x
		// demand multiplier for items that are consumed by processes.
		if(m_lCID == item.CommodityId())
			produceCount += m_lQtyProduced * 10;				// I can produce that commodity.
	}
}

// Time to run based on standard unboosted labor rate and all labor needed when all commodities are fetched.
unsigned int DeManufacturingProcess::SecondsEstimated() const
{
	unsigned int laborNeeded = m_lProcessLabor;
	for(DeProcessComponentArray::const_iterator pci = m_aComponents.cbegin(); pci != m_aComponents.cend(); pci++)
		if((*pci).IsRequired() && (*pci).QtyConsumed())
			laborNeeded += (*pci).QtyConsumed();

	unsigned int secsToRun = (laborNeeded + LABOR_PER_SEC - 1)/LABOR_PER_SEC;
	if(m_ShopsEnabled > 1)
		secsToRun /= m_ShopsEnabled;

	if(!secsToRun)
		secsToRun = 1;

	return secsToRun;
}

void DeManufacturingProcess::Run(const AuTime& start, unsigned int laborBoost, int productionBoost)
{
	unsigned int workForce;
	if(m_WorkersPresent)
		workForce = m_WorkersPresent;
	else
		workForce = 1;
	// Remember production boost for process completion.
	m_QtyToProduce = m_lQtyProduced;
	m_lProductionBoost = productionBoost * workForce;

	// Calculate ending time based on current time and labor rate.
	unsigned int laborPerSec = LABOR_PER_SEC + laborBoost;
	unsigned int secsToRun = (m_lLaborNeeded + laborPerSec - 1)/laborPerSec;
	if(m_WorkersPresent > 1)
		secsToRun /= m_WorkersPresent;

	if(!secsToRun)
		secsToRun = 1;

	m_tmComplete = start.AddSecs(secsToRun);
}

void DeManufacturingProcess::RunHandCraft(const AuKnownPatents& patents)
{
	AuZoneResource naturalResourcesPresent;	// Not needed because no handcrafting process is affected by natural resources.
	FinishOutput(naturalResourcesPresent, patents);
	if(m_LastQuantity > 1)
		m_LastQuantity = 1;				// Handcrafting only produces 1 item, regardless of the process.
}

// Uses UTC time sync'd with the server for best accuracy.
unsigned int DeManufacturingProcess::SecondsRemaining() const
{
	if(!IsRunning())
		return 0;
	AuTime serverUTC(g_Au.CoordinatedTime());
	int secsRem = serverUTC.SecsTo(m_tmComplete);	// Could be negative.
	secsRem += 10;	// For the tock delay when the process finishes. There is always a 10 second delay before it's really done.
	if(secsRem < 0)
		secsRem = 0;
	return (unsigned int)secsRem;
}

// Time to run based on standard unboosted labor rate and current labor needed, which grows as commodities are fetched.
unsigned int DeManufacturingProcess::SecondsAccumulated() const
{
	unsigned int secsToRun = (m_lLaborNeeded + LABOR_PER_SEC - 1)/LABOR_PER_SEC;
	secsToRun += 10;	// For the tock delay when the process finishes. There is always a 10 second delay before it's really done.
	if(!secsToRun)
		secsToRun = 1;
	return secsToRun;
}

void DeManufacturingProcess::SetShopsEnabled(quint32 shops)
{
	m_ShopsEnabled = shops;
}

void DeManufacturingProcess::SetStockLimit(quint32 stockLimit)
{
	m_StockLimit = stockLimit;
}

void DeManufacturingProcess::SetWorkersPresent(quint32 workers)
{
	m_WorkersPresent = workers;
	if(m_lMPID <= 0)
		return;

	// Adjust amounts for multiple workers.
	quint32 calcForWorkers = workers ? workers : 1;				// Calc values for 1 worker if disabled.
	const DgData& gd = g_Au.LockDataForRead();
	DbManufacturingProcess bMan;
	if(gd.ManufacturingProcess(m_lMPID, bMan))
	{
		// Some processes do not receive the manufacturing quantity multiplier, even when it is requested.
		// Tech, patent and non-cargo items such as vehicles are not manufactured in bulk quantities.
		m_lProcessLabor = bMan.m_labor * calcForWorkers;
		m_lQtyProduced = bMan.m_produces * calcForWorkers;

		// Apply bulk manufacturing multiplier.
		bool doMultiple;
		if(CommodityId() == CID_Electricity || CommodityId() == CID_Money ||
				((g_Au.Commodity(CommodityId()).m_combits & CB_Cargo) &&
				 !DbCommodity::IsPatent(CommodityId())))
		{
			// This process is subject to the bulk manufacturing multiplier.
			m_lProcessLabor *= c_ManufMult;
			m_lQtyProduced *= c_ManufMult;
			doMultiple = true;
		}
		else
		{
			// Do not do bulk manufacturing.
			doMultiple = false;
		}
		if(!IsRunning())
			m_lLaborNeeded = m_lProcessLabor;

		// Process components.
		for(DeProcessComponentArray::iterator pc = m_aComponents.begin(); pc != m_aComponents.end(); pc++)
		{
			if(!(*pc).IsRequired() || (*pc).QtyConsumed() == 0)
				continue;
			// Determine the database prescribed amount for qty consumed.
			for(DbManufacturingComponentArray::const_iterator it = gd.m_bManufacturingComponentArray.cbegin();
					it != gd.m_bManufacturingComponentArray.cend(); it++)
				if((*it).m_idmp == m_lMPID && (*it).m_idc == (*pc).CommodityId())
				{
					if((COMMODITYID)(*it).m_idc == CID_Money)
					{
						// When smelting gold from money, we must consume the same amount of pure money gold as the corresponding
						// bullion from gold process produces. The process data is built around the QualityBaseline of 87.
						// Perhaps the quality baseline should be 255?
						if(CommodityId() == CID_Gold)
							(*pc).SetQtyConsumed((*it).m_consumed * QualityBoostF(255) * calcForWorkers);
						else
							(*pc).SetQtyConsumed((*it).m_consumed * calcForWorkers);
					}
					else if(doMultiple && (*it).IsRequired() && (*it).m_consumed)
						(*pc).SetQtyConsumed((*it).m_consumed * c_ManufMult * calcForWorkers);
					else
						(*pc).SetQtyConsumed((*it).m_consumed * calcForWorkers);
				}
		}
	}
	g_Au.UnlockData();
}

quint64 DeManufacturingProcess::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(DeProcessComponent) * m_aComponents.capacity();
	mem += sizeof(m_ShopsEnabled);
	mem += sizeof(m_StockLimit);
	mem += sizeof(m_WorkersPresent);
	mem += sizeof(m_QtyToProduce);
	mem += sizeof(m_lLaborNeeded);
	mem += sizeof(m_lMPID);
											// m_lMPID is the -designId if a spacecraft.
											// m_lMPID is 0 when the "None" option is selected in a spacecraft factory.
	mem += sizeof(m_lProductionBoost);
	mem += StringMemoryUsage(m_sCName);
	mem += sizeof(m_QualityReq);
	mem += sizeof(m_QualitySet);
	mem += sizeof(m_tmComplete);
	mem += sizeof(m_ucLastQuality);
	mem += sizeof(m_LastQuantity);
	mem += sizeof(m_DNASpecies);
	mem += m_DNASpecies.MemoryUsage();
	mem += sizeof(m_DNAIsOutput);
	mem += sizeof(m_DNAIsSentient);

	mem += sizeof(m_idProcess);

	mem += sizeof(m_lCID);
	mem += sizeof(m_lProcessLabor);
	mem += sizeof(m_lQtyProduced);

	mem += sizeof(m_tmLastWork);
	return mem;
}

void DeManufacturingProcess::SetSpecies(const AuSpecies& species, bool isSentient)
{
	m_DNAIsSentient = isSentient;
	m_DNASpecies = species;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DeManufacturingProcessArray

quint16 DeManufacturingProcessArray::AvailableProcessId() const
{
	// List the ids that are in use. A map is used so they are ordered sequentially.
	QMap<quint16, bool> usedIds;
	for(const_iterator it = begin(); it != end(); it++)
		usedIds.insert((*it).ProcessId(), true);

	// Locate the first available process id.
	quint16 prevId = 0;
	for(QMap<quint16, bool>::const_iterator uii = usedIds.cbegin(); uii != usedIds.cend(); uii++)
		if(uii.key() == prevId + 1)
			prevId = uii.key();		// Walk to next sequential process id.
		else
			return prevId + 1;		// There is a gap so return the next process id after the previous valid one.

	// Did not find a gap in the process id sequence so return the next number past the end.
	return prevId + 1;
}

void DeManufacturingProcessArray::operator<<(QDataStream& in)
{
	clear();
	quint16 procs;
	in >> procs;
	while(procs)
	{
		DeManufacturingProcess ob;
		in >> ob;
		insert(end(), ob);
		procs--;
	}
}

void DeManufacturingProcessArray::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();

	// Store the data in this multi-hash in reverse order. That way the order is preserved when we reload it, due to the way
	// inserts into a multi-hash work according to Qt doc.
	if(size())
		for(const_iterator it = begin(); it != end(); it++)
			out << (*it);
}

DeManufacturingProcessArray::iterator DeManufacturingProcessArray::FindMPID(quint16 mpid)
{
	for(iterator si = begin(); si != end(); si++)
		if((*si).ManufacturingProcessId() == mpid)
			return si;
	return end();
}

const DeManufacturingProcess& DeManufacturingProcessArray::FindProcess(int index) const
{
	const_iterator si = cbegin();
	while(index)
	{
		si++;
		index--;
	}
	return *si;
}

DeManufacturingProcess& DeManufacturingProcessArray::FindProcess(int index)
{
	iterator si = begin();
	while(index)
	{
		si++;
		index--;
	}
	return *si;
}

void DeManufacturingProcessArray::MoveProcess(int fromListIndex, int toListIndex)
{
	if(fromListIndex < 0 || fromListIndex >= (int)size())
		return;
	if(toListIndex < 0 || toListIndex >= (int)size())
		return;
	if(fromListIndex == toListIndex)
		return;

	iterator si = begin();
	while(fromListIndex)
	{
		si++;
		fromListIndex--;
	}

	DeManufacturingProcess mp = *si;
	erase(si);

	// This will fall out of the loop with ii = end() if we erased the last one in the list and the toListIndex was the last one.
	iterator ii = begin();
	while(toListIndex)
	{
		ii++;
		toListIndex--;
	}
	insert(ii, mp);
}

int DeManufacturingProcessArray::MPIDIndex(quint16 mpid) const
{
	int procIndex = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++, procIndex++)
		if((*ii).ManufacturingProcessId() == mpid)
			return procIndex;
	return -1;
}

quint32 DeManufacturingProcessArray::WorkersPresent() const
{
	quint32 workers = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		workers += (*ii).WorkersPresent();
	return workers;
}

quint32 DeManufacturingProcessArray::ShopsEnabled() const
{
	quint32 workers = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		workers += (*ii).ShopsEnabled();
	return workers;
}

bool DeManufacturingProcessArray::DeleteMPID(quint16 mpid)
{
	iterator pi = FindMPID(mpid);
	if(pi == end())
		return false;
	erase(pi);
	return true;
}

quint64 DeManufacturingProcessArray::MemoryUsage() const
{
	quint64 mem = sizeof(DeManufacturingProcessArray);;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		mem += (*ii).MemoryUsage();
	return mem;
}
