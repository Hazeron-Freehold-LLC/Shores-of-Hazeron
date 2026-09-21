//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auid.h,v 1.31 2022/02/19 18:42:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuId_H
#define AuId_H

#include "audef.h"
#include "auutil.h"
#include <QList>
#include <QMetaType>

using namespace std;

class QDebug;

class AUUTIL_DLL AuId  
{
private:
	unsigned int		m_Id;

public:	// Construction
	inline AuId()
	{
		m_Id = 0;
	}

	inline AuId(unsigned int id)
	{
		m_Id = id;
	}

	inline AuId(const QString& sid)
	{
		m_Id = QVariant(sid).toUInt();	// Try turning the string directly into a number.
		if(!m_Id)
			SetStringId(sid);			// It wasn't an obvious number. Decode a string.
	}

	inline ~AuId()
	{
	}

public:	// Operators
	inline bool operator==(const AuId& c) const
	{
		if(m_Id != c.m_Id)
			return false;
		return true;
	}
	inline bool operator!=(const AuId& c) const
	{
		return !operator==(c);
	}
	inline bool operator<(const AuId& c) const
	{
		if(m_Id < c.m_Id)
			return true;
		return false;
	}
	inline bool operator>(const AuId& c) const
	{
		if(m_Id > c.m_Id)
			return true;
		return false;
	}

	// NOTE: operator+ is used by the Id allocator. Don't use indiscriminately.
	inline AuId operator+(unsigned int val)
	{
		return AuId(m_Id + val);
	}
	inline AuId operator-(unsigned int val)
	{
		return AuId(m_Id - val);
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Returns the first unreserved atom id, for initialization of a new database.
	static AuId FirstUnreserved();

	// Returns GALAXY identified by this AuId.
	// Returns GALAXY_Count on failure.
	GALAXY Galaxy() const;

	static AuId GalaxyId(GALAXY galaxy);

	inline unsigned int GetID() const
	{
		return(m_Id);
	}

	inline bool IsNull() const
	{
		return m_Id == 0 ? true : false;
	}

	static const AuId& NullId();

	static int RegisterMetaType();

	inline void SetNull()
	{
		m_Id = 0;
	}

	void SetStringId(const QString& sid);	// Sets this id value from a string id formatted using StringId().

	QString StringId() const;				// Expresses the id as a string.

	// Must never match an atom id.
	static AuId GovtMail()
	{
		return AuId(1);
	}

	// Must not match the id of a legitimate mail sender atom, such as a person or city.
	static AuId BankMail()
	{
		return AuId(5);
	}

	// Must not match the id of a legitimate mail sender atom, such as a person or city.
	static AuId LibraryMail()
	{
		return AuId(6);
	}

	// Must not match the id of a legitimate mail sender atom, such as a person or city.
	static AuId MailServer()
	{
		return  AuId(7);
	}

	static AuId Haxus()
	{
		return AuId(16);					// 'Q' :D
	}

	static AuId Silverwill()
	{
		return AuId(18);					// 'S' :D
	}

	// Atom id of a fictitious emperor of a pirate empire.
	static AuId AksonEmperor()
	{
		return AuId(80);
	}

	static AuId BaloriteEmperor()
	{
		return AuId(81);
	}

	static AuId DendraeEmperor()
	{
		return AuId(82);
	}

	static AuId HaxuEmperor()
	{
		return AuId(83);
	}

	static AuId KlatraEmperor()
	{
		return AuId(84);
	}

	static AuId MalaconEmperor()
	{
		return AuId(85);
	}

	static AuId MyntakaEmperor()
	{
		return AuId(86);
	}

	static AuId OgarEmperor()
	{
		return AuId(87);
	}

	static AuId OtariEmperor()
	{
		return AuId(88);
	}

	static AuId SeledonEmperor()
	{
		return AuId(89);
	}

	static AuId SythEmperor()
	{
		return AuId(90);
	}

	static AuId TassadEmperor()
	{
		return AuId(91);
	}

	static AuId VilmortiEmperor()
	{
		return AuId(92);
	}

	static AuId VreenEmperor()
	{
		return AuId(93);
	}

	static AuId ZuulEmperor()
	{
		return AuId(94);
	}

	// This is the id for communications from the Planetary Indemnification Guild.
	// ::InsurerName() (audef.h) provides an associated name.
	// This is also the highest known reserved atom id. If that changes, update FirstUnreserved.
	static AuId InsurerId()
	{
		return AuId(1052);	// "MOB"
	}

	static AuId PigId()
	{
		return AuId(4279);	// "PIG"
	}

	static AuId Universe()
	{
		return AuId(101);
	}
};

Q_DECLARE_METATYPE(AuId)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

class AUUTIL_DLL AuIdHash
{
public:
	inline size_t operator()(const AuId& o) const
	{
		return hash<unsigned int>()(o.GetID());
	}
};

inline unsigned int qHash(const AuId& key)
{
	return key.GetID();
}

inline QDataStream& operator<<(QDataStream& out, const AuId& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuId& o)
{
	o << in;
	return in;
}

AUUTIL_DLL QDebug operator<<(QDebug dbg, const AuId& id);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUUTIL_DLL AuIdList : public QList<AuId>
{
public:	// Operators
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

public:	// Methods
	bool AddIfUnique(const AuId &id);

	inline bool Contains(const AuId& id) const
	{
		return indexOf(id) >= 0;
	}

	static int RegisterMetaType();

	void Remove(const AuId &idRemove);
};

Q_DECLARE_METATYPE(AuIdList)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuId_H
