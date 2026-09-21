//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austubglobal.cpp,v 1.5 2022/02/19 18:28:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austubglobal.h"
//

AuStubGlobal::AuStubGlobal(const QString& appName, const QString& company, AuGlobal::MODE mode, DgData* data, AuEther* ether) :
	AuGlobal(appName, company, mode, data, ether)
{
}

void AuStubGlobal::AtomChanged(const AuId& id)
{
	Q_UNUSED(id);
	//qDebug() << "AuStubGlobal::AtomChanged" << id;
}
