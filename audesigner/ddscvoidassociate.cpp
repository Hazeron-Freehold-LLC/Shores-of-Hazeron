//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscvoidassociate.cpp,v 1.6 2022/02/19 18:27:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscvoidassociate.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auscdividedlg.h"
#include "audesignerview.h"

DdScVoidAssociate::DdScVoidAssociate(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScVoidAssociate::WikiTopic() const
{
	return "Designer_Menu#Part_Association";
}

void DdScVoidAssociate::Start()
{
	DdCommand::Start();

	auApp->DesignerAssociateParts(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
