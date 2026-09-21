//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbmanufacturingprocess.cpp,v 1.41 2022/02/19 18:20:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dbmanufacturingprocess.h"
//
#include "deprocesscomponent.h"
#include "dgdata.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include <QVariant>

#define c_ManufMult		10		// Multiplication factor for DbManufacturingProcess amounts, for bulk manufacturing. The db
								// amounts reflect one-off quantities as needed for hand-crafting.

DbManufacturingProcess::DbManufacturingProcess()
{
	m_craft = 0;
	m_id = 0;
	m_idc = 0;
	m_idi = 0;
	m_isDefault = false;
	m_labor = 0;	// Labor required
	m_nat = 1;
	m_produces = 0;	// Quantity produced
}

DbManufacturingProcess::~DbManufacturingProcess()
{
}

DbManufacturingProcess::DbManufacturingProcess(const DbManufacturingProcess &c)
{
	m_craft = c.m_craft;
	m_id = c.m_id;
	m_idc = c.m_idc;
	m_idi = c.m_idi;
	m_isDefault = c.m_isDefault;
	m_labor = c.m_labor;
	m_name = c.m_name;
	m_nat = c.m_nat;
	m_produces = c.m_produces;
}

DbManufacturingProcess& DbManufacturingProcess::operator=(const DbManufacturingProcess &c)
{
	m_craft = c.m_craft;
	m_id = c.m_id;
	m_idc = c.m_idc;
	m_idi = c.m_idi;
	m_isDefault = c.m_isDefault;
	m_labor = c.m_labor;
	m_name = c.m_name;
	m_nat = c.m_nat;
	m_produces = c.m_produces;
	return *this;
}

// Calculate cost of executing this process if possible.
float DbManufacturingProcess::Cost(const DgData& gd) const
{
	bool doMultiple = false;
	if(CommodityId() == CID_Electricity || CommodityId() == CID_Money ||
			((g_Au.Commodity(CommodityId()).m_combits & CB_Cargo) &&
			 !DbCommodity::IsPatent(CommodityId())))
		doMultiple = true;

	//if(CommodityId() == CID_Money || CommodityId() == CID_Gold || CommodityId() == CID_GoldOre)
		//qDebug() << "\nDbManufacturingProcess::Cost" << m_id << m_name << m_idc << g_Au.CommodityName((COMMODITYID)m_idc);

	float totalcost = 0;	// Total cost of all components.
	DbManufacturingComponentArray::const_iterator it = gd.m_bManufacturingComponentArray.constBegin();
	while(it != gd.m_bManufacturingComponentArray.constEnd())
	{
		// Only tally components that are consumed by this process.
		if((*it).m_idmp == m_id && (*it).IsRequired() && (*it).m_consumed)
		{
			if((*it).m_idc == CID_Money)
			{
				// Money just costs the money, no labor or quality modifiers are applied.
				totalcost += (*it).m_consumed;
				//if(CommodityId() == CID_Gold)
					//qDebug() << "Gold " << (*it).m_idc << g_Au.CommodityName((COMMODITYID)(*it).m_idc)
							//<< "consumed" << (*it).m_consumed << "totalcost" << totalcost;
			}
			else
			{
				float unitcost = g_Au.CommodityQualityCost((COMMODITYID)(*it).m_idc, QualityBaseline());
				if(unitcost < 0)
					return -1;	// The base cost of a component is not known yet.
				quint32 con = (*it).m_consumed;
				if(doMultiple)
					con *= c_ManufMult;
				float laborcost = DeProcessComponent::GetLaborNeeded(con) * LABOR_RATE;
				float cost = unitcost * con + laborcost;
				totalcost += cost;
				//if(CommodityId() == CID_Money || CommodityId() == CID_Gold || CommodityId() == CID_GoldOre)
					//qDebug() << " " << (*it).m_idc << g_Au.CommodityName((COMMODITYID)(*it).m_idc) << "unitcost" << unitcost
							//<< "consumed" << con << "laborcost" << laborcost << "cost" << cost << "totalcost" << totalcost;
			}
		}
		it++;
	}

	quint32 lab = m_labor;
	quint32 prod = m_produces;
	if(doMultiple)
	{
		prod *= c_ManufMult;
		lab *= c_ManufMult;
	}

	// Add overhead labor cost.
	totalcost += lab * LABOR_RATE;
	if(prod <= 1)
	{
		//if(CommodityId() == CID_Money || CommodityId() == CID_Gold || CommodityId() == CID_GoldOre)
			//qDebug() << "DbManufacturingProcess::Cost totalcost" << totalcost;
		return totalcost;
	}

	// Calculate unit cost based on the number produced by the process.
	float unitcost = totalcost / prod;
	//if(CommodityId() == CID_Money || CommodityId() == CID_Gold || CommodityId() == CID_GoldOre)
		//qDebug() << "DbManufacturingProcess::Cost totalcost" << totalcost << "produces" << prod << "unitcost" << unitcost;
	if(unitcost < 1)
		unitcost = 1;
	return unitcost;
}

