//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailcontacts.cpp,v 1.8 2022/02/19 18:36:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailcontacts.h"
//
#include <auglobal.h>
#include <dgdata.h>
#include <aulogincomm.h>
#include "aucontactnew.h"
#include <QGridLayout>
#include <QIcon>
#include <QToolButton>
#include <QHeaderView>
#include <QListWidget>

AuMailContacts::AuMailContacts(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Select Recipient(s)"));
	setAuDlgIcon(QIcon(":data/ContactBuddies.png"));

	QToolButton* pbok = new QToolButton(this);
	pbok->setIcon(QIcon(":data/SendMail.png"));
	pbok->setToolTip(tr("Add the selected contacts as recipients of the message"));
	pbok->setAutoRaise(true);
	connect(pbok, SIGNAL(clicked()), SLOT(Recipients()));

//	QTreeView* contacts = new QTreeView;
//////////////////////////////////////////////////////////////////////////

	QListWidget* lwcontacts = new QListWidget(this);
	QStringList headerLabels;
	headerLabels << "Name";
	lwcontacts->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwcontacts->setObjectName("lwcontacts");
	lwcontacts->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwcontacts->setSelectionMode(QAbstractItemView::MultiSelection);
	lwcontacts->setSortingEnabled(true);
	lwcontacts->setToolTip(tr("List of players who are your contacts."));

	connect(lwcontacts, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(EmitContactDoubleClicked(QListWidgetItem*)));

	//////////////////////////////////////////////////////////////////////////

	QGridLayout* lay = new QGridLayout();
	lay->addWidget(pbok, 0,0);
	lay->addWidget(lwcontacts, 1,0);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	const DgData& gd = g_Au.LockDataForRead();
	QString acctName = gd.GetAcctName();
	QByteArray acctPassword = gd.GetAcctPassCrypt();
	g_Au.UnlockData();

	// Solo mode already has all contacts in DgData.
	if(!g_Au.IsSceneSolo())
	{
		AuLoginComm login;
		login.RequestContacts(acctName, acctPassword);
	}
	ShowContactList();
}

void AuMailContacts::ShowContactList()
{
	QListWidget* lwcontacts = findChild<QListWidget*>("lwcontacts");
	if(!lwcontacts)
		return;

	// Refresh the contents of the table widget.
	lwcontacts->clear();
	lwcontacts->setSortingEnabled(false);

	const DgData& gd = g_Au.LockDataForRead();
	for(QList<AuContact>::const_iterator it = gd.GetContacts().begin(); it != gd.GetContacts().end(); it++)
	{
		if(!(*it).m_bIgnore)
		{
			QString contact = QString("%1 (%2)").arg((*it).m_name).arg((*it).m_id.StringId());
			QListWidgetItem* lwi = new QListWidgetItem(contact, lwcontacts);
			lwi->setData(Qt::UserRole, (*it).m_id.GetID());
			lwi->setData(Qt::UserRole+1, (*it).m_name);
		}
	}
	g_Au.UnlockData();

	lwcontacts->setSortingEnabled(true);
	lwcontacts->sortItems();
}

//void AuMailContacts::setVisible(bool visible)
//{
//	QWidget::setVisible(visible);
//
//	// Update data in controls when we become visible and the data is not valid.
//	// This happens the first time we become visible.
//	if(visible)
//		ShowContactList();
//}
//
void AuMailContacts::EmitContactDoubleClicked(QListWidgetItem* item)
{
	if(item)
	{
		AuContact contact;
		contact.m_id = AuId(item->data(Qt::UserRole).toUInt());
		contact.m_name = item->data(Qt::UserRole+1).toString();
		contact.m_bIgnore = false;
		emit ContactDoubleClicked(contact);
	}
}

void AuMailContacts::Recipients()
{
	// add all the selected buddies to the recipients list
	QListWidget* lwcontacts = findChild<QListWidget*>("lwcontacts");
	if(!lwcontacts)
		return;

	QList<QListWidgetItem *> selitems = lwcontacts->selectedItems();
	if(selitems.isEmpty())
		return;

	for(QList<QListWidgetItem *>::iterator it = selitems.begin(); it != selitems.end(); it++)
	{
		AuContact contact;
		contact.m_id = AuId((*it)->data(Qt::UserRole).toUInt());
		contact.m_name = (*it)->data(Qt::UserRole+1).toString();
		contact.m_bIgnore = false;
		m_Contacts.append(contact);
	}

	accept();
}
