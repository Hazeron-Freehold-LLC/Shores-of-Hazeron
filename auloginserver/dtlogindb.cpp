//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtlogindb.cpp,v 1.87 2022/04/08 16:09:11 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtlogindb.h"
//
#include "audef.h"
#include "augear.h"
#include "auglobal.h"
#include "auloginserverapp.h"
#include "auplayerrec.h"
#include "aucontactrec.h"
#include "dhdna.h"
#include <QCryptographicHash>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>

DtLoginDb::DtLoginDb(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_semAlive.acquire(1);	// I live...
}

DtLoginDb::~DtLoginDb()
{
	//qDebug("DtLoginDb terminated.");
}

bool DtLoginDb::OpenDatabase(QSqlDatabase& db)
{
	//qDebug("DtLoginDb::OpenDatabase");
	// auLogin database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	db.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
	db.setHostName("LoginDb.Hazeron.net");
	if(!db.open("audog","Gold"))
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
		return false;
	}
	//qDebug("DtLoginDb::OpenDatabase - ok");
	return true;
}

bool DtLoginDb::OpenUniverseDatabase(QSqlDatabase& db)
{
	// auLogin database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	db.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
	db.setHostName("UniverseDb.Hazeron.net");
	if(!db.open("audog","Gold"))
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
		return false;
	}

	if(g_Au.m_CommodityArray.empty())
		g_Au.m_CommodityArray.Load();
	return true;
}

void DtLoginDb::run()
{
	//qDebug("DtLoginDb started.");
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
	QSqlDatabase udb = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
	while(Alive())
	{
		// Check for db updates that are queued for my attention.
		if(auLoginServerApp->HasQueuedOperations())
			PerformQueuedOperations();

		// Decay the login cache items.
		DecayLoginCache();

		msleep(100);
	}

	// Remove the database.
	QSqlDatabase::removeDatabase("AuLogin");
	QSqlDatabase::removeDatabase("AuUniverse");
}

