//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogincomm.cpp,v 1.66 2022/04/07 22:43:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuLoginComm - Object to perform actual login communications. Can be used
//   without a widget.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#include "aulogincomm.h"
//
#include "aucrypt.h"
#include "audna.h"
#include "auglobal.h"
#include "dgdata.h"
#include "dhdna.h"
#include "dtloginserver.h"
#include "hashpack.h"
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTcpSocket>

AuLoginComm::AuLoginComm(QObject* parent) : QObject(parent)
{
	m_AccountId = 0;
	m_LoginServerPort = 0;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_Result = 1;		// Start with error code until we succeed.
}

int AuLoginComm::Login(const QString& acctName, const QByteArray& acctPassword)
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: Login '%1' passcrypt %2")
				.arg(acctName)
				.arg(acctPassword.isEmpty()));

	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: Login connecting to login server %1")
					.arg(m_LoginServerName));
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: Login connected to login server"));
			g_Au.SetNet3Uploading(100);
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
			out << (quint8)DtLoginServer::P_Login4;
			QString ver(AuUtil::LoginVersion());
			out << ver;
			out << acctNameE;
			out << acctPassword;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
		else
		{
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: Login failed to connect to login server"));
			m_Result = 1;
		}
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: Login result %1").arg(m_Result));
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
	return m_Result;
}

//static
// Ask for the list of contacts (buddies and squelchies)
int AuLoginComm::RequestContacts(const QString& acctName, const QByteArray& acctPassword)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Encrypt account name and password.
			short int wKey = AuUtil::NetCryptKey(0x0202);
			AuCrypt acctNameE;
			acctNameE.EncryptString(acctName, wKey);

			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_ContactsQuery;
			out << acctNameE;
			out << acctPassword;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
	return m_Result;
}

void AuLoginComm::RxData(QTcpSocket& tcpSocket)
{
	while(tcpSocket.bytesAvailable())
	{
		g_Au.SetNet3Downloading(100);
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

void AuLoginComm::ReadPacket(const QByteArray& ba)
{
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	quint8 ptype;
	block >> ptype;
	switch((DtLoginServer::LOGIN_PACKET)ptype)
	{
	case DtLoginServer::P_Login4:			// Login result good.
		{
			int confirmNum;
			quint8 acctType;
			AuTime acctExpiry;

			block >> confirmNum;
			block >> m_AccountId;
			block >> acctType;

			AuId idUnit[6];
			QString name[6];
			const AuDNABase* dna[6];
			AuDNABase::CHROMATID chroma[6];
			HANDEDNESS hand[6];
			COMMODITYID clothes[6];
			COMMODITYID armor[6];
			COMMODITYID hat[6];
			quint8 visor[6];
			AuHairStyle::STYLE hair[6];
			AuHairStyle::SHAVE shave[6];
			for(int iu=0; iu<6; iu++)
			{
				idUnit[iu] << block;
				block >> name[iu];

				dna[iu] = AuDNABase::ReadDynamic(block);

				quint8 ui8;
				block >> ui8;
				chroma[iu] = ui8 == Male ? AuDNABase::Alpha : AuDNABase::Beta;	// Server sends gender Male|Female. Convert to
																				// chromatid.
				block >> ui8;
				hand[iu] = (HANDEDNESS)ui8;
				block >> ui8;
				hair[iu] = (AuHairStyle::STYLE)ui8;
				block >> ui8;
				shave[iu] = (AuHairStyle::SHAVE)ui8;

				quint16 ui16;
				block >> ui16;
				clothes[iu] = (COMMODITYID)ui16;
				block >> ui16;
				armor[iu] = (COMMODITYID)ui16;
				block >> ui16;
				hat[iu] = (COMMODITYID)ui16;
				block >> visor[iu];
			}
			block >> acctExpiry;

			if(m_AccountId)
			{
				//for(int iu=0; iu<6; iu++)
					//qDebug("%d unit id %d name %s",
							//iu, idUnit[iu].GetID(), (const char*)name[iu].toAscii());
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetClientInfo(confirmNum, m_AccountId, idUnit, name, dna, chroma, hand, clothes, armor, hat, visor, hair, shave,
						(ACCOUNT)acctType, acctExpiry);
				g_Au.UnlockData();
				m_Result = 0;	// Success
			}
			else
			{
				//qDebug("AuLoginComm::ReadPacket received Login ERROR");
				m_Result = 1;	// Error
			}
			for(int iu=0; iu<6; iu++)
				if(dna[iu])
					delete dna[iu];
		}
		break;

	case DtLoginServer::P_LoginTest:		// Login test credentials result good.
		{
			int confirmNum;

			block >> confirmNum;
			block >> m_AccountId;

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
			block >> m_LoginServerName;
			block >> m_LoginServerPort;
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

			//qDebug("AuLoginComm::ReadPacket received AccountNew result %d", result);
			m_Result = result;
		}
		break;

	case DtLoginServer::P_ContactsQuery:	// The buddies/squelchies list
		{
			QByteArray bcomp;
			block >> bcomp;
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,"");
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				QList<AuContact> contacts;
				contacts.clear();
				quint16 count;
				ds >> count;
				for(;count ;count--)
				{
					AuContact contact;
					contact << ds;
					contacts.append(contact);
				}

				DgData& gd = g_Au.LockDataForWrite();
				gd.SetContacts(contacts);
				g_Au.UnlockData();
				m_Result = 0;	// Success
			}
		}
		break;

	case DtLoginServer::P_Ok:
		m_Result = 0;
		break;

	case DtLoginServer::P_AccountEmail:
	case DtLoginServer::P_AccountPassword:
	case DtLoginServer::P_AccountResetPass:
	case DtLoginServer::P_CheckIn:
	case DtLoginServer::P_ContactAdd:	
	case DtLoginServer::P_ContactDelete:	
	case DtLoginServer::P_ContactsAdd:	
	case DtLoginServer::P_ContactsDelete:	
	case DtLoginServer::P_InactiveUnitQuery:
	case DtLoginServer::P_LoginQuery:
	case DtLoginServer::P_LoginQueryUnit:
	case DtLoginServer::P_LoginSteam:
	case DtLoginServer::P_LoginSteamTest:
	case DtLoginServer::P_Logout:	
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
		m_Result = 1;	// Error
		qWarning("AuLoginComm::ReadPacket - ERROR! Unexpected packet type %d",
				ptype);
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm:ReadPacket error unexpected packet %2").arg(ptype));
		break;

#if defined(QT_NO_DEBUG)
	// We want to see warnings in the debug build for enum values we forget
	// to handle.
	default:
		m_Result = 1;	// Error
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm:ReadPacket error unknown packet %2").arg(ptype));
		qWarning("AuLoginComm::ReadPacket - ERROR! UNKNOWN PACKET TYPE %d",
				ptype);
		break;
#endif
	}
}

