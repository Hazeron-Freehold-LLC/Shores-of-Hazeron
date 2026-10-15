//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: damissile.cpp,v 1.145 2022/05/17 16:08:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "damissile.h"
//
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "ausensorblip.h"
#include "ausound.h"
#include "daarena.h"
#include "daatomlist.h"
#include "dacapsule.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworld.h"
#include "dgdata.h"
#include "psfiresmoketrail.h"
#include <QSqlError>
#include <QSqlQuery>

DaMissile::DaMissile(const AuItem& item, quint32 count, quint32 empireId, const AuId& targetId, const AuPoint& targetLoc) :
	DaItem(item)
{
	m_EmpireId = empireId;
	m_ExplodeShield = false;
	m_OwnerIgnored = false;
	m_TargetLoc = targetLoc;
	m_TargetId = targetId;

	AuItem* nitem = LockItemForWrite();
	nitem->SetCount(count);
	UnlockItem();

	m_AudioEngine = 0;
	m_SoundEngine = SOUND_None;
	m_pEngineTrail = 0;
}

DaMissile::DaMissile(const AuItem& item, quint32 count, quint32 empireId) : DaItem(item)
{
	m_EmpireId = empireId;
	m_OwnerIgnored = false;

	AuItem* nitem = LockItemForWrite();
	nitem->SetCount(count);
	UnlockItem();

	m_AudioEngine = 0;
	m_SoundEngine = SOUND_None;
	m_pEngineTrail = 0;
}

DaMissile::DaMissile()
{
	m_EmpireId = 0;
	m_OwnerIgnored = false;

	m_AudioEngine = 0;
	m_SoundEngine = SOUND_None;
	m_pEngineTrail = 0;
}

DaMissile::~DaMissile()
{
	if(m_pEngineTrail)
	{
		delete m_pEngineTrail;
		m_pEngineTrail = 0;
	}
}

DaMissile::DaMissile(const DaMissile &c) : DaItem(c)
{
	m_EmpireId = c.m_EmpireId;
	m_OwnerIgnored = c.m_OwnerIgnored;
	m_TargetLoc = c.m_TargetLoc;
	m_TargetId = c.m_TargetId;

	m_AudioEngine = c.m_AudioEngine;
	m_SoundEngine = c.m_SoundEngine;
	m_pEngineTrail = 0;	// The will be created when needed.
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaMissile::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaItem::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat |= 0x01;
	toClient << hasWhat;

	if(hasWhat & 0x01)
		toClient << m_EmpireId;	// For contrail color
}

// Receive updates to this atom from a server.
void DaMissile::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaItem::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat = 0;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
		fromServer >> m_EmpireId;
}

// Refresh data from valid counterpart.
bool DaMissile::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaItem::Refresh(pAtom, ari);

	const DaMissile* pA = (const DaMissile*)pAtom;
	m_EmpireId = pA->m_EmpireId;

	return false;
}

DaAtom *DaMissile::Clone() const
{
	return new DaMissile(*this);
}

QString DaMissile::Name() const
{
	return DaItem::Name();
}

QString DaMissile::SpotDescription(quint32 spotterEmpire) const
{
	return DaItem::SpotDescription(spotterEmpire);
}

void DaMissile::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	// Do not render distant missiles.
	if(pass != PASS_Near)
		return;

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, rmod.Matrix(Id()), ari.al, ari.fade);

	if(ItemCount() <= 1)
		DaItem::RenderItem(projection, ari, rmod, sceneAtom, renderLast, 1);
	else
	{
		double scale = ItemCount();
		DaItem::RenderItem(projection, ari, rmod, sceneAtom, renderLast, scale);
	}

	// Render engine trail in RenderLast.
	if(m_pEngineTrail)
		renderLast.Insert(rmod, this);
}

void DaMissile::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	DaItem::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);

	if(m_pEngineTrail)
	{
		AuMatrix4x4 tMe(rmod.Matrix(Id()));
		if(ItemCount() > 1)
		{
			double scale = ItemCount();
			tMe.Scale(scale, scale, scale);
		}

		// Render engine contrail.
		ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
		switch(CommodityId())
		{
		default:
			break;
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiRa:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiAc:
		//case AuCombatWeapon::CannonBall:
			{
				// Projectile renders points relative to the vehicle.
				const float fThrust = 500.f * c_UnitScale;
				m_pEngineTrail->SetVelocity(AuPoint(0, 0, -fThrust));
				if(m_pEngineTrail->Update())
					if(m_pEngineTrail->GetParticleCount())
						m_pEngineTrail->Render(projection, tMe, *ari.re);
			}
			break;
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			{
				// Rocket thrust renders points relative to the vehicle.
				m_pEngineTrail->SetNumToRelease(2);
				const float fThrust = 15.f * c_UnitScale;
				m_pEngineTrail->SetVelocity(AuPoint(0, 0, -fThrust));
				if(m_pEngineTrail->Update())
					if(m_pEngineTrail->GetParticleCount())
						m_pEngineTrail->Render(projection, tMe, *ari.re);
			}
			break;
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
			{
				// Rocket thrust renders points relative to the vehicle.
				m_pEngineTrail->SetNumToRelease(2);
				const float fThrust = 20.f * c_UnitScale;
				m_pEngineTrail->SetVelocity(AuPoint(0, 0, -fThrust));
				if(m_pEngineTrail->Update())
					if(m_pEngineTrail->GetParticleCount())
						m_pEngineTrail->Render(projection, tMe, *ari.re);
			}
			break;
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
			{
				// Gravity thrust renders points relative to the parent atom of this vehicle.
				AuMatrix4x4 tt = tMe * Transform().Inverse();
				m_pEngineTrail->SetNumToRelease(1);
				m_pEngineTrail->SetParent(ParentId(), Transform());
				if(m_pEngineTrail->Update())
					if(m_pEngineTrail->GetParticleCount())
						m_pEngineTrail->Render(projection, tt, *ari.re);
			}
			break;
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
			{
				// Gravity thrust renders points relative to the parent atom of this vehicle.
				AuMatrix4x4 tt = tMe * Transform().Inverse();
				m_pEngineTrail->SetNumToRelease(1);
				m_pEngineTrail->SetParent(ParentId(), Transform());
				if(m_pEngineTrail->Update())
					if(m_pEngineTrail->GetParticleCount())
						m_pEngineTrail->Render(projection, tt, *ari.re);
			}
			break;
		}
		ari.re->glDisable(GL_BLEND);
		ari.re->glDepthMask(GL_TRUE);
	}
}

