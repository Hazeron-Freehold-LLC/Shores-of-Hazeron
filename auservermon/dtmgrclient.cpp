//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmgrclient.cpp,v 1.62 2022/02/19 18:40:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtmgrclient.h"
//
#include "auglobal.h"
#include "dtmcpacket.h"

static int s_ResourceReportMetaType = 0;
static int s_ResourceReportAtomMetaType = 0;

unsigned int AuStatusReport::AverageCycleMs() const
{
	unsigned int totalAvgCycle = 0;
	unsigned int cycles = 0;
	for(QList<AuStatusAtom>::const_iterator ai = m_Atoms.constBegin(); ai != m_Atoms.constEnd(); ai++)
	{
		unsigned int avgCycle = (*ai).m_AvgTickCycle;
		if(avgCycle)
		{
			totalAvgCycle += avgCycle;
			cycles++;
		}
	}
	if(cycles)
		totalAvgCycle /= cycles;
	return totalAvgCycle;
}

unsigned int AuStatusReport::SystemsDeadlocked() const
{
	unsigned int deadlocked = 0;
	for(QList<AuStatusAtom>::const_iterator ai = m_Atoms.constBegin(); ai != m_Atoms.constEnd(); ai++)
		if((*ai).IsDeadlocked())
			deadlocked++;
	return deadlocked;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint32 AuResourceReport::TotalAtoms() const
{
	quint32 total = 0;
	for(QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ai = m_Atoms.cbegin(); ai != m_Atoms.cend(); ai++)
		total += *ai;
	return total;
}

quint32 AuResourceReport::TotalTrash() const
{
	quint32 total = 0;
	for(QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ai = m_Trash.cbegin(); ai != m_Trash.cend(); ai++)
		total += *ai;
	return total;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DtMgrClient::DtMgrClient(QObject* parent) : QThread(parent)
{
	m_ConnectDelay = 0;
	m_ConnectionPort = 0;
	m_MaxHistory = 100;
	m_PollAwaitingReply = false;
	m_PollInterval = 0;
	m_PollMode = 0;						// Default is continuous polling.
	m_PollUnitId = 0;
	m_ServerMode = AuGlobal::SceneWorker;

	// Comm
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_isConnected = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;

	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	if(!s_ResourceReportMetaType)
		s_ResourceReportMetaType = qRegisterMetaType<AuResourceReport>();
	if(!s_ResourceReportAtomMetaType)
		s_ResourceReportAtomMetaType = qRegisterMetaType<AuResourceReportAtom>();
}

DtMgrClient::~DtMgrClient()
{
}

void DtMgrClient::run()
{
	Q_ASSERT(!m_ConnectionName.isEmpty());
	Q_ASSERT(m_ConnectionPort);
	//qDebug() << "DtMgrClient::run connect" << m_ConnectionName << "port" << m_ConnectionPort << "delayMs" << m_ConnectDelay;
	if(m_ConnectDelay > 0)
	{
		msleep(m_ConnectDelay);
		//qDebug() << "DtMgrClient::run delay done";
	}

	// Creating the socket on the stack automatically associates it with the thread's
	// event loop. This ensures that Qt will not try to deliver events to our socket
	// from the main thread while we are accessing it from DtMgrClient::run().
	QTcpSocket tcpSocket;
	//connect(&tcpSocket, SIGNAL(readyRead()), SLOT(RxData()));
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(NetworkError(QAbstractSocket::SocketError)));

	// Connect to server.
	tcpSocket.abort();
	tcpSocket.connectToHost(m_ConnectionName, m_ConnectionPort);
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() != QAbstractSocket::ConnectedState)
	{
		// The connection attempt failed. The server or the network is probably not up.
		//qWarning("DtMgrClient::run - tcp connection attempt failed");
		return;
	}
	m_isConnected = true;
	emit Connected();
	//qDebug() << "DtMgrClient::run connected to" << m_ConnectionName << "port" << m_ConnectionPort;

	// Send connection packet that MUST be fist.
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtMgrChannelPacket::P_Connect;
		out << (quint32)AuId::Haxus().GetID();
		QueuePacket(ba);
	}

	while(m_isConnected && tcpSocket.state() == QAbstractSocket::ConnectedState && !tcpSocket.waitForDisconnected(10))
	{
		// Send anything that's ready to go.
		TxData(tcpSocket);

		// Pull in any data that is waiting for us.
		// Note: If we wait for the signal, it appears to come from the gui
		// thread, which causes us grief. It's better for us to get the data
		// when it's there.
		RxData(tcpSocket);

		// Poll for status.
		if(m_PollInterval && m_PollTime.elapsed() >= m_PollInterval)
		{
			if(m_PollMode == 0)
			{
				// Repeating polls to monitor the server.
				if(!m_PollAwaitingReply)
				{
					Poll();
					m_PollAwaitingReply = true;
				}
			}
			else
			{
				// One single poll to request information.
				Poll();
				m_PollInterval = 0;
			}
		}
	}

	m_isConnected = false;
	//qDebug("DtMgrClient::run - Disconnected.");
}

