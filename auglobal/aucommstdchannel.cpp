//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommstdchannel.cpp,v 1.38 2022/02/19 18:33:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommstdchannel.h"
//
#include "aucrypt.h"
#include "audef.h"
#include "dbcommodity.h"
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QVariant>

// Standard channel names, icons, and tooltips.
// The order of this list determines the initial left-to-right order of the channel tabs when opening the default channels.
AuCommStdChannel s_StandardChannels[] =
{
	AuCommStdChannel("Hail", ":data/CommHail.png",
			QObject::tr("Channel to everyone out to solar system range. Stations monitor this channel."),
			AuCommStdChannel::SolarSystem, false, AuCommStdChannel::Everyone, AuCommStdChannel::SolarSystem),
	AuCommStdChannel("Galactic", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Channel to everyone."),
			AuCommStdChannel::InvalidRange, true, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Voice", ":data/CommVoice.png",
			QObject::tr("Channel to everyone nearby who can hear."),
			AuCommStdChannel::Close, false, AuCommStdChannel::Audible, AuCommStdChannel::Close),
	AuCommStdChannel("Intercom", ":data/CommShip.png",
			QObject::tr("Channel to everyone aboard the ship you occupy and to the ship itself. "
				"Vehicles also have an intercom channel."),
			AuCommStdChannel::Close, false, AuCommStdChannel::Intercom, AuCommStdChannel::Close),
	AuCommStdChannel("Crew", ":data/Crewman.png",
			QObject::tr("Channel to crew mates, passengers and the ship. "
				"You must be a ship crew member or passenger to broadcast on this channel."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::CrewMates, AuCommStdChannel::SolarSystem),
	AuCommStdChannel("Trade", ":data/CommTrade.png",
			QObject::tr("Channel to everyone for trading purposes, out to solar system range. "
				"Cities and brokers monitor this channel."),
			AuCommStdChannel::SolarSystem, false, AuCommStdChannel::Everyone, AuCommStdChannel::SolarSystem),
	AuCommStdChannel("Friend", ":data/CommFriend.png",
			QObject::tr("Channel to members of your empire and friendly empires."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Friends, AuCommStdChannel::Universe),
	AuCommStdChannel("Help", ":data/CommHelp.png",
			QObject::tr("Channel to service staff for help playing the game."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Help, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Diplomacy", ":data/CommDiplomacy.png",
			QObject::tr("Channel to members of all governments. Only government officials may broadcast on this channel."),
			AuCommStdChannel::SolarSystem, false, AuCommStdChannel::Diplomacy, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Empire", ":data/CommEmpire.png",
			QObject::tr("Channel to members of your empire."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Empire, AuCommStdChannel::Universe),
	AuCommStdChannel("Government", ":data/CommGovernment.png",
			QObject::tr("Channel to members of your empire's government. "
				"Only government officials may broadcast on this channel."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Government, AuCommStdChannel::Universe),
	AuCommStdChannel("Fleet", ":data/FleetShip.png",
			QObject::tr("Channel to officers of spacecraft in your empire. Only officers in your chain-of-command will respond. "
				"Only spacecraft officers and government officials may broadcast on this channel."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Fleet, AuCommStdChannel::Universe),
	AuCommStdChannel("Company", ":data/CompanyShip.png",
			QObject::tr("Channel to officers of spacecraft in your company. "
				"Anyone may broadcast on this channel. "
				"Your company is the owner of the company ship on which you hold an officer berth; or it is you."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Company, AuCommStdChannel::Universe),
	AuCommStdChannel("Recruit", ":data/CommRecruit.png",
			QObject::tr("Channel to everyone for recruiting purposes."),
			AuCommStdChannel::SolarSystem, false, AuCommStdChannel::Everyone, AuCommStdChannel::SolarSystem, false),
	AuCommStdChannel("Thoughts", ":data/Brain.png",
			QObject::tr("Channel for thoughts vocalized in your own mind."),
			AuCommStdChannel::Universe, false, AuCommStdChannel::Thoughts, AuCommStdChannel::Universe),	// Universe range default
																										// needed for announcements.
	AuCommStdChannel("Agent", ":data/Agent.png",
			QObject::tr("Channel to all agents."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Agent, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Programmer", ":data/Programmer.png",
			QObject::tr("Channel to all programmers."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Programmer, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Architect", ":data/Architect.png",
			QObject::tr("Channel to all architects."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Architect, AuCommStdChannel::Universe, false),

	AuCommStdChannel("Galaktika", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Russian language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galaxie", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("French language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galaxia", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Spanish language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galaktischen", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("German language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	/*
	AuCommStdChannel("Ginga", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Japanese language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Xingxi", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Chinese language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Eunha", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Korean language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Sterrestelsel", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Afrikaans language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Realtra", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Irish language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galakse", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Norwegian language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galassia", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Italian language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
	AuCommStdChannel("Galaxias", ":data/RangeGalaxy.png",	// Cheesy but this icon name indicates a "Galactic" channel.
			QObject::tr("Greek language channel to everyone."),
			AuCommStdChannel::InvalidRange, false, AuCommStdChannel::Everyone, AuCommStdChannel::Universe, false),
			*/
};
const int s_StandardChannelCount = sizeof(s_StandardChannels)/(sizeof(AuCommStdChannel));

//static
QString AuCommStdChannel::ChannelDescription(const QString& channelName, SCOPE scope)
{
	if(scope < Agent || scope >= ScopeTypes)
		return QObject::tr("Channel to everyone.");

	// The description may be based on the channel name.
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return s_StandardChannels[ic].m_Description;

	// Return a description based on the scope.
	QString scopeDesc[] =
	{
		QObject::tr("Channel to all agents."),
		QObject::tr("Channel to all architects."),
		QObject::tr("Channel to crew mates, passengers and the ship. "
				"You must be a ship crew member or passenger to broadcast on this channel."),
		QObject::tr("Channel to members of all governments. Only government officials may broadcast on this channel."),
		QObject::tr("Channel to members of your empire."),
		QObject::tr("Channel to everyone."),
		QObject::tr("Channel to officers of spacecraft in your empire. "	// Fleet
				"Only officers in your chain-of-command will respond. "
				"Only spacecraft officers and government officials may broadcast on this channel."),
		QObject::tr("Channel to members of your empire and friendly empires."),
		QObject::tr("Channel to members of your empire's government. "
				"Only government officials may broadcast on this channel."),
		QObject::tr("Channel to everyone aboard the ship you occupy and to the ship itself. "
				"Vehicles also have an intercom channel."),
		QObject::tr("Channel to everyone associated with the same story."),
		QObject::tr("Channel to all programmers."),
		QObject::tr("Channel to officers of spacecraft in your squadron. "
				"Only officers in your chain-of-command will respond. "
				"Only spacecraft officers and government officials may broadcast on this channel."),
		QObject::tr("Channel to service staff for help playing the game."),
		QObject::tr("Channel for thoughts vocalized in your own mind."),
		QObject::tr("Channel for audible messages to anyone who can hear."),
		QObject::tr("Channel to officers of spacecraft in your company. "	// Company
				"Only officers in your chain-of-command will respond. "
				"Anyone may broadcast on this channel. "
				"Your company is the owner of the company ship on which you hold an officer berth; or it is you."),
	};
	return scopeDesc[scope];
}

//static
bool AuCommStdChannel::ChannelHasRange(const QString& channelName)
{
	if(VoiceChannelName() == channelName || IntercomChannelName() == channelName || ThoughtsChannelName() == channelName)
		return false;
	return true;
}

//static
QString AuCommStdChannel::ChannelIconName(const QString& channelName, SCOPE scope)
{
	if(scope < Agent || scope >= ScopeTypes)
		return QString(":data/CommChannel.png");

	// In Everyone scope, the icon may be based on the channel name.
	if(scope == AuCommStdChannel::Everyone)
		for(int ic=0; ic<s_StandardChannelCount; ic++)
			if(s_StandardChannels[ic].m_Scope == Everyone && s_StandardChannels[ic].m_Name == channelName)
				return s_StandardChannels[ic].m_Icon;			// Found a matching standard channel name with Everyone scope.

	// Return an icon based on the scope.
	QString scopeIcon[] =
	{
		QString(":data/Agent.png"),
		QString(":data/Architect.png"),
		QString(":data/Crewman.png"),
		QString(":data/CommDiplomacy.png"),
		QString(":data/CommEmpire.png"),
		QString(":data/CommChannel.png"),
		QString(":data/FleetShip.png"),
		QString(":data/CommFriend.png"),
		QString(":data/CommGovernment.png"),
		QString(":data/CommShip.png"),
		QString(":data/CommNarrator.png"),
		QString(":data/Programmer.png"),
		QString(":data/FleetShip.png"),
		QString(":data/CommHelp.png"),
		QString(":data/Brain.png"),
		QString(":data/CommVoice.png"),
		QString(":data/CompanyShip.png"),			// Company
	};
	return scopeIcon[scope];
}

//static
AuCommStdChannel::RANGE AuCommStdChannel::DefaultRange(const QString& channelName)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return s_StandardChannels[ic].m_DefaultRange;
	return Universe;
}

//static
QString AuCommStdChannel::DiplomacyChannelName()
{
	return QString("Diplomacy");
}

//static
QString AuCommStdChannel::EmpireChannelName()
{
	return QString("Empire");
}

//static
QString AuCommStdChannel::ThoughtsChannelName()
{
	return QString("Thoughts");
}

//static
QString AuCommStdChannel::CrewChannelName()
{
	return QString("Crew");
}

//static
QString AuCommStdChannel::FriendChannelName()
{
	return QString("Friend");
}

//static
QString AuCommStdChannel::CompanyChannelName()
{
	return QString("Company");
}

//static
QString AuCommStdChannel::FleetChannelName()
{
	return QString("Fleet");
}

//static
QString AuCommStdChannel::GovernmentChannelName()
{
	return QString("Government");
}

//static
QString AuCommStdChannel::HailChannelName()
{
	return QString("Hail");
}

//static
QString AuCommStdChannel::HelpChannelName()
{
	return QString("Help");
}

//static
QString AuCommStdChannel::IntercomChannelName()
{
	return QString("Intercom");
}

//static
QString AuCommStdChannel::RecruitChannelName()
{
	return QString("Recruit");
}

//static
QString AuCommStdChannel::TradeChannelName()
{
	return QString("Trade");
}

//static
QString AuCommStdChannel::VoiceChannelName()
{
	return QString("Voice");
}

//static
bool AuCommStdChannel::IsAgentChannel(const QString& channelName)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return s_StandardChannels[ic].m_Scope == Agent ||
				s_StandardChannels[ic].m_Scope == Programmer ||
				s_StandardChannels[ic].m_Scope == Architect;
	return false;
}

//static
bool AuCommStdChannel::IsHailChannel(const QString& channelName)
{
	return channelName == HailChannelName();
}

//static
bool AuCommStdChannel::IsRecruitChannel(const QString& channelName)
{
	return channelName == RecruitChannelName();
}

//static
bool AuCommStdChannel::IsTradeChannel(const QString& channelName)
{
	return channelName == TradeChannelName();
}

//static
bool AuCommStdChannel::IsVoiceChannel(const QString& channelName)
{
	return channelName == VoiceChannelName();
	//for(int ic=0; ic<s_StandardChannelCount; ic++)
		//if(s_StandardChannels[ic].m_Name == channelName)
			//return (s_StandardChannels[ic].m_Scope == AuCommMessage::Intercom) ? true : false;
	//return false;
}

//static
bool AuCommStdChannel::IsStandardChannel(const QString& channelName)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return true;
	return false;
}

//static
void AuCommStdChannel::ListDefaultChannels(QStringList& channelNames)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_OpenByDefault)
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
void AuCommStdChannel::ListOfficerReportChannels(QStringList& channelNames)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(IsOfficerReportChannel(s_StandardChannels[ic].m_Name))
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
bool AuCommStdChannel::IsOfficerReportChannel(const QString& channelName)
{
	return !IsAgentChannel(channelName) &&
		!IsGalacticChannel(channelName) &&
		!IsRecruitChannel(channelName) &&
		channelName != DiplomacyChannelName() &&
		channelName != HelpChannelName() &&
		channelName != ThoughtsChannelName();
}

//static
void AuCommStdChannel::ListStandardChannels(QStringList& channelNames, bool isMinion)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(!s_StandardChannels[ic].IsAgentChannel() &&
				(!isMinion || s_StandardChannels[ic].m_OkForMinion))
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
void AuCommStdChannel::ListAgentChannels(QStringList& channelNames)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Scope == Agent)
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
void AuCommStdChannel::ListProgrammerChannels(QStringList& channelNames)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Scope == Programmer)
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
void AuCommStdChannel::ListArchitectChannels(QStringList& channelNames)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Scope == Architect)
			channelNames.append(s_StandardChannels[ic].m_Name);
}

//static
bool AuCommStdChannel::IsOpenedAutomatically(const QString& channelName, RANGE senderActualRange, bool isMinion)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
	{
		if(isMinion && !s_StandardChannels[ic].m_OkForMinion)
			continue;
		if(s_StandardChannels[ic].m_Name == channelName)
			return senderActualRange <= s_StandardChannels[ic].m_AutoOpenRange;
	}
	return false;
}

//static
AuCommStdChannel::RANGE AuCommStdChannel::MaxChannelRange(const QString& channelName)
{
	if(IsHailChannel(channelName) || IsTradeChannel(channelName))
		return SolarSystem;
	else if(!ChannelHasRange(channelName))
		return DefaultRange(channelName);
	return Universe;
}

//static
AuCommStdChannel::SCOPE AuCommStdChannel::Scope(const QString& channelName)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return s_StandardChannels[ic].m_Scope;
	return Everyone;
}

//static
int AuCommStdChannel::ChannelIndex(const QString& channelName)
{
	for(int ic=0; ic<s_StandardChannelCount; ic++)
		if(s_StandardChannels[ic].m_Name == channelName)
			return ic;
	return -1;
}

//static
QString AuCommStdChannel::ChannelName(int channelIndex)
{
	if(channelIndex >= 0 && channelIndex < s_StandardChannelCount)
		return s_StandardChannels[channelIndex].m_Name;
	return QString();
}

//static
double AuCommStdChannel::CloseRangeDistance()
{
	return 300 * FeetPerMeter();
}

//static
bool AuCommStdChannel::IsWithinRange(double atDistance, RANGE testRange)
{
	if(testRange == Hail)
	{
		// Hail/Orbital range
		if(atDistance < FeetPerOrbit())
			return true;
	}
	else if(testRange == Close)
	{
		// Voice/Close range
		if(atDistance < CloseRangeDistance())
			return true;
	}
	return false;
}

//static
bool AuCommStdChannel::IsOkForMinion(const QString& channelName)
{
	int channelIndex = ChannelIndex(channelName);
	if(channelIndex >= 0 && channelIndex < s_StandardChannelCount)
		return s_StandardChannels[channelIndex].m_OkForMinion;
	return true;
}

//static
AuCommStdChannel::RANGE AuCommStdChannel::LimitRange(AuCommStdChannel::RANGE range, AuCommStdChannel::RANGE maxRange)
{
	switch(maxRange)
	{
	case InvalidRange:
	case RANGE_Count:
		break;
	case Close:
		return Close;
	case Hail:
		if(range > Hail)
			return Hail;
		break;
	case SolarSystem:
		if(range > SolarSystem)
			return SolarSystem;
		break;
	case Sector:
		if(range > Sector)
			return Sector;
		break;
	case Galaxy:
		if(range > Galaxy)
			return Galaxy;
		break;
	case Universe:
		if(range > Universe)
			return Universe;
		break;
	}
	return range;
}

//static
QString AuCommStdChannel::RangeDesc(RANGE range)
{
	if(range >= Close && range < RANGE_Count)
	{
		QString rangeDesc[RANGE_Count] =
		{
			QString("Close range reaches about three hundred meters."),
			QString("Hail range reaches an entire planet and its moons. "
					"May reach neighboring planets when orbital positions are favorable."),
			QString("Solar system range reaches the entire solar system."),
			QString("Sector range reaches the 3x3x3 cube of sectors that is centered on your current sector."),
			QString("Galaxy range reaches the entire galaxy."),
			QString("Universe range reaches the entire universe of Hazeron."),
		};
		return rangeDesc[range];
	}
	return QString("RangeDesc=%1").arg(range);
}

