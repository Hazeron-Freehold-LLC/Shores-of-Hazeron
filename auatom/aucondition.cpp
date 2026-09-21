//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucondition.cpp,v 1.140 2022/05/04 16:51:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucondition.h"
//
#include "aucombat.h"
#include "aucommpacket.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "davehicle.h"
#include "daworld.h"
#include "daworldll.h"
#include "dgdata.h"
#include "dhdna.h"
#include <QDebug>

AuCondition::AuCondition(const AuTime& createTime)
{
	m_CreateTime = createTime;
}

AuCondition::AuCondition()
{
}

AuCondition::~AuCondition()
{
}

AuCondition::AuCondition(const AuCondition& c)
{
	m_CreateTime = c.m_CreateTime;
}

void AuCondition::operator<<(QDataStream& in)
{
	in >> m_CreateTime;
}

void AuCondition::operator>>(QDataStream& out) const
{
	out << m_CreateTime;
}

AuCondition* AuCondition::CreateCondition(CONDITION_TYPE conditionType)
{
	switch(conditionType)
	{
	case Acid:
		return new AuConAcid;
	case Antitoxin:
		return new AuConAntitoxin;
	case Bandage:
		return new AuConBandage;
	case Bounty:
		return new AuConBounty;
	case Disease:
		return new AuConDisease;
	case Fire:
		return new AuConFire;
	case ImplantDNA:
		return new AuConImplantDNA;
	case ImplantSpecies:
		return new AuConImplantSpecies;
	case Paralysis:
		return new AuConParalysis;
	case Poison:
		return new AuConPoison;
	case Pregnant:
		return new AuConPregnant;
	case Thought:
		return new AuConThought;
	case UNUSED:
	case Invalid:
		break;
	}
	qWarning() << "error: AuCondition::CreateCondition invalid condition type" << conditionType;
	return 0;
}

int AuCondition::ConsoleIconSize() const
{
	return 0;
}

// Returns the quality of the condition.
quint8 AuCondition::Quality() const
{
	return 0;
}

void AuCondition::ReduceQuality(quint8 /*qualityReduction*/)
{
}

int AuCondition::RemovalCost() const
{
	return 0;
}

quint64 AuCondition::MemoryUsage() const
{
	quint64 mem = sizeof(AuCondition);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuConditionList::operator<<(QDataStream& in)
{
	Clear();

	quint16 conditions;
	in >> conditions;
	quint8 type;
	while(conditions--)
	{
		in >> type;
		AuCondition* condition = AuCondition::CreateCondition((AuCondition::CONDITION_TYPE)type);
		if(condition)
		{
			in >> *condition;
			append(condition);
		}
		else
			qWarning() << "AuConditionList::operator<< received invalid condition type" << type;
	}
}

void AuConditionList::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
	{
		out << (quint8)(*ci)->CType();
		out << **ci;
	}
}

quint64 AuConditionList::MemoryUsage() const
{
	quint64 mem = sizeof(AuConditionList);
	for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
		mem += (*ci)->MemoryUsage();
	return mem;
}

// If multiple bounty conditions compete for the highest, the first one in the list is returned.
AuConditionList::const_iterator AuConditionList::FindHighestBounty() const
{
	const AuConBounty* largestBounty = 0;
	AuConditionList::const_iterator largest = cend();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		const AuConBounty* conBounty = dynamic_cast<const AuConBounty*>(*ci);
		if(!conBounty)
			continue;
		// Found a bounty item.
		if(!largestBounty)
		{
			largestBounty = conBounty;
			largest = ci;
		}
		else if(largestBounty->Reward() < conBounty->Reward())
		{
			largestBounty = conBounty;
			largest = ci;
		}
	}
	return largest;
}

// If multiple bounty conditions compete for the highest, the first one in the list is returned.
AuConditionList::iterator AuConditionList::FindHighestBounty()
{
	AuConBounty* largestBounty = 0;
	AuConditionList::iterator largest = end();
	for(iterator ci = begin(); ci != end(); ci++)
	{
		AuConBounty* conBounty = dynamic_cast<AuConBounty*>(*ci);
		if(!conBounty)
			continue;
		// Found a bounty item.
		if(!largestBounty)
		{
			largestBounty = conBounty;
			largest = ci;
		}
		else if(largestBounty->Reward() < conBounty->Reward())
		{
			largestBounty = conBounty;
			largest = ci;
		}
	}
	return largest;
}

bool AuConditionList::RemoveBounty(const AuBountyOnItem& bounty)
{
	for(iterator ci = begin(); ci != end(); ci++)
	{
		if((*ci)->IsBounty(bounty))
		{
			erase(ci);
			return true;
		}
	}
	return false;
}

AuTime AuConditionList::DueDate() const
{
	const AuConPregnant* preg = dynamic_cast<const AuConPregnant*>(Condition(AuConPregnant::ConditionType()));
	if(preg)
		return preg->DueDate();
	return AuTime();
}

