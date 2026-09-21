//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloginserverapp.h,v 1.60 2022/04/15 20:44:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLoginServerApp_H
#define AuLoginServerApp_H

#include "aucontact.h"
#include "audef.h"
#include "auid.h"
#include "auloginserver.h"
#include "autime.h"
#include "dbcommodity.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QMap>
#include <QMutex>
#if defined(AUSTEAM)
#include <steam_api.h>
#endif	// AUSTEAM

class AuDNABase;
class DtLoginDb;
class QSqlDatabase;
class QTimer;

class AuLoginAttempt
{
public:
	enum RESULT
	{
		Success,
		Failure,
		Pending,					// Pending processing by db thread.
		LockedOut,					// Login failed because account is locked out.
	};

public:
	AuTime								m_AccountExpiry;
	ACCOUNT								m_AccountType;			// Result on Success.
	QString								m_ClientName;			// Client ip name making request.
	quint32								m_ConfirmNum;			// Result on Success
	QString								m_Name;
	QByteArray							m_PassCrypt;			// SHA-1 hash of password
	quint32								m_PlayerId;				// Result on Success
	RESULT								m_Result;
	unsigned int						m_ServerId;				// DtLoginServer id making request.
	bool								m_TestCredentialsOnly;	// Don't login; just test password.
	COMMODITYID							m_UnitArmor[6];			// valid if result is LoggedIn.
	COMMODITYID							m_UnitClothes[6];		// valid if result is LoggedIn.
	AuDNABase*							m_UnitDNA[6];			// Valid if result is LoggedIn. Could be 0.
	GENDER								m_UnitGender[6];		// valid if result is LoggedIn. This is actually the literal content
																// of the a_Person.sex field, which is now the converse of their
																// chromatid, for backward compatibility with Male/Female data.
	HANDEDNESS							m_UnitHand[6];			// valid if result is LoggedIn.
	quint8								m_UnitHair[6];			// valid if result is LoggedIn.
	COMMODITYID							m_UnitHat[6];			// valid if result is LoggedIn.
	AuId								m_UnitId[6];			// Valid if result is LoggedIn.
	QString								m_UnitName[6];			// Valid if result is LoggedIn.
	quint8								m_UnitShave[6];			// valid if result is LoggedIn.
	quint8								m_UnitVisor[6];			// Item State() of the helmet, if any.

public:
	AuLoginAttempt()
	{
		m_AccountType = ACCT_Count;
		m_ConfirmNum = 0;
		m_PlayerId = 0;
		m_Result = Pending;
		m_ServerId = 0;
		m_TestCredentialsOnly = true;
		for(int u=0; u<6; u++)
			m_UnitDNA[u] = 0;
	}

	AuLoginAttempt(unsigned int serverId, const QString& clientName,
			const QString& name, const QByteArray& passCrypt, bool testCredentialsOnly)
	{
		m_AccountType = ACCT_Count;
		m_ClientName = clientName;
		m_ConfirmNum = 0;
		m_Name = name;
		m_PassCrypt = passCrypt;
		m_PlayerId = 0;
		m_Result = Pending;
		m_ServerId = serverId;
		m_TestCredentialsOnly = testCredentialsOnly;
		for(int u=0; u<6; u++)
			m_UnitDNA[u] = 0;
	}

	~AuLoginAttempt();

	inline bool IsResolved() const
	{
		return m_Result != Pending ? true : false;
	}

	QString AccountTypeName() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#if defined(AUSTEAM)
class AuLoginSteamAttempt
{
public:
	enum RESULT
	{
		Success,
		Failure,
		Pending,					// Pending processing by db thread.
		Disabled,					// Login failed because account is disabled/inactive.
		BadKey,						// SteamEncryptedAppTicket_BDecryptTicket failed.
		BadKeyApp,					// SteamEncryptedAppTicket_BIsTicketForApp failed.
		BadKeyOld,					// Steam key is too old.
	};

public:
	AuTime								m_AccountExpiry;
	ACCOUNT								m_AccountType;			// Result on Success.
	QString								m_ClientName;			// Client ip name making request.
	quint32								m_ConfirmNum;			// Result on Success
	CSteamID							m_SteamId;
	quint32								m_PlayerId;				// Result on Success
	RESULT								m_Result;
	unsigned int						m_ServerId;				// DtLoginServer id making request.
	bool								m_TestCredentialsOnly;	// Don't login; just test password.
	COMMODITYID							m_UnitArmor[6];			// valid if result is LoggedIn.
	COMMODITYID							m_UnitClothes[6];		// valid if result is LoggedIn.
	AuDNABase*							m_UnitDNA[6];			// Valid if result is LoggedIn. Could be 0.
	GENDER								m_UnitGender[6];		// valid if result is LoggedIn. This is actually the literal content
																// of the a_Person.sex field, which is now the converse of their
																// chromatid, for backward compatibility with Male/Female data.
	HANDEDNESS							m_UnitHand[6];			// valid if result is LoggedIn.
	quint8								m_UnitHair[6];			// valid if result is LoggedIn.
	COMMODITYID							m_UnitHat[6];			// valid if result is LoggedIn.
	AuId								m_UnitId[6];			// Valid if result is LoggedIn.
	QString								m_UnitName[6];			// Valid if result is LoggedIn.
	quint8								m_UnitShave[6];			// valid if result is LoggedIn.
	quint8								m_UnitVisor[6];			// Item State() of the helmet, if any.

public:
	AuLoginSteamAttempt()
	{
		m_AccountType = ACCT_Count;
		m_ConfirmNum = 0;
		m_PlayerId = 0;
		m_Result = Pending;
		m_ServerId = 0;
		m_TestCredentialsOnly = true;
		for(int u=0; u<6; u++)
			m_UnitDNA[u] = 0;
	}

