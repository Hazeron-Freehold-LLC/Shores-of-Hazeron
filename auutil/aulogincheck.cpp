//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogincheck.cpp,v 1.15 2022/04/07 22:32:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulogincheck.h"
//
#include "aucrypt.h"
#include "audef.h"
#include "dtloginserver.h"
#include <QTcpSocket>

AuLoginCheck::AuLoginCheck(QObject* parent) : QObject(parent)
{
	m_AccountId = 0;
	m_LoginPort = AuUtil::LoginMasterPort();
	m_LoginServer = AuUtil::LoginMasterName();
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_Result = 1;		// Start with error code until we succeed.
}

int AuLoginCheck::LoginTestCredentials(const QString& acctName, const QByteArray& acctPassword)
{
	m_LoginPort = AuUtil::LoginMasterPort();
	m_LoginServer = AuUtil::LoginMasterName();

	//qDebug() << "AuLoginCheck::LoginTest" << acctName << "pass" << acctPassword;
	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		//qDebug() << "AuLoginCheck::LoginTest connnecting" << m_LoginServer << m_LoginPort;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServer, m_LoginPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			//qDebug() << "AuLoginCheck::LoginTest connnected" << acctName;
			// Encrypt account name and password.
			short int wKey = AuUtil::NetCryptKey(0x0101);
			AuCrypt acctNameE;
			acctNameE.EncryptString(acctName, wKey);

			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_LoginTest;
			QString ver(AuUtil::LoginVersion());
			out << ver;
			out << acctNameE;
			out << acctPassword;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
			m_PacketSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead();
			while(tcpSocket.state() == QAbstractSocket::ConnectedState ||
					tcpSocket.bytesAvailable())
			{
				//qDebug() << "AuLoginCheck::LoginTest rxdata" << tcpSocket.bytesAvailable();
				// Pull in any data that is waiting for us.
				RxData(tcpSocket);
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10);
			}

			// Close the tcp connection if it's still open.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				//qDebug() << "AuLoginCheck::LoginTest disconnecting";
				tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected();
			}
		}
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
	return m_Result;
}

void AuLoginCheck::RxData(QTcpSocket& tcpSocket)
{
	while(tcpSocket.bytesAvailable())
	{
		if(!m_PacketSize || m_PacketSizeBytesLeft)
		{
			AuUtil::ReadPacketSize(tcpSocket, m_PacketSize, m_PacketSizeBytesLeft);
			if(m_PacketSizeBytesLeft)
				break;					// We don't have all of the packet size bytes yet.
		}

		// Wait until all of the bytes arrive.
		if(tcpSocket.bytesAvailable() < m_PacketSize)
			break;

		// It is now safe to read our data packet from the socket buffer.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_PacketSize = 0;
		ReadPacket(block);	// Read the contents of the packet.
	}
}

