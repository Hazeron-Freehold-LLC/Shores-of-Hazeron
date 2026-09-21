//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacitizen.cpp,v 1.1137 2022/05/19 15:07:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dacitizen.h"
//
#include "auatomrenderinfo.h"
#include "aucombat.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausensorblip.h"
#include "ausound.h"
#include "ausphere.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dpbody.h"
#include "dpconsolesensor.h"
#include "dpconsoleshield.h"
#include "dpconsolesurgery.h"
#include "dpconsoleweapon.h"
#include <QSqlError>
#include <qsqlquery.h>

#define MOVE_NEAR					12	// Feet distance from target to stop within.

AuCitizenSound::AuCitizenSound(const QByteArray& soundData, AuSound& al)
{
	m_AudioBuffer = al.CreateAudioBuffer(soundData);
	m_IsPlaying = false;
}

AuCitizenSound::AuCitizenSound()
{
	m_AudioBuffer = 0;
	m_IsPlaying = false;
}

AuCitizenSound::AuCitizenSound(const AuCitizenSound& c)
{
	m_AudioBuffer = c.m_AudioBuffer;
	m_IsPlaying = c.m_IsPlaying;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaCitizen::DaCitizen(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage) :
	DaSentient(dna, chroma, hand, name, stage)
{
	m_Culled = 0;
	m_Lineage = LINEAGE_None;
	m_InitStand = 0;
	m_ShipBerthType = AuRosterEntry::Invalid;
	m_WeightCarried = 0;
}

DaCitizen::DaCitizen(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
		GROWTHSTAGE stage) : DaSentient(species, chroma, hand, name, stage)
{
	m_Culled = 0;
	m_Lineage = LINEAGE_None;
	m_InitStand = 0;
	m_ShipBerthType = AuRosterEntry::Invalid;
	m_WeightCarried = 0;
}

DaCitizen::DaCitizen()
{
	m_Culled = 0;
	m_Lineage = LINEAGE_None;
	m_InitStand = 0;
	m_ShipBerthType = AuRosterEntry::Invalid;
	m_WeightCarried = 0;
}

DaCitizen::~DaCitizen()
{
}

DaCitizen::DaCitizen(const DaCitizen &rhs) : DaSentient(rhs)
{
	m_Culled = rhs.m_Culled;
	m_Lineage = rhs.m_Lineage;
	m_InitStand = rhs.m_InitStand;
	m_Ship = rhs.m_Ship;
	m_ShipBerthType = rhs.m_ShipBerthType;

	rhs.m_GearLock.lockForRead();
	m_Gear = rhs.m_Gear;
	m_tmGearChange = rhs.m_tmGearChange;
	m_WeightCarried = rhs.m_WeightCarried;
	rhs.m_GearLock.unlock();
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaCitizen::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaSentient::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	// Decide what data this packet will have.
	quint8 hasWhat = m_ShipBerthType << 4;	// Top nibble
	if(lastTx.IsNull())
	{
		// First time to client so send everything.
		hasWhat |= 0x07;
	}
	else
	{
		m_GearLock.lockForRead();
		if(!m_tmGearChange.IsNull() && m_tmGearChange >= lastTx)
			hasWhat |= 0x01;	// tx inventory
		m_GearLock.unlock();

		if(!m_Ship.IsNull())
			hasWhat |= 0x02;
	}
	toClient << hasWhat;

	// Send the data that hasWhat says we're sending.
	if(hasWhat & 0x01)
	{
		m_GearLock.lockForRead();
		toClient << m_Gear;
		m_GearLock.unlock();
	}

	if(hasWhat & 0x02)
		m_Ship >> toClient;
}

// Receive updates to this atom from a server.
void DaCitizen::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaSentient::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;
	fromServer >> hasWhat;

	m_ShipBerthType = (AuRosterEntry::TYPE)(hasWhat >> 4);	// Top nibble

	if(hasWhat & 0x01)
	{
		fromServer >> m_Gear;
		m_WeightCarried = GearWeight();
	}

	if(hasWhat & 0x02)
		m_Ship << fromServer;
	else
		m_Ship = AuId();
}

// Refresh data from valid counterpart.
bool DaCitizen::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	AuPoint loc(Location());
	DaSentient::Refresh(pAtom, ari);

	const DaCitizen* pA = (const DaCitizen*)pAtom;
	m_Ship = pA->m_Ship;
	m_ShipBerthType = pA->m_ShipBerthType;

	pA->m_GearLock.lockForRead();
	m_GearLock.lockForWrite();
	if(ari)
		m_Gear.Refresh(pA->m_Gear, ari->re, ari->al);
	else
		m_Gear.Refresh(pA->m_Gear, 0, 0);
	m_WeightCarried = pA->m_WeightCarried;
	m_GearLock.unlock();
	pA->m_GearLock.unlock();

	// Watch for our location in the terrain to change and cue animate to query the environment.
	// Citizens get shuffled around so animate watches the culled flag as its cue to query the environment.
	if(Location() != loc && dynamic_cast<DaWorldLL*>(Parent()))
		m_Culled = 2;
	return false;
}

void DaCitizen::Create()
{
	DaSentient::Create();
	ChooseHairStyle();
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaCitizen::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Do not render distant creatures.
	if(pass != PASS_Near)
		return;

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, rmod.Matrix(Id()), ari.al, ari.fade);

	if(m_Culled != 1 || ari.IsViewTopDown())
		DaSentient::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaCitizen::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaSentient::CreateDeviceObjects(ari);

	// My dominant hand holds my one and only current item.
	AuGearPos gearPos(CurrentItemPos());
	const AuGear* gear = LockGearForRead();
	if(!gear->isEmpty())
		SetItemInHand(*ari.re, ari.al, gear->ItemAt(gearPos));
	else
		SetItemInHand(*ari.re, ari.al, 0);
	UnlockGear();
}

void DaCitizen::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaSentient::DeleteDeviceObjects(ari);

	// Current item.
	AuGear* gear = LockGearForWrite();
	if(!gear->isEmpty())
		(*gear)[0].Item().DeleteRenderObjects(*ari.re, ari.al);
	UnlockGear();
}

unsigned int DaCitizen::IsTocker() const
{
	unsigned int ms = DaSentient::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	if(ms > d_MSecPerSec)
	{
		if(IsBodyDyingOrDead() && !IsRider())
			ms = d_MSecPerSec;
		else if(IsFlying() && IsMounted() && IsTouchingGround())
			ms = d_MSecPerSec;
	}
	if(ms > 10*d_MSecPerSec)
	{
		if(IsInSpacecraft() && CapsuleRoomType() == ROOM_Fuel)
			ms = 10*d_MSecPerSec;
		else if(IsInAnyScene())
			ms = 10*d_MSecPerSec;
	}
	return ms;
}

