//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbmanufacturingprocess.h,v 1.24 2022/02/19 18:19:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DbManufacturingProcess_H
#define DbManufacturingProcess_H

#include "audata.h"
#include "dbindustry.h"
#include <QString>
#include <QVector>

#define LABOR_RATE			1.		// Cronodollar cost per unit of labor.

class DbManufacturingComponentArray;
class DgData;

class AUDATA_DLL DbManufacturingProcess
{
public:
	quint16			m_id;
	QString			m_name;
	quint16			m_idc;			// Commodity produced.
	quint8			m_idi;			// Industry that can invoke this process.
	quint16			m_produces;		// Quantity produced
	quint32			m_labor;		// Labor required
	bool			m_nat;			// 1 to average in natural commodity of result, e.g. hydrogen from atmosphere
	quint8			m_craft;		// handcraft: 0=no, 1=yes, 2=ok if in the proper industry
	bool			m_isDefault;	// true if this is the default process for an industry

public:
	DbManufacturingProcess();
	~DbManufacturingProcess();
	DbManufacturingProcess(const DbManufacturingProcess &c);
	DbManufacturingProcess& operator=(const DbManufacturingProcess &c);

	inline bool operator==(const DbManufacturingProcess& c) const
	{
		return m_id == c.m_id;
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_idc;
	}

	float Cost(const DgData& gd) const;	// Calculate unit cost by using this process.

	inline quint16 Id() const
	{
		return m_id;
	}

	inline INDUSTRYID IndustryId() const
	{
		return (INDUSTRYID)m_idi;
	}

	inline bool IsRnD() const
	{
		return DbCommodity::IsPatent(CommodityId());
	}

	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;
};

class AUDATA_DLL DbManufacturingProcessArray : public QVector<DbManufacturingProcess>
{
public:
	float Cost(int idCommodity, const DgData& gd) const;	// Calculate cost of manufacturing a commodity. -1 if not possible.

	// Returns true if 'industry' has any manufacturing process to make 'cid'.
	bool DoesIndustryMake(INDUSTRYID industry, COMMODITYID cid) const;

	// Returns true if 'industry' has any manufacturing process to make 'cid'.
	bool DoesIndustryUse(INDUSTRYID industry, COMMODITYID cid, const DbManufacturingComponentArray& manufacturingComponents) const;

	// Find a manufacturing process using its id.
	// Returns the index if the process is found, otherwise returns -1.
	int IndexOf(int mpid) const;

	void Load();
};

#endif // DbManufacturingProcess_H
