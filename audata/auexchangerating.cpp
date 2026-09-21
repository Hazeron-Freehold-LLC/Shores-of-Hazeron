//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auexchangerating.cpp,v 1.11 2022/02/19 18:20:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auexchangerating.h"
//
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuExchangeBlueprintRatingMetaType = qRegisterMetaType<AuExchangeBlueprintRating>();
static int s_AuExchangeRatingRecMetaType = qRegisterMetaType<AuExchangeRatingRec>();

void AuExchangeRatingRec::operator>>(QDataStream& out) const
{
	out << m_Rating;
	out << m_Comment;
	out << m_DesignId;
	out << m_Rater;
	out << m_RaterName;
	out << m_WhenRated;
}

void AuExchangeRatingRec::operator<<(QDataStream& in)
{
	in >> m_Rating;
	in >> m_Comment;
	in >> m_DesignId;
	in >> m_Rater;
	in >> m_RaterName;
	in >> m_WhenRated;
}

bool AuExchangeRatingRec::IsValid() const
{
	if(!m_DesignId)
		return false;
	if(m_Rater.IsNull())
		return false;
	if(m_RaterName.isEmpty())
		return false;
	if(m_Rating < 1 || m_Rating > 5)
		return false;
	if(m_WhenRated == AuTime())
		return false;
	return true;
}

void AuExchangeRatingRec::SqlStoreBd(QSqlDatabase& db) const
{
	// Delete current rating by the same person for the design.
	QString sq = QString("DELETE FROM n_BdRating WHERE rid=%1 AND designid=%2;")
		.arg(m_Rater.GetID())
		.arg(m_DesignId);
	QSqlQuery qdel(db);
	qdel.prepare(sq);
	if(!qdel.exec())
		qWarning() << "warning: AuExchangeRatingRec::SqlStoreBd - DELETE FAILED" << sq << qdel.lastError();

	// Insert the new rating for the design by the rater.
	sq = QString("INSERT INTO n_BdRating (rid,rname,designId,rating,comment,whenRated) VALUES (?,?,?,?,?,?);");
	QSqlQuery qa(db);
	qa.prepare(sq);
	qa.addBindValue(m_Rater.GetID());
	qa.addBindValue(m_RaterName);
	qa.addBindValue(m_DesignId);
	qa.addBindValue(m_Rating);
	qa.addBindValue(m_Comment);
	qa.addBindValue(m_WhenRated.UTC());
	if(!qa.exec())
		qWarning() << "warning: AuExchangeRatingRec::SqlStoreBd - INSERT FAILED" << sq << qa.lastError();
}

