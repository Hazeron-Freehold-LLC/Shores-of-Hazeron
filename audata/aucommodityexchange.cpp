//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommodityexchange.cpp,v 1.17 2022/02/19 18:20:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommodityexchange.h"
//
#include "dgdata.h"

EXPORT_POLICY AuCommoditySupply::ExportPolicy(COMMODITYID cid) const
{
	// Money is not given away to any other empire.
	if(cid == CID_Money)
		return EXPOL_ToNobody;
	const DgData& gd = g_Au.LockDataForRead();
	EXPORT_POLICY exPol = gd.EmpireCommerce(EmpireId(), cid);
	g_Au.UnlockData();
	return exPol;
}

quint64 AuCommoditySupply::MemoryUsage() const
{
	quint64 mem = sizeof(AuCommoditySupply);
	return mem;
}

void AuCommoditySupply::operator>>(QDataStream& out) const
{
	out << m_EmpireId;
}

void AuCommoditySupply::operator<<(QDataStream& in)
{
	in >> m_EmpireId;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommoditySuppliers::AuCommoditySuppliers()
{
}

AuCommoditySuppliers::AuCommoditySuppliers(const AuId& bdId, quint32 empireId)
{
	m_Suppliers.insert(bdId, AuCommoditySupply(empireId));
}

AuCommoditySuppliers::AuCommoditySuppliers(const AuCommoditySuppliers& c) : m_Suppliers(c.m_Suppliers)
{
}

const AuCommoditySuppliers& AuCommoditySuppliers::operator=(const AuCommoditySuppliers& c)
{
	m_Suppliers = c.m_Suppliers;
	return *this;
}

void AuCommoditySuppliers::AddSupplier(const AuId& bdId, quint32 empireId)
{
	QHash<AuId, AuCommoditySupply>::iterator si = m_Suppliers.find(bdId);
	if(si != m_Suppliers.end())
		*si = AuCommoditySupply(empireId);
	else
		m_Suppliers.insert(bdId, AuCommoditySupply(empireId));
}

void AuCommoditySuppliers::RemoveSupplier(const AuId& bdId)
{
	QHash<AuId, AuCommoditySupply>::iterator si = m_Suppliers.find(bdId);
	if(si != m_Suppliers.end())
		m_Suppliers.erase(si);
}

void AuCommoditySuppliers::ListSuppliersEmpire(quint32 importEmpireId, AuIdList& suppliers) const
{
	for(QHash<AuId, AuCommoditySupply>::const_iterator si = m_Suppliers.cbegin(); si != m_Suppliers.cend(); si++)
	{
		if((*si).EmpireId() != importEmpireId)
			continue;
		suppliers.append(si.key());
	}
}

void AuCommoditySuppliers::ListSuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers) const
{
	for(QHash<AuId, AuCommoditySupply>::const_iterator si = m_Suppliers.cbegin(); si != m_Suppliers.cend(); si++)
	{
		// Check supplier's export policy toward the importing empire.
		POLITICAL_STANCE stanceToImporter = PoliticalStance((*si).EmpireId(), importEmpireId);
		switch(stanceToImporter)
		{
		case POL_Empire:
		case POL_Vassal:
			break;
		case POL_Friend:
			if((*si).ExportPolicy(cid) == EXPOL_ToNobody)
				continue;				// Commodity cannot be exported to anybody, only to my own empire.
			break;
		case POL_Neutral:
			{
				EXPORT_POLICY exportPolicy = (*si).ExportPolicy(cid);
				if(exportPolicy == EXPOL_ToNobody || exportPolicy == EXPOL_ToFriend)
					continue;				// Commodity cannot be exported to neutrals, only to friends and my own empire.
			}
			break;
		case POL_Enemy:
			if((*si).ExportPolicy(cid) != EXPOL_ToEnemy)
				continue;				// Commodity cannot be exported to enemies.
			break;
		}

		// Supplier's export policy is ok.
		suppliers.append(si.key());
	}
}

