//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdfence.cpp,v 1.10 2022/02/19 18:26:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddbdfence.h"
//
#include "auapp.h"
#include "aubdfencedlg.h"
#include "auclientglobal.h"
#include "audesignerview.h"
#include "auframe.h"
#include "auscparts.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdBdFence::DdBdFence(AuDesignerView* view) : DdCommand(view)
{
	m_DesignId = 0;
	m_Mode = 0;
	m_Style = 0;
}

DdBdFence::~DdBdFence()
{

}

void DdBdFence::Start()
{
	DdCommand::Start();

	AuBdFenceDlg dlg(m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) != QDialog::Accepted)
	{
		Terminate();
		return;
	}

	m_DesignId = dlg.DesignId();
	m_Style = dlg.Style();
}

QString DdBdFence::WikiTopic() const
{
	return "Designer_Menu#Fence.2FWall";
}

void DdBdFence::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Start of fence segment. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 1)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">End of fence segment. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdBdFence::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		m_FromLoc = loc;
		m_Mode = 1;
		UpdatePrompt();
	}
	else if(m_Mode == 1)
	{
		MakeFence(loc);
		m_FromLoc = loc;
		UpdatePrompt();
	}
	return ploc;
}

// Set appropriate device cursor.
void DdBdFence::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdBdFence::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

// Render "cursor" objects in the scene.
void DdBdFence::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(m_Mode == 1)
	{
		// Make a VAO of the vertex locations.
		AuPointArray pts;
		pts.append(m_FromLoc);
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

		AuPoint vec = loc - m_FromLoc;
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

bool DdBdFence::MakeFence(const AuPoint& toLoc)
{
	AuScFence fence;
	fence.MakeFence((AuScFence::STYLE)m_Style, m_DesignId, m_FromLoc, toLoc);
	if(!fence.IsValid())
		return false;

	fence.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(fence);
	return true;
}
