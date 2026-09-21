//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deprocesscomponent.cpp,v 1.83 2022/05/06 18:31:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "deprocesscomponent.h"
//
#include "audice.h"
#include "auglobal.h"
#include "auitem.h"
#include "auzoneresource.h"
#include <QVariant>

DeProcessComponent::DeProcessComponent()
{
	m_lCID = CID_Invalid;
	m_byQuality = 0;
	m_Effect = CE_None;
	m_QtyApplied = 0;
	m_QtyConsumed = 0;
}

bool DeProcessComponent::IsSatisfied() const
{
	if(m_QtyConsumed > 0)
	{
		// This component consumes materials.
		if(m_QtyApplied >= m_QtyConsumed)
			return true;
	}
	else
	{
		// This component does not consume the materials.
		if(m_QtyApplied > 0)
			return true;
	}
	return false;
}

// Returns false if none was needed.
// Sets the quantity consumed and the labor needed to apply the materials.
bool DeProcessComponent::ApplyMaterial(quint32 iQuantity, quint8 byQuality, quint32 &iMaterialConsumed)
{
	qDebug("DeProcessComponent::ApplyMaterial - NEED CONDITION OF ITEM");
	Q_ASSERT(iQuantity > 0);

	// Init return values.
	iMaterialConsumed = 0;

	if(IsSatisfied())
		return false;	// This component must not need any more of this material.

	if(m_QtyConsumed > 0)
	{
		// This component consumes materials.
		AuItem iItem(CommodityId(), byQuality, 0, iQuantity);
		bool bUsed = ApplyMaterial(iItem);
		iMaterialConsumed = iQuantity - iItem.Count();
		return bUsed;
	}
	else
	{
		// This component does not consume the materials.
		m_QtyApplied = 1;

		// The lowest quality item decides the overall quality of this component.
		if(byQuality < m_byQuality || m_byQuality == 0)
			m_byQuality = byQuality;
		return true;
	}
}

// Modifies quantity in 'iItem' if any is consumed.
// Returns labor needed to consume the item(s).
int DeProcessComponent::ApplyMaterial(AuItem& iItem, bool& anyChange)
{
#if !defined(QT_NO_DEBUG)
	if(!iItem.IsValid())
	{
		qDebug("DeProcessComponent::ApplyMaterial inventory item is invalid");
		return 0;
	}
	Q_ASSERT(!IsSatisfied());
	//Q_ASSERT(iItem.Count() > 0);
	if(iItem.Count() == 0)
	{
		qWarning() << QString("DeProcessComponent::ApplyMaterial %1 Q%2 made in %4 count is ZERO!")
			.arg(g_Au.CommodityName(iItem.CommodityId())).arg(iItem.Quality()).arg(iItem.EmpireOfOrigin());
		return 0;
	}
#endif

	if(m_QtyConsumed == 0)
	{
		// This component does not consume the materials.
		if(!iItem.UseItem())
		{
			// The item failed so we need to consume one of them.
			iItem.Consume(1);
			return 1;												// Add labor for failure.
		}

		// The item did not fail its quality test.
		m_QtyApplied = 1;											// Satisfy the requirement for the material.

		// The lowest quality item decides the overall quality of this component.
		if(iItem.Quality() < m_byQuality || m_byQuality == 0)
			m_byQuality = iItem.Quality();
		anyChange = true;
		return 0;													// This component does not require labor.
	}

	// This process component needs to consume more of the item being applied.
	// Calculate the total number of units needed to finish off this component, based on failure rate.
	Q_ASSERT(m_QtyApplied < m_QtyConsumed);							// Due to IsSatisfied test.
	quint32 matlApplied;
	quint32 matlConsumed;
	if(iItem.Count() == 1)
	{
		// Singular items, as fetched from the ground or from a crate, are tested individually.
		if(iItem.TestQuality())
			matlApplied = 1;
		else
			matlApplied = 0;
		matlConsumed = 1;
		//qDebug("DeProcessComponent::ApplyMaterial cid %d count %d matlApplied %d matlConsumed %d",
				//iItem.CommodityId(), iItem.Count(), matlApplied, matlConsumed);
	}
	else
	{
		// Multiple items, as fetched from the inventory of a city, are tested based on rate of failure.
		matlApplied = m_QtyConsumed - m_QtyApplied;				// Number of good units needed.
		quint32 failedUnits = matlApplied - iItem.TestFailure(matlApplied);	// Number of units that fail due to quality.
		matlConsumed = matlApplied + failedUnits;				// Total units needed, considering failure rate.
		//qDebug("DeProcessComponent::ApplyMaterial cid %d count %d matlApplied %d failedUnits %d matlConsumed %d",
				//iItem.CommodityId(), iItem.Count(), matlApplied, failedUnits, matlConsumed);
		if(matlConsumed > iItem.Count())
		{
			// This process component needs more than the amount that was offered.
			if(matlApplied < matlConsumed)
			{
				float frac = (float)matlApplied / (float)matlConsumed;
				matlApplied = (quint32)ceilf(frac * iItem.Count());	// Adjust applied good units according to failure rate.
				if(!matlApplied)
					matlApplied = 1;								// Count at least 1 when multiples are offered.
			}
			matlConsumed = iItem.Count();						// Limit material consumed to the amount offered.
			//qDebug("DeProcessComponent::ApplyMaterial cid %d adjusted matlApplied %d matlConsumed %d",
				//iItem.CommodityId(), matlApplied, matlConsumed);
		}
	}

	if(matlApplied)
	{
		// At least one item passed the quality test to get applied here.
		// The lowest quality item decides the overall quality of this component.
		if(iItem.Quality() < m_byQuality || m_byQuality == 0 || m_QtyApplied == 0)
			m_byQuality = iItem.Quality();
		m_QtyApplied += matlApplied;								// Increase the amount of units applied to this component.
		anyChange = true;
	}

	// Consume the materials.
	iItem.Consume(matlConsumed);
	return GetLaborNeeded(matlConsumed);							// Return labor based on units consumed, not applied.
}

