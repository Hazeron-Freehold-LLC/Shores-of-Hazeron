//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daunit.cpp,v 1.359 2022/05/17 16:47:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daunit.h"
//
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "aufireparticles.h"
#include "auglobal.h"
#include "ausmokeparticles.h"
#include "daarena.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacitizen.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldringsection.h"
#include "dgempire.h"
#include "dgdata.h"
#include "pscloudacid.h"
#include "psclouddisease.h"
#include "pscloudpoison.h"
#include "pscloudstun.h"
#include "psnebula.h"
#include "psspeck.h"

// Light flicker will be identical between burning units because this is a static. Will anyone notice?
static GLuint							s_TexAcid = 0;
static GLuint							s_TexDisease = 0;
static GLuint							s_TexFire = 0;
static GLuint							s_TexPoison = 0;
static GLuint							s_TexSmoke = 0;
static GLuint							s_TexStun = 0;

DaUnit::DaUnit(const QString& name, int idEmpire)
{
	m_ArenaTeam = 0;
	m_sName = name;
	m_idAllegiance = idEmpire;
	m_IsPirateEmpire = 0;										// unknown
	m_PSAcid = 0;
	m_PSDisease = 0;
	m_PSFire = 0;
	m_PSNebula = 0;
	m_PSPoison = 0;
	m_PSSmoke = 0;
	m_PSSpeck = 0;
	m_PSStun = 0;
}

DaUnit::DaUnit()
{
	m_ArenaTeam = 0;
	m_idAllegiance = 0;
	m_IsPirateEmpire = 0;										// unknown
	m_PSAcid = 0;
	m_PSDisease = 0;
	m_PSFire = 0;
	m_PSNebula = 0;
	m_PSPoison = 0;
	m_PSSmoke = 0;
	m_PSSpeck = 0;
	m_PSStun = 0;
}

DaUnit::~DaUnit()
{
	if(m_PSAcid)
	{
		delete m_PSAcid;
		m_PSAcid = 0;
	}

	if(m_PSDisease)
	{
		delete m_PSDisease;
		m_PSDisease = 0;
	}

	if(m_PSFire)
	{
		delete m_PSFire;
		m_PSFire = 0;
	}

	if(m_PSNebula)
	{
		delete m_PSNebula;
		m_PSNebula = 0;
	}

	if(m_PSPoison)
	{
		delete m_PSPoison;
		m_PSPoison = 0;
	}

	if(m_PSSmoke)
	{
		delete m_PSSmoke;
		m_PSSmoke = 0;
	}

	if(m_PSSpeck)
	{
		delete m_PSSpeck;
		m_PSSpeck = 0;
	}

	if(m_PSStun)
	{
		delete m_PSStun;
		m_PSStun = 0;
	}

	m_Conditions.Clear();
}

DaUnit::DaUnit(const DaUnit &rhs) : DaAtom(rhs)
{
	m_ArenaTeam = rhs.m_ArenaTeam;
	m_IsPirateEmpire = rhs.m_IsPirateEmpire;

	rhs.m_UnitLock.lockForRead();
	m_idAllegiance = rhs.m_idAllegiance;
	m_sName = rhs.m_sName;
	m_tmChange = rhs.m_tmChange;
	m_tmConditionChange = rhs.m_tmConditionChange;
	m_tmConditionTick = rhs.m_tmConditionTick;
	m_UnitDamage = rhs.m_UnitDamage;
	rhs.m_UnitLock.unlock();

	rhs.m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = rhs.m_Conditions.cbegin(); ci != rhs.m_Conditions.cend(); ci++)
		m_Conditions.append((*ci)->Clone());
	rhs.m_ConditionsLock.unlock();

	m_PSAcid = 0;
	m_PSDisease = 0;
	m_PSFire = 0;
	m_PSNebula = 0;
	m_PSPoison = 0;
	m_PSSmoke = 0;
	m_PSSpeck = 0;
	m_PSStun = 0;
}

bool DaUnit::IsPirate() const
{
	if(m_IsPirateEmpire == 0)
	{
		if(InArena())
		{
			m_IsPirateEmpire = 1;
			if(ArenaTeam() == DgEmpire::ArenaTeam2Id())
			{
				// I could be an AI controlled team in the arena. Then I am considered a pirate.
				const DaArena* arena = FindParentArena();
				if(arena && arena->MatchType() == DaArena::SpacecraftTrial)
					m_IsPirateEmpire = 2;
			}
		}
		else
			m_IsPirateEmpire = DgEmpire::IsPirate(EmpireId()) ? 2 : 1;	// 2=yes, 1=no
	}
	return m_IsPirateEmpire == 2;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaUnit::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	quint8 hasWhat = 0;
	if(lastTx.IsNull())
	{
		// First time, send everything.
		hasWhat = 0x03;
	}
	else
	{
		// Send only what changed.
		m_UnitLock.lockForRead();
		if(!m_tmChange.IsNull() && m_tmChange >= lastTx)
			hasWhat |= 0x01;
		m_UnitLock.unlock();

		m_ConditionsLock.lockForRead();
		if(!m_tmConditionChange.IsNull() && m_tmConditionChange >= lastTx)
			hasWhat |= 0x02;
		m_ConditionsLock.unlock();

		if(Is(sceneAtom.Id()) && Vector().Length() > SpeedOfLight()*.9)
			hasWhat |= 0x04;	// for warp tunnel effect on scene atom

	}

	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		m_UnitLock.lockForRead();
		toClient << m_sName;
		toClient << m_idAllegiance;
		toClient << m_ArenaTeam;
		m_UnitLock.unlock();
	}

	if(hasWhat & 0x02)
	{
		m_ConditionsLock.lockForRead();
		toClient << m_Conditions;
		m_ConditionsLock.unlock();
	}

	if(hasWhat & 0x04)
	{
		AuPoint vec(Vector());
		toClient << (float)vec.x;
		toClient << (float)vec.y;
		toClient << (float)vec.z;
	}
}

// Receive updates to this atom from a server.
void DaUnit::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
	{
		fromServer >> m_sName;
		fromServer >> m_idAllegiance;
		fromServer >> m_ArenaTeam;
	}

	if(hasWhat & 0x02)
		fromServer >> m_Conditions;

	if(hasWhat & 0x04)
	{
		// This only actually gets sent for the scene atom when he is personally moving near the speed of light.
		float vx,vy,vz;
		fromServer >> vx;
		fromServer >> vy;
		fromServer >> vz;
		SetVector(AuPoint(vx,vy,vz));
	}
	else
		SetVector(AuPoint(0,0,0));
}

// Refresh data from valid counterpart.
bool DaUnit::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	AuMatrix4x4 oldT(Transform());
	bool transformChanged = DaAtom::Refresh(pAtom, ari);

	const DaUnit* pA = (const DaUnit*)pAtom;
	m_ArenaTeam = pA->m_ArenaTeam;

	pA->m_UnitLock.lockForRead();
	m_UnitLock.lockForWrite();
	m_idAllegiance = pA->m_idAllegiance;
	m_sName = pA->m_sName;
	m_UnitLock.unlock();
	pA->m_UnitLock.unlock();

	pA->m_ConditionsLock.lockForRead();
	m_ConditionsLock.lockForWrite();
	m_Conditions.Clear();
	for(AuConditionList::const_iterator ci = pA->m_Conditions.cbegin(); ci != pA->m_Conditions.cend(); ci++)
		m_Conditions.append((*ci)->Clone());
	m_ConditionsLock.unlock();
	pA->m_ConditionsLock.unlock();

	SetVector(pA->Vector());

	if(transformChanged && (m_PSNebula || m_PSSpeck))
	{
		AuMatrix4x4 t = Transform().Inverse() * oldT;
		if(m_PSNebula)
			m_PSNebula->Transform(t);
		if(m_PSSpeck)
			m_PSSpeck->Transform(t);
	}

	return transformChanged;
}

unsigned int DaUnit::IsTocker() const
{
	unsigned int ms = DaAtom::IsTocker();
	if(!IsNormal())
		return ms;
	if(ms == 0 || ms > d_MSecPerMin)
		ms = d_MSecPerMin;
	return ms;
}


