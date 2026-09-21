//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerrec.cpp,v 1.31 2022/04/14 21:24:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplayerrec.h"
//
#include "auglobal.h"
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

AuPlayerRec::AuPlayerRec()
{
	m_confirm = 0;
	m_id = 0;
	m_isLoggedIn = false;
	m_AccountType = ACCT_Active;
}

AuPlayerRec::AuPlayerRec(const QString& loginName, const QByteArray& passCrypt, const QString& email, ACCOUNT acctType) :
	m_email(email), m_LoginName(loginName), m_passCrypt(passCrypt)
{
	m_confirm = 0;
	m_id = 0;
	m_isLoggedIn = false;
	m_AccountType = acctType;
}

AuPlayerRec::~AuPlayerRec()
{
}

AuPlayerRec::AuPlayerRec(const AuPlayerRec& c) : m_email(c.m_email), m_LoginName(c.m_LoginName), m_passCrypt(c.m_passCrypt)
{
	for(int iu=0; iu<6; iu++)
		m_Avatar[iu] = c.m_Avatar[iu];
	m_confirm = c.m_confirm;
	m_id = c.m_id;
	m_isLoggedIn = c.m_isLoggedIn;
	m_AccountType = c.m_AccountType;
}

const AuPlayerRec &AuPlayerRec::operator=(const AuPlayerRec& c)
{
	for(int iu=0; iu<6; iu++)
		m_Avatar[iu] = c.m_Avatar[iu];
	m_confirm = c.m_confirm;
	m_email = c.m_email;
	m_id = c.m_id;
	m_isLoggedIn = c.m_isLoggedIn;
	m_LoginName = c.m_LoginName;
	m_passCrypt = c.m_passCrypt;
	m_AccountType = c.m_AccountType;
	return *this;
}

void AuPlayerRec::InitConfirmation()
{
	// Assign a valid confirmation number.
	m_confirm = g_Au.GenConfirmation();
}

//static
void AuPlayerRec::QueryAvatars(AuIdList& activeIds, AuIdList& inactiveIds)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return;

	QString sq("SELECT status, idu1, idu2, idu3, idu4, idu5, idu6 FROM g_Player;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv = 0;	// query value index
		int status = qry.value(qv++).toInt();
		if(status == ACCT_Expired)
		{
			// Inactive account.
			for(int iu=0; iu<6; iu++)
			{
				AuId idu = AuId(qry.value(qv++).toUInt());
				if(idu != AuId())
					inactiveIds.append(idu);
			}
		}
		else
		{
			// Active account.
			for(int iu=0; iu<6; iu++)
			{
				AuId idu = AuId(qry.value(qv++).toUInt());
				if(idu != AuId())
					activeIds.append(idu);
			}
		}
	}
}

bool AuPlayerRec::Select(unsigned int playerId)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return false;

	QSqlQuery qry(QString(), db);
	QString sq("SELECT name, status, passcrypt, email, confirm, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player WHERE id=?;");
	qry.prepare(sq);
	qry.addBindValue(playerId);
	if(qry.exec() && qry.first())
	{
		int qv=0;
		m_id = playerId;
		m_LoginName = qry.value(qv++).toString();
		m_AccountType = (ACCOUNT)qry.value(qv++).toInt();
		m_passCrypt = qry.value(qv++).toByteArray();
		m_email = qry.value(qv++).toString();
		m_confirm = qry.value(qv++).toInt();
		m_isLoggedIn = qry.value(qv++).toBool();

		QDateTime acctActivated = qry.value(qv++).toDateTime();
		acctActivated.setTimeSpec(Qt::UTC);
		m_Activated = acctActivated;
		m_ActiveDays = qry.value(qv++).toUInt();

		for(int iu=0; iu<6; iu++)
			m_Avatar[iu] = AuId(qry.value(qv++).toUInt());
		return true;
	}
	return false;
}

