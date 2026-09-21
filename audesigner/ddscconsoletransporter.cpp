//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscconsoletransporter.cpp,v 1.7 2022/02/19 18:26:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscconsoletransporter.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscconsolemoduledlg.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScConsoleTransporter::DdScConsoleTransporter(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
	m_Module = CID_Invalid;
	m_Rot = 0;
	m_WhichEffect = 0;
}

QString DdScConsoleTransporter::WikiTopic() const
{
	return "Designer_Menu#Transporter_2";
}

void DdScConsoleTransporter::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Base swivel point of transporter console. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">[Shift] Left/Right Arrow%1Rotate, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_PadLocs.empty())
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">First personnel transporter pad location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Personnel transporter pad location %3. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name())
			.arg(m_PadLocs.size()+1);
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScConsoleTransporter::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		m_ConsoleLoc = loc;
		m_ConsolePlane = DesignPlane();
		m_GetPoint = 1;
		UpdatePrompt();
	}
	else
	{
		m_PadLocs.append(loc);
		UpdatePrompt();
	}

	return loc;
}

void DdScConsoleTransporter::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

		AuScConsoleTransporter con;
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

		if(!m_PadLocs.empty())
		{
			AuPointArray verts;
			verts.MakeCircle(AuPoint(0,0), AuPoint(.5,0), 6);
			AuGLVAO vao;
			vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				re.glDisable(GL_DEPTH_TEST);
				re.glEnable(GL_LINE_SMOOTH);
				re.GLSL().LineShader().Select();
				re.GLSL().LineShader().SetColor(1,1,1,1);

				vao.SelectGL();
				for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
				{
					AuMatrix4x4 tpad(pmv);
					tpad.Translate(*pi);
					tpad.RotateZ(rot);
					re.GLSL().LineShader().SetProjectionModelView(tpad);
					vao.DrawGL(GL_LINE_STRIP);
				}
				vao.DeselectGL();
				vao.DeleteGL();

				// Restore rendering states.
				re.GLSL().LineShader().Deselect();
				re.glDisable(GL_LINE_SMOOTH);
				re.glEnable(GL_DEPTH_TEST);
			}
		}
	}

	// Draw the blip in pixel space.
	DrawBlip(pmv, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScConsoleTransporter::OnKeyDown(QKeyEvent* e)
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

	case Qt::Key_B:	// Back
		if(!m_PadLocs.empty())
		{
			m_PadLocs.pop_back();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		m_GetPoint = 0;
		m_PadLocs.clear();
		UpdatePrompt();
		return true;

	case Qt::Key_D:	// Done
		if(!m_PadLocs.empty())
		{
			AuScConsoleModuleDlg dlg(SYS_Transporter, m_pWnd);
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				m_Module = dlg.Module();
				m_WhichEffect = dlg.WhichEffect();
				MakeIt();
			}
			Terminate();
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

bool DdScConsoleTransporter::MakeIt()
{
	if(m_PadLocs.empty())
		return false;

	AuScConsoleTransporter con;
	con.MakeIcon(m_ConsoleLoc, m_Rot*DEG2RADf, m_ConsolePlane);
	con.SetModule(m_Module);
	con.SetWhichEffect(m_WhichEffect);
	con.SetPadLocs(m_PadLocs);
	if(!con.IsValid())
		return false;

	con.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(con);
	return true;
}
