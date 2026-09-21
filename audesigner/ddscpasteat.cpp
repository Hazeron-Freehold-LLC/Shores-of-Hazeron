//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscpasteat.cpp,v 1.9 2022/02/19 18:27:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscpasteat.h"
//
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMessageBox>
#include <QMouseEvent>

DdScPasteAt::DdScPasteAt(AuDesignerView* view) : DdCommand(view)
{
	m_PastedParts = new AuScParts;

	// Get pasted parts from the clipboard.
	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	if(mimeData)
	{
		DaPerson* sceneAtom;
		view->LockSceneAtom(&sceneAtom);
		if(sceneAtom)
		{
			const DaDesigner* pde = sceneAtom->FindParentDesignerInEnv();
			if(pde)
				pde->ReadMIME(sceneAtom->Id(), sceneAtom->Name(), m_pWnd->DesignerObjectMode(), *mimeData, *m_PastedParts);
		}
		view->UnlockSceneAtom();
		m_Rot.Rotate(m_pWnd->DesignPlane().GetRotation());		// Initial rotation matches the design plane.
	}
}

DdScPasteAt::~DdScPasteAt()
{
	delete m_PastedParts;
}

QString DdScPasteAt::WikiTopic() const
{
	return "Designer_Menu#Paste_At";
}

bool DdScPasteAt::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	default:
		return false;

	case Qt::Key_G:
		// Align to grid.
		m_Rot.LoadIdentity();
		m_Rot.Rotate(m_pWnd->DesignPlane().GetRotation());
		break;

	case Qt::Key_Up:
		// Rotate around X
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateX(DEG2RADf);
		else
			m_Rot.RotateX(DEG2RADf*10.f);
		break;
	case Qt::Key_Down:
		// Rotate around X
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateX(-DEG2RADf);
		else
			m_Rot.RotateX(-DEG2RADf*10.f);
		break;

	case Qt::Key_PageUp:
		// Rotate around Y
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateY(-DEG2RADf);
		else
			m_Rot.RotateY(-DEG2RADf*10.f);
		break;
	case Qt::Key_PageDown:
		// Rotate around Y
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateY(DEG2RADf);
		else
			m_Rot.RotateY(DEG2RADf*10.f);
		break;

	case Qt::Key_Right:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateZ(-DEG2RADf);
		else
			m_Rot.RotateZ(-DEG2RADf*10.f);
		break;
	case Qt::Key_Left:
		// Rotate around Z
		if(e->modifiers() & Qt::ShiftModifier)
			m_Rot.RotateZ(DEG2RADf);
		else
			m_Rot.RotateZ(DEG2RADf*10.f);
		break;
	}
	return true;
}

void DdScPasteAt::UpdatePrompt() const
{
	QString prompt;
	prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Paste clipboard contents. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">G%1Align to Grid, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">[Shift] Arrow/Pg%1Rotate, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScPasteAt::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	if(!m_PastedParts->isEmpty())
	{
		// Create a matrix for positioning the pasted shapes.
		AuMatrix4x4 t;
		t.Translate(loc);
		t *= m_Rot;

		// Transform the pasted shapes to the position specified.
		AuScParts pastedParts(*m_PastedParts);
		pastedParts.Transform(t);

		// Add the shapes to the designer, transformed to the position specified.
		if(m_pWnd->DesignerObjectMode())
			auApp->DesignerAddPartObjects(m_pWnd->DesignerObjectModeTargetPart(), pastedParts);
		else
			auApp->DesignerAddParts(pastedParts);

		if(m_pWnd->DesignerObjectMode() && m_pWnd->HasSealedPartsSelected())
			auDesignerApp->DesignerFrame()->DesignerSetObjectMode(false);
	}
	else
		Terminate();

	return loc;
}

void DdScPasteAt::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	AuMatrix4x4 t;
	t.Translate(loc);
	t *= m_Rot;

	// Line shader is nice and bright, no textures or shading.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * ts * t);
	/*
	re.glDisable(GL_LINE_SMOOTH);
	re.glDisable(GL_DEPTH_TEST);

	// Draw axis lines.
	double axislen = 10 * g_AuClient.DesignerUnitScale();	// g_AuClient.DesignPlaneRotatorAxisLengthFt();

	re.GLSL().LineShader().SetColor(1,0,0,1);
	re.glBegin(GL_LINES);
		re.glVertex3d(0, 0, 0);
		re.glVertex3d(axislen, 0, 0);
	re.glEnd();

	re.GLSL().LineShader().SetColor(0,1,0,1);
	re.glBegin(GL_LINES);
		re.glVertex3d(0, 0, 0);
		re.glVertex3d(0, axislen, 0);
	re.glEnd();

	re.GLSL().LineShader().SetColor(0,0,1,1);
	re.glBegin(GL_LINES);
		re.glVertex3d(0, 0, 0);
		re.glVertex3d(0, 0, axislen);
	re.glEnd();

	re.glEnable(GL_DEPTH_TEST);
	re.glEnable(GL_LINE_SMOOTH);
	*/

	// Render wireframe elements.
	re.GLSL().LineShader().SetColor(1,1,0,1);

	AuPointArray edges;
	for(AuScParts::const_iterator pi = m_PastedParts->constBegin(); pi != m_PastedParts->constEnd(); pi++)
		(*pi)->EdgeSegments(edges);

	if(!edges.empty())
	{
		// Make temporary VAO of the edge segments.
		AuGLVAO vao;
		vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	// Draw the blip in pixel space.
	AuMatrix4x4 mvp = projection * ts;
	DrawBlip(mvp, loc, m_pWnd->DesignPlane(), re, glw, snappedToExistingShape);
}
