//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctexturespherical.h,v 1.5 2022/02/19 18:24:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScTextureSpherical_H
#define DdScTextureSpherical_H

#include "ddcommand.h"

class DdScTextureSpherical : public DdCommand  
{
	int									m_Layer;

public:
	DdScTextureSpherical(int layer, AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScTextureSpherical_H
