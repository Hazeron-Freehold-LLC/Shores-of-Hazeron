//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumemory.cpp,v 1.25 2022/02/19 17:40:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumemory.h"
//
#include "augear.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworld.h"
#include "dhdna.h"

AuMemory::AuMemory()
{
	m_CreateTime.Set();
}

AuMemory::~AuMemory()
{
}

AuMemory::AuMemory(const AuMemory& c)
{
	m_CreateTime = c.m_CreateTime;
}

const AuMemory& AuMemory::operator=(const AuMemory& c)
{
	m_CreateTime = c.m_CreateTime;
	return *this;
}

void AuMemory::operator>>(QDataStream& out) const
{
	out << m_CreateTime.UTC();
}

void AuMemory::operator<<(QDataStream& in)
{
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_CreateTime = tm;
}

AuMemory* AuMemory::CreateMemory(quint8 memType)
{
	if(memType == AuMemCharacterDNA3::MemoryType())
		return new AuMemCharacterDNA3;
	else if(memType == AuMemCharacterDNA4::MemoryType())
		return new AuMemCharacterDNA4;
	else if(memType == AuMemStory::MemoryType())
		return new AuMemStory;
	else if(memType == AuMemStory1::MemoryType())
		return new AuMemStory1;
	else if(memType == AuMemStory2::MemoryType())
		return new AuMemStory2;
	else if(memType == AuMemCity::MemoryType())
		return new AuMemCity;
	else if(memType == AuMemClaim::MemoryType())
		return new AuMemClaim;
	else
		qWarning() << "error: AuMemory::CreateMemory unknown memType" << memType
			<< "Did you add a new memory type and forget to update CreateMemory?";
	return 0;
}

quint64 AuMemory::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemory);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuMemory& mem)
{
	mem.Debug(dbg);
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuMemories::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
	{
		out << (*ci)->Type();
		out << **ci;
	}
}

void AuMemories::operator<<(QDataStream& in)
{
	Clear();

	quint16 memories;
	in >> memories;
	quint8 type;
	while(memories--)
	{
		in >> type;
		AuMemory* memory = AuMemory::CreateMemory(type);
		if(memory)
		{
			in >> *memory;
			append(memory);
		}
		else
			qWarning() << "AuMemories::operator<< received invalid memory type" << type;
	}
}

void AuMemories::Remember(const AuMemory& mem)
{
	// Attempt to refresh existing memory.
	iterator mi = begin();
	while(mi != end())
	{
		if((*mi)->IsReplacement(mem))
		{
			delete *mi;
			mi = erase(mi);				// Existing memory is forgotten in favor of the new memory.
		}
		else if((*mi)->RefreshMemory(mem))
			return;						// Existing memory refreshed.
		else
			mi++;
	}

	// Existing memory not found or forgotten, store a new memory.
	append(mem.Clone());
}

AuTime AuMemories::FirstMemoryOfCity(const AuId& cityId) const
{
	AuTime firstMem;
	if(cityId.IsNull())
		return firstMem;

	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		if((*mi)->CityId() != cityId)
			continue;

		if(firstMem.IsNull() || (*mi)->CreateTime() < firstMem)
			firstMem = (*mi)->CreateTime();
	}

	return firstMem;
}

// Removes all memories associated with 'scriptId'.
void AuMemories::ForgetCity(const AuId& cityId)
{
	if(cityId.IsNull())
		return;

	iterator mi = begin();
	while(mi != end())
	{
		if((*mi)->CityId() == cityId)
		{
			delete *mi;
			mi = erase(mi);
		}
		else
			mi++;
	}
}

// Removes all memories associated with 'scriptId'.
void AuMemories::ForgetStoryStarted(quint32 scriptId)
{
	iterator mi = begin();
	while(mi != end())
		if((*mi)->ScriptId() == scriptId)
		{
			delete *mi;
			mi = erase(mi);
		}
		else
			mi++;
}

// Returns true if scriptId has ever been started.
bool AuMemories::HasStartedStory(quint32 scriptId) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi)->IsStory(scriptId))
			return true;
	return false;
}

