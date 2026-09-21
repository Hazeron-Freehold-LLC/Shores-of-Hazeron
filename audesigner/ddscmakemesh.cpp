//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakemesh.cpp,v 1.6 2022/02/19 18:27:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakemesh.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeMesh::DdScMakeMesh(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeMesh::WikiTopic() const
{
	return "Designer_Menu#Mesh_2";
}

void DdScMakeMesh::Start()
{
	DdCommand::Start();

	auApp->DesignerMakeMesh(m_pWnd->DesignerPartSelection());

	// We only take one shot at this.
	Terminate();
}
