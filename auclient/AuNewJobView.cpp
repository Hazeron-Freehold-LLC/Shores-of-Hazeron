//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuNewJobView.cpp,v 1.2 2022/02/19 18:16:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuNewJobView.h"
//
#include <auglobal.h>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QVariant>

AuNewJobView::AuNewJobView(QWidget* parent) : QTreeView(parent)
{
	setEditTriggers(QTreeView::NoEditTriggers);
}

AuNewJobView::~AuNewJobView()
{
}

void AuNewJobView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	if(selected.indexes().count() == 0)
		emit ItemSelected(QModelIndex());
	else
		emit ItemSelected(selected.indexes().first());
}

void AuNewJobView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// Let base class draw the row first.
	QTreeView::drawRow(painter, option, index);
}

void AuNewJobView::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeView::mouseReleaseEvent(event);
	QModelIndex curIdx = currentIndex();
	if(curIdx.isValid())
	{
		//qDebug("mouseReleaseEvent row %d selected", curIdx.row());
		emit ItemSelected(curIdx);
	}
}
