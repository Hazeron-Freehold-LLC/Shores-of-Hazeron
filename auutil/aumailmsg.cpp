//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailmsg.cpp,v 1.31 2022/02/19 18:42:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailmsg.h"
//
#include "dtmailserver.h"
#include <QDataStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpSocket>
#include <QVariant>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuMailMsgMetaType = qRegisterMetaType<AuMailMsg>();
static int s_AuMailMsgArrayMetaType = qRegisterMetaType<AuMailMsgArray>();
static int s_AuMailMsgIdList = qRegisterMetaType<AuMailMsgIdList>();

AuMailMsg::AuMailMsg()
{
	m_deleted = false;	// true if client deleted this message
	m_id = 0;
	m_idPlayer = 0;
	m_msgType = MSG_Mail;	// type of message. 0=mail, 1=city report, 2=helpful tip
	m_fromGalaxy = GALAXY_Count;		// We'll use this for the "invalid" state.
	m_fromGalaxyX = 0;
	m_fromGalaxyY = 0;
	m_fromGalaxyZ = 0;
}

AuMailMsg::AuMailMsg(const AuMailMsg& c)
{
	m_id = c.m_id;
	m_idPlayer = c.m_idPlayer;
	m_tmSent = c.m_tmSent;
	m_nameFrom = c.m_nameFrom;
	m_idFrom = c.m_idFrom;
	m_idTo = c.m_idTo;
	m_msgType = c.m_msgType;
	m_subject = c.m_subject;
	m_body = c.m_body;
	m_deleted = c.m_deleted;
	m_fromGalaxy = c.m_fromGalaxy;
	m_fromGalaxyX = c.m_fromGalaxyX;
	m_fromGalaxyY = c.m_fromGalaxyY;
	m_fromGalaxyZ = c.m_fromGalaxyZ;
	m_fromSystemId = c.m_fromSystemId;
	m_fromSystem = c.m_fromSystem;
	m_fromWorldId = c.m_fromWorldId;
	m_fromWorld = c.m_fromWorld;
}

AuMailMsg& AuMailMsg::operator=(const AuMailMsg& c)
{
	m_id = c.m_id;
	m_idPlayer = c.m_idPlayer;
	m_tmSent = c.m_tmSent;
	m_nameFrom = c.m_nameFrom;
	m_idFrom = c.m_idFrom;
	m_idTo = c.m_idTo;
	m_msgType = c.m_msgType;
	m_subject = c.m_subject;
	m_body = c.m_body;
	m_deleted = c.m_deleted;
	m_fromGalaxy = c.m_fromGalaxy;
	m_fromGalaxyX = c.m_fromGalaxyX;
	m_fromGalaxyY = c.m_fromGalaxyY;
	m_fromGalaxyZ = c.m_fromGalaxyZ;
	m_fromSystemId = c.m_fromSystemId;
	m_fromSystem = c.m_fromSystem;
	m_fromWorldId = c.m_fromWorldId;
	m_fromWorld = c.m_fromWorld;
	return *this;
}

void AuMailMsg::Tx(QDataStream& out) const
{
	out << m_id;
	//out << m_idPlayer;	// client doesn't need this info
	out << m_tmSent.UTC();
	out << m_nameFrom;		// name of sender
	out << m_idFrom;		// atom id of sender
	out << m_idTo;			// atom id of recipient

	// Squeezing in system id and city id in a way that doesn't mess with old messages, which transmitted the type as a ui16.
	quint16 ui16 = m_msgType;									// Type of message in bottom byte.
	if(!m_fromSystemId.IsNull())
		ui16 |= 0x0100;
	if(!m_fromWorldId.IsNull())
		ui16 |= 0x0200;
	if(m_fromGalaxy != GALAXY_Count)
		ui16 |= 0x0400;

	out << ui16;

	out << m_subject;		// subject line
	out << m_body;			// body text
	out << m_deleted;		// true if client deleted this message

	if(ui16 & 0x0100)
	{
		out << m_fromSystemId;
		out << m_fromSystem;
	}
	if(ui16 & 0x0200)
	{
		out << m_fromWorldId;
		out << m_fromWorld;
	}
	if(ui16 & 0x0400)
	{
		out << m_fromGalaxy;
		out << m_fromGalaxyX;
		out << m_fromGalaxyY;
		out << m_fromGalaxyZ;
	}
}

