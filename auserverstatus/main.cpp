//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.74 2022/03/04 16:39:15 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "auutil.h"
#include "auplayeronlinestatus.h"
#include "dtloginserver.h"
#include "dtmailserver.h"
#include "dtmcpacket.h"
#include "dtsspacket.h"
#include <iostream>
#include <QColor>
#include <QCoreApplication>
#include <QTcpSocket>
#include <QTextStream>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void DoTest();
void PrintUsage();
void PrintVersion();
void PollLoginServer();
void PollMailServer();
void PollSceneServer(int dbLoadTime);
void PollServerManager(int dbLoadTime);
void ReportGalacticChat(const QList<AuTime>& posted, const QList<quint8>& galaxies, const QStringList& senders,
		const QStringList& msgs);
void ReportPlayersOnline();

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int main(int argc, char** argv)
{
	bool didSomething = false;
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

	if(args.contains("-l"))
	{
		PollLoginServer();
		didSomething = true;
	}

	if(args.contains("-m"))
	{
		PollMailServer();
		didSomething = true;
	}

	if(args.contains("-t"))
	{
		DoTest();
		didSomething = true;
	}

	if(args.contains("-s"))
	{
		PollSceneServer(60);
		didSomething = true;
	}
	else
	{
		// Look for a '-sNN' argument.
		for(QStringList::const_iterator ai = args.constBegin(); ai != args.constEnd(); ai++)
		{
			if((*ai).left(2) == "-s")
			{
				int dbLoadTime = QVariant((*ai).mid(2)).toInt();
				if(dbLoadTime > 1)
					PollSceneServer(dbLoadTime);
				else
					PollSceneServer(1);
				didSomething = true;
				break;
			}
		}
	}

	if(args.contains("-g"))
	{
		PollServerManager(60);
		didSomething = true;
	}
	else
	{
		// Look for a '-gNN' argument.
		for(QStringList::const_iterator ai = args.constBegin(); ai != args.constEnd(); ai++)
		{
			if((*ai).left(2) == "-g")
			{
				int dbLoadTime = QVariant((*ai).mid(2)).toInt();
				if(dbLoadTime > 1)
					PollServerManager(dbLoadTime);
				else
					PollServerManager(1);
				didSomething = true;
				break;
			}
		}
	}

	if(!didSomething)
		PrintUsage();
}

void PrintUsage()
{
	QTextStream out(stdout);
	out << "Shores of Hazeron Server Status\n";
	out << " Usage: AuServerStatus [-l] [-m] [-s] [-v] [-?]\n";
	out << "  -l      Poll login master server for status. \n";
	out << "  -m      Poll mail master server for status. \n";
	out << "  -s[nn]  Poll scene master server for status on scene channel. 'nn' is estimated database load time in minutes, default 60.\n";
	out << "  -g[nn]  Poll scene master server for status on manager channel. 'nn' is estimated database load time in minutes, default 60.\n";
	out << "  -v      Outputs the program version.\n";
	out << "  -?      Outputs this program usage information.\n";
}

void PrintVersion()
{
	QTextStream out(stdout);
	out << "Shores of Hazeron Server Status " << AuUtil::VersionFull() << '\n';
}

void PollLoginServer()
{
	// Values we will get from the server on successful status query.
	quint8 status = 255;
	QDateTime tmUp;
	quint8 workers = 0;

	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(AuUtil::LoginMasterName(), AuUtil::LoginMasterPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		// Transmit the status query packet to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtLoginServer::P_StatusQuery;
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
				case DtLoginServer::P_StatusQuery:
					block >> status;	// Actual response from server.
					block >> tmUp;
					block >> workers;
					break;

				default:
					status = 100;		// Invalid response from server.
					break;
				}

				// Close the socket.
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
			}

			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(100);
		}
	}
	else
		status = 101;		// Could not connect to server.

	// Output the server status.
	QTextStream out(stdout);
	out << "<b>Login Servers</b><br>";
	switch(status)
	{
	case 0:
		out << "Master Up " << tmUp.toLocalTime().toString("M/d/yyyy h:mm") << " CDT<br>";
		if(workers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Worker Servers</span>";
		else if(workers == 1)
			out << "1 Worker Server";
		else
			out << workers << " Worker Servers";
		break;
	case 100:	// Invalid response from server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Confused</span>";
		break;
	case 101:	// Could not connect to server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Down</span>";
		break;
	case 255:	// Connected but broken communication.
		out << "<span style=\"color: rgb(255, 0, 0);\">Not Up</span>";
		break;
	default:
		out << "<span style=\"color: rgb(255, 0, 0);\">Hacked</span>";
		break;
	}
}

