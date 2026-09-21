//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakerotatingbeaconlens.h,v 1.5 2022/02/19 18:23:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeRotatingBeaconLens_H
#define DdScMakeRotatingBeaconLens_H

#include "ddcommand.h"

class DdScMakeRotatingBeaconLens : public DdCommand  
{
public:
	DdScMakeRotatingBeaconLens(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeRotatingBeaconLens_H
