//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscrotate.cpp,v 1.8 2022/02/19 18:27:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscrotate.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScRotate::DdScRotate(const AuPoint& ref, AuDesignerView* view) : DdCommand(view)
{
	m_ConstrainX = false;
	m_ConstrainY = false;
	m_ConstrainZ = false;
	m_DragPlane.Translate(ref);
	m_DragPlane.Rotate(view->DesignPlaneMatrix().GetRotation());
}

void DdScRotate::Start()
{
	DdCommand::Start();

	AuPoint ref = m_DragPlane.Translation();
	m_pWnd->DesignerDragStart(ref);
}

QString DdScRotate::WikiTopic() const
{
	return "Designer_Menu#Rotate_Selected_Objects";
}

void DdScRotate::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Rotate selection. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">X,Y,Z%1Rotation Axis, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Ctrl%1Copy, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScRotate::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	AuPoint rotVec = m_DragPlane.UnProject(loc);
	if(rotVec.IsZero())
		return loc;

	// Rotate relative to the grid.
	AuMatrix4x4 dragT(m_pWnd->DesignPlaneMatrix());
	if(fromKeyboard)
	{
		// Convert vector entered as if they typed in a 3D rotation, in degrees.
		AuPoint rot = rotVec / g_AuClient.DesignerUnitScale();	// Convert back to the values they entered, which were in meters, implied degrees.
		rot *= DEG2RAD;							// Convert from degrees to radians.
		dragT.Rotate(rot);
	}
	else
	{
		// Rotate around an axis.
		if(m_ConstrainX)
			dragT.RotateX(rotVec.XRotation());
		else if(m_ConstrainY)
			dragT.RotateY(rotVec.YRotation());
		else
			dragT.RotateZ(rotVec.ZRotation());
	}
	dragT *= m_pWnd->DesignPlaneMatrix().Inverse();

	if(!dragT.IsIdentity())
	{
		if(m_pWnd->DesignerObjectMode())
		{
			if(keymods == Qt::ControlModifier)
				auApp->DesignerCopyPartObjects(m_pWnd->DesignerPartObjectSelection(), dragT);
			else
				auApp->DesignerMovePartObjects(m_pWnd->DesignerPartObjectSelection(), dragT);
		}
		else
		{
			if(keymods == Qt::ControlModifier)
				auApp->DesignerCopyParts(m_pWnd->DesignerPartSelection(), dragT);
			else
				auApp->DesignerMoveParts(m_pWnd->DesignerPartSelection(), dragT);
		}
	}

	if(keymods == Qt::ControlModifier)
	{
		// Copying so keep going to make multiples easy.
		m_pWnd->DesignerDragStart(loc);
	}
	else
	{
		// Moving doesn't make sense to continue.
		Terminate();
	}

	return loc;
}

void DdScRotate::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	// Draw the blip in pixel space.
	AuMatrix4x4 mvp = projection * model * ts;
	DrawBlip(mvp, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScRotate::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_X:
		m_ConstrainX = !m_ConstrainX;
		m_ConstrainY = false;
		m_ConstrainZ = false;
		break;

	case Qt::Key_Y:
		m_ConstrainX = false;
		m_ConstrainY = !m_ConstrainY;
		m_ConstrainZ = false;
		break;

	case Qt::Key_Z:
		m_ConstrainX = false;
		m_ConstrainY = false;
		m_ConstrainZ = !m_ConstrainZ;
		break;

	default:
		break;
	}

	return false;
}

void DdScRotate::OnMouseMove(QMouseEvent* e)
{
	AuPoint destLoc = m_pWnd->FromPixelToDesignPlane(e->pos(), true, 0);
	AuPoint rotVec = m_DragPlane.UnProject(destLoc);
	if(rotVec.IsZero())
		return;

	// Rotate relative to the grid.
	AuMatrix4x4 dragT(m_pWnd->DesignPlaneMatrix());
	if(m_ConstrainX)
		dragT.RotateX(rotVec.XRotation());
	else if(m_ConstrainY)
		dragT.RotateY(rotVec.YRotation());
	else
		dragT.RotateZ(rotVec.ZRotation());
	dragT *= m_pWnd->DesignPlaneMatrix().Inverse();

	m_pWnd->DesignerDragging(dragT, e->modifiers() == Qt::ControlModifier);
}

AuMatrix4x4 DdScRotate::DesignPlane() const
{
	if(m_ConstrainX)
	{
		// Rotating around X requires us to rotate the drag plane.
		AuPoint org = m_DragPlane.Translation();
		AuPoint rot = m_pWnd->DesignPlaneMatrix().GetRotation();
		AuMatrix4x4 dragPlane;
		dragPlane.Translate(org);
		dragPlane.Rotate(rot);
		dragPlane.RotateY(PI/2);
		return dragPlane;
	}
	else if(m_ConstrainY)
	{
		// Rotating around Y requires us to rotate the drag plane.
		AuPoint org = m_DragPlane.Translation();
		AuPoint rot = m_pWnd->DesignPlaneMatrix().GetRotation();
		AuMatrix4x4 dragPlane;
		dragPlane.Translate(org);
		dragPlane.Rotate(rot);
		dragPlane.RotateX(PI/2);
		return dragPlane;
	}
	else
		return m_DragPlane;
}
