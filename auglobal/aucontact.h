//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontact.h,v 1.7 2022/02/19 18:30:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Buddy List and Squelch List items
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuContact_H
#define AuContact_H

#include "augloballib.h"
#include "auid.h"

// List of players this person has identified as a buddy or an annoyance.
class AUGLOBAL_DLL AuContact
{
public:
	AuId			m_id;			// Id of the person
	QString			m_name;			// Person name to help in lookup
	bool			m_bIgnore;		// Ignore messages from this person

public:	// Construction
	inline AuContact()
	{
		m_bIgnore = false;
	}

	inline AuContact(const AuId& id, const QString& name, bool bIgnore=false)
	{
		m_id = id;
		m_name = name;
		m_bIgnore = bIgnore;
	}

	inline AuContact(const AuContact& c)
	{
		m_id = c.m_id;
		m_name = c.m_name;
		m_bIgnore = c.m_bIgnore;
	}

public:	// Operators
	inline AuContact& operator=(const AuContact& c)
	{
		m_id = c.m_id;
		m_name = c.m_name;
		m_bIgnore = c.m_bIgnore;
		return *this;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_id;
		in >> m_name;
		in >> m_bIgnore;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_id;
		out << m_name;
		out << m_bIgnore;
	}

	inline bool operator==(const AuContact& c) const
	{
		if(m_id != c.m_id)
			return false;
		if(m_bIgnore != c.m_bIgnore)
			return false;
		if(m_name != c.m_name)
			return false;
		return true;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuContact& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuContact& o)
{
	o << in;
	return in;
}

#endif // AuContact_H
