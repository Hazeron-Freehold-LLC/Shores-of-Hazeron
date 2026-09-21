//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausynccontroller.cpp,v 1.5 2022/04/21 22:52:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausynccontroller.h"
//
#include <QDebug>
#include <typeinfo>						// for std::bad_cast

//static
AuSyncController* AuSyncController::Create(TYPE type)
{
	switch(type)
	{
	case SYNC_AtomPos:
		return new AuSyncAtomPos();
	case SYNC_Vector:
		return new AuSyncVector();
	case SYNC_Motion:
		return new AuSyncMotion();
	case SYNC_AtomPosVec:
		return new AuSyncAtomPosVec();
	case SYNC_Pose:
		return new AuSyncPose();
	}
	return 0;
}

QDebug operator<<(QDebug dbg, const AuSyncController& d)
{
	dbg.nospace() << "AuSyncController " << d.TypeName() << " par " << d.Pos().ParentId()
		<< " loc " << d.Pos().Transform().Translation() << "vec" << d.Vector();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncControllerList::operator>>(QDataStream& out) const
{
	quint8 c;
	if(size() <= 255)
		c = size();
	else
	{
		c = 255;
		qWarning() << "warning: AuSyncControllerList::operator>> has too many entries" << size();
	}

	out << c;
	for(const_iterator di = cbegin(); di != cend() && c; di++, c--)
	{
		out << (quint8)(*di)->Type();
		out << *(*di);
	}
}

void AuSyncControllerList::operator<<(QDataStream& in)
{
	Clear();
	Append(in);
}

void AuSyncControllerList::Append(QDataStream& in)
{
	quint8 c;
	in >> c;
	while(c--)
	{
		quint8 t;
		in >> t;
		AuSyncController* d = AuSyncController::Create((AuSyncController::TYPE)t);
		if(d)
		{
			in >> *d;
			append(d);
		}
		else
			qWarning() << "error: AuSyncControllerList::Append read bad type" << t;
	}
}

void AuSyncControllerList::AppendReplace(const AuSyncController& d)
{
	// Remove any that are replaced by 'd'.
	iterator ri = begin();
	while(ri != end())
	{
		if((*ri)->IsReplacedBy(d))
		{
			delete *ri;
			ri = erase(ri);
		}
		else
			ri++;
	}
	// Append a clone of 'd'.
	append(d.Clone());
}

QDebug operator<<(QDebug dbg, const AuSyncControllerList& d)
{
	dbg.nospace() << "AuSyncControllerList " << d.size() << " items\n";
	for(AuSyncControllerList::const_iterator ci = d.cbegin(); ci != d.cend(); ci++)
		dbg.space() << **ci << "\n";
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncAtomPos::operator>>(QDataStream& out) const
{
	out << m_Pos;
}

void AuSyncAtomPos::operator<<(QDataStream& in)
{
	in >> m_Pos;
}

bool AuSyncAtomPos::IsReplacedBy(const AuSyncController& c) const
{
	try
	{
		const AuSyncAtomPos& s = dynamic_cast<const AuSyncAtomPos&>(c);
		Q_UNUSED(s);
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncVector::operator>>(QDataStream& out) const
{
	out << m_Vector;
}

void AuSyncVector::operator<<(QDataStream& in)
{
	in >> m_Vector;
}

bool AuSyncVector::IsReplacedBy(const AuSyncController& c) const
{
	try
	{
		const AuSyncVector& s = dynamic_cast<const AuSyncVector&>(c);
		Q_UNUSED(s);
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncMotion::operator>>(QDataStream& out) const
{
	Q_UNUSED(out);
}

void AuSyncMotion::operator<<(QDataStream& in)
{
	Q_UNUSED(in);
}

bool AuSyncMotion::IsReplacedBy(const AuSyncController& c) const
{
	try
	{
		const AuSyncMotion& s = dynamic_cast<const AuSyncMotion&>(c);
		Q_UNUSED(s);
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncAtomPosVec::operator>>(QDataStream& out) const
{
	AuSyncAtomPos::operator>>(out);
	quint8 hasWhat = m_AtRest ? 0x01 : 0x00;
	if(!m_Vector.IsZero())
		hasWhat |= 0x02;

	out << hasWhat;
	if(hasWhat & 0x02)
		out << m_Vector;
}

void AuSyncAtomPosVec::operator<<(QDataStream& in)
{
	AuSyncAtomPos::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	
	m_AtRest = hasWhat & 0x01;
	if(hasWhat & 0x02)
		in >> m_Vector;
}

bool AuSyncAtomPosVec::IsReplacedBy(const AuSyncController& c) const
{
	try
	{
		const AuSyncAtomPosVec& s = dynamic_cast<const AuSyncAtomPosVec&>(c);
		Q_UNUSED(s);
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuSyncPose::operator>>(QDataStream& out) const
{
	out << m_HeadTilt;
	out << m_Pose;
	out << m_SeatIndex;
	out << m_StationId;
}

void AuSyncPose::operator<<(QDataStream& in)
{
	in >> m_HeadTilt;
	in >> m_Pose;
	in >> m_SeatIndex;
	in >> m_StationId;
}

bool AuSyncPose::IsReplacedBy(const AuSyncController& c) const
{
	try
	{
		const AuSyncPose& s = dynamic_cast<const AuSyncPose&>(c);
		Q_UNUSED(s);
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

