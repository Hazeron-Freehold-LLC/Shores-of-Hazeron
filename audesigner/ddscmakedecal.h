//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakedecal.h,v 1.5 2022/02/19 18:23:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeDecal_H
#define DdScMakeDecal_H

#include "ddcommand.h"

class DdScMakeDecal : public DdCommand  
{
public:
	DdScMakeDecal(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeDecal_H