AuId AuConditionList::FatherOfBaby() const
{
	const AuConPregnant* preg = dynamic_cast<const AuConPregnant*>(Condition(AuConPregnant::ConditionType()));
	if(preg)
		return preg->Father();
	return AuId();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConPoison::AuConPoison(quint8 strength, quint8 quality, const AuTime& createTime) : AuCondition(createTime)
{
	Q_ASSERT(quality);
	m_Strength = strength;
	m_Quality = quality;
}

AuConPoison::AuConPoison()
{
	m_Strength = 0;
	m_Quality = 1;
}

AuConPoison::AuConPoison(const AuConPoison& c) : AuCondition(c)
{
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuCondition* AuConPoison::Clone() const
{
	return new AuConPoison(*this);
}

QString AuConPoison::Description() const
{
	return QObject::tr("Condition: %2\nToxicity: %1").arg(Toxicity()).arg(Name());
}

void AuConPoison::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConPoison::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Strength;
	out << m_Quality;
}

// Reduces the quality severity of this condition.
void AuConPoison::ReduceQuality(quint8 qualityReduction)
{
	if(m_Quality > qualityReduction)
		m_Quality -= qualityReduction;
	else
		m_Quality = 1;
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConPoison::RemovalCost() const
{
	int toxicity = Toxicity();
	if(!toxicity)
		return 0;
	return g_Au.Dice().Roll(1,toxicity,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConPoison::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Poison affects only creature units.

	// Determine if this condition has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
		return false;		// Condition has run its course so remove it from the host.

	// Calculate damage caused to the host. The damage declines over time.
	int damageInflicted = g_Au.Dice().Roll(1, Toxicity()+1, -1);
	if(damageInflicted <= 0)
		return true;		// Condition did not cause any damage this time.

	// Cause damage to the host.
	AuCombatAttacker attacker(Name());

	AuCombatWeapon weapon(WM_Internal);
	weapon.m_CID = CID_Poison;
	weapon.m_Quality = m_Quality;
	weapon.m_w1.m_Effect = WE_Poison;

	AuCombatResult result;
	result.m_Damage1 = damageInflicted;

	hostCreature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
	if(result.m_IsDestroyed || hostCreature->IsBodyDyingOrDead())
		return false;											// Condition killed the host so remove it from the host.

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConPoison::CType() const
{
	return ConditionType();
}

int AuConPoison::ConsoleIconSize() const
{
	// Icon size reflects toxicity.
	int iconSize = Toxicity() + 8;
	if(iconSize > 32)
		return 32;
	return iconSize;
}

// Returns the quality of the condition.
quint8 AuConPoison::Quality() const
{
	return m_Quality;
}

QString AuConPoison::Name() const
{
	return QObject::tr("Poison %1").arg(m_Quality);
}

// Called for each nu citizen that emerges in/from the afflicted city.
void AuConPoison::InflictCreature(DaCreature* creature) const
{
	creature->AddCondition(Clone());
}

// Used when trading conditions during sex. So the creature I gave mine to doesn't give the same ones right back to me.
void AuConPoison::InflictCreatureIfUnique(DaCreature* creature) const
{
	if(creature->HasCondition(ConditionType()))
		return;							// Creature is already poisoned. That's good enough.
	creature->AddCondition(Clone());
}

quint64 AuConPoison::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConPoison) - sizeof(AuCondition);
	return mem;
}

QString AuConPoison::Bitmap() const
{
	return QString(":data/ConPoison.png");
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConDisease::AuConDisease(quint8 strength, quint8 contagion, quint8 quality, const AuTime& createTime) :
	AuCondition(createTime)
{
	Q_ASSERT(quality);
	m_Contagion = contagion;
	m_Strength = strength;
	m_Quality = quality;
}

AuConDisease::AuConDisease()
{
	m_Contagion = 0;
	m_Strength = 0;
	m_Quality = 1;
}

AuConDisease::AuConDisease(const AuConDisease& c) : AuCondition(c)
{
	m_Contagion = c.m_Contagion;
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuCondition* AuConDisease::Clone() const
{
	return new AuConDisease(*this);
}

QString AuConDisease::Description() const
{
	return QObject::tr("Condition: %2\nContagion: %3\nVirulence: %1").arg(Virulence()).arg(Name()).arg(m_Contagion);
}

void AuConDisease::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Contagion;
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConDisease::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Contagion;
	out << m_Strength;
	out << m_Quality;
}

// Reduces the quality severity of this condition.
void AuConDisease::ReduceQuality(quint8 qualityReduction)
{
	if(m_Quality > qualityReduction)
		m_Quality -= qualityReduction;
	else
		m_Quality = 1;
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConDisease::RemovalCost() const
{
	int virulence = Virulence();
	if(!virulence)
		return 0;
	return g_Au.Dice().Roll(1,virulence,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConDisease::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Disease affects only creature units.

	// Determine if this condition has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
		return false;		// Condition has run its course so remove it from the host.

	// Don't do anything when host creature is an agent.
	if(hostCreature->AgentPowerEnabled() || hostCreature->AgentInvisible())
		return true;

	// Check to see if others in the area are affected.
	if(IsContagious() && hostCreature->AtmosphereDensity() > 0)
	{
		// Spread to others nearby.
		if(hostCreature->IsInVehicle())
		{
			// Host is inside an enclosed vehicle. Everybody inside gets sick.
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(hostCreature->Parent());
			if(vehicle)
			{
				DaAtomList subs;
				vehicle->ListNormalSubatoms(subs);
				for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
				{
					DaCreature* creature = dynamic_cast<DaCreature*>(*ai);
					if(!creature)
						continue;
					if(creature == hostCreature)
						continue;		// Host does not reinfect themself.
					if(creature->IsProtectedByEnvironmentSuit())
						continue;		// Protected by environment suit

					// Check to see if the creature already has the disease.
					//if(!creature->HasDisease(m_Strength, m_Contagion, m_Quality))
					if(!creature->HasDisease())
					{
						// The creature gets infected with this disease.
						AuConDisease* disease = new AuConDisease(m_Strength, m_Contagion, m_Quality, AuTime::Now());
						creature->AddCondition(disease);
						creature->SetModified();
					}
				}
			}
		}
		else if(hostCreature->IsInsideCapsule())
		{
			// Host is inside a capsule. Everybody in the same room gets sick.
			DaCapsule* capsule = hostCreature->FindParentCapsuleInEnv();
			if(capsule)
			{
				DaAtomList subs;
				capsule->ListNormalSubatoms(subs);
				for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
				{
					DaCreature* creature = dynamic_cast<DaCreature*>(*ai);
					if(!creature)
						continue;
					if(creature == hostCreature)
						continue;		// Host does not reinfect themself.
					if(creature->IsProtectedByEnvironmentSuit())
						continue;		// Protected by environment suit
					if(creature->CapsuleRoomId() != hostCreature->CapsuleRoomId())
						continue;		// Different room.

					// Check to see if the creature already has the disease.
					//if(!creature->HasDisease(m_Strength, m_Contagion, m_Quality))
					if(!creature->HasDisease())
					{
						// The creature gets infected with this disease.
						AuConDisease* disease = new AuConDisease(m_Strength, m_Contagion, m_Quality, AuTime::Now());
						creature->AddCondition(disease);
						creature->SetModified();
					}
				}
			}
		}
		else if(hostCreature->IsInAtmosphere())
		{
			// Host is in an atmosphere. Everybody within 20' gets sick.
			DaWorld* ter = dynamic_cast<DaWorld*>(hostCreature->Parent());
			if(ter)
			{
				DaAtomList subs;
				ter->ListNormalSubatoms(subs);
				for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
				{
					DaCreature* creature = dynamic_cast<DaCreature*>(*ai);
					if(!creature)
						continue;
					if(creature == hostCreature)
						continue;		// Host does not reinfect themself.
					if(creature->IsProtectedByEnvironmentSuit())
						continue;		// Protected by environment suit
					if(creature->Location().Distance(hostCreature->Location()) > 15)
						continue;		// Too far away.

					// Check to see if the creature already has the disease.
					//if(!creature->HasDisease(m_Strength, m_Contagion, m_Quality))
					if(!creature->HasDisease())
					{
						// The creature gets infected with this disease.
						AuConDisease* disease = new AuConDisease(m_Strength, m_Contagion, m_Quality, AuTime::Now());
						creature->AddCondition(disease);
						creature->SetModified();
					}
				}
			}
		}
	}

	// Virulence rises to the max in the first 25% of the disease life then drops to 0 at end.
	int virulence = Virulence();
	if(virulence <= 0)
		return true;											// The disease is not dangerous yet.

	// Calculate damage caused to the host based on virulence.
	int damageInflicted;
	if(virulence == 1)
	{
		// Virulence is 1.
		damageInflicted = g_Au.Dice().Roll(1, 2, -1);			// Result can be 0. e.g. vir1 = 1D2-1 = [0..1] damage
		if(damageInflicted <= 0)
			return true;										// Condition did not cause any damage this time.
	}
	else if(virulence&1)
	{
		// Virulence is an odd number greater than 1.
		// Roll 2 D (1/2 virulence +1) - 1 to get a bell curve distribution in the range [1..virulence].
		// e.g. Virulence  3 = 2D2-1 = [1..3] damage
		// e.g. Virulence 11 = 2D6-1 = [1..11] damage
		virulence = virulence / 2 + 1;
		damageInflicted = g_Au.Dice().Roll(2, virulence, -1);	// Result can be 0. e.g. vir10 = 2D6-2 = [0..10] damage
	}
	else
	{
		// Virulence is an even number.
		// Roll 2 D (1/2 virulence +1) - 2 to get a bell curve distribution in the range [0..virulence].
		// e.g. Virulence  2 = 2D2-2 = [0..2] damage
		// e.g. Virulence 10 = 2D6-2 = [0..10] damage
		virulence = virulence / 2 + 1;
		damageInflicted = g_Au.Dice().Roll(2, virulence, -2);	// Result can be 0. e.g. vir10 = 2D6-2 = [0..10] damage
		if(damageInflicted <= 0)
			return true;										// Condition did not cause any damage this time.
	}

	// Cause damage to the host.
	AuCombatAttacker attacker(Name());

	AuCombatWeapon weapon(WM_Internal);
	weapon.m_CID = CID_Disease;
	weapon.m_Quality = m_Quality;
	weapon.m_w1.m_Effect = WE_Disease;

	AuCombatResult result;
	result.m_Damage1 = damageInflicted;

	hostCreature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
	if(result.m_IsDestroyed || hostCreature->IsBodyDyingOrDead())
		return false;											// Condition killed the host so remove it from the host.

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConDisease::CType() const
{
	return ConditionType();
}

int AuConDisease::ConsoleIconSize() const
{
	// Icon size reflects virulence.
	int iconSize = Virulence() + 8;
	if(iconSize > 32)
		return 32;
	return iconSize;
}

// Returns the quality of the condition.
quint8 AuConDisease::Quality() const
{
	return m_Quality;
}

QString AuConDisease::Name() const
{
	return QObject::tr("Disease %1").arg(m_Quality);
}

// Called for each nu citizen that emerges in/from the afflicted city.
void AuConDisease::InflictCreature(DaCreature* creature) const
{
	creature->AddCondition(Clone());
}

// Called for each nu citizen that emerges in/from the afflicted city.
void AuConDisease::InflictCreatureIfUnique(DaCreature* creature) const
{
	if(creature->HasCondition(ConditionType()))
		return;							// Creature is already diseased. That's good enough.
	creature->AddCondition(Clone());
}

quint64 AuConDisease::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConDisease) - sizeof(AuCondition);
	return mem;
}

QString AuConDisease::Bitmap() const
{
	return QString(":data/ConDisease.png");
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConParalysis::AuConParalysis(quint32 strength, quint8 quality, const AuTime& createTime) : AuCondition(createTime)
{
	Q_ASSERT(quality);
	if(strength < 100)
		m_Strength = strength;
	else
		m_Strength = 100;
	m_Quality = quality;
}

AuConParalysis::AuConParalysis()
{
	m_Strength = 0;
	m_Quality = 1;
}

AuConParalysis::AuConParalysis(const AuConParalysis& c) : AuCondition(c)
{
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuCondition* AuConParalysis::Clone() const
{
	return new AuConParalysis(*this);
}

QString AuConParalysis::Description() const
{
	return QObject::tr("Condition: %2\nMobility: -%1%").arg(SpeedReduction()).arg(Name());
}

void AuConParalysis::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConParalysis::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Strength;
	out << m_Quality;
}

// Reduces the quality severity of this condition.
void AuConParalysis::ReduceQuality(quint8 qualityReduction)
{
	if(m_Quality > qualityReduction)
		m_Quality -= qualityReduction;
	else
		m_Quality = 1;
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConParalysis::RemovalCost() const
{
	int mobility = SpeedReduction();
	return g_Au.Dice().Roll(1,mobility,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConParalysis::Tick(DaUnit* /*hostUnit*/) const
{
	// Determine if this condition has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	return (secsAlive < DurationSeconds());				// Returns false when the condition has run its course.
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConParalysis::CType() const
{
	return ConditionType();
}

int AuConParalysis::ConsoleIconSize() const
{
	// Icon size reflects speed reduction.
	int iconSize = SpeedReduction() + 8;
	if(iconSize > 32)
		return 32;
	return iconSize;
}

// Returns the quality of the condition.
quint8 AuConParalysis::Quality() const
{
	return m_Quality;
}

QString AuConParalysis::Name() const
{
	return QObject::tr("Paralysis %1").arg(m_Quality);
}

quint64 AuConParalysis::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConParalysis) - sizeof(AuCondition);
	return mem;
}

QString AuConParalysis::Bitmap() const
{
	return QString(":data/ConParalysis.png");
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConImplantDNA::AuConImplantDNA(const DhDNA& dna, const AuTime& createTime) : AuCondition(createTime)
{
	m_DNA = new DhDNA(dna);
}

AuConImplantDNA::AuConImplantDNA()
{
	m_DNA = new DhDNA;
}

AuConImplantDNA::~AuConImplantDNA()
{
	delete m_DNA;
}

AuConImplantDNA::AuConImplantDNA(const AuConImplantDNA& c) : AuCondition(c)
{
	m_DNA = new DhDNA(*c.m_DNA);
}

AuCondition* AuConImplantDNA::Clone() const
{
	return new AuConImplantDNA(*this);
}

QString AuConImplantDNA::Description() const
{
	int growthPercent = GrowthPercent();
	if(growthPercent >= 75)
		return QObject::tr("Condition: %1\nStage: Emergent").arg(Name());
	else if(growthPercent >= 25)
		return QObject::tr("Condition: %1\nStage: Growth").arg(Name());
	else
		return QObject::tr("Condition: %1\nStage: Embryonic").arg(Name());
}

void AuConImplantDNA::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> *m_DNA;
}

void AuConImplantDNA::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << *m_DNA;
}

// Reduces the quality severity of this condition.
void AuConImplantDNA::ReduceQuality(quint8 /*qualityReduction*/)
{
	// na
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConImplantDNA::RemovalCost() const
{
	int maxRemovalCost = m_DNA->GestateImplantHours() * GrowthPercent() / 100;
	return g_Au.Dice().Roll(1,maxRemovalCost,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConImplantDNA::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Implant affects only creature units.
	if(hostCreature->IsAboardVehicle())
		return true;											// Growing embryos are NEVER allowed aboard vehicles. Just wait...
	AuEther* ether = hostCreature->Ether();
	if(!ether)
		return true;

	// Determine if the implant is ready to be born.
	// Gestation time is based on the overall size factor in the implant's DNA.
	int hrsAlive = m_CreateTime.ElapsedSec()/(60*60);
	int gestationTime = m_DNA->GestateImplantHours();
	if(hrsAlive >= gestationTime)
	{
		// Don't put the nu creature subordinate to another creature.
		DaAtom* nuParentAtom = hostCreature->Parent();
		while(dynamic_cast<DaCreature*>(nuParentAtom))
			nuParentAtom = nuParentAtom->Parent();
		AuMatrix4x4 nuBabyM;
		hostCreature->GetTransformRelativeToParent(nuBabyM, nuParentAtom);

		// Happy birthday little implant.
		AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
		HANDEDNESS isLefty = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
		DaAnimal* animal = new DaAnimal(m_DNA->Species(), chroma, isLefty, Juvenile);
		animal->ConnectSignals();
		animal->Create();
		animal->SetPosAndOrient(AuAtomPos(nuParentAtom->Id(), nuBabyM));
		ether->AddAtom(animal);

		animal->ResetHitPoints();								// Creature is born healthy.
		animal->ResetStamina();									// Creature is born with full stamina.
		animal->SetHunger(0);									// Creature is born hungry.
		animal->Stand();										// Creature is born standing.
		int maxHp = animal->MaxHitPoints();						// Unmodified by growth time.
		animal->InitTimeToFullGrown();							// Init time for baby to grow to full size.
		g_Au.AtomAdded(animal->Id());
		animal->SetNormal();									// New atom is ready for normal thread processing.
		animal->TickTock();

		// Giving birth to an implant damages the host.
		int hostDamage = g_Au.Dice().Roll(1,maxHp/(gestationTime+1),1);
		if(hostDamage > 0)
		{
			// Cause damage to the host.
			AuCombatAttacker attacker(Name());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_ImplantBirth;
			weapon.m_w1.m_Effect = WE_Implant;

			AuCombatResult result;
			result.m_Damage1 = hostDamage;

			hostCreature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}

		return false;				// This implant is removed from the host.
	}

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConImplantDNA::CType() const
{
	return ConditionType();
}

int AuConImplantDNA::ConsoleIconSize() const
{
	// Icon grows larger over time.
	return GrowthPercent() * m_DNA->GestateImplantHours() / 100 + 8;
}

QString AuConImplantDNA::Name() const
{
	return QObject::tr("%1 Parasite").arg(m_DNA->Species().NameOrDesc());
}

quint64 AuConImplantDNA::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConImplantDNA) - sizeof(AuCondition);
	return mem;
}

int AuConImplantDNA::GrowthPercent() const
{
	int secsAlive = m_CreateTime.ElapsedSec();
	int secsGestation = m_DNA->GestateImplantHours() * (60*60);
	if(secsAlive >= secsGestation)
		return 100;
	return 100 - 100 * (secsGestation - secsAlive) / secsGestation;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConImplantSpecies::AuConImplantSpecies(const AuSpecies& species, const AuTime& createTime) : AuCondition(createTime),
	m_Species(species)
{
}

AuConImplantSpecies::AuConImplantSpecies()
{
}

AuConImplantSpecies::AuConImplantSpecies(const AuConImplantSpecies& c) : AuCondition(c), m_Species(c.m_Species)
{
}

AuCondition* AuConImplantSpecies::Clone() const
{
	return new AuConImplantSpecies(*this);
}

QString AuConImplantSpecies::Description() const
{
	int growthPercent = GrowthPercent();
	if(growthPercent >= 75)
		return QObject::tr("Condition: %1\nStage: Emergent").arg(Name());
	else if(growthPercent >= 25)
		return QObject::tr("Condition: %1\nStage: Growth").arg(Name());
	else
		return QObject::tr("Condition: %1\nStage: Embryonic").arg(Name());
}

void AuConImplantSpecies::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Species;
}

void AuConImplantSpecies::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Species;
}

// Reduces the quality severity of this condition.
void AuConImplantSpecies::ReduceQuality(quint8 /*qualityReduction*/)
{
	// na
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConImplantSpecies::RemovalCost() const
{
	AuDNABase* dna = m_Species.CreateDNA();
	if(!dna)
		return 0;
	int maxRemovalCost = dna->GestateImplantHours() * GrowthPercent() / 100;
	delete dna;
	return g_Au.Dice().Roll(1,maxRemovalCost,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConImplantSpecies::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Implant affects only creature units.
	if(hostCreature->IsAboardVehicle())
		return true;											// Growing embryos are NEVER allowed aboard vehicles. Just wait...
	AuEther* ether = hostCreature->Ether();
	if(!ether)
		return true;

	AuDNABase* dna = m_Species.CreateDNA();
	if(!dna)
		return false;
	int gestationTime = dna->GestateImplantHours();
	delete dna;

	// Determine if the implant is ready to be born.
	// Gestation time is based on the overall size factor in the implant's DNA.
	int hrsAlive = m_CreateTime.ElapsedSec()/(60*60);
	if(hrsAlive >= gestationTime)
	{
		// Don't put the nu creature subordinate to another creature.
		DaAtom* nuParentAtom = hostCreature->Parent();
		while(dynamic_cast<DaCreature*>(nuParentAtom))
			nuParentAtom = nuParentAtom->Parent();
		AuMatrix4x4 nuBabyM;
		hostCreature->GetTransformRelativeToParent(nuBabyM, nuParentAtom);

		// Happy birthday little implant.
		AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
		HANDEDNESS isLefty = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
		DaAnimal* animal = new DaAnimal(m_Species, chroma, isLefty, Juvenile);
		animal->ConnectSignals();
		animal->Create();
		animal->SetPosAndOrient(AuAtomPos(nuParentAtom->Id(), nuBabyM));
		ether->AddAtom(animal);

		animal->ResetHitPoints();								// Creature is born healthy.
		animal->ResetStamina();									// Creature is born with full stamina.
		animal->SetHunger(0);									// Creature is born hungry.
		animal->Stand();										// Creature is born standing.
		int maxHp = animal->MaxHitPoints();						// Unmodified by growth time.
		animal->InitTimeToFullGrown();							// Init time for baby to grow to full size.
		g_Au.AtomAdded(animal->Id());
		animal->SetNormal();									// New atom is ready for normal thread processing.
		animal->TickTock();

		// Giving birth to an implant damages the host.
		int hostDamage = g_Au.Dice().Roll(1,maxHp/(gestationTime+1),1);
		if(hostDamage > 0)
		{
			// Cause damage to the host.
			AuCombatAttacker attacker(Name());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_ImplantBirth;
			weapon.m_w1.m_Effect = WE_Implant;

			AuCombatResult result;
			result.m_Damage1 = hostDamage;

			hostCreature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}

		return false;				// This implant is removed from the host.
	}

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConImplantSpecies::CType() const
{
	return ConditionType();
}

int AuConImplantSpecies::ConsoleIconSize() const
{
	// Icon grows larger over time.
	AuDNABase* dna = m_Species.CreateDNA();
	if(!dna)
		return 1;
	int gestateImplantHours = dna->GestateImplantHours();
	delete dna;
	return GrowthPercent() * gestateImplantHours / 100 + 8;
}

QString AuConImplantSpecies::Name() const
{
	return QObject::tr("%1 Parasite").arg(m_Species.NameOrDesc());
}

quint64 AuConImplantSpecies::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConImplantSpecies) - sizeof(AuCondition);
	return mem;
}

int AuConImplantSpecies::GrowthPercent() const
{
	AuDNABase* dna = m_Species.CreateDNA();
	if(!dna)
		return 100;
	int secsAlive = m_CreateTime.ElapsedSec();
	int secsGestation = dna->GestateImplantHours() * (60*60);
	delete dna;
	if(secsAlive >= secsGestation)
		return 100;
	return 100 - 100 * (secsGestation - secsAlive) / secsGestation;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConAcid::AuConAcid(quint32 strength, quint8 quality, const AuTime& createTime) : AuCondition(createTime)
{
	Q_ASSERT(quality);
	if(strength < 255)
		m_Strength = strength;
	else
		m_Strength = 255;
	m_Quality = quality;
}

AuConAcid::AuConAcid()
{
	m_Strength = 0;
	m_Quality = 1;
}

AuConAcid::AuConAcid(const AuConAcid& c) : AuCondition(c)
{
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuCondition* AuConAcid::Clone() const
{
	return new AuConAcid(*this);
}

QString AuConAcid::Description() const
{
	return QObject::tr("Condition: %2\nCorrosiveness: %1").arg(Corrosiveness()).arg(Name());
}

void AuConAcid::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConAcid::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Strength;
	out << m_Quality;
}

// Reduces the quality severity of this condition.
void AuConAcid::ReduceQuality(quint8 qualityReduction)
{
	if(m_Quality > qualityReduction)
		m_Quality -= qualityReduction;
	else
		m_Quality = 1;
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConAcid::RemovalCost() const
{
	int corrosiveness = Corrosiveness();
	if(!corrosiveness)
		return 0;
	return g_Au.Dice().Roll(1,corrosiveness,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConAcid::Tick(DaUnit* hostUnit) const
{
	// Determine if this condition has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
		return false;		// Condition has run its course so remove it from the host.

	// Calculate damage caused to the host. The damage declines over time.
	int damageInflicted = g_Au.Dice().Roll(1, Corrosiveness()+1, -1);
	if(damageInflicted <= 0)
		return true;		// Condition did not cause any damage this time.

	// Cause damage to the host.
	AuCombatAttacker attacker(Name());

	AuCombatWeapon weapon(WM_Internal);
	weapon.m_CID = CID_Acid;
	weapon.m_Quality = m_Quality;
	weapon.m_w1.m_Effect = WE_Acid;

	AuCombatResult result;
	result.m_Damage1 = damageInflicted;

	hostUnit->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
	if(result.m_IsDestroyed)
		return false;											// Condition killed the host so remove it from the host.

	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(hostCreature && hostCreature->IsBodyDyingOrDead())
		return false;											// Condition hurt host creature's dying body so remove it.

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConAcid::CType() const
{
	return ConditionType();
}

int AuConAcid::ConsoleIconSize() const
{
	// Icon size reflects toxicity.
	int iconSize = Corrosiveness() + 8;
	if(iconSize > 32)
		return 32;
	return iconSize;
}

// Returns the quality of the condition.
quint8 AuConAcid::Quality() const
{
	return m_Quality;
}

QString AuConAcid::Name() const
{
	return QObject::tr("Acid %1").arg(m_Quality);
}

quint64 AuConAcid::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConAcid) - sizeof(AuCondition);
	return mem;
}

QString AuConAcid::Bitmap() const
{
	return QString(":data/ConAcid.png");
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConThought::AuConThought(const QString& message, const AuTime& createTime) : AuCondition(createTime)
{
	m_Message = message;
}

AuConThought::AuConThought()
{
}

AuConThought::AuConThought(const AuConThought& c) : AuCondition(c)
{
	m_Message = c.m_Message;
}

AuCondition* AuConThought::Clone() const
{
	return new AuConThought(*this);
}

QString AuConThought::Description() const
{
	return QObject::tr("Thought: %1")
		.arg(m_Message);
}

void AuConThought::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Message;
}

void AuConThought::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Message;
}

// Returns false when this condition should be removed from the host.
bool AuConThought::Tick(DaUnit* hostUnit) const
{
	// This condition should only be found on person units.
	DaPerson* person = dynamic_cast<DaPerson*>(hostUnit);
	if(!person)
		return false;

	// Do nothing until this person returns to a "normal" state, no longer spawning or traveling.
	if(!person->IsNormal())
		return true;

	// Do nothing until this person is on line.
	if(!person->IsOnLine())
		return true;

	// Person is normal and on line. Give them the thought.
	AuCommThought msg(m_Message, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);
	return false;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConThought::CType() const
{
	return ConditionType();
}

QString AuConThought::Name() const
{
	return QObject::tr("Thought");
}

quint64 AuConThought::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConThought) - sizeof(AuCondition);
	mem += StringMemoryUsage(m_Message);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConFire::AuConFire(const AuTime& createTime) : AuCondition(createTime)
{
}

AuConFire::AuConFire()
{
}

AuConFire::AuConFire(const AuConFire& c) : AuCondition(c)
{
}

AuCondition* AuConFire::Clone() const
{
	return new AuConFire(*this);
}

QString AuConFire::Description() const
{
	return QObject::tr("Fire");
}

void AuConFire::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
}

void AuConFire::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
}

// Returns false when this condition should be removed from the host.
bool AuConFire::Tick(DaUnit* hostUnit) const
{
	// Cause damage to the host.
	AuCombatAttacker attacker(Name());

	AuCombatWeapon weapon(WM_Internal);
	weapon.m_CID = CID_Fire;
	weapon.m_w1.m_Effect = WE_Heat;

	AuCombatResult result;
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(hostCreature && hostCreature->IsBodyDying())
		result.m_Damage1 = 1;
	else
		result.m_Damage1 = g_Au.Dice().Roll(1,20);

	hostUnit->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);

	// Condition removed when host killed.
	if(result.m_IsDestroyed)
		return false;
	if(hostCreature)
	{
		if(hostCreature->IsBodyDyingOrDead())
			return false;
		// 10% chance at each cycle that the fire will be extinquished.
		if(g_Au.Dice().Roll(1,100) <= 10)
			return false;
	}
	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConFire::CType() const
{
	return ConditionType();
}

QString AuConFire::Name() const
{
	return QObject::tr("Fire");
}

QString AuConFire::Bitmap() const
{
	return QString(":data/ConFire.png");
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConPregnant::AuConPregnant(const AuId& father, const AuDNABase& dna, quint8 chroma, LINEAGE_TYPE lineage,
		const AuTime& createTime) : AuCondition(createTime)
{
	m_DNA = dna.Clone();
	m_Chroma = chroma;
	m_Father = father;
	m_Lineage = lineage;
}

AuConPregnant::AuConPregnant()
{
	m_Chroma = AuDNABase::Alpha;
	m_DNA = 0;
	m_Lineage = 0;
}

AuConPregnant::AuConPregnant(const AuConPregnant& c) : AuCondition(c)
{
	m_Chroma = c.m_Chroma;
	if(c.m_DNA)
		m_DNA = c.m_DNA->Clone();
	else
		m_DNA = 0;
	m_Father = c.m_Father;
	m_Lineage = c.m_Lineage;
	m_Name = c.m_Name;
}

AuConPregnant::~AuConPregnant()
{
	if(m_DNA)
		delete m_DNA;
}

AuCondition* AuConPregnant::Clone() const
{
	return new AuConPregnant(*this);
}

QString AuConPregnant::SayDescription(const DaCreature* hostCreature, bool sayName) const
{
	if(!m_DNA)
		return "";

	Q_ASSERT(hostCreature);
	QString gender[] =
	{
		QObject::tr("girl"),
		QObject::tr("boy"),
		QObject::tr("asexual"),
		QObject::tr("hermaphrodite"),
	};

	QString lineage[] =
	{
		QObject::tr("None"),
		QObject::tr("citizen"),
		QObject::tr("demiavatar"),
		QObject::tr("highborn"),
	};

	QString say;
	if(sayName)
		say = QObject::tr("%1, you are pregnant.").arg(hostCreature->Name());
	else
		say = QObject::tr("You are pregnant.");

	int growthPercent = GrowthPercent();
	if(growthPercent >= 99)
	{
		say += QObject::tr(" You will soon give birth to a baby %1 %2.")
			.arg(gender[m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)])
			.arg(lineage[m_Lineage]);

		if(m_Lineage == LINEAGE_Avatar)
			say += QObject::tr(" It's a dazzling event to see.");
	}
	else if(growthPercent >= 75)
		say += QObject::tr(" You are in your third trimester. The baby is a %1 %2, due at %3.")
			.arg(gender[m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)])
			.arg(lineage[m_Lineage])
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	else if(growthPercent >= 66)
		say += QObject::tr(" You are in your third trimester. The baby is a %1, due at %2.")
			.arg(gender[m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)])
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	else if(growthPercent >= 50)
		say += QObject::tr(" You are in your second trimester. The baby is a %1, due at %2.")
			.arg(gender[m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)])
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	else if(growthPercent >= 33)
		say += QObject::tr(" You are in your second trimester, due at %1.")
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	else if(growthPercent > 1)
		say += QObject::tr(" You are in your first trimester, due at %1.")
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	else
		say += QObject::tr(" You're due at %1.")
			.arg(DueDate().UTC().toLocalTime().toString("h:mm a dddd MMMM d"));
	return say;
}

QString AuConPregnant::Description() const
{
	if(!m_DNA)
		return "";

	QString gender[] =
	{
		QObject::tr("Female"),
		QObject::tr("Male"),
	};

	QString lineage[] =
	{
		QObject::tr("None"),
		QObject::tr("Citizen"),
		QObject::tr("Demiavatar"),
		QObject::tr("Highborn"),
	};

	int growthPercent = GrowthPercent();
	if(growthPercent >= 99)
		return QObject::tr("Condition: %1\nStage: Prebirth\nGender: %3\nDue: %4\nLineage: %5")
			.arg(Name())
			.arg(GenderName(m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)))
			.arg(DueDate().UTC().toLocalTime().toString())
			.arg(lineage[m_Lineage]);
	else if(growthPercent >= 75)
		return QObject::tr("Condition: %1\nStage: Third Trimester\nGender: %3\nDue: %4\nLineage: %5")
			.arg(Name())
			.arg(GenderName(m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)))
			.arg(DueDate().UTC().toLocalTime().toString())
			.arg(lineage[m_Lineage]);
	else if(growthPercent >= 66)
		return QObject::tr("Condition: %1\nStage: Third Trimester\nGender: %3\nDue: %4")
			.arg(Name())
			.arg(GenderName(m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)))
			.arg(DueDate().UTC().toLocalTime().toString());
	else if(growthPercent >= 50)
		return QObject::tr("Condition: %1\nStage: Second Trimester\nGender: %3\nDue: %4")
			.arg(Name())
			.arg(GenderName(m_DNA->Gender((AuDNABase::CHROMATID)m_Chroma)))
			.arg(DueDate().UTC().toLocalTime().toString());
	else if(growthPercent >= 33)
		return QObject::tr("Condition: %1\nStage: Second Trimester\nGender: Undetermined\nDue: %4").arg(Name())
			.arg(DueDate().UTC().toLocalTime().toString());
	else if(growthPercent > 1)
		return QObject::tr("Condition: %1\nStage: First Trimester\nGender: Undetermined\nDue: %4").arg(Name())
			.arg(DueDate().UTC().toLocalTime().toString());
	else
		return QObject::tr("Condition: %1\nStage: Zygote\nGender: Undetermined\nDue: %4").arg(Name())
			.arg(DueDate().UTC().toLocalTime().toString());
}

void AuConPregnant::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);

	quint8 ui8 = 0x80 | m_Lineage;	// Top bit is the version.
	if(m_DNA)
		ui8 |= 0x40;
	else
		ui8 |= 0x20;	// bit says m_DNA was null

	out << ui8;
	if(m_DNA)
		m_DNA->WriteDynamic(out);
	out << m_Chroma;
	out << m_Name;
	out << m_Father;
}

