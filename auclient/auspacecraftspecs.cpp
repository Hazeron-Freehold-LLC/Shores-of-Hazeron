//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftspecs.cpp,v 1.4 2022/05/04 21:12:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspacecraftspecs.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

void AuSpacecraftSpecsDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleSpacecraftSpecsUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSpacecraftSpecs::AuSpacecraftSpecs(QWidget* parent) : QWidget(parent)
{
	m_Timer = 0;

	m_Analysis = new QTextBrowser(this);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Analysis);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuSpacecraftSpecs::~AuSpacecraftSpecs()
{
}

void AuSpacecraftSpecs::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(3000);
	TimerUpdate();
}

void AuSpacecraftSpecs::TimerUpdate()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	AuView* view = auApp->Frame()->View();
	QString report = view->DesignAnalysis();
	if(m_CurrentReport != report)
	{
		m_CurrentReport = report;
		m_Analysis->setHtml(report);
	}
}

QSize AuSpacecraftSpecs::sizeHint() const
{
	return QSize(500,600);
}

