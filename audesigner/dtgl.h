//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtgl.h,v 1.5 2022/04/15 20:39:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtGL_H
#define DtGL_H

#include "auelapsedtimer.h"
#include <QImage>
#include <QMutex>
#include <QSize>
#include <QThread>

class AuView;
class QOffscreenSurface;
class QOpenGLContext;
class QOpenGLFramebufferObject;

class DtGL : public QThread
{
    Q_OBJECT
private:
	QSemaphore							m_semAlive;				// Used to know when I'm supposed to die.

public:
    QOpenGLContext*						context;
    QOffscreenSurface*					surface;

private:
	AuElapsedTimer						m_LastAnim;
	AuView*								m_View;

public:
    DtGL(AuView* view);

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	void Kill()
	{
		if(Alive())
			m_semAlive.release(1);
	}

public slots:
    void shutDown();

protected:
	virtual void run();
};

#endif // DtGL_H
