//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audossier.cpp,v 1.33 2022/02/19 18:20:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audossier.h"
//
#include <QDebug>

static int s_AuWarCriteriaMetaType = qRegisterMetaType<AuWarCriteria>();

quint64 AuWarCriteria::MemoryUsage() const
{
	quint64 mem = sizeof(AuWarCriteria);
	mem += (sizeof(quint8) + sizeof(quint8)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
QString AuDocument::StateIcon(DOC_STATE state)
{
	if(state > DOC_Count)
		return QObject::tr("");

	QString stateIcons[DOC_Count+1] =
	{
		QString(":data/new12x12.png"),		// DOC_New,
		QString(":data/fire10x10.png"),	// DOC_Open,
		QString(":data/postponed10x10.png"),	// DOC_Pending,
		QString(":data/required10x10.png"),	// DOC_Closed,
		QString(":data/dropped10.png"),	// DOC_Deleted,
		QString(":data/question10x10.png")	// DOC_Count,
	};

	return stateIcons[state];
}

//static
QString AuDocument::StateName(DOC_STATE state)
{
	if(state > DOC_Count)
		return QObject::tr("Invalid");

	QString stateNames[DOC_Count+1] =
	{
		QObject::tr("New"),		// DOC_New,
		QObject::tr("Open"),	// DOC_Open,
		QObject::tr("Pending"),	// DOC_Pending,
		QObject::tr("Closed"),	// DOC_Closed,
		QObject::tr("Deleted"),	// DOC_Deleted,
		QObject::tr("None")	// DOC_Count,
	};

	return stateNames[state];
}

AuDocument::AuDocument()
{
	m_CreateTime.Set();
	m_State = DOC_New;
}

AuDocument::AuDocument(const AuTime& created)
{
	m_CreateTime = created;
	m_State = DOC_New;
}

AuDocument::~AuDocument()
{
}

AuDocument::AuDocument(const AuDocument& c)
{
	m_CreateTime = c.m_CreateTime;
	m_State = c.m_State;
}

void AuDocument::operator>>(QDataStream& out) const
{
	out << m_CreateTime.UTC();
	out << m_State;
}

void AuDocument::operator<<(QDataStream& in)
{
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_CreateTime = tm;
	in >> m_State;
}

AuDocument* AuDocument::CreateDocument(quint8 docType)
{
	if(docType == AuDocInitOverlord::DocumentType())
		return new AuDocInitOverlord;
	else if(docType == AuDocNewOverlord::DocumentType())
		return new AuDocNewOverlord;
	else if(docType == AuDocNote::DocumentType())
		return new AuDocNote;
	else if(docType == AuDocReciprocalStance::DocumentType())
		return new AuDocReciprocalStance;
	else if(docType == AuDocSetStance::DocumentType())
		return new AuDocSetStance;
	else if(docType == AuDocTrespass::DocumentType())
		return new AuDocTrespass;
	else if(docType == AuDocTrespassed::DocumentType())
		return new AuDocTrespassed;
	else if(docType == AuDocUnitContact::DocumentType())
		return new AuDocUnitContact;
	else if(docType == AuDocUnitDestroyed::DocumentType())
		return new AuDocUnitDestroyed;
	else if(docType == AuDocUnitKilled::DocumentType())
		return new AuDocUnitKilled;
	else if(docType == AuDocCityTrespassCity::DocumentType())
		return new AuDocCityTrespassCity;
	else if(docType == AuDocCityTrespassStation::DocumentType())
		return new AuDocCityTrespassStation;
	else if(docType == AuDocStationTrespassCity::DocumentType())
		return new AuDocStationTrespassCity;
	else if(docType == AuDocStationTrespassStation::DocumentType())
		return new AuDocStationTrespassStation;
	else if(docType == AuDocCitizenKilled::DocumentType())
		return new AuDocCitizenKilled;
	else if(docType == AuDocCityRevolt::DocumentType())
		return new AuDocCityRevolt;
	else if(docType == AuDocVehicleDestroyed::DocumentType())
		return new AuDocVehicleDestroyed;
	else if(docType == AuDocSpaceshipDestroyed::DocumentType())
		return new AuDocSpaceshipDestroyed;
	else if(docType == AuDocStarshipDestroyed::DocumentType())
		return new AuDocStarshipDestroyed;
	else if(docType == AuDocSpaceStationDestroyed::DocumentType())
		return new AuDocSpaceStationDestroyed;
	else if(docType == AuDocSpaceshipCaptured::DocumentType())
		return new AuDocSpaceshipCaptured;
	else if(docType == AuDocStarshipCaptured::DocumentType())
		return new AuDocStarshipCaptured;
	else if(docType == AuDocSpaceStationCaptured::DocumentType())
		return new AuDocSpaceStationCaptured;
	else if(docType == AuDocSpaceshipPlundered::DocumentType())
		return new AuDocSpaceshipPlundered;
	else if(docType == AuDocStarshipPlundered::DocumentType())
		return new AuDocStarshipPlundered;
	else if(docType == AuDocSpaceStationPlundered::DocumentType())
		return new AuDocSpaceStationPlundered;
	else if(docType == AuDocCityCaptured::DocumentType())
		return new AuDocCityCaptured;
	else if(docType == AuDocBaseCaptured::DocumentType())
		return new AuDocBaseCaptured;
	else if(docType == AuDocDiplomaticMessageReceived::DocumentType())
		return new AuDocDiplomaticMessageReceived;
	else if(docType == AuDocDiplomaticMessageSent::DocumentType())
		return new AuDocDiplomaticMessageSent;
	else if(docType == AuDocAvatarKilled::DocumentType())
		return new AuDocAvatarKilled;
	else if(docType == AuDocOfficerKilled::DocumentType())
		return new AuDocOfficerKilled;
	else if(docType == AuDocCrewKilled::DocumentType())
		return new AuDocCrewKilled;
	else if(docType == AuDocTroopKilled::DocumentType())
		return new AuDocTroopKilled;
	else if(docType == AuDocPassengerKilled::DocumentType())
		return new AuDocPassengerKilled;
	else if(docType == AuDocWorkerKilled::DocumentType())
		return new AuDocWorkerKilled;
	else if(docType == AuDocWar::DocumentType())
		return new AuDocWar;
	else if(docType == AuDocStanceDowngraded::DocumentType())
		return new AuDocStanceDowngraded;
	else if(docType == AuDocStanceUpgraded::DocumentType())
		return new AuDocStanceUpgraded;
	else if(docType == AuDocClaimHijacked::DocumentType())
		return new AuDocClaimHijacked;
	else if(docType == AuDocBdDamaged::DocumentType())
		return new AuDocBdDamaged;
	else if(docType == AuDocBdMilitaryDamaged::DocumentType())
		return new AuDocBdMilitaryDamaged;
	else if(docType == AuDocBdDestroyed::DocumentType())
		return new AuDocBdDestroyed;
	else if(docType == AuDocBdCityCapitolDestroyed::DocumentType())
		return new AuDocBdCityCapitolDestroyed;
	else if(docType == AuDocBdSectorCapitolDestroyed::DocumentType())
		return new AuDocBdSectorCapitolDestroyed;
	else if(docType == AuDocBdGalaxyCapitolDestroyed::DocumentType())
		return new AuDocBdGalaxyCapitolDestroyed;
	else if(docType == AuDocBdEmpireCapitolDestroyed::DocumentType())
		return new AuDocBdEmpireCapitolDestroyed;
	else if(docType == AuDocBdMilitaryDestroyed::DocumentType())
		return new AuDocBdMilitaryDestroyed;
	else if(docType == AuDocBdMilitaryHQDestroyed::DocumentType())
		return new AuDocBdMilitaryHQDestroyed;
	//else
		//qWarning() << "error: AuDocument::CreateDocument unknown docType" << docType
			//<< "Did you add a new document type and forget to update CreateDocument?";
	return 0;
}

// Packs this documents's type and data into a byte array using operator>>. Use Unpack to restore.
QByteArray AuDocument::Pack() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)Type();
	out << *this;
	return ba;
}

// Restores a document from a byte array that was stored using Pack.
// The returned pointer is allocated on the heap and becomes the property of the caller.
//static
AuDocument* AuDocument::Unpack(const QByteArray& ba)
{
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 docType;
	in >> docType;
	if(!docType)
	{
		qWarning() << "error: AuDocument::Unpack read docType 0";
		return 0;
	}
	AuDocument* item = CreateDocument(docType);
	if(!item)
		return 0;
	in >> *item;
	return item;
}

quint64 AuDocument::MemoryUsage() const
{
	quint64 mem = sizeof(AuDocument);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTime AuDossier::NewestNewDoc() const
{
	AuTime newest;
	for(const_iterator di = cbegin(); di != cend(); di++)
		if((*di)->State() == AuDocument::DOC_New && (*di)->CreateTime() > newest)
			newest = (*di)->CreateTime();
	return newest;
}

void AuDossier::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ci = constBegin(); ci != constEnd(); ci++)
	{
		out << (*ci)->Type();
		out << **ci;
	}
}

void AuDossier::operator<<(QDataStream& in)
{
	Clear();

	quint16 docs;
	in >> docs;
	quint8 type;
	while(docs--)
	{
		in >> type;
		AuDocument* doc = AuDocument::CreateDocument(type);
		if(doc)
		{
			in >> *doc;
			append(doc);
		}
		else
			qWarning() << "AuDossier::operator<< received invalid doc type" << type;
	}
}

bool AuDossier::File(const AuDocument& doc)
{
	// Attempt to refresh existing document.
	for(iterator mi = begin(); mi != end(); mi++)
	{
		int upd = (*mi)->Update(doc);
		if(upd)
			return upd == 2;	// Existing document updated, whether or not it changed.
	}

	// Existing document not found, store a new document.
	append(doc.Clone());
	return true;
}

AuTime AuDossier::FirstContact() const
{
	AuTime firstMem;
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if(firstMem.IsNull() || (*mi)->CreateTime() < firstMem)
			firstMem = (*mi)->CreateTime();
	return firstMem;
}

AuDocument::DOC_STATE AuDossier::NotableState() const
{
	AuDocument::DOC_STATE nState = AuDocument::DOC_Count;
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi)->State() < nState)
			nState = (*mi)->State();
	return nState;
}

// 'crit' key is AuDocument::DocumentType, value is number of new and open documents of the type that cause a war declaration.
bool AuDossier::CheckWarCriteria(const AuWarCriteria& crit) const
{
	// Empty crit means no criteria to trigger war.
	if(crit.isEmpty())
		return false;

	AuWarCriteria::const_iterator ki = crit.find(AuDocCitizenKilled::DocumentType());

	QHash<quint8, int> newdocs;	// key is AuDocument::DocumentType, value is the number of those documents that are new.
	for(AuDossier::const_iterator di = cbegin(); di != cend(); di++)
	{
		// Some documents are not a factor to war criteria.
		if(!(*di)->WarCriteria())
			continue;

		// Only new and open documents automatically trigger war.
		if((*di)->State() != AuDocument::DOC_New && (*di)->State() != AuDocument::DOC_Open)
			continue;

		// Locate the criteria for the document type.
		AuWarCriteria::const_iterator ci = crit.find((*di)->Type());
		if(ci == crit.cend())
			continue;			// Document type is not in the crit; it does not trigger war.
		if(!(*ci))
			continue;			// No count? It should not be there. We don't store zeros.

		// Count the number of new and open docs of the type.
		QHash<quint8, int>::iterator ni = newdocs.find((*di)->Type());
		if(ni == newdocs.end())
			ni = newdocs.insert((*di)->Type(), 1);
		else
			(*ni)++;

		// Test the count against the war criteria.
		if(*ni >= *ci)
			return true;

		if(ki != crit.cend() && dynamic_cast<const AuDocCitizenKilled*>(*di))
		{
			// Citizen killed is a base class of the document type.
			QHash<quint8, int>::iterator ni = newdocs.find(AuDocCitizenKilled::DocumentType());
			if(ni == newdocs.end())
				ni = newdocs.insert(AuDocCitizenKilled::DocumentType(), 1);
			else
				(*ni)++;

			// Test the count against the war criteria for citizen killing in general.
			if(*ni >= *ki)
				return true;
		}
	}
	return false;
}

