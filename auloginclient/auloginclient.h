//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloginclient.h,v 1.4 2022/02/19 18:34:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLoginClient_H
#define AuLoginClient_H

#include <augl.h>						// MUST include before QtGui due to OpenGL definitions
#include <QtGui>

/* Defines needed for dll linkage.
#if defined AULOGINCLIENT
#define AULOGINCLIENT_DLL Q_DECL_EXPORT
#else
#define AULOGINCLIENT_DLL Q_DECL_IMPORT
#endif
*/
#define AULOGINCLIENT_DLL	// Defined as nothing for static lib.

class AULOGINCLIENT_DLL AuLoginClient
{
public:
	AuLoginClient();
	virtual ~AuLoginClient();
};

#endif // AuLoginClient_H