// Returns last time scriptId was started.
// Returns AuTime() if it was never started.
AuTime AuMemories::LastStartedStory(quint32 scriptId) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi)->IsStory(scriptId))
			return (*mi)->ScriptLastStarted();
	return AuTime();
}

AuTime AuMemories::LastTerminatedStory(quint32 scriptId) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi)->IsStory(scriptId))
			return (*mi)->ScriptLastTerminated();
	return AuTime();
}

quint8 AuMemories::LastStoryResult(quint32 scriptId) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi)->IsStory(scriptId))
			return (*mi)->ScriptResult();
	return 0;
}

void AuMemories::RememberStoryResult(quint32 scriptId, quint8 result)
{
	for(iterator mi = begin(); mi != end(); mi++)
		if((*mi)->IsStory(scriptId))
		{
			AuMemStory1* ms1 = dynamic_cast<AuMemStory1*>(*mi);
			if(ms1)
			{
				// Must replace old memory with new memory to store the result.
				AuMemStory* ms = new AuMemStory(*ms1);
				ms->RememberScriptResult(result);
				delete ms1;
				erase(mi);
				append(ms);
				break;
			}
			else
				(*mi)->RememberScriptResult(result);
		}
}

void AuMemories::RememberStoryTerminated(quint32 scriptId)
{
	for(iterator mi = begin(); mi != end(); mi++)
		if((*mi)->IsStory(scriptId))
		{
			AuMemStory1* ms1 = dynamic_cast<AuMemStory1*>(*mi);
			AuMemStory2* ms2 = dynamic_cast<AuMemStory2*>(*mi);
			if(ms1)
			{
				// Must replace old memory with new memory to store the result.
				AuMemStory* ms = new AuMemStory(*ms1);
				ms->RememberScriptTerminated();
				delete ms1;
				erase(mi);
				append(ms);
				break;
			}
			else if(ms2)
			{
				// Must replace old memory with new memory to store the result.
				AuMemStory* ms = new AuMemStory(*ms2);
				ms->RememberScriptTerminated();
				delete ms2;
				erase(mi);
				append(ms);
				break;
			}
			else
				(*mi)->RememberScriptTerminated();
		}
}

quint64 AuMemories::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemories);
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		mem += (*mi)->MemoryUsage();
	return mem;
}

bool AuMemories::RecallCharacter(const QString& charName, AuMemCharacterDNA4& mem) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		const AuMemCharacterDNA4* cm = dynamic_cast<const AuMemCharacterDNA4*>(*mi);
		if(!cm)
			continue;
		if(cm->Name() != charName)
			continue;
		mem = *cm;
		return true;
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemCharacterDNA3::AuMemCharacterDNA3(const QString& name, const DhDNA& dna, GENDER gender, quint32 scriptId)
{
	m_DNA = new DhDNA(dna);
	m_Gender = gender;
	m_Name = name;
	m_ScriptId = scriptId;
}

AuMemCharacterDNA3::AuMemCharacterDNA3()
{
	m_DNA = new DhDNA;
	m_Gender = 0;
	m_ScriptId = 0;
}

AuMemCharacterDNA3::~AuMemCharacterDNA3()
{
	delete m_DNA;
}

AuMemCharacterDNA3::AuMemCharacterDNA3(const AuMemCharacterDNA3& c) : AuMemory(c)
{
	m_DNA = new DhDNA(*c.m_DNA);
	m_Gender = c.m_Gender;
	m_Name = c.m_Name;
	m_ScriptId = c.m_ScriptId;
}

void AuMemCharacterDNA3::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << *m_DNA;
	out << m_Gender;
	out << m_Name;
	out << m_ScriptId;
}

void AuMemCharacterDNA3::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> *m_DNA;
	in >> m_Gender;
	in >> m_Name;
	in >> m_ScriptId;
}

QString AuMemCharacterDNA3::Desc() const
{
	QString g(m_Gender ? "Male" : "Female");
	return QString("Story character %2, %3 DNA %4, first met %1. Last seen in story script %5.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_Name)
		.arg(g)
		.arg(m_DNA->SpeciesDescription())
		.arg(m_ScriptId);
}

void AuMemCharacterDNA3::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemCharacterDNA3::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemCharacterDNA3);
	mem += StringMemoryUsage(m_Name);
	return mem;
}

