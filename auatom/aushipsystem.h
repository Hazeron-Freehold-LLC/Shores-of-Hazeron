//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipsystem.h,v 1.56 2022/02/19 17:32:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuShipSystem_H
#define AuShipSystem_H

#include "auatom.h"
#include "dbcommodity.h"
#include <QDataStream>
#include <QList>

class AuBdReport;
class AuScReport;
class AuShipSystemArray;

class AuShipSystem
{
private:
	qint32			m_ConnectedConsumer;	// Index in AuShipSystemArray. Consumer of this system's power. -1 is disconnected.
	quint32			m_HealthCurrent;		// Current health level.
	quint32			m_HealthMax;			// Maximum health level. Also maximum power consumed/produced by this system.
	bool			m_IsOverclocked;		// True if overclocking power output.
	quint16			m_Module;				// System module currently installed.
	quint32			m_PowerConsumption;		// Sum of power suppliers connected to this consumer, up to m_HealthMax.
	quint8			m_Quality;				// System quality.
	quint8			m_System;				// System type. SYSTEM
	quint16			m_Which;				// System 'which'

public:
	AuShipSystem();
	AuShipSystem(SYSTEM system, quint16 which, COMMODITYID module, quint8 quality, unsigned int totalUnits, int systemIndex);
	AuShipSystem(const AuShipSystem& rhs);
	~AuShipSystem();

public:
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	inline bool operator==(const AuShipSystem& c) const
	{
		Q_ASSERT(false);				//DHH Function added to satisfy dll export build in Windows. Does anybody use it?
		return m_System == c.m_System && m_Which == c.m_Which;
	}

public:
	inline void DisconnectConsumer()
	{
		m_ConnectedConsumer = -1;
	}

	quint64 MemoryUsage() const;

	inline COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	// Set overclocking of supply power.
	inline void OverclockSupplier(bool overclock)
	{
		m_IsOverclocked = overclock;
	}

	inline AuSysId GetSysId() const
	{
		return AuSysId(GetSystemType(), GetSystemWhich());
	}

	// Accessors/Mutators
	inline SYSTEM GetSystemType() const
	{
		return (SYSTEM)m_System;
	}

	inline quint16 GetSystemWhich() const
	{
		return m_Which;
	}

	inline bool GetOverclocking() const
	{
		return m_IsOverclocked;
	}

	// Current system supply output, factoring in damage and overclocking effects.
	inline unsigned int GetPowerSupplyCurrent() const
	{
		if(m_IsOverclocked)
		{
			unsigned int uiPowerSupplyCurrent = 2 * m_HealthCurrent;
			if(uiPowerSupplyCurrent > m_HealthMax)
				return m_HealthMax;
			return uiPowerSupplyCurrent;
		}
		else
		{
			return m_HealthCurrent;
		}
	}

	inline unsigned int GetPowerSupplyMax() const
	{
		return m_HealthMax;
	}

	inline unsigned int GetPowerConsumptionCurrent() const
	{
		return m_PowerConsumption;
	}

	inline unsigned int GetPowerConsumptionMax() const
	{
		return m_HealthMax;
	}

	inline void SetHealthMax(unsigned int healthMax)
	{
		m_HealthMax = healthMax;
		if(m_HealthCurrent > healthMax)
			m_HealthCurrent = healthMax;
	}

	inline void SetPowerConsumptionCurrent(unsigned int uiPower)
	{
		if(uiPower < m_HealthMax)
			m_PowerConsumption = uiPower;
		else
			m_PowerConsumption = m_HealthMax;
	}

	inline unsigned int GetHealthCurrent() const
	{
		return m_HealthCurrent;
	}

	inline unsigned int GetHealthMax() const
	{
		return m_HealthMax;
	}

	inline void SetHealthCurrent(unsigned int uiHealth)
	{
		m_HealthCurrent = uiHealth;
	}

	inline int GetConnectedConsumer() const
	{
		return m_ConnectedConsumer;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline void ResetPowerConsumptionCurrent()
	{
		m_PowerConsumption = 0;
	}

	void Rx(QDataStream& fromServer);

	inline void SetConnectedConsumer(int iNewConsumer)
	{
		m_ConnectedConsumer = iNewConsumer;
	}

	inline void SetModule(COMMODITYID cid)
	{
		m_Module = cid;
	}

	inline void SetQuality(quint8 quality)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
	}

