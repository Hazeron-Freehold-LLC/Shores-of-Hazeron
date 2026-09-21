//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscstretch.cpp,v 1.9 2022/02/19 18:27:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscstretch.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScStretch::DdScStretch(const AuPoint& ref, double stretchBase, AuDesignerView* view) : DdCommand(view)
{
	m_ConstrainX = true;
	m_ConstrainY = true;
	m_ConstrainZ = true;
	m_DragPlane.Translate(ref);
	m_DragPlane.Rotate(view->DesignPlaneMatrix().GetRotation());
	m_StretchBase = stretchBase;
	if(m_StretchBase <= 0)
		m_StretchBase = 1;
}

void DdScStretch::Start()
{
	DdCommand::Start();

	AuPoint ref = m_DragPlane.Translation();
	m_pWnd->DesignerDragStart(ref);
}

QString DdScStretch::WikiTopic() const
{
	return "Designer_Menu#Stretch_Selected_Objects";
}

void DdScStretch::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
					"<span style=\"color: %2;\">Stretch selection. "
					"</span>"
					"<span style=\"color: rgb(255, 255, 0);\">X,Y,Z%1Lock Axis, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Ctrl%1Copy, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
					"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
					"</html>")
				.arg('=')
				.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScStretch::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	AuPoint stretchVec = m_DragPlane.UnProject(loc);
	if(stretchVec.IsZero())
		return loc;

	AuMatrix4x4 dragT(m_pWnd->DesignPlaneMatrix());
	if(fromKeyboard)
	{
		// Convert back to the values they entered, which were in meters, implied scale.
		AuPoint scl = stretchVec / g_AuClient.DesignerUnitScale();
		if(scl.x != 0 && scl.y == 0 && scl.z == 0)
		{
			// We get (N,0,0) when they enter a single number. This way, entering '2' is equivalent to (2,2,2).
			scl.y = scl.x;
			scl.z = scl.x;
		}
		dragT.Scale(scl);
	}
	else
	{
		double scale = stretchVec.Length() / m_StretchBase;
		double sx = scale;
		double sy = scale;
		double sz = scale;

		// No stretch constraint while all constraints are on.
		if(!m_ConstrainX || !m_ConstrainY || !m_ConstrainZ)
		{
			if(m_ConstrainX)
				sx = 1;
			if(m_ConstrainY)
				sy = 1;
			if(m_ConstrainZ)
				sz = 1;
		}

		// Scale relative to the grid.
		dragT.Scale(sx, sy, sz);
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

void DdScStretch::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

bool DdScStretch::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_X:
		m_ConstrainX = !m_ConstrainX;
		break;

	case Qt::Key_Y:
		m_ConstrainY = !m_ConstrainY;
		break;

	case Qt::Key_Z:
		m_ConstrainZ = !m_ConstrainZ;
		break;

	default:
		break;
	}

	return false;
}

void DdScStretch::OnMouseMove(QMouseEvent* e)
{
	AuPoint destLoc = m_pWnd->FromPixelToDesignPlane(e->pos(), true, 0);
	AuPoint stretchVec = m_DragPlane.UnProject(destLoc);
	if(stretchVec.IsZero())
		return;

	double scale = stretchVec.Length() / m_StretchBase;
	double sx = scale;
	double sy = scale;
	double sz = scale;

	// No stretch constraint while all constraints are on.
	if(!m_ConstrainX || !m_ConstrainY || !m_ConstrainZ)
	{
		if(m_ConstrainX)
			sx = 1;
		if(m_ConstrainY)
			sy = 1;
		if(m_ConstrainZ)
			sz = 1;
	}


	// Scale relative to the grid.
	AuMatrix4x4 dragT(m_pWnd->DesignPlaneMatrix());
	dragT.Scale(sx, sy, sz);
	dragT *= m_pWnd->DesignPlaneMatrix().Inverse();

	m_pWnd->DesignerDragging(dragT, e->modifiers() == Qt::ControlModifier);
}

AuMatrix4x4 DdScStretch::DesignPlane() const
{
	return m_DragPlane;
}