const QString DeProcessComponent::CommodityBitmapName() const
{
	return g_Au.CommodityBitmapName(CommodityId());
}

const char* DeProcessComponent::EffectBitmapName() const
{
	switch(m_Effect)
	{
	default:
	case CE_None:
		return 0;
	case CE_DecreasesTime:
		return ":data/mc_DecreasesTime.png";
	case CE_DecreasesMaterials:
		return ":data/mc_DecreasesMaterial.png";
	case CE_Required:
		return ":data/mc_Required.png";
	case CE_RequiredQ:
		return ":data/mc_RequiredQ.png";
	case CE_IncreasesQuantity:
		return ":data/mc_IncreasesProduction.png";
	}
}

void DeProcessComponent::operator>>(QDataStream& out) const
{
	out << (quint16)m_lCID;
	out << m_byQuality;
	out << m_Effect;
	out << m_QtyConsumed;
	out << m_QtyApplied;
	quint8 tl = 0;
	out << tl;
}

void DeProcessComponent::operator<<(QDataStream& in)
{
	quint16 cid;
	in >> cid;
	if(cid == CID_ComputerObsolete)
		cid = CID_Computer;
	m_lCID = (COMMODITYID)cid;
	in >> m_byQuality;
	in >> m_Effect;
	in >> m_QtyConsumed;
	in >> m_QtyApplied;
	quint8 tl;
	in >> tl;
}

void DeProcessComponent::Tx(QDataStream& out) const
{
	out << (quint16)m_lCID;
	out << m_byQuality;
	out << m_Effect;
	out << m_QtyConsumed;
	out << m_QtyApplied;
}

void DeProcessComponent::Rx(QDataStream& in)
{
	quint16 cid;
	in >> cid;
	m_lCID = (COMMODITYID)cid;
	in >> m_byQuality;
	in >> m_Effect;
	in >> m_QtyConsumed;
	in >> m_QtyApplied;
}

// Returns the cost of this commodity based on its quality and quality.
float DeProcessComponent::Cost() const
{
	return g_Au.CommodityQualityCost(CommodityId(), Quality());
}

void DeProcessComponent::SatisfyPower(quint8 elecSubQ)
{
	if(elecSubQ && CommodityId() == CID_Sunlight)
	{
		// If elecSubQ is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
		m_byQuality = elecSubQ;
	}
	else
	{
		// Power is always quality 255.
		m_byQuality = 255;
	}

	if(m_QtyConsumed == 0)
		m_QtyApplied = 1;
	else
		m_QtyApplied = m_QtyConsumed;
}

void DeProcessComponent::SetCommodity(COMMODITYID lCID, unsigned char byQuality)
{
	Q_ASSERT(byQuality);
	m_lCID = lCID;
	m_byQuality = byQuality;
}