void AuConPregnant::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	
	quint8 ui8;
	in >> ui8;	// Top bit is the version.

	m_Lineage = ui8 & 0x03;
	if(m_DNA)
		delete m_DNA;
	if(ui8 & 0x40)
		m_DNA = AuDNABase::ReadDynamic(in);
	else if(ui8 & 0x20)
		m_DNA = 0;	// DNA was assertively 0
	else
	{
		// Old version DNA
		m_DNA = new DhDNA;
		in >> *m_DNA;
	}
	in >> m_Chroma;
	in >> m_Name;
	if(ui8 & 0x80)
		in >> m_Father;
}

// Returns the number of hitpoints of damage caused by removing this condition surgically.
int AuConPregnant::RemovalCost() const
{
	if(!m_DNA)
		return 0;
	int maxRemovalCost = m_DNA->GestateBabyDays() * GrowthPercent() / 100;
	return g_Au.Dice().Roll(1,maxRemovalCost,-1);
}

// Returns false when this condition should be removed from the host.
bool AuConPregnant::Tick(DaUnit* hostUnit) const
{
	// Offspring lost when host unit traumatized or dead.
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;
	if(hostCreature->IsAboardVehicle())
		return true;											// Growing embryos are NEVER allowed aboard vehicles. Just wait...
	if(hostCreature->IsBodyDyingOrDead())
		return false;
	if(!m_DNA)
		return false;
	AuEther* ether = hostCreature->Ether();
	if(!ether)
		return true;

	// Determine if the baby is ready to be born.
	AuTime now(AuTime::Now());
	if(now >= DueDate() || m_Father == AuId::Haxus())
	{
		// Happy birthday little baby.
		HANDEDNESS isLefty = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
		QString name;
		if(m_Name.isEmpty())
			name = g_Au.Dice().RandomPersonName();
		else
			name = m_Name;

		// Don't put the nu creature subordinate to another creature.
		DaAtom* nuParentAtom = hostCreature->Parent();
		while(dynamic_cast<DaCreature*>(nuParentAtom))
			nuParentAtom = nuParentAtom->Parent();
		AuMatrix4x4 nuBabyM;
		hostCreature->GetTransformRelativeToParent(nuBabyM, nuParentAtom);

		DaCreature* baby = 0;
		int maxHp;
		if(Lineage() != LINEAGE_Citizen)
		{
			// Give birth to an avatar or demiavatar.
			DaPerson* babyP = new DaPerson(*m_DNA, (AuDNABase::CHROMATID)m_Chroma, isLefty, name, Juvenile,
					hostCreature->EmpireId());
			babyP->ConnectSignals();
			babyP->Create();
			babyP->SetPosAndOrient(AuAtomPos(nuParentAtom->Id(), nuBabyM));
			ether->AddAtom(babyP);

			baby = babyP;
			babyP->ResetHitPoints();							// Baby is born healthy.
			babyP->ResetStamina();								// Baby is born with full stamina.
			babyP->ResetHunger();								// Baby is not born starving.
			babyP->Stand();										// Baby is born standing.
			maxHp = babyP->MaxHitPoints();						// Unmodified by growth time.
			babyP->InitTimeToFullGrown();						// Init time for baby to grow to full size.
			babyP->SetLineage(Lineage(), m_Father, hostCreature->Id());
			babyP->SetOnLine(ACCT_Active, 0, now);					// So the baby will remain visible for a bit.
			babyP->ChooseHairStyle();
			g_Au.AtomAdded(babyP->Id());
			babyP->SetNormal();									// New atom is ready for normal thread processing.
			babyP->TickTock();

			babyP->AddCitizenToEmpire(0, babyP->EmpireId(), babyP->Id(), babyP->Name(), babyP->Gender());

			DaPerson* mom = dynamic_cast<DaPerson*>(hostCreature);
			if(mom)
			{
				// Create a birth announcement and add it as a thought condition. Mom will get it later if she's off line right now.
				float mass = babyP->Mass() * KilogramsPerPound();
				QString birthAnnounce = QObject::tr("I gave birth to %1 at %2. %3 weighed %4kg.")
					.arg(babyP->Name())
					.arg(AuTime::CurrentUTC().toString())
					.arg(babyP->SheHe(false))
					.arg(mass, 0, 'f', 2);
				mom->AddCondition(new AuConThought(birthAnnounce, AuTime::Now()));

				// When the mom is an avatar, baby avatars are added to the mother's login account, if a slot is available.
				babyP->AvatarBorn(hostCreature->Id(), babyP->Id());

				// Player Statistics
				mom->Stats().ChildBorn();
			}
			else if(!m_Father.IsNull())
			{
				// When the mom is a citizen, baby avatars are added to the father's login account, if a slot is available.
				babyP->AvatarBorn(m_Father, babyP->Id());
			}

			hostCreature->SetModified();
		}
		else
		{
			const DaCitizen* hostCitizen = dynamic_cast<const DaCitizen*>(hostCreature);
			if(hostCitizen)
			{
				// Give birth to a citizen.
				const DaWorldLL* world = hostCitizen->FindParentWorldInEnv();
				NPC_ROLE role;
				if(world && hostCitizen->IsNpcIndigenous() && !hostCitizen->IsAtBd())
					role = ROLE_IndigenousPerson;
				else
					role = ROLE_Citizen;

				DaNpc* babyP = new DaNpc(role, *m_DNA, (AuDNABase::CHROMATID)m_Chroma, isLefty, name);
				babyP->ConnectSignals();
				babyP->Create();
				babyP->SetPosAndOrient(AuAtomPos(nuParentAtom->Id(), nuBabyM));
				ether->AddAtom(babyP);

				babyP->SetLineage(Lineage());
				babyP->InitTimeToFullGrown();
				if(world)
					babyP->SetHomeWorld(world);
				baby = babyP;
			}

			baby->SetBodyQ(hostCreature->BodyQ());				// Baby gets same body Q as mom.
			baby->SetEmpireId(hostCreature->EmpireId());
			baby->ResetHitPoints();								// Creature is born healthy.
			baby->ResetStamina();								// Creature is born with full stamina.
			baby->ResetHunger();
			baby->Stand();										// Creature is born standing.
			maxHp = baby->MaxHitPoints();						// Unmodified by growth time.
			baby->ChooseHairStyle();
			g_Au.AtomAdded(baby->Id());
			baby->SetNormal();									// New atom is ready for normal thread processing.
			baby->TickTock();
		}
		//qDebug() << "AuConPregnant::Tick" << baby->Id() << baby->Name() << "born!";

		// Send them on their way in life with any souvenirs the mother has to offer.
		hostCreature->LockConditions().InflictCreature(baby);
		hostCreature->UnlockConditions();

		// Say hello.
		QString text = QObject::tr("mama");
		AuCommChat msg(text, AuCommStdChannel::VoiceChannelName(), baby->Id(), baby->Name(), baby->FindParentSector()->Id(),
				baby->Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(msg);

		// Giving birth damages the host a little.
		int gestationTime = m_DNA->GestateBabyDays();
		int hostDamage = g_Au.Dice().Roll(1,maxHp/(gestationTime+1),1);
		if(hostDamage > 0)
		{
			// Cause damage to the host.
			AuCombatAttacker attacker(Name());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_GivingBirth;
			weapon.m_w1.m_Effect = WE_Implant;

			AuCombatResult result;
			result.m_Damage1 = hostDamage;

			hostCreature->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}

		return false;				// This gestating baby is removed from the host.
	}

	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConPregnant::CType() const
{
	return ConditionType();
}

int AuConPregnant::ConsoleIconSize() const
{
	if(!m_DNA)
		return 100;
	// Icon grows larger over time.
	return GrowthPercent() * m_DNA->GestateBabyDays() * 2 / 100 + 8;
}

QString AuConPregnant::Name() const
{
	if(!m_Name.isEmpty() || !m_DNA)
		return m_Name;
	return QObject::tr("Baby %1").arg(m_DNA->Species().NameOrDesc());
}

quint64 AuConPregnant::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConPregnant) - sizeof(AuCondition);
	mem += StringMemoryUsage(m_Name);
	return mem;
}

