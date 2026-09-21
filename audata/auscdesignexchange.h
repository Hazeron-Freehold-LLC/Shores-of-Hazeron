//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdesignexchange.h,v 1.23 2022/02/19 18:19:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDesignExchange_H
#define AuScDesignExchange_H

#include "audata.h"
#include "auid.h"
#include "autime.h"

class AuExchangeBlueprintRatings;
class AuScReport;
class QSqlDatabase;

class AUDATA_DLL AuScDesignExchangeEntry
{
friend class AuScDesignExchange;
private:
	AuTime								m_RatedLast;			// When rated last.
	quint32								m_Raters;				// Number of avatars who have rated the design.
	float								m_Rating;				// 0 to 5
	AuScReport*							m_Report;				// Architect is the seller. Avatars cannot sell others' designs.
	quint8								m_State;				// AuDesign::STATE enum value.
	AuTime								m_When;					// When published.

	// Publishing terms.
	bool								m_CanCopy;				// Buyer can copy to clipboard and save to file. Implies CanView.
	bool								m_CanView;				// Buyer can view the design in a design studio.
	quint32								m_Empire;				// Empire of publisher.
	quint8								m_MinStance;			// Minimum POLITICAL_STANCE to buyer.
	quint32								m_Price;				// Price seller wants for the design.
	bool								m_UseOnce;				// Can buy one or set one manufacturing process, then it's gone.

public:	// Constructors
	AuScDesignExchangeEntry(const AuScReport& report, quint8 state, quint32 price, bool canCopy, bool canView, bool useOnce,
			quint32 publisherEmpire, POLITICAL_STANCE minStance);

	AuScDesignExchangeEntry();

	AuScDesignExchangeEntry(const AuScDesignExchangeEntry& c);

	~AuScDesignExchangeEntry();

public:	// Operators
	AuScDesignExchangeEntry& operator=(const AuScDesignExchangeEntry& c);

	bool operator==(const AuScDesignExchangeEntry& c) const;

	inline bool operator!=(const AuScDesignExchangeEntry& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	// Returns true if this exchange item is available to 'avatarId' in 'empireId', based on min stance restriction.
	bool AccessibleTo(const AuId& avatarId, quint32 empireId) const;

	// Returns true if this exchange item is available to someone in any empire, based on min stance restriction.
	bool AccessibleToAnyone() const;

	// True if the manufacturing process has been initialized.
	bool CanBeManufactured() const;

	inline bool CanCopy() const
	{
		return m_CanCopy;
	}

	inline bool CanView() const
	{
		return m_CanView;
	}

	AuId DesignArchitectId() const;

	const QString& DesignArchitectName() const;

	quint32 DesignId() const;

	const QString& DesignName() const;

	// Return value is a AuDesign::STATE.
	inline quint8 DesignState() const
	{
		return m_State;
	}

	SPACECRAFT_STRUCTURE DesignStructure() const;

	quint8 DesignQuality() const;

	double EstimatedCost() const;

	quint32 FaceCount() const;

	// Cubic meters.
	quint32 HullVolume() const;

	// Prevent designs by the same architect with the same name, if they are in the same state.
	bool IsDup(const AuScDesignExchangeEntry& c) const;

	inline AuTime LastMod() const
	{
		if(m_RatedLast != AuTime())
			return m_RatedLast;
		return m_When;
	}

	quint64 MemoryUsage() const;

	inline POLITICAL_STANCE MinStanceToBuyer() const
	{
		return (POLITICAL_STANCE)m_MinStance;
	}

	inline quint32 Price() const
	{
		return m_Price;
	}

	inline quint32 PublisherEmpire() const
	{
		return m_Empire;
	}

	inline const AuTime& RatedLast() const
	{
		return m_RatedLast;
	}

	inline quint32 Raters() const
	{
		return m_Raters;
	}

	inline float Rating() const
	{
		return m_Rating;
	}

	QString ReportText() const;

	QString ReportToolTip() const;

	inline void SetPublisherEmpire(quint32 empire)
	{
		m_Empire = empire;
	}

	// This information is transient, not stored in the database. It is acquired by querying the ratings data structures.
	inline void SetRating(quint32 raters, float rating, const AuTime& lastRated)
	{
		m_RatedLast = lastRated;
		m_Raters = raters;
		m_Rating = rating;
	}

	void SetReport(const AuScReport& report);

	inline void SetWhenPublished()
	{
		m_When.Set();
	}

	inline void SetWhenPublished(const AuTime& whenPub)
	{
		m_When = whenPub;
	}

	void SqlDelete(QSqlDatabase& db) const;

	void SqlInsert(QSqlDatabase& db) const;

	void SqlUpdate(QSqlDatabase& db) const;

	quint32 TriangleCount() const;

	inline bool UseOnce() const
	{
		return m_UseOnce;
	}

	inline const AuTime& WhenPublished() const
	{
		return m_When;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScDesignExchangeEntry& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScDesignExchangeEntry& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuScDesignExchangeEntry);	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is design id.

class AUDATA_DLL AuScDesignExchange : public QHash<quint32, AuScDesignExchangeEntry>
{
public:	// Constructors
	AuScDesignExchange();
	virtual ~AuScDesignExchange();
	AuScDesignExchange(const AuScDesignExchange& c);

public:	// Operators
	const AuScDesignExchange& operator=(const AuScDesignExchange& c);

public:	// Methods
	// Returns 1=added new entry, 2=replaced entry, 3=rejected dup entry
	int Add(const AuScDesignExchangeEntry& libraryEntry);

	QString DesignName(quint32 designId) const;

	// Gets an entry.
	// Returns false if there is no 'designId' entry.
	bool Get(quint32 designId, AuScDesignExchangeEntry& libraryEntry) const;

	QString HtmlListContents() const;

	// Lists the exchange entries that are visible to 'toWho' in 'toEmpire', based on political stance restrictions.
	void List(AuScDesignExchange& list, const AuId& toWho, quint32 toEmpire) const;

	// Lists the exchange entries that are visible to 'toWho' in 'toEmpire', based on political stance restrictions.
	// Sets ratings information in the entries that are listed.
	void ListAndRate(AuScDesignExchange& list, const AuId& toWho, quint32 toEmpire,
			const AuExchangeBlueprintRatings& ratings) const;

	// Lists the exchange entries that are visible to all empires, based on political stance restrictions.
	void ListPublic(AuScDesignExchange& list) const;

	quint64 MemoryUsage() const;

	void Merge(const AuScDesignExchange& mex);

	// Returns true if a 'designId' is actually removed.
	bool Remove(quint32 designId);

	void SqlSelectAll(QSqlDatabase& db);

	void SqlSelectEntry(QSqlDatabase& db, quint32 designId);
};

#endif // AuScDesignExchange_H