quint64 AuDossier::MemoryUsage() const
{
	quint64 mem = sizeof(AuDossier);
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		mem += (*mi)->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocUnitContact::AuDocUnitContact(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
		POLITICAL_STANCE theirStance, const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurStance = ourStance;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirStance = theirStance;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocUnitContact::AuDocUnitContact(const AuTime& when) : AuDocument(when)
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = POL_Neutral;
	m_TheirStance = POL_Neutral;
}

AuDocUnitContact::AuDocUnitContact()
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = POL_Neutral;
	m_TheirStance = POL_Neutral;
}

AuDocUnitContact::AuDocUnitContact(const AuDocUnitContact& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurStance = c.m_OurStance;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirStance = c.m_TheirStance;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocUnitContact::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurStance;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirStance;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocUnitContact::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurStance;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirStance;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocUnitContact::Update(const AuDocument& doc)
{
	try
	{
		const AuDocUnitContact& c = dynamic_cast<const AuDocUnitContact&>(doc);

		if(m_OurUnitId != c.m_OurUnitId)
			return 0;
		if(m_TheirUnitId != c.m_TheirUnitId)
			return 0;
		// Absorb additional contact records.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocUnitContact::ListIcon() const
{
	if(m_OurStance == POL_Enemy)
		return QString(":data/DocWar.png");
	else
		return QString(":data/DocUnitContact.png");
}

QString AuDocUnitContact::Subject() const
{
	return QObject::tr("Contact");
}

QString AuDocUnitContact::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Contact with %2</p>"
				"<p>Status: <img src=\"qrc%13\"> %14</p>"
				"<p>Initial contact was made with a faction called %2 at %3 in the %4 system, %5, %6. "
				"A citizen named %7 (%8) reported sighting a %2 unit named %9 (%10).</p>"
				"<p>As a result of diplomatic protocol, they established a political stance of %11 toward us. "
				"We took a political stance of %12 toward them.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(stances[m_TheirStance])
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Contact with %2</p>"
				"<p>Status: <img src=\"qrc%12\"> %13</p>"
				"<p>Initial contact was made with a faction called %2 in the %3 system, %4, %5. "
				"A citizen named %6 (%7) reported sighting a %2 unit named %8 (%9).</p>"
				"<p>As a result of diplomatic protocol, they established a political stance of %10 toward us. "
				"We took a political stance of %11 toward them.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(stances[m_TheirStance])
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_Sector.isEmpty() && m_Sector != QObject::tr("Sector"))
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Contact with %2</p>"
				"<p>Status: <img src=\"qrc%11\"> %12</p>"
				"<p>Initial contact was made with a faction called %2 in %3, %4. "
				"A citizen named %5 (%6) reported sighting a %2 unit named %7 (%8).</p>"
				"<p>As a result of diplomatic protocol, they established a political stance of %9 toward us. "
				"We took a political stance of %10 toward them.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(stances[m_TheirStance])
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(m_Galaxy < GALAXY_Count && m_OurUnitId.IsNull() && !m_TheirUnitId.IsNull())
	{
		// Initial vassal empire contact during new empire creation at startup, just before vassalage declaration doc.
		if(m_OurStance != POL_Enemy)
			return QObject::tr(
					"<p>Star Date %1</p>"
					"<p>Subject: Contact with %2</p>"
					"<p>Status: <img src=\"qrc%7\"> %8</p>"
					"<p>Initial contact was made with a faction called %2 in %3. "
					"We received a deep space transmission from their leader %4 (%5), formally requesting diplomatic relations.</p>"

					"<p>We dispatched an ambassador to meet them. "
					"Their envoy informed our ambassador of their intent to declare vassalage to us. "
					"Per diplomatic protocol, our ambassador responded by declaring a political stance of %6 toward them.</p>"
					)
				.arg(CreateTime().UTC().toString())
				.arg(empireName)
				.arg(GalaxyName((GALAXY)m_Galaxy))
				.arg(m_TheirUnitName)
				.arg(m_TheirUnitId.StringId())
				.arg(stances[m_OurStance])
				.arg(StateIcon())
				.arg(StateName());
		else	// Can't initially join empires with enemy default stance but things can change in the blink of an eye.
			return QObject::tr(
					"<p>Star Date %1</p>"
					"<p>Subject: Contact with %2</p>"
					"<p>Status: <img src=\"qrc%6\"> %7</p>"
					"<p>Initial contact was made with a faction called %2 in %3. "
					"We received a deep space transmission from their leader %4 (%5), formally requesting diplomatic relations.</p>"

					"<p>We dispatched an ambassador to meet them. "
					"Their envoy informed our ambassador of their intent to declare vassalage to us. "
					"Per diplomatic protocol, our ambassador responded by declaring war on them.</p>"
					)
				.arg(CreateTime().UTC().toString())
				.arg(empireName)
				.arg(GalaxyName((GALAXY)m_Galaxy))
				.arg(m_TheirUnitName)
				.arg(m_TheirUnitId.StringId())
				.arg(StateIcon())
				.arg(StateName());
	}
	else
	{
		// Doc made from old information. All that was known was the UTC.
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Contact with %2</p>"
				"<p>Status: <img src=\"qrc%3\"> %4</p>"
				"<p>Initial contact was made with a faction called %2. "
				"No details about the encounter were recorded.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocUnitContact::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocUnitContact) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSetStance::AuDocSetStance(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
		POLITICAL_STANCE theirStance,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurStance = ourStance;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirStance = theirStance;
	m_World = world;
}

AuDocSetStance::AuDocSetStance(const AuTime& when, POLITICAL_STANCE ourStance) : AuDocument(when)
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = ourStance;
	m_TheirStance = POL_Neutral;
}

AuDocSetStance::AuDocSetStance()
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = POL_Neutral;
	m_TheirStance = POL_Neutral;
}

AuDocSetStance::AuDocSetStance(const AuDocSetStance& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurStance = c.m_OurStance;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirStance = c.m_TheirStance;
	m_World = c.m_World;
}

void AuDocSetStance::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurStance;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirStance;
	out << m_World;
}

void AuDocSetStance::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurStance;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirStance;
	in >> m_World;
}

int AuDocSetStance::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocSetStance::ListIcon() const
{
	if(m_OurStance == POL_Enemy)
		return QString(":data/DocWar.png");
	else
		return QString(":data/DocSetStance.png");
}

QString AuDocSetStance::Subject() const
{
	QString stances[] =
	{
		QObject::tr("Friend"),	// POL_Friend
		QObject::tr("Neutral"),	// POL_Neutral
		QObject::tr("Enemy"),	// POL_Enemy
		QObject::tr("Empire"),	// POL_Empire
		QObject::tr("Vassal"),	// POL_Vassal
	};

	return QObject::tr("%1 Political Stance").arg(stances[m_OurStance]);
}

QString AuDocSetStance::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Political Stance to %2</p>"
				"<p>Status: <img src=\"qrc%11\"> %12</p>"
				"<p>Our political stance toward %2 was changed to %10. "
				"Their stance toward us was %9 at the time.</p>"
				"<p>This was changed by %7 (%8) at %3 in the %4 system, %5, %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(stances[m_TheirStance])
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Political Stance to %2</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>Our political stance toward %2 was changed to %9. "
				"Their stance toward us was %8 at the time.</p>"
				"<p>This was changed by %6 (%7) in the %3 system, %4, %5.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(stances[m_TheirStance])
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		// Doc made from old information. All that was known was the UTC and our stance.
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Political Stance to %2</p>"
				"<p>Status: <img src=\"qrc%4\"> %5</p>"
				"<p>Our political stance toward %2 was changed to %3. "
				"No details about this change in diplomatic policy were recorded.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSetStance::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSetStance) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocInitOverlord::AuDocInitOverlord(const QString& ourUnitName, POLITICAL_STANCE theirStance, GALAXY galaxy, const QString& sector)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_TheirStance = theirStance;
}

AuDocInitOverlord::AuDocInitOverlord()
{
	m_Galaxy = GALAXY_Count;
	m_TheirStance = POL_Neutral;
}

AuDocInitOverlord::AuDocInitOverlord(const AuDocInitOverlord& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_TheirStance = c.m_TheirStance;
}

void AuDocInitOverlord::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_TheirStance;
}

void AuDocInitOverlord::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_TheirStance;
}

