//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dzspecies.cpp,v 1.20 2022/02/19 18:21:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dzspecies.h"
//
#include "dhdna.h"
#include <QDataStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

// Must call qRegisterMetaType in order to use this class type in queued
// signal and slot connections. The Q_DECLARE_METATYPE macro is also required.
static int s_DzSpeciesMetaType = qRegisterMetaType<DzSpecies>();

DzSpecies::DzSpecies(const AuSpecies& species, const QString& name, const AuId& whoNamed) : m_Species(species)
{
	m_Name = name;
	m_WhenNamed.Set();
	m_WhoNamed = whoNamed;
}

DzSpecies::DzSpecies()
{
}

DzSpecies::~DzSpecies()
{
}

DzSpecies::DzSpecies(const DzSpecies &c) : m_Species(c.m_Species)
{
	m_Name = c.m_Name;
	m_WhenNamed = c.m_WhenNamed;
	m_WhoNamed = c.m_WhoNamed;
}

DzSpecies& DzSpecies::operator=(const DzSpecies &c)
{
	m_Name = c.m_Name;
	m_Species = c.m_Species;
	m_WhenNamed = c.m_WhenNamed;
	m_WhoNamed = c.m_WhoNamed;
	return *this;
}

void DzSpecies::operator<<(QDataStream& in)
{
	in >> m_Name;
	in >> m_Species;
	in >> m_WhenNamed;
	in >> m_WhoNamed;
}

void DzSpecies::operator>>(QDataStream& out) const
{
	out << m_Name;
	out << m_Species;
	out << m_WhenNamed;
	out << m_WhoNamed;
}

void DzSpecies::Insert(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq = QString("INSERT INTO g_Species (species,name,whenNamed,whoNamed) VALUES (?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(m_Species.Key());
	qa.addBindValue(m_Name);
	qa.addBindValue(m_WhenNamed.UTC());
	qa.addBindValue(m_WhoNamed.GetID());

	if(!qa.exec())
		qWarning("ERROR! DzSpecies::Insert FAILED '%s'", (const char*)sq.toLatin1());
}

void DzSpecies::Update(QSqlDatabase& db) const
{
	QSqlQuery qa(QString(), db);
	QString sq = QString("UPDATE g_Species SET name=?, whenNamed=?, whoNamed=? WHERE species=?;");
	qa.prepare(sq);
	qa.addBindValue(m_Name);
	qa.addBindValue(m_WhenNamed.UTC());
	qa.addBindValue(m_WhoNamed.GetID());
	qa.addBindValue(m_Species.Key());

	if(!qa.exec())
		qWarning("ERROR! DzSpecies::Update FAILED '%s'", (const char*)sq.toLatin1());
}

bool DzSpecies::Select(QSqlDatabase& db, const AuSpecies& species)
{
	QSqlQuery qry(QString(), db);
	QString sq = QString("SELECT name, whenNamed, whoNamed FROM g_Species WHERE species=?;");
	qry.prepare(sq);
	qry.addBindValue(species.Key());
	qry.exec();
	if(qry.first())
	{
		m_Name = qry.value(0).toString();
		m_Species = species;

		QDateTime t = qry.value(1).toDateTime();
		t.setTimeSpec(Qt::UTC);
		m_WhenNamed = t;

		m_WhoNamed = AuId(qry.value(2).toUInt());
		return true;
	}
	else
		return false;
}

quint64 DzSpecies::MemoryUsage() const
{
	quint64 mem = sizeof(DzSpecies);
	mem += StringMemoryUsage(m_Name);
	mem += m_Species.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void DzSpeciesHash::operator<<(QDataStream& in)
{
	clear();
	quint32 cnt;
	in >> cnt;
	DzSpecies species;
	for(; cnt; cnt--)
	{
		species << in;
		insert(pair<AuSpecies, DzSpecies>(species.Species(), species));
	}
}

void DzSpeciesHash::operator>>(QDataStream& out) const
{
	out << (quint32)size();
	for(const_iterator it = begin(); it != end(); it++)
		(*it).second >> out;
}

void DzSpeciesHash::Load()
{
	clear();

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	DzSpecies ob;
	QString sq("SELECT species, name, whenNamed, whoNamed FROM g_Species;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		ob.m_Species = AuSpecies(qry.value(0).toByteArray());
		ob.m_Name = qry.value(1).toString();

		QDateTime t = qry.value(2).toDateTime();
		t.setTimeSpec(Qt::UTC);
		ob.m_WhenNamed = t;

		ob.m_WhoNamed = AuId(qry.value(3).toUInt());

		insert(pair<AuSpecies, DzSpecies>(ob.m_Species, ob));
	}

	//if(count() == 0)
		//qDebug("DzSpeciesHash::Load no records loaded.");
}

quint64 DzSpeciesHash::MemoryUsage() const
{
	quint64 mem = sizeof(DzSpeciesHash);
	for(const_iterator it = begin(); it != end(); it++)
		mem += (*it).second.MemoryUsage();
	return mem;
}
