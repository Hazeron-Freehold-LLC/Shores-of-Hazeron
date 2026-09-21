//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakestrobelens.h,v 1.5 2022/02/19 18:23:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeStrobeLens_H
#define DdScMakeStrobeLens_H

#include "ddcommand.h"

class DdScMakeStrobeLens : public DdCommand  
{
public:
	DdScMakeStrobeLens(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeStrobeLens_H
