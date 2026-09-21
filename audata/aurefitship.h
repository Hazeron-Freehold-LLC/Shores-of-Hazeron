//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurefitship.h,v 1.6 2022/02/19 18:19:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRefitShip_H
#define AuRefitShip_H

#include "audata.h"
#include "dbcommodity.h"

class AUDATA_DLL AuRefitShip
{
public:
	quint32								m_Armor;				// Armor volume in cubic meters.
	QMap<quint16, quint32>				m_Bays;
	quint32								m_Capacitor;
	quint32								m_CryoBerths;
	QMap<quint16, quint32>				m_FTLDrives;
	quint32								m_Fuel;
	quint32								m_Hold;
	quint32								m_LifeSupport;
	quint32								m_ManeuverDrive;
	quint8								m_MinQ;					// Minimum quality of parts to use.
	quint32								m_PowerPlant;
	quint32								m_Sensor;
	quint32								m_Shield;
	quint32								m_Skeleton;

public:
	AuRefitShip();
	virtual ~AuRefitShip();
	AuRefitShip(const AuRefitShip& c);

public:	// Operators
	const AuRefitShip& operator=(const AuRefitShip& c);

	const AuRefitShip& operator==(const AuRefitShip& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	static int RegisterMetaType();

	// Returns the total volume of the refit, in cubic meters.
	quint32 Volume() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuRefitShip& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRefitShip& c)
{
	c << in;
	return in;
}

Q_DECLARE_METATYPE(AuRefitShip)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is commodity id, value is count.

class AUDATA_DLL AuRefitMaterials
{
public:
	QHash<COMMODITYID, quint32>			m_Materials;

public:
	inline AuRefitMaterials()
	{
	}

	inline AuRefitMaterials(const AuRefitMaterials& c) : m_Materials(c.m_Materials)
	{
	}

public:
	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)m_Materials.size();
		for(QHash<COMMODITYID, quint32>::const_iterator it = m_Materials.cbegin(); it != m_Materials.cend(); it++)
		{
			out << (quint16)it.key();
			out << *it;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		m_Materials.clear();
		quint16 c;
		in >> c;
		while(c--)
		{
			quint16 cid;
			quint32 amt;
			in >> cid;
			in >> amt;
			m_Materials.insert((COMMODITYID)cid, amt);
		}
	}

public:
	inline void Add(COMMODITYID cid, quint32 count)
	{
		QHash<COMMODITYID, quint32>::iterator it = m_Materials.find(cid);
		if(it == m_Materials.end())
			m_Materials.insert(cid, count);
		else
			*it += count;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuRefitMaterials& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRefitMaterials& c)
{
	c << in;
	return in;
}

#endif // AuRefitShip_H
