//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmailserver.h,v 1.17 2022/02/19 18:37:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtMailServer_H
#define DtMailServer_H

#include "aumailmsg.h"
#include <QTcpSocket>
#include <QThread>

class QSqlDatabase;

class DtMailServer : public QThread
{
	Q_OBJECT;
private:
	unsigned int						m_BadPackets;			// # of packets rx'd with unknown packet type
	unsigned int						m_BytesRx;				// Total bytes received from client
	unsigned int						m_BytesTx;				// Total bytes sent to client
	quint32								m_idPlayer;
	bool								m_LocalClient;			// true if the client appears to be on our LAN
	quint32								m_PacketSize;			// Data packet size
	int									m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int									m_socketDescriptor;		// Provided in the constructor.

	QMutex								m_MailMsgsLock;
	bool								m_MailMsgsReady;
	AuMailMsgArray						m_MailMsgsToGo;

public:
	// TCP Packet type values for in-game mail communication.
	enum MAIL_PACKET
	{
		P_CheckIn,			// Sent by MailServer to MailMaster periodically.
		P_DeferFailed,		// Defer failure sent by master server to client.
		P_Deferred,			// Server reply deferring client to another server.
		P_Delete,			// Client order to delete message(s).
		P_DoneOk,			// Server good termination of mail packet stream.
		P_LoginError,		// Server reply to mail request due to bad login.
		P_Message,			// Message packet.
		P_Request,			// Client request for mail.
		P_VersionError,		// Server reply to mail request due to bad version.
		P_StatusQuery,		// Sent by AuServerStatus to check my current status.
		P_BulkMessage,		// Message packet with list of ids to distribute the message to
	};

public:
	DtMailServer(int socketDescriptor, QObject* parent=0);

	int BadPacketCount() const
	{
		return m_BadPackets;
	}
	int BytesRx() const
	{
		return m_BytesRx;
	}
	int BytesTx() const
	{
		return m_BytesTx;
	}

	static void CheckIn();

public slots:
	void PlayerMailMessages(quint32 idPlayer, AuMailMsgArray msgs);

signals:
	void DeleteMailMessages(quint32 idPlayer, AuId idAvatar, AuMailMsgIdList delids);
	void MailMessageReceived(quint32 idPlayer, AuMailMsg msg);
	void MailMessageReceivedBulk(quint32 idPlayer, AuMailMsg msg, AuIdList recipients);
	void RequestMailMessages(quint32 idPlayer, AuId idAvatar, quint32 maxMsgId, AuIdList squelchIds, quint8 cityRptAutoDel,
			quint32 cityRptOldAge, quint8 fleetRptAutoDel, quint32 fleetRptOldAge, quint8 officerRptAutoDel,
			quint32 officerRptOldAge);
	void error(QTcpSocket::SocketError socketError);

protected:
	virtual void run();

private:
	void Defer(QTcpSocket& tcpSocket);
	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);
	void RxData(QTcpSocket& tcpSocket);
	void TxData(QTcpSocket& tcpSocket);
};

#endif // DtMailServer_H
