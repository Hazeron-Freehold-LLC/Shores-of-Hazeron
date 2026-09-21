//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontacts.cpp,v 1.17 2022/02/19 18:36:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucontacts.h"
//
#include "aucontactpage.h"
#include "aucontacttabs.h"
#include <aulogincomm.h>
#include <dgdata.h>
#include <QIcon>
#include <QPainter>
#include <QStackedLayout>

AuContacts::AuContacts(const AuId& idPerson, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Contacts.png"));
	setAuDlgTitle("Contacts");

	QStackedLayout* pages = new QStackedLayout();
	pages->setObjectName("ContactPages");

	AuContactTabs* tabs = new AuContactTabs(this);
	tabs->setObjectName("ContactTabs");
	connect(tabs, SIGNAL(currentChanged(int)), pages, SLOT(setCurrentIndex(int)));

	QGridLayout* lay = new QGridLayout();
	lay->addWidget(tabs, 0,0);
	lay->addLayout(pages, 1,0);
	lay->setMargin(0);
	lay->setSpacing(0);
	setAuDlgLayout(lay);

	// Get the contact info here so we can pass it to each of the pages
	const DgData& gd = g_Au.LockDataForRead();
	QString acctName = gd.GetAcctName();
	QByteArray acctPassword = gd.GetAcctPassCrypt();
	g_Au.UnlockData();

	// Solo mode already has all contacts in DgData.
	if(!g_Au.IsSceneSolo())
	{
		AuLoginComm login;
		login.RequestContacts(acctName,acctPassword);
	}

	CreatePage(idPerson, tr("Contacts"), "contacts", true, QIcon(":data/ContactBuddies.png"));
	CreatePage(idPerson, tr("Blocked"), "blocked", false, QIcon(":data/ContactBlocked.png"));

	tabs->setCurrentIndex(0);
	pages->setCurrentIndex(0);
}

void AuContacts::CreatePage(const AuId& idPerson, const QString& tabName, const QString& objectName, bool isBuddyPage,
		const QIcon& icon)
{
	AuContactPage* page = findChild<AuContactPage*>(objectName);
	if(page)
	{
		qDebug() << "AuContacts::CreatePage - Page already exists." << objectName;
		return;
	}
	AuContactTabs* tabs = findChild<AuContactTabs*>("ContactTabs");
	QStackedLayout* pages = findChild<QStackedLayout*>("ContactPages");
	page = new AuContactPage(idPerson, isBuddyPage, objectName, this);
	if(tabs && pages && page)
	{
		tabs->addTab(icon, tabName);
		pages->addWidget(page);

		connect(page, SIGNAL(UpdateSquelch(AuId, bool)), SIGNAL(UpdateSquelch(AuId, bool)));
		connect(page, SIGNAL(UpdateSquelches(AuIdList, bool)), SIGNAL(UpdateSquelches(AuIdList, bool)));
	}
	else
	{
		qDebug("AuContacts::CreatePage - tabs %c pages %c page %c",
				tabs?'y':'n',
				pages?'y':'n',
				page?'y':'n');
	}
}
