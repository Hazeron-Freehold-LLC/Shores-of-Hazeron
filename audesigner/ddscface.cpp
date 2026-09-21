//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscface.cpp,v 1.7 2022/02/19 18:26:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscface.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscfacedlg.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScFace::DdScFace(AuDesignerView* view) : DdCommand(view)
{
	m_Steps = 0;	// Not round
}

DdScFace::~DdScFace()
{

}

QString DdScFace::WikiTopic() const
{
	return "Designer#Face_2";
}

void DdScFace::UpdatePrompt() const
{
	QString prompt;
	if(m_Points.size() == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Start face shape or center of round face. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Steps)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Radius point of round face. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Not Round, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Points.size() == 1)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Continue face shape. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Round, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else if(m_Points.size() < 3)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Continue face shape. "
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
				"<span style=\"color: %2;\">Continue face shape. "
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

AuPoint DdScFace::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	m_Points.append(ploc);
	if(m_Steps && !m_Points.empty())
	{
		// Loc is the radius point.
		if(MakeFace(loc, m_Steps))
		{
			m_Points.clear();
			m_Steps = 0;
		}
	}
	UpdatePrompt();
	return ploc;
}

// Set appropriate device cursor.
void DdScFace::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScFace::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_S:	// Round toggle
		if(m_Points.size() == 1)
		{
			if(m_Steps)
				m_Steps = 0;
			else
			{
				AuScFaceDlg dlg(m_pWnd);
				if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
				{
					if(dlg.Mode() == AuScFaceDlg::RadiusEntered)
					{
						// Calculate radius point.
						AuPoint radLoc = m_Points.first() + m_pWnd->DesignPlane().ProjectVector(dlg.Radius(), 0, 0);
						if(MakeFace(radLoc, dlg.Steps()))
						{
							m_Points.clear();
							m_Steps = 0;
						}
					}
					else
						m_Steps = dlg.Steps();
				}
			}
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_B:	// Back
		if(!m_Points.empty())
		{
			m_Points.pop_back();
			if(m_Points.empty())
				m_Steps = 0;
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		if(!m_Points.empty())
		{
			m_Points.clear();
			m_Steps = 0;
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_D:	// Done
		if(m_Points.size() > 2)
		{
			if(MakeFace())
			{
				m_Points.clear();
				m_Steps = 0;
				UpdatePrompt();
			}
		}
		break;

	default:
		break;
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScFace::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
		AuPointArray pts;
		if(m_Steps > 2)
		{
			// Round
			// Calculate radius in the grid plane.
			AuPoint cp = m_pWnd->DesignPlane().UnProject(m_Points.first());
			AuPoint lp = m_pWnd->DesignPlane().UnProject(loc);
			lp.z = cp.z;
			pts.MakeCircle(cp, lp, m_Steps);
			pts.Transform(m_pWnd->DesignPlane());
		}
		else
		{
			// Make a VAO of the vertex locations.
			pts = m_Points;
			pts.append(loc);
		}

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

bool DdScFace::MakeFace()
{
	if(m_Points.size() < 3)
		return false;

	auApp->DesignerPartFillFace(m_pWnd->DesignerPartObjectSelection(), m_Points, m_pWnd->DesignPlane());
	return true;
}

bool DdScFace::MakeFace(const AuPoint& radPoint, int steps)
{
	if(m_Points.empty())
		return false;
	if(steps < 3)
		return false;

	// Make the circle in the grid plane.
	AuPoint cp = m_pWnd->DesignPlane().UnProject(m_Points.first());
	AuPoint rp = m_pWnd->DesignPlane().UnProject(radPoint);
	rp.z = cp.z;
	if(cp.FuzzyEquals(rp))
		return false;

	AuPointArray pts;
	pts.MakeCircle(cp, rp, steps);
	pts.Transform(m_pWnd->DesignPlane());
	if(pts.last() != pts.first())
		pts.append(pts.first());

	// Make the cut.
	auApp->DesignerPartFillFace(m_pWnd->DesignerPartSelection(), pts, m_pWnd->DesignPlane());

	return true;
}
