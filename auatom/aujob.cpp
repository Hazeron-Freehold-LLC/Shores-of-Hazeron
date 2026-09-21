//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujob.cpp,v 1.93 2022/02/19 17:40:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aujob.h"
//
#include "aubank.h"
#include "aucommpacket.h"
#include "auglobal.h"
#include "dabd.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "dgdata.h"
#include <QDebug>

AuJob::AuJob(const AuTime& start)
{
	m_Start = start;
}

AuJob::AuJob()
{
}

AuJob::~AuJob()
{
}

AuJob::AuJob(const AuJob& c)
{
	m_Start = c.m_Start;
}

const AuJob& AuJob::operator=(const AuJob& c)
{
	m_Start = c.m_Start;
	return *this;
}

//static
AuJob* AuJob::CreateJob(TYPE type)
{
	switch(type)
	{
	case Assassin:
		return new AuJobAssassin;
	case BountyHunterPerson:
		return new AuJobBountyHunterPerson;
	case BountyHunterShip:
		return new AuJobBountyHunterShip;
	case BountyHunterVehicle:
		return new AuJobBountyHunterVehicle;
	case CommandShipInOrbit:
		return new AuJobCommandShipInOrbit;
	case CommandShipOnGround:
		return new AuJobCommandShipOnGround;
	case DeliverCargoConstruction:
		return new AuJobDeliverCargoConstruction;
	case DeliverCargoDisease:
		return new AuJobDeliverCargoDisease;
	case DeliverCargoGasping:
		return new AuJobDeliverCargoGasping;
	case DeliverCargoManufacture:
		return new AuJobDeliverCargoManufacture;
	case DeliverCargoPoison:
		return new AuJobDeliverCargoPoison;
	case DeliverCargoRats:
		return new AuJobDeliverCargoRats;
	case DeliverCargoSpacecraftFactory:
		return new AuJobDeliverCargoSpacecraftFactory;
	case DeliverCargoSuffocating:
		return new AuJobDeliverCargoSuffocating;
	case DeliverFoodHungry:
		return new AuJobDeliverFoodHungry;
	case DeliverFoodStarving:
		return new AuJobDeliverFoodStarving;
	case DestroyEnemyStation:
		return new AuJobDestroyEnemyStation;
	case DeliverPassengersColonists:
		return new AuJobDeliverPassengersColonists;
	case DeliverPassengersDeserted:
		return new AuJobDeliverPassengersDeserted;
	case DeliverPassengersWorkers:
		return new AuJobDeliverPassengersWorkers;
	case InterdictSystem:
		return new AuJobInterdictSystem;
	case PickUpPassengersHomeless:
		return new AuJobPickUpPassengersHomeless;
	case PickUpPassengersOverpopulated:
		return new AuJobPickUpPassengersOverpopulated;
	case PickUpPassengersStranded:
		return new AuJobPickUpPassengersStranded;
	case Story:
		return new AuJobStory;
	case TransferPrisoners:
		return new AuJobTransferPrisoners;
	case VisitSystem:
		return new AuJobVisitSystem;
	case JobTypes:
	case InvalidJob:
		break;
	}
	return 0;
}

//static
QString AuJob::IconName(TYPE type)
{
	QString iconName;
	AuJob* job = CreateJob(type);
	if(job)
	{
		iconName = job->IconName();
		delete job;
	}
	return iconName;
}

//static
QString AuJob::TypeName(TYPE type)
{
	QString typeName;
	AuJob* job = CreateJob(type);
	if(job)
	{
		typeName = job->TypeName();
		delete job;
	}
	return typeName;
}

void AuJob::operator<<(QDataStream& in)
{
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_Start = tm;
}

void AuJob::operator>>(QDataStream& out) const
{
	out << m_Start.UTC();
}

bool AuJob::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	Q_UNUSED(sectorRange);
	// Do not offer jobs from empires they have not encountered.
	const DgData& gd = g_Au.LockDataForRead();
	bool isKnown = gd.IsEmpireKnown(fromEmpireId, toPerson.EmpireId());
	g_Au.UnlockData();
	if(!isKnown)
		return false;

	// Do not offer jobs that are from enemy empires.
	// In this case, the stance of the offerer's empire toward the offeree's empire is considered.
	// Jobs are not offered to people who are considered enemies.
	POLITICAL_STANCE stanceToPerson = PoliticalStance(fromEmpireId, toPerson.EmpireId());
	if(stanceToPerson == POL_Enemy)
		return false;

	return true;
}

bool AuJob::Compare(const AuJob* c)
{
	if(m_Start != c->m_Start)
		return false;
	return true;
}

int AuJob::CargoDelivered(DaPerson* /*person*/, const AuId& /*cityId*/, const AuItem& /*item*/)
{
	return 0;
}

int AuJob::CryoDelivered(DaPerson* /*person*/, const AuId& /*cityId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::CryoPickedUp(DaPerson* /*person*/, const AuId& /*cityId*/, const AuId& /*worldId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::PassengersDelivered(DaPerson* /*person*/, const AuId& /*cityId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::PassengersPickedUp(DaPerson* /*person*/, const AuId& /*cityId*/, const AuId& /*worldId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::PersonKilled(DaPerson* /*person*/, DaPerson* /*victim*/)
{
	return 0;
}

int AuJob::PrisonersLoaded(DaPerson* /*person*/, const AuId& /*cityId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::PrisonersUnloaded(DaPerson* /*person*/, const AuId& /*cityId*/, quint16 /*count*/)
{
	return 0;
}

int AuJob::SpacecraftCaptured(DaPerson* /*person*/, DaVessel* /*ship*/, const AuId& /*inSystemId*/)
{
	return 0;
}

int AuJob::SpacecraftCommanded(DaPerson* /*person*/, const AuId& /*shipId*/)
{
	return 0;
}

int AuJob::SpacecraftDestroyed(DaPerson* /*person*/, DaVessel* /*shipshipId*/, const AuId& /*inSystemId*/)
{
	return 0;
}

int AuJob::SystemVisited(DaPerson* /*person*/, const AuId& /*systemId*/, DaBd& /*bd*/)
{
	return 0;
}

int AuJob::VehicleCaptured(DaPerson* /*person*/, DaVehicle* /*vehiclevehicleId*/)
{
	return 0;
}

int AuJob::VehicleDestroyed(DaPerson* /*person*/, DaVehicle* /*vehiclevehicleId*/)
{
	return 0;
}

quint64 AuJob::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_Start);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuJobList::operator<<(QDataStream& in)
{
	Clear();

	quint16 jobcount;
	in >> jobcount;
	quint8 jobtype;
	while(jobcount--)
	{
		in >> jobtype;

		AuJob* job = AuJob::CreateJob((AuJob::TYPE)jobtype);
		if(!job)
		{
			qWarning() << "error: AuJobList::operator<< read bad object type" << jobtype << "discarding it";
			break;
		}

		in >> *job;
		if(job->IsObsolete())
			delete job;
		else
			append(job);
	}
}

void AuJobList::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() <= 0xffff);
	out << (quint16)count();
	for(const_iterator ji = constBegin(); ji != constEnd(); ji++)
	{
		out << (quint8)(*ji)->Type();
		out << **ji;
	}
}

bool AuJobList::CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->CargoDelivered(person, cityId, item))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::CryoDelivered(DaPerson* person, const AuId& cityId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->CryoDelivered(person, cityId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::CryoPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->CryoPickedUp(person, cityId, worldId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::PassengersDelivered(DaPerson* person, const AuId& cityId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->PassengersDelivered(person, cityId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->PassengersPickedUp(person, cityId, worldId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::PersonKilled(DaPerson* person, DaPerson* victim)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->PersonKilled(person, victim))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::PrisonersLoaded(DaPerson* person, const AuId& cityId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->PrisonersLoaded(person, cityId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::PrisonersUnloaded(DaPerson* person, const AuId& cityId, quint16 count)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->PrisonersUnloaded(person, cityId, count))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->SpacecraftCaptured(person, ship, inSystemId))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::SpacecraftCommanded(DaPerson* person, const AuId& shipId)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->SpacecraftCommanded(person, shipId))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->SpacecraftDestroyed(person, ship, inSystemId))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::SystemVisited(DaPerson* person, const AuId& systemId, DaBd& bd)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->SystemVisited(person, systemId, bd))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::VehicleCaptured(DaPerson* person, DaVehicle* vehicle)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->VehicleCaptured(person, vehicle))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

bool AuJobList::VehicleDestroyed(DaPerson* person, DaVehicle* vehicle)
{
	bool anyChange = false;
	iterator ji = begin();
	while(ji != end())
	{
		switch((*ji)->VehicleDestroyed(person, vehicle))
		{
		case 1:							// Job was changed by the event but it was not completed.
			anyChange = true;
			// Fall through, no break;
			[[fallthrough]];
		case 0:							// No change resulted from the event.
			ji++;
			break;
		case 2:							// Job was completed by the event.
			delete *ji;
			ji = erase(ji);
			anyChange = true;
			break;
		}
	}
	return anyChange;
}

quint64 AuJobList::MemoryUsage() const
{
	quint64 mem = sizeof(AuJobList);
	for(const_iterator ji = constBegin(); ji != constEnd(); ji++)
		mem += (*ji)->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobAssassin::AuJobAssassin(const AuId& preyId, const QString& preyName, const QString& lastSeen, quint32 reward) :
	AuJob(AuTime::Now())
{
	m_LastSeen = lastSeen;
	m_PreyId = preyId;
	m_PreyName = preyName;
	m_Reward = reward;
}

AuJobAssassin::AuJobAssassin()
{
	m_Reward = 0;
}

AuJobAssassin::AuJobAssassin(const AuJobAssassin& c) : AuJob(c)
{
	m_LastSeen = c.m_LastSeen;
	m_PreyId = c.m_PreyId;
	m_PreyName = c.m_PreyName;
	m_Reward = c.m_Reward;
}

void AuJobAssassin::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_LastSeen;
	in >> m_PreyId;
	in >> m_PreyName;
	in >> m_Reward;
}

void AuJobAssassin::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_LastSeen;
	out << m_PreyId;
	out << m_PreyName;
	out << m_Reward;
}

AuJob* AuJobAssassin::Clone() const
{
	return new AuJobAssassin(*this);
}

bool AuJobAssassin::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobAssassin* cm = dynamic_cast<const AuJobAssassin*>(c);
	if(!cm)
		return false;

	if(m_LastSeen != cm->m_LastSeen)
		return false;
	if(m_PreyId != cm->m_PreyId)
		return false;
	if(m_PreyName != cm->m_PreyName)
		return false;
	if(m_Reward != cm->m_Reward)
		return false;
	return true;
}

QString AuJobAssassin::IconName() const
{
	return QString(":data/JobAssassin.png");
}

bool AuJobAssassin::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

AuJob::TYPE AuJobAssassin::Type() const
{
	return JobType();
}

QString AuJobAssassin::TypeName()
{
	return QObject::tr("Assassinate Politician");
}

bool AuJobAssassin::DoesReplace(const AuJob* job) const
{
	const AuJobAssassin* otherJob = dynamic_cast<const AuJobAssassin*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_PreyId == otherJob->PreyId();
}

QString AuJobAssassin::Title() const
{
	return QObject::tr("%1 Assassination Contract").arg(m_PreyName);
}

QString AuJobAssassin::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("I am looking for somebody to assassinate a politician.");
}

QString AuJobAssassin::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("A reliable informant at %3 spotted %1 on %2. "
			"You would be paid %L4%5 to kill %1, discretely. "
			"If you get caught, we never met. ;-)")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward)
		.arg(g_Au.CurrencySymbol());
}

QString AuJobAssassin::Brief() const
{
	return QObject::tr("<p>Assassinate %1.</p>"
			"<p>The target is a politician, last seen %2, %3.</p>"
			"<p>This contract pays %L4%5 when %1 is eliminated.</p>"
			"<p>Leave the area discretely after you are finished. "
			"Diplomats will handle any political repercussions.</p>"
			"<p>If you are captured or killed, the secretary will disavow any knowledge of your actions.</p>")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward)
		.arg(g_Au.CurrencySymbol());
}

bool AuJobAssassin::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	Q_UNUSED(sectorRange);
	if(toPerson.Is(m_PreyId))
		return false;

	// Do not offer jobs from empires they have not encountered.
	const DgData& gd = g_Au.LockDataForRead();
	bool isKnown = gd.IsEmpireKnown(fromEmpireId, toPerson.EmpireId());
	g_Au.UnlockData();
	return isKnown;
}

int AuJobAssassin::PersonKilled(DaPerson* person, DaPerson* victim)
{
	if(!victim->Is(m_PreyId))
		return 0;

	//DHH Magic money. The fee for the assassination does not come from anywhere.
	person->CreditAccount(m_Reward);

	QString text = QObject::tr("Target %1 eliminated! My account was credited %L2%3, per the contract.")
		.arg(m_PreyName)
		.arg(m_Reward)
		.arg(g_Au.CurrencySymbol());
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobAssassin::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += StringMemoryUsage(m_LastSeen);
	mem += sizeof(m_PreyId);
	mem += StringMemoryUsage(m_PreyName);
	mem += sizeof(m_Reward);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobBountyHunter::AuJobBountyHunter(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward,
		const QStringList& reasons) :
	AuJob(AuTime::Now()), m_Reasons(reasons)
{
	m_LastSeen = lastSeen;
	m_PreyId = preyId;
	m_PreyName = preyName;
	m_Reward = reward;
}

AuJobBountyHunter::AuJobBountyHunter()
{
	m_Reward = 0;
}

AuJobBountyHunter::AuJobBountyHunter(const AuJobBountyHunter& c) : AuJob(c), m_Reasons(c.m_Reasons)
{
	m_LastSeen = c.m_LastSeen;
	m_PreyId = c.m_PreyId;
	m_PreyName = c.m_PreyName;
	m_Reward = c.m_Reward;
}

void AuJobBountyHunter::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_LastSeen;
	in >> m_PreyId;
	in >> m_PreyName;
	in >> m_Reasons;
	in >> m_Reward;
}

void AuJobBountyHunter::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_LastSeen;
	out << m_PreyId;
	out << m_PreyName;
	out << m_Reasons;
	out << m_Reward;
}

bool AuJobBountyHunter::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobBountyHunter* cm = dynamic_cast<const AuJobBountyHunter*>(c);
	if(!cm)
		return false;

	if(m_LastSeen != cm->m_LastSeen)
		return false;
	if(m_PreyId != cm->m_PreyId)
		return false;
	if(m_PreyName != cm->m_PreyName)
		return false;
	if(m_Reward != cm->m_Reward)
		return false;
	return true;
}

