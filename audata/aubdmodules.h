//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdmodules.h,v 1.5 2022/02/19 18:18:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdModules_H
#define AuBdModules_H

#include "audata.h"
#include "auhash.h"
#include "dbcommodity.h"

class QDataStream;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Helper class to store all the modules information for a spaceship design. It is also used in the final space ship to store
// current modules data.

class AUDATA_DLL AuBdModules
{
private:
	quint16								m_CapacitorModule;						// System module COMMODITYID.
	quint16								m_HullModule;							// System module COMMODITYID.
	quint16								m_LifeSupportModule;					// System module COMMODITYID.
	quint16								m_PowerPlantModule;						// System module COMMODITYID.

public:	// Constructors
	inline AuBdModules()
	{
		m_CapacitorModule = CID_Invalid;
		m_HullModule = CID_StoneHullModule;
		m_LifeSupportModule = CID_Invalid;
		m_PowerPlantModule = CID_Invalid;
	}

	inline AuBdModules(const AuBdModules& c)
	{
		m_CapacitorModule = c.m_CapacitorModule;
		m_HullModule = c.m_HullModule;
		m_LifeSupportModule = c.m_LifeSupportModule;
		m_PowerPlantModule = c.m_PowerPlantModule;
	}

public:	// Operators
	inline AuBdModules& operator=(const AuBdModules &c)
	{
		m_CapacitorModule = c.m_CapacitorModule;
		m_HullModule = c.m_HullModule;
		m_LifeSupportModule = c.m_LifeSupportModule;
		m_PowerPlantModule = c.m_PowerPlantModule;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint8 ver = 0x00;
		out << ver;
		out << m_HullModule;
		out << m_LifeSupportModule;
		out << m_PowerPlantModule;
		out << m_CapacitorModule;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 ver;
		in >> ver;
		in >> m_HullModule;
		in >> m_LifeSupportModule;
		in >> m_PowerPlantModule;
		in >> m_CapacitorModule;
	}

public:	// Methods
	inline COMMODITYID CapacitorModule() const
	{
		return (COMMODITYID)m_CapacitorModule;
	}

	inline COMMODITYID HullModule() const
	{
		return (COMMODITYID)m_HullModule;
	}

	inline COMMODITYID LifeSupportModule() const
	{
		return (COMMODITYID)m_LifeSupportModule;
	}

	inline COMMODITYID PowerPlantModule() const
	{
		return (COMMODITYID)m_PowerPlantModule;
	}

	inline void SetCapacitorModule(COMMODITYID cid)
	{
		m_CapacitorModule = cid;
	}

	inline void SetHullModule(COMMODITYID cid)
	{
		m_HullModule = cid;
	}

	inline void SetLifeSupportModule(COMMODITYID cid)
	{
		m_LifeSupportModule = cid;
	}

	inline void SetPowerPlantModule(COMMODITYID cid)
	{
		m_PowerPlantModule = cid;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuBdModules& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdModules& o)
{
	o << in;
	return in;
}

#endif // AuBdModules_H
