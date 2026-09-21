//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddbdbuildzone.cpp,v 1.6 2022/02/19 18:26:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddbdbuildzone.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aubdsitedlg.h"
#include "auscparts.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QInputDialog>
#include <QMouseEvent>

DdBdBuildZone::DdBdBuildZone(AuDesignerView* view) : DdCommand(view)
{
	m_Round = false;
	m_Steps = 16;
}

DdBdBuildZone::~DdBdBuildZone()
{

}

void DdBdBuildZone::Start()
{
	DdCommand::Start();
}

QString DdBdBuildZone::WikiTopic() const
{
	return "Designer_Menu#Build_Zone";
}

void DdBdBuildZone::UpdatePrompt() const
{
	QString prompt;
	if(m_Round)
	{
		if(m_Points.size() == 0)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Center of round build zone. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">G%1Round, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else if(m_Points.size() == 1)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Radius point of round build zone. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else
	{
		if(m_Points.size() == 0)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Start build zone shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">G%1Round, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		else if(m_Points.size() < 3)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Continue build zone shape. "
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
					"<span style=\"color: %2;\">Continue build zone shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdBdBuildZone::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	if(m_Points.empty() || m_Points.last() != loc)
		m_Points.append(loc);
	if(m_Round && m_Points.size() == 2)
	{
		// We have our two points to make a circle.
		AuPointArray pts;
		pts.MakeCircle(m_Points.first(), loc, m_Steps);
		if(MakeOutline(pts))
			Terminate();
	}
	else
		UpdatePrompt();
	return loc;
}

// Set appropriate device cursor.
void DdBdBuildZone::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdBdBuildZone::OnKeyDown(QKeyEvent* e)
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
		if(m_Round)
			break;
		if(MakeOutline(m_Points))
		{
			m_Points.clear();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_G:	// Round
		if(m_Points.size() == 0)
		{
			m_Round = !m_Round;
			if(m_Round)
			{
				// Round selected, ask for the number of circle sides.
				int lastSides = g_Au.Settings().readNumEntry("/Settings/BuildZoneCircleSteps", 16);
				bool ok;
				int circleSides = QInputDialog::getInt(m_pWnd, g_Au.ApplicationName(),
						QObject::tr("Enter the number of sides around the circle."), lastSides, 3, 1000, 1, &ok);
				if(ok)
				{
					m_Steps = circleSides;
					g_Au.Settings().writeEntry("/Settings/BuildZoneCircleSteps", m_Steps);
				}
				else
					m_Round = false;	// Put it back off.
			}
			UpdatePrompt();
		}
		break;

	default:
		break;
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdBdBuildZone::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(!m_Points.empty())
	{
		// Use line shader so lines will be super bright, even at night.
		re.GLSL().LineShader().Select();
		re.GLSL().LineShader().SetProjectionModelView(projection * ts);
		re.GLSL().LineShader().SetColor(.7f, 1, .7f);

		// Stairs conform exactly to the grid so the dimensions change between diagonal and straight.
		re.glDisable(GL_DEPTH_TEST);

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		if(m_Round)
		{
			AuPointArray pts;
			pts.MakeCircle(m_Points.last(), loc, m_Steps);
			vao.CreateForLines(re, pts, true, GL_STREAM_DRAW);
		}
		else
		{
			AuPointArray pts(m_Points);
			pts.append(loc);
			vao.CreateForLines(re, pts, true, GL_STREAM_DRAW);
		}
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

		re.glEnable(GL_DEPTH_TEST);

		// Restore rendering states.
		re.GLSL().LineShader().Deselect();
	}

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdBdBuildZone::MakeOutline(const AuPointArray& points)
{
	if(points.size() < 3)
		return false;

	AuScBuildZone mesh;
	mesh.MakeBuildZone(points, m_pWnd->DesignPlane());
	if(!mesh.IsValid())
		return false;

	mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(mesh);
	return true;
}

