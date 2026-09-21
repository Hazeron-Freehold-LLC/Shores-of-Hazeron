//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausensorblip.cpp,v 1.82 2022/02/19 17:41:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausensorblip.h"
//
#include "auglobal.h"
#include "daasteroid.h"
#include "dastar.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include <QDebug>

AuSensorBlip::AuSensorBlip(const AuId& id, const AuPoint& sysLoc)
{
	m_AtomId = id;
	m_SysLoc = sysLoc;
}

AuSensorBlip::AuSensorBlip()
{
}

AuSensorBlip::~AuSensorBlip()
{
}

AuSensorBlip::AuSensorBlip(const AuSensorBlip& c)
{
	m_AtomId = c.m_AtomId;
	m_SysLoc = c.m_SysLoc;
}

// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
void AuSensorBlip::TxMin(QDataStream& out) const
{
	//qDebug() << "AuSensorBlip::TxMin" << AtomId();
	out << m_AtomId;
	out << (float)m_SysLoc.x;
	out << (float)m_SysLoc.y;
	out << (float)m_SysLoc.z;
}

// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
void AuSensorBlip::RxMin(QDataStream& in)
{
	in >> m_AtomId;
	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_SysLoc = AuPoint(x,y,z);
	//qDebug() << "AuSensorBlip::RxMin" << AtomId();
}

void AuSensorBlip::operator>>(QDataStream& out) const
{
	out << m_AtomId;
	out << (float)m_SysLoc.x;
	out << (float)m_SysLoc.y;
	out << (float)m_SysLoc.z;
}

void AuSensorBlip::operator<<(QDataStream& in)
{
	in >> m_AtomId;
	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_SysLoc = AuPoint(x,y,z);
}

//static
AuSensorBlip* AuSensorBlip::Create(TYPE type)
{
	// This switch statement insures that I get a compiler warning if I derive a nu sensor blip class and forget to add it here.
	switch(type)
	{
	case Aircraft:
		return new AuBlipAircraft;
	case Animal:
		return new AuBlipAnimal;
	case Anomaly:
		return new AuBlipAnomaly;
	case Asteroid:
		return new AuBlipAsteroid;
	case Planetoid:
		return new AuBlipPlanetoid;
	case Avatar:
		return new AuBlipAvatar;
	case BdBase:
		return new AuBlipBdBase;
	case BdBaseHQ:
		return new AuBlipBdBaseHQ;
	case BdCapitolCity:
		return new AuBlipBdCapitolCity;
	case BdCapitolEmpire:
		return new AuBlipBdCapitolEmpire;
	case BdCapitolGalaxy:
		return new AuBlipBdCapitolGalaxy;
	case BdCapitolSector:
		return new AuBlipBdCapitolSector;
	case BdCity:
		return new AuBlipBdCity;
	case Beacon:
		return new AuBlipBeacon;
	case BlackHole:
		return new AuBlipBlackHole;
	case Boat:
		return new AuBlipBoat;
	case Citizen:
		return new AuBlipCitizen;
	case Crew:
		return new AuBlipCrew;
	case GasGiant:
		return new AuBlipGasGiant;
	case GroundVehicle:
		return new AuBlipGroundVehicle;
	case Titan:
		return new AuBlipTitan;
	case Missile:
		return new AuBlipMissile;
	case Moon:
		return new AuBlipMoon;
	case NeutronStarCompanion:
		return new AuBlipNeutronStarCompanion;
	case NeutronStarPrimary:
		return new AuBlipNeutronStar;
	case Officer:
		return new AuBlipOfficer;
	case Passenger:
		return new AuBlipPassenger;
	case Planet:
		return new AuBlipPlanet;
	case Ring:
		return new AuBlipRing;
	case RingSection:
		return new AuBlipRingSection;
	case SpacecraftHiding:
		return new AuBlipSpacecraftHiding;
	case Station:
		return new AuBlipStation;
	case StationShielded:
		return new AuBlipStationShielded;
	case Spaceship:
		return new AuBlipSpaceship;
	case SpaceshipShielded:
		return new AuBlipSpaceshipShielded;
	case StarCompanion:
		return new AuBlipStarCompanion;
	case StarPrimary:
		return new AuBlipStar;
	case Starship:
		return new AuBlipStarship;
	case StarshipShielded:
		return new AuBlipStarshipShielded;
	case TransporterBd:
		return new AuBlipTransporterBd;
	case TransporterSpacecraft:
		return new AuBlipTransporterSpacecraft;
	case TransporterStation:
		return new AuBlipTransporterStation;
	case Troop:
		return new AuBlipTroop;
	case Invalid:
	case BlipTypes:
		break;
	}
	return 0;
}

// static
// bigNumber is in feet.
QString AuSensorBlip::FormatBigNumber(double bigNumber)
{
	QString snum;
	if(bigNumber < FeetPerOrbit())
		snum = QObject::tr("%L1m").arg(round(bigNumber / FeetPerMeter()));
	else if(bigNumber < FeetPerParsec())
		snum = QObject::tr("%L1au").arg(FROUND(bigNumber/FeetPerAu(), .001));
	else
		snum = QObject::tr("%L1pc").arg(FROUND(bigNumber/FeetPerParsec(), .001));
	return snum;
}

// Construct a single line description suitable for the city report, in a cell of the enemy contacts table.
QString AuSensorBlip::HtmlDesc(const DaAtom* system, bool includeSystemName) const
{
	QStringList infoLines = InfoLines(system);
	if(infoLines.isEmpty())
		return "?";

	QString text;
	if(includeSystemName)
		text = system->Name() + ", " + infoLines[0];
	else
		text = infoLines[0];
	for(QStringList::const_iterator ili = infoLines.begin()+1; ili != infoLines.end(); ili++)
		text += ", " + *ili;
	return text;
}

QString AuSensorBlip::HtmlTableRow(const DaAtom* system, const AuPoint& scannerLoc, int columns) const
{
	// Row begins with the blip image in the first column.
	QString tableRow = QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td>").arg(BlipImage());
	int columnsAdded = 1;										// First column has been used.

	// Add a column for each info line, up to the number of columns wanted minus 1.
	// That leaves room for the range column at the end.
	QStringList infoLines = InfoLines(system);
	for(QStringList::const_iterator ili = infoLines.begin(); ili != infoLines.end() && columnsAdded < columns-1; ili++)
	{
		tableRow += "<td>" + *ili + "</td>";
		columnsAdded++;
	}

	// Fill out the row if we didn't use all the columns minus 1, leaving room for the range column at the end.
	for(; columnsAdded < columns-1; columnsAdded++)
		tableRow += "<td></td>";

	// Add the range column last, if there is room.
	if(columnsAdded < columns)
	{
		tableRow += QString("<td>%1</td>").arg(FormatBigNumber(Range(scannerLoc)));
		columnsAdded++;
	}

	// Append closing row tag.
	tableRow += "</tr>";
	return tableRow;
}

bool AuSensorBlip::IsPirateEmpire() const
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return false;
	return DgEmpire::IsPirate(empireId);
}

void AuSensorBlip::DrawBlip(QPainter& p, const AuMatrix4x4& t, double /*feetPerPixel*/, const DaAtom* /*system*/) const
{
	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Most blips can simply use the BlipImage.
	QPixmap bm(BlipImage());
	p.drawPixmap(px.x()-bm.width()/2, px.y()-bm.height()/2, bm);
}

void AuSensorBlip::DrawBlipStem(QPainter& p, const AuMatrix4x4& t, double /*feetPerPixel*/, const DaAtom* /*system*/) const
{
	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));
	AuPoint fpx0 = t.Project(EnvLocation().x, EnvLocation().y, 0.0);
	QPoint px0(round(fpx0.x),round(fpx0.y));
	if(px == px0)
		return;
	// Draw a vertical line to the XY plane of the blip.
	if(EnvLocation().z > 0)
		p.setPen(QPen(QColor(0,0,192), 0.f));
	else
		p.setPen(QPen(QColor(0,0,96), 0.f));
	p.drawLine(px0.x(), px0.y(), px.x(), px.y());
}

quint64 AuSensorBlip::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_AtomId);
	mem += sizeof(m_SysLoc);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuSensorBlip& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSensorBlipList::AuSensorBlipList(const AuSensorBlipList& c) : list<AuSensorBlip*>()
{
	for(const_iterator bi = c.begin(); bi != c.end(); bi++)
		push_back((*bi)->Clone());
}

const AuSensorBlipList& AuSensorBlipList::operator=(const AuSensorBlipList& c)
{
	Clear();
	for(const_iterator bi = c.begin(); bi != c.end(); bi++)
		push_back((*bi)->Clone());
	return *this;
}

void AuSensorBlipList::TxMin(QDataStream& out) const
{
	//qDebug() << "AuSensorBlipList::TxMin" << size();
	Q_ASSERT(size() <= 0xffff);
	out << (quint16)size();
	for(const_iterator cpi = begin(); cpi != end(); cpi++)
	{
		out << (quint8)(*cpi)->Type();
		(*cpi)->TxMin(out);
	}
}

void AuSensorBlipList::TxMinRangeLimited(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	// Send all the range-limited blips.
	quint16 blipCount = 0;				// Count of range limited blips.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited())
			blipCount++;

	//qDebug() << "AuSensorBlipList::TxMinRangeLimited total count" << size() << "sending" << blipCount;
	out << blipCount;					// Start with the number of blips we will send.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited())
		{
			out << (quint8)(*sbi)->Type();
			(*sbi)->TxMin(out);
		}
}

void AuSensorBlipList::TxRangeLimited(QDataStream& out) const
{
	Q_ASSERT(size() <= 0xffff);
	// Send all the range-limited blips.
	quint16 blipCount = 0;				// Count of range limited blips.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited())
			blipCount++;

	//qDebug() << "AuSensorBlipList::TxRangeLimited total count" << size() << "sending" << blipCount;
	out << blipCount;					// Start with the number of blips we will send.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited())
		{
			out << (quint8)(*sbi)->Type();
			out << **sbi;
		}
}

void AuSensorBlipList::TxRangeLimited(QDataStream& out, const AuPoint& scannerLoc, double scannerRange) const
{
	Q_ASSERT(size() <= 0xffff);
	// Send all the range-limited blips.
	quint16 blipCount = 0;				// Count of range limited blips.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited() && (*sbi)->Range(scannerLoc) <= scannerRange)
			blipCount++;

	//qDebug() << "AuSensorBlipList::TxRangeLimited total count" << size() << "sending" << blipCount;
	out << blipCount;					// Start with the number of blips we will send.
	for(const_iterator sbi = begin(); sbi != end(); sbi++)
		if((*sbi)->IsRangeLimited() && (*sbi)->Range(scannerLoc) <= scannerRange)
		{
			out << (quint8)(*sbi)->Type();
			out << **sbi;
		}
}

void AuSensorBlipList::RxMin(QDataStream& in)
{
	Clear();

	quint16 blipcount;
	in >> blipcount;

	//qDebug() << "AuSensorBlipList::RxMin count" << blipcount;
	quint8 bliptype;
	while(blipcount--)
	{
		in >> bliptype;

		AuSensorBlip* blip = AuSensorBlip::Create((AuSensorBlip::TYPE)bliptype);
		if(!blip)
		{
			qWarning() << "AuSensorBlipList::RxMin ERROR! read bad object type" << bliptype << blipcount << size();
			break;
		}

		blip->RxMin(in);
		push_back(blip);
	}
	//qDebug() << "AuSensorBlipList::RxMin final count" << size();
}

void AuSensorBlipList::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator cpi = begin(); cpi != end(); cpi++)
	{
		out << (quint8)(*cpi)->Type();
		out << **cpi;
	}
}

void AuSensorBlipList::operator<<(QDataStream& in)
{
	Clear();

	quint16 blipcount;
	in >> blipcount;

	quint8 bliptype;
	while(blipcount--)
	{
		in >> bliptype;

		AuSensorBlip* blip = AuSensorBlip::Create((AuSensorBlip::TYPE)bliptype);
		if(!blip)
		{
			qWarning() << "AuSensorBlipList::operator<< ERROR! read bad object type" << bliptype;
			break;
		}

		in >> *blip;
		push_back(blip);
	}
}

QPoint AuSensorBlipList::DrawSensorConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
		const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip,
		bool bCities, bool bLife,
		bool bPlanets, bool bShips, bool bStations,
		bool bStars, bool bVehicles, const AuPoint& consoleLoc,
		bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
		const AuId& scannerId, const AuPoint& scannerLoc) const
{
	//qDebug() << "AuSensorBlipList::DrawSensorConsoleBlips";
	if(feetPerPixel <= 0.0)
		return QPoint(0,0);

	QPoint pxSel(0,0);
	// Draw stems first so blips overlay them.
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			if(!bCities)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if(!(*ib)->IsSensorConsoleBlip())
		{
			// This test allows missiles and asteroids to be shown always, without having a toggle to display them or not.
			continue;
		}

		(*ib)->DrawBlipStem(p, t, feetPerPixel, system);
	}
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			if(!bCities)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if(!(*ib)->IsSensorConsoleBlip())
		{
			// This test allows missiles and asteroids to be shown always, without having a toggle to display them or not.
			continue;
		}

		(*ib)->DrawBlip(p, t, feetPerPixel, system);

		if(iSelectedBlip && (*ib)->IsSame(iSelectedBlip))
		{
			AuPoint fpx = t.Project(selLoc);
			pxSel = QPoint(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(0,255,255), 0.f));
			
			p.drawLine(pxSel.x()-5,pxSel.y()-5, pxSel.x()-7,pxSel.y()-5);
			p.drawLine(pxSel.x()-7,pxSel.y()-5, pxSel.x()-7,pxSel.y()+5);
			p.drawLine(pxSel.x()-7,pxSel.y()+5, pxSel.x()-5,pxSel.y()+5);

			p.drawLine(pxSel.x()+5,pxSel.y()-5, pxSel.x()+7,pxSel.y()-5);
			p.drawLine(pxSel.x()+7,pxSel.y()-5, pxSel.x()+7,pxSel.y()+5);
			p.drawLine(pxSel.x()+7,pxSel.y()+5, pxSel.x()+5,pxSel.y()+5);

			// Draw brief text next to reticle.
			QString sName = iSelectedBlip->ScreenName();
			double range = selLoc.Distance(consoleLoc);	// Distance in feet.
			QString sdist = AuSensorBlip::FormatBigNumber(range);
			QString sText = QString("%1 %2").arg(sName).arg(sdist);
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			p.setPen(QPen(QColor(0,192,192), 0.f));
			QRect rct(pxSel.x()+9,pxSel.y()-5,10,10);
			p.drawText(rct, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, sText);
		}

		if(!iLockedBlip.IsNull() && (*ib)->AtomId() == iLockedBlip)
		{
			//qDebug() << "iLockedBlip" << iLockedBlip << (*ib)->AtomId() << (*ib)->EnvLocation();
			AuPoint fpx = t.Project(selLoc);
			QPoint px(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(255,0,0), 0.f));
			
			p.drawLine(px.x()-5,px.y(), px.x()-11,px.y());
			p.drawLine(px.x()+5,px.y(), px.x()+11,px.y());
			p.drawLine(px.x(),px.y()-5, px.x(),px.y()-11);
			p.drawLine(px.x(),px.y()+5, px.x(),px.y()+11);
		}
	}
	return pxSel;
}

