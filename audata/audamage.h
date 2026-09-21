//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audamage.h,v 1.8 2022/02/19 18:18:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDamage_H
#define AuDamage_H

#include "audata.h"
#include "dbcommodity.h"

class AuBdReport;
class AuScReport;

class AuRepairParts : public QHash<COMMODITYID, quint32>
{
public:	// Constructors
	inline AuRepairParts(COMMODITYID cid, quint32 howMany)
	{
		insert(cid, howMany);
	}

	inline AuRepairParts()
	{
	}

public:	// Operators
    void operator>>(QDataStream& out) const;
    void operator<<(QDataStream& in);

public:	// Methods
	inline void Add(COMMODITYID cid, quint32 count)
	{
		iterator ci = find(cid);
		if(ci != end())
			*ci += count;
		else
			insert(cid, count);
	}

	inline void Add(const AuRepairParts& parts)
	{
		for(const_iterator ci = parts.cbegin(); ci != parts.cend(); ci++)
			Add(ci.key(), *ci);
	}

	inline quint32 Count() const
	{
		quint32 cnt = 0;
		for(const_iterator ci = cbegin(); ci != cend(); ci++)
			cnt += *ci;
		return cnt;
	}

	inline quint32 Count(COMMODITYID cid) const
	{
		const_iterator ci = find(cid);
		if(ci == cend())
			return 0;
		return *ci;
	}

	// Returns the equivalent hit points of damage. (10hp per part?)
	quint32 HitsDamage() const;

	// Repairs up to 'howMany' of 'cid'.
	// Returns the number actually repaired.
	inline quint32 Repair(COMMODITYID cid, quint32 howMany)
	{
		iterator ci = find(cid);
		if(ci == end())
			return 0;

		if(*ci > howMany)
		{
			*ci -= howMany;			// Reduced my number of parts needed by howMany.
			return howMany;			// Repaired howMany parts.
		}
		else if(*ci == howMany)
		{
			erase(ci);				// Repaired all howMany of my parts.
			return howMany;			// Repaired howMany parts.
		}
		else
		{
			quint32 repaired = *ci;	// Remember count of my parts.
			erase(ci);				// Repaired all my parts.
			return repaired;		// Repaired repaired parts.
		}
	}