	// This is a versioning mechanism, to trigger code to read TLs from old data.
	inline void SetReadTL()
	{
		m_Quality = 255;
	}

	inline AuSysId SysId() const
	{
		return AuSysId(GetSystemType(), GetSystemWhich());
	}

	void Tx(QDataStream& toClient) const;
};

inline unsigned int qHash(const AuShipSystem& key)
{
	Q_ASSERT(false);					//DHH Function added to satisfy dll export build in Windows. Does anybody use it?
	return (key.GetSystemType() << 16) | key.GetSystemWhich();
}

class AUATOM_DLL AuShipSystemArray : public QList<AuShipSystem>
{
private:
	quint16			m_CapacitorModule;		// Capacitor module currently installed.
	quint8			m_CapacitorQuality;		// Capacitor system.
	quint16			m_CnCModule;			// CommandAndControl module currently installed.
	quint8			m_CnCQuality;			// CommandAndControl system.
	quint16			m_HullModule;			// Hull module currently installed.
	quint8			m_HullQuality;			// Fuel, Hold and Hull systems.
	quint16			m_PowerPlantModule;		// Power plant module currently installed.
	quint8			m_PowerPlantQuality;	// PowerPlant system.

public:
	AuShipSystemArray();

public:
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

public:
	// Disconnects the consumer system from the supplier system 'supplierIndex'.
	void DisconnectConsumer(int supplierIndex);
	
	// Reduce the quality of a system to 'repairQ' after parts of that quality were installed, if system Q was higher.
	// Returns false if the system quality was already at 'repairQ' or below.
	// Returns true if the system quality changes.
	bool DowngradeSystemQuality(const AuSysId& sysId, quint8 repairQ);

	// Fills this vector with information about the design.
	// 'quality' becomes the starting quality of all systems. They can individually change over time after that.
	void GatherShipSystems(quint8 quality, const AuBdReport& design);
	void GatherShipSystems(quint8 quality, const AuScReport& design);

	unsigned int GetPowerConsumption(SYSTEM system, quint16 which) const;

	unsigned int GetPowerConsumption(SYSTEM system) const;

	bool HasConnectedSupplier(int consumerIndex) const;

	bool HasSystem(const AuSysId& sysId) const;

	bool HasVehicleTransporter() const;

	quint8 HighestQuality() const;

	quint8 LowestQuality() const;

	quint64 MemoryUsage() const;

	// Update the power consumption of a system based on the systems supplying it.
	// Returns true if any value changes that is sent in Tx.
	bool RecalculatePowerConsumption(int consumerIndex);

	// Resets all connections and turns off all overclocking.
	void Reset();

	void Rx(QDataStream& fromServer);

	bool SetSystemQuality(const AuSysId& sysId, quint8 quality);

	void Tx(QDataStream& toClient) const;

	COMMODITYID SystemModule(SYSTEM system) const;

	COMMODITYID SystemModule(SYSTEM system, quint16 which) const;

	inline COMMODITYID SystemModule(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return SystemModule(sysId.System(), sysId.Which());
		else
			return SystemModule(sysId.System());
	}

	quint8 SystemQuality(SYSTEM system) const;

	quint8 SystemQuality(SYSTEM system, quint16 which) const;

	inline quint8 SystemQuality(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return SystemQuality(sysId.System(), sysId.Which());
		else
			return SystemQuality(sysId.System());
	}

	// Fills 'whiches' with all which ids for a given 'system'.
	// 'whiches' is emptied if the system is not subdivided into sub systems.
	// Returns true on success.
	// Returns false if this vessel does not have any 'system'.
	bool SystemWhiches(SYSTEM system, QSet<quint16>& whiches) const;

	void RecalcHealthMax(const AuBdReport& design);
	void RecalcHealthMax(const AuScReport& design);

	// Returns true if changes were made.
	// Returns false if nothing changes.
	bool Reconcile(const AuBdReport& design);
	bool Reconcile(const AuScReport& design);

	// This could also be a downgrade.
	bool UpgradeSystem(SYSTEM system, COMMODITYID cid, quint8 quality);

	// This could also be a downgrade.
	bool UpgradeSystem(SYSTEM system, quint16 which, COMMODITYID cid, quint8 quality);
};

inline QDataStream& operator<<(QDataStream& out, const AuShipSystemArray& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuShipSystemArray& o)
{
	o << in;
	return in;
}

#endif // AuShipSystem_H

