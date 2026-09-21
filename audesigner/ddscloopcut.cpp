//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscloopcut.cpp,v 1.7 2022/02/19 18:27:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscloopcut.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScLoopCut::DdScLoopCut(AuDesignerView* view) : DdCommand(view)
{
}

void DdScLoopCut::Start()
{
	DdCommand::Start();
}

QString DdScLoopCut::WikiTopic() const
{
	return "Designer_Menu#Loop_Cut";
}

void DdScLoopCut::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Loop cut edge location. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScLoopCut::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	DoIt(loc);

	return loc;
}

void DdScLoopCut::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	// Find the loop.
	AuPointArray loop;
	pde->Loop(m_pWnd->DesignerPartObjectSelection(), loc, loop);

	// Draw the loop.
	if(!loop.empty())
	{
		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, loop, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,0,1,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();

			// Restore rendering states.
			re.GLSL().LineShader().Deselect();
			re.glDisable(GL_LINE_SMOOTH);
			re.glEnable(GL_DEPTH_TEST);
		}
	}

	// Draw the blip in pixel space.
	DrawBlip(pmv, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScLoopCut::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

bool DdScLoopCut::DoIt(const AuPoint& loc)
{
	AuPointArray loop;
	{
		DaPerson* sceneAtom;
		m_pWnd->LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				pde->Loop(m_pWnd->DesignerPartObjectSelection(), loc, loop);
		}
		m_pWnd->UnlockSceneAtom();
	}
	if(loop.size() < 3)
		return false;

	if(loop.last() != loop.first())
		loop.append(loop.first());

	// Make the cut.
	auApp->DesignerCutFaces(m_pWnd->DesignerPartSelection(), loop);

	return true;
}
