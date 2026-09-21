//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auqueryenvironment.h,v 1.54 2022/02/19 17:32:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuQueryEnvironment_H
#define AuQueryEnvironment_H

#include "auatom.h"
#include "auglobell.h"
#include "auid.h"
#include "aupoint.h"
#include "dbindustry.h"
#include <QColor>

enum ENV_BITS
{
	EB_None				= 0x00000000,
	EB_Atmosphere		= 0x00000001,	// planet has atmosphere density > 0
	EB_Breathable		= 0x00000002,	// Surrounding environment is breathable. Considers building/spacecraft life support.
	EB_AtBd				= 0x00000004,	// at a Bd building site
	EB_InBd				= 0x00000008,	// inside a Bd building
	EB_InAtmosphere		= 0x00000010,	// within atm range, even if no atm dens
	EB_Spacecraft		= 0x00000020,	// inside a spacecraft
	EB_CGUnderwater		= 0x00000040,	// center-of-gravity is under water.
	EB_InVehicle		= 0x00000080,	// inside an enclosed vehicle
	EB_GravityWell		= 0x00000100,	// atom is in gravity well
	EB_InWater			= 0x00000200,	// atom is in water and wet, not true inside spacecraft, vehicle, or undersea city.
	EB_UNUSED1			= 0x00000400,
	EB_UNUSED2			= 0x00000800,
	EB_OnWater			= 0x00001000,	// On water surface
	EB_Design			= 0x00002000,	// inside a design studio
	EB_OnVehicle		= 0x00004000,	// on an open vehicle
	EB_EyeUnderwater	= 0x00008000,	// eye is under water
	EB_Gravity			= 0x00010000,	// atom is affected by gravity
	EB_AtGeo			= 0x00020000,	// at a geo construction site or completed geo road site
	EB_UNUSED3			= 0x00040000,
	EB_UNUSED4			= 0x00080000,
	EB_UNUSED5			= 0x00100000,
	EB_Stairs			= 0x00200000,	// Stairs below this atom.
	EB_Pathable			= 0x00400000,	// In pathable space
	EB_PhotonSphere		= 0x00800000,	// Inside photon sphere of a black hole or neutron sphere of neutron star, at any level.
	EB_EventHorizon		= 0x01000000,	// Inside Schwarzschild radius of a black hole.
	EB_BlackHoleGravity	= 0x02000000,	// In black hole/neutron star system and not sub to a planet or in/on a vessel or vehicle.
	EB_Arena			= 0x04000000,	// Inside an arena.
	EB_MouthUnderwater	= 0x08000000,	// Mouth is under water
	EB_DesignPreview	= 0x10000000,	// Inside design environment that is in preview mode.
	EB_UNUSED6			= 0x20000000,
	EB_Road				= 0x40000000,	// On a road or road slab, could be under construction.
	EB_InLadderWell		= 0x80000000	// In position to grab a ladder. Might not be in the proper pose.
};

class AUATOM_DLL AuQueryEnvironment
{
friend class DaAtom;
friend class DaDesigner;
private:
	ATMTYPE								m_AtmType;				// Atmosphere type, only valid when IsInAtmosphere.
	AuId								m_BdCityId;				// Id of DaBd's city capitol DaBd if InBd or AtBd.
	AuId								m_BdId;					// Id of DaBd if InBd or AtBd.
	quint16								m_CapRoomId;			// AuScPart id of room if inside a capsule: Bd or Sc
	QString								m_CapRoomName;
	ROOMTYPE							m_CapRoomType;
	quint32								m_CityEmpire;			// Valid when in a city industry or bd city jurisdiction.
	QColor								m_colAtmosphere;		// AtmosphereColor of parent planet.
	AuId								m_EnclosureId;			// Id of enclosure that contains this atom.
	quint32								m_Environment;			// bitmask of ENV_BITS
	double								m_FallDist;				// Distance to fall to hard floor or ground. + is down, - is up.
	float								m_fAltitudeAGL;			// Altitude when inside an atm = dist to ground.
	float								m_fAltitudeMSL;			// Altitude above/below sea level when inside an atm.
	float								m_fCeilingDist;			// Distance from cg to ceiling.
	double								m_fCGAltitude;			// Distance from cg-to-cg above planet/star.
	float								m_fAirDensity;			// Atmosphere density of immediate environment 0..1.
	float								m_fAtmDensity;			// Atmosphere density 0..1 when inside atmo. Adjusted for altitude.
	float								m_fSeaDepth;			// Depth below sea level. 0 when above sea level.
	AuGlobeLLF							m_GlobeLL;				// Longitude/latitude when parent is a globe.
	AuPoint								m_GravityNormal;		// Points up with respect to gravity source.
	INDUSTRYID							m_InIndustry;			// Industry if standing in a DaBd site.
	AuGlobeLLF							m_InIndustryLL;			// Industry LL if standing in a DaBd site.
	ON_GROUND							m_OnGround;
	ORBITZONE							m_OrbitZone;			// orbit zone when near a planet, else OZONE_DeepSpace.
	GALAXY								m_Galaxy;				// Current galaxy (GALAXY enum).
	AuPoint								m_terNormal;			// Normal to surface if on ground.
	float								m_WaterDepth;			// Depth of water if EB_InWater is true. Distance from ground to
																// water surface.
	quint8								m_WorldAtomType;		// DaAtom::ATOM_TYPE of my parent world, at any depth.

public:
	inline AuQueryEnvironment()
	{
		Init();
	}