void AuLoginCheck::ReadPacket(const QByteArray& ba)
{
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	quint8 ptype;
	block >> ptype;
	//qDebug() << "AuLoginCheck::ReadPacket" << ptype;
	switch((DtLoginServer::LOGIN_PACKET)ptype)
	{
	case DtLoginServer::P_LoginTest:		// Login test credentials result good.
		{
			int confirmNum;

			block >> confirmNum;
			block >> m_AccountId;

			//qDebug() << "AuLoginCheck::ReadPacket" << m_AccountId;
			if(m_AccountId)
			{
				m_Result = 0;	// Success
			}
			else
			{
				m_Result = 1;	// Error
			}
		}
		break;

	case DtLoginServer::P_LoginFailure:	// Login failure probably bad password.
		m_Result = 2;			// Login Failure
		break;

	case DtLoginServer::P_VersionError:	// Incorrect software version.
		m_Result = 3;			// Wrong Version
		break;

	case DtLoginServer::P_LoginLockedOut:	// Login failure, account locked out.
		m_Result = 6;			// Account locked out
		break;

	case DtLoginServer::P_Deferred:	// Login deferred to another server.
		{
			block >> m_LoginServer;
			block >> m_LoginPort;
			m_Result = 4;			// Login Deferred
		}
		break;

	case DtLoginServer::P_DeferFailed:	// Defer to another server failed.
		m_Result = 1;
		break;

	case DtLoginServer::P_AccountNew:
		{
			quint8 result;

			block >> result;
			block >> m_AccountId;

			//qDebug("AuLoginCheck::ReadPacket received AccountNew result %d", result);
			m_Result = result;
		}
		break;

	case DtLoginServer::P_AccountEmail:
	case DtLoginServer::P_AccountPassword:
	case DtLoginServer::P_AccountResetPass:
	case DtLoginServer::P_CheckIn:
	case DtLoginServer::P_ContactAdd:	
	case DtLoginServer::P_ContactDelete:	
	case DtLoginServer::P_ContactsAdd:	
	case DtLoginServer::P_ContactsDelete:	
	case DtLoginServer::P_ContactsQuery:
	case DtLoginServer::P_InactiveUnitQuery:
	case DtLoginServer::P_Login4:
	case DtLoginServer::P_LoginQuery:
	case DtLoginServer::P_LoginQueryUnit:
	case DtLoginServer::P_LoginSteam:
	case DtLoginServer::P_LoginSteamTest:
	case DtLoginServer::P_Logout:	
	case DtLoginServer::P_Ok:	
	case DtLoginServer::P_QueryAccountAvatars:
	case DtLoginServer::P_QueryAvatars:
	case DtLoginServer::P_StatusQuery:	
	case DtLoginServer::P_Unit4:	
	case DtLoginServer::P_UnitBorn4:	
	case DtLoginServer::P_UnitDelete:	
	case DtLoginServer::P_UnitDied:	
	case DtLoginServer::P_UnitQuery:
	case DtLoginServer::P_UnitTransfer:
		// Client should never get this packet type.
		qWarning("AuLoginCheck::ReadPacket - ERROR! Unexpected packet type %d",
				ptype);
		break;

#if defined(QT_NO_DEBUG)
	// We want to see warnings in the debug build for enum values we forget
	// to handle.
	default:
		qWarning("AuLoginCheck::ReadPacket - ERROR! UNKNOWN PACKET TYPE %d",
				ptype);
		break;
#endif
	}
}

void AuLoginCheck::ChangeEmail(const QString& acctName, const QByteArray& password, const QString& email) const
{
	// Connect to the login server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(m_LoginServer, m_LoginPort);
	tcpSocket.waitForConnected();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// Send player id and confirmation number.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtLoginServer::P_AccountEmail;
		out << acctName;
		out << password;
		out << email;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		// Close the tcp connection.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected();
		}
	}
}

// returns 0=success, 1=error
int AuLoginCheck::CreateAccount(const QString& acctName, const QByteArray& password, const QString& email, ACCOUNT acctType)
{
	//qDebug("AuLoginCheck::CreateAccount '%s' '%s' '%s'",
			//(const char*)acctName.toAscii(),
			//(const char*)password.toAscii(),
			//(const char*)email.toAscii());
#if defined(Free2Play)
	if(acctType != ACCT_Active)
#else
	if(acctType != ACCT_LockedOut && acctType != ACCT_Trial)
#endif
		return 1;
	m_AccountId = 0;
	m_Result = 1;		// Start with error code until we succeed.

	// Connect to the login server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(m_LoginServer, m_LoginPort);
	tcpSocket.waitForConnected();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		//qDebug("AuLoginCheck::CreateAccount connected to %s:%d",
				//(const char*)m_LoginServer, m_LoginPort);
		// The connection attempt succeeded.
		// Transmit the account info to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtLoginServer::P_AccountNew;
		out << acctName;
		out << password;
		out << email;
		out << (quint8)acctType;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		// The server will send back a reply, good or bad, then it will disconnect.
		m_PacketSize = 0;
		m_PacketSizeBytesLeft = 0;
		tcpSocket.waitForReadyRead();
		while(tcpSocket.state() == QAbstractSocket::ConnectedState ||
				tcpSocket.bytesAvailable())
		{
			// Pull in any data that is waiting for us.
			RxData(tcpSocket);
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10);
		}

		// Close the tcp connection if it's still open.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected();
		}
	}
	return m_Result;
}

void AuLoginCheck::ChangePassword(const QString& acctName, const QByteArray& password, const QByteArray& newPassword)
{
	// Connect to the login server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(m_LoginServer, m_LoginPort);
	tcpSocket.waitForConnected();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// Send player id and confirmation number.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtLoginServer::P_AccountPassword;
		out << acctName;
		out << password;
		out << newPassword;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		// Close the tcp connection.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected();
		}
	}
}

void AuLoginCheck::ResetPassword(const QString& acctName)
{
	// Connect to the login server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(m_LoginServer, m_LoginPort);
	tcpSocket.waitForConnected();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// Send player id and confirmation number.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtLoginServer::P_AccountResetPass;
		out << acctName;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		// Close the tcp connection.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected();
		}
	}
}
