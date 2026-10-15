//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemspot.cpp,v 1.2 2024/02/07 23:04:40 daveh Exp $
// Copyright (c) 2024 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitemspot.h"

const AuItemSpot& AuItemSpot::operator=(const AuItemSpot& c)
{
	m_CGHeight = c.m_CGHeight;
	m_Matrix = c.m_Matrix;
	m_RoomName = c.m_RoomName;
	m_Switch = c.m_Switch;
	return *this;
}

AuPoint AuItemSpot::ItemLocation() const
{
	return m_Matrix.Project(0,0,m_CGHeight);
}

AuMatrix4x4 AuItemSpot::ItemMatrix() const
{
	AuMatrix4x4 m(m_Matrix);
	m.Translate(0,0,m_CGHeight);
	return m;
}

