//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austation.h,v 1.11 2022/02/19 18:19:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStation_H
#define AuStation_H

#include "audata.h"
#include "aumatrix4x4.h"
#include "aupose.h"

class AUDATA_DLL AuStation
{
public:
	// Station operators assume a certain body position to fit into the station. Stations could be control consoles, berths, gun
	// turrets, or vehicles. Basically, this is anything an avatar can 'E' into.
	enum OCCUPANT_POSITION
	{
		Occupant_Sit,					// Occupant can turn in Z, and look up/down, e.g. swivel chair or old console/screen combo.
		Occupant_SitStationary,			// Occupant cannot turn body but can turn head and look up/down.
		Occupant_SitSwivel3D,			// Occupant's body turns in all 3D directions, perhaps a turret position.
		Occupant_Stand,					// Occupant stands, can turn in Z and look up/down.
		Occupant_StandStationary,		// Occupant stands, cannot turn body but can turn head and look up/down.
		Occupant_Prone,					// Occupant lies prone. e.g. Medical patient, person in berth.
	};

	enum TYPE
	{
		Console_Command,
		Console_Engine,
		Console_FireControl,
		Console_Helm,
		Console_Medic,
		Console_Navigation,
		Console_Patient,
		Console_Power,
		Console_Sensor,
		Console_Shield,
		Console_Transporter,
		Integrated_Command,
		Integrated_Engine,
		Integrated_FireControl,
		Integrated_Helm,
		Integrated_Medic,
		Integrated_Navigation,
		Integrated_Patient,
		Integrated_Power,
		Integrated_Sensor,
		Integrated_Shield,
		Integrated_Transporter,
		Turret_Integrated,				// Turret station, position based on corresponding part.
		Turret_Bottom,					// Turret on bottom pointing down.
		Turret_Top,						// Turret on top pointing up.
		Turret_0,						// Turret on wall pointing to the right.
		Turret_45,						// Turret on wall pointing forward right.
		Turret_90,						// Turret on wall pointing forward.
		Turret_135,						// Turret on wall pointing forward left.
		Turret_180,						// Turret on wall pointing left.
		Turret_225,						// Turret on wall pointing back left.
		Turret_270,						// Turret on wall pointing back.
		Turret_315,						// Turret on wall pointing back right.
		Table_Game,						// Game table, position based on corresponding part.
		Table_Game_0,
		Table_Game_45,
		Berth_Captain,					// Berth for a captain, position based on corresponding part.
		Berth_Crew,						// Berth for a crewman, position based on corresponding part.
		Berth_Officer,					// Berth for an officer, position based on corresponding part.
		Berth_Passenger,				// Berth for a passenger, position based on corresponding part.
		Berth_Troop,					// Berth for a troop, position based on corresponding part.
		Berth_1_0,						// Berth for one person at angle 0.
		Berth_1_45,						// Berth for one person at angle 45.
		Berth_1_90,						// Berth for one person at angle 90.
		Berth_1_135,					// Berth for one person at angle 135.
		Berth_1_180,					// Berth for one person at angle 180.
		Berth_1_225,					// Berth for one person at angle 225.
		Berth_1_270,					// Berth for one person at angle 270.
		Berth_1_315,					// Berth for one person at angle 315.
		Berth_2_0,						// Berth for two people at angle 0.
		Berth_2_45,						// Berth for two people at angle 45.
		Berth_2_90,						// Berth for two people at angle 90.
		Berth_2_135,					// Berth for two people at angle 135.
		Berth_2_180,					// Berth for two people at angle 180.
		Berth_2_225,					// Berth for two people at angle 225.
		Berth_2_270,					// Berth for two people at angle 270.
		Berth_2_315,					// Berth for two people at angle 315.
		Troop,							// Troop post, position based on corresponding part.
		Citizen,						// Citizen post, position based on corresponding part.
		Livestock,						// Livestock post, position based on corresponding part.
		Integrated_Designer,
		TYPE_Count						// LAST ITEM
	};

private:	// Members
	OCCUPANT_POSITION					m_OccPos;				// Body position required when occupying this station.
	AuMatrix4x4							m_OpM;					// Matrix to position operator at this station.
	unsigned int						m_SeatIndex;			// Seat index of station.
	TYPE								m_Type;					// Type of this station.

public:	// Constructors
	inline AuStation(TYPE stationType, const AuMatrix4x4& opM, OCCUPANT_POSITION occPos, unsigned int seatIndex)
	{
		m_OccPos = occPos;
		m_OpM = opM;
		m_SeatIndex = seatIndex;
		m_Type = stationType;
	}

	inline AuStation()
	{
		m_OccPos = Occupant_Sit;
		m_SeatIndex = 0;
		m_Type = TYPE_Count;
	}

