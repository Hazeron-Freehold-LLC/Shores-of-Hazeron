//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsccopysymmetrical.h,v 1.5 2022/02/19 18:23:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScCopySymmetrical_H
#define DdScCopySymmetrical_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScCopySymmetrical : public DdCommand  
{
public:
	DdScCopySymmetrical(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScCopySymmetrical_H
