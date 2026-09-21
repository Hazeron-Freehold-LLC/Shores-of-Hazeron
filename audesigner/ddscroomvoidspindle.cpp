//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscroomvoidspindle.cpp,v 1.6 2022/02/19 18:27:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscroomvoidspindle.h"
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

DdScRoomVoidSpindle::DdScRoomVoidSpindle(AuDesignerView* view) : DdCommand(view)
{
	m_GravityType = G_Natural;
	m_HoldAccess = HOLD_Inaccessible;
	m_Mode = 0;
	m_PersonnelAccess = PERSONNEL_Everyone;
	m_SpinAxis = 0;
	m_SpinSteps = 16;
}

DdScRoomVoidSpindle::~DdScRoomVoidSpindle()
{

}

QString DdScRoomVoidSpindle::WikiTopic() const
{
	return "Designer_Menu#Room_Void";
}

void DdScRoomVoidSpindle::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		// Primary shape.
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
	}
	else if(m_Mode == 4)
	{
		// Gravity up start
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Locate bottom end of gravity UP direction vector. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 5)
	{
		// Gravity up start
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

AuPoint DdScRoomVoidSpindle::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	if(m_Mode == 0)
	{
		AuPoint ploc(loc);
		m_Points.append(ploc);
		UpdatePrompt();
	}
	else if(m_Mode == 4)
	{
		// Start of gravity up vector.
		m_UpStart = loc;
		m_Mode = 5;
		UpdatePrompt();
	}
	else if(m_Mode == 5)
	{
		// End of gravity up vector.
		if(loc != m_UpStart)
		{
			AuPoint upvec(loc - m_UpStart);
			upvec.Unit();
			if(MakeSpindle(m_SpinSteps, m_SpinAxis, m_GravityType, upvec, m_HoldAccess, m_PersonnelAccess))
			{
				m_Mode = 0;
				m_Points.clear();
				UpdatePrompt();
			}
		}
	}
	return loc;
}

// Set appropriate device cursor.
void DdScRoomVoidSpindle::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScRoomVoidSpindle::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	if(m_Mode == 0)
	{
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
				AuScSpindleDlg dlg(AuScRoomVoid::TypeStatic(), m_pWnd);
				if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
				{
					AuScVoidPropsDlg roomdlg(m_pWnd);
					if(auApp->Frame()->ExecDialog(roomdlg) == QDialog::Accepted)
					{
						switch(roomdlg.Mode())
						{
						case AuScVoidPropsDlg::UpZ:
							if(MakeSpindle(dlg.Steps(), dlg.Axis(), roomdlg.GravityType(), AuPoint(0,0,1), roomdlg.HoldAccess(),
										roomdlg.PersonnelAccess()))
							{
								m_Mode = 0;
								m_Points.clear();
								UpdatePrompt();
							}
							break;
						case AuScVoidPropsDlg::UpZGrid:
							if(MakeSpindle(dlg.Steps(), dlg.Axis(), roomdlg.GravityType(),
										DesignPlane().ProjectVector(AuPoint(0,0,1)), roomdlg.HoldAccess(),
										roomdlg.PersonnelAccess()))
							{
								m_Mode = 0;
								m_Points.clear();
								UpdatePrompt();
							}
							break;
						case AuScVoidPropsDlg::PickVector:
							m_GravityType = roomdlg.GravityType();
							m_HoldAccess = roomdlg.HoldAccess();
							m_Mode = 4;
							m_PersonnelAccess = roomdlg.PersonnelAccess();
							m_SpinAxis = dlg.Axis();
							m_SpinSteps = dlg.Steps();
							UpdatePrompt();
							break;
						}
					}
					return true;
				}
			}
			break;

		default:
			break;
		}
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdScRoomVoidSpindle::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	}
	else if(m_Mode == 4)
	{
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
			AuPointArray pts(m_Points);
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
		}
	}
	else if(m_Mode == 5)
	{
		if(!m_Points.empty())
		{
			// Make a VAO of the vertex locations.
			AuPointArray pts(m_Points);
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
		}

		AuPointArray pts;
		pts.append(m_UpStart);
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

bool DdScRoomVoidSpindle::MakeSpindle(quint16 steps, int axis, GRAVITY gravityType, const AuPoint& gravityN, HOLD_ACCESS holdAccess,
        quint8 personnelAccess)
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

	AuScRoomVoid mesh;
	mesh.MakeSpindle(m_Points, steps, spindleM);
	mesh.SetGravity(gravityType, gravityN);
	mesh.SetHoldAccess(holdAccess);
	mesh.SetPersonnelAccess(personnelAccess);

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
