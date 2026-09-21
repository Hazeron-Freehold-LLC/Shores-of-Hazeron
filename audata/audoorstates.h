//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audoorstates.h,v 1.7 2022/02/19 18:19:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDoorStates_H
#define AuDoorStates_H

#include "audata.h"
#include "autime.h"
#include <QByteArray>
#include <QDataStream>

class AUDATA_DLL AuDoorStates : public QByteArray
{
private:
	quint16								m_DoorCount;			// number of doors that can be opened/closed
	AuTime								m_ChangeTime;			// Time of last door state change
	bool								m_LockCrewDoors;		// crew doors can only be operated by crew when locked
	bool								m_LockHullDoors;		// hull doors can only be operated by crew when locked

public:
	// Construction/destruction.
	AuDoorStates();
	virtual ~AuDoorStates();
	AuDoorStates(const AuDoorStates& c);

	// Returns a reference to the base class QByteArray. Used when storing to a blob field, to retain backward compatibility when
	// this information was not all in a single object.
	inline const QByteArray& ByteArray() const
	{
		return *this;
	}

	// Returns the last time any door states changed.
	inline const AuTime& ChangeTime() const
	{
		return m_ChangeTime;
	}

	// Returns true if the state of the door changes
	bool CloseDoor(unsigned int doorIndex);

	// Returns the number of operable doors managed by this object.
	inline quint16 DoorCount() const
	{
		return m_DoorCount;
	}

	// Returns true if a door is closed.
	inline bool IsDoorClosed(unsigned int doorIndex) const
	{
		return !IsDoorOpen(doorIndex);
	}

	// Returns true if the crew door lock is set.
	inline bool IsCrewDoorLocked() const
	{
		return m_LockCrewDoors;
	}

	// Returns true if a door is jammed. It may be jammed open or jammed closed.
	bool IsDoorJammed(unsigned int doorIndex) const;

	// Returns true if a door is open.
	bool IsDoorOpen(unsigned int doorIndex) const;

	// Returns true if the hull door lock is set.
	inline bool IsHullDoorLocked() const
	{
		return m_LockHullDoors;
	}

	// Returns true if the state of the door changes
	bool JamDoorClosed(unsigned int doorIndex);

	// Returns true if the state of the door changes
	bool JamDoorOpen(unsigned int doorIndex);

	// Returns true if the state of the door changes
	bool OpenDoor(unsigned int doorIndex);

	// Streaming operator reads all data members.
	void operator<<(QDataStream& in);

	// Streaming operator writes all data members.
	void operator>>(QDataStream& out) const;

	// Copies only the portion of the door states data that is transmitted to the client using the Tx/Rx functions.
	// This includes the lock states because the parent space ship will have received them and set them into the refresh object.
	void Refresh(const AuDoorStates& c);

	// Receives only the subset of door states data that is sent to the scene client via Tx, which is just the byte array.
	// Client determines number of doors by analyzing the design.
	// Client does not need the change time or the actual number of operable doors.
	// Space ship receives lock states via a bit mask of switches.
	void Rx(QDataStream& in);

	// Sets the byte array data. Used when loading from a blob field, to retain backward compatibility when this information was
	// not all in a single object.
	// Does not update the last change time.
	inline void SetByteArray(const QByteArray& doorStates)
	{
		QByteArray::operator=(doorStates);
	}

	// Does not update the last change time.
	void SetCrewDoorLock(bool setLocked)
	{
		m_LockCrewDoors = setLocked;
	}

	// Resizes this array accordingly.
	void SetDoorCount(quint16 doors);

	// Does not update the last change time.
	void SetHullDoorLock(bool setLocked)
	{
		m_LockHullDoors = setLocked;
	}

	// Sends a subset of the door states data to the scene client. Not all data members are sent, only the underlying byte array.
	// Client determines number of doors by analyzing the design.
	// Client does not need the change time or the actual number of operable doors.
	// Space ship transmits lock states via a bit mask of switches.
	void Tx(QDataStream& out) const;

	// Returns true if the state of the door changes
	bool UnjamDoor(unsigned int doorIndex);

public:
	// 4 door states are stored per byte.
	// Door states must fit into 2 bits.
	// Bit0 - 0 = Closed, 1 = Open
	// Bit1 - 0 = Normal, 1 = Jammed
	enum DOOR_STATE
	{
		DOOR_Closed = 0,
		DOOR_Open = 1,
		DOOR_JammedClosed = 2,
		DOOR_JammedOpen = 3
	};
};

inline QDataStream& operator<<(QDataStream& out, const AuDoorStates& ds)
{
	ds >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDoorStates& ds)
{
	ds << in;
	return in;
}

#endif // AuDoorStates_H
