//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daanimal.cpp,v 1.541 2022/05/17 15:15:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daanimal.h"
//
#include "aucombat.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auscroute.h"
#include "ausensorblip.h"
#include "auterraininfo.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacapsule.h"
#include "dacitizen.h"
#include "daitem.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldll.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dpbody.h"
#include <QSqlError>
#include <qsqlquery.h>
#include <typeinfo>			// for bad_cast

DaAnimal::DaAnimal(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, GROWTHSTAGE stage) :
	DaCreature(species, chroma, hand, stage), m_AttackWeapon(WM_Not)
{
	m_Activity = Waiting;
	m_ActivityTick = 0;
	m_AssumePose = 0;
	m_Decision = 0;
	m_HeadTilt = 0;
	m_IsAlert = false;
	m_TargetRange = 0;
	m_TurnZ = 0;
	m_UseSpecWpn = false;
}

DaAnimal::DaAnimal() : m_AttackWeapon(WM_Not)
{
	m_Activity = Waiting;
	m_ActivityTick = 0;
	m_AssumePose = 0;
	m_Decision = 0;
	m_HeadTilt = 0;
	m_IsAlert = false;
	m_TargetRange = 0;
	m_TurnZ = 0;
	m_UseSpecWpn = false;
}

DaAnimal::~DaAnimal()
{
}

DaAnimal::DaAnimal(const DaAnimal &rhs) : DaCreature(rhs), m_AttackWeapon(WM_Not)
{
	// Do no read Think variables from rhs.
	m_Activity = Waiting;
	m_ActivityTick = 0;
	m_AssumePose = 0;
	m_Decision = 0;
	m_HeadTilt = 0;
	m_IsAlert = false;
	m_TargetRange = 0;
	m_TurnZ = 0;
	m_UseSpecWpn = false;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaAnimal::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaCreature::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	toClient << TurnZ();
}

// Receive updates to this atom from a server.
void DaAnimal::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaCreature::RxUpdate(fromServer, isFirstUpdate);

	fromServer >> m_TurnZ;
}

// Refresh data from valid counterpart.
bool DaAnimal::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaCreature::Refresh(pAtom, ari);

	const DaAnimal* pA = (const DaAnimal*)pAtom;
	//qDebug("DaAnimal::Refresh %s", (const char*)Name());

	pA->m_ThinkLock.lockForRead();
	m_ThinkLock.lockForWrite();
	m_TurnZ = pA->m_TurnZ;
	m_ThinkLock.unlock();
	pA->m_ThinkLock.unlock();

	return false;
}

DaAtom *DaAnimal::Clone() const
{
	return new DaAnimal(*this);
}

bool DaAnimal::IsTicker() const
{
	if(DaCreature::IsTicker())
		return true;

	if(IsFlying() && IsMounted() && IsTouchingGround())
		return true;

	if(!IsWild() || IsBodyDyingOrDead())
		return false;

	if(CanTurn() && TurnZ() != 0)
		return true;

	if(IsAtRest() && IsMovingPose())
		return true;

	return false;
}

void DaAnimal::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	//qDebug() << "DaAnimal::Tick" << Id() << Name() << "init" << NeedsInit() << "dying" << IsBodyDyingOrDead();
	DaCreature::Tick(last, now);
	if(IsDead())
		return;

	// Stop flying pose on the ground when carrying a rider.
	if(IsFlying() && IsMounted() && IsTouchingGround())
		SetGeneralPose(AuPose::Stand);

	if(!IsWild() || IsBodyDyingOrDead())
		return;

	// Calculate turn amounts based on turn rate and time span.
	if(CanTurn() && TurnZ() != 0)
	{
		// Calculate number of milliseconds elapsed since last tick.
		int msElapsed = last.elapsed();
		if(msElapsed < 0)
			msElapsed = 1;
		float rsec = (float)msElapsed/1000.f;	// Real world elapsed secs.
		float rMaxTurn = TurnRate() * rsec;		// Max turn in time period.

		m_ThinkLock.lockForWrite();
		float turnZNow;
		if(m_TurnZ > rMaxTurn)
		{
			turnZNow = rMaxTurn;
			m_TurnZ -= rMaxTurn;
		}
		else if(m_TurnZ < -rMaxTurn)
		{
			turnZNow = -rMaxTurn;
			m_TurnZ += rMaxTurn;
		}
		else
		{
			turnZNow = m_TurnZ;
			m_TurnZ = 0;
		}
		//qDebug() << "DaAnimal::Tick" << Id() << Name() << "turnZ" << (m_TurnZ*RAD2DEG) << "turnZNow" << (turnZNow*RAD2DEG)
			//<< "remainingTurn" << ((m_TurnZ - turnZNow)*RAD2DEG);
		m_ThinkLock.unlock();

		AuAtomPos tMe = Pos();
		tMe.RotateZ(turnZNow);
		ChangePosAndOrientNotParent(tMe);
		SetModified();
	}

	if(IsAtRest() && IsMovingPose())
	{
		SetInMotion();
		SetModified();
	}
}

unsigned int DaAnimal::IsTocker() const
{
	unsigned int ms = DaCreature::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	if(ms > 10*d_MSecPerSec)
	{
		if(IsInCapsule())
		{
			if(CapsuleRoomType() == ROOM_Fuel || CurrentGeneralPose().IsPatientStation())
				ms = 10*d_MSecPerSec;
		}
		else if(IsWild() || InIndustry() != IID_Farm ||
				(!IsAtRest() && CreatedTime().ElapsedMin()))
		{
			if(!IsMounted() && !IsInAnyScene())
				ms = 10*d_MSecPerSec;
		}

		if(IsInAnyScene() && HasAnyCondition())
			ms = 10*d_MSecPerSec;
	}

	if(IsBodyDyingOrDead())
	{
		if(!IsRider() ||	// Bleeding
				(!IsAirBreathable() && !IsMouthSubmerged()) ||	// Atmosphere damage
				(IsInVisibleWater() && (!LivesInWater() || !IsInSupercooledLiquid())))	// Liquid damage
		{
			if(ms == 0 || ms > d_MSecPerSec)
				ms = d_MSecPerSec;
		}
		return ms;
	}

	// Hunger
	if(ms > d_MSecPerMin)
		ms = d_MSecPerMin;

	if(IsWild() && !IsMounted())
	{
		if(ms > 200)
			ms = 200;
	}

	return ms;
}

// Return false if this atom is to be deleted upon return.
void DaAnimal::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	//if(IsInAnyScene())
		//qDebug() << "DaAnimal::Tock" << Id() << Name() << "init" << NeedsInit() << "dying" << IsBodyDyingOrDead();
	DaCreature::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;	// This atom (not body) is dead or awaiting transfer.

	// Take damage from the environment.
	// Test location to see if animal can be removed from the universe.
	if(IsInCapsule())
	{
		// Kill animals that somehow get into fuel cells.
		if(CapsuleRoomType() == ROOM_Fuel)
		{
			// Animals cannot live inside fuel tanks.
			DismountAllRiders();
			Kill();
			return;
		}
		else if(CurrentGeneralPose().IsPatientStation())
		{
			// I am a patient? Leave if the doc is gone.
			const DaCapsule* capsule = dynamic_cast<const DaCapsule*>(Parent());
			if(!capsule || !capsule->FindDoctor(*this))
				Stand();
		}
		else if(!IsInAnyScene())
		{
			if(LooksLikeVermin())
			{
				// Vermin disappear from ships when not in anyone's scene.
				// Vermin cannot have riders.
				Kill();
				return;
			}
			else
			{
				const DaCapsule* capsule = FindParentCapsuleInEnv();
				if(capsule && capsule->IsDesignDataCacheValid() && !capsule->TestTransportLocation(Location()))
				{
					// Creatures that are trapped disappear when not in anyone's scene.
					Kill();
					return;
				}
			}
		}
	}
	else if(IsWild() || InIndustry() != IID_Farm ||
			(!IsAtRest() && CreatedTime().ElapsedMin()))	// Moving and more than a minute old. Gives new one a chance to settle.
	{
		// Wild animal or animal not on a farm or moving animal.
		// Domestic animal can live if it's on a farm and not moving.
		if(!IsMounted() && !IsInAnyScene())
		{
			// The animal isn't in anyone's scene right now.
			if(!IsWild() && InIndustry() == IID_Farm && Vector() == AuPoint(0,0))
			{
				//qDebug() << "DaAnimal::Tock" << Id() << Name() << "setting immobile domestic animal at rest, created elapsedMin"
					//<< CreatedTime().ElapsedMin();
				SetAtRest();
			}
			else
			{
				//qDebug() << "DaAnimal::Tock" << Id() << Name() << "killing moving animal that is not in any scene";
				Kill();
				return;
			}
		}
	}

	if(IsInAnyScene())
		TickConditions();	// Conditions are ticked so they can periodically affect this animal.

	// Process dying state.
	if(last.ElapsedSecTicks(now) > 0)
	{
		// Body does not bleed when carried by another creature.
		if(IsBodyDyingOrDead() && !IsRider())
		{
			// Bleed out one point of health.
			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_InternalInjuries;
			weapon.m_w1.m_Effect = WE_Bleeding;

			AuCombatResult result;
			result.m_Damage1 = 1;

			CombatApplyDamageLater(AuCombatAttacker(), weapon, AuPoint(0,0), AuPoint(0,0), result);
		}

		// Take damage from the atmosphere if it is not breathable and I am not under water.
		if(!IsMouthSubmerged())
		{
			if(IsAirBreathable())
				Inhale();
			else if(!HoldBreath())
				ApplyAtmosphericDamage();
		}

		// Take damage from the surface liquid if it is not water.
		// "Water" in the inferno/frigid zone is not water.
		// Environment suit does not protect me from this.
		if(IsInVisibleWater())
		{
			// Some animals live in the supercooled liquid of gas giants.
			if(!LivesInWater() || !IsInSupercooledLiquid())
				ApplyLiquidDamage();
		}
	}

	// Remaining actions are not performed on creatures that are dead or dying.
	if(IsBodyDyingOrDead())
		return;

	// Calculate Gradual Hunger Growth on a per minute basis for wild animals.
	// Note that wild animals don't starve. Hunger affects their AI. They don't live long enough to starve to death.
	// Domestic animals don't get hungry...for now.
	if(last.ElapsedMinTicks(now) > 0)
	{
		if(IsWild())
		{
			if(CurrentHunger())
			{
				// Wild animal hunger value decays over time but they don't starve; they just get really hungry.
				// Eco strategy eat at different intervals.
				short int hungerGrowth = 0;
				switch(EcoStrategy())
				{
				case AuDNABase::ECO_Waiter:
				case AuDNABase::ECO_Hunter:
					// Gets hungry a few times per day.
					// This calc happens to be the same as player characters, which also happen to be ECO_Hunter types.
					if(last.MinIsOdd())
					{
						hungerGrowth = MaxHunger()/40;	// 1 point per 40 hp every other minute
						if(!hungerGrowth)
							hungerGrowth = 1;
					}
					break;
				case AuDNABase::ECO_Grazer:
					// Gets hungry a lot.
					hungerGrowth = CurrentHunger()/4;	// 1/4 of remaining hunger every minute.
					if(!hungerGrowth)
						hungerGrowth = CurrentHunger();
					break;
				case AuDNABase::ECO_Sneaker:
					// More active than the hunter so gets hungry slightly more often.
					hungerGrowth = MaxHunger()/40;	// 1 point per 40 hp every minute
					if(!hungerGrowth)
						hungerGrowth = 1;
					break;
				}
				if(hungerGrowth)
				{
					// This animal became hungrier.
					if(CurrentHunger() > hungerGrowth)
						SetHunger(CurrentHunger() - hungerGrowth);
					else
						SetHunger(1);	// Animals never quite starve due to its effect on stamina recovery.
				}
			}
		}
		else
		{
			// Domestic herbivores, e.g. cows, just stand and eat when conditions are right.
			if(IsHerbivore() && !IsEatingPose() && IsAirBreathable() &&
					(IsOnForest() || IsOnRock() || IsOnSand() || IsOnSavanna() || IsOnSnow()) &&
					InIndustry() != IID_Road &&
					dynamic_cast<const DaWorldLL*>(Parent()))
			{
				// Herbivore standing in the right environment for eating.
				EatOffGround();
				SetModified();
			}
		}
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Remainder from Think
	if(!IsWild() || IsMounted())
		return;

	// Wild animals have a mind of their own.
	MakeDecision(last, now);

	// Carry out current decision.
	ExecuteDecision();

	if(IsMovingPose())
	{
		// Creature is moving.
		if(!TurnZ() && (Vector().IsFuzzyZero() || last.ElapsedSecTicks(now) > 0))
		{
			// Animal is not turning.
			// Creature is just starting to move or a second has passed.
			// Make sure the environment ahead is appropriate.
			// i.e. Fish don't move onto land, etc.
			if(IsInAtmosphere() && !IsFlying() && !IsInCapsule())
			{
				// In the atmosphere range of a planet and not flying.
				// Check the terrain ahead.
				const DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					// Test a location 10 feet ahead, relative to the planet.
					AuMatrix4x4 trel;
					GetTransformRelativeToParent(trel, planet);
					AuPoint lookAhead = trel.Project(AuPoint(0,10,0));
					AuTerrainInfo terAhead;
					planet->QueryTerrainAtLoc(terAhead, lookAhead);

					bool badMove = false;
					DhDNA dna(DNA3());
					if(terAhead.IsWater())
					{
						// New location is in the water.
						if(!dna.LivesInWater)
						{
							// Creature doesn't live in the water.
							if(!IsInVisibleWater())
							{
								// Creature is not in the water.
								// Don't enter the water.
								badMove = true;
							}
							else
							{
								// Creature is already in the water.
								// Only move up hill to get out of the water.
								AuPoint myLoc = trel.Project(AuPoint(0,0));
								AuTerrainInfo myTer;
								planet->QueryTerrainAtLoc(myTer, myLoc);
								if(terAhead.m_rSurfaceRadius < myTer.m_rSurfaceRadius)
									badMove = true;	// Don't go deeper/lower in altitude.
							}
						}
						else if(!dna.LivesOnLand && !dna.LivesInAir && !InDeepEnoughLiquidToSwim())
						{
							// Aquatic creature is already in water that is too shallow.
							// Don't go shallower.
							// Only move down hill to get to deeper water.
							AuPoint myLoc = trel.Project(AuPoint(0,0));
							AuTerrainInfo myTer;
							planet->QueryTerrainAtLoc(myTer, myLoc);
							if(terAhead.m_rSurfaceRadius > myTer.m_rSurfaceRadius)
								badMove = true;	// Don't go shallower/higher in altitude.
						}
					}
					else
					{
						// New location is not in the water.
						if(!dna.LivesInAir && !dna.LivesOnLand)
						{
							// Creature doesn't live in air or on land.
							if(IsInVisibleWater())
							{
								// Creature is in the water.
								// Don't leave the water.
								badMove = true;
							}
							else
							{
								// Creature is not in the water.
								// Only move down hill to get into the water.
								AuPoint myLoc = trel.Project(AuPoint(0,0));
								AuTerrainInfo myTer;
								planet->QueryTerrainAtLoc(myTer, myLoc);
								if(terAhead.m_rSurfaceRadius > myTer.m_rSurfaceRadius)
									badMove = true;	// Don't go higher.
							}
						}
					}
					if(badMove)
					{
						// Turn a different direction.
						float turnZ = g_Au.Dice().Roll(1,361,-181);
						turnZ *= DEG2RADf;
						SetTurnZ(turnZ);
					}
				}
			}
		}
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

void DaAnimal::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	DaCreature::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	if(weapon.m_Mode != WM_Internal && CurrentHitPoints() > 0)
	{
		// I've been hurt! Become alert and test fight/flight reaction.
		m_IsAlert = true;
		FightOrFlightCheck(0, (result.m_Damage1 && result.m_Damage2) ? 2 : 1);
	}

	if(result.m_IsDestroyed)
	{
		//qDebug("DaAnimal::CombatApplyDamage %d has died of its wounds.", Id().GetID());
		if(IsInAnyScene() && !InDesignPreview())
		{
			AuId killerId = KillerId();
			QString killerName;
			if(!killerId.IsNull())
			{
				DaAtom* env = FindEnvironmentAtom();
				if(env)
				{
					DaAtom* killer = env->FindIdRecursive(killerId);
					if(killer)
						killerName = killer->Name();
				}
			}
			QString inscription;
			if(!killerName.isEmpty())
				inscription = QString("%1 %2 killed by %3 %4")
					.arg(Name()).arg(Id().StringId())
					.arg(killerName).arg(killerId.StringId());
			else
				inscription = QString("%1 %2")
					.arg(Name()).arg(Id().StringId());

			if(IsWild() && !killerId.IsNull())
			{
				DaWorldLL* llworld = FindParentWorldInEnv();
				if(llworld)
					llworld->IndigenousHelpedBy(killerId, false);	// Wild animal kills are good deeds but not contact.
			}

			// Create an item to encapsulate the head left behind.
			DhDNA dna(DNA3());
			AuItemDNA head(&dna, Chromatid(), true, false, Handedness(), GrowthStage(), AuHairStyle::Unkempt, AuHairStyle::Unshaven,
					CID_Head, BodyQ(), EmpireId());
			head.Inscribe(inscription);
			DropItem(head, CameraOffset());

			// Create an item to encapsulate the carcass left behind.
			// A fish will leave a CID_Fish behind unless it has a thick hide.
			// If a fish has a thick hide, it leaves a CID_Animal behind so skinning it will produce a CID_Leather commodity.
			// Skinning a CID_Fish only produces a CID_FishMeat commodity.
			AuItemDNA carcass(&dna, Chromatid(), true, false, Handedness(), GrowthStage(), AuHairStyle::Unkempt,
					AuHairStyle::Unshaven, CID_Animal, BodyQ(), EmpireId());
			if(dna.LivesInWater && !dna.LivesOnLand && dna.TopSkinThickness < 2 && dna.BottomSkinThickness < 2)
				carcass.SetCommodity(CID_Fish, BodyQ());				// Thin skinned fish.

			// Don't let too many identical meat piles appear in the same spot.
			if(Parent()->OkToDropMeat(Location(), carcass.CommodityId(), carcass.DNA()))
			{
				// Drop first item at my exact location.
				DropItem(carcass);

				int extraMeat = MaxHitPoints() / 100;					// One additional chunk of meat per 100 hit points.
				for(; extraMeat; extraMeat--)
				{
					// Pick a random location near me.
					AuPoint offsetVec(g_Au.Dice().Roll(1,9,-5), g_Au.Dice().Roll(1,9,-5), g_Au.Dice().Roll(1,9,-5));
					DropItem(carcass, offsetVec);
				}
			}
		}

		Kill();		// Remove the atom from the universe.
	}
}

