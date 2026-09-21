//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatom.h,v 1.4 2022/02/19 17:30:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtom_H
#define AuAtom_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>

/* Defines needed for dll linkage.
#if defined AUATOM
#define AUATOM_DLL Q_DECL_EXPORT
#else
#define AUATOM_DLL Q_DECL_IMPORT
#endif
*/
#define AUATOM_DLL	// Defined as nothing for static lib.

class AUATOM_DLL AuAtom
{
public:
	AuAtom();
	virtual ~AuAtom();
};

#endif // AuAtom_H
