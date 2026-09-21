//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumaillib.h,v 1.5 2022/02/19 18:36:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailLib_H
#define AuMailLib_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>

/* Defines needed for dll linkage.
#if defined AUMAIL
#define AUMAIL_DLL Q_DECL_EXPORT
#else
#define AUMAIL_DLL Q_DECL_IMPORT
#endif
*/
#define AUMAIL_DLL	// Defined as nothing for static lib.

class AUMAIL_DLL AuMailLib
{
public:
	AuMailLib();
	virtual ~AuMailLib();
};

#endif // AuMailLib_H
