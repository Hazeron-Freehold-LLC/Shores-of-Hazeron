//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augl.h,v 1.13 2022/02/19 18:29:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGL_H
#define AuGL_H

/* Defines needed for dll linkage.
#if defined AUGL
#define AUGL_DLL Q_DECL_EXPORT
#else
#define AUGL_DLL Q_DECL_IMPORT
#endif
*/
#define AUGL_DLL	// Defined as nothing for static lib.

#include <qgl.h>

class AUGL_DLL AuGL
{
public:
	AuGL();
};

#endif // AuGL_H
