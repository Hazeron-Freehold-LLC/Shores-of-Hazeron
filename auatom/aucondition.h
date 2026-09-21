//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucondition.h,v 1.58 2022/05/04 16:51:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCondition_H
#define AuCondition_H

#include "auid.h"
#include "auspecies.h"
#include "autime.h"
#include <QList>

class AuBountyOnItem;
class AuDNABase;
class DaCreature;
class DaUnit;
class DhDNA;

class AuCondition
{
public:
	enum CONDITION_TYPE
	{
		Invalid,
		Poison,
		Disease,
		Paralysis,
		ImplantDNA,						// Obsolete. Use ImplantSpecies instead.
		Acid,
		UNUSED,
		Thought,
		Fire,
		Pregnant,
		Bandage,
		Bounty,
		ImplantSpecies,
		Antitoxin,
	};

protected:
	AuTime								m_CreateTime;			// Time this condition was created.

public:
	AuCondition(const AuTime& createTime);
	AuCondition();
	AuCondition(const AuCondition& c);
	virtual ~AuCondition();

	// Conditions that return true to IsVisible should override this and provide the name of a bitmap for UI.
	virtual QString Bitmap() const
	{
		return QString();
	}

	// Returns a duplicate of this condition, allocated on the heap.
	virtual AuCondition* Clone() const = 0;

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	// The base class default is 0.
	virtual int ConsoleIconSize() const;

	static AuCondition* CreateCondition(CONDITION_TYPE conditionType);

	const AuTime& CreateTime() const
	{
		return m_CreateTime;
	}

	virtual QString Description() const = 0;

	// Called when people have sex and when they have babies.
	virtual void InflictCreature(DaCreature* creature) const
	{
		Q_UNUSED(creature);
	}

	// Used when trading conditions during sex. So the creature I gave mine to doesn't give the same ones right back to me.
	virtual void InflictCreatureIfUnique(DaCreature* creature) const
	{
		Q_UNUSED(creature);
	}

	// Bounties can be transferred to items, i.e. heads.
	// Return true if this bounty is the one that corresponds to the 'bounty' on an item. Used when the bounty is redeemed, to find
	// the corresponding condition to remove.
	virtual bool IsBounty(const AuBountyOnItem& bounty) const
	{
		Q_UNUSED(bounty);
		return false;
	}

	// True if this condition is removed if it was created after entry into an arena.
	virtual bool IsRemovedOnArenaExit(const AuTime& arenaEntered) const
	{
		return CreateTime() >= arenaEntered;
	}

	// True if this condition can be removed by a physician.
	virtual bool IsTreatable() const = 0;

	// True if this condition creates a visible effect on the afflicted creature.
	virtual bool IsVisible() const = 0;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const = 0;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	// The base class default does nothing.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	// The base class default returns 0.
	virtual int RemovalCost() const;

	// Returns the quality of the condition.
	// The base class default returns 0.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	// Tick is a const function because it is called using a clone of the actual condition.
	virtual bool Tick(DaUnit* hostUnit) const = 0;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const = 0;
};

inline QDataStream& operator<<(QDataStream& out, const AuCondition& condition)
{
	condition >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCondition& condition)
{
	condition << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConditionList : public QList<AuCondition*>
{
public:
	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	// Finds the first condition of a type found in this list.
	// Returns 0 if none found.
	inline AuCondition* Condition(quint8 conditionType) const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->CType() == conditionType)
				return *ci;
		return 0;
	}

	// Returns true if a condition of the type is contained in this list.
	// Returns false if none found.
	inline bool Contains(quint8 conditionType) const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->CType() == conditionType)
				return true;
		return false;
	}

	// Returns due date of baby of pregnancy condition, if any; otherwise returns null time.
	AuTime DueDate() const;

	// Returns id of father of baby of pregnancy condition, if any; otherwise returns null id.
	AuId FatherOfBaby() const;

	const_iterator FindHighestBounty() const;

	iterator FindHighestBounty();

	// Returns true if any conditions can be treated by a physician.
	inline bool HasTreatableConditions() const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->IsTreatable())
				return true;
		return false;
	}

	inline void InflictCreature(DaCreature* creature) const
	{
		for(const_iterator ci = cbegin(); ci != cend(); ci++)
			(*ci)->InflictCreature(creature);
	}

	// Used when trading conditions during sex. So the creature I gave mine to doesn't give the same ones right back to me.
	inline void InflictCreatureIfUnique(DaCreature* creature) const
	{
		for(const_iterator ci = cbegin(); ci != cend(); ci++)
			(*ci)->InflictCreatureIfUnique(creature);
	}

	// Returns true if any conditions are visible in the render scene.
	inline bool IsVisible() const
	{
		for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
			if((*ci)->IsVisible())
				return true;
		return false;
	}

	quint64 MemoryUsage() const;

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	// Returns true if a condition is actually removed.
	inline bool RemoveAt(unsigned int index)
	{
		if(index >= (unsigned int)count())
			return false;

		AuCondition* c = at(index);
		delete c;
		removeAt(index);
		return true;
	}

	bool RemoveBounty(const AuBountyOnItem& bounty);

	/*
	// Returns true if any condition is removed.
	inline bool Tick(DaUnit* hostUnit)
	{
		// We will do this using an index instead of an iterator because it is possible for a condition to add more conditions
		// to this list during Tick, pregnancy in particular.
		bool anyRemoved = false;
		for(int ci=0; ci<count(); ci++)
		{
			AuCondition* cond = at(ic);
			if(!cond->Tick(hostUnit))
			{
				// Time to remove the condition.
				delete cond;
				removeAt(ci);
				anyRemoved = true;
			}
		}
		return anyRemoved;
	}
	*/
};

