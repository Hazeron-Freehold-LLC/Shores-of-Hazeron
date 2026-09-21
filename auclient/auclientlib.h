//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientlib.h,v 1.8 2022/02/19 17:54:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientLib_H
#define AuClientLib_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>

// To change linkage, must also set dll|staticlib in CONFIG in reqt.opt and reqtr.bat.
//#define AUCLIENT_DYNAMIC
#if defined(AUCLIENT_DYNAMIC)
#if defined AUCLIENT
#define AUCLIENT_DLL Q_DECL_EXPORT		// Define function exports for internal build.
#else
#define AUCLIENT_DLL Q_DECL_IMPORT		// Define function imports for external builds.
#endif
#else	// AUCLIENT_DYNAMIC
#define AUCLIENT_DLL						// Defined as nothing for static lib.
#endif	// AUCLIENT_DYNAMIC

class AUCLIENT_DLL AuClientLib
{
public:
	AuClientLib();
	virtual ~AuClientLib();
};

#endif // AuClientLib_H
