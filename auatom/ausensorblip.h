//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausensorblip.h,v 1.65 2022/02/19 17:32:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSensorBlip_H
#define AuSensorBlip_H

#include "auatom.h"
#include "aueventlog.h"
#include "auglobell.h"
#include "auid.h"
#include "aupoint.h"
#include "auspecies.h"
#include "dastar.h"

class AuZoneResource;
class QDebug;

class AUATOM_DLL AuSensorBlip
{
public:
	// Add nu types at the end of this enum for the least impact on the world. If any existing enum values change, the network
	// version must be changed and all servers and clients must be rebuilt and restarted.
	// To my knowledge, the sensor blip type value never gets stored in any persistent storage, like a file or a database.
	enum TYPE
	{
		Invalid,
		Aircraft,
		Animal,
		Avatar,							// a player character
		Boat,
		Citizen,
		Crew,
		GasGiant,
		GroundVehicle,
		Titan,
		Moon,
		Officer,
		Passenger,
		Planet,
		Ring,							// a rocky ring, not a ringworld
		RingSection,					// a ringworld arc section
		SpacecraftHiding,				// spacecraft hiding in ground clutter: no power plant, drives, sensors or shields
		Station,						// station
		StationShielded,				// station, shields up
		Spaceship,						// maneuverable spacecraft with no FTL drives
		SpaceshipShielded,				// maneuverable spacecraft with no FTL drives, shields up
		StarCompanion,
		StarPrimary,
		Starship,						// maneuverable spacecraft with FTL drives
		StarshipShielded,				// maneuverable spacecraft with FTL drives, shields up
		TransporterSpacecraft,			// transporter room aboard a maneuverable spacecraft
		TransporterStation,				// transporter room aboard a station
		Troop,
		BlackHole,						// black hole
		Missile,
		Beacon,
		Anomaly,
		BdBase,							// Military building.
		BdBaseHQ,						// Military headquarters building.
		BdCapitolCity,					// Civilian city capitol.
		BdCapitolEmpire,				// Civilian empire capitol. Could also be a sector capitol.
		BdCapitolSector,				// Civilian sector capitol.
		BdCity,							// Civilian building.
		TransporterBd,					// Transporter room in a bd building
		BdCapitolGalaxy,				// Civilian galaxy capitol.
		NeutronStarPrimary,				// neutron star
		NeutronStarCompanion,			// neutron star
		Asteroid,
		Planetoid,						// Planetoid in an asteroid belt.
		BlipTypes						// LAST ENTRY EXPECTED NOT TO EXCEED A BYTE. ADD NEW TYPES BEFORE THIS ENTRY.
	};

protected:
	AuId								m_AtomId;				// Atom id responsible for generating this blip.
	AuPoint								m_SysLoc;				// Location of the blip relative to the solar system.

public:
	AuSensorBlip(const AuId& id, const AuPoint& sysLoc);
	AuSensorBlip();
	virtual ~AuSensorBlip();
	AuSensorBlip(const AuSensorBlip& c);

	// Distance at which anomalous things are detected on scanners. e.g. active henge portal.
	static inline double AnomalyDetectionRange()
	{
		return OrbitAltAboveAtm() + AtmosphereTopMSL()*2;
	}

	// Squared AnomalyDetectionRange.
	static inline double AnomalyDetectionRange2()
	{
		return AnomalyDetectionRange() * AnomalyDetectionRange();
	}

	inline const AuId& AtomId() const
	{
		return m_AtomId;
	}

	virtual const char* BlipImage() const
	{
		return "";
	}

	// Returns true if this object can be harvested using a weapon bay equipped with 'bayCID' module.
	virtual bool CanBeHarvested(COMMODITYID /*bayModule*/) const
	{
		return false;
	}

	// Returns a duplicate of this condition, allocated on the heap.
	virtual AuSensorBlip* Clone() const = 0;

	// Allocates the appropriate comm packet object on the heap based on the value returned by Type() for each derived class.
	// Useful when reading a comm packet from a data stream.
	static AuSensorBlip* Create(TYPE type);

