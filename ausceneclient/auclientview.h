//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientview.h,v 1.7 2022/02/19 18:39:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientView_H
#define AuClientView_H

#include "audesignerview.h"

class AuClientView : public AuDesignerView
{
	Q_OBJECT
public:
	AuClientView(QWidget *parent=0);
	~AuClientView();
};

#endif // AuClientView_H