void DtMgrClient::NetworkError(QAbstractSocket::SocketError socketError)
{
	switch(socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qWarning("DtMgrClient::NetworkError - RemoteHostClosedError");
		break;
	case QAbstractSocket::HostNotFoundError:
		qWarning("DtMgrClient::NetworkError - HostNotFoundError");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		qWarning("DtMgrClient::NetworkError - ConnectionRefusedError");
		break;
	default:
		//if(m_Tcp)
			//qWarning("DtMgrClient::NetworkError - Error %s",
					//(const char*)m_Tcp->errorString());
		//else
			qWarning("DtMgrClient::NetworkError - Error");
		break;
	}
}

void DtMgrClient::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtMgrClient::RxData");
	while(tcpSocket.bytesAvailable())
	{
		if(!m_PacketSize || m_PacketSizeBytesLeft)
		{
			int packetSizeBytesRead = AuUtil::ReadPacketSize(tcpSocket, m_PacketSize, m_PacketSizeBytesLeft);
			if(m_PacketSizeBytesLeft)
				break;					// We don't have all of the packet size bytes yet.
			m_BytesRx += packetSizeBytesRead;
		}

		// Wait until all of the bytes arrive.
		if(tcpSocket.bytesAvailable() < m_PacketSize)
			break;

		// It is now safe to read our data from the socket buffer.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_BytesRx += m_PacketSize;
		m_PacketSize = 0;
		if(!ReadPacket(block, tcpSocket))
		{
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
			m_isConnected = false;
		}
	}
}

void DtMgrClient::TxData(QTcpSocket& tcpSocket)
{
	// Check to see if we have anything to send in the tcp stream.
	//qDebug("DtMgrClient::TxData");
	m_TcpBufferLock.lock();
	if(!m_TcpBuffer.isEmpty())
	{
		// There is data to be sent.
		// It is already formatted into packets with block sizes.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			int written = tcpSocket.write(m_TcpBuffer);
			m_BytesTx += written;
			//qDebug("DtMgrClient::TxData - %d bytes written", written);
			m_TcpBuffer.clear();
		}
	}
	m_TcpBufferLock.unlock();
}

