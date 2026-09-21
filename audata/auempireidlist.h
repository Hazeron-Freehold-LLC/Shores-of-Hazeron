//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempireidlist.h,v 1.2 2022/02/19 18:19:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEmpireIdList_H
#define AuEmpireIdList_H

#include "audata.h"
#include <QList>

class AUDATA_DLL AuEmpireIdList : public QList<quint32>
{
public:
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);
};

#endif // AuEmpireIdList_H
