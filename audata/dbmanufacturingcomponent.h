//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbmanufacturingcomponent.h,v 1.13 2022/04/08 14:46:42 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DBMANUFACTURINGCOMPONENT_H
#define DBMANUFACTURINGCOMPONENT_H

#include "audata.h"
#include "dbcommodity.h"
#include <QVector>

class AuKnownPatents;

class AUDATA_DLL DbManufacturingComponent
{
public:
	quint16		m_id;			// id of this component
	quint16		m_idmp;			// manufacturing process id
	quint16		m_idc;			// commodity consumed/used
	quint32		m_consumed;		// qty consumed, may be zero
	quint8		m_ide;			// effect id of component

public:
	DbManufacturingComponent();
	~DbManufacturingComponent();
	DbManufacturingComponent(const DbManufacturingComponent &c);

public:	// Operators
	const DbManufacturingComponent& operator=(const DbManufacturingComponent &c);

	inline bool operator==(const DbManufacturingComponent& c) const
	{
		return m_id == c.m_id && m_idmp == c.m_idmp;
	}

public:	// Methods
	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_idc;
	}

	inline COMPEFFECT Effect() const
	{
		return (COMPEFFECT)m_ide;
	}

	// True if this component is required by the process, whether or not it is consumed.
	inline bool IsRequired() const
	{
		return m_ide == CE_Required || m_ide == CE_RequiredQ;
	}

	// True if this component is required but not consumed by the process.
	inline bool IsRequiredModifier() const
	{
		return IsRequired() && !QtyConsumed();
	}

	// Returns the total amount of this commodity that is consumed by this process.
	inline quint32 QtyConsumed() const
	{
		return m_consumed;
	}

	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;
};

class AUDATA_DLL DbManufacturingComponentArray : public QVector<DbManufacturingComponent>
{
public:
	// Returns true if all required patents by 'mpid' are available in 'patents'.
	// Returns true if 'mpid' does not require any patents at all.
	// Returns false if 'mpid' requires a patent but that patent is not available in 'patents' at least at Q1.
	bool HasNecessaryPatents(quint16 mpid, const AuKnownPatents& patents) const;

	// Returns true if a commodity is used by a manufacturing process.
	bool IsUsed(quint16 mpid, COMMODITYID cid) const;

	void Load();

	// Returns the lowest Quality of the patents required by 'mpid'. This acts as an upper limit on output Q of the process.
	// Returns 255 if no patents are required at all, imposing no upper limit.
	// Returns 0 if a required patent is not found in 'patents'.
	quint8 PatentQ(quint16 mpid, const AuKnownPatents& patents) const;

	// Returns true if 'cid' is a required component of the 'mpid' process.
	bool RequiresCommodity(quint16 mpid, COMMODITYID cid) const;
};

#endif // DBMANUFACTURINGCOMPONENT_H
