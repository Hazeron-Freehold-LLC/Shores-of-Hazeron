//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmaketurbolift.h,v 1.5 2022/02/19 18:23:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeTurboLift_H
#define DdScMakeTurboLift_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeTurboLift : public DdCommand  
{
public:
	DdScMakeTurboLift(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeTurboLift_H
