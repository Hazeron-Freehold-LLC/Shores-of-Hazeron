//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupatentstree.h,v 1.1 2022/05/05 22:38:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPatentsTree_H
#define AuPatentsTree_H

#include "audialog.h"
#include "dbcommodity.h"
#include <QTreeWidget>

class DgData;

class AuPatentsTree : public QTreeWidget
{
	Q_OBJECT;
	QTreeWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuPatentsTree(QWidget* parent=0);
	virtual ~AuPatentsTree()
	{
	}

	void ListPatents();

signals:
	void ItemRightClicked(QTreeWidgetItem*);

protected:
	void ListDependencies(QTreeWidgetItem* parent, const DgData& gd, COMMODITYID patentCID);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuPatentsTreeDlg : public AuDialog
{
	Q_OBJECT;
public:
	AuPatentsTreeDlg(QWidget* parent=0);
	virtual ~AuPatentsTreeDlg();

	virtual QSize sizeHint() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListPatents(AuPatentsTree* ptree);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuPatentsTree_H
