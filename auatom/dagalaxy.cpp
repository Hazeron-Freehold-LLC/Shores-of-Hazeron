//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dagalaxy.cpp,v 1.215 2022/05/17 16:05:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dagalaxy.h"
//
#include <audice.h>
#include <auglobal.h>
#include "daatomlist.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include <dgdata.h>
#include <qgl.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <QSqlError>
#include <qsqlquery.h>
#include <QVariant>
#include "augleffects.h"

class AuGalaxy
{
public:
	QColor								m_Color;
	int									m_CorePx;				// Width of the core in pixels.
	int									m_DiameterPx;			// Width of the galaxy in pixels.
	QString								m_ImageName;

public:
	inline AuGalaxy(const QString& imageName, int diameter, int core, const QColor& color)
	{
		m_ImageName = imageName;
		m_DiameterPx = diameter;
		m_CorePx = core;
		m_Color = color;
	}

	// Color used for skybox painting of this galaxy.
	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline int CoreDiameterPx() const
	{
		return m_CorePx;
	}

	// The radius is scaled by the ratio of the galaxy image width to the original ShoresOfHazeron.png width, which is 1488 pixels.
	// That way the size of the image determines the size of the galaxy, relative to that original image.
	inline double CoreRadius() const
	{
		return 2500. * m_CorePx / 1488.;
	}

	// Returns the radius of this galaxy in parsecs.
	// The radius is scaled by the ratio of the galaxy image width to the original ShoresOfHazeron.png width, which is 1488 pixels.
	// That way the size of the image determines the size of the galaxy, relative to that original image.
	inline double RadiusPc() const
	{
		return 2500. * m_DiameterPx / 1488.;
	}
};

// The order of this static data matches the GALAXY enum.
// The calculation in the radius column scales the radius by the the ratio of the galaxy image width to the original
// ShoresOfHazeron.png width, which is 1488 pixels.
static AuGalaxy s_Galaxy[] =
{
	AuGalaxy(":data/AndromedaRising.png", 829, 85, QColor(6, 5, 8)),
	AuGalaxy(":data/BlackHole.png", 350, 40, QColor(8, 5, 4)),
	AuGalaxy(":data/Core.png", 747, 85, QColor(3, 5, 8)),
	AuGalaxy(":data/CrownOfOthon.png", 1658, 120, QColor(8, 6, 4)),
	AuGalaxy(":data/DyrathonsRetreat.png", 640, 90, QColor(5, 7, 8)),
	AuGalaxy(":data/EdgeOfTheRift.png", 629, 40, QColor(8, 4, 3)),
	AuGalaxy(":data/FallasEmbrace.png", 834, 80, QColor(5, 5, 7)),
	AuGalaxy(":data/FallenLegionsOfMuturon.png", 423, 50, QColor(5, 8, 7)),
	AuGalaxy(":data/HeartOfVictorus.png", 1386, 150, QColor(4, 5, 8)),
	AuGalaxy(":data/HouseZanathar.png", 633, 50, QColor(1, 2, 8)),
	AuGalaxy(":data/IndigoSea.png", 580, 80, QColor(8, 5, 7)),
	AuGalaxy(":data/InkarBorderRegion.png", 948, 50, QColor(2, 4, 8)),
	AuGalaxy(":data/MuturonEncounter.png", 595, 50, QColor(4, 8, 7)),
	AuGalaxy(":data/RansuulsFlamingSword.png", 365, 28, QColor(8, 6, 3)),
	AuGalaxy(":data/SevenTen.png", 788, 50, QColor(2, 8, 8)),
	AuGalaxy(":data/ShoresOfHazeron.png", 1488, 150, QColor(4, 4, 10)),
	AuGalaxy(":data/ThustrasEye.png", 1226, 130, QColor(10, 7, 2)),
	AuGalaxy(":data/VeilOfTargoss.png", 894, 100, QColor(5, 6, 8)),
	AuGalaxy(":data/VreenoxEclipse.png", 453, 30, QColor(8, 6, 7)),
	AuGalaxy(":data/VulcansForge.png", 871, 60, QColor(13, 4, 2)),
	AuGalaxy(":data/MilkyWay.png", 1041, 110, QColor(6, 6, 6))
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaGalaxy::DaGalaxy()
{
	m_Galaxy = GALAXY_Count;
}

DaGalaxy::DaGalaxy(GALAXY galaxy)
{
	m_Galaxy = galaxy;
}

DaGalaxy::~DaGalaxy()
{
}

DaGalaxy::DaGalaxy(const DaGalaxy &rhs) : DaAtom(rhs)
{
	m_Galaxy = rhs.m_Galaxy;

	rhs.m_GalaxyLock.lockForRead();
	m_AssignableSectorIds = rhs.m_AssignableSectorIds;
	m_SectorLocs = rhs.m_SectorLocs;
	rhs.m_GalaxyLock.unlock();
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaGalaxy::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	toClient << (quint8)m_Galaxy;
}

// Receive updates to this atom from a server.
void DaGalaxy::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	quint8 ui8;
	fromServer >> ui8;
	m_Galaxy = (GALAXY)ui8;
}

// Refresh data from valid counterpart.
bool DaGalaxy::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaGalaxy* pA = (DaGalaxy*)pAtom;
	m_Galaxy = pA->m_Galaxy;
	return DaAtom::Refresh(pAtom, ari);
}

DaAtom *DaGalaxy::Clone() const
{
	return new DaGalaxy(*this);
}

void DaGalaxy::Create()
{
	if(m_Galaxy != GALAXY_Count)
		DaAtom::Create(AuId::GalaxyId(m_Galaxy));
	else
		DaAtom::Create();
}

void DaGalaxy::InitPos(const AuId& universeId)
{
	// Universe map
	AuPoint loc;
	if(m_Galaxy == MilkyWay)
	{
		loc = AuPoint(-1,0,10);
	}
	else
	{
		// Galaxies form a twisted ladder, like a DNA strand. The table visually lays them out, untwisted.
		// EdgeOfTheRift is lowest in Z and ThustrasEye is the highest in Z.
		GALAXY umap[GALAXY_Count] =
		{
										ThustrasEye,
			InkarBorderRegion,			RansuulsFlamingSword,
			VulcansForge,				CrownOfOthon,			
			HeartOfVictorus,			HouseZanathar,
			SevenTen,					DyrathonsRetreat,
			FallenLegionsOfMuturon,		IndigoSea,
			BlackHole,					Core,
			MuturonEncounter,			VreenoxEclipse,
			AndromedaRising,			FallasEmbrace,
			ShoresOfHazeron,			VeilOfTargoss,
			EdgeOfTheRift,
		};

		// Locate my index in the map.
		int umapIdx = -1;
		for(int umapi=0; umapi<GALAXY_Count; umapi++)
			if(umap[umapi] == m_Galaxy)
			{
				umapIdx = umapi;
				break;
			}

		// Calculate the location of this galaxy in the universe.
		int rung = 10 - (umapIdx + 1) / 2;							// top rung is 10, bottom rung is 0
		double side = (umapIdx & 0x01) ? -1 : 1;					// left side is -1, right side is 1
		double rot = rung * 360. / (GALAXY_Count/2) * DEG2RAD;
		loc = AuPoint(side, 0, rung).RotateZ(rot);
	}

	// Set my transformation matrix if it is not correct.
	AuMatrix4x4 m;
	m.Translate(loc);
	if(Ether())
		ChangePos(AuAtomPos(universeId, m));		// Change my position in existing ether.
	else
		SetPosAndOrient(AuAtomPos(universeId, m));	// Set my position in preparation for ether.
}

// Returns the radius of the galaxy in parsecs. There are 10 parsecs per sector.
//static
double DaGalaxy::Radius(GALAXY galaxy)
{
	Q_ASSERT(galaxy < GALAXY_Count);
	return s_Galaxy[galaxy].RadiusPc();
}

double DaGalaxy::CoreRadius(GALAXY galaxy)
{
	Q_ASSERT(galaxy < GALAXY_Count);
	return s_Galaxy[galaxy].CoreRadius();
}

