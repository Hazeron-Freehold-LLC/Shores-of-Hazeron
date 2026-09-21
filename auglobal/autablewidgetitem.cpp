//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autablewidgetitem.cpp,v 1.2 2022/02/19 18:33:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autablewidgetitem.h"
//
#include "audef.h"

AuTableWidgetDistanceItem::AuTableWidgetDistanceItem(double feet)
{
	if(feet)
	{
		QString dist = QString("%L1m").arg(feet/FeetPerMeter(), 0, 'f', 0);
		setText(dist);
	}
}

