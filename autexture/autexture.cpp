//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autexture.cpp,v 1.5 2022/02/19 18:41:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autexture.h"
//

AuTexture::AuTexture()
{
	//qDebug() << "AuTexture::AuTexture Init";
	//Q_INIT_RESOURCE(AuTexture);	// Only if I have a .qrc file.
	Q_INIT_RESOURCE(AuScTex1);
	Q_INIT_RESOURCE(AuScTex2);
	Q_INIT_RESOURCE(AuScTex3);
	Q_INIT_RESOURCE(AuScTex4);
	Q_INIT_RESOURCE(AuScTex5);
	Q_INIT_RESOURCE(AuShipTex0);
	//qDebug() << "AuTexture::AuTexture Ready";
}

AuTexture::~AuTexture()
{
	//qDebug() << "AuTexture::AuTexture Destroyed";
}
