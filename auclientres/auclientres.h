//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientres.h,v 1.2 2022/02/19 18:17:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientRes_H
#define AuClientRes_H

#include <QtGui>

#if defined AUCLIENTRES
#define AUCLIENTRES_DLL Q_DECL_EXPORT
#else
#define AUCLIENTRES_DLL Q_DECL_IMPORT
#endif

class AUCLIENTRES_DLL AuClientRes
{
public:
	AuClientRes();
	virtual ~AuClientRes();
};

#endif // AuClientRes_H
