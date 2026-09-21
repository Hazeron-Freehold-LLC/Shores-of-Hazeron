//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auwormhole.h,v 1.32 2022/02/19 17:33:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuWormhole_H
#define AuWormhole_H

#include "auatom.h"
#include "auid.h"
#include "aupoint.h"
#include <QMetaType>

class AuGLEffects;
class AuGLModelCache;
class DaSolarSystem;

class AUATOM_DLL AuWormhole
{
public:
	enum POLARITY
	{
		Invalid = 0,
		Positive = 1,
		Negative = 2,
		Intergalactic = 3,
		StargatePositive = 4,
		StargateNegative = 5,
		IntergalacticOneWay = 6,
		POLARITY_Count					// LAST ITEM
	};

private:
	QVector<quint32>	m_ExploredBy;	// Empire ids that have explored this.
	QString				m_Name;			// Name of wormhole if explored.
	POLARITY			m_Polarity;		// 0=Invalid, 1=positive, 2=negative
	AuId				m_ToSystem;		// Id of destination solar system. Id of dest galaxy for Intergalactic and Stargates.
	AuPoint				m_VecToDest;	// Vector to destination in parsecs, relative to the sector.
										// Galaxy loc for Stargates.

	// Not serialized.
	AuPoint				m_Loc;			// Relative to my parent atom, rotated by solar system from sector coords.

public:
	inline AuWormhole()
	{
		m_Polarity = Invalid;	// Invalid
	}
	inline AuWormhole(const QString name, POLARITY polarity, const AuId& toSystem, const AuPoint& vecToDest)
	{
		Q_ASSERT(polarity > Invalid && polarity < POLARITY_Count);
		m_Name = name;
		m_Polarity = polarity;
		m_ToSystem = toSystem;
		m_VecToDest = vecToDest;
	}

	inline AuWormhole(const AuWormhole& wh) : m_ExploredBy(wh.m_ExploredBy)
	{
		m_Name = wh.m_Name;
		m_Loc = wh.m_Loc;
		m_Polarity = wh.m_Polarity;
		m_ToSystem = wh.m_ToSystem;
		m_VecToDest = wh.m_VecToDest;
	}

	inline AuWormhole& operator=(const AuWormhole& wh)
	{
		m_ExploredBy = wh.m_ExploredBy;
		m_Name = wh.m_Name;
		m_Loc = wh.m_Loc;
		m_Polarity = wh.m_Polarity;
		m_ToSystem = wh.m_ToSystem;
		m_VecToDest = wh.m_VecToDest;
		return *this;
	}

	float CheckEntry(const DaSolarSystem* system, const AuPoint &oldloc, const AuPoint &nuloc) const;

	void CreateDeviceObjects(AuGLEffects& re, AuGLModelCache& mc);

	// Returns the wormhole drive polarity needed to enter.
	inline POLARITY DrivePolarity() const
	{
		if(m_Polarity == StargatePositive)
			return Positive;
		else if(m_Polarity == StargateNegative)
			return Negative;
		else if(m_Polarity == IntergalacticOneWay)
			return Intergalactic;
		return m_Polarity;
	}

	inline void ExploredBy(quint32 empire, const QString& nuName)
	{
		// Add empire id to explored list.
		if(!IsExploredBy(empire))
			m_ExploredBy.append(empire);
		if(!nuName.isEmpty() && m_Name.isEmpty())
			m_Name = nuName;
	}

	// Returns vector to destination system relative to the sector.
	// Length of the vector is the distance in parsecs.
	inline const AuPoint& GalaxyDestLoc() const
	{
		Q_ASSERT(IsStargate());
		return m_VecToDest;
	}

	inline bool HasName() const
	{
		return m_Name.isEmpty() ? false : true;
	}

	inline bool IsExplored() const
	{
		return m_ExploredBy.isEmpty() ? false : true;
	}

	inline bool IsExploredBy(quint32 empire) const
	{
		return m_ExploredBy.contains(empire) ? true : false;
	}

	inline bool IsIntergalactic() const
	{
		return(m_Polarity == Intergalactic || m_Polarity == IntergalacticOneWay);
	}

	inline bool IsStargate() const
	{
		return(m_Polarity == StargatePositive || m_Polarity == StargateNegative);
	}

	inline bool IsValid() const
	{
		return(m_Polarity > Invalid && m_Polarity < POLARITY_Count);
	}

	// Location is relative to solar system.
	AuPoint Location(const DaSolarSystem* system) const;

	quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	// The radius needed to safely/barely avoid hitting this wormhole.
	// Smaller than the PilotAvoidRadius.
	double ObstacleRadius() const;

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	// The radius at which pilots plan to avoid hitting this wormhole.
	// Larger than the ObstacleRadius.
	double PilotAvoidRadius() const;

	inline POLARITY Polarity() const
	{
		return m_Polarity;
	}

	void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc);

	void RenderWormholeParticles(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, POLARITY polarity);

	bool Repair(const AuPoint& vecToDest, double starDistance, const AuPoint& sysRot);

	void Rx(QDataStream& in);

	// Returns true if any of my values actually change.
	bool SetStarDistance(double distance, const AuPoint& sysRot);

	inline const AuId& ToGalaxy() const
	{
		Q_ASSERT(IsIntergalactic() || IsStargate());
		return m_ToSystem;
	}

	inline const AuId& ToSystem() const
	{
		return m_ToSystem;
	}

	void Tx(QDataStream& out) const;

	// Returns vector to destination system relative to the sector.
	// Length of the vector is the distance in parsecs.
	// Unit vector pointing +Z or -Z for stargate.
	inline AuPoint SectorVecToDestination() const
	{
		if(m_Polarity == StargatePositive)
			return AuPoint(0,0,-1);
		else if(m_Polarity == StargateNegative)
			return AuPoint(0,0,1);
		return m_VecToDest;
	}

	// Returns vector to destination system relative to the rotated solar system.
	// Length of the vector is the distance from the wormhole to the center of the solar system that contains this wormhole.
	inline const AuPoint SystemVecToDestination() const
	{
		return -m_Loc;
	}

	AuMatrix4x4 Transform() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuWormhole& wh)
{
	wh >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuWormhole& wh)
{
	wh << in;
	return in;
}

Q_DECLARE_METATYPE(AuWormhole)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

class AuWormholeArray : public QVector<AuWormhole>
{
public:
	bool HasWormhole(const AuId& toSystem, AuWormhole::POLARITY polarity) const;

	quint64 MemoryUsage() const;

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	void Rx(QDataStream& in);

	void Tx(QDataStream& out) const;
};

#endif // AuWormhole_H
