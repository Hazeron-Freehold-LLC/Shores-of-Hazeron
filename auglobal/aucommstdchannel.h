//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommstdchannel.h,v 1.30 2022/02/19 18:30:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommStdChannel_H
#define AuCommStdChannel_H

#include "augloballib.h"
#include <QIcon>

// Standard channel name, icon, and description.

class AUGLOBAL_DLL AuCommStdChannel
{
public:
	// NOTE: If you add or change RANGE's, update the RangeName function.
	enum RANGE
	{
		InvalidRange = -1,
		Close,				// Within 300m distance in same system.
		Hail,				// Within FeetPerOrbit distance in same system.
		SolarSystem,		// Within same solar system.
		Sector,				// Within same 3x3x3 sector cube centered on sender.
		Galaxy,				// Within same galaxy.
		Universe,			// Within same universe.
		RANGE_Count			// LAST ENTRY MUST BE <= 16 (4 bits). SEE AuCommPacket::operator>>
	};

	// Scope is a channel specific filter for determining the eligible recipients of messages.
	// NOTE: If you add or change SCOPE's, update the ChannelDescription, ChannelIconName, ScopeColor, and ScopeName functions.
	enum SCOPE
	{
		Agent,				// Sender&Recipient must be an agent.
		Architect,			// Sender&Recipient must be an agent, programmer or architect.
		CrewMates,			// Recipient must on roster of same ship. Scope data is ship id on which sender is berthed.
		Diplomacy,			// Sender&Recipient must hold office other than Citizen.
		Empire,				// Recipient must be in the same empire. Scope data is empire id of sender.
		Everyone,			// Recipient can be anyone.
		Fleet,				// Recipient ship must be in sender's chain-of-command. Scope data is empire id of sender.
		Friends,			// Sender&Recipient empires must be friendly toward each other. Scope data is empire id of sender
		Government,			// Sender&Recipient must hold office other than Citizen and they must be in the same empire.
		Intercom,			// Recipient must be aboard the same ship or vehicle.
		Narrator,			// Sender&Recipient must be in the story instance of the message.
		Programmer,			// Sender&Recipient must be an agent or programmer.
		Squadron,			// Recipient ship must be in sender's squadron. Scope data is squadron leader ship id.
		Help,				// Recipient can be anyone.
		Thoughts,			// Recipient is the person who is the target of the message.
		Audible,			// Recipient is anyone who can audibly hear the message.
		Company,			// Scope data is atom id of owner.
		ScopeTypes			// LAST ENTRY
	};

public:
	RANGE								m_AutoOpenRange;		// Sender range (or less) at which this channel opens automatically.
	RANGE								m_DefaultRange;			// Default range when this channel is opened.
	QString								m_Description;			// Description of this channel to use when appropriate.
	QString								m_Icon;					// Name of the icon resource to use when appropriate.
	QString								m_Name;					// Name of this standard channel.
	bool								m_OkForMinion;			// Ok to auto open for Minion/child player account.
	bool								m_OpenByDefault;		// true if this channel is open by default for first-time players.
	SCOPE								m_Scope;				// Scope of messages broadcast on this channel.

	AuCommStdChannel(const QString& name, const QString& icon, const QString& description, RANGE autoOpenRange,
			bool defaultOpen, SCOPE scope, RANGE defaultRange, bool okForMinion=true)
	{
		m_AutoOpenRange = autoOpenRange;
		m_DefaultRange = defaultRange;
		m_Description = description;
		m_Icon = icon;
		m_Name = name;
		m_OkForMinion = okForMinion;
		m_OpenByDefault = defaultOpen;
		m_Scope = scope;
	}

	static QString ChannelDescription(const QString& channelName, SCOPE scope);

	static bool ChannelHasRange(const QString& name);

	static inline QIcon ChannelIcon(const QString& channelName, SCOPE scope)
	{
		return QIcon(ChannelIconName(channelName, scope));
	}

	static QString ChannelIconName(const QString& channelName, SCOPE scope);

	// Returns -1 if 'channelName' is not a standard channel.
	static int ChannelIndex(const QString& channelName);

	static QString ChannelName(int channelIndex);

	static double CloseRangeDistance();

	static QString CompanyChannelName();

	static QString CrewChannelName();

	static RANGE DefaultRange(const QString& channelName);

	static QString DiplomacyChannelName();

	static QString EmpireChannelName();

	static QString FleetChannelName();

	static QString FriendChannelName();

	static QString GovernmentChannelName();

	static QString HailChannelName();

	static QString HelpChannelName();

	static QString IntercomChannelName();

	// True for all 'agent' type channels.
	inline bool IsAgentChannel() const
	{
		return m_Scope == Agent || m_Scope == Programmer || m_Scope == Architect; 
	}

	static bool IsAgentChannel(const QString& channelName);

	// Returns true for all language variations of the Galactic channel.
	static inline bool IsGalacticChannel(const QString& channelName)
	{
		return ChannelIconName(channelName, Everyone) == QString(":data/RangeGalaxy.png");
	}

	static bool IsHailChannel(const QString& channelName);

	static bool IsOfficerReportChannel(const QString& channelName);

	static bool IsOkForMinion(const QString& channelName);

	// Returns true for channels that open automatically for a potential message recipient if they are otherwise eligible to
	// receive the message, i.e. range and scope checks out.
	// 'senderActualRange' is the range of the sender from the recipient, not the range the message was broadcast.
	// 'isMinion' is true if the person is logged on a Minion type player account.
	static bool IsOpenedAutomatically(const QString& channelName, RANGE senderActualRange, bool isMinion);

