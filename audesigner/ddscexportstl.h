//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscexportstl.h,v 1.5 2022/02/19 18:23:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScExportSTL_H
#define DdScExportSTL_H

#include "audesign.h"
#include "ddcommand.h"

class DdScExportSTL : public DdCommand  
{
	AuDesignTex							m_Palette[SCTEXI_Count];

public:
	DdScExportSTL(AuDesignerView* view);

	virtual void Start();

	virtual QString WikiTopic() const;
};

#endif // DdScExportSTL_H