	AuLoginSteamAttempt(unsigned int serverId, const QString& clientName, const QByteArray& ticket, bool testCredentialsOnly);

	~AuLoginSteamAttempt();

	inline bool IsResolved() const
	{
		return m_Result != Pending ? true : false;
	}

	QString AccountTypeName() const;

	QString Name() const;
};
#endif	// AUSTEAM

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuLogoutAttempt
{
public:
	QString			m_ClientName;		// Client ip name making request.
	quint32			m_ConfirmNum;		// Confirmation number with request.
	quint32			m_PlayerId;			// Player id making the request.

public:
	AuLogoutAttempt()
	{
	}

	AuLogoutAttempt(quint32 playerId, quint32 confirmNum,
			const QString& clientName)
	{
		m_ClientName = clientName;
		m_ConfirmNum = confirmNum;
		m_PlayerId = playerId;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuLoginContacts
{
public:
	enum RESULT
	{
		Success,
		Failure,
		Pending,					// Pending processing by db thread.
	};

public:
	RESULT			m_Result;			// Result of db query.
	unsigned int	m_ServerId;			// DtLoginServer id making request.
	QString			m_ClientName;		// Client ip name making request.
	QString			m_Name;
	QByteArray		m_PassCrypt;			// SHA-1 hash key

	// one of these three will be valid but not more than one
	QList<AuContact>	m_Contacts;		// the list of contacts filled to the brim...to either replace, add or delete
	AuContact		m_Contact;			// or the single contact to add
	AuId			m_ContactId;		// or the single id to delete

public:
	AuLoginContacts()
	{
		m_ServerId = 0;
		m_Result = Pending;
	}

	AuLoginContacts(unsigned int serverId, const QString& clientName,
					const QString& name, const QByteArray& passCrypt)
	{
		m_ServerId = serverId;
		m_Result = Pending;
		m_ClientName = clientName;
		m_Name = name;
		m_PassCrypt = passCrypt;
	}
	
	AuLoginContacts(unsigned int serverId, const QString& clientName,
					const QString& name, const QByteArray& passCrypt, const QList<AuContact>* contacts)
	{
		m_ServerId = serverId;
		m_Result = Pending;
		m_ClientName = clientName;
		m_Name = name;
		m_PassCrypt = passCrypt;
		if(contacts)
			m_Contacts.append(*contacts);
	}
	
	AuLoginContacts(unsigned int serverId, const QString& clientName,
					const QString& name, const QByteArray& passCrypt, const AuContact* contact)
	{
		m_ServerId = serverId;
		m_Result = Pending;
		m_ClientName = clientName;
		m_Name = name;
		m_PassCrypt = passCrypt;
		if(contact)
			m_Contact = *contact;
	}
	
	AuLoginContacts(unsigned int serverId, const QString& clientName, 
					const QString& name, const QByteArray& passCrypt, const AuId& contactId)
	{
		m_ServerId = serverId;
		m_Result = Pending;
		m_ClientName = clientName;
		m_Name = name;
		m_PassCrypt = passCrypt;
		m_ContactId = contactId;
	}
	
	inline bool IsResolved() const
	{
		return m_Result != Pending ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuLoginQuery
{
public:
	enum RESULT
	{
		Pending,		// Pending processing by db thread.
		BadLogin,		// Invalid player id or confirmation number.
		LoggedIn,		// Player is logged in and confirm is valid.
		NotLoggedIn,	// Player is not logged in.
	};

public:
	AuTime								m_AccountExpiry;
	ACCOUNT								m_AccountType;		// Result on Success.
	QString								m_ClientName;		// Client ip name making request.
	quint32								m_ConfirmNum;		// Confirmation number with request.
	bool								m_IsUnitQuery;		// True if unitId and canCreate wanted.
	quint32								m_PlayerId;			// Player id making the request.
	RESULT								m_Result;			// Result of db query.
	unsigned int						m_ServerId;			// DtLoginServer id making request.
	AuId								m_UnitId[6];		// Valid if result is LoggedIn.

	AuLoginQuery()
	{
		m_AccountType = ACCT_Count;
		m_ConfirmNum = 0;
		m_IsUnitQuery = false;
		m_PlayerId = 0;
		m_Result = Pending;
		m_ServerId = 0;
	}

	AuLoginQuery(unsigned int serverId, quint32 playerId, quint32 confirmNum, bool isUnitQuery, const QString& clientName)
	{
		m_AccountType = ACCT_Count;
		m_ClientName = clientName;
		m_ConfirmNum = confirmNum;
		m_IsUnitQuery = isUnitQuery;
		m_PlayerId = playerId;
		m_Result = Pending;
		m_ServerId = serverId;
	}

	inline bool IsLoggedIn() const
	{
		return m_Result == LoggedIn ? true : false;
	}

	inline bool IsResolved() const
	{
		return m_Result != Pending ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuUnitQuery
{
public:
	enum RESULT
	{
		Pending,		// Pending processing by db thread.
		BadUnit,		// Invalid queryUnit, account not found.
		Done,			// Query finished, unit ids valid even if null.
	};

public:
	QString			m_ClientName;		// Client ip name making request.
	AuId			m_QueryUnit;		// Unit queried. Looks for the account that owns this unit.
	RESULT			m_Result;			// Result of db query.
	unsigned int	m_ServerId;			// DtLoginServer id making request.
	AuId			m_UnitId[6];		// Valid if result is Done. List of all ids owned by the same account as m_QueryUnit.

	AuUnitQuery()
	{
		m_Result = Pending;
		m_ServerId = 0;
	}

	AuUnitQuery(unsigned int serverId, const AuId& queryUnit, const QString& clientName)
	{
		m_ClientName = clientName;
		m_QueryUnit = queryUnit;
		m_Result = Pending;
		m_ServerId = serverId;
	}

	inline bool IsResolved() const
	{
		return m_Result != Pending;
	}

	inline bool IsValid() const
	{
		return m_Result == Done;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuQueryAvatars
{
public:
	enum RESULT
	{
		Pending,		// Pending processing by db thread.
		Done,			// Query finished, avatar ids valid even if null.
	};

public:
	QString			m_ClientName;		// Client ip name making request.
	RESULT			m_Result;			// Result of db query.
	unsigned int	m_ServerId;			// DtLoginServer id making request.
	AuIdList		m_ActiveIds;		// List of active avatar ids.
	AuIdList		m_InactiveIds;		// List of inactive avatar ids.

	AuQueryAvatars()
	{
		m_Result = Pending;
		m_ServerId = 0;
	}

	AuQueryAvatars(unsigned int serverId, const QString& clientName)
	{
		m_ClientName = clientName;
		m_Result = Pending;
		m_ServerId = serverId;
	}

	inline bool IsResolved() const
	{
		return m_Result != Pending;
	}

	inline bool IsDone() const
	{
		return m_Result == Done;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuQueryAccountAvatars
{
public:
	enum RESULT
	{
		Pending,		// Pending processing by db thread.
		Done,			// Query finished, avatar ids valid even if null.
		BadLogin,		// Invalid player id or confirmation number.
	};

public:
	quint32			m_AccountId;
	AuId			m_AvatarId[6];
	QString			m_AvatarName[6];
	QString			m_ClientName;		// Client ip name making request.
	quint32			m_ConfirmNum;		// Confirmation number with request.
	RESULT			m_Result;			// Result of db query.
	unsigned int	m_ServerId;			// DtLoginServer id making request.

	AuQueryAccountAvatars()
	{
		m_AccountId = 0;
		m_ConfirmNum = 0;
		m_Result = Pending;
		m_ServerId = 0;
	}

	AuQueryAccountAvatars(unsigned int serverId, const QString& clientName, quint32 accountId, quint32 confirmNum)
	{
		m_AccountId = accountId;
		m_ClientName = clientName;
		m_ConfirmNum = confirmNum;
		m_Result = Pending;
		m_ServerId = serverId;
	}

	AuQueryAccountAvatars(const AuQueryAccountAvatars& c)
	{
		m_AccountId = c.m_AccountId;
		for(int ia=0; ia<6; ia++)
		{
			m_AvatarId[ia] = c.m_AvatarId[ia];
			m_AvatarName[ia] = c.m_AvatarName[ia];
		}
		m_ClientName = c.m_ClientName;
		m_ConfirmNum = c.m_ConfirmNum;
		m_Result = c.m_Result;
		m_ServerId = c.m_ServerId;
	}

public:	// Operators
	const AuQueryAccountAvatars& operator=(const AuQueryAccountAvatars& c)
	{
		m_AccountId = c.m_AccountId;
		for(int ia=0; ia<6; ia++)
		{
			m_AvatarId[ia] = c.m_AvatarId[ia];
			m_AvatarName[ia] = c.m_AvatarName[ia];
		}
		m_ClientName = c.m_ClientName;
		m_ConfirmNum = c.m_ConfirmNum;
		m_Result = c.m_Result;
		m_ServerId = c.m_ServerId;
		return *this;
	}

public:	// Methods
	inline bool IsResolved() const
	{
		return m_Result != Pending;
	}

	inline bool IsDone() const
	{
		return m_Result == Done;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuNewPlayer
{
public:
	enum RESULT
	{
		Success,
		BadBlankName,
		BadBlankPassword,
		BadBlankEmail,
		BadEmail,					// Address format looks wrong
		BadAccountExists,			// Account already exists.
		BadAccountExistsPending,	// New account pending add to db
		BadDbErr,					// database had error inserting record
		Pending,					// Pending processing by db thread.
	};

public:
	quint32								m_AcctId;				// Account id if successful.
	ACCOUNT								m_AccountType;
	QString								m_Email;
	QString								m_Name;
	QByteArray							m_PassCrypt;
	RESULT								m_Result;
	unsigned int						m_ServerId;				// DtLoginServer id making request.

public:
	AuNewPlayer()
	{
		m_AcctId = 0;
		m_AccountType = ACCT_Count;
		m_Result = Pending;
		m_ServerId = 0;
	}

	AuNewPlayer(unsigned int serverId, const QString& name, const QByteArray& passCrypt, const QString& email, ACCOUNT acctType)
	{
		m_AcctId = 0;
		m_AccountType = acctType;
		m_Email = email;
		m_Name = name;
		m_PassCrypt = passCrypt;
		m_Result = Pending;
		m_ServerId = serverId;
	}

	inline bool IsResolved() const
	{
		return m_Result != Pending ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSetUnit
{
public:
	QString			m_ClientName;		// Client ip name making request.
	quint32			m_ConfirmNum;		// Confirmation number with request.
	bool			m_IsDelete;			// true if this is a delete unit transaction
	quint32			m_PlayerId;			// Player id making the request.
	AuId			m_UnitId;			// New unit id.

public:
	AuSetUnit()
	{
		m_ConfirmNum = 0;
		m_IsDelete = false;
		m_PlayerId = 0;
	}

	AuSetUnit(const QString& clientName, quint32 playerId, quint32 confirmNum, const AuId& unitId, bool isDelete)
	{
		m_ClientName = clientName;
		m_ConfirmNum = confirmNum;
		m_IsDelete = isDelete;
		m_PlayerId = playerId;
		m_UnitId = unitId;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuUnitBorn
{
public:
	QString			m_ClientName;		// Client ip name making request.
	AuId			m_MotherId;			// Mother of the new unit.
	AuId			m_UnitId;			// New unit id.

public:
	AuUnitBorn()
	{
	}

	AuUnitBorn(const QString& clientName, const AuId& motherId, const AuId& unitId)
	{
		m_ClientName = clientName;
		m_MotherId = motherId;
		m_UnitId = unitId;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuUnitDied
{
public:
	QString			m_ClientName;		// Client ip name making request.
	AuId			m_UnitId;			// Dead unit id.

public:
	AuUnitDied()
	{
	}

	AuUnitDied(const QString& clientName, const AuId& unitId)
	{
		m_ClientName = clientName;
		m_UnitId = unitId;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuUnitTransfer
{
public:
	AuId			m_AccepterId;		// Who accepted transfer of the avatar.
	AuId			m_AvatarId;			// Avatar to be transfered.
	QString			m_ClientName;		// Client ip name making request.

public:
	AuUnitTransfer()
	{
	}

	AuUnitTransfer(const QString& clientName, const AuId& avatar, const AuId& accepter)
	{
		m_ClientName = clientName;
		m_AccepterId = accepter;
		m_AvatarId = avatar;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuSetEmail
{
public:
	QString			m_Name;				// Player name making the change
	QByteArray		m_PassCrypt;		// Player password SHA-1 hash
	QString			m_Email;			// New email address.

public:
	AuSetEmail()
	{
	}

	AuSetEmail(const QString& playerName, const QByteArray& passCrypt,
			const QString& email)
	{
		m_Name = playerName;
		m_PassCrypt = passCrypt;
		m_Email = email;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuSetPassword
{
public:
	QString			m_Name;				// Player name making the change
	QByteArray		m_PassCrypt;			// Player password SHA-1 hash
	QByteArray		m_NewPassCrypt;		// New password SHA-1 hash

public:
	AuSetPassword()
	{
	}

	AuSetPassword(const QString& playerName, const QByteArray& passCrypt,
			const QByteArray& newPassCrypt)
	{
		m_Name = playerName;
		m_PassCrypt = passCrypt;
		m_NewPassCrypt = newPassCrypt;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuResetPassword
{
public:
	QString			m_Name;				// Player name making the request

public:
	AuResetPassword()
	{
	}

	AuResetPassword(const QString& playerName)
	{
		m_Name = playerName;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DhLoginServer
{
public:
	QDateTime	m_CheckIn;	// Last checkin time.
	QString		m_NetName;	// Internet resolvable name.
	quint16		m_NetPort;	// Internet port on which the server is listening.
	QString		m_PeerAddr;	// Address on LAN

public:
	DhLoginServer()
	{
		m_NetPort = 0;
	}

	DhLoginServer(const DhLoginServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerAddr = c.m_PeerAddr;
	}

	const DhLoginServer& operator=(const DhLoginServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerAddr = c.m_PeerAddr;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuLoginServerApp : public QCoreApplication
{
	Q_OBJECT;
public:
	AuLoginServerApp(int& argc, char** argv);
	~AuLoginServerApp();

	void HandleSignal(int value);	// Handles a signal caught in main.cpp.

	// Returns true if any of my queues have work for the db thread.
	bool HasQueuedOperations() const;

	QMap<QString, DhLoginServer>& LockServerMap()
	{
		m_muServerMap.lock();
		return m_ServerMap;
	}

	inline const QDateTime& TimeUp() const
	{
		return m_tmUp;
	}

	void UnlockServerMap()
	{
		m_muServerMap.unlock();
	}

	// Returns the number of worker servers.
	int WorkerCount();

protected:
	void StartServer();

protected slots:
	void UpdateStats();

private:
	QMutex								m_muServerMap;
	QMap<QString, DhLoginServer>		m_ServerMap;			// Used by LoginMaster.
	AuLoginServer						m_TcpServer;
	QTimer*								m_timer;
	DtLoginDb*							m_tLoginDb;
	QDateTime							m_tmCheckIn;			// Last time checked in with Master.
	QDateTime							m_tmUp;					// Time this object was constructed. For uptime.

public:
	// Values used to queue database operations that need to be performed by the database thread.
	mutable QMutex						m_LoginMutex;			// Guards all of these.
	QVector<AuLoginAttempt>				m_LoginAttempts;
	QVector<AuLoginQuery>				m_LoginQueries;
#if defined(AUSTEAM)
	QVector<AuLoginSteamAttempt>		m_LoginSteamAttempts;
#endif	// AUSTEAM
	QVector<AuLogoutAttempt>			m_LogoutAttempts;
	QVector<AuNewPlayer>				m_NewPlayers;
	QVector<AuQueryAccountAvatars>		m_QueryAccountAvatars;
	QVector<AuQueryAvatars>				m_QueryAvatars;
	QVector<AuResetPassword>			m_ResetPassword;
	QVector<AuSetEmail>					m_SetEmail;
	QVector<AuSetPassword>				m_SetPassword;
	QVector<AuSetUnit>					m_SetUnit;
	QVector<AuUnitBorn>					m_UnitBorn;
	QVector<AuUnitDied>					m_UnitDied;
	QVector<AuUnitQuery>				m_UnitQueries;
	QVector<AuUnitTransfer>				m_UnitTransfer;
	
	mutable QMutex						m_LoginContactsMutex;	
	QVector<AuLoginContacts>			m_ContactsQueries;
	QVector<AuLoginContacts>			m_ContactsUpdates;
	QVector<AuLoginContacts>			m_ContactsAdds;
	QVector<AuLoginContacts>			m_ContactsDeletes;
	QVector<AuLoginContacts>			m_ContactAdds;
	QVector<AuLoginContacts>			m_ContactDeletes;
};

#define auLoginServerApp	((AuLoginServerApp*)QCoreApplication::instance())

#endif // AuLoginServerApp_H
