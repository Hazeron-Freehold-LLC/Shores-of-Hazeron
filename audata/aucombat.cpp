//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucombat.cpp,v 1.19 2022/02/19 18:20:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucombat.h"
//
#include "auitem.h"
#include "ausound.h"

void AuCombatWeaponEffect::Magnify(float factor)
{
	if(factor <= 0)
		return;
	if(m_Damage)
	{
		m_Damage = (int)(factor * m_Damage);
		if(!m_Damage)
			m_Damage = 1;
	}

	if(m_DamageMod)
	{
		m_DamageMod = (int)(factor * m_DamageMod);
		if(!m_DamageMod)
			m_DamageMod = 1;
	}

	m_Radius *= factor;
	m_AP *= factor;
	m_ST *= factor;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCombatWeapon::AuCombatWeapon(const AuItem& weapon, INVENTORYSUB sub)
{
	int ammoCur;
	int ammoFull;
	weapon.WeaponInfo(sub, m_CID, m_Quality, m_Mode, m_Range, ammoCur, ammoFull,
			m_w1.m_Effect, m_w1.m_Damage, m_w1.m_DamageMod, m_w1.m_Radius, m_w1.m_AP, m_w1.m_ST,
			m_w2.m_Effect, m_w2.m_Damage, m_w2.m_DamageMod, m_w2.m_Radius, m_w2.m_AP, m_w2.m_ST);
}

AuCombatWeapon::AuCombatWeapon(const AuItem& explosive)
{
	m_CID = explosive.CommodityId();
	m_Quality = explosive.Quality();
	m_Range = 0;
	switch(m_CID)
	{
	case CID_StasisBeam:
	case CID_NeutronBeam:
	case CID_ElectronBeam:
	case CID_ErgRay:
	case CID_FreezeRay:
	case CID_CompressorBeam:
	case CID_DisruptorBeam:
	case CID_HeatRay:
	case CID_ProtonBeam:
	case CID_DisintegratorRay:
		m_Mode = WM_Not;	// These ammo items do not blow up when hit.
		break;

	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
		{
			// Bombs blow up when hit, with their normal damage effects.
			float range;
			int ammoCur;
			int ammoFull;
			explosive.WeaponInfo(IPS_Weapon1, m_CID, m_Quality, m_Mode, range, ammoCur, ammoFull,
					m_w1.m_Effect, m_w1.m_Damage, m_w1.m_DamageMod, m_w1.m_Radius, m_w1.m_AP, m_w1.m_ST,
					m_w2.m_Effect, m_w2.m_Damage, m_w2.m_DamageMod, m_w2.m_Radius, m_w2.m_AP, m_w2.m_ST);
		}
		break;

	case CID_Gigacell:
		// Gigacell blows up electric and acid when hit.
		try
		{
			const AuItemAmmo& ai = dynamic_cast<const AuItemAmmo&>(explosive);
			m_Mode = WM_Area;
			m_w1.m_Effect = WE_Electric;
			m_w1.m_Damage = ai.Damage1();
			m_w1.m_DamageMod = ai.DamageMod1();
			m_w1.m_Radius = 6;
			m_w1.m_AP = 0;
			m_w1.m_ST = 0;
			m_w2.m_Effect = WE_Acid;
			m_w2.m_Damage = ai.Damage1();
			m_w2.m_DamageMod = ai.DamageMod1();
			m_w2.m_Radius = 12;
			m_w2.m_AP = 0;
			m_w2.m_ST = 0;
		}
		catch(const bad_cast&)
		{
			m_Mode = WM_Not;
			break; // dynamic_cast failed
		}
		break;

	default:
		// Other ammo blows up when hit if it has an area effect, like rockets, missiles and heavy weapon ammo.
		try
		{
			const AuItemAmmo& ai = dynamic_cast<const AuItemAmmo&>(explosive);
			m_Mode = WM_Area;
			m_w1.m_Effect = ai.Effect1();
			m_w1.m_Damage = ai.Damage1();
			m_w1.m_DamageMod = ai.DamageMod1();
			m_w1.m_Radius = ai.Radius1();
			m_w1.m_AP = ai.ArmorPierce1F();
			m_w1.m_ST = ai.StrikeThrough1();
			m_w2.m_Effect = ai.Effect2();
			m_w2.m_Damage = ai.Damage2();
			m_w2.m_DamageMod = ai.DamageMod2();
			m_w2.m_Radius = ai.Radius2();
			m_w2.m_AP = ai.ArmorPierce2F();
			m_w2.m_ST = ai.StrikeThrough2();
		}
		catch(const bad_cast&)
		{
			m_Mode = WM_Not;
			break; // dynamic_cast failed
		}
	}
}

AuCombatWeapon::AuCombatWeapon(const AuItem* ammo, int count)
{
	m_CID = ammo->CommodityId();
	m_Quality = ammo->Quality();

	switch(m_CID)
	{
	case CID_StasisBeam:
	case CID_NeutronBeam:
	case CID_ElectronBeam:
	case CID_ErgRay:
	case CID_FreezeRay:
	case CID_CompressorBeam:
	case CID_DisruptorBeam:
	case CID_HeatRay:
	case CID_ProtonBeam:
	case CID_DisintegratorRay:
		{
			m_Mode = WM_Projectile;
			m_Range = TurretRange(m_Quality);	// For turrets; bay weapons override this range with the bay range.
			const AuItemAmmo* ai = dynamic_cast<const AuItemAmmo*>(ammo);
			if(ai)
			{
				// Energy weapons are modulated by the quality of the lens. Its effect has already been applied.
				m_w1.m_Effect = ai->Effect1();
				m_w1.m_Damage = ai->Damage1();
				m_w1.m_DamageMod = ai->DamageMod1();
				m_w1.m_Radius = ai->Radius1();
				m_w1.m_AP = ai->ArmorPierce1F();
				m_w1.m_ST = ai->StrikeThrough1();
				m_w2.m_Effect = ai->Effect2();
				m_w2.m_Damage = ai->Damage2();
				m_w2.m_DamageMod = ai->DamageMod2();
				m_w2.m_Radius = ai->Radius2();
				m_w2.m_AP = ai->ArmorPierce2F();
				m_w2.m_ST = ai->StrikeThrough2();
			}
		}
		break;

	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
		{
			m_Range = 0;
			int ammoCur;
			int ammoFull;
			ammo->WeaponInfo(IPS_Weapon1, m_CID, m_Quality, m_Mode, m_Range, ammoCur, ammoFull,
					m_w1.m_Effect, m_w1.m_Damage, m_w1.m_DamageMod, m_w1.m_Radius, m_w1.m_AP, m_w1.m_ST,
					m_w2.m_Effect, m_w2.m_Damage, m_w2.m_DamageMod, m_w2.m_Radius, m_w2.m_AP, m_w2.m_ST);
		}
		break;

	default:
		{
			m_Mode = WM_Area;
			m_Range = 0;
			const AuItemAmmo* ai = dynamic_cast<const AuItemAmmo*>(ammo);
			if(ai)
			{
				m_w1.m_Effect = ai->Effect1();
				m_w1.m_Damage = ai->Damage1();
				m_w1.m_DamageMod = ai->DamageMod1();
				m_w1.m_Radius = ai->Radius1();
				m_w1.m_AP = ai->ArmorPierce1F();
				m_w1.m_ST = ai->StrikeThrough1();
				m_w2.m_Effect = ai->Effect2();
				m_w2.m_Damage = ai->Damage2();
				m_w2.m_DamageMod = ai->DamageMod2();
				m_w2.m_Radius = ai->Radius2();
				m_w2.m_AP = ai->ArmorPierce2F();
				m_w2.m_ST = ai->StrikeThrough2();
			}
		}
	}

	if(count > 1)
	{
		m_w1.m_Damage *= count;
		m_w1.m_DamageMod *= count;
		m_w1.m_ST *= count;
		m_w2.m_Damage *= count;
		m_w2.m_DamageMod *= count;
		m_w2.m_ST *= count;
	}
}

float AuCombatWeapon::CraterRadius() const
{
	float craterRadius = 0;
	if(DbCommodity::HitMakesExplosionSound(m_CID))
	{
		if(m_w1.m_Radius > m_w2.m_Radius)
			craterRadius = m_w1.m_Radius;
		else
			craterRadius = m_w2.m_Radius;
	}
	else
	{
		// Get crater radius caused by area effect 1.
		switch(m_w1.m_Effect)
		{
		case WE_Acid:
		case WE_Audible:
		case WE_Bleeding:
		case WE_Cold:
		case WE_Corrosion:
		case WE_Disease:
		case WE_Electric:
		case WE_Implant:
		case WE_Invalid:
		case WE_None:
		case WE_Poison:
		case WE_Radiation:
		case WE_Starving:
		case WE_Stun:
		case WE_Suffocate:
			break;	// These weapon effects do not make craters.
		case WE_Bludgeon:
		case WE_Crush:
		case WE_Unused:
		case WE_Heat:
		case WE_Generic:
		case WE_Pierce:
		case WE_Slash:
		case WE_Sonic:
			craterRadius = m_w1.m_Radius;
			break;
		}

		// Get crater radius caused by area effect 2 if it is bigger.
		if(m_w2.m_Radius > craterRadius)
		{
			switch(m_w2.m_Effect)
			{
			case WE_Acid:
			case WE_Audible:
			case WE_Bleeding:
			case WE_Cold:
			case WE_Corrosion:
			case WE_Disease:
			case WE_Electric:
			case WE_Implant:
			case WE_Invalid:
			case WE_None:
			case WE_Poison:
			case WE_Radiation:
			case WE_Starving:
			case WE_Stun:
			case WE_Suffocate:
				break;	// These weapon effects do not make craters.
			case WE_Bludgeon:
			case WE_Crush:
			case WE_Unused:
			case WE_Heat:
			case WE_Generic:
			case WE_Pierce:
			case WE_Slash:
			case WE_Sonic:
				craterRadius = m_w2.m_Radius;
				break;
			}
		}
	}
	return craterRadius * .75f;
}

// Returns the sound effect of a hit by weapon 1.
int AuCombatWeapon::HitSound1() const
{
	if(DbCommodity::HitMakesExplosionSound(m_CID))
		return SOUND_HitExplosion;
	SOUND_EFFECT sound = SOUND_None;
	switch(m_w1.m_Effect)
	{
	case WE_Acid:
		sound = SOUND_HitAcid;
		break;
	case WE_Suffocate:
		sound = SOUND_HitGasping;
		break;
	case WE_Stun:
		sound = SOUND_HitParalysis;
		break;
	case WE_Poison:
		sound = SOUND_HitPoison;
		break;
	case WE_Radiation:
		sound = SOUND_HitRadiation;
		break;
	case WE_Sonic:
		sound = SOUND_HitSonic;
		break;
	case WE_Crush:
	case WE_Bludgeon:
		sound = SOUND_HitBludgeon;
		break;
	case WE_Cold:
		sound = SOUND_HitCold;
		break;
	case WE_Disease:
		sound = SOUND_HitDisease;
		break;
	case WE_Electric:
		sound = SOUND_HitElectric;
		break;
	case WE_Heat:
		sound = SOUND_HitFire;
		break;
	case WE_Pierce:
		sound = SOUND_HitPierce;
		break;
	case WE_Implant:
		sound = SOUND_HitSeed;
		break;
	case WE_Slash:
		sound = SOUND_HitSlash;
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
		sound = SOUND_None;
		break;
	}
	return sound;
}

// Returns the sound effect of a hit by weapon 2.
int AuCombatWeapon::HitSound2() const
{
	if(DbCommodity::HitMakesExplosionSound(m_CID))
		return SOUND_HitExplosion;
	SOUND_EFFECT sound = SOUND_None;
	switch(m_w2.m_Effect)
	{
	case WE_Acid:
		sound = SOUND_HitAcid;
		break;
	case WE_Suffocate:
		sound = SOUND_HitGasping;
		break;
	case WE_Stun:
		sound = SOUND_HitParalysis;
		break;
	case WE_Poison:
		sound = SOUND_HitPoison;
		break;
	case WE_Radiation:
		sound = SOUND_HitRadiation;
		break;
	case WE_Sonic:
		sound = SOUND_HitSonic;
		break;
	case WE_Crush:
	case WE_Bludgeon:
		sound = SOUND_HitBludgeon;
		break;
	case WE_Cold:
		sound = SOUND_HitCold;
		break;
	case WE_Disease:
		sound = SOUND_HitDisease;
		break;
	case WE_Electric:
		sound = SOUND_HitElectric;
		break;
	case WE_Heat:
		sound = SOUND_HitFire;
		break;
	case WE_Pierce:
		sound = SOUND_HitPierce;
		break;
	case WE_Implant:
		sound = SOUND_HitSeed;
		break;
	case WE_Slash:
		sound = SOUND_HitSlash;
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
		sound = SOUND_None;
		break;
	}
	return sound;
}

bool AuCombatWeapon::IsIncendiary1() const
{
	if(DbCommodity::HitMakesExplosionSound(m_CID))
		return true;

	switch(m_w1.m_Effect)
	{
	case WE_Radiation:
	case WE_Electric:
	case WE_Heat:
		return true;
	case WE_Acid:
	case WE_Suffocate:
	case WE_Stun:
	case WE_Poison:
	case WE_Sonic:
	case WE_Crush:
	case WE_Bludgeon:
	case WE_Cold:
	case WE_Disease:
	case WE_Pierce:
	case WE_Implant:
	case WE_Slash:
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
		break;
	}
	return false;
}

bool AuCombatWeapon::IsIncendiary2() const
{
	if(DbCommodity::HitMakesExplosionSound(m_CID))
		return true;

	switch(m_w2.m_Effect)
	{
	case WE_Radiation:
	case WE_Electric:
	case WE_Heat:
		return true;
	case WE_Acid:
	case WE_Suffocate:
	case WE_Stun:
	case WE_Poison:
	case WE_Sonic:
	case WE_Crush:
	case WE_Bludgeon:
	case WE_Cold:
	case WE_Disease:
	case WE_Pierce:
	case WE_Implant:
	case WE_Slash:
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
		break;
	}
	return false;
}

void AuCombatWeapon::Magnify(float factor)
{
	m_Range *= factor;
	m_w1.Magnify(factor);
	m_w2.Magnify(factor);
}