void DaCitizen::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	//qDebug("DaCitizen::Tock");
	DaSentient::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;	// This atom (not body) is dead or awaiting transfer.

	// Test location to see if citizen can be removed from the universe.
	if(IsInSpacecraft())
	{
		// Kill citizens that somehow get into fuel cells.
		if(CapsuleRoomType() == ROOM_Fuel)
		{
			ReleaseBerth();
			Kill();
			return;
		}
	}

	if(IsInAnyScene())
	{
		// Conditions are ticked so they can periodically affect this unit.
		TickConditions();

		// Server needs to know the current gun model CID for effect placement.
		AuGearPos gearPos(CurrentItemPos());
		m_GearLock.lockForRead();
		SetItemInHand(m_Gear.ItemAt(gearPos));
		m_GearLock.unlock();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Process dying state.
	// Body does not bleed when carried by another creature.
	if(IsBodyDyingOrDead() && !IsRider() && last.ElapsedSecTicks(now) > 0)
	{
		// Bleed out one point of health.
		AuCombatWeapon weapon(WM_Internal);
		weapon.m_CID = CID_InternalInjuries;
		weapon.m_w1.m_Effect = WE_Bleeding;

		AuCombatResult result;
		result.m_Damage1 = 1;

		CombatApplyDamageLater(AuCombatAttacker(), weapon, AuPoint(0,0), AuPoint(0,0), result);
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Stop flying pose on the ground.
	if(IsFlying() && IsMounted() && IsTouchingGround())
	{
		DoPose(AuPose::Stand);
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

void DaCitizen::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	// Get armor afforded by body.
	DaSentient::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// Armor is not tallied from inventory when in a surgery unit.
	if(!IsPatientStationPose())
	{
		// Calculate armor from items that are in valid "use" locations.
		float invAbsorb, invBlock;
		m_GearLock.lockForRead();
		m_Gear.CombatArmorEffectiveness(mode, effect, invAbsorb, invBlock);
		m_GearLock.unlock();

		// Add together the body's armor with any armor that is worn properly.
		auxAbsorb += invAbsorb;
		auxBlock += invBlock;
	}
}

void DaCitizen::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Do not accept additional damage if my body is already dead.
	if(IsBodyDead() || !IsNormal())
		return;

	// Armor takes damage before my body.
	// Wear down equipment.
	// Some weapon types are not blocked by armor.
	// Critical hits apply to body only, bypassing the armor.
	int bodyDamage1 = result.m_Damage1;
	int bodyDamage2 = result.m_Damage2;
	if(weapon.m_Mode != WM_Internal && weapon.m_Mode != WM_Psionic &&
			result.m_CriticalHits != 2 &&						// 2 crits means both effects were critical so no armor helps
			!IsPatientStationPose() &&
			weapon.m_CID != CID_MedicalProcedure &&
			weapon.m_CID != CID_GivingBirth &&
			weapon.m_CID != CID_ImplantBirth &&
			weapon.m_CID != CID_Poison &&
			weapon.m_CID != CID_Disease &&
			weapon.m_CID != CID_Eaten &&
			weapon.m_CID != CID_ShipCapture &&
			weapon.m_CID != CID_Wormhole &&
			weapon.m_CID != CID_Starvation &&
			weapon.m_CID != CID_InternalInjuries)
	{
		if(bodyDamage1 && !result.m_CriticalHits)
		{
			// Absorb as much of the damage with my armor as possible before allowing it to hurt my body.
			AuGear* gear = LockGearForWrite();
			int armorHits = gear->CombatArmorHits(weapon.m_Mode, weapon.m_w1.m_Effect);
			//qDebug() << Id() << Name() << "bodyDamage1" << bodyDamage1 << "armorHits" << armorHits;
			if(armorHits >= bodyDamage1)
			{
				// My armor absorbs all of the damage.
				if(gear->CombatApplyDamage(weapon.m_Mode, weapon.m_w1.m_Effect, bodyDamage1))
					UpdateGearWeight();
				bodyDamage1 = 0;
			}
			else
			{
				// My armor absorbs some of the damage.
				if(gear->CombatApplyDamage(weapon.m_Mode, weapon.m_w1.m_Effect, armorHits))
					UpdateGearWeight();
				bodyDamage1 -= armorHits;
			}
			UnlockGear();
		}
		if(bodyDamage2)
		{
			// Absorb as much of the damage with my armor as possible before allowing it to hurt my body.
			AuGear* gear = LockGearForWrite();
			int armorHits = gear->CombatArmorHits(weapon.m_Mode, weapon.m_w2.m_Effect);
			//qDebug() << Id() << Name() << "bodyDamage2" << bodyDamage2 << "armorHits" << armorHits;
			if(armorHits >= bodyDamage2)
			{
				// My armor absorbs all of the damage.
				if(gear->CombatApplyDamage(weapon.m_Mode, weapon.m_w2.m_Effect, bodyDamage2))
					UpdateGearWeight();
				bodyDamage2 = 0;
			}
			else
			{
				// My armor absorbs some of the damage.
				if(gear->CombatApplyDamage(weapon.m_Mode, weapon.m_w2.m_Effect, armorHits))
					UpdateGearWeight();
				bodyDamage2 -= armorHits;
			}
			UnlockGear();
		}
	}

	DaSentient::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	if(!attacker.m_Id.IsNull() && weapon.m_Mode != WM_Internal)
	{
		// I was hit by somebody.
		DaWorld* planet = 0;
		DaVessel* ship = 0;
		DaBd* bd = 0;
		if(IsInSpacecraft())
		{
			ship = FindParentVesselInEnv();
		}
		else if(!InBdCity().IsNull() && !attacker.m_Id.IsNull())
		{
			// I am in a bd city. Notify the city there has been an attack in town. This prevents workers from spawning
			// which helps to eliminate a genocide exploit. Players would hover over a city and systematically slaughter all the
			// citizens until there was no population. Then the loyalty is not a factor. That will still be possible in a small
			// town, if they wait until all of the populace spawns before attacking.
			// This also prevents an exploit where a troop at a console is killed, replacement ports to console, killed, repeat...
			planet = FindParentPlanetInEnv();
			if(planet)
			{
				bd = planet->FindBd(BdId());
				if(bd)
				{
					DaAtom* solar = planet->FindEnvironmentAtom();
					if(solar)
					{
						DaAtom* atom = solar->FindIdRecursive(attacker.m_Id);
						if(atom && atom->EmpireId())
						{
							POLITICAL_STANCE stance = PoliticalStance(EmpireId(), atom->EmpireId());
							if(stance == POL_Enemy || stance == POL_Neutral)
							{
								QString empireName;
								const DgData& gd = g_Au.LockDataForRead();
								gd.EmpireName(atom->EmpireId(), empireName);
								g_Au.UnlockData();
								QString attName = tr("%1 of %2").arg(atom->Name()).arg(empireName);
								bd->LogEvent(AuEventLogEntry::CitizenAttacked, attName);
							}
						}
					}
				}
			}
		}

		if(CurrentBodyHitPoints() > 0)
		{
			// I am still alive.
			if(!IsAlert())
			{
				QString text = tr("I am under attack by %1.").arg(attacker.m_Name);
				BecomeAlert(text);		// Become alert and alert nearby troops.
			}
			if(Posture() != POSTURE_HoldFire && attacker.m_Id != OrderTarget())
			{
				// I was hit by something that is not the target of my current order.
				switch(Order())
				{
				case ORDER_Sleep:
				case ORDER_SleepDayDream:
				case ORDER_SleepEat:
				case ORDER_SleepExercise:
				case ORDER_SleepGame:
				case ORDER_SleepLetter:
				case ORDER_SleepMovie:
				case ORDER_SleepMusic:
				case ORDER_SleepPine:
				case ORDER_SleepPhone:
				case ORDER_SleepRead:
				case ORDER_SleepTalk:
				case ORDER_SleepText:
				case ORDER_SleepTV:
				case ORDER_SleepWeb:
				case ORDER_DamageReport:
				case ORDER_SurveyScanPlanet:
				case ORDER_SurveyScanSystem:
				case ORDER_TacticalScan:
				case ORDER_Stay:
				case ORDER_FollowMe:
				case ORDER_FollowTarget:
				case ORDER_GoToRoom:
				case ORDER_GoToTarget:
				case ORDER_GoToTargetExactly:
				case ORDER_HengeCeremony:
					// Attack the atom that attacked me.
					TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
					break;

				case ORDER_BattleStation:
					// On a spacecraft I get up from my station. In city I stay at my station.
					if(IsHelmStationPose())
					{
						// Attack the atom that attacked me if the ship is not moving.
						if(!ship || !ship->IsMoving())
							TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
					}
					else if(IsMedicStationPose())
					{
						// Attack the atom that attacked me if I don't have a patient.
						if(ship)
						{
							if(!ship->FindPatient(*this))
								TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
						}
						else if(IsInsideBd())
						{
							if(!bd)
								bd = FindParentBdInEnv();
							if(!bd || !bd->FindPatient(*this))
								TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
						}
					}
					else if(IsInSpacecraft() || !IsTroopStationPose())
					{
						// Attack the atom that attacked me.
						TakeOrderAmendment(ORDER_AttackTarget, attacker.m_Id, AuPoint(0,0));
					}
					break;

				case ORDER_AttackTarget:
				case ORDER_ComeHere:
				case ORDER_ComeHereExactly:
				case ORDER_GetIn:
				case ORDER_GetOut:
				case ORDER_Garrison:
				case ORDER_CloseDoor:
				case ORDER_OpenDoor:
				case ORDER_GetItem:
				case ORDER_EnterSurgery:
				case ORDER_TakeCommand:
				case ORDER_ReleaseCommand:
				case ORDER_DropRider:
				case ORDER_MountMe:
				case ORDER_MountTarget:
					// Don't get distracted from my current order.
					break;
				}
			}
		}
	}

	// When body has died, drop meat and maybe an item.
	if(result.m_IsDestroyed)
		BodyHasDied(weapon.m_CID, weapon.m_Quality, weapon.m_w1.m_Effect, weapon.m_w2.m_Effect, attacker.m_Name);
	else if(weapon.m_CID != CID_MedicalProcedure)
		GetACheckup();										// Consider finding a sick bay for medical attention.
}

void DaCitizen::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaSentient::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
	if(!lastAnim.isValid())
		return;

	// Citizens are about the size of a human. This enables us to guess at a good distance to cull them.
	m_Culled = 0;

	// Only animate if my current pose is not 'stand', which is inanimate.
	if(CurrentGeneralPose().Pose() != AuPose::Stand)
	{
		AnimatePose(HasItemInHand(), MovementSpeedMod());
	}
	else if(m_InitStand < 2 && CurrentGeneralPose().Pose() == AuPose::Stand)
	{
		// Initial body positioning for standing pose.
		// This pose is currently inanimate; it saves a lot of frames per second to stop animating all the citizens standing around
		// my ship.
		AnimatePose(HasItemInHand(), 1);
		m_InitStand++;	//DHH I'm not sure yet why animate must get called twice for the body to be posed properly.
						//DHH This may have been fixed. Try it.
	}
}

void DaCitizen::SensorScan(AuSensorBlipList& blips, const AuPoint& /*scannerLoc*/, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int scanContent, const AuId& /*lockedBlip*/,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	// I am only scanned for AI turret gunners and transporter consoles.
	if(!(scanContent & 0x0024))
		return;

	// Only my own ship, that I hold a berth aboard, can detect me on transporter sensors.
	if(AssignedShip().IsNull() || AssignedShip() != scannerId)
		return;

	// Dying creatures do not show up on sensors.
	if(IsBodyDyingOrDead())
		return;

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);
	blips.Add(AuBlipCitizen(Name(), Species(), EmpireId(), stance, Id(), EnvLocation()));
}

AuPoint DaCitizen::Acceleration() const
{
	if(IsInGravity())
	{
		// I am inside the powerful immediate gravitation force of a planet's atmosphere or a spacecraft or a vehicle.
		// This creature does not get to control acceleration when not touching the ground, unless it is flying, swimming,
		// in space, or positioned for using a gravity well, hatch, or turret. Did I miss anything?
		if(!CanControlMovement())
			return DaSentient::Acceleration();				// Gravity force alone.
	}
	else
	{
		// I am outside any powerful immediate gravitational force. I can still be drawn to the nearest gravity well.
		// In space I can control my movement if I am wearing a space suit or if I am aboard a spacecraft.
		if(IsBodyDyingOrDead())
			return DaSentient::Acceleration();				// Gravity force alone.
		else if(!IsInSpacecraft() && !IsInAtmosphere())
		{
			if(InBlackHoleGravity())
				return DaSentient::Acceleration();			// Gravity force alone.

			// EVA suit holds position in most outer space environments, except near a black hole.
			const AuGear* gear = LockGearForRead();
			const AuItem* item = gear->ArmorItemWorn();
			bool g = !item || !item->IsEnvironmentSuit();
			UnlockGear();
			if(g)
				return DaSentient::Acceleration();			// Gravity force alone.
		}
	}

	// Movement speed determined by my current general body pose.
	double speed = MovementSpeed(1);	// in feet per second
	double force;
	if(speed == 0)
		force = 0;
	else
	{
		// Speed is non-zero.
		if(IsOnGround() || IsLadderPose() || (InLadderWell() && IsInGravity()))
			force = (speed + GravityStandard()) * 2;	// Need extra force to overcome ground friction and gravity.
		else
			force = speed;
	}

	// Movement actions.
	AuPoint fmove;		// Forward/backward movement.
	if(speed != 0)
		fmove = AuPoint(0,1,0);	// This animal is in a moving pose.

	// Cumulative movement vector.
	// Tilt movement vector according to head tilt.
	AuPoint lookMove = fmove.RotateX(CurrentHeadTilt());
	if(!lookMove.IsZero())
		lookMove.Unit(force);

	// I can control my movements so limit forward movement distance to stop better at hatches and things.
	AuPoint v(Vector());
	float mdl = MoveDistLimit();
	if(mdl >= 0)
	{
		AuPoint friction = Friction();
		if(v.y + lookMove.y - friction.y > mdl)
			lookMove.y = mdl - v.y + friction.y;
	}

	// Now that we know the final desired movement vector, check to see if the
	// citizen is too fast and accelerating. This citizen can't go faster if
	// they're already beyond their maximum speed; they can decelerate however.
	// This logic only applies when inside atmosphere range of a planet or
	// inside a spacecraft.
	if(IsInGravity())
	{
		if(fabs(v.x) > speed/2)
		{
			// Currently moving too fast in X; only permit deceleration.
			if(fabs(v.x + lookMove.x) > fabs(v.x))
				lookMove.x = 0;
		}
		if(fabs(v.y) > speed)
		{
			// Currently moving too fast in Y; only permit deceleration.
			if(fabs(v.y + lookMove.y) > fabs(v.y))
				lookMove.y = 0;
		}
		if(v.z > speed/2 || v.z < -speed)
		{
			// Currently moving too fast in Z; only permit deceleration.
			if(fabs(v.z + lookMove.z) > fabs(v.z))
				lookMove.z = 0;
		}

		// Adjust up vector based on air density.
		if(IsFlying() && lookMove.z > 0)
		{
			float fd = AirDensity() - .15f;	// According to DaCreature::CanFly, atmo density .15 is needed to fly.
			if(fd < 0)
				fd = 0;
			lookMove.z *= fd * 2;	// Earth standard density is .7.
		}

		// Add force of gravity, if any.
		// Weaken gravity while touching a road so going up and down steep road hills is easy.
		//if(IsTouchingGround() && InIndustry() == IID_Road)
			//lookMove += DaSentient::Acceleration()/2;
		//else
			lookMove += DaSentient::Acceleration();
	}
	else
	{
		// I am weightless. Try to control my speed.
		if(mdl > 0 && IsMovingPose())
		{
			// I must be hoping to get somewhere EVA. Good luck!
			// Try to keep my speed down to where I can stop at the destination.
			const float brakeForce = 15;						// Force of braking in fps along each XYZ axis.
			float secsToStopX = fabs(v.x) / brakeForce;	// Seconds to stop current X vector with brakes on.
			float secsToStopY = fabs(v.y) / brakeForce;	// Seconds to stop current Y vector with brakes on.
			float secsToStopZ = fabs(v.z) / brakeForce;	// Seconds to stop current Z vector with brakes on.
			float secsToGetThereX = 0;
			float secsToGetThereY = 0;
			float secsToGetThereZ = 0;
			if(v.x)
			{
				secsToGetThereX = mdl / fabs(v.x);	// Seconds to arrive in X at current speed.
				if(secsToStopX >= secsToGetThereX)
					lookMove.x = 0;								// Going too fast in X. Do not add to velocity in X.
			}
			if(v.y)
			{
				secsToGetThereY = mdl / fabs(v.y);	// Seconds to arrive in Y at current speed.
				if(secsToStopY >= secsToGetThereY)
					lookMove.y = 0;								// Going too fast in Y. Do not add to velocity in Y.
			}
			if(v.z)
			{
				secsToGetThereZ = mdl / fabs(v.z);	// Seconds to arrive in Z at current speed.
				if(secsToStopZ >= secsToGetThereZ)
					lookMove.z = 0;								// Going too fast in Z. Do not add to velocity in Z.
			}

			//qDebug() << "DaCitizen::Acceleration" << Id() << Name() << "dist" << mdl << "lookMove" << lookMove
				//<< "secsToStop" << secsToStopX << secsToStopY << secsToStopZ
				//<< "secsToGetThere" << secsToGetThereX << secsToGetThereY << secsToGetThereZ;
		}
		else
			lookMove = AuPoint(0,0);							// I don't want to move.
	}

	//if(Is(s_PathTrace))
		//qDebug("SN accel force %g speed %g lookMove %g,%g,%g friction %g,%g,%g inGrav %c moveLimit %g pose %d", force, speed,
				//lookMove.x, lookMove.y, lookMove.z, Friction().x, Friction().y, Friction().z,
				//IsInGravity()?'Y':'N', mdl, CurrentGeneralPose().Pose());

	return lookMove;
}