void DaUnit::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaAtom::Tock(last, now, scripts, tockTrans);

	// Refresh empire presense in parent solar system.
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(system)
	{
		system->SetEmpirePresent(EmpireId());
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.ElapsedMinTicks(now) > 0)
	{
		// Unit damage history decays with time.
		m_UnitLock.lockForRead();
		bool v = !m_UnitDamage.empty();
		m_UnitLock.unlock();
		if(v)
		{
			m_UnitLock.lockForWrite();
			m_UnitDamage.Decay();
			m_UnitLock.unlock();
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

void DaUnit::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaAtom::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
}

void DaUnit::RecordDamage(const AuId& byWho, quint32 damageDealt, COMMODITYID cidUsed, quint8 cidQuality)
{
	Q_ASSERT(damageDealt);
	Q_ASSERT(!byWho.IsNull() || cidUsed != CID_Invalid);		// We need an atom id OR a commodity id.
	Q_ASSERT(cidQuality);

	// Accumulate damage into the byWho atom's damage record, if one already exists.
	m_UnitLock.lockForWrite();
	AuUnitDamageHash::iterator udi = m_UnitDamage.find(byWho);
	if(udi != m_UnitDamage.end())
	{
		(*udi).second.m_CID = cidUsed;							// We always remember the last CID used to inflict the damage.
		(*udi).second.m_iDamage += damageDealt;
		(*udi).second.m_LastDamage.Set();
		(*udi).second.m_Quality = cidQuality;
	}
	else
	{
		// We get here if no record of damage by the byWho atom already exists.
		// Create a nu unit damage record for the byWho atom.
		AuUnitDamage atomDmg;
		atomDmg.m_CID = cidUsed;
		atomDmg.m_iDamage = damageDealt;
		atomDmg.m_LastDamage.Set();
		atomDmg.m_Quality = cidQuality;
		m_UnitDamage.insert(pair<AuId, AuUnitDamage>(byWho, atomDmg));
	}
	m_UnitLock.unlock();
}

// Returns the atom id who is to be rewarded for the most damage dealt on this unit.
AuId DaUnit::KillerId() const
{
	AuId mostId;
	quint32 mostDamage = 0;
	m_UnitLock.lockForRead();
	for(AuUnitDamageHash::const_iterator udi = m_UnitDamage.begin(); udi != m_UnitDamage.end(); udi++)
		if((*udi).second.m_iDamage > mostDamage)
		{
			mostDamage = (*udi).second.m_iDamage;
			mostId = (*udi).first;
		}
	m_UnitLock.unlock();
	return mostId;
}

// Adds a route of waypoints, via known wormholes, to a destination.
// If no wormhole route exists, no waypoints are added.
bool DaUnit::AddWaypointRouteTo(DeWaypointRoute& route, const AuId& origSystem, const AuId &idDest, const AuPoint& vFromDestAtom,
		bool negativeOk, bool neutralOk) const
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	if(InArena())
		return false;

	// Find destination solar system.
	DaAtom* pd = ether->FindNormalAtom(idDest);
	if(!pd)
		return false;
	DaGalaxy* galaxyDest = dynamic_cast<DaGalaxy*>(pd);	// Destination might be a galaxy.
	// Find origin solar system.
	const DaSolarSystem* psso = 0;
	if(route.count() > 0)
	{
		// Find the parent solar system of the last waypoint.
		const DeWaypoint &wp = route.last();
		psso = dynamic_cast<DaSolarSystem*>(ether->FindNormalAtom(wp.m_idSystem));
	}
	else if(!origSystem.IsNull())
	{
		// Origin system was specified.
		psso = dynamic_cast<DaSolarSystem*>(ether->FindNormalAtom(origSystem));
	}
	else
	{
		// Origin is MY solar system.
		psso = FindParentSolarSystem();
	}
	if(!psso || psso->IsEmptySystem())
		return false;

	if(galaxyDest)
	{
		// We don't build a route to dest galaxies. An intergalactic wormhole is expected to be in my current system.
		if(!neutralOk)
			return false;

		// Look for a wormhole to the galaxy, which is the next route destination.
		bool foundWorm = false;
		const AuWormholeArray& wha = psso->LockWormholes();
		for(AuWormholeArray::const_iterator it = wha.cbegin(); it != wha.cend(); it++)
		{
			// Do not enter negative wormholes unless it's ok.
			if((*it).DrivePolarity() != AuWormhole::Intergalactic)
				continue;	// Don't plan a route through this wormhole.

			GALAXY destGalaxy = DaSolarSystem::WormholeDestGalaxy(psso->Galaxy(), (*it).SectorVecToDestination());
			if(galaxyDest->Galaxy() == destGalaxy)
			{
				// Wormhole is not an atom so we need to use a relative location. Good that those things don't orbit.
				route.AddDestination(psso->Id(), (*it).Location(psso), psso->Id());
				route.AddDestination(galaxyDest->Id(), galaxyDest->Id());
				foundWorm = true;
				break;
			}
		}
		psso->UnlockWormholes();
		if(!foundWorm)
		{
			route.clear();
			return false;
		}
		return true;
	}

	DaSolarSystem *pssd = pd->FindParentSolarSystem();	// Parent solar system of idDest. May be idDest.
	if(!pssd || pssd->IsEmptySystem())
		return false;

	if(psso == pssd)
	{
		// The destination atom is in the origin system.
		// If the destination atom IS the origin system, don't bother.
		if(pd != pssd)
			route.AddDestination(pd->Id(), pssd->Id());	// Add a waypoint directly to it.
		else if(!vFromDestAtom.IsZero())
		{
			// Destination atom is offset in the origin system; it's probably a wormhole. (Which isn't an atom any more.)
			route.AddDestination(pd->Id(), vFromDestAtom, pssd->Id());	// Add a waypoint.
		}
		return true;
	}

	// Build an array of information about solar systems we have processed. All entries will refer to solar system atoms.
	DqAtomLocArray ar;
	DqAtomLoc al;
	al.m_pA = (DaAtom*)psso;			// First atom location is the origin solar system.
	al.m_fLocation.x = 0;				// Jump count is 0 at the origin system.
	ar.append(al);

	// Determine the optimum route via wormholes from the origin to the destination solar system.
	bool bFoundDest = false;
	int ijumps = 0;						// Start in the origin system, which is 0 jumps away.
	for(; ijumps<20 && !bFoundDest; ijumps++)
	{
		for(int ih=0; ih<ar.count() && !bFoundDest; ih++)
		{
			// Process all the systems that are at 'ijumps' distance.
			const DqAtomLoc& hl = ar[ih];
			if(hl.m_fLocation.x != ijumps)
				continue;

			// Add wormhole destinations to the array with a higher hop count.
			const DaSolarSystem* hsys = (const DaSolarSystem*)hl.m_pA;
			bool orgExplored = hsys->IsExploredBy(EmpireId());
			const AuWormholeArray& wha = hsys->LockWormholes();
			for(AuWormholeArray::const_iterator it = wha.cbegin(); it != wha.cend(); it++)
			{
				// Do not enter negative wormholes unless it's ok.
				if(!negativeOk && (*it).DrivePolarity() == AuWormhole::Negative)
					continue;	// Don't plan a route through the negative (non-positive) wormhole.
				else if((*it).DrivePolarity() != AuWormhole::Positive && (*it).DrivePolarity() != AuWormhole::Negative)
					continue;	// Don't plan a route through this unfamiliar wormhole.

				// Find solar system that is the destination of the wormhole.
				DaSolarSystem* pwd = dynamic_cast<DaSolarSystem*>(ether->FindNormalAtom((*it).ToSystem()));
				if(!pwd)
					continue;	// Destination system must be on another server. It could be a galaxy instead of a solar system.
				if(!orgExplored && !pwd->IsExploredBy(EmpireId()))
					continue;	// Neither the origin nor the destination system is explored by my empire. Don't use the wormhole.

				// Insure that we haven't looped back on ourself.
				bool bIsNewDest = true;
				for(int iloop=0; iloop<ar.count() && bIsNewDest; iloop++)
				{
					const DqAtomLoc& loop = ar[iloop];
					if(loop.m_pA == pwd)
						bIsNewDest = false;						// We've already found a route to this location.
				}

				// Add each nu destination to the end of the array with a slightly higher hop count.
				if(bIsNewDest)
				{
					al.m_pA = pwd;
					al.m_fLocation.x = ijumps+1;
					ar.append(al);
					if(pwd == pssd)
					{
						bFoundDest = true;
						break;
					}
				}
			}
			hsys->UnlockWormholes();
		}
	}

	if(!bFoundDest)
		return false;	// A wormhole route was not found to the destination.

	//qDebug("%s route to %s is %d jumps, ar %d", (const char*)Name().toAscii(), (const char*)pssd->Name().toAscii(), ijumps,
			//ar.count());
	//for(int ir=0; ir<ar.count(); ir++)
	//{
		//DqAtomLoc& rl = ar[ir];
		//qDebug(" %d %s is %g jumps", rl.m_pA->Id().GetID(), (const char*)rl.m_pA->Name().toAscii(), rl.m_fLocation.x);
	//}

	// A wormhole route was found; now to determine the exact routing.
	// This list will contain the atom routing, from destination atom to origin solar system.
	DaAtomList aroute;

	// Build the solar system route from the destination to the origin.
	DaSolarSystem *pfinalss = NULL;	// Final solar system.
	AuWormhole whfinal;				// Final emerging wormhole.
	aroute.append(pssd);			// Add final destination solar system.
	for(ijumps -= 1; ijumps; ijumps--)
	{
		// Search for a route destination with the current hop count that is accessible via wormhole from the current route end.
		const DaSolarSystem* routeSys = (const DaSolarSystem*)aroute.last();
		bool bFoundRoute = false;
		for(int ir=0; ir<ar.count() && !bFoundRoute; ir++)
		{
			DqAtomLoc& rl = ar[ir];
			if(rl.m_fLocation.x == ijumps)
			{
				// 'rl' has the right hop count.
				// Is it a destination of a wormhole at the route end?
				const AuWormholeArray& wha = routeSys->LockWormholes();
				for(AuWormholeArray::const_iterator it = wha.cbegin(); it != wha.cend(); it++)
				{
					// Do not exit positive wormholes unless it's ok.
					if(!negativeOk && (*it).DrivePolarity() == AuWormhole::Positive)
						continue;	// Don't plan a route back through the positive wormhole, which would enter negative.
					else if((*it).DrivePolarity() != AuWormhole::Positive && (*it).DrivePolarity() != AuWormhole::Negative)
						continue;	// Don't plan a route through this unfamiliar wormhole.

					if((*it).ToSystem() == rl.m_pA->Id())
					{
						// There is a wormhole from the route end solar system to 'rl', which represents a solar system
						// atom that is one hop closer to the origin.
						if(!whfinal.IsValid())
						{
							// We're finding our way back so the first wormhole out of the destination system is
							// the final emerging wormhole when we come from the other direction.
							whfinal = *it;	// Remember the last and final wormhole exit.
							// Remember the final wormhole's parent solar system.
							pfinalss = (DaSolarSystem*)aroute.last();
						}
						//qDebug(" route to %d %s", rl.m_pA->Id().GetID(), (const char*)rl.m_pA->Name().toAscii());
						aroute.append(rl.m_pA);
						bFoundRoute = true;
						break;
					}
				}
				routeSys->UnlockWormholes();
			}
		}
	}

	//qDebug("aroute count %d", aroute.count());

	// Now 'aroute' contains the solar system routing from the destination to the origin.
	// 'aroute' does NOT include the origin; it assumes the route begins in the origin system.
	// From that, we can construct the actual system by system routing to
	// get from the origin, via wormholes, to the destination atom.
	AuWormhole::POLARITY finalPolarity = AuWormhole::Invalid;
	const DaSolarSystem* porg = psso;	// First originating system is the current origin solar system.
	while(!aroute.isEmpty())
	{
		// Get the next destination solar system.
		DaSolarSystem *pss = (DaSolarSystem*)aroute.last();
		aroute.removeLast();

		// Look for a wormhole to 'pss', which is the next route destination.
		bool foundWorm = false;
		const AuWormholeArray& wha = porg->LockWormholes();
		for(AuWormholeArray::const_iterator it = wha.cbegin(); it != wha.cend(); it++)
		{
			// Do not enter negative wormholes unless it's ok.
			if(!negativeOk && (*it).DrivePolarity() == AuWormhole::Negative)
				continue;	// Don't plan a route through the negative (non-positive) wormhole.
			else if((*it).DrivePolarity() != AuWormhole::Positive && (*it).DrivePolarity() != AuWormhole::Negative)
				continue;	// Don't plan a route through this unfamiliar wormhole.

			if(pss->Is((*it).ToSystem()))
			{
				// Wormhole is not an atom so we need to use a relative location. Good that those things don't orbit.
				route.AddDestination(porg->Id(), (*it).Location(porg), porg->Id());
				finalPolarity = (*it).Polarity();
				foundWorm = true;
				break;
			}
		}
		porg->UnlockWormholes();
		if(!foundWorm)
		{
			//qWarning() << "ERROR!" << Name() << "found one-way wormhole in route. AddWaypointRouteTo origSystem"
				//<< origSystem << "idDest" << idDest << "vFromDestAtom" << vFromDestAtom
				//<< "No wormhole from system" << porg->Name() << "to system" << pss->Name();
			route.clear();
			return false;
		}

		// Now pss becomes the next originating system.
		// If it's the final destination, then we're actually done.
		porg = pss;
	}

	// Add a waypoint at the emerging wormhole.
	if(!pfinalss && !pssd->IsFinal())
	{
		// The emergent system is not final. The wormholes haven't been resolved there.
		// Make it our destination and assume the wormholes will resolve when we get there.
		pfinalss = pssd;
	}
	else if(!whfinal.IsValid() || !pfinalss)
	{
		bool foundWorm = false;
		const AuWormholeArray& wha = pssd->LockWormholes();
		for(AuWormholeArray::const_iterator it = wha.cbegin(); it != wha.cend(); it++)
		{
			// Do not exit positive wormholes unless it's ok.
			if(!negativeOk && (*it).DrivePolarity() == AuWormhole::Positive)
				continue;	// Don't plan a route back through the positive wormhole, which would enter negative.
			else if((*it).DrivePolarity() != AuWormhole::Positive && (*it).DrivePolarity() != AuWormhole::Negative)
				continue;	// Don't plan a route through this unfamiliar wormhole.

			if(psso->Is((*it).ToSystem()) && (*it).Polarity() != finalPolarity)
			{
				// There is a wormhole from the destination directly to the origin.
				whfinal = *it;		// Remember the final wormhole exit.
				pfinalss = pssd;	// Remember the final wormhole's parent solar system.
				foundWorm = true;
				break;
			}
		}
		pssd->UnlockWormholes();
		if(!foundWorm)
		{
			qWarning() << "ERROR!" << Name() << "one-way wormhole emerges at destination. AddWaypointRouteTo origSystem"
				<< origSystem << "idDest" << idDest << "vFromDestAtom" << vFromDestAtom
				<< "No wormhole back from system" << pssd->Name() << "to system" << psso->Name();
			route.clear();
			return false;
		}
	}

	if(pfinalss)
	{
		if(whfinal.IsValid())
		{
			// Wormhole is not an atom so we need to use a relative location. Good that those things don't orbit.
			route.AddDestination(pfinalss->Id(), whfinal.Location(pfinalss), pfinalss->Id());

			// Add a final waypoint at the destination atom, if it's not the solar system itself.
			if(pd != pfinalss)
				route.AddDestination(pd->Id(), pfinalss->Id());
		}
		else if(!pfinalss->IsFinal())
		{
			// Destination wormholes have not been resolved. May not exist.
			route.AddDestination(pfinalss->Id(), AuPoint(0,0), pfinalss->Id());

			// Add a final waypoint at the destination atom, if it's not the solar system itself.
			if(pd != pfinalss)
				route.AddDestination(pd->Id(), pfinalss->Id());
		}
	}

	if(route.count() > 1)
		return true;

	route.clear();
	return false;
}

