//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscvoidprops.cpp,v 1.8 2022/02/19 18:27:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscvoidprops.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscvoidpropsdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScVoidProperties::DdScVoidProperties(AuDesignerView* view) : DdCommand(view)
{
	m_GravityType = G_Natural;
	m_HoldAccess = HOLD_Inaccessible;
	m_Mode = 0;
	m_PersonnelAccess = PERSONNEL_Everyone;
}

DdScVoidProperties::~DdScVoidProperties()
{

}

QString DdScVoidProperties::WikiTopic() const
{
	return "Designer_Menu#Room_Void_Gravity_and_Hold_Access";
}

void DdScVoidProperties::Start()
{
	DdCommand::Start();
	AuScVoidPropsDlg roomdlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(roomdlg) == QDialog::Accepted)
	{
		switch(roomdlg.Mode())
		{
		case AuScVoidPropsDlg::UpZGrid:
			if(SetupVoid(roomdlg.GravityType(), DesignPlane().ProjectVector(AuPoint(0,0,1)), roomdlg.HoldAccess(),
						roomdlg.PersonnelAccess()))
				Terminate();
			break;
		case AuScVoidPropsDlg::UpZ:
			if(SetupVoid(roomdlg.GravityType(), AuPoint(0,0,1), roomdlg.HoldAccess(), roomdlg.PersonnelAccess()))
				Terminate();
			break;
		case AuScVoidPropsDlg::PickVector:
			m_GravityType = roomdlg.GravityType();
			m_HoldAccess = roomdlg.HoldAccess();
			m_Mode = 1;
			m_PersonnelAccess = roomdlg.PersonnelAccess();
			UpdatePrompt();
			break;
		}
	}
}

void DdScVoidProperties::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Configure gravity options. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 1)
	{
		// Gravity up start
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate bottom end of gravity UP direction vector. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 2)
	{
		// Gravity up end
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate top end of gravity UP direction vector. "
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

AuPoint DdScVoidProperties::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		UpdatePrompt();
	}
	else if(m_Mode == 1)
	{
		// Start of gravity up vector.
		m_UpStart = loc;
		m_Mode = 2;
		UpdatePrompt();
	}
	else if(m_Mode == 2)
	{
		// End of gravity up vector.
		if(loc != m_UpStart)
		{
			AuPoint upvec(loc - m_UpStart);
			upvec.Unit();
			
			if(SetupVoid(m_GravityType, upvec, m_HoldAccess, m_PersonnelAccess))
				Terminate();
		}
	}
	return ploc;
}

// Set appropriate device cursor.
void DdScVoidProperties::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScVoidProperties::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	if(m_Mode != 2)
		return false;

	// Height point
	switch(e->key())
	{
	case Qt::Key_C:	// Restart
		m_Mode = 1;
		UpdatePrompt();							// Reset prompt.
		return true;

	default:
		break;
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScVoidProperties::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	ts = model * ts.Inverse();

	// Use line shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts);
	re.GLSL().LineShader().SetColor(.7f, 1, .7f);

	// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
	re.glDisable(GL_DEPTH_TEST);

	if(m_Mode == 2)
	{
		// Make a VAO of the vertex locations.
		AuPointArray pts;
		pts.append(m_UpStart);
		pts.append(loc);
		AuGLVAO vao;
		vao.CreateForLines(re, pts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		AuPoint vec = loc - m_UpStart;
		vec = m_pWnd->DesignPlane().UnProjectVector(vec);
		QString sloc = g_AuClient.FormatCoordinate(vec);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		re.RenderText(projection, ts, loc, sloc, g_Au.FontDesigner(), QColor(255,255,255), m_pWnd->size().height());
		re.glDisable(GL_BLEND);
	}

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScVoidProperties::SetupVoid(GRAVITY gravityType, const AuPoint& gravityN, HOLD_ACCESS holdAccess, quint8 personnelAccess)
{
	auApp->DesignerPartsSetVoidProperties(m_pWnd->DesignerPartSelection(), gravityType, gravityN, holdAccess,
		personnelAccess);
	return true;
}