int AuConPregnant::GrowthPercent() const
{
	if(!m_DNA)
		return 100;
	int secsAlive = m_CreateTime.ElapsedSec();
	int secsGestation = m_DNA->GestateBabyDays() * (60*60*24);	// Overall size is number of days to gestate.
	if(secsAlive >= secsGestation)
		return 100;
	return 100 - 100 * (secsGestation - secsAlive) / secsGestation;
}

AuTime AuConPregnant::DueDate() const
{
	// Gestation time is based on the overall size factor in the baby's DNA.
	if(!m_DNA)
		return AuTime::Now();
	int secsGestation = m_DNA->GestateBabyDays() * (60*60*24);
	return CreateTime().AddSecs(secsGestation);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConBandage::AuConBandage(quint8 quality, const AuTime& createTime) : AuCondition(createTime)
{
	Q_ASSERT(quality);
	m_Quality = quality;
}

AuConBandage::AuConBandage()
{
	m_Quality = 1;
}

AuConBandage::AuConBandage(const AuConBandage& c) : AuCondition(c)
{
	m_Quality = c.m_Quality;
}

AuCondition* AuConBandage::Clone() const
{
	return new AuConBandage(*this);
}

QString AuConBandage::Description() const
{
	return QObject::tr("%1 Q%2")
		.arg(Name())
		.arg(m_Quality);
}

void AuConBandage::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConBandage::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Quality;
}

