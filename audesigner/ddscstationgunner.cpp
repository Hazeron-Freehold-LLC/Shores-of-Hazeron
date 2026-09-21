//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstationgunner.cpp,v 1.7 2022/02/19 18:27:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscstationgunner.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "auscstationgunnerdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScStationGunner::DdScStationGunner(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
	m_GunnerPos = AuScTurret::Gunner_Sit;
	m_Module = CID_Invalid;
}

QString DdScStationGunner::WikiTopic() const
{
	return "Designer_Menu#Turret_Gunner";
}

void DdScStationGunner::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Corner of the screen. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 1)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Opposite corner of the screen. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 2)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gunner's location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Gunner's facing. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Screen, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());

	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScStationGunner::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		// Drop the point to the grid plane.
		m_ScreenPlane = m_pWnd->DesignPlane();
		m_Corner1 = m_ScreenPlane.UnProject(loc);
		m_Corner1.z = 0;
		m_Corner1 = m_ScreenPlane.Project(m_Corner1);

		m_GetPoint = 1;
		UpdatePrompt();
	}
	else if(m_GetPoint == 1)
	{
		// Drop the point to the grid plane.
		m_Corner2 = m_ScreenPlane.UnProject(loc);
		m_Corner2.z = 0;
		m_Corner2 = m_ScreenPlane.Project(m_Corner2);

		// Get gunner position options.
		AuScStationGunnerDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			m_GunnerPos = dlg.Position();
			m_GetPoint = 2;	// Ask for gunner's location.
			m_Module = dlg.Module();
			UpdatePrompt();
		}
	}
	else if(m_GetPoint == 2)
	{
		m_GunnerLoc = loc;
		m_GetPoint = 3;	// Ask for gunner's facing.
		UpdatePrompt();
	}
	else
	{
		if(MakeIt(loc))
			Terminate();
	}

	return loc;
}

void DdScStationGunner::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
		// Drop the point to the plane.
		AuPoint cloc = m_pWnd->DesignPlane().UnProject(m_Corner1);
		loc = m_ScreenPlane.UnProject(loc);
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
	else if(m_GetPoint >= 2)
	{
		// Drop the point to the plane.
		AuPoint cloc1 = m_ScreenPlane.UnProject(m_Corner1);
		AuPoint cloc2 = m_ScreenPlane.UnProject(m_Corner2);

		AuPoint ll = cloc1.Min(cloc2);
		AuPoint ur = cloc1.Max(cloc2);
		AuPoint ul(ll.x, ur.y, 0);
		AuPoint lr(ur.x, ll.y, 0);

		AuPointArray verts;
		verts.append(m_ScreenPlane.Project(ll));
		verts.append(m_ScreenPlane.Project(ul));
		verts.append(m_ScreenPlane.Project(ur));
		verts.append(m_ScreenPlane.Project(lr));

		re.glDisable(GL_DEPTH_TEST);
		re.glEnable(GL_LINE_SMOOTH);
		re.GLSL().LineShader().Select();
		re.GLSL().LineShader().SetProjectionModelView(pmv);
		re.GLSL().LineShader().SetColor(1,1,1,1);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		if(m_GetPoint == 3)
		{
			verts.clear();
			verts.append(m_GunnerLoc);
			verts.append(loc);

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
		re.GLSL().LineShader().Deselect();
		re.glDisable(GL_LINE_SMOOTH);
		re.glEnable(GL_DEPTH_TEST);
	}

	// Draw the blip in pixel space.
	DrawBlip(pmv, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScStationGunner::OnKeyDown(QKeyEvent* e)
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

	case Qt::Key_S:	// Face screen
		if(m_GetPoint == 3)
			if(MakeIt(m_GunnerLoc))	// Pass op loc as facing to face the screen.
				Terminate();
		break;

	default:
		break;
	}

	return false;
}

bool DdScStationGunner::MakeIt(const AuPoint& gunnerFacing)
{
	if(m_Corner1 == m_Corner2)
		return false;

	// Make operator facing direction point a consistent meter in length.
	AuPoint gunnerDir(gunnerFacing);
	if(gunnerDir != m_GunnerLoc)
	{
		gunnerDir -= m_GunnerLoc;
		gunnerDir.Unit(FeetPerMeter());
		gunnerDir += m_GunnerLoc;
	}

	auApp->DesignerTurretStation(m_pWnd->DesignerPartSelection(), (AuScTurret::GUNNER_POSITION)m_GunnerPos, m_GunnerLoc, gunnerDir,
			m_Corner1, m_Corner2, m_ScreenPlane, m_Module);
	return true;
}
