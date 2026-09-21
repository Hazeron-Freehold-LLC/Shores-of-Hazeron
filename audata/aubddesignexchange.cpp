//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesignexchange.cpp,v 1.28 2022/02/19 18:19:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubddesignexchange.h"
//
#include "audesign.h"
#include "aubdreport.h"
#include "auexchangerating.h"
#include "dgdata.h"
#include <QSqlError>
#include <QSqlQuery>

static int s_AuBdDesignExchangeEntryMetaType = qRegisterMetaType<AuBdDesignExchangeEntry>();

AuBdDesignExchangeEntry::AuBdDesignExchangeEntry(const AuBdReport& report, quint8 state, quint32 price, bool canCopy, bool canView,
		bool useOnce, quint32 publisherEmpire, POLITICAL_STANCE minStance)
{
	m_CanCopy = canCopy;
	m_CanView = canView;
	m_Empire = publisherEmpire;
	m_MinStance = minStance;
	m_Price = price;
	m_Raters = 0;
	m_Rating = 0;
	m_Report = new AuBdReport(report);
	m_State = state;
	m_UseOnce = useOnce;
	m_When.Set();
}

AuBdDesignExchangeEntry::AuBdDesignExchangeEntry()
{
	m_CanCopy = false;
	m_CanView = false;
	m_Empire = 0;
	m_MinStance = POL_Empire;
	m_Price = 0;
	m_Raters = 0;
	m_Rating = 0;
	m_Report = new AuBdReport;
	m_State = 0;
	m_UseOnce = false;
}

AuBdDesignExchangeEntry::~AuBdDesignExchangeEntry()
{
	delete m_Report;
}

AuBdDesignExchangeEntry::AuBdDesignExchangeEntry(const AuBdDesignExchangeEntry& c)
{
	m_CanCopy = c.m_CanCopy;
	m_CanView = c.m_CanView;
	m_Empire = c.m_Empire;
	m_MinStance = c.m_MinStance;
	m_Price = c.m_Price;
	m_RatedLast = c.m_RatedLast;
	m_Raters = c.m_Raters;
	m_Rating = c.m_Rating;
	m_Report = new AuBdReport(*c.m_Report);
	m_State = c.m_State;
	m_UseOnce = c.m_UseOnce;
	m_When = c.m_When;
}

AuBdDesignExchangeEntry& AuBdDesignExchangeEntry::operator=(const AuBdDesignExchangeEntry& c)
{
	m_CanCopy = c.m_CanCopy;
	m_CanView = c.m_CanView;
	m_Empire = c.m_Empire;
	m_MinStance = c.m_MinStance;
	m_Price = c.m_Price;
	m_RatedLast = c.m_RatedLast;
	m_Raters = c.m_Raters;
	m_Rating = c.m_Rating;
	*m_Report = *c.m_Report;
	m_State = c.m_State;
	m_UseOnce = c.m_UseOnce;
	m_When = c.m_When;
	return *this;
}

bool AuBdDesignExchangeEntry::operator==(const AuBdDesignExchangeEntry& c) const
{
	if(m_Report->DesignId() != c.m_Report->DesignId())
		return false;
	if(m_Empire != c.m_Empire)
		return false;
	if(m_Price != c.m_Price)
		return false;
	if(m_State != c.m_State)
		return false;
	if(m_MinStance != c.m_MinStance)
		return false;
	if(m_CanCopy != c.m_CanCopy)
		return false;
	if(m_CanView != c.m_CanView)
		return false;
	if(m_UseOnce != c.m_UseOnce)
		return false;
	if(m_When != c.m_When)
		return false;
	// compare ratings?
	return true;
}

