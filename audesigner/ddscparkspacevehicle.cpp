//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscparkspacevehicle.cpp,v 1.7 2022/02/19 18:27:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscparkspacevehicle.h"
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

DdScParkSpaceVehicle::DdScParkSpaceVehicle(AuDesignerView* view) : DdCommand(view)
{
	m_Rot = 0;
}

QString DdScParkSpaceVehicle::WikiTopic() const
{
	return "Designer_Menu#Space_Vehicle_Parking_Spot";
}

void DdScParkSpaceVehicle::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Parking spot location. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">[Shift] Left/Right Arrow%1Rotate, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScParkSpaceVehicle::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);

	MakeIt(loc);

	return loc;
}

void DdScParkSpaceVehicle::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	ts = ts.Inverse();

	AuMatrix4x4 pmv = projection * model * ts;

	{
		float rot = m_Rot * DEG2RADf;
		while(rot < -PIf)
			rot += PIf*2;
		while(rot > PIf)
			rot -= PIf*2;

		AuMatrix4x4 trot(pmv);
		trot.Translate(loc);
		trot.Rotate(m_pWnd->DesignPlane().GetRotation());
		trot.RotateZ(rot);

		AuPointArray verts;
		verts.append(AuPoint(-FeetPerMeter()*6, -FeetPerMeter()*8.5));
		verts.append(AuPoint(-FeetPerMeter()*6, FeetPerMeter()*8.5));
		verts.append(AuPoint(FeetPerMeter()*6, FeetPerMeter()*8.5));
		verts.append(AuPoint(FeetPerMeter()*6, -FeetPerMeter()*8.5));
		verts.append(AuPoint(-FeetPerMeter()*6, -FeetPerMeter()*8.5));
		verts.append(AuPoint(0, FeetPerMeter()*8.5));
		verts.append(AuPoint(FeetPerMeter()*6, -FeetPerMeter()*8.5));

		AuGLVAO vao;
		vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetProjectionModelView(trot);
			re.GLSL().LineShader().SetColor(1,1,1,1);

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

bool DdScParkSpaceVehicle::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_Right:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot += -1;
		else
			m_Rot += -10;
		break;

	case Qt::Key_Left:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot += 1;
		else
			m_Rot += 10;
		break;

	default:
		break;
	}

	return false;
}

bool DdScParkSpaceVehicle::MakeIt(const AuPoint& loc)
{
	AuScSpaceVehicleParkingSpot park;
	park.MakeIcon(loc, m_Rot*DEG2RADf, m_pWnd->DesignPlane());
	if(!park.IsValid())
		return false;

	park.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(park);
	return true;
}