DaSector* DaGalaxy::CreateJoinEmpireHomeWorldSector(const AuId& joinSectorId, AuIdList& idNewSectors)
{
	if(g_Au.m_LogMessages & LOG_AtomCreate)
	{
		QString logmsg = QString("DaGalaxy::CreateJoinEmpireHomeWorldSector joinSector %1").arg(joinSectorId.GetID());
		g_Au.Log(logmsg);
	}

	// Find a location near the join sector.
	const DaSector* joinSector = dynamic_cast<const DaSector*>(FindSubatomId(joinSectorId));
	if(!joinSector)
		return CreateNewHomeWorldSector(8, idNewSectors);	// Sector to join not found so start anywhere.

	AuPoint sectorLoc;
	double radBase = Radius() * 5.4/8.0;					// Radius to start pt, in pc.
	radBase = (int)(radBase/10.0);							// Convert to sectors.
	radBase *= 10;											// Sector coordinate, in pc, to whole 10's.
	radBase = FROUND(radBase, 10.);
	int directions = 20;	// Try up to 20 different directions.
	do
	{
		// Start at center of the join sector.
		AuPoint sectorLoc = joinSector->Location();

		// Starting at a sector location near sectorLoc, walk in a straight line until we find a sector location that's available.
		// Sector locations are 10 parsecs apart.
		// Calculate a random walk vector to step in one-sector increments away from the start point.
		double dX = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		double dY = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		double dZ = 0;	//(3.0 - g_Au.Dice().Roll(1,5)) * 10.0;			// Walk parallel to the XY plane.
		AuPoint walkVec(dX, dY, dZ);

		// Walk up to 50 sectors in the walkVec direction to find a good sector with at least 10 systems, one of them unexplored.
		int steps = 50 + (40 - directions*2);	// Go farther and farther as we try more directions.
		do
		{
			const DaSector* sector = FindSectorAt(sectorLoc);

			// Check to see if the sector location is ok based on the proximity desired.
			bool sectorLocOk = false;
			if(!sector)
				sectorLocOk = true;
			else if(sector->HasAvailableHomeWorldSystem())
				sectorLocOk = true;

			if(!sectorLocOk)
			{
				// Skip the sector.
				sectorLoc += walkVec;
				if(g_Au.Dice().Roll(1,100) <= 10)
				{
					// Change direction every now and then.
					dX = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
					dY = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
					//dZ = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
				}
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_AtomCreate)
				{
					QString logmsg = QString("DaGalaxy::CreateJoinEmpireHomeWorldSector %1 %2 creating sector at (%3, %4, %5)")
						.arg(Id().GetID())
						.arg(Name())
						.arg(sectorLoc.x).arg(sectorLoc.y).arg(sectorLoc.z);
					g_Au.Log(logmsg);
				}
				DaSector* sector = CreateSectorCubeAt(sectorLoc, idNewSectors);
				if(sector && sector->HasAvailableHomeWorldSystem())
				{
					// When creating sector cubes, return the first one that works, regardless of proximity requirement.
					return sector;
				}
			}
		}
		while(steps--);
	}
	while(directions--);

	// Try again using remote proximity. That will try repeatedly until it succeeds.
	return CreateNewHomeWorldSector(8, idNewSectors);
}

// Creates a nu sector in the galaxy.
// proximity: 0=Eastern Cluster
//            1=Western Cluster
//            2=Northern Cluster
//            3=Southern Cluster
//            4=Eastern Frontier, in an unnamed sector next to a named sector near the eastern cluster
//            5=Western Frontier, in an unnamed sector next to a named sector near the western cluster
//            6=Northern Frontier, in an unnamed sector next to a named sector near the northern cluster
//            7=Southern Frontier, in an unnamed sector next to a named sector near the southern cluster
//            8=Remote, in an unnamed sector not next to a named sector
DaSector* DaGalaxy::CreateNewHomeWorldSector(int proximity, AuIdList& idNewSectors)
{
	if(g_Au.m_LogMessages & LOG_AtomCreate)
	{
		QString logmsg = QString("DaGalaxy::CreateNewHomeWorldSector proximity %1").arg(proximity);
		g_Au.Log(logmsg);
	}

	if(proximity >= 8)
	{
		// Find a random location, in an unnamed sector not next to a named sector
		int radSectors = (int)(s_Galaxy[m_Galaxy].RadiusPc()/10);	// radius of the galaxy, in sectors.
		int coreRadSectors = (int)(s_Galaxy[m_Galaxy].CoreRadius()/10) / 2;	// 1/2 the radius of the galactic core, in sectors.
		int tries = 100;
		do
		{
			// Create a random sector location in the galaxy.
			int radSector = g_Au.Dice().Roll(1, radSectors);
			float angle = (float)g_Au.Dice().Roll(1, 360000) / 1000.f;	// Degrees around Z, down to .001 degrees
			AuPoint sectorLoc = AuPoint(radSector, 0).RotateZ(angle*DEG2RADf);

			// Thickness of galaxy is linearly interpolated from 1/2 the thickness of the core out to 0 at the radius.
			float radfrac = 1.f - (float)radSector / (float)radSectors;	// Fractional distance from center to radSector, 1..0.
			int thickSectors = (int)(radfrac * coreRadSectors);
			if(thickSectors)
				sectorLoc.z = g_Au.Dice().Roll(1, thickSectors*2+1, -(thickSectors+1));

			// Convert sectorLoc from sectors to pc.
			sectorLoc *= 10;
			sectorLoc.Round(10);

			// Check star density at that sector loc.
			double starDensity = StarDensity(sectorLoc);
			int stars = (int)(starDensity * 50.0);		// Maximum is 50 stars in a sector.
			if(stars < 10)
				continue;	// Home sector must have at least 10 solar systems, to guarantee at least one is a home world system.

			const DaSector* sector = FindSectorAt(sectorLoc);
			if(sector && (sector->HasName() || !sector->HasAvailableHomeWorldSystem()))
				continue;	// Sector exists. It is named or it has no available home world systems.

			// Check for neighbor sectors.
			if(!HasNamedNeighborSector(sectorLoc))
			{
				// Randomly generated sector location has no named neighboring sectors.
				if(g_Au.m_LogMessages & LOG_AtomCreate)
				{
					QString logmsg = QString("DaGalaxy::CreateNewHomeWorldSector %1 %2 remote sector at %3, %4, %5")
						.arg(Id().GetID())
						.arg(Name())
						.arg(sectorLoc.x)
						.arg(sectorLoc.y)
						.arg(sectorLoc.z);
					g_Au.Log(logmsg);
				}
				return CreateSectorCubeAt(sectorLoc, idNewSectors);
			}
		}
		while(tries--);

		// We didn't find one, this is not likely.
		proximity++;
		if(proximity > 108)
		{
			// Give up. We don't want to be stuck here forever.
			qWarning() << "error: DaGalaxy::CreateNewHomeWorldSector" << Id() << Name() << "proximity" << proximity
				<< idNewSectors.size() << "sectors created so far. Failed to create sector, giving up.";
			return 0;
		}
		else
		{
			// Try again.
			qWarning() << "error: DaGalaxy::CreateNewHomeWorldSector" << Id() << Name() << "proximity" << proximity
				<< idNewSectors.size() << "sectors created so far. Failed to create sector, trying again...";
			return CreateNewHomeWorldSector(proximity, idNewSectors);
		}
	}

	// Find a location in a specific cluster region of the galaxy.
	AuPoint sectorLoc;
	double radBase = Radius() * 5.4/8.0;					// Radius to start pt, in pc.
	radBase = (int)(radBase/10.0);							// Convert to sectors.
	radBase *= 10;											// Sector coordinate, in pc, to whole 10's.
	radBase = FROUND(radBase, 10.);
	int directions = 20;	// Try up to 20 different directions.
	do
	{
		// Start at center of the cluster.
		switch(proximity)
		{
		case 0:	// Eastern Cluster
		case 4:	// Eastern Frontier
			sectorLoc = AuPoint(radBase, 0);
			break;
		case 1:	// Western Cluster
		case 5:	// Western Frontier
			sectorLoc = AuPoint(-radBase, 0);
			break;
		case 2:	// Northern Cluster
		case 6:	// Northern Frontier
			sectorLoc = AuPoint(0, radBase);
			break;
		case 3:	// Southern Cluster
		case 7:	// Southern Frontier
			sectorLoc = AuPoint(0, -radBase);
			break;
		}

		// Starting at a sector location near sectorLoc, walk in a straight line until we find a sector location that's available.
		// Sector locations are 10 parsecs apart.
		double dX = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		double dY = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		double dZ = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		sectorLoc += AuPoint(dX, dY, dZ);						// Initial bump away from start point.

		// Calculate a random walk vector to step in one-sector increments away from the start point.
		dX = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		dY = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
		dZ = 0;	//(3.0 - g_Au.Dice().Roll(1,5)) * 10.0;			// Walk parallel to the XY plane.
		AuPoint walkVec(dX, dY, dZ);

		// Walk up to 50 sectors in the walkVec direction to find a good sector with at least 10 systems, one of them unexplored.
		int steps = 50 + (40 - directions*2);	// Go farther and farther as we try more directions.
		do
		{
			const DaSector* sector = FindSectorAt(sectorLoc);

			// Check to see if the sector location is ok based on the proximity desired.
			bool sectorLocOk = false;
			if(proximity < 4)
			{
				// Looking for a sector in the cluster; close to people is ok.
				// A location with no sector is ok.
				if(!sector)
					sectorLocOk = true;
				else if(sector->SubatomCount() >= 10 && sector->HasAvailableHomeWorldSystem())
					sectorLocOk = true;
			}
			else
			{
				// Looking for a sector on the frontier. Requires an unnamed sector next to a named sector.
				if(sector && !sector->HasName() && sector->SubatomCount() >= 10 && sector->HasAvailableHomeWorldSystem() &&
						HasNamedNeighborSector(sectorLoc))
					sectorLocOk = true;
			}

			if(!sectorLocOk)
			{
				// Skip the sector.
				sectorLoc += walkVec;
				if(g_Au.Dice().Roll(1,100) <= 10)
				{
					// Change direction every now and then.
					dX = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
					dY = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
					//dZ = (3.0 - g_Au.Dice().Roll(1,5)) * 10.0;
				}
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_AtomCreate)
				{
					QString logmsg = QString("DaGalaxy::CreateNewHomeWorldSector %1 %2 creating sector at (%3, %4, %5)")
						.arg(Id().GetID())
						.arg(Name())
						.arg(sectorLoc.x).arg(sectorLoc.y).arg(sectorLoc.z);
					g_Au.Log(logmsg);
				}
				DaSector* sector = CreateSectorCubeAt(sectorLoc, idNewSectors);
				if(sector && sector->SubatomCount() >= 10 && sector->HasAvailableHomeWorldSystem())
				{
					// When creating sector cubes, return the first one that works, regardless of proximity requirement.
					return sector;
				}
			}
		}
		while(steps--);
	}
	while(directions--);

	// If we get here looking for a frontier sector, we can try again using the corresponding cluster proximity; it's less picky.
	if(proximity >= 4 && proximity <= 7)
		return CreateNewHomeWorldSector(proximity-4, idNewSectors);

	// Try again using remote proximity. That will try repeatedly until it succeeds.
	return CreateNewHomeWorldSector(8, idNewSectors);
}

