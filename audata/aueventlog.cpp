//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aueventlog.cpp,v 1.62 2022/02/19 18:20:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aueventlog.h"
//
#include <auglobal.h>

#if defined(WIN32)
unsigned int qHash(const AuEventLogEntry& key)
{
	return key.m_Time.MSecsSinceEpoch();	// Probably good enough for Windows.
}
#endif

void AuEventLogEntry::operator>>(QDataStream& out) const
{
	out << m_Text;
	out << m_Time.UTC();
	quint8 hasWhat = m_Type;
	if(!m_AtomId.IsNull())
		hasWhat |= 0x80;
	out << hasWhat;
	if(hasWhat & 0x80)
		out << m_AtomId;
}

void AuEventLogEntry::operator<<(QDataStream& in)
{
	in >> m_Text;

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_Time = tm;

	quint8 hasWhat;
	in >> hasWhat;
	m_Type = (TYPE)(hasWhat & 0x7f);
	if(hasWhat & 0x80)
		in >> m_AtomId;
}

const char* AuEventLogEntry::TypeImageName() const
{
	const char* typeImages[] =
	{
		":data/clBad.png",		// Invalid
		":data/clEComm.png",	// EnemyComm
		":data/BlipVehicleEnemy.png",		// EnemyVehicle
		":data/clNComm.png",	// NeutralComm
		":data/clNVeh.png",		// NeutralVehicle
		":data/clFComm.png",	// FriendComm
		":data/clFVeh.png",		// FriendVehicle
		":data/clMComm.png",	// EmpireComm
		":data/clMVeh.png",		// EmpireVehicle
		":data/BlipAircraftEnemy.png",		// EnemyAircraft
		":data/BlipBoatEnemy.png",			// EnemyBoat
		":data/BlipCapitolCityEnemy.png",	// EnemyCapitol
		":data/BlipCityEnemy.png",			// EnemyCity
		":data/BlipSpacecraftHullEnemy.png",	// EnemySpacecraftHull
		":data/BlipSpaceportEnemy.png",		// EnemyStation
		":data/BlipSpaceportEnemyS.png",	// EnemyStationS
		":data/BlipSpaceshipEnemy.png",		// EnemySpaceship
		":data/BlipSpaceshipEnemyS.png",	// EnemySpaceshipS
		"",
		"",
		":data/BlipStarshipEnemy.png",		// EnemyStarship
		":data/BlipStarshipEnemyS.png",		// EnemyStarshipS
		":data/Damage.png",
		":data/EnemyRangeIn.png",
		":data/EnemyRangeOut.png",
		":data/EventOutbreak.png",
		":data/EventPoison.png",
		":data/Rat.png",
		":data/OrderEnterSurgery.png",
		":data/DNA.png",
		":data/OrderGetOut.png",								// KickedStowaway
		":data/JobPirate.png",									// StandingDown
		":data/c_Money.png",									// Payday
		":data/Locator.png",									// Position
		":data/MailCityDecayed.png",							// BuildingDestroyed
		":data/PostureDefendArea.png",							// FlagLowered
		":data/MailCityOriginOcc.png",							// BuildingAttacked
		":data/PostureBerserk.png",								// CitizenAttacked
		":data/MsgSpot.png",									// EnemySpacecraftSpotted
		":data/MsgSpot.png",									// EnemyTroopSpotted
		":data/MsgSpot.png",									// EnemyVehicleSpotted
		":data/BlipBeacon.png",
		":data/BlipAnomaly.png",
		":data/BlipBaseEnemy.png",								// EnemyBase
		":data/JobPirate.png",									// ShipmentInPlunderedSector:
		":data/JobPirate.png",									// ShipmentInPlunderedSystem:
		":data/JobPirate.png",									// ShipmentOutPlunderedSector:
		":data/JobPirate.png",									// ShipmentOutPlunderedSystem:
		":data/Asteroid.png",									// AsteroidDetected
		"",
		":data/Asteroid.png",									// AsteroidObserved
		":data/Asteroid.png",									// AsteroidObservedCollide
		":data/CommTrade.png",									// TradeProposal
		":data/CommTrade.png",									// TradeAgreement
		":data/Asteroid.png",                                   // AsteroidCollisionBlackHole:
		":data/Asteroid.png",                                   // AsteroidCollisionGasGiant:
		":data/Asteroid.png",                                   // AsteroidCollisionMoon:
		":data/Asteroid.png",                                   // AsteroidCollisionPlanet:
		":data/Asteroid.png",                                   // AsteroidCollisionPlanetoid:
		":data/Asteroid.png",                                   // AsteroidCollisionRingworld:
		":data/Asteroid.png",                                   // AsteroidCollisionSun:
		":data/Asteroid.png",                                   // AsteroidCollisionTitan:
		":data/Asteroid.png",                                   // AsteroidCollisionMoonDestroyed:
		":data/Asteroid.png",                                   // AsteroidCollisionPlanetDestroyed:
		":data/Asteroid.png",                                   // AsteroidCollisionPlanetoidDestroyed:
		":data/Asteroid.png",                                   // AsteroidCollisionTitanDestroyed:
		":data/Asteroid.png",                                   // AsteroidDepartedDeepSpace,
	};
	int typeImageNames = sizeof(typeImages)/sizeof(char*);
	if(m_Type < typeImageNames)
	{
		return typeImages[m_Type];
	}
	else
	{
#if !defined(QT_NO_DEBUG)
		qWarning("AuEventLogEntry::TypeImageName type %d is invalid", m_Type);
#endif
		return typeImages[0];
	}
}