	// Calculate 2D distance from this blip to a pixel, when projected onto a display screen.
	// 'mScreen' is the projection to the screen.
	// 'pxHit' is the pixel location hit on the screen.
	virtual double DisplayDistance(const AuMatrix4x4& mScreen, const QPoint& pxHit, const DaAtom* /*system*/,
			float /*feetPerPixel*/) const
	{
		AuPoint fpx = mScreen.Project(EnvLocation());
		return AuPoint(pxHit).Distance_2D(fpx);
	}

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	// Draws the vertical line from the system plane to the blip.
	void DrawBlipStem(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	virtual quint32 EmpireId() const
	{
		return 0;
	}

	// This is here for now because it's the only object that needs it.
	static QString FormatBigNumber(double bigNumber);

	// Construct a single line description suitable for the city report, in a cell of the enemy contacts table.
	// Uses InfoLines.
	QString HtmlDesc(const DaAtom* system, bool includeSystemName=false) const;

	// Builds an html table row with 'columns' columns, using the info lines in this blip.
	// The first column contains a link to the associated BlipImage.
	// Adds each info line as a column up to the number of 'columns' desired minus 1, allowing for range in last column.
	// Adds empty columns up to the 'columns' minus 1, allowing for range in last column.
	// Adds a column containing the Range.
	// Here is an example of what might be returned if 5 'columns' are requested but this blip only produces 2 info lines.
	//   <tr><td>BlipImage</td><td>Info Line 1</td><td>Info Line 2</td><td></td><td>Range</td></tr>
	// 'scannerLoc' is the location from which range is determined, relative to the solar system.
	// Uses InfoLines.
	virtual QString HtmlTableRow(const DaAtom* system, const AuPoint& scannerLoc, int columns) const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	// NOTE: If any blip that is not a star or planet returns more than 3 info lines, the call to HtmlTableRow in DaCitCrew should
	//       be updated to allow more fields.
	virtual QStringList InfoLines(const DaAtom* system) const = 0;

	// Returns true if this blip might be the target of AI, pretty much any enemy unit or wild animal.
	// AI targets get sorted into the sensor scan data first.
	// That way AI can stop looking for a target when this function returns false.
	virtual bool IsAITarget() const
	{
		return false;
	}

	virtual bool IsAsteroid() const
	{
		return false;
	}

	virtual bool IsBuilding() const
	{
		return false;
	}

	virtual bool IsCity() const
	{
		return false;
	}

	virtual bool IsCivilian() const
	{
		return true;
	}

	virtual bool IsCreature() const
	{
		return false;
	}

	virtual bool IsEmpireCapitol() const
	{
		return false;
	}

	virtual bool IsEnemySpacecraft() const
	{
		return false;
	}

	virtual bool IsEnemySpacecraftLockedOn() const
	{
		return false;
	}

	virtual bool IsGalaxyCapitol() const
	{
		return false;
	}

	virtual bool IsNewSpacecraft() const
	{
		return false;
	}

	virtual bool IsNewVehicle() const
	{
		return false;
	}

	bool IsPirateEmpire() const;

	// TRUE if blip must be in "sensor range".
	// "big" things like stars and planets are not limited to normal sensor range.
	inline bool IsRangeLimited() const
	{
		return !IsStarOrPlanet();
	}

	virtual bool IsRingworldSection() const
	{
		return false;
	}

	// Base class only checks atom id and blip type, sufficient for most objects.
	// Transporter rooms create additional blips for a given city or spacecraft atom.
	virtual bool IsSame(const AuSensorBlip* c) const
	{
		return AtomId() == c->AtomId() && Type() == c->Type();
	}

	virtual bool IsSectorCapitol() const
	{
		return false;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return false;
	}

	// true for all spacecraft
	virtual bool IsSpacecraft() const
	{
		return false;
	}

	virtual bool IsSpacecraftHiding() const
	{
		return false;
	}

	// true for all spacecraft that have shields on
	virtual bool IsSpacecraftShielded() const
	{
		return false;
	}

	virtual bool IsSpaceStation() const
	{
		return false;
	}

	// true for all DaStar objects.
	virtual bool IsStar() const
	{
		return false;
	}

	// true for all DaStar and DaWorld objects.
	virtual bool IsStarOrPlanet() const
	{
		return false;
	}

	virtual bool IsFireControlConsoleBlip() const
	{
		return false;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return false;
	}

	// true for all transporters at cities
	virtual bool IsTransporterRoomCity() const
	{
		return false;
	}

	// true for all transporters on spacecraft, including stations.
	virtual bool IsTransporterRoomSpacecraft() const
	{
		return false;
	}

	// true only for transporters on stations
	virtual bool IsTransporterRoomStation() const
	{
		return false;
	}

	// true for all vehicles, not just ground vehicles.
	virtual bool IsVehicle() const
	{
		return false;
	}

	// true for all DaWorld objects.
	virtual bool IsWorld() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	// Called by AI helmsman (DaCitizen::OperatePilotConsole) to avoid objects.
	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const
	{
		return 0;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		return AuEventLogEntry::Invalid;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Called by AI helmsman (DaCitizen::OperatePilotConsole) to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	virtual float PilotAvoidRadius() const
	{
		return 0;
	}

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning non-zero here has an effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return 0;
	}

	// Returns political stance of scanner's empire toward this blip.
	virtual POLITICAL_STANCE PoliticalStance() const
	{
		return POL_Neutral;
	}

	// Returns distance to this sensor blip.
	virtual double Range(const AuPoint& sysLoc) const
	{
		return m_SysLoc.Distance(sysLoc);
	}

	// Returns squared distance to this sensor blip.
	virtual double Range2(const AuPoint& sysLoc) const
	{
		return m_SysLoc.Distance2(sysLoc);
	}

	virtual quint8 RingworldSectionIndex() const
	{
		return 0;
	}

	// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
	virtual void RxMin(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const = 0;

	virtual void SetPoliticalStance(POLITICAL_STANCE stance)
	{
		Q_UNUSED(stance);
	}

	inline const AuPoint& EnvLocation() const
	{
		return m_SysLoc;
	}

	// Tests this blip to see if it fits into any of the stance categories passed as true parameters.
	// Returns true if this blip is any of the 'true' stance categories.
	// 'isIndifferent' is true for blips with no stance, such as planets and suns.
	virtual bool TestStance(bool isEmpire, bool isFriend, bool isNeutral, bool isEnemy, bool isIndifferent) const
	{
		Q_UNUSED(isEmpire);
		Q_UNUSED(isFriend);
		Q_UNUSED(isNeutral);
		Q_UNUSED(isEnemy);
		return isIndifferent;
	}

	// Only valid for transporter room on a spacecraft.
	virtual qint8 TransporterRoomDeck() const
	{
		return 0;
	}

	// Only valid for transporter room in a city at an airport.
	virtual AuGlobeLLF TransporterRoomDevLL() const
	{
		return AuGlobeLLF();
	}

	// Only valid for transporter room on a spacecraft, sc or bd.
	virtual quint16 TransporterRoomNumber() const
	{
		return 0;
	}

	// Only valid for transporter room in a city at an airport.
	virtual quint16 TransporterRoomWhich() const
	{
		return 0;
	}

	// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
	virtual void TxMin(QDataStream& out) const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const = 0;

	// Used for configuring and logging chat messages.
	virtual QString TypeName() const = 0;

	// Returns target location, relative to solar system, for weapon bay energy on target calculations.
	// This is typically the center of the target.
	// Planets, ringworlds and suns will return a value that is based on hitting their atmosphere|photosphere at a certain depth
	// when using an atmosphere or particle harvester.
	// Rings will return the closest point on the ring.
	// 'system' is the solar system.
	// 'weaponLoc' is the location of the weapon relative to the solar system.
	// 'bayModule' is the weapon bay module in use, to decide where to lock on.
	virtual AuPoint WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& /*weaponLoc*/,
			COMMODITYID /*bayModule*/) const
	{
		return EnvLocation();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSensorBlip& blip)
{
	blip >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSensorBlip& blip)
{
	blip << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuSensorBlip& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuSensorBlipList : public list<AuSensorBlip*>
{
public:
	AuSensorBlipList()
	{
	}
	AuSensorBlipList(const AuSensorBlipList& c);
	virtual ~AuSensorBlipList()
	{
		Clear();
	}

	// Adds a clone of 'blip' to this sensor blip list.
	// Puts wild animals and enemies at the beginning of the list.
	inline void Add(const AuSensorBlip& blip)
	{
		if(blip.IsAITarget())
			push_front(blip.Clone());
		else
			push_back(blip.Clone());
	}

	// Adds 'blip' to this sensor blip list.
	// Puts wild animals and enemies at the beginning of the list.
	inline void AddObject(AuSensorBlip* blip)
	{
		if(blip->IsAITarget())
			push_front(blip);
		else
			push_back(blip);
	}

	inline void Clear()
	{
		while(!empty())
			delete takeFirst();
	}

	// 'targetType' specifies the type of target for 'targetE' display
	//   0 = no target energy display
	//   1 = energy on target
	//   2 = harvest units
	//   3 = heavy arms ammo
	//   4 = large missiles
	//   5 = large rockets
	//   6 = bombs
	//   7 = tractor beam
	void DrawFireControlConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
			const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip,
			bool bCities, bool bLife,
			bool bPlanets, bool bShips, bool bStations,
			bool bStars, bool bVehicles,
			int targetType, float targetEnergy, const AuPoint& consoleLoc,
			bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
			const AuId& scannerId, const AuPoint& scannerLoc) const;

	QPoint DrawSensorConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
			const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip,
			bool bCities, bool bLife,
			bool bPlanets, bool bShips, bool bStations,
			bool bStars, bool bVehicles, const AuPoint& consoleLoc,
			bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
			const AuId& scannerId, const AuPoint& scannerLoc) const;

	// 'transporterMode' is true when we only want ships that have transporters.
	QPoint DrawTransporterConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
			const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip, bool transporterMode,
			bool bLife, bool bCities, bool bPlanets, bool bShips, bool bStations, bool bStars, bool bVehicles,
			const AuPoint& consoleLoc, bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
			const AuId& scannerId, const AuPoint& scannerLoc) const;

