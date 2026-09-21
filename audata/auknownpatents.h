//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auknownpatents.h,v 1.11 2022/02/19 18:19:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuKnownPatents_H
#define AuKnownPatents_H

#include "audata.h"
#include "dbindustry.h"
#include <QHash>

class AUDATA_DLL AuKnownPatents
{
	QHash<COMMODITYID, quint8>			m_Patents;				// Key is patent CID, value is quality of patent.

public:	// Constructors
	AuKnownPatents();
	~AuKnownPatents();
	AuKnownPatents(const AuKnownPatents& c);

public:	// Operators
	const AuKnownPatents& operator=(const AuKnownPatents& c);

	bool operator==(const AuKnownPatents& c) const;

	inline bool operator!=(const AuKnownPatents& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline QHash<COMMODITYID, quint8>::const_iterator cbegin() const
	{
		return m_Patents.cbegin();
	}

	inline QHash<COMMODITYID, quint8>::const_iterator cend() const
	{
		return m_Patents.cend();
	}

	inline bool empty() const
	{
		return m_Patents.empty();
	}

	// 'patent' is the commodity id of a patent.
	// 'quality' is the quality of the patent requested.
	// I can satisfy the request if I have stored that patent at the same or higher quality.
	// If I have stored patent 1 at Q44, then HasPatent(1,43) returns true; HasPatent(1,45) return false;
	inline bool HasPatent(COMMODITYID patent, quint8 quality) const
	{
		quint8 knownQ = PatentQuality(patent);
		if(!knownQ)
			return false;				// The patent is not known at all.
		return quality <= knownQ;		// True if the requested patent quality is the same or lower than my known patent quality.
	}

	bool HasWarpTechnology() const;

	QString HtmlListContents() const;

	// Lists patents into 'patents' that are available for execution by 'execIndustry'.
	void ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID execIndustry) const;

	// Use this one if DgData is already locked.
	void ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID execIndustry, const DgData& gd) const;

	quint64 MemoryUsage() const;

	// Returns the highest known quality of a patent, 0 if not known.
	// 'patent' is the commodity id of a patent.
	inline quint8 PatentQuality(COMMODITYID patent) const
	{
		QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.find(patent);
		if(pi == m_Patents.cend())
			return 0;
		return *pi;
	}

	// Called when a patent should be added to my list of known patents, because it is new or because 'quality' is higher than
	// the currently know patent.
	// Returns true if the patent is added, false if it was already known or if 'patent' is not a patent cid.
	bool RecordPatent(COMMODITYID patent, quint8 quality);

	// Returns true if 'patent' is actually removed.
	bool RemovePatent(COMMODITYID patent);

	// Adds all patents in 'o' to this list of known patents.
	// Returns the count of patents that get recorded.
	int RecordPatents(const AuKnownPatents& o);

	inline int size() const
	{
		return m_Patents.size();
	}

	unsigned int WarpTechnology() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuKnownPatents& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuKnownPatents& o)
{
	o << in;
	return in;
}

#endif // AuKnownPatents_H
