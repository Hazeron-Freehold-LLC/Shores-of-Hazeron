//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasolarsystem.cpp,v 1.799 2022/05/19 15:55:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dasolarsystem.h"
//
#include "auatomrenderinfo.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "aunoise.h"
#include "auParticleSystem.h"
#include "ausegment.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QVector>

static int s_SystemAtoms = 1;
static int s_SystemEnv = 1;
static int s_SystemFarAtoms = 1;
static int s_SystemFarLast = 1;
static int s_SystemLast = 1;
static int s_SystemWarp = 1;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Universe map of intergalactic wormhole connections. These are all set up to create two-way connections. There are no
// one-way intergalactic wormholes.
class AuGConn_
{
public:
	GALAXY						m_FromGalaxy;
	bool						m_OneWayOnly;	// If true there are no wormholes at the core, only one-ways at stellar blackholes.
	GALAXY						m_ToGalaxy;
	AuPoint						m_VecToDest;

public:
	/*
	inline AuGConn_(GALAXY fromGalaxy, GALAXY toGalaxy, const AuPoint& vecToDest)
	{
		m_FromGalaxy = fromGalaxy;
		m_ToGalaxy = toGalaxy;
		m_VecToDest = vecToDest;
	}
	*/

	// 'degreesToDest' is trigonometric 0 to right, 90 up, 180 left, 270 down
	inline AuGConn_(GALAXY fromGalaxy, GALAXY toGalaxy, double degreesToDest, bool oneWayOnly=false)
	{
		m_FromGalaxy = fromGalaxy;
		m_OneWayOnly = oneWayOnly;
		m_ToGalaxy = toGalaxy;
		m_VecToDest = AuPoint(1,0).RotateZ(degreesToDest*DEG2RAD);
		m_VecToDest.Unit();
	}
};

// NOTE: There are no intergalactic wormholes between dimensions.
AuGConn_ s_GConns[] =
{
	AuGConn_(ThustrasEye, RansuulsFlamingSword, -40),
	AuGConn_(RansuulsFlamingSword, ThustrasEye, 140),

	AuGConn_(RansuulsFlamingSword, BlackHole, 280),
	AuGConn_(BlackHole, RansuulsFlamingSword, 100),

	AuGConn_(BlackHole, DyrathonsRetreat, 185),
	AuGConn_(DyrathonsRetreat, BlackHole, 5),

	AuGConn_(BlackHole, SevenTen, -45),
	AuGConn_(SevenTen, BlackHole, 135),

	AuGConn_(SevenTen, FallenLegionsOfMuturon, 50),
	AuGConn_(FallenLegionsOfMuturon, SevenTen, 230),

	AuGConn_(FallenLegionsOfMuturon, HeartOfVictorus, 100),
	AuGConn_(HeartOfVictorus, FallenLegionsOfMuturon, 280),

	AuGConn_(SevenTen, AndromedaRising, -50),
	AuGConn_(AndromedaRising, SevenTen, 130),

	AuGConn_(AndromedaRising, InkarBorderRegion, 200),
	AuGConn_(InkarBorderRegion, AndromedaRising, 20),

	AuGConn_(DyrathonsRetreat, ShoresOfHazeron, -75),
	AuGConn_(ShoresOfHazeron, DyrathonsRetreat, 105),

	AuGConn_(ShoresOfHazeron, InkarBorderRegion, -30),
	AuGConn_(InkarBorderRegion, ShoresOfHazeron, 150),

	AuGConn_(EdgeOfTheRift, VeilOfTargoss, -45),
	AuGConn_(VeilOfTargoss, EdgeOfTheRift, 135),

	AuGConn_(VeilOfTargoss, VreenoxEclipse, -50),
	AuGConn_(VreenoxEclipse, VeilOfTargoss, 130),

	AuGConn_(VreenoxEclipse, Core, 195),
	AuGConn_(Core, VreenoxEclipse, 15),

	AuGConn_(Core, HouseZanathar, 135),
	AuGConn_(HouseZanathar, Core, -45),

	AuGConn_(HouseZanathar, FallasEmbrace, 125),
	AuGConn_(FallasEmbrace, HouseZanathar, -55),

	AuGConn_(FallasEmbrace, EdgeOfTheRift, 20),
	AuGConn_(EdgeOfTheRift, FallasEmbrace, 200),

	AuGConn_(HouseZanathar, MuturonEncounter, 225),
	AuGConn_(MuturonEncounter, HouseZanathar, 45),

	AuGConn_(MuturonEncounter, CrownOfOthon, -60),
	AuGConn_(CrownOfOthon, MuturonEncounter, 120),

	AuGConn_(Core, IndigoSea, -50),
	AuGConn_(IndigoSea, Core, 130),

	AuGConn_(IndigoSea, VulcansForge, -40),
	AuGConn_(VulcansForge, IndigoSea, 140),

	AuGConn_(DyrathonsRetreat, FallasEmbrace, -100),
	AuGConn_(FallasEmbrace, DyrathonsRetreat, 80),

	AuGConn_(AndromedaRising, VreenoxEclipse, -105),
	AuGConn_(VreenoxEclipse, AndromedaRising, 75),

	// One-way connections only to milky way.
	AuGConn_(InkarBorderRegion, MilkyWay, -140, true),
	AuGConn_(MilkyWay, InkarBorderRegion, 40, true),

	AuGConn_(EdgeOfTheRift, MilkyWay, 50, true),
	AuGConn_(MilkyWay, EdgeOfTheRift, -130, true),
};
int s_GConCount = sizeof(s_GConns)/sizeof(AuGConn_);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaSolarSystem::DaSolarSystem()
{
	m_CatalogNameValid = false;
	m_Final = false;
	m_GenHab = HABITABLE_Unspecified;
	m_GenSeed = 0;
//	m_glModels = 0;
	m_HighestQShip = 0;
	m_isOnlyStarFieldLoaded = false;
	m_StarLight = -1.f;		// unset state.
	m_StrongGravity = 0;	// unknown
	m_WarpSignature = 0;
	m_WormholesFinalized = false;
	m_WormholesPositioned = false;
}

DaSolarSystem::~DaSolarSystem()
{
}

DaSolarSystem::DaSolarSystem(const DaSolarSystem &rhs) : DaAtom(rhs)
{
	m_StrongGravity = rhs.m_StrongGravity;
	m_Final = rhs.m_Final;
	m_GenHab = rhs.m_GenHab;
	m_GenSeed = rhs.m_GenSeed;
	m_HighestQShip = rhs.m_HighestQShip;
	m_isOnlyStarFieldLoaded = rhs.m_isOnlyStarFieldLoaded;
	m_WormholesFinalized = rhs.m_WormholesFinalized;
	m_WormholesPositioned = rhs.m_WormholesPositioned;

	rhs.m_HengeDestLock.lock();
	m_HengeDest = rhs.m_HengeDest;
	rhs.m_HengeDestLock.unlock();

	rhs.m_LastAvatarAccountActiveLock.lock();
	m_LastAvatarAccountActive = rhs.m_LastAvatarAccountActive;
	rhs.m_LastAvatarAccountActiveLock.unlock();

	rhs.m_CatalogNameLock.lockForRead();
	m_CatalogNameValid = rhs.m_CatalogNameValid;
	if(rhs.m_CatalogNameValid)
		m_CatalogName = rhs.m_CatalogName;
	rhs.m_CatalogNameLock.unlock();

	rhs.m_DetectedByLock.lockForRead();
	m_DetectedBy = rhs.m_DetectedBy;
	rhs.m_DetectedByLock.unlock();

	rhs.m_EmpiresPresentLock.lockForRead();
	m_EmpiresPresent = rhs.m_EmpiresPresent;
	rhs.m_EmpiresPresentLock.unlock();

	rhs.m_ExploredByLock.lockForRead();
	m_ExploredBy = rhs.m_ExploredBy;
	rhs.m_ExploredByLock.unlock();

	rhs.m_ScannedByLock.lockForRead();
	m_ScannedBy = rhs.m_ScannedBy;
	rhs.m_ScannedByLock.unlock();

	rhs.m_LastFlagInfluenceLock.lockForRead();
	m_LastFlagInfluence = rhs.m_LastFlagInfluence;
	rhs.m_LastFlagInfluenceLock.unlock();

	rhs.m_LastOfficerVisitLock.lockForRead();
	m_LastOfficerMod = rhs.m_LastOfficerMod;
	m_LastOfficerVisit = rhs.m_LastOfficerVisit;
	rhs.m_LastOfficerVisitLock.unlock();

	rhs.m_LastPersonVisitLock.lockForRead();
	m_LastPersonMod = rhs.m_LastPersonMod;
	m_LastPersonScene = rhs.m_LastPersonScene;
	m_LastPersonVisit = rhs.m_LastPersonVisit;
	rhs.m_LastPersonVisitLock.unlock();

	rhs.m_LastPirateLock.lockForRead();
	m_LastPirateShip = rhs.m_LastPirateShip;
	m_LastPirateStation = rhs.m_LastPirateStation;
	rhs.m_LastPirateLock.unlock();

	rhs.m_NameLock.lockForRead();
	m_sName = rhs.m_sName;
	rhs.m_NameLock.unlock();

	rhs.m_WarpSignatureLock.lock();
	m_LastWarpMod = rhs.m_LastWarpMod;
	m_WarpSignature = rhs.m_WarpSignature;
	rhs.m_WarpSignatureLock.unlock();

	rhs.m_StarLightLock.lockForRead();
	m_StarLight = rhs.m_StarLight;
	rhs.m_StarLightLock.unlock();

	rhs.m_WormholesLock.lockForRead();
	m_Wormholes = rhs.m_Wormholes;
	rhs.m_WormholesLock.unlock();

	rhs.m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange = rhs.m_CommodityExchange;
	rhs.m_CommodityExchangeLock.unlock();
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaSolarSystem::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	// Do not send the name to people whose empire has not explored this system.
	if(HasName() && IsExploredBy(sceneAtom.EmpireId()))
	{
		m_NameLock.lockForRead();
		toClient << m_sName;
		m_NameLock.unlock();
	}
	else
		toClient << QString();

	m_WormholesLock.lockForRead();
	m_Wormholes.Tx(toClient);
	m_WormholesLock.unlock();

	m_ExploredByLock.lockForRead();
	toClient << m_ExploredBy;			// Empire ids that have explored this system.
	m_ExploredByLock.unlock();

	m_ScannedByLock.lockForRead();
	toClient << m_ScannedBy;			// Empire ids that have scanned this system.
	m_ScannedByLock.unlock();

	toClient << (quint8)m_GenHab;
	toClient << m_GenSeed;
}

// Receive updates to this atom from a server.
void DaSolarSystem::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	fromServer >> m_sName;
	m_Wormholes.Rx(fromServer);
	fromServer >> m_ExploredBy;			// Empire ids that have explored this system.
	fromServer >> m_ScannedBy;			// Empire ids that have scanned this system.
	quint8 ui8;
	fromServer >> ui8;
	m_GenHab = (HABITABILITY)ui8;
	fromServer >> m_GenSeed;
}

// Refresh data from valid counterpart.
bool DaSolarSystem::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaSolarSystem* pA = (const DaSolarSystem*)pAtom;
	m_GenHab = pA->m_GenHab;
	m_GenSeed = pA->m_GenSeed;

	pA->m_ExploredByLock.lockForRead();
	m_ExploredByLock.lockForWrite();
	m_ExploredBy = pA->m_ExploredBy;
	m_ExploredByLock.unlock();
	pA->m_ExploredByLock.unlock();

	pA->m_ScannedByLock.lockForRead();
	m_ScannedByLock.lockForWrite();
	m_ScannedBy = pA->m_ScannedBy;
	m_ScannedByLock.unlock();
	pA->m_ScannedByLock.unlock();

	pA->m_NameLock.lockForRead();
	m_NameLock.lockForWrite();
	m_sName = pA->m_sName;
	m_NameLock.unlock();
	pA->m_NameLock.unlock();

	pA->m_WormholesLock.lockForRead();
	m_WormholesLock.lockForWrite();
	m_Wormholes = pA->m_Wormholes;
	m_WormholesLock.unlock();
	pA->m_WormholesLock.unlock();

	return false;
}

DaAtom *DaSolarSystem::Clone() const
{
	return new DaSolarSystem(*this);
}

void DaSolarSystem::Create(HABITABILITY genHab, quint32 genSeed)
{
	Q_ASSERT(genHab == HABITABLE_HomeWorld || genHab == HABITABLE_System || genHab == HABITABLE_EmptySystem);
	DaAtom::Create();
	m_GenHab = genHab;
	m_GenSeed = genSeed;
}

// Returns the default name of this solar system, based on galaxy catalog code and galaxy location.
// e.g. WOU'WWK'JL'a
QString DaSolarSystem::CatalogName() const
{
	if(IsEmptySystem())
		return "";

	// It is not valid. Wait on the mutex.
	m_CatalogNameLock.lockForRead();
	if(m_CatalogNameValid)
	{
		// Whoever unlocked the mutex must have done the work.
		QString n(m_CatalogName);
		m_CatalogNameLock.unlock();
		return n;
	}
	m_CatalogNameLock.unlock();

	const DaGalaxy* galaxy = FindParentGalaxy();
	if(!galaxy)
		return "";

	// This location will be in parsecs. System locations are rounded to one decimal place so we don't expect 1/100th's.
	AuPoint gloc;
	GetLocationRelativeToParent(gloc, galaxy);
	QString n(CatalogName(gloc));
	m_CatalogNameLock.lockForWrite();
	m_CatalogName = n;
	m_CatalogNameValid = true;
	m_CatalogNameLock.unlock();
	return n;
}

//static
QString DaSolarSystem::CatalogName(const AuPoint& galaxyLoc)
{
	AuPoint gloc(galaxyLoc * 10);								// So the significant numbers are not fractional.

	// Determine octant from signs of coordinate.
	int octant;
	if(gloc.z >= 0)
	{
		// Positive Z is octants 0-3, counter-clockwise starting from top right, +,+,+.
		if(gloc.x >= 0)
		{
			if(gloc.y >= 0)
				octant = 0;
			else
				octant = 3;
		}
		else
		{
			if(gloc.y >= 0)
				octant = 1;
			else
				octant = 2;
		}
	}
	else
	{
		// Negative Z is octants 4-7, counter-clockwise starting from top right, +,+,-.
		if(gloc.x >= 0)
		{
			if(gloc.y >= 0)
				octant = 4;
			else
				octant = 7;
		}
		else
		{
			if(gloc.y >= 0)
				octant = 5;
			else
				octant = 6;
		}
	}
	QString octantNames[8] =
	{
		"a",
		"b",
		"c",
		"d",
		"e",
		"f",
		"g",
		"h"
	};

	AuId idx((quint32)fabs((double)round(gloc.x)));
	AuId idy((quint32)fabs((double)round(gloc.y)));
	AuId idz((quint32)fabs((double)round(gloc.z)));

	/*
	if(includeGalaxy)
		m_CatalogName = QString("%1'%2'%3.%4.%7")
			.arg(idx.StringId())
			.arg(idy.StringId())
			.arg(idz.StringId())
			.arg(octantNames[octant])
			.arg(GalaxyCatalog(galaxy->Galaxy()));
	else
	*/

	QString n = QString("%1'%2'%3'%4")
		.arg(idx.StringId())
		.arg(idy.StringId())
		.arg(idz.StringId())
		.arg(octantNames[octant]);
	return n;
}

QString DaSolarSystem::Name() const
{
	if(IsEmptySystem())
		return "";
	m_NameLock.lockForRead();
	if(!m_sName.isEmpty())
	{
		QString s(m_sName);
		m_NameLock.unlock();
		return s;
	}
	m_NameLock.unlock();

	QString catName = CatalogName();
	if(catName.isEmpty())
		return QString(tr("System %1")).arg(Id().StringId());
	return catName;
}

void DaSolarSystem::StarMapInfo(quint32 empireId, AuStarMapSystem& sm) const
{
	sm.m_Name = NameKnownTo(empireId);
	sm.m_Loc = Location();
	sm.m_GenHab = GenHab();
	sm.m_GenSeed = GenSeed();
	bool isExplored = IsExploredBy(empireId);
	if(isExplored)
	{
		sm.m_InfoBits = AuStarMapSystem::Explored;				// System was explored by 'empireId'.
		if(IsScannedBy(empireId))
			sm.m_InfoBits |= AuStarMapSystem::Scanned;			// System was scanned by 'empireId'.
	}

	if(IsCoreBlackHole())
		sm.m_InfoBits |= AuStarMapSystem::CoreBlackHole;		// System contains the galaxy's core black hole.
	if(IsEmptySystem())
		sm.m_InfoBits |= AuStarMapSystem::EmptySystem;			// System contains no sun or worlds.

	quint32 ds = DetectedSignalStrength(empireId);				// Signal strength detected by empireId.
	if(ds > 0)
		sm.SetDetectedSignalStrength(ds);						// System emissions were detected.

	if(isExplored)
	{
		// This system is explored. We know where the wormholes go, even if they are not explored by us.
		// Fill the system record with information about wormholes, both explored and unexplored.
		const AuWormholeArray& wha = LockWormholes();
		for(AuWormholeArray::const_iterator wi = wha.cbegin(); wi != wha.cend(); wi++)
		{
			// Create a wormhole record for the viewer.
			AuStarMapWormhole gvwh((*wi).IsExploredBy(empireId), (*wi).Polarity(), (*wi).SectorVecToDestination());
			sm.m_Wormholes.append(gvwh);
		}
		UnlockWormholes();
	}
}

