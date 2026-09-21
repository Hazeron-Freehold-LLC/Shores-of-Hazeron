//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakedecal.cpp,v 1.6 2022/02/19 18:27:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakedecal.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeDecal::DdScMakeDecal(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeDecal::WikiTopic() const
{
	return "Designer_Menu#Decal_2";
}

void DdScMakeDecal::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeDecal(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
