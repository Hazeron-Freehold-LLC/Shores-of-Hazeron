//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augloballib.h,v 1.6 2022/02/19 18:30:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGlobalLib_H
#define AuGlobalLib_H

#include <QtCore>

/* Defines needed for dll linkage.
#if defined AUGLOBAL
#define AUGLOBAL_DLL Q_DECL_EXPORT
#else
#define AUGLOBAL_DLL Q_DECL_IMPORT
#endif
*/
#define AUGLOBAL_DLL	// Defined as nothing for static lib.

class AUGLOBAL_DLL AuGlobalLib
{
public:
	AuGlobalLib();
	virtual ~AuGlobalLib();
};

#endif // AuGlobalLib_H