// Render OpenAL audio sources
void DaMissile::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	float gain = RelativeAudioGain(sceneAtom) * fade;
	SOUND_EFFECT snd = SOUND_None;
	float pitch = 1;
	if(!IsFuzzyZero(gain) && HasItem())
	{
		// Sound is audible.
		switch(CommodityId())
		{
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
			{
				snd = SOUND_GravityDriveExt;
				gain *= ItemCount() + 5;
				pitch = 1.5f;
			}
			break;
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			{
				snd = SOUND_GravityDriveExt;
				gain *= 2.f;
				pitch = 2.f;
			}
			break;
		default:
			break;
		}
	}

	AuPoint epos = model.Translation();
	SOUND_EFFECT cur = (SOUND_EFFECT)m_SoundEngine;
	al->Play(m_AudioEngine, cur, snd, epos.x, epos.y, epos.z, gain, pitch, true,
			(float)AuSound::SoundReferenceDistance((SOUND_EFFECT)m_SoundEngine),
			AuSound::SoundRolloffFactor((SOUND_EFFECT)m_SoundEngine), 0);
	m_SoundEngine = cur;
}

// Drawn facing the negative Y direction with origin at 0,0.
void DaMissile::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaItem::CreateDeviceObjects(ari);

	if(!m_pEngineTrail)
	{
		// Get 3D extents of the 3ds model for positioning.
		AuPoint min,max;
		const AuItem* item = LockItem();
		if(item)
			item->ModelExtents(min, max);
		UnlockItem();

		double scale = 1;
		if(ItemCount() > 1)
			scale = ItemCount();

		switch(CommodityId())
		{
		default:
			break;
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiRa:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiAc:
			{
				// Projectile trails.
				GLuint texIdSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Smoke", ":data/smoke.png");

				m_pEngineTrail = new PsFireSmokeTrail();
				m_pEngineTrail->SetMaxParticles(100);
				m_pEngineTrail->SetNumToRelease(10);
				m_pEngineTrail->SetLifeCycle(0.5f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				QColor colFire;
				colFire.setRgbF(1, .5, .15);
				m_pEngineTrail->SetColor(colFire);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(scale);
				m_pEngineTrail->SetVelocityVar(0.5f);
				m_pEngineTrail->SetTextureID(texIdSmoke);
			}
			break;
			/*
		case AuCombatWeapon::CannonBall:
			{
				// Projectile trails.
				GLuint texIdSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Smoke", ":data/smoke.png");

				m_pEngineTrail = new PsFireSmokeTrail();
				m_pEngineTrail->SetMaxParticles(100);
				m_pEngineTrail->SetNumToRelease(10);
				m_pEngineTrail->SetLifeCycle(0.5f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				QColor colFire;
				colFire.setRgbF(1, .5, .15);
				m_pEngineTrail->SetColor(colFire);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(40.0f);
				m_pEngineTrail->SetVelocityVar(0.5f);
				m_pEngineTrail->SetTextureID(texIdSmoke);
			}
			break;
			*/
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			{
				// Rocket-type trails.
				GLuint texIdSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Smoke", ":data/smoke.png");

				m_pEngineTrail = new PsFireSmokeTrail();
				m_pEngineTrail->SetMaxParticles(200);
				m_pEngineTrail->SetNumToRelease(2);
				m_pEngineTrail->SetLifeCycle(1.0f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				QColor colFire;
				colFire.setRgbF(1, .25, .15);
				m_pEngineTrail->SetColor(colFire);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(10.0f*scale);
				m_pEngineTrail->SetVelocityVar(0.5f);
				m_pEngineTrail->SetTextureID(texIdSmoke);
			}
			break;
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
			{
				// Rocket-type trails.
				GLuint texIdSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Smoke", ":data/smoke.png");

				m_pEngineTrail = new PsFireSmokeTrail();
				m_pEngineTrail->SetMaxParticles(200);
				m_pEngineTrail->SetNumToRelease(2);
				m_pEngineTrail->SetLifeCycle(1.0f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				QColor colFire;
				colFire.setRgbF(1, .25, .15);
				m_pEngineTrail->SetColor(colFire);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(30.0f*scale);
				m_pEngineTrail->SetVelocityVar(0.5f);
				m_pEngineTrail->SetTextureID(texIdSmoke);
			}
			break;
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
			{
				// Gravity-drive like trails.
				GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
				GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

				// Get the contrail color of the empire.
				const DgData& gd = g_Au.LockDataForRead();
				QColor contrailColor = gd.EmpireContrailColor(EmpireId());
				g_Au.UnlockData();

				m_pEngineTrail = new PsEngineTrail();
				m_pEngineTrail->SetMaxParticles(35);
				m_pEngineTrail->SetLifeCycle(1.0f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				m_pEngineTrail->SetColor(contrailColor);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(600.0f * c_UnitScale);
				m_pEngineTrail->SetTextureID(texIdStreak);
				m_pEngineTrail->SetTextureID2(texIdGlow);
			}
			break;
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
			{
				// Gravity-drive like trails.
				GLuint texIdStreak = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EngineStreak", ":data/EngineStreak.png");
				GLuint texIdGlow = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");

				// Get the contrail color of the empire.
				const DgData& gd = g_Au.LockDataForRead();
				QColor contrailColor = gd.EmpireContrailColor(EmpireId());
				g_Au.UnlockData();

				m_pEngineTrail = new PsEngineTrail();
				m_pEngineTrail->SetMaxParticles(35);
				m_pEngineTrail->SetLifeCycle(1.0f);
				m_pEngineTrail->SetReleaseInterval(0.0f);
				m_pEngineTrail->SetColor(contrailColor);
				m_pEngineTrail->SetPosition(AuPoint(0, 0, min.z - 1));
				m_pEngineTrail->SetSize(1200.0f * c_UnitScale);
				m_pEngineTrail->SetTextureID(texIdStreak);
				m_pEngineTrail->SetTextureID2(texIdGlow);
			}
			break;
		}
	}
}

void DaMissile::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaItem::DeleteDeviceObjects(ari);

	// OpenAL audio sources
	ari.StopSound(m_AudioEngine);
	m_SoundEngine = SOUND_None;
}

// Insert first time database record.
void DaMissile::Insert(QSqlDatabase& /*db*/) const
{
	// Missiles do not live long enough to bother storing into the database.
}

// Update existing database record.
void DaMissile::Update(QSqlDatabase& /*db*/) const
{
	// Missiles do not live long enough to bother storing into the database.
}

void DaMissile::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaItem::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
}

bool DaMissile::IsTicker() const
{
	if(IsDead())
		return false;
	return true;
}

void DaMissile::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	DaItem::Tick(last, now);
	if(IsDead())
		return;

	// All sizes of missiles explode if they touch the ground. This includes the deck of a ship or floor of a building.
	if(IsOnGround() && CreatedTime().ElapsedMSec() > 50)
	{
		//qDebug() << "DaMissile::Tock" << Id() << Name() << "Explode on ground, secs" << CreatedTime().ElapsedSec()
			//<< OnGround();
		m_MissileLock.lockForWrite();
		m_ExplodeLoc = Location();
		m_ExplodeTarget = ParentId();
		m_ExplodeShield = false;

		// When exploding due to hitting a world, put the explosion at the level of the dirt.
		float alt = AltitudeAGL();
		if(alt < 0)
		{
			// Take a quick stab at getting the explosion above ground.
			AuPoint vec(Transform().ProjectVector(Vector()));	// Vector in the direction of my movement, relative to the world.
			vec.Unit(alt*1.414 / 10);	// AGL is negative so the direction of the vector is reversed to point back.
			m_ExplodeLoc += vec;				// Back up along my path, hopefully out of the dirt.
		}
		m_MissileLock.unlock();
	}
	else
	{
		// Locate a proximity target.
		// Only check peer atoms of my same parent atom. There's no need to get more thorough than that.
		const AuItem* item = LockItem();
		float proximityRad2 = item->BlastRadius();				// my blast radius
		UnlockItem();

		proximityRad2 /= 4;		// proximity radius is smaller than my blast radius
		AuId aid;
		if(m_OwnerIgnored)
			aid = ArmedBy();
		DaAtom* parent = Parent();
		DaAtomList subs;
		parent->ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator pi = subs.cbegin(); pi != subs.cend(); pi++)
		{
			// Do not hit the atom that launched me.
			if((*pi)->Is(aid))
				continue;

			// Watch for hits on spacecraft, vehicles and buildings.
			int hit = (*pi)->TestMissileHit(Location(), proximityRad2);
			if(hit)
			{
				m_MissileLock.lockForWrite();
				m_ExplodeLoc = Location();		// Explode at my location relative to our common parent.
				m_ExplodeTarget = (*pi)->Id();
				m_ExplodeShield = hit == 2;
				m_MissileLock.unlock();
				break;							// Do nothing more once I have decided to explode.
			}
		}
	}

	// Explode near proximity target.
	m_MissileLock.lockForRead();
	if(!m_ExplodeTarget.IsNull())
	{
		// Only check peer atoms of my same parent atom. There's no need to get more thorough than that.
		//qDebug() << "DaMissile::Tick" << Id() << Name() << "exploding";
		if(ParentId() == m_ExplodeTarget)
			Explode(Parent(), m_ExplodeLoc, m_ExplodeShield);
		else
		{
			DaAtom* parent = Parent();
			if(parent)
			{
				DaAtom* target = parent->FindSubatomId(m_ExplodeTarget);
				if(target)
					Explode(target, m_ExplodeLoc, m_ExplodeShield);
				else
					Explode(parent, m_ExplodeLoc, m_ExplodeShield);
			}
		}
		m_MissileLock.unlock();
		return;
	}

	bool s = !m_TargetId.IsNull() && !m_TurnM.IsIdentity();
	m_MissileLock.unlock();

	// Seeking missile turns toward its target.
	if(s)
	{
		m_MissileLock.lockForWrite();
		AuMatrix4x4 turn(m_TurnM);
		m_TurnM.LoadIdentity();			// Until I decide to turn again.
		m_MissileLock.unlock();
		AuAtomPos pos(Pos());
		pos.TransformBy(turn);
		ChangePosNotParent(pos);
	}

	// Determine the lifetime of this ordnance based on typical attack range.
	float lifetime;
	if(!Vector().IsZero())
	{
		if(DbCommodity::IsDeployableWeapon(CommodityId()))
		{
			// Lifetime is extended for dead fall munitions so they have time to hit the ground.
			lifetime = (float)(AttackRange()) * 1000.f / (float)Speed();	// in milliseconds
			if(IsInAtmosphere())
			{
				if(lifetime > 25000)
					lifetime = 25000;
			}
			else
			{
				if(lifetime > 15000)
					lifetime = 15000;
			}
		}
		else
		{
			// Range is doubled for accelerating missiles/rockets otherwise they peter out too quickly.
			lifetime = (float)(AttackRange()*2.) * 1000.f / (float)Speed();	// in milliseconds
			if(lifetime > 10000)
				lifetime = 10000;					// not to exceed 10 seconds
		}
	}
	else
	{
		if(DbCommodity::IsDeployableWeapon(CommodityId()))
		{
			if(IsInAtmosphere())
				lifetime = 25000;
			else
				lifetime = 15000;
		}
		else
			lifetime = 10000;
	}

	if(CreatedTime().ElapsedMSec() > lifetime)
		Kill();
}

