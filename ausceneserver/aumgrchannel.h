//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumgrchannel.h,v 1.3 2022/02/19 18:39:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMgrChannel_H
#define AuMgrChannel_H

#include <QTcpServer>

class AuMgrChannel : public QTcpServer
{
	Q_OBJECT;

public:
	AuMgrChannel(QObject* parent=0);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);
};

#endif // AuMgrChannel_H