void DaAnimal::EatTarget(DaAtom* target)
{
	Q_ASSERT(target);
	DaItem* item = dynamic_cast<DaItem*>(target);
	if(item)
	{
		// Target is an edible morsel on the ground. Consume it.
		int foodValue = item->FoodNutritionValue(EcoRole());		// Get its food nutrition value with each bite.
		if(foodValue == 0)
			qDebug("DaAnimal::ConsumeItem - %d %s %s consumed food - zero value: %s",
					Id().GetID(), (const char*)Name().toLatin1(), (const char*)Description().toLatin1(),
					(const char*)g_Au.CommodityName(item->CommodityId()).toLatin1());
		SetHungerLimitMax(CurrentHunger() + foodValue);			// Perhaps I will become satisfied and wander off.
		item->DegradeCondition();	// The item will vanish or perhaps turn into a bone when its condition is reduced to zero.
	}
	else
	{
		DaCreature* creature = dynamic_cast<DaCreature*>(target);
		if(creature)
		{
			// Target is a creature.
			if(creature->IsBodyDyingOrDead())
			{
				// The creature is dying so finish it off by biting.
				DhDNA dna(DNA3());
				int damage1 = ((int)dna.MouthSize + 1) * 5;
				// Overall size affects damage.
				int damageMod1 = (int)dna.OverallSize / 3;

				// Calculate its body location relative to me.
				AuPoint loc;
				const DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					AuMatrix4x4 targT;
					target->GetTransformRelativeToParent(targT, planet);
					loc = targT.Project(target->TargetLocation());				// Relative to planet.

					AuMatrix4x4 myT;
					GetTransformRelativeToParent(myT, planet);
					loc = myT.UnProject(loc);									// Relative to me
				}

				// Animals are not pirates.
				AuCombatAttacker attacker(Id(), Name(), EmpireId());
				if(IsInVehicle())
				{
					const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
					if(vehicle)
						attacker.m_Name = tr("%1 in %2").arg(Name()).arg(vehicle->Name());
				}
				else if(IsOnVehicle())
				{
					const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
					if(vehicle)
						attacker.m_Name = tr("%1 on %2").arg(Name()).arg(vehicle->Name());
				}
				else if(IsInSpacecraft())
				{
					const DaVessel* ship = FindParentVesselInEnv();
					if(ship)
						attacker.m_Name = tr("%1 aboard %2").arg(Name()).arg(ship->NameWithEmpire());
				}
				else if(IsInBd())
				{
					const DaBd* bd = FindParentBdInEnv();
					if(bd)
						attacker.m_Name = tr("%1 in %2").arg(Name()).arg(bd->CityBuildingName());
				}
				attacker.m_MuzzleLoc = HeadLocation();

				AuCombatDefender defender(target, loc, false, 0);

				AuCombatWeapon weapon(WM_Contact);
				weapon.m_CID = CID_Eaten;
				weapon.m_Range = 1;
				weapon.m_w1.m_Effect = WE_Pierce;
				weapon.m_w1.m_Damage = damage1;
				weapon.m_w1.m_DamageMod = damageMod1;

				AuCombatResult result;

				AttackTarget(attacker, defender, weapon, false, result);
			}
		}
	}
}

// Check fight or flight reflex to threat.
// May set m_IsAlert and m_Activity based on reaction to threat.
// May not do anything if this animal doesn't care.
// Health level of this animal affects the check. As health is reduced it is
// less inclined to attack and more inclined to flee.
void DaAnimal::FightOrFlightCheck(float threatRange, int reactMod)
{
	const int rangeMod = (int)(threatRange / 100);
	const int maxHits = MaxHitPoints();
	int healthMod;
	if(maxHits > 0)
		healthMod = CurrentHitPoints()*3/maxHits - 3;	// 0..-3 where 0 is most healthy and -3 is least healthy
	else
		healthMod = 0;

	GENDER gender = Gender();
	if(gender == Male || gender == Hermaphrodite)
	{
		// This is a male animal. Roll the attack reflex before the flee reflex.
		int fightReaction = g_Au.Dice().Roll(1,17, rangeMod -healthMod -reactMod -1);
		if(fightReaction <= FightReflex())
		{
			// This animal has become agitated to attack.
			// Get info about the weapon used at this range.
			AuDNABase::WEAPONLOC wLoc;
			AuCombatWeapon weapon(WM_Not);
			int ammoCur, ammoFull;
			BestWeaponAtRange(threatRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None)
			{
				SetActivity(Attacking);
				SetAlert(true);
			}
			else
			{
				SetActivity(Charging);
				SetAlert(true);
			}
		}
		else
		{
			// This animal has not decided to attack; does it flee
			// instead?
			int flightReaction = g_Au.Dice().Roll(1,17, rangeMod +healthMod -reactMod -1);
			if(flightReaction <= FlightReflex())
			{
				SetActivity(Fleeing);
				SetAlert(true);
			}
		}
	}
	else
	{
		// This is a female animal. Roll the flee reflex before the attack reflex.
		int flightReaction = g_Au.Dice().Roll(1,17, rangeMod +healthMod -reactMod -1);
		if(flightReaction <= FlightReflex())
		{
			SetActivity(Fleeing);
			SetAlert(true);
		}
		else
		{
			// This animal has not decided to flee; does it attack
			// instead?
			int fightReaction = g_Au.Dice().Roll(1,17, rangeMod -healthMod -reactMod -1);
			if(fightReaction <= FightReflex())
			{
				// This animal has become agitated to attack.
				// Get info about the weapon used at this range.
				AuDNABase::WEAPONLOC wLoc;
				AuCombatWeapon weapon(WM_Not);
				int ammoCur, ammoFull;
				BestWeaponAtRange(threatRange, wLoc, weapon, ammoCur, ammoFull);
				if(weapon.m_w1.m_Effect != WE_None)
				{
					SetActivity(Attacking);
					SetAlert(true);
				}
				else
				{
					SetActivity(Charging);
					SetAlert(true);
				}
			}
		}
	}
}

// Check flight reflex to danger.
// May set m_IsAlert and m_Activity based on reaction to threat.
// May not do anything if this animal doesn't care.
// Health level of this animal affects the check. As health is reduced it is more inclined to flee.
void DaAnimal::FlightCheck(float dangerRange)
{
	const int rangeMod = (int)(dangerRange / 100);
	const int maxHits = MaxHitPoints();
	int healthMod;
	if(maxHits > 0)
		healthMod = CurrentHitPoints()*3/maxHits - 3;	// 0..-3 where 0 is most healthy and -3 is least healthy
	else
		healthMod = 0;

	int flightReaction = g_Au.Dice().Roll(1,17, rangeMod +healthMod -1);
	if(flightReaction <= FlightReflex())
	{
		SetActivity(Fleeing);
		SetAlert(true);
	}
}

void DaAnimal::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaCreature::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	// When in terrain, only animate my pose if my terrain tile hasn't been culled from the scene. This relieves a lot of load.
	AnimatePose(false, 1);

	int msElapsed = lastAnim.elapsed();
	if(msElapsed <= 0)
		return;
	if(CanTurn() && TurnZ() != 0)
	{
		float rsec = (float)msElapsed/1000.f;	// Real world elapsed secs.
		float rMaxTurn = TurnRate() * rsec;		// Max turn in time period.

		float turnZNow;
		if(m_TurnZ > rMaxTurn)
		{
			turnZNow = rMaxTurn;
			m_TurnZ -= rMaxTurn;
		}
		else if(m_TurnZ < -rMaxTurn)
		{
			turnZNow = -rMaxTurn;
			m_TurnZ += rMaxTurn;
		}
		else
		{
			turnZNow = m_TurnZ;
			m_TurnZ = 0;
		}

		AuAtomPos tMe = Pos();
		tMe.RotateZ(turnZNow);
		ChangePosAndOrientNotParent(tMe);
		SetModified();
	}
}

void DaAnimal::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	Q_UNUSED(scannerLoc);
	Q_UNUSED(scannerSensorRange);
	Q_UNUSED(scannerWeaponRange);
	Q_UNUSED(lockedBlip);
	return;	// Scanners do not detect animals.
	// I am only scanned for AI turret gunners.
	if(!(scanContent & 0x0020))
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

	if(IsWild())
	{
		// Empires do not establish a political stance vs wild animals. Empire id 0 indicates wild animal.
		blips.Add(AuBlipAnimal(IsMounted(), Species(), 0, POL_Neutral, Id(), EnvLocation()));
	}
	else
	{
		// Domestic animals are subject to the political stance of the empire.
		POLITICAL_STANCE stance = PoliticalStance(scannerEmpire, EmpireId(), stanceCache);
		blips.Add(AuBlipAnimal(IsMounted(), Species(), EmpireId(), stance, Id(), EnvLocation()));
	}
}