	// Returns true if any parts are removed.
	inline bool RepairExcept(const QList<COMMODITYID>& cidExceptions)
	{
		bool anyRepaired = false;
		iterator ci = begin();
		while(ci != end())
		{
			if(cidExceptions.contains(ci.key()))
				ci++;	// Do not remove the excepted parts.
			else
			{
				ci = erase(ci);
				anyRepaired = true;
			}
		}
		return anyRepaired;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuRepairParts& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRepairParts& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDamage
{
	// Hit points of damage to each system.
	QHash<AuSysId, quint32>				m_Hits;

	// Parts needed for repairs.
	QHash<AuSysId, AuRepairParts>		m_Parts;

public:	// Construction
	AuDamage();
	~AuDamage();
	AuDamage(const AuDamage& c);

public:	// Operators
	const AuDamage& operator=(const AuDamage& c);

	bool operator==(const AuDamage& c) const;

    void operator>>(QDataStream& out) const;
    void operator<<(QDataStream& in);

public:	// Methods
	// Adds damage to a system. 'sysModule' is the module installed in the system. Some damage may be added as part damage.
	void AddDamage(const AuSysId& sysId, COMMODITYID sysModule, quint32 hits);

	// Adds 'hits' points of damage to a system.
	// 0 hits is allowed here, when building a comprehensive damage report that includes undamaged systems.
	void AddHitsDamage(const AuSysId& sysId, quint32 hits);

	// Adds 'count' units of 'part' damage to a system.
	void AddPartDamage(const AuSysId& sysId, COMMODITYID part, quint32 count);

	void AddPartsDamage(const AuSysId& sysId, const AuRepairParts& parts);

	// Returns the current hit points of damage to all systems, including aggregated hits for damaged parts.
	inline quint32 HitsDamage() const
	{
		return HitsDamageStructural() + HitsDamageNonstructural();
	}

	// Returns the current hit points of damage to a system, including aggregated hits for damaged parts.
	quint32 HitsDamageAddParts(const AuSysId& sysId) const;

	quint32 HitsDamageNoParts() const;

	quint32 HitsDamageNoParts(const AuSysId& sysId) const;

	// Returns the current hit points of damage to structural systems, including aggregated hits for damaged parts.
	quint32 HitsDamageStructural() const;

	// Returns the current hit points of damage to equipment/nonstructural systems, including aggregated hits for damaged parts.
	quint32 HitsDamageNonstructural() const;

	quint32 HitsRemainingAddParts(const AuSysId& sysId, const AuBdReport& design) const;
	quint32 HitsRemainingAddParts(const AuSysId& sysId, const AuScReport& design) const;

	quint32 HitsRemainingNoParts(const AuSysId& sysId, const AuBdReport& design) const;
	quint32 HitsRemainingNoParts(const AuSysId& sysId, const AuScReport& design) const;

	static inline quint32 HitsPerPart()
	{
		return 10;
	}

	inline bool IsDamaged() const
	{
		return !m_Hits.empty() || !m_Parts.empty();
	}

	quint64 MemoryUsage() const;

	// Returns the system id with the most damage points, as returned by the HitsDamage function.
	// The amount of damage to that system is returned in 'damageToSystem'.
	// Returns AuSysId() if no system is damaged.
	AuSysId MostDamagedSystem(quint32& damageToSystem) const;

	// Gets a list of all the parts needed for repairs.
	void PartsDamage(AuRepairParts& parts) const;

	// Gets a list of the parts needed to repair 'sysId'.
	void PartsDamage(const AuSysId& sysId, AuRepairParts& parts) const;

	// Returns the number of 'cid' damaged in system 'sysId'.
	quint32 PartsDamaged(const AuSysId& sysId, COMMODITYID cid) const;

	// Repairs 'fixUnits' hit points of damage. Does not repair parts.
	void Repair(quint32 fixUnits);

	// Repairs all damage.
	void RepairAll();

	// Repairs all hit points and parts damaged on a system.
	void RepairAll(const AuSysId& sysId);

	// Repairs all hit points and parts of a system 'sysId'.
	// 'systemsRepaired' is filled with the ids of systems that get parts installed, for quality downgrade.
	void RepairAll(const AuSysId& sysId, QSet<AuSysId>& systemsRepaired);

	// Repairs all hit points and parts damaged on a system, except parts listed in 'cidExceptions'.
	// 'systemsRepaired' is filled with the ids of systems that get parts installed, for quality downgrade.
	void RepairAllExcept(const AuSysId& sysId, const QList<COMMODITYID>& cidExceptions, QSet<AuSysId>& systemsRepaired);

	// Repairs all hit points of damage. Does not repair parts.
	void RepairAllHitPoints();

	// Repairs all hit points of damage to a system. Does not repair parts.
	void RepairAllHitPoints(const AuSysId& sysId);

	// Called when a ship is recovered from destruction. Fixes just enough to keep the thing in orbit and provide life support.
	void RepairCritical(const AuScReport& design);

	// Repairs hit points of damage. Does not repair parts.
	void RepairHitPoints(const AuSysId& sysId, quint32 hits);

	// Repairs 'cid' in all systems that need it, up to 'howMany'.
	// Repairs all plain hit points of damage.
	// 'systemsRepaired' is filled with the ids of systems that get parts installed, for quality downgrade.
	// Returns the number of parts acutally used to do the repairs.
	quint32 RepairParts(COMMODITYID cid, quint32 howMany, QSet<AuSysId>& systemsRepaired);

	// Repairs 'cid' in a system 'sysId', up to 'howMany'.
	// Repairs all plain hit points of damage to 'sysId'.
	// 'systemsRepaired' is filled with the ids of systems that get parts installed, for quality downgrade.
	// Returns the number of parts acutally used to do the repairs.
	quint32 RepairParts(const AuSysId& sysId, COMMODITYID cid, quint32 howMany, QSet<AuSysId>& systemsRepaired);

	// Retrieves ids of all known systems. This would normally only include damaged systems. When building a damage report, entries
	// are added with 0 hit points so all systems are represented.
	void SystemIds(QSet<AuSysId>& sysIds) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuDamage& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDamage& o)
{
	o << in;
	return in;
}

#endif // AuDamage_H
