//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipment.h,v 1.29 2022/02/19 18:19:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuShipment_H
#define AuShipment_H

#include "audata.h"
#include "auglobell.h"
#include "auid.h"
#include "autime.h"
#include "dbcommodity.h"
#include <QDataStream>

class AuEventLog;
class AuItemHash;
class AuItemTEUContainer;
class AuTradeAgreement;
class AuTradeAgreementList;

class AUDATA_DLL AuShipmentBroker
{
	quint8								m_Levels;

public:	// Constructors
	inline AuShipmentBroker(quint8 levels)
	{
		m_Levels = levels;
	}

	inline AuShipmentBroker()
	{
		m_Levels = 0;
	}

	inline AuShipmentBroker(const AuShipmentBroker& c)
	{
		m_Levels = c.m_Levels;
	}

public:	// Operators
	inline const AuShipmentBroker& operator=(const AuShipmentBroker& c)
	{
		m_Levels = c.m_Levels;
		return *this;
	}

	inline bool operator==(const AuShipmentBroker& c) const
	{
		if(m_Levels != c.m_Levels)
			return false;
		return true;
	}

	inline bool operator!=(const AuShipmentBroker& c) const
	{
		return !operator==(c);
	}

	/*
	inline void operator>>(QDataStream& out) const
	{
		out << m_Levels;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Levels;
	}
	*/

public:	// Methods
	inline quint8 Levels() const
	{
		return m_Levels;
	}

	quint64 MemoryUsage() const;
};

inline unsigned int qHash(const AuShipmentBroker& key)
{
	return key.Levels();
}

