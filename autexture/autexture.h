//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autexture.h,v 1.2 2022/02/19 18:41:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTexture_H
#define AuTexture_H

#include <QtGui>

#if defined AUTEXTURE
#define AUTEXTURE_DLL Q_DECL_EXPORT
#else
#define AUTEXTURE_DLL Q_DECL_IMPORT
#endif

class AUTEXTURE_DLL AuTexture
{
public:
	AuTexture();
	virtual ~AuTexture();
};

#endif // AuTexture_H
