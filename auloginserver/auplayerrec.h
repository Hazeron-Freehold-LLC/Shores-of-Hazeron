//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerrec.h,v 1.30 2022/02/19 18:35:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlayerRec_H
#define AuPlayerRec_H

#include "auid.h"
#include "autime.h"
#include <QString>

class QSqlDatabase;

class AuPlayerRec
{
private:
	ACCOUNT								m_AccountType;			// Type/Status of this account. This is the 'status' db field.
	AuTime								m_Activated;
	unsigned int						m_ActiveDays;
	AuId								m_Avatar[6];			// Id of each avatar.
	unsigned int						m_confirm;				// confirmation number for login
	QString								m_email;				// email address
	unsigned int						m_id;					// Database record id.
	bool								m_isLoggedIn;			// true while logged in
	QString								m_LoginName;			// user login name
	QByteArray							m_passCrypt;			// login password SHA-1 hash

public:	// Construction
	AuPlayerRec();
	AuPlayerRec(const QString& loginName, const QByteArray& passCrypt, const QString& email, ACCOUNT acctType);
	AuPlayerRec(const AuPlayerRec& c);
	~AuPlayerRec();

public:	// Operators
	const AuPlayerRec &operator=(const AuPlayerRec& c);


public:	// Methods
	AuTime AccountExpiry() const;

	inline ACCOUNT AccountType() const
	{
		return m_AccountType;
	}

	// Returns index of newly added avatar, -1 if no empty slot was available.
	int AddAvatar(const AuId& avatar);

	// Returns index of existing avatar 'id', -1 if not found.
	int AvatarIndex(const AuId& id) const;

	inline const AuId& AvatarId(int idx) const
	{
		return m_Avatar[idx];
	}

	inline bool CanMail() const			// true if player account can get/send mail
	{
		return true;
	}

	inline unsigned int ConfirmationNumber() const
	{
		return m_confirm;
	}

	// Creates a new account that is inactive.
	// Returns
	//  0 = ok
	//  1 = login name is blank
	//  2 = player record already exists
	//  3 = sql exec failed
	//  4 = player record failed select after insert
	int Create(QSqlDatabase& db);

	// Creates a new account that is active for 30 days, starting now.
	// Returns
	//  0 = ok
	//  1 = login name is blank
	//  2 = player record already exists
	//  3 = sql exec failed
	//  4 = player record failed select after insert
	int CreateSteam(QSqlDatabase& db);

	// Returns the index that is deleted, or -1 if none.
	int DeleteAvatar(const AuId& id);

	inline const QString& Email() const
	{
		return m_email;
	}

	// Returns the index of an empty avatar slot, or -1 if none.
	int EmptyAvatarIndex() const;

	inline unsigned int Id() const
	{
		return m_id;
	}

	void InitConfirmation();

	inline bool IsLoggedIn() const
	{
		return m_isLoggedIn;
	}

	inline const QString& LoginName() const
	{
		return m_LoginName;
	}

	inline const QByteArray& PassCrypt() const
	{
		return m_passCrypt;
	}

	// 'activeIds' will be filled with all avatar ids associated with active login accounts.
	// 'inactiveIds' will be filled with all avatar ids associated with inactive login accounts.
	// Both lists combined will represent a comprehensive list of all avatars.
	static void QueryAvatars(AuIdList& activeIds, AuIdList& inactiveIds);

	bool Select(unsigned int playerId);

	bool Select(unsigned int playerId, unsigned int confirmNum);

	bool Select(const AuId& unitId);

	bool Select(const QString& loginName);

	bool Select(const QString& loginName, const QByteArray& passCrypt);

	inline void SetLoggedIn(bool loggedIn)
	{
		m_isLoggedIn = loggedIn;
	}

	inline void SetPassCrypt(const QByteArray& passCrypt)
	{
		m_passCrypt = passCrypt;
	}

	// Returns the number of days that a Steam account is initially activated.
	static int SteamActivationDays();

	// Returns the number of days that a trial account is initially activated.
	static int TrialActivationDays();
};

#endif // AuPlayerRec_H