void PollMailServer()
{
	// Values we will get from the server on successful status query.
	quint8 status = 255;
	QDateTime tmUp;
	quint8 workers = 0;

	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(AuUtil::MailMasterName(), AuUtil::MailMasterPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		// Transmit the status query packet to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtMailServer::P_StatusQuery;
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
				case DtMailServer::P_StatusQuery:
					block >> status;	// Actual response from server.
					block >> tmUp;
					block >> workers;
					break;

				default:
					status = 100;		// Invalid response from server.
					qWarning("PollMailStatus received unexpected packet type %d reply", ptype);
					break;
				}

				// Close the socket.
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
			}

			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(100);
		}
	}
	else
		status = 101;		// Could not connect to server.

	// Output the server status.
	QTextStream out(stdout);
	out << "<b>Mail Servers</b><br>";
	switch(status)
	{
	case 0:
		out << "Master Up " << tmUp.toLocalTime().toString("M/d/yyyy h:mm") << " CDT<br>";
		if(workers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Worker Servers</span>";
		else if(workers == 1)
			out << "1 Worker Server";
		else
			out << workers << " Worker Servers";
		break;
	case 100:	// Invalid response from server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Confused</span>";
		break;
	case 101:	// Could not connect to server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Down</span>";
		break;
	case 255:	// Connected but broken communication.
		out << "<span style=\"color: rgb(255, 0, 0);\">Not Up</span>";
		break;
	default:
		out << "<span style=\"color: rgb(255, 0, 0);\">Hacked</span>";
		break;
	}
}

