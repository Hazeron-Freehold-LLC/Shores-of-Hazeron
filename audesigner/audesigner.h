//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesigner.h,v 1.5 2022/02/19 18:21:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesigner_H
#define AuDesigner_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>

// To change linkage, must also set dll|staticlib in CONFIG in reqt.opt and reqtr.bat.
//#define AUDESIGNER_DYNAMIC
#if defined(AUDESIGNER_DYNAMIC)
#if defined AUDESIGNER
#define AUDESIGNER_DLL Q_DECL_EXPORT	// Define function exports for internal build.
#else
#define AUDESIGNER_DLL Q_DECL_IMPORT	// Define function imports for external builds.
#endif
#else	// AUDESIGNER_DYNAMIC
#define AUDESIGNER_DLL					// Defined as nothing for static lib.
#endif	// AUDESIGNER_DYNAMIC

class AUDESIGNER_DLL AuDesigner
{
public:
	AuDesigner();
	virtual ~AuDesigner();
};

#endif // AuDesigner_H
