//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtgl.cpp,v 1.5 2022/04/15 16:39:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtgl.h"
//
#include "auapp.h"
#include "auview.h"
#include <QDebug>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QTime>

#define d_FrameRate						30						// Target frame rate, in frames per second.

DtGL::DtGL(AuView* view) : QThread(),
	m_semAlive(1),
	context(0),
	surface(0)
{
	m_View = view;
	m_semAlive.acquire(1);			// I live...
}

void DtGL::run()
{
	AuElapsedTimer startTime;
	//DaPerson* sceneAtom;
	while(Alive())
	{
		//qDebug() << "DtTarget::run running";
		startTime.restart();

		context->makeCurrent(surface);

		/*
		m_View->LockSceneAtom(&sceneAtom);
		m_View->AnimateSceneAtoms(sceneAtom, m_LastAnim, m_LastAnim.elapsed());
		m_View->UnlockSceneAtom();
		*/

		context->doneCurrent();

		m_LastAnim = startTime;

		// Wait for the time needed to try to even out the cycle time.
		qint64 waitMs = 1000/d_FrameRate - startTime.elapsed();
		if(waitMs > 0)
			msleep(waitMs);
	}
}

void DtGL::shutDown()
{
	delete context;

	// schedule this to be deleted only after we're done cleaning up
	surface->deleteLater();

	// Stop event processing, move the thread to GUI and make sure it is deleted.
	exit();
	moveToThread(QGuiApplication::instance()->thread());
}
