//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubodypartsres.h,v 1.3 2022/02/19 18:44:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBodyPartsRes_H
#define AuBodyPartsRes_H

#include <QtGui>

#if defined AUBODYPARTSRES
#define AUBODYPARTSRES_DLL Q_DECL_EXPORT
#else
#define AUBODYPARTSRES_DLL Q_DECL_IMPORT
#endif

class AUBODYPARTSRES_DLL AuBodyPartsRes
{
public:
	AuBodyPartsRes();
	virtual ~AuBodyPartsRes();
};

#endif // AuBodyPartsRes_H