// create the primary star and possibly one or two companions
// returns the primary star; it has not been added to the solar system's atom list
// NOTE: Does not create planets or other system contents.
DaStar* DaSolarSystem::CreateStars() const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	AuDice dice(m_GenSeed);
	// Determine the general nature of the system: solitary, binary, or trinary
	// The core is always at least a binary, so blackhole always has a companion star.
	bool isCoreBlackHole = IsCoreBlackHole();
	int lNature = dice.Roll(2,6, isCoreBlackHole ? 6 : 0);	// +6 guarantees a secondary star.
	bool isStellarBlackHole = false;
	if(!isCoreBlackHole && m_GenHab == HABITABLE_System)
	{
		// Check to see if this is a stellar black hole system.
		int nat = dice.Roll(1,100);
		if(nat <= 2)	// 2% chance?
		{
			isStellarBlackHole = true;
			lNature = dice.Roll(2,6, 6);	// +6 guarantees a secondary star.
		}
	}
	int lStars;
	if(lNature >= 12)
		lStars = 3;	// trinary system
	else if(lNature >= 8)
		lStars = 2;	// binary system
	else
		lStars = 1;	// solitary system

	//qDebug("DaSolarSystem::CreateStars - Creating %d stars.", lStars);

	int lCompTypeMod=0,lCompSizeMod=0;
	DaStar* pPrimary = 0;
	for(int lStar=0; lStar<lStars; lStar++)
	{
		DaStar* pStar = new DaStar;
		pStar->ConnectSignals();
		pStar->Create();
		if(lStar == 0)
		{
			// primary star
			pStar->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(pStar);

			pPrimary = pStar;
			if(isCoreBlackHole)
			{
				// Create the black hole star.
				GALAXY galaxy = Galaxy();
				pStar->GenSupermassiveBlackHole(dice, galaxy, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(galaxy);
			}
			else if(isStellarBlackHole)
			{
				// Create the black hole star.
				pStar->GenStellarBlackHole(dice, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(GALAXY_Count);
			}
			else
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					lCompTypeMod = pStar->GenPrimaryType(dice);
					dice.Random();
					lCompSizeMod = pStar->GenPrimarySize(dice);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
		}
		else
		{
			// Companion stars orbit the primary.
			Q_ASSERT(pPrimary);
			pStar->SetPosAndOrient(AuAtomPos(pPrimary->Id(), AuMatrix4x4()));
			ether->AddAtom(pStar);

			int companion;
			if(m_GenHab == HABITABLE_System && (pPrimary->IsAnyBlackHole() || pPrimary->IsNeutronStar()))
				companion = lStar;	// Push companions of black holes and neutron stars farther out.
			else
				companion = lStar-1;
			char chCompanionOrbit = pPrimary->GenCompanionOrbit(dice, companion);
			dice.Random();
			Q_ASSERT(chCompanionOrbit >= 0);
			pStar->SetOrbit(chCompanionOrbit);

			// Determine maximum number of orbits.
			// Minimum orbit that can be occupied by the companion star.
			char chMinOrb = pPrimary->MinOrbitAvailableFrom(chCompanionOrbit);
			// Maximum number of orbits for companion.
			char chCompMaxOrbits = chCompanionOrbit - chMinOrb;
			if(chCompMaxOrbits > chCompanionOrbit/2)
			{
				chCompMaxOrbits = chCompanionOrbit/2;
				chMinOrb = chCompanionOrbit - chCompMaxOrbits;
			}
			pStar->SetMaxOrbits(chCompMaxOrbits);

			// Generate a companion star that fits within the orbit space available.
			do
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					pStar->GenCompanionType(dice, lCompTypeMod);
					dice.Random();
					pStar->GenCompanionSize(dice, lCompSizeMod);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
			while(pStar->RadiusInOrbits() > chCompMaxOrbits);

			// reserve orbit zones around parent
			// max orbit this star might use around primary.
			char chMaxOrb = chCompanionOrbit + chCompMaxOrbits;
			pPrimary->ReserveZones(chMinOrb, chMaxOrb);
		}
		pStar->SetNormal();	// New atom is ready for normal thread processing.
	}
	return pPrimary;
}

// create the primary star and possibly one or two companions
// returns the primary star; it has not been added to the solar system's atom list
// NOTE: Does not create planets or other system contents.
DaStar* DaSolarSystem::CreateStarsForScan(GALAXY galaxy) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	AuDice dice(m_GenSeed);
	// Determine the general nature of the system: solitary, binary, or trinary
	// The core is always at least a binary, so blackhole always has a companion star.
	bool isCoreBlackHole = IsCoreBlackHole();
	int lNature = dice.Roll(2,6, isCoreBlackHole ? 6 : 0);	// +6 guarantees a secondary star.
	bool isStellarBlackHole = false;
	if(!isCoreBlackHole && m_GenHab == HABITABLE_System)
	{
		// Check to see if this is a stellar black hole system.
		int nat = dice.Roll(1,100);
		if(nat <= 2)	// 2% chance?
		{
			isStellarBlackHole = true;
			lNature = dice.Roll(2,6, 6);	// +6 guarantees a secondary star.
		}
	}
	int lStars;
	if(lNature >= 12)
		lStars = 3;	// trinary system
	else if(lNature >= 8)
		lStars = 2;	// binary system
	else
		lStars = 1;	// solitary system

	int lCompTypeMod=0,lCompSizeMod=0;
	DaStar* pPrimary = 0;
	for(int lStar=0; lStar<lStars; lStar++)
	{
		DaStar* pStar = new DaStar;
		pStar->CreateForScan();		// NOTE: This does not create any planets. Ids are client local, not real.
		if(lStar == 0)
		{
			// primary star
			pStar->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(pStar);

			pPrimary = pStar;
			if(isCoreBlackHole)
			{
				// Create the black hole star.
				pStar->GenSupermassiveBlackHole(dice, galaxy, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(galaxy);
			}
			else if(isStellarBlackHole)
			{
				// Create the black hole star.
				pStar->GenStellarBlackHole(dice, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(GALAXY_Count);
			}
			else
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					lCompTypeMod = pStar->GenPrimaryType(dice);
					dice.Random();
					lCompSizeMod = pStar->GenPrimarySize(dice);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
		}
		else
		{
			// Companion stars orbit the primary.
			Q_ASSERT(pPrimary);
			pStar->SetPosAndOrient(AuAtomPos(pPrimary->Id(), AuMatrix4x4()));
			ether->AddAtom(pStar);

			int companion;
			if(m_GenHab == HABITABLE_System && (pPrimary->IsAnyBlackHole() || pPrimary->IsNeutronStar()))
				companion = lStar;	// Push companions of black holes and neutron stars farther out.
			else
				companion = lStar-1;
			char chCompanionOrbit = pPrimary->GenCompanionOrbit(dice, companion);
			dice.Random();
			Q_ASSERT(chCompanionOrbit >= 0);
			pStar->SetOrbit(chCompanionOrbit);

			// Determine maximum number of orbits.
			// Minimum orbit that can be occupied by the companion star.
			char chMinOrb = pPrimary->MinOrbitAvailableFrom(chCompanionOrbit);
			// Maximum number of orbits for companion.
			char chCompMaxOrbits = chCompanionOrbit - chMinOrb;
			if(chCompMaxOrbits > chCompanionOrbit/2)
			{
				chCompMaxOrbits = chCompanionOrbit/2;
				chMinOrb = chCompanionOrbit - chCompMaxOrbits;
			}
			pStar->SetMaxOrbits(chCompMaxOrbits);

			// Generate a companion star that fits within the orbit space available.
			do
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					pStar->GenCompanionType(dice, lCompTypeMod);
					dice.Random();
					pStar->GenCompanionSize(dice, lCompSizeMod);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
			while(pStar->RadiusInOrbits() > chCompMaxOrbits);

			// reserve orbit zones around parent
			// max orbit this star might use around primary.
			char chMaxOrb = chCompanionOrbit + chCompMaxOrbits;
			pPrimary->ReserveZones(chMinOrb, chMaxOrb);
		}
		pStar->SetNormal();	// New atom is ready for normal thread processing.
	}
	return pPrimary;
}

// Report a longer description of this atom for examination purposes.
QString DaSolarSystem::Description() const
{
	AuPoint fLoc = Location();
	return QString("(%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);
}

// Prepare subatoms that may not exist until requested from the server.
// On the server farm this is ONLY CALLED ON THE SCENE MANAGER.
void DaSolarSystem::FinalizeContents()
{
	if(IsFinal())
		return;
	SetFinal(true);

	//qDebug() << "DaSolarSystem::FinalizeContents" << *this << "habitability =" << HabitabilityName(GenHab());
	if(IsEmptySystem())
	{
		// Notify scene master that we finalized a solar system.
		emit SolarSystemFinal(Id());
		SetModified(NOTIFY_Database);
		return;
	}
	else if(m_GenHab == HABITABLE_HomeWorld)
	{
		CreateHomeWorld();
		return;
	}

	// Create primary star and companions using the last gen seed that was set.
	DaStar* primary = CreateStars();
	if(!primary)
		return;	// wut?

	// Create planets around the stars, invoking logic to increase the prevalence of habitable worlds.
	primary->CreatePlanets(GenHab());
	if(primary->IsStellarBlackHole())
		m_StrongGravity = 3;
	else if(primary->IsNeutronStar())
		m_StrongGravity = 4;

	// Notify the world that a solar system was finalized.
	// Signals the addition of all the nu atoms so they get saved in the db.
	AddFinalPrimary(primary);
	SetLastPersonVisited();	// Prevent immediate decay of the nuly generated system contents.
	SetModified(NOTIFY_Database);
}

void DaSolarSystem::AddFinalPrimary(DaStar* primary)
{
	//qDebug("DaSolarSystem::AddFinalPrimary %d primary %d",
			//Id().GetID(), primary->Id().GetID());
	//Q_ASSERT(g_Au.IsSceneManager());
	SetNameIncludePlanets(Name(), false);

	PositionWormholes();

	// Signal the addition of all the nu atoms so they get saved in the db.
	DaAtomList allNew;
	primary->ListNormalSubatomsRecursive(allNew);
	for(DaAtomList::const_iterator it = allNew.begin(); it != allNew.end(); it++)
	{
		DaAtom* pAtom = *it;
		pAtom->InitSystemPosition();
		g_Au.AtomAdded(pAtom->Id());
	}

	// It is important to add the primary last; this causes it to be stored in
	// the database last. This cues the Scene Worker server that all of the
	// solar system's contents have been stored in the database. Otherwise, it
	// may load an assigned solar system before it has been completely stored.
	g_Au.AtomAdded(primary->Id());

	//qDebug("DaSolarSystem::AddFinalPrimary %d notifying scene master", Id().GetID());

	// Notify scene master that we finalized a solar system. It will load the primary star so the scene master knows it is explored.
	emit SolarSystemFinal(Id());

	//qDebug("DaSolarSystem::AddFinalPrimary %d done", Id().GetID());
}

void DaSolarSystem::PrepareForResourceScan(const QString& catalogName, const QString& systemName, GALAXY galaxy,
		HABITABILITY genHab, quint32 genSeed, bool isCoreBlackHole, const AuMatrix4x4& systemT, float starLight)
{
	SetFinal(true);
	m_NameLock.lockForWrite();
	m_sName = systemName;
	m_NameLock.unlock();

	m_CatalogNameLock.lockForWrite();
	m_CatalogName = catalogName;
	m_CatalogNameValid = true;
	m_CatalogNameLock.unlock();

	ChangePosAndOrient(AuAtomPos(ParentId(), systemT));

	CreateContentsForScan(galaxy, genHab, genSeed, isCoreBlackHole);

	m_StarLightLock.lockForWrite();
	m_StarLight = starLight;
	m_StarLightLock.unlock();

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->PrepareForScan();
}

void DaSolarSystem::CreateContentsForScan(GALAXY galaxy, HABITABILITY genHab, quint32 genSeed, bool isCoreBlackHole)
{
	m_GenHab = genHab;
	m_GenSeed = genSeed;
	m_StrongGravity = isCoreBlackHole ? 2 : 1;
	if(genHab == HABITABLE_EmptySystem)
		return;

	DaStar* primary = CreateStarsForScan(galaxy);	// Create primary star and companions
	if(!primary)
		return;	// wut?

	// Create planets around the stars, invoking logic to increase the prevalence of habitable worlds.
	primary->CreatePlanetsForScan(GenHab(), galaxy);
	if(primary->IsStellarBlackHole())
		m_StrongGravity = 3;
	else if(primary->IsNeutronStar())
		m_StrongGravity = 4;
}

// Return a planet suitable for a homeworld.
// Regenerates the stars and planets of this system to achieve this.
// This is only called on the scene manager.
// This is also called when running in single-user off-line designer mode.
DaWorldGlobe* DaSolarSystem::CreateHomeWorld()
{
	//qDebug() << "DaSolarSystem::CreateHomeWorld" << Id() << Name();
	//Q_ASSERT(g_Au.IsSceneManager());
	//Q_ASSERT(m_GenHab == HABITABLE_HomeWorld);
	// Generate stars and planets repeatedly until we're satisfied with the result.
	DaStar* primary;
	DaWorldGlobe* pHomeWorld;
	do
	{
		// Create primary star and companions using the last gen seed that was set.
		primary = CreateStars();

		// Create planets around the stars, invoking 'homeworld' logic.
		primary->CreatePlanets(GenHab());
		//qDebug() << "DaSolarSystem::CreateHomeWorld" << Id() << Name() << "Generated system contents, primary" << primary->Id();

		// To be a valid home system, insure there is eludium and lumenite present.
		// Due to the way resources are distributed on the terrain, this test is not perfect. It is possible to detect
		// lumenite without ever finding a mineable quantity of it anywhere. Eludium is rarely a problem because when it
		// is present it tends to be abundant.
		int eludium = 0;
		int lumenite = 0;
		pHomeWorld = dynamic_cast<DaWorldGlobe*>(primary->GetHabitableWorld(HABITABLE_HomeWorld));
		if(pHomeWorld)
		{
			primary->FindStartupResources(eludium, lumenite);
			//qDebug() << "DaSolarSystem::CreateHomeWorld" << Id() << Name() << "eludium" << eludium << "lumenite" << lumenite;
		}
		//else
			//qDebug() << "DaSolarSystem::CreateHomeWorld" << Id() << Name() << "no suitable homeWorld";

		if(eludium < 10 || lumenite < 10)
		{
			// Insufficient startup resources so force regen of the system contents.
			pHomeWorld = 0;
			TrashSubatom(primary->Id());					// Delete the failed attempt in preparation for next attempt.

			// Use noise to select a nu genSeed value for the next attempt.
			int randFunc = m_GenSeed % 12;	// The random noise function is based on my initial genSeed.
			do
			{
				double noise = AuNoise::IntegerNoise(randFunc, m_GenSeed);
				m_GenSeed += (quint32)(noise * 0xffffffff);
			}
			while(!m_GenSeed);
		}
	}
	while(!pHomeWorld);

	// Notify the world that a solar system was finalized.
	// Signals the addition of all the nu atoms so they get saved in the db.
	AddFinalPrimary(primary);
	SetLastPersonVisited();	// Prevent immediate decay of the nuly generated system contents.
	SetModified(NOTIFY_Database);

	//qDebug() << "DaSolarSystem::CreateHomeWorld" << Id() << Name() << "found homeworld" << pHomeWorld->Id() << pHomeWorld->Name();
	return pHomeWorld;
}

// Returns a pointer to a habitable planet within this solar system, if there is one. NULL if not.
DaWorldLL* DaSolarSystem::FindHabitableWorld(HABITABILITY hab) const
{
	// Query the stars for a habitable world.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* star = dynamic_cast<DaStar*>(*it);
		if(!star)
			continue;
		//qDebug("DaSolarSystem::FindHabitableWorld - checking star %d %s",
				//pStar->Id().GetID(), (const char*)pStar->Name().toAscii());
		DaWorldLL* pHomeWorld = star->GetHabitableWorld(hab);
		if(pHomeWorld)
			return pHomeWorld;
	}
	return 0;
}

// Returns a pointer to the primary star in this system, if there is one. 0 is returned if no star is found.
DaStar* DaSolarSystem::FindPrimaryStar() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Search through my atoms for the primary star. It will be my only star subatom.
		// Companion stars are subatoms of the primary star.
		DaStar* star = dynamic_cast<DaStar*>(*it);
		if(!star)
			continue;
		Q_ASSERT(star->IsPrimary());
		return star;
	}
	return 0;
}

DaStar* DaSolarSystem::FindPrimaryStarNotBlackHole() const
{
	// Locate the primary star.
	DaStar* primary = FindPrimaryStar();
	if(primary && primary->IsAnyBlackHole())
	{
		// Primary is a black hole.
		// Locate the first companion star.
		DaAtomList stars;
		primary->ListStars(stars);
		if(!stars.isEmpty())
		{
			// We expect to find companion stars around every black hole.
			primary = (DaStar*)stars.first();
			if(primary->StarNumber() > 0)
				primary = (DaStar*)stars.last();				// Must be a trinary and the "primary" is the last one in the list.
		}
	}
	return primary;
}

// Setup light sources for lighting calculations to render a given object.
void DaSolarSystem::SetupLights(DaPerson* sceneAtom, const AuMatrix4x4& model, AuGLEffects& re)
{
	// Collect information about all light sources.
	QList<AuGLSun> suns;
	QList<AuGLLight> lights;
	GetRenderLightsRecursive(*sceneAtom, model, suns, lights);
	SetRenderLights(re, suns, lights, StarLight());	// Starlight is the basis for most ambient lighting.
}

float DaSolarSystem::StarLight() const
{
	m_StarLightLock.lockForRead();
	if(m_StarLight < 0.f)
	{
		m_StarLightLock.unlock();

		// Get a count of the solar systems in this sector and neighboring sectors.
		int systems = 0;
		const DaSector* parsect = dynamic_cast<const DaSector*>(Parent());	// Parent is always a sector.
		if(parsect)
		{
			AuPoint fSectorLoc = parsect->Location();	// Location of this sector in the galaxy.
			const DaGalaxy *pg = dynamic_cast<const DaGalaxy*>(parsect->Parent());
			if(pg)
			{
				for (int ix=-10; ix<=10; ix+=10)
				{
					for (int iy=-10; iy<=10; iy+=10)
					{
						for (int iz=-10; iz<=10; iz+=10)
						{
							AuPoint frel(ix, iy, iz);
							AuPoint fn = fSectorLoc + frel;
							const DaSector *pn = pg->FindSectorAt(fn);
							if(pn)
								systems += pn->SolarSystemsInSector();
						}
					}
				}
			}
		}

		// We know the maximum possible systems per sector is 50 so the 3x3x3 sector box could contain...
		const float ssmax = 50 * 27;
		float ratio = (float)systems/ssmax;

		m_StarLightLock.lockForWrite();
		m_StarLight = ratio * (AuGLLight::MaxAmbientStarlight() - AuGLLight::AmbientDarkness()) + AuGLLight::AmbientDarkness();
		//qDebug("DaSolarSystem::StarLight - %g", m_StarLight);
	}
	float s = m_StarLight;
	m_StarLightLock.unlock();
	return s;
}

// Renders an entire scene from the perspective of this object.
// View transformation has already been set up.
void DaSolarSystem::RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, DaPerson* sceneAtom)
{
	// Set up lighting for the scene.
	// Must include inverse camera projection so world location of lights is proper
	// with respect to the camera. This is where vertices will end up projected in
	// the shader.
	AuMatrix4x4 tmodel = model * sceneAtom->EnvTransform().Inverse();
	SetupLights(sceneAtom, tmodel, *ari.re);

	// Render the solar system's environment first.
	if(!sceneAtom->IsEyeUnderWater())
	{
		DaAtom* env = FindEnvironmentAtom();
		if(env && s_SystemEnv)
		{
			env->RenderEnvironment(projection, farProjection, model, ari, sceneAtom);
			ari.re->glClear(GL_DEPTH_BUFFER_BIT);
		}
	}

	AuModelMatrixHash rmod;
	CollectModelMatrices(tmodel, rmod);

	DaAtomList shadSc;
	DaAtomList shadScFog;
	DaAtomList shadScGlobe;
	DaAtomList shadScGlobeFog;
	DaAtomList shadScRingworld;
	DaAtomList shadScRingworldFog;

	// Render atoms in this solar system.
	// The renderLast map accumulates info about atoms that must be rendered in a final pass, usually due to blending effects.
	// The map is keyed by z-depth so the resulting contents are z-sorted, farthest to nearest.
	DaAtomList subs;
	ListSubatoms(subs);
	if(s_SystemFarAtoms)
	{
		// Render distant objects.
		// Far atoms are drawn with a different near clip distance, so they don't have Z collisions. Then we clear the Z buffer.
		AuRenderLastMap renderFarLast;
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderEnvironmentObject(farProjection, ari, rmod, sceneAtom, renderFarLast, PASS_Far, shadSc, shadScFog,
					shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_Far);

		// RenderLast worlds.
		// Render worlds that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
		if(s_SystemFarLast)
		{
			for(AuRenderLastMap::iterator flri = renderFarLast.begin(); flri != renderFarLast.end(); flri++)
				if(dynamic_cast<DaWorldRing*>((*flri).second))
					(*flri).second->RenderLast(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
							shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_FarBlend);
		}

		// Render warp tunnel effect or nebula cloud around scene atom before RenderLast objects.
		// Find scene-atom's top-most spaceship.
		if(s_SystemWarp)
		{
			DaVessel* ship = sceneAtom->Parent()->FindParentVesselInEnv();
			while(ship && ship->Parent()->FindParentVesselInEnv())
				ship = ship->Parent()->FindParentVesselInEnv();
			if(ship)
				ship->RenderWarpOrNebula(farProjection, rmod.Matrix(ship->Id()), ari);
			else
			{
				DaVehicle* vehicle = dynamic_cast<DaVehicle*>(sceneAtom->Parent());
				if(vehicle)
					vehicle->RenderWarpOrNebula(farProjection, rmod.Matrix(vehicle->Id()), ari);
				else
					sceneAtom->RenderWarpOrNebula(farProjection, model, ari);
			}
		}

		// Render non-worlds that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
		if(s_SystemFarLast)
		{
			for(AuRenderLastMap::iterator flri = renderFarLast.begin(); flri != renderFarLast.end(); flri++)
				if(!dynamic_cast<DaWorldRing*>((*flri).second))
					(*flri).second->RenderLast(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
							shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_FarBlend);
		}

		// Don't render wormholes unless the person is outside the atmosphere range of a planet.
		if(!sceneAtom->IsInAtmosphere())
			RenderWormholes(farProjection, tmodel, PASS_Far, *ari.re, ari.Models(), rmod);

		// Clear the depth buffer for the remainder of the scene rendering. The depth buffer is not compatible with the rendering
		// of near objects using a different near clip distance. The contents of the scene so far simple fall into the backdrop.
		ari.re->glClear(GL_DEPTH_BUFFER_BIT);
	}

	AuRenderLastMap renderLast;
	if(s_SystemAtoms)
	{
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_Near);
	}

	// Don't render wormholes unless the person is outside the atmosphere range of a planet.
	if(!sceneAtom->IsInAtmosphere())
		RenderWormholes(projection, tmodel, PASS_Near, *ari.re, ari.Models(), rmod);

	// Render atoms that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
	if(s_SystemLast)
	{
		for(AuRenderLastMap::iterator lri = renderLast.begin(); lri != renderLast.end(); lri++)
			(*lri).second->RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
					shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_NearBlend);
	}
}

void DaSolarSystem::RenderWormholes(const AuMatrix4x4& projection, const AuMatrix4x4& model, RENDERPASS pass, AuGLEffects& re,
		AuGLModelCache& mc, const AuModelMatrixHash& rmod)
{
	AuMatrix4x4 whmodel = model;
	if(HasBlackHole())
	{
		const DaStar* ps = FindPrimaryStarNotBlackHole();
		if(ps)
			whmodel.Translate(ps->Location());	// Move to companion "primary" star location.
	}

	// Render positive and intergalactic wormholes.
	bool negWorm = false;	// true if we find a negative wormhole
	for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end();it++)
		if((*it).Polarity() == AuWormhole::Positive)
		{
			AuPoint wloc = (whmodel * (*it).Transform()).Translation();
			if(!rmod.RenderThisPass(pass, wloc.Length()))
				continue;
			(*it).Render(projection, whmodel, re, mc);	// Positive wh offset to companion in black hole system.
		}
		else if((*it).IsIntergalactic())
		{
			AuPoint wloc = (whmodel * (*it).Transform()).Translation();
			if(!rmod.RenderThisPass(pass, wloc.Length()))
				continue;
			(*it).Render(projection, model, re, mc);		// Intergalactic wormhole not offset to companion star.
		}
		else
			negWorm = true;

	// Render negative wormholes.
	if(negWorm)
		for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end();it++)
			if((*it).Polarity() == AuWormhole::Negative)
			{
				AuPoint wloc = (whmodel * (*it).Transform()).Translation();
				if(!rmod.RenderThisPass(pass, wloc.Length()))
					continue;
				(*it).Render(projection, whmodel, re, mc);
			}
}

DaAtom *DaSolarSystem::FindClosestAtom(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		const AuMatrix4x4 &tModel) const
{
	DaAtom *pClosest = DaAtom::FindClosestAtom(pxFind, tDevicePersp, tDeviceOrtho, tModel);

	AuPoint fpxFind(pxFind);
	double rClosest = -1.0;
	if (pClosest)
	{
		AuPoint fLoc = tModel.Project(pClosest->Location());
		rClosest = tDevicePersp.Project(fLoc).Distance_2D(fpxFind);
	}

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* pAtom = dynamic_cast<DaStar*>(*it);
		if(!pAtom)
			continue;

		AuMatrix4x4 tSub = tModel * pAtom->Transform();
		DaAtom *pSubSub = pAtom->FindClosestAtom(pxFind, tDevicePersp, tDeviceOrtho, tSub);
		if (!pSubSub)
			continue;

		AuPoint fLoc = tSub.Project(pSubSub->Location());
		AuPoint fpxLocOrtho = tDeviceOrtho.Project(fLoc);
		if (fpxLocOrtho.z >= 0.0)
			continue;	// Atom is behind the camera.

		AuPoint fpxLoc = tDevicePersp.Project(fLoc);
		double rdist = fpxLoc.Distance_2D(fpxFind);
		if (rClosest == -1.0 || rdist < rClosest)
		{
			rClosest = rdist;
			pClosest = pSubSub;
		}
	}
	return(pClosest);
}

// List atoms that should be in the scene when this atom is the scene atom.
void DaSolarSystem::ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	if(pSceneAtom->FindParentSolarSystem() == this || pSceneAtom->IsMonitoringSystem(Id()))
	{
		// The scene atom is in this solar system.
		// OR
		// The scene atom is monitoring this solar system.
		DaAtom::ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
	}
}

// Returns false if a known component of the starfield is missing. i.e. a sector isn't loaded yet that we need.
bool DaSolarSystem::ListStarFieldSubatoms(const DaAtom* pSceneAtom, DaAtomList& sceneAtoms) const
{
	Q_UNUSED(pSceneAtom);
	Q_UNUSED(sceneAtoms);
	// Do not look below the solar system level when listing star field atoms.
	return true;
}

void DaSolarSystem::ListLocationAndWormholes(DqAtomLocArray &aSystems, const AuPoint &fOffset, AuSegmentArray& exploredWormholes,
		AuSegmentArray& knownWormholes, quint32 empire)
{
	// Add my offset location to the solar systems array.
	DqAtomLoc al;
	al.m_fLocation = Location() + fOffset;
	al.m_pA = this;
	aSystems.append(al);

	// Add my "explored" and "known" wormhole segments to the wormholes arrays.
	if(empire)
	{
		bool sysExplored = IsExploredBy(empire);
		m_WormholesLock.lockForRead();
		for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend();it++)
		{
			if((*it).IsIntergalactic())
				continue;
			if((*it).IsExploredBy(empire))
			{
				AuSegment seg(al.m_fLocation, al.m_fLocation + (*it).SectorVecToDestination());
				exploredWormholes.append(seg);
			}
			else if(sysExplored)
			{
				AuSegment seg(al.m_fLocation, al.m_fLocation + (*it).SectorVecToDestination());
				knownWormholes.append(seg);
			}
		}
		m_WormholesLock.unlock();
	}
}

bool DaSolarSystem::IsWormholeAt(const AuPoint& loc) const
{
	//if(IsCoreBlackHole())
	//{
		// Use a large search radius because of the moving wormholes.
		m_WormholesLock.lockForRead();
		for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
			if((*it).Location(this).Distance2(loc) < 2500.*2500.)
			{
				m_WormholesLock.unlock();
				return true;
			}
		m_WormholesLock.unlock();
	//}
	//else
	//{
		//for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
			//if((*it).Location(this).FuzzyEquals(loc))
				//return true;
	//}
	return false;
}

AuWormhole DaSolarSystem::FindWormhole(const AuPoint& systemLoc) const
{
	//if(IsCoreBlackHole())
	//{
		// Use a large search radius because of the moving wormholes.
		m_WormholesLock.lockForRead();
		for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
			if((*it).Location(this).Distance2(systemLoc) < 2500.*2500.)
			{
				m_WormholesLock.unlock();
				return *it;
			}
		m_WormholesLock.unlock();
	//}
	//else
	//{
		//for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
			//if((*it).Location(this).FuzzyEquals(systemLoc))
				//return *it;
	//}
	return AuWormhole();	// Invalid wormhole.
}

QString DaSolarSystem::WormholeName(const AuWormhole& wh) const
{
	AuEther* ether = Ether();
	if(!ether)
		return "";
	if(!wh.IsValid())
		return "";

	// Determine the name of the destination solar system.
	QString destName;
	// All galaxies are rarely available when this code is called, i.e. on the client.
	if(wh.IsIntergalactic())
	{
		// Locate the wormhole in the galaxy map of wormhole connections based on its source galaxy and direction.
		const DaGalaxy* galaxy = FindParentGalaxy();
		if(!galaxy)
			return "";
		for(int gcon=0; gcon<s_GConCount; gcon++)
		{
			if(s_GConns[gcon].m_FromGalaxy == galaxy->Galaxy())
			{
				if(s_GConns[gcon].m_VecToDest == wh.SectorVecToDestination())
				{
					destName = GalaxyName(s_GConns[gcon].m_ToGalaxy);
					break;
				}
			}
		}
	}
	else
	{
		// pdest could be a solar system or a galaxy
		const DaAtom* pdest = ether->FindNormalAtom(wh.ToSystem());
		if(pdest)
			destName = pdest->Name();
	}

	// Determine the sign of the wormhole.
	QString sign;
	if(wh.Polarity() == AuWormhole::Positive)
		sign = "(+)";
	else if(wh.Polarity() == AuWormhole::Negative)
		sign = "(-)";
	else
		sign = "(=)";

	if(wh.HasName())
		return QString("%1 %2 %3").arg(wh.Name()).arg(sign).arg(destName);
	else
		return tr("Wormhole %1 %2").arg(sign).arg(destName);
}