// 'targetType' specifies the type of target for 'targetE' display
//   0 = no target energy display
//   1 = energy weapon units on target
//   2 = harvest units
//   3 = heavy arms ammo
//   4 = large missiles
//   5 = large rockets
//   6 = bombs
//   7 = tractor beam
void AuSensorBlipList::DrawFireControlConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
		const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip,
		bool bCities, bool bLife,
		bool bPlanets, bool bShips, bool bStations,
		bool bStars, bool bVehicles,
		int targetType, float targetEnergy, const AuPoint& consoleLoc,
		bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
		const AuId& scannerId, const AuPoint& scannerLoc) const
{
	if(feetPerPixel <= 0.0)
		return;

	// Draw stems first so blips overlap them.
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			if(!bCities)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if((*ib)->IsAsteroid())
		{
		}
		else
		{
			continue;
		}

		(*ib)->DrawBlipStem(p, t, feetPerPixel, system);
	}
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		bool isSpacecraft = false;
		bool isBd = false;
		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			isBd = true;
			if(!bCities)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
			isSpacecraft = true;
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
			isSpacecraft = true;
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if((*ib)->IsAsteroid())
		{
		}
		else
		{
			continue;
		}

		(*ib)->DrawBlip(p, t, feetPerPixel, system);

		if(iSelectedBlip && (*ib)->IsSame(iSelectedBlip))
		{
			AuPoint fpx = t.Project(selLoc);
			QPoint pxSel = QPoint(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(0,255,255), 0.f));
			
			p.drawLine(pxSel.x()-5,pxSel.y()-5, pxSel.x()-7,pxSel.y()-5);
			p.drawLine(pxSel.x()-7,pxSel.y()-5, pxSel.x()-7,pxSel.y()+5);
			p.drawLine(pxSel.x()-7,pxSel.y()+5, pxSel.x()-5,pxSel.y()+5);

			p.drawLine(pxSel.x()+5,pxSel.y()-5, pxSel.x()+7,pxSel.y()-5);
			p.drawLine(pxSel.x()+7,pxSel.y()-5, pxSel.x()+7,pxSel.y()+5);
			p.drawLine(pxSel.x()+7,pxSel.y()+5, pxSel.x()+5,pxSel.y()+5);

			// Draw brief text next to reticle.
			QString sName = iSelectedBlip->ScreenName();
			double range = selLoc.Distance(consoleLoc);	// Distance in feet.
			QString sdist = AuSensorBlip::FormatBigNumber(range);
			QString sText = QString("%1 %2").arg(sName).arg(sdist);
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			p.setPen(QPen(QColor(0,192,192), 0.f));
			QRect rct(pxSel.x()+9,pxSel.y()-5,10,10);
			p.drawText(rct, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, sText);

			switch(targetType)
			{
			case 1:
				{
					// Energy on target.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 beams at target\n%3").arg(targetEnergy).arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 beams at target\n%2").arg(targetEnergy).arg(infoLines[3]);
					}
					else
						se = QObject::tr("%1 beams at target").arg(targetEnergy);

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 2:
				{
					QString se = QObject::tr("Harvest %1u").arg(targetEnergy);
					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 3:
				{
					// Heavy arms ammo on target.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 rounds at target\n%3").arg(targetEnergy).arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 rounds at target\n%3").arg(targetEnergy).arg(infoLines[3]);
					}
					else
						se = QObject::tr("%1 rounds at target").arg(targetEnergy);

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 4:
				{
					// Large missiles on target.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 missiles at target\n%3").arg(targetEnergy).arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 missiles at target\n%3").arg(targetEnergy).arg(infoLines[3]);
					}
					else
						se = QObject::tr("%1 missiles at target").arg(targetEnergy);

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 5:
				{
					// Large rockets on target.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 rockets at target\n%3").arg(targetEnergy).arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 rockets at target\n%3").arg(targetEnergy).arg(infoLines[3]);
					}
					else
						se = QObject::tr("%1 rockets at target").arg(targetEnergy);

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 6:
				{
					// Bombs on target.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 bombs on target\n%3").arg(targetEnergy).arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						se = QObject::tr("%1 bombs on target\n%3").arg(targetEnergy).arg(infoLines[3]);
					}
					else
						se = QObject::tr("%1 bombs on target").arg(targetEnergy);

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;

			case 7:
				{
					// Tractor beam.
					QString se;
					if(isSpacecraft)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						if(targetEnergy)
							se = QObject::tr("In Range\n%1").arg(infoLines[4]);
						else
							se = QObject::tr("Too Far\n%1").arg(infoLines[4]);
					}
					else if(isBd)
					{
						QStringList infoLines = iSelectedBlip->InfoLines(system);
						if(targetEnergy)
							se = QObject::tr("In Range\n%1").arg(infoLines[3]);
						else
							se = QObject::tr("Too Far\n%1").arg(infoLines[3]);
					}
					else
					{
						if(targetEnergy)
							se = QObject::tr("In Range");
						else
							se = QObject::tr("Too Far");
					}

					QRect rc(pxSel.x()+9,pxSel.y()+5,10,10);
					p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, se);
				}
				break;
			}
		}

		if(!iLockedBlip.IsNull() && (*ib)->AtomId() == iLockedBlip)
		{
			AuPoint fpx = t.Project(selLoc);
			QPoint px(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(255,0,0), 0.f));
			
			p.drawLine(px.x()-5,px.y(), px.x()-11,px.y());
			p.drawLine(px.x()+5,px.y(), px.x()+11,px.y());
			p.drawLine(px.x(),px.y()-5, px.x(),px.y()-11);
			p.drawLine(px.x(),px.y()+5, px.x(),px.y()+11);
		}
	}
}

// 'transporterMode' is true when we only want ships that have transporters.
QPoint AuSensorBlipList::DrawTransporterConsoleBlips(QPainter& p, const AuMatrix4x4 &t, float feetPerPixel,
		const AuSensorBlip* iSelectedBlip, const AuId& iLockedBlip, bool transporterMode,
		bool bLife, bool bCities, bool bPlanets, bool bShips, bool bStations, bool bStars, bool bVehicles,
		const AuPoint& consoleLoc, bool showEmpire, bool showFriend, bool showNeutral, bool showEnemy, const DaAtom* system,
		const AuId& scannerId, const AuPoint& scannerLoc) const
{
	if(feetPerPixel <= 0.0)
		return QPoint(0,0);

	QPoint pxSel(0,0);
	// Draw stems first so blips overlap them.
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			if(transporterMode)
				continue;
			if(!bCities)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if(transporterMode)
				continue;	// Looking for transporter rooms that are on and inactive.
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if(transporterMode)
				continue;	// Looking for transporter rooms that are on and inactive.
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
		}
		else if((*ib)->IsTransporterRoomCity())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bCities)
				continue;
			if(!transporterMode)
				continue;
		}
		else if((*ib)->IsTransporterRoomStation())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bStations)
				continue;
			if(!transporterMode)
				continue;
		}
		else if((*ib)->IsTransporterRoomSpacecraft())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bShips)
				continue;
			if(!transporterMode)
				continue;
		}

		(*ib)->DrawBlipStem(p, t, feetPerPixel, system);
	}
	for(const_iterator ib = begin(); ib != end(); ib++)
	{
		// Filter based on political stance.
		if(!(*ib)->TestStance(showEmpire, showFriend, showNeutral, showEnemy, true))
			continue;

		AuPoint selLoc = (*ib)->EnvLocation();
		if((*ib)->IsVehicle())
		{
			if(!bVehicles)
				continue;
		}
		else if((*ib)->IsStar())
		{
			if(!bStars)
				continue;
		}
		else if((*ib)->IsCreature())
		{
			if(!bLife)
				continue;
		}
		else if((*ib)->IsCity() || (*ib)->IsBuilding())
		{
			if(transporterMode)
				continue;
			if(!bCities)
				continue;
		}
		else if((*ib)->IsWorld())
		{
			if(!bPlanets)
				continue;
			selLoc = (*ib)->WeaponLockLocation(system, scannerLoc, CID_Invalid);
		}
		else if((*ib)->IsSpaceStation())
		{
			if(!bStations)
				continue;
			if(transporterMode)
				continue;	// Looking for transporter rooms that are on and inactive.
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the Station that contains the panel.
		}
		else if((*ib)->IsSpacecraftHiding())
		{
			continue;
		}
		else if((*ib)->IsSpacecraft())
		{
			if(!bShips)
				continue;
			if(transporterMode)
				continue;	// Looking for transporter rooms that are on and inactive.
			if((*ib)->AtomId() == scannerId)
				continue;	// Don't show the ship that contains the panel.
		}
		else if((*ib)->IsTransporterRoomCity())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bCities)
				continue;
			if(!transporterMode)
				continue;
		}
		else if((*ib)->IsTransporterRoomStation())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bStations)
				continue;
			if(!transporterMode)
				continue;
		}
		else if((*ib)->IsTransporterRoomSpacecraft())
		{
			// Looking for transporter rooms that are on and inactive.
			if(!bShips)
				continue;
			if(!transporterMode)
				continue;
		}

		(*ib)->DrawBlip(p, t, feetPerPixel, system);

		if(iSelectedBlip && (*ib)->IsSame(iSelectedBlip))
		{
			AuPoint fpx = t.Project(selLoc);
			pxSel = QPoint(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(0,255,255), 0.f));
			
			p.drawLine(pxSel.x()-5,pxSel.y()-5, pxSel.x()-7,pxSel.y()-5);
			p.drawLine(pxSel.x()-7,pxSel.y()-5, pxSel.x()-7,pxSel.y()+5);
			p.drawLine(pxSel.x()-7,pxSel.y()+5, pxSel.x()-5,pxSel.y()+5);

			p.drawLine(pxSel.x()+5,pxSel.y()-5, pxSel.x()+7,pxSel.y()-5);
			p.drawLine(pxSel.x()+7,pxSel.y()-5, pxSel.x()+7,pxSel.y()+5);
			p.drawLine(pxSel.x()+7,pxSel.y()+5, pxSel.x()+5,pxSel.y()+5);

			// Draw brief text next to reticle.
			QString sName = iSelectedBlip->ScreenName();
			double range = selLoc.Distance(consoleLoc);	// Distance in feet.
			QString sdist = AuSensorBlip::FormatBigNumber(range);
			QString sText = QString("%1 %2").arg(sName).arg(sdist);
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			p.setPen(QPen(QColor(0,192,192), 0.f));
			QRect rct(1,1,10,10);
			p.drawText(rct, Qt::AlignLeft|Qt::AlignTop|Qt::TextDontClip, sText);
		}

		if(!iLockedBlip.IsNull() && (*ib)->AtomId() == iLockedBlip)
		{
			AuPoint fpx = t.Project(selLoc);
			QPoint px(round(fpx.x), round(fpx.y));
			p.setPen(QPen(QColor(255,0,0), 0.f));
			
			p.drawLine(px.x()-5,px.y(), px.x()-11,px.y());
			p.drawLine(px.x()+5,px.y(), px.x()+11,px.y());
			p.drawLine(px.x(),px.y()-5, px.x(),px.y()-11);
			p.drawLine(px.x(),px.y()+5, px.x(),px.y()+11);
		}
	}
	return pxSel;
}

quint64 AuSensorBlipList::MemoryUsage() const
{
	quint64 mem = sizeof(AuSensorBlipList);
	for(const_iterator ib = cbegin(); ib != cend(); ib++)
		mem += (*ib)->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipEmpire::AuBlipEmpire(quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuSensorBlip(id, sysLoc)
{
	m_Empire = empire;
	m_Stance = stance;
}

AuBlipEmpire::AuBlipEmpire()
{
	m_Empire = 0;
	m_Stance = POL_Neutral;
}

AuBlipEmpire::AuBlipEmpire(const AuBlipEmpire& c) : AuSensorBlip(c)
{
	m_Empire = c.m_Empire;
	m_Stance = c.m_Stance;
}

// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
void AuBlipEmpire::TxMin(QDataStream& out) const
{
	//qDebug() << "AuBlipEmpire::TxMin" << AtomId();
	AuSensorBlip::TxMin(out);
	out << m_Stance;
}

// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
void AuBlipEmpire::RxMin(QDataStream& in)
{
	AuSensorBlip::RxMin(in);
	in >> m_Stance;
	//qDebug() << "AuBlipEmpire::RxMin" << AtomId();
}

void AuBlipEmpire::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);
	out << m_Empire;
	out << m_Stance;
}

void AuBlipEmpire::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);
	in >> m_Empire;
	in >> m_Stance;
}

QString AuBlipEmpire::HtmlTableRow(const DaAtom* system, const AuPoint& scannerLoc, int columns) const
{
	if(!EmpireId())
		return AuSensorBlip::HtmlTableRow(system, scannerLoc, columns);

	// Row begins with the blip image in the first column.
	QString tableRow = QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td>").arg(BlipImage());
	int columnsAdded = 1;										// First column has been used.

	// Add a column for each info line except the last one, up to the number of columns wanted minus 2.
	// That leaves room for the empire name and range columns at the end.
	QStringList infoLines = InfoLines(system);
	for(QStringList::const_iterator ili = infoLines.begin(); ili != infoLines.end()-1 && columnsAdded < columns-2;
			ili++)
	{
		tableRow += "<td>" + *ili + "</td>";
		columnsAdded++;
	}

	// Fill out the row if we didn't use all the columns minus 2, leaving room for the empire name and range columns at the end.
	for(; columnsAdded < columns-2; columnsAdded++)
		tableRow += "<td></td>";

	// Add the empire name column if there is room.
	if(columnsAdded < columns-1)
	{
		tableRow += "<td>" + infoLines.last() + "</td>";
		columnsAdded++;
	}

	// Add the range column last, if there is room.
	if(columnsAdded < columns)
	{
		tableRow += QString("<td>%1</td>").arg(FormatBigNumber(Range(scannerLoc)));
		columnsAdded++;
	}

	// Append closing row tag.
	tableRow += "</tr>";
	return tableRow;
}

QStringList AuBlipEmpire::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	if(m_Empire)
	{
		QString empireName;
		bool requestEmpire = false;
		const DgData& gd = g_Au.LockDataForRead();

		// Scene client may need to request empire data about this blip's empire.
		if(g_Au.IsSceneClient() && !gd.HasEmpire(m_Empire))
			requestEmpire = true;

		// EmpireName returns 'Forgotten Empire NNN' for empires that don't exist.
		gd.EmpireName(m_Empire, empireName);
		g_Au.UnlockData();

		infoLines << empireName;

		if(requestEmpire)
		{
			// Must cast away the constness of system to emit a signal. :P
			((DaAtom*)system)->EmpireRequested(m_Empire);
		}
	}
	return infoLines;
}

quint64 AuBlipEmpire::MemoryUsage() const
{
	quint64 mem = AuSensorBlip::MemoryUsage();
	mem += sizeof(m_Empire);
	mem += sizeof(m_Stance);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipVehicle::AuBlipVehicle(COMMODITYID cid, quint8 quality, bool isNew,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	Q_ASSERT(quality);
	m_CID = cid;
	m_IsNew = isNew;
	m_Quality = quality;
}

AuBlipVehicle::AuBlipVehicle()
{
	m_CID = CID_Invalid;
	m_IsNew = false;
	m_Quality = 1;
}

AuBlipVehicle::AuBlipVehicle(const AuBlipVehicle& c) : AuBlipEmpire(c)
{
	m_CID = c.m_CID;
	m_IsNew = c.m_IsNew;
	m_Quality = c.m_Quality;
}

void AuBlipVehicle::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);

	quint16 cidNu = m_CID;
	if(m_IsNew)
		cidNu |= 0x8000;

	out << cidNu;
	out << m_Quality;
}

void AuBlipVehicle::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);

	quint16 cidNu;
	in >> cidNu;

	m_CID = cidNu & 0x7fff;
	m_IsNew = cidNu & 0x80000;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

// Name used next to selection reticle.
QString AuBlipVehicle::ScreenName() const
{
	return QObject::tr("%2 Q%1").arg(Quality()).arg(g_Au.CommodityName(CommodityId()));
}

QStringList AuBlipVehicle::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << ScreenName();
	infoLines << TypeName();

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

float AuBlipVehicle::ObstacleRadius() const
{
	return 15;
}

float AuBlipVehicle::PilotAvoidRadius() const
{
	return 25.f * (float)FeetPerMeter();	
}

quint64 AuBlipVehicle::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += sizeof(m_CID);
	mem += sizeof(m_IsNew);
	mem += sizeof(m_Quality);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipAircraft::AuBlipAircraft(COMMODITYID cid, quint8 quality, bool isNew,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipVehicle(cid, quality, isNew, empire, stance, id, sysLoc)
{
}

AuBlipAircraft::AuBlipAircraft()
{
}

AuBlipAircraft::AuBlipAircraft(const AuBlipAircraft& c) : AuBlipVehicle(c)
{
}

AuSensorBlip* AuBlipAircraft::Clone() const
{
	return new AuBlipAircraft(*this);
}

void AuBlipAircraft::operator>>(QDataStream& out) const
{
	AuBlipVehicle::operator>>(out);
}

void AuBlipAircraft::operator<<(QDataStream& in)
{
	AuBlipVehicle::operator<<(in);
}

const char* AuBlipAircraft::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipAircraftFriend.png",
		":data/BlipAircraftNeutral.png",
		":data/BlipAircraftEnemy.png",
		":data/BlipAircraftEmpire.png",
		":data/BlipAircraftEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipAircraft& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.CommodityId() << blip.Quality() << blip.IsNewVehicle()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBoat::AuBlipBoat(COMMODITYID cid, quint8 quality, bool isNew,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipVehicle(cid, quality, isNew, empire, stance, id, sysLoc)
{
}

AuBlipBoat::AuBlipBoat()
{
}

AuBlipBoat::AuBlipBoat(const AuBlipBoat& c) : AuBlipVehicle(c)
{
}

AuSensorBlip* AuBlipBoat::Clone() const
{
	return new AuBlipBoat(*this);
}

void AuBlipBoat::operator>>(QDataStream& out) const
{
	AuBlipVehicle::operator>>(out);
}

void AuBlipBoat::operator<<(QDataStream& in)
{
	AuBlipVehicle::operator<<(in);
}

const char* AuBlipBoat::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipBoatFriend.png",
		":data/BlipBoatNeutral.png",
		":data/BlipBoatEnemy.png",
		":data/BlipBoatEmpire.png",
		":data/BlipBoatEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipBoat& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.CommodityId() << blip.Quality() << blip.IsNewVehicle()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipGroundVehicle::AuBlipGroundVehicle(COMMODITYID cid, quint8 quality, bool isNew,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipVehicle(cid, quality, isNew, empire, stance, id, sysLoc)
{
}

AuBlipGroundVehicle::AuBlipGroundVehicle()
{
}

AuBlipGroundVehicle::AuBlipGroundVehicle(const AuBlipGroundVehicle& c) : AuBlipVehicle(c)
{
}

AuSensorBlip* AuBlipGroundVehicle::Clone() const
{
	return new AuBlipGroundVehicle(*this);
}

void AuBlipGroundVehicle::operator>>(QDataStream& out) const
{
	AuBlipVehicle::operator>>(out);
}

void AuBlipGroundVehicle::operator<<(QDataStream& in)
{
	AuBlipVehicle::operator<<(in);
}

const char* AuBlipGroundVehicle::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipVehicleFriend.png",
		":data/BlipVehicleNeutral.png",
		":data/BlipVehicleEnemy.png",
		":data/BlipVehicleEmpire.png",
		":data/BlipVehicleEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipGroundVehicle& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.CommodityId() << blip.Quality() << blip.IsNewVehicle()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipCreature::AuBlipCreature(const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc), m_Species(species)
{
}

AuBlipCreature::AuBlipCreature()
{
}

AuBlipCreature::AuBlipCreature(const AuBlipCreature& c) : AuBlipEmpire(c), m_Species(c.m_Species)
{
}

void AuBlipCreature::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);
	out << m_Species;
}

void AuBlipCreature::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);
	in >> m_Species;
}

QString AuBlipCreature::SpeciesName() const
{
	return m_Species.NameOrDesc();
}

QStringList AuBlipCreature::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << SpeciesName();

	// Get empire name from base class.
	if(EmpireId())
		infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

