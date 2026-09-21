//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipment.cpp,v 1.42 2022/02/19 18:20:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aushipment.h"
//
#include "aueventlog.h"
#include "auitem.h"
#include "dgdata.h"

static int s_MT_AuShipment = AuShipment::RegisterMetaType();
static int s_MT_AuTradeAgreement = AuTradeAgreement::RegisterMetaType();

//static
int AuShipment::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuShipment>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuShipment>();

	return metaType;
}

AuShipment::AuShipment(COMMODITYID cid, quint32 amount, quint8 minQ, quint8 repeat,
		const AuId& destCityId, const QString& destDesc, float destDistPc, const AuId& destSysId, const AuId& destSectorId,
		quint32 empireOfOrigin)
{
	Q_ASSERT(minQ);
	m_Amount = amount;
	m_CID = cid;
	m_CommodityFilter = FILTER_All;
	m_Container = new AuItemTEUContainer(destCityId, destDesc, destDistPc, destSysId, destSectorId, empireOfOrigin);
	m_MinQ = minQ;
	m_Repeat = repeat;
	m_State = Filling;
	m_Trade = 0;
}

AuShipment::AuShipment(COMMODITYFILTER filter, quint32 amount, quint8 minQ, quint8 repeat,
		const AuId& destCityId, const QString& destDesc, float destDistPc, const AuId& destSysId, const AuId& destSectorId,
		quint32 empireOfOrigin)
{
	Q_ASSERT(minQ);
	m_Amount = amount;
	m_CID = CID_Invalid;
	m_CommodityFilter = filter;
	m_Container = new AuItemTEUContainer(destCityId, destDesc, destDistPc, destSysId, destSectorId, empireOfOrigin);
	m_MinQ = minQ;
	m_Repeat = repeat;
	m_State = Filling;
	m_Trade = 0;
}

AuShipment::AuShipment(const AuId& cityId, quint32 cityEmpire, const AuTradeAgreement& ta)
{
	Q_ASSERT(ta.IsAgreement());
	if(cityId == ta.OriginCity())
	{
		m_Amount = ta.SendAmount();
		m_CID = ta.SendCommodityId();
		m_CommodityFilter = ta.SendCommodityFilter();
		m_Container = new AuItemTEUContainer(ta.DestinationCity(), ta.DestinationDesc(), ta.DistancePc(), ta.DestinationSystem(),
				ta.DestinationSector(), cityEmpire);
		m_MinQ = ta.SendMinQ();
		m_Repeat = 1;
		m_State = Filling;
	}
	else if(cityId == ta.DestinationCity())
	{
		m_Amount = ta.GetAmount();
		m_CID = ta.GetCommodityId();
		m_CommodityFilter = ta.GetCommodityFilter();
		m_Container = new AuItemTEUContainer(ta.OriginCity(), ta.OriginDesc(), ta.DistancePc(), ta.OriginSystem(),
				ta.OriginSector(), cityEmpire);
		m_MinQ = ta.GetMinQ();
		m_Repeat = 1;
		m_State = Filling;
	}
	else
	{
		m_Amount = 0;
		m_CID = CID_Invalid;
		m_CommodityFilter = FILTER_All;
		m_Container = new AuItemTEUContainer();
		m_MinQ = 1;
		m_Repeat = 0;
		m_State = Filling;
	}
	m_Trade = new AuTradeAgreement(ta);
}

AuShipment::AuShipment()
{
	m_Amount = 0;
	m_CID = CID_Invalid;
	m_Container = new AuItemTEUContainer();
	m_CommodityFilter = FILTER_All;
	m_MinQ = 1;
	m_Repeat = 0;
	m_State = Filling;
	m_Trade = 0;
}

AuShipment::~AuShipment()
{
	delete m_Container;
	if(m_Trade)
		delete m_Trade;
}

AuShipment::AuShipment(const AuShipment& c)
{
	m_Amount = c.m_Amount;
	m_CID = c.m_CID;
	m_CommodityFilter = c.m_CommodityFilter;
	m_Container = new AuItemTEUContainer(*c.m_Container);
	m_LastShipped = c.m_LastShipped;
	m_MinQ = c.m_MinQ;
	m_Repeat = c.m_Repeat;
	m_State = c.m_State;
	if(c.m_Trade)
		m_Trade = new AuTradeAgreement(*c.m_Trade);
	else
		m_Trade = 0;
}

const AuShipment& AuShipment::operator=(const AuShipment& c)
{
	m_Amount = c.m_Amount;
	m_CID = c.m_CID;
	m_CommodityFilter = c.m_CommodityFilter;
	*m_Container = *c.m_Container;
	m_LastShipped = c.m_LastShipped;
	m_MinQ = c.m_MinQ;
	m_Repeat = c.m_Repeat;
	m_State = c.m_State;
	if(c.m_Trade)
	{
		if(!m_Trade)
			m_Trade = new AuTradeAgreement(*c.m_Trade);
		else
			*m_Trade = *c.m_Trade;
	}
	else if(m_Trade)
	{
		delete m_Trade;
		m_Trade = 0;
	}
	return *this;
}

