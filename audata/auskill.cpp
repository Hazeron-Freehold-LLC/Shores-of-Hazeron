//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auskill.cpp,v 1.15 2022/02/19 18:20:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auskill.h"
//

AuSkill::AuSkill(SKILL skill, quint32 experience)
{
	m_Experience = experience;
	m_Skill = skill;
}

AuSkill::AuSkill()
{
	m_Skill = Invalid;
}

AuSkill::~AuSkill()
{
}

AuSkill::AuSkill(const AuSkill& c)
{
	m_Experience = c.m_Experience;
	m_Skill = c.m_Skill;
}

const AuSkill& AuSkill::operator=(const AuSkill& c)
{
	m_Experience = c.m_Experience;
	m_Skill = c.m_Skill;
	return *this;
}

bool AuSkill::operator==(const AuSkill& c) const
{
	if(m_Skill != c.m_Skill)
		return false;
	if(m_Experience != c.m_Experience)
		return false;
	return true;
}

void AuSkill::operator>>(QDataStream& out) const
{
	out << (quint16)m_Skill;
	out << m_Experience;
}

void AuSkill::operator<<(QDataStream& in)
{
	quint16 skill;
	in >> skill;
	m_Skill = (SKILL)skill;
	in >> m_Experience;
}

bool AuSkill::IsCrewStation() const
{
	switch(m_Skill)
	{
	case Engineer:
	case FireControl:
	case Helm:
	case Medic:
	case Navigator:
	case Power:
	case Sensor:
	case Shield:
	case Transporter:
		return true;
	case Architect:
	case BountyHunter:
	case Butcher:
	case CityPlanner:
	case Combat:
	case Command:
	case Construction:
	case EVSuit:
	case Forage:
	case Guard:
	case Handcraft:
	case Invalid:
	case Manufacture:
	case Exobiology:
	case SpacecraftCapture:
	case SpacecraftCombat:
	case SpaceExploration:
	case SpacecraftOperations:
	case SquadLeader:
	case Trade:
	case Turret:
	case VehicleCapture:
	case VehicleCombat:
	case VehicleDriver:
	case VehiclePilot:
	case SKILL_Count:
		break;
	};
	return false;
}

//static
QString AuSkill::Name(SKILL skill)
{
	if(skill < 0 || skill >= SKILL_Count)
		skill = Invalid;
	QString skillNames[SKILL_Count] =
	{
		QObject::tr("Invalid"),
		QObject::tr("Captain"),
		QObject::tr("Engineer"),
		QObject::tr("Fire Control"),
		QObject::tr("Helmsman"),
		QObject::tr("Navigator"),
		QObject::tr("Power Relay"),
		QObject::tr("Sensors"),
		QObject::tr("Shields"),
		QObject::tr("Medic"),
		QObject::tr("Transporter"),
		QObject::tr("Gunner"),									// Turret
		QObject::tr("Driver"),									// VehicleDriver
		QObject::tr("Pilot"),									// VehiclePilot
		QObject::tr("Guard"),
		QObject::tr("Handcraft"),
		QObject::tr("Architect"),
		QObject::tr("Spacecraft Capture"),
		QObject::tr("Spacecraft Combat"),
		QObject::tr("Space Exploration"),
		QObject::tr("Spacecraft Operations"),
		QObject::tr("Combat"),
		QObject::tr("Construction"),
		QObject::tr("Manufacture"),
		QObject::tr("Trade"),									// Trade
		QObject::tr("EV Suit"),									// EVSuit
		QObject::tr("Forage"),									// Forage
		QObject::tr("City Planner"),							// CityPlanner
		QObject::tr("Bounty Hunter"),							// BountyHunter
		QObject::tr("Vehicle Capture"),
		QObject::tr("Vehicle Combat"),
		QObject::tr("Squad Leader"),							// SquadLeader
		QObject::tr("Exobiology"),								// Exobiology
		QObject::tr("Butcher"),									// Butcher
	};
	return skillNames[skill];
}