void PollSceneServer(int dbLoadTime)
{
	// Values we will get from the server on successful status query.
	quint8 status = 255;
	QDateTime tmUp;
	quint16 managers = 0;
	quint16 workers = 0;
	quint16 workerCores = 0;
	quint32 players = 0;
	QList<AuTime> posted;
	QList<quint8> galaxies;
	QStringList senders;
	QStringList msgs;

	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(AuUtil::SceneMasterName(), AuUtil::SceneMasterPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		// Transmit the status query packet to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PZ_StatusQuery;
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
				case DtSceneServerPacket::PZ_StatusQuery:
					block >> status;	// Actual response from server.
					block >> tmUp;
					block >> managers;
					block >> workers;
					block >> players;
					block >> workerCores;
					block >> posted;
					block >> galaxies;
					block >> senders;
					block >> msgs;
					break;

				default:
					status = 100;		// Invalid response from server.
					qWarning("PollSceneServer received unexpected packet type %d reply", ptype);
					break;
				}

				// Close the socket.
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
			}

			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(100);
		}
	}
	else
		status = 101;		// Could not connect to server.

	// Output the server status.
	QTextStream out(stdout);
	out << "<b>Scene Servers</b><br>";
	switch(status)
	{
	case 0:
		// During the dbLoadTime, show orange text and an "estimated" ready time.
		if(tmUp.toLocalTime().secsTo(QDateTime::currentDateTime())/60 < dbLoadTime)
			out << "<span style=\"color: rgb(255, 128, 0);\">Master Est. Ready At " <<
				(tmUp.addSecs(dbLoadTime*60)).toLocalTime().toString("M/d/yyyy h:mm") << " CDT</span><br>";
		else
			out << "Master Up " << tmUp.toLocalTime().toString("M/d/yyyy h:mm") << " CDT<br>";
		if(managers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Manager Servers</span>";
		else if(managers == 1)
			out << "1 Manager Server";
		else
			out << managers << " Manager Servers";
		out << "<br>";

		if(workers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Worker Servers</span>";
		else if(workers == 1)
			out << "1 Worker Server";
		else
			out << workers << " Worker Servers";
		out << "<br>";

		if(workerCores == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Atom Managers</span>";
		else if(workerCores == 1)
			out << "1 Atom Manager";
		else
			out << workerCores << " Atom Managers";
		out << "<br>";

		if(players == 0)
			out << "<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">No Avatars Online</a>";
		else if(players == 1)
			out << "<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">1 Avatar Online</a>";
		else
			out << QString("<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">%1 Avatars Online</a>").arg(players);
		break;
	case 100:	// Invalid response from server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Confused</span>";
		break;
	case 101:	// Could not connect to server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Down</span>";
		break;
	case 255:	// Connected but broken communication.
		out << "<span style=\"color: rgb(255, 0, 0);\">Not Up</span>";
		break;
	default:
		out << "<span style=\"color: rgb(255, 0, 0);\">Hacked</span>";
		break;
	}

	//if(players)
		ReportPlayersOnline();

	// Report recent galactic chat.
	ReportGalacticChat(posted, galaxies, senders, msgs);
}

void PollServerManager(int dbLoadTime)
{
	// Values we will get from the server on successful status query.
	quint8 status = 255;
	QDateTime tmUp;
	quint16 managers = 0;
	quint16 workers = 0;
	quint16 workerCores = 0;
	quint32 players = 0;
	QList<AuTime> posted;
	QList<quint8> galaxies;
	QStringList senders;
	QStringList msgs;

	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(AuUtil::SceneMasterName(), 16762);
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		// Transmit the connect packet.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtMgrChannelPacket::P_Connect;
			out << AuId::Haxus();
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);
		}

		// Transmit the status query packet to the server and wait for a reply.
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtMgrChannelPacket::P_StatusQuery;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);
		}

		// The server will send back a reply to the status query, good or bad.
		quint32 packetSize = 0;
		int packetSizeBytesLeft = 0;
		tcpSocket.waitForReadyRead(10000);
		while(tcpSocket.state() == QAbstractSocket::ConnectedState || tcpSocket.bytesAvailable())
		{
			// Pull in any data that is waiting for us.
			while(tcpSocket.bytesAvailable())
			{
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
				case DtMgrChannelPacket::P_StatusQuery:
					block >> status;	// Actual response from server is DtDb::DBSTATUS enum.
					block >> tmUp;
					block >> managers;
					block >> workers;
					block >> players;
					block >> workerCores;
					block >> posted;
					block >> galaxies;
					block >> senders;
					block >> msgs;
					break;

				default:
					status = 100;		// Invalid response from server.
					qWarning("PollSceneServer received unexpected packet type %d reply", ptype);
					break;
				}

				// Close the socket.
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
			}

			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(100);
		}
	}
	else
		status = 101;		// Could not connect to server.

	// Output the server status.
	QTextStream out(stdout);
	out << "<b>Scene Servers</b><br>";
	switch(status)
	{
	case 0:	// NotLoaded,		// Database has not been loaded yet.
		// During the dbLoadTime, show orange text and an "estimated" ready time.
		out << "<span style=\"color: rgb(255, 128, 0);\">Master Loading Universe Database<br>"
			"Load Started " << tmUp.toLocalTime().toString("M/d/yyyy h:mm") << " CDT<br>"
			"Estimated Ready For Players At " << (tmUp.addSecs(dbLoadTime*60)).toLocalTime().toString("M/d/yyyy h:mm") << " CDT"
			"</span>";

		// Other status may be known.
		if(managers)
		{
			out << "<br>";
			if(managers == 1)
				out << "1 Manager Server";
			else
				out << managers << " Manager Servers";

			if(workers)
			{
				out << "<br>";
				if(workers == 1)
					out << "1 Worker Server";
				else
					out << workers << " Worker Servers";

				if(workerCores)
				{
					out << "<br>";
					if(workerCores == 1)
						out << "1 Atom Manager";
					else
						out << workerCores << " Atom Managers";
				}
			}
		}
		break;

	case 1:	// Loaded,			// Database loaded ok.
	case 2:	// Storing,		// Database loaded ok and is currently storing.
		// Master server is up and ready. Manager could still be loading.
		out << "Master Up " << tmUp.toLocalTime().toString("M/d/yyyy h:mm") << " CDT<br>";

		if(managers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Manager Servers</span>";
		else if(managers == 1)
			out << "1 Manager Server";
		else
			out << managers << " Manager Servers";
		out << "<br>";

		if(workers == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Worker Servers</span>";
		else if(workers == 1)
			out << "1 Worker Server";
		else
			out << workers << " Worker Servers";
		out << "<br>";

		if(workerCores == 0)
			out << "<span style=\"color: rgb(255, 0, 0);\">No Atom Managers</span>";
		else if(workerCores == 1)
			out << "1 Atom Manager";
		else
			out << workerCores << " Atom Managers";
		out << "<br>";

		if(players == 0)
			out << "<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">No Avatars Online</a>";
		else if(players == 1)
			out << "<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">1 Avatar Online</a>";
		else
			out << QString("<a class=\"aon\" style=\"color: rgb(112, 127, 255);\" href=\"playerson.php\">%1 Avatars Online</a>").arg(players);
		break;

	case 3:	// OpenError,		// Database did not open ok.
		out << "<span style=\"color: rgb(255, 0, 0);\">Database Open Failed</span>";
		break;

	case 4:	// DriverError,	// Database driver did not init ok.
		out << "<span style=\"color: rgb(255, 0, 0);\">SQL Driver Failed</span>";
		break;

	case 100:	// Invalid response from server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Confused</span>";
		break;
	case 101:	// Could not connect to server.
		out << "<span style=\"color: rgb(255, 0, 0);\">Down</span>";
		break;
	case 255:	// Connected but broken communication.
		out << "<span style=\"color: rgb(255, 0, 0);\">Not Up</span>";
		break;
	default:
		out << "<span style=\"color: rgb(255, 0, 0);\">Hacked</span>";
		break;
	}

	//if(players)
		ReportPlayersOnline();

	// Report recent galactic chat.
	ReportGalacticChat(posted, galaxies, senders, msgs);
}

void ReportPlayersOnline()
{
	// Values we will get from the server on successful players query.
	AuPlayerOnlineStatusVector playersList;

	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(AuUtil::SceneMasterName(), AuUtil::SceneMasterPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		// Transmit the players query packet to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PZ_PlayersQuery;
		out << (quint32)1;			// player id
		out << (quint32)7316411;	// a valid confirmation number (cobbed from a log file)
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
				case DtSceneServerPacket::PZ_PlayersQuery:
					playersList << block;
					break;

				default:
					qWarning("ReportPlayersOnline received unexpected packet type %d reply", ptype);
					break;
				}

				// Close the socket.
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
			}

			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(100);
		}
	}

	{
		// Copied from austylesheet.h
		QColor hazBlue5(25, 54, 128);
		qreal h,s,v;
		hazBlue5.getHsvF(&h, &s, &v);
		QColor hazBlue4;
		hazBlue4.setHsvF(h, s, v * .8f);
		QColor hazBlue3;
		hazBlue3.setHsvF(h, s, v * .6f);
		QColor hazBlue2;
		hazBlue2.setHsvF(h, s, v * .4f);
		QColor hazBlue1;
		hazBlue1.setHsvF(h, s, v * .2f);

		QString s_htmlStyles = "<style>"
			"a "
			"{ "
			" text-decoration: none; "
			"} "
			""
			"a:hover "
			"{ "
			" -webkit-transition-duration: .4s; "
		    " transition-duration: .4s; "
			" color: #ffffff; "
			"} "
			"</style>";

		QString s_htmlHeader = "<html>";

		QString s_bodyStart("<body style=\"color: rgb(112, 127, 255); background-color: rgb(0, 0, 0); font-family: Helvetica, Arial, "
				"sans-serif;\" alink=\"#ffffff\" lang=\"EN-US\" link=\"#a0b3f9\" vlink=\"#a0a0a0\">\n"
				"<?php include(\"sohpageheader.html\"); ?><br>");
		QString titleBar("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>Shores of Hazeron - %1</title></head>\n<link rel=\"shortcut icon\" "
				"href=\"https://Hazeron.com/favicon.png\" type=\"image/png\">\n");
		QString subPageTitle = "<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"color: #ffffff; font-weight: bold;\">%1</big></div><br><span style=\"font-family: "
			"sans-serif;\">%2</span><br><br>\n";
		QString tableStart = QString("<table style=\"text-align: left; width: 100%; background-color: %1; font-family: sans-serif;\" border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"2\">\n<tbody>")
			.arg(hazBlue2.name());
		QString subTableHeader = QString("<tr>"
			"<td colspan=\"2\" style=\"vertical-align: top; background-color: %1;\"><small><b>%2</b><br></small></td>"
			"</tr>\n").arg(hazBlue5.name());
		QString tableRow("<tr>"
			"<td width=\"32\" style=\"vertical-align: top;\"><a href=\"https://Hazeron.com/EmpireStandings/p%2.php\"><img style=\"max-width: %4px; max-height: %5px;\" "
			"align=\"center\" src=\"https://Hazeron.com/%3\"></a></td>"
			"<td style=\"vertical-align: top;\"><small><a class=\"ll\" href=\"https://Hazeron.com/EmpireStandings/p%2.php\">%1</a></small></td>"
			"</tr>\n");

		QString tableEnd = "</tbody>\n</table>\n";
		QString subHtmlFooter = "</body></html>\n";

		// Make a map of the player names so they'll be alphabetized.
		// The multi-map keeps duplicates so the count matches the total number online.
		QMultiMap<QString, AuPlayerOnlineStatus> playersOn;
		QSet<AuId> playersListed;
		for(AuPlayerOnlineStatusVector::const_iterator pli = playersList.constBegin(); pli != playersList.constEnd(); pli++)
		{
			// Skip null entries. Just logging on?
			if((*pli).m_Id.IsNull())
				continue;

			// Prevent duplicates.
			if(playersListed.contains((*pli).m_Id))
				continue;
			playersListed.insert((*pli).m_Id);

			// Hide some incognito agents.
			if((*pli).IsIncognito() && (*pli).m_Id == AuId::Haxus())
				continue;

			playersOn.insert((*pli).m_Name.toUpper(), *pli);
		}

		// History of players online count.
		// This code gets called once per minute. Let's only keep a time hack every 10 minutes.
		QDateTime now = QDateTime::currentDateTimeUtc();
		if(now.time().minute()%10 == 0)
		{
			QFile playerhist;
			playerhist.setFileName("/home/webmaster/hazeron/playerhistory.txt");
			if(playerhist.open(QIODevice::Append))
			{
				QTextStream sro(&playerhist);
				sro.setCodec("UTF-8");

				sro << "'" << now.toString() << "', '" << playersOn.count() << "'\n";
				playerhist.close();
			}
		}

		QFile playersfile;
		playersfile.setFileName("/home/webmaster/hazeron/playerson.php");
		if(playersfile.open(QIODevice::WriteOnly))
		{
			QTextStream sro(&playersfile);
			sro.setCodec("UTF-8");

			sro << s_htmlHeader;
			sro << s_htmlStyles;
			sro << titleBar.arg("Avatars Online");
			sro << s_bodyStart;

			if(playersOn.count())
				sro << subPageTitle
					.arg("Avatars Online")
					.arg(QString("%1 avatars are currently online.").arg(playersOn.count()));
			else
				sro << subPageTitle
					.arg("Avatars Online")
					.arg(QString("No avatars are currently online."));

			sro << tableStart;
			sro << subTableHeader
				.arg("Avatar");

			for(QMultiMap<QString, AuPlayerOnlineStatus>::const_iterator posi = playersOn.constBegin();
					posi != playersOn.constEnd(); posi++)
			{
				int height = 20;
				int width = 20;

				QString plico;
				if((*posi).IsAFK())
					plico = QString("images/ComputerEye.png");
				else if((*posi).IsAgent() && !(*posi).IsIncognito())
					plico = QString("images/SoHLogo32.png");
				else if((*posi).IsChatClient())
					plico = QString("images/Microphone.png");
				else if((*posi).IsNewEmperor())
					plico = QString("images/SurrenderFlag.png");
				else
					plico = QString("images/c_VacSuitHelmet.png");

				// Some wise guy named their avatar Athena%1. :/
				// All % characters are replaced with _.
				QString personName((*posi).m_Name);
				personName.replace("%","_");

				sro << tableRow
					.arg(personName)
					.arg((*posi).m_Id.StringId())
					.arg(plico)
					.arg(width)
					.arg(height);
			}

			sro << tableEnd;
			sro << subHtmlFooter;
			playersfile.close();
		}
	}
}

