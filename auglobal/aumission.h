//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumission.h,v 1.120 2022/05/04 19:38:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMission_H
#define AuMission_H

#include "aucommstdchannel.h"
#include "auid.h"
#include "augloballib.h"
#include "auglobell.h"
#include "aupoint.h"
#include "autime.h"
#include "dbcommodity.h"
#include <QComboBox>
#include <QDebug>
#include <QList>
#include <QListWidget>
#include <QTableWidget>
#include <typeinfo>						// for std::bad_cast

class QDataStream;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuHarvestOp
{
	quint32								m_Amount;
	quint8								m_AmountType;			// 0=percent, 1=actual count of total hold space to fill, 2=idle
	quint16								m_CommodityId;			// Commodity to harvest.

public:	// Constructors
	inline AuHarvestOp(COMMODITYID cid, quint32 amount, quint8 amountType)
	{
		m_Amount = amount;
		m_AmountType = amountType;
		m_CommodityId = cid;
	}

	inline AuHarvestOp()
	{
		m_Amount = 0;
		m_AmountType = 0;
		m_CommodityId = CID_Invalid;
	}

	inline AuHarvestOp(const AuHarvestOp& c)
	{
		m_Amount = c.m_Amount;
		m_AmountType = c.m_AmountType;
		m_CommodityId = c.m_CommodityId;
	}

public:	// Operators
	inline const AuHarvestOp& operator=(const AuHarvestOp& c)
	{
		m_Amount = c.m_Amount;
		m_AmountType = c.m_AmountType;
		m_CommodityId = c.m_CommodityId;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_AmountType;
		out << m_Amount;
		out << m_CommodityId;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_AmountType;
		in >> m_Amount;
		in >> m_CommodityId;
	}

	inline bool operator==(const AuHarvestOp& c) const
	{
		if(m_CommodityId != c.m_CommodityId)
			return false;
		if(m_Amount != c.m_Amount)
			return false;
		if(m_AmountType != c.m_AmountType)
			return false;
		return true;
	}

