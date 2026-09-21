//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auroadmap.h,v 1.7 2022/02/19 17:32:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRoadMap_H
#define AuRoadMap_H

#include "auatom.h"
#include "auid.h"
#include "auglobell.h"

class DaWorldLL;

// One segment of road or a building site; they are treated as roads for interconnectivity.
class AuRoadSegment
{
public:
	enum TYPE
	{
		Bd,
		GeoRoad,
	};

private:
	AuGlobeLLF							m_EndLL;
	double								m_EndMSL;
	AuId								m_Id;					// Id of DaBd or DaWorldLL.
	AuGlobeLLF							m_StartLL;
	double								m_StartMSL;
	TYPE								m_Type;
	float								m_Width;

public:
	AuRoadSegment(TYPE type, const AuGlobeLLF& startLL, double startMSL, const AuGlobeLLF& endLL, double endMSL, float width,
			const AuId& atomId);
	AuRoadSegment()
	{
		m_EndMSL = 0;
		m_StartMSL = 0;
		m_Type = Bd;
		m_Width = 0;
	}
	inline AuRoadSegment(const AuRoadSegment& c)
	{
		m_EndLL = c.m_EndLL;
		m_EndMSL = c.m_EndMSL;
		m_Id = c.m_Id;
		m_StartLL = c.m_StartLL;
		m_StartMSL = c.m_StartMSL;
		m_Type = c.m_Type;
		m_Width = c.m_Width;
	}

public:
	inline const AuRoadSegment& operator=(const AuRoadSegment& c)
	{
		m_EndLL = c.m_EndLL;
		m_EndMSL = c.m_EndMSL;
		m_Id = c.m_Id;
		m_StartLL = c.m_StartLL;
		m_StartMSL = c.m_StartMSL;
		m_Type = c.m_Type;
		m_Width = c.m_Width;
		return *this;
	}

	inline bool operator==(const AuRoadSegment& c) const
	{
		if(m_EndLL != c.m_EndLL)
			return false;
		if(m_EndMSL != c.m_EndMSL)
			return false;
		if(m_Id != c.m_Id)
			return false;
		if(m_StartLL != c.m_StartLL)
			return false;
		if(m_StartMSL != c.m_StartMSL)
			return false;
		if(m_Type != c.m_Type)
			return false;
		if(m_Width != c.m_Width)
			return false;
		return true;
	}

	inline bool operator!=(const AuRoadSegment& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_EndLL;
		out << m_EndMSL;
		out << m_Id;					// Id of DaBd or DaWorldLL.
		out << m_StartLL;
		out << m_StartMSL;
		out << (quint8)m_Type;
		out << m_Width;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_EndLL;
		in >> m_EndMSL;
		in >> m_Id;					// Id of DaBd or DaWorldLL.
		in >> m_StartLL;
		in >> m_StartMSL;
		quint8 ui8;
		in >> ui8;
		m_Type = (TYPE)ui8;
		in >> m_Width;
	}

public:
	bool Contains(const DaWorldLL& world, const AuGlobeLLF& ll, double searchRadius) const;

	static bool Contains(const DaWorldLL& world, const AuGlobeLLF& ll, const AuGlobeLLF& startLL, float startMSL,
			const AuGlobeLLF& endLL, float endMSL, float halfWidth, double searchRadius);

	void Dump() const;

	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline bool IsBd() const
	{
		return m_Type == Bd;
	}

	static double Length(const DaWorldLL& world, const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL, float endMSL);

	// Returns true if 'road' connects with this road segment.
	bool TestConnection(const DaWorldLL& world, const AuRoadSegment& road) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuRoadSegment& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRoadSegment& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuRoadNetwork
{
	QList<AuRoadSegment>				m_Roads;

public:
	AuRoadNetwork();
	AuRoadNetwork(const AuRoadNetwork& c) : m_Roads(c.m_Roads)
	{
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_Roads;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Roads;
	}

public:
	void Add(const AuRoadNetwork& net);

	void Add(const AuRoadSegment& road);

	void Connections(QSet<AuId>& connectedBdsAndCities) const;

	bool Contains(const AuId& id) const;

	void Dump() const;

	quint64 MemoryUsage() const;

	// Returns true if road connects with this network.
	bool TestConnection(const DaWorldLL& world, const AuRoadSegment& road) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuRoadNetwork& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRoadNetwork& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuRoadMap
{
	QList<AuRoadNetwork>				m_Networks;

public:
	AuRoadMap();
	inline AuRoadMap(const AuRoadMap& c) : m_Networks(c.m_Networks)
	{
	}

public:	// Operators
	inline const AuRoadMap& operator=(const AuRoadMap& c)
	{
		m_Networks = c.m_Networks;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Networks;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Networks;
	}

public:
	void AddRoad(const DaWorldLL& world, const AuRoadSegment& road);

	void Clear();

	void Connections(const AuId& bdId, QSet<AuId>& connectedBdsAndCities) const;

	void Dump() const;

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuRoadMap& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRoadMap& o)
{
	o << in;
	return in;
}

#endif // AuRoadMap_H
