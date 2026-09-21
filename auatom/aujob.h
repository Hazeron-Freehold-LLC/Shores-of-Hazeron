//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujob.h,v 1.50 2022/02/19 17:31:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuJob_H
#define AuJob_H

#include "auatom.h"
#include "audnabase.h"
#include "autime.h"
#include "auid.h"
#include "dbcommodity.h"
#include <QStringList>

class AuItem;
class DaBd;
class DaPerson;
class DaVehicle;
class DaVessel;

class AUATOM_DLL AuJob
{
public:
	// Add nu types at the end of this enum.
	// This job type value gets stored in persistent storage so be careful not to change existing values.
	enum TYPE
	{
		InvalidJob = -1,
		Assassin,						// AuJobAssassin
		BountyHunterPerson,				// AuJobBountyHunterPerson
		BountyHunterShip,				// AuJobBountyHunterShip
		BountyHunterVehicle,			// AuJobBountyHunterVehicle
		CommandShipInOrbit,				// AuJobCommandShipInOrbit
		CommandShipOnGround,			// AuJobCommandShipOnGround
		DeliverCargoConstruction,		// AuJobDeliverCargoConstruction
		DeliverCargoDisease,			// AuJobDeliverCargoDisease
		DeliverCargoGasping,			// AuJobDeliverCargoGasping
		DeliverCargoManufacture,		// AuJobDeliverCargoManufacture
		DeliverCargoPoison,				// AuJobDeliverCargoPoison
		DeliverCargoRats,				// AuJobDeliverCargoRats
		DeliverCargoSpacecraftFactory,	// AuJobDeliverCargoSpacecraftFactory
		DeliverCargoSuffocating,		// AuJobDeliverCargoSuffocating
		DeliverFoodHungry,				// AuJobDeliverFoodHungry
		DeliverFoodStarving,			// AuJobDeliverFoodStarving
		DeliverPassengersColonists,		// AuJobDeliverPassengersColonists
		DeliverPassengersDeserted,		// AuJobDeliverPassengersDeserted
		DeliverPassengersWorkers,		// AuJobDeliverPassengersWorkers
		DestroyEnemyStation,			// AuJobDestroyEnemyStation
		InterdictSystem,				// AuJobInterdictSystem
		PickUpPassengersHomeless,		// AuJobPickUpPassengersHomeless
		PickUpPassengersOverpopulated,	// AuJobPickUpPassengersOverpopulated
		PickUpPassengersStranded,		// AuJobPickUpPassengersStranded
		Story,							// AuJobStory
		TransferPrisoners,				// AuJobTransferPrisoners
		VisitSystem,					// AuJobVisitSystem
		JobTypes						// LAST ENTRY EXPECTED NOT TO EXCEED A BYTE. ADD NEW TYPES BEFORE THIS ENTRY.
	};

private:
	AuTime							m_Start;				// Date/time UTC this job was started.

public:
	AuJob();
	AuJob(const AuTime& start);
	virtual ~AuJob();
	AuJob(const AuJob& c);

public:	// Operators
	const AuJob& operator=(const AuJob& c);

public:	// Methods
	// Returns an appropriate verbal reply when an avatar accepts this job offer.
	virtual QString AcceptReply() const
	{
		return QObject::tr("Good luck.");
	}

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const = 0;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const = 0;

	// Virtual comparison for equality.
	// This base class implementation returns false if the start time differs or if the Type() value differs.
	virtual bool Compare(const AuJob* c);

	// Allocates the appropriate job object on the heap based on the value returned by Type() for each derived class.
	// Useful when reading a job from a data stream.
	static AuJob* CreateJob(TYPE type);

	// Return true if 'this' job replaces 'job' in the jobs posting queue.
	virtual bool DoesReplace(const AuJob* job) const = 0;

	// An icon that indicates the type of the job.
	virtual QString IconName() const = 0;

	static QString IconName(TYPE type);

