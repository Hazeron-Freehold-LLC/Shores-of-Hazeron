//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumission.cpp,v 1.190 2022/05/04 19:38:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumission.h"
//
#include "auglobal.h"
#include "auglobell.h"
#include <dbcommodity.h>
#include <QDataStream>

static int s_AuMissionMetaType = 0;
static AuMissionOrderStay s_DefaultOrder;						// Default stay order.
static AuHarvestOps s_HarvestOps;								// Default empty harvester operations.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuHarvestOps::MemoryUsage() const
{
	return (sizeof(quint16) + sizeof(AuHarvestOp)) * size();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuMissionList::Remove(const QString& missionName)
{
	iterator mi = find(missionName);
	if(mi == end())
		return false;
	erase(mi);
	return true;
}

QString AuMissionList::HtmlListContents() const
{
	// Format
	QString cont = QObject::tr("<hr><img height=\"16\" src=\"qrc:data/Folder.png\"> Missions <b>%1</b>").arg(size());
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		cont += QString("<br>%4 <img height=\"16\" src=\"qrc:data/c_MissionDisk.png\"> %1, %2 orders, %3")
			.arg(mi.key())
			.arg((*mi).Count())
			.arg((*mi).Type() == AuMission::Single?QObject::tr("Do Once"):QObject::tr("Cycle"))
			.arg(g_Au.MiddleDotSymbol());
	return cont;
}

void AuMissionList::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		out << mi.key();
		out << *mi;
	}
}

void AuMissionList::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		QString n;
		AuMission m;
		in >> n;
		in >> m;
		insert(n, m);
	}
}

quint64 AuMissionList::MemoryUsage() const
{
	quint64 mem = sizeof(AuMissionList);
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		mem += (*mi).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMission::AuMission()
{
	m_Active = true;
	m_Type = Single;

	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// Also needed to use this class in qVariantSetValue.
	// The Q_DECLARE_METATYPE macro is also required.
	if(!s_AuMissionMetaType)
		s_AuMissionMetaType = qRegisterMetaType<AuMission>();
}

AuMission::AuMission(const AuMission& c) : QList<AuMissionOrder*>()
{
	m_Active = c.m_Active;
	m_LastArriveId = c.m_LastArriveId;
	m_OrderStart = c.m_OrderStart;
	m_Type = c.m_Type;

	for(AuMission::const_iterator coi = c.constBegin(); coi != c.constEnd(); coi++)
		append((*coi)->Clone());

	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// Also needed to use this class in qVariantSetValue.
	// The Q_DECLARE_METATYPE macro is also required.
	if(!s_AuMissionMetaType)
		s_AuMissionMetaType = qRegisterMetaType<AuMission>();
}

void AuMission::operator<<(QDataStream& in)
{
	quint8 ui8;
	in >> ui8;
	m_Active = (ui8 & 0x80) ? true : false;						// top bit is 'active' flag
	m_Type = (TYPE)(ui8 & 0x7f);								// strip 'active' flag bit

	in >> m_OrderStart;
	in >> m_LastArriveId;

	Clear();

	quint8 ordercount;
	in >> ordercount;
	quint8 ordertype;
	while(ordercount--)
	{
		in >> ordertype;

		AuMissionOrder* order = AuMissionOrder::Create((AuMissionOrder::TYPE)ordertype);
		if(!order)
		{
			qWarning() << "AuMission::operator<< ERROR! read bad object type" << ordertype;
			break;
		}

		in >> *order;
		append(order);
	}
}

void AuMission::operator>>(QDataStream& out) const
{
	quint8 ui8 = m_Type;
	if(m_Active)
		ui8 |= 0x80;	// top bit is 'active' flag
	out << ui8;
	out << m_OrderStart;
	out << m_LastArriveId;

	out << (quint8)count();
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
	{
		out << (quint8)(*moi)->Type();
		out << **moi;
	}
}

void AuMission::operator>>(QDebug& dbg) const
{
	if(m_Active)
		dbg.nospace() << "Active";
	else
		dbg.nospace() << "Inactive";
		
	if(Type() == Single)
		dbg.nospace() << ", Do Once";
	else
		dbg.nospace() << ", Cycle";

	if(m_Active)
		dbg.nospace() << ", Order Started " << m_OrderStart;

	if(!m_LastArriveId.IsNull())
		dbg.nospace() << ", Last Arrived " << m_LastArriveId;

	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		dbg.nospace() << "\n" + g_Au.MiddleDotSymbol() + " " + (*moi)->ListDescription();
}

QDebug operator<<(QDebug dbg, const AuMission& m)
{
	m >> dbg;
	return dbg.space();
}

QString AuMission::HtmlReportTable() const
{
	QString rpt = QObject::tr("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: "
			"rgb(24, 24, 48);\"><tbody>");
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td><img height=\"16\" src=\"qrc%2\"></td><td>%3</td></tr>")
			.arg((*moi)->ListNameIcon())
			.arg((*moi)->ListDescriptionIcon())
			.arg((*moi)->ListDescription());
	rpt += "</tbody></table><br>";
	return rpt;
}

QString AuMission::HtmlDesc() const
{
	QString toolTip = QObject::tr("<b>%2</b>").arg(Type() == Single?QObject::tr("Do Once"):QObject::tr("Cycle"));
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		toolTip += "<br>" + g_Au.MiddleDotSymbol() + " " + (*moi)->ListDescription();
	return toolTip;
}

QString AuMission::HtmlToolTip() const
{
	QString toolTip = QObject::tr("<b>%2</b><br>%1 orders")
		.arg(Type() == Single?QObject::tr("Do Once"):QObject::tr("Cycle"))
		.arg(size());
	if(size())
	{
		const_iterator moi = constBegin();
		toolTip += "<br>" + g_Au.MiddleDotSymbol() + " " + (*moi)->ListDescription();
	}
	if(size() > 2)
	{
		toolTip += "<br> ...";
	}
	if(size() > 1)
	{
		const_iterator moi = constEnd()-1;
		toolTip += "<br>" + g_Au.MiddleDotSymbol() + " " + (*moi)->ListDescription();
	}
	return toolTip;
}

const AuMissionOrder& AuMission::CurrentOrder() const
{
	if(isEmpty() || !m_Active)
		return s_DefaultOrder;
	return *first();
}

QString AuMission::NextSystem() const
{
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		if((*moi)->Type() == AuMissionOrder::GoToSystemDirect_Deprecated ||
				(*moi)->Type() == AuMissionOrder::GoToSystemDirect ||
				(*moi)->Type() == AuMissionOrder::GoToSystemViaWormholes)
			return (*moi)->DestinationName();
	return "";
}

// Searches back from the end of the mission for the last travel order that contains a solar system id.
// Returns AuId(0) if no such orders exist.
AuId AuMission::SystemIdBefore(quint8 orderId) const
{
	for(const_iterator moi = FindId(orderId)-1; moi != constBegin()-1; moi--)
		if((*moi)->Type() == AuMissionOrder::GoToSystemDirect_Deprecated ||
				(*moi)->Type() == AuMissionOrder::GoToSystemDirect ||
				(*moi)->Type() == AuMissionOrder::GoToSystemViaWormholes)
			return (*moi)->DestinationId();
	return AuId(0);
}

// Searches back from the end of the mission for the last travel order that contains a solar system id.
// Returns AuId(0) if no such orders exist.
AuId AuMission::SystemIdFinal() const
{
	for(const_iterator moi = constEnd()-1; moi != constBegin()-1; moi--)
		if((*moi)->Type() == AuMissionOrder::GoToSystemDirect_Deprecated ||
				(*moi)->Type() == AuMissionOrder::GoToSystemDirect ||
				(*moi)->Type() == AuMissionOrder::GoToSystemViaWormholes)
			return (*moi)->DestinationId();
	return AuId(0);
}

// Appends the ListDescription of each order to 'listDescriptions'.
void AuMission::ListDescriptions(QStringList& listDescriptions) const
{
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		listDescriptions << (*moi)->ListDescription();
}

// Appends the ListDescription of each order to 'listDescriptions'.
// Do once orders are prepended with the term "Do once."
void AuMission::ListDescriptionsWithDoOnce(QStringList& listDescriptions) const
{
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		if((*moi)->IsDoOnce())
			listDescriptions << QObject::tr("Do once. ") + (*moi)->ListDescription();
		else
			listDescriptions << (*moi)->ListDescription();
}

// Returns the next id available for a mission order. Returns 0 if no ids are available, i.e. the mission is full.
// Note that ids are not necessarily sequential in a list of orders.
quint8 AuMission::NextId() const
{
	if(isEmpty())
		return 1;						// Easy 1. ;)
	if(count() >= 0xff)
		return 0;						// There can be no more ids available.

	// Find highest id in use.
	quint8 lowestId = 0xff;
	quint8 highestId = 0;
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
	{
		if((*moi)->MissionOrderId() > highestId)
			highestId = (*moi)->MissionOrderId();
		if((*moi)->MissionOrderId() < lowestId)
			lowestId = (*moi)->MissionOrderId();
	}
	if(highestId < 0xff)
		return highestId+1;				// Use next highest id.

	// Current highest id is at the top limit 0xff.
	if(lowestId > 1)
		return 1;						// 1 is available.

	// Find an unused id.
	// Make a hash of the ids that are currently in use.
	QSet<AuId> idsInUse;
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		idsInUse.insert((*moi)->MissionOrderId());

	// Now select an id that is not in use.
	// We limit the ids to a max of 0xff (255). That's plenty for mission orders.
	quint8 nextId = 1;
	do
	{
		if(!idsInUse.contains(nextId))
			return nextId;				// We found a valid non-zero id for a nu mission order.
		nextId++;						// 0xff will roll over to 0, stopping the loop.
	}
	while(nextId);

	// Return 0 if we cannot find an unused id, which is highly unlikely unless someone is trying something bogus.
	return 0;
}

quint64 AuMission::MemoryUsage() const
{
	quint64 mem = sizeof(AuMission);
	mem += sizeof(m_Active);
	mem += sizeof(m_LastArriveId);
	mem += sizeof(m_OrderStart);
	mem += sizeof(m_Type);
	for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		mem += (*moi)->MemoryUsage();
	return mem;
}

bool AuMission::operator==(const AuMission& c) const
{
	if(m_Active != c.m_Active)
		return false;

	//DHH This probably shouldn't be a part of the mission.
	//if(m_LastArriveId != c.m_LastArriveId)
		//return false;

	//DHH This probably shouldn't be a part of the mission.
	//if(m_OrderStart != c.m_OrderStart)
		//return false;

	if(m_Type != c.m_Type)
		return false;

	if(count() != c.count())
		return false;

	for(AuMission::const_iterator moi = constBegin(), coi = c.constBegin(); moi != constEnd() && coi != c.constEnd();
			moi++, coi++)
		if(*(*moi) != *(*coi))
			return false;

	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrder::AuMissionOrder(EXECTYPE execType)
{
	m_ExecType = execType;
	m_Id = 0;
}

AuMissionOrder::AuMissionOrder()
{
	m_ExecType = Sequential;
	m_Id = 0;
}

AuMissionOrder::AuMissionOrder(const AuMissionOrder& c)
{
	m_ExecType = c.m_ExecType;
	m_Id = c.m_Id;
}

const AuMissionOrder& AuMissionOrder::operator=(const AuMissionOrder& c)
{
	m_ExecType = c.m_ExecType;
	m_Id = c.m_Id;
	return *this;
}

//static
AuMissionOrder* AuMissionOrder::Create(TYPE orderType)
{
	switch(orderType)
	{
	case Invalid:
	case OrderTypes:
		break;
	case Attack:
		return new AuMissionOrderAttack;
	case CargoBuyCategory:
		return new AuMissionOrderCargoBuyCategory;
	case CargoBuyCommodity:
		return new AuMissionOrderCargoBuyCommodity;
	case CargoSellCategory:
		return new AuMissionOrderCargoSellCategory;
	case CargoSellCommodity:
		return new AuMissionOrderCargoSellCommodity;
	case Christen:
		return new AuMissionOrderChristen;
	case CrewLoad:
		return new AuMissionOrderCrewLoad;
	case CryoBerthsLoad:
		return new AuMissionOrderCryoBerthsLoad;
	case CryoBerthsUnload:
		return new AuMissionOrderCryoBerthsUnload;
	case DamageReport:
		return new AuMissionOrderDamageReport;
	case ExterminateRats:
		return new AuMissionOrderExterminateRats;
	case Follow:
		return new AuMissionOrderFollow;
	case GoToBestCity:
		return new AuMissionOrderGoToBestCity;
	case GoToBestStation:
		return new AuMissionOrderGoToBestStation;
	case GoToBestStationOrCity:
		return new AuMissionOrderGoToBestStationOrCity;
	case GoToCity:
		return new AuMissionOrderGoToCity;
	case GoToCityAnywhere:
		return new AuMissionOrderGoToCityAnywhere;
	case GoToClosestCity:
		return new AuMissionOrderGoToClosestCity;
	case GoToClosestStation:
		return new AuMissionOrderGoToClosestStation;
	case GoToClosestStationOrCity:
		return new AuMissionOrderGoToClosestStationOrCity;
	case GoToCoordinate:
		return new AuMissionOrderGoToCoordinate;
	case GoToHoverSite:
		return new AuMissionOrderGoToHoverSite;
	case GoToLand:
		return new AuMissionOrderGoToLand;
	case GoToLandingSite:
		return new AuMissionOrderGoToLandingSite;
	case GoToOrbit:
		return new AuMissionOrderGoToOrbit;
	case GoToOrbitBestPlanet:
		return new AuMissionOrderGoToOrbitBestPlanet;
	case GoToOrbitCity:
		return new AuMissionOrderGoToOrbitCity;
	case GoToOrbitCoords:
		return new AuMissionOrderGoToOrbitCoords;
	case GoToStation:
		return new AuMissionOrderGoToStation;
	case GoToSystemDirect_Deprecated:
		return new AuMissionOrderGoToSystemDirect_Deprecated;
	case GoToSystemDirect:
		return new AuMissionOrderGoToSystemDirect;
	case GoToSystemViaWormholes:
		return new AuMissionOrderGoToSystemViaWormholes;
	case Harvest:
		return new AuMissionOrderHarvest;
	case HarvestAtmosphere:
		return new AuMissionOrderHarvestAtmosphere;
	case HarvestMinerals:
		return new AuMissionOrderHarvestMinerals;
	case HarvestOperationByBay:
		return new AuMissionOrderHarvestOperationByBay;
	case HarvestOperationByModule:
		return new AuMissionOrderHarvestOperationByModule;
	case HarvestParticles:
		return new AuMissionOrderHarvestParticles;
	case HarvestSea:
		return new AuMissionOrderHarvestSea;
	case HarvestWell:
		return new AuMissionOrderHarvestWell;
	case Hide:
		return new AuMissionOrderHide;
	case HoldMission:
		return new AuMissionOrderHoldMission;
	case LightState:
		return new AuMissionOrderLightState;
	case PassengersLoad:
		return new AuMissionOrderPassengersLoad;
	case PassengersUnload:
		return new AuMissionOrderPassengersUnload;
	case PatrolCities:
		return new AuMissionOrderPatrolCities;
	case PatrolStations:
		return new AuMissionOrderPatrolStations;
	case PatrolSystem:
		return new AuMissionOrderPatrolSystem;
	case PatrolWormholes:
		return new AuMissionOrderPatrolWormholes;
	case Plunder:
		return new AuMissionOrderPlunder;
	case PilotPolicy:
		return new AuMissionOrderPilotPolicy;
	case Posture:
		return new AuMissionOrderPosture;
	case Repair:
		return new AuMissionOrderRepair;
	case Report:
		return new AuMissionOrderReport;
	case ScanSurveyPlanet:
		return new AuMissionOrderScanSurveyPlanet;
	case ScanSurveySystem:
		return new AuMissionOrderScanSurveySystem;
	case ScanTactical:
		return new AuMissionOrderScanTactical;
	case Scuttle:
		return new AuMissionOrderScuttle;
	case ShieldPolicy:
		return new AuMissionOrderShieldPolicy;
	case ShieldsDown:
		return new AuMissionOrderShieldsDown;
	case StandDown:
		return new AuMissionOrderStandDown;
	case Stay:
		return new AuMissionOrderStay;
	case TroopsLoad:
		return new AuMissionOrderTroopsLoad;
	case TransferCargoFrom:
		return new AuMissionOrderTransferCargoFrom;
	case TransferCargoTo:
		return new AuMissionOrderTransferCargoTo;
	case TransferCargoCategoryFrom:
		return new AuMissionOrderTransferCargoCategoryFrom;
	case TransferCargoCategoryTo:
		return new AuMissionOrderTransferCargoCategoryTo;
	case TransferMoneyFrom:
		return new AuMissionOrderTransferMoneyFrom;
	case TransferMoneyTo:
		return new AuMissionOrderTransferMoneyTo;
	case TransferPeopleFrom:
		return new AuMissionOrderTransferPeopleFrom;
	case TransferPeopleTo:
		return new AuMissionOrderTransferPeopleTo;
	}
	return 0;
}

void AuMissionOrder::operator<<(QDataStream& in)
{
	in >> m_ExecType;
	in >> m_Id;
}

void AuMissionOrder::operator>>(QDataStream& out) const
{
	out << m_ExecType;
	out << m_Id;
}

const AuHarvestOps& AuMissionOrder::HarvestOps() const
{
	return s_HarvestOps;
}

quint64 AuMissionOrder::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ExecType);
	mem += sizeof(m_Id);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderAttack::AuMissionOrderAttack(const AuId& targetId, const QString& targetName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_TargetId = targetId;
	m_TargetName = targetName;
}

AuMissionOrderAttack::AuMissionOrderAttack()
{
}

AuMissionOrderAttack::AuMissionOrderAttack(const AuMissionOrderAttack& c) : AuMissionOrder(c)
{
	m_TargetId = c.m_TargetId;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderAttack::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_TargetId;
	in >> m_TargetName;
}

void AuMissionOrderAttack::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_TargetId;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderAttack::Clone() const
{
	return new AuMissionOrderAttack(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderAttack::ListDescription() const
{
	return QObject::tr("Attack %1.").arg(m_TargetName);
}

QString AuMissionOrderAttack::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderAttack::ListName() const
{
	if(m_TargetName.isEmpty())
		return QObject::tr("Attack");
	return QObject::tr("Attack %1").arg(m_TargetName);
}

QString AuMissionOrderAttack::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderAttack::ListToolTip() const
{
	return QObject::tr("<html>Attack a unit. "
			"A unit to attack is selected from a list. "
			"Crew will not follow the unit through a wormhole.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderAttack::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Battle stations! Attack %1. Destroy it!")
		.arg(m_TargetName);
}

quint64 AuMissionOrderAttack::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_TargetId);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCargoBuyCategory::AuMissionOrderCargoBuyCategory(COMMODITYFILTER commodityFilter, quint32 amount, quint8 amountType,
		quint32 margin, quint8 marginType, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Amount = amount;
	m_AmountType = amountType;
	m_CommodityFilter = commodityFilter;
	m_Margin = margin;
	m_MarginType = marginType;
}

AuMissionOrderCargoBuyCategory::AuMissionOrderCargoBuyCategory()
{
	m_Amount = 0;
	m_AmountType = 0;
	m_CommodityFilter = 0xff;			// Invalid so default constructor produces default ListName.
	m_Margin = 0;
	m_MarginType = 0;
}

AuMissionOrderCargoBuyCategory::AuMissionOrderCargoBuyCategory(const AuMissionOrderCargoBuyCategory& c) : AuMissionOrder(c)
{
	m_Amount = c.m_Amount;
	m_AmountType = c.m_AmountType;
	m_CommodityFilter = c.m_CommodityFilter;
	m_Margin = c.m_Margin;
	m_MarginType = c.m_MarginType;
}

void AuMissionOrderCargoBuyCategory::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 ttf = 0x01;					// Bottom nibble is version.
	ttf |= m_AmountType << 4;			// amount type fits into two bits
	ttf |= m_MarginType << 6;			// margin type fits into two bits

	out << ttf;
	out << (quint16)0;					// Amount 0 positively identifies the change in format because it would not have been valid.
	out << m_Amount;
	out << m_Margin;
	out << m_CommodityFilter;
}