QString AuJobBountyHunter::IconName() const
{
	return QString(":data/JobBountyHunter.png");
}

bool AuJobBountyHunter::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

QString AuJobBountyHunter::Title() const
{
	return QObject::tr("%1 Wanted!").arg(m_PreyName);
}

bool AuJobBountyHunter::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	Q_UNUSED(sectorRange);
	// Do not offer jobs from empires they have not encountered.
	const DgData& gd = g_Au.LockDataForRead();
	bool isKnown = gd.IsEmpireKnown(fromEmpireId, toPerson.EmpireId());
	g_Au.UnlockData();
	return isKnown;
}

quint64 AuJobBountyHunter::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += StringMemoryUsage(m_LastSeen);
	mem += sizeof(m_PreyId);
	mem += StringMemoryUsage(m_PreyName);
	for(QStringList::const_iterator ri = m_Reasons.constBegin(); ri != m_Reasons.constEnd(); ri++)
		mem += StringMemoryUsage(*ri);
	mem += sizeof(m_Reward);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobBountyHunterPerson::AuJobBountyHunterPerson(const AuId& preyId, const QString& preyName, const QString& lastSeen,
		double reward, const QStringList& reasons) :
	AuJobBountyHunter(preyId, preyName, lastSeen, reward, reasons)
{
}

AuJobBountyHunterPerson::AuJobBountyHunterPerson()
{
}

AuJobBountyHunterPerson::AuJobBountyHunterPerson(const AuJobBountyHunterPerson& c) : AuJobBountyHunter(c)
{
}

bool AuJobBountyHunterPerson::Compare(const AuJob* c)
{
	if(!AuJobBountyHunter::Compare(c))
		return false;
	const AuJobBountyHunterPerson* cm = dynamic_cast<const AuJobBountyHunterPerson*>(c);
	if(!cm)
		return false;

	return true;
}

AuJob* AuJobBountyHunterPerson::Clone() const
{
	return new AuJobBountyHunterPerson(*this);
}

AuJob::TYPE AuJobBountyHunterPerson::Type() const
{
	return JobType();
}

QString AuJobBountyHunterPerson::TypeName()
{
	return QObject::tr("Bounty Hunt Person");
}

bool AuJobBountyHunterPerson::DoesReplace(const AuJob* job) const
{
	const AuJobBountyHunterPerson* otherJob = dynamic_cast<const AuJobBountyHunterPerson*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_PreyId == otherJob->PreyId();
}

QString AuJobBountyHunterPerson::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("I am looking for a bounty hunter to kill someone.");
}

QString AuJobBountyHunterPerson::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;
	text = QObject::tr("%1 has a price on their head, currently at %L4%5. "
			"I received a tip from a friend at %3; they spotted %1 on %2.")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobBountyHunterPerson::Brief() const
{
	QString text;
	text = QObject::tr("<p>A bounty was posted for the head of %1.</p>"
			"<p>All posted bounties will be paid when %1 is killed. "
			"Total bounty is currently %L4%5.</p>"
			"<p>%1 was last seen %2, %3. "
			"Move quickly; other bounty hunters could already be on the trail.</p>")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	text += QObject::tr("<p>Bounty kills do not lower your reputation.</p>");

	if(!m_Reasons.isEmpty())
	{
		text += QObject::tr("Here's why the bounty was posted.");
		for(QStringList::const_iterator ri = m_Reasons.constBegin(); ri != m_Reasons.constEnd(); ri++)
			text += QString("<br> ") + *ri;
	}

	return text;
}

bool AuJobBountyHunterPerson::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	if(toPerson.Is(m_PreyId))
		return false;

	if(!AuJobBountyHunter::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}

int AuJobBountyHunterPerson::PersonKilled(DaPerson* person, DaPerson* victim)
{
	if(!victim->Is(m_PreyId))
		return 0;

	double bounty = victim->TakeBounty();

	// Increase the person's reputation so they don't lose a point by killing a friendly or fellow empire citizen.
	person->IncreaseReputation(1);

	QString text;
	if(bounty > 0)
	{
		person->CreditAccount(bounty);
		text = QObject::tr("%1 killed! The posted bounty of %L2%3 was transferred to my personal account.")
			.arg(m_PreyName)
			.arg(bounty, 0, 'f', 0)
			.arg(g_Au.CurrencySymbol());
	}
	else
	{
		text = QObject::tr("%1 killed! Unfortunately, no bounty was paid. "
				"Another bounty hunter must have claimed it already.")
			.arg(m_PreyName);
	}
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobBountyHunterShip::AuJobBountyHunterShip(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward,
		const QStringList& reasons) :
	AuJobBountyHunter(preyId, preyName, lastSeen, reward, reasons)
{
}

AuJobBountyHunterShip::AuJobBountyHunterShip()
{
}

AuJobBountyHunterShip::AuJobBountyHunterShip(const AuJobBountyHunterShip& c) : AuJobBountyHunter(c)
{
}

bool AuJobBountyHunterShip::Compare(const AuJob* c)
{
	if(!AuJobBountyHunter::Compare(c))
		return false;
	const AuJobBountyHunterShip* cm = dynamic_cast<const AuJobBountyHunterShip*>(c);
	if(!cm)
		return false;

	return true;
}

AuJob* AuJobBountyHunterShip::Clone() const
{
	return new AuJobBountyHunterShip(*this);
}

AuJob::TYPE AuJobBountyHunterShip::Type() const
{
	return JobType();
}

QString AuJobBountyHunterShip::TypeName()
{
	return QObject::tr("Bounty Hunt Spacecraft");
}

bool AuJobBountyHunterShip::DoesReplace(const AuJob* job) const
{
	const AuJobBountyHunterShip* otherJob = dynamic_cast<const AuJobBountyHunterShip*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_PreyId == otherJob->PreyId();
}

QString AuJobBountyHunterShip::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("I am looking for a bounty hunter to destroy a ship.");
}

QString AuJobBountyHunterShip::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;
	text = QObject::tr("%1 was spotted at %3 on %2. "
			"There is a bounty of %L4%5 for its destruction. "
			"You'll get paid when the ship is destroyed.")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobBountyHunterShip::Brief() const
{
	QString text;
	text = QObject::tr("<p>%1 has a price on its hull.</p>"
			"<p>You will collect the posted bounty when the ship is destroyed. "
			"The posted bounty is currently %L4%5.</p>"
			"<p>The ship was last seen %2, %3. Move quickly; other bounty hunters could already be on the trail.</p>")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	text += QObject::tr("<p>That ship is a scourge of the spaceways.</p>");

	if(!m_Reasons.isEmpty())
	{
		text += QObject::tr("Here's why the bounty was posted.");
		for(QStringList::const_iterator ri = m_Reasons.constBegin(); ri != m_Reasons.constEnd(); ri++)
			text += QString("<br> ") + *ri;
	}

	return text;
}

bool AuJobBountyHunterShip::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	if(toPerson.SpawnShip() == m_PreyId)
		return false;

	if(!AuJobBountyHunter::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}

int AuJobBountyHunterShip::SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& /*inSystemId*/)
{
	if(!ship->Is(m_PreyId))
		return 0;

	double bounty = ship->TakeBounty();

	QString text;
	if(bounty > 0)
	{
		person->CreditAccount(bounty);
		text = QObject::tr("%1 destroyed! The posted bounty of %L2%3 was transferred to my personal account.")
			.arg(m_PreyName)
			.arg(bounty, 0, 'f', 0)
			.arg(g_Au.CurrencySymbol());
	}
	else
	{
		text = QObject::tr("%1 destroyed! Unfortunately, no bounty was paid. "
				"Another bounty hunter must have claimed it already.")
			.arg(m_PreyName);
	}
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobBountyHunterVehicle::AuJobBountyHunterVehicle(const AuId& preyId, const QString& preyName, const QString& lastSeen,
		double reward, const QStringList& reasons) :
	AuJobBountyHunter(preyId, preyName, lastSeen, reward, reasons)
{
}

AuJobBountyHunterVehicle::AuJobBountyHunterVehicle()
{
}

AuJobBountyHunterVehicle::AuJobBountyHunterVehicle(const AuJobBountyHunterVehicle& c) : AuJobBountyHunter(c)
{
}

bool AuJobBountyHunterVehicle::Compare(const AuJob* c)
{
	if(!AuJobBountyHunter::Compare(c))
		return false;
	const AuJobBountyHunterVehicle* cm = dynamic_cast<const AuJobBountyHunterVehicle*>(c);
	if(!cm)
		return false;

	return true;
}

AuJob* AuJobBountyHunterVehicle::Clone() const
{
	return new AuJobBountyHunterVehicle(*this);
}

AuJob::TYPE AuJobBountyHunterVehicle::Type() const
{
	return JobType();
}

QString AuJobBountyHunterVehicle::TypeName()
{
	return QObject::tr("Bounty Hunt Vehicle");
}

bool AuJobBountyHunterVehicle::DoesReplace(const AuJob* job) const
{
	const AuJobBountyHunterVehicle* otherJob = dynamic_cast<const AuJobBountyHunterVehicle*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_PreyId == otherJob->PreyId();
}

QString AuJobBountyHunterVehicle::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("I am looking for a bounty hunter to destroy a vehicle.");
}

QString AuJobBountyHunterVehicle::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;
	text = QObject::tr("%1 has a reward for its destruction of %L4%5. "
			"The vehicle was spotted at %3 on %2. "
			"Destroy it and you collect the money; it's that simple.")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobBountyHunterVehicle::Brief() const
{
	QString text;
	text = QObject::tr("<p>%1 has a reward for destroying it.</p>"
			"<p>You will collect the posted bounty when it is destroyed, currently %L4%5.</p>"
			"<p>The vehicle was last seen %2, %3. Move quickly; other bounty hunters could already be on the trail.</p>")
		.arg(m_PreyName)
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_LastSeen)
		.arg(m_Reward, 0,'f',0)
		.arg(g_Au.CurrencySymbol());

	text += QObject::tr("<p>Destroy it ASAP!</p>");

	if(!m_Reasons.isEmpty())
	{
		text += QObject::tr("Here's why the bounty was posted.");
		for(QStringList::const_iterator ri = m_Reasons.constBegin(); ri != m_Reasons.constEnd(); ri++)
			text += QString("<br> ") + *ri;
	}

	return text;
}

bool AuJobBountyHunterVehicle::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	if(toPerson.ParentId() == m_PreyId)
		return false;

	if(!AuJobBountyHunter::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}
	
int AuJobBountyHunterVehicle::VehicleDestroyed(DaPerson* person, DaVehicle* vehicle)
{
	if(!vehicle->Is(m_PreyId))
		return 0;

	double bounty = vehicle->TakeBounty();

	QString text;
	if(bounty > 0)
	{
		person->CreditAccount(bounty);
		text = QObject::tr("%1 destroyed! The posted bounty of %L2%3 was transferred to my personal account.")
			.arg(m_PreyName)
			.arg(bounty, 0, 'f', 0)
			.arg(g_Au.CurrencySymbol());
	}
	else
	{
		text = QObject::tr("%1 destroyed! Unfortunately, no bounty was paid. "
				"Another bounty hunter must have claimed it already.")
			.arg(m_PreyName);
	}
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargo::AuJobDeliverCargo(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
		const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount, quint32 pricePerUnit)
	: AuJob(AuTime::Now())
{
	Q_ASSERT(minQuality);
	m_CID = cid;
	m_CityId = cityId;
	m_CityName = cityName;
	m_MinCount = minCount;
	m_MinQuality = minQuality;
	m_PricePerUnit = pricePerUnit;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobDeliverCargo::AuJobDeliverCargo()
{
	m_CID = CID_Invalid;
	m_MinCount = 0;
	m_MinQuality = 1;
	m_PricePerUnit = 0;
}

AuJobDeliverCargo::AuJobDeliverCargo(const AuJobDeliverCargo& c) : AuJob(c)
{
	m_CID = c.m_CID;
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_MinCount = c.m_MinCount;
	m_MinQuality = c.m_MinQuality;
	m_PricePerUnit = c.m_PricePerUnit;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobDeliverCargo::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << (quint16)m_CID;
	out << m_CityId;
	out << m_CityName;
	out << m_MinCount;
	out << m_MinQuality;
	quint8 mintl = 0;
	out << mintl;
	out << m_PricePerUnit;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
}

void AuJobDeliverCargo::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);

	quint16 ui16;
	in >> ui16;
	m_CID = (COMMODITYID)ui16;
	in >> m_CityId;
	in >> m_CityName;
	in >> m_MinCount;
	in >> m_MinQuality;
	if(!m_MinQuality)
		m_MinQuality = 1;
	quint8 mintl;
	in >> mintl;
	in >> m_PricePerUnit;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
}

bool AuJobDeliverCargo::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverCargo* cm = dynamic_cast<const AuJobDeliverCargo*>(c);
	if(!cm)
		return false;

	if(m_CID != cm->m_CID)
		return false;
	if(m_CityId != cm->m_CityId)
		return false;
	if(m_CityName != cm->m_CityName)
		return false;
	if(m_MinCount != cm->m_MinCount)
		return false;
	if(m_MinQuality != cm->m_MinQuality)
		return false;
	if(m_PricePerUnit != cm->m_PricePerUnit)
		return false;
	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

bool AuJobDeliverCargo::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

bool AuJobDeliverCargo::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	if(!AuJob::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}

int AuJobDeliverCargo::CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item)
{
	if(cityId != m_CityId)
		return 0;
	if(item.CommodityId() != m_CID)
		return 0;
	if(item.Quality() < m_MinQuality)
		return 0;

	int countDelivered;
	if(item.Count() > m_MinCount)
	{
		countDelivered = m_MinCount;
		m_MinCount = 0;											// Reduce the count needed to zero.
	}
	else
	{
		countDelivered = item.Count();
		m_MinCount -= item.Count();								// Reduce the count needed by the count provided.
	}

	QString text = QObject::tr("The locals thanked me for delivering %L1 %2.")
		.arg(countDelivered)
		.arg(item.SayName());

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * countDelivered;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" As agreed, they paid my personal account an extra %L3%4 per unit for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0, 'f', 0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text += QObject::tr(" They were short on the promised reward. %L3%4 was deposited into my personal account. "
					"It was all they had.")
				.arg(city->CommunityAccountBalance(), 0,'f',0)
				.arg(g_Au.CurrencySymbol());

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text += QObject::tr(" Unfortunately they do not have any money to pay the promised reward.");
	}
	else
		text += QObject::tr(" They are very grateful that I brought it to them for no extra charge.");

	if(m_MinCount)
		text += QObject::tr(" They are still expecting %1 more units.").arg(m_MinCount);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	// This job is done if the count needed has been reduced to zero.
	return (m_MinCount == 0) ? 2 : 1;
}

