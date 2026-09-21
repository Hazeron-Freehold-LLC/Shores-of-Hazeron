//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auexchangerating.h,v 1.6 2022/02/19 18:19:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuExchangeRating_H
#define AuExchangeRating_H

#include "audata.h"
#include "auid.h"
#include "autime.h"

class QSqlDatabase;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuExchangeRatingRec is a fully encapsulated rating of a design by a rater.
// Fields correspond 1:1 with n_BdRating and n_ScRating tables in auUniverse.

class AUDATA_DLL AuExchangeRatingRec
{
private:
	QString								m_Comment;
	quint32								m_DesignId;
	AuId								m_Rater;
	QString								m_RaterName;
	quint8								m_Rating;
	AuTime								m_WhenRated;

public:	// Construction
	inline AuExchangeRatingRec(const AuId& rater, const QString& raterName, quint32 designId, quint8 rating, const QString& comment,
			const AuTime& whenRated)
	{
		m_Comment = comment;
		m_DesignId = designId;
		m_Rater = rater;
		m_RaterName = raterName;
		m_Rating = rating;
		m_WhenRated = whenRated;
	}

	inline AuExchangeRatingRec()
	{
		m_DesignId = 0;
		m_Rating = 0;
	}

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	inline const QString& Comment() const
	{
		return m_Comment;
	}

	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	bool IsValid() const;

	inline const AuId& Rater() const
	{
		return m_Rater;
	}

	inline const QString& RaterName() const
	{
		return m_RaterName;
	}

	inline quint8 Rating() const
	{
		return m_Rating;
	}

	void SqlStoreBd(QSqlDatabase& db) const;

	void SqlStoreSc(QSqlDatabase& db) const;

	inline const AuTime& WhenRated() const
	{
		return m_WhenRated;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuExchangeRatingRec& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuExchangeRatingRec& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuExchangeRatingRec)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuExchangeRating represents a rating of a single design by an avatar.

class AUDATA_DLL AuExchangeRating
{
	QString								m_Comment;				// Comment by avatar who gave this rating.
	QString								m_RaterName;			// Name of avatar who gave this rating.
	quint8								m_Rating;				// 0 to 5
	AuTime								m_WhenRated;

public:	// Constructors
	AuExchangeRating(const QString& raterName, quint8 rating, const QString& comment, const AuTime& whenRated);
	AuExchangeRating(const AuExchangeRatingRec& rating);
	AuExchangeRating();
	~AuExchangeRating();
	AuExchangeRating(const AuExchangeRating& c);

public:	// Operators
	const AuExchangeRating& operator=(const AuExchangeRating& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	inline const QString& Comment() const
	{
		return m_Comment;
	}

	inline const QString& RaterName() const
	{
		return m_RaterName;
	}

	inline quint8 Rating() const
	{
		return m_Rating;
	}

	inline const AuTime& WhenRated() const
	{
		return m_WhenRated;
	}

};

inline QDataStream& operator<<(QDataStream& out, const AuExchangeRating& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuExchangeRating& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuExchangeBlueprintRating contains all of the ratings for a partincular blueprint.

class AUDATA_DLL AuExchangeBlueprintRating
{
	QHash<AuId, AuExchangeRating>		m_Reviews;				// Ratings for a blueprint. Key is rater id.

public:	// Constructors
	AuExchangeBlueprintRating();
	~AuExchangeBlueprintRating();
	AuExchangeBlueprintRating(const AuExchangeBlueprintRating& c);

public:	// Operators
	const AuExchangeBlueprintRating& operator=(const AuExchangeBlueprintRating& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddReview(const AuExchangeRatingRec& rating);

	inline bool IsRated() const
	{
		return !m_Reviews.empty();
	}

	AuTime RatedLast() const;

	// Returns the average of my ratings.
	float RatingAverage() const;

	// Returns the name of a bitmap that represents the average rating.
	static QString RatingAverageIconName(float rating);
	inline QString RatingAverageIconName() const
	{
		return RatingAverageIconName(RatingAverage());
	}

	// Formats fractional rating with fraction characters.
	static QString RatingAverageString(float rating);
	inline QString RatingAverageString() const
	{
		return RatingAverageString(RatingAverage());
	}

	inline unsigned int Ratings() const
	{
		return m_Reviews.size();
	}

	inline const QHash<AuId, AuExchangeRating>& Reviews() const
	{
		return m_Reviews;
	}

	// Returns the html formatted reviews for the web page detailing the blueprint.
	// Returns an empty string if this blueprint has no reviews.
	QString WebReviews() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuExchangeBlueprintRating& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuExchangeBlueprintRating& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuExchangeBlueprintRating)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuExchangeBlueprintRatings contains the ratings for all blueprints.

class AUDATA_DLL AuExchangeBlueprintRatings
{
	QHash<quint32, AuExchangeBlueprintRating>					m_Blueprints;	// Rated blueprints. Key is blueprint id.

public:	// Constructors
	AuExchangeBlueprintRatings();
	~AuExchangeBlueprintRatings();
	AuExchangeBlueprintRatings(const AuExchangeBlueprintRatings& c);

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddReview(const AuExchangeRatingRec& rating);

	// Returns true if 'designId' is found in my data.
	bool Rating(quint32 designId, quint32& raters, float& rating, AuTime& lastRating) const;

	// Returns true if 'designId' is found in my data.
	bool Rating(quint32 designId, AuExchangeBlueprintRating& rating) const;

	void SqlSelectBd(QSqlDatabase& db, quint32 designId);
	void SqlSelectSc(QSqlDatabase& db, quint32 designId);

	inline size_t size() const
	{
		return m_Blueprints.size();
	}

	// Returns the html formatted reviews for the web page detailing the blueprint.
	// Returns an empty string if the blueprint has no reviews.
	QString WebReviews(quint32 designId) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuExchangeBlueprintRatings& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuExchangeBlueprintRatings& o)
{
	o << in;
	return in;
}

#endif // AuExchangeRating_H
