//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auqueryenvironment.cpp,v 1.10 2022/02/19 17:40:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auqueryenvironment.h"
//

quint64 AuQueryEnvironment::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_AtmType);
	mem += sizeof(m_BdCityId);
	mem += sizeof(m_BdId);
	mem += sizeof(m_CapRoomId);
	mem += StringMemoryUsage(m_CapRoomName);
	mem += sizeof(m_CapRoomType);
	mem += sizeof(m_CityEmpire);
	mem += sizeof(m_colAtmosphere);
	mem += sizeof(m_EnclosureId);
	mem += sizeof(m_Environment);
	mem += sizeof(m_FallDist);
	mem += sizeof(m_fAltitudeAGL);
	mem += sizeof(m_fAltitudeMSL);
	mem += sizeof(m_fCeilingDist);
	mem += sizeof(m_fCGAltitude);
	mem += sizeof(m_fAirDensity);
	mem += sizeof(m_fAtmDensity);
	mem += sizeof(m_fSeaDepth);
	mem += sizeof(m_GlobeLL);
	mem += sizeof(m_GravityNormal);
	mem += sizeof(m_InIndustry);
	mem += sizeof(m_InIndustryLL);
	mem += sizeof(m_OnGround);
	mem += sizeof(m_OrbitZone);
	mem += sizeof(m_Galaxy);
	mem += sizeof(m_terNormal);
	mem += sizeof(m_WaterDepth);
	mem += sizeof(m_WorldAtomType);
	return mem;
}
