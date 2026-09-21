//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempireidlist.cpp,v 1.2 2022/02/19 18:20:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auempireidlist.h"
//

void AuEmpireIdList::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() <= 0xffff);
	out << (quint16)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		out << *it;
}

void AuEmpireIdList::operator<<(QDataStream& in)
{
	clear();
	quint16 num;
	in >> num;
	for(;num ;num--)
	{
		quint32 empireid;
		in >> empireid;
		append(empireid);
	}
}