void DtLoginDb::PerformQueuedOperations()
{
	//qDebug("DtLoginDb::PerformQueuedOperations");
	QSqlDatabase db = QSqlDatabase::database("AuLogin", false);
	if(!OpenDatabase(db))
		return;

	// Universe database is only opened when needed.
	QSqlDatabase udb = QSqlDatabase::database("AuUniverse", false);

	auLoginServerApp->m_LoginMutex.lock();

	// New player create requests.
	for(QVector<AuNewPlayer>::iterator it = auLoginServerApp->m_NewPlayers.begin();
			it != auLoginServerApp->m_NewPlayers.end(); it++)
	{
		if((*it).IsResolved())
			continue;

		// Validate data
		if((*it).m_Name.isEmpty())
		{
			(*it).m_Result = AuNewPlayer::BadBlankName;
			continue;
		}
		else if((*it).m_PassCrypt.isEmpty())
		{
			(*it).m_Result = AuNewPlayer::BadBlankPassword;
			continue;
		}
		else if((*it).m_Email.isEmpty())
		{
			(*it).m_Result = AuNewPlayer::BadBlankEmail;
			continue;
		}
		else if((*it).m_Email.count('@') != 1 ||
			(*it).m_Email.count(' ') > 0 ||
			(*it).m_Email.count('.') == 0)
		{
			(*it).m_Result = AuNewPlayer::BadEmail;
			continue;
		}

		// Check for existing player with same name pending add.
		for(QVector<AuNewPlayer>::const_iterator ic = auLoginServerApp->m_NewPlayers.constBegin();
				ic != auLoginServerApp->m_NewPlayers.constEnd(); ic++)
		{
			if((*ic).m_Name == (*it).m_Name)
			{
				// Ignore the 'it' at the same index as 'ic'.
				if(it - auLoginServerApp->m_NewPlayers.begin() == ic - auLoginServerApp->m_NewPlayers.constBegin())
					continue;
				(*it).m_Result = AuNewPlayer::BadAccountExistsPending;
				break;
			}
		}
		if((*it).IsResolved())
			continue;

		// Query database for existing player record with the same name.
		AuPlayerRec exPlayer;
		if(exPlayer.Select((*it).m_Name))
		{
			(*it).m_Result = AuNewPlayer::BadAccountExists;
			continue;
		}

		// Attempt to add the new account to the user database.
		AuPlayerRec player((*it).m_Name, (*it).m_PassCrypt, (*it).m_Email, (*it).m_AccountType);
		int result = player.Create(db);
		if(result == 0)
		{
			// It looks like all went well.
			(*it).m_Result = AuNewPlayer::Success;
			(*it).m_AcctId = player.Id();
		}
		else if(result == 2)
		{
			(*it).m_Result = AuNewPlayer::BadAccountExists;
		}
		else if(result != 1)
		{
			(*it).m_Result = AuNewPlayer::BadDbErr;
			QSqlError er = db.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning() << "error: PerformQueuedOperations" << sdv << sdb << "Failed to exec database query.";
		}
	}

	// Player unit assignment.
	if(!auLoginServerApp->m_SetUnit.isEmpty())
	{
		for(QVector<AuSetUnit>::iterator it = auLoginServerApp->m_SetUnit.begin(); it != auLoginServerApp->m_SetUnit.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_PlayerId, (*it).m_ConfirmNum))
			{
				// Selected the player record from the database.
				bool changedPlayerRec = false;
				if((*it).m_IsDelete)
				{
					// Find the unit id slot to delete
					int iu = player.DeleteAvatar((*it).m_UnitId);
					if(iu >= 0)
					{
						QString sq;
						QSqlQuery ql(QString(), db);
						sq = QString("UPDATE g_Player SET idu%1=0 WHERE id=%2;")
							.arg(iu+1).arg((*it).m_PlayerId);
						ql.prepare(sq);
						if(!ql.exec())
							qWarning("error: PerformQueuedOperations - Player %d delete unit %d UPDATE FAILED.",
									(*it).m_PlayerId, (*it).m_UnitId.GetID());
						else
							changedPlayerRec = true;
					}
					else
					{
						qWarning("error: PerformQueuedOperations - Player %d has no unit id %d to delete.",
								(*it).m_PlayerId, (*it).m_UnitId.GetID());
					}
				}
				else
				{
					// Find an available unit id slot; that is one with a null unit id.
					int iu = player.AddAvatar((*it).m_UnitId);
					if(iu >= 0)
					{
						QString sq;
						QSqlQuery ql(QString(), db);
						sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
							.arg(iu+1).arg((*it).m_PlayerId);
						ql.prepare(sq);
						ql.addBindValue((*it).m_UnitId.GetID());
						if(!ql.exec())
							qWarning("ERROR! PerformQueuedOperations - Player %d Unit %d UPDATE FAILED.",
									(*it).m_PlayerId, (*it).m_UnitId.GetID());
						else
							changedPlayerRec = true;
					}
					else
					{
						qWarning("ERROR! PerformQueuedOperations - Player %d has no empty slots for new unit id %d.",
								(*it).m_PlayerId, (*it).m_UnitId.GetID());
					}
				}

				// The login cache must be updated immediately with this change.
				if(changedPlayerRec)
					CacheLoginAttempt((*it).m_ClientName, player);	// Update/insert cache item.
			}
		}
		auLoginServerApp->m_SetUnit.clear();
	}

	// Player units born.
	if(!auLoginServerApp->m_UnitBorn.isEmpty())
	{
		for(QVector<AuUnitBorn>::iterator it = auLoginServerApp->m_UnitBorn.begin(); it != auLoginServerApp->m_UnitBorn.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_MotherId))
			{
				// Selected the player record from the database.
				bool changedPlayerRec = false;

				// Find an available unit id slot; that is one with a null unit id.
				// If we don't find one, the attempt to add fails quietly. That is expected. There is no guarantee of a slot when
				// a new avatar is born.
				int iu = player.AddAvatar((*it).m_UnitId);
				if(iu >= 0)
				{
					QString sq;
					QSqlQuery ql(QString(), db);
					sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
						.arg(iu+1).arg(player.Id());
					ql.prepare(sq);
					ql.addBindValue((*it).m_UnitId.GetID());
					if(!ql.exec())
						qWarning("ERROR! PerformQueuedOperations - Player %d Unit born %d UPDATE FAILED.",
								player.Id(), (*it).m_UnitId.GetID());
					else
						changedPlayerRec = true;
				}

				// The login cache must be updated immediately with this change.
				if(changedPlayerRec)
					CacheLoginAttempt((*it).m_ClientName, player);	// Update/insert cache item.
			}
		}
		auLoginServerApp->m_UnitBorn.clear();
	}

	// Player units died.
	if(!auLoginServerApp->m_UnitDied.isEmpty())
	{
		for(QVector<AuUnitDied>::iterator it = auLoginServerApp->m_UnitDied.begin(); it != auLoginServerApp->m_UnitDied.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_UnitId))
			{
				// Selected the player record from the database.
				bool changedPlayerRec = false;

				// We loaded a player record with that unit id; we should find it among their current units.
				int iu = player.DeleteAvatar((*it).m_UnitId);
				if(iu >= 0)
				{
					QString sq;
					QSqlQuery ql(QString(), db);
					sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
						.arg(iu+1).arg(player.Id());
					ql.prepare(sq);
					ql.addBindValue(0);
					if(!ql.exec())
						qWarning("ERROR! PerformQueuedOperations - Player %d Unit died %d UPDATE FAILED.",
								player.Id(), (*it).m_UnitId.GetID());
					else
						changedPlayerRec = true;
				}

				// The login cache must be updated immediately with this change.
				if(changedPlayerRec)
					CacheLoginAttempt((*it).m_ClientName, player);	// Update/insert cache item.
			}
		}
		auLoginServerApp->m_UnitDied.clear();
	}

	// Player units to transfer.
	if(!auLoginServerApp->m_UnitTransfer.isEmpty())
	{
		for(QVector<AuUnitTransfer>::iterator it = auLoginServerApp->m_UnitTransfer.begin();
				it != auLoginServerApp->m_UnitTransfer.end(); it++)
		{
			AuPlayerRec playerOffering;
			AuPlayerRec playerAccepting;
			//qDebug() << "UnitTransfer" << (*it).m_AvatarId << (*it).m_AccepterId << playerOffering.Id() << playerAccepting.Id();
			if(playerOffering.Select((*it).m_AvatarId) && playerAccepting.Select((*it).m_AccepterId) &&
					playerOffering.Id() &&
					playerAccepting.Id() &&
					playerOffering.Id() != playerAccepting.Id())
			{
				// Selected the relevant player records from the database and they are not the same account.
				// Insure the accepter still has an empty slot.
				int iacc = playerAccepting.EmptyAvatarIndex();

				// Locate the avatar in the offerer's list of avatars.
				int ioff = playerOffering.AvatarIndex((*it).m_AvatarId);

				if(iacc >= 0 && ioff >= 0)
				{
					// Both accounts are ok for the transfer.
					bool changedAcceptingRec = false;
					bool changedOfferingRec = false;

					// Add the avatar to the accepter's account.
					{
						iacc = playerAccepting.AddAvatar((*it).m_AvatarId);

						QString sq;
						QSqlQuery ql(QString(), db);
						sq = QString("UPDATE g_Player SET idu%1=? WHERE id=%2;")
							.arg(iacc+1).arg(playerAccepting.Id());
						ql.prepare(sq);
						ql.addBindValue((*it).m_AvatarId.GetID());
						if(!ql.exec())
							qWarning("ERROR! PerformQueuedOperations - Player %d Unit transfer %d UPDATE FAILED.",
									playerAccepting.Id(), (*it).m_AvatarId.GetID());
						else
							changedAcceptingRec = true;
					}

					// Remove the avatar from the offerer's account.
					{
						playerOffering.DeleteAvatar((*it).m_AvatarId);

						QString sq;
						QSqlQuery ql(QString(), db);
						sq = QString("UPDATE g_Player SET idu%1=0 WHERE id=%2;")
							.arg(ioff+1).arg(playerOffering.Id());
						ql.prepare(sq);
						if(!ql.exec())
							qWarning("ERROR! PerformQueuedOperations - Player %d Unit transfer %d UPDATE FAILED.",
									playerOffering.Id(), (*it).m_AvatarId.GetID());
						else
							changedOfferingRec = true;
					}

					// The login cache must be updated immediately with this change.
					if(changedOfferingRec)
						CacheLoginAttempt((*it).m_ClientName, playerOffering);	// Update/insert cache item.
					if(changedAcceptingRec)
						CacheLoginAttempt((*it).m_ClientName, playerAccepting);	// Update/insert cache item.
				}
			}
		}
		auLoginServerApp->m_UnitTransfer.clear();
	}

	// Player email address changes.
	if(!auLoginServerApp->m_SetEmail.isEmpty())
	{
		for(QVector<AuSetEmail>::iterator it = auLoginServerApp->m_SetEmail.begin(); it != auLoginServerApp->m_SetEmail.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_Name, (*it).m_PassCrypt) && player.Email() != (*it).m_Email)
			{
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QObject::tr("UPDATE g_Player SET email=? WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				ql.addBindValue((*it).m_Email);
				if(!ql.exec())
					qWarning("ERROR! PerformQueuedOperations - Player %d Email %s UPDATE FAILED.",
							player.Id(), (const char*)(*it).m_Email.toLatin1());
			}
		}
		auLoginServerApp->m_SetEmail.clear();
	}

	// Player password changes.
	if(!auLoginServerApp->m_SetPassword.isEmpty())
	{
		for(QVector<AuSetPassword>::iterator it = auLoginServerApp->m_SetPassword.begin();
				it != auLoginServerApp->m_SetPassword.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_Name, (*it).m_PassCrypt) && player.PassCrypt() != (*it).m_NewPassCrypt)
			{
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QObject::tr("UPDATE g_Player SET passcrypt=? WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				ql.addBindValue((*it).m_NewPassCrypt);
				if(!ql.exec())
					qWarning() << "ERROR! PerformQueuedOperations - Player" << player.Id() << "password SHA-1" << (*it).m_PassCrypt
						<< "UPDATE FAILED.";

				// The player record must be flushed from the login cache due to this change.
				DeleteLoginCacheItem(player.Id());
			}
		}
		auLoginServerApp->m_SetPassword.clear();
	}

	// Player password reset.
	if(!auLoginServerApp->m_ResetPassword.isEmpty())
	{
		// Seed dice for generating random passwords.
		AuDice dice;
		dice.TimeSeed();

		for(QVector<AuResetPassword>::iterator it = auLoginServerApp->m_ResetPassword.begin();
				it != auLoginServerApp->m_ResetPassword.end(); it++)
		{
			AuPlayerRec player;
			if(player.Select((*it).m_Name))
			{
				// Generate a new password randomly.
				QString password = dice.RandomPassword(10);

				// Put password into a byte array.
				QByteArray passBytes;
				passBytes.append(password.toUtf8());

				// Create a SHA-1 hash from the password.
				QCryptographicHash passHash(QCryptographicHash::Sha1);
				passHash.addData(passBytes);
				player.SetPassCrypt(passHash.result());

				// Store the new password.
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QObject::tr("UPDATE g_Player SET passcrypt=? WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				ql.addBindValue(player.PassCrypt());
				if(!ql.exec())
					qWarning() << "ERROR! PerformQueuedOperations - Player" << player.Id() << "password" << password
						<< "SHA-1" << passBytes << "UPDATE FAILED.";

				// The player record must be flushed from the login cache due to this change.
				DeleteLoginCacheItem(player.Id());

				// Slip a little "unix" shell code in here to deal with the
				// mail easily. 'sure glad this doesn't have to run on Windows.
				char tmpname[64];
				strcpy(tmpname, "/tmp/AuLoginServerXXXXXX");
				int fd = mkstemp(tmpname);
				if(fd)
				{
					QFile file;
					if(file.open(fd, QIODevice::WriteOnly))
					{
						// Write the message into the file.
						QTextStream mailbody(&file);
						mailbody << (*it).m_Name << ",\n\n";
						mailbody << tr("This message was sent by the Shores of Hazeron account server at your request. "
							"Contact us directly for further assistance or to report abuse of this automated system.\n\n");
						mailbody << tr("Your Shores of Hazeron password is ") <<
							password << ".\n\n";
						mailbody << tr("Thanks for playing.\n\n");
						mailbody << tr("Hazeron Freehold LLC\n");
						mailbody << tr("3900 N. 37th St.\n");
						mailbody << tr("Norfolk, NE 68701\n");
						mailbody << tr("402-379-0181\n");
						mailbody << tr("Shores@Hazeron.com\n");
						mailbody << tr("https://Hazeron.com\n");
						file.close();

						// Mail the file to the address.
						QString mailcmd = QString("mail -s 'Shores of Hazeron Password' %1 < %2").arg(player.Email()).arg(tmpname);
						//sprintf(mailcmd, "mail -s 'Shores of Hazeron Password' "
						//		"%s < %s", (const char*)player.m_email.toLatin1(),
						//		tmpname);
						if(system((const char*)mailcmd.toLatin1()) == -1)
						{
							QString logmsg = QObject::tr("Password NOT sent to %1 at %2 due to system error")
								.arg((*it).m_Name).arg(player.Email());
							g_Au.Log(logmsg);
						}
						else
						{
							QString logmsg = QObject::tr("Password sent to %1 at %2").arg((*it).m_Name).arg(player.Email());
							g_Au.Log(logmsg);
						}
					}
					else
					{
						qWarning("%lu Failed to open temp file %d", (unsigned long)currentThreadId(), fd);
					}

					// Delete the temporary file.
					remove(tmpname);
				}
				else
				{
					qWarning("%lu mkstemp failed.", (unsigned long)currentThreadId());
				}
			}
		}
		auLoginServerApp->m_ResetPassword.clear();
	}

	// Resolve player login attempts
	for(QVector<AuLoginAttempt>::iterator it = auLoginServerApp->m_LoginAttempts.begin();
			it != auLoginServerApp->m_LoginAttempts.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);	// Try to load it from the cache.
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);	// Load from db.

			if(!player.Id())
			{
				// Failed to select player record or the account is disabled.
				(*it).m_Result = AuLoginAttempt::Failure;
			}
			else if((*it).m_TestCredentialsOnly)
			{
				// Password check succeeded. We won't actually login.
				// We don't include unit id, confirmation number, or authority
				// to create empires with this result. The caller doesn't need
				// to know it.
				(*it).m_Result = AuLoginAttempt::Success;
				(*it).m_PlayerId = player.Id();

#if defined(Free2Play)
				// Simply logging in activates an account for the next 30 days while free to play.
				ACCOUNT status;
				if(player.AccountType() == ACCT_Expired)
					status = ACCT_Active;
				else
					status = player.AccountType();
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QString("UPDATE g_Player SET status=%2, activated=?, days=30 WHERE id=%1;")
					.arg(player.Id())
					.arg(status);
				ql.prepare(sq);
				ql.addBindValue(QDateTime::currentDateTimeUtc());
				ql.exec();
#endif
			}
			else
			{
				// Password check succeeded.
				player.InitConfirmation();

				// Login to the database.
				QString sq;
				QSqlQuery ql(QString(), db);
#if defined(Free2Play)
				// Simply logging in activates an account for the next 30 days while free to play.
				ACCOUNT status;
				if(player.AccountType() == ACCT_Expired)
					status = ACCT_Active;
				else
					status = player.AccountType();
				sq = QString("UPDATE g_Player SET status=%2, isLoggedIn='y', confirm=?, activated=?, days=30 WHERE id=%1;")
					.arg(player.Id())
					.arg(status);
				ql.prepare(sq);
				ql.addBindValue(player.ConfirmationNumber());
				ql.addBindValue(QDateTime::currentDateTimeUtc());
#else
				sq = QString("UPDATE g_Player SET isLoggedIn='y', confirm=? WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				ql.addBindValue(player.ConfirmationNumber());
#endif
				if(ql.exec())
				{
					(*it).m_Result = AuLoginAttempt::Success;
					player.SetLoggedIn(true);

					// Include unit id, confirmation number, and authority
					// to create empires with this result.
					(*it).m_AccountExpiry = player.AccountExpiry();
					(*it).m_AccountType = player.AccountType();
					(*it).m_ConfirmNum = player.ConfirmationNumber();
					(*it).m_PlayerId = player.Id();
					for(int iu=0; iu<6; iu++)
						(*it).m_UnitId[iu] = player.AvatarId(iu);

					// Query universe database for avatar information.
					if(!udb.isOpen())
						OpenUniverseDatabase(udb);
					if(udb.isOpen())
					{
						for(int iu=0; iu<6; iu++)
						{
							if((*it).m_UnitId[iu].IsNull())
								continue;
							QString sq;
							QSqlQuery qu(QString(), udb);
							sq = QString("SELECT name, sex, isLefty, hair, shave, dna, dna4, inv FROM a_Person WHERE id=%1;")
								.arg((*it).m_UnitId[iu].GetID());
							qu.prepare(sq);
							if(!qu.exec())
								qWarning() << "error: DtLoginDb::PerformQueuedOperations sq" << sq << "err" << qu.lastError();
							if(qu.next())
							{
								int qv = 0;	// query value index
								(*it).m_UnitName[iu] = qu.value(qv++).toString();
								(*it).m_UnitGender[iu] = (GENDER)qu.value(qv++).toInt();
								(*it).m_UnitHand[iu] = qu.value(qv++).toBool() ? LeftHanded : RightHanded;
								(*it).m_UnitHair[iu] = qu.value(qv++).toInt();
								(*it).m_UnitShave[iu] = qu.value(qv++).toInt();

								// DNA - nu style DNA takes precedence over the old style.
								QByteArray baol = qu.value(qv++).toByteArray();
								QByteArray banu = qu.value(qv++).toByteArray();
								if(banu.isEmpty())
								{
									// Nu is empty so assume old is ok.
									(*it).m_UnitDNA[iu] = new DhDNA(baol);
								}
								else
								{
									// Nu is not empty so use it.
									QDataStream in(banu);
									in.setVersion(QDataStream::Qt_5_7);
									(*it).m_UnitDNA[iu] = AuDNABase::ReadDynamic(in);
								}

								// Gear
								QByteArray ba = qu.value(qv++).toByteArray();
								if(!ba.isEmpty())
								{
									QDataStream in(ba);
									in.setVersion(QDataStream::Qt_4_1);
									AuGear gear;
									in >> gear;
									(*it).m_UnitArmor[iu] = gear.ArmorWorn();
									(*it).m_UnitClothes[iu] = gear.ClothingWorn();
									(*it).m_UnitHat[iu] = gear.HelmetOrHatWorn();
									const AuItem* item = gear.HelmetItemWorn();
									if(item)
										(*it).m_UnitVisor[iu] = item->State();
									else
										(*it).m_UnitVisor[iu] = 0;
								}
							}
							else
							{
								// The avatar was not found. The universe database was opened successfully. We must assume the
								// avatar no longer exists and delete the login database reference to the defunct atom id.
								QSqlQuery qdel(QString(), db);
								QString sqdel = QString("UPDATE g_Player SET idu%2=0 WHERE id=%1;").arg(player.Id()).arg(iu+1);
								qdel.prepare(sqdel);
								qdel.exec();
								(*it).m_UnitId[iu] = AuId();
							}
						}
					}
				}
				else
				{
					(*it).m_Result = AuLoginAttempt::Failure;
				}
			}

			// Cache successful attempts.
			if(player.Id())
				CacheLoginAttempt((*it).m_ClientName, player);
		}
	}