int AuDocInitOverlord::Update(const AuDocument& doc)
{
	try
	{
		const AuDocInitOverlord& c = dynamic_cast<const AuDocInitOverlord&>(doc);

		// Reject additional init overlord records for the same dossier.
		Q_UNUSED(c);
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocInitOverlord::ListIcon() const
{
	return QString(":data/DocInitOverlord.png");
}

QString AuDocInitOverlord::Subject() const
{
	return QObject::tr("Initial Vassalage");
}

QString AuDocInitOverlord::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(!m_Sector.isEmpty() && m_Sector != QObject::tr("Sector"))
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Initial Vassalage to %2</p>"
					"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>%5 established our empire in %3, %4.</p>"

				"<p>We immediately sent a deep space transmission to %2, formally requesting diplomatic relations. "
				"They responded by dispatching an ambassador.</p>"

				"<p>Our envoy informed their ambassador of our desire to declare vassalage. "
				"Their ambassador responded favorably, establishing an initial political stance of %6 toward us.</p>"

				"<p>Upon hearing their reaction, vassalage to them was declared, by decree of %5. "
				"Henceforth, we will send them 10% of our treasury daily as tribute. "
				"Tributes will commence once we have a capital and money to send them.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(stances[m_TheirStance])
			.arg(StateIcon())
			.arg(StateName());
	else
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Initial Vassalage to %2</p>"
					"<p>Status: <img src=\"qrc%6\"> %7</p>"
				"<p>%4 established our empire in %3.</p>"

				"<p>We immediately sent a deep space transmission to %2, formally requesting diplomatic relations. "
				"They responded by dispatching an ambassador.</p>"

				"<p>Our envoy informed their ambassador of our desire to declare vassalage. "
				"Their ambassador responded favorably, establishing an initial political stance of %5 toward us.</p>"

				"<p>Upon hearing their reaction, vassalage to them was declared, by decree of %4. "
				"Henceforth, we will send them 10% of our treasury daily as tribute. "
				"Tributes will commence once we have a capital and money to send them.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(stances[m_TheirStance])
			.arg(StateIcon())
			.arg(StateName());
}

quint64 AuDocInitOverlord::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocInitOverlord) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocTrespass::AuDocTrespass(const AuId& ourUnitId, const QString& ourUnitName,
		const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocTrespass::AuDocTrespass()
{
	m_Galaxy = GALAXY_Count;
}

AuDocTrespass::AuDocTrespass(const AuDocTrespass& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocTrespass::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocTrespass::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocTrespass::Update(const AuDocument& doc)
{
	try
	{
		const AuDocTrespass& c = dynamic_cast<const AuDocTrespass&>(doc);

		if(m_OurUnitId != c.m_OurUnitId)
			return 0;
		if(m_TheirUnitId != c.m_TheirUnitId)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocTrespass::ListIcon() const
{
	return QString(":data/DocTrespass.png");
}

QString AuDocTrespass::Subject() const
{
	return QObject::tr("Trespasser");
}

QString AuDocTrespass::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>Trespass was reported by %2 at %3 in the %4 system, %5, %6. "
				"A citizen named %7 (%8) reported a %2 outpost called %9.</p>"
				"<p>This is an act of war. "
				"For the safety of our citizens, we immediately elevated our political stance toward them to enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>Trespass was reported by %2 in the %3 system, %4, %5. "
				"A citizen named %6 (%7) reported a %2 outpost called %8.</p>"
				"<p>This is an act of war. "
				"For the safety of our citizens, we immediately elevated our political stance toward them to enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>Trespass was reported by %2 in %3, %4. "
				"A citizen named %5 (%6) reported a %2 outpost called %7.</p>"
				"<p>This is an act of war. "
				"For the safety of our citizens, we immediately elevated our political stance toward them to enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocTrespass::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocTrespass) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocUnitKilled::AuDocUnitKilled(const AuId& ourUnitId, const QString& ourUnitName,
		const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocUnitKilled::AuDocUnitKilled()
{
	m_Galaxy = GALAXY_Count;
}

AuDocUnitKilled::AuDocUnitKilled(const AuDocUnitKilled& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocUnitKilled::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocUnitKilled::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocUnitKilled::Update(const AuDocument& doc)
{
	try
	{
		const AuDocUnitKilled& c = dynamic_cast<const AuDocUnitKilled&>(doc);

		if(m_OurUnitId != c.m_OurUnitId)
			return 0;
		// Absorb additional kill documents for the same unit.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocUnitKilled::ListIcon() const
{
	return QString(":data/DocUnitKilled.png");
}

QString AuDocUnitKilled::Subject() const
{
	return QObject::tr("Unit Killed");
}

QString AuDocUnitKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Unit Killed by %2</p>"
				"<p>Status: <img src=\"qrc%11\"> %12</p>"
				"<p>One of our units was destroyed today by the %2 at %3 in the %4 system, %5, %6. "
				"Our unit was identified as %7 (%8). "
				"It was destroyed by a %2 unit named %9 (%10).</p>"
				"<p>Our only recourse to this grave incident was to declare %2 to be our enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Unit Killed by %2</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>One of our units was destroyed today by the %2 in the %3 system, %4, %5. "
				"Our unit was identified as %6 (%7). "
				"It was destroyed by a %2 unit named %8 (%9).</p>"
				"<p>Our only recourse to this grave incident was to declare %2 to be our enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Unit Killed by %2</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our units was destroyed today by the %2 in %3, %4. "
				"Our unit was identified as %5 (%6). "
				"It was destroyed by a %2 unit named %8 (%8).</p>"
				"<p>Our only recourse to this grave incident was to declare %2 to be our enemy.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocUnitKilled::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocUnitKilled) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocReciprocalStance::AuDocReciprocalStance(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
		const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurStance = ourStance;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocReciprocalStance::AuDocReciprocalStance()
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = POL_Neutral;
}

AuDocReciprocalStance::AuDocReciprocalStance(const AuDocReciprocalStance& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurStance = c.m_OurStance;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocReciprocalStance::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurStance;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocReciprocalStance::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurStance;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocReciprocalStance::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocReciprocalStance::ListIcon() const
{
	if(m_OurStance == POL_Enemy)
		return QString(":data/DocWar.png");
	else
		return QString(":data/DocReciprocalStance.png");
}

QString AuDocReciprocalStance::Subject() const
{
	QString stances[] =
	{
		QObject::tr("Friend"),	// POL_Friend
		QObject::tr("Neutral"),	// POL_Neutral
		QObject::tr("Enemy"),	// POL_Enemy
		QObject::tr("Empire"),	// POL_Empire
		QObject::tr("Vassal"),	// POL_Vassal
	};

	return QObject::tr("Reciprocated %1 Stance").arg(stances[m_OurStance]);
}

QString AuDocReciprocalStance::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: Reciprocated Stance of %2</p>"
				"<p>Status: <img src=\"qrc%4\"> %5</p>"
			"<p>The %2 downgraded their political stance toward us to %3.</p>"
			"<p>Diplomatic protocol mandated that we reciprocate. "
			"Our stance toward them was change to %3.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(empireName)
		.arg(stances[m_OurStance])
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocReciprocalStance::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocReciprocalStance) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocNewOverlord::AuDocNewOverlord(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurStance = ourStance;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_World = world;
}

AuDocNewOverlord::AuDocNewOverlord()
{
	m_Galaxy = GALAXY_Count;
	m_OurStance = POL_Neutral;
}

AuDocNewOverlord::AuDocNewOverlord(const AuDocNewOverlord& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurStance = c.m_OurStance;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_World = c.m_World;
}

void AuDocNewOverlord::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurStance;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_World;
}

void AuDocNewOverlord::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurStance;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_World;
}

int AuDocNewOverlord::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocNewOverlord::ListIcon() const
{
	if(m_OurStance == POL_Enemy)
		return QString(":data/DocWar.png");
	else
		return QString(":data/DocNewOverlord.png");
}

QString AuDocNewOverlord::Subject() const
{
	return QObject::tr("Terminated Vassalage");
}

QString AuDocNewOverlord::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Terminated Vassalage to %2</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>Our allegiance to %2 was terminated in the process of swearing allegiance to a new overlord. "
				"Our political stance toward them is now %9.</p>"
				"<p>The governing official who initiated the change was %7 (%8) at %3 in the %4 system, %5, %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Terminated Vassalage to %2</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>Our allegiance to %2 was terminated in the process of swearing allegiance to a new overlord. "
				"Our political stance toward them is now %8.</p>"
				"<p>The governing official who initiated the change was %6 (%7) in the %3 system, %4, %5.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Terminated Vassalage to %2</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>Our allegiance to %2 was terminated in the process of swearing allegiance to a new overlord. "
				"Our political stance toward them is now %7.</p>"
				"<p>The governing official who initiated the change was %5 (%6) in %3, %4.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(stances[m_OurStance])
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocNewOverlord::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocNewOverlord) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocUnitDestroyed::AuDocUnitDestroyed(const AuId& ourUnitId, const QString& ourUnitName,
		const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocUnitDestroyed::AuDocUnitDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocUnitDestroyed::AuDocUnitDestroyed(const AuDocUnitDestroyed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocUnitDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocUnitDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocUnitDestroyed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocUnitDestroyed& c = dynamic_cast<const AuDocUnitDestroyed&>(doc);

		if(m_TheirUnitId != c.m_TheirUnitId)
			return 0;
		// Absorb repeated reports of destroying the same unit.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocUnitDestroyed::ListIcon() const
{
	return QString(":data/DocUnitDestroyed.png");
}

QString AuDocUnitDestroyed::Subject() const
{
	return QObject::tr("Enemy Unit Destroyed");
}

QString AuDocUnitDestroyed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Enemy %2 Unit Destroyed</p>"
				"<p>Status: <img src=\"qrc%11\"> %12</p>"
				"<p>%2 was declared an enemy when one of our units destroyed one of their units. "
				"This incident occurred at %3 in the %4 system, %5, %6.</p>"
				"<p>%7 (%8) acted bravely in self defense to preserve our territorial integrity and "
				"the safety and honor of our citizens. "
				"The aggressive %2 unit %9 (%10) had to be destroyed. "
				"We, of course, regret the loss of life.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Enemy %2 Unit Destroyed</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>%2 was declared an enemy when one of our units destroyed one of their units. "
				"This incident occurred in the %3 system, %4, %5.</p>"
				"<p>%6 (%7) acted bravely in self defense to preserve our territorial integrity and "
				"the safety and honor of our citizens. "
				"The aggressive %2 unit %8 (%9) had to be destroyed. "
				"We, of course, regret the loss of life.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Enemy %2 Unit Destroyed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>%2 was declared an enemy when one of our units destroyed one of their units. "
				"This incident occurred in %3, %4.</p>"
				"<p>%5 (%6) acted bravely in self defense to preserve our territorial integrity and "
				"the safety and honor of our citizens. "
				"The aggressive %2 unit %7 (%8) had to be destroyed. "
				"We, of course, regret the loss of life.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocUnitDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocUnitDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocTrespassed::AuDocTrespassed(const AuId& ourUnitId, const QString& ourUnitName,
		const AuId& theirUnitId, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocTrespassed::AuDocTrespassed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocTrespassed::AuDocTrespassed(const AuDocTrespassed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocTrespassed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocTrespassed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocTrespassed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocTrespassed& c = dynamic_cast<const AuDocTrespassed&>(doc);

		if(m_OurUnitId != c.m_OurUnitId)
			return 0;
		if(m_TheirUnitId != c.m_TheirUnitId)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocTrespassed::ListIcon() const
{
	return QString(":data/DocTrespassed.png");
}

QString AuDocTrespassed::Subject() const
{
	return QObject::tr("Territorial Dispute");
}

QString AuDocTrespassed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Territorial Dispute with %2</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>The %2 have declared us to be their enemy due to a territorial dispute at %3 in the %4 system, %5, %6. "
				"For the safety of our citizens, we reciprocated by changing our political stance toward them to be enemy.</p>"
				"<p>%8 claims the presence of %7 is a blatant provocation. "
				"They insist it is an illegal violation of their territorial integrity.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Territorial Dispute with %2</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>The %2 have declared us to be their enemy due to a territorial dispute in the %3 system, %4, %5. "
				"For the safety of our citizens, we reciprocated by changing our political stance toward them to be enemy.</p>"
				"<p>%7 claims the presence of %6 is a blatant provocation. "
				"They insist it is an illegal violation of their territorial integrity.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Territorial Dispute with %2</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>The %2 have declared us to be their enemy due to a territorial dispute in %3, %4. "
				"For the safety of our citizens, we reciprocated by changing our political stance toward them to be enemy.</p>"
				"<p>%6 claims the presence of %5 is a blatant provocation. "
				"They insist it is an illegal violation of their territorial integrity.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocTrespassed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocTrespassed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocNote::AuDocNote(const AuId& ourUnitId, const QString& ourUnitName, const QString& text,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_Text = text;
	m_World = world;
}

AuDocNote::AuDocNote()
{
	m_Galaxy = GALAXY_Count;
}

AuDocNote::AuDocNote(const AuDocNote& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_Text = c.m_Text;
	m_World = c.m_World;
}

void AuDocNote::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_Text;
	out << m_World;
}

void AuDocNote::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_Text;
	in >> m_World;
}

int AuDocNote::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocNote::ListIcon() const
{
	return QString(":data/DocNote.png");
}

QString AuDocNote::Subject() const
{
	return QObject::tr("Note");
}

QString AuDocNote::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Note on %2</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>%7 (%8) entered the following note at %3 in the %4 system, %5, %6.</p>"
				"%9"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_Text)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Note</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>%6 (%7) entered the following note in the %3 system, %4, %5.</p>"
				"%8"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_Text)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Note</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>%5 (%6) entered the following note in %3, %4.</p>"
				"%7"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_OurUnitId.StringId())
			.arg(m_Text)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocNote::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocNote) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_Text);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCityTrespassCity::AuDocCityTrespassCity(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocCityTrespassCity::AuDocCityTrespassCity()
{
	m_Galaxy = GALAXY_Count;
}

AuDocCityTrespassCity::AuDocCityTrespassCity(const AuDocCityTrespassCity& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocCityTrespassCity::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocCityTrespassCity::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocCityTrespassCity::Update(const AuDocument& doc)
{
	try
	{
		const AuDocCityTrespassCity& c = dynamic_cast<const AuDocCityTrespassCity&>(doc);

		if(m_TheirUnitName != c.m_TheirUnitName)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocCityTrespassCity::ListIcon() const
{
	return QString(":data/DocCityTrespassCity.png");
}

QString AuDocCityTrespassCity::Subject() const
{
	return QObject::tr("Trespass by City %1").arg(m_TheirUnitName);
}

QString AuDocCityTrespassCity::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %8</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>A trespassing %2 city was reported on %3 in the %4 system, %5, %6. "
				"Vigilant citizens in our city of %7 detected the trespassing city; it is called %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %7</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>A trespassing %2 city was reported in the %3 system, %4, %5. "
				"Vigilant citizens in our city of %6 detected the trespassing city; it is called %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %6</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>A trespassing %2 city was reported in %3, %4. "
				"Vigilant citizens in our city of %5 detected the trespassing city; it is called %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocCityTrespassCity::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocCityTrespassCity) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCityTrespassStation::AuDocCityTrespassStation(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocCityTrespassStation::AuDocCityTrespassStation()
{
	m_Galaxy = GALAXY_Count;
}

AuDocCityTrespassStation::AuDocCityTrespassStation(const AuDocCityTrespassStation& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocCityTrespassStation::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocCityTrespassStation::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocCityTrespassStation::Update(const AuDocument& doc)
{
	try
	{
		const AuDocCityTrespassStation& c = dynamic_cast<const AuDocCityTrespassStation&>(doc);

		if(m_TheirUnitName != c.m_TheirUnitName)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocCityTrespassStation::ListIcon() const
{
	return QString(":data/DocCityTrespassStation.png");
}

QString AuDocCityTrespassStation::Subject() const
{
	return QObject::tr("Trespass by Station %1").arg(m_TheirUnitName);
}

QString AuDocCityTrespassStation::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %8</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>A trespassing %2 space station was reported orbiting %3 in the %4 system, %5, %6. "
				"Vigilant citizens in our city of %7 detected the trespassing station; it is called %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %7</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>A trespassing %2 space station was reported orbiting in the %3 system, %4, %5. "
				"Vigilant citizens in our city of %6 detected the trespassing station; it is called %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %6</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>A trespassing %2 space station was reported orbiting in %3, %4. "
				"Vigilant citizens in our city of %5 detected the trespassing station; it is called %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocCityTrespassStation::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocCityTrespassStation) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStationTrespassCity::AuDocStationTrespassCity(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocStationTrespassCity::AuDocStationTrespassCity()
{
	m_Galaxy = GALAXY_Count;
}

AuDocStationTrespassCity::AuDocStationTrespassCity(const AuDocStationTrespassCity& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocStationTrespassCity::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocStationTrespassCity::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocStationTrespassCity::Update(const AuDocument& doc)
{
	try
	{
		const AuDocStationTrespassCity& c = dynamic_cast<const AuDocStationTrespassCity&>(doc);

		if(m_TheirUnitName != c.m_TheirUnitName)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocStationTrespassCity::ListIcon() const
{
	return QString(":data/DocStationTrespassCity.png");
}

QString AuDocStationTrespassCity::Subject() const
{
	return QObject::tr("Trespass by City %1").arg(m_TheirUnitName);
}

QString AuDocStationTrespassCity::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %8</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>A trespassing %2 city was reported on %3 in the %4 system, %5, %6. "
				"Alert crew aboard our station %7 detected the trespassing city; it is called %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %7</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>A trespassing %2 city was reported in the %3 system, %4, %5. "
				"Alert crew aboard our station %6 detected the trespassing city; it is called %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 City %6</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>A trespassing %2 city was reported in %3, %4. "
				"Alert crew aboard our station %5 detected the trespassing city; it is called %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocStationTrespassCity::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStationTrespassCity) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStationTrespassStation::AuDocStationTrespassStation(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocStationTrespassStation::AuDocStationTrespassStation()
{
	m_Galaxy = GALAXY_Count;
}

AuDocStationTrespassStation::AuDocStationTrespassStation(const AuDocStationTrespassStation& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocStationTrespassStation::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocStationTrespassStation::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocStationTrespassStation::Update(const AuDocument& doc)
{
	try
	{
		const AuDocStationTrespassStation& c = dynamic_cast<const AuDocStationTrespassStation&>(doc);
		if(m_TheirUnitName != c.m_TheirUnitName)
			return 0;
		// Absorb duplicate trespass reports.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocStationTrespassStation::ListIcon() const
{
	return QString(":data/DocStationTrespassStation.png");
}

QString AuDocStationTrespassStation::Subject() const
{
	return QObject::tr("Trespass by Station %1").arg(m_TheirUnitName);
}

QString AuDocStationTrespassStation::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %8</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>A trespassing %2 space station was reported orbiting %3 in the %4 system, %5, %6. "
				"Alert crew aboard our station %7 detected the trespassing station; it is called %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %7</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>A trespassing %2 space station was reported orbiting in the %3 system, %4, %5. "
				"Alert crew aboard our station %6 detected the trespassing station; it is called %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Trespass by %2 Station %6</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>A trespassing %2 space station was reported orbiting in %3, %4. "
				"Alert crew aboard our station %5 detected the trespassing station; it is called %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocStationTrespassStation::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStationTrespassStation) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCitizenKilled::AuDocCitizenKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocCitizenKilled::AuDocCitizenKilled()
{
	m_Galaxy = GALAXY_Count;
}

AuDocCitizenKilled::AuDocCitizenKilled(const AuDocCitizenKilled& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocCitizenKilled::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocCitizenKilled::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocCitizenKilled::Update(const AuDocument& doc)
{
	try
	{
		const AuDocCitizenKilled& c = dynamic_cast<const AuDocCitizenKilled&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return false;
	}
}

QString AuDocCitizenKilled::ListIcon() const
{
	return QString(":data/DocCitizenKilled.png");
}

QString AuDocCitizenKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocCitizenKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our citizens was killed today at %3 in the %4 system, %5, %6. "
				"A funeral was held for the victim, whose name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our citizens was killed today in the %3 system, %4, %5. "
				"A funeral was held for the victim, whose name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our citizens was killed today in %3, %4. "
				"A funeral was held for the victim, whose name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocCitizenKilled::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocCitizenKilled) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocVehicleDestroyed::AuDocVehicleDestroyed(const AuId& ourUnitId, const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitId = ourUnitId;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocVehicleDestroyed::AuDocVehicleDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocVehicleDestroyed::AuDocVehicleDestroyed(const AuDocVehicleDestroyed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitId = c.m_OurUnitId;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocVehicleDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitId;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocVehicleDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitId;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocVehicleDestroyed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocVehicleDestroyed& c = dynamic_cast<const AuDocVehicleDestroyed&>(doc);
		if(m_OurUnitId != c.m_OurUnitId)
			return 0;
		// Absorb additional kill documents for the same unit.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocVehicleDestroyed::ListIcon() const
{
	return QString(":data/DocVehicleDestroyed.png");
}

QString AuDocVehicleDestroyed::Subject() const
{
	return QObject::tr("%1 Destroyed").arg(m_OurUnitName);
}

QString AuDocVehicleDestroyed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Destroyed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our vehicles was destroyed today at %3 in the %4 system, %5, %6. "
				"The wreckage of the %7 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Destroyed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our vehicles was destroyed today in the %3 system, %4, %5. "
				"The wreckage of the %6 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Destroyed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our vehicles was destroyed today in %3, %4. "
				"The wreckage of the %5 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocVehicleDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocVehicleDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceshipDestroyed::AuDocSpaceshipDestroyed(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceshipDestroyed::AuDocSpaceshipDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceshipDestroyed::AuDocSpaceshipDestroyed(const AuDocSpaceshipDestroyed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceshipDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceshipDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceshipDestroyed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceshipDestroyed& c = dynamic_cast<const AuDocSpaceshipDestroyed&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceshipDestroyed::ListIcon() const
{
	return QString(":data/DocSpaceshipDestroyed.png");
}

QString AuDocSpaceshipDestroyed::Subject() const
{
	return QObject::tr("Spaceship %1 Destroyed").arg(m_OurUnitName);
}

QString AuDocSpaceshipDestroyed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %7 Destroyed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our spaceships, the %7, was destroyed today at %3 in the %4 system, %5, %6. "
				"The wreckage of %7 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %6 Destroyed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our spaceships, the %6, was destroyed today in the %3 system, %4, %5. "
				"The wreckage of %6 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %5 Destroyed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our spaceships, the %5, was destroyed today in %3, %4. "
				"The wreckage of %5 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceshipDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceshipDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStarshipDestroyed::AuDocStarshipDestroyed(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocStarshipDestroyed::AuDocStarshipDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocStarshipDestroyed::AuDocStarshipDestroyed(const AuDocStarshipDestroyed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocStarshipDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocStarshipDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocStarshipDestroyed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocStarshipDestroyed& c = dynamic_cast<const AuDocStarshipDestroyed&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocStarshipDestroyed::ListIcon() const
{
	return QString(":data/DocStarshipDestroyed.png");
}

QString AuDocStarshipDestroyed::Subject() const
{
	return QObject::tr("Starship %1 Destroyed").arg(m_OurUnitName);
}

QString AuDocStarshipDestroyed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %7 Destroyed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our starships, the %7, was destroyed today at %3 in the %4 system, %5, %6. "
				"The wreckage of %7 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %6 Destroyed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our starships, the %6, was destroyed today in the %3 system, %4, %5. "
				"The wreckage of %6 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %5 Destroyed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our starships, the %5, was destroyed today in %3, %4. "
				"The wreckage of %5 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocStarshipDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStarshipDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceStationDestroyed::AuDocSpaceStationDestroyed(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceStationDestroyed::AuDocSpaceStationDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceStationDestroyed::AuDocSpaceStationDestroyed(const AuDocSpaceStationDestroyed& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceStationDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceStationDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceStationDestroyed::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceStationDestroyed& c = dynamic_cast<const AuDocSpaceStationDestroyed&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceStationDestroyed::ListIcon() const
{
	return QString(":data/DocSpaceStationDestroyed.png");
}

QString AuDocSpaceStationDestroyed::Subject() const
{
	return QObject::tr("Space Station %1 Destroyed").arg(m_OurUnitName);
}

QString AuDocSpaceStationDestroyed::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %7 Destroyed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our space stations, the %7, was destroyed today at %3 in the %4 system, %5, %6. "
				"The wreckage of %7 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %6 Destroyed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our space stations, the %6, was destroyed today in the %3 system, %4, %5. "
				"The wreckage of %6 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %5 Destroyed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our space stations, the %5, was destroyed today in %3, %4. "
				"The wreckage of %5 was lost.</p>"
				"<p>Reliable witnesses said it was destroyed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceStationDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceStationDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceshipCaptured::AuDocSpaceshipCaptured(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceshipCaptured::AuDocSpaceshipCaptured()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceshipCaptured::AuDocSpaceshipCaptured(const AuDocSpaceshipCaptured& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceshipCaptured::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceshipCaptured::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceshipCaptured::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceshipCaptured& c = dynamic_cast<const AuDocSpaceshipCaptured&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceshipCaptured::ListIcon() const
{
	return QString(":data/DocSpaceshipCaptured.png");
}

QString AuDocSpaceshipCaptured::Subject() const
{
	return QObject::tr("Spaceship %1 Captured").arg(m_OurUnitName);
}

QString AuDocSpaceshipCaptured::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %7 Captured</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our spaceships, the %7, was captured today at %3 in the %4 system, %5, %6.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %6 Captured</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our spaceships, the %6, was captured today in the %3 system, %4, %5.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %5 Captured</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our spaceships, the %5, was captured today in %3, %4.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceshipCaptured::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceshipCaptured) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStarshipCaptured::AuDocStarshipCaptured(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocStarshipCaptured::AuDocStarshipCaptured()
{
	m_Galaxy = GALAXY_Count;
}

AuDocStarshipCaptured::AuDocStarshipCaptured(const AuDocStarshipCaptured& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocStarshipCaptured::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocStarshipCaptured::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocStarshipCaptured::Update(const AuDocument& doc)
{
	try
	{
		const AuDocStarshipCaptured& c = dynamic_cast<const AuDocStarshipCaptured&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocStarshipCaptured::ListIcon() const
{
	return QString(":data/DocStarshipCaptured.png");
}

QString AuDocStarshipCaptured::Subject() const
{
	return QObject::tr("Starship %1 Captured").arg(m_OurUnitName);
}

QString AuDocStarshipCaptured::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %7 Captured</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our starships, the %7, was captured today at %3 in the %4 system, %5, %6.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %6 Captured</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our starships, the %6,  was captured today in the %3 system, %4, %5.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %5 Captured</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our starships, the %5,  was captured today in %3, %4.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocStarshipCaptured::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStarshipCaptured) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceStationCaptured::AuDocSpaceStationCaptured(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceStationCaptured::AuDocSpaceStationCaptured()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceStationCaptured::AuDocSpaceStationCaptured(const AuDocSpaceStationCaptured& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceStationCaptured::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceStationCaptured::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceStationCaptured::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceStationCaptured& c = dynamic_cast<const AuDocSpaceStationCaptured&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceStationCaptured::ListIcon() const
{
	return QString(":data/DocSpaceStationCaptured.png");
}

QString AuDocSpaceStationCaptured::Subject() const
{
	return QObject::tr("Space Station %1 Captured").arg(m_OurUnitName);
}

QString AuDocSpaceStationCaptured::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %7 Captured</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our space stations, the %7, was captured today at %3 in the %4 system, %5, %6.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %6 Captured</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our space stations, the %6, was captured today in the %3 system, %4, %5.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %5 Captured</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our space stations was captured today in %3, %4.</p>"
				"<p>Reliable witnesses said it was captured by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceStationCaptured::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceStationCaptured) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceshipPlundered::AuDocSpaceshipPlundered(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceshipPlundered::AuDocSpaceshipPlundered()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceshipPlundered::AuDocSpaceshipPlundered(const AuDocSpaceshipPlundered& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceshipPlundered::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceshipPlundered::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceshipPlundered::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceshipPlundered& c = dynamic_cast<const AuDocSpaceshipPlundered&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceshipPlundered::ListIcon() const
{
	return QString(":data/DocSpaceshipPlundered.png");
}

QString AuDocSpaceshipPlundered::Subject() const
{
	return QObject::tr("Spaceship %1 Plundered").arg(m_OurUnitName);
}

QString AuDocSpaceshipPlundered::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %7 Plundered</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our spaceships, the %7, was plundered today at %3 in the %4 system, %5, %6.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %6 Plundered</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our spaceships, the %6, was plundered today in the %3 system, %4, %5.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Spaceship %5 Plundered</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our spaceships, the %5, was plundered today in %3, %4.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceshipPlundered::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceshipPlundered) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStarshipPlundered::AuDocStarshipPlundered(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocStarshipPlundered::AuDocStarshipPlundered()
{
	m_Galaxy = GALAXY_Count;
}

AuDocStarshipPlundered::AuDocStarshipPlundered(const AuDocStarshipPlundered& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocStarshipPlundered::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocStarshipPlundered::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocStarshipPlundered::Update(const AuDocument& doc)
{
	try
	{
		const AuDocStarshipPlundered& c = dynamic_cast<const AuDocStarshipPlundered&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocStarshipPlundered::ListIcon() const
{
	return QString(":data/DocStarshipPlundered.png");
}

QString AuDocStarshipPlundered::Subject() const
{
	return QObject::tr("Starship %1 Plundered").arg(m_OurUnitName);
}

QString AuDocStarshipPlundered::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %7 Plundered</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our starships, the %7, was plundered today at %3 in the %4 system, %5, %6.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %6 Plundered</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our starships, the %6, was plundered today in the %3 system, %4, %5.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Starship %5 Plundered</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our starships, the %5, was plundered today in %3, %4.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocStarshipPlundered::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStarshipPlundered) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocSpaceStationPlundered::AuDocSpaceStationPlundered(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocSpaceStationPlundered::AuDocSpaceStationPlundered()
{
	m_Galaxy = GALAXY_Count;
}

AuDocSpaceStationPlundered::AuDocSpaceStationPlundered(const AuDocSpaceStationPlundered& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocSpaceStationPlundered::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocSpaceStationPlundered::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocSpaceStationPlundered::Update(const AuDocument& doc)
{
	try
	{
		const AuDocSpaceStationPlundered& c = dynamic_cast<const AuDocSpaceStationPlundered&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocSpaceStationPlundered::ListIcon() const
{
	return QString(":data/DocSpaceStationPlundered.png");
}

QString AuDocSpaceStationPlundered::Subject() const
{
	return QObject::tr("Space Station %1 Plundered").arg(m_OurUnitName);
}

QString AuDocSpaceStationPlundered::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %7 Plundered</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our space stations, the %7, was plundered today at %3 in the %4 system, %5, %6.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %6 Plundered</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our space stations, the %6, was plundered today in the %3 system, %4, %5.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Space Station %5 Plundered</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our space stations, the %5, was plundered today in %3, %4.</p>"
				"<p>This brazen act of piracy was committed by a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocSpaceStationPlundered::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocSpaceStationPlundered) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCityCaptured::AuDocCityCaptured(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocCityCaptured::AuDocCityCaptured()
{
	m_Galaxy = GALAXY_Count;
}

AuDocCityCaptured::AuDocCityCaptured(const AuDocCityCaptured& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocCityCaptured::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocCityCaptured::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocCityCaptured::Update(const AuDocument& doc)
{
	try
	{
		const AuDocCityCaptured& c = dynamic_cast<const AuDocCityCaptured&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocCityCaptured::ListIcon() const
{
	return QString(":data/DocCityCaptured.png");
}

QString AuDocCityCaptured::Subject() const
{
	return QObject::tr("City of %1 Captured").arg(m_OurUnitName);
}

QString AuDocCityCaptured::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: City of %7 Captured</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>Our city of %7 was captured today on %3 in the %4 system, %5, %6.</p>"
				"<p>Partisans identified the enemy as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: City of %6 Captured</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>Our city of %6 was captured today in the %3 system, %4, %5.</p>"
				"<p>Partisans identified the enemy as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: City of %5 Captured</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>Our city of %5 was captured today in %3, %4.</p>"
				"<p>Partisans identified the enemy as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocCityCaptured::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocCityCaptured) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBaseCaptured::AuDocBaseCaptured(const QString& ourUnitName, quint8 baseIndex, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	m_BaseIndex = baseIndex;
	m_Galaxy = galaxy;
	m_OurUnitName = ourUnitName;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocBaseCaptured::AuDocBaseCaptured()
{
	m_BaseIndex = 0;
	m_Galaxy = GALAXY_Count;
}

AuDocBaseCaptured::AuDocBaseCaptured(const AuDocBaseCaptured& c) : AuDocument(c)
{
	m_BaseIndex = c.m_BaseIndex;
	m_Galaxy = c.m_Galaxy;
	m_OurUnitName = c.m_OurUnitName;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocBaseCaptured::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_BaseIndex;
	out << m_Galaxy;
	out << m_OurUnitName;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocBaseCaptured::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_BaseIndex;
	in >> m_Galaxy;
	in >> m_OurUnitName;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocBaseCaptured::Update(const AuDocument& doc)
{
	try
	{
		const AuDocBaseCaptured& c = dynamic_cast<const AuDocBaseCaptured&>(doc);
		Q_UNUSED(c);

		return 0;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocBaseCaptured::ListIcon() const
{
	return QString(":data/DocBaseCaptured.png");
}

QString AuDocBaseCaptured::Subject() const
{
	return QObject::tr("Military Post %1 Captured in %2")
		.arg(Phonetic('A' + m_BaseIndex))
		.arg(m_OurUnitName);
}

QString AuDocBaseCaptured::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Military Post %7 Captured in %8</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>Military defense base %7 in %8 was captured today on %3 in the %4 system, %5, %6.</p>"
				"<p>Before the base was overrun, the military contingent identified the enemy as a %2 unit named %9.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(Phonetic('A' + m_BaseIndex))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Military Post %6 Captured in %7</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>Military defense base %6 in %7 was captured today in the %3 system, %4, %5.</p>"
				"<p>Before the base was overrun, the military contingent identified the enemy as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(Phonetic('A' + m_BaseIndex))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Military Post %5 Captured in %6</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>Military defense base %5 in %6 was captured today in %3, %4.</p>"
				"<p>Before the base was overrun, the military contingent identified the enemy as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(Phonetic('A' + m_BaseIndex))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocBaseCaptured::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocBaseCaptured) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_OurUnitName);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocDiplomaticMessageReceived::AuDocDiplomaticMessageReceived(const AuId& ourUnitId, const QString& ourUnitName,
		const QString& text)
{
	m_FromUnitId = ourUnitId;
	m_FromUnitName = ourUnitName;
	m_Text = text;
}

AuDocDiplomaticMessageReceived::AuDocDiplomaticMessageReceived()
{
}

AuDocDiplomaticMessageReceived::AuDocDiplomaticMessageReceived(const AuDocDiplomaticMessageReceived& c) : AuDocument(c)
{
	m_FromUnitId = c.m_FromUnitId;
	m_FromUnitName = c.m_FromUnitName;
	m_Text = c.m_Text;
}

void AuDocDiplomaticMessageReceived::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_FromUnitId;
	out << m_FromUnitName;
	out << m_Text;
}

void AuDocDiplomaticMessageReceived::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_FromUnitId;
	in >> m_FromUnitName;
	in >> m_Text;
}

int AuDocDiplomaticMessageReceived::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocDiplomaticMessageReceived::ListIcon() const
{
	return QString(":data/DocDiplomaticMessageReceived.png");
}

QString AuDocDiplomaticMessageReceived::Subject() const
{
	return QObject::tr("Diplomatic Message Received");
}

QString AuDocDiplomaticMessageReceived::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: Diplomatic Message Received</p>"
				"<p>Status: <img src=\"qrc%6\"> %7</p>"
			"<p>A courier delivered the following diplomatic message. "
			"It was sent by %3 (%4) of %2.</p>"
			"%5"
			)
		.arg(CreateTime().UTC().toString())
		.arg(empireName)
		.arg(m_FromUnitName)
		.arg(m_FromUnitId.StringId())
		.arg(m_Text)
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocDiplomaticMessageReceived::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocDiplomaticMessageReceived) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_FromUnitName);
	mem += StringMemoryUsage(m_Text);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocDiplomaticMessageSent::AuDocDiplomaticMessageSent(const AuId& ourUnitId, const QString& ourUnitName, const QString& text)
{
	m_FromUnitId = ourUnitId;
	m_FromUnitName = ourUnitName;
	m_Text = text;
}

AuDocDiplomaticMessageSent::AuDocDiplomaticMessageSent()
{
}

AuDocDiplomaticMessageSent::AuDocDiplomaticMessageSent(const AuDocDiplomaticMessageSent& c) : AuDocument(c)
{
	m_FromUnitId = c.m_FromUnitId;
	m_FromUnitName = c.m_FromUnitName;
	m_Text = c.m_Text;
}

void AuDocDiplomaticMessageSent::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_FromUnitId;
	out << m_FromUnitName;
	out << m_Text;
}

void AuDocDiplomaticMessageSent::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_FromUnitId;
	in >> m_FromUnitName;
	in >> m_Text;
}

int AuDocDiplomaticMessageSent::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocDiplomaticMessageSent::ListIcon() const
{
	return QString(":data/DocDiplomaticMessageSent.png");
}

QString AuDocDiplomaticMessageSent::Subject() const
{
	return QObject::tr("Diplomatic Message Sent");
}

QString AuDocDiplomaticMessageSent::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: Diplomatic Message Sent</p>"
				"<p>Status: <img src=\"qrc%6\"> %7</p>"
			"<p>%3 (%4) sent the following diplomatic message to %2 by courier.</p>"
			"%5"
			)
		.arg(CreateTime().UTC().toString())
		.arg(empireName)
		.arg(m_FromUnitName)
		.arg(m_FromUnitId.StringId())
		.arg(m_Text)
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocDiplomaticMessageSent::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocDiplomaticMessageSent) - sizeof(AuDocument);
	mem += m_FromUnitName.capacity();
	mem += StringMemoryUsage(m_Text);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocAvatarKilled::AuDocAvatarKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
}

AuDocAvatarKilled::AuDocAvatarKilled()
{
}

AuDocAvatarKilled::AuDocAvatarKilled(const AuDocAvatarKilled& c) : AuDocCitizenKilled(c)
{
}

void AuDocAvatarKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
}

void AuDocAvatarKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
}

QString AuDocAvatarKilled::ListIcon() const
{
	return QString(":data/DocAvatarKilled.png");
}

QString AuDocAvatarKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocAvatarKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our avatars was killed today at %3 in the %4 system, %5, %6. "
				"The victim's name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our avatars was killed today in the %3 system, %4, %5. "
				"The victim's name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our avatars was killed today in %3, %4. "
				"The victim's name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocAvatarKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocAvatarKilled) - sizeof(AuDocCitizenKilled);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocOfficerKilled::AuDocOfficerKilled(const QString& ourUnitName, const QString& ourUnitHomeWorld, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
	m_HomeWorld = ourUnitHomeWorld;
}

AuDocOfficerKilled::AuDocOfficerKilled()
{
}

AuDocOfficerKilled::AuDocOfficerKilled(const AuDocOfficerKilled& c) : AuDocCitizenKilled(c)
{
	m_HomeWorld = c.m_HomeWorld;
}

void AuDocOfficerKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
	out << m_HomeWorld;
}

void AuDocOfficerKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
	in >> m_HomeWorld;
}

QString AuDocOfficerKilled::ListIcon() const
{
	return QString(":data/DocOfficerKilled.png");
}

QString AuDocOfficerKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocOfficerKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%10\"> %11</p>"
				"<p>One of our officers was killed today at %3 in the %4 system, %5, %6. "
				"The victim was %7, from %9.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(m_HomeWorld)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our officers was killed today in the %3 system, %4, %5. "
				"The victim was %6, from %8.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(m_HomeWorld)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our officers was killed today in %3, %4. "
				"The victim was %5, from %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(m_HomeWorld)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocOfficerKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocOfficerKilled) - sizeof(AuDocCitizenKilled);
	mem += StringMemoryUsage(m_HomeWorld);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCrewKilled::AuDocCrewKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
}

AuDocCrewKilled::AuDocCrewKilled()
{
}

AuDocCrewKilled::AuDocCrewKilled(const AuDocCrewKilled& c) : AuDocCitizenKilled(c)
{
}

void AuDocCrewKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
}

void AuDocCrewKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
}

QString AuDocCrewKilled::ListIcon() const
{
	return QString(":data/DocCrewKilled.png");
}

QString AuDocCrewKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocCrewKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our starmen was killed today at %3 in the %4 system, %5, %6. "
				"The victim's name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our starmen was killed today in the %3 system, %4, %5. "
				"The victim's name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %9</p>"
				"<p>One of our starmen was killed today in %3, %4. "
				"The victim's name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocCrewKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocCrewKilled) - sizeof(AuDocCitizenKilled);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocTroopKilled::AuDocTroopKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
}

AuDocTroopKilled::AuDocTroopKilled()
{
}

AuDocTroopKilled::AuDocTroopKilled(const AuDocTroopKilled& c) : AuDocCitizenKilled(c)
{
}

void AuDocTroopKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
}

void AuDocTroopKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
}

QString AuDocTroopKilled::ListIcon() const
{
	return QString(":data/DocTroopKilled.png");
}

QString AuDocTroopKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocTroopKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our troops was killed today at %3 in the %4 system, %5, %6. "
				"The victim's name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our troops was killed today in the %3 system, %4, %5. "
				"The victim's name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our troops was killed today in %3, %4. "
				"The victim's name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocTroopKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocTroopKilled) - sizeof(AuDocCitizenKilled);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocPassengerKilled::AuDocPassengerKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
}

AuDocPassengerKilled::AuDocPassengerKilled()
{
}

AuDocPassengerKilled::AuDocPassengerKilled(const AuDocPassengerKilled& c) : AuDocCitizenKilled(c)
{
}

void AuDocPassengerKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
}

void AuDocPassengerKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
}

QString AuDocPassengerKilled::ListIcon() const
{
	return QString(":data/DocPassengerKilled.png");
}

QString AuDocPassengerKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocPassengerKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>A passenger of a spacecraft bearing our flag was killed today at %3 in the %4 system, %5, %6. "
				"The victim's name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>A passenger of a spacecraft bearing our flag was killed today in the %3 system, %4, %5. "
				"The victim's name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>A passenger of a spacecraft bearing our flag was killed today in %3, %4. "
				"The victim's name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocPassengerKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocPassengerKilled) - sizeof(AuDocCitizenKilled);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocWorkerKilled::AuDocWorkerKilled(const QString& ourUnitName, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world) :
	AuDocCitizenKilled(ourUnitName, theirUnitName, galaxy, sector, system, world)
{
}

AuDocWorkerKilled::AuDocWorkerKilled()
{
}

AuDocWorkerKilled::AuDocWorkerKilled(const AuDocWorkerKilled& c) : AuDocCitizenKilled(c)
{
}

void AuDocWorkerKilled::operator>>(QDataStream& out) const
{
	AuDocCitizenKilled::operator>>(out);
}

void AuDocWorkerKilled::operator<<(QDataStream& in)
{
	AuDocCitizenKilled::operator<<(in);
}

QString AuDocWorkerKilled::ListIcon() const
{
	return QString(":data/DocWorkerKilled.png");
}

QString AuDocWorkerKilled::Subject() const
{
	return QObject::tr("%1 Killed").arg(m_OurUnitName);
}

QString AuDocWorkerKilled::Html(const QString& empireName) const
{
	if(!m_World.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %7 Killed</p>"
				"<p>Status: <img src=\"qrc%9\"> %10</p>"
				"<p>One of our workers was killed today at %3 in the %4 system, %5, %6. "
				"The victim's name was %7.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %8.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else if(!m_System.isEmpty())
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %6 Killed</p>"
				"<p>Status: <img src=\"qrc%8\"> %9</p>"
				"<p>One of our workers was killed today in the %3 system, %4, %5. "
				"The victim's name was %6.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %7.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
	else
	{
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %5 Killed</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>One of our workers was killed today in %3, %4. "
				"The victim's name was %5.</p>"
				"<p>With the help of trusted witnesses and DNA evidence, "
				"the murderer was positively identified as a %2 unit named %6.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_OurUnitName)
			.arg(m_TheirUnitName)
			.arg(StateIcon())
			.arg(StateName());
	}
}

quint64 AuDocWorkerKilled::MemoryUsage() const
{
	quint64 mem = AuDocCitizenKilled::MemoryUsage();
	mem += sizeof(AuDocWorkerKilled) - sizeof(AuDocCitizenKilled);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocWar::AuDocWar()
{
}

AuDocWar::AuDocWar(const AuDocWar& c) : AuDocument(c)
{
}

void AuDocWar::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
}

void AuDocWar::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
}

int AuDocWar::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocWar::ListIcon() const
{
	return QString(":data/DocWar.png");
}

QString AuDocWar::Subject() const
{
	return QObject::tr("War Declared");
}

QString AuDocWar::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: War Declared on %2</p>"
			"<p>Status: <img src=\"qrc%3\"> %4</p>"
			"<p>The %2 seem to be trouble makers. "
			"A number of diplomatic incidents were reported due to their actions. "
			"This triggered our automatic war policy.</p>"
			"<p>Due to this unprovoked aggression from the %2, "
			"and to protect the safety of our citizens in the area, "
			"we have declared war on the %2.<p>"
			"<p>May God help us all.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(empireName)
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocWar::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocWar) - sizeof(AuDocument);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStanceDowngraded::AuDocStanceDowngraded(POLITICAL_STANCE theirOldStance, POLITICAL_STANCE theirNewStance,
		const AuId& theirUnitId, const QString& theirUnitName)
{
	m_TheirNewStance = theirNewStance;
	m_TheirOldStance = theirOldStance;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
}

AuDocStanceDowngraded::AuDocStanceDowngraded()
{
	m_TheirNewStance = POL_Neutral;
	m_TheirOldStance = POL_Neutral;
}

AuDocStanceDowngraded::AuDocStanceDowngraded(const AuDocStanceDowngraded& c) : AuDocument(c)
{
	m_TheirNewStance = c.m_TheirNewStance;
	m_TheirOldStance = c.m_TheirOldStance;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
}

void AuDocStanceDowngraded::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_TheirNewStance;
	out << m_TheirOldStance;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
}

void AuDocStanceDowngraded::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_TheirNewStance;
	in >> m_TheirOldStance;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
}

int AuDocStanceDowngraded::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocStanceDowngraded::ListIcon() const
{
	if(m_TheirNewStance == POL_Enemy)
		return QString(":data/DocWar.png");
	else
		return QString(":data/DocStanceDowngraded.png");
}

QString AuDocStanceDowngraded::Subject() const
{
	QString stances[] =
	{
		QObject::tr("Friend"),	// POL_Friend
		QObject::tr("Neutral"),	// POL_Neutral
		QObject::tr("Enemy"),	// POL_Enemy
		QObject::tr("Empire"),	// POL_Empire
		QObject::tr("Vassal"),	// POL_Vassal
	};

	if(m_TheirNewStance == POL_Enemy)
		return QObject::tr("Their Declaration of War");
	else
		return QObject::tr("Their Political Stance Downgraded to %1").arg(stances[m_TheirNewStance]);
}

QString AuDocStanceDowngraded::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(m_TheirNewStance == POL_Enemy)
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Their Declaration of War</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>%2 declared war on us. "
				"They reduced their political stance toward our empire to %4. "
				"Their stance toward us was %3 at the time.</p>"
				"<p>This aggressive change in diplomatic policy was enacted by %5 (%6).</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(stances[m_TheirOldStance])
			.arg(stances[m_TheirNewStance])
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
	else
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Their Political Stance Downgraded to %4</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>%2 reduced their political stance toward our empire to %4. "
				"Their stance toward us was %3 at the time.</p>"
				"<p>This unfriendly change in diplomatic policy was enacted by %5 (%6).</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(stances[m_TheirOldStance])
			.arg(stances[m_TheirNewStance])
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
}

quint64 AuDocStanceDowngraded::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStanceDowngraded) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_TheirUnitName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocStanceUpgraded::AuDocStanceUpgraded(POLITICAL_STANCE theirOldStance, POLITICAL_STANCE theirNewStance, quint8 vassalPercent,
		const AuId& theirUnitId, const QString& theirUnitName)
{
	m_TheirNewStance = theirNewStance;
	m_TheirOldStance = theirOldStance;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_VassalPercent = vassalPercent;
}

