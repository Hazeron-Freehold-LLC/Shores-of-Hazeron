//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscenvvoidspindle.cpp,v 1.5 2022/02/19 18:26:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscenvvoidspindle.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auscspindledlg.h"
#include "auframe.h"
#include "auscparts.h"
#include "auscvoidpropsdlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScEnvVoidSpindle::DdScEnvVoidSpindle(AuDesignerView* view) : DdCommand(view)
{
	m_SpinAxis = 0;
	m_SpinSteps = 16;
}

DdScEnvVoidSpindle::~DdScEnvVoidSpindle()
{

}

QString DdScEnvVoidSpindle::WikiTopic() const
{
	return "Designer_Menu#Environment_Void";
}

void DdScEnvVoidSpindle::UpdatePrompt() const
{
	QString prompt;
	if(m_Points.size() == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Start spindle shape. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Points.size() < 2)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Continue spindle shape. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Continue spindle shape. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScEnvVoidSpindle::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	m_Points.append(ploc);
	UpdatePrompt();
	return loc;
}

// Set appropriate device cursor.
void DdScEnvVoidSpindle::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScEnvVoidSpindle::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_B:	// Back
		if(!m_Points.empty())
		{
			m_Points.pop_back();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		if(!m_Points.empty())
		{
			m_Points.clear();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_D:	// Done
		if(!m_Points.empty())
		{
			AuScSpindleDlg dlg(AuScEnvironmentVoid::TypeStatic(), m_pWnd);
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				if(MakeSpindle(dlg.Steps(), dlg.Axis()))
				{
					m_Points.clear();
					UpdatePrompt();
				}
				return true;
			}
		}
		break;

	default:
		break;
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdScEnvVoidSpindle::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(!m_Points.empty())
	{
		// Make a VAO of the vertex locations.
		AuPointArray pts(m_Points);
		pts.append(loc);
		AuGLVAO vao;
		vao.CreateForLines(re, pts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		AuPoint vec = loc - m_Points.last();
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

bool DdScEnvVoidSpindle::MakeSpindle(quint16 steps, int axis)
{
	if(steps < 3)
		return false;
	if(m_Points.size() < 2)
		return false;

	AuMatrix4x4 spindleM(m_pWnd->DesignPlane());
	if(axis == 1)
		spindleM.RotateZ(PI/2);
	else if(axis == 2)
		spindleM.RotateY(-PI/2);

	AuScEnvironmentVoid mesh;
	mesh.MakeSpindle(m_Points, steps, spindleM);

	AuScSelectInfo selInfo;
	mesh.VertexIds(selInfo.m_Vertices);
	mesh.GlowVertices(selInfo, QColor(32,32,255));
	mesh.SetFaceOpacity(.25f);

	if(!mesh.IsValid())
		return false;

	mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(mesh);
	return true;
}
