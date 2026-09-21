//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auroster.h,v 1.31 2022/02/19 17:32:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRoster_H
#define AuRoster_H

#include <auid.h>
#include <QString>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuRosterEntry
{
public:
	enum TYPE
	{
		Invalid,
		Captain,		// There should only be one entry of this type.
						// Occupies one berth in this order: officer, crew,
						// passenger, troop.
		CaptainSlot,	// Used when a ship has no berths to remember the capt.
						// A roster may have a single CaptainSlot entry when
						// the ship design has no berths of any kind.
		OfficerHuman,	// Human-controlled officer.
		Crew,
		OfficerAI,		// AI-controlled officer.
		PassengerAI,	// AI-controlled passenger.
		Troop,
		PassengerHuman,	// Human-controlled passenger.
	};

public:
	quint32			m_BerthId;	// Detail id of berth in design. There can be two roster entries for each detail (2 bunks).
	AuId			m_OccId;	// atom id of occupant
	QString			m_OccName;	// name of occupant
	TYPE			m_Type;

public:
	AuRosterEntry();
	AuRosterEntry(TYPE type, quint32 berthId, const AuId& occId, const QString& occName);
	virtual ~AuRosterEntry();
	AuRosterEntry(const AuRosterEntry& c);

	const AuRosterEntry& operator=(const AuRosterEntry& c);

	// Returns a one line description of the berth type and occupant, for display on screen when spotting the berth.
	QString BerthDesc() const;

	inline const char* BerthIcon() const
	{
		return BerthIcon(m_Type);
	}

	inline static const char* BerthIcon(TYPE type)
	{
		const char* icons[] =
		{
		":data/dp_Bridge.png",
		":data/CommRecruit.png",
		":data/CommRecruit.png",
		":data/dp_CaptCabin.png",
		":data/dp_CrewCabin.png",
		":data/dp_CaptCabin.png",
		":data/dp_Stateroom.png",
		":data/dp_Barracks.png",
		":data/dp_Stateroom.png",
		};
		return icons[type];
	}

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	inline bool IsCaptainBerth() const
	{
		return m_Type == Captain || m_Type == CaptainSlot;
	}

	// Note this does not include Troop berths.
	inline bool IsCrew() const
	{
		return IsCrew(m_Type);
	}

	inline static bool IsCrew(TYPE type)
	{
		switch(type)
		{
		case Captain:
		case CaptainSlot:
		case OfficerHuman:
		case OfficerAI:
		case Crew:
			return true;
		case Invalid:
		case PassengerAI:
		case PassengerHuman:
		case Troop:
			break;
		}
		return false;
	}

	inline static bool IsCrewOrTroop(TYPE type)
	{
		switch(type)
		{
		case Captain:
		case CaptainSlot:
		case OfficerHuman:
		case OfficerAI:
		case Crew:
		case Troop:
			return true;
		case Invalid:
		case PassengerAI:
		case PassengerHuman:
			break;
		}
		return false;
	}

	inline bool IsHumanOfficer() const
	{
		return IsHumanOfficer(m_Type);
	}

	inline static bool IsHumanOfficer(TYPE type)
	{
		switch(type)
		{
		case Captain:
		case CaptainSlot:
		case OfficerHuman:
			return true;
		case OfficerAI:
		case Crew:
		case Invalid:
		case PassengerAI:
		case PassengerHuman:
		case Troop:
			break;
		}
		return false;
	}

	inline bool IsOfficer() const
	{
		return IsOfficer(m_Type);
	}

	inline static bool IsOfficer(TYPE type)
	{
		switch(type)
		{
		case Captain:
		case CaptainSlot:
		case OfficerHuman:
		case OfficerAI:
			return true;
		case Crew:
		case Invalid:
		case PassengerAI:
		case PassengerHuman:
		case Troop:
			break;
		}
		return false;
	}

	inline bool IsPassenger() const
	{
		return IsPassenger(m_Type);
	}

	inline static bool IsPassenger(TYPE type)
	{
		switch(type)
		{
		case PassengerAI:
		case PassengerHuman:
			return true;
		case Captain:
		case CaptainSlot:
		case Crew:
		case Invalid:
		case OfficerHuman:
		case OfficerAI:
		case Troop:
			break;
		}
		return false;
	}

	quint64 MemoryUsage() const;

	inline const char* OccupantIcon() const
	{
		return OccupantIcon(m_Type);
	}

	inline static const char* OccupantIcon(TYPE type)
	{
		const char* icons[] =
		{
		":data/Contacts.png",
		":data/Captain.png",
		":data/Captain.png",
		":data/Officer.png",
		":data/Crewman.png",
		":data/Officer.png",
		":data/Passenger.png",
		":data/Troop.png",
		":data/Passenger.png",
		};
		return icons[type];
	}

	inline QString OccupantName() const
	{
		return m_OccName;
	}

	QString TypeName() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// The crew roster only contains entries for berths that are occupied.

class AuRoster : public QList<AuRosterEntry>
{
public:
	AuRoster();
	virtual ~AuRoster();
	AuRoster(const AuRoster& c);

public:	// Operators
	const AuRoster& operator=(const AuRoster& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Returns a one line description of the berth type and occupant, for display on screen when spotting the berth.
	QString BerthDesc(quint32 berthId) const;

	// Returns the detail id of the berth occupied by someone.
	// Returns 0 on failure.
	quint32 BerthDetailOccupied(const AuId& occupantId) const;

	// Returns the name of a berth occupant, or blank if they are not on the roster.
	QString BerthOccupantName(const AuId& occupantId) const;

	// returns the number of occupants of a berth
	unsigned int BerthOccupants(quint32 berthId) const;

	// 'type' is the type to query.
	unsigned int BerthsOccupied(AuRosterEntry::TYPE type) const;

	// Returns the type of berth occupied by someone.
	// Returns AuRosterEntry::Invalid on failure.
	AuRosterEntry::TYPE BerthTypeOccupied(const AuId& occupantId) const;

	// Returns the atom id that occupies the captains berth.
	// Returns 0 if nobody occupies the captains berth.
	AuId CaptainId() const;

	QString CaptainName() const;

	// Removes all crew members and troops (crew, officers, and troops) from the roster.
	// Returns true if anyone was removed.
	bool ClearCrewAndTroops();

	// Returns true if any officer on the roster is an AI-controlled OfficerAI.
	bool HasAIOfficer() const;

	// Returns true if any crew members remain in the roster. i.e. Captain, Officers, or Crew.
	bool HasAnyCrew() const;

	// Returns true if any crew members or troops remain in the roster. i.e. Captain, Officers, Crew, or Troops.
	bool HasAnyCrewOrTroops() const;

	// Returns true if any officer on the roster is a human-controlled OfficerHuman.
	bool HasHumanOfficer() const;

	quint64 MemoryUsage() const;

	// Occupies a berth with someone. May be an AI controlled citizen.
	void OccupyBerth(AuRosterEntry::TYPE type, quint32 berthId, const AuId& occupantId, const QString& occupantName);

	// Returns true if the occupant is removed.
	bool RemoveOccupant(const AuId& occupantId);
};

inline QDataStream& operator<<(QDataStream& out, const AuRoster& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRoster& o)
{
	o << in;
	return in;
}

#endif // AuRoster_H
