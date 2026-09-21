//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autime.cpp,v 1.2 2022/02/19 18:42:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autime.h"
//
#include <QDataStream>
#include <QDebug>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuTimeMetaType = qRegisterMetaType<AuTime>();

void AuTime::operator>>(QDataStream& out) const
{
	// This is one byte more efficient than storing a QDateTime, which stores a byte for the time spec.
	out << m_MSecsSinceEpoch;
}

void AuTime::operator<<(QDataStream& in)
{
	in >> m_MSecsSinceEpoch;
}

void AuTime::Tx16(QDataStream& out) const
{
	quint16 twoBytes = (quint16)MSecsSinceEpoch();
	out << twoBytes;
}

void AuTime::Tx32(QDataStream& out) const
{
	quint32 fourBytes = (quint32)MSecsSinceEpoch();
	out << fourBytes;
}

void AuTime::Rx16(QDataStream& in, qint64 serverTimeOffset)
{
	quint16 twoBytes;
	in >> twoBytes;

	// Combine the two bytes we received with the local time, coordinated with the server.
	m_MSecsSinceEpoch = (CurrentMSecsSinceEpoch() + serverTimeOffset) & 0xffffffffffff0000;	// Mask off the bottom 16 bits.
	m_MSecsSinceEpoch |= twoBytes;		// OR in the two bytes we received from the server.
}

void AuTime::Rx32(QDataStream& in, qint64 serverTimeOffset)
{
	quint32 fourBytes;
	in >> fourBytes;

	// Combine the two bytes we received with the local time, coordinated with the server.
	m_MSecsSinceEpoch = (CurrentMSecsSinceEpoch() + serverTimeOffset) & 0xffffffff00000000;	// Mask off the bottom 32 bits.
	m_MSecsSinceEpoch |= fourBytes;		// OR in the four bytes we received from the server.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const AuTime& t)
{
	dbg.nospace() << t.UTC() << "(" << t.MSecsSinceEpoch() << "ms epoch)";
	return dbg.space();
}
