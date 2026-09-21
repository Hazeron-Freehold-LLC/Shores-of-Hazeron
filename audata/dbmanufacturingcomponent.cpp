//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbmanufacturingcomponent.cpp,v 1.10 2022/04/08 14:47:22 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dbmanufacturingcomponent.h"
//
#include "auknownpatents.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include <QVariant>

DbManufacturingComponent::DbManufacturingComponent()
{
	m_id = 0;
	m_idmp = 0;
	m_idc = 0;
	m_consumed = 0;
	m_ide = 0;
}

DbManufacturingComponent::~DbManufacturingComponent()
{
}

DbManufacturingComponent::DbManufacturingComponent(const DbManufacturingComponent &c)
{
	m_id = c.m_id;
	m_idmp = c.m_idmp;
	m_idc = c.m_idc;
	m_consumed = c.m_consumed;
	m_ide = c.m_ide;
}

const DbManufacturingComponent& DbManufacturingComponent::operator=(const DbManufacturingComponent &c)
{
	m_id = c.m_id;
	m_idmp = c.m_idmp;
	m_idc = c.m_idc;
	m_consumed = c.m_consumed;
	m_ide = c.m_ide;
	return *this;
}

void DbManufacturingComponent::Rx(QDataStream& in)
{
	in >> m_id;
	in >> m_idmp;
	in >> m_idc;
	in >> m_consumed;
	in >> m_ide;
}

void DbManufacturingComponent::Tx(QDataStream& out) const
{
	out << m_id;
	out << m_idmp;
	out << m_idc;
	out << m_consumed;
	out << m_ide;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DbManufacturingComponentArray::Load()
{
	clear();

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	DbManufacturingComponent ob;
	QString sq("SELECT id, idmp, idc, consumed, ide FROM b_ManufacturingComponent;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		ob.m_id = qry.value(0).toInt();
		ob.m_idmp = qry.value(1).toInt();
		ob.m_idc = qry.value(2).toInt();
		ob.m_consumed = qry.value(3).toUInt();
		ob.m_ide = qry.value(4).toInt();
		append(ob);
	}
	if(count() == 0)
	qDebug("DbManufacturingComponentArray Loaded %d records.", count());
}

// Returns true if a commodity is used by a manufacturing process.
bool DbManufacturingComponentArray::IsUsed(quint16 mpid, COMMODITYID cid) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_idmp == mpid && (*it).m_idc == cid)
			return true;
	return false;
}

bool DbManufacturingComponentArray::HasNecessaryPatents(quint16 mpid, const AuKnownPatents& patents) const
{
	// Check to see if a patent is a required component.
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).m_idmp == mpid &&
				(*it).IsRequired() &&
				DbCommodity::IsPatent((*it).CommodityId()) &&
				!patents.HasPatent((*it).CommodityId(), 1))
			return false;	// Required patent is missing.
	return true;
}

quint8 DbManufacturingComponentArray::PatentQ(quint16 mpid, const AuKnownPatents& patents) const
{
	quint8 lowestRequiredPatentQ = 255;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).m_idmp == mpid && (*it).IsRequired() && DbCommodity::IsPatent((*it).CommodityId()))
		{
			quint8 patentQ = patents.PatentQuality((*it).CommodityId());
			if(!patentQ)
				return 0;
			if(lowestRequiredPatentQ > patentQ)
				lowestRequiredPatentQ = patentQ;
		}
	return lowestRequiredPatentQ;
}

bool DbManufacturingComponentArray::RequiresCommodity(quint16 mpid, COMMODITYID cid) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).m_idmp == mpid &&
				(*it).IsRequired() &&
				(*it).CommodityId() == cid)
			return true;	// Required patent is missing.
	return false;
}