void AuShipment::operator>>(QDataStream& out) const
{
	quint8 hasWhat = m_State;

	if(m_MinQ > 1)
		hasWhat |= 0x80;
	if(m_CommodityFilter != FILTER_All)
		hasWhat |= 0x40;
	if(m_Repeat)
		hasWhat |= 0x20;
	if(!m_LastShipped.IsNull())
		hasWhat |= 0x10;
	if(m_Trade)
		hasWhat |= 0x08;

	out << hasWhat;
	out << m_Amount;
	out << (quint16)m_CID;
	out << *m_Container;

	if(hasWhat & 0x80)
		out << m_MinQ;

	if(hasWhat & 0x40)
		out << (quint8)m_CommodityFilter;

	if(hasWhat & 0x20)
		out << m_Repeat;

	if(hasWhat & 0x10)
		out << m_LastShipped;

	if(hasWhat & 0x08)
		out << *m_Trade;
}

void AuShipment::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	m_State = (STATE)(hasWhat & 0x03);

	in >> m_Amount;
	quint16 ui16;
	in >> ui16;
	m_CID = (COMMODITYID)ui16;
	in >> *m_Container;

	if(hasWhat & 0x80)
		in >> m_MinQ;
	else
		m_MinQ = 1;

	if(hasWhat & 0x40)
	{
		quint8 ui8;
		in >> ui8;
		m_CommodityFilter = (COMMODITYFILTER)ui8;
	}
	else
		m_CommodityFilter = FILTER_All;

	if(hasWhat & 0x20)
		in >> m_Repeat;
	else
		m_Repeat = 0;

	if(hasWhat & 0x10)
		in >> m_LastShipped;
	else
		m_LastShipped = AuTime();

	if(m_Trade)
		delete m_Trade;
	if(hasWhat & 0x08)
	{
		m_Trade = new AuTradeAgreement;
		in >> *m_Trade;
	}
	else
		m_Trade = 0;
}

bool AuShipment::operator==(const AuShipment& c) const
{
	if(m_CID != c.m_CID)
		return false;
	if(m_CommodityFilter != c.m_CommodityFilter)
		return false;
	if(m_State != c.m_State)
		return false;
	if(m_Amount != c.m_Amount)
		return false;
	if(m_MinQ != c.m_MinQ)
		return false;
	if(m_Repeat != c.m_Repeat)
		return false;
	if(m_LastShipped != c.m_LastShipped)
		return false;
	if(*m_Container != *c.m_Container)
		return false;
	//m_Trade
	return true;
}

AuPoint AuShipment::DestSysLoc() const
{
	const DgData& gd = g_Au.LockDataForRead();
	AuPoint destSysLoc = gd.CitySystemLoc(m_Container->DestinationCity());
	g_Au.UnlockData();
	return destSysLoc;
}

bool AuShipment::IsReadyToDeliver() const
{
	if(!IsEnroute())
		return false;
	return m_LastShipped.ElapsedSec() >= m_Container->StandardDeliveryTime();
}

bool AuShipment::Fill(AuItemHash& source)
{
	if(m_State != Filling)
		return false;

	if(m_CID == CID_Invalid)
	{
		// Fill with a category of commodities.
		QList<COMMODITYID> cidFilter;
		g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, CommodityFilter());
		if(!cidFilter.empty())
		{
			// Count what we have accumulated so far.
			quint32 rawCount = 0;
			for(QList<COMMODITYID>::const_iterator ci = cidFilter.cbegin(); ci != cidFilter.cend(); ci++)
				rawCount += m_Container->Cargo()->RawCount(*ci);

			// Try to get the same amount of each kind. Eventually we will be full of something. :P
			quint32 amtper = m_Amount / cidFilter.size();
			if(!amtper)
				amtper = 1;
			for(QList<COMMODITYID>::const_iterator ci = cidFilter.cbegin(); ci != cidFilter.cend(); ci++)
			{
				AuItemHash::iterator si = source.FindBest(*ci);
				if(si == source.end())
					continue;	// Source has none.
				if((*si).second->Quality() < m_MinQ)
					continue;	// Best source Q is too low.

				quint32 curCount = m_Container->Cargo()->RawCount(*ci);
				if(curCount >= m_Amount)
					continue;	// We got full of that one kind.

				// Take as much of the commodity as we can.
				quint32 amountNeeded = m_Amount - curCount;
				if(amountNeeded > amtper)
					amountNeeded = amtper;
				if(amountNeeded + rawCount > m_Amount)
					amountNeeded = m_Amount - rawCount;	// Don't exceed raw m_Amount.
				if(!amountNeeded)
					continue;
				if((*si).second->Count() <= amountNeeded)
				{
					// Take the whole item from the source.
					rawCount += (*si).second->Count();
					m_Container->Cargo()->AddItem(*(*si).second);
					source.Erase(si);
				}
				else
				{
					// Take some of the item from the source.
					rawCount += amountNeeded;
					m_Container->Cargo()->AddItemCount(*(*si).second, amountNeeded);
					(*si).second->Consume(amountNeeded);
				}
				if(rawCount >= m_Amount)
					break;
			}
			if(rawCount >= m_Amount)
				m_State = AwaitingShipment;
		}
	}
	else
	{
		// Fill with a specific commodity.
		AuItemHash::iterator si = source.FindBest(CommodityId());
		if(si == source.end())
			return false;	// Source has none.
		if((*si).second->Quality() < m_MinQ)
			return false;	// Best source Q is too low.

		quint32 curCount = m_Container->Cargo()->RawCount(CommodityId());
		if(curCount < m_Amount)
		{
			// Take as much of the commodity as we can.
			quint32 amountNeeded = m_Amount - curCount;
			if((*si).second->Count() <= amountNeeded)
			{
				// Take the whole item from the source.
				m_Container->Cargo()->AddItem(*(*si).second);
				source.Erase(si);
			}
			else
			{
				// Take some of the item from the source.
				m_Container->Cargo()->AddItemCount(*(*si).second, amountNeeded);
				(*si).second->Consume(amountNeeded);
			}
		}
		if(m_Container->Cargo()->RawCount(CommodityId()) >= m_Amount)
			m_State = AwaitingShipment;
	}

	//qDebug() << "AuShipment::Fill" << m_Container->Cargo()->size() << m_Container->Cargo()->RawCount(CommodityId());
	return true;
}

