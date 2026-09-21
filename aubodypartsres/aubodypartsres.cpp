//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubodypartsres.cpp,v 1.6 2022/02/19 18:48:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubodypartsres.h"
//

AuBodyPartsRes::AuBodyPartsRes()
{
	//qDebug() << "AuBodyPartsRes::AuBodyPartsRes Init";
	//Q_INIT_RESOURCE(AuBodyPartsRes);	// Only if I have a .qrc file.
	Q_INIT_RESOURCE(AuBodyParts0);
	Q_INIT_RESOURCE(AuBodyParts1);
	Q_INIT_RESOURCE(AuBodyParts2);
	Q_INIT_RESOURCE(AuBodyParts3);
	Q_INIT_RESOURCE(AuBodyParts4);
	Q_INIT_RESOURCE(AuBodyParts5);
	Q_INIT_RESOURCE(AuBodyParts6);
	Q_INIT_RESOURCE(AuBodyParts7);
	Q_INIT_RESOURCE(AuPowerArmor);
	//qDebug() << "AuBodyPartsRes::AuBodyPartsRes Ready";
}

AuBodyPartsRes::~AuBodyPartsRes()
{
	//qDebug() << "AuBodyPartsRes::AuBodyPartsRes Destroyed";
}
