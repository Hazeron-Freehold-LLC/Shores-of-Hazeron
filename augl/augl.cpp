//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augl.cpp,v 1.4 2022/02/19 18:29:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augl.h"
//
#include <QDebug>

AuGL::AuGL()
{
	//qDebug() << "AuGL::AuGL Init";
	Q_INIT_RESOURCE(AuGL);
	//qDebug() << "AuGL::AuGL Ready";
}