// This might seem odd but cities pass us their account balance, which is a double.
// No process uses billions of cronos so the return result can be an int.
quint32 DeProcessComponent::SupplyMoney(double money)
{
	Q_ASSERT(money >= 0);									// Do not accept negative quantity of money.

	m_byQuality = 255;										// Money is always quality 255.
	if(m_QtyApplied >= m_QtyConsumed)
	{
		// No amount is needed.
		m_QtyApplied = m_QtyConsumed;						// Insure the applied qty didn't exceed the consumed qty somehow.
		return 0;
	}

	// Now we can trust that this subtraction won't roll over.
	quint32 needed = m_QtyConsumed - m_QtyApplied;
	if(money >= needed)
	{
		// Money is ample to supply all our needs.
		m_QtyApplied = m_QtyConsumed;
		return needed;
	}
	else
	{
		// Money is inadequate to supply all our needs.
		m_QtyApplied += money;
		return (quint32)money;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Returns true if all required construction components have been applied.
bool DeProcessComponentArray::IsCompleted() const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsRequired() && !(*ci).IsSatisfied())
			return false;

	// All required construction components must be happy.
	return true;
}

bool DeProcessComponentArray::WantsAnyComponent() const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if(!(*ci).IsSatisfied())
			return true;
	return false;
}

quint8 DeProcessComponentArray::LowestQualityApplied() const
{
	quint8 lowestQ = 0;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if(!(*ci).IsRequired())
			continue;	// Only consider required components.
		if(!(*ci).QtyConsumed())
			continue;	// Only consider the components that consume materials.
		if(!(*ci).Quality())
			continue;	// Only consider components that have a quality established.
		if(lowestQ > (*ci).Quality() || !lowestQ)
			lowestQ = (*ci).Quality();
	}
	return lowestQ;
}

DeProcessComponentArray::iterator DeProcessComponentArray::FindUnsatisfiedItem(COMMODITYID cid, quint8 elecSubQ, quint8 waterSubQ)
{
	for(iterator ci = begin(); ci != end(); ci++)
	{
		if((*ci).IsSatisfied())
			continue;
		if((*ci).CommodityId() == cid)
			return ci;
		if(elecSubQ && cid == CID_Electricity && (*ci).CommodityId() == CID_Sunlight)
			return ci;	// Electricity can satisfy the sunlight requirement.
		if(waterSubQ && cid == CID_Water && (*ci).CommodityId() == CID_Hydrographics)
			return ci;	// Water can satisfy the hydrographics requirement.
	}
	return end();
}

// So far, this is only called when a player is applying something from his own
// inventory to a construction or manufacturing console.
// Returns true if the inventory item is useful.
bool DeProcessComponentArray::ApplyInventoryItem(const AuItem& iItem, quint32& iMaterialConsumed)
{
	// Init return values.
	iMaterialConsumed = 0;

	// Find the inventory item in my list of ingredients.
	// No need to enable substitution of electricity for sunlight since no person will have electricity in their inventory.
	// Why not let them substitute their own bottled water? Perhaps not at a treatment plant.
	iterator ui = FindUnsatisfiedItem(iItem.CommodityId(), 0, 0);
	if(ui == end())
		return false;	// The item is not useful to this process.

	// Apply the inventory item to the construction component.
	if((*ui).ApplyMaterial(iItem.Count(), iItem.Quality(), iMaterialConsumed))
		return true;

	// The component must not need any more material.
	return false;
}

// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
// Modifies quantity in 'iItem' if any is consumed.
// Returns labor needed to consume the item(s).
quint32 DeProcessComponentArray::ApplyInventoryMaterial(AuItem& iItem, quint8 elecSubQ, quint8 waterSubQ, quint8 minQ)
{
	// Find the inventory item in my list of ingredients.
	iterator ui = FindUnsatisfiedItem(iItem.CommodityId(), elecSubQ, waterSubQ);
	if(ui == end())
		return 0;
	return ApplyInventoryMaterial(ui, iItem, elecSubQ, waterSubQ, minQ);
}

// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
quint32 DeProcessComponentArray::ApplyInventoryMaterial(iterator& myUnsatItem, AuItem& iItem, quint8 elecSubQ, quint8 waterSubQ,
		quint8 minQ)
{
	Q_ASSERT(myUnsatItem != end());
	// Commodities that are actually consumed must meet the minimum quality criteria.
	if((*myUnsatItem).QtyConsumed() && iItem.Quality() < minQ)
		return 0;

	// Apply the inventory item to the process component.
	quint32 labor;
	if(elecSubQ && (*myUnsatItem).CommodityId() == CID_Sunlight && iItem.CommodityId() == CID_Electricity)
	{
		// If elecSubQ is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
		iItem.SetCommodity(CID_Electricity, elecSubQ);
		labor = (*myUnsatItem).ApplyMaterial(iItem);
		// Restore original quality of the iItem commodity.
		// Electricity always has quality 255.
		iItem.SetCommodity(CID_Electricity, 255);
	}
	else if(waterSubQ && (*myUnsatItem).CommodityId() == CID_Hydrographics && iItem.CommodityId() == CID_Water)
	{
		// If waterSubQ is non-zero, water can be substituted for hydrographics but the waterSubQ is used.
		quint8 itemQ = iItem.Quality();
		iItem.SetCommodity(CID_Water, waterSubQ);
		labor = (*myUnsatItem).ApplyMaterial(iItem);
		// Restore original quality of the iItem commodity.
		iItem.SetCommodity(CID_Water, itemQ);
	}
	else if((*myUnsatItem).QtyConsumed() && (*myUnsatItem).QtyApplied())
	{
		// Quality is a factor when the component consumes the item.
		if((*myUnsatItem).Quality() < minQ)
		{
			qDebug("DeProcessComponentArray::ApplyInventoryMaterial iItem %d Q %d minQ %d component Q %d before "
					"ApplyMaterial", iItem.CommodityId(), iItem.Quality(), minQ, (*myUnsatItem).Quality());
			(*myUnsatItem).SetQuality(minQ);
		}
		labor = (*myUnsatItem).ApplyMaterial(iItem);
		if((*myUnsatItem).Quality() < minQ)
		{
			qDebug("DeProcessComponentArray::ApplyInventoryMaterial iItem %d Q %d minQ %d component Q %d AFTER "
					"ApplyMaterial", iItem.CommodityId(), iItem.Quality(), minQ, (*myUnsatItem).Quality());
			(*myUnsatItem).SetQuality(minQ);
		}
	}
	else
	{
		// Quality is not a factor when the component does not consume the item.
		labor = (*myUnsatItem).ApplyMaterial(iItem);
	}
	return labor;
}

bool DeProcessComponentArray::SupplyAllPower(quint8 elecSubQ)
{
	// Look for power in my list of ingredients.
	// The search will also find sunlight because electricity can substitute when elecSubQ is non-zero.
	iterator ui = FindUnsatisfiedItem(CID_Electricity, elecSubQ, 0);
	if(ui == end())
		return false;

	(*ui).SatisfyPower(elecSubQ);
	return true;
}

quint32 DeProcessComponentArray::SupplyMoney(double money)
{
	// Look for money in my list of ingredients.
	iterator ui = FindUnsatisfiedItem(CID_Money, 0, 0);
	if(ui != end())
		return (*ui).SupplyMoney(money);
	return 0;
}

// Returns labor needed to apply the item(s).
quint32 DeProcessComponentArray::ApplyNaturalConstructionResources(const AuZoneResource& res)
{
	quint32 lLabor = 0;
	for(iterator it = begin(); it != end(); it++)
	{
		if((*it).IsSatisfied())
			continue;
		int ifetched = res.Fetch((*it).CommodityId(), 1);
		if(ifetched > 0)
		{
			// The natural resources gave up some of the commodity.
			// Quality is limited by the quality of the natural resource and the quality of the building.
			quint8 quality = res.Quality((*it).CommodityId());	// 0..255
			AuItem iRes((*it).CommodityId(), quality, 0, ifetched);
			iRes.SetHitPoints(1);
			lLabor += (*it).ApplyMaterial(iRes);
		}
	}
	return lLabor;
}

// Returns labor needed to apply the item(s).
quint32 DeProcessComponentArray::ApplyNaturalManufacturingResources(const AuZoneResource& res, bool& anyChange, int workers)
{
	anyChange = false;
	quint32 lLabor = 0;
	for(iterator it = begin(); it != end(); it++)
	{
		if((*it).IsSatisfied())
			continue;
		int ifetched = res.Fetch((*it).CommodityId(), workers);
		if(ifetched > 0)
		{
			// The natural resources gave up some of the commodity.
			// Quality is limited by the quality of the natural resource.
			quint8 quality = res.Quality((*it).CommodityId());	// 0..255
			AuItem iRes((*it).CommodityId(), quality, 0, ifetched);
			iRes.SetHitPoints(1);
			lLabor += (*it).ApplyMaterial(iRes, anyChange);
		}
	}
	return lLabor;
}

