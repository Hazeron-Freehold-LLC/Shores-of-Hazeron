//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumodelres.h,v 1.2 2022/02/19 18:38:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuModelRes_H
#define AuModelRes_H

#include <QtGui>

#if defined AUMODELRES
#define AUMODELRES_DLL Q_DECL_EXPORT
#else
#define AUMODELRES_DLL Q_DECL_IMPORT
#endif

class AUMODELRES_DLL AuModelRes
{
public:
	AuModelRes();
	virtual ~AuModelRes();
};

#endif // AuModelRes_H
