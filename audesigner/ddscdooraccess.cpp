//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscdooraccess.cpp,v 1.9 2022/02/19 18:26:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscdooraccess.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdooraccessdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScDoorAccess::DdScDoorAccess(AuDesignerView* view) : DdCommand(view)
{
}

DdScDoorAccess::~DdScDoorAccess()
{

}

QString DdScDoorAccess::WikiTopic() const
{
	return "Designer_Menu#Door";
}

void DdScDoorAccess::Start()
{
	DdCommand::Start();

	// Configure defaults.
	Qt::CheckState isAutomatic = Qt::Checked;
	Qt::CheckState initLocked = Qt::Unchecked;
	Qt::CheckState remoteSpacecraft = Qt::Unchecked;
	Qt::CheckState remoteVehicle = Qt::Unchecked;

	// Get current properties from selection.
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	if(person)
	{
		const DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartSelectionDoorProperties(m_pWnd->DesignerPartSelection(), isAutomatic, initLocked, remoteSpacecraft,
					remoteVehicle);
	}
	m_pWnd->UnlockSceneAtom();

	AuScDoorAccessDlg dlg(isAutomatic, initLocked, remoteSpacecraft, remoteVehicle, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		if(SetupDoorAccess(dlg.IsAutomatic(), dlg.InitLocked(), dlg.RemoteSpacecraft(), dlg.RemoteVehicle()))
			Terminate();
	}
	else
		Terminate();
}

void DdScDoorAccess::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Configure door access options. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScDoorAccess::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	return loc;
}

// Set appropriate device cursor.
void DdScDoorAccess::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScDoorAccess::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;
	return false;
}

// Render "cursor" objects in the scene.
void DdScDoorAccess::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	ts = model * ts.Inverse();

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScDoorAccess::SetupDoorAccess(Qt::CheckState isAutomatic, Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft,
		Qt::CheckState remoteVehicle)
{
	auApp->DesignerPartsSetDoorProperties(m_pWnd->DesignerPartSelection(), isAutomatic, initLocked, remoteSpacecraft,
			remoteVehicle);
	return true;
}