bool AuShipment::Fill(const AuItemHash& source, AuItemHash& taken)
{
	if(m_State != Filling)
		return false;

	if(m_CID == CID_Invalid)
	{
		// Fill with a category of commodities.
		QList<COMMODITYID> cidFilter;
		g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, CommodityFilter());
		if(!cidFilter.empty())
		{
			// Count what we have accumulated so far.
			quint32 rawCount = 0;
			for(QList<COMMODITYID>::const_iterator ci = cidFilter.cbegin(); ci != cidFilter.cend(); ci++)
				rawCount += m_Container->Cargo()->RawCount(*ci);

			// Try to get the same amount of each kind. Eventually we will be full of something. :P
			quint32 amtper = m_Amount / cidFilter.size();
			if(!amtper)
				amtper = 1;
			for(QList<COMMODITYID>::const_iterator ci = cidFilter.cbegin(); ci != cidFilter.cend(); ci++)
			{
				AuItemHash::const_iterator si = source.FindBest(*ci);
				if(si == source.cend())
					continue;	// Source has none.
				if((*si).second->Quality() < m_MinQ)
					continue;	// Best source Q is too low.

				quint32 curCount = m_Container->Cargo()->RawCount(*ci);
				if(curCount >= m_Amount)
					continue;	// We got full of that one kind.

				// Take as much of the commodity as we can.
				quint32 amountNeeded = m_Amount - curCount;
				if(amountNeeded > amtper)
					amountNeeded = amtper;
				if(amountNeeded + rawCount > m_Amount)
					amountNeeded = m_Amount - rawCount;	// Don't exceed raw m_Amount.
				if(!amountNeeded)
					continue;
				if((*si).second->Count() <= amountNeeded)
				{
					// Take the whole item from the source.
					rawCount += (*si).second->Count();
					m_Container->Cargo()->AddItem(*(*si).second);
					taken.AddItem(*(*si).second);
				}
				else
				{
					// Take some of the item from the source.
					rawCount += amountNeeded;
					m_Container->Cargo()->AddItemCount(*(*si).second, amountNeeded);
					taken.AddItemCount(*(*si).second, amountNeeded);
				}
				if(rawCount >= m_Amount)
					break;
			}
			if(rawCount >= m_Amount)
				m_State = AwaitingShipment;
		}
	}
	else
	{
		// Fill with a specific commodity.
		AuItemHash::const_iterator si = source.FindBest(CommodityId());
		if(si == source.cend())
			return false;	// Source has none.
		if((*si).second->Quality() < m_MinQ)
			return false;	// Best source Q is too low.

		quint32 curCount = m_Container->Cargo()->RawCount(CommodityId());
		if(curCount < m_Amount)
		{
			// Take as much of the commodity as we can.
			quint32 amountNeeded = m_Amount - curCount;
			if((*si).second->Count() <= amountNeeded)
			{
				// Take the whole item from the source.
				m_Container->Cargo()->AddItem(*(*si).second);
				taken.AddItem(*(*si).second);
			}
			else
			{
				// Take some of the item from the source.
				m_Container->Cargo()->AddItemCount(*(*si).second, amountNeeded);
				taken.AddItemCount(*(*si).second, amountNeeded);
			}
		}
		if(m_Container->Cargo()->RawCount(CommodityId()) >= m_Amount)
			m_State = AwaitingShipment;
	}

	//qDebug() << "AuShipment::Fill" << m_Container->Cargo()->size() << m_Container->Cargo()->RawCount(CommodityId());
	return true;
}

void AuShipment::SetShipped()
{
	//qDebug() << "AuShipment::SetShipped" << m_Container->Cargo()->size();
	m_Repeat = 0;
	m_State = Enroute;
	m_LastShipped.Set();
}

bool AuShipment::ResetAfterShipped()
{
	m_Container->Cargo()->Clear();

	// Go back to filling.
	m_State = Filling;

	// If repeat is already at zero, this is an endless repeater.
	if(!m_Repeat)
		return false;

	// Reduce the repeat count.
	m_Repeat--;

	// Caller must remove this shipment on true return or we'll be doing this forever.
	return m_Repeat == 0;
}

QString AuShipment::ListName() const
{
	QString cname;
	if(CommodityId() != CID_Invalid)
		cname = g_Au.CommodityName(CommodityId());
	else
		cname = CommodityFilterName(CommodityFilter());
	quint32 teus = m_Container->TEUCount();
	if(teus == 1)
		return QObject::tr("%1, 1 TEU")
			.arg(cname);
	else
		return QObject::tr("%1, %L2 TEUs")
			.arg(cname)
			.arg(teus);
}