bool AuMemCharacterDNA3::RefreshMemory(const AuMemory& mem)
{
	try
	{
		const AuMemCharacterDNA3& c = dynamic_cast<const AuMemCharacterDNA3&>(mem);

		if(m_Name != c.m_Name)
			return false;
		if(m_Gender != c.m_Gender)
			return false;

		// We don't check DNA. A story could be restarted on a world with different indigenous DNA, which could have been the
		// source for this dude. Then there's the possibility of adding shape shifting characters. D:
		//if(m_DNA != m.m_DNA)
			//return false;

		*m_DNA = *c.m_DNA;
		m_ScriptId = c.m_ScriptId;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

bool AuMemCharacterDNA3::IsReplacement(const AuMemory& mem) const
{
	try
	{
		const AuMemCharacterDNA4& c = dynamic_cast<const AuMemCharacterDNA4&>(mem);
		return m_ScriptId == c.ScriptId() && m_Name == c.Name() && Chromatid() == c.Chromatid();
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

quint8 AuMemCharacterDNA3::Chromatid() const
{
	return m_Gender == Male ? AuDNABase::Alpha : AuDNABase::Beta;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemStory1::AuMemStory1(quint32 scriptId)
{
	m_LastStarted.Set();
	m_ScriptId = scriptId;
}

AuMemStory1::AuMemStory1(quint32 scriptId, const AuTime& whenStarted)
{
	m_LastStarted = whenStarted;
	m_ScriptId = scriptId;
}

AuMemStory1::AuMemStory1()
{
	m_ScriptId = 0;
}

AuMemStory1::AuMemStory1(const AuMemStory1& c) : AuMemory(c)
{
	m_LastStarted = c.m_LastStarted;
	m_ScriptId = c.m_ScriptId;
}

void AuMemStory1::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << m_LastStarted;
	out << m_ScriptId;
}

void AuMemStory1::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> m_LastStarted;
	in >> m_ScriptId;
}

QString AuMemStory1::Desc() const
{
	return QString("Story script %2 first started %1, last started %3.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_ScriptId)
		.arg(m_LastStarted.UTC().toString());
}

void AuMemStory1::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemStory1::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemStory1);
	return mem;
}

bool AuMemStory1::IsReplacement(const AuMemory& mem) const
{
	try
	{
		const AuMemStory& c = dynamic_cast<const AuMemStory&>(mem);
		return m_ScriptId == c.ScriptId();
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemStory2::AuMemStory2(quint32 scriptId)
{
	m_LastStarted.Set();
	m_Result = 0;
	m_ScriptId = scriptId;
}

AuMemStory2::AuMemStory2(quint32 scriptId, const AuTime& whenStarted, quint8 result)
{
	m_LastStarted = whenStarted;
	m_Result = result;
	m_ScriptId = scriptId;
}

AuMemStory2::AuMemStory2()
{
	m_Result = 0;
	m_ScriptId = 0;
}

AuMemStory2::AuMemStory2(const AuMemStory2& c) : AuMemory(c)
{
	m_LastStarted = c.m_LastStarted;
	m_Result = c.m_Result;
	m_ScriptId = c.m_ScriptId;
}

void AuMemStory2::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << m_ScriptId;
	out << m_LastStarted;
	out << m_Result;
}

void AuMemStory2::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> m_ScriptId;
	in >> m_LastStarted;
	in >> m_Result;
}

QString AuMemStory2::Desc() const
{
	return QString("Story script %2 first started %1, last started %3, result %4.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_ScriptId)
		.arg(m_LastStarted.UTC().toString())
		.arg(m_Result);
}

void AuMemStory2::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemStory2::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemStory2);
	return mem;
}

bool AuMemStory2::IsReplacement(const AuMemory& mem) const
{
	try
	{
		const AuMemStory& c = dynamic_cast<const AuMemStory&>(mem);
		return m_ScriptId == c.ScriptId();
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemStory::AuMemStory(quint32 scriptId)
{
	m_LastStarted.Set();
	m_Result = 0;
	m_ScriptId = scriptId;
}

AuMemStory::AuMemStory(quint32 scriptId, const AuTime& whenStarted, quint8 result)
{
	m_LastStarted = whenStarted;
	m_Result = result;
	m_ScriptId = scriptId;
}

AuMemStory::AuMemStory()
{
	m_Result = 0;
	m_ScriptId = 0;
}

AuMemStory::AuMemStory(const AuMemStory& c) : AuMemory(c)
{
	m_LastStarted = c.m_LastStarted;
	m_LastTerminated = c.m_LastTerminated;
	m_Result = c.m_Result;
	m_ScriptId = c.m_ScriptId;
}

AuMemStory::AuMemStory(const AuMemStory1& c) : AuMemory(c)
{
	m_LastStarted = c.ScriptLastStarted();
	m_Result = 0;
	m_ScriptId = c.ScriptId();
}

AuMemStory::AuMemStory(const AuMemStory2& c) : AuMemory(c)
{
	m_LastStarted = c.ScriptLastStarted();
	m_Result = c.ScriptResult();
	m_ScriptId = c.ScriptId();
}

void AuMemStory::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << m_ScriptId;
	out << m_LastStarted;
	out << m_LastTerminated;
	out << m_Result;
}

void AuMemStory::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> m_ScriptId;
	in >> m_LastStarted;
	in >> m_LastTerminated;
	in >> m_Result;
}

QString AuMemStory::Desc() const
{
	if(m_LastTerminated == AuTime())
		return QString("Story script %2 first started %1, last started %3, result %4.")
			.arg(m_CreateTime.UTC().toString())
			.arg(m_ScriptId)
			.arg(m_LastStarted.UTC().toString())
			.arg(m_Result);
	else
		return QString("Story script %2 first started %1, last started %3, last ended %5, result %4.")
			.arg(m_CreateTime.UTC().toString())
			.arg(m_ScriptId)
			.arg(m_LastStarted.UTC().toString())
			.arg(m_Result)
			.arg(m_LastTerminated.UTC().toString());
}

void AuMemStory::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemStory::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemStory);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemCity::AuMemCity(const AuId& cityId, const QString& cityName, const AuGlobeLLF& cityLL, const AuId& worldId,
		const QString& worldName, const AuId& systemId, const QString& systemName, const AuId& sectorId, const QString& sectorName,
		GALAXY galaxy)
{
	m_CityId = cityId;
	m_CityLL = cityLL;
	m_CityName = cityName;
	m_Galaxy = galaxy;
	m_SectorId = sectorId;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
	m_WorldId = worldId;
	m_WorldName = worldName;
}