// Returns: 0=ok, 1=error
int AuLoginComm::Logout()
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Send player id and confirmation number.
			// This makes it more difficult for hackers to randomly log people off.
			const DgData& gd = g_Au.LockDataForRead();
			quint32 idPlayer = gd.PlayerId();
			quint32 confirmNum = gd.ConfirmationNumber();
			g_Au.UnlockData();

			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_Logout;
			out << idPlayer;
			out << confirmNum;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
	return m_Result;
}

void AuLoginComm::SetPlayerUnit(unsigned int idPlayer, unsigned int confirmNum, const AuId& idUnit)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Send player id and confirmation number.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_Unit4;
			out << idPlayer;
			out << confirmNum;
			out << idUnit;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

void AuLoginComm::PlayerUnitBorn(const AuId& motherUnit, const AuId& idUnit)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_UnitBorn4;
			out << motherUnit;
			out << idUnit;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

void AuLoginComm::PlayerUnitDied(const AuId& avatar)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_UnitDied;
			out << avatar;
			out << AuId::Haxus();				// A little weak verification for the server.
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

void AuLoginComm::DeletePlayerUnit(unsigned int idPlayer, unsigned int confirmNum, const AuId& idUnit)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Send player id and confirmation number.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_UnitDelete;
			out << idPlayer;
			out << confirmNum;
			idUnit >> out;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