// Returns false when this condition should be removed from the host.
bool AuConBandage::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Bandage affects only creature units.
	if(hostCreature->IsBodyDyingOrDead())
		return true;			// Bandages do not revive dying bodies. They wait.
	quint32 maxHits = hostCreature->MaxHitPoints();
	quint32 curHits = hostCreature->CurrentHitPoints();
	if(curHits >= maxHits)
	{
		hostCreature->Thought(QObject::tr("Bandage dissolved because my body is not wounded."));
		return false;			// Bandages fall off of undamaged bodies.
	}
	if(!DbCommodity::TestQuality(m_Quality))
	{
		hostCreature->Thought(QObject::tr("Bandage wore out before my wound healed completely."));
		return false;			// Bandages fall off if they fail their quality check.
	}
	// Max healing rate is 10% of max hit points per 10 second period.
	quint32 maxHeal = maxHits / 10;
	if(!maxHeal)
		maxHeal = 1;
	// Calculate the number of hit points to heal.
	quint32 heal = maxHits - curHits;
	if(heal > maxHeal)
		heal = maxHeal;
	heal = DbCommodity::TestFailure(m_Quality, heal);	// Adjust heal points based on bandage quality.
	if(!heal)
		return true;			// Keep trying.
	hostCreature->SetHitPoints(curHits + heal);
	hostCreature->SetModified();
	bool retval = hostCreature->CurrentHitPoints() != hostCreature->MaxHitPoints();	// Fall off when creature is fully healed.
	if(!retval)
		hostCreature->Thought(QObject::tr("Bandage dissolved because my wound is fully healed."));
	else
		hostCreature->Thought(QObject::tr("Bandage healed %L1hp of my wound.").arg(heal));
	return retval;				// Bandages fall off when the body is fully healed.
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConBandage::CType() const
{
	return ConditionType();
}

