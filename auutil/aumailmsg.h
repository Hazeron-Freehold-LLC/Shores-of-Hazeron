//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailmsg.h,v 1.53 2022/02/19 18:42:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMailMsg_H
#define AuMailMsg_H

#include "auutil.h"
#include "auid.h"
#include "autime.h"
#include <QList>
#include <QString>

class QDataStream;
class QSqlDatabase;
class QTcpSocket;

#define MAIL_MESSAGE_MAX_SUBJECT		100						// Max characters in the subject line text.
#define MAIL_MESSAGE_MAX_TEXT			20000					// Max characters in the body text of MSG_Mail type messages.

class AUUTIL_DLL AuMailMsg
{
public:
	// NOTE: All message types are deleted by the mail server, in DtMailServer::DeleteOldMail. Refer to ListAutoDeletedTypes for decay schedule.
	enum MSGTYPE
	{
		MSG_Mail = 0,
		MSG_CityStatusReport,			// City status report
		MSG_Destroyed,					// Final message to occupants of a destroyed vehicle or spacecraft.
		MSG_CityOccupationReport,
		MSG_CityDistressReport,			// City status report in distress
		MSG_CityIntelligenceReport,
		MSG_CityStatusReportInfo,		// City status report with info
		MSG_DormantStatusReport,		// Dormant city status report
		MSG_DormantOccupationReport,
		MSG_DormantDistressReport,		// Dormant city status report in distress
		MSG_DormantIntelligenceReport,
		MSG_DormantStatusReportInfo,	// Dormant city status report with info
		MSG_OfficerReady,				// Notification that an officer is ready for deployment at a planet.
		MSG_ArchitectCommission,
		MSG_FundsTransfer,
		MSG_ShipLog,
		MSG_ShipLogAlert,
		MSG_ShipLogDistress,
		MSG_ShipLogFinal,
		MSG_Government,					// Message from the government. Office assignments, travel orders, etc.
		MSG_OfficerContact,				// Notification of the whereabouts of an officer.
		MSG_ShipReport,					// Reports are not auto-deleted after while.
		MSG_OfficerDeath,				// Reports are not auto-deleted after while.
		MSG_CityFinalDecayReport,		// Final city status report when decaying from existence.
		MSG_CityDiplomaticMessage,		// Diplomatic message from a player forwarded by a city to its chain of command.
		MSG_CityDiplomaticReceipt,		// Diplomatic message delivery receipt from the mail server.
		MSG_Types						// LAST TYPE ENUM
	};

public:
	quint32		m_id;
	AuTime		m_tmSent;		// date and time message was sent
	quint32		m_idPlayer;		// id of player account that sent the message
	QString		m_nameFrom;		// name of sender
	AuId		m_idFrom;		// atom id of sender. 1 for MSG_Government messages.
	AuId		m_idTo;			// atom id of recipient
	quint8		m_msgType;		// MSGTYPE type of message
	QString		m_subject;		// subject line
	QString		m_body;			// body text
	bool		m_deleted;		// true if client deleted this message

	// Optional information that is useful for navigating to the source of a mail message.
	quint8		m_fromGalaxy;	// GALAXY enum value.
	float		m_fromGalaxyX;	// Galaxy X coord of message origin, in parsecs (pc).
	float		m_fromGalaxyY;	// Galaxy Y coord of message origin, in parsecs (pc).
	float		m_fromGalaxyZ;	// Galaxy Z coord of message origin, in parsecs (pc).
	QString		m_fromSystem;	// name of sender's solar system. May be blank.
	AuId		m_fromSystemId;	// atom id of sender's solar system. May be 0.
	QString		m_fromWorld;	// name of sender's world. May be blank.
	AuId		m_fromWorldId;	// atom id of sender's world. May be 0.

public:
	AuMailMsg();
	AuMailMsg(const AuMailMsg& c);
	AuMailMsg& operator=(const AuMailMsg& c);

	// true if all fields are valid upon return.
	// false if any field remains invalid upon return.
	bool ApplyFieldLimits();

