//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auskill.h,v 1.11 2022/02/19 18:19:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSkill_H
#define AuSkill_H

#include "audata.h"
#include <QDataStream>
#include <QHash>

class AuSkill
{
public:
	// Never change the value of these SKILL enums.
	enum SKILL
	{
		Invalid,
		Command,
		Engineer,
		FireControl,
		Helm,
		Navigator,
		Power,
		Sensor,
		Shield,
		Medic,
		Transporter,
		Turret,
		VehicleDriver,					// 2D vehicles, e.g. motor bike, SUV, sailboat
		VehiclePilot,					// 3D vehicles, e.g. space fighter, airplane, submarine
		Guard,
		Handcraft,
		Architect,
		SpacecraftCapture,
		SpacecraftCombat,
		SpaceExploration,
		SpacecraftOperations,
		Combat,
		Construction,
		Manufacture,
		Trade,
		EVSuit,
		Forage,
		CityPlanner,
		BountyHunter,
		VehicleCapture,
		VehicleCombat,
		SquadLeader,
		Exobiology,
		Butcher,
		SKILL_Count						// This must not exceed 0xffff.
	};

private:	// Properties
	quint32								m_Experience;			// Experience performing the skill. (e.g. 1 per sec. is 136 years)
	SKILL								m_Skill;

public:	// Constructors
	AuSkill(SKILL skill, quint32 experience);
	AuSkill();
	virtual ~AuSkill();
	AuSkill(const AuSkill& c);

public:	// Operators
	const AuSkill& operator=(const AuSkill& c);

	bool operator==(const AuSkill& c) const;

	inline bool operator!=(const AuSkill& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	inline quint32 Experience() const
	{
		return m_Experience;
	}

	// Returns a resource bitmap name.
	inline QString IconName() const
	{
		return IconName(m_Skill);
	}

	static QString IconName(SKILL skill);

	inline void Increase(quint32 points)
	{
		m_Experience += points;
	}

	// Returns true for skills that can be acquired by officers and by avatars commanding ships.
	bool IsCommandExperience() const;

	bool IsCrewExperience() const;

	bool IsCrewStation() const;

	bool IsTroopExperience() const;

	inline QString Name() const
	{
		return Name(m_Skill);
	}

	static QString Name(SKILL skill);

	inline SKILL Skill() const
	{
		return m_Skill;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSkill& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSkill& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSkillSet
{
	QHash<AuSkill::SKILL, AuSkill>		m_Skills;

public:	// Constructors
	AuSkillSet();
	virtual ~AuSkillSet();
	AuSkillSet(const AuSkillSet& c);

public:	// Operators
	const AuSkillSet& operator=(const AuSkillSet& c);

	bool operator==(const AuSkillSet& c) const;

	inline bool operator!=(const AuSkillSet& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	AuSkill::SKILL BestCrewStationSkill() const;

	quint32 CommandExperience() const;

	inline unsigned int Count() const
	{
		return m_Skills.size();
	}

	quint32 CrewExperience() const;

	inline quint32 Experience(AuSkill::SKILL skill) const
	{
		QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.find(skill);
		if(si == m_Skills.cend())
			return 0;
		return (*si).Experience();
	}

	inline void IncreaseSkill(AuSkill::SKILL skill, quint32 points)
	{
		QHash<AuSkill::SKILL, AuSkill>::iterator si = m_Skills.find(skill);
		if(si == m_Skills.end())
			m_Skills.insert(skill, AuSkill(skill, points));
		else
			(*si).Increase(points);
	}

	inline const QHash<AuSkill::SKILL, AuSkill>& Skills() const
	{
		return m_Skills;
	}

	quint32 TotalExperience() const;

	quint32 TroopExperience() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuSkillSet& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSkillSet& o)
{
	o << in;
	return in;
}

#endif // AuSkill_H