void AuLoginComm::UpdateContacts(const QString& acctName, const QByteArray& acctPassword, QList<AuContact> &contacts, bool bDelete)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Encrypt account name and password.
			short int wKey = AuUtil::NetCryptKey(0x0303);
			AuCrypt acctNameE;
			acctNameE.EncryptString(acctName, wKey);

			// Send player id and confirmation number.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			if(bDelete)
				out << (quint8)DtLoginServer::P_ContactsDelete;
			else
				out << (quint8)DtLoginServer::P_ContactsAdd;
			out << acctNameE;
			out << acctPassword;

			// Compress the list
			{
				QByteArray bac;
				QDataStream ds(&bac, QIODevice::WriteOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				quint16 count = contacts.count();
				ds << count;
				for(QList<AuContact>::const_iterator it = contacts.constBegin(); it != contacts.constEnd(); it++)
					(*it) >> ds;
				QByteArray bcomp = qCompress(bac);
				out << bcomp;
			}
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

// most likely this will get called the most
// add/delete a single contact from the contacts list
void AuLoginComm::UpdateContact(const QString& acctName, const QByteArray& acctPassword, AuContact& contact, bool bDelete)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// Encrypt account name and password.
			short int wKey = AuUtil::NetCryptKey(0x0404);
			AuCrypt acctNameE;
			acctNameE.EncryptString(acctName, wKey);

			// Send player id and confirmation number.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			if(bDelete)
				out << (quint8)DtLoginServer::P_ContactDelete;
			else
				out << (quint8)DtLoginServer::P_ContactAdd;
			out << acctNameE;
			out << acctPassword;

			if(bDelete)
				out << contact.m_id;	// only send the id if we are deleting
			else
				contact >> out;

			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

void AuLoginComm::TransferPlayerUnit(const AuId& avatar, const AuId& accepter)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_LoginServerName = g_Au.LoginMasterName();
	m_LoginServerPort = AuUtil::LoginMasterPort();

	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_UnitTransfer;
			out << avatar;
			out << accepter;
			out << AuId::Haxus();				// A little weak verification for the server.
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
}

//static
void AuLoginComm::QueryAvatars(AuIdList& activeIds, AuIdList& inactiveIds)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	QString loginServerName = g_Au.LoginMasterName();
	quint16 loginPort = AuUtil::LoginMasterPort();

	// This will be our return result.
	bool deferred;
	do
	{
		// Connect to the login server.
		deferred = false;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(loginServerName, loginPort);
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_QueryAvatars;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() == QAbstractSocket::ConnectedState || tcpSocket.bytesAvailable())
			{
				// Pull in any data that is waiting for us.
				while(tcpSocket.bytesAvailable())
				{
					g_Au.SetNet3Downloading(100);
					if(!packetSize || packetSizeBytesLeft)
					{
						AuUtil::ReadPacketSize(tcpSocket, packetSize, packetSizeBytesLeft);
						if(packetSizeBytesLeft)
							break;					// We don't have all of the packet size bytes yet.
					}

					// Wait until all of the bytes arrive.
					if(tcpSocket.bytesAvailable() < packetSize)
						break;

					// It is now safe to read our data packet from the socket buffer.
					QByteArray ba = tcpSocket.read(packetSize);
					packetSize = 0;

					// Read the contents of the packet.
					QDataStream block(ba);
					block.setVersion(QDataStream::Qt_4_1);

					// The first byte is always the packet type.
					quint8 ptype;
					block >> ptype;
					switch(ptype)
					{
					case DtLoginServer::P_QueryAvatars:
						{
							// The server responds with one or more of these packets. The number of ids in each packet does not
							// exceed 64k. The server sends as many of these packets as necessary to get me all the active ids.
							quint16 idsInPacket;
							AuId idu;

							block >> idsInPacket;

							while(idsInPacket--)
							{
								block >> idu;
								activeIds.append(idu);
							}
						}
						break;

					case DtLoginServer::P_InactiveUnitQuery:
						{
							// The server responds with one or more of these packets. The number of ids in each packet does not
							// exceed 64k. The server sends as many of these packets as necessary to get me all the inactive ids.
							quint16 idsInPacket;
							AuId idu;

							block >> idsInPacket;

							while(idsInPacket--)
							{
								block >> idu;
								inactiveIds.append(idu);
							}
						}
						break;

					case DtLoginServer::P_Deferred:
						{
							block >> loginServerName;
							block >> loginPort;
							deferred = true;
						}
						break;

					case DtLoginServer::P_DeferFailed:
						// Server tried to defer us but failed. We're done now.
						break;

					default:
						qWarning("HACKER WARNING! AuLoginComm::QueryInactiveUnits Unexpected packet type %d", ptype);
						break;
					}
				}

				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(100);
			}

			// Close the tcp connection.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10000);
			}
		}
	}
	while(deferred);
}

