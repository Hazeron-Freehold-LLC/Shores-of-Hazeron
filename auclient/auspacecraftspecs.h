//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftspecs.h,v 1.5 2022/05/04 21:12:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftSpecs_H
#define AuSpacecraftSpecs_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QWidget>

class QTextBrowser;

class AUCLIENT_DLL AuSpacecraftSpecs : public QWidget
{
	Q_OBJECT;
	QString								m_CurrentReport;
	QTimer*								m_Timer;

	// Widgets
	QTextBrowser*						m_Analysis;

public:
	AuSpacecraftSpecs(QWidget* parent);
	virtual ~AuSpacecraftSpecs();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void TimerUpdate();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftSpecsDock : public AuDockWidget
{
public:
	AuSpacecraftSpecs*					m_Content;

public:
	AuSpacecraftSpecsDock(QWidget* parent) : AuDockWidget(":data/c_ScDesignDrawing.png",
			tr("<b>Spacecraft Specifications</b>"), tr("Specifications"), parent)
	{
		m_Content = new AuSpacecraftSpecs(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftSpecsAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftSpecsAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftSpecs_H
