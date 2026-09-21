//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscberthname.cpp,v 1.5 2022/02/19 18:26:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscberthname.h"
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

DdScBerthName::DdScBerthName(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
}

void DdScBerthName::Start()
{
	DdCommand::Start();
}

QString DdScBerthName::WikiTopic() const
{
	return "Designer_Menu#Berth_Assignee.27s_Name";
}

void DdScBerthName::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Corner of the berth name decal. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Opposite corner of the berth name decal. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScBerthName::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		// Slam the point to the plane.
		m_Corner = m_pWnd->DesignPlane().UnProject(loc);
		m_Corner.z = 0;
		m_Corner = m_pWnd->DesignPlane().Project(m_Corner);

		m_GetPoint = 1;
		UpdatePrompt();
		return m_Corner;
	}
	else
	{
		AuPoint cloc = m_pWnd->DesignPlane().UnProject(loc);
		cloc.z = 0;
		cloc = m_pWnd->DesignPlane().Project(cloc);
		if(MakeIt(cloc))
			Terminate();

		return cloc;
	}
}

void DdScBerthName::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(m_GetPoint == 1)
	{
		// Slam the point to the plane.
		AuPoint cloc = m_pWnd->DesignPlane().UnProject(m_Corner);
		loc = m_pWnd->DesignPlane().UnProject(loc);
		loc.z = 0;

		AuPoint ll = loc.Min(cloc);
		AuPoint ur = loc.Max(cloc);
		AuPoint ul(ll.x, ur.y, 0);
		AuPoint lr(ur.x, ll.y, 0);

		AuPointArray verts;
		verts.append(m_pWnd->DesignPlane().Project(ll));
		verts.append(m_pWnd->DesignPlane().Project(ul));
		verts.append(m_pWnd->DesignPlane().Project(ur));
		verts.append(m_pWnd->DesignPlane().Project(lr));

		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,1,1);

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

bool DdScBerthName::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_C:	// Restart
		m_GetPoint = 0;
		UpdatePrompt();
		break;

	default:
		break;
	}

	return false;
}

bool DdScBerthName::MakeIt(const AuPoint& corner)
{
	AuScBerthName nameDecal;

	nameDecal.MakeScreen(m_Corner, corner, m_pWnd->DesignPlane());
	nameDecal.FlattenVertices();
	if(!nameDecal.IsValid())
		return false;

	nameDecal.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(nameDecal);
	return true;
}