QString DaSolarSystem::WormholeNameAt(const AuPoint& loc) const
{
	AuWormhole wh = FindWormhole(loc);
	return WormholeName(wh);
}

void DaSolarSystem::AddWormholeConnection(const AuId &idSolarSystem, const AuPoint &vToThere, AuWormhole::POLARITY polarity)
{
	// Check to see if we already have a wormhole connected to that destination.
	if(FindWormhole(idSolarSystem, polarity).IsValid())
		return;

	AuWormhole wh("", polarity, idSolarSystem, vToThere);
	if(polarity == AuWormhole::Intergalactic || polarity == AuWormhole::IntergalacticOneWay)
	{
		const DaStar* ps = FindPrimaryStar();
		if(ps)
		{
			float rdist = ps->IntergalacticWormholeDistance();
			wh.SetStarDistance(rdist, Rotation());
		}
	}
	else
	{
		const DaStar* ps = FindPrimaryStarNotBlackHole();
		if(ps)
		{
			float rdist;
			if(polarity == AuWormhole::Positive)
				rdist = ps->Radius() * 2.f;
			else
				rdist = ps->Radius() * 2.f * 1.1f ;
			wh.SetStarDistance(rdist, Rotation());
		}
	}
	m_WormholesLock.lockForWrite();
	m_Wormholes.append(wh);
	m_WormholesLock.unlock();
}

AuWormhole DaSolarSystem::FindClosestWormhole(const AuPoint& systemLoc) const
{
	AuWormhole closestWorm;		// Invalid until set
	double closestDist = -1;	// Negative until set. Will be the square of the distance to the closest wormhole.
	m_WormholesLock.lockForRead();
	for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
	{
		double dist = (*it).Location(this).Distance2(systemLoc);	// distance squared
		if(closestDist < 0 || dist < closestDist)
		{
			closestWorm = *it;
			closestDist = dist;
		}
	}
	m_WormholesLock.unlock();
	return closestWorm;
}

// Find wormhole pointing in a specific destination direction.
AuWormhole DaSolarSystem::FindWormholeDir(const AuPoint &dir, AuWormhole::POLARITY polarity) const
{
	m_WormholesLock.lockForRead();
	for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
	{
		if((*it).Polarity() == polarity)
		{
			AuPoint wdir = (*it).SystemVecToDestination();
			wdir.Unit();
			if(wdir.FuzzyEquals(dir))
			{
				AuWormhole w(*it);
				m_WormholesLock.unlock();
				return w;
			}
		}
	}
	m_WormholesLock.unlock();
	return AuWormhole();	// Invalid wormhole.
}

// Returns the wormhole in this system that connects 'system' with a 'polarity'.
AuWormhole DaSolarSystem::FindWormhole(const AuId& system, AuWormhole::POLARITY polarity) const
{
	m_WormholesLock.lockForRead();
	for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
		if((*it).Polarity() == polarity && (*it).ToSystem() == system)
		{
			AuWormhole w(*it);
			m_WormholesLock.unlock();
			return w;
		}
	m_WormholesLock.unlock();
	return AuWormhole();	// Invalid wormhole.
}

// Flag the wormhole as "explored" when a spacecraft moves through it for the first time.
void DaSolarSystem::WormholeExplored(const AuWormhole& wh, DaVessel* ship, bool isEmerging)
{
	// The allegiance of the ship determines which empire now "knows" about the wormhole.
	quint32 empire = ship->EmpireId();
	if(!empire)
		return;	// Not really possible.

	if(wh.IsExploredBy(empire) && wh.HasName())
		return;	// Already explored and named so don't change anything.

	// Determine the nu name of the wormhole if it hasn't been named yet.
	// The captain of the ship gets the glory of naming the wormhole.
	QString exploredName;
	if(!wh.HasName() && !ship->CaptainId().IsNull())
	{
		// The captain or an officer only gets to rename the wormhole if they are aboard the ship and on line.
		// Locate an officer aboard the ship who is on line, preferably the captain.
		DaPerson* officer = ship->FindOfficerOnLineRecursive();
		if(officer)
		{
			int seq;
			if(isEmerging)
				seq = officer->WormholesExplored();
			else
				seq = officer->ExploredWormhole();
			exploredName = QString("%1 %2").arg(officer->Name()).arg(seq);
		}
		//else
			//qDebug("DaSolarSystem::WormholeExplored System %d No player gets to name the wormhole.", Id().GetID());
	}

	// Find the actual wormhole data in my list of wormholes that corresponds to the one passed.
	if(WormholeExplored(wh.ToSystem(), wh.Polarity(), empire, exploredName))
	{
		// On entry, reward the ship's crew with its empire's posted reward for discovering a wormhole.
		if(!isEmerging)
			ship->RewardShip(1, REW_ExploreWormhole, 0);

		// Kick database to save the changes to this solar system.
		//Q_ASSERT(g_Au.IsSceneWorker());
		SetModified();

		// Exploration of wormholes is propagated to all servers so they
		// can maintain the explored status of wormholes in their star fields.
		emit DaAtom::WormholeExplored(Id(), wh.ToSystem(), wh.Polarity(), empire, exploredName);
	}
	//else
		//qDebug("DaSolarSystem::WormholeExplored System %d The 'explored' wormhole was not found.", Id().GetID());
}

// Returns true if the wormhole is actually changed.
bool DaSolarSystem::WormholeExplored(const AuId& toSystem, AuWormhole::POLARITY polarity, quint32 empire, const QString& nuName)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug("DaSolarSystem::WormholeExplored toSystem %d polarity %d empire %d nuName %s wormholes %d this id %d",
				toSystem.GetID(), polarity, empire, (const char*)nuName.toLatin1(), m_Wormholes.count(), Id().GetID());
	// Find the identified wormhole data in my list of wormholes.
	m_WormholesLock.lockForWrite();
	for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end(); it++)
	{
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaSolarSystem::WormholeExplored test wormhole polarity %d toSystem %d",
					(*it).Polarity(), (*it).ToSystem().GetID());
		if((*it).Polarity() == polarity && (*it).ToSystem() == toSystem)
		{
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug("DaSolarSystem::WormholeExplored found wormhole");
			if(!(*it).IsExploredBy(empire) || !(*it).HasName())
			{
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug("DaSolarSystem::WormholeExplored explore wormhole");
				(*it).ExploredBy(empire, nuName);
				m_WormholesLock.unlock();
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug("DaSolarSystem::WormholeExplored explore wormhole done");
				return true;
			}
			break;
		}
	}
	m_WormholesLock.unlock();
	return false;
}

// Set the location of the wormholes based on the primary star size.
// NOTE: This is called in DtDb.
bool DaSolarSystem::PositionWormholes()
{
	if(IsEmptySystem())
	{
		m_WormholesPositioned = true;
		return false;
	}

	DaStar* ps = FindPrimaryStarNotBlackHole();
	if(!ps)
		return false;
	m_WormholesPositioned = true;

	// Distances for interstellar wormholes.
	const float rWormOrgDist = ps->Radius() * 2.f;
	const float rWormDestDist = rWormOrgDist * 1.1f;

	DaStar* blackHole = FindPrimaryStar();					// In a black hole system, this will be the black hole.
	const float rWormIntDist = blackHole->IntergalacticWormholeDistance();

	bool anyChange = false;
	if(IsCoreBlackHole())
	{
		// We expect to find intergalactic wormholes here.
		Q_ASSERT(blackHole);
		bool anyInt = false;
		m_WormholesLock.lockForWrite();
		for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end();it++)
			if((*it).IsIntergalactic())
			{
				if((*it).SetStarDistance(rWormIntDist, Rotation()))
					anyChange = true;
				anyInt = true;
			}
			else if((*it).Polarity() == AuWormhole::Positive)
			{
				if((*it).SetStarDistance(rWormOrgDist, Rotation()))
					anyChange = true;
			}
			else
			{
				if((*it).SetStarDistance(rWormDestDist, Rotation()))
					anyChange = true;
			}
		m_WormholesLock.unlock();

		// If we find any intergalactic wormholes then we're done.
		if(anyInt)
			return anyChange;

		// This is a core black hole system. There needs to be intergalactic wormholes here.
		const DaGalaxy* galaxy = FindParentGalaxy();
		if(!galaxy)
			return anyChange;
		const DaUniverse* universe = galaxy->FindParentUniverse();
		if(!universe)
			return anyChange;

		for(int gcon=0; gcon<s_GConCount; gcon++)
		{
			if(s_GConns[gcon].m_OneWayOnly)
				continue;	// One-way only connections do not have wormholes at the core.
			if(s_GConns[gcon].m_FromGalaxy == galaxy->Galaxy())
			{
				// Found a connection from my current galaxy.
				const DaGalaxy* destGalaxy = universe->FindGalaxy(s_GConns[gcon].m_ToGalaxy);
				AddWormholeConnection(destGalaxy->Id(), s_GConns[gcon].m_VecToDest, AuWormhole::Intergalactic);
				anyChange = true;
			}
		}
	}
	else if(blackHole && blackHole != ps && blackHole->IsStellarBlackHole())
	{
		// System with a stellar black hole will have one intergalactic wormhole that goes only one way to another galaxy.
		bool anyInt = false;
		m_WormholesLock.lockForWrite();
		for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end();it++)
			if((*it).IsIntergalactic())
			{
				if((*it).SetStarDistance(rWormIntDist, Rotation()))
					anyChange = true;
				anyInt = true;
			}
			else if((*it).Polarity() == AuWormhole::Positive)
			{
				if((*it).SetStarDistance(rWormOrgDist, Rotation()))
					anyChange = true;
			}
			else
			{
				if((*it).SetStarDistance(rWormDestDist, Rotation()))
					anyChange = true;
			}
		m_WormholesLock.unlock();

		// If we find any intergalactic wormholes then we're done.
		if(anyInt)
			return anyChange;

		// This is a black hole system. There needs to be a one-way intergalactic wormhole here.
		const DaGalaxy* galaxy = FindParentGalaxy();
		if(!galaxy)
			return anyChange;
		const DaUniverse* universe = galaxy->FindParentUniverse();
		if(!universe)
			return anyChange;

		// Tally intergalactic wormholes.
		QList<int> holeidxs;
		for(int gcon=0; gcon<s_GConCount; gcon++)
			if(s_GConns[gcon].m_FromGalaxy == galaxy->Galaxy())
				holeidxs.append(gcon);	// Found a connection from my current galaxy.
		if(holeidxs.empty())
			return anyChange;	// No intergalactic wormholes depart from my galaxy.
		int gcon = GenSeed() % holeidxs.size();	// Choose one based on my genseed.
		int didx = holeidxs[gcon];	// Index of galaxy connection.
		const DaGalaxy* destGalaxy = universe->FindGalaxy(s_GConns[didx].m_ToGalaxy);
		AddWormholeConnection(destGalaxy->Id(), s_GConns[didx].m_VecToDest, AuWormhole::IntergalacticOneWay);
		anyChange = true;
	}
	else
	{
		m_WormholesLock.lockForWrite();
		for(AuWormholeArray::iterator it = m_Wormholes.begin(); it != m_Wormholes.end();it++)
			if((*it).Polarity() == AuWormhole::Positive)
			{
				if((*it).SetStarDistance(rWormOrgDist, Rotation()))
					anyChange = true;
			}
			else
			{
				if((*it).SetStarDistance(rWormDestDist, Rotation()))
					anyChange = true;
			}
		m_WormholesLock.unlock();
	}
	return anyChange;
}

AuWormhole DaSolarSystem::CheckWormholeEntry(const AuPoint& oldloc, const AuPoint& nuloc, float& entryAngle) const
{
	m_WormholesLock.lockForRead();
	for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin(); it != m_Wormholes.cend(); it++)
	{
		float angle = (*it).CheckEntry(this, oldloc, nuloc);
		if(angle >= 0)
		{
			AuWormhole w(*it);
			m_WormholesLock.unlock();
			entryAngle = angle;
			return w;
		}
	}
	m_WormholesLock.unlock();
	entryAngle = -1;
	return AuWormhole();	// Invalid return object.
}

// Returns true if 'loc' is in the domain of this solar system atom.
// false if 'loc' is more than a sector away.
// false if CheckChildExit says no.
bool DaSolarSystem::IsInSystem(const AuPoint& loc) const
{
	// false if 'loc' is more than a sector away.
	if(fabs(loc.x) > FeetPerSector()*2 ||
			fabs(loc.y) > FeetPerSector()*2 ||
			fabs(loc.z) > FeetPerSector()*2)
		return false;

	// CheckChildExit returns true when 'loc' is NOT in this system; it returns false when 'loc' IS in this system.
	return !CheckChildExit(AuPoint(0,0), loc, 0);
}

// Returns true if a child moving to 'fnuloc' from 'foldloc' departs the influence of this atom.
bool DaSolarSystem::CheckChildExit(const AuPoint& foldloc, const AuPoint &fnuloc, const DaAtom* who) const
{
	// Ships in tow are managed by their parent so they don't get separated.
	const DaVessel* ship = dynamic_cast<const DaVessel*>(who);
	if(ship && ship->IsInTow())
		return false;

	Q_UNUSED(foldloc);
	Q_ASSERT(Parent());
	const DaSector* ps = (const DaSector*)Parent();
	if(!ps)
		return false;
	const DaGalaxy* galaxy = ps->FindParentGalaxy();
	if(!galaxy)
		return false;

	// Convert the nu system-relative location into a galaxy location, in parsecs.
	AuMatrix4x4 sysRot;
	sysRot.Rotate(Rotation());
	AuPoint galaxyLoc = Location() + sysRot.Project(fnuloc) / FeetPerParsec();	// sector location
	galaxyLoc += ps->Location();		// Convert from relative to my sector to relative to the galaxy.

	// Locate the closest solar system to the destination location if there is one. This test prevents us from moving into
	// the perimeter sectors that contain no solar systems; the test finds the sector that contains the loc and then returns its
	// closest solar system to that location.
	const DaSolarSystem* pcss = galaxy->FindClosestSolarSystemInSector(galaxyLoc);
	if(!pcss || pcss == this)
		return false;					// Still closest to this solar system.

	if(ship && ship->HasWarpFTLDrive() && ship->IsFTLDriveHot(ship->FTLWarpDrive()))
	{
		// A ship traveling at warp speed is not released until it leaves the sector. This reduces system transitions.
		if(pcss->ParentId() == ParentId())
			return false;				// Ship is still in my sector.
	}

	return true;
}

// Returns true if movement from foldloc to fnuloc would enter the area of influence of this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaSolarSystem::CheckPeerJoin(const AuPoint& /*foldloc*/, const AuPoint &fnuloc, const DaAtom*)
{
	const DaGalaxy* galaxy = FindParentGalaxy();
	if(!galaxy)
		return false;

	// Convert the nu sector-relative location into a galaxy location, in parsecs.
	AuPoint galaxyLoc = Location() + fnuloc / FeetPerParsec();

	// Locate the closest solar system to the destination location if there is one. This test prevents us from moving into
	// the perimeter sectors that contain no solar systems; the test finds the sector that contains the loc and then returns its
	// closest solar system to that location.
	const DaSolarSystem* pcss = galaxy->FindClosestSolarSystemInSector(galaxyLoc);
	if(!pcss)
		return false;					// Prohibit movement into sectors with no solar systems.

	if(pcss == this)
		return true;					// Closest to this solar system so enter.

	return false;
}

bool DaSolarSystem::GetTransformRelativeToParent(AuMatrix4x4 &tModel, const DaAtom *pParent) const
{
	if(Is(pParent->Id()))
	{
		// "I" am the desired parent atom. Return the model projection transformation as is.
		return true;
	}
	const DaAtom* parent = Parent();
	if(!parent)
		return false;	// Didn't find the real parent.

	// "I" am not the desired parent so prepend my transformation on the one passed and work my way up the chain.
	AuMatrix4x4 tSectorScaling;
	tSectorScaling.Scale(1.0/FeetPerParsec(), 1.0/FeetPerParsec(), 1.0/FeetPerParsec());
	tModel = Transform() * tSectorScaling * tModel;
	return parent->GetTransformRelativeToParent(tModel, pParent);
}

bool DaSolarSystem::MoveSubatom(DaAtom *pMove, const AuPoint &vMove, int /*elapsedMs*/)
{
	// Do not move subatoms when I'm only loaded for the starfield.
	if(IsOnlyStarFieldLoaded())
		return true;

	// Only the server checks for wormhole emergence when moving in a solar system.
	if(!g_Au.IsSceneClient())
	{
		// When a spacecraft is at exactly 1 or 2 units from the center, it is emerging from a wormhole.
		DaVessel* ship = dynamic_cast<DaVessel*>(pMove);
		if(ship && ship->EmergeFromWormhole(this))
			return true;
	}

	// Determine the destination point of the move.
	AuPoint foldloc(pMove->Location());
	AuPoint fnuloc = foldloc + vMove;

#if !defined(QT_NO_DEBUG)
	if(fnuloc.IsNaN())
	{
		qDebug() << "DaSolarSystem::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			<< "fnuloc" << fnuloc << fnuloc.IsNaN() << "vMove" << vMove << vMove.IsNaN() << "IsNaN ERROR!";
	}
#endif

	// Determine the result of attempting to move to the nu location.
	DaAtom *pHit = NULL;
	AuPoint hitLoc, hitNormal;
	COLLISION_TYPE eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitLoc, hitNormal);
	pMove->SetMoveResult(eCollide);
	/*
	if(pMove->Name() == "AtlatlTest")
	{
		qDebug() << "DaSolarSystem::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			<< "collide" << eCollide << "oldloc" << foldloc << "nuloc" << fnuloc;
		if(pHit)
			qDebug() << " hit" << pHit->Id() << pHit->Name() << "hitNormal" << hitNormal;
	}
	*/
	switch (eCollide)
	{
	case CT_HitParent:
//		qDebug("Hit parent %s", pHit->Name());
		return false;
	case CT_HitPeer:
	case CT_HitPeerShield:
//		qDebug("Hit Peer %s", pHit->Name());
		{
			// Reflect its movement vector.
			// Calculate surface normal relative to the hit atom's parent, which is this atom.
			hitNormal = pHit->Transform().ProjectVector(hitNormal);

			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			hitNormal.Unit();
			if(!FuzzyEquals(hitNormal.Length(), 1))
				hitNormal = AuPoint(0,0,1);
			pMove->MovementBlocked(pHit, hitLoc, hitNormal, eCollide == CT_HitPeerShield);
		}
		return false;
	case CT_JoinedPeer:
		// Move the atom subordinate to its nu parent.
		if(pHit)
		{
			//qDebug("DaSolarSystem::MoveSubatom - Entered %s",
					//(const char *)pHit->Name().toAscii());
			// Change the world transformation to be subordinate to the nu parent atom.
			AuMatrix4x4 tBody;
			tBody.Translate(fnuloc);
			tBody.Rotate(pMove->Rotation());
			AuMatrix4x4 t = pHit->Transform().Inverse() * tBody;

			// Combine the movement vectors.
			AuPoint nuMove(pMove->Vector());
			if(!pHit->Vector().IsZero())
				nuMove -= pMove->Transform().UnProjectVector(pHit->Vector());
			pMove->ForcePosMove(AuAtomPos(pHit->Id(), t), nuMove);
			pMove->SetModified();
			pMove->InvalidateEnvironment();	// Refresh environment on atom when it moves, in this thread.
		}
		return true;
	case CT_Miss:
		{
			// The moving atom did not hit anything.
			// Move the atom to the nu location relative to 'this' atom.
			// Determine a vector for movement.
			AuAtomPos savePos(pMove->Pos());
			if(!vMove.IsZero())
			{
				AuAtomPos pos(savePos);
				AuPoint fmove = pos.Transform().Inverse().ProjectVector(vMove);
				AuMatrix4x4 t;
				t.Translate(fmove);
				pos.TransformBy(t);
				pMove->ChangePos(pos);
				pMove->InvalidateEnvironmentRecursive();
			}

			// Check to see if the nu location is outside the scope of influence of this atom.
			if(CheckChildExit(foldloc, fnuloc, pMove))
			{
				//qDebug() << "DaSolarSystem::MoveSubatom" << Id() << Name() << "move" << pMove->Id() << pMove->Name() << "exited";
				if(!DepartedOnDeadHeading(pMove, fnuloc))
				{
					// The atom has reached the barrier at the edge of the galaxy or it was destroyed.
					// Restore atom's previous position and stop it from moving.
					if(pMove->IsNormal())
					{
						pMove->ForcePosStop(savePos);
						pMove->InvalidateEnvironmentRecursive();
					}
				}
			}
			else
				pMove->InvalidateEnvironment();	// Refresh environment on atom when it moves, in this thread.
			pMove->SetModified();
			return true;
		}
		return false;
	}
	return false;
}

// Insert first time database record.
void DaSolarSystem::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_SolarSystem (id, idp, locX,locY,locZ, rotX,rotY,rotZ, "
				"timeCreate, timeModified, timeTock, timeDeath, "
				"name,wormholes,exploredBy,scannedBy,lastActiveLand,lastVisit,lastOfficerVisit, "
				"genHab,genSeed,lastPirateStation,warpSignature,final) "
				"VALUES (?, ?, ?,?,?, ?,?,?, ?, ?, ?, ?, ?,?,?,?,?,?,?, ?,?,?,?,?);");
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
	qa.addBindValue(m_sName);

	{
		// Wormholes
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_Wormholes >> out;
		qa.addBindValue(ba);
	}

	{
		// Explored-by array of empire ids
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_ExploredByLock.lockForRead();
		out << m_ExploredBy;
		m_ExploredByLock.unlock();
		qa.addBindValue(ba);
	}

	{
		// Scanned-by array of empire ids
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_ScannedByLock.lockForRead();
		out << m_ScannedBy;
		m_ScannedByLock.unlock();
		qa.addBindValue(ba);
	}

	qa.addBindValue(m_LastAvatarAccountActive.UTC());
	qa.addBindValue(m_LastPersonVisit.UTC());
	qa.addBindValue(m_LastOfficerVisit.UTC());
	qa.addBindValue(m_GenHab);
	qa.addBindValue((qint32)m_GenSeed);
	qa.addBindValue(m_LastFlagInfluence.UTC());	// Used to be m_LastPirateStation.
	qa.addBindValue((qint32)m_WarpSignature);
	qa.addBindValue(m_Final);

	if(!qa.exec())
		qWarning("DaSolarSystem::Insert %s FAILED.", (const char*)Name().toLatin1());
}

void DaSolarSystem::StoredInDb()
{
	DaAtom::StoredInDb();
	ResetWormholesFinalized();
}

