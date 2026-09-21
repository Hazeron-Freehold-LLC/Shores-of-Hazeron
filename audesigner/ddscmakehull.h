//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakehull.h,v 1.5 2022/02/19 18:23:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeHull_H
#define DdScMakeHull_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeHull : public DdCommand  
{
public:
	DdScMakeHull(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeHull_H
