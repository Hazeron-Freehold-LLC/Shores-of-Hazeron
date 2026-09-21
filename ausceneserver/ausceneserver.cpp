//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceneserver.cpp,v 1.107 2022/02/19 18:39:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceneserver.h"
//
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "dtsceneserver.h"

AuSceneServer::AuSceneServer(QObject* parent) : QTcpServer(parent)
{
	m_NextId = 0;
}

void AuSceneServer::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuSceneServer::incomingConnection socketDesc %d", socketDescriptor);

	// Create a thread to handle the client connection.
	m_NextId++;
	while(!m_NextId)
		m_NextId++;	// skip zero
	DtSceneServer* tserver = new DtSceneServer(m_NextId, socketDescriptor, this);

	connect(tserver, SIGNAL(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)),
			auSceneServerApp, SLOT(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)));

	connect(tserver, SIGNAL(ChatReceived(QByteArray)), auSceneServerApp, SLOT(ChatReceived(QByteArray)));

	connect(tserver, SIGNAL(Disconnecting(AuId)), auSceneServerApp, SIGNAL(UnitDisconnected(AuId)));
	connect(tserver, SIGNAL(DeferClient(AuId)), SLOT(DeferClient(AuId)));
	connect(tserver, SIGNAL(EmpireAdded(quint32)), auSceneServerApp, SIGNAL(AddEmpireToDb(quint32)));
	connect(tserver, SIGNAL(Finished(unsigned int)), SLOT(ServerThreadFinished(unsigned int)));

	connect(tserver, SIGNAL(GovSetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
					quint8, quint8, QString, QString, QString)),
			auSceneServerApp, SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
					quint8, quint8, QString, QString, QString)));

	connect(tserver, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			auSceneServerApp, SLOT(OfficeAssigned(AuId, quint32, AuOffice)));

	connect(tserver, SIGNAL(PlayerConnecting(quint32, unsigned long)),
			auSceneServerApp, SIGNAL(DisconnectExcessSceneChannels(quint32, unsigned long)));
	connect(tserver, SIGNAL(SceneAtomChanged(AuId, AuId)), auSceneServerApp, SIGNAL(SceneAtomChanged(AuId, AuId)));
	connect(tserver, SIGNAL(PersonSystemChanged(AuId, quint32)), auSceneServerApp, SLOT(PersonSystemChanged(AuId, quint32)));
	connect(tserver, SIGNAL(AtomTraveled(AuId, AuId, AuId, AuId)), auSceneServerApp, SLOT(AtomTraveled(AuId, AuId, AuId, AuId)));

	// This signal closes all excess chat channels when a nu one is opened from a particular login account.
	connect(auSceneServerApp, SIGNAL(DisconnectExcessSceneChannels(quint32, unsigned long)),
			tserver, SLOT(DisconnectPlayer(quint32, unsigned long)));

	connect(auSceneServerApp, SIGNAL(DisconnectUnit(AuId)), tserver, SLOT(DisconnectUnit(AuId)));

	connect(auSceneServerApp, SIGNAL(DisconnectUnitTransferred(AuId)), tserver, SLOT(DisconnectUnitTransferred(AuId)));

	connect(auSceneServerApp, SIGNAL(UdpPacketReceived(quint32, QByteArray)),
			tserver, SLOT(UdpPacketReceived(quint32, QByteArray)));

	connect(auSceneServerApp, SIGNAL(AtomPresenceResolved(AuId, quint32, QString, quint16, QString)),
			tserver, SLOT(AtomPresenceResolved(AuId, quint32, QString, quint16, QString)));

	connect(auSceneServerApp, SIGNAL(EmpireRecordAvailable(quint32)), tserver, SLOT(EmpireRecordAvailable(quint32)));

	connect(auSceneServerApp, SIGNAL(SolarSystemChanged(AuId, AuId)), tserver, SLOT(SolarSystemChanged(AuId, AuId)));

	//qDebug("AuSceneServer::incomingConnection DtSceneServer thread created");

	// Start the thread.
	tserver->start();
	//qDebug("AuSceneServer::incomingConnection DtSceneServer thread started");

	// Instead, let's keep track of these server threads so we can monitor their status.
	m_ServersLock.lockForWrite();
	m_Servers.insert(pair<unsigned int, DtSceneServer*>(tserver->Id(), tserver));
	m_ServersLock.unlock();
	//qDebug("AuSceneServer::incomingConnection %d server threads.", m_Servers.count());
}

void AuSceneServer::ServerThreadFinished(unsigned int serverId)
{
	//qDebug() << "AuSceneServer::ServerThreadFinished id" << serverId;
	m_ServersLock.lockForWrite();
	unordered_map<unsigned int, DtSceneServer*>::iterator it = m_Servers.find(serverId);
	if(it != m_Servers.end())
	{
		// We found a server thread that is supposed to die and that has finished.
		//qDebug() << "AuSceneServer::ServerThreadFinished found server id" << (*it).second->Id();
		(*it).second->deleteLater();
		it = m_Servers.erase(it);
	}
	m_ServersLock.unlock();
}

// system id, # of clients
void AuSceneServer::SystemClients(unordered_map<AuId, AuSystemStatus, AuIdHash>& systems) const
{
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtSceneServer*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;

		// Get the solar system id that contains the client's unit.
		AuId system = (*it).second->ClientSolarSystem();

		// Look for the solar system id in the map.
		unordered_map<AuId, AuSystemStatus, AuIdHash>::iterator scit = systems.find(system);
		if(scit != systems.end())
			(*scit).second.m_Clients++;
		else
		{
			// This should not happen because AuSceneServerApp::CollectSystemStatus has already populatated the systems list
			// by calling each atom manager.
			//qDebug("AuSceneServer::SystemClients client %d claims to be in system %d not listed by manager.",
					//(*it).second->UnitId().GetID(), system.GetID());
			AuSystemStatus status;
			status.m_Clients = 1;
			systems.insert(pair<AuId, AuSystemStatus>(system, status));
		}
	}
	m_ServersLock.unlock();
}

void AuSceneServer::DeferClient(AuId unit)
{
	auSceneServerApp->QueryAtomServer(unit, 0, AuId(0));
}

void AuSceneServer::PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const
{
	players.clear();
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtSceneServer*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;
		AuPlayerOnlineStatus player;
		(*it).second->PlayerOnlineStatus(player);
		if(player.m_Id.IsNull())
			continue;
		if(player.m_Name.isEmpty())
			continue;
		if(players.Contains(player.m_Id))
			continue;
		players.append(player);
	}
	m_ServersLock.unlock();
}