// Update existing database record.
void DaSolarSystem::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	bool isMaster = g_Au.IsSceneMaster();
	bool isWorker = g_Au.IsSceneWorker();
	bool wormsFinalized = m_WormholesFinalized;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	// Master server only updates the 'detectedBy' field. It sometimes updates wormholes, when they get finalized.
	// Worker server never updates the 'detectedBy' field. It often updates wormholes, when they get explored.
	QSqlQuery qa(QString(), db);
	if(isMaster && wormsFinalized)
	{
		sq = QString("UPDATE a_SolarSystem SET wormholes=?, detectedBy=? WHERE id=%1;").arg(Id().GetID());
		qa.prepare(sq);

		{
			// Wormholes
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			m_Wormholes >> out;
			qa.addBindValue(ba);
		}

		{
			// Detected-by
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint32)m_DetectedBy.size();
			for(unordered_map<quint32, AuSysDet_>::const_iterator di = m_DetectedBy.begin(); di != m_DetectedBy.end(); di++)
			{
				out << (*di).first;
				out << (*di).second;
			}
			qa.addBindValue(ba);
		}
	}
	else if(isMaster)
	{
		sq = QString("UPDATE a_SolarSystem SET detectedBy=? WHERE id=%1;").arg(Id().GetID());
		qa.prepare(sq);

		{
			// Detected-by
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint32)m_DetectedBy.size();
			for(unordered_map<quint32, AuSysDet_>::const_iterator di = m_DetectedBy.begin(); di != m_DetectedBy.end(); di++)
			{
				out << (*di).first;
				out << (*di).second;
			}
			qa.addBindValue(ba);
		}
	}
	else
	{
		// Non-master server never updates the 'detectedBy' field. They don't even gather the information.
		// Only the worker server updates the henge dest. It is the only server to collect that information.
		QString hengeDestField;
		if(isWorker)
			hengeDestField = ", hengeDest=?";

		sq = QString("UPDATE a_SolarSystem SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
				"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, name=?, "
				"wormholes=?, exploredBy=?, scannedBy=?, lastActiveLand=?, lastVisit=?, lastOfficerVisit=?, genHab=?, genSeed=?, "
				"lastPirateStation=?, warpSignature=?, final=? %2 "
				"WHERE id=%1;").arg(Id().GetID()).arg(hengeDestField);
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
		qa.addBindValue(m_sName);

		{
			// Wormholes
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			m_Wormholes >> out;
			qa.addBindValue(ba);
		}

		{
			// Explored-by array of empire ids
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			m_ExploredByLock.lockForRead();
			out << m_ExploredBy;
			m_ExploredByLock.unlock();
			qa.addBindValue(ba);
		}

		{
			// Scanned-by array of empire ids
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			m_ScannedByLock.lockForRead();
			out << m_ScannedBy;
			m_ScannedByLock.unlock();
			qa.addBindValue(ba);
		}

		qa.addBindValue(m_LastAvatarAccountActive.UTC());
		qa.addBindValue(m_LastPersonVisit.UTC());
		qa.addBindValue(m_LastOfficerVisit.UTC());
		qa.addBindValue(m_GenHab);
		qa.addBindValue((qint32)m_GenSeed);
		qa.addBindValue(m_LastFlagInfluence.UTC());	// Used to be m_LastPirateStation.
		qa.addBindValue((qint32)m_WarpSignature);
		qa.addBindValue(m_Final);

		if(isWorker)
		{
			// Henge destinations cache.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			m_HengeDestLock.lock();
			out << m_HengeDest;
			m_HengeDestLock.unlock();
			qa.addBindValue(ba);
		}
	}

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning() << "error: DaSolarSystem::Update" << Id() << Name() << sq << err
			<< "genHab" << m_GenHab
			<< "genSeed" << m_GenSeed
			<< "warpSignature" << m_WarpSignature;
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaSolarSystem::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaSolarSystem* DaSolarSystem::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
		const AuId& idToLoad)
{
	bool isMaster = g_Au.IsSceneMaster();
	bool isWorker = g_Au.IsSceneWorker();
	QString sq;
	if(isMaster)
		sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, name,wormholes,exploredBy,scannedBy,lastActiveLand,lastVisit,"
				"lastOfficerVisit,"
				"genHab,genSeed,lastPirateStation,warpSignature,final,detectedBy "	// Only master loads detectedBy field.
				"FROM a_SolarSystem WHERE ");
	else if(isWorker)
		sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, name,wormholes,exploredBy,scannedBy,lastActiveLand,lastVisit,"
				"lastOfficerVisit,"
				"genHab,genSeed,lastPirateStation,warpSignature,final,hengeDest "
				"FROM a_SolarSystem WHERE ");
	else
		sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, name,wormholes,exploredBy,scannedBy,lastActiveLand,lastVisit,"
				"lastOfficerVisit,"
				"genHab,genSeed,lastPirateStation,warpSignature,final "
				"FROM a_SolarSystem WHERE ");
	if(!idToLoad.IsNull())
		sq += " id=" + QVariant(idToLoad.GetID()).toString();
	else
		sq += " idp=" + QVariant(parentId.GetID()).toString();
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaSolarSystem *pnu = new DaSolarSystem;
		if(pnu)
		{
			// DaAtom values.
			int qv = 0;
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

			// DaSolarSystem values.
			pnu->m_sName = qa.value(qv++).toString();

			pnu->m_isOnlyStarFieldLoaded = (loadWhat & LOAD_StarField) ? true : false;
			//if(!pnu->m_isOnlyStarFieldLoaded)
				//qDebug("DaSolarSystem::LoadSubatoms %d %s loaded, not a starfield system", pnu->Id().GetID(),
						//(const char*)pnu->Name().toAscii());

			// Load wormhole data.
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Wormholes << in;
			}

			// Load explored-by data.
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_ExploredBy;
			}

			// Load scanned-by data.
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->m_ScannedBy;
			}

			QDateTime lv = qa.value(qv++).toDateTime();
			lv.setTimeSpec(Qt::UTC);
			if(lv > now)
				lv = now;
			pnu->m_LastAvatarAccountActive = lv;

			lv = qa.value(qv++).toDateTime();
			lv.setTimeSpec(Qt::UTC);
			if(lv > now)
				lv = now;
			pnu->m_LastPersonVisit = lv;

			lv = qa.value(qv++).toDateTime();
			lv.setTimeSpec(Qt::UTC);
			if(lv > now)
				lv = now;
			pnu->m_LastOfficerVisit = lv;

			pnu->m_GenHab = (HABITABILITY)qa.value(qv++).toUInt();
			if(pnu->m_GenHab != HABITABLE_System &&
					pnu->m_GenHab != HABITABLE_HomeWorld &&
					pnu->m_GenHab != HABITABLE_EmptySystem)
				pnu->m_GenHab = HABITABLE_System;				// There's a lot more of these than HomeWorld systems.
			pnu->m_GenSeed = qa.value(qv++).toUInt();

			lv = qa.value(qv++).toDateTime();
			lv.setTimeSpec(Qt::UTC);
			if(lv > now)
				lv = now;
			pnu->m_LastFlagInfluence = lv;	// Used to be m_LastPirateStation.
			pnu->m_WarpSignature = qa.value(qv++).toUInt();
			pnu->m_Final = qa.value(qv++).toBool();

			if(isMaster)
			{
				// Load detected-by data.
				qvi = qv++;
				if(!qa.value(qvi).isNull())
				{
					QByteArray ba = qa.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_4_1);
					quint32 c;
					in >> c;
					while(c--)
					{
						quint32 empireId;
						AuSysDet_ det;
						in >> empireId;
						in >> det;
						pnu->m_DetectedBy.insert(pair<quint32, AuSysDet_>(empireId, det));
					}
				}
			}
			else if(isWorker)
			{
				int qvi = qv++;
				if(!qa.value(qvi).isNull())
				{
					QByteArray ba = qa.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_4_1);
					pnu->m_HengeDestLock.lock();
					in >> pnu->m_HengeDest;
					pnu->m_HengeDestLock.unlock();
				}
			}

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			if(loadWhat & LOAD_GamePieces)
			{
				DaStar::LoadSubatoms(ether, db, pnu->Id(), loadWhat);
				DaAsteroid::LoadSubatoms(ether, db, pnu->Id());
				DaSc::LoadSubatoms(ether, db, pnu->Id());
				DaVehicle::LoadSubatoms(ether, db, pnu->Id());
				DaPerson::LoadSubatoms(ether, db, pnu->Id());
				DaItem::LoadSubatoms(ether, db, pnu->Id());
				DaNpc::LoadSubatoms(ether, db, pnu->Id());

				// Update city connection information now that all of my subatoms are loaded.
				pnu->UpdateTradeConnections();
				pnu->CommoditySupplierUpdate();

				DaAtomList subs;
				pnu->ListSubatoms(subs);
				if(!subs.empty())
				{
					// Check for bad multiple star situation and delete the extra star(s).
					DaAtomList stars;
					for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
						if(dynamic_cast<DaStar*>(*it))
							stars.append(*it);
					if(stars.count() > 1)
					{
						// There are too many primary stars.
						qWarning() << "DaSolarSystem::LoadSubatoms" << pnu->Id() << pnu->Name()
							<< "Too many primary stars:" << stars.count();
						// Try removing those with no cities or people or spacecraft.
						DaAtomList::iterator si = stars.begin();
						while(si != stars.end() && stars.count() > 1)
						{
							DaStar* star = dynamic_cast<DaStar*>(*si);
							Q_ASSERT(star);
							if(star->FindPersonRecursive())
							{
								si++;
								continue;		// A person was found subordinate to the star.
							}

							DaAtomList bds;
							star->FindBdRecursive(bds);
							if(!bds.isEmpty())
							{
								si++;
								continue;		// A bd was found subordinate to the star.
							}

							DaAtomList ships;
							star->FindVesselsRecursive(ships);
							if(!ships.isEmpty())
							{
								si++;
								continue;		// A spaceship was found subordinate to the star.
							}

							// The star has nothing that prevents us from removing it.
							qWarning() << " DaSolarSystem::LoadSubatoms" << pnu->Id() << pnu->Name()
								<< "Killing primary star with no people, cities or ships." << star->Id() << star->Name();
							star->Kill();
							si = stars.erase(si);
						}
					}
				}
			}
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

void DaSolarSystem::SetNameIncludePlanets(const QString& sName, bool notifyDb)
{
	m_NameLock.lockForRead();
	if(m_sName == sName)
	{
		m_NameLock.unlock();
		return;
	}
	m_NameLock.unlock();

	m_NameLock.lockForWrite();
	QString oldName = m_sName;
	m_sName = sName;
	m_NameLock.unlock();

	SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->SolarSystemNameChanged(oldName, sName, notifyDb);
}

unsigned int DaSolarSystem::IsTocker() const
{
	if(IsOnlyStarFieldLoaded())
		return 0;
	if(!IsNormal())
		return 0;

	unsigned int ms = d_MSecPerDay;

	if(!m_WormholesPositioned)
		ms = 3 * d_MSecPerSec;

	if(ms > d_MSecPerMin)
	{
		m_EmpiresPresentLock.lockForRead();
		bool epres = !m_EmpiresPresent.empty();
		m_EmpiresPresentLock.unlock();
		if(epres)
			ms = d_MSecPerMin;
	}

	if(ms > d_MSecPerHr)
		if(m_WarpSignature)
			ms = d_MSecPerHr;
	return ms;
}

void DaSolarSystem::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	// Skip tocking solar systems that are loaded only for the star field.
	if(IsOnlyStarFieldLoaded())
		return;

	if(!m_WormholesPositioned)
		if(PositionWormholes())
		{
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
			SetModified();
		}

	// Decay highest quality non-pirate ship value once per day, so it has a way to go back down again. This is done before
	// tocking subatoms in case one of them wants to actually set the hightest Q ship value.
	if(!last.IsNull() && m_HighestQShip && last.ElapsedDayTicks(now) > 0)
		m_HighestQShip--;

	// Decay entries from the empires present list.
	m_EmpiresPresentLock.lockForWrite();
	if(!m_EmpiresPresent.empty())
	{
		unordered_map<quint32, AuElapsedTimer>::iterator epi = m_EmpiresPresent.begin();
		while(epi != m_EmpiresPresent.end())
		{
			if((*epi).second.ElapsedSecTicks() > 30)
			{
				//qDebug() << "DaSolarSystem::Tock" << *this << "empires present, empire" << (*epi).first << "no longer here";
				epi = m_EmpiresPresent.erase(epi);	// No refresh from that empire recently so forget about it.
			}
			else
				epi++;								// The empire entry is fresh enough to retain it.
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	m_EmpiresPresentLock.unlock();

	DaAtom::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	// Test for conditions that permit this solar system to decay back to a pristine empty state.
	if(last.ElapsedDayTicks(now) > 0)
	{
		// The day has changed.
		if(IsReadyToDecay())
		{
			// Prevent atoms from arriving or respawning here. This must be done immediately so somebody doesn't arrive here
			// before my signal reaches the appropriate handler in the app, who then calls my DecayContents function.
			SetStarFieldFlag(true);
			TickTockRecursive();

			emit SolarSystemReadyToDecay(Id());	// This solar system is ready for decay. Let the manager server do that.
		}
		else if(IsFinal() && HasHengeDestination())
		{
			// This system has been finalized and it has henge dest info in the cache. Create those henge destinations.
			CreateHengeDestinations();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// All solar systems must not broadcast this signal at the same time. It creates a huge lag spike.
	if(last.IsNewPeriod(now, EmissionPeriodMs(), Id().GetID()))
	{
		// It is time to emit my warp signature signal, if any.
		m_WarpSignatureLock.lock();
		quint32 w = m_WarpSignature;
		m_WarpSignature = 0;		// Reset warp signature value for fresh accumulation during the next period.
		m_WarpSignatureLock.unlock();
		if(w)
		{
			// Warp drive was engaged in this system within the last hour or two. Emit this as a signal that can be
			// detected by cities in other systems, to reveal the presence of warp drive activity here.
			if(g_Au.m_EnableSystemEmission)
			{
				const DaGalaxy* pg = FindParentGalaxy();
				if(pg)
				{
					AuPoint galaxyLoc;
					GetLocationRelativeToParent(galaxyLoc, pg);
					emit SystemWarpSignature(Id(), w, pg->Galaxy(), (float)galaxyLoc.x, (float)galaxyLoc.y, (float)galaxyLoc.z);
				}
			}
			//qDebug() << "DaSolarSystem::Tock" << Id() << Name() << "emitted" << p
				//<< "detectRange" << (p/DetectionRangeFactor()+1) << "pc" << last.UTC();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

// Cost of one unit of the item when the transaction occurs near a station in
// this solar system that is not in orbit around a planet.
quint32 DaSolarSystem::DemandCost(const AuItem& item) const
{
	// Determine solar system wide demand for the commodity.
	quint32 stock = 0;		// Stock of equal or higher quality of idCommodity.
	quint32 people = 0;	// The call picks up solar system wide population.
	quint32 needCount = 0;	// Count needed by all running processes, both manufacturing and construction.
	quint32 produceCount = 0;	// Count produced by all running mfg processes.
	ProcessDemandTally(item, stock, people, needCount, produceCount);
	//qDebug("DaSolarSystem::DemandCost cid %d qual %d qcost %g stock %d pop %d "
			//"need %d prod %d",
			//idCommodity, quality, qualCost, stock, people, needCount, produceCount);

	// The commodity calculates the demand.
	float demand = item.Demand(people, stock, needCount, produceCount);

	// Modify quality cost by demand.
	float cost = item.UnitCost() * demand;

	// Round UP.
	cost = ceilf(cost);
	if(cost < 1)
		cost = 1;

	return (quint32)cost;
}

void DaSolarSystem::ProcessDemandTally(const AuItem& item,
			unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* pAtom = dynamic_cast<DaStar*>(*it);
		if(!pAtom)
			continue;
		pAtom->ProcessDemandTally(item, stock, population, needCount, produceCount);
	}
}

void DaSolarSystem::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	/*
	// Can't scan wormholes because they don't have unique atom ids.
	for(AuWormholeArray::const_iterator it = m_Wormholes.cbegin();
			it != m_Wormholes.cend(); it++)
	{
		DqSensorBlip cBlip;
		cBlip.m_Id = Id();
		cBlip.m_bySize = 1;
		cBlip.m_eType = ((*it).Polarity() == AuWormhole::Positive) ?
			DqSensorBlip::BLIP_WormholePositive :
			DqSensorBlip::BLIP_WormholeNegative;
		cBlip.SetLocation(t.Project((*it).Location(this)));
		cBlip.m_fValue1 = (*it).ToSystem().GetID();
		aBlips.append(cBlip);
	}
	*/

	// Scan subordinate atoms.
	DaAtom::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
			scannerWeaponRange, scanContent, lockedBlip, stanceCache);
}

double DaSolarSystem::Gravity(double cgDistance) const
{
	const DaStar* star = FindPrimaryStar();
	if(star)
		return star->Gravity(cgDistance);
	return 0;
}

DaVehicle* DaSolarSystem::FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId)
{
	DaStar* star = FindPrimaryStar();
	if(star)
		return star->FindNewVehicle(vehicleCID, empireId);
	return 0;
}

DaVehicle* DaSolarSystem::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId)
{
	DaStar* star = FindPrimaryStar();
	if(star)
		return star->FindNewVehicleType(spotType, empireId);
	return 0;
}

DaVehicle* DaSolarSystem::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher)
{
	DaStar* star = FindPrimaryStar();
	if(star)
		return star->FindNewVehicleType(spotType, fetcher);
	return 0;
}

// Repairs all positive wormholes in this solar system.
// Preserves their names and explored status if possible.
// Does not touch wormholes in other solar systems, leaving other-ends of wormholes to be cleaned up by their
// own parent solar systems if they are left disconnected.
// 'notifyDb' is passed to SetModified calls to signal updates to the database.
// If 'notifyDb' is true, all systems in this sector and the 3x3x3 cube of neighboring sectors must be listed in 'systems'.
// 'notifyDb' is false when fixing positive wormholes in starfield-only perimeter sectors, in preparation for fixing negative
// wormholes in the sectors adjacent to those starfield-only sectors.
void DaSolarSystem::RepairPositiveWormholes(const DqAtomLocArray& systems, bool notifyDb)
{
	// posStarDistance can be 0 if this system has no stars yet.
	float posStarDistance = 0;
	DaStar* ps = FindPrimaryStarNotBlackHole();
	if(ps)
		posStarDistance = ps->Radius() * 2.f;

	// Verify that my positive wormholes connect to the closest systems.
	AuPoint myloc = Location();	// My location relative to my parent sector.
	AuIdList posDest;			// Systems with valid positive wormholes going to them, and this system.
	posDest.append(Id());		// Include myself so I'll be skipped in the closest system search.
	for(int pworms = PositiveWormholes(); pworms; pworms--)
	{
		// Locate the closest other system.
		DqAtomLocArray::const_iterator besti = systems.cend();
		double bestDist2 = -1;
		for(DqAtomLocArray::const_iterator si = systems.cbegin(); si != systems.cend(); si++)
		{
			if(posDest.contains((*si).m_pA->Id()))
				continue;		// Skip myself and the systems we've already found.

			// Wormholes have a limit to the distance they can connect. This limit is a sector-sized cube centered on the "from 
			// system", where each face is 5 sectors away from the "from system". This limit is integral to the wormhole algorithm
			// so that wormholes don't change as nu sectors are discovered.
			if(fabs((*si).m_fLocation.x - myloc.x) >= 5 || fabs((*si).m_fLocation.y - myloc.y) >= 5 ||
					fabs((*si).m_fLocation.z - myloc.z) >= 5)
				continue;		// Skip other system that is outside of the range box.

			double dist2 = (*si).m_fLocation.Distance2(myloc);	// Distance squared from the other system to my system.
			if(bestDist2 == -1 || dist2 < bestDist2)
			{
				besti = si;
				bestDist2 = dist2;
			}
		}
		if(besti == systems.cend())
			continue;	// Did not find a close other system to connect with. This can happen in void areas.
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>((*besti).m_pA);
		if(!system)
			continue;	// System is not a system? Not likely.

		// Found the next closest nearby solar system, possibly in a neighboring sector.
		posDest.append(system->Id());	// Skip the system next time through so we'll find the next closest system.

		// Check to see if we currently have a positive wormhole going to that system.
		AuPoint vToThere = (*besti).m_fLocation - myloc;
		bool foundPos = false;
		m_WormholesLock.lockForWrite();
		AuWormholeArray::iterator pwi = m_Wormholes.begin();
		while(pwi != m_Wormholes.end())
		{
			if((*pwi).Polarity() == AuWormhole::Positive && system->Is((*pwi).ToSystem()))
			{
				// Found a positive wormhole to the system. Insure the data is valid.
				if(!foundPos)
				{
					// This is the first positive wormhole to the destination system.
					foundPos = true;
					if((*pwi).Repair(vToThere, posStarDistance, Rotation()))
					{
						if(notifyDb)
							m_WormholesFinalized = true;
						SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
						//if(notifyDb)	// Only the ones that notify the database are important.
							//qDebug() << "DaSolarSystem::RepairPositiveWormholes" << Id() << Name() << "notifyDb" << notifyDb
								//<< "repaired positive wormhole to" << system->Id() << system->Name();
					}
					pwi++;
				}
				else
				{
					// We already found a positive wormhole to the system. This must be an extra one. Remove it.
					pwi = m_Wormholes.erase(pwi);
					if(notifyDb)
						m_WormholesFinalized = true;
					SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
					//if(notifyDb)	// Only the ones that notify the database are important.
						//qDebug() << "DaSolarSystem::RepairPositiveWormholes" << Id() << Name() << "notifyDb" << notifyDb
							//<< "removed excess positive wormhole to" << system->Id() << system->Name();
				}
			}
			else
				pwi++;
		}
		m_WormholesLock.unlock();
		if(!foundPos)
		{
			// There is no positive wormhole to the system. Create one.
			AddWormholeConnection(system->Id(), vToThere, AuWormhole::Positive);
			if(notifyDb)
				m_WormholesFinalized = true;
			SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
			//if(notifyDb)	// Only the ones that notify the database are important.
				//qDebug() << "DaSolarSystem::RepairPositiveWormholes" << Id() << Name() << "notifyDb" << notifyDb
					//<< "added missing positive wormhole to" << system->Id() << system->Name();
		}
	}

	// Remove remaining positive wormholes that don't belong here, if any.
	m_WormholesLock.lockForWrite();
	AuWormholeArray::iterator wi = m_Wormholes.begin();
	while(wi != m_Wormholes.end())
	{
		// We only care about our positive wormholes.
		if((*wi).Polarity() != AuWormhole::Positive)
		{
			// Skip non-positive wormholes.
			wi++;
			continue;
		}

		// If the wormhole destination is already in the posDest list it's ok. We've already fixed it and insured there are no
		// duplicate positive wormholes to that system.
		if(posDest.contains((*wi).ToSystem()))
		{
			// Skip wormholes that we want to keep.
			wi++;
			continue;
		}

		// Found a positive wormhole that should not be here.
		AuId toId = (*wi).ToSystem();
		wi = m_Wormholes.erase(wi);
		if(notifyDb)
			m_WormholesFinalized = true;
		SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
		//if(notifyDb)	// Only the ones that notify the database are important.
			//qDebug() << "DaSolarSystem::RepairPositiveWormholes" << Id() << Name() << "notifyDb" << notifyDb
				//<< "removed unwanted positive wormhole to" << toId;
	}
	m_WormholesLock.unlock();
}

