//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audamage.cpp,v 1.12 2022/02/19 18:20:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audamage.h"
//
#include "aubdreport.h"
#include "auglobal.h"
#include "auscreport.h"

void AuRepairParts::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xff);	// No system takes 256 different repair parts.
	out << (quint8)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << (quint16)ci.key();
		out << *ci;
	}
}

void AuRepairParts::operator<<(QDataStream& in)
{
	clear();
	quint8 c;
	in >> c;
	while(c--)
	{
		quint16 cid;
		quint32 cnt;
		in >> cid;
		in >> cnt;
		insert((COMMODITYID)cid, cnt);
	}
}

quint32 AuRepairParts::HitsDamage() const
{
	quint32 parts = 0;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		parts += *ci;
	return parts * AuDamage::HitsPerPart();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDamage::AuDamage()
{
}

AuDamage::~AuDamage()
{
}

AuDamage::AuDamage(const AuDamage& c) : m_Hits(c.m_Hits), m_Parts(c.m_Parts)
{
}

const AuDamage& AuDamage::operator=(const AuDamage& c)
{
	m_Hits = c.m_Hits;
	m_Parts = c.m_Parts;
	return *this;
}

bool AuDamage::operator==(const AuDamage& c) const
{
	if(m_Hits != c.m_Hits)
		return false;
	if(m_Parts != c.m_Parts)
		return false;
	return true;
}

void AuDamage::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(!m_Hits.empty())
		hasWhat |= 0x01;
	if(!m_Parts.empty())
		hasWhat |= 0x02;
	out << hasWhat;

	if(hasWhat & 0x01)
	{
		out << (quint16)m_Hits.size();
		for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
		{
			out << hi.key();
			out << (quint32)*hi;
		}
	}

	if(hasWhat & 0x02)
	{
		out << (quint16)m_Parts.size();
		for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
		{
			out << pi.key();
			out << *pi;
		}
	}
}

void AuDamage::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	m_Hits.clear();
	if(hasWhat & 0x01)
	{
		quint16 c;
		in >> c;
		while(c--)
		{
			AuSysId sysId;
			quint32 hits;
			in >> sysId;
			in >> hits;
			m_Hits.insert(sysId, hits);
		}
	}

	m_Parts.clear();
	if(hasWhat & 0x02)
	{
		quint16 c;
		in >> c;
		while(c--)
		{
			AuSysId sysId;
			AuRepairParts parts;
			in >> sysId;
			in >> parts;
			m_Parts.insert(sysId, parts);
		}
	}
}

void AuDamage::AddHitsDamage(const AuSysId& sysId, quint32 hits)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.end())
		*hi += hits;
	else
		m_Hits.insert(sysId, hits);
}

void AuDamage::AddPartDamage(const AuSysId& sysId, COMMODITYID part, quint32 count)
{
	QHash<AuSysId, AuRepairParts>::iterator pi = m_Parts.find(sysId);
	if(pi != m_Parts.end())
		(*pi).Add(part, count);
	else
		m_Parts.insert(sysId, AuRepairParts(part, count));
}

void AuDamage::AddPartsDamage(const AuSysId& sysId, const AuRepairParts& parts)
{
	for(AuRepairParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		AddPartDamage(sysId, pi.key(), *pi);
}

void AuDamage::AddDamage(const AuSysId& sysId, COMMODITYID sysModule, quint32 hits)
{
	// Hits of damage must be at least HitsPerPart to burn a part because we count every part as HitsPerPart hits.
	if(sysModule != CID_Invalid)
	{
		int burnPerc = hits / HitsPerPart();	// Truncated so less than HitsPerPart damage will not burn out any parts.
		if(burnPerc)
		{
			// It is possible that a part burned out.
			if(g_Au.Dice().Roll(1,100) <= burnPerc)
			{
				// A part burned out.
				COMMODITYID cid = sysId.RandomPart(sysModule);
				if(cid != CID_Invalid)
				{
					AddPartDamage(sysId, cid, 1);
					hits -= HitsPerPart();
				}
			}
		}
	}

	// Add remaining hits to the system as plain hit points. The possibly burned part never accounts for all the hits.
	AddHitsDamage(sysId, hits);
}

quint32 AuDamage::HitsDamageStructural() const
{
	if(m_Hits.empty() && m_Parts.empty())
		return 0;
	quint32 dmg = 0;
	for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
		if(hi.key().IsStructural())
			dmg += *hi;
	for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
		if(pi.key().IsStructural())
			dmg += (*pi).HitsDamage();
	return dmg;
}

quint32 AuDamage::HitsDamageNonstructural() const
{
	if(m_Hits.empty() && m_Parts.empty())
		return 0;
	quint32 dmg = 0;
	for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
		if(!hi.key().IsStructural())
			dmg += *hi;
	for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
		if(!pi.key().IsStructural())
			dmg += (*pi).HitsDamage();
	return dmg;
}

quint32 AuDamage::HitsDamageAddParts(const AuSysId& sysId) const
{
	quint32 dmg = 0;
	QHash<AuSysId, quint32>::const_iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.cend())
		dmg += *hi;
	QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.find(sysId);
	if(pi != m_Parts.cend())
		dmg += (*pi).HitsDamage();
	return dmg;
}