	// Returns true if this message has the location hierarchy in the header. Cities do this in their reports so they can
	// be organized into a tree structure by origin.
	// NOTE: The message header MUST conform to a specific format that can be dissected in aumail.cpp.
	inline bool HasHierarchyHeader() const
	{
		switch(Type())
		{
		case MSG_CityFinalDecayReport:
		case MSG_CityStatusReport:
		case MSG_CityStatusReportInfo:
		case MSG_CityOccupationReport:
		case MSG_CityDistressReport:
		case MSG_CityIntelligenceReport:
		case MSG_DormantStatusReport:
		case MSG_DormantStatusReportInfo:
		case MSG_DormantOccupationReport:
		case MSG_DormantDistressReport:
		case MSG_DormantIntelligenceReport:
			return true;
		case MSG_Mail:
		case MSG_Destroyed:
		case MSG_ArchitectCommission:
		case MSG_CityDiplomaticMessage:
		case MSG_CityDiplomaticReceipt:
		case MSG_FundsTransfer:
		case MSG_Government:
		case MSG_OfficerContact:
		case MSG_OfficerReady:
		case MSG_OfficerDeath:
		case MSG_ShipLog:
		case MSG_ShipLogAlert:
		case MSG_ShipLogDistress:
		case MSG_ShipLogFinal:
		case MSG_ShipReport:
		case MSG_Types:
			break;
		}
		return false;
	}

	// List the types of mail messages that should be automatically deleted by the mail server.
	// This is actually done in DtMailServer::DeleteOldMail.
	static inline void ListAutoDeletedTypes(QList<MSGTYPE>& deleteEveryHour,
			QList<MSGTYPE>& deleteEvery2Days,
			QList<MSGTYPE>& deleteEveryWeek,
			QList<MSGTYPE>& deleteEveryMonth,
			QList<MSGTYPE>& deleteEvery2Months)
	{
		// Cadets send their reports every hour and they don't contain important information.
		deleteEveryHour.append(MSG_OfficerReady);

		// Cities and officers send a lot of reports.
		deleteEvery2Days.append(MSG_CityDistressReport);
		deleteEvery2Days.append(MSG_CityIntelligenceReport);
		deleteEvery2Days.append(MSG_CityOccupationReport);
		deleteEvery2Days.append(MSG_CityStatusReport);
		deleteEvery2Days.append(MSG_CityStatusReportInfo);
		deleteEvery2Days.append(MSG_DormantDistressReport);
		deleteEvery2Days.append(MSG_DormantIntelligenceReport);
		deleteEvery2Days.append(MSG_DormantOccupationReport);
		deleteEvery2Days.append(MSG_DormantStatusReport);
		deleteEvery2Days.append(MSG_DormantStatusReportInfo);
		deleteEvery2Days.append(MSG_OfficerContact);
		deleteEvery2Days.append(MSG_ShipLog);
		deleteEvery2Days.append(MSG_ShipLogAlert);
		deleteEvery2Days.append(MSG_ShipLogDistress);

		// Architect commissions can generate a lot of mail messages and they aren't all that important.
		deleteEveryWeek.append(MSG_ArchitectCommission);
		deleteEveryWeek.append(MSG_FundsTransfer);
		deleteEveryWeek.append(MSG_ShipReport);

		// Regular mail between players can stay around for a month.
		// Final messages are somewhat rare so let them stay around for a long time.
		deleteEveryMonth.append(MSG_CityDiplomaticMessage);
		deleteEveryMonth.append(MSG_CityDiplomaticReceipt);
		deleteEveryMonth.append(MSG_Destroyed);
		deleteEveryMonth.append(MSG_Mail);
		deleteEveryMonth.append(MSG_OfficerDeath);
		deleteEveryMonth.append(MSG_ShipLogFinal);

		// City decay takes a long time so we keep those around for a while afterwards.
		// Government messages are kept for two months due to the announcement of loss of office, which happens after one month.
		deleteEvery2Months.append(MSG_CityFinalDecayReport);
		deleteEvery2Months.append(MSG_Government);

		// Every message type must be accounted for. No mail messages get to live forever.
		Q_ASSERT(deleteEveryHour.count() +
				deleteEvery2Days.count() +
				deleteEveryWeek.count() +
				deleteEveryMonth.count() +
				deleteEvery2Months.count() == MSG_Types);
	}

