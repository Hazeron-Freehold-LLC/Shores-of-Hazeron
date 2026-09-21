//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommpacket.cpp,v 1.253 2022/02/19 18:20:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommpacket.h"
//
#include "aucrypt.h"
#include "auglobal.h"
#include "auglobell.h"
#include "auitem.h"
#include "aushipment.h"
#include "dgdata.h"
#include "hashpack.h"

static AuIdList 						s_EmptyIdList;
static QString							s_EmptyString;

AuCommPacket::AuCommPacket(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope)
{
	m_Anonymous = false;
	m_ChannelName = channelName;
	m_FromGalaxy = fromGalaxy;
	m_FromId = fromId;
	m_FromName = fromName;
	m_FromSectorId = fromSector;
	m_RangeRx = range;
	m_RangeTx = range;
	m_Scope = scope;
	m_ScopeData = 0;
}

AuCommPacket::AuCommPacket()
{
	m_Anonymous = false;
	//m_ChannelName
	m_FromGalaxy = GALAXY_Count;
	//m_FromId
	//m_FromName
	//m_FromSectorId
	m_RangeRx = AuCommStdChannel::Close;
	m_RangeTx = AuCommStdChannel::Close;
	m_Scope = AuCommStdChannel::Everyone;
	m_ScopeData = 0;
}

AuCommPacket::~AuCommPacket()
{
}

AuCommPacket::AuCommPacket(const AuCommPacket& c)
{
	m_Anonymous = c.m_Anonymous;
	m_ChannelName = c.m_ChannelName;
	m_FromGalaxy = c.m_FromGalaxy;
	m_FromId = c.m_FromId;
	m_FromName = c.m_FromName;
	m_FromSectorId = c.m_FromSectorId;
	m_RangeRx = c.m_RangeRx;
	m_RangeTx = c.m_RangeTx;
	m_Scope = c.m_Scope;
	m_ScopeData = c.m_ScopeData;
}

void AuCommPacket::operator>>(QDataStream& out) const
{
	// We can save a lot by sending standard channels as just a byte.
	qint8 channelIndex = AuCommStdChannel::ChannelIndex(m_ChannelName);
	out << channelIndex;										// Send the index for standard channels, -1 for non-standard.
	if(channelIndex == -1)
		out << m_ChannelName;									// Non-standard channels need the channel name.

	quint8 ui8 = m_FromGalaxy & 0x1f;
	if(m_Anonymous)
		ui8 |= 0x20;
	if(AuCommStdChannel::ChannelHasRange(m_ChannelName))
		ui8 |= 0x40;
	if(AuCommStdChannel::ScopeHasData(m_Scope))
		ui8 |= 0x80;
	out << ui8;

	out << m_FromId;
	out << m_FromName;
	out << m_FromSectorId;

	if(ui8 & 0x40)
		out << (quint8)((m_RangeTx << 4) | m_RangeRx);

	out << (qint8)m_Scope;
	if(ui8 & 0x80)
		out << m_ScopeData;
}

void AuCommPacket::operator<<(QDataStream& in)
{
	qint8 i8;
	quint8 ui8;

	// Standard channels are sent as an index into the standard channels table.
	in >> i8;
	if(i8 == -1)
		in >> m_ChannelName;									// Non-standard channels send us the channel name.
	else
		m_ChannelName = AuCommStdChannel::ChannelName(i8);		// Standard channels send us the index.

	in >> ui8;
	m_FromGalaxy = (GALAXY)(ui8 & 0x1f);
	m_Anonymous = (ui8 & 0x20) ? true : false;					// Anonymous flag in b7.

	in >> m_FromId;
	in >> m_FromName;
	in >> m_FromSectorId;

	if(ui8 & 0x40)
	{
		quint8 range;
		in >> range;

		m_RangeTx = (AuCommStdChannel::RANGE)(range >> 4);		// Tx range in top nibble.
		m_RangeRx = (AuCommStdChannel::RANGE)(range & 0x0f);		// Rx range in bottom nibble.
	}
	else if(Type() == AuCommPacket::Announcement || Type() == AuCommPacket::Silverwill)
	{
		// Announcements and Silverwill messages are rangeless, always at Universe range.
		m_RangeTx = AuCommStdChannel::Universe;
		m_RangeRx = m_RangeTx;
	}
	else
	{
		m_RangeTx = AuCommStdChannel::Close;					// Rangeless channel range is always Close.
		m_RangeRx = m_RangeTx;
	}

	in >> i8;
	m_Scope = (AuCommStdChannel::SCOPE)i8;
	if(ui8 & 0x80)
		in >> m_ScopeData;
	else
		m_ScopeData = 0;
}

//static
AuCommPacket* AuCommPacket::CreateCommPacket(TYPE commType)
{
	/* I originally intended to create the packets like this because it felt more object-oriented. However, it was very easy to
	   forget to add code here when I derived a nu comm packet class.
	if(commType == AuCommChat::CommPacketType())
		return new AuCommChat;
		*/

	// This switch statement insures that I get a compiler warning if I derive a nu comm packet class and forget to add it here.
	switch(commType)
	{
	case Accept:
		return new AuCommAccept;
	case AcceptAvatar:
		return new AuCommAcceptAvatar;
	case AcceptBarter:
		return new AuCommAcceptBarter;
	case AcceptBerthCaptain:
		return new AuCommAcceptBerthCaptain;
	case AcceptBerthCaptainSlot:
		return new AuCommAcceptBerthCaptainSlot;
	case AcceptBerthCrew:
		return new AuCommAcceptBerthCrew;
	case AcceptBerthOfficer:
		return new AuCommAcceptBerthOfficer;
	case AcceptBerthPassenger:
		return new AuCommAcceptBerthPassenger;
	case AcceptBerthTroop:
		return new AuCommAcceptBerthTroop;
	case AcceptTeleport:
		return new AuCommAcceptTeleport;
	case AcceptInviteToEmpire:
		return new AuCommAcceptInviteToEmpire;
	case AcceptPickupShipment:
		return new AuCommAcceptPickupShipment;
	case AcceptRefit:
		return new AuCommAcceptRefit;
	case AcceptRepairAll:
		return new AuCommAcceptRepairAll;
	case AcceptRepairSystem:
		return new AuCommAcceptRepairSystem;
	case AcceptUpgradeInstall:
		return new AuCommAcceptUpgradeInstall;
	case Announcement:
		return new AuCommAnnouncement;
	case Auction:
		return new AuCommAuction;
	case BeamAtom:
		return new AuCommBeamAtom;
	case BeamCoordinate:
		return new AuCommBeamCoordinate;
	case BeamOver:
		return new AuCommBeamOver;
	case BeamPlanet:
		return new AuCommBeamPlanet;
	case BeamShip:
		return new AuCommBeamShip;
	case BeamTransporter:
		return new AuCommBeamTransporter;
	case BeamVehicleAtom:
		return new AuCommBeamVehicleAtom;
	case BeamVehicleCoordinate:
		return new AuCommBeamVehicleCoordinate;
	case BeamVehicleOver:
		return new AuCommBeamVehicleOver;
	case BeamVehiclePlanet:
		return new AuCommBeamVehiclePlanet;
	case BeamVehicleShip:
		return new AuCommBeamVehicleShip;
	case BeamVehicleTransporter:
		return new AuCommBeamVehicleTransporter;
	case Bid:
		return new AuCommBid;
	case BidBarter:
		return new AuCommBidBarter;
	case BidPickupShipment:
		return new AuCommBidPickupShipment;
	case BoardCrew:
		return new AuCommBoardCrew;
	case BoardCryo:
		return new AuCommBoardCryo;
	case BoardOfficers:
		return new AuCommBoardOfficers;
	case BoardPassengers:
		return new AuCommBoardPassengers;
	case BoardTroops:
		return new AuCommBoardTroops;
	case ChannelOpened:
		return new AuCommChannelOpened;
	case Chat:
		return new AuCommChat;
	case ChatAudio:
		return new AuCommChatAudio;
	case ChatChoice:
		return new AuCommChatChoice;
	case ChatContinued:
		return new AuCommChatContinued;
	case ChatImage:
		return new AuCommChatImage;
	case ChatStory:
		return new AuCommChatStory;
	case ChoicePicked:
		return new AuCommChoicePicked;
	case CitizenOrder:
		return new AuCommCitizenOrder;
	case CitizenPosture:
		return new AuCommCitizenPosture;
	case CityChat:
		return new AuCommCityChat;
	case CityMessage:
		return new AuCommCityMessage;
	case CitySurrenderAccepted:
		return new AuCommCitySurrenderAccepted;
	case CitySurrenderOffered:
		return new AuCommCitySurrenderOffered;
	case Combine:
		return new AuCommCombine;
	case ContactReply:
		return new AuCommContactReply;
	case ContactShip:
		return new AuCommContactShip;
	case CrewBoarded:
		return new AuCommCrewBoarded;
	case CrewBoardingComplete:
		return new AuCommCrewBoardingComplete;
	case CryoBoarded:
		return new AuCommCryoBoarded;
	case CryoBoardingComplete:
		return new AuCommCryoBoardingComplete;
	case CryoDebarked:
		return new AuCommCryoDebarked;
	case DamageInflicted:
		return new AuCommDamageInflicted;
	case DebarkCryo:
		return new AuCommDebarkCryo;
	case DebarkPassengers:
		return new AuCommDebarkPassengers;
	case Detonate:
		return new AuCommDetonate;
	case DiplomaticMessage:
		return new AuCommDiplomaticMessage;
	case FiringAtTarget:
		return new AuCommFiringAtTarget;
	case ForSale:
		return new AuCommForSale;
	case HailReply:
		return new AuCommHailReply;
	case HailRequest:
		return new AuCommHailRequest;
	case Harvest:
		return new AuCommHarvest;
	case InventoryItem:
		return new AuCommInventoryItem;
	case InviteTeleport:
		return new AuCommInviteTeleport;
	case InviteToEmpire:
		return new AuCommInviteToEmpire;
	case Jettison:
		return new AuCommJettison;
	case OpenTEU:
		return new AuCommOpenTEU;
	case Thought:
		return new AuCommThought;
	case ThoughtMailAlert:
		return new AuCommThoughtMailAlert;
	case ThoughtMessage:
		return new AuCommThoughtMessage;
	case Message:
		return new AuCommMessage;
	case MissionNew:
		return new AuCommMissionNew;
	case MissionOrderAdd:
		return new AuCommMissionOrderAdd;
	case MissionOrderInsert:
		return new AuCommMissionOrderInsert;
	case MissionOrderNew:
		return new AuCommMissionOrderNew;
	case MissionProperty:
		return new AuCommMissionProperty;
	case Narrate:
		return new AuCommNarrate;
	case NarrateAudio:
		return new AuCommNarrateAudio;
	case NarrateChoice:
		return new AuCommNarrateChoice;
	case NarrateImage:
		return new AuCommNarrateImage;
	case NarrateTitle:
		return new AuCommNarrateTitle;
	case NarrateVideo:
		//return new AuCommNarrateVideo;
		return 0;	//DHH AuCommNarrateVideo not implemented.
	case OfferAvatar:
		return new AuCommOfferAvatar;
	case OfferRefit:
		return new AuCommOfferRefit;
	case OfferRepairAll:
		return new AuCommOfferRepairAll;
	case OfferRepairSystem:
		return new AuCommOfferRepairSystem;
	case OfferTEU:
		return new AuCommOfferTEU;
	case OfferUpgradeInstall:
		return new AuCommOfferUpgradeInstall;
	case OfficerBoarded:
		return new AuCommOfficerBoarded;
	case PassengerBoardingComplete:
		return new AuCommPassengerBoardingComplete;
	case PassengersBoarded:
		return new AuCommPassengersBoarded;
	case PassengersDebarked:
		return new AuCommPassengersDebarked;
	case PreBoard:
		return new AuCommPreBoard;
	case RepairPartsMissing:
		return new AuCommRepairPartsMissing;
	case RequestBerthAny:
		return new AuCommRequestBerthAny;
	case RequestBerthCaptain:
		return new AuCommRequestBerthCaptain;
	case RequestBerthCaptainSlot:
		return new AuCommRequestBerthCaptainSlot;
	case RequestBerthCrew:
		return new AuCommRequestBerthCrew;
	case RequestBerthOfficer:
		return new AuCommRequestBerthOfficer;
	case RequestBerthPassenger:
		return new AuCommRequestBerthPassenger;
	case RequestBerthTroop:
		return new AuCommRequestBerthTroop;
	case RequestExterminator:
		return new AuCommRequestExterminator;
	case RequestHelp:
		return new AuCommRequestHelp;
	case RequestHouseKeeping:
		return new AuCommRequestHouseKeeping;
	case RequestInventory:
		return new AuCommRequestInventory;
	case RequestRefit:
		return new AuCommRequestRefit;
	case RequestRepairAll:
		return new AuCommRequestRepairAll;
	case RequestRepairSystem:
		return new AuCommRequestRepairSystem;
	case RequestUpgradeSystem:
		return new AuCommRequestUpgradeSystem;
	case RevokeBid:
		return new AuCommRevokeBid;
	case ShipmentItem:
		return new AuCommShipmentItem;
	case ShipStasisWait:
		return new AuCommShipStasisWait;
	case ShipStasisDimension:
		return new AuCommShipStasisDimension;
	case Silverwill:
		return new AuCommSilverwill;
	case SolicitAllSales:
		return new AuCommSolicitAllSales;
	case SolicitAmmoSales:
		return new AuCommSolicitAmmoSales;
	case SolicitArmorSales:
		return new AuCommSolicitArmorSales;
	case SolicitConstructionMaterialSales:
		return new AuCommSolicitConstructionMaterialSales;
	case SolicitFoodSales:
		return new AuCommSolicitFoodSales;
	case SolicitFuelSales:
		return new AuCommSolicitFuelSales;
	case SolicitItemSales:
		return new AuCommSolicitItemSales;
	case SolicitMedSales:
		return new AuCommSolicitMedSales;
	case SolicitMiscSales:
		return new AuCommSolicitMiscSales;
	case SolicitNaturalResourceSales:
		return new AuCommSolicitNaturalResourceSales;
	case SolicitPartsSales:
		return new AuCommSolicitPartsSales;
	case SolicitPatentSales:
		return new AuCommSolicitPatentSales;
	case SolicitShipments:
		return new AuCommSolicitShipments;
	case SolicitToolSales:
		return new AuCommSolicitToolSales;
	case SolicitWeaponSales:
		return new AuCommSolicitWeaponSales;
	case SOSAttack:
		return new AuCommSOSAttack;
	case SOSFuelMarooned:
		return new AuCommSOSFuelMarooned;
	case SOSFuelShip:
		return new AuCommSOSFuelShip;
	case SOSFuelStation:
		return new AuCommSOSFuelStation;
	case SOSManeuverDriveAdrift:
		return new AuCommSOSManeuverDriveAdrift;
	case SOSManeuverDriveMarooned:
		return new AuCommSOSManeuverDriveMarooned;
	case SOSManeuverDriveStation:
		return new AuCommSOSManeuverDriveStation;
	case SpotTargetOrTerrain:
		return new AuCommSpotTargetOrTerrain;
	case TroopBoardingComplete:
		return new AuCommTroopBoardingComplete;
	case TroopsBoarded:
		return new AuCommTroopsBoarded;
	case CommTypes:
		break;
	}
	return 0;
}

//static
QString AuCommPacket::TypeName(AuCommPacket::TYPE commType)
{
	QString typeName;
	AuCommPacket* msg = CreateCommPacket(commType);
	if(msg)
	{
		typeName = msg->TypeName();
		delete msg;
	}

	//if(typeName.isEmpty())
		//typeName = QString("AuCommPacket::TypeName INVALID commType %1").arg(commType);

	return typeName;
}

AuIdList AuCommPacket::Recipients() const
{
	return AuIdList();
}

COMMODITYID AuCommPacket::CommodityId() const
{
	if(Item())
		return Item()->CommodityId();
	return CID_Invalid;
}

quint32 AuCommPacket::FromEmpireId() const
{
	switch(Scope())
	{
	case AuCommStdChannel::Agent:
	case AuCommStdChannel::Architect:
	case AuCommStdChannel::Audible:
	case AuCommStdChannel::Diplomacy:
	case AuCommStdChannel::Empire:
	case AuCommStdChannel::Everyone:
	case AuCommStdChannel::Fleet:
	case AuCommStdChannel::Friends:
	case AuCommStdChannel::Government:
	case AuCommStdChannel::Help:
	case AuCommStdChannel::Programmer:
	case AuCommStdChannel::Thoughts:
		// Scope data is probably empire id.
		return m_ScopeData;
	case AuCommStdChannel::Company:		// Scope data is owner id.
	case AuCommStdChannel::CrewMates:	// Scope data is ship id.
	case AuCommStdChannel::Intercom:	// Scope data is ship id?
	case AuCommStdChannel::Narrator:	// Scope data is story instance?
	case AuCommStdChannel::ScopeTypes:	// Scope is bogus.
	case AuCommStdChannel::Squadron:	// Scope data is squadron identifier?
		break;
	}
	return 0;
}

// This base class implementation returns true if the Type() is in the valid range of enum values.
bool AuCommPacket::IsValid() const
{
	TYPE type = Type();
	if(type < 0 || type >= CommTypes)
	{
		qDebug("AuCommPacket::IsValid invalid message type %d", type);
		return false;
	}

	if(m_ChannelName.isEmpty())
	{
		qDebug("AuCommPacket::IsValid empty channel name");
		return false;
	}

	if(m_FromGalaxy > GALAXY_Count)
	{
		qDebug() << "AuCommPacket::IsValid bad from galaxy" << m_FromGalaxy;
		return false;
	}

	if(m_FromId.IsNull())
	{
		qDebug("AuCommPacket::IsValid null fromId");
		return false;
	}

	if(m_FromName.isEmpty())
	{
		qDebug("AuCommPacket::IsValid empty fromName");
		return false;
	}

	if(m_FromSectorId.IsNull())
	{
		qDebug("AuCommPacket::IsValid null fromSectorId");
		return false;
	}

	// Range
	if(m_RangeTx < AuCommStdChannel::Close || m_RangeTx >= AuCommStdChannel::RANGE_Count)
	{
		qDebug("AuCommPacket::IsValid invalid range %d", m_RangeTx);
		return false;
	}
	else if(m_RangeTx > AuCommStdChannel::SolarSystem &&
			(AuCommStdChannel::IsHailChannel(m_ChannelName) || AuCommStdChannel::IsTradeChannel(m_ChannelName)))
	{
		qDebug() << "AuCommPacket::IsValid invalid range" << m_RangeTx << "for channel" << m_ChannelName;
		return false;
	}
	else if(m_Scope != AuCommStdChannel::Thoughts && !AuCommStdChannel::ChannelHasRange(m_ChannelName))
	{
		// All other rangeless messages are sent at their default range, i.e. Thoughts, Voice and Intercom.
		if(m_RangeTx != AuCommStdChannel::DefaultRange(m_ChannelName))
		{
			qDebug() << "AuCommPacket::IsValid invalid range" << m_RangeTx << "for rangeless channel" << m_ChannelName;
			return false;
		}
	}

	// Scope
	if(m_Scope < AuCommStdChannel::Agent || m_Scope >= AuCommStdChannel::ScopeTypes)
	{
		qDebug("AuCommPacket::IsValid invalid scope %d", m_Scope);
		return false;
	}

	// Certain scopes must have scope data.
	if(AuCommStdChannel::ScopeHasData(m_Scope) && m_ScopeData == 0)
	{
		qDebug("AuCommPacket::IsValid scopeData missing for scope %d", m_Scope);
		return false;
	}

	// Some reserved channel names have a defined scope. Messages on these channels must be sent in the scope of the channel.
	// Audible scope is treated like Everyone scope in this case.
	// Unreserved channel names can also have a scope but we cannot recognize them here. They will report Everyone scope by name.
	AuCommStdChannel::SCOPE channelScope = AuCommStdChannel::Scope(m_ChannelName);
	if(channelScope != AuCommStdChannel::Everyone && channelScope != AuCommStdChannel::Audible &&
			m_Scope != AuCommStdChannel::Thoughts && channelScope != m_Scope)
	{
		qDebug("AuCommPacket::IsValid channelScope %d does not match message scope %d", channelScope, m_Scope);
		return false;
	}

	// Ran the gauntlet so this message must be valid.
	return true;
}

//static
AuCommPacket* AuCommPacket::RxDecrypt(QDataStream& in, short int wKey)
{
	// Receive the compressed/encrypted byte array from the data stream.
	AuCrypt dataCE;
	in >> dataCE;

	// Decrypt the compressed/encrypted byte array.
	dataCE.Crypt(wKey);

	// Decompress the compressed byte array.
	QByteArray data;
	if(!dataCE.isEmpty())
		data = AuUncompress(dataCE, __FILE__, __LINE__,"");
	if(data.isEmpty())
		return 0;

	// Stream this message object from the byte array.
	QDataStream inStream(data);
	inStream.setVersion(QDataStream::Qt_4_1);
	quint8 commType;
	inStream >> commType;
	AuCommPacket* msg = CreateCommPacket((AuCommPacket::TYPE)commType);
	if(msg)
		inStream >> *msg;
	return msg;
}

// Transmits encrypted message into stream.
void AuCommPacket::TxEncrypt(QDataStream& out, short int wKey) const
{
	// Stream this message object into a byte array.
	QByteArray data;
	QDataStream outStream(&data, QIODevice::WriteOnly);
	outStream.setVersion(QDataStream::Qt_4_1);
	outStream << (quint8)Type();
	outStream << *this;

	// Compress the byte array.
	QByteArray dataC(qCompress(data));

	// Encrypt the compressed byte array.
	AuCrypt dataCE;
	dataCE.EncryptByteArray(dataC, wKey);

	// Transmit the compressed/encrypted byte array into the data stream.
	out << dataCE;
}

quint32 AuCommPacket::StoryInstance() const
{
	return 0;
}

const QString& AuCommPacket::EmpireOfOrigin() const
{
	return s_EmptyString;
}

quint32 AuCommPacket::Count() const
{
	if(Item())
		return Item()->Count();
	return 0;
}

quint32 AuCommPacket::DesignId() const
{
	return 0;
}

quint32 AuCommPacket::Price() const
{
	return 0;
}

QString AuCommPacket::Icon2() const
{
	COMMODITYID cid = CommodityId();
	if(cid > CID_Invalid && cid < CID_Count)
		return QString(g_Au.CommodityBitmapName(cid));
	return QString();
}

//static
QString AuCommPacket::Icon(TYPE commType)
{
	QString icon;
	AuCommPacket* msg = CreateCommPacket(commType);
	if(msg)
	{
		icon = msg->Icon();
		delete msg;
	}
	return icon;
}

bool AuCommPacket::OnScreenByDefault() const
{
	return false;
}

bool AuCommPacket::OnScreenByDefault(TYPE commType)
{
	bool onScreen = false;
	AuCommPacket* msg = CreateCommPacket(commType);
	if(msg)
	{
		onScreen = msg->OnScreenByDefault();
		delete msg;
	}
	return onScreen;
}

const AuId& AuCommPacket::ToId() const
{
	return AuId::NullId();
}

bool AuCommPacket::IsDataLarge() const
{
	return false;
}

SYSTEM AuCommPacket::System() const
{
	return SYS_Count;
}

const QString& AuCommPacket::SystemDesc() const
{
	return s_EmptyString;
}

quint16 AuCommPacket::SystemWhich() const
{
	return 0;
}

VEHICLE_SPOT_TYPE AuCommPacket::SpotType() const
{
	return SPOT_Invalid;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCommPacketList::operator<<(QDataStream& in)
{
	Clear();

	quint32 commcount;
	in >> commcount;
	quint8 commtype;
	while(commcount--)
	{
		in >> commtype;

		AuCommPacket* comm = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)commtype);
		if(!comm)
		{
			qWarning() << "AuCommPacketList::operator<< ERROR! read bad object type" << commtype;
			break;
		}

		in >> *comm;
		append(comm);
	}
}

