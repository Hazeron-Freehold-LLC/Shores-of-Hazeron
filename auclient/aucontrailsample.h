//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontrailsample.h,v 1.6 2022/02/19 17:55:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuContrailSample_H
#define AuContrailSample_H

#include "auclientlib.h"
#include <augleffects.h>
#include <QOpenGLWidget>

class PsEngineTrail;

class AUCLIENT_DLL AuContrailSample : public QOpenGLWidget
{
	Q_OBJECT;
private:
	AuGLEffects			m_RE;
	QTimer*				m_Timer;			// Scene frame timer.
	PsEngineTrail*		m_Contrail;
	QColor				m_ContrailColor;

public:
	AuContrailSample(QWidget* parent=0);
	virtual ~AuContrailSample();

public slots:
	void SetContrailColor(quint8 hue, quint8 sat);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void showEvent(QShowEvent* event);

protected slots:
	void AnimateScene();
};

#endif // AuContrailSample_H
