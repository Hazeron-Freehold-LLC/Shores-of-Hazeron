//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientmailres.cpp,v 1.2 2022/02/19 18:17:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientmailres.h"
//

AuClientMailRes::AuClientMailRes()
{
	//qDebug() << "AuClientMailRes::AuClientMailRes Init";
	Q_INIT_RESOURCE(AuClientMailRes);
	//qDebug() << "AuClientMailRes::AuClientMailRes Ready";
}

AuClientMailRes::~AuClientMailRes()
{
	//qDebug() << "AuClientMailRes::AuClientMailRes Destroyed";
}