bool DtMgrClient::ReadPacket(const QByteArray& ba, QTcpSocket& /*tcpSocket*/)
{
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	quint8 ptype;
	block >> ptype;
	//qDebug("DtMgrClient::ReadPacket - %d bytes rx'd, PACKET Type %d",
			//ba.count(), ptype);
	// This cast generates a handy warning when an enum value is forgotten.
	switch((DtMgrChannelPacket::PACKET)ptype)
	{
	case DtMgrChannelPacket::P_MemoryUsage:
		{
			QStringList memrpt;
			block >> memrpt;
			emit MemoryUsageReport(m_ConnectionName, m_ConnectionPort, memrpt);
		}
		break;
	case DtMgrChannelPacket::P_Status:
		{
			quint8 specificStatus;
			block >> specificStatus;
			//qDebug("P_Status %d", specificStatus);

			m_PollAwaitingReply = false;	// This was the reply to our poll; we are no longer waiting for it.

			switch(specificStatus)
			{
			case 0:
			default:
				break;
			case 1:
				// Full text report.
				break;
			case 2:
				// Load status.
				// Sometimes, all we want is to know that we got an answer.
				ReadStatusOfLoads(block);						// Read and post the status report data.
				break;
			case 3:
				// User status
				ReadStatusOfUsers(block);						// Read and post the status report data.
				break;
			}
		}
		break;

	case DtMgrChannelPacket::P_Resources:
		{
			quint8 specificResources;
			block >> specificResources;
			//qDebug("P_Resources %d", specificResources);

			switch(specificResources)
			{
			case 0:
			default:
				break;
			case 1:
				ReadResourceReport(block);
				break;
			case 2:
				ReadResourceReportAtom(block);
				break;
			}
		}
		break;

	case DtMgrChannelPacket::P_Invalid:
	case DtMgrChannelPacket::P_Announcement:
	case DtMgrChannelPacket::P_CreateSectorAt:
	case DtMgrChannelPacket::P_Connect:
	case DtMgrChannelPacket::P_Debug:
	case DtMgrChannelPacket::P_DebugMessages:
	case DtMgrChannelPacket::P_FinalizeSystem:
	case DtMgrChannelPacket::P_KickUnit:
	case DtMgrChannelPacket::P_LimboRescue:
	case DtMgrChannelPacket::P_Reassign:
	case DtMgrChannelPacket::P_ReloadData:
	case DtMgrChannelPacket::P_ReloadScript:
	case DtMgrChannelPacket::P_RestoreEmpire:
	case DtMgrChannelPacket::P_Revoke:
	case DtMgrChannelPacket::P_SetModified:
	case DtMgrChannelPacket::P_Setting:
	case DtMgrChannelPacket::P_Shutdown:
	case DtMgrChannelPacket::P_StatusQuery:
#if defined(QT_NO_DEBUG)
	default:
#endif
		qWarning("DtMgrClient::ReadPacket - ERROR! UNEXPECTED PACKET TYPE %d", ptype);
		break;
	}
	return true;
}

void DtMgrClient::QueuePacket(const QByteArray& ba)
{
	// The packet is already formatted. Add it to the transmit data stream.
	m_TcpBufferLock.lock();
	AuUtil::WritePacketSize(m_TcpBuffer, ba.size());
	m_TcpBuffer.append(ba);
	m_TcpBufferLock.unlock();
}

void DtMgrClient::Disconnect(QString serverName, quint16 serverPort)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
		m_isConnected = false;
}

// Sends a single poll to the server and then signals UnitSearchResult when a response is received.
void DtMgrClient::FindUnitId(QString serverName, quint16 serverPort, quint32 unitId)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
	{
		m_PollTime.start();
		m_PollAwaitingReply = false;
		m_PollInterval = 1;				// Poll immediately.
		m_PollMode = 2;					// Poll once and signal.
		m_PollUnitId = unitId;			// Unit id to find
	}
}

// Sends a single poll to the server and then signals UnitSearchResult when a response is received.
void DtMgrClient::FindUnitName(QString serverName, quint16 serverPort, QString unitName)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
	{
		m_PollTime.start();
		m_PollAwaitingReply = false;
		m_PollInterval = 1;				// Poll immediately.
		m_PollMode = 3;					// Poll once and quit.
		m_PollUnitName = unitName;		// Unit name to find
	}
}

void DtMgrClient::Probe(QString serverName, quint16 serverPort)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
	{
		m_PollTime.start();
		m_PollAwaitingReply = false;
		m_PollInterval = 1;				// Poll immediately.
		m_PollMode = 1;					// Poll once and quit.
	}
}

void DtMgrClient::StartPolling(int intervalMs, QString serverName, quint16 serverPort)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
	{
		m_PollTime.start();
		m_PollAwaitingReply = false;
		m_PollInterval = intervalMs;	// Poll at interval.
		m_PollMode = 0;					// Poll continuously at intervals.
	}
}