	inline AuQueryEnvironment(const AuQueryEnvironment& c)
	{
		m_AtmType = c.m_AtmType;
		m_BdCityId = c.m_BdCityId;
		m_BdId = c.m_BdId;
		m_CapRoomId = c.m_CapRoomId;
		m_CapRoomName = c.m_CapRoomName;
		m_CapRoomType = c.m_CapRoomType;
		m_CityEmpire = c.m_CityEmpire;
		m_colAtmosphere = c.m_colAtmosphere;
		m_EnclosureId = c.m_EnclosureId;
		m_Environment = c.m_Environment;
		m_FallDist = c.m_FallDist;
		m_fAltitudeAGL = c.m_fAltitudeAGL;
		m_fAltitudeMSL = c.m_fAltitudeMSL;
		m_fCeilingDist = c.m_fCeilingDist;
		m_fCGAltitude = c.m_fCGAltitude;
		m_fAirDensity = c.m_fAirDensity;
		m_fAtmDensity = c.m_fAtmDensity;
		m_fSeaDepth = c.m_fSeaDepth;
		m_GlobeLL = c.m_GlobeLL;
		m_GravityNormal = c.m_GravityNormal;
		m_InIndustry = c.m_InIndustry;
		m_InIndustryLL = c.m_InIndustryLL;
		m_OnGround = c.m_OnGround;
		m_OrbitZone = c.m_OrbitZone;
		m_Galaxy = c.m_Galaxy;
		m_terNormal = c.m_terNormal;
		m_WaterDepth = c.m_WaterDepth;
	}

	inline const AuQueryEnvironment& operator=(const AuQueryEnvironment& c)
	{
		m_AtmType = c.m_AtmType;
		m_BdCityId = c.m_BdCityId;
		m_BdId = c.m_BdId;
		m_CapRoomId = c.m_CapRoomId;
		m_CapRoomName = c.m_CapRoomName;
		m_CapRoomType = c.m_CapRoomType;
		m_CityEmpire = c.m_CityEmpire;
		m_colAtmosphere = c.m_colAtmosphere;
		m_EnclosureId = c.m_EnclosureId;
		m_Environment = c.m_Environment;
		m_FallDist = c.m_FallDist;
		m_fAltitudeAGL = c.m_fAltitudeAGL;
		m_fAltitudeMSL = c.m_fAltitudeMSL;
		m_fCeilingDist = c.m_fCeilingDist;
		m_fCGAltitude = c.m_fCGAltitude;
		m_fAirDensity = c.m_fAirDensity;
		m_fAtmDensity = c.m_fAtmDensity;
		m_fSeaDepth = c.m_fSeaDepth;
		m_GlobeLL = c.m_GlobeLL;
		m_GravityNormal = c.m_GravityNormal;
		m_InIndustry = c.m_InIndustry;
		m_InIndustryLL = c.m_InIndustryLL;
		m_OnGround = c.m_OnGround;
		m_OrbitZone = c.m_OrbitZone;
		m_Galaxy = c.m_Galaxy;
		m_terNormal = c.m_terNormal;
		m_WaterDepth = c.m_WaterDepth;
		return *this;
	}

	inline void Init()
	{
		m_AtmType = ATMTYPE_Standard;
		m_BdCityId = AuId(0);
		m_BdId = AuId(0);
		m_CapRoomId = 0;
		m_CapRoomName = "";
		m_CapRoomType = ROOM_None;
		m_CityEmpire = 0;
		m_colAtmosphere = QColor(0,0,0);
		m_EnclosureId = AuId(0);
		m_Environment = EB_None;
		m_FallDist = 0;
		m_fAltitudeAGL = 0;
		m_fAltitudeMSL = 0;
		m_fCeilingDist = 100000;
		m_fCGAltitude = 0;
		m_fAirDensity = 0;
		m_fAtmDensity = 0;
		m_fSeaDepth = 0;
		m_GlobeLL = AuGlobeLLF(0,0);
		m_GravityNormal = AuPoint(0,0,1);
		m_InIndustry = IID_None;
		m_InIndustryLL = AuGlobeLLF(0,0);
		m_OnGround = OG_None;
		m_OrbitZone = OZONE_DeepSpace;
		m_Galaxy = ShoresOfHazeron;
		m_terNormal = AuPoint(0,0,1);
		m_WaterDepth = 0;
	}

public:
	inline float AltitudeMSL() const
	{
		return m_fAltitudeMSL;
	}

