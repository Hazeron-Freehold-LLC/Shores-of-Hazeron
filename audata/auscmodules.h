//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmodules.h,v 1.10 2022/02/19 18:19:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScModules_H
#define AuScModules_H

#include "audata.h"
#include "auhash.h"
#include "dbcommodity.h"

class QDataStream;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Helper class to store all the modules information for a spaceship design. It is also used in the final space ship to store
// current modules data.

class AUDATA_DLL AuScModules
{
private:
	quint16								m_CapacitorModule;						// System module COMMODITYID.
	quint16								m_CnCModule;							// System module COMMODITYID.
	quint16								m_HullModule;							// System module COMMODITYID.
	quint16								m_LifeSupportModule;					// System module COMMODITYID.
	quint16								m_ManeuverDriveModule;					// System module COMMODITYID.
	quint16								m_PowerPlantModule;						// System module COMMODITYID.

public:	// Constructors
	inline AuScModules()
	{
		m_CapacitorModule = CID_CapacitorModule;
		m_CnCModule = CID_CommandAndControlModule;
		m_HullModule = CID_MetalHullModule;
		m_LifeSupportModule = CID_LifeSupportModule;
		m_ManeuverDriveModule = CID_GravityDriveModule;
		m_PowerPlantModule = CID_HydrogenPowerPlantModule;
	}

	inline AuScModules(const AuScModules& c)
	{
		m_CapacitorModule = c.m_CapacitorModule;
		m_CnCModule = c.m_CnCModule;
		m_HullModule = c.m_HullModule;
		m_LifeSupportModule = c.m_LifeSupportModule;
		m_ManeuverDriveModule = c.m_ManeuverDriveModule;
		m_PowerPlantModule = c.m_PowerPlantModule;
	}

public:	// Operators
	inline AuScModules& operator=(const AuScModules &c)
	{
		m_CapacitorModule = c.m_CapacitorModule;
		m_CnCModule = c.m_CnCModule;
		m_HullModule = c.m_HullModule;
		m_LifeSupportModule = c.m_LifeSupportModule;
		m_ManeuverDriveModule = c.m_ManeuverDriveModule;
		m_PowerPlantModule = c.m_PowerPlantModule;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint16 modver = 0xc000 | m_CnCModule;
		out << modver;
		out << m_HullModule;
		out << m_LifeSupportModule;
		out << m_ManeuverDriveModule;
		out << m_PowerPlantModule;
		out << m_CapacitorModule;
	}

	inline void operator<<(QDataStream& in)
	{
		quint16 modver;
		in >> modver;
		if(modver & 0x8000)
		{
			m_CnCModule = modver & 0x3fff;
			in >> m_HullModule;
			in >> m_LifeSupportModule;
			in >> m_ManeuverDriveModule;
			in >> m_PowerPlantModule;
			if(modver & 0x4000)
				in >> m_CapacitorModule;
		}
		else
		{
			// Old version stored lists of modules for bays, etc. Those modules are now stored on the parts.
			// Bay modules
			quint16 which;
			quint16 module;
			quint16 c = modver;	// old version started with the count of the first hash
			while(c--)
			{
				in >> which;
				in >> module;
			}

			in >> m_CnCModule;

			// FTL drive modules.
			in >> c;
			while(c--)
			{
				in >> which;
				in >> module;
			}

			in >> m_HullModule;
			in >> m_LifeSupportModule;
			in >> m_ManeuverDriveModule;

			// Medical modules
			in >> c;
			while(c--)
			{
				in >> which;
				in >> module;
			}

			in >> m_PowerPlantModule;
			in >> module;	// Sensor module;
			in >> module;	// Shield module;

			// Transporter modules
			in >> c;
			while(c--)
			{
				in >> which;
				in >> module;
			}

			// Turret modules
			in >> c;
			while(c--)
			{
				in >> which;
				in >> module;
			}
		}
	}

public:	// Methods
	inline COMMODITYID CapacitorModule() const
	{
		return (COMMODITYID)m_CapacitorModule;
	}

	inline COMMODITYID CommandAndControlModule() const
	{
		return (COMMODITYID)m_CnCModule;
	}

	inline COMMODITYID HullModule() const
	{
		return (COMMODITYID)m_HullModule;
	}

	inline COMMODITYID LifeSupportModule() const
	{
		return (COMMODITYID)m_LifeSupportModule;
	}

	inline COMMODITYID ManeuverDriveModule() const
	{
		return (COMMODITYID)m_ManeuverDriveModule;
	}

	inline COMMODITYID PowerPlantModule() const
	{
		return (COMMODITYID)m_PowerPlantModule;
	}

	inline void SetCapacitorModule(COMMODITYID cid)
	{
		m_CapacitorModule = cid;
	}

	inline void SetCommandAndControlModule(COMMODITYID cid)
	{
		m_CnCModule = cid;
	}

	inline void SetHullModule(COMMODITYID cid)
	{
		m_HullModule = cid;
	}

	inline void SetLifeSupportModule(COMMODITYID cid)
	{
		m_LifeSupportModule = cid;
	}

	inline void SetManeuverDriveModule(COMMODITYID cid)
	{
		m_ManeuverDriveModule = cid;
	}

	inline void SetPowerPlantModule(COMMODITYID cid)
	{
		m_PowerPlantModule = cid;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScModules& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScModules& o)
{
	o << in;
	return in;
}

#endif // AuScModules_H
