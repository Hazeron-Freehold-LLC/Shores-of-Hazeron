//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austoryres.cpp,v 1.3 2022/02/19 18:41:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austoryres.h"
//

AuStoryRes::AuStoryRes()
{
	//qDebug() << "AuStoryRes::AuStoryRes Loading Resources";
	Q_INIT_RESOURCE(AuStoryRes);
	//qDebug() << "AuStoryRes::AuStoryRes Ready";
}

AuStoryRes::~AuStoryRes()
{
	//qDebug() << "AuStoryRes::~AuStoryRes Destroyed";
}