AuPoint DaCitizen::Friction() const
{
	// When mounted, friction is determined by the mount.
	if(IsRider())
		return AuPoint(0,0);

	// Determine if brake is on.
	// Check for cases in which this person cannot control their braking.
	if(IsInGravity())
	{
		// I am inside the powerful immediate gravitation force of a planet's atmosphere or a spacecraft or a vehicle.
		// This creature does not get to control acceleration when not touching the ground, unless it is flying, swimming,
		// in space, or positioned for using a gravity well, hatch, or turret. Did I miss anything?
		if(!CanControlMovement())
			return DaSentient::Friction();						// Friction from environment alone.
	}
	else
	{
		// I am outside any powerful immediate gravitational force. I can still be drawn to the nearest gravity well.
		// In space I can control my movement if I am wearing a space suit or if I am aboard a spacecraft.
		if(IsBodyDyingOrDead())
			return DaSentient::Friction();						// Friction from environment alone.
		else if(!IsInSpacecraft() && !IsInAtmosphere())
		{
			const AuGear* gear = LockGearForRead();
			const AuItem* item = gear->ArmorItemWorn();
			bool f = !item || !item->IsEnvironmentSuit();
			UnlockGear();
			if(f)
				return DaSentient::Friction();					// Friction from environment alone.
		}

		// I am weightless. Try to control my speed.
		bool applyBrake = false;
		float mdl = MoveDistLimit();
		if(mdl > 0 && IsMovingPose())
		{
			// I must be hoping to get somewhere EVA. Good luck!
			// Try to keep my speed down to where I can stop at the destination.
			AuPoint vec(Vector());
			const float brakeForce = 15;						// Force of braking in fps along each XYZ axis.
			float secsToStopX = fabs(vec.x) / brakeForce;	// Seconds to stop current X vector with brakes on.
			float secsToStopY = fabs(vec.y) / brakeForce;	// Seconds to stop current Y vector with brakes on.
			float secsToStopZ = fabs(vec.z) / brakeForce;	// Seconds to stop current Z vector with brakes on.
			float secsToGetThereX = 0;
			float secsToGetThereY = 0;
			float secsToGetThereZ = 0;
			if(vec.x)
			{
				secsToGetThereX = mdl / fabs(vec.x);	// Seconds to arrive in X at current speed.
				if(secsToStopX >= secsToGetThereX)
					applyBrake = true;							// Going too fast in X. Apply brakes in X.
			}
			if(vec.y)
			{
				secsToGetThereY = mdl / fabs(vec.y);	// Seconds to arrive in Y at current speed.
				if(secsToStopY >= secsToGetThereY)
					applyBrake = true;							// Going too fast in Y. Apply brakes in Y.
			}
			if(vec.z)
			{
				secsToGetThereZ = mdl / fabs(vec.z);	// Seconds to arrive in Z at current speed.
				if(secsToStopZ >= secsToGetThereZ)
					applyBrake = true;							// Going too fast in Z. Apply brakes in Z.
			}

			//qDebug() << "DaSentient::Friction" << Id() << Name() << "dist" << mdl << "applyBrake" << applyBrake
				//<< "secsToStop" << secsToStopX << secsToStopY << secsToStopZ
				//<< "secsToGetThere" << secsToGetThereX << secsToGetThereY << secsToGetThereZ;
		}
		else
			applyBrake = true;									// I don't want to move.

		// Apply normal braking if I want.
		if(applyBrake)
			return DaSentient::Friction() + AuPoint(15,15,15);
	}

	return DaSentient::Friction();								// Friction from environment alone.
}

QString DaCitizen::Name() const
{
	QString s(AssignedName());
	if(s.isEmpty())
		return tr("Citizen");
	else
		return s;
}

QString DaCitizen::SpotDescription(quint32 spotterEmpire) const
{
	QString name = tr("%1, %2 %3")
		.arg(Name())
		.arg(GenderName(Gender()))
		.arg(SpeciesName());

	int curHits = CurrentHitPoints();
	int maxHits = MaxHitPoints();
	if(curHits == maxHits)
		name += tr(" %1 hp").arg(maxHits);
	else
		name += tr(" %1/%2 hp").arg(curHits).arg(maxHits);

	if(EmpireId())
	{
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(EmpireId(), empire);
		POLITICAL_STANCE stance = gd.EmpirePoliticalStance(spotterEmpire, EmpireId());
		g_Au.UnlockData();
		switch(stance)
		{
		case POL_Friend:
			return tr("Friendly %1 %2").arg(empire).arg(name);
		case POL_Vassal:
			return tr("Overlord %1 %2").arg(empire).arg(name);
		case POL_Neutral:
			return tr("Neutral %1 %2").arg(empire).arg(name);
		case POL_Enemy:
			return tr("Enemy %1 %2").arg(empire).arg(name);
		case POL_Empire:
			return tr("%1 %2").arg(empire).arg(name);
		}
	}
	return name;
}

// Sets effect1 and effect2 to WE_None if the equipped weapon needs ammo but is out of ammo.
// When an object is used, 'loc' is set to WL_None and 'cid' is set to the
// commodity id of the item.
// When a physical body location is used, 'loc' is set to the weapon location and 'cid' is set to CID_Invalid.
// Returns true if the body special weapon is used.
bool DaCitizen::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
		int& ammoFull) const
{
	// Get info about physical body weapon; there is always a physical body weapon even if its my teeth.
	// This provides the base range provided by the physical body.
	// They physical body range is added to the weapon range.
	bool useSpecWpn = DaSentient::BestWeaponAtRange(targetRange, loc, weapon, ammoCur, ammoFull);

	// Check inventory item in use for weapon capability.
	AuGearPos gearPos(CurrentItemPos());
	const AuGear* gear = LockGearForRead();
	const AuItem* gearItem = gear->ItemAt(gearPos);
	if(!gearItem)
	{
		UnlockGear();
		return useSpecWpn;
	}

	INVENTORYSUB useSub;
	if(gearItem->IsWeapon())
		useSub = IPS_Weapon1;
	else
		useSub = IPS_Item;
	if(gearItem->IsWeapon(useSub))
	{
		// The citizen has a weapon in use.
		gearItem->WeaponInfo(useSub, weapon.m_CID, weapon.m_Quality, weapon.m_Mode, weapon.m_Range, ammoCur, ammoFull,
				weapon.m_w1.m_Effect, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod, weapon.m_w1.m_Radius,
				weapon.m_w1.m_AP, weapon.m_w1.m_ST,
				weapon.m_w2.m_Effect, weapon.m_w2.m_Damage, weapon.m_w2.m_DamageMod, weapon.m_w2.m_Radius,
				weapon.m_w2.m_AP, weapon.m_w2.m_ST);
		loc = DhDNA::WL_None;									// Body weapon was not employed.
		if(gearItem->CanFireWeapon(useSub))
		{
			// When using contact weapons and projectile weapons, I can add a little range to what I can physically reach.
			if(weapon.m_Mode == WM_Contact || weapon.m_Mode == WM_Projectile)
			{
				// Add a little reach range to the range of the weapon. This is measured from my eye.
				weapon.m_Range += 6;

				// When using contact weapons, add a damage modifier due to my strength.
				if(weapon.m_Mode == WM_Contact)
				{
					// Melee weapons are affected by strength and strength increases damage.
					weapon.m_w1.m_DamageMod += StrengthMod();						// Strength mod increases base damage.
				}
			}
		}
		else
		{
			// The weapon can't be fired, most likely out of ammo.
			weapon.m_w1.m_Effect = WE_None;
			weapon.m_w2.m_Effect = WE_None;
		}
	}
	else
	{
		weapon.m_Mode = WM_Not;
		weapon.m_w1.m_Effect = WE_None;
		weapon.m_w2.m_Effect = WE_None;
	}
	UnlockGear();
	return false;												// Did not use body special weapon.
}

void DaCitizen::WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const
{
	if(pos.IsBody())
	{
		// Body weapons are resolved by the base class.
		DaSentient::WeaponAtUsePos(pos, sub, weapon, ammoCur, ammoFull);
	}
	else if(pos.IsGear())
	{
		const AuGear* gear = LockGearForRead();
		const AuItem* gearItem = gear->ItemAt(pos);
		if(gearItem)
		{
			// Get weapon info about an item in my inventory.
			gearItem->WeaponInfo(sub, weapon.m_CID, weapon.m_Quality, weapon.m_Mode, weapon.m_Range, ammoCur, ammoFull,
					weapon.m_w1.m_Effect, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod, weapon.m_w1.m_Radius,
					weapon.m_w1.m_AP, weapon.m_w1.m_ST,
					weapon.m_w2.m_Effect, weapon.m_w2.m_Damage, weapon.m_w2.m_DamageMod, weapon.m_w2.m_Radius,
					weapon.m_w2.m_AP, weapon.m_w2.m_ST);

			if(gearItem->IsWeapon(sub))
			{
				// The person has a weapon in use.
				if(gearItem->CanFireWeapon(sub))
				{
					// When using contact weapons and projectile weapons, I can add a little range to what I can physically reach.
					if(weapon.m_Mode == WM_Contact || weapon.m_Mode == WM_Projectile)
					{
						// Add a little reach range to the range of the weapon. This is measured from my eye.
						weapon.m_Range += 6;

						// When using contact weapons, add a damage modifier due to my strength.
						if(weapon.m_Mode == WM_Contact)
						{
							// Melee weapons are affected by strength and strength increases damage.
							weapon.m_w1.m_DamageMod += StrengthMod();					// Strength mod increases base damage.
						}
					}
				}
				else
				{
					// The weapon can't be fired, most likely out of ammo.
					weapon.m_w1.m_Effect = WE_None;
					weapon.m_w2.m_Effect = WE_None;
				}
			}
		}
		else
		{
			// Not a body part and not a valid item in my inventory.
			// Set null values.
			weapon = AuCombatWeapon(WM_Not);
			ammoCur = 0;
			ammoFull = 0;
		}
		UnlockGear();
	}
}

bool DaCitizen::AddGearItemUseDrop(const AuItem& cItem, int useIt, bool dropIt)
{
	m_GearLock.lockForWrite();
	bool r = AddGearItemUseDropNoLock(cItem, useIt, dropIt);
	m_GearLock.unlock();
	return r;
}