AuDocStanceUpgraded::AuDocStanceUpgraded()
{
	m_TheirNewStance = POL_Neutral;
	m_TheirOldStance = POL_Neutral;
	m_VassalPercent = 0;
}

AuDocStanceUpgraded::AuDocStanceUpgraded(const AuDocStanceUpgraded& c) : AuDocument(c)
{
	m_TheirNewStance = c.m_TheirNewStance;
	m_TheirOldStance = c.m_TheirOldStance;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_VassalPercent = c.m_VassalPercent;
}

void AuDocStanceUpgraded::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_TheirNewStance;
	out << m_TheirOldStance;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_VassalPercent;
}

void AuDocStanceUpgraded::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_TheirNewStance;
	in >> m_TheirOldStance;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_VassalPercent;
}

int AuDocStanceUpgraded::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocStanceUpgraded::ListIcon() const
{
	return QString(":data/DocStanceUpgraded.png");
}

QString AuDocStanceUpgraded::Subject() const
{
	QString stances[] =
	{
		QObject::tr("Friend"),	// POL_Friend
		QObject::tr("Neutral"),	// POL_Neutral
		QObject::tr("Enemy"),	// POL_Enemy
		QObject::tr("Empire"),	// POL_Empire
		QObject::tr("Vassal"),	// POL_Vassal
	};

	if(m_TheirNewStance == POL_Vassal)
	{
		if(m_VassalPercent < 10)
			return QObject::tr("Their Declaration of Alliance");
		else
			return QObject::tr("Their Declaration of Vassalage");
	}
	else
		return QObject::tr("Their Political Stance Upgraded to %1").arg(stances[m_TheirNewStance]);
}

