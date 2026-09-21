//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddgrid3pt.cpp,v 1.8 2022/02/19 18:26:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddgrid3pt.h"
//
#include "auapp.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdGrid3pt::DdGrid3pt(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
}

QString DdGrid3pt::WikiTopic() const
{
	return "Designer_Menu#Position_Grid";
}

void DdGrid3pt::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate grid center. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Reset, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">F%1Flat, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">X,Y,Z%1Flip, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 1)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Align grid X axis. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Reset, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Align grid plane. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Reset, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

bool DdGrid3pt::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_B:	// Back
		if(m_GetPoint)
		{
			m_GetPoint--;
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		if(m_GetPoint)
		{
			m_GetPoint = 0;
			UpdatePrompt();							// Reset prompt.
			return true;
		}
		break;

	case Qt::Key_S:	// Reset
		m_pWnd->SetDesignPlane(AuMatrix4x4());
		Terminate();
		return true;

	case Qt::Key_F:	// Flat
		if(!m_GetPoint)
		{
			AuMatrix4x4 t;
			t.Translate(DesignPlane().Translation());
			m_pWnd->SetDesignPlane(t);
			Terminate();
			return true;
		}
		break;

	case Qt::Key_X:	// X flip
		if(!m_GetPoint)
		{
			AuMatrix4x4 t(DesignPlane());;
			t.RotateX(PI);
			m_pWnd->SetDesignPlane(t);
			Terminate();
			return true;
		}
		break;

	case Qt::Key_Y:	// Y flip
		if(!m_GetPoint)
		{
			AuMatrix4x4 t(DesignPlane());;
			t.RotateY(PI);
			m_pWnd->SetDesignPlane(t);
			Terminate();
			return true;
		}
		break;

	case Qt::Key_Z:	// Z flip
		if(!m_GetPoint)
		{
			AuMatrix4x4 t(DesignPlane());;
			t.RotateZ(PI);
			m_pWnd->SetDesignPlane(t);
			Terminate();
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

AuPoint DdGrid3pt::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint retloc(m_Origin);
	if(m_GetPoint == 0)
	{
		m_Origin = loc;
		retloc = m_Origin;
		m_GetPoint = 1;
		UpdatePrompt();
	}
	else if(m_GetPoint == 1)
	{
		if(loc != m_Origin)
		{
			m_XAxis = loc;
			m_GetPoint = 2;
			UpdatePrompt();
		}
	}
	else
	{
		if(loc != m_Origin && loc != m_XAxis)
		{
			// Position the design plane using the locations entered.
			AuPoint xvec = m_XAxis - m_Origin;
			xvec.Unit();
			AuPoint yvec = loc - m_Origin;
			yvec.Unit();
			AuMatrix4x4 newPlane;
			newPlane.Translate(m_Origin);
			newPlane.RotateToPlane(xvec, yvec);
			m_pWnd->SetDesignPlane(newPlane);
			Terminate();
		}
	}
	return retloc;
}

void DdGrid3pt::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	// Line shader is nice and bright, no textures or shading.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model * ts);
	re.glDisable(GL_DEPTH_TEST);

	if(m_GetPoint == 1)
	{
		// Draw X axis line.
		re.glEnable(GL_LINE_SMOOTH);
		re.GLSL().LineShader().SetColor(1,0,0,1);

		AuPointArray verts;
		verts.append(m_Origin);
		verts.append(loc);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		re.glDisable(GL_LINE_SMOOTH);
	}
	else if(m_GetPoint == 2)
	{
		// Draw X axis line.
		re.glEnable(GL_LINE_SMOOTH);
		re.GLSL().LineShader().SetColor(1,0,0,1);

		AuPointArray verts;
		verts.append(m_Origin);
		verts.append(m_XAxis);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		// Draw Y axis line.
		re.GLSL().LineShader().SetColor(0,1,0,1);

		AuPoint xv = m_XAxis - m_Origin;
		double xl = xv.Length();
		if(xl != 0)
			xv /= xl;
		AuPoint yv = loc - m_Origin;
		yv.Unit();
		AuPoint zv = xv.Cross(yv);
		zv.Unit();
		yv = zv.Cross(xv);
		yv.Unit();

		AuPoint yloc = m_Origin + yv * xl;

		verts.clear();
		verts.append(m_Origin);
		verts.append(yloc);

		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		re.glDisable(GL_LINE_SMOOTH);

		// Draw cursor line.
		re.GLSL().LineShader().SetColor(.5,.5,.5,1);

		double xoff = (loc - m_Origin).Dot(xv);
		double yoff = (loc - m_Origin).Dot(yv);

		verts.clear();
		verts.append(m_Origin);
		verts.append(m_Origin + yv * yoff);
		verts.append(loc);
		verts.append(m_Origin + xv * xoff);
		verts.append(m_Origin);

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
	re.glEnable(GL_DEPTH_TEST);
	re.GLSL().LineShader().Deselect();

	// Draw the blip in pixel space.
	AuMatrix4x4 mvp = projection * model * ts;
	DrawBlip(mvp, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}