void DaMissile::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Test normal in case I already exploded, like when my movement is blocked.
	if(IsNormal())
		Explode(0, Transform().Project(hitLoc), false);	// Explode kills this atom.
	else
		DaItem::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);
}

void DaMissile::operator>>(QDataStream& out) const
{
	DaItem::operator>>(out);
	out << m_EmpireId;
	out << m_OwnerIgnored;
	out << m_TargetId;
	out << m_TargetLoc;

	m_MissileLock.lockForRead();
	out << m_ExplodeLoc;
	out << m_ExplodeShield;
	out << m_ExplodeTarget;
	AuTime lastTurn;
	out << lastTurn;
	out << m_TurnM;
	m_MissileLock.unlock();
}

void DaMissile::operator<<(QDataStream& in)
{
	DaItem::operator<<(in);
	in >> m_EmpireId;
	in >> m_OwnerIgnored;
	in >> m_TargetId;
	in >> m_TargetLoc;

	m_MissileLock.lockForWrite();
	in >> m_ExplodeLoc;
	in >> m_ExplodeShield;
	in >> m_ExplodeTarget;
	AuTime lastTurn;
	in >> lastTurn;
	in >> m_TurnM;
	m_MissileLock.unlock();
}

// Missiles are not destroyed by hull stress.
void DaMissile::TestHullStress()
{
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaMissile::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	//qDebug() << "DaMissile::MovementBlocked" << Id() << Name();
	if(ExplodeTarget().IsNull())
	{
		// Calculate hit location relative to my parent atom.
		AuPoint h = EnvTransform().UnProject(hit->EnvTransform().Project(hitLoc));	// hitLoc relative to me.
		h = Transform().Project(h);	// hitLoc relative to my parent
		Explode(hit, h, hitShield);
	}
	return DaItem::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

bool DaMissile::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(sceneAtom);
	// Calculate scene atom location in my coordinate space.
	AuPoint sceneLoc = EnvTransform().UnProject(cameraEnvLoc);
	return sceneLoc.Length2() < (10000*FeetPerMeter() * 10000*FeetPerMeter());
}

// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
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
void DaMissile::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	Q_UNUSED(scannerLoc);
	Q_UNUSED(scannerSensorRange);
	Q_UNUSED(scannerWeaponRange);
	// Must scan me if looking for locked-on blip and I'm it.
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles and AI turret gunners.
		if(!(scanContent & 0x0021))
			return;
	}

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	if(IsInSpacecraft())
		return;	// Missiles don't show up when aboard a spacecraft.

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);
	const AuItem* item = LockItem();
	if(item)
		blips.Add(AuBlipMissile(item->CommodityId(), item->Quality(), scannerId == m_TargetId, item->Count(), EmpireId(), stance,
					Id(), EnvLocation()));
	UnlockItem();
}

const double s_LargeMissileAccel = 800.*FeetPerMeter();
const double s_SmallMissileAccel = 1000.*FeetPerMeter();
const double s_LargeMissileFric = .5;
const double s_SmallMissileFric = .25;

double DaMissile::MuzzleExitVelocity() const
{
	switch(CommodityId())
	{
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
		return s_LargeMissileAccel;

	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
		return s_SmallMissileAccel;

	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiAc:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiRa:
		return 500 * FeetPerMeter() * c_UnitScale;			// 500 mps muzzle velocity for big guns

	default:
		break;
	}
	return 0;
}

// Acceleration/deceleration in ft/sec/sec due to gravity and other factors.
AuPoint DaMissile::Acceleration() const
{
	// Gravity is usually applied whenever this vehicle is not touching the
	// ground. There are various exceptions handled in DaAtom::Acceleration.
	int applyGravity = 1;

	AuPoint accel;
	switch(CommodityId())
	{
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		{
			// Engine power will overcome the force of gravity.
			// Gravity is not applied when outside the atmosphere.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Forward/reverse acceleration.
			accel.z = s_LargeMissileAccel;
		}
		break;
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
		{
			// Engine power will overcome the force of gravity.
			// Gravity is not applied when outside the atmosphere.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Forward/reverse acceleration.
			accel.z = s_SmallMissileAccel;
		}
		break;
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
		{
			// Gravity is not applied when outside the atmosphere.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Forward/reverse acceleration.
			accel.z = s_LargeMissileAccel;
		}
		break;
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
		{
			// Gravity is not applied when outside the atmosphere.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;

			// Forward/reverse acceleration.
			accel.z = s_SmallMissileAccel;
		}
		break;
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiAc:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiRa:
		{
			// Gravity is not applied when outside the atmosphere.
			if(!IsInAtmosphere() && !InBlackHoleGravity())
				applyGravity = 0;
		}
		break;

	default:
		break;
	}

	// Add acceleration due to gravity.
	if(applyGravity)
		accel += DaItem::Acceleration();

	AuPoint vec(Vector());
	switch(CommodityId())
	{
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		{
			// Braking by the space fighter must be applied after gravity in order
			// to counteract it properly. In fact, it simply ignores it since the
			// fighter can always produce more thrust than gravity. This also makes
			// the calculation a lot easier.
			// Lateral braking.
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = (float)s_LargeMissileAccel;
				accel.x = -vec.x*2;	// brake with opposite force
				if(accel.x < 0)
				{
					if(accel.x < -maxBrakeThrust)
						accel.x = -maxBrakeThrust;
				}
				else
				{
					if(accel.x > maxBrakeThrust)
						accel.x = maxBrakeThrust;
				}
			}

			// Vertical braking.
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = (float)s_LargeMissileAccel;
				accel.y = -vec.y*2;	// brake with opposite force
				if(accel.y < 0)
				{
					if(accel.y < -maxBrakeThrust)
						accel.y = -maxBrakeThrust;
				}
				else
				{
					if(accel.y > maxBrakeThrust)
						accel.y = maxBrakeThrust;
				}
			}
		}
		break;
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
		{
			// Braking by the space fighter must be applied after gravity in order
			// to counteract it properly. In fact, it simply ignores it since the
			// fighter can always produce more thrust than gravity. This also makes
			// the calculation a lot easier.
			// Lateral braking.
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = (float)s_SmallMissileAccel;
				accel.x = -vec.x*2;	// brake with opposite force
				if(accel.x < 0)
				{
					if(accel.x < -maxBrakeThrust)
						accel.x = -maxBrakeThrust;
				}
				else
				{
					if(accel.x > maxBrakeThrust)
						accel.x = maxBrakeThrust;
				}
			}

			// Vertical braking.
			{
				// Use engine braking to stop.
				const float maxBrakeThrust = (float)s_SmallMissileAccel;
				accel.y = -vec.y*2;	// brake with opposite force
				if(accel.y < 0)
				{
					if(accel.y < -maxBrakeThrust)
						accel.y = -maxBrakeThrust;
				}
				else
				{
					if(accel.y > maxBrakeThrust)
						accel.y = maxBrakeThrust;
				}
			}
		}
		break;
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
		{
			// Lateral braking.
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = (float)s_LargeMissileAccel;

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.x + vec.x;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.x -= brakeForce;
				else
					accel.x += brakeForce;
			}

			// Vertical braking.
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = (float)s_LargeMissileAccel;

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.y + vec.y;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.y -= brakeForce;
				else
					accel.y += brakeForce;
			}
		}
		break;
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
		{
			// Lateral braking.
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = (float)s_SmallMissileAccel;

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.x + vec.x;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.x -= brakeForce;
				else
					accel.x += brakeForce;
			}

			// Vertical braking.
			{
				// Use engine braking to stop.
				// Calculate maximum braking on this axis.
				float brakeForce = (float)s_SmallMissileAccel;

				// Calculate braking force needed to reduce axis acceleration to zero.
				float nuaccel = accel.y + vec.y;				// New acceleration after 'accel' added to current vector.
				if(brakeForce > fabs(nuaccel))
					brakeForce = fabs(nuaccel);

				// Brake with opposite force vs nu acceleration.
				if(nuaccel > 0)
					accel.y -= brakeForce;
				else
					accel.y += brakeForce;
			}
		}
		break;
	default:
		break;
	}

	return accel;
}