QString AuShipment::ListDesc() const
{
	QString cname;
	if(CommodityId() != CID_Invalid)
		cname = g_Au.CommodityName(CommodityId());
	else
		cname = CommodityFilterName(CommodityFilter());
	if(m_MinQ > 1)
		cname += QObject::tr(", Minimum Q%1").arg(m_MinQ);
	QString ta;
	if(TradeAgreement())
		ta = QObject::tr("\n under trade agreement");
	if(m_State == Filling)
	{
		quint32 totalUnits = m_Container->Cargo()->TotalCargoVolume();
		if(m_Repeat)
			return QObject::tr("%1\n %L2 of %L3 units %4 times\n %5%6.")
				.arg(cname)
				.arg(totalUnits)
				.arg(m_Amount)
				.arg(m_Repeat)
				.arg(m_Container->DestinationDesc())
				.arg(ta);
		else
			return QObject::tr("%1\n %L2 of %L3 units Continuous\n %4%5.")
				.arg(cname)
				.arg(totalUnits)
				.arg(m_Amount)
				.arg(m_Container->DestinationDesc())
				.arg(ta);
	}
	else if(m_State == AwaitingShipment)
	{
		if(!m_Repeat)
			return QObject::tr("%1\n %L2 units Awaiting Shipment, Continuous\n %3%4.")
				.arg(cname)
				.arg(m_Amount)
				.arg(m_Container->DestinationDesc())
				.arg(ta);
		else
			return QObject::tr("%1\n %L2 units Awaiting Shipment\n %3%4.")
				.arg(cname)
				.arg(m_Amount)
				.arg(m_Container->DestinationDesc())
				.arg(ta);
	}
	else// if(m_State == Enroute)
	{
		// Enroute doesn't make sense to mention m_Repeat.
		return QObject::tr("%1\n %L2 units En Route\n %3%4.")
			.arg(cname)
			.arg(m_Amount)
			.arg(m_Container->DestinationDesc())
			.arg(ta);
	}
}

QString AuShipment::HtmlToolTip() const
{
	QString enroute;
	{
		quint32 delMin = m_Container->StandardDeliveryTime() / 60;
		quint32 delDay = delMin / (60*24);
		delMin -= delDay * (60*24);
		quint32 delHr = delMin / 60;
		delMin -= delHr * 60;
		if(delDay)
			enroute = QString("%L1 days, %2 hours, %3 minutes").arg(delDay).arg(delHr).arg(delMin);
		else if(delHr)
			enroute = QString("%L1 hours, %2 minutes").arg(delHr).arg(delMin);
		else
			enroute = QString("%L1 minutes").arg(delMin);
	}

	QString cname;
	if(CommodityId() != CID_Invalid)
		cname = g_Au.CommodityName(CommodityId());
	else
		cname = CommodityFilterName(CommodityFilter());
	if(m_State == Filling)
	{
		quint32 totalUnits = m_Container->Cargo()->TotalCargoVolume();
		if(m_Repeat)
			return QObject::tr("%1"
					"<br>"
					"Filling: <b>%2 of %3 units</b>"
					"<br>"
					"Minimum Quality: <b>%4</b>"
					"<br>"
					"Destination: <b>%6</b>"
					"<br>"
					"Fill Order: <b>%7 times</b>"
					"<br>"
					"Est. Time En Route: <b>%8</b>"
					"<br>"
					"Delivery Fee: <b>%L9%10</b>"
					"<br>"
					"Trade Agreement: <b>%11</b>")
				.arg(cname)
				.arg(totalUnits)
				.arg(m_Amount)
				.arg(m_MinQ)
				.arg(m_Container->DestinationDesc())
				.arg(m_Repeat)
				.arg(enroute)
				.arg(m_Container->EstimatePayment(m_Amount), 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(m_Trade ? QObject::tr("Yes") : QObject::tr("No"));
		else
			return QObject::tr("%1"
					"<br>"
					"Filling: <b>%2 of %3 units</b>"
					"<br>"
					"Minimum Quality: <b>%4</b>"
					"<br>"
					"Destination: <b>%6</b>"
					"<br>"
					"Fill Order: <b>Continuous</b>"
					"<br>"
					"Est. Time En Route: <b>%7</b>"
					"<br>"
					"Delivery Fee: <b>%L8%9</b>"
					"<br>"
					"Trade Agreement: <b>%10</b>")
				.arg(cname)
				.arg(totalUnits)
				.arg(m_Amount)
				.arg(m_MinQ)
				.arg(m_Container->DestinationDesc())
				.arg(enroute)
				.arg(m_Container->EstimatePayment(m_Amount), 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(m_Trade ? QObject::tr("Yes") : QObject::tr("No"));
	}
	else if(m_State == AwaitingShipment)
	{
		if(m_Repeat)
			return QObject::tr("%1"
					"<br>"
					"Quantity: <b>%2 units</b>"
					"<br>"
					"Minimum Quality: <b>%3</b>"
					"<br>"
					"Awaiting Shipment To: <b>%5</b>"
					"<br>"
					"Fill Order: <b>%6 more times</b>"
					"<br>"
					"Est. Time En Route: <b>%7</b>"
					"<br>"
					"Delivery Fee: <b>%L8%9</b>"
					"<br>"
					"Trade Agreement: <b>%10</b>")
				.arg(cname)
				.arg(m_Amount)
				.arg(m_MinQ)
				.arg(m_Container->DestinationDesc())
				.arg(m_Repeat == 1 ? QObject::tr("No") : QString("%1").arg(m_Repeat-1))
				.arg(enroute)
				.arg(m_Container->StandardPayment(), 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(m_Trade ? QObject::tr("Yes") : QObject::tr("No"));
		else
			return QObject::tr("%1"
					"<br>"
					"Quantity: <b>%2 units</b>"
					"<br>"
					"Minimum Quality: <b>%3</b>"
					"<br>"
					"Awaiting Shipment To: <b>%5</b>"
					"<br>"
					"Fill Order: <b>Continuous</b>"
					"<br>"
					"Est. Time En Route: <b>%6</b>"
					"<br>"
					"Delivery Fee: <b>%L7%8</b>"
					"<br>"
					"Trade Agreement: <b>%9</b>")
				.arg(cname)
				.arg(m_Amount)
				.arg(m_MinQ)
				.arg(m_Container->DestinationDesc())
				.arg(enroute)
				.arg(m_Container->StandardPayment(), 0,'f',0)
				.arg(g_Au.CurrencySymbol())
				.arg(m_Trade ? QObject::tr("Yes") : QObject::tr("No"));
	}
	else// if(m_State == Enroute)
	{
		AuTime estArrival = m_LastShipped.AddSecs(m_Container->StandardDeliveryTime());;
		return QObject::tr("%1"
				"<br>"
				"Quantity: <b>%2 units</b>"
				"<br>"
				"Minimum Quality: <b>%3</b>"
				"<br>"
				"En Route to: <b>%5</b>"
				"<br>"
				"Est. Delivery: <b>%6</b>"
				"<br>"
				"Trade Agreement: <b>%7</b>")
			.arg(cname)
			.arg(m_Amount)
			.arg(m_MinQ)
			.arg(m_Container->DestinationDesc())
			.arg(estArrival.UTC().toLocalTime().toString())
			.arg(m_Trade ? QObject::tr("Yes") : QObject::tr("No"));
	}
}

quint64 AuShipment::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_Amount);
	mem += sizeof(m_CID);
	mem += sizeof(m_Container);
	if(m_Container)
		mem += m_Container->MemoryUsage();
	mem += sizeof(m_LastShipped);
	mem += sizeof(m_MinQ);
	mem += sizeof(m_Repeat);
	mem += sizeof(m_State);
	return mem;
}

void AuShipment::SetTradeAgreement(const AuTradeAgreement& ta)
{
	if(m_Trade)
		delete m_Trade;
	m_Trade = new AuTradeAgreement(ta);
}

bool AuShipment::IsTradeAgreement(const AuTradeAgreement& ta) const
{
	if(!m_Trade)
		return false;
	return *m_Trade == ta;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuShipmentList::AnyFilling() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsFilling())
			return true;
	return false;
}