QString AuDocStanceUpgraded::Html(const QString& empireName) const
{
	QString stances[] =
	{
		QObject::tr("friend"),	// POL_Friend
		QObject::tr("neutral"),	// POL_Neutral
		QObject::tr("enemy"),	// POL_Enemy
		QObject::tr("empire"),	// POL_Empire
		QObject::tr("vassal"),	// POL_Vassal
	};

	if(m_TheirNewStance == POL_Vassal)
	{
		if(m_VassalPercent == 0)
		{
			// Ally, no tribute
			return QObject::tr(
					"<p>Star Date %1</p>"
					"<p>Subject: Their Declaration of Alliance</p>"
					"<p>Status: <img src=\"qrc%6\"> %7</p>"
					"<p>%2 declared an alliance with us. "
					"They increased their political stance toward our empire to ally. "
					"Their stance toward us was %3 at the time.</p>"
					"<p>This friendly change in diplomatic policy was enacted by %4 (%5).</p>"
					)
				.arg(CreateTime().UTC().toString())
				.arg(empireName)
				.arg(stances[m_TheirOldStance])
				.arg(m_TheirUnitName)
				.arg(m_TheirUnitId.StringId())
				.arg(StateIcon())
				.arg(StateName());
		}
		else if(m_VassalPercent < 10)
		{
			// Ally
			return QObject::tr(
					"<p>Star Date %1</p>"
					"<p>Subject: Their Declaration of Alliance</p>"
					"<p>Status: <img src=\"qrc%7\"> %8</p>"
					"<p>%2 declared an alliance with us. "
					"They generously established a daily tribute of %6%. "
					"Their stance toward us was %3 at the time.</p>"
					"<p>This friendly change in diplomatic policy was enacted by %4 (%5).</p>"
					)
				.arg(CreateTime().UTC().toString())
				.arg(empireName)
				.arg(stances[m_TheirOldStance])
				.arg(m_TheirUnitName)
				.arg(m_TheirUnitId.StringId())
				.arg(m_VassalPercent)
				.arg(StateIcon())
				.arg(StateName());
		}
		else
		{
			// Vassal
			return QObject::tr(
					"<p>Star Date %1</p>"
					"<p>Subject: Their Declaration of Vassalage</p>"
					"<p>Status: <img src=\"qrc%6\"> %7</p>"

					"<p>%2 declared vassalage to our empire. "
					"Daily tribute of %5% will be sent to our capital when their treasury is operational. "
					"It may take them a while to build the infrastructure. "
					"They'll need to declare a capitol and build a bank to accumulate a treasury. "
					"Then they'll need to tax some folks.</p>"

					"<p>Their vassalage to us was enacted by decree of %3 (%4).</p>"
					)
				.arg(CreateTime().UTC().toString())
				.arg(empireName)
				.arg(m_TheirUnitName)
				.arg(m_TheirUnitId.StringId())
				.arg(m_VassalPercent)
				.arg(StateIcon())
				.arg(StateName());
		}
	}
	else
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: Their Political Stance Upgraded to %4</p>"
				"<p>Status: <img src=\"qrc%7\"> %8</p>"
				"<p>%2 improved their political stance toward our empire to %4. "
				"Their stance toward us was %3 at the time.</p>"
				"<p>This friendly change in diplomatic policy was enacted by %5 (%6).</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(empireName)
			.arg(stances[m_TheirOldStance])
			.arg(stances[m_TheirNewStance])
			.arg(m_TheirUnitName)
			.arg(m_TheirUnitId.StringId())
			.arg(StateIcon())
			.arg(StateName());
}