DaVessel* DaUnit::StationInTradeRange(double myRoughRadius, bool& hasStation) const
{
	hasStation = false;
	if(!IsInAtmosphere())
	{
		// Look for a friendly station nearby since this unit is not in the atmosphere.
		const DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			// We'll look in the entire solar system.
			AuPoint myLoc = EnvLocation();
			DaAtomList atoms;
			system->FindVesselsRecursive(atoms);
			for(DaAtomList::iterator it = atoms.begin(); it != atoms.end(); it++)
			{
				DaVessel* ss = dynamic_cast<DaVessel*>(*it);
				if(!ss)
					continue;	// Not likely due to FindVesselsRecursive.
				if(!ss->IsSpaceStation())
					continue;	// Skip ships that are not stations.
				if(dynamic_cast<DaVessel*>(ss->Parent()))
					continue;	// Skip stations that are aboard spacecraft, thanks to Ikkir Isth.
				if(ss == this)
				{
					// This unit is a station that is not aboard another spacecraft.
					hasStation = true;
					return ss;
				}
				if(PoliticalStance(ss->EmpireId(), EmpireId()) == POL_Enemy)
					continue;	// Skip enemy stations.

				hasStation = true;

				AuPoint stationLoc = ss->EnvLocation();
				if(myLoc.Distance(stationLoc) < ss->StationTradeRange() + myRoughRadius)
					return ss;
			}
		}
	}
	return 0;
}

