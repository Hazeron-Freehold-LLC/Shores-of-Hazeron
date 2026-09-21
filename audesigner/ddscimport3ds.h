//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscimport3ds.h,v 1.5 2022/02/19 18:23:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScImport3ds_H
#define DdScImport3ds_H

#include "audesign.h"
#include "ddcommand.h"

class DdScImport3ds : public DdCommand  
{
	AuDesignTex							m_Palette[SCTEXI_Count];

public:
	DdScImport3ds(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScImport3ds_H