	// Locate a blip by using only the blip's atom id and return its const_iterator.
	// Returns end() if not found.
	inline const_iterator Find(const AuId& id) const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->AtomId() == id)
				return bi;
		return end();
	}

	// Locate a blip by using the IsSame test and return its const_iterator.
	// Returns end() if not found.
	inline const_iterator Find(const AuSensorBlip* blip) const
	{
		if(blip)
			for(const_iterator it = begin(); it != end(); it++)
				if((*it)->IsSame(blip))
					return it;
		return end();
	}

	// Checks for enemy sensor blips within 'sensorRange' of 'sysLoc'.
	inline bool HasEnemySensorBlips(const AuPoint& sysLoc, float sensorRange) const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->IsSensorConsoleBlip() &&
					(*bi)->TestStance(false, false, false, true, false) &&
					(*bi)->Range(sysLoc) <= sensorRange)
				return true;	// Found an enemy sensor console blip that is in range.
		return false;
	}

	inline bool HasEnemySpacecraftLockOn() const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->IsEnemySpacecraftLockedOn())
				return true;
		return false;
	}

	inline bool HasForeignSensorBlips(const AuPoint& sysLoc, float sensorRange) const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->IsSensorConsoleBlip() &&
					(*bi)->TestStance(false, true, true, true, false) &&
					(*bi)->Range(sysLoc) <= sensorRange)
				return true;	// Found a non-empire sensor console blip that is in range.
		return false;
	}

	inline bool HasNeutralSensorBlips(const AuPoint& sysLoc, float sensorRange) const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->IsSensorConsoleBlip() &&
					(*bi)->TestStance(false, false, true, true, false) &&
					(*bi)->Range(sysLoc) <= sensorRange)
				return true;	// Found a neutral or enemy sensor console blip that is in range.
		return false;
	}

	inline bool HasSensorBlips(const AuPoint& sysLoc, float sensorRange) const
	{
		for(const_iterator bi = begin(); bi != end(); bi++)
			if((*bi)->IsSensorConsoleBlip() &&
					(*bi)->TestStance(true, true, true, true, false) &&
					(*bi)->Range(sysLoc) <= sensorRange)
				return true;	// Found a sensor console blip that is in range.
		return false;
	}

	inline bool isEmpty() const
	{
		return empty();
	}

	quint64 MemoryUsage() const;

	const AuSensorBlipList& operator=(const AuSensorBlipList& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	// Discards blips that are beyond sensor range.
	// 'scannerLoc' is relative to the solar system.
	inline void RangeDiscard(const AuPoint& scannerLoc, double sensorRange)
	{
		iterator ib = begin();
		while(ib != end())
		{
			if(!(*ib)->IsRangeLimited() || (*ib)->Range(scannerLoc) <= sensorRange)
			{
				// The blip is not range limited or it is within sensor range.
				// Keep it in this list.
				ib++;
			}
			else
			{
				// The blip is beyond sensor range.
				// Remove it from this list.
				delete *ib;
				ib = erase(ib);
			}
		}
	}

	// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
	void RxMin(QDataStream& in);

	AuSensorBlip* takeFirst()
	{
		if(empty())
			return 0;

		AuSensorBlip* b = *begin();
		erase(begin());
		return b;
	}

	// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
	void TxMin(QDataStream& out) const;

	// Transmit bare minimal information for only range limited sensor blips. Client will get the unlimited blips at their end.
	void TxMinRangeLimited(QDataStream& out) const;

	// Transmit only range limited sensor blips. Client will get the unlimited blips at their end.
	void TxRangeLimited(QDataStream& out) const;

	// Transmit only range limited sensor blips that are within 'scannerRange' of 'scannerLoc', relative to the solar system.
	void TxRangeLimited(QDataStream& out, const AuPoint& scannerLoc, double scannerRange) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuSensorBlipList& blipList)
{
	blipList >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSensorBlipList& blipList)
{
	blipList << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipEmpire : public AuSensorBlip
{
protected:
	quint32								m_Empire;				// Empire id of blip.
private:
	quint8								m_Stance;				// Scanner empire's stance toward this blip.

public:
	AuBlipEmpire(quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipEmpire();
	AuBlipEmpire(const AuBlipEmpire& c);

	virtual quint32 EmpireId() const
	{
		return m_Empire;
	}

	virtual QString HtmlTableRow(const DaAtom* system, const AuPoint& scannerLoc, int columns) const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// Almost everything 'enemy' is a potential target. Exceptions include cities and teleporters.
	virtual bool IsAITarget() const
	{
		return Stance() == POL_Enemy;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual POLITICAL_STANCE PoliticalStance() const
	{
		return Stance();
	}

	// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
	virtual void RxMin(QDataStream& in);

	virtual void SetPoliticalStance(POLITICAL_STANCE stance)
	{
		m_Stance = stance;
	}

	inline POLITICAL_STANCE Stance() const
	{
		return (POLITICAL_STANCE)m_Stance;
	}

	// Tests this blip to see if it fits into any of the stance categories passed as true parameters.
	// Returns true if this blip is any of the 'true' stance categories.
	// 'isIndifferent' is true for blips with no stance, such as planets and suns.
	virtual bool TestStance(bool isEmpire, bool isFriend, bool isNeutral, bool isEnemy, bool isIndifferent) const
	{
		Q_UNUSED(isIndifferent);
		switch(Stance())
		{
		case POL_Empire:
		case POL_Vassal:
			return isEmpire;
		case POL_Friend:
			return isFriend;
		case POL_Neutral:
		default:
			return isNeutral;
		case POL_Enemy:
			return isEnemy;
		}
	}

	// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
	virtual void TxMin(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipVehicle : public AuBlipEmpire
{
private:
	quint16								m_CID;					// Commodity id of the vehicle.
	bool								m_IsNew;				// true if the vehicle is nu.
	quint8								m_Quality;				// Quality of the vehicle.

public:
	AuBlipVehicle(COMMODITYID cid, quint8 techLevel, bool isNew,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipVehicle();
	AuBlipVehicle(const AuBlipVehicle& c);

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	virtual bool IsNewVehicle() const
	{
		return m_IsNew;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	// true for all vehicles, not just ground vehicles.
	virtual bool IsVehicle() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline quint8 Quality() const
	{
		return m_Quality;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipAircraft : public AuBlipVehicle
{
private:

public:
	AuBlipAircraft(COMMODITYID cid, quint8 techLevel, bool isNew,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipAircraft();
	AuBlipAircraft(const AuBlipAircraft& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyAircraft;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Aircraft;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Aircraft");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipAircraft& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBoat : public AuBlipVehicle
{
private:

public:
	AuBlipBoat(COMMODITYID cid, quint8 techLevel, bool isNew,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipBoat();
	AuBlipBoat(const AuBlipBoat& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyBoat;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Boat;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Watercraft");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBoat& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipGroundVehicle : public AuBlipVehicle
{
private:

public:
	AuBlipGroundVehicle(COMMODITYID cid, quint8 techLevel, bool isNew,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipGroundVehicle();
	AuBlipGroundVehicle(const AuBlipGroundVehicle& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyVehicle;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return GroundVehicle;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Ground Vehicle");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipGroundVehicle& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipCreature : public AuBlipEmpire
{
private:
	AuSpecies							m_Species;				// Species key of the creature.

public:
	AuBlipCreature(const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipCreature();
	AuBlipCreature(const AuBlipCreature& c);

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCreature() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline const AuSpecies& Species() const
	{
		return m_Species;
	}

	QString SpeciesName() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipAnimal : public AuBlipCreature
{
private:
	bool								m_IsMounted;			// true if the animal is carrying a rider.

public:
	AuBlipAnimal(bool mounted,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipAnimal();
	AuBlipAnimal(const AuBlipAnimal& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// Almost everything 'enemy' is a potential target. Exceptions include cities and teleporters.
	virtual bool IsAITarget() const
	{
		return IsWild() || AuBlipCreature::IsAITarget();
	}

	virtual bool IsCivilian() const
	{
		return !IsMounted();
	}

	inline bool IsMounted() const
	{
		return m_IsMounted;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return !IsMounted();
	}

	inline bool IsWild() const
	{
		return EmpireId() == 0;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
	virtual void RxMin(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
	virtual void TxMin(QDataStream& out) const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Animal;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Animal");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipAnimal& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipAvatar : public AuBlipCreature
{
private:
	QString								m_Name;

public:
	AuBlipAvatar(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipAvatar();
	AuBlipAvatar(const AuBlipAvatar& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Avatar;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Avatar");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipAvatar& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipCitizen : public AuBlipCreature
{
private:
	QString								m_Name;

public:
	AuBlipCitizen(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipCitizen();
	AuBlipCitizen(const AuBlipCitizen& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Citizen;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Citizen");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipCitizen& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipCrew : public AuBlipCitizen
{
private:

public:
	AuBlipCrew(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipCrew();
	AuBlipCrew(const AuBlipCrew& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Crew;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Crew");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipCrew& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipOfficer : public AuBlipCitizen
{
private:

public:
	AuBlipOfficer(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipOfficer();
	AuBlipOfficer(const AuBlipOfficer& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Officer;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Officer");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipOfficer& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipPassenger : public AuBlipCitizen
{
private:

public:
	AuBlipPassenger(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipPassenger();
	AuBlipPassenger(const AuBlipPassenger& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Passenger;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipPassenger& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipTroop : public AuBlipCitizen
{
private:

public:
	AuBlipTroop(const QString& name,
			const AuSpecies& species,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipTroop();
	AuBlipTroop(const AuBlipTroop& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Troop;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Troop");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipTroop& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipTransporterSpacecraft : public AuBlipEmpire
{
private:
	quint8								m_Pads;					// Number of transporter pads in the room.
	quint16								m_RoomNumber;			// Number of room
	QString								m_SpacecraftName;		// Name of the spacecraft.

public:
	AuBlipTransporterSpacecraft(const QString& spacecraftName, quint16 roomNumber, quint8 pads,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipTransporterSpacecraft();
	AuBlipTransporterSpacecraft(const AuBlipTransporterSpacecraft& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsAITarget() const
	{
		return false;
	}

	virtual bool IsCivilian() const
	{
		return false;
	}

	// Base class only checks atom id. That is sufficient for most objects.
	// Transporter rooms can create additional blips for a given atom id.
	virtual bool IsSame(const AuSensorBlip* c) const
	{
		if(!AuBlipEmpire::IsSame(c))
			return false;

		const AuBlipTransporterSpacecraft* pc = dynamic_cast<const AuBlipTransporterSpacecraft*>(c);
		if(!pc)
			return false;

		if(RoomNumber() != pc->RoomNumber())
			return false;
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	// true for all transporters on spacecraft, including stations.
	virtual bool IsTransporterRoomSpacecraft() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline quint8 Pads() const
	{
		return m_Pads;
	}

	inline quint16 RoomNumber() const
	{
		return m_RoomNumber;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline const QString& SpacecraftName() const
	{
		return m_SpacecraftName;
	}

	virtual quint16 TransporterRoomNumber() const
	{
		return RoomNumber();
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return TransporterSpacecraft;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Spacecraft Transporter Room");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipTransporterSpacecraft& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipTransporterStation : public AuBlipTransporterSpacecraft
{
private:

public:
	AuBlipTransporterStation(const QString& spacecraftName, quint16 roomNumber, quint8 pads,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipTransporterStation();
	AuBlipTransporterStation(const AuBlipTransporterStation& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// true only for transporters on stations
	virtual bool IsTransporterRoomStation() const
	{
		return true;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return TransporterStation;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Station Transporter Room");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipTransporterStation& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipTransporterBd : public AuBlipEmpire
{
private:
	QString								m_BdName;				// Name of the building.
	quint8								m_IsMilitary;			// 1 if military, 0 if civilian
	quint8								m_Pads;					// Number of transporter pads in the room.
	quint16								m_RoomNumber;			// Number of room.

public:
	AuBlipTransporterBd(const QString& bdName, bool isMilitary, quint16 roomNumber, quint8 pads, quint32 empire,
			POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipTransporterBd();
	AuBlipTransporterBd(const AuBlipTransporterBd& c);

	inline const QString& BdName() const
	{
		return m_BdName;
	}

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsAITarget() const
	{
		return false;
	}

	virtual bool IsCivilian() const
	{
		return !m_IsMilitary;
	}

	// Base class only checks atom id. That is sufficient for most objects.
	// Transporter rooms can create additional blips for a given atom id.
	virtual bool IsSame(const AuSensorBlip* c) const
	{
		if(!AuBlipEmpire::IsSame(c))
			return false;

		const AuBlipTransporterBd* pc = dynamic_cast<const AuBlipTransporterBd*>(c);
		if(!pc)
			return false;

		if(RoomNumber() != pc->RoomNumber())
			return false;
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	// true for all transporters on spacecraft, including stations.
	virtual bool IsTransporterRoomSpacecraft() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline quint8 Pads() const
	{
		return m_Pads;
	}

	inline quint16 RoomNumber() const
	{
		return m_RoomNumber;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return TransporterBd;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Building Transporter Room");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipTransporterBd& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipSpacecraft : public AuBlipEmpire
{
private:
	quint32								m_CurHits;				// Current hit points of ship.
	QString								m_DesignName;			// Name of the design.
	bool								m_IsLockedOn;			// True if this ship's sensors are locked-on to the scanning atom.
	bool								m_IsNew;
	bool								m_IsPrivatelyOwned;
	quint32								m_MaxHits;				// Maximum hit points of ship when at full health.
	QString								m_Name;					// Name of the spacecraft.
	float								m_RoughRadius;
	quint8								m_Quality;
	AuPoint								m_Vector;				// Movement vector.
	quint32								m_Volume;				// Volume of hull in cubic meters.

public:
	AuBlipSpacecraft(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipSpacecraft();
	AuBlipSpacecraft(const AuBlipSpacecraft& c);

	inline quint32 CurHits() const
	{
		return m_CurHits;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	virtual bool IsEnemySpacecraft() const
	{
		return Stance() == POL_Enemy;
	}

	virtual bool IsEnemySpacecraftLockedOn() const
	{
		return IsLockedOn() && Stance() == POL_Enemy;
	}

	virtual bool IsFireControlConsoleBlip() const
	{
		return true;
	}

	inline bool IsLockedOn() const
	{
		return m_IsLockedOn;
	}

	inline bool IsNew() const
	{
		return m_IsNew;
	}

	virtual bool IsNewSpacecraft() const
	{
		return IsNew();
	}

	inline bool IsPrivatelyOwned() const
	{
		return m_IsPrivatelyOwned;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	// true for all spacecraft
	virtual bool IsSpacecraft() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		if(IsNew() && (Stance() == POL_Enemy || Stance() == POL_Neutral))
			return false;
		return true;
	}

	inline quint32 MaxHits() const
	{
		return m_MaxHits;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline float RoughRadius() const
	{
		return m_RoughRadius;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline const AuPoint& Vector() const
	{
		return m_Vector;
	}

	inline quint32 Volume() const
	{
		return m_Volume;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipSpacecraftShielded : public AuBlipSpacecraft
{
private:
	float								m_AvoidRadius;

public:
	AuBlipSpacecraftShielded(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits,
			const QString& designName, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipSpacecraftShielded();
	AuBlipSpacecraftShielded(const AuBlipSpacecraftShielded& c);

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// true for all spacecraft that have shields on
	virtual bool IsSpacecraftShielded() const
	{
		return true;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStation : public AuBlipSpacecraft
{
private:

public:
	AuBlipStation(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipStation();
	AuBlipStation(const AuBlipStation& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	virtual bool IsSpaceStation() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyStation;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Station;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Station");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStation& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStationShielded : public AuBlipSpacecraftShielded
{
private:

public:
	AuBlipStationShielded(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits,
			const QString& designName, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipStationShielded();
	AuBlipStationShielded(const AuBlipStationShielded& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	virtual bool IsSpaceStation() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyStationS;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return StationShielded;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Station");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStationShielded& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipSpaceship : public AuBlipSpacecraft
{
private:

public:
	AuBlipSpaceship(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipSpaceship();
	AuBlipSpaceship(const AuBlipSpaceship& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemySpaceship;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Spaceship;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Spaceship");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipSpaceship& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipSpaceshipShielded : public AuBlipSpacecraftShielded
{
private:

public:
	AuBlipSpaceshipShielded(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits,
			const QString& designName, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipSpaceshipShielded();
	AuBlipSpaceshipShielded(const AuBlipSpaceshipShielded& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemySpaceshipS;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return SpaceshipShielded;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Spaceship");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipSpaceshipShielded& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStarship : public AuBlipSpacecraft
{
private:

public:
	AuBlipStarship(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipStarship();
	AuBlipStarship(const AuBlipStarship& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyStarship;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Starship;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Starship");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStarship& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStarshipShielded : public AuBlipSpacecraftShielded
{
private:

public:
	AuBlipStarshipShielded(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits,
			const QString& designName, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipStarshipShielded();
	AuBlipStarshipShielded(const AuBlipStarshipShielded& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;

		return AuEventLogEntry::EnemyStarshipS;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return StarshipShielded;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Starship");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStarshipShielded& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipSpacecraftHiding : public AuBlipSpacecraft
{
private:

public:
	AuBlipSpacecraftHiding(const QString& name, quint8 techLevel, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
			float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipSpacecraftHiding();
	AuBlipSpacecraftHiding(const AuBlipSpacecraftHiding& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	virtual bool IsFireControlConsoleBlip() const
	{
		return false;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return false;
	}

	virtual bool IsSpacecraftHiding() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return false;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return SpacecraftHiding;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Spacecraft");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipSpacecraftHiding& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStar : public AuSensorBlip
{
private:
	bool								m_InSensorRange;
	float								m_Radius;				// Radius of the photosphere.
	quint8								m_SpectralClass;
	quint8								m_StarSize;
	quint8								m_StarType;

public:
	AuBlipStar(DaStar::TYPE type, DaStar::SIZE size, quint8 spectralClass, float radius, bool inSensorRange, const AuId& id,
			const AuPoint& sysLoc);
	AuBlipStar();
	AuBlipStar(const AuBlipStar& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_ParticleCollectorBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	inline bool InSensorRange() const
	{
		return m_InSensorRange;
	}

	virtual bool IsFireControlConsoleBlip() const
	{
		return true;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	virtual bool IsStar() const
	{
		return true;
	}

	virtual bool IsStarOrPlanet() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	inline float Radius() const
	{
		return m_Radius;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline quint8 SpectralClass() const
	{
		return m_SpectralClass;
	}

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning 1 here has a desired effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return 1;
	}

	inline DaStar::SIZE StarSize() const
	{
		return (DaStar::SIZE)m_StarSize;
	}

	inline DaStar::TYPE StarType() const
	{
		return (DaStar::TYPE)m_StarType;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return StarPrimary;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Star");
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStar& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipStarCompanion : public AuBlipStar
{
private:

public:
	AuBlipStarCompanion(DaStar::TYPE type, DaStar::SIZE size, quint8 spectralClass, float radius, bool inSensorRange,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipStarCompanion();
	AuBlipStarCompanion(const AuBlipStarCompanion& c);

	virtual AuSensorBlip* Clone() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return StarCompanion;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Star, Companion");
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipStarCompanion& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBlackHole : public AuBlipStar
{
private:

public:
	AuBlipBlackHole(DaStar::TYPE type, float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipBlackHole();
	AuBlipBlackHole(const AuBlipBlackHole& c);

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BlackHole;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Black Hole");
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBlackHole& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipNeutronStar : public AuBlipStar
{
private:

public:
	AuBlipNeutronStar(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipNeutronStar();
	AuBlipNeutronStar(const AuBlipNeutronStar& c);

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return NeutronStarPrimary;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Neutron Star");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipNeutronStar& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipNeutronStarCompanion : public AuBlipNeutronStar
{
private:

public:
	AuBlipNeutronStarCompanion(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipNeutronStarCompanion();
	AuBlipNeutronStarCompanion(const AuBlipNeutronStarCompanion& c);

	virtual AuSensorBlip* Clone() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return NeutronStarCompanion;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Neutron Star, Companion");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipNeutronStarCompanion& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipWorld : public AuSensorBlip
{
private:
	bool								m_InSensorRange;

public:
	AuBlipWorld(bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipWorld();
	AuBlipWorld(const AuBlipWorld& c);

	virtual QString BodyType() const
	{
		return TypeName();
	}

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsFireControlConsoleBlip() const
	{
		return true;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	inline bool InSensorRange() const
	{
		return m_InSensorRange;
	}

	virtual bool IsStarOrPlanet() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	virtual bool IsWorld() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

protected:
	void ReportZoneBioResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, map<QString, QString>& zonelist) const;

	void ReportZoneFetchResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, map<QString, QString>& zonelist) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipWorld& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipGasGiant : public AuBlipWorld
{
private:
	quint8								m_CoreRadius1000km;		// Radius of core in 1000's of kilometers.
	quint8								m_Radius1000km;			// Radius in 1000's of kilometers. Will be in the range 20-40.

public:
	AuBlipGasGiant(quint8 radius1000km, quint8 coreRadius1000km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipGasGiant();
	AuBlipGasGiant(const AuBlipGasGiant& c);

	float AtmosphereRadius() const;

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_AtmosphereCondenserBayModule ||
			bayModule == CID_ParticleCollectorBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	inline quint8 CoreRadius1000km() const
	{
		return m_CoreRadius1000km;
	}

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual quint64 MemoryUsage() const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	// Range within which a pilot maintains a slow safe speed limit.
	virtual float PilotSpeedLimitRange() const
	{
		// Planet spacing values taken from DaWorld::CheckChildExit.
		return FeetPerOrbit()/2;
	}

	inline quint8 Radius1000km() const
	{
		return m_Radius1000km;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return GasGiant;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_GasGiant);
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipGasGiant& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipGlobe : public AuBlipWorld
{
private:
	quint8								m_Radius100km;			// Radius of the globe at sea level, in 1000's of km.

public:
	AuBlipGlobe(quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipGlobe();
	AuBlipGlobe(const AuBlipGlobe& c);

	float AtmosphereRadius() const;

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	virtual quint64 MemoryUsage() const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	float Radius() const;

	inline quint8 Radius100km() const
	{
		return m_Radius100km;
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipTitan : public AuBlipGlobe
{
private:
	quint8								m_HydLiquid;			// HYDROGRAPHIC_LIQUID

public:
	AuBlipTitan(HYDROGRAPHIC_LIQUID hydLiquid, quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipTitan();
	AuBlipTitan(const AuBlipTitan& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_AtmosphereCondenserBayModule ||
			bayModule == CID_ProspectorBayModule ||
			(bayModule == CID_SiphonBayModule && HydrographicLiquid() != HYDRO_None) ||
			bayModule == CID_WildcatterBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	// Could be water, lava, or supercooled liquid
	inline HYDROGRAPHIC_LIQUID HydrographicLiquid() const
	{
		return (HYDROGRAPHIC_LIQUID)m_HydLiquid;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Range within which a pilot maintains a slow safe speed limit.
	virtual float PilotSpeedLimitRange() const
	{
		// Planet spacing values taken from DaWorld::CheckChildExit.
		return FeetPerMoonOrbit()*3/2;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Titan;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_Titan);
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipTitan& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipMoon : public AuBlipGlobe
{
private:

public:
	AuBlipMoon(quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipMoon();
	AuBlipMoon(const AuBlipMoon& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_ProspectorBayModule ||
			bayModule == CID_WildcatterBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Range within which a pilot maintains a slow safe speed limit.
	virtual float PilotSpeedLimitRange() const
	{
		// Planet spacing values taken from DaWorld::CheckChildExit.
		return FeetPerMoonOrbit()/2;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Moon;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_Moon);
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipMoon& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipPlanet : public AuBlipGlobe
{
private:
	quint8								m_HydLiquid;			// HYDROGRAPHIC_LIQUID

public:
	AuBlipPlanet(HYDROGRAPHIC_LIQUID hydLiquid, quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipPlanet();
	AuBlipPlanet(const AuBlipPlanet& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_AtmosphereCondenserBayModule ||
			bayModule == CID_ProspectorBayModule ||
			(bayModule == CID_SiphonBayModule && HydrographicLiquid() != HYDRO_None) ||
			bayModule == CID_WildcatterBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	// Could be water, lava, or supercooled liquid
	inline HYDROGRAPHIC_LIQUID HydrographicLiquid() const
	{
		return (HYDROGRAPHIC_LIQUID)m_HydLiquid;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Range within which a pilot maintains a slow safe speed limit.
	virtual float PilotSpeedLimitRange() const
	{
		// Planet spacing values taken from DaWorld::CheckChildExit.
		return FeetPerOrbit()/2;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Planet;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_Planet);
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipPlanet& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipRing : public AuBlipWorld
{
private:
	float								m_Radius;				// Radius of ring

public:
	AuBlipRing(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipRing();
	AuBlipRing(const AuBlipRing& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_ProspectorBayModule ||
			bayModule == CID_ParticleCollectorBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	// Calculate 2D distance from this blip to a pixel, when projected onto a display screen.
	// 'mScreen' is the projection to the screen.
	// 'pxHit' is the pixel location hit on the screen.
	virtual double DisplayDistance(const AuMatrix4x4& mScreen, const QPoint& pxHit, const DaAtom* system,
			float feetPerPixel) const;

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline float Radius() const
	{
		return m_Radius;
	}

	// Returns distance to this sensor blip.
	virtual double Range(const AuPoint& sysLoc) const;

	// Returns squared distance to this sensor blip.
	virtual double Range2(const AuPoint& sysLoc) const;

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	virtual bool IsTransporterConsoleBlip() const
	{
		return false;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Ring;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_Ring);
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipRing& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipRingSection : public AuBlipWorld
{
private:
	quint8								m_SectionIndex;			// Arc section index.

public:
	AuBlipRingSection(quint8 sectionIndex, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipRingSection();
	AuBlipRingSection(const AuBlipRingSection& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_AtmosphereCondenserBayModule ||
			bayModule == CID_ProspectorBayModule ||
			bayModule == CID_SiphonBayModule ||
			bayModule == CID_WildcatterBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	// Calculate 2D distance from this blip to a pixel, when projected onto a display screen.
	// 'mScreen' is the projection to the screen.
	// 'pxHit' is the pixel location hit on the screen.
	virtual double DisplayDistance(const AuMatrix4x4& mScreen, const QPoint& pxHit, const DaAtom* system,
			float feetPerPixel) const;

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	virtual bool IsRingworldSection() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning non-zero here has an effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		// Planet spacing values taken from DaWorld::CheckChildExit.
		return FeetPerOrbit()/2;
	}

	virtual quint8 RingworldSectionIndex() const
	{
		return SectionIndex();
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline quint8 SectionIndex() const
	{
		return m_SectionIndex;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return RingSection;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Ringworld Arc Section");
	}

	virtual AuPoint WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const;
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipRingSection& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipMissile : public AuBlipEmpire
{
private:
	quint16								m_CID;					// Commodity id of the missile.
	quint8								m_Count;				// Number of missiles in the bunch.
	quint8								m_SeekingScanner;		// 0=not seeking, 1=seeking target, 2=locked on to scanner atom,
																// 3=locked on to a different atom
	quint8								m_Quality;				// Quality of the missile.

public:
	AuBlipMissile(COMMODITYID cid, quint8 techLevel, bool seekingScanner, quint8 count,
			quint32 empire, POLITICAL_STANCE stance,
			const AuId& id, const AuPoint& sysLoc);
	AuBlipMissile();
	AuBlipMissile(const AuBlipMissile& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	inline quint8 Count() const
	{
		return m_Count;
	}

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	inline bool IsSeekingScanner() const
	{
		return m_SeekingScanner;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Missile;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Missile");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBeacon : public AuSensorBlip
{
private:
	QString								m_Message;

public:
	AuBlipBeacon(const QString& message, const AuId& id, const AuPoint& sysLoc);
	AuBlipBeacon();
	AuBlipBeacon(const AuBlipBeacon& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Message() const
	{
		return m_Message;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		return AuEventLogEntry::Beacon;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Beacon;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Beacon");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipAnomaly : public AuSensorBlip
{
private:

public:
	AuBlipAnomaly(const AuId& id, const AuPoint& sysLoc);
	AuBlipAnomaly();
	AuBlipAnomaly(const AuBlipAnomaly& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		return AuEventLogEntry::Anomaly;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Anomaly;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Anomaly");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBd : public AuBlipEmpire
{
private:
	quint32								m_CurHits;				// Current hit points of ship.
	quint8								m_Industry;				// INDUSTRYID of the building.
	bool								m_IsLockedOn;			// True if this ship's sensors are locked-on to the scanning atom.
	quint32								m_MaxHits;				// Maximum hit points of ship when at full health.
	QString								m_Name;					// Assigned name of the building, if any.
	float								m_RoughRadius;
	quint8								m_Quality;
	quint32								m_Volume;				// Volume of hull in cubic meters.

public:
	AuBlipBd(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBd();
	AuBlipBd(const AuBlipBd& c);

	inline quint32 CurHits() const
	{
		return m_CurHits;
	}

	inline INDUSTRYID IndustryId() const
	{
		return (INDUSTRYID)m_Industry;
	}

	virtual bool IsAITarget() const
	{
		return true;
	}

	virtual bool IsBuilding() const
	{
		return true;
	}

	virtual bool IsFireControlConsoleBlip() const
	{
		return true;
	}

	inline bool IsLockedOn() const
	{
		return m_IsLockedOn;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	virtual bool IsTransporterConsoleBlip() const
	{
		return true;
	}

	inline quint32 MaxHits() const
	{
		return m_MaxHits;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual float ObstacleRadius() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline float RoughRadius() const
	{
		return m_RoughRadius;
	}

	virtual void RxMin(QDataStream& in);

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	virtual void TxMin(QDataStream& out) const;

	inline quint32 Volume() const
	{
		return m_Volume;
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBd& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdBase : public AuBlipBd
{
public:
	AuBlipBdBase(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdBase();
	AuBlipBdBase(const AuBlipBdBase& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return false;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdBase;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Military Building");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdBase& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdBaseHQ : public AuBlipBd
{
public:
	AuBlipBdBaseHQ(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdBaseHQ();
	AuBlipBdBaseHQ(const AuBlipBdBaseHQ& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCity() const
	{
		return true;
	}

	virtual bool IsCivilian() const
	{
		return false;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;
		return AuEventLogEntry::EnemyBase;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdBaseHQ;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Military Headquarters");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdBase& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdCapitolCity : public AuBlipBd
{
public:
	AuBlipBdCapitolCity(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdCapitolCity();
	AuBlipBdCapitolCity(const AuBlipBdCapitolCity& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCity() const
	{
		return true;
	}

	virtual bool IsCivilian() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;
		return AuEventLogEntry::EnemyCity;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdCapitolCity;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("City Capitol");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdCapitolCity& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdCapitolEmpire : public AuBlipBd
{
public:
	AuBlipBdCapitolEmpire(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdCapitolEmpire();
	AuBlipBdCapitolEmpire(const AuBlipBdCapitolEmpire& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCity() const
	{
		return true;
	}

	virtual bool IsCivilian() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;
		return AuEventLogEntry::EnemyCity;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdCapitolEmpire;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Empire Capitol");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdCapitolEmpire& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdCapitolGalaxy : public AuBlipBd
{
public:
	AuBlipBdCapitolGalaxy(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdCapitolGalaxy();
	AuBlipBdCapitolGalaxy(const AuBlipBdCapitolGalaxy& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCity() const
	{
		return true;
	}

	virtual bool IsCivilian() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;
		return AuEventLogEntry::EnemyCity;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdCapitolGalaxy;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Galaxy Capitol");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdCapitolGalaxy& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdCapitolSector : public AuBlipBd
{
public:
	AuBlipBdCapitolSector(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdCapitolSector();
	AuBlipBdCapitolSector(const AuBlipBdCapitolSector& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCity() const
	{
		return true;
	}

	virtual bool IsCivilian() const
	{
		return true;
	}

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		if(Stance() != POL_Enemy)
			return AuEventLogEntry::Invalid;
		return AuEventLogEntry::EnemyCity;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdCapitolSector;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Sector Capitol");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdCapitolSector& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipBdCity : public AuBlipBd
{
public:
	AuBlipBdCity(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
			quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc);
	AuBlipBdCity();
	AuBlipBdCity(const AuBlipBdCity& c);

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsCivilian() const
	{
		return true;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return BdCity;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Building");
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipBdCity& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipAsteroid : public AuSensorBlip
{
private:
	quint16								m_Hits;
	quint8								m_Size;

public:
	AuBlipAsteroid(quint8 size, quint16 hits, const AuId& id, const AuPoint& sysLoc);
	AuBlipAsteroid();
	AuBlipAsteroid(const AuBlipAsteroid& c);

	inline quint8 AsteroidSize() const
	{
		return m_Size;
	}

	virtual const char* BlipImage() const;

	virtual AuSensorBlip* Clone() const;

	inline quint16 Hits() const
	{
		return m_Hits;
	}

	// If 'system' is 0, returns only the short one-line name for planet blips.
	virtual QStringList InfoLines(const DaAtom* system) const;

	virtual bool IsAITarget() const
	{
		return true;
	}

	virtual bool IsAsteroid() const
	{
		return true;
	}

	virtual bool IsFireControlConsoleBlip() const
	{
		return true;
	}

	virtual bool IsSensorConsoleBlip() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual float ObstacleRadius() const;

	// Returns the type of event log entry generated when an officer logs sensor contacts.
	virtual AuEventLogEntry::TYPE OfficerEventLogType() const
	{
		return AuEventLogEntry::AsteroidDetected;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual float PilotAvoidRadius() const;

	virtual POLITICAL_STANCE PoliticalStance() const
	{
		return POL_Enemy;
	}

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	virtual bool TestStance(bool isEmpire, bool isFriend, bool isNeutral, bool isEnemy, bool isIndifferent) const
	{
		Q_UNUSED(isEmpire);
		Q_UNUSED(isFriend);
		Q_UNUSED(isNeutral);
		Q_UNUSED(isIndifferent);
		// Asteroids are enemy of everyone.
		return isEnemy;
	}

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Asteroid;
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Asteroid");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBlipPlanetoid : public AuBlipGlobe
{
public:
	AuBlipPlanetoid(quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc);
	AuBlipPlanetoid();
	AuBlipPlanetoid(const AuBlipPlanetoid& c);

	virtual bool CanBeHarvested(COMMODITYID bayModule) const
	{
		return bayModule == CID_ProspectorBayModule ||
			bayModule == CID_ParticleCollectorBayModule;
	}

	virtual AuSensorBlip* Clone() const;

	virtual void DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Range within which a pilot maintains a slow safe speed limit.
	virtual float PilotSpeedLimitRange() const;

	// Name used next to selection reticle.
	virtual QString ScreenName() const;

	// Identifies each type of sensor blip in a data stream.
	virtual TYPE Type() const
	{
		return Planetoid;
	}

	virtual QString TypeName() const
	{
		return BodyTypeName(BODY_Planetoid);
	}
};

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuBlipPlanetoid& blip);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuSensorBlip_H

