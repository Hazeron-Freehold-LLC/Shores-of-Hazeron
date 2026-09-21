//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auviewerglobal.h,v 1.4 2022/02/19 18:42:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuViewerGlobal_H
#define AuViewerGlobal_H

#include "auglobal.h"

class AuViewerGlobal : public AuGlobal
{
public:
	AuViewerGlobal(const QString& appName, const QString& company, DgData* data, AuEther* etherium=0);

	virtual ~AuViewerGlobal();

public:
	virtual AuId NextAtomId(unsigned int howMany=1);
};

#define g_AuViewer		((AuViewerGlobal&)(*gp_AuGlobal))

#endif // AuViewerGlobal_H