void DaUnit::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_ArenaTeam;
	out << m_IsPirateEmpire;

	m_UnitLock.lockForRead();
	out << m_idAllegiance;
	out << m_sName;
	out << AuTime();	//m_tmChange;
	out << m_UnitDamage;
	m_UnitLock.unlock();

	m_ConditionsLock.lockForRead();
	out << m_Conditions;
	out << AuTime();	//m_tmConditionChange;
	AuTime tmConditionTick;
	out << tmConditionTick;
	m_ConditionsLock.unlock();
}

void DaUnit::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_ArenaTeam;
	in >> m_IsPirateEmpire;

	m_UnitLock.lockForWrite();
	in >> m_idAllegiance;
	in >> m_sName;
	AuTime _tmChange;
	in >> _tmChange;
	in >> m_UnitDamage;
	m_UnitLock.unlock();

	m_ConditionsLock.lockForWrite();
	in >> m_Conditions;
	AuTime _tmConditionChange;
	in >> _tmConditionChange;
	AuTime tmConditionTick;
	in >> tmConditionTick;
	m_ConditionsLock.unlock();
}

bool DaUnit::ReduceCondition(const AuCondition* condition, quint8 qualityReduction)
{
	Q_ASSERT(condition);
	m_ConditionsLock.lockForWrite();
	for(AuConditionList::iterator ci = m_Conditions.begin(); ci != m_Conditions.end(); ci++)
		if(condition->CreateTime() == (*ci)->CreateTime() && condition->CType() == (*ci)->CType())
		{
			(*ci)->ReduceQuality(qualityReduction);
			m_tmConditionChange.Set();
			m_ConditionsLock.unlock();
			return true;
		}
	m_ConditionsLock.unlock();
	return false;
}

bool DaUnit::RemoveCondition(const AuCondition* condition)
{
	Q_ASSERT(condition);
	m_ConditionsLock.lockForWrite();
	for(AuConditionList::iterator ci = m_Conditions.begin(); ci != m_Conditions.end(); ci++)
		if(condition->CreateTime() == (*ci)->CreateTime() && condition->CType() == (*ci)->CType())
		{
			delete *ci;
			m_Conditions.erase(ci);
			m_tmConditionChange.Set();
			m_ConditionsLock.unlock();
			return true;
		}
	m_ConditionsLock.unlock();
	return false;
}

bool DaUnit::IsPregnant() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConPregnant::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

AuTime DaUnit::DueDate() const
{
	m_ConditionsLock.lockForRead();
	AuTime dueDate(m_Conditions.DueDate());
	m_ConditionsLock.unlock();
	return dueDate;
}

AuId DaUnit::FatherOfBaby() const
{
	m_ConditionsLock.lockForRead();
	AuId fatherOfBaby(m_Conditions.FatherOfBaby());
	m_ConditionsLock.unlock();
	return fatherOfBaby;
}

// True if this unit has any acid condition.
bool DaUnit::HasAcid() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConAcid::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

// True if this unit has any disease condition.
bool DaUnit::HasDisease() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConDisease::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

bool DaUnit::HasDisease(quint8 strength, quint8 contagion, quint8 quality) const
{
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.cbegin(); ci != m_Conditions.cend(); ci++)
	{
		const AuConDisease* disease = dynamic_cast<const AuConDisease*>(*ci);
		if(disease && disease->IsDisease(strength, contagion, quality))
		{
			m_ConditionsLock.unlock();
			return true;
		}
	}
	m_ConditionsLock.unlock();
	return false;
}

// True if this unit has any implant condition.
bool DaUnit::HasImplant() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConImplantDNA::ConditionType()) ||
		m_Conditions.Contains(AuConImplantSpecies::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

// True if this unit has any poison condition.
bool DaUnit::IsBurning() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConFire::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

// True if this unit has any paralysis condition.
bool DaUnit::IsParalyzed() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConParalysis::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

// True if this unit has any poison condition.
bool DaUnit::IsPoisoned() const
{
	m_ConditionsLock.lockForRead();
	bool v = m_Conditions.Contains(AuConPoison::ConditionType());
	m_ConditionsLock.unlock();
	return v;
}

double DaUnit::Bounty() const
{
	double bounty = 0;
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.cbegin(); ci != m_Conditions.cend(); ci++)
	{
		const AuConBounty* conBounty = dynamic_cast<const AuConBounty*>(*ci);
		if(conBounty)
			bounty += conBounty->Reward();
	}
	m_ConditionsLock.unlock();
	return bounty;
}

void DaUnit::Bounties(QList<AuBountyOnItem>& bounties) const
{
	// We will sort these by amount, descending.
	QMultiMap<double, AuBountyOnItem> sorted;
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.cbegin(); ci != m_Conditions.cend(); ci++)
	{
		const AuConBounty* conBounty = dynamic_cast<const AuConBounty*>(*ci);
		if(conBounty)
			sorted.insert(conBounty->Reward(), AuBountyOnItem(conBounty->PosterId(), conBounty->PosterName(), Id(), Name(),
						conBounty->Reason(), conBounty->Reward()));
	}
	m_ConditionsLock.unlock();
	bounties.clear();
	if(sorted.empty())
		return;
	for(QMultiMap<double, AuBountyOnItem>::const_iterator si = sorted.cend()-1; si != sorted.cbegin()-1; si--)
		bounties.append(*si);
}

void DaUnit::Bounties(QMultiMap<AuTime, AuBountyOnItem>& bounties) const
{
	// We will sort these by amount, descending.
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.cbegin(); ci != m_Conditions.cend(); ci++)
	{
		const AuConBounty* conBounty = dynamic_cast<const AuConBounty*>(*ci);
		if(conBounty)
			bounties.insert(conBounty->CreateTime(), AuBountyOnItem(conBounty->PosterId(), conBounty->PosterName(), Id(), Name(),
						conBounty->Reason(), conBounty->Reward()));
	}
	m_ConditionsLock.unlock();
}

double DaUnit::TakeBounty()
{
	double bounty = 0;
	bool anyChange = false;
	m_ConditionsLock.lockForWrite();
	AuConditionList::iterator ci = m_Conditions.begin();
	while(ci != m_Conditions.cend())
	{
		AuConBounty* conBounty = dynamic_cast<AuConBounty*>(*ci);
		if(conBounty)
		{
			// Found a bounty item. Accumulate the total and remove the bounty.
			bounty += conBounty->Reward();
			delete conBounty;
			ci = m_Conditions.erase(ci);
			anyChange = true;
		}
		else
			ci++;
	}
	if(anyChange)
	{
		m_tmConditionChange.Set();
		SetModified();
	}
	m_ConditionsLock.unlock();
	return bounty;
}