/*
inline QDataStream& operator<<(QDataStream& out, const AuShipmentBroker& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuShipmentBroker& c)
{
	c << in;
	return in;
}
*/

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuShipment
{
	enum STATE
	{
		Filling,						// Shipment is being processed.
		AwaitingShipment,				// Shipment is waiting to be shipped.
		Enroute							// Shipment is enroute.
	};

private:
	quint32								m_Amount;				// Amount of commodities to ship.
	COMMODITYID							m_CID;					// Commodity to ship. CID_Invalid = use commodity filter
	COMMODITYFILTER						m_CommodityFilter;
	AuItemTEUContainer*					m_Container;			// The accumulated materials for the shipment.
	AuTime								m_LastShipped;
	quint8								m_MinQ;					// Minimum quality of materials.
	quint8								m_Repeat;				// Count of times to repeat. 0 for endless.
	STATE								m_State;				// STATE enum value;
	AuTradeAgreement*					m_Trade;				// Trade agreement if any.

public:	// Constructors
	AuShipment(COMMODITYID cid, quint32 amount, quint8 minQ, quint8 repeat,
			const AuId& destCityId, const QString& destDesc, float destDistPc, const AuId& destSysId, const AuId& destSectorId,
			quint32 empireOfOrigin);
	AuShipment(COMMODITYFILTER filter, quint32 amount, quint8 minQ, quint8 repeat,
			const AuId& destCityId, const QString& destDesc, float destDistPc, const AuId& destSysId, const AuId& destSectorId,
			quint32 empireOfOrigin);

	// Constructs a shipment at a broker in 'cityId', according to the terms of the trade agreement.
	AuShipment(const AuId& cityId, quint32 cityEmpire, const AuTradeAgreement& ta);

	AuShipment();
	AuShipment(const AuShipment& c);
	~AuShipment();

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	const AuShipment& operator=(const AuShipment& c);

	bool operator==(const AuShipment& c) const;

	inline bool operator!=(const AuShipment& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline quint32 Amount() const
	{
		return m_Amount;
	}

	inline COMMODITYID CommodityId() const
	{
		return m_CID;
	}

	inline COMMODITYFILTER CommodityFilter() const
	{
		return m_CommodityFilter;
	}

	inline const AuItemTEUContainer& Container() const
	{
		return *m_Container;
	}

	// Fills this shipment by taking commodites from 'source'.
	// Changes state to AwaitingShipment when filled.
	// Does nothing if this shipment is not Filling.
	// Returns true if anything changes.
	bool Fill(AuItemHash& source);

	// In this version of Fill, 'source' is not modified.
	// Instead, 'taken' is filled with the items taken from 'source'.
	bool Fill(const AuItemHash& source, AuItemHash& taken);

	// Returns the solar system location of the destination city, in galaxy coordinates.
	AuPoint DestSysLoc() const;

	QString HtmlToolTip() const;

	inline bool IsAwaitingShipment() const
	{
		return m_State == AwaitingShipment;
	}

	inline bool IsEnroute() const
	{
		return m_State == Enroute;
	}

	inline bool IsFilling() const
	{
		return m_State == Filling;
	}

	// True when Enroute and enough time has passed, based on the distance to the destination system.
	bool IsReadyToDeliver() const;

	// Returns true if this shipment belongs to 'ta'.
	bool IsTradeAgreement(const AuTradeAgreement& ta) const;

	// Returns a descriptive name for this shipment.
	QString ListDesc() const;

	// Returns a very brief name for this shipment that does not mention its status.
	QString ListName() const;

	quint64 MemoryUsage() const;

	// One office is required to process a shipment that is not associated with a trade agreement.
	// True return requires/causes an office to be allocated to this shipment.
	inline bool NeedsOffice() const
	{
		// Shipments associated with a trade agreement are not counted because the associated trade agreement allocates the office.
		if(m_Trade)
			return false;
		// Shipments enroute do not need an office.
		return m_State == Filling || m_State == AwaitingShipment;
	}

	static int RegisterMetaType();

	// Returns 0 if endless.
	inline quint8 RepeatCount() const
	{
		return m_Repeat;
	}

	// Called when this shipment has been shipped.
	// Clears the container so caller must grab that first.
	// Sets the last shipped time.
	// Returns true when this shipment is complete, i.e. do once.
	// Returns false when this shipment has been reset and is ready to start accumulating again.
	bool ResetAfterShipped();

	void SetShipped();

	void SetTradeAgreement(const AuTradeAgreement& ta);

	inline STATE ShipmentState() const
	{
		return m_State;
	}

	// Beware this can return 0. Non-zero means this shipment is associated with a trade agreement.
	inline const AuTradeAgreement* TradeAgreement() const
	{
		return m_Trade;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuShipment& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuShipment& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuShipment)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline unsigned int qHash(const AuShipment& c)
{
	return c.CommodityId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuShipmentList : public QList<AuShipment>
{
public:	// Constructors
	inline AuShipmentList()
	{
	}

	inline AuShipmentList(const AuShipmentList& c) : QList<AuShipment>(c)
	{
	}

public:	// Operators
	inline const AuShipmentList& operator=(const AuShipmentList& c)
	{
		QList<AuShipment>::operator=(c);
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			out << *ii;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 s;
		in >> s;
		while(s--)
		{
			AuShipment shipment;
			in >> shipment;
			append(shipment);
		}
	}

public:	// Methods
	bool AddShipment(const AuShipment& shipment);

	// True if any of my shipments are Filling.
	bool AnyFilling() const;

	// True if any of my shipments are Enroute and ready to be delivered.
	bool AnyReadyToDeliver() const;

	// True if any of my shipments are AwaitingShipment.
	bool AnyReadyToShip() const;

	// Returns the number of shipments enroute.
	quint16 CountEnroute() const;

	// A broker office is required to process each shipment.
	// That is shipments that return true to NeedsOffice.
	// Returns the number of offices needed to process shipments.
	quint16 CountOffices() const;

	// Delivers my shipments that are enroute.
	// 'deliveries' is filled with the shipments that are ready to be delivered. Those shipments are removed from this list.
	// 'tradesReady' is filled with agreements for shipments that are ready to be delivered. Those shipments remain unchanged.
	// Returns true if anything changes.
	bool Deliver(AuShipmentList& deliveries, AuTradeAgreementList& tradesReady);

	// Delivers my shipment associated with 'ta' that is enroute.
	// 'delivery' is filled with the shipment that is ready to be delivered. That shipment is removed from this list.
	// Returns: 0=trade shipment delivered, copy in 'delivery', 1=shipment not ready for delivery, 2=not my trade agreement
	int DeliverTrade(AuShipment& delivery, const AuTradeAgreement& ta);

	// Fills my shipments from 'source'.
	// Returns true if anything changes.
	bool Fill(AuItemHash& source);

	// In this version of Fill, 'source' is not modified.
	// Instead, 'taken' is filled with the items taken from 'source'.
	bool Fill(const AuItemHash& source, AuItemHash& taken);

	virtual quint64 MemoryUsage() const;

	// Called when a shipment is picked up. Returns true if the shipment is found. It is reset accordingly.
	bool PickUp(const AuShipment& shipment);

	// 'shipments' is filled with shipments that are ready to go
	void ReadyToShip(AuShipmentList& shipments) const;

	// Removes any shipments associated with a trade agreement that does not exist in 'trades'.
	// Returns true if any changes are made.
	bool Reconcile(const AuTradeAgreementList& trades);

	// Returns true if an exactly matching shipment is found and removed.
	bool Remove(const AuShipment& shipment);

	// Ships my orders that are AwaitingShipment, by marking them as Enroute.
	// 'broker' contains current information about the broker
	// 'cityIds' is the list of all known city ids. Shipments are removed if their destination city is not in the list.
	// Returns true if anything changes.
	bool Ship(const AuShipmentBroker& broker, const QSet<AuId>& cityIds);

	// Removes any shipments corresponding to the trade agreement.
	void TradeAgreementCanceled(const AuTradeAgreement& ta);
};

inline QDataStream& operator<<(QDataStream& out, const AuShipmentList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuShipmentList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuTradeAgreement
{
public:
	enum STATE
	{
		Proposal,						// New proposal waiting for acceptance.
		Declined,						// Proposal was assertively rejected at the destination.
		Countered,						// Proposal was assertively rejected and a counter offer was made.
		Undeliverable,					// Proposal could not be delivered to any broker.
		Agreement,						// Agreement is in effect.
		Terminated,						// Agreement was assertively terminated at the destination.
		Concluded,						// Agreement was executed to its conclusion.
	};

private:
	// Index 0 send: contains the source information and what is offered to send to them.
	// Index 1 get: contains the destination information and what is wanted to get from them.
	quint32								m_Amount[2];			// Amount of commodities to ship.
	COMMODITYID							m_CID[2];				// Commodity to ship. CID_Invalid = use commodity filter
	AuId								m_City[2];				// Id of offering city.
	QString								m_Desc[2];				// Desc of offering city.
	COMMODITYFILTER						m_Filter[2];
	quint8								m_MinQ[2];				// Minimum quality of materials.
	AuId								m_SectorId[2];			// Id of offering broker sector.
	AuId								m_SysId[2];				// Id of offering broker solar system.

	float								m_DistPc;				// Distance apart in parsecs.
	quint8								m_Repeat;				// Count of times to repeat. 0 for endless.
	STATE								m_State;				// STATE enum value;

public:	// Constructors
	// cid for cid
	AuTradeAgreement(COMMODITYID ocid, quint32 oamount, quint8 ominQ,
			const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
			COMMODITYID rcid, quint32 ramount, quint8 rminQ,
			const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
			quint8 repeat, float distPc);
	// cid for cat
	AuTradeAgreement(COMMODITYID ocid, quint32 oamount, quint8 ominQ,
			const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
			COMMODITYFILTER rfilter, quint32 ramount, quint8 rminQ,
			const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
			quint8 repeat, float distPc);
	// cat for cid
	AuTradeAgreement(COMMODITYFILTER ofilter, quint32 oamount, quint8 ominQ,
			const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
			COMMODITYID rcid, quint32 ramount, quint8 rminQ,
			const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
			quint8 repeat, float distPc);
	// cat for cat
	AuTradeAgreement(COMMODITYFILTER ofilter, quint32 oamount, quint8 ominQ,
			const AuId& oCityId, const QString& oDesc, const AuId& oSysId, const AuId& oSectorId,
			COMMODITYFILTER rfilter, quint32 ramount, quint8 rminQ,
			const AuId& rCityId, const QString& rDesc, const AuId& rSysId, const AuId& rSectorId,
			quint8 repeat, float distPc);

	AuTradeAgreement();
	AuTradeAgreement(const AuTradeAgreement& c);
	~AuTradeAgreement();

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	const AuTradeAgreement& operator=(const AuTradeAgreement& c);

	bool operator==(const AuTradeAgreement& c) const;

	inline bool operator!=(const AuTradeAgreement& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	// Returns true if my state changes.
	bool Accept();

	// Returns true if my state changes.
	bool Deactivate(STATE nuState);

	inline const AuId& DestinationCity() const
	{
		return m_City[1];
	}

	inline const QString& DestinationDesc() const
	{
		return m_Desc[1];
	}

	inline const AuId& DestinationSector() const
	{
		return m_SectorId[1];
	}

	inline const AuId& DestinationSystem() const
	{
		return m_SysId[1];
	}

	inline float DistancePc() const
	{
		return m_DistPc;
	}

	inline quint32 GetAmount() const
	{
		return m_Amount[1];
	}

	inline COMMODITYFILTER GetCommodityFilter() const
	{
		return m_Filter[1];
	}

	inline COMMODITYID GetCommodityId() const
	{
		return m_CID[1];
	}

	inline quint8 GetMinQ() const
	{
		return m_MinQ[1];
	}

	QString HtmlToolTip() const;

	inline bool IsAgreement() const
	{
		return m_State == Agreement;
	}

	inline bool IsConcluded() const
	{
		return m_State == Concluded;
	}

	inline bool IsCountered() const
	{
		return m_State == Countered;
	}

	inline bool IsDeclined() const
	{
		return m_State == Declined;
	}

	inline bool IsProposal() const
	{
		return m_State == Proposal;
	}

	inline bool IsTerminated() const
	{
		return m_State == Terminated;
	}

	inline bool IsUndeliverable() const
	{
		return m_State == Undeliverable;
	}

	// Returns a descriptive name for this trade agreement.
	QString ListDesc() const;

	// One office is required to process a trade agreement and its associated shipment.
	// True return requires/causes an office to be allocated to this trade agreement.
	inline bool NeedsOffice() const
	{
		// Only proposals and final agreements need an office. All other states exist to report status.
		return m_State == Proposal || m_State == Agreement;
	}

	inline const AuId& OriginCity() const
	{
		return m_City[0];
	}

	inline const QString& OriginDesc() const
	{
		return m_Desc[0];
	}

	inline const AuId& OriginSector() const
	{
		return m_SectorId[0];
	}

	inline const AuId& OriginSystem() const
	{
		return m_SysId[0];
	}

	static int RegisterMetaType();

	// Returns true if anything changes, which will include a state change to Concluded if called with a repeat count of one.
	inline bool ReduceRepeatCount()
	{
		if(m_Repeat == 0)
			return false;	// endless
		m_Repeat--;
		if(m_Repeat == 0)
			m_State = Concluded;
		return true;
	}

	// Returns 0 if endless.
	inline quint8 RepeatCount() const
	{
		return m_Repeat;
	}

	// Reports events to inform of trade agreement activity. Does not report agreed agreements, just those that beg for attention.
	void Report(AuEventLog& events) const;

	inline quint32 SendAmount() const
	{
		return m_Amount[0];
	}

	inline COMMODITYFILTER SendCommodityFilter() const
	{
		return m_Filter[0];
	}

	inline COMMODITYID SendCommodityId() const
	{
		return m_CID[0];
	}

	inline quint8 SendMinQ() const
	{
		return m_MinQ[0];
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuTradeAgreement& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTradeAgreement& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuTradeAgreement)	// Needed to use in queued signal/slot connections. Must also call qRegisterMetaType.

inline unsigned int qHash(const AuTradeAgreement& c)
{
	return c.SendCommodityId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuTradeAgreementList : public QList<AuTradeAgreement>
{
public:	// Constructors
	inline AuTradeAgreementList()
	{
	}

	inline AuTradeAgreementList(const AuTradeAgreementList& c) : QList<AuTradeAgreement>(c)
	{
	}

public:	// Operators
	inline const AuTradeAgreementList& operator=(const AuTradeAgreementList& c)
	{
		QList<AuTradeAgreement>::operator=(c);
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			out << *ii;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 s;
		in >> s;
		while(s--)
		{
			AuTradeAgreement ta;
			in >> ta;
			append(ta);
		}
	}

public:	// Methods
	// Returns true if an exactly matching trade agreement is found and accepted.
	// Modifies 'ta' to match the resulting agreement, after it is accepted.
	bool Accept(AuTradeAgreement& ta);

	bool AddTradeAgreement(const AuTradeAgreement& ta);

	// A broker office is required to process each trade agreement.
	// That is trade agreements that return true to NeedsOffice.
	// Returns the number of offices needed to process trade agreements.
	quint16 CountOffices() const;

	// Returns true if an exactly matching trade agreement is found and deactivated.
	bool Deactivate(const AuTradeAgreement& ta, AuTradeAgreement::STATE nuState);

	// Updates the 'ta' and the corresponding trade agreement. A non-zero RepeatCount is reduced and the state is set to Concluded
	// when done.
	void DeliveryComplete(AuTradeAgreement& ta);

	// Returns true if an exact match to 'ta' is found.
	bool HasTradeAgreement(const AuTradeAgreement& ta) const;

	// Returns true if an exactly matching trade agreement is found and removed.
	bool Remove(const AuTradeAgreement& ta);

	// Reports events to inform of trade agreement activity. Does not report agreed agreements, just those that beg for attention.
	void Report(AuEventLog& events) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuTradeAgreementList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTradeAgreementList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuShipment_H
