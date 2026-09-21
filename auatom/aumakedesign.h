//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumakedesign.h,v 1.4 2022/02/19 17:32:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMakeDesign_H
#define AuMakeDesign_H

#include "auatom.h"
#include "auid.h"

class AUATOM_DLL AuMakeDesign
{
private:
	quint32								m_DesignId;
	QString								m_DesignName;
	bool								m_IsReadyToSpawn;
	quint32								m_OwnerEmpire;
	AuId								m_OwnerId;
	QString								m_OwnerName;
	quint8								m_Quality;			// Design Q when not ready to spawn; output Q when ready to spawn.

public:	// Constructors
	inline AuMakeDesign()
	{
		m_DesignId = 0;
		m_IsReadyToSpawn = false;
		m_OwnerEmpire = 0;
	}

	inline AuMakeDesign(quint32 designId, const QString& designName, quint8 designQ)
	{
		Q_ASSERT(designQ);
		m_DesignId = designId;
		m_DesignName = designName;
		m_IsReadyToSpawn = false;
		m_Quality = designQ;
		m_OwnerEmpire = 0;
		Q_ASSERT(designId != 4294967083 && designId != 213);
	}

	inline AuMakeDesign(quint32 designId, const QString& designName, quint8 designQ, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire)
	{
		Q_ASSERT(designQ);
		m_DesignId = designId;
		m_DesignName = designName;
		m_IsReadyToSpawn = false;
		m_OwnerEmpire = ownerEmpire;
		m_OwnerId = ownerId;
		m_OwnerName = ownerName;
		m_Quality = designQ;
		Q_ASSERT(designId != 4294967083 && designId != 213);
	}

	inline AuMakeDesign(const AuMakeDesign& c)
	{
		m_DesignId = c.m_DesignId;
		m_DesignName = c.m_DesignName;
		m_IsReadyToSpawn = c.m_IsReadyToSpawn;
		m_OwnerEmpire = c.m_OwnerEmpire;
		m_OwnerId = c.m_OwnerId;
		m_OwnerName = c.m_OwnerName;
		m_Quality = c.m_Quality;
	}

public:	// Operators
	inline const AuMakeDesign& operator=(const AuMakeDesign& c)
	{
		m_DesignId = c.m_DesignId;
		m_DesignName = c.m_DesignName;
		m_IsReadyToSpawn = c.m_IsReadyToSpawn;
		m_OwnerEmpire = c.m_OwnerEmpire;
		m_OwnerId = c.m_OwnerId;
		m_OwnerName = c.m_OwnerName;
		m_Quality = c.m_Quality;
		return *this;
	}

	inline bool operator==(const AuMakeDesign& c) const
	{
		if(m_DesignId != c.m_DesignId)
			return false;
		if(m_DesignName != c.m_DesignName)
			return false;
		if(m_IsReadyToSpawn != c.m_IsReadyToSpawn)
			return false;
		if(m_OwnerEmpire != c.m_OwnerEmpire)
			return false;
		if(m_OwnerId != c.m_OwnerId)
			return false;
		if(m_OwnerName != c.m_OwnerName)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		return true;
	}

	inline bool operator!=(const AuMakeDesign& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline quint32 DesignId() const
	{
		return m_DesignId;
	}

	inline const QString& DesignName() const
	{
		return m_DesignName;
	}

	inline bool IsNull() const
	{
		return !m_DesignId;
	}

	inline bool IsReadyToSpawn() const
	{
		return m_IsReadyToSpawn;
	}

	quint64 MemoryUsage() const;

	inline quint32 OwnerEmpire() const
	{
		return m_OwnerEmpire;
	}

	inline const AuId& OwnerId() const
	{
		return m_OwnerId;
	}

	inline const QString& OwnerName() const
	{
		return m_OwnerName;
	}

	inline void SetOwner(const AuId& id, const QString& name, quint32 empire)
	{
		m_OwnerEmpire = empire;
		m_OwnerId = id;
		m_OwnerName = name;
	}

	inline void SetReadyToSpawn(bool isReady, quint8 outputQ)
	{
		Q_ASSERT(outputQ);
		m_IsReadyToSpawn = isReady;
		m_Quality = outputQ;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuMakeDesign& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMakeDesign& c)
{
	c << in;
	return in;
}

#endif // AuMakeDesign_H