	inline AuStation(const AuStation& c) : m_OpM(c.m_OpM)
	{
		m_OccPos = c.m_OccPos;
		m_SeatIndex = c.m_SeatIndex;
		m_Type = c.m_Type;
	}

public:	// Operators
	inline const AuStation& operator=(const AuStation& c)
	{
		m_OccPos = c.m_OccPos;
		m_OpM = c.m_OpM;
		m_SeatIndex = c.m_SeatIndex;
		m_Type = c.m_Type;
		return *this;
	}

public:	// Methods
	// Command stations are operated by officers.
	inline bool IsCommand() const
	{
		return IsCommand(Type());
	}

	inline static bool IsCommand(TYPE type)
	{
		switch(type)
		{
		case Console_Command:
		case Integrated_Command:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsControl() const
	{
		return IsControl(Type());
	}

	// Control stations are operated by crew.
	inline static bool IsControl(TYPE type)
	{
		switch(type)
		{
		case Console_Engine:
		case Console_FireControl:
		case Console_Helm:
		case Console_Medic:
		case Console_Navigation:
		case Console_Power:
		case Console_Sensor:
		case Console_Shield:
		case Console_Transporter:
		case Integrated_Engine:
		case Integrated_FireControl:
		case Integrated_Helm:
		case Integrated_Medic:
		case Integrated_Navigation:
		case Integrated_Power:
		case Integrated_Sensor:
		case Integrated_Shield:
		case Integrated_Transporter:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsFireControl() const
	{
		return IsFireControl(Type());
	}

	inline static bool IsFireControl(TYPE type)
	{
		switch(type)
		{
		case Console_FireControl:
		case Integrated_FireControl:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsGameTable() const
	{
		return IsGameTable(Type());
	}

	inline static bool IsGameTable(TYPE type)
	{
		switch(type)
		{
		case Table_Game:
		case Table_Game_0:
		case Table_Game_45:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsHelm() const
	{
		return IsHelm(Type());
	}

	inline static bool IsHelm(TYPE type)
	{
		switch(type)
		{
		case Console_Helm:
		case Integrated_Helm:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsMedic() const
	{
		return IsMedic(Type());
	}

	inline static bool IsMedic(TYPE type)
	{
		switch(type)
		{
		case Console_Medic:
		case Integrated_Medic:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsNavigation() const
	{
		return IsNavigation(Type());
	}

	inline static bool IsNavigation(TYPE type)
	{
		switch(type)
		{
		case Console_Navigation:
		case Integrated_Navigation:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsPatient() const
	{
		return IsPatient(Type());
	}

	inline static bool IsPatient(TYPE type)
	{
		switch(type)
		{
		case Console_Patient:
		case Integrated_Patient:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsStationary() const
	{
		switch(m_OccPos)
		{
		case Occupant_SitStationary:
		case Occupant_StandStationary:
			return true;
		case Occupant_Sit:
		case Occupant_SitSwivel3D:
		case Occupant_Stand:
		case Occupant_Prone:
			break;
		}
		return false;
	}

	inline bool IsTransporter() const
	{
		return IsTransporter(Type());
	}

	inline static bool IsTransporter(TYPE type)
	{
		switch(type)
		{
		case Console_Transporter:
		case Integrated_Transporter:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsTurret() const
	{
		return IsTurret(Type());
	}

	inline static bool IsTurret(TYPE type)
	{
		switch(type)
		{
		case Turret_Bottom:
		case Turret_Top:
		case Turret_0:
		case Turret_45:
		case Turret_90:
		case Turret_135:
		case Turret_180:
		case Turret_225:
		case Turret_270:
		case Turret_315:
		case Turret_Integrated:
			return true;
		default:
			break;
		}
		return false;
	}

	static QString NameInBuilding(TYPE type);

	static QString NameInSpacecraft(TYPE type);

	inline OCCUPANT_POSITION OccPos() const
	{
		return m_OccPos;
	}

	static OCCUPANT_POSITION OccPosForPose(const AuPose& pose);

	inline void OffsetOpM(const AuPoint& offset)
	{
		m_OpM.Translate(offset);
	}

	inline const AuMatrix4x4& OpM() const
	{
		return m_OpM;
	}

	// Returns the general body pose required of the operator.
	// 'seatIndex' applies only to game tables.
	static AuPose OpPose(TYPE stationType, OCCUPANT_POSITION occPos, int seatIndex);

	inline AuPose OpPose() const
	{
		return OpPose(Type(), OccPos(), SeatIndex());
	}

	// Height of standard console seat.
	inline static double SeatHeight()
	{
		return 20./12.;
	}

	inline unsigned int SeatIndex() const
	{
		return m_SeatIndex;
	}

	inline TYPE Type() const
	{
		return m_Type;
	}
};

#endif // AuStation_H