#if defined(AUSTEAM)
	// Resolve steam player login attempts
	for(QVector<AuLoginSteamAttempt>::iterator it = auLoginServerApp->m_LoginSteamAttempts.begin();
			it != auLoginServerApp->m_LoginSteamAttempts.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			// Generate a password from their name.
			QString password((*it).Name() + "*");

			// Put password into a byte array.
			QByteArray passBytes;
			passBytes.append(password);

			// Create a SHA-1 hash from the password.
			QCryptographicHash passHash(QCryptographicHash::Sha1);
			passHash.addData(passBytes);
			QByteArray passCrypt = passHash.result();

			// Attempt to load the player account record.
			AuPlayerRec player;
			QString name((*it).Name());
			//qDebug() << "AuLoginSteamAttempt name" << (*it).Name() << "clientName" << (*it).m_ClientName;
			CheckLoginCache(name, passCrypt, (*it).m_ClientName, player);	// Try to load it from the cache.
			if(!player.Id())
			{
				// Player record was not in the cache.
				if(!player.Select(name))	// Load from db.
				{
					// Player record was not in the database.
					// Create a new login account for the steam player.
#if defined(Free2Play)
					player = AuPlayerRec(name, passCrypt, "", ACCT_Active);
#else
					player = AuPlayerRec(name, passCrypt, "", ACCT_Trial);
#endif
					player.CreateSteam(db);
					//qDebug() << "AuLoginSteamAttempt not in db, created id" << player.Id();
				}
				//else
				//{
					//qDebug() << "AuLoginSteamAttempt already in db, existing id" << player.Id();
				//}
			}

			if(!player.Id())
			{
				// Failed to select or create player record.
				(*it).m_Result = AuLoginSteamAttempt::Failure;
			}
			else if((*it).m_TestCredentialsOnly)
			{
				// Password check succeeded. We won't actually login.
				// We don't include unit id, confirmation number, or authority
				// to create empires with this result. The caller doesn't need
				// to know it.
				(*it).m_Result = AuLoginSteamAttempt::Success;
				(*it).m_PlayerId = player.Id();
			}
			else
			{
				// Password check succeeded.
				player.InitConfirmation();

				// Login to the database.
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QString("UPDATE g_Player SET isLoggedIn='y', confirm=? WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				ql.addBindValue(player.ConfirmationNumber());
				if(ql.exec())
				{
					(*it).m_Result = AuLoginSteamAttempt::Success;
					player.SetLoggedIn(true);

					// Include unit id, confirmation number, and authority
					// to create empires with this result.
					(*it).m_AccountExpiry = player.AccountExpiry();
					(*it).m_AccountType = player.AccountType();
					(*it).m_ConfirmNum = player.ConfirmationNumber();
					(*it).m_PlayerId = player.Id();
					for(int iu=0; iu<6; iu++)
						(*it).m_UnitId[iu] = player.AvatarId(iu);

					// Query universe database for avatar information.
					if(!udb.isOpen())
						OpenUniverseDatabase(udb);
					if(udb.isOpen())
					{
						for(int iu=0; iu<6; iu++)
						{
							if((*it).m_UnitId[iu].IsNull())
								continue;
							QString sq;
							QSqlQuery qu(QString(), udb);
							sq = QString("SELECT name, sex, isLefty, hair, shave, dna, dna4, inv FROM a_Person WHERE id=%1;")
								.arg((*it).m_UnitId[iu].GetID());
							qu.prepare(sq);
							if(!qu.exec())
								qWarning() << "error: DtLoginDb::PerformQueuedOperations sq" << sq << "err" << qu.lastError();
							if(qu.next())
							{
								int qv = 0;	// query value index
								(*it).m_UnitName[iu] = qu.value(qv++).toString();
								(*it).m_UnitGender[iu] = (GENDER)qu.value(qv++).toInt();
								(*it).m_UnitHand[iu] = qu.value(qv++).toBool() ? LeftHanded : RightHanded;
								(*it).m_UnitHair[iu] = qu.value(qv++).toInt();
								(*it).m_UnitShave[iu] = qu.value(qv++).toInt();

								// DNA - nu style DNA takes precedence over the old style.
								QByteArray baol = qu.value(qv++).toByteArray();
								QByteArray banu = qu.value(qv++).toByteArray();
								if(banu.isEmpty())
								{
									// Nu is empty so assume old is ok.
									(*it).m_UnitDNA[iu] = new DhDNA(baol);
								}
								else
								{
									// Nu is not empty so use it.
									QDataStream in(banu);
									in.setVersion(QDataStream::Qt_5_7);
									(*it).m_UnitDNA[iu] = AuDNABase::ReadDynamic(in);
								}

								// Gear
								QByteArray ba = qu.value(qv++).toByteArray();
								if(!ba.isEmpty())
								{
									QDataStream in(ba);
									in.setVersion(QDataStream::Qt_4_1);
									AuGear gear;
									in >> gear;
									(*it).m_UnitArmor[iu] = gear.ArmorWorn();
									(*it).m_UnitClothes[iu] = gear.ClothingWorn();
									(*it).m_UnitHat[iu] = gear.HelmetOrHatWorn();
									const AuItem* item = gear.HelmetItemWorn();
									if(item)
										(*it).m_UnitVisor[iu] = item->State();
									else
										(*it).m_UnitVisor[iu] = 0;
								}
							}
							else
							{
								// The avatar was not found. The universe database was opened successfully. We must assume the
								// avatar no longer exists and delete the login database reference to the defunct atom id.
								QSqlQuery qdel(QString(), db);
								QString sqdel = QString("UPDATE g_Player SET idu%2=0 WHERE id=%1;").arg(player.Id()).arg(iu+1);
								qdel.prepare(sqdel);
								qdel.exec();
								(*it).m_UnitId[iu] = AuId();
							}
						}
					}
				}
				else
				{
					(*it).m_Result = AuLoginSteamAttempt::Failure;
				}
			}

			// Cache successful attempts.
			if(player.Id())
				CacheLoginAttempt((*it).m_ClientName, player);
		}
	}
