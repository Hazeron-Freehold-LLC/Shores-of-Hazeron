//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscmove.cpp,v 1.8 2022/02/19 18:27:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscmove.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScMove::DdScMove(const AuPoint& ref, AuDesignerView* view) : DdCommand(view)
{
	m_ConstrainX = false;
	m_ConstrainY = false;
	m_ConstrainZ = false;
	m_DragPlane.Translate(ref);
	m_DragPlane.Rotate(view->DesignPlaneMatrix().GetRotation());
}

QString DdScMove::WikiTopic() const
{
	return "Designer_Menu#Grab_Selected_Objects";
}

void DdScMove::Start()
{
	DdCommand::Start();

	AuPoint ref = m_DragPlane.Translation();
	m_pWnd->DesignerDragStart(ref);
}

void DdScMove::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Move selection. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">X,Y,Z%1Movement Axis, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Ctrl%1Copy, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScMove::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	AuPoint startLoc = m_DragPlane.Translation();
	AuPoint vec = loc - startLoc;
	if(!fromKeyboard)
	{
		// Apply constraints when not from keyboard.
		if(m_ConstrainX || m_ConstrainY || m_ConstrainZ)
		{
			// Constraints must be applied in the space of the work plane.
			vec = m_pWnd->DesignPlaneMatrix().UnProjectVector(vec);
			if(m_ConstrainX)
			{
				vec.y = 0;
				vec.z = 0;
			}
			else if(m_ConstrainY)
			{
				vec.x = 0;
				vec.z = 0;
			}
			else if(m_ConstrainZ)
			{
				vec.x = 0;
				vec.y = 0;
			}
			vec = m_pWnd->DesignPlaneMatrix().ProjectVector(vec);
		}
	}

	if(!vec.IsZero())
	{
		AuMatrix4x4 dragT;
		dragT.Translate(vec);

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
		// Must update the drag plane since the selected items are now at a new location.
		m_DragPlane.LoadIdentity();
		m_DragPlane.Translate(loc);
		m_DragPlane.Rotate(m_pWnd->DesignPlaneMatrix().GetRotation());
		m_pWnd->DesignerDragStart(loc);
	}
	else
	{
		// Moving doesn't make sense to continue.
		Terminate();
	}

	return loc;
}

void DdScMove::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

bool DdScMove::OnKeyDown(QKeyEvent* e)
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

void DdScMove::OnMouseMove(QMouseEvent* e)
{
	AuPoint destLoc = m_pWnd->FromPixelToDesignPlane(e->pos(), true, 0);
	AuPoint startLoc = m_DragPlane.Translation();
	AuPoint vec = destLoc - startLoc;

	// Apply constraints.
	if(m_ConstrainX || m_ConstrainY || m_ConstrainZ)
	{
		// Constraints must be applied in the space of the work plane.
		vec = m_pWnd->DesignPlaneMatrix().UnProjectVector(vec);
		if(m_ConstrainX)
		{
			vec.y = 0;
			vec.z = 0;
		}
		else if(m_ConstrainY)
		{
			vec.x = 0;
			vec.z = 0;
		}
		else if(m_ConstrainZ)
		{
			vec.x = 0;
			vec.y = 0;
		}
		vec = m_pWnd->DesignPlaneMatrix().ProjectVector(vec);
	}

	AuMatrix4x4 dragT;
	if(!vec.IsZero())
		dragT.Translate(vec);
	m_pWnd->DesignerDragging(dragT, e->modifiers() == Qt::ControlModifier);
}

AuMatrix4x4 DdScMove::DesignPlane() const
{
	// This works for DESIN_Grab and simple dragging.
	if(!m_ConstrainZ)
		return DdCommand::DesignPlane();

	// Dragging in Z requires us to rotate the drag plane.
	AuPoint org = m_DragPlane.Translation();
	AuPoint rot = m_pWnd->DesignPlaneMatrix().GetRotation();
	AuMatrix4x4 dragPlane;
	dragPlane.Translate(org);
	dragPlane.Rotate(rot);
	dragPlane.RotateX(PI/2);
	return dragPlane;
}
