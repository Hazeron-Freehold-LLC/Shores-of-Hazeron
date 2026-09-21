//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakeroom.h,v 1.5 2022/02/19 18:23:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeRoom_H
#define DdScMakeRoom_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeRoom : public DdCommand  
{
public:
	DdScMakeRoom(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeRoom_H
