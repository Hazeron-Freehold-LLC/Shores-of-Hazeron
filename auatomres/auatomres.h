//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomres.h,v 1.4 2022/02/19 18:44:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomRes_H
#define AuAtomRes_H

#include <QtGui>

#if defined AUATOMRES
#define AUATOMRES_DLL Q_DECL_EXPORT
#else
#define AUATOMRES_DLL Q_DECL_IMPORT
#endif

class AUATOMRES_DLL AuAtomRes
{
public:
	AuAtomRes();
	virtual ~AuAtomRes();
};

#endif // AuAtomRes_H