AuMemCity::AuMemCity()
{
	m_Galaxy = GALAXY_Count;
}

AuMemCity::AuMemCity(const AuMemCity& c) : AuMemory(c)
{
	m_CityId = c.m_CityId;
	m_CityLL = c.m_CityLL;
	m_CityName = c.m_CityName;
	m_Galaxy = c.m_Galaxy;
	m_SectorId = c.m_SectorId;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_WorldId = c.m_WorldId;
	m_WorldName = c.m_WorldName;
}

void AuMemCity::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << m_CityId;
	out << m_CityLL;
	out << m_CityName;
	out << m_Galaxy;
	out << m_SectorId;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
	out << m_WorldId;
	out << m_WorldName;
}

void AuMemCity::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> m_CityId;
	in >> m_CityLL;
	in >> m_CityName;
	in >> m_Galaxy;
	in >> m_SectorId;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_WorldId;
	in >> m_WorldName;
}

QString AuMemCity::Desc() const
{
	return QString("City %2 %3, %4, first entered %1.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_CityId.GetID())
		.arg(m_CityName)
		.arg(GalaxyName((GALAXY)m_Galaxy));
}

void AuMemCity::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemCity::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemCity);
	mem += StringMemoryUsage(m_CityName);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	mem += StringMemoryUsage(m_WorldName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemClaim::AuMemClaim(const AuGlobeLLF& claimLL, const AuId& worldId, const QString& worldName, const AuId& systemId,
		const QString& systemName, const AuId& sectorId, const QString& sectorName, GALAXY galaxy)
{
	m_ClaimLL = claimLL;
	m_Galaxy = galaxy;
	m_SectorId = sectorId;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
	m_WorldId = worldId;
	m_WorldName = worldName;
}

AuMemClaim::AuMemClaim()
{
	m_Galaxy = GALAXY_Count;
}

AuMemClaim::AuMemClaim(const AuMemClaim& c) : AuMemory(c)
{
	m_ClaimLL = c.m_ClaimLL;
	m_Galaxy = c.m_Galaxy;
	m_SectorId = c.m_SectorId;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_WorldId = c.m_WorldId;
	m_WorldName = c.m_WorldName;
}

void AuMemClaim::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	out << m_ClaimLL;
	out << m_Galaxy;
	out << m_SectorId;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
	out << m_WorldId;
	out << m_WorldName;
}

