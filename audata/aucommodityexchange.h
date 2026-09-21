//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommodityexchange.h,v 1.16 2022/02/19 18:18:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommodityExchange_H
#define AuCommodityExchange_H

#include "audata.h"
#include "auid.h"
#include "dbcommodity.h"

class AUDATA_DLL AuCommoditySupply
{
private:	// Properties
	quint32								m_EmpireId;				// Empire id of supplier.

public:	// Constructors
	inline AuCommoditySupply(quint32 empireId)
	{
		m_EmpireId = empireId;
	}

	inline AuCommoditySupply()
	{
		m_EmpireId = 0;
	}

	inline AuCommoditySupply(const AuCommoditySupply& c)
	{
		m_EmpireId = c.m_EmpireId;
	}

public:	// Operators
	inline const AuCommoditySupply& operator=(const AuCommoditySupply& c)
	{
		m_EmpireId = c.m_EmpireId;
		return *this;
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	inline quint32 EmpireId() const
	{
		return m_EmpireId;
	}

	quint64 MemoryUsage() const;

	EXPORT_POLICY ExportPolicy(COMMODITYID cid) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCommoditySupply& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCommoditySupply& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommoditySuppliers
{
private:	// Properties
	QHash<AuId, AuCommoditySupply>		m_Suppliers;			// Key is building id.

public:	// Constructors
	AuCommoditySuppliers();
	AuCommoditySuppliers(const AuId& bdId, quint32 empireId);
	AuCommoditySuppliers(const AuCommoditySuppliers& c);

public:	// Operators
	const AuCommoditySuppliers& operator=(const AuCommoditySuppliers& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddSupplier(const AuId& bdId, quint32 empireId);

	bool IsConnected(const QHash<AuId, int>& tradeConnections) const;

	// Lists suppliers whose export policy permits export to 'importEmpireId'.
	void ListSuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers) const;

	// Lists only suppliers that are in the same empire is 'importEmpireId'.
	void ListSuppliersEmpire(quint32 importEmpireId, AuIdList& suppliers) const;

	quint64 MemoryUsage() const;

	void RemoveSupplier(const AuId& bdId);
};

inline QDataStream& operator<<(QDataStream& out, const AuCommoditySuppliers& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCommoditySuppliers& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommodityExchange
{
private:	// Properties
	QHash<COMMODITYID, AuCommoditySuppliers>					m_Items;

public:	// Constructors
	AuCommodityExchange();
	AuCommodityExchange(const AuCommodityExchange& c);

public:	// Operators
	const AuCommodityExchange& operator=(const AuCommodityExchange& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddSupplier(COMMODITYID cid, const AuId& bdId, quint32 empireId);

	void CantinaSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void FindSupplies(QList<COMMODITYID>& cidsAvailable, const QList<COMMODITYID>& cidsSought,
			const QHash<AuId, int>& tradeConnections) const;

	void GrocerySupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void HospitalSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	// Lists 'suppliers' of 'cid' whose export policy permits export to 'importEmpireId'.
	void ListSuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers) const;

	quint64 MemoryUsage() const;

	void MilitaryAirBaseSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void MilitaryDockSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void MilitaryHQSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void MilitaryMotorPoolSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void RemoveSupplier(COMMODITYID cid, const AuId& bdId);

	void RetailSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCommodityExchange& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCommodityExchange& o)
{
	o << in;
	return in;
}

#endif // AuCommodityExchange_H