quint64 AuJobDeliverCargo::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobDeliverCargo) - sizeof(AuJob);
	mem += StringMemoryUsage(m_CityName);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoGasping::AuJobDeliverCargoGasping(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoGasping::AuJobDeliverCargoGasping()
{
}

AuJobDeliverCargoGasping::AuJobDeliverCargoGasping(const AuJobDeliverCargoGasping& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoGasping::Clone() const
{
	return new AuJobDeliverCargoGasping(*this);
}

bool AuJobDeliverCargoGasping::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoGasping* cm = dynamic_cast<const AuJobDeliverCargoGasping*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoGasping::IconName() const
{
	return QString(":data/dp_LifeSupport.png");
}

AuJob::TYPE AuJobDeliverCargoGasping::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoGasping::TypeName()
{
	return QObject::tr("Haul Air, People Gasping");
}

bool AuJobDeliverCargoGasping::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoGasping* otherJob = dynamic_cast<const AuJobDeliverCargoGasping*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoGasping::Title() const
{
	return QObject::tr("%1 People Are Gasping For Air").arg(m_CityName);
}

QString AuJobDeliverCargoGasping::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("People are gasping for air in %1.")
		.arg(m_CityName);
}

QString AuJobDeliverCargoGasping::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("The citizens of %2 are gasping for air. "
			"The city is in the %3 system; it's in the %4 sector. "
			"Take a load of %1 to them; they're getting nervous.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They're willing to pay an extra %L1%2 per unit.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoGasping::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>People are gasping for breath. The air smells stale. ");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoSuffocating::AuJobDeliverCargoSuffocating(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoSuffocating::AuJobDeliverCargoSuffocating()
{
}

AuJobDeliverCargoSuffocating::AuJobDeliverCargoSuffocating(const AuJobDeliverCargoSuffocating& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoSuffocating::Clone() const
{
	return new AuJobDeliverCargoSuffocating(*this);
}

bool AuJobDeliverCargoSuffocating::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoSuffocating* cm = dynamic_cast<const AuJobDeliverCargoSuffocating*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoSuffocating::IconName() const
{
	return QString(":data/dp_LifeSupport.png");
}

AuJob::TYPE AuJobDeliverCargoSuffocating::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoSuffocating::TypeName()
{
	return QObject::tr("Haul Air, People Suffocating");
}

bool AuJobDeliverCargoSuffocating::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoSuffocating* otherJob = dynamic_cast<const AuJobDeliverCargoSuffocating*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoSuffocating::Title() const
{
	return QObject::tr("%1 People Are Suffocating!").arg(m_CityName);
}

QString AuJobDeliverCargoSuffocating::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("People are suffocating in %1 because they are out of air!")
		.arg(m_CityName);
}

QString AuJobDeliverCargoSuffocating::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("This is an emergency! People are dying. "
			"You have to take %1 to %2 right now. "
			"The city is in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units right away.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They said they'll pay an extra %L1%2 per unit.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	text += QObject::tr(" Hurry!");

	return text;
}

QString AuJobDeliverCargoSuffocating::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Quickly! People are suffocating to death.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed right away.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoRats::AuJobDeliverCargoRats(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoRats::AuJobDeliverCargoRats()
{
}

AuJobDeliverCargoRats::AuJobDeliverCargoRats(const AuJobDeliverCargoRats& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoRats::Clone() const
{
	return new AuJobDeliverCargoRats(*this);
}

bool AuJobDeliverCargoRats::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoRats* cm = dynamic_cast<const AuJobDeliverCargoRats*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoRats::IconName() const
{
	return QString(":data/Rat.png");
}

AuJob::TYPE AuJobDeliverCargoRats::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoRats::TypeName()
{
	return QObject::tr("Haul Pharmaceuticals, Vermin Infestation");
}

bool AuJobDeliverCargoRats::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoRats* otherJob = dynamic_cast<const AuJobDeliverCargoRats*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoRats::Title() const
{
	return QObject::tr("%1 Infested With Vermin").arg(m_CityName);
}

QString AuJobDeliverCargoRats::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("Rats have infested the city of %1. They need %1.")
		.arg(m_CityName)
		.arg(g_Au.CommodityName(m_CID));
}

QString AuJobDeliverCargoRats::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("Rats have infested the food stores in %2. "
			"They need someone to bring lots of %1. "
			"The city is in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" At least %L1 units are needed to poison them.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They'll pay an extra %L1%2 per unit.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoRats::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Vermin are eating the food stores.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed to poison them.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoDisease::AuJobDeliverCargoDisease(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoDisease::AuJobDeliverCargoDisease()
{
}

AuJobDeliverCargoDisease::AuJobDeliverCargoDisease(const AuJobDeliverCargoDisease& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoDisease::Clone() const
{
	return new AuJobDeliverCargoDisease(*this);
}

bool AuJobDeliverCargoDisease::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoDisease* cm = dynamic_cast<const AuJobDeliverCargoDisease*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoDisease::IconName() const
{
	return QString(":data/EventOutbreak.png");
}

AuJob::TYPE AuJobDeliverCargoDisease::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoDisease::TypeName()
{
	return QObject::tr("Haul Antitoxins, Disease Outbreak");
}

bool AuJobDeliverCargoDisease::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoDisease* otherJob = dynamic_cast<const AuJobDeliverCargoDisease*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoDisease::Title() const
{
	return QObject::tr("%1 Disease Outbreak!").arg(m_CityName);
}

QString AuJobDeliverCargoDisease::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("There has been an outbreak of disease in %2. The city needs %1.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName);
}

QString AuJobDeliverCargoDisease::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("Some kind of epidemic has broken out in %2. "
			"They need %1 ASAP! "
			"The city is in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units to cure everybody.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" These people are desperate; they're paying an extra %L1%2 per unit.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoDisease::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Outbreak! Disease is ravaging the population. "
			"A lot of people are sick; some are dying.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed to cure everyone who is infected.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoPoison::AuJobDeliverCargoPoison(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoPoison::AuJobDeliverCargoPoison()
{
}

AuJobDeliverCargoPoison::AuJobDeliverCargoPoison(const AuJobDeliverCargoPoison& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoPoison::Clone() const
{
	return new AuJobDeliverCargoPoison(*this);
}

bool AuJobDeliverCargoPoison::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoPoison* cm = dynamic_cast<const AuJobDeliverCargoPoison*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoPoison::IconName() const
{
	return QString(":data/EventPoison.png");
}

AuJob::TYPE AuJobDeliverCargoPoison::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoPoison::TypeName()
{
	return QObject::tr("Haul Antidotes, Poisoning");
}

bool AuJobDeliverCargoPoison::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoPoison* otherJob = dynamic_cast<const AuJobDeliverCargoPoison*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoPoison::Title() const
{
	return QObject::tr("%1 Poisoned").arg(m_CityName);
}

QString AuJobDeliverCargoPoison::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("There has been a mass poisoning in %2. The city needs %1.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName);
}

QString AuJobDeliverCargoPoison::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("There's been a mass poisoning in %2. "
			"They need someone to bring a bunch of %1 to them in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They are asking for %L1 units.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They offered an extra %L1%2 per unit as an incentive.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoPoison::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Poisoning has run rampant, perhaps from food received in the last shipment. "
			"Lots of people are heaving their guts out. "
			"Nobody wants to eat anything.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoSpacecraftFactory::AuJobDeliverCargoSpacecraftFactory(const AuId& cityId, const QString& cityName,
		const AuId& systemId, const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoSpacecraftFactory::AuJobDeliverCargoSpacecraftFactory()
{
}

AuJobDeliverCargoSpacecraftFactory::AuJobDeliverCargoSpacecraftFactory(const AuJobDeliverCargoSpacecraftFactory& c) :
	AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoSpacecraftFactory::Clone() const
{
	return new AuJobDeliverCargoSpacecraftFactory(*this);
}

bool AuJobDeliverCargoSpacecraftFactory::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoSpacecraftFactory* cm = dynamic_cast<const AuJobDeliverCargoSpacecraftFactory*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoSpacecraftFactory::IconName() const
{
	return QString(":data/c_Spacecraft.png");
}

AuJob::TYPE AuJobDeliverCargoSpacecraftFactory::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoSpacecraftFactory::TypeName()
{
	return QObject::tr("Haul Parts, Spacecraft Production");
}

bool AuJobDeliverCargoSpacecraftFactory::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoSpacecraftFactory* otherJob = dynamic_cast<const AuJobDeliverCargoSpacecraftFactory*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoSpacecraftFactory::Title() const
{
	return QObject::tr("%1 Spacecraft Production Halted").arg(m_CityName);
}

QString AuJobDeliverCargoSpacecraftFactory::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("Spacecraft production is halted in %2. The city needs %1.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName);
}

QString AuJobDeliverCargoSpacecraftFactory::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("Spacecraft production in %2 is on hold, waiting for parts. "
			"You could save the day by taking a load of %1 to them in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units to get going again.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" This is important so they're paying an extra %L1%2 per unit.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoSpacecraftFactory::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Spacecraft production is halted until these essential parts arrive.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed to continue production.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoConstruction::AuJobDeliverCargoConstruction(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
}

AuJobDeliverCargoConstruction::AuJobDeliverCargoConstruction()
{
}

AuJobDeliverCargoConstruction::AuJobDeliverCargoConstruction(const AuJobDeliverCargoConstruction& c) : AuJobDeliverCargo(c)
{
}

AuJob* AuJobDeliverCargoConstruction::Clone() const
{
	return new AuJobDeliverCargoConstruction(*this);
}

bool AuJobDeliverCargoConstruction::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoConstruction* cm = dynamic_cast<const AuJobDeliverCargoConstruction*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoConstruction::IconName() const
{
	return QString(":data/bp_ClearLand.png");
}

AuJob::TYPE AuJobDeliverCargoConstruction::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoConstruction::TypeName()
{
	return QObject::tr("Haul Construction Materials");
}

bool AuJobDeliverCargoConstruction::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoConstruction* otherJob = dynamic_cast<const AuJobDeliverCargoConstruction*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoConstruction::Title() const
{
	return QObject::tr("%1 Construction Halted").arg(m_CityName);
}

QString AuJobDeliverCargoConstruction::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("Building construction is halted in %2. The city needs %1.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName);
}

QString AuJobDeliverCargoConstruction::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("A construction company in %2 has put out the call for materials. "
			"They need somebody to bring a load of %1 to them in the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They said they need %L1 units.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They offered an extra %L1%2 per unit to get it there pronto.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoConstruction::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Building is on hold until more construction materials get here.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed to continue building.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverCargoManufacture::AuJobDeliverCargoManufacture(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, COMMODITYID cid, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit, COMMODITYID cidStopped) :
	AuJobDeliverCargo(cityId, cityName, systemId, systemName, sectorName, cid, minQuality, minCount, pricePerUnit)
{
	m_CIDStopped = cidStopped;
}

AuJobDeliverCargoManufacture::AuJobDeliverCargoManufacture()
{
	m_CIDStopped = CID_Invalid;
}

AuJobDeliverCargoManufacture::AuJobDeliverCargoManufacture(const AuJobDeliverCargoManufacture& c) : AuJobDeliverCargo(c)
{
	m_CIDStopped = c.m_CIDStopped;
}

void AuJobDeliverCargoManufacture::operator<<(QDataStream& in)
{
	AuJobDeliverCargo::operator<<(in);

	quint16 ui16;

	in >> ui16;
	m_CIDStopped = (COMMODITYID)ui16;
}

void AuJobDeliverCargoManufacture::operator>>(QDataStream& out) const
{
	AuJobDeliverCargo::operator>>(out);
	out << (quint16)m_CIDStopped;
}

AuJob* AuJobDeliverCargoManufacture::Clone() const
{
	return new AuJobDeliverCargoManufacture(*this);
}

bool AuJobDeliverCargoManufacture::Compare(const AuJob* c)
{
	if(!AuJobDeliverCargo::Compare(c))
		return false;
	const AuJobDeliverCargoManufacture* cm = dynamic_cast<const AuJobDeliverCargoManufacture*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverCargoManufacture::IconName() const
{
	if(m_CIDStopped > CID_Invalid && m_CIDStopped < CID_Count)
		return g_Au.CommodityBitmapName(m_CIDStopped);
	return QString(":data/HammerAndSickle.png");
}

AuJob::TYPE AuJobDeliverCargoManufacture::Type() const
{
	return JobType();
}

QString AuJobDeliverCargoManufacture::TypeName()
{
	return QObject::tr("Haul Manufacturing Components");
}

bool AuJobDeliverCargoManufacture::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverCargoManufacture* otherJob = dynamic_cast<const AuJobDeliverCargoManufacture*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverCargoManufacture::Title() const
{
	if(m_CIDStopped != CID_Invalid)
		return QObject::tr("%1 Manufacturing %2 Halted")
			.arg(m_CityName)
			.arg(g_Au.CommodityName(m_CIDStopped));
	else
		return QObject::tr("%1 Manufacturing Halted")
			.arg(m_CityName);
}

QString AuJobDeliverCargoManufacture::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("%3 manufacturing is on hold in %2. The city needs %1.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(g_Au.CommodityName(m_CIDStopped));
}

QString AuJobDeliverCargoManufacture::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("A factory in %2 is short of materials. "
			"They need %1 yesterday. "
			"Someone needs to make a run to the %3 system, %4.")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_CIDStopped != CID_Invalid)
		text += QObject::tr(" There's a work stoppage on making %1 until they get the stuff.")
			.arg(g_Au.CommodityName(m_CIDStopped));
	else
		text += QObject::tr(" There's a general work stoppage until they get the stuff.");

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units to get rolling again.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" It must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" The boss authorized an extra %L1%2 per unit upon delivery.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverCargoManufacture::Brief() const
{
	QString text = QObject::tr("<p>Haul %1 to %2; located in the %3 system, %4.</p>")
		.arg(g_Au.CommodityName(m_CID))
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_CIDStopped != CID_Invalid)
		text += QObject::tr("<p>%1 manufacturing is on hold until the city gets these components.")
			.arg(g_Au.CommodityName(m_CIDStopped));
	else
		text += QObject::tr("<p>Manufacturing is on hold until the city gets these components.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed to continue manufacturing.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The %1 must be at least %2 Q%3.")
			.arg(g_Au.CommodityName(m_CID))
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

quint64 AuJobDeliverCargoManufacture::MemoryUsage() const
{
	quint64 mem = AuJobDeliverCargo::MemoryUsage();
	mem += sizeof(AuJobDeliverCargoManufacture) - sizeof(AuJobDeliverCargo);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverPassengers::AuJobDeliverPassengers(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, quint32 minCount, quint32 pricePerUnit) :
	AuJob(AuTime::Now())
{
	m_CityId = cityId;
	m_CityName = cityName;
	m_MinCount = minCount;
	m_PricePerUnit = pricePerUnit;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobDeliverPassengers::AuJobDeliverPassengers()
{
	m_MinCount = 0;
	m_PricePerUnit = 0;
}

AuJobDeliverPassengers::AuJobDeliverPassengers(const AuJobDeliverPassengers& c) : AuJob(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_MinCount = c.m_MinCount;
	m_PricePerUnit = c.m_PricePerUnit;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobDeliverPassengers::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_CityId;
	in >> m_CityName;
	in >> m_MinCount;
	in >> m_PricePerUnit;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuJobDeliverPassengers::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_CityId;
	out << m_CityName;
	out << m_MinCount;
	out << m_PricePerUnit;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
}

bool AuJobDeliverPassengers::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverPassengers* cm = dynamic_cast<const AuJobDeliverPassengers*>(c);
	if(!cm)
		return false;

	if(m_CityId != cm->m_CityId)
		return false;
	if(m_CityName != cm->m_CityName)
		return false;
	if(m_MinCount != cm->m_MinCount)
		return false;
	if(m_PricePerUnit != cm->m_PricePerUnit)
		return false;
	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

QString AuJobDeliverPassengers::IconName() const
{
	return QString(":data/Passenger.png");
}

bool AuJobDeliverPassengers::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

bool AuJobDeliverPassengers::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	// Person must be in command of a ship with berths.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	if(!AuJob::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}

int AuJobDeliverPassengers::PassengersDelivered(DaPerson* person, const AuId& cityId, quint16 count)
{
	if(cityId != m_CityId)
		return 0;

	int countDelivered;
	if(count > m_MinCount)
	{
		countDelivered = m_MinCount;
		m_MinCount = 0;											// Reduce the count needed to zero.
	}
	else
	{
		countDelivered = count;
		m_MinCount -= count;									// Reduce the count needed by the count provided.
	}

	QString text = QObject::tr("The locals thanked me for bringing %L1 people.").arg(countDelivered);

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * countDelivered;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" As agreed, they paid my personal account an extra %L3%4 per person for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0,'f',0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text += QObject::tr(" They were embarrassed; they no longer had the full amount of the reward that was offered. "
					"They deposited %L1%2 into my personal account. It was all the money they had left.")
				.arg(city->CommunityAccountBalance(), 0,'f',0)
				.arg(g_Au.CurrencySymbol());

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text += QObject::tr(" Unfortunately they are completely broke. They cannot pay the reward that was offered.");
	}
	else
		text += QObject::tr(" They are very grateful to me for bringing people for no extra charge.");

	if(m_MinCount)
		text += QObject::tr(" They are still hoping for %1 more people.").arg(m_MinCount);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	// This job is done if the count needed has been reduced to zero.
	return (m_MinCount == 0) ? 2 : 1;
}

quint64 AuJobDeliverPassengers::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(m_CityId);
	mem += StringMemoryUsage(m_CityName);
	mem += sizeof(m_MinCount);
	mem += sizeof(m_PricePerUnit);
	mem += StringMemoryUsage(m_SectorName);
	mem += sizeof(m_SystemId);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverPassengersColonists::AuJobDeliverPassengersColonists(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverPassengers(cityId, cityName, systemId, systemName, sectorName, minCount, pricePerUnit)
{
}

AuJobDeliverPassengersColonists::AuJobDeliverPassengersColonists()
{
}

AuJobDeliverPassengersColonists::AuJobDeliverPassengersColonists(const AuJobDeliverPassengersColonists& c) :
	AuJobDeliverPassengers(c)
{
}

AuJob* AuJobDeliverPassengersColonists::Clone() const
{
	return new AuJobDeliverPassengersColonists(*this);
}

bool AuJobDeliverPassengersColonists::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverPassengersColonists* cm = dynamic_cast<const AuJobDeliverPassengersColonists*>(c);
	if(!cm)
		return false;

	return true;
}

AuJob::TYPE AuJobDeliverPassengersColonists::Type() const
{
	return JobType();
}

QString AuJobDeliverPassengersColonists::TypeName()
{
	return QObject::tr("Transport Colonists");
}

bool AuJobDeliverPassengersColonists::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverPassengersColonists* otherJob = dynamic_cast<const AuJobDeliverPassengersColonists*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverPassengersColonists::Title() const
{
	return QObject::tr("%1 Needs Colonists").arg(m_CityName);
}

QString AuJobDeliverPassengersColonists::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("There are lots of empty homes in %1. They need colonists badly.")
		.arg(m_CityName);
}

QString AuJobDeliverPassengersColonists::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("The city of %1 is in trouble; it's way over built. "
			"There's lots of empty homes but hardly anybody lives there. "
			"The place is going to disintegrate without more people to maintain the buildings. "
			"Someone needs to bring colonists to them in the %2 system, %3 soon.")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 more people.").arg(m_MinCount);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They're paying an extra bonus of %L1%2 per colonist that you drop off.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverPassengersColonists::Brief() const
{
	QString text = QObject::tr("<p>Transport colonists to %1, located in the %2 system, %3.</p>")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>The city is too big. "
			"There are too many homes. "
			"The city will crumble away without more people there to maintain the buildings.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 people are needed.").arg(m_MinCount);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per person when you offload them.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverPassengersDeserted::AuJobDeliverPassengersDeserted(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverPassengers(cityId, cityName, systemId, systemName, sectorName, minCount, pricePerUnit)
{
}

AuJobDeliverPassengersDeserted::AuJobDeliverPassengersDeserted()
{
}

AuJobDeliverPassengersDeserted::AuJobDeliverPassengersDeserted(const AuJobDeliverPassengersDeserted& c) : AuJobDeliverPassengers(c)
{
}

AuJob* AuJobDeliverPassengersDeserted::Clone() const
{
	return new AuJobDeliverPassengersDeserted(*this);
}

bool AuJobDeliverPassengersDeserted::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverPassengersDeserted* cm = dynamic_cast<const AuJobDeliverPassengersDeserted*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverPassengersDeserted::IconName() const
{
	return QString(":data/City.png");
}

AuJob::TYPE AuJobDeliverPassengersDeserted::Type() const
{
	return JobType();
}

QString AuJobDeliverPassengersDeserted::TypeName()
{
	return QObject::tr("Transport People, City Deserted");
}

bool AuJobDeliverPassengersDeserted::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverPassengersDeserted* otherJob = dynamic_cast<const AuJobDeliverPassengersDeserted*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverPassengersDeserted::Title() const
{
	return QObject::tr("%1 Is Deserted!").arg(m_CityName);
}

QString AuJobDeliverPassengersDeserted::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("The city of %1 is deserted. They need people badly.")
		.arg(m_CityName);
}

QString AuJobDeliverPassengersDeserted::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("The city of %1 is deserted. "
			"It will vanish into the dust if nobody goes to live there. "
			"Someone needs to take them a load of passengers. "
			"The city is in the %2 system, %3.")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 people to save the place.").arg(m_MinCount);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" The city will pay an extra %L1%2 per person when you offload them.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverPassengersDeserted::Brief() const
{
	QString text = QObject::tr("<p>Transport passengers to %1, located in the %2 system, %3.</p>")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Hurry! The city is deserted. "
			"It will crumble away with nobody there to maintain the buildings.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 people are needed to save the place.").arg(m_MinCount);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per person when you offload them.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverPassengersWorkers::AuJobDeliverPassengersWorkers(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverPassengers(cityId, cityName, systemId, systemName, sectorName, minCount, pricePerUnit)
{
}

AuJobDeliverPassengersWorkers::AuJobDeliverPassengersWorkers()
{
}

AuJobDeliverPassengersWorkers::AuJobDeliverPassengersWorkers(const AuJobDeliverPassengersWorkers& c) : AuJobDeliverPassengers(c)
{
}

AuJob* AuJobDeliverPassengersWorkers::Clone() const
{
	return new AuJobDeliverPassengersWorkers(*this);
}

bool AuJobDeliverPassengersWorkers::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverPassengersWorkers* cm = dynamic_cast<const AuJobDeliverPassengersWorkers*>(c);
	if(!cm)
		return false;

	return true;
}

AuJob::TYPE AuJobDeliverPassengersWorkers::Type() const
{
	return JobType();
}

QString AuJobDeliverPassengersWorkers::TypeName()
{
	return QObject::tr("Transport Laborers");
}

bool AuJobDeliverPassengersWorkers::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverPassengersWorkers* otherJob = dynamic_cast<const AuJobDeliverPassengersWorkers*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverPassengersWorkers::Title() const
{
	return QObject::tr("%1 Needs Laborers").arg(m_CityName);
}

QString AuJobDeliverPassengersWorkers::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("I heard there were jobs for the asking in %1. They need workers badly.")
		.arg(m_CityName);
}

QString AuJobDeliverPassengersWorkers::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("The city of %1 is in trouble; it's way over built. "
			"There's lots of factories and jobs but hardly anybody lives there. "
			"The place will crumble away without more people to maintain the buildings and work the jobs. "
			"Someone needs to bring workers to them in the %2 system, %3 soon.")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 more people.").arg(m_MinCount);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They're paying an added bonus of %L1%2 per worker that you drop off.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverPassengersWorkers::Brief() const
{
	QString text = QObject::tr("<p>Transport workers to %1, located in the %2 system, %3.</p>")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>The city is too big. "
			"There are too many industries. "
			"The city will crumble away without more people there to maintain the buildings.");

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 people are needed.").arg(m_MinCount);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per person when you offload them.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobPickUpPassengers::AuJobPickUpPassengers(const AuId& cityId, const QString& cityName, const QString& worldName,
		const AuId& systemId, const QString& systemName, const QString& sectorName, quint32 pricePerUnit) :
	AuJob(AuTime::Now())
{
	m_CityId = cityId;
	m_CityName = cityName;
	m_PricePerUnit = pricePerUnit;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
	m_WorldName = worldName;
}

AuJobPickUpPassengers::AuJobPickUpPassengers()
{
	m_PricePerUnit = 0;
}

AuJobPickUpPassengers::AuJobPickUpPassengers(const AuJobPickUpPassengers& c) : AuJob(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_PricePerUnit = c.m_PricePerUnit;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_WorldName = c.m_WorldName;
}

void AuJobPickUpPassengers::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_CityId;
	in >> m_CityName;
	in >> m_PricePerUnit;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_WorldName;
}

void AuJobPickUpPassengers::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_CityId;
	out << m_CityName;
	out << m_PricePerUnit;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
	out << m_WorldName;
}

bool AuJobPickUpPassengers::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobPickUpPassengers* cm = dynamic_cast<const AuJobPickUpPassengers*>(c);
	if(!cm)
		return false;

	if(m_CityId != cm->m_CityId)
		return false;
	if(m_CityName != cm->m_CityName)
		return false;
	if(m_PricePerUnit != cm->m_PricePerUnit)
		return false;
	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	if(m_WorldName != cm->m_WorldName)
		return false;
	return true;
}

QString AuJobPickUpPassengers::IconName() const
{
	return QString(":data/Contacts.png");
}

bool AuJobPickUpPassengers::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

bool AuJobPickUpPassengers::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	if(!AuJob::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	// Person must be in command of a ship with berths.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	// Check empire export policies, so the person doesn't get there and get refused passengers.
	// CID_Invalid is used for the export policy governing citizens.
	const DgData& gd = g_Au.LockDataForRead();
	POLITICAL_STANCE stanceToPerson = gd.EmpirePoliticalStance(fromEmpireId, toPerson.EmpireId());
	bool exportRestricted = gd.EmpireCommerceRestricted(fromEmpireId, CID_Invalid, stanceToPerson);
	g_Au.UnlockData();
	return !exportRestricted;
}

quint64 AuJobPickUpPassengers::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobPickUpPassengers) - sizeof(AuJob);
	mem += StringMemoryUsage(m_CityName);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	mem += StringMemoryUsage(m_WorldName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobPickUpPassengersHomeless::AuJobPickUpPassengersHomeless(const AuId& cityId, const QString& cityName, const QString& worldName,
		const AuId& systemId, const QString& systemName, const QString& sectorName, quint32 pricePerUnit) :
	AuJobPickUpPassengers(cityId, cityName, worldName, systemId, systemName, sectorName, pricePerUnit)
{
}

AuJobPickUpPassengersHomeless::AuJobPickUpPassengersHomeless()
{
}

AuJobPickUpPassengersHomeless::AuJobPickUpPassengersHomeless(const AuJobPickUpPassengersHomeless& c) :
	AuJobPickUpPassengers(c)
{
}

AuJob* AuJobPickUpPassengersHomeless::Clone() const
{
	return new AuJobPickUpPassengersHomeless(*this);
}

bool AuJobPickUpPassengersHomeless::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobPickUpPassengersHomeless* cm = dynamic_cast<const AuJobPickUpPassengersHomeless*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobPickUpPassengersHomeless::IconName() const
{
	return AuJobPickUpPassengers::IconName();
}

AuJob::TYPE AuJobPickUpPassengersHomeless::Type() const
{
	return JobType();
}

QString AuJobPickUpPassengersHomeless::TypeName()
{
	return QObject::tr("Pick Up Homeless People");
}

bool AuJobPickUpPassengersHomeless::DoesReplace(const AuJob* job) const
{
	const AuJobPickUpPassengersHomeless* otherJob = dynamic_cast<const AuJobPickUpPassengersHomeless*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobPickUpPassengersHomeless::Title() const
{
	return QObject::tr("%1 Homelessness").arg(m_CityName);
}

QString AuJobPickUpPassengersHomeless::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("There's people sleeping in the streets in %1. They've got way too many homeless people. ")
		.arg(m_CityName);
}

QString AuJobPickUpPassengersHomeless::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;

	text = QObject::tr("There's a lot of homeless people in %1. "
			"It's located on %2 in the %3 system, %4. "
			"The crowds are ridiculous. "
			"They need someone to transport the vagrants somewhere, anywhere.")
		.arg(m_CityName)
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They'll pay %L1%2 per head that you take away.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobPickUpPassengersHomeless::Brief() const
{
	QString text;

	text = QObject::tr("<p>Pick up homeless people at %1; located on %2 in the %3 system, %4.</p>")
		.arg(m_CityName)
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>People are living in the streets. "
			"Please take some of them elsewhere.</p>");

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay %L1%2 per person that you take away.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

int AuJobPickUpPassengersHomeless::PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& /*worldId*/, quint16 count)
{
	if(cityId != m_CityId)
		return 0;

	QString text = QObject::tr("The locals thanked me for rousting out %L1 homeless people.").arg(count);

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * count;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" As agreed, they paid my personal account %L3%4 per person for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0, 'f', 0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text += QObject::tr(" They paid my personal account %L5%4; it was all they had.")
				.arg(g_Au.CurrencySymbol())
				.arg(city->CommunityAccountBalance(), 0, 'f', 0);

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text += QObject::tr(" Unfortunately they are broke; they couldn't pay any of the reward that was promised.");
	}
	else
		text += QObject::tr(" They are very grateful that I took them away for no charge.");

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobPickUpPassengersOverpopulated::AuJobPickUpPassengersOverpopulated(const AuId& cityId, const QString& cityName,
		const AuId& worldId, const QString& worldName, const AuId& systemId, const QString& systemName, const QString& sectorName,
		quint32 pricePerUnit) :
	AuJobPickUpPassengers(cityId, cityName, worldName, systemId, systemName, sectorName, pricePerUnit)
{
	m_WorldId = worldId;
}

AuJobPickUpPassengersOverpopulated::AuJobPickUpPassengersOverpopulated()
{
}

AuJobPickUpPassengersOverpopulated::AuJobPickUpPassengersOverpopulated(const AuJobPickUpPassengersOverpopulated& c) :
	AuJobPickUpPassengers(c)
{
	m_WorldId = c.m_WorldId;
}

AuJob* AuJobPickUpPassengersOverpopulated::Clone() const
{
	return new AuJobPickUpPassengersOverpopulated(*this);
}

void AuJobPickUpPassengersOverpopulated::operator<<(QDataStream& in)
{
	AuJobPickUpPassengers::operator<<(in);
	in >> m_WorldId;
}

void AuJobPickUpPassengersOverpopulated::operator>>(QDataStream& out) const
{
	AuJobPickUpPassengers::operator>>(out);
	out << m_WorldId;
}

bool AuJobPickUpPassengersOverpopulated::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobPickUpPassengersOverpopulated* cm = dynamic_cast<const AuJobPickUpPassengersOverpopulated*>(c);
	if(!cm)
		return false;

	if(m_WorldId != cm->m_WorldId)
		return false;
	return true;
}

QString AuJobPickUpPassengersOverpopulated::IconName() const
{
	return AuJobPickUpPassengers::IconName();
}

AuJob::TYPE AuJobPickUpPassengersOverpopulated::Type() const
{
	return JobType();
}

QString AuJobPickUpPassengersOverpopulated::TypeName()
{
	return QObject::tr("Pick Up Overcrowded People");
}

bool AuJobPickUpPassengersOverpopulated::DoesReplace(const AuJob* job) const
{
	const AuJobPickUpPassengersOverpopulated* otherJob = dynamic_cast<const AuJobPickUpPassengersOverpopulated*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_WorldId == otherJob->WorldId();					// Overcrowding is a world-wide issue. All cities report it.
}

QString AuJobPickUpPassengersOverpopulated::Title() const
{
	return QObject::tr("%1 Is Overcrowded").arg(m_WorldName);
}

QString AuJobPickUpPassengersOverpopulated::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("There's too many people in %1. Passengers want to leave.")
		.arg(m_CityName);
}

