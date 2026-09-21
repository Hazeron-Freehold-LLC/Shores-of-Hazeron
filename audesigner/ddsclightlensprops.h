//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsclightlensprops.h,v 1.6 2022/02/19 18:23:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DdScLightLensProps_H
#define DdScLightLensProps_H

#include "audef.h"
#include "ddcommand.h"
#include <QCursor>
#include <QMouseEvent>

class DdScLightLensProps : public DdCommand  
{
public:
	DdScLightLensProps(AuDesignerView* view);
	virtual ~DdScLightLensProps();

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	virtual bool OnKeyDown(QKeyEvent* e);

	virtual void Start();

	virtual QString WikiTopic() const;

protected:
	bool SetupVoid(GRAVITY gravityType, const AuPoint& gravityN, HOLD_ACCESS holdAccess, quint8 personnelAccess);

	virtual void UpdatePrompt() const;
};

#endif // DdScLightLensProps_H