void DtMgrClient::StopPolling(QString serverName, quint16 serverPort)
{
	if(serverName.isEmpty() || (m_ConnectionName == serverName && m_ConnectionPort == serverPort))
	{
		m_PollAwaitingReply = false;
		m_PollInterval = 0;				// No polling.
		m_PollMode = 0;					// So we don't disconnect if an answer to our probe comes in after this.
	}
}

void DtMgrClient::Poll()
{
	m_PollTime.restart();

	quint8 requestWhat;
	if(m_ServerMode == AuGlobal::SceneMaster)
		requestWhat = 3;	// Users
	else
		requestWhat = 2;	// Load values

	// Send status poll packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Status;
	out << requestWhat;
	QueuePacket(ba);
}

void DtMgrClient::ReadStatusOfLoads(QDataStream& in)
{
	AuStatusReport status;
	in >> status.m_AtomManagers;

	// Atoms Managed Status
	quint16 systemsManaged;
	in >> systemsManaged;
	for(int system=0; system<systemsManaged; system++)
	{
		AuStatusAtom atom;
		atom << in;
		status.m_Atoms.append(atom);
	}

	// Connected Clients Status
	quint16 clients;
	in >> clients;
	for(int clientIdx=0; clientIdx<clients; clientIdx++)
	{
		AuStatusClient client;
		client << in;
		status.m_Clients.append(client);
	}

	in >> status.m_AtomLoad;
	in >> status.m_EtheriumLock;
	in >> status.m_BdLock;
	in >> status.m_DataLock;
	in >> status.m_ItLock;
	in >> status.m_ScLock;
	in >> status.m_TrashLock;

	m_StatusReportsLock.lock();
	m_StatusReports.append(status);
	while(m_StatusReports.count() > m_MaxHistory)
		m_StatusReports.removeFirst();
	m_StatusReportsLock.unlock();

	int avgCycleMs = status.AverageCycleMs();	// This is in 1/2 ms increments.

	// With single-poll queries, we signal the result immediately.
	if(m_PollMode == 0)
		emit MonitorAnswered(m_ConnectionName, m_ConnectionPort, avgCycleMs, status.m_AtomLoad, status.m_EtheriumLock,
				status.SystemsDeadlocked(), status.BdLock(), status.DataLock(), status.ItLock(), status.ScLock(),
				status.TrashLock());
	else if(m_PollMode == 1)
		emit ProbeAnswered(m_ConnectionName, m_ConnectionPort, avgCycleMs, status.m_AtomLoad, status.m_EtheriumLock,
				status.SystemsDeadlocked(), status.BdLock(), status.DataLock(), status.ItLock(), status.ScLock(),
				status.TrashLock());
	else if(m_PollMode == 2)
	{
		// Searching for a particular unit id.
		quint32 unitIdFound = 0;
		QString unitNameFound;
		for(QList<AuStatusClient>::const_iterator sc = status.m_Clients.constBegin(); sc != status.m_Clients.constEnd(); sc++)
		{
			if((*sc).m_UnitId == m_PollUnitId)
			{
				unitIdFound = (*sc).m_UnitId;
				unitNameFound = (*sc).m_UnitName;
				break;
			}
		}
		emit UnitSearchResult(m_ConnectionName, m_ConnectionPort, unitIdFound, unitNameFound);
	}
	else if(m_PollMode == 3)
	{
		// Searching for a particular unit name.
		quint32 unitIdFound = 0;
		QString unitNameFound;
		for(QList<AuStatusClient>::const_iterator sc = status.m_Clients.constBegin(); sc != status.m_Clients.constEnd(); sc++)
		{
			if((*sc).m_UnitName == m_PollUnitName)
			{
				unitIdFound = (*sc).m_UnitId;
				unitNameFound = (*sc).m_UnitName;
				break;
			}
		}
		emit UnitSearchResult(m_ConnectionName, m_ConnectionPort, unitIdFound, unitNameFound);
	}
}