quint64 AuDocStanceUpgraded::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocStanceUpgraded) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_TheirUnitName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocCityRevolt::AuDocCityRevolt(const QString& city, GALAXY galaxy, const QString& sector, const QString& system,
		const QString& world)
{
	m_City = city;
	m_Galaxy = galaxy;
	m_Sector = sector;
	m_System = system;
	m_World = world;
}

AuDocCityRevolt::AuDocCityRevolt()
{
	m_Galaxy = GALAXY_Count;
}

AuDocCityRevolt::AuDocCityRevolt(const AuDocCityRevolt& c) : AuDocument(c)
{
	m_City = c.m_City;
	m_Galaxy = c.m_Galaxy;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_World = c.m_World;
}

void AuDocCityRevolt::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_City;
	out << m_Galaxy;
	out << m_Sector;
	out << m_System;
	out << m_World;
}

void AuDocCityRevolt::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_City;
	in >> m_Galaxy;
	in >> m_Sector;
	in >> m_System;
	in >> m_World;
}

int AuDocCityRevolt::Update(const AuDocument& doc)
{
	try
	{
		const AuDocCityRevolt& c = dynamic_cast<const AuDocCityRevolt&>(doc);
		if(m_City != c.m_City)
			return 0;
		// Absorb additional revolution records.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocCityRevolt::ListIcon() const
{
	return QString(":data/DocCityRevolt.png");
}

QString AuDocCityRevolt::Subject() const
{
	return QObject::tr("City Revolted");
}

QString AuDocCityRevolt::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: Revolution in %2; Citizens Joined %3</p>"
			"<p>Status: <img src=\"qrc%8\"> %9</p>"
			"<p>The citizens of %2 revolted against the benevolent reign of our empire. "
			"In an act of high treason, dissidents declared allegiance to %3 and hoisted their flag.</p>"
			"<p>The city is on %4 in the %5 system, %6, %7.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(m_City)
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Sector)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocCityRevolt::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocCityRevolt) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_City);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocClaimHijacked::AuDocClaimHijacked(const AuId& theirUnitId, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world)
{
	m_Galaxy = galaxy;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitId = theirUnitId;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocClaimHijacked::AuDocClaimHijacked()
{
	m_Galaxy = GALAXY_Count;
}

AuDocClaimHijacked::AuDocClaimHijacked(const AuDocClaimHijacked& c) : AuDocument(c)
{
	m_Galaxy = c.m_Galaxy;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitId = c.m_TheirUnitId;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocClaimHijacked::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_Galaxy;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitId;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocClaimHijacked::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_Galaxy;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitId;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocClaimHijacked::Update(const AuDocument& doc)
{
	try
	{
		const AuDocClaimHijacked& c = dynamic_cast<const AuDocClaimHijacked&>(doc);
		if(m_World != c.m_World)
			return 0;
		// Absorb additional hijacked claim records.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocClaimHijacked::ListIcon() const
{
	return QString(":data/DocWorldClaimed.png");
}

QString AuDocClaimHijacked::Subject() const
{
	return QObject::tr("%1 Claim Hijacked").arg(m_World);
}

QString AuDocClaimHijacked::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %3 Claim Hijacked</p>"
			"<p>Status: <img src=\"qrc%9\"> %10</p>"
			"<p>Our claim to the world of %3 was hijacked by %2. "
			"The world is in the %4 system, %5, %6. "
			"</p>"
			"<p>With the help of trusted witnesses and DNA evidence, "
			"the hijacker was positively identified as a %2 unit named %7 (%8).</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Sector)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_TheirUnitId.StringId())
		.arg(StateIcon())
		.arg(StateName());
}

quint64 AuDocClaimHijacked::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocClaimHijacked) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdDestroyed::AuDocBdDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
		const QString& system, const QString& world, const QString& city)
{
	m_BdName = name;
	m_City = city;
	m_Galaxy = galaxy;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocBdDestroyed::AuDocBdDestroyed()
{
	m_Galaxy = GALAXY_Count;
}

AuDocBdDestroyed::AuDocBdDestroyed(const AuDocBdDestroyed& c) : AuDocument(c)
{
	m_BdName = c.m_BdName;
	m_City = c.m_City;
	m_Galaxy = c.m_Galaxy;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocBdDestroyed::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_BdName;
	out << m_City;
	out << m_Galaxy;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocBdDestroyed::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_BdName;
	in >> m_City;
	in >> m_Galaxy;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocBdDestroyed::Update(const AuDocument& doc)
{
	Q_UNUSED(doc);	// Update
	return 0;
}

QString AuDocBdDestroyed::ListIcon() const
{
	return QString(":data/City.png");
}

QString AuDocBdDestroyed::Subject() const
{
	if(m_City.isEmpty())
		return QObject::tr("%1 Building Destroyed").arg(m_BdName);
	else
		return QObject::tr("%1 Building Destroyed in %2").arg(m_BdName).arg(m_City);
}

QString AuDocBdDestroyed::Html(const QString& empireName) const
{
	if(!m_City.isEmpty())
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %10 Destroyed in %11</p>"
				"<p>Status: <img src=\"qrc%2\"> %3</p>"
				"<p>Our %10 building in the city of %11 was destroyed by %4. "
				"%11 is on the world of %5, in the %6 system, %7, %8. "
				"</p>"
				"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
				"the attacker was positively identified as a %4 unit named %9.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(StateIcon())
			.arg(StateName())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_TheirUnitName)
			.arg(m_BdName)
			.arg(m_City);
	else
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %10 Destroyed</p>"
				"<p>Status: <img src=\"qrc%2\"> %3</p>"
				"<p>Our %10 building on the world of %5 was destroyed by %4. "
				"The world is in the %6 system, %7, %8. "
				"</p>"
				"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
				"the attacker was positively identified as a %4 unit named %9.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(StateIcon())
			.arg(StateName())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_TheirUnitName)
			.arg(m_BdName);
}

quint64 AuDocBdDestroyed::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocBdDestroyed) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_BdName);
	mem += StringMemoryUsage(m_City);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdCityCapitolDestroyed::AuDocBdCityCapitolDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdCityCapitolDestroyed::AuDocBdCityCapitolDestroyed()
{
}

