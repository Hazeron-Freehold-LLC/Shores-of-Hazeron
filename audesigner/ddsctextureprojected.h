//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctextureprojected.h,v 1.5 2022/02/19 18:24:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScTextureProjected_H
#define DdScTextureProjected_H

#include "ddcommand.h"

class DdScTextureProjected : public DdCommand  
{
	int									m_Layer;

public:
	DdScTextureProjected(int layer, AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScTextureProjected_H
