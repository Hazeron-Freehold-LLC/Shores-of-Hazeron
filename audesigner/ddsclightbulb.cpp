//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsclightbulb.cpp,v 1.8 2022/02/19 18:26:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsclightbulb.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScLightBulb::DdScLightBulb(AuDesignerView* view) : DdCommand(view)
{
	m_GetPoint = 0;
}

QString DdScLightBulb::WikiTopic() const
{
	return "Designer_Menu#Light_Bulb_2";
}

void DdScLightBulb::UpdatePrompt() const
{
	QString prompt;
	if(m_GetPoint == 0)
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Light location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Light aim point. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Downward, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">F%1Forward, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">N%1Omnidirectional, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScLightBulb::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	if(m_GetPoint == 0)
	{
		m_EmitLoc = loc;
		m_GetPoint = 1;
		UpdatePrompt();
	}
	else
	{
		if(DoSpot(loc))
			Terminate();
	}

	return loc;
}

void DdScLightBulb::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuGLEffects& re, QOpenGLWidget* glw, bool snappedToExistingShape)
{
	const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
	if(!pde)
		return;

	// Cursor lines must be relative to scene atom.
	AuMatrix4x4 ts;
	sceneAtom->GetTransformRelativeToParent(ts, pde);
	ts = ts.Inverse();

	AuMatrix4x4 pmv = projection * model * ts;

	// Get cursor location in coordinates relative to the design environment.
	AuPoint loc = m_pWnd->FromPixelToDesignPlane(sceneAtom, pxCursor, true, 0);

	if(m_GetPoint)
	{
		AuPointArray verts;
		verts.append(m_EmitLoc);
		verts.append(loc);

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(pmv);
			re.GLSL().LineShader().SetColor(1,1,0,1);

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

bool DdScLightBulb::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_D:
		if(m_GetPoint)
		{
			AuPoint aimLoc = m_EmitLoc + AuPoint(0,0,-1);
			DoSpot(aimLoc);
			Terminate();
			return true;
		}
		break;

	case Qt::Key_F:
		if(m_GetPoint)
		{
			AuPoint aimLoc = m_EmitLoc + AuPoint(0,1,0);
			DoSpot(aimLoc);
			Terminate();
			return true;
		}
		break;

	case Qt::Key_N:
		if(m_GetPoint)
		{
			DoOmni();
			Terminate();
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

bool DdScLightBulb::DoSpot(const AuPoint& aimLoc)
{
	QColor color;
	float kL;
	float kQ;
	float cosInnerCone;
	float cosOuterCone;
	if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Spacecraft)
	{
		// Typical spacecraft headlight.
		float atten = 200 * FeetPerMeter();
		color = AuGLLight::ColorCarbonArc();
		kL = .005f;
		kQ = 1/(atten * atten);
		cosInnerCone = .939692621f;	// 20 degrees
		cosOuterCone = .5f;			// 60 degrees
	}
	else if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Building)
	{
		// Typical street light.
		float atten = 10 * FeetPerMeter();
		color = AuGLLight::ColorSodiumVapor();
		kL = .005f;
		kQ = 1/(atten * atten);
		cosInnerCone = .866025404f;	// 30 degrees
		cosOuterCone = .173648178f;	// 80 degrees
	}
	else
	{
		// Typical lantern.
		float atten = 10 * FeetPerMeter();
		color = AuGLLight::ColorHalogen();
		kL = .0065f;
		kQ = 1/(atten * atten);
		cosInnerCone = .866025404f;	// 30 degrees
		cosOuterCone = .5f;			// 60 degrees
	}

	// Calculate the plane and rotation so we can point the icon in the right direction.
	AuPoint xv = aimLoc - m_EmitLoc;
	xv.Unit();
	AuPoint yv = AuPoint(0,0,1).Cross(xv);
	yv.Unit();

	AuMatrix4x4 plane(m_EmitLoc, xv, yv);
	plane.RotateZ(-PI/2);

	AuScLightBulb bulb;
	bulb.SetLight(color, kL, kQ, cosOuterCone, cosInnerCone);
	bulb.MakeIcon(plane);
	if(!bulb.IsValid())
		return false;

	bulb.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(bulb);
	return true;
}

bool DdScLightBulb::DoOmni()
{
	QColor color;
	float kL;
	float kQ;
	float cosInnerCone = -1;
	float cosOuterCone = -1;
	if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Spacecraft)
	{
		// Typical spacecraft headlight.
		float atten = 200 * FeetPerMeter();
		color = AuGLLight::ColorCarbonArc();
		kL = .005f;
		kQ = 1/(atten * atten);
	}
	else if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Building)
	{
		// Typical street light.
		float atten = 10 * FeetPerMeter();
		color = AuGLLight::ColorSodiumVapor();
		kL = .005f;
		kQ = 1/(atten * atten);
	}
	else
	{
		// Typical lantern.
		float atten = 10 * FeetPerMeter();
		color = AuGLLight::ColorHalogen();
		kL = .0065f;
		kQ = 1/(atten * atten);
	}

	AuScLightBulb bulb;
	bulb.SetLight(color, kL, kQ, cosOuterCone, cosInnerCone);
	bulb.MakeIcon(m_EmitLoc, 0, m_pWnd->DesignPlane());
	if(!bulb.IsValid())
		return false;

	bulb.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(bulb);
	return true;
}