quint64 AuBlipCreature::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += m_Species.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipAnimal::AuBlipAnimal(bool isMounted,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCreature(species, empire, stance, id, sysLoc)
{
	m_IsMounted = isMounted;
}

AuBlipAnimal::AuBlipAnimal()
{
	m_IsMounted = false;
}

AuBlipAnimal::AuBlipAnimal(const AuBlipAnimal& c) : AuBlipCreature(c)
{
	m_IsMounted = c.m_IsMounted;
}

AuSensorBlip* AuBlipAnimal::Clone() const
{
	return new AuBlipAnimal(*this);
}

// Transmits the barest minimum of data to the client. Most blips send only their atom id, location and political stance.
void AuBlipAnimal::TxMin(QDataStream& out) const
{
	//qDebug() << "AuBlipAnimal::TxMin" << AtomId();
	AuBlipCreature::TxMin(out);

	quint8 wm = 0;
	if(IsWild())
		wm |= 0x01;
	if(IsMounted())
		wm |= 0x02;
	out << wm;
}

// Receives the barest minimum of data from the server. Most blips send only their atom id, location and political stance.
void AuBlipAnimal::RxMin(QDataStream& in)
{
	AuBlipCreature::RxMin(in);

	quint8 wm;
	in >> wm;
	m_Empire = (wm & 0x01) ? 0 : 1;
	m_IsMounted = (wm & 0x02) ? true : false;
	//qDebug() << "AuBlipAnimal::RxMin" << AtomId();
}

void AuBlipAnimal::operator>>(QDataStream& out) const
{
	AuBlipCreature::operator>>(out);

	out << m_IsMounted;
}

void AuBlipAnimal::operator<<(QDataStream& in)
{
	AuBlipCreature::operator<<(in);

	in >> m_IsMounted;
}

// Name used next to selection reticle.
QString AuBlipAnimal::ScreenName() const
{
	if(IsWild())
	{
		if(m_IsMounted)
			return QObject::tr("Mounted Wild %1").arg(SpeciesName());
		else
			return QObject::tr("Wild %1").arg(SpeciesName());
	}
	else
	{
		if(m_IsMounted)
			return QObject::tr("Mounted %1").arg(SpeciesName());
		else
			return SpeciesName();
	}
}

QStringList AuBlipAnimal::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	if(IsWild())
	{
		if(m_IsMounted)
			infoLines << QObject::tr("Mounted Wild Animal");
		else
			infoLines << QObject::tr("Wild Animal");
	}
	else
	{
		if(m_IsMounted)
			infoLines << QObject::tr("Mounted Animal");
		else
			infoLines << QObject::tr("Animal");
	}

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipAnimal::BlipImage() const
{
	if(!IsWild())
	{
		if(IsMounted())
		{
			const char* blipImages[] =
			{
				":data/BlipAnimalMFriend.png",
				":data/BlipAnimalMNeutral.png",
				":data/BlipAnimalMEnemy.png",
				":data/BlipAnimalMEmpire.png",
				":data/BlipAnimalMEmpire.png",
			};
			int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
			if(Stance() < 0 || Stance() >= blipImagesCount)
				return "";
			return blipImages[Stance()];
		}
		else
		{
			const char* blipImages[] =
			{
				":data/BlipAnimalFriend.png",
				":data/BlipAnimalNeutral.png",
				":data/BlipAnimalEnemy.png",
				":data/BlipAnimalEmpire.png",
				":data/BlipAnimalEmpire.png",
			};
			int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
			if(Stance() < 0 || Stance() >= blipImagesCount)
				return "";
			return blipImages[Stance()];
		}
	}
	else
	{
		if(IsMounted())
			return ":data/BlipAnimalMWild.png";
		else
			return ":data/BlipAnimalWild.png";
	}
}

quint64 AuBlipAnimal::MemoryUsage() const
{
	quint64 mem = AuBlipCreature::MemoryUsage();
	mem += sizeof(m_IsMounted);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipAnimal& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.IsMounted() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipAvatar::AuBlipAvatar(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCreature(species, empire, stance, id, sysLoc)
{
	m_Name = name;
}

AuBlipAvatar::AuBlipAvatar()
{
}

AuBlipAvatar::AuBlipAvatar(const AuBlipAvatar& c) : AuBlipCreature(c)
{
	m_Name = c.m_Name;
}

AuSensorBlip* AuBlipAvatar::Clone() const
{
	return new AuBlipAvatar(*this);
}

void AuBlipAvatar::operator>>(QDataStream& out) const
{
	AuBlipCreature::operator>>(out);

	out << m_Name;
}

void AuBlipAvatar::operator<<(QDataStream& in)
{
	AuBlipCreature::operator<<(in);

	in >> m_Name;
}

// Name used next to selection reticle.
QString AuBlipAvatar::ScreenName() const
{
	return Name();
}

QStringList AuBlipAvatar::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipAvatar::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipPersonFriend.png",
		":data/BlipPersonNeutral.png",
		":data/BlipPersonEnemy.png",
		":data/BlipPersonEmpire.png",
		":data/BlipPersonEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

quint64 AuBlipAvatar::MemoryUsage() const
{
	quint64 mem = AuBlipCreature::MemoryUsage();
	mem += StringMemoryUsage(m_Name);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipAvatar& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipCitizen::AuBlipCitizen(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCreature(species, empire, stance, id, sysLoc)
{
	m_Name = name;
}

AuBlipCitizen::AuBlipCitizen()
{
}

AuBlipCitizen::AuBlipCitizen(const AuBlipCitizen& c) : AuBlipCreature(c)
{
	m_Name = c.m_Name;
}

AuSensorBlip* AuBlipCitizen::Clone() const
{
	return new AuBlipCitizen(*this);
}

void AuBlipCitizen::operator>>(QDataStream& out) const
{
	AuBlipCreature::operator>>(out);

	out << m_Name;
}

void AuBlipCitizen::operator<<(QDataStream& in)
{
	AuBlipCreature::operator<<(in);

	in >> m_Name;
}

// Name used next to selection reticle.
QString AuBlipCitizen::ScreenName() const
{
	return Name();
}

QStringList AuBlipCitizen::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipCitizen::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCitizenFriend.png",
		":data/BlipCitizenNeutral.png",
		":data/BlipCitizenEnemy.png",
		":data/BlipCitizenEmpire.png",
		":data/BlipCitizenEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

quint64 AuBlipCitizen::MemoryUsage() const
{
	quint64 mem = AuBlipCreature::MemoryUsage();
	mem += StringMemoryUsage(m_Name);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipCitizen& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipCrew::AuBlipCrew(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCitizen(name, species, empire, stance, id, sysLoc)
{
}

AuBlipCrew::AuBlipCrew()
{
}

AuBlipCrew::AuBlipCrew(const AuBlipCrew& c) : AuBlipCitizen(c)
{
}

AuSensorBlip* AuBlipCrew::Clone() const
{
	return new AuBlipCrew(*this);
}

void AuBlipCrew::operator>>(QDataStream& out) const
{
	AuBlipCitizen::operator>>(out);
}

void AuBlipCrew::operator<<(QDataStream& in)
{
	AuBlipCitizen::operator<<(in);
}

QStringList AuBlipCrew::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipCrew::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCrewMateFriend.png",
		":data/BlipCrewMateNeutral.png",
		":data/BlipCrewMateEnemy.png",
		":data/BlipCrewMateEmpire.png",
		":data/BlipCrewMateEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipCrew& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipOfficer::AuBlipOfficer(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCitizen(name, species, empire, stance, id, sysLoc)
{
}

AuBlipOfficer::AuBlipOfficer()
{
}

AuBlipOfficer::AuBlipOfficer(const AuBlipOfficer& c) : AuBlipCitizen(c)
{
}

AuSensorBlip* AuBlipOfficer::Clone() const
{
	return new AuBlipOfficer(*this);
}

void AuBlipOfficer::operator>>(QDataStream& out) const
{
	AuBlipCitizen::operator>>(out);
}

void AuBlipOfficer::operator<<(QDataStream& in)
{
	AuBlipCitizen::operator<<(in);
}

QStringList AuBlipOfficer::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipOfficer::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCrewMateFriend.png",
		":data/BlipCrewMateNeutral.png",
		":data/BlipCrewMateEnemy.png",
		":data/BlipCrewMateEmpire.png",
		":data/BlipCrewMateEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipOfficer& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipPassenger::AuBlipPassenger(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCitizen(name, species, empire, stance, id, sysLoc)
{
}

AuBlipPassenger::AuBlipPassenger()
{
}

AuBlipPassenger::AuBlipPassenger(const AuBlipPassenger& c) : AuBlipCitizen(c)
{
}

AuSensorBlip* AuBlipPassenger::Clone() const
{
	return new AuBlipPassenger(*this);
}

void AuBlipPassenger::operator>>(QDataStream& out) const
{
	AuBlipCitizen::operator>>(out);
}

void AuBlipPassenger::operator<<(QDataStream& in)
{
	AuBlipCitizen::operator<<(in);
}

QStringList AuBlipPassenger::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipPassenger::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipPassengerFriend.png",
		":data/BlipPassengerNeutral.png",
		":data/BlipPassengerEnemy.png",
		":data/BlipPassengerEmpire.png",
		":data/BlipPassengerEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipPassenger& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipTroop::AuBlipTroop(const QString& name,
		const AuSpecies& species,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipCitizen(name, species, empire, stance, id, sysLoc)
{
}

AuBlipTroop::AuBlipTroop()
{
}

AuBlipTroop::AuBlipTroop(const AuBlipTroop& c) : AuBlipCitizen(c)
{
}

AuSensorBlip* AuBlipTroop::Clone() const
{
	return new AuBlipTroop(*this);
}

void AuBlipTroop::operator>>(QDataStream& out) const
{
	AuBlipCitizen::operator>>(out);
}

void AuBlipTroop::operator<<(QDataStream& in)
{
	AuBlipCitizen::operator<<(in);
}

QStringList AuBlipTroop::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << Name();

	// Get specie name and empire name from base class.
	infoLines << AuBlipCreature::InfoLines(system);
	return infoLines;
}

const char* AuBlipTroop::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipTroopFriend.png",
		":data/BlipTroopNeutral.png",
		":data/BlipTroopEnemy.png",
		":data/BlipTroopEmpire.png",
		":data/BlipTroopEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipTroop& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.SpeciesName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipTransporterSpacecraft::AuBlipTransporterSpacecraft(const QString& spacecraftName, quint16 roomNumber, quint8 pads,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	m_Pads = pads;
	m_RoomNumber = roomNumber;
	m_SpacecraftName = spacecraftName;
}

AuBlipTransporterSpacecraft::AuBlipTransporterSpacecraft()
{
	m_Pads = 0;
	m_RoomNumber = 0;
}

AuBlipTransporterSpacecraft::AuBlipTransporterSpacecraft(const AuBlipTransporterSpacecraft& c) : AuBlipEmpire(c)
{
	m_Pads = c.m_Pads;
	m_RoomNumber = c.m_RoomNumber;
	m_SpacecraftName = c.m_SpacecraftName;
}

AuSensorBlip* AuBlipTransporterSpacecraft::Clone() const
{
	return new AuBlipTransporterSpacecraft(*this);
}

void AuBlipTransporterSpacecraft::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);

	out << m_Pads;
	out << m_RoomNumber;
	out << m_SpacecraftName;
}

void AuBlipTransporterSpacecraft::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);

	in >> m_Pads;
	in >> m_RoomNumber;
	in >> m_SpacecraftName;
}

// Name used next to selection reticle.
QString AuBlipTransporterSpacecraft::ScreenName() const
{
	return QObject::tr("%1 TR %2").arg(SpacecraftName()).arg(RoomNumber());
}

QStringList AuBlipTransporterSpacecraft::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << SpacecraftName();
	infoLines << QObject::tr("Transporter Room %1").arg(RoomNumber());

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

const char* AuBlipTransporterSpacecraft::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipTransporterShipFriend.png",
		":data/BlipTransporterShipNeutral.png",
		":data/BlipTransporterShipEnemy.png",
		":data/BlipTransporterShipEmpire.png",
		":data/BlipTransporterShipEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

quint64 AuBlipTransporterSpacecraft::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += sizeof(m_Pads);
	mem += sizeof(m_RoomNumber);
	mem += StringMemoryUsage(m_SpacecraftName);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipTransporterSpacecraft& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.SpacecraftName() << blip.RoomNumber() << "pads" << blip.Pads()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipTransporterStation::AuBlipTransporterStation(const QString& spacecraftName, quint16 roomNumber, quint8 pads,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipTransporterSpacecraft(spacecraftName, roomNumber, pads, empire, stance, id, sysLoc)
{
}

AuBlipTransporterStation::AuBlipTransporterStation()
{
}

AuBlipTransporterStation::AuBlipTransporterStation(const AuBlipTransporterStation& c) : AuBlipTransporterSpacecraft(c)
{
}

AuSensorBlip* AuBlipTransporterStation::Clone() const
{
	return new AuBlipTransporterStation(*this);
}

void AuBlipTransporterStation::operator>>(QDataStream& out) const
{
	AuBlipTransporterSpacecraft::operator>>(out);
}

void AuBlipTransporterStation::operator<<(QDataStream& in)
{
	AuBlipTransporterSpacecraft::operator<<(in);
}

QStringList AuBlipTransporterStation::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << SpacecraftName();
	infoLines << QObject::tr("Transporter Room %3").arg(RoomNumber());

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

const char* AuBlipTransporterStation::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipTransporterStationFriend.png",
		":data/BlipTransporterStationNeutral.png",
		":data/BlipTransporterStationEnemy.png",
		":data/BlipTransporterStationEmpire.png",
		":data/BlipTransporterStationEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipTransporterStation& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.SpacecraftName() << blip.RoomNumber() << "pads" << blip.Pads()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipSpacecraft::AuBlipSpacecraft(const QString& name, quint8 quality, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
		float roughRadius, const AuPoint& vector, quint32 volume, quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	Q_ASSERT(quality);
	m_CurHits = curHits;
	m_DesignName = designName;
	m_IsLockedOn = isLockedOn;
	m_IsNew = isNew;
	m_IsPrivatelyOwned = isPrivatelyOwned;
	m_MaxHits = maxHits;
	m_Name = name;
	m_RoughRadius = roughRadius;
	m_Quality = quality;
	m_Vector = vector;
	m_Volume = volume;
}

AuBlipSpacecraft::AuBlipSpacecraft()
{
	m_CurHits = 0;
	m_IsLockedOn = false;
	m_IsNew = false;
	m_IsPrivatelyOwned = false;
	m_MaxHits = 0;
	m_RoughRadius = 0;
	m_Quality = 1;
	m_Volume = 0;
}

AuBlipSpacecraft::AuBlipSpacecraft(const AuBlipSpacecraft& c) : AuBlipEmpire(c)
{
	m_CurHits = c.m_CurHits;
	m_DesignName = c.m_DesignName;
	m_IsLockedOn = c.m_IsLockedOn;
	m_IsNew = c.m_IsNew;
	m_IsPrivatelyOwned = c.m_IsPrivatelyOwned;
	m_MaxHits = c.m_MaxHits;
	m_Name = c.m_Name;
	m_RoughRadius = c.m_RoughRadius;
	m_Quality = c.m_Quality;
	m_Vector = c.m_Vector;
	m_Volume = c.m_Volume;
}

void AuBlipSpacecraft::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);

	out << m_CurHits;
	out << m_DesignName;
	out << m_IsLockedOn;
	out << m_IsNew;
	out << m_IsPrivatelyOwned;
	out << m_MaxHits;
	out << m_Name;
	out << m_RoughRadius;
	out << m_Quality;
	out << m_Vector;
	out << m_Volume;
}

void AuBlipSpacecraft::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);

	in >> m_CurHits;
	in >> m_DesignName;
	in >> m_IsLockedOn;
	in >> m_IsNew;
	in >> m_IsPrivatelyOwned;
	in >> m_MaxHits;
	in >> m_Name;
	in >> m_RoughRadius;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_Vector;
	in >> m_Volume;
}

// Name used next to selection reticle.
QString AuBlipSpacecraft::ScreenName() const
{
	return QObject::tr("%1 Q%2").arg(Name()).arg(Quality());
}

QStringList AuBlipSpacecraft::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1 class").arg(DesignName());
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	if(IsNew())
		infoLines << "New";

	if(IsPrivatelyOwned())
		infoLines << "Private";

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

float AuBlipSpacecraft::ObstacleRadius() const
{
	return RoughRadius();
}

float AuBlipSpacecraft::PilotAvoidRadius() const
{
	return ObstacleRadius() + 50.f * FeetPerMeter();
}

