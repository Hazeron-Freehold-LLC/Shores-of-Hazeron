//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscvoidassociate.h,v 1.5 2022/02/19 18:24:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScVoidAssociate_H
#define DdScVoidAssociate_H

#include "ddcommand.h"

class DdScVoidAssociate : public DdCommand  
{
public:
	DdScVoidAssociate(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScVoidAssociate_H
