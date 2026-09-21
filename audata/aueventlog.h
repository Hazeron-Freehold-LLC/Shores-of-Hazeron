//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aueventlog.h,v 1.54 2022/02/19 18:19:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEventLog_H
#define AuEventLog_H

#include "audata.h"
#include "auid.h"
#include "autime.h"
#include <QDataStream>
#include <QString>
#include <QList>

class AUDATA_DLL AuEventLogEntry
{
public:
	// NOTE: If you add a TYPE, update the AuEventLogEntry::TypeImageName and AuEventLogEntry::TypeName functions.
	enum TYPE
	{
		Invalid = 0,
		EnemyComm,			// Enemy communication was detected.
		EnemyVehicle,		// Enemy ground vehicle was detected.
		NeutralComm,		// Neutral communication was detected.
		NeutralVehicle,		// Neutral vehicle was detected.
		FriendComm,			// Friendly communication was detected.
		FriendVehicle,		// Friendly vehicle was detected.
		EmpireComm,			// Empire communication was detected.
		EmpireVehicle,		// Empire vehicle was detected.
		EnemyAircraft,		// Enemy aircraft was detected.
		EnemyBoat = 10,		// Enemy boat was detected.
		EnemyCapital,		// Enemy city was detected.
		EnemyCity,			// Enemy city was detected.
		EnemySpacecraftHull,
		EnemyStation,
		EnemyStationS,
		EnemySpaceship,
		EnemySpaceshipS,
		//unused
		//unused
		EnemyStarship = 20,
		EnemyStarshipS,
		DamageReceived,
		EnemyWeaponRangeIn,				// Entered enemy weapon range.
		EnemyWeaponRangeOut,			// Departed enemy weapon range.
		Disease,
		Poison,
		Infestation,					// Rats
		Medical,						// Medical log entry.
		CitizenDNAChange,
		KickedStowaway = 30,
		StandingDown,					// Officer standing down for boarding.
		Payday,
		Position,						// Position update.
		BuildingDestroyed,
		FlagLowered,					// string is 'empire name' of attacker
		BuildingAttacked,				// string is 'atom name of empire name' of attacker
		CitizenAttacked,				// string is 'atom name of empire name' of attacker
		EnemySpacecraftSpotted,			// string is DaVessel::NameWithEmpire of the ship, e.g. Imperium Starship Enterprise
		EnemyTroopSpotted,				// string is 'empire name' of troop
		EnemyVehicleSpotted = 40,		// string is 'empire name vehicle name' of vehicle
		Beacon,
		Anomaly,
		EnemyBase,						// Enemy military base hq was detected.
		ShipmentInPlunderedSector,		// Outgoing broker TEU shipment was plundered by pirates with a base in the sector.
		ShipmentInPlunderedSystem,		// Incoming broker TEU shipment was plundered by pirates with a base in the system.
		ShipmentOutPlunderedSector,		// Outgoing broker TEU shipment was plundered by pirates with a base in the sector.
		ShipmentOutPlunderedSystem,		// Incoming broker TEU shipment was plundered by pirates with a base in the system.
		AsteroidDetected,				// Asteroid detected in sensor range.
		//unused
		AsteroidObserved = 50,			// Asteroid observed by observatory.
		AsteroidObservedCollide,		// Asteroid observed by observatory. Will collide with planet.
		TradeProposal,
		TradeAgreement,
		AsteroidCollisionBlackHole,
		AsteroidCollisionGasGiant,
		AsteroidCollisionMoon,
		AsteroidCollisionPlanet,
		AsteroidCollisionPlanetoid,
		AsteroidCollisionRingworld,
		AsteroidCollisionSun,
		AsteroidCollisionTitan,
		AsteroidCollisionMoonDestroyed,
		AsteroidCollisionPlanetDestroyed,
		AsteroidCollisionPlanetoidDestroyed,
		AsteroidCollisionTitanDestroyed,
		AsteroidDepartedDeepSpace,
	};	// Must not exceed 127.
	// NOTE: If you add a TYPE, update the AuEventLogEntry::TypeImageName and AuEventLogEntry::TypeName functions.

public:
	AuId				m_AtomId;	// Relevant atom id, if any. If not null, only the type and atom id has to match in IsRepeat.
	QString				m_Text;		// Text of the log entry.
	AuTime				m_Time;		// Date and time when this log entry was created.
	TYPE				m_Type;		// Type of log entry.

public:	// Construction
	inline AuEventLogEntry(TYPE type, const QString& text=QString(""), const AuId& atomId=AuId())
	{
		m_AtomId = atomId;
		m_Text = text;
		m_Time.Set();
		m_Type = type;
	}

	inline AuEventLogEntry()
	{
		m_Type = Invalid;
	}

