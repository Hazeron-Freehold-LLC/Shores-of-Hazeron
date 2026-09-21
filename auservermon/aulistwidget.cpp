//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulistwidget.cpp,v 1.3 2022/02/19 18:40:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulistwidget.h"
//
#include <QMouseEvent>

AuListWidget::AuListWidget(QWidget* parent) : QListWidget(parent)
{
	m_LastButtonDown = Qt::NoButton;
	m_LastButtonUp = Qt::NoButton;
}

AuListWidget::~AuListWidget()
{
}

void AuListWidget::mousePressEvent(QMouseEvent* event)
{
	m_LastButtonDown = event->button();

	QListWidget::mousePressEvent(event);
}

void AuListWidget::mouseReleaseEvent(QMouseEvent* event)
{
	m_LastButtonUp = event->button();

	QListWidget::mouseReleaseEvent(event);

}
