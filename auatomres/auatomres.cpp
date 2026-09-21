//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomres.cpp,v 1.23 2022/02/19 18:47:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomres.h"
//

AuAtomRes::AuAtomRes()
{
	//qDebug() << "AuAtomRes::AuAtomRes Init";
	Q_INIT_RESOURCE(AuAtomRes);
	Q_INIT_RESOURCE(AuAtomR1);
	Q_INIT_RESOURCE(AuAtomR2);
	Q_INIT_RESOURCE(AuAtomR3);
	Q_INIT_RESOURCE(AuAtomR4);
	Q_INIT_RESOURCE(AuAtomR5);
	Q_INIT_RESOURCE(AuAtomR6);
	//qDebug() << "AuAtomRes::AuAtomRes Ready";
}

AuAtomRes::~AuAtomRes()
{
	//qDebug() << "AuAtomRes::~AuAtomRes Destroyed";
}