AuDocBdCityCapitolDestroyed::AuDocBdCityCapitolDestroyed(const AuDocBdCityCapitolDestroyed& c) : AuDocBdDestroyed(c)
{
}

QString AuDocBdCityCapitolDestroyed::Subject() const
{
	return QObject::tr("%1 City Destroyed").arg(m_City);
}

QString AuDocBdCityCapitolDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %11 City Destroyed</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building in the city of %11 was destroyed by %4. "
			"The city is gone. "
			"%11 was on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Sector)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdSectorCapitolDestroyed::AuDocBdSectorCapitolDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdSectorCapitolDestroyed::AuDocBdSectorCapitolDestroyed()
{
}

AuDocBdSectorCapitolDestroyed::AuDocBdSectorCapitolDestroyed(const AuDocBdSectorCapitolDestroyed& c) : AuDocBdDestroyed(c)
{
}

QString AuDocBdSectorCapitolDestroyed::Subject() const
{
	return QObject::tr("%1 Sector Capital City Destroyed").arg(m_City);
}

QString AuDocBdSectorCapitolDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %11 Sector Capital City Destroyed</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building in the city of %11 was destroyed by %4. "
			"The sector capital city is gone. "
			"%11 was on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Sector)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdGalaxyCapitolDestroyed::AuDocBdGalaxyCapitolDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdGalaxyCapitolDestroyed::AuDocBdGalaxyCapitolDestroyed()
{
}