bool AuBdDesignExchangeEntry::AccessibleTo(const AuId& avatarId, quint32 empireId) const
{
	// No empire is available to everybody.
	if(!m_Empire)
		return true;
	// Architect can always access their own designs.
	if(avatarId == DesignArchitectId())
		return true;

	// Check political stance restriction.
	switch(MinStanceToBuyer())
	{
	case POL_Friend:	// Friend or better. No enemy or neutral.
		{
			POLITICAL_STANCE stance = PoliticalStance(m_Empire, empireId);
			if(stance == POL_Enemy || stance == POL_Neutral)
				return false;
		}
		break;

	case POL_Neutral:	// Neutral or better. No enemy.
		if(PoliticalStance(m_Empire, empireId) == POL_Enemy)
			return false;
		break;

	case POL_Enemy:		// Enemy or better. That means everybody.
		break;

	case POL_Empire:	// Only architect's empire.
		return m_Empire == empireId;

	case POL_Vassal:	// This is not valid.
		break;
	}
	return true;
}

bool AuBdDesignExchangeEntry::AccessibleToAnyone() const
{
	// No empire is available to everybody.
	if(!m_Empire)
		return true;

	// Check political stance restriction.
	switch(MinStanceToBuyer())
	{
	case POL_Empire:	// Only architect's empire.
	case POL_Friend:	// Friend or better. No enemy or neutral.
		return false;

	case POL_Neutral:	// Neutral is close enough to "anyone" for the purpose this function is used.
	case POL_Enemy:		// Enemy or better. That means everybody.
		break;

	case POL_Vassal:	// This is not valid.
		break;
	}
	return true;
}

void AuBdDesignExchangeEntry::operator>>(QDataStream& out) const
{
	quint8 hasWhat = m_State;
	if(m_CanCopy)
		hasWhat |= 0x80;
	if(m_CanView)
		hasWhat |= 0x40;
	if(m_UseOnce)
		hasWhat |= 0x20;
	if(m_When != AuTime())
		hasWhat |= 0x10;
	hasWhat |= 0x08;		// Bit says we have added another hasWhat byte.

	out << hasWhat;
	out << m_Empire;
	out << m_MinStance;
	out << m_Price;
	out << *m_Report;
	if(hasWhat & 0x10)
		out << m_When;

	quint8 hasWhat2 = 0;
	if(m_Raters)
		hasWhat2 |= 0x01;

	out << hasWhat2;
	if(hasWhat2 & 0x01)
	{
		out << m_Raters;
		out << m_Rating;
		out << m_RatedLast;
	}
}

void AuBdDesignExchangeEntry::operator<<(QDataStream& in)
{
	quint8 hasWhat;

	in >> hasWhat;

	m_State = hasWhat & 0x07;	// bottom 3 bits for state.
	m_CanCopy = hasWhat & 0x80;
	m_CanView = hasWhat & 0x40;
	m_UseOnce = hasWhat & 0x20;

	in >> m_Empire;
	in >> m_MinStance;
	in >> m_Price;
	in >> *m_Report;
	if(hasWhat & 0x10)
		in >> m_When;

	quint8 hasWhat2;
	if(hasWhat & 0x08)
		in >> hasWhat2;
	else
		hasWhat2 = 0;

	if(hasWhat2 & 0x01)
	{
		in >> m_Raters;
		in >> m_Rating;
		in >> m_RatedLast;
	}
}

void AuBdDesignExchangeEntry::SqlInsert(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO n_BdExchange (id,state,canCopy,canView,empire,minStance,price,useOnce,report,whenPub) "
			"VALUES (?,?,?,?,?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(DesignId());
	qa.addBindValue(m_State);
	qa.addBindValue(m_CanCopy);
	qa.addBindValue(m_CanView);
	qa.addBindValue(m_Empire);
	qa.addBindValue(m_MinStance);
	qa.addBindValue((qint32)m_Price);
	qa.addBindValue(m_UseOnce);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << *m_Report;
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_When.UTC());

	if(!qa.exec())
		qWarning() << "error: AuBdDesignExchangeEntry::SqlInsert" << sq;
}