bool DaUnit::TakeItemBounty(const QString& bountyHunter, const AuBountyOnItem& bountyOnItem)
{
	Q_UNUSED(bountyHunter);
	// Find the bounty condition.
	m_ConditionsLock.lockForWrite();
	if(!m_Conditions.RemoveBounty(bountyOnItem))
	{
		// No bounty condition was found.
		m_ConditionsLock.unlock();
		return false;
	}
	m_tmConditionChange.Set();
	m_ConditionsLock.unlock();
	SetModified();
	return true;
}

void DaUnit::AddBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, int why)
{
	bool added = false;
	double total = 0;
	if(why)
	{
		// System generated bounties are combined when the poster id and reason are identical.
		m_ConditionsLock.lockForWrite();
		for(AuConditionList::iterator ci = m_Conditions.begin(); ci != m_Conditions.end(); ci++)
		{
			AuConBounty* bounty = dynamic_cast<AuConBounty*>(*ci);
			if(!bounty)
				continue;
			if(bounty->PosterId() != posterId || bounty->Reason() != reason)
				continue;
			total = bounty->AddReward(reward);
			m_tmConditionChange.Set();
			added = true;
			break;						// Only add the reward to one existing bounty condition.
		}
		m_ConditionsLock.unlock();
	}
	if(!added)
	{
		// Reward was not added to an existing bounty. Create a new bounty condition.
		AuConBounty* bounty = new AuConBounty(posterId, posterName, reason, reward, AuTime::Now());
		AddCondition(bounty);
	}
	SetModified();

	QString text;
	if(!added)
		text = tr("%1 placed a %L2%3 bounty on my head because '%4'.")
			.arg(posterName)
			.arg(reward, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(reason);
	else
		text = tr("%1 increased the bounty on my head by %L2%3 because '%4'. That bounty is up to %L5%3 now.")
			.arg(posterName)
			.arg(reward, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(reason)
			.arg(total, 0,'f',0);

	// Add this as a condition in case they are not on line.
	AddCondition(new AuConThought(text, AuTime::Now()));
}

void DaUnit::UnitKilled(DaUnit* victim)
{
	if(!EmpireId())
		return;													// I am probably a wild animal.

	victim->KilledByUnit(this);
}

void DaUnit::SetEmpireId(quint32 idEmpire)
{
	if(Allegiance() != idEmpire)
	{
		SetAllegiance(idEmpire);
		m_IsPirateEmpire = 0;		// Unknown until requested.
		//SetModified();			// Can't do this here.

		if(EmpireId() && Parent())
		{
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
			if(system)
				system->SetEmpirePresent(EmpireId());
		}
	}
}

POLITICAL_STANCE DaUnit::PoliticalStanceMakeContact(const DaUnit* unit) const
{
	return DaAtom::PoliticalStanceMakeContact(EmpireId(), unit, unit->EmpireId());
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaUnit::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	Q_UNUSED(gunnerPosture);
	if(!IsInAtmosphere())
		return false;	// Turrets don't reach out of the atmosphere.
	if(IsUnderGround())
		return false;	// Turrets don't reach under ground.

	if(gunLoc.Distance2(Location()) > gunRange*gunRange)
		return false;	// I am out of range.

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return false;	// I am not an enemy.

	if(IsNoncombatant())
		return false;	// I am a noncombatant unit.

	// I am a target.
	targetLoc = Transform().Project(TargetLocation());
	return true;
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaUnit::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerPosture);
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
	if(targetVec.Length2() > gunRange*gunRange)
		return false;

	if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
		return false;

	if(gunnerEmpire && PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return false;	// I am not an enemy.

	targetSysLoc = EnvTransform().Project(TargetLocation());
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaUnit::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerPosture);
	Q_UNUSED(gunSysVec);
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return;

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
		return;

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return;	// I am not an enemy.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = EnvTransform().Project(TargetLocation());
}

// Find atoms that might cause shields to be raised.
// 'shipEmpire' is 0 to ignore political stance.
void DaUnit::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	Q_UNUSED(shipId);
	if(!IsGunnerDomainTarget(shipDomain, shipWorldId, shipId))
		return;

	// Skip unit if I am not a threat based on ship's shield policy.
	switch(shieldPolicy)
	{
	case ShieldUp:						// Up all the time
	case ShieldDown:					// Down all the time
		return;
	case ShieldUpIfAnyone:				// Up if any contact
		break;
	case ShieldUpIfForeign:				// Up if any non-empire contact
		if(shipEmpire == EmpireId())
			return;						// I am not a foreign empire.
		break;
	case ShieldUpIfNeutral:				// Up if any neutral or enemy contact
		{
			POLITICAL_STANCE stance = PoliticalStance(shipEmpire, EmpireId());
			if(stance != POL_Neutral && stance != POL_Enemy)
				return;					// I am not neutral or an enemy.
		}
		break;
	case ShieldUpIfEnemy:				// Up if any enemy contact
		if(PoliticalStance(shipEmpire, EmpireId()) != POL_Enemy)
			return;						// I am not an enemy.
		break;
	}

	// Perform range test.
	AuPoint targetVec = EnvLocation() - shipSysLoc;		// Vector to target from ship's location.
	double range2 = targetVec.Length2();
	if(range2 > sensorRange*sensorRange)
		return;

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	// I am the closest threat.
	targetAtom = this;
	targetRange = sqrt(range2);
}

void DaUnit::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::DeleteDeviceObjects(ari);

	if(m_WarpVAO.VAO())
		m_WarpVAO.DeleteGL();
}

