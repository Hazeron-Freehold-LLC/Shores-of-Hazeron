//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aujoboffer.cpp,v 1.16 2022/02/19 17:40:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aujoboffer.h"
//
#include "daperson.h"
#include <dgdata.h>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuJobOfferMetaType = qRegisterMetaType<AuJobOffer>();

AuJobOffer::AuJobOffer(const QString& fromName, quint32 fromEmpireId, const AuPoint& fromSectorLoc, GALAXY fromGalaxy,
		const AuJob* job)
{
	m_FromEmpireId = fromEmpireId;
	m_FromGalaxy = fromGalaxy;
	m_FromName = fromName;
	m_FromSectorLoc = fromSectorLoc;
	m_Job = job->Clone();
}

AuJobOffer::AuJobOffer()
{
	m_FromEmpireId = 0;
	m_FromGalaxy = GALAXY_Count;
	m_Job = 0;
}

AuJobOffer::~AuJobOffer()
{
	if(m_Job)
	{
		delete m_Job;
		m_Job = 0;
	}
}

AuJobOffer::AuJobOffer(const AuJobOffer& c)
{
	m_FromEmpireId = c.m_FromEmpireId;
	m_FromGalaxy = c.m_FromGalaxy;
	m_FromName = c.m_FromName;
	m_FromSectorLoc = c.m_FromSectorLoc;

	if(c.m_Job)
		m_Job = c.m_Job->Clone();
	else
		m_Job = 0;
}

const AuJobOffer& AuJobOffer::operator=(const AuJobOffer& c)
{
	m_FromEmpireId = c.m_FromEmpireId;
	m_FromGalaxy = c.m_FromGalaxy;
	m_FromName = c.m_FromName;
	m_FromSectorLoc = c.m_FromSectorLoc;

	if(m_Job)
		delete m_Job;

	if(c.m_Job)
		m_Job = c.m_Job->Clone();
	else
		m_Job = 0;

	return *this;
}

void AuJobOffer::operator>>(QDataStream& out) const
{
	out << m_FromEmpireId;
	out << m_FromName;
	m_FromSectorLoc >> out;
	if(m_Job)
	{
		out << (quint8)m_Job->Type();
		out << *m_Job;
	}
	else
		out << (quint8)AuJob::InvalidJob;
	out << m_FromGalaxy;
}

void AuJobOffer::operator<<(QDataStream& in)
{
	quint8 ui8;

	in >> m_FromEmpireId;
	in >> m_FromName;
	m_FromSectorLoc << in;
	in >> ui8;
	m_Job = AuJob::CreateJob((AuJob::TYPE)ui8);				// Returns 0 if the job type is not valid.
	if(m_Job)
		in >> *m_Job;
	in >> m_FromGalaxy;
}

bool AuJobOffer::operator==(const AuJobOffer& c) const
{
	if(m_FromEmpireId != c.m_FromEmpireId)
		return false;
	if(m_FromGalaxy != c.m_FromGalaxy)
		return false;
	if(m_FromName != c.m_FromName)
		return false;
	if(!m_FromSectorLoc.FuzzyEquals(c.m_FromSectorLoc))
		return false;
	if(!m_Job || !c.m_Job)
		return false;
	return m_Job->Compare(c.m_Job);
}

QString AuJobOffer::HtmlDesc() const
{
	return QString("<html><b>%1</b><br>"
			"<i>%2</i><br>"
			"%3</html>")
		.arg(Title())
		.arg(FromName())
		.arg(Brief());
}

QString AuJobOffer::HtmlToolTip() const
{
	return QString("<html><b>%1</b><br>"
			"<i>%2</i></html>")
		.arg(Title())
		.arg(FromName());
}

bool AuJobOffer::IsOkToOffer(const DaPerson& toPerson, int sectorRange) const
{
	if(!m_Job)
		return false;
	return m_Job->IsOkToOffer(toPerson, FromEmpireId(), sectorRange);
}
