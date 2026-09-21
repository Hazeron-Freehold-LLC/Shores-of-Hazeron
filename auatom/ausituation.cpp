//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausituation.cpp,v 1.25 2022/02/19 17:41:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausituation.h"
//
#include "aucondition.h"
#include "dacitizen.h"
#include "dgdata.h"
#include <QDebug>

AuSituation::AuSituation(const AuTime& createTime)
{
	m_CreateTime = createTime;
}

AuSituation::AuSituation()
{
}

AuSituation::~AuSituation()
{
}

AuSituation::AuSituation(const AuSituation& c)
{
	m_CreateTime = c.m_CreateTime;
}

void AuSituation::operator<<(QDataStream& in)
{
	in >> m_CreateTime;
}

void AuSituation::operator>>(QDataStream& out) const
{
	out << m_CreateTime;
}

AuSituation* AuSituation::CreateSituation(quint8 situationType)
{
	if(situationType == AuSitDisease::SituationType())
		return new AuSitDisease;
	else if(situationType == AuSitPoison::SituationType())
		return new AuSitPoison;
	else if(situationType == AuSitRatsDNA::SituationType())
		return new AuSitRatsDNA;
	else if(situationType == AuSitRatsSpecies::SituationType())
		return new AuSitRatsSpecies;
	return 0;
}

// Returns the quality of the situation.
quint8 AuSituation::Quality() const
{
	return 0;
}

void AuSituation::InflictCitizen(DaCitizen* /*citizen*/) const
{
}

/*DHH DaCity
// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
QString AuSituation::SayNeed(const DaCity* city) const
{
	return "";
}
*/

quint64 AuSituation::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_CreateTime);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSituationList::operator<<(QDataStream& in)
{
	Clear();

	quint16 situations;
	in >> situations;
	quint8 type;
	while(situations--)
	{
		in >> type;
		AuSituation* situation = AuSituation::CreateSituation(type);
		if(situation)
		{
			in >> *situation;
			append(situation);
		}
		else
			qWarning() << "AuSituationList::operator<< received invalid situation type" << type;
	}
}

void AuSituationList::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << (*ci)->Type();
		out << **ci;
	}
}

