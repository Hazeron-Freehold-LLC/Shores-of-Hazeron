//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailserver.h,v 1.3 2022/02/19 18:37:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUMAILSERVER_H
#define AUMAILSERVER_H

#include <QTcpServer>
#include <QVector>

class DtMailServer;

class AuMailServer : public QTcpServer
{
	Q_OBJECT;

public:
	AuMailServer(QObject* parent=0);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);

protected slots:
	void ServerThreadFinished();

public:
	QVector<DtMailServer*>	m_Servers;
};

#endif // AUMAILSERVER_H
