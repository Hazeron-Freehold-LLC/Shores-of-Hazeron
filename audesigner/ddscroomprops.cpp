//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscroomprops.cpp,v 1.7 2022/02/19 18:27:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscroomprops.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscroompropsdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScRoomProperties::DdScRoomProperties(AuDesignerView* view) : DdCommand(view)
{
}

DdScRoomProperties::~DdScRoomProperties()
{

}

QString DdScRoomProperties::WikiTopic() const
{
	return "Designer_Menu#Room_Lighting";
}

void DdScRoomProperties::Start()
{
	DdCommand::Start();
	AuScRoomPropsDlg roomdlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(roomdlg) == QDialog::Accepted)
	{
		SetupRoom(roomdlg.IsSunlit(), roomdlg.InitialLighting());
		Terminate();
	}
}

void DdScRoomProperties::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Configure room lighting options. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScRoomProperties::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	UpdatePrompt();
	return loc;
}

// Set appropriate device cursor.
void DdScRoomProperties::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScRoomProperties::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

// Render "cursor" objects in the scene.
void DdScRoomProperties::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

bool DdScRoomProperties::SetupRoom(bool isSunlit, INTERIOR_LIGHT initLight)
{
	auApp->DesignerPartsSetRoomProperties(m_pWnd->DesignerPartSelection(), isSunlit, initLight);
	return true;
}
