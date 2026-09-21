//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomicstructure.h,v 1.6 2022/02/19 17:49:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomicStructure_H
#define AuAtomicStructure_H

#include "auclientlib.h"
#include <audialog.h>
#include <QTreeWidget>

class DaAtom;
class QTreeWidgetItem;

class AUCLIENT_DLL AuAtomicTree : public QTreeWidget
{
	Q_OBJECT;
	QTreeWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuAtomicTree(QWidget* parent=0);
	virtual ~AuAtomicTree()
	{
	}

signals:
	void ItemRightClicked(QTreeWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuAtomicStructure : public AuDialog
{
	Q_OBJECT;
	// Widgets
	AuAtomicTree*						m_Tree;

public:
	AuAtomicStructure(const DaAtom& atom, QWidget* parent=0);
	virtual ~AuAtomicStructure();

public slots:
	void AtomRightClicked(QTreeWidgetItem* item);
	void GoToAvatar();
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	void ListAtomRecursive(QTreeWidgetItem* atomItem, const DaAtom& atom);
};

#endif // AuAtomicStructure_H