// 'useIt' is 0=no, 1=yes, 2=only put in dominant hand if it's a weapon
bool DaCitizen::AddGearItemUseDropNoLock(const AuItem& cItem, int useIt, bool dropIt)
{
	// Select an appropriate location to carry the item.
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::Invalid;
	if(useIt)
	{
		// Place weapon in my dominant hand and wear clothing/armor appropriately.
		if(cItem.Content())
		{
			slotType = AuGearSlot::Body;
		}
		else if(cItem.IsWornOnHead())
		{
			slotType = AuGearSlot::Head;
		}
		else if(cItem.IsWornOnFace())
		{
			slotType = AuGearSlot::Face;
		}
		else if(cItem.IsWornOnNeck())
		{
			slotType = AuGearSlot::Neck;
		}
		else if(cItem.IsWornOnBody())
		{
			slotType = AuGearSlot::BodyWear;
		}
		else if(cItem.IsWornOnLimb())
		{
			slotType = AuGearSlot::Limb;
		}
		else if(cItem.IsWornOnDigit())
		{
			slotType = AuGearSlot::Digit;
		}
		else if(useIt == 1 || cItem.IsWeapon())
		{
			slotType = AuGearSlot::Hand;
		}

		// Insure I am permitted to use that item in that position.
		if(slotType != AuGearSlot::Invalid && !CanUseGear(cItem, slotType))
			slotType = AuGearSlot::Invalid;
	}

	return AddGearItemAt(m_Gear, slotType, cItem, dropIt);
}

int DaCitizen::UseItem(AuItem& item, INVENTORYSUB sub)
{
	// Do not flag ammo usage as a gear change otherwise the client gets my gear every time I shoot a bullet.
	// The client never knows how many shots I have in my gun. They only care when I reload, which takes a clip out of my gear.
	int useResult = DaSentient::UseItem(item, sub);
	if(useResult == 2)
	{
		// Condition degraded so client needs to know.
		m_tmGearChange.Set();
		SetModified();
	}
	return useResult;
}

void DaCitizen::InitGear(AuItemHash& cinv, AuItemHash& itemsTaken, bool equipFully)
{
	// Clothing in the order of preference, by gender.
	COMMODITYID clothingM[] =
	{
		CID_JeansAndShirt,
		CID_LeatherClothes,
	};
	int clothingTypesM = sizeof(clothingM)/sizeof(COMMODITYID);

	COMMODITYID clothingF[] =
	{
		CID_CapriHalterOutfit,
		CID_JeansAndShirt,
		CID_LeatherClothes,
	};
	int clothingTypesF = sizeof(clothingF)/sizeof(COMMODITYID);

	if(equipFully)
	{
		// Choose a random item from the city's inventory that any citizen might have.
		// Choose an item that can be carried. Do not choose clothing, weapons, ammunition or armor.
		COMMODITYID items[] =
		{
			//CID_Invalid = 0,
			CID_Flag,
			CID_Computer,
			//CID_Electricity,
			CID_Hammer,
			CID_Air,				// breathable compressed air
			//CID_Helicopter,
			//CID_SpaceRocket,
			//CID_SpaceFighter,
			//CID_SpaceTransport,
			//CID_PistolAmmo,
			//CID_HeavyArmsAmmo,
			//CID_PlasticExplosive,
			//CID_Animal = 14,
			CID_Beans,
			CID_Beer,
			//CID_LeatherBodyArmor,
			CID_Bread,
			CID_Candy,
			CID_Cheese,
			CID_Coal,
			CID_CrudeOil,
			CID_Crystals,
			//CID_Dynamite,
			CID_Eggs,
			CID_ElectronicPart,
			//CID_FarmMachinery,
			CID_Fertilizer,
			CID_Fiber,
			//CID_Pistol = 31,
			//CID_Rifle,
			CID_Fish,
			CID_Flour,
			CID_Fruit,
			CID_Gasoline,
			CID_Gems,
			CID_Grain,
			CID_Grapes,
			CID_GunPowder,
			//CID_WovenHelmet,
			CID_Herbs,
			CID_Hops,
			CID_Hydrogen,
			CID_Ice,
			CID_CrystalJewelry,
			CID_Leather,
			CID_Lumber,
			CID_AnimalMeat,
			CID_MechanicalPart,
			CID_Metal,
			CID_IceCream,
			CID_Milk,
			CID_Minerals,
			//CID_MineMachinery,
			CID_NaturalGas,
			CID_Nuts,
			CID_Ore,
			CID_Pastry,
			CID_Petrochemicals,
			CID_Pharmaceuticals,
			CID_Plastic,
			CID_Radioactives,
			//CID_CommercialShip,
			//CID_NavalShip,
			//CID_VacSuitHelmet,
			CID_MedicalKit,
			CID_Spices,
			//CID_RifleAmmo,
			CID_Stone,
			//CID_Submarine,
			CID_Sugar,
			CID_Textiles,
			//CID_VacSuit,
			CID_Vegetable,
			//CID_SUV,
			CID_Water,
			CID_Whiskey,
			CID_Wine,
			CID_Wood,
			//CID_Atmosphere,
			//CID_Hydrographics,
			//CID_Vegetation,
			//CID_PlasticBodyArmor,
			//CID_WovenBodyArmor,
			//CID_NitrateExplosive,
			//CID_Sunlight,
			//CID_LeatherClothes,
			//CID_LeatherHelmet,
			//CID_PlasticHelmet,
			CID_GoldBracelet,
			CID_GoldRing,
			CID_GemRing,
			CID_FishMeat,
			CID_Vodka,
			//CID_HeavyWeapon,
			//CID_Tank,
			//CID_APC,
			CID_Crate,
			CID_Beacon,
			CID_Lantern,
			CID_Knapsack,
			CID_Backpack,
			CID_Lumenite,
			CID_BlankDisk,
			CID_Shovel,
			CID_Gin,
			//CID_Knife,
			//CID_Shotgun,
			//CID_ShotgunAmmo,
			CID_Antidote,
			CID_Antitoxin,
			CID_Rum,
			CID_Tequila,
			CID_FittingWrench,
			CID_GlueGun,
			CID_ScrewDriver,
			CID_SewingNeedle,
			CID_SolderIron,
			CID_Tongs,
			CID_Wrench,
			CID_Hay,
			CID_Candle,
			CID_Torch,
			//CID_SailBoat,
			//CID_DirtBike,
			CID_Eludium,
			CID_Pliers,
			CID_TorqueWrench,
			CID_Vulcanite,
			CID_Ioplasma,
			CID_Magmex,
			CID_Phlogiston,
			CID_Bolite,
			CID_Cryozine,
			CID_Myrathane,
			CID_Polytaride,
			CID_Vulcium,
			CID_Magmium,
			//CID_Magma,
			//CID_SupercooledLiquid,
			//CID_InfernoAtmosphere,
			//CID_FrigidAtmosphere,
			//CID_MagmiumKnife,
			//CID_VulciumKnife,
			//CID_MagmiumPowerArmor,
			//CID_MagmiumPowerArmorHelmet,
			//CID_VulciumPowerArmor,
			//CID_VulciumPowerArmorHelmet,
			//CID_Money,
			CID_GravCoupling,
			CID_CryoHeatSink,
			CID_IoTube,
			CID_Clamp,
			CID_OScope,
			//CID_AdvSpaceFighter,
			//CID_MissionDisk,
			//CID_Bone,
			//CID_CommandAndControlModule,
			//CID_GravityDriveModule,
			//CID_RocketDriveModule,
			//CID_LifeSupportModule,
			//CID_PowerPlanetModule,
			//CID_EnergyShieldModule,
			//CID_KineticShieldModule,
			//CID_CloakingShieldModule,
			//CID_SurgeryMedicalModule,
			//CID_GeneticsMedicalModule,
			//CID_CyberneticsMedicalModule,
			//CID_PersonnelTransporterModule,
			//CID_VehicleTransporterModule,
			//CID_AtmosphereCondenserBayModule,
			//CID_ProspectorBayModule,
			//CID_TractorBeamBayModule,
			//CID_WormholeFTLDriveModule,
			//CID_WarpFTLDriveModule,
			//CID_WeaponBayModule,
			//CID_GunTurretModule,
			//CID_SensorModule,
			CID_MagmiumHeatSink,
			CID_VulciumHeatSink,
			CID_Myrasplicer,
			CID_Microscope,
			CID_Gigacell,
			CID_TesloidDissipator,
			CID_NanopathCalibrator,
			CID_AntifluxParticles,
			CID_BorexinoPrecipitate,
			//CID_TypeOPreons,
			//CID_TypeBPreons,
			//CID_TypeAPreons,
			//CID_TypeFPreons,
			//CID_TypeGPreons,
			//CID_TypeKPreons,
			//CID_TypeMPreons,
			//CID_MagmiumHullModule,
			//CID_VulciumHullModule,
			CID_Paper,
			CID_RedBinder,
			CID_GreenBinder,
			CID_BlueBinder,
			//CID_ParticleCollectorBayModule,
			CID_DNASampleKit,
			//CID_DNASample,
			CID_UtilityPouch,
			CID_GravPack,
			CID_Silage,
			//CID_Count				// LAST ITEM
		};
		int itemTypes = sizeof(items)/sizeof(COMMODITYID);

		if(Gender() == Female)
			InitGear(cinv, itemsTaken,
					0, 0, 0,										// Weapon/ammo
					0, 0,											// Armor
					0, 0,											// Helmet
					clothingF, clothingTypesF,						// Clothing
					items, itemTypes);								// Items
		else
			InitGear(cinv, itemsTaken,
					0, 0, 0,										// Weapon/ammo
					0, 0,											// Armor
					0, 0,											// Helmet
					clothingM, clothingTypesM,						// Clothing
					items, itemTypes);								// Items
	}
	else
	{
		if(Gender() == Female)
			InitGear(cinv, itemsTaken,
					0, 0, 0,										// Weapon/ammo
					0, 0,											// Armor
					0, 0,											// Helmet
					clothingF, clothingTypesF,						// Clothing
					0, 0);											// Items
		else
			InitGear(cinv, itemsTaken,
					0, 0, 0,										// Weapon/ammo
					0, 0,											// Armor
					0, 0,											// Helmet
					clothingM, clothingTypesM,						// Clothing
					0, 0);											// Items
	}
}

