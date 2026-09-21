//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audockwidget.cpp,v 1.9 2022/02/19 18:33:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audockwidget.h"
//
#include "audocktitle.h"
#include "auglobal.h"
#include <QEvent>
#include <QIcon>

AuDockWidget::AuDockWidget(QWidget* parent) : QDockWidget(parent)
{
}

AuDockWidget::AuDockWidget(const QIcon& titleIcon, const QString& titleText, const QString& windowTitle, QWidget* parent) :
	QDockWidget(parent)
{
	m_OriginalTitle = titleText;

	setObjectName(windowTitle+"Dock");
	setWindowTitle(windowTitle);

	AuDockTitle* title = new AuDockTitle(this);
	title->SetText(titleText);
	title->setTitleIcon(titleIcon);
	setTitleBarWidget(title);

	setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
}

AuDockWidget::AuDockWidget(const QString& titleIcon, const QString& titleText, const QString& windowTitle, QWidget* parent) :
	QDockWidget(parent)
{
	m_OriginalTitle = titleText;

	setObjectName(windowTitle+"Dock");
	setWindowTitle(windowTitle);

	AuDockTitle* title = new AuDockTitle(this);
	title->SetText(titleText);
	title->setTitleIcon(QIcon(titleIcon));
	setTitleBarWidget(title);

	setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
}

AuDockWidget::~AuDockWidget()
{
}

void AuDockWidget::changeEvent(QEvent* event)
{
	QDockWidget::changeEvent(event);
	if(event->type() == QEvent::ActivationChange)
	{
		// Configure transparency state of this widget.
		if(isFloating())
		{
			float opacity = (float)g_Au.WindowOpacity(isActiveWindow())/100.f;
			setWindowOpacity(opacity);
		}
		else
		{
			// This appears to have no effect when docked but here goes anyway.
			setWindowOpacity(1);
		}
	}
}

void AuDockWidget::SetTitle(const QString& text)
{
	QWidget* titleBar = titleBarWidget();
	if(titleBar)
	{
		AuDockTitle* dockTitle = qobject_cast<AuDockTitle*>(titleBar);
		if(dockTitle)
			dockTitle->SetText(text);
		else
			titleBar->setWindowTitle(text);
	}
}

void AuDockWidget::SetHelpUrl(const QUrl& url)
{
	AuDockTitle* titleBar = dynamic_cast<AuDockTitle*>(titleBarWidget());
	if(titleBar)
		titleBar->SetHelpUrl(url);
}

void AuDockWidget::SetWikiTopic(const QString& topic)
{
	AuDockTitle* titleBar = dynamic_cast<AuDockTitle*>(titleBarWidget());
	if(titleBar)
		titleBar->SetWikiTopic(topic);
}