int AuConBandage::ConsoleIconSize() const
{
	return 32;
}

// Returns the quality of the condition.
quint8 AuConBandage::Quality() const
{
	return m_Quality;
}

QString AuConBandage::Name() const
{
	return QObject::tr("Bandage");
}

quint64 AuConBandage::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConBandage) - sizeof(AuCondition);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConBounty::AuConBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward,
		const AuTime& createTime) : AuCondition(createTime)
{
	m_PosterId = posterId;
	m_PosterName = posterName;
	m_Reason = reason;
	m_Reward = round(reward);
}

AuConBounty::AuConBounty()
{
	m_Reward = 0;
}

AuConBounty::AuConBounty(const AuConBounty& c) : AuCondition(c)
{
	m_PosterId = c.m_PosterId;
	m_PosterName = c.m_PosterName;
	m_Reason = c.m_Reason;
	m_Reward = c.m_Reward;
}

AuCondition* AuConBounty::Clone() const
{
	return new AuConBounty(*this);
}

QString AuConBounty::Description() const
{
	if(!m_PosterId.IsNull())
		return QObject::tr("Bounty: %1\nReason: %2\nPosted By:%3 %4")
			.arg(Name())
			.arg(m_Reason)
			.arg(m_PosterName)
			.arg(m_PosterId.StringId());
	else
		return QObject::tr("Bounty: %1\nReason: %2")
			.arg(Name())
			.arg(m_Reason);
}

