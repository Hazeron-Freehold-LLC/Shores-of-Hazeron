//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmovedoorswitches.cpp,v 1.7 2022/02/19 18:27:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmovedoorswitches.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScMoveDoorSwitches::DdScMoveDoorSwitches(AuDesignerView* view) : DdCommand(view)
{
	m_Mode = 0;
}

DdScMoveDoorSwitches::~DdScMoveDoorSwitches()
{

}

QString DdScMoveDoorSwitches::WikiTopic() const
{
	return "Designer_Menu#Door_Switch_Positions";
}

void DdScMoveDoorSwitches::Start()
{
	DdCommand::Start();
}

void DdScMoveDoorSwitches::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">First door switch location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Center of Door, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 1)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Second door switch location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Center of Door, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScMoveDoorSwitches::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		m_Switch1 = loc;
		m_Mode = 1;
		UpdatePrompt();
	}
	else
	{
		if(MoveSwitches(loc))
			Terminate();
	}
	return ploc;
}

// Set appropriate device cursor.
void DdScMoveDoorSwitches::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScMoveDoorSwitches::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_C:
		if(m_Mode == 0)
		{
			m_Switch1 = AuPoint::Invalid();
			m_Mode = 1;
			UpdatePrompt();
			return true;
		}
		else if(m_Mode == 1)
		{
			MoveSwitches(AuPoint::Invalid());
			Terminate();
		}
		break;

	default:
		break;
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScMoveDoorSwitches::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	/*
	// Use line shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts);
	re.GLSL().LineShader().SetColor(.7f, 1, .7f);

	// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
	re.glDisable(GL_DEPTH_TEST);

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();
	*/

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScMoveDoorSwitches::MoveSwitches(const AuPoint& loc)
{
	auApp->DesignerPartsMoveDoorSwitches(m_pWnd->DesignerPartSelection(), m_Switch1, loc);
	return true;
}
