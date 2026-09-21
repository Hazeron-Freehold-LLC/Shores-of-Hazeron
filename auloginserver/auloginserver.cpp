//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloginserver.cpp,v 1.13 2022/02/19 18:35:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auloginserver.h"
//
#include <auglobal.h>
#include "auloginserverapp.h"
#include "dtloginserver.h"

AuLoginServer::AuLoginServer(QObject* parent) : QTcpServer(parent)
{
	m_NextId = 0;
}

void AuLoginServer::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuLoginServer::incomingConnection");

	// Create a thread to handle the client connection.
	DtLoginServer* tserver = new DtLoginServer(m_NextId++, socketDescriptor,
			this);

	// By connecting the thread's finished() signal to QObject::deleteLater(),
	// we ensure that the thread gets deleted once it has finished.
	connect(tserver, SIGNAL(finished()), tserver, SLOT(deleteLater()));

	// Start the thread.
	tserver->start();
}