// Insert first time database record.
void DaAnimal::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_Animal (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
			"timeCreate,timeModified,timeTock,timeDeath, name,allegiance,arenaTeam,conditions, "
			"hunger,seatIndex,hp,sex,dna,dna4,islefty,pose,whichConsole,atRest,vecX,vecY,vecZ,stamina, minsToFullGrown,bodyQ) "
			"VALUES (?,?,?,?,?,?,?,?,?,?,?,?, ?,?,?, ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, ?,?);");
	
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

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(CurrentBodyHitPoints());
	qa.addBindValue((Chromatid() == AuDNABase::Alpha) ? AuDNABase::Beta : AuDNABase::Alpha);	// reversed for old gender values
	{
		// Clear old dna field.
		QByteArray ba;
		qa.addBindValue(ba);

		// Nu dna.
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		AuDNABase* dna = DNAnew();
		dna->WriteDynamic(out);
		delete dna;
		qa.addBindValue(ba);
	}
	qa.addBindValue(IsLeftHanded());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);
	qa.addBindValue(m_MinutesToFullGrown);
	qa.addBindValue(m_BodyQ);

	// DaAnimal values.

	if(!qa.exec())
		qWarning("DaAnimal::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaAnimal::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Animal SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
			"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, name=?, "
			"allegiance=?, arenaTeam=?, conditions=?, hunger=?, seatIndex=?, hp=?, pose=?, whichConsole=?, atRest=?, "
			"vecX=?, vecY=?, vecZ=?, "
			"stamina=?, minsToFullGrown=?, bodyQ=? WHERE id=%2;").arg(Id().GetID());
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

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(CurrentBodyHitPoints());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);	
	qa.addBindValue(m_MinutesToFullGrown);
	qa.addBindValue(m_BodyQ);

	// DaAnimal values.

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaAnimal::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaAnimal::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaAnimal* DaAnimal::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, name,allegiance,arenaTeam,conditions, hunger,seatIndex,hp,sex,dna,dna4,"
			"islefty,pose,whichConsole,atRest,vecX,vecY,vecZ,stamina,minsToFullGrown,bodyQ FROM a_Animal "
			"WHERE ");
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
		DaAnimal *pnu = new DaAnimal;
		if(pnu)
		{
			int qv = 0;	// query value index
			// DaAtom values.
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

			// DaUnit values.
			pnu->SetName(qa.value(qv++).toString());
			pnu->SetAllegiance(qa.value(qv++).toUInt());
			pnu->SetArenaTeam(qa.value(qv++).toUInt());
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Conditions
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->LockConditionsForWrite();
				pnu->UnlockConditions();
			}

			// DaCreature values.
			pnu->SetHunger(qa.value(qv++).toInt());
			pnu->m_SeatIndex = qa.value(qv++).toInt();
			pnu->SetHitPoints(qa.value(qv++).toInt());
			pnu->SetChromatid((AuDNABase::CHROMATID)qa.value(qv++).toInt() ? AuDNABase::Alpha : AuDNABase::Beta);	// Was gender
			{
				// The nu style DNA takes precedence over the old style.
				QByteArray baol = qa.value(qv++).toByteArray();
				QByteArray banu = qa.value(qv++).toByteArray();
				if(banu.isEmpty())
					pnu->SetDNA(DhDNA(baol));	// Nu is empty so assume old is ok.
				else
				{
					// Nu is not empty so use it.
					QDataStream in(banu);
					in.setVersion(QDataStream::Qt_5_7);
					AuDNABase* dna = AuDNABase::ReadDynamic(in);
					if(dna)
					{
						pnu->SetDNA(*dna);
						delete dna;
					}
				}
			}
			pnu->SetLeftHanded(qa.value(qv++).toBool());
			AuPose::POSE curPose = (AuPose::POSE)qa.value(qv++).toInt();
			pnu->SetGeneralPose(curPose, false);
			pnu->m_ControlId = qa.value(qv++).toUInt();
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_Stamina = qa.value(qv++).toInt();
			pnu->SetMinutesToFullGrown(qa.value(qv++).toUInt());	// Updates my growth stage in animalia
			pnu->m_BodyQ = qa.value(qv++).toUInt();
			if(pnu->m_BodyQ == 0)
				pnu->m_BodyQ = 128;

			// DaAnimal values

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

