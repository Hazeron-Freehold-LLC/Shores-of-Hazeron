//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstationmedic.cpp,v 1.7 2022/02/19 18:27:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscstationmedic.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "auscstationmedicdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScStationMedic::DdScStationMedic(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
	m_MedicPos = AuScStation::Operator_Sit;
	m_Module = CID_Invalid;
	m_PatientPos = AuScStationMedic::Patient_Prone;
}

QString DdScStationMedic::WikiTopic() const
{
	return "Designer_Menu#Medic";
}

void DdScStationMedic::UpdatePrompt() const
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
				"<span style=\"color: %2;\">Medic's location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 3)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Medic's facing. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Screen, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_GetPoint == 4)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Patient's location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
	{
		if(m_PatientPos == AuScConsoleMedic::Patient_Prone)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Patient's head location. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Patient's facing. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}

	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScStationMedic::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		// Drop the point to the plane.
		m_ScreenPlane = m_pWnd->DesignPlane();
		m_Corner1 = m_ScreenPlane.UnProject(loc);
		m_Corner1.z = 0;
		m_Corner1 = m_ScreenPlane.Project(m_Corner1);

		m_GetPoint = 1;
		UpdatePrompt();
	}
	else if(m_GetPoint == 1)
	{
		// Drop the point to the plane.
		m_Corner2 = m_ScreenPlane.UnProject(loc);
		m_Corner2.z = 0;
		m_Corner2 = m_ScreenPlane.Project(m_Corner2);

		// Get medic and patient position options.
		AuScStationMedicDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			m_MedicPos = dlg.MedicPosition();
			m_Module = dlg.Module();
			m_PatientPos = dlg.PatientPosition();
			m_GetPoint = 2;
			UpdatePrompt();
		}
	}
	else if(m_GetPoint == 2)
	{
		m_MedicLoc = loc;
		m_GetPoint = 3;
		UpdatePrompt();
	}
	else if(m_GetPoint == 3)
	{
		m_MedicFacing = loc;
		m_GetPoint = 4;
		UpdatePrompt();
	}
	else if(m_GetPoint == 4)
	{
		m_PatientLoc = loc;
		m_GetPoint = 5;
		UpdatePrompt();
	}
	else
	{
		if(MakeIt(loc))
			Terminate();
	}

	return loc;
}

void DdScStationMedic::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	else if(m_GetPoint == 2 || m_GetPoint == 3 || m_GetPoint == 5)
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
			verts.append(m_MedicLoc);
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
		else if(m_GetPoint == 5)
		{
			if(m_MedicFacing != m_MedicLoc)
			{
				verts.clear();
				verts.append(m_MedicLoc);
				verts.append(m_MedicFacing);

				vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_LINE_STRIP);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}

			verts.clear();
			verts.append(m_PatientLoc);
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

bool DdScStationMedic::OnKeyDown(QKeyEvent* e)
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
		{
			m_MedicFacing = m_MedicLoc;
			m_GetPoint = 4;
			UpdatePrompt();
		}
		break;

	default:
		break;
	}

	return false;
}

bool DdScStationMedic::MakeIt(const AuPoint& patientFacing)
{
	// Make medic direction point a consistent meter in length.
	AuPoint medicDir(m_MedicFacing);
	if(medicDir != m_MedicLoc)
	{
		medicDir -= m_MedicLoc;
		medicDir.Unit(FeetPerMeter());
		medicDir += m_MedicLoc;
	}

	// Make patient direction point a consistent meter in length.
	AuPoint patientDir(patientFacing);
	if(patientDir != m_PatientLoc)
	{
		patientDir -= m_PatientLoc;
		patientDir.Unit(FeetPerMeter());
		patientDir += m_PatientLoc;
	}

	AuScStationMedic station;
	station.MakeStation((AuScStation::OPERATOR_POSITION)m_MedicPos, m_MedicLoc, medicDir, m_Corner1, m_Corner2, m_ScreenPlane);
	station.SetModule(m_Module);
	station.SetPatient((AuScStationMedic::PATIENT_POSITION)m_PatientPos, m_PatientLoc, patientDir);
	if(!station.IsValid())
		return false;

	station.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(station);
	return true;
}
