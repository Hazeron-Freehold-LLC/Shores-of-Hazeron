//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdshrub.cpp,v 1.7 2022/02/19 18:26:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddbdshrub.h"
//
#include "aubdshrubdlg.h"
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdBdShrub::DdBdShrub(AuDesignerView* view) : DdCommand(view)
{
	m_PlantCID = CID_Invalid;
}

void DdBdShrub::Start()
{
	DdCommand::Start();

	AuBdShrubDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		m_PlantCID = dlg.PlantCID();
	else
		Terminate();
}

QString DdBdShrub::WikiTopic() const
{
	return "Designer_Menu#Shrub";
}

void DdBdShrub::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Shrub location. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdBdShrub::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	MakeIt(loc);

	return loc;
}

void DdBdShrub::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Get cursor location in coordinates relative to the design environment.
	AuPoint loc = m_pWnd->FromPixelToDesignPlane(sceneAtom, pxCursor, true, 0);

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = ts.Inverse();

	AuMatrix4x4 pmv = projection * model * ts;

	// Draw the blip in pixel space.
	DrawBlip(pmv, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdBdShrub::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

bool DdBdShrub::MakeIt(const AuPoint& loc)
{
	AuScShrub plant;
	plant.MakeIcon(loc, 0, m_pWnd->DesignPlane());
	plant.SetPlant(m_PlantCID);
	if(!plant.IsValid())
		return false;

	plant.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(plant);
	return true;
}
