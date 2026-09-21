//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommpacket.h,v 1.178 2022/02/19 18:18:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommPacket_H
#define AuCommPacket_H

#include "audata.h"
#include "auid.h"
#include "aumission.h"
#include "aurefitship.h"

class AuItem;
class AuItemDesignSpacecraft;
class AuShipment;

class AUDATA_DLL AuCommPacket
{
public:
	// Add nu types at the end of this enum for the least impact on the world. If any existing enum values change, the network
	// version must be changed and all servers and clients must be rebuilt and restarted.
	// To my knowledge, the comm packet type value never gets stored in any persistent storage, like a file or a database.
	enum TYPE
	{
		Accept,							// AuCommAccept
		AcceptAvatar,					// AuCommAcceptAvatar
		AcceptBarter,					// AuCommAcceptBarter
		AcceptBerthCaptain,				// AuCommAcceptBerthCaptain
		AcceptBerthCaptainSlot,			// AuCommAcceptBerthCaptainSlot
		AcceptBerthCrew,				// AuCommAcceptBerthCrew
		AcceptBerthOfficer,				// AuCommAcceptBerthOfficer
		AcceptBerthPassenger,			// AuCommAcceptBerthPassenger
		AcceptBerthTroop,				// AuCommAcceptBerthTroop
		AcceptInviteToEmpire,			// AuCommAcceptInviteToEmpire
		AcceptRepairAll,				// AuCommAcceptRepairAll
		AcceptRepairSystem,				// AuCommAcceptRepairSystem
		AcceptUpgradeInstall,			// AuCommAcceptUpgradeInstall
		Announcement,					// AuCommAnnouncement
		Auction,						// AuCommAuction
		BeamAtom,						// AuCommBeamAtom
		BeamCoordinate,					// AuCommBeamCoordinate
		BeamOver,						// AuCommBeamOver
		BeamPlanet,						// AuCommBeamPlanet
		BeamShip,						// AuCommBeamShip
		BeamTransporter,				// AuCommBeamTransporter
		Bid,							// AuCommBid
		BidBarter,						// AuCommBidBarter
		BoardCrew,						// AuCommBoardCrew
		BoardOfficers,					// AuCommBoardOfficers
		BoardPassengers,				// AuCommBoardPassengers
		BoardTroops,					// AuCommBoardTroops
		Chat,							// AuCommChat
		ChatAudio,						// AuCommChatAudio
		ChatChoice,						// AuCommChatChoice
		ChatContinued,					// AuCommChatContinued
		ChatStory,						// AuCommChatStory
		CitizenOrder,					// AuCommCitizenOrder
		CitizenPosture,					// AuCommCitizenPosture
		CityChat,						// AuCommCityChat
		CityMessage,					// AuCommCityMessage
		CitySurrenderAccepted,			// AuCommCitySurrenderAccepted
		CitySurrenderOffered,			// AuCommCitySurrenderOffered
		CrewBoarded,					// AuCommCrewBoarded
		CrewBoardingComplete,			// AuCommCrewBoardingComplete
		DamageInflicted,				// AuCommDamageInflicted
		DebarkPassengers,				// AuCommDebarkPassengers
		Detonate,						// AuCommDetonate
		DiplomaticMessage,				// AuCommDiplomaticMessage
		FiringAtTarget,					// AuCommFiringAtTarget
		ForSale,						// AuCommForSale
		HailReply,						// AuCommHailReply
		HailRequest,					// AuCommHailRequest
		Harvest,						// AuCommHarvest
		InventoryItem,					// AuCommInventoryItem
		InviteToEmpire,					// AuCommInviteToEmpire
		Jettison,						// AuCommJettison
		Message,						// AuCommMessage
		MissionNew,						// AuCommMissionNew
		MissionOrderAdd,				// AuCommMissionOrderAdd
		MissionOrderInsert,				// AuCommMissionOrderInsert
		MissionOrderNew,				// AuCommMissionOrderNew
		MissionProperty,				// AuCommMissionProperty
		Narrate,						// AuCommNarrate
		NarrateAudio,					// AuCommNarrateAudio
		NarrateChoice,					// AuCommNarrateChoice
		NarrateImage,					// AuCommNarrateImage
		NarrateTitle,					// AuCommNarrateTitle
		NarrateVideo,					// AuCommNarrateVideo DHH not implemented
		OfferAvatar,					// AuCommOfferAvatar
		OfferRepairAll,					// AuCommOfferRepairAll
		OfferRepairSystem,				// AuCommOfferRepairSystem
		OfferUpgradeInstall,			// AuCommOfferUpgradeInstall
		OfficerBoarded,					// AuCommOfficerBoarded
		PassengerBoardingComplete,		// AuCommPassengerBoardingComplete
		PassengersBoarded,				// AuCommPassengersBoarded
		PassengersDebarked,				// AuCommPassengersDebarked
		PreBoard,						// AuCommPreBoard
		RequestBerthAny,				// AuCommRequestBerthAny
		RequestBerthCaptain,			// AuCommRequestBerthCaptain
		RequestBerthCaptainSlot,		// AuCommRequestBerthCaptainSlot
		RequestBerthCrew,				// AuCommRequestBerthCrew
		RequestBerthOfficer,			// AuCommRequestBerthOfficer
		RequestBerthPassenger,			// AuCommRequestBerthPassenger
		RequestBerthTroop,				// AuCommRequestBerthTroop
		RequestExterminator,			// AuCommRequestExterminator
		RequestHelp,					// AuCommRequestHelp
		RequestInventory,				// AuCommRequestInventory
		RequestRepairAll,				// AuCommRequestRepairAll
		RequestRepairSystem,			// AuCommRequestRepairSystem
		RequestUpgradeSystem,			// AuCommRequestUpgradeSystem
		RevokeBid,						// AuCommRevokeBid
		SOSAttack,						// AuCommSOSAttack
		SOSFuelMarooned,				// AuCommSOSFuelMarooned
		SOSFuelShip,					// AuCommSOSFuelShip
		SOSFuelStation,					// AuCommSOSFuelStation
		SOSManeuverDriveAdrift,			// AuCommSOSManeuverDriveAdrift
		SOSManeuverDriveMarooned,		// AuCommSOSManeuverDriveMarooned
		SOSManeuverDriveStation,		// AuCommSOSManeuverDriveStation
		SolicitAllSales,				// AuCommSolicitAllSales
		SolicitAmmoSales,				// AuCommSolicitAmmoSales
		SolicitArmorSales,				// AuCommSolicitArmorSales
		SolicitConstructionMaterialSales,	// AuCommSolicitConstructionMaterialSales
		SolicitFoodSales,				// AuCommSolicitFoodSales
		SolicitFuelSales,				// AuCommSolicitFuelSales
		SolicitItemSales,				// AuCommSolicitItemSales
		SolicitMedSales,				// AuCommSolicitMedSales
		SolicitPartsSales,				// AuCommSolicitPartsSales
		SolicitToolSales,				// AuCommSolicitToolSales
		SolicitWeaponSales,				// AuCommSolicitWeaponSales
		SpotTargetOrTerrain,			// AuCommSpotTargetOrTerrain
		Thought,						// AuCommThought
		ThoughtMailAlert,				// AuCommThoughtMailAlert
		TroopBoardingComplete,			// AuCommTroopBoardingComplete
		TroopsBoarded,					// AuCommTroopsBoarded
		ShipStasisDimension,			// AuCommShipStasisDimension
		ShipStasisWait,					// AuCommShipStasisWait
		Silverwill,						// AuCommSilverwill
		ThoughtMessage,					// AuCommThoughtMessage
		BeamVehicleAtom,				// AuCommBeamVehicleAtom
		BeamVehicleCoordinate,			// AuCommBeamVehicleCoordinate
		BeamVehicleOver,				// AuCommBeamVehicleOver
		BeamVehiclePlanet,				// AuCommBeamVehiclePlanet
		BeamVehicleShip,				// AuCommBeamVehicleShip
		BeamVehicleTransporter,			// AuCommBeamVehicleTransporter
		BoardCryo,						// AuCommBoardCryo
		CryoBoarded,					// AuCommCryoBoarded
		CryoBoardingComplete,			// AuCommCryoBoardingComplete
		DebarkCryo,						// AuCommDebarkCryo
		CryoDebarked,					// AuCommCryoDebarked
		RequestRefit,					// AuCommRequestRefit sent by player with refit details.
		OfferRefit,						// AuCommOfferRefit reply by city with refit details and estimate.
		AcceptRefit,					// AuCommAcceptRefit sent by player to accept refit offer.
		RequestHouseKeeping,			// AuCommRequestHouseKeeping
		ChannelOpened,					// AuCommChannelOpened
		SolicitNaturalResourceSales,	// AuCommSolicitNaturalResourceSales
		InviteTeleport,					// AuCommInviteTeleport
		AcceptTeleport,					// AuCommAcceptTeleport
		SolicitPatentSales,				// AuCommSolicitPatentSales
		Combine,						// AuCommCombine
		SolicitMiscSales,				// AuCommSolicitMiscSales
		RepairPartsMissing,				// AuCommRepairPartsMissing
		ChatImage,						// AuCommChatImage
		ChoicePicked,					// AuCommChoicePicked
		ShipmentItem,					// AuCommShipmentItem
		SolicitShipments,				// AuCommSolicitShipments
		BidPickupShipment,				// AuCommBidPickupShipment
		AcceptPickupShipment,			// AuCommAcceptPickupShipment
		OpenTEU,						// AuCommOpenTEU
		OfferTEU,						// AuCommOfferTEU
		ContactShip,					// AuCommContactShip
		ContactReply,					// AuCommContactReply
		CommTypes						// LAST ENTRY EXPECTED NOT TO EXCEED A BYTE. ADD NEW TYPES BEFORE THIS ENTRY.
	};

protected:
	bool								m_Anonymous;			// true if the sender's id is to be kept anonymous
	QString								m_ChannelName;			// Name of channel on which message was sent.
	GALAXY								m_FromGalaxy;			// Galaxy containing sender atom.
	AuId								m_FromId;				// Sender.
	QString								m_FromName;				// Atom name of sender.
	AuId								m_FromSectorId;			// Sector containing sender atom.
	qint8								m_RangeRx;				// Range at which this message was received.
	qint8								m_RangeTx;				// Range at which this message was transmitted.
	AuCommStdChannel::SCOPE				m_Scope;				// Scope of this message.
	quint32								m_ScopeData;			// Scope-specific info for resolving eligible recipients.
																// Empire Id of sender in Empire|Friends|Fleet|Government scope.
																// Ship Id of sender in CrewMates scope.
																// Owner Id of in Company scope.

public:
	AuCommPacket(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommPacket();
	virtual ~AuCommPacket();
	AuCommPacket(const AuCommPacket& c);

	inline const QString& ChannelName() const
	{
		return m_ChannelName;
	}

	// Returns a duplicate of this condition, allocated on the heap.
	virtual AuCommPacket* Clone() const = 0;

	// Lots of packets have a commodity id associated with them. This base class returns Item()->CommodityId() or CID_Invalid.
	virtual COMMODITYID CommodityId() const;

	// This base class implementation returns Item()->Count() or 0.
	virtual quint32 Count() const;

	// Allocates the appropriate comm packet object on the heap based on the value returned by Type() for each derived class.
	// Useful when reading a comm packet from a data stream.
	static AuCommPacket* CreateCommPacket(TYPE commType);

	// This base class implementation returns 0.
	virtual quint32 DesignId() const;

	// Returns name of empire that manufactured the associated commodity, if any.
	// This base class returns blank.
	virtual const QString& EmpireOfOrigin() const;

	// This base class returns scope data when it is an empire id, otherwise returns 0.
	virtual quint32 FromEmpireId() const;

	inline GALAXY FromGalaxy() const
	{
		return m_FromGalaxy;
	}

	inline const AuId& FromId() const
	{
		return m_FromId;
	}

	inline const QString& FromName() const
	{
		return m_FromName;
	}

	// Only valid in Company scope.
	inline AuId FromOwnerId() const
	{
		if(Scope() == AuCommStdChannel::Company)
			return AuId(m_ScopeData);
		else
			return AuId();
	}

	inline const AuId& FromSectorId() const
	{
		return m_FromSectorId;
	}

	// Only valid in CrewMates scope.
	inline AuId FromShipId() const
	{
		if(Scope() == AuCommStdChannel::CrewMates)
			return AuId(m_ScopeData);
		else
			return AuId();
	}

	// The fromId of anonymous messages is changed to hide the sender's identity. A 0 would cause the IsValid test to fail. This is
	// done immediately before sending the message to a client so all validity testing can be performed, such as range checking.
	// Those things need to know the id of the sender all the way to the very end of the process, in DtChatChannel::ChatToClients.
	void SetFromId(const AuId& id)
	{
		Q_ASSERT(!id.IsNull());			// A null id renders the message invalid.
		m_FromId = id;
	}

	// An icon that indicates the type of the message.
	// If blank is returned, the message type does not show on the comm settings ui.
	virtual QString Icon() const = 0;

	// An optional secondary icon related to the message. This default implementation returns the commodity icon corresponding to
	// the return from CommodityId(), or nothing at all if that is CID_Invalid.
	virtual QString Icon2() const;

	// If blank is returned, the message type does not show on the comm settings ui.
	static QString Icon(TYPE commType);

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	inline bool IsNarratorMessage() const
	{
		return IsNarratorMessage(Type());
	}

	inline static bool IsNarratorMessage(TYPE type)
	{
		return type == Narrate || type == NarrateTitle || type == NarrateChoice ||
			type == NarrateAudio || type == NarrateImage || type == NarrateVideo;
	}

	// Order and Posture messages are directed at a specific list of recipient citizen ids.
	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	// Also BeamVehicle messages.
	// This base class implementation returns false.
	virtual bool IsRecipient(const AuId& /*id*/) const
	{
		return false;
	}

	// Messages missed within the recent past are sent to users when they log in, so they don't miss messages when moving between
	// servers or when briefly relogging.
	// A false return will prevent a recent message from being repeated when somebody relogs.
	virtual bool IsRepeatedAtLogin() const
	{
		return true;
	}

	inline bool IsSenderAnonymous() const
	{
		return m_Anonymous;
	}

	inline bool IsSOSMessage() const
	{
		return IsSOSMessage(Type());
	}

	inline static bool IsSOSMessage(TYPE type)
	{
		return type == SOSAttack ||
			type == SOSFuelMarooned ||
			type == SOSFuelShip ||
			type == SOSFuelStation ||
			type == SOSManeuverDriveAdrift ||
			type == SOSManeuverDriveMarooned ||
			type == SOSManeuverDriveStation;
	}

	inline bool IsThought() const
	{
		return IsThought(Type());
	}

	inline static bool IsThought(TYPE type)
	{
		return type == Thought || type == ThoughtMailAlert || type == ThoughtMessage || type == Silverwill;
	}

	virtual bool IsValid() const;

	inline bool IsWithinRange(double atDistance) const
	{
		return AuCommStdChannel::IsWithinRange(atDistance, TransmissionRange());
	}

	// Many packets have an associated item, often the primary subject of the message.
	virtual const AuItem* Item() const
	{
		return 0;
	}

	virtual quint8 MinQ() const
	{
		return 0;
	}

	// When bartering, an item is offered in exchange.
	virtual const AuItem* OfferItem() const
	{
		return 0;
	}

	virtual QString OfferEmpireOfOrigin() const
	{
		return QString();
	}

	// This base class implementation returns false.
	virtual bool OnScreenByDefault() const;

	static bool OnScreenByDefault(TYPE commType);

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// This base class implementation returns 0.
	virtual quint32 Price() const;

	inline AuCommStdChannel::RANGE ReceptionRange() const
	{
		return (AuCommStdChannel::RANGE)m_RangeRx;
	}

	// Order and Posture messages are directed at a specific list of recipient citizen ids.
	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	// Also BeamVehicle messages.
	// This base class implementation returns 0.
	virtual int RecipientCount() const
	{
		return 0;
	}

	// Order and Posture messages are directed at a specific list of recipient citizen ids.
	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	// Also BeamVehicle messages.
	// This base class implementation returns an empty list.
	virtual AuIdList Recipients() const;

	// Returns the id of the ship for which repairs or refit were accepted.
	virtual AuId RepairShipId() const
	{
		return AuId(0);
	}

	// Allocates a packet on the heap with nu and returns it. Caller is responsible for the object after that.
	static AuCommPacket* RxDecrypt(QDataStream& in, short int wKey);

	inline AuCommStdChannel::SCOPE Scope() const
	{
		return m_Scope;
	}

	inline quint32 ScopeData() const
	{
		return m_ScopeData;
	}

	inline void SetAnonymous(bool isAnonymous)
	{
		m_Anonymous = isAnonymous;
	}

	// Sets the range at which this message was actually received.
	// This is how strongly the broadcast was received.
	inline void SetReceptionRange(AuCommStdChannel::RANGE range)
	{
		m_RangeRx = range;
	}

	// Sets the range at which this message was transmitted.
	// This is the amount of power behind the broadcast.
	inline void SetTransmissionRange(AuCommStdChannel::RANGE range)
	{
		m_RangeTx = range;
	}

	inline void SetScope(AuCommStdChannel::SCOPE scope)
	{
		m_Scope = scope;
	}

	// Channel scope specific data.
	//   - Empire|Friends|Fleet|Government = Empire id of sender.
	//   - CrewMates = Ship id crewed by sender.
	//   - Company = Owner id.
	inline void SetScopeData(quint32 scopeData)
	{
		m_ScopeData = scopeData;
	}

	virtual const AuShipment* Shipment() const
	{
		return 0;
	}

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	// This base class implementation returns SPOT_Invalid.
	virtual VEHICLE_SPOT_TYPE SpotType() const;

	inline AuSysId SysId() const
	{
		return AuSysId(System(), SystemWhich());
	}

	// This base class implementation returns 0.
	virtual SYSTEM System() const;

	// This base class implementation returns "".
	virtual const QString& SystemDesc() const;

	// This base class implementation returns 0.
	virtual quint16 SystemWhich() const;

	// This base class implementation returns 0.
	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	// If blank is returned, the message is not sent to the client player.
	virtual QString Text() const = 0;

	virtual quint32 ToEmpireId() const
	{
		return 0;
	}

	// This base class returns 0.
	virtual const AuId& ToId() const;

	inline AuCommStdChannel::RANGE TransmissionRange() const
	{
		return (AuCommStdChannel::RANGE)m_RangeTx;
	}

	// Transmits encrypted message into stream.
	void TxEncrypt(QDataStream& outStream, short int wKey) const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const = 0;

	// Used for configuring and logging chat messages.
	virtual QString TypeName() const = 0;

	// Used for configuring and logging chat messages.
	static QString TypeName(TYPE type);
};

inline QDataStream& operator<<(QDataStream& out, const AuCommPacket& commPacket)
{
	commPacket >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCommPacket& commPacket)
{
	commPacket << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommPacketList : public QList<AuCommPacket*>
{
public:
	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCommPacketList& commPacketList)
{
	commPacketList >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCommPacketList& commPacketList)
{
	commPacketList << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAccept : public AuCommPacket
{
private:
	AuId								m_BuyerId;
	QString								m_BuyerName;
	QString								m_EmpireOfOrigin;
	AuItem*								m_Item;
	quint32								m_Price;

public:
	AuCommAccept(const AuId& buyerId, const QString& buyerName, const AuItem& item, const QString& empireOfOrigin, quint32 price,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAccept();
	AuCommAccept(const AuCommAccept& c);
	virtual ~AuCommAccept();

	inline const AuId& BuyerId() const
	{
		return m_BuyerId;
	}

	inline const QString& BuyerName() const
	{
		return m_BuyerName;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Accept;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Offer");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBarter : public AuCommPacket
{
private:
	AuId								m_BuyerId;
	QString								m_BuyerName;
	QString								m_EmpireOfOrigin;
	AuItem*								m_Item;

	// The price.
	AuItem*								m_OfferItem;
	QString								m_OfferEmpireOfOrigin;

public:
	AuCommAcceptBarter(const AuId& buyerId, const QString& buyerName, const AuItem& item, const QString& empireOfOrigin,
			const AuItem& offerItem, const QString& offerEmpireOfOrigin,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBarter();
	AuCommAcceptBarter(const AuCommAcceptBarter& c);
	virtual ~AuCommAcceptBarter();

	inline const AuId& BuyerId() const
	{
		return m_BuyerId;
	}

	inline const QString& BuyerName() const
	{
		return m_BuyerName;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBarter;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual QString OfferEmpireOfOrigin() const
	{
		return m_OfferEmpireOfOrigin;
	}

	virtual const AuItem* OfferItem() const
	{
		return m_OfferItem;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Trade");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommAcceptBerth derived packets.
// Not a packet type per se.

class AUDATA_DLL AuCommAcceptBerth : public AuCommPacket
{
protected:
	AuId								m_ShipId;
	bool								m_ShipIsPrivate;
	QString								m_ShipName;
	AuId								m_UnitId;
	bool								m_UnitIsNewOwner;
	QString								m_UnitName;

public:
	AuCommAcceptBerth(const AuId& unitId, const QString& unitName, bool unitIsNewOwner, const AuId& shipId, const QString& shipName,
			bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerth();
	AuCommAcceptBerth(const AuCommAcceptBerth& c);

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	inline bool ShipIsPrivate() const
	{
		return m_ShipIsPrivate;
	}

	inline const QString& ShipName() const
	{
		return m_ShipName;
	}

	inline const AuId& UnitId() const
	{
		return m_UnitId;
	}

	inline bool UnitIsNewOwner() const
	{
		return m_UnitIsNewOwner;
	}

	inline const QString& UnitName() const
	{
		return m_UnitName;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthCaptain : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthCaptain(const AuId& unitId, const QString& unitName, bool unitIsNewOwner, const AuId& shipId,
			const QString& shipName, bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthCaptain();
	AuCommAcceptBerthCaptain(const AuCommAcceptBerthCaptain& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthCaptain;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Captain.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Captain's Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthCaptainSlot : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthCaptainSlot(const AuId& unitId, const QString& unitName, bool unitIsNewOwner, const AuId& shipId,
			const QString& shipName, bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthCaptainSlot();
	AuCommAcceptBerthCaptainSlot(const AuCommAcceptBerthCaptainSlot& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthCaptainSlot;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Captain.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Captain's Position");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthCrew : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthCrew(const AuId& unitId, const QString& unitName, const AuId& shipId, const QString& shipName,
			bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthCrew();
	AuCommAcceptBerthCrew(const AuCommAcceptBerthCrew& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthCrew;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Crewman.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Crew Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthOfficer : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthOfficer(const AuId& unitId, const QString& unitName, const AuId& shipId, const QString& shipName,
			bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthOfficer();
	AuCommAcceptBerthOfficer(const AuCommAcceptBerthOfficer& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthOfficer;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Officer.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Officer Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthPassenger : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthPassenger(const AuId& unitId, const QString& unitName, const AuId& shipId, const QString& shipName,
			bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthPassenger();
	AuCommAcceptBerthPassenger(const AuCommAcceptBerthPassenger& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthPassenger;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Passenger Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptBerthTroop : public AuCommAcceptBerth
{
public:
	AuCommAcceptBerthTroop(const AuId& unitId, const QString& unitName, const AuId& shipId, const QString& shipName,
			bool isPrivate,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptBerthTroop();
	AuCommAcceptBerthTroop(const AuCommAcceptBerthTroop& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptBerthTroop;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Troop.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Troop Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptInviteToEmpire : public AuCommPacket
{
private:
	quint32								m_EmpireId;				// Empire that posted the invitation to join.
	AuId								m_ToId;					// Recipient id of message.

public:
	AuCommAcceptInviteToEmpire(const AuId& toId, quint32 empireId,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptInviteToEmpire();
	AuCommAcceptInviteToEmpire(const AuCommAcceptInviteToEmpire& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptInviteToEmpire;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/CommGovernment.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Invitation to Empire");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommAcceptRepair derived packets.
// Not a packet type per se.

class AUDATA_DLL AuCommAcceptRepair : public AuCommPacket
{
protected:
	AuId								m_BuyerId;
	QString								m_BuyerName;
	AuId								m_CityId;
	quint8								m_MinQ;
	quint32								m_Price;				// Price of repairs.
	quint16								m_RepairSecs;			// Seconds required to carry out the repairs.
	AuId								m_ShipId;				// Id of ship to be repaired.

public:
	AuCommAcceptRepair(const AuId& shipId, const AuId& buyerId, const QString& buyerName, const AuId& cityId, quint32 price,
			quint16 repairSecs, quint8 minQ, const QString& channelName, const AuId& fromId, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptRepair();
	AuCommAcceptRepair(const AuCommAcceptRepair& c);

	inline const AuId& BuyerId() const
	{
		return m_BuyerId;
	}

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual QString Icon2() const
	{
		return QString(":data/Damage.png");
	}

	virtual quint8 MinQ() const
	{
		return m_MinQ;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	inline quint16 RepairSeconds() const
	{
		return m_RepairSecs;
	}

	virtual AuId RepairShipId() const
	{
		return m_ShipId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptRepairAll : public AuCommAcceptRepair
{
private:
	QList<COMMODITYID>					m_NoQ;					// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_ShortQ;				// Commodities in short supply at min Q.
	QList<COMMODITYID>					m_NoSupply;				// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_Enough;				// Commodities in adequate supply at lower Q.

public:
	AuCommAcceptRepairAll(const AuId& shipId, const AuId& buyerId, const QString& buyerName, const AuId& cityId, quint32 price,
			quint16 repairSecs, const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply,
			const QList<COMMODITYID>& enough, quint8 minQ,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptRepairAll();
	AuCommAcceptRepairAll(const AuCommAcceptRepairAll& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptRepairAll;
	}

	virtual QString Icon() const;

	inline const QList<COMMODITYID>& PartsUnavailableAtMinQ() const
	{
		return m_NoQ;
	}

	inline const QList<COMMODITYID>& PartsShortAtMinQ() const
	{
		return m_ShortQ;
	}

	inline const QList<COMMODITYID>& PartsNone() const
	{
		return m_NoSupply;
	}

	inline const QList<COMMODITYID>& PartsEnough() const
	{
		return m_Enough;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Repair All Systems");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptRepairSystem : public AuCommAcceptRepair
{
private:
	QList<COMMODITYID>					m_NoQ;					// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_ShortQ;				// Commodities in short supply at min Q.
	QList<COMMODITYID>					m_NoSupply;				// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_Enough;				// Commodities in adequate supply at lower Q.
	quint8								m_System;				// SYSTEM
	QString								m_SystemDesc;			// System description.
	quint16								m_Which;				// System 'which' identifier.

public:
	AuCommAcceptRepairSystem(const AuId& shipId, const AuId& buyerId, const QString& buyerName, const AuId& cityId, quint8 system,
			quint16 which, const QString& systemDesc, quint32 price, quint16 repairSecs, const QList<COMMODITYID>& noQ,
			const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply, const QList<COMMODITYID>& enough, quint8 minQ,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptRepairSystem();
	AuCommAcceptRepairSystem(const AuCommAcceptRepairSystem& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptRepairSystem;
	}

	virtual QString Icon() const;

	inline const QList<COMMODITYID>& PartsUnavailableAtMinQ() const
	{
		return m_NoQ;
	}

	inline const QList<COMMODITYID>& PartsShortAtMinQ() const
	{
		return m_ShortQ;
	}

	inline const QList<COMMODITYID>& PartsNone() const
	{
		return m_NoSupply;
	}

	inline const QList<COMMODITYID>& PartsEnough() const
	{
		return m_Enough;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual SYSTEM System() const
	{
		return (SYSTEM)m_System;
	}

	virtual const QString& SystemDesc() const
	{
		return m_SystemDesc;
	}

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Repair Specific System");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptUpgradeInstall : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;
	quint16								m_InstallSecs;			// Seconds required to install the module.
	AuItem*								m_Item;
	quint32								m_Price;
	quint16								m_Which;				// System 'which' identifier.

public:
	AuCommAcceptUpgradeInstall(const AuItem& item, const QString& empireOfOrigin, quint16 systemWhich, quint16 installSecs,
			quint32 price,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptUpgradeInstall();
	AuCommAcceptUpgradeInstall(const AuCommAcceptUpgradeInstall& c);
	virtual ~AuCommAcceptUpgradeInstall();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptUpgradeInstall;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/ShipUpgrade.png");
	}

	inline quint16 InstallSeconds() const
	{
		return m_InstallSecs;
	}

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	virtual SYSTEM System() const;

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept System Module Installation");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAuction : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;
	AuItem*								m_Item;

public:
	AuCommAuction(const AuItem& item, const QString& empireOfOrigin,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAuction();
	AuCommAuction(const AuCommAuction& c);
	virtual ~AuCommAuction();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/OrderGetItem.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Auction;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Auction Item");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamOver : public AuCommPacket
{
private:
	quint8								m_Count;

public:
	AuCommBeamOver(quint8 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamOver();
	AuCommBeamOver(const AuCommBeamOver& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamOver;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Over");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Used to beam to all sorts of atoms, including the world atom itself. Sometimes there are multiple possible targets at a
// destination atom, i.e. world claim flag and world active henge.

class AUDATA_DLL AuCommBeamAtom : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_CityId;				// The destination atom.
	QString								m_CityName;
	quint8								m_Count;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	quint8								m_Target;				// TRANSPORTER_TARGET enum

public:
	AuCommBeamAtom(const AuId& crewman, quint8 count, quint8 dir, const AuId& cityId, const QString& cityName,
			TRANSPORTER_TARGET target, const QString& channelName, const AuId& fromId, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamAtom();
	AuCommBeamAtom(const AuCommBeamAtom& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamAtom;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	inline TRANSPORTER_TARGET Target() const
	{
		return (TRANSPORTER_TARGET)m_Target;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam to City");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamCoordinate : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	quint8								m_Count;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	float								m_X;					// X offset, in meters.
	float								m_Y;					// Y offset, in meters.
	float								m_Z;					// Z offset, in meters.

public:
	AuCommBeamCoordinate(const AuId& crewman, quint8 count, quint8 dir, float x, float y, float z,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamCoordinate();
	AuCommBeamCoordinate(const AuCommBeamCoordinate& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamCoordinate;
	}

	// Returns the offset coordinate in feet.
	inline AuPoint Coordinate() const
	{
		return AuPoint(m_X, m_Y, m_Z) * FeetPerMeter();
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam to Coordinate");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamPlanet : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	quint8								m_Count;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	float								m_Latitude;				// Latitude angle in radians.
	float								m_Longitude;			// Longitude angle in radians.

public:
	AuCommBeamPlanet(const AuId& crewman, quint8 count, quint8 dir, float longitude, float latitude,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamPlanet();
	AuCommBeamPlanet(const AuCommBeamPlanet& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamPlanet;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	inline float Latitude() const
	{
		return m_Latitude;
	}

	inline float Longitude() const
	{
		return m_Longitude;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam to Planet");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamShip : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	quint8								m_Count;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	AuId								m_ShipId;
	QString								m_ShipName;

public:
	AuCommBeamShip(const AuId& crewman, quint8 count, quint8 dir, const AuId& shipId, const QString& shipName,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamShip();
	AuCommBeamShip(const AuCommBeamShip& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamShip;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam to Spacecraft");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamTransporter : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	quint8								m_Count;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	AuId								m_ShipId;
	QString								m_ShipName;
	quint16								m_WhichTransporter;

public:
	AuCommBeamTransporter(const AuId& crewman, quint8 count, quint8 dir, const AuId& shipId, const QString& shipName,
			quint16 whichTransporter,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamTransporter();
	AuCommBeamTransporter(const AuCommBeamTransporter& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamTransporter;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	virtual quint16 SystemWhich() const
	{
		return m_WhichTransporter;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam to Transporter");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehicleOver : public AuCommPacket
{
private:
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE

public:
	AuCommBeamVehicleOver(VEHICLE_SPOT_TYPE spotType,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehicleOver();
	AuCommBeamVehicleOver(const AuCommBeamVehicleOver& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehicleOver;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Vehicle Beam Over");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehicleAtom : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_CityId;
	QString								m_CityName;
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE
	quint8								m_Target;				// TRANSPORTER_TARGET enum

public:
	AuCommBeamVehicleAtom(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, const AuId& cityId, const QString& cityName,
			TRANSPORTER_TARGET target, const QString& channelName, const AuId& fromId, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehicleAtom();
	AuCommBeamVehicleAtom(const AuCommBeamVehicleAtom& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehicleAtom;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	// Beam messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Beam messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// Beam messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	inline TRANSPORTER_TARGET Target() const
	{
		return (TRANSPORTER_TARGET)m_Target;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Vehicle to City");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehicleCoordinate : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	float								m_X;					// X offset, in meters.
	float								m_Y;					// Y offset, in meters.
	float								m_Z;					// Z offset, in meters.
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE

public:
	AuCommBeamVehicleCoordinate(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, float x, float y, float z,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehicleCoordinate();
	AuCommBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehicleCoordinate;
	}

	// Returns the offset coordinate in feet.
	inline AuPoint Coordinate() const
	{
		return AuPoint(m_X, m_Y, m_Z) * FeetPerMeter();
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Vehicle to Coordinate");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehiclePlanet : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	float								m_Latitude;				// Latitude angle in radians.
	float								m_Longitude;			// Longitude angle in radians.
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE

public:
	AuCommBeamVehiclePlanet(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, float longitude, float latitude,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehiclePlanet();
	AuCommBeamVehiclePlanet(const AuCommBeamVehiclePlanet& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehiclePlanet;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	inline float Latitude() const
	{
		return m_Latitude;
	}

	inline float Longitude() const
	{
		return m_Longitude;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Vehicle to Planet");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehicleShip : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	AuId								m_ShipId;
	QString								m_ShipName;
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE

public:
	AuCommBeamVehicleShip(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, const AuId& shipId, const QString& shipName,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehicleShip();
	AuCommBeamVehicleShip(const AuCommBeamVehicleShip& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehicleShip;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Vehicle to Spacecraft");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBeamVehicleTransporter : public AuCommPacket
{
private:
	quint8								m_BeamDir;				// 0=down, 1=up
	AuId								m_Crewman;				// Crewman manning the transporter console to receive this message.
	AuId								m_ShipId;
	QString								m_ShipName;
	quint8								m_SpotType;				// VEHICLE_SPOT_TYPE
	quint16								m_WhichTransporter;

public:
	AuCommBeamVehicleTransporter(const AuId& crewman, VEHICLE_SPOT_TYPE spotType, quint8 dir, const AuId& shipId,
			const QString& shipName, quint16 whichTransporter,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBeamVehicleTransporter();
	AuCommBeamVehicleTransporter(const AuCommBeamVehicleTransporter& c);

	// Returns 0=beam down, 1=beam up
	inline int BeamDirection() const
	{
		return m_BeamDir;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgTransport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BeamVehicleTransporter;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual quint32 Count() const
	{
		return 1;
	}

	virtual QString Icon() const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Crewman == id;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual int RecipientCount() const
	{
		return 1;
	}

	// BeamAtom, BeamCoordinate, BeamPlanet, BeamShip, and BeamTransporter messages are directed to the transporter operator.
	virtual AuIdList Recipients() const;

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the AuScPart::PART_TYPE for parking the vehicle that is the subject of a BeamVehicle message.
	virtual VEHICLE_SPOT_TYPE SpotType() const
	{
		return (VEHICLE_SPOT_TYPE)m_SpotType;
	}

	virtual quint16 SystemWhich() const
	{
		return m_WhichTransporter;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Beam Vehicle to Transporter");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBid : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;
	AuItem*								m_Item;
	quint32								m_Price;				// Total price offered for count units of cid.

public:
	AuCommBid(const AuItem& item, const QString& empireOfOrigin, quint32 price,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBid();
	AuCommBid(const AuCommBid& c);
	virtual ~AuCommBid();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgBuyOffer.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Bid;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Bid On Item");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBidBarter : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;
	AuItem*								m_Item;

	// The price.
	QString								m_OfferEmpireOfOrigin;
	AuItem*								m_OfferItem;

public:
	AuCommBidBarter(const AuItem& item, const QString& empireOfOrigin, const AuItem& offerItem, const QString& offerEmpireOfOrigin,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBidBarter();
	AuCommBidBarter(const AuCommBidBarter& c);
	virtual ~AuCommBidBarter();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommTrade.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BidBarter;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual QString OfferEmpireOfOrigin() const
	{
		return m_OfferEmpireOfOrigin;
	}

	virtual const AuItem* OfferItem() const
	{
		return m_OfferItem;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Barter For Item");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBoardCrew : public AuCommPacket
{
private:
	quint8								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommBoardCrew(const AuId& loaderId, quint8 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBoardCrew();
	AuCommBoardCrew(const AuCommBoardCrew& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BoardCrew;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Crewman.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Crew Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBoardOfficers : public AuCommPacket
{
private:
	quint8								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommBoardOfficers(const AuId& loaderId, quint8 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBoardOfficers();
	AuCommBoardOfficers(const AuCommBoardOfficers& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BoardOfficers;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Officer.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Officer Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBoardPassengers : public AuCommPacket
{
private:
	quint16								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommBoardPassengers(const AuId& loaderId, quint16 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBoardPassengers();
	AuCommBoardPassengers(const AuCommBoardPassengers& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BoardPassengers;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBoardTroops : public AuCommPacket
{
private:
	quint16								m_Count;				// Count of troops wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommBoardTroops(const AuId& loaderId, quint16 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBoardTroops();
	AuCommBoardTroops(const AuCommBoardTroops& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BoardTroops;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Troop.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Troop Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChat : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this chat message.

public:
	AuCommChat(const QString& text,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChat();
	AuCommChat(const AuCommChat& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgChat.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Chat;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const;

	virtual bool IsValid() const;

	inline static int MaxChatText()
	{
		return 256;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChatAudio : public AuCommPacket
{
private:
	QByteArray							m_SoundData;			// Sound data.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommChatAudio(const QString& text, quint32 storyInstance, const QByteArray& soundData,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChatAudio();
	AuCommChatAudio(const AuCommChatAudio& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgChatAudio.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChatAudio;
	}

	virtual QString Icon() const;

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const QByteArray& SoundData() const
	{
		return m_SoundData;
	}

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat Audio");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChatImage : public AuCommPacket
{
private:
	QImage								m_Image;				// Hardware independent image data.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.
	QString								m_Title;

public:
	AuCommChatImage(const QString& title, const QString& text, quint32 storyInstance, const QImage& image,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChatImage();
	AuCommChatImage(const AuCommChatImage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateImage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChatImage;
	}

	virtual QString Icon() const;

	inline const QImage& Image() const
	{
		return m_Image;
	}

	inline const QString& ImageTitle() const
	{
		return m_Title;
	}

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat Picture");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChatChoice : public AuCommPacket
{
private:
	qint8								m_ChoiceId;				// Id of the choice, among multiple choices.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.
	AuId								m_ToId;					// Choices are always directed to a specific recipient.

public:
	AuCommChatChoice(const AuId& toId, const QString& text, quint32 storyInstance, qint8 choiceId,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChatChoice();
	AuCommChatChoice(const AuCommChatChoice& c);

	inline qint8 ChoiceId() const
	{
		return m_ChoiceId;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateChoice.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChatChoice;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return false;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat Choice");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChatContinued : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this chat message.

public:
	AuCommChatContinued(const QString& text,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChatContinued();
	AuCommChatContinued(const AuCommChatContinued& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgChat.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChatContinued;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const;

	inline static int MaxChatText()
	{
		return 8192;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat Continuation");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChatStory : public AuCommPacket
{
private:
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommChatStory(const QString& text, quint32 storyInstance,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChatStory();
	AuCommChatStory(const AuCommChatStory& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgChat.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChatStory;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat From Story");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCitizenOrder : public AuCommPacket
{
private:
	CITIZEN_ORDER						m_Order;				// Order issued.
	AuId								m_OrderTarget;			// Target atom to which the order refers.
	QString								m_OrderText;			// Room name for ORDER_GoToRoom.
	double								m_OrderX;				// X location of order.
	double								m_OrderY;				// Y location of order.
	double								m_OrderZ;				// Z location of order.
	AuIdList							m_Recipients;			// Recipients of the order.

public:
	AuCommCitizenOrder(const AuIdList& orderTo, CITIZEN_ORDER order, const AuId& orderTarget, double orderX, double orderY,
			double orderZ, const QString& orderText,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCitizenOrder();
	AuCommCitizenOrder(const AuCommCitizenOrder& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommRecruit.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CitizenOrder;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(OrderIcon(m_Order));
	}

	// Orders are directed at a specific list of recipient citizen ids.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Recipients.contains(id);
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline CITIZEN_ORDER Order() const
	{
		return m_Order;
	}

	inline const AuId& OrderTarget() const
	{
		return m_OrderTarget;
	}

	inline const QString& OrderText() const
	{
		return m_OrderText;
	}

	inline double OrderX() const
	{
		return m_OrderX;
	}

	inline double OrderY() const
	{
		return m_OrderY;
	}

	inline double OrderZ() const
	{
		return m_OrderZ;
	}

	// Orders are directed at a specific list of recipient citizen ids.
	virtual int RecipientCount() const
	{
		return m_Recipients.count();
	}

	// Orders are directed at a specific list of recipient citizen ids.
	virtual AuIdList Recipients() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Unit Order");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCitizenPosture : public AuCommPacket
{
private:
	CITIZEN_POSTURE						m_Posture;				// Posture issued.
	AuIdList							m_Recipients;			// Recipients of the order.

public:
	AuCommCitizenPosture(const AuIdList& orderTo, CITIZEN_POSTURE posture,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCitizenPosture();
	AuCommCitizenPosture(const AuCommCitizenPosture& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommRecruit.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CitizenPosture;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(PostureIcon(m_Posture));
	}

	// Posture changes are directed at a specific list of recipient citizen ids.
	virtual bool IsRecipient(const AuId& id) const
	{
		return m_Recipients.contains(id);
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline CITIZEN_POSTURE Posture() const
	{
		return m_Posture;
	}

	// Posture changes are directed at a specific list of recipient citizen ids.
	virtual int RecipientCount() const
	{
		return m_Recipients.count();
	}

	// Posture changes are directed at a specific list of recipient citizen ids.
	virtual AuIdList Recipients() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Unit Posture");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCityChat : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this chat message.

public:
	AuCommCityChat(const QString& text,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCityChat();
	AuCommCityChat(const AuCommCityChat& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/City.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CityChat;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("City Chat");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Canned statement that looks like a regular chat message.

class AUDATA_DLL AuCommMessage : public AuCommPacket
{
public:
	enum MESSAGE
	{
		ShipSummoned,
		CannotAcceptBerthType,
		CaptBerthOccupied,
		CaptBerthRestricted,
		CaptBerthRestrictedStation,
		NoCrewBerth,
		NoOfficerBerth,
		NoPassengerBerth,
		NoTroopBerth,
		OfficerBerthRestrictedEmpire,
		RequestorAlreadyHasBerthType,
		RequestorHasNoEmpire,
		MessageCount					// LAST ITEM
	};

private:
	MESSAGE								m_Message;				// Message to be made by this message.

public:
	AuCommMessage(MESSAGE message,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMessage();
	AuCommMessage(const AuCommMessage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgChat.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Message;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Chat Message");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Canned statement from a city.

class AUDATA_DLL AuCommCityMessage : public AuCommPacket
{
public:
	enum MESSAGE
	{
		UpgradesNeedSpacecraft,
		UpgradeProhibited,
		MustBeOfficerToUpgrade,
		UpgradesNeedStation,
		UpgradesNeedCity,
		NoUpgradeMoving,
		NoUpgradeSensors,
		NoUpgradeShields,
		NoUpgradeFTL,
		NoUpgradeManeuver,
		NoUpgradePowerPlant,
		MoveToBuildingToSell,
		SellAtRetail,
		SellAtCantina,
		SellAtGrocery,
		SellAtDesignStudio,
		SellAtBroker,
		SellElsewhere,
		WeNoWantThose,
		MoveToDesignStudioToSell,
		AwaitingLibraryVerification,
		DesignAlreadyInLibrary,
		DesignNotArchitect,
		MustBeOfficerToBuy,
		MoveTransporterStationToBuy,
		MoveTransporterToBuy,
		MoveStationToBuy,
		MoveToBuy,
		MoveToBuyPersonal,
		BuyAtRetail,
		BuyAtCantina,
		BuyAtGrocery,
		BuyAtDesignStudio,
		BuyAtBroker,
		BuyElsewhere,
		NoRoomToAddThem,
		NoRoomToAddIt,
		MoveToDesignStudioToBuy,
		ExitShipToBuyDesigns,
		OfferTooLow,
		NoRoomToAddDesign,
		CitizensRestricted,
		MustBeOfficerToLoadCrew,
		NoCrewInStudio,
		PopulationTooLowToLeave,
		NobodyToLeave,
		MoveTransporterStationToBoardCrew,
		MoveTransporterToBoardCrew,
		MoveStationToBoardCrew,
		MoveToBoardCrew,
		MustBeOfficerToLoadOfficers,
		OfficerAlreadyDeployed,
		WorldCannotProduceOfficers,
		NoAirportNoOfficer,
		NoUniversityNoOfficer,
		NoFactoryNoOfficer,
		OfficersRestricted,
		NoChainNoOfficer,
		NoOfficersInStudio,
		MoveTransporterStationToBoardOfficer,
		MoveTransporterToBoardOfficer,
		MoveStationToBoardOfficer,
		MoveToBoardOfficer,
		MustBeOfficerToLoadPassengers,
		NoPassengersInStudio,
		MoveTransporterStationToBoardPassengers,
		MoveTransporterToBoardPassengers,
		MoveStationToBoardPassengers,
		MoveToBoardPassengers,
		CitizensRestrictedVehicle,
		MustBeDriverToLoadPassengers,
		MoveToLoadPassengersVehicle,
		NoTroopsToDeploy,
		MustBeOfficerToLoadTroops,
		NoTroopsInStudio,
		MoveTransporterStationToBoardTroops,
		MoveTransporterToBoardTroops,
		MoveStationToBoardTroops,
		MoveToBoardTroops,
		MustBeOfficerToUnloadPassengers,
		MoveTransporterStationToDebarkPassengers,
		MoveTransporterToDebarkPassengers,
		MoveStationToDebarkPassengers,
		MoveToDebarkPassengers,
		MustBeDriverToUnloadPassengers,
		MoveToUnloadPassengersVehicle,
		PriceTooHigh,
		ChannelOpenedCity,
		ExterminateAboardShip,
		MustBeOfficerToExterminate,
		RatNestRemoved,
		RatsExterminated,
		RatNestExterminated,
		NoRepairShop,
		NoChainNoInventory,
		ReportOnPrivateChannel,
		RepairAboardShip,
		RepairProhibited,
		MustBeOfficerToRepair,
		NoShipments,
		ShipNotDamaged,
		ShipmentsOnPrivateChannel,
		UpgradeAboardShip,
		ItemsOnPrivateChannel,
		NothingToSell,
		UpgradesOnPrivateChannel,
		NoModulesForSystem,
		NoBlankDisks,
		DesignsOnPrivateChannel,
		ArchitectCommission,
		DesignsForSale,
		BuyerSellerSameSystem,
		BidNotActive,
		MoveTransporterStationToSell,
		MoveTransporterToSell,
		MoveStationToSell,
		MoveToSell,
		TooFarToTransactPersonal,
		TooFarToTransactCargo,
		BadLocToTransact,
		TooFarToTransactItem,
		BuyerInventoryFull,
		ShipNoFuelToSell,
		ShipNoCargoToSell,
		PersonalNoItemToSell,
		BuyerSellerSameShipTransferDesigns,
		TooFarToTransactDesign,
		DesignRequested,
		PublishCongrats,
		NoDesignDisk,
		CitySameSystem,
		AcceptRepairAboardShip,
		RepairOfferExpired,
		MoveStationToRepair,
		MoveToRepair,
		NoRepairMoving,
		NoRepairSensors,
		NoRepairShields,
		NoRepairFTL,
		NoRepairManeuver,
		NoRepairPowerPlant,
		PassengersSameSystem,
		PassengerBoardingNotActive,
		TooFarToTransactPassengers,
		PassengerDebarkingNotActive,
		TroopsSameSystem,
		TroopBoardingNotActive,
		TooFarToTransactTroops,
		NobodyCanAffordToLeave,
		WeNoAffordThose,
		BuildingPublishCongrats,
		BuildingNoBlankDisks,
		BuildingDesignsOnPrivateChannel,
		BuildingDesignsForSale,
		BuildingArchitectCommission,
		OfficerCityOccupied,
		DiplomaticMessageSent,
		DiplomaticMessageWait,
		DiplomaticMessageIgnored,
		UNUSED,
		BuildingUnderConstruction,
		DiplomaticMessageIgnoredByPirate,
		OfficerCityAttacked,
		MoveToDesignStudioOrArenaToSell,
		MoveToDesignStudioOrArenaToBuy,
		UnwantedStoryItem,
		NoRepairShipGettingRepairs,
		MustBeOfficerToLoadCryo,
		NoCryoInStudio,
		MoveTransporterStationToBoardCryo,
		MoveTransporterToBoardCryo,
		MoveStationToBoardCryo,
		MoveToBoardCryo,
		MustBeOfficerToUnloadCryo,
		MoveTransporterStationToDebarkCryo,
		MoveTransporterToDebarkCryo,
		MoveStationToDebarkCryo,
		MoveToDebarkCryo,
		CryoSameSystem,
		CryoBoardingNotActive,
		CryoDebarkingNotActive,
		TooFarToTransactCryo,
		RefitAboardShip,
		MustBeOfficerToRefit,
		CannotRefit,
		InvalidRefit,
		RefitNoChange,
		NoRepairShopOrAirport,
		HouseKeepingAboardShip,
		MustBeOfficerToHouseKeeping,
		BuildingQTooLowToInstall,
		NoRefitShipGettingRepairs,
		NoHomesForTravellers,
		WeNoWantAllOfThose,
		DiplomaticMessageIgnoredSameEmpire,
		NoHomesForRecruits,
		MoveBdToSell,
		MoveBdStationToSell,
		MoveBdTransporterToSell,
		MoveBdTransporterStationToSell,
		MoveBdToRepair,
		MoveBdStationToRepair,
		MoveBdToBuy,
		MoveBdStationToBuy,
		MoveBdTransporterToBuy,
		MoveBdTransporterStationToBuy,
		MoveBdToBoardCrew,
		MoveBdStationToBoardCrew,
		MoveBdTransporterToBoardCrew,
		MoveBdTransporterStationToBoardCrew,
		MoveBdToBoardCryo,
		MoveBdStationToBoardCryo,
		MoveBdTransporterToBoardCryo,
		MoveBdTransporterStationToBoardCryo,
		MoveBdToDebarkCryo,
		MoveBdStationToDebarkCryo,
		MoveBdTransporterToDebarkCryo,
		MoveBdTransporterStationToDebarkCryo,
		MoveBdToBoardOfficer,
		MoveBdStationToBoardOfficer,
		MoveBdTransporterToBoardOfficer,
		MoveBdTransporterStationToBoardOfficer,
		MoveBdToBoardPassengers,
		MoveBdStationToBoardPassengers,
		MoveBdTransporterToBoardPassengers,
		MoveBdTransporterStationToBoardPassengers,
		MoveBdToDebarkPassengers,
		MoveBdStationToDebarkPassengers,
		MoveBdTransporterToDebarkPassengers,
		MoveBdTransporterStationToDebarkPassengers,
		MoveBdToBoardTroops,
		MoveBdStationToBoardTroops,
		MoveBdTransporterToBoardTroops,
		MoveBdTransporterStationToBoardTroops,
		NoCrewToLeave,
		NoCryoToLeave,
		NoPassengersToLeave,
		PersonnelRestricted,
		NoBaseCryoToLeave,
		GreetSalesman,
		MoveBdToBuyPower,
		MoveBdToSellPower,
		UninsuredTEU,
		UnwantedTEU,
		DeliveryDefault,
		MustBeOfficerToDeliverTEU,
		MoveBdToDeliverTEU,
		MoveBdStationToDeliverTEU,
		MoveBdTransporterToDeliverTEU,
		MoveBdTransporterStationToDeliverTEU,
		RejectBadShipper,
		NoRepairShopWorkers,
		NotEnoughModulesForSystem,
		NoMetalForHull,
		NotEnoughMetalForHull,
		MessageCount					// LAST ITEM
	};

private:
	MESSAGE								m_Message;				// Message to be made by this message.

public:
	AuCommCityMessage(MESSAGE message,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCityMessage();
	AuCommCityMessage(const AuCommCityMessage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/City.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CityMessage;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("City Message");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCrewBoarded : public AuCommPacket
{
private:
	bool								m_BeamedUp;				// True if they beamed aboard.
	quint8								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommCrewBoarded(const AuId& loaderId, quint8 count, bool beamedUp,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCrewBoarded();
	AuCommCrewBoarded(const AuCommCrewBoarded& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Crewman.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CrewBoarded;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedUp)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgLoadColonists.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Crew Boarding Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCrewBoardingComplete : public AuCommPacket
{
public:
	AuCommCrewBoardingComplete(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCrewBoardingComplete();
	AuCommCrewBoardingComplete(const AuCommCrewBoardingComplete& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgRevoke.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CrewBoardingComplete;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Crewman.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Crew Boarding Complete");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommPassengerBoardingComplete : public AuCommPacket
{
public:
	AuCommPassengerBoardingComplete(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommPassengerBoardingComplete();
	AuCommPassengerBoardingComplete(const AuCommPassengerBoardingComplete& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgRevoke.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::PassengerBoardingComplete;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger Boarding Complete");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBoardCryo : public AuCommPacket
{
private:
	quint32								m_Count;				// Count of cryo passengers wanted.
	AuId								m_LoaderId;				// Id of ship that will load the crew. Not the same as sender id.

public:
	AuCommBoardCryo(const AuId& loaderId, quint32 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBoardCryo();
	AuCommBoardCryo(const AuCommBoardCryo& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BoardCryo;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/CryoPassenger.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Cryo Traveller Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCryoBoarded : public AuCommPacket
{
private:
	bool								m_BeamedUp;				// True if they beamed aboard.
	quint32								m_Count;				// Count of cryo passengers wanted.
	AuId								m_LoaderId;				// Id of ship that will load the cryo. Not the same as sender id.
	QStringList							m_Species;

public:
	AuCommCryoBoarded(const AuId& loaderId, quint32 count, bool beamedUp, const QStringList& species,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCryoBoarded();
	AuCommCryoBoarded(const AuCommCryoBoarded& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CryoPassenger.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CryoBoarded;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedUp)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgLoadColonists.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Cryo Traveller Boarding Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCryoBoardingComplete : public AuCommPacket
{
public:
	AuCommCryoBoardingComplete(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCryoBoardingComplete();
	AuCommCryoBoardingComplete(const AuCommCryoBoardingComplete& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgRevoke.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CryoBoardingComplete;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/CryoPassenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Cryo Traveller Boarding Complete");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommDamageInflicted : public AuCommPacket
{
private:
	quint32								m_Armor1;				// Amount of effect1 absorbed by armor.
	quint32								m_Armor2;				// Amount of effect2 absorbed by armor.
	quint8								m_CriticalHits;
	quint32								m_Damage1;
	quint32								m_Damage2;
	WEAPONEFFECT						m_Effect1;
	WEAPONEFFECT						m_Effect2;
	bool								m_IsDestroyed;
	QString								m_VictimName;

public:
	AuCommDamageInflicted(const QString& victimName, quint8 criticalHits, bool isDestroyed,
			WEAPONEFFECT effect1, quint32 damage1, quint32 armor1,
			WEAPONEFFECT effect2, quint32 damage2, quint32 armor2,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommDamageInflicted();
	AuCommDamageInflicted(const AuCommDamageInflicted& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/TabClose.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::DamageInflicted;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Damage Inflicted");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommDebarkCryo : public AuCommPacket
{
private:
	quint32								m_Count;				// Count of cryo travellers debarked.
	AuId								m_UnloaderId;			// Id of ship that unloaded them. Not the same as sender id.

public:
	AuCommDebarkCryo(const AuId& unloaderId, quint32 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommDebarkCryo();
	AuCommDebarkCryo(const AuCommDebarkCryo& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgUnloadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::DebarkCryo;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/CryoPassenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Cryo Traveller Debarking Announcement");
	}

	inline const AuId& UnloaderId() const
	{
		return m_UnloaderId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommDebarkPassengers : public AuCommPacket
{
private:
	quint16								m_Count;				// Count of passengers debarked.
	AuId								m_UnloaderId;			// Id of ship that unloaded them. Not the same as sender id.

public:
	AuCommDebarkPassengers(const AuId& unloaderId, quint16 count,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommDebarkPassengers();
	AuCommDebarkPassengers(const AuCommDebarkPassengers& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgUnloadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::DebarkPassengers;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger Debarking Announcement");
	}

	inline const AuId& UnloaderId() const
	{
		return m_UnloaderId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommDetonate : public AuCommPacket
{
public:
	AuCommDetonate(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommDetonate();
	AuCommDetonate(const AuCommDetonate& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommDetonate.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Detonate;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Detonate Munition Signal");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommFiringAtTarget : public AuCommPacket
{
private:
	QString								m_Location;
	QString								m_Sector;
	QString								m_Target;
	quint8								m_WhatFired;			// 0=station, 1=system station, 2=spacecraft

public:
	AuCommFiringAtTarget(quint8 whatFired, const QString& target, const QString& location, const QString& sector,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommFiringAtTarget();
	AuCommFiringAtTarget(const AuCommFiringAtTarget& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/clEShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::FiringAtTarget;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_WhatFired <= 2)
			return QString(":data/CommStation.png");
		return QString(":data/c_Spacecraft.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Firing At Target");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommForSale : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;		// Empire that made the commodity. May be blank.
	AuItem*								m_Item;
	quint32								m_Price;				// Price offered per unit.

public:
	AuCommForSale(const AuItem& item, const QString& empireOfOrigin, quint32 price,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommForSale();
	AuCommForSale(const AuCommForSale& c);
	virtual ~AuCommForSale();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/OrderGetItem.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ForSale;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Item For Sale");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommHailReply : public AuCommPacket
{
private:
	AuId								m_SystemId;				// Solar system of sender.
	QString								m_Text;					// Text of the reply.

public:
	AuCommHailReply(const QString& text, const AuId& senderSystemId,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommHailReply();
	AuCommHailReply(const AuCommHailReply& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommAnnounce.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::HailReply;
	}

	const AuId& FromSolarSystemId() const
	{
		return m_SystemId;
	}

	virtual QString Icon() const;

	/*
	virtual QString Icon2() const
	{
		return QString(":data/Officer.png");
	}
	*/

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Hail Reply");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommHailRequest : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.
	QString								m_Text;					// Additional text, after canned hail.

public:
	AuCommHailRequest(quint32 senderEmpire, const QString& addText,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommHailRequest();
	AuCommHailRequest(const AuCommHailRequest& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommHail.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::HailRequest;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Hail Request");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommHarvest : public AuCommPacket
{
private:
	COMMODITYID							m_CIDModule;			// Commodity id of the bay module used to harvest
	AuItem*								m_Item;					// Commodity harvested.

public:
	AuCommHarvest(COMMODITYID cidModule, const AuItem& item,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommHarvest();
	AuCommHarvest(const AuCommHarvest& c);
	virtual ~AuCommHarvest();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Harvest.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Harvest;
	}

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Harvest Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommInventoryItem : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;		// Empire that manufactured the item.
	AuItem*								m_Item;

public:
	AuCommInventoryItem(const AuItem& item, const QString& empireOfOrigin,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommInventoryItem();
	AuCommInventoryItem(const AuCommInventoryItem& c);
	virtual ~AuCommInventoryItem();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommManifest.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::InventoryItem;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Item Listing");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommInviteToEmpire : public AuCommPacket
{
private:
	quint32								m_EmpireId;				// Empire that posted the invitation to join.
	AuId								m_ToId;					// Recipient id of message.
	QString								m_ToName;

public:
	AuCommInviteToEmpire(const AuId& toId, const QString& toName, quint32 empireId,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommInviteToEmpire();
	AuCommInviteToEmpire(const AuCommInviteToEmpire& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommGovernment.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::InviteToEmpire;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Invitation to An Empire");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommJettison : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;		// Empire that made the item. May be blank.
	AuItem*								m_Item;					// The item to be jettisoned.

public:
	AuCommJettison(const AuItem& item, const QString& empireOfOrigin,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommJettison();
	AuCommJettison(const AuCommJettison& c);
	virtual ~AuCommJettison();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommJettison.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Jettison;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Jettison Cargo");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAnnouncement : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this announcement.

public:
	AuCommAnnouncement(const QString& text, const QString& fromName = QObject::tr("Hazeron"));
	AuCommAnnouncement();
	AuCommAnnouncement(const AuCommAnnouncement& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Speaker.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Announcement;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	void SetText(const QString& text)
	{
		m_Text = text;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Universal Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommThought : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this log message. Thought messages should be expressed in
																// the first person; they are presented as thoughts of the player.

public:
	AuCommThought(const QString& text, const AuId& fromAndToId, const QString& fromName);
	AuCommThought();
	AuCommThought(const AuCommThought& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/OrderDaydream.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Thought;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	void SetText(const QString& text)
	{
		m_Text = text;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		// Thought messages are always sent privately to the from id.
		return FromId();
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Thought");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommThoughtMailAlert : public AuCommThought
{
private:
	quint8								m_MsgType;

public:
	AuCommThoughtMailAlert(quint8 msgType, const QString& text, const AuId& fromAndToId, const QString& fromName);
	AuCommThoughtMailAlert();
	AuCommThoughtMailAlert(const AuCommThoughtMailAlert& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MailMsg.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ThoughtMailAlert;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mail Arrived Thought");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommThoughtMessage : public AuCommPacket
{
public:
	enum MESSAGE
	{
		LiftCalled,
		LiftCalledDoorBlocked,
		LiftCalledDoorStuck,
		IImpregnatedThem,				// Text is their name.
		TheyImpregnatedMe,				// Text is their name.
		TurretCharged,
		TurretLoaded,
		NothingRipe,
		StaticDisplayFruit,
		NoItemsPlaced,
		NoPlaceLikeHome,
		RecallCanceled,
		WorldClaimByEmpire,				// fields: empire name, claim time, flagLL, claim person
		WorldClaimByFriend,				// fields: empire name, claim time, flagLL, claim person
		WorldClaimByVassal,				// fields: empire name, claim time, flagLL, claim person
		WorldClaimByNeutral,			// fields: empire name, claim time, flagLL, claim person
		WorldClaimAlienHQ,				// fields: empire name
		WorldClaimAlienCapitol,			// fields: empire name
		WorldClaimNoFlag,
		WorldClaimedHabitableDiffDNA,	// fields: empire name
		WorldClaimedHabitableSameDNA,	// fields: empire name
		WorldClaimedUninhabitable,		// fields: empire name
		WorldUnclaimed,
		WorldClaimLostNeutral,			// fields: empire name
		WorldClaimLostEnemy,			// fields: empire name
		QuitClaimNoClaim,				// fields: empire name
		QuitClaimCapitol,
		QuitClaimHQ,
		QuitClaimCoC,
		QuitClaim,						// fields: empire name
		IImatureToMate,
		TheyImatureToMate,				// Text is their name.
		ItemPurchased,					// Fields: item name, purchase amount
		ItemPurchasedDropped,			// Fields: item name, purchase amount, dropped
		ItemTraded,						// Fields: offer item name, offer amt, buy item name, buy amount
		ItemTradedDropped,				// Fields: offer item name, offer amt, buy item name, buy amount, dropped
		BuyItemGone,					// Fields: item name
		InsufficientFunds,				// Fields: item name, amount offered
		OfferItemGone,					// Fields: item name, amount offered
		ItemNoDeal,						// Fields: item name, my bottom line
		ItemTaken,						// Fields: item name, amount
		ItemTakenDropped,				// Fields: item name, amount, dropped
		Boink,
		O,
		ItemDiscarded,					// Fields: item name, amount
		MessageCount					// LAST ITEM
	};

private:
	QStringList							m_Fields;				// Text values for QString %NN fields.
	MESSAGE								m_Message;				// Message to be made by this thought.

public:
	// Constructor for messages with no fields.
	AuCommThoughtMessage(MESSAGE message, const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with one field.
	AuCommThoughtMessage(MESSAGE message, const QString& field, const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with two fields.
	AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const AuId& fromAndToId,
			const QString& fromName);

	// Constructor for messages with three fields.
	AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
			const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with four fields.
	AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
			const QString& field4, const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with five fields.
	AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
			const QString& field4, const QString& field5, const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with six fields.
	AuCommThoughtMessage(MESSAGE message, const QString& field1, const QString& field2, const QString& field3,
			const QString& field4, const QString& field5, const QString& field6, const AuId& fromAndToId, const QString& fromName);

	// Constructor for messages with multiple fields.
	AuCommThoughtMessage(MESSAGE message, const QStringList& fields, const AuId& fromAndToId, const QString& fromName);

	AuCommThoughtMessage();
	AuCommThoughtMessage(const AuCommThoughtMessage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/OrderDaydream.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ThoughtMessage;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		// Thought messages are always sent privately to the from id.
		return FromId();
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Thought Message");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommMissionNew : public AuCommPacket
{
private:
	AuMission							m_Mission;				// The mission.
	QString								m_MissionName;			// Name of the mission.
	quint32								m_SenderEmpire;			// Empire id of sender.
	AuId								m_ToId;					// Ship the mission is directed to.

public:
	AuCommMissionNew(const AuId& toId, const QString& missionName, const AuMission& mission,
			quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMissionNew();
	AuCommMissionNew(const AuCommMissionNew& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/FleetShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::MissionNew;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/c_MissionDisk.png");
	}

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	inline const AuMission& Mission() const
	{
		return m_Mission;
	}

	inline const QString& MissionName() const
	{
		return m_MissionName;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mission New");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommMissionOrderAdd : public AuCommPacket
{
private:
	AuMissionOrder*						m_MissionOrder;			// The mission order.
	quint32								m_SenderEmpire;			// Empire id of sender.
	AuId								m_ToId;					// Ship the order is directed to.

public:
	AuCommMissionOrderAdd(const AuId& toId, const AuMissionOrder& missionOrder,
			quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMissionOrderAdd();
	virtual ~AuCommMissionOrderAdd()
	{
		if(m_MissionOrder)
			delete m_MissionOrder;
	}
	AuCommMissionOrderAdd(const AuCommMissionOrderAdd& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/FleetShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::MissionOrderAdd;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Plus.png");
	}

	inline const AuMissionOrder* MissionOrder() const
	{
		return m_MissionOrder;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mission Order Add");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommMissionOrderInsert : public AuCommPacket
{
private:
	AuMissionOrder*						m_MissionOrder;			// The mission order.
	quint32								m_SenderEmpire;			// Empire id of sender.
	AuId								m_ToId;					// Ship the order is directed to.

public:
	AuCommMissionOrderInsert(const AuId& toId, const AuMissionOrder& missionOrder,
			quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMissionOrderInsert();
	virtual ~AuCommMissionOrderInsert()
	{
		if(m_MissionOrder)
			delete m_MissionOrder;
	}
	AuCommMissionOrderInsert(const AuCommMissionOrderInsert& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/FleetShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::MissionOrderInsert;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/I.png");
	}

	inline const AuMissionOrder* MissionOrder() const
	{
		return m_MissionOrder;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mission Order Insert");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommMissionOrderNew : public AuCommPacket
{
private:
	AuMissionOrder*						m_MissionOrder;			// The mission order.
	quint32								m_SenderEmpire;			// Empire id of sender.
	AuId								m_ToId;					// Ship the order is directed to.

public:
	AuCommMissionOrderNew(const AuId& toId, const AuMissionOrder& missionOrder,
			quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMissionOrderNew();
	virtual ~AuCommMissionOrderNew()
	{
		if(m_MissionOrder)
			delete m_MissionOrder;
	}
	AuCommMissionOrderNew(const AuCommMissionOrderNew& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/FleetShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::MissionOrderNew;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/NarrateChosen.png");
	}

	virtual const AuMissionOrder* MissionOrder() const
	{
		return m_MissionOrder;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mission Order New");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommMissionProperty : public AuCommPacket
{
private:
	quint8								m_Property;				// 0=req mission, 1=active state, 2=cycle, 3=req disk
	quint32								m_SenderEmpire;			// Empire id of sender.
	AuId								m_ToId;					// Ship the message is directed toward.
	quint8								m_Value;				// Property value to set, if appropriate.

public:
	AuCommMissionProperty(const AuId& toId, quint8 property, quint8 value,
			quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommMissionProperty();
	AuCommMissionProperty(const AuCommMissionProperty& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/FleetShip.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::MissionProperty;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/GovSpacecraft.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline quint8 Property() const
	{
		return m_Property;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Mission Setting Change");
	}

	inline quint8 Value() const
	{
		return m_Value;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommNarrate : public AuCommPacket
{
private:
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommNarrate(const QString& text, quint32 storyInstance,
			const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommNarrate();
	AuCommNarrate(const AuCommNarrate& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Microphone.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Narrate;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Narrate");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommNarrateAudio : public AuCommPacket
{
private:
	QByteArray							m_SoundData;			// Sound data.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommNarrateAudio(const QString& text, quint32 storyInstance, const QByteArray& soundData,
			const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommNarrateAudio();
	AuCommNarrateAudio(const AuCommNarrateAudio& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateAudio.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::NarrateAudio;
	}

	virtual QString Icon() const;

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const QByteArray& SoundData() const
	{
		return m_SoundData;
	}

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Narrate Audio");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommNarrateChoice : public AuCommPacket
{
private:
	qint8								m_ChoiceId;				// Id of the choice, among multiple choices.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommNarrateChoice(const QString& text, quint32 storyInstance, qint8 choiceId,
			const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommNarrateChoice();
	AuCommNarrateChoice(const AuCommNarrateChoice& c);

	inline qint8 ChoiceId() const
	{
		return m_ChoiceId;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateChoice.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::NarrateChoice;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return false;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Narrate Choice");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommNarrateImage : public AuCommPacket
{
private:
	QImage								m_Image;				// Hardware independent image data.
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.
	QString								m_Title;

public:
	AuCommNarrateImage(const QString& title, const QString& text, quint32 storyInstance, const QImage& image,
			const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommNarrateImage();
	AuCommNarrateImage(const AuCommNarrateImage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateImage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::NarrateImage;
	}

	virtual QString Icon() const;

	// Return true if the data size of this packet in a data stream will exceed 63k.
	virtual bool IsDataLarge() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const QImage& Image() const
	{
		return m_Image;
	}

	inline const QString& ImageTitle() const
	{
		return m_Title;
	}

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Narrate Picture");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommNarrateTitle : public AuCommPacket
{
private:
	quint32								m_StoryInstance;		// Instance id of the associated story.
	QString								m_Text;					// Text of this chat message.

public:
	AuCommNarrateTitle(const QString& text, quint32 storyInstance,
			const QString& channelName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommNarrateTitle();
	AuCommNarrateTitle(const AuCommNarrateTitle& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Speaker.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::NarrateTitle;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 StoryInstance() const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Narrate Title");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class of all AuCommOfferRepair derived packets.
// Not a packet type per se.

class AUDATA_DLL AuCommOfferRepair : public AuCommPacket
{
protected:
	quint8								m_MinQ;					// Minimum quality of parts to install.
	quint32								m_Price;				// Price of repairs.
	quint16								m_RepairSecs;			// Seconds required to carry out the repairs.
	AuId								m_ShipId;				// The spacecraft for which the offer is made.
	QString								m_ShipName;
	quint16								m_Workers;

public:
	AuCommOfferRepair(const AuId& shipId, const QString& shipName, quint32 price, quint16 repairSecs, quint8 minQ, quint16 workers,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferRepair();
	AuCommOfferRepair(const AuCommOfferRepair& c);

	// Only cities offer repairs. The from id is the city id.
	const AuId& CityId() const
	{
		return FromId();
	}

	virtual quint8 MinQ() const
	{
		return m_MinQ;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	inline quint16 RepairSeconds() const
	{
		return m_RepairSecs;
	}

	virtual AuId RepairShipId() const
	{
		return m_ShipId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferRepairAll : public AuCommOfferRepair
{
private:
	QList<COMMODITYID>					m_NoQ;					// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_ShortQ;				// Commodities in short supply at min Q.
	QList<COMMODITYID>					m_NoSupply;				// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_Enough;				// Commodities in adequate supply at lower Q.

public:
	AuCommOfferRepairAll(const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply,
			const QList<COMMODITYID>& enough, const AuId& shipId, const QString& shipName, quint32 price, quint16 repairSecs,
			quint8 minQ, quint16 workers,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferRepairAll();
	AuCommOfferRepairAll(const AuCommOfferRepairAll& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Damage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferRepairAll;
	}

	virtual QString Icon() const;

	inline const QList<COMMODITYID>& PartsUnavailableAtMinQ() const
	{
		return m_NoQ;
	}

	inline const QList<COMMODITYID>& PartsShortAtMinQ() const
	{
		return m_ShortQ;
	}

	inline const QList<COMMODITYID>& PartsNone() const
	{
		return m_NoSupply;
	}

	inline const QList<COMMODITYID>& PartsEnough() const
	{
		return m_Enough;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Offer to Repair All Systems");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferRepairSystem : public AuCommOfferRepair
{
private:
	QList<COMMODITYID>					m_NoQ;					// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_ShortQ;				// Commodities in short supply at min Q.
	QList<COMMODITYID>					m_NoSupply;				// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_Enough;				// Commodities in adequate supply at lower Q.
	quint8								m_System;				// SYSTEM
	QString								m_SystemDesc;			// System description.
	quint16								m_Which;				// System 'which' identifier.

public:
	AuCommOfferRepairSystem(quint8 system, quint16 which, const QString& systemDesc, const QList<COMMODITYID>& noQ,
			const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply, const QList<COMMODITYID>& enough,
			const AuId& shipId, const QString& shipName, quint32 price, quint16 repairSecs, quint8 minQ, quint16 workers,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferRepairSystem();
	AuCommOfferRepairSystem(const AuCommOfferRepairSystem& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Damage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferRepairSystem;
	}

	virtual QString Icon() const;

	inline const QList<COMMODITYID>& PartsUnavailableAtMinQ() const
	{
		return m_NoQ;
	}

	inline const QList<COMMODITYID>& PartsShortAtMinQ() const
	{
		return m_ShortQ;
	}

	inline const QList<COMMODITYID>& PartsNone() const
	{
		return m_NoSupply;
	}

	inline const QList<COMMODITYID>& PartsEnough() const
	{
		return m_Enough;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual SYSTEM System() const
	{
		return (SYSTEM)m_System;
	}

	virtual const QString& SystemDesc() const
	{
		return m_SystemDesc;
	}

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Offer to Repair Specific System");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferUpgradeInstall : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;		// Empire that manufactured the module.
	AuItem*								m_Item;
	quint16								m_InstallSecs;			// Seconds required to install the module.
	quint32								m_Price;				// Price of installing the module.
	quint16								m_Which;				// System 'which' identifier.
	quint16								m_Workers;				// Number of workers to do the work.

public:
	AuCommOfferUpgradeInstall(const AuItem& item, const QString& empireOfOrigin, quint16 systemWhich, quint32 price,
			quint16 installSecs, quint16 workers,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferUpgradeInstall();
	AuCommOfferUpgradeInstall(const AuCommOfferUpgradeInstall& c);
	virtual ~AuCommOfferUpgradeInstall();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/ShipUpgrade.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferUpgradeInstall;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	inline quint16 InstallSeconds() const
	{
		return m_InstallSecs;
	}

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual quint32 Price() const
	{
		return m_Price;
	}

	virtual SYSTEM System() const;

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Offer to Install System Module");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfficerBoarded : public AuCommPacket
{
private:
	bool								m_BeamedUp;				// True if they beamed aboard.
	AuId								m_LoaderId;				// Id of ship. Not the same as sender id.
	QString								m_OfficerName;			// Name of officer who boarded.

public:
	AuCommOfficerBoarded(const AuId& loaderId, const QString& officerName, bool beamedUp,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfficerBoarded();
	AuCommOfficerBoarded(const AuCommOfficerBoarded& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Officer.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfficerBoarded;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedUp)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgLoadColonists.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Officer Boarding Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommPassengersBoarded : public AuCommPacket
{
private:
	bool								m_BeamedUp;				// True if they beamed aboard.
	quint16								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship. Not the same as sender id.

public:
	AuCommPassengersBoarded(const AuId& loaderId, quint16 count, bool beamedUp,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommPassengersBoarded();
	AuCommPassengersBoarded(const AuCommPassengersBoarded& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Passenger.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::PassengersBoarded;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedUp)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgLoadColonists.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger Boarding Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCryoDebarked : public AuCommPacket
{
private:
	bool								m_BeamedDown;			// True if the cryo travellers beamed ashore from the ship.
	quint32								m_Count;				// Count of cryo travellers wanted.
	QStringList							m_Species;
	AuId								m_UnloaderId;			// Id of ship. Not the same as sender id.

public:
	AuCommCryoDebarked(const AuId& unloaderId, quint32 count, bool beamedDown, const QStringList& species,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCryoDebarked();
	AuCommCryoDebarked(const AuCommCryoDebarked& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CryoPassenger.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CryoDebarked;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedDown)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgUnloadColonists.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Cryo Traveller Debarking Result");
	}

	inline const AuId& UnloaderId() const
	{
		return m_UnloaderId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommPassengersDebarked : public AuCommPacket
{
private:
	bool								m_BeamedDown;			// True if the passengers beamed ashore from the ship.
	quint16								m_Count;				// Count of crew wanted.
	quint32								m_Fee;					// Transport fee collected.
	AuId								m_UnloaderId;			// Id of ship. Not the same as sender id.

public:
	AuCommPassengersDebarked(const AuId& unloaderId, quint16 count, quint32 transportFee, bool beamedDown,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommPassengersDebarked();
	AuCommPassengersDebarked(const AuCommPassengersDebarked& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Passenger.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::PassengersDebarked;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedDown)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgUnloadColonists.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Passenger Debarking Result");
	}

	inline const AuId& UnloaderId() const
	{
		return m_UnloaderId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommPreBoard : public AuCommPacket
{
private:
	quint8								m_Berths;				// Berths available.
	quint8								m_BerthType;			// 1=1stCl pass, 2=2ndCl pass, 3=3rdCl pass, 4=troop, 5=crew
	QString								m_LocLongLat;			// City or station coordinates.
	QString								m_LocName;				// City or station name.
	quint8								m_LocType;				// Location type: 0=road slab, 1=planet station, 2=system station
	quint8								m_Minutes;				// Minutes to board.
	QString								m_NextStop;				// Next solar system destination. May be blank.
	AuId								m_ShipId;				// Id of ship that is going to board passengers.
	QString								m_ShipName;				// Name of ship boarding passengers.

public:
	AuCommPreBoard(const AuId& shipId, const QString& shipName, quint8 minutes, quint8 berths, quint8 berthType,
			quint8 locType, const QString& locName, const QString& locLongLat, const QString& nextStop,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommPreBoard();
	AuCommPreBoard(const AuCommPreBoard& c);

	quint8 BerthType() const
	{
		return m_BerthType;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgLoadColonists.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::PreBoard;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/mpRunning.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Pre Boarding Announcement");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthAny : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthAny(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthAny();
	AuCommRequestBerthAny(const AuCommRequestBerthAny& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthAny;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/c_Spacecraft.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Any Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthCaptain : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthCaptain(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthCaptain();
	AuCommRequestBerthCaptain(const AuCommRequestBerthCaptain& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthCaptain;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Captain.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Captain's Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthCaptainSlot : public AuCommPacket
{
public:
	AuCommRequestBerthCaptainSlot(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthCaptainSlot();
	AuCommRequestBerthCaptainSlot(const AuCommRequestBerthCaptainSlot& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthCaptainSlot;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Captain.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Captain's Position");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthCrew : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthCrew(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthCrew();
	AuCommRequestBerthCrew(const AuCommRequestBerthCrew& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthCrew;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Crewman.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Crew Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthOfficer : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthOfficer(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthOfficer();
	AuCommRequestBerthOfficer(const AuCommRequestBerthOfficer& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthOfficer;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Officer.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Officer Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthPassenger : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthPassenger(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthPassenger();
	AuCommRequestBerthPassenger(const AuCommRequestBerthPassenger& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthPassenger;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Passenger Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestBerthTroop : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommRequestBerthTroop(quint32 senderEmpire,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestBerthTroop();
	AuCommRequestBerthTroop(const AuCommRequestBerthTroop& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RequestBerth.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestBerthTroop;
	}

	virtual quint32 FromEmpireId() const;

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Troop.png");
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Troop Berth");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestExterminator : public AuCommPacket
{
public:
	AuCommRequestExterminator(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestExterminator();
	AuCommRequestExterminator(const AuCommRequestExterminator& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Rat.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestExterminator;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/c_Spaceship.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Exterminator");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestHouseKeeping : public AuCommPacket
{
public:
	AuCommRequestHouseKeeping(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestHouseKeeping();
	AuCommRequestHouseKeeping(const AuCommRequestHouseKeeping& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Sweep.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestHouseKeeping;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/c_Spaceship.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request House Keeping");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestInventory : public AuCommPacket
{
public:
	AuCommRequestInventory(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestInventory();
	AuCommRequestInventory(const AuCommRequestInventory& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommManifest.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestInventory;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Inventory");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestHelp : public AuCommPacket
{
public:
	AuCommRequestHelp(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range);
	AuCommRequestHelp();
	AuCommRequestHelp(const AuCommRequestHelp& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommHelp.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestHelp;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Help");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestRepairAll : public AuCommPacket
{
private:
	quint8								m_MinQ;					// Minimum quality of parts to install.

public:
	AuCommRequestRepairAll(quint8 minQ,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestRepairAll();
	AuCommRequestRepairAll(const AuCommRequestRepairAll& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Damage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestRepairAll;
	}

	virtual QString Icon() const;

	virtual quint8 MinQ() const
	{
		return m_MinQ;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Repairs to All Systems");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestRepairSystem : public AuCommPacket
{
private:
	quint8								m_MinQ;					// Minimum quality of parts to install.
	quint8								m_System;				// SYSTEM
	QString								m_SystemDesc;			// System description.
	quint16								m_Which;				// System 'which' identifier.

public:
	AuCommRequestRepairSystem(quint8 system, quint16 which, const QString& systemDesc, quint8 minQ,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestRepairSystem();
	AuCommRequestRepairSystem(const AuCommRequestRepairSystem& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Damage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestRepairSystem;
	}

	virtual QString Icon() const;

	virtual quint8 MinQ() const
	{
		return m_MinQ;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual SYSTEM System() const
	{
		return (SYSTEM)m_System;
	}

	inline const QString& SystemDesc() const
	{
		return m_SystemDesc;
	}

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Repairs to Specific System");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestUpgradeSystem : public AuCommPacket
{
private:
	quint8								m_System;				// SYSTEM
	QString								m_SystemDesc;			// System description.
	quint16								m_Which;				// System 'which' identifier.

public:
	AuCommRequestUpgradeSystem(quint8 system, quint16 which, const QString& systemDesc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestUpgradeSystem();
	AuCommRequestUpgradeSystem(const AuCommRequestUpgradeSystem& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/ShipUpgrade.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestUpgradeSystem;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual SYSTEM System() const
	{
		return (SYSTEM)m_System;
	}

	virtual const QString& SystemDesc() const
	{
		return m_SystemDesc;
	}

	virtual quint16 SystemWhich() const
	{
		return m_Which;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request System Module Upgrade");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRevokeBid : public AuCommPacket
{
public:
	AuCommRevokeBid(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRevokeBid();
	AuCommRevokeBid(const AuCommRevokeBid& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgRevoke.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RevokeBid;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/c_Money.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Revoke Bid");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitAllSales : public AuCommPacket
{
public:
	AuCommSolicitAllSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitAllSales();
	AuCommSolicitAllSales(const AuCommSolicitAllSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitAllSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_All));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For All Item Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitAmmoSales : public AuCommPacket
{
public:
	AuCommSolicitAmmoSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitAmmoSales();
	AuCommSolicitAmmoSales(const AuCommSolicitAmmoSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitAmmoSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Ammo));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Ammunition Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitArmorSales : public AuCommPacket
{
public:
	AuCommSolicitArmorSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitArmorSales();
	AuCommSolicitArmorSales(const AuCommSolicitArmorSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitArmorSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_ArmorAndClothes));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Armor And Clothing Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitConstructionMaterialSales : public AuCommPacket
{
public:
	AuCommSolicitConstructionMaterialSales(const QString& channelName, const AuId& fromId, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitConstructionMaterialSales();
	AuCommSolicitConstructionMaterialSales(const AuCommSolicitConstructionMaterialSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitConstructionMaterialSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_ConstructionMaterials));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Construction Material Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitFoodSales : public AuCommPacket
{
public:
	AuCommSolicitFoodSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitFoodSales();
	AuCommSolicitFoodSales(const AuCommSolicitFoodSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitFoodSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Food));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Food And Drink Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitFuelSales : public AuCommPacket
{
public:
	AuCommSolicitFuelSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitFuelSales();
	AuCommSolicitFuelSales(const AuCommSolicitFuelSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitFuelSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Fuel));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Fuel Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitItemSales : public AuCommPacket
{
private:
	COMMODITYID							m_CID;					// The item requested.

public:
	AuCommSolicitItemSales(COMMODITYID cid,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitItemSales();
	AuCommSolicitItemSales(const AuCommSolicitItemSales& c);

	virtual AuCommPacket* Clone() const;

	virtual COMMODITYID CommodityId() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitItemSales;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Item Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitMedSales : public AuCommPacket
{
public:
	AuCommSolicitMedSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitMedSales();
	AuCommSolicitMedSales(const AuCommSolicitMedSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitMedSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_MedicalSupplies));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Medical Supply Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitPartsSales : public AuCommPacket
{
public:
	AuCommSolicitPartsSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitPartsSales();
	AuCommSolicitPartsSales(const AuCommSolicitPartsSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitPartsSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_SpareParts));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Spare Part Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitPatentSales : public AuCommPacket
{
public:
	AuCommSolicitPatentSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitPatentSales();
	AuCommSolicitPatentSales(const AuCommSolicitPatentSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitPatentSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Patents));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Patent Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitToolSales : public AuCommPacket
{
public:
	AuCommSolicitToolSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitToolSales();
	AuCommSolicitToolSales(const AuCommSolicitToolSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitToolSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Tools));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Tool Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitWeaponSales : public AuCommPacket
{
public:
	AuCommSolicitWeaponSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitWeaponSales();
	AuCommSolicitWeaponSales(const AuCommSolicitWeaponSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitWeaponSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Weapons));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Weapon Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitNaturalResourceSales : public AuCommPacket
{
public:
	AuCommSolicitNaturalResourceSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitNaturalResourceSales();
	AuCommSolicitNaturalResourceSales(const AuCommSolicitNaturalResourceSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitNaturalResourceSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_NaturalResources));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Natural Resource Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitMiscSales : public AuCommPacket
{
public:
	AuCommSolicitMiscSales(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitMiscSales();
	AuCommSolicitMiscSales(const AuCommSolicitMiscSales& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitMiscSales;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(CommodityFilterIcon(FILTER_Misc));
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Miscellaneous Sales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSpotTargetOrTerrain : public AuCommPacket
{
private:
	bool								m_IsCivilian;
	POLITICAL_STANCE					m_StanceTowardTarget;	// Stance of spotter toward the target.
	QString								m_Text;					// Text of this spot message.

public:
	AuCommSpotTargetOrTerrain(const QString& text, POLITICAL_STANCE stanceTowardTarget, bool isCivilian,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSpotTargetOrTerrain();
	AuCommSpotTargetOrTerrain(const AuCommSpotTargetOrTerrain& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSpot.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SpotTargetOrTerrain;
	}

	virtual QString Icon() const;

	inline bool IsCivilian() const
	{
		return m_IsCivilian;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline POLITICAL_STANCE StanceTowardTarget() const
	{
		return m_StanceTowardTarget;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Spot Target Or Terrain");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommTroopBoardingComplete : public AuCommPacket
{
public:
	AuCommTroopBoardingComplete(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommTroopBoardingComplete();
	AuCommTroopBoardingComplete(const AuCommTroopBoardingComplete& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgRevoke.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::TroopBoardingComplete;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Troop.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Troop Boarding Complete");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommTroopsBoarded : public AuCommPacket
{
private:
	bool								m_BeamedUp;				// True if they beamed aboard.
	quint16								m_Count;				// Count of crew wanted.
	AuId								m_LoaderId;				// Id of ship. Not the same as sender id.

public:
	AuCommTroopsBoarded(const AuId& loaderId, quint16 count, bool beamedUp,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommTroopsBoarded();
	AuCommTroopsBoarded(const AuCommTroopsBoarded& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Troop.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::TroopsBoarded;
	}

	virtual quint32 Count() const
	{
		return m_Count;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		if(m_BeamedUp)
			return QString(":data/MsgTransport.png");
		else
			return QString(":data/MsgLoadColonists.png");
	}

	inline const AuId& LoaderId() const
	{
		return m_LoaderId;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Troop Boarding Result");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommDiplomaticMessage : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this chat message.

public:
	AuCommDiplomaticMessage(const QString& text,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommDiplomaticMessage();
	AuCommDiplomaticMessage(const AuCommDiplomaticMessage& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommDiplomacy.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::DiplomaticMessage;
	}

	// Returns the literal text of the message sent by the person.
	inline const QString& DiplomaticMessage() const
	{
		return m_Text;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const;

	inline static int MaxChatText()
	{
		return 256;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Diplomatic Message");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCitySurrenderOffered : public AuCommPacket
{
private:
	AuIdList							m_CityIds;
	QStringList							m_CityNames;
	quint32								m_FromEmpireId;			// Empire surrendering the cities.
	quint32								m_ToEmpireId;			// Empire to which the cities are offered.

public:
	AuCommCitySurrenderOffered(quint32 fromEmpireId, quint32 toEmpireId, const AuIdList& cityIds, const QStringList& cityNames,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCitySurrenderOffered();
	AuCommCitySurrenderOffered(const AuCommCitySurrenderOffered& c);

	inline void CityIds(AuIdList& cityIds) const
	{
		cityIds = m_CityIds;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/SurrenderFlag.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CitySurrenderOffered;
	}

	virtual quint32 FromEmpireId() const
	{
		return m_FromEmpireId;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const
	{
		if(m_CityIds.isEmpty())
			return false;
		if(m_CityIds.count() != m_CityNames.count())
			return false;
		if(!m_FromEmpireId)
			return false;
		if(!m_ToEmpireId)
			return false;
		if(m_FromEmpireId == m_ToEmpireId)
			return false;
		return AuCommPacket::IsValid();
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual quint32 ToEmpireId() const
	{
		return m_ToEmpireId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("City Surrender Offered");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCitySurrenderAccepted : public AuCommPacket
{
private:
	AuIdList							m_CityIds;
	quint32								m_AcceptingEmpireId;	// Empire to which the cities were offered.
	AuId								m_SurrenderingAvatarId;	// Avatar surrendering the cities.
	quint32								m_SurrenderingEmpireId;	// Empire surrendering the cities.

public:
	AuCommCitySurrenderAccepted(quint32 acceptingEmpireId, const AuId& surrenderingAvatarId, quint32 surrenderingEmpireId,
			const AuIdList& cityIds,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCitySurrenderAccepted();
	AuCommCitySurrenderAccepted(const AuCommCitySurrenderAccepted& c);

	inline void CityIds(AuIdList& cityIds) const
	{
		cityIds = m_CityIds;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::CitySurrenderAccepted;
	}

	virtual quint32 FromEmpireId() const
	{
		return m_AcceptingEmpireId;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const
	{
		if(m_CityIds.isEmpty())
			return false;
		if(!m_AcceptingEmpireId)
			return false;
		if(m_SurrenderingAvatarId.IsNull())
			return false;
		if(!m_SurrenderingEmpireId)
			return false;
		if(m_AcceptingEmpireId == m_SurrenderingEmpireId)
			return false;
		return AuCommPacket::IsValid();
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	inline const AuId& SurrenderingAvatarId() const
	{
		return m_SurrenderingAvatarId;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual quint32 ToEmpireId() const
	{
		return m_SurrenderingEmpireId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("City Surrender Accepted");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOS : public AuCommPacket
{
private:
	QString								m_Loc;

public:
	AuCommSOS(const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOS();
	AuCommSOS(const AuCommSOS& c);

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSOS.png");
	}

	virtual QString Icon() const;

	virtual bool IsValid() const;

	inline const QString& LocDesc() const
	{
		return m_Loc;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSManeuverDriveAdrift : public AuCommSOS
{
public:
	AuCommSOSManeuverDriveAdrift(const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSManeuverDriveAdrift();
	AuCommSOSManeuverDriveAdrift(const AuCommSOSManeuverDriveAdrift& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSManeuverDriveAdrift;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Ship Maneuver Drive, Adrift");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSManeuverDriveMarooned : public AuCommSOS
{
public:
	AuCommSOSManeuverDriveMarooned(const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSManeuverDriveMarooned();
	AuCommSOSManeuverDriveMarooned(const AuCommSOSManeuverDriveMarooned& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSManeuverDriveMarooned;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Ship Maneuver Drive, Marooned");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSManeuverDriveStation : public AuCommSOS
{
public:
	AuCommSOSManeuverDriveStation(const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSManeuverDriveStation();
	AuCommSOSManeuverDriveStation(const AuCommSOSManeuverDriveStation& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSManeuverDriveStation;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Station Maneuver Drive");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSAttack : public AuCommSOS
{
private:
	QString								m_AttackerName;
	QString								m_NameWithEmpire;

public:
	AuCommSOSAttack(const QString& nameWithEmpire, const QString& attackerName, const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSAttack();
	AuCommSOSAttack(const AuCommSOSAttack& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSAttack;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Under Attack");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSFuelMarooned : public AuCommSOS
{
public:
	AuCommSOSFuelMarooned(const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSFuelMarooned();
	AuCommSOSFuelMarooned(const AuCommSOSFuelMarooned& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSFuelMarooned;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Ship Fuel, Marooned");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSFuelShip : public AuCommSOS
{
private:
	quint8								m_FuelPercent;			// 0 to 100

public:
	AuCommSOSFuelShip(quint8 fuelPercent, const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSFuelShip();
	AuCommSOSFuelShip(const AuCommSOSFuelShip& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSFuelShip;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Ship Fuel");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSOSFuelStation : public AuCommSOS
{
private:
	quint8								m_FuelPercent;			// 0 to 100

public:
	AuCommSOSFuelStation(quint8 fuelPercent, const QString& loc,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSOSFuelStation();
	AuCommSOSFuelStation(const AuCommSOSFuelStation& c);

	virtual AuCommPacket* Clone() const;

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SOSFuelStation;
	}

	virtual bool IsValid() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("SOS Station Fuel");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferAvatar : public AuCommPacket
{
private:
	AuId								m_ToId;					// Person offered the avatar, who is the sender of the message.
	QString								m_ToName;

public:
	AuCommOfferAvatar(const AuId& toId, const QString& toName,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferAvatar();
	AuCommOfferAvatar(const AuCommOfferAvatar& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Passenger.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferAvatar;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Offer Avatar");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptAvatar : public AuCommPacket
{
private:
	AuId								m_AvatarId;				// The avatar offered is also the toId of this message.
	QString								m_AvatarName;


public:
	AuCommAcceptAvatar(const AuId& avatarId, const QString& avatarName,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptAvatar();
	AuCommAcceptAvatar(const AuCommAcceptAvatar& c);

	const AuId& AvatarId() const
	{
		return m_AvatarId;
	}

	const QString& AvatarName() const
	{
		return m_AvatarName;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptAvatar;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/Passenger.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_AvatarId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Avatar");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommShipStasisDimension : public AuCommPacket
{
private:
	AuTime								m_ReadyTime;			// Time the ship will be ready to restore from stasis.

public:
	AuCommShipStasisDimension(const AuTime& readyTime,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommShipStasisDimension();
	AuCommShipStasisDimension(const AuCommShipStasisDimension& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/RangeGalaxy.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ShipStasisDimension;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Stasis Restore Wrong Dimension");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommShipStasisWait : public AuCommPacket
{
private:
	AuTime								m_ReadyTime;			// Time the ship will be ready to restore from stasis.

public:
	AuCommShipStasisWait(const AuTime& readyTime,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommShipStasisWait();
	AuCommShipStasisWait(const AuCommShipStasisWait& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/mpRunning.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ShipStasisWait;
	}

	virtual QString Icon() const;

	virtual bool IsValid() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Stasis Restore Wait");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSilverwill : public AuCommPacket
{
private:
	QString								m_Text;					// Text of this announcement.

public:
	AuCommSilverwill(const QString& text, const QString& channelName);
	AuCommSilverwill();
	AuCommSilverwill(const AuCommSilverwill& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Silverwill.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Silverwill;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	void SetText(const QString& text)
	{
		m_Text = text;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Silverwill Task Manager");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRequestRefit : public AuCommPacket
{
private:
	AuRefitShip							m_Refit;
	AuId								m_ShipId;

public:
	AuCommRequestRefit(const AuId& shipId, const AuRefitShip& refit,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRequestRefit();
	AuCommRequestRefit(const AuCommRequestRefit& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/ShipRefit.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RequestRefit;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	const AuRefitShip& Refit() const
	{
		return m_Refit;
	}

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Request Spacecraft Refit");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferRefit : public AuCommPacket
{
private:
	double								m_EstCost;
	AuRefitMaterials					m_EstMaterials;
	quint32								m_EstSeconds;
	AuRefitShip							m_Refit;
	AuId								m_ShipId;
	quint16								m_Workers;

public:
	AuCommOfferRefit(const AuId& shipId, const AuRefitShip& refit, double estCost, quint32 estSec, const AuRefitMaterials& estMat,
			quint16 workers,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferRefit();
	AuCommOfferRefit(const AuCommOfferRefit& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/ShipRefit.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferRefit;
	}

	inline double EstCost() const
	{
		return m_EstCost;
	}

	inline const AuRefitMaterials& EstMaterials() const
	{
		return m_EstMaterials;
	}

	inline quint32 EstSeconds() const
	{
		return m_EstSeconds;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	const AuRefitShip& Refit() const
	{
		return m_Refit;
	}

	virtual AuId RepairShipId() const
	{
		return m_ShipId;
	}

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Offer Spacecraft Refit");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptRefit : public AuCommPacket
{
private:
	AuId								m_CityId;				// The city who offered the refit.
	double								m_EstCost;
	quint32								m_EstSeconds;
	AuRefitShip							m_Refit;
	AuId								m_ShipId;

public:
	AuCommAcceptRefit(const AuId& cityId, const AuId& shipId, const AuRefitShip& refit, double estCost, quint32 estSec,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptRefit();
	AuCommAcceptRefit(const AuCommAcceptRefit& c);

	inline const AuId& CityId() const
	{
		return m_CityId;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptRefit;
	}

	inline double EstCost() const
	{
		return m_EstCost;
	}

	inline quint32 EstSeconds() const
	{
		return m_EstSeconds;
	}

	virtual QString Icon() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	const AuRefitShip& Refit() const
	{
		return m_Refit;
	}

	virtual AuId RepairShipId() const
	{
		return m_ShipId;
	}

	inline const AuId& ShipId() const
	{
		return m_ShipId;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Spacecraft Refit");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChannelOpened : public AuCommPacket
{
private:
	AuId								m_ToId;
	QString								m_ToName;

public:
	AuCommChannelOpened(const AuId& toId, const QString& toName, const QString& channelName, const AuId& fromId,
			const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, AuCommStdChannel::RANGE range,
			AuCommStdChannel::SCOPE scope);
	AuCommChannelOpened();
	AuCommChannelOpened(const AuCommChannelOpened& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommChannel.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChannelOpened;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Comm Channel Opened");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommInviteTeleport : public AuCommPacket
{
private:
	AuId								m_ToId;					// Recipient id of message.
	QString								m_ToName;

public:
	AuCommInviteTeleport(const AuId& toId, const QString& toName,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommInviteTeleport();
	AuCommInviteTeleport(const AuCommInviteTeleport& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/InviteTeleport.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::InviteTeleport;
	}

	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Invitation to Teleport");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptTeleport : public AuCommPacket
{
private:
	AuId								m_ToId;					// Recipient id of message.

public:
	AuCommAcceptTeleport(const AuId& toId,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptTeleport();
	AuCommAcceptTeleport(const AuCommAcceptTeleport& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptTeleport;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return QString(":data/InviteTeleport.png");
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	virtual const AuId& ToId() const
	{
		return m_ToId;
	}

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Invitation to Teleport");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommCombine : public AuCommPacket
{
private:
	QString								m_EmpireOfOrigin;		// Empire that made the item. May be blank.
	QString								m_EmpireOfOrigin2;		// Empire that made the item. May be blank.
	AuItem*								m_Item;					// The item to be combined.
	AuItem*								m_Item2;				// The item to be combined.

public:
	AuCommCombine(const AuItem& item, const QString& empireOfOrigin, const AuItem& item2, const QString& empireOfOrigin2,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommCombine();
	AuCommCombine(const AuCommCombine& c);
	virtual ~AuCommCombine();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CombineCargo.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::Combine;
	}

	virtual const QString& EmpireOfOrigin() const;

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual const AuItem* OfferItem() const
	{
		return m_Item2;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Combine Cargo");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommRepairPartsMissing : public AuCommPacket
{
private:
	QList<COMMODITYID>					m_NoQ;					// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_ShortQ;				// Commodities in short supply at min Q.
	QList<COMMODITYID>					m_NoSupply;				// Commodities unavailable at min Q.
	QList<COMMODITYID>					m_Enough;				// Commodities in adequate supply at lower Q.
	quint8								m_MinQ;

public:
	AuCommRepairPartsMissing(const QList<COMMODITYID>& noQ, const QList<COMMODITYID>& shortQ, const QList<COMMODITYID>& noSupply,
			const QList<COMMODITYID>& enough, quint8 minQ,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommRepairPartsMissing();
	AuCommRepairPartsMissing(const AuCommRepairPartsMissing& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/Damage.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::RepairPartsMissing;
	}

	virtual QString Icon() const;

	inline const QList<COMMODITYID>& PartsUnavailableAtMinQ() const
	{
		return m_NoQ;
	}

	inline const QList<COMMODITYID>& PartsShortAtMinQ() const
	{
		return m_ShortQ;
	}

	inline const QList<COMMODITYID>& PartsNone() const
	{
		return m_NoSupply;
	}

	inline const QList<COMMODITYID>& PartsEnough() const
	{
		return m_Enough;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Repair Parts Missing");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommChoicePicked : public AuCommChat
{
private:
	QString								m_Text;					// Text of this chat message.

public:
	AuCommChoicePicked(const QString& text,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommChoicePicked();
	AuCommChoicePicked(const AuCommChoicePicked& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/NarrateNotChosen.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ChoicePicked;
	}

	virtual QString Icon() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Choice Picked");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommShipmentItem : public AuCommPacket
{
private:
	AuShipment*							m_Shipment;

public:
	AuCommShipmentItem(const AuShipment& shipment,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommShipmentItem();
	AuCommShipmentItem(const AuCommShipmentItem& c);
	virtual ~AuCommShipmentItem();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/c_Container.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ShipmentItem;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const;

	virtual const AuItem* Item() const;

	virtual const AuShipment* Shipment() const
	{
		return m_Shipment;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Shipment Listing");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommSolicitShipments : public AuCommPacket
{
public:
	AuCommSolicitShipments(const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector,
			GALAXY fromGalaxy, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommSolicitShipments();
	AuCommSolicitShipments(const AuCommSolicitShipments& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgSolicitSales.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::SolicitShipments;
	}

	virtual QString Icon() const;

	virtual QString Icon2() const
	{
		return "c_Container.png";
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Ask For Shipments");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommBidPickupShipment : public AuCommPacket
{
private:
	AuShipment*							m_Shipment;

public:
	AuCommBidPickupShipment(const AuShipment& shipment,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommBidPickupShipment();
	AuCommBidPickupShipment(const AuCommBidPickupShipment& c);
	virtual ~AuCommBidPickupShipment();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/c_Container.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::BidPickupShipment;
	}

	bool CorrespondsTo(const AuCommPacket* commPacket) const;

	virtual QString Icon() const;

	virtual QString Icon2() const;

	virtual const AuItem* Item() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual const AuShipment* Shipment() const
	{
		return m_Shipment;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Bid to Pickup Shipment");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommAcceptPickupShipment : public AuCommPacket
{
private:
	AuId								m_BuyerId;
	QString								m_BuyerName;
	AuShipment*							m_Shipment;

public:
	AuCommAcceptPickupShipment(const AuId& buyerId, const QString& buyerName, const AuShipment& shipment,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommAcceptPickupShipment();
	AuCommAcceptPickupShipment(const AuCommAcceptPickupShipment& c);
	virtual ~AuCommAcceptPickupShipment();

	inline const AuId& BuyerId() const
	{
		return m_BuyerId;
	}

	inline const QString& BuyerName() const
	{
		return m_BuyerName;
	}

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/MsgAccept.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::AcceptPickupShipment;
	}

	virtual QString Icon() const;

	virtual const AuItem* Item() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual const AuShipment* Shipment() const
	{
		return m_Shipment;
	}

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Accept Pickup Shipment");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOpenTEU : public AuCommPacket
{
private:
	AuItem*								m_Item;					// The item to be jettisoned.

public:
	AuCommOpenTEU(const AuItem& item,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOpenTEU();
	AuCommOpenTEU(const AuCommOpenTEU& c);
	virtual ~AuCommOpenTEU();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/c_Container.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OpenTEU;
	}

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Open TEU in Hold");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommOfferTEU : public AuCommPacket
{
private:
	AuItem*								m_Item;

public:
	AuCommOfferTEU(const AuItem& item,
			const QString& channelName, const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope);
	AuCommOfferTEU();
	AuCommOfferTEU(const AuCommOfferTEU& c);
	virtual ~AuCommOfferTEU();

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/c_Container.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::OfferTEU;
	}

	virtual QString Icon() const;

	virtual const AuItem* Item() const
	{
		return m_Item;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("TEU For Delivery");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommContactShip : public AuCommPacket
{
private:
	quint32								m_SenderEmpire;			// Empire id of sender.

public:
	AuCommContactShip(quint32 senderEmpire, const AuId& senderShipId,
			const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy);
	AuCommContactShip();
	AuCommContactShip(const AuCommContactShip& c);

	virtual AuCommPacket* Clone() const;

	// Returns blank so this message type will not appear on the comm settings ui.
	inline static QString CommPacketIcon()
	{
		return QString();
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ContactShip;
	}

	virtual quint32 FromEmpireId() const;

	// Returns blank so this message type will not appear on the comm settings ui.
	virtual QString Icon() const;

	virtual bool OnScreenByDefault() const
	{
		return false;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Contact Ship");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCommContactReply : public AuCommPacket
{
private:
	AuId								m_SystemId;				// Solar system of sender.
	QString								m_Text;					// Text of the reply.

public:
	// 'fromId' is the ship id.
	AuCommContactReply(const QString& text, const AuId& senderSystemId,
			const AuId& fromId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			AuCommStdChannel::RANGE range);
	AuCommContactReply();
	AuCommContactReply(const AuCommContactReply& c);

	virtual AuCommPacket* Clone() const;

	inline static QString CommPacketIcon()
	{
		return QString(":data/CommAnnounce.png");
	}

	// Returns the unique number assigned to this comm packet derived class. Every derived class must return a unique number.
	inline static TYPE CommPacketType()
	{
		return AuCommPacket::ContactReply;
	}

	const AuId& FromSolarSystemId() const
	{
		return m_SystemId;
	}

	virtual QString Icon() const;

	/*
	virtual QString Icon2() const
	{
		return QString(":data/Officer.png");
	}
	*/

	virtual bool IsRepeatedAtLogin() const
	{
		return false;
	}

	virtual bool OnScreenByDefault() const
	{
		return true;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	// Returns the text of the comm message.
	virtual QString Text() const;

	// Returns a number unique to each derived comm packet class. Identifies each type of comm packet in a data stream.
	virtual TYPE Type() const;

	virtual QString TypeName() const
	{
		return QObject::tr("Login Announcement from Ship");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuCommPacket_H