void AuMissionOrderCargoBuyCategory::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 ttf;
	in >> ttf;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Non-zero amount identifies old format.
		m_Amount = u16;
		in >> m_Margin;
		m_CommodityFilter = ttf & 0x0f;		// COMMODITYFILTER used to fit into a nibble.
	}
	else
	{
		// Amount 0 identifies new format.
		int ver = ttf & 0x0f;
		if(ver >= 1)
			in >> m_Amount;
		else
		{
			in >> u16;
			m_Amount = u16;
		}
		in >> m_Margin;
		in >> m_CommodityFilter;
	}
	m_AmountType = (ttf >> 4) & 0x03;	// amount type fits into two bits
	m_MarginType = (ttf >> 6) & 0x03;	// margin type fits into two bits
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCargoBuyCategory::Clone() const
{
	return new AuMissionOrderCargoBuyCategory(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCargoBuyCategory::ListDescription() const
{
	// Start with the primary order.
	QString text;
	if(m_MarginType == 0)
		text = QObject::tr("Buy %1 if price is less than %2% of nominal price.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(m_Margin);
	else if(m_MarginType == 1)
		text = QObject::tr("Buy %1 if price is less than %2%3.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(m_Margin)
			.arg(g_Au.CurrencySymbol());
	else
		text = QObject::tr("Buy %1 at any price.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));

	// Describe the amount desired.
	if(m_AmountType == 0)
		text += " " + QObject::tr("Manifest calls for %L1% of the hold.").arg(m_Amount);
	else if(m_AmountType == 1)
		text += " " + QObject::tr("Manifest calls for %L1 total units in the hold.").arg(m_Amount);
	else
		text += " " + QObject::tr("Manifest calls for %L1 units of each kind in the hold.").arg(m_Amount);

	return text;
}

QString AuMissionOrderCargoBuyCategory::ListDescriptionIcon() const
{
	return CommodityFilterIcon((COMMODITYFILTER)m_CommodityFilter);
}

QString AuMissionOrderCargoBuyCategory::ListName() const
{
	if(m_CommodityFilter == 0xff)
		return QObject::tr("Buy Cargo Category");
	return QObject::tr("Buy %1").arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderCargoBuyCategory::ListNameIcon() const
{
	return ":data/MsgBuyOffer.png";
}

QString AuMissionOrderCargoBuyCategory::ListToolTip() const
{
	return QObject::tr("<html>Buy a category of cargo. "
			"A total number of units or a percentage of total space is specified; "
			"a number of each kind of commodity in the category may also be specified. "
			"Space is cargo hold, fuel cell or capacitor, depending on the commodity. "
			"That space is divided by all possible commodities in the category to decide how many of each "
			"commodity to buy.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCargoBuyCategory::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Make ready to load %1, if we can get any here.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderCargoBuyCategory::HailIntentions() const
{
	return QObject::tr("We are buying %1.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

quint64 AuMissionOrderCargoBuyCategory::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Amount);
	mem += sizeof(m_AmountType);
	mem += sizeof(m_CommodityFilter);
	mem += sizeof(m_Margin);
	mem += sizeof(m_MarginType);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCargoBuyCommodity::AuMissionOrderCargoBuyCommodity(COMMODITYID commodityId, quint32 amount, quint8 amountType,
		quint32 margin, quint8 marginType, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Amount = amount;
	m_AmountType = amountType;
	m_CommodityId = commodityId;
	m_Margin = margin;
	m_MarginType = marginType;
}

AuMissionOrderCargoBuyCommodity::AuMissionOrderCargoBuyCommodity()
{
	m_Amount = 0;
	m_AmountType = 0;
	m_CommodityId = CID_Invalid;		// Invalid so default constructor produces default ListName.
	m_Margin = 0;
	m_MarginType = 0;
}

AuMissionOrderCargoBuyCommodity::AuMissionOrderCargoBuyCommodity(const AuMissionOrderCargoBuyCommodity& c) : AuMissionOrder(c)
{
	m_Amount = c.m_Amount;
	m_AmountType = c.m_AmountType;
	m_CommodityId = c.m_CommodityId;
	m_Margin = c.m_Margin;
	m_MarginType = c.m_MarginType;
}

void AuMissionOrderCargoBuyCommodity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 ttf;
	in >> ttf;
	int ver = ttf & 0x0f;
	if(ver >= 1)
		in >> m_Amount;
	else
	{
		quint16 u16;
		in >> u16;
		m_Amount = u16;
	}
	in >> m_CommodityId;
	in >> m_Margin;

	m_AmountType  = (ttf >> 4) & 0x03;	// amount type fits into two bits
	m_MarginType  = (ttf >> 6) & 0x03;	// margin type fits into two bits
}

void AuMissionOrderCargoBuyCommodity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 ttf = 0x01;					// Bottom nibble is version.
	ttf |= m_AmountType << 4;			// amount type fits into two bits
	ttf |= m_MarginType << 6;			// margin type fits into two bits

	out << ttf;
	out << m_Amount;
	out << m_CommodityId;
	out << m_Margin;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCargoBuyCommodity::Clone() const
{
	return new AuMissionOrderCargoBuyCommodity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCargoBuyCommodity::ListDescription() const
{
	// Start with the primary order.
	QString text;
	if(m_MarginType == 0)
		text = QObject::tr("Buy %1 if price is less than %2% of nominal price.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_Margin);
	else if(m_MarginType == 1)
		text = QObject::tr("Buy %1 if price is less than %2%3.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_Margin)
			.arg(g_Au.CurrencySymbol());
	else
		text = QObject::tr("Buy %1 at any price.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));

	// Describe the amount desired.
	if(m_CommodityId == CID_Hydrogen)
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Manifest calls for %L1% of the fuel cell.").arg(m_Amount);
		else
			text += " " + QObject::tr("Manifest calls for %L1 units in the fuel cell.").arg(m_Amount);
	}
	else if(m_CommodityId == CID_Electricity)
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Manifest calls for %L1% of the capacitor.").arg(m_Amount);
		else
			text += " " + QObject::tr("Manifest calls for %L1 units in the capacitor.").arg(m_Amount);
	}
	else
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Manifest calls for %L1% of the hold.").arg(m_Amount);
		else
			text += " " + QObject::tr("Manifest calls for %L1 units in the hold.").arg(m_Amount);
	}

	return text;
}

QString AuMissionOrderCargoBuyCommodity::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderCargoBuyCommodity::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Buy Cargo");
	return QObject::tr("Buy %1").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderCargoBuyCommodity::ListNameIcon() const
{
	return ":data/MsgBuyOffer.png";
}

QString AuMissionOrderCargoBuyCommodity::ListToolTip() const
{
	return QObject::tr("<html>Buy a cargo commodity. "
			"A maximum number of units or a percentage of total space is specified. "
			"Space is cargo hold, fuel cell or capacitor, depending on the commodity.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCargoBuyCommodity::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_CommodityId == CID_Hydrogen)
		return QObject::tr("Prepare fuel cells for refueling; I'm going to see if we can get some %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	else if(m_CommodityId == CID_Electricity)
		return QObject::tr("Prepare capacitor for charging at the external coupling; "
				"I'm going to see if we can get some %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	else
		return QObject::tr("Prepare the hold to bring on cargo; I'm going to see if we can get some %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderCargoBuyCommodity::HailIntentions() const
{
	return QObject::tr("We are buying %1.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

quint64 AuMissionOrderCargoBuyCommodity::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Amount);
	mem += sizeof(m_AmountType);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_Margin);
	mem += sizeof(m_MarginType);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCargoSellCategory::AuMissionOrderCargoSellCategory(COMMODITYFILTER commodityFilter, quint32 amount, quint8 amountType,
		quint32 margin, quint8 marginType, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Amount = amount;
	m_AmountType = amountType;
	m_CommodityFilter = commodityFilter;
	m_Margin = margin;
	m_MarginType = marginType;
}

AuMissionOrderCargoSellCategory::AuMissionOrderCargoSellCategory()
{
	m_Amount = 0;
	m_AmountType = 0;
	m_CommodityFilter = 0xff;			// Invalid so default constructor produces default ListName.
	m_Margin = 0;
	m_MarginType = 0;
}

AuMissionOrderCargoSellCategory::AuMissionOrderCargoSellCategory(const AuMissionOrderCargoSellCategory& c) : AuMissionOrder(c)
{
	m_Amount = c.m_Amount;
	m_AmountType = c.m_AmountType;
	m_CommodityFilter = c.m_CommodityFilter;
	m_Margin = c.m_Margin;
	m_MarginType = c.m_MarginType;
}

void AuMissionOrderCargoSellCategory::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 ttf = 0x01;					// Bottom nibble is version.
	ttf |= m_AmountType << 4;			// amount type fits into two bits
	ttf |= m_MarginType << 6;			// margin type fits into two bits

	out << ttf;
	out << (quint16)0;					// Amount 0 positively identifies the change in format because it would not have been valid.
	out << m_Amount;
	out << m_Margin;
	out << m_CommodityFilter;
}

void AuMissionOrderCargoSellCategory::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 ttf;
	in >> ttf;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Non-zero amount identifies old format.
		m_Amount = u16;
		in >> m_Margin;
		m_CommodityFilter = ttf & 0x0f;		// COMMODITYFILTER used to fit into a nibble.
	}
	else
	{
		// Amount 0 identifies new format.
		int ver = ttf & 0x0f;
		if(ver >= 1)
			in >> m_Amount;
		else
		{
			in >> u16;
			m_Amount = u16;
		}
		in >> m_Margin;
		in >> m_CommodityFilter;
	}
	m_AmountType = (ttf >> 4) & 0x03;	// amount type fits into two bits
	m_MarginType = (ttf >> 6) & 0x03;	// margin type fits into two bits
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCargoSellCategory::Clone() const
{
	return new AuMissionOrderCargoSellCategory(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCargoSellCategory::ListDescription() const
{
	// Start with the primary order.
	QString text;
	if(m_MarginType == 0)
		text = QObject::tr("Sell %1 if price is more than %2% of nominal price.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(m_Margin);
	else if(m_MarginType == 1)
		text = QObject::tr("Sell %1 if price is more than %2%3.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(m_Margin)
			.arg(g_Au.CurrencySymbol());
	else
		text = QObject::tr("Sell %1 at any price.")
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));

	// Describe the amount desired.
	if(m_AmountType == 0)
		text += " " + QObject::tr("Offer up to %L1% of the hold.").arg(m_Amount);
	else
		text += " " + QObject::tr("Offer up to %L1 units of each kind from the hold.").arg(m_Amount);

	return text;
}

QString AuMissionOrderCargoSellCategory::ListDescriptionIcon() const
{
	return CommodityFilterIcon((COMMODITYFILTER)m_CommodityFilter);
}

QString AuMissionOrderCargoSellCategory::ListName() const
{
	if(m_CommodityFilter == 0xff)
		return QObject::tr("Sell Cargo Category");
	return QObject::tr("Sell %1").arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderCargoSellCategory::ListNameIcon() const
{
	return ":data/OrderGetItem.png";
}

QString AuMissionOrderCargoSellCategory::ListToolTip() const
{
	return QObject::tr("<html>Sell a category of cargo. "
			"A number of units of each kind or a percentage of total space is specified. "
			"Space is cargo hold, fuel cell or capacitor, depending on the commodity. "
			"That space is divided by all possible commodities in the category to decide how many of each "
			"commodity to sell.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCargoSellCategory::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Make ready to unload %1. I'll see if they want any here.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderCargoSellCategory::HailIntentions() const
{
	return QObject::tr("We are selling %1.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

quint64 AuMissionOrderCargoSellCategory::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Amount);
	mem += sizeof(m_AmountType);
	mem += sizeof(m_CommodityFilter);
	mem += sizeof(m_Margin);
	mem += sizeof(m_MarginType);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCargoSellCommodity::AuMissionOrderCargoSellCommodity(COMMODITYID commodityId, quint32 amount, quint8 amountType,
		quint32 margin, quint8 marginType, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Amount = amount;
	m_AmountType = amountType;
	m_CommodityId = commodityId;
	m_Margin = margin;
	m_MarginType = marginType;
}

AuMissionOrderCargoSellCommodity::AuMissionOrderCargoSellCommodity()
{
	m_Amount = 0;
	m_AmountType = 0;
	m_CommodityId = CID_Invalid;		// Invalid so default constructor produces default ListName.
	m_Margin = 0;
	m_MarginType = 0;
}

AuMissionOrderCargoSellCommodity::AuMissionOrderCargoSellCommodity(const AuMissionOrderCargoSellCommodity& c) : AuMissionOrder(c)
{
	m_Amount = c.m_Amount;
	m_AmountType = c.m_AmountType;
	m_CommodityId = c.m_CommodityId;
	m_Margin = c.m_Margin;
	m_MarginType = c.m_MarginType;
}

void AuMissionOrderCargoSellCommodity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 ttf;
	in >> ttf;
	int ver = ttf & 0x0f;
	if(ver >= 1)
		in >> m_Amount;
	else
	{
		quint16 u16;
		in >> u16;
		m_Amount = u16;
	}
	in >> m_CommodityId;
	in >> m_Margin;

	m_AmountType  = (ttf >> 4) & 0x03;	// amount type fits into two bits
	m_MarginType  = (ttf >> 6) & 0x03;	// margin type fits into two bits
}

void AuMissionOrderCargoSellCommodity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 ttf = 0x01;					// Bottom nibble is version.
	ttf |= m_AmountType << 4;			// amount type fits into two bits
	ttf |= m_MarginType << 6;			// margin type fits into two bits

	out << ttf;
	out << m_Amount;
	out << m_CommodityId;
	out << m_Margin;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCargoSellCommodity::Clone() const
{
	return new AuMissionOrderCargoSellCommodity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCargoSellCommodity::ListDescription() const
{
	// Start with the primary order.
	QString text;
	if(m_MarginType == 0)
		text = QObject::tr("Sell %1 if price is more than %2% of nominal price.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_Margin);
	else if(m_MarginType == 1)
		text = QObject::tr("Sell %1 if price is more than %2%3.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_Margin)
			.arg(g_Au.CurrencySymbol());
	else
		text = QObject::tr("Sell %1 at any price.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));

	// Describe the amount desired.
	if(m_CommodityId == CID_Hydrogen)
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Offer up to %L1% of the fuel cell.").arg(m_Amount);
		else
			text += " " + QObject::tr("Offer up to %L1 units from the fuel cell.").arg(m_Amount);
	}
	else if(m_CommodityId == CID_Electricity)
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Offer up to %L1% of the capacitor.").arg(m_Amount);
		else
			text += " " + QObject::tr("Offer up to %L1 units from the capacitor.").arg(m_Amount);
	}
	else
	{
		if(m_AmountType == 0)
			text += " " + QObject::tr("Offer up to %L1% of the hold.").arg(m_Amount);
		else
			text += " " + QObject::tr("Offer up to %L1 units from the hold.").arg(m_Amount);
	}

	return text;
}

QString AuMissionOrderCargoSellCommodity::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderCargoSellCommodity::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Sell Cargo");
	return QObject::tr("Sell %1").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderCargoSellCommodity::ListNameIcon() const
{
	return ":data/OrderGetItem.png";
}

QString AuMissionOrderCargoSellCommodity::ListToolTip() const
{
	return QObject::tr("<html>Buy a cargo commodity. "
			"A maximum number of units or a percentage of total space is specified. "
			"Space is cargo hold, fuel cell or capacitor, depending on the commodity.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCargoSellCommodity::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_CommodityId == CID_Hydrogen)
		return QObject::tr("Prepare fuel cells to offload fuel; I'm going to try to sell %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	else if(m_CommodityId == CID_Electricity)
		return QObject::tr("Prepare capacitor for discharging at the external coupling; I'm going to try to sell %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	else
		return QObject::tr("Prepare the hold to offload cargo; I'm going to try to sell %1 here.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderCargoSellCommodity::HailIntentions() const
{
	return QObject::tr("We are selling %1.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

quint64 AuMissionOrderCargoSellCommodity::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Amount);
	mem += sizeof(m_AmountType);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_Margin);
	mem += sizeof(m_MarginType);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderChristen::AuMissionOrderChristen(const QString& shipName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_ShipName = shipName;
}

AuMissionOrderChristen::AuMissionOrderChristen()
{
}

AuMissionOrderChristen::AuMissionOrderChristen(const AuMissionOrderChristen& c) : AuMissionOrder(c)
{
	m_ShipName = c.m_ShipName;
}

void AuMissionOrderChristen::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_ShipName;
}

void AuMissionOrderChristen::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_ShipName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderChristen::Clone() const
{
	return new AuMissionOrderChristen(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderChristen::ListDescription() const
{
	return QObject::tr("Christen the spacecraft '%1'.").arg(m_ShipName);
}

QString AuMissionOrderChristen::ListDescriptionIcon() const
{
	return ":data/MsgChristen.png";
}

QString AuMissionOrderChristen::ListName() const
{
	return QObject::tr("Christen Spacecraft");
}

QString AuMissionOrderChristen::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderChristen::ListToolTip() const
{
	return QObject::tr("<html>Christen spacecraft. "
			"A new name for the spacecraft is specified.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderChristen::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("All hands, attention please. I christen this ship %1. May God bless her and all who sail in her.")
		.arg(m_ShipName);
}

quint64 AuMissionOrderChristen::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += StringMemoryUsage(m_ShipName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCrewLoad::AuMissionOrderCrewLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
	m_PreBoardMinutes = preBoardMinutes;
}

AuMissionOrderCrewLoad::AuMissionOrderCrewLoad()
{
	m_Count = 0;
	m_PreBoardMinutes = 0;
}

AuMissionOrderCrewLoad::AuMissionOrderCrewLoad(const AuMissionOrderCrewLoad& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
	m_PreBoardMinutes = c.m_PreBoardMinutes;
}

void AuMissionOrderCrewLoad::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
	in >> m_PreBoardMinutes;
}

void AuMissionOrderCrewLoad::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
	out << m_PreBoardMinutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCrewLoad::Clone() const
{
	return new AuMissionOrderCrewLoad(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCrewLoad::ListDescription() const
{
	QString text;
	if(!m_Count)
		text = QObject::tr("Board crew.");
	else if(m_Count == 1)
		text = QObject::tr("Board one crewman.");
	else
		text = QObject::tr("Board %1 crewmen.").arg(m_Count);

	if(m_PreBoardMinutes)
		text += " " + QObject::tr("Announce boarding %1 minutes prior.").arg(m_PreBoardMinutes);
	
	return text;
}

QString AuMissionOrderCrewLoad::ListDescriptionIcon() const
{
	return ":data/Crewman.png";
}

QString AuMissionOrderCrewLoad::ListName() const
{
	return QObject::tr("Board Crew");
}

QString AuMissionOrderCrewLoad::ListNameIcon() const
{
	return ":data/MsgLoadColonists.png";
}

QString AuMissionOrderCrewLoad::ListToolTip() const
{
	return QObject::tr("<html>Board crew members. "
			"The number of crew berths to load may be specified. "
			"All available crew berths are offered when no count is specified.<br>"
			"Preboarding time gives players an opportunity to accept the boarding announcement. "
			"At the end of the preboarding time, citizen crew members are loaded if available. "
			"When boarding is complete, crew members are kicked from the roster if they are not aboard the spacecraft.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCrewLoad::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("I am going to hire a new crewman if I can find one. Be sure to welcome it aboard.");
	else
		return QObject::tr("I am going to hire some new crewmen if I can find any. Be sure to welcome them aboard.");
}

QString AuMissionOrderCrewLoad::HailIntentions() const
{
	return QObject::tr("We are hiring crewmen.");
}

quint64 AuMissionOrderCrewLoad::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	mem += sizeof(m_PreBoardMinutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderDamageReport::AuMissionOrderDamageReport(bool notifyCoC, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_NotifyCoC = notifyCoC;
}

AuMissionOrderDamageReport::AuMissionOrderDamageReport()
{
	m_NotifyCoC = false;
}

AuMissionOrderDamageReport::AuMissionOrderDamageReport(const AuMissionOrderDamageReport& c) : AuMissionOrder(c)
{
	m_NotifyCoC = c.m_NotifyCoC;
}

void AuMissionOrderDamageReport::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_NotifyCoC;
}

void AuMissionOrderDamageReport::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_NotifyCoC;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderDamageReport::Clone() const
{
	return new AuMissionOrderDamageReport(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderDamageReport::ListDescription() const
{
	if(m_NotifyCoC)
		return QObject::tr("Report damage and crew status to entire chain-of-command.");
	else
		return QObject::tr("Report damage and crew status to the captain.");
}

QString AuMissionOrderDamageReport::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderDamageReport::ListName() const
{
	return QObject::tr("Damage Report");
}

QString AuMissionOrderDamageReport::ListNameIcon() const
{
	return ":data/OrderDamageReport.png";
}

QString AuMissionOrderDamageReport::ListToolTip() const
{
	return QObject::tr("<html>Report any damage to the spacecraft.<br>"
			"The report is sent my mail.<br>"
			"An engineer or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderDamageReport::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& engineer,
		const QString& power, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!engineer.isEmpty())
	{
		if(m_NotifyCoC)
			return QObject::tr("%1 assemble a report of ship damage and crew status. Mail it to the entire chain-of-command.")
				.arg(engineer);
		else
			return QObject::tr("%1 assemble a report of ship damage and crew status. Mail it to the captain.").arg(engineer);
	}
	else if(!power.isEmpty())
	{
		if(m_NotifyCoC)
			return QObject::tr("%1 assemble a report of ship damage and crew status. Mail it to the entire chain-of-command.")
				.arg(power);
		else
			return QObject::tr("%1 assemble a report of ship damage and crew status. Mail it to the captain.").arg(power);
	}
	return "";
}

quint64 AuMissionOrderDamageReport::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_NotifyCoC);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderFollow::AuMissionOrderFollow(const AuId& targetId, const QString& targetName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_TargetId = targetId;
	m_TargetName = targetName;
}

AuMissionOrderFollow::AuMissionOrderFollow()
{
}

AuMissionOrderFollow::AuMissionOrderFollow(const AuMissionOrderFollow& c) : AuMissionOrder(c)
{
	m_TargetId = c.m_TargetId;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderFollow::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_TargetId;
	in >> m_TargetName;
}

void AuMissionOrderFollow::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_TargetId;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderFollow::Clone() const
{
	return new AuMissionOrderFollow(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderFollow::ListDescription() const
{
	return QObject::tr("Follow %1.").arg(m_TargetName);
}

QString AuMissionOrderFollow::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderFollow::ListName() const
{
	if(m_TargetName.isEmpty())
		return QObject::tr("Follow");
	return QObject::tr("Follow %1").arg(m_TargetName);
}

QString AuMissionOrderFollow::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderFollow::ListToolTip() const
{
	return QObject::tr("<html>Follow a unit. "
			"A unit to follow is selected from a list. "
			"Crew will follow the unit through a wormhole.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderFollow::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 follow %2, maintain visual separation. %3 watch for them to go FTL and do your best to follow.")
		.arg(pilot)
		.arg(m_TargetName)
		.arg(navigator);
}

quint64 AuMissionOrderFollow::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_TargetId);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToBestCity::AuMissionOrderGoToBestCity(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToBestCity::AuMissionOrderGoToBestCity(const AuMissionOrderGoToBestCity& c) : AuMissionOrder(c)
{
}

void AuMissionOrderGoToBestCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToBestCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToBestCity::Clone() const
{
	return new AuMissionOrderGoToBestCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToBestCity::ListDescription() const
{
	return QObject::tr("Land at the best airport.");
}

QString AuMissionOrderGoToBestCity::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderGoToBestCity::ListName() const
{
	return QObject::tr("Land at Best Airport");
}

QString AuMissionOrderGoToBestCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToBestCity::ListToolTip() const
{
	return QObject::tr("<html>Land at the best airport. "
			"The best airport is at the city with the highest population, "
			"preferably with a repair shop capable of repairing the spacecraft. "
			"A preference is made for empire, friend then neutral airports.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToBestCity::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world with the best airport. Land us there %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToBestStation::AuMissionOrderGoToBestStation(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToBestStation::AuMissionOrderGoToBestStation(const AuMissionOrderGoToBestStation& c) : AuMissionOrder(c)
{
}

void AuMissionOrderGoToBestStation::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToBestStation::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToBestStation::Clone() const
{
	return new AuMissionOrderGoToBestStation(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToBestStation::ListDescription() const
{
	return QObject::tr("Move near the best station.");
}

QString AuMissionOrderGoToBestStation::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderGoToBestStation::ListName() const
{
	return QObject::tr("Move to Best Station");
}

QString AuMissionOrderGoToBestStation::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToBestStation::ListToolTip() const
{
	return QObject::tr("<html>Move to the best station. "
			"The best station is the one with the highest quality and the newest design. "
			"A preference is made for empire, friend then neutral stations.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToBestStation::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world orbited by the best station. Park us next to the station %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToBestStationOrCity::AuMissionOrderGoToBestStationOrCity(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToBestStationOrCity::AuMissionOrderGoToBestStationOrCity(const AuMissionOrderGoToBestStationOrCity& c) :
	AuMissionOrder(c)
{
}

void AuMissionOrderGoToBestStationOrCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToBestStationOrCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToBestStationOrCity::Clone() const
{
	return new AuMissionOrderGoToBestStationOrCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToBestStationOrCity::ListDescription() const
{
	return QObject::tr("Move near the best station or land at the best airport.");
}

QString AuMissionOrderGoToBestStationOrCity::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderGoToBestStationOrCity::ListName() const
{
	return QObject::tr("Move to Best Station or Land at Best Airport");
}

QString AuMissionOrderGoToBestStationOrCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToBestStationOrCity::ListToolTip() const
{
	return QObject::tr("<html>Move to the best station or land at the best airport. "
			"A station is preferred over landing at an airport. "
			"A preference is made for empire, friend then neutral stations and airports.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToBestStationOrCity::Verbalization(const QString& pilot, const QString& navigator,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world orbited by the best station. "
			"Go to the best airport if there's no station. Park us there %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToCity::AuMissionOrderGoToCity(const AuId& cityId, const QString& cityName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_CityId = cityId;
	m_CityName = cityName;
}

AuMissionOrderGoToCity::AuMissionOrderGoToCity()
{
}

AuMissionOrderGoToCity::AuMissionOrderGoToCity(const AuMissionOrderGoToCity& c) : AuMissionOrder(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
}

void AuMissionOrderGoToCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_CityId;
	in >> m_CityName;
}

void AuMissionOrderGoToCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_CityId;
	out << m_CityName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToCity::Clone() const
{
	return new AuMissionOrderGoToCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToCity::ListDescription() const
{
	return QObject::tr("Land at %1 airport.").arg(m_CityName);
}

QString AuMissionOrderGoToCity::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderGoToCity::ListName() const
{
	if(m_CityName.isEmpty())
		return QObject::tr("Land at Airport");
	return QObject::tr("Land at %1 Airport").arg(m_CityName);
}

QString AuMissionOrderGoToCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToCity::ListToolTip() const
{
	return QObject::tr("<html>Land at an airport. "
			"A city with an airport is selected from a list.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToCity::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world that hosts %2. Land us at the airport %3.")
		.arg(navigator)
		.arg(m_CityName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToCity::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_CityId);
	mem += StringMemoryUsage(m_CityName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToCityAnywhere::AuMissionOrderGoToCityAnywhere(const AuId& cityId, const QString& cityName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_CityId = cityId;
	m_CityName = cityName;
}

AuMissionOrderGoToCityAnywhere::AuMissionOrderGoToCityAnywhere()
{
}

AuMissionOrderGoToCityAnywhere::AuMissionOrderGoToCityAnywhere(const AuMissionOrderGoToCityAnywhere& c) : AuMissionOrder(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
}

void AuMissionOrderGoToCityAnywhere::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_CityId;
	in >> m_CityName;
}

void AuMissionOrderGoToCityAnywhere::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_CityId;
	out << m_CityName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToCityAnywhere::Clone() const
{
	return new AuMissionOrderGoToCityAnywhere(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToCityAnywhere::ListDescription() const
{
	return QObject::tr("Land at %1.").arg(m_CityName);
}

QString AuMissionOrderGoToCityAnywhere::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderGoToCityAnywhere::ListName() const
{
	if(m_CityName.isEmpty())
		return QObject::tr("Land at City");
	return QObject::tr("Land at %1.").arg(m_CityName);
}

QString AuMissionOrderGoToCityAnywhere::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToCityAnywhere::ListToolTip() const
{
	return QObject::tr("<html>Land in or near a city. "
			"A destination city or base is selected from a list. "
			"Pilot lands at a spacecraft parking spot at a building, on a road slab, or on any road of the destination. "
			"<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToCityAnywhere::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world that hosts %2. Land us at the city %3, preferably on pavement.")
		.arg(navigator)
		.arg(m_CityName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToCityAnywhere::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_CityId);
	mem += StringMemoryUsage(m_CityName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToClosestCity::AuMissionOrderGoToClosestCity(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToClosestCity::AuMissionOrderGoToClosestCity(const AuMissionOrderGoToClosestCity& c) : AuMissionOrder(c)
{
}

void AuMissionOrderGoToClosestCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToClosestCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToClosestCity::Clone() const
{
	return new AuMissionOrderGoToClosestCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToClosestCity::ListDescription() const
{
	return QObject::tr("Land at the closest airport.");
}

QString AuMissionOrderGoToClosestCity::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderGoToClosestCity::ListName() const
{
	return QObject::tr("Land at Closest Airport");
}

QString AuMissionOrderGoToClosestCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToClosestCity::ListToolTip() const
{
	return QObject::tr("<html>Land at the closest airport. "
			"A preference is made for empire, friend then neutral airports.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToClosestCity::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world with the closest airport. Land us there %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToClosestStation::AuMissionOrderGoToClosestStation(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToClosestStation::AuMissionOrderGoToClosestStation(const AuMissionOrderGoToClosestStation& c) : AuMissionOrder(c)
{
}

void AuMissionOrderGoToClosestStation::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToClosestStation::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToClosestStation::Clone() const
{
	return new AuMissionOrderGoToClosestStation(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToClosestStation::ListDescription() const
{
	return QObject::tr("Move near the closest station.");
}

QString AuMissionOrderGoToClosestStation::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderGoToClosestStation::ListName() const
{
	return QObject::tr("Move to Closest Station");
}

QString AuMissionOrderGoToClosestStation::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToClosestStation::ListToolTip() const
{
	return QObject::tr("<html>Move to the closest station. "
			"A preference is made for empire, friend then neutral stations.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToClosestStation::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world orbited by the closest station. Park us next to the station %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToClosestStationOrCity::AuMissionOrderGoToClosestStationOrCity(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToClosestStationOrCity::AuMissionOrderGoToClosestStationOrCity(const AuMissionOrderGoToClosestStationOrCity& c) :
	AuMissionOrder(c)
{
}

void AuMissionOrderGoToClosestStationOrCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToClosestStationOrCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToClosestStationOrCity::Clone() const
{
	return new AuMissionOrderGoToClosestStationOrCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToClosestStationOrCity::ListDescription() const
{
	return QObject::tr("Move near the closest station or land at the closest airport.");
}

QString AuMissionOrderGoToClosestStationOrCity::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderGoToClosestStationOrCity::ListName() const
{
	return QObject::tr("Move to Closest Station or Land at Closest Airport");
}

QString AuMissionOrderGoToClosestStationOrCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToClosestStationOrCity::ListToolTip() const
{
	return QObject::tr("<html>Move to the closest station or land at the closest airport. "
			"A station is preferred over landing at an airport. "
			"A preference is made for empire, friend then neutral stations and airports.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToClosestStationOrCity::Verbalization(const QString& pilot, const QString& navigator,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world orbited by the closest station. "
			"Go to the closest airport if there's no station. Park us there %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToCoordinate::AuMissionOrderGoToCoordinate(CONTEXT context, float x, float y, float z, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Context = context;
	m_X = x;
	m_Y = y;
	m_Z = z;
}

AuMissionOrderGoToCoordinate::AuMissionOrderGoToCoordinate(CONTEXT context, const AuPoint& loc, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Context = context;
	m_X = loc.x;
	m_Y = loc.y;
	m_Z = loc.z;
}

AuMissionOrderGoToCoordinate::AuMissionOrderGoToCoordinate()
{
	m_Context = 0xff;					// Invalid so default constructor produces default ListName.
	m_X = 0;
	m_Y = 0;
	m_Z = 0;
}

AuMissionOrderGoToCoordinate::AuMissionOrderGoToCoordinate(const AuMissionOrderGoToCoordinate& c) : AuMissionOrder(c)
{
	m_Context = c.m_Context;
	m_X = c.m_X;
	m_Y = c.m_Y;
	m_Z = c.m_Z;
}

void AuMissionOrderGoToCoordinate::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Context;
	in >> m_X;
	in >> m_Y;
	in >> m_Z;
}

void AuMissionOrderGoToCoordinate::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Context;
	out << m_X;
	out << m_Y;
	out << m_Z;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToCoordinate::Clone() const
{
	return new AuMissionOrderGoToCoordinate(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToCoordinate::ListDescription() const
{
	switch(DestinationContext())
	{
	case Galaxy:
		return QObject::tr("Travel direct to (%L1, %L2, %L3)pc galactic.")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	case Sector:
		return QObject::tr("Travel direct to (%L1, %L2, %L3)pc relative to the sector.")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	case SolarSystem:
		return QObject::tr("Travel direct to (%L1, %L2, %L3)au relative to the solar system.")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	default:
		return QObject::tr("Travel direct to garbled coordinate.");
	}
}

QString AuMissionOrderGoToCoordinate::ListDescriptionIcon() const
{
	return ":data/Locator.png";
}

QString AuMissionOrderGoToCoordinate::ListName() const
{
	switch(DestinationContext())
	{
	case Galaxy:
		return QObject::tr("Travel Direct to (%L1, %L2, %L3)pc in Galaxy")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	case Sector:
		return QObject::tr("Travel Direct to (%L1, %L2, %L3)pc in Sector")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	case SolarSystem:
		return QObject::tr("Travel Direct to (%L1, %L2, %L3)au in Solar System")
			.arg(m_X, 0,'f',4).arg(m_Y, 0,'f',4).arg(m_Z, 0,'f',4);
	default:
		return QObject::tr("Travel Direct to Coordinate");
	}
}

QString AuMissionOrderGoToCoordinate::ListNameIcon() const
{
	return ":data/RangeGalaxy.png";
}

QString AuMissionOrderGoToCoordinate::ListToolTip() const
{
	return QObject::tr("<html>Travel direct to coordinate. "
			"Coordinate is relative to the current galaxy, sector or solar system. "
			"Navigator plots a course directly to the destination coordinate without using wormholes.<br>"
			"Engineer operates FTL drive if the ship is equipped.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToCoordinate::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	switch(DestinationContext())
	{
	case Galaxy:
		return QObject::tr("%1 plot a course direct to %2 %3 %4 galactic. Best speed %5.")
			.arg(navigator)
			.arg(m_X, 0,'f',2).arg(m_Y, 0,'f',2).arg(m_Z, 0,'f',2)
			.arg(pilot);
	case Sector:
		return QObject::tr("%1 plot a course direct to sector coordinate %2 %3 %4. Best speed %5.")
			.arg(navigator)
			.arg(m_X, 0,'f',2).arg(m_Y, 0,'f',2).arg(m_Z, 0,'f',2)
			.arg(pilot);
	case SolarSystem:
		return QObject::tr("%1 plot a course direct to system coordinate %2 %3 %4. Best speed %5.")
			.arg(navigator)
			.arg(m_X, 0,'f',2).arg(m_Y, 0,'f',2).arg(m_Z, 0,'f',2)
			.arg(pilot);
	default:
		return QObject::tr("Go somewhere.");
	}
}

quint64 AuMissionOrderGoToCoordinate::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Context);
	mem += sizeof(m_X);
	mem += sizeof(m_Y);
	mem += sizeof(m_Z);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToHoverSite::AuMissionOrderGoToHoverSite(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL,
		quint16 altitudeAGL, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AltitudeAGL = altitudeAGL;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_GlobeLL = globeLL;
}

AuMissionOrderGoToHoverSite::AuMissionOrderGoToHoverSite()
{
	m_AltitudeAGL = 0;
}

AuMissionOrderGoToHoverSite::AuMissionOrderGoToHoverSite(const AuMissionOrderGoToHoverSite& c) : AuMissionOrder(c)
{
	m_AltitudeAGL = c.m_AltitudeAGL;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
}

const AuMissionOrderGoToHoverSite& AuMissionOrderGoToHoverSite::operator=(const AuMissionOrderGoToHoverSite& c)
{
	AuMissionOrder::operator=(c);
	m_AltitudeAGL = c.m_AltitudeAGL;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
	return *this;
}

void AuMissionOrderGoToHoverSite::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AltitudeAGL;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_GlobeLL;
}

void AuMissionOrderGoToHoverSite::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AltitudeAGL;
	out << m_BodyId;
	out << m_BodyName;
	out << m_GlobeLL;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToHoverSite::Clone() const
{
	return new AuMissionOrderGoToHoverSite(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToHoverSite::ListDescription() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Hover %L2m above %1.").arg(m_GlobeLL.ToString()).arg(m_AltitudeAGL);
	else
		return QObject::tr("Hover %L3m above %1 on %2.").arg(m_GlobeLL.ToString())
			.arg(m_BodyName).arg(m_AltitudeAGL);
}

QString AuMissionOrderGoToHoverSite::ListDescriptionIcon() const
{
	return ":data/Locator.png";
}

QString AuMissionOrderGoToHoverSite::ListName() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Hover at Coordinates");
	return QObject::tr("Hover at %1").arg(m_BodyName);
}

QString AuMissionOrderGoToHoverSite::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToHoverSite::ListToolTip() const
{
	return QObject::tr("<html>Hover at longitude latitude coordinates. "
				"An altitude above ground/sea level is specified in meters. "
				"The hover coordinates are initially set to your current coordinates. "
				"A planet may be selected. "
				"The current planet is used when no planet is selected.<br>"
				"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToHoverSite::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("%1 hold her steady %L2 meters AGL at %3.")
			.arg(pilot)
			.arg(m_AltitudeAGL)
			.arg(m_GlobeLL.ToString());
	else
		return QObject::tr("%1 plot a course to %2. Hold her steady %L3 meters AGL at %4 %5.")
			.arg(navigator)
			.arg(m_BodyName)
			.arg(m_AltitudeAGL)
			.arg(m_GlobeLL.ToString())
			.arg(pilot);
}

quint64 AuMissionOrderGoToHoverSite::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AltitudeAGL);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_GlobeLL);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToLand::AuMissionOrderGoToLand(const AuId& bodyId, const QString& bodyName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
}

AuMissionOrderGoToLand::AuMissionOrderGoToLand()
{
}

AuMissionOrderGoToLand::AuMissionOrderGoToLand(const AuMissionOrderGoToLand& c) : AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
}

void AuMissionOrderGoToLand::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_BodyId;
	in >> m_BodyName;
}

void AuMissionOrderGoToLand::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_BodyId;
	out << m_BodyName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToLand::Clone() const
{
	return new AuMissionOrderGoToLand(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToLand::ListDescription() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Land on the surface of the planet.");
	else
		return QObject::tr("Land on the surface of %1.").arg(m_BodyName);
}

QString AuMissionOrderGoToLand::ListDescriptionIcon() const
{
	return ":data/World.png";
}

QString AuMissionOrderGoToLand::ListName() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Land on Planet");
	return QObject::tr("Land on %1").arg(m_BodyName);
}

QString AuMissionOrderGoToLand::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToLand::ListToolTip() const
{
	return QObject::tr("<html>Land on a planet. "
			"Pilot choses a landing site that is on land, preferably in day light. "
			"A planet may be selected. "
			"The current planet is used when no planet is selected.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToLand::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("%1 put her down on the planet's surface. Look for a level site in daylight.").arg(pilot);
	else
		return QObject::tr("%1 plot a course to %2. Put us on the planet's surface %3. Look for a level site in daylight.")
			.arg(navigator)
			.arg(m_BodyName)
			.arg(pilot);
}

quint64 AuMissionOrderGoToLand::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToLandingSite::AuMissionOrderGoToLandingSite(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL,
		EXECTYPE execType) : AuMissionOrder(execType)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_GlobeLL = globeLL;
}

AuMissionOrderGoToLandingSite::AuMissionOrderGoToLandingSite()
{
}

AuMissionOrderGoToLandingSite::AuMissionOrderGoToLandingSite(const AuMissionOrderGoToLandingSite& c) : AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
}

const AuMissionOrderGoToLandingSite& AuMissionOrderGoToLandingSite::operator=(const AuMissionOrderGoToLandingSite& c)
{
	AuMissionOrder::operator=(c);
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
	return *this;
}

void AuMissionOrderGoToLandingSite::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_GlobeLL;
}

void AuMissionOrderGoToLandingSite::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_BodyId;
	out << m_BodyName;
	out << m_GlobeLL;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToLandingSite::Clone() const
{
	return new AuMissionOrderGoToLandingSite(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToLandingSite::ListDescription() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Land at %1.").arg(m_GlobeLL.ToString());
	else
		return QObject::tr("Land at %1 on %2.").arg(m_GlobeLL.ToString()).arg(m_BodyName);
}

QString AuMissionOrderGoToLandingSite::ListDescriptionIcon() const
{
	return ":data/Locator.png";
}

QString AuMissionOrderGoToLandingSite::ListName() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Land at Coordinates");
	return QObject::tr("Land on %1").arg(m_BodyName);
}

QString AuMissionOrderGoToLandingSite::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToLandingSite::ListToolTip() const
{
	return QObject::tr("<html>Land at longitude latitude coordinates. "
			"The landing coordinates are initially set to your current coordinates. "
			"A planet may be selected. "
			"The current planet is used when no planet is selected.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToLandingSite::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("%1 set her down at %2.")
			.arg(pilot)
			.arg(m_GlobeLL.ToString());
	else
		return QObject::tr("%1 plot a course to %2. Set her down at %3 %4.")
			.arg(navigator)
			.arg(m_BodyName)
			.arg(m_GlobeLL.ToString())
			.arg(pilot);
}

quint64 AuMissionOrderGoToLandingSite::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_GlobeLL);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToOrbit::AuMissionOrderGoToOrbit(const AuId& bodyId, const QString& bodyName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
}

AuMissionOrderGoToOrbit::AuMissionOrderGoToOrbit()
{
}

AuMissionOrderGoToOrbit::AuMissionOrderGoToOrbit(const AuMissionOrderGoToOrbit& c) : AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
}

void AuMissionOrderGoToOrbit::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_BodyId;
	in >> m_BodyName;
}

void AuMissionOrderGoToOrbit::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_BodyId;
	out << m_BodyName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToOrbit::Clone() const
{
	return new AuMissionOrderGoToOrbit(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToOrbit::ListDescription() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Orbit the current planet or star.");
	else
		return QObject::tr("Orbit %1.").arg(m_BodyName);
}

QString AuMissionOrderGoToOrbit::ListDescriptionIcon() const
{
	return ":data/World.png";
}

QString AuMissionOrderGoToOrbit::ListName() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Orbit Planet or Star");
	return QObject::tr("Orbit %1").arg(m_BodyName);
}

QString AuMissionOrderGoToOrbit::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToOrbit::ListToolTip() const
{
	return QObject::tr("<html>Orbit a planet or a star. "
			"A planet or star may be selected. "
			"The current planet or star is orbited when no planet or star is selected.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToOrbit::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("%1 put us in orbit.")
			.arg(pilot);
	else
		return QObject::tr("%1 plot a course to %2. Give me a standard orbit there %3.")
			.arg(navigator)
			.arg(m_BodyName)
			.arg(pilot);
}

quint64 AuMissionOrderGoToOrbit::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToOrbitBestPlanet::AuMissionOrderGoToOrbitBestPlanet(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderGoToOrbitBestPlanet::AuMissionOrderGoToOrbitBestPlanet(const AuMissionOrderGoToOrbitBestPlanet& c) :
	AuMissionOrder(c)
{
}

void AuMissionOrderGoToOrbitBestPlanet::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderGoToOrbitBestPlanet::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToOrbitBestPlanet::Clone() const
{
	return new AuMissionOrderGoToOrbitBestPlanet(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToOrbitBestPlanet::ListDescription() const
{
	return QObject::tr("Orbit the best planet.");
}

QString AuMissionOrderGoToOrbitBestPlanet::ListDescriptionIcon() const
{
	return ":data/World.png";
}

QString AuMissionOrderGoToOrbitBestPlanet::ListName() const
{
	return QObject::tr("Orbit Best Planet");
}

QString AuMissionOrderGoToOrbitBestPlanet::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToOrbitBestPlanet::ListToolTip() const
{
	return QObject::tr("<html>Orbit the best planet. "
			"The best planet is in the habitable zone and has a breathable atmosphere. "
			"Planets in the inner or outer zones with a breathable atmosphere are secondary. "
			"Planets with unbreathable atmospheres are last.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToOrbitBestPlanet::Verbalization(const QString& pilot, const QString& navigator,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the best planet. Give me a standard orbit there %2.")
		.arg(navigator)
		.arg(pilot);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToOrbitCity::AuMissionOrderGoToOrbitCity(const AuId& cityId, const QString& cityName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_CityId = cityId;
	m_CityName = cityName;
}

AuMissionOrderGoToOrbitCity::AuMissionOrderGoToOrbitCity()
{
}

AuMissionOrderGoToOrbitCity::AuMissionOrderGoToOrbitCity(const AuMissionOrderGoToOrbitCity& c) : AuMissionOrder(c)
{
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
}

void AuMissionOrderGoToOrbitCity::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_CityId;
	in >> m_CityName;
}

void AuMissionOrderGoToOrbitCity::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_CityId;
	out << m_CityName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToOrbitCity::Clone() const
{
	return new AuMissionOrderGoToOrbitCity(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToOrbitCity::ListDescription() const
{
	return QObject::tr("Orbit over the city %1.").arg(m_CityName);
}

QString AuMissionOrderGoToOrbitCity::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderGoToOrbitCity::ListName() const
{
	if(m_CityName.isEmpty())
		return QObject::tr("Orbit Over City");
	return QObject::tr("Orbit over %1").arg(m_CityName);
}

QString AuMissionOrderGoToOrbitCity::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToOrbitCity::ListToolTip() const
{
	return QObject::tr("<html>Orbit over a certain city. "
			"A city is selected.<br>"
			"Navigator plots a course directly overhead the destination city.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToOrbitCity::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world that hosts %2. Orbit over the city %3.")
		.arg(navigator)
		.arg(m_CityName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToOrbitCity::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_CityId);
	mem += StringMemoryUsage(m_CityName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToOrbitCoords::AuMissionOrderGoToOrbitCoords(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL,
		EXECTYPE execType) : AuMissionOrder(execType)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_GlobeLL = globeLL;
}

AuMissionOrderGoToOrbitCoords::AuMissionOrderGoToOrbitCoords()
{
}

AuMissionOrderGoToOrbitCoords::AuMissionOrderGoToOrbitCoords(const AuMissionOrderGoToOrbitCoords& c) : AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
}

const AuMissionOrderGoToOrbitCoords& AuMissionOrderGoToOrbitCoords::operator=(const AuMissionOrderGoToOrbitCoords& c)
{
	AuMissionOrder::operator=(c);
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_GlobeLL = c.m_GlobeLL;
	return *this;
}

void AuMissionOrderGoToOrbitCoords::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_GlobeLL;
}

void AuMissionOrderGoToOrbitCoords::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_BodyId;
	out << m_BodyName;
	out << m_GlobeLL;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToOrbitCoords::Clone() const
{
	return new AuMissionOrderGoToOrbitCoords(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToOrbitCoords::ListDescription() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Orbit at %1.").arg(m_GlobeLL.ToString());
	else
		return QObject::tr("Orbit at %1 on %2.").arg(m_GlobeLL.ToString()).arg(m_BodyName);
}

QString AuMissionOrderGoToOrbitCoords::ListDescriptionIcon() const
{
	return ":data/Locator.png";
}

QString AuMissionOrderGoToOrbitCoords::ListName() const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("Orbit at Coordinates");
	return QObject::tr("Orbit %1 at Coordinates").arg(m_BodyName);
}

QString AuMissionOrderGoToOrbitCoords::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToOrbitCoords::ListToolTip() const
{
	return QObject::tr("<html>Orbit a planet or star at longitude latitude coordinates. "
			"The orbit coordinates are initially set to your current coordinates. "
			"A planet or star may be selected. "
			"The current planet or star is used when nothing is selected.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToOrbitCoords::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_BodyName.isEmpty())
		return QObject::tr("%1 put us in orbit at %2.")
			.arg(pilot)
			.arg(m_GlobeLL.ToString());
	else
		return QObject::tr("%1 plot a course to %2. Give me a standard orbit at %3 %4.")
			.arg(navigator)
			.arg(m_BodyName)
			.arg(m_GlobeLL.ToString())
			.arg(pilot);
}

quint64 AuMissionOrderGoToOrbitCoords::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_GlobeLL);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToStation::AuMissionOrderGoToStation(const AuId& stationId, const QString& stationName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_StationId = stationId;
	m_StationName = stationName;
}

AuMissionOrderGoToStation::AuMissionOrderGoToStation()
{
}

AuMissionOrderGoToStation::AuMissionOrderGoToStation(const AuMissionOrderGoToStation& c) : AuMissionOrder(c)
{
	m_StationId = c.m_StationId;
	m_StationName = c.m_StationName;
}

void AuMissionOrderGoToStation::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_StationId;
	in >> m_StationName;
}

void AuMissionOrderGoToStation::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_StationId;
	out << m_StationName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToStation::Clone() const
{
	return new AuMissionOrderGoToStation(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToStation::ListDescription() const
{
	return QObject::tr("Move near station %1.").arg(m_StationName);
}

QString AuMissionOrderGoToStation::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderGoToStation::ListName() const
{
	if(m_StationName.isEmpty())
		return QObject::tr("Move to Station");
	return QObject::tr("Move near %1").arg(m_StationName);
}

QString AuMissionOrderGoToStation::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderGoToStation::ListToolTip() const
{
	return QObject::tr("<html>Move to a station. "
			"A station is selected from a list.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToStation::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course to the world orbited by %2. Park us next to the station %3.")
		.arg(navigator)
		.arg(m_StationName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToStation::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_StationId);
	mem += StringMemoryUsage(m_StationName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToSystemDirect_Deprecated::AuMissionOrderGoToSystemDirect_Deprecated(const AuId& systemId,
		const QString& systemName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuMissionOrderGoToSystemDirect_Deprecated::AuMissionOrderGoToSystemDirect_Deprecated()
{
}

AuMissionOrderGoToSystemDirect_Deprecated::AuMissionOrderGoToSystemDirect_Deprecated(
		const AuMissionOrderGoToSystemDirect_Deprecated& c) : AuMissionOrder(c)
{
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuMissionOrderGoToSystemDirect_Deprecated::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuMissionOrderGoToSystemDirect_Deprecated::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_SystemId;
	out << m_SystemName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToSystemDirect_Deprecated::Clone() const
{
	return new AuMissionOrderGoToSystemDirect_Deprecated(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToSystemDirect_Deprecated::ListDescription() const
{
	return QObject::tr("Travel direct to %1.").arg(m_SystemName);
}

QString AuMissionOrderGoToSystemDirect_Deprecated::ListDescriptionIcon() const
{
	return ":data/SolarSystem.png";
}

QString AuMissionOrderGoToSystemDirect_Deprecated::ListName() const
{
	if(m_SystemName.isEmpty())
		return QObject::tr("Travel Direct to Solar System");
	return QObject::tr("Travel Direct to %1").arg(m_SystemName);
}

QString AuMissionOrderGoToSystemDirect_Deprecated::ListNameIcon() const
{
	return ":data/Sector.png";
}

QString AuMissionOrderGoToSystemDirect_Deprecated::ListToolTip() const
{
	return QObject::tr("<html>Travel direct to a solar system. "
			"A solar system is selected. "
			"Navigator plots a course directly to the destination solar system without using wormholes.<br>"
			"Engineer operates FTL drive if the ship is equipped.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToSystemDirect_Deprecated::Verbalization(const QString& pilot, const QString& navigator,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course direct to %2. Best speed %3.")
		.arg(navigator)
		.arg(m_SystemName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToSystemDirect_Deprecated::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_SystemId);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToSystemDirect::AuMissionOrderGoToSystemDirect(const AuId& systemId, const QString& systemName, float x, float y,
		float z, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_SystemId = systemId;
	m_SystemName = systemName;
	m_X = x;
	m_Y = y;
	m_Z = z;
}

AuMissionOrderGoToSystemDirect::AuMissionOrderGoToSystemDirect(const AuId& systemId, const QString& systemName,
		const AuPoint& galaxyLoc, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_SystemId = systemId;
	m_SystemName = systemName;
	m_X = galaxyLoc.x;
	m_Y = galaxyLoc.y;
	m_Z = galaxyLoc.z;
}

AuMissionOrderGoToSystemDirect::AuMissionOrderGoToSystemDirect()
{
	m_X = 0;
	m_Y = 0;
	m_Z = 0;
}

AuMissionOrderGoToSystemDirect::AuMissionOrderGoToSystemDirect(const AuMissionOrderGoToSystemDirect& c) : AuMissionOrder(c)
{
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
	m_X = c.m_X;
	m_Y = c.m_Y;
	m_Z = c.m_Z;
}

void AuMissionOrderGoToSystemDirect::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_X;
	in >> m_Y;
	in >> m_Z;
}

void AuMissionOrderGoToSystemDirect::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_SystemId;
	out << m_SystemName;
	out << m_X;
	out << m_Y;
	out << m_Z;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToSystemDirect::Clone() const
{
	return new AuMissionOrderGoToSystemDirect(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToSystemDirect::ListDescription() const
{
	return QObject::tr("Travel direct to %1 at (%L2, %L3, %L4)pc galactic.").arg(m_SystemName).arg(m_X).arg(m_Y).arg(m_Z);
}

QString AuMissionOrderGoToSystemDirect::ListDescriptionIcon() const
{
	return ":data/SolarSystem.png";
}

QString AuMissionOrderGoToSystemDirect::ListName() const
{
	if(m_SystemName.isEmpty())
		return QObject::tr("Travel Direct to Solar System");
	return QObject::tr("Travel Direct to %1").arg(m_SystemName);
}

QString AuMissionOrderGoToSystemDirect::ListNameIcon() const
{
	return ":data/Sector.png";
}

QString AuMissionOrderGoToSystemDirect::ListToolTip() const
{
	return QObject::tr("<html>Travel direct to a solar system. "
			"A solar system is selected. "
			"Navigator plots a course directly to the destination solar system without using wormholes.<br>"
			"Engineer operates FTL drive if the ship is equipped.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToSystemDirect::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("%1 plot a course direct to %2. Best speed %3.")
		.arg(navigator)
		.arg(m_SystemName)
		.arg(pilot);
}

quint64 AuMissionOrderGoToSystemDirect::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_SystemId);
	mem += StringMemoryUsage(m_SystemName);
	mem += sizeof(m_X);
	mem += sizeof(m_Y);
	mem += sizeof(m_Z);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderGoToSystemViaWormholes::AuMissionOrderGoToSystemViaWormholes(const AuId& systemId, const QString& systemName,
		EXECTYPE execType) : AuMissionOrder(execType)
{
	m_SystemId = systemId;
	m_SystemName = systemName;
}

AuMissionOrderGoToSystemViaWormholes::AuMissionOrderGoToSystemViaWormholes()
{
}

AuMissionOrderGoToSystemViaWormholes::AuMissionOrderGoToSystemViaWormholes(const AuMissionOrderGoToSystemViaWormholes& c) :
	AuMissionOrder(c)
{
	m_SystemId = c.m_SystemId;
	m_SystemName = c.m_SystemName;
}

void AuMissionOrderGoToSystemViaWormholes::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_SystemId;
	in >> m_SystemName;
}

void AuMissionOrderGoToSystemViaWormholes::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_SystemId;
	out << m_SystemName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderGoToSystemViaWormholes::Clone() const
{
	return new AuMissionOrderGoToSystemViaWormholes(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderGoToSystemViaWormholes::ListDescription() const
{
	if(m_SystemId.Galaxy() != GALAXY_Count)
		return QObject::tr("Travel via intergalactic wormhole to %1.").arg(m_SystemName);
	else
		return QObject::tr("Travel via wormholes to %1.").arg(m_SystemName);
}

QString AuMissionOrderGoToSystemViaWormholes::ListDescriptionIcon() const
{
	return ":data/SolarSystem.png";
}

QString AuMissionOrderGoToSystemViaWormholes::ListName() const
{
	if(m_SystemName.isEmpty())
		return QObject::tr("Travel Via Wormholes");
	return QObject::tr("Travel Via Wormholes to %1").arg(m_SystemName);
}

QString AuMissionOrderGoToSystemViaWormholes::ListNameIcon() const
{
	return ":data/SectorWormholes.png";
}

QString AuMissionOrderGoToSystemViaWormholes::ListToolTip() const
{
	return QObject::tr("<html>Travel via wormholes to a solar system. "
			"A solar system is selected. "
			"Navigator plots a course to the destination solar system using wormholes. "
			"No course is set if a wormhole route cannot be determined.<br>"
			"Engineer operates FTL drive if the ship is equipped.<br>"
			"Pilot travels direct when wormhole drive is inoperative.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderGoToSystemViaWormholes::Verbalization(const QString& pilot, const QString& navigator, const QString& engineer,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_SystemId.Galaxy() != GALAXY_Count)
	{
		// Destination is a distant galaxy, via intergalactic wormhole.
		if(engineer.isEmpty())
			return QObject::tr("%1 plot an intergalactic wormhole course to %2. "
					"Don't spaghettify us %4; we only get one shot at this.")
				.arg(navigator)
				.arg(m_SystemName)
				.arg(pilot);
		else
			return QObject::tr("%1 plot an intergalactic wormhole course to %2. "
					"Ready the FTL drive %3. "
					"Don't spaghettify us %4; we only get one shot at this.")
				.arg(navigator)
				.arg(m_SystemName)
				.arg(engineer)
				.arg(pilot);
	}
	else
	{
		if(engineer.isEmpty())
			return QObject::tr("%1 plot a wormhole course to %2. "
					"Prepare to thread the needle %3.")
				.arg(navigator)
				.arg(m_SystemName)
				.arg(pilot);
		else
			return QObject::tr("%1 plot a wormhole course to %2. "
					"Ready the FTL drive %3; try not to cook us. "
					"Prepare to thread the needle %4.")
				.arg(navigator)
				.arg(m_SystemName)
				.arg(engineer)
				.arg(pilot);
	}
}

quint64 AuMissionOrderGoToSystemViaWormholes::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_SystemId);
	mem += StringMemoryUsage(m_SystemName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvest::AuMissionOrderHarvest(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderHarvest::AuMissionOrderHarvest()
{
	m_Minutes = 0;
}

AuMissionOrderHarvest::AuMissionOrderHarvest(const AuMissionOrderHarvest& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderHarvest::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderHarvest::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvest::Clone() const
{
	return new AuMissionOrderHarvest(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvest::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Harvest forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Harvest one minute.");
	else
		return QObject::tr("Harvest %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderHarvest::ListDescriptionIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvest::ListName() const
{
	return QObject::tr("Harvest");
}

QString AuMissionOrderHarvest::ListNameIcon() const
{
	return ":data/mpRunning.png";
}

QString AuMissionOrderHarvest::ListToolTip() const
{
	return QObject::tr("<html>Harvest for a time span, specified in minutes. "
			"Crew members operate harvester bays during this time. "
			"Time span of zero results in endless harvesting. "
			"Pilot holds current position.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvest::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("Harvesters commence resource collection until further notice.");
	else if(m_Minutes == 1)
		return QObject::tr("Harvesters commence resource collection for one minute.");
	else
		return QObject::tr("Harvesters commence resource collection for %1 minutes.").arg(m_Minutes);
}

quint64 AuMissionOrderHarvest::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestAtmosphere::AuMissionOrderHarvestAtmosphere(COMMODITYID commodityId, bool allBays, quint8 whichBay,
		const AuId& bodyId, const QString& bodyName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AllBays = allBays;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_CommodityId = commodityId;
	m_WhichBay = whichBay;
}

AuMissionOrderHarvestAtmosphere::AuMissionOrderHarvestAtmosphere()
{
	m_AllBays = 0;
	m_CommodityId = CID_Invalid;
	m_WhichBay = 0;
}

AuMissionOrderHarvestAtmosphere::AuMissionOrderHarvestAtmosphere(const AuMissionOrderHarvestAtmosphere& c) : AuMissionOrder(c)
{
	m_AllBays = c.m_AllBays;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_CommodityId = c.m_CommodityId;
	m_WhichBay = c.m_WhichBay;
}

void AuMissionOrderHarvestAtmosphere::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AllBays;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_CommodityId;
	if(!m_AllBays)
		in >> m_WhichBay;
	else
		m_WhichBay = 0;
}

void AuMissionOrderHarvestAtmosphere::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AllBays;
	out << m_BodyId;
	out << m_BodyName;
	out << m_CommodityId;
	if(!m_AllBays)
		out << m_WhichBay;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestAtmosphere::Clone() const
{
	return new AuMissionOrderHarvestAtmosphere(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestAtmosphere::ListDescription() const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure all atmosphere condenser bays to gather %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);

		return QObject::tr("Configure all condenser bays to gather %1 from atmosphere.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure atmosphere condenser bay %3 to gather %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName)
				.arg(m_WhichBay+1);

		return QObject::tr("Configure condenser bay %3 to gather %1 from atmosphere.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_WhichBay+1);
	}
}

QString AuMissionOrderHarvestAtmosphere::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderHarvestAtmosphere::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Harvest Atmosphere");

	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvest %1 From %2")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_BodyName);

	return QObject::tr("Harvest %1 From Atmosphere")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderHarvestAtmosphere::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestAtmosphere::ListToolTip() const
{
	return QObject::tr("<html>Configures an atmosphere condenser bay to harvest materials from the atmosphere of a planet or "
			"gas giant. "
			"A world may be selected; this does not set a destination for the spacecraft. "
			"The closest atmosphere in range is harvested when nothing is selected. "
			"Harvesting is performed during the time-based harvest order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestAtmosphere::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bays configure all atmosphere condensers to gather %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bays configure all atmosphere condensers to gather %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bay %1 configure your atmosphere condenser to gather %2 from %3.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bay %1 configure your atmosphere condenser to gather %2.")
			.arg(m_WhichBay+1)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
}

quint64 AuMissionOrderHarvestAtmosphere::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AllBays);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_WhichBay);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestMinerals::AuMissionOrderHarvestMinerals(COMMODITYID commodityId, bool allBays, quint8 whichBay,
		const AuId& bodyId, const QString& bodyName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AllBays = allBays;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_CommodityId = commodityId;
	m_WhichBay = whichBay;
}

AuMissionOrderHarvestMinerals::AuMissionOrderHarvestMinerals()
{
	m_AllBays = 0;
	m_CommodityId = CID_Invalid;
	m_WhichBay = 0;
}

AuMissionOrderHarvestMinerals::AuMissionOrderHarvestMinerals(const AuMissionOrderHarvestMinerals& c) : AuMissionOrder(c)
{
	m_AllBays = c.m_AllBays;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_CommodityId = c.m_CommodityId;
	m_WhichBay = c.m_WhichBay;
}

void AuMissionOrderHarvestMinerals::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AllBays;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_CommodityId;
	if(!m_AllBays)
		in >> m_WhichBay;
	else
		m_WhichBay = 0;
}

void AuMissionOrderHarvestMinerals::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AllBays;
	out << m_BodyId;
	out << m_BodyName;
	out << m_CommodityId;
	if(!m_AllBays)
		out << m_WhichBay;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestMinerals::Clone() const
{
	return new AuMissionOrderHarvestMinerals(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestMinerals::ListDescription() const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure all prospector bays to mine %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);

		return QObject::tr("Configure all prospector bays to mine %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure prospector bay %3 to mine %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName)
				.arg(m_WhichBay+1);

		return QObject::tr("Configure prospector bay %3 to mine %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_WhichBay+1);
	}
}

QString AuMissionOrderHarvestMinerals::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderHarvestMinerals::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Harvest Minerals");

	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvest %1 From %2")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_BodyName);

	return QObject::tr("Harvest %1")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderHarvestMinerals::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestMinerals::ListToolTip() const
{
	return QObject::tr("<html>Configures a prospector bay to harvest materials from a planet, moon, ringworld or ring. "
			"A planet, moon, ringworld or ring may be selected; this does not set a destination for the spacecraft. "
			"The closest planet, moon, ringworld or ring in range is harvested when nothing is selected. "
			"Harvesting is performed during the time-based harvest order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestMinerals::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bays configure all prospectors to mine %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bays configure all prospectors to mine %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bay %1 configure your prospector to mine %2 from %3.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bay %1 configure your prospector to mine %2.")
			.arg(m_WhichBay+1)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
}

quint64 AuMissionOrderHarvestMinerals::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AllBays);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_WhichBay);
	return mem;
}

bool AuMissionOrderHarvestMinerals::IsValid() const
{
	if(m_CommodityId != CID_Bolite &&
			m_CommodityId != CID_Coal &&
			m_CommodityId != CID_Crystals &&
			m_CommodityId != CID_Eludium &&
			m_CommodityId != CID_Gems &&
			m_CommodityId != CID_GoldOre &&
			m_CommodityId != CID_Ice &&
			m_CommodityId != CID_Lumenite &&
			m_CommodityId != CID_Minerals &&
			m_CommodityId != CID_Ore &&
			m_CommodityId != CID_Radioactives &&
			m_CommodityId != CID_Stone &&
			m_CommodityId != CID_Adamantite &&
			m_CommodityId != CID_Vulcanite)
		return false;
	return AuMissionOrder::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestParticles::AuMissionOrderHarvestParticles(COMMODITYID commodityId, bool allBays, quint8 whichBay,
		const AuId& bodyId, const QString& bodyName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AllBays = allBays;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_CommodityId = commodityId;
	m_WhichBay = whichBay;
}

AuMissionOrderHarvestParticles::AuMissionOrderHarvestParticles()
{
	m_AllBays = 0;
	m_CommodityId = CID_Invalid;
	m_WhichBay = 0;
}

AuMissionOrderHarvestParticles::AuMissionOrderHarvestParticles(const AuMissionOrderHarvestParticles& c) : AuMissionOrder(c)
{
	m_AllBays = c.m_AllBays;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_CommodityId = c.m_CommodityId;
	m_WhichBay = c.m_WhichBay;
}

void AuMissionOrderHarvestParticles::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AllBays;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_CommodityId;
	if(!m_AllBays)
		in >> m_WhichBay;
	else
		m_WhichBay = 0;
}

void AuMissionOrderHarvestParticles::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AllBays;
	out << m_BodyId;
	out << m_BodyName;
	out << m_CommodityId;
	if(!m_AllBays)
		out << m_WhichBay;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestParticles::Clone() const
{
	return new AuMissionOrderHarvestParticles(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestParticles::ListDescription() const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure all particle collector bays to attract %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName);

		if((COMMODITYID)m_CommodityId == CID_AntifluxParticles)
			return QObject::tr("Configure all particle collector bays to attract %1 from ring.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
		else if((COMMODITYID)m_CommodityId == CID_BorexinoPrecipitate)
			return QObject::tr("Configure all particle collector bays to attract %1 from gas giant.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));

		QString cidName;
		if(DbCommodity::IsPreon((COMMODITYID)m_CommodityId))
			cidName = QObject::tr("preons");
		else
			cidName = g_Au.CommodityName((COMMODITYID)m_CommodityId);
		return QObject::tr("Configure all particle collector bays to attract %1 from sun.")
			.arg(cidName);
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure particle collector bay %3 to attract %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName).arg(m_WhichBay+1);

		if((COMMODITYID)m_CommodityId == CID_AntifluxParticles)
			return QObject::tr("Configure particle collector bay %3 to attract %1 from ring.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_WhichBay+1);
		else if((COMMODITYID)m_CommodityId == CID_BorexinoPrecipitate)
			return QObject::tr("Configure particle collector bay %3 to attract %1 from gas giant.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_WhichBay+1);

		QString cidName;
		if(DbCommodity::IsPreon((COMMODITYID)m_CommodityId))
			cidName = QObject::tr("preons");
		else
			cidName = g_Au.CommodityName((COMMODITYID)m_CommodityId);
		return QObject::tr("Configure particle collector bay %3 to attract %1 from sun.")
			.arg(cidName).arg(m_WhichBay+1);
	}
}

QString AuMissionOrderHarvestParticles::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderHarvestParticles::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Harvest Particles");

	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvest %1 From %2").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName);

	if((COMMODITYID)m_CommodityId == CID_AntifluxParticles)
		return QObject::tr("Harvest %1 From Ring").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	else if((COMMODITYID)m_CommodityId == CID_BorexinoPrecipitate)
		return QObject::tr("Harvest %1 From Gas Giant").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	return QObject::tr("Harvest %1 From Sun").arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderHarvestParticles::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestParticles::ListToolTip() const
{
	return QObject::tr("<html>Configures a particle collector bay to harvest materials from a gas giant, ring or sun. "
			"A gas giant, ring or sun may be selected; this does not set a destination for the spacecraft. "
			"The closest appropriate body in range is harvested when nothing is selected. "
			"Harvesting is performed during the time-based harvest order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestParticles::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bays configure all particle collectors to attract %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);

		if((COMMODITYID)m_CommodityId == CID_AntifluxParticles)
			return QObject::tr("Harvester bays configure all particle collectors to attract %1 from a ring.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
		else if((COMMODITYID)m_CommodityId == CID_BorexinoPrecipitate)
			return QObject::tr("Harvester bays configure all particle collectors to attract %1 from a gas giant.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));

		QString cidName;
		if(DbCommodity::IsPreon((COMMODITYID)m_CommodityId))
			cidName = QObject::tr("preons");
		else
			cidName = g_Au.CommodityName((COMMODITYID)m_CommodityId);
		return QObject::tr("Harvester bays configure all particle collectors to attract %1 from the sun.")
			.arg(cidName);
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bay %1 configure your particle collector to attract %2 from %3.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);

		if((COMMODITYID)m_CommodityId == CID_AntifluxParticles)
			return QObject::tr("Harvester bay %1 configure your particle collector to attract %2 from a ring.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
		else if((COMMODITYID)m_CommodityId == CID_BorexinoPrecipitate)
			return QObject::tr("Harvester bay %1 configure your particle collector to attract %2 from a gas giant.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));

		QString cidName;
		if(DbCommodity::IsPreon((COMMODITYID)m_CommodityId))
			cidName = QObject::tr("preons");
		else
			cidName = g_Au.CommodityName((COMMODITYID)m_CommodityId);
		return QObject::tr("Harvester bay %1 configure your particle collector to attract %2 from the sun.")
			.arg(m_WhichBay+1)
			.arg(cidName);
	}
}

quint64 AuMissionOrderHarvestParticles::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AllBays);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_WhichBay);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestSea::AuMissionOrderHarvestSea(COMMODITYID commodityId, bool allBays, quint8 whichBay,
		const AuId& bodyId, const QString& bodyName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AllBays = allBays;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_CommodityId = commodityId;
	m_WhichBay = whichBay;
}

AuMissionOrderHarvestSea::AuMissionOrderHarvestSea()
{
	m_AllBays = 0;
	m_CommodityId = CID_Invalid;
	m_WhichBay = 0;
}

AuMissionOrderHarvestSea::AuMissionOrderHarvestSea(const AuMissionOrderHarvestSea& c) : AuMissionOrder(c)
{
	m_AllBays = c.m_AllBays;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_CommodityId = c.m_CommodityId;
	m_WhichBay = c.m_WhichBay;
}

void AuMissionOrderHarvestSea::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AllBays;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_CommodityId;
	if(!m_AllBays)
		in >> m_WhichBay;
	else
		m_WhichBay = 0;
}

void AuMissionOrderHarvestSea::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AllBays;
	out << m_BodyId;
	out << m_BodyName;
	out << m_CommodityId;
	if(!m_AllBays)
		out << m_WhichBay;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestSea::Clone() const
{
	return new AuMissionOrderHarvestSea(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestSea::ListDescription() const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure all siphon bays to suck %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName);
		return QObject::tr("Configure all siphon bays to suck up %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure siphon bay %3 to suck %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName).arg(m_WhichBay+1);
		return QObject::tr("Configure siphon bay %3 to suck up %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_WhichBay+1);
	}
}

QString AuMissionOrderHarvestSea::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderHarvestSea::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Harvest Sea Resource");

	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvest %1 From %2")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_BodyName);

	return QObject::tr("Harvest %1")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderHarvestSea::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestSea::ListToolTip() const
{
	return QObject::tr("<html>Configures a siphon bay to harvest materials from the sea of a planet, moon or ringworld. "
			"A planet, moon or ringworld may be selected; this does not set a destination for the spacecraft. "
			"The closest planet, moon or ringworld with seas in range is harvested when nothing is selected. "
			"Harvesting is performed during the time-based harvest order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestSea::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bays configure all siphons to suck %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bays configure all siphons to suck up %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bay %1 configure your siphon to suck %2 from %3.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bay %1 configure your siphon to suck up %2.")
			.arg(m_WhichBay+1)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
}

quint64 AuMissionOrderHarvestSea::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AllBays);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_WhichBay);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestWell::AuMissionOrderHarvestWell(COMMODITYID commodityId, bool allBays, quint8 whichBay,
		const AuId& bodyId, const QString& bodyName, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_AllBays = allBays;
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_CommodityId = commodityId;
	m_WhichBay = whichBay;
}

AuMissionOrderHarvestWell::AuMissionOrderHarvestWell()
{
	m_AllBays = 0;
	m_CommodityId = CID_Invalid;
	m_WhichBay = 0;
}

AuMissionOrderHarvestWell::AuMissionOrderHarvestWell(const AuMissionOrderHarvestWell& c) : AuMissionOrder(c)
{
	m_AllBays = c.m_AllBays;
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_CommodityId = c.m_CommodityId;
	m_WhichBay = c.m_WhichBay;
}

void AuMissionOrderHarvestWell::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_AllBays;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_CommodityId;
	if(!m_AllBays)
		in >> m_WhichBay;
	else
		m_WhichBay = 0;
}

void AuMissionOrderHarvestWell::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_AllBays;
	out << m_BodyId;
	out << m_BodyName;
	out << m_CommodityId;
	if(!m_AllBays)
		out << m_WhichBay;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestWell::Clone() const
{
	return new AuMissionOrderHarvestWell(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestWell::ListDescription() const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure all wildcatter bays to drill %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName);
		return QObject::tr("Configure all wildcatter bays to drill %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Configure wildcatter bay %3 to drill %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_BodyName).arg(m_WhichBay+1);
		return QObject::tr("Configure wildcatter bay %3 to drill %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId)).arg(m_WhichBay+1);
	}
}

QString AuMissionOrderHarvestWell::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderHarvestWell::ListName() const
{
	if(m_CommodityId == CID_Invalid)
		return QObject::tr("Harvest Well Resource");

	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvest %1 From %2")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(m_BodyName);

	return QObject::tr("Harvest %1")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
}

QString AuMissionOrderHarvestWell::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestWell::ListToolTip() const
{
	return QObject::tr("<html>Configures a wildcatter bay to harvest materials from a planet, moon or ringworld. "
			"A planet, moon or ringworld may be selected; this does not set a destination for the spacecraft. "
			"The closest planet, moon or ringworld in range is harvested when nothing is selected. "
			"Harvesting is performed during the time-based harvest order. "
			"<br>"
			"<b>Obsolete. <i>Harvester Operations</i> order recommended instead.</b></html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestWell::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_AllBays)
	{
		// All bays
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bays configure all wildcatters to drill %1 from %2.")
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bays configure all wildcatters to drill %1.")
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
	else
	{
		if(!m_BodyName.isEmpty())
			return QObject::tr("Harvester bay %1 configure your wildcatter to drill %2 from %3.")
				.arg(m_WhichBay+1)
				.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
				.arg(m_BodyName);
		return QObject::tr("Harvester bay %1 configure your wildcatter to drill %2.")
			.arg(m_WhichBay+1)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId));
	}
}

quint64 AuMissionOrderHarvestWell::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_AllBays);
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_CommodityId);
	mem += sizeof(m_WhichBay);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHide::AuMissionOrderHide(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderHide::AuMissionOrderHide()
{
	m_Minutes = 0;
}

AuMissionOrderHide::AuMissionOrderHide(const AuMissionOrderHide& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderHide::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderHide::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHide::Clone() const
{
	return new AuMissionOrderHide(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHide::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Hide in ground clutter forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Hide in ground clutter one minute.");
	else
		return QObject::tr("Hide in ground clutter %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderHide::ListDescriptionIcon() const
{
	return ":data/PostureDefendSelf.png";
}

QString AuMissionOrderHide::ListName() const
{
	return QObject::tr("Hide in Ground Clutter");
}

QString AuMissionOrderHide::ListNameIcon() const
{
	return ":data/mpRunning.png";
}

QString AuMissionOrderHide::ListToolTip() const
{
	return QObject::tr("<html>Hide in ground clutter for a time span, specified in minutes. "
			"Time span of zero results in endless hiding. "
			"Crew rigs spacecraft to hide from sensors: "
			"sensors off, shields off, power plant off, maneuver drive off and FTL drive off. "
			"<b>Hiding in ground clutter only works when the spacecraft is sitting motionless on the ground.</b><br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHide::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("All hands rig your systems for minimum RF signature. "
				"We are going to hide the ship in ground clutter until further notice.");
	else if(m_Minutes == 1)
		return QObject::tr("All hands rig your systems for minimum RF signature. "
				"We are going to hide the ship in ground clutter for one minute.");
	else
		return QObject::tr("All hands rig your systems for minimum RF signature. "
				"We are going to hide the ship in ground clutter for %1 minutes.").arg(m_Minutes);
}

quint64 AuMissionOrderHide::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHoldMission::AuMissionOrderHoldMission(EXECTYPE execType) : AuMissionOrder(execType)
{
}

AuMissionOrderHoldMission::AuMissionOrderHoldMission(const AuMissionOrderHoldMission& c) : AuMissionOrder(c)
{
}

void AuMissionOrderHoldMission::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderHoldMission::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHoldMission::Clone() const
{
	return new AuMissionOrderHoldMission(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHoldMission::ListDescription() const
{
	return QObject::tr("Hold mission.");
}

QString AuMissionOrderHoldMission::ListDescriptionIcon() const
{
	return ":data/mpNotReady.png";
}

QString AuMissionOrderHoldMission::ListName() const
{
	return QObject::tr("Hold Mission");
}

QString AuMissionOrderHoldMission::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderHoldMission::ListToolTip() const
{
	return QObject::tr("<html>Hold mission. "
			"The mission is deactivated.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHoldMission::Verbalization(const QString& /*pilot*/, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Stand fast everyone. %1 belay our course.").arg(navigator);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderLightState::AuMissionOrderLightState(bool headlight, bool navLight, bool beacon, bool strobe, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Headlight = headlight;
	m_NavLight = navLight;
	m_RotatingBeacon = beacon;
	m_Strobe = strobe;
}

AuMissionOrderLightState::AuMissionOrderLightState()
{
	m_Headlight = false;
	m_NavLight = false;
	m_RotatingBeacon = false;
	m_Strobe = false;
}

AuMissionOrderLightState::AuMissionOrderLightState(const AuMissionOrderLightState& c) : AuMissionOrder(c)
{
	m_Headlight = c.m_Headlight;
	m_NavLight = c.m_NavLight;
	m_RotatingBeacon = c.m_RotatingBeacon;
	m_Strobe = c.m_Strobe;
}

void AuMissionOrderLightState::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 lights = 0;
	if(m_Headlight)
		lights |= 0x01;
	if(m_NavLight)
		lights |= 0x02;
	if(m_RotatingBeacon)
		lights |= 0x04;
	if(m_Strobe)
		lights |= 0x08;
	out << lights;
}

void AuMissionOrderLightState::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 lights;
	in >> lights;
	m_Headlight = lights & 0x01;
	m_NavLight = lights & 0x02;
	m_RotatingBeacon = lights & 0x04;
	m_Strobe = lights & 0x08;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderLightState::Clone() const
{
	return new AuMissionOrderLightState(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderLightState::ListDescription() const
{
	QString headlight;
	if(m_Headlight)
		headlight = QObject::tr("on");
	else
		headlight = QObject::tr("off");

	QString navLight;
	if(m_NavLight)
		navLight = QObject::tr("on");
	else
		navLight = QObject::tr("off");

	QString beacon;
	if(m_RotatingBeacon)
		beacon = QObject::tr("on");
	else
		beacon = QObject::tr("off");

	QString strobe;
	if(m_Strobe)
		strobe = QObject::tr("on");
	else
		strobe = QObject::tr("off");

	return QObject::tr("Switch headlight %1, nav lights %2, beacons %3, strobes %4.")
		.arg(headlight)
		.arg(navLight)
		.arg(beacon)
		.arg(strobe);
}

QString AuMissionOrderLightState::ListDescriptionIcon() const
{
	return ":data/Light.png";
}

QString AuMissionOrderLightState::ListName() const
{
	return QObject::tr("Set Lights");
}

QString AuMissionOrderLightState::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderLightState::ListToolTip() const
{
	return QObject::tr("<html>Sets up lights. "
			"The exterior lighting of the ship is configured. "
			"Headlight, nav lights, rotating beacons and strobes are configured individually.<br>"
			"<b>Old style ship designs do not have nav lights, beacons or strobes.</b>"
			"<br>"
			"The captain, an officer or the pilot carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderLightState::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Switching lights.");
}

quint64 AuMissionOrderLightState::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Headlight);
	mem += sizeof(m_NavLight);
	mem += sizeof(m_RotatingBeacon);
	mem += sizeof(m_Strobe);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPassengersLoad::AuMissionOrderPassengersLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
	m_PreBoardMinutes = preBoardMinutes;
}

AuMissionOrderPassengersLoad::AuMissionOrderPassengersLoad()
{
	m_Count = 0;
	m_PreBoardMinutes = 0;
}

AuMissionOrderPassengersLoad::AuMissionOrderPassengersLoad(const AuMissionOrderPassengersLoad& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
	m_PreBoardMinutes = c.m_PreBoardMinutes;
}

void AuMissionOrderPassengersLoad::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
	in >> m_PreBoardMinutes;
}

void AuMissionOrderPassengersLoad::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
	out << m_PreBoardMinutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPassengersLoad::Clone() const
{
	return new AuMissionOrderPassengersLoad(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPassengersLoad::ListDescription() const
{
	QString text;
	if(!m_Count)
		text = QObject::tr("Board passengers.");
	else if(m_Count == 1)
		text = QObject::tr("Board one passenger.");
	else
		text = QObject::tr("Board %1 passengers.").arg(m_Count);

	if(m_PreBoardMinutes)
		text += " " + QObject::tr("Announce boarding %1 minutes prior.").arg(m_PreBoardMinutes);
	
	return text;
}

QString AuMissionOrderPassengersLoad::ListDescriptionIcon() const
{
	return ":data/Passenger.png";
}

QString AuMissionOrderPassengersLoad::ListName() const
{
	return QObject::tr("Board Passengers");
}

QString AuMissionOrderPassengersLoad::ListNameIcon() const
{
	return ":data/MsgLoadColonists.png";
}

QString AuMissionOrderPassengersLoad::ListToolTip() const
{
	return QObject::tr("<html>Board passengers. "
			"The number of passenger berths to load may be specified. "
			"All available passenger berths are offered when no count is specified.<br>"
			"Preboarding time gives players an opportunity to accept the boarding announcement. "
			"At the end of the preboarding time, citizen passengers are loaded if available. "
			"When boarding is complete, passengers are kicked from the roster if they are not aboard the spacecraft.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPassengersLoad::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("I am going to take on a passenger if I can find one. "
				"Be cordial but don't get too friendly; in case we have to eat it.");
	else
		return QObject::tr("I am going to take on some passengers if I can find any. "
				"Be cordial but don't get too friendly; in case we have to eat them.");
}

QString AuMissionOrderPassengersLoad::HailIntentions() const
{
	return QObject::tr("We are boarding passengers.");
}

quint64 AuMissionOrderPassengersLoad::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	mem += sizeof(m_PreBoardMinutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPassengersUnload::AuMissionOrderPassengersUnload(quint8 count, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
}

AuMissionOrderPassengersUnload::AuMissionOrderPassengersUnload()
{
	m_Count = 0;
}

AuMissionOrderPassengersUnload::AuMissionOrderPassengersUnload(const AuMissionOrderPassengersUnload& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
}

void AuMissionOrderPassengersUnload::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
}

void AuMissionOrderPassengersUnload::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPassengersUnload::Clone() const
{
	return new AuMissionOrderPassengersUnload(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPassengersUnload::ListDescription() const
{
	if(!m_Count)
		return QObject::tr("Debark passengers.");
	else if(m_Count == 1)
		return QObject::tr("Debark one passenger.");
	else
		return QObject::tr("Debark %1 passengers.").arg(m_Count);
}

QString AuMissionOrderPassengersUnload::ListDescriptionIcon() const
{
	return ":data/Passenger.png";
}

QString AuMissionOrderPassengersUnload::ListName() const
{
	return QObject::tr("Debark Passengers");
}

QString AuMissionOrderPassengersUnload::ListNameIcon() const
{
	return ":data/MsgUnloadColonists.png";
}

QString AuMissionOrderPassengersUnload::ListToolTip() const
{
	return QObject::tr("<html>Debark passengers. "
			"The number of passengers to unload may be specified. "
			"All passengers are unloaded when no count is specified.<br>"
			"Passengers will not be unloaded to an overpopulated city."
			"<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPassengersUnload::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("We are going to debark a passenger here. "
				"Could someone help it out and make sure it doesn't leave any surprises behind.");
	else
		return QObject::tr("We are going to debark some passengers here. "
				"Someone put on the steward hat and herd them out of here, please.");
}

QString AuMissionOrderPassengersUnload::HailIntentions() const
{
	return QObject::tr("We are debarking passengers.");
}

quint64 AuMissionOrderPassengersUnload::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCryoBerthsLoad::AuMissionOrderCryoBerthsLoad(quint32 count, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
}

AuMissionOrderCryoBerthsLoad::AuMissionOrderCryoBerthsLoad()
{
	m_Count = 0;
}

AuMissionOrderCryoBerthsLoad::AuMissionOrderCryoBerthsLoad(const AuMissionOrderCryoBerthsLoad& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
}

void AuMissionOrderCryoBerthsLoad::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
}

void AuMissionOrderCryoBerthsLoad::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCryoBerthsLoad::Clone() const
{
	return new AuMissionOrderCryoBerthsLoad(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCryoBerthsLoad::ListDescription() const
{
	QString text;
	if(!m_Count)
		text = QObject::tr("Board cryo travellers.");
	else if(m_Count == 1)
		text = QObject::tr("Board one cryo traveller.");
	else
		text = QObject::tr("Board %L1 cryo travellers.").arg(m_Count);
	return text;
}

QString AuMissionOrderCryoBerthsLoad::ListDescriptionIcon() const
{
	return ":data/CryoPassenger.png";
}

QString AuMissionOrderCryoBerthsLoad::ListName() const
{
	return QObject::tr("Board Cryo Travellers");
}

QString AuMissionOrderCryoBerthsLoad::ListNameIcon() const
{
	return ":data/MsgLoadColonists.png";
}

QString AuMissionOrderCryoBerthsLoad::ListToolTip() const
{
	return QObject::tr("<html>Board cryo travellers. "
			"The number of cryo berths to load may be specified. "
			"All available cryo berths are offered when no count is specified.<br>"
			"Cryo berths are loaded with citizens, if any are available. "
			"<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCryoBerthsLoad::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("I am going to find some poor sap to take a cryo berth. "
				"Time to fill the freezer.");
	else
		return QObject::tr("I am going to find some poor saps willing to travel by cryo berth. "
				"Time to fill the freezer.");
}

QString AuMissionOrderCryoBerthsLoad::HailIntentions() const
{
	return QObject::tr("We are loading cryo travellers.");
}

quint64 AuMissionOrderCryoBerthsLoad::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderCryoBerthsUnload::AuMissionOrderCryoBerthsUnload(quint32 count, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
}

AuMissionOrderCryoBerthsUnload::AuMissionOrderCryoBerthsUnload()
{
	m_Count = 0;
}

AuMissionOrderCryoBerthsUnload::AuMissionOrderCryoBerthsUnload(const AuMissionOrderCryoBerthsUnload& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
}

void AuMissionOrderCryoBerthsUnload::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
}

void AuMissionOrderCryoBerthsUnload::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderCryoBerthsUnload::Clone() const
{
	return new AuMissionOrderCryoBerthsUnload(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderCryoBerthsUnload::ListDescription() const
{
	if(!m_Count)
		return QObject::tr("Debark cryo travellers.");
	else if(m_Count == 1)
		return QObject::tr("Debark one cryo traveller.");
	else
		return QObject::tr("Debark %1 cryo travellers.").arg(m_Count);
}

QString AuMissionOrderCryoBerthsUnload::ListDescriptionIcon() const
{
	return ":data/CryoPassenger.png";
}

QString AuMissionOrderCryoBerthsUnload::ListName() const
{
	return QObject::tr("Debark Cryo Travellers");
}

QString AuMissionOrderCryoBerthsUnload::ListNameIcon() const
{
	return ":data/MsgUnloadColonists.png";
}

QString AuMissionOrderCryoBerthsUnload::ListToolTip() const
{
	return QObject::tr("<html>Debark cryo travellers. "
			"The number of cryo travellers to unload may be specified. "
			"All cryo travellers are unloaded when no count is specified.<br>"
			"Cryo travellers will not be unloaded to an overpopulated city."
			"<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderCryoBerthsUnload::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("We are going to unload a cryo traveller here. "
				"Could someone get it out of the freezer and start it thawing.");
	else
		return QObject::tr("We are going to unload some cryo travellers here. "
				"Someone get them out of the freezer and thaw them out.");
}

QString AuMissionOrderCryoBerthsUnload::HailIntentions() const
{
	return QObject::tr("We are unloading cryo travellers.");
}

quint64 AuMissionOrderCryoBerthsUnload::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPatrolCities::AuMissionOrderPatrolCities(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderPatrolCities::AuMissionOrderPatrolCities()
{
	m_Minutes = 0;
}

AuMissionOrderPatrolCities::AuMissionOrderPatrolCities(const AuMissionOrderPatrolCities& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderPatrolCities::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderPatrolCities::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPatrolCities::Clone() const
{
	return new AuMissionOrderPatrolCities(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPatrolCities::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Patrol cities forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Patrol cities for one minute.");
	else
		return QObject::tr("Patrol cities for %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderPatrolCities::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderPatrolCities::ListName() const
{
	return QObject::tr("Patrol Cities");
}

QString AuMissionOrderPatrolCities::ListNameIcon() const
{
	return ":data/MsgSpot.png";
}

QString AuMissionOrderPatrolCities::ListToolTip() const
{
	return QObject::tr("<html>Patrol cities for a time span, specified in minutes. "
			"Time span of zero results in endless patrol.<br>"
			"Each empire city and base with an airport is visited in turn. "
			"Pilot hovers briefly above the capitol at each city, HQ at each base.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPatrolCities::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("We're on city patrol until further notice everybody. "
				"%1 plot a course that will take us to every city capitol and base HQ in turn. "
				"Best speed %2 and try not to blow up the ship in any atmospheres.")
			.arg(navigator)
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("We're on city patrol for one minute everybody, not that we're going to get very far. "
				"%1 plot a course that will take us to every city capitol and base HQ in turn. "
				"Best speed %2 and try not to blow up the ship in any atmospheres.")
			.arg(navigator)
			.arg(pilot);
	else
		return QObject::tr("We're on city patrol for %3 minutes everybody. "
				"%1 plot a course that will take us to every city capitol and base HQ in turn. "
				"Best speed %2 and try not to blow up the ship in any atmospheres.")
			.arg(navigator)
			.arg(pilot)
			.arg(m_Minutes);
}

quint64 AuMissionOrderPatrolCities::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPatrolStations::AuMissionOrderPatrolStations(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderPatrolStations::AuMissionOrderPatrolStations()
{
	m_Minutes = 0;
}

AuMissionOrderPatrolStations::AuMissionOrderPatrolStations(const AuMissionOrderPatrolStations& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderPatrolStations::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderPatrolStations::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPatrolStations::Clone() const
{
	return new AuMissionOrderPatrolStations(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPatrolStations::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Patrol stations forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Patrol stations for one minute.");
	else
		return QObject::tr("Patrol stations for %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderPatrolStations::ListDescriptionIcon() const
{
	return ":data/CommStation.png";
}

QString AuMissionOrderPatrolStations::ListName() const
{
	return QObject::tr("Patrol Stations");
}

QString AuMissionOrderPatrolStations::ListNameIcon() const
{
	return ":data/MsgSpot.png";
}

QString AuMissionOrderPatrolStations::ListToolTip() const
{
	return QObject::tr("<html>Patrol stations for a time span, specified in minutes. "
			"Time span of zero results in endless patrol.<br>"
			"Each empire station is visited in turn. "
			"Pilot holds briefly near each station.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPatrolStations::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("We're on station patrol until further notice everybody. "
				"%1 plot a course that will take us to every station in turn. "
				"Best speed %2; keep us out of the atmosphere.")
			.arg(navigator)
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("We're on station patrol for one minute everybody, not that we're going to get very far. "
				"%1 plot a course that will take us to every station in turn. "
				"Best speed %2; keep us out of the atmosphere.")
			.arg(navigator)
			.arg(pilot);
	else
		return QObject::tr("We're on station patrol for %3 minutes everybody. "
				"%1 plot a course that will take us to every station in turn. "
				"Best speed %2; keep us out of the atmosphere.")
			.arg(navigator)
			.arg(pilot)
			.arg(m_Minutes);
}

quint64 AuMissionOrderPatrolStations::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPatrolSystem::AuMissionOrderPatrolSystem(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderPatrolSystem::AuMissionOrderPatrolSystem()
{
	m_Minutes = 0;
}

AuMissionOrderPatrolSystem::AuMissionOrderPatrolSystem(const AuMissionOrderPatrolSystem& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderPatrolSystem::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderPatrolSystem::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPatrolSystem::Clone() const
{
	return new AuMissionOrderPatrolSystem(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPatrolSystem::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Patrol planets forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Patrol planets for one minute.");
	else
		return QObject::tr("Patrol planets for %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderPatrolSystem::ListDescriptionIcon() const
{
	return ":data/World.png";
}

QString AuMissionOrderPatrolSystem::ListName() const
{
	return QObject::tr("Patrol Planets");
}

QString AuMissionOrderPatrolSystem::ListNameIcon() const
{
	return ":data/MsgSpot.png";
}

QString AuMissionOrderPatrolSystem::ListToolTip() const
{
	return QObject::tr("<html>Patrol planets for a time span, specified in minutes. "
			"Time span of zero results in endless patrol.<br>"
			"Each planet in the solar system that is not a satellite of another planet is visited in turn. "
			"Pilot orbits briefly near each planet.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPatrolSystem::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("We're on system patrol until further notice everybody. "
				"%1 plot a course that will take us to every planet in turn. "
				"Best speed %2; keep us out of their atmospheres.")
			.arg(navigator)
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("We're on system patrol for one minute everybody, not that we're going to get very far. "
				"%1 plot a course that will take us to every planet in turn. "
				"Best speed %2; keep us out of their atmospheres.")
			.arg(navigator)
			.arg(pilot);
	else
		return QObject::tr("We're on system patrol for %3 minutes everybody. "
				"%1 plot a course that will take us to every planet in turn. "
				"Best speed %2; keep us out of their atmospheres.")
			.arg(navigator)
			.arg(pilot)
			.arg(m_Minutes);
}

quint64 AuMissionOrderPatrolSystem::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPatrolWormholes::AuMissionOrderPatrolWormholes(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderPatrolWormholes::AuMissionOrderPatrolWormholes()
{
	m_Minutes = 0;
}

AuMissionOrderPatrolWormholes::AuMissionOrderPatrolWormholes(const AuMissionOrderPatrolWormholes& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderPatrolWormholes::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderPatrolWormholes::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPatrolWormholes::Clone() const
{
	return new AuMissionOrderPatrolWormholes(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPatrolWormholes::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Patrol wormholes forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Patrol wormholes for one minute.");
	else
		return QObject::tr("Patrol wormholes for %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderPatrolWormholes::ListDescriptionIcon() const
{
	return ":data/WormholePositive.bmp";
}

QString AuMissionOrderPatrolWormholes::ListName() const
{
	return QObject::tr("Patrol Wormholes");
}

QString AuMissionOrderPatrolWormholes::ListNameIcon() const
{
	return ":data/MsgSpot.png";
}

QString AuMissionOrderPatrolWormholes::ListToolTip() const
{
	return QObject::tr("<html>Patrol wormholes for a time span, specified in minutes. "
			"Time span of zero results in endless patrol.<br>"
			"Each wormhole is visited in turn. "
			"Pilot holds briefly near each wormhole.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPatrolWormholes::Verbalization(const QString& pilot, const QString& navigator, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("We're on wormhole patrol until further notice everybody. "
				"%1 plot a course that will take us to every wormhole near the sun in turn. "
				"Best speed %2; keep us out of the photosphere.")
			.arg(navigator)
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("We're on wormhole patrol for one minute everybody, not that we're going to get very far. "
				"%1 plot a course that will take us to every wormhole near the sun in turn. "
				"Best speed %2; keep us out of the photosphere.")
			.arg(navigator)
			.arg(pilot);
	else
		return QObject::tr("We're on wormhole patrol for %3 minutes everybody. "
				"%1 plot a course that will take us to every wormhole near the sun in turn. "
				"Best speed %2; keep us out of the photosphere.")
			.arg(navigator)
			.arg(pilot)
			.arg(m_Minutes);
}

quint64 AuMissionOrderPatrolWormholes::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPlunder::AuMissionOrderPlunder(const AuId& targetId, const QString& targetName, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_TargetId = targetId;
	m_TargetName = targetName;
}

AuMissionOrderPlunder::AuMissionOrderPlunder()
{
}

AuMissionOrderPlunder::AuMissionOrderPlunder(const AuMissionOrderPlunder& c) : AuMissionOrder(c)
{
	m_TargetId = c.m_TargetId;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderPlunder::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_TargetId;
	in >> m_TargetName;
}

void AuMissionOrderPlunder::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_TargetId;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPlunder::Clone() const
{
	return new AuMissionOrderPlunder(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPlunder::ListDescription() const
{
	return QObject::tr("Plunder %1.").arg(m_TargetName);
}

QString AuMissionOrderPlunder::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderPlunder::ListName() const
{
	if(m_TargetName.isEmpty())
		return QObject::tr("Plunder Spacecraft");
	return QObject::tr("Plunder %1").arg(m_TargetName);
}

QString AuMissionOrderPlunder::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderPlunder::ListToolTip() const
{
	return QObject::tr("<html>Plunder a spacecraft. "
			"A spacecraft to plunder is selected from a list. "
			"Crew will attack the target until the target stands down. "
			"Crew will not follow the unit through a wormhole. "
			"The captain or an officer transfers money, cargo and passengers from the target.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPlunder::Verbalization(const QString& pilot, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("Battle stations! We are going to plunder %1. Attack them until they submit. "
			"%2 bring us close enough to transfer their cargo and passengers.")
		.arg(m_TargetName)
		.arg(pilot);
}

quint64 AuMissionOrderPlunder::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_TargetId);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPosture::AuMissionOrderPosture(quint8 posture, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Posture = posture;
}

AuMissionOrderPosture::AuMissionOrderPosture()
{
	m_Posture = 0xff;					// Invalid so default constructor produces default ListName.
}

AuMissionOrderPosture::AuMissionOrderPosture(const AuMissionOrderPosture& c) : AuMissionOrder(c)
{
	m_Posture = c.m_Posture;
}

void AuMissionOrderPosture::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Posture;
}

void AuMissionOrderPosture::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Posture;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPosture::Clone() const
{
	return new AuMissionOrderPosture(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPosture::ListDescription() const
{
	switch((CITIZEN_POSTURE)m_Posture)
	{
	case POSTURE_Berserk:
		return QObject::tr("Assume berserk posture. Pursue and attack targets in sensor range. Break away on red alert.");
	case POSTURE_Offensive:
		return QObject::tr("Assume offensive posture. Pursue and attack targets in sensor range. Break away on yellow alert.");
	case POSTURE_DefendSelf:
		return QObject::tr("Assume defend self posture. Attack targets in weapon range that have attacked us. "
				"Break away on yellow alert.");
	case POSTURE_DefendArea:
		return QObject::tr("Assume defend area posture. Attack targets in weapon range. Break away on yellow alert.");
	case POSTURE_HoldFire:
		return QObject::tr("Assume hold fire posture. Do not attack any targets.");
	}
	return "";
}

QString AuMissionOrderPosture::ListDescriptionIcon() const
{
	return PostureIcon((CITIZEN_POSTURE)m_Posture);
}

QString AuMissionOrderPosture::ListName() const
{
	switch((CITIZEN_POSTURE)m_Posture)
	{
	case POSTURE_Berserk:
		return QObject::tr("Berserk Posture");
	case POSTURE_Offensive:
		return QObject::tr("Offensive Posture");
	case POSTURE_DefendSelf:
		return QObject::tr("Defend Self Posture");
	case POSTURE_DefendArea:
		return QObject::tr("Defend Area Posture");
	case POSTURE_HoldFire:
		return QObject::tr("Hold Fire Posture");
	}
	return QObject::tr("Assume Posture");
}

QString AuMissionOrderPosture::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderPosture::ListToolTip() const
{
	return QObject::tr("<html>Assume a posture. "
			"Sets posture of officers, crew and troops.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPosture::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	switch((CITIZEN_POSTURE)m_Posture)
	{
	case POSTURE_Berserk:
		return QObject::tr("All hands, time for some bloodlust! Pursue and attack all targets in sensor range. "
				"We're going after them until this ship is blinking red alert.");
	case POSTURE_Offensive:
		return QObject::tr("All hands, we are going on the offensive. Pursue and attack all targets in sensor range. "
				"We'll break away on yellow alert.");
	case POSTURE_DefendSelf:
		return QObject::tr("All hands defend the ship. Return fire on any targets in weapon range that attack the ship; "
				"do not pursue.");
	case POSTURE_DefendArea:
		return QObject::tr("All hands defend this area. Attack any targets that come within weapon range; do not pursue.");
	case POSTURE_HoldFire:
		return QObject::tr("All hands hold your fire. Do not attack any targets.");
	}
	return "";
}

quint64 AuMissionOrderPosture::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Posture);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderRepair::AuMissionOrderRepair(int applyMinQ, quint8 minQ, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_ApplyQ = applyMinQ;
	if(m_ApplyQ > 2)
		m_ApplyQ = 0;
	m_MinQ = minQ;
	if(!m_MinQ)
		m_MinQ = 1;
}

AuMissionOrderRepair::AuMissionOrderRepair(EXECTYPE execType) : AuMissionOrder(execType)
{
	m_ApplyQ = 0;
	m_MinQ = 1;
}

AuMissionOrderRepair::AuMissionOrderRepair(const AuMissionOrderRepair& c) : AuMissionOrder(c)
{
	m_ApplyQ = c.m_ApplyQ;
	m_MinQ = c.m_MinQ;
}

void AuMissionOrderRepair::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_ApplyQ;
	in >> m_MinQ;
}

void AuMissionOrderRepair::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_ApplyQ;
	out << m_MinQ;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderRepair::Clone() const
{
	return new AuMissionOrderRepair(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderRepair::ListDescription() const
{
	QString text = QObject::tr("Rig the spacecraft for repair or refit.");
	switch(m_ApplyQ)
	{
	case 0:
		text += QObject::tr(" Officer repair by system; get parts at module quality or better.");
		break;
	case 1:
		text += QObject::tr(" Officer get parts at spacecraft manufacture quality or better.");
		break;
	case 2:
		text += QObject::tr(" Officer get parts at Q%1 quality or better.").arg(m_MinQ);
		break;
	}
	return text;
}

QString AuMissionOrderRepair::ListDescriptionIcon() const
{
	return ":data/ShipRefit.png";
}

QString AuMissionOrderRepair::ListName() const
{
	return QObject::tr("Rig for Repair or Refit. Officer Get Repairs");
}

QString AuMissionOrderRepair::ListNameIcon() const
{
	return ":data/Damage.png";
}

QString AuMissionOrderRepair::ListToolTip() const
{
	return QObject::tr("<html>Crew, prepare the spacecraft for dry dock repairs or refit. "
			"Officer, repair any damage to the spacecraft.<br>"
			"<b>The mission spacecraft must be positioned where it can be moored; that is near a station or on the ground.</b> "
			"<br>"
			"Crew rigs spacecraft for repairs: spacecraft moored, sensors off, shields off, power plant off, "
			"maneuver drive off and FTL drive off."
			"<br>"
			"The captain or an officer calls for repairs from an airport repair shop, "
			"if the spacecraft is near a station or landed at an airport. "
			"Minimum part quality can be specified. "
			"<br>"
			"Officer skips to the next order before mooring, if the ship does not need repairs.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderRepair::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return QObject::tr("All hands rig for repairs. I will talk to the local shop about fixing the ship. "
			"We could be in dry dock for a while until they're done.");
}

// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
QString AuMissionOrderRepair::HailIntentions() const
{
	return QObject::tr("We need repairs.");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderScanSurveyPlanet::AuMissionOrderScanSurveyPlanet(bool notifyCoC, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_NotifyCoC = notifyCoC;
}

AuMissionOrderScanSurveyPlanet::AuMissionOrderScanSurveyPlanet()
{
	m_NotifyCoC = false;
}

AuMissionOrderScanSurveyPlanet::AuMissionOrderScanSurveyPlanet(const AuMissionOrderScanSurveyPlanet& c) : AuMissionOrder(c)
{
	m_NotifyCoC = c.m_NotifyCoC;
}

void AuMissionOrderScanSurveyPlanet::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_NotifyCoC;
}

void AuMissionOrderScanSurveyPlanet::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_NotifyCoC;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderScanSurveyPlanet::Clone() const
{
	return new AuMissionOrderScanSurveyPlanet(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderScanSurveyPlanet::ListDescription() const
{
	if(m_NotifyCoC)
		return QObject::tr("Survey scan the planet. Report to entire chain-of-command.");
	else
		return QObject::tr("Survey scan the planet. Report to the captain.");
}

QString AuMissionOrderScanSurveyPlanet::ListDescriptionIcon() const
{
	return ":data/World.png";
}

QString AuMissionOrderScanSurveyPlanet::ListName() const
{
	return QObject::tr("Planet Survey Scan");
}

QString AuMissionOrderScanSurveyPlanet::ListNameIcon() const
{
	return ":data/SensorScan.png";
}

QString AuMissionOrderScanSurveyPlanet::ListToolTip() const
{
	return QObject::tr("<html>Survey scan the planet the spacecraft is at using sensors. "
			"Reports information that is on sensors; the mission spacecraft must have sensors. "
			"Mail a report to the captain and optionally everyone in the chain-of-command of the spacecraft.<br>"
			"The sensor operator carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderScanSurveyPlanet::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& sensor, const QString& /*shields*/) const
{
	if(!sensor.isEmpty())
	{
		if(m_NotifyCoC)
			return QObject::tr("%1 conduct a survey scan of the planet. Mail your report to the entire chain-of-command.")
				.arg(sensor);
		else
			return QObject::tr("%1 conduct a survey scan of the planet. Mail it to the captain.")
				.arg(sensor);
	}
	return "";
}

quint64 AuMissionOrderScanSurveyPlanet::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_NotifyCoC);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderScanSurveySystem::AuMissionOrderScanSurveySystem(bool notifyCoC, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_NotifyCoC = notifyCoC;
}

AuMissionOrderScanSurveySystem::AuMissionOrderScanSurveySystem()
{
	m_NotifyCoC = false;
}

AuMissionOrderScanSurveySystem::AuMissionOrderScanSurveySystem(const AuMissionOrderScanSurveySystem& c) : AuMissionOrder(c)
{
	m_NotifyCoC = c.m_NotifyCoC;
}

void AuMissionOrderScanSurveySystem::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_NotifyCoC;
}

void AuMissionOrderScanSurveySystem::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_NotifyCoC;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderScanSurveySystem::Clone() const
{
	return new AuMissionOrderScanSurveySystem(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderScanSurveySystem::ListDescription() const
{
	if(m_NotifyCoC)
		return QObject::tr("Survey scan the solar system. Notify entire chain-of-command when finished.");
	else
		return QObject::tr("Survey scan the solar system.");
}

QString AuMissionOrderScanSurveySystem::ListDescriptionIcon() const
{
	return ":data/SolarSystem.png";
}

QString AuMissionOrderScanSurveySystem::ListName() const
{
	return QObject::tr("Solar System Survey Scan");
}

QString AuMissionOrderScanSurveySystem::ListNameIcon() const
{
	return ":data/SensorScan.png";
}

QString AuMissionOrderScanSurveySystem::ListToolTip() const
{
	return QObject::tr("<html>Survey scan the solar system using sensors. "
			"Reports detailed information to headquarters about the solar sytem, its planets and resources, "
			"for inclusion in the empire's star map archive. "
			"Optionally mails a notification to everyone in the chain-of-command of the spacecraft. "
			"The mail only informs them that a survey scan was performed. "
			"Star map archive data is available to everyone in the empire using the star map.<p>"
			"The mission spacecraft must have sensors. The sensor operator carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderScanSurveySystem::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& sensor, const QString& /*shields*/) const
{
	if(!sensor.isEmpty())
	{
		if(m_NotifyCoC)
			return QObject::tr("%1 conduct a survey scan of the system and transmit the data to headquarters. "
					"Notify the entire chain-of-command when you're finished.")
				.arg(sensor);
		else
			return QObject::tr("%1 conduct a survey scan of the system and transmit the data to headquarters.")
				.arg(sensor);
	}
	return "";
}

quint64 AuMissionOrderScanSurveySystem::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_NotifyCoC);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderScanTactical::AuMissionOrderScanTactical(bool notifyCoC, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_NotifyCoC = notifyCoC;
}

AuMissionOrderScanTactical::AuMissionOrderScanTactical()
{
	m_NotifyCoC = false;
}

AuMissionOrderScanTactical::AuMissionOrderScanTactical(const AuMissionOrderScanTactical& c) : AuMissionOrder(c)
{
	m_NotifyCoC = c.m_NotifyCoC;
}

void AuMissionOrderScanTactical::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_NotifyCoC;
}

void AuMissionOrderScanTactical::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_NotifyCoC;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderScanTactical::Clone() const
{
	return new AuMissionOrderScanTactical(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderScanTactical::ListDescription() const
{
	if(m_NotifyCoC)
		return QObject::tr("Tactical scan units and cities. Report to entire chain-of-command.");
	else
		return QObject::tr("Tactical scan units and cities. Report to the captain.");
}

QString AuMissionOrderScanTactical::ListDescriptionIcon() const
{
	return ":data/City.png";
}

QString AuMissionOrderScanTactical::ListName() const
{
	return QObject::tr("Tactical Scan");
}

QString AuMissionOrderScanTactical::ListNameIcon() const
{
	return ":data/SensorScan.png";
}

QString AuMissionOrderScanTactical::ListToolTip() const
{
	return QObject::tr("<html>Tactical scan units and cities using sensors. "
			"Reports information that is on sensors; the mission spacecraft must have sensors. "
			"Mail a report to the captain and optionally everyone in the chain-of-command of the spacecraft.<br>"
			"The sensor operator carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderScanTactical::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& sensor, const QString& /*shields*/) const
{
	if(!sensor.isEmpty())
	{
		if(m_NotifyCoC)
			return QObject::tr("%1 prepare a tactical report of the units and cities on your scope. "
					"Mail it to the entire chain-of-command.")
				.arg(sensor);
		else
			return QObject::tr("%1 mail a tactical report of the units and cities on your scope to the captain.")
				.arg(sensor);
	}
	return "";
}

quint64 AuMissionOrderScanTactical::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_NotifyCoC);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderScuttle::AuMissionOrderScuttle(quint16 seconds, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Seconds = seconds;
}

AuMissionOrderScuttle::AuMissionOrderScuttle()
{
	m_Seconds = 0;
}

AuMissionOrderScuttle::AuMissionOrderScuttle(const AuMissionOrderScuttle& c) : AuMissionOrder(c)
{
	m_Seconds = c.m_Seconds;
}

void AuMissionOrderScuttle::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Seconds;
}

void AuMissionOrderScuttle::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Seconds;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderScuttle::Clone() const
{
	return new AuMissionOrderScuttle(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderScuttle::ListDescription() const
{
	if(m_Seconds)
		return QObject::tr("Self destruct the spacecraft after %1 seconds.").arg(m_Seconds);
	else
		return QObject::tr("Scuttle the spacecraft immediately.");
}

QString AuMissionOrderScuttle::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderScuttle::ListName() const
{
	return QObject::tr("Self Destruct Spacecraft");
}

QString AuMissionOrderScuttle::ListNameIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderScuttle::ListToolTip() const
{
	return QObject::tr("<html>Scuttle the spacecraft. Initiates the self destruct sequence of the spacecraft. "
			"The spacecraft is scuttled after the destruct time has passed, in seconds. "
			"Time span of zero results in immediate scuttle. "
			"The spacecraft will go to red alert while the self destruct order is active. "
			"The spacecraft carries out this order; no officer or crew is required.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderScuttle::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Seconds)
		return QObject::tr("All hands, the ship is being scuttled. "
				"Self destruct sequence has been activated; "
				"you have %1 seconds to get overboard or go down with the ship.")
			.arg(m_Seconds);
	else
		return QObject::tr("All hands, the ship is being scuttled immediately. It's been nice working with you.");
}

quint64 AuMissionOrderScuttle::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Seconds);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderPilotPolicy::AuMissionOrderPilotPolicy(PILOT_UP_POLICY upPolicy, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_PilotUpPolicy = upPolicy;
}

AuMissionOrderPilotPolicy::AuMissionOrderPilotPolicy()
{
	m_PilotUpPolicy = UpAwayFromGravity;
}

AuMissionOrderPilotPolicy::AuMissionOrderPilotPolicy(const AuMissionOrderPilotPolicy& c) : AuMissionOrder(c)
{
	m_PilotUpPolicy = c.m_PilotUpPolicy;
}

void AuMissionOrderPilotPolicy::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	in >> m_PilotUpPolicy;
}

void AuMissionOrderPilotPolicy::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	out << m_PilotUpPolicy;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderPilotPolicy::Clone() const
{
	return new AuMissionOrderPilotPolicy(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderPilotPolicy::ListDescription() const
{
	switch(m_PilotUpPolicy)
	{
	case UpAwayFromGravity:
	default:
		return QObject::tr("Fly ship upright at all times.");
	case UpTowardGravity:
		return QObject::tr("Fly ship inverted in space.");
	case PortTowardGravity:
		return QObject::tr("Fly ship port side to gravity well.");
	case StarboardTowardGravity:
		return QObject::tr("Fly ship starboard side to gravity well.");
	}
}

QString AuMissionOrderPilotPolicy::ListDescriptionIcon() const
{
	return ":data/pc_AutoMode.png";
}

QString AuMissionOrderPilotPolicy::ListName() const
{
	return QObject::tr("Set Ship Orientation Policy");
}

QString AuMissionOrderPilotPolicy::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderPilotPolicy::ListToolTip() const
{
	return QObject::tr("<html>Sets ship orientation policy. "
			"The flying orientation policy for the pilot is configured. "
			"This can also be configured manually on the Spacecraft Policy window.<br>"
			"This policy tells the pilot which direction to orient the up direction of the ship. "
			"This policy only applies when the ship is outside atmosphere range of a world, "
			"regardless of whether it actually has an atmosphere."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderPilotPolicy::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	switch(m_PilotUpPolicy)
	{
	case UpAwayFromGravity:
	default:
		return QObject::tr("Fly right side up, %1. Keep the dirt below our feet.").arg(pilot);
	case UpTowardGravity:
		return QObject::tr("Fly inverted in space, %1. Give us a nice view overhead.").arg(pilot);
	case PortTowardGravity:
		return QObject::tr("Fly port side to the gravity well, %1. Give us a nice view on the left side.").arg(pilot);
	case StarboardTowardGravity:
		return QObject::tr("Fly starboard side to the gravity well, %1. Give us a nice view on the right side.").arg(pilot);
	}
}

quint64 AuMissionOrderPilotPolicy::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_PilotUpPolicy);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderShieldPolicy::AuMissionOrderShieldPolicy(SHIELD_POLICY shieldPolicy, SHIELDFUEL_POLICY shieldFuelPolicy,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_ShieldFuelPolicy = shieldFuelPolicy;
	m_ShieldPolicy = shieldPolicy;
}

AuMissionOrderShieldPolicy::AuMissionOrderShieldPolicy()
{
	m_ShieldFuelPolicy = 0;
	m_ShieldPolicy = 0;
}

AuMissionOrderShieldPolicy::AuMissionOrderShieldPolicy(const AuMissionOrderShieldPolicy& c) : AuMissionOrder(c)
{
	m_ShieldFuelPolicy = c.m_ShieldFuelPolicy;
	m_ShieldPolicy = c.m_ShieldPolicy;
}

void AuMissionOrderShieldPolicy::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);

	quint8 ss;
	in >> ss;
	m_ShieldFuelPolicy = ss >> 4;
	m_ShieldPolicy = ss & 0x0f;
}

void AuMissionOrderShieldPolicy::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);

	quint8 ss = (m_ShieldFuelPolicy << 4) | m_ShieldPolicy;
	out << ss;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderShieldPolicy::Clone() const
{
	return new AuMissionOrderShieldPolicy(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderShieldPolicy::ListDescription() const
{
	QString fuelrule;
	if(m_ShieldFuelPolicy == ShieldUpIgnoreFuel)
		fuelrule = QObject::tr("regardless of ship fuel status");
	else
		fuelrule = QObject::tr("off if the ship has no fuel");

	switch(m_ShieldPolicy)
	{
	case ShieldUp:							// Up all the time
	default:
		return QObject::tr("Shields up at all times, %1.").arg(fuelrule);
	case ShieldDown:							// Down all the time
		return QObject::tr("Shields down at all times, %1.").arg(fuelrule);
	case ShieldUpIfAnyone:					// Up if any contact
		return QObject::tr("Shields down unless any contacts are detected, %1.").arg(fuelrule);
	case ShieldUpIfForeign:					// Up if any non-empire contact
		return QObject::tr("Shields down unless non-empire contacts are detected, %1.").arg(fuelrule);
	case ShieldUpIfNeutral:					// Up if any neutral or enemy contact
		return QObject::tr("Shields down unless neutral or enemy contacts are detected, %1.").arg(fuelrule);
	case ShieldUpIfEnemy:					// Up if any enemy contact
		return QObject::tr("Shields down unless enemy contacts are detected, %1.").arg(fuelrule);
	}
}

QString AuMissionOrderShieldPolicy::ListDescriptionIcon() const
{
	return ":data/clMShip.png";
}

QString AuMissionOrderShieldPolicy::ListName() const
{
	return QObject::tr("Set Shields Policy");
}

QString AuMissionOrderShieldPolicy::ListNameIcon() const
{
	return ":data/Officer.png";
}

QString AuMissionOrderShieldPolicy::ListToolTip() const
{
	return QObject::tr("<html>Sets shields policy. "
			"The operating policy of the shields is configured.<br>"
			"This policy tells the shield operator what conditions require the shields to be raised, lowered or powered off."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderShieldPolicy::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& shields) const
{
	QString fuelrule;
	if(!m_ShieldFuelPolicy)
		fuelrule = QObject::tr("; shut 'em off if we're out of fuel");

	switch(m_ShieldPolicy)
	{
	case 0:
	default:
		return QObject::tr("Shields up until further notice, %1%2.").arg(shields).arg(fuelrule);
	case 1:
		return QObject::tr("Shields down until further notice, %1%2.").arg(shields).arg(fuelrule);
	case 2:
		return QObject::tr("%1 bring up the shields if any contacts are detected%2.").arg(shields).arg(fuelrule);
	case 3:
		return QObject::tr("%1 bring up the shields if non-empire contacts are detected%2.").arg(shields).arg(fuelrule);
	case 4:
		return QObject::tr("%1 bring up the shields if neutral or enemy contacts are detected%2.").arg(shields).arg(fuelrule);
	case 5:
		return QObject::tr("%1 bring up the shields if enemy contacts are detected%2.").arg(shields).arg(fuelrule);
	}
}

quint64 AuMissionOrderShieldPolicy::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_ShieldFuelPolicy);
	mem += sizeof(m_ShieldPolicy);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderShieldsDown::AuMissionOrderShieldsDown(quint16 seconds, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Seconds = seconds;
}

AuMissionOrderShieldsDown::AuMissionOrderShieldsDown()
{
	m_Seconds = 0;
}

AuMissionOrderShieldsDown::AuMissionOrderShieldsDown(const AuMissionOrderShieldsDown& c) : AuMissionOrder(c)
{
	m_Seconds = c.m_Seconds;
}

void AuMissionOrderShieldsDown::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Seconds;
}

void AuMissionOrderShieldsDown::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Seconds;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderShieldsDown::Clone() const
{
	return new AuMissionOrderShieldsDown(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderShieldsDown::ListDescription() const
{
	if(!m_Seconds)
		return QObject::tr("Shields down forever.");
	else if(m_Seconds == 1)
		return QObject::tr("Shields down one second.");
	else
		return QObject::tr("Shields down %1 seconds.").arg(m_Seconds);
}

QString AuMissionOrderShieldsDown::ListDescriptionIcon() const
{
	return ":data/OrderOpenDoor.png";
}

QString AuMissionOrderShieldsDown::ListName() const
{
	return QObject::tr("Shields Down");
}

QString AuMissionOrderShieldsDown::ListNameIcon() const
{
	return ":data/mpRunning.png";
}

QString AuMissionOrderShieldsDown::ListToolTip() const
{
	return QObject::tr("<html>Wait with shields down for a time span, specified in seconds. "
			"Shields are set down, not off, so they can continue charging. "
			"Time span of zero results in endless wait with shields down. "
			"Pilot holds current position.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderShieldsDown::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& shields) const
{
	if(!shields.isEmpty())
	{
		if(!m_Seconds)
			return QObject::tr("%1 bring the shields down until further notice. Let them charge.").arg(shields);
		else if(m_Seconds == 1)
			return QObject::tr("%1 bring the shields down for one second. Let them charge.").arg(shields);
		else
			return QObject::tr("%1 bring the shields down for %2 seconds. Let them charge.").arg(shields).arg(m_Seconds);
	}
	return "";
}

quint64 AuMissionOrderShieldsDown::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Seconds);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderStandDown::AuMissionOrderStandDown(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderStandDown::AuMissionOrderStandDown()
{
	m_Minutes = 0;
}

AuMissionOrderStandDown::AuMissionOrderStandDown(const AuMissionOrderStandDown& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderStandDown::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderStandDown::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderStandDown::Clone() const
{
	return new AuMissionOrderStandDown(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderStandDown::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Stand down from battle stations forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Stand down from battle stations for one minute.");
	else
		return QObject::tr("Stand down from battle stations for %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderStandDown::ListDescriptionIcon() const
{
	return ":data/PostureHoldFire.png";
}

QString AuMissionOrderStandDown::ListName() const
{
	return QObject::tr("Stand Down");
}

QString AuMissionOrderStandDown::ListNameIcon() const
{
	return ":data/mpRunning.png";
}

QString AuMissionOrderStandDown::ListToolTip() const
{
	return QObject::tr("<html>Stand down from battle stations for a time span, specified in minutes. "
			"Time span of zero results in endless stand-down. "
			"Hull and crew doors are unlocked. "
			"Pilot holds current position. "
			"Crew lowers shields but does not shut them off. "
			"Fire control stations and turret gunners hold fire. "
			"Money, people and cargo become accessible for transfer by all spacecraft nearby.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderStandDown::Verbalization(const QString& pilot, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("All hands stand down and prepare for boarding. Hold our position %1. "
				"Nobody make any provocative actions; we don't need any heroes today.")
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("All hands stand down for one minute; prepare for boarding. Hold our position %1. "
				"Nobody make any provocative actions; we don't need any heroes today.")
			.arg(pilot);
	else
		return QObject::tr("All hands stand down for %1 minutes; prepare for boarding. Hold our position %2. "
				"Nobody make any provocative actions; we don't need any heroes today.")
			.arg(m_Minutes)
			.arg(pilot);
}

quint64 AuMissionOrderStandDown::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderStay::AuMissionOrderStay(quint16 minutes, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Minutes = minutes;
}

AuMissionOrderStay::AuMissionOrderStay()
{
	m_Minutes = 0;
}

AuMissionOrderStay::AuMissionOrderStay(const AuMissionOrderStay& c) : AuMissionOrder(c)
{
	m_Minutes = c.m_Minutes;
}

void AuMissionOrderStay::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Minutes;
}

void AuMissionOrderStay::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Minutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderStay::Clone() const
{
	return new AuMissionOrderStay(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderStay::ListDescription() const
{
	if(!m_Minutes)
		return QObject::tr("Wait forever.");
	else if(m_Minutes == 1)
		return QObject::tr("Wait one minute.");
	else
		return QObject::tr("Wait %1 minutes.").arg(m_Minutes);
}

QString AuMissionOrderStay::ListDescriptionIcon() const
{
	return ":data/OrderStay.png";
}

QString AuMissionOrderStay::ListName() const
{
	return QObject::tr("Wait");
}

QString AuMissionOrderStay::ListNameIcon() const
{
	return ":data/mpRunning.png";
}

QString AuMissionOrderStay::ListToolTip() const
{
	return QObject::tr("<html>Wait for a time span, specified in minutes. "
			"Time span of zero results in endless wait.<br>"
			"Pilot holds current position.<br>"
			"Officer attacks enemy spacecraft in sensor range according to posture while executing this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderStay::Verbalization(const QString& pilot, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_Minutes)
		return QObject::tr("%1 hold our position until further notice.")
			.arg(pilot);
	else if(m_Minutes == 1)
		return QObject::tr("%1 hold our position for one minute.")
			.arg(pilot);
	else
		return QObject::tr("%1 hold our position for %2 minutes.")
			.arg(pilot)
			.arg(m_Minutes);
}

quint64 AuMissionOrderStay::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Minutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTroopsLoad::AuMissionOrderTroopsLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Count = count;
	m_PreBoardMinutes = preBoardMinutes;
}

AuMissionOrderTroopsLoad::AuMissionOrderTroopsLoad()
{
	m_Count = 0;
	m_PreBoardMinutes = 0;
}

AuMissionOrderTroopsLoad::AuMissionOrderTroopsLoad(const AuMissionOrderTroopsLoad& c) : AuMissionOrder(c)
{
	m_Count = c.m_Count;
	m_PreBoardMinutes = c.m_PreBoardMinutes;
}

void AuMissionOrderTroopsLoad::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Count;
	in >> m_PreBoardMinutes;
}

void AuMissionOrderTroopsLoad::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Count;
	out << m_PreBoardMinutes;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTroopsLoad::Clone() const
{
	return new AuMissionOrderTroopsLoad(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTroopsLoad::ListDescription() const
{
	QString text;
	if(!m_Count)
		text = QObject::tr("Board troops.");
	else if(m_Count == 1)
		text = QObject::tr("Board one troop.");
	else
		text = QObject::tr("Board %1 troops.").arg(m_Count);

	if(m_PreBoardMinutes)
		text += " " + QObject::tr("Announce boarding %1 minutes prior.").arg(m_PreBoardMinutes);
	
	return text;
}

QString AuMissionOrderTroopsLoad::ListDescriptionIcon() const
{
	return ":data/Troop.png";
}

QString AuMissionOrderTroopsLoad::ListName() const
{
	return QObject::tr("Board Troops");
}

QString AuMissionOrderTroopsLoad::ListNameIcon() const
{
	return ":data/MsgLoadColonists.png";
}

QString AuMissionOrderTroopsLoad::ListToolTip() const
{
	return QObject::tr("<html>Board troops. "
			"The number of troop berths to load may be specified. "
			"All available troop berths are offered when no count is specified.<br>"
			"Preboarding time gives players an opportunity to accept the boarding announcement. "
			"At the end of the preboarding time, citizen troops are loaded if available. "
			"When boarding is complete, troops are kicked from the roster if they are not aboard the spacecraft.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTroopsLoad::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(m_Count == 1)
		return QObject::tr("I am going to hire a new troop if I can find one. "
				"Don't mess with it; these guys have a rough reputation around here.");
	else
		return QObject::tr("I am going to hire some new troops if I can find any. "
				"Don't mess with them; these guys have a rough reputation around here.");
}

QString AuMissionOrderTroopsLoad::HailIntentions() const
{
	return QObject::tr("We are hiring troops.");
}

quint64 AuMissionOrderTroopsLoad::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Count);
	mem += sizeof(m_PreBoardMinutes);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderExterminateRats::AuMissionOrderExterminateRats(EXECTYPE execType) :
	AuMissionOrder(execType)
{
}

//AuMissionOrderExterminateRats::AuMissionOrderExterminateRats()
//{
//}

AuMissionOrderExterminateRats::AuMissionOrderExterminateRats(const AuMissionOrderExterminateRats& c) : AuMissionOrder(c)
{
}

void AuMissionOrderExterminateRats::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
}

void AuMissionOrderExterminateRats::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderExterminateRats::Clone() const
{
	return new AuMissionOrderExterminateRats(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderExterminateRats::ListDescription() const
{
	return QObject::tr("Hire an Exterminator and a Cleaning Service.");
}

QString AuMissionOrderExterminateRats::ListDescriptionIcon() const
{
	return ":data/Rat.png";
}

QString AuMissionOrderExterminateRats::ListName() const
{
	return QObject::tr("Exterminate Rats and Clean House");
}

QString AuMissionOrderExterminateRats::ListNameIcon() const
{
	return ":data/MsgBuyOffer.png";
}

QString AuMissionOrderExterminateRats::ListToolTip() const
{
	return QObject::tr("<html>Exterminate Rats and Clean House. "
			"An exterminator is hired to remove rats from the ship. "
			"The cost is 100%1 to company ships, free to fleet ships.<br>"
			"A house keeping service is hired to clean up food that attracts rats. "
			"The cost is 100%1 to company ships, free to fleet ships.<br>"
			"The mission spacecraft must be near a station or landed on a road slab.<br>"
			"The captain or an officer carries out this order."
			"</html>").arg(g_Au.CurrencySymbol());
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderExterminateRats::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);
	return QObject::tr("I am calling an exterminator to get these rats out of here. "
			"Then I'll hire a house keeping service to clean up the place. "
			"Don't bother them and do not eat any of them this time, even if they seem lost. ");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferCargoFrom::AuMissionOrderTransferCargoFrom(const QString& targetName, const AuId& auth,
		COMMODITYID commodityId, quint32 commodityAmount, quint8 commodityAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_CommodityAmount = commodityAmount;
	m_CommodityAmountType = commodityAmountType;
	m_CommodityId = commodityId;
	m_TargetName = targetName;
}

AuMissionOrderTransferCargoFrom::AuMissionOrderTransferCargoFrom()
{
	m_CommodityAmount = 0;
	m_CommodityAmountType = 0;
	m_CommodityId = CID_Invalid;
}

AuMissionOrderTransferCargoFrom::AuMissionOrderTransferCargoFrom(const AuMissionOrderTransferCargoFrom& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_CommodityAmount = c.m_CommodityAmount;
	m_CommodityAmountType = c.m_CommodityAmountType;
	m_CommodityId = c.m_CommodityId;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferCargoFrom::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << (quint16)0;	// Invalid 0 amount identifies the version change.
	out << m_CommodityAmountType;
	out << m_CommodityAmount;
	out << m_CommodityId;
	out << m_TargetName;
}

void AuMissionOrderTransferCargoFrom::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Old version 16 bit count.
		in >> m_CommodityAmountType;
		m_CommodityAmount = u16;
	}
	else
	{
		in >> m_CommodityAmountType;
		in >> m_CommodityAmount;
	}
	in >> m_CommodityId;
	in >> m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferCargoFrom::Clone() const
{
	return new AuMissionOrderTransferCargoFrom(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferCargoFrom::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_CommodityAmountType)
		return QObject::tr("Transfer %L1 units of %2 from %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %L1% of the %2 from %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferCargoFrom::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderTransferCargoFrom::ListName() const
{
	return QObject::tr("Transfer Cargo from a Spacecraft");
}

QString AuMissionOrderTransferCargoFrom::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferCargoFrom::ListToolTip() const
{
	return QObject::tr("<html>Transfer Cargo from a Spacecraft. "
			"Transfers cargo from another spacecraft to the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferCargoFrom::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Prepare to transfer a shipment of %1 from %2.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
		.arg(target);
}

QString AuMissionOrderTransferCargoFrom::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 from %2.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
		.arg(target);
}

quint64 AuMissionOrderTransferCargoFrom::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_CommodityAmount);
	mem += sizeof(m_CommodityAmountType);
	mem += sizeof(m_CommodityId);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferCargoTo::AuMissionOrderTransferCargoTo(const QString& targetName, const AuId& auth,
		COMMODITYID commodityId, quint32 commodityAmount, quint8 commodityAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_CommodityAmount = commodityAmount;
	m_CommodityAmountType = commodityAmountType;
	m_CommodityId = commodityId;
	m_TargetName = targetName;
}

AuMissionOrderTransferCargoTo::AuMissionOrderTransferCargoTo()
{
	m_CommodityAmount = 0;
	m_CommodityAmountType = 0;
	m_CommodityId = CID_Invalid;
}

AuMissionOrderTransferCargoTo::AuMissionOrderTransferCargoTo(const AuMissionOrderTransferCargoTo& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_CommodityAmount = c.m_CommodityAmount;
	m_CommodityAmountType = c.m_CommodityAmountType;
	m_CommodityId = c.m_CommodityId;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferCargoTo::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << (quint16)0;	// Invalid 0 amount identifies the version change.
	out << m_CommodityAmountType;
	out << m_CommodityAmount;
	out << m_CommodityId;
	out << m_TargetName;
}

void AuMissionOrderTransferCargoTo::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Old version 16 bit count.
		in >> m_CommodityAmountType;
		m_CommodityAmount = u16;
	}
	else
	{
		in >> m_CommodityAmountType;
		in >> m_CommodityAmount;
	}
	in >> m_CommodityId;
	in >> m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferCargoTo::Clone() const
{
	return new AuMissionOrderTransferCargoTo(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferCargoTo::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_CommodityAmountType)
		return QObject::tr("Transfer %L1 units of %2 to %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %L1% of our %2 to %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferCargoTo::ListDescriptionIcon() const
{
	return g_Au.CommodityBitmapName((COMMODITYID)m_CommodityId);
}

QString AuMissionOrderTransferCargoTo::ListName() const
{
	return QObject::tr("Transfer Cargo to a Spacecraft");
}

QString AuMissionOrderTransferCargoTo::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferCargoTo::ListToolTip() const
{
	return QObject::tr("<html>Transfer Cargo to a Spacecraft. "
			"Transfers cargo to another spacecraft from the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferCargoTo::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Prepare to transfer a shipment of %1 to %2.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
		.arg(target);
}

QString AuMissionOrderTransferCargoTo::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 to %2.")
		.arg(g_Au.CommodityName((COMMODITYID)m_CommodityId))
		.arg(target);
}

quint64 AuMissionOrderTransferCargoTo::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_CommodityAmount);
	mem += sizeof(m_CommodityAmountType);
	mem += sizeof(m_CommodityId);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferCargoCategoryFrom::AuMissionOrderTransferCargoCategoryFrom(const QString& targetName, const AuId& auth,
		COMMODITYFILTER commodityFilter, quint32 commodityAmount, quint8 commodityAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_CommodityAmount = commodityAmount;
	m_CommodityAmountType = commodityAmountType;
	m_CommodityFilter = commodityFilter;
	m_TargetName = targetName;
}

AuMissionOrderTransferCargoCategoryFrom::AuMissionOrderTransferCargoCategoryFrom()
{
	m_CommodityAmount = 0;
	m_CommodityAmountType = 0;
	m_CommodityFilter = 0xff;			// Invalid so default constructor produces default ListName.
}

AuMissionOrderTransferCargoCategoryFrom::AuMissionOrderTransferCargoCategoryFrom(const AuMissionOrderTransferCargoCategoryFrom& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_CommodityAmount = c.m_CommodityAmount;
	m_CommodityAmountType = c.m_CommodityAmountType;
	m_CommodityFilter = c.m_CommodityFilter;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferCargoCategoryFrom::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << (quint16)0;	// Invalid 0 amount identifies the version change.
	out << m_CommodityAmountType;
	out << m_CommodityAmount;
	out << m_CommodityFilter;
	out << m_TargetName;
}

void AuMissionOrderTransferCargoCategoryFrom::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Old version 16 bit count.
		in >> m_CommodityAmountType;
		m_CommodityAmount = u16;
	}
	else
	{
		in >> m_CommodityAmountType;
		in >> m_CommodityAmount;
	}
	in >> m_CommodityFilter;
	in >> m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferCargoCategoryFrom::Clone() const
{
	return new AuMissionOrderTransferCargoCategoryFrom(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferCargoCategoryFrom::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_CommodityAmountType == 0)
		return QObject::tr("Transfer %L1% of the %2 from %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
	else if(m_CommodityAmountType == 1)
		return QObject::tr("Transfer %L1 units of %2 from %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %L1 units of each kind of %2 from %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferCargoCategoryFrom::ListDescriptionIcon() const
{
	return CommodityFilterIcon((COMMODITYFILTER)m_CommodityFilter);
}

QString AuMissionOrderTransferCargoCategoryFrom::ListName() const
{
	if(m_CommodityFilter == 0xff)
		return QObject::tr("Transfer Cargo Category from a Spacecraft");
	return QObject::tr("Transfer %1 from a Spacecraft").arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderTransferCargoCategoryFrom::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferCargoCategoryFrom::ListToolTip() const
{
	return QObject::tr("<html>Transfer Cargo Category from a Spacecraft. "
			"Transfers a category of cargo from another spacecraft to the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferCargoCategoryFrom::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Prepare to transfer a shipment of %1 from %2.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
		.arg(target);
}

QString AuMissionOrderTransferCargoCategoryFrom::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 from %2.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
		.arg(target);
}

quint64 AuMissionOrderTransferCargoCategoryFrom::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_CommodityAmount);
	mem += sizeof(m_CommodityAmountType);
	mem += sizeof(m_CommodityFilter);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferCargoCategoryTo::AuMissionOrderTransferCargoCategoryTo(const QString& targetName, const AuId& auth,
		COMMODITYFILTER commodityFilter, quint32 commodityAmount, quint8 commodityAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_CommodityAmount = commodityAmount;
	m_CommodityAmountType = commodityAmountType;
	m_CommodityFilter = commodityFilter;
	m_TargetName = targetName;
}

AuMissionOrderTransferCargoCategoryTo::AuMissionOrderTransferCargoCategoryTo()
{
	m_CommodityAmount = 0;
	m_CommodityAmountType = 0;
	m_CommodityFilter = 0xff;			// Invalid so default constructor produces default ListName.
}

AuMissionOrderTransferCargoCategoryTo::AuMissionOrderTransferCargoCategoryTo(const AuMissionOrderTransferCargoCategoryTo& c) :
	AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_CommodityAmount = c.m_CommodityAmount;
	m_CommodityAmountType = c.m_CommodityAmountType;
	m_CommodityFilter = c.m_CommodityFilter;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferCargoCategoryTo::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << (quint16)0;	// Invalid 0 amount identifies the version change.
	out << m_CommodityAmountType;
	out << m_CommodityAmount;
	out << m_CommodityFilter;
	out << m_TargetName;
}

void AuMissionOrderTransferCargoCategoryTo::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	quint16 u16;
	in >> u16;
	if(u16)
	{
		// Old version 16 bit count.
		in >> m_CommodityAmountType;
		m_CommodityAmount = u16;
	}
	else
	{
		in >> m_CommodityAmountType;
		in >> m_CommodityAmount;
	}
	in >> m_CommodityFilter;
	in >> m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferCargoCategoryTo::Clone() const
{
	return new AuMissionOrderTransferCargoCategoryTo(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferCargoCategoryTo::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_CommodityAmountType == 0)
		return QObject::tr("Transfer %L1% of our %2 to %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
	else if(m_CommodityAmountType == 1)
		return QObject::tr("Transfer %L1 units of %2 to %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %L1 units of each kind of %2 to %3. Authorization %4.")
			.arg(m_CommodityAmount)
			.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferCargoCategoryTo::ListDescriptionIcon() const
{
	return CommodityFilterIcon((COMMODITYFILTER)m_CommodityFilter);
}

QString AuMissionOrderTransferCargoCategoryTo::ListName() const
{
	if(m_CommodityFilter == 0xff)
		return QObject::tr("Transfer Cargo Category to a Spacecraft");
	return QObject::tr("Transfer %1 from a Spacecraft").arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter));
}

QString AuMissionOrderTransferCargoCategoryTo::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferCargoCategoryTo::ListToolTip() const
{
	return QObject::tr("<html>Transfer Cargo Category to a Spacecraft. "
			"Transfers a category of cargo to another spacecraft from the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferCargoCategoryTo::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Prepare to transfer a shipment of %1 to %2.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
		.arg(target);
}

QString AuMissionOrderTransferCargoCategoryTo::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 to %2.")
		.arg(CommodityFilterName((COMMODITYFILTER)m_CommodityFilter))
		.arg(target);
}

quint64 AuMissionOrderTransferCargoCategoryTo::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_CommodityAmount);
	mem += sizeof(m_CommodityAmountType);
	mem += sizeof(m_CommodityFilter);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferMoneyFrom::AuMissionOrderTransferMoneyFrom(const QString& targetName, const AuId& auth,
		double moneyAmount, quint8 moneyAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_MoneyAmount = moneyAmount;
	m_MoneyAmountType = moneyAmountType;
	m_TargetName = targetName;
}

AuMissionOrderTransferMoneyFrom::AuMissionOrderTransferMoneyFrom()
{
	m_MoneyAmount = 0;
	m_MoneyAmountType = 0;
}

AuMissionOrderTransferMoneyFrom::AuMissionOrderTransferMoneyFrom(const AuMissionOrderTransferMoneyFrom& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_MoneyAmount = c.m_MoneyAmount;
	m_MoneyAmountType = c.m_MoneyAmountType;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferMoneyFrom::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	in >> m_MoneyAmount;
	in >> m_MoneyAmountType;
	in >> m_TargetName;
}

void AuMissionOrderTransferMoneyFrom::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << m_MoneyAmount;
	out << m_MoneyAmountType;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferMoneyFrom::Clone() const
{
	return new AuMissionOrderTransferMoneyFrom(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferMoneyFrom::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_MoneyAmountType)
		return QObject::tr("Transfer %L1%2 from %3's account. Authorization %4.")
			.arg(m_MoneyAmount)
			.arg(g_Au.CurrencySymbol())
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %1% from %2's account. Authorization %3.")
			.arg(m_MoneyAmount)
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferMoneyFrom::ListDescriptionIcon() const
{
	return ":data/c_Money.png";
}

QString AuMissionOrderTransferMoneyFrom::ListName() const
{
	return QObject::tr("Transfer Money from a Spacecraft");
}

QString AuMissionOrderTransferMoneyFrom::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferMoneyFrom::ListToolTip() const
{
	return QObject::tr("<html>Transfer Money from a Spacecraft. "
			"Transfers money from another spacecraft to the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferMoneyFrom::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Hold on while I complete a transfer from %2.")
		.arg(target);
}

QString AuMissionOrderTransferMoneyFrom::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring cargo from %1.")
		.arg(target);
}

quint64 AuMissionOrderTransferMoneyFrom::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_MoneyAmount);
	mem += sizeof(m_MoneyAmountType);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferMoneyTo::AuMissionOrderTransferMoneyTo(const QString& targetName, const AuId& auth,
		double moneyAmount, quint8 moneyAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_MoneyAmount = moneyAmount;
	m_MoneyAmountType = moneyAmountType;
	m_TargetName = targetName;
}

AuMissionOrderTransferMoneyTo::AuMissionOrderTransferMoneyTo()
{
	m_MoneyAmount = 0;
	m_MoneyAmountType = 0;
}

AuMissionOrderTransferMoneyTo::AuMissionOrderTransferMoneyTo(const AuMissionOrderTransferMoneyTo& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_MoneyAmount = c.m_MoneyAmount;
	m_MoneyAmountType = c.m_MoneyAmountType;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferMoneyTo::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	in >> m_MoneyAmount;
	in >> m_MoneyAmountType;
	in >> m_TargetName;
}

void AuMissionOrderTransferMoneyTo::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << m_MoneyAmount;
	out << m_MoneyAmountType;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferMoneyTo::Clone() const
{
	return new AuMissionOrderTransferMoneyTo(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferMoneyTo::ListDescription() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_MoneyAmountType)
		return QObject::tr("Transfer %L1%2 of our account to %3. Authorization %4.")
			.arg(m_MoneyAmount)
			.arg(g_Au.CurrencySymbol())
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %1% of our account to %2. Authorization %3.")
			.arg(m_MoneyAmount)
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferMoneyTo::ListDescriptionIcon() const
{
	return ":data/c_Money.png";
}

QString AuMissionOrderTransferMoneyTo::ListName() const
{
	return QObject::tr("Transfer Money to a Spacecraft");
}

QString AuMissionOrderTransferMoneyTo::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferMoneyTo::ListToolTip() const
{
	return QObject::tr("<html>Transfer Money to a Spacecraft. "
			"Transfers money to another spacecraft from the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferMoneyTo::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("Hold on while I complete a transfer to %1.")
		.arg(target);
}

QString AuMissionOrderTransferMoneyTo::HailIntentions() const
{
	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring cargo to %1.")
		.arg(target);
}

quint64 AuMissionOrderTransferMoneyTo::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_MoneyAmount);
	mem += sizeof(m_MoneyAmountType);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferPeopleFrom::AuMissionOrderTransferPeopleFrom(const QString& targetName, const AuId& auth,
		quint8 peopleType, quint16 peopleAmount, quint8 peopleAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_PeopleAmount = peopleAmount;
	m_PeopleAmountType = peopleAmountType;
	m_PeopleType = peopleType;
	m_TargetName = targetName;
}

AuMissionOrderTransferPeopleFrom::AuMissionOrderTransferPeopleFrom()
{
	m_PeopleAmount = 0;
	m_PeopleAmountType = 0;
	m_PeopleType = 0;
}

AuMissionOrderTransferPeopleFrom::AuMissionOrderTransferPeopleFrom(const AuMissionOrderTransferPeopleFrom& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_PeopleAmount = c.m_PeopleAmount;
	m_PeopleAmountType = c.m_PeopleAmountType;
	m_PeopleType = c.m_PeopleType;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferPeopleFrom::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	in >> m_PeopleAmount;
	in >> m_PeopleAmountType;
	in >> m_PeopleType;
	in >> m_TargetName;
}

void AuMissionOrderTransferPeopleFrom::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << m_PeopleAmount;
	out << m_PeopleAmountType;
	out << m_PeopleType;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferPeopleFrom::Clone() const
{
	return new AuMissionOrderTransferPeopleFrom(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferPeopleFrom::ListDescription() const
{
	const QString types[4] =
	{
		QObject::tr("officers"),
		QObject::tr("crew"),
		QObject::tr("troops"),
		QObject::tr("passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_PeopleAmountType)
		return QObject::tr("Transfer %1 %2 from %3. Authorization %4.")
			.arg(m_PeopleAmount)
			.arg(types[m_PeopleType])
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %1% of the %2 from %3. Authorization %4.")
			.arg(m_PeopleAmount)
			.arg(types[m_PeopleType])
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferPeopleFrom::ListDescriptionIcon() const
{
	const QString types[4] =
	{
		QObject::tr(":data/Officer.png"),
		QObject::tr(":data/Crewman.png"),
		QObject::tr(":data/Troop.png"),
		QObject::tr(":data/Passenger.png"),
	};

	return types[m_PeopleType];
}

QString AuMissionOrderTransferPeopleFrom::ListName() const
{
	return QObject::tr("Transfer People from a Spacecraft");
}

QString AuMissionOrderTransferPeopleFrom::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferPeopleFrom::ListToolTip() const
{
	return QObject::tr("<html>Transfer People from a Spacecraft. "
			"Transfers officers, crew, troops or passengers from another spacecraft to the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferPeopleFrom::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	const QString types[4] =
	{
		QObject::tr("Officers"),
		QObject::tr("Crew"),
		QObject::tr("Troops"),
		QObject::tr("Passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("%1 are boarding from %2. Someone show them to their cabins.")
		.arg(types[m_PeopleType])
		.arg(target);
}

QString AuMissionOrderTransferPeopleFrom::HailIntentions() const
{
	const QString types[4] =
	{
		QObject::tr("officers"),
		QObject::tr("crew"),
		QObject::tr("troops"),
		QObject::tr("passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 from %2.")
		.arg(types[m_PeopleType])
		.arg(target);
}

quint64 AuMissionOrderTransferPeopleFrom::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_PeopleAmount);
	mem += sizeof(m_PeopleAmountType);
	mem += sizeof(m_PeopleType);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderTransferPeopleTo::AuMissionOrderTransferPeopleTo(const QString& targetName, const AuId& auth,
		quint8 peopleType, quint16 peopleAmount, quint8 peopleAmountType,
		EXECTYPE execType) :
	AuMissionOrder(execType)
{
	m_Authorization = auth;
	m_PeopleAmount = peopleAmount;
	m_PeopleAmountType = peopleAmountType;
	m_PeopleType = peopleType;
	m_TargetName = targetName;
}

AuMissionOrderTransferPeopleTo::AuMissionOrderTransferPeopleTo()
{
	m_PeopleAmount = 0;
	m_PeopleAmountType = 0;
	m_PeopleType = 0;
}

AuMissionOrderTransferPeopleTo::AuMissionOrderTransferPeopleTo(const AuMissionOrderTransferPeopleTo& c) : AuMissionOrder(c)
{
	m_Authorization = c.m_Authorization;
	m_PeopleAmount = c.m_PeopleAmount;
	m_PeopleAmountType = c.m_PeopleAmountType;
	m_PeopleType = c.m_PeopleType;
	m_TargetName = c.m_TargetName;
}

void AuMissionOrderTransferPeopleTo::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_Authorization;
	in >> m_PeopleAmount;
	in >> m_PeopleAmountType;
	in >> m_PeopleType;
	in >> m_TargetName;
}

void AuMissionOrderTransferPeopleTo::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_Authorization;
	out << m_PeopleAmount;
	out << m_PeopleAmountType;
	out << m_PeopleType;
	out << m_TargetName;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderTransferPeopleTo::Clone() const
{
	return new AuMissionOrderTransferPeopleTo(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderTransferPeopleTo::ListDescription() const
{
	const QString types[4] =
	{
		QObject::tr("officers"),
		QObject::tr("crew"),
		QObject::tr("troops"),
		QObject::tr("passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	if(m_PeopleAmountType)
		return QObject::tr("Transfer %1 %2 to %3. Authorization %4.")
			.arg(m_PeopleAmount)
			.arg(types[m_PeopleType])
			.arg(target)
			.arg(m_Authorization.StringId());
	else
		return QObject::tr("Transfer %1% of our %2 to %3. Authorization %4.")
			.arg(m_PeopleAmount)
			.arg(types[m_PeopleType])
			.arg(target)
			.arg(m_Authorization.StringId());
}

QString AuMissionOrderTransferPeopleTo::ListDescriptionIcon() const
{
	const QString types[4] =
	{
		QObject::tr(":data/Officer.png"),
		QObject::tr(":data/Crewman.png"),
		QObject::tr(":data/Troop.png"),
		QObject::tr(":data/Passenger.png"),
	};

	return types[m_PeopleType];
}

QString AuMissionOrderTransferPeopleTo::ListName() const
{
	return QObject::tr("Transfer People to a Spacecraft");
}

QString AuMissionOrderTransferPeopleTo::ListNameIcon() const
{
	return ":data/c_Crate.png";
}

QString AuMissionOrderTransferPeopleTo::ListToolTip() const
{
	return QObject::tr("<html>Transfer People to a Spacecraft. "
			"Transfers officers, crew, troops or passengers to another spacecraft from the mission spacecraft. "
			"The target of the transfer can be a station or a named spacecraft.<br>"
			"The mission spacecraft must be near the other spacecraft.<br>"
			"The captain or an officer carries out this order."
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderTransferPeopleTo::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	Q_UNUSED(pilot);

	const QString types[4] =
	{
		QObject::tr("Officers"),
		QObject::tr("Crew"),
		QObject::tr("Troops"),
		QObject::tr("Passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("%1 are debarking to %2. Someone show them the way out.")
		.arg(types[m_PeopleType])
		.arg(target);
}

QString AuMissionOrderTransferPeopleTo::HailIntentions() const
{
	const QString types[4] =
	{
		QObject::tr("officers"),
		QObject::tr("crew"),
		QObject::tr("troops"),
		QObject::tr("passengers"),
	};

	QString target(m_TargetName);
	if(target.isEmpty())
		target = QObject::tr("the station");

	return QObject::tr("We are transferring %1 to %2.")
		.arg(types[m_PeopleType])
		.arg(target);
}

quint64 AuMissionOrderTransferPeopleTo::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_Authorization);
	mem += sizeof(m_PeopleAmount);
	mem += sizeof(m_PeopleAmountType);
	mem += sizeof(m_PeopleType);
	mem += StringMemoryUsage(m_TargetName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestOperationByBay::AuMissionOrderHarvestOperationByBay(const AuId& bodyId, const QString& bodyName,
		quint8 fuelThreshold, const AuHarvestOps& harvestOps, EXECTYPE execType) :
	AuMissionOrder(execType), m_HarvestOps(harvestOps)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_FuelThreshold = fuelThreshold;
}

AuMissionOrderHarvestOperationByBay::AuMissionOrderHarvestOperationByBay()
{
	m_FuelThreshold = 0;
}

AuMissionOrderHarvestOperationByBay::AuMissionOrderHarvestOperationByBay(const AuMissionOrderHarvestOperationByBay& c) :
	AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_FuelThreshold = c.m_FuelThreshold;
	m_HarvestOps = c.m_HarvestOps;
}

void AuMissionOrderHarvestOperationByBay::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_FuelThreshold;
	out << m_BodyId;
	out << m_BodyName;
	out << m_HarvestOps;
}

void AuMissionOrderHarvestOperationByBay::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_FuelThreshold;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_HarvestOps;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestOperationByBay::Clone() const
{
	return new AuMissionOrderHarvestOperationByBay(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestOperationByBay::ListDescription() const
{
	QString desc;
	if(!m_BodyName.isEmpty())
		desc = QObject::tr("Conduct harvesting operations at %1. Fuel threshold %2%.")
			.arg(m_BodyName).arg(m_FuelThreshold);
	else
		desc = QObject::tr("Conduct harvesting operations. Fuel threshold %2%.").arg(m_FuelThreshold);

	for(AuHarvestOps::const_iterator oi = m_HarvestOps.cbegin(); oi != m_HarvestOps.cend(); oi++)
	{
		QString cidName;
		if(DbCommodity::IsPreon((*oi).CommodityId()))
			cidName = QObject::tr("Preons");
		else
			cidName = g_Au.CommodityName((*oi).CommodityId());
		switch((*oi).AmountType())
		{
		case 0:
			desc += QObject::tr("\n  Bay %1: %2, %3% of hold.")
				.arg(oi.key())
				.arg(cidName)
				.arg((*oi).Amount());
			break;
		case 1:
			desc += QObject::tr("\n  Bay %1: %2, %L3 units.")
				.arg(oi.key())
				.arg(cidName)
				.arg((*oi).Amount());
			break;
		case 2:
			break;
		}
	}
	return desc;
}

QString AuMissionOrderHarvestOperationByBay::ListDescriptionIcon() const
{
	return ":data/c_EnergyWeapon.png";
}

QString AuMissionOrderHarvestOperationByBay::ListName() const
{
	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvester Operations at %1 by Bay")
			.arg(m_BodyName);
	else
		return QObject::tr("Harvester Operations by Bay");
}

QString AuMissionOrderHarvestOperationByBay::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestOperationByBay::ListToolTip() const
{
	return QObject::tr("<html>"
			"<p>"
			"Conducts resource harvesting operations from a gas giant, ring or sun. "
			"A gas giant, ring or sun may be selected; this does not set a destination for the spacecraft. "
			"The closest appropriate body in range is harvested when nothing is selected. "
			"Harvesting is performed until the hold is full, fuel drops below the threshold, "
			"or all harvester bay orders are filled."
			"</p>"
			"<p>"
			"When harvesting preons, the type of preon specified in the order is irrelevant. "
			"The star type determines the type of preons actually harvested. "
			"</p>"
			"<p>"
			"Harvest operations are configured for specific bays. "
			"This order can only be issued while aboard a spacecraft, so its bay information is available. "
			"This order can only work for designs with the specified bays. "
			"Multiple bays with the same module can simultaneously harvest different resources using this order. "
			"</p>"
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestOperationByBay::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_BodyName.isEmpty())
		return QObject::tr("%1 hold our position. Harvesters get to work on %2. It's time to earn our keep.")
			.arg(pilot)
			.arg(m_BodyName);
	else
		return QObject::tr("%1 hold our position. Harvesters get to work. It's time to earn our keep.")
			.arg(pilot);
}

quint64 AuMissionOrderHarvestOperationByBay::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_FuelThreshold);
	mem += sizeof(m_HarvestOps);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderHarvestOperationByModule::AuMissionOrderHarvestOperationByModule(const AuId& bodyId, const QString& bodyName,
		quint8 fuelThreshold, const AuHarvestOps& harvestOps, EXECTYPE execType) :
	AuMissionOrder(execType), m_HarvestOps(harvestOps)
{
	m_BodyId = bodyId;
	m_BodyName = bodyName;
	m_FuelThreshold = fuelThreshold;
}

AuMissionOrderHarvestOperationByModule::AuMissionOrderHarvestOperationByModule()
{
	m_FuelThreshold = 0;
}

AuMissionOrderHarvestOperationByModule::AuMissionOrderHarvestOperationByModule(const AuMissionOrderHarvestOperationByModule& c) :
	AuMissionOrder(c)
{
	m_BodyId = c.m_BodyId;
	m_BodyName = c.m_BodyName;
	m_FuelThreshold = c.m_FuelThreshold;
	m_HarvestOps = c.m_HarvestOps;
}

void AuMissionOrderHarvestOperationByModule::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	out << m_FuelThreshold;
	out << m_BodyId;
	out << m_BodyName;
	out << m_HarvestOps;
}

void AuMissionOrderHarvestOperationByModule::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	in >> m_FuelThreshold;
	in >> m_BodyId;
	in >> m_BodyName;
	in >> m_HarvestOps;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderHarvestOperationByModule::Clone() const
{
	return new AuMissionOrderHarvestOperationByModule(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderHarvestOperationByModule::ListDescription() const
{
	QString desc;
	if(!m_BodyName.isEmpty())
		desc = QObject::tr("Conduct harvesting operations at %1. Fuel threshold %2%.")
			.arg(m_BodyName).arg(m_FuelThreshold);
	else
		desc = QObject::tr("Conduct harvesting operations. Fuel threshold %2%.").arg(m_FuelThreshold);

	for(AuHarvestOps::const_iterator oi = m_HarvestOps.cbegin(); oi != m_HarvestOps.cend(); oi++)
	{
		QString cidName;
		if(DbCommodity::IsPreon((*oi).CommodityId()))
			cidName = QObject::tr("Preons");
		else
			cidName = g_Au.CommodityName((*oi).CommodityId());
		switch((*oi).AmountType())
		{
		case 0:
			desc += QObject::tr("\n  %1: %2, %3% of hold.")
				.arg(g_Au.CommodityName((COMMODITYID)oi.key()))
				.arg(cidName)
				.arg((*oi).Amount());
			break;
		case 1:
			desc += QObject::tr("\n  %1: %2, %L3 units.")
				.arg(g_Au.CommodityName((COMMODITYID)oi.key()))
				.arg(cidName)
				.arg((*oi).Amount());
			break;
		case 2:
			break;
		}
	}
	return desc;
}

QString AuMissionOrderHarvestOperationByModule::ListDescriptionIcon() const
{
	return ":data/c_BayModule.png";
}

QString AuMissionOrderHarvestOperationByModule::ListName() const
{
	if(!m_BodyName.isEmpty())
		return QObject::tr("Harvester Operations at %1 by Module")
			.arg(m_BodyName);
	else
		return QObject::tr("Harvester Operations by Module");
}

QString AuMissionOrderHarvestOperationByModule::ListNameIcon() const
{
	return ":data/Harvest.png";
}

QString AuMissionOrderHarvestOperationByModule::ListToolTip() const
{
	return QObject::tr("<html>"
			"<p>"
			"Conducts resource harvesting operations from a gas giant, ring or sun. "
			"A gas giant, ring or sun may be selected; this does not set a destination for the spacecraft. "
			"The closest appropriate body in range is harvested when nothing is selected. "
			"Harvesting is performed until the hold is full, fuel drops below the threshold, "
			"or all harvester bay orders are filled."
			"</p>"
			"<p>"
			"When harvesting preons, the type of preon specified in the order is irrelevant. "
			"The star type determines the type of preons actually harvested. "
			"</p>"
			"<p>"
			"Harvest operations are configured for equipment bays according to the module installed. "
			"The target ship's bay information is not needed. "
			"This order can work for all spacecraft design configurations with harvester bays. "
			"Multiple bays with the same module cannot simultaneously harvest different resources using this order. "
			"</p>"
			"</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderHarvestOperationByModule::Verbalization(const QString& pilot, const QString& /*navigator*/,
		const QString& /*engineer*/, const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	if(!m_BodyName.isEmpty())
		return QObject::tr("%1 hold our position. Harvesters get to work on %2. It's time to earn our keep.")
			.arg(pilot)
			.arg(m_BodyName);
	else
		return QObject::tr("%1 hold our position. Harvesters get to work. It's time to earn our keep.")
			.arg(pilot);
}

quint64 AuMissionOrderHarvestOperationByModule::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += sizeof(m_BodyId);
	mem += StringMemoryUsage(m_BodyName);
	mem += sizeof(m_FuelThreshold);
	mem += m_HarvestOps.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuMissionOrderReport::AuMissionOrderReport(quint16 reportWhat, const QString& sayText, const QString& channel,
		AuCommStdChannel::RANGE range, EXECTYPE execType) : AuMissionOrder(execType)
{
	m_Channel = channel;
	m_Range = range;
	m_ReportWhat = reportWhat;
	m_Text = sayText;
}

AuMissionOrderReport::AuMissionOrderReport()
{
	m_Range = AuCommStdChannel::Universe;
	m_ReportWhat = 0;
}

AuMissionOrderReport::AuMissionOrderReport(const AuMissionOrderReport& c) : AuMissionOrder(c)
{
	m_Channel = c.m_Channel;
	m_Range = c.m_Range;
	m_ReportWhat = c.m_ReportWhat;
	m_Text = c.m_Text;
}

void AuMissionOrderReport::operator>>(QDataStream& out) const
{
	AuMissionOrder::operator>>(out);
	quint16 hasWhat = m_ReportWhat;
	if(!m_Text.isEmpty())
		hasWhat |= 0x8000;
	out << hasWhat;
	out << m_Range;
	out << m_Channel;
	if(hasWhat & 0x8000)
		out << m_Text;
}

void AuMissionOrderReport::operator<<(QDataStream& in)
{
	AuMissionOrder::operator<<(in);
	quint16 hasWhat;
	in >> hasWhat;
	m_ReportWhat = hasWhat & 0x7fff;
	in >> m_Range;
	in >> m_Channel;
	if(hasWhat & 0x8000)
		in >> m_Text;
}

// Returns a duplicate of this mission order, allocated on the heap.
AuMissionOrder* AuMissionOrderReport::Clone() const
{
	return new AuMissionOrderReport(*this);
}

// Returns a description of the mission order.
QString AuMissionOrderReport::ListDescription() const
{
	QString desc = QObject::tr("Report on the %1 channel.").arg(m_Channel);
	if(m_ReportWhat)
	{
		QStringList what;
		if(m_ReportWhat & 0x0001)
			what << QObject::tr("position");
		if(m_ReportWhat & 0x0002)
			what << QObject::tr("crew status");
		if(m_ReportWhat & 0x0004)
			what << QObject::tr("fuel status");
		if(m_ReportWhat & 0x0008)
			what << QObject::tr("cargo status");
		if(m_ReportWhat & 0x0010)
			what << QObject::tr("damage summary");
		if(m_ReportWhat & 0x0020)
			what << QObject::tr("sensor summary");

		if(what.size() == 1)
			desc += QObject::tr(" Include %1.").arg(what.first());
		else if(what.size() == 2)
			desc += QObject::tr(" Include %1 and %2.").arg(what.first()).arg(what.last());
		else
		{
			desc += QObject::tr(" Include ");
			for(QStringList::const_iterator wi = what.cbegin(); wi != what.cend(); wi++)
			{
				if(wi == what.cbegin())
					desc += *wi;
				else if(wi < what.cend() - 1)
					desc += QObject::tr(", %1").arg(*wi);
				else
					desc += QObject::tr(", and %1.").arg(*wi);
			}
		}
	}

	if(!m_Text.isEmpty())
		desc += QObject::tr(" Say, \"%1\"").arg(m_Text);

	return desc;
}

QString AuMissionOrderReport::ListDescriptionIcon() const
{
	return ":data/c_Spacecraft.png";
}

QString AuMissionOrderReport::ListName() const
{
	return QObject::tr("Report");
}

QString AuMissionOrderReport::ListNameIcon() const
{
	return ":data/Comm.png";
}

QString AuMissionOrderReport::ListToolTip() const
{
	return QObject::tr("<html>Report on a comm channel. "
			"The report contains various information as requested by the order. "
			"<br>"
			"An officer carries out this order.</html>");
}

// 'pilot' and 'navigator' are never blank.
// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
QString AuMissionOrderReport::Verbalization(const QString& /*pilot*/, const QString& /*navigator*/, const QString& /*engineer*/,
		const QString& /*power*/, const QString& /*sensor*/, const QString& /*shields*/) const
{
	return "";
}

quint64 AuMissionOrderReport::MemoryUsage() const
{
	quint64 mem = AuMissionOrder::MemoryUsage();
	mem += StringMemoryUsage(m_Channel);
	mem += sizeof(m_Range);
	mem += sizeof(m_ReportWhat);
	mem += StringMemoryUsage(m_Text);
	return mem;
}

bool AuMissionOrderReport::IsValid() const
{
	if(AuCommStdChannel::IsStandardChannel(m_Channel))
	{
		// Some standard channels are off limits to officer reporting.
		if(!AuCommStdChannel::IsOfficerReportChannel(m_Channel))
			return false;	// Standard channel cannot be used for officer reporting channel.
	}

	if(m_Range >= AuCommStdChannel::RANGE_Count)
		return false;

	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