// Creates 3x3x3 sector cube centered at 'fSectorLoc' if any of the sectors don't already exist.
// Insures wormholes are correct for the center sector and surrounding sectors.
// Returns the pointer to the center sector.
DaSector *DaGalaxy::CreateSectorCubeAt(const AuPoint &fSectorLoc, AuIdList& idNewSectors)
{
	if(g_Au.m_LogMessages & LOG_AtomCreate)
	{
		QString logmsg = QString("DaGalaxy::CreateSectorCubeAt (%1, %2, %3)")
			.arg(fSectorLoc.x).arg(fSectorLoc.y).arg(fSectorLoc.z);
		g_Au.Log(logmsg);
	}

	int created = 0;
	DaSector* pCenterSect = 0;
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				AuPoint fn = fSectorLoc + AuPoint(ix, iy, iz);
				DaSector *ps = FindSectorAt(fn);
				if(!ps)
				{
					// The sector doesn't already exist.
					ps = CreateSectorAt(fn);
					if(ps)
					{
						// We created a nu sector.
						idNewSectors.append(ps->Id());
						created++;
					}
				}

				// Remember the center sector for return, whether it was just
				// created or already existed.
				if(ix == 0 && iy == 0 && iz == 0)
					pCenterSect = ps;
			}
		}
	}

	if(g_Au.m_LogMessages & LOG_AtomCreate)
	{
		QString logmsg = QString("DaGalaxy::CreateSectorCubeAt %1 %2 at %3 created %4 nu sectors")
			.arg(pCenterSect->Id().GetID()).arg(pCenterSect->Name()).arg(pCenterSect->Description()).arg(created);
		g_Au.Log(logmsg);
	}

	return pCenterSect;
}

DaSector* DaGalaxy::CreateSectorAt(const AuPoint &fSectorLoc)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	//Q_ASSERT(g_Au.IsSceneMaster());		// Only the scene master creates sectors. Also created in stand-alone designer.
	DaSector* sector = FindSectorAt(fSectorLoc);
	if(sector)
		return sector;					// A sector at that location already exists.

	//qDebug("Creating Sector at: %g, %g, %g", fSectorLoc.x, fSectorLoc.y, fSectorLoc.z);

	AuMatrix4x4 tLoc;
	tLoc.Translate(fSectorLoc);

	// Create the nu sector.
	sector = new DaSector;
	sector->ConnectSignals();
	sector->Create();
	sector->SetPosAndOrient(AuAtomPos(Id(), tLoc));
	ether->AddAtom(sector);

	sector->Init();

	// Update the sector locations and assignable sectors cache.
	AddSectorLoc(sector->Id(), fSectorLoc);
	DaAtomList nulyAssignableSectors;	// Sectors that are no longer perimeter sectors and thus need wormholes finalized.
	DaAtomList perimeterSectors;
	for(int ix=-10; ix<=10; ix+=10)
		for(int iy=-10; iy<=10; iy+=10)
			for(int iz=-10; iz<=10; iz+=10)
				if(ix==0 && iy==0 && iz==0)
				{
					// (0,0,0) is the nu sector we just created.
					// Add the nu sector to the assignable sectors cache if all of its neighbors exist.
					if(!IsPerimeterSector(sector->Location()))
					{
						// All neighbor sectors exist. The sector is assignable.
						// It has just become assignable, we must finalize its wormholes. This process requires all perimeter
						// sectors to exist.
						m_GalaxyLock.lockForWrite();
						m_AssignableSectorIds.insert(pair<AuId, int>(sector->Id(), sector->SolarSystemCount()));
						m_GalaxyLock.unlock();
						nulyAssignableSectors.append(sector);
					}
					else
						perimeterSectors.AddIfUnique(sector);	// Perimeter sector wormholes affected by addition of a nu sector.
				}
				else
				{
					// Sector location is one of the neighbors of the sector we just created.
					AuPoint fn = fSectorLoc + AuPoint(ix, iy, iz);
					DaSector* pn = FindSectorAt(fn);
					if(!pn)
						continue;

					// Found a neighboring sector.
					if(!IsPerimeterSector(pn->Location()))
					{
						// Neighbor sector is not a perimeter sector; it is assignable.
						// Add the sector to the assignable sectors cache if it is not already there.
						m_GalaxyLock.lockForWrite();
						if(m_AssignableSectorIds.find(pn->Id()) == m_AssignableSectorIds.end())
						{
							// The neighbor sector has just become assignable, we must finalize its wormholes.
							m_AssignableSectorIds.insert(pair<AuId, int>(pn->Id(), pn->SolarSystemCount()));
							m_GalaxyLock.unlock();
							nulyAssignableSectors.append(pn);

							// Unfortunately, we must locate all perimeter sectors of the nuly assignable neighbor sector to
							// finalize the wormholes properly.
							for(int pix=-10; pix<=10; pix+=10)
								for(int piy=-10; piy<=10; piy+=10)
									for(int piz=-10; piz<=10; piz+=10)
										if(pix || piy || piz)
										{
											// Sector location is one of the neighbors of the sector we just created.
											// Perimeter sectors must be included in our wormhole finalizing logic below.
											AuPoint fnp = pn->Location() + AuPoint(pix, piy, piz);
											DaSector* pnp = FindSectorAt(fnp);
											if(pnp && IsPerimeterSector(fnp))
												perimeterSectors.AddIfUnique(pnp);
										}
						}
						else
							m_GalaxyLock.unlock();
					}
					else
						perimeterSectors.AddIfUnique(pn);	// Perimeter sector wormholes affected by addition of a nu sector.
				}
	/*
	if((g_Au.m_LogMessages & LOG_Debug) && !g_Au.IsSceneApp())
	{
		qDebug() << "DaGalaxy::CreateSectorAt" << Id() << Name()
			<< "sector" << sector->Id() << sector->Name() << sector->Description();
		const DaAtomList* subatoms = sector->LockSubatoms();
		if(subatoms)
		{
			for(DaAtomList::const_iterator si = subatoms->cbegin(); si != subatoms->cend(); si++)
			{
				qDebug() << "  DaGalaxy::CreateSectorAt" << Id() << Name()
					<< "system" << (*si)->Id() << (*si)->Name() << (*si)->Description()
					<< ((*si)->IsNormal()?"normal":"NOT NORMAL");
			}
		}
		sector->UnlockSubatoms();
	}
	*/

	// Finalize wormholes in all sectors that are no longer perimeter sectors as a result of the creation of the nu sector.
	// Sectors and their solar systems are managed by scene managers and scene workers after they are assigned.
	// These sectors and their contents have never been assigned to a manager yet so I (master server) can update them and their
	// solar systems in the database.
	FinalizeWormholes(nulyAssignableSectors, perimeterSectors);

	// Return the nu sector created.
	sector->SetNormal();	// New atom is ready for normal thread processing.
	//sector->TickTock();	// Master does not run tick or tock threads.
	return sector;
}