#endif	// AUSTEAM

	// Player logout attempts
	if(!auLoginServerApp->m_LogoutAttempts.isEmpty())
	{
		for(QVector<AuLogoutAttempt>::iterator it = auLoginServerApp->m_LogoutAttempts.begin();
				it != auLoginServerApp->m_LogoutAttempts.end(); it++)
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_PlayerId, (*it).m_ConfirmNum, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_PlayerId, (*it).m_ConfirmNum);

			if(player.Id() && player.IsLoggedIn())
			{
				QString sq;
				QSqlQuery ql(QString(), db);
				sq = QObject::tr("UPDATE g_Player SET isLoggedIn='n' WHERE id=%1;").arg(player.Id());
				ql.prepare(sq);
				if(!ql.exec())
					qWarning("ERROR! PerformQueuedOperations - Player %d logout UPDATE FAILED.", player.Id());

				// Remove cache items upon logout.
				DeleteLoginCacheItem(player.Id());
			}
		}
		auLoginServerApp->m_LogoutAttempts.clear();
	}

	// Resolve login queries that have been posted.
	for(QVector<AuLoginQuery>::iterator it = auLoginServerApp->m_LoginQueries.begin();
			it != auLoginServerApp->m_LoginQueries.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_PlayerId, (*it).m_ConfirmNum, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_PlayerId, (*it).m_ConfirmNum);

			if(player.Id())
			{
				// Player record was selected.
				if(!player.IsLoggedIn())
					(*it).m_Result = AuLoginQuery::NotLoggedIn;
				else
				{
					// Player is logged in and confirmation number matches.
					(*it).m_Result = AuLoginQuery::LoggedIn;
					for(int iu=0; iu<6; iu++)
						(*it).m_UnitId[iu] = player.AvatarId(iu);
					(*it).m_AccountType = player.AccountType();
					(*it).m_AccountExpiry = player.AccountExpiry();

					// Refresh cache item for this player login.
					CacheLoginAttempt((*it).m_ClientName, player);
				}
			}
			else
			{
				// Player record was not selected.
				(*it).m_Result = AuLoginQuery::BadLogin;
			}
		}
	}

	// Resolve Unit queries that have been posted.
	for(QVector<AuUnitQuery>::iterator it = auLoginServerApp->m_UnitQueries.begin();
			it != auLoginServerApp->m_UnitQueries.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_QueryUnit, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_QueryUnit);

			if(player.Id())
			{
				// Player record was selected.
				(*it).m_Result = AuUnitQuery::Done;
				for(int iu=0; iu<6; iu++)
					(*it).m_UnitId[iu] = player.AvatarId(iu);

				// Refresh cache item for this player login.
				CacheLoginAttempt((*it).m_ClientName, player);
			}
			else
			{
				// Player record was not selected.
				(*it).m_Result = AuUnitQuery::BadUnit;
			}
		}
	}

	// Resolve Query Avatars queries that have been posted.
	for(QVector<AuQueryAvatars>::iterator it = auLoginServerApp->m_QueryAvatars.begin();
			it != auLoginServerApp->m_QueryAvatars.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			AuPlayerRec::QueryAvatars((*it).m_ActiveIds, (*it).m_InactiveIds);
			(*it).m_Result = AuQueryAvatars::Done;
		}
	}

	// Resolve Query Avatars queries that have been posted.
	for(QVector<AuQueryAccountAvatars>::iterator it = auLoginServerApp->m_QueryAccountAvatars.begin();
			it != auLoginServerApp->m_QueryAccountAvatars.end(); it++)
	{
		if(!(*it).IsResolved())
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_AccountId, (*it).m_ConfirmNum, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_AccountId, (*it).m_ConfirmNum);

			if(player.Id())
			{
				// Player record was selected.
				(*it).m_Result = AuQueryAccountAvatars::Done;
				for(int iu=0; iu<6; iu++)
					(*it).m_AvatarId[iu] = player.AvatarId(iu);

				// Query universe database for avatar information.
				if(!udb.isOpen())
					OpenUniverseDatabase(udb);
				if(udb.isOpen())
				{
					for(int iu=0; iu<6; iu++)
					{
						if((*it).m_AvatarId[iu].IsNull())
							continue;
						QString sq;
						QSqlQuery qu(QString(), udb);
						sq = QString("SELECT name FROM a_Person WHERE id=%1;")
							.arg((*it).m_AvatarId[iu].GetID());
						qu.prepare(sq);
						if(!qu.exec())
							qWarning() << "error: DtLoginDb::PerformQueuedOperations sq" << sq << "err" << qu.lastError();
						if(qu.next())
						{
							int qv = 0;	// query value index
							(*it).m_AvatarName[iu] = qu.value(qv++).toString();
						}
					}
				}

				// Refresh cache item for this player login.
				CacheLoginAttempt((*it).m_ClientName, player);
			}
			else
			{
				// Player record was not selected.
				(*it).m_Result = AuQueryAccountAvatars::BadLogin;
			}
		}
	}

	auLoginServerApp->m_LoginMutex.unlock();

	auLoginServerApp->m_LoginContactsMutex.lock();

	// Requests for the Contacts list (buddies and squelchies)
	if(!auLoginServerApp->m_ContactsQueries.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsQueries.begin();
				it != auLoginServerApp->m_ContactsQueries.end(); it++)
		{
			// Attempt to load the player account record.
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				(*it).m_Contacts.clear();
				
				AuContactRec contactrec;
				contactrec.Select(player.Id(), (*it).m_Contacts); // Select the player's contacts into the list of contacts
				
				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
	}

	// Player contacts updates. Replaces all current with new list
	if(!auLoginServerApp->m_ContactsUpdates.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsUpdates.begin(); 
				it != auLoginServerApp->m_ContactsUpdates.end(); it++)
		{
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				AuContactRec contactrec;
				contactrec.Update(player.Id(), (*it).m_Contacts);

				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
		auLoginServerApp->m_ContactsUpdates.clear();
	}

	// Player contacts updates. Add the list to the player's contacts
	if(!auLoginServerApp->m_ContactsAdds.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsAdds.begin(); 
				it != auLoginServerApp->m_ContactsAdds.end(); it++)
		{
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				AuContactRec contactrec;
				contactrec.Add(player.Id(), (*it).m_Contacts);

				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
		auLoginServerApp->m_ContactsAdds.clear();
	}

	// Player contacts updates. Delete the list of contacts from the player's contacts
	if(!auLoginServerApp->m_ContactsDeletes.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsDeletes.begin(); 
				it != auLoginServerApp->m_ContactsDeletes.end(); it++)
		{
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				AuContactRec contactrec;
				contactrec.Delete(player.Id(), (*it).m_Contacts);

				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
		auLoginServerApp->m_ContactsDeletes.clear();
	}

	// Player contacts updates. Add a single contact to the player's contacts
	if(!auLoginServerApp->m_ContactAdds.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactAdds.begin(); 
				it != auLoginServerApp->m_ContactAdds.end(); it++)
		{
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				AuContactRec contactrec;
				contactrec.Add(player.Id(), (*it).m_Contact);

				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
		auLoginServerApp->m_ContactAdds.clear();
	}

	// Player contacts updates. Delete a single contact from the player's contacts
	if(!auLoginServerApp->m_ContactDeletes.isEmpty())
	{
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactDeletes.begin(); 
				it != auLoginServerApp->m_ContactDeletes.end(); it++)
		{
			AuPlayerRec player;
			CheckLoginCache((*it).m_Name, (*it).m_PassCrypt, (*it).m_ClientName, player);
			if(!player.Id())
				player.Select((*it).m_Name, (*it).m_PassCrypt);

			if(player.Id() && player.IsLoggedIn())
			{
				AuContactRec contactrec;
				contactrec.Delete(player.Id(), (*it).m_ContactId);

				(*it).m_Result = AuLoginContacts::Success;		
			}
			else
				(*it).m_Result = AuLoginContacts::Failure;
		}
		auLoginServerApp->m_ContactDeletes.clear();
	}

	auLoginServerApp->m_LoginContactsMutex.unlock();

	db.close();		// We don't keep the real database open.

	if(udb.isOpen())
		udb.close();
}

