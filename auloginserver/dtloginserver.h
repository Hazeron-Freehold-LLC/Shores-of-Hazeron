//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtloginserver.h,v 1.35 2022/04/07 22:25:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DTLOGINSERVER_H
#define DTLOGINSERVER_H

#include <QMutex>
#include <QTcpSocket>
#include <QThread>

class DtLoginServer : public QThread
{
	Q_OBJECT;
public:
	// TCP Packet type values for login communication.
	enum LOGIN_PACKET
	{
		P_CheckIn,			// Sent by LoginServer to LoginMaster periodically.
		P_DeferFailed,		// Defer failure sent by master server to client.
		P_Deferred,			// Server reply to Login - deferred to another servr
		P_Login4,			// Login account info packet.
		P_LoginFailure,		// Server reply to Login/LoginTest - login failure.
		P_LoginQuery,		// Query to server to determine if person logged in. Reply to sender with result.
		P_LoginQueryUnit,	// Query to server to determine if person logged in. Reply to sender with result, including unit ids.
		P_Logout,			// Sent by client to server.
		P_Unit4,			// Sent by client to server with player unit assignment.
		P_VersionError,		// Server reply to Login - bad version.
		P_AccountEmail,		// Sent to worker by client with new acct email.
		P_AccountNew,		// Sent to worker by client with new account info. Sent to client with result code.
		P_AccountPassword,	// Sent to worker by client with new acct password.
		P_AccountResetPass,	// Sent to worker by client requesting password reset and the result sent via e-mail to account address.
		P_LoginTest,		// Login account query packet. Queries like Login but doesn't actually log in.
		P_UnitDelete,		// Sent by client to server with player unit to delete.
		P_StatusQuery,		// Sent by AuServerStatus to check my current status.
		P_ContactsQuery,	// Query for list of contacts (buddies and squelchies)
		P_ContactsAdd,		// Add a list of contacts to the player's contacts list
		P_ContactsDelete,	// Delete a list of contacts from the player's contacts list
		P_ContactAdd,		// Add a single contact
		P_ContactDelete,	// Delete a single contact
		P_LoginLockedOut,	// Server reply to Login - account locked out
		P_UnitBorn4,		// Sent by client to server with new unit that was born in the game.
		P_UnitDied,			// Sent by client to server when demiavatar dies permanently.
		P_UnitQuery,		// Query for list of units owned by a player.
		P_UnitTransfer,		// Sent by client to server to transfer an avatar from one account to another.
		P_InactiveUnitQuery,	// Query for list of units owned by all inactive players.
		P_QueryAvatars,		// Query for lists active and inactive avatars.
		P_QueryAccountAvatars,	// Query for list of avatars associated with a particular account.
		P_LoginSteam,		// Login steam account info packet.
		P_LoginSteamTest,	// Login steam account query packet. Queries like LoginSteam but doesn't actually log in.
		P_Ok,				// empty acknowledgement
	};

public:
	DtLoginServer(unsigned int id, int socketDescriptor, QObject* parent=0);

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

signals:
	void error(QTcpSocket::SocketError socketError);

protected:
	virtual void run();

private:
	void Defer(QTcpSocket& tcpSocket);
	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);

private:
	unsigned int	m_Id;				// Id of this connection instance.

	// Comm
	unsigned int	m_BadPackets;		// # of packets rx'd with bad data
	unsigned int	m_BytesRx;			// Total bytes received from client
	unsigned int	m_BytesTx;			// Total bytes sent to client
	bool			m_LocalClient;		// true if the client appears to be on
										// our LAN
	quint32			m_PacketSize;		// Data packet size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int				m_socketDescriptor;	// Provided in the constructor.
};

#endif // DTLOGINSERVER_H