// Get the id of a wormhole's destination solar system based on its direction vector.
// 'vecToDest' is an input and an output parameter.
// On input, vecToDest is a unit vector pointing in the direction of the destination system.
// On output, vecToDest is the actual wormhole vector, in parsecs, to the destination.
AuId DaGalaxy::WormholeDestinationSolarSystem(DaSolarSystem* fromSystem, AuPoint& vecToDest) const
{
	DaSector* fromSector = fromSystem->FindParentSector();
	Q_ASSERT(fromSector);

	AuPoint fSectorLoc = fromSector->Location();

	// Make a list of the sectors and a list of the ids of their solar systems.
	DaAtomList alSectors;
	alSectors.append(fromSector);
	AuIdList idSolarSystems;
	fromSector->ListSubatomIds(idSolarSystems);
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				if(ix==0 && iy==0 && iz==0)
					continue;	// (0,0,0) is the nu sector we just created.

				AuPoint fn = fSectorLoc + AuPoint(ix, iy, iz);
				DaSector *pn = FindSectorAt(fn);
				if(pn)
				{
					alSectors.append(pn);
					pn->ListSubatomIds(idSolarSystems);
				}
			}
		}
	}

	// Now I have a list of all neighboring sector pointers (alSectors), including the center sector,
	// and a list of all neighboring solar system ids (idSolarSystems), including those in the center sector.
	AuPointArray aSolarSystems;
	DaAtomList alSolarSystems;
	AuIdList::const_iterator it;
	for(it = idSolarSystems.cbegin(); it != idSolarSystems.cend(); it++)
	{
		const AuId &idss = (*it);
		if(fromSystem->Is(idss))
			continue;	// Don't add the fromSystem to the list of potential destinations.
		for(DaAtomList::const_iterator it = alSectors.begin(); it != alSectors.end(); it++)
		{
			DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>((*it)->FindNormalSubatomId(idss));
			if(pss)
			{
				alSolarSystems.append(pss);

				AuPoint fs;
				pss->GetLocationRelativeToParent(fs, this);
				aSolarSystems.append(fs);
				break;
			}
		}
	}

	// Now I have all the neighboring solar systems (and mine) in one list of atom pointers (alSolarSystems).
	// I also have a list of their locations in the Galaxy (aSolarSystems).
	// This is all I need to figure out who the wormhole goes to.
	// Calculate "from system" location.
	AuPoint fs;
	fromSystem->GetLocationRelativeToParent(fs, this);

	// Now find the closest other system in the desired direction.
	while(!aSolarSystems.isEmpty())
	{
		// Determine "closest system" index and location.
		int ic = aSolarSystems.FindClosest(fs);
		AuPoint fto = aSolarSystems[ic];
		aSolarSystems.erase(aSolarSystems.begin()+ic);	// Remove the location so we'll find the next closest one in the next pass.

		DaAtomList::iterator it = alSolarSystems.begin() + ic;
		DaSolarSystem* pcss = (DaSolarSystem*)(*it);
		alSolarSystems.erase(it);						// Remove the system pointer to keep the lists in sync.

		// Now I just need to determine if the vector is correct.
		AuPoint vto = fto - fs;
		vto.Unit();
		if(vto.FuzzyEquals(vecToDest))
		{
			// Set the destination system vector to the proper length.
			// Return the destination system id.
			vecToDest = fto - fs;
			return pcss->Id();
		}
	}

	return AuId(0);
}

DaSector* DaGalaxy::FindSectorContaining(const AuPoint& galaxyLoc) const
{
	AuPoint sectorLoc(galaxyLoc);
	sectorLoc.Round(10);
	return FindSectorAt(sectorLoc);
}

DaSector* DaGalaxy::FindSectorAt(const AuPoint &fLoc) const
{
	m_GalaxyLock.lockForRead();
	if(m_SectorLocs.isEmpty())
	{
		m_GalaxyLock.unlock();

		UpdateSectorLocCache();

		m_GalaxyLock.lockForRead();
	}

	QMap<AuPoint, AuId>::const_iterator sli = m_SectorLocs.constFind(fLoc);
	if(sli == m_SectorLocs.cend())
	{
		m_GalaxyLock.unlock();
		return 0;
	}

	DaSector* s = (DaSector*)FindIdRecursive(*sli);
	m_GalaxyLock.unlock();
	return s;
	/*
	if(m_pAtoms)
	{
		//qDebug("DaGalaxy::FindSectorAt %g,%g,%g out of %d sectors",
				//fLoc.x, fLoc.y, fLoc.z, m_pAtoms->count());
		for(DaAtomList::const_iterator it = m_pAtoms->begin(); it != m_pAtoms->end(); it++)
		{
			DaSector* sector = dynamic_cast<DaSector*>(*it);
			if(!sector)
				continue;
			if(!sector->IsNormal())
				continue;
			if(sector->Location().FuzzyEquals(fLoc))
				return sector;
		}
	}
	return 0;
	*/
}

DaSector* DaGalaxy::FindSectorAt(const DaAtomList* subatoms, const AuPoint &fLoc) const
{
	m_GalaxyLock.lockForRead();
	if(m_SectorLocs.isEmpty())
	{
		m_GalaxyLock.unlock();

		UpdateSectorLocCache(subatoms);

		m_GalaxyLock.lockForRead();
	}

	QMap<AuPoint, AuId>::const_iterator sli = m_SectorLocs.constFind(fLoc);
	if(sli == m_SectorLocs.cend())
	{
		m_GalaxyLock.unlock();
		return 0;
	}

	DaSector* s = (DaSector*)FindIdRecursive(*sli);
	m_GalaxyLock.unlock();
	return s;
}

