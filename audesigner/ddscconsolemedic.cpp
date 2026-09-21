//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscconsolemedic.cpp,v 1.7 2022/02/19 18:26:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscconsolemedic.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auscconsolemedicdlg.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScConsoleMedic::DdScConsoleMedic(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
	m_Module = CID_Invalid;
	m_PatientPos = AuScConsoleMedic::Patient_Stand;
	m_Rot = 0;
}

QString DdScConsoleMedic::WikiTopic() const
{
	return "Designer_Menu#Designer_Menu#Medic_2";
}

void DdScConsoleMedic::UpdatePrompt() const
{
	if(m_GetPoint == 0)
	{
		QString prompt;
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Base swivel point of medic console. </span>"
				"<span style=\"color: rgb(255, 255, 0);\">[Shift] Left/Right Arrow%1Rotate, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		m_pWnd->CommandPromptChanged(prompt);
	}
	else if(m_GetPoint == 1)
	{
		QString prompt;
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Patient's location. </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		m_pWnd->CommandPromptChanged(prompt);
	}
	else
	{
		QString prompt;
		if(m_PatientPos == AuScConsoleMedic::Patient_Prone)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Patient's head location. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Patient's facing location. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		m_pWnd->CommandPromptChanged(prompt);
	}
}

AuPoint DdScConsoleMedic::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		AuScConsoleMedicDlg dlg(m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		{
			m_ConsoleLoc = loc;
			m_ConsolePlane = DesignPlane();
			m_GetPoint = 1;
			m_PatientPos = dlg.PatientPosition();
			m_Module = dlg.Module();
			UpdatePrompt();
		}
	}
	else if(m_GetPoint == 1)
	{
		m_PatientLoc = loc;
		m_GetPoint = 2;
		UpdatePrompt();
	}
	else
	{
		if(MakeIt(loc))
			Terminate();
	}

	return loc;
}

void DdScConsoleMedic::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	AuPoint cloc;
	if(m_GetPoint == 0)
		cloc = loc;
	else
		cloc = m_ConsoleLoc;

	{
		float rot = m_Rot * DEG2RADf;
		while(rot < -PIf)
			rot += PIf*2;
		while(rot > PIf)
			rot -= PIf*2;

		AuMatrix4x4 trot(pmv);
		trot.Translate(cloc);
		trot.RotateZ(rot);

		AuScConsoleMedic con;
		AuPointArray verts;
		verts.append(AuPoint(-con.IconLeft(), -con.IconBack()));
		verts.append(AuPoint(-con.IconLeft(), con.IconFore()));
		verts.append(AuPoint(con.IconRight(), con.IconFore()));
		verts.append(AuPoint(con.IconRight(), -con.IconBack()));

		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(trot);
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

bool DdScConsoleMedic::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_Right:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot += -1;
		else
			m_Rot += -10;
		break;

	case Qt::Key_Left:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot += 1;
		else
			m_Rot += 10;
		break;

	default:
		break;
	}

	return false;
}

bool DdScConsoleMedic::MakeIt(const AuPoint& patientFacing)
{
	// Make patient direction point a consistent meter in length.
	AuPoint patientDir(patientFacing - m_PatientLoc);
	patientDir.Unit(FeetPerMeter());
	patientDir += m_PatientLoc;

	AuScConsoleMedic con;
	con.MakeIcon(m_ConsoleLoc, m_Rot*DEG2RADf, m_ConsolePlane);
	con.SetModule(m_Module);
	con.SetPatient((AuScPart::PATIENT_POSITION)m_PatientPos, m_PatientLoc, patientDir);
	if(!con.IsValid())
		return false;

	con.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(con);
	return true;
}