void DaUnit::RenderFire(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(!s_TexFire)
		s_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_PSFire)
	{
		m_PSFire = new AuFireParticles(size);
		m_PSFire->SetMaxParticles(1000);
		m_PSFire->SetLifeCycle(1.5f);
		m_PSFire->SetVelocityVar(size * .7f);
		m_PSFire->SetNumToRelease(20);
		m_PSFire->SetReleaseInterval(0.05f);
		QColor colFire;
		colFire.setRgbF(1, .25, .15);
		m_PSFire->SetColor(colFire);
		m_PSFire->SetSize(size);
		m_PSFire->SetTextureID(s_TexFire);
		m_PSFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSFire->Init(re);
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(s_TexFire);
	re.GLSL().SpriteSizeShader().SetColor(m_PSFire->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSFire->GetSize()*100 *c_UnitScale * MiniScale());

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSFire->Update())
		m_PSFire->Render(projection, model, re, MiniScale());
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void DaUnit::RenderSmoke(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(g_Au.m_DebugSwitches & 0x40000000)
		return;
	if(!s_TexSmoke)
		s_TexSmoke = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SmokeT", ":data/smoketransparent.png");

	if(!m_PSSmoke)
	{
		m_PSSmoke = new AuSmokeParticles();
		m_PSSmoke->SetMaxParticles(1000);
		m_PSSmoke->SetLifeCycle(7.5f);
		m_PSSmoke->SetVelocityVar(size / 2.f);
		m_PSSmoke->SetNumToRelease(1);
		m_PSSmoke->SetReleaseInterval(0.25f);
		QColor colSmoke;
		colSmoke.setRgbF(.5, .5, .5);
		m_PSSmoke->SetColor(colSmoke);
		m_PSSmoke->SetSize(size*100);
		m_PSSmoke->SetTextureID(s_TexSmoke);
		m_PSSmoke->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSSmoke->Init(re);
	}
	if(Vector().IsZero())
	{
		m_PSSmoke->SetReleaseInterval(0.25f);
		m_PSSmoke->SetVelocity(AuPoint(0,0));
	}
	else
	{
		m_PSSmoke->SetReleaseInterval(0.05f);
		m_PSSmoke->SetVelocity(-Vector());
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(s_TexSmoke);
	re.GLSL().SpriteSizeShader().SetColor(m_PSSmoke->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSSmoke->GetSize()*10 *c_UnitScale * MiniScale());

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if(m_PSSmoke->Update())
		m_PSSmoke->Render(projection, model, re, MiniScale());
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void DaUnit::RenderAcid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(!s_TexAcid)
		s_TexAcid = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WAc", ":data/acidsprite.png");

	if(!m_PSAcid)
		m_PSAcid = new PsCloudAcid(size, size, s_TexAcid);

	if(m_PSAcid->Update())
		m_PSAcid->Render(projection, model, re, MiniScale());
}

void DaUnit::RenderDisease(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(!s_TexDisease)
		s_TexDisease = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WDi", ":data/diseasesprite.png");

	if(!m_PSDisease)
		m_PSDisease = new PsCloudDisease(size, size, s_TexDisease);

	if(m_PSDisease->Update())
		m_PSDisease->Render(projection, model, re, MiniScale());
}

void DaUnit::RenderParalysis(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(!s_TexStun)
		s_TexStun = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WSt", ":data/stunsprite.png");

	if(!m_PSStun)
		m_PSStun = new PsCloudStun(size, size, s_TexStun);

	if(m_PSStun->Update())
		m_PSStun->Render(projection, model, re, MiniScale());
}

void DaUnit::RenderPoison(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size)
{
	if(!s_TexPoison)
		s_TexPoison = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WPo", ":data/poisonsprite.png");

	if(!m_PSPoison)
		m_PSPoison = new PsCloudPoison(size, size, s_TexPoison);

	if(m_PSPoison->Update())
		m_PSPoison->Render(projection, model, re, MiniScale());
}

void DaUnit::TickConditions()
{
	// We only tick our conditions once per 10 seconds.
	int conditions;
	m_ConditionsLock.lockForWrite();
	if(m_tmConditionTick.ElapsedSec() >= 10 || !m_tmConditionTick.isValid())
	{
		conditions = m_Conditions.count();
		m_tmConditionTick.restart();
	}
	else
		conditions = 0;
	m_ConditionsLock.unlock();
	if(!conditions)
		return;

	for(int ic=0; ic<conditions; ic++)
	{
		// Tick using a clone of the condition to avoid deadlocks when a ticking condition attempts to add a condition.
		AuCondition* c;
		m_ConditionsLock.lockForRead();
		if(ic < m_Conditions.count())
			c = m_Conditions[ic]->Clone();
		else
			c = 0;
		m_ConditionsLock.unlock();
		if(!c)
			break;

		// Tick the clone and clean it up.
		bool stillAlive = c->Tick(this);
		delete c;

		// Remove conditions when they are done.
		if(!stillAlive)
		{
			m_ConditionsLock.lockForWrite();
			if(ic < m_Conditions.count())
				if(m_Conditions.RemoveAt(ic))
				{
					m_tmConditionChange.Set();
					SetModified();
				}
			m_ConditionsLock.unlock();
		}
	}
}

quint32 DaUnit::EmpireId() const
{
	if(InArena())
		return m_ArenaTeam;
	return Allegiance();
}

void DaUnit::NameTheBaby(const QString& babyName)
{
	m_ConditionsLock.lockForWrite();
	for(AuConditionList::iterator ci = m_Conditions.begin(); ci != m_Conditions.end(); ci++)
	{
		AuConPregnant* preg = dynamic_cast<AuConPregnant*>(*ci);
		if(!preg)
			continue;

		if(preg->BabyName() != babyName)
		{
			preg->SetBabyName(babyName);
			m_tmConditionChange.Set();
			m_ConditionsLock.unlock();
			SetModified();

			AuCommThought msg(tr("I will call my baby %1.").arg(babyName), Id(), Name());
			g_Au.SendAtomChat(msg);
			return;
		}

		// We only permit one pregnancy per mom at the same time.
		break;
	}
	m_ConditionsLock.unlock();
}

void DaUnit::CreateWarpTunnelVAO(AuGLEffects& re)
{
	// In case this function gets called when a model was already present
	if(m_WarpVAO.VAO())
		return;

	double radius = FeetPerOrbit() * 2;
	QList<AuTriTex> tris;
	int lats = 32;
	int longs = 32;
	for(int i = 0; i <= lats; i++)
	{
		if(i == 0 && i != lats)
			continue;
		float lat0 = PIf * (-0.5f + (float) (i - 1) / lats);
		float z0 = sinf(lat0);
		float zr0 = cosf(lat0);

		float lat1 = PIf * (-0.5f + (float) i / lats);
		float z1 = sinf(lat1);
		float zr1 = cosf(lat1);

		// Texture coords for this row.
		// Texture coord x is the alpha of the vertex.
		float row0Alpha = 1.f;	//fabsf(zr0);
		//if(i == 1)
			//row0Alpha = 0;
		float row1Alpha = 1.f;	//fabsf(zr1);
		//if(i == lats)
			//row1Alpha = 0;

		// Texture coord y is the red shift (+1), neutral (0), blue shift(-1).
		float row0redshift = -z0;
		float row1redshift = -z1;

		AuPoint prev0, prev1;
		for(int j = 0; j <= longs; j++)
		{
			float lng = 2 * PIf * (float) (j - 1) / longs;
			float x = cosf(lng);
			float y = sinf(lng);

			AuPoint v0(x * zr0, y * zr0, z0);
			AuPoint v1(x * zr1, y * zr1, z1);
			v0 *= radius;
			v1 *= radius;
			if(j)
			{
				AuTriTex tri0;
				tri0.m_V0 = AuPointTex(prev0, row0Alpha, row0redshift);
				tri0.m_V1 = AuPointTex(prev1, row1Alpha, row1redshift);
				tri0.m_V2 = AuPointTex(v1, row1Alpha, row1redshift);
				tris.append(tri0);

				AuTriTex tri1;
				tri1.m_V0 = AuPointTex(v1, row1Alpha, row1redshift);
				tri1.m_V1 = AuPointTex(v0, row0Alpha, row0redshift);
				tri1.m_V2 = AuPointTex(prev0, row0Alpha, row0redshift);
				tris.append(tri1);
			}
			prev0 = v0;
			prev1 = v1;
		}
	}

	m_WarpVAO.CreateForTexture(re, tris, GL_STATIC_DRAW);
}

bool DaUnit::RenderWarpTunnel(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	const DaSector* sector = FindParentSector();
	if(!sector)
		return false;
	GLuint skybox = sector->SkyBoxTexture();
	if(!skybox)
		return false;

	AuPoint vec(Vector());
	double speed = vec.Length();
	if(speed <= SpeedOfLight()*.9)
		return false;

	CreateWarpTunnelVAO(*ari.re);

	// Move in my current direction by rotating the angle at the vertices.
	AuPoint evec = Transform().ProjectVector(Vector());
	evec.Unit();
	double dot = Location().Dot(evec);
	double period = fmod(dot, (double)FeetPerSolarSystem()*5);
	period /= FeetPerSolarSystem()*5;
	float tunnelAngle = period * PIf*2;

	// Tunnel becomes visible near the speed of light, quickly ramps to max alpha.
	float tunnelAlpha = sqrt((speed - SpeedOfLight()*.9) / (SpeedOfLight()*.2));
	if(tunnelAlpha > .9f)
		tunnelAlpha = .9f;

	// Model matrix.
	AuMatrix4x4 m = model * Transform().Inverse();
	m.Translate(Location());
	m *= AuPoint(0,0,1).GetAlignmentTransform(evec);

	// Tunnel wall.
	ari.re->glFrontFace(GL_CCW);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// Additive blending for extra brightness.

	// Configure shader.
	ari.re->GLSL().WarpTunnelShader().Select();
	ari.re->GLSL().WarpTunnelShader().SetProjectionModelView(projection * m);
	ari.re->GLSL().WarpTunnelShader().SetTexture(skybox);
	ari.re->GLSL().WarpTunnelShader().SetColor(1,1,1,tunnelAlpha);
	ari.re->GLSL().WarpTunnelShader().SetRotationAngle(tunnelAngle);

	m_WarpVAO.SelectGL();
	m_WarpVAO.DrawGL();
	m_WarpVAO.DeselectGL();

	ari.re->GLSL().WarpTunnelShader().Deselect();

	ari.re->glDisable(GL_BLEND);
	ari.re->glFrontFace(GL_CW);
	return true;
}

void DaUnit::RenderNebula(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	if(!g_Au.DrawNebulaCloud())
		return;

	//if(IsInAtmosphere() && AtmosphereDensity() > 0)
		//return;
	if(InDesignerNotPreview() || IsEyeUnderWater())
		return;

	const DaSector* sector = FindParentSector();
	if(!sector)
		return;

	float density = sector->NebulaDensity();
	if(density == 0.f)
		return;

	QColor colNebula = sector->NebulaColor();
	if(colNebula == QColor(0,0,0))
		return;

	GLuint nebTex;
	switch(sector->NebulaTexture())
	{
	default:
	case 0:
	case 1:
	case 2:
		if(!s_TexAcid)
			s_TexAcid = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WAc", ":data/acidsprite.png");
		nebTex = s_TexAcid;
		break;
	case 3:
	case 4:
		if(!s_TexSmoke)
			s_TexSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SmokeT", ":data/smoketransparent.png");
		nebTex = s_TexSmoke;
		break;
	case 5:
		nebTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "Neb1", ":data/poisontransparent.png");
		break;
	}

	if(!m_PSNebula)
	{
		m_PSNebula = new PsNebula();
		m_PSNebula->Init(*ari.re);
	}

	// We blend layers of these. Don't start with too much alpha.
	colNebula.setAlphaF(density * .25f);

	m_PSNebula->SetColor(colNebula);
	m_PSNebula->SetDensity(density);
	m_PSNebula->SetTextureID(nebTex);
	m_PSNebula->SetVelocity(Vector());

	m_PSNebula->Render(projection, model, *ari.re, MiniScale());
}

