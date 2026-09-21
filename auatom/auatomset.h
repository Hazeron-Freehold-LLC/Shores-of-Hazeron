//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomset.h,v 1.2 2022/02/19 17:30:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomSet_H
#define AuAtomSet_H

#include "auatom.h"
#include <QSet>

class AuIdList;
class DaAtom;

class AUATOM_DLL AuAtomSet : public QSet<DaAtom*>
{
public:
	void ListIds(AuIdList& ids) const;

	void ListIdsNoInstance(AuIdList& ids) const;

	void ListNormalIds(AuIdList& ids) const;

	void ListNormalIdsNoInstance(AuIdList& ids) const;
};

#endif // AuAtomSet_H