void AuExchangeRatingRec::SqlStoreSc(QSqlDatabase& db) const
{
	// Delete current rating by the same person for the design.
	QString sq = QString("DELETE FROM n_ScRating WHERE rid=%1 AND designid=%2;")
		.arg(m_Rater.GetID())
		.arg(m_DesignId);
	QSqlQuery qdel(db);
	qdel.prepare(sq);
	if(!qdel.exec())
		qWarning() << "warning: AuExchangeRatingRec::SqlStoreSc - DELETE FAILED" << sq << qdel.lastError();

	// Insert the new rating for the design by the rater.
	sq = QString("INSERT INTO n_ScRating (rid,rname,designId,rating,comment,whenRated) VALUES (?,?,?,?,?,?);");
	QSqlQuery qa(db);
	qa.prepare(sq);
	qa.addBindValue(m_Rater.GetID());
	qa.addBindValue(m_RaterName);
	qa.addBindValue(m_DesignId);
	qa.addBindValue(m_Rating);
	qa.addBindValue(m_Comment);
	qa.addBindValue(m_WhenRated.UTC());
	if(!qa.exec())
		qWarning() << "warning: AuExchangeRatingRec::SqlStoreSc - INSERT FAILED" << sq << qa.lastError();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuExchangeRating::AuExchangeRating(const QString& raterName, quint8 rating, const QString& comment, const AuTime& whenRated)
{
	m_Comment = comment;
	m_RaterName = raterName;
	m_Rating = rating;
	m_WhenRated = whenRated;
}

AuExchangeRating::AuExchangeRating(const AuExchangeRatingRec& rating)
{
	m_Comment = rating.Comment();
	m_RaterName = rating.RaterName();
	m_Rating = rating.Rating();
	m_WhenRated = rating.WhenRated();
}

AuExchangeRating::AuExchangeRating()
{
	m_Rating = 0;
}

AuExchangeRating::~AuExchangeRating()
{
}

AuExchangeRating::AuExchangeRating(const AuExchangeRating& c)
{
	m_Comment = c.m_Comment;
	m_RaterName = c.m_RaterName;
	m_Rating = c.m_Rating;
	m_WhenRated = c.m_WhenRated;
}

const AuExchangeRating& AuExchangeRating::operator=(const AuExchangeRating& c)
{
	m_Comment = c.m_Comment;
	m_RaterName = c.m_RaterName;
	m_Rating = c.m_Rating;
	m_WhenRated = c.m_WhenRated;
	return *this;
}

void AuExchangeRating::operator>>(QDataStream& out) const
{
	quint8 hasWhat = m_Rating;	// bottom 3 bits
	if(!m_Comment.isEmpty())
		hasWhat |= 0x80;

	out << hasWhat;
	out << m_RaterName;
	out << m_WhenRated;
	if(hasWhat & 0x80)
		out << m_Comment;
}

void AuExchangeRating::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	m_Rating = hasWhat & 0x07;	// bottom 3 bits
	in >> m_RaterName;
	in >> m_WhenRated;
	if(hasWhat & 0x80)
		in >> m_Comment;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuExchangeBlueprintRating::AuExchangeBlueprintRating()
{
}

AuExchangeBlueprintRating::~AuExchangeBlueprintRating()
{
}

AuExchangeBlueprintRating::AuExchangeBlueprintRating(const AuExchangeBlueprintRating& c) : m_Reviews(c.m_Reviews)
{
}

const AuExchangeBlueprintRating& AuExchangeBlueprintRating::operator=(const AuExchangeBlueprintRating& c)
{
	m_Reviews = c.m_Reviews;
	return *this;
}

void AuExchangeBlueprintRating::AddReview(const AuExchangeRatingRec& rating)
{
	// Erase current rating by the rater id.
	m_Reviews.remove(rating.Rater());
	m_Reviews.insert(rating.Rater(), AuExchangeRating(rating));
}

float AuExchangeBlueprintRating::RatingAverage() const
{
	if(m_Reviews.empty())
		return 0;
	float totalRating = 0;
	for(QHash<AuId, AuExchangeRating>::const_iterator ri = m_Reviews.cbegin(); ri != m_Reviews.cend(); ri++)
		totalRating += (*ri).Rating();
	if(m_Reviews.size() > 1)
		totalRating /= m_Reviews.size();
	return totalRating;
}

//static
QString AuExchangeBlueprintRating::RatingAverageIconName(float rating)
{
	if(rating < 1 || rating > 5)
		return QString();
	return QString(":data/Stars%1.png").arg(round(rating));
}

//static
QString AuExchangeBlueprintRating::RatingAverageString(float rating)
{
	if(rating <= 0)
		return QString();
	QString rf = QString("%1").arg(rating, 0,'f',1);
	QString ri = QString("%1").arg(floorf(rating), 0,'f',1);
	if(rf == ri)
		return QString("%1").arg(rating);	// Rating formatted the same as its integer so we don't need the decimal part.
	return rf;
}

void AuExchangeBlueprintRating::operator>>(QDataStream& out) const
{
	out << (quint32)m_Reviews.size();
	for(QHash<AuId, AuExchangeRating>::const_iterator ri = m_Reviews.cbegin(); ri != m_Reviews.cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuExchangeBlueprintRating::operator<<(QDataStream& in)
{
	m_Reviews.clear();
	quint32 c;
	in >> c;
	while(c--)
	{
		AuId rater;
		AuExchangeRating rating;
		in >> rater;
		in >> rating;
		m_Reviews.insert(rater, rating);
	}
}

QString AuExchangeBlueprintRating::WebReviews() const
{
	if(m_Reviews.empty())
		return QString();
	QString reviews;
	reviews += "<div style=\"margin-left: 10px; margin-top: 10px; background-color: #0a1533\">";
	if(Ratings() > 1)
		reviews += QString("<img src=\"qrc%1\" width=\"32\" height=\"32\" style=\"margin: 4 4;\"> %2 Stars Average of %3 Reviews<br>")
			.arg(RatingAverageIconName())
			.arg(RatingAverageString())
			.arg(Ratings());
	else
		reviews += QString("<img src=\"qrc%1\" width=\"32\" height=\"32\" style=\"margin: 4 4;\"> %2 Stars from One Review<br>")
			.arg(RatingAverageIconName())
			.arg(RatingAverageString());

	reviews += "<br><table border=\"0\" style=\"margin-left: 8\"><tbody>";
	// Sort by date ascending.
	QMultiMap<AuTime, QHash<AuId, AuExchangeRating>::const_iterator> sorted;
	for(QHash<AuId, AuExchangeRating>::const_iterator ri = m_Reviews.cbegin(); ri != m_Reviews.cend(); ri++)
		sorted.insert((*ri).WhenRated(), ri);

	for(QMultiMap<AuTime, QHash<AuId, AuExchangeRating>::const_iterator>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
	{
		QHash<AuId, AuExchangeRating>::const_iterator ri = *si;
		// Gold separator bar row.
		reviews += "<tr style=\"vertical-align: top;\">";
		reviews += "<td colspan=\"2\">";
		reviews += "<img src=\"qrc:data/RatingLine.png\"><br>";
		reviews += "</td>";
		reviews += "</tr>";

		// Rating and review row.
		reviews += "<tr style=\"vertical-align: top;\">";
		reviews += "<td width=\"16\">";
		reviews += QString("<img src=\"qrc%1\" width=\"16\" height=\"16\">")
			.arg(RatingAverageIconName((*ri).Rating()));
		reviews += "</td>";

		reviews += "<td>";
		reviews += QString("<small><small>%1</small></small><br>"
				"<big><b>%2</b></big> <small><i>%3</i></small>")
			.arg((*ri).WhenRated().UTC().toString())
			.arg((*ri).RaterName())
			.arg(ri.key().StringId());
		if(!(*ri).Comment().isEmpty())
			reviews += QString("<div style=\"margin-left: 10px;\">%1</div>")
				.arg((*ri).Comment());
		reviews += "<br>";
		reviews += "</td>";
		reviews += "</tr>";
	}
	reviews += "</tbody></table>";
	reviews += "</div>";
	return reviews;
}

AuTime AuExchangeBlueprintRating::RatedLast() const
{
	AuTime lastRated;
	for(QHash<AuId, AuExchangeRating>::const_iterator ri = m_Reviews.cbegin(); ri != m_Reviews.cend(); ri++)
		if(ri == m_Reviews.cbegin() || lastRated < (*ri).WhenRated())
			lastRated = (*ri).WhenRated();
	return lastRated;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuExchangeBlueprintRatings::AuExchangeBlueprintRatings()
{
}

AuExchangeBlueprintRatings::~AuExchangeBlueprintRatings()
{
}

AuExchangeBlueprintRatings::AuExchangeBlueprintRatings(const AuExchangeBlueprintRatings& c) : m_Blueprints(c.m_Blueprints)
{
}

void AuExchangeBlueprintRatings::SqlSelectBd(QSqlDatabase& db, quint32 designId)
{
	// Erase current ratings for the design id.
	m_Blueprints.remove(designId);

	// Select ratings from the database for the design id.
	AuExchangeBlueprintRating blueprintRating;
	QString sq;
	sq = QString("SELECT comment, rid, rname, rating, whenRated "
			"FROM n_BdRating WHERE designid=%1;").arg(designId);
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	AuTime w;
	while(qry.next())
	{
		int qv=0;
		QString comment = qry.value(qv++).toString();
		AuId rater = AuId(qry.value(qv++).toUInt());
		QString raterName = qry.value(qv++).toString();
		unsigned int rating = qry.value(qv++).toUInt();
		QDateTime whenRated = qry.value(qv++).toDateTime();
		whenRated.setTimeSpec(Qt::UTC);
		w = whenRated;
		blueprintRating.AddReview(AuExchangeRatingRec(rater, raterName, designId, rating, comment, w));
	}
	if(blueprintRating.IsRated())
		m_Blueprints.insert(designId, blueprintRating);
}

void AuExchangeBlueprintRatings::SqlSelectSc(QSqlDatabase& db, quint32 designId)
{
	// Erase current ratings for the design id.
	m_Blueprints.remove(designId);

	// Select ratings from the database for the design id.
	AuExchangeBlueprintRating blueprintRating;
	QString sq;
	sq = QString("SELECT comment, rid, rname, rating, whenRated "
			"FROM n_ScRating WHERE designid=%1;").arg(designId);
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	AuTime w;
	while(qry.next())
	{
		int qv=0;
		QString comment = qry.value(qv++).toString();
		AuId rater = AuId(qry.value(qv++).toUInt());
		QString raterName = qry.value(qv++).toString();
		unsigned int rating = qry.value(qv++).toUInt();
		QDateTime whenRated = qry.value(qv++).toDateTime();
		whenRated.setTimeSpec(Qt::UTC);
		w = whenRated;
		blueprintRating.AddReview(AuExchangeRatingRec(rater, raterName, designId, rating, comment, w));
	}
	if(blueprintRating.IsRated())
		m_Blueprints.insert(designId, blueprintRating);
}

void AuExchangeBlueprintRatings::AddReview(const AuExchangeRatingRec& rating)
{
	if(!rating.IsValid())
		return;
	QHash<quint32, AuExchangeBlueprintRating>::iterator bi = m_Blueprints.find(rating.DesignId());
	if(bi == m_Blueprints.end())
		bi = m_Blueprints.insert(rating.DesignId(), AuExchangeBlueprintRating());
	(*bi).AddReview(rating);
}

bool AuExchangeBlueprintRatings::Rating(quint32 designId, quint32& raters, float& rating, AuTime& lastRating) const
{
	QHash<quint32, AuExchangeBlueprintRating>::const_iterator bi = m_Blueprints.find(designId);
	if(bi == m_Blueprints.cend())
	{
		raters = 0;
		rating = 0;
		lastRating = AuTime();
		return false;
	}
	raters = (*bi).Ratings();
	rating = (*bi).RatingAverage();
	lastRating = (*bi).RatedLast();
	return true;
}

bool AuExchangeBlueprintRatings::Rating(quint32 designId, AuExchangeBlueprintRating& rating) const
{
	QHash<quint32, AuExchangeBlueprintRating>::const_iterator bi = m_Blueprints.find(designId);
	if(bi == m_Blueprints.cend())
		return false;
	rating = *bi;
	return true;
}

QString AuExchangeBlueprintRatings::WebReviews(quint32 designId) const
{
	QHash<quint32, AuExchangeBlueprintRating>::const_iterator bi = m_Blueprints.find(designId);
	if(bi == m_Blueprints.cend())
		return QString();
	return (*bi).WebReviews();
}

void AuExchangeBlueprintRatings::operator>>(QDataStream& out) const
{
	out << m_Blueprints;
}

void AuExchangeBlueprintRatings::operator<<(QDataStream& in)
{
	in >> m_Blueprints;
}

