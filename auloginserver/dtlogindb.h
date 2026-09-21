//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtlogindb.h,v 1.12 2022/02/19 18:35:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DTLOGINDB_H
#define DTLOGINDB_H

#include "auloginserverapp.h"
#include "auplayerrec.h"
#include <QDateTime>
#include <QSemaphore>
#include <QThread>

class DaAtom;
class QSqlDatabase;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuLoginCacheItem
{
public:
	QString			m_ClientName;	// Client whos request created this item.
	QDateTime		m_LastAccess;	// Last check time for decay of cache items.
	AuPlayerRec		m_PlayerRec;	// Player rec that was retrieved.
									// Player rec id will be 0 if SELECT failed.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DtLoginDb : public QThread
{
	Q_OBJECT;
public:
	enum DBSTATUS
	{
		NotLoaded,		// Database has not been loaded yet.
		Loaded,			// Database loaded ok.
		OpenError,		// Database did not open ok.
		DriverError,	// Database driver did not init ok.
	};

public:
	DtLoginDb(QObject* parent=0);
	~DtLoginDb();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	void Kill()
	{
		m_semAlive.release(1);
	}

protected:
	virtual void run();

private:
	// Update login cache entry for a given player record.
	void CacheLoginAttempt(const QString& clientName,
			const AuPlayerRec& player);

	// Try to load player record from the cache.
	void CheckLoginCache(const QString& name, const QByteArray& passCrypt, const QString& clientName, AuPlayerRec& player) const;
	void CheckLoginCache(unsigned int playerId, unsigned int confirmNum, const QString& clientName, AuPlayerRec& player) const;
	void CheckLoginCache(const AuId& unitId, const QString& clientName, AuPlayerRec& player) const;

	// Remove cache items that have gotten too old.
	void DecayLoginCache();

	// Removes a specific item from the cache.
	void DeleteLoginCacheItem(unsigned int playerId);

	bool OpenDatabase(QSqlDatabase& db);
	bool OpenUniverseDatabase(QSqlDatabase& db);

	void PerformQueuedOperations();

private:
	QSemaphore		m_semAlive;		// Used to know when I'm supposed to die.
	QMap<QString, AuLoginCacheItem>	m_LoginCache;	// Key is player name
};

#endif // DTLOGINDB_H