QString AuJobPickUpPassengersOverpopulated::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;

	text = QObject::tr("The world of %1 is overpopulated. "
			"It's in the %3 system, %4. "
			"Lots of people want to get out of there; they're desperate for someone to take them away.")
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" You'll be paid an extra %L1%2 per person that you pick up from there.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobPickUpPassengersOverpopulated::Brief() const
{
	QString text;

	text = QObject::tr("<p>Pick up passengers on %2 in the %3 system, %4.</p>")
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>There are too many people inhabiting the cities of that world. "
			"The overcrowding is intolerable. "
			"Please take some of them away.</p>");

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay %L1%2 per person that you take away.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

int AuJobPickUpPassengersOverpopulated::PassengersPickedUp(DaPerson* person, const AuId& /*cityId*/, const AuId& worldId,
		quint16 count)
{
	if(worldId != m_WorldId)
		return 0;

	QString text = QObject::tr("The locals thanked me for picking up %L1 people.").arg(count);

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * count;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" As agreed, they paid my personal account %L3%4 per person for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0, 'f', 0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text += QObject::tr(" They paid my personal account %L5%4. It was all the money they had.")
				.arg(g_Au.CurrencySymbol())
				.arg(city->CommunityAccountBalance(), 0, 'f', 0);

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text += QObject::tr(" Sadly, they had no money to pay the reward that was promised.");
	}
	else
		text += QObject::tr(" They are most grateful that I took people away for no charge.");

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobPickUpPassengersOverpopulated::MemoryUsage() const
{
	quint64 mem = AuJobPickUpPassengers::MemoryUsage();
	mem += sizeof(m_WorldId);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobPickUpPassengersStranded::AuJobPickUpPassengersStranded(const AuId& cityId, const QString& cityName, const QString& worldName,
		const AuId& systemId, const QString& systemName, const QString& sectorName, quint32 pricePerUnit) :
	AuJobPickUpPassengers(cityId, cityName, worldName, systemId, systemName, sectorName, pricePerUnit)
{
}

AuJobPickUpPassengersStranded::AuJobPickUpPassengersStranded()
{
}

AuJobPickUpPassengersStranded::AuJobPickUpPassengersStranded(const AuJobPickUpPassengersStranded& c) :
	AuJobPickUpPassengers(c)
{
}

AuJob* AuJobPickUpPassengersStranded::Clone() const
{
	return new AuJobPickUpPassengersStranded(*this);
}

bool AuJobPickUpPassengersStranded::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobPickUpPassengersStranded* cm = dynamic_cast<const AuJobPickUpPassengersStranded*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobPickUpPassengersStranded::IconName() const
{
	return AuJobPickUpPassengers::IconName();
}

AuJob::TYPE AuJobPickUpPassengersStranded::Type() const
{
	return JobType();
}

QString AuJobPickUpPassengersStranded::TypeName()
{
	return QObject::tr("Pick Up Stranded Colonists");
}

bool AuJobPickUpPassengersStranded::DoesReplace(const AuJob* job) const
{
	const AuJobPickUpPassengersStranded* otherJob = dynamic_cast<const AuJobPickUpPassengersStranded*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobPickUpPassengersStranded::Title() const
{
	return QObject::tr("%1 Colonists Stranded!").arg(m_CityName);
}

QString AuJobPickUpPassengersStranded::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("Colonists are stranded in %1. People need to be evacuated.")
		.arg(m_CityName);
}

QString AuJobPickUpPassengersStranded::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text;

	text = QObject::tr("Colonists are stranded at %1. "
			"It's located on %2 in the %3 system, %4. "
			"There's no way to leave so they're feeling trapped. "
			"The situation is getting ugly.")
		.arg(m_CityName)
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" Those colonists are offering an extra %L1%2 per person to get them out of there.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobPickUpPassengersStranded::Brief() const
{
	QString text;

	text = QObject::tr("<p>Pick up colonists stranded at %1; located on %2 in the %3 system, %4.</p>")
		.arg(m_CityName)
		.arg(m_WorldName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>People want to leave but there is no way for them to get out of there without an airport. "
			"They are feeling trapped; some of them are becoming suicidal. "
			"Please rescue them from that hell hole.</p>");

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>They will pay %L1%2 per person that you take away.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

int AuJobPickUpPassengersStranded::PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& /*worldId*/, quint16 count)
{
	if(cityId != m_CityId)
		return 0;

	QString text = QObject::tr("%L1 stranded colonists thanked me for rescuing them.").arg(count);

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * count;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" As agreed, they paid my personal account %L3%4 per person for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0, 'f', 0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text = QObject::tr(" They paid my personal account %L5%4. It was all the money they had between them.")
				.arg(g_Au.CurrencySymbol())
				.arg(city->CommunityAccountBalance(), 0, 'f', 0);

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text = QObject::tr(" Unfortunately, none of them had any money to pay the reward they promised.");
	}
	else
		text = QObject::tr(" They are very grateful that I rescued them for no charge.");

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDestroyEnemyStation::AuJobDestroyEnemyStation(const AuId& stationId, const QString& stationName, const AuId& systemId,
		const QString& systemName, const QString& sectorName) :
	AuJob(AuTime::Now())
{
	m_SectorName = sectorName;
	m_StationId = stationId;
	m_StationName = stationName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobDestroyEnemyStation::AuJobDestroyEnemyStation()
{
}

AuJobDestroyEnemyStation::AuJobDestroyEnemyStation(const AuJobDestroyEnemyStation& c) : AuJob(c)
{
	m_SectorName = c.m_SectorName;
	m_StationId = c.m_StationId;
	m_StationName = c.m_StationName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobDestroyEnemyStation::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_SectorName;
	in >> m_StationId;
	in >> m_StationName;
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuJobDestroyEnemyStation::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_SectorName;
	out << m_StationId;
	out << m_StationName;
	out << m_SystemId;
	out << m_SystemName;
}

AuJob* AuJobDestroyEnemyStation::Clone() const
{
	return new AuJobDestroyEnemyStation(*this);
}

bool AuJobDestroyEnemyStation::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDestroyEnemyStation* cm = dynamic_cast<const AuJobDestroyEnemyStation*>(c);
	if(!cm)
		return false;

	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_StationId != cm->m_StationId)
		return false;
	if(m_StationName != cm->m_StationName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

QString AuJobDestroyEnemyStation::IconName() const
{
	return QString(":data/clEStation.png");
}

bool AuJobDestroyEnemyStation::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

AuJob::TYPE AuJobDestroyEnemyStation::Type() const
{
	return JobType();
}

QString AuJobDestroyEnemyStation::TypeName()
{
	return QObject::tr("Attack Enemy Station");
}

bool AuJobDestroyEnemyStation::DoesReplace(const AuJob* job) const
{
	const AuJobDestroyEnemyStation* otherJob = dynamic_cast<const AuJobDestroyEnemyStation*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_StationId == otherJob->StationId();
}

QString AuJobDestroyEnemyStation::Title() const
{
	return QObject::tr("%1 Enemy Station Detected").arg(m_StationName);
}

QString AuJobDestroyEnemyStation::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("An enemy station was spotted in the %1 system.")
		.arg(m_SystemName);
}

QString AuJobDestroyEnemyStation::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("An enemy station was detected in the %2 system, %3. "
			"The last sensor contact was reported %4. "
			"Destroy that station; it is providing a base of operations for enemy spacecraft to harass our trade vessels.")
		.arg(m_SystemName)
		.arg(m_SectorName)
		.arg(Started().UTC().toLocalTime().toString());
}

