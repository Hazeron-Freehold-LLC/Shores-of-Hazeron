//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscconsole.cpp,v 1.7 2022/02/19 18:26:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscconsole.h"
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

DdScConsole::DdScConsole(quint8 partType, AuDesignerView* view) : DdCommand(view)
{
	m_PartType = partType;
	m_Rot = 0;
}

void DdScConsole::Start()
{
	DdCommand::Start();
}

QString DdScConsole::WikiTopic() const
{
	return "Designer_Menu#Swivel_Consoles_w.2FChairs";
}

void DdScConsole::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Base swivel point of console. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">[Shift] Left/Right Arrow%1Rotate, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScConsole::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	// Some consoles correspond to a system that has a module.
	SYSTEM system;
	switch(m_PartType)
	{
	default:
	case AuScPart::Part_ConsoleCommand:
	case AuScPart::Part_ConsoleHelm:
	case AuScPart::Part_ConsoleNavigator:
	case AuScPart::Part_ConsolePowerRelay:
		system = SYS_Count;
		break;
	case AuScPart::Part_ConsoleFireControl:
	case AuScPart::Part_ConsoleTransporter:
		// Handled by a different command.
		Q_ASSERT(false);
		system = SYS_Count;
		break;
	case AuScPart::Part_ConsoleEngineer:
		system = SYS_FTLDrive;
		break;
	case AuScPart::Part_ConsoleMedic:
		system = SYS_SurgeryUnit;
		break;
	case AuScPart::Part_ConsoleSensor:
		system = SYS_Sensor;
		break;
	case AuScPart::Part_ConsoleShield:
		system = SYS_Shield;
		break;
	}

	if(system != SYS_Count)
	{
		AuScConsoleModuleDlg dlg(system, m_pWnd);
		if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			MakeIt(loc, dlg.Module());
	}
	else
		MakeIt(loc, CID_Invalid);

	Terminate();

	return loc;
}

void DdScConsole::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	{
		float rot = m_Rot * DEG2RADf;
		while(rot < -PIf)
			rot += PIf*2;
		while(rot > PIf)
			rot -= PIf*2;

		AuMatrix4x4 trot(pmv);
		trot.Translate(loc);
		trot.RotateZ(rot);

		AuScConsoleHelm con;
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

bool DdScConsole::OnKeyDown(QKeyEvent* e)
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

bool DdScConsole::MakeIt(const AuPoint& loc, COMMODITYID module)
{
	AuScPart* part = AuScPart::Create((AuScPart::PART_TYPE)m_PartType);
	if(!part)
		return false;

	AuScConsole* con = dynamic_cast<AuScConsole*>(part);
	if(!con)
	{
		delete part;
		return false;
	}

	con->MakeIcon(loc, m_Rot*DEG2RADf, m_pWnd->DesignPlane());
	con->SetModule(module);
	if(!con->IsValid())
	{
		delete con;
		return false;
	}

	con->SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(*con);
	delete con;
	return true;
}