void DaUnit::RenderGravel(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	if(!g_Au.DrawSpecks())
		return;

	const DaSector* sector = FindParentSector();
	if(!sector)
		return;

	float density = sector->NebulaDensity() + .1f;

	// Increase density a lot in an asteroid belt.
	const DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(system)
		density += system->AsteroidBeltDensity(EnvLocation());

	if(!m_PSSpeck)
	{
		m_PSSpeck = new PsSpeck();
		m_PSSpeck->Init(*ari.re);

		GLuint speckTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "AstSpeck", ":data/Asteroid.png");
		m_PSSpeck->SetTextureID(speckTex);
	}

	m_PSSpeck->SetDensity(density);
	m_PSSpeck->SetVelocity(Vector());
	m_PSSpeck->Render(projection, model, *ari.re, MiniScale());
}

void DaUnit::RenderWarpOrNebula(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari)
{
	// Do not paint nebula when the warp effect is active. It looks better that way.
	if(!RenderWarpTunnel(projection, model, ari))
	{
		RenderNebula(projection, model, ari);

		// Do not paint gravel specks when inside the atmosphere.
		if(!IsInAtmosphere() && !IsEyeUnderWater())
		{
			// Don't paint them when close to an asteroid either, it looks very strange.
			const DaWorld* par = dynamic_cast<const DaWorld*>(Parent());
			if(!par || !par->IsPlanetoidSize())
				RenderGravel(projection, model, ari);
		}
	}
}

VEHICLE_SPOT_TYPE DaUnit::InVehicleTransporter() const
{
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
	if(!vehicle || !vehicle->IsInCapsule())
		return SPOT_Invalid;

	// I am in a vehicle that is in a spacecraft.
	const DaCapsule* ship = vehicle->FindParentCapsuleInEnv();
	if(!ship || !ship->HasVehicleTransporter() || !vehicle->InParkingSpot())
		return SPOT_Invalid;

	// Spacecraft has a vehicle transporter and the vehicle is in a parking spot.
	return vehicle->ParkingSpotType();
}

void DaUnit::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(IsBurning())
	{
		AuPoint lightPos = model.Translation();
		QColor colLight(AuGLLight::ColorTorch());
		float lightRadius = (FireRadius() * 10) * AuItem::LightFlicker(CID_StoneFirePit, Id());
		float kQ = 1.f/(lightRadius * lightRadius);
		float kL = .0065f;
		lights.append(AuGLLight(Id(), AuGLLight::Effect, kL,kQ, colLight, lightPos));
	}
	DaAtom::GetRenderLightsRecursive(person, model, suns, lights);
}

bool DaUnit::RepairConditions()
{
	bool anyChange = false;
	m_ConditionsLock.lockForWrite();
	AuConditionList::iterator ci = m_Conditions.begin();
	while(ci != m_Conditions.end())
	{
		switch((*ci)->CType())
		{
		case AuCondition::Acid:
		case AuCondition::Disease:
		case AuCondition::Fire:
		case AuCondition::Paralysis:
		case AuCondition::Poison:
			delete *ci;
			ci = m_Conditions.erase(ci);
			anyChange = true;
			break;
		case AuCondition::Antitoxin:
		case AuCondition::Bandage:
		case AuCondition::Bounty:
		case AuCondition::UNUSED:
		case AuCondition::ImplantDNA:
		case AuCondition::ImplantSpecies:
		case AuCondition::Invalid:
		case AuCondition::Pregnant:
		case AuCondition::Thought:
			ci++;
			break;
		}
	}
	if(anyChange)
		m_tmConditionChange.Set();
	m_ConditionsLock.unlock();
	return anyChange;
}

