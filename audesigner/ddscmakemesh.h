//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakemesh.h,v 1.5 2022/02/19 18:23:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScMakeMesh_H
#define DdScMakeMesh_H

#include "aumatrix4x4.h"
#include "ddcommand.h"

class DdScMakeMesh : public DdCommand  
{
public:
	DdScMakeMesh(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScMakeMesh_H
