//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumakedesign.cpp,v 1.6 2022/02/19 17:40:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumakedesign.h"
//
//#include "augleffects.h"
//#include "auscreport.h"
//#include "daworldll.h"
//#include "demanufacturingprocess.h"
//#include "dgdata.h"
//#include <typeinfo>			// for bad_cast

void AuMakeDesign::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(m_IsReadyToSpawn)
		hasWhat |= 0x01;
	if(m_Quality)
		hasWhat |= 0x04;
	if(!m_OwnerId.IsNull())
		hasWhat |= 0x08;
	if(m_OwnerEmpire)
		hasWhat |= 0x20;
	out << hasWhat;
	out << m_DesignId;
	out << m_DesignName;
	if(hasWhat & 0x04)
		out << m_Quality;
	if(hasWhat & 0x08)
	{
		out << m_OwnerId;
		out << m_OwnerName;
	}
	if(hasWhat & 0x20)
		out << m_OwnerEmpire;
}

void AuMakeDesign::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	m_IsReadyToSpawn = hasWhat & 0x01;
	in >> m_DesignId;
	in >> m_DesignName;

	if(hasWhat & 0x04)
		in >> m_Quality;
	else
		m_Quality = 1;

	if(hasWhat & 0x08)
	{
		in >> m_OwnerId;
		in >> m_OwnerName;
	}
	else
		m_OwnerId = AuId(0);
	if(hasWhat & 0x20)
		in >> m_OwnerEmpire;
	else
		m_OwnerEmpire = 0;

	//DHH Tech nerf. Remove former development of Spacecraft Factory Technology.
	if(m_DesignId == 213)
		m_DesignId = 0;
}

quint64 AuMakeDesign::MemoryUsage() const
{
	quint64 mem = sizeof(AuMakeDesign);
	mem += StringMemoryUsage(m_DesignName);
	mem += StringMemoryUsage(m_OwnerName);
	return mem;
}