bool AuShipmentList::AnyReadyToShip() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsAwaitingShipment())
			return true;
	return false;
}

bool AuShipmentList::AnyReadyToDeliver() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsReadyToDeliver())
			return true;
	return false;
}

quint16 AuShipmentList::CountEnroute() const
{
	quint16 c = 0;
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsEnroute())
			c++;
	return c;
}

quint16 AuShipmentList::CountOffices() const
{
	quint16 c = 0;
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).NeedsOffice())
			c++;
	return c;
}

bool AuShipmentList::AddShipment(const AuShipment& shipment)
{
	if(size() >= 0xffff)
		return false;
	append(shipment);
	return true;
}

bool AuShipmentList::Remove(const AuShipment& shipment)
{
	for(iterator si = begin(); si != end(); si++)
		if(*si == shipment)
		{
			erase(si);
			return true;
		}
	return false;
}

bool AuShipmentList::Fill(AuItemHash& source)
{
	bool anyChange = false;
	for(iterator si = begin(); si != end(); si++)
		if((*si).Fill(source))
		{
			// Fill again to get the second best stuff too, in case the best has only a few in each cycle.
			(*si).Fill(source);
			anyChange = true;
		}
	return anyChange;
}

bool AuShipmentList::Fill(const AuItemHash& source, AuItemHash& taken)
{
	bool anyChange = false;
	for(iterator si = begin(); si != end(); si++)
		if((*si).Fill(source, taken))
		{
			// Fill again to get the second best stuff too, in case the best has only a few in each cycle.
			(*si).Fill(source, taken);
			anyChange = true;
		}
	return anyChange;
}

bool AuShipmentList::Ship(const AuShipmentBroker& broker, const QSet<AuId>& cityIds)
{
	AuShipmentList shipped;
	bool anyChange = false;
	iterator si = begin();
	while(si != end())
	{
		if(!cityIds.contains((*si).Container().DestinationCity()))
		{
			// Found an order whose destination city does not exist.
			si = erase(si);
			anyChange = true;
			continue;	// Skip si++.
		}
		else if((*si).IsAwaitingShipment())
		{
			// Found an order awaiting shipment. Can its broker handle any more enroute?
			if(CountEnroute() < broker.Levels())
			{
				// Broker can handle another shipment enroute.
				anyChange = true;
				if(shipped.AddShipment(*si))
				{
					// Configure the enroute shipment.
					iterator esi = shipped.end()-1;
					(*esi).SetShipped();
				}

				// Reset the shipment that represents the order, or delete it if the order is complete.
				if((*si).ResetAfterShipped())
				{
					// The order is complete.
					si = erase(si);
					continue;	// Skip si++.
				}
			}
		}
		si++;
	}

	// Append the shipped orders, to be delivered later.
	if(!shipped.empty())
		append(shipped);
	return anyChange;
}

