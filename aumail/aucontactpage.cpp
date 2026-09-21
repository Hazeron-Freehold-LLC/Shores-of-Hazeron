//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontactpage.cpp,v 1.21 2022/04/07 22:16:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucontactpage.h"
//
#include "aucontact.h"
#include "aucontactnew.h"
#include "auglobal.h"
#include "aulogincomm.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QIcon>
#include <QToolButton>
#include <QHeaderView>
#include <QListWidget>
#include <QMessageBox>

AuContactPage::AuContactPage(const AuId& idPerson, bool isBuddyList, const QString& objectName, QWidget* parent) :
	QWidget(parent)
{
	m_idPerson = idPerson;
	m_isBuddyList = isBuddyList;

	setObjectName(objectName);

	QToolButton* pbadd = new QToolButton(this);
	pbadd->setIcon(QIcon(":data/ContactNew.png"));
	if(isBuddyList)
		pbadd->setToolTip(tr("Add a new contact."));
	else
		pbadd->setToolTip(tr("Add a new block."));
	pbadd->setAutoRaise(true);
	connect(pbadd, SIGNAL(clicked()), SLOT(Add()));

	QToolButton* pbdel = new QToolButton(this);
	pbdel->setIcon(QIcon(":data/ContactDelete.png"));
	if(isBuddyList)
		pbdel->setToolTip(tr("Delete the selected contact..."));
	else
		pbdel->setToolTip(tr("Delete the selected block..."));
	pbdel->setAutoRaise(true);
	connect(pbdel, SIGNAL(clicked()), SLOT(Delete()));

//	QTreeView* contacts = new QTreeView;
//////////////////////////////////////////////////////////////////////////

	QListWidget* lwcontacts = new QListWidget(this);
	QStringList headerLabels;
	headerLabels << "Name";
	lwcontacts->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lwcontacts->setObjectName("lwcontacts");
	lwcontacts->setSelectionBehavior(QAbstractItemView::SelectRows);
	lwcontacts->setSelectionMode(QAbstractItemView::SingleSelection);
	lwcontacts->setSortingEnabled(true);
	if(isBuddyList)
		lwcontacts->setToolTip(tr("List of players who are your contacts."));
	else
		lwcontacts->setToolTip(tr("List of players you have blocked."));

	//////////////////////////////////////////////////////////////////////////

	QGridLayout* lay = new QGridLayout();
	//lay->addWidget(pbnew, 0,0);
	lay->addWidget(pbadd, 0,0);
	lay->addWidget(pbdel, 0,1);
	lay->addWidget(lwcontacts, 1,0, 1,3);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuContactPage::Add()
{
	AuContactNew addnew(m_isBuddyList, this);
	if(addnew.exec() == QDialog::Accepted)
	{
		// First check to see if this contact is already in the list, don't send the update if it exists
		AuId id = addnew.ContactId();
		QString name = addnew.ContactName();

		const DgData& gd = g_Au.LockDataForRead();
		bool exists=false;
		for(QList<AuContact>::const_iterator it = gd.GetContacts().begin(); it != gd.GetContacts().end(); it++)
		{
			if((*it).m_id == id)
			{
				exists = true;	// don't care whether it is a buddy or squelchy. if in the list, don't add again.
				break;
			}
		}
		g_Au.UnlockData();

		if(!exists)	// not in the list, send it to the database
		{
			if(id == m_idPerson && !m_isBuddyList)
			{
				QMessageBox::warning(this, g_Au.ApplicationName(), tr("Don't block yourself."));
			}
			else
			{
				AuContact contact;
				contact.m_id = id;
				contact.m_name = name;
				contact.m_bIgnore = !m_isBuddyList;

				const DgData& gd = g_Au.LockDataForRead();
				QString sAcctName = gd.GetAcctName();
				QByteArray sAcctPassword = gd.GetAcctPassCrypt();
				g_Au.UnlockData();

				if(!g_Au.IsSceneSolo())
				{
					AuLoginComm login;
					login.UpdateContact(sAcctName, sAcctPassword, contact, false);
				}

				DgData& gdwrite = g_Au.LockDataForWrite();
				gdwrite.AddContact(contact);
				g_Au.UnlockData();

				ShowContactList();
			}
		}
		else
		{
			QMessageBox::warning(this, g_Au.ApplicationName(),
					tr("%1 is already in your list. They can only be listed once, as either a contact or a block.").arg(name));
		}
	}
}

void AuContactPage::Delete()
{
	QListWidget* lwcontacts = findChild<QListWidget*>("lwcontacts");
	if(!lwcontacts)
		return;
	QList<QListWidgetItem *> selitems = lwcontacts->selectedItems();
	if(selitems.isEmpty())
		return;

	const DgData& gd = g_Au.LockDataForRead();
	QString acctName = gd.GetAcctName();
	QByteArray acctPassword = gd.GetAcctPassCrypt();
	g_Au.UnlockData();

	if(selitems.count() == 1)
	{
		// only deleting one
		QListWidgetItem* item = selitems.first();
		AuContact contact;
		contact.m_id = AuId(item->data(Qt::UserRole).toUInt());
		contact.m_name = item->data(Qt::UserRole+1).toString();
		contact.m_bIgnore = !m_isBuddyList;

		QString smsg = tr("<html>"
				"Confirm delete of %1. "
				"</html>").arg(contact.m_name);
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
		if(mb.exec() != QMessageBox::Ok)
			return;

		// now remove it from the list and refresh list widget
		DgData& gd = g_Au.LockDataForWrite();
		gd.DeleteContact(contact);
		g_Au.UnlockData();

		if(!g_Au.IsSceneSolo())
		{
			AuLoginComm login;
			login.UpdateContact(acctName, acctPassword, contact, true);
			if(!m_isBuddyList)
				emit UpdateSquelch(contact.m_id, true);
		}
	}
	else
	{
		// Make a list of contacts and send them off for deletion
		QString smsg = tr("<html>"
				"Confirm delete of multiple selected contacts. "
				"</html>");
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
		if(mb.exec() != QMessageBox::Ok)
			return;

		QList<AuContact> contacts;
		AuIdList idContacts;
		for(QList<QListWidgetItem *>::iterator it = selitems.begin(); it != selitems.end(); it++)
		{
			AuContact contact;
			contact.m_id = AuId((*it)->data(Qt::UserRole).toUInt());
			contact.m_name = (*it)->data(Qt::UserRole+1).toString();
			contact.m_bIgnore = !m_isBuddyList;
			contacts.append(contact);
			
			idContacts.append(contact.m_id);
		}

		// now remove it from the list and refresh list widget
		DgData& gd = g_Au.LockDataForWrite();
		gd.DeleteContacts(contacts);
		g_Au.UnlockData();

		if(!g_Au.IsSceneSolo())
		{
			AuLoginComm login;
			login.UpdateContacts(acctName, acctPassword, contacts, true);
			if(!m_isBuddyList)
				emit UpdateSquelches(idContacts, true);
		}
	}
	ShowContactList();
}

void AuContactPage::ShowContactList()
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
		QString contact = tr("%1 (%2)").arg((*it).m_name).arg((*it).m_id.StringId());
		if(m_isBuddyList)
		{
			// Buddies
			if(!(*it).m_bIgnore)
			{
				QListWidgetItem* lwi = new QListWidgetItem(contact, lwcontacts);
				lwi->setData(Qt::UserRole, (*it).m_id.GetID());
				lwi->setData(Qt::UserRole+1, (*it).m_name);
			}
		}
		else
		{
			// Squelchies
			if((*it).m_bIgnore)
			{
				QListWidgetItem* lwi = new QListWidgetItem(contact, lwcontacts);
				lwi->setData(Qt::UserRole, (*it).m_id.GetID());
				lwi->setData(Qt::UserRole+1, (*it).m_name);
			}
		}
	}
	g_Au.UnlockData();

	lwcontacts->setSortingEnabled(true);
	lwcontacts->sortItems();
}

void AuContactPage::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	// Update data in controls when we become visible and the data is not valid.
	// This happens the first time we become visible.
	if(visible)
		ShowContactList();
}
