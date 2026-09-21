//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austubglobal.h,v 1.4 2022/02/19 18:40:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStubGlobal_H
#define AuStubGlobal_H

#include "auglobal.h"
#include <QMutex>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStubGlobal : public AuGlobal
{
public:
	AuStubGlobal(const QString& appName, const QString& company, AuGlobal::MODE mode, DgData* data, AuEther* etherium=0);

	virtual void AtomChanged(const AuId& id)
	{
		qDebug() << "AuStubGlobal::AtomChanged" << id;
	}
};

#define g_AuStub		((AuStubGlobal&)(*gp_AuGlobal))

#endif // AuStubGlobal_H
