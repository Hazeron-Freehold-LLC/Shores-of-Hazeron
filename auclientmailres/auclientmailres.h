//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientmailres.h,v 1.2 2022/02/19 18:17:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientMailRes_H
#define AuClientMailRes_H

#include <QtGui>

#if defined AUCLIENTMAILRES
#define AUCLIENTMAILRES_DLL Q_DECL_EXPORT
#else
#define AUCLIENTMAILRES_DLL Q_DECL_IMPORT
#endif

class AUCLIENTMAILRES_DLL AuClientMailRes
{
public:
	AuClientMailRes();
	virtual ~AuClientMailRes();
};

#endif // AuClientMailRes_H