void DeProcessComponentArray::ConsumeAppliedInventory()
{
	for(iterator it = begin(); it != end(); it++)
	{
		(*it).SetQtyApplied(0);
		(*it).SetQuality(0);
	}
}

/*
// Try to satisfy required items that don't get consumed when applying labor.
// Returns false if a required item is missing, e.g. a tool.
bool DeProcessComponentArray::ApplyInventoryItems(DaAtom* pByWho, bool hasPower)
{
	bool reqMissing = false;
	DeProcessComponentArray::iterator cc;
	for(cc = begin(); cc != end(); ++cc)
	{
		if(!cc->ApplyInventoryItems(pByWho, hasPower))
			reqMissing = true;
	}
	return !reqMissing;	// true if ok, false if a required item is missing.
}
*/

/*
// Returns boost resulting from this person's equipment when applying labor.
int DeProcessComponentArray::GetLaborBoost(DaAtom pByWho, bool hasPower) const
{
	int lLaborBoost = 0;
	DeProcessComponentArray::const_iterator cc;
	for(cc = begin(); cc != end(); ++cc)
		lLaborBoost += cc->GetLaborBoost(pByWho, hasPower);
	return lLaborBoost;
}
*/

/*
// Returns boost resulting from this person's equipment when applying labor.
int DeProcessComponentArray::GetProductionBoost(DaAtom* pByWho, bool hasPower) const
{
	int lProdBoost = 0;
	DeProcessComponentArray::const_iterator cc;
	for(cc = begin(); cc != end(); ++cc)
		lProdBoost += cc->GetProductionBoost(pByWho, hasPower);
	return lProdBoost;
}
*/

void DeProcessComponentArray::Tx(QDataStream& out) const
{
	out << (quint8)count();
	DeProcessComponentArray::const_iterator pc;
	for(pc = constBegin(); pc != constEnd(); ++pc)
		(*pc).Tx(out);
}

void DeProcessComponentArray::Rx(QDataStream& in)
{
	clear();
	quint8 comps;
	in >> comps;
	while(comps)
	{
		DeProcessComponent pc;
		pc.Rx(in);
		append(pc);
		comps--;
	}
}

void DeProcessComponentArray::ChangeMaterial(COMMODITYID idCommodity, COMMODITYID idNewCommodity)
{
	if(idCommodity != idNewCommodity)
		for(DeProcessComponentArray::iterator it = begin(); it != end(); it++)
			if((*it).CommodityId() == idCommodity)
				(*it).SetCommodity(idNewCommodity, (*it).Quality());
}

// returns true if all qty could be added.
void DeProcessComponentArray::AddRequirement(COMMODITYID cid, COMPEFFECT effect, quint32 qty)
{
	// Look for this requirement so we can increase the quantity. We trust that no component ever reaches a max quint32.
	for(DeProcessComponentArray::iterator it = begin(); it != end(); it++)
		if((*it).CommodityId() == cid && (*it).Effect() == effect)
		{
			// This is the appropriate commodity and it's not maxed out.
			// Increase the quantity of this requirement.
			(*it).SetQtyConsumed((*it).QtyConsumed() + qty);
			return;
		}

	// The requirement could not be combined with an existing entry so append it as a nu component.
	DeProcessComponent cc;
	cc.SetCommodity(cid, 1);
	cc.SetQtyConsumed(qty);
	cc.SetEffect(effect);
	cc.SetQtyApplied(0);
	append(cc);
}

// Returns true if all qty could be subtracted.
bool DeProcessComponentArray::SubtractRequirement(COMMODITYID cid, COMPEFFECT effect, quint32 qty)
{
	// Look for the commodity requirement so we can decrease the quantity.
	for(DeProcessComponentArray::iterator it = begin(); it != end() && qty;)
	{
		if((*it).CommodityId() == cid && (*it).Effect() == effect)
		{
			// This is the appropriate commodity.
			// Decrease the quantity of this requirement.
			if(qty >= (*it).QtyConsumed())
			{
				// The reduction will eliminate the entry entirely.
				qty -= (*it).QtyConsumed();						// Subtract the quantity
				it = erase(it);									// Delete the entry, pointing iterator at next
			}
			else
			{
				// The entire quantity can be subtracted from this entry.
				(*it).SetQtyConsumed((*it).QtyConsumed() - qty);
				qty = 0;										// We're done.
			}
			break;
		}
		else
			it++;
	}

	if(qty)
		return false;	// Could not subtract all of the quantity.
	else
		return true;	// All of the quantity was subtracted.
}

