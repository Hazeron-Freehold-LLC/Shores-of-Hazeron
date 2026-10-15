//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemspot.h,v 1.2 2024/02/07 23:05:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItemSpot_H
#define AuItemSpot_H

#include "audata.h"
#include "aumatrix4x4.h"

class AuItemSpot
{
	double								m_CGHeight;
	AuMatrix4x4							m_Matrix;
	QString								m_RoomName;
	quint8								m_Switch;

public:
	inline AuItemSpot(const AuMatrix4x4& m, double cgHeight, quint8 switchState, const QString& roomName) :
		m_Matrix(m), m_RoomName(roomName)
	{
		m_CGHeight = cgHeight;
		m_Switch = switchState;
	}

	inline AuItemSpot()
	{
		m_CGHeight = 0;
		m_Switch = 0;
	}

	inline AuItemSpot(const AuItemSpot& c) : m_Matrix(c.m_Matrix), m_RoomName(c.m_RoomName)
	{
		m_CGHeight = c.m_CGHeight;
		m_Switch = c.m_Switch;
	}

public:
	const AuItemSpot& operator=(const AuItemSpot& c);

public:
	inline double CGHeight() const
	{
		return m_CGHeight;
	}

	// Calculate the location at CG height above the matrix origin.
	AuPoint ItemLocation() const;

	// Returns matrix translated up by the cg height.
	AuMatrix4x4 ItemMatrix() const;

	inline const AuMatrix4x4& Matrix() const
	{
		return m_Matrix;
	}

	inline const QString& RoomName() const
	{
		return m_RoomName;
	}

	inline quint8 SwitchState() const
	{
		return m_Switch;
	}
};

#endif // AuItemSpot_H