// Insert first time database record.
void DaGalaxy::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Galaxy (id, idp, locX,locY,locZ, rotX,rotY,rotZ, "
				"timeCreate, timeModified, timeTock, timeDeath, galaxy) VALUES "
				"(?, ?, ?,?,?, ?,?,?, ?, ?, ?, ?, ?);");
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());
	qa.addBindValue(m_Galaxy);

	if(!qa.exec())
		qWarning("DaGalaxy::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaGalaxy::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Galaxy SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
				"rotZ=?, timeModified=?, timeTock=?, timeDeath=? "
				"WHERE id=%2;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaGalaxy::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaGalaxy::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaGalaxy* DaGalaxy::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, galaxy FROM a_Galaxy WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaGalaxy *pnu = new DaGalaxy;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			pnu->m_Galaxy = (GALAXY)qa.value(qv++).toUInt();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

// Lists sector ids and the number of systems in each.
void DaGalaxy::ListAssignableSectorIds(unordered_map<AuId, int, AuIdHash>& sectorIds) const
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	if(!HasSubatoms())
	{
		// The answer is easy when we have no sectors. This condition doesn't last long.
		m_GalaxyLock.lockForWrite();
		m_AssignableSectorIds.clear();
		m_GalaxyLock.unlock();
		return;
	}

	m_GalaxyLock.lockForRead();
	if(m_AssignableSectorIds.empty())
	{
		m_GalaxyLock.unlock();

		// Get a list of sectors that can be assigned. We do not assign sectors that form the perimeter of explored space.
		// In other words, we do not assign sectors unless all 26 neighboring sectors exist. Perimeter sectors do not contain any
		// people or facilities so they don't need to be ticked or otherwise managed by anyone but the scene master. This also
		// enables the scene worker to expect the neighboring sectors of all solar systems assigned to exist in the database for
		// the star field.
		// Look for sectors that have a complete shell of neighboring sectors.
		sectorIds.clear();
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaSector* sector = dynamic_cast<DaSector*>(*it);
			if(!sector)
				continue;

			// Add the sector to the assignable sectors cache if all of its neighbors exist.
			if(!IsPerimeterSector(&subs, sector->Location()))
				sectorIds.insert(pair<AuId, int>(sector->Id(), sector->SolarSystemCount()));
		}

		m_GalaxyLock.lockForWrite();
		m_AssignableSectorIds = sectorIds;
		m_GalaxyLock.unlock();
	}
	else
	{
		sectorIds = m_AssignableSectorIds;
		m_GalaxyLock.unlock();
	}
}

// Test for all 26 of the sector's neighbors to exist.
// Returns true if this sector is missing one of its neighbors.
// Returns false if all 26 neighbors of this sector exist.
bool DaGalaxy::IsPerimeterSector(const AuPoint& sectorLoc) const
{
	m_GalaxyLock.lockForRead();
	if(m_SectorLocs.isEmpty())
	{
		m_GalaxyLock.unlock();

		UpdateSectorLocCache();

		m_GalaxyLock.lockForRead();
	}

	// Test the sector loc to see if all of its neighbors are present.
	bool hasAllNeighbors = true;
	for(int ix=-10; ix<=10 && hasAllNeighbors; ix+=10)
	{
		for(int iy=-10; iy<=10 && hasAllNeighbors; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				// Skip the center, that's 'sectorLoc'.
				if(ix == 0 && iy == 0 && iz == 0)
					continue;

				AuPoint fn = sectorLoc + AuPoint(ix, iy, iz);
				if(!m_SectorLocs.contains(fn))
				{
					// The neighbor sector doesn't exist.
					hasAllNeighbors = false;
					break;
				}
			}
		}
	}

	m_GalaxyLock.unlock();

	// This is a perimeter sector if it does not have all 26 of its neighbors.
	return !hasAllNeighbors;
}

// Test for all 26 of the sector's neighbors to exist.
// Returns true if this sector is missing one of its neighbors.
// Returns false if all 26 neighbors of this sector exist.
bool DaGalaxy::IsPerimeterSector(const DaAtomList* subatoms, const AuPoint& sectorLoc) const
{
	m_GalaxyLock.lockForRead();
	if(m_SectorLocs.isEmpty())
	{
		m_GalaxyLock.unlock();

		UpdateSectorLocCache(subatoms);

		m_GalaxyLock.lockForRead();
	}

	// Test the sector loc to see if all of its neighbors are present.
	bool hasAllNeighbors = true;
	for(int ix=-10; ix<=10 && hasAllNeighbors; ix+=10)
	{
		for(int iy=-10; iy<=10 && hasAllNeighbors; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				// Skip the center, that's 'sectorLoc'.
				if(ix == 0 && iy == 0 && iz == 0)
					continue;

				AuPoint fn = sectorLoc + AuPoint(ix, iy, iz);
				if(!m_SectorLocs.contains(fn))
				{
					// The neighbor sector doesn't exist.
					hasAllNeighbors = false;
					break;
				}
			}
		}
	}

	m_GalaxyLock.unlock();

	// This is a perimeter sector if it does not have all 26 of its neighbors.
	return !hasAllNeighbors;
}

// List the sectors in this galaxy.
void DaGalaxy::ListSectors(DaAtomList& sectors) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSector* pAtom = dynamic_cast<DaSector*>(*it);
		if(!pAtom)
			continue;
		sectors.append(pAtom);
	}
}

void DaGalaxy::ListSceneSubatoms(const DaPerson*, const AuPoint& cameraEnvLoc, DaAtomList&)
{
	Q_UNUSED(cameraEnvLoc);
	// Galaxy does not add all subordinate sectors to the scene. This would be
	// the result if the base DaAtom::ListSceneSubatoms was called.
	// Instead, the sector is going to be queried so let it get the appropriate
	// neighbor sectors.
}

void DaGalaxy::RemoveGamePiecesFromStarField()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSector* pAtom = dynamic_cast<DaSector*>(*it);
		if(!pAtom)
			continue;
		pAtom->RemoveGamePiecesFromStarField();
	}
}

// Finalize wormholes in all sectors that are not perimeter sectors.
// Sectors and their solar systems are managed by scene managers and scene workers after they are assigned.
// The sectors and their contents have not been assigned to a manager yet so I (master server) can update them and their
// solar systems in the database.
void DaGalaxy::FinalizeAllWormholes()
{
	if(g_Au.m_LogMessages & LOG_Debug)
	{
		QString logmsg = QString("DaGalaxy::FinalizeAllWormholes %1 %2").arg(Id().GetID()).arg(Name());
		g_Au.Log(logmsg);
	}

	//Q_ASSERT(g_Au.IsSceneMaster());
	m_GalaxyLock.lockForWrite();
	m_AssignableSectorIds.clear();
	m_GalaxyLock.unlock();

	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(!subs.empty())
	{
		if(g_Au.m_LogMessages & LOG_Debug)
		{
			QString logmsg = QString("DaGalaxy::FinalizeAllWormholes %1 %2 done, no sectors.").arg(Id().GetID()).arg(Name());
			g_Au.Log(logmsg);
		}
		return;
	}

	// Get a list of sectors that can be assigned. We do not assign sectors that form the perimeter of explored space.
	// In other words, we do not assign sectors unless all 26 neighboring sectors exist. Perimeter sectors do not contain any
	// people or facilities so they don't need to be ticked or otherwise managed by anyone but the scene master. This also
	// enables the scene worker to expect the neighboring sectors of all solar systems assigned to exist in the database for
	// the star field.
	// Look for sectors that have a complete shell of neighboring sectors.
	DaAtomList assignableSectors;
	DaAtomList perimeterSectors;
	unordered_map<AuId, int, AuIdHash> assignableSectorIds;
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSector* sector = dynamic_cast<DaSector*>(*it);
		if(!sector)
			continue;

		// Add the sector to the assignable sectors cache if all of its neighbors exist.
		if(!IsPerimeterSector(&subs, sector->Location()))
		{
			assignableSectorIds.insert(pair<AuId, int>(sector->Id(), sector->SolarSystemCount()));
			assignableSectors.append(sector);
		}
		else
			perimeterSectors.append(sector);
	}

	m_GalaxyLock.lockForWrite();
	m_AssignableSectorIds = assignableSectorIds;
	m_GalaxyLock.unlock();

	FinalizeWormholes(assignableSectors, perimeterSectors);

	if(g_Au.m_LogMessages & LOG_Debug)
	{
		QString logmsg = QString("DaGalaxy::FinalizeAllWormholes %1 %2 done. Sectors assignable %3, perimeter %4.")
			.arg(Id().GetID()).arg(Name()).arg(assignableSectors.count()).arg(perimeterSectors.count());
		g_Au.Log(logmsg);
	}
}