// Returns friction in ft/sec/sec, typically due to surface contact.
AuPoint DaMissile::Friction() const
{
	// Determine friction forward and lateral due to ground surface contact.
	// Assumes I'm oriented to the surface appropriately.
	double fricFwd = 0, fricLat = 0, fricVrt = 0;	// forward is +Z. lateral is X and vertical is Y.
	AuPoint vec(Vector());
	switch(CommodityId())
	{
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		if(IsInAtmosphere())
		{
			// Inside atmosphere range of a planet.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 10;
				fricLat = 10;
				fricVrt = 10;
			}
			else
			{
				fricFwd = s_LargeMissileFric;
				fricLat = s_LargeMissileFric;
				fricVrt = s_LargeMissileFric;
			}
		}
		else
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(fabs(vec.x) < 2)
				fricLat = .5;
			if(fabs(vec.y) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag
						// from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += 20;
			fricLat += 100;
			break;
		case OG_Snow:
			fricFwd += 30;
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Metal:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Pavement:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Rock:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Sand:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Savanna:
			fricFwd += 15;
			fricLat += 75;
			break;
		}

		// Space rocket sits on its tail, hence the vertical friction.
		fricVrt = fricLat;
		break;
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
		if(IsInAtmosphere())
		{
			// Inside atmosphere range of a planet.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 10;
				fricLat = 10;
				fricVrt = 10;
			}
			else
			{
				fricFwd = s_SmallMissileFric;
				fricLat = s_SmallMissileFric;
				fricVrt = s_SmallMissileFric;
			}
		}
		else
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(fabs(vec.x) < 2)
				fricLat = .5;
			if(fabs(vec.y) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag
						// from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += 20;
			fricLat += 100;
			break;
		case OG_Snow:
			fricFwd += 30;
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Metal:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Pavement:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricFwd += 10;
			fricLat += 50;
			break;
		case OG_Rock:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Sand:
			fricFwd += 15;
			fricLat += 75;
			break;
		case OG_Savanna:
			fricFwd += 15;
			fricLat += 75;
			break;
		}

		// Space rocket sits on its tail, hence the vertical friction.
		fricVrt = fricLat;
		break;
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
		if(IsInAtmosphere())
		{
			// Inside atmosphere range of a planet.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 10;
				fricLat = 10;
				fricVrt = 10;
			}
			else
			{
				fricFwd = s_LargeMissileFric;
				fricLat = s_LargeMissileFric;
				fricVrt = s_LargeMissileFric;
			}
		}
		else
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(fabs(vec.x) < 2)
				fricLat = .5;
			if(fabs(vec.y) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag
						// from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricLat += 100;
			break;
		case OG_Snow:
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricLat += 50;
			break;
		case OG_Metal:
			fricLat += 50;
			break;
		case OG_Pavement:
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricLat += 50;
			break;
		case OG_Rock:
			fricLat += 75;
			break;
		case OG_Sand:
			fricLat += 75;
			break;
		case OG_Savanna:
			fricLat += 75;
			break;
		}

		// Space rocket sits on its tail, hence the vertical friction.
		fricVrt = fricLat;
		break;
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
		if(IsInAtmosphere())
		{
			// Inside atmosphere range of a planet.
			if(WaterDepth() > 1.f)
			{
				// In water induces some drag.
				fricFwd = 10;
				fricLat = 10;
				fricVrt = 10;
			}
			else
			{
				fricFwd = s_SmallMissileFric;
				fricLat = s_SmallMissileFric;
				fricVrt = s_SmallMissileFric;
			}
		}
		else
		{
			// Outside atmosphere range of a planet and maneuver drives are
			// operating. Induce some friction if the throttles aren't on so
			// the ship will grind to a halt.
			// I could be inside a spacecraft in space.
			if(fabs(vec.x) < 2)
				fricLat = .5;
			if(fabs(vec.y) < 2)
				fricVrt = .5;
		}

		// Add drag induced by terrain and also braking.
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag
						// from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricLat += 100;
			break;
		case OG_Snow:
			fricLat += 150;
			break;
		case OG_HardFloor:
			fricLat += 50;
			break;
		case OG_Metal:
			fricLat += 50;
			break;
		case OG_Pavement:
			fricLat += 50;
			break;
		case OG_WoodFloor:
			fricLat += 50;
			break;
		case OG_Rock:
			fricLat += 75;
			break;
		case OG_Sand:
			fricLat += 75;
			break;
		case OG_Savanna:
			fricLat += 75;
			break;
		}

		// Space rocket sits on its tail, hence the vertical friction.
		fricVrt = fricLat;
		break;
	default:
		break;
	}
	return AuPoint(fricLat, fricVrt, fricFwd);
}

