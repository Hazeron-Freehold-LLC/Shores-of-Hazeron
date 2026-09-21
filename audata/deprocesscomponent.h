//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deprocesscomponent.h,v 1.53 2022/02/19 18:19:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DEPROCESSCOMPONENT_H
#define DEPROCESSCOMPONENT_H

#include "audata.h"
#include "dbcommodity.h"
#include <QVector>

class AuZoneResource;
class AuItem;

class AUDATA_DLL DeProcessComponent
{
private:
	quint16								m_lCID;					// Id of the commodity from the database.
	quint8								m_byQuality;			// Quality of this commodity. 1-255. 0 is reserved. 255 is perfect.
	quint8								m_Effect;				// The effect of this component on the process.
	quint32								m_QtyConsumed;			// Total quantity that will be consumed by the process. May be 0.
	quint32								m_QtyApplied;			// Quantity of the commodity added to the process so far.
	
public:
	DeProcessComponent();

	// Returns false if none was needed. Sets the labor needed to apply the materials and the amount of materials consumed.
	bool ApplyMaterial(quint32 iQuantity, quint8 byQuality, quint32& iMaterialConsumed);

	// Modifies iItem. Returns labor needed to consume the item(s). Sets 'anyChange' true if any property of this component changes.
	int ApplyMaterial(AuItem& iItem, bool& anyChange);

	inline int ApplyMaterial(AuItem& iItem)
	{
		bool anyChange;
		return ApplyMaterial(iItem, anyChange);
	}

	const QString CommodityBitmapName() const;	// Returns the resource bitmap id used to represent this component.

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_lCID;
	}

	// Returns the cost of this commodity based on its quality and quality.
	float Cost() const;

	inline COMPEFFECT Effect() const
	{
		return (COMPEFFECT)m_Effect;
	}

	const char* EffectBitmapName() const;	// Returns the resource bitmap id used to represent this component.

	// Calculate labor required to apply some quantity of this material to a process.
	inline static quint32 GetLaborNeeded(quint32 iMaterialConsumed)
	{
		return iMaterialConsumed;
	}

	inline bool IsPower() const
	{
		return (CommodityId() == CID_Electricity) ? true : false;
	}

	// True if this component is required by the process, whether or not it is consumed.
	inline bool IsRequired() const
	{
		return m_Effect == CE_Required || m_Effect == CE_RequiredQ;
	}

	// True if this component is required but not consumed by the process.
	inline bool IsRequiredModifier() const
	{
		return IsRequired() && !QtyConsumed();
	}

	bool IsSatisfied() const;	// Returns true if this component is valid and has been satisfied.

	inline bool operator==(const DeProcessComponent &c) const
	{
		if(m_lCID != c.m_lCID)
			return false;
		if(m_byQuality != c.m_byQuality)
			return false;
		if(m_Effect != c.m_Effect)
			return false;
		if(m_QtyConsumed != c.m_QtyConsumed)
			return false;
		if(m_QtyApplied != c.m_QtyApplied)
			return false;
		return true;
	}

	inline bool operator!=(const DeProcessComponent &c) const
	{
		return !operator==(c);
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline quint8 Quality() const
	{
		return m_byQuality;
	}

	// Returns the amount of this commodity that has been applied to this process so far.
	inline quint32 QtyApplied() const
	{
		return m_QtyApplied;
	}

	// Returns the total amount of this commodity that is consumed by this process.
	inline quint32 QtyConsumed() const
	{
		return m_QtyConsumed;
	}

	void SatisfyPower(quint8 elecSubQ);

	void SetCommodity(COMMODITYID lCID, unsigned char byQuality);

	inline void SetEffect(COMPEFFECT effect)
	{
		m_Effect = effect;
	}

	inline void SetQtyApplied(quint32 applied)
	{
		m_QtyApplied = applied;
	}

	inline void SetQtyConsumed(quint32 consumed)
	{
		m_QtyConsumed = consumed;
	}

	// Quality can be zero here.
	inline void SetQuality(quint8 quality)
	{
		m_byQuality = quality;
	}

	// This might seem odd but cities pass us their account balance, which is a double.
	// No process uses billions of cronos so the return result can be an int.
	quint32 SupplyMoney(double money);

	// Satisfies this vehicle component if the vehicle is listed in 'vehiclesAvailable'.
	// Vehicles are never consumed by a process; they are always modifiers.
	inline bool SupplyVehicle(const QList<COMMODITYID>& vehiclesAvailable)
	{
		if(m_QtyApplied)
			return false;				// Already satisfied. No change.

		if(!vehiclesAvailable.contains(CommodityId()))
			return false;				// My commodity is not in the vehicles available list.

		m_byQuality = 1;				// Vehicle quality doesn't really matter because it's always a modifier, not consumed.
		m_QtyApplied = 1;
		return true;
	}

	void SetAppliedMaterial(quint32 iQuantity, quint8 byQuality)
	{
		Q_ASSERT(byQuality);
		m_QtyApplied = iQuantity;
		m_byQuality = byQuality;
	}

	// Data stream
	void Tx(QDataStream& out) const;
	void Rx(QDataStream& in);
};