// Update login cache entry for a given player record.
void DtLoginDb::CacheLoginAttempt(const QString& clientName, const AuPlayerRec& player)
{
	return;	// We no longer cache logins so as not to be out of sync with the database. Servers no longer repeatedly log in.

	// Add/replace the cache item that corresponds to this login attempt
	// using the current date and time.
	AuLoginCacheItem cache;
	cache.m_ClientName = clientName;
	cache.m_LastAccess = QDateTime::currentDateTime();
	cache.m_PlayerRec = player;
	m_LoginCache.insert(player.LoginName(), cache);
}

// Try to load player record from the cache.
void DtLoginDb::CheckLoginCache(const QString& name, const QByteArray& passCrypt, const QString& clientName,
		AuPlayerRec& player) const
{
	if(m_LoginCache.contains(name))
	{
		// The player name is in the cache.
		const AuLoginCacheItem& cache = m_LoginCache[name];

		// Check the password and client name to insure two different clients
		// aren't checking in.
		if(cache.m_PlayerRec.PassCrypt() == passCrypt && cache.m_ClientName == clientName)
		{
			// Password and client name matches. Retrieve the record.
			player = cache.m_PlayerRec;
			//qDebug() << "DtLoginDb::CheckLoginCache HIT" << clientName
				//<< player.Id() << player.m_name << "acct" << player.AccountType();
		}
	}
}

