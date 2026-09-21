//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailtreeview.cpp,v 1.8 2022/02/19 18:36:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailtreeview.h"
//
#include "auglobal.h"

AuMailTreeView::AuMailTreeView(QWidget* parent) : QTreeView(parent)
{
	setEditTriggers(QTreeView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
}

AuMailTreeView::~AuMailTreeView()
{
}

void AuMailTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);
	emit SelectionChanged();
}

void AuMailTreeView::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeView::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		// Get the selected item.
		QItemSelectionModel* ism = selectionModel();
		if(ism)
			emit ItemRightClicked(ism->currentIndex());
	}
}