quint64 AuSituationList::MemoryUsage() const
{
	quint64 mem = sizeof(AuSituationList);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += (*ci)->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSitPoison::AuSitPoison(quint8 strength, quint8 quality, const AuTime& createTime) : AuSituation(createTime)
{
	Q_ASSERT(quality);
	m_Antidotes = 0;
	m_Strength = strength;
	m_Quality = quality;
}

AuSitPoison::AuSitPoison()
{
	m_Antidotes = 0;
	m_Strength = 0;
	m_Quality = 1;
}

AuSitPoison::AuSitPoison(const AuSitPoison& c) : AuSituation(c)
{
	m_Antidotes = c.m_Antidotes;
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuSituation* AuSitPoison::Clone() const
{
	return new AuSitPoison(*this);
}

void AuSitPoison::operator<<(QDataStream& in)
{
	AuSituation::operator<<(in);
	in >> m_Antidotes;
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuSitPoison::operator>>(QDataStream& out) const
{
	AuSituation::operator>>(out);
	out << m_Antidotes;
	out << m_Strength;
	out << m_Quality;
}

/*DHH DaCity
// Returns false when this situation should be removed from the city.
bool AuSitPoison::Report(DaCity* city)
{
	// Determine if this situation has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
	{
		// Log the event.
		QString msg = QObject::tr("Food poisoning abated. %1 has passed.").arg(Name());
		city->LogEvent(AuEventLogEntry::Poison, msg);
		return false;		// Situation has run its course so remove it from the city.
	}

	// Calculate the number of people who need antidotes. This is the maximum number of antidotes that can be consumed this round.
	int sickPeople = city->Population() - m_Antidotes / (m_Quality+1);	// People who still need antidotes.
	if(sickPeople <= 0)
	{
		QString msg = QObject::tr("Food poisoning cured. All remaining citizens received antidotes for %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Poison, msg);
		return false;		// Everybody in town has been cured.
	}

	// Calculate the number of antidote units needed by the populace.
	int needByPeople = city->Population() * (m_Quality+1) - m_Antidotes;	// Number of antidote units needed by people.
	Q_ASSERT(needByPeople > 0);

	// Make a list of the antidotes available.
	QList<AuItem*> items;
	AuItemHash& inv = city->LockInventoryForWrite();
	for(AuItemHash::const_iterator ci = inv.find(CID_Antidote); ci != inv.end() && (*ci).first == CID_Antidote; ci++)
		items.append((*ci).second);

	// Apply antidotes.
	int itemsUsed = 0;
	for(QList<AuItem*>::const_iterator iii = items.cbegin(); iii != items.cend(); iii++)
	{
		// Locate the item in the city's inventory.
		AuItemHash::const_iterator ci = inv.Find(**iii);
		if(ci == inv.end())
			continue;

		// Determine how many of the items we can use.
		int itemsToUse;
		if((int)(*ci).second->Count() > sickPeople - itemsUsed)
			itemsToUse = sickPeople - itemsUsed;				// Can only use one item per person per round.
		else
			itemsToUse = (*ci).second->Count();							// Can use all of the items.

		// Apply the item.
		int unitsInItem = itemsToUse * ((*ci).second->Quality()+1);	// Total units available by consuming the entire item.
		if(unitsInItem >= needByPeople)
		{
			// Use as much of the item as we need to cure everybody.
			itemsToUse = (needByPeople + (*ci).second->Quality()) / ((*ci).second->Quality()+1);
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Antidotes += needByPeople;						// Tally total units consumed.
			needByPeople = 0;									// Remaining need is 0.
		}
		else
		{
			// Use all of the item.
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Antidotes += unitsInItem;							// Tally total units consumed;
			needByPeople -= unitsInItem;						// Reduce remaining need.
		}

		// Consume some or all of the item from the city's inventory.
		city->RemoveInventoryNoLock(*(*ci).second, itemsToUse);

		// Must stop consuming items once we've given one to each sick person this round.
		if(itemsUsed >= sickPeople || !needByPeople)
			break;
	}
	city->UnlockInventory();

	// Did we manage to cure everybody this round?
	if(!needByPeople)
	{
		QString msg = QObject::tr("Food poisoning cured. Gave %2 antidotes to citizens for %1.").arg(Name()).arg(itemsUsed);
		city->LogEvent(AuEventLogEntry::Poison, msg);
		return false;											// The last of the affected citizens was cured.
	}

	sickPeople = (needByPeople + Quality()) / (Quality()+1);
	QString msg;
	if(itemsUsed)
		msg = QObject::tr("Food poisoning. Gave %1 antidotes to citizens; %2 antidotes still needed to cure %3. ")
			.arg(itemsUsed).arg(sickPeople).arg(Name());
	else
		msg = QObject::tr("Food poisoning. %1 antidotes needed to cure %2. ").arg(sickPeople).arg(Name());

	// Calculate damage caused to the city. The damage declines over time.
	int peopleAffected = sickPeople * Toxicity() / 255;
	int peopleKilled = g_Au.Dice().Roll(1, peopleAffected+1, -1);
	if(peopleKilled <= 0)
	{
		city->LogEvent(AuEventLogEntry::Poison, msg);
		return true;											// Situation did not cause any deaths this time.
	}
	if(peopleKilled > sickPeople)
		peopleKilled = sickPeople;								// The last of the unprotected citizens have died.
	msg += QObject::tr("%1 citizens died.").arg(peopleKilled);
	city->LogEvent(AuEventLogEntry::Poison, msg);

	// Cause damage to the city.
	if(peopleKilled > 0)
		city->DecreasePopulation(peopleKilled);
	city->SetModified(DaAtom::NOTIFY_Database);

	return true;
}
*/

// Returns a number that is unique to each derived situation class.
quint8 AuSitPoison::Type() const
{
	return SituationType();
}

// Returns the quality of the situation.
quint8 AuSitPoison::Quality() const
{
	return m_Quality;
}

QString AuSitPoison::Name() const
{
	return QObject::tr("Poison %1").arg(m_Quality+1);
}

// Called for each nu citizen that emerges in/from the afflicted city.
void AuSitPoison::InflictCitizen(DaCitizen* citizen) const
{
	AuConPoison* poison = new AuConPoison(m_Strength, m_Quality, AuTime::Now());
	citizen->AddCondition(poison);
}

/*DHH DaCity
// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
QString AuSitPoison::SayNeed(const DaCity* city) const
{
	int unitsNeeded = city->Population() * (m_Quality+1) - m_Antidotes;	// Number of people who are not protected by antidotes.
	if(unitsNeeded <= 0)
		return "";
	int remainingNeed = (unitsNeeded + Quality()) / (Quality()+1);
	return QObject::tr(" We need %1 units of %2 Q%3; we can use lower quality %2.")
		.arg(remainingNeed).arg(g_Au.CommodityName(CID_Antidote)).arg(m_Quality);
}
*/

quint64 AuSitPoison::MemoryUsage() const
{
	quint64 mem = AuSituation::MemoryUsage();
	mem += sizeof(m_Antidotes);
	mem += sizeof(m_Strength);
	mem += sizeof(m_Quality);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSitDisease::AuSitDisease(quint8 strength, quint8 contagion, quint8 quality, const AuTime& createTime) :
	AuSituation(createTime)
{
	Q_ASSERT(quality);
	m_Antitoxins = 0;
	m_Contagion = contagion;
	m_Strength = strength;
	m_Quality = quality;
}

AuSitDisease::AuSitDisease()
{
	m_Antitoxins = 0;
	m_Contagion = 0;
	m_Strength = 0;
	m_Quality = 1;
}

AuSitDisease::AuSitDisease(const AuSitDisease& c) : AuSituation(c)
{
	m_Antitoxins = c.m_Antitoxins;
	m_Contagion = c.m_Contagion;
	m_Strength = c.m_Strength;
	m_Quality = c.m_Quality;
}

AuSituation* AuSitDisease::Clone() const
{
	return new AuSitDisease(*this);
}

void AuSitDisease::operator<<(QDataStream& in)
{
	AuSituation::operator<<(in);
	in >> m_Antitoxins;
	in >> m_Contagion;
	in >> m_Strength;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuSitDisease::operator>>(QDataStream& out) const
{
	AuSituation::operator>>(out);
	out << m_Antitoxins;
	out << m_Contagion;
	out << m_Strength;
	out << m_Quality;
}

/*DHH DaCity
// Returns false when this situation should be removed from the city.
bool AuSitDisease::Report(DaCity* city)
{
	// Determine if this situation has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
	{
		// Log the event.
		QString msg = QObject::tr("Epidemic abated. %1 has run its course.").arg(Name());
		city->LogEvent(AuEventLogEntry::Disease, msg);
		return false;		// Situation has run its course so remove it from the city.
	}

	// Calculate the number of people who need antitoxins. This is the maximum number of antitoxins that can be consumed this round.
	int sickPeople = city->Population() - m_Antitoxins / (m_Quality+1);	// People who still need antitoxins.
	if(sickPeople <= 0)
	{
		QString msg = QObject::tr("Epidemic cured. All remaining citizens received antitoxins for %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Disease, msg);
		return false;		// Everybody in town has been cured.
	}

	// Calculate the number of antitoxin units needed by the populace.
	int needByPeople = city->Population() * (m_Quality+1) - m_Antitoxins;	// Number of antitoxin units needed by people.
	Q_ASSERT(needByPeople > 0);

	// Make a list of the antitoxins available.
	QList<AuItem*> items;
	AuItemHash& inv = city->LockInventoryForWrite();
	for(AuItemHash::const_iterator ci = inv.find(CID_Antitoxin); ci != inv.end() && (*ci).first == CID_Antitoxin; ci++)
		items.append((*ci).second);

	// Apply antitoxins.
	int itemsUsed = 0;
	for(QList<AuItem*>::const_iterator iii = items.cbegin(); iii != items.cend(); iii++)
	{
		// Locate the item in the city's inventory.
		AuItemHash::const_iterator ci = inv.Find(**iii);
		if(ci == inv.end())
			continue;

		// Determine how many of the items we can use.
		int itemsToUse;
		if((int)(*ci).second->Count() > sickPeople - itemsUsed)
			itemsToUse = sickPeople - itemsUsed;				// Can only use one item per person per round.
		else
			itemsToUse = (*ci).second->Count();							// Can use all of the items.

		// Apply the item.
		int unitsInItem = itemsToUse * ((*ci).second->Quality()+1);	// Total units available by consuming the entire item.
		if(unitsInItem >= needByPeople)
		{
			// Use as much of the item as we need to cure everybody.
			itemsToUse = (needByPeople + (*ci).second->Quality()) / ((*ci).second->Quality()+1);
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Antitoxins += needByPeople;						// Tally total units consumed.
			needByPeople = 0;									// Remaining need is 0.
		}
		else
		{
			// Use all of the item.
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Antitoxins += unitsInItem;						// Tally total units consumed;
			needByPeople -= unitsInItem;						// Reduce remaining need.
		}

		// Consume some or all of the item from the city's inventory.
		city->RemoveInventoryNoLock(*(*ci).second, itemsToUse);

		// Must stop consuming items once we've given one to each sick person this round.
		if(itemsUsed >= sickPeople || !needByPeople)
			break;
	}
	city->UnlockInventory();

	// Did we manage to cure everybody this round?
	if(needByPeople <= 0)
	{
		QString msg = QObject::tr("Epidemic cured. Gave %2 antitoxins to citizens for %1.").arg(Name()).arg(itemsUsed);
		city->LogEvent(AuEventLogEntry::Disease, msg);
		return false;											// The last of the affected citizens was cured.
	}

	sickPeople = (needByPeople + Quality()) / (Quality()+1);
	QString msg;
	if(itemsUsed)
		msg = QObject::tr("Epidemic. Gave %1 antitoxins to citizens; %2 antitoxins still needed to cure %3. ")
			.arg(itemsUsed).arg(sickPeople).arg(Name());
	else
		msg = QObject::tr("Epidemic. %1 antitoxins needed to cure %2. ").arg(sickPeople).arg(Name());

	// Calculate damage caused to the city. The damage declines over time.
	int peopleAffected = sickPeople * Virulence() / 255;
	int peopleKilled = g_Au.Dice().Roll(1, peopleAffected+1, -1);
	if(peopleKilled <= 0)
	{
		city->LogEvent(AuEventLogEntry::Disease, msg);
		return true;											// Situation did not cause any deaths this time.
	}
	if(peopleKilled > sickPeople)
		peopleKilled = sickPeople;								// The last of the unprotected citizens have died.
	msg += QObject::tr("%1 citizens died.").arg(peopleKilled);
	city->LogEvent(AuEventLogEntry::Disease, msg);

	// Cause damage to the city.
	if(peopleKilled > 0)
		city->DecreasePopulation(peopleKilled);
	city->SetModified(DaAtom::NOTIFY_Database);

	return true;
}
*/

// Returns a number that is unique to each derived situation class.
quint8 AuSitDisease::Type() const
{
	return SituationType();
}

// Returns the quality of the situation.
quint8 AuSitDisease::Quality() const
{
	return m_Quality;
}

QString AuSitDisease::Name() const
{
	return QObject::tr("Disease %1").arg(m_Quality+1);
}

// Called for each nu citizen that emerges in/from the afflicted city.
void AuSitDisease::InflictCitizen(DaCitizen* citizen) const
{
	AuConDisease* disease = new AuConDisease(m_Strength, m_Contagion, m_Quality, AuTime::Now());
	citizen->AddCondition(disease);
}

/*DHH DaCity
// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
QString AuSitDisease::SayNeed(const DaCity* city) const
{
	int unitsNeeded = city->Population() * (m_Quality+1) - m_Antitoxins;	// Number of people who are not protected by antitoxins.
	if(unitsNeeded <= 0)
		return "";
	int remainingNeed = (unitsNeeded + Quality()) / (Quality()+1);
	return QObject::tr(" We need %1 units of %2 Q%3; we can use lower quality %2.")
		.arg(remainingNeed).arg(g_Au.CommodityName(CID_Antitoxin)).arg(m_Quality);
}
*/

quint64 AuSitDisease::MemoryUsage() const
{
	quint64 mem = AuSituation::MemoryUsage();
	mem += sizeof(m_Antitoxins);
	mem += sizeof(m_Contagion);
	mem += sizeof(m_Strength);
	mem += sizeof(m_Quality);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSitRatsDNA::AuSitRatsDNA(const DhDNA& dna, quint8 voracity, quint8 quality, const AuTime& createTime) : AuSituation(createTime)
{
	Q_ASSERT(quality);
	m_DNA = dna;
	m_Pharmaceuticals = 0;
	m_Quality = quality;
	m_Voracity = voracity;
}

AuSitRatsDNA::AuSitRatsDNA()
{
	m_Pharmaceuticals = 0;
	m_Quality = 1;
	m_Voracity = 0;
}

AuSitRatsDNA::AuSitRatsDNA(const AuSitRatsDNA& c) : AuSituation(c)
{
	m_DNA = c.m_DNA;
	m_Pharmaceuticals = c.m_Pharmaceuticals;
	m_Quality = c.m_Quality;
	m_Voracity = c.m_Voracity;
}

AuSituation* AuSitRatsDNA::Clone() const
{
	return new AuSitRatsDNA(*this);
}

void AuSitRatsDNA::operator<<(QDataStream& in)
{
	AuSituation::operator<<(in);
	in >> m_DNA;
	in >> m_Pharmaceuticals;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_Voracity;
}

void AuSitRatsDNA::operator>>(QDataStream& out) const
{
	AuSituation::operator>>(out);
	out << m_DNA;
	out << m_Pharmaceuticals;
	out << m_Quality;
	out << m_Voracity;
}

/*DHH DaCity
// Returns false when this situation should be removed from the city.
bool AuSitRatsDNA::Report(DaCity* city)
{
	// Determine if this situation has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
	{
		// Log the event.
		QString msg = QObject::tr("Infestation abated. %1 have gone away.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// Situation has run its course so remove it from the city.
	}

	// Measure how much food the vermin might eat.
	int totalFood = city->LockInventoryForRead().TotalFoodUnits(m_DNA.EcoRole());	// Returns total number of food commodity units.
	city->UnlockInventory();
	if(totalFood <= 0)
	{
		QString msg = QObject::tr("Infestation starved. City has no food for %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// All the food in town has been eaten.
	}

	// Attempt to kill the remaining vermin with the pharmaceuticals available.
	int ratPopulation = totalFood * (m_Voracity+1) / 256;		// Current rat population.

	// Calculate the number of rats who need poisoning. This is the maximum number of poisons that can be consumed this round.
	int healthyRats = ratPopulation - m_Pharmaceuticals / (m_Quality+1);	// Rats who still need poisoning.
	if(healthyRats <= 0)
	{
		QString msg = QObject::tr("Infestation eliminated. Pharmaceuticals poisoned the remaining %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// Everybody in town has been cured.
	}

	// Calculate the number of food units that may be eaten by the rats this round.
	int needByRats = ratPopulation * (m_Quality+1) - m_Pharmaceuticals;	// Number of food units to be eaten by rats.
	Q_ASSERT(needByRats > 0);

	// Make a list of the pharmaceuticals available.
	QList<AuItem*> items;
	AuItemHash& inv = city->LockInventoryForWrite();
	for(AuItemHash::const_iterator ci = inv.find(CID_Pharmaceuticals);
			ci != inv.end() && (*ci).first == CID_Pharmaceuticals; ci++)
		items.append((*ci).second);

	// Apply pharmaceuticals.
	int itemsUsed = 0;
	for(QList<AuItem*>::const_iterator iii = items.cbegin(); iii != items.cend(); iii++)
	{
		// Locate the item in the city's inventory.
		AuItemHash::const_iterator ci = inv.Find(**iii);
		if(ci == inv.end())
			continue;

		// Determine how many of the items we can use.
		int itemsToUse;
		if((int)(*ci).second->Count() > healthyRats - itemsUsed)
			itemsToUse = healthyRats - itemsUsed;				// Can only use one item per rat per round.
		else
			itemsToUse = (*ci).second->Count();							// Can use all of the items.

		// Apply the item.
		int unitsInItem = itemsToUse * ((*ci).second->Quality()+1);	// Total units available by consuming the entire item.
		if(unitsInItem >= needByRats)
		{
			// Use as much of the item as we need to cure everybody.
			itemsToUse = (needByRats + (*ci).second->Quality()) / ((*ci).second->Quality()+1);
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Pharmaceuticals += needByRats;					// Tally total units consumed.
			needByRats = 0;										// Remaining need is 0.
		}
		else
		{
			// Use all of the item.
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Pharmaceuticals += unitsInItem;					// Tally total units consumed;
			needByRats -= unitsInItem;							// Reduce remaining need.
		}

		// Consume some or all of the item from the city's inventory.
		city->RemoveInventoryNoLock(*(*ci).second, itemsToUse);

		// Must stop consuming items once we've given one to each health rat this round.
		if(itemsUsed >= healthyRats || !needByRats)
			break;
	}
	city->UnlockInventory();

	// Did we manage to kill every rat this round?
	if(!needByRats)
	{
		QString msg = QObject::tr("Infestation eliminated. Poisoned %1 with %2 pharmaceuticals.").arg(Name()).arg(itemsUsed);
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;											// The last of the affected citizens was cured.
	}

	healthyRats = (needByRats + Quality()) / (Quality()+1);
	QString msg;
	if(itemsUsed)
		msg = QObject::tr("Infestation. Poisoned %3 using %1 pharmaceuticals; %2 pharmaceuticals still needed to kill them off. ")
			.arg(itemsUsed).arg(healthyRats).arg(Name());
	else
		msg = QObject::tr("Infestation. %1 pharmaceuticals needed to kill off %2. ").arg(healthyRats).arg(Name());

	// Calculate damage caused to the city.
	int foodEaten = g_Au.Dice().Roll(1, needByRats);
	foodEaten = city->EatFoodUnits(foodEaten, m_DNA.EcoRole());	// Returns the actual number of units consumed.

	msg += QObject::tr("%1 food eaten.").arg(foodEaten);
	city->LogEvent(AuEventLogEntry::Infestation, msg);
	city->SetModified(DaAtom::NOTIFY_Database);

	return true;
}
*/

// Returns a number that is unique to each derived situation class.
quint8 AuSitRatsDNA::Type() const
{
	return SituationType();
}

// Returns the quality of the situation.
quint8 AuSitRatsDNA::Quality() const
{
	return m_Quality;
}

QString AuSitRatsDNA::Name() const
{
	return QObject::tr("%1 vermin %2").arg(m_DNA.SpeciesDescription()).arg(m_Quality+1);
}

/*DHH DaCity
// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
QString AuSitRatsDNA::SayNeed(const DaCity* city) const
{
	int totalFood = city->LockInventoryForRead().TotalFoodUnits(m_DNA.EcoRole());	// Returns total number of food commodity units.
	city->UnlockInventory();
	if(totalFood <= 0)
		return "";

	int mayEat = totalFood * (m_Quality+1) * (m_Voracity+1) / 256;	// Food units to be eaten.
	if(!mayEat)
		mayEat = 1;
	mayEat -= m_Pharmaceuticals;				// Pharmaceuticals act as poison to vermin.
	if(mayEat <= 0)
		return "";
	int remainingNeed = (mayEat + Quality()) / (Quality()+1);
	return QObject::tr(" We need %1 units of %2 Q%3; we can use lower quality %2.")
		.arg(remainingNeed).arg(g_Au.CommodityName(CID_Pharmaceuticals)).arg(m_Quality);
}
*/

quint64 AuSitRatsDNA::MemoryUsage() const
{
	quint64 mem = AuSituation::MemoryUsage();
	mem += sizeof(m_DNA);
	mem += sizeof(m_Pharmaceuticals);
	mem += sizeof(m_Quality);
	mem += sizeof(m_Voracity);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSitRatsSpecies::AuSitRatsSpecies(const AuSpecies& species, quint8 voracity, quint8 quality, const AuTime& createTime) :
	AuSituation(createTime), m_Species(species)
{
	Q_ASSERT(quality);
	m_Pharmaceuticals = 0;
	m_Quality = quality;
	m_Voracity = voracity;
}

AuSitRatsSpecies::AuSitRatsSpecies()
{
	m_Pharmaceuticals = 0;
	m_Quality = 1;
	m_Voracity = 0;
}

AuSitRatsSpecies::AuSitRatsSpecies(const AuSitRatsSpecies& c) : AuSituation(c), m_Species(c.m_Species)
{
	m_Pharmaceuticals = c.m_Pharmaceuticals;
	m_Quality = c.m_Quality;
	m_Voracity = c.m_Voracity;
}

AuSituation* AuSitRatsSpecies::Clone() const
{
	return new AuSitRatsSpecies(*this);
}

void AuSitRatsSpecies::operator<<(QDataStream& in)
{
	AuSituation::operator<<(in);
	in >> m_Species;
	in >> m_Pharmaceuticals;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_Voracity;
}

void AuSitRatsSpecies::operator>>(QDataStream& out) const
{
	AuSituation::operator>>(out);
	out << m_Species;
	out << m_Pharmaceuticals;
	out << m_Quality;
	out << m_Voracity;
}

/*DHH DaCity
// Returns false when this situation should be removed from the city.
bool AuSitRatsSpecies::Report(DaCity* city)
{
	// Determine if this situation has worn off.
	int secsAlive = m_CreateTime.ElapsedSec();
	if(secsAlive >= DurationSeconds())
	{
		// Log the event.
		QString msg = QObject::tr("Infestation abated. %1 have gone away.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// Situation has run its course so remove it from the city.
	}

	// Measure how much food the vermin might eat.
	int totalFood = city->LockInventoryForRead().TotalFoodUnits(m_DNA.EcoRole());	// Returns total number of food commodity units.
	city->UnlockInventory();
	if(totalFood <= 0)
	{
		QString msg = QObject::tr("Infestation starved. City has no food for %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// All the food in town has been eaten.
	}

	// Attempt to kill the remaining vermin with the pharmaceuticals available.
	int ratPopulation = totalFood * (m_Voracity+1) / 256;		// Current rat population.

	// Calculate the number of rats who need poisoning. This is the maximum number of poisons that can be consumed this round.
	int healthyRats = ratPopulation - m_Pharmaceuticals / (m_Quality+1);	// Rats who still need poisoning.
	if(healthyRats <= 0)
	{
		QString msg = QObject::tr("Infestation eliminated. Pharmaceuticals poisoned the remaining %1.").arg(Name());
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;		// Everybody in town has been cured.
	}

	// Calculate the number of food units that may be eaten by the rats this round.
	int needByRats = ratPopulation * (m_Quality+1) - m_Pharmaceuticals;	// Number of food units to be eaten by rats.
	Q_ASSERT(needByRats > 0);

	// Make a list of the pharmaceuticals available.
	QList<AuItem*> items;
	AuItemHash& inv = city->LockInventoryForWrite();
	for(AuItemHash::const_iterator ci = inv.find(CID_Pharmaceuticals);
			ci != inv.end() && (*ci).first == CID_Pharmaceuticals; ci++)
		items.append((*ci).second);

	// Apply pharmaceuticals.
	int itemsUsed = 0;
	for(QList<AuItem*>::const_iterator iii = items.cbegin(); iii != items.cend(); iii++)
	{
		// Locate the item in the city's inventory.
		AuItemHash::const_iterator ci = inv.Find(**iii);
		if(ci == inv.end())
			continue;

		// Determine how many of the items we can use.
		int itemsToUse;
		if((int)(*ci).second->Count() > healthyRats - itemsUsed)
			itemsToUse = healthyRats - itemsUsed;				// Can only use one item per rat per round.
		else
			itemsToUse = (*ci).second->Count();							// Can use all of the items.

		// Apply the item.
		int unitsInItem = itemsToUse * ((*ci).second->Quality()+1);	// Total units available by consuming the entire item.
		if(unitsInItem >= needByRats)
		{
			// Use as much of the item as we need to cure everybody.
			itemsToUse = (needByRats + (*ci).second->Quality()) / ((*ci).second->Quality()+1);
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Pharmaceuticals += needByRats;					// Tally total units consumed.
			needByRats = 0;										// Remaining need is 0.
		}
		else
		{
			// Use all of the item.
			itemsUsed += itemsToUse;							// Tally total items consumed.
			m_Pharmaceuticals += unitsInItem;					// Tally total units consumed;
			needByRats -= unitsInItem;							// Reduce remaining need.
		}

		// Consume some or all of the item from the city's inventory.
		city->RemoveInventoryNoLock(*(*ci).second, itemsToUse);

		// Must stop consuming items once we've given one to each health rat this round.
		if(itemsUsed >= healthyRats || !needByRats)
			break;
	}
	city->UnlockInventory();

	// Did we manage to kill every rat this round?
	if(!needByRats)
	{
		QString msg = QObject::tr("Infestation eliminated. Poisoned %1 with %2 pharmaceuticals.").arg(Name()).arg(itemsUsed);
		city->LogEvent(AuEventLogEntry::Infestation, msg);
		return false;											// The last of the affected citizens was cured.
	}

	healthyRats = (needByRats + Quality()) / (Quality()+1);
	QString msg;
	if(itemsUsed)
		msg = QObject::tr("Infestation. Poisoned %3 using %1 pharmaceuticals; %2 pharmaceuticals still needed to kill them off. ")
			.arg(itemsUsed).arg(healthyRats).arg(Name());
	else
		msg = QObject::tr("Infestation. %1 pharmaceuticals needed to kill off %2. ").arg(healthyRats).arg(Name());

	// Calculate damage caused to the city.
	int foodEaten = g_Au.Dice().Roll(1, needByRats);
	foodEaten = city->EatFoodUnits(foodEaten, m_DNA.EcoRole());	// Returns the actual number of units consumed.

	msg += QObject::tr("%1 food eaten.").arg(foodEaten);
	city->LogEvent(AuEventLogEntry::Infestation, msg);
	city->SetModified(DaAtom::NOTIFY_Database);

	return true;
}
*/

// Returns a number that is unique to each derived situation class.
quint8 AuSitRatsSpecies::Type() const
{
	return SituationType();
}

// Returns the quality of the situation.
quint8 AuSitRatsSpecies::Quality() const
{
	return m_Quality;
}

QString AuSitRatsSpecies::Name() const
{
	return QObject::tr("%1 vermin %2").arg(m_Species.Description()).arg(m_Quality+1);
}

/*DHH DaCity
// State a brief sentence to a traveller of what is needed by the city to remedy this situation.
QString AuSitRatsSpecies::SayNeed(const DaCity* city) const
{
	int totalFood = city->LockInventoryForRead().TotalFoodUnits(m_DNA.EcoRole());	// Returns total number of food commodity units.
	city->UnlockInventory();
	if(totalFood <= 0)
		return "";

	int mayEat = totalFood * (m_Quality+1) * (m_Voracity+1) / 256;	// Food units to be eaten.
	if(!mayEat)
		mayEat = 1;
	mayEat -= m_Pharmaceuticals;				// Pharmaceuticals act as poison to vermin.
	if(mayEat <= 0)
		return "";
	int remainingNeed = (mayEat + Quality()) / (Quality()+1);
	return QObject::tr(" We need %1 units of %2 Q%3; we can use lower quality %2.")
		.arg(remainingNeed).arg(g_Au.CommodityName(CID_Pharmaceuticals)).arg(m_Quality);
}
*/

quint64 AuSitRatsSpecies::MemoryUsage() const
{
	quint64 mem = AuSituation::MemoryUsage();
	mem += sizeof(m_Pharmaceuticals);
	mem += sizeof(m_Quality);
	mem += m_Species.MemoryUsage();
	mem += sizeof(m_Voracity);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSitBounty::AuSitBounty(const QString& reason, double reward, const AuTime& createTime) : AuSituation(createTime)
{
	m_Reason = reason;
	m_Reward = reward;
}

AuSitBounty::AuSitBounty()
{
	m_Reward = 0;
}

AuSitBounty::AuSitBounty(const AuSitBounty& c) : AuSituation(c)
{
	m_Reason = c.m_Reason;
	m_Reward = c.m_Reward;
}

AuSituation* AuSitBounty::Clone() const
{
	return new AuSitBounty(*this);
}

void AuSitBounty::operator<<(QDataStream& in)
{
	AuSituation::operator<<(in);
	in >> m_Reason;
	in >> m_Reward;
}

void AuSitBounty::operator>>(QDataStream& out) const
{
	AuSituation::operator>>(out);
	out << m_Reason;
	out << m_Reward;
}

/*DHH DaCity
bool AuSitBounty::Report(DaCity* city)
{
	return true;
}
*/

// Returns a number that is unique to each derived condition class.
quint8 AuSitBounty::Type() const
{
	return SituationType();
}

QString AuSitBounty::Name() const
{
	return QObject::tr("Reward %L1%2").arg(m_Reward).arg(g_Au.CurrencySymbol());
}

quint64 AuSitBounty::MemoryUsage() const
{
	quint64 mem = AuSituation::MemoryUsage();
	mem += StringMemoryUsage(m_Reason);
	mem += sizeof(m_Reward);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