void DtMgrClient::ReadStatusOfUsers(QDataStream& in)
{
	AuPlayerOnlineStatusVector playersOn;
	playersOn << in;
	//qDebug() << "DtMgrClient::ReadStatusOfUsers playersOn" << playersOn.size();

	m_PlayersOnLock.lock();
	m_PlayersOn = playersOn;
	m_PlayersOnLock.unlock();

	// With single-poll queries, we signal the result immediately.
	if(m_PollMode == 0)
		emit MonitorAnswered(m_ConnectionName, m_ConnectionPort, 0,0,0,0, false,false,false,false,false);
	else if(m_PollMode == 1)
		emit ProbeAnswered(m_ConnectionName, m_ConnectionPort, 0,0,0,0, false,false,false,false,false);
	else if(m_PollMode == 2)
	{
		// Searching for a particular unit id.
		quint32 unitIdFound = 0;
		QString unitNameFound;
		m_PlayersOnLock.lock();
		for(AuPlayerOnlineStatusVector::const_iterator pi = m_PlayersOn.cbegin(); pi != m_PlayersOn.cend(); pi++)
		{
			if((*pi).m_Id.GetID() == m_PollUnitId)
			{
				unitIdFound = (*pi).m_Id.GetID();
				unitNameFound = (*pi).m_Name;
				break;
			}
		}
		m_PlayersOnLock.unlock();
		emit UnitSearchResult(m_ConnectionName, m_ConnectionPort, unitIdFound, unitNameFound);
	}
	else if(m_PollMode == 3)
	{
		// Searching for a particular unit name.
		quint32 unitIdFound = 0;
		QString unitNameFound;
		m_PlayersOnLock.lock();
		for(AuPlayerOnlineStatusVector::const_iterator pi = m_PlayersOn.cbegin(); pi != m_PlayersOn.cend(); pi++)
		{
			if((*pi).m_Name == m_PollUnitName)
			{
				unitIdFound = (*pi).m_Id.GetID();
				unitNameFound = (*pi).m_Name;
				break;
			}
		}
		m_PlayersOnLock.unlock();
		emit UnitSearchResult(m_ConnectionName, m_ConnectionPort, unitIdFound, unitNameFound);
	}
}

// This reads the report formatted by AuSceneServerApp::ResourceReport.
void DtMgrClient::ReadResourceReportAtom(QDataStream& in)
{
	AuResourceReportAtom report;
	report.m_ServerName = m_ConnectionName;
	report.m_ServerPort = m_ConnectionPort;

	// Specific atom info.
	in >> report.m_AtomId;
	in >> report.m_AtomType;
	if(report.m_AtomType)
	{
		// Server found the atom so more data follows.
		in >> report.m_Name;
		in >> report.m_Desc;
		in >> report.m_SpotDesc;
		in >> report.m_TimeCreated;
		in >> report.m_TimeModified;
		in >> report.m_TimeTicked;
		in >> report.m_TimeTocked;
		in >> report.m_TimeScene;
		in >> report.m_InScene;
		in >> report.m_AtRest;
		in >> report.m_VecX;
		in >> report.m_VecY;
		in >> report.m_VecZ;

		// Atoms
		in >> report.m_dead;
		in >> report.m_dying;
		in >> report.m_travel;
		in >> report.m_atomsInMotion;
		in >> report.m_peopleOnLine;

		quint8 n;
		in >> n;
		while(n--)
		{
			quint8 atomType;
			quint32 atomCount;
			in >> atomType;
			in >> atomCount;
			report.m_Atoms.insert((DaAtom::ATOM_TYPE)atomType, atomCount);
		}

		// Trash Atoms
		in >> report.m_deadTrash;
		in >> report.m_dyingTrash;
		in >> report.m_travelTrash;
		in >> n;
		while(n--)
		{
			quint8 atomType;
			quint32 atomCount;
			in >> atomType;
			in >> atomCount;
			report.m_Trash.insert((DaAtom::ATOM_TYPE)atomType, atomCount);
		}

		quint16 tickers;
		in >> tickers;
		while(tickers--)
		{
			AuThinkerReport ticker;

			in >> ticker.atomId;
			in >> ticker.avgThinkTime;
			in >> ticker.maxThinkTime;

			report.m_tickers.append(ticker);
		}

		quint16 tockers;
		in >> tockers;
		while(tockers--)
		{
			AuThinkerReport tocker;

			in >> tocker.atomId;
			in >> tocker.avgThinkTime;
			in >> tocker.maxThinkTime;

			report.m_tockers.append(tocker);
		}
	}

	emit ResourceReportAtom(report);
}