//static
QString AuSkill::IconName(SKILL skill)
{
	if(skill < 0 || skill >= SKILL_Count)
		skill = Invalid;
	QString skillNames[SKILL_Count] =
	{
		QObject::tr(""),
		QObject::tr(":data/Captain.png"),
		QObject::tr(":data/dp_EngineerConsole.png"),
		QObject::tr(":data/dp_FireControlConsole.png"),
		QObject::tr(":data/dp_PilotConsole.png"),
		QObject::tr(":data/dp_NavConsole.png"),
		QObject::tr(":data/dp_PowerConsole.png"),
		QObject::tr(":data/dp_SensorConsole.png"),
		QObject::tr(":data/dp_ShieldConsole.png"),
		QObject::tr(":data/dp_SurgeryConsole.png"),
		QObject::tr(":data/dp_TransporterConsole.png"),
		QObject::tr(":data/dp_TurretTopBottom.png"),
		QObject::tr(":data/c_APC.png"),
		QObject::tr(":data/c_SpaceFighter.png"),
		QObject::tr(":data/ScTroop.png"),						// Guard
		QObject::tr(":data/it_HandCraft.png"),					// Handcraft
		QObject::tr(":data/bDesignStudio.png"),					// Architect
		QObject::tr(":data/DocStarshipCaptured.png"),			// SpacecraftCapture
		QObject::tr(":data/clEShip.png"),						// SpacecraftCombat
		QObject::tr(":data/SectorWormholes.png"),
		QObject::tr(":data/GovSpacecraft.png"),
		QObject::tr(":data/DocWar.png"),
		QObject::tr(":data/ConstructionSign.png"),
		QObject::tr(":data/HammerAndSickle.png"),
		QObject::tr(":data/CommTrade.png"),						// Trade
		QObject::tr(":data/c_VacSuitHelmet.png"),				// EVSuit
		QObject::tr(":data/Hand.png"),							// Forage
		QObject::tr(":data/City.png"),							// CityPlanner
		QObject::tr(":data/JobBountyHunter.png"),				// BountyHunter
		QObject::tr(":data/DocSpaceshipCaptured.png"),			// VehicleCapture
		QObject::tr(":data/clEVeh.png"),						// VehicleCombat
		QObject::tr(":data/CommMilitary.png"),					// SquadLeader
		QObject::tr(":data/DNA.png"),							// Exobiology
		QObject::tr(":data/c_Knife.png"),						// Butcher
	};
	return skillNames[skill];
}

bool AuSkill::IsCrewExperience() const
{
	switch(m_Skill)
	{
	case Combat:
	case Engineer:
	case EVSuit:
	case FireControl:
	case Helm:
	case Medic:
	case Navigator:
	case Power:
	case Sensor:
	case Shield:
	case SpacecraftCapture:
	case SpacecraftCombat:
	case SpaceExploration:
	case SpacecraftOperations:
	case Transporter:
	case VehicleCapture:
	case VehicleCombat:
	case VehicleDriver:
	case VehiclePilot:
		return true;
	case Architect:
	case BountyHunter:
	case Butcher:
	case CityPlanner:
	case Command:
	case Construction:
	case Forage:
	case Guard:
	case Handcraft:
	case Invalid:
	case Manufacture:
	case Exobiology:
	case SquadLeader:
	case Trade:
	case Turret:
	case SKILL_Count:
		break;
	};
	return false;
}

