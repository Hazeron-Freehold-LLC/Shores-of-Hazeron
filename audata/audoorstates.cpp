//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audoorstates.cpp,v 1.6 2022/02/19 18:20:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audoorstates.h"
//

AuDoorStates::AuDoorStates()
{
	m_DoorCount = 0;
	m_LockCrewDoors = false;
	m_LockHullDoors = false;									// Must default to unlocked so non-crew may enter nu ships.
}

AuDoorStates::~AuDoorStates()
{
}

AuDoorStates::AuDoorStates(const AuDoorStates& c) : QByteArray(c)
{
	m_ChangeTime = c.m_ChangeTime;
	m_DoorCount = c.m_DoorCount;
	m_LockCrewDoors = c.m_LockCrewDoors;
	m_LockHullDoors = c.m_LockHullDoors;
}

void AuDoorStates::operator<<(QDataStream& in)
{
	::operator>>(in, (QByteArray&)(*this));
	in >> m_DoorCount;

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_ChangeTime = tm;

	// Switches are combined into a single byte.
	quint8 switches;
	in >> switches;
	m_LockCrewDoors = (switches & 0x01) ? true : false;
	m_LockHullDoors = (switches & 0x02) ? true : false;
}

void AuDoorStates::operator>>(QDataStream& out) const
{
	::operator<<(out, (const QByteArray&)(*this));
	out << m_DoorCount;
	out << m_ChangeTime.UTC();

	// Combine switches into a single byte.
	quint8 switches = 0;
	if(m_LockCrewDoors)
		switches |= 0x01;
	if(m_LockHullDoors)
		switches |= 0x02;
	out << switches;
}

void AuDoorStates::Rx(QDataStream& in)
{
	::operator>>(in, (QByteArray&)(*this));

	// Client determines number of doors by analyzing the design.
	// Client does not need the change time or the actual number of operable doors.
	// Space ship receives lock states via a bit mask of switches.
}

void AuDoorStates::Tx(QDataStream& out) const
{
	::operator<<(out, (const QByteArray&)(*this));

	// Client determines number of doors by analyzing the design.
	// Client does not need the change time or the actual number of operable doors.
	// Space ship transmits lock states via a bit mask of switches.
}

void AuDoorStates::Refresh(const AuDoorStates& c)
{
	// Only the door states are sent to the client from the server.
	QByteArray::operator=(c);

	// The space ship received lock states and set them into this object. They should be refreshed here.
	m_LockCrewDoors = c.m_LockCrewDoors;
	m_LockHullDoors = c.m_LockHullDoors;
}

bool AuDoorStates::JamDoorClosed(unsigned int doorIndex)
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		iterator it = begin() + doorbyte;
		quint8 states = *it;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			states &= (0xfe - 0x02);
			break;
		case 1:
			states &= (0xfb - 0x08);
			break;
		case 2:
			states &= (0xef - 0x20);
			break;
		case 3:
			states &= (0xbf - 0x80);
			break;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
		if((quint8)(*it) != states)
		{
			*it = states;
			m_ChangeTime.Set();
			return true;
		}
	}
	return false;
}

bool AuDoorStates::CloseDoor(unsigned int doorIndex)
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		iterator it = begin() + doorbyte;
		quint8 states = *it;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			states &= 0xfe;
			break;
		case 1:
			states &= 0xfb;
			break;
		case 2:
			states &= 0xef;
			break;
		case 3:
			states &= 0xbf;
			break;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
		if((quint8)(*it) != states)
		{
			*it = states;
			m_ChangeTime.Set();
			return true;
		}
	}
	return false;
}

bool AuDoorStates::JamDoorOpen(unsigned int doorIndex)
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		iterator it = begin() + doorbyte;
		quint8 states = *it;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			states |= 0x01|0x02;
			break;
		case 1:
			states |= 0x04|0x08;
			break;
		case 2:
			states |= 0x10|0x20;
			break;
		case 3:
			states |= 0x40|0x80;
			break;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
		if((quint8)(*it) != states)
		{
			*it = states;
			m_ChangeTime.Set();
			return true;
		}
	}
	return false;
}

bool AuDoorStates::UnjamDoor(unsigned int doorIndex)
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		iterator it = begin() + doorbyte;
		quint8 states = *it;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			states &= (0xff - 0x02);
			break;
		case 1:
			states &= (0xff - 0x08);
			break;
		case 2:
			states &= (0xff - 0x20);
			break;
		case 3:
			states &= (0xff - 0x80);
			break;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
		if((quint8)(*it) != states)
		{
			*it = states;
			m_ChangeTime.Set();
			return true;
		}
	}
	return false;
}

bool AuDoorStates::OpenDoor(unsigned int doorIndex)
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		iterator it = begin() + doorbyte;
		quint8 states = *it;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			states |= 0x01;
			break;
		case 1:
			states |= 0x04;
			break;
		case 2:
			states |= 0x10;
			break;
		case 3:
			states |= 0x40;
			break;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
		if((quint8)(*it) != states)
		{
			*it = states;
			m_ChangeTime.Set();
			return true;
		}
	}
	return false;
}

bool AuDoorStates::IsDoorJammed(unsigned int doorIndex) const
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		const_iterator it = constBegin() + doorbyte;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			return ((*it) & 0x02) ? true : false;
		case 1:
			return ((*it) & 0x08) ? true : false;
		case 2:
			return ((*it) & 0x20) ? true : false;
		case 3:
			return ((*it) & 0x80) ? true : false;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
	}
	return false;
}

bool AuDoorStates::IsDoorOpen(unsigned int doorIndex) const
{
	unsigned int doorbyte = doorIndex/4;	// byte index of door states
	if(doorbyte < (unsigned int)count())
	{
		const_iterator it = constBegin() + doorbyte;
		unsigned int subidx = doorIndex - doorbyte*4;
		switch(subidx)
		{
		case 0:
			return ((*it) & 0x01) ? true : false;
		case 1:
			return ((*it) & 0x04) ? true : false;
		case 2:
			return ((*it) & 0x10) ? true : false;
		case 3:
			return ((*it) & 0x40) ? true : false;
		default:
			Q_ASSERT(false);
			break;	// not possible
		}
	}
	return false;
}

void AuDoorStates::SetDoorCount(quint16 doors)
{
	m_DoorCount = doors;

	// The number of operable doors changed.
	int curbytes = size();
	int doorbytes = (doors + 3) / 4;							// Number of bytes needed to store door states.
	if(curbytes != doorbytes)
	{
		resize(doorbytes);

		// If bytes were added at the end, fill them with zeros, which is the normal closed state.
		while(curbytes < doorbytes)
			operator[](curbytes++) = 0;

		m_ChangeTime.Set();
	}
}

