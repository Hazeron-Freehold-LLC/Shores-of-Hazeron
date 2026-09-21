//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmakelightlens.cpp,v 1.7 2022/02/19 18:27:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmakelightlens.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMakeLightLens::DdScMakeLightLens(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScMakeLightLens::WikiTopic() const
{
	return "Designer_Menu#Light_Lens_2";
}

void DdScMakeLightLens::Start()
{
	DdCommand::Start();

	QColor defaultColor;
	if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Spacecraft)
		defaultColor = AuGLLight::ColorCarbonArc();
	else if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Building)
		defaultColor = AuGLLight::ColorSodiumVapor();
	else
		defaultColor = AuGLLight::ColorHalogen();

	auApp->DesignerMakeLightLens(m_pWnd->DesignerPartSelection(), defaultColor);

	// We only take one shot at this.
	Terminate();
}