// Look around and perceive things that matter to this animal's life strategy
// using eyes, ears, and nose.
// 'food' is nearest edible non-creature in sight
// 'prey' is nearest creature in sight that can be eaten if killed
// 'threat' is nearest threat in sight
// Distances will be -1 for things that are not seen. i.e. foodRange = -1 when
// no food is in sight.
// Vectors returned are relative to this atom.
// Vehicles may be identified as threats or prey.
void DaAnimal::Look(DaAtom*& foodAtom, AuPoint& foodDir, float& foodRange, DaAtom*& preyAtom, AuPoint& preyDir, float& preyRange,
		DaAtom*& threatAtom, AuPoint& threatDir, float& threatRange, DaAtom*& dangerAtom, AuPoint& dangerDir, float& dangerRange)
{
	//qDebug("DaAnimal::Look %d %s looking", Id().GetID(),
			//(const char*)Description());
	foodAtom = 0;
	foodRange = 0;
	preyAtom = 0;
	preyRange = 0;
	threatAtom = 0;
	threatRange = 0;
	dangerAtom = 0;
	dangerRange = 0;
	DhDNA dna(DNA3());
	if(IsInCapsule())
	{
		//qDebug("DaAnimal::Look - %d Aboard ship. Can't see.", Id().GetID());
		// Animal vision on board ship.
		DaCapsule* capsule = FindParentCapsuleInEnv();
		if(capsule)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, capsule);
			AuPoint eyeLoc = CameraOffset();
			AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Based on eye size, scan the terrain in this animal's field of vision.
			float earRange = dna.HearingRange();	// Range of ears.
			if(IsEyeSubmerged())
				earRange *= 2.f;
			else if(!IsUnderGround() && !AtmosphereDensity())
				earRange = 0;
			float noseRange = dna.SmellRange();	// Range of nose.
			if(IsEyeSubmerged())
				noseRange /= 2.f;
			else if(IsUnderGround())
				noseRange = 0;
			else if(!AtmosphereDensity())
				noseRange = 0;
			float eyeRange = dna.SightRange();	// Range of eyes.
			if(IsUnderGround())
				eyeRange = 0;
			else if(IsEyeSubmerged())
				eyeRange /= 3.f;
			if(IsAlert())
			{
				// Increase range of senses when alert.
				eyeRange *= 1.2f;
				earRange *= 1.15f;
				noseRange *= 1.1f;
			}
			float eyeAngle = dna.FieldOfViewAngle();	// Field of view angle is 1/2 field of view.

			// My size is a factor when sizing up other creatures.
			float myVolume = 0;	// Initialized when needed.

			// Scan the interior of the spacecraft.
			DaAtomList subs;
			capsule->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom == this)
					continue;	// skip myself
				if(!pAtom->DoesQueryEnvironment())
					continue;

				DaCreature* creature = dynamic_cast<DaCreature*>(pAtom);
				if(creature)
				{
					// A creature is present; could be a citizen, person, or animal.
					{
						DaPerson* person = dynamic_cast<DaPerson*>(creature);
						if(person)
						{
							// Ignore people that are not sightable as
							// a target. i.e. waiting to spawn elsewhere or not on line
							if(!person->IsSightTarget(this))
								continue;
						}
						else if(creature->HasDNA3() && creature->DNA3() == dna)
						{
							//qDebug("DaAnimal::Look - %d ignoring "
									//"other creature %d of my own "
									//"kind", Id().GetID(),
									//creature->Id().GetID());
							if(creature->IsAlert())
							{
								// The other creature of my kind is
								// alert so I might become alert.
								if(g_Au.Dice().Roll(1,100) > 95)
									SetAlert(true);
							}
							continue;	// Ignore others of my own kind.
						}
					}

					// Make sure I can enter the creature's environment.
					if(creature->IsUnderGround())
					{
						if(!dna.LivesUnderGround || IsFlying())
							continue;	// I can't go underground.
					}
					else if(creature->IsInVisibleWater())
					{
						if(!dna.LivesInWater)
							continue;	// I can't go in the water.
					}
					else if(creature->IsFlying())
					{
						if(!dna.CanFly())
							continue;	// I can't fly.
					}
					else
					{
						// Other creature is on the ground.
						if(!dna.LivesInAir && !dna.LivesOnLand)
							continue;	// I can't go on land.
					}

					AuMatrix4x4 targT;
					pAtom->GetTransformRelativeToParent(targT, capsule);
					AuPoint loc = targT.Project(pAtom->TargetLocation());		// Relative to capsule.
					loc = myT.UnProject(loc);									// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist == 0)
						sightVec = AuPoint(0,1,0);	// In my eye?
					else
						sightVec /= sightDist;	// Make unit vector.
					bool smellIt = false;
					if(sightDist < noseRange)
					{
						if(creature->IsUnderGround())
							smellIt = false;	// under ground.
						else if(creature->IsCGSubmerged() &&
								IsEyeSubmerged())
							smellIt = true;		// Both under water.
						else if(!creature->IsEyeSubmerged() &&
								!IsEyeSubmerged())
							smellIt = true;	// Both not under water.
					}
					bool seeIt = false;
					if(sightDist < eyeRange)
					{
						// Restrict sight test to field of vision
						// only when not alert.
						// Otherwise this creature will stop
						// noticing a threat after it turns to run
						// away.
						if(creature->IsUnderGround())
							seeIt = false;	// It is under ground.
						else
						{
							if(IsAlert())
								seeIt = true;
							else
							{
								float sightAngle =
									eyeVec.AngleTo(sightVec);
								if(sightAngle <= eyeAngle)
									seeIt = true;	// Inside F.O.V.
							}
						}
					}

					// Noise level of creature affects perception
					// range with ears.
					bool hearIt = false;
					float noise = creature->NoiseLevel();
					if(sightDist < earRange * noise)
						hearIt = true;
					if(!seeIt && !smellIt && !hearIt)
						continue;	// Out of range to see, smell
									// or hear.

					// My size is a factor when sizing up other
					// creatures.
					if(myVolume == 0.)
						myVolume = Volume();

					// This animal "sees" the other creature.
					// Relative size and ecological roles are the
					// primary factors in this animal's perception
					// of the other.
					bool isOtherDying = (creature->CurrentHitPoints() <= 0) ? true : false;
					bool isFood = false;	// other can be eaten
					bool isPrey = false;	// other can be killed for food
					bool isThreat = false;	// other makes me nervous
					float otherVolume = creature->Volume();
					switch(dna.EcoRole())
					{
					case ECO_Carnivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else
								isPrey = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume*3.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume*4.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						}
						break;
					case ECO_Herbivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(!isOtherDying)
								isThreat = true;
							break;
						case ECO_Herbivore:
							break;
						case ECO_Omnivore:
							if(!isOtherDying)
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(!isOtherDying &&
									otherVolume > myVolume/3.f)
								isThreat = true;
							break;
						}
						break;
					case ECO_Omnivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume/2.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else
								isPrey = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(!isOtherDying &&
									otherVolume > myVolume/2.f)
								isThreat = true;
							break;
						}
						break;
					case ECO_Scavenger:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume*2.f)
								isThreat = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume/2.f)
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume)
								isThreat = true;
							break;
						}
						break;
					}
					if(isFood)
					{
						if(foodRange == 0 || sightDist < foodRange)
						{
							// The creature is the nearest food.
							foodAtom = pAtom;
							foodRange = loc.Length();
							if(foodRange)
								foodDir = loc/foodRange;
							else
								foodDir = AuPoint(0,1,0);
						}
					}
					if(isPrey)
					{
						if(preyRange == 0 || sightDist < preyRange)
						{
							// The creature is the nearest prey.
							preyAtom = pAtom;
							preyRange = loc.Length();
							if(preyRange)
								preyDir = loc/preyRange;
							else
								preyDir = AuPoint(0,1,0);
						}
					}
					if(isThreat)
					{
						if(threatRange == 0 ||
								sightDist < threatRange)
						{
							// The creature is the nearest threat.
							threatAtom = pAtom;
							threatRange = loc.Length();
							if(threatRange)
								threatDir = loc/threatRange;
							else
								threatDir = AuPoint(0,1,0);
						}
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(pAtom);
					if(vehicle)
					{
						// A vehicle is present; could be considered a threat or prey.
						// Make sure I can enter the creature's environment.
						if(vehicle->IsUnderGround())
						{
							if(!dna.LivesUnderGround || IsFlying())
								continue;	// I can't go underground.
						}
						else if(vehicle->IsInVisibleWater())
						{
							if(!dna.LivesInWater)
								continue;	// I can't go in the water.
						}
						else if(vehicle->IsFlying())
						{
							if(!dna.CanFly())
								continue;	// I can't fly.
						}
						else
						{
							// Other creature is on the ground.
							if(!dna.LivesInAir && !dna.LivesOnLand)
								continue;	// I can't go on land.
						}

						// People aboard check is a little more work 
						// than the previous easy stuff.
						const DaPerson* person = vehicle->FindPersonOnLineRecursive();
						if(!person)
							continue;	// Nobody on board so ignore it.
						if(!person->IsSightTarget(this))
							continue;

						// Target a person sighted aboard open vehicle.
						// Target the vehicle when it is enclosed.
						DaAtom* targetVeh;
						if(vehicle->IsEnclosed())
							targetVeh = vehicle;			// vehicle
						else
							targetVeh = (DaAtom*)person;	// passenger

						AuMatrix4x4 targT;
						targetVeh->GetTransformRelativeToParent(targT, capsule);
						AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to capsule.
						loc = myT.UnProject(loc);								// Relative to me
						AuPoint sightVec = loc - eyeLoc;
						float sightDist = sightVec.Length();
						if(sightDist == 0)
							sightVec = AuPoint(0,1,0);	// In my eye?
						else
							sightVec /= sightDist;	// Make unit vector.
						bool smellIt = false;
						if(sightDist < noseRange)
						{
							if(vehicle->IsUnderGround())
								smellIt = false;	// under ground
							else if(vehicle->IsCGSubmerged() &&
									IsEyeSubmerged())
								smellIt = true;		// Both under water.
							else if(!vehicle->IsEyeSubmerged() &&
									!IsEyeSubmerged())
								smellIt = true;	// Both not under water.
						}
						bool seeIt = false;
						if(sightDist < eyeRange)
						{
							// Restrict sight test to field of vision
							// only when not alert.
							// Otherwise this creature will stop
							// noticing a threat after it turns to run
							// away.
							if(vehicle->IsUnderGround())
								seeIt = false;	// It is under ground.
							else
							{
								if(IsAlert())
									seeIt = true;
								else
								{
									float sightAngle =
										eyeVec.AngleTo(sightVec);
									if(sightAngle <= eyeAngle)
										seeIt = true;	// In F.O.V.
								}
							}
						}

						// Noise level of vehicle affects perception
						// range with ears.
						bool hearIt = false;
						float noise = vehicle->NoiseLevel();
						if(sightDist < earRange * noise)
							hearIt = true;
						if(!seeIt && !smellIt && !hearIt)
							continue;	// Out of range to see, smell or hear.

						// My size is a factor when sizing up other
						// creatures.
						if(myVolume == 0)
							myVolume = Volume();

						// This animal "sees" the vehicle or the
						// passenger of open vehicles.
						// Relative size and ecological roles are the
						// primary factors in this animal's perception
						// of the target. Since the vehicle isn't an
						// animal, this animal will treat it as a
						// carnivore, even if it is targeting a
						// passenger.
						bool isPrey = false;	// target can be killed for food
						bool isThreat = false;	// target makes me nervous
						float otherVolume = targetVeh->Mass() / targetVeh->Density();
						switch(dna.EcoRole())
						{
						case ECO_Carnivore:
							if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							isThreat = true;
							break;
						case ECO_Omnivore:
							if(otherVolume < myVolume/2.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							isThreat = true;
							break;
						}
						if(isPrey)
						{
							if(preyRange == 0 || sightDist < preyRange)
							{
								// The vehicle or passenger is the
								// nearest prey.
								preyAtom = targetVeh;
								preyRange = loc.Length();
								if(preyRange)
									preyDir = loc/preyRange;
								else
									preyDir = AuPoint(0,1,0);
							}
						}
						if(isThreat)
						{
							if(threatRange == 0 ||
									sightDist < threatRange)
							{
								// The vehicle or passenger is the
								// nearest threat.
								threatAtom = targetVeh;
								threatRange = loc.Length();
								if(threatRange)
									threatDir = loc/threatRange;
								else
									threatDir = AuPoint(0,1,0);
							}
						}
					}
					else
					{
						// If an item is present, it might be recognized as food. This animal "sees" the item.
						// Some items are seen as a threat, such as roaring fires and tepees.
						const DaItem* item = dynamic_cast<const DaItem*>(pAtom);
						if(item)
						{
							if(!item->IsStaticDisplay())
							{
								// Ignore static display items.
							}
							else if(dna.EcoRole() == ECO_Carnivore || dna.EcoRole() == ECO_Omnivore)
							{
								// Carnivorous and omnivorous wild animals inside capsules are monsters. They are not distracted by
								// food items. Domestic animals do not call this function.
							}
							else if(item->FoodNutritionValue(dna.EcoRole()) ||			// Item is food
									(dna.EcoRole() == ECO_Herbivore &&			// I am a herbivore and
									 DbCommodity::IsFlowerPot(item->CommodityId()) &&	// item is a flower pot and
									 item->DNAIsViable()))								// a plant is growing in the pot, nom.
							{
								// The item is food to me.
								AuMatrix4x4 targT;
								pAtom->GetTransformRelativeToParent(targT, capsule);
								AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to capsule.
								loc = myT.UnProject(loc);	// Rel. to me
								AuPoint sightVec = loc - eyeLoc;
								float sightDist = sightVec.Length();
								if(sightDist == 0)
									sightVec = AuPoint(0,1,0);	// In my eye?
								else
									sightVec /= sightDist;	// Make a unit vector.
								bool smellIt = false;
								if(sightDist < noseRange)
									smellIt = true;
								bool seeIt = false;
								if(sightDist < eyeRange)
								{
									// Restrict sight test to field of vision only
									// when not alert.
									// Otherwise this creature might stop noticing
									// food dropped by killing prey.
									if(IsAlert())
										seeIt = true;
									else
									{
										float sightAngle =eyeVec.AngleTo(sightVec);
										if(sightAngle <= eyeAngle)
											seeIt = true;	// Inside field of view.
									}
								}
								if(!seeIt && !smellIt)
									continue;	// Out of range to see or smell.
								if(foodRange == 0. || sightDist < foodRange)
								{
									// The item is the nearest food.
									foodAtom = pAtom;
									foodRange = loc.Length();
									if(foodRange)
										foodDir = loc/foodRange;
									else
										foodDir = AuPoint(0,1,0);
								}
							}
							else if(item->IsAvoidedByWildAnimals())
							{
								// The item is a tent or fire, viewed as a danger.
								AuMatrix4x4 targT;
								pAtom->GetTransformRelativeToParent(targT, capsule);
								AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to capsule.
								loc = myT.UnProject(loc);								// Relative to me.
								AuPoint sightVec = loc - eyeLoc;
								float sightDist = sightVec.Length();
								if(sightDist == 0)
									sightVec = AuPoint(0,1,0);	// In my eye?
								else
									sightVec /= sightDist;	// Make a unit vector.
								bool smellIt = false;
								if(sightDist < noseRange)
									smellIt = true;
								bool seeIt = false;
								if(sightDist < eyeRange)
								{
									// Restrict sight test to field of vision only
									// when not alert.
									// Otherwise this creature might stop noticing
									// food dropped by killing prey.
									if(IsAlert())
										seeIt = true;
									else
									{
										float sightAngle =eyeVec.AngleTo(sightVec);
										if(sightAngle <= eyeAngle)
											seeIt = true;	// Inside field of view.
									}
								}
								if(!seeIt && !smellIt)
									continue;	// Out of range to see or smell.
								if(dangerRange == 0 || sightDist < dangerRange)
								{
									// The item is the nearest danger.
									dangerAtom = pAtom;
									dangerRange = loc.Length();
									if(dangerRange)
										dangerDir = loc/dangerRange;
									else
										dangerDir = AuPoint(0,1,0);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld)
		{
			AuMatrix4x4 myT;
			GetTransformRelativeToParent(myT, llworld);
			AuPoint eyeLoc = CameraOffset();
			AuPoint eyeVec = AuPoint(0,1,0).RotateX(CurrentHeadTilt());

			// Based on eye size, scan the terrain in this animal's field of vision.
			float earRange = dna.HearingRange();	// Range of ears.
			if(IsEyeSubmerged())
				earRange *= 2.f;
			else if(!IsUnderGround() && !AtmosphereDensity())
				earRange = 0;
			float noseRange = dna.SmellRange();	// Range of nose.
			if(IsEyeSubmerged())
				noseRange /= 2.f;
			else if(IsUnderGround())
				noseRange = 0;
			else if(!AtmosphereDensity())
				noseRange = 0;
			float eyeRange = dna.SightRange();	// Range of eyes.
			if(IsUnderGround())
				eyeRange = 0;
			else if(IsEyeSubmerged())
				eyeRange /= 3.f;
			if(IsAlert())
			{
				// Increase range of senses when alert.
				eyeRange *= 1.2f;
				earRange *= 1.15f;
				noseRange *= 1.1f;
			}
			float eyeAngle = dna.FieldOfViewAngle();	// Field of view angle is 1/2 field of view.

			// My size is a factor when sizing up other creatures.
			float myVolume = 0;	// Initialized when needed.

			// Things in the area of fire pits and tents are avoided. We don't go get food there and we notice threats there.
			AuPointArray avoidLocs;
			bool avoidLocsValid = false;

			// Scan an area centered on this animal.
			DaAtomList subs;
			llworld->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(pAtom == this)
					continue;	// skip myself
				if(!pAtom->IsNormal())
					continue;
				if(!pAtom->DoesQueryEnvironment())
					continue;

				bool nearAvoidLoc = false;	// True if pAtom is near a fire pit or tent.
				double closestDist = 0;		// Distance to avoid location if near one.
				const DaItem* item = dynamic_cast<const DaItem*>(pAtom);
				if(!item || !item->IsThreatToWildAnimals())
				{
					// Ignore non-threatening things near tepees and fire pits.
					// Not an item or not an item that is viewed as a threat, such as a fire. Threats are handled a bit later.
					if(!avoidLocsValid)
					{
						for(DaAtomList::const_iterator tfi = subs.cbegin(); tfi != subs.cend(); tfi++)
						{
							const DaItem* item = dynamic_cast<const DaItem*>(*tfi);
							if(!item)
								continue;
							if(item->IsAvoidedByWildAnimals())
								avoidLocs.append(item->Location());
						}
						avoidLocsValid = true;
					}
					if(!avoidLocs.isEmpty())
					{
						// Avoid the atom if it is within 20m of the center of a tent or fire.
						if(avoidLocs.FindClosest(pAtom->Location(), closestDist) >= 0 && closestDist < 20*FeetPerMeter())
							nearAvoidLoc = true;
					}
				}

				DaCreature* creature = dynamic_cast<DaCreature*>(pAtom);
				if(creature)
				{
					// A creature is present; could be a citizen, person, or animal.
					DaCitizen* citizen = dynamic_cast<DaCitizen*>(creature);
					if(citizen && !citizen->IsTargetToWildAnimal())
						continue;	// Ignore workers.

					DaPerson* person = dynamic_cast<DaPerson*>(creature);
					if(person)
					{
						// Ignore people that are not sightable as
						// a target. i.e. waiting to spawn elsewhere or not on line
						if(!person->IsSightTarget(this))
							continue;
						if(person->InIndustry() == IID_TownSquare)
							continue;				// Town square is safe from wild animals.
					}
					else if(creature->HasDNA3() && creature->DNA3() == dna)
					{
						//qDebug("DaAnimal::Look - %d ignoring "
								//"other creature %d of my own "
								//"kind", Id().GetID(),
								//creature->Id().GetID());
						if(creature->IsAlert())
						{
							// The other creature of my kind is
							// alert so I might become alert.
							if(g_Au.Dice().Roll(1,100) > 95)
								SetAlert(true);
						}
						continue;	// Ignore others of my own kind.
					}

					// Make sure I can enter the creature's environment.
					if(creature->IsUnderGround())
					{
						if(!dna.LivesUnderGround || IsFlying())
							continue;	// I can't go underground.
					}
					else if(creature->IsInVisibleWater())
					{
						if(!dna.LivesInWater)
							continue;	// I can't go in the water.
					}
					else if(creature->IsFlying())
					{
						if(!dna.CanFly())
							continue;	// I can't fly.
					}
					else
					{
						// Other creature is on the ground.
						if(!dna.LivesInAir && !dna.LivesOnLand)
							continue;	// I can't go on land.
					}

					const AuMatrix4x4& targT = pAtom->Transform();				// atom is directly subordinate to the parent planet
					AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
					loc = myT.UnProject(loc);								// Relative to me
					AuPoint sightVec = loc - eyeLoc;
					float sightDist = sightVec.Length();
					if(sightDist == 0)
						sightVec = AuPoint(0,1,0);	// In my eye?
					else
						sightVec /= sightDist;	// Make unit vector.
					bool smellIt = false;
					if(sightDist < noseRange)
					{
						if(creature->IsUnderGround())
							smellIt = false;	// under ground.
						else if(creature->IsCGSubmerged() &&
								IsEyeSubmerged())
							smellIt = true;		// Both under water.
						else if(!creature->IsEyeSubmerged() &&
								!IsEyeSubmerged())
							smellIt = true;	// Both not under water.
					}
					bool seeIt = false;
					if(sightDist < eyeRange)
					{
						// Restrict sight test to field of vision
						// only when not alert.
						// Otherwise this creature will stop
						// noticing a threat after it turns to run
						// away.
						if(creature->IsUnderGround())
							seeIt = false;	// It is under ground.
						else
						{
							if(IsAlert())
								seeIt = true;
							else
							{
								float sightAngle = eyeVec.AngleTo(sightVec);
								if(sightAngle <= eyeAngle)
									seeIt = true;	// Inside F.O.V.
							}
						}
					}

					// Noise level of creature affects perception
					// range with ears.
					bool hearIt = false;
					float noise = creature->NoiseLevel();
					if(sightDist < earRange * noise)
						hearIt = true;
					if(!seeIt && !smellIt && !hearIt)
						continue;	// Out of range to see, smell or hear.

					// My size is a factor when sizing up other creatures.
					if(myVolume == 0.)
						myVolume = Volume();

					// This animal "sees" the other creature.
					// Relative size and ecological roles are the
					// primary factors in this animal's perception
					// of the other.
					bool isOtherDying = (creature->CurrentHitPoints() <= 0) ? true : false;
					bool isFood = false;	// other can be eaten
					bool isPrey = false;	// other can be killed for food
					bool isThreat = false;	// other makes me nervous
					float otherVolume = creature->Volume();
					switch(dna.EcoRole())
					{
					case ECO_Carnivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else
								isPrey = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume*3.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume*4.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						}
						break;
					case ECO_Herbivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(!isOtherDying)
								isThreat = true;
							break;
						case ECO_Herbivore:
							break;
						case ECO_Omnivore:
							if(!isOtherDying)
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(!isOtherDying &&
									otherVolume > myVolume/3.f)
								isThreat = true;
							break;
						}
						break;
					case ECO_Omnivore:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume/2.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else
								isPrey = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(!isOtherDying &&
									otherVolume > myVolume/2.f)
								isThreat = true;
							break;
						}
						break;
					case ECO_Scavenger:
						switch(creature->EcoRole())
						{
						case ECO_Carnivore:
							if(isOtherDying)
								isFood = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume*2.f)
								isThreat = true;
							break;
						case ECO_Omnivore:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume/2.f)
								isThreat = true;
							break;
						case ECO_Scavenger:
							if(isOtherDying)
								isFood = true;
							else if(otherVolume > myVolume)
								isThreat = true;
							break;
						}
						break;
					}
					if(isFood)
					{
						if(nearAvoidLoc && closestDist < 8)
							continue;		// Ignore food in tents and fire pits.
						if(foodRange == 0 || sightDist < foodRange)
						{
							// The creature is the nearest food.
							foodAtom = pAtom;
							foodRange = loc.Length();
							if(foodRange)
								foodDir = loc/foodRange;
							else
								foodDir = AuPoint(0,1,0);
						}
					}
					if(isPrey)
					{
						if(nearAvoidLoc)
							continue;		// Ignore prey near tents and fire pits.
						if(preyRange == 0 || sightDist < preyRange)
						{
							// The creature is the nearest prey.
							preyAtom = pAtom;
							preyRange = loc.Length();
							if(preyRange)
								preyDir = loc/preyRange;
							else
								preyDir = AuPoint(0,1,0);
						}
					}
					if(isThreat)
					{
						if(nearAvoidLoc)
							continue;		// Ignore threats near tents and fire pits.
						if(threatRange == 0 || sightDist < threatRange)
						{
							// The creature is the nearest threat.
							threatAtom = pAtom;
							threatRange = loc.Length();
							if(threatRange)
								threatDir = loc/threatRange;
							else
								threatDir = AuPoint(0,1,0);
						}
					}
				}
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(pAtom);
					if(vehicle)
					{
						// A vehicle is present; could be considered a
						// threat or prey.

						// Make sure I can enter the creature's
						// environment.
						if(vehicle->IsUnderGround())
						{
							if(!dna.LivesUnderGround || IsFlying())
								continue;	// I can't go underground.
						}
						else if(vehicle->IsInVisibleWater())
						{
							if(!dna.LivesInWater)
								continue;	// I can't go in the water.
						}
						else if(vehicle->IsFlying())
						{
							if(!dna.CanFly())
								continue;	// I can't fly.
						}
						else
						{
							// Other creature is on the ground.
							if(!dna.LivesInAir && !dna.LivesOnLand)
								continue;	// I can't go on land.
						}

						// People aboard check is a little more work 
						// than the previous easy stuff.
						const DaPerson* person = vehicle->FindPersonOnLineRecursive();
						if(!person)
							continue;	// Nobody on board so ignore it.
						if(!person->IsSightTarget(this))
							continue;

						// Target a person sighted aboard open vehicle.
						// Target the vehicle when it is enclosed.
						DaAtom* targetVeh;
						if(vehicle->IsEnclosed())
							targetVeh = vehicle;			// vehicle
						else
							targetVeh = (DaAtom*)person;	// passenger

						const AuMatrix4x4& targT = pAtom->Transform();			// atom is directly subordinate to the parent planet
						AuPoint loc = targT.Project(targetVeh->TargetLocation());	// Relative to planet.
						loc = myT.UnProject(loc);									// Relative to me
						AuPoint sightVec = loc - eyeLoc;
						float sightDist = sightVec.Length();
						if(sightDist == 0)
							sightVec = AuPoint(0,1,0);	// In my eye?
						else
							sightVec /= sightDist;	// Make unit vector.
						bool smellIt = false;
						if(sightDist < noseRange)
						{
							if(vehicle->IsUnderGround())
								smellIt = false;	// under ground
							else if(vehicle->IsCGSubmerged() &&
									IsEyeSubmerged())
								smellIt = true;		// Both under water.
							else if(!vehicle->IsEyeSubmerged() &&
									!IsEyeSubmerged())
								smellIt = true;	// Both not under water.
						}
						bool seeIt = false;
						if(sightDist < eyeRange)
						{
							// Restrict sight test to field of vision
							// only when not alert.
							// Otherwise this creature will stop
							// noticing a threat after it turns to run
							// away.
							if(vehicle->IsUnderGround())
								seeIt = false;	// It is under ground.
							else
							{
								if(IsAlert())
									seeIt = true;
								else
								{
									float sightAngle =
										eyeVec.AngleTo(sightVec);
									if(sightAngle <= eyeAngle)
										seeIt = true;	// In F.O.V.
								}
							}
						}

						// Noise level of vehicle affects perception
						// range with ears.
						bool hearIt = false;
						float noise = vehicle->NoiseLevel();
						if(sightDist < earRange * noise)
							hearIt = true;
						if(!seeIt && !smellIt && !hearIt)
							continue;	// Out of range to see, smell
										// or hear.

						// My size is a factor when sizing up other
						// creatures.
						if(myVolume == 0)
							myVolume = Volume();

						// This animal "sees" the vehicle or the
						// passenger of open vehicles.
						// Relative size and ecological roles are the
						// primary factors in this animal's perception
						// of the target. Since the vehicle isn't an
						// animal, this animal will treat it as a
						// carnivore, even if it is targeting a
						// passenger.
						bool isPrey = false;	// target can be killed for food
						bool isThreat = false;	// target makes me nervous
						float otherVolume = targetVeh->Mass() / targetVeh->Density();
						switch(dna.EcoRole())
						{
						case ECO_Carnivore:
							if(otherVolume < myVolume)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Herbivore:
							isThreat = true;
							break;
						case ECO_Omnivore:
							if(otherVolume < myVolume/2.f)
								isPrey = true;
							else
								isThreat = true;
							break;
						case ECO_Scavenger:
							isThreat = true;
							break;
						}
						if(isPrey)
						{
							if(nearAvoidLoc)
								continue;		// Ignore prey near tents and fire pits.
							if(preyRange == 0 || sightDist < preyRange)
							{
								// The vehicle or passenger is the
								// nearest prey.
								preyAtom = targetVeh;
								preyRange = loc.Length();
								if(preyRange)
									preyDir = loc/preyRange;
								else
									preyDir = AuPoint(0,1,0);
							}
						}
						if(isThreat)
						{
							if(nearAvoidLoc)
								continue;		// Ignore threats near tents and fire pits.
							if(threatRange == 0 || sightDist < threatRange)
							{
								// The vehicle or passenger is the
								// nearest threat.
								threatAtom = targetVeh;
								threatRange = loc.Length();
								if(threatRange)
									threatDir = loc/threatRange;
								else
									threatDir = AuPoint(0,1,0);
							}
						}
					}
					else if(item)
					{
						// If an item is present, it might be recognized as food. This animal "sees" the item.
						// Some items are seen as a threat, such as roaring fires and tepees.
						if(!item->IsStaticDisplay())
						{
							// Ignore static display items.
						}
						else if(item->FoodNutritionValue(dna.EcoRole()))
						{
							if(nearAvoidLoc && closestDist < 8)
								continue;		// Ignore food in tents and fire pits.
							const AuMatrix4x4& targT = pAtom->Transform();	// atom is directly subordinate to the parent planet
							AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
							loc = myT.UnProject(loc);								// Relative to me.
							AuPoint sightVec = loc - eyeLoc;
							float sightDist = sightVec.Length();
							if(sightDist == 0)
								sightVec = AuPoint(0,1,0);	// In my eye?
							else
								sightVec /= sightDist;	// Make a unit vector.
							bool smellIt = false;
							if(sightDist < noseRange)
								smellIt = true;
							bool seeIt = false;
							if(sightDist < eyeRange)
							{
								// Restrict sight test to field of vision only
								// when not alert.
								// Otherwise this creature might stop noticing
								// food dropped by killing prey.
								if(IsAlert())
									seeIt = true;
								else
								{
									float sightAngle =eyeVec.AngleTo(sightVec);
									if(sightAngle <= eyeAngle)
										seeIt = true;	// Inside field of view.
								}
							}
							if(!seeIt && !smellIt)
								continue;	// Out of range to see or smell.
							if(foodRange == 0. || sightDist < foodRange)
							{
								// The item is the nearest food.
								foodAtom = pAtom;
								foodRange = loc.Length();
								if(foodRange)
									foodDir = loc/foodRange;
								else
									foodDir = AuPoint(0,1,0);
							}
						}
						else if(item->IsAvoidedByWildAnimals())
						{
							// The item is a tent or fire, viewed as a danger.
							const AuMatrix4x4& targT = pAtom->Transform();	// atom is directly subordinate to the parent planet
							AuPoint loc = targT.Project(pAtom->TargetLocation());	// Relative to planet.
							loc = myT.UnProject(loc);								// Relative to me.
							AuPoint sightVec = loc - eyeLoc;
							float sightDist = sightVec.Length();
							if(sightDist == 0)
								sightVec = AuPoint(0,1,0);	// In my eye?
							else
								sightVec /= sightDist;	// Make a unit vector.
							bool smellIt = false;
							if(sightDist < noseRange)
								smellIt = true;
							bool seeIt = false;
							if(sightDist < eyeRange)
							{
								// Restrict sight test to field of vision only
								// when not alert.
								// Otherwise this creature might stop noticing
								// food dropped by killing prey.
								if(IsAlert())
									seeIt = true;
								else
								{
									float sightAngle =eyeVec.AngleTo(sightVec);
									if(sightAngle <= eyeAngle)
										seeIt = true;	// Inside field of view.
								}
							}
							if(!seeIt && !smellIt)
								continue;	// Out of range to see or smell.
							if(dangerRange == 0 || sightDist < dangerRange)
							{
								// The item is the nearest danger.
								dangerAtom = pAtom;
								dangerRange = loc.Length();
								if(dangerRange)
									dangerDir = loc/dangerRange;
								else
									dangerDir = AuPoint(0,1,0);
							}
						}
					}
				}
			}
		}
		//else
		//{
			//qDebug("DaAnimal::Look - %d Not in terrain. Can't see.", Id().GetID());
		//}
	}
	//qDebug("DaAnimal::Look - foodRange %g preyRange %g threatRange %g",
			//foodRange, preyRange, threatRange);
}

