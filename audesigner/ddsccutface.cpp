//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsccutface.cpp,v 1.7 2022/02/19 18:26:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsccutface.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auscspindledlg.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScCutFace::DdScCutFace(AuDesignerView* view) : DdCommand(view)
{
}

DdScCutFace::~DdScCutFace()
{

}

QString DdScCutFace::WikiTopic() const
{
	return "Designer_Menu#Cut_Face";
}

void DdScCutFace::UpdatePrompt() const
{
	QString prompt;
	if(m_Points.size() == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Start cut line. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Continue cut line. "
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

AuPoint DdScCutFace::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	AuPoint ploc(loc);
	m_Points.append(ploc);
	UpdatePrompt();
	return ploc;
}

// Set appropriate device cursor.
void DdScCutFace::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScCutFace::OnKeyDown(QKeyEvent* e)
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
		if(m_Points.size() > 1)
		{
			if(DoIt())
			{
				m_Points.clear();
				UpdatePrompt();
			}
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScCutFace::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	re.glDisable(GL_DEPTH_TEST);
	re.glEnable(GL_LINE_SMOOTH);

	// Use line shader so lines will be super bright, even at night.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts);
	re.GLSL().LineShader().SetColor(1, 0, 1);

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

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	re.glDisable(GL_LINE_SMOOTH);
	re.glEnable(GL_DEPTH_TEST);

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScCutFace::DoIt()
{
	if(m_Points.size() < 2)
		return false;

	// Make the cut.
	auApp->DesignerCutFaces(m_pWnd->DesignerPartSelection(), m_Points);

	return true;
}