void AuMailMsg::Rx(QDataStream& in)
{
	in >> m_id;
	//in >> m_idPlayer;	// client doesn't need this info
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_tmSent = tm;
	in >> m_nameFrom;		// name of sender
	in >> m_idFrom;			// atom id of sender
	in >> m_idTo;			// atom id of recipient

	quint16 ui16;
	in >> ui16;
	m_msgType = (MSGTYPE)(ui16 & 0x00ff);						// type of message in bottom byte

	in >> m_subject;		// subject line
	in >> m_body;			// body text
	in >> m_deleted;		// true if client deleted this message

	if(ui16 & 0x0100)
	{
		in >> m_fromSystemId;
		in >> m_fromSystem;
	}
	else
	{
		m_fromSystemId = AuId(0);
		m_fromSystem = "";
	}
	if(ui16 & 0x0200)
	{
		in >> m_fromWorldId;
		in >> m_fromWorld;
	}
	else
	{
		m_fromWorldId = AuId(0);
		m_fromWorld = "";
	}
	if(ui16 & 0x0400)
	{
		in >> m_fromGalaxy;
		in >> m_fromGalaxyX;
		in >> m_fromGalaxyY;
		in >> m_fromGalaxyZ;
	}
	else
	{
		m_fromGalaxy = GALAXY_Count;
		m_fromGalaxyX = 0;
		m_fromGalaxyY = 0;
		m_fromGalaxyZ = 0;
	}
}