bool AuCommoditySuppliers::IsConnected(const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<AuId, AuCommoditySupply>::const_iterator si = m_Suppliers.cbegin(); si != m_Suppliers.cend(); si++)
		if(tradeConnections.contains(si.key()))
			return true;
	return false;
}

quint64 AuCommoditySuppliers::MemoryUsage() const
{
	quint64 mem = sizeof(AuCommoditySuppliers);
	for(QHash<AuId, AuCommoditySupply>::const_iterator si = m_Suppliers.cbegin(); si != m_Suppliers.cend(); si++)
		mem += sizeof(AuId) + (*si).MemoryUsage();
	return mem;
}

void AuCommoditySuppliers::operator>>(QDataStream& out) const
{
	out << m_Suppliers;
}

void AuCommoditySuppliers::operator<<(QDataStream& in)
{
	in >> m_Suppliers;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommodityExchange::AuCommodityExchange()
{
}

AuCommodityExchange::AuCommodityExchange(const AuCommodityExchange& c) : m_Items(c.m_Items)
{
}

const AuCommodityExchange& AuCommodityExchange::operator=(const AuCommodityExchange& c)
{
	m_Items = c.m_Items;
	return *this;
}

void AuCommodityExchange::AddSupplier(COMMODITYID cid, const AuId& bdId, quint32 empireId)
{
	QHash<COMMODITYID, AuCommoditySuppliers>::iterator ei = m_Items.find(cid);
	if(ei != m_Items.end())
		(*ei).AddSupplier(bdId, empireId);
	else
		m_Items.insert(cid, AuCommoditySuppliers(bdId, empireId));
}

void AuCommodityExchange::RemoveSupplier(COMMODITYID cid, const AuId& bdId)
{
	QHash<COMMODITYID, AuCommoditySuppliers>::iterator ei = m_Items.find(cid);
	if(ei != m_Items.end())
		(*ei).RemoveSupplier(bdId);
}

void AuCommodityExchange::ListSuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers) const
{
	QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ei = m_Items.find(cid);
	if(ei != m_Items.end())
		(*ei).ListSuppliers(cid, importEmpireId, suppliers);
}

void AuCommodityExchange::CantinaSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsCantinaItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::MilitaryAirBaseSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsMilitaryAirBaseItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::MilitaryDockSupplies(QList<COMMODITYID>& cidsAvailable,
		const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsMilitaryDockItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::MilitaryMotorPoolSupplies(QList<COMMODITYID>& cidsAvailable,
		const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsMilitaryMotorPoolItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::MilitaryHQSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsMilitaryHQItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::GrocerySupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsGroceryItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::HospitalSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsHospitalItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::RetailSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(g_Au.Commodity(ii.key()).IsRetailItem() && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

void AuCommodityExchange::FindSupplies(QList<COMMODITYID>& cidsAvailable, const QList<COMMODITYID>& cidsSought,
		const QHash<AuId, int>& tradeConnections) const
{
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		if(cidsSought.contains(ii.key()) && (*ii).IsConnected(tradeConnections))
			cidsAvailable.append(ii.key());
}

quint64 AuCommodityExchange::MemoryUsage() const
{
	quint64 mem = sizeof(AuCommodityExchange);
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		mem += sizeof(COMMODITYID) + (*ii).MemoryUsage();
	return mem;
}

void AuCommodityExchange::operator>>(QDataStream& out) const
{
	out << (quint16)m_Items.size();
	for(QHash<COMMODITYID, AuCommoditySuppliers>::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
	{
		out << (quint16)ii.key();
		out << *ii;
	}
}

void AuCommodityExchange::operator<<(QDataStream& in)
{
	m_Items.clear();
	quint16 cnt;
	in >> cnt;
	while(cnt--)
	{
		quint16 cid;
		AuCommoditySuppliers s;
		in >> cid;
		in >> s;
		m_Items.insert((COMMODITYID)cid, s);
	}
}