void DaGalaxy::FinalizeWormholes(const DaAtomList& assignableSectors, const DaAtomList& perimeterSectors)
{
	/*
	if(g_Au.m_LogMessages & LOG_Debug)
	{
		QString logmsg = QString("DaGalaxy::FinalizeWormholes %1 assignable, %2 perimeter sectors.")
			.arg(assignableSectors.count())
			.arg(perimeterSectors.count());
		g_Au.Log(logmsg);
	}
	*/

	// There are no wormholes in the Milky Way galaxy.
	if(m_Galaxy == MilkyWay)
		return;

	// Repair positive wormholes before the negative wormholes in our nuly assignable sectors and their perimeter.
	// Database updates are only signalled for the nuly assignable sectors.
	for(DaAtomList::const_iterator fsi = assignableSectors.begin(); fsi != assignableSectors.end(); fsi++)
	{
		// The algorithm insures the wormholes are correct in any sector where all the neighboring sectors are present when
		// the repair is performed. Therefore, once a sector has been repaired, it should never need repairing again.
		DaSector* asect = dynamic_cast<DaSector*>(*fsi);
		if(asect)
			asect->FinalizePositiveWormholes(true);
	}

	// Do not signal database changes to perimeter sectors. We just need to fix their positive wormholes so we can repair
	// the negative wormholes in the nuly assignable sectors.
	for(DaAtomList::const_iterator psi = perimeterSectors.begin(); psi != perimeterSectors.end(); psi++)
	{
		DaSector* psect = dynamic_cast<DaSector*>(*psi);
		if(psect)
			psect->FinalizePositiveWormholes(true);
	}

	// Repair negative wormholes after positive wormholes in our nuly assignable sectors.
	for(DaAtomList::const_iterator fsi = assignableSectors.begin(); fsi != assignableSectors.end(); fsi++)
	{
		DaSector* asect = dynamic_cast<DaSector*>(*fsi);
		if(asect)
			asect->FinalizeNegativeWormholes();
	}

	// Repairing negative wormholes adds hours to the FinalizeAllWormholes process and it is not really needed. The only case
	// where those wormholes turn out to be missed is when DaUnit attempts to find a wormhole route into an unexplored system.
	// The returning wormhole won't exist yet. Since we know it will exist by the time we get there, the AI assumes there is a
	// corresponding wormhole there.
	/*
	// Repair negative wormholes after positive wormholes in perimeter sectors.
	for(DaAtomList::const_iterator psi = perimeterSectors.begin(); psi != perimeterSectors.end(); psi++)
	{
		DaSector* psect = dynamic_cast<DaSector*>(*psi);
		if(psect)
			psect->FinalizeNegativeWormholes();
	}
	*/

	/*
	if(g_Au.m_LogMessages & LOG_Debug)
	{
		QString logmsg = "DaGalaxy::FinalizeWormholes done.";
		g_Au.Log(logmsg);
	}
	*/
}

// Cache a map of all the sectors, keyed by sector location.
void DaGalaxy::UpdateSectorLocCache() const
{
	QMap<AuPoint, AuId> sectorLocs;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSector* sector = dynamic_cast<const DaSector*>(*it);
		if(!sector)
			continue;
		sectorLocs.insert(sector->Location(), sector->Id());
	}

	m_GalaxyLock.lockForWrite();
	m_SectorLocs = sectorLocs;
	m_GalaxyLock.unlock();

	//qDebug() << "DaGalaxy::UpdateSectorLocCache created sectorLocs cache," << m_SectorLocs.count() << "sectors";
}

// Cache a map of all the sectors, keyed by sector location.
void DaGalaxy::UpdateSectorLocCache(const DaAtomList* subatoms) const
{
	QMap<AuPoint, AuId> sectorLocs;
	if(subatoms)
		for(DaAtomList::const_iterator it = subatoms->cbegin(); it != subatoms->cend(); it++)
		{
			const DaSector* sector = dynamic_cast<const DaSector*>(*it);
			if(!sector)
				continue;
			if(!sector->IsNormal())
				continue;
			sectorLocs.insert(sector->Location(), sector->Id());
		}

	m_GalaxyLock.lockForWrite();
	m_SectorLocs = sectorLocs;
	m_GalaxyLock.unlock();

	//qDebug() << "DaGalaxy::UpdateSectorLocCache created sectorLocs cache," << m_SectorLocs.count() << "sectors";
}

void DaGalaxy::AddSectorLoc(const AuId& sectorId, const AuPoint& sectorLoc)
{
	m_GalaxyLock.lockForWrite();
#if !defined(QT_NO_DEBUG)
	if(m_SectorLocs.contains(sectorLoc))
		qWarning() << "DaGalaxy::AddSectorLoc" << Id() << Name() << "m_SectorLocs already contains sectorLoc" << sectorLoc
			<< "with id" << m_SectorLocs[sectorLoc] << ", replacing it with sectorId" << sectorId;
#endif
	m_SectorLocs.insert(sectorLoc, sectorId);
	m_GalaxyLock.unlock();
}

#if !defined(QT_NO_DEBUG)
void DaGalaxy::RemoveSectorLoc(const AuId& sectorId, const AuPoint& sectorLoc)
{
	m_GalaxyLock.lockForWrite();
	if(m_SectorLocs.contains(sectorLoc))
		if(m_SectorLocs[sectorLoc] != sectorId)
			qWarning() << "DaGalaxy::RemoveSectorLoc" << Id() << Name() << "removing sectorLoc" << sectorLoc
				<< "with id" << m_SectorLocs[sectorLoc] << ", does not match requested sectorId" << sectorId;
	m_SectorLocs.remove(sectorLoc);
	m_GalaxyLock.unlock();
}
#else
void DaGalaxy::RemoveSectorLoc(const AuId& /*sectorId*/, const AuPoint& sectorLoc)
{
	m_GalaxyLock.lockForWrite();
	m_SectorLocs.remove(sectorLoc);
	m_GalaxyLock.unlock();
}
#endif

// List ids of sectors that are ready to decay.
// A sector is ready to decay when !IsExplored
//  and
// none of its neighboring sectors IsExplored.
void DaGalaxy::ListSectorsReadyToDecay(AuIdList& sectorIds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);

	// The IsExplored test is a lot of work. Make a list of the sectors that are explored so we only do it once per sector.
	QSet<AuId> exploredSectors;
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSector* sector = dynamic_cast<const DaSector*>(*it);
		if(!sector)
			continue;	// Non-sector subatom?
		if(!sector->IsReadyToDecay())
			exploredSectors.insert(sector->Id());
	}

	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaSector* sector = dynamic_cast<const DaSector*>(*it);
		if(!sector)
			continue;	// Non-sector subatom?

		// Explored sector is not ready to decay.
		if(exploredSectors.contains(sector->Id()))
			continue;	// Explored sector is not ready for decay.

		// The sector is unexplored.
		// None of the unexplored sector's neighbors can be explored either.
		bool neighborExplored = false;
		for(int ix=-10; ix<=10 && !neighborExplored; ix+=10)
		{
			for(int iy=-10; iy<=10 && !neighborExplored; iy+=10)
			{
				for(int iz=-10; iz<=10; iz+=10)
				{
					// Skip the center, that's 'sector'.
					if(ix == 0 && iy == 0 && iz == 0)
						continue;

					AuPoint fn = sector->Location() + AuPoint(ix, iy, iz);
					const DaSector* neighborSector = FindSectorAt(&subs, fn);
					if(neighborSector && exploredSectors.contains(neighborSector->Id()))
					{
						neighborExplored = true;
						break;
					}
				}
			}
		}
		if(!neighborExplored)
		{
			// Sector is not explored and none of its neighbors are explored either; it can decay.
			qDebug() << "DaGalaxy::ListSectorsReadyToDecay sector" << sector->Id() << sector->Name() << "is ready to decay";
			sectorIds.append(sector->Id());
		}
	}
}