void AuCommPacketList::operator>>(QDataStream& out) const
{
	out << (quint32)count();
	for(const_iterator cpi = constBegin(); cpi != constEnd(); cpi++)
	{
		out << (quint8)(*cpi)->Type();
		out << **cpi;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAccept::AuCommAccept(const AuId& buyerId, const QString& buyerName, const AuItem& item,
		const QString& empireOfOrigin, quint32 price,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BuyerId = buyerId;
	m_BuyerName = buyerName;
	m_Item = item.Clone();
	m_EmpireOfOrigin = empireOfOrigin;
	m_Price = price;
}

AuCommAccept::AuCommAccept()
{
	//m_BuyerId
	//m_BuyerName
	//m_EmpireOfOrigin
	m_Item = 0;
	m_Price = 0;
}

AuCommAccept::AuCommAccept(const AuCommAccept& c) : AuCommPacket(c)
{
	m_BuyerId = c.m_BuyerId;
	m_BuyerName = c.m_BuyerName;
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_Price = c.m_Price;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommAccept::~AuCommAccept()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommAccept::Clone() const
{
	return new AuCommAccept(*this);
}

QString AuCommAccept::Icon() const
{
	return CommPacketIcon();
}

void AuCommAccept::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BuyerId;
	in >> m_BuyerName;
	in >> m_EmpireOfOrigin;
	in >> m_Price;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommAccept::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BuyerId;
	out << m_BuyerName;
	out << m_EmpireOfOrigin;
	out << m_Price;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommAccept::Text() const
{
	if(!m_Item)
	{
		qWarning() << "AuCommAccept::Text item corrupted"
			<< "buyer" << m_BuyerId << m_BuyerName
			<< "from" << m_FromId << m_FromName << "sect" << m_FromSectorId << "gxy" << m_FromGalaxy
			<< "anon" << m_Anonymous
			<< "chan" << m_ChannelName
			<< "range" << m_RangeTx << m_RangeRx
			<< "scope" << m_Scope << m_ScopeData;
		return QObject::tr("AuCommAccept::Text item corrupted.");
	}

	QString text;
	if(m_Price == 0)
	{
		text = QObject::tr("%1. Free offer accepted.")
			.arg(m_Item->ListDesc(m_EmpireOfOrigin));
	}
	else
	{
		text = QObject::tr("%1. %L7%8 accepted.")
			.arg(m_Item->ListDesc(m_EmpireOfOrigin))
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol());
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAccept::Type() const
{
	return CommPacketType();
}

const QString& AuCommAccept::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBarter::AuCommAcceptBarter(const AuId& buyerId, const QString& buyerName, const AuItem& item,
		const QString& empireOfOrigin, const AuItem& offerItem, const QString& offerEmpireOfOrigin,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BuyerId = buyerId;
	m_BuyerName = buyerName;
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
	m_OfferEmpireOfOrigin = offerEmpireOfOrigin;
	m_OfferItem = offerItem.Clone();
}

AuCommAcceptBarter::AuCommAcceptBarter()
{
	//m_BuyerId
	//m_BuyerName
	//m_EmpireOfOrigin
	m_Item = 0;
	//m_OfferEmpireOfOrigin
	m_OfferItem = 0;
}

AuCommAcceptBarter::AuCommAcceptBarter(const AuCommAcceptBarter& c) : AuCommPacket(c)
{
	m_BuyerId = c.m_BuyerId;
	m_BuyerName = c.m_BuyerName;
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_OfferEmpireOfOrigin = c.m_OfferEmpireOfOrigin;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;

	if(c.m_OfferItem)
		m_OfferItem = c.m_OfferItem->Clone();
	else
		m_OfferItem = 0;
}

AuCommAcceptBarter::~AuCommAcceptBarter()
{
	if(m_Item)
		delete m_Item;
	if(m_OfferItem)
		delete m_OfferItem;
}

AuCommPacket* AuCommAcceptBarter::Clone() const
{
	return new AuCommAcceptBarter(*this);
}

QString AuCommAcceptBarter::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBarter::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BuyerId;
	in >> m_BuyerName;
	in >> m_EmpireOfOrigin;
	in >> m_OfferEmpireOfOrigin;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;

	if(m_OfferItem)
		delete m_OfferItem;

	in >> itemType;
	if(itemType)
	{
		m_OfferItem = AuItem::Create(itemType);
		if(m_OfferItem)
			in >> *m_OfferItem;
	}
	else
		m_OfferItem = 0;
}

void AuCommAcceptBarter::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BuyerId;
	out << m_BuyerName;
	out << m_EmpireOfOrigin;
	out << m_OfferEmpireOfOrigin;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;

	if(m_OfferItem)
	{
		out << m_OfferItem->Type();
		out << *m_OfferItem;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommAcceptBarter::Text() const
{
	Q_ASSERT(m_Item);
	Q_ASSERT(m_OfferItem);
	return QObject::tr("%1. Trade for %2 accepted.")
		.arg(m_Item->ListDesc(m_EmpireOfOrigin))
		.arg(m_OfferItem->ListDesc(m_OfferEmpireOfOrigin));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBarter::Type() const
{
	return CommPacketType();
}

const QString& AuCommAcceptBarter::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommAcceptBerth derived packets.
// Not a packet type per se.

AuCommAcceptBerth::AuCommAcceptBerth(const AuId& unitId, const QString& unitName, bool unitIsNewOwner, const AuId& shipId,
		const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ShipId = shipId;
	m_ShipIsPrivate = isPrivate;
	m_ShipName = shipName;
	m_UnitId = unitId;
	m_UnitIsNewOwner = unitIsNewOwner;
	m_UnitName = unitName;
}

AuCommAcceptBerth::AuCommAcceptBerth()
{
	m_ShipIsPrivate = false;
	m_UnitIsNewOwner = false;
}

AuCommAcceptBerth::AuCommAcceptBerth(const AuCommAcceptBerth& c) : AuCommPacket(c)
{
	m_ShipId = c.m_ShipId;
	m_ShipIsPrivate = c.m_ShipIsPrivate;
	m_ShipName = c.m_ShipName;
	m_UnitId = c.m_UnitId;
	m_UnitIsNewOwner = c.m_UnitIsNewOwner;
	m_UnitName = c.m_UnitName;
}

void AuCommAcceptBerth::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ShipId;
	in >> m_ShipIsPrivate;
	in >> m_ShipName;
	in >> m_UnitId;
	in >> m_UnitIsNewOwner;
	in >> m_UnitName;
}

void AuCommAcceptBerth::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ShipId;
	out << m_ShipIsPrivate;
	out << m_ShipName;
	out << m_UnitId;
	out << m_UnitIsNewOwner;
	out << m_UnitName;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthCaptain::AuCommAcceptBerthCaptain(const AuId& unitId, const QString& unitName, bool unitIsNewOwner,
		const AuId& shipId, const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, unitIsNewOwner, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector,
			fromGalaxy, range, scope)
{
}

AuCommAcceptBerthCaptain::AuCommAcceptBerthCaptain()
{
}

AuCommAcceptBerthCaptain::AuCommAcceptBerthCaptain(const AuCommAcceptBerthCaptain& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthCaptain::Clone() const
{
	return new AuCommAcceptBerthCaptain(*this);
}

QString AuCommAcceptBerthCaptain::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthCaptain::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthCaptain::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthCaptain::Text() const
{
	if(UnitIsNewOwner())
		return QObject::tr("Captain's berth assigned. Welcome aboard sir. "
				"Title to this unowned spacecraft was assigned to you.");
	else
		return QObject::tr("Captain's berth assigned. Welcome aboard sir.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthCaptain::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthCaptainSlot::AuCommAcceptBerthCaptainSlot(const AuId& unitId, const QString& unitName, bool unitIsNewOwner,
		const AuId& shipId, const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, unitIsNewOwner, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector,
			fromGalaxy, range, scope)
{
}

AuCommAcceptBerthCaptainSlot::AuCommAcceptBerthCaptainSlot()
{
}

AuCommAcceptBerthCaptainSlot::AuCommAcceptBerthCaptainSlot(const AuCommAcceptBerthCaptainSlot& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthCaptainSlot::Clone() const
{
	return new AuCommAcceptBerthCaptainSlot(*this);
}

QString AuCommAcceptBerthCaptainSlot::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthCaptainSlot::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthCaptainSlot::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthCaptainSlot::Text() const
{
	if(UnitIsNewOwner())
		return QObject::tr("Captain's position assigned. Welcome aboard sir. "
				"Title to this unowned spacecraft was assigned to you.");
	else
		return QObject::tr("Captain's position assigned. Welcome aboard sir.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthCaptainSlot::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthCrew::AuCommAcceptBerthCrew(const AuId& unitId, const QString& unitName, const AuId& shipId,
		const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, false, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope)
{
}

AuCommAcceptBerthCrew::AuCommAcceptBerthCrew()
{
}

AuCommAcceptBerthCrew::AuCommAcceptBerthCrew(const AuCommAcceptBerthCrew& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthCrew::Clone() const
{
	return new AuCommAcceptBerthCrew(*this);
}

QString AuCommAcceptBerthCrew::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthCrew::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthCrew::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthCrew::Text() const
{
	return QObject::tr("Crew berth assigned. Welcome aboard mate.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthCrew::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthOfficer::AuCommAcceptBerthOfficer(const AuId& unitId, const QString& unitName, const AuId& shipId,
		const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, false, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope)
{
}

AuCommAcceptBerthOfficer::AuCommAcceptBerthOfficer()
{
}

AuCommAcceptBerthOfficer::AuCommAcceptBerthOfficer(const AuCommAcceptBerthOfficer& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthOfficer::Clone() const
{
	return new AuCommAcceptBerthOfficer(*this);
}

QString AuCommAcceptBerthOfficer::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthOfficer::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthOfficer::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthOfficer::Text() const
{
	return QObject::tr("Officer berth assigned. Welcome aboard sir.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthOfficer::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthPassenger::AuCommAcceptBerthPassenger(const AuId& unitId, const QString& unitName, const AuId& shipId,
		const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, false, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope)
{
}

AuCommAcceptBerthPassenger::AuCommAcceptBerthPassenger()
{
}

AuCommAcceptBerthPassenger::AuCommAcceptBerthPassenger(const AuCommAcceptBerthPassenger& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthPassenger::Clone() const
{
	return new AuCommAcceptBerthPassenger(*this);
}

QString AuCommAcceptBerthPassenger::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthPassenger::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthPassenger::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthPassenger::Text() const
{
	return QObject::tr("Passenger berth assigned. Welcome aboard.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthPassenger::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptBerthTroop::AuCommAcceptBerthTroop(const AuId& unitId, const QString& unitName, const AuId& shipId,
		const QString& shipName, bool isPrivate,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptBerth(unitId, unitName, false, shipId, shipName, isPrivate, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope)
{
}

AuCommAcceptBerthTroop::AuCommAcceptBerthTroop()
{
}

AuCommAcceptBerthTroop::AuCommAcceptBerthTroop(const AuCommAcceptBerthTroop& c) : AuCommAcceptBerth(c)
{
}

AuCommPacket* AuCommAcceptBerthTroop::Clone() const
{
	return new AuCommAcceptBerthTroop(*this);
}

QString AuCommAcceptBerthTroop::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptBerthTroop::operator<<(QDataStream& in)
{
	AuCommAcceptBerth::operator<<(in);
}

void AuCommAcceptBerthTroop::operator>>(QDataStream& out) const
{
	AuCommAcceptBerth::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommAcceptBerthTroop::Text() const
{
	return QObject::tr("Troop berth assigned. Welcome aboard.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptBerthTroop::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptInviteToEmpire::AuCommAcceptInviteToEmpire(const AuId& toId, quint32 empireId,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireId = empireId;
	m_ToId = toId;
}

AuCommAcceptInviteToEmpire::AuCommAcceptInviteToEmpire()
{
	m_EmpireId = 0;
}

AuCommAcceptInviteToEmpire::AuCommAcceptInviteToEmpire(const AuCommAcceptInviteToEmpire& c) : AuCommPacket(c)
{
	m_EmpireId = c.m_EmpireId;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommAcceptInviteToEmpire::Clone() const
{
	return new AuCommAcceptInviteToEmpire(*this);
}

QString AuCommAcceptInviteToEmpire::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptInviteToEmpire::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireId;
	in >> m_ToId;
}

void AuCommAcceptInviteToEmpire::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireId;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommAcceptInviteToEmpire::Text() const
{
	return QObject::tr("I accept your invitation to join your empire.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptInviteToEmpire::Type() const
{
	return CommPacketType();
}

quint32 AuCommAcceptInviteToEmpire::FromEmpireId() const
{
	return m_EmpireId;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommAcceptRepair derived packets.

AuCommAcceptRepair::AuCommAcceptRepair(const AuId& shipId, const AuId& buyerId, const QString& buyerName, const AuId& cityId,
		quint32 price, quint16 repairSecs, quint8 minQ,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BuyerId = buyerId;
	m_BuyerName = buyerName;
	m_CityId = cityId;
	m_MinQ = minQ;
	m_Price = price;
	m_RepairSecs = repairSecs;
	m_ShipId = shipId;
}

AuCommAcceptRepair::AuCommAcceptRepair()
{
	m_MinQ = 1;
	m_Price = 0;
	m_RepairSecs = 0;
}

AuCommAcceptRepair::AuCommAcceptRepair(const AuCommAcceptRepair& c) : AuCommPacket(c)
{
	m_BuyerId = c.m_BuyerId;
	m_BuyerName = c.m_BuyerName;
	m_CityId = c.m_CityId;
	m_MinQ = c.m_MinQ;
	m_Price = c.m_Price;
	m_RepairSecs = c.m_RepairSecs;
	m_ShipId = c.m_ShipId;
}

void AuCommAcceptRepair::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_BuyerId;
	in >> m_BuyerName;
	in >> m_CityId;
	in >> m_MinQ;
	in >> m_Price;
	in >> m_RepairSecs;
	in >> m_ShipId;
}

void AuCommAcceptRepair::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BuyerId;
	out << m_BuyerName;
	out << m_CityId;
	out << m_MinQ;
	out << m_Price;
	out << m_RepairSecs;
	out << m_ShipId;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptRepairAll::AuCommAcceptRepairAll(const AuId& shipId, const AuId& buyerId, const QString& buyerName, const AuId& cityId,
		quint32 price, quint16 repairSecs, const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ,
		const QList<COMMODITYID>& noSupply, const QList<COMMODITYID>& enough, quint8 minQ,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptRepair(shipId, buyerId, buyerName, cityId, price, repairSecs, minQ, channelName, fromId, fromName, fromSector,
			fromGalaxy, range, scope),
	m_NoQ(noQ), m_ShortQ(shortQ), m_NoSupply(noSupply), m_Enough(enough)
{
}

AuCommAcceptRepairAll::AuCommAcceptRepairAll()
{
}

AuCommAcceptRepairAll::AuCommAcceptRepairAll(const AuCommAcceptRepairAll& c) : AuCommAcceptRepair(c), m_NoQ(c.m_NoQ),
	m_ShortQ(c.m_ShortQ), m_NoSupply(c.m_NoSupply), m_Enough(c.m_Enough)
{
}

AuCommPacket* AuCommAcceptRepairAll::Clone() const
{
	return new AuCommAcceptRepairAll(*this);
}

QString AuCommAcceptRepairAll::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptRepairAll::operator<<(QDataStream& in)
{
	AuCommAcceptRepair::operator<<(in);

	quint8 ui8;
	quint16 ui16;

	m_NoQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoQ.append((COMMODITYID)ui16);
	}

	m_ShortQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_ShortQ.append((COMMODITYID)ui16);
	}

	m_NoSupply.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoSupply.append((COMMODITYID)ui16);
	}

	m_Enough.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_Enough.append((COMMODITYID)ui16);
	}
}

void AuCommAcceptRepairAll::operator>>(QDataStream& out) const
{
	AuCommAcceptRepair::operator>>(out);

	out << (quint8)m_NoQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoQ.constBegin(); mi != m_NoQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_ShortQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.constBegin(); mi != m_ShortQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_NoSupply.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.constBegin(); mi != m_NoSupply.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_Enough.count();
	for(QList<COMMODITYID>::const_iterator mi = m_Enough.constBegin(); mi != m_Enough.constEnd(); mi++)
		out << (quint16)(*mi);
}

// Returns the text of the comm message.
QString AuCommAcceptRepairAll::Text() const
{
	QString text;
	if(m_Price == 0)
		text = QObject::tr("Offer to repair all systems for free accepted.");
	else
		text = QObject::tr("Offer to repair all systems for %L1%2 accepted.")
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptRepairAll::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptRepairSystem::AuCommAcceptRepairSystem(const AuId& shipId, const AuId& buyerId, const QString& buyerName,
		const AuId& cityId, quint8 system, quint16 which, const QString& systemDesc, quint32 price, quint16 repairSecs,
		const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply,
		const QList<COMMODITYID>& enough, quint8 minQ,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommAcceptRepair(shipId, buyerId, buyerName, cityId, price, repairSecs, minQ, channelName, fromId, fromName, fromSector,
			fromGalaxy, range, scope),
	m_NoQ(noQ), m_ShortQ(shortQ), m_NoSupply(noSupply), m_Enough(enough)
{
	m_System = system;
	m_SystemDesc = systemDesc;
	m_Which = which;
}

AuCommAcceptRepairSystem::AuCommAcceptRepairSystem()
{
	m_System = 0;
	m_Which = 0;
}

AuCommAcceptRepairSystem::AuCommAcceptRepairSystem(const AuCommAcceptRepairSystem& c) : AuCommAcceptRepair(c),
	m_NoQ(c.m_NoQ),
	m_ShortQ(c.m_ShortQ), m_NoSupply(c.m_NoSupply), m_Enough(c.m_Enough)
{
	m_System = c.m_System;
	m_SystemDesc = c.m_SystemDesc;
	m_Which = c.m_Which;
}

AuCommPacket* AuCommAcceptRepairSystem::Clone() const
{
	return new AuCommAcceptRepairSystem(*this);
}

QString AuCommAcceptRepairSystem::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptRepairSystem::operator<<(QDataStream& in)
{
	AuCommAcceptRepair::operator<<(in);

	quint8 ui8;
	quint16 ui16;

	m_NoQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoQ.append((COMMODITYID)ui16);
	}

	m_ShortQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_ShortQ.append((COMMODITYID)ui16);
	}

	m_NoSupply.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoSupply.append((COMMODITYID)ui16);
	}

	m_Enough.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_Enough.append((COMMODITYID)ui16);
	}

	in >> m_System;
	in >> m_SystemDesc;
	in >> m_Which;
}

void AuCommAcceptRepairSystem::operator>>(QDataStream& out) const
{
	AuCommAcceptRepair::operator>>(out);

	out << (quint8)m_NoQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoQ.constBegin(); mi != m_NoQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_ShortQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.constBegin(); mi != m_ShortQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_NoSupply.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.constBegin(); mi != m_NoSupply.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_Enough.count();
	for(QList<COMMODITYID>::const_iterator mi = m_Enough.constBegin(); mi != m_Enough.constEnd(); mi++)
		out << (quint16)(*mi);

	out << m_System;
	out << m_SystemDesc;
	out << m_Which;
}

// Returns the text of the comm message.
QString AuCommAcceptRepairSystem::Text() const
{
	QString text;
	if(m_Price == 0)
		text = QObject::tr("Offer to repair the %1 for free accepted.")
			.arg(m_SystemDesc);
	else
		text = QObject::tr("Offer to repair the %1 for %L2%3 accepted.")
			.arg(m_SystemDesc)
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptRepairSystem::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptUpgradeInstall::AuCommAcceptUpgradeInstall(const AuItem& item, const QString& empireOfOrigin, quint16 systemWhich,
		quint16 installSecs, quint32 price,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_InstallSecs = installSecs;
	m_Item = item.Clone();
	m_Price = price;
	m_Which = systemWhich;
}

AuCommAcceptUpgradeInstall::AuCommAcceptUpgradeInstall()
{
	m_InstallSecs = 0;
	m_Item = 0;
	m_Price = 0;
	m_Which = 0;
}

AuCommAcceptUpgradeInstall::AuCommAcceptUpgradeInstall(const AuCommAcceptUpgradeInstall& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_InstallSecs = c.m_InstallSecs;
	m_Price = c.m_Price;
	m_Which = c.m_Which;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommAcceptUpgradeInstall::~AuCommAcceptUpgradeInstall()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommAcceptUpgradeInstall::Clone() const
{
	return new AuCommAcceptUpgradeInstall(*this);
}

QString AuCommAcceptUpgradeInstall::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptUpgradeInstall::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_EmpireOfOrigin;
	in >> m_InstallSecs;
	in >> m_Price;
	in >> m_Which;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommAcceptUpgradeInstall::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_InstallSecs;
	out << m_Price;
	out << m_Which;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommAcceptUpgradeInstall::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("Offer to install %1 for %L6%7 accepted.")
		.arg(m_Item->ListDesc(m_EmpireOfOrigin))
		.arg(m_Price)
		.arg(AuGlobal::CurrencySymbol());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptUpgradeInstall::Type() const
{
	return CommPacketType();
}

const QString& AuCommAcceptUpgradeInstall::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

SYSTEM AuCommAcceptUpgradeInstall::System() const
{
	Q_ASSERT(m_Item);
	if(HullMaterialModule(m_Item->CommodityId()) != CID_Invalid)
		return SYS_Hull;
	return ScModuleSystem(m_Item->CommodityId());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAuction::AuCommAuction(const AuItem& item, const QString& empireOfOrigin,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
}

AuCommAuction::AuCommAuction()
{
	//m_EmpireOfOrigin
	m_Item = 0;
}

AuCommAuction::AuCommAuction(const AuCommAuction& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommAuction::~AuCommAuction()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommAuction::Clone() const
{
	return new AuCommAuction(*this);
}

QString AuCommAuction::Icon() const
{
	return CommPacketIcon();
}

void AuCommAuction::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommAuction::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommAuction::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("%1. Make an offer.").arg(m_Item->ListDesc(m_EmpireOfOrigin));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAuction::Type() const
{
	return CommPacketType();
}

const QString& AuCommAuction::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamOver::AuCommBeamOver(quint8 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
}

AuCommBeamOver::AuCommBeamOver()
{
	m_Count = 0;
}

AuCommBeamOver::AuCommBeamOver(const AuCommBeamOver& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
}

AuCommPacket* AuCommBeamOver::Clone() const
{
	return new AuCommBeamOver(*this);
}

QString AuCommBeamOver::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamOver::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Count;
}

void AuCommBeamOver::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Count;
}

// Returns the text of the comm message.
QString AuCommBeamOver::Text() const
{
	if(Scope() == AuCommStdChannel::Intercom)
	{
		if(m_Count == 1)
			return QObject::tr("Beam me to the transporter room.");
		else
			return QObject::tr("%1 to beam to the transporter room.").arg(m_Count);
	}
	else if(Scope() == AuCommStdChannel::CrewMates)
	{
		if(m_Count == 1)
			return QObject::tr("Beam me aboard.");
		else
			return QObject::tr("%1 to beam aboard.").arg(m_Count);
	}
	else
	{
		if(m_Count == 1)
			return QObject::tr("I request a transporter beam to board your vessel.");
		else
			return QObject::tr("I request a transporter beam for %1 to board your vessel.").arg(m_Count);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamOver::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamAtom::AuCommBeamAtom(const AuId& crewman, quint8 count, quint8 dir, const AuId& cityId, const QString& cityName,
		TRANSPORTER_TARGET target, const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_CityId = cityId;
	m_CityName = cityName;
	m_Count = count;
	m_Crewman = crewman;
	m_Target = target;
}

AuCommBeamAtom::AuCommBeamAtom()
{
	m_BeamDir = 0;
	m_Count = 0;
	m_Target = 0;
}

AuCommBeamAtom::AuCommBeamAtom(const AuCommBeamAtom& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_Count = c.m_Count;
	m_Crewman = c.m_Crewman;
	m_Target = c.m_Target;
}

AuCommPacket* AuCommBeamAtom::Clone() const
{
	return new AuCommBeamAtom(*this);
}

QString AuCommBeamAtom::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamAtom::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_CityId;
	in >> m_CityName;
	in >> m_Count;
	in >> m_Crewman;
	in >> m_Target;
}

void AuCommBeamAtom::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_CityId;
	out << m_CityName;
	out << m_Count;
	out << m_Crewman;
	out << m_Target;
}

AuIdList AuCommBeamAtom::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamAtom::Text() const
{
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2, %1 to beam ashore.")
			.arg(m_Count)
			.arg(m_CityName);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2, %1 to beam aboard.")
			.arg(m_Count)
			.arg(m_CityName);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamAtom::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamCoordinate::AuCommBeamCoordinate(const AuId& crewman, quint8 count, quint8 dir, float x, float y, float z,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_Count = count;
	m_Crewman = crewman;
	m_X = x;
	m_Y = y;
	m_Z = z;
}

AuCommBeamCoordinate::AuCommBeamCoordinate()
{
	m_BeamDir = 0;
	m_Count = 0;
	m_X = 0;
	m_Y = 0;
	m_Z = 0;
}

AuCommBeamCoordinate::AuCommBeamCoordinate(const AuCommBeamCoordinate& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_Count = c.m_Count;
	m_Crewman = c.m_Crewman;
	m_X = c.m_X;
	m_Y = c.m_Y;
	m_Z = c.m_Z;
}

AuCommPacket* AuCommBeamCoordinate::Clone() const
{
	return new AuCommBeamCoordinate(*this);
}

QString AuCommBeamCoordinate::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamCoordinate::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_Count;
	in >> m_Crewman;
	in >> m_X;
	in >> m_Y;
	in >> m_Z;
}

void AuCommBeamCoordinate::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_Count;
	out << m_Crewman;
	out << m_X;
	out << m_Y;
	out << m_Z;
}

AuIdList AuCommBeamCoordinate::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamCoordinate::Text() const
{
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to coordinates (%L2, %L3, %L4)m relative to the ship, %1 to beam away.")
			.arg(m_Count)
			.arg(m_X)
			.arg(m_Y)
			.arg(m_Z);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to coordinates (%L2, %L3, %L4)m relative to the ship, %1 to beam aboard.")
			.arg(m_Count)
			.arg(m_X)
			.arg(m_Y)
			.arg(m_Z);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamCoordinate::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamPlanet::AuCommBeamPlanet(const AuId& crewman, quint8 count, quint8 dir, float longitude, float latitude,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_Count = count;
	m_Crewman = crewman;
	m_Latitude = latitude;
	m_Longitude = longitude;
}

AuCommBeamPlanet::AuCommBeamPlanet()
{
	m_BeamDir = 0;
	m_Count = 0;
	m_Latitude = 0;
	m_Longitude = 0;
}

AuCommBeamPlanet::AuCommBeamPlanet(const AuCommBeamPlanet& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_Count = c.m_Count;
	m_Crewman = c.m_Crewman;
	m_Latitude = c.m_Latitude;
	m_Longitude = c.m_Longitude;
}

AuCommPacket* AuCommBeamPlanet::Clone() const
{
	return new AuCommBeamPlanet(*this);
}

QString AuCommBeamPlanet::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamPlanet::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_Count;
	in >> m_Crewman;
	in >> m_Latitude;
	in >> m_Longitude;
}

void AuCommBeamPlanet::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_Count;
	out << m_Crewman;
	out << m_Latitude;
	out << m_Longitude;
}

AuIdList AuCommBeamPlanet::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamPlanet::Text() const
{
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2 on the planet's surface, %1 to beam ashore.")
			.arg(m_Count)
			.arg(AuGlobeLL::ToString(m_Longitude, m_Latitude));
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2 on the planet's surface, %1 to beam aboard.")
			.arg(m_Count)
			.arg(AuGlobeLL::ToString(m_Longitude, m_Latitude));
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamPlanet::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamShip::AuCommBeamShip(const AuId& crewman, quint8 count, quint8 dir, const AuId& shipId, const QString& shipName,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_Count = count;
	m_Crewman = crewman;
	m_ShipId = shipId;
	m_ShipName = shipName;
}

AuCommBeamShip::AuCommBeamShip()
{
	m_BeamDir = 0;
	m_Count = 0;
}

AuCommBeamShip::AuCommBeamShip(const AuCommBeamShip& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_Count = c.m_Count;
	m_Crewman = c.m_Crewman;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
}

AuCommPacket* AuCommBeamShip::Clone() const
{
	return new AuCommBeamShip(*this);
}

QString AuCommBeamShip::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamShip::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_Count;
	in >> m_Crewman;
	in >> m_ShipId;
	in >> m_ShipName;
}

void AuCommBeamShip::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_Count;
	out << m_Crewman;
	out << m_ShipId;
	out << m_ShipName;
}

AuIdList AuCommBeamShip::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamShip::Text() const
{
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2, %1 to beam away.")
			.arg(m_Count)
			.arg(m_ShipName);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2, %1 to beam aboard.")
			.arg(m_Count)
			.arg(m_ShipName);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamShip::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamTransporter::AuCommBeamTransporter(const AuId& crewman, quint8 count, quint8 dir, const AuId& shipId,
		const QString& shipName, quint16 whichTransporter,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_Count = count;
	m_Crewman = crewman;
	m_ShipId = shipId;
	m_ShipName = shipName;
	m_WhichTransporter = whichTransporter;
}

AuCommBeamTransporter::AuCommBeamTransporter()
{
	m_BeamDir = 0;
	m_Count = 0;
	m_WhichTransporter = 0;
}

AuCommBeamTransporter::AuCommBeamTransporter(const AuCommBeamTransporter& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_Count = c.m_Count;
	m_Crewman = c.m_Crewman;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
	m_WhichTransporter = c.m_WhichTransporter;
}

AuCommPacket* AuCommBeamTransporter::Clone() const
{
	return new AuCommBeamTransporter(*this);
}

QString AuCommBeamTransporter::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamTransporter::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_Count;
	out << m_Crewman;
	out << m_ShipId;
	out << m_ShipName;
	out << m_WhichTransporter;
}

void AuCommBeamTransporter::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_Count;
	in >> m_Crewman;
	in >> m_ShipId;
	in >> m_ShipName;
	in >> m_WhichTransporter;
}

AuIdList AuCommBeamTransporter::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamTransporter::Text() const
{
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to transporter %3 aboard %2, %1 to beam away.")
			.arg(m_Count)
			.arg(m_ShipName)
			.arg(m_WhichTransporter);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to transporter %3 aboard %2, %1 to beam aboard.")
			.arg(m_Count)
			.arg(m_ShipName)
			.arg(m_WhichTransporter);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamTransporter::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehicleOver::AuCommBeamVehicleOver(VEHICLE_SPOT_TYPE spotType, const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SpotType = spotType;
}

AuCommBeamVehicleOver::AuCommBeamVehicleOver()
{
	m_SpotType = SPOT_Invalid;
}

AuCommBeamVehicleOver::AuCommBeamVehicleOver(const AuCommBeamVehicleOver& c) : AuCommPacket(c)
{
	m_SpotType = c.m_SpotType;
}

AuCommPacket* AuCommBeamVehicleOver::Clone() const
{
	return new AuCommBeamVehicleOver(*this);
}

QString AuCommBeamVehicleOver::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehicleOver::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SpotType;
}

void AuCommBeamVehicleOver::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SpotType;
}

// Returns the text of the comm message.
QString AuCommBeamVehicleOver::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(Scope() == AuCommStdChannel::Intercom)
		return QObject::tr("Beam my %1 to parking.").arg(spotType);
	else if(Scope() == AuCommStdChannel::CrewMates)
		return QObject::tr("Beam my %1 aboard.").arg(spotType);
	else
		return QObject::tr("I request a %1 transporter beam to board your vessel.").arg(spotType);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehicleOver::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehicleAtom::AuCommBeamVehicleAtom(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, const AuId& cityId,
		const QString& cityName, TRANSPORTER_TARGET target, const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_CityId = cityId;
	m_CityName = cityName;
	m_Crewman = crewman;
	m_SpotType = spotType;
	m_Target = target;
}

AuCommBeamVehicleAtom::AuCommBeamVehicleAtom()
{
	m_BeamDir = 0;
	m_SpotType = SPOT_Invalid;
	m_Target = 0;
}

AuCommBeamVehicleAtom::AuCommBeamVehicleAtom(const AuCommBeamVehicleAtom& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_CityId = c.m_CityId;
	m_CityName = c.m_CityName;
	m_SpotType = c.m_SpotType;
	m_Crewman = c.m_Crewman;
	m_Target = c.m_Target;
}

AuCommPacket* AuCommBeamVehicleAtom::Clone() const
{
	return new AuCommBeamVehicleAtom(*this);
}

QString AuCommBeamVehicleAtom::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehicleAtom::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_CityId;
	in >> m_CityName;
	in >> m_SpotType;
	in >> m_Crewman;
	in >> m_Target;
}

void AuCommBeamVehicleAtom::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_CityId;
	out << m_CityName;
	out << m_SpotType;
	out << m_Crewman;
	out << m_Target;
}

AuIdList AuCommBeamVehicleAtom::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamVehicleAtom::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2, %1 to beam ashore.")
			.arg(spotType)
			.arg(m_CityName);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2, %1 to beam aboard.")
			.arg(spotType)
			.arg(m_CityName);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehicleAtom::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehicleCoordinate::AuCommBeamVehicleCoordinate(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, float x,
		float y, float z, const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_SpotType = spotType;
	m_Crewman = crewman;
	m_X = x;
	m_Y = y;
	m_Z = z;
}

AuCommBeamVehicleCoordinate::AuCommBeamVehicleCoordinate()
{
	m_BeamDir = 0;
	m_SpotType = SPOT_Invalid;
	m_X = 0;
	m_Y = 0;
	m_Z = 0;
}

AuCommBeamVehicleCoordinate::AuCommBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_SpotType = c.m_SpotType;
	m_Crewman = c.m_Crewman;
	m_X = c.m_X;
	m_Y = c.m_Y;
	m_Z = c.m_Z;
}

AuCommPacket* AuCommBeamVehicleCoordinate::Clone() const
{
	return new AuCommBeamVehicleCoordinate(*this);
}

QString AuCommBeamVehicleCoordinate::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehicleCoordinate::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_SpotType;
	in >> m_Crewman;
	in >> m_X;
	in >> m_Y;
	in >> m_Z;
}

void AuCommBeamVehicleCoordinate::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_SpotType;
	out << m_Crewman;
	out << m_X;
	out << m_Y;
	out << m_Z;
}

AuIdList AuCommBeamVehicleCoordinate::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamVehicleCoordinate::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to coordinates (%L2, %L3, %L4)m relative to the ship, %1 to beam away.")
			.arg(spotType)
			.arg(m_X)
			.arg(m_Y)
			.arg(m_Z);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to coordinates (%L2, %L3, %L4)m relative to the ship, %1 to beam aboard.")
			.arg(spotType)
			.arg(m_X)
			.arg(m_Y)
			.arg(m_Z);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehicleCoordinate::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehiclePlanet::AuCommBeamVehiclePlanet(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, float longitude,
		float latitude, const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_SpotType = spotType;
	m_Crewman = crewman;
	m_Latitude = latitude;
	m_Longitude = longitude;
}

AuCommBeamVehiclePlanet::AuCommBeamVehiclePlanet()
{
	m_BeamDir = 0;
	m_SpotType = SPOT_Invalid;
	m_Latitude = 0;
	m_Longitude = 0;
}

AuCommBeamVehiclePlanet::AuCommBeamVehiclePlanet(const AuCommBeamVehiclePlanet& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_SpotType = c.m_SpotType;
	m_Crewman = c.m_Crewman;
	m_Latitude = c.m_Latitude;
	m_Longitude = c.m_Longitude;
}

AuCommPacket* AuCommBeamVehiclePlanet::Clone() const
{
	return new AuCommBeamVehiclePlanet(*this);
}

QString AuCommBeamVehiclePlanet::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehiclePlanet::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_SpotType;
	in >> m_Crewman;
	in >> m_Latitude;
	in >> m_Longitude;
}

void AuCommBeamVehiclePlanet::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_SpotType;
	out << m_Crewman;
	out << m_Latitude;
	out << m_Longitude;
}

AuIdList AuCommBeamVehiclePlanet::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamVehiclePlanet::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2 on the planet's surface, %1 to beam ashore.")
			.arg(spotType)
			.arg(AuGlobeLL::ToString(m_Longitude, m_Latitude));
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2 on the planet's surface, %1 to beam aboard.")
			.arg(spotType)
			.arg(AuGlobeLL::ToString(m_Longitude, m_Latitude));
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehiclePlanet::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehicleShip::AuCommBeamVehicleShip(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, const AuId& shipId,
		const QString& shipName, const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_SpotType = spotType;
	m_Crewman = crewman;
	m_ShipId = shipId;
	m_ShipName = shipName;
}

AuCommBeamVehicleShip::AuCommBeamVehicleShip()
{
	m_BeamDir = 0;
	m_SpotType = SPOT_Invalid;
}

AuCommBeamVehicleShip::AuCommBeamVehicleShip(const AuCommBeamVehicleShip& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_SpotType = c.m_SpotType;
	m_Crewman = c.m_Crewman;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
}

AuCommPacket* AuCommBeamVehicleShip::Clone() const
{
	return new AuCommBeamVehicleShip(*this);
}

QString AuCommBeamVehicleShip::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehicleShip::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_SpotType;
	in >> m_Crewman;
	in >> m_ShipId;
	in >> m_ShipName;
}

void AuCommBeamVehicleShip::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_SpotType;
	out << m_Crewman;
	out << m_ShipId;
	out << m_ShipName;
}

AuIdList AuCommBeamVehicleShip::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamVehicleShip::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to %2, %1 to beam away.")
			.arg(spotType)
			.arg(m_ShipName);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to %2, %1 to beam aboard.")
			.arg(spotType)
			.arg(m_ShipName);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehicleShip::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBeamVehicleTransporter::AuCommBeamVehicleTransporter(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir,
		const AuId& shipId, const QString& shipName, quint16 whichTransporter,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamDir = dir;
	m_SpotType = spotType;
	m_Crewman = crewman;
	m_ShipId = shipId;
	m_ShipName = shipName;
	m_WhichTransporter = whichTransporter;
}

AuCommBeamVehicleTransporter::AuCommBeamVehicleTransporter()
{
	m_BeamDir = 0;
	m_SpotType = SPOT_Invalid;
	m_WhichTransporter = 0;
}

AuCommBeamVehicleTransporter::AuCommBeamVehicleTransporter(const AuCommBeamVehicleTransporter& c) : AuCommPacket(c)
{
	m_BeamDir = c.m_BeamDir;
	m_SpotType = c.m_SpotType;
	m_Crewman = c.m_Crewman;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
	m_WhichTransporter = c.m_WhichTransporter;
}

AuCommPacket* AuCommBeamVehicleTransporter::Clone() const
{
	return new AuCommBeamVehicleTransporter(*this);
}

QString AuCommBeamVehicleTransporter::Icon() const
{
	return CommPacketIcon();
}

void AuCommBeamVehicleTransporter::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BeamDir;
	out << m_SpotType;
	out << m_Crewman;
	out << m_ShipId;
	out << m_ShipName;
	out << m_WhichTransporter;
}

void AuCommBeamVehicleTransporter::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BeamDir;
	in >> m_SpotType;
	in >> m_Crewman;
	in >> m_ShipId;
	in >> m_ShipName;
	in >> m_WhichTransporter;
}

AuIdList AuCommBeamVehicleTransporter::Recipients() const
{
	AuIdList recipients;
	recipients.append(m_Crewman);
	return recipients;
}