void DaAnimal::Search()
{
	if(Activity() != Searching)
	{
		// Start searching.
		SetActivity(Searching);
	}
	else
	{
		// Already searching. Check for possibility of doing something else
		// based on ecological strategy.
		int changeMind = g_Au.Dice().Roll(1,100);
		switch(EcoStrategy())
		{
		case AuDNABase::ECO_Waiter:	// Waits a lot; searches very seldom.
			if(changeMind > 92)
				SetActivity(Waiting);
			break;
		case AuDNABase::ECO_Grazer:	// Waits seldom; search usually finds food.
			if(changeMind > 97)
				SetActivity(Waiting);
			break;
		case AuDNABase::ECO_Hunter:	// Alternates searching and waiting.
			if(changeMind > 98)
				SetActivity(Waiting);
			break;
		case AuDNABase::ECO_Sneaker:	// Searches a lot; waits very seldom.
			if(changeMind > 96)
				SetActivity(Waiting);
			break;
		}
	}
}

void DaAnimal::Wait()
{
	if(Activity() != Waiting)
	{
		// Start waiting.
		SetActivity(Waiting);
	}
	else
	{
		// Already waiting. Check for possibility of doing something else
		// based on ecological strategy.
		int changeMind = g_Au.Dice().Roll(1,100);
		switch(EcoStrategy())
		{
		case AuDNABase::ECO_Waiter:	// Waits a lot; searches very seldom.
			if(changeMind > 96)
				SetActivity(Searching);
			break;
		case AuDNABase::ECO_Grazer:	// Waits seldom; search usually finds food.
			if(changeMind > 94)
				SetActivity(Searching);
			break;
		case AuDNABase::ECO_Hunter:	// Alternates searching and waiting.
			if(changeMind > 98)
				SetActivity(Searching);
			break;
		case AuDNABase::ECO_Sneaker:	// Searches a lot; waits very seldom.
			if(changeMind > 92)
				SetActivity(Searching);
			break;
		}
	}
}

AuPoint DaAnimal::Acceleration() const
{
	// Creature does not get to control acceleration when not touching the
	// ground, unless it is flying, swimming, in space, or positioned for
	// using a gravity well, hatch, or turret. Did I miss anything?
	if(!CanControlMovement())
		return DaCreature::Acceleration();	// Gravity force alone.

	// Movement speed determined by my current general body pose.
	double speed = MovementSpeed(1);	// in feet per second

	// When mounted, respond to the desires of the rider.
	// Riderless animals are handled elsewhere.
	const DaPerson* rider = FindPersonOnLineRecursive();

	// Movement actions.
	AuPoint vmove;		// Vertical movement
	bool moveForward = false;
	bool moveBackward = false;
	if(rider)
	{
		unsigned int riderActions = rider->Actions();
		//qDebug("DaAnimal::Acceleration riderActions %x", riderActions);
		if(!(rider->Actions() & ACT_Brake))
		{
			if((riderActions & ACT_MoveForward) || (IsFlying() && !CanHover()))
				moveForward = true;
			else if (riderActions & ACT_MoveBackward)
				moveBackward = true;
		}

		if(riderActions & ACT_MoveDown)
			vmove = AuPoint(0,0,-1);
		else if (riderActions & ACT_MoveUp)
		{
			// Only allow up movement in zero G, swimming, flying, or climbing a ladder.
			if(!IsInGravity() || IsInGravityWell() || IsInWater() || IsFlying() || IsLadderPose() || InLadderWell())
				vmove = AuPoint(0,0,0.5);
		}
	}
	else if(speed != 0)
	{
		// This animal is in a moving pose.
		moveForward = true;
	}

	AuPoint fmove;		// Forward/backward movement.
	if(moveForward || moveBackward)
	{
		// Calculate forward movement vector.
		if(moveForward)
			fmove = AuPoint(0,1,0);
		else
			fmove = AuPoint(0,-1,0);
		if(IsOnGround() && IsInGravity() && !CanBeUnderGround() && !IsInGravityWell() && !IsInWater() && !IsJumping() &&
				!IsFlying() && !IsLadderPose() && !InLadderWell())
		{
			// Tilt forward vector to move parallel to the ground.
			double gDot = fmove.Dot(TerrainNormal());	// Distance along gravity vector.
			if(gDot != 0)
			{
				fmove -= TerrainNormal() * gDot;
				if(gDot < 0)
					speed *= fmove.Length();	// Uphill movement reduces speed based on steepness.
				fmove.Unit();
			}
		}
		else
		{
			// Tilt forward movement vector according to head tilt.
			fmove = fmove.RotateX(CurrentHeadTilt());
		}
	}

	// Calculate movement force.
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

	// Cumulative movement vector.
	// Tilt movement vector according to head tilt.
	AuPoint lookMove = fmove + vmove;
	if(!lookMove.IsZero())
		lookMove.Unit(force);

	// Now that we know the final desired movement vector, check to see if the
	// animal is too fast and accelerating. This animal can't go faster if
	// they're already beyond their maximum speed; they can decelerate however.
	// This logic only applies when inside atmosphere range of a planet or
	// inside a spacecraft.
	if(IsInGravity())
	{
		AuPoint v(Vector());
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
			//lookMove += DaCreature::Acceleration()/2;
		//else
			lookMove += DaCreature::Acceleration();
	}
#if !defined(QT_NO_DEBUG)
	if((g_Au.m_LogMessages & LOG_Physics) && rider)
		qDebug() << "DaAnimal::Acceleration" << Id() << Name() << lookMove;
#endif
	return lookMove;
}

