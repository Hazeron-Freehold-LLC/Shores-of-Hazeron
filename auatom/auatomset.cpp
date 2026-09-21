//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomset.cpp,v 1.2 2022/02/19 17:39:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomset.h"
//
#include "auglobal.h"
#include "daatom.h"

void AuAtomSet::ListIds(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		ids.append((*it)->Id());
}

void AuAtomSet::ListNormalIds(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal())
			ids.append((*it)->Id());
}

void AuAtomSet::ListIdsNoInstance(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if(!(*it)->IsInstanceUniverse())
			ids.append((*it)->Id());
}

void AuAtomSet::ListNormalIdsNoInstance(AuIdList& ids) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it)->IsNormal() && !(*it)->IsInstanceUniverse())
			ids.append((*it)->Id());
}