QString AuEventLogEntry::TypeName() const
{
	const char* typeNames[] =
	{
		"Invalid",
		"Enemy Comm",
		"Enemy Vehicle",
		"Neutral Comm",
		"Neutral Vehicle",
		"Friend Comm",
		"Friend Vehicle",
		"Empire Comm",
		"Empire Vehicle",
		"Enemy Aircraft",
		"Enemy Boat",
		"Enemy Capital",
		"Enemy City",
		"Enemy Spacecraft Hull",
		"Enemy Station",
		"Enemy Station w/Shields",
		"Enemy Spaceship",
		"Enemy Spaceship w/Shields",
		"",
		"",
		"Enemy Starship",
		"Enemy Starship w/Shields",
		"Damaged",
		"Entered Enemy Weapon Range",
		"Departed Enemy Weapon Range",
		"Disease",
		"Poison",
		"Infestation",
		"Medical",
		"Citizen Change",
		"Stowaway",
		"Surrender",
		"Payday",
		"Position",
		"Building Destroyed",
		"Flag Lowered",
		"Building Attacked",
		"Citizen Attacked",
		"Enemy Spacecraft Spotted",
		"Enemy Troop Spotted",
		"Enemy Vehicle Spotted",
		"Beacon",
		"Sensor Anomaly",
		"Enemy Base Spotted",
		"Incoming Shipment Plundered",
		"Incoming Shipment Plundered",
		"Outgoing Shipment Plundered",
		"Outgoing Shipment Plundered",
		"Asteroid Detected",
		"",
		"Asteroid Observed",
		"Asteroid Will Collide",								// AsteroidObservedCollide
		"Trade Proposal",										// TradeProposal
		"Trade Agreement",										// TradeAgreement
		"Asteroid Collided with Black Hole",					// AsteroidCollisionBlackHole,
		"Asteroid Collided with Gas Giant",						// AsteroidCollisionGasGiant,
		"Asteroid Collided with Moon",							// AsteroidCollisionMoon,
		"Asteroid Collided with Planet",						// AsteroidCollisionPlanet,
		"Asteroid Collided with Planetoid",						// AsteroidCollisionPlanetoid,
		"Asteroid Collided with Ringworld",						// AsteroidCollisionRingworld,
		"Asteroid Collided with Sun",							// AsteroidCollisionSun,
		"Asteroid Collided with Titan",							// AsteroidCollisionTitan,
		"Asteroid Collision Destroyed Moon",                    // AsteroidCollisionMoonDestroyed:
		"Asteroid Collision Destroyed Planet",                  // AsteroidCollisionPlanetDestroyed:
		"Asteroid Collision Destroyed Planetoid",               // AsteroidCollisionPlanetoidDestroyed:
		"Asteroid Collision Destroyed Titan",                   // AsteroidCollisionTitanDestroyed:
		"Asteroid Orbited to Deep Space",						// AsteroidDepartedDeepSpace:
	};
	int typeNameCount = sizeof(typeNames)/sizeof(char*);
	if(m_Type < typeNameCount)
	{
		return QString(typeNames[m_Type]);
	}
	else
	{
#if !defined(QT_NO_DEBUG)
		qWarning("AuEventLogEntry::TypeName type %d is invalid", m_Type);
#endif
		return typeNames[0];
	}
}