quint32 AuDamage::HitsDamageNoParts(const AuSysId& sysId) const
{
	quint32 dmg = 0;
	QHash<AuSysId, quint32>::const_iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.cend())
		dmg += *hi;
	return dmg;
}

quint32 AuDamage::HitsDamageNoParts() const
{
	quint32 dmg = 0;
	for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
		dmg += *hi;
	return dmg;
}

void AuDamage::PartsDamage(AuRepairParts& parts) const
{
	parts.clear();
	for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
		parts.Add(*pi);
}

void AuDamage::PartsDamage(const AuSysId& sysId, AuRepairParts& parts) const
{
	// When the which id is 0 for the vehicle landing system, the caller gets the total damage to all landing systems.
	parts.clear();
	if(sysId.System() == SYS_VehicleLanding && sysId.Which() == 0)
	{
		// Return the sum of all damaged vehicle landing systems.
		parts.clear();
		for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
			if(pi.key().System() == SYS_VehicleLanding)
				parts.Add(*pi);
	}
	else
	{
		QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.find(sysId);
		if(pi != m_Parts.cend())
			parts = *pi;
	}
}

quint64 AuDamage::MemoryUsage() const
{
	quint64 mem = sizeof(AuDamage);
	mem += (sizeof(AuSysId) + sizeof(quint32)) * m_Hits.size();
	mem += (sizeof(AuSysId) + sizeof(AuRepairParts)) * m_Parts.size();
	return mem;
}

quint32 AuDamage::HitsRemainingAddParts(const AuSysId& sysId, const AuBdReport& design) const
{
	quint32 hitsMax = design.SystemHits(sysId);
	quint32 v = HitsDamageAddParts(sysId);
	if(v == 0)
		return hitsMax;
	if(v >= hitsMax)
		return 0;
	return hitsMax - v;
}

quint32 AuDamage::HitsRemainingAddParts(const AuSysId& sysId, const AuScReport& design) const
{
	quint32 hitsMax = design.SystemHits(sysId);
	quint32 v = HitsDamageAddParts(sysId);
	if(v == 0)
		return hitsMax;
	if(v >= hitsMax)
		return 0;
	return hitsMax - v;
}

quint32 AuDamage::HitsRemainingNoParts(const AuSysId& sysId, const AuBdReport& design) const
{
	quint32 hitsMax = design.SystemHits(sysId);
	quint32 v = HitsDamageNoParts(sysId);
	if(v == 0)
		return hitsMax;
	if(v >= hitsMax)
		return 0;
	return hitsMax - v;
}

quint32 AuDamage::HitsRemainingNoParts(const AuSysId& sysId, const AuScReport& design) const
{
	quint32 hitsMax = design.SystemHits(sysId);
	quint32 v = HitsDamageNoParts(sysId);
	if(v == 0)
		return hitsMax;
	if(v >= hitsMax)
		return 0;
	return hitsMax - v;
}

AuSysId AuDamage::MostDamagedSystem(quint32& damageToSystem) const
{
	damageToSystem = 0;
	AuSysId mostDmgId;
	for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
	{
		if(*hi > damageToSystem)
		{
			mostDmgId = hi.key();
			damageToSystem = *hi;
		}
	}
	return mostDmgId;
}

void AuDamage::SystemIds(QSet<AuSysId>& sysIds) const
{
	for(QHash<AuSysId, quint32>::const_iterator hi = m_Hits.cbegin(); hi != m_Hits.cend(); hi++)
		sysIds.insert(hi.key());
	for(QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.cbegin(); pi != m_Parts.cend(); pi++)
		sysIds.insert(pi.key());
}

void AuDamage::Repair(quint32 fixUnits)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.begin();
	while(hi != m_Hits.end() && fixUnits)
	{
		if(*hi > fixUnits)
		{
			// The system has more damage than the fix units. Fix them and we're done.
			*hi -= fixUnits;
			break;
		}
		else
		{
			// The system has less than or equal fixUnits damage. Fix the system entirely and keep going.
			fixUnits -= *hi;		// Reduce the number of fix units remaining.
			hi = m_Hits.erase(hi);	// Repair the system.
		}
	}
}

quint32 AuDamage::RepairParts(COMMODITYID cid, quint32 howMany, QSet<AuSysId>& systemsRepaired)
{
	quint32 totalUsed = 0;
	QHash<AuSysId, AuRepairParts>::iterator pi = m_Parts.begin();
	while(pi != m_Parts.end() && howMany)
	{
		quint32 repaired = (*pi).Repair(cid, howMany);
		if(repaired)
		{
			systemsRepaired.insert(pi.key());
			howMany -= repaired;			// Decrease the number of parts available.
			totalUsed += repaired;			// Accumulate the total number of parts we use.
		}
		if((*pi).empty())
			pi = m_Parts.erase(pi);	// Remove broken parts record for system when all the parts are gone.
		else
			pi++;
	}
	return totalUsed;
}

