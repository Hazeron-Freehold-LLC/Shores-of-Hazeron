//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakehull.cpp,v 1.6 2022/02/19 18:27:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakehull.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeHull::DdScMakeHull(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeHull::WikiTopic() const
{
	return "Designer_Menu#Hull_2";
}

void DdScMakeHull::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeHull(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
