//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auvehiclespot.cpp,v 1.2 2024/02/07 23:04:50 daveh Exp $
// Copyright (c) 2024 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auvehiclespot.h"

const AuVehicleSpot& AuVehicleSpot::operator=(const AuVehicleSpot& c)
{
	m_CGHeight = c.m_CGHeight;
	m_Matrix = c.m_Matrix;
	return *this;
}

AuMatrix4x4 AuVehicleSpot::VehicleMatrix() const
{
	AuMatrix4x4 m(m_Matrix);
	m.Translate(0,0,m_CGHeight);
	return m;
}