// Try to load player record from the cache.
void DtLoginDb::CheckLoginCache(unsigned int playerId, unsigned int confirmNum, const QString& clientName,
		AuPlayerRec& player) const
{
	for(QMap<QString, AuLoginCacheItem>::const_iterator lci = m_LoginCache.begin(); lci != m_LoginCache.constEnd(); lci++)
	{
		if((*lci).m_PlayerRec.Id() == playerId)
		{
			// The player id is in the cache.
			// Check the confirm number and client name to insure two different
			// clients aren't checking in.
			if((*lci).m_PlayerRec.ConfirmationNumber() == confirmNum &&
					(*lci).m_ClientName == clientName)
			{
				// Password and client name matches. Retrieve the record.
				player = (*lci).m_PlayerRec;
				//qDebug("Successful cache hit: playerId %d clientName %s", playerId, (const char*)clientName.toLatin1());
				//qDebug() << "DtLoginDb::CheckLoginCache HIT" << clientName
					//<< player.Id() << player.m_name << "acct" << player.AccountType();
				break;
			}
		}
	}
}

// Try to load player record from the cache.
void DtLoginDb::CheckLoginCache(const AuId& unitId, const QString& clientName, AuPlayerRec& player) const
{
	for(QMap<QString, AuLoginCacheItem>::const_iterator lci = m_LoginCache.begin(); lci != m_LoginCache.constEnd(); lci++)
	{
		if((*lci).m_PlayerRec.AvatarIndex(unitId) >= 0)
		{
			// The player id is in the cache.
			// Check the confirm number and client name to insure two different
			// clients aren't checking in.
			if((*lci).m_ClientName == clientName)
			{
				// Password and client name matches. Retrieve the record.
				player = (*lci).m_PlayerRec;
				//qDebug("Successful cache hit: playerId %d clientName %s", playerId, (const char*)clientName.toLatin1());
				//qDebug() << "DtLoginDb::CheckLoginCache HIT" << clientName
					//<< player.Id() << player.m_name << "acct" << player.AccountType();
				break;
			}
		}
	}
}

