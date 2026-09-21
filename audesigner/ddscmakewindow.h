//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakewindow.h,v 1.5 2022/02/19 18:24:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeWindow_H
#define DdScMakeWindow_H

#include "ddcommand.h"

class DdScMakeWindow : public DdCommand  
{
public:
	DdScMakeWindow(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeWindow_H
