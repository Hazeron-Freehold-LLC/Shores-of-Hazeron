//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayeronlinestatus.cpp,v 1.13 2022/03/24 14:59:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplayeronlinestatus.h"
//

static int s_AuPlayerOnlineStatusVectorMetaType = qRegisterMetaType<AuPlayerOnlineStatusVector>();

//static
QString AuPlayerOnlineStatus::OsVersionName(quint8 osType, quint16 osVer)
{
	QString osVerName;
	if(osType == 0)
	{
		// Linux
		// SEiLINUXPLATFORM version numbers come from the linuxPlatform.h file.
		//  0 = Unknown
		//  1 = Debian 9
		//  2 = Manjaro 18
		//  3 = Mint 19
		//  4 = Ubuntu 18
		//  5 = Debian 10
		//  6 = Manjaro 20
		//  7 = Ubuntu 20
		//  8 = Manjaro 21
		//  9 = Fedora 35
		//  10 = Debian 11
		switch(osVer)
		{
		case 0:		// Unknown (old build?)
		default:
			osVerName = " ";
			break;
		case 1:
			osVerName = "Debian 9";
			break;
		case 2:
			osVerName = "Manjaro 18";
			break;
		case 3:
			osVerName = "Mint 19";
			break;
		case 4:
			osVerName = "Ubuntu 18";
			break;
		case 5:
			osVerName = "Debian 10";
			break;
		case 6:
			osVerName = "Manjaro 20";
			break;
		case 7:
			osVerName = "Ubuntu 20";
			break;
		case 8:
			osVerName = "Manjaro 21";
			break;
		case 9:
			osVerName = "Fedora 35";
			break;
		case 10:
			osVerName = "Debian 11";
			break;
		}
	}
	else if(osType == 1)
	{
		// Windows
		switch(osVer)
		{
		case QSysInfo::WV_NT:
			osVerName = "NT";
			break;
		case QSysInfo::WV_2000:
			osVerName = "2000";
			break;
		case QSysInfo::WV_XP:
			osVerName = "XP";
			break;
		case QSysInfo::WV_2003:
			osVerName = "2003";
			break;
		case QSysInfo::WV_VISTA:
			osVerName = "Vista";
			break;
		case QSysInfo::WV_WINDOWS7:
			osVerName = "7";
			break;
		case QSysInfo::WV_WINDOWS8:
			osVerName = "8";
			break;
		case QSysInfo::WV_WINDOWS8_1:
			osVerName = "8.1";
			break;
		case QSysInfo::WV_WINDOWS10:
			osVerName = "10";
			break;
		case QSysInfo::WV_DOS_based:
			osVerName = "DOS based";
			break;
		case QSysInfo::WV_NT_based:
			osVerName = "NT based";
			break;
		}
	}
	else if(osType == 2)
	{
		// Mac
		const char* osVerNames[] =
		{
			"Unknown",					// QSysInfo::MV_Unknown
			"OS 9",						// QSysInfo::MV_9
			"OS X 10.0 Cheetah",		// QSysInfo::MV_10_0
			"OS X 10.1 Puma",			// QSysInfo::MV_10_1
			"OS X 10.2 Jaguar",			// QSysInfo::MV_10_2
			"OS X 10.3 Panther",		// QSysInfo::MV_10_3
			"OS X 10.4 Tiger",			// QSysInfo::MV_10_4
			"OS X 10.5 Leopard",		// QSysInfo::MV_10_5
			"OS X 10.6 Snow Leapard",	// QSysInfo::MV_10_6
			"OS X 10.7 Lion",			// QSysInfo::MV_10_7
			"OS X 10.8 Mountain Lion",	// QSysInfo::MV_10_8
			"OS X 10.9 Mavericks",		// QSysInfo::MV_10_9
			"OS X 10.10 Yosemite",		// QSysInfo::MV_10_10
			"OS X 10.11 El Capitan",	// QSysInfo::MV_10_11
			"macOS 10.12 Sierra",		// QSysInfo::MV_10_12
			"macOS 10.13 High Sierra",	// ?
			"macOS 10.14 Mojave",		// ?
		};
		if(osVer < sizeof(osVerNames)/sizeof(const char*))
			osVerName = osVerNames[osVer];
	}

	if(osVerName.isEmpty())
		osVerName = QString("0x%1").arg(osVer, 4, 16, QChar('0'));
	return osVerName;
}

//static
QString AuPlayerOnlineStatus::OsVersionIcon(quint8 osType, quint16 osVer, quint8 wordSize)
{
	QString osIconName;
	if(osType == 0)
	{
		// Linux
		// SEiLINUXPLATFORM version numbers come from the linuxPlatform.h file.
		//  0 = Unknown
		//  1 = Debian 9
		//  2 = Manjaro 18
		//  3 = Mint 19
		//  4 = Ubuntu 18
		//  5 = Debian 10
		//  6 = Manjaro 20
		//  7 = Ubuntu 20
		//  8 = Manjaro 21
		//  9 = Fedora 35
		//  10 = Debian 11
		switch(osVer)
		{
		case 0:		// Unknown (old build?)
		default:
			osIconName = ":data/OS_Linux.png";
			break;
		case 1:		// Debian 9
		case 5:		// Debian 10
		case 10:	// Debian 11
			osIconName = ":data/OS_LinuxDebian.png";
			break;
		case 2:		// Manjaro 18
		case 6:		// Manjaro 20
		case 8:		// Manjaro 21
			osIconName = ":data/OS_LinuxManjaro.png";
			break;
		case 3:		// Mint 19
			osIconName = ":data/OS_LinuxMint.png";
			break;
		case 4:		// Ubuntu 18
		case 7:		// Ubuntu 20
			osIconName = ":data/OS_LinuxUbuntu.png";
			break;
		case 9:		// Fedora 35
			osIconName = ":data/OS_LinuxFedora.png";
			break;
		}
	}
	else if(osType == 1)
	{
		// Windows
		if(wordSize == 32)
			osIconName = ":data/OS_Windows32.png";
		else
			osIconName = ":data/OS_Windows.png";
	}
	else if(osType == 2)
		osIconName = ":data/OS_Mac.png";
	else
		osIconName = ":data/question10x10.png";
	return osIconName;
}