// 'explodeLoc' is the location of the explosion relative to this missile's parent atom.
void DaMissile::Explode(DaAtom* target, const AuPoint& explodeLoc, bool hitShield)
{
	//qDebug() << "DaMissile::Explode" << *this << "armed by" << ArmedBy() << "target" << *target;
	if(target)
	{
		if(Is(target->Id()))
			qWarning() << "warning: DaMissile::Explode" << *this << "target" << *target << " is this missile BOOM!";
		else if(target->Is(ArmedBy()))
			qWarning() << "error: DaMissile::Explode" << *this << "target" << *target << "armed this missile BOOM!";
	}

	// Roll for a dud based on Quality.
	const AuItem* item = LockItem();
	int countExploded = item->TestFailure(item->Count());
	if(!countExploded)
	{
		UnlockItem();
		Kill();
		return;
	}

	float gain = (item->BlastRadius() * countExploded) / 5.f;

	AuCombatWeapon weapon(item, countExploded);

	UnlockItem();

	// Create effect of missile explosion.
	if(target)
	{
		// Add the effect to the target.
		AuPoint eloc;
		if(target == Parent())
			eloc = explodeLoc;	// already in the space of the target atom
		else
			eloc = target->Transform().UnProject(explodeLoc);	// in the space of the target atom
		AuEffect effect(eloc, Rotation(), AuPoint(0,0), SOUND_HitExplosion, gain, 1, VISUAL_Explosion);
		effect.SetShieldHit(hitShield);
		if(weapon.m_w1.m_Effect != WE_None && weapon.m_w1.m_Radius > 0)
			effect.SetExplosionEffect1(weapon.m_w1.m_Effect, weapon.m_w1.m_Radius);
		if(weapon.m_w2.m_Effect != WE_None && weapon.m_w2.m_Radius > 0)
			effect.SetExplosionEffect2(weapon.m_w2.m_Effect, weapon.m_w2.m_Radius);
		target->AddEffect(effect);
		//qDebug() << "DaMissile::Explode" << Id() << Name() << "exploded on target" << target->Id() << target->Name()
			//<< "hitShield" << hitShield;
	}
	else
	{
		DaAtom* parent = Parent();
		if(parent)
		{
			// Create an effect atom to carry the explosion effect.
			DaEffect* explode = parent->CreateEffect();

			// Give it my vector.
			AuPoint explodeRot = Rotation();
			AuPoint explodeVec = Transform().ProjectVector(Vector());

			AuEffect effect(explodeLoc, explodeRot, explodeVec, SOUND_HitExplosion, gain, 1, VISUAL_Explosion);
			if(weapon.m_w1.m_Effect != WE_None && weapon.m_w1.m_Radius > 0)
				effect.SetExplosionEffect1(weapon.m_w1.m_Effect, weapon.m_w1.m_Radius);
			if(weapon.m_w2.m_Effect != WE_None && weapon.m_w2.m_Radius > 0)
				effect.SetExplosionEffect2(weapon.m_w2.m_Effect, weapon.m_w2.m_Radius);
			explode->AddEffect(effect);
		}
	}

	AuCombatAttacker attacker(ArmedBy(), OwnerName(), ArmedByEmpire());

	AuPoint hitLoc = Transform().UnProject(explodeLoc);	// hit location relative to the attacker, which is me
	AuCombatDefender defender(target, hitLoc, hitShield, 0);

	AuCombatResult result;

	SetDormant();		// So I am ignored by my explosion.
	AttackTarget(attacker, defender, weapon, true, result);

	if(!hitShield)
		MakeCrater(hitLoc, weapon);

	if(!attacker.m_Id.IsNull() && dynamic_cast<DaUnit*>(target))
	{
		// Missile launched by a ship needs to notify the ship of the results, for mission completion.
		DaAtom* system = FindEnvironmentAtom();
		DaVessel* ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(attacker.m_Id));
		if(ship)
		{
			// Owner of the missile is a ship.
			if(!ship->IsPirateWithFreeWill())
			{
				// Send notification on the crew channel when a unit is hit.
				AuCommDamageInflicted msg(target->Name(), result.m_CriticalHits, result.m_IsDestroyed,
						weapon.m_w1.m_Effect, result.m_Damage1, result.Armor1(),
						weapon.m_w2.m_Effect, result.m_Damage2, result.Armor2(),
						AuCommStdChannel::CrewChannelName(), ship->Id(), ship->Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
				msg.SetScopeData(ship->Id().GetID());			// Required for CrewMates message to be valid.
				g_Au.SendAtomChat(msg);
			}

			if(result.m_IsDestroyed && (ship->IsSpaceStation() || ship->HasFirstOfficer()))
			{
				AuMissionOrder::TYPE t = ship->MissionOrderType();
				if((t == AuMissionOrder::Attack || t == AuMissionOrder::Plunder) &&
						target->Is(ship->MissionOrderTarget()) &&
						ship->IsCommandedByAI())
					ship->MissionOrderComplete(ship->FindCommandingOfficer(), 1);	// Destroyed target of ship's attack order.

				if(!ship->IsPirateWithFreeWill())
				{
					// Send a message on the friend channel.
					// Do this when someone is the captain, which could only be true if they were an agent. ;)
					QString text = tr("%1 destroyed.").arg(target->Name());
					AuCommChat msg(text, AuCommStdChannel::FriendChannelName(), ship->Id(), ship->Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
					msg.SetScopeData(ship->EmpireId());	// Friend channel needs my empire id to know the scope of the message.
					g_Au.SendAtomChat(msg);
				}
			}
		}
	}
	Kill();	// It's ok to trash me now.
}

DaAtom::COLLISION_TYPE DaMissile::CheckCollisionRelativeToAtom(DaAtom* pParent, const AuPoint &foldloc, const AuPoint &fnuloc,
		DaAtom*& pHit, AuPoint& hitLoc, AuPoint& hitNormal, const AuBox& fMoveExt)
{
	// Check to see if my movement is blocked by my parent atom.
	// My owner/launcher atom is exempt.
	AuId aid;
	if(m_OwnerIgnored)
		aid = ArmedBy();
	if(!pParent->Is(aid) && pParent->IsPathBlocked(foldloc, fnuloc, hitLoc, hitNormal, this))
	{
		hitLoc = Transform().UnProject(hitLoc);	// relative to me
		return CT_HitParent;
	}

	AuBox fext;
	AuPoint myoldloc = Transform().UnProject(foldloc);		// loc relative to me.
	AuPoint mynuloc = Transform().UnProject(fnuloc);		// loc relative to me.
	DaAtomList subs;
	pParent->ListNormalSubatoms(subs);						// Get a list of my peer atoms from my parent.
	for(DaAtomList::const_iterator it = subs.begin(); it != subs.end(); it++)
	{
		DaAtom* pAtom = *it;
		if(Is(pAtom->Id()))
			continue;	// Don't collide with myself.

		// Check did atom "enter" a peer, e.g. through a doorway.
		if(pAtom->CheckPeerJoin(foldloc, fnuloc, this))
		{
			pHit = pAtom;
			return CT_JoinedPeer;
		}

		if(!pAtom->IsObstacle(this))
			continue;	// Atom is not an obstacle to movement.

		if(pAtom->DoesQueryEnvironment())
		{
			// Missiles and rockets fired by the city do not hit atoms that are inside buildings of the launching city.
			// Those atoms will show up as peers of this missile as it travels through the city.
			// Only projectiles fired from bay weapons are owned by the city. Turret gun bullets are owned by the gunner.
			// This logic lets bases fire without hitting their own city buildings, like a ship fires without hitting its own hull.
			if(pAtom->BdId() == aid && pAtom->IsInsideBd())
				continue;
		}

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
				// Determine exact location to explode.
				unsigned int sightedPart = 0;
				bool sightedShield = false;
				AuPoint eyeLoc(myoldloc);
				AuPoint sightedLoc(mynuloc);
				DaAtom* parent = Parent();
				DaAtom* hit = parent ?
					parent->SightTarget(this, eyeLoc, Transform().Inverse(), 0, sightedLoc, sightedPart, sightedShield, 0) : 0;
				if(hit && !hit->Is(aid))
				{
					//qDebug() << "DaMissile::CheckCollisionRelativeToParent" << Id() << Name() << "hit" << hit->Id() <<
						//hit->Name() << "hitLoc" << sightedLoc;
					hitLoc = hit->EnvTransform().UnProject(EnvTransform().Project(sightedLoc));	// Relative to hit atom.
					pHit = hit;
					m_MissileLock.lockForWrite();
					m_ExplodeTarget = hit->Id();
					m_ExplodeLoc = Transform().Project(sightedLoc);	// Relative to my parent.
					m_ExplodeShield = sightedShield;
					m_MissileLock.unlock();
					return CT_HitPeer;
				}
			}
			else
			{
				// Explode when the extents of another atom are intersected by mine.
				AuPoint atomLoc = pAtom->Location();	// relative to parent
				hitNormal = fnuloc - atomLoc;
				hitNormal = t.UnProjectVector(hitNormal);	// Convert normal into space of hit atom.
				hitNormal.Unit();
				hitLoc = t.UnProject(atomLoc);	// relative to hit atom
				pHit = pAtom;
				// Determine exact location to explode.
				m_MissileLock.lockForWrite();
				m_ExplodeTarget = pHit->Id();
				m_ExplodeLoc = pHit->Location();	// Relative to my parent.
				m_ExplodeShield = false;
				m_MissileLock.unlock();
				return CT_HitPeer;
			}
		}
		else
		{
			// Extents don't intersect so we will at least check to see if the path is blocked.
			// Calculate locations relative to the atom.
			AuPoint oloc = t.UnProject(foldloc);
			AuPoint nloc = t.UnProject(fnuloc);
			if(pAtom->IsPathBlocked(oloc, nloc, hitLoc, hitNormal, this))
			{
				pHit = pAtom;
				// Determine exact location to explode.
				m_MissileLock.lockForWrite();
				m_ExplodeTarget = pHit->Id();
				m_ExplodeLoc = pHit->Transform().Project(hitLoc);	// Relative to my parent.
				m_ExplodeShield = false;
				m_MissileLock.unlock();
				return CT_HitPeer;
			}
		}
	}
	return CT_Miss;
}

