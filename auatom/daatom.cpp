//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daatom.cpp,v 1.1368 2022/05/17 15:18:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daatom.h"
//
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "aueffect.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auitem.h"
#include "auParticleSystem.h"
#include "auquad.h"
#include "ausound.h"
#include "ausynccontroller.h"
#include "auterraininfo.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacapsule.h"
#include "dadesigner.h"
#include "dadesignplane.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dgempire.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QApplication>
#include <QDataStream>
#include <QDebug>
#include <QSqlQuery>
#include <QVector>
#include "float.h"

#if defined(MEM_TRACE_ATOM)
static quint64 s_AllocatedAtoms = 0;
static quint64 s_FreedAtoms = 0;
#endif

//static unsigned int systemTHits = 0;
//static unsigned int systemTMisses = 0;

#define d_BombChainReaction				4						// Number of other bombs each bomb can detonate in an explosion.
#define d_WaterTolerance				.083					// Distance cg has to be below water surface to be "under water".

AuModelMatrixHash::AuModelMatrixHash()
{
	// How about far enough to orbit the largest gas giant without clipping away its atmosphere.
	// 32000 is the near clip distance set in the far projection in AuView.
	m_FarDistance = ::Radius100km(400) + AtmosphereTopGasGiantMSL() + 32000;
	//qDebug() << "AuModelMatrixHash::AuModelMatrixHash oldFar" << sqrt(DaVessel::SightRange2()) << "farDist" << m_FarDistance;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuRenderLastMap::Insert(AuModelMatrixHash& rmod, DaAtom* atom)
{
	AuModelMatrixHash::iterator rmi = rmod.find(atom->Id());
	if(rmi == rmod.end())
		return;
	Insert(*rmi, atom);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaAtom::~DaAtom()
{
	// Last test showed showed scene server hitting 61% and scene client hitting 49%.
	//if(systemTHits || systemTMisses)
		//qDebug() << "DaAtom::~DaAtom" << Id() << "systemT cache hits" << systemTHits << "misses" << systemTMisses << "score"
			//<< (systemTHits * 100) / (systemTHits + systemTMisses) << "% hits";
	if(g_Au.m_LogMessages & LOG_AtomKillAndDestruct)
		g_Au.Log(tr("0x%1 DaAtom::~DaAtom %2")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(Id().GetID()));

	if(m_Effects)
		delete m_Effects;

	if(m_Environ)
		delete m_Environ;

	if(m_SyncController)
		delete m_SyncController;
}

DaAtom::DaAtom(const DaAtom &rhs) : QObject()
{
	m_AnyScene = rhs.m_AnyScene;
	m_AtRest = rhs.m_AtRest;
	m_Ether = rhs.m_Ether;
	m_Id = rhs.m_Id;
	m_MiniScale = rhs.m_MiniScale;
	m_MoveResult = rhs.m_MoveResult;
	m_State = rhs.m_State;
	m_Ticked = rhs.m_Ticked;
	m_tmCreated = rhs.m_tmCreated;

	rhs.m_PosLock.lockForRead();
	m_OrientRotation = rhs.m_OrientRotation;
	m_Pos = rhs.m_Pos;
	m_PosChange = rhs.m_PosChange;
	m_Vector = rhs.m_Vector;
	rhs.m_PosLock.unlock();

	rhs.m_EnvTransformLock.lockForRead();
	m_EnvTransform = rhs.m_EnvTransform;
	m_EnvTransformValid = rhs.m_EnvTransformValid;
	rhs.m_EnvTransformLock.unlock();

	rhs.m_ControlLock.lockForRead();
	m_Controller = rhs.m_Controller;
	if(rhs.m_SyncController)
		m_SyncController = new AuSyncControllerList(*rhs.m_SyncController);
	else
		m_SyncController = 0;
	m_tmControlled = rhs.m_tmControlled;
	rhs.m_ControlLock.unlock();

	rhs.m_DeathLock.lockForRead();
	m_tmDeath = rhs.m_tmDeath;
	rhs.m_DeathLock.unlock();

	rhs.m_ModifyLock.lockForRead();
	m_tmModify = rhs.ModifyTime();
	rhs.m_ModifyLock.unlock();

	rhs.m_TickLock.lockForRead();
	m_tmTicked = rhs.m_tmTicked;
	rhs.m_TickLock.unlock();

	rhs.m_TockLock.lockForRead();
	m_tmTocked = rhs.m_tmTocked;
	rhs.m_TockLock.unlock();

	rhs.m_EffectsLock.lockForRead();
	if(rhs.m_Effects)
		m_Effects = new AuEffectArray(*rhs.m_Effects);
	else
		m_Effects = 0;
	m_tmEffectsChange = rhs.m_tmEffectsChange;
	rhs.m_EffectsLock.unlock();

	rhs.m_EnvironLock.lockForRead();
	if(rhs.m_Environ)
		m_Environ = new AuQueryEnvironment(*rhs.m_Environ);
	else
		m_Environ = 0;
	m_EnvironState = rhs.m_EnvironState;
	m_EnvVector = rhs.m_EnvVector;
	m_Illumination = rhs.m_Illumination;
	rhs.m_EnvironLock.unlock();

	m_FirstActiveAudioIdx = ATOM_SOUNDS;
	for(int cs=0; cs<ATOM_SOUNDS; cs++)
	{
		m_Audio[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaAtom::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	//if(Is(AuId::Haxus()))
		//qDebug() << "DaAtom::TxUpdate" << Id() << Name() << "parchange" << (bool)(m_tmParentChange >= tmLastUpdate);
	toClient << m_Id;

	m_ModifyLock.lockForRead();
	m_tmModify >> toClient;
	m_ModifyLock.unlock();

	m_EffectsLock.lockForRead();
	int effcount = m_Effects?m_Effects->CountNew(tmLastUpdate):0;
	m_EffectsLock.unlock();

	quint8 hasWhat = 0;
	if(lastTx.IsNull())
	{
		if(effcount)
			hasWhat = 0xff;		// everything, including effects
		else
			hasWhat = 0xef;		// everything except effects
	}
	else
	{
		m_PosLock.lockForRead();
		if(m_PosChange >= lastTx)
			hasWhat |= 0x01;
		m_PosLock.unlock();

		if(IsDying())
			hasWhat |= 0x04;

		m_EffectsLock.lockForRead();
		if(m_tmEffectsChange >= tmLastUpdate && effcount)
			hasWhat |= 0x10;
		m_EffectsLock.unlock();
	}
	if(m_AtRest)
		hasWhat |= 0x20;
	if(m_SyncController && IsControllerNoReset(sceneAtom.Id()))
		hasWhat |= 0x40;
	toClient << hasWhat;

	if(hasWhat & 0x01)
		Pos().Tx(toClient);
	//toClient << m_State;
	if(hasWhat & 0x02)
		m_tmCreated >> toClient;	// only set on init
	if(hasWhat & 0x04)
	{
		m_DeathLock.lockForRead();
		m_tmDeath >> toClient;
		m_DeathLock.unlock();
	}
	if(hasWhat & 0x10)
	{
		// Send effects added since last time.
		if(effcount)
		{
			// Recount the number of effects we will send.
			m_EffectsLock.lockForRead();
			effcount = 0;
			for(QList<AuEffect>::const_iterator it = m_Effects->m_Effects.cbegin(); it != m_Effects->m_Effects.cend(); it++)
				if((*it).IsNew(tmLastUpdate))
					effcount++;

			// Limit the number of effects sent.
			quint8 sendCount;
			if(effcount > 10)
				sendCount = 10;
			else
				sendCount = effcount;
			toClient << sendCount;

			// Send the effects.
			for(QList<AuEffect>::const_iterator it = m_Effects->m_Effects.cbegin(); it != m_Effects->m_Effects.cend() && sendCount;
					it++)
				if((*it).IsNew(tmLastUpdate))
				{
					(*it) >> toClient;
					sendCount--;
				}
			m_EffectsLock.unlock();
		}
	}
	if(hasWhat & 0x40)
	{
		m_ControlLock.lockForWrite();
		if(m_SyncController)
		{
			toClient << *m_SyncController;
			m_SyncController->Clear();
			delete m_SyncController;
			m_SyncController = 0;
		}
		else
			toClient << (quint8)0;			// wut? Send a list count of zero so the receiving end doesn't blow up.
		m_ControlLock.unlock();
	}
}

// Receive base id and modify time for this atom from a server.
void DaAtom::RxUpdateId(QDataStream& fromServer)
{
	fromServer >> m_Id;
	fromServer >> m_tmModify;
	//if(Is(AuId::Haxus()))
		//qDebug() << "DaAtom::RxUpdateId" << Id() << Name() << "modify" << m_tmModify;
}

// Receive updates to this atom from a server.
// DaAtom Id and ParentId data is received during RxUpdateBase. The atom is then
// connected to the proper parent before RxUpdate is called to get the rest of
// the data.
void DaAtom::RxUpdateBase(QDataStream& fromServer)
{
	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
	{
		AuAtomPos pos;
		pos.Rx(fromServer);
		ChangePos(pos);
		InvalidateEnvironment();
	}

	m_State = Normal;	// Server does not send abnormal atoms.
	if(hasWhat & 0x02)
		m_tmCreated << fromServer;
	if(hasWhat & 0x04)
		m_tmDeath << fromServer;
	if(hasWhat & 0x10)
	{
		// Effects
		if(!m_Effects)
			m_Effects = new AuEffectArray;
		quint8 effcount;
		fromServer >> effcount;
		AuEffect effect;
		while(effcount)
		{
			effect << fromServer;
			m_Effects->AddIfUnique(effect);
			effcount--;
		}
	}

	m_AtRest = (hasWhat & 0x20) ? true : false;

	if(hasWhat & 0x40)
	{
		// Physics demands are only sent once. I can accumulate them.
		m_ControlLock.lockForWrite();
		if(!m_SyncController)
			m_SyncController = new AuSyncControllerList;
		m_SyncController->Append(fromServer);
		if(m_SyncController->empty())
		{
			// wut?
			delete m_SyncController;
			m_SyncController = 0;
		}
		m_ControlLock.unlock();
	}
}

// Refresh data from valid counterpart.
bool DaAtom::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	Q_UNUSED(ari);
	bool transformChanged = false;	// Return value;

	Q_ASSERT(pAtom);
	m_State = pAtom->m_State;
	m_tmCreated = pAtom->m_tmCreated;

	pAtom->m_DeathLock.lockForRead();
	m_DeathLock.lockForWrite();
	m_tmDeath = pAtom->m_tmDeath;
	m_DeathLock.unlock();
	pAtom->m_DeathLock.unlock();

	pAtom->m_ModifyLock.lockForRead();
	m_ModifyLock.lockForWrite();
	m_tmModify = pAtom->m_tmModify;
	m_ModifyLock.unlock();
	pAtom->m_ModifyLock.unlock();

	m_AtRest = pAtom->m_AtRest;

	AuAtomPos apos(pAtom->Pos());
	AuAtomPos mpos(Pos());
	if(apos != mpos)
	{
		ChangePos(apos);
		transformChanged = true;
	}

	pAtom->m_EffectsLock.lockForRead();
	m_EffectsLock.lockForWrite();
	if(pAtom->m_Effects && !pAtom->m_Effects->m_Effects.isEmpty())
	{
		// The source atom has some nu audio/visual effects for me.
		if(!m_Effects)
			m_Effects = new AuEffectArray;
		if(m_Effects->AddNewer(*pAtom->m_Effects, m_tmEffectsChange))
			m_tmEffectsChange.Set();
	}
	m_EffectsLock.unlock();
	pAtom->m_EffectsLock.unlock();

	/*DHH Perhaps I can deal with these in the etherium?
	// Physics demands are only sent once. I can accumulate them.
	pAtom->m_ControlLock.lockForWrite();
	m_ControlLock.lockForWrite();
	if(pAtom->m_SyncController)
	{
		if(!m_SyncController)
			m_SyncController = new AuSyncControllerList(*pAtom->m_SyncController);
		else
			m_SyncController.Append(*pAtom->m_SyncController);

		pAtom->m_SyncController->Clear();
		delete pAtom->m_SyncController;
		pAtom->m_SyncController = 0;
	}
	m_ControlLock.unlock();
	pAtom->m_ControlLock.unlock();
	*/
	if(transformChanged)
		InvalidateEnvironment();

	return transformChanged;
}

void DaAtom::RefreshComplete()
{
	// Clear the source atom's effects data that we've grabbed a copy.
	// Effects are so transient that we don't want to keep them around.
	// Only atoms in the render scene get Refresh'd. These atoms will
	// manage their effects during DaAtom::AnimateDeviceObjects.
	if(m_Effects)
		m_Effects->Tick();	// Throw them away as they get too old.
}

bool DaAtom::IsTicker() const
{
	if(IsDead())
		return false;
	if(!IsAtRest() && !IsClientControlledNoReset())
		return true;
	return false;
}

void DaAtom::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	SetTickTime(now);
	m_Ticked = 2;
	if(IsDead())
		return;

	if(!DoesQueryEnvironment())
		return;

	// Apply movement physics.
	if(!IsClientControlled())
	{
		// Last client control was long enough ago that I will resume handling movement physics on this server.
		TickMovement(last);
	}
}

void DaAtom::TickOnClient(const AuElapsedTimer& last)
{
	SetTickTime(last);

	// Propagate the time pulse to the subatoms.
	DaAtomList subs;
	ListSubatoms(subs);
	subs.TickOnClient(last);

	/*
	// Animate audio/visual effects to remove those that have grown too old to bother sending to anyone.
	if(m_Effects && !m_Effects->isEmpty())
		m_Effects->Tick();
		*/
}

// Move due to movement vector, i.e. gravity, inertia. This requires up-to-date environment query.
// Movement is performed before moving subatoms due to EnvTransform.
void DaAtom::TickMovement(const AuElapsedTimer& last)
{
	if(IsAtRest())
		return;
	if(!IsNormal())
		return;

	// Calculate number of milliseconds elapsed since last tick.
	qint64 msElapsed = last.elapsed();
	if(msElapsed < 0)
		msElapsed = 1;

	if(msElapsed > 500)
	{
		if(g_Au.m_LogMessages & LOG_Physics)
			qDebug() << "DaAtom::TickMovement" << *this << "msElapsed" << msElapsed << "too long for movement";
		return;
	}

	const double rsec = (double)msElapsed/1000.;	// Real seconds passed.

	if(OkToOrient())
	{
		// Rotate to desired orientation.
		AuPoint rot = Rotation();
		AuPoint orientRot = OrientRotation();
		if(!rot.FuzzyEquals(orientRot))
		{
			double maxChange = rsec * OrientationSpeed();
			AuPoint rotVec = orientRot - rot;
			if(rotVec.x > PI)
				rotVec.x -= PI*2;
			else if(rotVec.x < -PI)
				rotVec.x += PI*2;

			if(rotVec.y > PI)
				rotVec.y -= PI*2;
			else if(rotVec.y < -PI)
				rotVec.y += PI*2;

			if(rotVec.z > PI)
				rotVec.z -= PI*2;
			else if(rotVec.z < -PI)
				rotVec.z += PI*2;

			if(rotVec.x > maxChange)
				rotVec.x = maxChange;
			else if(rotVec.x < -maxChange)
				rotVec.x = -maxChange;

			if(rotVec.y > maxChange)
				rotVec.y = maxChange;
			else if(rotVec.y < -maxChange)
				rotVec.y = -maxChange;

			if(rotVec.z > maxChange)
				rotVec.z = maxChange;
			else if(rotVec.z < -maxChange)
				rotVec.z = -maxChange;

			AuPoint nuRot = rot + rotVec;

			// Preserve movement as a vector relative to parent. This will be immune to changes in my rotation.
			AuAtomPos pos(Pos());
			AuPoint omv = pos.ProjectVector(Vector());
			AuPoint ogn = pos.ProjectVector(GravityNormal());
			AuPoint otn = pos.ProjectVector(TerrainNormal());

			AuMatrix4x4 tOrient;
			tOrient.Translate(Location());
			tOrient.Rotate(nuRot);
			pos.SetTransform(tOrient);
			ChangePos(pos);
			//AuPoint finrot = Rotation();
			//qDebug() << "orientRot" << (orientRot*RAD2DEG) << "rot" << (rot*RAD2DEG) << "rotVec" << (rotVec*RAD2DEG)
				//<< "nuRot" << (nuRot*RAD2DEG) << "finrot" << (finrot*RAD2DEG);

			// Restore movement as a vector relative to me.
			SetVector(pos.UnProjectVector(omv));
			SetGravityNormal(pos.UnProjectVector(ogn));
			SetTerrainNormal(pos.UnProjectVector(otn));
		}
	}

	// Calc acceleration change according to gravity.
	const AuPoint accel = Acceleration();
	const AuPoint drag = Drag();
	const AuPoint fric = Friction();
	const AuPoint dragfric = (drag + fric) * rsec;
	AuPoint vector = Vector() + accel * rsec;

	if(vector.x > 0)
	{
		vector.x -= dragfric.x;
		if(vector.x < 0)
			vector.x = 0;
	}
	else if(vector.x < 0)
	{
		vector.x += dragfric.x;
		if(vector.x > 0)
			vector.x = 0;
	}
	if(vector.y > 0)
	{
		vector.y -= dragfric.y;
		if(vector.y < 0)
			vector.y = 0;
	}
	else if(vector.y < 0)
	{
		vector.y += dragfric.y;
		if(vector.y > 0)
			vector.y = 0;
	}
	if(vector.z > 0)
	{
		vector.z -= dragfric.z;
		if(vector.z < 0)
			vector.z = 0;
	}
	else if(vector.z < 0)
	{
		vector.z += dragfric.z;
		if(vector.z > 0)
			vector.z = 0;
	}

	// Perform atom-specific filtering on the final move vector.
	// Atoms need this to keep from creeping across the terrain.
	//AuPoint veco(vector);
	FilterFinalMoveVector(vector);
	//qDebug() << "DaAtom::TickMovement" << Id() << Name() << "veco" << veco << "filtered vector" << vector;

	if(IsInGravity())
	{
		// Prevent feet from hanging through the floor/ground surface.
		bool moveMe = false;
		AuAtomPos pos;
		bool canbug = CanBeUnderGround();	// virtual, could try to lock m_Environ
		LockEnvironmentQuery();
		if(!canbug && FuzzyLessThan(m_Environ->m_FallDist, 0))
		{
			// This atom is below the floor surface and it is not permitted to be there.
			// Move up (-) to adjust to ground surface altitude.
			// Force myself up to the proper height instantly without any collision checking.
			pos = Pos();
			pos.Translate(m_Environ->m_GravityNormal * -m_Environ->m_FallDist);
			moveMe = true;

			// Update environment query data.
			m_EnvironLock.unlock();
			m_EnvironLock.lockForWrite();
			if(m_Environ)
			{
				if(m_Environ->m_fSeaDepth > m_Environ->m_FallDist)
					m_Environ->m_fSeaDepth -= m_Environ->m_FallDist;
				else
					m_Environ->m_fSeaDepth = 0;
				m_Environ->m_FallDist = 0;
			}
		}
		UnlockEnvironmentQuery();
		if(moveMe)
			ChangePos(pos);
	}

	// Calculate move amounts based on move rate and time span.
	AuPoint tmove;						// Move vector in this time period.
	if(vector.IsFuzzyZero())
	{
		// Insure my movement vector is thoroughly zeroed, not fuzzy zero.
		vector = AuPoint(0,0);
		SetVector(vector);
	}
	else
		tmove = vector * rsec;			// Calculate move vector in this time period

	AuId oldParentId = ParentId();		// Watch for parent change resulting from the move, possible even if vec = 0.
	bool moved = Move(tmove, msElapsed);
	if(moved)
	{
		// Parent change may have modified my movement vector. e.g. exiting a moving spacecraft
		if(oldParentId != ParentId())
			vector = Vector();	// Get my nu movement vector.
	}
	else
	{
		// Could not move.
		vector = Vector();	// May have been reflected or absorbed.

		// Resume 'at rest' state when no longer able to move.
		if(vector.IsFuzzyZero())
		{
			bool r = PosChangeTime().ElapsedSec() > 1;
			if(r)
				SetAtRest();
		}
	}

	if(!IsNormal())
	{
		// Atom movement resulted in change to non-normal state, such as atom Kill or wormhole handoff.
		SetModified();
		return;
	}

	// Atom movement did not result in change to non-normal state, such as atom Kill or wormhole handoff.
	// Perform atom-specific filtering on the post-move vector.
	// Atoms need this to nullify the gravity vector when they have moved on the ground.
	FilterPostMoveVector(vector);

	if(!IsInGravity())
	{
		// In a weightless environ.
		if(accel.IsZero())
		{
			if(vector.IsFuzzyZero())
			{
				bool r = PosChangeTime().ElapsedSec() > 1;
				if(r)
					SetAtRest();
			}
		}
	}
	else if(IsTouchingGround())
	{
		// Touching a ground or floor surface with gravity.
		// Resume 'at rest' state when no longer moving on the ground.
		if(vector.IsZero())
		{
			bool r = PosChangeTime().ElapsedSec() > 1;
			if(r)
				SetAtRest();
		}
	}
	else if(IsInWater())
	{
		// In water.
		// Resume 'at rest' state when no longer moving.
		if(vector.IsZero())
		{
			bool r = PosChangeTime().ElapsedSec() > 1;
			if(r)
				SetAtRest();
		}
	}

	if(!IsAtRest() && vector != Vector())
		SetVector(vector);

	SetModified();
}

unsigned int DaAtom::IsTocker() const
{
	if(IsDead())
		return d_MSecPerSec;			// So I will get moved to the trash.
	if(IsTraveling() || IsDormant() || m_State == Trash)
		return 0;
	if(ParentId().IsNull())
		return 0;

	if(m_Effects)
	{
		m_EffectsLock.lockForRead();
		bool hasEffects = m_Effects && !m_Effects->m_Effects.empty();
		m_EffectsLock.unlock();
		if(hasEffects)
			return d_MSecPerSec/2;
	}

	if(IsDying())
		return d_MSecPerSec;

	// Every atom in an atmosphere that is not inside a vehicle or spacecraft must test hull stress once per second.
	if(DoesQueryEnvironment())
	{
		//if(IsInAtmosphere() && !IsInVehicle() && !IsInsideSpacecraft())
			//return d_MSecPerSec;
		// Reworked so environment query is not needed.
		const DaAtom* parent = Parent();
		if(parent)
		{
			const DaVehicle* veh = dynamic_cast<const DaVehicle*>(parent);
			bool insideVeh = veh && veh->IsEnclosed();
			if(!insideVeh && !parent->FindParentCapsuleInEnv())
			{
				// I am not inside a vehicle, spacecraft or building.
				// Test IsInAtmosphere.
				const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(parent);
				if(world)
				{
					// I am at a world.
					if(world->InAtmosphereCGAltitude(Location()) != -100000)
						return d_MSecPerSec;	// I am in the atmosphere envelope of a world.
				}
			}
		}

		// Only atoms that query their environment can have combat damage.
		m_DamageToApplyLock.lockForRead();
		bool anyDamage = !m_DamageToApply.isEmpty();
		m_DamageToApplyLock.unlock();
		if(anyDamage)
			return d_MSecPerSec;
	}

	if(IsInAnyScene() || IsTicking())
		return 10*d_MSecPerSec;

	return 0;
}

void DaAtom::Tock(const AuTime& last, const AuTime& now, const AuScripts& /*scripts*/, DtTockTransactions& tockTrans)
{
	SetTockTime(now);

	// Check for dying.
	m_DeathLock.lockForRead();
	bool isd = (!m_tmDeath.IsNull() && now >= m_tmDeath);
	m_DeathLock.unlock();
	if(isd)
	{
		// This atom has died a scheduled death.
		//qDebug() << "DaAtom::Tock" << Id() << Name() << "time to die" << DeathTime() << "now" << AuTime::Now();
		Kill();				// Kill it and all of its subatoms recursively. Could fail.
	}
	if(IsDead())
		return;		// It's time for this atom to go to a higher place. The caller will move it into the trash for deletion.
	//if(IsDying())
		//qDebug() << "DaAtom::Tock" << Id() << Name() << "will die at" << DeathTime() << "now" << AuTime::Now();

	if(last.IsNewPeriod(now, 10*d_MSecPerSec, Id().GetID()))
	{
		// Ticked to a new second and it is a 10 second interval, skewed by my id so all atoms don't do this at once.
		if(m_AnyScene)
			m_AnyScene--;
		if(m_Ticked)
			m_Ticked--;
	}

	// Apply combat damage queue.
	m_DamageToApplyLock.lockForRead();
	bool anyDamage = !m_DamageToApply.empty();
	m_DamageToApplyLock.unlock();
	if(anyDamage)
	{
		m_DamageToApplyLock.lockForWrite();
		QList<AuDamageToApply> dmgList(m_DamageToApply);
		m_DamageToApply.clear();
		m_DamageToApplyLock.unlock();
		for(QList<AuDamageToApply>::iterator di = dmgList.begin(); di != dmgList.end(); di++)
			CombatApplyDamage((*di).m_Attacker, (*di).m_Weapon, (*di).m_WpnLoc, (*di).m_HitLoc, (*di).m_Result);
		if(IsDead())
			return;												// Atom was destroyed.
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Animate audio/visual effects to remove those that have grown too old to bother sending to anyone.
	if(m_Effects)
	{
		m_EffectsLock.lockForWrite();
		if(m_Effects && !m_Effects->m_Effects.empty())
			m_Effects->Tick();
		m_EffectsLock.unlock();
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Every atom in an atmosphere that is not inside a vehicle or spacecraft must test hull stress once per second.
	if(last.ElapsedSecTicks(now))
		if(DoesQueryEnvironment() && IsInAtmosphere() && !IsInVehicle() && !IsInsideCapsule())
		{
			TestHullStress();
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		}
}

// Kills this atom and all subatoms recursively.
void DaAtom::Kill()
{
	if(g_Au.m_LogMessages & LOG_AtomKillAndDestruct)
		g_Au.Log(tr("0x%1 DaAtom::Kill %2 %3 %4")
				.arg((size_t)this, 12,16,QChar('0'))
				.arg(Id().GetID())
				.arg(Name())
				.arg(AtomTypeName()));

	// Prevent Killing DaPerson atoms.
	if(DaAtom::FindPersonRecursive())
	{
		qWarning() << "DaAtom::Kill" << Id() << Name() << "has a person subatom. CANNOT KILL!";
		return;
	}

	m_State = Dead;

	StopDying();		// No longer return 'true' to the IsDying query.

	// Propagate the kill to subatoms so everybody under this atom dies and gets cleaned up properly.
	KillSubatoms();

	// Notify global data of atom death. This causes it to be deleted from the database.
	g_Au.AtomDeleted(Id());
	TickTock();
}

void DaAtom::CreateForScan()
{
	//Q_ASSERT(g_Au.IsSceneApp());
	m_Id = g_Au.NextAtomId();		// On the client, this allocates a local id that is not valid in the real universe.
	m_tmCreated.Set();
	SetModifyTime(m_tmCreated);
}

// Called the first time this object exists in the universe.
void DaAtom::Create()
{
	Create(g_Au.NextAtomId());
}

// Called the first time this object exists in the universe.
void DaAtom::Create(const AuId& myAtomId)
{
	m_Id = myAtomId;
	m_tmCreated.Set();
	SetModifyTime(m_tmCreated);
}

void DaAtom::SetModified(SET_MODIFIED_BITS notifyBits)
{
	// This debug produces a lot of output.
	if(g_Au.m_LogMessages & LOG_AtomModify)
		g_Au.Log(tr("Atom Modified %2 %3 Type %1 notifyBits %4").arg(AtomTypeName()).arg(Id().GetID()).arg(Name()).arg(notifyBits));

	if(Id().IsNull())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg;
			const DaAtom* parent = Parent();
			if(parent)
				logmsg = QObject::tr("DaAtom::SetModified - Atom with NULL id. name %1 desc %2 parent %3 %4 %5")
					.arg(Name()).arg(Description()).arg(parent->Id().GetID()).arg(parent->Name()).arg(parent->Description());
			else
				logmsg = QObject::tr("DaAtom::SetModified - Atom with NULL id. name %1 desc %2.").arg(Name()).arg(Description());
			g_Au.Error(logmsg);
		}
		return;
	}

	if(notifyBits & 0x02)
		SetModifyTime(AuTime::Now());	// Update scene clients.
	if(notifyBits & 0x01 && g_Au.m_EnableSetModified)
		g_Au.AtomChanged(Id());			// Notify global data of atom change. This causes it to be UPDATEd by the database thread.
}

// Report a longer description of this atom for examination purposes.
QString DaAtom::Description() const
{
	return QString();
}

void DaAtom::ConnectSignals()
{
	// Connect our signals to the slots in the app. Beware not to do this every time we get added to another atom or we
	// accumulate multiple signal/slot connections.
	if(g_Au.IsSceneSolo())
	{
		// This is a solo app.
		if(qApp->connect(this, SIGNAL(OperateTurboLift(AuId, AuId, quint16)),
					SLOT(OperateTurboLiftUI(AuId, AuId, quint16)),
					(Qt::ConnectionType)(Qt::QueuedConnection|Qt::UniqueConnection)))
		{
			// These connections were copied from the IsSceneClient leg.
			//qApp->connect(this, SIGNAL(EmpireRequested(quint32)),
					//SLOT(EmpireRequested(quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(RestoreConsoleSettings(quint8, QByteArray)),
					SLOT(RestoreConsoleSettings(quint8, QByteArray)), Qt::QueuedConnection);
			//qApp->connect(this, SIGNAL(EnteredWormholeOnClient(AuId, AuId, AuWormhole, float)),
					//SLOT(AtomEnteredWormhole(AuId, AuId, AuWormhole, float)), Qt::QueuedConnection);
			//qApp->connect(this, SIGNAL(CollisionOnClient(AuId, AuPoint, AuPoint, AuId, AuPoint, AuPoint, AuPoint, bool)),
					//SLOT(AtomCollision(AuId, AuPoint, AuPoint, AuId, AuPoint, AuPoint, AuPoint, bool)), Qt::QueuedConnection);
			//qApp->connect(this, SIGNAL(DeadheadedOnClient(AuId, AuId, AuPoint)),
					//SLOT(AtomDeadheaded(AuId, AuId, AuPoint)), Qt::QueuedConnection);
			//qApp->connect(this, SIGNAL(FellOnClient(AuId, quint32, int)),
					//SLOT(AtomFell(AuId, quint32, int)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(TransporterCoord(AuPoint)),
					SLOT(TransporterCoordUI(AuPoint)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(TransporterLongLat(AuGlobeLLF)),
					SLOT(TransporterLongLatUI(AuGlobeLLF)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CreateEmpire()),
					SLOT(CreateEmpireUI()), Qt::QueuedConnection);

			// These connections were copied from the server leg.
			qApp->connect(this, SIGNAL(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)), 
					SLOT(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(ArenaReady(AuArenaReady)),
					SLOT(ArenaReady(AuArenaReady)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarAccountActive(AuIdList)), 
					SLOT(AvatarAccountActive(AuIdList)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarBorn(AuId, AuId)), 
					SLOT(AvatarBorn(AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
					SLOT(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(CityStatusChanged(AuId, int, qint8)),
					SLOT(CityStatusChanged(AuId, int, qint8)),
					Qt::QueuedConnection);

			qApp->connect(this,
					SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
							QString, AuId, quint8)),
					SLOT(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
							QString, AuId, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(EnteredDesigner(AuId, int)), 
					SLOT(EnteredDesigner(AuId, int)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)), 
					SLOT(GovDossierAddDocument(quint32, quint32, QByteArray)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
					SLOT(GovSetCapitalUp(quint32, AuId, AuSpecies, AuId, int)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(DemiavatarDied(AuId)), 
					SLOT(DemiavatarDied(AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeActivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeActivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeCreatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeCreated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeDeactivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeDeactivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeDestinationSignal(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
							bool, quint8, bool)),
					SLOT(HengeDestinationConfirmed(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
							bool, quint8, bool)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeTeleportSignal(AuId, AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeTeleporterEntered(AuId, AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
					SLOT(OfficeAssigned(AuId, quint32, AuOffice)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)),
					SLOT(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(EmpireRecordPatent(quint32, quint16, quint8)),
					SLOT(EmpireRecordPatent(quint32, quint16, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(ClaimTitle(AuId, QString, AuId, QString, quint32)),
					SLOT(ClaimTitleUp(AuId, QString, AuId, QString, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SectorPirateBaseEncounter(AuId, AuId)),
					SLOT(SectorPirateBaseEncounter(AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
					SLOT(SendFundsUp(AuId, QString, AuId, double, quint8, quint8, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(ShipIt(AuItemTEUContainer)),
					SLOT(ShipIt(AuItemTEUContainer)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SpacecraftNameChanged(AuId, QString)),
					SLOT(SpacecraftNameChanged(AuId, QString)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
					SLOT(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
					SLOT(BountyPosted(AuId, QString, AuId, QString, double, int)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CityCaptured(quint32, quint32, AuId)),
					SLOT(CityCaptured(quint32, quint32, AuId)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CityDecayed(quint32, AuId)),
					SLOT(CityDecayed(quint32, AuId)));
			qApp->connect(this, SIGNAL(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)),
					SLOT(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)));
			qApp->connect(this, SIGNAL(CreditCityBank(float, AuId)),
					SLOT(CreditCityBank(float, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GameOver(AuId, quint16, quint8, QByteArray)),
					SLOT(GameOver(AuId, quint16, quint8, QByteArray)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(GameTurnPoll(AuId, quint16, AuId)),
					SLOT(GameTurn(AuId, quint16, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OfficerKilled(AuId)),
					SLOT(OfficerKilledUp(AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarOnLine(AuId, quint32)),
					SLOT(AvatarOnLineUp(AuId, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(PostJobOffer(AuJobOffer)), SLOT(PostJobOffer(AuJobOffer)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(WarpSignatureDetected(AuId, quint32, quint32)),
					SLOT(WarpSignatureDetected(AuId, quint32, quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SystemWarpSignature(AuId, quint32, quint8, float, float, float)),
					SLOT(SystemWarpSignature(AuId, quint32, quint8, float, float, float)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(ReleaseBerthSignal(AuId, AuId)), SLOT(ReleaseBerthUp(AuId, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
							quint8, quint8, QString, QString, QString)),
					SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
							quint8, quint8, QString, QString, QString)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SolarSystemExplored(AuId, quint32)), SLOT(SolarSystemExplored(AuId, quint32)),
					Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemScanned(AuId, quint32, quint8, quint32)),
					SLOT(SolarSystemScanned(AuId, quint32, quint8, quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemFinal(AuId)), SLOT(SolarSystemFinal(AuId)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemReadyToDecay(AuId)), SLOT(SolarSystemReadyToDecay(AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(StargateClosed(AuId, AuPoint)),
					SLOT(StargateClosed(AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
					SLOT(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(Spawn(AuId, quint32, AuId, AuId, AuId, quint8)),
					SLOT(SpawnAtom(AuId, quint32, AuId, AuId, AuId, quint8)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TradeReadyToShip(AuId, AuTradeAgreement)),
					SLOT(TradeReadyToShip(AuId, AuTradeAgreement)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TradeShipped(AuId, AuTradeAgreement)),
					SLOT(TradeShipped(AuId, AuTradeAgreement)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(Travel(AuId, AuId, AuId, AuId)),
					SLOT(AtomTraveled(AuId, AuId, AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TravelGalaxyLoc(AuId, AuId, AuId, AuPoint)),
					SLOT(AtomTraveledGalaxyLoc(AuId, AuId, AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TravelStargate(AuId, AuId, AuId, AuPoint)),
					SLOT(AtomTraveledStargate(AuId, AuId, AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(WormholeExplored(AuId, AuId, qint8, quint32, QString)),
					SLOT(WormholeExplored(AuId, AuId, qint8, quint32, QString)), Qt::QueuedConnection);
		}
	}
	else if(g_Au.IsSceneClient())
	{
		// This is a net client application.
		if(qApp->connect(this, SIGNAL(OperateTurboLift(AuId, AuId, quint16)),
                    SLOT(OperateTurboLiftUI(AuId, AuId, quint16)),
					(Qt::ConnectionType)(Qt::QueuedConnection|Qt::UniqueConnection)))
		{
			qApp->connect(this, SIGNAL(EmpireRequested(quint32)),
					SLOT(EmpireRequested(quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(RestoreConsoleSettings(quint8, QByteArray)),
					SLOT(RestoreConsoleSettings(quint8, QByteArray)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(EnteredWormholeOnClient(AuId, AuId, AuWormhole, float)),
					SLOT(AtomEnteredWormhole(AuId, AuId, AuWormhole, float)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CollisionOnClient(AuId, AuPoint, AuPoint, AuId, AuPoint, AuPoint, AuPoint, bool)),
					SLOT(AtomCollision(AuId, AuPoint, AuPoint, AuId, AuPoint, AuPoint, AuPoint, bool)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(DeadheadedOnClient(AuId, AuId, AuPoint)),
					SLOT(AtomDeadheaded(AuId, AuId, AuPoint)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(FellOnClient(AuId, quint32, int)),
					SLOT(AtomFell(AuId, quint32, int)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(TransporterCoord(AuPoint)),
					SLOT(TransporterCoordUI(AuPoint)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(TransporterLongLat(AuGlobeLLF)),
					SLOT(TransporterLongLatUI(AuGlobeLLF)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CreateEmpire()),
					SLOT(CreateEmpireUI()), Qt::QueuedConnection);
		}
	}
	else if(!g_Au.IsWebCGI())
	{
		// This is a server application.
		if(qApp->connect(this, SIGNAL(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)),
					SLOT(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)),
					(Qt::ConnectionType)(Qt::QueuedConnection|Qt::UniqueConnection)))
		{
			qApp->connect(this, SIGNAL(ArenaReady(AuArenaReady)),
					SLOT(ArenaReady(AuArenaReady)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarAccountActive(AuIdList)), 
					SLOT(AvatarAccountActive(AuIdList)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarBorn(AuId, AuId)), 
					SLOT(AvatarBorn(AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
					SLOT(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(CityStatusChanged(AuId, int, qint8)),
					SLOT(CityStatusChanged(AuId, int, qint8)),
					Qt::QueuedConnection);

			qApp->connect(this,
					SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
							QString, AuId, quint8)),
					SLOT(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
							QString, AuId, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(EnteredDesigner(AuId, int)), 
					SIGNAL(EnteredDesigner(AuId, int)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)), 
					SLOT(GovDossierAddDocument(quint32, quint32, QByteArray)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
					SLOT(GovSetCapitalUp(quint32, AuId, AuSpecies, AuId, int)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(DemiavatarDied(AuId)), 
					SLOT(DemiavatarDied(AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeActivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeActivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeCreatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeCreated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeDeactivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeDeactivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeDestinationSignal(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
							bool, quint8, bool)),
					SLOT(HengeDestinationConfirmed(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
							bool, quint8, bool)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(HengeTeleportSignal(AuId, AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					SLOT(HengeTeleporterEntered(AuId, AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
					SLOT(OfficeAssigned(AuId, quint32, AuOffice)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)),
					SLOT(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(EmpireRecordPatent(quint32, quint16, quint8)),
					SLOT(EmpireRecordPatent(quint32, quint16, quint8)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(ClaimTitle(AuId, QString, AuId, QString, quint32)),
					SLOT(ClaimTitleUp(AuId, QString, AuId, QString, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SectorPirateBaseEncounter(AuId, AuId)),
					SLOT(SectorPirateBaseEncounter(AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
					SLOT(SendFundsUp(AuId, QString, AuId, double, quint8, quint8, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(ShipIt(AuItemTEUContainer)),
					SLOT(ShipIt(AuItemTEUContainer)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SpacecraftNameChanged(AuId, QString)),
					SLOT(SpacecraftNameChanged(AuId, QString)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
					SLOT(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
					SLOT(BountyPosted(AuId, QString, AuId, QString, double, int)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CityCaptured(quint32, quint32, AuId)),
					SLOT(CityCaptured(quint32, quint32, AuId)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(CityDecayed(quint32, AuId)),
					SLOT(CityDecayed(quint32, AuId)));
			qApp->connect(this, SIGNAL(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)),
					SLOT(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)));
			qApp->connect(this, SIGNAL(CreditCityBank(float, AuId)),
					SLOT(CreditCityBank(float, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(GameOver(AuId, quint16, quint8, QByteArray)),
					SIGNAL(GameOver(AuId, quint16, quint8, QByteArray)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(GameTurnPoll(AuId, quint16, AuId)),
					SIGNAL(GameTurn(AuId, quint16, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(OfficerKilled(AuId)),
					SLOT(OfficerKilledUp(AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(AvatarOnLine(AuId, quint32)),
					SLOT(AvatarOnLineUp(AuId, quint32)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(PostJobOffer(AuJobOffer)), SLOT(PostJobOffer(AuJobOffer)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(WarpSignatureDetected(AuId, quint32, quint32)),
					SLOT(WarpSignatureDetected(AuId, quint32, quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SystemWarpSignature(AuId, quint32, quint8, float, float, float)),
					SLOT(SystemWarpSignature(AuId, quint32, quint8, float, float, float)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(ReleaseBerthSignal(AuId, AuId)), SLOT(ReleaseBerthUp(AuId, AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
							quint8, quint8, QString, QString, QString)),
					SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
							quint8, quint8, QString, QString, QString)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(SolarSystemExplored(AuId, quint32)), SLOT(SolarSystemExplored(AuId, quint32)),
					Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemScanned(AuId, quint32, quint8, quint32)),
					SLOT(SolarSystemScanned(AuId, quint32, quint8, quint32)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemFinal(AuId)), SLOT(SolarSystemFinal(AuId)), Qt::QueuedConnection);
			qApp->connect(this, SIGNAL(SolarSystemReadyToDecay(AuId)), SLOT(SolarSystemReadyToDecay(AuId)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(StargateClosed(AuId, AuPoint)),
					SLOT(StargateClosed(AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
					SLOT(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(Spawn(AuId, quint32, AuId, AuId, AuId, quint8)),
					SLOT(SpawnAtom(AuId, quint32, AuId, AuId, AuId, quint8)), Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TradeReadyToShip(AuId, AuTradeAgreement)),
					SLOT(TradeReadyToShip(AuId, AuTradeAgreement)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TradeShipped(AuId, AuTradeAgreement)),
					SLOT(TradeShipped(AuId, AuTradeAgreement)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(Travel(AuId, AuId, AuId, AuId)),
					SLOT(AtomTraveled(AuId, AuId, AuId, AuId)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TravelGalaxyLoc(AuId, AuId, AuId, AuPoint)),
					SLOT(AtomTraveledGalaxyLoc(AuId, AuId, AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(TravelStargate(AuId, AuId, AuId, AuPoint)),
					SLOT(AtomTraveledStargate(AuId, AuId, AuId, AuPoint)),
					Qt::QueuedConnection);

			qApp->connect(this, SIGNAL(WormholeExplored(AuId, AuId, qint8, quint32, QString)),
					SLOT(WormholeExplored(AuId, AuId, qint8, quint32, QString)), Qt::QueuedConnection);
		}
	}
}

const DaPerson* DaAtom::FindPersonOnLine() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaPerson* person = dynamic_cast<const DaPerson*>(*it);
		if(!person)
			continue;
		if(person->IsOnLine())
			return person;
	}
	return 0;
}

DaNpc* DaAtom::FindCharacterRecursive() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindCharacterRecursive();
}

DaNpc* DaAtom::FindCharacterRecursive(const QString& charName, quint32 storyInstance) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindCharacterRecursive(charName, storyInstance);
}

const DaPerson* DaAtom::FindAIPersonRecursive() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindAIPersonRecursive();
}

const DaAsteroid* DaAtom::FindAsteroidRecursive(const AuId& targetId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindAsteroidRecursive(targetId);
}

float DaAtom::AsteroidBeltDensityRecursive(const AuPoint& envLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.AsteroidBeltDensityRecursive(envLoc);
}

void DaAtom::FindAsteroidsRecursive(DaAtomList& asteroids) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindAsteroidsRecursive(asteroids);
}

const DaNpc* DaAtom::FindOfficerRecursive() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindOfficerRecursive();
}

const DaPerson* DaAtom::FindPersonOnLineRecursive() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindPersonOnLineRecursive();
}

void DaAtom::FindPeopleOnLine(DaAtomList& people)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPeopleOnLine(people);
}

void DaAtom::FindPeopleOnLineRecursive(DaAtomList& people) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPeopleOnLineRecursive(people);
}

void DaAtom::FindPeopleAwakeRecursive(DaAtomList& people)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPeopleAwakeRecursive(people);
}

void DaAtom::FindPeopleOnLineRecursiveInStory(DaAtomList& people, quint32 storyInstance)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPeopleOnLineRecursiveInStory(people, storyInstance);
}

DaBd* DaAtom::FindBdHQ() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;
		if(bd->IndustryId() != IID_BaseHeadquarters)
			continue;
		if(bd->IsUnderConstruction())
			continue;
		return (DaBd*)bd;	// cast away constness.
	}
	return 0;
}

void DaAtom::FindBds(DaAtomList& bds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;
		bds.append((DaAtom*)bd);	// cast away constness.
	}
}

void DaAtom::FindBdBase(DaAtomList& bdBase) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;
		if(!bd->IsMilitary())
			continue;
		bdBase.append((DaAtom*)bd);	// cast away constness.
	}
}

void DaAtom::FindBdCity(DaAtomList& bdCity, const AuId& capitolBdId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*it);
		if(!bd)
			continue;
		if(!bd->InCommunityCity(capitolBdId))
			continue;
		bdCity.append((DaAtom*)bd);	// cast away constness.
	}
}

void DaAtom::FindBeaconsRecursive(DaAtomList& bds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindBeaconsRecursive(bds);
}

void DaAtom::FindBdRecursive(DaAtomList& bds) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindBdRecursive(bds);
}

void DaAtom::FindBdRecursive(DaAtomList& bds, quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindBdRecursive(bds, empireId);
}

void DaAtom::FindCitiesBdRecursive(DaAtomList& cities) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCitiesBdRecursive(cities);
}

void DaAtom::FindCitiesBdRecursive(DaAtomList& cities, quint32 empireId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCitiesBdRecursive(cities, empireId);
}

void DaAtom::FindCitiesBdRecursive(DaAtomList& supplyCities, DaAtomList& repairCities, DaAtomList& troopCities, quint32 empireId)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCitiesBdRecursive(supplyCities, repairCities, troopCities, empireId);
}

void DaAtom::FindLivePeopleRecursive(DaAtomList& people)
{
	DaAtomList subs;
	ListSubatoms(subs);
	subs.FindLivePeopleRecursive(people);
}

void DaAtom::FindPeopleRecursive(DaAtomList& people) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPeopleRecursive(people);
}

void DaAtom::FindMoons(DaAtomList& moons) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaWorldLL* moon = dynamic_cast<const DaWorldLL*>(*it);
		if(!moon)
			continue;
		moons.append((DaWorldLL*)moon);
	}
}

void DaAtom::FindPlanetsRecursive(DaAtomList& planets) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPlanetsRecursive(planets);
}

void DaAtom::FindTeleportersRecursive(DaAtomList& teleporters) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindTeleportersRecursive(teleporters);
}

void DaAtom::FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindShipMatesRecursive(shipAssigned, mates);
}

void DaAtom::FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCrewRecursive(shipAssigned, crewMembers);
}

void DaAtom::FindAnimalsRecursive(DaAtomList& animals) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindAnimalsRecursive(animals);
}

void DaAtom::FindItems(DaAtomList& items) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaItem* item = dynamic_cast<DaItem*>(*it);
		if(!item)
			continue;
		if(item->IsStaticDisplay())
			continue;
		items.append(item);
	}
}

void DaAtom::FindItemsStatic(DaAtomList& items) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaItem* item = dynamic_cast<DaItem*>(*it);
		if(!item)
			continue;
		if(!item->IsStaticDisplay())
			continue;
		items.append(item);
	}
}

void DaAtom::FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindItemsRecursive(items, includeStaticDisplayedItems);
}

void DaAtom::FindAIPeopleRecursive(const AuId& shipAssigned, DaAtomList& aipeople) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindAIPeopleRecursive(shipAssigned, aipeople);
}

void DaAtom::FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindOfficersRecursive(shipAssigned, officers);
}

void DaAtom::FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPassengersRecursive(shipAssigned, passengers);
}

void DaAtom::FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindTroopsRecursive(shipAssigned, troops);
}

void DaAtom::FindTroopsRecursive(quint32 empireId, DaAtomList& troops)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindTroopsRecursive(empireId, troops);
}

void DaAtom::FindCapsulesRecursive(DaAtomList& capsules) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCapsulesRecursive(capsules);
}

void DaAtom::FindVesselsRecursive(DaAtomList& spaceships) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindVesselsRecursive(spaceships);
}

void DaAtom::FindPirateStationsRecursive(DaAtomList& stations) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindPirateStationsRecursive(stations);
}

void DaAtom::StartVoiceConversationRecursive(const DaPerson& talker) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.StartVoiceConversationRecursive(talker);
}

void DaAtom::FindCitizensRecursive(DaAtomList& citizens) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindCitizensRecursive(citizens);
}

void DaAtom::FindVehiclesRecursive(DaAtomList& vehicles) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindVehiclesRecursive(vehicles);
}

void DaAtom::FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindVehiclesRecursive(vehicles, motherShipId);
}

// Finds creature atoms that are !IsDead. This means that people who are Normal or Travelling will be found.
DaCreature* DaAtom::FindCreatureRecursive()
{
	DaAtomList subs;
	ListSubatoms(subs);
	return subs.FindCreatureRecursive();
}

DaPerson* DaAtom::FindPersonRecursive() const
{
	DaAtomList subs;
	ListSubatoms(subs);
	return subs.FindPersonRecursive();
}

DaDesigner* DaAtom::FindDesigner(quint16 studioId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaDesigner* de = dynamic_cast<DaDesigner*>(*it);
		if(!de)
			continue;
		if(de->StudioId() == studioId)
			return de;
	}
	return 0;
}

DaDesignPlane* DaAtom::FindDesignPlane(const AuId& owner) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaDesignPlane* plane = dynamic_cast<DaDesignPlane*>(*it);
		if(!plane)
			continue;
		if(owner.IsNull() || plane->Owner() == owner)
			return plane;
	}
	return 0;
}

bool DaAtom::GetTransformRelativeToParent(AuMatrix4x4 &tModel, const DaAtom *pParent) const
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
	tModel = Transform() * tModel;
	return parent->GetTransformRelativeToParent(tModel, pParent);
}

// List atoms that should be in the scene when this atom is the scene atom.
void DaAtom::ListSceneSubatoms(const DaPerson* pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsInScene(pSceneAtom, cameraEnvLoc))
			continue;

		// Add the atom's id to the list.
		sceneAtoms.AddIfUnique(pAtom);

		pAtom->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
	}
}

// NOTE: Atom comparisons must use Ids because two different atom lists are in use when
//       this function is called. See DtDb::SyncClient and how it adds the star field
//       atoms to the scene.
// Returns false if a known component of the starfield is missing. i.e. a sector isn't loaded yet that we need.
bool DaAtom::ListStarFieldSubatoms(const DaAtom* pSceneAtom, DaAtomList& sceneAtoms) const
{
	bool allPresent = true;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaAtom* pAtom = *it;

		// Not all atoms are needed for the starfield. We mostly need only the
		// sectors and solar systems. If the viewer happens to be monitoring a
		// system with a nav console, we also pick up the stars, planets and moons
		// from that system.
		if(!pAtom->InStarField())
			continue;

		// Add the atom's id to the list.
		sceneAtoms.AddIfUniqueId((DaAtom*)pAtom);

		if(!pAtom->ListStarFieldSubatoms(pSceneAtom, sceneAtoms))
			allPresent = false;
	}
	return allPresent;
}

// This is only called on the scene atom. It should search only atoms that are
// visible in the current scene.
// This default handler simply does a non-recursive search of its sub-atoms.
DaAtom *DaAtom::FindClosestAtom(const QPoint &pxFind,
		const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		const AuMatrix4x4 &tModel) const
{
	DaAtom *pClosest = NULL;
	AuPoint fpxFind(pxFind);
	double rClosest = -1.0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;

		AuPoint fLoc = tModel.Project(pAtom->Location());
		AuPoint fpxLocOrtho = tDeviceOrtho.Project(fLoc);
		if(fpxLocOrtho.z >= 0.0)
			continue;	// Atom is behind the camera.

		AuPoint fpxLoc = tDevicePersp.Project(fLoc);
		double rdist = fpxLoc.Distance_2D(fpxFind);
		if(rClosest == -1.0 || rdist < rClosest)
		{
			rClosest = rdist;
			pClosest = pAtom;
		}
	}
	return(pClosest);
}

// Configure a directional (infinite) light source.
// Called before the first scene is rendered that contains this object.
void DaAtom::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	// Recurse.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->CreateDeviceObjects(ari);
}

// Called between rendering scene frames that contains this object.
void DaAtom::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	// Animate audio/visual effects will remove those that have grown too old to play any more.
	if(m_Effects && !m_Effects->m_Effects.isEmpty())
		m_Effects->Animate(ari);

	// Release audio buffers that have finished playing their sound.
	if(m_FirstActiveAudioIdx < ATOM_SOUNDS)
	{
		int cs = m_FirstActiveAudioIdx;
		m_FirstActiveAudioIdx = ATOM_SOUNDS;	// We will refresh the state of this value here.
		for(; cs<ATOM_SOUNDS; cs++)
		{
			if(m_Audio[cs])
			{
				ALenum state;
				alGetSourcei(m_Audio[cs], AL_SOURCE_STATE, &state);
				if(state != AL_PLAYING)
				{
					ari.StopSound(m_Audio[cs]);
					m_Sound[cs] = SOUND_None;
				}
				else if(m_FirstActiveAudioIdx == ATOM_SOUNDS)
					m_FirstActiveAudioIdx = cs;
			}
		}
	}

	// Animate all subatoms.
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->AnimateDeviceObjects(ari, sceneAtom, lastAnim);
}

// Called after last scene is rendered that contains this object.
void DaAtom::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	// Delete all textures associated with 'this' atom.
	// Atoms typically use their ID value to identify textures.
	// Some atoms do not. These are generally cases where all instances of a
	// commonly used atom class object can use the exact same OpenGL display
	// list object. Such objects are usually created once and referenced by all
	// instances of the atom class. They don't get deleted in
	// DeleteDeviceObjects and just hang around until the program exits.
	ari.re->DeleteTextures(Id().GetID());

	// OpenAL audio sources
	for(int cs=0; cs<ATOM_SOUNDS; cs++)
	{
		ari.StopSound(m_Audio[cs]);
		m_Sound[cs] = SOUND_None;
	}
	m_FirstActiveAudioIdx = ATOM_SOUNDS;

	// Delete device objects of all subatoms.
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->DeleteDeviceObjects(ari);
}

void DaAtom::CollectModelMatrices(const AuMatrix4x4& model, AuModelMatrixHash& rmod)
{
	// Calculate my model view matrix.
	rmod.insert(Id(), model);

	// Collect model view matrices of my subatoms recursively.
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		AuMatrix4x4 atomModel = model * (*it)->Transform();
		(*it)->CollectModelMatrices(atomModel, rmod);
	}
}

void DaAtom::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(projection);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	// Do not render effects attached to distant objects.
	if(pass != PASS_Near)
		return;
	// Render effects associated with this atom.
	if(!m_Effects || m_Effects->m_Effects.isEmpty())
		return;

	float gain = RelativeAudioGain(sceneAtom) * ari.fade;

	const AuMatrix4x4& model = rmod.Matrix(Id());
	for(QList<AuEffect>::iterator it = m_Effects->m_Effects.begin(); it != m_Effects->m_Effects.end(); it++)
	{
		// Visual Effects are saved for the last rendering pass.
		if((*it).Visual() != VISUAL_None)
		{
			// Calculate eye distance to effect.
			AuPoint eloc = model.Project((*it).OriginLoc());				// Effect location in eye space.
			renderLast.Insert(eloc.Length2(), this);
		}

		// When displaying the mini me, don't make any sound effects.
		if(IsMini())
			continue;

		// Sound Effect
		if((*it).Sound() != SOUND_None)
		{
			// Sound effects only get played once, when we have a buffer
			// available. If they get too old before getting played, they
			// get deleted in AnimateDeviceObjects. The sound buffers are
			// cleaned up during AnimateDeviceObjects when the sound
			// has finished playing or has become too old to start playing.
			// When the sound has been started, the Sound() call returns
			// SOUND_None so we don't try to play the same sound again.
			for(int cs = 0; cs < ATOM_SOUNDS; cs++)
			{
				if(m_Sound[cs] == SOUND_None || m_Sound[cs] == (*it).Sound())
				{
					// Found an available sound buffer
					//  or
					// found a buffer already playing that sound.
					AuPoint epos;
					if((*it).Sound() == SOUND_KnifeSwish)
					{
						if((*it).SecondsOffset() < 1)
						{
							// Move sound location from start to target.
							epos = (*it).OriginLoc() - (*it).TargetLoc();
							epos *= (*it).SecondsOffset();
							epos += (*it).TargetLoc();
						}
						else
							epos = (*it).OriginLoc();
					}
					else
						epos = (*it).OriginLoc();
					epos = model.Project(epos);

					// Force restart of current sound when it's being
					// replaced by the same sound again, for rapid-fire effect.
					if(m_Sound[cs] == (*it).Sound())
					{
						ari.StopSound(m_Audio[cs]);
						m_Sound[cs] = SOUND_None;
					}

					// Play the sound without looping.
					//qDebug("DaAtom::RenderEnvironmentObject %d sound %d "
							//"pos %g,%g,%g gain %g pitch %g secoff %g",
							//Id().GetID(), (*it).Sound(),
							//epos.x, epos.y, epos.z, (*it).Gain(),
							//(*it).Pitch(), (*it).SecondsOffset());
					ari.PlaySound(m_Audio[cs], m_Sound[cs], (*it).Sound(),
							epos.x, epos.y, epos.z,
							(*it).Gain()*gain, (*it).Pitch(), false,
							(float)(*it).ReferenceDistance(), (*it).RolloffFactor(),
							0);//(*it).SecondsOffset());

					// AnimateDeviceObjects maintains this.
					if(cs < m_FirstActiveAudioIdx)
						m_FirstActiveAudioIdx = cs;

					// Clear sound after playing.
					// This way we don't try to play the same sound twice.
					// We only rapid-fire if we get another sound effect
					// that restarts the sound.
					(*it).ClearSound();
					break;
				}
			}
		}
	}
}

void DaAtom::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	if(!m_Effects || m_Effects->m_Effects.isEmpty())
		return;

	// Render effects associated with this atom.
	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(!IsMini())
	{
		// Get ambient lighting and fog values.
		float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
		bool sunsBlocked;
		SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(),
				sunsBlocked);

		//qDebug() << "DaAtom::RenderLast" << *this << "effects" << m_Effects->m_Effects.size();
		for(QList<AuEffect>::iterator it = m_Effects->m_Effects.begin(); it != m_Effects->m_Effects.end(); it++)
			(*it).Render(projection, model, *ari.re, ambientR,ambientG,ambientB, fogDensity,fogR,fogG,fogB, 1.);
	}
	else
	{
		for(QList<AuEffect>::iterator it = m_Effects->m_Effects.begin(); it != m_Effects->m_Effects.end(); it++)
			(*it).Render(projection, model, *ari.re, 1,1,1, 0,0,0,0, MiniScale());
	}
}

bool DaAtom::RoughExtentsWithShields(AuBox &fExt) const
{
	return RoughExtents(fExt);
}

bool DaAtom::RoughExtents(AuBox &fExt) const
{
	bool bFirst = true;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(bFirst)
		{
			pAtom->RoughExtents(fExt);
			fExt.Transform(pAtom->Transform());
			bFirst = false;
		}
		else
		{
			AuBox subExt;
			pAtom->RoughExtents(subExt);
			subExt.Transform(pAtom->Transform());
			fExt.Union(fExt, subExt);
		}
	}

	if(!bFirst)
		return true;
	return false;
}

// 'this' atom is moving from 'foldloc' to 'fnuloc', relative to the parent
// atom.
// Determine if any peer atoms are in the way and the result of the
// collision.
//	Returns 0 if this atom does not obstruct its movement.
//	Returns 1 if this atom absorbs 'pMove' as a subordinate atom. e.g. person
// enters building
//	Returns 2 if this atom obstructs movement from 'foldloc' to 'fnuloc'.
// This function is called iteratively on peer atoms in an environment when an
// atom 'pMove' is moving
// from one location 'foldloc' to a nu location 'fnuloc'. Locations are
// relative to the parent atom.
// The parent has already applied any restriction to the destination due to
// friction, gravity, etc.
// This function determines if the movement of 'pMove' to 'fnuloc' results in a handoff to a subordinate
// atom. If a handoff is possible, the handoff is performed.
// Examples: a person walks into a building; a vehicle drives into a cargo bay
// of a spaceship; a spaceship enters orbit around a planet.
// This function is expected to do several things. The return result indicates
// what happened.
// 0. Obstruction testing is performed to see if pMove collides with 'this'
// atom. If no collision occurs, 0 is returned.
// 1. If 'pMove' collides with 'this' atom in a way that allows it to be
// absorbed by this atom, the atom is taken as a child of this atom and 1 is
// returned.
// 2. If 'pMove' collides with 'this' atom and movement to 'fnuloc' is
// blocked, 2 is returned.
DaAtom::COLLISION_TYPE DaAtom::CheckCollision(const AuPoint &foldloc, const AuPoint &fnuloc, DaAtom *&pHit,
		AuPoint& hitLoc, AuPoint& hitNormal)
{
	Q_ASSERT(!fnuloc.IsNaN());
	if(IsEthereal())
		return CT_Miss;		// Atoms do not hit anything while ethereal.
	DaAtom* pParent = Parent();
	if(!pParent)
		return CT_Miss;		// No parent means no peers.

	// Get my extents at the proposed location, relative to my parent atom.
	AuBox fMoveExt;
	if(!RoughExtentsWithShields(fMoveExt))
		return CT_Miss;
	AuMatrix4x4 tmove;
	tmove.Translate(fnuloc - foldloc);
	tmove *= Transform();
	fMoveExt.Transform(tmove);

	// Check for collision with parent atom and my peers there.
	return CheckCollisionRelativeToAtom(pParent, foldloc, fnuloc, pHit, hitLoc, hitNormal, fMoveExt);
}

DaAtom::COLLISION_TYPE DaAtom::CheckCollisionRelativeToAtom(DaAtom* pParent, const AuPoint &foldloc, const AuPoint &fnuloc,
		DaAtom*& pHit, AuPoint& hitLoc, AuPoint& hitNormal, const AuBox& fMoveExt)
{
	// Check to see if my movement is blocked by my parent atom.
	if(pParent->IsPathBlocked(foldloc, fnuloc, hitLoc, hitNormal, this))
	{
		hitLoc = Transform().UnProject(hitLoc);	// relative to me
		return CT_HitParent;
	}

	// Get a list of my peer atoms from my parent.
	AuIdList peers;
	DaAtomList subs;
	pParent->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(Is(pAtom->Id()))
			continue;	// Don't collide with myself.
		peers.append(pAtom->Id());
	}

	// When the movement step is large, we check peers for rough collision with the movement segment.
	bool checkPathBlocked = foldloc.Distance2(fnuloc) > FeetPerMeter()*FeetPerMeter();
	AuBox fext;
	for(AuIdList::iterator it = peers.begin(); it != peers.end(); it++)
	{
		DaAtom* pAtom = subs.FindId(*it);
		if(!pAtom)
			continue;
		if(!pAtom->IsNormal())
			continue;

		// Check did atom "enter" a peer, e.g. through a doorway.
		if(pAtom->CheckPeerJoin(foldloc, fnuloc, this))
		{
			pHit = pAtom;
			return CT_JoinedPeer;
		}

		if(!pAtom->IsObstacle(this))
			continue;	// Atom is not an obstacle to movement.

		// Get the extents of the peer atom.
		if(!pAtom->RoughExtentsWithShields(fext))
			continue;
		AuMatrix4x4 t = pAtom->Transform();
		fext.Transform(t);

		// Perform an intersection test on the extents.
		if(fext.DoesIntersect(fMoveExt))
		{
			DaCapsule* capsule = dynamic_cast<DaCapsule*>(pAtom);
			if(capsule)
			{
				// Check did atom "enter" the capsule, e.g. through a doorway.
				// Calculate locations relative to the capsule.
				AuPoint oloc = t.UnProject(foldloc);
				AuPoint nloc = t.UnProject(fnuloc);

				// If the capsule says the path isn't blocked, we must be in a void that is outside the hull but within the overall
				// extents of the hull.
				if(capsule->IsPathBlocked(oloc, nloc, hitLoc, hitNormal, this))
				{
					pHit = pAtom;
					return CT_HitPeer;
				}
			}
			else
			{
				// To avoid getting stuck inside other objects, we always permit movement away from them.
				AuPoint atomLoc = pAtom->Location();	// relative to parent
				if(atomLoc.Distance2(fnuloc) <= atomLoc.Distance2(foldloc))
				{
					// Calculate default hit normal. IsPathBlocked might override it.
					hitNormal = fnuloc - atomLoc;
					hitNormal = t.UnProjectVector(hitNormal);	// Convert normal into space of hit atom.
					hitNormal.Unit();
					hitLoc = t.UnProject(atomLoc);	// relative to hit atom
					pHit = pAtom;
					return CT_HitPeer;
				}
			}
		}
		else if(checkPathBlocked)
		{
			// Extents don't intersect so we will at least check to see if the path is blocked.
			// Calculate locations relative to the atom.
			AuPoint oloc = t.UnProject(foldloc);
			AuPoint nloc = t.UnProject(fnuloc);
			if(pAtom->IsPathBlocked(oloc, nloc, hitLoc, hitNormal, this))
			{
				pHit = pAtom;
				return CT_HitPeer;
			}
		}
	}
	return CT_Miss;
}

// Returns true if movement from foldloc to fnuloc would enter the area of influence of
// this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaAtom::CheckPeerJoin(const AuPoint &, const AuPoint &, const DaAtom*)
{
	return false;
}

// This function looks for nearby atoms.
// This function is only called by the scene atom so certain assumptions are
// made.
// The resulting atom may be my peer. (Control panel in ship environment.)
// The resulting atom may be a subatom of my peer.
// (Door switch on the exterior hull of a spaceship. I'm outside.)
// The resulting atom may be a subatom of a peer of my parent.
// (Door switch in the next hull section of a spaceship.)
// The resulting atom may be a peer of my parent. (Door switch on the
// exterior hull of a spaceship. I'm inside a hull section.)
// Polygon index and a location within the polygon are returned by
// FindControlAtPixel.
DaAtom* DaAtom::FindAtomAtPixel(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		unsigned int &uiPoly, AuPoint &fPolyLoc) const
{
	// Get my parent so it can check my peers and their children.
	DaAtom *pParent = Parent();
	if(!pParent)
	{
		Q_ASSERT(false);
		return 0;		// No parent means no peers.
	}

	// Climb up to my grandparent if my parent is not the environment atom.
	if(!pParent->CanRenderEnvironment())
	{
		DaAtom* pGrandparent = pParent->Parent();
		if(pGrandparent)
			pParent = pGrandparent;
	}

	// Get the reverse transformation back up to the parent.
	AuMatrix4x4 tModel;
	if(pParent->CanRenderEnvironment())
		tModel = EnvTransform();
	else
		GetTransformRelativeToParent(tModel, pParent);
	tModel = tModel.Inverse();

	// Ask the parent to find the closest known atom to the pixel point.
	double rBestZDepth = 1.0;	// Init to 'none found yet' state.
	DaAtom *pBest = pParent->FindAtomAtPixel(pxFind, tDevicePersp, tDeviceOrtho,
			tModel, (DaAtom *)-1, rBestZDepth, uiPoly, fPolyLoc, this);
	return pBest;
}

// Recursive search for an atom that can be selected.
// 'pBest' is the best atom found so far.
// 'rBestZDepth' is the best atom z depth. (Always negative. Least negative is
// closest.)
// Polygon index and a location within the polygon are returned by
// FindAtomAtPixel.
DaAtom *DaAtom::FindAtomAtPixel(const QPoint &pxFind,
		const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		const AuMatrix4x4 &tModel, DaAtom *pBest, double &rBestZDepth,
		unsigned int &uiPoly, AuPoint &fPolyLoc, const DaAtom *pSceneAtom)
{
	// 'pBest' is -1 only the first time this is called, not on recursive calls.
	// This indicates that 'this' atom should not be selected because it must be
	// the environment.
	if (pBest == (DaAtom *)-1)
	{
		pBest = NULL;
	}
	else
	{
		// We do not select the scene atom itself.
		// We do not select atoms that don't want to be selected.
		if(pSceneAtom != this && CanBeSelected())
		{
			// This atom can be selected.
			// Get a list of the selection quads. They're the only polygons that
			// are relevant.
			AuQuadArray aSelSurf;
			EnumSelectionSurfaces(aSelSurf, tModel);

			// If the point is within the bounds of one of my control panels and
			// it's closer in Z, return me. Otherwise, return the previous best.
			AuPoint fpxFind(pxFind);
			const AuQuad *pq = aSelSurf.begin();
			for(int ipanel=0; ipanel<aSelSurf.count(); ipanel++, pq++)
			{
				const AuQuad &q = *pq;

				// The ortho projection Z result tells us if points are in front
				// or behind the camera.
				AuQuad qo(q);
				qo.Transform(tDeviceOrtho);
				if (rBestZDepth < 0.0 && qo.vlr.z < rBestZDepth &&
						qo.vll.z < rBestZDepth && qo.vul.z < rBestZDepth &&
						qo.vur.z < rBestZDepth)
					continue;		// This panel quad is farther away than the
									// best one found so far.

				AuMatrix4x4 tPlane;
				tPlane.Translate(q.vll);
				tPlane.Rotate(AuPoint::CalculateRotation(q.vlr - q.vll,
							q.vul - q.vll));

				AuPoint fLoc;
				tDevicePersp.UnProjectIntoPlane(fLoc, fpxFind, tPlane);
				AuPoint fpxLoc = tDeviceOrtho.Project(fLoc);
				if (fpxLoc.z >= 0.0)
					continue;	// plane intersect point is behind the camera.

				if(rBestZDepth >= 0.0 ||
						(qo.vlr.z < 0 && qo.vlr.z >= rBestZDepth) ||
						(qo.vll.z < 0 && qo.vll.z >= rBestZDepth) ||
						(qo.vul.z < 0 && qo.vul.z >= rBestZDepth))
				{
					// At least one vertex of the triangle is closer than the
					// best location found so far.
					// Check to see if it is inside the bounds of the triangle.
					if (fLoc.InTriangle_3D(q.vlr, q.vll, q.vul))
					{
						// The point is inside the first triangle of the quad.
						if (rBestZDepth >= 0 || fpxLoc.z > rBestZDepth)
						{
							// Calculate fPolyLoc relative to the lower left
							// corner of the panel quad.
							AuPoint vx(q.vlr - q.vll);
							vx.Unit();
							AuPoint vy(q.vul - q.vll);
							vy.Unit();
							AuPoint vpoly = fLoc - q.vll;
							fPolyLoc.x = vpoly.Dot(vx);
							fPolyLoc.y = vpoly.Dot(vy);
							fPolyLoc.z = 0.0;

							uiPoly = ipanel;
							rBestZDepth = fpxLoc.z;
							pBest = this;
						}
					}
				}
				if (rBestZDepth >= 0.0 ||
						(qo.vul.z < 0 && qo.vul.z >= rBestZDepth) ||
						(qo.vur.z < 0 && qo.vur.z >= rBestZDepth) ||
						(qo.vlr.z < 0 && qo.vlr.z >= rBestZDepth))
				{
					// At least one vertex of the triangle is closer than the
					// best location found so far.
					// Check to see if it is inside the bounds of the triangle.
					if (fLoc.InTriangle_3D(q.vul, q.vur, q.vlr))
					{
						// The point is inside the second triangle of the quad.
						if (rBestZDepth >= 0 || fpxLoc.z > rBestZDepth)
						{
							// Calculate fPolyLoc relative to the lower left
							// corner of the panel quad.
							AuPoint vx(q.vlr - q.vll);
							vx.Unit();
							AuPoint vy(q.vul - q.vll);
							vy.Unit();
							AuPoint vpoly = fLoc - q.vll;
							fPolyLoc.x = vpoly.Dot(vx);
							fPolyLoc.y = vpoly.Dot(vy);
							fPolyLoc.z = 0.0;

							uiPoly = ipanel;
							rBestZDepth = fpxLoc.z;
							pBest = this;
						}
					}
				}
			}
		}
	}

	// Check my subatoms.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// This is a recursive search.
		// Each atom eventually passes back up the best so far and its z depth.
		pBest = (*it)->FindAtomAtPixel(pxFind, tDevicePersp, tDeviceOrtho,
				tModel * (*it)->Transform(), pBest, rBestZDepth, uiPoly,
				fPolyLoc, pSceneAtom);
	}
	return pBest;	// The old best is still the best.
}

// This function looks for nearby atoms that can accept control input.
// This function is only called by the scene atom so certain assumptions are
// made.
// The resulting atom may be my peer. (Control panel in ship environment.)
// The resulting atom may be a subatom of my peer. (Door switch on the
// exterior hull of a spaceship. I'm outside.)
// The resulting atom may be a subatom of a peer of my parent. (Door switch
// in the next hull section of a spaceship.)
// The resulting atom may be a peer of my parent. (Door switch on the
// exterior hull of a spaceship. I'm inside a hull section.)
// Polygon index and a location within the polygon are returned by
// FindControlAtPixel.
DaAtom *DaAtom::FindControlAtPixel(const QPoint &pxFind,
		const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		qint16& idPanel, AuPoint &fPolyLoc, double& zDepth) const
{
	// Get my parent so it can check my peers and their children.
	DaAtom *pParent = Parent();
	if(!pParent)
	{
		Q_ASSERT(false);
		return 0;		// No parent means no peers.
	}

	// Climb up to my grandparent if my parent is not the environment atom.
	if(!pParent->CanRenderEnvironment())
	{
		DaAtom* pGrandparent = pParent->Parent();
		if(pGrandparent)
			pParent = pGrandparent;
	}

	// Get the reverse transformation back up to the parent.
	AuMatrix4x4 tModel;
	if(pParent->CanRenderEnvironment())
		tModel = EnvTransform();
	else
		GetTransformRelativeToParent(tModel, pParent);
	tModel = tModel.Inverse();

	// Ask the parent to find the closest known control to the pixel point.
	zDepth = 1.0;	// Init to 'none found yet' state.
	DaAtom *pBest = pParent->FindControlAtPixel(this, pxFind, tDevicePersp,
			tDeviceOrtho, tModel, NULL, zDepth, idPanel, fPolyLoc);
	/*
#if !defined(QT_NO_DEBUG)
	if(pBest)
		qDebug("DaAtom::FindControlAtPixel - Found %s z %g panel %d",
				(const char*)pBest->Name(), rBestZDepth, idPanel);
#endif
*/
	return pBest;
}

// Recursive search for a control.
// 'pBest' is the best atom found so far.
// 'rBestZDepth' is the best atom z depth. (Always negative. Least negative is
// closest.)
// Polygon index and a location within the polygon are returned by
// FindControlAtPixel.
DaAtom *DaAtom::FindControlAtPixel(const DaAtom* sceneAtom,
		const QPoint &pxFind,
		const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
		const AuMatrix4x4 &tModel, DaAtom *pBest, double &rBestZDepth,
		qint16& idPanel, AuPoint &fPolyLoc)
{
	// Check my subatoms.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// This is a recursive search.
		// Each atom eventually passes back up the best so far and its z depth.
		pBest = (*it)->FindControlAtPixel(sceneAtom, pxFind, tDevicePersp, tDeviceOrtho, tModel * (*it)->Transform(), pBest,
				rBestZDepth, idPanel, fPolyLoc);
	}
	
	if((!pBest || rBestZDepth >= 0) && IsControl(sceneAtom))
	{
		//qDebug() << "DaAtom::FindControlAtPixel checking myself" << Id() << Name() << "best" << (pBest?pBest->Name():QString(""))
			//<< "bestZ" << rBestZDepth << "panel" << idPanel;
		// This atom is a control and we have not found a subordinate control close by. We don't check this atom when a subordinate
		// control has been found, otherwise it's difficult to pick up items lying on top of controls, e.g. item lying on top of a
		// shrub.
		// Get a list of the control panel quads. They're the only polygons that are relevant.
		AuQuadArray aControlPanels;
		QVector<qint16> idPanels;
		EnumControlPanels(aControlPanels, tModel, idPanels, sceneAtom);
		Q_ASSERT(aControlPanels.count() == idPanels.count());

		// If the point is within the bounds of one of my control panels and
		// it's closer in Z, return me. Otherwise, return the previous best.
		AuPoint fpxFind(pxFind);
		const AuQuad *pq = aControlPanels.begin();
		for(int ipanel=0; ipanel<aControlPanels.count(); ipanel++, pq++)
		{
			const AuQuad &q = *pq;

			// The ortho projection Z result tells us if points are in front or
			// behind the camera.
			AuQuad qo(q);
			qo.Transform(tDeviceOrtho);
			if(rBestZDepth < 0.0 && qo.vlr.z < rBestZDepth &&
					qo.vll.z < rBestZDepth &&
					qo.vul.z < rBestZDepth && qo.vur.z < rBestZDepth)
				continue;		// This panel quad is farther away than the best
								// one found so far.

			AuMatrix4x4 tPlane;
			tPlane.Translate(q.vll);
			tPlane.Rotate(AuPoint::CalculateRotation(q.vlr - q.vll, q.vul - q.vll));

			AuPoint fLoc;
			tDevicePersp.UnProjectIntoPlane(fLoc, fpxFind, tPlane);
			AuPoint fpxLoc = tDeviceOrtho.Project(fLoc);
			if(fpxLoc.z >= 0.0)
				continue;	// The plane intersect point is behind the camera.
			if(rBestZDepth >= 0.0 ||
					(qo.vlr.z < 0 && qo.vlr.z >= rBestZDepth) ||
					(qo.vll.z < 0 && qo.vll.z >= rBestZDepth) ||
					(qo.vul.z < 0 && qo.vul.z >= rBestZDepth))
			{
				// At least one vertex of the triangle is closer than the best
				// location found so far.
				// Check to see if it is inside the bounds of the triangle.
				if(fLoc.InTriangle_3D(q.vlr, q.vll, q.vul))
				{
					// The point is inside the first triangle of the quad.
					if(rBestZDepth >= 0 || fpxLoc.z > rBestZDepth)
					{
						// Calculate fPolyLoc relative to the lower left corner
						// of the panel quad.
						AuPoint vx(q.vlr - q.vll);
						vx.Unit();
						AuPoint vy(q.vul - q.vll);
						vy.Unit();
						AuPoint vpoly = fLoc - q.vll;
						fPolyLoc.x = vpoly.Dot(vx);
						fPolyLoc.y = vpoly.Dot(vy);
						fPolyLoc.z = 0.0;

						idPanel = idPanels[ipanel];
						rBestZDepth = fpxLoc.z;
						pBest = this;
					}
				}
			}
			if(rBestZDepth >= 0.0 ||
					(qo.vul.z < 0 && qo.vul.z >= rBestZDepth) ||
					(qo.vur.z < 0 && qo.vur.z >= rBestZDepth) ||
					(qo.vlr.z < 0 && qo.vlr.z >= rBestZDepth))
			{
				// At least one vertex of the triangle is closer than the best
				// location found so far.
				// Check to see if it is inside the bounds of the triangle.
				if(fLoc.InTriangle_3D(q.vul, q.vur, q.vlr))
				{
					// The point is inside the second triangle of the quad.
					if(rBestZDepth >= 0 || fpxLoc.z > rBestZDepth)
					{
						// Calculate fPolyLoc relative to the lower left corner
						// of the panel quad.
						AuPoint vx(q.vlr - q.vll);
						vx.Unit();
						AuPoint vy(q.vul - q.vll);
						vy.Unit();
						AuPoint vpoly = fLoc - q.vll;
						fPolyLoc.x = vpoly.Dot(vx);
						fPolyLoc.y = vpoly.Dot(vy);
						fPolyLoc.z = 0.0;

						idPanel = idPanels[ipanel];
						rBestZDepth = fpxLoc.z;
						pBest = this;
					}
				}
			}
		}
	}

	return pBest;	// The old best is still the best.
}

DaWorldLL* DaAtom::FindTerrainLocationAtPixel(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp,
		const AuMatrix4x4 &tDeviceOrtho, AuPoint &fTerrainLoc) const
{
	// Get my parent planet.
	const DaWorldLL* planet = Parent()->FindParentWorldInEnv();
	if(!planet)
		return 0;		// No parent planet.

	// Get the reverse transformation back up to the parent.
	AuMatrix4x4 tModel;
	GetTransformRelativeToParent(tModel, planet);
	tModel = tModel.Inverse();

	// Ask the planet to find the terrain that contains the pixel point.
	if(planet->FindTerrainLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, tModel, fTerrainLoc))
		return (DaWorldLL*)planet;	// Cast away constness.
	else if(planet->FindSeaLevelLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, tModel, fTerrainLoc))
		return (DaWorldLL*)planet;	// Cast away constness.
	return 0;
}

bool DaAtom::Move(const AuPoint& vMove, int elapsedMs)
{
	if(!IsNormal())
		return false;
	DaAtom* parent = Parent();
	if(!parent)
		return false;

	// Determine a vector for movement.
	AuPoint fmove = Transform().ProjectVector(vMove);
#if !defined(QT_NO_DEBUG)
	if(fmove.IsNaN() || vMove.IsNaN())
		qDebug() << "DaAtom::Move" << Id() << Name() << "loc" << Location() << "vMove" << vMove << "fmove" << fmove
			<< "transform" << Transform() << "IsNaN ERROR!";
#endif
	bool moved = parent->MoveSubatom(this, fmove, elapsedMs);
	return moved;
}

bool DaAtom::MoveSubatom(DaAtom *pMove, const AuPoint &vMove, int /*elapsedMs*/)
{
	// Determine the destination point of the move.
	AuPoint foldloc(pMove->Location());
	AuPoint fnuloc = foldloc + vMove;
#if !defined(QT_NO_DEBUG)
	if(fnuloc.IsNaN() || vMove.IsNaN())
	{
		qDebug() << "DaAtom::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			<< "oldloc" << foldloc << "nuloc" << fnuloc << fnuloc.IsNaN() << "vMove" << vMove << vMove.IsNaN() << "IsNaN ERROR!";
	}
#endif

	// Determine the result of attempting to move to the nu location.
	DaAtom *pHit = NULL;
	AuPoint hitLoc, hitNormal;
	COLLISION_TYPE eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitLoc, hitNormal);
	pMove->SetMoveResult(eCollide);
	//qDebug() << "DaAtom::MoveSubatom" << Id() << Name() << "move" << pMove->Id() << pMove->Name() << "vMove" << vMove
		//<< "result" << eCollide;
//doTheSwitch:
	switch (eCollide)
	{
	case CT_HitParent:
		//qDebug("DaAtom::MoveSubatom %d %s, %d %s hit parent %d %s",
				//Id().GetID(), (const char*)Name().toAscii(),
				//pMove->Id().GetID(), (const char*)pMove->Name().toAscii(),
				//pHit->Id().GetID(), (const char*)pHit->Name().toAscii());
		// The moving sub atom hit this atom.
		{
			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			pMove->MovementBlocked(this, hitLoc, hitNormal, false);
			/*
			AuPoint bounceVec = pMove->MovementBlocked(this, hitNormal);

			// Move in the bounce direction if possible.
			// Determine the destination point of the move in the nu direction.
			double movedist = vMove.Length();
			bounceVec = pMove->Transform().ProjectVector(bounceVec);	// Subatom's bounce vector relative to me.
			bounceVec.Unit(movedist);
			fnuloc = foldloc + bounceVec;
			//FilterDestinationLoc(pMove, foldloc, fnuloc);
			eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitNormal);
			if(eCollide != CT_HitParent && eCollide != CT_HitPeer)
				goto doTheSwitch;
				*/
		}
		return false;
	case CT_HitPeer:
	case CT_HitPeerShield:
		//qDebug("DaAtom::MoveSubatom %d %s, %d %s hit peer %d %s",
				//Id().GetID(), (const char*)Name().toAscii(),
				//pMove->Id().GetID(), (const char*)pMove->Name().toAscii(),
				//pHit->Id().GetID(), (const char*)pHit->Name().toAscii());
		{
			// Reflect its movement vector.
			// Calculate surface normal relative to the hit atom's parent, which is this atom.
			hitNormal = pHit->Transform().ProjectVector(hitNormal);

			// Calculate surface normal relative to the moving atom.
			hitNormal = pMove->Transform().UnProjectVector(hitNormal);
			pMove->MovementBlocked(pHit, hitLoc, hitNormal, eCollide == CT_HitPeerShield);
			/*
			AuPoint bounceVec = pMove->MovementBlocked(pHit, hitNormal);

			// Move in the bounce direction if possible.
			// Determine the destination point of the move in the nu direction.
			double movedist = vMove.Length();
			bounceVec = pMove->Transform().ProjectVector(bounceVec);	// Subatom's bounce vector relative to me.
			bounceVec.Unit(movedist);
			fnuloc = foldloc + bounceVec;
			//FilterDestinationLoc(pMove, foldloc, fnuloc);
			eCollide = pMove->CheckCollision(foldloc, fnuloc, pHit, hitNormal);
			if(eCollide != CT_HitParent && eCollide != CT_HitPeer)
				goto doTheSwitch;
				*/
		}
		return false;
	case CT_JoinedPeer:
		// Move the atom subordinate to its nu parent.
		if(pHit)
		{
			//qDebug() << "DaAtom::MoveSubatom" << Id() << Name() << "moving" << pMove->Id() << pMove->Name() << pMove->Location()
				//<< "oldloc" << foldloc << "nuloc" << fnuloc << "hit" << pHit->Id() << pHit->Name();
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
			pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.

			// When a unit moves into a solar system, the system is marked as explored by the empire owning the unit.
			quint32 empireId = pMove->EmpireId();
			if(empireId)
			{
				DaUnit* unit = dynamic_cast<DaUnit*>(pMove);
				if(unit)
				{
					DaSolarSystem* pcss = dynamic_cast<DaSolarSystem*>(pHit);
					if(pcss)
					{
						//DHH How could any object be moving subordinate to a sector and hit a peer solar system?
						qDebug() << "DaAtom::MoveSubatom" << Id() << Name() << "moveAtom" << pMove->Id() << pMove->Name()
							<< "JoinedPeer" << pHit->Id() << pHit->Name() << "Does this ever happen?";
						pcss->ExploredBy(empireId);	// Mark the system explored by the empire owning the unit.
					}
				}
			}
		}
		return true;
	case CT_Miss:
		{
			// The moving atom did not hit anything.
			// Move the atom to the nu location relative to 'this' atom.
			bool atomMoved = false;

			// Watch for spacecraft entering wormholes when they are not subordinate to the primary star.
			DaVessel* ship = dynamic_cast<DaVessel*>(pMove);
			if(ship && ship->IsReadyToWormhole())
			{
				// The ship is ready to wormhole.
				// Check the movement vector against the primary star's wormholes.
				DaAtom* mypar = Parent();
				DaSolarSystem* pss = mypar ? mypar->FindParentSolarSystemInEnv() : 0;
				if(pss)
				{
					quint16 whichFTL = ship->FTLWormholeDrive();

					DaStar* star;
					if(ship->FTLDriveSetting(whichFTL) == 2)
						star = pss->FindPrimaryStar();
					else
						star = pss->FindPrimaryStarNotBlackHole();
					if(star)
					{
						AuMatrix4x4 tstar;
						if(GetTransformRelativeToParent(tstar, star))
						{
							// This atom is subordinate to the "primary" star.
							AuPoint ostarloc = tstar.Project(foldloc);	// Ship's old location relative to the primary star.
							AuPoint nstarloc = tstar.Project(fnuloc);	// Ship's nu location relative to the primary star.
							float entryAngle = -1;
							AuWormhole wh = star->CheckWormholeEntry(ostarloc, nstarloc, entryAngle);
							if(wh.IsValid() && entryAngle >= 0)
							{
								//qDebug("DaAtom::MoveSubatom %d %s spacecraft %d %s entered wormhole.",
										//Id().GetID(), (const char*)Name().toAscii(),
										//ship->Id().GetID(), (const char*)ship->Name().toAscii());
								return star->TraverseWormhole(ship, wh, entryAngle);
							}
						}
					}
				}
			}

			//AuPoint fmove = pMove->Transform().Inverse().ProjectVector(vMove);
			//AuMatrix4x4 t;
			//t.Translate(fmove);
			//pMove->TransformBy(t);
			if(!fnuloc.FuzzyEquals(foldloc))
			{
				AuMatrix4x4 tBody;
				tBody.Translate(fnuloc-foldloc);
				tBody *= pMove->Transform();
				pMove->ChangePos(AuAtomPos(Id(), tBody));
				pMove->InvalidateEnvironmentRecursive();
				atomMoved = true;
			}

			// Check to see if the nu location is outside the scope of
			// influence of this atom.
			if(CheckChildExit(foldloc, fnuloc, pMove))
			{
				// Move the atom to be subordinate to my parent atom.
				// Separate the movement vectors.
				AuAtomPos mpos(pMove->Pos());
				AuPoint nuMove = pMove->Vector() + mpos.Transform().UnProjectVector(Vector());

				// Change the world transformation to be subordinate to the nu parent atom.
				AuMatrix4x4 t = Transform() * mpos.Transform();
				pMove->ForcePosMove(AuAtomPos(ParentId(), t), nuMove);

				// Immediately try moving into the next atom domain.
				if(pMove->IsNormal())
				{
					DaAtom* mypar = Parent();
					if(mypar)
						mypar->MoveSubatom(pMove, AuPoint(0,0,0), 0);
				}
				atomMoved = true;
			}
			else
				pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.
			pMove->SetModified();
			return atomMoved;
		}
		break;
	}
	return false;
}

/*
void DaAtom::SampleControls(DiControlInputArray &aActiveControls, const QPoint &pxHit, bool bContinuous, const DiPanelControl *pControls, unsigned int uiControlCount) const
{
	const DiPanelControl *pc = pControls;
	for (unsigned int ic=0; ic<uiControlCount; ic++, pc++)
	{
		// Skip non-continuous controls when the caller is looking for continuous streaming input.
		if (bContinuous && !pc->m_bContinuous)
			continue;

		if (pc->m_rcControl.contains(pxHit))
		{
			if (pc->m_bWantCoords)
			{
				// This control needs to know the id and the location within the control texture.
				QPoint pxControl;
				pxControl.setX(pxHit.x() - pc->m_rcControl.left());
				pxControl.setY(pc->m_rcControl.bottom() - pxHit.y() - 1);
				DiControlInput ci(pc->m_uiControl, pxControl);
				aActiveControls.append(ci);
			}
			else
			{
				// Only the Id matters when activating this control.
				DiControlInput ci(pc->m_uiControl);
				aActiveControls.append(ci);
			}
		}
	}
}
*/

// This default handler returns true if the atom has non-zero rough extents.
bool DaAtom::CanBeSelected() const
{
	AuBox fExt;
	if(!RoughExtents(fExt))
		return false;
	if(fExt.IsZero())
		return false;
	return true;
}

// Fill the array with quads for determining hit locations when selecting this atom.
// This default handler returns the six surfaces created by the rough extents.
void DaAtom::EnumSelectionSurfaces(AuQuadArray &aSelectionSurfaces, const AuMatrix4x4 &tModel) const
{
	AuBox fExt;
	if(RoughExtents(fExt))
	{
		fExt.Transform(tModel);
		fExt.GetQuads(aSelectionSurfaces);
	}
}

// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
// 'scannerLoc' is the location of the atom who is performing the scan, relative to the solar sytem.
// 'scannerId' is the id of the atom who is performing the scan. Currently either a ship or a city.
// 'scannerEmpire' is the empire id of the atom who is performing the scan.
// 'scannerSensorRange' is the range in feet of the scanner atom's sensors.
// 'scannerWeaponRange' is the range in feet of the scanner atom's bay/base weapons.
// 'scanContent' is a bitmask of data that is sought in the scan
//   b0 = sensor console blips
//   b1 = fire control console blips
//   b2 = transporter console blips
//   b3 = locked-on blip
//   b4 = AI pilot obstacle blips
//   b5 = AI turret gunner target blips
//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
// 'lockedBlip' is the id of the currently locked sensor blip
void DaAtom::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		pAtom->SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
				scannerWeaponRange, scanContent, lockedBlip, stanceCache);
	}
}

void DaAtom::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;
		(*it)->GetRenderLightsRecursive(person, model * (*it)->Transform(), suns, lights);
	}

	m_EffectsLock.lockForRead();
	if(m_Effects)
		m_Effects->GetRenderLights(model, lights);
	m_EffectsLock.unlock();
}

// Orient and OrientationSpeed have no effect unless this function returns true.
bool DaAtom::OkToOrient() const
{
	const DaAtom* parent = Parent();
	if(!parent || dynamic_cast<const DaAnimal*>(parent) || dynamic_cast<const DaVehicle*>(parent))
		return false;
	if(TerrainNormal().IsZero() || GravityNormal().IsZero())
		return false;
	return true;
}

bool DaAtom::Orient(const AuPoint& atLoc, const AuPoint& terNormal)
{
	if(!OkToOrient())
	{
		SetOrientRotation(Rotation());
		return false;	// Rotation was not changed.
	}

	if(TerrainNormal().FuzzyEquals(AuPoint(0,0,1)))
	{
		SetOrientRotation(Rotation());
		return false;
	}

	// Calculate rotation to orient level with the up vector.
	AuAtomPos pos(Pos());
	AuPoint vy = pos.Transform().ProjectVector(AuPoint(0,1,0));
	vy.Unit();
	AuPoint vx;
	if(fabs(vy.Dot(terNormal)) > .9)
	{
		// Orient based on X.
		vx = pos.Transform().ProjectVector(AuPoint(1,0,0));
		vx.Unit();
		vy = terNormal.Cross(vx);
		vy.Unit();
	}
	else
	{
		// Orient based on Y.
		vx = vy.Cross(terNormal);
		vx.Unit();
		vy = terNormal.Cross(vx);
		vy.Unit();
	}

	AuPoint nuRot = AuPoint::CalculateRotation(vx,vy);
	SetOrientRotation(nuRot);

	if(nuRot.FuzzyEquals(Rotation()))
		return false;	// No significant change in rotation.

	if(!IsAtRest())
		return false;

	// Orient atom to the terrain.
	AuMatrix4x4 tOrient;
	tOrient.Translate(atLoc);
	tOrient.Rotate(nuRot);
	pos.SetTransform(tOrient);
	ChangePos(pos);
	return true;
}

void DaAtom::InvalidateEnvironmentRecursive() const
{
	if(IsDead() || IsTrashOrInvalid())
		return;

	InvalidateEnvironment();

	DaAtomList subs;
	ListSubatoms(subs);
	subs.InvalidateEnvironmentRecursive();
}

void DaAtom::InvalidateEnvironment() const
{
	if(!m_Environ)
		return;
	m_EnvironLock.lockForWrite();
	m_EnvironState = 0;		// No longer current.
	m_EnvironLock.unlock();
}

const AuQueryEnvironment* DaAtom::LockEnvironmentQuery() const
{
	Q_ASSERT(DoesQueryEnvironment());
	// Check for current environment.
	m_EnvironLock.lockForRead();
	if(m_Environ && m_EnvironState)
		return m_Environ;	// Environment is 1) current or 2) not current and someone else is rebuilding it.
	m_EnvironLock.unlock();

	// Set flag to indicate that I am doing the work to query the environment.
	m_EnvironLock.lockForWrite();
	if(m_Environ && m_EnvironState)
		return m_Environ;	// Someone beat me to it. Environment is 1) current or 2) not current and someone else is rebuilding it.
	m_EnvironState = 2;		// I am creating the environment query. Others will now use the old data until I am finished.
	m_EnvironLock.unlock();

	// Query the environment with the environment lock unlocked.
	AuQueryEnvironment* qe = CreateEnvironmentQuery();

	// Save the query result. It is the most up-to-date environment query, even if it is now invalid.
	m_EnvironLock.lockForWrite();
	if(m_Environ)
		delete m_Environ;
	m_Environ = qe;

	// Someone could have invalidated the environment data while I was performing the query. If so, let my state remain invalid. 
	if(m_EnvironState == 2)
		m_EnvironState = 1;	// Environment data is now current.
	return m_Environ;
}

void DaAtom::UpdateEnvironment()
{
	AuQueryEnvironment* qe = CreateEnvironmentQuery();
	m_EnvironLock.lockForWrite();
	if(m_Environ)
		delete m_Environ;
	m_Environ = qe;
	m_EnvironLock.unlock();
}

AuQueryEnvironment* DaAtom::CreateEnvironmentQuery() const
{
	if(!DoesQueryEnvironment())
		return 0;
	AuQueryEnvironment* qe = new AuQueryEnvironment;
	DaAtom* pParent = Parent();
	if(!pParent)
		return qe;
	qe->m_Galaxy = pParent->Galaxy();
	AuAtomPos pos(Pos());

	// Creature insertion point is at its eye but gravity calculations really need to be relative to its body pivot point.
	AuPoint cgOffset(-CGOffset());
	double cameraHt = CameraOffset().z;							// Height above pivot point. + above, - below.
	double mouthHt = HeadOffset().z;							// Height above pivot point. + above, - below.

	const DaCapsule* capsule = pParent->FindParentCapsuleInEnv();
	const DaVessel* ves = dynamic_cast<const DaVessel*>(capsule);
	if(capsule)
	{
		// I am subordinate to a capsule at some level.
		const DaBd* bd = dynamic_cast<const DaBd*>(capsule);
		if(bd)
		{
			qe->m_Environment |= EB_InBd;			// DaBd
			qe->m_BdId = bd->Id();
			qe->m_BdCityId = bd->CapitolId();		// null if bd is not part of a city.
			qe->m_CityEmpire = bd->EmpireId();
			qe->m_InIndustry = bd->IndustryId();
			qe->m_InIndustryLL = bd->GlobeLL();
		}
		else if(ves)
			qe->m_Environment |= EB_Spacecraft;		// DaSc

		if(capsule->IsDesignDataCacheValid())
		{
			// Capsule's design cache is valid so it can determine what room I am in.
			if(capsule == (const DaVessel*)pParent)
			{
				// The capsule is my direct parent.
				bool capG;	// True if capsule has interior gravity.
				if(ves)
					capG = capsule->Gravity(0) != 0;
				else
					capG = true;

				double htAbvFloor;
				AuPoint gravityN;
				if(capsule->InsideRoomType(Location(), capG || DoGetRoomUp(), qe->m_CapRoomId, qe->m_CapRoomType, htAbvFloor,
							qe->m_CapRoomName, gravityN))
					qe->m_Environment |= EB_Pathable;			// Unit is in the pathable space of the capsule.

				qe->m_GravityNormal = pos.Transform().UnProjectVector(gravityN);

				/*
				// When a spacecraft thinks it is inside another spacecraft; additional testing is required.
				// Otherwise it is possible to carry one ship "inside" another by having the pivot point at the end of a
				// boom that can be positioned inside a doorway of the mothership.
				if(thisCap && qe->IsInsideSpacecraft())
				{
					// Ikkir suggested we require the center point of ships to be inside the parent ship as well as the pivot point.
					// Let's see how that works here.
					ROOMTYPE roomType;
					QString roomName;
					quint16 roomId = ship->InsideRoomType(Location() + thisCap->DesignCenterOffset(), roomType, roomName);
					if(roomType == ROOM_None)
					{
						qe->m_CapRoomId = 0;
						qe->m_CapRoomType = ROOM_None;	// This clears the "inside" status.
					}
				}
				*/

				// Calculate height to the ceiling for up constraint. Default for AuQueryEnvironment results in no ceiling limit.
				// Distance below the next hard ceiling above.
				qe->m_fCeilingDist = capsule->CeilingHt(Location(), gravityN, qe->IsInsideCapsule());

				// Gravity well does not require gravity drive or it is unsafe.
				// Give it a built-in safety feature that guarantees zero-gravity, even when the ship has no power.
				bool inGravityWell = false;	// Output parameter of FloorHt
				bool onStairs = false;
				AuPoint stairNormal;
				if(capsule->FloorHt(Location(), gravityN, htAbvFloor, inGravityWell, onStairs, stairNormal, qe->IsInsideCapsule(),
							this))
				{
					qe->m_Environment |= EB_Stairs;
					qe->m_terNormal = pos.Transform().UnProjectVector(stairNormal);	// Relative to this atom.
					qe->m_FallDist = htAbvFloor - GroundHeightAboveCG() - 1./12.;	// Dist to capsule floor w/extra 1".
					//if(dynamic_cast<const DaItem*>(this))
						//qDebug() << "DaAtom::CreateEnvironmentQuery" << *this << "htAbvFloor" << htAbvFloor
							//<< "fallDist" << qe->m_FallDist;
				}
				else
					qe->m_terNormal = qe->m_GravityNormal;
				if(inGravityWell)
					qe->m_Environment |= EB_GravityWell;

				// Creatures are immune to falling while they are in reach of a ladder, just like a gravity well.
				if(ves)
				{
					if(CanGrabLadderOnMyShip())
						qe->m_Environment |= EB_InLadderWell;	// Turn the bit on.

					// When the capsule is in atmosphere range of a planet or when its
					// gravity drives are operational, there is interior gravity.
					// It also has interior gravity when aboard another vessel with interior gravity.
					if(capG)
					{
						// Ship has internal gravity so atom can be on the floor.
						qe->m_Environment |= EB_Gravity;
						if(qe->m_FallDist < d_GroundTolerance)
							qe->m_OnGround = OG_Metal;	// Ship deck plates.
					}
				}
				else
				{
					if(CanGrabLadderOnMyBd())
						qe->m_Environment |= EB_InLadderWell;	// Turn the bit on.

					// Bd has internal gravity so atom can be on the floor.
					qe->m_Environment |= EB_Gravity;
					if(qe->m_FallDist < d_GroundTolerance)
						qe->m_OnGround = OG_HardFloor;
				}
			}
			else
			{
				// I am aboard but not directly subordinate. Must be in a vehicle or riding a creature.
				pParent->CapsuleLocation(qe->m_CapRoomId, qe->m_CapRoomType, qe->m_CapRoomName);
			}

			if(qe->IsInsideCapsule())
			{
				// Inside capsule get atmosphere conditions from the capsule.
				qe->m_AtmType = ATMTYPE_Standard;
				qe->m_fAirDensity = capsule->AirPressure(qe->m_CapRoomName);
				if(qe->m_fAirDensity >= .15f && qe->m_fAirDensity <= .85f)
					qe->m_Environment |= EB_Breathable;
			}
			else if(capsule->IsInsideCapsule())
			{
				// Not inside capsule but parent capsule is inside a capsule so get atmosphere conditions from parent capsule.
				qe->m_AtmType = capsule->InAtmosphereType();
				qe->m_fAirDensity = capsule->AirDensity();
				if(capsule->IsAirBreathable())
					qe->m_Environment |= EB_Breathable;
			}
			else if(capsule->IsAtBd())
			{
				// Capsule is on a building site.
				// Get building site location info from capsule.
				qe->m_Environment |= EB_AtBd;
				qe->m_BdId = capsule->BdId();
				qe->m_BdCityId = capsule->InBdCity();
				qe->m_CityEmpire = capsule->InCityEmpire();
				qe->m_InIndustry = capsule->InIndustry();
				qe->m_InIndustryLL = capsule->InIndustryLL();
			}
			else if(capsule->IsAtGeo())
			{
				qe->m_Environment |= EB_AtGeo;
				qe->m_BdId = capsule->BdId();
				qe->m_BdCityId = capsule->InBdCity();
				qe->m_CityEmpire = capsule->InCityEmpire();
				qe->m_InIndustry = capsule->InIndustry();
				qe->m_InIndustryLL = capsule->InIndustryLL();
			}
		}
		else
		{
			// The parent capsule design cache is not valid yet.
			// Set values that will not kill me if I am in a harsh environment.
			qe->m_CapRoomType = ROOM_Corridor;
			if(capsule == (const DaCapsule*)pParent)
			{
				qe->m_GravityNormal = pos.Transform().UnProjectVector(AuPoint(0,0,1));
				qe->m_terNormal = qe->m_GravityNormal;
				qe->m_FallDist = 0;
				qe->m_fCeilingDist = DeckHeight();
				qe->m_Environment |= EB_Gravity;
				qe->m_OnGround = OG_Metal;	// Ship deck plates.
			}

			// Set favorable atmospheric conditions until we know more about the bd.
			qe->m_AtmType = ATMTYPE_Standard;
			qe->m_Environment |= EB_Breathable;
			qe->m_fAirDensity = .5f;
		}
	}

	// Determine the type of atom of our direct parent.
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(pParent);
	if(vehicle)
	{
		qe->m_Environment |= EB_Gravity;		// For stamina effect.
		if(vehicle->IsEnclosed())
		{
			// Inside a vehicle that is enclosed. e.g. SUV, space fighter.
			qe->m_Environment |= EB_InVehicle;

			// Enclosed vehicle has breathable atmosphere.
			qe->m_AtmType = ATMTYPE_Standard;
			qe->m_Environment |= EB_Breathable;
			qe->m_fAirDensity = .5f;
		}
		else
		{
			// On a vehicle that is not enclosed. e.g. dirt bike, sail boat.
			qe->m_Environment |= EB_OnVehicle;
		}
	}

	// Am I inside a design environment instance?
	const DaDesignBase* de = pParent->FindParentDesignBaseInEnv();
	if(de)
	{
		de->GetAtomEnvironment(*qe, this);
		if(!de->IsPreview())
			return qe;
	}

	const DaArena* arena = pParent->FindParentArena();
	if(arena)
	{
		qe->m_Environment |= EB_Arena;

		//DHH For now, arena does not contain planets.

		// People don't suffocate at the sidelines.
		if(InArenaSidelines())
			qe->m_Environment |= EB_Breathable;
		return qe;
	}
	
	// Remainder is only relevant if I am on a planet and not in a design studio.
	const DaWorld* planet = pParent->FindParentPlanetInEnv();
	if(!planet)
	{
		const DaStar* star = pParent->FindParentStarInEnv();
		if(star && (star->IsAnyBlackHole() || star->IsNeutronStar()))
		{
			// I am subordinate to a black hole or neutron star.
			AuPoint bhloc;
			GetLocationRelativeToParent(bhloc, star);
			double bhdist = bhloc.Length();					// distance to the black hole
			if(bhdist <= star->AtmosphereRadius())
			{
				// Inside photon sphere of black hole or neutron sphere of neutron star.
				qe->m_Environment |= EB_PhotonSphere;	// Inside photon or neutron sphere.
				if(star->IsAnyBlackHole() && bhdist <= star->Radius())
					qe->m_Environment |= EB_EventHorizon;
			}
		}

		// Not near a planet. Probably near a star or at least a solar system.
		if(!qe->IsInSpacecraft() && !qe->IsAboardVehicle())
		{
			// Not in a spacecraft or in/on a vehicle.
			// Determine gravity if subordinate to a star or solar system.
			if(pParent == star || dynamic_cast<const DaSolarSystem*>(pParent))
			{
				// When my parent star is a black hole the gravity of the black hole affects me.
				if(star && (star->IsAnyBlackHole() || star->IsNeutronStar()))
				{
					if(qe->m_Environment & EB_PhotonSphere)
						qe->m_Environment |= EB_BlackHoleGravity;	// Black hole gravity has an irresistable effect.
					qe->m_Environment |= EB_Gravity;				// Gravity affects the atom here
				}

				qe->m_fCGAltitude = Location().Length();		// Altitude to eye location is close enough.
				qe->m_GravityNormal = pos.Transform().UnProjectVector(Location());
				// Make gravity normal into a unit vector.
				if(qe->m_fCGAltitude > 0.)
					qe->m_GravityNormal /= qe->m_fCGAltitude;
#if !defined(QT_NO_DEBUG)
				if(Location().IsNaN())
				{
					qDebug() << "DaAtom::CreateEnvironmentQuery" << Id() << Name() << "loc" << Location() << "T" << pos.Transform()
						<< "IsNaN ERROR";
					qe->m_fCGAltitude = 0;
					qe->m_GravityNormal = AuPoint(0,0,1);
				}
#endif
			}
		}
		return qe;
	}

	// I am subordinate to a DaWorld at some level.
	qe->m_WorldAtomType = planet->AtomType();
	qe->m_colAtmosphere = planet->AtmosphereColor();
	qe->m_OrbitZone = planet->OrbitZone();

	// Get my transformation matrix relative to the parent world.
	AuMatrix4x4 tModel;
	GetTransformRelativeToParent(tModel, planet);
	if(std::isnan(tModel[0]))
	{
		// My transformation matrix to the parent world is broken. What's going on?
		qDebug() << "DaAtom::CreateEnvironmentQuery" << Id() << Name() << "at" << planet->Id() << planet->Name() << "IsNaN ERROR";
		// Show transform info about all atoms up to my parent world.
		const DaAtom* parent = this;
		do
		{
			qDebug() << " atom" << parent->Id() << parent->Name() << parent->Transform() << "loc"
				<< parent->Location() << "rot" << parent->Rotation()
				<< "envM" << parent->IsEnvTransformValid() << parent->EnvTransform();
			parent = parent->Parent();
		}
		while(parent && !dynamic_cast<const DaWorld*>(parent));
	}

	// Center of gravity location may be different from eye location so creatures don't lie on the ground at eyeball level.
	AuPoint cgLoc = tModel.Project(cgOffset);
	qe->m_fCGAltitude = planet->InAtmosphereCGAltitude(cgLoc);	// Altitude of pivot point for creatures, not eye loc.
	if(!qe->IsInSpacecraft())
		qe->m_GravityNormal = tModel.UnProjectVector(-planet->GravityDir(cgLoc));
#if !defined(QT_NO_DEBUG)
	if(cgLoc.IsNaN())
	{
		AuPoint myLoc = tModel.Project(AuPoint(0,0));				// My atom location relative to parent planet.
		qDebug() << "DaAtom::CreateEnvironmentQuery" << Id() << Name() << "cgLoc" << cgLoc << "tModel" << tModel 
			<< "cgOffset" << cgOffset << "myLoc" << myLoc << "planet" << planet->Id() << planet->Name()
			<< "T" << pos.Transform() << "IsNaN ERROR";
		qe->m_GravityNormal = AuPoint(0,0,1);
		qe->m_fCGAltitude = 0;
	}
#endif

	// My longitude/latitude is often needed on globes and ringworld sections.
	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
	if(llworld)
		llworld->LLOfLoc(qe->m_GlobeLL, cgLoc);			// Fails if cgLoc is outside the terrain area of the world.

	// We can stop here if the cgLoc is not in atmosphere range of the world.
	if(qe->m_fCGAltitude == -100000)
	{
		// 'cgLoc' is not inside the atmosphere envelope of the planet or ringworld.
		qe->m_fCGAltitude = planet->CGAltitude(cgLoc);	// Altitude of pivot point for creatures, not eye loc.
		return qe;
	}

	// 'cgLoc' is inside the atmosphere envelope of the planet or ringworld, whether or not it has an atmosphere.
	qe->m_Environment |= EB_InAtmosphere;
	qe->m_Environment |= EB_Gravity;			// Gravity affects the atom here
	qe->m_fAltitudeMSL = planet->AltitudeMSL(cgLoc);
	qe->m_fAtmDensity = planet->AtmosphereDensity(qe->m_fAltitudeMSL);
	if(qe->m_fAtmDensity > 0)
		qe->m_Environment |= EB_Atmosphere;

	if(!qe->IsInsideCapsule() && !qe->IsInVehicle())
	{
		// I am exposed to the atmosphere.
		qe->m_fAirDensity = qe->m_fAtmDensity;
		qe->m_AtmType = planet->AtmosphereType();
		if(planet->HasBreathableAtmosphere())
			qe->m_Environment |= EB_Breathable;
	}

	// Query terrain details at this atom's location.
	AuTerrainInfo terQ;
	planet->QueryTerrainAtLoc(terQ, cgLoc);
	// Calculate fall distance to touch the ground. This is to the body pivot point for creatures, not the eye location.
	double fallDist = (qe->m_fCGAltitude - cgOffset.z) - GroundHeightAboveCG() - terQ.m_rSurfaceRadius;
	bool fallToGround;
	if(!capsule)
		fallToGround = true;	// Not in a capsule.
	else if(qe->IsInsideCapsule())
		fallToGround = false;	// I am inside the capsule.
	else if(fallDist < qe->m_FallDist)
		fallToGround = true;	// Ground is closer than any other floor.
	//else if(capsule->OnStairs())
		//fallToGround = false;	// Capsule is under my feet.
	else
		fallToGround = false;
	if(fallToGround)
	{
		// Set floor height based on terrain.
		qe->m_FallDist = fallDist;
		if(qe->m_FallDist < d_GroundTolerance)
		{
			// Unit is touching the ground, possibly under water.
			// Foliage takes precedence for establishing ground contact type, which drives the sound effect and friction.
			if(terQ.m_byFoliage >= 50)
				qe->m_OnGround = OG_Forest;
			else if(terQ.m_byFoliage >= 25)
				qe->m_OnGround = OG_Savanna;
			else
			{
				// No significant foliage.
				if(terQ.m_ElevType == TER_Peak || terQ.m_ElevType == TER_Alpine)
					qe->m_OnGround = OG_Snow;		// No plants in this area.
				else if(terQ.m_ElevType == TER_Highlands)
					qe->m_OnGround = OG_Rock;		// No plants in this area.
				else
					qe->m_OnGround = OG_Sand;
			}
		}
	}

	// Is the unit under water, as determined by the globe model?
	float rEyeAltitude = qe->m_fCGAltitude + cameraHt;
	qe->m_fAltitudeAGL = rEyeAltitude - terQ.m_rSurfaceRadius;	// Altitude of eye location.
	if(terQ.IsWater() && qe->m_fAltitudeAGL >= -.1f)
	{
		// This creature's eye is above the surface of the soil, which may be the sea bottom.
		// Calc sea depth if this unit is in the water.
		// Sea depth is the depth of the eye below the actual water surface.
		qe->m_fSeaDepth = terQ.m_rSeaLevelRadius - rEyeAltitude;
		if(LessThanOrFuzzyEquals(qe->m_fSeaDepth, d_WaterTolerance))
			qe->m_fSeaDepth = 0;						// Eye is above the water sfc.
		else
			qe->m_Environment |= EB_EyeUnderwater;		// Eye is below the water surface.

		// Is my mouth under water, for drowning?
		double mouthDepth = terQ.m_rSeaLevelRadius - (qe->m_fCGAltitude + mouthHt);
		if(mouthDepth > .5)
			qe->m_Environment |= EB_MouthUnderwater;	// Mouth is too far below the water surface.

		// When calculating CG environment, we need a minimum of 2'
		// above the body location. The 2' allows our cg to be below
		// water before our eyes, even in "eyes only" seat positions.
		// This triggers some render logic to obscure the sea below
		// when the eye is above the water surface but the cg is not.
		double cgalt;
		if(cameraHt < 2 && dynamic_cast<const DaCreature*>(this))
			cgalt = rEyeAltitude - 2;
		else
			cgalt = qe->m_fCGAltitude;
		double cgdepth = terQ.m_rSeaLevelRadius - cgalt;
		if(FuzzyGreaterThan(cgdepth, d_WaterTolerance))
			qe->m_Environment |= EB_CGUnderwater;

		// Set a flag if this creature is touching the water, even though it may not be under the water.
		// This is only set true if this unit is not inside an enclosed vehicle or capsule.
		if(!qe->IsInsideCapsule() && !qe->IsInVehicle())
		{
			// Not in a capsule or enclosed vehicle so this unit could be wet.
			if(fabs(qe->m_fCGAltitude - terQ.m_rSeaLevelRadius) < d_GroundTolerance)
				qe->m_Environment |= EB_OnWater;
			float rBaseAlt = qe->m_fCGAltitude - GroundHeightAboveCG();
			if(GreaterThanOrFuzzyEquals(terQ.m_rSeaLevelRadius, rBaseAlt))
			{
				qe->m_Environment |= EB_InWater;	// Some part is physically in the water getting wet.
				qe->m_WaterDepth = terQ.WaterDepth();
			}
		}
	}

	if(!llworld)
		return qe;	// What world is this? D:

	// Check to see if I am on the site of a bd.
	if(!qe->IsInBd() && !qe->IsAtBd() && !qe->IsAtGeo())
	{
		// I am not subordinate to any capsule.
		// I am not already known to be on a site or geo road, which could only be true if I was subordinate to a capsule there.
		AuPoint myLoc = tModel.Project(AuPoint(0,0));	// Base location relative to parent planet.

		// Find all geo constructions at myLoc. Find geo roads that are completed.
		AuGlobeLLF geoll;
		INDUSTRYID geoiid = llworld->FindGeoIndustryContaining(myLoc, geoll);
		if(geoiid != IID_None)
		{
			qe->m_Environment |= EB_AtGeo;
			qe->m_BdId = llworld->Id();
			qe->m_CityEmpire = 0;	// In case we aren't in any jurisdiction.
			qe->m_BdCityId = llworld->JurisdictionAt(myLoc, qe->m_CityEmpire);
			qe->m_InIndustry = geoiid;
			qe->m_InIndustryLL = geoll;
			if(qe->m_OnGround != OG_None)
			{
				// I am on the ground. Geo may override the natural OnGround type, to make pavement sound like pavement, etc.
				ON_GROUND geoGround = llworld->GeoGround(geoll);
				if(geoGround != OG_None)
					qe->m_OnGround = geoGround;		// OG_Rock, OG_Sand, or OG_Pavement
			}
		}

		// Roads take precedence over sites unless the building is a platform.
		ON_GROUND bdGround = qe->m_OnGround;
		int hideGrass;
		FINDDEV filter = FIND_SkipFinishedLandscapers;
		const DaBd* bdSite = llworld->FindBdContaining(myLoc, filter, bdGround, hideGrass);
		if(bdSite && bdSite->EnvironmentQueryValid() && (bdSite->IsPlatform() || geoiid != IID_Road))
		{
			// I am on a building site.
			qe->m_Environment |= EB_AtBd;
			qe->m_BdId = bdSite->Id();
			qe->m_BdCityId = bdSite->CapitolId();		// null if bd is not part of a city.
			qe->m_CityEmpire = bdSite->EmpireId();
			qe->m_InIndustry = bdSite->IndustryId();
			qe->m_InIndustryLL = bdSite->GlobeLL();
			// bdGround was set to OG_None, OG_Pavement, OG_Rock, or OG_Sand by FindBdContaining.
			if(qe->IsOnNaturalGround() && bdGround != OG_None)
			{
				// I am touching natural unpaved ground and the building site suggests a different sound for ground contact.
				// Override certain ground sounds based on presence of grass.
				switch(hideGrass)
				{
				case 0:	// no - grass not hidden
					if(qe->m_OnGround == OG_Rock || qe->m_OnGround == OG_Sand)
						qe->m_OnGround = bdGround;		// Don't override natural foliage sounds.
					break;
				case 1:	// yes - grass hidden
					qe->m_OnGround = bdGround;			// Override natural foliage sounds.
					break;
				case 2:	// mow existing grass
					if(qe->m_OnGround == OG_Forest)
						qe->m_OnGround = OG_Savanna;	// Tone down the forest sound when mowed.
					else if(qe->m_OnGround != OG_Savanna)
						qe->m_OnGround = bdGround;		// Don't override natural grass sound.
					break;
				case 3:	// lawn
					qe->m_OnGround = OG_Savanna;		// Grass is present everywhere on the site.
					break;
				}
			}

			if(!qe->IsInSpacecraft() && !qe->IsAboardVehicle())
			{
				// Not in a capsule or vehicle.
				// Check to see if the building creates a floor below my feet, as with a platform.
				bool inGravityWell = false;	// Output parameter of FloorHt
				bool onStairs = false;
				AuPoint stairNormal;
				double htAbvFloor;
				const DaBd* bdFloor;
				if(!bdSite->PlatformId().IsNull())
				{
					// Building site is on a platform. Get floor from the platform.
					bdFloor = llworld->FindBd(bdSite->PlatformId());
					if(!bdFloor)
						bdFloor = bdSite;	// wut?
				}
				else
					bdFloor = bdSite;
				AuAtomPos bpos(bdFloor->Pos());
				AuPoint bdLoc(bpos.Transform().UnProject(myLoc));		// My location relative to bd.
				AuPoint gravityN(0,0,1);									// Could a building site have any other?
				if(bdFloor->FloorHt(bdLoc, gravityN, htAbvFloor, inGravityWell, onStairs, stairNormal, false, this))
				{
					fallDist = (htAbvFloor - cgOffset.z) - GroundHeightAboveCG() - 1./12.;   // Dist to floor w/extra 1".
					if(fallDist < qe->m_FallDist)
					{
						// The building floor is closer to me than my current floor.
						qe->m_Environment |= EB_Stairs;
						qe->m_terNormal = pos.Transform().UnProjectVector(bpos.Transform().ProjectVector(stairNormal));
						qe->m_FallDist = fallDist;

						// Bd has internal gravity so atom can be on the floor. Bds are not built in weightless environments.
						qe->m_Environment |= EB_Gravity;
						if(qe->m_FallDist < d_GroundTolerance)
							qe->m_OnGround = OG_HardFloor;
					}
					else
						qe->m_terNormal = qe->m_GravityNormal;
				}
				else
					qe->m_terNormal = qe->m_GravityNormal;
				if(inGravityWell)
					qe->m_Environment |= EB_GravityWell;
			}
		}
	}

	if(!qe->IsOnGround() && EnvironmentQueryValid())
	{
		// Check for things below my feet that I can stop against.
		double cgHt = GroundHeightAboveCG();
		AuPoint sightedLoc;
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		AuPoint eyeLoc(0,0);
		AuPoint lookLoc(0,0,-(cgHt+d_GroundTolerance));
		DaAtom* atomBelow = SightTarget(sightedLoc, sightedPart, sightedShield, eyeLoc, lookLoc);
		if(atomBelow && !sightedShield && atomBelow->IsObstacle(this))
		{
			// Don't do this when I am in a capsule and the obstacle below me is the capsule. In that case, my height above
			// the floor has already been accurately calculated. This avoids standing on top of meshes that are not meant to be.
			bool shipBelow = (qe->IsInSpacecraft() || qe->IsInBd()) && dynamic_cast<DaCapsule*>(atomBelow);
			if(!shipBelow)
			{
				// Atom below is an obstacle to my movement downward.
				qe->m_FallDist = -sightedLoc.z - cgHt;
				if(qe->m_FallDist < d_GroundTolerance)
				{
					qe->m_OnGround = OG_Metal;	// The only things not really accounted for already could sound metallic.
					qe->m_terNormal = qe->m_GravityNormal;
				}
			}
		}
	}

	// Check for enclosures.
	if((!qe->IsAirBreathable() || qe->IsEyeUnderWater()) && !qe->IsInsideCapsule() && !qe->IsInVehicle())
	{
		int inEnclosure = llworld->InEnclosureExcept(qe->m_GlobeLL, qe->m_fAltitudeMSL, Id(), qe->m_EnclosureId);
		if(inEnclosure < 0)
		{
			// An enclosure is present on the world that does not have its design analyzed.
			qe->m_AtmType = ATMTYPE_Standard;
			qe->m_fAirDensity = .5f;
			qe->m_Environment |= EB_Breathable;
			qe->m_Environment &= ~(EB_OnWater|EB_InWater|EB_MouthUnderwater);	// Not in/on/under water.
		}
		else if(inEnclosure == 2)
		{
			// Unit is inside an under sea enclosure.
			// Under sea Enclosures do not have surface water inside them.
			qe->m_AtmType = ATMTYPE_Standard;
			qe->m_fAirDensity = .5f;
			qe->m_Environment |= EB_Breathable;
			qe->m_Environment &= ~(EB_OnWater|EB_InWater|EB_MouthUnderwater);	// Not in/on/under water.
		}
		else
		{
			// Not inside an under sea city enclosure.
			if(llworld->InsidePressureTent(cgLoc))
			{
				// Unit is inside a pressure tent.
				// This has precedence because it could be inside land-based building sites and Enclosures and still be in water.
				// However, the pressure tent excludes water.
				qe->m_AtmType = ATMTYPE_Standard;
				qe->m_fAirDensity = .5f;
				qe->m_Environment |= EB_Breathable;
				qe->m_Environment &= ~(EB_OnWater|EB_InWater|EB_MouthUnderwater);	// Not in/on/under water.
			}
			else
			{
				// Not inside a pressure tent.
				if(inEnclosure)
				{
					// Unit is inside a land-based pressurized road, building enclosure, or enclosure, but not on a road or
					// building site. These do not exclude water on their site.
					qe->m_AtmType = ATMTYPE_Standard;
					qe->m_fAirDensity = .5f;
					qe->m_Environment |= EB_Breathable;
				}
			}
		}
	}

	// Inherit inside enclosure bit from my parent.
	if(!qe->IsInsideEnclosure() && !qe->IsInsideBd())
	{
		// I am not inside an enclosure or building. Check my parent.
		if(ves)
		{
			// I am aboard a ship.
			if(ves->IsInsideEnclosure())
				qe->m_EnclosureId = ves->EnclosureId();
		}
		else if(vehicle)
		{
			// I am aboard a vehicle.
			if(vehicle->IsInsideEnclosure())
				qe->m_EnclosureId = vehicle->EnclosureId();
		}
		else
		{
			const DaCreature* mount = dynamic_cast<const DaCreature*>(pParent);
			if(mount && mount->IsInsideEnclosure())
				qe->m_EnclosureId = mount->EnclosureId();
		}
	}

	// Final terrain normal.
	if(!qe->IsInCapsule() && !qe->OnStairs())
	{
		// Terrain normal is straight up in gravity wells, when airborne, and on level floors of buildings.
		if(qe->IsInGravityWell() || qe->IsOnBuildingFloor())
			qe->m_terNormal = qe->m_GravityNormal;						// Get the gravity normal to orient properly.
		else if(!EnvironmentQueryValid() || OkToOrient())
		{
			AuGlobeLL ll;
			ll.FromGlobeLLF(qe->m_GlobeLL);
			qe->m_terNormal = tModel.UnProjectVector(llworld->TerrainNormal(ll));	// Get the real terrain normal.
			qe->m_terNormal.Unit();
		}
		else
			qe->m_terNormal = qe->m_GravityNormal;						// Orient to the gravity normal.
	}
	return qe;
}

void DaAtom::QueryEnvironmentLighting() const
{
	float illum;
	if(CGAltitude() == -100000)
	{
		// not inside the atmosphere envelope of the planet or ringworld.
		illum = -1.f;
	}
	else
	{
		// inside the atmosphere envelope of the planet or ringworld, whether or not it has an atmosphere.
		const DaAtom* parent = Parent();
		const DaWorld* planet = parent ? parent->FindParentPlanetInEnv() : 0;
		if(planet)
		{
			AuPoint planetLoc;
			GetLocationRelativeToParent(planetLoc, planet);
			illum = planet->QuerySunlight(planetLoc);
		}
		else
			illum = -1.f;
	}
	m_EnvironLock.lockForWrite();
	m_Illumination = illum;
	m_EnvironLock.unlock();
}

void DaAtom::QueryEnvironmentVector()
{
	// People calculate their movement vector with respect to the environment for the purpose of visual effects.
	AuPoint vec(Vector());										// Start with my vector.
	if(IsAboardVehicle() || IsInSpacecraft() || IsRider())
	{
		// Include my parent atom's movement vector.
		const DaAtom* parent = Parent();
		if(parent)
		{
			AuAtomPos pos(Pos());
			vec += pos.Transform().UnProjectVector(parent->Vector());	// Calc parent vector relative to me.

			// Recursion is probably overkill here. Let's allow for one more parent level, which might be a moving spacecraft.
			const DaVessel* parship = dynamic_cast<const DaVessel*>(parent->Parent());
			if(parship)
			{
				AuMatrix4x4 t = parent->Transform() * pos.Transform();
				vec += t.UnProjectVector(parship->Vector());		// Calc grandparent vector relative to me.
			}
		}
	}
	m_EnvironLock.lockForWrite();
	m_EnvVector = vec;
	m_EnvironLock.unlock();
}

// Acceleration in ft/sec/sec.
// This base class implementation only calculates acceleration due to gravity.
AuPoint DaAtom::Acceleration() const
{
	// Unaffected by gravity in a gravity well.
	LockEnvironmentQuery();
	if(m_Environ->IsInGravityWell())
	{
		UnlockEnvironmentQuery();
		return AuPoint(0,0);
	}

	// No gravity in design studio unless I am in a place where it effects me.
	if(m_Environ->InDesignEnvironment() && !m_Environ->InDesignPreview() && !m_Environ->IsInGravity())
	{
		UnlockEnvironmentQuery();
		return AuPoint(0,0);
	}

	const DaAtom* parent = Parent();
	if(!parent)
	{
		UnlockEnvironmentQuery();
		return AuPoint(0,0);
	}
	if(m_Environ->IsInSpacecraft() && !m_Environ->IsInsideSpacecraft() && m_Environ->InLadderWell())
	{
		// I am outside a spacecraft at a ladder below a hatch or I am near a ladder.
		if(!parent->IsInGravity())
		{
			UnlockEnvironmentQuery();
			return AuPoint(0,0);				// The ship is not affected by gravity so I am not affected by gravity either.
		}
	}

	bool inEncG;	// true if inside an enclosure with artificial gravity.
	if(m_Environ->WorldAtomType() == ATOM_WorldGasGiant)
		inEncG = m_Environ->IsInsideEnclosure() || DbIndustry::IsPlatform(m_Environ->m_InIndustry);
	else
		inEncG = false;
	double gees;
	if(inEncG)
		gees = GravityStandard();		// Buildings on gas giants need grav couplings due to the massive gravity.
	else
		gees = parent->Gravity(m_Environ->m_fCGAltitude);

	AuPoint accel;
	if(m_Environ->IsInWater())
	{
		// Buoyancy is a factor. Gravity might not be.
		float buoyancy = Buoyancy(*m_Environ);
		if(buoyancy == 0.)
		{
			// Neither ascend nor descend in the water. Gravity is not a factor.
		}
		else if(buoyancy > 0.)
		{
			// Float upward at a rate based on my buoyancy. Gravity is not a factor other than up direction.
			//qDebug() << "float up parentG" << parent->Gravity(m_Environ->m_fCGAltitude) << "buoy" << buoyancy << "comb"
				//<< (parent->Gravity(m_Environ->m_fCGAltitude)/3 * buoyancy);
			accel = m_Environ->GravityNormal() * gees/3 * buoyancy;
		}
		else
		{
			// Sink in the water. Gravity pulls me down. Buoyancy resists.
			accel = m_Environ->GravityNormal() * gees * buoyancy;
		}
	}
	else
		accel = m_Environ->GravityNormal() * -gees;	// Gravity pulls me down.
	UnlockEnvironmentQuery();
	return accel;
}

// Drag in ft/sec/sec.
// From Eric Tolman, http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
// "...the equation for the drag force is:
//
//  D = 1/2 CpAv*v
//
//  Where D is the drag force, C is the drag coefficient for the object
//  (usually from 0.5 to 1.0), p is the density of the fluid, A is the
//  cross-sectional area of the falling object, v is the velocity of the
//  falling object."
AuPoint DaAtom::Drag() const
{
	// Density of the fluid
	float p;
	if(IsInVisibleWater())
		p = .5f;		// Density of water
	else
	{
		if(AirDensity() == 0)
			return AuPoint(0,0);
		p = AirDensity() / 10.f;
	}

	// Velocity squared
	double vxv = Vector().Length2();
	if(vxv == 0)
		return AuPoint(0,0);

	float C = DragCoefficient();
	AuPoint A = CrossSectionalArea();
	double df = .5 * C * p * vxv;
	AuPoint drag = A * df;
#if !defined(QT_NO_DEBUG)
	if((g_Au.m_LogMessages & LOG_Physics) && FindIdRecursive(AuId::Haxus()))
	{
		qDebug("DaAtom::Drag %d %s vec %g,%g,%g vxv %g C %g A %g,%g,%g df %g p %g drag %g,%g,%g",
				Id().GetID(), (const char*)Name().toLatin1(),
				Vector().x, Vector().y, Vector().z, vxv,
				C,
				A.x,A.y,A.z,
				df, p,
				drag.x,drag.y,drag.z);
	}
#endif
	return drag;
}

// Add a visual and/or audio effect to this atom.
void DaAtom::AddEffect(AuEffect& effect)
{
	m_EffectsLock.lockForWrite();
	if(!m_Effects)
		m_Effects = new AuEffectArray;
	m_Effects->m_Effects.append(effect);
	//qDebug() << "DaAtom::AddEffect" << *this << "visual" << effect.Visual() << "sound" << effect.Sound()
		//<< "effects" << m_Effects->m_Effects.size();
	m_Effects->m_Effects.last().SetStartTime();
	m_tmEffectsChange.Set();
	m_EffectsLock.unlock();
	SetModified();
	TickTock();
}

bool DaAtom::HasSoundEffect(int effect)
{
	if(!m_Effects)
		return false;

	m_EffectsLock.lockForRead();
	bool v;
	if(m_Effects)
		v = m_Effects->HasSoundEffect((SOUND_EFFECT)effect);
	else
		v = false;
	m_EffectsLock.unlock();

	return v;
}

bool DaAtom::HasVisualEffect(int effect)
{
	if(!m_Effects)
		return false;

	m_EffectsLock.lockForRead();
	bool v;
	if(m_Effects)
		v = m_Effects->HasVisualEffect((VISUAL_EFFECT)effect);
	else
		v = false;
	m_EffectsLock.unlock();

	return v;
}

DaArena* DaAtom::FindParentArena()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentArena();
	return 0;
}

const DaArena* DaAtom::FindParentArena() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentArena();
	return 0;
}

const DaGalaxy* DaAtom::FindParentGalaxy() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentGalaxy();
	return 0;
}

DaGalaxy* DaAtom::FindParentGalaxy()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentGalaxy();
	return 0;
}

DaSector* DaAtom::FindParentSector()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentSector();
	return 0;
}

const DaSector* DaAtom::FindParentSector() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentSector();
	return 0;
}

const DaSolarSystem* DaAtom::FindParentSolarSystem() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentSolarSystem();
	return 0;
}

DaSolarSystem* DaAtom::FindParentSolarSystem()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentSolarSystem();
	return 0;
}

const DaVessel* DaAtom::FindParentVessel() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentVessel();
	return 0;
}

DaVessel* DaAtom::FindParentVessel()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentVessel();
	return 0;
}

DaUniverse* DaAtom::FindParentUniverse()
{
	DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentUniverse();
	return 0;
}

const DaUniverse* DaAtom::FindParentUniverse() const
{
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentUniverse();
	return 0;
}

DaGalaxy* DaAtom::FindGalaxy(GALAXY galaxy) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaGalaxy* atom = dynamic_cast<DaGalaxy*>(*it);
		if(!atom)
			continue;
		if(atom->Galaxy() != galaxy)
			continue;
		return atom;
	}
	return 0;
}

DaCapsule* DaAtom::FindCapsule() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCapsule* pAtom = dynamic_cast<DaCapsule*>(*it);
		if(!pAtom)
			continue;
		return pAtom;
	}
	return NULL;
}

DaVessel* DaAtom::FindVessel() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVessel* pAtom = dynamic_cast<DaVessel*>(*it);
		if(!pAtom)
			continue;
		return pAtom;
	}
	return NULL;
}

// This is not recursive.
void DaAtom::FindCitizens(DaAtomList& citizens)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCitizen* pAtom = dynamic_cast<DaCitizen*>(*it);
		if(!pAtom)
			continue;
		citizens.append(pAtom);
	}
}

DaStar* DaAtom::FindStar() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* pAtom = dynamic_cast<DaStar*>(*it);
		if(!pAtom)
			continue;
		return pAtom;
	}
	return NULL;
}

DaAtom* DaAtom::FindNormalSubatomId(const AuId& id) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindId(id);
}

DaAtom* DaAtom::FindSubatomId(const AuId& id) const
{
	DaAtomList subs;
	ListSubatoms(subs);
	return subs.FindId(id);
}

const DaAtom* DaAtom::FindParentAtomId(const AuId& parentId) const
{
	if(Is(parentId))
		return this;
	const DaAtom* parent = Parent();
	if(parent)
		return parent->FindParentAtomId(parentId);
	return 0;
}

void DaAtom::ListPlanets(DaAtomList &al) const	// List planets subordinate to this atom. Not recursive.
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorld* pAtom = dynamic_cast<DaWorld*>(*it);
		if(!pAtom)
			continue;
		al.append(pAtom);
	}
}

void DaAtom::ListStars(DaAtomList &al) const	// List stars subordinate to this atom. Not recursive.
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaStar* pAtom = dynamic_cast<DaStar*>(*it);
		if(!pAtom)
			continue;
		al.append(pAtom);
	}
}

// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaAtom::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(Vector().IsFuzzyZero())
		return AuPoint(0,0);
	if(hitNormal.IsFuzzyZero())
	{
		SetVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	AuPoint hitN = hitNormal;
	hitN.Unit();

	// Calculate unit vector in my direction of movement.
	AuPoint bounceVec(Vector());		// My undampened reflected movement vector.
	double speed = bounceVec.Length();
	AuPoint vec = bounceVec / speed;

	// Determine reflection axis.
	AuPoint axis = vec.Cross(hitN);
	axis.Unit();

	// Determine angle of incidence.
	double ai = vec.AngleTo(hitN);

	// Calculate angle of reflection.
	double af = ai * 2 - PI;
	AuMatrix4x4 t;
	t.RotateAxis(af, axis);

	// The only time hit is a world is when we have hit a building.
	QString hitName = hit->Name();

	// Get transformation matrices to our common parent atom.
	DaAtom* parent = CommonParent(hit, 0);
	if(!parent)
	{
		qDebug() << "DaAtom::MovementBlocked" << *this << "hit" << *hit << "no common parent";
		return AuPoint(0,0);
	}
	AuMatrix4x4 myM;
	GetTransformRelativeToParent(myM, parent);
	AuMatrix4x4 hitM;
	hit->GetTransformRelativeToParent(hitM, parent);

	// Calculate movement vectors relative to our common parent atom.
	// 'hit' atom vector only applies when I'm not subordinate to it.
	AuPoint myVec = myM.ProjectVector(Vector());
	AuPoint hitVec;
	if(Parent() != hit)
		hitVec = hitM.ProjectVector(hit->Vector());

	// Calculate momentum result.
	AuPoint myMoment = myVec * Mass();
	AuPoint hitMoment = hitVec * hit->Mass();
	AuPoint totalMoment = myMoment + hitMoment;

	// Calculate movement vector for both atoms resulting from the collision, relative to our common parent.
	AuPoint resultVector = totalMoment / (Mass() + hit->Mass());

	// Calculate change in direction.
	AuPoint myChange = resultVector - myVec;
	AuPoint hitChange = resultVector - hitVec;

	// Bounce the vector off the hit atom.
	myVec = myM.UnProjectVector(resultVector);
	bounceVec = t.Project(myVec);
	SetVector(bounceVec);
	SetModified();

	// No collision damage when speeds are less than 10 meters per second.
	if(speed < 15*FeetPerMeter() && hitVec.Length() < 15*FeetPerMeter())
		return bounceVec;

	AuPoint myHitLoc = hitM.Project(hitLoc);					// Hit loc rel to common parent.
	myHitLoc = myM.UnProject(myHitLoc);							// Hit loc rel to this atom.

	//AuPoint hitFromLoc = myM.Project(AuPoint(0,0));				// My loc rel to common parent.
	//hitFromLoc = hitM.UnProject(hitFromLoc);					// My loc rel to the hit atom.
	AuPoint hitFromLoc(hitLoc);									// Hit loc rel to hit atom.

	AuPoint hitResultVec = hitM.UnProjectVector(resultVector);	// Result vector rel to the hit atom.

	// On the scene client, the only atoms that get moved are the scene atom and the atom it is directly controlling, such as a
	// vehicle or spacecraft.
	if(g_Au.IsSceneClient())
	{
		// Notify the server.
		emit CollisionOnClient(Id(), myChange, myHitLoc, hit->Id(), hitChange, hitFromLoc, hitResultVec, hitShield);
	}
	else
	{
		// Apply damage resulting from the collision.
		Collide(myChange, myHitLoc, hit, hitChange, hitFromLoc, hitResultVec, hitShield);
	}
	return bounceVec;
}

bool DaAtom::WeaponOnCollision(AuCombatAttacker& attacker, AuCombatWeapon& weapon, int hitShield, double kineticEnergy) const
{
	// Apply damage due to absorption of kinetic energy required to make the change in movement vector.
	// Calculate kinetic energy required to make the change.
	// K = 1/2 * m * v^2
	//   K is kinetic energy
	//   m is mass
	//   v is velocity
	const double khp = 50000;				// Kinetic energy absorbed per hit point of damage. Bigger = less damage, linear.
	if(kineticEnergy < khp)
		return false;

	double hitsDamage = kineticEnergy/khp;	// This could be a really big number.
	quint32 collisionHits;
	if(hitsDamage > 0x3fffffff)
		collisionHits = 0x3fffffff;
	else
		collisionHits = (quint32)hitsDamage;

	QString reason;
	if(hitShield == 1)
		reason = tr("collision with shield of %1").arg(Name());
	else if(hitShield == 2)
		reason = tr("shield collision by %1").arg(Name());
	else
		reason = tr("collision with %1").arg(Name());
	attacker = AuCombatAttacker(reason);

	weapon = AuCombatWeapon(WM_Contact);
	weapon.m_CID = CID_Collision;
	weapon.m_w1.m_Effect = WE_Bludgeon;
	weapon.m_w1.m_Damage = collisionHits/2;
	weapon.m_w1.m_DamageMod = collisionHits - weapon.m_w1.m_Damage;
	return true;
}

void DaAtom::Collide(const AuPoint& myChange, const AuPoint& myHitLoc, DaAtom* hit, const AuPoint& hitChange,
		const AuPoint& hitFromLoc, const AuPoint& hitResultVec, bool hitShield)
{
	if(!IsNoncombatant())
	{
		// Apply damage due to absorption of kinetic energy required to make the change in movement vector.
		// Calculate kinetic energy required to make the change.
		// K = 1/2 * m * v^2
		//   K is kinetic energy
		//   m is mass
		//   v is velocity
		double myK = (Mass() * myChange.Length2())/2.;
		//qDebug() << "DaAtom::Collide" << Id() << Name() << "mass" << Mass() << "vel" << myChange.Length()
			//<< "myK" << myK << "hitsDamage" << (double)(myK/khp);
		AuCombatAttacker attacker;
		AuCombatWeapon weapon(WM_Contact);
		if(hit->WeaponOnCollision(attacker, weapon, hitShield ? 1 : 0, myK))
		{
			// Damage this atom due to the collision with 'hit'.
			AuCombatDefender defendCrash(this, hitFromLoc, false, 0);
			AuCombatResult result;
			hit->AttackTarget(attacker, defendCrash, weapon, true, result);
		}
	}

	if(!hit->IsNoncombatant())
	{
		// Apply damage due to absorption of kinetic energy required to make the change in movement vector.
		// Calculate kinetic energy required to make the change.
		// K = 1/2 * m * v^2
		//   K is kinetic energy
		//   m is mass
		//   v is velocity
		double hitK = (hit->Mass() * hitChange.Length2())/2.;
		//qDebug() << "DaAtom::Collide hit" << hit->Id() << hit->Name() << "mass" << hit->Mass()
			//<< "vel" << hitChange.Length() << "hitK" << hitK << "hitsDamage" << (double)(hitK/khp);
		AuCombatAttacker attacker;
		AuCombatWeapon weapon(WM_Contact);
		if(WeaponOnCollision(attacker, weapon, hitShield ? 2 : 0, hitK))
		{
			AuCombatDefender defendCrash(hit, myHitLoc, hitShield, 0);
			AuCombatResult result;
			AttackTarget(attacker, defendCrash, weapon, true, result);
			if(result.m_IsDestroyed)
				return;
		}

		// Induce motion in the atom that was hit, unless its shield was hit.
		// Only certain atoms can move.
		if(!hitShield && hit->CanSetInMotion())
		{
			hit->SetVector(hitResultVec);
			hit->SyncController(AuSyncVector(hitResultVec));
			hit->SetInMotion();
			hit->SetModified();
			hit->TickTock();
		}
	}
}

const DaTeleporter* DaAtom::FindClosestOriginTeleporter(const AuPoint& nearLoc, AuPoint& telLoc, double& telDistance) const
{
	telDistance = 0;
	const DaTeleporter* closestTel = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaTeleporter* tel = dynamic_cast<const DaTeleporter*>(*it);
		if(!tel)
			continue;
		if(!tel->IsOrigin())
			continue;

		// Measure distance to each of the origin teleporter's portal locations.
		const AuPointArray& tlocs = tel->LockTeleporterLocs();
		for(AuPointArray::const_iterator ti = tlocs.cbegin(); ti != tlocs.cend(); ti++)
		{
			AuPoint tloc = tel->Transform().Project(*ti);	// Project portal location into my coordinate system.
			double dist = nearLoc.Distance2(tloc);			// Compare squared distances for speed.
			if(!closestTel || dist < telDistance)
			{
				closestTel = tel;
				telDistance = dist;
				telLoc = tloc;
			}
		}
		tel->UnlockTeleporterLocs();
	}

	if(closestTel)
		telDistance = sqrt(telDistance);					// Calculate distance from square of distance.
	return closestTel;
}

//static
QString DaAtom::FormatDamageReceived(int criticalHits, WEAPONEFFECT effect1, int damage1, int armor1,
		WEAPONEFFECT effect2, int damage2, int armor2, const QString& attackerName)
{
	// Hit by who.
	QString text;
	if(!attackerName.isEmpty())
	{
		if(criticalHits == 1)
			text = tr("Critical Hit by %1!").arg(attackerName);
		else if(criticalHits == 2)
			text = tr("Double Critical Hit by %1!").arg(attackerName);
		else
			text = tr("Hit by %1!").arg(attackerName);
	}
	else
	{
		if(criticalHits == 1)
			text = tr("Critical Hit!");
		else if(criticalHits == 2)
			text = tr("Double Critical Hit!");
		else
			text = tr("Hit!");
	}

	// Damage inflicted.
	if(damage1 && damage2)
		text += tr(" Received %1 hits of %2 damage and %3 hits of %4 damage.")
			.arg(damage1)
			.arg(EffectName(effect1))
			.arg(damage2)
			.arg(EffectName(effect2));
	else if(damage1)
		text += tr(" Received %1 hits of %2 damage.")
			.arg(damage1)
			.arg(EffectName(effect1));
	else if(damage2)
		text += tr(" Received %1 hits of %2 damage.")
			.arg(damage2)
			.arg(EffectName(effect2));

	// Damage absorbed.
	if(armor1 && armor2)
		text += tr(" %1 hits of %2 damage and %3 hits of %4 damage absorbed.")
			.arg(armor1)
			.arg(EffectName(effect1))
			.arg(armor2)
			.arg(EffectName(effect2));
	else if(armor1)
		text += tr(" %1 hits of %2 damage absorbed.")
			.arg(armor1)
			.arg(EffectName(effect1));
	else if(armor2)
		text += tr(" %1 hits of %2 damage absorbed.")
			.arg(armor2)
			.arg(EffectName(effect2));

	return text;
}

void DaAtom::CombatApplyDamage(const AuCombatAttacker& /*attacker*/, const AuCombatWeapon& /*weapon*/, const AuPoint& /*wpnLoc*/,
		const AuPoint& /*hitLoc*/, AuCombatResult& /*result*/)
{
	/*
	if(!whoId.IsNull() && (damage1 || damage2 || kineticArmor1 || energyArmor1 || kineticArmor2 || energyArmor2))
	{
		// Send a log message. If 'whoId' is a person online, they will receive it. Other atoms will ignore it.
		AuCommDamageInflicted
		QString text = FormatDamageInflicted(criticalHits, effect1, damage1, kineticArmor1, energyArmor1,
				effect2, damage2, kineticArmor2, energyArmor2, Name(), false);
		AuCommMessage msg(whoId, whoName, FindParentSector()->Id(), text);		// Log message.
		g_Au.SendAtomChat(msg);
	}
	*/
}

void DaAtom::FilterFinalMoveVector(AuPoint& vector) const
{
	Q_UNUSED(vector);
	// When sitting on the ground, eliminate the downward portion of the movement vector.
	//if(IsInGravity() && IsTouchingGround() && vector.z < 0 && !CanBeUnderGround())
		//vector.z = 0;
	//else if(IsInWater() && Density() < DensityOfWater() && vector.z < 0)
		//vector.z = 0;
}

void DaAtom::FilterPostMoveVector(AuPoint& vector) const
{
	if(!IsInGravity())
		return;
	if(vector.z >= 0)
		return;
	// When sitting on the ground after the move, eliminate the downward portion of the movement vector.
	if(IsOnGround() && !CanBeUnderGround())
		vector.z = 0;
	else if(MoveResultBlocked())
		vector.z = 0;	// Eliminate gravity vector when blocked to help get free from the side of buildings.
}

// Returns Name() with special characters substituted, e.g. '&' becomes '&amp;'.
//static
QString DaAtom::HtmlName(const QString& text)
{
	// Array of characters that are forbidden in html text.
	const char badChars[] =
	{
		'&',
		'"',
		'<',
		'>',
	};
	int badCharCount = sizeof(badChars)/sizeof(const char);

	// Array of encodings that correspond to the forbidden characters.
	const char* encChars[] =
	{
		"&amp;",
		"&quot;",
		"&lt;",
		"&gt;",
	};
	Q_ASSERT(sizeof(encChars)/sizeof(const char*) == badCharCount);	// Both arrays must have the same number of entries in each.

	// Replace the forbidden characters with their corresponding html encodings.
	QString name = text;
	for(int reChar=0; reChar<badCharCount; reChar++)
		name.replace(badChars[reChar], encChars[reChar]);
	return name;
}

// Returns the nearest common parent in the parenting chain between this atom and 'otherAtom'.
// If a common parent atom is already known, it can be passed in 'knownCommonParent'.
// If 'knownCommonParent' is 0, the solar system is tried.
DaAtom* DaAtom::CommonParent(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const
{
	Q_ASSERT(otherAtom);
	// We need to know a common parent to get started.
	if(!knownCommonParent)
	{
		// Try the solar system. We'll work our way down from there.
		knownCommonParent = FindEnvironmentAtom();
		if(knownCommonParent == otherAtom)
			return (DaAtom*)knownCommonParent;
		if(!knownCommonParent || knownCommonParent != otherAtom->FindEnvironmentAtom())
			return 0;	// I'm not in a solar system or we're not in the same solar system together.
	}

	// Get the atom path from the known common parent to this atom.
	DaAtomList mypath;
	if(!ListAtomsFromParent(mypath, knownCommonParent))
		return 0;	// I have no path to the known common parent?

	// Get the atom path from the known common parent to the other atom.
	DaAtomList otherpath;
	if(!otherAtom->ListAtomsFromParent(otherpath, knownCommonParent))
		return 0;	// Other atom has no path to the known common parent?

	// Starting from the bottom of my path, look for an atom that is also in the other atom's path.
	while(!mypath.isEmpty())
	{
		// Work our way up from the end of my path, which is the closest atom to me.
		DaAtom* lastAtom = mypath.takeLast();
		if(otherpath.contains(lastAtom))
			return lastAtom;
	}

	// We didn't find a better common parent atom than the known one.
	return (DaAtom*)knownCommonParent;
}

double DaAtom::DistanceToAtom(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const
{
	const DaAtom* common = CommonParent(otherAtom, knownCommonParent);
	if(!common)
	{
		qWarning() << "error: DaAtom::DistanceToAtom" << *this << "no common parent with" << *otherAtom;
		return FeetPerAu();
	}

	AuPoint myloc;
	if(!GetLocationRelativeToParent(myloc, common))
	{
		qWarning() << "error: DaAtom::DistanceToAtom atom" << *this << "no longer sub to" << *common;
		return FeetPerAu();
	}

	AuPoint oloc;
	if(!otherAtom->GetLocationRelativeToParent(oloc, common))
	{
		qWarning() << "error: DaAtom::DistanceToAtom other atom" << *otherAtom << "no longer sub to" << *common;
		return FeetPerAu();
	}

	return myloc.Distance(oloc);
}

double DaAtom::DistanceToAtom2(const DaAtom* otherAtom, const DaAtom* knownCommonParent) const
{
	const DaAtom* common = CommonParent(otherAtom, knownCommonParent);
	if(!common)
	{
		//qWarning() << "error: DaAtom::DistanceToAtom2" << *this << "no common parent with" << *otherAtom;
		return FeetPerAu()*FeetPerAu();
	}

	AuPoint myloc;
	if(!GetLocationRelativeToParent(myloc, common))
	{
		qWarning() << "error: DaAtom::DistanceToAtom2 atom" << *this << "no longer sub to" << *common;
		return FeetPerAu()*FeetPerAu();
	}

	AuPoint oloc;
	if(!otherAtom->GetLocationRelativeToParent(oloc, common))
	{
		qWarning() << "error: DaAtom::DistanceToAtom2 other atom" << *otherAtom << "no longer sub to" << *common;
		return FeetPerAu()*FeetPerAu();
	}

	return myloc.Distance2(oloc);
}

void DaAtom::operator>>(QDataStream& out) const
{
	out << m_AnyScene;
	out << m_AtRest;
	out << m_Id;
	out << (quint8)m_MoveResult;
	out << (quint8)m_State;
	out << m_Ticked;
	out << m_tmCreated;

	m_ControlLock.lockForRead();
	out << m_Controller;
	AuTime tmControlled;
	out << tmControlled;
	m_ControlLock.unlock();

	m_DeathLock.lockForRead();
	out << m_tmDeath;
	m_DeathLock.unlock();

	m_ModifyLock.lockForRead();
	out << m_tmModify;
	m_ModifyLock.unlock();

	m_PosLock.lockForRead();
	out << m_OrientRotation;
	out << m_Pos;
	out << AuTime();	//m_PosChange;
	out << m_Vector;
	m_PosLock.unlock();

	m_TickLock.lockForRead();
	AuTime tmTicked;
	out << tmTicked;
	m_TickLock.unlock();

	m_TockLock.lockForRead();
	out << m_tmTocked;
	m_TockLock.unlock();
}

void DaAtom::operator<<(QDataStream& in)
{
	in >> m_AnyScene;
	in >> m_AtRest;
	in >> m_Id;
	quint8 ui8;
	in >> ui8;
	m_MoveResult = (COLLISION_TYPE)ui8;
	in >> ui8;
	m_State = (ATOM_STATE)ui8;
	in >> m_Ticked;
	in >> m_tmCreated;

	m_ControlLock.lockForWrite();
	in >> m_Controller;
	AuTime tmControlled;
	in >> tmControlled;
	m_ControlLock.unlock();

	m_DeathLock.lockForWrite();
	in >> m_tmDeath;
	m_DeathLock.unlock();

	m_ModifyLock.lockForWrite();
	in >> m_tmModify;
	m_ModifyLock.unlock();

	m_PosLock.lockForWrite();
	in >> m_OrientRotation;
	in >> m_Pos;
	AuTime posChange;
	in >> posChange;
	in >> m_Vector;
	m_PosLock.unlock();

	m_TickLock.lockForWrite();
	AuTime tmTicked;
	in >> tmTicked;
	m_TickLock.unlock();

	m_TockLock.lockForWrite();
	in >> m_tmTocked;
	m_TockLock.unlock();
}

// Called by atoms who want to be affected by solar radiation damage when too close to a sun.
// Only call this if the parent atom is a DaStar.
void DaAtom::ApplySolarRadiation(const DaStar* parentStar, float dmgPerSecond)
{
	if(parentStar != Parent())
		return;
	const DaAtom* parentParentStar = parentStar->Parent();
	if(!parentParentStar)
		return;

	float sunRadius = parentStar->Radius();
	float sunRad2 = sunRadius * sunRadius;		// Squared for faster distance checking

	float distance = Location().Length2();		// This atom's squared distance from center of the parent sun
	if(distance < sunRad2)
	{
		// Determine hits of damage in one second at this range.
		// Using squared distances gives an exponential increase in damage as distance decreases.
		float stopDist = (sunRadius - AtmosphereTopMSL());
		stopDist *= stopDist;			// Squared distance to the altitude where things cannot move.

		float dmgRatio = 1.f - (distance - stopDist) / (sunRad2 - stopDist);
		float hitsPerSecond;
		if(InBlackHoleGravity() || Speed() < 10000)
			hitsPerSecond = dmgRatio * 100000;			// Slow death in black hole or sun when moving slowly.
		else
			hitsPerSecond = dmgRatio * dmgPerSecond;
		int hits = (int)ceil(hitsPerSecond);
		int fireHits = g_Au.Dice().Roll(2,hits);
		int radHits = g_Au.Dice().Roll(2,hits);
		const DaSolarSystem* system = parentParentStar->FindParentSolarSystemInEnv();

		AuCombatAttacker attacker;
		if(system)
			attacker.m_Name = parentStar->Description() + " in " + system->Name();
		else
			attacker.m_Name = parentStar->Description();

		AuCombatResult result;
		result.m_Damage1 = fireHits;
		result.m_Damage2 = radHits;

		AuCombatWeapon weapon(WM_Internal);
		if(parentStar->IsAnyBlackHole())
			weapon.m_CID = CID_Spaghettification;
		else
			weapon.m_CID = CID_SunBurn;
		weapon.m_w1.m_Effect = WE_Slash;
		weapon.m_w2.m_Effect = WE_Sonic;

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
	}
}

// Tests for presense of another identical meat pile that is too close to loc.
bool DaAtom::OkToDropMeat(const AuPoint& loc, COMMODITYID cidMeat, const AuDNABase* dna) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaItem* item = dynamic_cast<const DaItem*>(*it);
		if(!item)
			continue;
		
		// Found an item.
		if(item->CommodityId() != cidMeat)
			continue;	// Not the same kind of meat.
		if(!item->IsDNA(dna))
			continue;	// Not the same DNA.
		
		// Found an identical item. How close is it to loc?
		// Compare squared distances to avoid sqrt.
		if(item->Location().Distance2(loc) < 7*7)
			return false;
	}
	return true;	// Didn't find an identical piece of meat within 5'.
}

// Called by stations and cities.
// Searches recursively for any trespassing cities or stations.
// 'trespassPolicy' is from the empire record: 0=ok (I'm not called with this), 1=not on planets with cities, 2=not at all
const DaAtom* DaAtom::FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return subs.FindTrespasserRecursive(empireId, systemLoc, sensorRange);
}

POLITICAL_STANCE DaAtom::PoliticalStanceMakeContact(quint32 myEmpireId, const DaAtom* other, quint32 otherEmpireId) const
{
	if(!myEmpireId)
	{
		// 'of' unit has no empire.
		if(otherEmpireId)
			return POL_Enemy;	// Non empire units view empire units as enemy.
		else
			return POL_Neutral;	// Non empire units view non empire units as neutral.
	}
	else
	{
		// 'of' unit has an empire.
		if(!otherEmpireId)
			return POL_Enemy;	// Empire units view non empire units as enemy.
		else if(myEmpireId == otherEmpireId)
			return POL_Empire;	// Empire units view fellow empire units as comrades.
	}

	if(InArena())
	{
		if(otherEmpireId == DgEmpire::ArenaObserverId())
			return POL_Friend;
		if((myEmpireId == DgEmpire::ArenaTeam1Id() || myEmpireId == DgEmpire::ArenaTeam2Id()) &&
				(otherEmpireId == DgEmpire::ArenaTeam1Id() || otherEmpireId == DgEmpire::ArenaTeam2Id()))
			return POL_Enemy;
		return POL_Neutral;
	}

	POLITICAL_STANCE ourStance;									// This will be our return value.

	// New empire encounters are always mutual.
	// Check to see if the unit's empire is known to the scene atom's empire and vice versa.
	const DgData& gd = g_Au.LockDataForRead();
	quint32 commonOverlord = gd.EmpireCommonOverlord(myEmpireId, otherEmpireId);
	bool bKnownToMe = gd.IsEmpireKnown(myEmpireId, otherEmpireId);				// Returns true for nonexistent myEmpireId.
	bool bKnownToOther = gd.IsEmpireKnown(otherEmpireId, myEmpireId);			// Returns true for nonexistent otherEmpireId.
	if(!bKnownToMe || !bKnownToOther)
	{
		// Empire is not already known. Set default political stance toward the other empire.
		ourStance = gd.EmpireDefaultPoliticalStance(myEmpireId);							// Our default stance.
		// Don't assume their empire doesn't know about us. If not, we'll get their default stance.
		POLITICAL_STANCE theirStance = gd.EmpirePoliticalStance(otherEmpireId, myEmpireId);	// Their stance, likely the default.
		g_Au.UnlockData();

		// A common overlord softens enemy reaction to neutral.
		if(commonOverlord)
		{
			if(!bKnownToMe && ourStance == POL_Enemy)
				ourStance = POL_Neutral;
			if(!bKnownToOther && theirStance == POL_Enemy)
				theirStance = POL_Neutral;
		}

		// Downgrade our stance to match their stance.
		switch(theirStance)
		{
		case POL_Friend:
		case POL_Empire:
		case POL_Vassal:
			break;
		case POL_Neutral:
			if(ourStance != POL_Enemy)
				ourStance = POL_Neutral;						// Match their neutral stance.
			break;
		case POL_Enemy:
			ourStance = POL_Enemy;								// Match their enemy stance.
			break;
		}

		// Downgrade their stance to match our stance.
		switch(ourStance)
		{
		case POL_Friend:
		case POL_Empire:
		case POL_Vassal:
			break;
		case POL_Neutral:
			if(theirStance != POL_Enemy)
				theirStance = POL_Neutral;						// Match our neutral stance.
			break;
		case POL_Enemy:
			theirStance = POL_Enemy;							// Match our enemy stance.
			break;
		}

		const DaAtom* otherPar = other->Parent();
		if(!otherPar)
			return POL_Neutral;	// wut?

		const DaWorld* planet = otherPar->FindParentPlanetInEnv();
		QString worldName = planet ? planet->Name() : "";
		const DaSolarSystem* system = otherPar->FindParentSolarSystem();
		QString systemName = system ? system->Name() : "";
		const DaSector* sector = otherPar->FindParentSector();
		QString sectorName = sector ? sector->Name() : "";
		const DaGalaxy* gal = otherPar->FindParentGalaxy();
		GALAXY galaxy = gal ? gal->Galaxy() : GALAXY_Count;

		// I must make the change locally or the contact that triggered this message will continue triggering it until the
		// master server gets done updating the empire record and propagates the change back to me. That produces a lot of mail.
		// This change is not stored in the database until the master gets the notification and updates the database. It will then
		// propagate a notification back down to the entire server farm that the empire records have changed.
		QString myName(Name());	// Name function sometimes locks data for read.
		QString otherName(other->Name());	// Name function sometimes locks data for read.
		DgData& gd = g_Au.LockDataForWrite();
		if(!bKnownToMe)
		{
			DgEmpireHash::iterator ie = gd.m_gEmpires.find(myEmpireId);
			if(ie != gd.m_gEmpires.end())
			{
				QString otherEmpireName;
				gd.EmpireName(otherEmpireId, otherEmpireName);
				(*ie).second.SetPoliticalStance(otherEmpireId, otherEmpireName, ourStance, 0, AuDocUnitContact::DocumentType(),
					Id(), myName, theirStance, other->Id(), otherName, galaxy, sectorName, systemName, worldName);
			}
		}
		if(!bKnownToOther)
		{
			DgEmpireHash::iterator ie = gd.m_gEmpires.find(otherEmpireId);
			if(ie != gd.m_gEmpires.end())
			{
				QString myEmpireName;
				gd.EmpireName(myEmpireId, myEmpireName);
				(*ie).second.SetPoliticalStance(myEmpireId, myEmpireName, theirStance, 0, AuDocUnitContact::DocumentType(),
					other->Id(), otherName, ourStance, Id(), myName, galaxy, sectorName, systemName, worldName);
			}
		}
		g_Au.UnlockData();

		// Notify the universe that the political stances of both empires have changed.
		emit SetPoliticalStance(otherEmpireId, myEmpireId, ourStance, 0, Id(), myName, theirStance, other->Id(), otherName,
				AuDocUnitContact::DocumentType(), galaxy, sectorName, systemName, worldName);
		emit SetPoliticalStance(myEmpireId, otherEmpireId, theirStance, 0, other->Id(), otherName, ourStance, Id(), myName,
				AuDocUnitContact::DocumentType(), galaxy, sectorName, systemName, worldName);
	}
	else
	{
		// Both empires already know about each other.
		ourStance = gd.EmpirePoliticalStance(myEmpireId, otherEmpireId);
		g_Au.UnlockData();
	}

	return ourStance;
}

DaAtom* DaAtom::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	// Don't sight myself or my subatoms.
	if(sightingAtom == this)
		return pBest;

	// This is a recursive search.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(pAtom->IsInstanceUniverse())
			continue;	// Skip subordinate environment atoms, such as arenas and design studios.
		pBest = pAtom->SightTarget(sightingAtom, eyeLoc, tModel * pAtom->Transform(), pBest, sightedLoc, sightedPart,
				sightedShield, sightedSurfaceNormal);
	}
	return pBest;
}

void DaAtom::AttackArea(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuItem& weaponIt,
		INVENTORYSUB sub, bool observeProtection)
{
	AuCombatWeapon weapon(weaponIt, sub);
	AuCombatResult result;
	AttackArea(attacker, defender, weapon, observeProtection, result);
}

// Location is relative to this atom.
void DaAtom::AttackArea(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuCombatWeapon& weapon,
		bool observeProtection, AuCombatResult& result)
{
	// Area attack affects all other creatures within weapon effect radius except others of my own kind.
	const DaAtom* env = FindEnvironmentAtom();
	Q_ASSERT(env);

	// Get my projection matrix relative to the solar system.
	const AuMatrix4x4& myT = EnvTransform();

	float maxRad = weapon.MaxRadius();

	// Create an attacker from the exploding weapon so we can call AttackTarget individually on those hit by shrapnel.
	AuCombatAttacker bomb(attacker);
	bomb.m_MuzzleLoc = defender.m_HitLoc;						// Source of attack is center of area effect/explosion.

	// Inflict damage on the atoms in the system that are in range.
	int bombsExploded = 0;										// Number of explosive items blown up, to limit chain reactions.
	DaAtomList hitAtoms;										// Atoms that are hit by individual testing.
	DaAtomList systemAtoms;
	env->ListNormalSubatomsRecursive(systemAtoms);	// Does not list 'abnormal' atoms. i.e. dead or spawning.
	for(DaAtomList::const_iterator it = systemAtoms.cbegin(); it != systemAtoms.cend(); it++)
	{
		DaAtom* pAtom = *it;
		if(!pAtom->IsNormal())
			continue;
		if(!pAtom->IsSightTarget(this))
			continue;											// Skip atoms that are not harmed by gunfire, such as planets.

		// Skip subordinate environment atoms, such as arenas and design studios.
		if(pAtom->IsInstanceUniverse())
			continue;

		// We can skip a lot of work by doing this check here.
		if(observeProtection && pAtom->IsNoncombatant())
			continue;

		// Determine the maximum distance at which the atom might be affected by the weapon.
		double maxDist = maxRad;
		bool targetIsBomb = false;
		DaCreature* creature = dynamic_cast<DaCreature*>(pAtom);
		if(creature)
		{
			if(attacker.IsSpeciesExempt(creature->Species()))
				continue;										// Ignore creatures of this kind.
			maxDist += 200;
		}
		else
		{
			DaCapsule* capsule = dynamic_cast<DaCapsule*>(pAtom);
			if(capsule)
				maxDist += capsule->RoughRadius();
			else if(dynamic_cast<DaVehicle*>(pAtom))
				maxDist += 50;
			else
			{
				DaItem* item = dynamic_cast<DaItem*>(pAtom);
				if(item)
				{
					if(item->DoesExplodeWhenDamaged())
						targetIsBomb = true;
				}
				else
				{
					DaWorld* world = dynamic_cast<DaWorld*>(pAtom);
					if(world)
					{
						DaWorldRingSection* rsworld = dynamic_cast<DaWorldRingSection*>(pAtom);
						if(rsworld)
							maxDist = 0;										// Coarse radius test skipped for ring sections.
						else
							maxDist += world->AtmosphereRadius();
					}
				}
			}
		}

		// Determine the atom's location relative to this atom.
		const AuMatrix4x4& targetT = pAtom->EnvTransform();
		AuPoint loc = targetT.Project(AuPoint(0,0));
		loc = myT.UnProject(loc);								// Project the atom's location to be relative to 'this' atom.

		// Determine if the atom is hit.
		AuPoint hitLoc;											// The hit location on the atom, relative to 'this' atom.
		bool hitShield = false;
		double targetRange;
		if(bomb.m_MuzzleLoc.FuzzyEquals(loc))
		{
			// Don't do SightTarget test when this close. It crashes if the distance is 0.
			// This happens when this atom is the hit target of a bay weapon and it doesn't sight on hull. It has to use my origin
			// location in that case.
			hitAtoms.append(pAtom);								// Remember the individual atoms that have been hit.
			hitLoc = loc;										// Hit its origin for range calc.
			hitShield = pAtom == this ? defender.m_HitShield : false;
			targetRange = 0;
		}
		else
		{
			// Perform coarse distance test to avoid doing too much work on far distant atoms.
			double dist2 = bomb.m_MuzzleLoc.Distance2(loc);
			if(maxDist > 0 && dist2 > maxDist*maxDist)
				continue;										// Atom is beyond maximum radius of effect.

			// Try shooting a ray from the target location to the location of the atom, to test for obstructions.
			if(loc.Length() > maxRad)
				loc.Unit(maxRad);								// Limit the distance to the blast radius.
			unsigned int hitPart = 0;
			DaAtom* target = SightTarget(hitLoc, hitPart, hitShield, bomb.m_MuzzleLoc, loc);	// What did we hit, if anything?
			if(!target)
				continue;										// Did not hit the target atom.
			if(target != pAtom)
				continue;										// Hit an atom other than the target atom.
			hitAtoms.append(pAtom);								// Remember the individual atoms that have been hit.
			targetRange = hitLoc.Distance(bomb.m_MuzzleLoc);
		}

		// The sight ray hit the atom so there was no intervening obstruction.
		// Inflict damage on the creature if it is within range.
		bool targetKilled;
		if(targetRange < weapon.m_w1.m_Radius && targetRange < weapon.m_w2.m_Radius)
		{
			// Hit with both weapon effects.
			// Limit the number of bombs that get blown up in a chain reaction.
			if(targetIsBomb && bombsExploded >= d_BombChainReaction)
				continue;				// Skip the bomb. The chain reaction of the other bombs will probably set it off.

			// Create shrapnel from the exploding weapon.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = maxRad;
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(pAtom, hitLoc, hitShield, targetRange);
			if(pAtom == defender.m_Atom)
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
		else if(targetRange < weapon.m_w1.m_Radius)
		{
			// Hit only with primary weapon effect.
			// Limit the number of bombs that get blown up in a chain reaction.
			if(targetIsBomb && bombsExploded >= d_BombChainReaction)
				continue;				// Skip the bomb. The chain reaction of the other bombs will probably set it off.

			// Create shrapnel from the exploding weapon.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = weapon.m_w1.m_Radius;
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Effect = WE_None;
			shrapnel.m_w2.m_Damage = 0;
			shrapnel.m_w2.m_DamageMod = 0;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(pAtom, hitLoc, hitShield, targetRange);
			if(pAtom == defender.m_Atom)
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
		else if(targetRange < weapon.m_w2.m_Radius)
		{
			// Hit only with secondary weapon effect.
			// Limit the number of bombs that get blown up in a chain reaction.
			if(targetIsBomb && bombsExploded >= d_BombChainReaction)
				continue;				// Skip the bomb. The chain reaction of the other bombs will probably set it off.

			// Create shrapnel from the exploding weapon.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = weapon.m_w2.m_Radius;
			shrapnel.SwapWeapons();
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Effect = WE_None;
			shrapnel.m_w2.m_Damage = 0;
			shrapnel.m_w2.m_DamageMod = 0;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(pAtom, hitLoc, hitShield, targetRange);
			if(pAtom == defender.m_Atom)
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				targetKilled = AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
		else
			targetKilled = false;

		if(targetKilled && targetIsBomb)
			bombsExploded++;
	}

	// Now we must inflict damage in all directions to hit buildings and terrain. They are not seen by the individual atom tests.
	// This is only possible when the target location is inside the atmosphere of a planet.
	AuPoint icoverts[] =
	{
		AuPoint(0, 0.525731, 0.850651),
		AuPoint(0, 0.525731, -0.850651),
		AuPoint(0, -0.525731, 0.850651),
		AuPoint(0, -0.525731, -0.850651),
		AuPoint(0.525731, 0.850651),
		AuPoint(0.525731, -0.850651),
		AuPoint(-0.525731, 0.850651),
		AuPoint(-0.525731, -0.850651),
		AuPoint(0.850651, 0, 0.525731),
		AuPoint(-0.850651, 0, 0.525731),
		AuPoint(0.850651, 0, -0.525731),
		AuPoint(-0.850651, 0, -0.525731)
	};
	for(int shrapIdx=0; shrapIdx<12; shrapIdx++)
	{
		// Try shooting a ray from the target location to the location of the atom, to test for obstructions.
		AuPoint loc = bomb.m_MuzzleLoc + icoverts[shrapIdx] * maxRad;
		AuPoint hitLoc;
		unsigned int hitPart = 0;
		bool hitShield = false;
		DaAtom* target = SightTarget(hitLoc, hitPart, hitShield, bomb.m_MuzzleLoc, loc);	// What did we hit, if anything?
		if(!target)
			continue;											// Hit nothing.
		if(hitAtoms.contains(target))
			continue;											// Already hit that atom.

		// The sight ray hit an atom so there was no intervening obstruction.
		// Inflict damage on the creature if it is within range.
		double targetRange = hitLoc.Distance(bomb.m_MuzzleLoc);
		if(targetRange < weapon.m_w1.m_Radius && targetRange < weapon.m_w2.m_Radius)
		{
			// Hit with both weapon effects.
			// Create shrapnel from the exploding weapon. We will reduce the damage due to multiple particles.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = maxRad;
			shrapnel.m_w1.m_Damage = shrapnel.m_w1.m_Damage/2;
			shrapnel.m_w1.m_DamageMod = shrapnel.m_w1.m_DamageMod/2;
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Damage = shrapnel.m_w2.m_Damage/2;
			shrapnel.m_w2.m_DamageMod = shrapnel.m_w2.m_DamageMod/2;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(target, hitLoc, hitShield, targetRange);
			if(target == defender.m_Atom)
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
		else if(targetRange < weapon.m_w1.m_Radius)
		{
			// Hit only with primary weapon effect.
			// Create shrapnel from the exploding weapon. We will reduce the damage due to multiple particles.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = weapon.m_w1.m_Radius;
			shrapnel.m_w1.m_Damage = shrapnel.m_w1.m_Damage/2;
			shrapnel.m_w1.m_DamageMod = shrapnel.m_w1.m_DamageMod/2;
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Effect = WE_None;
			shrapnel.m_w2.m_Damage = 0;
			shrapnel.m_w2.m_DamageMod = 0;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(target, hitLoc, hitShield, targetRange);
			if(target == defender.m_Atom)
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
		else if(targetRange < weapon.m_w2.m_Radius)
		{
			// Hit only with secondary weapon effect.
			// Create shrapnel from the exploding weapon. We will reduce the damage due to multiple particles.
			AuCombatWeapon shrapnel(weapon);
			shrapnel.m_Mode = WM_Projectile;
			shrapnel.m_Range = weapon.m_w2.m_Radius;
			shrapnel.SwapWeapons();
			shrapnel.m_w1.m_Damage = shrapnel.m_w1.m_Damage/2;
			shrapnel.m_w1.m_DamageMod = shrapnel.m_w1.m_DamageMod/2;
			shrapnel.m_w1.m_Radius = 0;
			shrapnel.m_w2.m_Effect = WE_None;
			shrapnel.m_w2.m_Damage = 0;
			shrapnel.m_w2.m_DamageMod = 0;
			shrapnel.m_w2.m_Radius = 0;

			AuCombatDefender defendShrap(target, hitLoc, hitShield, targetRange);
			if(target == defender.m_Atom)
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, result);
			else
			{
				AuCombatResult cres;
				AttackTarget(bomb, defendShrap, shrapnel, observeProtection, cres);
			}
		}
	}
}

bool DaAtom::AttackTarget(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuItem& weaponIt,
		INVENTORYSUB sub, bool observeProtection)
{
	AuCombatWeapon weapon(weaponIt, sub);
	AuCombatResult result;
	return AttackTarget(attacker, defender, weapon, observeProtection, result);
}

// 'targetLoc' is the sighted location on the target, relative to this atom.
bool DaAtom::AttackTarget(const AuCombatAttacker& attacker, const AuCombatDefender& defender, const AuCombatWeapon& weapon,
		bool observeProtection, AuCombatResult& result)
{
	//if(Is(AuId(AuId::Haxus())))
		//qDebug() << "DaAtom::AttackTarget" << Id() << Name() << "target" << target->Id() << target->Name()
			//<< "targLoc" << targetLoc << "targRange" << targetRange << "wCID" << wCID << "wTL" << weaponQ << "wMode" << wMode
			//<< "wRange" << wRange
			//<< "effect1" << wEffect1 << "dmg1" << wDamage1 << "dmgMod1" << wDamageMod1 << "wRad1" << wRadius1
			//<< "effect2" << wEffect2 << "dmg2" << wDamage2 << "dmgMod2" << wDamageMod2 << "wRad2" << wRadius2;

	// Radius weapons are treated as though they hit the target and then have an area effect at the impact point.
	if(weapon.m_w1.m_Radius > 0 && weapon.m_w2.m_Radius > 0)
	{
		// Attack all targets in the area of the target location using the radius effect of the weapons.
		// No animal DNA is exempt.
		AttackArea(attacker, defender, weapon, observeProtection, result);

		// Both effects were area effects so we're done.
		return false;
	}
	else if(weapon.m_w1.m_Radius > 0)
	{
		// First effect has a radius.
		// Attack all targets in the area of the target location using the radius effect of the weapon.
		// No animal DNA is exempt.
		AuCombatWeapon areaw(weapon);
		areaw.m_w2.m_Effect = WE_None;
		areaw.m_w2.m_Damage = 0;
		areaw.m_w2.m_DamageMod = 0;
		areaw.m_w2.m_Radius = 0;
		AttackArea(attacker, defender, areaw, observeProtection, result);

		// If there is no effect 2, we're done.
		if(weapon.m_w2.m_Effect == WE_None || !defender.m_Atom)
			return false;

		// Effect 2 is not an area effect. Shuffle the numbers to effect 1 and proceed.
		AuCombatWeapon wpn(weapon);
		wpn.SwapWeapons();
		wpn.m_w2.m_Effect = WE_None;
		wpn.m_w2.m_Damage = 0;
		wpn.m_w2.m_DamageMod = 0;
		wpn.m_w2.m_Radius = 0;

		AuCombatResult res2;	// Second result so we can combine them.
		AttackTarget(attacker, defender, wpn, observeProtection, res2);

		// Combine the results.
		result.m_AuxArmor2 = res2.m_AuxArmor1;
		result.m_BaseArmor2 = res2.m_BaseArmor1;
		result.m_Damage2 = res2.m_Damage1;
		result.m_CriticalHits += res2.m_CriticalHits;
		if(res2.m_IsDestroyed)
			result.m_IsDestroyed = true;
		return result.m_IsDestroyed;
	}
	else if(weapon.m_w2.m_Radius > 0)
	{
		// Second effect has a radius.
		// Attack all targets in the area of the target location using the radius effect of the weapon.
		// No animal DNA is exempt.
		// Peace is observed because calls to AttackTarget with radius weapons are only done by players. Wild animals do
        // not have natural body weapons that have a radius effect at the impact point, like a grenade launcher.
		AuCombatWeapon areaw(weapon);
		areaw.SwapWeapons();
		areaw.m_w2.m_Effect = WE_None;
		areaw.m_w2.m_Damage = 0;
		areaw.m_w2.m_DamageMod = 0;
		areaw.m_w2.m_Radius = 0;
		AttackArea(attacker, defender, areaw, true, result);

		// If there is no effect 1, we're done.
		if(weapon.m_w1.m_Effect == WE_None || !defender.m_Atom)
			return false;

		// Effect 1 is not an area effect.
		AuCombatWeapon wpn(weapon);
		wpn.m_w2.m_Effect = WE_None;
		wpn.m_w2.m_Damage = 0;
		wpn.m_w2.m_DamageMod = 0;
		wpn.m_w2.m_Radius = 0;
		AuCombatResult res2;	// Second result so we can combine them.
		AttackTarget(attacker, defender, wpn, observeProtection, res2);

		// Combine the results.
		// 'result' currently holds the weapon 2 result in the result 1 slots. 'res2' holds the results of weapon 1.
		result.m_AuxArmor2 = result.m_AuxArmor1;
		result.m_BaseArmor2 = result.m_BaseArmor1;
		result.m_Damage2 = result.m_Damage1;
		result.m_AuxArmor1 = res2.m_AuxArmor1;
		result.m_BaseArmor1 = res2.m_BaseArmor1;
		result.m_Damage1 = res2.m_Damage1;
		result.m_CriticalHits += res2.m_CriticalHits;
		if(res2.m_IsDestroyed)
			result.m_IsDestroyed = true;
		return result.m_IsDestroyed;
	}

	// From here on, we need to be operating on a specific defender atom.
	if(!defender.m_Atom)
		return false;
	if(defender.m_Range > weapon.m_Range)
		return false;
	if(observeProtection && defender.m_Atom->IsNoncombatant())
        return false;

	// Convert target location to be relative to the target for the hit effect and armor determination. If the targetLoc is zero,
	// we'll use the insertion point of the target.
	// Target location has been specified relative to this atom. I need to convert it to be relative to the target atom.
	// Find a common parent atom, which will at least be the solar system.
	DaAtom* commonParent = CommonParent(defender.m_Atom, 0);
	if(!commonParent)
		return false;
	AuMatrix4x4 mycp;
	GetTransformRelativeToParent(mycp, commonParent);
	AuMatrix4x4 defcp;
	defender.m_Atom->GetTransformRelativeToParent(defcp, commonParent);
	AuPoint targLocCP = mycp.Project(defender.m_HitLoc);						// rel to common parent
	AuPoint wpnLocCP = mycp.Project(attacker.m_MuzzleLoc);						// rel to common parent
	AuPoint targLoc = defcp.Inverse().Project(targLocCP);						// rel to target atom
	AuPoint wpnLoc = defcp.Inverse().Project(wpnLocCP);							// rel to target atom

	// Resolve the combat.
	result = AuCombatResult();			// Clear the result values.
	defender.m_Atom->TargetAttacked(wpnLoc, targLoc, defender.m_HitShield, weapon, result);

	// Apply the damage.
	defender.m_Atom->CombatApplyDamage(attacker, weapon, wpnLoc, targLoc, result);

	// Create special effects for the weapon hit sounds.
	{
		AuPoint rot;

		SOUND_EFFECT sound1 = (SOUND_EFFECT)weapon.HitSound1();
		VISUAL_EFFECT visual1 = VISUAL_None;
		int rotStyle = 2;	// 0=random, 1=align, 2=none
		switch(weapon.m_w1.m_Effect)
		{
		case WE_Acid:
			visual1 = VISUAL_HitAcid;
			break;
		case WE_Suffocate:
			visual1 = VISUAL_HitGasping;
			break;
		case WE_Stun:
			visual1 = VISUAL_HitParalysis;
			break;
		case WE_Poison:
			visual1 = VISUAL_HitPoison;
			break;
		case WE_Radiation:
			visual1 = VISUAL_HitRadiation;
			break;
		case WE_Sonic:
			visual1 = VISUAL_HitSonic;
			break;
		case WE_Crush:
		case WE_Bludgeon:
			visual1 = VISUAL_HitBludgeon;
			break;
		case WE_Cold:
			visual1 = VISUAL_HitCold;
			break;
		case WE_Disease:
			visual1 = VISUAL_HitDisease;
			break;
		case WE_Electric:
			visual1 = VISUAL_HitElectric;
			break;
		case WE_Heat:
			visual1 = VISUAL_HitFire;
			break;
		case WE_Pierce:
			visual1 = VISUAL_HitPierce;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		case WE_Implant:
			visual1 = VISUAL_HitSeed;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		case WE_Slash:
			visual1 = VISUAL_HitSlash;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		// Remaining weapon effects are not inflicted by creatures at all.
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:	// Used when creature is dying but not dead.
		case WE_Corrosion:
		case WE_Unused:
		case WE_Generic:	// shield vulnerability category
		case WE_Starving:
			visual1 = VISUAL_None;
			break;
		}

		if(rotStyle == 0)
		{
			// Some effects want a random rotation for the effect at the effect location.
			rot.x = g_Au.Dice().Roll(1,360) * DEG2RADf;
			rot.y = g_Au.Dice().Roll(1,360) * DEG2RADf;
			rot.z = g_Au.Dice().Roll(1,360) * DEG2RADf;
		}
		else if(rotStyle == 1)
		{
			// Calculate rotation to align effect to strike vector.
			// The HitPierce 3ds model is oriented along the +Z axis.
			AuPoint hitDir = wpnLoc - targLoc;					// rel to target
			hitDir.Unit();
			AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(hitDir);	// Spray toward weapon.
			rot = trot.GetRotation();
		}
		else if(rotStyle == 2)
		{
			rot = AuPoint(0,0);
		}

		DaEffect* hitEff = 0;
		AuPoint hitEffVec;

		if(sound1 != SOUND_None)
		{
			// Select corresponding sound for special hit situation.
			float gain;
			if(result.m_Damage1 == 0)
			{
				if(sound1 != SOUND_HitExplosion)
					sound1 = (SOUND_EFFECT)(sound1 + 2);
				gain = 1;
				visual1 = VISUAL_None;
			}
			else
			{
				if(result.m_CriticalHits && sound1 != SOUND_HitExplosion)
					sound1 = (SOUND_EFFECT)(sound1 + 1);
				gain = (float)result.m_Damage1/100.f;
			}
			if(defender.m_HitShield)
			{
				// Hit on energy shield.
				gain = (float)(result.m_Damage1 + result.m_BaseArmor1 + result.m_AuxArmor1)/1000.f;
				visual1 = VISUAL_HitElectric;
				if(sound1 != SOUND_HitExplosion)
					sound1 = SOUND_HitElectric;
			}

			//qDebug() << "DaAtom::AttackTarget" << Id() << Name() << "visual1" << visual1 << "sound1" << sound1 << "gain" << gain;
			if(result.m_IsDestroyed)
			{
				// Defender is destroyed.
				// Create an effect atom to carry the hit effect.
				AuAtomPos dpos(defender.m_Atom->Pos());
				if(!hitEff)
				{
					DaAtom* defPar = defender.m_Atom->Parent();
					if(defPar)
					{
						hitEff = defPar->CreateEffect();
						hitEffVec = dpos.Transform().ProjectVector(defender.m_Atom->Vector());
					}
				}

				if(hitEff)
				{
					AuEffect effect(dpos.Transform().Project(targLoc), rot, hitEffVec, sound1, gain, 1, visual1);
					effect.SetTargetLoc(dpos.Transform().Project(wpnLoc));
					hitEff->AddEffect(effect);
				}
			}
			else
			{
				AuEffect effect(targLoc, rot, AuPoint(0,0), sound1, gain, 1, visual1);
				effect.SetTargetLoc(wpnLoc);
				effect.SetShieldHit(defender.m_HitShield);
				defender.m_Atom->AddEffect(effect);
			}
		}

		SOUND_EFFECT sound2 = (SOUND_EFFECT)weapon.HitSound2();
		VISUAL_EFFECT visual2 = VISUAL_None;
		rotStyle = 2;	// 0=random, 1=align, 2=none
		switch(weapon.m_w2.m_Effect)
		{
		case WE_Acid:
			visual2 = VISUAL_HitAcid;
			break;
		case WE_Suffocate:
			visual2 = VISUAL_HitGasping;
			break;
		case WE_Stun:
			visual2 = VISUAL_HitParalysis;
			break;
		case WE_Poison:
			visual2 = VISUAL_HitPoison;
			break;
		case WE_Radiation:
			visual2 = VISUAL_HitRadiation;
			break;
		case WE_Sonic:
			visual2 = VISUAL_HitSonic;
			break;
		case WE_Crush:
		case WE_Bludgeon:
			visual2 = VISUAL_HitBludgeon;
			break;
		case WE_Cold:
			visual2 = VISUAL_HitCold;
			break;
		case WE_Disease:
			visual2 = VISUAL_HitDisease;
			break;
		case WE_Electric:
			visual2 = VISUAL_HitElectric;
			break;
		case WE_Heat:
			visual2 = VISUAL_HitFire;
			break;
		case WE_Pierce:
			visual2 = VISUAL_HitPierce;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		case WE_Implant:
			visual2 = VISUAL_HitSeed;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		case WE_Slash:
			visual2 = VISUAL_HitSlash;
			
			// Calculate rotation to align effect to strike vector.
			if(wpnLoc != targLoc)
				rotStyle = 1;
			break;
		// Remaining weapon effects are not inflicted by creatures at all.
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:	// Used when creature is dying but not dead.
		case WE_Corrosion:
		case WE_Unused:
		case WE_Generic:	// shield vulnerability category
		case WE_Starving:
			visual2 = VISUAL_None;
			break;
		}

		if(rotStyle == 0)
		{
			// Some effects want a random rotation for the effect at the effect location.
			rot.x = g_Au.Dice().Roll(1,360) * DEG2RADf;
			rot.y = g_Au.Dice().Roll(1,360) * DEG2RADf;
			rot.z = g_Au.Dice().Roll(1,360) * DEG2RADf;
		}
		else if(rotStyle == 1)
		{
			// Calculate rotation to align effect to strike vector.
			// The HitPierce 3ds model is oriented along the +Z axis.
			AuPoint hitDir = wpnLoc - targLoc;					// rel to target
			hitDir.Unit();
			AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(hitDir);	// Spray toward weapon.
			rot = trot.GetRotation();
		}
		else if(rotStyle == 2)
		{
			rot = AuPoint(0,0);
		}

		if(sound2 != SOUND_None)
		{
			// Select corresponding sound for special hit situation.
			float gain;
			if(result.m_Damage2 == 0)
			{
				if(sound2 != SOUND_HitExplosion)
					sound2 = (SOUND_EFFECT)(sound2 + 2);
				gain = 1;
				visual2 = VISUAL_None;
			}
			else
			{
				if(result.m_CriticalHits && sound2 != SOUND_HitExplosion)
					sound2 = (SOUND_EFFECT)(sound2 + 1);
				gain = (float)result.m_Damage2/100.f;
			}
			if(defender.m_HitShield)
			{
				// Hit on energy shield.
				gain = (float)(result.m_Damage2 + result.m_BaseArmor2 + result.m_AuxArmor2)/1000.f;
				visual2 = VISUAL_HitElectric;
				if(sound2 != SOUND_HitExplosion)
					sound2 = SOUND_HitElectric;
			}

			if(result.m_IsDestroyed)
			{
				// Defender is destroyed.
				// Create an effect atom to carry the hit effect.
				AuAtomPos dpos(defender.m_Atom->Pos());
				if(!hitEff)
				{
					DaAtom* defPar = defender.m_Atom->Parent();
					if(defPar)
					{
						hitEff = defPar->CreateEffect();
						hitEffVec = dpos.Transform().ProjectVector(defender.m_Atom->Vector());
					}
				}

				if(hitEff)
				{
					AuEffect effect(dpos.Transform().Project(targLoc), rot, hitEffVec, sound2, gain, 1, visual2);
					effect.SetTargetLoc(dpos.Transform().Project(wpnLoc));
					hitEff->AddEffect(effect);
				}
			}
			else
			{
				AuEffect effect(targLoc, rot, AuPoint(0,0), sound2, gain, 1, visual2);
				effect.SetTargetLoc(wpnLoc);
				effect.SetShieldHit(defender.m_HitShield);
				defender.m_Atom->AddEffect(effect);
			}
		}
	}
	return result.m_IsDestroyed;
}

// I am the target of an attack.
void DaAtom::TargetAttacked(const AuPoint& weaponLoc, const AuPoint& hitLoc, bool hitShield, const AuCombatWeapon& weapon,
		AuCombatResult& result) const
{
	Q_ASSERT(weapon.m_Mode != WM_Not);							// Must use a valid weapon mode.
	if(weapon.m_w1.m_Effect == WE_None)
	{
		qDebug() << "DaAtom::TargetAttacked" << Id() << Name() << "weapon" << weapon.m_CID << "effect WE_None!";
		return;
	}

	// Determine armor absorption factors vs weapon effect 1.
	float base1Absorb, base1Block, aux1Absorb, aux1Block;
	CombatArmorEffectiveness(weapon.m_Mode, weapon.m_w1.m_Effect, hitShield, weaponLoc, hitLoc,
			base1Absorb, base1Block, aux1Absorb, aux1Block);

	// Determine armor absorption factors vs weapon effect 2.
	float base2Absorb, base2Block, aux2Absorb, aux2Block;
	if(weapon.m_w2.m_Effect != WE_None)
		CombatArmorEffectiveness(weapon.m_Mode, weapon.m_w2.m_Effect, hitShield, weaponLoc, hitLoc,
				base2Absorb, base2Block, aux2Absorb, aux2Block);
	else
	{
		base2Absorb = 0;
		base2Block = 0;
		aux2Absorb = 0;
		aux2Block = 0;
	}

	// Check for critical hits.
	// Rolling a 95% or better is considered a critical hit.
	result.m_CriticalHits = 0;									// Number of critical hits: 0, 1, or 2.
	if(g_Au.Dice().Roll(1,100) > 95)
	{
		// Critical hit nullifies the armor of the target.
		result.m_CriticalHits = 1;
		base1Absorb /= 2.f;
		base1Block /= 2.f;
		//aux1Absorb /= 2.f;
		//aux1Block /= 2.f;

		// Rolling another 95% or better is considered a double critical hit.
		if(weapon.m_w2.m_Effect != WE_None && g_Au.Dice().Roll(1,100) > 95)
		{
			// Second critical hit nullifies the second armor of the target.
			result.m_CriticalHits = 2;
			base2Absorb /= 2.f;
			base2Block /= 2.f;
			//aux2Absorb /= 2.f;
			//aux2Block /= 2.f;
		}
	}

	// Weapon 1 effect
	{
		// Apply Armor Piercing and Strike Through.
		// These factors do not apply to damage absorbed by shields.
		if(weapon.m_w1.m_AP > 0)
		{
			base1Absorb -= weapon.m_w1.m_AP;
			if(base1Absorb < 0.f)
				base1Absorb = 0.f;
			//aux1Absorb -= weapon.m_w1.m_AP;
			//if(aux1Absorb < 0.f)
				//aux1Absorb = 0.f;
		}
		if(weapon.m_w1.m_ST > 0)
		{
			base1Block -= weapon.m_w1.m_ST;
			if(base1Block < 0.f)
				base1Block = 0.f;
			//aux1Block -= weapon.m_w1.m_ST;
			//if(aux1Block < 0.f)
				//aux1Block = 0.f;
		}

		// Determine raw damage caused by weapon 1, unmodified by armor.
		int damageRoll1 = g_Au.Dice().Roll(1, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod);
		if(damageRoll1 > 0)
		{
			// Calculate armor effects vs weapon 1.
			// The aux armor absorbs as much damage as possible before the underlying base armor absorbs damage.
			result.m_AuxArmor1 = (int)ceilf((float)damageRoll1 * aux1Absorb + aux1Block);	// aux armor vs effect 1.
			if((int)result.m_AuxArmor1 >= damageRoll1)
			{
				// Aux armor was sufficient to absorb all of the damage.
				result.m_Damage1 = 0;											// All damage was absorbed by armor.
				result.m_AuxArmor1 = damageRoll1;								// Aux armor absorbed all of the damage.
				result.m_BaseArmor1 = 0;										// Base armor unaffected because aux absorbed all.
			}
			else
			{
				result.m_BaseArmor1 = (int)ceilf((float)damageRoll1 * base1Absorb + base1Block);	// base armor vs effect 1.
				if((int)(result.m_AuxArmor1 + result.m_BaseArmor1) >= damageRoll1)
				{
					// Aux and base armor were sufficient to absorb all of the damage.
					result.m_Damage1 = 0;										// All damage was absorbed by armor.
					result.m_BaseArmor1 = damageRoll1 - result.m_AuxArmor1;		// Base armor absorbed some; aux armor absorbed max.
				}
				else
				{
					// Aux and base armor were not sufficient to absorb all of the damage. They absorbed the max with points left
					// over.
					result.m_Damage1 = damageRoll1 - (result.m_BaseArmor1 + result.m_AuxArmor1);
				}
			}
		}
		else
		{
			// Rolled zero damage from weapon 1.
			result.m_Damage1 = 0;
			result.m_AuxArmor1 = 0;
			result.m_BaseArmor1 = 0;
		}
	}

	if(weapon.m_w2.m_Effect != WE_None)
	{
		// Apply Armor Piercing and Strike Through.
		// These factors do not apply to damage absorbed by shields.
		if(weapon.m_w2.m_AP > 0)
		{
			base2Absorb -= weapon.m_w2.m_AP;
			if(base2Absorb < 0.f)
				base2Absorb = 0.f;
			//aux2Absorb -= weapon.m_w2.m_AP;
			//if(aux2Absorb < 0.f)
				//aux2Absorb = 0.f;
		}
		if(weapon.m_w2.m_ST > 0)
		{
			base2Block -= weapon.m_w2.m_ST;
			if(base2Block < 0.f)
				base2Block = 0.f;
			//aux2Block -= weapon.m_w2.m_ST;
			//if(aux2Block < 0.f)
				//aux2Block = 0.f;
		}

		// Determine raw damage caused by weapon 2, unmodified by armor.
		int damageRoll2 = g_Au.Dice().Roll(1, weapon.m_w2.m_Damage, weapon.m_w2.m_DamageMod);
		if(damageRoll2 > 0)
		{
			// Calculate armor effects vs weapon 2.
			// The aux armor absorbs as much damage as possible before the underlying base armor absorbs damage.
			result.m_AuxArmor2 = (int)ceilf((float)damageRoll2 * aux2Absorb + aux2Block);	// aux armor vs effect 2.
			if((int)result.m_AuxArmor2 >= damageRoll2)
			{
				// Aux armor was sufficient to absorb all of the damage.
				result.m_Damage2 = 0;											// All damage was absorbed by armor.
				result.m_AuxArmor2 = damageRoll2;								// Aux armor absorbed all of the damage.
				result.m_BaseArmor2 = 0;										// Base armor unaffected because aux absorbed all.
			}
			else
			{
				result.m_BaseArmor2 = (int)ceilf((float)damageRoll2 * base2Absorb + base2Block);	// base armor vs effect 2.
				if((int)(result.m_AuxArmor2 + result.m_BaseArmor2) >= damageRoll2)
				{
					// Aux and base armor were sufficient to absorb all of the damage.
					result.m_Damage2 = 0;										// All damage was absorbed by armor.
					result.m_BaseArmor2 = damageRoll2 - result.m_AuxArmor2;		// Base armor absorbed some; aux armor absorbed max.
				}
				else
				{
					// Aux and base armor were not sufficient to absorb all of the damage. They absorbed the max with points left
					// over.
					result.m_Damage2 = damageRoll2 - (result.m_BaseArmor2 + result.m_AuxArmor2);
				}
			}
		}
		else
		{
			// Rolled zero damage from weapon 2.
			result.m_Damage2 = 0;
			result.m_AuxArmor2 = 0;
			result.m_BaseArmor2 = 0;
		}
	}
	else
	{
		// No weapon 2.
		result.m_Damage2 = 0;
		result.m_AuxArmor2 = 0;
		result.m_BaseArmor2 = 0;
	}
}

// true if this atom is riding on a creature, in any capacity, either as a passenger or as a carried body or equipment.
bool DaAtom::IsRider() const
{
	return dynamic_cast<const DaCreature*>(Parent()) ? true : false;
}

// Returns the transformation matrix relative to this atom's parent solar system.
AuMatrix4x4 DaAtom::EnvTransform() const
{
	// Try for precalculated env transform.
	m_EnvTransformLock.lockForRead();
	if(m_EnvTransformValid)
	{
		AuMatrix4x4 et(m_EnvTransform);
		m_EnvTransformLock.unlock();
		return et;
	}
	m_EnvTransformLock.unlock();

	// Must calculate env transform.
	m_EnvTransformLock.lockForWrite();
	if(!m_EnvTransformValid)
	{
		// Determine my current transform relative to my parent solar system.
		const DaAtom* parent = Parent();
		if(!parent)
		{
			// I have no parent atom.
			m_EnvTransform.LoadIdentity();
			//qDebug() << "DaAtom::EnvTransform" << Id() << Name() << "No parent atom" << m_EnvTransform;
		}
		else if(parent->CanRenderEnvironment())
		{
			// My parent atom is the solar system. System transform is the same as my atom transform.
			m_EnvTransform = Transform();
			//qDebug() << "DaAtom::EnvTransform" << Id() << Name() << "parent is solar system" << m_EnvTransform;
		}
		else
		{
			// My parent atom is not the environment atom. Env transform is my parent's env transform times my atom transform.
			m_EnvTransform = parent->EnvTransform() * Transform();
			//qDebug() << "DaAtom::EnvTransform" << Id() << Name() << "parent is not solar system" << m_EnvTransform;
		}

		// The transform is valid until somebody says otherwise.
		m_EnvTransformValid = true;
	}
	AuMatrix4x4 et(m_EnvTransform);
	m_EnvTransformLock.unlock();

	// Return this atom's transform relative to its parent environment atom.
	return et;
}

void DaAtom::DesignSetGravity(bool gravOn)
{
	// The scene should be subordinate to a design environment.
	DaAtom* parent = Parent();
	if(!parent)
		return;
	DaDesignBase* pde = parent->FindParentDesignBaseInEnv();
	if(pde)
	{
		// The design environment contains a design plane, which is the grid.
		DaDesignPlane* pdp = (DaDesignPlane *)pde->FindDesignPlane(Id());
		if(pdp && pdp->IsGravityOn() != gravOn)
		{
			pdp->GravityToggle();
			pdp->SetModified();
		}
	}
}

// 'loc' is relative to this atom.
// 'personAtom' is a return value.
// 'personRange' is a return value. It is the squared distance to the personAtom.
void DaAtom::FindClosestPersonOnLineRecursive(const AuPoint& loc, DaPerson*& personAtom, double& personRange) const
{
	DaAtomList people;
	FindPeopleOnLineRecursive(people);
	for(DaAtomList::const_iterator it = people.cbegin(); it != people.cend(); it++)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(*it);
		if(!person)
			continue;
		if(!person->IsNormal())
			continue;
		if(person->AgentInvisible())
			continue;										// Ignore people who are invisible

		AuPoint personLoc;
		person->GetLocationRelativeToParent(personLoc, this);
		double range = loc.Distance2(personLoc);
		if(!personAtom || range < personRange)
		{
			personAtom = person;
			personRange = range;
		}
	}
}

void DaAtom::DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc)
{
	DaAtomList atoms;
	ListNormalSubatoms(atoms);
	atoms.DetectWarpSignatureRecursive(systemId, warpFactors, galaxyLoc);
}

DaEffect* DaAtom::CreateEffect()
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// First try to locate an effect atom subordinate to this atom.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaEffect* effect = dynamic_cast<DaEffect*>(*it);
		if(effect)
			return effect;
	}

	// An effect atom was not found. Create one now.
	DaEffect* effect = new DaEffect();
	effect->ConnectSignals();
	effect->Create();
	effect->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
	ether->AddAtom(effect);
	effect->SetNormal();	// New atom is ready for normal thread processing.
	effect->TickTock();
	//qDebug() << "DaAtom::CreateEffect" << Id() << Name() << "effect created" << effect->Id() << effect->Name();
	return effect;
}

// Returns current flight instrument readings:
//  'centerOffset' specifies an offset to this atom's location for position calculations.
//  'mode' specifies relativity desired: 0=auto, 1=planet, 2=system, 3=sector, 4=star
// Readings are returned in the following parameters:
//  'speed' overall speed in fps, always positive or zero.
//  'speedF' forward speed in fps, negative is backward
//  'speedL' lateral speed in fps, negative is left
//  'speedV' vertical speed in fps, negative is down
//  'altitude' is altitude above reference in feet. MSL altitude in planet mode.
//  'pitch' is pitch angle
//  'roll' is roll angle
//  'heading' is heading angle
//  'longitude' is real longitude angle. Negative is west; positive is east. This matches AuPoint::ArcTangent_2D.
//  'latitude' is real latitude angle. Negative is south; positive is north. This matches AuPoint::PolarIncline.
// returns actual mode of reference.
// returns 0 if 'mode' produced no reference target; only speed values will be valid.
int DaAtom::FlightInstruments(const AuPoint& centerOffset, int mode, double& speed, double& speedF, double& speedL, double& speedV,
		double& altitude, double& pitch, double& roll, double& heading, double& longitude, double& latitude) const
{
	// Speed readings don't need a reference object.
	speed = Speed();				// Overall speed.
	speedF = ForwardSpeed();		// Forward(+) or backward(-) speed.
	speedL = LateralSpeed();		// Right(+) or left(-) speed.
	speedV = VerticalSpeed();		// Up(+) or down(-) speed.

	// Locate our reference object based on mode.
	const DaAtom* parent = Parent();
	if(!parent)
	{
		altitude = 0;
		pitch = 0;
		roll = 0;
		heading = 0;
		longitude = 0;
		latitude = 0;
		return 0;
	}

	const DaWorld* planet = 0;			// Needed for mode 1.
	const DaStar* star = 0;				// Needed for mode 4.
	const DaAtom* system = 0;			// Needed for mode 2.
	const DaSector* sector = 0;			// Needed for mode 3.
	int modeAct = 0;					// The actual reference mode we decide upon.
	switch(mode)
	{
	case 0:		// auto mode
	default:
		planet = parent->FindParentPlanetInEnv();
		if(planet)
			modeAct = 1;
		else
		{
			star = parent->FindParentStarInEnv();
			if(star)
				modeAct = 4;
			else
			{
				system = FindEnvironmentAtom();
				if(system)
					modeAct = 2;
				else
				{
					sector = parent->FindParentSector();
					if(sector)
						modeAct = 3;
				}
			}
		}
		break;
	case 1:		// planet mode
		planet = parent->FindParentPlanetInEnv();
		if(planet)
			modeAct = 1;
		break;
	case 2:		// system mode
		system = FindEnvironmentAtom();
		if(system)
			modeAct = 2;
		break;
	case 3:		// sector mode
		sector = parent->FindParentSector();
		if(sector)
			modeAct = 3;
		break;
	case 4:		// star mode
		star = parent->FindParentStarInEnv();
		if(star)
			modeAct = 4;
		break;
	}

	// Without a mode reference object we cannot calculate any other flight instrumentation readings.
	if(!modeAct)
	{
		altitude = 0;
		pitch = 0;
		roll = 0;
		heading = 0;
		longitude = 0;
		latitude = 0;
		return 0;
	}

	// Longitude, latitude, altitude, pitch, roll and heading.
	AuMatrix4x4 tp;
	AuPoint vup;
	double rz = 0;
	if(modeAct == 1)
	{
		// Planet mode.
		// Calculate altitude above mean sea level.
		GetTransformRelativeToParent(tp, planet);
		AuPoint floc = tp.Project(centerOffset);
		altitude = planet->AltitudeMSL(floc);	// Altitude above sea level.

		// Calculate exact longitude and latitude angles.
		// Negative longitude is west.
		// Negative latitude is south.
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
		if(llworld)
		{
			AuGlobeLLF ll;
			if(llworld->LLOfLoc(ll, floc))
			{
				latitude = ll.Latitude();
				longitude = ll.Longitude();
			}
			else
			{
				latitude = 0;
				longitude = 0;
			}
		}
		else
		{
			latitude = 0;
			longitude = 0;
		}

		// Calculate the ship's direction in planet space.
		AuPoint vdir = tp.ProjectVector(AuPoint(0,1,0));	// Direction vector in planet space.
		
		// Calculate heading.
		AuPoint vnorth, veast;
		if(planet->CompassVectors(floc, vnorth, veast, vup))
		{
			// Calculate the heading using compass vectors.
			double rx = vdir.Dot(veast);
			double ry = vdir.Dot(vnorth);
			rz = vdir.Dot(vup);
			heading = PI/2. - AuPoint(rx, ry).ArcTangent_2D();
		}
		else
			heading = 0.;
	}
	else if(modeAct == 4)
	{
		// Star mode.
		// Calculate altitude above mean sea level.
		GetTransformRelativeToParent(tp, star);
		AuPoint floc = tp.Project(centerOffset);
		altitude = floc.Length() - star->Radius();	// Altitude above photosphere.

		// Calculate exact longitude and latitude angles.
		// Negative longitude is west.
		// Negative latitude is south.
		latitude = floc.PolarIncline();
		longitude = floc.ArcTangent_2D();

		// Calculate the ship's direction in planet space.
		AuPoint vdir = tp.ProjectVector(AuPoint(0,1,0));	// Direction vector in star space.
		
		// Calculate the heading using compass vectors.
		AuPoint vnorth, veast;
		star->CompassVectors(floc, vnorth, veast, vup);
		double rx = vdir.Dot(veast);
		double ry = vdir.Dot(vnorth);
		rz = vdir.Dot(vup);
		heading = PI/2. - AuPoint(rx, ry).ArcTangent_2D();
	}
	else
	{
		// Longitude and latitude are not relevant to system or sector modes.
		latitude = 0;
		longitude = 0;

		if(modeAct == 2)
		{
			// System mode.
			// Calculate altitude above the plane of the ecliptic.
			tp = EnvTransform();
		}
		else
		{
			// Sector mode.
			// Calculate altitude above the plane of the ecliptic.
			const DaSolarSystem* system = parent->FindParentSolarSystemInEnv();
			if(!system)
			{
				altitude = 0;
				pitch = 0;
				roll = 0;
				heading = 0;
				longitude = 0;
				latitude = 0;
				return -1;	// This should not happen.
			}
			tp = EnvTransform();

			AuMatrix4x4 sysRot;
			sysRot.Rotate(system->Rotation());
			tp = sysRot * tp;
		}
		AuPoint floc = tp.Project(centerOffset);
		altitude = floc.z;

		// Calculate pitch, roll and heading.
		// Up vector is relative to the plane of the ecliptic of either the system or the galaxy.
		vup = AuPoint(0,0,1);

		// Determine north direction vector.
		AuPoint vnorth(0,1,0);

		// Calculate east vector from north and up vectors.
		AuPoint veast(1,0,0);

		// Calculate the ship's direction in space.
		AuPoint vdir = tp.ProjectVector(AuPoint(0,1,0));	// Direction vector in system or sector space.
		
		// Calculate the heading using all these great vectors.
		double rx = vdir.Dot(veast);
		double ry = vdir.Dot(vnorth);
		rz = vdir.Dot(vup);
		heading = PI/2. - AuPoint(rx, ry).ArcTangent_2D();
	}

	// Normalize heading.
	while(heading < 0.)
		heading += PI*2.;
	while(heading > PI*2.)
		heading -= PI*2.;

	// Calculate pitch.
	if(rz >= 1)
		pitch = PI/2;
	else if(rz <= -1)
		pitch = -PI/2;
	else
		pitch = -asin(rz);

	// Calculate roll from my up vector.
	AuPoint myvup = tp.UnProjectVector(vup);					// Up vector in my space.
	roll = PI/2.0 - AuPoint(myvup.x, myvup.z).ArcTangent_2D();
	if(roll < -PI)
		roll += PI*2;
	else if(roll > PI)
		roll -= PI*2;

	return modeAct;
}

// Returns the audio gain applied due to obstructions/vacuum between this atom
// and the scene atom.
// If this atom is the scene atom, 0 is returned when in a vacuum.
float DaAtom::RelativeAudioGain(const DaPerson* sceneAtom) const
{
	// Atoms that do not query their environment cannot answer the IsInVacuum question.
	if(!DoesQueryEnvironment())
	{
		if(sceneAtom->HelmetClosed())
			return .5f;
		return 1;
	}

	// The scene atom always hears himself well, even in a vacuum. Total silence is boring.
	if(sceneAtom == this)
	{
		if(sceneAtom->HelmetClosed())
			return .5f;
		if(IsInVacuum())
			return 0;
		return 1;
	}

	// Can't hear others in a vacuum.
	if(IsInVacuum())
		return 0;

	const DaAtom* mypar = Parent();
	if(!mypar)
		return 0;
	const DaAtom* sparent = sceneAtom->Parent();
	if(!sparent)
		return 0;

	// This atom is not the scene atom.
	// Find our respective ship and vehicle atoms.
	const DaCapsule* myship;
	if(IsInCapsule())
		myship = mypar->FindParentCapsuleInEnv();
	else
		myship = 0;
	const DaCapsule* personship;
	if(sceneAtom->IsInCapsule())
		personship = sparent->FindParentCapsuleInEnv();
	else
		personship = 0;

	const DaVehicle* mycar;
	if(IsInVehicle())
		mycar = dynamic_cast<const DaVehicle*>(mypar);
	else
		mycar = 0;

	const DaVehicle* personcar;
	if(sceneAtom->IsInVehicle())
		personcar = dynamic_cast<const DaVehicle*>(sparent);
	else
		personcar = 0;

	float gain = 1;
	if(myship && personship)
	{
		// Both of us are aboard spacecraft. The same one?
		if(myship != personship)
		{
			// Not the same ship.
			const DaAtom* msparent = myship->Parent();
			if(!msparent)
				gain = 0;
			else if(msparent->FindParentCapsuleInEnv() == personship ||
					sparent->FindParentCapsuleInEnv() == myship)
				gain = .2f;		// One of our two ships is subordinate to the other.
			else if(myship->IsInVacuum() || personship->IsInVacuum())
				gain = 0;		// One of our two ships is in vacuum.
			else
				gain = .075f;	// Neither ship is in vacuum and neither is subordinate to the other.
		}
	}
	else if(personship)
	{
		// Scene person is aboard a spacecraft but I am not.
		// I am not in a vacuum, as checked above.
		// Is the person's ship in a vacuum? It could be sitting outside a pressurized building in which I am standing.
		if(personship->IsInVacuum())
			gain = 0;			// ship holding scene person is in vacuum
		else
			gain = .15f;		// ship holding scene person is in air
	}
	else if(myship)
	{
		// I am aboard a spacecraft but the scene person is not.
		if(myship->IsInVacuum())
			gain = 0;			// ship holding me is in vacuum
		else
			gain = .15f;		// ship holding me is in air
	}
	else if(mycar && personcar)
	{
		// Both of us are inside enclosed vehicles. The same one?
		if(mycar != personcar)
		{
			// Not the same vehicle.
			if(mycar->IsInVacuum() || personcar->IsInVacuum())
				gain = 0;		// One of our two vehicles is in vacuum.
			else
				gain = .1f;		// Neither vehicle is in vacuum.
		}
	}
	else if(personcar)
	{
		// Scene person is inside an enclosed vehicle but I am not.
		// I am not in a vacuum, as checked above.
		// Is the person's vehicle in a vacuum? It could be sitting outside
		// a pressurized building in which I am standing.
		if(personcar->IsInVacuum())
			gain = 0;			// vehicle holding scene person is in vacuum
		else
			gain = .2f;			// vehicle holding scene person is in air
	}
	else if(mycar)
	{
		// I am inside an enclosed vehicle but the scene person is not.
		if(mycar->IsInVacuum())
			gain = 0;			// vehicle holding me is in vacuum
		else
			gain = .15f;		// vehicle holding me is in air
	}
	else
	{
		// Neither of us is aboard a spacecraft or inside an enclosed vehicle.
		// What about buildings?
		if(sceneAtom->IsInsideBd() && IsInsideBd())
		{
			// Both of us are in a building. The same one?
			if(sceneAtom->InIndustryLL() != InIndustryLL())
				gain = .05f;	// We are in separate buildings.
		}
		else if(sceneAtom->IsInsideBd())
			gain = .10f;		// Scene person is in a building but I am not. I am not in a vacuum, as checked above.
		else if(IsInsideBd())
			gain = .10f;		// I am in a building but the scene person is not.
	}
	if(gain > 0 && sceneAtom->HelmetClosed())
		gain *= .5f;			// Scene atom's helmet muffles all exterior sounds.
	return gain;
}

// Returns true if this atom appears to be in transition between systems. Client shows shooting stars wait animation when this
// returns true scene atom. Recurses upward if parent is a spaceship or a vehicle.
bool DaAtom::IsInTransition() const
{
	// Any client-side atom that is not normal must be in some kind of transition.
	if(!IsNormal())
		return true;

	// A client-side atom with no parent is just plain broken.
	const DaAtom* parent = Parent();
	if(!parent)
		return true;

	const DaStar* star = dynamic_cast<const DaStar*>(parent);
	if(star)
	{
		// Transitioning atoms are placed in the center of stars for safe keeping.
		if(Location().Length() < 10 && !InDesignEnvironment())
			return true;				// We're in the special nougat.
	}
	else
	{
		// Parent vehicle must answer the question.
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(parent);
		if(vehicle)
			return vehicle->IsInTransition();					// Recurse upward in case vehicle is transiting.

		// Parent creature must answer the question.
		const DaCreature* creature = dynamic_cast<const DaCreature*>(parent);
		if(creature)
			return creature->IsInTransition();					// Recurse upward in case creature is transiting.

		const DaVessel* ship = parent->FindParentVessel();
		if(ship)
		{
			if(ship->IsInTransition())						// Recurse upward in case ship is transiting.
				return true;
			const DaAtom* shipParent = ship->Parent();
			if(!shipParent)
				return true;
			const DaVessel* shipship = shipParent->FindParentVessel();
			if(shipship && shipship->IsInTransition())
				return true;
		}
	}
	return false;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAtom::FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestDefenseBaseTargetRecursive(gunnerDomain, gunnerWorldId, gunnerEmpire, gunSysLoc, gunRange, targetAtom,
			targetRange, targetSysLoc);
}


// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAtom::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
			gunSysVec, targetAtom, targetRange, targetSysLoc);
}

void DaAtom::FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, const DaAtom*& targetAtom,
		double& targetRange, AuPoint& targetSysLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestShipWeaponTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, shipSysLoc,
			gunRange, targetAtom, targetRange, targetSysLoc);
}

void DaAtom::FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc, float gunRange,
		const DaAtom*& targetAtom, double& targetRange) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestHarvesterTargetRecursive(harvesterModule, shipSysLoc, gunRange, targetAtom, targetRange);
}

void DaAtom::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestShipThreatRecursive(shipId, shipDomain, shipWorldId, shipEmpire, shieldPolicy, shipSysLoc, sensorRange,
			targetAtom, targetRange);
}

void DaAtom::FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
		const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
		const DaVessel*& enemyVessel, double& vesselRange,
		const DaVessel*& nonEnemyStation, double& stationRange,
		const DaBd*& enemyMilBd, double& milRange,
		const DaBd*& anyBd, double& bdRange) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.FindClosestShipLockOnRecursive(scannerDomain, scannerWorldId, scannerEmpire, scannerCapsuleId, scannerSysLoc, sensorRange,
			enemyVessel, vesselRange, nonEnemyStation, stationRange, enemyMilBd, milRange, anyBd, bdRange);
}

// Beware that 'this' atom might be the 'sceneAtom'.
void DaAtom::SceneRenderParameters(const DaAtom* sceneAtom, float& ambientR, float& ambientG, float& ambientB, float& fogDensity,
		float& fogR, float& fogG, float& fogB, bool viewIsTopDown, bool& sunsBlocked) const
{
	if(!DoesQueryEnvironment())
	{
		// This is a planet or something does not query its environment, probably to render a special effect. Get the info from
		// the scene atom; it will be close enough.
		Q_ASSERT(sceneAtom);	// In this case, we MUST have a scene atom.
		sceneAtom->SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, viewIsTopDown,
				sunsBlocked);
		sunsBlocked = false;
		return;
	}

	const DaAtom* mypar = Parent();
	if(!mypar)
		return;

	const DaWorld* world;
	if(IsInAtmosphere())
	{
		world = mypar->FindParentPlanetInEnv();
		if(!world)
		{
			// Uh oh. Not on the planet? I am probably aboard a spacecraft.
			fogDensity = 0;
			fogR = 0;
			fogG = 0;
			fogB = 0;
			ambientR = 1;
			ambientG = 0;	// This is an error so we will show up bright purple.
			ambientB = 1;
			sunsBlocked = false;
		}
	}
	else
		world = 0;

	// My ambient lighting does not depend on the scene atom.
	float fillum = 0;
	if(IsInsideCapsule())
	{
		// I am inside a capsule.
		const DaCapsule* capsule = mypar->FindParentCapsuleInEnv();
		if(capsule)
			capsule->InteriorAmbientLight(CapsuleRoomName(), ambientR, ambientG, ambientB, sunsBlocked);
		else
		{
			ambientR = 0;
			ambientG = 0;
			ambientB = 0;
			sunsBlocked = false;
		}
	}
	else
	{
		// Ambient light starts with starlight.
		float starLight;
		const DaSolarSystem* pss = mypar->FindParentSolarSystemInEnv();
		if(pss)
			starLight = pss->StarLight();							// Actual starlight level based on local star density.
		else
			starLight = 0.2f;	// Designer preview?
		fillum = starLight;
		ambientR = fillum;
		ambientG = fillum;
		ambientB = fillum;
		if(IsInAtmosphere())
		{
			// I am inside atmosphere range of a planet, even if it doesn't have any atmosphere density.
			//if(sunLight < 0)
			//{
				QueryEnvironmentLighting();
				float sunLight = Illumination();
			//}
			fillum = starLight + sunLight;
			ambientR = fillum;
			ambientG = fillum;
			ambientB = fillum;

			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(world);
			if(llworld)
			{
				float cityAmbientLight;
				if(BdId().IsNull() || InIndustry() == IID_Road)
					cityAmbientLight = llworld->CityAmbientLight(GlobeLL(), WORLDTEX_Count);
				else
					cityAmbientLight = llworld->CityAmbientLight(GlobeLL(), WORLDTEX_DirtSmooth);
				//if(Is(16))
					//qDebug() << "DaAtom::SceneRenderParameters" << Id() << Name() << "cityAmbientLight" << cityAmbientLight;
				if(cityAmbientLight > 0.f)
				{
					const QColor& c = AuGLLight::ColorSodiumVapor();
					ambientR += cityAmbientLight * c.redF() * AuGLLight::AmbientLitCity();
					ambientG += cityAmbientLight * c.greenF() * AuGLLight::AmbientLitCity();
					ambientB += cityAmbientLight * c.blueF() * AuGLLight::AmbientLitCity();
				}
			}

			if(ambientR < 0)
				ambientR = 0;
			else if(ambientR > 1)
				ambientR = 1;

			if(ambientG < 0)
				ambientG = 0;
			else if(ambientG > 1)
				ambientG = 1;

			if(ambientB < 0)
				ambientB = 0;
			else if(ambientB > 1)
				ambientB = 1;

			if(IsUnderGround())
				sunsBlocked = AltitudeAGL() < -1;
			else
				sunsBlocked = false;
		}
		else
			sunsBlocked = false;
	}

	// We cannot calculate fog without the scene atom.
	// We do not apply fog when in top down view.
	if(!sceneAtom || viewIsTopDown)
	{
		// DaSc sometimes calls me with no scene atom, just to get external ambient lighting. See DaSc::InteriorAmbientLight.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
		return;
	}

	// Fog depends on the atmospheric conditions separating me and the scene atom.
	if(IsInsideCapsule() && sceneAtom->IsInsideCapsule() &&
			(this == sceneAtom || dynamic_cast<const DaCapsule*>(CommonParent(sceneAtom, FindEnvironmentAtom()))))
	{
		// We are both inside the same capsule.
		QColor colAtm(255,253,250);
		fogDensity = sceneAtom->AirDensity()/1000.f;
		fogR = colAtm.redF() * ambientR;
		fogG = colAtm.greenF() * ambientG;
		fogB = colAtm.blueF() * ambientB;
		return;
	}
	else if(sceneAtom->IsInsideSpacecraft() && FindIdRecursive(sceneAtom->Id()))
	{
		// Scene atom is aboard this ship at some level.
		// If we fog their ship, they see the distant outside parts of the ship as fogged when they look out the window.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
		return;
	}

	if(!IsInAtmosphere() || !world)
	{
		// I am not inside atmosphere range of any planet and not inside a spacecraft.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
		return;
	}

	if(this != sceneAtom && ParentId() == sceneAtom->ParentId() && (IsInVehicle() || IsRider()))
	{
		// We are riding in the same vehicle or on the same mount.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
		return;
	}

	// I am inside the atmosphere radius of a world.
	if(world->Hydrographics())
	{
		// The world has seas. Are we separated by water?
		bool seaFogMe;
		if(!sceneAtom->IsInAtmosphere())
		{
			// Scene atom is outside the atmosphere. Fog makes me turn white.
			fogDensity = 0;
			fogR = 0;
			fogG = 0;
			fogB = 0;
			return;
		}
		else if(sceneAtom->IsEyeUnderWater() && !sceneAtom->IsInsideBd() && !sceneAtom->IsInsideEnclosure())
		{
			// Viewer is below the sea surface and they are not inside a building or enclosure. They are viewing their surroundings
			// through water.
			seaFogMe = true;			// Viewer sees me through water, unless we are both in the same undersea city.
		}
		else
		{
			// Viewer is above the sea surface or they are inside a building or enclosure.
			seaFogMe = false;			// Viewer sees me clearly.
		}

		if(seaFogMe)
		{
			// Setup atmospheric fog to present underwater environment.
			QColor colSea = world->SeaColor();
			fogDensity = world->SeaFogDensity();
			fogR = colSea.redF() * fillum;
			fogG = colSea.greenF() * fillum;
			fogB = colSea.blueF() * fillum;
			return;
		}
	}

	// I am inside an atmosphere and not separated by sea water.
	// Calculate atmospheric fog.
	float atmDensity = sceneAtom->AtmosphereDensity();
	if(atmDensity > 0 && sceneAtom->IsInAtmosphere())
	{
		// Setup atmospheric fog based on atmosphere density.
		QColor colAtm = ColorOfAtmosphere();
		if(world->IsGasGiant() && sceneAtom->IsEyeUnderWater())
			fogDensity = world->SeaFogDensity();
		else
			fogDensity = atmDensity/1000.f;
		fogR = colAtm.redF() * fillum;
		fogG = colAtm.greenF() * fillum;
		fogB = colAtm.blueF() * fillum;
	}
	else
	{
		// I am inside atmosphere range but there is no atmosphere density.
		fogDensity = 0;
		fogR = 0;
		fogG = 0;
		fogB = 0;
	}
}

const DaAtom* DaAtom::FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
		quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
		double& bestDistance) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Skip subordinate environment atoms, such as arenas and design studios.
		if((*it)->IsInstanceUniverse())
			continue;

		bestAtom = (*it)->FindClosestMissileTarget(launcherId, missileDomain, missileWorldId, missileEmpireId, missileSysLoc,
				bestAtom, bestSysLoc, bestDistance);
	}
	return bestAtom;
}

bool DaAtom::InArena() const
{
	bool v;
	m_EnvironLock.lockForRead();
	if(m_Environ)
	{
		v = (m_Environ->m_Environment & EB_Arena) ? true : false;
		m_EnvironLock.unlock();
	}
	else
	{
		m_EnvironLock.unlock();
		const DaAtom* parent = Parent();
		if(parent)
			v = parent->FindParentArena();
		else
			v = false;
	}
	return v;
}

// In an arena instance when/where engines cannot be used.
bool DaAtom::InArenaAsObserver() const
{
	if(!InArena())
		return false;

	const DaAtom* mypar = Parent();
	if(!mypar)
		return false;
	const DaArena* arena = mypar->FindParentArena();
	if(!arena)
		return false;	// Not in an arena at all.

	return EmpireId() == DgEmpire::ArenaObserverId();
}

// In an arena instance when/where engines cannot be used.
bool DaAtom::InArenaCannotAccelerate() const
{
	if(!InArena())
		return false;

	const DaAtom* mypar = Parent();
	if(!mypar)
		return false;
	const DaArena* arena = mypar->FindParentArena();
	if(!arena)
		return false;	// Not in an arena at all.

	// Observers can always move around.
	if(EmpireId() == DgEmpire::ArenaObserverId())
		return false;

	// Arena must be in Play state to fire weapons.
	if(arena->MatchState() == DaArena::Join)
		return true;	// In an arena that is joining players.

	return false;
}

// In an arena instance when/where weapons cannot be used.
bool DaAtom::InArenaCannotFireWeapons() const
{
	if(!InArena())
		return false;

	// Observers cannot fire weapons at all.
	if(EmpireId() == DgEmpire::ArenaObserverId())
		return true;

	const DaAtom* parent = Parent();
	if(!parent)
		return false;
	const DaArena* arena = parent->FindParentArena();
	if(!arena)
		return false;	// Not in an arena at all.

	// Arena must be in Play state to fire weapons.
	if(arena->MatchState() != DaArena::Play)
		return true;	// In an arena that is joining players or in aftermath.

	// Cannot fire weapons when outside the ring.
	return EnvLocation().Length2() > c_ArenaRingRadius*c_ArenaRingRadius;
}

bool DaAtom::MustNotTickTock() const
{
	if(!(g_Au.m_DebugSwitches & 0x01000000))
	{
		if(IsTraveling() || IsDormant() || m_State == New || IsTrashOrInvalid() || !Ether() || ParentId().IsNull())
			return true;
		const DaAtom* parent = Parent();
		if(!parent || parent->IsTraveling())
			return true;
		if(!DoesQueryEnvironment())
			return false;
		if(EnvironmentQueryValid() && !IsInCapsule())
			return false;
		const DaCapsule* ship = parent->FindParentCapsuleInEnv();
		if(ship && !ship->IsDesignDataCacheValid())
			return true;
		return false;
	}
	else
	{
		AuElapsedTimer now(AuElapsedTimer::Now());
		if(IsTraveling() || IsDormant() || m_State == New || IsTrashOrInvalid() || !Ether() || ParentId().IsNull())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return true;
		}
		const DaAtom* parent = Parent();
		if(!parent || parent->IsTraveling())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return true;
		}
		if(!DoesQueryEnvironment())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return false;
		}
		if(EnvironmentQueryValid() && !IsInCapsule())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return false;
		}
		const DaCapsule* ship = parent->FindParentCapsuleInEnv();
		if(ship && !ship->IsDesignDataCacheValid())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return true;
		}
		if(now.elapsed() > 10000)
			qDebug() << "SLOW DaAtom::MustNotTick" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
		return false;
	}
}

// Returns a vector relative to this atom.
AuPoint DaAtom::Wind() const
{
	if(!DoesQueryEnvironment())
		return AuPoint(0,0);

	double windSec = (double)g_Au.CoordinatedTime().MSecsSinceEpoch() / 1000. + Id().GetID();
	double period = 10.;	// seconds per cycle
	double force = sin(windSec / period * PI*2.);
	if(force < 0)
		force /= 2.f;		// Spring back a little weaker.

	if(!IsInAtmosphere() || IsInsideCapsule() || IsInVehicle())
	{
		double windForce = force * AirDensity() / 10.;
		double windDir = Location().ZRotation() + PI/2.;
		return AuPoint(windForce, 0, 0).RotateZ(windDir);
	}

	const DaAtom* parent = Parent();
	if(!parent)
		return AuPoint(0,0);
	const DaWorld* world = parent->FindParentPlanetInEnv();
	if(!world)
		return AuPoint(0,0);

	// Environmental wind when not inside a building.
	const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
	if(rsworld)
	{
		// Inside atmosphere of a ringworld.
		QPointF wind = rsworld->WindAt(GlobeLL());
		if(wind.x() != 0.f || wind.y() != 0.f)
		{
			AuPoint w(wind.x(), wind.y());
			float windDir = w.ArcTangent_2D();
			float windForce = w.Length();	// miles per hour
			windForce = sqrtf(windForce / 100);
			if(windForce > 1)
				windForce = 1;

			// Calculate wind vector.
			AuPoint vnorth(0,0,1);
			AuPoint vup = -rsworld->GravityDir(Location());
			AuPoint veast = vnorth.Cross(vup);
			veast.Unit(windForce);			// Wind vec pointing east.
			AuMatrix4x4 tWind;
			tWind.RotateAxis(windDir, vup);
			AuPoint vwind = tWind.Project(veast);

			// Project wind vector into eye space for the shader.
			return Transform().UnProjectVector(vwind) * force;		// Into my space.
		}
	}
	else
	{
		// Inside atmosphere of a globe.
		const DaWorldSphere* glworld = dynamic_cast<const DaWorldSphere*>(world);
		if(glworld)
		{
			QPointF wind = glworld->WindAt(GlobeLL());
			if(wind.x() != 0.f || wind.y() != 0.f)
			{
				AuPoint w(wind.x(), wind.y());
				float windDir = w.ArcTangent_2D();
				float windForce = w.Length();	// miles per hour
				windForce = sqrtf(windForce / 100);
				if(windForce > 1)
					windForce = 1;

				// Calculate wind vector.
				AuPoint vnorth(0,0,1);
				AuPoint vup = -glworld->GravityDir(Location());
				AuPoint veast = vnorth.Cross(vup);
				veast.Unit(windForce);			// Wind vec pointing east.
				AuMatrix4x4 tWind;
				tWind.RotateAxis(windDir, vup);
				AuPoint vwind = tWind.Project(veast);

				// Project wind vector into eye space for the shader.
				return Transform().UnProjectVector(vwind) * force;		// Into my space.
			}
		}
	}

	return AuPoint(0,0);
}

// Fills 'places' with my id and the ids of the places up my parent chain to the universe.
void DaAtom::ChainOfPlaces(AuIdList& places) const
{
	const DaWorld* p = FindParentPlanetInEnv();					// This will find me if I am a planet.
	if(p)
		places.append(p->Id());

	const DaSolarSystem* ss = FindParentSolarSystem();			// This will be me if I am a solar system.
	if(ss)
		places.append(ss->Id());

	const DaSector* s = FindParentSector();						// This will be me if I am a sector.
	if(s)
		places.append(s->Id());

	const DaGalaxy* g = FindParentGalaxy();						// This will be me if I am a galaxy.
	if(g)
		places.append(g->Id());

	const DaUniverse* u = FindParentUniverse();					// This will be me if I am a universe.
	if(u)
		places.append(u->Id());
	else
		places.append(AuId::Universe());						// Client does not get the universe atom.
}

QString DaAtom::SpottedLocation() const
{
	if(!DoesQueryEnvironment())
		return "";
	const DaAtom* parent = Parent();
	if(!parent)
		return "";

	if(IsRider())
		return tr("%1 is riding %2. %3")
			.arg(Name())
			.arg(parent->Name())
			.arg(parent->SpottedLocation());

	if(IsInVehicle())
		return tr("%1 is in a %2. %3")
			.arg(Name())
			.arg(parent->Name())
			.arg(parent->SpottedLocation());

	if(IsInSpacecraft())
	{
		QString shipRoomName = ShipRoomName();
		return tr("%1 is in %2.")
			.arg(Name())
			.arg(shipRoomName);
	}

	if(IsInAtmosphere())
		return tr("%1 is at %2.")
			.arg(Name())
			.arg(GlobeLL().ToString());

	return tr("%1 is in space.")
		.arg(Name());
}

QString DaAtom::TargetDesc(const AuPoint& spottedLocation, quint32 spotterEmpire, AuId& targetId,
		POLITICAL_STANCE& stanceToTarget) const
{
	Q_UNUSED(spottedLocation);
	QString targetDesc = SpotDescription(spotterEmpire);		// This will be our return value.

	targetId = Id();

	// When a unit is sighted, we have a political stance.
	stanceToTarget = PoliticalStance(spotterEmpire, EmpireId());

	const DaAtom* parent = Parent();
	if(!parent)
		return "";
	if(DoesQueryEnvironment() && IsInSpacecraft() && parent)
	{
		const DaVessel* ship = parent->FindParentVesselInEnv();
		if(ship)
			targetDesc += " aboard " + ship->SpotDescription(spotterEmpire);
	}

	// Include target location information.
	if(InArena())
	{
		const DaArena* arena = parent->FindParentArena();
		if(arena)
			targetDesc += " in " + arena->Name();
	}
	else if(InDesignEnvironment())
	{
		QString sloc;
		const DaWorld* planet = parent->FindParentPlanetInEnv();
		if(IsInAtmosphere())
		{
			AuPoint loc;
			if(GetLocationRelativeToParent(loc, planet))
			{
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				if(llworld)
				{
					AuGlobeLL ll;
					if(llworld->LLOfLoc(ll, loc))
						sloc = ll.ToString() + " " + llworld->Name();
				}
			}
		}
		else if(planet)
		{
			// At a planet but not in the atmosphere.
			sloc = planet->Name();
		}
		else
		{
			// Not at a planet.
			const DaDesigner* de = parent->FindParentDesignerInEnv();
			if(de)
				sloc = de->Name();
		}
		if(!sloc.isEmpty())
			targetDesc += " at " + sloc;
	}
	else
	{
		QString sloc;
		const DaWorld* planet = parent->FindParentPlanetInEnv();
		const DaSector* sector = parent->FindParentSector();
		if(IsInAtmosphere())
		{
			AuPoint loc;
			if(GetLocationRelativeToParent(loc, planet))
			{
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				if(llworld)
				{
					AuGlobeLL ll;
					if(llworld->LLOfLoc(ll, loc))
						sloc = ll.ToString() + " " + llworld->Name() + ", " + sector->Name() + " " +
							sector->Description() + ", " + GalaxyName(Galaxy());
				}
			}
		}
		else if(planet)
		{
			// At a planet but not in the atmosphere.
			sloc = planet->Name() + ", " + sector->Name() + " " + sector->Description() + ", " + GalaxyName(Galaxy());
		}
		else
		{
			// Not at a planet.
			const DaSolarSystem* system = parent->FindParentSolarSystemInEnv();
			if(system && !system->IsEmptySystem())
				sloc = system->Name() + ", " + sector->Name() + " " + sector->Description() + ", " + GalaxyName(Galaxy());
		}
		if(!sloc.isEmpty())
			targetDesc += " at " + sloc;
	}

	return targetDesc;
}

float DaAtom::Buoyancy(const AuQueryEnvironment& qe) const
{
	float density = Density();
	if(density == 0)
		return 0;												// Neither ascend nor descend in water.
	if(DoesQueryEnvironment() && qe.InDesignEnvironment() && !qe.InDesignPreview())
		return 0;

	// Density of water is 62.316.
	float buoyancy = DensityOfWater() / density - 1;			// Buoyancy when fully submerged.
	if(buoyancy < 0)
		return buoyancy;										// This thing sinks.

	double altmsl = qe.AltitudeMSL();							// MSL altitude of my CG.
	if(altmsl > 0)
	{
		// My CG is above the water surface.
		double cght = GroundHeightAboveCG();
		if(altmsl >= cght)
			return -1;											// Just barely in the water or above it. Maximum gravity vector.
		return -altmsl / cght;									// Sink to my cg.
	}
	else
	{
		// My CG is below the water surface.
		if(density == DensityOfWater())
			return 0;											// Neither ascend nor descend in water.

		// Dampen in the last 1' top side to keep things from springing up out of the water.
		if(altmsl >= -1)
			return buoyancy * -altmsl;							// Buoyancy drops off at the surface.

		// Below the range of dampening at the surface.
		return buoyancy;										// Return maximum buoyancy.
	}
}

void DaAtom::SetControlled(const AuId& controller)
{
	if(controller.IsNull())
	{
		m_ControlLock.lockForWrite();
		m_Controller = AuId();
		m_tmControlled = AuElapsedTimer();
		m_ControlLock.unlock();
		ApplySync();					// The server applies any remaining syncs, to be sure I am where I'm supposed to be.
	}
	else
	{
		m_ControlLock.lockForWrite();
		m_Controller = controller;
		m_tmControlled.restart();
		m_ControlLock.unlock();
	}
}

bool DaAtom::IsClientControlled()
{
	bool clearC = false;
	m_ControlLock.lockForRead();
	bool is = !m_Controller.IsNull() && m_tmControlled.isValid();
	if(is && m_tmControlled.elapsed() > 1500)
	{
		// Turn off the controlled time so we stop calling elapsed(), in case the controller got left on.
		is = false;
		clearC = true;
	}
	m_ControlLock.unlock();
	if(clearC)
		SetControlled(AuId());
	return is;
}

AuPoint DaAtom::BaseLoc() const
{
	AuPoint loc = CGOffset();
	return Transform().Project(AuPoint(loc.x, loc.y, loc.z - GroundHeightAboveCG()));
}

bool DaAtom::InTransporterRoom() const
{
	if(!DoesQueryEnvironment())
		return false;
	if(!IsInsideCapsule())
		return false;

	// Check for my current room being a transporter room.
	const DaAtom* parent = Parent();
	if(!parent)
		return false;
	const DaCapsule* v = parent->FindParentCapsuleInEnv();
	if(!v)
		return false;

	return v->IsTransporterRoom(CapsuleRoomId());
}

quint16 DaAtom::AtTransporterWhich() const
{
	if(!DoesQueryEnvironment())
		return 0;
	if(!IsInsideCapsule())
		return 0;

	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	const DaCapsule* v = parent->FindParentCapsuleInEnv();
	if(!v)
		return 0;

	if(!v->IsTransporterRoom(CapsuleRoomId()))
		return 0;

	return v->TransporterWhich(*this);
}

bool DaAtom::GetRelativeLocation(const DaAtom& atom, AuPoint& loc) const
{
	AuMatrix4x4 ct;
	if(!GetRelativeTransform(atom, ct))
		return false;
	loc = ct.Translation();
	return true;
}

bool DaAtom::GetRelativeTransform(const DaAtom& atom, AuMatrix4x4& t) const
{
	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return false;
	DaAtom* cpar = CommonParent(&atom, FindEnvironmentAtom());
	if(!cpar)
		return false;
	AuMatrix4x4 mt;
	GetTransformRelativeToParent(mt, cpar);
	AuMatrix4x4 ot;
	atom.GetTransformRelativeToParent(ot, cpar);
	t = mt.Inverse() * ot;
	return true;
}

bool DaAtom::IsInLava() const
{
	bool inLava;
	const AuQueryEnvironment* qe = LockEnvironmentQuery();
	if(qe->IsInWater())
		inLava = qe->OrbitZone() == OZONE_Inferno &&
			qe->WorldAtomType() != ATOM_WorldGasGiant;
	else
		inLava = false;
	UnlockEnvironmentQuery();
	return inLava;
}

bool DaAtom::IsInSupercooledLiquid() const
{
	bool inSupercooledLiquid;
	const AuQueryEnvironment* qe = LockEnvironmentQuery();
	if(qe->IsInWater())
		inSupercooledLiquid = qe->OrbitZone() == OZONE_Frigid ||
			qe->WorldAtomType() == ATOM_WorldGasGiant;
	else
		inSupercooledLiquid = false;
	UnlockEnvironmentQuery();
	return inSupercooledLiquid;
}

void DaAtom::ListOccupiedSurgeryUnits(QSet<quint16>& consoles) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCreature* pAtom = dynamic_cast<DaCreature*>(*it);
		if(!pAtom)
			continue;
		quint16 stationId = pAtom->StationId();
		if(!stationId)
			continue;
		if(!AuStation::IsPatient(pAtom->ConsoleType()))
			continue;				// Skip non-patients.
		consoles.insert(stationId);
	}
}

bool DaAtom::HasConsoleOperator(quint16 stationId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->StationId() != stationId)
			continue;
		if(pAtom->ConsoleSeatIndex() != 0)
			continue;	// On new surgery station, station index 0 is the doctor; 1 is the patient.
		return true;
	}
	return false;
}

void DaAtom::ListDestinationConsoles(QSet<quint16>& consoles) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* citizen = dynamic_cast<DaSentient*>(*it);
		if(!citizen)
			continue;
		if(!citizen->IsGoingToConsole())
			continue;
		consoles.insert(citizen->OrderStationId());	// Found citizen going to a console, turret, surgery unit, or troop station.
	}
}

bool DaAtom::IsConsoleSeatOccupied(quint16 stationId, unsigned int seatIndex) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaCreature* creature = dynamic_cast<const DaCreature*>(*it);
		if(!creature)
			continue;
		if(creature->StationId() != stationId)
			continue;
		if(creature->ConsoleSeatIndex() != seatIndex)
			continue;
		return true;
	}
	return false;
}

// Test for console occupied by anyone, including people and ai controlled citizens.
bool DaAtom::IsConsoleOccupied(quint16 stationId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaCreature* creature = dynamic_cast<const DaCreature*>(*it);
		if(!creature)
			continue;
		if(creature->StationId() != stationId)
			continue;
		return true;
	}
	return false;
}

// List the locations of consoles and turrets occupied by anyone who is not invisible, including AI controlled citizens.
void DaAtom::ListVisiblyOccupiedConsoles(const DaPerson* sceneAtom, QSet<quint16>& consoles) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCreature* pAtom = dynamic_cast<DaCreature*>(*it);
		if(!pAtom)
			continue;
		if(pAtom != sceneAtom && pAtom->AgentInvisible())
			continue;				// The atom is a person who is invisible.
		quint16 stationId = pAtom->StationId();
		if(!stationId)
			continue;
		if(pAtom->ConsoleSeatIndex() != 0)
			continue;				// Skip non-zero seat index, for medic station. 1 is the patient.
		consoles.insert(stationId);
	}
}

// Test for console manned by a non-AI controlled person.
bool DaAtom::IsConsoleManned(quint16 stationId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsAIControlled())
			continue;
		if(pAtom->StationId() != stationId)
			continue;
		return true;
	}
	return false;
}

bool DaAtom::IsConsoleSeatManned(quint16 stationId, unsigned int seatIndex) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsAIControlled())
			continue;
		if(pAtom->StationId() != stationId)
			continue;
		if(pAtom->ConsoleSeatIndex() != seatIndex)
			continue;
		return true;
	}
	return false;
}

void DaAtom::SetRenderLights(AuGLEffects& re, const QList<AuGLSun>& suns, const QList<AuGLLight>& lights, float ambientLight)
{
	// Configure ambient lighting.
	re.SetAmbientLight(ambientLight, ambientLight, ambientLight, 1.f);

	// We never have more than three suns so we can configure all of them.
	Q_ASSERT(suns.size() <= 3);
	re.DisableSuns();
	int isun = 0;
	for(QList<AuGLSun>::const_iterator si = suns.cbegin(); si != suns.cend(); si++, isun++)
		re.SetSun(isun, *si);

	// Point light sources can easily exceed the capabilities of our shaders, currently maxed at 4 point light sources.
	re.DisableLights();
	if((unsigned int)lights.size() > re.MaxLights())
	{
		// Pick the best point light sources.
		QList<AuGLLight> bestLights;
		QMap<double, int> ships;		// Key is scene distance.
		QMap<double, int> hengeFires;	// Key is scene distance.
		QMap<double, int> other;		// Key is scene distance.
		int ilight = 0;
		for(QList<AuGLLight>::const_iterator li = lights.cbegin(); li != lights.cend(); li++, ilight++)
		{
			switch((*li).m_Role)
			{
			case AuGLLight::Unknown:
				break;
			case AuGLLight::ScenePerson:
			case AuGLLight::SceneVehicle:
			case AuGLLight::SceneSpacecraft:
				bestLights.append(*li);	// There could only be one of each of these in the list.
				break;
			case AuGLLight::Person:
				{
					double dist = (*li).EyeDistance();
					if(dist < 1000*FeetPerMeter())
						other.insert(dist, ilight);
				}
				break;
			case AuGLLight::Vehicle:
				{
					double dist = (*li).EyeDistance();
					if(dist < 2000*FeetPerMeter())
						other.insert(dist, ilight);
				}
				break;
			case AuGLLight::HengeFire:
				{
					double dist = (*li).EyeDistance();
					if(dist < 5000*FeetPerMeter())
						hengeFires.insert(dist, ilight);
				}
				break;
			case AuGLLight::Spacecraft:
				{
					double dist = (*li).EyeDistance();
					if(dist < 5000*FeetPerMeter())
						ships.insert(dist, ilight);
				}
				break;
			case AuGLLight::Item:
			case AuGLLight::Effect:
				{
					double dist = (*li).EyeDistance();
					if(dist < 1000*FeetPerMeter())
						other.insert(dist, ilight);
				}
				break;
			}
		}
		// Add lights to the list of best lights until it is full.
		if(!ships.empty() && (unsigned int)bestLights.size() < re.MaxLights())
		{
			// The light of the closest other spacecraft gets a high priority. These are usually very bright.
			QMap<double, int>::iterator ii = ships.begin();
			bestLights.append(lights.at(*ii));
			ships.erase(ii);
		}
		if(!hengeFires.empty() && (unsigned int)bestLights.size() < re.MaxLights())
		{
			// The light of the closest henge fire gets a high priority. These aren't bright but people search for them.
			QMap<double, int>::iterator ii = hengeFires.begin();
			bestLights.append(lights.at(*ii));
			hengeFires.erase(ii);
		}
		// Add lights of the closest other people, vehicles and items, from closest to farthest.
		for(QMap<double, int>::const_iterator ii = other.cbegin(); ii != other.cend() &&
				(unsigned int)bestLights.size() < re.MaxLights(); ii++)
			bestLights.append(lights.at(*ii));
		// Add lights of the remaining other spacecraft, if any, from closest to farthest.
		for(QMap<double, int>::const_iterator ii = ships.cbegin(); ii != ships.cend() &&
				(unsigned int)bestLights.size() < re.MaxLights(); ii++)
			bestLights.append(lights.at(*ii));
		// Add lights of the remaining other henge fires, if any, from closest to farthest.
		for(QMap<double, int>::const_iterator ii = hengeFires.cbegin(); ii != hengeFires.cend() &&
				(unsigned int)bestLights.size() < re.MaxLights(); ii++)
			bestLights.append(lights.at(*ii));

		// Set up the best point light sources we can handle.
		ilight = 0;
		for(QList<AuGLLight>::const_iterator li = bestLights.cbegin(); li != bestLights.cend(); li++, ilight++)
			re.SetLight(ilight, *li);
	}
	else
	{
		// Set up all the point light sources; we can handle all of them.
		int ilight = 0;
		for(QList<AuGLLight>::const_iterator li = lights.cbegin(); li != lights.cend(); li++, ilight++)
			re.SetLight(ilight, *li);
	}
}

void DaAtom::TickTock() const
{
	if(!(g_Au.m_DebugSwitches & 0x01000000))
	{
		if(MustNotTickTock())
		{
			g_Au.ReleaseTickTock(Id());
			return;
		}

		// Check to see if I need high priority Tick processing, fast enough for movement physics.
		const DaSolarSystem* system = 0;
		if(IsTicker())
		{
			system = FindParentSolarSystem();
			if(system && system->IsTickingContents())
				g_Au.UpdateOrAddTicker(Id(), AtomType(), system->Id());
			else
				g_Au.ReleaseTicker(Id());
		}
		else
			g_Au.ReleaseTicker(Id());

		// Check to see if I need lower priority Tock processing, on my desired interval.
		unsigned int desiredCycleMs = IsTocker();
		if(desiredCycleMs)
		{
			if(!system)
				system = FindParentSolarSystem();
			if(system && system->IsTockingContents())
				g_Au.UpdateOrAddTocker(Id(), AtomType(), system->Id(), desiredCycleMs);
			else
				g_Au.ReleaseTocker(Id());
		}
		else
			g_Au.ReleaseTocker(Id());
	}
	else
	{
		AuElapsedTimer now(AuElapsedTimer::Now());
		if(MustNotTickTock())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			g_Au.ReleaseTickTock(Id());
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			return;
		}

		// Check to see if I need high priority Tick processing, fast enough for movement physics.
		if(now.elapsed() > 10000)
			qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
		const DaSolarSystem* system = 0;
		if(IsTicker())
		{
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			system = FindParentSolarSystem();
			if(system && system->IsTickingContents())
			{
				g_Au.UpdateOrAddTicker(Id(), AtomType(), system->Id());
				if(now.elapsed() > 10000)
					qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			}
			else
			{
				g_Au.ReleaseTicker(Id());
				if(now.elapsed() > 10000)
					qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			}
		}
		else
		{
			g_Au.ReleaseTicker(Id());
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
		}

		// Check to see if I need lower priority Tock processing, on my desired interval.
		if(now.elapsed() > 10000)
			qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
		unsigned int desiredCycleMs = IsTocker();
		if(desiredCycleMs)
		{
			if(!system)
				system = FindParentSolarSystem();
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			if(system && system->IsTockingContents())
			{
				g_Au.UpdateOrAddTocker(Id(), AtomType(), system->Id(), desiredCycleMs);
				if(now.elapsed() > 10000)
					qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			}
			else
			{
				g_Au.ReleaseTocker(Id());
				if(now.elapsed() > 10000)
					qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
			}
		}
		else
		{
			g_Au.ReleaseTocker(Id());
			if(now.elapsed() > 10000)
				qDebug() << "SLOW DaAtom::TickTock" << *this << now.elapsed() << "ms in" << __FILE__ << ":line" << __LINE__;
		}
	}
}

void DaAtom::TickTockRecursive() const
{
	TickTock();
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->TickTockRecursive();
}

bool DaAtom::OkToTick(const AuId& inSystemId) const
{
	if(MustNotTickTock())
		return false;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return false;
	return system->Is(inSystemId);
}

bool DaAtom::OkToTock(const AuId& inSystemId) const
{
	if(MustNotTickTock())
		return false;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return false;
	return system->Is(inSystemId);
}

// Queues a call to CombatApplyDamage to occur during Tick.
void DaAtom::CombatApplyDamageLater(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, const AuCombatResult& result)
{
	AuDamageToApply damage(attacker, weapon, wpnLoc, hitLoc, result);
	m_DamageToApplyLock.lockForWrite();
	m_DamageToApply.append(damage);
	if(m_DamageToApply.size() > 30)
	{
		qDebug() << "DaAtom::CombatApplyDamageLater" << Id() << Name() << "attacker" << attacker.m_Name << "weapon"
			<< g_Au.CommodityName(weapon.m_CID) << "Warning: lots of damage to apply later" << m_DamageToApply.size() << "items";
	}
	m_DamageToApplyLock.unlock();
}

DaBd* DaAtom::FindBd(const AuId& bdId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	return dynamic_cast<DaBd*>(subs.FindId(bdId));
}

void DaAtom::SetInMotion()
{
	if(CanSetInMotion())
	{
		m_PosLock.lockForWrite();
		SetAtRestState(false);
		m_PosChange.Set();	// So I'll stay in motion for a second.
		m_PosLock.unlock();
	}
}

void DaAtom::ListDesignsRecursive(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns) const
{
	DaAtomList subs;
	ListSubatoms(subs);
	subs.ListDesignsRecursive(bdDesigns, scDesigns);
}

void DaAtom::RenderSc(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog, RENDERPASS pass)
{
	if(!shadSc.empty())
	{
		ari.re->GLSL().ScShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadSc.cbegin(); ai != shadSc.cend(); ai++)
			(*ai)->RenderSc(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScShader().Deselect();
		shadSc.clear();
	}

	if(!shadScFog.empty())
	{
		ari.re->GLSL().ScFogShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadScFog.cbegin(); ai != shadScFog.cend(); ai++)
			(*ai)->RenderScFog(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScFogShader().Deselect();
		shadScFog.clear();
	}

	if(!shadScGlobe.empty())
	{
		ari.re->GLSL().ScGlobeShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadScGlobe.cbegin(); ai != shadScGlobe.cend(); ai++)
			(*ai)->RenderScGlobe(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScGlobeShader().Deselect();
		shadScGlobe.clear();
	}

	if(!shadScGlobeFog.empty())
	{
		ari.re->GLSL().ScGlobeFogShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadScGlobeFog.cbegin(); ai != shadScGlobeFog.cend(); ai++)
			(*ai)->RenderScGlobeFog(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScGlobeFogShader().Deselect();
		shadScGlobeFog.clear();
	}

	if(!shadScRingworld.empty())
	{
		ari.re->GLSL().ScRingworldShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadScRingworld.cbegin(); ai != shadScRingworld.cend(); ai++)
			(*ai)->RenderScRingworld(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScRingworldShader().Deselect();
		shadScRingworld.clear();
	}

	if(!shadScRingworldFog.empty())
	{
		ari.re->GLSL().ScRingworldFogShader().Select(projection, *ari.re);
		for(DaAtomList::const_iterator ai = shadScRingworldFog.cbegin(); ai != shadScRingworldFog.cend(); ai++)
			(*ai)->RenderScRingworldFog(ari, rmod, sceneAtom, pass);
		ari.re->GLSL().ScRingworldFogShader().Deselect();
		shadScRingworldFog.clear();
	}
}

quint64 DaAtom::MemoryUsage() const
{
	quint64 mem = sizeof(DaAtom);
	mem += sizeof(AuDamageToApply) * m_DamageToApply.size();
	m_EffectsLock.lockForRead();
	if(m_Effects)
		mem += m_Effects->MemoryUsage();
	m_EffectsLock.unlock();

	m_EnvironLock.lockForRead();
	if(m_Environ)
		mem += m_Environ->MemoryUsage();
	mem += sizeof(m_EnvVector);
	mem += sizeof(m_Illumination);
	m_EnvironLock.unlock();
	return mem;
}

#if defined(MEM_TRACE_ATOM)
void* DaAtom::operator new(size_t size)
{
	void* p = malloc(size);
	s_AllocatedAtoms++;
	return p;
}

void DaAtom::operator delete(void* p)
{
	s_FreedAtoms++;
	free(p);
}

//static
quint64 DaAtom::AllocatedCount()
{
	return s_AllocatedAtoms;
}

//static
quint64 DaAtom::FreedCount()
{
	return s_FreedAtoms;
}
#endif

int DaAtom::GunnerDomain(AuId& worldId) const
{
	if(!IsInAtmosphere())
		return 3;	// Definitely in space.
	const DaAtom* parent = Parent();
	if(!parent)
		return 3;	// wut?
	const DaWorld* w = parent->FindParentPlanetInEnv();
	if(!w)
		return 3;	// wut?
	// Inside atmosphere range of a world.
	worldId = w->Id();
	if(IsUnderGround() && AltitudeAGL() < -1)
		return 0;	// Under ground
	else if(IsCGUnderWater())
		return 1;	// Under water
	return 2;		// In atmosphere
}

bool DaAtom::IsGunnerDomainTarget(int gunnerDomain, const AuId& gunnerWorldId, const AuId& gunnerCapsuleId) const
{
	if(gunnerDomain == 0)
		return false;	// Gunner is under ground.
	if(Is(gunnerCapsuleId))
		return false;	// I am the capsule containing the gunner.
	AuId myWorldId;
	int myDomain = GunnerDomain(myWorldId);
	if(myDomain == 0)
		return false;	// I am under ground.
	if(abs(gunnerDomain - myDomain) > 1)
		return false;	// We are separated by a domain.
	if(gunnerDomain < 3 && myDomain < 3 && gunnerWorldId != myWorldId)
		return false;	// World domains and not at the same world.
	return true;	// Gunner can target me.
}

QString DaAtom::CommName() const
{
	return Name();
}

void DaAtom::FallStopped(double oldCgAlt, double nuCgAlt, double stopMove, int elapsedMs, int stopReason)
{
	// Atom elevation needed adjusting.
	// Limit stopMove damage to the atom's current vertical velocity.
	double vstop = stopMove;
	double vel = fabs(nuCgAlt - oldCgAlt);
	if(vstop > vel)
		vstop = vel;

	// Calculate the amount of movement per second.
	vstop = vstop * 1000 / elapsedMs;						// Movement in one second.

	double fallSafeSpeed = FallSafeSpeed();
	if(vstop > fallSafeSpeed)
	{
		// Vertical velocity is greater than my safe fall speed. Ignore less so we don't take damage driving around.
		vstop -= fallSafeSpeed;	// Speed in excess of my safe falling speed causes damage.

		// Calculate kinetic energy required to make the change.
		// K = 1/2 * m * v^2
		//   K is kinetic energy
		//   m is mass
		//   v is velocity
		const double khp = 7500.;							// Kinetic energy absorbed per hit point of damage.
		double myK = (Mass() * vstop*vstop)/2.;
		if(myK > khp)
		{
			// My kinetic energy dissipated is enough to cause at least one point of damage.
			double hitsDamage = myK/khp;					// This could be a really big number.
			quint32 hits;
			if(hitsDamage > 0x3fffffff)
				hits = 0x3fffffff;
			else
				hits = (quint32)hitsDamage;

			if(g_Au.IsSceneClient())
				emit FellOnClient(Id(), hits, stopReason);	// Notify the server.
			else
				Fell(hits, stopReason);	// Apply damage resulting from the fall.
			//qDebug() << "DaAtom::FallStopped" << Id() << Name()
				//<< "vel" << vel << "stopMove" << stopMove << stopReason << "vstop" << vstop << "elapsedMs" << elapsedMs
				//<< "vcalc" << (double)(vstop * elapsedMs / 1000.)
				//<< hits << "hits damage" << "myK" << myK << "mass" << Mass();
		}
	}
}

void DaAtom::Fell(quint32 hits, int reason)
{
	return;
	QString stopReason[] =
	{
		tr("collision with building floor"),
		tr("collision with building ceiling"),
		tr("collision with terrain"),
	};
	AuCombatAttacker attacker(stopReason[reason]);

	AuCombatWeapon weapon(WM_Contact);
	weapon.m_CID = CID_Collision;
	weapon.m_w1.m_Effect = WE_Bludgeon;

	AuCombatResult result;
	result.m_Damage1 = hits;

	CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
}

void DaAtom::SetPos(const AuAtomPos& pos)
{
	Q_ASSERT(!Ether());
	Q_ASSERT(!Is(pos.ParentId()));
	m_PosLock.lockForWrite();
	m_Pos = pos;
	m_PosChange.Set();
	m_PosLock.unlock();
}

void DaAtom::SetPosByEther(const AuAtomPos& pos, const AuEther* ether)
{
	Q_UNUSED(ether);					// Eliminate warning in release build.
	Q_ASSERT(Ether() == ether);
	Q_ASSERT(!Is(pos.ParentId()));
	m_PosLock.lockForWrite();
	bool hadNullParent = m_Pos.ParentId().IsNull();
	m_Pos = pos;
	m_PosChange.Set();
	m_PosLock.unlock();
	if(hadNullParent)
		TickTock();
}

void DaAtom::SetPosAndOrient(const AuAtomPos& pos)
{
	Q_ASSERT(!Ether());
	Q_ASSERT(!Is(pos.ParentId()));
	m_PosLock.lockForWrite();
	m_Pos = pos;
	m_OrientRotation = pos.Rotation();
	m_PosChange.Set();
	m_PosLock.unlock();
}

void DaAtom::SetPosAndOrientByEther(const AuAtomPos& pos, const AuEther* ether)
{
	Q_UNUSED(ether);					// Eliminate warning in release build.
	Q_ASSERT(Ether() == ether);
	Q_ASSERT(!Is(pos.ParentId()));
	m_PosLock.lockForWrite();
	bool hadNullParent = m_Pos.ParentId().IsNull();
	m_Pos = pos;
	m_OrientRotation = pos.Rotation();
	m_PosChange.Set();
	m_PosLock.unlock();
	if(hadNullParent)
		TickTock();
}

void DaAtom::ChangePos(const AuAtomPos& pos)
{
	//Q_ASSERT(!Is(pos.ParentId()));
	AuEther* ether = Ether();
	m_PosLock.lockForWrite();
	if(pos.ParentId() == m_Pos.ParentId() || !ether)
	{
		// My parent is not changing. Do what SetPos does.
		m_Pos = pos;

		m_PosChange.Set();
		m_PosLock.unlock();
		InvalidateEnvTransform();
	}
	else
	{
		// My parent is changing.
		m_PosLock.unlock();
		ether->ChangeParent(Id(), pos);	// The ether has to do the change. This ends up in a nnnByEther function.
	}
}

void DaAtom::ChangePosAndOrient(const AuAtomPos& pos)
{
	//Q_ASSERT(!Is(pos.ParentId()));
	AuEther* ether = Ether();
	m_PosLock.lockForWrite();
	if(pos.ParentId() == m_Pos.ParentId() || !ether)
	{
		// My parent is not changing. Do what SetPos does.
		m_Pos = pos;
		m_OrientRotation = pos.Rotation();
		m_PosChange.Set();
		m_PosLock.unlock();
		InvalidateEnvTransform();
	}
	else
	{
		// My parent is changing.
		m_PosLock.unlock();
		ether->ChangeParentAndOrient(Id(), pos);	// The ether has to do the change.
	}
}

void DaAtom::ForceMotion()
{
	if(CanSetInMotion())
	{
		m_PosLock.lockForWrite();
		SetAtRestState(false);
		m_PosChange.Set();	// So I'll stay in motion for a second.
		m_PosLock.unlock();
		SyncController(AuSyncMotion());
		TickTock();
	}
}

void DaAtom::ForcePos(const AuAtomPos& pos)
{
	ChangePos(pos);
	SyncController(AuSyncAtomPosVec(pos, m_AtRest, Vector()));
}

void DaAtom::ForcePosMove(const AuAtomPos& pos, const AuPoint& vector)
{
	bool atRest = !CanSetInMotion();
	ChangePos(pos);
	SetVector(vector);
	SetAtRestState(atRest);
	SyncController(AuSyncAtomPosVec(pos, atRest, vector));
	TickTock();
}

void DaAtom::ForcePosStop(const AuAtomPos& pos)
{
	bool atRest = true;
	ChangePos(pos);
	SetAtRest();
	SyncController(AuSyncAtomPosVec(pos, atRest, AuPoint(0,0)));
	TickTock();
}

void DaAtom::ForceVector(const AuPoint& movementVector)
{
	m_PosLock.lockForWrite();
	m_Vector = movementVector;
	if(m_Vector.IsZero())
		SetAtRestState(true);
	else
		SetAtRestState(false);
	m_PosChange.Set();	// So I'll stay in motion for a second.
	m_PosLock.unlock();
	SyncController(AuSyncVector(movementVector));
}

DaBd* DaAtom::FindParentBdInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentBdInEnv();	// Not me. How about my parent?
}

const DaBd* DaAtom::FindParentBdInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentBdInEnv();	// Not me. How about my parent?
}

DaCapsule* DaAtom::FindParentCapsuleInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentCapsuleInEnv();	// Not me. How about my parent?
}

const DaCapsule* DaAtom::FindParentCapsuleInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentCapsuleInEnv();	// Not me. How about my parent?
}

DaDesignBase* DaAtom::FindParentDesignBaseInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentDesignBaseInEnv();	// Not me. How about my parent?
}

const DaDesignBase* DaAtom::FindParentDesignBaseInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentDesignBaseInEnv();	// Not me. How about my parent?
}

DaDesigner* DaAtom::FindParentDesignerInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentDesignerInEnv();	// Not me. How about my parent?
}

const DaDesigner* DaAtom::FindParentDesignerInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentDesignerInEnv();	// Not me. How about my parent?
}

DaWorld* DaAtom::FindParentPlanetInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentPlanetInEnv();	// Not me. How about my parent?
}

const DaWorld* DaAtom::FindParentPlanetInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentPlanetInEnv();	// Not me. How about my parent?
}

DaSolarSystem* DaAtom::FindParentSolarSystemInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentSolarSystemInEnv();	// Not me. How about my parent?
}

const DaSolarSystem* DaAtom::FindParentSolarSystemInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentSolarSystemInEnv();	// Not me. How about my parent?
}

DaStar* DaAtom::FindParentStarInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentStarInEnv();	// Not me. How about my parent?
}

const DaStar* DaAtom::FindParentStarInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentStarInEnv();	// Not me. How about my parent?
}

DaVessel* DaAtom::FindParentVesselInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentVesselInEnv();	// Not me. How about my parent?
}

const DaVessel* DaAtom::FindParentVesselInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentVesselInEnv();	// Not me. How about my parent?
}

DaWorldLL* DaAtom::FindParentWorldInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentWorldInEnv();	// Not me. How about my parent?
}

const DaWorldLL* DaAtom::FindParentWorldInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentWorldInEnv();	// Not me. How about my parent?
}

DaWorldLL* DaAtom::FindParentWorldNotSatelliteInEnv()
{
	DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentWorldNotSatelliteInEnv();	// Not me. How about my parent?
}

const DaWorldLL* DaAtom::FindParentWorldNotSatelliteInEnv() const
{
	const DaAtom* parent = Parent();
	if(!parent)
		return 0;
	return parent->FindParentWorldNotSatelliteInEnv();	// Not me. How about my parent?
}

//static
QStringList DaAtom::DbLimboCheck(QSqlDatabase& dbUniverse, const char* prefixStep)
{
	QStringList textLines;
	// Get the universe id.
	AuId universeId;
	{
		QString sq;
		sq = QString("SELECT id FROM a_Universe;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
			universeId = AuId(qry.value(0).toUInt());
	}
	if(universeId.IsNull())
	{
		textLines << "Failed to get universe id.";
		return textLines;
	}

	AuIdList galaxyIds;
	{
		QString sq;
		sq = QString("SELECT id FROM a_Galaxy;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			AuId galaxyId = AuId(qry.value(0).toUInt());
			galaxyIds.append(galaxyId);
		}
	}
	if(galaxyIds.isEmpty())
	{
		textLines << "Failed to get galaxy ids.";
		return textLines;
	}

	// Locate lost ships. Do this first because it is fast.
	AuIdList limboIds;
	textLines = DbLostShips(dbUniverse, limboIds);

	// Locate all people in the database that have not been deleted.
	textLines << "Looking for people stuck in limbo...";
	int startCount = limboIds.count();
	{
		QString sq;
		sq = QString("SELECT id,idp,name,lastOnLine,spawnCity,spawnCityName,pose FROM a_Person WHERE idp >= 0;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qvi=0;		// query value index
			AuId atomId = AuId(qry.value(qvi++).toUInt());
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			QDateTime lastOnLine = qry.value(qvi++).toDateTime();
			lastOnLine.setTimeSpec(Qt::UTC);
			AuId spawnCity = AuId(qry.value(qvi++).toUInt());
			QString spawnCityName = qry.value(qvi++).toString();
			int pose = qry.value(qvi++).toInt();

			// Universe parent id AuId(2) used to identify people waiting to spawn. They are no longer subordinate to the universe
			// when waiting.
			// People waiting to spawn are subordinate to their parent galaxy.
			if(galaxyIds.contains(parentId) || parentId == AuId(2))
			{
				if(pose == 54)
					textLines << QString("  Awaiting Spawn: %1 %2 lastOn %3 spawnCity %4 %5 pose %6")
						.arg(atomId.GetID())
						.arg(name)
						.arg(lastOnLine.toString())
						.arg(spawnCity.GetID())
						.arg(spawnCityName)
						.arg(pose);
				else
				{
					textLines << QString("  Universe Limbo: %1 %2 lastOn %3 spawnCity %4 %5 pose %6")
						.arg(atomId.GetID())
						.arg(name)
						.arg(lastOnLine.toString())
						.arg(spawnCity.GetID())
						.arg(spawnCityName)
						.arg(pose);
					limboIds.append(atomId);
				}
			}
			else if(!DbFindParent(dbUniverse, atomId, universeId))
			{
				textLines << QString("  Limbo: %1 %2 lastOn %3")
					.arg(atomId.GetID())
					.arg(name)
					.arg(lastOnLine.toString());
				limboIds.append(atomId);

				// Report parent atom chain of the atom, starting at the atom.
				AuId reportAtom(atomId);
				while(!reportAtom.IsNull())
					reportAtom = DbQueryAtom(textLines, dbUniverse, reportAtom, false, false, "", prefixStep);
			}
			else
			{
				// People can be stuck aboard ships that are in stasis.
				QString sq;
				sq = QString("SELECT state FROM a_Sc WHERE id=%1;").arg(parentId.GetID());
				QSqlQuery qrysc(QString(), dbUniverse);
				qrysc.prepare(sq);
				qrysc.exec();
				if(qrysc.next())
				{
					int state = qrysc.value(0).toInt();
					if(state == DaAtom::Dormant)
					{
						textLines << QString("  Dormant Limbo: %1 %2 sc %3")
							.arg(atomId.GetID())
							.arg(name)
							.arg(parentId.GetID());
						limboIds.append(atomId);
					}
				}
			}
		}
	}
	if(limboIds.count() != startCount)
		textLines << QString("%1 people stuck in limbo.")
			.arg(limboIds.count() - startCount);
	else
		textLines << "No people stuck in limbo.";

	// Locate all officers in the database.
	textLines << "Looking for officers stuck in limbo...";
	startCount = limboIds.count();
	{
		QString sq;
		sq = QString("SELECT id,name FROM a_Npc WHERE role=%1;").arg(ROLE_Officer);
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qvi=0;		// query value index
			AuId atomId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();

			if(!DbFindParent(dbUniverse, atomId, universeId))
			{
				textLines << QString("  Limbo: %1 %2")
					.arg(atomId.GetID())
					.arg(name);
				limboIds.append(atomId);

				// Report parent atom chain of the atom, starting at the atom.
				AuId reportAtom(atomId);
				while(!reportAtom.IsNull())
					reportAtom = DbQueryAtom(textLines, dbUniverse, reportAtom, false, false, "", prefixStep);
			}
		}
	}
	if(limboIds.count() != startCount)
		textLines << QString("%1 officers stuck in limbo.")
			.arg(limboIds.count() - startCount);
	else
		textLines << "No officers stuck in limbo.";

	if(!limboIds.isEmpty())
	{
		QFile file("rescue.bin");
		if(file.open(QFile::WriteOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			ds << limboIds;
			textLines << QString("%1 atom ids stored in 'rescue.bin'.")
				.arg(limboIds.count());
		}
	}
	return textLines;
}

//static
QStringList DaAtom::DbLostShips(QSqlDatabase& dbUniverse, AuIdList& limboRescueMe)
{
	QStringList textLines;
	int lostShips = 0;
	int lostPlayers = 0;
	int lostOfficers = 0;

	QHash<AuId, AuId> primaries;	// key is system id, value is star id
	textLines << "Looking for lost spacecraft...";
	QString sq = QString("SELECT id,idp,name,timeModified FROM a_Sc WHERE idp IN (SELECT id FROM a_SolarSystem WHERE final='f');");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());
		AuId idp = AuId(qa.value(qv++).toUInt());
		QString name = qa.value(qv++).toString();
		QDateTime mod = qa.value(qv++).toDateTime();

		textLines << QString("  Lost: %1 %2 last modified %4 in nonfinal solar system %3")
			.arg(id.GetID())
			.arg(name)
			.arg(idp.GetID())
			.arg(mod.toString());
		lostShips++;

		// A lost ship can be limbo rescued by rescuing one of the people or officers aboard. We don't need to rescue all of them.
		bool limboRescued = false;

		QString sqp = QString("SELECT id,name,timeModified FROM a_Person WHERE idp=%1;").arg(id.GetID());
		QSqlQuery qap(sqp, dbUniverse);
		while(qap.next())
		{
			int qv=0;
			AuId idp = AuId(qap.value(qv++).toUInt());
			QString namep = qap.value(qv++).toString();
			QDateTime modp = qap.value(qv++).toDateTime();

			// A lost ship with people aboard can be restored using limbo rescue on a person or officer.
			if(!limboRescued)
			{
				limboRescueMe.AddIfUnique(idp);
				limboRescued = true;
			}

			textLines << QString("    Avatar %1 %2 aboard, modified %3")
				.arg(idp.GetID())
				.arg(namep)
				.arg(modp.toString());
			lostPlayers++;
		}

		QString sqo = QString("SELECT id,name,timeModified FROM a_Npc WHERE idp=%1 AND role=%2;").arg(id.GetID()).arg(ROLE_Officer);
		QSqlQuery qao(sqo, dbUniverse);
		while(qao.next())
		{
			int qv=0;
			AuId ido = AuId(qao.value(qv++).toUInt());
			QString nameo = qao.value(qv++).toString();
			QDateTime modo = qao.value(qv++).toDateTime();

			// A lost ship with officers aboard can be restored using limbo rescue on a person or officer.
			if(!limboRescued)
			{
				limboRescueMe.AddIfUnique(ido);
				limboRescued = true;
			}

			textLines << QString("    Officer %1 %2 aboard, modified %3")
				.arg(ido.GetID())
				.arg(nameo)
				.arg(modo.toString());
			lostOfficers++;
		}
	}

	if(lostPlayers || lostOfficers)
		textLines << QString("%1 lost spacecraft found with %2 avatars and %3 officers aboard. Can be limbo rescued.")
			.arg(lostShips)
			.arg(lostPlayers)
			.arg(lostOfficers);
	else if(lostShips)
		textLines << QString("%1 lost spacecraft found with no avatars or officers aboard.")
			.arg(lostShips);
	else
		textLines << "No lost spacecraft found.";
	return textLines;
}

//static
bool DaAtom::DbFindParent(QSqlDatabase& dbUniverse, const AuId& atomId, const AuId& findParentId)
{
	AuId parentId(atomId);
	while(!parentId.IsNull())
	{
		QString sq = QString("SELECT idp FROM a_Atom WHERE id=%1;").arg(parentId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			parentId = AuId(qry.value(0).toUInt());
			if(parentId == findParentId)
				return true;				// The parent atom was found in the database.
		}
		else
			return false;					// Parent atom was not found in the database.
	}
	return false;
}

// Reports information about an atom and returns its parent atom id.
//static
AuId DaAtom::DbQueryAtom(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& atomId, bool includeGear,
		bool includeMemory, const char* prefix, const char* prefixStep)
{
	// Try person.
	{
		QString sq;
		sq = QString("SELECT idp,name,allegiance,lastOnLine,dna,dna4,inv,seaChest,memory,pose,spawnShip,spawnCity,spawnCityName,"
				"spawnSystem,spawnSystemName,stats,conditions "
				"FROM a_Person WHERE id=%1;")
			.arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			quint32 empireId = qry.value(qvi++).toUInt();
			QDateTime lastOnLine = qry.value(qvi++).toDateTime();
			AuDNABase* dna;
			lastOnLine.setTimeSpec(Qt::UTC);
			{
				// The nu style DNA takes precedence over the old style.
				QByteArray baol = qry.value(qvi++).toByteArray();
				QByteArray banu = qry.value(qvi++).toByteArray();
				if(banu.isEmpty())
					dna = new DhDNA(baol);	// Nu is empty so assume old is ok.
				else
				{
					// Nu is not empty so use it.
					QDataStream in(banu);
					in.setVersion(QDataStream::Qt_5_7);
					dna = AuDNABase::ReadDynamic(in);
				}
			}

			AuGear gear;
			QByteArray ba = qry.value(qvi++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> gear;
			}

			AuItemHash seaChest[2];
			ba = qry.value(qvi++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				for(int zone=0; zone<2; zone++)
					seaChest[zone] << in;
			}

			AuMemories memory;
			ba = qry.value(qvi++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> memory;
			}
			AuPose::POSE curPose = (AuPose::POSE)qry.value(qvi++).toInt();
			AuId spawnShip = AuId(qry.value(qvi++).toUInt());
			AuId spawnCity = AuId(qry.value(qvi++).toUInt());
			QString spawnCityName = qry.value(qvi++).toString();
			AuId spawnSystem = AuId(qry.value(qvi++).toUInt());
			QString spawnSystemName = qry.value(qvi++).toString();

			AuPlayerStats stats;
			ba = qry.value(qvi++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				stats << in;
			}

			AuConditionList conditions;
			ba = qry.value(qvi++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> conditions;
			}

			const DgData& gd = g_Au.LockDataForRead();
			textLines << QString("%1 Person: %2 %3 empire %4 %5 lastOn %6")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(empireId)
				.arg(gd.EmpireHtmlName(empireId))
				.arg(lastOnLine.toString());
			bool hasOffice = gd.IsEmpireGovernor(empireId, atomId);
			AuOffice office = gd.EmpireCitizenOffice(empireId, atomId);
			g_Au.UnlockData();

			if(dna)
			{
				AuSpecies species = dna->Species();
				delete dna;
				textLines << QString("%1%4 Species %2 %3")
					.arg(prefix)
					.arg(species.Name())
					.arg(species.Description())
					.arg(prefixStep);
			}

			if(hasOffice)
			{
				textLines << QString("%1%4 Office: %2 boss %3")
					.arg(prefix)
					.arg(office.TitleOfOffice())
					.arg(office.BossId().GetID())
					.arg(prefixStep);
				for(AuOfficeDomain::const_iterator di = office.Domain().cbegin(); di != office.Domain().cend(); di++)
					textLines << QString("%1%4%4 %2 %3")
						.arg(prefix)
						.arg((*di)->Type())
						.arg((*di)->Name())
						.arg(prefixStep);
			}

			if(curPose == AuPose::Spawn)
				textLines << QString("%1%2 Spawning: ship %3, bd %4 %5, %6 %7")
					.arg(prefix)
					.arg(prefixStep)
					.arg(spawnShip.GetID())
					.arg(spawnCity.GetID())
					.arg(spawnCityName)
					.arg(spawnSystem.GetID())
					.arg(spawnSystemName);
			else
				textLines << QString("%1%2 Spawn Tie: ship %3, bd %4 %5, %6 %7")
					.arg(prefix)
					.arg(prefixStep)
					.arg(spawnShip.GetID())
					.arg(spawnCity.GetID())
					.arg(spawnCityName)
					.arg(spawnSystem.GetID())
					.arg(spawnSystemName);

			// Stories
			QList<quint32> scriptIds;
			stats.StoriesFinished(scriptIds);
			if(!scriptIds.empty())
			{
				QString scriptNames[] =
				{
					"Custom Story Script",	// 0
					"Veil of Targoss",		// 1
					"Space Program",		// 2
					"Getting Started",		// 3
					"Rocket Training",		// 4
					"Caroler",				// 5
					"Relic",				// 6
					"Dangerous Exchange",	// 7
					"Beacon",				// 8
					"Reach for the Stars",	// 9
					"Undertow",				// 10
				};
				textLines << QString("%1%2 Stories Finished").arg(prefix).arg(prefixStep);
				for(QList<quint32>::const_iterator si = scriptIds.cbegin(); si != scriptIds.cend(); si++)
					textLines << QString("%1%4%4 Script %2 %3").arg(prefix).arg(*si).arg(scriptNames[*si]).arg(prefixStep);
			}

			// Conditions
			if(!conditions.empty())
			{
				textLines << QString("%1%2 Conditions").arg(prefix).arg(prefixStep);
				for(AuConditionList::const_iterator ci = conditions.cbegin(); ci != conditions.cend(); ci++)
					textLines << QString("%1%3%3 %2").arg(prefix).arg((*ci)->Description()).arg(prefixStep);
			}

			// Report gear.
			if(includeGear)
			{
				textLines << QString("%1%3 Gear %2 top level items")
					.arg(prefix)
					.arg(gear.size())
					.arg(prefixStep);
				for(AuGear::const_iterator gi = gear.cbegin(); gi != gear.cend(); gi++)
				{
					const AuItem& gearItem = (*gi).Item();
					if(gearItem.StoryDebug() != "")
						textLines << QString("%1%5%5%5 %2 %3 %4")
							.arg(prefix)
							.arg((*gi).SlotTypeName())
							.arg(gearItem.ListDesc())
							.arg(gearItem.StoryDebug())
							.arg(prefixStep);
					else
						textLines << QString("%1%4%4%4 %2 %3")
							.arg(prefix)
							.arg((*gi).SlotTypeName())
							.arg(gearItem.ListDesc())
							.arg(prefixStep);
					if(gearItem.Content())
					{
						QString prefix2(prefix);
						prefix2 += QString("%1%1%1%1 ").arg(prefixStep);
						DbItemContent(textLines, *gearItem.Content(), prefix2.toLocal8Bit(), prefixStep);
					}
				}

				for(int zone=0; zone<2; zone++)
				{
					textLines << QString("%1%4 Sea chest %3 has %2 top level items")
						.arg(prefix)
						.arg(seaChest[zone].size())
						.arg(zone+1)
						.arg(prefixStep);
					for(AuItemHash::const_iterator ci = seaChest[zone].cbegin(); ci != seaChest[zone].cend(); ci++)
					{
						if((*ci).second->StoryDebug() != "")
							textLines << QString("%1%4%4%4 %2 %3")
								.arg(prefix)
								.arg((*ci).second->ListDesc())
								.arg((*ci).second->StoryDebug())
								.arg(prefixStep);
						else
							textLines << QString("%1%3%3%3 %2")
								.arg(prefix)
								.arg((*ci).second->ListDesc())
								.arg(prefixStep);
						if((*ci).second->Content())
						{
							QString prefix2(prefix);
							prefix2 += QString("%1%1%1%1 ").arg(prefixStep);
							DbItemContent(textLines, *(*ci).second->Content(), prefix2.toLocal8Bit(), prefixStep);
						}
					}
				}
			}

			// Memories
			if(includeMemory)
			{
				textLines << QString("%1%2 Memory:")
					.arg(prefix)
					.arg(prefixStep);
				for(AuMemories::const_iterator mi = memory.cbegin(); mi != memory.cend(); mi++)
					textLines << QString("%1%3%3 %2")
						.arg(prefix)
						.arg((*mi)->Desc())
						.arg(prefixStep);
			}
			return parentId;
		}
	}

	// Try animal.
	{
		QString sq;
		sq = QString("SELECT idp,name FROM a_Animal WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();

			textLines << QString("%1 Animal: %2 %3")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name);
			return parentId;
		}
	}

	// Try bd building.
	{
		QString sq;
		sq = QString("SELECT idp,name,allegiance,designId,industry,cargo4 FROM a_Bd WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			quint32 empireId = qry.value(qvi++).toUInt();
			quint32 designId = qry.value(qvi++).toUInt();
			INDUSTRYID iid = (INDUSTRYID)qry.value(qvi++).toInt();

			// Cargo
			AuItemHash cargo, tools;
			AuKnownPatents patents;
			AuSpecies livestock;
			quint16 storeCommodity = CID_Invalid;
			QByteArray ba4 = qry.value(qvi++).toByteArray();
			if(!ba4.isEmpty())
			{
				QDataStream in(ba4);
				in.setVersion(QDataStream::Qt_5_7);
				quint8 ver;
				in >> ver;
				cargo << in;
				tools << in;
				in >> patents;
				in >> livestock;
				in >> storeCommodity;
			}

			const DgData& gd = g_Au.LockDataForRead();
			textLines << QString("%1 Bd: %2 %3 %4 empire %5 %6 design %7")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(gd.IndustryName(iid))
				.arg(empireId)
				.arg(gd.EmpireHtmlName(empireId))
				.arg(designId);
			g_Au.UnlockData();

			if(includeGear)
			{
				textLines << QString("%1%2 Cargo %L3 Tools %L4 Patents %L5 Store %L6")
					.arg(prefix)
					.arg(prefixStep)
					.arg(cargo.size())
					.arg(tools.size())
					.arg(patents.size())
					.arg(g_Au.CommodityName((COMMODITYID)storeCommodity));
				textLines << QString("%1%2%2 Cargo")
						.arg(prefix)
						.arg(prefixStep);
				for(AuItemHash::const_iterator ci = cargo.cbegin(); ci != cargo.cend(); ci++)
				{
					textLines << QString("%1%2%2%2 %3")
						.arg(prefix)
						.arg(prefixStep)
						.arg((*ci).second->ListDesc());
				}
				textLines << QString("%1%2%2 Tools")
						.arg(prefix)
						.arg(prefixStep);
				for(AuItemHash::const_iterator ci = tools.cbegin(); ci != tools.cend(); ci++)
				{
					textLines << QString("%1%2%2%2 %3")
						.arg(prefix)
						.arg(prefixStep)
						.arg((*ci).second->ListDesc());
				}
				textLines << QString("%1%2%2 Patents")
						.arg(prefix)
						.arg(prefixStep);
				for(QHash<COMMODITYID, quint8>::const_iterator pi = patents.cbegin(); pi != patents.cend(); pi++)
				{
					textLines << QString("%1%2%2%2 %3 Q%4")
						.arg(prefix)
						.arg(prefixStep)
						.arg(g_Au.CommodityName(pi.key()))
						.arg(*pi);
				}
			}
			return parentId;
		}
	}

	// Try sc vessel.
	{
		QString sq;
		sq = QString("SELECT idp,name,allegiance,state,idDesign,switches FROM a_Sc WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			quint32 empireId = qry.value(qvi++).toUInt();
			int state = qry.value(qvi++).toInt();
			quint32 designId = qry.value(qvi++).toUInt();
			int switches = qry.value(qvi++).toInt();

			bool moored = switches & 0x00000200;

			QString atomStateNames[] =
			{
				"Normal",
				"Dead",
				"Traveling",
				"Dormant",
				"Client Reparent",
				"New",
				"Trash",
			};
			QString stateDesc(atomStateNames[state]);
			if(state == Dormant)
			{
				if(moored)
					stateDesc = "Hangared";
				else
					stateDesc = "Stasis";
			}
			const DgData& gd = g_Au.LockDataForRead();
			textLines << QString("%1 Sc: %2 %3 empire %4 %5 state %6 design %7")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(empireId)
				.arg(gd.EmpireHtmlName(empireId))
				.arg(stateDesc)
				.arg(designId);
			g_Au.UnlockData();
			return parentId;
		}
	}

	// Try vehicle.
	{
		QString sq;
		sq = QString("SELECT idp,cid,allegiance FROM a_Vehicle WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			COMMODITYID cid = (COMMODITYID)qry.value(qvi++).toInt();
			quint32 empireId = qry.value(qvi++).toUInt();

			const DgData& gd = g_Au.LockDataForRead();
			textLines << QString("%1 Vehicle: %2 %3 empire %4 %5")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(g_Au.CommodityName(cid))
				.arg(empireId)
				.arg(gd.EmpireHtmlName(empireId));
			g_Au.UnlockData();
			return parentId;
		}
	}

	// Try planet.
	{
		QString sq;
		sq = QString("SELECT idp,name,orbitZone FROM a_World WHERE id=%1;")
			.arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qv=0;		// query value index
			AuId parentId = AuId(qry.value(qv++).toUInt());
			QString name = qry.value(qv++).toString();

			/*
			// Terrain generation factors unique to this world.
			float terrFrequency;		// Noise frequency.
			float terrPersFactor;	// Noise scaling factor due to persistence.
			float terrPersistence;	// Persistence causes jaggier terrain boundaries as it goes up.
			float terrXOff;			// Perlin noise X offset.
			float terrYOff;			// Perlin noise Y offset.
			float terrWaterFactor;	// Modifier due to water coverage.
			int qvi=qv++;
			if(!qry.value(qvi).isNull())
			{
				QByteArray ba = qry.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				in >> terrFrequency;
				in >> terrPersFactor;
				in >> terrPersistence;
				in >> terrXOff;
				in >> terrYOff;
				in >> terrWaterFactor;
			}
			*/

			// DaWorldGlobe values.
			//quint8 byRadius = qry.value(qv++).toInt();
			quint8 chOrbitZone = qry.value(qv++).toInt();
			//quint8 byAtmType = qry.value(qv++).toInt();
			//qint8 byAtmDensity = qry.value(qv++).toInt();
			//qint8 byWater = qry.value(qv++).toInt();

			//qDebug() << prefix << "Planet:" << atomId << name << "tFreq" << terrFrequency << "tPersF" << terrPersFactor
				//<< "tPers" << terrPersistence << "tX" << terrXOff << "tY" << terrYOff << "tWater" << terrWaterFactor
				//<< "radius" << byRadius << "OZone" << chOrbitZone << "atmType" << byAtmType << "atmDensity" << byAtmDensity
				//<< "water" << byWater << "%";
			textLines << QString("%1 World: %2 %3 orbitZone %4")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(chOrbitZone);
			return parentId;
		}
	}

	// Try star.
	{
		QString sq;
		sq = QString("SELECT idp FROM a_Star WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());

			textLines << QString("%1 Star: %2")
				.arg(prefix)
				.arg(atomId.GetID());
			return parentId;
		}
	}

	// Try solar system.
	{
		QString sq;
		sq = QString("SELECT idp,name FROM a_SolarSystem WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();

			textLines << QString("%1 Solar System: %2 %3")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name);
			return parentId;
		}
	}

	// Try sector.
	{
		QString sq;
		sq = QString("SELECT idp,name,locX,locY,locZ FROM a_Sector WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			double vx = qry.value(qvi++).toDouble();
			double vy = qry.value(qvi++).toDouble();
			double vz = qry.value(qvi++).toDouble();

			textLines << QString("%1 Sector: %2 %3 (%4, %5, %6)pc")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(vx)
				.arg(vy)
				.arg(vz);
			return parentId;
		}
	}

	// Try galaxy.
	{
		QString sq;
		sq = QString("SELECT idp,galaxy FROM a_Galaxy WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qv=0;		// query value index
			AuId parentId = AuId(qry.value(qv++).toUInt());
			GALAXY galaxyEnum = (GALAXY)qry.value(qv++).toInt();

			textLines << QString("%1 Galaxy: %2 %3")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(GalaxyName(galaxyEnum));
			return parentId;
		}
	}

	// Try universe.
	{
		QString sq;
		sq = QString("SELECT idp FROM a_Universe WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());

			textLines << QString("%1 Universe: %2")
				.arg(prefix)
				.arg(atomId.GetID());
			return parentId;
		}
	}

	// Try npc.
	{
		QString sq;
		sq = QString("SELECT idp,name,role,allegiance FROM a_Npc WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			QString name = qry.value(qvi++).toString();
			NPC_ROLE npcRole = (NPC_ROLE)qry.value(qvi++).toInt();
			quint32 empireId = qry.value(qvi++).toUInt();

			const DgData& gd = g_Au.LockDataForRead();
			textLines << QString("%1 %6: %2 %3 empire %4 %5")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(name)
				.arg(empireId)
				.arg(gd.EmpireHtmlName(empireId))
				.arg(NpcRoleName(npcRole));
			g_Au.UnlockData();
			return parentId;
		}
	}

	// Try Item.
	{
		QString sq;
		sq = QString("SELECT idp,cid FROM a_Item WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			COMMODITYID cid = (COMMODITYID)qry.value(qvi++).toInt();

			textLines << QString("%1 Item: %2 %3")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(g_Au.CommodityName(cid));
			return parentId;
		}
	}

	// Try Designer
	{
		QString sq;
		sq = QString("SELECT idp,archid,archname,designMode,ownerId FROM a_Designer WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());
			AuId archId = AuId(qry.value(qvi++).toUInt());
			QString archName = qry.value(qvi++).toString();
			int designMode = qry.value(qvi++).toInt();
			AuId ownerId = AuId(qry.value(qvi++).toUInt());

			textLines << QString("%1 Designer: %2 owner %3 arch %4 %5 designMode %6")
				.arg(prefix)
				.arg(atomId.GetID())
				.arg(ownerId.GetID())
				.arg(archId.GetID())
				.arg(archName)
				.arg(designMode);
			return parentId;
		}
	}

	// Try Design Plane.
	{
		QString sq;
		sq = QString("SELECT idp FROM a_DesignPlane WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());

			textLines << QString("%1 Design Plane: %2")
				.arg(prefix)
				.arg(atomId.GetID());
			return parentId;
		}
	}

	// Try any atom.
	{
		QString sq;
		sq = QString("SELECT idp FROM a_Atom WHERE id=%1;").arg(atomId.GetID());
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qvi=0;		// query value index
			AuId parentId = AuId(qry.value(qvi++).toUInt());

			textLines << QString("%1 Atom: %2")
				.arg(prefix)
				.arg(atomId.GetID());
			return parentId;
		}
	}

	// The atom was not found.
	textLines << QString("%1 Atom NOT found! %2")
		.arg(prefix)
		.arg(atomId.GetID());
	return AuId(0);
}

//static
void DaAtom::DbItemContent(QStringList& textLines, const AuItemContent& content, const char* prefix, const char* prefixStep)
{
	textLines << QString("%1%4 Contents %2 items %3 pack units")
		.arg(prefix)
		.arg(content.size())
		.arg(content.ContainsPackUnits())
		.arg(prefixStep);
	for(AuItemContent::const_iterator ci = content.cbegin(); ci != content.cend(); ci++)
	{
		if((*ci).second->StoryDebug() != "")
			textLines << QString("%1%5%5 %2 %3 %4")
				.arg(prefix)
				.arg((*ci).first.Id())
				.arg((*ci).second->ListDesc())
				.arg((*ci).second->StoryDebug())
				.arg(prefixStep);
		else
			textLines << QString("%1%4%4 %2 %3")
				.arg(prefix)
				.arg((*ci).first.Id())
				.arg((*ci).second->ListDesc())
				.arg(prefixStep);
		if((*ci).second->Content())
		{
			QString prefix2(prefix);
			prefix2 += QString("%1%1%1 ").arg(prefixStep);
			DbItemContent(textLines, *(*ci).second->Content(), prefix2.toLocal8Bit(), prefixStep);
		}
	}
}

// Locates an atom id in the database and reports information about it.
//static
void DaAtom::DbFindPerson(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& personName, bool reportGear,
		bool reportMemory, const char* prefixStep)
{
	// Find all people with that name.
	QString sq;
	sq = QString("SELECT id FROM a_Person WHERE name='%1' AND idp > 0;").arg(personName);
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qvi=0;		// query value index
		AuId atomId = AuId(qry.value(qvi++).toUInt());
		DbFindAtom(textLines, dbUniverse, atomId, reportGear, reportMemory, prefixStep);
		textLines << "<hr>";
	}
}

//static
void DaAtom::DbFindSpacecraft(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& shipName, bool reportGear,
		bool reportMemory, const char* prefixStep)
{
	// Find all people with that name.
	{
		QString sq = QString("SELECT id FROM a_Sc WHERE name='%1' AND idp > 0;").arg(shipName);
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qvi=0;		// query value index
			AuId atomId = AuId(qry.value(qvi++).toUInt());
			DbFindAtom(textLines, dbUniverse, atomId, reportGear, reportMemory, prefixStep);
			textLines << "<hr>";
		}
	}
}

// Locates an atom id in the database and reports information about it.
//static
void DaAtom::DbFindAtom(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& atomId, bool reportGear, bool reportMemory,
		const char* prefixStep)
{
	textLines << QString("Atom: %1 %2")
		.arg(atomId.GetID())
		.arg(atomId.StringId());

	// Report parent atom chain of the atom, starting at the atom.
	AuId reportAtom(atomId);
	while(!reportAtom.IsNull())
		reportAtom = DbQueryAtom(textLines, dbUniverse, reportAtom, reportGear, reportMemory, "", prefixStep);
}

//static
void DaAtom::DbQuerySystem(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& systemName, const char* prefixStep)
{
	textLines << QString("ReportSystem %1")
		.arg(systemName);
	QString sq;
	sq = QString("SELECT id FROM a_SolarSystem WHERE name = '%1';").arg(systemName);
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qvi=0;		// query value index
		AuId systemId = AuId(qry.value(qvi++).toUInt());
		DbQuerySystem(textLines, dbUniverse, systemId, prefixStep);
		textLines << "<hr>";
	}
}

//static
void DaAtom::DbQuerySystem(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& systemId, const char* prefixStep)
{
	textLines << QString("Report Solar System %1")
		.arg(systemId.GetID());
	AuEther tempEther("DaAtom::DbQuerySystem");
	DaSolarSystem* system = DaSolarSystem::LoadSubatoms(tempEther, dbUniverse, AuId(), LOAD_All, systemId);
	if(system)
	{
		// Explored by
		QString exploredBy("Explored by");
		const QVector<quint32>& es = system->LockExploredBy();
		for(QVector<quint32>::const_iterator ei = es.cbegin(); ei != es.cend(); ei++)
			exploredBy += QString(" %1").arg(*ei);
		system->UnlockExploredBy();
		textLines << exploredBy;

		// Scanned by
		QString scannedBy("Scanned by");
		const QSet<quint32>& ss = system->LockScannedBy();
		for(QSet<quint32>::const_iterator si = ss.cbegin(); si != ss.cend(); si++)
			scannedBy += QString(" %1").arg(*si);
		system->UnlockScannedBy();
		textLines << scannedBy;

		// Wormholes
		int wormcount = system->LockWormholes().count();
		system->UnlockWormholes();

		textLines << QString("%1 %2 wormholes %3")
			.arg(system->Id().GetID())
			.arg(system->Name())
			.arg(wormcount);
		const AuWormholeArray& worms = system->LockWormholes();
		for(AuWormholeArray::const_iterator wi = worms.cbegin(); wi != worms.cend(); wi++)
			textLines << QString("%8%1 %2 to system %3 sectVecToDest %4,%5,%6 dist %7 pc ")
				.arg((*wi).Name())
				.arg((*wi).Polarity())
				.arg((*wi).ToSystem().GetID())
				.arg((*wi).SectorVecToDestination().x)
				.arg((*wi).SectorVecToDestination().y)
				.arg((*wi).SectorVecToDestination().z)
				.arg((*wi).SectorVecToDestination().Length())
				.arg(prefixStep);
		system->UnlockWormholes();

		DaAtomList subs;
		system->ListSubatoms(subs);
		if(!subs.empty())
		{
			DaAtomList subAtoms;
			system->ListNormalSubatomsRecursive(subAtoms);
			textLines << QString("%1 %2 has %3 subatoms %4 total subatoms, lastPersonVisit %5 SystemFinal %6")
				.arg(system->Id().GetID())
				.arg(system->Name())
				.arg(subs.count())
				.arg(subAtoms.count())
				.arg(system->LastPersonVisited().UTC().toString())
				.arg(system->IsFinal()?"Yes":"No");

			// Immediate subatoms
			for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
				textLines << QString("%4%1 %2 %3")
					.arg((*ai)->Id().GetID())
					.arg((*ai)->Name())
					.arg((*ai)->Description())
					.arg(prefixStep);
		}
		else
			textLines << QString("%1 %2 loaded, no subatoms, lastPersonVisit %3")
				.arg(system->Id().GetID())
				.arg(system->Name())
				.arg(system->LastPersonVisited().UTC().toString());

		// People
		DaAtomList people;
		system->FindPeopleRecursive(people);
		if(!people.isEmpty())
		{
			textLines << QString("%1 people")
				.arg(people.count());
			for(DaAtomList::const_iterator pi = people.cbegin(); pi != people.cend(); pi++)
			{
				DaPerson* person = (DaPerson*)(*pi);
				person->m_AgentBits = 0;	// Don't report person as "Entity" or some animal. :P
				textLines << QString("%1%2 %3, %4 lastMod %5 pose %6 isDead %7 dormant %8 travel %9 normal %10 parent %11 %12 "
						"loc %13,%14,%15")
					.arg(prefixStep)
					.arg(person->Id().GetID())
					.arg(person->Name())
					.arg(person->Description())
					.arg(person->ModifyTime().UTC().toString())
					.arg(person->CurrentGeneralPose().Pose())
					.arg(person->IsDead()?"yes":"no")
					.arg(person->IsDormant()?"yes":"no")
					.arg(person->IsTraveling()?"yes":"no")
					.arg(person->IsNormal()?"yes":"no")
					.arg(person->Parent()->Name())
					.arg(person->Parent()->Description())
					.arg(person->Location().x)
					.arg(person->Location().y)
					.arg(person->Location().z);
			}
		}

		// Spacecraft
		DaAtomList spacecraft;
		system->FindVesselsRecursive(spacecraft);
		if(!spacecraft.isEmpty())
		{
			textLines << QString("%1 spacecraft")
				.arg(spacecraft.count());
			for(DaAtomList::const_iterator si = spacecraft.cbegin(); si != spacecraft.cend(); si++)
			{
				DaVessel* ship = (DaVessel*)(*si);
				textLines << QString("%1%2 %3, %4 lastMod %5 isDead %6 dormant %7 travel %8 normal %9 parent %10 %11 "
						"loc %12,%13,%14 moored %15")
					.arg(prefixStep)
					.arg(ship->Id().GetID())
					.arg(ship->Name())
					.arg(ship->Description())
					.arg(ship->ModifyTime().UTC().toString())
					.arg(ship->IsDead()?"yes":"no")
					.arg(ship->IsDormant()?"yes":"no")
					.arg(ship->IsTraveling()?"yes":"no")
					.arg(ship->IsNormal()?"yes":"no")
					.arg(ship->Parent()->Name())
					.arg(ship->Parent()->Description())
					.arg(ship->Location().x)
					.arg(ship->Location().y)
					.arg(ship->Location().z)
					.arg(ship->IsMoored()?"yes":"no");
			}
		}

		// Items
		DaAtomList all;
		system->ListNormalSubatomsRecursive(all);
		textLines << QString("%1 atoms subordinate to system")
			.arg(all.count());
		if(!all.isEmpty())
		{
			QMap<QString, int> atomsByType;	// Count of atoms by type.
			QMap<QString, int> itemCounts;				// Key is item name.
			for(DaAtomList::const_iterator ci = all.cbegin(); ci != all.cend(); ci++)
			{
				QString typeName(DaAtom::AtomTypeName((*ci)->AtomType()));
				QMap<QString, int>::iterator abt = atomsByType.find(typeName);
				if(abt == atomsByType.end())
					atomsByType.insert(typeName, 1);
				else
					(*abt)++;

				const DaItem* item = dynamic_cast<const DaItem*>(*ci);
				if(!item)
					continue;
				QMap<QString, int>::iterator ii = itemCounts.find(item->Name());
				if(ii == itemCounts.end())
					itemCounts.insert(item->Name(), 1);
				else
					(*ii)++;
			}
			if(!itemCounts.empty())
			{
				textLines << QString("%1 different items")
					.arg(itemCounts.count());
				for(QMap<QString, int>::iterator ii = itemCounts.begin(); ii != itemCounts.end(); ii++)
					textLines << QString("  %1 %2")
						.arg(ii.key())
						.arg(*ii);
			}

			// Troops
			QMap<QString, int> troopCounts;	// Key is parent name of the troop.
			for(DaAtomList::const_iterator ci = all.cbegin(); ci != all.cend(); ci++)
			{
				const DaCitizen* troop = dynamic_cast<const DaCitizen*>(*ci);
				if(!troop || !troop->IsNpcTroop())
					continue;
				const DaAtom* troopParent = troop->Parent();
				if(!troopParent)
					return;
				QMap<QString, int>::iterator ti = troopCounts.find(troopParent->Name());
				if(ti == troopCounts.end())
					troopCounts.insert(troopParent->Name(), 1);
				else
					(*ti)++;
			}
			if(!itemCounts.empty())
			{
				textLines << QString("%1 atoms with troops")
					.arg(troopCounts.count());
				for(QMap<QString, int>::iterator ti = troopCounts.begin(); ti != troopCounts.end(); ti++)
					textLines << QString("  %1 has %2 troops")
						.arg(ti.key())
						.arg(*ti);
			}

			textLines << QString("Subatom counts by type");
			for(QMap<QString, int>::const_iterator abt = atomsByType.cbegin(); abt != atomsByType.cend(); abt++)
				textLines << QString("  %1 %L2").arg(abt.key()).arg(abt.value());
		}
	}
	else
	{
		textLines << QString("Failed to load solar system id %1")
			.arg(systemId.GetID());
	}
}

//static
void DaAtom::DbQueryWorld(QStringList& textLines, QSqlDatabase& dbUniverse, const QString& worldName, const char* prefixStep)
{
	textLines << QString("Report World %1")
		.arg(worldName);
	QString sq;
	sq = QString("SELECT id FROM a_World WHERE name = '%1';").arg(worldName);
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qvi=0;		// query value index
		AuId worldId = AuId(qry.value(qvi++).toUInt());
		DbQueryWorld(textLines, dbUniverse, worldId, prefixStep);
		textLines << "<hr>";
	}
}

//static
void DaAtom::DbQueryWorld(QStringList& textLines, QSqlDatabase& dbUniverse, const AuId& worldId, const char* prefixStep)
{
	textLines << QString("ReportWorld %1")
		.arg(worldId.GetID());
	AuEther tempEther("DaAtom::DbQueryWorld");
	DaWorld* world = DaWorldGlobe::LoadSubatoms(tempEther, dbUniverse, AuId(0), LOAD_All, worldId);
	if(!world)
		world = DaWorldRingSection::LoadSubatoms(tempEther, dbUniverse, AuId(0), LOAD_All, worldId);
	if(!world)
		world = DaWorldGasGiant::LoadSubatoms(tempEther, dbUniverse, AuId(0), LOAD_All, worldId);
	if(!world)
		world = DaWorldRing::LoadSubatoms(tempEther, dbUniverse, AuId(0), LOAD_All, worldId);
	if(world)
	{
		// Geological features.
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(world);
		if(llworld)
		{
			textLines << "Geological Features";
			const AuGeologicalFeatures& geof = llworld->LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
				textLines << QString("%2%1")
					.arg((*gi)->Name())
					.arg(prefixStep);
			llworld->UnlockGeologicalFeatures();
		}

		DaAtomList subs;
		world->ListSubatoms(subs);
		if(!subs.empty())
		{
			DaAtomList subAtoms;
			world->ListNormalSubatomsRecursive(subAtoms);
			textLines << QString("%1 %2 has %3 subatoms %4 total subatoms")
				.arg(world->Id().GetID())
				.arg(world->Name())
				.arg(subs.count())
				.arg(subAtoms.count());

			// Immediate subatoms
			for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
				textLines << QString("%4%1 %2 %3")
					.arg((*ai)->Id().GetID())
					.arg((*ai)->Name())
					.arg((*ai)->Description())
					.arg(prefixStep);
		}
		else
			textLines << QString("%1 %2 loaded, no subatoms")
				.arg(world->Id().GetID())
				.arg(world->Name());

		// People
		DaAtomList people;
		world->FindPeopleRecursive(people);
		if(!people.isEmpty())
		{
			textLines << QString("%1 people")
				.arg(people.count());
			for(DaAtomList::const_iterator pi = people.cbegin(); pi != people.cend(); pi++)
			{
				DaPerson* person = (DaPerson*)(*pi);
				person->m_AgentBits = 0;	// Don't report person as "Entity" or some animal. :P
				textLines << QString("%1%2 %3, %4 lastMod %5 pose %6 isDead %7 dormant %8 travel %9 normal %10 parent %11 %12 "
						"loc %13,%14,%15")
					.arg(prefixStep)
					.arg(person->Id().GetID())
					.arg(person->Name())
					.arg(person->Description())
					.arg(person->ModifyTime().UTC().toString())
					.arg(person->CurrentGeneralPose().Pose())
					.arg(person->IsDead()?"yes":"no")
					.arg(person->IsDormant()?"yes":"no")
					.arg(person->IsTraveling()?"yes":"no")
					.arg(person->IsNormal()?"yes":"no")
					.arg(person->Parent()->Name())
					.arg(person->Parent()->Description())
					.arg(person->Location().x)
					.arg(person->Location().y)
					.arg(person->Location().z);
			}
		}

		// Spacecraft
		DaAtomList spacecraft;
		world->FindVesselsRecursive(spacecraft);
		if(!spacecraft.isEmpty())
		{
			textLines << QString("%1 spacecraft")
				.arg(spacecraft.count());
			for(DaAtomList::const_iterator si = spacecraft.cbegin(); si != spacecraft.cend(); si++)
			{
				DaVessel* ship = (DaVessel*)(*si);
				textLines << QString("%1%2 %3, %4 lastMod %5 isDead %6 dormant %7 travel %8 normal %9 parent %10 %11 "
						"loc %12,%13,%14 moored %15")
					.arg(prefixStep)
					.arg(ship->Id().GetID())
					.arg(ship->Name())
					.arg(ship->Description())
					.arg(ship->ModifyTime().UTC().toString())
					.arg(ship->IsDead()?"yes":"no")
					.arg(ship->IsDormant()?"yes":"no")
					.arg(ship->IsTraveling()?"yes":"no")
					.arg(ship->IsNormal()?"yes":"no")
					.arg(ship->Parent()->Name())
					.arg(ship->Parent()->Description())
					.arg(ship->Location().x)
					.arg(ship->Location().y)
					.arg(ship->Location().z)
					.arg(ship->IsMoored()?"yes":"no");
			}
		}

		// Items
		DaAtomList all;
		world->ListNormalSubatomsRecursive(all);
		textLines << QString("%1 atoms subordinate to world, reporting items and troops")
			.arg(all.count());
		if(!all.isEmpty())
		{
			QMap<QString, int> itemCounts;	// Key is item name.
			for(DaAtomList::const_iterator ci = all.cbegin(); ci != all.cend(); ci++)
			{
				const DaItem* item = dynamic_cast<const DaItem*>(*ci);
				if(!item)
					continue;
				QMap<QString, int>::iterator ii = itemCounts.find(item->Name());
				if(ii == itemCounts.end())
					itemCounts.insert(item->Name(), 1);
				else
					(*ii)++;
			}
			for(QMap<QString, int>::iterator ii = itemCounts.begin(); ii != itemCounts.end(); ii++)
				textLines << QString("%1 %2")
					.arg(ii.key())
					.arg(*ii);

			// Troops
			QMap<QString, int> troopCounts;	// Key is parent name of the troop.
			for(DaAtomList::const_iterator ci = all.cbegin(); ci != all.cend(); ci++)
			{
				const DaCitizen* troop = dynamic_cast<const DaCitizen*>(*ci);
				if(!troop || !troop->IsNpcTroop())
					continue;
				const DaAtom* troopParent = troop->Parent();
				if(!troopParent)
					return;
				QMap<QString, int>::iterator ti = troopCounts.find(troopParent->Name());
				if(ti == troopCounts.end())
					troopCounts.insert(troopParent->Name(), 1);
				else
					(*ti)++;
			}
			for(QMap<QString, int>::iterator ti = troopCounts.begin(); ti != troopCounts.end(); ti++)
				textLines << QString("%1 has %2 troops")
					.arg(ti.key())
					.arg(*ti);
		}
	}
	else
	{
		textLines << QString("Failed to load world id %1")
			.arg(worldId.GetID());
	}
}

void DaAtom::ListNormalSubatomIds(AuIdList& ids) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		ids.append((*it)->Id());
}

// Lists ids of all subatoms. Not recursive.
void DaAtom::ListSubatomIds(AuIdList& ids) const
{
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		ids.append((*it)->Id());
}

// Returns the number of subatoms that are not IsDead. Not recursive.
int DaAtom::SubatomCount() const
{
	DaAtomList subs;
	ListSubatoms(subs);
	return subs.LiveCount();
}

// Recursively invalidates the system transform of this atom and all its subatoms.
void DaAtom::InvalidateEnvTransform()
{
	if(IsEnvTransformValid())
	{
		m_EnvTransformValid = false;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		subs.InvalidateEnvTransform();
	}
}

void DaAtom::AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	subs.AdjustNavLocation(shipLoc, navLoc);
}

bool DaAtom::ListAtomsFromParent(DaAtomList &aPath, const DaAtom *pParent) const
{
	//if (Is(pParent->Id()))
	if(this == pParent)
	{
		// "I" am the desired parent atom. Return the path as is.
		return true;
	}

	DaAtom* parent = Parent();
	if(!parent)
		return false;	// Didn't find the real parent.

	// "I" am not the desired parent so prepend my pointer onto the path
	// list passed and work my way up the chain.
	aPath.prepend((DaAtom*)this);
	return parent->ListAtomsFromParent(aPath, pParent);
}

// Kills a subatom. It will be deleted during the next Tick cycle.
// Recursive.
void DaAtom::KillSubatom(const AuId &id)
{
	DaAtomList subs;
	ListSubatoms(subs);
	subs.KillAtom(id);
}

// Kills all subatoms. They will be deleted during the next Tick cycle.
// Recursive.
void DaAtom::KillSubatoms()
{
	DaAtomList subs;
	ListSubatoms(subs);
	subs.KillAtoms();
}

GALAXY DaAtom::Galaxy() const
{
	GALAXY v;
	m_EnvironLock.lockForRead();
	if(m_Environ)
	{
		v = m_Environ->Galaxy();
		m_EnvironLock.unlock();
	}
	else
	{
		m_EnvironLock.unlock();
		const DaAtom* parent = Parent();
		if(parent)
			v = parent->Galaxy();
		else
			v = GALAXY_Count;
	}
	if(v == GALAXY_Count)
	{
		// This is here because of a dormant ship that seemed to think it was in GALAXY_Count. Not sure why. Possibly did not
		// query environment properly due to dormancy?
		const DaGalaxy* galaxy = FindParentGalaxy();
		if(galaxy)
			v = galaxy->Galaxy();
	}
	return v;
}

void DaAtom::SetGravityNormal(const AuPoint& gN)
{
	if(!m_Environ)
		return;
	m_EnvironLock.lockForWrite();
	if(m_Environ)
		m_Environ->m_GravityNormal = gN;
	m_EnvironLock.unlock();
}

void DaAtom::SetTerrainNormal(const AuPoint& tN)
{
	if(!m_Environ)
		return;
	m_EnvironLock.lockForWrite();
	if(m_Environ)
		m_Environ->m_terNormal = tN;
	m_EnvironLock.unlock();
}

void DaAtom::CapsuleLocation(quint16& roomId, ROOMTYPE& roomType, QString& roomName) const
{
	const AuQueryEnvironment* qe = LockEnvironmentQuery();
	roomId = qe->m_CapRoomId;
	roomType = qe->m_CapRoomType;
	roomName = qe->m_CapRoomName;
	UnlockEnvironmentQuery();
}

void DaAtom::WorldCaptured(quint32 byEmpire)
{
	Q_UNUSED(byEmpire);
}

QString DaAtom::DebugName() const
{
	return Name();
}

QDebug operator<<(QDebug dbg, const DaAtom& a)
{
	const DaAtom* atom = &a;
	if(atom)
		dbg.nospace() << atom->Id() << atom->AtomTypeName() << atom->DebugName();
	else
		dbg.nospace() << "0Atom";
	return dbg.space();
}

void DaAtom::AvatarAccountActiveRecursive(const AuIdList& land)
{
	if(IsDead())
		return;
	DaAtomList subs;
	ListSubatoms(subs);
	subs.AvatarAccountActiveRecursive(land);
}

void DaAtom::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire)
{
	if(IsDead())
		return;
	DaAtomList subs;
	ListSubatoms(subs);
	subs.ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
}

QString DaAtom::TransporterTargetName() const
{
	return tr("%1 at %2").arg(Name()).arg(GlobeLL().ToString());
}

DaBd* DaAtom::FindPlatformLocationAtPixel(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp,
		const AuMatrix4x4 &tDeviceOrtho, AuPoint &fTerrainLoc) const
{
	// Get my parent planet.
	const DaWorldLL* planet = Parent()->FindParentWorldInEnv();
	if(!planet)
		return 0;		// No parent planet.

	// Get the reverse transformation back up to the parent.
	AuMatrix4x4 tModel;
	GetTransformRelativeToParent(tModel, planet);
	tModel = tModel.Inverse();

	// Ask the planet to find the terrain that contains the pixel point.
	return planet->FindPlatformLocationAtPixel(pxFind, tDevicePersp, tDeviceOrtho, tModel, fTerrainLoc);
}

DaAtom::ATOM_TYPE DaAtom::AtomType() const
{
	return ATOM_Atom;
}

bool DaAtom::OnRingworld() const
{
	return WorldAtomType() == ATOM_WorldRingSection;
}

// Packs this atom's item type and data into a byte array using operator>>. Use Unpack to restore.
QByteArray DaAtom::Pack() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_0);
	out << (quint8)AtomType();
	out << *this;
	return ba;
}

// Restores an atom from a byte array that was stored using Pack.
// The returned pointer is allocated on the heap and becomes the property of the caller.
//static
DaAtom* DaAtom::Unpack(const QByteArray& ba)
{
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_5_0);
	quint8 atomType;
	in >> atomType;
	if(!atomType)
	{
		qWarning() << "error: DaAtom::Unpack read atomType 0";
		return 0;
	}
	DaAtom* atom = Create((ATOM_TYPE)atomType);
	if(!atom)
		return 0;
	in >> *atom;
	return atom;
}

//static
DaAtom* DaAtom::Create(ATOM_TYPE atomType)
{
	switch(atomType)
	{
	case ATOM_Animal:
		return new DaAnimal();
	case ATOM_Arena:
		return new DaArena();
	case ATOM_Asteroid:
		return new DaAsteroid();
	case ATOM_Bd:
		return new DaBd();
	case ATOM_Designer:
		return new DaDesigner();
	case ATOM_DesignPlane:
		return new DaDesignPlane();
	case ATOM_Effect:
		return new DaEffect();
	case ATOM_Galaxy:
		return new DaGalaxy();
	case ATOM_Item:
		return new DaItem();
	case ATOM_Missile:
		return new DaMissile();
	case ATOM_Avatar:
		return new DaPerson();
	case ATOM_Npc:
		return new DaNpc();
	case ATOM_Sc:
		return new DaSc();
	case ATOM_Sector:
		return new DaSector();
	case ATOM_SolarSystem:
		return new DaSolarSystem();
	case ATOM_Star:
		return new DaStar();
	case ATOM_Teleporter:
		return new DaTeleporter();
	case ATOM_WorldGasGiant:
		return new DaWorldGasGiant();
	case ATOM_WorldGlobe:
		return new DaWorldGlobe();
	case ATOM_WorldRingSection:
		return new DaWorldRingSection();
	case ATOM_WorldRing:
		return new DaWorldRing();
	case ATOM_Universe:
		return new DaUniverse();
	case ATOM_Vehicle:
		return new DaVehicle();

	case ATOM_Invalid:
	case ATOM_Atom:
	case ATOM_Capsule:
	case ATOM_CitCharacter_Deprecated:
	case ATOM_CitCrew_Deprecated:
	case ATOM_CitIndigenous_Deprecated:
	case ATOM_Citizen:
	case ATOM_CitOfficer_Deprecated:
	case ATOM_CitPassenger_Deprecated:
	case ATOM_CitTroop_Deprecated:
	case ATOM_CitWorker_Deprecated:
	case ATOM_Creature:
	case ATOM_DesignBase:
	case ATOM_Sentient:
	case ATOM_Unit:
	case ATOM_Vessel:
	case ATOM_World:
	case ATOM_WorldLL:
	case ATOM_WorldSphere:
	case ATOM_Count:
		// These atom types cannot exist by themselves.
		break;
	}
	qWarning() << "error: DaAtom::Create invalid atomType" << atomType;
	return 0;
}

//static
QString DaAtom::AtomTypeName(ATOM_TYPE atomType)
{
	if(atomType < ATOM_Invalid || atomType >= ATOM_Count)
		return QString("Invalid Atom Type %1").arg(atomType);
	QString names[ATOM_Count] =
	{
		QObject::tr("Invalid"),
		QObject::tr("Atom"),
		QObject::tr("Animal"),
		QObject::tr("Arena"),
		QObject::tr("Asteroid"),
		QObject::tr("Avatar"),
		QObject::tr("Bd"),
		QObject::tr("Capsule"),
		QObject::tr("Citizen, Character DEPRECATED"),
		QObject::tr("Citizen, Crew DEPRECATED"),
		QObject::tr("Citizen, Indigenous DEPRECATED"),
		QObject::tr("Citizen"),
		QObject::tr("Citizen, Officer DEPRECATED"),
		QObject::tr("Citizen, Passenger DEPRECATED"),
		QObject::tr("Citizen, Troop DEPRECATED"),
		QObject::tr("Citizen, Worker DEPRECATED"),
		QObject::tr("Creature"),
		QObject::tr("Design Base"),
		QObject::tr("Designer"),
		QObject::tr("Design Plane"),
		QObject::tr("Effect"),
		QObject::tr("Galaxy"),
		QObject::tr("Item"),
		QObject::tr("Missile"),
		QObject::tr("Npc"),
		QObject::tr("Sc"),
		QObject::tr("Sector"),
		QObject::tr("Sentient"),
		QObject::tr("Solar System"),
		QObject::tr("Star"),
		QObject::tr("Teleporter"),
		QObject::tr("Unit"),
		QObject::tr("Universe"),
		QObject::tr("Vehicle"),
		QObject::tr("Vessel"),
		QObject::tr("World, Gas Giant"),
		QObject::tr("World, Globe"),
		QObject::tr("World"),
		QObject::tr("World, LL"),
		QObject::tr("World, Ring"),
		QObject::tr("World, Ringworld Arc Section"),
		QObject::tr("World, Sphere"),
	};
	return names[atomType];
}

int DaAtom::HighestWorldOrbitIndex(double orbitRadiusAu) const
{
	int highestOrbitIndex = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(*it);
		if(!globe)
			continue;
		if(!FuzzyEquals(globe->OrbitRadiusAu(), orbitRadiusAu))
			continue;
		if(highestOrbitIndex < globe->OrbitIndex())
			highestOrbitIndex = globe->OrbitIndex();
	}
	return highestOrbitIndex;
}

void DaAtom::SyncController(const AuSyncController& sync)
{
	if(!IsClientControlled())
		return;
	if(g_Au.IsSceneClient())
	{
#if !defined(QT_NO_DEBUG)
		qWarning() << "warning: DaAtom::SyncController" << *this << sync << "called on client";
#endif
		return;
	}
	m_ControlLock.lockForWrite();
	if(!m_SyncController)
	{
		m_SyncController = new AuSyncControllerList;
		m_SyncController->append(sync.Clone());
	}
	else
		m_SyncController->AppendReplace(sync);
	m_ControlLock.unlock();
}

void DaAtom::ApplySync()
{
	// Take my syncs.
	m_ControlLock.lockForWrite();
	AuSyncControllerList* d = m_SyncController;
	m_SyncController = 0;
	m_ControlLock.unlock();
	if(!d)
		return;

	// Apply my syncs to the args.
	bool env = false;
	for(AuSyncControllerList::const_iterator di = d->cbegin(); di != d->cend(); di++)
	{
		switch((*di)->Type())
		{
		case AuSyncController::SYNC_AtomPos:
			ChangePosAndOrient((*di)->Pos());
			env = true;
			break;
		case AuSyncController::SYNC_Vector:
			if((*di)->Vector().IsZero())
				SetAtRest();
			else
			{
				SetVector((*di)->Vector());
				SetInMotion();
			}
			break;
		case AuSyncController::SYNC_Motion:
			SetInMotion();
			break;
		case AuSyncController::SYNC_AtomPosVec:
			ChangePosAndOrient((*di)->Pos());
			env = true;
			if((*di)->Vector().IsZero())
				SetAtRest();
			else
			{
				SetVector((*di)->Vector());
				SetInMotion();
			}
			break;
		case AuSyncController::SYNC_Pose:
			TiltHeadTo((*di)->HeadTilt());
			SetGeneralPose((*di)->Pose(), (*di)->StationId());
			SetVehicleSeatIndex((*di)->SeatIndex());
			break;
		}
	}

	if(env)
		InvalidateEnvironment();

	// Cleanup syncs.
	delete d;
}

void DaAtom::ClearSync()
{
	// Delete my syncs.
	m_ControlLock.lockForWrite();
	AuSyncControllerList* d = m_SyncController;
	m_SyncController = 0;
	m_ControlLock.unlock();
	if(d)
		delete d;
}

void DaAtom::ControllerSync(AuAtomPos& pos, bool& atRest, AuPoint& vec, float& headTilt, AuPose& pose, quint16& stationId,
		quint8& seatIndex)
{
	// Take my syncs.
	m_ControlLock.lockForWrite();
	AuSyncControllerList* d = m_SyncController;
	m_SyncController = 0;
	m_ControlLock.unlock();
	if(!d)
		return;

	// Apply my syncs to the args.
	for(AuSyncControllerList::const_iterator di = d->cbegin(); di != d->cend(); di++)
	{
		switch((*di)->Type())
		{
		case AuSyncController::SYNC_AtomPos:
			pos = (*di)->Pos();
			//qDebug() << "DaAtom::ControllerSync" << *this << "AtomPos" << pos;
			break;
		case AuSyncController::SYNC_Vector:
			vec = (*di)->Vector();
			if(vec.IsZero())
				atRest = true;
			else
				atRest = false;
			break;
		case AuSyncController::SYNC_Motion:
			atRest = false;
			break;
		case AuSyncController::SYNC_AtomPosVec:
			pos = (*di)->Pos();
			atRest = (*di)->AtRest();
			vec = (*di)->Vector();
			//qDebug() << "DaAtom::ControllerSync" << *this << "AtomPosVec" << pos << "atRest" << atRest << "vec" << vec;
			break;
		case AuSyncController::SYNC_Pose:
			headTilt = (*di)->HeadTilt();
			pose = (*di)->Pose();
			stationId = (*di)->StationId();
			seatIndex = (*di)->SeatIndex();
			break;
		}
	}

	// Cleanup syncs.
	delete d;
}

void DaAtom::MovedByTurboLift()
{
	SetModified();
}

DaAtom* DaAtom::Parent() const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	return ether->FindAtom(ParentId());
}

void DaAtom::ListNormalSubatoms(DaAtomList& atoms) const
{
	AuEther* ether = Ether();
	if(ether)
		ether->ListNormalSubatoms(Id(), atoms);
}

void DaAtom::ListSubatoms(DaAtomList& atoms) const
{
	AuEther* ether = Ether();
	if(ether)
		ether->ListSubatoms(Id(), atoms);
}

// Recursively deletes atoms !InStarField.
// Actually only moves them to the trash atom.
// NOTE: When deleting atoms, be sure the root atom mutex is locked for write.
void DaAtom::TrashNotInStarField()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Determine who will be deleted.
	AuIdList atomsToDelete;
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* atom = *it;
		if(atom->InStarField())
			atom->TrashNotInStarField();		// The atom stays but go after its children.
		else
			atomsToDelete.append(atom->Id());	// The atom will be deleted.
	}
	ether->TrashAtoms(atomsToDelete);
}

void DaAtom::TrashSubatom(const AuId &id)	// Deletes a subatom and removes it now.
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	DaAtom* atom = FindSubatomId(id);
	if(!atom)
		return;	// The atom is not subordinate to me.
	ether->TrashAtom(id);
}

void DaAtom::TrashSubatoms()		// Deletes all subatoms and removes them now.
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	AuIdList subIds;
	ListSubatomIds(subIds);
	ether->TrashAtoms(subIds);
}

bool DaAtom::HasSubatoms() const
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	return ether->HasSubatoms(Id());
}

void DaAtom::ListSubatomsRecursive(DaAtomList& atoms) const
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	ether->ListSubatomsRecursive(Id(), atoms);
}

void DaAtom::ListNormalSubatomsRecursive(DaAtomList& atoms) const
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	ether->ListNormalSubatomsRecursive(Id(), atoms);
}

DaAtom* DaAtom::FindIdRecursive(const AuId &id) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!ether->HasSubatoms(Id()))
		return 0;

	DaAtom* atom = ether->FindNormalAtom(id);
	if(!atom)
		return 0;	// The atom id is not in the ether.

	// Insure the atom found is below this atom.
	if(!atom->FindParentAtomId(Id()))
		return 0;	// The atom is not subordinate to this atom at any level.

	// The atom was found.
	return atom;
}

DaAtom* DaAtom::FindLiveIdRecursive(const AuId &id)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;

	DaAtom* atom = ether->FindLiveAtom(id);
	if(!atom)
		return 0;	// The atom id is not in the ether.

	// Insure the atom found is below this atom.
	if(!atom->FindParentAtomId(Id()))
		return 0;	// The atom is not subordinate to this atom at any level.

	// The atom was found.
	return atom;
}

void DaAtom::ClearPendingDamage()
{
	// Must remove pending damage, i.e. bleeding, or the patient immediately dies after being defibrillated.
	m_DamageToApplyLock.lockForWrite();
	m_DamageToApply.clear();
	m_DamageToApplyLock.unlock();
}

const DaWorld* DaAtom::FindDestinationWorld(const AuId& id, const QString& name) const
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
	QString myName(Name());
	for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
	{
		const DaWorld* world = (const DaWorld*)(*pi);
		if(world->DefaultName(myName) == name)
			return world;
	}

	return 0;
}

bool DaAtom::IsInVisibleWater() const
{
	// The primary goal is to never return true if the player does not "see" visible water or lava under their feet.
	const AuQueryEnvironment* qe = LockEnvironmentQuery();

	// I determined the depth number by running along the "visible" water line on my homeworld, just in the water, to see what water
	// depth was reported by this debug message. I kept the deepest number I saw, which was about .1.
	// This means that sometimes it looks like I'm in the water but I'm not, based on the splashy sounds of my footsteps.
	// I'd rather find out the lava I see at my feet isn't killing me, instead of being killed by lava I can't see.
	//if(Is(6992) && qe->IsInWater())
		//qDebug() << "DaAtom::IsInVisibleWater" << *this << qe->m_WaterDepth;

	bool v = qe->IsInWater() && qe->m_WaterDepth > .1f;
	UnlockEnvironmentQuery();
	return v;
}

void DaAtom::SetAtRest()
{
	SetAtRestState(true);
	SetVector(AuPoint(0,0));
}