	// Returns true if this message is important enough to alert the player when it arrives.
	bool IsAlert() const;

	inline bool IsGovtOrigin() const
	{
		switch(Type())
		{
		case MSG_Government:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsOfficerMessage() const
	{
		switch(Type())
		{
		case MSG_OfficerContact:
		case MSG_OfficerReady:
		case MSG_OfficerDeath:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns true if this message is ready to be automatically deleted from the client, based on client mail settings.
	bool IsReadyToAutoDelete(bool doCityReports, int cityOldAge, bool doFleetReports, int fleetOldAge, bool doOfficerReports,
			int officerOldAge) const;

	inline bool IsShipLog() const
	{
		switch(Type())
		{
		case MSG_ShipLog:
		case MSG_ShipLogAlert:
		case MSG_ShipLogDistress:
		case MSG_ShipLogFinal:
		case MSG_ShipReport:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool operator==(const AuMailMsg& c) const
	{
		return m_id == c.m_id;
	}

	// Return an icon appropriate to represent the originator of the message.
	inline QString OriginIcon() const
	{
		switch(Type())
		{
		case MSG_Mail:
			return QString(":data/ContactNew.png");

		case MSG_CityDiplomaticMessage:
			return QString(":data/CommDiplomacy.png");

		case MSG_CityDiplomaticReceipt:
			return QString(":data/MailParcXOrigin.png");

		case MSG_CityFinalDecayReport:
		case MSG_CityStatusReport:
		case MSG_CityStatusReportInfo:
		case MSG_CityOccupationReport:
		case MSG_CityDistressReport:
		case MSG_CityIntelligenceReport:
		case MSG_DormantStatusReport:
		case MSG_DormantStatusReportInfo:
		case MSG_DormantOccupationReport:
		case MSG_DormantDistressReport:
		case MSG_DormantIntelligenceReport:
			return QString(":data/City.png");
		case MSG_ArchitectCommission:
			return QString(":data/Library.png");
		case MSG_Destroyed:
			return QString(":data/TabClose.png");
		case MSG_FundsTransfer:
			return QString(":data/Bank.png");
		case MSG_OfficerContact:
		case MSG_OfficerReady:
			return QString(":data/Officer.png");
		case MSG_OfficerDeath:
			return QString(":data/ShipLogFinal.png");
		case MSG_ShipLog:
		case MSG_ShipReport:
			return QString(":data/FleetShip.png");
		case MSG_ShipLogAlert:
			return QString(":data/FleetShipAlert.png");
		case MSG_ShipLogDistress:
			return QString(":data/FleetShipDistress.png");
		case MSG_ShipLogFinal:
			return QString(":data/FleetShipFinal.png");
		case MSG_Government:
			return QString(":data/MailGovtOrigin.png");
		case MSG_Types:
			break;
		}
		return QString();	// Invalid message type.
	}

	inline int Priority() const
	{
		switch(Type())
		{
		case MSG_CityFinalDecayReport:
			return 6;
		case MSG_CityDiplomaticMessage:
		case MSG_CityDiplomaticReceipt:
			return 5;
		case MSG_CityOccupationReport:
		case MSG_DormantOccupationReport:
			return 4;
		case MSG_CityDistressReport:
		case MSG_CityIntelligenceReport:
		case MSG_DormantDistressReport:
		case MSG_DormantIntelligenceReport:
			return 3;
		case MSG_CityStatusReportInfo:
		case MSG_DormantStatusReportInfo:
			return 2;
		case MSG_CityStatusReport:
		case MSG_DormantStatusReport:
			return 1;
		case MSG_Mail:
		case MSG_Destroyed:
		case MSG_OfficerContact:
		case MSG_OfficerReady:
		case MSG_ArchitectCommission:
		case MSG_FundsTransfer:
		case MSG_Government:
		case MSG_Types:
			break;
		case MSG_ShipLog:
		case MSG_ShipReport:
			return 1;
		case MSG_ShipLogAlert:
			return 2;
		case MSG_ShipLogDistress:
			return 3;
		case MSG_OfficerDeath:
		case MSG_ShipLogFinal:
			return 4;
		}
		return 0;
	}

	void Rx(QDataStream& in);

	void Store(QSqlDatabase& db) const;

	inline MSGTYPE Type() const
	{
		return (MSGTYPE)m_msgType;
	}

	// Return an icon appropriate to represent the type of the message.
	inline QString TypeIcon() const
	{
		switch(Type())
		{
		case MSG_Government:
		case MSG_Mail:
			return QString(":data/MailMessage.png");
		case MSG_CityDiplomaticMessage:
		case MSG_CityDiplomaticReceipt:
			return QString(":data/CommDiplomacy.png");
		case MSG_CityFinalDecayReport:
			return QString(":data/MailCityDecayed.png");
		case MSG_CityStatusReport:
			return QString(":data/MailCityStatus.png");
		case MSG_Destroyed:
			return QString(":data/PostureBerserk.png");
		case MSG_CityOccupationReport:
			return QString(":data/MailCityOccupation.png");
		case MSG_CityDistressReport:
			return QString(":data/MailCityDistress.png");
		case MSG_CityIntelligenceReport:
			return QString(":data/MailCityIntel.png");
		case MSG_CityStatusReportInfo:
			return QString(":data/MailCityStatusInfo.png");
		case MSG_DormantStatusReport:
			return QString(":data/MailCityStatus.png");
		case MSG_DormantOccupationReport:
			return QString(":data/MailCityOccupation.png");
		case MSG_DormantDistressReport:
			return QString(":data/MailCityDistress.png");
		case MSG_DormantIntelligenceReport:
			return QString(":data/MailCityIntel.png");
		case MSG_DormantStatusReportInfo:
			return QString(":data/MailCityStatusInfo.png");
		case MSG_OfficerContact:
		case MSG_OfficerReady:
			return QString(":data/OfficerRank.png");
		case MSG_OfficerDeath:
			return QString(":data/Bugle.png");
		case MSG_ArchitectCommission:
			return QString(":data/c_DesignDisk.png");
		case MSG_FundsTransfer:
			return QString(":data/c_Money.png");
		case MSG_ShipLog:
			return QString(":data/ShipLog.png");
		case MSG_ShipReport:
			return QString(":data/Crewman.png");
		case MSG_ShipLogAlert:
			return QString(":data/ShipLogAlert.png");
		case MSG_ShipLogDistress:
			return QString(":data/ShipLogDistress.png");
		case MSG_ShipLogFinal:
			return QString(":data/ShipLogFinal.png");
		case MSG_Types:
			break;
		}
		return QString();	// Invalid message type.
	}

	void Tx(QDataStream& out) const;
};

Q_DECLARE_METATYPE(AuMailMsg)	// Needed to this use class in queued signal/slot connections. Must also call qRegisterMetaType.

inline unsigned int qHash(const AuMailMsg& key)
{
	return key.m_id;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AUUTIL_DLL AuMailMsgArray : public QList<AuMailMsg>
{
public:
	void AddMessage(const AuMailMsg& msg);

	void DeleteMail(unsigned int idMessage);

	// 'delids' is an output parameter listing message ids to be deleted
	void DeleteMailFrom(const AuId& idSender, QList<quint32>& delids);

	bool GetMailMessage(AuMailMsg& msg) const;	// Get mail message with msg.m_id.

	qint32 HighestId() const;

	void Load(QSqlDatabase& db, const AuId& idRecipient, unsigned int afterMsgId);

	unsigned int Tx(QTcpSocket& tcpSocket);
};

Q_DECLARE_METATYPE(AuMailMsgArray)	// Needed to this use class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUUTIL_DLL AuMailMsgIdList : public QList<quint32>
{
};

Q_DECLARE_METATYPE(AuMailMsgIdList)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuMailMsg_H
