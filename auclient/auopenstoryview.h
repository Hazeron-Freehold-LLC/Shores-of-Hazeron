//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auopenstoryview.h,v 1.6 2022/02/19 17:59:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUOPENSTORYVIEW_H
#define AUOPENSTORYVIEW_H

#include "auclientlib.h"
#include <QTreeView>

class QMouseEvent;

class AUCLIENT_DLL AuOpenStoryView : public QTreeView
{
	Q_OBJECT;
public:
	AuOpenStoryView(QWidget* parent=0);
	~AuOpenStoryView();

signals:
	void ItemSelected(const QModelIndex& selIndex);

protected slots:
	virtual void selectionChanged(const QItemSelection& selected,
			const QItemSelection& deselected);

protected:
	virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& option,
			const QModelIndex& index) const;
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AUOPENSTORYVIEW_H