QString AuEventLogEntry::Text() const
{
	switch(m_Type)
	{
	default:
		return m_Text;

	case Anomaly:
		if(m_Text.isEmpty())
			return QObject::tr("Anomalous sensor reading detected.");
		else
			return QObject::tr("Anomalous sensor reading detected. %1").arg(m_Text);

	case Beacon:
		if(m_Text.isEmpty())
			return QObject::tr("Beacon detected.");
		else
			return QObject::tr("Beacon detected, broadcasting: %1").arg(m_Text);

	case AsteroidDetected:
		return QObject::tr("Potentially hazardous asteroid detected. %1").arg(m_Text);

	case AsteroidObserved:
		return QObject::tr("Potentially hazardous asteroid observed. %1").arg(m_Text);

	case AsteroidObservedCollide:
		return QObject::tr("Hazardous asteroid observed! %1").arg(m_Text);

	case AsteroidCollisionGasGiant:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a gas giant. The asteroid was absorbed.");
		else
			return QObject::tr("Asteroid collided with the gas giant %1. The asteroid was absorbed.").arg(m_Text);

	case AsteroidCollisionMoon:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a moon. Any bases there were destroyed. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collided with the moon %1. Any bases there were destroyed. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionPlanet:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a planet. Any cities there were destroyed. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collided with the planet %1. Any cities there were destroyed. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionPlanetoid:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a planetoid. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collided with the planetoid %1. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionRingworld:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a ringworld. Any cities on the section were destroyed.");
		else
			return QObject::tr("Asteroid collided with the ringworld %1. Any cities on the section were destroyed.").arg(m_Text);

	case AsteroidCollisionBlackHole:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a black hole. The asteroid was spaghettified.");
		else
			return QObject::tr("Asteroid collided with the black hole %1. The asteroid was spaghettified.").arg(m_Text);

	case AsteroidCollisionSun:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a sun. The asteroid was absorbed.");
		else
			return QObject::tr("Asteroid collided with the sun %1. The asteroid was absorbed.").arg(m_Text);

	case AsteroidCollisionTitan:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collided with a titan. Any cities there were destroyed. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collided with the titan %1. Any cities there were destroyed. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionMoonDestroyed:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collision destroyed a moon. The rubble formed a ring around its world. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collision destroyed the moon %1. The rubble formed a ring around its world. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionPlanetDestroyed:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collision destroyed a planet. The rubble formed an asteroid belt around its sun. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collision destroyed the planet %1. The rubble formed an asteroid belt around its sun. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionPlanetoidDestroyed:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collision destroyed a planetoid. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collision destroyed the planetoid %1. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidCollisionTitanDestroyed:
		if(m_Text.isEmpty())
			return QObject::tr("Asteroid collision destroyed a titan. The rubble formed a ring around its world. "
					"Debris makes system navigation hazardous.");
		else
			return QObject::tr("Asteroid collision destroyed titan %1. The rubble formed a ring around its world. "
					"Debris makes system navigation hazardous.").arg(m_Text);

	case AsteroidDepartedDeepSpace:
		return QObject::tr("Asteroid orbited harmlessly out to deep space.");

	case KickedStowaway:
		return QObject::tr("Kicked stowaway %1.").arg(m_Text);

	case StandingDown:
		return QObject::tr("%1 is hitting the ship pretty hard. "
				"To avoid further losses, I ordered the crew to stand down for boarding.").arg(m_Text);

	case Payday:
		{
			QStringList amts = m_Text.split("|");
			if(amts.count() == 2)
			{
				// Back pay was paid by a city.
				float totalBackPaid = QVariant(amts[0]).toFloat();
				return QObject::tr("%L1%2 payroll received from bank at %3 for crew back pay.")
					.arg(totalBackPaid, 0,'f',0)
					.arg(g_Au.CurrencySymbol())
					.arg(amts[1]);
			}
			else if(amts.count() == 3)
			{
				// Payday on a spacecraft.
				float totalPay = QVariant(amts[0]).toFloat();
				float totalBackPaid = QVariant(amts[1]).toFloat();
				float totalBackPay = QVariant(amts[2]).toFloat();

				QString pay, backPay, backPaid;
				if(totalPay > 0)
					pay = QObject::tr("%L1%2 paid to crew. ").arg(totalPay, 0,'f',0).arg(g_Au.CurrencySymbol());
				if(totalBackPaid > 0)
					backPaid = QObject::tr("%L1%2 back pay settled. ").arg(totalBackPaid, 0,'f',0).arg(g_Au.CurrencySymbol());
				if(totalBackPay > 0)
					backPay = QObject::tr("%L1%2 back pay owed. ").arg(totalBackPay, 0,'f',0).arg(g_Au.CurrencySymbol());

				return pay + backPaid + backPay;
			}
			else
				return QObject::tr("Something bad happened on payday.");
		}
		break;

	case Position:
		{
			QStringList fields = m_Text.split("|");
			if(fields.count() == 6)
			{
				// Fields
				// 0 = type
				// 1 = city name
				// 2 = long lat
				// 3 = planet name
				// 4 = system name
				// 5 = sector name
				int type = QVariant(fields[0]).toInt();
				switch(type)
				{
				case 0:
				default:
					return QObject::tr("Lost in space. %1 %2. %4. %5.")
						.arg(type)
						.arg(fields.count())
						.arg(fields[4])
						.arg(fields[5]);

				case 1:
					if(fields[1].isEmpty())
						return QObject::tr("On the ground on %1 at %2. %4. %5.")
							.arg(fields[3])
							.arg(fields[2])
							.arg(fields[4])
							.arg(fields[5]);
					else
						return QObject::tr("On the ground in %3 on %1 at %2. %4. %5.")
							.arg(fields[3])
							.arg(fields[2])
							.arg(fields[1])
							.arg(fields[4])
							.arg(fields[5]);

				case 2:
					return QObject::tr("Flying over %1. %4. %5.")
						.arg(fields[3])
						.arg(fields[4])
						.arg(fields[5]);

				case 3:
					return QObject::tr("Orbiting %1. %4. %5.")
						.arg(fields[3])
						.arg(fields[4])
						.arg(fields[5]);

				case 4:
					return QObject::tr("Maneuvering near %1. %4. %5.")
						.arg(fields[3])
						.arg(fields[4])
						.arg(fields[5]);

				case 5:
					return QObject::tr("Holding our position in solar system. %4. %5.")
						.arg(fields[4])
						.arg(fields[5]);

				case 6:
					return QObject::tr("Traveling through solar system. %4. %5.")
						.arg(fields[4])
						.arg(fields[5]);

				case 7:
					return QObject::tr("Holding our position in deep space. %4. %5.")
						.arg(fields[4])
						.arg(fields[5]);

				case 8:
					return QObject::tr("Traveling through deep space. %4. %5.")
						.arg(fields[4])
						.arg(fields[5]);
				}
			}
		}
		return m_Text;

	case BuildingDestroyed:
		{
			QStringList fields = m_Text.split("|");
			if(fields.count() == 2)
				return QObject::tr("%1 at %2 destroyed.").arg(fields[0]).arg(fields[1]);
			else if(fields.count() == 3)
				return QObject::tr("%3 destroyed the %1 at %2.").arg(fields[0]).arg(fields[1]).arg(fields[2]);
		}
		return m_Text;

	case FlagLowered:					// string is 'empire name' of attacker
		return QObject::tr("We are under attack! An enemy %1 unit lowered the flag.").arg(m_Text);

	case BuildingAttacked:				// string is 'atom of empire name' of attacker
		return QObject::tr("We are under attack! %1 damaged a building.").arg(m_Text);

	case CitizenAttacked:				// string is 'atom of empire name' of attacker
		return QObject::tr("We are under attack! %1 attacked a citizen.").arg(m_Text);

	case EnemySpacecraftSpotted:			// string is DaVessel::NameWithEmpire of the ship
		return QObject::tr("We are under attack! A citizen spotted enemy %1.").arg(m_Text);

	case EnemyTroopSpotted:				// string is 'empire name' of troop
		return QObject::tr("We are under attack! A citizen spotted an enemy %1 troop.").arg(m_Text);

	case EnemyVehicleSpotted:			// string is 'empire name vehicle name' of vehicle
		return QObject::tr("We are under attack! A citizen spotted an enemy %1.").arg(m_Text);

	case ShipmentInPlunderedSector:
		return QObject::tr("Incoming shipment of %1 was plundered. "
				"Pirates must be building a base in this sector.").arg(m_Text);
	case ShipmentInPlunderedSystem:
		return QObject::tr("Incoming shipment of %1 was plundered. "
				"A pirate base in this system is attacking every ship that flies.").arg(m_Text);
	case ShipmentOutPlunderedSector:
		return QObject::tr("Outgoing shipment of %1 was plundered. "
				"Pirates must be building a base in this sector.").arg(m_Text);
	case ShipmentOutPlunderedSystem:
		return QObject::tr("Outgoing shipment of %1 was plundered. "
				"A pirate base in this system is attacking every ship that flies.").arg(m_Text);
	}
}

