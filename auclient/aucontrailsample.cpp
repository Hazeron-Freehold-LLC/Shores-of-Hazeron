//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontrailsample.cpp,v 1.4 2022/02/19 18:10:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucontrailsample.h"
//
#include <psenginetrail.h>
#include <QTimer>
#include <auglobal.h>	//DHH debugs only

AuContrailSample::AuContrailSample(QWidget* parent) : QOpenGLWidget(parent)
{
	m_Contrail = 0;
	m_ContrailColor.setHsv(240, 127, 255);	// Default blue
	m_Timer = 0;
}

AuContrailSample::~AuContrailSample()
{
	if(m_Contrail)
	{
		delete m_Contrail;
		m_Contrail = 0;
	}
}

void AuContrailSample::initializeGL()
{
	// Configure our rendering effects object.
	m_RE.InitializeGL(g_Au.MaxLights());

	// Configure GL settings here that NEVER change.
	m_RE.glClearColor(0, 0, 0, 1);	// Let OpenGL clear to black

	// Configure for solid rendered display.
	m_RE.glFrontFace(GL_CW);			// Front faces are those drawn clockwise.
	m_RE.glCullFace(GL_BACK);		// Cull back-facing faces.
	m_RE.glEnable(GL_CULL_FACE);		// Enable face culling.
	m_RE.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//DHH glsl m_RE.glShadeModel(GL_SMOOTH);

	// Enable depth calculations
	m_RE.glEnable(GL_DEPTH_TEST);

	GLuint texIdStreak = m_RE.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
	GLuint texIdGlow = m_RE.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

	// Create the engine trail emitter.
	m_Contrail = new PsEngineTrail;
	m_Contrail->SetMaxParticles(35);
	m_Contrail->SetLifeCycle(1);
	m_Contrail->SetReleaseInterval(0);
	m_Contrail->SetColor(m_ContrailColor);
	m_Contrail->SetSegmentScale(1.f/100.f);
	m_Contrail->SetSize(60);
	m_Contrail->SetTextureID(texIdStreak);
	m_Contrail->SetTextureID2(texIdGlow);
	m_Contrail->SetVelocity(AuPoint(-10,0,0));
}

void AuContrailSample::showEvent(QShowEvent* event)
{
	QOpenGLWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(AnimateScene()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(1000/15);	// 15fps should be plenty
}

void AuContrailSample::AnimateScene()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	if(m_Contrail)
		m_Contrail->Update();

	// Repaint the display.
	update();
}

void AuContrailSample::paintGL()
{
	// Clear the color and depth buffers. (Clears the background.)
	m_RE.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup View Projection
	// Apply perspective transformation that accounts for aspect ratio of screen.
	AuMatrix4x4 projection;
	if(rect().width() > 0)
		projection.Perspective(45.0*DEG2RAD, (double)rect().height()/(double)rect().width(), 2, 1000000);
	else
		projection.Perspective(45.0*DEG2RAD, 1, 2, 1000000);

	// Setup model projection into eye space.
	AuMatrix4x4 model;
	model.Translate(4, 0, -5);			// Move 5 units out in front of the eye plane, past the near clip disance of 2.

	// Render Scene
	m_RE.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	m_RE.glEnable(GL_BLEND);
	m_RE.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

	if(m_Contrail)
	{
		m_Contrail->SetColor(m_ContrailColor);
		m_Contrail->Render(projection, model, m_RE);
	}

	m_RE.glDisable(GL_BLEND);
	m_RE.glDepthMask(GL_TRUE);
}

void AuContrailSample::SetContrailColor(quint8 hue, quint8 sat)
{
	m_ContrailColor.setHsv(hue * 360 / 255, sat, 255);
}