void AuMailMsg::Store(QSqlDatabase& db) const
{
	if(m_id == 0)
		return;
	if(!db.isOpen())
		return;

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO g_Mail (id,tmSent,idPlayer,nameFrom,idFrom,idTo,msgType,"
			"subject,body,deleted,fromSystemId,fromSystem,fromWorldId,fromWorld,fromGalaxy,fromGalaxyX,fromGalaxyY,fromGalaxyZ"
			") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(m_id);
	qa.addBindValue(m_tmSent.UTC());
	qa.addBindValue(m_idPlayer);
	qa.addBindValue(m_nameFrom);
	qa.addBindValue(m_idFrom.GetID());
	qa.addBindValue(m_idTo.GetID());
	qa.addBindValue(m_msgType);
	qa.addBindValue(m_subject);
	qa.addBindValue(m_body);
	qa.addBindValue(m_deleted);
	qa.addBindValue(m_fromSystemId.GetID());
	qa.addBindValue(m_fromSystem);
	qa.addBindValue(m_fromWorldId.GetID());
	qa.addBindValue(m_fromWorld);
	qa.addBindValue(m_fromGalaxy);
	qa.addBindValue(m_fromGalaxyX);
	qa.addBindValue(m_fromGalaxyY);
	qa.addBindValue(m_fromGalaxyZ);

	if(!qa.exec())
		qWarning() << "AuMailMsg::Store INSERT FAILED!" << m_id << m_idFrom << m_nameFrom << m_subject;
}

bool AuMailMsg::IsAlert() const
{
	switch(Type())
	{
	case MSG_Mail:
	case MSG_CityDiplomaticMessage:
	case MSG_CityDiplomaticReceipt:
	case MSG_CityDistressReport:
	case MSG_CityFinalDecayReport:
	case MSG_Destroyed:
	case MSG_DormantDistressReport:
	case MSG_OfficerDeath:
	case MSG_ShipLogAlert:
	case MSG_ShipLogDistress:
	case MSG_ShipLogFinal:
		return true;
	default:
		break;
	}
	return false;
}

// true if all fields are valid upon return.
// false if any field remains invalid upon return.
bool AuMailMsg::ApplyFieldLimits()
{
	if(m_tmSent.IsNull())
		m_tmSent.Set();

	// Player id is not transmitted via Tx/Rx so don't validate it here. We already
	// handle this appropriately.
	//if(!m_idPlayer)
		//return false;		// Cannot make this into a valid value.

	m_nameFrom = m_nameFrom.trimmed();
	if(m_nameFrom.isEmpty())
		return false;

	if(m_idFrom.IsNull())
		return false;		// Cannot make this into a valid value.

	if(m_idTo.IsNull())
		return false;		// Cannot make this into a valid value.

	if(m_msgType >= MSG_Types)
		return false;		// Cannot make this into a valid value.

	m_subject = m_subject.trimmed();
	m_body = m_body.trimmed();
	if(m_subject.isEmpty() && m_body.isEmpty())
		return false;		// Need at least one of the two, subject or body.

	if(m_deleted)
		return false;

	m_nameFrom.truncate(MaxNameLength()*2 + 1);	// Allow for maximum title length plus maximum name length and a space.
	m_subject.truncate(MAIL_MESSAGE_MAX_SUBJECT);
	if(m_msgType == MSG_Mail)
		m_body.truncate(MAIL_MESSAGE_MAX_TEXT);	// Regular mail is limited in length.
	return true;
}

bool AuMailMsg::IsReadyToAutoDelete(bool doCityReports, int cityOldAge, bool doFleetReports, int fleetOldAge, bool doOfficerReports,
		int officerOldAge) const
{
	switch(Type())
	{
	case MSG_ArchitectCommission:
	case MSG_CityDiplomaticMessage:
	case MSG_CityDiplomaticReceipt:
	case MSG_CityFinalDecayReport:
	case MSG_Destroyed:
	case MSG_FundsTransfer:
	case MSG_Government:
	case MSG_Mail:
	case MSG_OfficerDeath:
	case MSG_ShipLogFinal:
	case MSG_ShipReport:
	case MSG_Types:
		// These types are not auto deleted on the client based on mail settings.
		break;

	case MSG_CityDistressReport:
	case MSG_CityIntelligenceReport:
	case MSG_CityOccupationReport:
	case MSG_CityStatusReport:
	case MSG_CityStatusReportInfo:
	case MSG_DormantDistressReport:
	case MSG_DormantIntelligenceReport:
	case MSG_DormantOccupationReport:
	case MSG_DormantStatusReport:
	case MSG_DormantStatusReportInfo:
		// Routine city reports.
		if(doCityReports && m_tmSent.ElapsedSec() > cityOldAge * 60 * 60)
			return true;
		break;

	case MSG_OfficerReady:
		// Cadet ready for deployment.
		if(doOfficerReports && m_tmSent.ElapsedSec() > officerOldAge * 60 * 60)
			return true;
		break;

	case MSG_ShipLog:
	case MSG_ShipLogAlert:
	case MSG_ShipLogDistress:
	case MSG_OfficerContact:
		// Fleet contacts.
		if(doFleetReports && m_tmSent.ElapsedSec() > fleetOldAge * 60 * 60)
			return true;
		break;
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuMailMsgArray::AddMessage(const AuMailMsg& msg)
{
	// First look for existing message with same id to replace.
	for(AuMailMsgArray::iterator it = begin(); it != end(); it++)
		if((*it).m_id == msg.m_id)
		{
			// Replace existing message.
			//qDebug("AuMailMsgArray::AddMessage - Replace id %d from %s type %d "
					//"subject %s", msg.m_id, (const char*)msg.m_nameFrom,
					//msg.m_msgType, (const char*)msg.m_subject);
			*it = msg;
			return;
		}

	// Didn't replace so append nu message.
	//qDebug("AuMailMsgArray::AddMessage - Add id %d from %s type %d "
			//"subject %s", msg.m_id, (const char*)msg.m_nameFrom, msg.m_msgType, 
			//(const char*)msg.m_subject);
	append(msg);
}

qint32 AuMailMsgArray::HighestId() const
{
	quint32 highestId = 0;
	for(AuMailMsgArray::const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_id > highestId)
			highestId = (*it).m_id;
	return highestId;
}

unsigned int AuMailMsgArray::Tx(QTcpSocket& tcpSocket)
{
	unsigned int written = 0;
	for(AuMailMsgArray::iterator it = begin(); it != end(); it++)
	{
		// Force fields to be within acceptable limits.
		// If this can't be done the message is not sent.
		if(!(*it).ApplyFieldLimits())
			continue;

		// Format the mail message packet.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtMailServer::P_Message;			// mail message packet type
		(*it).Tx(out);
		written += AuUtil::WritePacketSize(tcpSocket, ba.size());
		written += tcpSocket.write(ba);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			tcpSocket.waitForBytesWritten(10000);
	}
	return written;
}

void AuMailMsgArray::Load(QSqlDatabase& db, const AuId& idRecipient,
		unsigned int afterMsgId)
{
	clear();

	if(!db.isOpen())
		return;

	AuMailMsg ob;
	QString sq = QString("SELECT id,tmSent,idPlayer,nameFrom,idFrom,idTo,msgType,subject,"
			"body,fromSystemId,fromSystem,fromWorldId,fromWorld,fromGalaxy,fromGalaxyX,fromGalaxyY,fromGalaxyZ "
			"FROM g_Mail WHERE id > %1 AND deleted = 'f' AND idTo = %2 ORDER BY tmSent DESC;")
		.arg(afterMsgId).arg(idRecipient.GetID());

	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		ob.m_id = qry.value(qv++).toUInt();

		QDateTime tmSent = qry.value(qv++).toDateTime();
		tmSent.setTimeSpec(Qt::UTC);
		ob.m_tmSent = tmSent;

		ob.m_idPlayer = qry.value(qv++).toUInt();
		ob.m_nameFrom = qry.value(qv++).toString();
		ob.m_idFrom = AuId(qry.value(qv++).toUInt());
		ob.m_idTo = AuId(qry.value(qv++).toUInt());
		ob.m_msgType = (AuMailMsg::MSGTYPE)qry.value(qv++).toInt();
		ob.m_subject = qry.value(qv++).toString();
		ob.m_body = qry.value(qv++).toString();
		ob.m_fromSystemId = AuId(qry.value(qv++).toUInt());
		ob.m_fromSystem = qry.value(qv++).toString();
		ob.m_fromWorldId = AuId(qry.value(qv++).toUInt());
		ob.m_fromWorld = qry.value(qv++).toString();
		ob.m_fromGalaxy = qry.value(qv++).toInt();
		ob.m_fromGalaxyX = qry.value(qv++).toFloat();
		ob.m_fromGalaxyY = qry.value(qv++).toFloat();
		ob.m_fromGalaxyZ = qry.value(qv++).toFloat();
		ob.m_deleted = false;
		append(ob);
	}
}

// Get mail message with msg.m_id.
// Returns true if found.
bool AuMailMsgArray::GetMailMessage(AuMailMsg& msg) const
{
	for(AuMailMsgArray::const_iterator it = constBegin();
			it != constEnd(); it++)
		if((*it).m_id == msg.m_id)
		{
			msg = *it;
			return true;
		}
	return false;
}

void AuMailMsgArray::DeleteMail(unsigned int idMessage)
{
	for(AuMailMsgArray::iterator it = begin(); it != end(); it++)
		if((*it).m_id == idMessage)
		{
			erase(it);
			return;
		}
}

// delids is output parm
void AuMailMsgArray::DeleteMailFrom(const AuId& idSender, QList<quint32>& delids)
{
	for(AuMailMsgArray::iterator it = begin(); it != end();)
		if((*it).m_idFrom == idSender)
		{
			delids.append((*it).m_id);
			it = erase(it);
		}
		else
			it++;
}