quint64 AuEventLogEntry::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_AtomId);
	mem += StringMemoryUsage(m_Text);
	mem += sizeof(m_Time);
	mem += sizeof(m_Type);
	return mem;
}

bool AuEventLogEntry::IsAlert() const
{
	switch(m_Type)
	{
	case Invalid:
	case Anomaly:
	case AsteroidDepartedDeepSpace:
	case AsteroidDetected:
	case AsteroidObserved:
	case Beacon:
	case NeutralVehicle:
	case FriendVehicle:
	case EmpireVehicle:
	case CitizenDNAChange:
	case Payday:
	case Position:
	case TradeAgreement:
	case TradeProposal:
		// This type does not produce an alert icon.
		break;

	case NeutralComm:
	case FriendComm:
	case EmpireComm:
		break;

	case EnemyComm:
	case EnemyVehicle:
	case EnemyAircraft:
	case EnemyBase:
	case EnemyBoat:
	case EnemyCapital:
	case EnemyCity:
	case EnemySpacecraftHull:
	case EnemyStation:
	case EnemyStationS:
	case EnemySpaceship:
	case EnemySpaceshipS:
	case EnemyStarship:
	case EnemyStarshipS:
	case DamageReceived:
	case EnemyWeaponRangeIn:
	case EnemyWeaponRangeOut:
	case Disease:
	case Poison:
	case Infestation:
	case Medical:
	case KickedStowaway:
	case StandingDown:
	case BuildingDestroyed:
	case FlagLowered:					// string is 'empire name' of attacker
	case BuildingAttacked:				// string is 'atom of empire name' of attacker
	case CitizenAttacked:				// string is 'atom of empire name' of attacker
	case EnemySpacecraftSpotted:		// string is DaVessel::NameWithEmpire of the ship
	case EnemyTroopSpotted:				// string is 'empire name' of troop
	case EnemyVehicleSpotted:			// string is 'vehicle name of empire name' of troop
	case ShipmentInPlunderedSector:
	case ShipmentInPlunderedSystem:
	case ShipmentOutPlunderedSector:
	case ShipmentOutPlunderedSystem:
	case AsteroidObservedCollide:
	case AsteroidCollisionBlackHole:
	case AsteroidCollisionGasGiant:
	case AsteroidCollisionMoon:
	case AsteroidCollisionPlanet:
	case AsteroidCollisionPlanetoid:
	case AsteroidCollisionRingworld:
	case AsteroidCollisionSun:
	case AsteroidCollisionTitan:
	case AsteroidCollisionMoonDestroyed:
	case AsteroidCollisionPlanetDestroyed:
	case AsteroidCollisionPlanetoidDestroyed:
	case AsteroidCollisionTitanDestroyed:
		// This type produces an alert icon.
		return true;
	}
	return false;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuEventLog::AddEventTrim(const AuEventLogEntry& entry)
{
	if(entry.m_Type == AuEventLogEntry::Position)
	{
		// It's preferable for position entries to log the time of arrival, so we ignore nu position entries that duplicate the
		// current final position entry.
		// Search backward from the end of the log for the final position entry.
		for(iterator ei = end()-1; ei != begin()-1; ei--)
			if((*ei).m_Type == AuEventLogEntry::Position)
			{
				// Found the final position entry in the log.
				if((*ei).IsRepeat(entry))
					return;				// New position entry is not different. Keep the old one and ignore the nu one.

				// New entry is not a repeat. Stop looping so it can be appended to this log.
				break;
			}
	}
	else
	{
		// Search from the end of the log for a previous matching entry in the same area.
		// We determine same area by the position entries; they mark the arrival at each nu area.
		// Search from the end for a previous repeat of 'entry' before we get to the final position entry.
		for(iterator ei = end()-1; ei != begin()-1; ei--)
			if((*ei).m_Type == AuEventLogEntry::Position)
				break;					// Stop when we hit the final position entry.
			else if((*ei).IsRepeat(entry))
			{
				// Found a repeat of 'entry' before we found the final position entry.
				erase(ei);				// Remove the previous matching entry from the list.
				break;					// We can stop; this logic insures there won't be another match before the position entry.
			}
	}

	// Discard all log entries that are more than three days old.
	Clear(entry.m_Time.AddDays(-3));

	// Append the nu entry at the end of the log.
	AddEvent(entry);
}

void AuEventLog::AddLogTrim(const AuEventLog& log)
{
	for(const_iterator li = log.constBegin(); li != log.constEnd(); li++)
		AddEventTrim(*li);
}

void AuEventLog::operator<<(QDataStream& in)
{
	clear();
	quint16 count;
	in >> count;
	for(;count;count--)
	{
		AuEventLogEntry logEntry;
		logEntry << in;
		append(logEntry);
	}
}

void AuEventLog::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() <= 0xffff);
	out << (quint16)count();
	for(const_iterator ei = constBegin(); ei != constEnd(); ei++)
		(*ei) >> out;
}