	static bool IsRecruitChannel(const QString& channelName);

	static bool IsStandardChannel(const QString& channelName);

	static bool IsTradeChannel(const QString& channelName);

	static bool IsVoiceChannel(const QString& channelName);

	static bool IsWithinRange(double atDistance, RANGE testRange);

	// Limits 'range' to the maximum imposed by 'maxRange'
	// Returns the resulting range.
	static RANGE LimitRange(RANGE range, RANGE maxRange);

	// Lists all agent-only channels. Does not clear the incoming list.
	// Does not list architect or programmer channels.
	static void ListAgentChannels(QStringList& channelNames);

	// Lists all architect-only channels. Does not clear the incoming list.
	// Does not list agent or programmer channels.
	static void ListArchitectChannels(QStringList& channelNames);

	static void ListDefaultChannels(QStringList& channelNames);

	// List all standard channels that an officer can be ordered to report on. Omits Galactic and IsAgentChannel channels.
	// Does not clear the incoming list.
	static void ListOfficerReportChannels(QStringList& channelNames);

	// Lists all programmer-only channels. Does not clear the incoming list.
	// Does not list agent or architect channels.
	static void ListProgrammerChannels(QStringList& channelNames);

	// List all standard channels. Omits IsAgentChannel channels. Does not clear the incoming list.
	// 'isMinion' is true if the person is logged on a Minion type player account.
	static void ListStandardChannels(QStringList& channelNames, bool isMinion);

	static RANGE MaxChannelRange(const QString& channelName);

	static QString RangeDesc(RANGE range);

	static inline QString RangeIcon(RANGE range)
	{
		if(range >= Close && range < RANGE_Count)
		{
			QString rangeName[RANGE_Count] =
			{
				QString(":data/OrderPhone.png"),
				QString(":data/RangeHail.png"),
				QString(":data/RangeSystem.png"),
				QString(":data/RangeSector.png"),
				QString(":data/RangeGalaxy.png"),
				QString(":data/RangeUniverse.png"),
			};
			return rangeName[range];
		}
		return "";
	}

	static inline QString RangeName(RANGE range)
	{
		if(range >= Close && range < RANGE_Count)
		{
			QString rangeName[RANGE_Count] =
			{
				QObject::tr("Close"),
				QObject::tr("Hail"),
				QObject::tr("Solar System"),
				QObject::tr("Sector"),
				QObject::tr("Galaxy"),
				QObject::tr("Universe")
			};
			return rangeName[range];
		}
		return QObject::tr("INVALID RANGE");
	}

	static QString RecruitChannelName();

	// Returns the default message scope for a standard channel name.
	// Returns Everyone for all non-standard channel names.
	static SCOPE Scope(const QString& channelName);

	// Used as a basis for coloring on-screen messages.
	// The colors that will be modulated by range have V=100 in the HSV color model.
	// Intercom and Narrator messages are not modulated by range.
	static QColor ScopeColor(SCOPE scope)
	{
		if(scope >= 0 && scope < ScopeTypes)
		{
			QColor scopeColor[ScopeTypes] =
			{
				QColor(255, 25, 76),			// Agent
				QColor(255, 255, 255),			// Architect
				QColor(32, 255, 171),			// CrewMates
				QColor(255, 166, 32),			// Diplomacy
				QColor(96, 96, 255),			// Empire
				QColor(223, 215, 255),			// Everyone
				QColor(32, 255, 255),			// Fleet
				QColor(64, 255, 64),			// Friends
				QColor(145, 31, 255),			// Government
				QColor(26, 65, 255),			// Intercom
				QColor(241, 193, 24),			// Narrator
				QColor(255, 87, 247),			// Programmer
				QColor(32, 181, 255),			// Squadron
				QColor(255, 8, 8),				// Help
				QColor(128, 250, 50),			// Thoughts
				QColor(255, 140, 80),			// Voice
				QColor(32, 181, 255),			// Company
			};
			return scopeColor[scope];
		}
		return QColor(255,192,128);
	}

	static inline QString ScopeName(SCOPE scope)
	{
		if(scope >= 0 && scope < ScopeTypes)
		{
			QString scopeName[ScopeTypes] =
			{
				QObject::tr("Agent"),
				QObject::tr("Architect"),
				QObject::tr("Crew Mates"),
				QObject::tr("Diplomacy"),
				QObject::tr("Empire"),
				QObject::tr("Everyone"),
				QObject::tr("Fleet"),
				QObject::tr("Friends"),
				QObject::tr("Government"),
				QObject::tr("Intercom"),
				QObject::tr("Narrator"),
				QObject::tr("Programmer"),
				QObject::tr("Squadron"),
				QObject::tr("Help"),
				QObject::tr("Thoughts"),
				QObject::tr("Audible"),
				QObject::tr("Company"),
			};
			return scopeName[scope];
		}
		return QObject::tr("INVALID SCOPE");
	}

	inline static bool ScopeHasData(SCOPE scope)
	{
		return scope == AuCommStdChannel::Company ||			// Data is owner id.
			scope == AuCommStdChannel::CrewMates ||				// Data is ship id.
			scope == AuCommStdChannel::Empire ||				// Data is empire id.
			scope == AuCommStdChannel::Fleet ||					// Data is empire id.
			scope == AuCommStdChannel::Friends ||				// Data is empire id.
			scope == AuCommStdChannel::Government ||			// Data is empire id.
			scope == AuCommStdChannel::Squadron;				// Data is empire id.
	}

	static QString ThoughtsChannelName();

	static QString TradeChannelName();

	static QString VoiceChannelName();
};

#endif // AuCommStdChannel_H