bool AuShipmentList::Deliver(AuShipmentList& deliveries, AuTradeAgreementList& trades)
{
	bool anyChange = false;
	iterator si = begin();
	while(si != end())
	{
		if((*si).IsReadyToDeliver())
		{
			// Found an order that is enroute and ready to be delivered.
			if((*si).TradeAgreement())
			{
				// The shipment signals that it is ready to be delivered. We require acknowledgement from the receiver when they
				// send the trade goods.
				trades.append(*(*si).TradeAgreement());
			}
			else
			{
				// The shipment is delivered.
				deliveries.AddShipment(*si);
				si = erase(si);
				anyChange = true;
				continue;	// Skip si++.
			}
		}
		si++;
	}
	return anyChange;
}

int AuShipmentList::DeliverTrade(AuShipment& delivery, const AuTradeAgreement& ta)
{
	for(iterator si = begin(); si != end(); si++)
		if((*si).TradeAgreement() && *(*si).TradeAgreement() == ta)
		{
			if((*si).IsReadyToDeliver())
			{
				// A trade agreement has only one associated shipment.
				delivery = *si;
				si = erase(si);
				return 0;		// Trade shipment is delivered.
			}
			else
				return 1;		// Trade agreement found here but it is not ready for delivery.
		}
	return 2;					// Trade agreement not found here at all.
}

quint64 AuShipmentList::MemoryUsage() const
{
	quint64 mem = sizeof(AuShipmentList);
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		mem += (*ii).MemoryUsage();
	return mem;
}

void AuShipmentList::ReadyToShip(AuShipmentList& shipments) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).IsAwaitingShipment())
			shipments.AddShipment(*si);
}

bool AuShipmentList::PickUp(const AuShipment& shipment)
{
	if(!shipment.IsAwaitingShipment())
		return false;
	for(iterator si = begin(); si != end(); si++)
		if(*si == shipment)
		{
			// Reset the shipment that represents the order, or delete it if the order is complete.
			if((*si).ResetAfterShipped())
				erase(si);	// The order is complete.
			return true;	// The order was found for pickup.
		}
	return false;
}

void AuShipmentList::TradeAgreementCanceled(const AuTradeAgreement& ta)
{
	for(iterator si = begin(); si != end(); si++)
	{
		if(!(*si).TradeAgreement())
			continue;
		if(*(*si).TradeAgreement() == ta)
		{
			erase(si);
			return;
		}
	}
}