	inline bool operator!=(const AuHarvestOp& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline quint32 Amount() const
	{
		return m_Amount;
	}

	inline quint8 AmountType() const
	{
		return m_AmountType;
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	inline void SetAmount(quint32 amount)
	{
		m_Amount = amount;
	}

	inline void SetAmountType(quint8 amountType)
	{
		m_AmountType = amountType;
	}

	inline void SetCommodityId(COMMODITYID cid)
	{
		m_CommodityId = cid;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHarvestOp& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHarvestOp& c)
{
	c << in;
	return in;
}

inline unsigned int qHash(const AuHarvestOp& key)
{
	return key.CommodityId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuHarvestOps : public QMap<quint16, AuHarvestOp>
{
public:	// Constructors
	AuHarvestOps()
	{
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator oi = cbegin(); oi != cend(); oi++)
		{
			out << oi.key();
			out << *oi;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 s;
		in >> s;
		while(s--)
		{
			quint16 k;
			AuHarvestOp o;
			in >> k;
			in >> o;
			insert(k, o);
		}
	}

public:	// Methods
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuHarvestOps& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHarvestOps& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrder
{
public:
	enum CONTEXT
	{
		Galaxy,
		Sector,
		SolarSystem,
	};

	// Add nu types at the end of this enum for the least impact on the world. If any existing enum values change, the network
	// version must be changed and all servers and clients must be rebuilt and restarted.
	enum TYPE
	{
		Invalid,
		Attack,							// AuMissionOrderAttack
		CargoBuyCategory,				// AuMissionOrderCargoBuyCategory
		CargoBuyCommodity,				// AuMissionOrderCargoBuyCommodity
		CargoSellCategory,				// AuMissionOrderCargoSellCategory
		CargoSellCommodity,				// AuMissionOrderCargoSellCommodity
		Christen,						// AuMissionOrderChristen
		CrewLoad,						// AuMissionOrderCrewLoad
		DamageReport,					// AuMissionOrderDamageReport
		Follow,							// AuMissionOrderFollow
		GoToBestCity,					// AuMissionOrderGoToBestCity
		GoToBestStation,				// AuMissionOrderGoToBestStation
		GoToBestStationOrCity,			// AuMissionOrderGoToBestStationOrCity
		GoToCity,						// AuMissionOrderGoToCity
		GoToCityAnywhere,				// AuMissionOrderGoToCityAnywhere
		GoToClosestCity,				// AuMissionOrderGoToClosestCity
		GoToClosestStation,				// AuMissionOrderGoToClosestStation
		GoToClosestStationOrCity,		// AuMissionOrderGoToClosestStationOrCity
		GoToCoordinate,					// AuMissionOrderGoToCoordinate
		GoToHoverSite,					// AuMissionOrderGoToHoverSite
		GoToLand,						// AuMissionOrderGoToLand
		GoToLandingSite,				// AuMissionOrderGoToLandingSite
		GoToOrbit,						// AuMissionOrderGoToOrbit
		GoToOrbitBestPlanet,			// AuMissionOrderGoToOrbitBestPlanet
		GoToOrbitCity,					// AuMissionOrderGoToOrbitCity
		GoToOrbitCoords,				// AuMissionOrderGoToOrbitCoords
		GoToStation,					// AuMissionOrderGoToStation
		GoToSystemDirect_Deprecated,	// AuMissionOrderGoToSystemDirect_Deprecated
		GoToSystemViaWormholes,			// AuMissionOrderGoToSystemViaWormholes
		Harvest,						// AuMissionOrderHarvest
		HarvestAtmosphere,				// AuMissionOrderHarvestAtmosphere
		HarvestMinerals,				// AuMissionOrderHarvestMinerals
		HarvestParticles,				// AuMissionOrderHarvestParticles
		HarvestSea,						// AuMissionOrderHarvestSea
		HarvestWell,					// AuMissionOrderHarvestWell
		Hide,							// AuMissionOrderHide
		HoldMission,					// AuMissionOrderHoldMission
		LightState,						// AuMissionOrderLightState
		PassengersLoad,					// AuMissionOrderPassengersLoad
		PassengersUnload,				// AuMissionOrderPassengersUnload
		PatrolCities,					// AuMissionOrderPatrolCities
		PatrolStations,					// AuMissionOrderPatrolStations
		PatrolSystem,					// AuMissionOrderPatrolSystem
		PatrolWormholes,				// AuMissionOrderPatrolWormholes
		Plunder,						// AuMissionOrderPlunder
		Posture,						// AuMissionOrderPosture
		GoToSystemDirect,				// AuMissionOrderGoToSystemDirect
		ScanSurveyPlanet,				// AuMissionOrderScanSurveyPlanet
		ScanSurveySystem,				// AuMissionOrderScanSurveySystem
		ScanTactical,					// AuMissionOrderScanTactical
		Scuttle,						// AuMissionOrderScuttle
		ShieldPolicy,					// AuMissionOrderShieldPolicy
		ShieldsDown,					// AuMissionOrderShieldsDown
		StandDown,						// AuMissionOrderStandDown
		Stay,							// AuMissionOrderStay
		TroopsLoad,						// AuMissionOrderTroopsLoad
		PilotPolicy,					// AuMissionOrderPilotPolicy
		ExterminateRats,				// AuMissionOrderExterminateRats
		TransferCargoFrom,				// AuMissionOrderTransferCargoFrom
		TransferCargoTo,				// AuMissionOrderTransferCargoTo
		TransferMoneyFrom,				// AuMissionOrderTransferMoneyFrom
		TransferMoneyTo,				// AuMissionOrderTransferMoneyTo
		TransferPeopleFrom,				// AuMissionOrderTransferPeopleFrom
		TransferPeopleTo,				// AuMissionOrderTransferPeopleTo
		TransferCargoCategoryFrom,		// AuMissionOrderTransferCargoCategoryFrom
		TransferCargoCategoryTo,		// AuMissionOrderTransferCargoCategoryTo
		HarvestOperationByBay,			// AuMissionOrderHarvestOperationByBay
		HarvestOperationByModule,		// AuMissionOrderHarvestOperationByModule
		Report,							// AuMissionOrderReport
		CryoBerthsLoad,					// AuMissionOrderCryoBerthsLoad
		CryoBerthsUnload,				// AuMissionOrderCryoBerthsUnload
		Repair,							// AuMissionOrderRepair
		OrderTypes						// LAST ENTRY EXPECTED NOT TO EXCEED A BYTE. ADD NEW TYPES BEFORE THIS ENTRY.
	};

	enum EXECTYPE
	{
		Sequential,					// Order is executed in sequence. Most orders are sequential orders.
		DoOnce,						// Order is executed once, no matter if the mission is cyclic or not. Used for temporary orders.
	};

protected:
	quint8								m_ExecType;				// EXECTYPE enum
	quint8								m_Id;					// Order id unique within a mission. 0 is invalid. 1-1023 is valid.

public:
	AuMissionOrder(EXECTYPE execType);
	AuMissionOrder();
	virtual ~AuMissionOrder()
	{
	}
	AuMissionOrder(const AuMissionOrder& c);

	inline void AddListWidgetEntry(QListWidget* lw, bool isSelected=false)
	{
		QListWidgetItem* lwi = new QListWidgetItem(QIcon(ListNameIcon()), ListName(), lw);
		lwi->setData(Qt::UserRole, Type());
		lwi->setData(Qt::UserRole+1, MissionOrderId());
		lwi->setSelected(isSelected);
		lwi->setToolTip(ListToolTip());
	}

public:	// Operators
	const AuMissionOrder& operator=(const AuMissionOrder& c);

public:	// Methods

	virtual bool AllBays() const
	{
		return false;
	}

	// Return altitude in meters.
	virtual quint16 Altitude() const
	{
		return 0;
	}

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return 0;
	}

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual double AmountOfMoney() const
	{
		return 0;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return 0;
	}

	// Returns 0=use system Q and repair by system, 1=use ship manufacture Q, 2=use min Q.
	virtual int ApplyMinQ() const
	{
		return 0;
	}

	virtual AuId Authorization() const
	{
		return AuId(0);
	}

	virtual CITIZEN_POSTURE CitizenPosture() const
	{
		return POSTURE_Offensive;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const = 0;

	virtual COMMODITYFILTER CommodityFilter() const
	{
		return FILTER_All;
	}

	virtual COMMODITYID CommodityId() const
	{
		return CID_Invalid;
	}

	virtual QString CommChannel() const
	{
		return "";
	}
	
	virtual AuCommStdChannel::RANGE CommRange() const
	{
		return AuCommStdChannel::InvalidRange;
	}

	// Allocates the appropriate mission order object on the heap based on the value returned by Type() for each derived class.
	// Useful when reading a mission order from a data stream.
	static AuMissionOrder* Create(TYPE orderType);

	// Destination id returned for orders that refer to an atom.
	virtual AuId DestinationId() const
	{
		return AuId(0);
	}

	virtual CONTEXT DestinationContext() const
	{
		return Galaxy;
	}

	virtual AuPoint DestinationCoordinate() const
	{
		return AuPoint(0,0);
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return QString("");
	}

	virtual bool HeadlightOn() const
	{
		return false;
	}

	virtual quint8 FuelThreshold() const
	{
		return 0;
	}

	// Returns longitude and latitude coordinate.
	virtual AuGlobeLLF GlobeLL() const
	{
		return AuGlobeLLF();
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const
	{
		return QString();
	}

	// Harvest id returned for orders that refer to an atom.
	virtual AuId HarvestId() const
	{
		return AuId(0);
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return QString("");
	}

	virtual const AuHarvestOps& HarvestOps() const;

	// Returns the head count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return 0;
	}

	inline bool IsDoOnce() const
	{
		return m_ExecType == DoOnce;
	}

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& /*orderStarted*/) const
	{
		return false;
	}

	// Patrol orders return true.
	virtual bool IsPatrol() const
	{
		return false;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& c) const
	{
		//if(m_Id != c.m_Id)
			//return false;
		if(m_ExecType != c.m_ExecType)
			return false;
		return true;
	}

	// Star travel orders return true.
	virtual bool IsStarTravelOrder() const
	{
		return false;
	}

	virtual bool IsTravelOrder() const
	{
		return false;
	}

	virtual bool IsValid() const
	{
		// Id 0 is allowed in the IsValid test since those ids are AuMission-dependent and a single mission order could exist
		// independent of an AuMission. Many cases exist, particularly in the UI that creates nu mission orders.
		if(m_ExecType != Sequential && m_ExecType != DoOnce)
			return false;
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const = 0;

	// Provides an icon representing the description of the order.
	virtual QString ListDescriptionIcon() const = 0;

	// Returns the name of the mission order.
	virtual QString ListName() const = 0;

	// Provides an icon representing the type of the order.
	virtual QString ListNameIcon() const = 0;

	virtual QString ListToolTip() const = 0;

	quint64 MemoryUsage() const;

	virtual quint8 MinQ() const
	{
		return 0;
	}

	virtual quint16 Minutes() const
	{
		return 0;
	}

	// Id of this order when it is part of a mission.
	inline quint8 MissionOrderId() const
	{
		return m_Id;
	}

	virtual bool NavLightOn() const
	{
		return false;
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const = 0;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& c) const
	{
		if(m_Id != c.m_Id)
			return false;
		if(m_ExecType != c.m_ExecType)
			return false;
		return true;
	}

	inline bool operator!=(const AuMissionOrder& c) const
	{
		return !operator==(c);
	}

	// 0=officer, 1=crew, 2=troop, 3=passenger
	virtual quint8 PeopleType() const
	{
		return 0;
	}

	virtual PILOT_UP_POLICY PilotUpPolicy() const
	{
		return UpAwayFromGravity;
	}

	virtual int PreboardingAnnouncementMinutes() const
	{
		return 0;
	}

	// 0=percent of nominal, 1=actual price, 2=any price
	virtual quint8 PriceMarginType() const
	{
		return 0;
	}

	// Return price margin. May be a percentage 0..100 or an actual number of cronos, depending on PriceMarginType.
	virtual quint32 PriceMarginValue() const
	{
		return 0;
	}

	// Return true for report generating orders that are supposed to send that report to their entire chain of command.
	virtual bool ReportToCommand() const
	{
		return false;
	}

	virtual quint16 ReportWhat() const
	{
		return 0;
	}

	virtual bool RotatingBeaconOn() const
	{
		return false;
	}

	virtual QString SayText() const
	{
		return QString("");
	}

	inline void SetMissionOrderId(quint8 orderId)
	{
		m_Id = orderId;
	}

	// Ship id returned for orders that refer to an atom.
	virtual AuId ShipId() const
	{
		return AuId(0);
	}

	// Ship name returned for orders that refer to an atom.
	virtual QString ShipName() const
	{
		return QString("");
	}

	virtual SHIELDFUEL_POLICY ShipShieldFuelPolicy() const
	{
		return ShieldUpUnlessNoFuel;
	}

	virtual SHIELD_POLICY ShipShieldPolicy() const
	{
		return ShieldUp;
	}

	virtual bool StrobeOn() const
	{
		return false;
	}

	// Target id returned for orders that refer to an atom.
	virtual AuId TargetId() const
	{
		return AuId(0);
	}

	// Target name returned for orders that refer to an atom.
	virtual QString TargetName() const
	{
		return QString("");
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const = 0;

	// Returns the statement of the officer in command when issuing this order.
	// 'pilot' and 'navigator' are never blank.
	// 'navigator' will be "Navigator" if there is no actual person/AI identified as such.
	// 'pilot' will be "Helmsman" if there is no actual person/AI identified as such.
	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const = 0;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return 0;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuMissionOrder& orderPacket)
{
	orderPacket >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMissionOrder& orderPacket)
{
	orderPacket << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMission : protected QList<AuMissionOrder*>
{
public:
	enum TYPE
	{
		Single,							// Mission is executed only once. Current/top item is removed when completed.
		Repeat,							// Mission is executed repeated. Current/top item is moved to the end when completed.
		TypeCount						// LAST ITEM. NOT A VALID TYPE VALUE.
	};

private:
	bool								m_Active;				// true if mission is active. Order() returns Stay when inactive.
	AuId								m_LastArriveId;			// Atom id of last destination at which the ship arrived.
	AuTime								m_OrderStart;			// Time current order started/activated and time patrol point
																// reached. For waiting and loitering.
	TYPE								m_Type;					// Type of mission.

public:
	AuMission();
	virtual ~AuMission()
	{
		Clear();
	}
	AuMission(const AuMission& c);

	inline void AddTableWidgetEntries(QTableWidget* tw)
	{
		tw->setRowCount(count());

		int row=0;
		for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
		{
			int col=0;
			QTableWidgetItem* tworder = new QTableWidgetItem(QIcon((*moi)->ListNameIcon()), "");
			tworder->setData(Qt::UserRole, (*moi)->MissionOrderId());
			tw->setItem(row,col++, tworder);

			QTableWidgetItem* twdesc;
			if((*moi)->IsDoOnce())
				twdesc = new QTableWidgetItem(QIcon((*moi)->ListDescriptionIcon()),
						QObject::tr("Do once. %1").arg((*moi)->ListDescription()));
			else
				twdesc = new QTableWidgetItem(QIcon((*moi)->ListDescriptionIcon()), (*moi)->ListDescription());
			twdesc->setData(Qt::UserRole, (*moi)->MissionOrderId());
			tw->setItem(row,col++, twdesc);
			row++;
		}
	}

	inline bool Append(const AuMissionOrder& order)
	{
		quint8 orderId = NextId();
		if(!orderId)
			return false;

		AuMissionOrder* nuOrder = order.Clone();
		Q_ASSERT(nuOrder);
		nuOrder->SetMissionOrderId(orderId);

		append(nuOrder);
		if(count() == 1)
			m_OrderStart.Set();			// This is now the current order.
		return true;
	}

	inline void ArrivedAtDestination(const AuId& destId)
	{
		m_LastArriveId = destId;
	}

	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}

	inline bool Contains(quint8 orderId) const
	{
		return FindId(orderId) != constEnd();
	}

	inline int Count() const
	{
		return count();
	}

	// Returns the top order in this list or a reference to a default constructed order if this order list is empty.
	const AuMissionOrder& CurrentOrder() const;

	// Returns the Id of the order that is currently first in the list of orders to be completed.
	// Returns 0 if there is no current order.
	inline quint8 CurrentOrderId() const
	{
		if(isEmpty())
			return 0;
		return first()->MissionOrderId();
	}

	inline void Delete(quint8 orderId)
	{
		iterator moi = FindId(orderId);
		if(moi != end())
		{
			// Update order start time when the current/first order is deleted and orders remain.
			if(m_Active && moi == begin() && count() > 1)
				m_OrderStart.Set();								// A nu order has become current.

			// Delete the order from this mission list.
			delete *moi;
			moi = erase(moi);
		}
	}

	// Returns end() if 'orderId' is not found.
	inline iterator FindId(quint8 orderId)
	{
		for(iterator moi = begin(); moi != end(); moi++)
			if((*moi)->MissionOrderId() == orderId)
				return moi;
		return end();
	}

	// Returns constEnd() if 'orderId' is not found.
	inline const_iterator FindId(quint8 orderId) const
	{
		for(const_iterator moi = constBegin(); moi != constEnd(); moi++)
			if((*moi)->MissionOrderId() == orderId)
				return moi;
		return constEnd();
	}

	// Returns 0 if 'orderId' is not found.
	inline const AuMissionOrder* FindOrder(quint8 orderId) const
	{
		const_iterator moi = FindId(orderId);
		if(moi == constEnd())
			return 0;
		return *moi;
	}

	QString HtmlDesc() const;

	QString HtmlReportTable() const;

	QString HtmlToolTip() const;

	// Inserts 'order' into the mission.
	// 'beforeId' is the id of the mission order before which the nu order is inserted.
	// 'beforeId' of 0 inserts at the beginning of the list, otherwise the insert fails if 'beforeId' is not found.
	inline bool Insert(quint8 beforeId, const AuMissionOrder& order)
	{
		// Inserting to an empty list == append, unless the caller specified a 'beforeId'.
		if(isEmpty())
		{
			if(beforeId)
				return false;									// Existing id not found.
			return Append(order);
		}

		// Find where to insert the nu order.
		iterator moi;
		if(beforeId)
		{
			// Caller passed an order id. If the order is not found, the Insert fails.
			moi = FindId(beforeId);
			if(moi == end())
				return false;									// Existing id not found.

			// This is the nu current order if it is at the beginning.
			if(moi == begin())
				m_OrderStart.Set();								// This is now the current order.
		}
		else
		{
			// Caller passed invalid order id; this inserts at the head.
			moi = begin();										// Existing id not valid so insert at the beginning.
			m_OrderStart.Set();									// This is now the current order.
		}

		quint8 orderId = NextId();
		if(!orderId)
			return false;										// No unused order ids are available. List must be full.

		// Create a copy of the order for this list, with an id appropriate to this list.
		AuMissionOrder* nuOrder = order.Clone();
		Q_ASSERT(nuOrder);
		nuOrder->SetMissionOrderId(orderId);

		// Insert the nu order into this list.
		insert(moi, nuOrder);
		return true;
	}

	inline bool IsActive() const
	{
		return m_Active;
	}

	inline bool IsEmpty() const
	{
		return isEmpty();
	}

	inline const AuId& LastArrivalId() const
	{
		return m_LastArriveId;
	}

	// Appends the ListDescription of each order to 'listDescriptions'.
	void ListDescriptions(QStringList& listDescriptions) const;

	// Appends the ListDescription of each order to 'listDescriptions'.
	// Do once orders are prepended with the term "Do once."
	void ListDescriptionsWithDoOnce(QStringList& listDescriptions) const;

	quint64 MemoryUsage() const;

	// Returns the next id available for a mission order. Returns 0 if no ids are available, i.e. the mission is full.
	// Note that ids are not necessarily sequential in a list of orders.
	quint8 NextId() const;

	// Returns the name of the next solar system that is a destination.
	QString NextSystem() const;

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	void operator>>(QDebug& dbg) const;

	inline const AuMission& operator=(const AuMission& c)
	{
		m_Active = c.m_Active;
		m_LastArriveId = c.m_LastArriveId;
		m_OrderStart = c.m_OrderStart;
		m_Type = c.m_Type;

		Clear();
		for(AuMission::const_iterator coi = c.constBegin(); coi != c.constEnd(); coi++)
			append((*coi)->Clone());
		return *this;
	}

	bool operator==(const AuMission& c) const;

	inline bool operator!=(const AuMission& c) const
	{
		return !operator==(c);
	}

	inline void operator+=(const AuMission& c)
	{
		for(const_iterator moi = c.constBegin(); moi != c.constEnd(); moi++)
		{
			// Get the next order id, if one is available.
			quint8 nextId = NextId();
			if(!nextId)
				break;

			AuMissionOrder* order = (*moi)->Clone();
			Q_ASSERT(order);
			order->SetMissionOrderId(nextId);
			append(order);
		}
	}

	inline const AuTime& OrderStart() const
	{
		return m_OrderStart;
	}

	// Replaces 'orderId' with 'order' in the mission.
	// 'orderId' is the id of the mission order that will be replaced by the nu order.
	// Returns true if 'orderId' is found and replaced.
	// Returns false if 'orderId' is not found.
	inline bool Replace(quint8 orderId, const AuMissionOrder& order)
	{
		// Find where to insert the nu order.
		// If the existing order is not found, the Insert fails.
		iterator moi = FindId(orderId);
		if(moi == end())
			return false;										// Existing id not found.

		// This is the nu current order if it is at the beginning.
		if(moi == begin())
			m_OrderStart.Set();									// This is now the current order.

		// Create a copy of the order for this list, reusing the id of the old order.
		AuMissionOrder* nuOrder = order.Clone();
		Q_ASSERT(nuOrder);
		nuOrder->SetMissionOrderId(orderId);					// We can reuse the id since the old order is being replaced.

		// Replace the existing order in the list.
		delete *moi;
		*moi = nuOrder;
		return true;
	}

	// Select the next order.
	// Returns true if the current order changes, even if it changes to nothing.
	// Returns true if the mission is cyclic and there is only one order in the list, because the start time for the current order
	// gets reset.
	// Returns false if the mission is empty on entry to the function.
	inline bool SelectNextOrder()
	{
		// Empty mission list results in no change.
		if(isEmpty())
			return false;

		if(m_Type == Single || first()->IsDoOnce())
			delete takeFirst();									// Current order is removed.
		else if(count() > 1)
			append(takeFirst());								// Current order is rotated to the last position.

		if(m_Active && !isEmpty())
			m_OrderStart.Set();									// Update current order start time.

		return true;
	}

	inline void SetActive(bool isActive)
	{
		m_Active = isActive;
		if(m_Active)
			m_OrderStart.Set();									// Reset current order start time.
	}

	inline void SetType(TYPE type)
	{
		m_Type = type;
	}

	// Searches back from the order preceding 'orderId' for the last travel order that contains a solar system id.
	// Returns AuId(0) if no solar system travel orders exist prior to 'orderId'.
	AuId SystemIdBefore(quint8 orderId) const;

	// Searches back from the end of the mission for the last travel order that contains a solar system id.
	// Returns AuId(0) if no solar system travel orders exist.
	AuId SystemIdFinal() const;

	inline TYPE Type() const
	{
		return m_Type;
	}
};

Q_DECLARE_METATYPE(AuMission)		// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.
									// Also needed to use this class in qVariantSetValue.

inline QDataStream& operator<<(QDataStream& out, const AuMission& mission)
{
	mission >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMission& mission)
{
	mission << in;
	return in;
}

AUGLOBAL_DLL QDebug operator<<(QDebug dbg, const AuMission& mission);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is mission name.

class AUGLOBAL_DLL AuMissionList : public QMap<QString, AuMission>
{
public:	// Constructors
	inline AuMissionList()
	{
	}

	inline AuMissionList(const AuMissionList& c) : QMap<QString, AuMission>(c)
	{
	}

public:	// Operators
	inline const AuMissionList& operator=(const AuMissionList& c)
	{
		QMap<QString, AuMission>::operator=(c);
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	QString HtmlListContents() const;

	quint64 MemoryUsage() const;

	bool Remove(const QString& missionName);
};

Q_DECLARE_METATYPE(AuMissionList)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.
									// Also needed to use this class in qVariantSetValue.

inline QDataStream& operator<<(QDataStream& out, const AuMissionList& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMissionList& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderAttack : public AuMissionOrder
{
private:
	AuId								m_TargetId;
	QString								m_TargetName;

public:
	AuMissionOrderAttack(const AuId& targetId, const QString& targetName, EXECTYPE execType = Sequential);
	AuMissionOrderAttack();
	AuMissionOrderAttack(const AuMissionOrderAttack& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderAttack& c = dynamic_cast<const AuMissionOrderAttack&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_TargetId.IsNull() || m_TargetName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderAttack& c = dynamic_cast<const AuMissionOrderAttack&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual AuId TargetId() const
	{
		return m_TargetId;
	}

	// Target name returned for orders that refer to an atom.
	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Attack;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCargoBuyCategory : public AuMissionOrder
{
private:
	quint32								m_Amount;				// Number of units or percent 1..100.
	quint8								m_AmountType;			// 0=percent, 1=actual count, 2=each count
	quint8								m_CommodityFilter;
	quint32								m_Margin;				// Maximum price or percent of nominal.
	quint8								m_MarginType;			// 0=max percent of nominal, 1=actual max price, 2=pay any price

public:
	AuMissionOrderCargoBuyCategory(COMMODITYFILTER commodityFilter, quint32 amount, quint8 amountType, quint32 margin,
			quint8 marginType, EXECTYPE execType = Sequential);
	AuMissionOrderCargoBuyCategory();
	AuMissionOrderCargoBuyCategory(const AuMissionOrderCargoBuyCategory& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_Amount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_AmountType;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYFILTER CommodityFilter() const
	{
		return (COMMODITYFILTER)m_CommodityFilter;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoBuyCategory& c = dynamic_cast<const AuMissionOrderCargoBuyCategory&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_AmountType == 0)
		{
			if(m_Amount == 0 || m_Amount > 100)
				return false;
		}
		else
		{
			if(m_Amount == 0)
				return false;
		}
		if(!DbCommodity::IsFilterValid((COMMODITYFILTER)m_CommodityFilter))
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoBuyCategory& c = dynamic_cast<const AuMissionOrderCargoBuyCategory&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 4;
	}

	// 0=percent of nominal, 1=actual price, 2=any price
	virtual quint8 PriceMarginType() const
	{
		return m_MarginType;
	}

	// Return price margin. May be a percentage 0..100 or an actual number of cronos, depending on PriceMarginType.
	virtual quint32 PriceMarginValue() const
	{
		return m_Margin;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CargoBuyCategory;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCargoBuyCommodity : public AuMissionOrder
{
private:
	quint32								m_Amount;				// Number of units or percent 1..100.
	quint8								m_AmountType;			// 0=percent, 1=actual count
	quint16								m_CommodityId;
	quint32								m_Margin;				// Maximum price or percent of nominal.
	quint8								m_MarginType;			// 0=max percent of nominal, 1=actual max price, 2=pay any price

public:
	AuMissionOrderCargoBuyCommodity(COMMODITYID commodityId, quint32 amount, quint8 amountType, quint32 margin,
			quint8 marginType, EXECTYPE execType = Sequential);
	AuMissionOrderCargoBuyCommodity();
	AuMissionOrderCargoBuyCommodity(const AuMissionOrderCargoBuyCommodity& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_Amount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_AmountType;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoBuyCommodity& c = dynamic_cast<const AuMissionOrderCargoBuyCommodity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_AmountType == 0)
		{
			if(m_Amount == 0 || m_Amount > 100)
				return false;
		}
		else
		{
			if(m_Amount == 0)
				return false;
		}
		if(m_CommodityId <= CID_Invalid || m_CommodityId >= CID_Count)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoBuyCommodity& c = dynamic_cast<const AuMissionOrderCargoBuyCommodity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// 0=percent of nominal, 1=actual price, 2=any price
	virtual quint8 PriceMarginType() const
	{
		return m_MarginType;
	}

	// Return price margin. May be a percentage 0..100 or an actual number of cronos, depending on PriceMarginType.
	virtual quint32 PriceMarginValue() const
	{
		return m_Margin;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CargoBuyCommodity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCargoSellCategory : public AuMissionOrder
{
private:
	quint32								m_Amount;				// Number of units or percent 1..100.
	quint8								m_AmountType;			// 0=percent, 1=actual count
	quint8								m_CommodityFilter;
	quint32								m_Margin;				// Minimum price or percent of nominal.
	quint8								m_MarginType;			// 0=min percent of nominal, 1=actual min price, 2=accept any price

public:
	AuMissionOrderCargoSellCategory(COMMODITYFILTER commodityFilter, quint32 amount, quint8 amountType, quint32 margin,
			quint8 marginType, EXECTYPE execType = Sequential);
	AuMissionOrderCargoSellCategory();
	AuMissionOrderCargoSellCategory(const AuMissionOrderCargoSellCategory& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_Amount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_AmountType;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYFILTER CommodityFilter() const
	{
		return (COMMODITYFILTER)m_CommodityFilter;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoSellCategory& c = dynamic_cast<const AuMissionOrderCargoSellCategory&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_AmountType == 0)
		{
			if(m_Amount == 0 || m_Amount > 100)
				return false;
		}
		else
		{
			if(m_Amount == 0)
				return false;
		}
		if(!DbCommodity::IsFilterValid((COMMODITYFILTER)m_CommodityFilter))
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoSellCategory& c = dynamic_cast<const AuMissionOrderCargoSellCategory&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 4;
	}

	// 0=percent of nominal, 1=actual price, 2=any price
	virtual quint8 PriceMarginType() const
	{
		return m_MarginType;
	}

	// Return price margin. May be a percentage 0..100 or an actual number of cronos, depending on PriceMarginType.
	virtual quint32 PriceMarginValue() const
	{
		return m_Margin;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CargoSellCategory;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCargoSellCommodity : public AuMissionOrder
{
private:
	quint32								m_Amount;				// Number of units to sell or percent 1..100.
	quint8								m_AmountType;			// 0=percent, 1=actual count
	quint16								m_CommodityId;
	quint32								m_Margin;				// Minimum price or percent of nominal.
	quint8								m_MarginType;			// 0=min percent of nominal, 1=actual min price, 2=accept any price

public:
	AuMissionOrderCargoSellCommodity(COMMODITYID commodityId, quint32 amount, quint8 amountType, quint32 margin,
			quint8 marginType, EXECTYPE execType = Sequential);
	AuMissionOrderCargoSellCommodity();
	AuMissionOrderCargoSellCommodity(const AuMissionOrderCargoSellCommodity& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_Amount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_AmountType;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoSellCommodity& c = dynamic_cast<const AuMissionOrderCargoSellCommodity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_AmountType == 0)
		{
			if(m_Amount == 0 || m_Amount > 100)
				return false;
		}
		else
		{
			if(m_Amount == 0)
				return false;
		}
		if(m_CommodityId <= CID_Invalid || m_CommodityId >= CID_Count)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCargoSellCommodity& c = dynamic_cast<const AuMissionOrderCargoSellCommodity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Amount != c.m_Amount)
				return false;
			if(m_AmountType != c.m_AmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_Margin != c.m_Margin)
				return false;
			if(m_MarginType != c.m_MarginType)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// 0=percent of nominal, 1=actual price, 2=any price
	virtual quint8 PriceMarginType() const
	{
		return m_MarginType;
	}

	// Return price margin. May be a percentage 0..100 or an actual number of cronos, depending on PriceMarginType.
	virtual quint32 PriceMarginValue() const
	{
		return m_Margin;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CargoSellCommodity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderChristen : public AuMissionOrder
{
private:
	QString								m_ShipName;

public:
	AuMissionOrderChristen(const QString& shipName, EXECTYPE execType = Sequential);
	AuMissionOrderChristen();
	AuMissionOrderChristen(const AuMissionOrderChristen& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderChristen& c = dynamic_cast<const AuMissionOrderChristen&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_ShipName != c.m_ShipName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(!IsNameValid(m_ShipName))
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderChristen& c = dynamic_cast<const AuMissionOrderChristen&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_ShipName != c.m_ShipName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Ship name returned for orders that refer to an atom.
	virtual QString ShipName() const
	{
		return m_ShipName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Christen;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCrewLoad : public AuMissionOrder
{
private:
	quint8								m_Count;				// Number to load. 0=as many as possible.
	quint8								m_PreBoardMinutes;		// Pre boarding announcement lead time, in minutes.

public:
	AuMissionOrderCrewLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType = Sequential);
	AuMissionOrderCrewLoad();
	AuMissionOrderCrewLoad(const AuMissionOrderCrewLoad& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_PreBoardMinutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCrewLoad& c = dynamic_cast<const AuMissionOrderCrewLoad&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCrewLoad& c = dynamic_cast<const AuMissionOrderCrewLoad&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	virtual int PreboardingAnnouncementMinutes() const
	{
		return m_PreBoardMinutes;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CrewLoad;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderDamageReport : public AuMissionOrder
{
private:
	bool								m_NotifyCoC;			// true to notify entire chain-of-command, false for capt/officers.

public:
	AuMissionOrderDamageReport(bool notifyCoC, EXECTYPE execType = Sequential);
	AuMissionOrderDamageReport();
	AuMissionOrderDamageReport(const AuMissionOrderDamageReport& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderDamageReport& c = dynamic_cast<const AuMissionOrderDamageReport&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderDamageReport& c = dynamic_cast<const AuMissionOrderDamageReport&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 5;
	}

	// Return true for report generating orders that are supposed to send that report to their entire chain of command.
	virtual bool ReportToCommand() const
	{
		return m_NotifyCoC;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return DamageReport;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderFollow : public AuMissionOrder
{
private:
	AuId								m_TargetId;
	QString								m_TargetName;

public:
	AuMissionOrderFollow(const AuId& targetId, const QString& targetName, EXECTYPE execType = Sequential);
	AuMissionOrderFollow();
	AuMissionOrderFollow(const AuMissionOrderFollow& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderFollow& c = dynamic_cast<const AuMissionOrderFollow&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_TargetId.IsNull() || m_TargetName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderFollow& c = dynamic_cast<const AuMissionOrderFollow&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual AuId TargetId() const
	{
		return m_TargetId;
	}

	// Target name returned for orders that refer to an atom.
	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Follow;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToBestCity : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToBestCity(EXECTYPE execType = Sequential);
	AuMissionOrderGoToBestCity(const AuMissionOrderGoToBestCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestCity& c = dynamic_cast<const AuMissionOrderGoToBestCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestCity& c = dynamic_cast<const AuMissionOrderGoToBestCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToBestCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToBestStation : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToBestStation(EXECTYPE execType = Sequential);
	AuMissionOrderGoToBestStation(const AuMissionOrderGoToBestStation& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestStation& c = dynamic_cast<const AuMissionOrderGoToBestStation&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestStation& c = dynamic_cast<const AuMissionOrderGoToBestStation&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToBestStation;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToBestStationOrCity : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToBestStationOrCity(EXECTYPE execType = Sequential);
	AuMissionOrderGoToBestStationOrCity(const AuMissionOrderGoToBestStationOrCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestStationOrCity& c = dynamic_cast<const AuMissionOrderGoToBestStationOrCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToBestStationOrCity& c = dynamic_cast<const AuMissionOrderGoToBestStationOrCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToBestStationOrCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToCity : public AuMissionOrder
{
private:
	AuId								m_CityId;
	QString								m_CityName;

public:
	AuMissionOrderGoToCity(const AuId& cityId, const QString& cityName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToCity();
	AuMissionOrderGoToCity(const AuMissionOrderGoToCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_CityId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_CityName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCity& c = dynamic_cast<const AuMissionOrderGoToCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_CityId.IsNull() || m_CityName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCity& c = dynamic_cast<const AuMissionOrderGoToCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToCityAnywhere : public AuMissionOrder
{
private:
	AuId								m_CityId;
	QString								m_CityName;

public:
	AuMissionOrderGoToCityAnywhere(const AuId& cityId, const QString& cityName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToCityAnywhere();
	AuMissionOrderGoToCityAnywhere(const AuMissionOrderGoToCityAnywhere& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_CityId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_CityName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCityAnywhere& c = dynamic_cast<const AuMissionOrderGoToCityAnywhere&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_CityId.IsNull() || m_CityName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCityAnywhere& c = dynamic_cast<const AuMissionOrderGoToCityAnywhere&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToCityAnywhere;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToClosestCity : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToClosestCity(EXECTYPE execType = Sequential);
	AuMissionOrderGoToClosestCity(const AuMissionOrderGoToClosestCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestCity& c = dynamic_cast<const AuMissionOrderGoToClosestCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestCity& c = dynamic_cast<const AuMissionOrderGoToClosestCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToClosestCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToClosestStation : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToClosestStation(EXECTYPE execType = Sequential);
	AuMissionOrderGoToClosestStation(const AuMissionOrderGoToClosestStation& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestStation& c = dynamic_cast<const AuMissionOrderGoToClosestStation&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestStation& c = dynamic_cast<const AuMissionOrderGoToClosestStation&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToClosestStation;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToClosestStationOrCity : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToClosestStationOrCity(EXECTYPE execType = Sequential);
	AuMissionOrderGoToClosestStationOrCity(const AuMissionOrderGoToClosestStationOrCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestStationOrCity& c = dynamic_cast<const AuMissionOrderGoToClosestStationOrCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToClosestStationOrCity& c = dynamic_cast<const AuMissionOrderGoToClosestStationOrCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToClosestStationOrCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToCoordinate : public AuMissionOrder
{
private:
	quint8								m_Context;
	float								m_X;
	float								m_Y;
	float								m_Z;

public:
	AuMissionOrderGoToCoordinate(CONTEXT context, float x, float y, float z, EXECTYPE execType = Sequential);
	AuMissionOrderGoToCoordinate(CONTEXT context, const AuPoint& loc, EXECTYPE execType = Sequential);
	AuMissionOrderGoToCoordinate();
	AuMissionOrderGoToCoordinate(const AuMissionOrderGoToCoordinate& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual CONTEXT DestinationContext() const
	{
		return (CONTEXT)m_Context;
	}

	// Returns coordinate associated with this order.
	virtual AuPoint DestinationCoordinate() const
	{
		return AuPoint(m_X, m_Y, m_Z);
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCoordinate& c = dynamic_cast<const AuMissionOrderGoToCoordinate&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Context != c.m_Context)
				return false;
			if(m_X != c.m_X)
				return false;
			if(m_Y != c.m_Y)
				return false;
			if(m_Z != c.m_Z)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsStarTravelOrder() const
	{
		return DestinationContext() != SolarSystem;	// Coordinate relative to system is not usually star travel, but could be.
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_Context != Galaxy && m_Context != Sector && m_Context != SolarSystem)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToCoordinate& c = dynamic_cast<const AuMissionOrderGoToCoordinate&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Context != c.m_Context)
				return false;
			if(m_X != c.m_X)
				return false;
			if(m_Y != c.m_Y)
				return false;
			if(m_Z != c.m_Z)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToCoordinate;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToHoverSite : public AuMissionOrder
{
private:
	quint16								m_AltitudeAGL;			// Altitude in meters above ground/sea level, whichever is higher.
	AuId								m_BodyId;				// Could be a world or a star id or null.
	QString								m_BodyName;
	AuGlobeLLF							m_GlobeLL;

public:
	AuMissionOrderGoToHoverSite(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL, quint16 altitudeAGL,
			EXECTYPE execType = Sequential);
	AuMissionOrderGoToHoverSite();
	AuMissionOrderGoToHoverSite(const AuMissionOrderGoToHoverSite& c);

public:	// Operators
	const AuMissionOrderGoToHoverSite& operator=(const AuMissionOrderGoToHoverSite& c);

public:	// Methods
	// Return altitude in meters.
	virtual quint16 Altitude() const
	{
		return m_AltitudeAGL;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_BodyId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_BodyName;
	}

	// Returns coordinate associated with this order.
	virtual AuGlobeLLF GlobeLL() const
	{
		return m_GlobeLL;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToHoverSite& c = dynamic_cast<const AuMissionOrderGoToHoverSite&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AltitudeAGL != c.m_AltitudeAGL)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_AltitudeAGL == 0)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToHoverSite& c = dynamic_cast<const AuMissionOrderGoToHoverSite&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AltitudeAGL != c.m_AltitudeAGL)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToHoverSite;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToLand : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Could be a world.
	QString								m_BodyName;

public:
	AuMissionOrderGoToLand(const AuId& bodyId, const QString& bodyName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToLand();
	AuMissionOrderGoToLand(const AuMissionOrderGoToLand& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_BodyId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToLand& c = dynamic_cast<const AuMissionOrderGoToLand&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToLand& c = dynamic_cast<const AuMissionOrderGoToLand&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToLand;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToLandingSite : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Could be a world or null.
	QString								m_BodyName;
	AuGlobeLLF							m_GlobeLL;

public:
	AuMissionOrderGoToLandingSite(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL,
			EXECTYPE execType = Sequential);
	AuMissionOrderGoToLandingSite();
	AuMissionOrderGoToLandingSite(const AuMissionOrderGoToLandingSite& c);

public:	// Operators
	const AuMissionOrderGoToLandingSite& operator=(const AuMissionOrderGoToLandingSite& c);

public:	// Methods
	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_BodyId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_BodyName;
	}

	// Returns coordinate associated with this order.
	virtual AuGlobeLLF GlobeLL() const
	{
		return m_GlobeLL;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToLandingSite& c = dynamic_cast<const AuMissionOrderGoToLandingSite&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToLandingSite& c = dynamic_cast<const AuMissionOrderGoToLandingSite&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToLandingSite;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToOrbit : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Could be a world or a star id or null.
	QString								m_BodyName;

public:
	AuMissionOrderGoToOrbit(const AuId& bodyId, const QString& bodyName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToOrbit();
	AuMissionOrderGoToOrbit(const AuMissionOrderGoToOrbit& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_BodyId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbit& c = dynamic_cast<const AuMissionOrderGoToOrbit&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbit& c = dynamic_cast<const AuMissionOrderGoToOrbit&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToOrbit;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToOrbitBestPlanet : public AuMissionOrder
{
private:

public:
	AuMissionOrderGoToOrbitBestPlanet(EXECTYPE execType = Sequential);
	AuMissionOrderGoToOrbitBestPlanet(const AuMissionOrderGoToOrbitBestPlanet& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitBestPlanet& c = dynamic_cast<const AuMissionOrderGoToOrbitBestPlanet&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitBestPlanet& c = dynamic_cast<const AuMissionOrderGoToOrbitBestPlanet&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToOrbitBestPlanet;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToOrbitCity : public AuMissionOrder
{
private:
	AuId								m_CityId;
	QString								m_CityName;

public:
	AuMissionOrderGoToOrbitCity(const AuId& cityId, const QString& cityName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToOrbitCity();
	AuMissionOrderGoToOrbitCity(const AuMissionOrderGoToOrbitCity& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_CityId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_CityName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitCity& c = dynamic_cast<const AuMissionOrderGoToOrbitCity&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_CityId.IsNull() || m_CityName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitCity& c = dynamic_cast<const AuMissionOrderGoToOrbitCity&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_CityId != c.m_CityId)
				return false;
			if(m_CityName != c.m_CityName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToOrbitCity;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToOrbitCoords : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Could be a world or a star id.
	QString								m_BodyName;
	AuGlobeLLF							m_GlobeLL;

public:
	AuMissionOrderGoToOrbitCoords(const AuId& bodyId, const QString& bodyName, const AuGlobeLLF& globeLL,
			EXECTYPE execType = Sequential);
	AuMissionOrderGoToOrbitCoords();
	AuMissionOrderGoToOrbitCoords(const AuMissionOrderGoToOrbitCoords& c);

public:	// Operators
	const AuMissionOrderGoToOrbitCoords& operator=(const AuMissionOrderGoToOrbitCoords& c);

public:	// Methods

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_BodyId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_BodyName;
	}

	// Returns coordinate associated with this order.
	virtual AuGlobeLLF GlobeLL() const
	{
		return m_GlobeLL;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitCoords& c = dynamic_cast<const AuMissionOrderGoToOrbitCoords&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToOrbitCoords& c = dynamic_cast<const AuMissionOrderGoToOrbitCoords&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_GlobeLL != c.m_GlobeLL)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToOrbitCoords;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToStation : public AuMissionOrder
{
private:
	AuId								m_StationId;
	QString								m_StationName;

public:
	AuMissionOrderGoToStation(const AuId& stationId, const QString& stationName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToStation();
	AuMissionOrderGoToStation(const AuMissionOrderGoToStation& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_StationId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_StationName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToStation& c = dynamic_cast<const AuMissionOrderGoToStation&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_StationId != c.m_StationId)
				return false;
			if(m_StationName != c.m_StationName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_StationId.IsNull() || m_StationName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToStation& c = dynamic_cast<const AuMissionOrderGoToStation&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_StationId != c.m_StationId)
				return false;
			if(m_StationName != c.m_StationName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToStation;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToSystemDirect_Deprecated : public AuMissionOrder
{
private:
	AuId								m_SystemId;
	QString								m_SystemName;

public:
	AuMissionOrderGoToSystemDirect_Deprecated(const AuId& systemId, const QString& systemName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToSystemDirect_Deprecated();
	AuMissionOrderGoToSystemDirect_Deprecated(const AuMissionOrderGoToSystemDirect_Deprecated& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_SystemId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_SystemName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemDirect_Deprecated& c =
				dynamic_cast<const AuMissionOrderGoToSystemDirect_Deprecated&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsStarTravelOrder() const
	{
		return true;
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_SystemId.IsNull() || m_SystemName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemDirect_Deprecated& c =
				dynamic_cast<const AuMissionOrderGoToSystemDirect_Deprecated&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToSystemDirect_Deprecated;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Nu version of travel direct order includes the galaxy coordinate.

class AUGLOBAL_DLL AuMissionOrderGoToSystemDirect : public AuMissionOrder
{
private:
	AuId								m_SystemId;
	QString								m_SystemName;
	float								m_X;					// Galaxy coord of destination system.
	float								m_Y;
	float								m_Z;

public:
	AuMissionOrderGoToSystemDirect(const AuId& systemId, const QString& systemName, float x, float y, float z,
			EXECTYPE execType = Sequential);
	AuMissionOrderGoToSystemDirect(const AuId& systemId, const QString& systemName, const AuPoint& galaxyLoc,
			EXECTYPE execType = Sequential);
	AuMissionOrderGoToSystemDirect();
	AuMissionOrderGoToSystemDirect(const AuMissionOrderGoToSystemDirect& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual CONTEXT DestinationContext() const
	{
		return Galaxy;
	}

	virtual AuPoint DestinationCoordinate() const
	{
		return AuPoint(m_X, m_Y, m_Z);
	}

	virtual AuId DestinationId() const
	{
		return m_SystemId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_SystemName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemDirect& c = dynamic_cast<const AuMissionOrderGoToSystemDirect&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			if(m_X != c.m_X)
				return false;
			if(m_Y != c.m_Y)
				return false;
			if(m_Z != c.m_Z)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsStarTravelOrder() const
	{
		return true;
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_SystemId.IsNull() || m_SystemName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemDirect& c = dynamic_cast<const AuMissionOrderGoToSystemDirect&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			if(m_X != c.m_X)
				return false;
			if(m_Y != c.m_Y)
				return false;
			if(m_Z != c.m_Z)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToSystemDirect;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderGoToSystemViaWormholes : public AuMissionOrder
{
private:
	AuId								m_SystemId;
	QString								m_SystemName;

public:
	AuMissionOrderGoToSystemViaWormholes(const AuId& systemId, const QString& systemName, EXECTYPE execType = Sequential);
	AuMissionOrderGoToSystemViaWormholes();
	AuMissionOrderGoToSystemViaWormholes(const AuMissionOrderGoToSystemViaWormholes& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual AuId DestinationId() const
	{
		return m_SystemId;
	}

	// Destination name returned for orders that refer to an atom.
	virtual QString DestinationName() const
	{
		return m_SystemName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemViaWormholes& c = dynamic_cast<const AuMissionOrderGoToSystemViaWormholes&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsStarTravelOrder() const
	{
		return true;
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	virtual bool IsValid() const
	{
		if(m_SystemId.IsNull() || m_SystemName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderGoToSystemViaWormholes& c = dynamic_cast<const AuMissionOrderGoToSystemViaWormholes&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_SystemId != c.m_SystemId)
				return false;
			if(m_SystemName != c.m_SystemName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return GoToSystemViaWormholes;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderHarvest : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderHarvest(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderHarvest();
	AuMissionOrderHarvest(const AuMissionOrderHarvest& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvest& c = dynamic_cast<const AuMissionOrderHarvest&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvest& c = dynamic_cast<const AuMissionOrderHarvest&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Harvest;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Configures harvester bay(s) to harvest a specific commodity.

class AUGLOBAL_DLL AuMissionOrderHarvestAtmosphere : public AuMissionOrder
{
private:
	bool								m_AllBays;				// True to configure all bays vs specified m_WhichBay.
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint16								m_CommodityId;			// Commodity to configure bay for harvesting.
	quint8								m_WhichBay;				// Harvester bay to configure.

public:
	AuMissionOrderHarvestAtmosphere(COMMODITYID commodityId, bool allBays, quint8 whichBay, const AuId& bodyId,
			const QString& bodyName, EXECTYPE execType = Sequential);
	AuMissionOrderHarvestAtmosphere();
	AuMissionOrderHarvestAtmosphere(const AuMissionOrderHarvestAtmosphere& c);

	virtual bool AllBays() const
	{
		return m_AllBays;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestAtmosphere& c = dynamic_cast<const AuMissionOrderHarvestAtmosphere&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_CommodityId != CID_Air &&
				m_CommodityId != CID_Cryozine &&
				m_CommodityId != CID_Hydrogen &&
				m_CommodityId != CID_Ioplasma)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestAtmosphere& c = dynamic_cast<const AuMissionOrderHarvestAtmosphere&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestAtmosphere;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return m_WhichBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Configures harvester bay(s) to harvest a specific commodity.

class AUGLOBAL_DLL AuMissionOrderHarvestMinerals : public AuMissionOrder
{
private:
	bool								m_AllBays;				// True to configure all bays vs specified m_WhichBay.
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint16								m_CommodityId;			// Commodity to configure bay for harvesting.
	quint8								m_WhichBay;				// Harvester bay to configure.

public:
	AuMissionOrderHarvestMinerals(COMMODITYID commodityId, bool allBays, quint8 whichBay, const AuId& bodyId,
			const QString& bodyName, EXECTYPE execType = Sequential);
	AuMissionOrderHarvestMinerals();
	AuMissionOrderHarvestMinerals(const AuMissionOrderHarvestMinerals& c);

	virtual bool AllBays() const
	{
		return m_AllBays;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestMinerals& c = dynamic_cast<const AuMissionOrderHarvestMinerals&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const;

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestMinerals& c = dynamic_cast<const AuMissionOrderHarvestMinerals&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestMinerals;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return m_WhichBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Configures harvester bay(s) to harvest a specific commodity.

class AUGLOBAL_DLL AuMissionOrderHarvestParticles : public AuMissionOrder
{
private:
	bool								m_AllBays;				// True to configure all bays vs specified m_WhichBay.
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint16								m_CommodityId;			// Commodity to configure bay for harvesting.
	quint8								m_WhichBay;				// Harvester bay to configure.

public:
	AuMissionOrderHarvestParticles(COMMODITYID commodityId, bool allBays, quint8 whichBay, const AuId& bodyId,
			const QString& bodyName, EXECTYPE execType = Sequential);
	AuMissionOrderHarvestParticles();
	AuMissionOrderHarvestParticles(const AuMissionOrderHarvestParticles& c);

	virtual bool AllBays() const
	{
		return m_AllBays;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestParticles& c = dynamic_cast<const AuMissionOrderHarvestParticles&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_CommodityId != CID_AntifluxParticles &&
				m_CommodityId != CID_BorexinoPrecipitate &&
				m_CommodityId != CID_TypeOPreons &&
				m_CommodityId != CID_TypeBPreons &&
				m_CommodityId != CID_TypeAPreons &&
				m_CommodityId != CID_TypeFPreons &&
				m_CommodityId != CID_TypeGPreons &&
				m_CommodityId != CID_TypeKPreons &&
				m_CommodityId != CID_TypeMPreons)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestParticles& c = dynamic_cast<const AuMissionOrderHarvestParticles&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestParticles;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return m_WhichBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Configures harvester bay(s) to harvest a specific commodity.

class AUGLOBAL_DLL AuMissionOrderHarvestSea : public AuMissionOrder
{
private:
	bool								m_AllBays;				// True to configure all bays vs specified m_WhichBay.
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint16								m_CommodityId;			// Commodity to configure bay for harvesting.
	quint8								m_WhichBay;				// Harvester bay to configure.

public:
	AuMissionOrderHarvestSea(COMMODITYID commodityId, bool allBays, quint8 whichBay, const AuId& bodyId, const QString& bodyName,
			EXECTYPE execType = Sequential);
	AuMissionOrderHarvestSea();
	AuMissionOrderHarvestSea(const AuMissionOrderHarvestSea& c);

	virtual bool AllBays() const
	{
		return m_AllBays;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestSea& c = dynamic_cast<const AuMissionOrderHarvestSea&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_CommodityId != CID_Magmex &&
				m_CommodityId != CID_Myrathane &&
				m_CommodityId != CID_Flomentum &&
				m_CommodityId != CID_Water)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestSea& c = dynamic_cast<const AuMissionOrderHarvestSea&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestSea;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return m_WhichBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Configures harvester bay(s) to harvest a specific commodity.

class AUGLOBAL_DLL AuMissionOrderHarvestWell : public AuMissionOrder
{
private:
	bool								m_AllBays;				// True to configure all bays vs specified m_WhichBay.
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint16								m_CommodityId;			// Commodity to configure bay for harvesting.
	quint8								m_WhichBay;				// Harvester bay to configure.

public:
	AuMissionOrderHarvestWell(COMMODITYID commodityId, bool allBays, quint8 whichBay, const AuId& bodyId, const QString& bodyName,
			EXECTYPE execType = Sequential);
	AuMissionOrderHarvestWell();
	AuMissionOrderHarvestWell(const AuMissionOrderHarvestWell& c);

	virtual bool AllBays() const
	{
		return m_AllBays;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestWell& c = dynamic_cast<const AuMissionOrderHarvestWell&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_CommodityId != CID_NaturalGas &&
				m_CommodityId != CID_CrudeOil &&
				m_CommodityId != CID_Phlogiston &&
				m_CommodityId != CID_Polytaride &&
				m_CommodityId != CID_Viathol &&
				m_CommodityId != CID_Water)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestWell& c = dynamic_cast<const AuMissionOrderHarvestWell&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_AllBays != c.m_AllBays)
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_WhichBay != c.m_WhichBay)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestWell;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;

	// Returns bay 'which' identifier.
	virtual quint8 WhichBay() const
	{
		return m_WhichBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderHide : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderHide(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderHide();
	AuMissionOrderHide(const AuMissionOrderHide& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHide& c = dynamic_cast<const AuMissionOrderHide&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHide& c = dynamic_cast<const AuMissionOrderHide&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Hide;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderHoldMission : public AuMissionOrder
{
private:

public:
	AuMissionOrderHoldMission(EXECTYPE execType = Sequential);
	AuMissionOrderHoldMission(const AuMissionOrderHoldMission& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHoldMission& c = dynamic_cast<const AuMissionOrderHoldMission&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHoldMission& c = dynamic_cast<const AuMissionOrderHoldMission&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HoldMission;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderLightState : public AuMissionOrder
{
private:
	bool								m_Headlight;
	bool								m_NavLight;
	bool								m_RotatingBeacon;
	bool								m_Strobe;

public:
	AuMissionOrderLightState(bool headlight, bool navLight, bool beacon, bool strobe, EXECTYPE execType = Sequential);
	AuMissionOrderLightState();
	AuMissionOrderLightState(const AuMissionOrderLightState& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual bool HeadlightOn() const
	{
		return m_Headlight;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderLightState& c = dynamic_cast<const AuMissionOrderLightState&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Headlight != c.m_Headlight)
				return false;
			if(m_NavLight != c.m_NavLight)
				return false;
			if(m_RotatingBeacon != c.m_RotatingBeacon)
				return false;
			if(m_Strobe != c.m_Strobe)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual bool NavLightOn() const
	{
		return m_NavLight;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderLightState& c = dynamic_cast<const AuMissionOrderLightState&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Headlight != c.m_Headlight)
				return false;
			if(m_NavLight != c.m_NavLight)
				return false;
			if(m_RotatingBeacon != c.m_RotatingBeacon)
				return false;
			if(m_Strobe != c.m_Strobe)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual bool RotatingBeaconOn() const
	{
		return m_RotatingBeacon;
	}

	virtual bool StrobeOn() const
	{
		return m_Strobe;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return LightState;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPassengersLoad : public AuMissionOrder
{
private:
	quint8								m_Count;				// Number to load. 0=as many as possible.
	quint8								m_PreBoardMinutes;		// Pre boarding announcement lead time, in minutes.

public:
	AuMissionOrderPassengersLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType = Sequential);
	AuMissionOrderPassengersLoad();
	AuMissionOrderPassengersLoad(const AuMissionOrderPassengersLoad& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_PreBoardMinutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPassengersLoad& c = dynamic_cast<const AuMissionOrderPassengersLoad&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPassengersLoad& c = dynamic_cast<const AuMissionOrderPassengersLoad&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	virtual int PreboardingAnnouncementMinutes() const
	{
		return m_PreBoardMinutes;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PassengersLoad;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPassengersUnload : public AuMissionOrder
{
private:
	quint8								m_Count;				// Number to unload. 0=as many as possible.

public:
	AuMissionOrderPassengersUnload(quint8 count, EXECTYPE execType = Sequential);
	AuMissionOrderPassengersUnload();
	AuMissionOrderPassengersUnload(const AuMissionOrderPassengersUnload& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPassengersUnload& c = dynamic_cast<const AuMissionOrderPassengersUnload&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPassengersUnload& c = dynamic_cast<const AuMissionOrderPassengersUnload&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PassengersUnload;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCryoBerthsLoad : public AuMissionOrder
{
private:
	quint32								m_Count;				// Number to load. 0=as many as possible.

public:
	AuMissionOrderCryoBerthsLoad(quint32 count, EXECTYPE execType = Sequential);
	AuMissionOrderCryoBerthsLoad();
	AuMissionOrderCryoBerthsLoad(const AuMissionOrderCryoBerthsLoad& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCryoBerthsLoad& c = dynamic_cast<const AuMissionOrderCryoBerthsLoad&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCryoBerthsLoad& c = dynamic_cast<const AuMissionOrderCryoBerthsLoad&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CryoBerthsLoad;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderCryoBerthsUnload : public AuMissionOrder
{
private:
	quint32								m_Count;				// Number to unload. 0=as many as possible.

public:
	AuMissionOrderCryoBerthsUnload(quint32 count, EXECTYPE execType = Sequential);
	AuMissionOrderCryoBerthsUnload();
	AuMissionOrderCryoBerthsUnload(const AuMissionOrderCryoBerthsUnload& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCryoBerthsUnload& c = dynamic_cast<const AuMissionOrderCryoBerthsUnload&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderCryoBerthsUnload& c = dynamic_cast<const AuMissionOrderCryoBerthsUnload&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return CryoBerthsUnload;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPatrolCities : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderPatrolCities(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderPatrolCities();
	AuMissionOrderPatrolCities(const AuMissionOrderPatrolCities& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Patrol orders return true.
	virtual bool IsPatrol() const
	{
		return true;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolCities& c = dynamic_cast<const AuMissionOrderPatrolCities&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolCities& c = dynamic_cast<const AuMissionOrderPatrolCities&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PatrolCities;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPatrolStations : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderPatrolStations(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderPatrolStations();
	AuMissionOrderPatrolStations(const AuMissionOrderPatrolStations& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Patrol orders return true.
	virtual bool IsPatrol() const
	{
		return true;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolStations& c = dynamic_cast<const AuMissionOrderPatrolStations&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolStations& c = dynamic_cast<const AuMissionOrderPatrolStations&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PatrolStations;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPatrolSystem : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderPatrolSystem(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderPatrolSystem();
	AuMissionOrderPatrolSystem(const AuMissionOrderPatrolSystem& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Patrol orders return true.
	virtual bool IsPatrol() const
	{
		return true;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolSystem& c = dynamic_cast<const AuMissionOrderPatrolSystem&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolSystem& c = dynamic_cast<const AuMissionOrderPatrolSystem&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PatrolSystem;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPatrolWormholes : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderPatrolWormholes(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderPatrolWormholes();
	AuMissionOrderPatrolWormholes(const AuMissionOrderPatrolWormholes& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Patrol orders return true.
	virtual bool IsPatrol() const
	{
		return true;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolWormholes& c = dynamic_cast<const AuMissionOrderPatrolWormholes&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsTravelOrder() const
	{
		return true;
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPatrolWormholes& c = dynamic_cast<const AuMissionOrderPatrolWormholes&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PatrolWormholes;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPlunder : public AuMissionOrder
{
private:
	AuId								m_TargetId;
	QString								m_TargetName;

public:
	AuMissionOrderPlunder(const AuId& targetId, const QString& targetName, EXECTYPE execType = Sequential);
	AuMissionOrderPlunder();
	AuMissionOrderPlunder(const AuMissionOrderPlunder& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPlunder& c = dynamic_cast<const AuMissionOrderPlunder&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_TargetId.IsNull() || m_TargetName.isEmpty())
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPlunder& c = dynamic_cast<const AuMissionOrderPlunder&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_TargetId != c.m_TargetId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual AuId TargetId() const
	{
		return m_TargetId;
	}

	// Target name returned for orders that refer to an atom.
	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Plunder;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPosture : public AuMissionOrder
{
private:
	quint8								m_Posture;				// CITIZEN_POSTURE enum value

public:
	AuMissionOrderPosture(quint8 posture, EXECTYPE execType = Sequential);
	AuMissionOrderPosture();
	AuMissionOrderPosture(const AuMissionOrderPosture& c);

	virtual CITIZEN_POSTURE CitizenPosture() const
	{
		return (CITIZEN_POSTURE)m_Posture;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPosture& c = dynamic_cast<const AuMissionOrderPosture&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Posture != c.m_Posture)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Posture > POSTURE_HoldFire)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPosture& c = dynamic_cast<const AuMissionOrderPosture&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Posture != c.m_Posture)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Posture;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderRepair : public AuMissionOrder
{
private:
	quint8								m_ApplyQ;				// What minQ to apply
	quint8								m_MinQ;					// minimum Q when m_ApplyQ == 2

public:
	// pass minQ=0 to use the manufacture quality of the ship as the minimum.
	AuMissionOrderRepair(int applyMinQ, quint8 minQ, EXECTYPE execType = Sequential);
	AuMissionOrderRepair(EXECTYPE execType = Sequential);
	AuMissionOrderRepair(const AuMissionOrderRepair& c);

	// Returns 0=use system Q and repair by system, 1=use ship manufacture Q, 2=use min Q.
	virtual int ApplyMinQ() const
	{
		return m_ApplyQ;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderRepair& c = dynamic_cast<const AuMissionOrderRepair&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_MinQ != c.m_MinQ)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint8 MinQ() const
	{
		return m_MinQ;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderRepair& c = dynamic_cast<const AuMissionOrderRepair&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_MinQ != c.m_MinQ)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 5;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Repair;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderScanSurveyPlanet : public AuMissionOrder
{
private:
	bool								m_NotifyCoC;			// true to notify entire chain-of-command, false for capt/officers.

public:
	AuMissionOrderScanSurveyPlanet(bool notifyCoC, EXECTYPE execType = Sequential);
	AuMissionOrderScanSurveyPlanet();
	AuMissionOrderScanSurveyPlanet(const AuMissionOrderScanSurveyPlanet& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanSurveyPlanet& c = dynamic_cast<const AuMissionOrderScanSurveyPlanet&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanSurveyPlanet& c = dynamic_cast<const AuMissionOrderScanSurveyPlanet&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 5;
	}

	// Return true for report generating orders that are supposed to send that report to their entire chain of command.
	virtual bool ReportToCommand() const
	{
		return m_NotifyCoC;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ScanSurveyPlanet;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderScanSurveySystem : public AuMissionOrder
{
private:
	bool								m_NotifyCoC;			// true to notify entire chain-of-command, false for capt/officers.

public:
	AuMissionOrderScanSurveySystem(bool notifyCoC, EXECTYPE execType = Sequential);
	AuMissionOrderScanSurveySystem();
	AuMissionOrderScanSurveySystem(const AuMissionOrderScanSurveySystem& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanSurveySystem& c = dynamic_cast<const AuMissionOrderScanSurveySystem&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanSurveySystem& c = dynamic_cast<const AuMissionOrderScanSurveySystem&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 9;
	}

	// Return true for report generating orders that are supposed to send that report to their entire chain of command.
	virtual bool ReportToCommand() const
	{
		return m_NotifyCoC;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ScanSurveySystem;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderScanTactical : public AuMissionOrder
{
private:
	bool								m_NotifyCoC;			// true to notify entire chain-of-command, false for capt/officers.

public:
	AuMissionOrderScanTactical(bool notifyCoC, EXECTYPE execType = Sequential);
	AuMissionOrderScanTactical();
	AuMissionOrderScanTactical(const AuMissionOrderScanTactical& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanTactical& c = dynamic_cast<const AuMissionOrderScanTactical&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScanTactical& c = dynamic_cast<const AuMissionOrderScanTactical&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_NotifyCoC != c.m_NotifyCoC)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 7;
	}

	// Return true for report generating orders that are supposed to send that report to their entire chain of command.
	virtual bool ReportToCommand() const
	{
		return m_NotifyCoC;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ScanTactical;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderScuttle : public AuMissionOrder
{
private:
	quint16								m_Seconds;

public:
	AuMissionOrderScuttle(quint16 seconds, EXECTYPE execType = Sequential);
	AuMissionOrderScuttle();
	AuMissionOrderScuttle(const AuMissionOrderScuttle& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Seconds;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScuttle& c = dynamic_cast<const AuMissionOrderScuttle&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Seconds != c.m_Seconds)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderScuttle& c = dynamic_cast<const AuMissionOrderScuttle&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Seconds != c.m_Seconds)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Scuttle;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderShieldPolicy : public AuMissionOrder
{
private:
	quint8								m_ShieldFuelPolicy;			// SHIELDFUEL_POLICY enum value
	quint8								m_ShieldPolicy;				// SHIELD_POLICY enum value

public:
	AuMissionOrderShieldPolicy(SHIELD_POLICY shieldPolicy, SHIELDFUEL_POLICY shieldFuelPolicy, EXECTYPE execType = Sequential);
	AuMissionOrderShieldPolicy();
	AuMissionOrderShieldPolicy(const AuMissionOrderShieldPolicy& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderShieldPolicy& c = dynamic_cast<const AuMissionOrderShieldPolicy&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_ShieldFuelPolicy != c.m_ShieldFuelPolicy)
				return false;
			if(m_ShieldPolicy != c.m_ShieldPolicy)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_ShieldFuelPolicy > 1)
			return false;
		if(m_ShieldPolicy > 5)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderShieldPolicy& c = dynamic_cast<const AuMissionOrderShieldPolicy&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_ShieldFuelPolicy != c.m_ShieldFuelPolicy)
				return false;
			if(m_ShieldPolicy != c.m_ShieldPolicy)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual SHIELDFUEL_POLICY ShipShieldFuelPolicy() const
	{
		return (SHIELDFUEL_POLICY)m_ShieldFuelPolicy;
	}

	virtual SHIELD_POLICY ShipShieldPolicy() const
	{
		return (SHIELD_POLICY)m_ShieldPolicy;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ShieldPolicy;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderPilotPolicy : public AuMissionOrder
{
private:
	quint8								m_PilotUpPolicy;		// PILOT_UP_POLICY enum value

public:
	AuMissionOrderPilotPolicy(PILOT_UP_POLICY upPolicy, EXECTYPE execType = Sequential);
	AuMissionOrderPilotPolicy();
	AuMissionOrderPilotPolicy(const AuMissionOrderPilotPolicy& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPilotPolicy& c = dynamic_cast<const AuMissionOrderPilotPolicy&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_PilotUpPolicy != c.m_PilotUpPolicy)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_PilotUpPolicy >= PILOT_UP_POLICY_Count)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderPilotPolicy& c = dynamic_cast<const AuMissionOrderPilotPolicy&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_PilotUpPolicy != c.m_PilotUpPolicy)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	virtual PILOT_UP_POLICY PilotUpPolicy() const
	{
		return (PILOT_UP_POLICY)m_PilotUpPolicy;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return PilotPolicy;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderShieldsDown : public AuMissionOrder
{
private:
	quint16								m_Seconds;				// 0=indefinitely

public:
	AuMissionOrderShieldsDown(quint16 seconds, EXECTYPE execType = Sequential);
	AuMissionOrderShieldsDown();
	AuMissionOrderShieldsDown(const AuMissionOrderShieldsDown& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Seconds)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Seconds;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderShieldsDown& c = dynamic_cast<const AuMissionOrderShieldsDown&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Seconds != c.m_Seconds)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderShieldsDown& c = dynamic_cast<const AuMissionOrderShieldsDown&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Seconds != c.m_Seconds)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ShieldsDown;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderStandDown : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderStandDown(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderStandDown();
	AuMissionOrderStandDown(const AuMissionOrderStandDown& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderStandDown& c = dynamic_cast<const AuMissionOrderStandDown&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderStandDown& c = dynamic_cast<const AuMissionOrderStandDown&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return StandDown;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderStay : public AuMissionOrder
{
private:
	quint16								m_Minutes;				// 0=indefinitely

public:
	AuMissionOrderStay(quint16 minutes, EXECTYPE execType = Sequential);
	AuMissionOrderStay();
	AuMissionOrderStay(const AuMissionOrderStay& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		if(!m_Minutes)
			return false;
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_Minutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderStay& c = dynamic_cast<const AuMissionOrderStay&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual quint16 Minutes() const
	{
		return m_Minutes;
	}

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderStay& c = dynamic_cast<const AuMissionOrderStay&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Minutes != c.m_Minutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 1;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Stay;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTroopsLoad : public AuMissionOrder
{
private:
	quint8								m_Count;				// Number to load. 0=as many as possible.
	quint8								m_PreBoardMinutes;		// Pre boarding announcement lead time, in minutes.

public:
	AuMissionOrderTroopsLoad(quint8 count, quint8 preBoardMinutes, EXECTYPE execType = Sequential);
	AuMissionOrderTroopsLoad();
	AuMissionOrderTroopsLoad(const AuMissionOrderTroopsLoad& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Returns the berth count, depending on the command.
	virtual quint32 HeadCount() const
	{
		return m_Count;
	}

	// For orders that have a time limit, like Stay and Stand Down.
	virtual bool IsExpired(const AuTime& orderStarted) const
	{
		int elapsedSec = orderStarted.ElapsedSec();
		return elapsedSec > m_PreBoardMinutes * 60;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTroopsLoad& c = dynamic_cast<const AuMissionOrderTroopsLoad&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTroopsLoad& c = dynamic_cast<const AuMissionOrderTroopsLoad&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Count != c.m_Count)
				return false;
			if(m_PreBoardMinutes != c.m_PreBoardMinutes)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	virtual int PreboardingAnnouncementMinutes() const
	{
		return m_PreBoardMinutes;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TroopsLoad;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderExterminateRats : public AuMissionOrder
{
public:
	AuMissionOrderExterminateRats(EXECTYPE execType = Sequential);
	//AuMissionOrderExterminateRats();
	AuMissionOrderExterminateRats(const AuMissionOrderExterminateRats& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderExterminateRats& c = dynamic_cast<const AuMissionOrderExterminateRats&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderExterminateRats& c = dynamic_cast<const AuMissionOrderExterminateRats&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return ExterminateRats;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferCargoFrom : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint32								m_CommodityAmount;		// Number of units or percent 1..100.
	quint8								m_CommodityAmountType;	// 0=percent, 1=actual count
	quint16								m_CommodityId;
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferCargoFrom(const QString& targetName, const AuId& auth, COMMODITYID commodityId, quint32 commodityAmount,
			quint8 commodityAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferCargoFrom();
	AuMissionOrderTransferCargoFrom(const AuMissionOrderTransferCargoFrom& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_CommodityAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_CommodityAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoFrom& c = dynamic_cast<const AuMissionOrderTransferCargoFrom&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(m_CommodityId == CID_Invalid)
			return false;
		if(!m_CommodityAmount)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoFrom& c = dynamic_cast<const AuMissionOrderTransferCargoFrom&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferCargoFrom;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferCargoTo : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint32								m_CommodityAmount;		// Number of units or percent 1..100.
	quint8								m_CommodityAmountType;	// 0=percent, 1=actual count
	quint16								m_CommodityId;
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferCargoTo(const QString& targetName, const AuId& auth, COMMODITYID commodityId, quint32 commodityAmount,
			quint8 commodityAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferCargoTo();
	AuMissionOrderTransferCargoTo(const AuMissionOrderTransferCargoTo& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_CommodityAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_CommodityAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CommodityId;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoTo& c = dynamic_cast<const AuMissionOrderTransferCargoTo&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(m_CommodityId == CID_Invalid)
			return false;
		if(!m_CommodityAmount)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoTo& c = dynamic_cast<const AuMissionOrderTransferCargoTo&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityId != c.m_CommodityId)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferCargoTo;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferCargoCategoryFrom : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint32								m_CommodityAmount;		// Number of units or percent 1..100.
	quint8								m_CommodityAmountType;	// 0=percent, 1=actual count, 2=each count
	quint8								m_CommodityFilter;
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferCargoCategoryFrom(const QString& targetName, const AuId& auth, COMMODITYFILTER commodityFilter,
			quint32 commodityAmount, quint8 commodityAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferCargoCategoryFrom();
	AuMissionOrderTransferCargoCategoryFrom(const AuMissionOrderTransferCargoCategoryFrom& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_CommodityAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_CommodityAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYFILTER CommodityFilter() const
	{
		return (COMMODITYFILTER)m_CommodityFilter;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoCategoryFrom& c = dynamic_cast<const AuMissionOrderTransferCargoCategoryFrom&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(!DbCommodity::IsFilterValid((COMMODITYFILTER)m_CommodityFilter))
			return false;
		if(!m_CommodityAmount)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoCategoryFrom& c = dynamic_cast<const AuMissionOrderTransferCargoCategoryFrom&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferCargoCategoryFrom;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferCargoCategoryTo : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint32								m_CommodityAmount;		// Number of units or percent 1..100.
	quint8								m_CommodityAmountType;	// 0=percent, 1=actual count, 2=each count
	quint8								m_CommodityFilter;
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferCargoCategoryTo(const QString& targetName, const AuId& auth, COMMODITYFILTER commodityFilter,
			quint32 commodityAmount, quint8 commodityAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferCargoCategoryTo();
	AuMissionOrderTransferCargoCategoryTo(const AuMissionOrderTransferCargoCategoryTo& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_CommodityAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_CommodityAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual COMMODITYFILTER CommodityFilter() const
	{
		return (COMMODITYFILTER)m_CommodityFilter;
	}

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoCategoryTo& c = dynamic_cast<const AuMissionOrderTransferCargoCategoryTo&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(!DbCommodity::IsFilterValid((COMMODITYFILTER)m_CommodityFilter))
			return false;
		if(!m_CommodityAmount)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferCargoCategoryTo& c = dynamic_cast<const AuMissionOrderTransferCargoCategoryTo&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_CommodityAmount != c.m_CommodityAmount)
				return false;
			if(m_CommodityAmountType != c.m_CommodityAmountType)
				return false;
			if(m_CommodityFilter != c.m_CommodityFilter)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferCargoCategoryTo;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferMoneyFrom : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	double								m_MoneyAmount;
	quint8								m_MoneyAmountType;		// 0=percent, 1=actual count
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferMoneyFrom(const QString& targetName, const AuId& auth, double moneyAmount, quint8 moneyAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferMoneyFrom();
	AuMissionOrderTransferMoneyFrom(const AuMissionOrderTransferMoneyFrom& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return (quint32)m_MoneyAmount;
	}

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual double AmountOfMoney() const
	{
		return m_MoneyAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_MoneyAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferMoneyFrom& c = dynamic_cast<const AuMissionOrderTransferMoneyFrom&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_MoneyAmount != c.m_MoneyAmount)
				return false;
			if(m_MoneyAmountType != c.m_MoneyAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(m_MoneyAmount <= 0)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferMoneyFrom& c = dynamic_cast<const AuMissionOrderTransferMoneyFrom&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_MoneyAmount != c.m_MoneyAmount)
				return false;
			if(m_MoneyAmountType != c.m_MoneyAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferMoneyFrom;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferMoneyTo : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	double								m_MoneyAmount;
	quint8								m_MoneyAmountType;		// 0=percent, 1=actual count
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferMoneyTo(const QString& targetName, const AuId& auth, double moneyAmount, quint8 moneyAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferMoneyTo();
	AuMissionOrderTransferMoneyTo(const AuMissionOrderTransferMoneyTo& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return (quint32)m_MoneyAmount;
	}

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual double AmountOfMoney() const
	{
		return m_MoneyAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_MoneyAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferMoneyTo& c = dynamic_cast<const AuMissionOrderTransferMoneyTo&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_MoneyAmount != c.m_MoneyAmount)
				return false;
			if(m_MoneyAmountType != c.m_MoneyAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(m_MoneyAmount <= 0)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferMoneyTo& c = dynamic_cast<const AuMissionOrderTransferMoneyTo&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_MoneyAmount != c.m_MoneyAmount)
				return false;
			if(m_MoneyAmountType != c.m_MoneyAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferMoneyTo;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferPeopleFrom : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint16								m_PeopleAmount;			// Number of people or percent 1..100.
	quint8								m_PeopleAmountType;		// 0=percent, 1=actual count
	quint8								m_PeopleType;			// 0=officers, 1=crew, 2=troops, 3=passengers
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferPeopleFrom(const QString& targetName, const AuId& auth, quint8 peopleType, quint16 peopleAmount,
			quint8 peopleAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferPeopleFrom();
	AuMissionOrderTransferPeopleFrom(const AuMissionOrderTransferPeopleFrom& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_PeopleAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_PeopleAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferPeopleFrom& c = dynamic_cast<const AuMissionOrderTransferPeopleFrom&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_PeopleType != c.m_PeopleType)
				return false;
			if(m_PeopleAmount != c.m_PeopleAmount)
				return false;
			if(m_PeopleAmountType != c.m_PeopleAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(!m_PeopleAmount)
			return false;
		if(m_PeopleType > 3)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferPeopleFrom& c = dynamic_cast<const AuMissionOrderTransferPeopleFrom&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_PeopleType != c.m_PeopleType)
				return false;
			if(m_PeopleAmount != c.m_PeopleAmount)
				return false;
			if(m_PeopleAmountType != c.m_PeopleAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// 0=officer, 1=crew, 2=troop, 3=passenger
	virtual quint8 PeopleType() const
	{
		return m_PeopleType;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferPeopleFrom;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderTransferPeopleTo : public AuMissionOrder
{
private:
	AuId								m_Authorization;		// Id of order issuer. Used to gain access to the target.
	quint16								m_PeopleAmount;			// Number of people or percent 1..100.
	quint8								m_PeopleAmountType;		// 0=percent, 1=actual count
	quint8								m_PeopleType;			// 0=officers, 1=crew, 2=troops, 3=passengers
	QString								m_TargetName;			// Blank = "the station".

public:
	AuMissionOrderTransferPeopleTo(const QString& targetName, const AuId& auth, quint8 peopleType, quint16 peopleAmount,
			quint8 peopleAmountType,
			EXECTYPE execType = Sequential);
	AuMissionOrderTransferPeopleTo();
	AuMissionOrderTransferPeopleTo(const AuMissionOrderTransferPeopleTo& c);

	// Amount as a percent 0..100 or a literal number of units, depending on AmountType.
	virtual quint32 Amount() const
	{
		return m_PeopleAmount;
	}

	// 0=percent, 1=actual count
	virtual quint8 AmountType() const
	{
		return m_PeopleAmountType;
	}

	virtual AuId Authorization() const
	{
		return m_Authorization;
	}

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	// Returns a string that is included with the AuCommHailRequest transmitted when this becomes the current order.
	virtual QString HailIntentions() const;

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferPeopleTo& c = dynamic_cast<const AuMissionOrderTransferPeopleTo&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_PeopleType != c.m_PeopleType)
				return false;
			if(m_PeopleAmount != c.m_PeopleAmount)
				return false;
			if(m_PeopleAmountType != c.m_PeopleAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const
	{
		if(m_Authorization.IsNull())
			return false;
		if(!m_PeopleAmount)
			return false;
		if(m_PeopleType > 3)
			return false;
		return AuMissionOrder::IsValid();
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderTransferPeopleTo& c = dynamic_cast<const AuMissionOrderTransferPeopleTo&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_Authorization != c.m_Authorization)
				return false;
			if(m_PeopleType != c.m_PeopleType)
				return false;
			if(m_PeopleAmount != c.m_PeopleAmount)
				return false;
			if(m_PeopleAmountType != c.m_PeopleAmountType)
				return false;
			if(m_TargetName != c.m_TargetName)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	// 0=officer, 1=crew, 2=troop, 3=passenger
	virtual quint8 PeopleType() const
	{
		return m_PeopleType;
	}

	virtual QString TargetName() const
	{
		return m_TargetName;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return TransferPeopleTo;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Specifies harvest operations for specific bays.

class AUGLOBAL_DLL AuMissionOrderHarvestOperationByBay : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint8								m_FuelThreshold;		// Break off operations when ship fuel gets to this level or below.
	AuHarvestOps						m_HarvestOps;			// Harvester operations. Key is bay which id, value is the op.

public:
	AuMissionOrderHarvestOperationByBay(const AuId& bodyId, const QString& bodyName, quint8 fuelThreshold,
			const AuHarvestOps& harvestOps, EXECTYPE execType = Sequential);
	AuMissionOrderHarvestOperationByBay();
	AuMissionOrderHarvestOperationByBay(const AuMissionOrderHarvestOperationByBay& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual quint8 FuelThreshold() const
	{
		return m_FuelThreshold;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	virtual const AuHarvestOps& HarvestOps() const
	{
		return m_HarvestOps;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestOperationByBay& c = dynamic_cast<const AuMissionOrderHarvestOperationByBay&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_FuelThreshold != c.m_FuelThreshold)
				return false;
			if(m_HarvestOps != c.m_HarvestOps)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestOperationByBay& c = dynamic_cast<const AuMissionOrderHarvestOperationByBay&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_FuelThreshold != c.m_FuelThreshold)
				return false;
			if(m_HarvestOps != c.m_HarvestOps)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	virtual AuId TargetId() const
	{
		return m_BodyId;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestOperationByBay;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Specifies harvest operations for bay equipped with specific modules.

class AUGLOBAL_DLL AuMissionOrderHarvestOperationByModule : public AuMissionOrder
{
private:
	AuId								m_BodyId;				// Id of body to harvest.
	QString								m_BodyName;				// Name of body to harvest.
	quint8								m_FuelThreshold;		// Break off operations when ship fuel gets to this level or below.
	AuHarvestOps						m_HarvestOps;			// Harvester operations. Key is bay module cid, value is the op.

public:
	AuMissionOrderHarvestOperationByModule(const AuId& bodyId, const QString& bodyName, quint8 fuelThreshold,
			const AuHarvestOps& harvestOps, EXECTYPE execType = Sequential);
	AuMissionOrderHarvestOperationByModule();
	AuMissionOrderHarvestOperationByModule(const AuMissionOrderHarvestOperationByModule& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual quint8 FuelThreshold() const
	{
		return m_FuelThreshold;
	}

	virtual AuId HarvestId() const
	{
		return m_BodyId;
	}

	// Harvest name returned for orders that refer to an atom.
	virtual QString HarvestName() const
	{
		return m_BodyName;
	}

	virtual const AuHarvestOps& HarvestOps() const
	{
		return m_HarvestOps;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestOperationByModule& c = dynamic_cast<const AuMissionOrderHarvestOperationByModule&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_FuelThreshold != c.m_FuelThreshold)
				return false;
			if(m_HarvestOps != c.m_HarvestOps)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderHarvestOperationByModule& c = dynamic_cast<const AuMissionOrderHarvestOperationByModule&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_BodyId != c.m_BodyId)
				return false;
			if(m_BodyName != c.m_BodyName)
				return false;
			if(m_FuelThreshold != c.m_FuelThreshold)
				return false;
			if(m_HarvestOps != c.m_HarvestOps)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 3;
	}

	virtual AuId TargetId() const
	{
		return m_BodyId;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return HarvestOperationByModule;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuMissionOrderReport : public AuMissionOrder
{
private:
	QString								m_Channel;			// Channel for the report.
	quint8								m_Range;			// Range for the report.
	quint16								m_ReportWhat;
	QString								m_Text;				// Additional text to say.

public:
	AuMissionOrderReport(quint16 reportWhat, const QString& sayText, const QString& channel, AuCommStdChannel::RANGE range,
			EXECTYPE execType = Sequential);
	AuMissionOrderReport();
	AuMissionOrderReport(const AuMissionOrderReport& c);

	// Returns a duplicate of this mission order, allocated on the heap.
	virtual AuMissionOrder* Clone() const;

	virtual QString CommChannel() const
	{
		return m_Channel;
	}
	
	virtual AuCommStdChannel::RANGE CommRange() const
	{
		return (AuCommStdChannel::RANGE)m_Range;
	}

	// Same as operator== except that the MissionOrderId is not compared.
	virtual bool IsSame(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderReport& c = dynamic_cast<const AuMissionOrderReport&>(order);
			if(!AuMissionOrder::IsSame(c))
				return false;
			if(m_ReportWhat != c.m_ReportWhat)
				return false;
			if(m_Range != c.m_Range)
				return false;
			if(m_Channel != c.m_Channel)
				return false;
			if(m_Text != c.m_Text)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	virtual bool IsValid() const;

	// Returns a description of the mission order.
	virtual QString ListDescription() const;

	virtual QString ListDescriptionIcon() const;

	virtual QString ListName() const;

	virtual QString ListNameIcon() const;

	virtual QString ListToolTip() const;

	virtual quint64 MemoryUsage() const;

	virtual void operator<<(QDataStream& in);

	virtual void operator>>(QDataStream& out) const;

	virtual bool operator==(const AuMissionOrder& order) const
	{
		try
		{
			const AuMissionOrderReport& c = dynamic_cast<const AuMissionOrderReport&>(order);
			if(!AuMissionOrder::operator==(c))
				return false;
			if(m_ReportWhat != c.m_ReportWhat)
				return false;
			if(m_Range != c.m_Range)
				return false;
			if(m_Channel != c.m_Channel)
				return false;
			if(m_Text != c.m_Text)
				return false;
			return true;
		}
		catch(const std::bad_cast&)
		{
			// dynamic_cast failed, different class of object
			return false;
		}
	}

	// Returns the number of seconds needed to complete the order by a highly skilled AI.
	virtual int OrderTime() const
	{
		return 2;
	}

	virtual quint16 ReportWhat() const
	{
		return m_ReportWhat;
	}

	virtual QString SayText() const
	{
		return m_Text;
	}

	// Returns a number unique to each derived mission order class.
	// Identifies each type of mission order in a data stream.
	virtual TYPE Type() const
	{
		return Report;
	}

	virtual QString Verbalization(const QString& pilot, const QString& navigator, const QString& engineer, const QString& power,
			const QString& sensor, const QString& shields) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuMission_H
