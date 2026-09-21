//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscturretgun.cpp,v 1.7 2022/02/19 18:27:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscturretgun.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScTurretGun::DdScTurretGun(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
}

QString DdScTurretGun::WikiTopic() const
{
	return "Designer_Menu#Turret_Gun_Position";
}

void DdScTurretGun::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gun pivot point. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 1)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gun pivot axis. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 2)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gun fire emit point. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 3)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gun fire aim point. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gun sight point, from where the gunner sees. Moves with gun."
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScTurretGun::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		m_GunPivot = loc;
		m_GetPoint = 1;
		UpdatePrompt();
	}
	else if(m_GetPoint == 1)
	{
		m_GunAxis = loc - m_GunPivot;
		m_GunAxis.Unit();
		m_GetPoint = 2;
		UpdatePrompt();
	}
	else if(m_GetPoint == 2)
	{
		m_GunEmitLoc = loc;
		m_GetPoint = 3;
		UpdatePrompt();
	}
	else if(m_GetPoint == 3)
	{
		m_AimLoc = loc;
		m_GetPoint = 4;
		UpdatePrompt();
	}
	else
	{
		if(DoIt(loc))
			Terminate();
	}

	return loc;
}

void DdScTurretGun::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = ts.Inverse();

	AuMatrix4x4 pmv = projection * model * ts;

	// Get cursor location in coordinates relative to the design environment.
	AuPoint loc = m_pWnd->FromPixelToDesignPlane(sceneAtom, pxCursor, true, 0);

	if(m_GetPoint == 1)
	{
		AuPointArray verts;
		verts.append(m_GunPivot);
		verts.append(loc);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,0,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();

			// Restore rendering states.
			re.GLSL().LineShader().Deselect();
			re.glDisable(GL_LINE_SMOOTH);
			re.glEnable(GL_DEPTH_TEST);
		}
	}
	else if(m_GetPoint == 2)
	{
		AuPointArray verts;
		verts.append(m_GunPivot);
		verts.append(m_GunPivot + m_GunAxis * g_AuClient.DesignerUnitScale());

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,0,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();

			// Restore rendering states.
			re.GLSL().LineShader().Deselect();
			re.glDisable(GL_LINE_SMOOTH);
			re.glEnable(GL_DEPTH_TEST);
		}
	}
	else if(m_GetPoint == 3)
	{
		AuPointArray verts;
		verts.append(m_GunPivot);
		verts.append(m_GunPivot + m_GunAxis * g_AuClient.DesignerUnitScale());
		verts.append(m_GunEmitLoc);
		verts.append(loc);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,0,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();

			// Restore rendering states.
			re.GLSL().LineShader().Deselect();
			re.glDisable(GL_LINE_SMOOTH);
			re.glEnable(GL_DEPTH_TEST);
		}
	}
	else if(m_GetPoint == 4)
	{
		AuPointArray verts;
		verts.append(m_GunPivot);
		verts.append(m_GunPivot + m_GunAxis * g_AuClient.DesignerUnitScale());
		verts.append(m_GunEmitLoc);
		AuPoint aimVec(m_AimLoc - m_GunEmitLoc);
		aimVec.Unit(g_AuClient.DesignerUnitScale());
		verts.append(m_GunEmitLoc + aimVec);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,0,1);

			vao.SelectGL();
			vao.DrawGL(GL_LINES);
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

bool DdScTurretGun::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

bool DdScTurretGun::DoIt(const AuPoint& sightLoc)
{
	if(m_AimLoc == m_GunEmitLoc)
		return false;

	AuPoint emitVec(m_AimLoc - m_GunEmitLoc);
	emitVec.Unit();

	auApp->DesignerTurretGun(m_pWnd->DesignerPartSelection(), m_GunPivot, m_GunAxis, m_GunEmitLoc, emitVec,
		sightLoc);
	return true;
}
