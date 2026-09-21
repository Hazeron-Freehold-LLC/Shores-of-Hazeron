//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakelandinggear.h,v 1.5 2022/02/19 18:23:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeLandingGear_H
#define DdScMakeLandingGear_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeLandingGear : public DdCommand  
{
public:
	DdScMakeLandingGear(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeLandingGear_H