//static
void AuLoginComm::ListPlayerUnits(unsigned int idPlayer, unsigned int confirmNum, AuId unitIds[6], QString unitNames[6])
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	QString loginServerName = g_Au.LoginMasterName();
	quint16 loginPort = AuUtil::LoginMasterPort();

	// This will be our return result.
	bool deferred;
	do
	{
		// Connect to the login server.
		deferred = false;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(loginServerName, loginPort);
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet3Uploading(100);
			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_QueryAccountAvatars;
			out << (quint32)idPlayer;
			out << (quint32)confirmNum;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() == QAbstractSocket::ConnectedState || tcpSocket.bytesAvailable())
			{
				// Pull in any data that is waiting for us.
				while(tcpSocket.bytesAvailable())
				{
					g_Au.SetNet3Downloading(100);
					if(!packetSize || packetSizeBytesLeft)
					{
						AuUtil::ReadPacketSize(tcpSocket, packetSize, packetSizeBytesLeft);
						if(packetSizeBytesLeft)
							break;					// We don't have all of the packet size bytes yet.
					}

					// Wait until all of the bytes arrive.
					if(tcpSocket.bytesAvailable() < packetSize)
						break;

					// It is now safe to read our data packet from the socket buffer.
					QByteArray ba = tcpSocket.read(packetSize);
					packetSize = 0;

					// Read the contents of the packet.
					QDataStream block(ba);
					block.setVersion(QDataStream::Qt_4_1);

					// The first byte is always the packet type.
					quint8 ptype;
					block >> ptype;
					switch(ptype)
					{
					case DtLoginServer::P_QueryAccountAvatars:
						{
							// The server responds with one or more of these packets. The number of ids in each packet does not
							// exceed 64k. The server sends as many of these packets as necessary to get me all the active ids.
							for(int ia=0; ia<6; ia++)
							{
								block >> unitIds[ia];
								block >> unitNames[ia];
							}
							//qDebug() << "AuLoginComm::QueryAccountAvatars" << idPlayer << confirmNum
								//<< unitIds[0] << unitNames[0]
								//<< unitIds[1] << unitNames[1]
								//<< unitIds[2] << unitNames[2]
								//<< unitIds[3] << unitNames[3]
								//<< unitIds[4] << unitNames[4]
								//<< unitIds[5] << unitNames[5];
						}
						break;

					case DtLoginServer::P_Deferred:
						{
							block >> loginServerName;
							block >> loginPort;
							deferred = true;
						}
						break;

					case DtLoginServer::P_DeferFailed:
						// Server tried to defer us but failed. We're done now.
						break;

					default:
						qWarning("HACKER WARNING! AuLoginComm::ListPlayerUnits Unexpected packet type %d", ptype);
						break;
					}
				}

				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(100);
			}

			// Close the tcp connection.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10000);
			}
		}
	}
	while(deferred);
}

int AuLoginComm::LoginSteam(const quint8* ticketData, int ticketDataSize)
{
	QByteArray ticket((const char*)ticketData, ticketDataSize);
	return LoginSteam(ticket);
}

int AuLoginComm::LoginSteam(const QByteArray& ticket)
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: LoginSteam %1")
				.arg(ticket.size()));
	m_Result = 1;		// Start with error code until we succeed.
	do
	{
		// Connect to the login server.
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: LoginSteam connecting to login server %1")
					.arg(m_LoginServerName));
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_LoginServerName, m_LoginServerPort);
		tcpSocket.waitForConnected();
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: LoginSteam connected to login server"));
			g_Au.SetNet3Uploading(100);
			// Encrypt ticket even though it is already encrypted.
			short int wKey = AuUtil::NetCryptKey(0x0101);
			AuCrypt ticketE;
			ticketE.EncryptByteArray(ticket, wKey);

			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_LoginSteam;
			QString ver(AuUtil::LoginVersion());
			out << ver;
			out << ticketE;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();

			// The server will send back a reply, good or bad, then it will disconnect.
			m_PacketSize = 0;
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
		else
		{
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: LoginSteam failed to connect to login server"));
			m_Result = 1;
		}
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("AuLoginComm: LoginSteam result %1").arg(m_Result));
	}
	while(m_Result == 4);	// Try again when deferred to another login server.
	return m_Result;
}
