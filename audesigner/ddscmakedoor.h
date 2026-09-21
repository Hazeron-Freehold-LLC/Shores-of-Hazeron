//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakedoor.h,v 1.5 2022/02/19 18:23:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeDoor_H
#define DdScMakeDoor_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeDoor : public DdCommand  
{
public:
	DdScMakeDoor(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeDoor_H