bool AuPlayerRec::Select(unsigned int playerId, unsigned int confirmNum)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return false;

	QSqlQuery qry(QString(), db);
	QString sq("SELECT name, status, passcrypt, email, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player WHERE id=? AND confirm=?;");
	qry.prepare(sq);
	qry.addBindValue(playerId);
	qry.addBindValue(confirmNum);
	if(qry.exec() && qry.first())
	{
		int qv=0;
		m_id = playerId;
		m_LoginName = qry.value(qv++).toString();
		m_AccountType = (ACCOUNT)qry.value(qv++).toInt();
		m_passCrypt = qry.value(qv++).toByteArray();
		m_email = qry.value(qv++).toString();
		m_confirm = confirmNum;
		m_isLoggedIn = qry.value(qv++).toBool();

		QDateTime acctActivated = qry.value(qv++).toDateTime();
		acctActivated.setTimeSpec(Qt::UTC);
		m_Activated = acctActivated;
		m_ActiveDays = qry.value(qv++).toUInt();

		for(int iu=0; iu<6; iu++)
			m_Avatar[iu] = AuId(qry.value(qv++).toUInt());
		return true;
	}
	return false;
}

bool AuPlayerRec::Select(const AuId& unitId)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return false;

	QSqlQuery qry(QString(), db);
	QString sq = QString("SELECT id, name, status, passcrypt, email, confirm, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player WHERE idu1=%1 OR idu2=%1 OR idu3=%1 OR idu4=%1 OR idu5=%1 OR idu6=%1;").arg(unitId.GetID());
	qry.prepare(sq);
	if(qry.exec() && qry.first())
	{
		int qv=0;
		m_id = qry.value(qv++).toInt();
		m_LoginName = qry.value(qv++).toString();
		m_AccountType = (ACCOUNT)qry.value(qv++).toInt();
		m_passCrypt = qry.value(qv++).toByteArray();
		m_email = qry.value(qv++).toString();
		m_confirm = qry.value(qv++).toInt();
		m_isLoggedIn = qry.value(qv++).toBool();

		QDateTime acctActivated = qry.value(qv++).toDateTime();
		acctActivated.setTimeSpec(Qt::UTC);
		m_Activated = acctActivated;
		m_ActiveDays = qry.value(qv++).toUInt();

		for(int iu=0; iu<6; iu++)
			m_Avatar[iu] = AuId(qry.value(qv++).toUInt());
		return true;
	}
	return false;
}

bool AuPlayerRec::Select(const QString& loginName)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return false;

	QSqlQuery qry(QString(), db);
	QString sq("SELECT id, status, passcrypt, email, confirm, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player WHERE name=?;");
	qry.prepare(sq);
	qry.addBindValue(loginName);
	if(qry.exec() && qry.first())
	{
		int qv=0;
		m_id = qry.value(qv++).toInt();
		m_AccountType = (ACCOUNT)qry.value(qv++).toInt();
		m_LoginName = loginName;
		m_passCrypt = qry.value(qv++).toByteArray();
		m_email = qry.value(qv++).toString();
		m_confirm = qry.value(qv++).toInt();
		m_isLoggedIn = qry.value(qv++).toBool();

		QDateTime acctActivated = qry.value(qv++).toDateTime();
		acctActivated.setTimeSpec(Qt::UTC);
		m_Activated = acctActivated;
		m_ActiveDays = qry.value(qv++).toUInt();

		for(int iu=0; iu<6; iu++)
			m_Avatar[iu] = AuId(qry.value(qv++).toUInt());
		return true;
	}
	return false;
}

bool AuPlayerRec::Select(const QString& loginName, const QByteArray& passCrypt)
{
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!db.isOpen())
		return false;

	//qDebug() << "AuPlayerRec::Select" << loginName << "passCrypt" << passCrypt;

	QSqlQuery qry(QString(), db);
	QString sq("SELECT id, status, email, confirm, isloggedin, activated, days, "
			"idu1, "
			"idu2, "
			"idu3, "
			"idu4, "
			"idu5, "
			"idu6 "
			"FROM g_Player WHERE name=? AND passCrypt=?;");
	qry.prepare(sq);
	qry.addBindValue(loginName);
	qry.addBindValue(passCrypt);
	if(qry.exec() && qry.first())
	{
		int qv=0;
		m_id = qry.value(qv++).toInt();
		m_AccountType = (ACCOUNT)qry.value(qv++).toInt();
		m_LoginName = loginName;
		m_passCrypt = passCrypt;
		m_email = qry.value(qv++).toString();
		m_confirm = qry.value(qv++).toInt();
		m_isLoggedIn = qry.value(qv++).toBool();

		QDateTime acctActivated = qry.value(qv++).toDateTime();
		acctActivated.setTimeSpec(Qt::UTC);
		m_Activated = acctActivated;
		m_ActiveDays = qry.value(qv++).toUInt();

		for(int iu=0; iu<6; iu++)
			m_Avatar[iu] = AuId(qry.value(qv++).toUInt());
		return true;
	}
	return false;
}

