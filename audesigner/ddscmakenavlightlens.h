//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakenavlightlens.h,v 1.5 2022/02/19 18:23:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeNavLightLens_H
#define DdScMakeNavLightLens_H

#include "ddcommand.h"

class DdScMakeNavLightLens : public DdCommand  
{
public:
	DdScMakeNavLightLens(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeNavLightLens_H