void AuMemClaim::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	in >> m_ClaimLL;
	in >> m_Galaxy;
	in >> m_SectorId;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_WorldId;
	in >> m_WorldName;
}

QString AuMemClaim::Desc() const
{
	return QString("Claim %2 %3, %4, first claimed %1.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_WorldId.GetID())
		.arg(m_WorldName)
		.arg(GalaxyName((GALAXY)m_Galaxy));
}

void AuMemClaim::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemClaim::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemClaim);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	mem += StringMemoryUsage(m_WorldName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMemCharacterDNA4::AuMemCharacterDNA4(const QString& name, const AuDNABase& dna, quint8 chromatid, quint32 scriptId,
		AuHairStyle::STYLE hair, AuHairStyle::SHAVE shave, HANDEDNESS hand, GROWTHSTAGE stage, quint32 empireId,
		const AuGear* gear, GALAXY homeGalaxy, const AuId& homeSectorId, const QString& homeSectorName, const AuId& homeSystemId,
		const QString& homeSystemName, const AuId& homeWorldId, const QString& homeWorldName, BODY_TYPE homeWorldBodyType)
{
	m_Chromatid = chromatid;
	m_DNA = dna.Clone();
	m_EmpireId = empireId;
	if(gear)
		m_Gear = new AuGear(*gear);
	else
		m_Gear = 0;
	m_Hair = hair;
	m_Hand = hand;
	m_Name = name;
	m_ScriptId = scriptId;
	m_Shave = shave;
	m_Stage = stage;

	m_Galaxy = homeGalaxy;
	m_SectorId = homeSectorId;
	m_SectorName = homeSectorName;
	m_SystemId = homeSystemId;
	m_SystemName = homeSystemName;
	m_WorldBodyType = homeWorldBodyType;
	m_WorldId = homeWorldId;
	m_WorldName = homeWorldName;
}

AuMemCharacterDNA4::AuMemCharacterDNA4(const QString& name, const AuDNABase& dna, quint8 chromatid, quint32 scriptId,
		AuHairStyle::STYLE hair, AuHairStyle::SHAVE shave, HANDEDNESS hand, GROWTHSTAGE stage, quint32 empireId,
		const AuGear* gear, const DaWorld* homeWorld)
{
	m_Chromatid = chromatid;
	m_DNA = dna.Clone();
	m_EmpireId = empireId;
	if(gear)
		m_Gear = new AuGear(*gear);
	else
		m_Gear = 0;
	m_Hair = hair;
	m_Hand = hand;
	m_Name = name;
	m_ScriptId = scriptId;
	m_Shave = shave;
	m_Stage = stage;

	if(homeWorld)
	{
		m_Galaxy = homeWorld->Galaxy();
		m_WorldBodyType = homeWorld->BodyType();
		m_WorldId = homeWorld->Id();
		m_WorldName = homeWorld->Name();

		const DaSolarSystem* system = homeWorld->FindParentSolarSystemInEnv();
		if(system)
		{
			m_SystemId = system->Id();
			m_SystemName = system->Name();

			const DaSector* sector = system->FindParentSector();
			if(sector)
			{
				m_SectorId = sector->Id();
				m_SectorName = sector->Name();
			}
		}
	}
	else
	{
		m_Galaxy = GALAXY_Count;
		m_WorldBodyType = BODY_TYPE_Count;
	}
}

AuMemCharacterDNA4::AuMemCharacterDNA4()
{
	m_Chromatid = AuDNABase::Alpha;
	m_DNA = 0;
	m_EmpireId = 0;
	m_Gear = 0;
	m_Hair = AuHairStyle::Unkempt;
	m_Hand = LeftHanded;
	m_ScriptId = 0;
	m_Shave = AuHairStyle::Unshaven;
	m_Stage = Mature;

	m_Galaxy = GALAXY_Count;
	m_WorldBodyType = BODY_TYPE_Count;
}

AuMemCharacterDNA4::~AuMemCharacterDNA4()
{
	if(m_DNA)
		delete m_DNA;
	if(m_Gear)
		delete m_Gear;
}

AuMemCharacterDNA4::AuMemCharacterDNA4(const AuMemCharacterDNA4& c) : AuMemory(c)
{
	m_Chromatid = c.m_Chromatid;
	if(c.m_DNA)
		m_DNA = c.m_DNA->Clone();
	else
		m_DNA = 0;
	m_EmpireId = c.m_EmpireId;
	if(c.m_Gear)
		m_Gear = new AuGear(*c.m_Gear);
	else
		m_Gear = 0;
	m_Hair = c.m_Hair;
	m_Hand = c.m_Hand;
	m_Name = c.m_Name;
	m_ScriptId = c.m_ScriptId;
	m_Shave = c.m_Shave;
	m_Stage = c.m_Stage;

	m_Galaxy = c.m_Galaxy;
	m_SectorId = c.m_SectorId;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_WorldBodyType = c.m_WorldBodyType;
	m_WorldId = c.m_WorldId;
	m_WorldName = c.m_WorldName;
}

const AuMemCharacterDNA4& AuMemCharacterDNA4::operator=(const AuMemCharacterDNA4& c)
{
	AuMemory::operator=(c);
	m_Chromatid = c.m_Chromatid;
	if(m_DNA)
		delete m_DNA;
	if(c.m_DNA)
		m_DNA = c.m_DNA->Clone();
	else
		m_DNA = 0;
	m_EmpireId = c.m_EmpireId;
	if(m_Gear)
		delete m_Gear;
	if(c.m_Gear)
		m_Gear = new AuGear(*c.m_Gear);
	else
		m_Gear = 0;
	m_Hair = c.m_Hair;
	m_Hand = c.m_Hand;
	m_Name = c.m_Name;
	m_ScriptId = c.m_ScriptId;
	m_Shave = c.m_Shave;
	m_Stage = c.m_Stage;

	m_Galaxy = c.m_Galaxy;
	m_SectorId = c.m_SectorId;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_WorldBodyType = c.m_WorldBodyType;
	m_WorldId = c.m_WorldId;
	m_WorldName = c.m_WorldName;
	return *this;
}

void AuMemCharacterDNA4::operator>>(QDataStream& out) const
{
	AuMemory::operator>>(out);
	quint8 hasWhat = 0;
	if(m_DNA)
		hasWhat |= 0x01;
	if(m_Hair != AuHairStyle::Unkempt || m_Shave != AuHairStyle::Unshaven || m_Hand != LeftHanded || m_Stage != Mature)
		hasWhat |= 0x02;
	if(m_EmpireId)
		hasWhat |= 0x04;
	if(m_Gear)
		hasWhat |= 0x08;
	if(!m_WorldId.IsNull())
		hasWhat |= 0x10;
	out << hasWhat;

	if(hasWhat & 0x01)
		m_DNA->WriteDynamic(out);
	if(hasWhat & 0x02)
	{
		out << (quint8)m_Hair;
		out << (quint8)m_Shave;
		out << (quint8)m_Hand;
		out << (quint8)m_Stage;
	}
	if(hasWhat & 0x04)
		out << m_EmpireId;
	if(hasWhat & 0x08)
		out << *m_Gear;
	out << m_Chromatid;
	out << m_Name;
	out << m_ScriptId;

	if(hasWhat & 0x10)
	{
		out << (quint8)m_Galaxy;
		out << m_SectorId;
		out << m_SectorName;
		out << m_SystemId;
		out << m_SystemName;
		out << m_WorldBodyType;
		out << m_WorldId;
		out << m_WorldName;
	}
}

void AuMemCharacterDNA4::operator<<(QDataStream& in)
{
	AuMemory::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;

	if(m_DNA)
		delete m_DNA;
	if(hasWhat & 0x01)
		m_DNA = AuDNABase::ReadDynamic(in);
	else
		m_DNA = 0;

	if(hasWhat & 0x02)
	{
		quint8 ui8;
		in >> ui8;
		m_Hair = (AuHairStyle::STYLE)ui8;
		in >> ui8;
		m_Shave = (AuHairStyle::SHAVE)ui8;
		in >> ui8;
		m_Hand = (HANDEDNESS)ui8;
		in >> ui8;
		m_Stage = (GROWTHSTAGE)ui8;
	}

	if(hasWhat & 0x04)
		in >> m_EmpireId;
	if(hasWhat & 0x08)
	{
		if(!m_Gear)
			m_Gear = new AuGear;
		in >> *m_Gear;
	}

	in >> m_Chromatid;
	in >> m_Name;
	in >> m_ScriptId;

	if(hasWhat & 0x10)
	{
		quint8 ui8;
		in >> ui8;
		m_Galaxy = (GALAXY)ui8;
		in >> m_SectorId;
		in >> m_SectorName;
		in >> m_SystemId;
		in >> m_SystemName;
		in >> m_WorldBodyType;
		in >> m_WorldId;
		in >> m_WorldName;
	}
}

QString AuMemCharacterDNA4::Desc() const
{
	QString g(GenderName(m_DNA->Gender((AuDNABase::CHROMATID)Chromatid())));
	return QString("Story character %2, %3 DNA %4, first met %1. Last seen in story script %5.")
		.arg(m_CreateTime.UTC().toString())
		.arg(m_Name)
		.arg(g)
		.arg(m_DNA->SpeciesDescription())
		.arg(m_ScriptId);
}

void AuMemCharacterDNA4::Debug(QDebug dbg) const
{
	dbg.nospace() << Desc();
}

quint64 AuMemCharacterDNA4::MemoryUsage() const
{
	quint64 mem = sizeof(AuMemCharacterDNA4);
	mem += StringMemoryUsage(m_Name);
	return mem;
}

bool AuMemCharacterDNA4::RefreshMemory(const AuMemory& mem)
{
	try
	{
		const AuMemCharacterDNA4& c = dynamic_cast<const AuMemCharacterDNA4&>(mem);
		if(m_Chromatid != c.m_Chromatid)
			return false;
		if(m_Hair != c.m_Hair)
			return false;
		if(m_Hand != c.m_Hand)
			return false;
		if(m_Name != c.m_Name)
			return false;
		if(m_Shave != c.m_Shave)
			return false;

		// We don't check DNA. A story could be restarted on a world with different indigenous DNA, which could have been the
		// source for this dude. Then there's the possibility of adding shape shifting characters. D:
		//if(m_DNA != m.m_DNA)
			//return false;

		if(m_DNA && c.m_DNA)
			*m_DNA = *c.m_DNA;
		else if(!m_DNA && c.m_DNA)
			m_DNA = c.m_DNA->Clone();
		m_EmpireId = c.m_EmpireId;
		if(m_Gear)
			delete m_Gear;
		if(c.m_Gear)
			m_Gear = new AuGear(*c.m_Gear);
		else
			m_Gear = 0;
		m_ScriptId = c.m_ScriptId;
		m_Stage = c.m_Stage;

		m_Galaxy = c.m_Galaxy;
		m_SectorId = c.m_SectorId;
		m_SectorName = c.m_SectorName;
		m_SystemId = c.m_SystemId;
		m_SystemName = c.m_SystemName;
		m_WorldBodyType = c.m_WorldBodyType;
		m_WorldId = c.m_WorldId;
		m_WorldName = c.m_WorldName;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

quint8 AuMemCharacterDNA4::Chromatid() const
{
	return m_Chromatid;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