void DaCitizen::InitGear(AuItemHash& cinv, AuItemHash& itemsTaken,
		COMMODITYID* weapon, COMMODITYID* ammo, int weaponTypes,
		COMMODITYID* armor, int armorTypes,
		COMMODITYID* helmet, int helmetTypes,
		COMMODITYID* clothing, int clothingTypes,
		COMMODITYID* items, int itemTypes)
{
	AuGearPos gearPos(CurrentItemPos());
	m_GearLock.lockForWrite();
	if(weapon && ammo && weaponTypes)
	{
		// Choose a weapon.
		const AuItem* gearItem = m_Gear.ItemAt(gearPos);	// My current weapon or other item in hand.
		for(int ii = 0; ii < weaponTypes; ii++)
		{
			AuItemHash::const_iterator itemI = cinv.FindBest(weapon[ii]);			// Equip with the best.

			// If I already have the weapon in hand, just try to get ammo for it.
			if(gearItem && gearItem->CommodityId() == weapon[ii] &&
					(itemI == cinv.cend() || gearItem->Quality() >= (*itemI).second->Quality()))
			{
				// I already have that weapon in hand or the city does not have that weapon. Do I need ammo for it?
				if(ammo[ii] != CID_Invalid)
				{
					// The weapon uses ammo.
					// Count units of ammunition in my inventory.
					int ammoCount = 0;
					for(AuGear::const_iterator ai = m_Gear.begin(); ai != m_Gear.end(); ai++)
					{
						const AuItemAmmo* ammoItem = dynamic_cast<const AuItemAmmo*>(&(*ai).Item());
						if(!ammoItem)
							continue;	// The item is not ammo.
						if(ammoItem->CommodityId() == ammo[ii])
							ammoCount++;
					}
					if(ammoCount >= 5)
						break;		// I don't need ammo for my weapon and I don't need a nu weapon.

					// Locate ammo in the city's inventory.
					AuItemHash::const_iterator ammoI = cinv.FindBest(ammo[ii]);		// Equip with the best.
					if(ammoI == cinv.end())
					{
						if(ammoCount == 0)
							continue;	// The city doesn't have any ammo for the weapon. Continue looking for another weapon.
						else
							break;		// I don't need ammo for my weapon and I don't need a nu weapon.
					}

					// Take ammo from the city's inventory.
					AuItem* item = (*ammoI).second->Clone();
					item->SetCount(1);
					int ammoToTake = 5 - ammoCount;				// Enough to fill free hand and 4 waist positions.
					if(ammoToTake > (int)(*ammoI).second->Count())
						ammoToTake = (int)(*ammoI).second->Count();
					for(; ammoToTake; ammoToTake--)
					{
						if(!AddGearItemUseDropNoLock(*item, 0, false))	// Try to add one ammo to my inventory.
							break;	// Stop trying if no more will fit.
						itemsTaken.AddItemCount(*(*ammoI).second, 1);
						cinv.TakeItem(*(*ammoI).second, 1);	// Take one ammo from the city's inventory.
					}
					delete item;
					item = 0;
				}
				break;	// I don't need a nu weapon.
			}

			// Here if I think I need a nu weapon.
			if(itemI != cinv.end())
			{
				// The city has the weapon in stock.
				// If the weapon needs ammo, insure the city has ammo.
				if(ammo[ii] != CID_Invalid)
				{
					// The weapon needs ammo.
					AuItemHash::const_iterator ammoI = cinv.FindBest(ammo[ii]);		// Equip with the best.
					if(ammoI == cinv.end())
						continue;	// The city doesn't have any ammo for the weapon.

					// Add the weapon to my inventory.
					AuItem* item = (*itemI).second->Clone();
					item->SetCount(1);
					if(AddGearItemUseDropNoLock(*item, 1, false))
					{
						// Take a weapon from the city's inventory.
						itemsTaken.AddItemCount(*(*itemI).second, 1);
						cinv.TakeItem(*(*itemI).second, 1);

						// Take ammo from the city's inventory.
						AuItem* ammoItem = (*ammoI).second->Clone();
						ammoItem->SetCount(1);
						int ammoToTake = 5;				// Enough to fill free hand and 4 waist positions.
						if(ammoToTake > (int)(*ammoI).second->Count())
							ammoToTake = (int)(*ammoI).second->Count();
						for(; ammoToTake; ammoToTake--)
						{
							// Add the ammo to my inventory.
							if(!AddGearItemUseDropNoLock(*ammoItem, 0, false))	// Try to add one ammo to my inventory.
								break;	// Stop trying if no more will fit.
							itemsTaken.AddItemCount(*(*ammoI).second, 1);
							cinv.TakeItem(*(*ammoI).second, 1);	// Take one ammo from the city's inventory.
						}
						delete ammoItem;
						ammoItem = 0;
					}
					delete item;
					item = 0;
				}
				else
				{
					// Take a weapon from the city's inventory that doesn't need ammo.
					// Add the weapon to my inventory.
					AuItem* item = (*itemI).second->Clone();
					item->SetCount(1);
					if(AddGearItemUseDropNoLock(*item, 1, false))
					{
						// Take a weapon from the city's inventory that doesn't need ammo.
						itemsTaken.AddItemCount(*(*itemI).second, 1);
						cinv.TakeItem(*(*itemI).second, 1);
					}
					delete item;
					item = 0;
				}
				break;
			}
		}
	}

	if(armor && armorTypes)
	{
		// Choose armor.
		const AuItem* gearItem = m_Gear.ArmorItemWorn();
		bool foundMine = false;
		for(int ii = 0; ii < armorTypes; ii++)
		{
			if(gearItem && armor[ii] == gearItem->CommodityId())
				foundMine = true;		// We have reached my kind of armor in the list.

			AuItemHash::const_iterator itemI = cinv.FindBest(armor[ii]);			// Equip with the best.
			if(itemI != cinv.end())
			{
				// Decide whether to upgrade my armor.
				if(gearItem)
				{
					// The list of items is supplied in order of preference.
					// If we haven't gotten to our own armor type yet, we'll take any that we find.
					if(foundMine)
					{
						// We have reached our current armor type in the list.
						// At this point we will take the same kind if the quality is higher.
						// If it's a different kind or the quality is not an improvement, we don't want it and we're done.
						if(gearItem->CommodityId() != (*itemI).second->CommodityId() ||
								gearItem->Quality() >= (*itemI).second->Quality())
							break;								// The nu armor is not an improvement over what I am wearing.
					}
				}

				// Add the armor to my inventory.
				AuItem* item = (*itemI).second->Clone();
				item->SetCount(1);
				if(AddGearItemUseDropNoLock(*item, 1, false))
				{
					// Take armor from the city's inventory.
					itemsTaken.AddItemCount(*(*itemI).second, 1);
					cinv.TakeItem(*(*itemI).second, 1);
				}
				delete item;
				item = 0;
				break;
			}
		}
	}
	
	if(helmet && helmetTypes)
	{
		// Choose a helmet.
		const AuItem* gearItem = m_Gear.HelmetItemWorn();
		bool foundMine = false;
		for(int ii = 0; ii < helmetTypes; ii++)
		{
			if(gearItem && helmet[ii] == gearItem->CommodityId())
				foundMine = true;		// We have reached my kind of helmet in the list.

			AuItemHash::const_iterator itemI = cinv.FindBest(helmet[ii]);			// Equip with the best.
			if(itemI != cinv.end())
			{
				// Decide whether to upgrade my helmet.
				if(gearItem)
				{
					// The list of items is supplied in order of preference.
					// If we haven't gotten to our own helmet type yet, we'll take any that we find.
					if(foundMine)
					{
						// We have reached our current helmet type in the list.
						// At this point we will take the same kind if the quality is higher.
						// If it's a different kind or the quality is not an improvement, we don't want it and we're done.
						if(gearItem->CommodityId() != (*itemI).second->CommodityId() ||
								gearItem->Quality() >= (*itemI).second->Quality())
							break;								// The nu helmet is not an improvement over what I am wearing.
					}
				}

				// Add the helmet to my inventory.
				AuItem* item = (*itemI).second->Clone();
				item->SetCount(1);
				if(AddGearItemUseDropNoLock(*item, 1, false))
				{
					// Take a helmet from the city's inventory.
					itemsTaken.AddItemCount(*(*itemI).second, 1);
					cinv.TakeItem(*(*itemI).second, 1);
				}
				delete item;
				item = 0;
				break;
			}
		}
	}

	if(clothing && clothingTypes)
	{
		// Choose clothing.
		// Don't change my clothing if I am already wearing any at all.
		if(!m_Gear.ClothingItemWorn())
		{
			for(int ii = 0; ii < clothingTypes; ii++)
			{
				AuItemHash::const_iterator itemI = cinv.find(clothing[ii]);	// Not necessarily the best clothing.
				if(itemI != cinv.end())
				{
					// Add the clothing to my inventory.
					AuItem* item = (*itemI).second->Clone();
					item->SetCount(1);
					if(AddGearItemUseDropNoLock(*item, 1, false))
					{
						// Take clothing from the city's inventory.
						itemsTaken.AddItemCount(*(*itemI).second, 1);
						cinv.TakeItem(*(*itemI).second, 1);
					}
					delete item;
					item = 0;
					break;
				}
			}
		}
	}

	if(items && itemTypes)
	{
		int itemType = g_Au.Dice().Roll(1,itemTypes,-1);
		AuItemHash::const_iterator itemI = cinv.find(items[itemType]);
		if(itemI != cinv.end())
		{
			// Add the item to my inventory.
			AuItem* item = (*itemI).second->Clone();
			item->SetCount(1);
			if(AddGearItemUseDropNoLock(*item, 0, false))
			{
				// Take item from the city's inventory.
				itemsTaken.AddItemCount(*(*itemI).second, 1);
				cinv.TakeItem(*(*itemI).second, 1);
			}
			delete item;
			item = 0;
		}
	}
	m_GearLock.unlock();
}

