//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audata.h,v 1.5 2022/02/19 18:18:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuData_H
#define AuData_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>
#include <unordered_map>

/* Defines needed for dll linkage.
#if defined AUDATA
#define AUDATA_DLL Q_DECL_EXPORT
#else
#define AUDATA_DLL Q_DECL_IMPORT
#endif
*/
#define AUDATA_DLL	// Defined as nothing for static lib.

class AUDATA_DLL AuData
{
public:
	AuData();
	virtual ~AuData();
};

#endif // AuData_H
