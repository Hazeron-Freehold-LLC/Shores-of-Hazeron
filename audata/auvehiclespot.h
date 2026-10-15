//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auvehiclespot.h,v 1.2 2024/02/07 23:05:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuVehicleSpot_H
#define AuVehicleSpot_H

#include "audata.h"
#include "aumatrix4x4.h"

class AuVehicleSpot
{
	double								m_CGHeight;
	AuMatrix4x4							m_Matrix;

public:
	inline AuVehicleSpot(const AuMatrix4x4& m, double cgHeight) : m_Matrix(m)
	{
		m_CGHeight = cgHeight;
	}

	inline AuVehicleSpot()
	{
		m_CGHeight = 0;
	}

	inline AuVehicleSpot(const AuVehicleSpot& c) : m_Matrix(c.m_Matrix)
	{
		m_CGHeight = c.m_CGHeight;
	}

public:
	const AuVehicleSpot& operator=(const AuVehicleSpot& c);

public:
	inline double CGHeight() const
	{
		return m_CGHeight;
	}

	inline const AuMatrix4x4& Matrix() const
	{
		return m_Matrix;
	}

	// Returns matrix translated up by the cg height.
	AuMatrix4x4 VehicleMatrix() const;
};

#endif // AuVehicleSpot_H