bool DaMissile::IsPirate() const
{
	bool isPirate;
	if(InArena())
	{
		isPirate = false;
		if(EmpireId() == DgEmpire::ArenaTeam2Id())
		{
			// I could be an AI controlled team in the arena. Then I am considered a pirate.
			const DaArena* arena = FindParentArena();
			if(arena && arena->MatchType() == DaArena::SpacecraftTrial)
				isPirate = true;
		}
	}
	else
		isPirate = DgEmpire::IsPirate(EmpireId());
	return isPirate;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaMissile::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerPosture);
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
	targetSysLoc = EnvLocation();
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaMissile::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
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

	targetSysLoc = EnvLocation();
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaMissile::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	Q_UNUSED(gunnerPosture);
	if(!IsInAtmosphere())
		return false;	// Turrets don't reach out of the atmosphere.

	if(gunLoc.Distance2(Location()) > gunRange*gunRange)
		return false;	// I am out of range.

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return false;	// I am not an enemy.

	// I am a target.
	targetLoc = Transform().Project(TargetLocation());
	return true;
}

unsigned int DaMissile::IsTocker() const
{
	unsigned int ms = DaItem::IsTocker();
	if(!IsNormal())
		return ms;
	if(ms == 0 || ms > 250)
		ms = 250;
	return ms;
}

