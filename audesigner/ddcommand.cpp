//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddcommand.cpp,v 1.8 2022/02/19 18:26:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddcommand.h"
//
#include "auclientglobal.h"
#include "audesignerview.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include <QCursor>

DdCommand::DdCommand(AuDesignerView* view)
{
	m_pWnd = view;
}

DdCommand::~DdCommand()
{
	m_pWnd = 0;
}

// Set appropriate device cursor. Default sets Cross cursor if pSceneAtom != 0, otherwise sets Arrow cursor.
void DdCommand::SetCursor(const AuMatrix4x4& /*tCamera*/, DaPerson* pSceneAtom, const QPoint& /*pxCursor*/)
{
	if(m_pWnd)
	{
		if(pSceneAtom)
			m_pWnd->setCursor(QCursor(Qt::CrossCursor));
		else
			m_pWnd->setCursor(QCursor(Qt::ArrowCursor));
	}
}

void DdCommand::OnButtonDown(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		AuPoint loc = m_pWnd->FromPixelToDesignPlane(e->pos(), true, 0);
		m_pWnd->CoordinateEntered(loc, false, e->modifiers());	// Calls my virtual CoordinateEntered function back.
	}
}

// Draw an X blip.
void DdCommand::DrawBlip(const AuMatrix4x4& mvp, const AuPoint& loc, const AuMatrix4x4& designPlane, AuGLEffects& re,
		QOpenGLWidget* glw, bool snappedToExistingShape)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuPoint refLoc = designPlane.UnProject(loc);
	pFrame->DesignerBlipCoordinate(refLoc);

	// Do nothing if the window is smashed.
	if(glw->height() <= 0 || glw->width() <= 0)
		return;

	// Calculate the location of 'loc' on the screen.
	AuPoint screenLoc = mvp.Project(loc);

	// Calculate pixel size of blip, corrected for the window's aspect ratio.
	double blipSizeY = 1./(double)glw->height() * 10.;
	double blipSizeX = blipSizeY * AuGLEffects::WindowAspectRatio(glw->rect());

	// Compile a list of vertex locations for the x blip.
	AuPointArray verts;
	verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y-blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y+blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y-blipSizeY, 0));
	verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y+blipSizeY, 0));
	if(snappedToExistingShape)
	{
		verts.append(AuPoint(screenLoc.x+blipSizeX, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x+blipSizeX*3, screenLoc.y, 0));

		verts.append(AuPoint(screenLoc.x-blipSizeX, screenLoc.y, 0));
		verts.append(AuPoint(screenLoc.x-blipSizeX*3, screenLoc.y, 0));

		verts.append(AuPoint(screenLoc.x, screenLoc.y+blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y+blipSizeY*3, 0));

		verts.append(AuPoint(screenLoc.x, screenLoc.y-blipSizeY, 0));
		verts.append(AuPoint(screenLoc.x, screenLoc.y-blipSizeY*3, 0));
	}

	// Make a temporary VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, verts, false, GL_STREAM_DRAW);

	// Configure for drawing crispy little lines.
	re.glDisable(GL_DEPTH_TEST);

	// Line shader is nice and bright, no textures or shading.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(AuMatrix4x4());
	re.GLSL().LineShader().SetColor(.5,1,1,1);

	// Display the X cursor VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_LINES);
	vao.DeselectGL();
	vao.DeleteGL();

	// Restore rendering states.
	re.GLSL().LineShader().Deselect();

	re.glEnable(GL_DEPTH_TEST);
}

bool DdCommand::OnKeyDown(QKeyEvent* e)
{
	// The coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(pFrame->DesignerKeyReceived(e))
		return true;
	if(e->key() == Qt::Key_Question)
		return ShowWiki();
	return false;
}

AuMatrix4x4 DdCommand::DesignPlane() const
{
	return m_pWnd->DesignPlaneMatrix();
}

void DdCommand::Terminate()
{
	m_pWnd->EndCommand(RunLevel());
}

bool DdCommand::ShowWiki()
{
	QString topic(WikiTopic());
	if(topic.isEmpty())
		return false;
	auApp->Frame()->ShowWikiTopic(topic);
	return true;
}

void DdCommand::Start()
{
	UpdatePrompt();
}

void DdCommand::RenderCursor(const AuMatrix4x4& projection, const AuMatrix4x4& camera, DaPerson* sceneAtom,
		const QPoint& pxCursor, AuAtomRenderInfo& ari, bool snappedToExistingShape)
{
	RenderCursor(projection, camera, sceneAtom, pxCursor, *ari.re, ari.glw, snappedToExistingShape);
}
