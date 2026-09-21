//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpaint.h,v 1.5 2022/02/19 18:24:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScPaint_H
#define DdScPaint_H

#include "ddcommand.h"

class DdScPaint : public DdCommand  
{
public:
	DdScPaint(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScPaint_H