void DoTest()
{
	QList<AuTime> posted;
	QList<quint8> galaxies;
	QStringList senders;
	QStringList msgs;

	posted << AuTime::Now();
	galaxies << VeilOfTargoss;
	senders << "Haxus";
	msgs << "Is there anybody out there?";

	posted << AuTime::Now();
	galaxies << FallenLegionsOfMuturon;
	senders << "Targoss";
	msgs << "No";

	ReportGalacticChat(posted, galaxies, senders, msgs);
}

void ReportGalacticChat(const QList<AuTime>& posted, const QList<quint8>& galaxies, const QStringList& senders,
		const QStringList& msgs)
{
	QMap<AuTime, QString> messages;

	// Load last known messages.
	QFile msgfile;
	msgfile.setFileName("/home/webmaster/hazeron/Galactic.msgs");
	if(msgfile.open(QIODevice::ReadOnly))
	{
		QDataStream ds(&msgfile);
		ds.setVersion(QDataStream::Qt_4_1);

		ds >> messages;

		msgfile.close();
	}

	// Merge with recently posted messages. There could be duplicates. This gets called every minute and the recent messages data
	// contains all messages in the past five minutes.
	if(!posted.isEmpty())
	{
		// These colors came from the s_Galaxy table in dagalaxy.cpp.
		QColor galaxyColors[] =
		{
			QColor(6, 5, 8),
			QColor(8, 5, 4),
			QColor(3, 5, 8),
			QColor(8, 6, 4),
			QColor(5, 7, 8),
			QColor(8, 4, 3),
			QColor(5, 5, 7),
			QColor(5, 8, 7),
			QColor(4, 5, 8),
			QColor(1, 2, 8),
			QColor(8, 5, 7),
			QColor(2, 4, 8),
			QColor(4, 8, 7),
			QColor(8, 6, 3),
			QColor(2, 8, 8),
			QColor(4, 4, 10),
			QColor(10, 7, 2),
			QColor(5, 6, 8),
			QColor(8, 6, 7),
			QColor(13, 4, 2)
		};

		QList<AuTime>::const_iterator pi = posted.cbegin();
		QList<quint8>::const_iterator gi = galaxies.cbegin();
		QStringList::const_iterator si = senders.cbegin();
		QStringList::const_iterator mi = msgs.cbegin();
		while(pi != posted.cend())
		{
			// Expand links in the text of the message.
			QUrl url = UrlFromString(*mi);

			QString msg(*mi);
			msg.replace("<", "&lt");
			msg.replace(">", "&gt");

			QString msgtext;
			if(url == QUrl())
			{
				if((*gi) >= GALAXY_Count)
				{
					// System broadcast messages are from galaxy GALAXY_Count.
					msgtext = QString("<font color=\"#4e961e\"><small>%1 <small>UTC</small><br>"
							"%3</small></font><br><div style=\"margin-left: 10px;\">"
							"<font color=\"#80fa32\">%4</font></div>\n")
						.arg((*pi).UTC().toString("hh:mm"))
						.arg(*si)
						.arg(msg);

					// Check for the marker that tells us the servers just came up.
					if(msgtext.indexOf("Preparing ") >= 0)
						messages.clear();
				}
				else
				{
					// Regular player messages.
					qreal h,s,v;
					galaxyColors[(*gi)].getHsvF(&h, &s, &v);
					QColor galcol;
					galcol.setHsvF(h, s, 1);

					msgtext = QString("<font color=\"#55aaff\"><small>%1 <small>UTC</small> &middot</font>"
							"<font color=\"%5\"> %2</font><br>"
							"<font color=\"#55aaff\">%3</small></font><br><div style=\"margin-left: 10px;\">"
							"%4</div>\n")
						.arg((*pi).UTC().toString("hh:mm"))
						.arg(GalaxyName((GALAXY)*gi))
						.arg(*si)
						.arg(msg)
						.arg(galcol.name());
				}
			}
			else
			{
				if((*gi) >= GALAXY_Count)
				{
					// System broadcast messages are from galaxy GALAXY_Count.
					msgtext = QString("<font color=\"#4e961e\">"
							"<small>%1 <small>UTC</small><br>"
							"%3</small></font><br>"
							"<div style=\"margin-left: 10px;\">"
							"<a class=\"glink\" href=\"%6\" rel=\"nofollow\">"
							"<font color=\"#80fa32\">%4</font>"
							"</a>"
							"</div>"
							"\n")
						.arg((*pi).UTC().toString("hh:mm"))
						.arg(*si)
						.arg(msg)
						.arg(url.toString());

					// Check for the marker that tells us the servers just came up.
					if(msgtext.indexOf("Preparing ") >= 0)
						messages.clear();
				}
				else
				{
					// Regular player messages.
					qreal h,s,v;
					galaxyColors[(*gi)].getHsvF(&h, &s, &v);
					QColor galcol;
					galcol.setHsvF(h, s, 1);

					msgtext = QString("<font color=\"#55aaff\">"
							"<small>%1 <small>UTC</small> &middot"
							"</font>"
							"<font color=\"%5\"> %2</font><br>"
							"<font color=\"#55aaff\">%3</font></small><br>"
							"<div style=\"margin-left: 10px;\">"
							"<a class=\"glink\" href=\"%6\" rel=\"nofollow\">"
							"%4"
							"</a>"
							"</div>"
							"\n")
						.arg((*pi).UTC().toString("hh:mm"))
						.arg(GalaxyName((GALAXY)*gi))
						.arg(*si)
						.arg(msg)
						.arg(galcol.name())
						.arg(url.toString());
				}
			}

			messages.insert(*pi, msgtext);

			pi++;
			gi++;
			si++;
			mi++;
		}
	}

	// Trim messages to keep only the last ones.
	while(messages.size() > 16)
		messages.erase(messages.begin());

	// Output the messages into the file that is loaded by the web page.
	QFile chatfile;
	chatfile.setFileName("/home/webmaster/hazeron/galactic.html");
	if(chatfile.open(QIODevice::WriteOnly))
	{
		QTextStream sro(&chatfile);
		sro.setCodec("UTF-8");

		if(!messages.isEmpty())
		{
			AuTime prev = messages.cbegin().key();
			for(QMap<AuTime, QString>::const_iterator mi = messages.cbegin(); mi != messages.cend(); mi++)
			{
				// Leave a blank line when there is a time span.
				if(prev.MinsTo(mi.key()) > 3)
					sro << "<br>\n";
				sro << *mi;
				prev = mi.key();
			}
		}
		else
			sro << QString("<font color=\"#55aaff\"><small>No recent chatter.</small></font><br>");

		chatfile.close();
	}

	// Store last known messages
	if(msgfile.open(QIODevice::WriteOnly))
	{
		QDataStream ds(&msgfile);
		ds.setVersion(QDataStream::Qt_4_1);

		ds << messages;

		msgfile.close();
	}
}