// List ids of sectors that are ready to decay among the 27 potential sectors centered on 'centerSector'.
void DaGalaxy::ListSectorsReadyToDecay(const DaSector* centerSector, AuIdList& sectorIds) const
{
	Q_ASSERT(centerSector);
	Q_ASSERT(centerSector->IsNormal());
	if(!HasSubatoms())
		return;

	// The IsExplored test is a lot of work. Make a list of the sectors that are explored so we only do it once per sector.
	// We only need the ones within two sectors of centerSector because we will be testing the sectors within 1 sector for decay.
	// To test a sector for decay, we need to know if any of its 26 neighboring sectors is explored.
	DaAtomList neighborSectors;			// Sectors within 1 sector of centerSector, including centerSector.
	QSet<AuId> exploredSectors;	// Sectors within 2 sectors of centerSector that are IsExplored.
	for(int ix=-20; ix<=20; ix+=10)
	{
		for(int iy=-20; iy<=20; iy+=10)
		{
			for(int iz=-20; iz<=20; iz+=10)
			{
				DaSector* neighborSector;
				if(ix == 0 && iy == 0 && iz == 0)
					neighborSector = (DaSector*)centerSector;
				else
				{
					AuPoint fn = centerSector->Location() + AuPoint(ix, iy, iz);
					neighborSector = (DaSector*)FindSectorAt(fn);
					if(!neighborSector)
						continue;
					if(!neighborSector->IsNormal())
						continue;
				}

				// Remember the sectors within 1 sector of centerSector, including centerSector. These are the sectors that must
				// be tested for decay. The explored status of their neighbors is needed to make that determination.
				if(abs(ix) < 20 && abs(iy) < 20 && abs(iz) < 20)
					neighborSectors.append(neighborSector);

				// Remember explored sectors out to 2 sectors from centerSector. That way we have all the neighbors of the 3x3x3
				// sectors centered on centerSector.
				if(!neighborSector->IsReadyToDecay())
					exploredSectors.insert(neighborSector->Id());
			}
		}
	}

	// Now we have the explored status of the 5x5x5 sectors centered on centerSector.
	// Test the 3x3x3 sectors centered on centerSector to see if they can decay.
	for(DaAtomList::const_iterator it = neighborSectors.begin(); it != neighborSectors.end(); it++)
	{
		const DaSector* sector = dynamic_cast<const DaSector*>(*it);
		if(!sector)
			continue;	// Non-sector subatom?

		// Explored sector is not ready to decay.
		if(exploredSectors.contains(sector->Id()))
			continue;	// Explored sector is not ready for decay.

		// The sector is unexplored.
		// None of the unexplored sector's neighbors can be explored either.
		bool neighborExplored = false;
		for(int ix=-10; ix<=10 && !neighborExplored; ix+=10)
		{
			for(int iy=-10; iy<=10 && !neighborExplored; iy+=10)
			{
				for(int iz=-10; iz<=10; iz+=10)
				{
					// Skip the center, that's 'sector'.
					if(ix == 0 && iy == 0 && iz == 0)
						continue;

					AuPoint fn = sector->Location() + AuPoint(ix, iy, iz);
					const DaSector* neighborSector = FindSectorAt(fn);
					if(neighborSector && exploredSectors.contains(neighborSector->Id()))
					{
						neighborExplored = true;
						break;
					}
				}
			}
		}
		if(!neighborExplored)
		{
			// Sector is not explored and none of its neighbors are explored either; it can decay.
			qDebug() << "DaGalaxy::ListSectorsReadyToDecay centerSector" << centerSector->Id() << centerSector->Name()
				<< "sector" << sector->Id() << sector->Name() << "is ready to decay";
			sectorIds.append(sector->Id());
		}
	}
}

// Deletes 'sectorsToDecay' from the galaxy.
// 'deletedSectors' is filled with the ids of sectors that get deleted.
// ONLY CALLED ON THE SCENE MASTER
void DaGalaxy::DecaySectors(const AuIdList& sectorsToDecay, AuIdList& deletedSectors)
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	if(!sectorsToDecay.isEmpty())
	{
		if(g_Au.m_LogMessages & LOG_LoadBalance)
		{
			QString logmsg = QString("DaGalaxy::DecaySectors %1 %2 %3 sectors are ready to decay")
				.arg(Id().GetID())
				.arg(Name())
				.arg(sectorsToDecay.size());
			g_Au.Log(logmsg);
		}
		int decayed = 0;
		for(AuIdList::const_iterator si = sectorsToDecay.cbegin(); si != sectorsToDecay.cend(); si++)
		{
			DaSector* sector = dynamic_cast<DaSector*>(FindIdRecursive(*si));
			if(sector)
			{
				// Remove the sector location from our cache of sector locations.
				AuPoint sectorLoc = sector->Location();
				m_GalaxyLock.lockForWrite();
				m_SectorLocs.remove(sectorLoc);
				m_GalaxyLock.unlock();

				// Kill the sector and delete its subatoms. Those have to be killed by the responsible manager server.
				deletedSectors.append(*si);
				sector->TrashSubatoms();
				sector->Kill();

				decayed++;
				if(g_Au.m_LogMessages & LOG_LoadBalance)
				{
					if(decayed%10 == 0 || decayed == sectorsToDecay.count())
					{
						int percent = Percent(decayed, sectorsToDecay.count());
						QString logmsg = QString("DaGalaxy::DecaySectors %1 %2 %3 sectors decayed, %4 to go, %5% complete")
							.arg(Id().GetID())
							.arg(Name())
							.arg(decayed)
							.arg(sectorsToDecay.count() - decayed)
							.arg(percent);
						g_Au.Log(logmsg);
					}
				}
			}
		}

		// Sector decay invalidates the assignable sectors cache.
		if(decayed)
			ResetAssignableSectorsCache();

		if(g_Au.m_LogMessages & LOG_LoadBalance)
		{
			QString logmsg = QString("DaGalaxy::DecaySectors %1 %2 %3 sectors decayed")
				.arg(Id().GetID())
				.arg(Name())
				.arg(decayed);
			g_Au.Log(logmsg);
		}
	}
}

const QColor& DaGalaxy::Color() const
{
	return s_Galaxy[m_Galaxy].Color();
}

