//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctexturecylindrical.h,v 1.5 2022/02/19 18:24:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScTextureCylindrical_H
#define DdScTextureCylindrical_H

#include "ddcommand.h"

class DdScTextureCylindrical : public DdCommand  
{
	int									m_Layer;

public:
	DdScTextureCylindrical(int layer, AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScTextureCylindrical_H
