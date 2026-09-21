//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunitdamage.cpp,v 1.5 2022/02/19 17:41:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auunitdamage.h"
//

void AuUnitDamage::operator<<(QDataStream& in)
{
	in >> m_CID;
	in >> m_iDamage;
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_LastDamage = tm;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

void AuUnitDamage::operator>>(QDataStream& out) const
{
	out << m_CID;
	out << m_iDamage;
	out << m_LastDamage.UTC();
	out << m_Quality;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuUnitDamageHash::MemoryUsage() const
{
	quint64 mem = (sizeof(AuId) + sizeof(AuUnitDamage)) * size();
	return mem;
}
