//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: deinvitemid.h,v 1.12 2022/02/19 18:19:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DeInvItemId_H
#define DeInvItemId_H

#include "audata.h"
#include "dbcommodity.h"

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Used to positively identify an inventory item in a collection of inventory items.
// Useful in lists and comm.

class AUDATA_DLL DeInvItemId
{
public:
	quint16								m_CID;					// COMMODITYID
	quint32								m_EmpireOfOrigin;		// Empire id of origin. May be 0.
	quint32								m_IdNumber;			// Design id for design disk. Mission text length for mission disk.
	QString								m_IdText;			// Mission name for mission disk.
	quint8								m_Quality;				// 0 is not really valid

public:
	inline DeInvItemId(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin)
	{
		Q_ASSERT(quality);
		m_CID = cid;
		m_EmpireOfOrigin = empireOfOrigin;
		m_IdNumber = 0;
		m_Quality = quality;
	}

	inline DeInvItemId(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 spacecraftDesignId)
	{
		Q_ASSERT(quality);
		m_CID = cid;
		m_EmpireOfOrigin = empireOfOrigin;
		m_IdNumber = spacecraftDesignId;
		m_Quality = quality;
	}

	inline DeInvItemId(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& missionName,
			quint32 missionTextLength)
	{
		Q_ASSERT(quality);
		m_CID = cid;
		m_EmpireOfOrigin = empireOfOrigin;
		m_IdNumber = missionTextLength;
		m_IdText = missionName;
		m_Quality = quality;
	}

	inline DeInvItemId()
	{
		m_CID = CID_Invalid;
		m_EmpireOfOrigin = 0;
		m_IdNumber = 0;
		m_Quality = 1;
	}

	inline DeInvItemId(const QString& s)
	{
		FromString(s);
	}

	inline DeInvItemId(const DeInvItemId& c)
	{
		m_CID = c.m_CID;
		m_EmpireOfOrigin = c.m_EmpireOfOrigin;
		m_IdNumber = c.m_IdNumber;
		m_IdText = c.m_IdText;
		m_Quality = c.m_Quality;
	}

	inline const DeInvItemId& operator=(const DeInvItemId& c)
	{
		m_CID = c.m_CID;
		m_EmpireOfOrigin = c.m_EmpireOfOrigin;
		m_IdNumber = c.m_IdNumber;
		m_IdText = c.m_IdText;
		m_Quality = c.m_Quality;

		return *this;
	}

public:	// Operators
	// This function is needed to use this object as a key in a QMap.
	inline bool operator<(const DeInvItemId& c) const
	{
		if(m_CID < c.m_CID)
			return true;
		if(m_EmpireOfOrigin < c.m_EmpireOfOrigin)
			return true;
		if(m_IdNumber < c.m_IdNumber)
			return true;
		if(m_Quality < c.m_Quality)
			return true;
		if(m_IdText < c.m_IdText)
			return true;
		return false;
	}

	inline bool operator==(const DeInvItemId& c) const
	{
		if(m_CID != c.m_CID)
			return false;
		if(m_EmpireOfOrigin != c.m_EmpireOfOrigin)
			return false;
		if(m_IdNumber != c.m_IdNumber)
			return false;
		if(m_IdText != c.m_IdText)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		return true;
	}

	inline bool operator!=(const DeInvItemId& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_CID;
		out << m_EmpireOfOrigin;
		out << m_IdNumber;
		out << m_IdText;
		out << m_Quality;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_CID;
		in >> m_EmpireOfOrigin;
		in >> m_IdNumber;
		in >> m_IdText;
		in >> m_Quality;
	}

public:	// Methods
	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	// Decode our encoded identifier string, as returned by ToString.
	inline void FromString(const QString& s)
	{
		m_CID = s.left(4).toUInt(0, 16);
		m_Quality = s.mid(4,2).toUInt(0, 16);
		if(!m_Quality)
			m_Quality = 1;

		int psign = s.indexOf(QChar('#'));
		if(psign < 9)
		{
			m_EmpireOfOrigin = 0;
			m_IdNumber = 0;
			m_IdText = "";
			return;
		}
		m_EmpireOfOrigin = s.mid(6, psign-6).toUInt(0, 16);

		int pdollar = s.indexOf(QChar('$'));
		if(pdollar < 11)
		{
			m_IdNumber = 0;
			m_IdText = "";
			return;
		}
		m_IdNumber = s.mid(psign+1, pdollar-(psign+1)).toUInt(0, 16);

		if(s.length() <= pdollar+1)
			m_IdText = "";
		else
			m_IdText = s.mid(pdollar+1);
	}

	bool IsValid() const;

	// Encode this identifier into a compact string that can be decoded using FromString.
	inline QString ToString() const
	{
		return QString("%1%2%3#%5$%6")
			.arg(m_CID, 4, 16, QChar('0'))
			.arg(m_Quality, 2, 16, QChar('0'))
			.arg(m_EmpireOfOrigin, 0, 16)
			.arg(m_IdNumber, 0, 16)
			.arg(m_IdText);
	}

#if defined(Q_OS_WIN)
	// Every function in this class is inline, which is valid in C++.
	// The Windows linker emits a warning that this class does not export any functions.
	// When did that become a requirement of C++?
	// This function exists to suppress that linker warning.
	void BogusFunctionToSuppressWindowsLinkerWarning() const;
#endif
};

inline QDataStream& operator<<(QDataStream& out, const DeInvItemId& invId)
{
	invId >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeInvItemId& invId)
{
	invId << in;
	return in;
}

#endif // DeInvItemId_H