QString AuJobDestroyEnemyStation::Brief() const
{
	return QObject::tr("<p>Enemy station %1 was detected in the %2 system, %3.</p>"
			"<p>Destroy that station; it is providing a base of operations for enemy spacecraft to harass our trade vessels. "
			"The last sensor contact was reported %4.</p>"
			"<p>Standard empire reward rates apply.</p>")
		.arg(m_StationName)
		.arg(m_SystemName)
		.arg(m_SectorName)
		.arg(Started().UTC().toLocalTime().toString());
}

bool AuJobDestroyEnemyStation::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within two sectors.
	if(sectorRange > 2)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	// Only offer this one to friends, vassals and empire.
	POLITICAL_STANCE stanceToPerson = PoliticalStance(fromEmpireId, toPerson.EmpireId());
	if(stanceToPerson == POL_Enemy || stanceToPerson == POL_Neutral)
		return false;

	return true;
}

int AuJobDestroyEnemyStation::SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& /*inSystemId*/)
{
	if(!ship->Is(m_StationId))
		return 0;

	QString text = QObject::tr("%1 captured! With that station out of the way, our trade vessels will be safe in this system. "
			"Mission accomplished!")
		.arg(m_StationName);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

int AuJobDestroyEnemyStation::SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& /*inSystemId*/)
{
	if(!ship->Is(m_StationId))
		return 0;

	QString text = QObject::tr("%1 destroyed! With that station out of the way, our trade vessels will be safe in this system. "
			"Mission accomplished!")
		.arg(m_StationName);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobDestroyEnemyStation::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += StringMemoryUsage(m_SectorName);
	mem += sizeof(m_StationId);
	mem += StringMemoryUsage(m_StationName);
	mem += sizeof(m_SystemId);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobInterdictSystem::AuJobInterdictSystem(quint32 spottedEmpire, const AuId& systemId, const QString& systemName,
		const QString& sectorName) :
	AuJob(AuTime::Now())
{
	m_SectorName = sectorName;
	m_SpottedEmpire = spottedEmpire;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobInterdictSystem::AuJobInterdictSystem()
{
	m_SpottedEmpire = 0;
}

AuJobInterdictSystem::AuJobInterdictSystem(const AuJobInterdictSystem& c) : AuJob(c)
{
	m_SectorName = c.m_SectorName;
	m_SpottedEmpire = c.m_SpottedEmpire;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

const AuJobInterdictSystem& AuJobInterdictSystem::operator=(const AuJobInterdictSystem& c)
{
	AuJob::operator=(c);
	m_SectorName = c.m_SectorName;
	m_SpottedEmpire = c.m_SpottedEmpire;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	return *this;
}

void AuJobInterdictSystem::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_SpottedEmpire;
}

void AuJobInterdictSystem::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
	out << m_SpottedEmpire;
}

AuJob* AuJobInterdictSystem::Clone() const
{
	return new AuJobInterdictSystem(*this);
}

bool AuJobInterdictSystem::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobInterdictSystem* cm = dynamic_cast<const AuJobInterdictSystem*>(c);
	if(!cm)
		return false;

	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SpottedEmpire != cm->m_SpottedEmpire)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

QString AuJobInterdictSystem::IconName() const
{
	return QString(":data/clEShip.png");
}

bool AuJobInterdictSystem::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

AuJob::TYPE AuJobInterdictSystem::Type() const
{
	return JobType();
}

QString AuJobInterdictSystem::TypeName()
{
	return QObject::tr("Attack Enemy Spacecraft, Interdict System");
}

bool AuJobInterdictSystem::DoesReplace(const AuJob* job) const
{
	const AuJobInterdictSystem* otherJob = dynamic_cast<const AuJobInterdictSystem*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_SystemId == otherJob->SystemId();
}

QString AuJobInterdictSystem::Title() const
{
	return QObject::tr("%1 Interdiction Zone").arg(m_SystemName);
}

QString AuJobInterdictSystem::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(m_SpottedEmpire, empireName);
	g_Au.UnlockData();
	return QObject::tr("Enemy %1 spacecraft were spotted in the %2 system.")
		.arg(empireName)
		.arg(m_SystemName);
}

