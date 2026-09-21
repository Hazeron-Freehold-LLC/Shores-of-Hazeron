//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autransportertarget.h,v 1.4 2022/02/19 18:19:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTransporterTarget_H
#define AuTransporterTarget_H

#include "audata.h"
#include "audef.h"
#include "auid.h"

class AUDATA_DLL AuTransporterTarget
{
	AuId                                m_Id;
	QString								m_Name;
	TRANSPORTER_TARGET                  m_Target;				// For sub destination target locations.

public:
	AuTransporterTarget(const AuId& id, const QString& name, TRANSPORTER_TARGET target);
	AuTransporterTarget();
	AuTransporterTarget(const AuTransporterTarget& c);

public:
	const AuTransporterTarget& operator=(const AuTransporterTarget& c);
	bool operator==(const AuTransporterTarget& c);
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	const char* Icon() const;

	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	static int RegisterMetaType();

	inline TRANSPORTER_TARGET Target() const
	{
		return m_Target;
	}

	inline TRANSPORTER_TARGET TargetType() const
	{
		if(m_Target < TT_GeologicalFeatureFirst)
			return m_Target;
		else if(m_Target <= TT_GeologicalFeatureLast)
			return TT_GeologicalFeatureFirst;
		return TT_Invalid;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuTransporterTarget& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTransporterTarget& c)
{
	c << in;
	return in;
}

Q_DECLARE_METATYPE(AuTransporterTarget)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuTransporterShip
{
	AuId                                m_Id;
	QString								m_Name;
	QString								m_SpotDesc;
	TRANSPORTER_TARGET                  m_Target;				// For sub destination target locations.

public:
	AuTransporterShip(const AuId& id, const QString& name, const QString& spotDesc, TRANSPORTER_TARGET target);
	AuTransporterShip();
	AuTransporterShip(const AuTransporterShip& c);

public:
	const AuTransporterShip& operator=(const AuTransporterShip& c);
	bool operator==(const AuTransporterShip& c);
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	const char* Icon() const;

	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline const QString& SpotDesc() const
	{
		return m_SpotDesc;
	}

	inline TRANSPORTER_TARGET Target() const
	{
		return m_Target;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuTransporterShip& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTransporterShip& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuTransporterSystem
{
	AuId                                m_Id;
	QString								m_Name;
	QString								m_SpotDesc;
	quint16								m_Which;

public:
	AuTransporterSystem(const AuId& id, const QString& name, const QString& spotDesc, quint16 which);
	AuTransporterSystem();
	AuTransporterSystem(const AuTransporterSystem& c);

public:
	const AuTransporterSystem& operator=(const AuTransporterSystem& c);
	bool operator==(const AuTransporterSystem& c);
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	const char* Icon() const;

	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline const QString& SpotDesc() const
	{
		return m_SpotDesc;
	}

	inline quint16 Which() const
	{
		return m_Which;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuTransporterSystem& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTransporterSystem& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuTransporterTarget_H