// Repairs all negative wormholes in this solar system.
// Preserves their names and explored status if possible.
// Does not touch wormholes in other solar systems, leaving other-ends of wormholes to be cleaned up by their
// own parent solar systems if they are left disconnected.
// Requires that all systems in this sector and the 3x3x3 cube of neighboring sectors be listed in 'systems'.
// NOTE: RepairPositiveWormholes must be called on all 'systems' before calling RepairNegativeWormholes.
void DaSolarSystem::RepairNegativeWormholes(const DqAtomLocArray& systems, bool notifyDb)
{
	// negStarDistance can be 0 if this system has no stars yet.
	float negStarDistance = 0;
	DaStar* ps = FindPrimaryStarNotBlackHole();
	if(ps)
		negStarDistance = ps->Radius() * 2.f * 1.1f;

	// Look through all systems for positive wormholes that are connected to this solar system.
	AuPoint myloc = Location();	// My location relative to my parent sector.
	AuIdList negDest;			// Systems with valid negative wormholes going to them.
	for(DqAtomLocArray::const_iterator si = systems.cbegin(); si != systems.cend(); si++)
	{
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>((*si).m_pA);
		if(!system)
			continue;	// System isn't a system? Not likely.

		// Check the system's wormholes to see if any positive wormholes connect to this system.
		system->m_WormholesLock.lockForRead();
		for(AuWormholeArray::const_iterator pwi = system->m_Wormholes.cbegin(); pwi != system->m_Wormholes.cend(); pwi++)
		{
			if((*pwi).Polarity() != AuWormhole::Positive)
				continue;
			if(!Is((*pwi).ToSystem()))
				continue;

			// Found a nearby solar system with a positive wormhole to this system, possibly in a neighboring sector.
			negDest.append(system->Id());

			// Check to see if we currently have a negative wormhole going to that system.
			AuPoint vToThere = (*si).m_fLocation - myloc;
			bool foundNeg = false;
			m_WormholesLock.lockForWrite();
			AuWormholeArray::iterator nwi = m_Wormholes.begin();
			while(nwi != m_Wormholes.end())
			{
				if((*nwi).Polarity() == AuWormhole::Negative && system->Is((*nwi).ToSystem()))
				{
					// Found a negative wormhole to the system. Insure the data is valid.
					if(!foundNeg)
					{
						// This is the first negative wormhole to the destination system.
						foundNeg = true;
						if((*nwi).Repair(vToThere, negStarDistance, Rotation()))
						{
							if(notifyDb)
								m_WormholesFinalized = true;
							SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
							//if(notifyDb)	// Only the ones that notify the database are important.
								//qDebug() << "DaSolarSystem::RepairNegativeWormholes" << Id() << Name() << "notifyDb" << notifyDb
									//<< "repaired negative wormhole to" << system->Id() << system->Name();
						}
						nwi++;
					}
					else
					{
						// We already found a negative wormhole to the system. This must be an extra one. Remove it.
						nwi = m_Wormholes.erase(nwi);
						if(notifyDb)
							m_WormholesFinalized = true;
						SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
						//if(notifyDb)	// Only the ones that notify the database are important.
							//qDebug() << "DaSolarSystem::RepairNegativeWormholes" << Id() << Name() << "notifyDb" << notifyDb
								//<< "removed excess negative wormhole to" << system->Id() << system->Name();
					}
				}
				else
					nwi++;
			}
			m_WormholesLock.unlock();
			if(!foundNeg)
			{
				// There is no negative wormhole to the system. Create one.
				AddWormholeConnection(system->Id(), vToThere, AuWormhole::Negative);
				if(notifyDb)
					m_WormholesFinalized = true;
				SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
				//if(notifyDb)	// Only the ones that notify the database are important.
					//qDebug() << "DaSolarSystem::RepairNegativeWormholes" << Id() << Name() << "notifyDb" << notifyDb
						//<< "added missing negative wormhole to" << system->Id() << system->Name();
			}
		}
		system->m_WormholesLock.unlock();
	}

	// Remove remaining negative wormholes that don't belong here, if any.
	m_WormholesLock.lockForWrite();
	AuWormholeArray::iterator wi = m_Wormholes.begin();
	while(wi != m_Wormholes.end())
	{
		// We only care about our negative wormholes.
		if((*wi).Polarity() != AuWormhole::Negative)
		{
			// Skip non-negative wormholes.
			wi++;
			continue;
		}

		// If the wormhole destination is in the negDest list it's ok. We've already fixed it and insured there are no
		// duplicate negative wormholes to that system.
		if(negDest.contains((*wi).ToSystem()))
		{
			// Skip wormholes that we want to keep.
			wi++;
			continue;
		}

		// Found a negative wormhole that should not be here.
		AuId toId = (*wi).ToSystem();
		wi = m_Wormholes.erase(wi);
		if(notifyDb)
			m_WormholesFinalized = true;
		SetModified(notifyDb?NOTIFY_All:NOTIFY_Scene);
		//if(notifyDb)	// Only the ones that notify the database are important.
			//qDebug() << "DaSolarSystem::RepairNegativeWormholes" << Id() << Name() << "notifyDb" << notifyDb
				//<< "removed unwanted negative wormhole to" << toId;
	}
	m_WormholesLock.unlock();
}

// ::Agent Teleport ArenaGoTo InviteTeleport AcceptTeleport
void DaSolarSystem::AdminGoTo(DaAtom* moveAtom, DaAtom* destAtom)
{
	Q_ASSERT(moveAtom);
	Q_ASSERT(destAtom);

	// This has been repurposed to move people who accept invitations to teleport from other avatars.
	// This has been repurposed to move people into arenas.
	DaPerson* movePerson = dynamic_cast<DaPerson*>(moveAtom);
	DaArena* destArena = dynamic_cast<DaArena*>(destAtom);
	if(movePerson && destArena)
	{
		// Put the moving person at the perimeter of the field.
		AuMatrix4x4 t;
		if(movePerson->ArenaTeam() == DgEmpire::ArenaTeam1Id())
			t.Translate((c_ArenaRingRadius+c_ArenaBoundsRadius)/2, 0, 0);
		else if(movePerson->ArenaTeam() == DgEmpire::ArenaTeam2Id())
			t.Translate(-(c_ArenaRingRadius+c_ArenaBoundsRadius)/2, 0, 0);
		else
			t.Translate(0, 0, (c_ArenaRingRadius+c_ArenaBoundsRadius)/2);
		movePerson->ForcePosStop(AuAtomPos(destArena->Id(), t));
		movePerson->SetModified();
		return;
	}

	DaCreature* moveCreature = dynamic_cast<DaCreature*>(moveAtom);
	DaVessel* moveShip = dynamic_cast<DaVessel*>(moveAtom);
	DaVehicle* moveVehicle = dynamic_cast<DaVehicle*>(moveAtom);
	if(moveCreature || moveShip || moveVehicle)
	{
		DaCreature* destCreature = dynamic_cast<DaCreature*>(destAtom);
		DaCapsule* destCapsule = dynamic_cast<DaCapsule*>(destAtom);
		DaVehicle* destVehicle = dynamic_cast<DaVehicle*>(destAtom);
		DaStar* destStar = dynamic_cast<DaStar*>(destAtom);
		if(destAtom == this)
			destStar = FindPrimaryStarNotBlackHole();	// Destination is the primary star instead of this solar system itself.

		// Check for destination creature that is in a vehicle. Change destination to vehicle in that case.
		if(destCreature)
		{
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(destCreature->Parent());
			if(vehicle)
			{
				// Destination creature is aboard a vehicle.
					// The dest vehicle must have an empty seat or MountVehicle will fail, leaving the agent in limbo.
				destVehicle = vehicle;
				AuPoint seatLoc;
				AuPose seatPose;
				if(destVehicle->FindEmptySeat(seatLoc, seatPose) >= 0)
					destCreature = 0;	// Use destVehicle logic vs destCreature logic.
			}
		}

		//qDebug() << "DaSolarSystem::AdminGoTo" << Id() << Name() << "moveAtom" << moveAtom->Id() << moveAtom->Name()
			//<< "destAtom" << destAtom->Id() << destAtom->Name();

		// Insure atom is not the parent of the destination atom. This happens when teleporting a person with their parent
		// ship/vehicle/animal to a person who is aboard that same ship/vehicle/animal. It might also happen if a person tried
		// to teleport to someone they were carrying piggyback.
		if(moveAtom->FindIdRecursive(destAtom->Id()))
			return;

		// Insure this solar system is not dormant.
		SetLastPersonVisited();

		// Destination atom is a creature or city.

		// The kind of atom that is moving and the kind of destination atom determine how the move works.
		DaWorld* destPlanet = destAtom->FindParentPlanetInEnv();
		if(moveCreature)
		{
			// A creature is moving, probably a person or their mount.
			moveCreature->ExitConsole();						// In case they were in a table game.
			moveCreature->SetVehicleSeatIndex(0);				// Clear this for creature that moved from vehicle.
			if(destCreature)
			{
				// Find top-most destination creature.
				while(dynamic_cast<DaCreature*>(destCreature->Parent()))
					destCreature = dynamic_cast<DaCreature*>(destCreature->Parent());
				if(destCreature->InDesignEnvironment())
				{
					// Destination creature thinks it is in a design environment.
					DaPerson* movePerson = dynamic_cast<DaPerson*>(moveCreature);
					if(movePerson)
					{
						DaDesignBase* pde = destCreature->FindParentDesignBaseInEnv();
						if(pde)
						{
							// The destination creature really is in a design environment instance.
							// Put the moving person at the same location as the top-most destination creature in the design
							// environment. The destination creature should be a person since we don't let animals in here.
							moveCreature->ForcePosStop(AuAtomPos(destCreature->ParentId(), destCreature->Transform()));

							// Insure there is a design plane present for the admin who just moved into the design studio.
							pde->InitDesignPlane(movePerson->Id());
						}
						else
						{
							// Put the moving creature at the same location as the top-most destination creature.
							moveCreature->ForcePosStop(AuAtomPos(destCreature->ParentId(), destCreature->Transform()));
						}
					}
					else
					{
						// Animals aren't allowed in the design environment.
						// Place the creature subordinate to the terrain instead of in the design environment.
						DaWorld* ter = destCreature->FindParentPlanetInEnv();
						AuMatrix4x4 t;
						destCreature->GetTransformRelativeToParent(t, ter);
						moveCreature->ForcePosStop(AuAtomPos(ter->Id(), t));
					}
				}
				else
				{
					// Put the moving creature at the same location as the top-most destination creature.
					moveCreature->ForcePosStop(AuAtomPos(destCreature->ParentId(), destCreature->Transform()));

					// Mount the destination creature. This is very helpful when the creature is moving, especially when flying.
					DaPerson* movePerson = dynamic_cast<DaPerson*>(moveCreature);
					if(movePerson && movePerson->IsOnLineAsAgent())
						movePerson->ForceMountCreature(destCreature);
				}
			}
			else if(destCapsule)
			{
				// Destination is a ship and a creature is moving. Find a place to beam aboard.
				// If no void loc is found, the origin of the ship will suffice. The moving atom will figure it out from there.
				AuPoint voidLoc;
				destCapsule->TransporterVoidLocation(voidLoc);
				AuMatrix4x4 t;
				t.Translate(voidLoc);
				moveCreature->ForcePosStop(AuAtomPos(destCapsule->Id(), t));
			}
			else if(destVehicle)
			{
				// Destination is a vehicle and a creature is moving. Find an empty seat if possible.
				moveCreature->ForceMountVehicle(destVehicle);
			}
			else if(destStar)
			{
				// Put the moving creature above the star, just outside its radius.
				// We also end up here if the destination is this solar system.
				AuMatrix4x4 t;
				t.Translate(0, 0, destStar->AtmosphereRadius() + ParkAltAboveSun());
				moveCreature->ForcePosStop(AuAtomPos(destStar->Id(), t));
			}
			else if(destPlanet)
			{
				// Put the moving creature on the surface.
				AuPoint loc = destPlanet->RandomAvatarLocation();
				double cg = destPlanet->CGAltitude(loc);
				destPlanet->ElevateLoc(loc, cg + moveCreature->GroundHeightAboveCG());
				AuMatrix4x4 t;
				t.Translate(loc);
				moveCreature->ForcePosStop(AuAtomPos(destPlanet->Id(), t));
			}
			else if(destAtom == this)
			{
				// Destination is this system and no star was found.
				AuMatrix4x4 t;
				moveCreature->ForcePosStop(AuAtomPos(destAtom->Id(), t));
			}
			else
			{
				// Put the moving creature at the destination atom's location.
				moveCreature->ForcePosStop(AuAtomPos(destAtom->ParentId(), destAtom->Transform()));
			}

			// Agent always arrives invisible and invincible.
			DaPerson* movePerson = dynamic_cast<DaPerson*>(moveCreature);
			if(movePerson && movePerson->IsOnLineAsAgent())
				movePerson->m_AgentBits |= (AGENT_Invisible|AGENT_Invincible);

			if(destVehicle || (destCreature && moveCreature->Parent() == destCreature))
				moveCreature->SetAtRest();
			else
			{
				moveCreature->ForceVector(AuPoint(0,0));
				moveCreature->Stand();
			}
			moveCreature->SetModified();

			// Mark this system as explored by the empire who owns the creature.
			ExploredBy(moveCreature->EmpireId());
		}
		else if(moveShip)
		{
			// A spacecraft is moving.
			if(destPlanet)
			{
				// Destination atom is at a planet. Put the ship in orbit over the destination atom.
				AuPoint spawnAt;
				if(destPlanet != destAtom)
					destAtom->GetLocationRelativeToParent(spawnAt, destPlanet);	// Destination atom is at the destination planet.
				else
					spawnAt = AuPoint(1,0);										// Destination atom IS the destination planet.

				const double orbitAlt = destPlanet->AtmosphereRadius() + ParkAltAboveAtm();
				spawnAt = destPlanet->FindOrbitSpawnLoc(spawnAt, moveShip->Id(), moveShip->RoughRadius(), orbitAlt, true);
				AuPoint downVec(destPlanet->GravityDir(spawnAt));

				// Position the spacecraft in orbit.
				AuMatrix4x4 t;
				t.Translate(spawnAt);

				// Align with up, according to ship orbit policies.
				switch(moveShip->PilotUpPolicy())
				{
				case UpAwayFromGravity:
					t *= AuPoint(0,0,1).GetAlignmentTransform(-downVec);
					break;
				case UpTowardGravity:
					t *= AuPoint(0,0,1).GetAlignmentTransform(downVec);
					break;
				case PortTowardGravity:
					t *= AuPoint(0,0,1).GetAlignmentTransform(-downVec);	// Orient so down is toward the planet.
					t.RotateY(-PI/2);										// Roll port side down.
					break;
				case StarboardTowardGravity:
					t *= AuPoint(0,0,1).GetAlignmentTransform(-downVec);	// Orient so down is toward the planet.
					t.RotateY(PI/2);										// Roll starboard side down.
					break;
				case PILOT_UP_POLICY_Count:
					break;
				}
				moveShip->ForcePosStop(AuAtomPos(destPlanet->Id(), t));
			}
			else if(destStar)
			{
				// Put the moving ship above the star, just outside its radius.
				// We also end up here if the destination IS this solar system.
				AuMatrix4x4 t;
				if(destAtom == this)
				{
					// The system is the destination. Let's act like we just arrived from deep space.
					if(IsCoreBlackHole())
						t.Translate(0, 0, FeetPerSolarSystem());	// Enter core black hole system from the farthest outskirts.
					else
						t.Translate(0, 0, ActualRadius());			// Enter regular systems from the actual outskirts.
				}
				else
					t.Translate(0, 0, destStar->AtmosphereRadius() + ParkAltAboveSun());	// Sun is the destination.
				moveShip->ForcePosStop(AuAtomPos(destStar->Id(), t));
			}
			else if(destAtom == this)
			{
				// Destination is this system and no star was found.
				AuMatrix4x4 t;
				moveShip->ForcePosStop(AuAtomPos(destAtom->Id(), t));
			}
			else
			{
				// Destination atom is not at a planet and is not a planet, star or this solar system; it must be in this solar
				// system somewhere.
				double dist = moveShip->RoughRadius();
				if(destCapsule)
					dist += destCapsule->RoughRadius();
				dist += 100*FeetPerMeter();
				DaSolarSystem* destSystem = destAtom->FindParentSolarSystem();
				Q_ASSERT(destSystem);
				AuMatrix4x4 t = destAtom->EnvTransform();
				t.Translate(0,-dist,0);											// Directly behind the target.
				moveShip->ForcePosMove(AuAtomPos(destSystem->Id(), t), destAtom->Vector());	// Match movement vector of other atom.
			}
			moveShip->MissionSetActive(false);					// Deactivate the mission on arrival.
			moveShip->SetInMotion();
			moveShip->SetModified();
			moveShip->TickTockRecursive();

			// Mark this system as explored by the empire who owns the spacecraft.
			ExploredBy(moveShip->EmpireId());
		}
		else //if(moveVehicle)
		{
			// A vehicle is moving.
			if(destCreature)
			{
				// Destination is a creature.
				if(destCreature->InDesignEnvironment())
				{
					// Place the vehicle subordinate to the terrain.
					DaWorld* ter = destCreature->FindParentPlanetInEnv();
					AuMatrix4x4 t;
					destCreature->GetTransformRelativeToParent(t, ter);
					moveVehicle->ForcePosStop(AuAtomPos(ter->Id(), t));
				}
				else if(destCreature->IsInSpacecraft())
				{
					// Place the vehicle at a safe distance from the top-most spacecraft in the tree.
					DaVessel* ship = destCreature->FindParentVesselInEnv();
					while(ship->IsInSpacecraft())
						ship = ship->FindParentVesselInEnv();
					AuMatrix4x4 t = ship->Transform();
					t.Translate(0, -(ship->Length()/2 + 100), 0);	// Directly behind the target.
					moveVehicle->ForcePosMove(AuAtomPos(ship->ParentId(), t), ship->Vector());	// Match vector of other spacecraft.
				}
				else if(destCreature->IsInVehicle())
				{
					// Place the vehicle at a safe distance from the destination creature's vehicle.
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(destCreature->Parent());
					AuMatrix4x4 t = vehicle->Transform();
					t.Translate(0, -200, 0);	// Directly behind the target.
					moveVehicle->ForcePosMove(AuAtomPos(vehicle->ParentId(), t), vehicle->Vector());	// Match vector of vehicle.
				}
				else
				{
					// Place the vehicle at a safe distance from the top-most creature in the tree; the destination atom could
					// be riding another creature.
					while(dynamic_cast<DaCreature*>(destCreature->Parent()))
						destCreature = dynamic_cast<DaCreature*>(destCreature->Parent());
					AuMatrix4x4 t = destCreature->Transform();
					t.Translate(0, -100, 0);	// Directly behind the target.
					moveVehicle->ForcePosStop(AuAtomPos(destCreature->ParentId(), t));
				}
			}
			else if(destStar)
			{
				// Put the moving vehicle above the star, just outside its radius.
				// We also end up here if the destination at is this solar system.
				AuMatrix4x4 t;
				t.Translate(0, 0, destStar->AtmosphereRadius() + ParkAltAboveSun());
				moveVehicle->ForcePosStop(AuAtomPos(destStar->Id(), t));
			}
			else if(destAtom == this)
			{
				// Destination is this system and no star was found.
				AuMatrix4x4 t;
				moveVehicle->ForcePosStop(AuAtomPos(destAtom->Id(), t));
			}
			else
			{
				// Put the moving vehicle near the destination atom.
				AuMatrix4x4 t = destAtom->Transform();
				t.Translate(0, -100, 0);	// Directly behind the target.
				moveVehicle->ForcePosStop(AuAtomPos(destAtom->ParentId(), t));
			}
			moveVehicle->SetModified();

			// Mark this system as explored by the empire who owns the vehicle.
			ExploredBy(moveVehicle->EmpireId());
		}
	}
	else
	{
		qWarning("DaSolarSystem::AdminGoTo moveAtom %d %s is not a creature, ship, or vehicle!",
				moveAtom->Id().GetID(), (const char*)moveAtom->Name().toLatin1());
	}
}

bool DaSolarSystem::HasRingworld() const
{
	const DaStar* primaryStar = FindPrimaryStar();
	if(!primaryStar)
		return false;

	if(primaryStar->IsRingworldStar())
		return true;

	DaAtomList subs;
	primaryStar->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaStar* companionStar = dynamic_cast<const DaStar*>(*it);
		if(!companionStar)
			continue;
		if(companionStar->IsRingworldStar())
			return true;
	}
	return false;
}

// A solar system with no cities or players is ready for decay one week after it was last visited by a human player or AI officer.
bool DaSolarSystem::IsReadyToDecay() const
{
	// Ringworld solar systems do not decay, for henge gates.
	if(HasRingworld())
	{
		//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "no because ringworld present.";
		return false;
	}

	// Land solar systems do not decay.
	// Active time is reset once per day by avatars who have designated this system as their land and whose account is active.
	m_LastAvatarAccountActiveLock.lock();
	bool v = !m_LastAvatarAccountActive.IsNull() && m_LastAvatarAccountActive.ElapsedDay() < 3;	// true = protected
	m_LastAvatarAccountActiveLock.unlock();
	if(v)
	{
		//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "no because avatar account active in past 3 days.";
		return false;
	}

	// Last visited time is set as soon as a person or spacecraft enters this system.
	// Officers update it once per minute.
	// People update it during their scene sync.
	m_LastOfficerVisitLock.lockForRead();
	v = m_LastOfficerVisit.IsNull() || m_LastOfficerVisit.ElapsedDay() >= 7;
	m_LastOfficerVisitLock.unlock();
	if(v)
	{
		// No officer has visited in the past week.
		AuTime t = LastPersonVisited();
		if((t.IsNull() || t.ElapsedDay() >= 7) && !FindPersonRecursive())
		{
			// One week has passed since this system was last visited.
			// If there are no buildings or cities, this system can decay completely.
			DaAtomList bdsOrCities;
			FindBdRecursive(bdsOrCities);
			if(bdsOrCities.isEmpty())
			{
				//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "yes.";
				return true;		// This solar system is ready for decay. Let the manager server do that.
			}
			//else
				//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "no because buildings are here.";
		}
		//else
			//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "no because person present or visited online in past week.";
	}
	//else
		//qDebug() << "DaSolarSystem::IsReadyToDecay" << *this << "no because officer visited in past week.";
	return false;
}

void DaSolarSystem::DecayContents()
{
	//Q_ASSERT(g_Au.IsSceneWorker());	// This is only called on the worker server.
	SetStarFieldFlag(true);				// Prevent atoms from arriving or respawning here.
	TickTockRecursive();

	// Clear the remaining system contents.
	if(HasSubatoms())
	{
		// Take all player characters out of this solar system.
		// Move them to be subordinate to the galaxy so they can respawn somewhere in that galaxy.
		// NOTE: Write access to parent atoms of this solar system is thread safe because this function is only called when the
		//       root atom is locked for write. See AuSceneServerApp::SolarSystemReadyToDecay.
		DaGalaxy* galaxy = FindParentGalaxy();
		if(galaxy)
		{
			DaAtomList people;
			FindLivePeopleRecursive(people);
			while(!people.isEmpty())
			{
				DaPerson* person = dynamic_cast<DaPerson*>(people.takeFirst());
				if(person)
				{
					person->ForcePosStop(AuAtomPos(galaxy->Id(), AuMatrix4x4()));
					person->Respawn(false);
				}
			}
		}

		// Kill off the remaining subatoms of this solar system. This signals the database to delete their records.
		KillSubatoms();

		// Delete the actual atoms; this system's contents will not be Ticked or Tocked again as it is now starfield only.
		TrashSubatoms();
	}
}

// Returns true if any units are present that 'empireId' considers to be an enemy.
bool DaSolarSystem::IsEnemyEmpirePresent(quint32 empireId) const
{
	list<quint32> empires;
	m_EmpiresPresentLock.lockForRead();
	for(unordered_map<quint32, AuElapsedTimer>::const_iterator epi = m_EmpiresPresent.begin(); epi != m_EmpiresPresent.end(); epi++)
		empires.insert(empires.end(), (*epi).first);
	m_EmpiresPresentLock.unlock();

	for(list<quint32>::const_iterator ei = empires.begin(); ei != empires.end(); ei++)
		if(PoliticalStance(empireId, *ei) == POL_Enemy)
			return true;		// A unit or city of an enemy empire is present.
	return false;				// No units from enemy empires present.
}

// Returns true if any units are present that 'empireId' considers to be an enemy.
bool DaSolarSystem::IsNeutralEmpirePresent(quint32 empireId) const
{
	list<quint32> empires;
	m_EmpiresPresentLock.lockForRead();
	for(unordered_map<quint32, AuElapsedTimer>::const_iterator epi = m_EmpiresPresent.begin(); epi != m_EmpiresPresent.end(); epi++)
		empires.insert(empires.end(), (*epi).first);
	m_EmpiresPresentLock.unlock();

	for(list<quint32>::const_iterator ei = empires.begin(); ei != empires.end(); ei++)
		if(PoliticalStance(empireId, *ei) == POL_Neutral)
			return true;		// A unit or city of a neutral empire is present.
	return false;				// No units from enemy empires present.
}

// 'victim' is the ship that triggered this encounter.
int DaSolarSystem::PirateEncounter(DaVessel* victim, bool agentTriggered)
{
	// Probability of an encounter is directly affected by the ubiquity of pirate empires in this galaxy.
	// When all pirate empires are ready, ubiquity in all galaxies ranges from 28 to 57.
	const DgData& gd = g_Au.LockDataForRead();
	float pirateUbiquity = gd.PirateUbiquity(victim->Galaxy());
	g_Au.UnlockData();
	if(!pirateUbiquity)
	{
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounter" << *this << "victim" << *victim << "in" << GalaxyName(victim->Galaxy())
				<< "no pirate empires ready";
		return -1;				// No pirates in this galaxy that are ready for encounters.
	}
	pirateUbiquity /= 10.f;	// 2.8 to 5.7 when all pirate empires are ready.

	// Pirates appear more frequently in areas explored by a lot of people.
	m_ExploredByLock.lockForRead();
	float exploredByFactor = 1.f + (float)m_ExploredBy.size()/10.f;
	m_ExploredByLock.unlock();
	float probEncounter = ceilf(exploredByFactor * pirateUbiquity);	// 0 to 100 % probability of a pirate encounter this time.

	// Probability of a pirate encounter increases a lot when a pirate base is in this system.
	quint32 empireId;
	if(HasPirateBase(empireId))
		probEncounter += 50;

	// Probability of a pirate encounter increases with time.
	m_LastPirateLock.lockForRead();
	bool v = m_LastPirateShip.isValid();
	int lastPirateMins = m_LastPirateShip.ElapsedMin();			// Minutes since the last pirate ship was in this system.
	m_LastPirateLock.unlock();
	if(v)
	{
		// System had pirates since the system was loaded on this particular server. Last pirate time is not stored in db.
		probEncounter += (float)lastPirateMins / 60.f;			// Increase probability by 1% per hour.
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounter" << *this << "victim" << *victim << "in" << GalaxyName(victim->Galaxy())
				<< "ubiq" << pirateUbiquity << "explo" << exploredByFactor << "hasBase" << (empireId?true:false)
				<< "lastMins" << lastPirateMins << "prob" << probEncounter;
	}
	else
	{
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounter" << *this << "victim" << *victim << "in" << GalaxyName(victim->Galaxy())
				<< "ubiq" << pirateUbiquity << "explo" << exploredByFactor << "hasBase" << (empireId?true:false)
				<< "prob" << probEncounter;
	}

	if(!agentTriggered)
	{
		// Encounter was not triggered by an agent so roll dice to see if it happens.
		if(probEncounter <= 0)
			return -2;											// No possibility of a pirate encounter this time.
		if(probEncounter < 100 && g_Au.Dice().Roll(1,100) > (int)probEncounter)
			return 1000+probEncounter;							// No pirate encounter this time.
	}

	// This call can trigger the manager server to spawn a pirate base in this sector.
	return PirateEncounterBase(victim, agentTriggered);
}

