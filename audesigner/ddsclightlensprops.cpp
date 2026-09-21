//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsclightlensprops.cpp,v 1.7 2022/02/19 18:27:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsclightlensprops.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausclightlensprop.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScLightLensProps::DdScLightLensProps(AuDesignerView* view) : DdCommand(view)
{
}

DdScLightLensProps::~DdScLightLensProps()
{

}

QString DdScLightLensProps::WikiTopic() const
{
	return "Designer_Menu#Light_Lens";
}

void DdScLightLensProps::Start()
{
	DdCommand::Start();

	// Configure defaults.
	QColor color;
	if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Spacecraft)
		color = AuGLLight::ColorCarbonArc();
	else if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Building)
		color = AuGLLight::ColorSodiumVapor();
	else
		color = AuGLLight::ColorHalogen();

	AuScSelection sel = m_pWnd->DesignerPartSelection();

	// Get current properties from selection.
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	if(person)
	{
		const DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartSelectionLightLensProperties(sel, color);
	}
	m_pWnd->UnlockSceneAtom();

	AuScLightLensProp dlg(color, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		auApp->DesignerPartsSetLightLensProperties(m_pWnd->DesignerPartSelection(), dlg.LightColor());

	// We only take one shot at this.
	Terminate();
}

void DdScLightLensProps::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Configure light bulb properties. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

bool DdScLightLensProps::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;
	return false;
}