QString AuEventLog::ReportHtml(const QString& title, const AuTime& asOf) const
{
	QString rpt;
	if(isEmpty())
		return rpt;

	if(asOf.IsNull() || ContainsNewerEvent(asOf))
	{
		// This event log has events nuer than the 'asOf' date/time.
		rpt += QString("<b>%1</b><p style=\"margin-left: 10px;\">").arg(title);
		rpt += QString("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(36, 44, 94);\">"
				"<tbody>");
		rpt += QString("<tr>"
				"<td><b>%1</b></td>"
				"<td><b>%2</b></td>"
				"</tr>")
			.arg(QObject::tr("Time"))
			.arg(QObject::tr("Note"));

		for(const_iterator ei = constBegin(); ei != constEnd(); ei++)
			if((*ei).m_Time > asOf)
				rpt += QString("<tr>"
						"<td><img height=\"21\" width=\"21\" src=\"qrc%1\"> %2</td>"
						"<td>%3</td>"
						"</tr>")
					.arg((*ei).TypeImageName())
					.arg(g_Au.EncodeUTCString((*ei).m_Time))
					.arg((*ei).Text());

		rpt += "</tbody></table><br>";
		rpt += "</p><br>";
	}
	return rpt;
}

QString AuEventLog::ReportHtmlToolTip(const QString& title) const
{
	QString rpt;
	if(!isEmpty())
	{
		rpt += QString("%1<p style=\"margin-left: 10px;\">").arg(title);
		rpt += QString("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\">"
				"<tbody>");
		rpt += QString("<tr>"
				"<td><b>%1</b></td>"
				"<td><b>%2</b></td>"
				"<td><b>%3</b></td>"
				"</tr>")
			.arg(QObject::tr("Event"))
			.arg(QObject::tr("Time"))
			.arg(QObject::tr("Note"));

		for(const_iterator ei = constBegin(); ei != constEnd(); ei++)
			rpt += QString("<tr>"
					//DHH Images don't seem to work on tool tips "<td><img style=\"width: 21px; height: 21px;\" src=\"qrc%1\">%2</td>"
					"<td>%1</td>"
					"<td>%2</td>"
					"<td>%3</td>"
					"</tr>")
				//DHH .arg((*ei).TypeImageName())
				.arg((*ei).TypeName())
				.arg(g_Au.EncodeUTCString((*ei).m_Time))
				.arg((*ei).Text());

		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}
	return rpt;
}

// Lists a bullet for each alert item, terminated by <br>.
QString AuEventLog::ReportAlertsHtml() const
{
	QString rpt;
	if(!isEmpty())
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
			if((*ei).IsAlert())
				rpt += QString("%1<br>").arg((*ei).TypeName());
	return rpt;
}

quint64 AuEventLog::MemoryUsage() const
{
	quint64 mem = sizeof(AuEventLog);
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		mem += (*ei).MemoryUsage();
	return mem;
}
