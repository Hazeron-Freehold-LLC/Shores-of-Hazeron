//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientframe.h,v 1.9 2022/02/19 18:39:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientFrame_H
#define AuClientFrame_H

#include "audesignerframe.h"

class AuDesignerView;

class AuClientFrame : public AuDesignerFrame
{
	Q_OBJECT;
public:
	AuClientFrame(QWidget *parent=0);
	virtual ~AuClientFrame();

	virtual AuDesignerView* CreateViewWidget();
};

#endif // AuClientFrame_H