QColor DaGalaxy::Color(const AuPoint& fLoc) const
{
	double radLoc = fLoc.Length_2D();							// 2D distance to sample point from core.
	if(radLoc >= Radius())
		return QColor(0,0,0,0);									// Galaxy nonexistent beyond its radius.

	m_GalaxyLock.lockForRead();
	if(m_Image.isNull())
	{
		m_GalaxyLock.unlock();

		m_GalaxyLock.lockForWrite();
		m_Image.load(s_Galaxy[m_Galaxy].m_ImageName);
		m_GalaxyLock.unlock();
	}

	// Calculate scaling factor to convert between pixels and parsecs.
	double pxPerPcX = m_Image.width()/(Radius()*2);				// Pixels per parsec in X.
	double pxPerPcY = m_Image.height()/(Radius()*2);			// Pixels per parsec in Y.
	if(pxPerPcX == 0 || pxPerPcY == 0)
	{
		m_GalaxyLock.unlock();
		return QColor(0,0,0,0);
	}

	// Convert 'floc' to pixels.
	// We will clamp the loc to stay inside the galaxy image.
	// Distance dropoff factors will fade away the edges.
	QPoint pxLoc(m_Image.width()/2 + (int)(fLoc.x * pxPerPcX), m_Image.height()/2 - (int)(fLoc.y * pxPerPcY));
	if(pxLoc.x() < 0)
		pxLoc.setX(0);
	else if(pxLoc.x() >= m_Image.width())
		pxLoc.setX(m_Image.width()-1);
	if(pxLoc.y() < 0)
		pxLoc.setY(0);
	else if(pxLoc.y() >= m_Image.height())
		pxLoc.setY(m_Image.height()-1);

	// Determine the thickness of the galaxy at the sample location. Calculate a linear dropoff from max thickness at the center
	// to min thickness at the edges.
	const int coreRadPx = s_Galaxy[m_Galaxy].CoreDiameterPx() / 2;
	const double coreRadPc = coreRadPx / pxPerPcX;				// Core radius in parsecs.
	double linearThickness = (1. - radLoc / Radius()) * coreRadPc;	// Thickness at loc based on linear dropoff from center.

	// Use the HSV value to modulate the thickness to give 3D bumpiness.
	QColor color(m_Image.pixel(pxLoc));							// Get the pixel color from the galaxy image at that location.
	m_GalaxyLock.unlock();

	double bumpyThickness = linearThickness * color.valueF();	// Image brightness modulates the thickness for bumpy galaxy.
	if(bumpyThickness <= 0)
		return QColor(0,0,0,0);									// Density has reached zero.

	// Now we know the thickness at the pixel location.
	double zoff = fabs(fLoc.z);									// Distance from the Z plane, above or below.
	if(zoff >= bumpyThickness)
		return QColor(0,0,0,0);									// Density is zero above and below the galaxy.

	// Reduce density with Z distance from the galactic plane.
	// This is done on a sine curve to round things off a bit and avoid cones.
	double distRatio = zoff / bumpyThickness;					// Linear value: 0 at galactic plane, 1 at thickness above or below.
	double distAng = acos(distRatio);							// 90 degrees at galactic plane, 0 at thickness distance from plane.
	double density = distAng / (PI/2);							// Calculate density based on angle for rounded dropoff.
	//Q_ASSERT(density >= 0 && density <= 1);

	// The alpha value of the color is the density. Set the density in the alpha value.
	float finalAlpha = density * color.valueF();
	int starsInSector = (int)(finalAlpha * 50.0);				// Maximum is 50 stars in a sector.
	if(!starsInSector)
		finalAlpha = 0;											// Absolute black when no stars in the sector, for star map widget.
	color.setAlphaF(finalAlpha);
	return color;
}

double DaGalaxy::StarDensity(const AuPoint& fLoc) const
{
	// Get the color of the galaxy at that location.
	// The alpha value is the star density.
	return Color(fLoc).alphaF();
}

const DaSolarSystem* DaGalaxy::FindClosestSolarSystemInSector(const AuPoint& fLoc) const
{
	// Locate the sector that contains fLoc.
	const DaSector* sector = FindSectorContaining(fLoc);
	if(!sector)
		return 0;

	// Locate the closest solar system within the sector.
	AuPoint locInSector = fLoc - sector->Location();	// We can do this simply since sector locations are not rotated or scaled.
	return sector->FindClosestSolarSystem(locInSector);
}

// Returns true if any sector neighboring 'sectorLoc' has been named.
bool DaGalaxy::HasNamedNeighborSector(const AuPoint& sectorLoc) const
{
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				if(ix == 0 && iy == 0 && iz == 0)
					continue;

				AuPoint fn = sectorLoc + AuPoint(ix, iy, iz);
				const DaSector* ps = FindSectorAt(fn);
				if(ps && ps->HasName())
					return true;
			}
		}
	}
	return false;
}

quint64 DaGalaxy::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaGalaxy) - sizeof(DaAtom);
	mem += (sizeof(AuId) + sizeof(int)) * m_AssignableSectorIds.size();
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_Image.sizeInBytes();
#else
	mem += m_Image.byteCount();
#endif
	mem += (sizeof(AuPoint) + sizeof(AuId)) * m_SectorLocs.size();
	return mem;
}

//static
AuPoint DaGalaxy::PixelLocation(GALAXY galaxy, int px, int py)
{
	QImage gimg(s_Galaxy[galaxy].m_ImageName);
	if(gimg.isNull())
	{
		qDebug() << "galaxy image not loaded" << s_Galaxy[galaxy].m_ImageName;
		return AuPoint(0,0);
	}

	double pxPerPcX = gimg.width()/(s_Galaxy[galaxy].RadiusPc()*2);				// Pixels per parsec in X.
	double pxPerPcY = gimg.height()/(s_Galaxy[galaxy].RadiusPc()*2);			// Pixels per parsec in Y.
	if(pxPerPcX == 0 || pxPerPcY == 0)
		return AuPoint(0,0);

	/*
	// Convert 'floc' to pixels.
	// We will clamp the loc to stay inside the galaxy image.
	// Distance dropoff factors will fade away the edges.
	QPoint pxLoc;
	pxLoc.x = gimg.width()/2 + (int)(fLoc.x * pxPerPcX);
	pxLoc.y = gimg.height()/2 - (int)(fLoc.y * pxPerPcY);

	pxLoc.x - gimg.width()/2 = (int)(fLoc.x * pxPerPcX);
	pxLoc.y - gimg.height()/2 = -(int)(fLoc.y * pxPerPcY);

	(pxLoc.x - gimg.width()/2)/pxPerPcX = fLoc.x;
	-(pxLoc.y - gimg.height()/2)/pxPerPcY = fLoc.y;
	*/

	AuPoint fLoc;
	fLoc.x = (px - gimg.width()/2)/pxPerPcX;
	fLoc.y = -(py - gimg.height()/2)/pxPerPcY;
	return fLoc;
}

const DaGalaxy* DaGalaxy::FindParentGalaxy() const
{
	return this;
}

DaGalaxy* DaGalaxy::FindParentGalaxy()
{
	return this;
}

void DaGalaxy::SetGalaxy(GALAXY galaxy)
{
	m_Galaxy = galaxy;
}

DaAtom::ATOM_TYPE DaGalaxy::AtomType() const
{
	return ATOM_Galaxy;
}

void DaGalaxy::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << (quint8)m_Galaxy;

	m_GalaxyLock.lockForRead();
	out << (quint32)m_AssignableSectorIds.size();
	for(unordered_map<AuId, int, AuIdHash>::const_iterator si = m_AssignableSectorIds.cbegin(); si != m_AssignableSectorIds.cend();
			si++)
	{
		out << (*si).first;
		out << (*si).second;
	}
	out << m_SectorLocs;
	m_GalaxyLock.unlock();
}

void DaGalaxy::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	quint8 galaxy;
	in >> galaxy;
	m_Galaxy = (GALAXY)galaxy;

	m_GalaxyLock.lockForWrite();
	m_AssignableSectorIds.clear();
	quint32 cnt;
	in >> cnt;
	while(cnt--)
	{
		AuId id;
		int v;
		in >> id;
		in >> v;
		m_AssignableSectorIds.insert(pair<AuId, int>(id,v));
	}
	in >> m_SectorLocs;
	m_GalaxyLock.unlock();
}

void DaGalaxy::ListSupersector(const AuId& centerSector, DaAtomList& supersector) const
{
	// Locate center sector.
	const DaSector* center = dynamic_cast<const DaSector*>(FindSubatomId(centerSector));
	if(!center)
		return;
	AuPoint fSectorLoc(center->Location());
	for(int ix=-10; ix<=10; ix+=10)
	{
		for(int iy=-10; iy<=10; iy+=10)
		{
			for(int iz=-10; iz<=10; iz+=10)
			{
				AuPoint fn = fSectorLoc + AuPoint(ix, iy, iz);
				DaSector* ps = FindSectorAt(fn);
				if(ps)
					supersector.append(ps);
			}
		}
	}
}

quint32 DaGalaxy::GenSeed() const
{
	if(g_Au.IsSceneSolo())
		return 42 + m_Galaxy;	// Deantwo suggested 42 be the seed value of the universe, in solo play. Here it is.
	return (quint32)(CreatedTime().MSecsSinceEpoch() + m_Galaxy * 100000);
}