bool AuShipmentList::Reconcile(const AuTradeAgreementList& trades)
{
	bool anyChange = false;
	iterator si = begin();
	while(si != end())
	{
		if((*si).TradeAgreement() && !trades.HasTradeAgreement(*(*si).TradeAgreement()))
		{
			// Shipment is associated with a trade agreement that does not exist in the list of trades.
			anyChange = true;
			si = erase(si);
		}
		else
			si++;	// Shipment is not associated with a trade agreement or its agreement was found in the list of trades.
	}
	return anyChange;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTradeAgreement::AuTradeAgreement(COMMODITYID ocid, quint32 oamount, quint8 ominQ,
		const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
		COMMODITYID rcid, quint32 ramount, quint8 rminQ,
		const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
		quint8 repeat, float distPc)
{
	m_Amount[0] = oamount;
	m_CID[0] = ocid;
	m_City[0] = oCityId;
	m_Desc[0] = oDesc;
	m_Filter[0] = FILTER_All;
	m_MinQ[0] = ominQ;
	m_SectorId[0] = oSectorId;
	m_SysId[0] = oSysId;

	m_Amount[1] = ramount;
	m_CID[1] = rcid;
	m_City[1] = rCityId;
	m_Desc[1] = rDesc;
	m_Filter[1] = FILTER_All;
	m_MinQ[1] = rminQ;
	m_SectorId[1] = rSectorId;
	m_SysId[1] = rSysId;

	m_DistPc = distPc;
	m_Repeat = repeat;
	m_State = Proposal;
}

AuTradeAgreement::AuTradeAgreement(COMMODITYID ocid, quint32 oamount, quint8 ominQ,
		const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
		COMMODITYFILTER rfilter, quint32 ramount, quint8 rminQ,
		const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
		quint8 repeat, float distPc)
{
	m_Amount[0] = oamount;
	m_CID[0] = ocid;
	m_City[0] = oCityId;
	m_Desc[0] = oDesc;
	m_Filter[0] = FILTER_All;
	m_MinQ[0] = ominQ;
	m_SectorId[0] = oSectorId;
	m_SysId[0] = oSysId;

	m_Amount[1] = ramount;
	m_CID[1] = CID_Invalid;
	m_City[1] = rCityId;
	m_Desc[1] = rDesc;
	m_Filter[1] = rfilter;
	m_MinQ[1] = rminQ;
	m_SectorId[1] = rSectorId;
	m_SysId[1] = rSysId;

	m_DistPc = distPc;
	m_Repeat = repeat;
	m_State = Proposal;
}

AuTradeAgreement::AuTradeAgreement(COMMODITYFILTER ofilter, quint32 oamount, quint8 ominQ,
		const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
		COMMODITYID rcid, quint32 ramount, quint8 rminQ,
		const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
		quint8 repeat, float distPc)
{
	m_Amount[0] = oamount;
	m_CID[0] = CID_Invalid;
	m_City[0] = oCityId;
	m_Desc[0] = oDesc;
	m_Filter[0] = ofilter;
	m_MinQ[0] = ominQ;
	m_SectorId[0] = oSectorId;
	m_SysId[0] = oSysId;

	m_Amount[1] = ramount;
	m_CID[1] = rcid;
	m_City[1] = rCityId;
	m_Desc[1] = rDesc;
	m_Filter[1] = FILTER_All;
	m_MinQ[1] = rminQ;
	m_SectorId[1] = rSectorId;
	m_SysId[1] = rSysId;

	m_DistPc = distPc;
	m_Repeat = repeat;
	m_State = Proposal;
}

AuTradeAgreement::AuTradeAgreement(COMMODITYFILTER ofilter, quint32 oamount, quint8 ominQ,
		const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
		COMMODITYFILTER rfilter, quint32 ramount, quint8 rminQ,
		const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
		quint8 repeat, float distPc)
{
	m_Amount[0] = oamount;
	m_CID[0] = CID_Invalid;
	m_City[0] = oCityId;
	m_Desc[0] = oDesc;
	m_Filter[0] = ofilter;
	m_MinQ[0] = ominQ;
	m_SectorId[0] = oSectorId;
	m_SysId[0] = oSysId;

	m_Amount[1] = ramount;
	m_CID[1] = CID_Invalid;
	m_City[1] = rCityId;
	m_Desc[1] = rDesc;
	m_Filter[1] = rfilter;
	m_MinQ[1] = rminQ;
	m_SectorId[1] = rSectorId;
	m_SysId[1] = rSysId;

	m_DistPc = distPc;
	m_Repeat = repeat;
	m_State = Proposal;
}

AuTradeAgreement::AuTradeAgreement()
{
	for(int i=0; i<2; i++)
	{
		m_Amount[i] = 0;
		m_CID[i] = CID_Invalid;
		m_Filter[i] = FILTER_All;
		m_MinQ[i] = 1;
	}
	m_DistPc = 0;
	m_Repeat = 0;
	m_State = Proposal;
}

AuTradeAgreement::AuTradeAgreement(const AuTradeAgreement& c)
{
	for(int i=0; i<2; i++)
	{
		m_Amount[i] = c.m_Amount[i];
		m_CID[i] = c.m_CID[i];
		m_City[i] = c.m_City[i];
		m_Desc[i] = c.m_Desc[i];
		m_Filter[i] = c.m_Filter[i];
		m_MinQ[i] = c.m_MinQ[i];
		m_SectorId[i] = c.m_SectorId[i];
		m_SysId[i] = c.m_SysId[i];
	}
	m_DistPc = c.m_DistPc;
	m_Repeat = c.m_Repeat;
	m_State = c.m_State;
}

AuTradeAgreement::~AuTradeAgreement()
{
}

void AuTradeAgreement::operator>>(QDataStream& out) const
{
	out << (quint8)m_State;
	out << m_DistPc;
	out << m_Repeat;
	for(int i=0; i<2; i++)
	{
		out << m_Amount[i];
		out << (quint16)m_CID[i];
		out << m_City[i];
		out << m_Desc[i];
		out << (quint8)m_Filter[i];
		out << m_MinQ[i];
		out << m_SectorId[i];
		out << m_SysId[i];
	}
}

void AuTradeAgreement::operator<<(QDataStream& in)
{
	quint8 ui8;
	in >> ui8;
	m_State = (STATE)ui8;
	in >> m_DistPc;
	in >> m_Repeat;
	for(int i=0; i<2; i++)
	{
		in >> m_Amount[i];
		quint16 ui16;
		in >> ui16;
		m_CID[i] = (COMMODITYID)ui16;
		in >> m_City[i];
		in >> m_Desc[i];
		in >> ui8;
		m_Filter[i] = (COMMODITYFILTER)ui8;
		in >> m_MinQ[i];
		in >> m_SectorId[i];
		in >> m_SysId[i];
	}
}

const AuTradeAgreement& AuTradeAgreement::operator=(const AuTradeAgreement& c)
{
	for(int i=0; i<2; i++)
	{
		m_Amount[i] = c.m_Amount[i];
		m_CID[i] = c.m_CID[i];
		m_City[i] = c.m_City[i];
		m_Desc[i] = c.m_Desc[i];
		m_Filter[i] = c.m_Filter[i];
		m_MinQ[i] = c.m_MinQ[i];
		m_SectorId[i] = c.m_SectorId[i];
		m_SysId[i] = c.m_SysId[i];
	}
	m_DistPc = c.m_DistPc;
	m_Repeat = c.m_Repeat;
	m_State = c.m_State;
	return *this;
}

bool AuTradeAgreement::operator==(const AuTradeAgreement& c) const
{
	if(m_State != c.m_State)
		return false;
	if(m_Repeat != c.m_Repeat)
		return false;
	if(m_DistPc != c.m_DistPc)
		return false;
	for(int i=0; i<2; i++)
	{
		if(m_Amount[i] != c.m_Amount[i])
			return false;
		if(m_CID[i] != c.m_CID[i])
			return false;
		if(m_City[i] != c.m_City[i])
			return false;
		if(m_Desc[i] != c.m_Desc[i])
			return false;
		if(m_Filter[i] != c.m_Filter[i])
			return false;
		if(m_MinQ[i] != c.m_MinQ[i])
			return false;
		if(m_SectorId[i] != c.m_SectorId[i])
			return false;
		if(m_SysId[i] != c.m_SysId[i])
			return false;
	}
	return true;
}

//static
int AuTradeAgreement::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuTradeAgreement>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuTradeAgreement>();

	return metaType;
}

