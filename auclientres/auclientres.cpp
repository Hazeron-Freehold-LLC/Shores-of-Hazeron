//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientres.cpp,v 1.2 2022/02/19 18:18:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientres.h"
//

AuClientRes::AuClientRes()
{
	//qDebug() << "AuClientRes::AuClientRes Init";
	Q_INIT_RESOURCE(AuClientRes);
	Q_INIT_RESOURCE(AuClientRes1);
	//qDebug() << "AuClientRes::AuClientRes Ready";
}

AuClientRes::~AuClientRes()
{
	//qDebug() << "AuClientRes::AuClientRes Destroyed";
}
