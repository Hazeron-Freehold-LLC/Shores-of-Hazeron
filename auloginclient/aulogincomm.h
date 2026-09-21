//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogincomm.h,v 1.35 2022/04/07 22:43:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLoginComm_H
#define AuLoginComm_H

#include "auloginclient.h"
#include <QObject>

class AuId;
class AuIdList;
class QByteArray;
class QTcpSocket;
class AuContact;

class AULOGINCLIENT_DLL AuLoginComm : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(AuLoginComm);

private:
	quint32								m_AccountId;	// Returned if m_Result is 0
	QString								m_LoginServerName;
	quint16								m_LoginServerPort;
	quint32								m_PacketSize;	// TCP block size
	int									m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int									m_Result;		// 0=ok, 1=error, 2=login failure, 3=wrong version,
														// 4=deferred to another server, never returned
														// 5=new acct name already exists
														// 6=account inactive

public:
	AuLoginComm(QObject* parent=0);

	// Only valid after successful Login and successful CreateAccount.
	inline quint32 AccountId() const
	{
		return m_AccountId;
	}

	void DeletePlayerUnit(unsigned int idPlayer, unsigned int confirmNum, const AuId& idUnit);

	static void ListPlayerUnits(unsigned int idPlayer, unsigned int confirmNum, AuId unitIds[6], QString unitNames[6]);

	// A confirmation number is assigned by the login server. This confirmation number is used to verify that
	// the same account is not logged-in multiple times simultaneously to play the game.
	// To test login without assigning a confirmation number, use AuLoginCheck::Login.
	// Returns 0=ok, 1=error, 2=failure, 3=version error, 6=account inactive
	int Login(const QString& acctName, const QByteArray& acctPassword);
	int LoginSteam(const QByteArray& ticket);
	int LoginSteam(const quint8* ticketData, int ticketDataSize);

	int Logout();	// Returns 0=ok, 1=error

	// Adds a new unit to the login account who owns motherUnit. Fails quietly if no free unit slots are available.
	void PlayerUnitBorn(const AuId& motherUnit, const AuId& idUnit);

	// Removes a unit from the owner's login account.
	void PlayerUnitDied(const AuId& avatar);

	// Returns two lists of avatar ids. These lists combined are all avatar ids associated with any login accounts.
	// 'activeIds' returns avatars associated with active accounts.
	// 'inactiveIds' returns avatars associated with inactive accounts.
	// This is used periodically when making demiavatars go dormant who belong to inactive accounts and to reconcile unused units.
	static void QueryAvatars(AuIdList& activeIds, AuIdList& inactiveIds);

	// Ask for the list of contacts (buddies and squelchies)
	// Returns 0=ok, 1=error, 2=failure, 3=version error // not sure yet
	int RequestContacts(const QString& acctName, const QByteArray& password);

	void SetPlayerUnit(unsigned int idPlayer, unsigned int confirmNum, const AuId& idUnit);

	// Transfers an avatar from one login account to another.
	// Removes a unit from the owner's login account and moves it to the account that owns the accepter unit.
	void TransferPlayerUnit(const AuId& avatar, const AuId& accepter);

	void UpdateContact(const QString& acctName, const QByteArray& password, AuContact &Contact, bool bDelete);	// single

	void UpdateContacts(const QString& acctName, const QByteArray& password, QList<AuContact> &contacts, bool bDelete);	// multiple

private:
	void ReadPacket(const QByteArray& ba);
	void RxData(QTcpSocket& tcpSocket);
};

#endif // AuLoginComm_H
