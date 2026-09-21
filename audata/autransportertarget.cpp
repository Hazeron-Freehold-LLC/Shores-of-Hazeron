//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autransportertarget.cpp,v 1.3 2022/02/19 18:20:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autransportertarget.h"
//

static int s_MT_AuTransporterTarget = AuTransporterTarget::RegisterMetaType();

//static
int AuTransporterTarget::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuTransporterTarget>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuTransporterTarget>();

	return metaType;
}

AuTransporterTarget::AuTransporterTarget(const AuId& id, const QString& name, TRANSPORTER_TARGET target)
{
	m_Id = id;
	m_Name = name;
	m_Target = target;
}

AuTransporterTarget::AuTransporterTarget()
{
	m_Target = TT_Invalid;
}

AuTransporterTarget::AuTransporterTarget(const AuTransporterTarget& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_Target = c.m_Target;
}

const AuTransporterTarget& AuTransporterTarget::operator=(const AuTransporterTarget& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_Target = c.m_Target;
	return *this;
}

bool AuTransporterTarget::operator==(const AuTransporterTarget& c)
{
	if(m_Id != c.m_Id)
		return false;
	if(m_Name != c.m_Name)
		return false;
	if(m_Target != c.m_Target)
		return false;
	return true;
}

void AuTransporterTarget::operator>>(QDataStream& out) const
{
	out << m_Id;
	out << (quint8)m_Target;
	out << m_Name;
}

void AuTransporterTarget::operator<<(QDataStream& in)
{
	in >> m_Id;
	quint8 ui8;
	in >> ui8;
	m_Target = (TRANSPORTER_TARGET)ui8;
	in >> m_Name;
}

const char* AuTransporterTarget::Icon() const
{
	return TTIcon(m_Target);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTransporterShip::AuTransporterShip(const AuId& id, const QString& name, const QString& spotDesc, TRANSPORTER_TARGET target)
{
	m_Id = id;
	m_Name = name;
	m_SpotDesc = spotDesc;
	m_Target = target;
}

AuTransporterShip::AuTransporterShip()
{
	m_Target = TT_Invalid;
}

AuTransporterShip::AuTransporterShip(const AuTransporterShip& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_SpotDesc = c.m_SpotDesc;
	m_Target = c.m_Target;
}

const AuTransporterShip& AuTransporterShip::operator=(const AuTransporterShip& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_SpotDesc = c.m_SpotDesc;
	m_Target = c.m_Target;
	return *this;
}

bool AuTransporterShip::operator==(const AuTransporterShip& c)
{
	if(m_Id != c.m_Id)
		return false;
	if(m_Name != c.m_Name)
		return false;
	if(m_SpotDesc != c.m_SpotDesc)
		return false;
	if(m_Target != c.m_Target)
		return false;
	return true;
}

void AuTransporterShip::operator>>(QDataStream& out) const
{
	out << m_Id;
	out << (quint8)m_Target;
	out << m_Name;
	out << m_SpotDesc;
}

void AuTransporterShip::operator<<(QDataStream& in)
{
	in >> m_Id;
	quint8 ui8;
	in >> ui8;
	m_Target = (TRANSPORTER_TARGET)ui8;
	in >> m_Name;
	in >> m_SpotDesc;
}

const char* AuTransporterShip::Icon() const
{
	return TTIcon(m_Target);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuTransporterSystem::AuTransporterSystem(const AuId& id, const QString& name, const QString& spotDesc, quint16 which)
{
	m_Id = id;
	m_Name = name;
	m_SpotDesc = spotDesc;
	m_Which = which;
}

AuTransporterSystem::AuTransporterSystem()
{
	m_Which = 0;
}

AuTransporterSystem::AuTransporterSystem(const AuTransporterSystem& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_SpotDesc = c.m_SpotDesc;
	m_Which = c.m_Which;
}

const AuTransporterSystem& AuTransporterSystem::operator=(const AuTransporterSystem& c)
{
	m_Id = c.m_Id;
	m_Name = c.m_Name;
	m_SpotDesc = c.m_SpotDesc;
	m_Which = c.m_Which;
	return *this;
}

bool AuTransporterSystem::operator==(const AuTransporterSystem& c)
{
	if(m_Id != c.m_Id)
		return false;
	if(m_Name != c.m_Name)
		return false;
	if(m_SpotDesc != c.m_SpotDesc)
		return false;
	if(m_Which != c.m_Which)
		return false;
	return true;
}

void AuTransporterSystem::operator>>(QDataStream& out) const
{
	out << m_Id;
	out << m_Which;
	out << m_Name;
	out << m_SpotDesc;
}

void AuTransporterSystem::operator<<(QDataStream& in)
{
	in >> m_Id;
	in >> m_Which;
	in >> m_Name;
	in >> m_SpotDesc;
}

const char* AuTransporterSystem::Icon() const
{
	return TTIcon(TT_Transporter);
}