AuDocBdGalaxyCapitolDestroyed::AuDocBdGalaxyCapitolDestroyed(const AuDocBdGalaxyCapitolDestroyed& c) : AuDocBdDestroyed(c)
{
}

QString AuDocBdGalaxyCapitolDestroyed::Subject() const
{
	return QObject::tr("%1 Galaxy Capital City Destroyed").arg(m_City);
}

QString AuDocBdGalaxyCapitolDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %11 Galaxy Capital City Destroyed</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building in the city of %11 was destroyed by %4. "
			"The galaxy capital city is gone. "
			"%11 was on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Galaxy)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdEmpireCapitolDestroyed::AuDocBdEmpireCapitolDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdEmpireCapitolDestroyed::AuDocBdEmpireCapitolDestroyed()
{
}

AuDocBdEmpireCapitolDestroyed::AuDocBdEmpireCapitolDestroyed(const AuDocBdEmpireCapitolDestroyed& c) : AuDocBdDestroyed(c)
{
}

QString AuDocBdEmpireCapitolDestroyed::Subject() const
{
	return QObject::tr("%1 Empire Capital City Destroyed").arg(m_City);
}

QString AuDocBdEmpireCapitolDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %11 Empire Capital City Destroyed</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building in the city of %11 was destroyed by %4. "
			"The capital city of our empire is gone. "
			"%11 was on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Galaxy)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdMilitaryDestroyed::AuDocBdMilitaryDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdMilitaryDestroyed::AuDocBdMilitaryDestroyed()
{
}

AuDocBdMilitaryDestroyed::AuDocBdMilitaryDestroyed(const AuDocBdMilitaryDestroyed& c) : AuDocBdDestroyed(c)
{
}

QString AuDocBdMilitaryDestroyed::ListIcon() const
{
	return QString(":data/MilitaryBase.png");
}

QString AuDocBdMilitaryDestroyed::Subject() const
{
	return QObject::tr("%1 Destroyed in %2").arg(m_BdName).arg(m_City);
}

QString AuDocBdMilitaryDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %10 Destroyed in %11</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building at %11 was destroyed by %4. "
			"%11 is on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Galaxy)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdMilitaryHQDestroyed::AuDocBdMilitaryHQDestroyed(const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdMilitaryDestroyed(name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdMilitaryHQDestroyed::AuDocBdMilitaryHQDestroyed()
{
}

AuDocBdMilitaryHQDestroyed::AuDocBdMilitaryHQDestroyed(const AuDocBdMilitaryHQDestroyed& c) : AuDocBdMilitaryDestroyed(c)
{
}

QString AuDocBdMilitaryHQDestroyed::Subject() const
{
	return QObject::tr("%1 Military Base Destroyed").arg(m_City);
}

QString AuDocBdMilitaryHQDestroyed::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %11 Military Base Destroyed</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building at %11 was destroyed by %4. "
			"The military base was completely destroyed. "
			"%11 was on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Galaxy)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdDamaged::AuDocBdDamaged(const AuId& id, const QString& name, const QString& theirUnitName, GALAXY galaxy,
		const QString& sector, const QString& system, const QString& world, const QString& city)
{
	m_BdId = id;
	m_BdName = name;
	m_City = city;
	m_Galaxy = galaxy;
	m_Sector = sector;
	m_System = system;
	m_TheirUnitName = theirUnitName;
	m_World = world;
}

AuDocBdDamaged::AuDocBdDamaged()
{
	m_Galaxy = GALAXY_Count;
}

AuDocBdDamaged::AuDocBdDamaged(const AuDocBdDamaged& c) : AuDocument(c)
{
	m_BdId = c.m_BdId;
	m_BdName = c.m_BdName;
	m_City = c.m_City;
	m_Galaxy = c.m_Galaxy;
	m_Sector = c.m_Sector;
	m_System = c.m_System;
	m_TheirUnitName = c.m_TheirUnitName;
	m_World = c.m_World;
}

void AuDocBdDamaged::operator>>(QDataStream& out) const
{
	AuDocument::operator>>(out);
	out << m_BdId;
	out << m_BdName;
	out << m_City;
	out << m_Galaxy;
	out << m_Sector;
	out << m_System;
	out << m_TheirUnitName;
	out << m_World;
}

void AuDocBdDamaged::operator<<(QDataStream& in)
{
	AuDocument::operator<<(in);
	in >> m_BdId;
	in >> m_BdName;
	in >> m_City;
	in >> m_Galaxy;
	in >> m_Sector;
	in >> m_System;
	in >> m_TheirUnitName;
	in >> m_World;
}

int AuDocBdDamaged::Update(const AuDocument& doc)
{
	try
	{
		const AuDocBdDamaged& c = dynamic_cast<const AuDocBdDamaged&>(doc);
		if(m_BdId != c.m_BdId)
			return 0;
		// Absorb additional damage records.
		return 1;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed so it's not this kind of document.
		return 0;
	}
}

QString AuDocBdDamaged::ListIcon() const
{
	return QString(":data/City.png");
}

QString AuDocBdDamaged::Subject() const
{
	if(m_City.isEmpty())
		return QObject::tr("%1 Building Damaged").arg(m_BdName);
	else
		return QObject::tr("%1 Building Damaged in %2").arg(m_BdName).arg(m_City);
}

QString AuDocBdDamaged::Html(const QString& empireName) const
{
	if(!m_City.isEmpty())
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %10 Damaged in %11</p>"
				"<p>Status: <img src=\"qrc%2\"> %3</p>"
				"<p>Our %10 building in the city of %11 was damaged by %4. "
				"%11 is on the world of %5, in the %6 system, %7, %8. "
				"</p>"
				"<p>With the help of trusted witnesses and evidence gathered from the debris, "
				"the attacker was positively identified as a %4 unit named %9.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(StateIcon())
			.arg(StateName())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_TheirUnitName)
			.arg(m_BdName)
			.arg(m_City);
	else
		return QObject::tr(
				"<p>Star Date %1</p>"
				"<p>Subject: %10 Damaged</p>"
				"<p>Status: <img src=\"qrc%2\"> %3</p>"
				"<p>Our %10 building on the world of %5 was damaged by %4. "
				"The world is in the %6 system, %7, %8. "
				"</p>"
				"<p>With the help of trusted witnesses and evidence gathered from the debris, "
				"the attacker was positively identified as a %4 unit named %9.</p>"
				)
			.arg(CreateTime().UTC().toString())
			.arg(StateIcon())
			.arg(StateName())
			.arg(empireName)
			.arg(m_World)
			.arg(m_System)
			.arg(m_Sector)
			.arg(GalaxyName((GALAXY)m_Galaxy))
			.arg(m_TheirUnitName)
			.arg(m_BdName);
}

quint64 AuDocBdDamaged::MemoryUsage() const
{
	quint64 mem = AuDocument::MemoryUsage();
	mem += sizeof(AuDocBdDamaged) - sizeof(AuDocument);
	mem += StringMemoryUsage(m_BdName);
	mem += StringMemoryUsage(m_City);
	mem += StringMemoryUsage(m_Sector);
	mem += StringMemoryUsage(m_System);
	mem += StringMemoryUsage(m_TheirUnitName);
	mem += StringMemoryUsage(m_World);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDocBdMilitaryDamaged::AuDocBdMilitaryDamaged(const AuId& id, const QString& name, const QString& theirUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world, const QString& city) :
	AuDocBdDamaged(id, name, theirUnitName, galaxy, sector, system, world, city)
{
}

AuDocBdMilitaryDamaged::AuDocBdMilitaryDamaged()
{
}

AuDocBdMilitaryDamaged::AuDocBdMilitaryDamaged(const AuDocBdMilitaryDamaged& c) : AuDocBdDamaged(c)
{
}

QString AuDocBdMilitaryDamaged::ListIcon() const
{
	return QString(":data/MilitaryBase.png");
}

QString AuDocBdMilitaryDamaged::Subject() const
{
	return QObject::tr("%1 Damaged in %2").arg(m_BdName).arg(m_City);
}

QString AuDocBdMilitaryDamaged::Html(const QString& empireName) const
{
	return QObject::tr(
			"<p>Star Date %1</p>"
			"<p>Subject: %10 Damaged in %11</p>"
			"<p>Status: <img src=\"qrc%2\"> %3</p>"
			"<p>Our %10 building at %11 was damaged by %4. "
			"%11 is on the world of %5, in the %6 system, %7, %8. "
			"</p>"
			"<p>With the help of trusted witnesses and evidence gathered from the rubble, "
			"the attacker was positively identified as a %4 unit named %9.</p>"
			)
		.arg(CreateTime().UTC().toString())
		.arg(StateIcon())
		.arg(StateName())
		.arg(empireName)
		.arg(m_World)
		.arg(m_System)
		.arg(m_Galaxy)
		.arg(GalaxyName((GALAXY)m_Galaxy))
		.arg(m_TheirUnitName)
		.arg(m_BdName)
		.arg(m_City);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
