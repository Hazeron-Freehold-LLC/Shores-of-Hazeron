//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscjigcut.h,v 1.5 2022/02/19 18:23:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScJigCut_H
#define DdScJigCut_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScJigCut : public DdCommand  
{
public:
	DdScJigCut(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScJigCut_H