void AuBdDesignExchangeEntry::SqlUpdate(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq = QString("UPDATE n_BdExchange SET state=?, canCopy=?, canView=?, empire=?, minStance=?, price=?, useOnce=?, "
			"report=?, whenPub=? "
			"WHERE id=%1;").arg(DesignId());
	qa.prepare(sq);
	qa.addBindValue(m_State);
	qa.addBindValue(m_CanCopy);
	qa.addBindValue(m_CanView);
	qa.addBindValue(m_Empire);
	qa.addBindValue(m_MinStance);
	qa.addBindValue((qint32)m_Price);
	qa.addBindValue(m_UseOnce);
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_0);
		out << *m_Report;
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_When.UTC());

	if(!qa.exec())
	{
		qWarning() << "error: AuBdDesignExchangeEntry::SqlUpdate" << sq << "trying SqlInsert";
		SqlInsert(db);
	}
}

void AuBdDesignExchangeEntry::SqlDelete(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq = QString("DELETE FROM n_BdExchange WHERE id=%1;").arg(DesignId());
	qa.prepare(sq);
	if(!qa.exec())
		qWarning() << "error: AuBdDesignExchangeEntry::SqlDelete" << sq << QSqlError(sq);
}

bool AuBdDesignExchangeEntry::CanBeConstructed() const
{
	return m_Report->CanBeConstructed();
}

AuId AuBdDesignExchangeEntry::DesignArchitectId() const
{
	return m_Report->ArchitectId();
}

const QString& AuBdDesignExchangeEntry::DesignArchitectName() const
{
	return m_Report->ArchitectName();
}

quint32 AuBdDesignExchangeEntry::DesignId() const
{
	return m_Report->DesignId();
}

const QString& AuBdDesignExchangeEntry::DesignName() const
{
	return m_Report->DesignName();
}

INDUSTRYID AuBdDesignExchangeEntry::IndustryId() const
{
	return m_Report->IndustryId();
}

BUILDING_STRUCTURE AuBdDesignExchangeEntry::DesignStructure() const
{
	return m_Report->Structure();
}

quint8 AuBdDesignExchangeEntry::DesignQuality() const
{
	return m_Report->Quality();
}

// Cubic meters.
quint32 AuBdDesignExchangeEntry::HullVolume() const
{
	return m_Report->HullVolume();
}

QString AuBdDesignExchangeEntry::ReportText() const
{
	return m_Report->ReportAnalysis(m_State);
}

QString AuBdDesignExchangeEntry::ReportToolTip() const
{
	return m_Report->ReportToolTip();
}

quint32 AuBdDesignExchangeEntry::FaceCount() const
{
	return m_Report->FaceCount();
}

quint32 AuBdDesignExchangeEntry::TriangleCount() const
{
	return m_Report->TriangleCount();
}

int AuBdDesignExchangeEntry::IndustryCanUse(const DbIndustry& ind) const
{
	return m_Report->IndustryCanUse(ind);
}