void DaMissile::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaItem::Tock(last, now, scripts, tockTrans);

	//qDebug() << "DaMissile::Tock" << Id() << Name() << "gnd" << IsOnGround()
		//<< "inAtm" << IsInAtmosphere() << "altAGL" << AltitudeAGL() << Location() << "expTarg" << m_ExplodeTarget
		//<< "target" << m_TargetId << "targLoc" << m_TargetLoc;
	if(!m_ExplodeTarget.IsNull())
		return;							// Do nothing more once I have decided to explode.

	// Rockets and missiles explode when inside any spacecraft or inside a building that is not in the launching city.
	AuId aid;
	if(m_OwnerIgnored)
		aid = ArmedBy();
	if(IsInsideSpacecraft() || (IsInsideBd() && BdId() != aid))
	{
		m_MissileLock.lockForWrite();
		m_ExplodeLoc = Location();
		m_ExplodeTarget = ParentId();
		m_ExplodeShield = false;
		m_MissileLock.unlock();
		//qDebug() << "DaMissile::Tock" << Id() << Name() << "inside explode target" << m_ExplodeTarget;
		return;							// Do nothing more once I have decided to explode.
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Turn toward my target if I am a seeker.
	if(!m_TargetId.IsNull())
	{
		m_MissileLock.lockForWrite();
		if(m_LastTurn.isValid())
		{
			// Only seeking missiles have a target id. Seek it.
			DaAtom* target = FindEnvironmentAtom()->FindIdRecursive(m_TargetId);
			if(target)
			{
				// Turn toward the target I am seeking.
				// Target location relative to me.
				AuPoint targLoc = EnvTransform().UnProject(target->EnvTransform().Project(m_TargetLoc));
				AuPoint targVec(targLoc);
				targVec.Unit();
				float angleToTurn = AuPoint(0,0,1).AngleTo(targVec) * 2;	// Doubled because I am accelerating. Need to oversteer.
				//qDebug() << "DaMissile::Tock" << Id() << Name() << "angleToTurn" << (angleToTurn*RAD2DEGf);
				if(angleToTurn > DEG2RADf)
				{
					// I need to turn more than one degree to the target.
					float turnRate = 180.f * DEG2RADf;						// Turn rate in radians per second.
					float msElapsed = m_LastTurn.elapsed();
					if(msElapsed > 250.f)
						msElapsed = 250.f;
					float rsec = (float)msElapsed/1000.f;					// Real world secs that have passed.
					float maxTurn = turnRate * rsec;
					if(angleToTurn > maxTurn)
						angleToTurn = maxTurn;

					// Turn around the axis that is perpendicular to the turn.
					AuPoint turnAxis = AuPoint(0,0,1).Cross(targVec);
					turnAxis.Unit();

					m_TurnM.LoadIdentity();
					m_TurnM.RotateAxis(angleToTurn, turnAxis);
				}
			}
		}

		m_LastTurn.restart();
		m_MissileLock.unlock();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

quint64 DaMissile::MemoryUsage() const
{
	quint64 mem = DaItem::MemoryUsage();
	mem += sizeof(DaMissile) - sizeof(DaItem);
	return mem;
}

bool DaMissile::WeaponOnCollision(AuCombatAttacker& attacker, AuCombatWeapon& weapon, int hitShield, double kineticEnergy) const
{
	// Roll for a dud based on Quality.
	const AuItem* item = LockItem();
	int countExploded = item->TestFailure(item->Count());
	if(!countExploded)
	{
		UnlockItem();
		return DaItem::WeaponOnCollision(attacker, weapon, hitShield, kineticEnergy);	// Kinetic damage.
	}
	weapon = AuCombatWeapon(item, countExploded);
	UnlockItem();

	// Clear the radius effects because they will have their effect in Explode.
	if(weapon.m_w1.m_Radius > 0)
	{
		weapon.m_w1.m_Effect = WE_None;
		weapon.m_w1.m_Damage = 0;
		weapon.m_w1.m_DamageMod = 0;
		weapon.m_w1.m_Radius = 0;
	}
	if(weapon.m_w2.m_Radius > 0)
	{
		weapon.m_w2.m_Effect = WE_None;
		weapon.m_w2.m_Damage = 0;
		weapon.m_w2.m_DamageMod = 0;
		weapon.m_w2.m_Radius = 0;
	}
	if(weapon.m_w1.m_Effect == WE_None && weapon.m_w2.m_Effect == WE_None)
		return DaItem::WeaponOnCollision(attacker, weapon, hitShield, kineticEnergy);	// Kinetic damage.

	QString reason;
	if(hitShield)
		reason = tr("hit on shield of %1").arg(Name());
	else
		reason = tr("hit %1").arg(Name());
	attacker = AuCombatAttacker(reason);
	return true;
}

DaAtom::ATOM_TYPE DaMissile::AtomType() const
{
	return ATOM_Missile;
}

void DaMissile::FallStopped(double oldCgAlt, double nuCgAlt, double stopMove, int elapsedMs, int stopReason)
{
	Q_UNUSED(oldCgAlt);
	Q_UNUSED(nuCgAlt);
	Q_UNUSED(stopMove);
	Q_UNUSED(elapsedMs);
	Q_UNUSED(stopReason);
	m_MissileLock.lockForWrite();
	m_ExplodeLoc = Location();
	m_ExplodeTarget = ParentId();
	m_ExplodeShield = false;
	m_MissileLock.unlock();
}