AuPoint DaAnimal::Friction() const
{
	// Determine if brake is on.
	// When mounted, respond to the desires of the rider.
	const DaPerson* rider = FindPersonOnLineRecursive();
	if(rider && (rider->Actions() & ACT_Brake))
	{
#if !defined(QT_NO_DEBUG)
		if(g_Au.m_LogMessages & LOG_Physics)
			qDebug() << "DaAnimal::Friction braking" << Id() << Name() << "rider" << rider->Id() << rider->Name()
				<< "onGround" << IsOnGround() << "canFly" << CanFly() << "isFlying" << IsFlying();
#endif
		if(!IsOnGround() && CanFly() && IsFlying())
			return DaCreature::Friction() + AuPoint(30,30,30);	// Flying creatures need lots of friction in the air to stop.
		return DaCreature::Friction() + AuPoint(15,15,15);
	}
	return DaCreature::Friction();
}

// True if this atom is permitted to go below the surface of the terrain.
bool DaAnimal::CanBeUnderGround() const
{
	// Domestic animals never go under ground, even if subterranean.
	if(!IsWild())
		return false;

	// Alert creatures that are not already under ground will not go there.
	if(IsAlert() && !IsUnderGround())
		return false;

	// Let the base class decide.
	return DaCreature::CanBeUnderGround();
}

QString DaAnimal::Name() const
{
	QString s(AssignedName());
	if(s.isEmpty())
		return SpeciesName();	// Get the name of the specie, or a generic name when the specie is not named.
	else
		return s;
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaAnimal::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitNormal.IsZero())
	{
		SetVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	AuPoint bounceVec = DaCreature::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
	if(hitShield)
		return bounceVec;	// I must be moving pretty fast.

	// The base class has reflected my movement vector based on the hit normal at the bounce location.
	if(IsWild())
	{
		// A wild animal has to decide what to do. Domestic animals just stand there and don't move around or turn.
		// Turn to face my nu direction so I can move away from the object.
		float turnZ;
		if(Vector().IsFuzzyZero())
		{
			turnZ = g_Au.Dice().Roll(1,361,-181);
			turnZ *= DEG2RADf;
		}
		else
		{
			turnZ = Vector().ArcTangent_2D() - PIf/2;
			if(turnZ < -PIf)
				turnZ += PIf*2;
			if(IsInCapsule() && hit == Parent())
				turnZ /= 2;			// Turn parallel to the wall.
		}
		if(turnZ != TurnZ())
		{
			SetTurnZ(turnZ);
			SetModified(NOTIFY_Scene);	// db doesn't need to know about facing changes but the client scenes need to be updated.
		}
	}

	return bounceVec;
}

void DaAnimal::operator>>(QDataStream& out) const
{
	DaCreature::operator>>(out);
	out << m_ActivityTick;
	out << m_IsAlert;

	m_ThinkLock.lockForRead();
	out << (quint8)m_Activity;
	out << (qint16)m_AssumePose;
	out << m_AttackWeapon;
	out << m_Decision;
	QTime ffCheck;
	out << ffCheck;
	out << m_HasAttacked;
	out << m_HeadTilt;
	out << m_TargetId;
	out << m_TargetDir;
	out << m_TargetRange;
	out << m_TurnZ;
	out << m_UseSpecWpn;
	m_ThinkLock.unlock();
}

void DaAnimal::operator<<(QDataStream& in)
{
	DaCreature::operator<<(in);
	in >> m_ActivityTick;
	in >> m_IsAlert;

	m_ThinkLock.lockForWrite();
	quint8 ui8;
	in >> ui8;
	m_Activity = (ACTIVITY)ui8;
	qint16 i16;
	in >> i16;
	m_AssumePose = i16;
	in >> m_AttackWeapon;
	in >> m_Decision;
	QTime ffCheck;
	in >> ffCheck;
	in >> m_HasAttacked;
	in >> m_HeadTilt;
	in >> m_TargetId;
	in >> m_TargetDir;
	in >> m_TargetRange;
	in >> m_TurnZ;
	in >> m_UseSpecWpn;
	m_ThinkLock.unlock();
}