	// Jobs that return true always get added to the queue. Non-critical jobs may be ignored.
	virtual bool IsCritical() const
	{
		return false;
	}

	// AI query to see if a job is critical when deciding what jobs to offer a person.
	// 'sectorRange' is how far away the job is right now, in whole sectors. 0=current sector.
	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return false;
	}

	inline bool IsLike(const AuJob& job) const
	{
		return Title()+Brief() == job.Title()+job.Brief();
	}

	// Return true if this object should be discarded when deserialized.
	virtual bool IsObsolete() const
	{
		return false;
	}

	// Return true if this job is expired, specifically when this job is presented as a job offer.
	virtual bool IsOfferExpired(const AuTime& asOf) const = 0;

	// 'toPerson' is already known to be in the same galaxy as this job.
	// 'rangeInSectors' is the range in whole sectors between the 'toPerson's sector and this job's sector location.
	// Return true if it is ok to offer this job to 'toPerson' when the offering empire is 'fromEmpireId'.
	// This base class returns false if the 'fromEmpireId' considers toPerson's empire id to be an enemy.
	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Complete description of this job, stated for the comm channel.
	virtual QString Proposal(const QString& fromName) const = 0;

	// Returns an appropriate verbal reply when an avatar rejects this job offer after hearing the proposal.
	virtual QString RejectProposalReply() const
	{
		return QObject::tr("Come back if you change your mind.");
	}

	// Returns an appropriate verbal reply when an avatar rejects this job offer after hearing the teaser.
	virtual QString RejectTeaserReply() const
	{
		return QObject::tr("Maybe another time.");
	}

	inline const AuTime& Started() const
	{
		return m_Start;
	}

	// Statement made by sentient offering this job to a person.
	virtual QString Teaser(const QString& fromName) const = 0;

	// Provide a brief one-line description of this job.
	virtual QString Title() const = 0;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const = 0;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName() = 0;

	// Calls the virtual TypeName function for the corresponding object type.
	static QString TypeName(TYPE type);

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	// These base class stubs all return 0 without doing anything.
	virtual int CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item);
	virtual int CryoDelivered(DaPerson* person, const AuId& cityId, quint16 count);
	virtual int CryoPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
	virtual int PassengersDelivered(DaPerson* person, const AuId& cityId, quint16 count);
	virtual int PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
	virtual int PersonKilled(DaPerson* person, DaPerson* victim);
	virtual int PrisonersLoaded(DaPerson* person, const AuId& cityId, quint16 count);
	virtual int PrisonersUnloaded(DaPerson* person, const AuId& cityId, quint16 count);
	virtual int SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	virtual int SpacecraftCommanded(DaPerson* person, const AuId& shipId);
	virtual int SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	virtual int SystemVisited(DaPerson* person, const AuId& systemId, DaBd& bd);
	virtual int VehicleCaptured(DaPerson* person, DaVehicle* vehicle);
	virtual int VehicleDestroyed(DaPerson* person, DaVehicle* vehicle);
};

inline QDataStream& operator<<(QDataStream& out, const AuJob& job)
{
	job >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuJob& job)
{
	job << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobList : public QList<AuJob*>
{
public:
	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	quint64 MemoryUsage() const;

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// The event is propagated to every job in this list.
	// Returns 'true' if any jobs are changed or completed as a result of this event.
	bool CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item);
	bool CryoDelivered(DaPerson* person, const AuId& cityId, quint16 count);
	bool CryoPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
	bool PassengersDelivered(DaPerson* person, const AuId& cityId, quint16 count);
	bool PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
	bool PersonKilled(DaPerson* person, DaPerson* victim);
	bool PrisonersLoaded(DaPerson* person, const AuId& cityId, quint16 count);
	bool PrisonersUnloaded(DaPerson* person, const AuId& cityId, quint16 count);
	bool SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	bool SpacecraftCommanded(DaPerson* person, const AuId& shipId);
	bool SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	bool SystemVisited(DaPerson* person, const AuId& systemId, DaBd& bd);
	bool VehicleCaptured(DaPerson* person, DaVehicle* vehicle);
	bool VehicleDestroyed(DaPerson* person, DaVehicle* vehicle);
};

