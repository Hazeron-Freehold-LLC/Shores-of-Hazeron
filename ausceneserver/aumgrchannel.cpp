//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumgrchannel.cpp,v 1.20 2022/02/19 18:39:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumgrchannel.h"
//
#include "ausceneserverapp.h"
#include "dtmgrchannel.h"

AuMgrChannel::AuMgrChannel(QObject* parent) : QTcpServer(parent)
{
}

void AuMgrChannel::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuMgrChannel::incomingConnection socketDesc %d", socketDescriptor);
	// Create a thread to handle the client connection.
	DtMgrChannel* tserver = new DtMgrChannel(socketDescriptor, this);

	connect(tserver, SIGNAL(CreateSectorAt(quint8, double,double,double)),
			auSceneServerApp, SLOT(CreateSectorAt(quint8, double,double,double)));
	connect(tserver, SIGNAL(DisconnectUnit(AuId)), auSceneServerApp, SIGNAL(DisconnectUnit(AuId)));
	connect(tserver, SIGNAL(FinalizeSystem(AuId)), auSceneServerApp, SLOT(FinalizeSystem(AuId)));
	connect(tserver, SIGNAL(LimboRescue(AuId)), auSceneServerApp, SLOT(LimboRescue(AuId)));
	connect(tserver, SIGNAL(ReassignSystem(AuId, QString, quint16)),
			auSceneServerApp, SLOT(ReassignSystem(AuId, QString, quint16)));
	connect(tserver, SIGNAL(ReloadData()), auSceneServerApp, SIGNAL(ReloadData()));
	connect(tserver, SIGNAL(ReloadScript(quint32)), auSceneServerApp, SLOT(ReloadScript(quint32)));
	connect(tserver, SIGNAL(RestoreEmpire(DgEmpire)), auSceneServerApp, SLOT(RestoreEmpire(DgEmpire)));
	connect(tserver, SIGNAL(RevokeAssignment(AuId)), auSceneServerApp, SLOT(RevokeAssignment(AuId)));
	connect(tserver, SIGNAL(SendResources(quint8, AuId)), auSceneServerApp, SLOT(MgrResourceRequest(quint8, AuId)));
	connect(tserver, SIGNAL(SendMemoryUsage()), auSceneServerApp, SLOT(MgrMemoryUsageRequest()));
	connect(tserver, SIGNAL(SendStatus(quint8)), auSceneServerApp, SLOT(MgrStatusRequest(quint8)));
	connect(tserver, SIGNAL(ServerBroadcast(QString)), auSceneServerApp, SLOT(ServerBroadcast(QString)));
	connect(tserver, SIGNAL(SetAtomModified(AuId, bool)), auSceneServerApp, SLOT(SetAtomModified(AuId, bool)));
	connect(tserver, SIGNAL(Shutdown(QString, QString, qint16)), auSceneServerApp, SLOT(Shutdown(QString, QString, qint16)));

	// By connecting the thread's finished() signal to QObject::deleteLater(),
	// we ensure that the thread gets deleted once it has finished.
	connect(tserver, SIGNAL(finished()), tserver, SLOT(deleteLater()));
	//qDebug("AuMgrChannel::incomingConnection DtMgrChannel thread created");

	connect(auSceneServerApp, SIGNAL(MgrSendPacket(QByteArray)), tserver, SLOT(MgrSendPacket(QByteArray)));

	// Start the thread.
	tserver->start();
	//qDebug("AuMgrChannel::incomingConnection DtMgrChannel thread started");
}