void DaAnimal::DoAI(const AuTime& last, const AuTime& now)
{
	// Decide what to do now.
	DaAtom* foodAtom;
	AuPoint foodDir;			// Food that is not a creature.
	float foodRange;
	DaAtom* preyAtom;
	AuPoint preyDir;			// Creature that is potential food.
	float preyRange;
	DaAtom* threatAtom;
	AuPoint threatDir;		// Creature that is a threat.
	float threatRange;
	DaAtom* dangerAtom;
	AuPoint dangerDir;		// Creature that is a danger.
	float dangerRange;
	Look(foodAtom, foodDir, foodRange,
			preyAtom, preyDir, preyRange,
			threatAtom, threatDir, threatRange,
			dangerAtom, dangerDir, dangerRange);

	// Rats always eat and hunt, regardless of hunger level.
	if(!LooksLikeVermin())
	{
		// I am not a rat.
		// Decide if I care about food based on hunger level.
		if(CurrentHunger() > HungryThreshold())
		{
			// This creature doesn't want to eat food right now.
			foodAtom = 0;
			foodRange = 0;
		}

		// Decide if I care about prey based on hunger level.
		if(CurrentHunger() > HuntThreshold())
		{
			// This creature doesn't want to hunt right now.
			preyAtom = 0;
			preyRange = 0;
		}
	}
/*
	if(foodRange > 0. || preyRange > 0. || threatRange > 0.)
	{
		double fZ = (foodDir.ArcTangent_2D() - PI/2.) * RAD2DEG;
		double pZ = (preyDir.ArcTangent_2D() - PI/2.) * RAD2DEG;
		double tZ = (threatDir.ArcTangent_2D() - PI/2.) * RAD2DEG;
		if(foodRange > 0.)
		qDebug("DaAnimal::Tick - %s %d %s sees food bearing %g %g ft %g",
				(const char*)Name(), Id().GetID(),
				(const char*)Description(), fZ, foodRange,
				foodDir.PolarIncline()*RAD2DEG);
		if(preyRange > 0.)
		qDebug("DaAnimal::Tick - %s %d %s sees prey bearing %g %g ft %g",
				(const char*)Name(), Id().GetID(),
				(const char*)Description(), pZ, preyRange,
				preyDir.PolarIncline()*RAD2DEG);
		if(threatRange > 0.)
		qDebug("DaAnimal::Tick - %s %d %s sees a threat bearing %g %g "
				"ft %g", (const char*)Name(), Id().GetID(),
				(const char*)Description(), tZ, threatRange,
				threatDir.PolarIncline()*RAD2DEG);
	}
*/

	// Make fight or flight check every few seconds.
	m_ThinkLock.lockForRead();
	float turnZ = m_TurnZ;
	bool ffCheck = !m_FFCheck.isValid() || m_FFCheck.elapsed() >= 3000;
	m_ThinkLock.unlock();
	if(ffCheck)
	{
		m_ThinkLock.lockForWrite();
		m_FFCheck.restart();
		m_ThinkLock.unlock();

		if(dangerRange > 0 && !IsAlert())
		{
			// There is a danger within range and this animal is not alert.
			// Test to see if it becomes agitated; m_IsAlert will become true.
			FlightCheck(dangerRange);
		}

		if(threatRange > 0 && !IsAlert())
		{
			// There is a threat within range and this animal is not alert.
			// Test to see if it becomes agitated; m_IsAlert will become true.
			FightOrFlightCheck(threatRange, 0);
		}

		if(!IsAlert())
		{
			// Not alert so think about turning a little to look around.
			if(g_Au.Dice().Roll(1,7) > 1)
			{
				// Turn left or right based on turn rate. -50..+50
				float turnroll = g_Au.Dice().Roll(1,101,-51);
				turnZ = turnroll/100.f * TurnRate();
			}
		}
		else if(dangerRange == 0 && threatRange == 0 && preyRange == 0)
		{
			// Animal is alert but no danger, threats or prey were seen in range.
			if(g_Au.Dice().Roll(1,100) > 90)
				SetAlert(false);	// Animal relaxes.
		}
	}

	DaAtom* targetAtom = 0;
	AuPoint targetDir;
	float targetRange = 0;
	AuDNABase::WEAPONLOC wLoc;
	AuCombatWeapon weapon(WM_Not);
	int ammoCur, ammoFull;
	bool useSpecWpn = false;
	switch(Activity())
	{
	case Attacking:
		if(threatRange > 0 &&
				(preyRange == 0 || threatRange < preyRange) &&
				(foodRange == 0 || threatRange < foodRange*2))
		{
			// This animal is attacking and there is a threat in sight.
			targetAtom = threatAtom;
			targetDir = threatDir;
			targetRange = threatRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect == WE_None || !weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d Charging to Attack", Id().GetID());
				SetActivity(Charging);	// Charge to close distance.
				SetAlert(true);
			}
		}
		else if(preyRange > 0 &&
				(foodRange == 0 || preyRange < foodRange/2))
		{
			// This animal is attacking and there is prey in sight.
			targetAtom = preyAtom;
			targetDir = preyDir;
			targetRange = preyRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect == WE_None || !weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d Chasing to Attack",
						//Id().GetID());
				SetActivity(Chasing);	// Chase to close distance.
				SetAlert(true);
			}
		}
		else
		{
			// Resume search for food.
			Search();
		}
		break;
	case Charging:
		if(threatRange > 0)
		{
			// This animal is charging and there is a threat in sight.
			targetAtom = threatAtom;
			targetDir = threatDir;
			targetRange = threatRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None && weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d caught threat, "
						//"Attacking", Id().GetID());
				SetActivity(Attacking);	// In range to attack.
				SetAlert(true);
			}
		}
		else
		{
			// Resume search for food.
			Search();
		}
		break;
	case Chasing:
		if(preyRange > 0)
		{
			// This animal is chasing and there is prey in sight.
			targetAtom = preyAtom;
			targetDir = preyDir;
			targetRange = preyRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None && weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d caught prey, Attacking",
						//Id().GetID());
				SetActivity(Attacking);	// In range to attack.
				SetAlert(true);
			}
		}
		else
		{
			// Resume search for food.
			Search();
		}
		break;
	case Eating:
	case Gathering:
		if(foodRange > 0)
		{
			// This animal is eating or gathering and there is food in sight.
			targetAtom = foodAtom;
			targetDir = foodDir;
			targetRange = foodRange;
			if(foodRange <= BiteRange() + CGHeight())
			{
				//qDebug("DaAnimal::Tick %d gathered food, Eating", Id().GetID());
				SetActivity(Eating);
			}
			else
			{
				//qDebug("DaAnimal::Tick %d Gathering food", Id().GetID());
				SetActivity(Gathering);
			}
		}
		else
		{
			// Resume search for food.
			Search();
		}
		break;
	case Fleeing:
		if(dangerRange > 0)
		{
			// This animal is fleeing and a danger is in sight;
			// keep fleeing.
			targetAtom = dangerAtom;
			targetDir = -dangerDir;		// Direction to run.
			targetRange = dangerRange;
		}
		else if(threatRange > 0)
		{
			// This animal is fleeing and a threat is in sight;
			// keep fleeing.
			targetAtom = threatAtom;
			targetDir = -threatDir;		// Direction to run.
			targetRange = threatRange;
		}
		else
		{
			// Resume search for food.
			Search();
		}
		break;
	case Searching:
		if(preyRange > 0 &&
				(foodRange == 0 || preyRange < foodRange/2))
		{
			// This animal is searching and there is prey in sight.
			targetAtom = preyAtom;
			targetDir = preyDir;
			targetRange = preyRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None && weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d Searching for prey, "
						//"Attacking", Id().GetID());
				SetActivity(Attacking);	// In range to attack.
				SetAlert(true);
			}
			else
			{
				//qDebug("DaAnimal::Tick %d Searching for prey, "
						//"Chasing", Id().GetID());
				SetActivity(Chasing);
				SetAlert(true);
			}
		}
		else if(foodRange > 0)
		{
			// This animal is searching and there is food in sight.
			targetAtom = foodAtom;
			targetDir = foodDir;
			targetRange = foodRange;
			if(foodRange <= BiteRange() + CGHeight())
			{
				//qDebug("DaAnimal::Tick %d Searching for food, Eating", Id().GetID());
				SetActivity(Eating);
			}
			else
			{
				//qDebug("DaAnimal::Tick %d Searching for food, Gathering", Id().GetID());
				SetActivity(Gathering);
			}
		}
		else
		{
			// Continue searching for food, or change my mind and wait for a while.
			Search();
		}
		break;
	case Waiting:
		if(preyRange > 0 &&
				(foodRange == 0 || preyRange < foodRange/2))
		{
			// This animal is waiting and there is prey in sight.
			targetAtom = preyAtom;
			targetDir = preyDir;
			targetRange = preyRange;

			// Get info about the weapon used at this range.
			useSpecWpn = BestWeaponAtRange(targetRange, wLoc, weapon, ammoCur, ammoFull);
			if(weapon.m_w1.m_Effect != WE_None && weapon.IsOptimumRange(targetRange))
			{
				//qDebug("DaAnimal::Tick %d Waiting for prey, "
						//"Attacking", Id().GetID());
				SetActivity(Attacking);	// In range to attack.
				SetAlert(true);
			}
			else
			{
				//qDebug("DaAnimal::Tick %d Waiting for prey, "
						//"Chasing", Id().GetID());
				SetActivity(Chasing);
				SetAlert(true);
			}
		}
		else if(foodRange > 0)
		{
			// This animal is waiting and there is food in sight.
			targetAtom = foodAtom;
			targetDir = foodDir;
			targetRange = foodRange;
			if(foodRange <= BiteRange() + CGHeight())
			{
				//qDebug("DaAnimal::Tick %d Waiting for food, Eating", Id().GetID());
				SetActivity(Eating);
			}
			else
			{
				//qDebug("DaAnimal::Tick %d Waiting for food, Gathering", Id().GetID());
				SetActivity(Gathering);
			}
		}
		else
		{
			// Continue waiting for food, or change my mind and search for a while.
			Wait();
		}
		break;
	}

	// Turn to face the target (away from target when fleeing).
	m_ThinkLock.lockForWrite();
	m_HeadTilt = CurrentHeadTilt();
	DhDNA dna(DNA3());
	if(targetRange > 0)
	{
		// I have spotted a target.
		if(IsInCapsule())
		{
			DaAtom* parent = Parent();
			if(parent == targetAtom->Parent())
			{
				// Find a path to the target and turn onto the path.
				if(dynamic_cast<DaSc*>(parent))
				{
					DaSc* sc = (DaSc*)parent;
					AuPoint toLoc = targetAtom->Location();
					AuScRoute route;
					if(!sc->Route(route, Location(), toLoc, this, false) || route.empty())
					{
						// No route to the destination.
						m_Activity = Waiting;
					}
					else
					{
						// Successful path so turn toward first waypoint.
						AuPoint toVec = Transform().UnProject(route.FirstLoc());	// First location in my space.
						targetRange = toVec.Length();
						targetDir = toVec;
						if(targetRange != 0)
							targetDir /= targetRange;
					}
				}
			}
		}
		turnZ = targetDir.ArcTangent_2D() - PIf/2.f;	// Turn directly toward the target, which could be a waypoint in a capsule.
		if(IsFlying())
		{
			// When flying away from the target, we must allow an
			// outbound leg so we have room to turn and attack the
			// target.
			const float outboundLeg = dna.SightRange()/2.f;
			if(targetRange > outboundLeg ||
					(weapon.m_w1.m_Effect == WE_None && fabs(turnZ) < PIf/2.f))
			{
				// Target is in front of me and beyond attack range
				// or the target is beyond outbound leg distance.
				// Turn inbound.
				// Tilt head to face target.
				m_HeadTilt = targetDir.PolarIncline();
			}
			else
			{
				// Climb when flying the outbound leg.
				float prefAlt = dna.SightRange() * .7071f;
				if(prefAlt > AltitudeAGL())
					m_HeadTilt = 25.f*DEG2RADf;
				else
					m_HeadTilt = 0;
				turnZ = 0;
			}
		}
		else if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
		{
			// When flying or swimming away from the target, we
			// must allow an outbound leg so we have room to turn
			// and attack the target.
			float outboundLeg;
			if(dna.SightRange() > dna.HearingRange())
				outboundLeg = dna.SightRange()/2;
			else
				outboundLeg = dna.HearingRange()/2;
			if(targetRange > outboundLeg ||
					(weapon.m_w1.m_Effect == WE_None && fabs(turnZ) < PIf/2.f))
			{
				// Target is in front of me and beyond attack range
				// or the target is beyond outbound leg distance.
				// Turn inbound.
				// Tilt head to face target.
				m_HeadTilt = targetDir.PolarIncline();
			}
			else
			{
				// Stay level on the outbound leg.
				m_HeadTilt = 0;
				turnZ = 0;
			}
		}
		else
		{
			// Tilt head to face target.
			m_HeadTilt = targetDir.PolarIncline();
		}
	}
	else
		m_HeadTilt = 0;

	// Turn to break free from obstacles.
	if(MoveResult() == CT_HitParent || MoveResult() == CT_HitPeer)
	{
		// Something is blocking my path.
		turnZ -= PIf/4;		// Turn
		if(now.IsNewPeriod(last, 10*d_MSecPerSec))
			turnZ = -turnZ;	// Reverse turn direction in each 10 second period.
		if(IsLeftHanded())
			turnZ = -turnZ;	// Reverse turn direction for lefties.
	}

	// Assume the appropriate pose.
	m_AssumePose = 0;
	switch(m_Activity)
	{
	case Attacking:
		if(IsUnderGround())
			m_AssumePose = -1;
		else if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
			m_AssumePose = -2;
		else if(CanHover())
			m_AssumePose = -3;
		else if(CanFly())
			m_AssumePose = -4;
		else
			m_AssumePose = -5;

		// Use weapon on the target.
		{
			// I can attack once per positive bite cycle.
			float rhythm = dna.Rhythm(AuPose::Bite, 0, 1);
			if(!m_HasAttacked && rhythm > 0)
			{
				if(targetAtom)
				{
					m_Decision = 1;
					m_TargetId = targetAtom->Id();
					m_TargetDir = targetDir;
					m_TargetRange = targetRange;
					m_UseSpecWpn = useSpecWpn;
					m_AttackWeapon = weapon;
				}
			}
			else
			{
				// Reset the attack flag on the downward side of the bite cycle.
				// Then I can attack again when the cycle turns positive.
				if(m_HasAttacked && rhythm < 0)
					m_HasAttacked = false;	// Now it's ok to attack on the next positive body rhythm cycle.
			}
		}
		break;
	case Eating:
		m_AssumePose = -6;				// eat off ground
		m_HeadTilt = -PIf*3.f/8.f;		// Look down when eating.
		
		// Eat the target.
		{
			// I can eat once per positive bite cycle.
			float rhythm = dna.Rhythm(AuPose::Eat, 0, 1);
			if(!m_HasAttacked && rhythm > 0)
			{
				if(targetAtom)
				{
					// Eat the target atom. It will give up food nutrition value when it is gone.
					m_Decision = 2;
				}
				else if(IsHerbivore())
				{
					// No real food normally for Herbivores in the environment.
					// So pretend they graze on vegetation at hand.
					// 10 is the food nutrition value of the Hay commodity.
					m_Decision = 3;
				}

				// Now I can't eat again until the next positive bite cycle.
				m_HasAttacked = true;
			}
			else
			{
				// Reset the attack flag on the downward side of the bite cycle.
				// Then I can eat again when the cycle turns positive.
				if(m_HasAttacked && rhythm < 0)
					m_HasAttacked = false;	// Now it's ok to eat on the next positive body rhythm cycle.
			}
		}
		break;
	case Charging:
	case Chasing:
		if(IsUnderGround())
		{
			m_AssumePose = -7;
			if(InIndustry() == IID_None ||
					InIndustry() == IID_Park ||
					InIndustry() == IID_Farm ||
					InIndustry() == IID_Orchard ||
					InIndustry() == IID_LoggingCamp ||
					InIndustry() == IID_House ||
					InIndustry() == IID_AerialPlatform ||
					InIndustry() == IID_SeaPlatform ||
					InIndustry() == IID_DrillingRig ||
					InIndustry() == IID_Well)
				m_HeadTilt = PIf/2.f;	// Look/dig straight up to surface where ground is not too developed.
			else
				m_HeadTilt = 0;		// Burrow straight ahead until I get out from under the buildings.
		}
		else if(IsCGSubmerged() || InDeepEnoughLiquidToSwim())
		{
			// Fast if target is moving.
			if(m_Stamina && !targetAtom->Vector().IsFuzzyZero())
				m_AssumePose = AuPose::SwimFast;
			else
				m_AssumePose = AuPose::Swim;
		}
		else if(CanFly())
		{
			// Fast if target is moving.
			if(m_Stamina && !targetAtom->Vector().IsFuzzyZero())
				m_AssumePose = AuPose::FlyFast;
			else
				m_AssumePose = AuPose::Fly;
		}
		else if(IsLeaper() && m_Stamina > StaminaToJump() && CanJump() && targetRange > 20 && fabs(turnZ) < 2*DEG2RAD)
		{
			// All systems go to jump toward target.
			m_Decision = 4;
			m_TargetRange = targetRange;
		}
		else
		{
			// When target is close and we're turning, stand to turn. Running or walking causes me to circle the target.
			if(targetRange < 30 && fabs(turnZ) > 2*DEG2RAD)
				m_AssumePose = AuPose::Stand;
			// Run if target is moving.
			else if(m_Stamina && !targetAtom->Vector().IsFuzzyZero())
				m_AssumePose = AuPose::StandRun;
			else
				m_AssumePose = AuPose::StandWalk;
		}
		break;
	case Fleeing:
		if(IsUnderGround())
		{
			m_AssumePose = -7;

			// Look/dig straight down to flee.
			float prefDep = -dna.HearingRange()/2.f;
			if(prefDep < AltitudeAGL())
				m_HeadTilt = -PIf/2.f;	// Descend to preferred alt.
			else
				m_HeadTilt = 0;			// Stay Level
		}
		else if(IsCGSubmerged() || InDeepEnoughLiquidToSwim())
		{
			if(m_Stamina)
				m_AssumePose = AuPose::SwimFast;
			else
				m_AssumePose = AuPose::Swim;
			m_HeadTilt = 0;
		}
		else if(CanFly())
		{
			if(m_Stamina)
				m_AssumePose = AuPose::FlyFast;
			else
				m_AssumePose = AuPose::Fly;

			// Not going after food so climb/dive quickly to
			// preferred altitude.
			float prefAlt = dna.SightRange() * .7071f;
			if((prefAlt-10) > AltitudeAGL())
				m_HeadTilt = 15.f*DEG2RADf;		// Max climb to
											// preferred altitude.
			else if((prefAlt+10) < AltitudeAGL())
				m_HeadTilt = -45.f*DEG2RADf;	// Dive to preferred alt
			else
				m_HeadTilt = 0;
		}
		else if(IsLeaper() && m_Stamina > StaminaToJump() && CanJump() && fabs(turnZ) < 2*DEG2RAD)
		{
			m_Decision = 5;	// All systems go to jump away from target.
			m_HeadTilt = -PIf/8.f;	// Jump away with head down
		}
		else
		{
			if(m_Stamina)
				m_AssumePose = AuPose::StandRun;
			else
				m_AssumePose = AuPose::StandWalk;
			m_HeadTilt = -PIf/8.f;	// Flee with head tilted down
		}
		break;
	case Gathering:
		if(IsUnderGround())
			m_AssumePose = -7;
		else if(IsCGSubmerged() || InDeepEnoughLiquidToSwim())
			m_AssumePose = AuPose::Swim;
		else if(CanFly())
			m_AssumePose = AuPose::Fly;
		else
			m_AssumePose = AuPose::StandWalk;
		m_HeadTilt = 0;
		break;
	case Searching:
		if(IsUnderGround() || (CanBeUnderGround() &&
					(InIndustry() == IID_None ||
					InIndustry() == IID_Park ||
					InIndustry() == IID_Farm ||
					InIndustry() == IID_Orchard ||
					InIndustry() == IID_LoggingCamp ||
					InIndustry() == IID_House ||
					InIndustry() == IID_AerialPlatform ||
					InIndustry() == IID_SeaPlatform ||
					InIndustry() == IID_DrillingRig ||
					InIndustry() == IID_Well)))
		{
			// Creature is under ground or can be under ground and the area isn't too developed to dig.
			m_AssumePose = -7;

			// Ascend/descend to preferred depth.
			float prefDep = -dna.HearingRange()/2.f;
			if((prefDep-10) > AltitudeAGL())
				m_HeadTilt = PIf/4.f;		// Climb to preferred altitude.
			else if((prefDep+10) < AltitudeAGL())
				m_HeadTilt = -PIf/4.f;	// Descend to preferred alt.
			else
				m_HeadTilt = 0;			// Stay Level
		}
		else if(IsCGSubmerged() || InDeepEnoughLiquidToSwim())
		{
			m_AssumePose = AuPose::Swim;
			m_HeadTilt = 0;	// No climb/descend except with terrain.
		}
		else if(CanFly())
		{
			m_AssumePose = AuPose::Fly;

			// Climb/descend to preferred altitude.
			float prefAlt = dna.SightRange() * .7071f;
			if((prefAlt-10) > AltitudeAGL())
				m_HeadTilt = 10.f*DEG2RADf;	// Climb to pref alt.
			else if((prefAlt+10) < AltitudeAGL())
				m_HeadTilt = -15.f*DEG2RADf;	// Descend to pref alt.
			else
				m_HeadTilt = 0;		// Stay Level
		}
		else if(IsLeaper() && m_Stamina > StaminaToJump() && CanJump() && fabs(turnZ) < 2*DEG2RAD)
		{
			m_Decision = 5;	// All systems go to jump in search of food.
			m_HeadTilt = -PIf/8.f;	// Jump away with head down
		}
		else
		{
			m_AssumePose = AuPose::StandWalk;
			m_HeadTilt = 0;
		}
		break;
	case Waiting:
		if(IsUnderGround())
			m_AssumePose = AuPose::Stand;
		else if(IsCGSubmerged() && InDeepEnoughLiquidToSwim())
			m_AssumePose = AuPose::TreadWater;
		else if(CanHover())
			m_AssumePose = AuPose::Hover;
		else if(CanFly())
		{
			m_AssumePose = AuPose::Glide;
			if(turnZ < 0)
				turnZ = -turnZ;		// Circle
			if(IsLeftHanded())
				turnZ = -turnZ;		// Circle the other way.
		}
		else
		{
			// Wild herbivores just stand and eat when conditions are right while "waiting".
			if(IsHerbivore() && IsAirBreathable() &&
					(IsOnForest() || IsOnRock() || IsOnSand() || IsOnSavanna() || IsOnSnow()) &&
					InIndustry() != IID_Road &&
					dynamic_cast<const DaWorldLL*>(Parent()))
			{
				// Herbivore standing in the right environment for eating.
				if(!IsEatingPose())
					m_AssumePose = -6;
			}
			else
				m_AssumePose = AuPose::Stand;
		}
		m_HeadTilt = 0;			// Wait/Glide at current alt.
		break;
	}

	bool turnZChanged = turnZ != m_TurnZ;
	if(turnZChanged)
	{
		//qDebug() << "DaAnimal::DoAI" << Id() << Name() << "turnZ" << (turnZ*RAD2DEG) << "oldTurnZ" << (m_TurnZ*RAD2DEG);
		m_TurnZ = turnZ;
	}

	m_ThinkLock.unlock();

	if(turnZChanged)
		SetModified(NOTIFY_Scene);	// db doesn't need to know about facing changes but the client scenes need to be updated.
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaAnimal::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	if(!IsInAtmosphere())
		return false;	// Turrets don't reach out of the atmosphere.
	if(IsUnderGround())
		return false;	// Turrets don't reach under ground.
	if(IsBodyDyingOrDead())
		return false;

	if(IsWild())
	{
		// Wild animals are only targets when gunner posture is offensive or berserk.
		if(gunnerPosture != POSTURE_Offensive && gunnerPosture != POSTURE_Berserk)
			return false;

		if(gunLoc.Distance2(Location()) > gunRange*gunRange)
			return false;	// I am out of range.
	}
	else
	{
		// Domestic animal.
		if(!IsMounted())
		{
			// Unmounted domestic animals are not targets unless the gunner is berserk.
			if(gunnerPosture != POSTURE_Berserk)
				return false;	// Gunner is not berserk.
		}

		if(gunLoc.Distance2(Location()) > gunRange*gunRange)
			return false;	// I am out of range.

		if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
			return false;	// I am not an enemy.
	}

	// I am a target.
	targetLoc = Transform().Project(TargetLocation());
	return true;
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaAnimal::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;
	if(IsBodyDyingOrDead())
		return false;
	if(IsWild())
	{
		// Wild animals are only targets when gunner posture is offensive or berserk.
		if(gunnerPosture != POSTURE_Offensive && gunnerPosture != POSTURE_Berserk)
			return false;

		// Perform range test.
		AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
		if(targetVec.Length2() > gunRange*gunRange)
			return false;

		if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
			return false;
	}
	else
	{
		// Domestic animal.
		if(!IsMounted())
		{
			// Unmounted domestic animals are not targets unless the gunner is berserk.
			if(gunnerPosture != POSTURE_Berserk)
				return false;	// Gunner is not berserk.
		}

		// Perform range test.
		AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
		if(targetVec.Length2() > gunRange*gunRange)
			return false;

		if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
			return false;

		if(gunnerEmpire && PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
			return false;	// I am not an enemy.
	}

	targetSysLoc = EnvTransform().Project(TargetLocation());
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaAnimal::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	AuPoint myTargLoc;
	if(!IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange, gunSysVec,
				myTargLoc))
		return;

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = myTargLoc.Distance(gunSysLoc);
	targetSysLoc = myTargLoc;
}

