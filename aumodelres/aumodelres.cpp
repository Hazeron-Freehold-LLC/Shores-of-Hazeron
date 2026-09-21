//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumodelres.cpp,v 1.6 2022/02/28 21:51:50 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumodelres.h"
//

AuModelRes::AuModelRes()
{
	//qDebug() << "AuModelRes::AuModelRes Init";
	//Q_INIT_RESOURCE(AuModelRes);	// Only if I have a .qrc file.
	Q_INIT_RESOURCE(AuModel1);
	Q_INIT_RESOURCE(AuModel2);
	Q_INIT_RESOURCE(AuModel3);
	Q_INIT_RESOURCE(AuModel4);
	Q_INIT_RESOURCE(AuModel5);
	Q_INIT_RESOURCE(AuModel6);
	Q_INIT_RESOURCE(AuModel7);
	Q_INIT_RESOURCE(AuModel8);
	Q_INIT_RESOURCE(AuModel9);
	Q_INIT_RESOURCE(AuModel10);
	//qDebug() << "AuModelRes::AuModelRes Ready";
}

AuModelRes::~AuModelRes()
{
	//qDebug() << "AuModelRes::AuModelRes Destroyed";
}