void DaCitizen::InitPirateGear(quint8 maxQ)
{
	// Clothing by gender, order irrelevant.
	COMMODITYID clothingM[] =
	{
		CID_BlackUniform,
		CID_GreyUniform,
		CID_BrownUniform,
		CID_OrangeUniform,
		CID_GreenUniform,
		CID_RedUniform,
		CID_BlueUniform,
		CID_GoldUniform,
		CID_WhiteUniform,
		CID_TealUniform,
		CID_PinkUniform,
		CID_LeatherClothes,
		CID_JeansAndShirt,
	};
	int clothingTypesM = sizeof(clothingM)/sizeof(COMMODITYID);

	COMMODITYID clothingF[] =
	{
		CID_BlackUniform,
		CID_GreyUniform,
		CID_BrownUniform,
		CID_OrangeUniform,
		CID_GreenUniform,
		CID_RedUniform,
		CID_BlueUniform,
		CID_GoldUniform,
		CID_WhiteUniform,
		CID_PinkUniform,
		CID_TealUniform,
		CID_LeatherClothes,
		CID_JeansAndShirt,
		CID_CapriHalterOutfit,
	};
	int clothingTypesF = sizeof(clothingF)/sizeof(COMMODITYID);

	// Choose a random item that any citizen might have.
	// Choose an item that can be carried. Do not choose clothing, weapons, ammunition or armor.
	COMMODITYID items[] =
	{
		//CID_Invalid = 0,
		CID_Flag,
		CID_Computer,
		//CID_Electricity,
		CID_Hammer,
		CID_Air,				// breathable compressed air
		//CID_Helicopter,
		//CID_SpaceRocket,
		//CID_SpaceFighter,
		//CID_SpaceTransport,
		//CID_PistolAmmo,
		//CID_HeavyArmsAmmo,
		//CID_PlasticExplosive,
		//CID_Animal = 14,
		CID_Beans,
		CID_Beer,
		//CID_LeatherBodyArmor,
		CID_Bread,
		CID_Candy,
		CID_Cheese,
		CID_Coal,
		CID_CrudeOil,
		CID_Crystals,
		//CID_Dynamite,
		CID_Eggs,
		CID_ElectronicPart,
		//CID_FarmMachinery,
		CID_Fertilizer,
		CID_Fiber,
		//CID_Pistol = 31,
		//CID_Rifle,
		CID_Fish,
		CID_Flour,
		CID_Fruit,
		CID_Gasoline,
		CID_Gems,
		CID_Grain,
		CID_Grapes,
		CID_GunPowder,
		//CID_WovenHelmet,
		CID_Herbs,
		CID_Hops,
		CID_Hydrogen,
		CID_Ice,
		CID_CrystalJewelry,
		CID_Leather,
		CID_Lumber,
		CID_AnimalMeat,
		CID_MechanicalPart,
		CID_Metal,
		CID_IceCream,
		CID_Milk,
		CID_Minerals,
		//CID_MineMachinery,
		CID_NaturalGas,
		CID_Nuts,
		CID_Ore,
		CID_Pastry,
		CID_Petrochemicals,
		CID_Pharmaceuticals,
		CID_Plastic,
		CID_Radioactives,
		//CID_CommercialShip,
		//CID_NavalShip,
		//CID_VacSuitHelmet,
		CID_MedicalKit,
		CID_Spices,
		//CID_RifleAmmo,
		CID_Stone,
		//CID_Submarine,
		CID_Sugar,
		CID_Textiles,
		//CID_VacSuit,
		CID_Vegetable,
		//CID_SUV,
		CID_Water,
		CID_Whiskey,
		CID_Wine,
		CID_Wood,
		//CID_Atmosphere,
		//CID_Hydrographics,
		//CID_Vegetation,
		//CID_PlasticBodyArmor,
		//CID_WovenBodyArmor,
		//CID_NitrateExplosive,
		//CID_Sunlight,
		//CID_LeatherClothes,
		//CID_LeatherHelmet,
		//CID_PlasticHelmet,
		CID_GoldBracelet,
		CID_GoldRing,
		CID_Gold,
		CID_GemRing,
		CID_FishMeat,
		CID_Vodka,
		//CID_HeavyWeapon,
		//CID_Tank,
		//CID_APC,
		CID_Crate,
		CID_Beacon,
		CID_Lantern,
		CID_Knapsack,
		CID_Backpack,
		CID_Lumenite,
		CID_BlankDisk,
		CID_Shovel,
		CID_Gin,
		//CID_Knife,
		//CID_Shotgun,
		//CID_ShotgunAmmo,
		CID_Antidote,
		CID_Antitoxin,
		CID_Rum,
		CID_Tequila,
		CID_FittingWrench,
		CID_GlueGun,
		CID_ScrewDriver,
		CID_SewingNeedle,
		CID_SolderIron,
		CID_Tongs,
		CID_Wrench,
		CID_Hay,
		CID_Candle,
		CID_Torch,
		//CID_SailBoat,
		//CID_DirtBike,
		CID_Eludium,
		CID_Pliers,
		CID_TorqueWrench,
		CID_Vulcanite,
		CID_Ioplasma,
		CID_Magmex,
		CID_Phlogiston,
		CID_Bolite,
		CID_Cryozine,
		CID_Myrathane,
		CID_Polytaride,
		CID_Vulcium,
		CID_Magmium,
		//CID_Magma,
		//CID_SupercooledLiquid,
		//CID_InfernoAtmosphere,
		//CID_FrigidAtmosphere,
		//CID_MagmiumKnife,
		//CID_VulciumKnife,
		//CID_MagmiumPowerArmor,
		//CID_MagmiumPowerArmorHelmet,
		//CID_VulciumPowerArmor,
		//CID_VulciumPowerArmorHelmet,
		//CID_Money,
		CID_GravCoupling,
		CID_CryoHeatSink,
		CID_IoTube,
		CID_Clamp,
		CID_OScope,
		//CID_AdvSpaceFighter,
		//CID_MissionDisk,
		//CID_Bone,
		//CID_CommandAndControlModule,
		//CID_GravityDriveModule,
		//CID_RocketDriveModule,
		//CID_LifeSupportModule,
		//CID_PowerPlanetModule,
		//CID_EnergyShieldModule,
		//CID_KineticShieldModule,
		//CID_CloakingShieldModule,
		//CID_SurgeryMedicalModule,
		//CID_GeneticsMedicalModule,
		//CID_CyberneticsMedicalModule,
		//CID_PersonnelTransporterModule,
		//CID_VehicleTransporterModule,
		//CID_AtmosphereCondenserBayModule,
		//CID_ProspectorBayModule,
		//CID_TractorBeamBayModule,
		//CID_WormholeFTLDriveModule,
		//CID_WarpFTLDriveModule,
		//CID_WeaponBayModule,
		//CID_GunTurretModule,
		//CID_SensorModule,
		CID_MagmiumHeatSink,
		CID_VulciumHeatSink,
		CID_Myrasplicer,
		CID_Microscope,
		CID_Gigacell,
		CID_TesloidDissipator,
		CID_NanopathCalibrator,
		CID_AntifluxParticles,
		CID_BorexinoPrecipitate,
		//CID_TypeOPreons,
		//CID_TypeBPreons,
		//CID_TypeAPreons,
		//CID_TypeFPreons,
		//CID_TypeGPreons,
		//CID_TypeKPreons,
		//CID_TypeMPreons,
		//CID_MagmiumHullModule,
		//CID_VulciumHullModule,
		CID_Paper,
		CID_RedBinder,
		CID_GreenBinder,
		CID_BlueBinder,
		//CID_ParticleCollectorBayModule,
		CID_DNASampleKit,
		//CID_DNASample,
		CID_UtilityPouch,
		CID_GravPack,
		CID_Silage,
		CID_PreonRingO,
		CID_PreonRingB,
		CID_PreonRingA,
		CID_PreonRingF,
		CID_PreonRingG,
		CID_PreonRingK,
		CID_PreonRingM,
		//CID_Count				// LAST ITEM
	};
	int itemTypes = sizeof(items)/sizeof(COMMODITYID);

	if(Gender() == Female)
		InitPirateGear(maxQ,
				0, 0, 0,								// Weapon/ammo
				0, 0,												// Armor
				0, 0,												// Helmet
				clothingF, clothingTypesF,							// Clothing
				items, itemTypes);									// Items
	else
		InitPirateGear(maxQ,
				0, 0, 0,								// Weapon/ammo
				0, 0,												// Armor
				0, 0,												// Helmet
				clothingM, clothingTypesM,							// Clothing
				items, itemTypes);									// Items
}

void DaCitizen::InitPirateGear(quint8 maxQ,
		COMMODITYID* weapon, COMMODITYID* ammo, int weaponTypes,
		COMMODITYID* armor, int armorTypes,
		COMMODITYID* helmet, int helmetTypes,
		COMMODITYID* clothing, int clothingTypes,
		COMMODITYID* items, int itemTypes)
{
	// Create weapon.
	if(weapon && weaponTypes)
	{
		int weaponType = g_Au.Dice().Roll(1, weaponTypes, -1);
		{
			quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
			if(!quality)
				quality = 1;
			AuItem* item = AuItem::Create(weapon[weaponType], quality, EmpireId());
			if(item)
			{
				AddGearItemUseDrop(*item, 1, false);
				delete item;
			}
		}

		// If the weapon needs ammo, make that too.
		if(ammo && ammo[weaponType] != CID_Invalid)
		{
			// The weapon needs ammo.
			// Create ammo.
			quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
			if(!quality)
				quality = 1;
			AuItem* item = AuItem::Create(ammo[weaponType], quality, EmpireId());
			if(item)
			{
				// Enough to fill hand and 4 waist positions. Back is reserved for my random item.
				for(int ammoToTake = 5; ammoToTake; ammoToTake--)
					if(!AddGearItemUseDrop(*item, 0, false))	// Try to add one ammo to my inventory.
						break;	// Stop trying if no more will fit.

				delete item;
			}
		}
	}

	// Create armor.
	if(armor && armorTypes)
	{
		int armorType = g_Au.Dice().Roll(1, armorTypes, -1);
		quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
		if(!quality)
			quality = 1;
		AuItem* item = AuItem::Create(armor[armorType], quality, EmpireId());
		if(item)
		{
			AddGearItemUseDrop(*item, 1, false);
			delete item;
		}
	}

	// Create helmet.
	if(helmet && helmetTypes)
	{
		int helmetType = g_Au.Dice().Roll(1, helmetTypes, -1);
		quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
		if(!quality)
			quality = 1;
		AuItem* item = AuItem::Create(helmet[helmetType], quality, EmpireId());
		if(item)
		{
			AddGearItemUseDrop(*item, 1, false);
			delete item;
		}
	}

	// Create clothing.
	if(clothing && clothingTypes)
	{
		int clothingType = g_Au.Dice().Roll(1, clothingTypes, -1);
		quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
		if(!quality)
			quality = 1;
		AuItem* item = AuItem::Create(clothing[clothingType], quality, EmpireId(), 1);
		if(item)
		{
			AddGearItemUseDrop(*item, 1, false);
			delete item;
		}
	}

	// Create an item.
	if(items && itemTypes)
	{
		int itemType = g_Au.Dice().Roll(1,itemTypes,-1);
		quint8 quality = g_Au.Dice().Roll(1,maxQ/2,maxQ/2);
		if(!quality)
			quality = 1;
		AuItem* item = AuItem::Create(items[itemType], quality, EmpireId(), 1);
		if(item)
		{
			AddGearItemUseDrop(*item, 0, false);
			delete item;
		}
	}
}

void DaCitizen::SetAmmo(COMMODITYID ammoCID)
{
	// Drop all the ammo I am currently carrying.
	m_GearLock.lockForWrite();
	SetAmmoCountNoLock(ammoCID, 0);
	if(ammoCID == CID_Invalid)
	{
		// Ammo type has been set to none. Drop all my ammo at my feet.
		m_GearLock.unlock();
		return;
	}

	// Load up with the ammo I am supposed to use.
	// The most I could possibly carry would be six clips since a weapon will be in one of my hands.
	SetAmmoCountNoLock(ammoCID, 6);
	m_GearLock.unlock();
}

void DaCitizen::SetAmmoCount(int count)
{
	m_GearLock.lockForWrite();
	SetAmmoCountNoLock(count);
	m_GearLock.unlock();
}