void DaAnimal::MakeDecision(const AuTime& last, const AuTime& now)
{
	// Wild animals have a mind of their own.
	// They're a bit dim-witted though.
	if(MoveResult() == CT_HitParent || MoveResult() == CT_HitPeer)
		m_ActivityTick = 1;	// Do something now.
	else if(!m_ActivityTick)
	{
		// This animal is not waiting to make a decision. See if it is time for another run at the decision making code. Bottom bits
		// of id are used as number of Ticks until the next decision making step. This scatters animals out a bit so they don't all
		// do their AI work in the same Tick cycle. Some are smarter than others because they will think in fewer cycles.
		ACTIVITY act = Activity();
		if(act == Charging || act == Chasing)
			m_ActivityTick = (Id().GetID()&0x3)+2;	// in 2-5 tick cycles
		else
			m_ActivityTick = (Id().GetID()&0xf)+10;	// in 10-25 tick cycles
	}
	
	if(m_ActivityTick == 1) 						// This Animal's turn
		DoAI(last, now);							// Decide what to do now.

	// When waiting to make a decision, I must decrement my Tick frame counter. When it reaches 1, I'll decide what to do. When it
	// reaches 0, it will get reset to start waiting to decide what to do again.
	m_ActivityTick--;
}

void DaAnimal::ExecuteDecision()
{
	// Grab decision values so we can unlock the lock.
	m_ThinkLock.lockForRead();
	int assumePose = m_AssumePose;
	float headTilt = m_HeadTilt;
	quint8 decision = m_Decision;
	AuCombatWeapon weapon(m_AttackWeapon);
	AuId targetId(m_TargetId);
	AuPoint targetDir(m_TargetDir);
	double targetRange(m_TargetRange);
	bool useSpecWpn = m_UseSpecWpn;
	m_ThinkLock.unlock();

	bool isModified = false;

	// Head swiveling actions.
	if(TiltHeadTo(headTilt))
		isModified = true;

	if(assumePose < 0)
	{
		bool posechanged;
		switch(assumePose)
		{
		case -1:
			posechanged = PoseAttackUnderGround();
			break;
		case -2:
			posechanged = PoseAttackUnderWater();
			break;
		case -3:
			posechanged = PoseAttackHovering();
			break;
		case -4:
			posechanged = PoseAttackFlying();
			break;
		case -5:
			posechanged = PoseAttackStanding();
			break;
		case -6:
			posechanged = PoseEatOffGround();
			break;
		case -7:
			posechanged = PoseBurrow(1);
			break;
		default:
			posechanged = false;
			break;
		}

		if(posechanged)
			isModified = true;
	}
	else if(assumePose > 0)
	{
		if(SetGeneralPose((AuPose::POSE)assumePose, false))
			isModified = true;
	}

	switch(decision)
	{
	case 0:
	default:
		break;
	case 1:
		// Attack a target.
		if(weapon.m_Mode == WM_Area)
		{
			// Perform area attack that excludes me and others of my own kind.
			// Wild animals do not observe peace dimension protection.
			// Animals are not pirates.
			AuCombatAttacker attacker(Id(), Name(), EmpireId());
			attacker.SetSpeciesExempt(Species());

			AuCombatDefender defender;

			AuCombatResult result;

			AttackTarget(attacker, defender, weapon, false, result);
		}
		else
		{
			// Attack the target atom.
			// Calculate a random z height from the ground up to their camera location.
			DaAtom* env = FindEnvironmentAtom();
			DaAtom* targetAtom = env->FindIdRecursive(targetId);
			if(targetAtom)
			{
				AuPoint targetLoc = targetDir * targetRange;
				float gcg = targetAtom->GroundHeightAboveCG();
				targetLoc.z -= gcg;
				targetLoc.z += gcg * g_Au.Dice().Roll(2,50) / 100;

				// Animals are not pirates.
				AuCombatAttacker attacker(Id(), Name(), EmpireId());
				if(IsInVehicle())
				{
					const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
					if(vehicle)
						attacker.m_Name = tr("%1 in %2").arg(Name()).arg(vehicle->Name());
				}
				else if(IsOnVehicle())
				{
					const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(Parent());
					if(vehicle)
						attacker.m_Name = tr("%1 on %2").arg(Name()).arg(vehicle->Name());
				}
				else if(IsInSpacecraft())
				{
					const DaVessel* ship = FindParentVesselInEnv();
					if(ship)
						attacker.m_Name = tr("%1 aboard %2").arg(Name()).arg(ship->NameWithEmpire());
				}
				else if(IsInBd())
				{
					const DaBd* bd = FindParentBdInEnv();
					if(bd)
						attacker.m_Name = tr("%1 in %2").arg(Name()).arg(bd->CityBuildingName());
				}

				AuCombatDefender defender(targetAtom, targetLoc, false, targetRange);

				AuCombatResult result;

				AttackTarget(attacker, defender, weapon, false, result);
			}
		}

		// Notify body the special weapon was used so we can't
		// use it again until its ready.
		if(useSpecWpn)
			UseSpecialWeapon();

		break;

	case 2:
		{
			// Eat the target atom. It will give up food nutrition value when it is gone.
			DaAtom* env = FindEnvironmentAtom();
			DaAtom* targetAtom = env->FindIdRecursive(targetId);
			if(targetAtom)
				EatTarget(targetAtom);
		}
		break;

	case 3:
		// No real food normally for Herbivores in the environment.
		// So pretend they graze on vegetation at hand.
		// 10 is the food nutrition value of the Hay commodity.
		SetHungerLimitMax(CurrentHunger() + 10);
		break;

	case 4:
		{
			// Jump toward target.
			SetGeneralPose(AuPose::Jump, false);

			// Reduce stamina for jumping.
			// Jump strength is reduced linearly if insufficient
			// stamina to jump at full power.
			float jF = JumpForce();
			const int stjump = StaminaToJump();
			if(m_Stamina >= stjump)
			{
				// Creature has enough stamina for full force jump.
				m_Stamina -= stjump;
			}
			else
			{
				// Creature can only pull off a partial jump.
				jF *= (float)m_Stamina/(float)stjump;
				m_Stamina = 0;
			}

			// Reduce jump force by burden.
			jF *= MovementSpeedMod();

			AuPoint nuVec(Vector());
			if(targetRange > 50)
				nuVec += AuPoint(0,0,jF*1.2).RotateX(-45*DEG2RAD);
			else
				nuVec += AuPoint(0,0,jF).RotateX(-45*DEG2RAD);
			SetVector(nuVec);

			isModified = true;
		}
		break;

	case 5:
		{
			// All systems go to jump in search of food.
			SetGeneralPose(AuPose::Jump, false);

			// Reduce stamina for jumping.
			// Jump strength is reduced linearly if insufficient
			// stamina to jump at full power.
			float jF = JumpForce();
			const int stjump = StaminaToJump();
			if(m_Stamina >= stjump)
			{
				// Creature has enough stamina for full force jump.
				m_Stamina -= stjump;
			}
			else
			{
				// Creature can only pull off a partial jump.
				jF *= (float)m_Stamina/(float)stjump;
				m_Stamina = 0;
			}

			// Reduce jump force by burden.
			jF *= MovementSpeedMod();

			AuPoint nuVec = Vector() + AuPoint(0,0,jF*1.2).RotateX(-45*DEG2RAD);
			SetVector(nuVec);

			isModified = true;
		}
		break;
	}

	m_ThinkLock.lockForWrite();
	m_AssumePose = 0;
	m_Decision = 0;
	m_ThinkLock.unlock();

	if(isModified)
		SetModified();
}

// Used for determination of color highlighting based on political stance toward this unit.
// Civilian units use a different color palette than military units.
bool DaAnimal::IsCivilian() const
{
	// Animals are considered civilian unless they are mounted, then the rider determines its civilian status.
	if(!IsMounted())
		return true;

	// This animal is mounted so the rider determines the civilian status of the animal.
	bool v;
	DaAtomList subs;
	ListSubatoms(subs);
	if(subs.empty())
		return true;
	const DaUnit* rider = dynamic_cast<const DaUnit*>(subs.first());
	if(rider && !rider->IsCivilian())
		v = false;
	else
		v = true;
	return v;
}

void DaAnimal::Defibrillated(quint32 byEmpireId)
{
	if(IsWild())
		SetEmpireId(byEmpireId);
	DaCreature::Defibrillated(byEmpireId);
}

quint64 DaAnimal::MemoryUsage() const
{
	quint64 mem = DaCreature::MemoryUsage();
	mem += sizeof(m_ActivityTick);
	mem += sizeof(m_IsAlert);
	mem += sizeof(m_ThinkLock);
	mem += sizeof(m_Activity);
	mem += sizeof(m_AssumePose);
	mem += sizeof(m_AttackWeapon);
	mem += sizeof(m_Decision);
	mem += sizeof(m_FFCheck);
	mem += sizeof(m_HasAttacked);
	mem += sizeof(m_HeadTilt);
	mem += sizeof(m_TargetId);
	mem += sizeof(m_TargetDir);
	mem += sizeof(m_TargetRange);
	mem += sizeof(m_TurnZ);
	mem += sizeof(m_UseSpecWpn);
	return mem;
}

void DaAnimal::EmpireDeleted(quint32 empireId)
{
	if(EmpireId() != empireId)
		return;
	SetEmpireId(0);	// go wild
}

void DaAnimal::FindAnimalsRecursive(DaAtomList& animals) const
{
	//if(!IsNormal())
		//return;
	DaCreature::FindAnimalsRecursive(animals);
	animals.append((DaAtom*)this);	// Cast away constness.
}

DaAtom::ATOM_TYPE DaAnimal::AtomType() const
{
	return ATOM_Animal;
}

float DaAnimal::Density() const
{
	if(LivesInWater())
		return (float)DensityOfWater();		// Swimmers do what they want.
	else
		return (float)DensityOfWater() - 1;	// Non-swimmers float.
}

void DaAnimal::Dismounted()
{
	if(IsBodyDyingOrDead())
		return;

	// I am still conscious. Change my pose to stand in place.
	AuPose pose = StationaryPose();
	if(IsFlying())
		pose = AuPose::Stand;			// Don't start gliding/hovering.
	SetGeneralPose(pose);
	SetInMotion();						// So hovering flyers aren't left hanging in the air.
	TickTock();
}
