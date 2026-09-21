//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsctransporterpadlocs.cpp,v 1.7 2022/02/19 18:27:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsctransporterpadlocs.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScTransporterPadLocs::DdScTransporterPadLocs(AuDesignerView* view) : DdCommand(view)
{
}

DdScTransporterPadLocs::~DdScTransporterPadLocs()
{

}

QString DdScTransporterPadLocs::WikiTopic() const
{
	return "Designer_Menu#Transporter_Station_Pad_Locations";
}

void DdScTransporterPadLocs::Start()
{
	DdCommand::Start();
}

void DdScTransporterPadLocs::UpdatePrompt() const
{
	QString prompt;
	if(m_PadLocs.empty())
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">First personnel transporter pad location. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	else
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Personnel transporter pad location %3. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">B%1Back, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">C%1Restart, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">D%1Done, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name())
			.arg(m_PadLocs.size()+1);
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScTransporterPadLocs::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	m_PadLocs.append(loc);
	UpdatePrompt();
	return loc;
}

// Set appropriate device cursor.
void DdScTransporterPadLocs::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScTransporterPadLocs::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	switch(e->key())
	{
	case Qt::Key_B:	// Back
		if(!m_PadLocs.empty())
		{
			m_PadLocs.pop_back();
			UpdatePrompt();
			return true;
		}
		break;

	case Qt::Key_C:	// Cancel
		m_PadLocs.clear();
		UpdatePrompt();
		return true;

	case Qt::Key_D:	// Done
		if(!m_PadLocs.empty())
		{
			DoIt();
			Terminate();
			return true;
		}
		break;

	default:
		break;
	}
	return false;
}

// Render "cursor" objects in the scene.
void DdScTransporterPadLocs::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(!m_PadLocs.empty())
	{
		AuPointArray verts;
		verts.MakeCircle(AuPoint(0,0), AuPoint(.5,0), 6);
		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			re.glDisable(GL_DEPTH_TEST);
			re.glEnable(GL_LINE_SMOOTH);
			re.GLSL().LineShader().Select();
			re.GLSL().LineShader().SetColor(1,1,1,1);

			vao.SelectGL();
			for(AuPointArray::const_iterator pi = m_PadLocs.cbegin(); pi != m_PadLocs.cend(); pi++)
			{
				AuMatrix4x4 tpad(pmv);
				tpad.Translate(*pi);
				re.GLSL().LineShader().SetProjectionModelView(tpad);
				vao.DrawGL(GL_LINE_STRIP);
			}
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

bool DdScTransporterPadLocs::DoIt()
{
	auApp->DesignerPartsTransporterPadLocs(m_pWnd->DesignerPartSelection(), m_PadLocs);
	return true;
}