QString AuTradeAgreement::ListDesc() const
{
	QString oname;
	if(SendCommodityId() != CID_Invalid)
		oname = QObject::tr("%L1 units of %2").arg(SendAmount()).arg(g_Au.CommodityName(SendCommodityId()));
	else
		oname = QObject::tr("%L1 units of %2").arg(SendAmount()).arg(CommodityFilterName(SendCommodityFilter()));
	if(SendMinQ() > 1)
		oname += QObject::tr(", Minimum Q%1").arg(SendMinQ());

	QString gname;
	if(GetCommodityId() != CID_Invalid)
		gname = QObject::tr("%L1 units of %2").arg(GetAmount()).arg(g_Au.CommodityName(GetCommodityId()));
	else
		gname = QObject::tr("%L1 units of %2").arg(GetAmount()).arg(CommodityFilterName(GetCommodityFilter()));
	if(GetMinQ() > 1)
		gname += QObject::tr(", Minimum Q%1").arg(GetMinQ());

	QString terms;
	if(m_Repeat == 1)
		terms = QObject::tr("concluded after one trade");
	else if(m_Repeat)
		terms = QObject::tr("concluded after %L1 trades").arg(m_Repeat);
	else
		terms = QObject::tr("continuously until terminated by either broker");

	switch(m_State)
	{
	case Proposal:
		return QObject::tr("Trade Proposal\n%1\nproposes to trade %2\nwith %3\nin exchange for %4\n%5.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Declined:
		return QObject::tr("Trade Proposal Declined\n%1\nproposal to trade %2\nwith %3\nin exchange for %4\n%5\nwas declined.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Countered:
		return QObject::tr("Trade Proposal Countered\n%1\nproposal to trade %2\nwith %3\nin exchange for %4\n%5\nwas countered.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Undeliverable:
		return QObject::tr("Trade Proposal Undeliverable\n%1\nproposal to trade %2\nwith %3\nin exchange for %4\n%5\n"
				"could not be delivered.\nNo broker offices are available at the destination.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Agreement:
		return QObject::tr("Trade Agreement\n%1\nwill trade %2\nwith %3\nin exchange for %4\n%5.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Terminated:
		return QObject::tr("Trade Agreement Terminated\n%1\nagreement to trade %2\nwith %3\nin exchange for %4\n%5\n"
				"was terminated.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	case Concluded:
		return QObject::tr("Trade Agreement Concluded\n%1\nagreement to trade %2\nwith %3\nin exchange for %4\n%5\nwas concluded.")
			.arg(OriginDesc())
			.arg(oname)
			.arg(DestinationDesc())
			.arg(gname)
			.arg(terms);
	}
	return QString("Trade Agreement State %1").arg(m_State);
}

QString AuTradeAgreement::HtmlToolTip() const
{
	QString text(ListDesc());
	QString title;
	int bs = text.indexOf('\n');
	if(bs > 0)
	{
		title += QString("<b>%1</b><br>").arg(text.left(bs));	// bold title
		text = text.mid(bs+1);
	}
	text.replace('\n', ' ');
	return title + text;
}

bool AuTradeAgreement::Deactivate(STATE nuState)
{
	Q_ASSERT(nuState != Proposal && nuState != Agreement);	// These are not inactive states.
	if(m_State == nuState)
		return false;
	m_State = nuState;
	return true;
}

bool AuTradeAgreement::Accept()
{
	// Only a proposal can be accepted.
	if(m_State != Proposal)
		return false;
	m_State = Agreement;
	return true;
}

void AuTradeAgreement::Report(AuEventLog& events) const
{
	switch(m_State)
	{
	case Proposal:
	case Declined:
	case Countered:
	case Undeliverable:
		events.AddEventTrim(AuEventLogEntry(AuEventLogEntry::TradeProposal, HtmlToolTip()));
		break;
	case Agreement:	// Don't report agreements that are chugging along just fine.
	case Concluded:	// Don't report agreements that concluded according to the terms of the agreement.
		break;
	case Terminated:
		events.AddEventTrim(AuEventLogEntry(AuEventLogEntry::TradeAgreement, HtmlToolTip()));
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuTradeAgreementList::AddTradeAgreement(const AuTradeAgreement& ta)
{
	if(size() >= 0xffff)
		return false;
	append(ta);
	return true;
}

bool AuTradeAgreementList::Deactivate(const AuTradeAgreement& ta, AuTradeAgreement::STATE nuState)
{
	for(iterator si = begin(); si != end(); si++)
		if(*si == ta)
			return (*si).Deactivate(nuState);
	return false;
}

bool AuTradeAgreementList::Remove(const AuTradeAgreement& ta)
{
	for(iterator si = begin(); si != end(); si++)
		if(*si == ta)
		{
			erase(si);
			return true;
		}
	return false;
}

quint16 AuTradeAgreementList::CountOffices() const
{
	quint16 c = 0;
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).NeedsOffice())
			c++;
	return c;
}

bool AuTradeAgreementList::Accept(AuTradeAgreement& ta)
{
	for(iterator si = begin(); si != end(); si++)
		if(*si == ta)
		{
			bool accepted = (*si).Accept();
			if(accepted)
				ta = *si;
			return accepted;
		}
	return false;
}

void AuTradeAgreementList::DeliveryComplete(AuTradeAgreement& ta)
{
	for(iterator si = begin(); si != end(); si++)
		if(*si == ta)
		{
			// Update my trade agreement.
			if((*si).ReduceRepeatCount())
				ta = *si;	// Return the changes, which will include a state change to Concluded if the repeat count was one.
			break;
		}
}

bool AuTradeAgreementList::HasTradeAgreement(const AuTradeAgreement& ta) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if(*si == ta)
			return true;
	return false;
}

void AuTradeAgreementList::Report(AuEventLog& events) const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		(*si).Report(events);
}