// Returns the text of the comm message.
QString AuCommBeamVehicleTransporter::Text() const
{
	QString spotType = VehicleSpotTypeName(SpotType());;
	if(m_BeamDir == 0)
	{
		// Beam Down
		return QObject::tr("Lock on to transporter %3 aboard %2, %1 to beam away.")
			.arg(spotType)
			.arg(m_ShipName)
			.arg(m_WhichTransporter);
	}
	else
	{
		// Beam Up
		return QObject::tr("Lock on to transporter %3 aboard %2, %1 to beam aboard.")
			.arg(spotType)
			.arg(m_ShipName)
			.arg(m_WhichTransporter);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBeamVehicleTransporter::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBid::AuCommBid(const AuItem& item, const QString& empireOfOrigin, quint32 price,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
	m_Price = price;
}

AuCommBid::AuCommBid()
{
	//m_EmpireOfOrigin
	m_Item = 0;
	m_Price = 0;
}

AuCommBid::AuCommBid(const AuCommBid& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_Price = c.m_Price;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommBid::~AuCommBid()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommBid::Clone() const
{
	return new AuCommBid(*this);
}

bool AuCommBid::CorrespondsTo(const AuCommPacket* commPacket) const
{
	const AuCommBid* bid = dynamic_cast<const AuCommBid*>(commPacket);
	if(bid)
	{
		if(!m_Item->IsItem(*bid->Item()))
			return false;
		if(Price() != bid->Price())
			return false;
		return true;
	}
	else
	{
		const AuCommAccept* accept = dynamic_cast<const AuCommAccept*>(commPacket);
		if(accept)
		{
			if(!m_Item->IsItem(*accept->Item()))
				return false;
			if(Price() != accept->Price())
				return false;
			return true;
		}
	}
	return false;
}

QString AuCommBid::Icon() const
{
	return CommPacketIcon();
}

void AuCommBid::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;
	in >> m_Price;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommBid::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_Price;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommBid::Text() const
{
	Q_ASSERT(m_Item);
	QString text;
	if(m_Price == 0)
	{
		text = QObject::tr("%1. I will take it for free.")
			.arg(m_Item->ListDesc(m_EmpireOfOrigin));
	}
	else
	{
		text = QObject::tr("%1. I offer %L6%7.")
			.arg(m_Item->ListDesc(m_EmpireOfOrigin))
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol());
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBid::Type() const
{
	return CommPacketType();
}

const QString& AuCommBid::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBidBarter::AuCommBidBarter(const AuItem& item, const QString& empireOfOrigin, const AuItem& offerItem,
		const QString& offerEmpireOfOrigin,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
	m_OfferEmpireOfOrigin = offerEmpireOfOrigin;
	m_OfferItem = offerItem.Clone();
}

AuCommBidBarter::AuCommBidBarter()
{
	m_Item = 0;
	m_OfferItem = 0;
	//m_EmpireOfOrigin
	//m_OfferEmpireOfOrigin
}

AuCommBidBarter::AuCommBidBarter(const AuCommBidBarter& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_OfferEmpireOfOrigin = c.m_OfferEmpireOfOrigin;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;

	if(c.m_OfferItem)
		m_OfferItem = c.m_OfferItem->Clone();
	else
		m_OfferItem = 0;
}

AuCommBidBarter::~AuCommBidBarter()
{
	if(m_Item)
		delete m_Item;
	if(m_OfferItem)
		delete m_OfferItem;
}

AuCommPacket* AuCommBidBarter::Clone() const
{
	return new AuCommBidBarter(*this);
}

bool AuCommBidBarter::CorrespondsTo(const AuCommPacket* commPacket) const
{
	const AuCommBidBarter* bid = dynamic_cast<const AuCommBidBarter*>(commPacket);
	if(bid)
	{
		if(!m_Item->IsItem(*bid->Item()))
			return false;
		if(!m_OfferItem->IsItem(*bid->OfferItem()))
			return false;
		return true;
	}
	else
	{
		const AuCommAcceptBarter* accept = dynamic_cast<const AuCommAcceptBarter*>(commPacket);
		if(accept)
		{
			if(!m_Item->IsItem(*accept->Item()))
				return false;
			if(!m_OfferItem->IsItem(*accept->OfferItem()))
				return false;
			return true;
		}
	}
	return false;
}

QString AuCommBidBarter::Icon() const
{
	return CommPacketIcon();
}

void AuCommBidBarter::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;
	in >> m_OfferEmpireOfOrigin;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;

	if(m_OfferItem)
		delete m_OfferItem;

	in >> itemType;
	if(itemType)
	{
		m_OfferItem = AuItem::Create(itemType);
		if(m_OfferItem)
			in >> *m_OfferItem;
	}
	else
		m_OfferItem = 0;
}

void AuCommBidBarter::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_OfferEmpireOfOrigin;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;

	if(m_OfferItem)
	{
		out << m_OfferItem->Type();
		out << *m_OfferItem;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommBidBarter::Text() const
{
	Q_ASSERT(m_Item);
	Q_ASSERT(m_OfferItem);
	QString text = QObject::tr("%1. I offer to trade %2.")
		.arg(m_Item->ListDesc(m_EmpireOfOrigin))
		.arg(m_OfferItem->ListDesc(m_OfferEmpireOfOrigin));

	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBidBarter::Type() const
{
	return CommPacketType();
}

const QString& AuCommBidBarter::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBoardCrew::AuCommBoardCrew(const AuId& loaderId, quint8 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommBoardCrew::AuCommBoardCrew()
{
	m_Count = 0;
}

AuCommBoardCrew::AuCommBoardCrew(const AuCommBoardCrew& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommBoardCrew::Clone() const
{
	return new AuCommBoardCrew(*this);
}

QString AuCommBoardCrew::Icon() const
{
	return CommPacketIcon();
}

void AuCommBoardCrew::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Count;
	in >> m_LoaderId;
}

void AuCommBoardCrew::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Count;
	out << m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommBoardCrew::Text() const
{
	QString text;
	if(m_Count == 1)
		text = QObject::tr("Now boarding. One berth is available for a crew member.");
	else
		text = QObject::tr("Now boarding. %1 berths are available for crew members.")
			.arg(m_Count);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBoardCrew::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBoardOfficers::AuCommBoardOfficers(const AuId& loaderId, quint8 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommBoardOfficers::AuCommBoardOfficers()
{
	m_Count = 0;
}

AuCommBoardOfficers::AuCommBoardOfficers(const AuCommBoardOfficers& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommBoardOfficers::Clone() const
{
	return new AuCommBoardOfficers(*this);
}

QString AuCommBoardOfficers::Icon() const
{
	return CommPacketIcon();
}

void AuCommBoardOfficers::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Count;
	in >> m_LoaderId;
}

void AuCommBoardOfficers::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Count;
	out << m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommBoardOfficers::Text() const
{
	QString text;
	if(m_Count == 1)
		text = QObject::tr("Now boarding. One berth is available for an officer.");
	else
		text = QObject::tr("Now boarding. %1 berths are available for officers.")
			.arg(m_Count);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBoardOfficers::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBoardPassengers::AuCommBoardPassengers(const AuId& loaderId, quint16 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommBoardPassengers::AuCommBoardPassengers()
{
	m_Count = 0;
}

AuCommBoardPassengers::AuCommBoardPassengers(const AuCommBoardPassengers& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommBoardPassengers::Clone() const
{
	return new AuCommBoardPassengers(*this);
}

QString AuCommBoardPassengers::Icon() const
{
	return CommPacketIcon();
}

void AuCommBoardPassengers::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)0;	// Used to be 8 bit count. A 0 was invalid so it now flags the change to a 16 bit value.
	out << m_Count;
	out << m_LoaderId;
}

void AuCommBoardPassengers::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ocount;
	in >> ocount;
	if(ocount)
		m_Count = ocount;
	else
		in >> m_Count;
	in >> m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommBoardPassengers::Text() const
{
	QString text;
	if(m_Count == 1)
		text = QObject::tr("Now boarding. One berth is available for a passenger.");
	else
		text = QObject::tr("Now boarding. %L1 berths are available for passengers.")
			.arg(m_Count);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBoardPassengers::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBoardTroops::AuCommBoardTroops(const AuId& loaderId, quint16 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommBoardTroops::AuCommBoardTroops()
{
	m_Count = 0;
}

AuCommBoardTroops::AuCommBoardTroops(const AuCommBoardTroops& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommBoardTroops::Clone() const
{
	return new AuCommBoardTroops(*this);
}

QString AuCommBoardTroops::Icon() const
{
	return CommPacketIcon();
}

void AuCommBoardTroops::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)0;	// Used to be 8 bit count. A 0 was invalid so it now flags the change to a 16 bit value.
	out << m_Count;
	out << m_LoaderId;
}

void AuCommBoardTroops::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ocount;
	in >> ocount;
	if(ocount)
		m_Count = ocount;
	else
		in >> m_Count;
	in >> m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommBoardTroops::Text() const
{
	QString text;
	if(m_Count == 1)
		text = QObject::tr("Now boarding. One berth is available for a troop.");
	else
		text = QObject::tr("Now boarding. %L1 berths are available for troops.")
			.arg(m_Count);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBoardTroops::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChat::AuCommChat(const QString& text,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Text = text;
}

AuCommChat::AuCommChat()
{
}

AuCommChat::AuCommChat(const AuCommChat& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommChat::Clone() const
{
	return new AuCommChat(*this);
}

QString AuCommChat::Icon() const
{
	return CommPacketIcon();
}

QString AuCommChat::Icon2() const
{
	QUrl url = UrlFromString(m_Text);
	if(url != QUrl())
	{
		// Watch for our stuff.
		if(m_Text.toUpper().contains("HAZERON.COM") || m_Text.toUpper().contains("DIGIPROSE.COM"))
			return QString(":data/OrderGame.png");
		else
			return QString(":data/OrderWeb.png");
	}
	return QString();
}

void AuCommChat::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Text;
	if(m_Text.length() > MaxChatText())
		m_Text = m_Text.left(MaxChatText()).trimmed();
	else
		m_Text = m_Text.trimmed();
}

void AuCommChat::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_Text.length() > MaxChatText())
		out << m_Text.left(MaxChatText()).trimmed();
	else
		out << m_Text.trimmed();
}

// Returns the text of the comm message.
QString AuCommChat::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChat::Type() const
{
	return CommPacketType();
}

bool AuCommChat::IsValid() const
{
	if(m_Text.isEmpty())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChatAudio::AuCommChatAudio(const QString& text, quint32 storyInstance, const QByteArray& soundData,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SoundData = soundData;
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommChatAudio::AuCommChatAudio()
{
	m_StoryInstance = 0;
}

AuCommChatAudio::AuCommChatAudio(const AuCommChatAudio& c) : AuCommPacket(c), m_SoundData(c.m_SoundData)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommChatAudio::Clone() const
{
	return new AuCommChatAudio(*this);
}

QString AuCommChatAudio::Icon() const
{
	return CommPacketIcon();
}

void AuCommChatAudio::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SoundData;
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommChatAudio::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SoundData;
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommChatAudio::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChatAudio::Type() const
{
	return CommPacketType();
}

quint32 AuCommChatAudio::StoryInstance() const
{
	return m_StoryInstance;
}

bool AuCommChatAudio::IsDataLarge() const
{
	return m_SoundData.count() > 63*1024;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChatImage::AuCommChatImage(const QString& title, const QString& text, quint32 storyInstance, const QImage& image,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Image = image;
	m_StoryInstance = storyInstance;
	m_Text = text;
	m_Title = title;
}

AuCommChatImage::AuCommChatImage()
{
	m_StoryInstance = 0;
}

AuCommChatImage::AuCommChatImage(const AuCommChatImage& c) : AuCommPacket(c), m_Image(c.m_Image)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
	m_Title = c.m_Title;
}

AuCommPacket* AuCommChatImage::Clone() const
{
	return new AuCommChatImage(*this);
}

QString AuCommChatImage::Icon() const
{
	return CommPacketIcon();
}

void AuCommChatImage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Image;
	in >> m_StoryInstance;
	in >> m_Text;
	in >> m_Title;
}

void AuCommChatImage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Image;
	out << m_StoryInstance;
	out << m_Text;
	out << m_Title;
}

// Returns the text of the comm message.
QString AuCommChatImage::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChatImage::Type() const
{
	return CommPacketType();
}

quint32 AuCommChatImage::StoryInstance() const
{
	return m_StoryInstance;
}

bool AuCommChatImage::IsDataLarge() const
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	return m_Image.sizeInBytes() > 63*1024;
#else
	return m_Image.byteCount() > 63*1024;
#endif
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChatChoice::AuCommChatChoice(const AuId& toId, const QString& text, quint32 storyInstance, qint8 choiceId,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ChoiceId = choiceId;
	m_StoryInstance = storyInstance;
	m_Text = text;
	m_ToId = toId;
}

AuCommChatChoice::AuCommChatChoice()
{
	m_ChoiceId = 0;
	m_StoryInstance = 0;
}

AuCommChatChoice::AuCommChatChoice(const AuCommChatChoice& c) : AuCommPacket(c)
{
	m_ChoiceId = c.m_ChoiceId;
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommChatChoice::Clone() const
{
	return new AuCommChatChoice(*this);
}

QString AuCommChatChoice::Icon() const
{
	return CommPacketIcon();
}

void AuCommChatChoice::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ChoiceId;
	in >> m_StoryInstance;
	in >> m_Text;
	in >> m_ToId;
}

void AuCommChatChoice::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ChoiceId;
	out << m_StoryInstance;
	out << m_Text;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommChatChoice::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChatChoice::Type() const
{
	return CommPacketType();
}

quint32 AuCommChatChoice::StoryInstance() const
{
	return m_StoryInstance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChatContinued::AuCommChatContinued(const QString& text,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Text = text;
}

AuCommChatContinued::AuCommChatContinued()
{
}

AuCommChatContinued::AuCommChatContinued(const AuCommChatContinued& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommChatContinued::Clone() const
{
	return new AuCommChatContinued(*this);
}

QString AuCommChatContinued::Icon() const
{
	return CommPacketIcon();
}

void AuCommChatContinued::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Text;
	if(m_Text.length() > MaxChatText())
		m_Text = m_Text.left(MaxChatText()).trimmed();
	else
		m_Text = m_Text.trimmed();
}

void AuCommChatContinued::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_Text.length() > MaxChatText())
		out << m_Text.left(MaxChatText()).trimmed();
	else
		out << m_Text.trimmed();
}

// Returns the text of the comm message.
QString AuCommChatContinued::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChatContinued::Type() const
{
	return CommPacketType();
}

bool AuCommChatContinued::IsValid() const
{
	if(m_Text.isEmpty())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChatStory::AuCommChatStory(const QString& text, quint32 storyInstance,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommChatStory::AuCommChatStory()
{
	m_StoryInstance = 0;
}

AuCommChatStory::AuCommChatStory(const AuCommChatStory& c) : AuCommPacket(c)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommChatStory::Clone() const
{
	return new AuCommChatStory(*this);
}

QString AuCommChatStory::Icon() const
{
	return CommPacketIcon();
}

void AuCommChatStory::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommChatStory::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommChatStory::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChatStory::Type() const
{
	return CommPacketType();
}

quint32 AuCommChatStory::StoryInstance() const
{
	return m_StoryInstance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCitizenOrder::AuCommCitizenOrder(const AuIdList& orderTo, CITIZEN_ORDER order, const AuId& orderTarget, double orderX,
		double orderY, double orderZ, const QString& orderText,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Order = order;
	m_OrderTarget = orderTarget;
	m_OrderText = orderText;
	m_OrderX = orderX;
	m_OrderY = orderY;
	m_OrderZ = orderZ;
	m_Recipients = orderTo;
}

AuCommCitizenOrder::AuCommCitizenOrder()
{
	m_Order = ORDER_Stay;
	m_OrderX = 0;
	m_OrderY = 0;
	m_OrderZ = 0;
}

AuCommCitizenOrder::AuCommCitizenOrder(const AuCommCitizenOrder& c) : AuCommPacket(c), m_Recipients(c.m_Recipients)
{
	m_Order = c.m_Order;
	m_OrderTarget = c.m_OrderTarget;
	m_OrderText = c.m_OrderText;
	m_OrderX = c.m_OrderX;
	m_OrderY = c.m_OrderY;
	m_OrderZ = c.m_OrderZ;
}

AuCommPacket* AuCommCitizenOrder::Clone() const
{
	return new AuCommCitizenOrder(*this);
}

QString AuCommCitizenOrder::Icon() const
{
	return CommPacketIcon();
}

void AuCommCitizenOrder::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_Order;
	out << m_OrderTarget;
	out << m_OrderX;
	out << m_OrderY;
	out << m_OrderZ;

	out << (quint16)m_Recipients.count();
	for(AuIdList::const_iterator ri = m_Recipients.constBegin(); ri != m_Recipients.constEnd(); ri++)
		out << *ri;

	if(m_Order == ORDER_GoToRoom)
		out << m_OrderText;
}

void AuCommCitizenOrder::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ui8;
	in >> ui8;
	m_Order = (CITIZEN_ORDER)ui8;
	in >> m_OrderTarget;
	in >> m_OrderX;
	in >> m_OrderY;
	in >> m_OrderZ;

	m_Recipients.clear();
	quint16 ui16;
	in >> ui16;
	AuId recipientId;
	for(; ui16; ui16--)
	{
		in >> recipientId;
		m_Recipients.append(recipientId);
	}

	if(m_Order == ORDER_GoToRoom)
		in >> m_OrderText;
}

AuIdList AuCommCitizenOrder::Recipients() const
{
	return m_Recipients;
}

// Returns the text of the comm message.
QString AuCommCitizenOrder::Text() const
{
	if(m_Recipients.count() > 1)
	{
		switch(m_Order)
		{
#if defined(QT_NO_DEBUG)			// I want a compiler warning in the debug build if I forget to handle a nu order here.
		default:
#endif
		case ORDER_Sleep:
		case ORDER_SleepDayDream:
		case ORDER_SleepEat:
		case ORDER_SleepExercise:
		case ORDER_SleepGame:
		case ORDER_SleepLetter:
		case ORDER_SleepMovie:
		case ORDER_SleepMusic:
		case ORDER_SleepPine:
		case ORDER_SleepPhone:
		case ORDER_SleepRead:
		case ORDER_SleepTalk:
		case ORDER_SleepText:
		case ORDER_SleepTV:
		case ORDER_SleepWeb:
		case ORDER_DamageReport:
		case ORDER_SurveyScanPlanet:
		case ORDER_SurveyScanSystem:
		case ORDER_TacticalScan:
			// These orders cannot be issued directly to citizens.
			return QObject::tr("All of you do something!");
		case ORDER_HengeCeremony:
			return QObject::tr("Do the henge ceremony.");
		case ORDER_Stay:
			return QObject::tr("%1 units stay here.").arg(m_Recipients.count());
		case ORDER_AttackTarget:
			return QObject::tr("%1 units attack that target.").arg(m_Recipients.count());
		case ORDER_BattleStation:
			return QObject::tr("%1 units man stations.").arg(m_Recipients.count());
		case ORDER_ComeHere:
			return QObject::tr("%1 units come here.").arg(m_Recipients.count());
		case ORDER_ComeHereExactly:
			return QObject::tr("%1 units come right here.").arg(m_Recipients.count());
		case ORDER_FollowMe:
			return QObject::tr("%1 units follow me.").arg(m_Recipients.count());
		case ORDER_FollowTarget:
			return QObject::tr("%1 units follow that target.").arg(m_Recipients.count());
		case ORDER_Garrison:
			return QObject::tr("%1 units garrison your location.").arg(m_Recipients.count());
		case ORDER_GetIn:
			return QObject::tr("%1 units get in.").arg(m_Recipients.count());
		case ORDER_GetOut:
			return QObject::tr("%1 units get out.").arg(m_Recipients.count());
		case ORDER_GoToRoom:
			return QObject::tr("%1 units go to %2.").arg(m_Recipients.count()).arg(m_OrderText);
		case ORDER_GoToTarget:
			return QObject::tr("%1 units go to that location.").arg(m_Recipients.count());
		case ORDER_GoToTargetExactly:
			return QObject::tr("%1 units go right to that location.").arg(m_Recipients.count());
		case ORDER_CloseDoor:
			return QObject::tr("%1 units close the door.").arg(m_Recipients.count());
		case ORDER_OpenDoor:
			return QObject::tr("%1 units open the door.").arg(m_Recipients.count());
		case ORDER_GetItem:
			return QObject::tr("%1 units get that.").arg(m_Recipients.count());
		case ORDER_EnterSurgery:
			return QObject::tr("%1 units report to sick bay for a checkup.").arg(m_Recipients.count());
		case ORDER_TakeCommand:
			return QObject::tr("%1 units, decide who among you will take the con.").arg(m_Recipients.count());
		case ORDER_ReleaseCommand:
			return QObject::tr("%1 units, you are relieved of the con.").arg(m_Recipients.count());
		case ORDER_DropRider:
			return QObject::tr("%1 units, drop your riders.").arg(m_Recipients.count());
		case ORDER_MountMe:
			return QObject::tr("%1 units, mount me.").arg(m_Recipients.count());
		case ORDER_MountTarget:
			return QObject::tr("%1 units, mount that target.").arg(m_Recipients.count());
		}
	}
	else
	{
		switch(m_Order)
		{
#if defined(QT_NO_DEBUG)			// I want a compiler warning in the debug build if I forget to handle a nu order here.
		default:
#endif
		case ORDER_Sleep:
		case ORDER_SleepDayDream:
		case ORDER_SleepEat:
		case ORDER_SleepExercise:
		case ORDER_SleepGame:
		case ORDER_SleepLetter:
		case ORDER_SleepMovie:
		case ORDER_SleepMusic:
		case ORDER_SleepPine:
		case ORDER_SleepPhone:
		case ORDER_SleepRead:
		case ORDER_SleepTalk:
		case ORDER_SleepText:
		case ORDER_SleepTV:
		case ORDER_SleepWeb:
		case ORDER_DamageReport:
		case ORDER_SurveyScanPlanet:
		case ORDER_SurveyScanSystem:
		case ORDER_TacticalScan:
			// These orders cannot be issued directly to citizens.
			return QObject::tr("Do something!");
		case ORDER_HengeCeremony:
			return QObject::tr("Do the henge ceremony.");
		case ORDER_Stay:
			return QObject::tr("Stay here.");
		case ORDER_AttackTarget:
			return QObject::tr("Attack that target.");
		case ORDER_BattleStation:
			return QObject::tr("Man a station.");
		case ORDER_ComeHere:
			return QObject::tr("Come here.");
		case ORDER_ComeHereExactly:
			return QObject::tr("Come right here.");
		case ORDER_FollowMe:
			return QObject::tr("Follow me.");
		case ORDER_FollowTarget:
			return QObject::tr("Follow that target.");
		case ORDER_Garrison:
			return QObject::tr("Garrison your location.");
		case ORDER_GetIn:
			return QObject::tr("Get in.");
		case ORDER_GetOut:
			return QObject::tr("Get out.");
		case ORDER_GoToRoom:
			return QObject::tr("Go to %2.").arg(m_OrderText);
		case ORDER_GoToTarget:
			return QObject::tr("Go to that location.");
		case ORDER_GoToTargetExactly:
			return QObject::tr("Go right to that location.");
		case ORDER_CloseDoor:
			return QObject::tr("Close the door.");
		case ORDER_OpenDoor:
			return QObject::tr("Open the door.");
		case ORDER_GetItem:
			return QObject::tr("Get that.");
		case ORDER_EnterSurgery:
			return QObject::tr("Report to sick bay for a checkup.");
		case ORDER_TakeCommand:
			return QObject::tr("You have the con.");
		case ORDER_ReleaseCommand:
			return QObject::tr("You are relieved of the con.");
		case ORDER_DropRider:
			return QObject::tr("Drop your rider.");
		case ORDER_MountMe:
			return QObject::tr("Mount me.");
		case ORDER_MountTarget:
			return QObject::tr("Mount that target.");
		}
	}
	return "Order?";
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCitizenOrder::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCitizenPosture::AuCommCitizenPosture(const AuIdList& orderTo, CITIZEN_POSTURE posture,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Posture = posture;
	m_Recipients = orderTo;
}

AuCommCitizenPosture::AuCommCitizenPosture()
{
	m_Posture = POSTURE_Offensive;
}

AuCommCitizenPosture::AuCommCitizenPosture(const AuCommCitizenPosture& c) : AuCommPacket(c), m_Recipients(c.m_Recipients)
{
	m_Posture = c.m_Posture;
}

AuCommPacket* AuCommCitizenPosture::Clone() const
{
	return new AuCommCitizenPosture(*this);
}

QString AuCommCitizenPosture::Icon() const
{
	return CommPacketIcon();
}

void AuCommCitizenPosture::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;
	in >> ui8;
	m_Posture = (CITIZEN_POSTURE)ui8;

	m_Recipients.clear();
	quint16 ui16;
	in >> ui16;
	AuId recipientId;
	for(; ui16; ui16--)
	{
		in >> recipientId;
		m_Recipients.append(recipientId);
	}
}

void AuCommCitizenPosture::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_Posture;

	out << (quint16)m_Recipients.count();
	for(AuIdList::const_iterator ri = m_Recipients.constBegin(); ri != m_Recipients.constEnd(); ri++)
		out << *ri;
}

AuIdList AuCommCitizenPosture::Recipients() const
{
	return m_Recipients;
}

// Returns the text of the comm message.
QString AuCommCitizenPosture::Text() const
{
	if(m_Recipients.count() > 1)
	{
		switch(m_Posture)
		{
		case POSTURE_Berserk:
			return QObject::tr("%1 units be merciless. Attack all enemy units including civilians and farm "
					"animals.").arg(m_Recipients.count());
		case POSTURE_DefendArea:
			return QObject::tr("%1 units be on guard. Attack enemy units that get close.").arg(m_Recipients.count());
		case POSTURE_DefendSelf:
			return QObject::tr("%1 units be quiet. Do not attack unless you are attacked.").arg(m_Recipients.count());
		case POSTURE_HoldFire:
			return QObject::tr("%1 units hold your fire. Do not attack enemy units for any reason.").arg(m_Recipients.count());
		case POSTURE_Offensive:
			return QObject::tr("%1 units be alert. Attack enemy units at will.").arg(m_Recipients.count());
		}
	}
	else
	{
		switch(m_Posture)
		{
		case POSTURE_Berserk:
			return QObject::tr("Be merciless. Attack all enemy units including civilians and farm animals.");
		case POSTURE_DefendArea:
			return QObject::tr("Be on guard. Attack enemy units that get close.");
		case POSTURE_DefendSelf:
			return QObject::tr("Be quiet. Do not attack unless you are attacked.");
		case POSTURE_HoldFire:
			return QObject::tr("Hold your fire. Do not attack enemy units for any reason.");
		case POSTURE_Offensive:
			return QObject::tr("Be alert. Attack enemy units at will.");
		}
	}
	return "Posture?";
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCitizenPosture::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCityChat::AuCommCityChat(const QString& text,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Text = text;
}

AuCommCityChat::AuCommCityChat()
{
}

AuCommCityChat::AuCommCityChat(const AuCommCityChat& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommCityChat::Clone() const
{
	return new AuCommCityChat(*this);
}

QString AuCommCityChat::Icon() const
{
	return CommPacketIcon();
}

void AuCommCityChat::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Text;
}

void AuCommCityChat::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommCityChat::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCityChat::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMessage::AuCommMessage(MESSAGE message,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Message = message;
}

AuCommMessage::AuCommMessage()
{
	m_Message = MessageCount;
}

AuCommMessage::AuCommMessage(const AuCommMessage& c) : AuCommPacket(c)
{
	m_Message = c.m_Message;
}

AuCommPacket* AuCommMessage::Clone() const
{
	return new AuCommMessage(*this);
}

QString AuCommMessage::Icon() const
{
	return CommPacketIcon();
}

void AuCommMessage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_Message = (MESSAGE)ui8;
}

void AuCommMessage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_Message;
}

// Returns the text of the comm message.
QString AuCommMessage::Text() const
{
	if(m_Message < 0 || m_Message >= MessageCount)
		return QString("Garbled transmission...ZZzzzttt...sounds faintly like your software needs updating.");

	const QString messages[MessageCount] =
	{
		QObject::tr("Spacecraft recovered from stasis. It will hail on the Crew channel when it arrives overhead."),	//ShipSummoned
		QObject::tr("Ship policy prevents automatic acceptance of your berth request, except while boarding. An officer of the ship could accept your request."),	//CannotAcceptBerthType,
		QObject::tr("The captain's berth is already occupied."),	//CaptBerthOccupied,
		QObject::tr("Ship empire policy prevents granting the captain's berth of fleet spacecraft to citizens of your empire."),	//CaptBerthRestricted,
		QObject::tr("The captain's berth on a fleet station is restricted to members of the station's empire."),	//CaptBerthRestrictedStation,
		QObject::tr("No empty crew berth is available."),	//NoCrewBerth,
		QObject::tr("No empty officer berth is available."),	//NoOfficerBerth,
		QObject::tr("No empty passenger berth is available."),	//NoPassengerBerth,
		QObject::tr("No empty troop berth is available."),	//NoTroopBerth,
		QObject::tr("A ship cannot grant an officer berth to a citizen of another empire."),	//OfficerBerthRestrictedEmpire,
		QObject::tr("You already have a berth of that type aboard this ship."),	//RequestorAlreadyHasBerthType,
		QObject::tr("The ship cannot grant that berth to a savage."),	//RequestorHasNoEmpire,
	};
	return messages[m_Message];
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMessage::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCityMessage::AuCommCityMessage(MESSAGE message,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Message = message;
}

AuCommCityMessage::AuCommCityMessage()
{
	m_Message = MessageCount;
}

AuCommCityMessage::AuCommCityMessage(const AuCommCityMessage& c) : AuCommPacket(c)
{
	m_Message = c.m_Message;
}

AuCommPacket* AuCommCityMessage::Clone() const
{
	return new AuCommCityMessage(*this);
}

QString AuCommCityMessage::Icon() const
{
	return CommPacketIcon();
}

void AuCommCityMessage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_Message = (MESSAGE)ui8;
}

void AuCommCityMessage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_Message;
}

// Returns the text of the comm message.
QString AuCommCityMessage::Text() const
{
	if(m_Message < 0 || m_Message >= MessageCount)
		return QString("Garbled transmission...ZZzzzttt...sounds faintly like your software needs updating.");

	const QString messages[MessageCount] =
	{
		QObject::tr("Upgrades must be installed while you are aboard a spacecraft."),
		QObject::tr("We are prohibited from upgrading your spacecraft."),
		QObject::tr("Only an officer of the spacecraft can purchase upgrades."),
		QObject::tr("To get upgrades installed, you must land on a road slab here or moor at a friendly station."),
		QObject::tr("To get upgrades installed here, you must land on a road slab in this city."),
		QObject::tr("We cannot upgrade your spacecraft while it is moving."),
		QObject::tr("We cannot upgrade your spacecraft while its sensors are on."),
		QObject::tr("We cannot upgrade your spacecraft while its shields are up."),
		QObject::tr("We cannot upgrade your spacecraft while its FTL drive is on."),
		QObject::tr("We cannot upgrade your spacecraft while its maneuver drive is on."),
		QObject::tr("We cannot upgrade your spacecraft while its power plant is on."),
		QObject::tr("You must move to a building or construction site here to sell personal items."),
		QObject::tr("I do not buy those here. Try a retail store or a place that uses or sells them."),
		QObject::tr("I do not buy those here. Try a cantina in a town where the locals eat those or a place that uses or sells them."),
		QObject::tr("I do not buy those here. Try a grocery in a town where the locals eat those or a place that uses or sells them."),
		QObject::tr("I do not buy those here. Try a design studio or a place that uses or sells them."),
		QObject::tr("I do not buy those here. Try a broker or a place that uses or sells them."),
		QObject::tr("I do not buy those here. Try a place that uses or sells them."),
		QObject::tr("We do not want those. We have no place to store any of them."),	// WeNoWantThose
		QObject::tr("You must move to a design studio here to sell blueprints."),
		QObject::tr("I have requested verification from the library, to confirm the blueprint is not already published and you are the architect. I should hear back soon. Please wait a moment and try again."),
		QObject::tr("That blueprint is already available in the library of our empire."),
		QObject::tr("You are not the architect of the blueprint. A blueprint may only be published by its architect."),
		QObject::tr("To buy cargo, you must be an officer of the ship."),
		QObject::tr("To buy cargo, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),
		QObject::tr("To buy cargo here, you must move within transporter range of this city or land on a road slab here."),
		QObject::tr("To buy cargo, you must land on a road slab here or move closer to a friendly station."),
		QObject::tr("To buy cargo here, you must land on a road slab in this city."),
		QObject::tr("You must move to a building here to buy personal items."),
		QObject::tr("I do not sell those here. Try a retail store or a place that makes them."),
		QObject::tr("I do not sell those here. Try a cantina in a town where the locals eat those or a place that makes them."),
		QObject::tr("I do not sell those here. Try a grocery in a town where the locals eat those or a place that makes them."),
		QObject::tr("I do not sell those here. Try a design studio or a place that makes them."),
		QObject::tr("I do not sell those here. Try a broker or a place that makes them."),
		QObject::tr("I do not sell those here. Try a place that makes them."),
		QObject::tr("Your gear has insufficient room to add any of the items."),
		QObject::tr("Your gear has insufficient room to add the item."),
		QObject::tr("You must move to a design studio here to buy blueprints."),
		QObject::tr("You must exit your spacecraft to buy blueprints."),
		QObject::tr("Your offer is too low."),
		QObject::tr("Your gear has insufficient room to add the blueprint."),
		QObject::tr("Our citizens are restricted from taking a berth aboard your ship."),
		QObject::tr("Only an officer of the spacecraft can load crew."),
		QObject::tr("Crew cannot be loaded in the Design Studio."),
		QObject::tr("Our population is too low. Nobody is willing to leave."),
		QObject::tr("There are no citizens here. This city is deserted."),
		QObject::tr("To load crew, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),	// MoveTransporterStationToBoardCrew,
		QObject::tr("To load crew here, you must move within transporter range of this city or land on a road slab here."),	// MoveTransporterToBoardCrew,
		QObject::tr("To load crew, you must land on a road slab here or move closer to a friendly station."),	// MoveStationToBoardCrew,
		QObject::tr("To load crew here, you must land on a road slab in this city."),	// MoveToBoardCrew,
		QObject::tr("Only an officer of the spacecraft can load officers."),
		QObject::tr("The officer from this planet is already deployed."),
		QObject::tr("This uninhabitable world cannot produce spacecraft officers."),
		QObject::tr("This city cannot produce spacecraft officers because we have no airport."),
		QObject::tr("This city cannot produce spacecraft officers because we have no university."),
		QObject::tr("This city cannot produce spacecraft officers because we have no spacecraft factory."),
		QObject::tr("Our officer is restricted from taking a berth aboard your ship."),
		QObject::tr("You must be in the chain of command of this world to load our officer aboard your ship."),
		QObject::tr("Officers cannot be loaded in the Design Studio."),
		QObject::tr("To load our officer, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load the officer here, you must move within transporter range of this city or land on a road slab here."),
		QObject::tr("To load our officer, you must land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load the officer here, you must land on a road slab in this city."),
		QObject::tr("Only an officer of the spacecraft can load passengers."),
		QObject::tr("Passengers cannot be loaded in the Design Studio."),
		QObject::tr("To load passengers, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load passengers here, you must move within transporter range of this city or land on a road slab here."),
		QObject::tr("To load passengers, you must land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load passengers here, you must land on a road slab in this city."),
		QObject::tr("Our citizens are restricted from entering your vehicle."),
		QObject::tr("Only the driver of the vehicle can load passengers."),
		QObject::tr("To load passengers here, your vehicle must be in this city."),
		QObject::tr("No troops are ready for deployment."),
		QObject::tr("Only an officer of the ship can load troops."),
		QObject::tr("Troops cannot be loaded in the Design Studio."),
		QObject::tr("To load troops, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load troops here, you must move within transporter range of this city or land on a road slab here."),
		QObject::tr("To load troops, you must land on a road slab here or move closer to a friendly station."),
		QObject::tr("To load troops here, you must land on a road slab in this city."),
		QObject::tr("Only an officer of the spacecraft can debark passengers."),
		QObject::tr("To debark passengers, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),
		QObject::tr("To debark passengers here, you must move within transporter range of this city or land on a road slab here."),
		QObject::tr("To debark passengers, you must land on a road slab here or move closer to a friendly station."),
		QObject::tr("To debark passengers here, you must land on a road slab in this city."),
		QObject::tr("Only the driver of the vehicle can debark passengers."),
		QObject::tr("To debark passengers here, you must be in this city."),
		QObject::tr("Your price is too high."),
		QObject::tr("Channel opened from city."),
		QObject::tr("Exterminator must be requested while you are aboard a spacecraft."),
		QObject::tr("Only an officer of the spacecraft can request an exterminator."),
		QObject::tr("Rat nest was removed."),
		QObject::tr("Rats were exterminated."),
		QObject::tr("Rats were exterminated. Rat nest was removed."),
		QObject::tr("We do not have an airport repair shop."),
		QObject::tr("Only people in this city's chain of command may receive an inventory."),
		QObject::tr("A reply was sent on a private channel."),	// ReportOnPrivateChannel
		QObject::tr("Repairs must be requested while you are aboard a spacecraft."),
		QObject::tr("We are prohibited from repairing your spacecraft."),
		QObject::tr("Only an officer of the spacecraft can purchase repairs."),
		QObject::tr("We have no shipments ready for pickup."),	// NoShipments
		QObject::tr("Your spacecraft does not need any repairs."),
		QObject::tr("Shipments ready for pickup were listed on a private channel."),	// ShipmentsOnPrivateChannel
		QObject::tr("Upgrades must be requested while you are aboard a spacecraft."),
		QObject::tr("Items for sale were listed on a private channel."),
		QObject::tr("We have nothing we can sell to you."),
		QObject::tr("Upgrades were listed on a private channel."),
		QObject::tr("We have no modules for that system."),	// NoModulesForSystem
		QObject::tr("We have no blank disks in stock. We cannot sell published spacecraft blueprints unless we have blank disks."),
		QObject::tr("Published spacecraft blueprints for sale were listed on a private channel."),
		QObject::tr("To publish an old-style spacecraft blueprint, sell us a blueprint disk. A published blueprint becomes available at every design studio and arena in our empire. New style blueprints are published using the blueprint exchange on your spacecraft window."),	// ArchitectCommission
		QObject::tr("Published spacecraft blueprints for sale."),
		QObject::tr("The buyer must be in the same solar system as the seller."),
		QObject::tr("That bid is no longer active."),	// BidNotActive
		QObject::tr("To sell cargo, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),	// MoveTransporterStationToSell
		QObject::tr("To sell cargo here, you must move within transporter range of this city or land on a road slab here."),	// MoveTransporterToSell
		QObject::tr("To sell cargo, you must land on a road slab here or move closer to a friendly station."),	// MoveStationToSell
		QObject::tr("To sell cargo here, you must land on a road slab in this city."),	// MoveToSell
		QObject::tr("We are too far apart to transfer the item."),
		QObject::tr("Our space ships are too far apart to transfer the cargo."),
		QObject::tr("Our locations prevent this transaction from occurring. Both parties must be aboard different ships to trade cargo. Both parties must be aboard the same ship or no ship to trade personal items."),
		QObject::tr("We are too far apart to transfer the item."),
		QObject::tr("The buyer's inventory doesn't have room."),
		QObject::tr("Your ship does not have the fuel to sell."),
		QObject::tr("Your ship does not have the cargo to sell."),	// ShipNoCargoToSell
		QObject::tr("You do not have the item to sell."),	// PersonalNoItemToSell
		QObject::tr("The buyer and seller must be aboard the same ship to transfer blueprints."),
		QObject::tr("We are too far apart to transfer the blueprint."),
		QObject::tr("I have requested a copy of the blueprint from the library. Please wait a moment and try again."),
		QObject::tr("Congratulations! Your spacecraft blueprint has been published to the library. In a few minutes it will be available at every design studio and arena in our empire."),	// PublishCongrats
		QObject::tr("You do not have the blueprint to sell."),	// NoDesignDisk
		QObject::tr("The city must be in the same solar system to repair your spacecraft."),
		QObject::tr("You must be aboard a spacecraft to accept repairs."),
		QObject::tr("That repair offer is no longer active."),
		QObject::tr("To get repairs, you must land on a road slab here or moor at a friendly station."),
		QObject::tr("To get repairs here, you must land on a road slab in this city."),
		QObject::tr("We cannot repair your spacecraft while it is moving."),
		QObject::tr("We cannot repair your spacecraft while its sensors are on."),
		QObject::tr("We cannot repair your spacecraft while its shields are up."),
		QObject::tr("We cannot repair your spacecraft while its FTL drive is on."),
		QObject::tr("We cannot repair your spacecraft while its maneuver drive is on."),
		QObject::tr("We cannot repair your spacecraft while its power plant is on."),
		QObject::tr("Ships must be in the same solar system to transfer passengers."),
		QObject::tr("That passenger boarding announcement is no longer active."),
		QObject::tr("Our ships are too far apart to transfer passengers."),
		QObject::tr("That passengers debarking announcement is no longer active."),
		QObject::tr("Ships must be in the same solar system to transfer troops."),
		QObject::tr("That troop boarding announcement is no longer active."),
		QObject::tr("Our ships are too far apart to transfer troops."),
		QObject::tr("Nobody here has enough money to afford space travel."),	// NobodyCanAffordToLeave
		QObject::tr("We do not have enough money to buy any. Do we have something you will accept in trade?"),	// WeNoAffordThose
		QObject::tr("Congratulations! Your building blueprint has been published to the library. In a few minutes it will be available at every design studio in our empire. It will also be available on the Construction window of all members of your empire."),	// BuildingPublishCongrats
		QObject::tr("We have no blank disks or paper in stock. We cannot sell published building blueprints unless we have blank disks or paper."),	// BuildingNoBlankDisks
		QObject::tr("Published building blueprints for sale were listed on a private channel."),	// BuildingDesignsOnPrivateChannel
		QObject::tr("Published building blueprints for sale."),	// BuildingDesignsForSale
		QObject::tr("To publish an old-style building blueprint, sell us a blueprint disk or drawing. A published blueprint becomes available on the build window and at every design studio in our empire."),	// BuildingArchitectCommission
		QObject::tr("Officers cannot be deployed from this occupied city."),	// OfficerCityOccupied
		QObject::tr("We displatched a courier with your diplomatic message."),	// DiplomaticMessageSent
		QObject::tr("Your diplomatic message was not sent. We can only dispatch one courier every five minutes."),	// DiplomaticMessageWait
		QObject::tr("We have no leadership. There is nobody to receive your message."),	// DiplomaticMessageIgnored
		QObject::tr(""),	// UNUSED
		QObject::tr("This building is under construction. Come back after the grand opening and someone will be glad to help you."),	// BuildingUnderConstruction
		QObject::tr("We have no need for diplomacy. Be off before we destroy you for trespassing."),	// DiplomaticMessageIgnoredByPirate
		QObject::tr("Officers cannot be deployed from a city that is under attack."),	// OfficerCityAttacked
		QObject::tr("You must move to a design studio or arena here to sell spacecraft blueprints. You should consider publishing it to the blueprint exchange."),	// MoveToDesignStudioOrArenaToSell,
		QObject::tr("You must move to a design studio or arena here to buy spacecraft blueprints. They can also be bought from the blueprint exchange."),	// MoveToDesignStudioOrArenaToBuy,
		QObject::tr("That is a unique and interesting item. We don't deal in those."),	// UnwantedStoryItem
		QObject::tr("Repairs cannot be purchased while your ship is getting repaired."),	// NoRepairShipGettingRepairs
		QObject::tr("Only an officer of the spacecraft can load cryo travellers."),			// MustBeOfficerToLoadCryo
		QObject::tr("Cryo travellers cannot be loaded in the Design Studio."),				// NoCryoInStudio
		QObject::tr("To load cryo travellers, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),	// MoveTransporterStationToBoardCryo
		QObject::tr("To load cryo travellers here, you must move within transporter range of this city or land on a road slab here."),	// MoveTransporterToBoardCryo,
		QObject::tr("To load cryo travellers, you must land on a road slab here or move closer to a friendly station."),	// MoveStationToBoardCryo,
		QObject::tr("To load cryo travellers here, you must land on a road slab in this city."),	// MoveToBoardCryo,

		QObject::tr("Only an officer of the spacecraft can debark cryo travellers."),	// MustBeOfficerToUnloadCryo
		QObject::tr("To debark cryo travellers, you must move within transporter range of this city, land on a road slab here or move closer to a friendly station."),	// MoveTransporterStationToDebarkCryo,
		QObject::tr("To debark cryo travellers here, you must move within transporter range of this city or land on a road slab here."),	// MoveTransporterToDebarkCryo,
		QObject::tr("To debark cryo travellers, you must land on a road slab here or move closer to a friendly station."),	// MoveStationToDebarkCryo,
		QObject::tr("To debark cryo travellers here, you must land on a road slab in this city."),	// MoveToDebarkCryo,
		QObject::tr("Ships must be in the same solar system to transfer cryo travellers."),	// CryoSameSystem,
		QObject::tr("That cryo traveller boarding announcement is no longer active."),	// CryoBoardingNotActive,
		QObject::tr("That cryo traveller debarking announcement is no longer active."),	// CryoDebarkingNotActive,
		QObject::tr("Our ships are too far apart to transfer cryo travellers."),	// TooFarToTransactCryo,
		QObject::tr("You must be aboard a spacecraft to request refit."),	// RefitAboardShip,
		QObject::tr("Only an officer of the spacecraft can order a refit."),	// MustBeOfficerToRefit,
		QObject::tr("Your spacecraft cannot be refit."),	// CannotRefit
		QObject::tr("Your spacecraft cannot be refit according to those specifications."),	// InvalidRefit
		QObject::tr("The refit specification does not change anything about your spacecraft."),	// RefitNoChange
		QObject::tr("We do not have an airport terminal or repair shop to provide that service."),	// NoRepairShopOrAirport
		QObject::tr("House keeping must be requested while you are aboard a spacecraft."),	// HouseKeepingAboardShip
		QObject::tr("Only an officer of the spacecraft can call for house keeping service."),	// MustBeOfficerToHouseKeeping
		QObject::tr("The quality of our repair shop is too low to install the modules we have in stock."),	// BuildingQTooLowToInstall
		QObject::tr("Refit cannot be purchased while your ship is getting repaired."),	// NoRefitShipGettingRepairs
		QObject::tr("We have no accommodations for travellers. Our homes are filled to capacity."),	// NoHomesForTravellers,
		QObject::tr("We do not want those. We have no place to store your entire offer."),	// WeNoWantAllOfThose
		QObject::tr("We can't deliver diplomatic messages to our own empire."),	// DiplomaticMessageIgnoredSameEmpire
		QObject::tr("We can't take recruits. Our barracks are filled to capacity."),	// NoHomesForRecruits,
		QObject::tr("To sell cargo here, you must land at a building site in this city."),	// MoveBdToSell
		QObject::tr("To sell cargo, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToSell
		QObject::tr("To sell cargo here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToSell
		QObject::tr("To sell cargo, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToSell
		QObject::tr("To get repairs here, you must land at a building site in this city."),	// MoveBdToRepair
		QObject::tr("To get repairs, you must land at a building site here or moor at a friendly station."),	// MoveBdStationToRepair
		QObject::tr("To buy cargo here, you must land at a building site in this city."),	// MoveBdToBuy,
		QObject::tr("To buy cargo, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToBuy,
		QObject::tr("To buy cargo here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToBuy,
		QObject::tr("To buy cargo, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToBuy,
		QObject::tr("To load crew here, you must land at a building site in this city."),	// MoveBdToBoardCrew,
		QObject::tr("To load crew, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToBoardCrew,
		QObject::tr("To load crew here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToBoardCrew,
		QObject::tr("To load crew, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToBoardCrew,
		QObject::tr("To load cryo travellers here, you must land at a building site in this city."),	// MoveBdToBoardCryo,
		QObject::tr("To load cryo travellers, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToBoardCryo,
		QObject::tr("To load cryo travellers here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToBoardCryo,
		QObject::tr("To load cryo travellers, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToBoardCryo,
		QObject::tr("To debark cryo travellers here, you must land at a building site in this city."),	// MoveBdToBoardCryo,
		QObject::tr("To debark cryo travellers, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToBoardCryo,
		QObject::tr("To debark cryo travellers here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToBoardCryo,
		QObject::tr("To debark cryo travellers, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToBoardCryo,
		QObject::tr("To load the officer here, you must land at a building site in this city."),	// MoveBdToBoardOfficer,
		QObject::tr("To load our officer, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToBoardOfficer,
		QObject::tr("To load the officer here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToBoardOfficer,
		QObject::tr("To load our officer, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToBoardOfficer,
		QObject::tr("To load passengers here, you must land at a building site in this city."),
		QObject::tr("To load passengers, you must land at a building site here or move closer to a friendly station."),
		QObject::tr("To load passengers here, you must move within transporter range of this city or land at a building site here."),
		QObject::tr("To load passengers, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),
		QObject::tr("To debark passengers here, you must land at a building site in this city."),
		QObject::tr("To debark passengers, you must land at a building site here or move closer to a friendly station."),
		QObject::tr("To debark passengers here, you must move within transporter range of this city or land at a building site here."),
		QObject::tr("To debark passengers, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),
		QObject::tr("To load troops here, you must land at a building site in this city."),
		QObject::tr("To load troops, you must land at a building site here or move closer to a friendly station."),
		QObject::tr("To load troops here, you must move within transporter range of this city or land at a building site here."),
		QObject::tr("To load troops, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),
		QObject::tr("No crew are available here."),	// NoCrewToLeave
		QObject::tr("No cryo travellers are available here."),	// NoCryoToLeave
		QObject::tr("No space travellers are available here."),	// NoPassengersToLeave
		QObject::tr("Our personnel are restricted from taking a berth aboard your ship."),	// PersonnelRestricted
		QObject::tr("No personnel are here to take cryo berths."),	// NoBaseCryoToLeave
		QObject::tr("Welcome friend. What merchandise do you have to sell? I am listening."),	// GreetSalesman
		QObject::tr("To buy power here, you must land at a building site in this city."),	// MoveBdToBuyPower,
		QObject::tr("To sell power here, you must land at a building site in this city."),	// MoveBdToSellPower,
		QObject::tr("The container contents are yours. Open the TEU to access the things inside."),	// UninsuredTEU
		QObject::tr("The container shipment is not addressed to us. We cannot accept delivery."),	// UnwantedTEU
		QObject::tr("We cannot pay the delivery fee. Under the rules of acquisition, the container shipment is yours to keep. I will notify the shipper of their loss."),	// DeliveryDefault
		QObject::tr("Only an officer of the spacecraft can deliver TEU shipments."),	// MustBeOfficerToDeliverTEU
		QObject::tr("To deliver TEUs here, you must land at a building site in this city."),	// MoveBdToDeliverTEU
		QObject::tr("To deliver TEUs, you must land at a building site here or move closer to a friendly station."),	// MoveBdStationToDeliverTEU
		QObject::tr("To deliver TEUs here, you must move within transporter range of this city or land at a building site here."),	// MoveBdTransporterToDeliverTEU
		QObject::tr("To deliver TEUs, you must move within transporter range of this city, land at a building site here or move closer to a friendly station."),	// MoveBdTransporterStationToDeliverTEU
		QObject::tr("You are black listed by our insurance company. Try getting that cleared up first."),	// RejectBadShipper
		QObject::tr("We have no airport repair shop workers to do the job."),	// NoRepairShopWorkers
		QObject::tr("We do not have enough modules to upgrade that system on your ship."),	// NotEnoughModulesForSystem
		QObject::tr("We have no metal of any kind for your hull."),	// NoMetalForHull
		QObject::tr("We do not have enough metal of any kind for your hull."),	// NotEnoughMetalForHull
	};
	return messages[m_Message];
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCityMessage::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCrewBoarded::AuCommCrewBoarded(const AuId& loaderId, quint8 count, bool beamedUp,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamedUp = beamedUp;
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommCrewBoarded::AuCommCrewBoarded()
{
	m_BeamedUp = false;
	m_Count = 0;
}

AuCommCrewBoarded::AuCommCrewBoarded(const AuCommCrewBoarded& c) : AuCommPacket(c)
{
	m_BeamedUp = c.m_BeamedUp;
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommCrewBoarded::Clone() const
{
	return new AuCommCrewBoarded(*this);
}

QString AuCommCrewBoarded::Icon() const
{
	return CommPacketIcon();
}

void AuCommCrewBoarded::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedUp = ui8;
	in >> m_Count;
	in >> m_LoaderId;
}

void AuCommCrewBoarded::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_BeamedUp;
	out << m_Count;
	out << m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommCrewBoarded::Text() const
{
	QString text;
	if(m_BeamedUp)
	{
		if(m_Count == 1)
			return QObject::tr("One crew member beamed aboard the ship.");
		else
			return QObject::tr("%1 crew members beamed aboard the ship.")
				.arg(m_Count);
	}
	else
	{
		if(m_Count == 1)
			return QObject::tr("One crew member boarded the ship.");
		else
			return QObject::tr("%1 crew members boarded the ship.")
				.arg(m_Count);
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCrewBoarded::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCrewBoardingComplete::AuCommCrewBoardingComplete(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommCrewBoardingComplete::AuCommCrewBoardingComplete()
{
}

AuCommCrewBoardingComplete::AuCommCrewBoardingComplete(const AuCommCrewBoardingComplete& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommCrewBoardingComplete::Clone() const
{
	return new AuCommCrewBoardingComplete(*this);
}

QString AuCommCrewBoardingComplete::Icon() const
{
	return CommPacketIcon();
}

void AuCommCrewBoardingComplete::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommCrewBoardingComplete::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommCrewBoardingComplete::Text() const
{
	return QObject::tr("Crew boarding complete. Kicking crew from the roster who are not aboard.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCrewBoardingComplete::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommDamageInflicted::AuCommDamageInflicted(const QString& victimName, quint8 criticalHits, bool isDestroyed,
		WEAPONEFFECT effect1, quint32 damage1, quint32 armor1,
		WEAPONEFFECT effect2, quint32 damage2, quint32 armor2,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Armor1 = armor1;
	m_Armor2 = armor2;
	m_CriticalHits = criticalHits;
	m_Damage1 = damage1;
	m_Damage2 = damage2;
	m_Effect1 = effect1;
	m_Effect2 = effect2;
	m_IsDestroyed = isDestroyed;
	m_VictimName = victimName;
}

AuCommDamageInflicted::AuCommDamageInflicted()
{
	m_Armor1 = 0;
	m_Armor2 = 0;
	m_CriticalHits = 0;
	m_Damage1 = 0;
	m_Damage2 = 0;
	m_Effect1 = WE_None;
	m_Effect2 = WE_None;
	m_IsDestroyed = false;
}

AuCommDamageInflicted::AuCommDamageInflicted(const AuCommDamageInflicted& c) : AuCommPacket(c)
{
	m_Armor1 = c.m_Armor1;
	m_Armor2 = c.m_Armor2;
	m_CriticalHits = c.m_CriticalHits;
	m_Damage1 = c.m_Damage1;
	m_Damage2 = c.m_Damage2;
	m_Effect1 = c.m_Effect1;
	m_Effect2 = c.m_Effect2;
	m_IsDestroyed = c.m_IsDestroyed;
	m_VictimName = c.m_VictimName;
}

AuCommPacket* AuCommDamageInflicted::Clone() const
{
	return new AuCommDamageInflicted(*this);
}

QString AuCommDamageInflicted::Icon() const
{
	return CommPacketIcon();
}

void AuCommDamageInflicted::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 hasWhat;
	in >> hasWhat;
	m_CriticalHits = hasWhat&0x03;								// Will be 2 at most.
	m_IsDestroyed = (hasWhat&0x80)?true:false;

	quint8 ui8;
	in >> ui8;
	m_Effect1 = (WEAPONEFFECT)ui8;
	in >> m_VictimName;

	if(hasWhat & 0x40)
		in >> m_Damage1;
	else
		m_Damage1 = 0;
	if(hasWhat & 0x20)
		in >> m_Damage2;
	else
		m_Damage2 = 0;
	if(hasWhat & 0x10)
		in >> m_Armor1;
	else
		m_Armor1 = 0;
	if(hasWhat & 0x08)
		in >> m_Armor2;
	else
		m_Armor2 = 0;
	if(hasWhat & 0x04)
	{
		in >> ui8;
		m_Effect2 = (WEAPONEFFECT)ui8;
	}
	else
		m_Effect2 = WE_None;
}

void AuCommDamageInflicted::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	quint8 hasWhat = m_CriticalHits;
	if(m_IsDestroyed)
		hasWhat |= 0x80;
	if(m_Damage1)
		hasWhat |= 0x40;
	if(m_Damage2)
		hasWhat |= 0x20;
	if(m_Armor1)
		hasWhat |= 0x10;
	if(m_Armor2)
		hasWhat |= 0x08;
	if(m_Effect2 != WE_None)
		hasWhat |= 0x04;
	out << hasWhat;

	out << (quint8)m_Effect1;
	out << m_VictimName;

	if(hasWhat & 0x40)
		out << m_Damage1;
	if(hasWhat & 0x20)
		out << m_Damage2;
	if(hasWhat & 0x10)
		out << m_Armor1;
	if(hasWhat & 0x08)
		out << m_Armor2;
	if(hasWhat & 0x04)
		out << (quint8)m_Effect2;
}

// Returns the text of the comm message.
QString AuCommDamageInflicted::Text() const
{
	// Hit who.
	QString text;
	if(m_CriticalHits == 1)
		text = QObject::tr("Critical Hit %1!").arg(m_VictimName);
	else if(m_CriticalHits == 2)
		text = QObject::tr("Double Critical Hit %1!").arg(m_VictimName);
	else
		text = QObject::tr("Hit %1!").arg(m_VictimName);

	// Damage inflicted.
	if(m_Damage1 && m_Damage2)
		text += QObject::tr(" Inflicted %1 hits of %2 damage and %3 hits of %4 damage.")
			.arg(m_Damage1)
			.arg(EffectName(m_Effect1))
			.arg(m_Damage2)
			.arg(EffectName(m_Effect2));
	else if(m_Damage1)
		text += QObject::tr(" Inflicted %1 hits of %2 damage.")
			.arg(m_Damage1)
			.arg(EffectName(m_Effect1));
	else if(m_Damage2)
		text += QObject::tr(" Inflicted %1 hits of %2 damage.")
			.arg(m_Damage2)
			.arg(EffectName(m_Effect2));

	// Damage absorbed.
	if(m_Armor1 && m_Armor2)
		text += QObject::tr(" %1 hits of %2 damage and %3 hits of %4 damage absorbed.")
			.arg(m_Armor1)
			.arg(EffectName(m_Effect1))
			.arg(m_Armor2)
			.arg(EffectName(m_Effect2));
	else if(m_Armor1)
		text += QObject::tr(" %1 hits of %2 damage absorbed.")
			.arg(m_Armor1)
			.arg(EffectName(m_Effect1));
	else if(m_Armor2)
		text += QObject::tr(" %1 hits of %2 damage absorbed.")
			.arg(m_Armor2)
			.arg(EffectName(m_Effect2));

	// Destroyed
	if(m_IsDestroyed)
		text += QObject::tr(" %1 destroyed!").arg(m_VictimName);

	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommDamageInflicted::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommDebarkCryo::AuCommDebarkCryo(const AuId& unloaderId, quint32 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_UnloaderId = unloaderId;
}

AuCommDebarkCryo::AuCommDebarkCryo()
{
	m_Count = 0;
}

AuCommDebarkCryo::AuCommDebarkCryo(const AuCommDebarkCryo& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_UnloaderId = c.m_UnloaderId;
}

AuCommPacket* AuCommDebarkCryo::Clone() const
{
	return new AuCommDebarkCryo(*this);
}

QString AuCommDebarkCryo::Icon() const
{
	return CommPacketIcon();
}

void AuCommDebarkCryo::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)0;	// Used to be 8 bit count. A 0 was invalid so it now flags the change to a 32 bit value.
	out << m_Count;
	out << m_UnloaderId;
}

void AuCommDebarkCryo::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ocount;
	in >> ocount;
	if(ocount)
		m_Count = ocount;
	else
		in >> m_Count;
	in >> m_UnloaderId;
}

// Returns the text of the comm message.
QString AuCommDebarkCryo::Text() const
{
	if(m_Count == 1)
		return QObject::tr("One cryo traveller is ready to debark.");
	else
		return QObject::tr("%L1 cryo travellers are ready to debark.")
			.arg(m_Count);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommDebarkCryo::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommDebarkPassengers::AuCommDebarkPassengers(const AuId& unloaderId, quint16 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_UnloaderId = unloaderId;
}

AuCommDebarkPassengers::AuCommDebarkPassengers()
{
	m_Count = 0;
}

AuCommDebarkPassengers::AuCommDebarkPassengers(const AuCommDebarkPassengers& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_UnloaderId = c.m_UnloaderId;
}

AuCommPacket* AuCommDebarkPassengers::Clone() const
{
	return new AuCommDebarkPassengers(*this);
}

QString AuCommDebarkPassengers::Icon() const
{
	return CommPacketIcon();
}

void AuCommDebarkPassengers::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)0;	// Used to be 8 bit count. A 0 was invalid so it now flags the change to a 16 bit value.
	out << m_Count;
	out << m_UnloaderId;
}

void AuCommDebarkPassengers::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ocount;
	in >> ocount;
	if(ocount)
		m_Count = ocount;
	else
		in >> m_Count;
	in >> m_UnloaderId;
}

// Returns the text of the comm message.
QString AuCommDebarkPassengers::Text() const
{
	if(m_Count == 1)
		return QObject::tr("One passenger is ready to debark.");
	else
		return QObject::tr("%L1 passengers are ready to debark.")
			.arg(m_Count);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommDebarkPassengers::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommDetonate::AuCommDetonate(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommDetonate::AuCommDetonate()
{
}

AuCommDetonate::AuCommDetonate(const AuCommDetonate& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommDetonate::Clone() const
{
	return new AuCommDetonate(*this);
}

QString AuCommDetonate::Icon() const
{
	return CommPacketIcon();
}

void AuCommDetonate::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommDetonate::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommDetonate::Text() const
{
	return QObject::tr("Detonate!");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommDetonate::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommFiringAtTarget::AuCommFiringAtTarget(quint8 whatFired, const QString& target, const QString& location, const QString& sector,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Location = location;
	m_Target = target;
	m_Sector = sector;
	m_WhatFired = whatFired;
}

AuCommFiringAtTarget::AuCommFiringAtTarget()
{
	m_WhatFired = 0;
}

AuCommFiringAtTarget::AuCommFiringAtTarget(const AuCommFiringAtTarget& c) : AuCommPacket(c)
{
	m_Location = c.m_Location;
	m_Target = c.m_Target;
	m_Sector = c.m_Sector;
	m_WhatFired = c.m_WhatFired;
}

AuCommPacket* AuCommFiringAtTarget::Clone() const
{
	return new AuCommFiringAtTarget(*this);
}

QString AuCommFiringAtTarget::Icon() const
{
	return CommPacketIcon();
}

void AuCommFiringAtTarget::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Location;
	in >> m_Target;
	in >> m_Sector;
	in >> m_WhatFired;
}

void AuCommFiringAtTarget::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Location;
	out << m_Target;
	out << m_Sector;
	out << m_WhatFired;
}

// Returns the text of the comm message.
QString AuCommFiringAtTarget::Text() const
{
	switch(m_WhatFired)
	{
	case 0:
		return QObject::tr("Station firing at %1 at %2, %3.").arg(m_Target).arg(m_Location).arg(m_Sector);
	case 1:
		return QObject::tr("System station firing at %1 in %2, %3.").arg(m_Target).arg(m_Location).arg(m_Sector);
	case 2:
		return QObject::tr("Spacecraft firing at %1 in %2, %3.").arg(m_Target).arg(m_Location).arg(m_Sector);
	}
	return "";
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommFiringAtTarget::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommForSale::AuCommForSale(const AuItem& item, const QString& empireOfOrigin, quint32 price,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
	m_Price = price;
}

AuCommForSale::AuCommForSale()
{
	m_Item = 0;
	m_Price = 0;
}

AuCommForSale::AuCommForSale(const AuCommForSale& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_Price = c.m_Price;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommForSale::~AuCommForSale()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommForSale::Clone() const
{
	return new AuCommForSale(*this);
}

QString AuCommForSale::Icon() const
{
	return CommPacketIcon();
}

void AuCommForSale::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;
	in >> m_Price;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommForSale::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_Price;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommForSale::Text() const
{
	Q_ASSERT(m_Item);
	if(m_Price == 0)
	{
		if(m_Item->Count() == 1)
			return QObject::tr("%1. I will give this away for free.")
				.arg(m_Item->ListDesc(m_EmpireOfOrigin));
		else
			return QObject::tr("%1. I will give these away for free.")
				.arg(m_Item->ListDesc(m_EmpireOfOrigin));
	}
	else
	{
		if(m_Item->Count() == 1)
			return QObject::tr("%1. I will sell this for %L7%8.")
				.arg(m_Item->ListDesc(m_EmpireOfOrigin))
				.arg(m_Price)
				.arg(AuGlobal::CurrencySymbol());
		else
			return QObject::tr("%1. I will sell these for %L7%8 each.")
				.arg(m_Item->ListDesc(m_EmpireOfOrigin))
				.arg(m_Price)
				.arg(AuGlobal::CurrencySymbol());
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommForSale::Type() const
{
	return CommPacketType();
}

const QString& AuCommForSale::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommHailReply::AuCommHailReply(const QString& text, const AuId& senderSystemId,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SystemId = senderSystemId;
	m_Text = text;
}

AuCommHailReply::AuCommHailReply()
{
}

AuCommHailReply::AuCommHailReply(const AuCommHailReply& c) : AuCommPacket(c)
{
	m_SystemId = c.m_SystemId;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommHailReply::Clone() const
{
	return new AuCommHailReply(*this);
}

QString AuCommHailReply::Icon() const
{
	return CommPacketIcon();
}

void AuCommHailReply::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SystemId;
	in >> m_Text;
}

void AuCommHailReply::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SystemId;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommHailReply::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommHailReply::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommHailRequest::AuCommHailRequest(quint32 senderEmpire, const QString& addText,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
	m_Text = addText;
}

AuCommHailRequest::AuCommHailRequest()
{
	m_SenderEmpire = 0;
}

AuCommHailRequest::AuCommHailRequest(const AuCommHailRequest& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommHailRequest::Clone() const
{
	return new AuCommHailRequest(*this);
}

QString AuCommHailRequest::Icon() const
{
	return CommPacketIcon();
}

void AuCommHailRequest::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_SenderEmpire;
	quint8 ui8;
	in >> ui8;

	if(ui8 & 0x80)
		in >> m_Text;
	else
		m_Text = "";
}

void AuCommHailRequest::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;

	quint8 ui8 = 0;
	if(!m_Text.isEmpty())
		ui8 |= 0x80;

	out << ui8;

	if(ui8 & 0x80)
		out << m_Text;
}

// Returns the text of the comm message.
QString AuCommHailRequest::Text() const
{
	QString text;

	// Close range is conversational.
	if(TransmissionRange() == AuCommStdChannel::Close && Scope() == AuCommStdChannel::Audible)
		text = QObject::tr("Hello.");

	else if(Scope() == AuCommStdChannel::Agent || Scope() == AuCommStdChannel::Programmer || Scope() == AuCommStdChannel::Architect)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Calling all agents. Please respond.");
		else
			text = QObject::tr("Calling agents within range. Please respond.");
	}

	else if(Scope() == AuCommStdChannel::CrewMates)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Crew. Ship. Do you read me?");
		else
			text = QObject::tr("Crew. Ship. Are you in range?");
	}

	else if(Scope() == AuCommStdChannel::Empire)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Hailing all people in my empire. Is anybody there?");
		else
			text = QObject::tr("Hailing people in my empire. Is anybody in range?");
	}

	else if(Scope() == AuCommStdChannel::Friends)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Hailing all people in my empire and friendly empires. Is anyone there?");
		else
			text = QObject::tr("Hailing people in my empire and friendly empires. Is anyone in range?");
	}

	else if(Scope() == AuCommStdChannel::Government)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Hailing all empire governors. Is anyone listening?");
		else
			text = QObject::tr("Hailing empire governors. Is anyone in range?");
	}

	else if(Scope() == AuCommStdChannel::Company)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
		{
			if(ChannelName() != AuCommStdChannel::FleetChannelName())
				text = QObject::tr("Hailing %1. Do you read me?").arg(ChannelName());
			else
				text = QObject::tr("Hailing all company spacecraft. Please respond.");
		}
		else
		{
			if(ChannelName() != AuCommStdChannel::FleetChannelName())
				text = QObject::tr("Hailing %1. Are you in range?").arg(ChannelName());
			else
				text = QObject::tr("Hailing all company spacecraft in range.");
		}
	}

	else if(Scope() == AuCommStdChannel::Fleet)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
		{
			if(ChannelName() != AuCommStdChannel::FleetChannelName())
				text = QObject::tr("Hailing %1. Do you read me?").arg(ChannelName());
			else
				text = QObject::tr("Hailing all fleet spacecraft. Please respond.");
		}
		else
		{
			if(ChannelName() != AuCommStdChannel::FleetChannelName())
				text = QObject::tr("Hailing %1. Are you in range?").arg(ChannelName());
			else
				text = QObject::tr("Hailing all fleet spacecraft in range.");
		}
	}

	else if(Scope() == AuCommStdChannel::Diplomacy)
	{
		if(TransmissionRange() == AuCommStdChannel::Universe)
			text = QObject::tr("Hailing governors of all empires. A reply is respectfully requested.");
		else
			text = QObject::tr("Hailing governors of all empires within range. A reply is respectfully requested.");
	}

	else if(AuCommStdChannel::IsRecruitChannel(m_ChannelName))
		text = QObject::tr("Hailing on the recruit channel. Is anyone listening?");

	// Maximum range on Trade and Hail channels is System. We have already handled Close range.
	else if(AuCommStdChannel::IsTradeChannel(m_ChannelName))
	{
		if(TransmissionRange() == AuCommStdChannel::SolarSystem)
			text = QObject::tr("Hailing all cities in this system. Please respond.");

		else if(TransmissionRange() == AuCommStdChannel::Hail)
			text = QObject::tr("Hailing nearby cities. Please respond.");
	}

	else if(AuCommStdChannel::IsHailChannel(m_ChannelName))
	{
		if(TransmissionRange() == AuCommStdChannel::SolarSystem)
			text = QObject::tr("Hailing all spacecraft in this system. Is anybody here?");

		else if(TransmissionRange() == AuCommStdChannel::Hail)
			text = QObject::tr("Hailing nearby spacecraft. Traffic in the area please advise.");
	}

	// Remaining channels are either Galactic or a named channel, which could be one opened by a city. Scope is everyone.
	else if(TransmissionRange() == AuCommStdChannel::Universe)
		text = QObject::tr("Hailing the universe. Is anybody out there?");

	else if(TransmissionRange() == AuCommStdChannel::Galaxy)
		text = QObject::tr("Hailing the galaxy. Is anybody out there?");

	else if(TransmissionRange() == AuCommStdChannel::Sector)
		text = QObject::tr("Hailing at sector range. Is anybody in this region?");

	else if(TransmissionRange() == AuCommStdChannel::SolarSystem)
		text = QObject::tr("Hailing this system. Is anybody here?");

	else if(TransmissionRange() == AuCommStdChannel::Hail)
		text = QObject::tr("Hailing anyone nearby. Is anybody listening?");

	if(text.isEmpty())
		text = QObject::tr("Hailing anyone. Please respond.");

	if(!m_Text.isEmpty())
		text += ' ' + m_Text;

	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommHailRequest::Type() const
{
	return CommPacketType();
}

quint32 AuCommHailRequest::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommHarvest::AuCommHarvest(COMMODITYID cidModule, const AuItem& item,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_CIDModule = cidModule;
	m_Item = item.Clone();
}

AuCommHarvest::AuCommHarvest()
{
	m_CIDModule = CID_Invalid;
	m_Item = 0;
}

AuCommHarvest::AuCommHarvest(const AuCommHarvest& c) : AuCommPacket(c)
{
	m_CIDModule = c.m_CIDModule;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommHarvest::~AuCommHarvest()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommHarvest::Clone() const
{
	return new AuCommHarvest(*this);
}

QString AuCommHarvest::Icon() const
{
	return CommPacketIcon();
}

void AuCommHarvest::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint16 ui16;

	in >> ui16;
	m_CIDModule = (COMMODITYID)ui16;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommHarvest::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint16)m_CIDModule;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommHarvest::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("%1 harvested %2.")
		.arg(g_Au.CommodityName(m_CIDModule))
		.arg(m_Item->ListDesc(""));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommHarvest::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommInventoryItem::AuCommInventoryItem(const AuItem& item, const QString& empireOfOrigin,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
}

AuCommInventoryItem::AuCommInventoryItem()
{
	m_Item = 0;
}

AuCommInventoryItem::AuCommInventoryItem(const AuCommInventoryItem& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommInventoryItem::~AuCommInventoryItem()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommInventoryItem::Clone() const
{
	return new AuCommInventoryItem(*this);
}

QString AuCommInventoryItem::Icon() const
{
	return CommPacketIcon();
}

void AuCommInventoryItem::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommInventoryItem::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommInventoryItem::Text() const
{
	Q_ASSERT(m_Item);
	return m_Item->ListDesc(m_EmpireOfOrigin);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommInventoryItem::Type() const
{
	return CommPacketType();
}

const QString& AuCommInventoryItem::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommInviteToEmpire::AuCommInviteToEmpire(const AuId& toId, const QString& toName, quint32 empireId,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireId = empireId;
	m_ToId = toId;
	m_ToName = toName;
}

AuCommInviteToEmpire::AuCommInviteToEmpire()
{
	m_EmpireId = 0;
}

AuCommInviteToEmpire::AuCommInviteToEmpire(const AuCommInviteToEmpire& c) : AuCommPacket(c)
{
	m_EmpireId = c.m_EmpireId;
	m_ToId = c.m_ToId;
	m_ToName = c.m_ToName;
}

AuCommPacket* AuCommInviteToEmpire::Clone() const
{
	return new AuCommInviteToEmpire(*this);
}

QString AuCommInviteToEmpire::Icon() const
{
	return CommPacketIcon();
}

void AuCommInviteToEmpire::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireId;
	in >> m_ToId;
	in >> m_ToName;
}

void AuCommInviteToEmpire::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireId;
	out << m_ToId;
	out << m_ToName;
}

// Returns the text of the comm message.
QString AuCommInviteToEmpire::Text() const
{
	return QObject::tr("%1, please accept this invitation to join my empire.").arg(m_ToName);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommInviteToEmpire::Type() const
{
	return CommPacketType();
}

quint32 AuCommInviteToEmpire::FromEmpireId() const
{
	return m_EmpireId;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommJettison::AuCommJettison(const AuItem& item, const QString& empireOfOrigin,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_Item = item.Clone();
}

AuCommJettison::AuCommJettison()
{
	m_Item = 0;
}

AuCommJettison::AuCommJettison(const AuCommJettison& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommJettison::~AuCommJettison()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommJettison::Clone() const
{
	return new AuCommJettison(*this);
}

QString AuCommJettison::Icon() const
{
	return CommPacketIcon();
}

void AuCommJettison::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommJettison::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommJettison::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("Jettison %1.")
		.arg(m_Item->ListDesc(m_EmpireOfOrigin));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommJettison::Type() const
{
	return CommPacketType();
}

const QString& AuCommJettison::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Passes AuId(1) for from id so the message will not fail its IsValid test.
// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by announcements.
AuCommAnnouncement::AuCommAnnouncement(const QString& text, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), AuId(1), fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Universe,
			AuCommStdChannel::Thoughts)
{
	m_Text = text;
}

AuCommAnnouncement::AuCommAnnouncement()
{
}

AuCommAnnouncement::AuCommAnnouncement(const AuCommAnnouncement& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommAnnouncement::Clone() const
{
	return new AuCommAnnouncement(*this);
}

QString AuCommAnnouncement::Icon() const
{
	return CommPacketIcon();
}

void AuCommAnnouncement::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	SetAnonymous(false);										// Thought messages are never anonymous but they may come from id 1.
	in >> m_Text;
}

void AuCommAnnouncement::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommAnnouncement::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAnnouncement::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThought::AuCommThought(const QString& text, const AuId& fromAndToId, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close,
			AuCommStdChannel::Thoughts)
{
	m_Text = text;
}

AuCommThought::AuCommThought()
{
}

AuCommThought::AuCommThought(const AuCommThought& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommThought::Clone() const
{
	return new AuCommThought(*this);
}

QString AuCommThought::Icon() const
{
	return CommPacketIcon();
}

void AuCommThought::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	SetAnonymous(false);										// Thought messages are never anonymous but they may come from id 1.
	in >> m_Text;
}

void AuCommThought::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommThought::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommThought::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMailAlert::AuCommThoughtMailAlert(quint8 msgType, const QString& text, const AuId& fromAndToId,
		const QString& fromName) : AuCommThought(text, fromAndToId, fromName)
{
	m_MsgType = msgType;
}

AuCommThoughtMailAlert::AuCommThoughtMailAlert()
{
	m_MsgType = 0;
}

AuCommThoughtMailAlert::AuCommThoughtMailAlert(const AuCommThoughtMailAlert& c) : AuCommThought(c)
{
	m_MsgType = c.m_MsgType;
}

AuCommPacket* AuCommThoughtMailAlert::Clone() const
{
	return new AuCommThoughtMailAlert(*this);
}

QString AuCommThoughtMailAlert::Icon() const
{
	AuMailMsg mailmsg;
	mailmsg.m_msgType = m_MsgType;
	return mailmsg.OriginIcon();
}

void AuCommThoughtMailAlert::operator<<(QDataStream& in)
{
	AuCommThought::operator<<(in);
	in >> m_MsgType;
}

void AuCommThoughtMailAlert::operator>>(QDataStream& out) const
{
	AuCommThought::operator>>(out);
	out << m_MsgType;
}

// Returns the text of the comm message.
QString AuCommThoughtMailAlert::Text() const
{
	return QObject::tr("Mail arrived. %1").arg(AuCommThought::Text());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommThoughtMailAlert::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const AuId& fromAndToId, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close,
			AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field, const AuId& fromAndToId,
		const QString& fromName) : AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1),
			GALAXY_Count, AuCommStdChannel::Close, AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const AuId& fromAndToId,
		const QString& fromName) : AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1),
			GALAXY_Count, AuCommStdChannel::Close, AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field1);
	m_Fields.append(field2);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
		const AuId& fromAndToId, const QString& fromName) : AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId,
			fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close, AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field1);
	m_Fields.append(field2);
	m_Fields.append(field3);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
		const QString& field4, const AuId& fromAndToId, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close,
			AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field1);
	m_Fields.append(field2);
	m_Fields.append(field3);
	m_Fields.append(field4);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
		const QString& field4, const QString& field5, const AuId& fromAndToId, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close,
			AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field1);
	m_Fields.append(field2);
	m_Fields.append(field3);
	m_Fields.append(field4);
	m_Fields.append(field5);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
		const QString& field4, const QString& field5, const QString& field6, const AuId& fromAndToId, const QString& fromName) :
	AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1), GALAXY_Count, AuCommStdChannel::Close,
			AuCommStdChannel::Thoughts)
{
	Q_ASSERT(message < MessageCount);
	m_Fields.append(field1);
	m_Fields.append(field2);
	m_Fields.append(field3);
	m_Fields.append(field4);
	m_Fields.append(field5);
	m_Fields.append(field6);
	m_Message = message;
}

// Passes AuId(1) for sector id so the message will not fail its IsValid test. Origin sector id is not needed by thoughts.
AuCommThoughtMessage::AuCommThoughtMessage(MESSAGE message, const QStringList& fields, const AuId& fromAndToId,
		const QString& fromName) : AuCommPacket(AuCommStdChannel::ThoughtsChannelName(), fromAndToId, fromName, AuId(1),
			GALAXY_Count, AuCommStdChannel::Close, AuCommStdChannel::Thoughts), m_Fields(fields)
{
	Q_ASSERT(message < MessageCount);
	m_Message = message;
}

AuCommThoughtMessage::AuCommThoughtMessage()
{
	m_Message = MessageCount;
}

AuCommThoughtMessage::AuCommThoughtMessage(const AuCommThoughtMessage& c) : AuCommPacket(c), m_Fields(c.m_Fields)
{
	m_Message = c.m_Message;
}

AuCommPacket* AuCommThoughtMessage::Clone() const
{
	return new AuCommThoughtMessage(*this);
}

QString AuCommThoughtMessage::Icon() const
{
	return CommPacketIcon();
}

void AuCommThoughtMessage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 hasWhat = m_Message;
	if(!m_Fields.isEmpty())
		hasWhat |= 0x80;
	out << hasWhat;
	if(hasWhat & 0x80)
		out << m_Fields;
}

void AuCommThoughtMessage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_Message = (MESSAGE)(hasWhat & 0x7f);
	if(hasWhat & 0x80)
		in >> m_Fields;
	else
		m_Fields.clear();
}

// Returns the text of the comm message.
QString AuCommThoughtMessage::Text() const
{
	if(m_Message < 0 || m_Message >= MessageCount)
		return QString("I hear voices in my head...they say my software needs updating.");
	const QString messages[MessageCount] =
	{
		QObject::tr("Turbo lift called to %1."),	//LiftCalled
		QObject::tr("Turbo lift called to %1 but a door is blocked open."),	//LiftCalledDoorBlocked
		QObject::tr("Turbo lift called to %1 but a door is stuck open."),	//LiftCalledDoorStuck
		QObject::tr("Boink! I impregnated %1."),	//IImpregnatedThem
		QObject::tr("Boink! %1 impregnated me."),	//TheyImpregnatedMe
		QObject::tr("Turret %1 charged with energy from capacitor."),	//TurretCharged
		QObject::tr("Turret loaded with %1."),	//TurretLoaded
		QObject::tr("Nothing has grown ripe enough to pick."),	//NothingRipe
		QObject::tr("This plant was prepared for display. The fruit is just for looks."),	//StaticDisplayFruit
		QObject::tr("I did not place any items."),	//NoItemsPlaced
		QObject::tr("There's no place like home."),	//NoPlaceLikeHome
		QObject::tr("Recall home canceled."),	//RecallCanceled
		QObject::tr("This world was claimed by %4 of %1 on %2. The flag is at %3."),	// WorldClaimByEmpire
		QObject::tr("This world was claimed by %4 of %1, friend of my empire, on %2. Their flag is at %3."),	// WorldClaimByFriend
		QObject::tr("This world was claimed by %4 of %1, overlord of my empire, on %2. Their flag is at %3."),	// WorldClaimByVassal
		QObject::tr("This world was claimed by %4 of %1, a neutral empire, on %2. Their flag is at %3. I can override their claim, if I try again within a minute."),	// WorldClaimByNeutral
		QObject::tr("I cannot claim this world. %1 has a military HQ here."),	// WorldClaimAlienHQ
		QObject::tr("I cannot claim this world. %1 has a capitol here."),	// WorldClaimAlienCapitol
		QObject::tr("I cannot claim this world. My current item is not a flag."),	// WorldClaimNoFlag
		QObject::tr("I claim this world for %1. May the inhabitants of this world live peacefully with their new masters."),	// WorldClaimedHabitableDiffDNA
		QObject::tr("I claim this world for %1. My people shall inhabit this land forever."),	// WorldClaimedHabitableSameDNA
		QObject::tr("I claim this inhospitable world for %1."),	// WorldClaimedUninhabitable
		QObject::tr("This world is not claimed by any empire."),	// WorldUnclaimed
		QObject::tr("The tenuous claim of %1 to this world, made by %3 on %2, was annulled."),	// WorldClaimLostNeutral
		QObject::tr("An absurd claim to this world by %1, made by %3 on %2, was ignored."),	// WorldClaimLostEnemy
		QObject::tr("%1 has no claim on this world."),	// QuitClaimNoClaim,
		QObject::tr("I cannot quit the claim. There is a capitol building."),	// QuitClaimCapitol,
		QObject::tr("I cannot quit the claim. There is a military headquarters."),	// QuitClaimHQ,
		QObject::tr("I cannot quit the claim. I did not stake the claim and the world is not in my chain of command."),	// QuitClaimCoC,
		QObject::tr("%1 has quit its claim to this world."),	// QuitClaim
		QObject::tr("I cannot mate until I am fully grown. My body is immature."),	//IImatureToMate
		QObject::tr("%1 cannot mate until she is fully grown. Her body is immature."),	//TheyImatureToMate
		QObject::tr("I bought %2 %1 for %3."),	// ItemPurchased
		QObject::tr("I bought %2 %1 for %3. I could not carry %4 of them so they dropped at my feet."),	// ItemPurchasedDropped
		QObject::tr("I traded %2 %1 for %4 %3."),	// ItemTraded
		QObject::tr("I traded %2 %1 for %4 %3. I could not carry %5 of them so they dropped at my feet."),	// ItemTradedDropped
		QObject::tr("They do not have that many %1."),	// BuyItemGone
		QObject::tr("I do not have %2 to pay for the %1."),	// InsufficientFunds
		QObject::tr("I do not have %2 %1 to complete the trade."),	// OfferItemGone
		QObject::tr("My offer is not valuable enough. They won't accept less than %2 for the %1."),	// ItemNoDeal
		QObject::tr("I took %2 %1."),	// ItemTaken
		QObject::tr("I took %2 %1. I could not carry %3 of them so they dropped at my feet."),	// ItemTakenDropped
		QObject::tr("Boink!"),	// Boink
		QObject::tr("O!"),	// O
		QObject::tr("I trashed %2 %1."),	// ItemDiscarded
	};
	if(m_Fields.isEmpty())
		return messages[m_Message];
	QString text(messages[m_Message]);
	for(QStringList::const_iterator fi = m_Fields.cbegin(); fi != m_Fields.cend(); fi++)
		text = text.arg(*fi);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommThoughtMessage::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMissionNew::AuCommMissionNew(const AuId& toId, const QString& missionName, const AuMission& mission,
		quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_Mission(mission)
{
	m_MissionName = missionName;
	m_SenderEmpire = senderEmpire;
	m_ToId = toId;
}

AuCommMissionNew::AuCommMissionNew()
{
	m_SenderEmpire = 0;
}

AuCommMissionNew::AuCommMissionNew(const AuCommMissionNew& c) : AuCommPacket(c), m_Mission(c.m_Mission)
{
	m_MissionName = c.m_MissionName;
	m_SenderEmpire = c.m_SenderEmpire;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommMissionNew::Clone() const
{
	return new AuCommMissionNew(*this);
}

QString AuCommMissionNew::Icon() const
{
	return CommPacketIcon();
}

void AuCommMissionNew::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_MissionName;
	m_Mission << in;
	in >> m_SenderEmpire;
	in >> m_ToId;
}

void AuCommMissionNew::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_MissionName;
	m_Mission >> out;
	out << m_SenderEmpire;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommMissionNew::Text() const
{
	return QObject::tr("Mission %1 attached, %2 steps. "
			"Accept this message to make it your ship's current mission.")
		.arg(m_MissionName)
		.arg(m_Mission.Count());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMissionNew::Type() const
{
	return CommPacketType();
}

quint32 AuCommMissionNew::FromEmpireId() const
{
	return m_SenderEmpire;
}

bool AuCommMissionNew::IsDataLarge() const
{
	QByteArray data;
	QDataStream outStream(&data, QIODevice::WriteOnly);
	outStream.setVersion(QDataStream::Qt_4_1);
	m_Mission >> outStream;

	return data.count() > 63*1024;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMissionOrderAdd::AuCommMissionOrderAdd(const AuId& toId, const AuMissionOrder& missionOrder,
		quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MissionOrder = missionOrder.Clone();
	m_SenderEmpire = senderEmpire;
	m_ToId = toId;
}

AuCommMissionOrderAdd::AuCommMissionOrderAdd()
{
	m_MissionOrder = 0;
	m_SenderEmpire = 0;
}

AuCommMissionOrderAdd::AuCommMissionOrderAdd(const AuCommMissionOrderAdd& c) : AuCommPacket(c)
{
	if(c.m_MissionOrder)
		m_MissionOrder = c.m_MissionOrder->Clone();
	else
		m_MissionOrder = 0;
	m_SenderEmpire = c.m_SenderEmpire;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommMissionOrderAdd::Clone() const
{
	return new AuCommMissionOrderAdd(*this);
}

QString AuCommMissionOrderAdd::Icon() const
{
	return CommPacketIcon();
}

void AuCommMissionOrderAdd::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	if(m_MissionOrder)
		delete m_MissionOrder;
	in >> ui8;
	m_MissionOrder = AuMissionOrder::Create((AuMissionOrder::TYPE)ui8);
	if(m_MissionOrder)
		in >> *m_MissionOrder;

	in >> m_SenderEmpire;
	in >> m_ToId;
}

void AuCommMissionOrderAdd::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_MissionOrder)
	{
		out << (quint8)m_MissionOrder->Type();
		out << *m_MissionOrder;
	}
	else
		out << (quint8)AuMissionOrder::Invalid;

	out << m_SenderEmpire;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommMissionOrderAdd::Text() const
{
	return m_MissionOrder->ListDescription();
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMissionOrderAdd::Type() const
{
	return CommPacketType();
}

quint32 AuCommMissionOrderAdd::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMissionOrderInsert::AuCommMissionOrderInsert(const AuId& toId, const AuMissionOrder& missionOrder,
		quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MissionOrder = missionOrder.Clone();
	m_SenderEmpire = senderEmpire;
	m_ToId = toId;
}

AuCommMissionOrderInsert::AuCommMissionOrderInsert()
{
	m_MissionOrder = 0;
	m_SenderEmpire = 0;
}

AuCommMissionOrderInsert::AuCommMissionOrderInsert(const AuCommMissionOrderInsert& c) : AuCommPacket(c)
{
	if(c.m_MissionOrder)
		m_MissionOrder = c.m_MissionOrder->Clone();
	else
		m_MissionOrder = 0;
	m_SenderEmpire = c.m_SenderEmpire;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommMissionOrderInsert::Clone() const
{
	return new AuCommMissionOrderInsert(*this);
}

QString AuCommMissionOrderInsert::Icon() const
{
	return CommPacketIcon();
}

void AuCommMissionOrderInsert::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	if(m_MissionOrder)
		delete m_MissionOrder;
	in >> ui8;
	m_MissionOrder = AuMissionOrder::Create((AuMissionOrder::TYPE)ui8);
	if(m_MissionOrder)
		in >> *m_MissionOrder;

	in >> m_SenderEmpire;
	in >> m_ToId;
}

void AuCommMissionOrderInsert::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_MissionOrder)
	{
		out << (quint8)m_MissionOrder->Type();
		out << *m_MissionOrder;
	}
	else
		out << (quint8)AuMissionOrder::Invalid;

	out << m_SenderEmpire;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommMissionOrderInsert::Text() const
{
	return m_MissionOrder->ListDescription();
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMissionOrderInsert::Type() const
{
	return CommPacketType();
}

quint32 AuCommMissionOrderInsert::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMissionOrderNew::AuCommMissionOrderNew(const AuId& toId, const AuMissionOrder& missionOrder,
		quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MissionOrder = missionOrder.Clone();
	m_SenderEmpire = senderEmpire;
	m_ToId = toId;
}

AuCommMissionOrderNew::AuCommMissionOrderNew()
{
	m_MissionOrder = 0;
	m_SenderEmpire = 0;
}

AuCommMissionOrderNew::AuCommMissionOrderNew(const AuCommMissionOrderNew& c) : AuCommPacket(c)
{
	if(c.m_MissionOrder)
		m_MissionOrder = c.m_MissionOrder->Clone();
	else
		m_MissionOrder = 0;
	m_SenderEmpire = c.m_SenderEmpire;
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommMissionOrderNew::Clone() const
{
	return new AuCommMissionOrderNew(*this);
}

QString AuCommMissionOrderNew::Icon() const
{
	return CommPacketIcon();
}

void AuCommMissionOrderNew::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	if(m_MissionOrder)
		delete m_MissionOrder;
	in >> ui8;
	m_MissionOrder = AuMissionOrder::Create((AuMissionOrder::TYPE)ui8);
	if(m_MissionOrder)
		in >> *m_MissionOrder;

	in >> m_SenderEmpire;
	in >> m_ToId;
}

void AuCommMissionOrderNew::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_MissionOrder)
	{
		out << (quint8)m_MissionOrder->Type();
		out << *m_MissionOrder;
	}
	else
		out << (quint8)AuMissionOrder::Invalid;

	out << m_SenderEmpire;
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommMissionOrderNew::Text() const
{
	return m_MissionOrder->ListDescription();
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMissionOrderNew::Type() const
{
	return CommPacketType();
}

quint32 AuCommMissionOrderNew::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommMissionProperty::AuCommMissionProperty(const AuId& toId, quint8 property, quint8 value,
		quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Property = property;
	m_SenderEmpire = senderEmpire;
	m_ToId = toId;
	m_Value = value;
}

AuCommMissionProperty::AuCommMissionProperty()
{
	m_Property = 0;
	m_SenderEmpire = 0;
	m_Value = 0;
}

AuCommMissionProperty::AuCommMissionProperty(const AuCommMissionProperty& c) : AuCommPacket(c)
{
	m_Property = c.m_Property;
	m_SenderEmpire = c.m_SenderEmpire;
	m_ToId = c.m_ToId;
	m_Value = c.m_Value;
}

AuCommPacket* AuCommMissionProperty::Clone() const
{
	return new AuCommMissionProperty(*this);
}

QString AuCommMissionProperty::Icon() const
{
	return CommPacketIcon();
}

void AuCommMissionProperty::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_Property;
	in >> m_SenderEmpire;
	in >> m_ToId;
	in >> m_Value;
}

void AuCommMissionProperty::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Property;
	out << m_SenderEmpire;
	out << m_ToId;
	out << m_Value;
}

// Returns the text of the comm message.
QString AuCommMissionProperty::Text() const
{
	switch(m_Property)
	{
	case 0:	// Request for mission.
		return QObject::tr("Report your mission orders.");
	case 1:	// Active state
		if(m_Value)
			return QObject::tr("Activate your mission.");
		return QObject::tr("Deactivate your mission.");
	case 2:	// Cycle state
		if(m_Value)
			return QObject::tr("Set your mission to cycle.");
		return QObject::tr("Set your mission to execute only once.");
	case 3:	// Request for mission to be sent.
		return QObject::tr("Send your mission to me.");
	}
	return "Mission Setting ?";
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommMissionProperty::Type() const
{
	return CommPacketType();
}

quint32 AuCommMissionProperty::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommNarrate::AuCommNarrate(const QString& text, quint32 storyInstance, const QString& channelName, const AuId& fromSector,
		GALAXY fromGalaxy) :
	AuCommPacket(channelName, AuId(0), QString(""), fromSector, fromGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Narrator)
{
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommNarrate::AuCommNarrate()
{
	m_StoryInstance = 0;
}

AuCommNarrate::AuCommNarrate(const AuCommNarrate& c) : AuCommPacket(c)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommNarrate::Clone() const
{
	return new AuCommNarrate(*this);
}

QString AuCommNarrate::Icon() const
{
	return CommPacketIcon();
}

void AuCommNarrate::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommNarrate::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommNarrate::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommNarrate::Type() const
{
	return CommPacketType();
}

quint32 AuCommNarrate::StoryInstance() const
{
	return m_StoryInstance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommNarrateAudio::AuCommNarrateAudio(const QString& text, quint32 storyInstance, const QByteArray& soundData,
		const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy) :
	AuCommPacket(channelName, AuId(0), QString(""), fromSector, fromGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Narrator)
{
	m_SoundData = soundData;
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommNarrateAudio::AuCommNarrateAudio()
{
	m_StoryInstance = 0;
}

AuCommNarrateAudio::AuCommNarrateAudio(const AuCommNarrateAudio& c) : AuCommPacket(c), m_SoundData(c.m_SoundData)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommNarrateAudio::Clone() const
{
	return new AuCommNarrateAudio(*this);
}

QString AuCommNarrateAudio::Icon() const
{
	return CommPacketIcon();
}

void AuCommNarrateAudio::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SoundData;
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommNarrateAudio::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SoundData;
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommNarrateAudio::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommNarrateAudio::Type() const
{
	return CommPacketType();
}

quint32 AuCommNarrateAudio::StoryInstance() const
{
	return m_StoryInstance;
}

bool AuCommNarrateAudio::IsDataLarge() const
{
	return m_SoundData.count() > 63*1024;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommNarrateChoice::AuCommNarrateChoice(const QString& text, quint32 storyInstance, qint8 choiceId,
		const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy) :
	AuCommPacket(channelName, AuId(0), QString(""), fromSector, fromGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Narrator)
{
	m_ChoiceId = choiceId;
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommNarrateChoice::AuCommNarrateChoice()
{
	m_ChoiceId = 0;
	m_StoryInstance = 0;
}

AuCommNarrateChoice::AuCommNarrateChoice(const AuCommNarrateChoice& c) : AuCommPacket(c)
{
	m_ChoiceId = c.m_ChoiceId;
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommNarrateChoice::Clone() const
{
	return new AuCommNarrateChoice(*this);
}

QString AuCommNarrateChoice::Icon() const
{
	return CommPacketIcon();
}

void AuCommNarrateChoice::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ChoiceId;
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommNarrateChoice::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ChoiceId;
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommNarrateChoice::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommNarrateChoice::Type() const
{
	return CommPacketType();
}

quint32 AuCommNarrateChoice::StoryInstance() const
{
	return m_StoryInstance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommNarrateImage::AuCommNarrateImage(const QString& title, const QString& text, quint32 storyInstance, const QImage& image,
		const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy) :
	AuCommPacket(channelName, AuId(0), QString(""), fromSector, fromGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Narrator)
{
	m_Image = image;
	m_StoryInstance = storyInstance;
	m_Text = text;
	m_Title = title;
}

AuCommNarrateImage::AuCommNarrateImage()
{
	m_StoryInstance = 0;
}

AuCommNarrateImage::AuCommNarrateImage(const AuCommNarrateImage& c) : AuCommPacket(c), m_Image(c.m_Image)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
	m_Title = c.m_Title;
}

AuCommPacket* AuCommNarrateImage::Clone() const
{
	return new AuCommNarrateImage(*this);
}

QString AuCommNarrateImage::Icon() const
{
	return CommPacketIcon();
}

void AuCommNarrateImage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Image;
	in >> m_StoryInstance;
	in >> m_Text;
	in >> m_Title;
}

void AuCommNarrateImage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Image;
	out << m_StoryInstance;
	out << m_Text;
	out << m_Title;
}

// Returns the text of the comm message.
QString AuCommNarrateImage::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommNarrateImage::Type() const
{
	return CommPacketType();
}

quint32 AuCommNarrateImage::StoryInstance() const
{
	return m_StoryInstance;
}

bool AuCommNarrateImage::IsDataLarge() const
{
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	return m_Image.sizeInBytes() > 63*1024;
#else
	return m_Image.byteCount() > 63*1024;
#endif
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommNarrateTitle::AuCommNarrateTitle(const QString& text, quint32 storyInstance, const QString& channelName,
		const AuId& fromSector, GALAXY fromGalaxy) :
	AuCommPacket(channelName, AuId(0), QString(""), fromSector, fromGalaxy, AuCommStdChannel::Close, AuCommStdChannel::Narrator)
{
	m_StoryInstance = storyInstance;
	m_Text = text;
}

AuCommNarrateTitle::AuCommNarrateTitle()
{
	m_StoryInstance = 0;
}

AuCommNarrateTitle::AuCommNarrateTitle(const AuCommNarrateTitle& c) : AuCommPacket(c)
{
	m_StoryInstance = c.m_StoryInstance;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommNarrateTitle::Clone() const
{
	return new AuCommNarrateTitle(*this);
}

QString AuCommNarrateTitle::Icon() const
{
	return CommPacketIcon();
}

void AuCommNarrateTitle::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_StoryInstance;
	in >> m_Text;
}

void AuCommNarrateTitle::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_StoryInstance;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommNarrateTitle::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommNarrateTitle::Type() const
{
	return CommPacketType();
}

quint32 AuCommNarrateTitle::StoryInstance() const
{
	return m_StoryInstance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommOfferRepair derived packets.
// Not a packet type per se.

AuCommOfferRepair::AuCommOfferRepair(const AuId& shipId, const QString& shipName, quint32 price, quint16 repairSecs, quint8 minQ,
		quint16 workers,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MinQ = minQ;
	m_Price = price;
	m_RepairSecs = repairSecs;
	m_ShipId = shipId;
	m_ShipName = shipName;
	m_Workers = workers;
}

AuCommOfferRepair::AuCommOfferRepair()
{
	m_MinQ = 1;
	m_Price = 0;
	m_RepairSecs = 0;
	m_Workers = 0;
}

AuCommOfferRepair::AuCommOfferRepair(const AuCommOfferRepair& c) : AuCommPacket(c)
{
	m_MinQ = c.m_MinQ;
	m_Price = c.m_Price;
	m_RepairSecs = c.m_RepairSecs;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
	m_Workers = c.m_Workers;
}

void AuCommOfferRepair::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_MinQ;
	in >> m_Price;
	in >> m_RepairSecs;
	in >> m_ShipId;
	in >> m_ShipName;
	in >> m_Workers;
}

void AuCommOfferRepair::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_MinQ;
	out << m_Price;
	out << m_RepairSecs;
	out << m_ShipId;
	out << m_ShipName;
	out << m_Workers;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferRepairAll::AuCommOfferRepairAll(const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ,
		const QList<COMMODITYID>& noSupply, const QList<COMMODITYID>& enough, const AuId& shipId, const QString& shipName,
		quint32 price, quint16 repairSecs, quint8 minQ, quint16 workers,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommOfferRepair(shipId, shipName, price, repairSecs, minQ, workers, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope),
	m_NoQ(noQ), m_ShortQ(shortQ), m_NoSupply(noSupply), m_Enough(enough)
{
}

AuCommOfferRepairAll::AuCommOfferRepairAll()
{
}

AuCommOfferRepairAll::AuCommOfferRepairAll(const AuCommOfferRepairAll& c) : AuCommOfferRepair(c), m_NoQ(c.m_NoQ),
	m_ShortQ(c.m_ShortQ), m_NoSupply(c.m_NoSupply), m_Enough(c.m_Enough)
{
}

AuCommPacket* AuCommOfferRepairAll::Clone() const
{
	return new AuCommOfferRepairAll(*this);
}

QString AuCommOfferRepairAll::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferRepairAll::operator<<(QDataStream& in)
{
	AuCommOfferRepair::operator<<(in);

	quint8 ui8;
	quint16 ui16;

	m_NoQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoQ.append((COMMODITYID)ui16);
	}

	m_ShortQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_ShortQ.append((COMMODITYID)ui16);
	}

	m_NoSupply.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoSupply.append((COMMODITYID)ui16);
	}

	m_Enough.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_Enough.append((COMMODITYID)ui16);
	}
}

void AuCommOfferRepairAll::operator>>(QDataStream& out) const
{
	AuCommOfferRepair::operator>>(out);

	out << (quint8)m_NoQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoQ.constBegin(); mi != m_NoQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_ShortQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.constBegin(); mi != m_ShortQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_NoSupply.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.constBegin(); mi != m_NoSupply.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_Enough.count();
	for(QList<COMMODITYID>::const_iterator mi = m_Enough.constBegin(); mi != m_Enough.constEnd(); mi++)
		out << (quint16)(*mi);
}

// Returns the text of the comm message.
QString AuCommOfferRepairAll::Text() const
{
	QString text;
	if(m_Price == 0)
		text = QObject::tr("Our repair shop offers to fix all the systems of %1 for free.")
			.arg(m_ShipName);
	else
		text = QObject::tr("Our repair shop offers to fix all the systems of %3 for %L1%2.")
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol())
			.arg(m_ShipName);

	if(m_MinQ > 1)
		text += QObject::tr(" Replacement parts will be at least Q%1.").arg(m_MinQ);

	// Repair time estimate.
	if(m_RepairSecs < 99)
		text += QObject::tr(" %L2 specialists will complete the repairs in %L1 seconds.").arg(m_RepairSecs).arg(m_Workers);
	else
		text += QObject::tr(" %L2 specialists will complete the repairs in about %L1 minutes.").arg(m_RepairSecs/60).arg(m_Workers);

	// Availability of parts.
	if(!m_NoQ.empty())
	{
		text += QObject::tr(" Parts unavailable at that quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoQ.cbegin(); mi != m_NoQ.cend(); mi++)
		{
			if(mi == m_NoQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_ShortQ.empty())
	{
		if(m_MinQ > 1)
			text += QObject::tr(" Parts in short supply at that quality: ");
		else
			text += QObject::tr(" Parts in short supply: ");
		for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.cbegin(); mi != m_ShortQ.cend(); mi++)
		{
			if(mi == m_ShortQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_NoSupply.empty())
	{
		text += QObject::tr(" Parts not available: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.cbegin(); mi != m_NoSupply.cend(); mi++)
		{
			if(mi == m_NoSupply.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_Enough.empty())
	{
		text += QObject::tr(" We have enough of these parts at lower quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_Enough.cbegin(); mi != m_Enough.cend(); mi++)
		{
			if(mi == m_Enough.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferRepairAll::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferRepairSystem::AuCommOfferRepairSystem(quint8 system, quint16 which, const QString& systemDesc,
		const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply,
		const QList<COMMODITYID>& enough, const AuId& shipId, const QString& shipName, quint32 price, quint16 repairSecs,
		quint8 minQ, quint16 workers,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommOfferRepair(shipId, shipName, price, repairSecs, minQ, workers, channelName, fromId, fromName, fromSector, fromGalaxy,
			range, scope),
	m_NoQ(noQ), m_ShortQ(shortQ), m_NoSupply(noSupply), m_Enough(enough)
{
	m_System = system;
	m_SystemDesc = systemDesc;
	m_Which = which;
}

AuCommOfferRepairSystem::AuCommOfferRepairSystem()
{
	m_System = 0;
	m_Which = 0;
}

AuCommOfferRepairSystem::AuCommOfferRepairSystem(const AuCommOfferRepairSystem& c) : AuCommOfferRepair(c), m_NoQ(c.m_NoQ),
	m_ShortQ(c.m_ShortQ), m_NoSupply(c.m_NoSupply), m_Enough(c.m_Enough)
{
	m_System = c.m_System;
	m_SystemDesc = c.m_SystemDesc;
	m_Which = c.m_Which;
}

AuCommPacket* AuCommOfferRepairSystem::Clone() const
{
	return new AuCommOfferRepairSystem(*this);
}

QString AuCommOfferRepairSystem::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferRepairSystem::operator<<(QDataStream& in)
{
	AuCommOfferRepair::operator<<(in);

	quint8 ui8;
	quint16 ui16;

	m_NoQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoQ.append((COMMODITYID)ui16);
	}

	m_ShortQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_ShortQ.append((COMMODITYID)ui16);
	}

	m_NoSupply.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoSupply.append((COMMODITYID)ui16);
	}

	m_Enough.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_Enough.append((COMMODITYID)ui16);
	}

	in >> m_System;
	in >> m_SystemDesc;
	in >> m_Which;
}

void AuCommOfferRepairSystem::operator>>(QDataStream& out) const
{
	AuCommOfferRepair::operator>>(out);

	out << (quint8)m_NoQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoQ.constBegin(); mi != m_NoQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_ShortQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.constBegin(); mi != m_ShortQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_NoSupply.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.constBegin(); mi != m_NoSupply.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_Enough.count();
	for(QList<COMMODITYID>::const_iterator mi = m_Enough.constBegin(); mi != m_Enough.constEnd(); mi++)
		out << (quint16)(*mi);

	out << m_System;
	out << m_SystemDesc;
	out << m_Which;
}

// Returns the text of the comm message.
QString AuCommOfferRepairSystem::Text() const
{
	QString text;
	if(m_Price == 0)
		text = QObject::tr("We will repair the %1 of %2 for free.")
			.arg(m_SystemDesc)
			.arg(m_ShipName);
	else
		text = QObject::tr("We will repair the %1 of %4 for %L2%3.")
			.arg(m_SystemDesc)
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol())
			.arg(m_ShipName);

	if(m_MinQ > 1)
		text += QObject::tr(" Replacement parts will be at least Q%1.").arg(m_MinQ);

	// Repair time estimate.
	text += QObject::tr(" Repair time estimate %1 seconds.").arg(m_RepairSecs);

	// Availability of parts.
	if(!m_NoQ.empty())
	{
		text += QObject::tr(" Parts unavailable at that quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoQ.cbegin(); mi != m_NoQ.cend(); mi++)
		{
			if(mi == m_NoQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_ShortQ.empty())
	{
		if(m_MinQ > 1)
			text += QObject::tr(" Parts in short supply at that quality: ");
		else
			text += QObject::tr(" Parts in short supply: ");
		for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.cbegin(); mi != m_ShortQ.cend(); mi++)
		{
			if(mi == m_ShortQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_NoSupply.empty())
	{
		text += QObject::tr(" Parts not available: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.cbegin(); mi != m_NoSupply.cend(); mi++)
		{
			if(mi == m_NoSupply.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_Enough.empty())
	{
		text += QObject::tr(" We have enough of these parts at lower quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_Enough.cbegin(); mi != m_Enough.cend(); mi++)
		{
			if(mi == m_Enough.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}

	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferRepairSystem::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferUpgradeInstall::AuCommOfferUpgradeInstall(const AuItem& item, const QString& empireOfOrigin, quint16 systemWhich,
		quint32 price, quint16 installSecs, quint16 workers,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_InstallSecs = installSecs;
	m_Item = item.Clone();
	m_Price = price;
	m_Which = systemWhich;
	m_Workers = workers;
}

AuCommOfferUpgradeInstall::AuCommOfferUpgradeInstall()
{
	m_InstallSecs = 0;
	m_Item = 0;
	m_Price = 0;
	m_Which = 0;
	m_Workers = 0;
}

AuCommOfferUpgradeInstall::AuCommOfferUpgradeInstall(const AuCommOfferUpgradeInstall& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_InstallSecs = c.m_InstallSecs;
	m_Price = c.m_Price;
	m_Which = c.m_Which;
	m_Workers = c.m_Workers;

	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommOfferUpgradeInstall::~AuCommOfferUpgradeInstall()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommOfferUpgradeInstall::Clone() const
{
	return new AuCommOfferUpgradeInstall(*this);
}

QString AuCommOfferUpgradeInstall::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferUpgradeInstall::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_InstallSecs;
	out << m_Price;
	out << m_Which;
	out << m_Workers;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

void AuCommOfferUpgradeInstall::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_EmpireOfOrigin;
	in >> m_InstallSecs;
	in >> m_Price;
	in >> m_Which;
	in >> m_Workers;

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

// Returns the text of the comm message.
QString AuCommOfferUpgradeInstall::Text() const
{
	Q_ASSERT(m_Item);
	QString text;
	if(m_Price == 0)
		text = QObject::tr("Our repair shop offers to upgrade your %1 using %2 for free.")
			.arg(ScSystemName(System()))
			.arg(m_Item->ListDesc(m_EmpireOfOrigin));
	else
		text = QObject::tr("Our repair shop offers to upgrade your %1 using %2 for %L3%4.")
			.arg(ScSystemName(System()))
			.arg(m_Item->ListDesc(m_EmpireOfOrigin))
			.arg(m_Price)
			.arg(AuGlobal::CurrencySymbol());

	// Install time estimate.
	if(m_InstallSecs < 99)
		text += QObject::tr(" %L2 specialists will complete the upgrade in %L1 seconds.")
			.arg(m_InstallSecs).arg(m_Workers);
	else
		text += QObject::tr(" %L2 specialists will complete the upgrade in about %L1 minutes.")
			.arg(m_InstallSecs/60).arg(m_Workers);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferUpgradeInstall::Type() const
{
	return CommPacketType();
}

const QString& AuCommOfferUpgradeInstall::EmpireOfOrigin() const
{
	return m_EmpireOfOrigin;
}

SYSTEM AuCommOfferUpgradeInstall::System() const
{
	Q_ASSERT(m_Item);
	if(HullMaterialModule(m_Item->CommodityId()) != CID_Invalid)
		return SYS_Hull;
	return ScModuleSystem(m_Item->CommodityId());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfficerBoarded::AuCommOfficerBoarded(const AuId& loaderId, const QString& officerName, bool beamedUp,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamedUp = beamedUp;
	m_OfficerName = officerName;
	m_LoaderId = loaderId;
}

AuCommOfficerBoarded::AuCommOfficerBoarded()
{
	m_BeamedUp = false;
}

AuCommOfficerBoarded::AuCommOfficerBoarded(const AuCommOfficerBoarded& c) : AuCommPacket(c)
{
	m_BeamedUp = c.m_BeamedUp;
	m_LoaderId = c.m_LoaderId;
	m_OfficerName = c.m_OfficerName;
}

AuCommPacket* AuCommOfficerBoarded::Clone() const
{
	return new AuCommOfficerBoarded(*this);
}

QString AuCommOfficerBoarded::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfficerBoarded::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedUp = ui8;
	in >> m_LoaderId;
	in >> m_OfficerName;
}

void AuCommOfficerBoarded::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)m_BeamedUp;
	out << m_LoaderId;
	out << m_OfficerName;
}

// Returns the text of the comm message.
QString AuCommOfficerBoarded::Text() const
{
	QString text;
	if(m_BeamedUp)
	{
		return QObject::tr("%1 beamed aboard the ship.")
			.arg(m_OfficerName);
	}
	else
	{
		return QObject::tr("%1 boarded the ship.")
			.arg(m_OfficerName);
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfficerBoarded::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommPassengerBoardingComplete::AuCommPassengerBoardingComplete(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommPassengerBoardingComplete::AuCommPassengerBoardingComplete()
{
}

AuCommPassengerBoardingComplete::AuCommPassengerBoardingComplete(const AuCommPassengerBoardingComplete& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommPassengerBoardingComplete::Clone() const
{
	return new AuCommPassengerBoardingComplete(*this);
}

QString AuCommPassengerBoardingComplete::Icon() const
{
	return CommPacketIcon();
}

void AuCommPassengerBoardingComplete::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommPassengerBoardingComplete::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommPassengerBoardingComplete::Text() const
{
	return QObject::tr("Passenger boarding complete. Kicking passengers from the roster who are not aboard.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommPassengerBoardingComplete::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBoardCryo::AuCommBoardCryo(const AuId& loaderId, quint32 count,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommBoardCryo::AuCommBoardCryo()
{
	m_Count = 0;
}

AuCommBoardCryo::AuCommBoardCryo(const AuCommBoardCryo& c) : AuCommPacket(c)
{
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommBoardCryo::Clone() const
{
	return new AuCommBoardCryo(*this);
}

QString AuCommBoardCryo::Icon() const
{
	return CommPacketIcon();
}

void AuCommBoardCryo::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint8)0;	// Used to be 8 bit count. A 0 was invalid so it now flags the change to a 32 bit value.
	out << m_Count;
	out << m_LoaderId;
}

void AuCommBoardCryo::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	quint8 ocount;
	in >> ocount;
	if(ocount)
		m_Count = ocount;
	else
		in >> m_Count;
	in >> m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommBoardCryo::Text() const
{
	QString text;
	if(m_Count == 1)
		text = QObject::tr("Now boarding. One cryo berth is available for a traveller.");
	else
		text = QObject::tr("Now boarding. %L1 cryo berths are available for travellers.")
			.arg(m_Count);
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBoardCryo::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCryoBoarded::AuCommCryoBoarded(const AuId& loaderId, quint32 count, bool beamedUp, const QStringList& species,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_Species(species)
{
	m_BeamedUp = beamedUp;
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommCryoBoarded::AuCommCryoBoarded()
{
	m_BeamedUp = false;
	m_Count = 0;
}

AuCommCryoBoarded::AuCommCryoBoarded(const AuCommCryoBoarded& c) : AuCommPacket(c), m_Species(c.m_Species)
{
	m_BeamedUp = c.m_BeamedUp;
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommCryoBoarded::Clone() const
{
	return new AuCommCryoBoarded(*this);
}

QString AuCommCryoBoarded::Icon() const
{
	return CommPacketIcon();
}

void AuCommCryoBoarded::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 verbeam = m_BeamedUp;
	verbeam |= 0x80;
	out << verbeam;
	out << m_Count;
	out << m_LoaderId;
	out << m_Species;
}

void AuCommCryoBoarded::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedUp = ui8 & 0x01;
	if(ui8 & 0x80)
		in >> m_Count;
	else
	{
		quint8 ocount;
		in >> ocount;
		m_Count = ocount;
	}
	in >> m_LoaderId;
	in >> m_Species;
}

// Returns the text of the comm message.
QString AuCommCryoBoarded::Text() const
{
	if(m_Species.size() == 1)
	{
		// Only one specie boarded.
		if(m_BeamedUp)
		{
			if(m_Count == 1)
				return QObject::tr("One %1 cryo traveller beamed aboard the ship.")
					.arg(m_Species.first());
			else
				return QObject::tr("%L1 %2 cryo travellers beamed aboard the ship.")
					.arg(m_Count)
					.arg(m_Species.first());
		}
		else
		{
			if(m_Count == 1)
				return QObject::tr("One %1 cryo traveller boarded the ship.")
					.arg(m_Species.first());
			else
				return QObject::tr("%L1 %2 cryo travellers boarded the ship.")
					.arg(m_Count)
					.arg(m_Species.first());
		}
	}
	else
	{
		// More than one specie boarded the ship.
		QString specieNames;
		for(QStringList::const_iterator di = m_Species.cbegin(); di != m_Species.cend(); di++)
		{
			if(di == m_Species.cbegin())
				specieNames = *di;
			else if(di == m_Species.cend() - 1)
				specieNames += " and " + *di;
			else
				specieNames += ", " + *di;
		}
		if(m_BeamedUp)
		{
			if(m_Count == 2)
				return QObject::tr("%L1 cryo travellers beamed aboard the ship, a %2.")
					.arg(m_Count)
					.arg(specieNames);
			else
				return QObject::tr("%L1 cryo travellers beamed aboard the ship. They were a mix of %2.")
					.arg(m_Count)
					.arg(specieNames);
		}
		else
		{
			if(m_Count == 2)
				return QObject::tr("%L1 cryo travellers boarded the ship, a %2.")
					.arg(m_Count)
					.arg(specieNames);
			else
				return QObject::tr("%L1 cryo travellers boarded the ship. They were a mix of %2.")
					.arg(m_Count)
					.arg(specieNames);
		}
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCryoBoarded::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCryoBoardingComplete::AuCommCryoBoardingComplete(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommCryoBoardingComplete::AuCommCryoBoardingComplete()
{
}

AuCommCryoBoardingComplete::AuCommCryoBoardingComplete(const AuCommCryoBoardingComplete& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommCryoBoardingComplete::Clone() const
{
	return new AuCommCryoBoardingComplete(*this);
}

QString AuCommCryoBoardingComplete::Icon() const
{
	return CommPacketIcon();
}

void AuCommCryoBoardingComplete::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommCryoBoardingComplete::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommCryoBoardingComplete::Text() const
{
	return QObject::tr("Cryo traveller boarding complete.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCryoBoardingComplete::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommPassengersBoarded::AuCommPassengersBoarded(const AuId& loaderId, quint16 count, bool beamedUp,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamedUp = beamedUp;
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommPassengersBoarded::AuCommPassengersBoarded()
{
	m_BeamedUp = false;
	m_Count = 0;
}

AuCommPassengersBoarded::AuCommPassengersBoarded(const AuCommPassengersBoarded& c) : AuCommPacket(c)
{
	m_BeamedUp = c.m_BeamedUp;
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommPassengersBoarded::Clone() const
{
	return new AuCommPassengersBoarded(*this);
}

QString AuCommPassengersBoarded::Icon() const
{
	return CommPacketIcon();
}

void AuCommPassengersBoarded::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 verbeam = m_BeamedUp;
	verbeam |= 0x80;
	out << verbeam;
	out << m_Count;
	out << m_LoaderId;
}

void AuCommPassengersBoarded::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedUp = ui8 & 0x01;
	if(ui8 & 0x80)
		in >> m_Count;
	else
	{
		quint8 ocount;
		in >> ocount;
		m_Count = ocount;
	}
	in >> m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommPassengersBoarded::Text() const
{
	QString text;
	if(m_BeamedUp)
	{
		if(m_Count == 1)
			return QObject::tr("One passenger beamed aboard the ship.");
		else
			return QObject::tr("%L1 passengers beamed aboard the ship.")
				.arg(m_Count);
	}
	else
	{
		if(m_Count == 1)
			return QObject::tr("One passenger boarded.");
		else
			return QObject::tr("%L1 passengers boarded.")
				.arg(m_Count);
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommPassengersBoarded::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCryoDebarked::AuCommCryoDebarked(const AuId& unloaderId, quint32 count, bool beamedDown, const QStringList& species,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_Species(species)
{
	// dna list may be empty.
	m_BeamedDown = beamedDown;
	m_Count = count;
	m_UnloaderId = unloaderId;
}

AuCommCryoDebarked::AuCommCryoDebarked()
{
	m_BeamedDown = false;
	m_Count = 0;
}

AuCommCryoDebarked::AuCommCryoDebarked(const AuCommCryoDebarked& c) : AuCommPacket(c), m_Species(c.m_Species)
{
	m_BeamedDown = c.m_BeamedDown;
	m_Count = c.m_Count;
	m_UnloaderId = c.m_UnloaderId;
}

AuCommPacket* AuCommCryoDebarked::Clone() const
{
	return new AuCommCryoDebarked(*this);
}

QString AuCommCryoDebarked::Icon() const
{
	return CommPacketIcon();
}

void AuCommCryoDebarked::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 verbeam = m_BeamedDown;
	verbeam |= 0x80;
	out << verbeam;
	out << m_Count;
	out << m_UnloaderId;
	out << m_Species;
}

void AuCommCryoDebarked::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedDown = ui8 & 0x01;
	if(ui8 & 0x80)
		in >> m_Count;
	else
	{
		quint8 ocount;
		in >> ocount;
		m_Count = ocount;
	}
	in >> m_UnloaderId;
	in >> m_Species;
}

// Returns the text of the comm message.
QString AuCommCryoDebarked::Text() const
{
	if(m_Species.empty())
	{
		// This case occurs when a player offers to debark cryo passengers and the offer is accepted by a human player.
		// The DNA of the cryo travellers is not know when this message is created; they haven't been transferred yet.
		if(m_BeamedDown)
		{
			if(m_Count == 1)
				return QObject::tr("One cryo traveller beamed off the ship.");
			else
				return QObject::tr("%L1 cryo travellers beamed off the ship.")
					.arg(m_Count);
		}
		else
		{
			if(m_Count == 1)
				return QObject::tr("One cryo traveller debarked.");
			else
				return QObject::tr("%L1 cryo travellers debarked.")
					.arg(m_Count);
		}
	}
	else if(m_Species.size() == 1)
	{
		// Only one specie debarked.
		if(m_BeamedDown)
		{
			if(m_Count == 1)
				return QObject::tr("One %1 cryo traveller beamed off the ship.")
					.arg(m_Species.first());
			else
				return QObject::tr("%L1 %2 cryo travellers beamed off the ship.")
					.arg(m_Count)
					.arg(m_Species.first());
		}
		else
		{
			if(m_Count == 1)
				return QObject::tr("One %1 cryo traveller debarked.")
					.arg(m_Species.first());
			else
				return QObject::tr("%L1 %2 cryo travellers debarked.")
					.arg(m_Count)
					.arg(m_Species.first());
		}
	}
	else
	{
		// More than one specie debarked.
		QString specieNames;
		for(QStringList::const_iterator di = m_Species.cbegin(); di != m_Species.cend(); di++)
		{
			if(di == m_Species.cbegin())
				specieNames = *di;
			else if(di == m_Species.cend() - 1)
				specieNames += " and " + *di;
			else
				specieNames += ", " + *di;
		}
		if(m_BeamedDown)
		{
			if(m_Count == 2)
				return QObject::tr("%L1 cryo travellers beamed off the ship, a %2.")
					.arg(m_Count)
					.arg(specieNames);
			else
				return QObject::tr("%L1 cryo travellers beamed off the ship. They were a mix of %2.")
					.arg(m_Count)
					.arg(specieNames);
		}
		else
		{
			if(m_Count == 2)
				return QObject::tr("%L1 cryo travellers debarked, a %2.")
					.arg(m_Count)
					.arg(specieNames);
			else
				return QObject::tr("%L1 cryo travellers debarked. They were a mix of %2.")
					.arg(m_Count)
					.arg(specieNames);
		}
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCryoDebarked::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommPassengersDebarked::AuCommPassengersDebarked(const AuId& unloaderId, quint16 count, quint32 transportFee, bool beamedDown,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamedDown = beamedDown;
	m_Count = count;
	m_Fee = transportFee;
	m_UnloaderId = unloaderId;
}

AuCommPassengersDebarked::AuCommPassengersDebarked()
{
	m_BeamedDown = false;
	m_Count = 0;
	m_Fee = 0;
}

AuCommPassengersDebarked::AuCommPassengersDebarked(const AuCommPassengersDebarked& c) : AuCommPacket(c)
{
	m_BeamedDown = c.m_BeamedDown;
	m_Count = c.m_Count;
	m_Fee = c.m_Fee;
	m_UnloaderId = c.m_UnloaderId;
}

AuCommPacket* AuCommPassengersDebarked::Clone() const
{
	return new AuCommPassengersDebarked(*this);
}

QString AuCommPassengersDebarked::Icon() const
{
	return CommPacketIcon();
}

void AuCommPassengersDebarked::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 verbeam = m_BeamedDown;
	verbeam |= 0x80;
	out << verbeam;
	out << m_Count;
	out << m_Fee;
	out << m_UnloaderId;
}

void AuCommPassengersDebarked::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedDown = ui8 & 0x01;
	if(ui8 & 0x80)
		in >> m_Count;
	else
	{
		quint8 ocount;
		in >> ocount;
		m_Count = ocount;
	}
	in >> m_Fee;
	in >> m_UnloaderId;
}

// Returns the text of the comm message.
QString AuCommPassengersDebarked::Text() const
{
	if(m_BeamedDown)
	{
		if(m_Fee == 0)
		{
			if(m_Count == 1)
				return QObject::tr("One passenger beamed off the ship. No transport fee collected.");
			else
				return QObject::tr("%L1 passengers beamed off the ship. No transport fees collected.")
					.arg(m_Count);
		}
		else
		{
			if(m_Count == 1)
				return QObject::tr("One passenger beamed off the ship. %L1%2 transport fee collected.")
					.arg(m_Fee)
					.arg(AuGlobal::CurrencySymbol());
			else
				return QObject::tr("%L1 passengers beamed off the ship. %L2%3 transport fees collected.")
					.arg(m_Count)
					.arg(m_Fee)
					.arg(AuGlobal::CurrencySymbol());
		}
	}
	else
	{
		if(m_Fee == 0)
		{
			if(m_Count == 1)
				return QObject::tr("One passenger debarked. No transport fee collected.");
			else
				return QObject::tr("%L1 passengers debarked. No transport fees collected.")
					.arg(m_Count);
		}
		else
		{
			if(m_Count == 1)
				return QObject::tr("One passenger debarked. %L1%2 transport fee collected.")
					.arg(m_Fee)
					.arg(AuGlobal::CurrencySymbol());
			else
				return QObject::tr("%L1 passengers debarked. %L2%3 transport fees collected.")
					.arg(m_Count)
					.arg(m_Fee)
					.arg(AuGlobal::CurrencySymbol());
		}
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommPassengersDebarked::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommPreBoard::AuCommPreBoard(const AuId& shipId, const QString& shipName, quint8 minutes, quint8 berths, quint8 berthType,
		quint8 locType, const QString& locName, const QString& locLongLat, const QString& nextStop,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Berths = berths;
	m_BerthType = berthType;
	m_LocLongLat = locLongLat;
	m_LocName = locName;
	m_LocType = locType;
	m_Minutes = minutes;
	m_NextStop = nextStop;
	m_ShipId = shipId;
	m_ShipName = shipName;
}

AuCommPreBoard::AuCommPreBoard()
{
	m_Berths = 0;
	m_BerthType = 0;
	m_LocType = 0;
	m_Minutes = 0;
}

AuCommPreBoard::AuCommPreBoard(const AuCommPreBoard& c) : AuCommPacket(c)
{
	m_Berths = c.m_Berths;
	m_BerthType = c.m_BerthType;
	m_LocLongLat = c.m_LocLongLat;
	m_LocName = c.m_LocName;
	m_LocType = c.m_LocType;
	m_Minutes = c.m_Minutes;
	m_NextStop = c.m_NextStop;
	m_ShipId = c.m_ShipId;
	m_ShipName = c.m_ShipName;
}

AuCommPacket* AuCommPreBoard::Clone() const
{
	return new AuCommPreBoard(*this);
}

QString AuCommPreBoard::Icon() const
{
	return CommPacketIcon();
}

void AuCommPreBoard::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Berths;
	in >> m_BerthType;
	in >> m_LocLongLat;
	in >> m_LocName;
	in >> m_LocType;
	in >> m_Minutes;
	in >> m_NextStop;
	in >> m_ShipId;
	in >> m_ShipName;
}

void AuCommPreBoard::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Berths;
	out << m_BerthType;
	out << m_LocLongLat;
	out << m_LocName;
	out << m_LocType;
	out << m_Minutes;
	out << m_NextStop;
	out << m_ShipId;
	out << m_ShipName;
}

// Returns the text of the comm message.
QString AuCommPreBoard::Text() const
{
	// Develop the location description.
	QString locDesc;
	if(m_LocType == 0)
		locDesc = QObject::tr("We are at the %1 station at %2.").arg(m_LocName).arg(m_LocLongLat);
	else if(m_LocType == 1)
		locDesc = QObject::tr("We are near the %1 station orbiting at %2.").arg(m_LocName).arg(m_LocLongLat);
	else
		locDesc = QObject::tr("We are near the %1 system station.").arg(m_LocName);

			// PreBoard fields: minutes|berths|class|shipId|shipName|locType|locName|locLongLat|nextStop|berthType
	if(m_NextStop.isEmpty())
	{
		if(m_BerthType <= 3)
			return QObject::tr("All aboard! %1 %2 passenger berths are available. "
					"Final boarding in %3 minutes. %4 Accept this message to take a berth.")
				.arg(m_Berths)
				.arg(m_BerthType == 1 ? QObject::tr("first class") :
						m_BerthType == 2 ? QObject::tr("second class") : QObject::tr("third class"))
				.arg(m_Minutes)
				.arg(locDesc);
		else
			return QObject::tr("All aboard! %1 %2 berths are available. "
					"Final boarding in %3 minutes. %4 Accept this message to take a berth.")
				.arg(m_Berths)
				.arg(m_BerthType == 4 ? QObject::tr("troop") : QObject::tr("crew"))
				.arg(m_Minutes)
				.arg(locDesc);
	}
	else
	{
		if(m_BerthType <= 3)
			return QObject::tr("All aboard! %1 %2 passenger berths are available to %3. "
					"Final boarding in %4 minutes. %5 Accept this message to take a berth.")
				.arg(m_Berths)
				.arg(m_BerthType == 1 ? QObject::tr("first class") :
						m_BerthType == 2 ? QObject::tr("second class") : QObject::tr("third class"))
				.arg(m_NextStop)
				.arg(m_Minutes)
				.arg(locDesc);
		else
			return QObject::tr("All aboard! %1 %2 berths are available to %3. "
					"Final boarding in %4 minutes. %5 Accept this message to take a berth.")
				.arg(m_Berths)
				.arg(m_BerthType == 4 ? QObject::tr("troop") : QObject::tr("crew"))
				.arg(m_NextStop)
				.arg(m_Minutes)
				.arg(locDesc);
	}
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommPreBoard::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthAny::AuCommRequestBerthAny(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthAny::AuCommRequestBerthAny()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthAny::AuCommRequestBerthAny(const AuCommRequestBerthAny& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthAny::Clone() const
{
	return new AuCommRequestBerthAny(*this);
}

QString AuCommRequestBerthAny::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthAny::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthAny::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthAny::Text() const
{
	return QObject::tr("I request a berth aboard your vessel.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthAny::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthAny::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthCaptain::AuCommRequestBerthCaptain(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthCaptain::AuCommRequestBerthCaptain()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthCaptain::AuCommRequestBerthCaptain(const AuCommRequestBerthCaptain& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthCaptain::Clone() const
{
	return new AuCommRequestBerthCaptain(*this);
}

QString AuCommRequestBerthCaptain::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthCaptain::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthCaptain::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthCaptain::Text() const
{
	return QObject::tr("I request the captain's berth.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthCaptain::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthCaptain::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthCaptainSlot::AuCommRequestBerthCaptainSlot(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommRequestBerthCaptainSlot::AuCommRequestBerthCaptainSlot()
{
}

AuCommRequestBerthCaptainSlot::AuCommRequestBerthCaptainSlot(const AuCommRequestBerthCaptainSlot& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRequestBerthCaptainSlot::Clone() const
{
	return new AuCommRequestBerthCaptainSlot(*this);
}

QString AuCommRequestBerthCaptainSlot::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthCaptainSlot::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRequestBerthCaptainSlot::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRequestBerthCaptainSlot::Text() const
{
	return QObject::tr("I request the captain's position.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthCaptainSlot::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthCrew::AuCommRequestBerthCrew(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthCrew::AuCommRequestBerthCrew()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthCrew::AuCommRequestBerthCrew(const AuCommRequestBerthCrew& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthCrew::Clone() const
{
	return new AuCommRequestBerthCrew(*this);
}

QString AuCommRequestBerthCrew::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthCrew::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthCrew::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthCrew::Text() const
{
	return QObject::tr("I request a crew berth.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthCrew::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthCrew::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthOfficer::AuCommRequestBerthOfficer(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthOfficer::AuCommRequestBerthOfficer()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthOfficer::AuCommRequestBerthOfficer(const AuCommRequestBerthOfficer& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthOfficer::Clone() const
{
	return new AuCommRequestBerthOfficer(*this);
}

QString AuCommRequestBerthOfficer::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthOfficer::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthOfficer::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthOfficer::Text() const
{
	return QObject::tr("I request an officer berth.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthOfficer::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthOfficer::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthPassenger::AuCommRequestBerthPassenger(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthPassenger::AuCommRequestBerthPassenger()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthPassenger::AuCommRequestBerthPassenger(const AuCommRequestBerthPassenger& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthPassenger::Clone() const
{
	return new AuCommRequestBerthPassenger(*this);
}

QString AuCommRequestBerthPassenger::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthPassenger::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthPassenger::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthPassenger::Text() const
{
	return QObject::tr("I request a passenger berth.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthPassenger::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthPassenger::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestBerthTroop::AuCommRequestBerthTroop(quint32 senderEmpire, 
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_SenderEmpire = senderEmpire;
}

AuCommRequestBerthTroop::AuCommRequestBerthTroop()
{
	m_SenderEmpire = 0;
}

AuCommRequestBerthTroop::AuCommRequestBerthTroop(const AuCommRequestBerthTroop& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommRequestBerthTroop::Clone() const
{
	return new AuCommRequestBerthTroop(*this);
}

QString AuCommRequestBerthTroop::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestBerthTroop::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommRequestBerthTroop::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

// Returns the text of the comm message.
QString AuCommRequestBerthTroop::Text() const
{
	return QObject::tr("I request a troop berth.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestBerthTroop::Type() const
{
	return CommPacketType();
}

quint32 AuCommRequestBerthTroop::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestExterminator::AuCommRequestExterminator(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommRequestExterminator::AuCommRequestExterminator()
{
}

AuCommRequestExterminator::AuCommRequestExterminator(const AuCommRequestExterminator& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRequestExterminator::Clone() const
{
	return new AuCommRequestExterminator(*this);
}

QString AuCommRequestExterminator::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestExterminator::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRequestExterminator::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRequestExterminator::Text() const
{
	return QObject::tr("Send an exterminator to get the rats out of this spacecraft.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestExterminator::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestHouseKeeping::AuCommRequestHouseKeeping(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommRequestHouseKeeping::AuCommRequestHouseKeeping()
{
}

AuCommRequestHouseKeeping::AuCommRequestHouseKeeping(const AuCommRequestHouseKeeping& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRequestHouseKeeping::Clone() const
{
	return new AuCommRequestHouseKeeping(*this);
}

QString AuCommRequestHouseKeeping::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestHouseKeeping::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRequestHouseKeeping::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRequestHouseKeeping::Text() const
{
	return QObject::tr("House keeping service is requested, to clean up this spacecraft.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestHouseKeeping::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestInventory::AuCommRequestInventory(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommRequestInventory::AuCommRequestInventory()
{
}

AuCommRequestInventory::AuCommRequestInventory(const AuCommRequestInventory& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRequestInventory::Clone() const
{
	return new AuCommRequestInventory(*this);
}

QString AuCommRequestInventory::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestInventory::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRequestInventory::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRequestInventory::Text() const
{
	return QObject::tr("Report your status.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestInventory::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestHelp::AuCommRequestHelp(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, AuCommStdChannel::Help)
{
}

AuCommRequestHelp::AuCommRequestHelp()
{
}

AuCommRequestHelp::AuCommRequestHelp(const AuCommRequestHelp& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRequestHelp::Clone() const
{
	return new AuCommRequestHelp(*this);
}

QString AuCommRequestHelp::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestHelp::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRequestHelp::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRequestHelp::Text() const
{
	return QObject::tr("I request assistance.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestHelp::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestRepairAll::AuCommRequestRepairAll(quint8 minQ, const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MinQ = minQ;
}

AuCommRequestRepairAll::AuCommRequestRepairAll()
{
	m_MinQ = 1;
}

AuCommRequestRepairAll::AuCommRequestRepairAll(const AuCommRequestRepairAll& c) : AuCommPacket(c)
{
	m_MinQ = c.m_MinQ;
}

AuCommPacket* AuCommRequestRepairAll::Clone() const
{
	return new AuCommRequestRepairAll(*this);
}

QString AuCommRequestRepairAll::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestRepairAll::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_MinQ;
}

void AuCommRequestRepairAll::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_MinQ;
}

// Returns the text of the comm message.
QString AuCommRequestRepairAll::Text() const
{
	if(m_MinQ > 1)
		return QObject::tr("Can anyone repair all the systems of my spacecraft? "
				"Replacement parts must be at least Q%1.")
			.arg(m_MinQ);
	else
		return QObject::tr("Can anyone repair all the systems of my spacecraft?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestRepairAll::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestRepairSystem::AuCommRequestRepairSystem(quint8 system, quint16 which, const QString& systemDesc, quint8 minQ,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_MinQ = minQ;
	m_System = system;
	m_SystemDesc = systemDesc;
	m_Which = which;
}

AuCommRequestRepairSystem::AuCommRequestRepairSystem()
{
	m_MinQ = 1;
	m_System = 0;
	m_Which = 0;
}

AuCommRequestRepairSystem::AuCommRequestRepairSystem(const AuCommRequestRepairSystem& c) : AuCommPacket(c)
{
	m_MinQ = c.m_MinQ;
	m_System = c.m_System;
	m_SystemDesc = c.m_SystemDesc;
	m_Which = c.m_Which;
}

AuCommPacket* AuCommRequestRepairSystem::Clone() const
{
	return new AuCommRequestRepairSystem(*this);
}

QString AuCommRequestRepairSystem::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestRepairSystem::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_MinQ;
	in >> m_System;
	in >> m_SystemDesc;
	in >> m_Which;
}

void AuCommRequestRepairSystem::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_MinQ;
	out << m_System;
	out << m_SystemDesc;
	out << m_Which;
}

// Returns the text of the comm message.
QString AuCommRequestRepairSystem::Text() const
{
	if(m_MinQ > 1)
		return QObject::tr("Can anyone repair the %1 of my spacecraft? "
				"Replacement parts must be at least Q%2.")
			.arg(m_SystemDesc)
			.arg(m_MinQ);
	else
		return QObject::tr("Can anyone repair the %1 of my spacecraft?").arg(m_SystemDesc);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestRepairSystem::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestUpgradeSystem::AuCommRequestUpgradeSystem(quint8 system, quint16 which, const QString& systemDesc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_System = system;
	m_SystemDesc = systemDesc;
	m_Which = which;
}

AuCommRequestUpgradeSystem::AuCommRequestUpgradeSystem()
{
	m_System = 0;
	m_Which = 0;
}

AuCommRequestUpgradeSystem::AuCommRequestUpgradeSystem(const AuCommRequestUpgradeSystem& c) : AuCommPacket(c)
{
	m_System = c.m_System;
	m_SystemDesc = c.m_SystemDesc;
	m_Which = c.m_Which;
}

AuCommPacket* AuCommRequestUpgradeSystem::Clone() const
{
	return new AuCommRequestUpgradeSystem(*this);
}

QString AuCommRequestUpgradeSystem::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestUpgradeSystem::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_System;
	in >> m_SystemDesc;
	in >> m_Which;
}

void AuCommRequestUpgradeSystem::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_System;
	out << m_SystemDesc;
	out << m_Which;
}

// Returns the text of the comm message.
QString AuCommRequestUpgradeSystem::Text() const
{
	return QObject::tr("Can anyone upgrade the %1 of my spacecraft?").arg(m_SystemDesc);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestUpgradeSystem::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRevokeBid::AuCommRevokeBid(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommRevokeBid::AuCommRevokeBid()
{
}

AuCommRevokeBid::AuCommRevokeBid(const AuCommRevokeBid& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommRevokeBid::Clone() const
{
	return new AuCommRevokeBid(*this);
}

QString AuCommRevokeBid::Icon() const
{
	return CommPacketIcon();
}

void AuCommRevokeBid::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommRevokeBid::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommRevokeBid::Text() const
{
	return QObject::tr("My bid is canceled.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRevokeBid::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitAllSales::AuCommSolicitAllSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitAllSales::AuCommSolicitAllSales()
{
}

AuCommSolicitAllSales::AuCommSolicitAllSales(const AuCommSolicitAllSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitAllSales::Clone() const
{
	return new AuCommSolicitAllSales(*this);
}

QString AuCommSolicitAllSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitAllSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitAllSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitAllSales::Text() const
{
	return QObject::tr("Does anyone have anything for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitAllSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitAmmoSales::AuCommSolicitAmmoSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitAmmoSales::AuCommSolicitAmmoSales()
{
}

AuCommSolicitAmmoSales::AuCommSolicitAmmoSales(const AuCommSolicitAmmoSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitAmmoSales::Clone() const
{
	return new AuCommSolicitAmmoSales(*this);
}

QString AuCommSolicitAmmoSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitAmmoSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitAmmoSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitAmmoSales::Text() const
{
	return QObject::tr("Does anyone have any ammunition for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitAmmoSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitArmorSales::AuCommSolicitArmorSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitArmorSales::AuCommSolicitArmorSales()
{
}

AuCommSolicitArmorSales::AuCommSolicitArmorSales(const AuCommSolicitArmorSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitArmorSales::Clone() const
{
	return new AuCommSolicitArmorSales(*this);
}

QString AuCommSolicitArmorSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitArmorSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitArmorSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitArmorSales::Text() const
{
	return QObject::tr("Does anyone have any armor for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitArmorSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitConstructionMaterialSales::AuCommSolicitConstructionMaterialSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitConstructionMaterialSales::AuCommSolicitConstructionMaterialSales()
{
}

AuCommSolicitConstructionMaterialSales::AuCommSolicitConstructionMaterialSales(const AuCommSolicitConstructionMaterialSales& c) :
	AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitConstructionMaterialSales::Clone() const
{
	return new AuCommSolicitConstructionMaterialSales(*this);
}

QString AuCommSolicitConstructionMaterialSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitConstructionMaterialSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitConstructionMaterialSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitConstructionMaterialSales::Text() const
{
	return QObject::tr("Does anyone have any construction materials for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitConstructionMaterialSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitFoodSales::AuCommSolicitFoodSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitFoodSales::AuCommSolicitFoodSales()
{
}

AuCommSolicitFoodSales::AuCommSolicitFoodSales(const AuCommSolicitFoodSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitFoodSales::Clone() const
{
	return new AuCommSolicitFoodSales(*this);
}

QString AuCommSolicitFoodSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitFoodSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitFoodSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitFoodSales::Text() const
{
	return QObject::tr("Does anyone have any food for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitFoodSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitFuelSales::AuCommSolicitFuelSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitFuelSales::AuCommSolicitFuelSales()
{
}

AuCommSolicitFuelSales::AuCommSolicitFuelSales(const AuCommSolicitFuelSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitFuelSales::Clone() const
{
	return new AuCommSolicitFuelSales(*this);
}

QString AuCommSolicitFuelSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitFuelSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitFuelSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitFuelSales::Text() const
{
	return QObject::tr("Does anyone have any fuel for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitFuelSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitItemSales::AuCommSolicitItemSales(COMMODITYID cid,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_CID = cid;
}

AuCommSolicitItemSales::AuCommSolicitItemSales()
{
	m_CID = CID_Invalid;
}

AuCommSolicitItemSales::AuCommSolicitItemSales(const AuCommSolicitItemSales& c) : AuCommPacket(c)
{
	m_CID = c.m_CID;
}

AuCommPacket* AuCommSolicitItemSales::Clone() const
{
	return new AuCommSolicitItemSales(*this);
}

QString AuCommSolicitItemSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitItemSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint16 ui16;

	in >> ui16;
	m_CID = (COMMODITYID)ui16;
}

void AuCommSolicitItemSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << (quint16)m_CID;
}

// Returns the text of the comm message.
QString AuCommSolicitItemSales::Text() const
{
	return QObject::tr("Does anyone have any %1 for sale?").arg(g_Au.CommodityName(m_CID));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitItemSales::Type() const
{
	return CommPacketType();
}

COMMODITYID AuCommSolicitItemSales::CommodityId() const
{
	return m_CID;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitMedSales::AuCommSolicitMedSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitMedSales::AuCommSolicitMedSales()
{
}

AuCommSolicitMedSales::AuCommSolicitMedSales(const AuCommSolicitMedSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitMedSales::Clone() const
{
	return new AuCommSolicitMedSales(*this);
}

QString AuCommSolicitMedSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitMedSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitMedSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitMedSales::Text() const
{
	return QObject::tr("Does anyone have any medical supplies for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitMedSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitPartsSales::AuCommSolicitPartsSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitPartsSales::AuCommSolicitPartsSales()
{
}

AuCommSolicitPartsSales::AuCommSolicitPartsSales(const AuCommSolicitPartsSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitPartsSales::Clone() const
{
	return new AuCommSolicitPartsSales(*this);
}

QString AuCommSolicitPartsSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitPartsSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitPartsSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitPartsSales::Text() const
{
	return QObject::tr("Does anyone have any spare parts for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitPartsSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitPatentSales::AuCommSolicitPatentSales(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitPatentSales::AuCommSolicitPatentSales()
{
}

AuCommSolicitPatentSales::AuCommSolicitPatentSales(const AuCommSolicitPatentSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitPatentSales::Clone() const
{
	return new AuCommSolicitPatentSales(*this);
}

QString AuCommSolicitPatentSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitPatentSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitPatentSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitPatentSales::Text() const
{
	return QObject::tr("Does anyone have any patents for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitPatentSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitMiscSales::AuCommSolicitMiscSales(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitMiscSales::AuCommSolicitMiscSales()
{
}

AuCommSolicitMiscSales::AuCommSolicitMiscSales(const AuCommSolicitMiscSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitMiscSales::Clone() const
{
	return new AuCommSolicitMiscSales(*this);
}

QString AuCommSolicitMiscSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitMiscSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitMiscSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitMiscSales::Text() const
{
	return QObject::tr("Does anyone have any miscellanous items for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitMiscSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitToolSales::AuCommSolicitToolSales(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitToolSales::AuCommSolicitToolSales()
{
}

AuCommSolicitToolSales::AuCommSolicitToolSales(const AuCommSolicitToolSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitToolSales::Clone() const
{
	return new AuCommSolicitToolSales(*this);
}

QString AuCommSolicitToolSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitToolSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitToolSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitToolSales::Text() const
{
	return QObject::tr("Does anyone have any tools for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitToolSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitWeaponSales::AuCommSolicitWeaponSales(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitWeaponSales::AuCommSolicitWeaponSales()
{
}

AuCommSolicitWeaponSales::AuCommSolicitWeaponSales(const AuCommSolicitWeaponSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitWeaponSales::Clone() const
{
	return new AuCommSolicitWeaponSales(*this);
}

QString AuCommSolicitWeaponSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitWeaponSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitWeaponSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitWeaponSales::Text() const
{
	return QObject::tr("Does anyone have any weapons for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitWeaponSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitNaturalResourceSales::AuCommSolicitNaturalResourceSales(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitNaturalResourceSales::AuCommSolicitNaturalResourceSales()
{
}

AuCommSolicitNaturalResourceSales::AuCommSolicitNaturalResourceSales(const AuCommSolicitNaturalResourceSales& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitNaturalResourceSales::Clone() const
{
	return new AuCommSolicitNaturalResourceSales(*this);
}

QString AuCommSolicitNaturalResourceSales::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitNaturalResourceSales::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitNaturalResourceSales::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitNaturalResourceSales::Text() const
{
	return QObject::tr("Does anyone have any natural resources for sale?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitNaturalResourceSales::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSpotTargetOrTerrain::AuCommSpotTargetOrTerrain(const QString& text, POLITICAL_STANCE stanceTowardTarget, bool isCivilian,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_IsCivilian = isCivilian;
	m_StanceTowardTarget = stanceTowardTarget;
	m_Text = text;
}

AuCommSpotTargetOrTerrain::AuCommSpotTargetOrTerrain()
{
	m_IsCivilian = false;
	m_StanceTowardTarget = POL_Neutral;
}

AuCommSpotTargetOrTerrain::AuCommSpotTargetOrTerrain(const AuCommSpotTargetOrTerrain& c) : AuCommPacket(c)
{
	m_IsCivilian = c.m_IsCivilian;
	m_StanceTowardTarget = c.m_StanceTowardTarget;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommSpotTargetOrTerrain::Clone() const
{
	return new AuCommSpotTargetOrTerrain(*this);
}

QString AuCommSpotTargetOrTerrain::Icon() const
{
	return CommPacketIcon();
}

void AuCommSpotTargetOrTerrain::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 ui8 = m_StanceTowardTarget;
	if(m_IsCivilian)
		ui8 |= 0x80;
	out << ui8;
	out << m_Text;
}

void AuCommSpotTargetOrTerrain::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_StanceTowardTarget = (POLITICAL_STANCE)(ui8 & 0x03);
	m_IsCivilian = ui8 & 0x80;
	in >> m_Text;
}

// Returns the text of the comm message.
QString AuCommSpotTargetOrTerrain::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSpotTargetOrTerrain::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommTroopBoardingComplete::AuCommTroopBoardingComplete(const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommTroopBoardingComplete::AuCommTroopBoardingComplete()
{
}

AuCommTroopBoardingComplete::AuCommTroopBoardingComplete(const AuCommTroopBoardingComplete& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommTroopBoardingComplete::Clone() const
{
	return new AuCommTroopBoardingComplete(*this);
}

QString AuCommTroopBoardingComplete::Icon() const
{
	return CommPacketIcon();
}

void AuCommTroopBoardingComplete::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommTroopBoardingComplete::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommTroopBoardingComplete::Text() const
{
	return QObject::tr("Troop boarding complete. Kicking troops from the roster who are not aboard.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommTroopBoardingComplete::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommTroopsBoarded::AuCommTroopsBoarded(const AuId& loaderId, quint16 count, bool beamedUp,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BeamedUp = beamedUp;
	m_Count = count;
	m_LoaderId = loaderId;
}

AuCommTroopsBoarded::AuCommTroopsBoarded()
{
	m_BeamedUp = false;
	m_Count = 0;
}

AuCommTroopsBoarded::AuCommTroopsBoarded(const AuCommTroopsBoarded& c) : AuCommPacket(c)
{
	m_BeamedUp = c.m_BeamedUp;
	m_Count = c.m_Count;
	m_LoaderId = c.m_LoaderId;
}

AuCommPacket* AuCommTroopsBoarded::Clone() const
{
	return new AuCommTroopsBoarded(*this);
}

QString AuCommTroopsBoarded::Icon() const
{
	return CommPacketIcon();
}

void AuCommTroopsBoarded::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	quint8 verbeam = m_BeamedUp;
	verbeam |= 0x80;
	out << verbeam;
	out << m_Count;
	out << m_LoaderId;
}

void AuCommTroopsBoarded::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;

	in >> ui8;
	m_BeamedUp = ui8 & 0x01;
	if(ui8 & 0x80)
		in >> m_Count;
	else
	{
		quint8 ocount;
		in >> ocount;
		m_Count = ocount;
	}
	in >> m_LoaderId;
}

// Returns the text of the comm message.
QString AuCommTroopsBoarded::Text() const
{
	QString text;
	if(m_BeamedUp)
	{
		if(m_Count == 1)
			return QObject::tr("One troop beamed aboard the ship.");
		else
			return QObject::tr("%L1 troops beamed aboard the ship.")
				.arg(m_Count);
	}
	else
	{
		if(m_Count == 1)
			return QObject::tr("One troop boarded.");
		else
			return QObject::tr("%L1 troops boarded.")
				.arg(m_Count);
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommTroopsBoarded::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommDiplomaticMessage::AuCommDiplomaticMessage(const QString& text,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Text = text;
}

AuCommDiplomaticMessage::AuCommDiplomaticMessage()
{
}

AuCommDiplomaticMessage::AuCommDiplomaticMessage(const AuCommDiplomaticMessage& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommDiplomaticMessage::Clone() const
{
	return new AuCommDiplomaticMessage(*this);
}

QString AuCommDiplomaticMessage::Icon() const
{
	return CommPacketIcon();
}

void AuCommDiplomaticMessage::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Text;
	if(m_Text.length() > MaxChatText())
		m_Text = m_Text.left(MaxChatText()).trimmed();
	else
		m_Text = m_Text.trimmed();
}

void AuCommDiplomaticMessage::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_Text.length() > MaxChatText())
		out << m_Text.left(MaxChatText()).trimmed();
	else
		out << m_Text.trimmed();
}

// Returns the text of the comm message.
QString AuCommDiplomaticMessage::Text() const
{
	return QObject::tr("Deliver this message to your leaders: %1").arg(m_Text);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommDiplomaticMessage::Type() const
{
	return CommPacketType();
}

bool AuCommDiplomaticMessage::IsValid() const
{
	if(m_Text.isEmpty())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCitySurrenderOffered::AuCommCitySurrenderOffered(quint32 fromEmpireId, quint32 toEmpireId, const AuIdList& cityIds,
		const QStringList& cityNames,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_CityIds = cityIds;
	m_CityNames = cityNames;
	m_FromEmpireId = fromEmpireId;
	m_ToEmpireId = toEmpireId;
}

AuCommCitySurrenderOffered::AuCommCitySurrenderOffered()
{
	m_FromEmpireId = 0;
	m_ToEmpireId = 0;
}

AuCommCitySurrenderOffered::AuCommCitySurrenderOffered(const AuCommCitySurrenderOffered& c) : AuCommPacket(c)
{
	m_CityIds = c.m_CityIds;
	m_CityNames = c.m_CityNames;
	m_FromEmpireId = c.m_FromEmpireId;
	m_ToEmpireId = c.m_ToEmpireId;
}

AuCommPacket* AuCommCitySurrenderOffered::Clone() const
{
	return new AuCommCitySurrenderOffered(*this);
}

QString AuCommCitySurrenderOffered::Icon() const
{
	return CommPacketIcon();
}

void AuCommCitySurrenderOffered::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_CityIds;
	in >> m_CityNames;
	in >> m_FromEmpireId;
	in >> m_ToEmpireId;
}

void AuCommCitySurrenderOffered::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	out << m_CityIds;
	out << m_CityNames;
	out << m_FromEmpireId;
	out << m_ToEmpireId;
}

// Returns the text of the comm message.
QString AuCommCitySurrenderOffered::Text() const
{
	if(m_CityIds.count() == 1)
		return QObject::tr("Please accept surrender of the city of %1 to your empire.").arg(m_CityNames.first());

	QString text = QObject::tr("Please accept surrender of these cities to your empire: ");
	for(QStringList::const_iterator ci = m_CityNames.cbegin(); ci != m_CityNames.cend(); ci++)
	{
		if(ci == m_CityNames.cbegin())
			text += *ci;
		else if(ci == m_CityNames.cend()-1)
			text += QObject::tr(" and %1").arg(*ci);
		else
			text += QString(", %1").arg(*ci);
	}
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCitySurrenderOffered::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCitySurrenderAccepted::AuCommCitySurrenderAccepted(quint32 acceptingEmpireId, const AuId& surrenderingAvatarId,  
		quint32 surrenderingEmpireId, const AuIdList& cityIds, const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_AcceptingEmpireId = acceptingEmpireId;
	m_CityIds = cityIds;
	m_SurrenderingAvatarId = surrenderingAvatarId;
	m_SurrenderingEmpireId = surrenderingEmpireId;
}

AuCommCitySurrenderAccepted::AuCommCitySurrenderAccepted()
{
	m_AcceptingEmpireId = 0;
	m_SurrenderingEmpireId = 0;
}

AuCommCitySurrenderAccepted::AuCommCitySurrenderAccepted(const AuCommCitySurrenderAccepted& c) : AuCommPacket(c)
{
	m_AcceptingEmpireId = c.m_AcceptingEmpireId;
	m_CityIds = c.m_CityIds;
	m_SurrenderingAvatarId = c.m_SurrenderingAvatarId;
	m_SurrenderingEmpireId = c.m_SurrenderingEmpireId;
}

AuCommPacket* AuCommCitySurrenderAccepted::Clone() const
{
	return new AuCommCitySurrenderAccepted(*this);
}

QString AuCommCitySurrenderAccepted::Icon() const
{
	return CommPacketIcon();
}

void AuCommCitySurrenderAccepted::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_AcceptingEmpireId;
	in >> m_CityIds;
	in >> m_SurrenderingAvatarId;
	in >> m_SurrenderingEmpireId;
}

void AuCommCitySurrenderAccepted::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	out << m_AcceptingEmpireId;
	out << m_CityIds;
	out << m_SurrenderingAvatarId;
	out << m_SurrenderingEmpireId;
}

// Returns the text of the comm message.
QString AuCommCitySurrenderAccepted::Text() const
{
	if(m_CityIds.count() == 1)
		return QObject::tr("I accept surrender of that city.");
	return QObject::tr("I accept surrender of those cities.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCitySurrenderAccepted::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOS::AuCommSOS(const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Loc = loc;
}

AuCommSOS::AuCommSOS()
{
}

AuCommSOS::AuCommSOS(const AuCommSOS& c) : AuCommPacket(c)
{
	m_Loc = c.m_Loc;
}

QString AuCommSOS::Icon() const
{
	return CommPacketIcon();
}

void AuCommSOS::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_Loc;
	m_Loc = m_Loc.trimmed();
}

void AuCommSOS::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Loc.trimmed();
}

bool AuCommSOS::IsValid() const
{
	if(m_Loc.isEmpty())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSManeuverDriveAdrift::AuCommSOSManeuverDriveAdrift(const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSOSManeuverDriveAdrift::AuCommSOSManeuverDriveAdrift()
{
}

AuCommSOSManeuverDriveAdrift::AuCommSOSManeuverDriveAdrift(const AuCommSOSManeuverDriveAdrift& c) : AuCommSOS(c)
{
}

AuCommPacket* AuCommSOSManeuverDriveAdrift::Clone() const
{
	return new AuCommSOSManeuverDriveAdrift(*this);
}

void AuCommSOSManeuverDriveAdrift::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
}

void AuCommSOSManeuverDriveAdrift::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSOSManeuverDriveAdrift::Text() const
{
	QString text = QObject::tr("Mayday! Our maneuver drive is out. "
			"We're adrift! %1")
		.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSManeuverDriveAdrift::Type() const
{
	return CommPacketType();
}

bool AuCommSOSManeuverDriveAdrift::IsValid() const
{
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSManeuverDriveMarooned::AuCommSOSManeuverDriveMarooned(const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSOSManeuverDriveMarooned::AuCommSOSManeuverDriveMarooned()
{
}

AuCommSOSManeuverDriveMarooned::AuCommSOSManeuverDriveMarooned(const AuCommSOSManeuverDriveMarooned& c) : AuCommSOS(c)
{
}

AuCommPacket* AuCommSOSManeuverDriveMarooned::Clone() const
{
	return new AuCommSOSManeuverDriveMarooned(*this);
}

void AuCommSOSManeuverDriveMarooned::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
}

void AuCommSOSManeuverDriveMarooned::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSOSManeuverDriveMarooned::Text() const
{
	QString text = QObject::tr("Mayday! Our maneuver drive is out. "
			"We're marooned! %1")
		.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSManeuverDriveMarooned::Type() const
{
	return CommPacketType();
}

bool AuCommSOSManeuverDriveMarooned::IsValid() const
{
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSManeuverDriveStation::AuCommSOSManeuverDriveStation(const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSOSManeuverDriveStation::AuCommSOSManeuverDriveStation()
{
}

AuCommSOSManeuverDriveStation::AuCommSOSManeuverDriveStation(const AuCommSOSManeuverDriveStation& c) : AuCommSOS(c)
{
}

AuCommPacket* AuCommSOSManeuverDriveStation::Clone() const
{
	return new AuCommSOSManeuverDriveStation(*this);
}

void AuCommSOSManeuverDriveStation::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
}

void AuCommSOSManeuverDriveStation::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSOSManeuverDriveStation::Text() const
{
	QString text = QObject::tr("Mayday! Our maneuver drive is out. "
			"We're going in hard! %1")
		.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSManeuverDriveStation::Type() const
{
	return CommPacketType();
}

bool AuCommSOSManeuverDriveStation::IsValid() const
{
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSAttack::AuCommSOSAttack(const QString& nameWithEmpire, const QString& attackerName, const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_AttackerName = attackerName;
	m_NameWithEmpire = nameWithEmpire;
}

AuCommSOSAttack::AuCommSOSAttack()
{
}

AuCommSOSAttack::AuCommSOSAttack(const AuCommSOSAttack& c) : AuCommSOS(c)
{
	m_AttackerName = c.m_AttackerName;
	m_NameWithEmpire = c.m_NameWithEmpire;
}

AuCommPacket* AuCommSOSAttack::Clone() const
{
	return new AuCommSOSAttack(*this);
}

void AuCommSOSAttack::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
	in >> m_AttackerName;
	in >> m_NameWithEmpire;
}

void AuCommSOSAttack::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
	out << m_AttackerName;
	out << m_NameWithEmpire;
}

// Returns the text of the comm message.
QString AuCommSOSAttack::Text() const
{
	QString text = QObject::tr("%1 is under attack by %2! %3")
		.arg(m_NameWithEmpire)
		.arg(m_AttackerName)
		.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSAttack::Type() const
{
	return CommPacketType();
}

bool AuCommSOSAttack::IsValid() const
{
	if(m_AttackerName.isEmpty())
		return false;
	if(m_NameWithEmpire.isEmpty())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSFuelMarooned::AuCommSOSFuelMarooned(const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSOSFuelMarooned::AuCommSOSFuelMarooned()
{
}

AuCommSOSFuelMarooned::AuCommSOSFuelMarooned(const AuCommSOSFuelMarooned& c) : AuCommSOS(c)
{
}

AuCommPacket* AuCommSOSFuelMarooned::Clone() const
{
	return new AuCommSOSFuelMarooned(*this);
}

void AuCommSOSFuelMarooned::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
}

void AuCommSOSFuelMarooned::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSOSFuelMarooned::Text() const
{
	QString text = QObject::tr("We have a fuel emergency! "
			"We're marooned! %1")
		.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSFuelMarooned::Type() const
{
	return CommPacketType();
}

bool AuCommSOSFuelMarooned::IsValid() const
{
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSFuelShip::AuCommSOSFuelShip(quint8 fuelPercent, const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_FuelPercent = fuelPercent;
}

AuCommSOSFuelShip::AuCommSOSFuelShip()
{
	m_FuelPercent = 0;
}

AuCommSOSFuelShip::AuCommSOSFuelShip(const AuCommSOSFuelShip& c) : AuCommSOS(c)
{
	m_FuelPercent = c.m_FuelPercent;
}

AuCommPacket* AuCommSOSFuelShip::Clone() const
{
	return new AuCommSOSFuelShip(*this);
}

void AuCommSOSFuelShip::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
	in >> m_FuelPercent;
}

void AuCommSOSFuelShip::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
	out << m_FuelPercent;
}

// Returns the text of the comm message.
QString AuCommSOSFuelShip::Text() const
{
	QString text;
	if(m_FuelPercent)
		text = QObject::tr("Mayday! We have a fuel emergency! "
				"Flying on capacitor power only, %1% remaining. %2")
			.arg(m_FuelPercent)
			.arg(LocDesc());
	else
		text = QObject::tr("Mayday! We have a fuel emergency and our capacitor is empty! "
				"We're adrift. %1")
			.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSFuelShip::Type() const
{
	return CommPacketType();
}

bool AuCommSOSFuelShip::IsValid() const
{
	if(m_FuelPercent > 100)
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSOSFuelStation::AuCommSOSFuelStation(quint8 fuelPercent, const QString& loc,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommSOS(loc, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_FuelPercent = fuelPercent;
}

AuCommSOSFuelStation::AuCommSOSFuelStation()
{
	m_FuelPercent = 0;
}

AuCommSOSFuelStation::AuCommSOSFuelStation(const AuCommSOSFuelStation& c) : AuCommSOS(c)
{
	m_FuelPercent = c.m_FuelPercent;
}

AuCommPacket* AuCommSOSFuelStation::Clone() const
{
	return new AuCommSOSFuelStation(*this);
}

void AuCommSOSFuelStation::operator<<(QDataStream& in)
{
	AuCommSOS::operator<<(in);
	in >> m_FuelPercent;
}

void AuCommSOSFuelStation::operator>>(QDataStream& out) const
{
	AuCommSOS::operator>>(out);
	out << m_FuelPercent;
}

// Returns the text of the comm message.
QString AuCommSOSFuelStation::Text() const
{
	QString text;
	if(m_FuelPercent)
		text = QObject::tr("Mayday! We have a fuel emergency! "
				"This station is maintaining orbit on capacitor power only, %1% remaining. %2")
			.arg(m_FuelPercent)
			.arg(LocDesc());
	else
		text = QObject::tr("Mayday! We have a fuel emergency and our capacitor is empty! "
				"This station's orbit is decaying rapidly. %1")
			.arg(LocDesc());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSOSFuelStation::Type() const
{
	return CommPacketType();
}

bool AuCommSOSFuelStation::IsValid() const
{
	if(m_FuelPercent > 100)
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferAvatar::AuCommOfferAvatar(const AuId& toId, const QString& toName,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ToId = toId;
	m_ToName = toName;
}

AuCommOfferAvatar::AuCommOfferAvatar()
{
}

AuCommOfferAvatar::AuCommOfferAvatar(const AuCommOfferAvatar& c) : AuCommPacket(c)
{
	m_ToId = c.m_ToId;
	m_ToName = c.m_ToName;
}

AuCommPacket* AuCommOfferAvatar::Clone() const
{
	return new AuCommOfferAvatar(*this);
}

QString AuCommOfferAvatar::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferAvatar::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ToId;
	in >> m_ToName;
}

void AuCommOfferAvatar::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ToId;
	out << m_ToName;
}

// Returns the text of the comm message.
QString AuCommOfferAvatar::Text() const
{
	return QObject::tr("%1, please accept me as your avatar.").arg(m_ToName);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferAvatar::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptAvatar::AuCommAcceptAvatar(const AuId& avatarId, const QString& avatarName,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_AvatarId = avatarId;
	m_AvatarName = avatarName;
}

AuCommAcceptAvatar::AuCommAcceptAvatar()
{
}

AuCommAcceptAvatar::AuCommAcceptAvatar(const AuCommAcceptAvatar& c) : AuCommPacket(c)
{
	m_AvatarId = c.m_AvatarId;
	m_AvatarName = c.m_AvatarName;
}

AuCommPacket* AuCommAcceptAvatar::Clone() const
{
	return new AuCommAcceptAvatar(*this);
}

QString AuCommAcceptAvatar::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptAvatar::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	in >> m_AvatarId;
	in >> m_AvatarName;
}

void AuCommAcceptAvatar::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	out << m_AvatarId;
	out << m_AvatarName;
}

// Returns the text of the comm message.
QString AuCommAcceptAvatar::Text() const
{
	return QObject::tr("I accept you, avatar %1").arg(m_AvatarName);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptAvatar::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommShipStasisDimension::AuCommShipStasisDimension(const AuTime& readyTime,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ReadyTime = readyTime;
}

AuCommShipStasisDimension::AuCommShipStasisDimension()
{
}

AuCommShipStasisDimension::AuCommShipStasisDimension(const AuCommShipStasisDimension& c) : AuCommPacket(c)
{
	m_ReadyTime = c.m_ReadyTime;
}

AuCommPacket* AuCommShipStasisDimension::Clone() const
{
	return new AuCommShipStasisDimension(*this);
}

QString AuCommShipStasisDimension::Icon() const
{
	return CommPacketIcon();
}

void AuCommShipStasisDimension::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ReadyTime;
}

void AuCommShipStasisDimension::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ReadyTime;
}

// Returns the text of the comm message.
QString AuCommShipStasisDimension::Text() const
{
	/*
	if(AuTime::Now() >= m_ReadyTime)
		return QObject::tr("Sir, this spacecraft is ready to be recovered from stasis in the %1. "
				"You must go to a repair shop in the %1 to recover the ship.")
			.arg(::DimensionName(::Dimension(FromGalaxy())));
	else
		return QObject::tr("Sir, this spacecraft is in stasis in the %1 until %2. "
				"You must go to a repair shop in the %1 to recover the ship.")
			.arg(::DimensionName(::Dimension(FromGalaxy())))
			.arg(m_ReadyTime.UTC().toLocalTime().toString());
			*/
	return "wut?";
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommShipStasisDimension::Type() const
{
	return CommPacketType();
}

bool AuCommShipStasisDimension::IsValid() const
{
	if(m_ReadyTime.IsNull())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommShipStasisWait::AuCommShipStasisWait(const AuTime& readyTime,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ReadyTime = readyTime;
}

AuCommShipStasisWait::AuCommShipStasisWait()
{
}

AuCommShipStasisWait::AuCommShipStasisWait(const AuCommShipStasisWait& c) : AuCommPacket(c)
{
	m_ReadyTime = c.m_ReadyTime;
}

AuCommPacket* AuCommShipStasisWait::Clone() const
{
	return new AuCommShipStasisWait(*this);
}

QString AuCommShipStasisWait::Icon() const
{
	return CommPacketIcon();
}

void AuCommShipStasisWait::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ReadyTime;
}

void AuCommShipStasisWait::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ReadyTime;
}

// Returns the text of the comm message.
QString AuCommShipStasisWait::Text() const
{
	return QObject::tr("Sir, this spacecraft is in stasis until %1. "
			"You must go to an airport repair shop to recover the ship.")
		.arg(m_ReadyTime.UTC().toLocalTime().toString());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommShipStasisWait::Type() const
{
	return CommPacketType();
}

bool AuCommShipStasisWait::IsValid() const
{
	if(m_ReadyTime.IsNull())
		return false;
	return AuCommPacket::IsValid();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSilverwill::AuCommSilverwill(const QString& text, const QString& channelName) :
	AuCommPacket(channelName, AuId::Silverwill(), QObject::tr("Silverwill"), AuId(1), GALAXY_Count,
			AuCommStdChannel::MaxChannelRange(channelName), AuCommStdChannel::Thoughts)
{
	m_Text = text;
}

AuCommSilverwill::AuCommSilverwill()
{
}

AuCommSilverwill::AuCommSilverwill(const AuCommSilverwill& c) : AuCommPacket(c)
{
	m_Text = c.m_Text;
}

AuCommPacket* AuCommSilverwill::Clone() const
{
	return new AuCommSilverwill(*this);
}

QString AuCommSilverwill::Icon() const
{
	return CommPacketIcon();
}

void AuCommSilverwill::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	SetAnonymous(false);									// Silverwill messages are never anonymous but they may come from id 1.
	in >> m_Text;
}

void AuCommSilverwill::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommSilverwill::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSilverwill::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRequestRefit::AuCommRequestRefit(const AuId& shipId, const AuRefitShip& refit,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_Refit(refit)
{
	m_ShipId = shipId;
}

AuCommRequestRefit::AuCommRequestRefit()
{
}

AuCommRequestRefit::AuCommRequestRefit(const AuCommRequestRefit& c) : AuCommPacket(c), m_Refit(c.m_Refit)
{
	m_ShipId = c.m_ShipId;
}

AuCommPacket* AuCommRequestRefit::Clone() const
{
	return new AuCommRequestRefit(*this);
}

QString AuCommRequestRefit::Icon() const
{
	return CommPacketIcon();
}

void AuCommRequestRefit::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ShipId;
	out << m_Refit;
}

void AuCommRequestRefit::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ShipId;
	in >> m_Refit;
}

// Returns the text of the comm message.
QString AuCommRequestRefit::Text() const
{
	return QObject::tr("I request a refit estimate according to the attached specifications.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRequestRefit::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferRefit::AuCommOfferRefit(const AuId& shipId, const AuRefitShip& refit, double estCost, quint32 estSec,
		const AuRefitMaterials& estMat, quint16 workers,
		const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_EstMaterials(estMat), m_Refit(refit)
{
	m_EstCost = estCost;
	m_EstSeconds = estSec;
	m_ShipId = shipId;
	m_Workers = workers;
}

AuCommOfferRefit::AuCommOfferRefit()
{
	m_EstCost = 0;
	m_EstSeconds = 0;
	m_Workers = 0;
}

AuCommOfferRefit::AuCommOfferRefit(const AuCommOfferRefit& c) : AuCommPacket(c), m_EstMaterials(c.m_EstMaterials),
	m_Refit(c.m_Refit)
{
	m_EstCost = c.m_EstCost;
	m_EstSeconds = c.m_EstSeconds;
	m_ShipId = c.m_ShipId;
	m_Workers = c.m_Workers;
}

AuCommPacket* AuCommOfferRefit::Clone() const
{
	return new AuCommOfferRefit(*this);
}

QString AuCommOfferRefit::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferRefit::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ShipId;
	out << m_Refit;
	out << m_EstCost;
	out << m_EstMaterials;
	out << m_EstSeconds;
	out << m_Workers;
}

void AuCommOfferRefit::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ShipId;
	in >> m_Refit;
	in >> m_EstCost;
	in >> m_EstMaterials;
	in >> m_EstSeconds;
	in >> m_Workers;
}

// Returns the text of the comm message.
QString AuCommOfferRefit::Text() const
{
	quint32 secs = m_EstSeconds;
	int days = secs / SecondsPerDay();
	if(days)
		secs -= days * SecondsPerDay();
	int hours = secs / (60*60);
	if(hours)
		secs -= hours * (60*60);
	int mins = secs / 60;
	if(mins)
		secs -= mins * 60;

	// List materials.
	QString smatl;
	if(!m_EstMaterials.m_Materials.empty())
	{
		smatl = QObject::tr("Materials needed: ");
		for(QHash<COMMODITYID, quint32>::const_iterator mi = m_EstMaterials.m_Materials.cbegin();
				mi != m_EstMaterials.m_Materials.cend(); mi++)
		{
			if(*mi > 1)
			{
				if(mi == m_EstMaterials.m_Materials.cbegin())
					smatl += QString("%L1 %2").arg(*mi).arg(g_Au.CommodityName(mi.key()));
				else
					smatl += QString(", %L1 %2").arg(*mi).arg(g_Au.CommodityName(mi.key()));
			}
			else
			{
				if(mi == m_EstMaterials.m_Materials.cbegin())
					smatl += QString("%2").arg(g_Au.CommodityName(mi.key()));
				else
					smatl += QString(", %2").arg(g_Au.CommodityName(mi.key()));
			}
		}
		smatl += ".";
	}
	else
		smatl = QObject::tr("No additional materials will be needed.");

	// Minimum quality.
	QString minq;
	if(m_Refit.m_MinQ > 1)
		minq = QObject::tr("Parts will be at least Q%1.").arg(m_Refit.m_MinQ);

	QString price;
	if(m_EstCost == 0)
		price = QObject::tr("free");
	else
		price = QString("%L1%2").arg(m_EstCost, 0,'f',0).arg(AuGlobal::CurrencySymbol());

	if(days)
		return QObject::tr("Our repair shop offers to do the refit for %1. "
				"%L8 specialists will do the work. "
				"The ship will remain moored in dry dock for %3 days, %4 hours, %5 minutes. "
				"%6 %7")
			.arg(price)
			.arg(days)
			.arg(hours)
			.arg(mins)
			.arg(smatl)
			.arg(minq)
			.arg(m_Workers);
	else if(hours)
		return QObject::tr("Our repair shop offers to do the refit for %1. "
				"%L8 specialists will do the work. "
				"The ship will remain moored in dry dock for %3 hours, %4 minutes. "
				"%6")
			.arg(price)
			.arg(hours)
			.arg(mins)
			.arg(smatl)
			.arg(m_Workers);
	else if(mins > 1)
		return QObject::tr("Our repair shop offers to do the refit for %1. "
				"%L8 specialists will do the work. "
				"The ship will remain moored in dry dock for %3 minutes. "
				"%6")
			.arg(price)
			.arg(mins)
			.arg(smatl)
			.arg(m_Workers);
	else
		return QObject::tr("Our repair shop offers to do the refit for %1. "
				"%L8 specialists will do the work. "
				"The ship will remain moored in dry dock for about a minute. "
				"%6")
			.arg(price)
			.arg(smatl)
			.arg(m_Workers);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferRefit::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptRefit::AuCommAcceptRefit(const AuId& cityId, const AuId& shipId, const AuRefitShip& refit, double estCost,
		quint32 estSec, const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
		GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope), m_Refit(refit)
{
	m_CityId = cityId;
	m_EstCost = estCost;
	m_EstSeconds = estSec;
	m_ShipId = shipId;
}

AuCommAcceptRefit::AuCommAcceptRefit()
{
	m_EstCost = 0;
	m_EstSeconds = 0;
}

AuCommAcceptRefit::AuCommAcceptRefit(const AuCommAcceptRefit& c) : AuCommPacket(c), m_Refit(c.m_Refit)
{
	m_CityId = c.m_CityId;
	m_EstCost = c.m_EstCost;
	m_EstSeconds = c.m_EstSeconds;
	m_ShipId = c.m_ShipId;
}

AuCommPacket* AuCommAcceptRefit::Clone() const
{
	return new AuCommAcceptRefit(*this);
}

QString AuCommAcceptRefit::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptRefit::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ShipId;
	out << m_Refit;
	out << m_EstCost;
	out << m_EstSeconds;
	out << m_CityId;
}

void AuCommAcceptRefit::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ShipId;
	in >> m_Refit;
	in >> m_EstCost;
	in >> m_EstSeconds;
	in >> m_CityId;
}

// Returns the text of the comm message.
QString AuCommAcceptRefit::Text() const
{
	return QObject::tr("Refit of spacecraft systems for %L1%2 accepted.")
		.arg(m_EstCost, 0,'f',0)
		.arg(AuGlobal::CurrencySymbol());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptRefit::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChannelOpened::AuCommChannelOpened(const AuId& toId, const QString& toName, const QString& channelName, const AuId& fromId,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
		AuCommStdChannel::SCOPE scope) : AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ToId = toId;
	m_ToName = toName;
}

AuCommChannelOpened::AuCommChannelOpened()
{
}

AuCommChannelOpened::AuCommChannelOpened(const AuCommChannelOpened& c) : AuCommPacket(c)
{
	m_ToId = c.m_ToId;
	m_ToName = c.m_ToName;
}

AuCommPacket* AuCommChannelOpened::Clone() const
{
	return new AuCommChannelOpened(*this);
}

QString AuCommChannelOpened::Icon() const
{
	return CommPacketIcon();
}

void AuCommChannelOpened::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ToId;
	out << m_ToName;
}

void AuCommChannelOpened::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ToId;
	in >> m_ToName;
}

// Returns the text of the comm message.
QString AuCommChannelOpened::Text() const
{
	return QObject::tr("Opened channel '%1' to %2 %3.").arg(ChannelName()).arg(m_ToName).arg(m_ToId.StringId());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChannelOpened::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommInviteTeleport::AuCommInviteTeleport(const AuId& toId, const QString& toName,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ToId = toId;
	m_ToName = toName;
}

AuCommInviteTeleport::AuCommInviteTeleport()
{
}

AuCommInviteTeleport::AuCommInviteTeleport(const AuCommInviteTeleport& c) : AuCommPacket(c)
{
	m_ToId = c.m_ToId;
	m_ToName = c.m_ToName;
}

AuCommPacket* AuCommInviteTeleport::Clone() const
{
	return new AuCommInviteTeleport(*this);
}

QString AuCommInviteTeleport::Icon() const
{
	return QString();		// nerf
	//return CommPacketIcon();
}

void AuCommInviteTeleport::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ToId;
	in >> m_ToName;
}

void AuCommInviteTeleport::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ToId;
	out << m_ToName;
}

// Returns the text of the comm message.
QString AuCommInviteTeleport::Text() const
{
	return QObject::tr("%1, please accept this invitation to teleport to my location.").arg(m_ToName);
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommInviteTeleport::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptTeleport::AuCommAcceptTeleport(const AuId& toId,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_ToId = toId;
}

AuCommAcceptTeleport::AuCommAcceptTeleport()
{
}

AuCommAcceptTeleport::AuCommAcceptTeleport(const AuCommAcceptTeleport& c) : AuCommPacket(c)
{
	m_ToId = c.m_ToId;
}

AuCommPacket* AuCommAcceptTeleport::Clone() const
{
	return new AuCommAcceptTeleport(*this);
}

QString AuCommAcceptTeleport::Icon() const
{
	return QString();		// nerf
	//return CommPacketIcon();
}

void AuCommAcceptTeleport::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_ToId;
}

void AuCommAcceptTeleport::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_ToId;
}

// Returns the text of the comm message.
QString AuCommAcceptTeleport::Text() const
{
	return QObject::tr("I accept your invitation to teleport.");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptTeleport::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommCombine::AuCommCombine(const AuItem& item, const QString& empireOfOrigin,
		const AuItem& item2, const QString& empireOfOrigin2,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_EmpireOfOrigin = empireOfOrigin;
	m_EmpireOfOrigin2 = empireOfOrigin2;
	m_Item = item.Clone();
	m_Item2 = item2.Clone();
}

AuCommCombine::AuCommCombine()
{
	m_Item = 0;
	m_Item2 = 0;
}

AuCommCombine::AuCommCombine(const AuCommCombine& c) : AuCommPacket(c)
{
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
	m_EmpireOfOrigin2 = c.m_EmpireOfOrigin2;
	if(c.m_Item2)
		m_Item2 = c.m_Item2->Clone();
	else
		m_Item2 = 0;
}

AuCommCombine::~AuCommCombine()
{
	if(m_Item)
		delete m_Item;
	if(m_Item2)
		delete m_Item2;
}

AuCommPacket* AuCommCombine::Clone() const
{
	return new AuCommCombine(*this);
}

QString AuCommCombine::Icon() const
{
	return CommPacketIcon();
}

void AuCommCombine::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_EmpireOfOrigin;
	in >> m_EmpireOfOrigin2;

	if(m_Item)
		delete m_Item;
	if(m_Item2)
		delete m_Item2;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;

	in >> itemType;
	if(itemType)
	{
		m_Item2 = AuItem::Create(itemType);
		if(m_Item2)
			in >> *m_Item2;
	}
	else
		m_Item2 = 0;
}

void AuCommCombine::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_EmpireOfOrigin;
	out << m_EmpireOfOrigin2;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;

	if(m_Item2)
	{
		out << m_Item2->Type();
		out << *m_Item2;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommCombine::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("Combine %1 with %2.")
		.arg(m_Item->ListDesc(m_EmpireOfOrigin))
		.arg(m_Item2->ListDesc(m_EmpireOfOrigin2));
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommCombine::Type() const
{
	return CommPacketType();
}

const QString& AuCommCombine::EmpireOfOrigin() const
{
	if(!m_EmpireOfOrigin.isEmpty())
		return m_EmpireOfOrigin;
	return m_EmpireOfOrigin2;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommRepairPartsMissing::AuCommRepairPartsMissing(const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ,
		const QList<COMMODITYID>& noSupply, const QList<COMMODITYID>& enough, quint8 minQ,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope),
	m_NoQ(noQ), m_ShortQ(shortQ), m_NoSupply(noSupply), m_Enough(enough)
{
	m_MinQ = minQ;
}

AuCommRepairPartsMissing::AuCommRepairPartsMissing()
{
	m_MinQ = 1;
}

AuCommRepairPartsMissing::AuCommRepairPartsMissing(const AuCommRepairPartsMissing& c) : AuCommPacket(c), m_NoQ(c.m_NoQ),
	m_ShortQ(c.m_ShortQ), m_NoSupply(c.m_NoSupply), m_Enough(c.m_Enough)
{
	m_MinQ = c.m_MinQ;
}

AuCommPacket* AuCommRepairPartsMissing::Clone() const
{
	return new AuCommRepairPartsMissing(*this);
}

QString AuCommRepairPartsMissing::Icon() const
{
	return CommPacketIcon();
}

void AuCommRepairPartsMissing::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	quint8 ui8;
	quint16 ui16;

	m_NoQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoQ.append((COMMODITYID)ui16);
	}

	m_ShortQ.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_ShortQ.append((COMMODITYID)ui16);
	}

	m_NoSupply.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_NoSupply.append((COMMODITYID)ui16);
	}

	m_Enough.clear();
	in >> ui8;
	for(; ui8; ui8--)
	{
		in >> ui16;
		m_Enough.append((COMMODITYID)ui16);
	}

	in >> m_MinQ;
}

void AuCommRepairPartsMissing::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	out << (quint8)m_NoQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoQ.constBegin(); mi != m_NoQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_ShortQ.count();
	for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.constBegin(); mi != m_ShortQ.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_NoSupply.count();
	for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.constBegin(); mi != m_NoSupply.constEnd(); mi++)
		out << (quint16)(*mi);

	out << (quint8)m_Enough.count();
	for(QList<COMMODITYID>::const_iterator mi = m_Enough.constBegin(); mi != m_Enough.constEnd(); mi++)
		out << (quint16)(*mi);

	out << m_MinQ;
}

// Returns the text of the comm message.
QString AuCommRepairPartsMissing::Text() const
{
	QString text;
	if(m_MinQ > 1)
		text = QObject::tr("We cannot perform any repairs, using Q%1 quality parts.").arg(m_MinQ);
	else
		text = QObject::tr("We cannot perform any repairs.");

	// Availability of parts.
	if(!m_NoQ.empty())
	{
		text += QObject::tr(" Parts unavailable at that quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoQ.cbegin(); mi != m_NoQ.cend(); mi++)
		{
			if(mi == m_NoQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_ShortQ.empty())
	{
		if(m_MinQ > 1)
			text += QObject::tr(" Parts in short supply at that quality: ");
		else
			text += QObject::tr(" Parts in short supply: ");
		for(QList<COMMODITYID>::const_iterator mi = m_ShortQ.cbegin(); mi != m_ShortQ.cend(); mi++)
		{
			if(mi == m_ShortQ.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_NoSupply.empty())
	{
		text += QObject::tr(" Parts not available: ");
		for(QList<COMMODITYID>::const_iterator mi = m_NoSupply.cbegin(); mi != m_NoSupply.cend(); mi++)
		{
			if(mi == m_NoSupply.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}
	if(!m_Enough.empty())
	{
		text += QObject::tr(" We have enough of these parts at lower quality: ");
		for(QList<COMMODITYID>::const_iterator mi = m_Enough.cbegin(); mi != m_Enough.cend(); mi++)
		{
			if(mi == m_Enough.cbegin())
				text += g_Au.CommodityName(*mi);
			else
				text += ", " + g_Au.CommodityName(*mi);
		}
		text += ".";
	}

	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommRepairPartsMissing::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommChoicePicked::AuCommChoicePicked(const QString& text,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommChat(text, channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommChoicePicked::AuCommChoicePicked()
{
}

AuCommChoicePicked::AuCommChoicePicked(const AuCommChoicePicked& c) : AuCommChat(c)
{
}

AuCommPacket* AuCommChoicePicked::Clone() const
{
	return new AuCommChoicePicked(*this);
}

QString AuCommChoicePicked::Icon() const
{
	return CommPacketIcon();
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommChoicePicked::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommShipmentItem::AuCommShipmentItem(const AuShipment& shipment,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Shipment = new AuShipment(shipment);
}

AuCommShipmentItem::AuCommShipmentItem()
{
	m_Shipment = new AuShipment();
}

AuCommShipmentItem::AuCommShipmentItem(const AuCommShipmentItem& c) : AuCommPacket(c)
{
	m_Shipment = new AuShipment(*c.m_Shipment);
}

AuCommShipmentItem::~AuCommShipmentItem()
{
	delete m_Shipment;
}

AuCommPacket* AuCommShipmentItem::Clone() const
{
	return new AuCommShipmentItem(*this);
}

QString AuCommShipmentItem::Icon() const
{
	return CommPacketIcon();
}

void AuCommShipmentItem::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << *m_Shipment;
}

void AuCommShipmentItem::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> *m_Shipment;
}

// Returns the text of the comm message.
QString AuCommShipmentItem::Text() const
{
	QString what = m_Shipment->ListName();
	QString where = m_Shipment->Container().DestinationDesc();
	quint32 teus = m_Shipment->Container().TEUCount();
	float fee = m_Shipment->Container().EstimatePayment(teus);
	return QObject::tr("%1 going to %2. On time delivery fee %L3%4.")
		.arg(what)
		.arg(where)
		.arg(fee, 0,'f',0)
		.arg(g_Au.CurrencySymbol());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommShipmentItem::Type() const
{
	return CommPacketType();
}

const AuItem* AuCommShipmentItem::Item() const
{
	return &m_Shipment->Container();
}

QString AuCommShipmentItem::Icon2() const
{
	QString cname;
	if(m_Shipment->CommodityId() != CID_Invalid)
		cname = g_Au.CommodityBitmapName(m_Shipment->CommodityId());
	else
		cname = CommodityFilterIcon(m_Shipment->CommodityFilter());
	return cname;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommSolicitShipments::AuCommSolicitShipments(const QString& channelName, const AuId& fromId, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
}

AuCommSolicitShipments::AuCommSolicitShipments()
{
}

AuCommSolicitShipments::AuCommSolicitShipments(const AuCommSolicitShipments& c) : AuCommPacket(c)
{
}

AuCommPacket* AuCommSolicitShipments::Clone() const
{
	return new AuCommSolicitShipments(*this);
}

QString AuCommSolicitShipments::Icon() const
{
	return CommPacketIcon();
}

void AuCommSolicitShipments::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
}

void AuCommSolicitShipments::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
}

// Returns the text of the comm message.
QString AuCommSolicitShipments::Text() const
{
	return QObject::tr("Does anyone have any shipments that are ready for pickup?");
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommSolicitShipments::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommBidPickupShipment::AuCommBidPickupShipment(const AuShipment& shipment,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Shipment = new AuShipment(shipment);
}

AuCommBidPickupShipment::AuCommBidPickupShipment()
{
	m_Shipment = new AuShipment();
}

AuCommBidPickupShipment::AuCommBidPickupShipment(const AuCommBidPickupShipment& c) : AuCommPacket(c)
{
	m_Shipment = new AuShipment(*c.m_Shipment);
}

AuCommBidPickupShipment::~AuCommBidPickupShipment()
{
	delete m_Shipment;
}

AuCommPacket* AuCommBidPickupShipment::Clone() const
{
	return new AuCommBidPickupShipment(*this);
}

bool AuCommBidPickupShipment::CorrespondsTo(const AuCommPacket* commPacket) const
{
	const AuCommBidPickupShipment* bid = dynamic_cast<const AuCommBidPickupShipment*>(commPacket);
	if(bid)
	{
		if(*m_Shipment != *bid->Shipment())
			return false;
		return true;
	}
	else
	{
		const AuCommAcceptPickupShipment* accept = dynamic_cast<const AuCommAcceptPickupShipment*>(commPacket);
		if(accept)
		{
			if(*m_Shipment != *accept->Shipment())
				return false;
			return true;
		}
	}
	return false;
}

QString AuCommBidPickupShipment::Icon() const
{
	return CommPacketIcon();
}

void AuCommBidPickupShipment::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << *m_Shipment;
}

void AuCommBidPickupShipment::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> *m_Shipment;
}

// Returns the text of the comm message.
QString AuCommBidPickupShipment::Text() const
{
	return QObject::tr("%1. I offer to pickup and deliver the shipment.")
		.arg(m_Shipment->ListName());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommBidPickupShipment::Type() const
{
	return CommPacketType();
}

QString AuCommBidPickupShipment::Icon2() const
{
	QString cname;
	if(m_Shipment->CommodityId() != CID_Invalid)
		cname = g_Au.CommodityBitmapName(m_Shipment->CommodityId());
	else
		cname = CommodityFilterIcon(m_Shipment->CommodityFilter());
	return cname;
}

const AuItem* AuCommBidPickupShipment::Item() const
{
	return &m_Shipment->Container();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommAcceptPickupShipment::AuCommAcceptPickupShipment(const AuId& buyerId, const QString& buyerName, const AuShipment& shipment,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_BuyerId = buyerId;
	m_BuyerName = buyerName;
	m_Shipment = new AuShipment(shipment);
}

AuCommAcceptPickupShipment::AuCommAcceptPickupShipment()
{
	//m_BuyerId
	//m_BuyerName
	m_Shipment = new AuShipment();
}

AuCommAcceptPickupShipment::AuCommAcceptPickupShipment(const AuCommAcceptPickupShipment& c) : AuCommPacket(c)
{
	m_BuyerId = c.m_BuyerId;
	m_BuyerName = c.m_BuyerName;
	m_Shipment = new AuShipment(*c.m_Shipment);
}

AuCommAcceptPickupShipment::~AuCommAcceptPickupShipment()
{
	delete m_Shipment;
}

AuCommPacket* AuCommAcceptPickupShipment::Clone() const
{
	return new AuCommAcceptPickupShipment(*this);
}

QString AuCommAcceptPickupShipment::Icon() const
{
	return CommPacketIcon();
}

void AuCommAcceptPickupShipment::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_BuyerId;
	out << m_BuyerName;
	out << *m_Shipment;
}

void AuCommAcceptPickupShipment::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_BuyerId;
	in >> m_BuyerName;
	in >> *m_Shipment;
}

// Returns the text of the comm message.
QString AuCommAcceptPickupShipment::Text() const
{
	QString text = QObject::tr("%1 transferred to your hold.")
		.arg(m_Shipment->ListName());
	return text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommAcceptPickupShipment::Type() const
{
	return CommPacketType();
}

const AuItem* AuCommAcceptPickupShipment::Item() const
{
	return &m_Shipment->Container();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOpenTEU::AuCommOpenTEU(const AuItem& item,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Item = item.Clone();
}

AuCommOpenTEU::AuCommOpenTEU()
{
	m_Item = 0;
}

AuCommOpenTEU::AuCommOpenTEU(const AuCommOpenTEU& c) : AuCommPacket(c)
{
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommOpenTEU::~AuCommOpenTEU()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommOpenTEU::Clone() const
{
	return new AuCommOpenTEU(*this);
}

QString AuCommOpenTEU::Icon() const
{
	return CommPacketIcon();
}

void AuCommOpenTEU::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);

	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommOpenTEU::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommOpenTEU::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("Open %1.")
		.arg(m_Item->ListName());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOpenTEU::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommOfferTEU::AuCommOfferTEU(const AuItem& item,
		const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope) :
	AuCommPacket(channelName, fromId, fromName, fromSector, fromGalaxy, range, scope)
{
	m_Item = item.Clone();
}

AuCommOfferTEU::AuCommOfferTEU()
{
	m_Item = 0;
}

AuCommOfferTEU::AuCommOfferTEU(const AuCommOfferTEU& c) : AuCommPacket(c)
{
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
}

AuCommOfferTEU::~AuCommOfferTEU()
{
	if(m_Item)
		delete m_Item;
}

AuCommPacket* AuCommOfferTEU::Clone() const
{
	return new AuCommOfferTEU(*this);
}

QString AuCommOfferTEU::Icon() const
{
	return CommPacketIcon();
}

void AuCommOfferTEU::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	if(m_Item)
		delete m_Item;

	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			in >> *m_Item;
	}
	else
		m_Item = 0;
}

void AuCommOfferTEU::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

// Returns the text of the comm message.
QString AuCommOfferTEU::Text() const
{
	Q_ASSERT(m_Item);
	return QObject::tr("%1 is ready for delivery.")
		.arg(m_Item->ListName());
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommOfferTEU::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommContactShip::AuCommContactShip(quint32 senderEmpire, const AuId& senderShipId,
		const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy) :
	AuCommPacket(AuCommStdChannel::CrewChannelName(), fromId, fromName, fromSector, fromGalaxy, AuCommStdChannel::Universe,
			AuCommStdChannel::CrewMates)
{
	SetScopeData(senderShipId.GetID());
	m_SenderEmpire = senderEmpire;
}

AuCommContactShip::AuCommContactShip()
{
	m_SenderEmpire = 0;
}

AuCommContactShip::AuCommContactShip(const AuCommContactShip& c) : AuCommPacket(c)
{
	m_SenderEmpire = c.m_SenderEmpire;
}

AuCommPacket* AuCommContactShip::Clone() const
{
	return new AuCommContactShip(*this);
}

QString AuCommContactShip::Icon() const
{
	return CommPacketIcon();
}

void AuCommContactShip::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SenderEmpire;
}

void AuCommContactShip::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SenderEmpire;
}

QString AuCommContactShip::Text() const
{
	// This message is not visible to players.
	return QString();
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommContactShip::Type() const
{
	return CommPacketType();
}

quint32 AuCommContactShip::FromEmpireId() const
{
	return m_SenderEmpire;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCommContactReply::AuCommContactReply(const QString& text, const AuId& senderSystemId,
		const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		AuCommStdChannel::RANGE range) :
	AuCommPacket(AuCommStdChannel::CrewChannelName(), fromId, fromName, fromSector, fromGalaxy, range, AuCommStdChannel::CrewMates)
{
	SetScopeData(fromId.GetID());
	m_SystemId = senderSystemId;
	m_Text = text;
}

AuCommContactReply::AuCommContactReply()
{
}

AuCommContactReply::AuCommContactReply(const AuCommContactReply& c) : AuCommPacket(c)
{
	m_SystemId = c.m_SystemId;
	m_Text = c.m_Text;
}

AuCommPacket* AuCommContactReply::Clone() const
{
	return new AuCommContactReply(*this);
}

QString AuCommContactReply::Icon() const
{
	return CommPacketIcon();
}

void AuCommContactReply::operator<<(QDataStream& in)
{
	AuCommPacket::operator<<(in);
	in >> m_SystemId;
	in >> m_Text;
}

void AuCommContactReply::operator>>(QDataStream& out) const
{
	AuCommPacket::operator>>(out);
	out << m_SystemId;
	out << m_Text;
}

// Returns the text of the comm message.
QString AuCommContactReply::Text() const
{
	return m_Text;
}

// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
AuCommPacket::TYPE AuCommContactReply::Type() const
{
	return CommPacketType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
