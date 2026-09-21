//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomicstructure.cpp,v 1.6 2022/02/19 18:05:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomicstructure.h"
//
#include "auagentteleport.h"
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "daatom.h"
#include "daatomlist.h"
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QTreeWidget>
#include <QVBoxLayout>

AuAtomicTree::AuAtomicTree(QWidget* parent) : QTreeWidget(parent)
{
	setColumnCount(4);

	QStringList hdr;
	hdr << tr("Name");
	hdr << tr("Id");
	hdr << tr("Location");
	hdr << tr("Description");
	setHeaderLabels(hdr);

	header()->setSectionResizeMode(0, QHeaderView::Stretch);
	header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(3, QHeaderView::Interactive);
}

void AuAtomicTree::mousePressEvent(QMouseEvent* event)
{
	QTreeWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuAtomicTree::mouseReleaseEvent(QMouseEvent* event)
{
	QTreeWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTreeWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAtomicStructure::AuAtomicStructure(const DaAtom& atom, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Atomic Structure"));
	setAuDlgIcon(QIcon(":data/c_Microscope.png"));

	m_Tree = new AuAtomicTree;
	connect(m_Tree, SIGNAL(ItemRightClicked(QTreeWidgetItem*)), SLOT(AtomRightClicked(QTreeWidgetItem*)));

	QTreeWidgetItem* atomItem = new QTreeWidgetItem(m_Tree);
	ListAtomRecursive(atomItem, atom);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Tree);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuAtomicStructure::~AuAtomicStructure()
{
}

void AuAtomicStructure::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuAtomicStructure::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuAtomicStructure/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuAtomicStructure/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuAtomicStructure/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuAtomicStructure/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuAtomicStructure::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuAtomicStructure/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuAtomicStructure/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuAtomicStructure/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuAtomicStructure/SizeY",sz.height());
}

void AuAtomicStructure::ListAtomRecursive(QTreeWidgetItem* atomItem, const DaAtom& atom)
{
	// Make a tree node for the atom.
	atomItem->setIcon(0, QIcon(":data/TabClose.png"));
	atomItem->setText(0, atom.Name());
	atomItem->setData(0, Qt::UserRole, atom.Id().GetID());
	atomItem->setText(1, atom.Id().StringId());
	atomItem->setText(2, QString("(%1, %2, %3) %4")
			.arg(atom.Location().x/FeetPerMeter())
			.arg(atom.Location().y/FeetPerMeter())
			.arg(atom.Location().z/FeetPerMeter())
			.arg(atom.Location().Length()/FeetPerMeter()));
	atomItem->setText(3, atom.Description());

	// Make tree nodes for its subatoms.
	DaAtomList subs;
	atom.ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		QTreeWidgetItem* subItem = new QTreeWidgetItem(atomItem);
		ListAtomRecursive(subItem, **it);
	}
}

void AuAtomicStructure::AtomRightClicked(QTreeWidgetItem* item)
{
	if(!item)
		return;

	// Our player's id and name are in column 0.
	AuId gotoId(item->data(0, Qt::UserRole).toUInt());
	QString gotoName = item->text(0);

	QMenu menu;

	QAction* goToAvatar = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport to %1...").arg(gotoName));
	connect(goToAvatar, SIGNAL(triggered()), SLOT(GoToAvatar()));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;	// Menu canceled.
}

void AuAtomicStructure::GoToAvatar()
{
	QTreeWidgetItem* item = m_Tree->currentItem();
	if(item)
	{
		AuId gotoId(item->data(0, Qt::UserRole).toUInt());
		if(gotoId.IsNull())
			return;
		QMessageBox mb(g_Au.ApplicationName(),
				tr("This action will teleport you to the destination atom.\n\n"
					"After 3 minutes, if the destination atom is not found, the agent teleport is redirected to the capital "
					"city of a randomly selected empire."),
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
		if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
			return;

		AuAgentTeleport::GoToDestination(gotoId, false, false);
	}
}
