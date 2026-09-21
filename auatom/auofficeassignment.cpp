//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auofficeassignment.cpp,v 1.8 2022/02/19 17:40:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auofficeassignment.h"
//
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

void AuOfficeAssignment::operator>>(QDataStream& out) const
{
	out << (quint32)m_Empire;
	m_Person >> out;
}

void AuOfficeAssignment::operator<<(QDataStream& in)
{
	quint32 ui32;
	in >> ui32;
	m_Empire = ui32;
	m_Person << in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void AuOfficeAssignmentArray::Assign(const AuId& person, unsigned int empire)
{
	// Remove any existing assignment to the person or the empire.
	AuOfficeAssignmentArray::iterator it = begin();
	while(it != end())
	{
		if((*it).m_Person == person || (*it).m_Empire == empire)
			it = erase(it);
		else
			it++;
	}

	append(AuOfficeAssignment(person, empire));
}

AuId AuOfficeAssignmentArray::Person(unsigned int empire) const
{
	for(AuOfficeAssignmentArray::const_iterator it = constBegin();
			it != constEnd(); it++)
		if((*it).m_Empire == empire)
			return (*it).m_Person;
	return AuId(0);		// Return null id on failure.
}

void AuOfficeAssignmentArray::operator<<(QDataStream& in)
{
	quint32 cnt;
	in >> cnt;
	for(;cnt;cnt--)
	{
		AuOfficeAssignment oa;
		oa << in;
		append(oa);
	}
}

void AuOfficeAssignmentArray::operator>>(QDataStream& out) const
{
	out << (quint32)count();
	for(AuOfficeAssignmentArray::const_iterator it = constBegin();
			it != constEnd(); it++)
		(*it) >> out;
}

// Load this array with the assignments for a governed atom.
/*
void AuOfficeAssignmentArray::Load(QSqlDatabase& db, const AuId& governed)
{
	const char* sTableName = "e_OfficeAssignment";
	QString sq;
	sq.sprintf("SELECT ide,idp FROM %s WHERE ida=%d;",
			sTableName, governed.GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qv=0;
		unsigned int empire = qa.value(qv++).toUInt();
		AuId person = AuId(qa.value(qv++).toUInt());

		AuOfficeAssignment oa(person, empire);
		append(oa);
	}
}
*/

/*
void AuOfficeAssignmentArray::Store(QSqlDatabase& db,
		const AuId& governed) const
{
	for(AuOfficeAssignmentArray::const_iterator it = constBegin();
			it != constEnd(); it++)
		(*it).Store(db, governed);
}
*/

bool AuOfficeAssignmentArray::Revoke(const AuId& person)
{
	// Remove any existing assignment to the person.
	bool anyremoved = false;
	AuOfficeAssignmentArray::iterator it = begin();
	while(it != end())
	{
		if((*it).m_Person == person)
		{
			it = erase(it);
			anyremoved = true;
		}
		else
			it++;
	}
	return anyremoved;
}