void DbManufacturingProcess::Tx(QDataStream& out) const
{
	quint8 bits = 0;
	if(m_isDefault)
		bits |= 0x01;
	if(m_nat)
		bits |= 0x04;

	out << bits;
	out << m_id;
	out << m_name;
	out << m_idc;
	out << m_idi;
	out << m_produces;
	out << m_labor;
	out << m_craft;
}

void DbManufacturingProcess::Rx(QDataStream& in)
{
	quint8 bits;
	in >> bits;
	m_isDefault = bits & 0x01;
	m_nat = bits & 0x04;

	in >> m_id;
	in >> m_name;
	in >> m_idc;
	in >> m_idi;
	in >> m_produces;
	in >> m_labor;
	in >> m_craft;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void DbManufacturingProcessArray::Load()
{
	clear();

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	// Select the manufacturing processes so they are alphabetized by name.
	DbManufacturingProcess ob;
	QString sq("SELECT id, name, idc, idi, qty, labor, nat, craft, isdefault FROM b_ManufacturingProcess "
			"ORDER BY name ASC;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		ob.m_id = qry.value(qv++).toUInt();
		ob.m_name = qry.value(qv++).toString();
		ob.m_idc = qry.value(qv++).toUInt();
		ob.m_idi = qry.value(qv++).toUInt();
		ob.m_produces = qry.value(qv++).toUInt();
		ob.m_labor = qry.value(qv++).toUInt();
		ob.m_nat = qry.value(qv++).toBool();
		ob.m_craft = qry.value(qv++).toUInt();
		ob.m_isDefault = qry.value(qv++).toBool();
		append(ob);
	}
	if(count() == 0)
		qWarning("DbManufacturingProcessArray Loaded %d records.", count());
}

// Returns the cost of the cheapest process that produces idCommodity.
// returns -1 if not possible
// returns 1 if no process exists
float DbManufacturingProcessArray::Cost(int idCommodity, const DgData& gd) const
{
	bool anyprocess = false;
	float bestcost = -1;
	for(const_iterator it = constBegin(); it != constEnd(); ++it)
	{
		if((*it).m_idc == idCommodity)
		{
			anyprocess = true;
			float cost = (*it).Cost(gd);
			if(cost >= 0)
			{
				if(bestcost < 0 || cost < bestcost)
					bestcost = cost;
			}
		}
	}
	if(!anyprocess)
	{
		qDebug() << "DbManufacturingProcessArray::Cost no process to make idCommodity" << idCommodity <<
				g_Au.CommodityName((COMMODITYID)idCommodity);
		return 1;	// No process exists to manufacture the commodity.
	}
	return bestcost;
}

// Find a manufacturing process using its id.
int DbManufacturingProcessArray::IndexOf(int mpid) const
{
	int procIdx = 0;
	for(const_iterator it = constBegin(); it != constEnd(); it++, procIdx++)
		if((*it).m_id == mpid)
			return procIdx;
	return -1;
}

bool DbManufacturingProcessArray::DoesIndustryMake(INDUSTRYID industry, COMMODITYID cid) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_idi == industry && ((*it).m_idc == cid))
			return true;
	return false;
}

bool DbManufacturingProcessArray::DoesIndustryUse(INDUSTRYID industry, COMMODITYID cid,
		const DbManufacturingComponentArray& manufacturingComponents) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_idi == industry)
		{
			// Found a manufacturing process for the industry. Does it use the commodity?
			if(manufacturingComponents.IsUsed((*it).m_id, cid))
				return true;	// The component is used by the manufacturing process.
		}
	return false;
}