QString AuJobInterdictSystem::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(m_SpottedEmpire, empireName);
	g_Au.UnlockData();
	return QObject::tr("Enemy activity was detected %1 in the %2 system, %3. "
			"Spacecraft belonging to %4 were picked up on sensors. "
			"Interdict any enemy spacecraft you find there.")
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_SystemName)
		.arg(m_SectorName)
		.arg(empireName);
}

QString AuJobInterdictSystem::Brief() const
{
	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(m_SpottedEmpire, empireName);
	g_Au.UnlockData();
	return QObject::tr("<p>Enemy activity was detected in the %2 system, %3.</p>"
			"<p>Interdict any enemy spacecraft you find there. "
			"The last sensor contact was a %4 spacecraft at %1.</p>"
			"<p>Standard empire reward rates apply.</p>")
		.arg(Started().UTC().toLocalTime().toString())
		.arg(m_SystemName)
		.arg(m_SectorName)
		.arg(empireName);
}

bool AuJobInterdictSystem::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within two sectors.
	if(sectorRange > 2)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	// Only offer this one to friends, vassals and empire.
	POLITICAL_STANCE stanceToPerson = PoliticalStance(fromEmpireId, toPerson.EmpireId());
	if(stanceToPerson == POL_Enemy || stanceToPerson == POL_Neutral)
		return false;

	return true;
}

int AuJobInterdictSystem::SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId)
{
	if(inSystemId != m_SystemId)
		return 0;

	QString text = QObject::tr("%1 captured! Interdiction complete. I better remain on alert for a while. "
			"There could be more enemy spacecraft in this system.")
		.arg(ship->NameWithEmpire());

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

int AuJobInterdictSystem::SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId)
{
	if(inSystemId != m_SystemId)
		return 0;

	QString text = QObject::tr("%1 destroyed! Interdiction complete. I better remain on alert for a while. "
			"There could be more enemy spacecraft in this system.")
		.arg(ship->NameWithEmpire());

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobInterdictSystem::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobInterdictSystem) - sizeof(AuJob);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobStory::AuJobStory(quint32 storyInstance, const QString& title, const QString& author, const QString& brief) :
	AuJob(AuTime::Now())
{
	m_Author = author;
	m_Brief = brief;
	m_StoryInstance = storyInstance;
	m_Title = title;
}

AuJobStory::AuJobStory()
{
	m_StoryInstance = 0;
}

AuJobStory::AuJobStory(const AuJobStory& c) : AuJob(c)
{
	m_Author = c.m_Author;
	m_Brief = c.m_Brief;
	m_StoryInstance = c.m_StoryInstance;
	m_Title = c.m_Title;
}

void AuJobStory::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);

	quint8 ui8;

	in >> m_Author;
	in >> m_Brief;
	in >> ui8;
	in >> m_StoryInstance;
	in >> m_Title;
}

void AuJobStory::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_Author;
	out << m_Brief;
	out << (quint8)0;	// Used to be m_ExecFirstBlock
	out << m_StoryInstance;
	out << m_Title;
}

AuJob* AuJobStory::Clone() const
{
	return new AuJobStory(*this);
}

bool AuJobStory::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobStory* cm = dynamic_cast<const AuJobStory*>(c);
	if(!cm)
		return false;

	if(m_Author != cm->m_Author)
		return false;
	if(m_Brief != cm->m_Brief)
		return false;
	if(m_StoryInstance != cm->m_StoryInstance)
		return false;
	if(m_Title != cm->m_Title)
		return false;
	return true;
}

QString AuJobStory::IconName() const
{
	return QString(":data/CommNarrator.png");
}

bool AuJobStory::IsOfferExpired(const AuTime& asOf) const
{
	Q_UNUSED(asOf);
	return false;												// No expiration. Offer is based on story catalyst.
}

AuJob::TYPE AuJobStory::Type() const
{
	return JobType();
}

QString AuJobStory::TypeName()
{
	return QObject::tr("Story");
}

bool AuJobStory::DoesReplace(const AuJob* job) const
{
	const AuJobStory* otherJob = dynamic_cast<const AuJobStory*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_StoryInstance == otherJob->StoryInstance();
}

QString AuJobStory::Title() const
{
	return QObject::tr("%1 by %2").arg(m_Title).arg(m_Author);
}

QString AuJobStory::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return m_Brief;	//DHH
}

QString AuJobStory::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return m_Brief;
}

QString AuJobStory::Brief() const
{
	QString b(m_Brief);
	b.replace("\n", "<br>");
	return b;
}

bool AuJobStory::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	return AuJob::IsOkToOffer(toPerson, fromEmpireId, sectorRange);
}

quint64 AuJobStory::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += StringMemoryUsage(m_Author);
	mem += StringMemoryUsage(m_Brief);
	mem += sizeof(m_StoryInstance);
	mem += StringMemoryUsage(m_Title);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobTransferPrisoners::AuJobTransferPrisoners(const AuId& fromCityId, const QString& fromCityName, const AuId& fromSystemId,
			const QString& fromSystemName, const QString& fromSectorName, const AuId& toCityId, const QString& toCityName,
			const AuId& toSystemId, const QString& toSystemName, const QString& toSectorName, quint32 minCount,
			quint32 pricePerUnit) :
	AuJob(AuTime::Now())
{
	m_FromCityId = fromCityId;
	m_FromCityName = fromCityName;
	m_FromSectorName = fromSectorName;
	m_FromSystemId = fromSystemId;
	m_FromSystemName = fromSystemName;
	m_MinCount = minCount;
	m_PricePerUnit = pricePerUnit;
	m_ToCityId = toCityId;
	m_ToCityName = toCityName;
	m_ToSectorName = toSectorName;
	m_ToSystemId = toSystemId;
	m_ToSystemName = toSystemName;
}

AuJobTransferPrisoners::AuJobTransferPrisoners()
{
	m_MinCount = 0;
	m_PricePerUnit = 0;
}

AuJobTransferPrisoners::AuJobTransferPrisoners(const AuJobTransferPrisoners& c) : AuJob(c)
{
	m_FromCityId = c.m_FromCityId;
	m_FromCityName = c.m_FromCityName;
	m_FromSectorName = c.m_FromSectorName;
	m_FromSystemId = c.m_FromSystemId;
	m_FromSystemName = c.m_FromSystemName;
	m_MinCount = c.m_MinCount;
	m_PricePerUnit = c.m_PricePerUnit;
	m_ToCityId = c.m_ToCityId;
	m_ToCityName = c.m_ToCityName;
	m_ToSectorName = c.m_ToSectorName;
	m_ToSystemId = c.m_ToSystemId;
	m_ToSystemName = c.m_ToSystemName;
}

void AuJobTransferPrisoners::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_FromCityId;
	in >> m_FromCityName;
	in >> m_FromSectorName;
	in >> m_FromSystemId;
	in >> m_FromSystemName;
	in >> m_MinCount;
	in >> m_PricePerUnit;
	in >> m_ToCityId;
	in >> m_ToCityName;
	in >> m_ToSectorName;
	in >> m_ToSystemId;
	in >> m_ToSystemName;
}

void AuJobTransferPrisoners::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_FromCityId;
	out << m_FromCityName;
	out << m_FromSectorName;
	out << m_FromSystemId;
	out << m_FromSystemName;
	out << m_MinCount;
	out << m_PricePerUnit;
	out << m_ToCityId;
	out << m_ToCityName;
	out << m_ToSectorName;
	out << m_ToSystemId;
	out << m_ToSystemName;
}

AuJob* AuJobTransferPrisoners::Clone() const
{
	return new AuJobTransferPrisoners(*this);
}

bool AuJobTransferPrisoners::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobTransferPrisoners* cm = dynamic_cast<const AuJobTransferPrisoners*>(c);
	if(!cm)
		return false;

	if(m_FromCityId != cm->m_FromCityId)
		return false;
	if(m_FromCityName != cm->m_FromCityName)
		return false;
	if(m_FromSectorName != cm->m_FromSectorName)
		return false;
	if(m_FromSystemId != cm->m_FromSystemId)
		return false;
	if(m_FromSystemName != cm->m_FromSystemName)
		return false;
	if(m_MinCount != cm->m_MinCount)
		return false;
	if(m_PricePerUnit != cm->m_PricePerUnit)
		return false;
	if(m_ToCityId != cm->m_ToCityId)
		return false;
	if(m_ToCityName != cm->m_ToCityName)
		return false;
	if(m_ToSectorName != cm->m_ToSectorName)
		return false;
	if(m_ToSystemId != cm->m_ToSystemId)
		return false;
	if(m_ToSystemName != cm->m_ToSystemName)
		return false;
	return true;
}

QString AuJobTransferPrisoners::IconName() const
{
	return QString(":data/dp_Brig.png");
}

bool AuJobTransferPrisoners::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

AuJob::TYPE AuJobTransferPrisoners::Type() const
{
	return JobType();
}

QString AuJobTransferPrisoners::TypeName()
{
	return QObject::tr("Transfer Prisoners");
}

bool AuJobTransferPrisoners::DoesReplace(const AuJob* job) const
{
	const AuJobTransferPrisoners* otherJob = dynamic_cast<const AuJobTransferPrisoners*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_FromSystemId == otherJob->FromSystemId() && m_ToSystemId == otherJob->ToSystemId();
}

QString AuJobTransferPrisoners::Title() const
{
	return QObject::tr("%1 Prisoner Transfer To %2").arg(m_FromCityName).arg(m_ToCityName);
}

QString AuJobTransferPrisoners::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("Prisoners are ready for transport from %1 to %2?")
		.arg(m_FromCityName)
		.arg(m_ToCityName);
}

QString AuJobTransferPrisoners::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	if(m_MinCount == 1)
		return QObject::tr("There's a prisoner at %2 in %3, %4. "
				"Transport it to %5 in %6, %7. "
				"You will be paid %L8%9 when you drop it off.")
			.arg(m_FromCityName)
			.arg(m_FromSystemName)
			.arg(m_FromSectorName)
			.arg(m_ToCityName)
			.arg(m_ToSystemName)
			.arg(m_ToSectorName)
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());
	else
		return QObject::tr("There's %L1 prisoners at %2 in %3, %4. "
				"Transport them to %5 in %6, %7. "
				"You will be paid %L8%9 when you drop them off.")
			.arg(m_MinCount)
			.arg(m_FromCityName)
			.arg(m_FromSystemName)
			.arg(m_FromSectorName)
			.arg(m_ToCityName)
			.arg(m_ToSystemName)
			.arg(m_ToSectorName)
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());
}

QString AuJobTransferPrisoners::Brief() const
{
	return QObject::tr("<p>Transfer %L1 prisoners.</p>"
			"<p>Pick them up at %2 in %3, %4.</p>"
			"<p>Deliver them to %5 in %6, %7.</p>"
			"<p>You will be paid %L8%9 when you drop them off.</p>"
			"<p>Be careful! "
			"Lock them in the brig. "
			"Vicious scoundrels could be among them.</p>")
		.arg(m_MinCount)
		.arg(m_FromCityName)
		.arg(m_FromSystemName)
		.arg(m_FromSectorName)
		.arg(m_ToCityName)
		.arg(m_ToSystemName)
		.arg(m_ToSectorName)
		.arg(m_PricePerUnit)
		.arg(g_Au.CurrencySymbol());
}

bool AuJobTransferPrisoners::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	// Only offer this one to friends, vassals and empire.
	POLITICAL_STANCE stanceToPerson = PoliticalStance(fromEmpireId, toPerson.EmpireId());
	if(stanceToPerson == POL_Enemy || stanceToPerson == POL_Neutral)
		return false;

	return true;
}

int AuJobTransferPrisoners::PrisonersLoaded(DaPerson* person, const AuId& cityId, quint16 count)
{
	if(cityId != m_FromCityId)
		return 0;

	QString text;
	if(count == 1)
		text = QObject::tr("This scalawag needs to get to %1 right away; I better watch my back.")
			.arg(m_ToCityName);
	else
		text = QObject::tr("These are filthy miscreants. I need to dump them off in %1 as quick as possible.")
			.arg(m_ToCityName);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 0;
}

int AuJobTransferPrisoners::PrisonersUnloaded(DaPerson* person, const AuId& cityId, quint16 count)
{
	if(cityId != m_ToCityId)
		return 0;

	DaSolarSystem* system = person->FindParentSolarSystemInEnv();
	if(!system)
		return 0;
	DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_ToCityId));
	if(!city)
		return 0;

	int countDelivered;
	if(count > m_MinCount)
	{
		countDelivered = m_MinCount;
		m_MinCount = 0;											// Reduce the count needed to zero.
	}
	else
	{
		countDelivered = count;
		m_MinCount -= count;									// Reduce the count needed by the count provided.
	}

	QString text = QObject::tr("%1 prisoners locked away safe and sound.").arg(countDelivered);

	double reward = m_PricePerUnit * countDelivered;
	if(reward <= city->CommunityAccountBalance())
	{
		text += QObject::tr(" My personal account was credited %L3%4 per prisoner for a total of %L5%4.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol())
			.arg(reward, 0, 'f', 0);

		person->CreditAccount(reward);
		city->CommunityDebitAccount(reward, AuBank::Gov_DebitRewards);
	}
	else if(city->GovtAccountBalance())
	{
		text += QObject::tr(" My personal account was credited %L5%4; it was all the money they had.")
			.arg(g_Au.CurrencySymbol())
			.arg(city->GovtAccountBalance(), 0, 'f', 0);

		person->CreditAccount(city->GovtAccountBalance());
		city->CommunityDebitAccount(city->GovtAccountBalance(), AuBank::Gov_DebitRewards);
	}
	else
		text += QObject::tr(" Unfortunately they had no money at this time to pay for the transfer.");

	if(m_MinCount)
		text += QObject::tr(" They are still expecting %1 more prisoners.").arg(m_MinCount);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	// This job is done if the count needed has been reduced to zero.
	return (m_MinCount == 0) ? 2 : 1;
}

