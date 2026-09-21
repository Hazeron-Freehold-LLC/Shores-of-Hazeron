//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumemory.h,v 1.19 2022/02/19 17:32:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMemory_H
#define AuMemory_H

#include "auatom.h"
#include "auglobell.h"
#include "auhair.h"
#include "auid.h"
#include "autime.h"
#include <QDebug>
#include <QList>
#include <typeinfo>			// for bad_cast

class AuDNABase;
class AuGear;
class DaWorld;
class DhDNA;

class AuMemory
{
protected:
	AuTime						m_CreateTime;					// Time this memory was created.

public:	// Constructors/destructors
	AuMemory();													// Sets created time to current time.
	virtual ~AuMemory();
	AuMemory(const AuMemory& c);

public:	// Operators
	const AuMemory& operator=(const AuMemory& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuId CityId() const
	{
		return AuId();
	}

	// Returns a duplicate of this memory, allocated on the heap.
	virtual AuMemory* Clone() const = 0;

	static AuMemory* CreateMemory(quint8 memType);

	const AuTime& CreateTime() const
	{
		return m_CreateTime;
	}

	virtual void Debug(QDebug dbg) const = 0;

	virtual QString Desc() const = 0;

	// Returns true if 'mem' must replace this memory when Remember'ing, instead of Refresh'ing the memory.
	virtual bool IsReplacement(const AuMemory& mem) const = 0;

	// Returns true for the primary AuMemStory memory for the 'scriptId'. Other memories associated with the script id return false.
	virtual bool IsStory(quint32 scriptId) const
	{
		Q_UNUSED(scriptId);
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual void RememberScriptResult(quint8 result)
	{
		Q_UNUSED(result);
	}

	virtual void RememberScriptTerminated()
	{
	}

	virtual quint32 ScriptId() const
	{
		return 0;
	}

	virtual AuTime ScriptLastStarted() const
	{
		return AuTime();
	}

	virtual AuTime ScriptLastTerminated() const
	{
		return AuTime();
	}

	virtual quint8 ScriptResult() const
	{
		return 0;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const = 0;

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem) = 0;
};

inline QDataStream& operator<<(QDataStream& out, const AuMemory& mem)
{
	mem >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMemory& mem)
{
	mem << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuMemory& mem);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// First version of character memory using strictly DhDNA.

class AuMemCharacterDNA3 : public AuMemory
{
private:
	DhDNA*								m_DNA;
	quint8								m_Gender;				// 0=female, 1=male
	QString								m_Name;
	quint32								m_ScriptId;				// Script they were in.

public:	// Construction/destruction
	AuMemCharacterDNA3(const QString& name, const DhDNA& dna, GENDER gender, quint32 scriptId);
	AuMemCharacterDNA3();
	AuMemCharacterDNA3(const AuMemCharacterDNA3& c);
	~AuMemCharacterDNA3();

public:	// Operators
	const AuMemCharacterDNA3& operator=(const AuMemCharacterDNA3& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	quint8 Chromatid() const;

	virtual AuMemory* Clone() const
	{
		return new AuMemCharacterDNA3(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const;

	static quint8 MemoryType()
	{
		return 1;
	}

	virtual quint64 MemoryUsage() const;

	virtual quint32 ScriptId() const
	{
		return m_ScriptId;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	bool RefreshMemory(const AuMemory& mem);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// First version of AuMemStory, without result code.

class AuMemStory1 : public AuMemory
{
private:
	AuTime								m_LastStarted;			// Last time the script was started.
	quint32								m_ScriptId;				// Story script.

public:	// Construction/destruction
	AuMemStory1(quint32 scriptId);
	AuMemStory1(quint32 scriptId, const AuTime& whenStarted);
	AuMemStory1();
	AuMemStory1(const AuMemStory1& c);

public:	// Operators
	const AuMemStory1& operator=(const AuMemStory1& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuMemory* Clone() const
	{
		return new AuMemStory1(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const;

	virtual bool IsStory(quint32 scriptId) const
	{
		return m_ScriptId == scriptId;
	}

	static quint8 MemoryType()
	{
		return 2;
	}

	virtual quint64 MemoryUsage() const;

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem)
	{
		try
		{
			const AuMemStory1& c = dynamic_cast<const AuMemStory1&>(mem);

			if(m_ScriptId != c.m_ScriptId)
				return false;

			m_LastStarted = c.m_LastStarted;			// Remember its last started time, which is its create time.
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual quint32 ScriptId() const
	{
		return m_ScriptId;
	}

	virtual AuTime ScriptLastStarted() const
	{
		return m_LastStarted;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMemCity : public AuMemory
{
private:
	AuId								m_CityId;
	AuGlobeLLF							m_CityLL;
	QString								m_CityName;
	quint8								m_Galaxy;
	AuId								m_SectorId;
	QString								m_SectorName;
	AuId								m_SystemId;
	QString								m_SystemName;
	AuId								m_WorldId;
	QString								m_WorldName;

public:	// Construction/destruction
	AuMemCity(const AuId& cityId, const QString& cityName, const AuGlobeLLF& cityLL, const AuId& worldId, const QString& worldName,
			const AuId& systemId, const QString& systemName, const AuId& sectorId, const QString& sectorName, GALAXY galaxy);
	AuMemCity();
	AuMemCity(const AuMemCity& c);

public:	// Operators
	const AuMemCity& operator=(const AuMemCity& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuId CityId() const
	{
		return m_CityId;
	}

	virtual AuMemory* Clone() const
	{
		return new AuMemCity(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const
	{
		Q_UNUSED(mem);
		return false;	// 'mem' does not replace this memory.
	}

	static quint8 MemoryType()
	{
		return 3;
	}

	virtual quint64 MemoryUsage() const;

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem)
	{
		try
		{
			const AuMemCity& c = dynamic_cast<const AuMemCity&>(mem);

			if(m_CityId != c.m_CityId)
				return false;

			// m_CityLL;	// Won't change.
			m_CityName = c.m_CityName;
			// m_Galaxy;	// Won't change.
			// m_SectorId;	// Won't change.
			m_SectorName = c.m_SectorName;
			// m_SystemId;	// Won't change.
			m_SystemName = c.m_SystemName;
			// m_WorldId;	// Won't change.
			m_WorldName = c.m_WorldName;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMemClaim : public AuMemory
{
private:
	AuGlobeLLF							m_ClaimLL;
	quint8								m_Galaxy;
	AuId								m_SectorId;
	QString								m_SectorName;
	AuId								m_SystemId;
	QString								m_SystemName;
	AuId								m_WorldId;
	QString								m_WorldName;

public:	// Construction/destruction
	AuMemClaim(const AuGlobeLLF& claimLL, const AuId& worldId, const QString& worldName, const AuId& systemId,
			const QString& systemName, const AuId& sectorId, const QString& sectorName, GALAXY galaxy);
	AuMemClaim();
	AuMemClaim(const AuMemClaim& c);

public:	// Operators
	const AuMemClaim& operator=(const AuMemClaim& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuMemory* Clone() const
	{
		return new AuMemClaim(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const
	{
		Q_UNUSED(mem);
		return false;	// 'mem' does not replace this memory.
	}

	static quint8 MemoryType()
	{
		return 4;
	}

	virtual quint64 MemoryUsage() const;

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem)
	{
		try
		{
			const AuMemClaim& c = dynamic_cast<const AuMemClaim&>(mem);

			m_ClaimLL = c.m_ClaimLL;
			// m_Galaxy;	// Won't change.
			// m_SectorId;	// Won't change.
			m_SectorName = c.m_SectorName;
			// m_SystemId;	// Won't change.
			m_SystemName = c.m_SystemName;
			// m_WorldId;	// Won't change.
			m_WorldName = c.m_WorldName;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Second version of AuMemStory, without last ended time.

class AuMemStory2 : public AuMemory
{
private:
	AuTime								m_LastStarted;			// Last time the script was started.
	quint8								m_Result;				// Result of running this story script.
	quint32								m_ScriptId;				// Story script.

public:	// Construction/destruction
	AuMemStory2(quint32 scriptId);
	AuMemStory2(quint32 scriptId, const AuTime& whenStarted, quint8 result);
	AuMemStory2();
	AuMemStory2(const AuMemStory2& c);

public:	// Operators
	const AuMemStory2& operator=(const AuMemStory2& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuMemory* Clone() const
	{
		return new AuMemStory2(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const;

	virtual bool IsStory(quint32 scriptId) const
	{
		return m_ScriptId == scriptId;
	}

	static quint8 MemoryType()
	{
		return 5;
	}

	virtual quint64 MemoryUsage() const;

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem)
	{
		try
		{
			const AuMemStory2& c = dynamic_cast<const AuMemStory2&>(mem);

			if(m_ScriptId != c.m_ScriptId)
				return false;

			m_LastStarted = c.m_LastStarted;			// Remember its last started time, which is its create time.
			m_Result = 0;								// Forget the result from the last run.
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void RememberScriptResult(quint8 result)
	{
		m_Result = result;
	}

	virtual quint32 ScriptId() const
	{
		return m_ScriptId;
	}

	virtual AuTime ScriptLastStarted() const
	{
		return m_LastStarted;
	}

	virtual quint8 ScriptResult() const
	{
		return m_Result;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMemStory : public AuMemory
{
private:
	AuTime								m_LastStarted;			// Last time the script was started.
	AuTime								m_LastTerminated;		// Last time the script was terminated. May have been dropped.
	quint8								m_Result;				// Result of running this story script.
	quint32								m_ScriptId;				// Story script.

public:	// Construction/destruction
	AuMemStory(quint32 scriptId);
	AuMemStory(quint32 scriptId, const AuTime& whenStarted, quint8 result);
	AuMemStory();
	AuMemStory(const AuMemStory& c);
	AuMemStory(const AuMemStory1& c);
	AuMemStory(const AuMemStory2& c);

public:	// Operators
	const AuMemStory& operator=(const AuMemStory& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuMemory* Clone() const
	{
		return new AuMemStory(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	virtual bool IsReplacement(const AuMemory& mem) const
	{
		Q_UNUSED(mem);
		return false;	// 'mem' does not replace this memory.
	}

	virtual bool IsStory(quint32 scriptId) const
	{
		return m_ScriptId == scriptId;
	}

	static quint8 MemoryType()
	{
		return 6;
	}

	virtual quint64 MemoryUsage() const;

	virtual void RememberScriptResult(quint8 result)
	{
		m_Result = result;
	}

	virtual void RememberScriptTerminated()
	{
		m_LastTerminated.Set();
	}

	virtual quint32 ScriptId() const
	{
		return m_ScriptId;
	}

	virtual AuTime ScriptLastStarted() const
	{
		return m_LastStarted;
	}

	virtual quint8 ScriptResult() const
	{
		return m_Result;
	}

	virtual AuTime ScriptLastTerminated() const
	{
		return m_LastTerminated;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	virtual bool RefreshMemory(const AuMemory& mem)
	{
		try
		{
			const AuMemStory& c = dynamic_cast<const AuMemStory&>(mem);

			if(m_ScriptId != c.m_ScriptId)
				return false;

			m_LastStarted = c.m_LastStarted;			// Remember its last started time, which is its create time.
			//m_LastTerminated = c.m_LastTerminated;	// Do not forget the last termination time.
			m_Result = 0;								// Forget the result from the last run.
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMemCharacterDNA4 : public AuMemory
{
private:
	quint8								m_Chromatid;
	AuDNABase*							m_DNA;
	quint32								m_EmpireId;
	AuGear*								m_Gear;
	AuHairStyle::STYLE					m_Hair;
	HANDEDNESS							m_Hand;
	QString								m_Name;
	quint32								m_ScriptId;				// Script they were in.
	AuHairStyle::SHAVE					m_Shave;
	GROWTHSTAGE							m_Stage;

	// Home World
	GALAXY								m_Galaxy;
	AuId								m_SectorId;				// Home sector of this Npc.
	QString								m_SectorName;			// Home sector of this Npc.
	AuId								m_SystemId;				// Home solar system of this Npc.
	QString								m_SystemName;			// Home solar system of this Npc.
	quint8								m_WorldBodyType;		// BODY_TYPE of homeworld
	AuId								m_WorldId;				// Home planet of this Npc. Null for pirates.
	QString								m_WorldName;			// Home planet of this Npc.

public:	// Construction/destruction
	// 'gear' can be 0
	AuMemCharacterDNA4(const QString& name, const AuDNABase& dna, quint8 chromatid, quint32 scriptId, AuHairStyle::STYLE hair,
			AuHairStyle::SHAVE shave, HANDEDNESS hand, GROWTHSTAGE stage, quint32 empireId, const AuGear* gear,
			GALAXY homeGalaxy, const AuId& homeSectorId, const QString& homeSectorName, const AuId& homeSystemId,
			const QString& homeSystemName, const AuId& homeWorldId, const QString& homeWorldName, BODY_TYPE homeWorldBody);
	// 'homeWorld' can be 0.
	AuMemCharacterDNA4(const QString& name, const AuDNABase& dna, quint8 chromatid, quint32 scriptId, AuHairStyle::STYLE hair,
			AuHairStyle::SHAVE shave, HANDEDNESS hand, GROWTHSTAGE stage, quint32 empireId, const AuGear* gear,
			const DaWorld* homeWorld);
	AuMemCharacterDNA4();
	AuMemCharacterDNA4(const AuMemCharacterDNA4& c);
	~AuMemCharacterDNA4();

public:	// Operators
	const AuMemCharacterDNA4& operator=(const AuMemCharacterDNA4& c);

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	quint8 Chromatid() const;

	virtual AuMemory* Clone() const
	{
		return new AuMemCharacterDNA4(*this);
	}

	virtual void Debug(QDebug dbg) const;

	virtual QString Desc() const;

	inline const AuDNABase* DNA() const
	{
		return m_DNA;
	}

	inline quint32 EmpireId() const
	{
		return m_EmpireId;
	}

	inline const AuGear* Gear() const
	{
		return m_Gear;
	}

	inline GROWTHSTAGE GrowthStage() const
	{
		return m_Stage;
	}

	inline AuHairStyle::STYLE Hair() const
	{
		return m_Hair;
	}

	inline HANDEDNESS Hand() const
	{
		return m_Hand;
	}

	inline GALAXY HomeGalaxy() const
	{
		return m_Galaxy;
	}

	inline const AuId& HomeSectorId() const
	{
		return m_SectorId;
	}

	inline const QString& HomeSectorName() const
	{
		return m_SectorName;
	}

	inline const AuId& HomeSystemId() const
	{
		return m_SystemId;
	}

	inline const QString& HomeSystemName() const
	{
		return m_SystemName;
	}

	inline BODY_TYPE HomeWorldBodyType() const
	{
		return (BODY_TYPE)m_WorldBodyType;
	}

	inline const AuId& HomeWorldId() const
	{
		return m_WorldId;
	}

	inline const QString& HomeWorldName() const
	{
		return m_WorldName;
	}

	virtual bool IsReplacement(const AuMemory& mem) const
	{
		Q_UNUSED(mem);
		return false;	// 'mem' does not replace this memory.
	}

	static quint8 MemoryType()
	{
		return 7;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	// Looks for a matching memory to update.
	// Returns true if this memory is updated.
	bool RefreshMemory(const AuMemory& mem);

	virtual quint32 ScriptId() const
	{
		return m_ScriptId;
	}

	inline AuHairStyle::SHAVE Shave() const
	{
		return m_Shave;
	}

	// Returns a number that is unique to each derived memory class.
	virtual quint8 Type() const
	{
		return MemoryType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMemories : public QList<AuMemory*>
{
public:	// Constructors/Destructors
	AuMemories()
	{
	}

	~AuMemories()
	{
		Clear();
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	// Returns the time of the first memory of a city.
	// Returns AuTime() if no memory of the city exists.
	AuTime FirstMemoryOfCity(const AuId& cityId) const;

	// Removes all memories associated with 'cityId'.
	void ForgetCity(const AuId& cityId);

	// Removes all memories associated with 'scriptId'.
	void ForgetStoryStarted(quint32 scriptId);

	// Returns true if scriptId has ever been started.
	bool HasStartedStory(quint32 scriptId) const;

	// Returns last time scriptId was started.
	// Returns AuTime() if it was never started.
	AuTime LastStartedStory(quint32 scriptId) const;

	quint8 LastStoryResult(quint32 scriptId) const;

	// Returns AuTime() if it was never terminated.
	AuTime LastTerminatedStory(quint32 scriptId) const;

	quint64 MemoryUsage() const;

	bool RecallCharacter(const QString& charName, AuMemCharacterDNA4& mem) const;

	// Stores or refreshes a memory.
	void Remember(const AuMemory& mem);

	void RememberStoryResult(quint32 scriptId, quint8 result);

	void RememberStoryTerminated(quint32 scriptId);

	// Returns true if a condition is actually removed.
	inline bool RemoveAt(unsigned int index)
	{
		if(index >= (unsigned int)count())
			return false;

		AuMemory* c = at(index);
		delete c;
		removeAt(index);
		return true;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuMemories& memories)
{
	memories >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMemories& memories)
{
	memories << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuMemory_H