void DtLoginDb::DecayLoginCache()
{
	// Decay time must be at least the interval between city reports so the
	// worker servers stay in the cache until they shut down.
	// Scene clients update their login status at a longer interval than this
	// so they do NOT benefit from the cache. Otherwise, multiple clients could
	// use the same login account.
	int decaySecs = 15 * 60;
	QDateTime now = QDateTime::currentDateTime();
	QMap<QString, AuLoginCacheItem>::iterator lci = m_LoginCache.begin();
	while(lci != m_LoginCache.end())
	{
		if((*lci).m_LastAccess.secsTo(now) > decaySecs)
		{
			//qDebug() << "DtLoginDb::DecayLoginCache decayed" << (*lci).m_ClientName
				//<< (*lci).m_PlayerRec.Id() << (*lci).m_PlayerRec.m_name << "after" << (*lci).m_LastAccess.secsTo(now) << "secs";
			lci = m_LoginCache.erase(lci);
		}
		else
			lci++;
	}
}

void DtLoginDb::DeleteLoginCacheItem(unsigned int playerId)
{
	QMap<QString, AuLoginCacheItem>::iterator lci = m_LoginCache.begin();
	while(lci != m_LoginCache.end())
	{
		if((*lci).m_PlayerRec.Id() == playerId)
			lci = m_LoginCache.erase(lci);
		else
			lci++;
	}
}