// This reads the report formatted by AuSceneServerApp::ResourceReport.
void DtMgrClient::ReadResourceReport(QDataStream& in)
{
	AuResourceReport report;
	report.m_ServerName = m_ConnectionName;
	report.m_ServerPort = m_ConnectionPort;

	// Atoms
	in >> report.m_dead;
	in >> report.m_dying;
	in >> report.m_travel;
	in >> report.m_atomsInMotion;
	in >> report.m_peopleOnLine;

	quint8 n;
	in >> n;
	while(n--)
	{
		quint8 atomType;
		quint32 atomCount;
		in >> atomType;
		in >> atomCount;
		report.m_Atoms.insert((DaAtom::ATOM_TYPE)atomType, atomCount);
	}

	// Trash Atoms
	in >> report.m_deadTrash;
	in >> report.m_dyingTrash;
	in >> report.m_travelTrash;
	in >> n;
	while(n--)
	{
		quint8 atomType;
		quint32 atomCount;
		in >> atomType;
		in >> atomCount;
		report.m_Trash.insert((DaAtom::ATOM_TYPE)atomType, atomCount);
	}

	quint16 tickers;
	in >> tickers;
	while(tickers--)
	{
		AuThinkerReport ticker;

		in >> ticker.atomId;
		in >> ticker.avgThinkTime;
		in >> ticker.maxThinkTime;

		report.m_tickers.append(ticker);
	}

	quint16 tockers;
	in >> tockers;
	while(tockers--)
	{
		AuThinkerReport tocker;

		in >> tocker.atomId;
		in >> tocker.avgThinkTime;
		in >> tocker.maxThinkTime;

		report.m_tockers.append(tocker);
	}

	// Model Cache
	in >> report.m_hasCache;
	if(report.m_hasCache)
	{
		quint32 models;
		in >> models;
		for(unsigned int modelIdx=0; modelIdx<models; modelIdx++)
		{
			AuResourceModel model;
			model << in;
			report.m_modelCache.append(model);
		}
	}

	quint16 sccount;
	in >> sccount;
	while(sccount--)
	{
		AuResourceDesign scd;
		scd << in;
		report.m_ScDesigns.append(scd);
	}

	quint16 bdcount;
	in >> bdcount;
	while(bdcount--)
	{
		AuResourceDesign bdd;
		bdd << in;
		report.m_BdDesigns.append(bdd);
	}

	emit ResourceReport(report);
}

void DtMgrClient::SetMaxHistory(int maxHistory)
{
	m_MaxHistory = maxHistory;
}

void DtMgrClient::KickUnit(QString serverName, quint16 serverPort, quint32 unitId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	// Send kick player packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_KickUnit;
	out << unitId;
	QueuePacket(ba);
}

void DtMgrClient::RestoreEmpire(QString serverName, quint16 serverPort, DgEmpire empire)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.
	if(!empire.m_id)
		return;

	// Send packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_RestoreEmpire;
	empire >> out;
	QueuePacket(ba);
}

void DtMgrClient::LimboRescue(QString serverName, quint16 serverPort, quint32 atomId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	// Send packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_LimboRescue;
	out << atomId;
	QueuePacket(ba);
}

void DtMgrClient::LimboRescueList(QString serverName, quint16 serverPort, QList<quint32> atomIds)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	//qDebug() << "LimboRescueList" << atomIds.count() << "atom ids";
	for(QList<quint32>::const_iterator li = atomIds.constBegin(); li != atomIds.constEnd(); li++)
	{
		// Send packet.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtMgrChannelPacket::P_LimboRescue;
		out << *li;
		QueuePacket(ba);
	}
}

void DtMgrClient::DebugSwitch(QString serverName, quint16 serverPort, quint8 specificDebug, quint8 value, quint32 atomId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Debug;
	out << specificDebug;
	out << value;
	out << atomId;
	QueuePacket(ba);
}

