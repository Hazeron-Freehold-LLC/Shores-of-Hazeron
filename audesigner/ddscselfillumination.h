//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscselfillumination.h,v 1.5 2022/02/19 18:24:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScSelfIllumination_H
#define DdScSelfIllumination_H

#include "ddcommand.h"

class DdScSelfIllumination : public DdCommand  
{
public:
	DdScSelfIllumination(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScSelfIllumination_H
