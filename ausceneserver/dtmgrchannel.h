//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmgrchannel.h,v 1.25 2022/02/19 18:39:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtMgrChannel_H
#define DtMgrChannel_H

#include "auid.h"
#include "autime.h"
#include "dgempire.h"
#include <QMutex>
#include <QTcpSocket>
#include <QThread>

class DtMgrChannel : public QThread
{
	Q_OBJECT;
public:
	DtMgrChannel(int socketDescriptor, QObject* parent=0);

	void SendPacket(const QByteArray& ba);					// Adds a packet to the transmit buffer.

signals:
	void error(QTcpSocket::SocketError socketError);
	void CreateSectorAt(quint8 galaxyEnum, double sectorX, double sectorY, double sectorZ);
	void DisconnectUnit(AuId unitId);
	void FinalizeSystem(AuId systemId);
	void LimboRescue(AuId atomId);
	void ReassignSystem(AuId systemId, QString workerServer, quint16 workerPort);
	void ReloadData();
	void ReloadScript(quint32 scriptId);	// 0 reloads all scripts
	void RestoreEmpire(DgEmpire empire);
	void RevokeAssignment(AuId atomId);
	void SendMemoryUsage();
	void SendResources(quint8 specificResources, AuId atomId);
	void SendStatus(quint8 specificStatus);
	void ServerBroadcast(QString text);
	void SetAtomModified(AuId atomId, bool recursive);
	void Shutdown(QString warnMsg, QString haltMsg, qint16 shutdownSecs);

public slots:
	void MgrSendPacket(QByteArray pkt);
	
protected:
	virtual void run();

	void Disconnect();

	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);

	void RxData(QTcpSocket& tcpSocket);

private:
	bool			m_HasConnected;		// true if we've received a valid P_Connect packet.
	Qt::HANDLE		m_ThreadId;

	// Comm
	unsigned int	m_BadPackets;		// # of packets rx'd with unknown packet type
	unsigned int	m_BytesRx;			// Total bytes received from client
	unsigned int	m_BytesTx;			// Total bytes sent to client
	bool			m_Disconnect;		// true when time to disconnect.
	quint32			m_PacketSize;		// Data packet size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int				m_socketDescriptor;	// Provided in the constructor.
	AuTime			m_tmConnect;		// Time connection is established.
	QByteArray		m_TxBuffer;			// On the client, the tx buffer is a property of AuApp.
	QMutex			m_muTxBuffer;		// Guards m_TxBuffer.
};

#endif // DtMgrChannel_H
