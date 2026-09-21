//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailnew.cpp,v 1.18 2022/02/19 18:36:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailnew.h"
//
#include "aucontacts.h"
#include "aumailcontacts.h"
#include <dgdata.h>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QToolButton>

AuMailNew::AuMailNew(QWidget* parent, AuId idPerson, QList<AuContact>* qlContacts, QString contactName, QString subject,
		QString body) : AuDialog(parent)
{
	m_idPerson = idPerson;
	if(qlContacts)
		m_Contacts.append(*qlContacts);

	setAuDlgTitle(tr("New Mail Message"));
	setAuDlgIcon(QIcon(":data/MailMessage.png"));

	QPushButton* pbto = new QPushButton(QIcon(":data/ContactBuddies.png"), tr("To:"), this);
	connect(pbto, SIGNAL(clicked()), SLOT(ToClicked()));

	m_Send = new QToolButton(this);
	m_Send->setIcon(QIcon(":data/SendMail.png"));
	m_Send->setToolTip(tr("Send Message"));
	m_Send->setAutoRaise(true);
	connect(m_Send, SIGNAL(clicked()), SLOT(SendClicked()));

	m_ToLabel = new QLabel(this);
	if(contactName == "")
	{
		// get the list of name(s) from the list of m_Recipients
		for(QList<AuContact>::iterator it = m_Contacts.begin(); it != m_Contacts.end(); it++)
			contactName += (*it).m_name + ", ";
	}
	m_ToLabel->setText(contactName);

	QLabel* lblsubj = new QLabel(tr("Subject:"), this);

	m_Subject = new QLineEdit(this);
	m_Subject->setMaxLength(MAIL_MESSAGE_MAX_SUBJECT);
	m_Subject->setText(subject);

	m_Message = new QTextEdit(this);
	m_Message->setText(body);
	connect(m_Message, SIGNAL(textChanged()), SLOT(MessageChanged()));

	QGridLayout* lay = new QGridLayout;
	lay->addWidget(pbto, 0,0);
	lay->addWidget(m_ToLabel, 0,1);
	lay->addWidget(m_Send, 0,2);
	lay->addWidget(lblsubj, 1,0);
	lay->addWidget(m_Subject, 1,1, 1,2);
	lay->addWidget(m_Message, 2,0, 1,3);
	lay->setColumnStretch(1, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	
	setAuDlgLayout(lay);
}

void AuMailNew::ToClicked()
{
	AuMailContacts cnt(this);
	if(cnt.exec() == QDialog::Accepted)
	{
		m_Contacts.clear();
		m_Contacts.append(cnt.m_Contacts);

		QString contactNames;
		int iCount = m_Contacts.count();
		if(iCount == 1)
		{
			contactNames = m_Contacts[0].m_name;
		}
		else if(iCount > 1)
		{
			int iCurrent=0;
			for(QList<AuContact>::iterator it = m_Contacts.begin(); it != m_Contacts.end(); it++)
			{
				iCurrent++;
				contactNames += (*it).m_name;
				if(iCurrent != iCount)
					contactNames += ", ";
			}
		}
		m_ToLabel->setText(contactNames);
	}
}

void AuMailNew::SendClicked()
{
	if(!m_Contacts.count())
		return;

	const DgData& gd = g_Au.LockDataForRead();
	unsigned int idPlayer = gd.PlayerId();
	unsigned int confirmNum = gd.ConfirmationNumber();
	QString charname = gd.ClientUnitName(m_idPerson);
	g_Au.UnlockData();

	// Get the subject and body text.
	QString subject = m_Subject->text();
	QString body = m_Message->toHtml();

	// SendBulkMail checks to see if there are multiple recipients.
	// Calls SendMail if only one
	AuIdList recipientIds;
	for(QList<AuContact>::iterator it = m_Contacts.begin(); it != m_Contacts.end(); it++)
		if(!(*it).m_id.IsNull())
			recipientIds.append((*it).m_id);

	if(!recipientIds.isEmpty())
		g_Au.SendBulkMail(idPlayer, confirmNum, m_idPerson, charname, recipientIds, AuMailMsg::MSG_Mail, subject, body,
				AuId(0), "", AuId(0), "", GALAXY_Count, AuPoint(0,0,0));

	// CC to myself
	QString sTo = m_ToLabel->text();

	// Prepend a salutation to the body for my CC'd copy.
	QString myccbody = tr("<b>Sent To: ") + sTo + "</b><hr>" + body;
	g_Au.SendMail(idPlayer, confirmNum, m_idPerson, charname, m_idPerson, AuMailMsg::MSG_Mail, subject, myccbody,
			AuId(0), "", AuId(0), "", GALAXY_Count, AuPoint(0,0,0));

	accept();
}

void AuMailNew::MessageChanged()
{
	// Disable the Send button if the body text will be truncated.
	QString body = m_Message->toHtml();
	QString bodyTruncated(body);
	bodyTruncated.truncate(MAIL_MESSAGE_MAX_TEXT);
	if(body != bodyTruncated)
	{
		// The message body is too long.
		m_Send->setEnabled(false);
		m_Send->setToolTip(tr("<html>Send Message<br><i>Disabled</i> because message body is too long.</html>"));
	}
	else
	{
		// The message body length is ok. It did not get truncated.
		m_Send->setEnabled(true);
		m_Send->setToolTip(tr("Send Message"));
	}
}
