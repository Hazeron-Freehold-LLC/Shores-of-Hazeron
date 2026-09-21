//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulogincheck.h,v 1.8 2022/02/19 18:42:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLoginCheck_H
#define AuLoginCheck_H

#include "audef.h"
#include "auutil.h"
#include <QObject>

class AuId;
class AuIdList;
class DhDNA;
class QByteArray;
class QTcpSocket;
class AuContact;

class AUUTIL_DLL AuLoginCheck : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(AuLoginCheck);
public:
	AuLoginCheck(QObject* parent=0);

	// Only valid after successful Login and successful CreateAccount.
	inline quint32 AccountId() const
	{
		return m_AccountId;
	}

	void ChangeEmail(const QString& acctName, const QByteArray& password, const QString& email) const;

	void ChangePassword(const QString& acctName, const QByteArray& password, const QByteArray& newPassword);

	// 'acctType' must be ACCT_LockedOut or ACCT_Trial (ACCT_Active when Free2Play).
	// returns 0=ok, 1=error, 5=duplicate name
	int CreateAccount(const QString& acctName, const QByteArray& password, const QString& email, ACCOUNT acctType);

	// Only tests the login name and password without assigning a confirmation number, Web cgi programs use this
	// when verifying login information, Otherwise, this would cause a player grief if they were in the game when 
	// the web cgi checked their credentials.
	// Returns 0=ok, 1=error, 2=failure, 3=version error, 6=account inactive
	int LoginTestCredentials(const QString& acctName, const QByteArray& acctPassword);

	void ResetPassword(const QString& acctName);

private:
	void ReadPacket(const QByteArray& ba);
	void RxData(QTcpSocket& tcpSocket);

private:
	quint32			m_AccountId;	// Returned if m_Result is 0
	quint16			m_LoginPort;
	QString			m_LoginServer;
	quint32			m_PacketSize;	// TCP block size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int				m_Result;		// 0=ok, 1=error, 2=login failure, 3=wrong version,
									// 4=deferred to another server, never returned
									// 5=new acct name already exists
									// 6=account inactive
};

#endif // AuLoginCheck_H