int DaSolarSystem::PirateEncounterBase(DaVessel* victim, bool agentTriggered)
{
	AuEther* ether = Ether();
	if(!ether)
		return -13;
	// Determine range at which the encounter occurs, at maximum sensor range so the enemy doesn't just appear on their scope.
	double pirateRange = victim->SensorRange();
	if(pirateRange < FeetPerMoonOrbit())
		pirateRange = FeetPerMoonOrbit();
	else if(pirateRange > FeetPerSolarSystem())
		pirateRange = FeetPerSolarSystem();

	quint32 empireId;
	int ret;
	AuPoint encLoc;
	if(HasPirateBase(empireId))
	{
		// This system contains a pirate base.
		// Calculate encounter location in a random direction, as if the pirates are coming to help.
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << "victim" << *victim
				<< "pirate base already in this system";
		AuPoint randomVec = AuParticleSystem::GetRandomVector() * pirateRange;
		encLoc = victim->EnvLocation() + randomVec;
		ret = 0;
	}
	else
	{
		// Check to see if my sector contains a pirate base.
		// 'empireId' is an output parameter of EncounterPirateBase, if a base is found.
		DaSector* sector = FindParentSector();
		const DaSolarSystem* baseSystem = sector->EncounterPirateBase(Id(), empireId);
		if(!baseSystem)
			return -11;	// No pirate base in sector. The call could have triggered creation of one. We must wait until it arrives.
		if(baseSystem->Is(Id()))
			return -12;	// wut?
		// Calculate encounter location in the direction of the base, as if the pirates are coming from the base.
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << "victim" << *victim
				<< "pirate base in system" << *baseSystem;
		AuPoint baseSectorLoc = baseSystem->Location();				// Location of base's system relative to our parent sector.
		AuPoint vecToBase = Transform().UnProject(baseSectorLoc);	// Location of base's system relative to this system.
		vecToBase.Unit(pirateRange);
		encLoc = victim->EnvLocation() + vecToBase;
		AuPoint randomVec = AuParticleSystem::GetRandomVector() * 1000*FeetPerMeter();
		encLoc += randomVec;	// So two encounters in a row don't land on top of each other.
		ret = 1;
	}

	if(!IsInSystem(encLoc))
	{
		if(agentTriggered)
			qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << "encounter location falls outside system, "
				"sensorRange" << victim->SensorRange() << "pirateRange" << pirateRange;
		return -13;		// The encounter location is not in this solar system.
	}

	// Pirate encounter location is in the system.
	if(agentTriggered)
		qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << "victim" << *victim << "encountered a pirate in this system at"
			<< encLoc << "pirate empire" << empireId;
	// Select a blueprint for a spacecraft.
	AuScPrefs::SCROLE scrole;
	const DgData& gd = g_Au.LockDataForRead();
	quint32 designId = gd.PirateEncounterSpacecraft(empireId, Galaxy(), scrole);
	int rank = gd.PirateEmpireRank(empireId);
	g_Au.UnlockData();
	if(!designId)
		return -14;	// This should not happen. The encounter empire was checked to be sure it was ready for encounters.
	quint8 quality = rank * 16;

	AuScReport shipDesign;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool loaded = sdd.ScDesignReport(designId, shipDesign);	// Signals load if needed but all pirate designs are already loaded.
	g_Au.UnlockScDesigns();
	if(!loaded)
		return -15;	// This should not happen. We are supposed to insure all pirate designs are loaded at all times.

	// Prepare to make spaceships.
	const DaSector* sector = FindParentSector();
	QString empire;
	const DgData& gd1 = g_Au.LockDataForRead();
	gd1.EmpireName(empireId, empire);
	g_Au.UnlockData();

	// Create one or more spaceship(s).
	int piratesMax = (int)ceilf(victim->Mass() / shipDesign.MassLbs());	// Pirates needed for a match of equal mass.
	int pirateShips;
	if(piratesMax == 1)
		pirateShips = 1;
	else
		pirateShips = g_Au.Dice().Roll(1, piratesMax);			// Randomly determine number of pirate ships in this encounter.
	if(pirateShips > 3)
		pirateShips = 3;										// Limit encounters to 3 ships max.
	for(int ps=0; ps<pirateShips; ps++)
	{
		// Place ship at initial spawn location.
		AuMatrix4x4 t;
		t.Translate(encLoc);

		// Create the spaceship at the encounter location.
		DaSc* ship = new DaSc();
		ship->ConnectSignals();
		ship->Create();
		ship->SetPosAndOrient(AuAtomPos(Id(), t));
		ether->AddAtom(ship);

		ship->SetDesign(shipDesign);
		ship->SetupPirate(empireId, quality);

		// Develop a description of where this ship was made.
		QString whereMade;
		if(sector)
			whereMade = tr("%1 encounter with %2 in %3, %4.")
				.arg(empire).arg(victim->NameWithEmpire()).arg(Name()).arg(sector->Name());
		else
			whereMade = tr("%1 encounter with %2 in %3.")
				.arg(empire).arg(victim->NameWithEmpire()).arg(Name());
		ship->SetWhereMade(whereMade);

		g_Au.AtomAdded(ship->Id());
		ship->SetNormal();	// New atom is ready for normal thread processing.

		// Give the pirate a mission.
		if(ret == 0)
		{
			// Defend the base. Give the ship a mission order to destroy the victim.
			ship->MissionInsertOrder(0, AuMissionOrderAttack(victim->Id(), victim->Name()));
			qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << *ship << "attacking" << *victim;
		}
		else
		{
			// Loot the target. Give the ship a mission order to plunder the victim.
			ship->MissionInsertOrder(0, AuMissionOrderPlunder(victim->Id(), victim->Name()));
			qDebug() << "DaSolarSystem::PirateEncounterBase" << *this << *ship << "plundering" << *victim;
		}
		ship->MissionSetType(AuMission::Single);
		ship->MissionSetActive(true);						// Activate my mission.

		// Spread out multiple ships.
		encLoc.x += ship->PilotAvoidRadius() * 3;
		ship->TickTockRecursive();
	}

	// Note the presence of pirates.
	SetLastPirateShip();
	ExploredBy(empireId);
	return ret;
}

// Returns true if all the planets in this system that have cities returns true to OkToRename.
// At least one planet in the system must have a city controlled by 'empireId'.
// A system with no planets is always ok to rename.
bool DaSolarSystem::OkToRename(quint32 empireId) const
{
	DaAtomList planets;
	FindPlanetsRecursive(planets);
	if(planets.isEmpty())
		return true;

	// Check all planets. All flags of all cities must belong to 'empireId' and no city may be occupied.
	bool anyCityOfEmpire = false;
	for(DaAtomList::const_iterator pi = planets.begin(); pi != planets.end(); pi++)
	{
		const DaWorld* planet = dynamic_cast<const DaWorld*>(*pi);
		if(!planet)
			continue;

		int okToRename = planet->OkToRename(empireId);
		if(okToRename == 0)
			anyCityOfEmpire = true;								// Planet with empire city is ok to rename.
		else if(okToRename == 1)
			continue;											// Planet has no cities at all.
		else
			return false;										// Occupied city or flag of a different empire found.
	}

	// If we get here, we need to have found a city of the empire to enable rename.
	return anyCityOfEmpire;
}

// Returns the transformation matrix relative to this atom's parent solar system.
AuMatrix4x4 DaSolarSystem::EnvTransform() const
{
	return AuMatrix4x4();
	/*
	// Try for precalculated env transform.
	m_EnvTransformLock.lockForRead();
	if(IsEnvTransformValid())
	{
		AuMatrix4x4 et(m_EnvTransform);
		m_EnvTransformLock.unlock();
		return et;
	}
	m_EnvTransformLock.unlock();

	// Must calculate env transform.
	m_EnvTransformLock.lockForWrite();
	if(!IsEnvTransformValid())
	{
		// This IS the parent solar system. Solar systems are not subordinate to other solar systems.
		m_EnvTransform.LoadIdentity();

		// The transform is valid until somebody says otherwise.
		m_EnvTransformValid = true;
	}
	AuMatrix4x4 et(m_EnvTransform);
	m_EnvTransformLock.unlock();

	// Return this atom's transform relative to its parent environment atom.
	return et;
	*/
}

// Adjusts systemLoc so it is not inside the atmosphere/photosphere of any planets/stars.
// Will be a close orbit radius outside of those.
// 'shipLoc' is the reference loc for deciding where to move systemLoc. It is probably the location of a navigator's ship.
void DaSolarSystem::AdjustNavLocation(const AuPoint& shipLoc, AuPoint& systemLoc) const
{
	DaStar* primary = FindPrimaryStar();
	if(primary)
		primary->AdjustNavLocation(shipLoc, systemLoc);
}

int DaSolarSystem::PositiveWormholes() const
{
	if(IsEmptySystem())
		return 0;

	// 1-4 wormholes originate from each solar system.
	// Use noise to determine the count. Id() and GenSeed() cannot be used.
	AuPoint galaxyLoc = GalaxyLoc();
	double nx = AuNoise::IntegerNoise(0, (int)galaxyLoc.x);
	double ny = AuNoise::IntegerNoise(1, (int)galaxyLoc.y);
	double nz = AuNoise::IntegerNoise(2, (int)galaxyLoc.z);

	int worms = 1;
	if(nx > .15)
		worms++;
	if(ny > .15)
		worms++;
	if(nz > .15)
		worms++;

	// Do not exceed three positive wormholes in a system.
	if(worms > 3)
		worms = 3;
	return worms;
}

// Returns the radius of this solar system based on the farthest orbiting body from the sun at this moment.
// Radius returned is from center of primary star to center of farthest orbiting body.
double DaSolarSystem::ActualRadius() const
{
	DaStar* star = FindStar();
	if(!star)
		return FeetPerSolarSystem()/2;

	// Create a list of all the stars and planets in the solar system.
	DaAtomList starsAndPlanets;

	// Append the primary star to the list.
	//why? starsAndPlanets.append(star);

	// Append companion stars to the list. They are always subordinate to the primary.
	star->ListStars(starsAndPlanets);

	// Append all planets and moons to the list.
	star->FindPlanetsRecursive(starsAndPlanets);

	// Determine which star or planet is farthest from the primary star at this time.
	double wormholeRadius = star->Radius() * 2.2;				// Start at farthest wormhole radius, possibly farther than planets.
	double farthestDist2 = wormholeRadius * wormholeRadius;		// Distance squared to the farthest star or planet.
	for(DaAtomList::const_iterator spi = starsAndPlanets.begin(); spi != starsAndPlanets.end(); spi++)
	{
		AuPoint sysLoc = (*spi)->EnvLocation();
		double dist2 = sysLoc.Length2();
		if(dist2 > farthestDist2)
			farthestDist2 = dist2;
	}

	// Return the actual radius, not the squared value.
	return sqrt(farthestDist2);
}

int DaSolarSystem::StrongGravity() const
{
	if(m_StrongGravity == 0)
	{
		// The core black hole solar system is at 0,0,0 in the core sector.
		if(Location() != AuPoint(0,0,0))
			m_StrongGravity = 1;
		else
		{
			// Parent sector must say if we are really at the core.
			const DaSector* sector = FindParentSector();
			if(!sector)
				m_StrongGravity = 1;
			else
				m_StrongGravity = sector->IsCore() ? 2 : 1;
		}

		if(m_StrongGravity == 1)
		{
			// Supermassive black hole not found. Check primary star for stellar black hole.
			const DaStar* star = FindPrimaryStar();
			if(star)
			{
				if(star->IsStellarBlackHole())
					m_StrongGravity = 3;
				else if(star->IsNeutronStar())
					m_StrongGravity = 4;
				else
				{
					// Check companions for neutron stars.
					DaAtomList stars;
					star->ListStars(stars);
					for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
					{
						star = (const DaStar*)(*si);
						if(star->IsNeutronStar())
							m_StrongGravity = 4;
					}
				}
			}
		}
	}
	return m_StrongGravity - 1;
}

bool DaSolarSystem::HasBlackHole() const
{
	int grav = StrongGravity();
	return grav == 1 || grav == 2;
}

bool DaSolarSystem::IsCoreBlackHole() const
{
	return StrongGravity() == 1;
}

QString DaSolarSystem::SystemScanReport(int scanEx) const
{
	Q_UNUSED(scanEx);
	QString rpt = QString("<html><b>%1</b><p>").arg(Name());

	rpt += tr("Catalog Name %1").arg(CatalogName());

	AuPoint fLoc = Location();
	rpt += tr("<br>(%1, %2, %3)pc Relative to Sector Center").arg(fLoc.x, 0,'f',1).arg(fLoc.y, 0,'f',1).arg(fLoc.z, 0,'f',1);

	/* We don't have enough information to do this.
	AuPoint fRot = Rotation() * RAD2DEG;
	fRot.Round(.1);
	rpt += tr("<br>(%1, %2, %3)%4 Rotation Relative to Sector").arg(fRot.x).arg(fRot.y).arg(fRot.z).arg(AuGlobal::DegreeSymbol());

	m_StarLightLock.lockForRead();
	if(m_StarLight >= 0.f)
		rpt += tr("<br>Ambient Star Light %1").arg(m_StarLight);
	m_StarLightLock.unlock();
	*/

	bool checkNeutronStars = false;
	switch(StrongGravity())
	{
	default:
		break;
	case 1:
		rpt += tr("<br>Galaxy Core Black Hole Present"); 
		checkNeutronStars = true;
		break;
	case 2:
		rpt += tr("<br>Stellar Black Hole Present"); 
		checkNeutronStars = true;
		break;
	case 3:
		rpt += tr("<br>Neutron Star Present"); 
		break;
	}
	if(checkNeutronStars)
	{
		// Black holes can be orbited by neutron star companions. If we're going to mention them, we must mention all of them.
		const DaStar* star = FindPrimaryStar();
		if(star)
		{
			if(star->IsNeutronStar())
				rpt += tr("<br>Neutron Star Present"); 
			else
			{
				// Check companions for neutron stars.
				DaAtomList stars;
				star->ListStars(stars);
				for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
				{
					star = (const DaStar*)(*si);
					if(star->IsNeutronStar())
					{
						rpt += tr("<br>Neutron Star Present"); 
						break;
					}
				}
			}
		}
	}

	rpt += "</html>";
	return rpt;
}

bool DaSolarSystem::DepartedOnDeadHeading(DaAtom* departAtom, const AuPoint& departLoc)
{
	// Scene client notifies server to do the transition, who then calls this function on the server side.
	if(g_Au.IsSceneClient())
	{
		// Scene client notifies server to do the transition.
		departAtom->SetTraveling();
		emit DeadheadedOnClient(Id(), departAtom->Id(), departLoc);
		return true;
	}

	// Some atoms are destroyed rather than depart the solar system.
	if(!departAtom->DepartingSolarSystem())
		return false;

	const DaSector* ps = (const DaSector*)Parent();
	if(!ps)
		return false;

	const DaGalaxy* galaxy = ps->FindParentGalaxy();
	if(!galaxy)
		return false;

	// Convert departLoc to pc relative to the galaxy.
	AuMatrix4x4 sysRot;
	sysRot.Rotate(Rotation());
	AuPoint galaxyLoc = Location() + sysRot.Project(departLoc) / FeetPerParsec();	// sector location
	galaxyLoc += ps->Location();			// Convert from relative to my sector to relative to the galaxy.

	// Locate the closest solar system to the destination location if there is one. This test prevents us from moving into
	// the perimeter sectors that contain no solar systems; the test finds the sector that contains the loc and then returns its
	// closest solar system to that location.
	const DaSolarSystem* pcss = galaxy->FindClosestSolarSystemInSector(galaxyLoc);
	if(!pcss)
		return false;						// Prohibit movement into sectors with no solar systems.

	// Spacecraft can tow atoms with tractor beams.
	AuIdList towed;
	DaVessel* departShip = dynamic_cast<DaVessel*>(departAtom);
	if(departShip)
		departShip->ListTractorBeamTowed(towed);

	if(!DepartedOnDeadHeading(departAtom, galaxy, ps, pcss))
		return false;

	// Primary ship made it. Take any towed vessels along too.
	for(AuIdList::const_iterator ti = towed.cbegin(); ti != towed.cend(); ti++)
	{
		DaAtom* towedAtom = FindIdRecursive(*ti);
		if(!towedAtom || !towedAtom->IsNormal())
			continue;
		DepartedOnDeadHeading(towedAtom, galaxy, ps, pcss);
	}

	return true;
}

bool DaSolarSystem::DepartedOnDeadHeading(DaAtom* departAtom, const DaGalaxy* galaxy, const DaSector* ps, const DaSolarSystem* pcss)
{
	departAtom->SetTraveling();			// Mark the atom as traveling elsewhere, so it will be ignored.

	// Change the world transformation to be subordinate to the nu parent. This can happen across sector boundaries.
	const DaSector* phs = (const DaSector*)pcss->Parent();
	AuMatrix4x4 ts, tm;
	if(phs == ps)
	{
		// Hand it off directly to the closest solar system.
		//qDebug("DaSolarSystem::DepartedOnDeadHeading - Handoff to nu solar system");
		pcss->GetTransformRelativeToParent(ts, ps);
		departAtom->GetTransformRelativeToParent(tm, ps);
	}
	else
	{
		// The atom is changing sectors.
		//qDebug("DaSolarSystem::DepartedOnDeadHeading - Handoff to nu sector");
		pcss->GetTransformRelativeToParent(ts, galaxy);
		departAtom->GetTransformRelativeToParent(tm, galaxy);
	}

	// Player Statistics
	DaPerson* person = dynamic_cast<DaPerson*>(departAtom);
	if(person)
	{
		person->Stats().EVADeadHead();
		if(phs != ps)
			person->Stats().EVASectorChange();
		person->IncreaseSkill(AuSkill::SpaceExploration, 1);
	}
	DaAtomList people;
	departAtom->FindPeopleAwakeRecursive(people);
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* per = (DaPerson*)*pi;
		per->Stats().DeadHeaded(departAtom);
		if(phs != ps)
			per->Stats().SectorChange(departAtom);
		per->IncreaseSkill(AuSkill::SpaceExploration, 1);
	}

	//qDebug("DaSolarSystem::MoveSubatom - Handoff set transform");
	AuMatrix4x4 t = ts.Inverse() * tm;
	departAtom->ChangePosAndOrient(AuAtomPos(Id(), t));
	departAtom->InvalidateEnvironmentRecursive();
	departAtom->SetModified();

	if(g_Au.m_LogMessages & LOG_Travel)
	{
		QString logmsg = QString("Atom %1 %2 dead-headed from system %3 %4 to system %5 %6.")
			.arg(departAtom->Id().GetID()).arg(departAtom->Name())
			.arg(Id().GetID()).arg(Name())
			.arg(pcss->Id().GetID()).arg(pcss->Name());
		g_Au.Log(logmsg);
	}

	// Cannot handoff the atom to the destination system directly due to mutex locking.
	// We will post the departing atom data as a signal so the app can do the transfer.
	emit Travel(departAtom->Id(), Id(), pcss->Id(), phs->Id());
	//qDebug("DaSolarSystem::DepartOnDeadHeading - Handoff complete");

	return true;
}

// Returns the star with a stargate that is either an origin or destination, depending on 'origin'.
// Returns 0 if none found.
DaStar* DaSolarSystem::FindStargateStar(bool origin) const
{
	DaStar* primary = FindPrimaryStar();
	if(!primary)
		return 0;

	if(primary->HasStargate(origin))
		return primary;

	DaAtomList stars;
	primary->ListStars(stars);
	for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
	{
		DaStar* companion = (DaStar*)(*si);
		if(companion->HasStargate(origin))
			return companion;
	}
	return 0;
}

// Returns the expiration of any positive stargate that is active.
AuTime DaSolarSystem::PositiveStargateExpiry() const
{
	DaStar* primary = FindPrimaryStar();
	if(!primary)
		return AuTime();

	AuTime expiry = primary->PositiveStargateExpiry();
	if(!expiry.IsNull())
		return expiry;

	DaAtomList stars;
	primary->ListStars(stars);
	for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
	{
		DaStar* companion = (DaStar*)(*si);
		expiry = companion->PositiveStargateExpiry();
		if(!expiry.IsNull())
			return expiry;
	}

	return AuTime();
}

void DaSolarSystem::SetLastOfficerVisited()
{
	bool callSetModified = false;
	m_LastOfficerVisitLock.lockForWrite();
	bool wasTicking = !m_LastOfficerVisit.IsNull() && m_LastOfficerVisit.ElapsedSec() <= 60*3;
	m_LastOfficerVisit.Set();
	if(!m_LastOfficerMod.isValid() || m_LastOfficerMod.ElapsedMin() > 10)
	{
		m_LastOfficerMod.restart();
		callSetModified = true;
	}
	m_LastOfficerVisitLock.unlock();
	if(callSetModified)
		SetModified(NOTIFY_Database);
	if(!wasTicking)
		TickTockRecursive();
}

void DaSolarSystem::SetLastPersonScene()
{
	m_LastPersonVisitLock.lockForWrite();
	bool wasTicking = m_LastPersonScene.isValid() && m_LastPersonScene.ElapsedSec() <= 60*3;
	m_LastPersonScene.restart();
	m_LastPersonVisitLock.unlock();
	SetLastPersonVisited();
	if(!wasTicking)
		TickTockRecursive();
}

void DaSolarSystem::SetLastPersonVisited()
{
	// This gets called in every sync frame. Let's not call SetModified in every frame or the client will get an updated copy of
	// this solar system in every frame.
	bool callSetModified = false;
	m_LastPersonVisitLock.lockForWrite();
	m_LastPersonVisit.Set();
	if(!m_LastPersonMod.isValid() || m_LastPersonMod.ElapsedMin() > 10)
	{
		m_LastPersonMod.restart();
		callSetModified = true;
	}
	m_LastPersonVisitLock.unlock();
	if(callSetModified)
		SetModified(NOTIFY_Database);
}

DaStar* DaSolarSystem::HengeSun(quint8 sun)
{
	if(!IsFinal())
		return 0;

	// The primary is easy to find.
	if(sun == 0)
		return FindPrimaryStarNotBlackHole();

	DaStar* primary = FindStar();	// Could be a black hole.
	if(!primary)
		return 0;

	// Locate the companion star.
	DaAtomList subs;
	primary->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* companion = dynamic_cast<DaStar*>(*it);
		if(!companion)
			continue;
		if(companion->StarNumber() == sun)
			return companion;	// Found the desired companion star.
	}
	return 0;
}

bool DaSolarSystem::HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		quint8 toSun, quint8 toArcSegment)
{
	if(!IsFinal())
		return false;

	DaStar* star = HengeSun(toSun);
	if(!star)
		return false;

	return star->HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, toArcSegment);
}