bool AuSkill::IsCommandExperience() const
{
	switch(m_Skill)
	{
	case Combat:
	case Command:
	case Engineer:
	case EVSuit:
	case FireControl:
	case Helm:
	case Medic:
	case Navigator:
	case Power:
	case Sensor:
	case Shield:
	case SpacecraftCapture:
	case SpacecraftCombat:
	case SpaceExploration:
	case SpacecraftOperations:
	case SquadLeader:
	case Trade:
	case Transporter:
	case Turret:
	case VehicleCapture:
	case VehicleCombat:
	case VehicleDriver:
	case VehiclePilot:
		return true;
	case Architect:
	case BountyHunter:
	case Butcher:
	case CityPlanner:
	case Construction:
	case Forage:
	case Guard:
	case Handcraft:
	case Invalid:
	case Manufacture:
	case Exobiology:
	case SKILL_Count:
		break;
	};
	return false;
}

bool AuSkill::IsTroopExperience() const
{
	switch(m_Skill)
	{
	case Combat:
	case EVSuit:
	case Guard:
	case SpacecraftCapture:
	case SpacecraftCombat:
	case SpaceExploration:
	case SpacecraftOperations:
	case SquadLeader:
	case Turret:
	case VehicleCapture:
	case VehicleCombat:
	case VehicleDriver:
	case VehiclePilot:
		return true;
	case Architect:
	case BountyHunter:
	case Butcher:
	case CityPlanner:
	case Command:
	case Construction:
	case Engineer:
	case FireControl:
	case Forage:
	case Handcraft:
	case Helm:
	case Invalid:
	case Manufacture:
	case Medic:
	case Navigator:
	case Power:
	case Exobiology:
	case Sensor:
	case Shield:
	case Trade:
	case Transporter:
	case SKILL_Count:
		break;
	};
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSkillSet::AuSkillSet()
{
}

AuSkillSet::~AuSkillSet()
{
}

AuSkillSet::AuSkillSet(const AuSkillSet& c) : m_Skills(c.m_Skills)
{
}

const AuSkillSet& AuSkillSet::operator=(const AuSkillSet& c)
{
	m_Skills = c.m_Skills;
	return *this;
}

bool AuSkillSet::operator==(const AuSkillSet& c) const
{
	if(m_Skills.size() != c.m_Skills.size())
		return false;
	QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin();
	QHash<AuSkill::SKILL, AuSkill>::const_iterator csi = c.m_Skills.cbegin();
	while(si != m_Skills.cend() && csi != c.m_Skills.cend())
	{
		if(*si != *csi)
			return false;
		si++;
		csi++;
	}
	return true;
}

void AuSkillSet::operator>>(QDataStream& out) const
{
	quint8 ver = 0;
	out << ver;
	out << (quint16)m_Skills.size();
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
		out << *si;
}

void AuSkillSet::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;

	m_Skills.clear();
	quint16 skills;
	in >> skills;
	while(skills--)
	{
		AuSkill skill;
		in >> skill;
		m_Skills.insert(skill.Skill(), skill);
	}
}

AuSkill::SKILL AuSkillSet::BestCrewStationSkill() const
{
	AuSkill::SKILL bestSkill = AuSkill::Invalid;
	quint32 bestXp = 0;
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
	{
		if(!(*si).IsCrewStation())
			continue;
		if((*si).Experience() > bestXp || bestSkill == AuSkill::Invalid)
			bestSkill = (*si).Skill();
	}
	return bestSkill;
}

quint32 AuSkillSet::CrewExperience() const
{
	quint32 xp = 0;
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
		if((*si).IsCrewExperience())
			xp += (*si).Experience();
	return xp;
}

quint32 AuSkillSet::CommandExperience() const
{
	quint32 xp = 0;
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
		if((*si).IsCommandExperience())
			xp += (*si).Experience();
	return xp;
}

quint32 AuSkillSet::TroopExperience() const
{
	quint32 xp = 0;
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
		if((*si).IsTroopExperience())
			xp += (*si).Experience();
	return xp;
}

quint32 AuSkillSet::TotalExperience() const
{
	quint32 xp = 0;
	for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = m_Skills.cbegin(); si != m_Skills.cend(); si++)
		xp += (*si).Experience();
	return xp;
}

