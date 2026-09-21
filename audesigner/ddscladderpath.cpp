//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddscladderpath.cpp,v 1.7 2022/02/19 18:26:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddscladderpath.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auscparts.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScLadderPath::DdScLadderPath(AuDesignerView* view) : DdCommand(view)
{
	m_Mode = 0;
}

DdScLadderPath::~DdScLadderPath()
{

}

QString DdScLadderPath::WikiTopic() const
{
	return "Designer_Menu#Ladder_Path";
}

void DdScLadderPath::Start()
{
	DdCommand::Start();
}

void DdScLadderPath::UpdatePrompt() const
{
	QString prompt;
	if(m_Mode == 0)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Bottom of ladder path segment. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 1)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Top of ladder path segment. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	else if(m_Mode == 2)
	{
		prompt = QObject::tr("<html>"
				"<span style=\"color: %2;\">Facing direction of climber. "
				"</span>"
				"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
				"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
				"</html>")
			.arg('=')
			.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	}
	m_pWnd->CommandPromptChanged(prompt);
}

AuPoint DdScLadderPath::CoordinateEntered(const AuPoint& loc, bool fromKeyboard, int keymods)
{
	Q_UNUSED(fromKeyboard);
	Q_UNUSED(keymods);
	AuPoint ploc(loc);
	if(m_Mode == 0)
	{
		m_Bottom = loc;
		m_Mode = 1;
		UpdatePrompt();
	}
	else if(m_Mode == 1)
	{
		m_Top = loc;
		m_Mode = 2;
		UpdatePrompt();
	}
	else if(m_Mode == 2)
	{
		if(MakePath(loc))
			Terminate();
	}
	return ploc;
}

// Set appropriate device cursor.
void DdScLadderPath::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* /*sceneAtom*/, const QPoint& /*pxCursor*/)
{
	m_pWnd->setCursor(QCursor(Qt::CrossCursor));
}

bool DdScLadderPath::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;

	return false;
}

// Render "cursor" objects in the scene.
void DdScLadderPath::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& model, DaPerson* sceneAtom,
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

	if(m_Mode == 1)
	{
		// Make a VAO of the vertex locations.
		AuPointArray pts;
		pts.append(m_Bottom);
		pts.append(loc);
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

		AuPoint vec = loc - m_Bottom;
		vec = m_pWnd->DesignPlane().UnProjectVector(vec);
		QString sloc = g_AuClient.FormatCoordinate(vec);

		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		re.RenderText(projection, ts, loc, sloc, g_Au.FontDesigner(), QColor(255,255,255), m_pWnd->size().height());
		re.glDisable(GL_BLEND);
	}
	else if(m_Mode == 2)
	{
		// Calculate facing loc.
		AuPoint vx(m_Top - m_Bottom);
		vx.Unit();
		AuPoint vy(loc - m_Bottom);
		vy.Unit();
		AuPoint vz(vx.Cross(vy));
		vz.Unit();
		vy = vz.Cross(vx);
		vy.Unit();

		// Make a VAO of the vertex locations.
		AuPointArray pts;
		pts.append(m_Bottom);
		pts.append(m_Top);

		pts.append(m_Top);
		pts.append(m_Top + vy * FeetPerMeter());

		pts.append(m_Bottom);
		pts.append(m_Bottom + vy * FeetPerMeter());

		AuGLVAO vao;
		vao.CreateForLines(re, pts, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	re.glEnable(GL_DEPTH_TEST);

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	AuMatrix4x4 mvp = projection * ts;
	AuMatrix4x4 designPlane = pde->GetDesignPlaneTransform(sceneAtom->Id());
	DrawBlip(mvp, loc, designPlane, re, glw, snappedToExistingShape);
}

bool DdScLadderPath::MakePath(const AuPoint& loc)
{
	if(m_Top.FuzzyEquals(loc))
		return false;

	// Calculate facing loc.
	AuPoint vx(m_Top - m_Bottom);
	vx.Unit();
	AuPoint vy(loc - m_Bottom);
	vy.Unit();
	AuPoint vz(vx.Cross(vy));
	vz.Unit();
	vy = vz.Cross(vx);
	vy.Unit();

	AuScPathLadder path;
	path.MakePath(m_Bottom, m_Top);
	path.SetFacing(vy);
	if(!path.IsValid())
		return false;

	path.SetArchitect(m_pWnd->SceneAtomId(), m_pWnd->SceneAtomName());
	auApp->DesignerPartAdd(path);
	return true;
}
