//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailserver.cpp,v 1.6 2022/02/19 18:37:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailserver.h"
//
#include "aumailserverapp.h"
#include "dtmailserver.h"

AuMailServer::AuMailServer(QObject* parent) : QTcpServer(parent)
{
}

void AuMailServer::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuMailServer::incomingConnection");

	// Create a thread to handle the client connection.
	DtMailServer* tserver = new DtMailServer(socketDescriptor, this);

	// By connecting the thread's finished() signal to QObject::deleteLater(), we ensure
	// that the thread gets deleted once it has finished.
	//connect(tserver, SIGNAL(finished()), tserver, SLOT(deleteLater()));
	connect(tserver, SIGNAL(DeleteMailMessages(quint32, AuId, AuMailMsgIdList)),
			auMailServerApp, SLOT(DeleteMailMessages(quint32, AuId, AuMailMsgIdList)),
			Qt::QueuedConnection);
	connect(tserver, SIGNAL(MailMessageReceived(quint32, AuMailMsg)),
			auMailServerApp, SLOT(MailMessageReceived(quint32, AuMailMsg)),
			Qt::QueuedConnection);
	connect(tserver, SIGNAL(MailMessageReceivedBulk(quint32, AuMailMsg, AuIdList)),
			auMailServerApp, SLOT(MailMessageReceivedBulk(quint32, AuMailMsg, AuIdList)),
			Qt::QueuedConnection);
	connect(tserver, SIGNAL(RequestMailMessages(quint32, AuId, quint32, AuIdList, quint8, quint32, quint8, quint32, quint8,
					quint32)),
			auMailServerApp, SLOT(RequestMailMessages(quint32, AuId, quint32, AuIdList, quint8, quint32, quint8, quint32, quint8,
					quint32)),
			Qt::QueuedConnection);
	connect(auMailServerApp, SIGNAL(PlayerMailMessages(quint32, AuMailMsgArray)),
			tserver, SLOT(PlayerMailMessages(quint32, AuMailMsgArray)),
			Qt::QueuedConnection);

	// Instead, let's keep track of these server threads so we can monitor their status.
	m_Servers.append(tserver);
	connect(tserver, SIGNAL(finished()), SLOT(ServerThreadFinished()));

	// Start the thread.
	tserver->start();
}

void AuMailServer::ServerThreadFinished()
{
	//qDebug("AuMailServer::ServerThreadFinished");
	QVector<DtMailServer*>::iterator it = m_Servers.begin();
	while(it != m_Servers.end())
	{
		if((*it)->isFinished())
		{
			// We found a server thread that has finished.
			//qDebug("AuMailServer::ServerThreadFinished - Found the server thread.");
			(*it)->deleteLater();
			it = m_Servers.erase(it);
		}
		else
		{
			it++;
		}
	}
}