quint32 AuDamage::RepairParts(const AuSysId& sysId, COMMODITYID cid, quint32 howMany, QSet<AuSysId>& systemsRepaired)
{
	// When the which id is 0 for a vehicle landing system, we just need to fix any one that is broken.
	QHash<AuSysId, AuRepairParts>::iterator pi;
	if(sysId.System() == SYS_VehicleLanding && sysId.Which() == 0)
	{
		// Locate any damaged vehicle landing systems.
		pi = m_Parts.begin();
		while(pi != m_Parts.end() && pi.key().System() != SYS_VehicleLanding)
			pi++;
	}
	else
		pi = m_Parts.find(sysId);
	if(pi == m_Parts.end())
		return 0;
	quint32 repaired = (*pi).Repair(cid, howMany);
	if(repaired)
		systemsRepaired.insert(pi.key());	// Get sys id from the part we actually repaired, for vehicle landers.
	return repaired;
}

void AuDamage::RepairAll()
{
	m_Hits.clear();
	m_Parts.clear();
}

void AuDamage::RepairAll(const AuSysId& sysId)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.end())
		m_Hits.erase(hi);
	QHash<AuSysId, AuRepairParts>::iterator pi = m_Parts.find(sysId);
	if(pi != m_Parts.end())
		m_Parts.erase(pi);
}

void AuDamage::RepairAll(const AuSysId& sysId, QSet<AuSysId>& systemsRepaired)
{
	QHash<AuSysId, AuRepairParts>::iterator pi = m_Parts.find(sysId);
	if(pi != m_Parts.end())
		systemsRepaired.insert(sysId);
	RepairAll(sysId);
}

void AuDamage::RepairAllExcept(const AuSysId& sysId, const QList<COMMODITYID>& cidExceptions, QSet<AuSysId>& systemsRepaired)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.end())
		m_Hits.erase(hi);
	QHash<AuSysId, AuRepairParts>::iterator pi = m_Parts.find(sysId);
	if(pi != m_Parts.end())
	{
		// Remove parts that are not in the exceptions list. If any get removed, my system gets logged in systemsRepaired.
		if((*pi).RepairExcept(cidExceptions))
			systemsRepaired.insert(sysId);
		if((*pi).empty())
			m_Parts.erase(pi);
	}
}

void AuDamage::RepairAllHitPoints()
{
	m_Hits.clear();
}

void AuDamage::RepairAllHitPoints(const AuSysId& sysId)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.find(sysId);
	if(hi != m_Hits.end())
		m_Hits.erase(hi);
}

void AuDamage::RepairHitPoints(const AuSysId& sysId, quint32 hits)
{
	QHash<AuSysId, quint32>::iterator hi = m_Hits.find(sysId);
	if(hi == m_Hits.end())
		return;
	if(*hi > hits)
		*hi -= hits;
	else
		m_Hits.erase(hi);
}

quint32 AuDamage::PartsDamaged(const AuSysId& sysId, COMMODITYID cid) const
{
	QHash<AuSysId, AuRepairParts>::const_iterator pi = m_Parts.find(sysId);
	if(pi == m_Parts.cend())
		return 0;
	AuRepairParts::const_iterator ri = (*pi).find(cid);
	if(ri == (*pi).cend())
		return 0;
	return *ri;
}

void AuDamage::RepairCritical(const AuScReport& design)
{
	QSet<AuSysId> sysIds;
	design.SystemIds(sysIds, false);

	// Repair all control stations.
	for(QSet<AuSysId>::const_iterator si = sysIds.cbegin(); si != sysIds.cend(); si++)
		if((*si).System() == SYS_CommandAndControl)
			RepairAll(*si);

	// Critical systems will be repaired up to 5% health.
	SYSTEM critSys[] =
	{
		SYS_Capacitor,
		SYS_ManeuverDrive,
		SYS_PowerPlant,
		SYS_LifeSupport,
	};
	int critCount = sizeof(critSys) / sizeof(SYSTEM);
	for(int c=0; c<critCount; c++)
	{
		// Repair system damage up to 5%, without replacing any parts.
		AuSysId csys(critSys[c]);
		quint32 cdmg = HitsDamageAddParts(csys);
		quint32 cmax = design.SystemHits(csys);
		quint32 cdmg95 = cmax * 95 / 100;	// Hit points of damage when at 5% health.
		if(cdmg > cdmg95)
		{
			quint32 fixdmg = cdmg - cdmg95;
			RepairHitPoints(csys, fixdmg);	// Repair hit points but don't replace broken parts.
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