	inline ATMTYPE AtmosphereType() const
	{
		return m_AtmType;
	}

	inline quint16 BdRoomId() const
	{
		if(!IsInBd())
			return 0;
		return CapsuleRoomId();
	}

	inline QString BdRoomName() const
	{
		if(!IsInBd())
			return QString();
		return m_CapRoomName;
	}

	inline quint16 CapsuleRoomId() const
	{
		return m_CapRoomId;
	}

	inline const AuId& EnclosureId() const
	{
		return m_EnclosureId;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline const AuPoint& GravityNormal() const
	{
		return m_GravityNormal;
	}

	inline bool InDesignEnvironment() const
	{
		return m_Environment & EB_Design;
	}

	inline bool InDesignPreview() const
	{
		return m_Environment & EB_DesignPreview;
	}

	inline bool InLadderWell() const
	{
		return m_Environment & EB_InLadderWell;
	}

	inline bool IsAboardVehicle() const
	{
		return m_Environment & (EB_InVehicle|EB_OnVehicle);
	}

	inline bool IsAirBreathable() const
	{
		return m_Environment & EB_Breathable;
	}

	inline bool IsAtBd() const
	{
		return m_Environment & EB_AtBd;
	}

	inline bool IsAtGeo() const
	{
		return m_Environment & EB_AtGeo;
	}

	inline bool IsCGSubmerged() const
	{
		// This is an AND query. All of the bits must be set.
		const unsigned int crit = EB_CGUnderwater|EB_InWater;
		return (m_Environment & crit) == crit;
	}

	inline bool IsEyeUnderWater() const
	{
		return m_Environment & EB_EyeUnderwater;
	}

	inline bool IsInBd() const
	{
		return m_Environment & EB_InBd;
	}

	inline bool IsInCapsule() const
	{
		return IsInSpacecraft() || IsInBd();
	}

	inline bool IsInGravity() const
	{
		return m_Environment & EB_Gravity;
	}

	inline bool IsInGravityWell() const
	{
		return m_Environment & EB_GravityWell;
	}

	inline bool IsInsideBd() const
	{
		if(!IsInBd())
			return false;
		return m_CapRoomId;
	}

	inline bool IsInsideCapsule() const
	{
		return IsInsideSpacecraft() || IsInsideBd();
	}

	inline bool IsInsideEnclosure() const
	{
		return !m_EnclosureId.IsNull();
	}

	inline bool IsInsideSpacecraft() const
	{
		if(!IsInSpacecraft())
			return false;
		return m_CapRoomType != ROOM_None;
	}

	inline bool IsInSpacecraft() const
	{
		return m_Environment & EB_Spacecraft;
	}

	inline bool IsInVehicle() const
	{
		return m_Environment & EB_InVehicle;
	}

	inline bool IsInWater() const
	{
		return m_Environment & EB_InWater;
	}

	inline bool IsMouthUnderWater() const
	{
		return m_Environment & EB_MouthUnderwater;
	}

	inline bool IsOnBuildingFloor() const
	{
		return m_OnGround == OG_HardFloor || m_OnGround == OG_WoodFloor;
	}

	inline bool IsOnGround() const
	{
		return m_OnGround != OG_None;
	}

	inline bool IsOnNaturalGround() const
	{
		switch(m_OnGround)
		{
		case OG_None:
		case OG_HardFloor:
		case OG_Metal:
		case OG_Pavement:
		case OG_Snow:		// In the one case where this function is used, I want snow to return false.
		case OG_WoodFloor:
			return false;
		case OG_Forest:
		case OG_Rock:
		case OG_Sand:
		case OG_Savanna:
		case OG_WaterSoundOnly:
			return true;
		}
		return false;
	}

	inline bool IsOnWater() const
	{
		return m_Environment & EB_OnWater;
	}

	quint64 MemoryUsage() const;

	inline bool OnStairs() const
	{
		return m_Environment & EB_Stairs;
	}

	inline ORBITZONE OrbitZone() const
	{
		return m_OrbitZone;
	}

	inline float SeaDepth() const
	{
		return m_fSeaDepth;
	}

	inline quint16 VesselRoomId() const
	{
		if(!IsInSpacecraft())
			return 0;
		return m_CapRoomId;
	}

	inline QString VesselRoomName() const
	{
		if(!IsInSpacecraft())
			return 0;
		return m_CapRoomName;
	}

	inline ROOMTYPE VesselRoomType() const
	{
		if(!IsInSpacecraft())
			return ROOM_None;
		return m_CapRoomType;
	}

	inline quint8 WorldAtomType() const
	{
		return m_WorldAtomType;
	}
};

#endif // AuQueryEnvironment_H
