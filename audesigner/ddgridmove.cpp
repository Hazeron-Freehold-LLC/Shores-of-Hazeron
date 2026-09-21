//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddgridmove.cpp,v 1.7 2022/02/19 18:26:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddgridmove.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdGridMove::DdGridMove(AuDesignerView* view) : DdCommand(view)
{
}

QString DdGridMove::WikiTopic() const
{
	return "Designer_Menu#Move_Grid";
}

void DdGridMove::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Locate grid center. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">S%1Reset, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdGridMove::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	if(!g_AuClient.DesignerGridFollowsWork())
	{
		// Move the design plane to the location that was entered.
		// When DesignerGridFollowsWork is enabled, this will be done by the view when I return.
		AuMatrix4x4 dPlane = m_pWnd->DesignPlane();
		AuMatrix4x4 newPlane;
		newPlane.Translate(loc);
		newPlane.Rotate(dPlane.GetRotation());
		m_pWnd->SetDesignPlane(newPlane);
	}

	Terminate();
	return loc;
}

void DdGridMove::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Get cursor location in coordinates relative to the design environment.
	AuPoint loc = m_pWnd->FromPixelToDesignPlane(sceneAtom, pxCursor, true, 0);

	double axislen = 10 * g_AuClient.DesignerUnitScale();	// g_AuClient.DesignPlaneRotatorAxisLengthFt();

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = ts.Inverse();
	AuPoint locO = ts.Project(loc);
	AuPoint locX = ts.Project(loc + m_pWnd->DesignPlane().ProjectVector(AuPoint(axislen, 0, 0)));
	AuPoint locY = ts.Project(loc + m_pWnd->DesignPlane().ProjectVector(AuPoint(0, axislen, 0)));
	AuPoint locZ = ts.Project(loc + m_pWnd->DesignPlane().ProjectVector(AuPoint(0, 0, axislen)));

	// Line shader is nice and bright, no textures or shading.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);
	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_LINE_SMOOTH);

	AuGLVAO vao;
	AuPointArray verts;

	// X axis
	{
		re.GLSL().LineShader().SetColor(1,0,0,1);
		verts.append(locO);
		verts.append(locX);
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Y axis
	{
		re.GLSL().LineShader().SetColor(0,1,0,1);
		verts.clear();
		verts.append(locO);
		verts.append(locY);
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Z axis
	{
		re.GLSL().LineShader().SetColor(0,0,1,1);
		verts.clear();
		verts.append(locO);
		verts.append(locZ);
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Restore rendering states.
	re.glDisable(GL_LINE_SMOOTH);
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();

	// Draw the blip in pixel space.
	AuMatrix4x4 mvp = projection * model * ts;
	DrawBlip(mvp, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdGridMove::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_S:	// Reset
		{
			// Restore plane to origin without changing its rotation.
			AuMatrix4x4 dPlane = m_pWnd->DesignPlane();
			AuMatrix4x4 newPlane;
			newPlane.Rotate(dPlane.GetRotation());
			m_pWnd->SetDesignPlane(newPlane);
			Terminate();
		}
		break;

	default:
		break;
	}

	return false;
}