int DaUnit::FindTransporterTarget(const AuId& targetId, TRANSPORTER_TARGET target, DaAtom*& targAtom, AuPoint& targLoc,
		AuPoint& targZ) const
{
	targAtom = 0;
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 1;	// Not likely.
	DaAtom* dest = system->FindIdRecursive(targetId);
	if(!dest)
		return 1;	// Not likely.
	const DaCapsule* ship = FindParentCapsuleInEnv();
	if(!ship)
		return 1;
	// Geological features are a range of TT_ values, with a common target atom, which is the world.
	if(target >= TT_GeologicalFeatureFirst && target <= TT_GeologicalFeatureLast)
	{
		// The target is one of the geological features on my world.
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(dest);
		if(!llworld)
			return 1;

		int tt = TT_GeologicalFeatureFirst;
		const AuGeologicalFeatures& geof = llworld->LockGeologicalFeatures();
		for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
		{
			if(!(*gi)->IsTransporterTarget())
				continue;
			if((TRANSPORTER_TARGET)tt == target)
			{
				// Found the target feature.
				AuGlobeLLF targLL = (*gi)->LL();
				targAtom = llworld;
				targLoc = llworld->LocOfLL(targLL);
				targZ = -llworld->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter() + targY * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter();

				//DHH bd city check location for coverage by shield.
				break;
			}
			tt++;
			if(tt > TT_GeologicalFeatureLast)
				break;
		}
		llworld->UnlockGeologicalFeatures();
	}
	else
	{
		// Remaining targets are only one per atom id.
		switch(target)
		{
		case TT_GeologicalFeatureFirst:
		case TT_GeologicalFeatureLast:
		case TT_Invalid:
		case TT_Transporter:
			return 1;

		case TT_Spaceship:
		case TT_Starship:
		case TT_SpaceStation:
			{
				DaVessel* ves = dynamic_cast<DaVessel*>(dest);
				if(!ves)
					return 1;
				if(ves->ShieldsBlockTransportAboard())
					return 4;	// Blocked by shields.
				if(!ves->TransporterVoidLocation(targLoc))
					return 1;
				targAtom = ves;
				quint16 roomId;
				ROOMTYPE roomType;
				double htAbvFloor;
				QString roomName;
				ves->InsideRoomType(targLoc, true, roomId, roomType, htAbvFloor, roomName, targZ);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();
			}
			break;

		case TT_Bd:
			{
				DaBd* bd = dynamic_cast<DaBd*>(dest);
				if(!bd)
					return 1;
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(bd->FindParentPlanetInEnv());
				if(!llworld)
					return 1;	// Must have a planet.

				// Get a site location from the building.
				if(bd->TransporterVoidLocation(targLoc))
				{
					targAtom = bd;
					quint16 roomId;
					ROOMTYPE roomType;
					double htAbvFloor;
					QString roomName;
					bd->InsideRoomType(targLoc, true, roomId, roomType, htAbvFloor, roomName, targZ);
				}
				else
				{
					targAtom = llworld;
					targLoc = bd->RandomSpawnLoc();	// Location is relative to my parent world.
					targZ = -llworld->GravityDir(targLoc);
				}

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				if(PoliticalStance(bd->EmpireId(), ship->EmpireId()) == POL_Enemy &&
						llworld->SubatomShieldsBlockTransporters(ship->EmpireId(), targLoc))
					return 2;	// Blocked by shields.
			}
			break;

		case TT_Beacon:
		case TT_Avatar:
		case TT_Captain:
		case TT_Officer:
		case TT_Crew:
		case TT_Troop:
		case TT_Passenger:
			{
				if(dest->IsInCapsule() || dest->IsAboardVehicle())
					return 1;
				DaWorld* world = dynamic_cast<DaWorld*>(dest->FindParentPlanetInEnv());
				if(!world)
					return 1;	// Must have a planet.
				if(!dest->GetLocationRelativeToParent(targLoc, world))
					return 1;
				targAtom = world;
				targZ = -world->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

				//DHH bd city check location for coverage by shield.
			}
			break;

		case TT_ClaimFlag:
			{
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(dest);
				if(!llworld)
					return 1;
				if(!llworld->ClaimedByEmpire())
					return 1;
				AuGlobeLLF targLL = llworld->ClaimedAtLL();
				targAtom = llworld;
				targLoc = llworld->LocOfLL(targLL);
				targZ = -llworld->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

				//DHH bd city check location for coverage by shield.
			}
			break;

		case TT_Henge:
			{
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(dest);
				if(!llworld)
					return 1;
				if(!llworld->IsHengeActive())
					return 1;
				AuGlobeLLF targLL = llworld->HengeLL();
				targAtom = llworld;
				targLoc = llworld->LocOfLL(targLL);
				targZ = -llworld->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter() + targY * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter();

				//DHH bd city check location for coverage by shield.
			}
			break;

		case TT_RingworldControlCenter:
			{
				DaWorldRingSection* llworld = dynamic_cast<DaWorldRingSection*>(dest);
				if(!llworld)
					return 1;
				if(llworld->SectionIndex())
					return 1;	// Only section 0 has a control center.
				AuGlobeLLF targLL(0,0);
				targAtom = llworld;
				targLoc = llworld->LocOfLL(targLL);
				targZ = -llworld->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

				//DHH bd city check location for coverage by shield.
			}
			break;

		case TT_Village:
			{
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(dest);
				if(!llworld)
					return 1;
				if(!llworld->HasVillage())
					return 1;
				AuGlobeLLF targLL = llworld->VillageLL();
				targAtom = llworld;
				targLoc = llworld->LocOfLL(targLL);
				targZ = -llworld->GravityDir(targLoc);

				// Also calculate X and Y vectors to find a random location nearby.
				AuPoint targX;
				targZ.ArbitraryAxis(targX);
				AuPoint targY = targZ.Cross(targX);
				targY.Unit();

				// Offset randomly from the target location.
				targLoc += targX * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter() + targY * g_Au.Dice().Roll(1,41,-21)*FeetPerMeter();

				//DHH bd city check location for coverage by shield.
			}
			break;
		}
	}
	if(!targAtom)
		return 1;
	return 0;	// good return
}

quint64 DaUnit::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaUnit) - sizeof(DaAtom);
	mem += StringMemoryUsage(m_sName);
	mem += m_UnitDamage.MemoryUsage();
	mem += m_Conditions.MemoryUsage();
	return mem;
}

void DaUnit::ListTreatableConditions(QStringList& conditions) const
{
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
		if((*ci)->IsTreatable())
			conditions.append((*ci)->Name());
	m_ConditionsLock.unlock();
}

bool DaUnit::HasInsuranceBounty() const
{
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
	{
		const AuConBounty* bounty = dynamic_cast<const AuConBounty*>(*ci);
		if(!bounty)
			continue;
		if(bounty->PosterId() != AuId::InsurerId())
			continue;
		m_ConditionsLock.unlock();
		return true;
	}
	m_ConditionsLock.unlock();
	return false;
}

double DaUnit::InsuranceBounty() const
{
	double reward = 0;
	m_ConditionsLock.lockForRead();
	for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
	{
		const AuConBounty* bounty = dynamic_cast<const AuConBounty*>(*ci);
		if(!bounty)
			continue;
		if(bounty->PosterId() != AuId::InsurerId())
			continue;
		reward += bounty->Reward();
	}
	m_ConditionsLock.unlock();
	return reward;
}

void DaUnit::PayInsuranceBounty(double amount)
{
	bool anyPaid = false;
	m_ConditionsLock.lockForWrite();
	AuConditionList::iterator ci = m_Conditions.begin();
	while(ci != m_Conditions.end())
	{
		AuConBounty* bounty = dynamic_cast<AuConBounty*>(*ci);
		if(bounty && bounty->PosterId() == AuId::InsurerId())
		{
			if(bounty->Reward() > amount)
			{
				// The entire amount can be deducted from the reward.
				bounty->ReduceReward(amount);
				anyPaid = true;
				break;
			}
			else
			{
				// The amount exceeds the reward.
				amount -= bounty->Reward();		// Reduce the amount.
				delete *ci;						// Delete the bounty.
				ci = m_Conditions.erase(ci);	// Remove the bounty from my list of conditions.
				anyPaid = true;
			}
		}
		else
			ci++;
	}
	if(anyPaid)
		m_tmConditionChange.Set();
	m_ConditionsLock.unlock();
}

bool DaUnit::IsBlackListedByInsurer() const
{
	return InsuranceBounty() > 1000000;
}

DaAtom::ATOM_TYPE DaUnit::AtomType() const
{
	return ATOM_Unit;
}

void DaUnit::EmpireAndFriendlies(QList<quint32>& empires) const
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return;

	// Append my empire id.
	empires.append(empireId);

	// Append empires that consider my empire to be their friend.
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireFriendlies(empireId, empires);
	g_Au.UnlockData();
}

void DaUnit::EmpireAndFriends(QList<quint32>& empires) const
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return;

	// Append my empire id.
	empires.append(empireId);

	// Append empires that my empire considers to be friends.
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireFriends(empireId, empires);
	g_Au.UnlockData();
}

bool DaUnit::IsNoncombatant() const
{
	quint32 empireId = EmpireId();
	if(!empireId)
		return false;		// Units without an empire are not protected from attack.
	if(DgEmpire::IsPirate(empireId))
		return false;		// Pirates are not protected from attack.
	if(InArena())
		return false;		// Units in the arena are not protected.
	if(InDesignPreview())
		return false;		// Units in design preview are never protected.

	// Unit must be at a world that has an empire base.
	if(WorldAtomType() == ATOM_Invalid)
		return false;		// Not at a world.

	// When my empire has no citizens online, units at worlds with empire bases are protected from attack.
	// When my empire has no citizens at all, none of our units are protected from attack.
	// When my empire does not exist, none of our units are protected from attack.
	bool isDormant;
	const DgData& gd = g_Au.LockDataForRead();
	if(gd.HasEmpire(empireId))
		isDormant = !gd.EmpireIsCitizenOnLine(empireId);	// Within the past fifteen minutes.
	else
		isDormant = false;	// Empire does not exist so units are not protected. Possibly deleted because of no avatars.
	g_Au.UnlockData();
	if(!isDormant)
		return false;

	// I am at a world. Does it contain an empire base?
	const DaWorldLL* world = FindParentWorldInEnv();
	if(!world)
		return false;		// Env query out of date?
	if(world->ClaimedByEmpire() != empireId)
		return false;		// World is not claimed by my empire so it cannot have a base.
	if(!world->HQHere())
		return false;		// World does not have a military base.

	// All tests passed to be considered a noncombatant unit.
	return true;
}