void AuBdDesignExchangeEntry::IndustryInfo(const DbIndustry& dbi, quint32& homesLarge, quint32& homesMedium, quint32& homesSmall,
		quint32& manufacturingJobs, quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const
{
	return m_Report->IndustryInfo(dbi, homesLarge, homesMedium, homesSmall, manufacturingJobs, serviceJobs, stores, lounges,
			proJobs);
}

const DeConstructionProcess& AuBdDesignExchangeEntry::ConstructionProcess() const
{
	return m_Report->ConstructionProcess();
}

COMMODITYID AuBdDesignExchangeEntry::ConstructionMaterial() const
{
	return m_Report->HullMaterial();
}

const AuPoint& AuBdDesignExchangeEntry::HullMax() const
{
	return m_Report->HullMax();
}

const AuPoint& AuBdDesignExchangeEntry::HullMin() const
{
	return m_Report->HullMin();
}

const AuPoint& AuBdDesignExchangeEntry::SiteMax() const
{
	return m_Report->SiteMax();
}

const AuPoint& AuBdDesignExchangeEntry::SiteMin() const
{
	return m_Report->SiteMin();
}

quint64 AuBdDesignExchangeEntry::MemoryUsage() const
{
	quint64 mem = 0;
	mem += m_Report->MemoryUsage();
	mem += sizeof(m_State);

	mem += sizeof(m_CanCopy);
	mem += sizeof(m_CanView);
	mem += sizeof(m_Empire);
	mem += sizeof(m_MinStance);
	mem += sizeof(m_Price);
	mem += sizeof(m_UseOnce);
	return mem;
}

void AuBdDesignExchangeEntry::SetReport(const AuBdReport& report)
{
	*m_Report = report;
}

// Prevent designs by the same architect with the same name, if they are in the same state.
bool AuBdDesignExchangeEntry::IsDup(const AuBdDesignExchangeEntry& c) const
{
	if(DesignId() == c.DesignId())
		return false;					// Matching design id is not considered a duplicate so it can be replaced by republishing.
	if(DesignArchitectId() != c.DesignArchitectId())
		return false;
	if(DesignState() != c.DesignState())
		return false;
	if(DesignName() != c.DesignName())
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdDesignExchange::AuBdDesignExchange()
{
}

AuBdDesignExchange::~AuBdDesignExchange()
{
}

AuBdDesignExchange::AuBdDesignExchange(const AuBdDesignExchange& c) : QHash<quint32, AuBdDesignExchangeEntry>(c)
{
}

const AuBdDesignExchange& AuBdDesignExchange::operator=(const AuBdDesignExchange& c)
{
	QHash<quint32, AuBdDesignExchangeEntry>::operator=(c);
	return *this;
}

int AuBdDesignExchange::Add(const AuBdDesignExchangeEntry& libraryEntry)
{
	// Look for an existing exchange entry by the same publisher with the same name.
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).IsDup(libraryEntry))
			return 3;

	iterator li = find(libraryEntry.DesignId());
	if(li == end())
	{
		insert(libraryEntry.DesignId(), libraryEntry);
		return 1;
	}
	else
	{
		*li = libraryEntry;
		return 2;
	}
}

bool AuBdDesignExchange::Get(quint32 designId, AuBdDesignExchangeEntry& libraryEntry) const
{
	const_iterator li = find(designId);
	if(li == cend())
		return false;

	libraryEntry = *li;
	return true;
}

bool AuBdDesignExchange::Remove(quint32 designId)
{
	return remove(designId);
}

void AuBdDesignExchange::SqlSelectAll(QSqlDatabase& db)
{
	clear();

	QString sq("SELECT state,canCopy,canView,empire,minStance,price,useOnce,report,whenPub "
			"FROM n_BdExchange;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		AuBdDesignExchangeEntry ob;

		int qv=0;
		ob.m_State = qry.value(qv++).toUInt();
		ob.m_CanCopy = qry.value(qv++).toBool();
		ob.m_CanView = qry.value(qv++).toBool();
		ob.m_Empire = qry.value(qv++).toUInt();
		ob.m_MinStance = qry.value(qv++).toUInt();
		ob.m_Price = qry.value(qv++).toUInt();
		ob.m_UseOnce = qry.value(qv++).toBool();
		{
			QByteArray ba = qry.value(qv++).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Report;
		}
		QDateTime whenPub = qry.value(qv++).toDateTime();
		whenPub.setTimeSpec(Qt::UTC);
		ob.m_When = whenPub;

		if(ob.DesignId())
			Add(ob);
	}
}

