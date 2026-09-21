//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpushpull.cpp,v 1.8 2022/02/19 18:27:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscpushpull.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScPushPull::DdScPushPull(const AuPoint& ref, AuDesignerView* view) : DdCommand(view)
{
	m_ConstrainX = false;
	m_ConstrainY = false;
	m_ConstrainZ = false;
	m_DragNegative = false;
	m_DragPlane.Translate(ref);
	m_DragPlane.Rotate(view->DesignPlaneMatrix().GetRotation());
	m_Together = true;
}

void DdScPushPull::Start()
{
	DdCommand::Start();
}

QString DdScPushPull::WikiTopic() const
{
	return "Designer_Menu#Push_Pull";
}

void DdScPushPull::UpdatePrompt() const
{
	QString prompt;
	if(m_Together)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Push/pull together. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">X,Y,Z%1Movement Axis, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Separate, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Ctrl%1Copy, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Push/pull separate. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">S%1Together, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Ctrl%1Copy, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScPushPull::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	quint16 partId = m_pWnd->DesignerObjectModeTargetPart();
	if(!partId)
	{
		Terminate();
		return loc;
	}

	AuPoint dragVec = m_pWnd->DesignPlane().UnProject(loc);
	if(dragVec.IsZero())
	{
		Terminate();
		return loc;
	}

	AuScFaceList faces;
	{
		DaPerson* sceneAtom;
		m_pWnd->LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(!pde)
			{
				m_pWnd->UnlockSceneAtom();
				Terminate();
				return loc;
			}

			// Get a copy of the selected faces.
			pde->Faces(faces, m_pWnd->DesignerPartObjectSelection());
		}
		m_pWnd->UnlockSceneAtom();
	}

	// Create the result of the operation.
	AuScFaceList resultFaces;
	resultFaces.AddFaces(faces);
	if(resultFaces.empty())
	{
		Terminate();
		return loc;
	}

	if(m_Together)
	{
		// Every face moves together.
		if(!fromKeyboard)
		{
			// Apply constraints in the space of the grid.
			if(m_ConstrainX || m_ConstrainY || m_ConstrainZ)
			{
				// Constraints must be applied in the space of the work plane.
				if(m_ConstrainX)
				{
					dragVec.y = 0;
					dragVec.z = 0;
				}
				else if(m_ConstrainY)
				{
					dragVec.x = 0;
					dragVec.z = 0;
				}
				else if(m_ConstrainZ)
				{
					// We tipped the grid plane for Z drag so the Y axis is aligned with the Z constraint axis. 
					// Treat it like a Y axis constraint.
					dragVec.x = 0;
					dragVec.z = 0;
				}
			}
		}

		dragVec = m_pWnd->DesignPlane().ProjectVector(dragVec);	// dragVec projected back to designer coordinate system.

		// This can be done with a single transformation of the entire list.
		AuMatrix4x4 t;
		t.Translate(dragVec);
		resultFaces.Transform(t);

		// Fill with intermediate faces where there is a resulting edge that does not share another resulting edge.
		AuSegmentArray edges;
		for(AuScFaceList::const_iterator fi = resultFaces.cbegin(); fi != resultFaces.cend(); fi++)
		{
			AuSegmentArray fedges;
			(*fi)->EdgeSegments(fedges);
			edges.Intersect(fedges);
		}

		// Add side fill faces.
		for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
		{
			AuPoint pll((*ei).m_fLoc1);
			AuPoint pul((*ei).m_fLoc1 - dragVec);
			AuPoint pur((*ei).m_fLoc2 - dragVec);
			AuPoint plr((*ei).m_fLoc2);
			AuPoint fx = plr - pll;
			fx.Unit();
			AuPoint fy = pul - pll;
			fy.Unit();
			AuPoint fn = fx.Cross(fy);
			fn.Unit();

			AuScPoint ll(pll, 0,0, fn);
			AuScPoint ul(pul, 0,0, fn);
			AuScPoint ur(pur, 0,0, fn);
			AuScPoint lr(plr, 0,0, fn);
			AuScFaceQuad fq(*resultFaces.first(), ll, ul, ur, lr);
			resultFaces.AddFace(fq);
		}
	}
	else
	{
		// Every face moves individually, according to its facing.
		double distToMove = dragVec.Length();
		if(distToMove != 0)
		{
			// We have no view projection at this point so use the last known sign indicator from the drag.
			if(m_DragNegative)
				distToMove = -distToMove;

			AuScFaceList fillFaces;
			for(AuScFaceList::const_iterator fi = resultFaces.cbegin(); fi != resultFaces.cend(); fi++)
			{
				AuPoint fvec((*fi)->FaceNormal() * distToMove);

				AuMatrix4x4 tmove;
				tmove.Translate(fvec);
				(*fi)->Transform(tmove);

				AuSegmentArray edges;
				(*fi)->EdgeSegments(edges);

				// Add side fill faces.
				for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
				{
					AuPoint pll;
					AuPoint pul;
					AuPoint pur;
					AuPoint plr;
					if(distToMove > 0)
					{
						pll = AuPoint((*ei).m_fLoc1);
						pul = AuPoint((*ei).m_fLoc1 - fvec);
						pur = AuPoint((*ei).m_fLoc2 - fvec);
						plr = AuPoint((*ei).m_fLoc2);
					}
					else
					{
						pll = AuPoint((*ei).m_fLoc1 - fvec);
						pul = AuPoint((*ei).m_fLoc1);
						pur = AuPoint((*ei).m_fLoc2);
						plr = AuPoint((*ei).m_fLoc2 - fvec);
					}
					AuPoint fx = plr - pll;
					fx.Unit();
					AuPoint fy = pul - pll;
					fy.Unit();
					AuPoint fn = fx.Cross(fy);
					fn.Unit();

					AuScPoint ll(pll, 0,0, fn);
					AuScPoint ul(pul, 0,0, fn);
					AuScPoint ur(pur, 0,0, fn);
					AuScPoint lr(plr, 0,0, fn);
					AuScFaceQuad fq(**fi, ll, ul, ur, lr);
					fillFaces.AddFace(fq);
				}
			}

			resultFaces.append(fillFaces);
			fillFaces.clear();
		}
	}

	// When Ctrl is not held, delete the selected faces. This also clears the selection.
	if(keymods != Qt::ControlModifier)
	{
		// Remove the old faces and add the resulting faces to the current object so it gets done in one undo.
		auApp->DesignerPartReplaceFaces(partId, m_pWnd->DesignerPartObjectSelection(), resultFaces);
		m_pWnd->DesignerSelectNone();
	}
	else
	{
		// Add the resulting faces to the current object.
		auApp->DesignerPartAddFaces(partId, resultFaces);
	}

	Terminate();
	return loc;
}

