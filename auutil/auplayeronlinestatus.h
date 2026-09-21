//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayeronlinestatus.h,v 1.19 2022/02/19 18:42:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlayerOnlineStatus_H
#define AuPlayerOnlineStatus_H

#include "auid.h"
#include <QDataStream>
#include <QString>
#include <QMetaType>
#include <QVector>

class AUUTIL_DLL AuPlayerOnlineStatus
{
public:
	quint32		m_EmpireId;		// Empire id of player.
	AuId		m_Id;			// Player atom id.
	QString		m_Name;			// Player name;
	quint16		m_StatusBits;	// b0=nu emperor w/o town square
								// b1=agent
								// b2=afk, top byte is agent bits
								// b3=anonymous
								// b4=chat client "Hazeron Commander"
								// Top byte is agent bits.
	quint8		m_OsType;		// 0=Linux, 1=Windows, 2=Mac, 3=Invalid
	quint16		m_OsVer;		// Dependent on OS type.
	quint8		m_WordSize;		// Number of bits

public:	// Constructors
	inline AuPlayerOnlineStatus()
	{
		m_EmpireId = 0;
		m_StatusBits = 0;
		m_OsType = 0;
		m_OsVer = 0;
		m_WordSize = 0;
	}

	inline AuPlayerOnlineStatus(const AuPlayerOnlineStatus& c)
	{
		m_EmpireId = c.m_EmpireId;
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_StatusBits = c.m_StatusBits;
		m_OsType = c.m_OsType;
		m_OsVer = c.m_OsVer;
		m_WordSize = c.m_WordSize;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_EmpireId;
		out << m_Id;
		out << m_Name;
		out << m_StatusBits;
		out << m_OsType;
		out << m_OsVer;
		out << m_WordSize;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_EmpireId;
		in >> m_Id;
		in >> m_Name;
		in >> m_StatusBits;
		in >> m_OsType;
		in >> m_OsVer;
		in >> m_WordSize;
	}

	inline AuPlayerOnlineStatus& operator=(const AuPlayerOnlineStatus& c)
	{
		m_EmpireId = c.m_EmpireId;
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_StatusBits = c.m_StatusBits;
		m_OsType = c.m_OsType;
		m_OsVer = c.m_OsVer;
		m_WordSize = c.m_WordSize;
		return *this;
	}

	inline bool operator==(const AuPlayerOnlineStatus& c) const
	{
		if(m_EmpireId != c.m_EmpireId)
			return false;
		if(m_Id != c.m_Id)
			return false;
		if(m_Name != c.m_Name)
			return false;
		if(m_StatusBits != c.m_StatusBits)
			return false;
		if(m_OsType != c.m_OsType)
			return false;
		if(m_OsVer != c.m_OsVer)
			return false;
		if(m_WordSize != c.m_WordSize)
			return false;
		return true;
	}

public:	// Methods
	inline bool IsAFK() const
	{
		return m_StatusBits & 0x0004;
	}

	inline bool IsAgent() const
	{
		return m_StatusBits & 0x0002;
	}

	inline bool IsAgentPowerEnabled() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Enable;
	}

	inline bool IsAnimal() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Animal;
	}

	inline bool IsAnonymous() const
	{
		return m_StatusBits & 0x0008;
	}

	inline bool IsChatClient() const
	{
		return m_StatusBits & 0x0010;
	}

	inline bool IsEntity() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Entity;
	}

	inline bool IsIncognito() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Incognito;
	}

	inline bool IsInvincible() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Invincible;
	}

	inline bool IsInvisible() const
	{
		quint8 agentBits = m_StatusBits >> 8;
		return agentBits & AGENT_Invisible;
	}

	inline bool IsNewEmperor() const
	{
		return m_StatusBits & 0x0001;
	}

	inline const AuPlayerOnlineStatus CopyForClient(bool forAgent) const
	{
		if(!IsAnonymous())
		{
			// Return my information if I am not anonymous.
			AuPlayerOnlineStatus status(*this);
			if(!forAgent)
			{
				// Non-agents don't need to know information about the OS or computer of players.
				status.m_OsType = 0;
				status.m_OsVer = 0;
				status.m_WordSize = 0;
				status.m_StatusBits &= ~0x0010;	// Turn off chat client bit.
			}
			return status;
		}
		else
		{
			// Return null information if I am anonymous.
			AuPlayerOnlineStatus status;
			status.m_EmpireId = 0;
			status.m_Id = AuId(0);
			status.m_Name = QObject::tr("Anonymous");
			status.m_OsType = 0;
			status.m_OsVer = 0;
			status.m_WordSize = 0;
			return status;
		}
	}

	inline static QString OsTypeName(unsigned int osType)
	{
		const char* osTypes[] =
		{
			"Linux",
			"Windows",
			"Mac",
		};
		unsigned int osCount = sizeof(osTypes) / sizeof(const char*);
		if(osType >= osCount)
			return QString("Invalid OS Type %1").arg(osType);
		return osTypes[osType];
	}
	inline QString OsTypeName() const
	{
		return OsTypeName(m_OsType);
	}

	static QString OsVersionIcon(quint8 osType, quint16 osVer, quint8 wordSize);
	inline QString OsVersionIcon() const
	{
		return OsVersionIcon(m_OsType, m_OsVer, m_WordSize);
	}

	static QString OsVersionName(quint8 osType, quint16 osVer);
	inline QString OsVersionName() const
	{
		return OsVersionName(m_OsType, m_OsVer);
	}

	inline unsigned int WordSize() const
	{
		return m_WordSize;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuPlayerOnlineStatusVector : public QVector<AuPlayerOnlineStatus>
{
public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint32)count();
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			*si >> out;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint32 cnt;
		in >> cnt;
		for(; cnt; cnt--)
		{
			AuPlayerOnlineStatus pos;
			pos << in;
			append(pos);
		}
	}

public:
	inline void AddUnique(const AuPlayerOnlineStatusVector& players)
	{
		for(const_iterator si = players.constBegin(); si != players.constEnd(); si++)
			if(!Contains((*si).m_Id))
				append(*si);
	}

	inline bool Contains(const AuId& id) const
	{
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).m_Id == id)
				return true;
		return false;
	}

	inline void CopyForClient(AuPlayerOnlineStatusVector& players, bool forAgent) const
	{
		players.clear();
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			players.append((*si).CopyForClient(forAgent));
	}
};

Q_DECLARE_METATYPE(AuPlayerOnlineStatusVector);	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuPlayerOnlineStatus_H
