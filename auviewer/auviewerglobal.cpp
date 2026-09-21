//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auviewerglobal.cpp,v 1.4 2022/02/19 18:42:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auviewerglobal.h"
//

static AuId s_NextSingleUserAtomId;								// Used in single-user client-only mode to provide atom ids.

AuViewerGlobal::AuViewerGlobal(const QString& appName, const QString& company, DgData* data, AuEther* ether) :
	AuGlobal(appName, company, AuGlobal::WebCGI, data, ether)
{
}

AuViewerGlobal::~AuViewerGlobal()
{
}

AuId AuViewerGlobal::NextAtomId(unsigned int howMany)
{
	if(s_NextSingleUserAtomId.IsNull())
		s_NextSingleUserAtomId = AuId(g_Au.Dice().Roll(2,100000000));			// So single-user mode has random ids.

	AuId retId = s_NextSingleUserAtomId;
	s_NextSingleUserAtomId = s_NextSingleUserAtomId + howMany;
	return retId;
}
