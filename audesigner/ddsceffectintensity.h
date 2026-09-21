//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsceffectintensity.h,v 1.3 2022/02/19 18:23:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScEffectIntensity_H
#define DdScEffectIntensity_H

#include "ddcommand.h"

class DdScEffectIntensity : public DdCommand  
{
public:
	DdScEffectIntensity(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScEffectIntensity_H
