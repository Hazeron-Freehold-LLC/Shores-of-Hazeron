//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesignexchange.h,v 1.28 2022/02/19 18:18:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdDesignExchange_H
#define AuBdDesignExchange_H

#include "audata.h"
#include "auid.h"
#include "autime.h"
#include "dbindustry.h"

class AuBdReport;
class AuExchangeBlueprintRatings;
class DbIndustry;
class DeConstructionProcess;
class QSqlDatabase;

class AUDATA_DLL AuBdDesignExchangeEntry
{
friend class AuBdDesignExchange;
private:
	AuTime								m_RatedLast;			// When rated last.
	quint32								m_Raters;				// Number of avatars who have rated the design.
	float								m_Rating;				// 0 to 5
	AuBdReport*							m_Report;				// Architect is the seller. Avatars cannot sell others' designs.
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
	AuBdDesignExchangeEntry(const AuBdReport& report, quint8 state, quint32 price, bool canCopy, bool canView, bool useOnce,
			quint32 publisherEmpire, POLITICAL_STANCE minStance);

	AuBdDesignExchangeEntry();

	AuBdDesignExchangeEntry(const AuBdDesignExchangeEntry& c);

	~AuBdDesignExchangeEntry();

public:	// Operators
	AuBdDesignExchangeEntry& operator=(const AuBdDesignExchangeEntry& c);

	bool operator==(const AuBdDesignExchangeEntry& c) const;

	inline bool operator!=(const AuBdDesignExchangeEntry& c) const
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

	// True if the construction process has been initialized.
	bool CanBeConstructed() const;

	inline bool CanCopy() const
	{
		return m_CanCopy;
	}

	inline bool CanView() const
	{
		return m_CanView;
	}

	COMMODITYID ConstructionMaterial() const;

	const DeConstructionProcess& ConstructionProcess() const;

	AuId DesignArchitectId() const;

	const QString& DesignArchitectName() const;

	quint32 DesignId() const;

	const QString& DesignName() const;

	// Return value is a AuDesign::STATE.
	inline quint8 DesignState() const
	{
		return m_State;
	}

	BUILDING_STRUCTURE DesignStructure() const;

	quint8 DesignQuality() const;

	quint32 FaceCount() const;

	const AuPoint& HullMax() const;

	const AuPoint& HullMin() const;

	// Cubic meters.
	quint32 HullVolume() const;

	// Returns 0 if ok. Returns reason code if not.
	int IndustryCanUse(const DbIndustry& ind) const;

	// Returns the industry for which the blueprint was intended. It could be used for other industries.
	// Returns IID_None if the intended use for the blueprint was not specified.
	INDUSTRYID IndustryId() const;

	// Returns homes and jobs when this blueprint is used for a specific industry.
	// 'manufacturingJobs' are those that can actually run a manufacturing process to make something.
	// 'proJobs' are the professional jobs. For manufacturing buildings, this is 0. For other buildings, this is
	//   dependent on the needs of the building, e.g. auditoriums for churches, lounges for cantinas. No manufacturing building
	//   creates professional jobs also.
	// 'serviceJobs' are all other jobs created by the building, not including the manufacturing and professional jobs.
	// 'stores' are for sales to the public, set to store capacity if allowed at the industry.
	// 'lounges' are for relaxation by the public, set to lounge capacity if allowed at the industry.
	void IndustryInfo(const DbIndustry& dbi, quint32& homesLarge, quint32& homesMedium, quint32& homesSmall,
			quint32& manufacturingJobs, quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const;

	// Prevent designs by the same architect with the same name, if they are in the same state.
	bool IsDup(const AuBdDesignExchangeEntry& c) const;

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

	inline const AuBdReport& Report() const
	{
		return *m_Report;
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

	void SetReport(const AuBdReport& report);

	inline void SetWhenPublished()
	{
		m_When.Set();
	}

	inline void SetWhenPublished(const AuTime& whenPub)
	{
		m_When = whenPub;
	}

	const AuPoint& SiteMax() const;

	const AuPoint& SiteMin() const;

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

inline QDataStream& operator<<(QDataStream& out, const AuBdDesignExchangeEntry& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdDesignExchangeEntry& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuBdDesignExchangeEntry);	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is design id.

class AUDATA_DLL AuBdDesignExchange : public QHash<quint32, AuBdDesignExchangeEntry>
{
public:	// Constructors
	AuBdDesignExchange();
	virtual ~AuBdDesignExchange();
	AuBdDesignExchange(const AuBdDesignExchange& c);

public:	// Operators
	const AuBdDesignExchange& operator=(const AuBdDesignExchange& c);

public:	// Methods
	// Returns 1=added new entry, 2=replaced entry.
	int Add(const AuBdDesignExchangeEntry& libraryEntry);

	QString DesignName(quint32 designId) const;

	// Gets an entry.
	// Returns false if there is no 'designId' entry.
	bool Get(quint32 designId, AuBdDesignExchangeEntry& libraryEntry) const;

	QString HtmlListContents() const;

	// Lists the exchange entries that are visible to 'toWho' in 'toEmpire', based on political stance restrictions.
	void List(AuBdDesignExchange& list, const AuId& toWho, quint32 toEmpire) const;

	// Lists the exchange entries that are visible to 'toWho' in 'toEmpire', based on political stance restrictions.
	// Sets ratings information in the entries that are listed.
	void ListAndRate(AuBdDesignExchange& list, const AuId& toWho, quint32 toEmpire,
			const AuExchangeBlueprintRatings& ratings) const;

	// Lists the exchange entries that are visible to all empires, based on political stance restrictions.
	void ListPublic(AuBdDesignExchange& list) const;

	quint64 MemoryUsage() const;

	void Merge(const AuBdDesignExchange& mex);

	// Returns true if a 'designId' is actually removed.
	bool Remove(quint32 designId);

	void SqlSelectAll(QSqlDatabase& db);

	void SqlSelectEntry(QSqlDatabase& db, quint32 designId);
};

#endif // AuBdDesignExchange_H