void DdScPushPull::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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
	AuMatrix4x4 mv = model * ts;
	AuMatrix4x4 mvp = projection * mv;

	// Get a copy of the selected faces.
	AuScFaceList faces;
	pde->Faces(faces, m_pWnd->DesignerPartObjectSelection());

	// Create the result of the operation.
	if(!faces.empty())
	{
		AuScFaceList resultFaces;
		resultFaces.AddFaces(faces);
		AuPoint dragVec = m_pWnd->DesignPlane().UnProject(loc);

		if(m_Together)
		{
			// Every face moves together.
			// Apply constraints in the space of the grid.
			if(m_ConstrainX || m_ConstrainY || m_ConstrainZ)
			{
				// Constraints must be applied in the space of the work plane.
				if(m_ConstrainX)
				{
					dragVec.y = 0;
					dragVec.z = 0;
				}
				else if(m_ConstrainY)
				{
					dragVec.x = 0;
					dragVec.z = 0;
				}
				else if(m_ConstrainZ)
				{
					// We tipped the grid plane for Z drag so the Y axis is aligned with the Z constraint axis. 
					// Treat it like a Y axis constraint.
					dragVec.x = 0;
					dragVec.z = 0;
				}
			}

			dragVec = m_pWnd->DesignPlane().ProjectVector(dragVec);	// dragVec projected back to designer coordinate system.

			// This can be done with a single transformation of the entire list.
			AuMatrix4x4 t;
			t.Translate(dragVec);
			resultFaces.Transform(t);

			// Fill with intermediate faces where there is a resulting edge that does not share another resulting edge.
			AuSegmentArray edges;
			for(AuScFaceList::const_iterator fi = resultFaces.cbegin(); fi != resultFaces.cend(); fi++)
			{
				AuSegmentArray fedges;
				(*fi)->EdgeSegments(fedges);
				edges.Intersect(fedges);
			}

			// Add side fill faces.
			for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
			{
				AuPoint pll((*ei).m_fLoc1);
				AuPoint pul((*ei).m_fLoc1 - dragVec);
				AuPoint pur((*ei).m_fLoc2 - dragVec);
				AuPoint plr((*ei).m_fLoc2);
				AuPoint fx = plr - pll;
				fx.Unit();
				AuPoint fy = pul - pll;
				fy.Unit();
				AuPoint fn = fx.Cross(fy);
				fn.Unit();

				AuScPoint ll(pll, 0,0, fn);
				AuScPoint ul(pul, 0,0, fn);
				AuScPoint ur(pur, 0,0, fn);
				AuScPoint lr(plr, 0,0, fn);
				AuScFaceQuad fq(*resultFaces.first(), ll, ul, ur, lr);
				resultFaces.AddFace(fq);
			}
		}
		else
		{
			// Every face moves individually, according to its facing.
			double distToMove = dragVec.Length();
			if(distToMove != 0)
			{
				AuPoint fpxo = mvp.Project(m_DragPlane.Translation());
				AuPoint fpxl = mvp.Project(loc);
				if(fpxo.y < fpxl.y)
				{
					m_DragNegative = true;
					distToMove = -distToMove;
				}

				AuScFaceList fillFaces;
				for(AuScFaceList::const_iterator fi = resultFaces.cbegin(); fi != resultFaces.cend(); fi++)
				{
					AuPoint fvec((*fi)->FaceNormal() * distToMove);

					AuMatrix4x4 tmove;
					tmove.Translate(fvec);
					(*fi)->Transform(tmove);

					AuSegmentArray edges;
					(*fi)->EdgeSegments(edges);

					// Add side fill faces.
					for(AuSegmentArray::const_iterator ei = edges.cbegin(); ei != edges.cend(); ei++)
					{
						AuPoint pll;
						AuPoint pul;
						AuPoint pur;
						AuPoint plr;
						if(distToMove > 0)
						{
							pll = AuPoint((*ei).m_fLoc1);
							pul = AuPoint((*ei).m_fLoc1 - fvec);
							pur = AuPoint((*ei).m_fLoc2 - fvec);
							plr = AuPoint((*ei).m_fLoc2);
						}
						else
						{
							pll = AuPoint((*ei).m_fLoc1 - fvec);
							pul = AuPoint((*ei).m_fLoc1);
							pur = AuPoint((*ei).m_fLoc2);
							plr = AuPoint((*ei).m_fLoc2 - fvec);
						}
						AuPoint fx = plr - pll;
						fx.Unit();
						AuPoint fy = pul - pll;
						fy.Unit();
						AuPoint fn = fx.Cross(fy);
						fn.Unit();

						AuScPoint ll(pll, 0,0, fn);
						AuScPoint ul(pul, 0,0, fn);
						AuScPoint ur(pur, 0,0, fn);
						AuScPoint lr(plr, 0,0, fn);
						AuScFaceQuad fq(**fi, ll, ul, ur, lr);
						fillFaces.AddFace(fq);
					}
				}

				resultFaces.append(fillFaces);
				fillFaces.clear();
			}
		}

		// Render the result.
		if(g_AuClient.DesignerShowFaces())
			pde->RenderFaces(projection, mv, re, resultFaces, sceneAtom->EmpireId());
		if(g_AuClient.DesignerShowEdges())
			pde->RenderEdges(projection, mv, re, resultFaces);
	}

	// Draw the blip in pixel space.
	DrawBlip(mvp, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}

bool DdScPushPull::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_S:
		m_Together = !m_Together;
		UpdatePrompt();
		break;

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

AuMatrix4x4 DdScPushPull::DesignPlane() const
{
	if(!m_ConstrainZ)
		return m_DragPlane;

	// Dragging in Z requires us to rotate the drag plane.
	AuPoint org = m_DragPlane.Translation();
	AuPoint rot = m_pWnd->DesignPlaneMatrix().GetRotation();
	AuMatrix4x4 dragPlane;
	dragPlane.Translate(org);
	dragPlane.Rotate(rot);
	dragPlane.RotateX(PI/2);
	return dragPlane;
}
