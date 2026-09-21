//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audossier.h,v 1.20 2022/02/19 18:19:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDossier_H
#define AuDossier_H

#include "audef.h"
#include "audata.h"
#include "auid.h"
#include "autime.h"
#include <QList>
#include <typeinfo>			// for bad_cast

// Key is an AuDocument::DocumentType. Value is the number of new and open documents that trigger automatic enemy stance.
class AUDATA_DLL AuWarCriteria : public QHash<quint8, quint8>
{
public:
	inline void SetCriteria(quint8 docType, quint8 warCount)
	{
		if(warCount)
			insert(docType, warCount);	// Replace current war count.
		else
			remove(docType);			// Remove current war count.
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint8)count();			// The number we will store.
		for(const_iterator ci = cbegin(); ci != cend(); ci++)
		{
			out << ci.key();			// Document type
			out << *ci;					// Count
		}
	}

	inline void operator<<(QDataStream& in)
	{
		clear();

		quint8 n, d, c;
		in >> n;						// Number that were stored.
		while(n--)
		{
			in >> d;					// Document type
			in >> c;					// Count
			insert(d, c);
		}
	}

	quint64 MemoryUsage() const;
};

Q_DECLARE_METATYPE(AuWarCriteria)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuWarCriteria& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuWarCriteria& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocument
{
public:
	// NOTE: If you add states, update the StateName and StateIcon functions.
	enum DOC_STATE
	{
		DOC_New,
		DOC_Open,
		DOC_Pending,
		DOC_Closed,
		DOC_Deleted,
		DOC_Count				// LAST ITEM
	};

protected:
	AuTime						m_CreateTime;					// Time this document was created.
	quint8						m_State;						// DOC_STATE

public:	// Constructors/destructors
	AuDocument();												// Sets created time to current time.
	AuDocument(const AuTime& created);							// For importing old data.
	virtual ~AuDocument();
	AuDocument(const AuDocument& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Returns a duplicate of this document, allocated on the heap.
	virtual AuDocument* Clone() const = 0;

	static AuDocument* CreateDocument(quint8 docType);

	inline const AuTime& CreateTime() const
	{
		return m_CreateTime;
	}

	virtual QString Html(const QString& empireName) const = 0;

	virtual QString ListIcon() const = 0;

	virtual QString ListName() const = 0;

	virtual quint64 MemoryUsage() const;

	// Packs this document's type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	inline void SetDocState(DOC_STATE state)
	{
		if(state < DOC_New || state >= DOC_Count)
			return;
		m_State = state;
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	// Must be unique.
	virtual int SortKey() const = 0;

	inline DOC_STATE State() const
	{
		return (DOC_STATE)m_State;
	}

	inline QString StateIcon() const
	{
		return StateIcon(State());
	}

	static QString StateIcon(DOC_STATE state);

	inline QString StateName() const
	{
		return StateName(State());
	}

	static QString StateName(DOC_STATE state);

	virtual QString Subject() const = 0;

	// Returns a number that is unique to each derived class.
	virtual quint8 Type() const = 0;

	// Restores an item from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static AuDocument* Unpack(const QByteArray& ba);

	// Looks for a matching document to update, to avoid duplicates.
	// Returns 2 if this document is updated and changed, which prevents 'doc' from being added to the dossier.
	// Returns 1 if this document is a duplicate and not changed, which prevents 'doc' from being added to the dossier.
	// Returns 0 if this document is not a duplicate and not changed.
	virtual int Update(const AuDocument& doc) = 0;

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const = 0;
};

inline QDataStream& operator<<(QDataStream& out, const AuDocument& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDocument& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDossier : public QList<AuDocument*>
{
public:	// Constructors/Destructors
	inline AuDossier()
	{
	}

	inline ~AuDossier()
	{
		Clear();
	}

	inline AuDossier(const AuDossier& c) : QList<AuDocument*>()
	{
		for(AuDossier::const_iterator di = c.cbegin(); di != c.cend(); di++)
			append((*di)->Clone());
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline const AuDossier& operator=(const AuDossier& c)
	{
		Clear();
		for(AuDossier::const_iterator di = c.cbegin(); di != c.cend(); di++)
			append((*di)->Clone());
		return *this;
	}

public:	// Methods
	// 'crit' key is AuDocument::DocumentType, value is number of new and open documents of the type that cause a war declaration.
	bool CheckWarCriteria(const AuWarCriteria& crit) const;

	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	// Stores a new document if no corresponding document can be updated.
	// Returns true if any data actually changes.
	bool File(const AuDocument& doc);

	// Returns the time of the first document, regardless if it is a contact record.
	// Returns AuTime() if no documents exist.
	AuTime FirstContact() const;

	quint64 MemoryUsage() const;

	// Returns time on the newest 'new' document.
	AuTime NewestNewDoc() const;

	// Returns the state whose icon appears next to an empire, to highlight the most notable state in its dossier.
	AuDocument::DOC_STATE NotableState() const;

	// Returns true if a document is actually removed.
	inline bool RemoveAt(unsigned int index)
	{
		if(index >= (unsigned int)count())
			return false;

		AuDocument* c = at(index);
		delete c;
		removeAt(index);
		return true;
	}

	// 'docTime', 'docType' and 'docIndex' together are used to identify the exact document.
	inline void SetDocState(const AuTime& docTime, quint8 docType, quint16 docIndex, AuDocument::DOC_STATE state)
	{
		if(docIndex >= size())
			return;
		if(empty())
			return;

		iterator di = begin() + docIndex;
		if((*di)->Type() == docType && (*di)->CreateTime() == docTime)
		{
			if(state == AuDocument::DOC_Deleted)
			{
				delete *di;
				erase(di);
			}
			else
				(*di)->SetDocState(state);
		}
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuDossier& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDossier& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocUnitContact : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	quint8								m_OurStance;			// Our POLITICAL_STANCE taken when encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	quint8								m_TheirStance;			// Their POLITICAL_STANCE taken when encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocUnitContact(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
			POLITICAL_STANCE theirStance, const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocUnitContact(const AuTime& when);	// For importing old data.
	AuDocUnitContact();
	AuDocUnitContact(const AuDocUnitContact& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocUnitContact(*this);
	}

	inline static quint8 DocumentType()
	{
		return 1;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Contact");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocSetStance : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where stance was set.
	quint8								m_OurStance;			// Our POLITICAL_STANCE taken.
	AuId								m_OurUnitId;			// Id of the unit who set the stance.
	QString								m_OurUnitName;			// Name of the unit who set the stance.
	QString								m_Sector;				// Sector name where stance was set.
	QString								m_System;				// Solar system name where stance was set.
	quint8								m_TheirStance;			// Their POLITICAL_STANCE taken when stance was set.
	QString								m_World;				// World name where stance was set.

public:	// Construction/destruction
	AuDocSetStance(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
			POLITICAL_STANCE theirStance, GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSetStance(const AuTime& when, POLITICAL_STANCE ourStance);	// For importing old data.
	AuDocSetStance();
	AuDocSetStance(const AuDocSetStance& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSetStance(*this);
	}

	inline static quint8 DocumentType()
	{
		return 2;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Change Stance");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocInitOverlord : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where empire started.
	QString								m_OurUnitName;			// Name of the unit who started the empire.
	QString								m_Sector;				// Sector name where empire started.
	quint8								m_TheirStance;			// Their POLITICAL_STANCE taken.

public:	// Construction/destruction
	AuDocInitOverlord(const QString& ourUnitName, POLITICAL_STANCE theirStance, GALAXY galaxy, const QString& sector);
	AuDocInitOverlord();
	AuDocInitOverlord(const AuDocInitOverlord& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocInitOverlord(*this);
	}

	inline static quint8 DocumentType()
	{
		return 3;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Initial Overlord");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// They trespassed on our property and caused an automatic stance change.

class AUDATA_DLL AuDocTrespass : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocTrespass(const AuId& ourUnitId, const QString& ourUnitName, const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocTrespass();
	AuDocTrespass(const AuDocTrespass& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocTrespass(*this);
	}

	inline static quint8 DocumentType()
	{
		return 4;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Trespass");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the slaying of our unit by an enemy unit that caused a political stance change.

class AUDATA_DLL AuDocUnitKilled : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocUnitKilled(const AuId& ourUnitId, const QString& ourUnitName, const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocUnitKilled();
	AuDocUnitKilled(const AuDocUnitKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocUnitKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 5;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Unit Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocReciprocalStance : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	quint8								m_OurStance;			// Our POLITICAL_STANCE taken when encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocReciprocalStance(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
			const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocReciprocalStance();
	AuDocReciprocalStance(const AuDocReciprocalStance& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocReciprocalStance(*this);
	}

	inline static quint8 DocumentType()
	{
		return 6;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Reciprocal Stance");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocNewOverlord : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	quint8								m_OurStance;			// Our POLITICAL_STANCE taken when encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocNewOverlord(POLITICAL_STANCE ourStance, const AuId& ourUnitId, const QString& ourUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocNewOverlord();
	AuDocNewOverlord(const AuDocNewOverlord& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocNewOverlord(*this);
	}

	inline static quint8 DocumentType()
	{
		return 7;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Terminated Vassalage");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 120;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the destruction of an enemy unit by one of our units that caused a political stance change.

class AUDATA_DLL AuDocUnitDestroyed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocUnitDestroyed(const AuId& ourUnitId, const QString& ourUnitName, const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocUnitDestroyed();
	AuDocUnitDestroyed(const AuDocUnitDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocUnitDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 8;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Unit Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// We trespassed on their property.

class AUDATA_DLL AuDocTrespassed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocTrespassed(const AuId& ourUnitId, const QString& ourUnitName, const AuId& theirUnitId, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocTrespassed();
	AuDocTrespassed(const AuDocTrespassed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocTrespassed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 9;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Territorial Dispute");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocNote : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where encountered.
	AuId								m_OurUnitId;			// Id of the unit encountered.
	QString								m_OurUnitName;			// Name of the unit encountered.
	QString								m_Sector;				// Sector name where encountered.
	QString								m_System;				// Solar system name where encountered.
	QString								m_Text;					// Text of the note.
	QString								m_World;				// World name where encountered.

public:	// Construction/destruction
	AuDocNote(const AuId& ourUnitId, const QString& ourUnitName, const QString& text,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocNote();
	AuDocNote(const AuDocNote& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocNote(*this);
	}

	inline static quint8 DocumentType()
	{
		return 10;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Note");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A city reported a city trespassed on our property.

class AUDATA_DLL AuDocCityTrespassCity : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where this was reported.
	QString								m_OurUnitName;			// Name of the unit who reported this.
	QString								m_Sector;				// Sector name where this was reported.
	QString								m_System;				// Solar system name where this was reported.
	QString								m_TheirUnitName;		// Name of the unit that triggered this report.
	QString								m_World;				// World name where this was reported.

public:	// Construction/destruction
	AuDocCityTrespassCity(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCityTrespassCity();
	AuDocCityTrespassCity(const AuDocCityTrespassCity& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCityTrespassCity(*this);
	}

	inline static quint8 DocumentType()
	{
		return 11;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("City Reports Trespassing City");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 81;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A city reported a station trespassed on our property.

class AUDATA_DLL AuDocCityTrespassStation : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where this was reported.
	QString								m_OurUnitName;			// Name of the unit who reported this.
	QString								m_Sector;				// Sector name where this was reported.
	QString								m_System;				// Solar system name where this was reported.
	QString								m_TheirUnitName;		// Name of the unit that triggered this report.
	QString								m_World;				// World name where this was reported.

public:	// Construction/destruction
	AuDocCityTrespassStation(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCityTrespassStation();
	AuDocCityTrespassStation(const AuDocCityTrespassStation& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCityTrespassStation(*this);
	}

	inline static quint8 DocumentType()
	{
		return 12;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("City Reports Trespassing Station");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 90;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A station reported a city trespassed on our property.

class AUDATA_DLL AuDocStationTrespassCity : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where this was reported.
	QString								m_OurUnitName;			// Name of the unit who reported this.
	QString								m_Sector;				// Sector name where this was reported.
	QString								m_System;				// Solar system name where this was reported.
	QString								m_TheirUnitName;		// Name of the unit that triggered this report.
	QString								m_World;				// World name where this was reported.

public:	// Construction/destruction
	AuDocStationTrespassCity(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocStationTrespassCity();
	AuDocStationTrespassCity(const AuDocStationTrespassCity& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStationTrespassCity(*this);
	}

	inline static quint8 DocumentType()
	{
		return 13;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Station Reports Trespassing City");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 100;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A station reported a station trespassed on our property.

class AUDATA_DLL AuDocStationTrespassStation : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where this was reported.
	QString								m_OurUnitName;			// Name of the unit who reported this.
	QString								m_Sector;				// Sector name where this was reported.
	QString								m_System;				// Solar system name where this was reported.
	QString								m_TheirUnitName;		// Name of the unit that triggered this report.
	QString								m_World;				// World name where this was reported.

public:	// Construction/destruction
	AuDocStationTrespassStation(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocStationTrespassStation();
	AuDocStationTrespassStation(const AuDocStationTrespassStation& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStationTrespassStation(*this);
	}

	inline static quint8 DocumentType()
	{
		return 14;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Station Reports Trespassing Station");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 110;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the slaying of our citizen.

class AUDATA_DLL AuDocCitizenKilled : public AuDocument
{
protected:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocCitizenKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCitizenKilled();
	AuDocCitizenKilled(const AuDocCitizenKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCitizenKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 15;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Citizen Killed, Any Profession");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 130;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the destruction of our vehicle.

class AUDATA_DLL AuDocVehicleDestroyed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	AuId								m_OurUnitId;
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocVehicleDestroyed(const AuId& ourUnitId, const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocVehicleDestroyed();
	AuDocVehicleDestroyed(const AuDocVehicleDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocVehicleDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 16;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Vehicle Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 70;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the destruction of our spaceships.

class AUDATA_DLL AuDocSpaceshipDestroyed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceshipDestroyed(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceshipDestroyed();
	AuDocSpaceshipDestroyed(const AuDocSpaceshipDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceshipDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 17;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Spaceship Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 30;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the destruction of our starships.

class AUDATA_DLL AuDocStarshipDestroyed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocStarshipDestroyed(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocStarshipDestroyed();
	AuDocStarshipDestroyed(const AuDocStarshipDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStarshipDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 18;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Starship Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 10;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the destruction of our space stations.

class AUDATA_DLL AuDocSpaceStationDestroyed : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceStationDestroyed(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceStationDestroyed();
	AuDocSpaceStationDestroyed(const AuDocSpaceStationDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceStationDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 19;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Space Station Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 50;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the capture of our spaceships.

class AUDATA_DLL AuDocSpaceshipCaptured : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceshipCaptured(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceshipCaptured();
	AuDocSpaceshipCaptured(const AuDocSpaceshipCaptured& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceshipCaptured(*this);
	}

	inline static quint8 DocumentType()
	{
		return 20;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Spaceship Captured");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the capture of our starships.

class AUDATA_DLL AuDocStarshipCaptured : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocStarshipCaptured(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocStarshipCaptured();
	AuDocStarshipCaptured(const AuDocStarshipCaptured& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStarshipCaptured(*this);
	}

	inline static quint8 DocumentType()
	{
		return 21;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Starship Captured");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the capture of our space stations.

class AUDATA_DLL AuDocSpaceStationCaptured : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceStationCaptured(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceStationCaptured();
	AuDocSpaceStationCaptured(const AuDocSpaceStationCaptured& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceStationCaptured(*this);
	}

	inline static quint8 DocumentType()
	{
		return 22;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Space Station Captured");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the plunder of our spaceships.

class AUDATA_DLL AuDocSpaceshipPlundered : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceshipPlundered(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceshipPlundered();
	AuDocSpaceshipPlundered(const AuDocSpaceshipPlundered& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceshipPlundered(*this);
	}

	inline static quint8 DocumentType()
	{
		return 23;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Spaceship Plundered");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 40;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the plunder of our starships.

class AUDATA_DLL AuDocStarshipPlundered : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocStarshipPlundered(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocStarshipPlundered();
	AuDocStarshipPlundered(const AuDocStarshipPlundered& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStarshipPlundered(*this);
	}

	inline static quint8 DocumentType()
	{
		return 24;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Starship Plundered");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 20;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the plunder of our space stations.

class AUDATA_DLL AuDocSpaceStationPlundered : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the unit killed.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocSpaceStationPlundered(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocSpaceStationPlundered();
	AuDocSpaceStationPlundered(const AuDocSpaceStationPlundered& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocSpaceStationPlundered(*this);
	}

	inline static quint8 DocumentType()
	{
		return 25;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Space Station Plundered");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 60;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;					// This document types makes sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the capture of our city.

class AUDATA_DLL AuDocCityCaptured : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the city captured.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocCityCaptured(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCityCaptured();
	AuDocCityCaptured(const AuDocCityCaptured& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCityCaptured(*this);
	}

	inline static quint8 DocumentType()
	{
		return 26;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("City Captured");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents the capture of one of our bases.

class AUDATA_DLL AuDocBaseCaptured : public AuDocument
{
private:
	quint8								m_BaseIndex;
	quint8								m_Galaxy;				// GALAXY where killed.
	QString								m_OurUnitName;			// Name of the city captured.
	QString								m_Sector;				// Sector name where killed.
	QString								m_System;				// Solar system name where killed.
	QString								m_TheirUnitName;		// Name of the unit who did the killing.
	QString								m_World;				// World name where killed.

public:	// Construction/destruction
	AuDocBaseCaptured(const QString& ourUnitName, quint8 baseIndex, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocBaseCaptured();
	AuDocBaseCaptured(const AuDocBaseCaptured& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBaseCaptured(*this);
	}

	inline static quint8 DocumentType()
	{
		return 27;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Military Flag Captured");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents a diplomatic message received from another empire.

class AUDATA_DLL AuDocDiplomaticMessageReceived : public AuDocument
{
private:
	AuId								m_FromUnitId;			// Id of the unit who sent the diplomatic message.
	QString								m_FromUnitName;			// Name of the unit who sent the diplomatic message.
	QString								m_Text;					// Text of the diplomatic message.

public:	// Construction/destruction
	AuDocDiplomaticMessageReceived(const AuId& fromUnitId, const QString& fromUnitName, const QString& text);
	AuDocDiplomaticMessageReceived();
	AuDocDiplomaticMessageReceived(const AuDocDiplomaticMessageReceived& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocDiplomaticMessageReceived(*this);
	}

	inline static quint8 DocumentType()
	{
		return 28;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Diplomatic Message Received");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Documents a diplomatic message sent by one of our governors.

class AUDATA_DLL AuDocDiplomaticMessageSent : public AuDocument
{
private:
	AuId								m_FromUnitId;			// Id of the unit who sent the diplomatic message.
	QString								m_FromUnitName;			// Name of the unit who sent the diplomatic message.
	QString								m_Text;					// Text of the diplomatic message.

public:	// Construction/destruction
	AuDocDiplomaticMessageSent(const AuId& fromUnitId, const QString& fromUnitName, const QString& text);
	AuDocDiplomaticMessageSent();
	AuDocDiplomaticMessageSent(const AuDocDiplomaticMessageSent& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocDiplomaticMessageSent(*this);
	}

	inline static quint8 DocumentType()
	{
		return 29;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Diplomatic Message Sent");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocAvatarKilled : public AuDocCitizenKilled
{
public:	// Construction/destruction
	AuDocAvatarKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocAvatarKilled();
	AuDocAvatarKilled(const AuDocAvatarKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocAvatarKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 30;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Avatar Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 140;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocOfficerKilled : public AuDocCitizenKilled
{
	QString								m_HomeWorld;

public:	// Construction/destruction
	AuDocOfficerKilled(const QString& ourUnitName, const QString& ourUnitHomeWorld, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocOfficerKilled();
	AuDocOfficerKilled(const AuDocOfficerKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocOfficerKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 31;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Officer Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 150;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocCrewKilled : public AuDocCitizenKilled
{
public:	// Construction/destruction
	AuDocCrewKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCrewKilled();
	AuDocCrewKilled(const AuDocCrewKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCrewKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 32;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Crew Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 160;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocTroopKilled : public AuDocCitizenKilled
{
public:	// Construction/destruction
	AuDocTroopKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocTroopKilled();
	AuDocTroopKilled(const AuDocTroopKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocTroopKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 33;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Troop Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 170;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocPassengerKilled : public AuDocCitizenKilled
{
public:	// Construction/destruction
	AuDocPassengerKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocPassengerKilled();
	AuDocPassengerKilled(const AuDocPassengerKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocPassengerKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 34;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Passenger Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 180;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocWorkerKilled : public AuDocCitizenKilled
{
public:	// Construction/destruction
	AuDocWorkerKilled(const QString& ourUnitName, const QString& theirUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocWorkerKilled();
	AuDocWorkerKilled(const AuDocWorkerKilled& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocWorkerKilled(*this);
	}

	inline static quint8 DocumentType()
	{
		return 35;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Worker Killed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 190;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocWar : public AuDocument
{
public:	// Construction/destruction
	AuDocWar();
	AuDocWar(const AuDocWar& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocWar(*this);
	}

	inline static quint8 DocumentType()
	{
		return 36;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("War Declaration");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;					// This document types does not make sense as war criteria.
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocStanceDowngraded : public AuDocument
{
private:
	quint8								m_TheirNewStance;		// Their POLITICAL_STANCE after they changed it.
	quint8								m_TheirOldStance;		// Their POLITICAL_STANCE before they changed it.
	AuId								m_TheirUnitId;			// Id of the unit who made the change.
	QString								m_TheirUnitName;		// Name of the unit who made the change.

public:	// Construction/destruction
	AuDocStanceDowngraded(POLITICAL_STANCE theirOldStance, POLITICAL_STANCE theirNewStance, const AuId& theirUnitId,
			const QString& theirUnitName);
	AuDocStanceDowngraded();
	AuDocStanceDowngraded(const AuDocStanceDowngraded& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStanceDowngraded(*this);
	}

	inline static quint8 DocumentType()
	{
		return 37;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Stance Downgraded");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 115;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocStanceUpgraded : public AuDocument
{
private:
	quint8								m_TheirNewStance;		// Their POLITICAL_STANCE after they changed it.
	quint8								m_TheirOldStance;		// Their POLITICAL_STANCE before they changed it.
	AuId								m_TheirUnitId;			// Id of the unit who made the change.
	QString								m_TheirUnitName;		// Name of the unit who made the change.
	quint8								m_VassalPercent;

public:	// Construction/destruction
	AuDocStanceUpgraded(POLITICAL_STANCE theirOldStance, POLITICAL_STANCE theirNewStance, quint8 vassalPercent,
			const AuId& theirUnitId, const QString& theirUnitName);
	AuDocStanceUpgraded();
	AuDocStanceUpgraded(const AuDocStanceUpgraded& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocStanceUpgraded(*this);
	}

	inline static quint8 DocumentType()
	{
		return 38;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Stance Upgraded");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocCityRevolt : public AuDocument
{
private:
	QString								m_City;					// Name of city that revolted.
	quint8								m_Galaxy;				// GALAXY where the city revolted.
	QString								m_Sector;				// Sector name where city revolted.
	QString								m_System;				// Solar system name where city revolted.
	QString								m_World;				// World name where city revolted.

public:	// Construction/destruction
	AuDocCityRevolt(const QString& city, GALAXY galaxy, const QString& sector, const QString& system, const QString& world);
	AuDocCityRevolt();
	AuDocCityRevolt(const AuDocCityRevolt& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocCityRevolt(*this);
	}

	inline static quint8 DocumentType()
	{
		return 39;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("City Revolted");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 999;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocClaimHijacked : public AuDocument
{
private:
	quint8								m_Galaxy;				// GALAXY where stance was set.
	QString								m_Sector;				// Sector name where stance was set.
	QString								m_System;				// Solar system name where stance was set.
	AuId								m_TheirUnitId;			// Id of the unit encountered.
	QString								m_TheirUnitName;		// Name of the unit encountered.
	QString								m_World;				// World name where stance was set.

public:	// Construction/destruction
	AuDocClaimHijacked(const AuId& theirUnitId, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world);
	AuDocClaimHijacked();
	AuDocClaimHijacked(const AuDocClaimHijacked& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocClaimHijacked(*this);
	}

	inline static quint8 DocumentType()
	{
		return 40;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Claim Hijacked");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 80;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdDestroyed : public AuDocument
{
protected:
	QString								m_BdName;				// Name of the building that was destroyed.
	QString								m_City;					// City or base name of the building, or blank.
	quint8								m_Galaxy;				// GALAXY where building was destroyed.
	QString								m_Sector;				// Sector name where building was destroyed.
	QString								m_System;				// Solar system name where building was destroyed.
	QString								m_TheirUnitName;		// Name of the unit who destroyed a building.
	QString								m_World;				// World name where building was destroyed.

public:	// Construction/destruction
	AuDocBdDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdDestroyed();
	AuDocBdDestroyed(const AuDocBdDestroyed& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 41;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Building Destroyed");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 75;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdCityCapitolDestroyed : public AuDocBdDestroyed
{
public:	// Construction/destruction
	AuDocBdCityCapitolDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdCityCapitolDestroyed();
	AuDocBdCityCapitolDestroyed(const AuDocBdCityCapitolDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdCityCapitolDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 42;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListName() const
	{
		return QObject::tr("City Capitol Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 74;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdSectorCapitolDestroyed : public AuDocBdDestroyed
{
public:	// Construction/destruction
	AuDocBdSectorCapitolDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdSectorCapitolDestroyed();
	AuDocBdSectorCapitolDestroyed(const AuDocBdSectorCapitolDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdSectorCapitolDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 43;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListName() const
	{
		return QObject::tr("Sector Capital City Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 73;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdGalaxyCapitolDestroyed : public AuDocBdDestroyed
{
public:	// Construction/destruction
	AuDocBdGalaxyCapitolDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdGalaxyCapitolDestroyed();
	AuDocBdGalaxyCapitolDestroyed(const AuDocBdGalaxyCapitolDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdGalaxyCapitolDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 44;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListName() const
	{
		return QObject::tr("Galaxy Capital City Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 72;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdEmpireCapitolDestroyed : public AuDocBdDestroyed
{
public:	// Construction/destruction
	AuDocBdEmpireCapitolDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdEmpireCapitolDestroyed();
	AuDocBdEmpireCapitolDestroyed(const AuDocBdEmpireCapitolDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdEmpireCapitolDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 45;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListName() const
	{
		return QObject::tr("Empire Capital City Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 71;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdMilitaryDestroyed : public AuDocBdDestroyed
{
public:	// Construction/destruction
	AuDocBdMilitaryDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdMilitaryDestroyed();
	AuDocBdMilitaryDestroyed(const AuDocBdMilitaryDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdMilitaryDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 46;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Military Building Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 78;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdMilitaryHQDestroyed : public AuDocBdMilitaryDestroyed
{
public:	// Construction/destruction
	AuDocBdMilitaryHQDestroyed(const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdMilitaryHQDestroyed();
	AuDocBdMilitaryHQDestroyed(const AuDocBdMilitaryHQDestroyed& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdMilitaryHQDestroyed(*this);
	}

	inline static quint8 DocumentType()
	{
		return 47;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListName() const
	{
		return QObject::tr("Military Base Destroyed");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 77;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdDamaged : public AuDocument
{
protected:
	AuId								m_BdId;
	QString								m_BdName;				// Name of the building that was damaged.
	QString								m_City;					// City or base name of the building, or blank.
	quint8								m_Galaxy;				// GALAXY where building was damaged.
	QString								m_Sector;				// Sector name where building was damaged.
	QString								m_System;				// Solar system name where building was damaged.
	QString								m_TheirUnitName;		// Name of the unit who damaged a building.
	QString								m_World;				// World name where building was damaged.

public:	// Construction/destruction
	AuDocBdDamaged(const AuId& id, const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdDamaged();
	AuDocBdDamaged(const AuDocBdDamaged& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdDamaged(*this);
	}

	inline static quint8 DocumentType()
	{
		return 48;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Building Damaged");
	}

	virtual quint64 MemoryUsage() const;

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 76;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}

	virtual int Update(const AuDocument& doc);

	// Returns true for documents that make sense to be included in automatic war declaration criteria.
	virtual bool WarCriteria() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDocBdMilitaryDamaged : public AuDocBdDamaged
{
public:	// Construction/destruction
	AuDocBdMilitaryDamaged(const AuId& id, const QString& name, const QString& theirUnitName, GALAXY galaxy, const QString& sector,
			const QString& system, const QString& world, const QString& city);
	AuDocBdMilitaryDamaged();
	AuDocBdMilitaryDamaged(const AuDocBdMilitaryDamaged& c);

public:
	virtual AuDocument* Clone() const
	{
		return new AuDocBdMilitaryDamaged(*this);
	}

	inline static quint8 DocumentType()
	{
		return 49;
	}

	virtual QString Html(const QString& empireName) const;

	virtual QString ListIcon() const;

	virtual QString ListName() const
	{
		return QObject::tr("Military Building Damaged");
	}

	// Used to order the war criteria settings on the user interface. See AuWarCriteriaDlg.
	virtual int SortKey() const
	{
		return 79;
	}

	virtual QString Subject() const;

	// Returns a number that is unique to each derived condition class.
	virtual quint8 Type() const
	{
		return DocumentType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuDossier_H