quint64 AuJobTransferPrisoners::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobTransferPrisoners) - sizeof(AuJob);
	mem += StringMemoryUsage(m_FromCityName);
	mem += StringMemoryUsage(m_FromSectorName);
	mem += StringMemoryUsage(m_FromSystemName);
	mem += StringMemoryUsage(m_ToCityName);
	mem += StringMemoryUsage(m_ToSectorName);
	mem += StringMemoryUsage(m_ToSystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobVisitSystem::AuJobVisitSystem(const AuId& cityId, const AuId& systemId, const QString& systemName, const QString& sectorName,
		quint32 reward) :
	AuJob(AuTime::Now())
{
	m_CityId = cityId;
	m_Reward = reward;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobVisitSystem::AuJobVisitSystem()
{
	m_Reward = 0;
}

AuJobVisitSystem::AuJobVisitSystem(const AuJobVisitSystem& c) : AuJob(c)
{
	m_CityId = c.m_CityId;
	m_Reward = c.m_Reward;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobVisitSystem::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);
	in >> m_CityId;
	in >> m_Reward;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuJobVisitSystem::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_CityId;
	out << m_Reward;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
}

AuJob* AuJobVisitSystem::Clone() const
{
	return new AuJobVisitSystem(*this);
}

bool AuJobVisitSystem::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobVisitSystem* cm = dynamic_cast<const AuJobVisitSystem*>(c);
	if(!cm)
		return false;

	if(m_CityId != cm->m_CityId)
		return false;
	if(m_Reward != cm->m_Reward)
		return false;
	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

QString AuJobVisitSystem::IconName() const
{
	return QString(":data/CommGovernment.png");
}

bool AuJobVisitSystem::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

AuJob::TYPE AuJobVisitSystem::Type() const
{
	return JobType();
}

QString AuJobVisitSystem::TypeName()
{
	return QObject::tr("Visit Abandoned Citizens");
}

bool AuJobVisitSystem::DoesReplace(const AuJob* job) const
{
	const AuJobVisitSystem* otherJob = dynamic_cast<const AuJobVisitSystem*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	if(m_SystemId != otherJob->SystemId())
		return false;

	// Same city job replaces itself or higher reward offered in the same system.
	return m_CityId == otherJob->CityId() || m_Reward > otherJob->Reward();
}

QString AuJobVisitSystem::Title() const
{
	return QObject::tr("%1 Abandoned").arg(m_SystemName);
}

QString AuJobVisitSystem::Teaser(const QString& fromName) const
{
	return QObject::tr("People living in %2 in the %1 system have lost contact with our empire.")
		.arg(m_SystemName)
		.arg(fromName);
}

QString AuJobVisitSystem::Proposal(const QString& fromName) const
{
	if(m_Reward)
		return QObject::tr("No one has traveled through the %1 system in a long time; it's in the %2. "
				"The people in %5 need assurance that the empire has not abandoned them. "
				"You will be paid %L3%4 when you arrive in the system and hail them on the Trade channel.")
			.arg(m_SystemName)
			.arg(m_SectorName)
			.arg(m_Reward)
			.arg(g_Au.CurrencySymbol())
			.arg(fromName);
	else
		return QObject::tr("No one has traveled through the %1 system in a long time; it's in the %2. "
				"The people in %3 need assurance that the empire has not abandoned them. "
				"Hail them on the Trade channel when you arrive in the system, to let them know you are there.")
			.arg(m_SystemName)
			.arg(m_SectorName)
			.arg(fromName);
}

QString AuJobVisitSystem::Brief() const
{
	if(m_Reward)
		return QObject::tr("<p>Travel to the %1 system, %2.</p>"
				"<p>No one has traveled through that system in a long time. "
				"Visit the people there. "
				"They need assurance that the empire has not abandoned them.</p>"
				"<p>You will be paid %L3%4 when you arrive in the system and hail on the Trade channel.</p>")
			.arg(m_SystemName)
			.arg(m_SectorName)
			.arg(m_Reward)
			.arg(g_Au.CurrencySymbol());
	else
		return QObject::tr("<p>Travel to the %1 system, %2.</p>"
				"<p>No one has traveled through that system in a long time. "
				"Visit the people there. "
				"They need assurance that the empire has not abandoned them.</p>"
				"<p>Hail on the Trade channel when you arrive in the system.</p>")
			.arg(m_SystemName)
			.arg(m_SectorName);
}

bool AuJobVisitSystem::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within two sectors.
	if(sectorRange > 2)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	// Only offer this to members of the same empire.
	if(toPerson.EmpireId() != fromEmpireId)
		return false;

	return true;
}

int AuJobVisitSystem::SystemVisited(DaPerson* person, const AuId& systemId, DaBd& bd)
{
	if(systemId != m_SystemId)
		return 0;
	if(bd.CapitolId() != m_CityId)
		return 0;

	QString text = QObject::tr("The citizens of the %1 system are relieved that I am here. "
			"My presence assures them they have not been abandoned or forgotten by the empire.")
		.arg(m_SystemName);
	if(m_Reward)
	{
		if(m_Reward <= bd.CommunityAccountBalance())
		{
			text += QObject::tr(" The agreed fee of %L1%2 was credited to my personal account.")
				.arg(m_Reward)
				.arg(g_Au.CurrencySymbol());

			person->CreditAccount(m_Reward);
			bd.CommunityDebitAccount(m_Reward, AuBank::Gov_DebitRewards);
		}
		else if(bd.AccountBalance())
		{
			text += QObject::tr(" A fee of %L1%2 was credited to my personal account; it was all the %3 had.")
				.arg(bd.AccountBalance(), 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(bd.IndustryName());

			person->CreditAccount(bd.AccountBalance());
			bd.CommunityDebitAccount(bd.AccountBalance(), AuBank::Gov_DebitRewards);
		}
		else
			text += QObject::tr(" Unfortunately the %1 had no money to pay the reward that was offered.")
				.arg(bd.IndustryName());
	}

	if(!bd.CityName().isEmpty())
		text += QObject::tr(" The people of %1 invited me to visit their city and share tales of my adventures.")
			.arg(bd.CityName());
	else
		text += QObject::tr(" The residents and workers at the %1 invited me to visit them and share tales of my adventures.")
			.arg(bd.Name());

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);
	return 2;
}

quint64 AuJobVisitSystem::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobVisitSystem) - sizeof(AuJob);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobCommandShip::AuJobCommandShip(const AuId& shipId, const QString& shipName, const QString& shipDesc,
		const AuId& systemId, const QString& systemName, const QString& sectorName) :
	AuJob(AuTime::Now())
{
	m_SectorName = sectorName;
	m_ShipDescription = shipDesc;
	m_ShipId = shipId;
	m_ShipName = shipName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobCommandShip::AuJobCommandShip()
{
}

AuJobCommandShip::AuJobCommandShip(const AuJobCommandShip& c) : AuJob(c)
{
	m_SectorName = c.m_SectorName;
	m_ShipDescription = c.m_ShipDescription;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobCommandShip::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);

	quint32 ui32;

	in >> m_SectorName;
	in >> m_ShipDescription;
	in >> ui32;					//DHH Used to be ship empire id but it wasn't needed.
	in >> m_ShipId;
	in >> m_ShipName;
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuJobCommandShip::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_SectorName;
	out << m_ShipDescription;
	out << (quint32)0;			//DHH Used to be ship empire id but it wasn't needed.
	out << m_ShipId;
	out << m_ShipName;
	out << m_SystemId;
	out << m_SystemName;
}

bool AuJobCommandShip::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobCommandShip* cm = dynamic_cast<const AuJobCommandShip*>(c);
	if(!cm)
		return false;

	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_ShipDescription != cm->m_ShipDescription)
		return false;
	if(m_ShipId != cm->m_ShipId)
		return false;
	if(m_ShipName != cm->m_ShipName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

QString AuJobCommandShip::IconName() const
{
	return QString(":data/Captain.png");
}

bool AuJobCommandShip::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60 * 2;
}

bool AuJobCommandShip::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	// Do not offer this to players who are captain of their own ship.
	if(!toPerson.SpawnShip().IsNull() &&
			toPerson.SpawnShipBerthType() == AuRosterEntry::Captain)
		return false;

	// Only offer this to members of the same empire.
	if(fromEmpireId != toPerson.EmpireId())
		return false;

	return true;
}

quint64 AuJobCommandShip::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobCommandShip) - sizeof(AuJob);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_ShipDescription);
	mem += StringMemoryUsage(m_ShipName);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobCommandShipInOrbit::AuJobCommandShipInOrbit(const AuId& shipId, const QString& shipName, const QString& shipDesc,
		const QString& planetName, const AuId& systemId, const QString& systemName, const QString& sectorName) :
	AuJobCommandShip(shipId, shipName, shipDesc, systemId, systemName, sectorName)
{
	m_PlanetName = planetName;
}

AuJobCommandShipInOrbit::AuJobCommandShipInOrbit()
{
}

AuJobCommandShipInOrbit::AuJobCommandShipInOrbit(const AuJobCommandShipInOrbit& c) : AuJobCommandShip(c)
{
	m_PlanetName = c.m_PlanetName;
}

void AuJobCommandShipInOrbit::operator<<(QDataStream& in)
{
	AuJobCommandShip::operator<<(in);
	in >> m_PlanetName;
}

void AuJobCommandShipInOrbit::operator>>(QDataStream& out) const
{
	AuJobCommandShip::operator>>(out);
	out << m_PlanetName;
}

AuJob* AuJobCommandShipInOrbit::Clone() const
{
	return new AuJobCommandShipInOrbit(*this);
}

bool AuJobCommandShipInOrbit::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobCommandShipInOrbit* cm = dynamic_cast<const AuJobCommandShipInOrbit*>(c);
	if(!cm)
		return false;

	if(m_PlanetName != cm->m_PlanetName)
		return false;
	return true;
}

AuJob::TYPE AuJobCommandShipInOrbit::Type() const
{
	return JobType();
}

QString AuJobCommandShipInOrbit::TypeName()
{
	return QObject::tr("Command Spacecraft In Orbit");
}

bool AuJobCommandShipInOrbit::DoesReplace(const AuJob* job) const
{
	const AuJobCommandShipInOrbit* otherJob = dynamic_cast<const AuJobCommandShipInOrbit*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_ShipId == otherJob->ShipId();
}

QString AuJobCommandShipInOrbit::Title() const
{
	return QObject::tr("%1 Orbiting %2 Needs A Captain").arg(m_ShipName).arg(m_PlanetName);
}

QString AuJobCommandShipInOrbit::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("A ship of our empire needs a captain. It is orbiting %1.")
		.arg(m_PlanetName);
}

QString AuJobCommandShipInOrbit::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("%1 is a %2, manufactured in the %3 system, %4.")
		.arg(m_ShipName)
		.arg(m_ShipDescription)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr(" It is moored in orbit over %1.").arg(m_PlanetName);

	text += QObject::tr(" The ship needs a captain. Go there and take command of her.");

	return text;
}

QString AuJobCommandShipInOrbit::Brief() const
{
	QString text = QObject::tr("<p>%1 is a %2, manufactured in the %3 system, %4.")
		.arg(m_ShipName)
		.arg(m_ShipDescription)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr(" It is moored in orbit over %1.</p>").arg(m_PlanetName);

	text += QObject::tr("<p>The ship needs a captain. Go there and take command of her.</p>");

	return text;
}

int AuJobCommandShipInOrbit::SpacecraftCommanded(DaPerson* person, const AuId& shipId)
{
	if(shipId != m_ShipId)
		return 0;

	QString text = QObject::tr("%1 is under my command! "
			"My first orders of duty: christen the ship, move to the nearest station or airport, "
			"then board a crew using the Trade comm channel. "
			"Check fuel; I can call for fuel when the ship is near a station or on the tarmac at an airport. "
			"When the ship is ready, I'll ask around for more jobs. Crew, passengers and townspeople can provide leads.")
		.arg(m_ShipName);
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobCommandShipInOrbit::MemoryUsage() const
{
	quint64 mem = AuJobCommandShip::MemoryUsage();
	mem += sizeof(AuJobCommandShipInOrbit) - sizeof(AuJobCommandShip);
	mem += StringMemoryUsage(m_PlanetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobCommandShipOnGround::AuJobCommandShipOnGround(const AuId& shipId, const QString& shipName, const QString& shipDesc,
		const QString& planetName, const AuId& systemId, const QString& systemName, const QString& sectorName) :
	AuJobCommandShip(shipId, shipName, shipDesc, systemId, systemName, sectorName)
{
	m_CityName = planetName;
}

AuJobCommandShipOnGround::AuJobCommandShipOnGround()
{
}

AuJobCommandShipOnGround::AuJobCommandShipOnGround(const AuJobCommandShipOnGround& c) : AuJobCommandShip(c)
{
	m_CityName = c.m_CityName;
}

void AuJobCommandShipOnGround::operator<<(QDataStream& in)
{
	AuJobCommandShip::operator<<(in);
	in >> m_CityName;
}

void AuJobCommandShipOnGround::operator>>(QDataStream& out) const
{
	AuJobCommandShip::operator>>(out);
	out << m_CityName;
}

AuJob* AuJobCommandShipOnGround::Clone() const
{
	return new AuJobCommandShipOnGround(*this);
}

bool AuJobCommandShipOnGround::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobCommandShipOnGround* cm = dynamic_cast<const AuJobCommandShipOnGround*>(c);
	if(!cm)
		return false;

	if(m_CityName != cm->m_CityName)
		return false;
	return true;
}

AuJob::TYPE AuJobCommandShipOnGround::Type() const
{
	return JobType();
}

QString AuJobCommandShipOnGround::TypeName()
{
	return QObject::tr("Command Spacecraft At Airport");
}

bool AuJobCommandShipOnGround::DoesReplace(const AuJob* job) const
{
	const AuJobCommandShipOnGround* otherJob = dynamic_cast<const AuJobCommandShipOnGround*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_ShipId == otherJob->ShipId();
}

QString AuJobCommandShipOnGround::Title() const
{
	return QObject::tr("%1 In City %2 Needs A Captain").arg(m_ShipName).arg(m_CityName);
}

QString AuJobCommandShipOnGround::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("A ship of our empire needs a captain. It is on the ground in %1, in the %2 system.")
		.arg(m_CityName)
		.arg(m_SystemName);
}

QString AuJobCommandShipOnGround::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("%1 is a %2, manufactured in the %3 system, %4.")
		.arg(m_ShipName)
		.arg(m_ShipDescription)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr(" It is on the ground in %1.").arg(m_CityName);

	text += QObject::tr(" The ship needs a captain. Go there and take command of her.");

	return text;
}

QString AuJobCommandShipOnGround::Brief() const
{
	QString text = QObject::tr("<p>%1 is a %2, manufactured in the %3 system, %4.")
		.arg(m_ShipName)
		.arg(m_ShipDescription)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr(" It is on the ground near the spacecraft factory in %1.</p>").arg(m_CityName);

	text += QObject::tr("<p>The ship needs a captain. Go there and take command of her.</p>");

	return text;
}