void AuBdDesignExchange::SqlSelectEntry(QSqlDatabase& db, quint32 designId)
{
	if(!designId)
		return;

	QString sq = QString("SELECT state,canCopy,canView,empire,minStance,price,useOnce,report,whenPub "
			"FROM n_BdExchange WHERE id=%1;").arg(designId);
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	if(qry.next())
	{
		AuBdDesignExchangeEntry ob;

		int qv=0;
		ob.m_State = qry.value(qv++).toUInt();
		ob.m_CanCopy = qry.value(qv++).toBool();
		ob.m_CanView = qry.value(qv++).toBool();
		ob.m_Empire = qry.value(qv++).toUInt();
		ob.m_MinStance = qry.value(qv++).toUInt();
		ob.m_Price = qry.value(qv++).toUInt();
		ob.m_UseOnce = qry.value(qv++).toBool();
		{
			QByteArray ba = qry.value(qv++).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_5_0);
			in >> *ob.m_Report;
		}
		QDateTime whenPub = qry.value(qv++).toDateTime();
		whenPub.setTimeSpec(Qt::UTC);
		ob.m_When = whenPub;

		if(ob.DesignId())
			Add(ob);
	}
}

void AuBdDesignExchange::List(AuBdDesignExchange& list, const AuId& toWho, quint32 toEmpire) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).AccessibleTo(toWho, toEmpire))
			list.insert(ei.key(), *ei);
}

void AuBdDesignExchange::ListPublic(AuBdDesignExchange& list) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).AccessibleToAnyone())
			list.insert(ei.key(), *ei);
}

QString AuBdDesignExchange::HtmlListContents() const
{
	// Alphabetize
	QMap<QString, quint32> designs;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		designs.insert((*ei).DesignName().toUpper(), ei.key());

	// Format
	QString cont = QObject::tr("<hr><img height=\"16\" src=\"qrc:data/Folder.png\"> Building Blueprints <b>%1</b>")
		.arg(size());
	for(QMap<QString, quint32>::const_iterator ai = designs.cbegin(); ai != designs.cend(); ai++)
	{
		const_iterator ei = find(*ai);
		if((*ei).DesignState() == AuDesign::Final)
			cont += QString("<br>%5 <img height=\"16\" src=\"qrc:data/c_BdDesignDrawing.png\"> %1 %2 Q%4 by %3")
				.arg(ei.key())
				.arg((*ei).DesignName())
				.arg((*ei).DesignArchitectName())
				.arg((*ei).DesignQuality())
				.arg(g_Au.MiddleDotSymbol());
		else
			cont += QString("<br>%4 <img height=\"16\" src=\"qrc:data/c_BdDesignDrawing.png\"> %1 %2 Assembly by %3")
				.arg(ei.key())
				.arg((*ei).DesignName())
				.arg((*ei).DesignArchitectName())
				.arg(g_Au.MiddleDotSymbol());
	}
	return cont;
}

quint64 AuBdDesignExchange::MemoryUsage() const
{
	quint64 mem = sizeof(AuBdDesignExchange);
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		mem += (*ei).MemoryUsage();
	return mem;
}

void AuBdDesignExchange::ListAndRate(AuBdDesignExchange& list, const AuId& toWho, quint32 toEmpire,
		const AuExchangeBlueprintRatings& ratings) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).AccessibleTo(toWho, toEmpire))
		{
			AuBdDesignExchange::iterator di = list.insert(ei.key(), *ei);
			quint32 raters;
			float rating;
			AuTime lastRated;
			if(ratings.Rating(ei.key(), raters, rating, lastRated))
				(*di).SetRating(raters, rating, lastRated);
		}
}

QString AuBdDesignExchange::DesignName(quint32 designId) const
{
	const_iterator li = find(designId);
	if(li == cend())
		return "";
	return (*li).DesignName();
}

void AuBdDesignExchange::Merge(const AuBdDesignExchange& mex)
{
	// Add design exchange entries that are not already here.
	for(const_iterator it = mex.cbegin(); it != mex.cend(); it++)
	{
		if(contains(it.key()))
			continue;	// I already have that design exchange entry.
		insert(it.key(), *it);
	}
}

