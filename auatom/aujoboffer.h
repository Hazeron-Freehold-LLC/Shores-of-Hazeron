//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujoboffer.h,v 1.20 2022/02/19 17:31:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuJobOffer_H
#define AuJobOffer_H

#include "auatom.h"
#include "aujob.h"
#include <aupoint.h>
#include <QDataStream>
#include <QIcon>
#include <QMetaType>

class AUATOM_DLL AuJobOffer
{
private:
	quint32								m_FromEmpireId;			// Empire id so we don't offer jobs to enemies.
	QString								m_FromName;				// Name for sorting on lists.
	AuPoint								m_FromSectorLoc;		// Sector location for selection of jobs by range.
	quint8								m_FromGalaxy;			// GALAXY enum for range
	AuJob*								m_Job;					// The job that is offered.

public:
	AuJobOffer(const QString& fromName, quint32 fromEmpireId, const AuPoint& fromSectorLoc, GALAXY fromGalaxy, const AuJob* job);
	AuJobOffer();
	AuJobOffer(const AuJobOffer& c);
	~AuJobOffer();

public:
	const AuJobOffer& operator=(const AuJobOffer& c);

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	bool operator==(const AuJobOffer& c) const;

public:
	// Returns an appropriate verbal reply when an avatar accepts this job offer.
	inline QString AcceptReply() const
	{
		if(!m_Job)
			return QString();
		return m_Job->AcceptReply();
	}

	inline QString Brief() const
	{
		if(!m_Job)
			return QString();
		return m_Job->Brief();
	}

	inline bool DoesReplace(const AuJobOffer& jobOffer) const
	{
		if(!m_Job)
			return false;										// A job offer with no job cannot replace anything.
		if(!jobOffer.Job())
			return true;										// A job offer with no job needs to be replaced.
		return m_Job->DoesReplace(jobOffer.Job());
	}

	inline quint32 FromEmpireId() const
	{
		return m_FromEmpireId;
	}

	inline GALAXY FromGalaxy() const
	{
		return (GALAXY)m_FromGalaxy;
	}

	inline const QString& FromName() const
	{
		return m_FromName;
	}

	inline const AuPoint& FromSectorLoc() const
	{
		return m_FromSectorLoc;
	}

	QString HtmlDesc() const;

	QString HtmlToolTip() const;

	inline QIcon Icon() const
	{
		if(!m_Job)
			return QIcon();
		return QIcon(m_Job->IconName());
	}

	inline bool IsCritical() const
	{
		if(!m_Job)
			return false;
		return m_Job->IsCritical();
	}

	inline bool IsCritical(int sectorRange) const
	{
		if(!m_Job)
			return false;
		return m_Job->IsCritical(sectorRange);
	}

	inline bool IsExpired(const AuTime& asOf) const
	{
		if(!m_Job)
			return true;										// A job offer with no job needs to expire.
		return m_Job->IsOfferExpired(asOf);
	}

	inline bool IsLike(const AuJobOffer& jobOffer) const
	{
		if(!m_Job)
			return !jobOffer.Job();	// True if jobOffer also has no job.
		return m_Job->IsLike(*jobOffer.Job());
	}

	// 'toPerson' is already known to be in the same galaxy as this job.
	// 'rangeInSectors' is the range in whole sectors between the 'toPerson's sector and this job's sector location.
	bool IsOkToOffer(const DaPerson& toPerson, int rangeInSectors) const;

	inline const AuJob* Job() const
	{
		return m_Job;
	}

	inline AuJob::TYPE JobType() const
	{
		if(!m_Job)
			return AuJob::JobTypes;
		return m_Job->Type();
	}

	inline QString Proposal() const
	{
		if(!m_Job)
			return QString();
		return m_Job->Proposal(m_FromName);
	}

	// Returns an appropriate verbal reply when an avatar rejects this job offer after hearing the proposal.
	inline QString RejectProposalReply() const
	{
		if(!m_Job)
			return QString();
		return m_Job->RejectProposalReply();
	}

	// Returns an appropriate verbal reply when an avatar rejects this job offer after hearing the teaser.
	inline QString RejectTeaserReply() const
	{
		if(!m_Job)
			return QString();
		return m_Job->RejectTeaserReply();
	}

	inline QString Teaser() const
	{
		if(!m_Job)
			return QString();
		return m_Job->Teaser(m_FromName);
	}

	inline QString Title() const
	{
		if(!m_Job)
			return QString();
		return m_Job->Title();
	}
};

Q_DECLARE_METATYPE(AuJobOffer)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline unsigned int qHash(const AuJobOffer& key)
{
	return (unsigned int)key.JobType();
}

inline QDataStream& operator<<(QDataStream& out, const AuJobOffer& job)
{
	job >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuJobOffer& job)
{
	job << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuJobOffers : public QList<AuJobOffer>
{
};

#endif // AuJobOffer_H
