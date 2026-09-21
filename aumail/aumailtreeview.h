//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailtreeview.h,v 1.6 2022/02/19 18:36:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailTreeView_H
#define AuMailTreeView_H

#include "aumaillib.h"
#include <QTreeView>

class AUMAIL_DLL AuMailTreeView : public QTreeView
{
	Q_OBJECT;
public:
	AuMailTreeView(QWidget* parent=0);
	~AuMailTreeView();

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event);

protected slots:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

signals:
	void ItemRightClicked(const QModelIndex& selIndex);
	void SelectionChanged();
};

#endif // AuMailTreeView_H
