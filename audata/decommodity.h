//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: decommodity.h,v 1.62 2022/02/19 18:19:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DECOMMODITY_H
#define DECOMMODITY_H

#include "audata.h"
#include <auglobal.h>
#include <dbcommodity.h>

class SPoint;
class QImage;

class AUDATA_DLL DeCommodity  
{
protected:
	quint16								m_lCID;					// Id of the commodity from the database.
	quint8								m_byQuality;			// Quality of this commodity. 1-255. 0 is reserved. 255 is perfect.

public:
	inline DeCommodity()
	{
		m_lCID = CID_Invalid;
		m_byQuality = 1;
	}

	// Returns the small square resource bitmap id used to represent this
	// component on signs and things.
	inline const QString BitmapName() const
	{
		return g_Au.CommodityBitmapName(CommodityId());
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_lCID;
	}

	// Returns the commodity name of this item.
	QString CommodityName() const;

	// Returns the cost of this commodity based on its quality.
	float Cost() const;

	//GLuint CreateDeviceObject(unsigned int uiTextureOwner, SGLEffects& re,
			//const SPoint &fSize, bool bright);

	// Returns the commodity id of the helmet that is intended to go with a specific suit commodity.
	inline static COMMODITYID HelmetForSuit(COMMODITYID suitCID)
	{
		switch(suitCID)
		{
		case CID_VacSuit:
			return CID_VacSuitHelmet;
		case CID_MetalPowerArmor:
			return CID_MetalPowerArmorHelmet;
		case CID_MagmiumPowerArmor:
			return CID_MagmiumPowerArmorHelmet;
		case CID_VulciumPowerArmor:
			return CID_VulciumPowerArmorHelmet;
		default:
			return CID_Invalid;
		}
	}

	inline bool IsEnvironmentHelmet() const
	{
		return IsEnvironmentHelmet(CommodityId());
	}

	inline static bool IsEnvironmentHelmet(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_VacSuitHelmet:
		case CID_MetalPowerArmorHelmet:
		case CID_MagmiumPowerArmorHelmet:
		case CID_VulciumPowerArmorHelmet:
			return true;
		default:
			return false;
		}
	}

	inline bool IsEnvironmentSuit() const
	{
		return IsEnvironmentSuit(CommodityId());
	}

	inline static bool IsEnvironmentSuit(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_VacSuit:
		case CID_MetalPowerArmor:
		case CID_MagmiumPowerArmor:
		case CID_VulciumPowerArmor:
			return true;
		default:
			return false;
		}
	}

	inline bool IsFreeToFriendlyGovShip() const
	{
		return IsFreeToFriendlyGovShip(CommodityId());
	}

	static inline bool IsFreeToFriendlyGovShip(COMMODITYID cid)
	{
		return DbCommodity::IsFreeToFriendlyGovShip(cid);
	}

	inline bool IsFuel() const
	{
		switch(CommodityId())
		{
		case CID_Electricity:	// Sort of like fuel. Works in this category for all the purposes IsFuel is called so far.
		case CID_Coal:			// Fuel for coal power plant.
		case CID_Gasoline:		// Fuel for vehicles and gasoline power plant.
		case CID_Hydrogen:		// Fuel for spacecraft and hydrogen power plant.
		case CID_NaturalGas:	// Fuel for natural gas power plant.
		case CID_Radioactives:	// Fuel for nuclear power plant.
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsPower() const
	{
		//DHH should this be in deinventoryitem so it can test combits instead?
		return (CommodityId() == CID_Electricity) ? true : false;
	}

	inline bool IsValid() const
	{
		return CommodityId() != CID_Invalid;
	}

	inline bool operator==(const DeCommodity &c) const
	{
		if(m_lCID != c.m_lCID)
			return false;
		if(m_byQuality != c.m_byQuality)
			return false;
		return true;
	}

	inline bool operator!=(const DeCommodity &c) const
	{
		return !operator==(c);
	}

	// Returns the filename of the bitmap used to represent this component in a
	// pack. Aspect ratio should match the pack volume requred.
	inline const QString PackBitmapName() const
	{
		//DHH This will work until I have pack pictures at the proper aspect ratio.
		return g_Au.CommodityBitmapName(CommodityId());
	}

	inline unsigned char Quality() const
	{
		return m_byQuality;
	}

	// Returns word to describe this commodity's quality
	inline QString QualityDesc() const
	{
		return DbCommodity::QualityDesc(m_byQuality);
	}

	inline void SetCommodity(COMMODITYID lCID, unsigned char byQuality)
	{
		Q_ASSERT(byQuality);
		m_lCID = lCID;
		m_byQuality = byQuality;
	}

	// Calculates the failure rate of a certain quantity of this commodity, based on quality.
	// Returns the number of units of 'count' that do NOT fail due to quality failure rate.
	// The number returned will never exceed 'count'.
	inline quint32 TestFailure(quint32 count) const
	{
		return DbCommodity::TestFailure(m_byQuality, count);
	}

	// Performs a random test to determine failure of a single item due to quality.
	// Returns true if the commodity did not fail.
	// Returns false if the commodity failed.
	inline bool TestQuality() const
	{
		return DbCommodity::TestQuality(m_byQuality);
	}
};

#endif // DECOMMODITY_H