quint64 AuBlipSpacecraft::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += sizeof(m_CurHits);
	mem += StringMemoryUsage(m_DesignName);
	mem += sizeof(m_IsLockedOn);
	mem += sizeof(m_IsNew);
	mem += sizeof(m_IsPrivatelyOwned);
	mem += sizeof(m_MaxHits);
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_RoughRadius);
	mem += sizeof(m_Quality);
	mem += sizeof(m_Vector);
	mem += sizeof(m_Volume);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipSpacecraftShielded::AuBlipSpacecraftShielded(const QString& name, quint8 quality, bool isLockedOn, bool isNew,
		bool isPrivatelyOwned, float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraft(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
	m_AvoidRadius = avoidRadius;
}

AuBlipSpacecraftShielded::AuBlipSpacecraftShielded()
{
	m_AvoidRadius = 0;
}

AuBlipSpacecraftShielded::AuBlipSpacecraftShielded(const AuBlipSpacecraftShielded& c) : AuBlipSpacecraft(c)
{
	m_AvoidRadius = c.m_AvoidRadius;
}

void AuBlipSpacecraftShielded::operator>>(QDataStream& out) const
{
	AuBlipSpacecraft::operator>>(out);
	out << m_AvoidRadius;
}

void AuBlipSpacecraftShielded::operator<<(QDataStream& in)
{
	AuBlipSpacecraft::operator<<(in);
	in >> m_AvoidRadius;
}

QStringList AuBlipSpacecraftShielded::InfoLines(const DaAtom* system) const
{
	// Get spacecraft info and empire name from base class.
	QStringList infoLines;
	infoLines << AuBlipSpacecraft::InfoLines(system);

	// Report shield up state.
	infoLines << "Shields Up";
	return infoLines;
}

float AuBlipSpacecraftShielded::PilotAvoidRadius() const
{
	return m_AvoidRadius;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStation::AuBlipStation(const QString& name, quint8 quality, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
		float roughRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraft(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipStation::AuBlipStation()
{
}

AuBlipStation::AuBlipStation(const AuBlipStation& c) : AuBlipSpacecraft(c)
{
}

AuSensorBlip* AuBlipStation::Clone() const
{
	return new AuBlipStation(*this);
}

void AuBlipStation::operator>>(QDataStream& out) const
{
	AuBlipSpacecraft::operator>>(out);
}

void AuBlipStation::operator<<(QDataStream& in)
{
	AuBlipSpacecraft::operator<<(in);
}

const char* AuBlipStation::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSpaceportFriend.png",
		":data/BlipSpaceportNeutral.png",
		":data/BlipSpaceportEnemy.png",
		":data/BlipSpaceportEmpire.png",
		":data/BlipSpaceportEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipStation& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStationShielded::AuBlipStationShielded(const QString& name, quint8 quality, bool isLockedOn, bool isNew,
		bool isPrivatelyOwned, float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraftShielded(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, avoidRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipStationShielded::AuBlipStationShielded()
{
}

AuBlipStationShielded::AuBlipStationShielded(const AuBlipStationShielded& c) : AuBlipSpacecraftShielded(c)
{
}

AuSensorBlip* AuBlipStationShielded::Clone() const
{
	return new AuBlipStationShielded(*this);
}

void AuBlipStationShielded::operator>>(QDataStream& out) const
{
	AuBlipSpacecraftShielded::operator>>(out);
}

void AuBlipStationShielded::operator<<(QDataStream& in)
{
	AuBlipSpacecraftShielded::operator<<(in);
}

const char* AuBlipStationShielded::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSpaceportFriendS.png",
		":data/BlipSpaceportNeutralS.png",
		":data/BlipSpaceportEnemyS.png",
		":data/BlipSpaceportEmpireS.png",
		":data/BlipSpaceportEmpireS.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipStationShielded& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipSpaceship::AuBlipSpaceship(const QString& name, quint8 quality, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
		float roughRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraft(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipSpaceship::AuBlipSpaceship()
{
}

AuBlipSpaceship::AuBlipSpaceship(const AuBlipSpaceship& c) : AuBlipSpacecraft(c)
{
}

AuSensorBlip* AuBlipSpaceship::Clone() const
{
	return new AuBlipSpaceship(*this);
}

void AuBlipSpaceship::operator>>(QDataStream& out) const
{
	AuBlipSpacecraft::operator>>(out);
}

void AuBlipSpaceship::operator<<(QDataStream& in)
{
	AuBlipSpacecraft::operator<<(in);
}

const char* AuBlipSpaceship::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSpaceshipFriend.png",
		":data/BlipSpaceshipNeutral.png",
		":data/BlipSpaceshipEnemy.png",
		":data/BlipSpaceshipEmpire.png",
		":data/BlipSpaceshipEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipSpaceship& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipSpaceshipShielded::AuBlipSpaceshipShielded(const QString& name, quint8 quality, bool isLockedOn, bool isNew,
		bool isPrivatelyOwned, float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraftShielded(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, avoidRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipSpaceshipShielded::AuBlipSpaceshipShielded()
{
}

AuBlipSpaceshipShielded::AuBlipSpaceshipShielded(const AuBlipSpaceshipShielded& c) : AuBlipSpacecraftShielded(c)
{
}

AuSensorBlip* AuBlipSpaceshipShielded::Clone() const
{
	return new AuBlipSpaceshipShielded(*this);
}

void AuBlipSpaceshipShielded::operator>>(QDataStream& out) const
{
	AuBlipSpacecraftShielded::operator>>(out);
}

void AuBlipSpaceshipShielded::operator<<(QDataStream& in)
{
	AuBlipSpacecraftShielded::operator<<(in);
}

const char* AuBlipSpaceshipShielded::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSpaceshipFriendS.png",
		":data/BlipSpaceshipNeutralS.png",
		":data/BlipSpaceshipEnemyS.png",
		":data/BlipSpaceshipEmpireS.png",
		":data/BlipSpaceshipEmpireS.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipSpaceshipShielded& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStarship::AuBlipStarship(const QString& name, quint8 quality, bool isLockedOn, bool isNew, bool isPrivatelyOwned,
		float roughRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraft(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipStarship::AuBlipStarship()
{
}

AuBlipStarship::AuBlipStarship(const AuBlipStarship& c) : AuBlipSpacecraft(c)
{
}

AuSensorBlip* AuBlipStarship::Clone() const
{
	return new AuBlipStarship(*this);
}

void AuBlipStarship::operator>>(QDataStream& out) const
{
	AuBlipSpacecraft::operator>>(out);
}

void AuBlipStarship::operator<<(QDataStream& in)
{
	AuBlipSpacecraft::operator<<(in);
}

const char* AuBlipStarship::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipStarshipFriend.png",
		":data/BlipStarshipNeutral.png",
		":data/BlipStarshipEnemy.png",
		":data/BlipStarshipEmpire.png",
		":data/BlipStarshipEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipStarship& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStarshipShielded::AuBlipStarshipShielded(const QString& name, quint8 quality, bool isLockedOn, bool isNew,
		bool isPrivatelyOwned, float roughRadius, float avoidRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraftShielded(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, avoidRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipStarshipShielded::AuBlipStarshipShielded()
{
}

AuBlipStarshipShielded::AuBlipStarshipShielded(const AuBlipStarshipShielded& c) : AuBlipSpacecraftShielded(c)
{
}

AuSensorBlip* AuBlipStarshipShielded::Clone() const
{
	return new AuBlipStarshipShielded(*this);
}

void AuBlipStarshipShielded::operator>>(QDataStream& out) const
{
	AuBlipSpacecraftShielded::operator>>(out);
}

void AuBlipStarshipShielded::operator<<(QDataStream& in)
{
	AuBlipSpacecraftShielded::operator<<(in);
}

const char* AuBlipStarshipShielded::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipStarshipFriendS.png",
		":data/BlipStarshipNeutralS.png",
		":data/BlipStarshipEnemyS.png",
		":data/BlipStarshipEmpireS.png",
		":data/BlipStarshipEmpireS.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipStarshipShielded& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipSpacecraftHiding::AuBlipSpacecraftHiding(const QString& name, quint8 quality, bool isLockedOn, bool isNew,
		bool isPrivatelyOwned, float roughRadius, const AuPoint& vector, quint32 volume,
		quint32 maxHits, quint32 curHits, const QString& designName,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipSpacecraft(name, quality, isLockedOn, isNew, isPrivatelyOwned, roughRadius, vector, volume,
			maxHits, curHits, designName, empire, stance, id, sysLoc)
{
}

AuBlipSpacecraftHiding::AuBlipSpacecraftHiding()
{
}

AuBlipSpacecraftHiding::AuBlipSpacecraftHiding(const AuBlipSpacecraftHiding& c) : AuBlipSpacecraft(c)
{
}

AuSensorBlip* AuBlipSpacecraftHiding::Clone() const
{
	return new AuBlipSpacecraftHiding(*this);
}

void AuBlipSpacecraftHiding::operator>>(QDataStream& out) const
{
	AuBlipSpacecraft::operator>>(out);
}

void AuBlipSpacecraftHiding::operator<<(QDataStream& in)
{
	AuBlipSpacecraft::operator<<(in);
}

const char* AuBlipSpacecraftHiding::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSpacecraftHullFriend.png",
		":data/BlipSpacecraftHullNeutral.png",
		":data/BlipSpacecraftHullEnemy.png",
		":data/BlipSpacecraftHullEmpire.png",
		":data/BlipSpacecraftHullEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

QDebug operator<<(QDebug dbg, const AuBlipSpacecraftHiding& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.Name() << blip.Quality() << blip.IsNew() << blip.IsPrivatelyOwned() << blip.RoughRadius()
		<< blip.Vector() << blip.Volume() << blip.CurHits() << blip.MaxHits() << blip.DesignName()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStar::AuBlipStar(DaStar::TYPE type, DaStar::SIZE size, quint8 spectralClass, float radius, bool inSensorRange, const AuId& id,
		const AuPoint& sysLoc) :
	AuSensorBlip(id, sysLoc)
{
	m_InSensorRange = inSensorRange;
	m_Radius = radius;
	m_SpectralClass = spectralClass;
	m_StarSize = size;
	m_StarType = type;
}

AuBlipStar::AuBlipStar()
{
	m_InSensorRange = false;
	m_Radius = 0;
	m_SpectralClass = 0;
	m_StarSize = DaStar::SIZE_Ia;
	m_StarType = DaStar::TYPE_O;
}

AuBlipStar::AuBlipStar(const AuBlipStar& c) : AuSensorBlip(c)
{
	m_InSensorRange = c.m_InSensorRange;
	m_Radius = c.m_Radius;
	m_SpectralClass = c.m_SpectralClass;
	m_StarSize = c.m_StarSize;
	m_StarType = c.m_StarType;
}

AuSensorBlip* AuBlipStar::Clone() const
{
	return new AuBlipStar(*this);
}

void AuBlipStar::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);

	quint8 rs = ((quint8)m_InSensorRange << 7) | m_SpectralClass;	// top bit is m_InSensorRange.
	quint8 ts = (m_StarType << 4) | m_StarSize;					// top nibble is star type; bottom nibble is star size
	out << rs;
	out << ts;
	out << m_Radius;
}

void AuBlipStar::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);

	quint8 rs, ts;
	in >> rs;
	in >> ts;
	in >> m_Radius;

	m_InSensorRange = rs >> 7;									// top bit is m_InSensorRange.
	m_SpectralClass = rs & 0x0f;

	m_StarType = ts >> 4;										// top nibble is star type
	m_StarSize = ts & 0x0f;										// bottom nibble is star size
}

AuPoint AuBlipStar::WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& weaponLoc, COMMODITYID /*bayModule*/) const
{
	AuPoint vecToWeapon = weaponLoc;							// Vec to weapon from primary star, which is always at the center.
	vecToWeapon.Unit(Radius() - 10000);							// Vec scaled to 10,000' inside star photosphere.
	return vecToWeapon;											// Return system relative lock location.
}

// Name used next to selection reticle.
QString AuBlipStar::ScreenName() const
{
	return QObject::tr("%1 Primary").arg(DaStar::ScreenName(StarType(), StarSize(), SpectralClass()));
}

QStringList AuBlipStar::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	const DaStar* star = (const DaStar*)system->FindIdRecursive(AtomId());	// system: primary star, companion star
	if(!star)
		return infoLines;

	QString sName = star->Name();

	QString sOrbiting;
	if(!star->IsPrimary())
	{
		// Orbital position.
		double orbitAngle = PI/2 - star->Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(star->Parent()->Name())
			.arg(FormatBigNumber(star->Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	const char *ptype;
	const char *pcolor;
	switch(StarType())
	{
	case DaStar::TYPE_O:
		ptype = "O";
		pcolor = "Blue";
		break;
	case DaStar::TYPE_B:
		ptype = "B";
		pcolor = "Blue White";
		break;
	case DaStar::TYPE_A:
		ptype = "A";
		pcolor = "White";
		break;
	case DaStar::TYPE_F:
		ptype = "F";
		pcolor = "Yellow White";
		break;
	default:
		Q_ASSERT(false);
	case DaStar::TYPE_G:
		ptype = "G";
		pcolor = "Yellow";
		break;
	case DaStar::TYPE_K:
		ptype = "K";
		pcolor = "Orange";
		break;
	case DaStar::TYPE_M:
		ptype = "M";
		pcolor = "Red";
		break;
	}
	QString sType = QObject::tr("Type %1 %2").arg(ptype).arg(pcolor);

	QString sSize;
	QString sSpecDec;
	switch(StarSize())
	{
	case DaStar::SIZE_Ia:
		sSize = QObject::tr("Size Ia Bright Supergiant");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_Ib:
		sSize = QObject::tr("Size Ib Weaker Supergiant");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_II:
		sSize = QObject::tr("Size II Bright Giant");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_III:
		sSize = QObject::tr("Size III Giant");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_IV:
		sSize = QObject::tr("Size IV Subgiant");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_V:
		sSize = QObject::tr("Size V Main Sequence");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_VI:
		sSize = QObject::tr("Size VI Subdwarf");
		sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
		break;
	case DaStar::SIZE_D:
		sSize = QObject::tr("Size D Dwarf");
		// Spectral decimal classification does not apply to dwarfs.
		break;
	}

	QString sDia;
	double rdia = (star->Radius() / FeetPerAu()) * 2.;
	sDia = QObject::tr("%1au Diameter").arg(rdia);

	QString sTempK;
	quint16 k = star->SurfaceTempK();
	if(k)
		sTempK = QObject::tr("%L1%2K Surface Temperature").arg(k).arg(g_Au.DegreeSymbol());

	// Light intensity
	double rHabLight = DaStar::OrbitRadius(star->HabitableZone());	// Orbit radius in au
	QString sLight;
	if(star->HabitableZone() == -1)
	{
		// Habitable radius is within the star.
		sLight = QObject::tr("No Habitable Zone - Too Cool");
	}
	else if(star->HabitableZone() >= 16)
	{
		// Habitable radius is beyond the solar system.
		sLight = QObject::tr("No Habitable Zone - Too Hot");
	}
	else
	{
		if(rHabLight <= rdia/2.)
			// Habitable radius is within the star.
			sLight = QObject::tr("No Habitable Zone - Too Cool");
		else
			sLight = QObject::tr("Habitable Zone at %1au").arg(rHabLight);
	}

	// Make a list of the natural resources the scanner can detect.
	// Stars are always in the system in my scene.
	map<QString, QString> reslist;
	if(InSensorRange())
	{
		// Scan star for resources.
		QString cname;
		COMMODITYID starres[] =
		{
			CID_TypeOPreons,
			CID_TypeBPreons,
			CID_TypeAPreons,
			CID_TypeFPreons,
			CID_TypeGPreons,
			CID_TypeKPreons,
			CID_TypeMPreons,
		};
		int starrescount = sizeof(starres)/sizeof(COMMODITYID);
		for(int sri=0; sri<starrescount; sri++)
		{
			COMMODITYID cid = starres[sri];
			int prob = star->FetchProbability(cid);
			if(prob)
			{
				// Show Q factor and probability % only if this resource is present.
				cname = g_Au.CommodityName(cid);
				reslist.insert(pair<QString, QString>(cname, QString("Q%1 %2%").arg(star->ResourceQuality(cid)).arg(prob)));
			}
		}
	}

	// Put it all together.
	// Show general info about the star.
	infoLines << sName;
	if(!sOrbiting.isEmpty())
		infoLines << sOrbiting;
	else
		infoLines << QObject::tr("Primary");
	infoLines << " " + sType;
	if(!sSpecDec.isEmpty())
		infoLines << " " + sSpecDec;
	if(!sTempK.isEmpty())
		infoLines << " " + sTempK;
	infoLines << " " + sSize;
	infoLines << " " + sLight;

	if(!InSensorRange())
		infoLines << QObject::tr("Data Limited - Out of Range");

	// Photosphere
	infoLines << QObject::tr("Photosphere");
	infoLines << " " + sDia;
	for(map<QString,QString>::const_iterator ri = reslist.begin(); ri != reslist.end(); ri++)
		infoLines << " " + (*ri).first + " " + (*ri).second;

	return infoLines;
}

float AuBlipStar::ObstacleRadius() const
{
	return Radius();
}

float AuBlipStar::PilotAvoidRadius() const
{
	return Radius() + OrbitAltAboveSun();
}

void AuBlipStar::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	// system: primary star, companion star
	const DaStar* star = (const DaStar*)system->FindIdRecursive(AtomId());
	if(!star)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a circle at the size of the star, flat to the view.
	p.setPen(QPen(star->ChartColor(), 0.f));

	double rad = star->Radius();								// Radius in feet.
	double radpix = rad / feetPerPixel;							// Radius in pixels.
	int lrad = round(radpix);
	QRect rc(-lrad, -lrad, lrad*2+1, lrad*2+1);
	if(lrad == 0)
	{
		rc.setRight(rc.right()+1);
		rc.setBottom(rc.bottom()+1);
	}
	rc.translate(px.x(), px.y());
	p.drawArc(rc, 0, 5760);
}

quint64 AuBlipStar::MemoryUsage() const
{
	quint64 mem = AuSensorBlip::MemoryUsage();
	mem += sizeof(m_InSensorRange);
	mem += sizeof(m_Radius);
	mem += sizeof(m_SpectralClass);
	mem += sizeof(m_StarSize);
	mem += sizeof(m_StarType);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipStar& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipStarCompanion::AuBlipStarCompanion(DaStar::TYPE type, DaStar::SIZE size, quint8 spectralClass, float radius,
		bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipStar(type, size, spectralClass, radius, inSensorRange, id, sysLoc)
{
}

AuBlipStarCompanion::AuBlipStarCompanion()
{
}

AuBlipStarCompanion::AuBlipStarCompanion(const AuBlipStarCompanion& c) : AuBlipStar(c)
{
}

AuSensorBlip* AuBlipStarCompanion::Clone() const
{
	return new AuBlipStarCompanion(*this);
}

void AuBlipStarCompanion::operator>>(QDataStream& out) const
{
	AuBlipStar::operator>>(out);
}

void AuBlipStarCompanion::operator<<(QDataStream& in)
{
	AuBlipStar::operator<<(in);
}

AuPoint AuBlipStarCompanion::WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& weaponLoc, COMMODITYID /*bayModule*/) const
{
	AuPoint vecToWeapon = weaponLoc - EnvLocation();			// Vec to weapon from star.
	vecToWeapon.Unit(Radius() - 10000);							// Vec scaled to 10,000' inside star photosphere.
	return EnvLocation() + vecToWeapon;						// Return system relative lock location.
}

// Name used next to selection reticle.
QString AuBlipStarCompanion::ScreenName() const
{
	return QObject::tr("%1 Companion").arg(DaStar::ScreenName(StarType(), StarSize(), SpectralClass()));
}

QDebug operator<<(QDebug dbg, const AuBlipStarCompanion& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBlackHole::AuBlipBlackHole(DaStar::TYPE type, float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipStar(type, DaStar::SIZE_Ia, 0, radius, inSensorRange, id, sysLoc)
{
	Q_ASSERT(type == DaStar::TYPE_SupermassiveBlackHole || type == DaStar::TYPE_StellarBlackHole);
}

AuBlipBlackHole::AuBlipBlackHole()
{
}

AuBlipBlackHole::AuBlipBlackHole(const AuBlipBlackHole& c) : AuBlipStar(c)
{
}

AuSensorBlip* AuBlipBlackHole::Clone() const
{
	return new AuBlipBlackHole(*this);
}

void AuBlipBlackHole::operator>>(QDataStream& out) const
{
	AuBlipStar::operator>>(out);
}

void AuBlipBlackHole::operator<<(QDataStream& in)
{
	AuBlipStar::operator<<(in);
}

AuPoint AuBlipBlackHole::WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& weaponLoc, COMMODITYID /*bayModule*/) const
{
	AuPoint vecToWeapon = weaponLoc - EnvLocation();			// Vec to weapon from star.
	vecToWeapon.Unit(Radius());									// Vec scaled to the event horizon.
	return EnvLocation() + vecToWeapon;						// Return system relative lock location.
}

// Name used next to selection reticle.
QString AuBlipBlackHole::ScreenName() const
{
	return QObject::tr("Black Hole");
}

QStringList AuBlipBlackHole::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	const DaStar* star = (const DaStar*)system->FindIdRecursive(AtomId());	// system: primary star, companion star
	if(!star)
		return infoLines;

	QString sName = star->Name();

	QString sOrbiting;
	if(!star->IsPrimary())
	{
		// Orbital position.
		double orbitAngle = PI/2 - star->Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(star->Parent()->Name())
			.arg(FormatBigNumber(star->Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	QString ehRad;
	double ehrad = (star->Radius() / FeetPerAu());
	ehRad = QObject::tr("%1au Schwarzschild Radius").arg(ehrad);

	QString psRad;
	double psrad = (star->PhotonSphereRadius() / FeetPerAu());
	psRad = QObject::tr("%1au Photon Sphere Radius").arg(psrad);

	// Put it all together.
	// Show general info about the black hole.
	infoLines << sName;
	if(!sOrbiting.isEmpty())
		infoLines << sOrbiting;
	infoLines << " " + ehRad;
	infoLines << " " + psRad;

	return infoLines;
}

float AuBlipBlackHole::ObstacleRadius() const
{
	return Radius() * 1.5f;										// Photon sphere is 1.5x the radius of the black hole.
}

float AuBlipBlackHole::PilotAvoidRadius() const
{
	return Radius() * 1.5f + OrbitAltAboveSun();				// Photon sphere is 1.5x the radius of the black hole.
}

QDebug operator<<(QDebug dbg, const AuBlipBlackHole& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipNeutronStar::AuBlipNeutronStar(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipStar(DaStar::TYPE_NeutronStar, DaStar::SIZE_Ia, 0, radius, inSensorRange, id, sysLoc)
{
}

AuBlipNeutronStar::AuBlipNeutronStar()
{
}

AuBlipNeutronStar::AuBlipNeutronStar(const AuBlipNeutronStar& c) : AuBlipStar(c)
{
}

AuSensorBlip* AuBlipNeutronStar::Clone() const
{
	return new AuBlipNeutronStar(*this);
}

void AuBlipNeutronStar::operator>>(QDataStream& out) const
{
	AuBlipStar::operator>>(out);
}

void AuBlipNeutronStar::operator<<(QDataStream& in)
{
	AuBlipStar::operator<<(in);
}

// Name used next to selection reticle.
QString AuBlipNeutronStar::ScreenName() const
{
	return QObject::tr("Neutron Star");
}

QStringList AuBlipNeutronStar::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	const DaStar* star = (const DaStar*)system->FindIdRecursive(AtomId());	// system: primary star, companion star
	if(!star)
		return infoLines;

	QString sName = star->Name();
	QString sType = QObject::tr("Neutron Star");

	QString sOrbiting;
	if(!star->IsPrimary())
	{
		// Orbital position.
		double orbitAngle = PI/2 - star->Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(star->Parent()->Name())
			.arg(FormatBigNumber(star->Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	double rdia = (star->Radius() / FeetPerAu()) * 2.;
	QString sDia = QObject::tr("%1au Core Diameter").arg(rdia, 0,'f',2);

	rdia = (star->NeutronSphereRadius() / FeetPerAu()) * 2.;
	QString sNeutronSphere = QObject::tr("%1au Neutron Sphere Diameter").arg(rdia, 0,'f',2);

	QString sLight = QObject::tr("No Habitable Zone - Too Cool");

	// Make a list of the natural resources the scanner can detect.
	// Stars are always in the system in my scene.
	map<QString, QString> reslist;
	if(InSensorRange())
	{
		// Scan star for resources.
		/*
		QString cname;
		COMMODITYID starres[] =
		{
			CID_TypeOPreons,
			CID_TypeBPreons,
			CID_TypeAPreons,
			CID_TypeFPreons,
			CID_TypeGPreons,
			CID_TypeKPreons,
			CID_TypeMPreons,
		};
		int starrescount = sizeof(starres)/sizeof(COMMODITYID);
		for(int sri=0; sri<starrescount; sri++)
		{
			COMMODITYID cid = starres[sri];
			int prob = star->FetchProbability(cid);
			if(prob)
			{
				// Show Q factor and probability % only if this resource is present.
				cname = g_Au.CommodityName(cid);
				reslist.insert(pair<QString, QString>(cname, QString("Q%1 %2%").arg(star->ResourceQuality(cid)).arg(prob)));
			}
		}
		*/
	}

	// Put it all together.
	// Show general info about the black hole.
	infoLines << sName;
	if(!sOrbiting.isEmpty())
		infoLines << sOrbiting;
	else
		infoLines << QObject::tr("Primary");
	infoLines << " " + sType;
	infoLines << " " + sLight;

	if(!InSensorRange())
		infoLines << QObject::tr("Data Limited - Out of Range");

	// Photosphere
	infoLines << QObject::tr("Photosphere");
	infoLines << " " + sDia;
	infoLines << " " + sNeutronSphere;
	for(map<QString,QString>::const_iterator ri = reslist.begin(); ri != reslist.end(); ri++)
		infoLines << " " + (*ri).first + " " + (*ri).second;

	return infoLines;
}

float AuBlipNeutronStar::ObstacleRadius() const
{
	return Radius() * 1.05f;
}

float AuBlipNeutronStar::PilotAvoidRadius() const
{
	return Radius() * 1.05f + OrbitAltAboveSun();
}

QDebug operator<<(QDebug dbg, const AuBlipNeutronStar& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipNeutronStarCompanion::AuBlipNeutronStarCompanion(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipNeutronStar(radius, inSensorRange, id, sysLoc)
{
}

AuBlipNeutronStarCompanion::AuBlipNeutronStarCompanion()
{
}

AuBlipNeutronStarCompanion::AuBlipNeutronStarCompanion(const AuBlipNeutronStarCompanion& c) : AuBlipNeutronStar(c)
{
}

AuSensorBlip* AuBlipNeutronStarCompanion::Clone() const
{
	return new AuBlipNeutronStarCompanion(*this);
}

void AuBlipNeutronStarCompanion::operator>>(QDataStream& out) const
{
	AuBlipNeutronStar::operator>>(out);
}

void AuBlipNeutronStarCompanion::operator<<(QDataStream& in)
{
	AuBlipNeutronStar::operator<<(in);
}

// Name used next to selection reticle.
QString AuBlipNeutronStarCompanion::ScreenName() const
{
	return QObject::tr("%1 Companion").arg(DaStar::ScreenName(StarType(), StarSize(), SpectralClass()));
}

QDebug operator<<(QDebug dbg, const AuBlipNeutronStarCompanion& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipWorld::AuBlipWorld(bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuSensorBlip(id, sysLoc)
{
	m_InSensorRange = inSensorRange;
}

AuBlipWorld::AuBlipWorld()
{
	m_InSensorRange = false;
}

AuBlipWorld::AuBlipWorld(const AuBlipWorld& c) : AuSensorBlip(c)
{
	m_InSensorRange = c.m_InSensorRange;
}

void AuBlipWorld::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);

	out << m_InSensorRange;
}

void AuBlipWorld::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);

	in >> m_InSensorRange;
}

QStringList AuBlipWorld::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	const DaWorld* planet = (const DaWorld*)system->FindIdRecursive(AtomId());	// primary star, companion star, planet, moon
	if(!planet)
		return infoLines;

	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	if(planet->CanHaveFauna() || planet->CanHaveFlora())
		sBio = "Biosphere";								// Animals or plants exist there.
	else if(dynamic_cast<const DaWorldSphere*>(planet))
		sBio = "Biosphere Potential";					// No animals or plants exist there but a colony could make them.

	QString sOrbitZone;
	switch(planet->OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		break;
	}

	QString sLiquid;
	switch(planet->HydrographicLiquid())
	{
	case HYDRO_None:
		break;
	case HYDRO_Lava:
		sLiquid = QObject::tr("Lava");
		break;
	case HYDRO_Water:
		sLiquid = QObject::tr("Water");
		break;
	case HYDRO_SupercooledLiquid:
		sLiquid = QObject::tr("Liquid");
		break;
	}

	const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(planet);
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(planet);
	QString sSize;
	QString sGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	if(ring)
	{
		sBodyType = TypeName();
		double rad = ring->Location().Length();			// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%L1m Radius").arg(rad);
	}
	else if(rsworld)
	{
		sBodyType = QObject::tr("Ringworld Arc %1").arg(rsworld->SectionIndex()+1);
		double rad = rsworld->OrbitRadius();	// Radius in feet.
		rad /= FeetPerMeter();			// Convert to meters.
		rad = round(rad/100) * 100;		// Round to nearest 100 meters.
		sSize = QObject::tr("%1Lm Radius").arg(rad);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(rsworld->Gravity(0)/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());

		if(planet->MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density")
				.arg(planet->MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(planet->AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");

		if(planet->Hydrographics() > 0)
			sWater = QObject::tr("%1% Surface %2").arg(planet->Hydrographics()).arg(sLiquid);
		else
			sWater = QObject::tr("No Surface %1").arg(sLiquid);
	}
	else
	{
		sBodyType = TypeName();
		double rdia = planet->SeaLevelAltitude() * 2.;	// Diameter in feet.
		rdia /= FeetPerMeter();			// Convert to meters.
		rdia = round(rdia/100) * 100;	// Round to nearest 100 meters.
		sSize = QObject::tr("%1m Diameter").arg(rdia);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(planet->Gravity(planet->SeaLevelAltitude())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());

		if(planet->MSLAtmosphereDensity() > 0)
			sAtm = QObject::tr("%2, %1% Density")
				.arg(planet->MSLAtmosphereDensity()*100)
				.arg(AtmoTypeName(planet->AtmosphereType()));
		else
			sAtm = QObject::tr("No Atmosphere");

		if(planet->Hydrographics() > 0)
			sWater = QObject::tr("%1% Surface %2").arg(planet->Hydrographics()).arg(sLiquid);
		else
			sWater = QObject::tr("No Surface %1").arg(sLiquid);
	}

	// Make a list of the natural resources the scanner can detect.
	map<QString, QString> atmlist;			// Atmosphere resources.
	map<QString, QString> biolist;			// Biosphere resources.
	map<QString, QString> geolist;			// Geosphere resources.
	map<QString, QString> hydlist;			// Geosphere resources.
	if(InSensorRange())
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<planet->ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			planet->QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Biosphere
			if(!sBio.isEmpty())
			{
				COMMODITYID biores[] =
				{
					CID_Animal,
					CID_Beans,
					CID_Cheese,
					CID_Eggs,
					CID_Fertilizer,
					CID_Fiber,
					CID_Fish,
					CID_Fruit,
					CID_Grain,
					CID_Grapes,
					CID_Herbs,
					CID_Hops,
					CID_Milk,
					CID_Nuts,
					CID_Spices,
					CID_Vegetable,
					CID_Vegetation,
					CID_Wood,
				};
				int biorescount = sizeof(biores)/sizeof(COMMODITYID);
				ReportZoneBioResources(rz, res, biores, biorescount, biocount, biolist);
			}

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = atmlist.find(g_Au.CommodityName((*bci).first));
				if(ni != atmlist.end())
					atmlist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = biolist.find(g_Au.CommodityName((*bci).first));
				if(ni != biolist.end())
					biolist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = geolist.find(g_Au.CommodityName((*bci).first));
				if(ni != geolist.end())
					geolist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = hydlist.find(g_Au.CommodityName((*bci).first));
				if(ni != hydlist.end())
					hydlist.erase(ni);
			}
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - planet->Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(planet->Parent()->Name())
			.arg(FormatBigNumber(planet->Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	// Show page of general info about the planet.
	QString defname = planet->DefaultName(system->Name());
	infoLines << planet->Name();
	if(defname != planet->Name())
		infoLines << defname;
	infoLines << sOrbiting;
	infoLines << " " + sOrbitZone;

	if(!InSensorRange())
		infoLines << QObject::tr("Data Limited - Out of Range");

	// Geosphere
	infoLines << QObject::tr("Geosphere");
	infoLines << " " + sBodyType + ", " + sSize;
	if(!sGrav.isEmpty())
		infoLines << " " + sGrav;
	for(map<QString,QString>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
		infoLines << " " + (*ri).first + " " + (*ri).second;

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		infoLines << QObject::tr("Hydrosphere");
		infoLines << " " + sWater;
		for(map<QString,QString>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.empty())
	{
		infoLines << QObject::tr("Atmosphere");
		if(!sAtm.isEmpty())
			infoLines << " " + sAtm;
		for(map<QString,QString>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	// Biosphere
	if(!sBio.isEmpty() && !biolist.empty())
	{
		infoLines << sBio;
		for(map<QString,QString>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	return infoLines;
}

void AuBlipWorld::ReportZoneFetchResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, map<QString, QString>& zonelist) const
{
	COMMODITYID cid;
	QString cname;
	for(int zri=0; zri<zonerescount; zri++)
	{
		cid = zoneres[zri];
		cname = g_Au.CommodityName(cid);

		// Determine if the natural resource can be acquired here.
		int prob = res.FetchProbability(cid);
		if(prob)
		{
			// Show Q factor and probability %.
			if(rz == 0)
			{
				zonecount.insert(pair<COMMODITYID, int>(cid, prob));
				zonelist.insert(pair<QString, QString>(cname, QString("Q%1 %2%").arg(res.Quality(cid)).arg(prob)));
			}
			else
			{
				zonecount[cid] += prob;
				zonelist[cname] += QString(", Q%1 %2%").arg(res.Quality(cid)).arg(prob);
			}
		}
		else
		{
			// Don't show a Q factor when the probability is 0%.
			if(rz == 0)
			{
				zonecount.insert(pair<COMMODITYID, int>(cid, 0));
				zonelist.insert(pair<QString, QString>(cname, QString("None")));
			}
			else
				zonelist[cname] += QString(", None");
		}
	}
}

void AuBlipWorld::ReportZoneBioResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
		unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, map<QString, QString>& zonelist) const
{
	for(int zri=0; zri<zonerescount; zri++)
	{
		COMMODITYID cid = zoneres[zri];
		QString cname = g_Au.CommodityName(cid);
		quint8 quality = res.Quality(cid);

		// Determine if the natural resource can be acquired here.
		if(quality)
		{
			// Show Q factor and probability %.
			if(rz == 0)
			{
				zonecount.insert(pair<COMMODITYID, int>(cid, 1));
				zonelist.insert(pair<QString, QString>(cname, QString("Q%1").arg(quality)));
			}
			else
			{
				zonecount[cid] = 1;
				zonelist[cname] += QString(", Q%1").arg(quality);
			}
		}
		else
		{
			// Don't show a Q factor when the probability is 0%.
			if(rz == 0)
			{
				zonecount.insert(pair<COMMODITYID, int>(cid, 0));
				zonelist.insert(pair<QString, QString>(cname, QString("None")));
			}
			else
				zonelist[cname] += QString(", None");
		}
	}
}

QDebug operator<<(QDebug dbg, const AuBlipWorld& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

quint64 AuBlipWorld::MemoryUsage() const
{
	quint64 mem = AuSensorBlip::MemoryUsage();
	mem += sizeof(m_InSensorRange);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipGasGiant::AuBlipGasGiant(quint8 radius1000km, quint8 coreRadius1000km, bool inSensorRange, const AuId& id,
		const AuPoint& sysLoc) : AuBlipWorld(inSensorRange, id, sysLoc)
{
	m_CoreRadius1000km = coreRadius1000km;
	m_Radius1000km = radius1000km;
}

AuBlipGasGiant::AuBlipGasGiant()
{
	m_CoreRadius1000km = 0;
	m_Radius1000km = 0;
}

AuBlipGasGiant::AuBlipGasGiant(const AuBlipGasGiant& c) : AuBlipWorld(c)
{
	m_CoreRadius1000km = c.m_CoreRadius1000km;
	m_Radius1000km = c.m_Radius1000km;
}

AuSensorBlip* AuBlipGasGiant::Clone() const
{
	return new AuBlipGasGiant(*this);
}

void AuBlipGasGiant::operator>>(QDataStream& out) const
{
	AuBlipWorld::operator>>(out);

	out << m_CoreRadius1000km;
	out << m_Radius1000km;
}

void AuBlipGasGiant::operator<<(QDataStream& in)
{
	AuBlipWorld::operator<<(in);

	in >> m_CoreRadius1000km;
	in >> m_Radius1000km;
}

float AuBlipGasGiant::AtmosphereRadius() const
{
	return (float)::Radius100km(Radius1000km()*10) + AtmosphereTopGasGiantMSL();
}

AuPoint AuBlipGasGiant::WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& weaponLoc,
		COMMODITYID /*bayModule*/) const
{
	AuPoint vecToWeapon = weaponLoc - EnvLocation();			// Vec to weapon from planet.
	vecToWeapon.Unit(AtmosphereRadius() - 1000);				// Vec scaled to 1,000' inside planet atmosphere.
	return EnvLocation() + vecToWeapon;						// Return system relative lock location.
}

// Name used next to selection reticle.
QString AuBlipGasGiant::ScreenName() const
{
	return BodyTypeName(BODY_GasGiant);
}

float AuBlipGasGiant::ObstacleRadius() const
{
	return AtmosphereRadius();
}

float AuBlipGasGiant::PilotAvoidRadius() const
{
	return AtmosphereRadius() + OrbitAltAboveAtm();
}

void AuBlipGasGiant::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	// system: primary star, companion star, planet
	const DaWorldGasGiant* planet = dynamic_cast<const DaWorldGasGiant*>(system->FindIdRecursive(AtomId()));
	if(!planet)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a circle at the size of the planet, flat to the view.
	p.setPen(QPen(planet->AtmosphereColor(), 0.f));

	double rad = planet->SeaLevelRadius();						// Radius in feet.
	double radpix = rad / feetPerPixel;							// Radius in pixels.
	int lrad = round(radpix);
	QRect rc(-lrad, -lrad, lrad*2+1, lrad*2+1);
	if(lrad == 0)
	{
		rc.setRight(rc.right()+1);
		rc.setBottom(rc.bottom()+1);
	}
	rc.translate(px.x(), px.y());
	p.drawArc(rc, 0, 5760);
}

QDebug operator<<(QDebug dbg, const AuBlipGasGiant& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtmosphereRadius() << blip.AtomId() << blip.EnvLocation()
		<< blip.CoreRadius1000km();
	return dbg.space();
}

QStringList AuBlipGasGiant::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	const DaWorldGasGiant* planet = (const DaWorldGasGiant*)system->FindIdRecursive(AtomId());
	if(!planet)
		return infoLines;

	// Biosphere exists where it is possible. It is shown as 'potential' where it can be grown at a colony. Blank where
	// no biosphere can exist, such as gas giants and rocky rings.
	QString sBio;
	if(planet->CanHaveFauna() || planet->CanHaveFlora())
		sBio = "Biosphere";								// Animals or plants exist there.
	else
		sBio = "Biosphere Potential";					// No animals or plants exist there but a colony could make them.

	QString sOrbitZone;
	switch(planet->OrbitZone())
	{
	case OZONE_Inferno:
		sOrbitZone = QObject::tr("Inferno Zone");
		break;
	case OZONE_Inner:
		sOrbitZone = QObject::tr("Inner Zone");
		break;
	case OZONE_Habitable:
		sOrbitZone = QObject::tr("Habitable Zone");
		break;
	case OZONE_Outer:
		sOrbitZone = QObject::tr("Outer Zone");
		break;
	case OZONE_Frigid:
		sOrbitZone = QObject::tr("Frigid Zone");
		break;
	case OZONE_DeepSpace:
		sOrbitZone = QObject::tr("Deep Space");
		break;
	}

	QString sLiquid;
	switch(planet->HydrographicLiquid())
	{
	case HYDRO_None:
		break;
	case HYDRO_Lava:
		sLiquid = QObject::tr("Lava");
		break;
	case HYDRO_Water:
		sLiquid = QObject::tr("Water");
		break;
	case HYDRO_SupercooledLiquid:
		sLiquid = QObject::tr("Liquid");
		break;
	}

	QString sSize;
	QString sGrav;
	QString sGeoSize;
	QString sGeoGrav;
	QString sAtm;
	QString sWater;
	QString sBodyType;
	{
		sBodyType = TypeName();
		double rdia = planet->SeaLevelAltitude() * 2.;	// Diameter in feet.
		rdia /= FeetPerMeter();			// Convert to meters.
		rdia = round(rdia/100) * 100;	// Round to nearest 100 meters.
		sSize = QObject::tr("%1m Diameter").arg(rdia);
		sGrav = QObject::tr("%1m/s%2 Sea Level Gravity")
			.arg(planet->Gravity(planet->SeaLevelAltitude())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());
		double rgeodia = planet->GlobeRadius() * 2.;	// Diameter in feet.
		rgeodia /= FeetPerMeter();			// Convert to meters.
		rgeodia = round(rgeodia/100) * 100;	// Round to nearest 100 meters.
		sGeoSize = QObject::tr("%1m Diameter").arg(rgeodia);
		sGeoGrav = QObject::tr("%1m/s%2 Core Gravity")
			.arg(planet->Gravity(planet->GlobeRadius())/FeetPerMeter(), 0,'f',2)
			.arg(AuGlobal::SquaredSymbol());

		sAtm = QObject::tr("%2, %1% Density")
			.arg(planet->MSLAtmosphereDensity()*100)
			.arg(AtmoTypeName(planet->AtmosphereType()));

		sWater = QObject::tr("%1% Surface %2").arg(planet->Hydrographics()).arg(sLiquid);
	}

	// Make a list of the natural resources the scanner can detect.
	map<QString, QString> atmlist;			// Atmosphere resources.
	map<QString, QString> biolist;			// Biosphere resources.
	map<QString, QString> geolist;			// Geosphere resources.
	map<QString, QString> hydlist;			// Geosphere resources.
	if(InSensorRange())
	{
		// Must scan every resource zone.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> atmcount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> biocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> geocount;	// Count of total between all zones. We will remove the zero ones at the end.
		unordered_map<COMMODITYID, int, COMMODITYID_Hash> hydcount;	// Count of total between all zones. We will remove the zero ones at the end.
		for(int rz=0; rz<planet->ResourceZones(); rz++)
		{
			QString cname;
			AuZoneResource res;
			planet->QueryNaturalResources(res, rz);

			if(rz == 0)
			{
				// These resources exist in only a single planet-wide zone.
				// There is no need to repeat the information in multiple zones.

				// Atmosphere
				COMMODITYID atmres[] =
				{
					CID_Air,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					CID_Cryozine,
					CID_Hydrogen,
					CID_Ioplasma,
				};
				int atmrescount = sizeof(atmres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, atmres, atmrescount, atmcount, atmlist);

				// Hydrosphere
				COMMODITYID hydres[] =
				{
					CID_Hydrographics,
					CID_Magmex,
					CID_Myrathane,
					CID_Flomentum,
				};
				int hydrescount = sizeof(hydres)/sizeof(COMMODITYID);
				ReportZoneFetchResources(rz, res, hydres, hydrescount, hydcount, hydlist);
			}

			// Remaining resources can exist in multiple zones.
			// If they are in any zone, we need to report all the zones so the scanner shows which zone contains each Q.
			// To do this, we'll report all of them and keep count of the total probability.
			// At the end, we'll remove the ones that ended up with zero total probability.

			// Biosphere
			if(!sBio.isEmpty())
			{
				COMMODITYID biores[] =
				{
					CID_Animal,
					CID_Beans,
					CID_Cheese,
					CID_Eggs,
					CID_Fertilizer,
					CID_Fiber,
					CID_Fish,
					CID_Fruit,
					CID_Grain,
					CID_Grapes,
					CID_Herbs,
					CID_Hops,
					CID_Milk,
					CID_Nuts,
					CID_Spices,
					CID_Vegetable,
					CID_Vegetation,
					CID_Wood,
				};
				int biorescount = sizeof(biores)/sizeof(COMMODITYID);
				ReportZoneBioResources(rz, res, biores, biorescount, biocount, biolist);
			}

			// Geosphere
			COMMODITYID geores[] =
			{
				CID_Bolite,
				CID_Coal,
				CID_CrudeOil,
				CID_Crystals,
				CID_Eludium,
				CID_Gems,
				CID_GoldOre,
				CID_Ice,
				CID_Lumenite,
				CID_Minerals,
				CID_NaturalGas,
				CID_Ore,
				CID_Phlogiston,
				CID_Polytaride,
				CID_Radioactives,
				CID_Stone,
				CID_Vulcanite,
				CID_Adamantite,
				CID_Viathol,
			};
			int georescount = sizeof(geores)/sizeof(COMMODITYID);
			ReportZoneFetchResources(rz, res, geores, georescount, geocount, geolist);
		}

		// Remove the ones that ended up with zero total probability.
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = atmcount.begin(); bci != atmcount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = atmlist.find(g_Au.CommodityName((*bci).first));
				if(ni != atmlist.end())
					atmlist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = biocount.begin(); bci != biocount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = biolist.find(g_Au.CommodityName((*bci).first));
				if(ni != biolist.end())
					biolist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = geocount.begin(); bci != geocount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = geolist.find(g_Au.CommodityName((*bci).first));
				if(ni != geolist.end())
					geolist.erase(ni);
			}
		for(unordered_map<COMMODITYID, int, COMMODITYID_Hash>::const_iterator bci = hydcount.begin(); bci != hydcount.end(); bci++)
			if(!(*bci).second)
			{
				map<QString, QString>::iterator ni = hydlist.find(g_Au.CommodityName((*bci).first));
				if(ni != hydlist.end())
					hydlist.erase(ni);
			}
	}

	// Orbital position.
	QString sOrbiting;
	{
		double orbitAngle = PI/2 - planet->Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(planet->Parent()->Name())
			.arg(FormatBigNumber(planet->Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	// Put it all together.
	// Show page of general info about the planet.
	QString defname = planet->DefaultName(system->Name());
	infoLines << planet->Name();
	if(defname != planet->Name())
		infoLines << defname;
	infoLines << sOrbiting;
	infoLines << " " + sOrbitZone;

	if(!InSensorRange())
		infoLines << QObject::tr("Data Limited - Out of Range");

	// Geosphere
	infoLines << QObject::tr("Geosphere");
	infoLines << " " + sBodyType + ", " + sGeoSize;
	infoLines << " " + sGeoGrav;
	infoLines << " " + sGrav;
	for(map<QString,QString>::const_iterator ri = geolist.begin(); ri != geolist.end(); ri++)
		infoLines << " " + (*ri).first + " " + (*ri).second;

	// Hydrosphere
	if(!sWater.isEmpty())
	{
		infoLines << QObject::tr("Hydrosphere");
		infoLines << " " + sWater;
		infoLines << " " + sSize;
		for(map<QString,QString>::const_iterator ri = hydlist.begin(); ri != hydlist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	// Atmosphere
	if(!sAtm.isEmpty() || !atmlist.empty())
	{
		infoLines << QObject::tr("Atmosphere");
		if(!sAtm.isEmpty())
			infoLines << " " + sAtm;
		for(map<QString,QString>::const_iterator ri = atmlist.begin(); ri != atmlist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	// Biosphere
	if(!sBio.isEmpty() && !biolist.empty())
	{
		infoLines << sBio;
		for(map<QString,QString>::const_iterator ri = biolist.begin(); ri != biolist.end(); ri++)
			infoLines << " " + (*ri).first + " " + (*ri).second;
	}

	return infoLines;
}

quint64 AuBlipGasGiant::MemoryUsage() const
{
	quint64 mem = AuBlipWorld::MemoryUsage();
	mem += sizeof(m_CoreRadius1000km);
	mem += sizeof(m_Radius1000km);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipGlobe::AuBlipGlobe(quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipWorld(inSensorRange, id, sysLoc)
{
	m_Radius100km = radius100km;
}

AuBlipGlobe::AuBlipGlobe()
{
	m_Radius100km = 0;
}

AuBlipGlobe::AuBlipGlobe(const AuBlipGlobe& c) : AuBlipWorld(c)
{
	m_Radius100km = c.m_Radius100km;
}

void AuBlipGlobe::operator>>(QDataStream& out) const
{
	AuBlipWorld::operator>>(out);

	out << m_Radius100km;
}

void AuBlipGlobe::operator<<(QDataStream& in)
{
	AuBlipWorld::operator<<(in);

	in >> m_Radius100km;
}

float AuBlipGlobe::AtmosphereRadius() const
{
	return (float)::Radius100km(Radius100km()) + AtmosphereTopMSL();
}

float AuBlipGlobe::Radius() const
{
	return (float)::Radius100km(Radius100km());
}

AuPoint AuBlipGlobe::WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const
{
	AuPoint vecToWeapon = weaponLoc - EnvLocation();			// Vec to weapon from planet.

	// Scale vector to the weapon lock location.
	if(bayModule == CID_AtmosphereCondenserBayModule ||
			bayModule == CID_ParticleCollectorBayModule)
		vecToWeapon.Unit(AtmosphereRadius() - 1000);			// Vec scaled to 1,000' inside planet atmosphere.
	else if(bayModule == CID_SiphonBayModule)
		vecToWeapon.Unit(Radius());								// Vec scaled to sea surface.
	else
	{
		// Planet must calculate closest point on terrain from ship.
		// system: primary star, companion star, planet, moon
		const DaWorldGlobe* planet = dynamic_cast<const DaWorldGlobe*>(system->FindIdRecursive(AtomId()));
		if(!planet)
			return AuBlipWorld::WeaponLockLocation(system, weaponLoc, bayModule);

		AuPoint shipLoc = planet->EnvTransform().UnProject(weaponLoc);		// Ship loc rel to planet.
		AuGlobeLL ll;
		planet->LLOfLoc(ll, shipLoc);
		vecToWeapon.Unit(Radius() + planet->TerrainAltitudeMSL(ll));			// Vec scaled to ground surface.
	}

	return EnvLocation() + vecToWeapon;						// Return system relative lock location.
}

float AuBlipGlobe::ObstacleRadius() const
{
	return AtmosphereRadius();
}

float AuBlipGlobe::PilotAvoidRadius() const
{
	return AtmosphereRadius() + OrbitAltAboveAtm();
}

void AuBlipGlobe::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	// system: primary star, companion star, planet, moon
	const DaWorldGlobe* planet = dynamic_cast<const DaWorldGlobe*>(system->FindIdRecursive(AtomId()));
	if(!planet)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a circle at the size of the planet, flat to the view.
	p.setPen(QPen(planet->AtmosphereColor(), 0.f));

	double rad = planet->SeaLevelRadius();						// Radius in feet.
	double radpix = rad / feetPerPixel;							// Radius in pixels.
	int lrad = round(radpix);
	QRect rc(-lrad, -lrad, lrad*2+1, lrad*2+1);
	if(lrad == 0)
	{
		rc.setRight(rc.right()+1);
		rc.setBottom(rc.bottom()+1);
	}
	rc.translate(px.x(), px.y());
	p.drawArc(rc, 0, 5760);
}

quint64 AuBlipGlobe::MemoryUsage() const
{
	quint64 mem = AuBlipWorld::MemoryUsage();
	mem += sizeof(m_Radius100km);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipTitan::AuBlipTitan(HYDROGRAPHIC_LIQUID hydLiquid, quint8 radius100km, bool inSensorRange, const AuId& id,
		const AuPoint& sysLoc) : AuBlipGlobe(radius100km, inSensorRange, id, sysLoc)
{
	m_HydLiquid = hydLiquid;
}

AuBlipTitan::AuBlipTitan()
{
	m_HydLiquid = HYDRO_None;
}

AuBlipTitan::AuBlipTitan(const AuBlipTitan& c) : AuBlipGlobe(c)
{
	m_HydLiquid = c.m_HydLiquid;
}

AuSensorBlip* AuBlipTitan::Clone() const
{
	return new AuBlipTitan(*this);
}

void AuBlipTitan::operator>>(QDataStream& out) const
{
	AuBlipGlobe::operator>>(out);

	out << m_HydLiquid;
}

void AuBlipTitan::operator<<(QDataStream& in)
{
	AuBlipGlobe::operator<<(in);

	in >> m_HydLiquid;
}

// Name used next to selection reticle.
QString AuBlipTitan::ScreenName() const
{
	return BodyTypeName(BODY_Titan);
}

quint64 AuBlipTitan::MemoryUsage() const
{
	quint64 mem = AuBlipGlobe::MemoryUsage();
	mem += sizeof(m_HydLiquid);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipTitan& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipMoon::AuBlipMoon(quint8 radius100km, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipGlobe(radius100km, inSensorRange, id, sysLoc)
{
}

AuBlipMoon::AuBlipMoon()
{
}

AuBlipMoon::AuBlipMoon(const AuBlipMoon& c) : AuBlipGlobe(c)
{
}

AuSensorBlip* AuBlipMoon::Clone() const
{
	return new AuBlipMoon(*this);
}

void AuBlipMoon::operator>>(QDataStream& out) const
{
	AuBlipGlobe::operator>>(out);
}

void AuBlipMoon::operator<<(QDataStream& in)
{
	AuBlipGlobe::operator<<(in);
}

// Name used next to selection reticle.
QString AuBlipMoon::ScreenName() const
{
	return BodyTypeName(BODY_Moon);
}

QDebug operator<<(QDebug dbg, const AuBlipMoon& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipPlanet::AuBlipPlanet(HYDROGRAPHIC_LIQUID hydLiquid, quint8 radius100km, bool inSensorRange, const AuId& id,
		const AuPoint& sysLoc) : AuBlipGlobe(radius100km, inSensorRange, id, sysLoc)
{
	m_HydLiquid = hydLiquid;
}

AuBlipPlanet::AuBlipPlanet()
{
	m_HydLiquid = HYDRO_None;
}

AuBlipPlanet::AuBlipPlanet(const AuBlipPlanet& c) : AuBlipGlobe(c)
{
	m_HydLiquid = c.m_HydLiquid;
}

AuSensorBlip* AuBlipPlanet::Clone() const
{
	return new AuBlipPlanet(*this);
}

void AuBlipPlanet::operator>>(QDataStream& out) const
{
	AuBlipGlobe::operator>>(out);

	out << m_HydLiquid;
}

void AuBlipPlanet::operator<<(QDataStream& in)
{
	AuBlipGlobe::operator<<(in);

	in >> m_HydLiquid;
}

// Name used next to selection reticle.
QString AuBlipPlanet::ScreenName() const
{
	return BodyTypeName(BODY_Planet);
}

quint64 AuBlipPlanet::MemoryUsage() const
{
	quint64 mem = AuBlipGlobe::MemoryUsage();
	mem += sizeof(m_HydLiquid);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipPlanet& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipRing::AuBlipRing(float radius, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipWorld(inSensorRange, id, sysLoc)
{
	m_Radius = radius;
}

AuBlipRing::AuBlipRing()
{
	m_Radius = 0;
}

AuBlipRing::AuBlipRing(const AuBlipRing& c) : AuBlipWorld(c)
{
	m_Radius = c.m_Radius;
}

AuSensorBlip* AuBlipRing::Clone() const
{
	return new AuBlipRing(*this);
}

void AuBlipRing::operator>>(QDataStream& out) const
{
	AuBlipWorld::operator>>(out);

	out << m_Radius;
}

void AuBlipRing::operator<<(QDataStream& in)
{
	AuBlipWorld::operator<<(in);

	in >> m_Radius;
}

// Returns distance to this sensor blip.
double AuBlipRing::Range(const AuPoint& sysLoc) const
{
	AuPoint lockLoc = sysLoc.ClosestPointOnCircle(EnvLocation(), Radius(), AuPoint(0,0,1));	// Closest point on ring in sys.
	return lockLoc.Distance(sysLoc);							// Return system relative lock location.
}

// Returns squared distance to this sensor blip.
double AuBlipRing::Range2(const AuPoint& sysLoc) const
{
	AuPoint lockLoc = sysLoc.ClosestPointOnCircle(EnvLocation(), Radius(), AuPoint(0,0,1));	// Closest point on ring in sys.
	return lockLoc.Distance2(sysLoc);							// Return system relative lock location.
}

AuPoint AuBlipRing::WeaponLockLocation(const DaAtom* /*system*/, const AuPoint& weaponLoc, COMMODITYID /*bayModule*/) const
{
	AuPoint lockLoc = weaponLoc.ClosestPointOnCircle(EnvLocation(), Radius(), AuPoint(0,0,1));	// Closest point on ring in sys.
	return lockLoc;												// Return system relative lock location.
}

// Name used next to selection reticle.
QString AuBlipRing::ScreenName() const
{
	return BodyTypeName(BODY_Ring);
}

// Calculate 2D distance from this blip to a pixel, when projected onto a display screen.
// 'mScreen' is the projection to the screen.
// 'pxHit' is the pixel location hit on the screen.
double AuBlipRing::DisplayDistance(const AuMatrix4x4& mScreen, const QPoint& pxHit, const DaAtom* system,
		float feetPerPixel) const
{
	// Ring must calculate closest point on ring from weapon.
	// system: primary star, companion star, planet, moon
	const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(system->FindIdRecursive(AtomId()));
	if(!ring)
		return AuBlipWorld::DisplayDistance(mScreen, pxHit, system, feetPerPixel);

	// The ring is drawn as a circle around the parent planet.
	double ringRad = ring->Location().Length();
	double cpx = (ringRad*2 / feetPerPixel) * PI;				// circumference in pixels
	int segs = cpx / 8;											// 8 pixels per segment
	if(segs < 5)
		segs = 5;
	else if(segs > 100)
		segs = 100;
	float ang;
	AuPoint arcpt, fpxarc;
	double bestdist = 0;
	for(int seg=0; seg<segs; seg++)
	{
		ang = (PIf*2) * (float)seg / (float)segs;
		arcpt = EnvLocation() + AuPoint(ringRad, 0).RotateZ(ang);
		fpxarc = mScreen.Project(arcpt);
		double rdist = AuPoint(pxHit).Distance_2D(fpxarc);
		if(seg == 0 || rdist < bestdist)
			bestdist = rdist;
	}
	return bestdist;
}

void AuBlipRing::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	// system: primary star, companion star, planet, moon
	const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(system->FindIdRecursive(AtomId()));
	if(!ring)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a circle of dots around the parent planet in the plane of the solar system.
	p.setPen(QPen(QColor(128,128,128), 0.f));

	double ringRad = ring->Location().Length();					// Radius of the ring in feet.
	double cpx = (ringRad*2 / feetPerPixel) * PI;				// circumference in pixels
	int segs = cpx / 8;											// 8 pixels per segment
	if(segs < 5)
		segs = 5;
	else if(segs > 100)
		segs = 100;
	float ang;
	AuPoint arcpt, fpxarc;
	for(int seg=0; seg<segs; seg++)
	{
		ang = (PIf*2) * (float)seg / (float)segs;
		arcpt = EnvLocation() + AuPoint(ringRad, 0).RotateZ(ang);
		fpxarc = t.Project(arcpt);
		p.drawPoint(QPointF(fpxarc.x, fpxarc.y));
	}
}

quint64 AuBlipRing::MemoryUsage() const
{
	quint64 mem = AuBlipWorld::MemoryUsage();
	mem += sizeof(m_Radius);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipRing& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipRingSection::AuBlipRingSection(quint8 sectionIndex, bool inSensorRange, const AuId& id, const AuPoint& sysLoc) :
	AuBlipWorld(inSensorRange, id, sysLoc)
{
	m_SectionIndex = sectionIndex;
}

AuBlipRingSection::AuBlipRingSection()
{
	m_SectionIndex = 0;
}

AuBlipRingSection::AuBlipRingSection(const AuBlipRingSection& c) : AuBlipWorld(c)
{
	m_SectionIndex = c.m_SectionIndex;
}

AuSensorBlip* AuBlipRingSection::Clone() const
{
	return new AuBlipRingSection(*this);
}

void AuBlipRingSection::operator>>(QDataStream& out) const
{
	AuBlipWorld::operator>>(out);

	out << m_SectionIndex;
}

void AuBlipRingSection::operator<<(QDataStream& in)
{
	AuBlipWorld::operator<<(in);

	in >> m_SectionIndex;
}

AuPoint AuBlipRingSection::WeaponLockLocation(const DaAtom* system, const AuPoint& weaponLoc, COMMODITYID bayModule) const
{
	// Ringworld section must calculate closest point on ringworld surface from ship.
	// system: primary star, companion star, ringworld
	const DaWorldRingSection* ring = dynamic_cast<const DaWorldRingSection*>(system->FindIdRecursive(AtomId()));
	if(!ring)
		return AuBlipWorld::WeaponLockLocation(system, weaponLoc, bayModule);

	double seaRad = ring->OrbitRadius();						// Radius of ringworld sea level.
	AuPoint cen = ring->Parent()->EnvLocation();				// Location of center of the ringworld.

	if(bayModule == CID_AtmosphereCondenserBayModule || bayModule == CID_ParticleCollectorBayModule)
	{
		double atmRad = seaRad - ring->AtmosphereRadius() + 1000;	// Radius 1000' inside ringworld atmosphere.
		return weaponLoc.ClosestPointOnCircle(cen, atmRad, AuPoint(0,0,1));	// Closest loc rel to sys.
	}
	else if(bayModule == CID_SiphonBayModule)
	{
		return weaponLoc.ClosestPointOnCircle(cen, seaRad, AuPoint(0,0,1));	// Closest loc rel to sys.
	}
	else
	{
		AuPoint lockLoc = weaponLoc.ClosestPointOnCircle(cen, seaRad, AuPoint(0,0,1));	// Closest loc rel to sys.
		AuPoint ringloc = ring->EnvTransform().UnProject(lockLoc);	// loc rel to ring
		AuGlobeLL ll;
		if(ring->LLOfLoc(ll, ringloc))
		{
			// Adjust lock location to the terrain surface.
			ring->ElevateLocMSL(ringloc, ring->TerrainAltitudeMSL(ll));
			lockLoc = ring->EnvTransform().Project(ringloc);	// Convert loc rel to sys.
		}
		return lockLoc;
	}
}

// Name used next to selection reticle.
QString AuBlipRingSection::ScreenName() const
{
	return QObject::tr("Ringworld Arc %1").arg(SectionIndex() + 1);
}

// Calculate 2D distance from this blip to a pixel, when projected onto a display screen.
// 'mScreen' is the projection to the screen.
// 'pxHit' is the pixel location hit on the screen.
double AuBlipRingSection::DisplayDistance(const AuMatrix4x4& mScreen, const QPoint& pxHit, const DaAtom* system,
		float feetPerPixel) const
{
	// Ring must calculate closest point on ring from weapon.
	// system: primary star, companion star, ringworld
	const DaWorldRingSection* ring = dynamic_cast<const DaWorldRingSection*>(system->FindIdRecursive(AtomId()));
	if(!ring)
		return AuBlipWorld::DisplayDistance(mScreen, pxHit, system, feetPerPixel);

	// The ringworld section is drawn as a circle around the parent star.
	AuPoint sunLoc = ring->Parent()->EnvLocation();
	double ringRad = ring->OrbitRadius();
	double cpx = (ringRad*2 / feetPerPixel) * PI;				// circumference in pixels
	int segs = cpx / 40;										// 40 pixels per segment
	if(segs < 32)
		segs = 32;
	else if(segs > 100)
		segs = 100;
	float ang;
	AuPoint arcpt, fpxarc;
	double bestdist = 0;
	for(int seg=0; seg<segs; seg++)
	{
		ang = (PIf*2) * (float)seg / (float)segs;
		arcpt = sunLoc + AuPoint(ringRad, 0).RotateZ(ang);
		fpxarc = mScreen.Project(arcpt);
		double rdist = AuPoint(pxHit).Distance_2D(fpxarc);
		if(seg == 0 || rdist < bestdist)
			bestdist = rdist;
	}
	return bestdist;
}

void AuBlipRingSection::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	// We will draw the entire ringworld so this is only done for ringworld section 0.
	if(SectionIndex())
		return;

	// system: primary star, companion star, ringworld
	const DaWorldRingSection* ring = dynamic_cast<const DaWorldRingSection*>(system->FindIdRecursive(AtomId()));
	if(!ring)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a pair of circles above/below the plane of the ringworld, spaced according to its width.
	AuPoint sunLoc = ring->Parent()->EnvLocation();
	double ringWid = DaWorldRingSection::TerrainYSize();
	double ringWid2 = ringWid/2;									    // 1/2 the ring width
	double ringRad = ring->OrbitRadius();
	double cpx = (ringRad*2 / feetPerPixel) * PI;						// circumference in pixels
	int segs = cpx / 40;												// 40 pixels per segment
	if(segs < 32)
		segs = 32;
	else if(segs > 100)
		segs = 100;
	float ang;
	AuPoint arcptT, arcptB, fpxarcT, fpxarcB;
	p.setPen(QPen(QColor(192,192,192), 0.f));
	AuPoint prevpxT = t.Project(sunLoc + AuPoint(ringRad, 0, ringWid2));		// Starting point at angle 0.
	AuPoint prevpxB = t.Project(sunLoc + AuPoint(ringRad, 0, -ringWid2));	// Starting point at angle 0.
	if((int)prevpxT.x == (int)prevpxB.x && (int)prevpxT.y == (int)prevpxB.y)
	{
		// Only one ring is needed.
		for(int seg=1; seg<=segs; seg++)
		{
			if(seg < segs)
				ang = (PIf*2) * (float)seg / (float)segs;
			else
				ang = 0;
			arcptT = sunLoc + AuPoint(ringRad, 0, ringWid2).RotateZ(ang);
			fpxarcT = t.Project(arcptT);
			p.drawLine(QPointF(prevpxT.x, prevpxT.y), QPointF(fpxarcT.x, fpxarcT.y));
			prevpxT = fpxarcT;
		}
	}
	else
	{
		// Both rings are distinct.
		for(int seg=1; seg<=segs; seg++)
		{
			if(seg < segs)
				ang = (PIf*2) * (float)seg / (float)segs;
			else
				ang = 0;
			arcptT = sunLoc + AuPoint(ringRad, 0, ringWid2).RotateZ(ang);
			arcptB = sunLoc + AuPoint(ringRad, 0, -ringWid2).RotateZ(ang);
			fpxarcT = t.Project(arcptT);
			fpxarcB = t.Project(arcptB);
			p.drawLine(QPointF(prevpxT.x, prevpxT.y), QPointF(fpxarcT.x, fpxarcT.y));
			p.drawLine(QPointF(prevpxB.x, prevpxB.y), QPointF(fpxarcB.x, fpxarcB.y));
			p.drawLine(QPointF(fpxarcT.x, fpxarcT.y), QPointF(fpxarcB.x, fpxarcB.y));
			prevpxT = fpxarcT;
			prevpxB = fpxarcB;
		}
	}
}

quint64 AuBlipRingSection::MemoryUsage() const
{
	quint64 mem = AuBlipWorld::MemoryUsage();
	mem += sizeof(m_SectionIndex);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipRingSection& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipMissile::AuBlipMissile(COMMODITYID cid, quint8 quality, bool seekingScanner, quint8 count,
		quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	Q_ASSERT(quality);
	m_CID = cid;
	m_Count = count;
	m_SeekingScanner = seekingScanner;
	m_Quality = quality;
}

AuBlipMissile::AuBlipMissile()
{
	m_CID = CID_Invalid;
	m_Count = 0;
	m_SeekingScanner = false;
	m_Quality = 1;
}

AuBlipMissile::AuBlipMissile(const AuBlipMissile& c) : AuBlipEmpire(c)
{
	m_CID = c.m_CID;
	m_Count = c.m_Count;
	m_SeekingScanner = c.m_SeekingScanner;
	m_Quality = c.m_Quality;
}

AuSensorBlip* AuBlipMissile::Clone() const
{
	return new AuBlipMissile(*this);
}

// Name used next to selection reticle.
QString AuBlipMissile::ScreenName() const
{
	if(m_Count > 1)
		return QObject::tr("%1 Q%2 x %3")
			.arg(g_Au.CommodityName(CommodityId()))
			.arg(Quality())
			.arg(m_Count);
	else
		return QObject::tr("%1 Q%2")
			.arg(g_Au.CommodityName(CommodityId()))
			.arg(Quality());
}

QStringList AuBlipMissile::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << ScreenName();

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);

	if(m_Count > 1)
		infoLines << QObject::tr("Cluster of %1").arg(m_Count);

	switch(m_SeekingScanner)
	{
	case 0:
		break;
	case 1:
		infoLines << QObject::tr("Seeking Target");
		break;
	case 2:
		infoLines << QObject::tr("LOCKED ON US");
		break;
	case 3:
		infoLines << QObject::tr("Locked On Target");
		break;
	}
	return infoLines;
}

void AuBlipMissile::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);
	out << m_CID;
	out << m_Count;
	out << m_SeekingScanner;
	out << m_Quality;
}

void AuBlipMissile::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);
	in >> m_CID;
	in >> m_Count;
	in >> m_SeekingScanner;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
}

const char* AuBlipMissile::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipMissileFriend.png",
		":data/BlipMissileNeutral.png",
		":data/BlipMissileEnemy.png",
		":data/BlipMissileEmpire.png",
		":data/BlipMissileEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

quint64 AuBlipMissile::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += sizeof(m_CID);
	mem += sizeof(m_Count);
	mem += sizeof(m_SeekingScanner);
	mem += sizeof(m_Quality);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipMissile& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.CommodityId() << blip.Quality() << blip.IsSeekingScanner() << blip.Count()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBeacon::AuBlipBeacon(const QString& message, const AuId& id, const AuPoint& sysLoc) :
	AuSensorBlip(id, sysLoc)
{
	m_Message = message;
}

AuBlipBeacon::AuBlipBeacon()
{
}

AuBlipBeacon::AuBlipBeacon(const AuBlipBeacon& c) : AuSensorBlip(c)
{
	m_Message = c.m_Message;
}

AuSensorBlip* AuBlipBeacon::Clone() const
{
	return new AuBlipBeacon(*this);
}

// Name used next to selection reticle.
QString AuBlipBeacon::ScreenName() const
{
	return QObject::tr("Beacon");
}

QStringList AuBlipBeacon::InfoLines(const DaAtom* system) const
{
	Q_UNUSED(system);
	QStringList infoLines;
	infoLines << ScreenName();

	if(!m_Message.isEmpty())
		infoLines << QObject::tr("Broadcast: %1").arg(m_Message);

	return infoLines;
}

void AuBlipBeacon::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);
	out << m_Message;
}

void AuBlipBeacon::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);
	in >> m_Message;
}

const char* AuBlipBeacon::BlipImage() const
{
	return ":data/BlipBeacon.png";
}

quint64 AuBlipBeacon::MemoryUsage() const
{
	quint64 mem = AuSensorBlip::MemoryUsage();
	mem += StringMemoryUsage(m_Message);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipBeacon& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.Message();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipAnomaly::AuBlipAnomaly(const AuId& id, const AuPoint& sysLoc) : AuSensorBlip(id, sysLoc)
{
}

AuBlipAnomaly::AuBlipAnomaly()
{
}

AuBlipAnomaly::AuBlipAnomaly(const AuBlipAnomaly& c) : AuSensorBlip(c)
{
}

AuSensorBlip* AuBlipAnomaly::Clone() const
{
	return new AuBlipAnomaly(*this);
}

// Name used next to selection reticle.
QString AuBlipAnomaly::ScreenName() const
{
	return QObject::tr("Anomaly");
}

QStringList AuBlipAnomaly::InfoLines(const DaAtom* system) const
{
	Q_UNUSED(system);
	QStringList infoLines;
	infoLines << ScreenName();

	return infoLines;
}

void AuBlipAnomaly::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);
}

void AuBlipAnomaly::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);
}

const char* AuBlipAnomaly::BlipImage() const
{
	return ":data/BlipAnomaly.png";
}

QDebug operator<<(QDebug dbg, const AuBlipAnomaly& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipTransporterBd::AuBlipTransporterBd(const QString& bdName, bool isMilitary, quint16 roomNumber, quint8 pads, quint32 empire,
		POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	m_BdName = bdName;
	m_IsMilitary = isMilitary;
	m_Pads = pads;
	m_RoomNumber = roomNumber;
}

AuBlipTransporterBd::AuBlipTransporterBd()
{
	m_IsMilitary = 0;
	m_Pads = 0;
	m_RoomNumber = 0;
}

AuBlipTransporterBd::AuBlipTransporterBd(const AuBlipTransporterBd& c) : AuBlipEmpire(c)
{
	m_BdName = c.m_BdName;
	m_IsMilitary = c.m_IsMilitary;
	m_Pads = c.m_Pads;
	m_RoomNumber = c.m_RoomNumber;
}

AuSensorBlip* AuBlipTransporterBd::Clone() const
{
	return new AuBlipTransporterBd(*this);
}

void AuBlipTransporterBd::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);

	out << m_BdName;
	out << m_IsMilitary;
	out << m_Pads;
	out << m_RoomNumber;
}

void AuBlipTransporterBd::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);

	in >> m_BdName;
	in >> m_IsMilitary;
	in >> m_Pads;
	in >> m_RoomNumber;
}

// Name used next to selection reticle.
QString AuBlipTransporterBd::ScreenName() const
{
	return QObject::tr("%1 TR %2").arg(BdName()).arg(RoomNumber());
}

QStringList AuBlipTransporterBd::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << BdName();
	infoLines << QObject::tr("Transporter Room %1").arg(RoomNumber());

	// Get empire name from base class.
	infoLines << AuBlipEmpire::InfoLines(system);
	return infoLines;
}

const char* AuBlipTransporterBd::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipTransporterCityFriend.png",
		":data/BlipTransporterCityNeutral.png",
		":data/BlipTransporterCityEnemy.png",
		":data/BlipTransporterCityEmpire.png",
		":data/BlipTransporterCityEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

quint64 AuBlipTransporterBd::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += StringMemoryUsage(m_BdName);
	mem += sizeof(m_IsMilitary);
	mem += sizeof(m_Pads);
	mem += sizeof(m_RoomNumber);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipTransporterBd& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId()
		<< blip.BdName() << (blip.IsCivilian()?"Civilian":"Military") << blip.RoomNumber() << "pads" << blip.Pads()
		<< blip.EmpireId() << blip.Stance()
		<< blip.EnvLocation();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBd::AuBlipBd(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
		quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc) :
	AuBlipEmpire(empire, stance, id, sysLoc)
{
	Q_ASSERT(quality);
	m_CurHits = curHits;
	m_Industry = iid;
	m_IsLockedOn = isLockedOn;
	m_MaxHits = maxHits;
	m_Name = name;
	m_RoughRadius = roughRadius;
	m_Quality = quality;
	m_Volume = volume;
}

AuBlipBd::AuBlipBd()
{
	m_CurHits = 0;
	m_Industry = IID_None;
	m_IsLockedOn = false;
	m_MaxHits = 0;
	m_RoughRadius = 0;
	m_Quality = 1;
	m_Volume = 0;
}

AuBlipBd::AuBlipBd(const AuBlipBd& c) : AuBlipEmpire(c)
{
	m_CurHits = c.m_CurHits;
	m_Industry = c.m_Industry;
	m_IsLockedOn = c.m_IsLockedOn;
	m_MaxHits = c.m_MaxHits;
	m_Name = c.m_Name;
	m_RoughRadius = c.m_RoughRadius;
	m_Quality = c.m_Quality;
	m_Volume = c.m_Volume;
}

void AuBlipBd::TxMin(QDataStream& out) const
{
	AuBlipEmpire::TxMin(out);
	out << m_Industry;
}

void AuBlipBd::RxMin(QDataStream& in)
{
	AuBlipEmpire::RxMin(in);
	in >> m_Industry;
}

void AuBlipBd::operator>>(QDataStream& out) const
{
	AuBlipEmpire::operator>>(out);

	quint8 bits = 0;
	if(m_IsLockedOn)
		bits |= 0x01;
	if(!m_Name.isEmpty())
		bits |= 0x02;

	out << bits;
	out << m_CurHits;
	out << m_Industry;
	out << m_MaxHits;
	if(bits & 0x02)
		out << m_Name;
	out << m_RoughRadius;
	out << m_Quality;
	out << m_Volume;
}

void AuBlipBd::operator<<(QDataStream& in)
{
	AuBlipEmpire::operator<<(in);

	quint8 bits;
	in >> bits;
	m_IsLockedOn = bits & 0x01;

	in >> m_CurHits;
	in >> m_Industry;
	in >> m_MaxHits;
	if(bits & 0x02)
		in >> m_Name;
	in >> m_RoughRadius;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_Volume;
}

// Name used next to selection reticle.
QString AuBlipBd::ScreenName() const
{
	if(!Name().isEmpty())
		return QObject::tr("%1 Q%2").arg(Name()).arg(Quality());

	const DgData& gd = g_Au.LockDataForRead();
	QString iname = gd.IndustryName(IndustryId());
	g_Au.UnlockData();
	return QObject::tr("%1 Q%2").arg(iname).arg(Quality());
}

float AuBlipBd::ObstacleRadius() const
{
	return RoughRadius();
}

float AuBlipBd::PilotAvoidRadius() const
{
	return ObstacleRadius() + 50.f * FeetPerMeter();
}

quint64 AuBlipBd::MemoryUsage() const
{
	quint64 mem = AuBlipEmpire::MemoryUsage();
	mem += sizeof(m_CurHits);
	mem += sizeof(m_Industry);
	mem += sizeof(m_IsLockedOn);
	mem += sizeof(m_MaxHits);
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_RoughRadius);
	mem += sizeof(m_Quality);
	mem += sizeof(m_Volume);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdBase::AuBlipBdBase(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
		quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdBase::AuBlipBdBase()
{
}

AuBlipBdBase::AuBlipBdBase(const AuBlipBdBase& c) : AuBlipBd(c)
{
}

const char* AuBlipBdBase::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipBaseFriend.png",
		":data/BlipBaseNeutral.png",
		":data/BlipBaseEnemy.png",
		":data/BlipBaseEmpire.png",
		":data/BlipBaseEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdBase::Clone() const
{
	return new AuBlipBdBase(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdBase::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdBaseHQ::AuBlipBdBaseHQ(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius,
		quint32 volume, quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id,
		const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdBaseHQ::AuBlipBdBaseHQ()
{
}

AuBlipBdBaseHQ::AuBlipBdBaseHQ(const AuBlipBdBaseHQ& c) : AuBlipBd(c)
{
}

const char* AuBlipBdBaseHQ::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipHQFriend.png",
		":data/BlipHQNeutral.png",
		":data/BlipHQEnemy.png",
		":data/BlipHQEmpire.png",
		":data/BlipHQEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdBaseHQ::Clone() const
{
	return new AuBlipBdBaseHQ(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdBaseHQ::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdCapitolCity::AuBlipBdCapitolCity(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn,
		float roughRadius, quint32 volume, quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdCapitolCity::AuBlipBdCapitolCity()
{
}

AuBlipBdCapitolCity::AuBlipBdCapitolCity(const AuBlipBdCapitolCity& c) : AuBlipBd(c)
{
}

const char* AuBlipBdCapitolCity::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCapitolFriend.png",
		":data/BlipCapitolNeutral.png",
		":data/BlipCapitolEnemy.png",
		":data/BlipCapitolEmpire.png",
		":data/BlipCapitolEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdCapitolCity::Clone() const
{
	return new AuBlipBdCapitolCity(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdCapitolCity::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdCapitolEmpire::AuBlipBdCapitolEmpire(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn,
		float roughRadius, quint32 volume, quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdCapitolEmpire::AuBlipBdCapitolEmpire()
{
}

AuBlipBdCapitolEmpire::AuBlipBdCapitolEmpire(const AuBlipBdCapitolEmpire& c) : AuBlipBd(c)
{
}

const char* AuBlipBdCapitolEmpire::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCapitolCityFriend.png",
		":data/BlipCapitolCityNeutral.png",
		":data/BlipCapitolCityEnemy.png",
		":data/BlipCapitolCityEmpire.png",
		":data/BlipCapitolCityEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdCapitolEmpire::Clone() const
{
	return new AuBlipBdCapitolEmpire(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdCapitolEmpire::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdCapitolGalaxy::AuBlipBdCapitolGalaxy(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn,
		float roughRadius, quint32 volume, quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdCapitolGalaxy::AuBlipBdCapitolGalaxy()
{
}

AuBlipBdCapitolGalaxy::AuBlipBdCapitolGalaxy(const AuBlipBdCapitolGalaxy& c) : AuBlipBd(c)
{
}

const char* AuBlipBdCapitolGalaxy::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipGalaxyCapitolFriend.png",
		":data/BlipGalaxyCapitolNeutral.png",
		":data/BlipGalaxyCapitolEnemy.png",
		":data/BlipGalaxyCapitolEmpire.png",
		":data/BlipGalaxyCapitolEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdCapitolGalaxy::Clone() const
{
	return new AuBlipBdCapitolGalaxy(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdCapitolGalaxy::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdCapitolSector::AuBlipBdCapitolSector(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn,
		float roughRadius, quint32 volume, quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance,
		const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdCapitolSector::AuBlipBdCapitolSector()
{
}

AuBlipBdCapitolSector::AuBlipBdCapitolSector(const AuBlipBdCapitolSector& c) : AuBlipBd(c)
{
}

const char* AuBlipBdCapitolSector::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipSectorCapitolFriend.png",
		":data/BlipSectorCapitolNeutral.png",
		":data/BlipSectorCapitolEnemy.png",
		":data/BlipSectorCapitolEmpire.png",
		":data/BlipSectorCapitolEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdCapitolSector::Clone() const
{
	return new AuBlipBdCapitolSector(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdCapitolSector::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipBdCity::AuBlipBdCity(const QString& name, INDUSTRYID iid, quint8 quality, bool isLockedOn, float roughRadius, quint32 volume,
		quint32 maxHits, quint32 curHits, quint32 empire, POLITICAL_STANCE stance, const AuId& id, const AuPoint& sysLoc) :
	AuBlipBd(name, iid, quality, isLockedOn, roughRadius, volume, maxHits, curHits, empire, stance, id, sysLoc)
{
}

AuBlipBdCity::AuBlipBdCity()
{
}

AuBlipBdCity::AuBlipBdCity(const AuBlipBdCity& c) : AuBlipBd(c)
{
}

const char* AuBlipBdCity::BlipImage() const
{
	const char* blipImages[] =
	{
		":data/BlipCityFriend.png",
		":data/BlipCityNeutral.png",
		":data/BlipCityEnemy.png",
		":data/BlipCityEmpire.png",
		":data/BlipCityEmpire.png",
	};
	int blipImagesCount = sizeof(blipImages)/sizeof(const char*);
	if(Stance() < 0 || Stance() >= blipImagesCount)
		return "";
	return blipImages[Stance()];
}

AuSensorBlip* AuBlipBdCity::Clone() const
{
	return new AuBlipBdCity(*this);
}

// If 'system' is 0, returns only the short one-line name for planet blips.
QStringList AuBlipBdCity::InfoLines(const DaAtom* system) const
{
	QStringList infoLines;
	infoLines << TypeName();
	infoLines << ScreenName();
	infoLines << QObject::tr("%1m%2").arg(Volume()).arg(g_Au.CubedSymbol());

	// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
	//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
	//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
	if(CurHits() == MaxHits())
		infoLines << QObject::tr("%L1 hp").arg(CurHits());
	else
		infoLines << QObject::tr("%L1/%L2 hp").arg(CurHits()).arg(MaxHits());

	// Get empire name from base class.
	infoLines << AuBlipBd::InfoLines(system);
	return infoLines;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipAsteroid::AuBlipAsteroid(quint8 size, quint16 hits, const AuId& id, const AuPoint& sysLoc) :
	AuSensorBlip(id, sysLoc)
{
	m_Hits = hits;
	m_Size = size;
}

AuBlipAsteroid::AuBlipAsteroid()
{
	m_Hits = 0;
	m_Size = 0;
}

AuBlipAsteroid::AuBlipAsteroid(const AuBlipAsteroid& c) : AuSensorBlip(c)
{
	m_Hits = c.m_Hits;
	m_Size = c.m_Size;
}

AuSensorBlip* AuBlipAsteroid::Clone() const
{
	return new AuBlipAsteroid(*this);
}

// Name used next to selection reticle.
QString AuBlipAsteroid::ScreenName() const
{
	return QObject::tr("Asteroid %1").arg(m_AtomId.StringId());
}

QStringList AuBlipAsteroid::InfoLines(const DaAtom* system) const
{
	Q_UNUSED(system);
	QStringList infoLines;
	infoLines << ScreenName();

	double dia = DaAsteroid::ObstacleRadius(m_Size) * 2 / FeetPerMeter();	// diameter in meters
	dia = round(dia/10) * 10;		// Round to nearest 10 meters.
	infoLines << QObject::tr("%L1m Diameter").arg(dia, 0,'f',0);

	infoLines << QObject::tr("%L1 hp").arg(m_Hits);

	return infoLines;
}

void AuBlipAsteroid::operator>>(QDataStream& out) const
{
	AuSensorBlip::operator>>(out);
	out << m_Hits;
	out << m_Size;
}

void AuBlipAsteroid::operator<<(QDataStream& in)
{
	AuSensorBlip::operator<<(in);
	in >> m_Hits;
	in >> m_Size;
}

const char* AuBlipAsteroid::BlipImage() const
{
	return ":data/BlipAsteroid.png";
}

quint64 AuBlipAsteroid::MemoryUsage() const
{
	quint64 mem = AuSensorBlip::MemoryUsage();
	mem += sizeof(m_Hits);
	mem += sizeof(m_Size);
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipAsteroid& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << "roidSize" << blip.AsteroidSize() << "hp" << blip.Hits();
	return dbg.space();
}

float AuBlipAsteroid::ObstacleRadius() const
{
	return DaAsteroid::ObstacleRadius(m_Size);
}

float AuBlipAsteroid::PilotAvoidRadius() const
{
	return DaAsteroid::ObstacleRadius(m_Size) + 1000*FeetPerMeter();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBlipPlanetoid::AuBlipPlanetoid(quint8 radius100km, bool inSensorRange, const AuId& id,
		const AuPoint& sysLoc) : AuBlipGlobe(radius100km, inSensorRange, id, sysLoc)
{
}

AuBlipPlanetoid::AuBlipPlanetoid()
{
}

AuBlipPlanetoid::AuBlipPlanetoid(const AuBlipPlanetoid& c) : AuBlipGlobe(c)
{
}

AuSensorBlip* AuBlipPlanetoid::Clone() const
{
	return new AuBlipPlanetoid(*this);
}

void AuBlipPlanetoid::operator>>(QDataStream& out) const
{
	AuBlipGlobe::operator>>(out);
}

void AuBlipPlanetoid::operator<<(QDataStream& in)
{
	AuBlipGlobe::operator<<(in);
}

// Name used next to selection reticle.
QString AuBlipPlanetoid::ScreenName() const
{
	return BodyTypeName(BODY_Planetoid);
}

quint64 AuBlipPlanetoid::MemoryUsage() const
{
	quint64 mem = AuBlipGlobe::MemoryUsage();
	return mem;
}

QDebug operator<<(QDebug dbg, const AuBlipPlanetoid& blip)
{
	dbg.nospace() << blip.Type() << blip.TypeName() << blip.AtomId() << blip.EnvLocation();
	return dbg.space();
}

void AuBlipPlanetoid::DrawBlip(QPainter& p, const AuMatrix4x4& t, double feetPerPixel, const DaAtom* system) const
{
	const DaWorldGlobe* planet = dynamic_cast<const DaWorldGlobe*>(system->FindIdRecursive(AtomId()));
	if(!planet)
		return;

	AuPoint fpx = t.Project(EnvLocation());
	QPoint px(round(fpx.x),round(fpx.y));

	// Draw a circle at the size of the main rock, flat to the view.
	p.setPen(QPen(planet->AtmosphereColor(), 0.f));

	double rad = planet->SeaLevelRadius();						// Radius in feet.
	double radpix = rad / feetPerPixel;							// Radius in pixels.
	int lrad = round(radpix);
	QRect rc(-lrad, -lrad, lrad*2+1, lrad*2+1);
	if(lrad == 0)
	{
		rc.setRight(rc.right()+1);
		rc.setBottom(rc.bottom()+1);
	}
	rc.translate(px.x(), px.y());
	p.drawArc(rc, 0, 5760);

	// Draw a circle of dots around the parent star in the plane of the solar system.
	p.setPen(QPen(QColor(128,128,128), 0.f));

	double ringRad = planet->Location().Length();				// Radius of the ring in feet.
	double cpx = (ringRad*2 / feetPerPixel) * PI;				// circumference in pixels
	int segs = cpx / 8;											// 8 pixels per segment
	if(segs < 5)
		segs = 5;
	else if(segs > 100)
		segs = 100;
	AuPoint starLoc = planet->Parent()->EnvLocation();
	float ang;
	AuPoint arcpt, fpxarc;
	for(int seg=0; seg<segs; seg++)
	{
		ang = (PIf*2) * (float)seg / (float)segs;
		arcpt = starLoc + AuPoint(ringRad, 0).RotateZ(ang);
		fpxarc = t.Project(arcpt);
		p.drawPoint(QPointF(fpxarc.x, fpxarc.y));
	}
}

float AuBlipPlanetoid::PilotSpeedLimitRange() const
{
	// Planet spacing values taken from DaWorldGlobe::CheckChildExit.
	return FeetPerMoonOrbit();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

