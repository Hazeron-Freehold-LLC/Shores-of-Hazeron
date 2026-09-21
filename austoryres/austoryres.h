//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austoryres.h,v 1.3 2022/02/19 18:41:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStoryRes_H
#define AuStoryRes_H

#include <QtGui>

#if defined AUSTORYRES
#define AUSTORYRES_DLL Q_DECL_EXPORT
#else
#define AUSTORYRES_DLL Q_DECL_IMPORT
#endif

class AUSTORYRES_DLL AuStoryRes
{
public:
	AuStoryRes();
	virtual ~AuStoryRes();
};

#endif // AuStoryRes_H