void DtMgrClient::DebugMessages(QString serverName, quint16 serverPort, quint32 debugMsgBits)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_DebugMessages;
	out << debugMsgBits;
	QueuePacket(ba);
}

void DtMgrClient::RequestMemoryUsage(QString serverName, quint16 serverPort)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_MemoryUsage;
	QueuePacket(ba);
}

void DtMgrClient::RequestResourceReport(QString serverName, quint16 serverPort, quint8 specificReport, quint32 atomId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Resources;
	out << specificReport;
	out << atomId;
	QueuePacket(ba);
}

void DtMgrClient::RequestReloadData(QString serverName, quint16 serverPort)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_ReloadData;
	QueuePacket(ba);
}

void DtMgrClient::Shutdown(QString serverName, quint16 serverPort, QString warnMsg, QString haltMsg, qint16 shutdownSecs)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Shutdown;
	out << warnMsg;
	out << haltMsg;
	out << shutdownSecs;
	QueuePacket(ba);
}

void DtMgrClient::ShutdownHalt(QString serverName, quint16 serverPort, QString haltMsg)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Shutdown;
	out << QString("");
	out << haltMsg;
	out << (qint16)-1;
	QueuePacket(ba);
}

void DtMgrClient::FinalizeSystem(QString serverName, quint16 serverPort, quint32 atomId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_FinalizeSystem;
	out << atomId;
	QueuePacket(ba);
}

void DtMgrClient::ReassignAtom(QString mgrServerName, quint16 mgrServerPort, quint32 atomId, QString wrkServerName,
		quint16 wrkServerPort)
{
	if(m_ConnectionName != mgrServerName || m_ConnectionPort != mgrServerPort)
		return;	// Not for me.

	//qDebug() << "DtMgrClient::ReassignAtom" << mgrServerName << mgrServerPort << atomId << wrkServerName << wrkServerPort;
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Reassign;
	out << atomId;
	out << wrkServerName;
	out << wrkServerPort;
	QueuePacket(ba);
}

void DtMgrClient::RevokeAtom(QString serverName, quint16 serverPort, quint32 atomId)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Revoke;
	out << atomId;
	QueuePacket(ba);
}

void DtMgrClient::SetAtomModified(QString serverName, quint16 serverPort, quint32 atomId, bool recursive)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_SetModified;
	out << atomId;
	out << recursive;
	QueuePacket(ba);
}

void DtMgrClient::SetControlDecay(QString serverName, quint16 serverPort, quint16 decayMs)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	// Send kick player packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Setting;
	out << (quint8)0;											// Control Decay Margin
	out << decayMs;
	QueuePacket(ba);
}

void DtMgrClient::SetUdpDecay(QString serverName, quint16 serverPort, quint16 decayMs)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	// Send kick player packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Setting;
	out << (quint8)1;											// Udp Decay Margin
	out << decayMs;
	QueuePacket(ba);
}

void DtMgrClient::CreateSectorAt(QString serverName, quint16 serverPort, quint8 galaxy, double sectorX, double sectorY,
		double sectorZ)
{
	if(m_ConnectionName != serverName || m_ConnectionPort != serverPort)
		return;	// Not for me.

	AuPoint sectorLocPc(sectorX, sectorY, sectorZ);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_CreateSectorAt;
	out << galaxy;
	out << sectorLocPc;
	QueuePacket(ba);
}

void DtMgrClient::ReloadScript(QString mgrServerName, quint16 mgrServerPort, quint32 scriptId)
{
	if(m_ConnectionName != mgrServerName || m_ConnectionPort != mgrServerPort)
		return;	// Not for me.

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_ReloadScript;
	out << scriptId;
	QueuePacket(ba);
}

void DtMgrClient::Broadcast(QString mgrServerName, quint16 mgrServerPort, QString message)
{
	if(m_ConnectionName != mgrServerName || m_ConnectionPort != mgrServerPort)
		return;	// Not for me.
	if(message.isEmpty())
		return;

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Announcement;
	out << message;
	QueuePacket(ba);
}

