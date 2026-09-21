//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscalignliftstop.cpp,v 1.7 2022/02/19 18:26:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscalignliftstop.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auscdividedlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"

DdScAlignLiftStop::DdScAlignLiftStop(AuDesignerView* view) : DdCommand(view)
{
	m_Mode = 0;
}

QString DdScAlignLiftStop::WikiTopic() const
{
	return "Designer_Menu#Align_Turbo_Lift_Stop";
}

void DdScAlignLiftStop::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Orient grid at start location. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else
	{
		prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Orient grid at destination. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}

	m_pWnd->CommandPromptChanged(prompt);
}

bool DdScAlignLiftStop::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	if(m_Mode == 0)
	{
		switch(e->key())
		{
		case Qt::Key_D:	// Done
			// Grab a copy of the design plane at its current location.
			m_StartM = DesignPlane();
			m_Mode = 1;
			UpdatePrompt();
			return true;

		default:
			break;
		}
	}
	else if(m_Mode == 1)
	{
		switch(e->key())
		{
		case Qt::Key_C:	// Restart
			m_Mode = 0;
			UpdatePrompt();							// Reset prompt.
			return true;

		case Qt::Key_D:	// Done
			DoMove();
			Terminate();
			return true;

		default:
			break;
		}
	}

	return false;
}

void DdScAlignLiftStop::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	Q_UNUSED(pxCursor);
	Q_UNUSED(glw);
	Q_UNUSED(snappedToExistingShape);
	if(m_Mode == 0)
		return;

	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = model * ts.Inverse();

	// Use line shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts);
	re.GLSL().LineShader().SetColor(.7f, 1, .7f);

	// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
	re.glDisable(GL_DEPTH_TEST);

	// Make a VAO of the vertex locations.
	AuPoint o = m_StartM.Translation();
	AuPoint x = o + m_StartM.ProjectVector(FeetPerMeter(),0,0);
	AuPoint y = o + m_StartM.ProjectVector(0,FeetPerMeter(),0);
	AuPoint z = o + m_StartM.ProjectVector(0,0,FeetPerMeter());

	// Draw origin plane lines.
	AuGLVAO vao;
	AuPointArray cseg;
	cseg.append(o);
	cseg.append(x);
	cseg.append(o);
	cseg.append(y);
	cseg.append(o);
	cseg.append(z);
	vao.CreateForLines(re, cseg, false, GL_STREAM_DRAW);
	if(vao.VAO())
	{
		// Display the VAO once and clean it up.
		vao.SelectGL();
		vao.DrawGL(GL_LINES);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();
}

void DdScAlignLiftStop::DoMove()
{
	if(m_StartM == DesignPlane())
		return;	// Design plane didn't move?

	AuMatrix4x4 t = DesignPlane() * m_StartM.Inverse();

	auApp->DesignerMoveLiftStops(m_pWnd->DesignerPartSelection(), t);
}