	inline AuEventLogEntry(const AuEventLogEntry& c)
	{
		m_AtomId = c.m_AtomId;
		m_Text = c.m_Text;
		m_Time = c.m_Time;
		m_Type = c.m_Type;
	}

public:	// Operators
	inline AuEventLogEntry& operator=(const AuEventLogEntry& c)
	{
		m_AtomId = c.m_AtomId;
		m_Text = c.m_Text;
		m_Time = c.m_Time;
		m_Type = c.m_Type;
		return *this;
	}

	inline bool operator==(const AuEventLogEntry& c) const
	{
		if(m_AtomId != c.m_AtomId)
			return false;
		if(m_Type != c.m_Type)
			return false;
		if(m_Time != c.m_Time)
			return false;
		if(m_Text != c.m_Text)
			return false;
		return true;
	}

	inline bool operator!=(const AuEventLogEntry& c) const
	{
		return !operator==(c);
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:	// Methods
	bool IsAlert() const;

	// Compares two log entries for repetition. Time value is not compared.
	inline bool IsRepeat(const AuEventLogEntry& c) const
	{
		if(m_Type != c.m_Type)
			return false;
		if(!m_AtomId.IsNull() && m_AtomId == c.m_AtomId)
			return true;		// Non null atom id matches so its a repeat.
		if(m_Text != c.m_Text)
			return false;

		// Allow payday events to duplicate. The pay is often the same but we want to see each one.
		if(m_Type == Payday)
			return false;

		return true;
	}

	quint64 MemoryUsage() const;

	QString Text() const;

	const char* TypeImageName() const;

	QString TypeName() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuEventLogEntry& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuEventLogEntry& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#if defined(WIN32)
// qHash is needed to use AuEventLogEntry in a QList in Windows.
AUDATA_DLL unsigned int qHash(const AuEventLogEntry& key);
#endif

class AUDATA_DLL AuEventLog : public QList<AuEventLogEntry>
{
public:	// Constructors

public:	// Operators
	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline bool operator==(const AuEventLog& c) const
	{
		if(count() != c.count())
			return false;

		for(const_iterator ei = constBegin(), ci = c.constBegin(); ei != constEnd() && ci != c.constEnd(); ei++, ci++)
			if(*ei != *ci)
				return false;

		return true;
	}

	inline bool operator!=(const AuEventLog& c) const
	{
		return !operator==(c);
	}

public:
	// Adds a log entry.
	// Keeps only the last matching entry that is logged since the final position entry, ignoring the timestamp in the comparison.
	// This is so multiple events reporting the same enemy unit at the same position will only result in one log entry.
	// Entries that are more than three days old are removed during this call.
	void AddEventTrim(const AuEventLogEntry& entry);

	// Adds all entries in 'log' to this log, using AddEventTrim.
	void AddLogTrim(const AuEventLog& log);

	inline void Clear(const AuTime& before)
	{
		iterator ei = begin();
		while(ei != end() && (*ei).m_Time < before)
			ei = erase(ei);
	}

	// Returns true if this event log contains anything that should cause the 'alert' icon to show on the ShipLog mail message.
	// Only checks events nuer than the 'asOf' date/time.
	inline bool ContainsAlert(const AuTime& asOf) const
	{
		if(isEmpty())
			return false;

		for(const_iterator ei = constEnd()-1; ei != constBegin()-1; ei--)
		{
			// Events are listed in chronological order, oldest to nuest.
			// We can stop once we find an event that is too old.
			if((*ei).m_Time <= asOf)
				break;

			if((*ei).IsAlert())
				return true;
		}
		return false;
	}

	// Returns true if any item is nuer than the 'asOf' date/time.
	inline bool ContainsNewerEvent(const AuTime& asOf) const
	{
		if(isEmpty())
			return false;

		// Events are listed in chronological order, oldest to nuest.
		if(last().m_Time > asOf)
			return true;			// Last event is nuer than the 'asOf' date/time.

		// Event log does not contain any nuer events.
		return false;
	}

	quint64 MemoryUsage() const;

	// Lists a bullet for each alert item, terminated by <br>.
	QString ReportAlertsHtml() const;

	// Only items nuer than the 'asOf' date/time are reported.
	QString ReportHtml(const QString& title, const AuTime& asOf) const;

	QString ReportHtmlToolTip(const QString& title) const;

protected:
	// Adds a log entry.
	inline void AddEvent(const AuEventLogEntry& entry)
	{
		append(entry);

		// Insure the event log does not accumulate too many entries.
		while(count() > 0xffff)
			removeFirst();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuEventLog& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuEventLog& c)
{
	c << in;
	return in;
}

#endif // AuEventLog_H
