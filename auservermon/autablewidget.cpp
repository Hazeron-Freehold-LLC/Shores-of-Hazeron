//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autablewidget.cpp,v 1.3 2022/02/19 18:40:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autablewidget.h"
//
#include <QMouseEvent>
#include <QSettings>

AuTableWidget::AuTableWidget(QWidget* parent) : QTableWidget(parent)
{
	m_LastButtonDown = Qt::NoButton;
	m_LastButtonUp = Qt::NoButton;
}

AuTableWidget::~AuTableWidget()
{
}

void AuTableWidget::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);

	m_LastButtonDown = event->button();
}

void AuTableWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	m_LastButtonUp = event->button();
}

void AuTableWidget::SaveColumnPositions(const QString& key, QSettings& settings) const
{
	settings.beginWriteArray(key);

	for(int column=0; column<columnCount(); column++)
	{
		settings.setArrayIndex(column);
		settings.setValue("ColumnWidth", columnWidth(column));
	}

	settings.endArray();
}

void AuTableWidget::RestoreColumnPositions(const QString& key, QSettings& settings)
{
	settings.beginReadArray(key);

	for(int column=0; column<columnCount(); column++)
	{
		settings.setArrayIndex(column);
		int curColWid = columnWidth(column);
		setColumnWidth(column, settings.value("ColumnWidth", curColWid).toInt());
	}

	settings.endArray();
}