void DaCitizen::SetAmmoCountNoLock(int count)
{
	if(count <= 0)
		SetAmmoCountNoLock(CID_Invalid, 0);
	else
	{
		// Determine the commodity ID of my current ammo.
		for(AuGear::const_iterator ii = m_Gear.cbegin(); ii != m_Gear.cend(); ii++)
		{
			const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
			if(!ammo)
				continue;	// The item is not ammo.
			// Get more of the same ammo.
			SetAmmoCountNoLock((*ii).Item().CommodityId(), count);
			return;
		}

		// I don't have any ammo so locate ammo in the hold that fits my gun.
		AMMOTYPE ammoType = AMMO_None;
		const AuItem* gearItem = m_Gear.ItemInSlot(AuGearSlot::Hand);
		if(gearItem)
		{
			const AuItemWeaponAmmo* wammo = dynamic_cast<const AuItemWeaponAmmo*>(gearItem);
			if(wammo)
				ammoType = wammo->Ammo1Type();
		}
		if(ammoType != AMMO_None && IsInSpacecraft())
		{
			// In a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(!ship || (!ship->Is(AssignedShip()) && ShipRoomType() != ROOM_Hold && ShipRoomType() != ROOM_Barracks))
				return;

			// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
			// barracks, which enables access to weapons in the hold.
			// Look through ship's cargo for ammo for my weapon.
			const AuItemHash& sinv = ship->LockCargoForRead();
			for(AuItemHash::const_iterator ci = sinv.begin(); ci != sinv.end(); ci++)
			{
				const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>((*ci).second);
				if(!ammo)
					continue;	// The cargo item is not ammo.
				if(ammo->AmmoType() != ammoType || DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
					continue;	// The ammo does not fit my weapon.
				COMMODITYID ammoCID = ammo->CommodityId();
				ship->UnlockCargo();

				SetAmmoCountNoLock(ammoCID, count);
				return;
			}
			ship->UnlockCargo();
		}

		// No ammo in hold so look for ammo in the area.
		DaItem* ammoItem;
		AuPoint ammoDir;
		float ammoRange;
		DaCitizen::LookForAmmoType(ammoType, ammoItem, ammoDir, ammoRange);
		if(ammoItem)
			SetAmmoCountNoLock(ammoItem->CommodityId(), count);
	}
}

void DaCitizen::SetAmmoCountNoLock(COMMODITYID ammoCID, int count)
{
	if(count < 0)
		return;

	// Count my total ammo.
	int ammoCount = 0;
	for(AuGear::const_iterator ii = m_Gear.cbegin(); ii != m_Gear.cend(); ii++)
	{
		const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
		if(!ammo)
			continue;	// The item is not ammo.
		ammoCount++;
	}
	if(ammoCount == count)
		return;
	//qDebug("DaCitizen::SetAmmoCountNoLock cid %d count %d ammoCount %d", ammoCID, count, ammoCount);

	DaVessel* ship = 0;
	if(IsInSpacecraft())
		ship = FindParentVesselInEnv();
	if(ship && (ship->Is(AssignedShip()) || ShipRoomType() == ROOM_Hold || ShipRoomType() == ROOM_Barracks))
	{
		// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
		// barracks, which enables access to ammo in the hold.
		if(ammoCount > count)
		{
			// Unload some of my ammo.
			//qDebug("Unload some of my ammo to the ship.");
			AuGear::iterator ii = m_Gear.begin();
			while(ii != m_Gear.end() && ammoCount > count)
			{
				const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
				if(!ammo)
				{
					ii++;
					continue;	// The item is not ammo.
				}

				// Add the ammo to the hold of the ship; drop it if there's no room.
				if(ship->CanAddCargo(*ammo))
					ship->AddCargo(*ammo);
				else
					DropItem(*ammo);

				// Remove the ammo from my inventory.
				ii = m_Gear.erase(ii);
				ammoCount--;
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
		else if(ammoCID != CID_Invalid)
		{
			// Load some ammo.
			//qDebug("Load some ammo from the ship.");
			// The most I could possibly carry would be five clips since the weapon will be in one of my hands.
			for(int clip=0; clip<5 && ammoCount < count; clip++)
			{
				// Look through ship's cargo for ammo for my weapon.
				const AuItemHash& sinv = ship->LockCargoForRead();
				AuItemHash::const_iterator ci = sinv.FindBest(ammoCID);
				if(ci == sinv.end())
				{
					ship->UnlockCargo();
					break;		// No more ammo in ship's hold.
				}

				AuItem* nuAmmo = (*ci).second->Clone();
				ship->UnlockCargo();

				nuAmmo->SetCount(1);
				if(AddGearItemUseDropNoLock(*nuAmmo, 0, true))
				{
					ship->RemoveCargo(*nuAmmo, 1);
					ammoCount++;
					delete nuAmmo;
				}
				else
				{
					delete nuAmmo;
					break;	// My inventory is full. Can't load more ammo.
				}
			}
			//qDebug("Loaded some ammo from the ship. Final ammoCount %d", ammoCount);
		}
	}
	else
	{
		// Here if not in a spacecraft or not in a place that has access to a spacecraft's hold.
		// To facilitate transferring ammo, excess is dropped to the ground at a troop's feet. When loading, the closest ammo within
		// a 100' radius is picked up.
		if(ammoCount > count)
		{
			// Unload some of my ammo by dropping it.
			AuGear::iterator ii = m_Gear.begin();
			while(ii != m_Gear.end() && ammoCount > count)
			{
				const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
				if(!ammo)
				{
					ii++;
					continue;	// The item is not ammo.
				}

				// Drop the ammo to the ground.
				DropItem(*ammo);

				// Remove the ammo from my inventory.
				ii = m_Gear.erase(ii);
				ammoCount--;
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
		else if(ammoCID != CID_Invalid)
		{
			// Load some ammo by locating it in the area.
			// The most I could possibly carry would be five clips since the weapon will be in one of my hands.
			for(int clip=0; clip<5 && ammoCount < count; clip++)
			{
				// Look in my area for ammo for my weapon.
				DaItem* ammoItem;
				AuPoint ammoDir;
				float ammoRange;
				LookForAmmoCID(ammoCID, ammoItem, ammoDir, ammoRange);
				if(!ammoItem)
					break;	// No more ammo to be found.

				// Found an ammo item to pick up.
				const AuItemAmmo* item = dynamic_cast<const AuItemAmmo*>(ammoItem->LockItem());
				if(!item)
				{
					ammoItem->UnlockItem();
					break;	// Not likely.
				}

				// Add the item to this atom's inventory if I have room.
				if(AddGearItemUseDropNoLock(*item, 0, true))
				{
					// The item fit into my inventory somewhere.
					ammoItem->UnlockItem();
					ammoCount++;
					ammoItem->Kill();					// Delete the ammo item atom from the environment.
				}
				else
				{
					// The item would not fit into my inventory.
					ammoItem->UnlockItem();
					break;
				}
			}
		}
	}
}

// 'item' is invalid to remove my current armor without wearing anything else instead.
void DaCitizen::SetArmor(const AuItem& item)
{
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::BodyWear;
	m_GearLock.lockForWrite();
	if(item.IsValid())
	{
		const AuItem* gearItem = m_Gear.ArmorItemWorn();
		if(gearItem && item.IsItem(*gearItem))
		{
			m_GearLock.unlock();
			return;	// Already using that gear.
		}
	}

	// Locate the gear I am supposed to use.
	DaVessel* ship = 0;
	if(IsInSpacecraft())
		ship = FindParentVesselInEnv();
	if(ship && (ship->Is(AssignedShip()) || ShipRoomType() == ROOM_Hold || ShipRoomType() == ROOM_Barracks))
	{
		// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
		// barracks, which enables access to items in the hold.
		AuItem* nuGear = 0;
		if(item.IsValid())
		{
			// Selecting a nu gear.
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator cargoI = sinv.Find(item);
			if(cargoI == sinv.end())
			{
				ship->UnlockCargo();
				m_GearLock.unlock();
				return;
			}

			nuGear = (*cargoI).second->Clone();
			ship->UnlockCargo();

			nuGear->SetCount(1);

			if(!CanUseGear(*nuGear, slotType))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// This citizen is not permitted to use that item.
			}
			if(!ship->RemoveCargo(*nuGear, 1))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// Couldn't take my nu gear from the hold so don't continue.
			}
		}

		// Put my current gear into the cargo hold.
		AuGearPos pos = m_Gear.FindArmorWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Add the gear to the hold of the ship; drop it if there's no room.
			if(ship->CanAddCargo(*gearItem))
				ship->AddCargo(*gearItem);
			else
				DropItem(*gearItem);

			// Remove the ammo from my inventory.
			m_Gear.Erase(pos);
		}

		// Put the nu item into my gear, if any. I might be selecting no item at all.
		if(nuGear)
		{
			m_Gear.Insert(slotType, m_Gear.NextWhich(slotType), *nuGear);
			delete nuGear;
		}

		// Signal changes to my inventory.
		UpdateGearWeight();
	}
	else if(!item.IsValid())
	{
		AuGearPos pos = m_Gear.FindArmorWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Not in a spacecraft and I am supposed to drop my item.
			// Drop my item to the ground.
			DropItem(*gearItem);

			// Remove the item from my inventory.
			m_Gear.Erase(pos);

			// When drifting in space, an inventory change could allow me to begin to be pulled by gravity.
			if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			{
				SetInMotion();
				TickTock();
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
	}
	m_GearLock.unlock();
}

void DaCitizen::SetClothing(const AuItem& item)
{
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::BodyWear;
	m_GearLock.lockForWrite();
	if(item.IsValid())
	{
		const AuItem* gearItem = m_Gear.ClothingItemWorn();
		if(gearItem && item.IsItem(*gearItem))
		{
			m_GearLock.unlock();
			return;	// Already using that gear.
		}
	}

	// Locate the gear I am supposed to use.
	DaVessel* ship = 0;
	if(IsInSpacecraft())
		ship = FindParentVesselInEnv();
	if(ship && (ship->Is(AssignedShip()) || ShipRoomType() == ROOM_Hold || ShipRoomType() == ROOM_Barracks))
	{
		// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
		// barracks, which enables access to items in the hold.
		AuItem* nuGear = 0;
		if(item.IsValid())
		{
			// Selecting a nu gear.
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator cargoI = sinv.Find(item);
			if(cargoI == sinv.end())
			{
				ship->UnlockCargo();
				m_GearLock.unlock();
				return;
			}

			nuGear = (*cargoI).second->Clone();
			ship->UnlockCargo();

			nuGear->SetCount(1);

			if(!CanUseGear(*nuGear, slotType))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// This citizen is not permitted to use that item.
			}
			if(!ship->RemoveCargo(*nuGear, 1))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// Couldn't take my nu gear from the hold so don't continue.
			}
		}

		// Put my current gear into the cargo hold.
		AuGearPos pos = m_Gear.FindClothingWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Add the gear to the hold of the ship; drop it if there's no room.
			if(ship->CanAddCargo(*gearItem))
				ship->AddCargo(*gearItem);
			else
				DropItem(*gearItem);

			// Remove the ammo from my inventory.
			m_Gear.Erase(pos);
		}

		// Put the nu item into my gear, if any. I might be selecting no item at all.
		if(nuGear)
		{
			m_Gear.Insert(slotType, m_Gear.NextWhich(slotType), *nuGear);
			delete nuGear;
		}

		// Signal changes to my inventory.
		UpdateGearWeight();
	}
	else if(!item.IsValid())
	{
		AuGearPos pos = m_Gear.FindClothingWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Not in a spacecraft and I am supposed to drop my item.
			// Drop my item to the ground.
			DropItem(*gearItem);

			// Remove the item from my inventory.
			m_Gear.Erase(pos);

			// When drifting in space, an inventory change could allow me to begin to be pulled by gravity.
			if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			{
				SetInMotion();
				TickTock();
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
	}
	m_GearLock.unlock();
}

void DaCitizen::SetHelmet(const AuItem& item)
{
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::Head;
	m_GearLock.lockForWrite();
	if(item.IsValid())
	{
		const AuItem* gearItem = m_Gear.HelmetItemWorn();
		if(gearItem && item.IsItem(*gearItem))
		{
			m_GearLock.unlock();
			return;	// Already using that gear.
		}
	}

	// Locate the gear I am supposed to use.
	DaVessel* ship = 0;
	if(IsInSpacecraft())
		ship = FindParentVesselInEnv();
	if(ship && (ship->Is(AssignedShip()) || ShipRoomType() == ROOM_Hold || ShipRoomType() == ROOM_Barracks))
	{
		// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
		// barracks, which enables access to items in the hold.
		AuItem* nuGear = 0;
		if(item.IsValid())
		{
			// Selecting a nu gear.
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator cargoI = sinv.Find(item);
			if(cargoI == sinv.end())
			{
				ship->UnlockCargo();
				m_GearLock.unlock();
				return;
			}

			nuGear = (*cargoI).second->Clone();
			ship->UnlockCargo();

			nuGear->SetCount(1);

			if(!CanUseGear(*nuGear, slotType))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// This citizen is not permitted to use that item.
			}
			if(!ship->RemoveCargo(*nuGear, 1))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// Couldn't take my nu gear from the hold so don't continue.
			}
		}

		// Put my current gear into the cargo hold.
		AuGearPos pos = m_Gear.FindHelmetWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Add the gear to the hold of the ship; drop it if there's no room.
			if(ship->CanAddCargo(*gearItem))
				ship->AddCargo(*gearItem);
			else
				DropItem(*gearItem);

			// Remove the ammo from my inventory.
			m_Gear.Erase(pos);
		}

		// Put the nu item into my gear, if any. I might be selecting no item at all.
		if(nuGear)
		{
			m_Gear.Insert(slotType, m_Gear.NextWhich(slotType), *nuGear);
			delete nuGear;
		}

		// Signal changes to my inventory.
		UpdateGearWeight();
	}
	else if(!item.IsValid())
	{
		AuGearPos pos = m_Gear.FindHelmetWorn();
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Not in a spacecraft and I am supposed to drop my item.
			// Drop my item to the ground.
			DropItem(*gearItem);

			// Remove the item from my inventory.
			m_Gear.Erase(pos);

			// When drifting in space, an inventory change could allow me to begin to be pulled by gravity.
			if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			{
				SetInMotion();
				TickTock();
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
	}
	m_GearLock.unlock();
}

