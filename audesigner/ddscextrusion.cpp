//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscextrusion.cpp,v 1.8 2022/02/19 18:26:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscextrusion.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auscextrusiondlg.h"
#include "auscparts.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScExtrusion::DdScExtrusion(quint8 partType, AuDesignerView* view) : DdCommand(view)
{
	m_Mode = 0;
	m_PartType = partType;
	m_ExtSteps = 1;
}

DdScExtrusion::~DdScExtrusion()
{

}

QString DdScExtrusion::WikiTopic() const
{
	return "Designer#Extrusion";
}

void DdScExtrusion::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		// Primary shape.
		if(m_Points.size() == 0)
		{
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Start extrusion shape. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
		}
		else if(m_Points.size() < 3)
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Continue extrusion shape. "
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
					"<span style=\"color: %2;\">Continue extrusion shape. "
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
	else if(m_Mode == 1)
	{
		// Hole
	}
	else if(m_Mode == 2)
	{
		// Height point
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate height reference. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 3)
	{
		// Vector point
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate direction reference. "
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

AuPoint DdScExtrusion::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		// Primary shape.
		m_Points.append(ploc);
		UpdatePrompt();
	}
	else if(m_Mode == 1)
	{
		// Hole
	}
	else if(m_Mode == 2)
	{
		// Height point
		// Convert point entered to be relative to the grid plane.
		AuPoint extVec = m_pWnd->DesignPlane().UnProject(loc);
		if(MakeExtrusion(AuPoint(0,0,extVec.z)))
		{
			m_Mode = 0;
			m_Points.clear();
			UpdatePrompt();
		}
	}
	else if(m_Mode == 3)
	{
		// Vector point
		// Convert point entered to be a vector relative to the grid plane.
		if(!m_Points.empty())
		{
			AuPoint extVec = loc - m_Points.first();
			extVec = m_pWnd->DesignPlane().UnProjectVector(extVec);
			if(MakeExtrusion(extVec))
			{
				m_Mode = 0;
				m_Points.clear();
				UpdatePrompt();
			}
		}
	}
	return ploc;
}

// Set appropriate device cursor.
void DdScExtrusion::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScExtrusion::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	if(m_Mode == 0)
	{
		// Primary shape
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
				AuScExtrusionDlg dlg(m_PartType, m_pWnd);
				if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
				{
					m_ExtSteps = dlg.Steps();
					switch(dlg.Mode())
					{
					case AuScExtrusionDlg::HeightEntered:
						{
							AuPoint extVec(0,0, dlg.Height());
							if(MakeExtrusion(extVec))
							{
								m_Points.clear();
								UpdatePrompt();
							}
						}
						break;
					case AuScExtrusionDlg::PickHeight:
						m_Mode = 2;
						UpdatePrompt();
						break;
					case AuScExtrusionDlg::PickVector:
						m_Mode = 3;
						UpdatePrompt();
						break;
					}

					return true;
				}
			}
			break;

		default:
			break;
		}
	}
	else if(m_Mode == 1)
	{
		// Hole
	}
	else if(m_Mode == 2)
	{
		// Height point
		switch(e->key())
		{
		case Qt::Key_C:	// Restart
			m_Mode = 0;
			m_Points.clear();
			UpdatePrompt();							// Reset prompt.
			return true;

		default:
			break;
		}
	}
	else if(m_Mode == 3)
	{
		// Vector point
		switch(e->key())
		{
		case Qt::Key_C:	// Restart
			m_Mode = 0;
			m_Points.clear();
			UpdatePrompt();							// Reset prompt.
			return true;

		default:
			break;
		}
	}

	return false;
}

// Render "cursor" objects in the scene.
void DdScExtrusion::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(m_Mode == 0)
	{
		// Primary shape.
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
			AuPointArray pts(m_Points);
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

			AuPoint vec = loc - m_Points.last();
			vec = m_pWnd->DesignPlane().UnProjectVector(vec);
			QString sloc = g_AuClient.FormatCoordinate(vec);

			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			re.RenderText(projection, ts, loc, sloc, g_Au.FontDesigner(), QColor(255,255,255), m_pWnd->size().height());
			re.glDisable(GL_BLEND);
		}
	}
	else if(m_Mode == 1)
	{
		// Hole
	}
	else if(m_Mode == 2)
	{
		// Height point
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(re, m_Points, true, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Draw height line.
			AuPointArray cseg;
			cseg.append(m_Points.first());
			cseg.append(loc);
			vao.CreateForLines(re, cseg, true, GL_STREAM_DRAW);
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
			QString sloc = QObject::tr("%1 height").arg(g_AuClient.FormatLength(vec.z));

			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			re.RenderText(projection, ts, loc, sloc, g_Au.FontDesigner(), QColor(255,255,255), m_pWnd->size().height());
			re.glDisable(GL_BLEND);
		}
	}
	else if(m_Mode == 3)
	{
		// Vector point
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(re, m_Points, true, GL_STREAM_DRAW);
			if(vao.VAO())
			{
				// Display the VAO once and clean it up.
				vao.SelectGL();
				vao.DrawGL(GL_LINE_STRIP);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			// Draw height line.
			AuPointArray cseg;
			cseg.append(m_Points.first());
			cseg.append(loc);
			vao.CreateForLines(re, cseg, true, GL_STREAM_DRAW);
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
	}

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScExtrusion::MakeExtrusion(const AuPoint& vec)
{
	if(m_Points.size() < 3)
		return false;

	if(m_PartType == AuScHull::TypeStatic())
	{
		AuScHull mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScRoom::TypeStatic())
	{
		AuScRoom mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps);
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScLift::TypeStatic())
	{
		AuScLift mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps);
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScLandingGear::TypeStatic())
	{
		AuScLandingGear mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);
		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScBarrier::TypeStatic())
	{
		AuScBarrier mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);

		AuScSelectInfo selInfo;
		mesh.VertexIds(selInfo.m_Vertices);
		mesh.GlowVertices(selInfo, QColor(255,32,32));
		mesh.SetFaceOpacity(.75f);

		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScHullVoid::TypeStatic())
	{
		AuScHullVoid mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);

		AuScSelectInfo selInfo;
		mesh.VertexIds(selInfo.m_Vertices);
		mesh.GlowVertices(selInfo, QColor(32,255,255));
		mesh.SetFaceOpacity(.25f);

		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else if(m_PartType == AuScRoomVoid::TypeStatic())
	{
		AuScRoomVoid mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);

		AuScSelectInfo selInfo;
		mesh.VertexIds(selInfo.m_Vertices);
		mesh.GlowVertices(selInfo, QColor(32,255,32));
		mesh.SetFaceOpacity(.25f);

		if(!mesh.IsValid())
			return false;

		mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
		auApp->DesignerPartAdd(mesh);
		return true;
	}
	else
	{
		// Make a generic mesh.
		AuScMesh mesh;
		mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), m_ExtSteps, true);
		if(!mesh.IsValid())
			return false;

		quint16 partId = m_pWnd->DesignerObjectModeTargetPart();
		if(partId)
		{
			// Add the faces of the mesh to the face mode target mesh.
			AuScFaceList faces;
			mesh.Faces(faces);
			auApp->DesignerPartAddFaces(partId, faces);
		}
		else
		{
			// Create a new mesh.
			mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
			auApp->DesignerPartAdd(mesh);
		}
		return true;
	}
}
