//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscimportstl.h,v 1.5 2022/02/19 18:23:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScImportSTL_H
#define DdScImportSTL_H

#include "audesign.h"
#include "ddcommand.h"

class DdScImportSTL : public DdCommand  
{
	AuDesignTex							m_Palette[SCTEXI_Count];

public:
	DdScImportSTL(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScImportSTL_H