class AUDATA_DLL DeProcessComponentArray : public QVector<DeProcessComponent>
{
public:
	void AddRequirement(COMMODITYID cid, COMPEFFECT effect, quint32 qty);

	// Returns true if the inventory item is useful.
	bool ApplyInventoryItem(const AuItem& iItem, quint32 &iMaterialConsumed);

	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	// Modifies iItem. Returns labor needed to consume the items.
	quint32 ApplyInventoryMaterial(AuItem& iItem, quint8 elecSubQ, quint8 waterSubQ, quint8 minQ);

	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	// Modifies iItem. Returns labor needed to consume the items.
	quint32 ApplyInventoryMaterial(iterator& myUnsatItem, AuItem& iItem, quint8 elecSubQ, quint8 waterSubQ, quint8 minQ);

	quint32 ApplyNaturalConstructionResources(const AuZoneResource& res);

	// 'workers' is the number of workers working the job.
	// 'anyChange' is set true if anything changes. Changes do not always require labor.
	quint32 ApplyNaturalManufacturingResources(const AuZoneResource& res, bool& anyChange, int workers);

	void ChangeMaterial(COMMODITYID idCommodity, COMMODITYID idNewCommodity);
	void ConsumeAppliedInventory();

	quint32 ConstructionDemandTally(const AuItem& item) const;

	// Returns the number of hull materials (metal, magmium, etc.). Used when upgrading the hull of a ship.
	quint32 CountHullMaterials() const;

	quint32 ManufacturingDemandTally(const AuItem& item, quint8 minQ) const;

	// Returns true if there is at least one required component that consumes something.
	inline bool DoesConsumeCommodity() const
	{
		for(const_iterator cc = constBegin(); cc != constEnd(); cc++)
			if((*cc).IsRequired() && (*cc).QtyConsumed())
				return true;	// Found a required component that consumes something.
		return false;
	}

	quint8 FuelQuality() const;	// Returns quality of fuel applied.

	bool IsAnyModuleRequired() const;

	bool IsCompleted() const;	// Returns true if all required components have been provided.

	// Returns true if 'cid' is a component in any way.
	bool IsComponent(COMMODITYID cid) const;

	// Returns true if cid IsRequired() by this process, whether or not 'cid' is consumed by this process.
	bool IsRequired(COMMODITYID cid) const;

	// Returns the lowest quality of the components that are required and consumed that have been applied.
	// 0 is returned if no required component has been applied.
	quint8 LowestQualityApplied() const;

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	// Returns the number of 'cid' commodities that have been applied to this process.
	quint32 QtyApplied(COMMODITYID cid) const;

	bool SubtractRequirement(COMMODITYID cid, COMPEFFECT effect, quint32 qty);

	// 'elecSubQ' is non-zero if it is permitted to substitute electricity for the sunlight requirement.
	// This is done in farming processes to simulate the effect of having artificial lighting.
	// Returns true if components change.
	bool SupplyAllPower(quint8 elecSubQ);

	// This might seem odd but cities pass us their account balance, which is a double.
	// No process uses billions of cronos so the return result can be an int.
	quint32 SupplyMoney(double money);

	// Returns true if any component is not satisfied, even optional components.
	bool WantsAnyComponent() const;

protected:
	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	iterator FindUnsatisfiedItem(COMMODITYID cid, quint8 elecSubQ, quint8 waterSubQ);

public:
	// Data stream
	void Tx(QDataStream& out) const;
	void Rx(QDataStream& in);
};

#endif // DEPROCESSCOMPONENT_H