void DaSolarSystem::HengeCreated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		quint8 toSun, quint8 toArcSegment)
{
	const DaSector* sector = FindParentSector();
	if(!sector)
		return;

	if(!IsFinal())
	{
		// My contents are not final. Create a facsimile so we can proceed. The henge destination must be determined.
		// Check my memory to save a lot of time reloading saved solo universe games.
		bool foundIt = false;
		AuId sectorId(sector->Id());
		m_HengeDestLock.lock();
		for(QList<AuHengeDest_>::const_iterator hi = m_HengeDest.cbegin(); hi != m_HengeDest.cend(); hi++)
		{
			if((*hi).IsHenge(sectorId, fromSysId, fromSun, fromShadowPanels, fromArcSegment, toSun, toArcSegment))
			{
				emit HengeDestinationSignal(sectorId, Id(), Name(), (*hi).m_FromSun, (*hi).m_WorldCatName, (*hi).m_WorldName,
						(*hi).m_WorldAtomType, (*hi).m_HengeLL, (*hi).m_HasBreathableAtm, (*hi).m_OrbitZone, (*hi).m_OutsideCity);
				foundIt = true;
				break;
			}
		}
		m_HengeDestLock.unlock();

		if(!foundIt)
		{
			AuEther tempEther("DaSolarSystem::HengeCreated");
			DaSolarSystem* tempSys = new DaSolarSystem;
			tempSys->SetId(Id());
			tempEther.AddAtom(tempSys);
			tempSys->PrepareForResourceScan(CatalogName(), Name(), Galaxy(), GenHab(), GenSeed(), IsCoreBlackHole(), Transform(),
					-1);
			DaStar* star = tempSys->HengeSun(toSun);
			if(star)
				star->HengeCreated(this, sectorId, fromSysId, fromSun, fromShadowPanels, fromArcSegment, toSun, toArcSegment);
		}
	}
	else
	{
		DaStar* star = HengeSun(toSun);
		if(star)
			star->HengeCreated(this, sector->Id(), fromSysId, fromSun, fromShadowPanels, fromArcSegment, toSun, toArcSegment);
	}
}

void DaSolarSystem::HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		quint8 toSun, quint8 toArcSegment)
{
	if(!IsFinal())
		return;

	DaStar* star = HengeSun(toSun);
	if(!star)
		return;

	star->HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, toArcSegment);
}

void DaSolarSystem::HengeTeleportArrived(DaAtom* arrived, quint8 destSun, quint8 destArc, const AuId& fromSysId, quint8 fromSun,
		quint8 fromArc)
{
	DaStar* star = HengeSun(destSun);
	if(!star)
		return;

	star->HengeTeleportArrived(arrived, destArc, fromSysId, fromSun, fromArc);
}

const DaAtom* DaSolarSystem::LocationBlocked(const AuPoint& systemLoc, double pilotAvoidRadius) const
{
	DaAtomList atoms;
	ListNormalSubatomsRecursive(atoms);
	if(atoms.empty())
		return 0;
	for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
	{
		const DaAtom* atom = *ai;
		if(atom->FindParentSolarSystemInEnv() != this)
			continue;	// In a designer or arena?
		double distApart = atom->EnvLocation().Distance(systemLoc);
		if(distApart > atom->PilotAvoidRadius() + pilotAvoidRadius + 100)
			continue;	// Too far apart to matter.
		// Found an atom within the rough radii of 'systemLoc'.
		return atom;
	}
	return 0;	// No blocking atom was found.
}

const DaAtom* DaSolarSystem::WormholeBlocked(const AuPoint& loc) const
{
	// The primary star cannot block its own wormholes.
	DaStar* primary = FindPrimaryStar();
	if(!primary)
		return 0;	// No primary? Not likely.

	// Check companion stars, if any.
	DaAtomList companions;
	primary->ListStars(companions);
	for(DaAtomList::const_iterator ci = companions.cbegin(); ci != companions.cend(); ci++)
	{
		DaStar* companion = (DaStar*)*ci;
		if(companion->EnvLocation().Distance(loc) <= companion->PilotAvoidRadius())
			return companion;	// Blocked by companion star.
	}

	// Check gas giants and globes. We ignore ringworlds and rings because they never block wormholes.
	DaAtomList worlds;
	primary->FindPlanetsRecursive(worlds);
	for(DaAtomList::const_iterator wi = worlds.cbegin(); wi != worlds.cend(); wi++)
	{
		const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(*wi);
		if(glworld)
		{
			if(glworld->EnvLocation().Distance(loc) <= glworld->PilotAvoidRadius())
				return glworld;	// Blocked by globe world.
		}
	}

	// No suns or worlds block the location.
	return 0;
}

void DaSolarSystem::FindATC(const AuPoint& shipLoc, quint32 shipEmpire, const DaVessel*& atcStation, const DaAtom*& atcCity,
		const DaVessel*& enemyStation, const DaAtom*& enemyCity) const
{
	// Init
	atcStation = 0;
	atcCity = 0;
	enemyStation = 0;
	enemyCity = 0;

	// Look for an ATC city and an enemy city that have scanners that can reach loc.
	DaAtomList cities;
	FindBdRecursive(cities);	// Find all bd atoms.
	for(DaAtomList::const_iterator ci = cities.cbegin(); ci != cities.cend(); ci++)
	{
		const DaBd* atom = dynamic_cast<const DaBd*>(*ci);
		if(!atom)
			continue;
		if(!atom->HasSensors() || !atom->SensorsWorking())
			continue;
		if(atom->IsPirate())
			continue;	// Pirate cities do not warn anyone off.

		POLITICAL_STANCE stance = PoliticalStance(atom->EmpireId(), shipEmpire);
		switch(stance)
		{
		case POL_Empire:
			// Empire city overrides any other city found so far.
			if(shipLoc.Distance(atom->EnvLocation()) < atom->SensorRange())
				atcCity = atom;
			break;
		case POL_Friend:
		case POL_Neutral:
		case POL_Vassal:
			// Non-empire friendly city does not override city already found; it could be an empire city.
			if(!atcCity && shipLoc.Distance(atom->EnvLocation()) < atom->SensorRange())
				atcCity = atom;
			break;
		case POL_Enemy:
			// Enemy city does not override city already found to save the math. We don't care which one it is.
			if(!enemyCity && shipLoc.Distance(atom->EnvLocation()) < atom->SensorRange())
				enemyCity = atom;
			break;
		}
	}

	// Look for an ATC station and an enemy station that have scanners that can reach loc.
	DaAtomList vessels;
	FindVesselsRecursive(vessels);
	for(DaAtomList::const_iterator vi = vessels.cbegin(); vi != vessels.cend(); vi++)
	{
		const DaVessel* station = dynamic_cast<const DaVessel*>(*vi);
		if(!station)
			continue;
		if(!station->IsSpaceStation())
			continue;
		if(!station->HasSensors() || !station->SensorsWorking())
			continue;
		if(station->IsPirateWithFreeWill())
			continue;	// Pirate stations do not warn anyone off.

		POLITICAL_STANCE stance = PoliticalStance(station->EmpireId(), shipEmpire);
		switch(stance)
		{
		case POL_Empire:
			// Empire station overrides any other station found so far.
			if(shipLoc.Distance(station->EnvLocation()) < station->SensorRange())
				atcStation = station;
			break;
		case POL_Friend:
		case POL_Neutral:
		case POL_Vassal:
			// Non-empire friendly station does not override station already found; it could be an empire station.
			if(!atcStation && shipLoc.Distance(station->EnvLocation()) < station->SensorRange())
				atcStation = station;
			break;
		case POL_Enemy:
			// Enemy station does not override station already found to save the math. We don't care which one it is.
			if(!enemyStation && shipLoc.Distance(station->EnvLocation()) < station->SensorRange())
				enemyStation = station;
			break;
		}
	}
}

void DaSolarSystem::DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc)
{
	// Systems do not detect their own emissions.
	if(Is(systemId))
		return;
	DaAtom::DetectWarpSignatureRecursive(systemId, warpFactors, galaxyLoc);
}

// Only valid on scene master. Only used to provide 'detected systems' in starfield information.
void DaSolarSystem::DetectedBy(quint32 empire, quint32 signalStrength)
{
	//qDebug() << "DaSolarSystem::DetectedBy" << Id() << Name() << "empire" << empire << "signalStrength" << signalStrength
		//<< "already explored" << IsExploredBy(empire) << "already detected" << DetectedSignalStrength(empire);
	//Q_ASSERT(g_Au.IsSceneMaster());
	// Add empire id to detected-by list.
	m_DetectedByLock.lockForWrite();
	unordered_map<quint32, AuSysDet_>::iterator di = m_DetectedBy.find(empire);
	if(di == m_DetectedBy.end())
		m_DetectedBy.insert(pair<quint32, AuSysDet_>(empire, AuSysDet_(signalStrength)));
	else
		(*di).second.UpdateSignalStrength(signalStrength);

	// Remove detections that are more than 48 hours old, for systems that stop broadcasting.
	di = m_DetectedBy.begin();
	while(di != m_DetectedBy.end())
	{
		if((*di).second.m_When.ElapsedSec() < 60*60*48)	// Check for detection within the past 48 hours.
			di++;	// Detection is fresh enough.
		else
			di = m_DetectedBy.erase(di);
	}

	m_DetectedByLock.unlock();
	SetModified(NOTIFY_Database);	// Detected system information is only stored by the master server.
}

void DaSolarSystem::ExploredBy(quint32 empire)
{
	if(!empire)
		return;	// Empire 0 does not explore systems.
	if(IsExploredBy(empire))
		return;

	// Add empire id to explored list.
	m_ExploredByLock.lockForWrite();
	m_ExploredBy.append(empire);
	m_ExploredByLock.unlock();

	// Only the scene worker responsible for this solar system updates the database.
	if(!g_Au.IsSceneWorker())
		return;

	if(IsOnlyStarFieldLoaded())
		return;

	SetModified(NOTIFY_All);
	emit SolarSystemExplored(Id(), empire);
}

void DaSolarSystem::ScannedBy(quint32 empire, HABITABILITY genHab, quint32 genSeed)
{
	if(!empire)
		return;	// Empire 0 does not scan systems.
	// Add empire id to explored list.
	m_ScannedByLock.lockForWrite();
	m_ScannedBy.insert(empire);
	m_ScannedByLock.unlock();

	if(g_Au.IsSceneMaster() || g_Au.IsSceneManager())
	{
		// The scene master may not have the genHab and genSeed values from the finalized solar system. The client sends
		// them to us as a convenience when a system is scanned. They need to accompany starmap data so the client can regen
		// the resources properly for the scan report.
		if(m_GenHab != genHab)
			qWarning() << "warning: DaSolarSystem::ScannedBy" << Id() << Name() << "my genHab" << m_GenHab
				<< "scanned genHab" << genHab;
		m_GenHab = genHab;
		m_GenSeed = genSeed;
	}

	// Only the scene worker responsible for this solar system updates the database.
	if(!g_Au.IsSceneWorker())
		return;

	if(IsOnlyStarFieldLoaded())
		return;

	SetModified(NOTIFY_All);
	emit SolarSystemScanned(Id(), empire, m_GenHab, m_GenSeed);
}

void DaSolarSystem::CommodityAvailable(COMMODITYID cid, const AuId& bdId, quint32 empireId)
{
	m_CommodityExchangeLock.lockForWrite();
	m_CommodityExchange.AddSupplier(cid, bdId, empireId);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityNotAvailable(COMMODITYID cid, const AuId& bdId)
{
	m_CommodityExchangeLock.lockForWrite();
	m_CommodityExchange.RemoveSupplier(cid, bdId);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommoditySuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers)
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.ListSuppliers(cid, importEmpireId, suppliers);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommoditySupplierUpdate()
{
	// We will use a list of all bds and cities so there are no locking issues.
	DaAtomList bdsAndCities;
	FindBdRecursive(bdsAndCities);
	m_CommodityExchangeLock.lockForWrite();
	for(DaAtomList::const_iterator bi = bdsAndCities.cbegin(); bi != bdsAndCities.cend(); bi++)
		(*bi)->CommoditySupplies(m_CommodityExchange);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::UpdateTradeConnections()
{
	// We will use a list of all bds and cities so there are no locking issues.
	DaAtomList bds;
	FindBdRecursive(bds);

	// Must do all the ground connections first, so each bd knows if it is connected to a wharf and/or airport.
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		bd->UpdateGroundTradeConnections();
	}

	// Now we can do the sea connections, so each bd knows if it is connected to a wharf.
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		bd->UpdateSeaTradeConnections();
	}

	// Finally we can do the air/space connections, knowing wharf and airport status of all buildings.
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = (DaBd*)*bi;
		bd->UpdateOffWorldTradeConnections(bds);
	}
}

void DaSolarSystem::CommodityCantinaSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.CantinaSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityHQSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.MilitaryHQSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityAirBaseSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.MilitaryAirBaseSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityDockSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.MilitaryDockSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityMotorPoolSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.MilitaryMotorPoolSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityGrocerySupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.GrocerySupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityRetailSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.RetailSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityFindSupplies(QList<COMMODITYID>& cidsAvailable, const QList<COMMODITYID>& cidsSought,
		const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.FindSupplies(cidsAvailable, cidsSought, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::CommodityHospitalSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const
{
	m_CommodityExchangeLock.lockForRead();
	m_CommodityExchange.HospitalSupplies(cidsAvailable, tradeConnections);
	m_CommodityExchangeLock.unlock();
}

bool DaSolarSystem::IsTickingContents() const
{
	if(IsDead() || IsDying())
		return true;	// Must return true to finish dying.

	// Skip ticking solar systems that are loaded only for the star field.
	// NOTE: This means they won't die if assigned a time to die and they won't be removed when "Kill"ed.
	if(IsOnlyStarFieldLoaded())
		return false;

	// This time needs to be at least one full minute because officers call SetLastVisited once per minute.
	// Ticking is needed to keep their spacecraft moving.
	m_LastOfficerVisitLock.lockForRead();
	bool v = !m_LastOfficerVisit.IsNull() && m_LastOfficerVisit.ElapsedSec() <= 60*3;
	m_LastOfficerVisitLock.unlock();
	if(v)
		return true;

	// People call SetLastPersonScene on their current solar system every time they get a scene sync in DtSceneServer::SyncClient.
	// Ticking is needed so thinks around them continue moving.
	m_LastPersonVisitLock.lockForRead();
	bool tickMe = m_LastPersonScene.isValid() && m_LastPersonScene.ElapsedSec() <= 60*3;
	m_LastPersonVisitLock.unlock();
	return tickMe;
}

quint64 DaSolarSystem::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaSolarSystem) - sizeof(DaAtom);
	mem += StringMemoryUsage(m_CatalogName);
	mem += (sizeof(quint32) + sizeof(AuSysDet_)) * m_DetectedBy.size();
	mem += (sizeof(quint32) + sizeof(AuElapsedTimer)) * m_EmpiresPresent.size();
	mem += sizeof(quint32) * m_ExploredBy.capacity();
	mem += sizeof(quint32) * m_ScannedBy.capacity();
	mem += sizeof(m_LastAvatarAccountActiveLock);
	mem += sizeof(m_LastAvatarAccountActive);
	mem += StringMemoryUsage(m_sName);
	mem += m_Wormholes.MemoryUsage();
	mem += m_CommodityExchange.MemoryUsage();
	return mem;
}

DaBd* DaSolarSystem::FindParentBdInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaBd* DaSolarSystem::FindParentBdInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaCapsule* DaSolarSystem::FindParentCapsuleInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaCapsule* DaSolarSystem::FindParentCapsuleInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaDesignBase* DaSolarSystem::FindParentDesignBaseInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaDesignBase* DaSolarSystem::FindParentDesignBaseInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaDesigner* DaSolarSystem::FindParentDesignerInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaDesigner* DaSolarSystem::FindParentDesignerInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorld* DaSolarSystem::FindParentPlanetInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorld* DaSolarSystem::FindParentPlanetInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaSolarSystem* DaSolarSystem::FindParentSolarSystemInEnv()
{
	return this;
}

const DaSolarSystem* DaSolarSystem::FindParentSolarSystemInEnv() const
{
	return this;
}

DaStar* DaSolarSystem::FindParentStarInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaStar* DaSolarSystem::FindParentStarInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaVessel* DaSolarSystem::FindParentVesselInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaVessel* DaSolarSystem::FindParentVesselInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorldLL* DaSolarSystem::FindParentWorldInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorldLL* DaSolarSystem::FindParentWorldInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

void DaSolarSystem::WarpEngaged(quint32 warpFactors)
{
	bool callSetModified = false;
	m_WarpSignatureLock.lock();
	quint32 oldSig = m_WarpSignature;
	m_WarpSignature += warpFactors;
	if(!m_LastWarpMod.isValid() || m_LastWarpMod.ElapsedMin() > 10)
	{
		m_LastWarpMod.restart();
		callSetModified = true;
	}
	m_WarpSignatureLock.unlock();
	if(callSetModified)
		SetModified(NOTIFY_Database);
	if(!oldSig)
		TickTock();
}

const DaSolarSystem* DaSolarSystem::FindParentSolarSystem() const
{
	return this;
}

DaSolarSystem* DaSolarSystem::FindParentSolarSystem()
{
	return this;
}

bool DaSolarSystem::HasPirateBase(quint32& empireId) const
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId base = gd.PirateBaseInSystem(Id(), empireId);	// Check for existing pirate base in this system.
	g_Au.UnlockData();
	return !base.IsNull();
}

void DaSolarSystem::PirateBaseEncounter()
{
	quint32 empireId;
	//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "final" << IsFinal() << "hasBase" << HasPirateBase(empireId);
	if(!IsFinal())
	{
		//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "NOT final";
		return;		// This system is not final?!
	}
	if(HasPirateBase(empireId))
	{
		//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "pirate base already here, empire" << empireId;
		return;		// A base is already here.
	}
	// Don't spawn a pirate base while any non-pirate units are present.
	m_EmpiresPresentLock.lockForRead();
	bool epres = false;
	for(unordered_map<quint32, AuElapsedTimer>::const_iterator ei = m_EmpiresPresent.cbegin(); ei != m_EmpiresPresent.cend(); ei++)
		if(!DgEmpire::IsPirate((*ei).first))
		{
			epres = true;	// A non-pirate is present in this system.
			break;
		}
	m_EmpiresPresentLock.unlock();
	if(epres)
	{
		//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "no encounter, non-pirate empires present";
		return;
	}

	// Choose a world for our pirate base. It must be a world that can be claimed by the pirate empire.
	DaAtom* world = FindHabitableWorld(HABITABLE_System);
	if(!world)
		world = FindHabitableWorld(HABITABLE_Unspecified);
	if(!world)
	{
		// Nothing remotely habitable.
		DaAtomList planets;
		FindPlanetsRecursive(planets);
		if(planets.empty())
		{
			//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "no worlds at all";
			return;	// No worlds at all.
		}
		DaAtomList worlds;
		for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
		{
			DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(*pi);
			if(!llworld)
				continue;	// Skip non ll worlds. We would already have found ringworlds in FindHabitableWorld.
			if(llworld->Hydrographics() > 90)
				continue;	// Skip water/lava worlds.
			if(llworld->IsGasGiant())
				continue;	// Skip gas giants.
			if(llworld->BdHere() ||
					llworld->BaseHere() ||
					llworld->HQHere())
				continue;	// Skip world with any cities, bases or buildings.
			if(llworld->FindVessel())
				continue;	// Skip world with any spacecraft in the atmosphere or in orbit.
			if(llworld->FindPersonOnLine())
				continue;	// Skip world with any person who is on line.
			worlds.append(llworld);
		}
		if(worlds.empty())
		{
			//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "no usable worlds";
			return;	// No usable worlds.
		}
		if(worlds.size() == 1)
			world = worlds.first();
		else
			world = worlds[g_Au.Dice().Roll(1, (unsigned int)worlds.size(), -1)];
	}
	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(world);
	if(!llworld)
	{
		//qDebug() << "DaSolarSystem::PirateBaseEncounter" << *this << "world" << *world << "is not an LL world?";
		return;
	}
	llworld->PirateBaseEncounter();
}

DaAtom::ATOM_TYPE DaSolarSystem::AtomType() const
{
	return ATOM_SolarSystem;
}

