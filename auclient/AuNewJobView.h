//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuNewJobView.h,v 1.6 2022/02/19 17:59:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuNewJobView_H
#define AuNewJobView_H

#include "auclientlib.h"
#include <QTreeView>

class QMouseEvent;

class AUCLIENT_DLL AuNewJobView : public QTreeView
{
	Q_OBJECT;
public:
	AuNewJobView(QWidget* parent=0);
	~AuNewJobView();

signals:
	void ItemSelected(const QModelIndex& selIndex);

protected slots:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
	virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AuNewJobView_H