inline QDataStream& operator<<(QDataStream& out, const AuJobList& jobList)
{
	jobList >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuJobList& jobList)
{
	jobList << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Job to assassinate a government official in a neutral or enemy empire. Killing neutral target does not trigger mutual enemy
// stance declaration.

class AuJobAssassin : public AuJob
{
private:
	QString								m_LastSeen;				// Description of the place the target was last seen.
	AuId								m_PreyId;				// Prey can be any DaUnit atom.
	QString								m_PreyName;				// Name of prey.
	quint32								m_Reward;				// Reward paid on success.

public:
	AuJobAssassin(const AuId& preyId, const QString& preyName, const QString& lastSeen, quint32 reward);
	AuJobAssassin();
	AuJobAssassin(const AuJobAssassin& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return true;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::Assassin;
	}

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& PreyId() const
	{
		return m_PreyId;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PersonKilled(DaPerson* person, DaPerson* victim);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobBountyHunter : public AuJob
{
protected:
	QString								m_LastSeen;				// Description of the place the target was last seen.
	AuId								m_PreyId;				// Prey can be any DaUnit atom.
	QString								m_PreyName;				// Name of prey.
	QStringList							m_Reasons;				// Reasons why there is a bounty on this guy.
	double								m_Reward;				// Current bounty when job was offered.

public:
	AuJobBountyHunter(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward, 
			const QStringList& reasons);
	AuJobBountyHunter();
	AuJobBountyHunter(const AuJobBountyHunter& c);

	virtual bool Compare(const AuJob* c);

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return true;
	}

	// Bounties are not handled via bounty hunter missions any more.
	virtual bool IsObsolete() const
	{
		return true;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& PreyId() const
	{
		return m_PreyId;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobBountyHunterPerson : public AuJobBountyHunter
{
public:
	AuJobBountyHunterPerson(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward, 
			const QStringList& reasons);
	AuJobBountyHunterPerson();
	AuJobBountyHunterPerson(const AuJobBountyHunterPerson& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return true;
	}

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::BountyHunterPerson;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PersonKilled(DaPerson* person, DaPerson* victim);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobBountyHunterShip : public AuJobBountyHunter
{
public:
	AuJobBountyHunterShip(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward, 
			const QStringList& reasons);
	AuJobBountyHunterShip();
	AuJobBountyHunterShip(const AuJobBountyHunterShip& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return true;
	}

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::BountyHunterShip;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& /*inSystemId*/);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobBountyHunterVehicle : public AuJobBountyHunter
{
public:
	AuJobBountyHunterVehicle(const AuId& preyId, const QString& preyName, const QString& lastSeen, double reward, 
			const QStringList& reasons);
	AuJobBountyHunterVehicle();
	AuJobBountyHunterVehicle(const AuJobBountyHunterVehicle& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	virtual bool Compare(const AuJob* c);

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool DoesReplace(const AuJob* job) const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		Q_UNUSED(sectorRange);
		return true;
	}

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::BountyHunterVehicle;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int VehicleDestroyed(DaPerson* person, DaVehicle* vehicle);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargo : public AuJob
{
protected:
	COMMODITYID							m_CID;					// Commodity to deliver.
	AuId								m_CityId;				// City id to deliver cargo.
	QString								m_CityName;				// Name of city.
	quint32								m_MinCount;				// Minimum quantity to deliver.
	quint8								m_MinQuality;			// Minimum quality wanted.
	quint32								m_PricePerUnit;			// Price per unit on success.
	QString								m_SectorName;			// Name of sector.
	AuId								m_SystemId;				// System id to deliver cargo.
	QString								m_SystemName;			// Name of system.

public:
	AuJobDeliverCargo(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargo();
	AuJobDeliverCargo(const AuJobDeliverCargo& c);

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual bool Compare(const AuJob* c);

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoGasping : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoGasping(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoGasping();
	AuJobDeliverCargoGasping(const AuJobDeliverCargoGasping& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoGasping;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoSuffocating : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoSuffocating(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoSuffocating();
	AuJobDeliverCargoSuffocating(const AuJobDeliverCargoSuffocating& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 1;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoSuffocating;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoRats : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoRats(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoRats();
	AuJobDeliverCargoRats(const AuJobDeliverCargoRats& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoRats;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoDisease : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoDisease(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoDisease();
	AuJobDeliverCargoDisease(const AuJobDeliverCargoDisease& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoDisease;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoPoison : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoPoison(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoPoison();
	AuJobDeliverCargoPoison(const AuJobDeliverCargoPoison& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoPoison;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoSpacecraftFactory : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoSpacecraftFactory(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoSpacecraftFactory();
	AuJobDeliverCargoSpacecraftFactory(const AuJobDeliverCargoSpacecraftFactory& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoSpacecraftFactory;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoConstruction : public AuJobDeliverCargo
{
public:
	AuJobDeliverCargoConstruction(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit);
	AuJobDeliverCargoConstruction();
	AuJobDeliverCargoConstruction(const AuJobDeliverCargoConstruction& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoConstruction;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverCargoManufacture : public AuJobDeliverCargo
{
private:
	COMMODITYID							m_CIDStopped;			// Commodity manufacture that is halted.

public:
	AuJobDeliverCargoManufacture(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, COMMODITYID cid, quint8 minQuality, quint32 minCount,
			quint32 pricePerUnit, COMMODITYID cidStopped);
	AuJobDeliverCargoManufacture();
	AuJobDeliverCargoManufacture(const AuJobDeliverCargoManufacture& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverCargoManufacture;
	}

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverFood : public AuJob
{
protected:
	AuId								m_CityId;				// City id to deliver cargo.
	QString								m_CityName;				// Name of city.
	ECOROLE								m_ForEcoRole;			// Food category to deliver.
	quint32								m_MinCount;				// Minimum quantity to deliver.
	quint8								m_MinQuality;			// Minimum quality wanted.
	quint32								m_PricePerUnit;			// Price per unit on success.
	QString								m_SectorName;			// Name of sector.
	AuId								m_SystemId;				// System id to deliver cargo.
	QString								m_SystemName;			// Name of system.

public:
	AuJobDeliverFood(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverFood();
	AuJobDeliverFood(const AuJobDeliverFood& c);

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual bool Compare(const AuJob* c);

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int CargoDelivered(DaPerson* person, const AuId& cityId, const AuItem& item);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverFoodHungry : public AuJobDeliverFood
{
public:
	AuJobDeliverFoodHungry(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverFoodHungry();
	AuJobDeliverFoodHungry(const AuJobDeliverFoodHungry& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverFoodHungry;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverFoodStarving : public AuJobDeliverFood
{
public:
	AuJobDeliverFoodStarving(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, ECOROLE forEcoRole, quint8 minQuality, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverFoodStarving();
	AuJobDeliverFoodStarving(const AuJobDeliverFoodStarving& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 1;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverFoodStarving;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverPassengers : public AuJob
{
protected:
	AuId								m_CityId;				// City id to deliver cargo.
	QString								m_CityName;				// Name of city.
	quint32								m_MinCount;				// Minimum quantity to deliver.
	quint32								m_PricePerUnit;			// Price per unit on success.
	QString								m_SectorName;			// Name of sector.
	AuId								m_SystemId;				// System id to deliver cargo.
	QString								m_SystemName;			// Name of system.

public:
	AuJobDeliverPassengers(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverPassengers();
	AuJobDeliverPassengers(const AuJobDeliverPassengers& c);

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual bool Compare(const AuJob* c);

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PassengersDelivered(DaPerson* person, const AuId& cityId, quint16 count);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverPassengersColonists : public AuJobDeliverPassengers
{
public:
	AuJobDeliverPassengersColonists(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverPassengersColonists();
	AuJobDeliverPassengersColonists(const AuJobDeliverPassengersColonists& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverPassengersColonists;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverPassengersDeserted : public AuJobDeliverPassengers
{
public:
	AuJobDeliverPassengersDeserted(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverPassengersDeserted();
	AuJobDeliverPassengersDeserted(const AuJobDeliverPassengersDeserted& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverPassengersDeserted;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobDeliverPassengersWorkers : public AuJobDeliverPassengers
{
public:
	AuJobDeliverPassengersWorkers(const AuId& cityId, const QString& cityName, const AuId& systemId, const QString& systemName,
			const QString& sectorName, quint32 minCount, quint32 pricePerUnit);
	AuJobDeliverPassengersWorkers();
	AuJobDeliverPassengersWorkers(const AuJobDeliverPassengersWorkers& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DeliverPassengersWorkers;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobPickUpPassengers : public AuJob
{
protected:
	AuId								m_CityId;				// City id to deliver cargo.
	QString								m_CityName;				// Name of city.
	quint32								m_PricePerUnit;			// Price per unit on success.
	QString								m_SectorName;			// Name of sector.
	AuId								m_SystemId;				// System id.
	QString								m_SystemName;			// Name of system.
	QString								m_WorldName;			// Name of world.

public:
	AuJobPickUpPassengers(const AuId& cityId, const QString& cityName, const QString& worldName, const AuId& systemId,
			const QString& systemName, const QString& sectorName, quint32 pricePerUnit);
	AuJobPickUpPassengers();
	AuJobPickUpPassengers(const AuJobPickUpPassengers& c);

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual bool Compare(const AuJob* c);

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobPickUpPassengersHomeless : public AuJobPickUpPassengers
{
public:
	AuJobPickUpPassengersHomeless(const AuId& cityId, const QString& cityName, const QString& worldName, const AuId& systemId,
			const QString& systemName, const QString& sectorName, quint32 pricePerUnit);
	AuJobPickUpPassengersHomeless();
	AuJobPickUpPassengersHomeless(const AuJobPickUpPassengersHomeless& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::PickUpPassengersHomeless;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobPickUpPassengersOverpopulated : public AuJobPickUpPassengers
{
private:
	AuId								m_WorldId;				// World id.

public:
	AuJobPickUpPassengersOverpopulated(const AuId& cityId, const QString& cityName, const AuId& worldId, const QString& worldName,
			const AuId& systemId, const QString& systemName, const QString& sectorName, quint32 pricePerUnit);
	AuJobPickUpPassengersOverpopulated();
	AuJobPickUpPassengersOverpopulated(const AuJobPickUpPassengersOverpopulated& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::PickUpPassengersOverpopulated;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

	inline const AuId& WorldId() const
	{
		return m_WorldId;
	}

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobPickUpPassengersStranded : public AuJobPickUpPassengers
{
public:
	AuJobPickUpPassengersStranded(const AuId& cityId, const QString& cityName, const QString& worldName,
			const AuId& systemId, const QString& systemName, const QString& sectorName, quint32 pricePerUnit);
	AuJobPickUpPassengersStranded();
	AuJobPickUpPassengersStranded(const AuJobPickUpPassengersStranded& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::PickUpPassengersStranded;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PassengersPickedUp(DaPerson* person, const AuId& cityId, const AuId& worldId, quint16 count);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Job to destroy a pirate base.

class AuJobDestroyEnemyStation : public AuJob
{
private:
	QString								m_SectorName;			// Name of sector.
	AuId								m_StationId;			// Station id to destroy.
	QString								m_StationName;			// Name of pirate station.
	AuId								m_SystemId;				// System id that contains the pirate station.
	QString								m_SystemName;			// Name of system.

public:
	AuJobDestroyEnemyStation(const AuId& stationId, const QString& stationName, const AuId& systemId, const QString& systemName,
			const QString& sectorName);
	AuJobDestroyEnemyStation();
	AuJobDestroyEnemyStation(const AuJobDestroyEnemyStation& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::DestroyEnemyStation;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& StationId() const
	{
		return m_StationId;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	virtual int SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Job to be a pirate in a solar system.

class AuJobInterdictSystem : public AuJob
{
private:
	QString								m_SectorName;			// Name of sector.
	quint32								m_SpottedEmpire;		// Enemy empire spotted that triggered this mission.
	AuId								m_SystemId;				// System id that contains the pirate station.
	QString								m_SystemName;			// Name of system.

public:
	AuJobInterdictSystem(quint32 spottedEmpire, const AuId& systemId, const QString& systemName, const QString& sectorName);
	AuJobInterdictSystem();
	AuJobInterdictSystem(const AuJobInterdictSystem& c);

public:	// Operators
	const AuJobInterdictSystem& operator=(const AuJobInterdictSystem& c);

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

public:	// Methods
	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 2;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::InterdictSystem;
	}

	inline const AuId& SystemId() const
	{
		return m_SystemId;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SpacecraftCaptured(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
	virtual int SpacecraftDestroyed(DaPerson* person, DaVessel* ship, const AuId& inSystemId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobStory : public AuJob
{
private:
	QString								m_Author;
	QString								m_Brief;
	quint32								m_StoryInstance;		// story instance id. EXCEPTION! Story script id for job offers.
	QString								m_Title;

public:
	AuJobStory(quint32 storyInstance, const QString& title, const QString& author, const QString& brief);
	AuJobStory();
	AuJobStory(const AuJobStory& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::Story;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline quint32 StoryInstance() const
	{
		return m_StoryInstance;
	}

	inline const QString& StoryAuthor() const
	{
		return m_Author;
	}

	inline const QString& StoryBrief() const
	{
		return m_Brief;
	}

	inline const QString& StoryTitle() const
	{
		return m_Title;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobTransferPrisoners : public AuJob
{
private:
	AuId								m_FromCityId;			// City id to pick up cargo.
	QString								m_FromCityName;			// Name of city.
	QString								m_FromSectorName;		// Name of sector.
	AuId								m_FromSystemId;			// System id to pick up cargo.
	QString								m_FromSystemName;		// Name of system.
	quint32								m_MinCount;				// Minimum quantity to deliver.
	quint32								m_PricePerUnit;			// Price per unit on success.
	AuId								m_ToCityId;				// City id to deliver prisoners.
	QString								m_ToCityName;			// Name of city.
	QString								m_ToSectorName;			// Name of sector.
	AuId								m_ToSystemId;			// System id to deliver prisoners.
	QString								m_ToSystemName;			// Name of system.

public:
	AuJobTransferPrisoners(const AuId& fromCityId, const QString& fromCityName, const AuId& fromSystemId,
			const QString& fromSystemName, const QString& fromSectorName, const AuId& toCityId, const QString& toCityName,
			const AuId& toSystemId, const QString& toSystemName, const QString& toSectorName, quint32 minCount,
			quint32 pricePerUnit);
	AuJobTransferPrisoners();
	AuJobTransferPrisoners(const AuJobTransferPrisoners& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	inline const AuId& FromSystemId() const
	{
		return m_FromSystemId;
	}

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::TransferPrisoners;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	inline const AuId& ToSystemId() const
	{
		return m_ToSystemId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int PrisonersLoaded(DaPerson* person, const AuId& cityId, quint16 count);
	virtual int PrisonersUnloaded(DaPerson* person, const AuId& cityId, quint16 count);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobVisitSystem : public AuJob
{
private:
	AuId								m_CityId;				// City who posted the reward.
	quint32								m_Reward;				// Reward paid on success.
	QString								m_SectorName;			// Name of sector.
	AuId								m_SystemId;				// System id to deliver cargo.
	QString								m_SystemName;			// Name of system.

public:
	AuJobVisitSystem(const AuId& cityId, const AuId& systemId, const QString& systemName, const QString& sectorName,
			quint32 reward);
	AuJobVisitSystem();
	AuJobVisitSystem(const AuJobVisitSystem& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 3;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::VisitSystem;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline quint32 Reward() const
	{
		return m_Reward;
	}

	inline const AuId& SystemId() const
	{
		return m_SystemId;
	}

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SystemVisited(DaPerson* person, const AuId& systemId, DaBd& bd);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobCommandShip : public AuJob
{
protected:
	QString								m_SectorName;			// Name of sector.
	QString								m_ShipDescription;		// Description of ship.
	AuId								m_ShipId;				// Id of ship that needs a captain.
	QString								m_ShipName;				// Name of ship.
	AuId								m_SystemId;				// System id
	QString								m_SystemName;			// Name of system.

public:
	AuJobCommandShip(const AuId& shipId, const QString& shipName, const QString& shipDesc, const AuId& systemId,
			const QString& systemName, const QString& sectorName);
	AuJobCommandShip();
	AuJobCommandShip(const AuJobCommandShip& c);

	virtual bool Compare(const AuJob* c);

	// An icon that indicates the type of the job.
	virtual QString IconName() const;

	virtual bool IsCritical() const
	{
		return true;
	}

	virtual bool IsCritical(int sectorRange) const
	{
		return sectorRange <= 0;
	}

	virtual bool IsOfferExpired(const AuTime& asOf) const;

	virtual bool IsOkToOffer(const DaPerson& toPerson, quint32 fromEmpireId, int sectorRange) const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobCommandShipInOrbit : public AuJobCommandShip
{
private:
	QString								m_PlanetName;			// Planet where ship is at.

public:
	AuJobCommandShipInOrbit(const AuId& shipId, const QString& shipName, const QString& shipDesc, const QString& planetName,
			const AuId& systemId, const QString& systemName, const QString& sectorName);
	AuJobCommandShipInOrbit();
	AuJobCommandShipInOrbit(const AuJobCommandShipInOrbit& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::CommandShipInOrbit;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SpacecraftCommanded(DaPerson* person, const AuId& shipId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuJobCommandShipOnGround : public AuJobCommandShip
{
private:
	QString								m_CityName;				// City where ship is at.

public:
	AuJobCommandShipOnGround(const AuId& shipId, const QString& shipName, const QString& shipDesc, const QString& cityName,
			const AuId& systemId, const QString& systemName, const QString& sectorName);
	AuJobCommandShipOnGround();
	AuJobCommandShipOnGround(const AuJobCommandShipOnGround& c);

	// Provide a description of this job. Html formatting is ok.
	virtual QString Brief() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Proposal(const QString& fromName) const;

	virtual QString Teaser(const QString& fromName) const;

	// Returns a duplicate of this job, allocated on the heap.
	virtual AuJob* Clone() const;

	virtual bool Compare(const AuJob* c);

	virtual bool DoesReplace(const AuJob* job) const;

	// Returns the unique number assigned to this job derived class. Every derived class must return a unique number.
	inline static TYPE JobType()
	{
		return AuJob::CommandShipOnGround;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Provide a brief one-line description of this job.
	virtual QString Title() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	// Returns a name for the type, suitable for the tooltip on a filter of message types.
	virtual QString TypeName();

public:
	// Events may trigger job completion or changes.
	// Event handlers are called when certain things happen.
	// Return values are:
	//  0 = Event ignored.
	//  1 = Job changed by event.
	//  2 = Job completed by event.
	virtual int SpacecraftCommanded(DaPerson* person, const AuId& shipId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuJob_H