quint32 DeProcessComponentArray::ConstructionDemandTally(const AuItem& item) const
{
	quint32 totalDemand = 0;
	for(DeProcessComponentArray::const_iterator pc = constBegin(); pc != constEnd(); pc++)
	{
		// We are checking for this process' need for a specific commodity.
		if((*pc).CommodityId() != item.CommodityId())
			continue;

		// Attribute a demand value based on the quantity that is a consumed.
		// Items that are not consumed get a little value or they will never be demanded.
		if((*pc).QtyConsumed())
			totalDemand += (*pc).QtyConsumed() + 1;				// Demand the quantity of items that are consumed, plus fudge.
		else
			totalDemand += 1;									// Moderate demand for tools and other items useful to the process.
	}
	return totalDemand;
}

quint32 DeProcessComponentArray::ManufacturingDemandTally(const AuItem& item, quint8 minQ) const
{
	quint32 totalDemand = 0;
	for(DeProcessComponentArray::const_iterator pc = constBegin(); pc != constEnd(); pc++)
	{
		// We are checking for this process' need for a specific commodity.
		if((*pc).CommodityId() != item.CommodityId())
			continue;

		// Attribute a demand value based on the quantity that is a consumed.
		// Items that are not consumed get a little value or they will never be demanded.
		if((*pc).QtyConsumed())
		{
			// Commodities that are consumed by the process must meet the quality minimum for the process.
			if(item.Quality() < minQ)
				continue;
			totalDemand += (*pc).QtyConsumed() * 10;			// Highest demand for items that are consumed.
		}
		else if((*pc).IsRequired())
			totalDemand += 2;									// Moderate demand for required items that are not consumed.
		else
			totalDemand += 1;									// Slight demand for optional items that are not consumed.
	}
	return totalDemand;
}

// Returns quality of fuel applied.
unsigned char DeProcessComponentArray::FuelQuality() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).CommodityId() == CID_Hydrogen)
			return (*it).Quality();
	return 0;
}

bool DeProcessComponentArray::IsComponent(COMMODITYID cid) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).CommodityId() == cid)
			return true;
	return false;
}

bool DeProcessComponentArray::IsRequired(COMMODITYID cid) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).CommodityId() == cid && (*it).IsRequired())
		{
			if((*it).QtyConsumed() && (*it).QtyApplied() >= (*it).QtyConsumed())
				continue;	// Skip components that are fully satisfied.
			return true;
		}
	return false;
}

void DeProcessComponentArray::operator<<(QDataStream& in)
{
	clear();
	quint8 comps;
	in >> comps;
	while(comps)
	{
		DeProcessComponent pc;
		pc << in;
		append(pc);
		comps--;
	}
}

void DeProcessComponentArray::operator>>(QDataStream& out) const
{
	out << (quint8)count();
	for(DeProcessComponentArray::const_iterator pc = constBegin(); pc != constEnd(); pc++)
		*pc >> out;
}

quint32 DeProcessComponentArray::QtyApplied(COMMODITYID cid) const
{
	quint32 a = 0;
	for(DeProcessComponentArray::const_iterator pc = constBegin(); pc != constEnd(); pc++)
	{
		if((*pc).CommodityId() != cid)
			continue;
		a += (*pc).QtyApplied();
	}
	return a;
}

quint32 DeProcessComponentArray::CountHullMaterials() const
{
	quint32 a = 0;
	for(DeProcessComponentArray::const_iterator pc = constBegin(); pc != constEnd(); pc++)
	{
		if(HullMaterialModule((*pc).CommodityId()) == CID_Invalid)
			continue;	// Commodity is not a hull material.
		a += (*pc).QtyConsumed();
	}
	return a;
}

bool DeProcessComponentArray::IsAnyModuleRequired() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if(DbCommodity::IsModule((*it).CommodityId()) && (*it).IsRequired())
			return true;
	return false;
}