int AuJobCommandShipOnGround::SpacecraftCommanded(DaPerson* person, const AuId& shipId)
{
	if(shipId != m_ShipId)
		return 0;

	QString text = QObject::tr("%1 is under my command! "
			"My first orders of duty: christen the ship and load a crew using the Trade comm channel. "
			"Check fuel; I can call for fuel while sitting here on the tarmac at the airport or when the ship is near a station. "
			"When the ship is ready, I'll ask around for more jobs. Crew, passengers and townspeople can provide leads.")
		.arg(m_ShipName);
	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	return 2;
}

quint64 AuJobCommandShipOnGround::MemoryUsage() const
{
	quint64 mem = AuJobCommandShip::MemoryUsage();
	mem += sizeof(AuJobCommandShipOnGround) - sizeof(AuJobCommandShip);
	mem += StringMemoryUsage(m_CityName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverFood::AuJobDeliverFood(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
		const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality, quint32 minCount,
		quint32 pricePerUnit) :
	AuJob(AuTime::Now())
{
	Q_ASSERT(minQuality);
	m_CityId = cityId;
	m_CityName = cityName;
	m_ForEcoRole = forEcoRole;
	m_MinCount = minCount;
	m_MinQuality = minQuality;
	m_PricePerUnit = pricePerUnit;
	m_SectorName = sectorName;
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuJobDeliverFood::AuJobDeliverFood()
{
	m_ForEcoRole = ECO_Carnivore;
	m_MinCount = 0;
	m_MinQuality = 1;
	m_PricePerUnit = 0;
}

AuJobDeliverFood::AuJobDeliverFood(const AuJobDeliverFood& c) : AuJob(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_ForEcoRole = c.m_ForEcoRole;
	m_MinCount = c.m_MinCount;
	m_MinQuality = c.m_MinQuality;
	m_PricePerUnit = c.m_PricePerUnit;
	m_SectorName = c.m_SectorName;
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuJobDeliverFood::operator>>(QDataStream& out) const
{
	AuJob::operator>>(out);
	out << m_CityId;
	out << m_CityName;
	out << (quint8)m_ForEcoRole;
	out << m_MinCount;
	out << m_MinQuality;
	quint8 mintl = 0;
	out << mintl;;
	out << m_PricePerUnit;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
}

void AuJobDeliverFood::operator<<(QDataStream& in)
{
	AuJob::operator<<(in);

	quint8 ui8;
	in >> m_CityId;
	in >> m_CityName;
	in >> ui8;
	m_ForEcoRole = (ECOROLE)ui8;
	in >> m_MinCount;
	in >> m_MinQuality;
	if(!m_MinQuality)
		m_MinQuality = 1;
	quint8 mintl;
	in >> mintl;
	in >> m_PricePerUnit;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
}

bool AuJobDeliverFood::Compare(const AuJob* c)
{
	if(!AuJob::Compare(c))
		return false;
	const AuJobDeliverFood* cm = dynamic_cast<const AuJobDeliverFood*>(c);
	if(!cm)
		return false;

	if(m_CityId != cm->m_CityId)
		return false;
	if(m_CityName != cm->m_CityName)
		return false;
	if(m_ForEcoRole != cm->m_ForEcoRole)
		return false;
	if(m_MinCount != cm->m_MinCount)
		return false;
	if(m_MinQuality != cm->m_MinQuality)
		return false;
	if(m_PricePerUnit != cm->m_PricePerUnit)
		return false;
	if(m_SectorName != cm->m_SectorName)
		return false;
	if(m_SystemId != cm->m_SystemId)
		return false;
	if(m_SystemName != cm->m_SystemName)
		return false;
	return true;
}

bool AuJobDeliverFood::IsOfferExpired(const AuTime& asOf) const
{
	return Started().SecsTo(asOf) > 60 * 60;
}

bool AuJobDeliverFood::IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const
{
	// Must be within one sector.
	if(sectorRange > 1)
		return false;

	// Person must be in command of a ship.
	if(toPerson.SpawnShip().IsNull() ||
			(toPerson.SpawnShipBerthType() != AuRosterEntry::Captain &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::CaptainSlot &&
			 toPerson.SpawnShipBerthType() != AuRosterEntry::OfficerHuman))
		return false;

	if(!AuJob::IsOkToOffer(toPerson, fromEmpireId, sectorRange))
		return false;

	return true;
}

int AuJobDeliverFood::CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item)
{
	if(cityId != m_CityId)
		return 0;
	if(item.Quality() < m_MinQuality)
		return 0;
	if(!item.IsFood(m_ForEcoRole))
		return 0;												// The commodity is not food for the ecological role.

	int countDelivered;
	if(item.Count() > m_MinCount)
	{
		countDelivered = m_MinCount;
		m_MinCount = 0;											// Reduce the count needed to zero.
	}
	else
	{
		countDelivered = item.Count();
		m_MinCount -= item.Count();								// Reduce the count needed by the count provided.
	}

	QString text = QObject::tr("The locals thanked me for bringing %L1 units of %2.")
		.arg(countDelivered)
		.arg(item.SayName());

	switch(m_ForEcoRole)
	{
	case ECO_Carnivore:
		text += QObject::tr(" These carnivores say it tastes great.").arg(countDelivered);
		break;
	case ECO_Omnivore:
		text += QObject::tr(" These omnivores say it tastes great.").arg(countDelivered);
		break;
	case ECO_Scavenger:
		text += QObject::tr(" These scavengers say it tastes great.").arg(countDelivered);
		break;
	case ECO_Herbivore:
		text += QObject::tr(" These herbivores say it tastes great.").arg(countDelivered);
		break;
	}

	if(m_PricePerUnit)
	{
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(!system)
			return 0;
		DaBd* city = dynamic_cast<DaBd*>(system->FindIdRecursive(m_CityId));
		if(!city)
			return 0;

		double reward = m_PricePerUnit * countDelivered;
		if(reward <= city->CommunityAccountBalance())
		{
			text += QObject::tr(" My personal account was credited an extra %L3%4 per unit for a total of %L5%4.")
				.arg(m_PricePerUnit)
				.arg(g_Au.CurrencySymbol())
				.arg(reward, 0, 'f', 0);

			person->CreditAccount(reward);
			city->CommunityDebitAccount(reward, AuBank::Civ_DebitRewards);
		}
		else if(city->CommunityAccountBalance())
		{
			text += QObject::tr(" My personal account was credited %L5%4. It was all the money the city had.")
				.arg(g_Au.CurrencySymbol())
				.arg(city->CommunityAccountBalance(), 0, 'f', 0);

			person->CreditAccount(city->CommunityAccountBalance());
			city->CommunityDebitAccount(city->CommunityAccountBalance(), AuBank::Civ_DebitRewards);
		}
		else
			text += QObject::tr(" The city had no money to pay the promised reward.");
	}
	else
		text += QObject::tr(" They felt it was humane of me to bring food at no extra charge.");

	if(m_MinCount)
		text += QObject::tr(" They were hoping I would bring %1 more units.").arg(m_MinCount);

	AuCommThought msg(text, person->Id(), person->Name());
	g_Au.SendAtomChat(msg);

	// This job is done if the count needed has been reduced to zero.
	return (m_MinCount == 0) ? 2 : 1;
}

quint64 AuJobDeliverFood::MemoryUsage() const
{
	quint64 mem = AuJob::MemoryUsage();
	mem += sizeof(AuJobDeliverFood) - sizeof(AuJob);
	mem += StringMemoryUsage(m_CityName);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverFoodHungry::AuJobDeliverFoodHungry(const AuId& cityId, const QString& cityName,
		const AuId& systemId, const QString& systemName, const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverFood(cityId, cityName, systemId, systemName, sectorName, forEcoRole, minQuality, minCount,
			pricePerUnit)
{
}

AuJobDeliverFoodHungry::AuJobDeliverFoodHungry()
{
}

AuJobDeliverFoodHungry::AuJobDeliverFoodHungry(const AuJobDeliverFoodHungry& c) : AuJobDeliverFood(c)
{
}

AuJob* AuJobDeliverFoodHungry::Clone() const
{
	return new AuJobDeliverFoodHungry(*this);
}

bool AuJobDeliverFoodHungry::Compare(const AuJob* c)
{
	if(!AuJobDeliverFood::Compare(c))
		return false;
	const AuJobDeliverFoodHungry* cm = dynamic_cast<const AuJobDeliverFoodHungry*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverFoodHungry::IconName() const
{
	return QString(":data/MsgSolicitFood.png");
}

AuJob::TYPE AuJobDeliverFoodHungry::Type() const
{
	return JobType();
}

QString AuJobDeliverFoodHungry::TypeName()
{
	return QObject::tr("Haul Food, People Hungry");
}

bool AuJobDeliverFoodHungry::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverFoodHungry* otherJob = dynamic_cast<const AuJobDeliverFoodHungry*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverFoodHungry::Title() const
{
	return QObject::tr("%1 People Are Hungry").arg(m_CityName);
}

QString AuJobDeliverFoodHungry::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("People in %1 are hungry. They need food.")
		.arg(m_CityName);
}

QString AuJobDeliverFoodHungry::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("There's a lot of hungry people in %1; it's located in the %2 system, %3.")
			.arg(m_CityName)
			.arg(m_SystemName)
			.arg(m_SectorName);

	switch(m_ForEcoRole)
	{
	case ECO_Carnivore:
		text += QObject::tr(" The locals are carnivores.");
		break;
	case ECO_Omnivore:
		text += QObject::tr(" The locals are omnivores.");
		break;
	case ECO_Scavenger:
		text += QObject::tr(" The locals are scavengers.");
		break;
	case ECO_Herbivore:
		text += QObject::tr(" The locals are herbivores.");
		break;
	}

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units of food.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The food must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They will pay an extra %L1%2 per unit when you get there.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverFoodHungry::Brief() const
{
	QString text = QObject::tr("<p>Haul food to %1; located in the %2 system, %3.</p>")
			.arg(m_CityName)
			.arg(m_SystemName)
			.arg(m_SectorName);

	text += QObject::tr("<p>Cupboards are bare. People are hungry.");

	switch(m_ForEcoRole)
	{
	case ECO_Carnivore:
		text += QObject::tr(" The locals are carnivores.");
		break;
	case ECO_Omnivore:
		text += QObject::tr(" The locals are omnivores.");
		break;
	case ECO_Scavenger:
		text += QObject::tr(" The locals are scavengers.");
		break;
	case ECO_Herbivore:
		text += QObject::tr(" The locals are herbivores.");
		break;
	}

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The food must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuJobDeliverFoodStarving::AuJobDeliverFoodStarving(const AuId& cityId, const QString& cityName, const AuId& systemId,
		const QString& systemName, const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality,
		quint32 minCount, quint32 pricePerUnit) :
	AuJobDeliverFood(cityId, cityName, systemId, systemName, sectorName, forEcoRole, minQuality, minCount,
			pricePerUnit)
{
}

AuJobDeliverFoodStarving::AuJobDeliverFoodStarving()
{
}

AuJobDeliverFoodStarving::AuJobDeliverFoodStarving(const AuJobDeliverFoodStarving& c) : AuJobDeliverFood(c)
{
}

AuJob* AuJobDeliverFoodStarving::Clone() const
{
	return new AuJobDeliverFoodStarving(*this);
}

bool AuJobDeliverFoodStarving::Compare(const AuJob* c)
{
	if(!AuJobDeliverFood::Compare(c))
		return false;
	const AuJobDeliverFoodStarving* cm = dynamic_cast<const AuJobDeliverFoodStarving*>(c);
	if(!cm)
		return false;

	return true;
}

QString AuJobDeliverFoodStarving::IconName() const
{
	return QString(":data/MsgSolicitFood.png");
}

AuJob::TYPE AuJobDeliverFoodStarving::Type() const
{
	return JobType();
}

QString AuJobDeliverFoodStarving::TypeName()
{
	return QObject::tr("Haul Food, People Starving");
}

bool AuJobDeliverFoodStarving::DoesReplace(const AuJob* job) const
{
	const AuJobDeliverFoodStarving* otherJob = dynamic_cast<const AuJobDeliverFoodStarving*>(job);
	if(!otherJob)
		return false;											// This job can only replace other jobs of the same kind.
	return m_CityId == otherJob->CityId();
}

QString AuJobDeliverFoodStarving::Title() const
{
	return QObject::tr("%1 People Are Starving!").arg(m_CityName);
}

QString AuJobDeliverFoodStarving::Teaser(const QString& fromName) const
{
	Q_UNUSED(fromName);
	return QObject::tr("People in %1 are starving to death. They need food urgently.")
		.arg(m_CityName);
}

QString AuJobDeliverFoodStarving::Proposal(const QString& fromName) const
{
	Q_UNUSED(fromName);
	QString text = QObject::tr("Emergency! People are starving to death in %2; located in the %3 system, %4. "
			"They need a load of food urgently.")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	switch(m_ForEcoRole)
	{
	case ECO_Carnivore:
		text += QObject::tr(" The locals there are carnivores.");
		break;
	case ECO_Omnivore:
		text += QObject::tr(" The locals there are omnivores.");
		break;
	case ECO_Scavenger:
		text += QObject::tr(" The locals there are scavengers.");
		break;
	case ECO_Herbivore:
		text += QObject::tr(" The locals there are herbivores.");
		break;
	}

	if(m_MinCount > 0)
		text += QObject::tr(" They need %L1 units of food.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The food must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	if(m_PricePerUnit > 0)
		text += QObject::tr(" They will pay an extra %L1%2 per unit when you offload the food.")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

QString AuJobDeliverFoodStarving::Brief() const
{
	QString text = QObject::tr("<p>Haul food to %2; located in the %3 system, %4.</p>")
		.arg(m_CityName)
		.arg(m_SystemName)
		.arg(m_SectorName);

	text += QObject::tr("<p>Hurry! People are starving to death.");

	switch(m_ForEcoRole)
	{
	case ECO_Carnivore:
		text += QObject::tr(" The locals there are carnivores.");
		break;
	case ECO_Omnivore:
		text += QObject::tr(" The locals there are omnivores.");
		break;
	case ECO_Scavenger:
		text += QObject::tr(" The locals there are scavengers.");
		break;
	case ECO_Herbivore:
		text += QObject::tr(" The locals there are herbivores.");
		break;
	}

	if(m_MinCount > 0)
		text += QObject::tr(" %L1 units are needed.").arg(m_MinCount);

	if(m_MinQuality > 0)
		text += QObject::tr(" The food must be at least %2 Q%3.")
			.arg(DbCommodity::QualityDesc(m_MinQuality))
			.arg(m_MinQuality);

	text += "</p>";

	if(m_PricePerUnit > 0)
		text += QObject::tr("<p>The city will pay an extra %L1%2 per unit upon delivery.</p>")
			.arg(m_PricePerUnit)
			.arg(g_Au.CurrencySymbol());

	return text;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