int AuPlayerRec::Create(QSqlDatabase& db)
{
	// Name must not be blank.
	if(m_LoginName.isEmpty())
		return 1;

	// Query database for existing player record with the same name.
	AuPlayerRec existing;
	if(existing.Select(m_LoginName))
		return 2;

	QSqlQuery qa(QString(), db);
	if(m_AccountType == ACCT_Trial)
	{
		// New trial accounts are created with some initial activation time.
		QString sq("INSERT INTO g_Player (name,passcrypt,email,timeCreate,status, activated,days) VALUES (?,?,?,?,?, ?,?);");
		qa.prepare(sq);
		qa.addBindValue(m_LoginName);
		qa.addBindValue(m_passCrypt);
		qa.addBindValue(m_email);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(m_AccountType);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(TrialActivationDays());
	}
	else
	{
		// New Closed or Active account.
		QString sq("INSERT INTO g_Player (name,passcrypt,email,timeCreate,status) VALUES (?,?,?,?,?);");
		qa.prepare(sq);
		qa.addBindValue(m_LoginName);
		qa.addBindValue(m_passCrypt);
		qa.addBindValue(m_email);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(m_AccountType);
	}
	if(!qa.exec())
		return 3;

	// Select the resulting record to get our unique account id.
	if(!Select(m_LoginName))
		return 4;
	return 0;
}

int AuPlayerRec::CreateSteam(QSqlDatabase& db)
{
	// Name must not be blank.
	if(m_LoginName.isEmpty())
		return 1;

	// Query database for existing player record with the same name.
	AuPlayerRec existing;
	if(existing.Select(m_LoginName))
		return 2;

	QSqlQuery qa(QString(), db);
	if(m_AccountType == ACCT_Trial)
	{
		// New trial accounts are created with some initial activation time.
		QString sq("INSERT INTO g_Player (name,passcrypt,email,timeCreate,status, activated,days) VALUES (?,?,?,?,?, ?,?);");
		qa.prepare(sq);
		qa.addBindValue(m_LoginName);
		qa.addBindValue(m_passCrypt);
		qa.addBindValue(m_email);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(m_AccountType);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(SteamActivationDays());
	}
	else
	{
		// New Closed or Active account.
		QString sq("INSERT INTO g_Player (name,passcrypt,email,timeCreate,status) VALUES (?,?,?,?,?);");
		qa.prepare(sq);
		qa.addBindValue(m_LoginName);
		qa.addBindValue(m_passCrypt);
		qa.addBindValue(m_email);
		qa.addBindValue(QDateTime::currentDateTimeUtc());
		qa.addBindValue(m_AccountType);
	}
	if(!qa.exec())
		return 3;

	// Select the resulting record to get our unique account id.
	if(!Select(m_LoginName))
		return 4;
	return 0;
}

int AuPlayerRec::AddAvatar(const AuId& avatar)
{
	int iu = EmptyAvatarIndex();
	if(iu >= 0)
	{
		m_Avatar[iu] = avatar;
		return iu;
	}
	return -1;
}

int AuPlayerRec::DeleteAvatar(const AuId& id)
{
	int iu = AvatarIndex(id);
	if(iu >= 0)
	{
		m_Avatar[iu] = AuId();
		return iu;
	}
	return -1;
}

int AuPlayerRec::EmptyAvatarIndex() const
{
	for(int iu=0; iu<6; iu++)
		if(m_Avatar[iu].IsNull())
			return iu;
	return -1;
}

int AuPlayerRec::AvatarIndex(const AuId& id) const
{
	for(int iu=0; iu<6; iu++)
		if(m_Avatar[iu] == id)
			return iu;
	return -1;
}

//static
int AuPlayerRec::SteamActivationDays()
{
	return 30;
}

//static
int AuPlayerRec::TrialActivationDays()
{
	return 7;
}

AuTime AuPlayerRec::AccountExpiry() const
{
	return m_Activated.AddDays(m_ActiveDays);
}