bool DaSolarSystem::StarMapSearch(const AuStarMapSystemSearch& search) const
{
	// Name already checked by caller.
	// Range already checked by caller.
	DaStar* star = FindPrimaryStar();
	if(!star)
		return false;	// wut? No primary?

	// Create a list of all the stars and planets in the solar system.
	DaAtomList starsAndPlanets;

	// Append the primary star to the list.
	starsAndPlanets.append(star);

	// Append companion stars to the list. They are always subordinate to the primary.
	star->ListStars(starsAndPlanets);

	if(search.m_Stars)
	{
		if(starsAndPlanets.size() < search.m_Stars)
			return false;	// Less stars than the caller wanted.
		// Tally what kinds of stars we have.
		bool blackHole = false;
		bool neutronStar = false;
		bool starTypeO = false;
		bool starTypeB = false;
		bool starTypeA = false;
		bool starTypeF = false;
		bool starTypeG = false;
		bool starTypeK = false;
		bool starTypeM = false;
		for(DaAtomList::const_iterator spi = starsAndPlanets.begin(); spi != starsAndPlanets.end(); spi++)
		{
			const DaStar* star = dynamic_cast<const DaStar*>(*spi);
			if(!star)
				continue;
			switch(star->StarType())
			{
			case DaStar::TYPE_O:
				starTypeO = true;
				break;
			case DaStar::TYPE_B:
				starTypeB = true;
				break;
			case DaStar::TYPE_A:
				starTypeA = true;
				break;
			case DaStar::TYPE_F:
				starTypeF = true;
				break;
			case DaStar::TYPE_G:
				starTypeG = true;
				break;
			case DaStar::TYPE_K:
				starTypeK = true;
				break;
			case DaStar::TYPE_M:
				starTypeM = true;
				break;
			case DaStar::TYPE_SupermassiveBlackHole:
			case DaStar::TYPE_StellarBlackHole:
				blackHole = true;
				break;
			case DaStar::TYPE_NeutronStar:
				neutronStar = true;
				break;
			}
		}
		bool foundSun = (search.m_BlackHole && blackHole) ||
			(search.m_NeutronStar && neutronStar) ||
			(search.m_StarTypeO && starTypeO) ||
			(search.m_StarTypeB && starTypeB) ||
			(search.m_StarTypeA && starTypeA) ||
			(search.m_StarTypeF && starTypeF) ||
			(search.m_StarTypeG && starTypeG) ||
			(search.m_StarTypeK && starTypeK) ||
			(search.m_StarTypeM && starTypeM);
		if(!foundSun)
			return false;
	}

	// Append all planets and moons to the list.
	star->FindPlanetsRecursive(starsAndPlanets);

	// Worlds
	if(search.m_Worlds)
	{
		// Tally what kinds of worlds we have.
		bool gasGiantInferno = false;
		bool gasGiantInner = false;
		bool gasGiantHabitable = false;
		bool gasGiantOuter = false;
		bool gasGiantFrigid = false;
		int globeInferno = 0;
		int globeInner = 0;
		int globeHabitable = 0;
		int globeOuter = 0;
		int globeFrigid = 0;
		bool ringInferno = false;
		bool ringInner = false;
		bool ringHabitable = false;
		bool ringOuter = false;
		bool ringFrigid = false;
		int ringworldArcs = 0;
		int worlds = 0;
		for(DaAtomList::const_iterator spi = starsAndPlanets.begin(); spi != starsAndPlanets.end(); spi++)
		{
			const DaWorld* world = dynamic_cast<const DaWorld*>(*spi);
			if(!world)
				continue;	// a sun?
			const DaWorldGasGiant* gg = dynamic_cast<const DaWorldGasGiant*>(world);
			if(gg)
			{
				switch(gg->OrbitZone())
				{
				case OZONE_Inferno:
					gasGiantInferno = true;
					break;
				case OZONE_Inner:
					gasGiantInner = true;
					break;
				case OZONE_Habitable:
					gasGiantHabitable = true;
					break;
				case OZONE_Outer:
					gasGiantOuter = true;
					break;
				case OZONE_Frigid:
					gasGiantFrigid = true;
					break;
				case OZONE_DeepSpace:
					break;
				}
				worlds++;
			}
			const DaWorldRing* ring = dynamic_cast<const DaWorldRing*>(world);
			if(ring)
			{
				switch(ring->OrbitZone())
				{
				case OZONE_Inferno:
					ringInferno = true;
					break;
				case OZONE_Inner:
					ringInner = true;
					break;
				case OZONE_Habitable:
					ringHabitable = true;
					break;
				case OZONE_Outer:
					ringOuter = true;
					break;
				case OZONE_Frigid:
					ringFrigid = true;
					break;
				case OZONE_DeepSpace:
					break;
				}
				worlds++;
			}
			const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(world);
			if(globe)
			{
				switch(globe->OrbitZone())
				{
				case OZONE_Inferno:
					if(globeInferno < globe->Radius100km())
						globeInferno = globe->Radius100km();		// Remember size of largest globe in the zone.
					break;
				case OZONE_Inner:
					if(globeInner < globe->Radius100km())
						globeInner = globe->Radius100km();			// Remember size of largest globe in the zone.
					break;
				case OZONE_Habitable:
					if(globeHabitable < globe->Radius100km())
						globeHabitable = globe->Radius100km();		// Remember size of largest globe in the zone.
					break;
				case OZONE_Outer:
					if(globeOuter < globe->Radius100km())
						globeOuter = globe->Radius100km();			// Remember size of largest globe in the zone.
					break;
				case OZONE_Frigid:
					if(globeFrigid < globe->Radius100km())
						globeFrigid = globe->Radius100km();			// Remember size of largest globe in the zone.
					break;
				case OZONE_DeepSpace:
					break;
				}
				worlds++;
			}
			const DaWorldRingSection* rw = dynamic_cast<const DaWorldRingSection*>(world);
			if(rw)
			{
				ringworldArcs++;
				if(rw->SectionIndex() == 0)
					worlds++;	// Count one world for each entire ringworld by counting arc section 0, which is the control center.
			}
		}

		if(search.m_GasGiantInferno ||
				search.m_GasGiantInner ||
				search.m_GasGiantHabitable ||
				search.m_GasGiantOuter ||
				search.m_GasGiantFrigid)
		{
			// Gas giant sought.
			bool ggok = false;
			if(search.m_GasGiantInferno && gasGiantInferno)
				ggok = true;
			if(search.m_GasGiantInner && gasGiantInner)
				ggok = true;
			if(search.m_GasGiantHabitable && gasGiantHabitable)
				ggok = true;
			if(search.m_GasGiantOuter && gasGiantOuter)
				ggok = true;
			if(search.m_GasGiantFrigid && gasGiantFrigid)
				ggok = true;
			if(!ggok)
				return false;
		}

		if(search.m_GlobeInferno ||
				search.m_GlobeInner ||
				search.m_GlobeHabitable ||
				search.m_GlobeOuter ||
				search.m_GlobeFrigid)
		{
			// Globe sought.
			bool glok = false;
			if(search.m_GlobeInferno && globeInferno >= search.m_GlobeInferno)
				glok = true;
			if(search.m_GlobeInner && globeInner >= search.m_GlobeInner)
				glok = true;
			if(search.m_GlobeHabitable && globeHabitable >= search.m_GlobeHabitable)
				glok = true;
			if(search.m_GlobeOuter && globeOuter >= search.m_GlobeOuter)
				glok = true;
			if(search.m_GlobeFrigid && globeFrigid >= search.m_GlobeFrigid)
				glok = true;
			if(!glok)
				return false;
		}

		if(search.m_RingInferno ||
				search.m_RingInner ||
				search.m_RingHabitable ||
				search.m_RingOuter ||
				search.m_RingFrigid)
		{
			// Ring sought.
			bool rok = false;
			if(search.m_RingInferno && ringInferno)
				rok = true;
			if(search.m_RingInner && ringInner)
				rok = true;
			if(search.m_RingHabitable && ringHabitable)
				rok = true;
			if(search.m_RingOuter && ringOuter)
				rok = true;
			if(search.m_RingFrigid && ringFrigid)
				rok = true;
			if(!rok)
				return false;
		}

		// Ringworlds
		if(search.m_RingworldArcs && ringworldArcs < search.m_RingworldArcs)
			return false;

		// Worlds in general
		if(search.m_Worlds && worlds < search.m_Worlds)
			return false;
	}

	// Natural resources
	if(!search.m_NaturalResources.empty())
	{
		if(search.m_AndOr)
		{
			// Must find at least one of the desired natural resources at the minimum Q.
			bool nifound = false;
			for(QList<int>::const_iterator ni = search.m_NaturalResources.cbegin(); ni != search.m_NaturalResources.cend(); ni++)
			{
				// Determine if any star or planet has the natural resource.
				for(DaAtomList::const_iterator spi = starsAndPlanets.begin(); spi != starsAndPlanets.end(); spi++)
					if((*spi)->FindNaturalResource((COMMODITYID)*ni, search.m_MinQ))
					{
						nifound = true;	// The natural resource was found in this system, at the minimum Q.
						break;
					}
				if(nifound)
					break;
			}
			if(!nifound)
				return false;	// None of the natural resources was found in this system, at the minimum Q.
		}
		else
		{
			// Must find all desired natural resources at the minimum Q.
			for(QList<int>::const_iterator ni = search.m_NaturalResources.cbegin(); ni != search.m_NaturalResources.cend(); ni++)
			{
				// Determine if any star or planet has the natural resource.
				bool nifound = false;
				for(DaAtomList::const_iterator spi = starsAndPlanets.begin(); spi != starsAndPlanets.end(); spi++)
					if((*spi)->FindNaturalResource((COMMODITYID)*ni, search.m_MinQ))
					{
						nifound = true;
						break;
					}
				if(!nifound)
					return false;	// The natural resource was not found in this system, at the minimum Q.
			}
		}
	}

	// Ran the gauntlet so this system matches the star map search.
	return true;
}

void DaSolarSystem::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	quint8 ver = 1;
	quint8 verGenHab = (ver << 4) | m_GenHab;

	out << m_Final;
	out << verGenHab;
	out << m_GenSeed;
	out << m_HighestQShip;
	out << m_isOnlyStarFieldLoaded;
	out << m_StrongGravity;
	out << m_WormholesFinalized;

	m_HengeDestLock.lock();
	out << m_HengeDest;
	m_HengeDestLock.unlock();

	m_LastAvatarAccountActiveLock.lock();
	out << m_LastAvatarAccountActive;
	m_LastAvatarAccountActiveLock.unlock();

	m_CatalogNameLock.lockForRead();
	out << m_CatalogName;
	out << m_CatalogNameValid;
	m_CatalogNameLock.unlock();

	m_DetectedByLock.lockForRead();
	out << (quint32)m_DetectedBy.size();
	for(unordered_map<quint32, AuSysDet_>::const_iterator di = m_DetectedBy.cbegin(); di != m_DetectedBy.cend(); di++)
	{
		out << (*di).first;
		out << (*di).second;
	}
	m_DetectedByLock.unlock();

	m_EmpiresPresentLock.lockForRead();
	out << (quint32)m_EmpiresPresent.size();
	for(unordered_map<quint32, AuElapsedTimer>::const_iterator ei = m_EmpiresPresent.cbegin(); ei != m_EmpiresPresent.cend(); ei++)
	{
		out << (*ei).first;
		AuTime tm;
		out << tm;
	}
	m_EmpiresPresentLock.unlock();

	m_ExploredByLock.lockForRead();
	out << m_ExploredBy;
	m_ExploredByLock.unlock();

	m_ScannedByLock.lockForRead();
	out << m_ScannedBy;
	m_ScannedByLock.unlock();

	m_LastFlagInfluenceLock.lockForRead();
	out << m_LastFlagInfluence;
	m_LastFlagInfluenceLock.unlock();

	m_LastOfficerVisitLock.lockForRead();
	AuTime lastOfficerMod;
	out << lastOfficerMod;
	out << m_LastOfficerVisit;
	m_LastOfficerVisitLock.unlock();

	m_LastPersonVisitLock.lockForRead();
	AuTime lastPersonMod;
	out << lastPersonMod;
	AuTime lastPersonScene;
	out << lastPersonScene;
	out << m_LastPersonVisit;
	m_LastPersonVisitLock.unlock();

	m_LastPirateLock.lockForRead();
	AuTime lastPirateShip;
	out << lastPirateShip;
	AuTime lastPirateStation;
	out << lastPirateStation;
	m_LastPirateLock.unlock();

	m_NameLock.lockForRead();
	out << m_sName;
	m_NameLock.unlock();

	m_WarpSignatureLock.lock();
	AuTime lastWarpMod;
	out << lastWarpMod;
	out << m_WarpSignature;
	m_WarpSignatureLock.unlock();

	m_StarLightLock.lockForRead();
	out << m_StarLight;
	m_StarLightLock.unlock();

	m_WormholesLock.lockForRead();
	out << m_Wormholes;
	m_WormholesLock.unlock();

	m_CommodityExchangeLock.lockForRead();
	out << m_CommodityExchange;
	m_CommodityExchangeLock.unlock();
}

void DaSolarSystem::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	quint8 verGenHab;
	in >> m_Final;
	in >> verGenHab;
	m_GenHab = (HABITABILITY)(verGenHab & 0x0f);
	quint8 ver = verGenHab >> 4;
	in >> m_GenSeed;
	in >> m_HighestQShip;
	in >> m_isOnlyStarFieldLoaded;
	in >> m_StrongGravity;
	in >> m_WormholesFinalized;

	if(ver >= 1)
	{
		m_HengeDestLock.lock();
		in >> m_HengeDest;
		m_HengeDestLock.unlock();
	}

	m_LastAvatarAccountActiveLock.lock();
	in >> m_LastAvatarAccountActive;
	m_LastAvatarAccountActiveLock.unlock();

	m_CatalogNameLock.lockForWrite();
	in >> m_CatalogName;
	in >> m_CatalogNameValid;
	m_CatalogNameLock.unlock();

	m_DetectedByLock.lockForWrite();
	m_DetectedBy.clear();
	quint32 ui32;
	in >> ui32;
	while(ui32--)
	{
		quint32 id;
		AuSysDet_ d;
		in >> id;
		in >> d;
		m_DetectedBy.insert(pair<quint32, AuSysDet_>(id, d));
	}
	m_DetectedByLock.unlock();

	m_EmpiresPresentLock.lockForWrite();
	m_EmpiresPresent.clear();
	in >> ui32;
	while(ui32--)
	{
		quint32 id;
		AuTime t;
		in >> id;
		in >> t;
		m_EmpiresPresent.insert(pair<quint32, AuElapsedTimer>(id, AuElapsedTimer::Now()));
	}
	m_EmpiresPresentLock.unlock();

	m_ExploredByLock.lockForWrite();
	in >> m_ExploredBy;
	m_ExploredByLock.unlock();

	m_ScannedByLock.lockForWrite();
	in >> m_ScannedBy;
	m_ScannedByLock.unlock();

	m_LastFlagInfluenceLock.lockForWrite();
	in >> m_LastFlagInfluence;
	m_LastFlagInfluenceLock.unlock();

	m_LastOfficerVisitLock.lockForWrite();
	AuTime lastOfficerMod;
	in >> lastOfficerMod;
	in >> m_LastOfficerVisit;
	m_LastOfficerVisitLock.unlock();

	m_LastPersonVisitLock.lockForWrite();
	AuTime lastPersonMod;
	in >> lastPersonMod;
	AuTime lastPersonScene;
	in >> lastPersonScene;
	in >> m_LastPersonVisit;
	m_LastPersonVisitLock.unlock();

	m_LastPirateLock.lockForWrite();
	AuTime lastPirateShip;
	in >> lastPirateShip;
	AuTime lastPirateStation;
	in >> lastPirateStation;
	m_LastPirateLock.unlock();

	m_NameLock.lockForWrite();
	in >> m_sName;
	m_NameLock.unlock();

	m_WarpSignatureLock.lock();
	AuTime lastWarpMod;
	in >> lastWarpMod;
	in >> m_WarpSignature;
	m_WarpSignatureLock.unlock();

	m_StarLightLock.lockForWrite();
	in >> m_StarLight;
	m_StarLightLock.unlock();

	m_WormholesLock.lockForWrite();
	in >> m_Wormholes;
	m_WormholesLock.unlock();

	m_CommodityExchangeLock.lockForWrite();
	in >> m_CommodityExchange;
	m_CommodityExchangeLock.unlock();
}

bool DaSolarSystem::IsSystem(const QString& systemName, GALAXY galaxy, HABITABILITY genHab, quint32 genSeed, bool isCoreBlackHole,
		const AuPoint& systemLoc, float starLight)
{
	Q_UNUSED(galaxy);
	Q_UNUSED(isCoreBlackHole);
	Q_UNUSED(starLight);
	if(systemName != Name())
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "systemName" << systemName << "differs from Name()" << Name();
#endif
		return false;
	}
	/* System prepared for scan does not retain this information.
	if(galaxy != Galaxy())
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "galaxy" << galaxy << "differs from Galaxy()" << Galaxy();
#endif
		return false;
	}
	*/
	if(genHab != m_GenHab)
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "genHab" << galaxy << "differs from m_GenHab" << m_GenHab;
#endif
		return false;
	}
	if(genSeed != m_GenSeed)
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "genSeed" << galaxy << "differs from m_GenSeed" << m_GenSeed;
#endif
		return false;
	}
	/* System prepared for scan does not retain this information.
	if(isCoreBlackHole != IsCoreBlackHole())
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "galaxy" << galaxy << "differs from Galaxy()" << Galaxy();
#endif
		return false;
	}
	*/
	// systemLoc is in galaxy coordinates. Convert to sector coordinates.
	AuPoint sectorLoc(systemLoc);
	sectorLoc.Round(10.);
	AuPoint systemLocSector = systemLoc - sectorLoc;
	if(systemLocSector != Location())
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "systemLoc" << systemLoc << "differs from Location()" << Location();
#endif
		return false;
	}
	/* System prepared for scan does not retain this information.
	if(starLight != StarLight())
	{
#if !defined(QT_NO_DEBUG)
		qDebug() << "DaSolarSystem::IsSystem" << *this << "starLight" << starLight << "differs from StarLight()" << StarLight();
#endif
		return false;
	}
	*/
	return true;
}

AuTime DaSolarSystem::AsteroidImpactTime(const AuId& targetId, AuPoint& asteroidLoc) const
{
	const DaAsteroid* roid = FindAsteroidRecursive(targetId);
	if(!roid)
		return AuTime();
	asteroidLoc = roid->EnvLocation();
	return roid->TargetTime();
}

void DaSolarSystem::AvatarAccountActiveRecursive(const AuIdList& land)
{
	if(IsOnlyStarFieldLoaded())
		return;
	if(!land.contains(Id()))
		return;
	// Nobody below me cares. DaAtom::AvatarAccountActiveRecursive(land);
	if(IsDead())
		return;

	// An avatar whose home is in this system has pinged that its account is active.
	//qDebug() << "DaSolarSystem::AvatarAccountActiveRecursive" << *this;
	SetLastAvatarAccountActive();
	SetModified(NOTIFY_Database);
}

float DaSolarSystem::AsteroidBeltDensity(const AuPoint& envLoc) const
{
	return AsteroidBeltDensityRecursive(envLoc);
}

AuPoint DaSolarSystem::GalaxyLoc() const
{
	const DaGalaxy* galaxy = FindParentGalaxy();
	if(!galaxy)
		return AuPoint(0,0);

	// This location will be in parsecs. System locations are rounded to one decimal place so we don't expect 1/100th's.
	AuPoint gloc;
	GetLocationRelativeToParent(gloc, galaxy);
	return gloc;
}

const DaStar* DaSolarSystem::FindDestinationStar(const AuId& id, const QString& name) const
{
	const DaAtom* star = FindIdRecursive(id);
	if(star)
		return dynamic_cast<const DaStar*>(star);

	// Didn't find it by id, try the name.
	star = FindPrimaryStar();
	if(!star)
		return 0;
	if(star->Name() == name)
		return dynamic_cast<const DaStar*>(star);

	DaAtomList stars;
	((const DaStar*)star)->ListStars(stars);
	for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
	{
		star = *si;
		if(star->Name() == name)
			return dynamic_cast<const DaStar*>(star);
	}
	
	return 0;
}

const DaWorld* DaSolarSystem::FindDestinationWorld(const AuId& id, const QString& name) const
{
	const DaAtom* world = FindIdRecursive(id);
	if(world)
		return dynamic_cast<const DaWorld*>(world);

	DaAtomList planets;
	FindPlanetsRecursive(planets);

	// First pass, compare literal planet names.
	for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
	{
		const DaWorld* world = (const DaWorld*)(*pi);
		if(world->Name() == name)
			return world;
	}

	// Second pass, compare catalog names in case the order was created using system scan data. This takes more work.
	QString myCat(CatalogName());
	QString myName(Name());
	for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
	{
		const DaWorld* world = (const DaWorld*)(*pi);
		if(world->DefaultName(myCat) == name || world->DefaultName(myName) == name)
			return world;
	}

	return 0;
}

bool DaSolarSystem::CouldHaveRingworld() const
{
	if(GenHab() == HABITABLE_EmptySystem)
		return false;

	AuDice dice(m_GenSeed);
	// Determine the general nature of the system: solitary, binary, or trinary
	// The core is always at least a binary, so blackhole always has a companion star.
	bool isCoreBlackHole = IsCoreBlackHole();
	int lNature = dice.Roll(2,6, isCoreBlackHole ? 6 : 0);	// +6 guarantees a secondary star.
	bool isStellarBlackHole = false;
	if(!isCoreBlackHole && m_GenHab == HABITABLE_System)
	{
		// Check to see if this is a stellar black hole system.
		int nat = dice.Roll(1,100);
		if(nat <= 2)	// 2% chance?
		{
			isStellarBlackHole = true;
			lNature = dice.Roll(2,6, 6);	// +6 guarantees a secondary star.
		}
	}
	int lStars;
	if(lNature >= 12)
		lStars = 3;	// trinary system
	else if(lNature >= 8)
		lStars = 2;	// binary system
	else
		lStars = 1;	// solitary system

	GALAXY galaxy = Galaxy();
	DaAtomList starsInSystem;
	int lCompTypeMod=0,lCompSizeMod=0;
	DaStar* pPrimary = 0;
	QList<int> starNumbers;
	for(int lStar=0; lStar<lStars; lStar++)
	{
		DaStar* pStar = new DaStar;
		starsInSystem.append(pStar);
		if(lStar == 0)
		{
			// primary star
			pPrimary = pStar;
			if(isCoreBlackHole)
			{
				// Create the black hole star.
				pStar->GenSupermassiveBlackHole(dice, galaxy, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(galaxy);
			}
			else if(isStellarBlackHole)
			{
				// Create the black hole star.
				pStar->GenStellarBlackHole(dice, lCompTypeMod, lCompSizeMod);

				// Init orbit zones now that we know the type and size of the star.
				pStar->InitBlackHoleZones(GALAXY_Count);
			}
			else
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					lCompTypeMod = pStar->GenPrimaryType(dice);
					dice.Random();
					lCompSizeMod = pStar->GenPrimarySize(dice);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
		}
		else
		{
			// Companion stars orbit the primary.
			int companion;
			if(m_GenHab == HABITABLE_System && (pPrimary->IsAnyBlackHole() || pPrimary->IsNeutronStar()))
				companion = lStar;	// Push companions of black holes and neutron stars farther out.
			else
				companion = lStar-1;
			char chCompanionOrbit = pPrimary->GenCompanionOrbit(dice, companion);
			dice.Random();
			Q_ASSERT(chCompanionOrbit >= 0);
			pStar->SetOrbit(chCompanionOrbit);

			// Determine maximum number of orbits.
			// Minimum orbit that can be occupied by the companion star.
			char chMinOrb = pPrimary->MinOrbitAvailableFrom(chCompanionOrbit);
			// Maximum number of orbits for companion.
			char chCompMaxOrbits = chCompanionOrbit - chMinOrb;
			if(chCompMaxOrbits > chCompanionOrbit/2)
			{
				chCompMaxOrbits = chCompanionOrbit/2;
				chMinOrb = chCompanionOrbit - chCompMaxOrbits;
			}
			pStar->SetMaxOrbits(chCompMaxOrbits);

			// Generate a companion star that fits within the orbit space available.
			do
			{
				// 1% chance for a neutron star
				if(m_GenHab == HABITABLE_System && dice.Roll(1,100) <= 1)
					pStar->GenNeutronStar(dice, lCompTypeMod, lCompSizeMod);
				else
				{
					pStar->GenCompanionType(dice, lCompTypeMod);
					dice.Random();
					pStar->GenCompanionSize(dice, lCompSizeMod);
				}
				pStar->InitZones();		// Init orbit zones now that we know the type and size of the star.
			}
			while(pStar->RadiusInOrbits() > chCompMaxOrbits);

			// reserve orbit zones around parent
			// max orbit this star might use around primary.
			char chMaxOrb = chCompanionOrbit + chCompMaxOrbits;
			pPrimary->ReserveZones(chMinOrb, chMaxOrb);
		}
	}

	// Check for ringworlds around the stars in the system.
	bool couldHave = false;
	for(DaAtomList::const_iterator it = starsInSystem.cbegin(); it != starsInSystem.cend(); it++)
	{
		DaStar* star = (DaStar*)*it;
		if(star->CouldHaveRingworld(GenSeed(), starsInSystem))
		{
			couldHave = true;
			break;
		}
	}

	// Delete our star clones.
	starsInSystem.DeleteContents();
	return couldHave;
}

//static
GALAXY DaSolarSystem::WormholeDestGalaxy(GALAXY fromGalaxy, const AuPoint& vecToDest)
{
	for(int gcon=0; gcon<s_GConCount; gcon++)
		if(s_GConns[gcon].m_FromGalaxy == fromGalaxy)
			if(s_GConns[gcon].m_VecToDest == vecToDest)
				return s_GConns[gcon].m_ToGalaxy;

#if !defined(QT_NO_DEBUG)
	qDebug() << "DaSolarSystem::WormholeDestGalaxy fromGalaxy" << GalaxyName(fromGalaxy) << "vec" << vecToDest << "NO DEST GALAXY";
	for(int gcon=0; gcon<s_GConCount; gcon++)
		if(s_GConns[gcon].m_FromGalaxy == fromGalaxy)
			qDebug() << " toGalaxy" << GalaxyName(s_GConns[gcon].m_ToGalaxy) << "vec" << s_GConns[gcon].m_VecToDest;
#endif
	return GALAXY_Count;
}

// This function only gets called in solo play and on scene worker servers.
void DaSolarSystem::RememberHengeDestination(const AuId& mySectorId,
		const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		ATOM_TYPE worldAtomType, const QString& worldCatName, const QString& worldName,
		const AuGlobeLLF& hengLL, bool hasBreathableAtm, ORBITZONE orbitZone, bool outsideCity,
		quint8 toSun, quint8 toArcSegment)
{
	if(IsFinal())
		return;		// n/a for finalized systems.
	m_HengeDestLock.lock();

	// Remove any entry that this new one replaces.
	QList<AuHengeDest_>::iterator hi = m_HengeDest.begin();
	while(hi != m_HengeDest.end())
	{
		if((*hi).IsHenge(mySectorId, fromSysId, fromSun, fromShadowPanels, fromArcSegment, toSun, toArcSegment))
			hi = m_HengeDest.erase(hi);
		else
			hi++;
	}

	// Add the new henge entry.
	AuHengeDest_ hd(mySectorId,
			fromSysId, fromSun, fromShadowPanels, fromArcSegment,
			worldAtomType, worldCatName, worldName,
			hengLL, hasBreathableAtm, orbitZone, outsideCity,
			toSun, toArcSegment);
	m_HengeDest.append(hd);

	m_HengeDestLock.unlock();
	SetModified(NOTIFY_Database);
}

void DaSolarSystem::CreateHengeDestinations()
{
	//Q_ASSERT(IsFinal());
	m_HengeDestLock.lock();
	QList<AuHengeDest_> hengeDest(m_HengeDest);
	m_HengeDest.clear();
	m_HengeDestLock.unlock();

	for(QList<AuHengeDest_>::const_iterator hi = hengeDest.cbegin(); hi != hengeDest.cend(); hi++)
		HengeCreated((*hi).m_FromSysId, (*hi).m_FromSun, (*hi).m_FromShadowPanels, (*hi).m_FromArcSegment, (*hi).m_ToSun,
				(*hi).m_ToArcSegment);
}

