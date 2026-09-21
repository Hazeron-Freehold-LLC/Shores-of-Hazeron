//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.16 2022/04/07 19:22:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "audef.h"
#include "audice.h"
#include "auutil.h"
#include <iostream>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QTextStream>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void ActivateAccounts();
void ExpireAccounts(bool verbose);
void PrintUsage();
void PrintVersion();

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int main(int argc, char** argv)
{
	bool didSomething = false;
	bool verbose = false;
	QCoreApplication a(argc, argv);
	QStringList args = a.arguments();

	// Extract my path from arg 0.
	QFileInfo fileInfo(args.first());
	QString dataPath = fileInfo.path();

	if(args.contains("-v"))
	{
		PrintVersion();
		didSomething = true;
	}

	if(args.contains("-vv"))
		verbose = true;

	if(args.contains("-?"))
	{
		PrintVersion();
		didSomething = true;
	}

	if(!didSomething)
		ExpireAccounts(verbose);	// Deactivate accounts that have expired.
}

void PrintUsage()
{
	QTextStream out(stdout);
	out << "Shores of Hazeron Account Expirer\n";
	out << " Usage: AuAcctExpire [-v] [-?]\n";
	out << "  -v      Outputs the program version.\n";
	out << "  -vv     Verbose output.\n";
	out << "  -?      Outputs this program usage information.\n";
}

void PrintVersion()
{
	QTextStream out(stdout);
	out << "Shores of Hazeron Account Expirer " << AuUtil::VersionFull() << '\n';
}

void ExpireAccounts(bool verbose)
{
	QTextStream out(stdout);
	// auLogin database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
	db.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
	db.setHostName("LoginDb.Hazeron.net");
	if(db.open("audog","Gold"))
	{
		QSet<quint32> expiredAccounts;
		QSqlQuery qry(QString(), db);
		QString sq = QString("SELECT id, name, activated, days FROM g_Player WHERE (status=%1 OR status=%2) AND neverExpire='f';")
			.arg(ACCT_Active)
			.arg(ACCT_Trial);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qv=0;
			quint32 acctId = qry.value(qv++).toUInt();
			QString acctName = qry.value(qv++).toString();
			QDateTime acctActivated = qry.value(qv++).toDateTime();
			acctActivated.setTimeSpec(Qt::UTC);
			int acctDays = qry.value(qv++).toInt();

			// Check for account expiration.
			QDateTime expiry = acctActivated.addDays(acctDays);
			QDateTime utc = QDateTime::currentDateTimeUtc();
			if(utc > expiry)
			{
				// Emperor or trial account expired.
				expiredAccounts.insert(acctId);
				if(verbose)
					out << "Account Expired " << acctId << " " << acctName
						<< " activated " << acctActivated.toString("d/M/yyyy HH:mm t") << " days " << acctDays
						<< " expiry " << expiry.toString("d/M/yyyy HH:mm t") << "\n";
			}
		}

		if(!expiredAccounts.isEmpty())
		{
			// One or more accounts have expired. Change their status to 1=closed in the database.
			QString squ = QString("UPDATE g_Player SET status=%1, confirm=0 WHERE").arg(ACCT_Expired);
			for(QSet<quint32>::const_iterator ei = expiredAccounts.cbegin(); ei != expiredAccounts.cend(); ei++)
			{
				if(ei == expiredAccounts.cbegin())
					squ += QString(" id=%1").arg(*ei);
				else
					squ += QString(" OR id=%1").arg(*ei);
			}
			squ += ";";
			QSqlQuery qryu(squ, db);	// Construct and execute the query.
		}
	}
	else
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
	}
}

void ActivateAccounts()
{
	// auLogin database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
	db.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
	db.setHostName("LoginDb.Hazeron.net");
	if(db.open("audog","Gold"))
	{
		// Activate all accounts that are inactive.
		QString squ = QString("UPDATE g_Player SET status=%1 WHERE status=%2 OR status=%3;")
			.arg(ACCT_Active)
			.arg(ACCT_Expired)
			.arg(ACCT_Trial);
		QSqlQuery qryu(squ, db);	// Construct and execute the query.
	}
	else
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
	}
}

