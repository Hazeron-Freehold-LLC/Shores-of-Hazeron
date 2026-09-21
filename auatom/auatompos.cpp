//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatompos.cpp,v 1.5 2022/02/19 17:38:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatompos.h"
//
#include <QDebug>

void AuAtomPos::operator>>(QDataStream& out) const
{
	out << m_ParentId;
	out << m_Transform.Translation();
	out << m_Transform.GetRotation();
}

void AuAtomPos::operator<<(QDataStream& in)
{
	AuPoint translate, rotate;

	in >> m_ParentId;
	in >> translate;
	in >> rotate;

	m_Transform.LoadIdentity();
	m_Transform.Translate(translate);
	m_Transform.Rotate(rotate);
}

void AuAtomPos::Tx(QDataStream& out) const
{
	AuPoint rotate(m_Transform.GetRotation());
	float rx = (float)rotate.x;
	float ry = (float)rotate.y;
	float rz = (float)rotate.z;
	AuPoint translate(m_Transform.Translation());
	float tx = (float)translate.x;
	float ty = (float)translate.y;
	float tz = (float)translate.z;

	// Decide what to send.
	quint8 hasWhat = 0x00;
	if(!m_ParentId.IsNull())
		hasWhat |= 0x01;	// Parent id

	// Rotation
	if(rx != 0)
		hasWhat |= 0x10;
	if(ry != 0)
		hasWhat |= 0x20;
	if(rz != 0)
		hasWhat |= 0x40;

	// Translation
	if(fabs(translate.x) > 100000 || fabs(translate.y) > 100000 || fabs(translate.z) > 100000)
	{
		// Send doubles.
		hasWhat |= 0x80;
		if(translate.x != 0)
			hasWhat |= 0x02;
		if(translate.y != 0)
			hasWhat |= 0x04;
		if(translate.z != 0)
			hasWhat |= 0x08;
	}
	else
	{
		// Send floats.
		if(tx != 0)
			hasWhat |= 0x02;
		if(ty != 0)
			hasWhat |= 0x04;
		if(tz != 0)
			hasWhat |= 0x08;
	}

	// Transmit the data.
	out << hasWhat;
	if(hasWhat & 0x01)
		out << m_ParentId;

	// Rotation.
	if(hasWhat & 0x10)
		out << rx;
	if(hasWhat & 0x20)
		out << ry;
	if(hasWhat & 0x40)
		out << rz;

	// Translation
	if(hasWhat & 0x80)
	{
		// Send doubles.
		if(hasWhat & 0x02)
			out << translate.x;
		if(hasWhat & 0x04)
			out << translate.y;
		if(hasWhat & 0x08)
			out << translate.z;
	}
	else
	{
		// Send floats.
		if(hasWhat & 0x02)
			out << tx;
		if(hasWhat & 0x04)
			out << ty;
		if(hasWhat & 0x08)
			out << tz;
	}
}

void AuAtomPos::Rx(QDataStream& in)
{
	quint8 hasWhat;
	double tx, ty, tz;
	float rx, ry, rz;

	in >> hasWhat;

	// Parent id
	if(hasWhat & 0x01)
		in >> m_ParentId;

	// Rotation
	if(hasWhat & 0x10)
		in >> rx;
	else
		rx = 0;
	if(hasWhat & 0x20)
		in >> ry;
	else
		ry = 0;
	if(hasWhat & 0x40)
		in >> rz;
	else
		rz = 0;

	// Translation
	if(hasWhat & 0x80)
	{
		// Receive doubles.
		if(hasWhat & 0x02)
			in >> tx;
		else
			tx = 0;
		if(hasWhat & 0x04)
			in >> ty;
		else
			ty = 0;
		if(hasWhat & 0x08)
			in >> tz;
		else
			tz = 0;
	}
	else
	{
		// Receive floats.
		float f;
		if(hasWhat & 0x02)
		{
			in >> f;
			tx = f;
		}
		else
			tx = 0;
		if(hasWhat & 0x04)
		{
			in >> f;
			ty = f;
		}
		else
			ty = 0;
		if(hasWhat & 0x08)
		{
			in >> f;
			tz = f;
		}
		else
			tz = 0;
	}

	// Configure my transform.
	m_Transform.LoadIdentity();
	m_Transform.Translate(tx,ty,tz);
	m_Transform.Rotate(AuPoint(rx,ry,rz));
}

QDebug operator<<(QDebug dbg, const AuAtomPos& o)
{
	dbg.nospace() << o.ParentId() << o.Translation();
	return dbg.space();
}

