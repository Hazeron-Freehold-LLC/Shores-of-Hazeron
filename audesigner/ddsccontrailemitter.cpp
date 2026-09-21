//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsccontrailemitter.cpp,v 1.7 2022/02/19 18:26:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsccontrailemitter.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScContrailEmitter::DdScContrailEmitter(AuDesignerView* view) : DdCommand(view)
{
}

QString DdScContrailEmitter::WikiTopic() const
{
	return "Designer_Menu#Contrail_Emitter";
}

void DdScContrailEmitter::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Contrail emitter location. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScContrailEmitter::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	MakeIt(loc);

	return loc;
}

void DdScContrailEmitter::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

bool DdScContrailEmitter::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

bool DdScContrailEmitter::MakeIt(const AuPoint& loc)
{
	AuScContrailEmitter pod;
	pod.MakeIcon(loc, 0, m_pWnd->DesignPlane());
	if(!pod.IsValid())
		return false;

	pod.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(pod);
	return true;
}
