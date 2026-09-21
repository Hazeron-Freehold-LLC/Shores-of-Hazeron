//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloginserver.h,v 1.7 2022/02/19 18:35:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AULOGINSERVER_H
#define AULOGINSERVER_H

#include <QTcpServer>
#include <QVector>

class DtLoginServer;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuLoginServer : public QTcpServer
{
	Q_OBJECT;

public:
	AuLoginServer(QObject* parent=0);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);

private:
	unsigned int			m_NextId;	// Ids for login server threads
};

#endif // AULOGINSERVER_H