void AuConBounty::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_PosterId;
	out << m_PosterName;
	out << m_Reason;
	out << m_Reward;
}

void AuConBounty::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_PosterId;
	in >> m_PosterName;
	in >> m_Reason;
	in >> m_Reward;
}

// Returns false when this condition should be removed from the host.
bool AuConBounty::Tick(DaUnit* /*hostUnit*/) const
{
	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConBounty::CType() const
{
	return ConditionType();
}

QString AuConBounty::Name() const
{
	return QObject::tr("Reward %L1%2").arg(m_Reward, 0,'f',0).arg(g_Au.CurrencySymbol());
}

quint64 AuConBounty::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConBounty) - sizeof(AuCondition);
	mem += StringMemoryUsage(m_PosterName);
	mem += StringMemoryUsage(m_Reason);
	return mem;
}

bool AuConBounty::IsBounty(const AuBountyOnItem& bounty) const
{
	if(m_Reward != bounty.Amount())
		return false;
	if(m_PosterId != bounty.PosterId())
		return false;
	if(m_PosterName != bounty.PosterName())
		return false;
	if(m_Reason != bounty.Reason())
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConAntitoxin::AuConAntitoxin(quint8 quality, const AuTime& createTime) : AuCondition(createTime)
{
	Q_ASSERT(quality);
	m_Quality = quality;
}

AuConAntitoxin::AuConAntitoxin()
{
	m_Quality = 1;
}

AuConAntitoxin::AuConAntitoxin(const AuConAntitoxin& c) : AuCondition(c)
{
	m_Quality = c.m_Quality;
}

AuCondition* AuConAntitoxin::Clone() const
{
	return new AuConAntitoxin(*this);
}

QString AuConAntitoxin::Description() const
{
	return QObject::tr("%1 Q%2")
		.arg(Name())
		.arg(m_Quality);
}

void AuConAntitoxin::operator<<(QDataStream& in)
{
	AuCondition::operator<<(in);
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuConAntitoxin::operator>>(QDataStream& out) const
{
	AuCondition::operator>>(out);
	out << m_Quality;
}

// Returns false when this condition should be removed from the host.
bool AuConAntitoxin::Tick(DaUnit* hostUnit) const
{
	DaCreature* hostCreature = dynamic_cast<DaCreature*>(hostUnit);
	if(!hostCreature)
		return false;											// Antitoxin affects only creature units.
	if(hostCreature->IsBodyDyingOrDead())
		return true;			// Antitoxins do not revive dying bodies. They wait.

	// Antitoxin goes away after a while.
	quint8 effectiveQ = EffectiveQuality();
	if(effectiveQ == 0)
		return false;

	// Fight the disease.
	AuCondition* condition = hostCreature->FindConditionClone(AuConDisease::ConditionType());
	if(!condition)
		return true;	// Creature is not sick.

	// Antitoxin loses effectiveness with time.
	if(effectiveQ >= condition->Quality())
	{
		// Cure the disease.
		if(hostCreature->RemoveCondition(condition))
		{
			hostCreature->SetModified();
			hostCreature->Thought(QObject::tr("Immunization eliminated %1.").arg(condition->Name()));
		}
	}
	else
	{
		// Reduce one disease.
		if(hostCreature->ReduceCondition(condition, effectiveQ))
		{
			hostCreature->SetModified();
			hostCreature->Thought(QObject::tr("Immunization reduced %1.").arg(condition->Name()));
		}
	}
	delete condition;
	return true;
}

// Returns a number that is unique to each derived condition class.
AuCondition::CONDITION_TYPE AuConAntitoxin::CType() const
{
	return ConditionType();
}

// Returns the quality of the condition.
quint8 AuConAntitoxin::Quality() const
{
	return m_Quality;
}

QString AuConAntitoxin::Name() const
{
	return QObject::tr("Antitoxin");
}

quint64 AuConAntitoxin::MemoryUsage() const
{
	quint64 mem = AuCondition::MemoryUsage();
	mem += sizeof(AuConAntitoxin) - sizeof(AuCondition);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
