//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auofficeassignment.h,v 1.7 2022/02/19 17:32:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuOfficeAssignment_H
#define AuOfficeAssignment_H

#include <auid.h>
#include <QList>

class QSqlDatabase;

class AuOfficeAssignment
{
public:
	unsigned int	m_Empire;		// Empire of office assignment.
	AuId			m_Person;		// Person assigned this office.

public:
	AuOfficeAssignment()
	{
		m_Empire = 0;
	}
	AuOfficeAssignment(const AuId& person, unsigned int empire)
	{
		m_Empire = empire;
		m_Person = person;
	}

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;
};

class AuOfficeAssignmentArray : public QList<AuOfficeAssignment>
{
public:
	void Assign(const AuId& person, unsigned int empire);
	//void Load(QSqlDatabase& db, const AuId& governed);
	AuId Person(unsigned int empire) const;
	bool Revoke(const AuId& person);	// true if an assignment is revoked

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;
};

#endif // AuOfficeAssignment_H
