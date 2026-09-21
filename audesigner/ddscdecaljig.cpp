//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscdecaljig.cpp,v 1.8 2022/02/19 18:26:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscdecaljig.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscdecaljigdlg.h"
#include "auscjigcircledlg.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScDecalJig::DdScDecalJig(AuDesignerView* view) : DdCommand(view)
{
	m_Mode = 0;
	m_Steps = 0;	// Not round
}

DdScDecalJig::~DdScDecalJig()
{

}

QString DdScDecalJig::WikiTopic() const
{
	return "Designer_Menu#Decal";
}

void DdScDecalJig::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		// Primary shape.
		if(m_Points.size() == 0)
		{
			prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Start jig shape or center of round jig. "
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
					"<span style=\"color: %2;\">Radius point of round jig. "
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
					"<span style=\"color: %2;\">Continue jig shape. "
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
					"<span style=\"color: %2;\">Continue jig shape. "
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
					"<span style=\"color: %2;\">Continue jig shape. "
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
				"<span style=\"color: %2;\">Locate length reference. "
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

AuPoint DdScDecalJig::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		// Primary boundary.
		if(m_Steps && !m_Points.empty())
		{
			AuPoint cp = m_pWnd->DesignPlane().UnProject(m_Points.first());
			AuPoint rp = m_pWnd->DesignPlane().UnProject(loc);

			m_Points.clear();
			m_Points.MakeCircle(cp, rp, m_Steps);
			m_Points.Transform(m_pWnd->DesignPlane());

			AuScDecalJigDlg dlg(m_pWnd);
			if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
			{
				switch(dlg.Mode())
				{
				case AuScDecalJigDlg::HeightEntered:
					{
						AuPoint extVec(0,0, dlg.Height());
						if(MakeJig(extVec))
						{
							m_Steps = 0;
							m_Points.clear();
							UpdatePrompt();
						}
					}
					break;
				case AuScDecalJigDlg::PickHeight:
					m_Mode = 2;
					UpdatePrompt();
					break;
				case AuScDecalJigDlg::PickVector:
					m_Mode = 3;
					UpdatePrompt();
					break;
				}
			}
		}
		else
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
		if(MakeJig(AuPoint(0,0,extVec.z)))
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
			if(MakeJig(extVec))
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
void DdScDecalJig::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScDecalJig::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	if(m_Mode == 0)
	{
		// Primary boundary
		switch(e->key())
		{
		case Qt::Key_S:	// Round toggle
			if(m_Points.size() == 1)
			{
				if(m_Steps)
					m_Steps = 0;
				else
				{
					AuScJigCircleDlg dlg(m_pWnd);
					if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
					{
						if(dlg.Mode() == AuScJigCircleDlg::RadiusEntered)
						{
							// Calculate center and radius points.
							AuPoint cp = m_pWnd->DesignPlane().UnProject(m_Points.first());
							AuPoint rp = cp + AuPoint(dlg.Radius(), 0, 0);

							m_Points.clear();
							m_Points.MakeCircle(cp, rp, dlg.Steps());
							m_Points.Transform(m_pWnd->DesignPlane());

							AuScDecalJigDlg dlg(m_pWnd);
							if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
							{
								switch(dlg.Mode())
								{
								case AuScDecalJigDlg::HeightEntered:
									{
										AuPoint extVec(0,0, dlg.Height());
										if(MakeJig(extVec))
										{
											m_Points.clear();
											UpdatePrompt();
										}
									}
									break;
								case AuScDecalJigDlg::PickHeight:
									m_Mode = 2;
									UpdatePrompt();
									break;
								case AuScDecalJigDlg::PickVector:
									m_Mode = 3;
									UpdatePrompt();
									break;
								}

								return true;
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
				if(m_Points.empty())
					m_Steps = 0;
				UpdatePrompt();
				return true;
			}
			break;

		case Qt::Key_D:	// Done
			if(!m_Points.empty())
			{
				AuScDecalJigDlg dlg(m_pWnd);
				if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
				{
					switch(dlg.Mode())
					{
					case AuScDecalJigDlg::HeightEntered:
						{
							AuPoint extVec(0,0, dlg.Height());
							if(MakeJig(extVec))
							{
								m_Points.clear();
								UpdatePrompt();
							}
						}
						break;
					case AuScDecalJigDlg::PickHeight:
						m_Mode = 2;
						UpdatePrompt();
						break;
					case AuScDecalJigDlg::PickVector:
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
void DdScDecalJig::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
		// Primary boundary.
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
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

			// Draw length line.
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
			QString sloc = QObject::tr("%1 length").arg(g_AuClient.FormatLength(vec.z));

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

bool DdScDecalJig::MakeJig(const AuPoint& vec)
{
	if(m_Points.size() < 3)
		return false;

	AuScDecalJig mesh;
	mesh.MakeExtrusion(m_Points, vec, m_pWnd->DesignPlane(), 1, true);
	if(!mesh.IsValid())
		return false;

	mesh.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(mesh);
	return true;
}