inline QDataStream& operator<<(QDataStream& out, const AuConditionList& conditionList)
{
	conditionList >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuConditionList& conditionList)
{
	conditionList << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConPoison : public AuCondition
{
private:
	quint8							m_Strength;					// Strength of this poisoning.
	quint8							m_Quality;

public:
	AuConPoison(quint8 strength, quint8 quality, const AuTime& createTime);
	AuConPoison();
	AuConPoison(const AuConPoison& c);

	virtual QString Bitmap() const;

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Poison;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	virtual QString Description() const;

	// Duration of this condition.
	inline int DurationSeconds() const
	{
		return (m_Strength+1) * (m_Quality+1) * 60;
	}

	// Called when people have sex and when they have babies.
	virtual void InflictCreature(DaCreature* creature) const;

	// Used when trading conditions during sex. So the creature I gave mine to doesn't give the same ones right back to me.
	virtual void InflictCreatureIfUnique(DaCreature* creature) const;

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// Returns the current strength level of this poison.
	inline int Toxicity() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;
		
		// The strength starts at its peak and drops off to 0 at the end.
		if(!m_Strength)
			return 0;
		return (m_Strength * (secsDuration - secsAlive) / secsDuration + 9) / 10;
	}

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConDisease : public AuCondition
{
private:
	quint8							m_Contagion;				// Contagion level.
	quint8							m_Strength;					// Strength of this disease.
	quint8							m_Quality;

public:
	AuConDisease(quint8 strength, quint8 contagion, quint8 quality, const AuTime& createTime);
	AuConDisease();
	AuConDisease(const AuConDisease& c);

	virtual QString Bitmap() const;

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Disease;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	inline quint8 Contagion() const
	{
		return m_Contagion;
	}

	virtual QString Description() const;

	// Duration of this condition.
	inline int DurationSeconds() const
	{
		return (m_Strength+1) * (m_Quality+1) * 60;
	}

	// Called when people have sex and when they have babies.
	virtual void InflictCreature(DaCreature* creature) const;

	// Used when trading conditions during sex. So the creature I gave mine to doesn't give the same ones right back to me.
	virtual void InflictCreatureIfUnique(DaCreature* creature) const;

	inline bool IsDisease(quint8 strength, quint8 contagion, quint8 quality) const
	{
		return strength == m_Strength && contagion == m_Contagion && quality == m_Quality;
	}

	inline bool IsContagious() const
	{
		// Contagion level determines whether others in the area are affected.
		// When virulence is close to full strength, within the contagion range, it is time to start spreading.
		return Virulence() + m_Contagion > m_Strength;
	}

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	inline quint8 Strength() const
	{
		return m_Strength;
	}

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;

	// Returns the current virulence level of this disease.
	inline int Virulence() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;

		// The virulence rises to its peak in the first 25% of the disease life then drops to 0 at end.
		if(!m_Strength)
			return 0;
		int peakSecs = secsDuration/4;
		if(secsAlive <= peakSecs && peakSecs)
			return (m_Strength * secsAlive / peakSecs + 9) / 10;				// Slope up to peak strength at 25% of life,
		else
			return (m_Strength - m_Strength * (secsAlive - peakSecs) / (secsDuration - peakSecs) + 9) / 10;	// down from peak to 0.
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConParalysis : public AuCondition
{
private:
	quint8							m_Strength;					// Strength of this paralysis, in % speed reduction.
	quint8							m_Quality;

public:
	AuConParalysis(quint32 strength, quint8 quality, const AuTime& createTime);
	AuConParalysis();
	AuConParalysis(const AuConParalysis& c);

	virtual QString Bitmap() const;

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Paralysis;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	virtual QString Description() const;

	// Duration of this condition.
	inline int DurationSeconds() const
	{
		return (m_Strength+1) * (m_Quality+1) * 60;
	}

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// Returns the percentage of speed reduction induced by this paralysis.
	inline int SpeedReduction() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;
		
		// The strength starts at its peak and drops off to 0 at the end.
		if(!m_Strength)
			return 0;
		return (m_Strength * (secsDuration - secsAlive) / secsDuration + 9) / 10;
	}

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConImplantDNA : public AuCondition
{
private:
	DhDNA*								m_DNA;					// DNA of the implanted creature.

public:
	AuConImplantDNA(const DhDNA& dna, const AuTime& createTime);
	AuConImplantDNA();
	AuConImplantDNA(const AuConImplantDNA& c);
	virtual ~AuConImplantDNA();

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return ImplantDNA;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	virtual QString Description() const;

	// Returns a number from 0 to 100.
	int GrowthPercent() const;

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConImplantSpecies : public AuCondition
{
private:
	AuSpecies							m_Species;				// DNA of the implanted creature.

public:
	AuConImplantSpecies(const AuSpecies& species, const AuTime& createTime);
	AuConImplantSpecies();
	AuConImplantSpecies(const AuConImplantSpecies& c);

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return ImplantSpecies;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	virtual QString Description() const;

	// Returns a number from 0 to 100.
	int GrowthPercent() const;

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConAcid : public AuCondition
{
private:
	quint8							m_Strength;					// Strength of this acid. Also duration in seconds.
	quint8							m_Quality;

public:
	AuConAcid(quint32 strength, quint8 quality, const AuTime& createTime);
	AuConAcid();
	AuConAcid(const AuConAcid& c);

	virtual QString Bitmap() const;

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Acid;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	// Returns the current strength level of this acid.
	inline int Corrosiveness() const
	{
		int secsDuration = DurationSeconds();
		int secsAlive = m_CreateTime.ElapsedSec();
		if(secsAlive >= secsDuration)
			return 0;
		
		// The strength starts at its peak and drops off to 0 at the end.
		if(!m_Strength)
			return 0;
		return (m_Strength * (secsDuration - secsAlive) / secsDuration + 9) / 10;
	}

	virtual QString Description() const;

	// Duration of this condition.
	inline int DurationSeconds() const
	{
		return (m_Strength+1) * (m_Quality+1);
	}

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Reduces the quality severity of this condition.
	virtual void ReduceQuality(quint8 qualityReduction);

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Preserves the death message so they can read it after they recover from death.

class AuConThought : public AuCondition
{
private:
	QString								m_Message;

public:
	AuConThought(const QString& message, const AuTime& createTime);
	AuConThought();
	AuConThought(const AuConThought& c);

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Thought;
	}

	virtual QString Description() const;

	// True if this condition is removed if it was created after entry into an arena.
	virtual bool IsRemovedOnArenaExit(const AuTime& /*arenaEntered*/) const
	{
		return false;
	}

	virtual bool IsTreatable() const
	{
		return false;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	inline const QString& Message() const
	{
		return m_Message;
	}

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConFire : public AuCondition
{
public:
	AuConFire(const AuTime& createTime);
	AuConFire();
	AuConFire(const AuConFire& c);

	virtual QString Bitmap() const;

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Fire;
	}

	virtual QString Description() const;

	virtual bool IsTreatable() const
	{
		return true;
	}

	virtual bool IsVisible() const
	{
		return true;
	}

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConPregnant : public AuCondition
{
private:
	AuDNABase*							m_DNA;
	quint8								m_Chroma;				// Chromatid
	AuId								m_Father;				// Father id, only if the father is an avatar, else 0.
	quint8								m_Lineage;
	QString								m_Name;

public:
	AuConPregnant(const AuId& father, const AuDNABase& dna, quint8 chroma, LINEAGE_TYPE lineage, const AuTime& createTime);
	AuConPregnant();
	AuConPregnant(const AuConPregnant& c);
	virtual ~AuConPregnant();

	// Sets the name of the baby.
	inline const QString& BabyName() const
	{
		return m_Name;
	}

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Pregnant;
	}

	// Returns the square edge size of the icon for this condition when shown on the surgery console.
	virtual int ConsoleIconSize() const;

	inline LINEAGE_TYPE Lineage() const
	{
		return (LINEAGE_TYPE)m_Lineage;
	}

	virtual QString Description() const;

	AuTime DueDate() const;

	inline const AuId& Father() const
	{
		return m_Father;
	}

	// Returns a number from 0 to 100.
	int GrowthPercent() const;

	// True if this condition is removed if it was created after entry into an arena.
	virtual bool IsRemovedOnArenaExit(const AuTime& /*arenaEntered*/) const
	{
		return false;
	}

	virtual bool IsTreatable() const
	{
		return false;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Returns the number of hitpoints of damage caused by removing this condition surgically.
	virtual int RemovalCost() const;

	// Used by doctor.
	QString SayDescription(const DaCreature* hostCreature, bool sayName) const;

	// Sets the name of the baby.
	void SetBabyName(const QString& name)
	{
		m_Name = name;
	}

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConBandage : public AuCondition
{
private:
	quint8							m_Quality;					// For quality test when healing.

public:
	AuConBandage(quint8 quality, const AuTime& createTime);
	AuConBandage();
	AuConBandage(const AuConBandage& c);

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Bandage;
	}

	virtual int ConsoleIconSize() const;

	virtual QString Description() const;

	virtual bool IsTreatable() const
	{
		return false;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConBounty : public AuCondition
{
private:
	AuId								m_PosterId;
	QString								m_PosterName;
	QString								m_Reason;
	double								m_Reward;				// Rounded. No fractions.

public:
	AuConBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, const AuTime& createTime);
	AuConBounty();
	AuConBounty(const AuConBounty& c);

	inline double AddReward(double reward)
	{
		Q_ASSERT(reward >= 0);
		m_Reward += round(reward);
		return m_Reward;
	}

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Bounty;
	}

	virtual QString Description() const;

	virtual bool IsBounty(const AuBountyOnItem& bounty) const;

	// True if this condition is removed if it was created after entry into an arena.
	virtual bool IsRemovedOnArenaExit(const AuTime& /*arenaEntered*/) const
	{
		return false;
	}

	virtual bool IsTreatable() const
	{
		return false;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

	inline const AuId& PosterId() const
	{
		return m_PosterId;
	}

	inline const QString& PosterName() const
	{
		return m_PosterName;
	}

	inline const QString& Reason() const
	{
		return m_Reason;
	}

	inline double ReduceReward(double reward)
	{
		Q_ASSERT(reward >= 0);
		m_Reward -= round(reward);
		if(m_Reward < 0)
			m_Reward = 0;
		return m_Reward;
	}

	inline double Reward() const
	{
		return m_Reward;
	}

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConAntitoxin : public AuCondition
{
private:
	quint8							m_Quality;					// For quality test when healing.

public:
	AuConAntitoxin(quint8 quality, const AuTime& createTime);
	AuConAntitoxin();
	AuConAntitoxin(const AuConAntitoxin& c);

	virtual AuCondition* Clone() const;

	static CONDITION_TYPE ConditionType()
	{
		return Antitoxin;
	}

	virtual QString Description() const;

	// Duration of this condition.
	inline int DurationHours() const
	{
		// At max quality, duration is 255, which is almost one game year at 312 hours.
		return m_Quality;
	}

	inline quint8 EffectiveQuality() const
	{
		int hoursActive = m_CreateTime.ElapsedHour();
		if(hoursActive >= m_Quality)
			return 0;
		return m_Quality - hoursActive;
	}

	virtual bool IsTreatable() const
	{
		return false;
	}

	virtual bool IsVisible() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual void operator<<(QDataStream& in);
	virtual void operator>>(QDataStream& out) const;

	// Returns the quality of the condition.
	virtual quint8 Quality() const;

	// This function is called during the atom tock cycle, which is about every 10 seconds.
	// Returns false when this condition should be removed from the host.
	virtual bool Tick(DaUnit* hostUnit) const;

	// Returns a number that is unique to each derived condition class.
	virtual CONDITION_TYPE CType() const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuCondition_H