// 'item' is invalid when selecting no weapon.
void DaCitizen::SetWeapon(const AuItem& item)
{
	AuGearSlot::SLOTTYPE slotType = AuGearSlot::Hand;
	m_GearLock.lockForWrite();
	if(item.IsValid())
	{
		const AuItem* gearItem = m_Gear.ItemInSlot(slotType);
		if(gearItem && item.IsItem(*gearItem))
		{
			m_GearLock.unlock();
			return;	// Already using that weapon.
		}
	}

	// Locate the weapon I am supposed to use.
	DaVessel* ship = 0;
	if(IsInSpacecraft())
		ship = FindParentVesselInEnv();
	if(ship && (ship->Is(AssignedShip()) || ShipRoomType() == ROOM_Hold || ShipRoomType() == ROOM_Barracks))
	{
		// Found the ship and it is my assigned ship, which means I can access hold items anywhere, or I am in the hold or
		// barracks, which enables access to weapons in the hold.
		AuItem* nuGear = 0;
		if(item.IsValid())
		{
			// Selecting a nu weapon.
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator cargoI = sinv.Find(item);
			if(cargoI == sinv.end())
			{
				ship->UnlockCargo();
				m_GearLock.unlock();
				return;
			}

			nuGear = (*cargoI).second->Clone();
			ship->UnlockCargo();

			nuGear->SetCount(1);

			if(!CanUseGear(*nuGear, slotType))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// This citizen is not permitted to use that item.
			}
			if(!ship->RemoveCargo(*nuGear, 1))
			{
				m_GearLock.unlock();
				delete nuGear;
				return;		// Couldn't take my nu gear from the hold so don't continue.
			}
		}

		// Put my current gear into the cargo hold.
		AuGearPos pos = m_Gear.WeaponSlotPos(slotType);
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Add the gear to the hold of the ship; drop it if there's no room.
			if(ship->CanAddCargo(*gearItem))
				ship->AddCargo(*gearItem);
			else
				DropItem(*gearItem);

			// Remove the ammo from my inventory.
			m_Gear.Erase(pos);
		}

		// Put the nu item into my gear, if any. I might be selecting no item at all.
		// Get ammo type of nu weapon so I can unload all of my ammo that does not fit the nu weapon.
		COMMODITYID ammoCID = CID_Invalid;
		AMMOTYPE ammoType = AMMO_None;
		if(nuGear)
		{
			const AuItemWeaponAmmo* wammo = dynamic_cast<const AuItemWeaponAmmo*>(nuGear);
			if(wammo)
				ammoType = wammo->Ammo1Type();

			m_Gear.Insert(slotType, m_Gear.NextWhich(slotType), *nuGear);
			delete nuGear;
		}

		// Unload all of my ammo that does not fit the nu weapon.
		AuGear::iterator ii = m_Gear.begin();
		while(ii != m_Gear.end())
		{
			const AuItemAmmo* ammo = dynamic_cast<const AuItemAmmo*>(&(*ii).Item());
			// We must explicitly skip energy weapons here. They are used like ammunition for bay weapons and turrets but
			// they can't be used as ammo for a laser rifle or laser pistol.
			if(!ammo || DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
			{
				ii++;
				continue;	// The item is not ammo.
			}
			if(ammo->AmmoType() == ammoType)
			{
				ammoCID = ammo->CommodityId();
				//qDebug("found ammo cid %d in inventory that fits weapon", ammoCID);
				ii++;
				continue;	// The ammo fits my nu weapon.
			}

			// Add the ammo to the hold of the ship; drop it if there's no room.
			if(ship->CanAddCargo(*ammo))
				ship->AddCargo(*ammo);
			else
				DropItem(*ammo);

			// Remove the ammo from my inventory.
			ii = m_Gear.erase(ii);
		}

		// Load up with ammo that fits the nu weapon.
		// The most I could possibly carry would be six clips since the weapon will be in one of my hands.
		if(ammoType != AMMO_None)
		{
			if(ammoCID != CID_Invalid)
				SetAmmoCountNoLock(ammoCID, 6);
			else
				SetAmmoCountNoLock(6);	// Try to locate suitable ammo.
		}

		// Signal changes to my inventory.
		UpdateGearWeight();
	}
	else if(!item.IsValid())
	{
		AuGearPos pos = m_Gear.WeaponSlotPos(slotType);
		const AuItem* gearItem = m_Gear.ItemAt(pos);
		if(gearItem)
		{
			// Not in a spacecraft and I am supposed to drop my item.
			// Drop my item to the ground.
			DropItem(*gearItem);

			// Remove the item from my inventory.
			m_Gear.Erase(pos);

			// Drop my ammo to the ground.
			SetAmmoCountNoLock(0);

			// When drifting in space, an inventory change could allow me to begin to be pulled by gravity.
			if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			{
				SetInMotion();
				TickTock();
			}

			// Signal changes to my inventory.
			UpdateGearWeight();
		}
	}
	m_GearLock.unlock();
}

void DaCitizen::operator>>(QDataStream& out) const
{
	DaSentient::operator>>(out);
	quint8 hasWhat = m_Lineage;		// Bottom nibble.
	if(m_ShipBerthType != AuRosterEntry::Invalid)
		hasWhat |= 0x80;

	out << hasWhat;
	out << m_Ship;
	if(hasWhat & 0x80)
		out << m_ShipBerthType;

	m_GearLock.lockForRead();
	out << m_Gear;
	out << AuTime();	//m_tmGearChange;
	out << m_WeightCarried;
	m_GearLock.unlock();
}

void DaCitizen::operator<<(QDataStream& in)
{
	DaSentient::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	m_Lineage = (LINEAGE_TYPE)(hasWhat & 0x0f);	// Bottom nibble.
	in >> m_Ship;
	if(hasWhat & 0x80)
		in >> m_ShipBerthType;

	m_GearLock.lockForWrite();
	in >> m_Gear;
	AuTime tmGearChange;
	in >> tmGearChange;
	in >> m_WeightCarried;
	m_GearLock.unlock();
}

bool DaCitizen::DepartingSolarSystem()
{
	// Citizens do not leave the solar system sans vehicle or spacecraft.
	ReleaseBerth();
	DismountAllRiders();
	Kill();
	return false;
}

void DaCitizen::KilledByPerson(DaPerson* killer)
{
	// This citizen was killed by a person.
	killer->CitizenKilled(this);
}

// Render OpenAL audio sources
void DaCitizen::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	// Some items make noise. Check my current item.
	AuGear* gear = LockGearForWrite();
	AuItem* gearItem = gear->ItemInSlot(AuGearSlot::Hand);
	if(gearItem && gearItem->HasSound())
	{
		AuMatrix4x4 im(model);
		im.Translate(WeaponLoc());

		float gain = RelativeAudioGain(sceneAtom) * fade;
		float pitch = 1;
		gearItem->RenderSound(im, *al, gain, pitch);
	}
	UnlockGear();
}

void DaCitizen::FindCitizensRecursive(DaAtomList& citizens) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindCitizensRecursive(citizens);
	citizens.append((DaCitizen*)this);	// Cast away constness.
}

bool DaCitizen::IsReadyForMating() const
{
	if(!g_Au.IsMatingOk())
		return false;			// Nerf solo sex.
	if(IsBodyDyingOrDead())
		return false;			// I am dead or almost there. No necrophilia.
	if(IsLockedPose())
		return false;			// Not while at any station.
	if(Gender() == Asexual)
		return false;			// Asexual creatures do not mate sexually.
	if(IsPregnant())
		return false;			// I am already pregnant.
	if(!IsFullGrown())
		return false;			// I am immature. No pedophilia here.
	return IsAirBreathable();	// I'll get naked if the air is breathable.
}

void DaCitizen::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire)
{
	DaSentient::ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	if(IsDead())
		return;

	bool anyChange = false;
	m_GearLock.lockForWrite();
	if(m_Gear.VoidOwnershipTitle(propertyId, ownerId, ownerName))
	{
		anyChange = true;
		m_tmGearChange.Set();
	}
	m_GearLock.unlock();
	if(anyChange)
		SetModified();
}

DaAtom::ATOM_TYPE DaCitizen::AtomType() const
{
	return ATOM_Citizen;
}

float DaCitizen::Density() const
{
	if(LivesInWater())
		return (float)DensityOfWater();		// Swimmers do what they want.
	else
		return (float)DensityOfWater() - 1;	// Non-swimmers float.
}

int DaCitizen::ComparePersonLineage(LINEAGE_TYPE personLineage) const
{
	switch(personLineage)
	{
	case LINEAGE_Avatar:
		// Person is highborn. All citizens are beneath highborns.
		return -1;
	case LINEAGE_Demiavatar:
		// Person is a demi. Demi citizen is the same rank.
		if(IsLineageDemiavatar())
			return 0;
		// All non-demi citizens are beneath demis.
		return -1;
	case LINEAGE_None:
		// Person is a noob.
		// Demi outranks human noob.
		if(IsLineageDemiavatar())
			return 1;
		// All non-demi citizens are equal to noobs.
		return 0;
	case LINEAGE_Citizen:
		// Person is a citizen? Not possible.
		break;
	}
	return -1;
}

void DaCitizen::SetGear(const AuGear& gear)
{
	m_GearLock.lockForWrite();
	m_Gear = gear;
	UpdateGearWeight();
	m_GearLock.unlock();
}

QString DaCitizen::OutcryCapturedByPirates() const
{
	const QString outcries[] =
	{
		tr("Don't let them take me."),
		tr("Get away from me with that thing."),
		tr("Don't hurt me."),
		tr("What is that?!"),
		tr("Aieeeee!"),
		tr("Nooooo!"),
		tr("Please, I have a family."),
		tr("Let me go. I know the emperor personally."),
		tr("Do I get a phone call?"),
		tr("I have cash, how much to let me go?"),
		tr("I'm going to be sick."),
		tr("Has anyone seen my dog? I can't go without my dog."),
		tr("I'm not worth anything as a slave. Let me go!"),
		tr("Captain! This wasn't in the contract. I shall lodge a formal complaint."),
		tr("Cheer up guys. How bad can it get?"),
		tr("How ironic. I captured my first wife this way."),
		tr("Does slavery pay well?"),
		tr("I hope we don't have to clean up after scavengers."),
		tr("Mommy!"),
		tr("It's cold in here. I have a bad feeling about this."),
		tr("Somebody turn down the heat!"),
		tr("Is that a pig they're roasting? Oh my..."),
		tr("It's so muggy. Is the life support system working?"),
		tr("Don't touch me with that filthy thing."),
		tr("You touch me, I'll kill you!"),
		tr("What is that odor? This place really stinks."),
		tr("Are those bugs? Bugs give me the creeps."),
		tr("Was that a rat? Vermin make my insides churn."),
		tr("Will we be sold soon? I really don't like space travel."),
		tr("You are going to sell us, right? You're not carnivores, are you?"),
		tr("Stay away! I'm intensely xenophobic."),
		tr("Don't take me; I'm one of the crew."),
		tr("Take me to your Captain. I have a proposition."),
		tr("Was that a scream coming from that passageway?"),
		tr("Torture? No! Well ok, if it doesn't hurt."),
		tr("I hope the rats taste better on this ship."),
		tr("Do you people work with Targoss?"),
		tr("How did I get here?"),
		tr("I'm going to hurl! Do these things ever bathe?"),
		tr("This ship is a wreck. Is something burning?"),
	};
	const int outcrycount = sizeof(outcries)/sizeof(QString);
	if(IsLineageDemiavatar())
		return outcries[g_Au.Dice().Roll(1,outcrycount,-1)] + tr(" My dad was an avatar; "
				"he's going to hunt you down and boy are you going to get it! "
				"Release me immediately.");
	else
		return outcries[g_Au.Dice().Roll(1,outcrycount,-1)];
}

