//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucombat.h,v 1.24 2022/02/19 18:18:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCombat_H
#define AuCombat_H

#include "audata.h"
#include "augleffects.h"
#include "auid.h"
#include "aupoint.h"
#include "auspecies.h"
#include "dbcommodity.h"
#include <QString>

class AuItem;
class AuItemAmmo;
class DaAtom;

class AUDATA_DLL AuCombatAttacker
{
private:
	AuSpecies							m_Species1;				// Some attacks do not affect others of the same species.
	AuSpecies							m_Species2;				// We have two in case rider uses mount's special weapon.
public:
	quint32								m_EmpireId;				// May be 0.
	AuId								m_Id;					// May be null. Used for crediting the damage.
	bool								m_IsPirate;				// True if the attacker is a pirate.
	AuPoint								m_MuzzleLoc;			// Firing origin point of the weapon, relative to the attacker.
	QString								m_Name;					// May be empty. Used for crediting the damage.

public:
	inline AuCombatAttacker()
	{
		m_EmpireId = 0;
		m_IsPirate = false;
	}

	inline AuCombatAttacker(const QString& name)
	{
		m_EmpireId = 0;
		m_IsPirate = false;
		m_Name = name;
	}

	inline AuCombatAttacker(const AuId& id, const QString& name, quint32 empireId, bool isPirate=false)
	{
		m_EmpireId = empireId;
		m_Id = id;
		m_IsPirate = isPirate;
		m_Name = name;
	}

	inline AuCombatAttacker(const AuCombatAttacker& c)
	{
		m_Species1 = c.m_Species1;
		m_Species2 = c.m_Species2;
		m_EmpireId = c.m_EmpireId;
		m_Id = c.m_Id;
		m_IsPirate = c.m_IsPirate;
		m_MuzzleLoc = c.m_MuzzleLoc;
		m_Name = c.m_Name;
	}

	inline const AuCombatAttacker& operator=(const AuCombatAttacker& c)
	{
		m_Species1 = c.m_Species1;
		m_Species2 = c.m_Species2;
		m_EmpireId = c.m_EmpireId;
		m_Id = c.m_Id;
		m_IsPirate = c.m_IsPirate;
		m_MuzzleLoc = c.m_MuzzleLoc;
		m_Name = c.m_Name;
		return *this;
	}

public:
	inline bool IsSpeciesExempt(const AuSpecies& species) const
	{
		if(species.IsNull())
			return false;
		return m_Species1 == species || m_Species2 == species;
	}

	inline void SetSpeciesExempt(const AuSpecies& species)
	{
		m_Species1 = species;
	}

	inline void SetSpeciesExempt(const AuSpecies& species1, const AuSpecies& species2)
	{
		m_Species1 = species1;
		m_Species2 = species2;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCombatDefender
{
public:
	DaAtom*								m_Atom;					// The target of the attack.
	AuPoint								m_HitLoc;				// Hit location on the target, relative to the attacker.
	bool								m_HitShield;			// Hit was on shield bubble.
	float								m_Range;				// Distance to the target.

public:
	inline AuCombatDefender()
	{
		m_Atom = 0;
		m_HitShield = false;
		m_Range = 0;
	}

	inline AuCombatDefender(DaAtom* atom, const AuPoint& hitLoc, bool hitShield, float range)
	{
		m_Atom = atom;
		m_HitLoc = hitLoc;
		m_HitShield = hitShield;
		m_Range = range;
	}

	// Constructor for non-specific defender, as when detonating an area effect weapon without targeting anything.
	inline AuCombatDefender(const AuPoint& hitLoc, bool hitShield)
	{
		m_Atom = 0;
		m_HitLoc = hitLoc;
		m_HitShield = hitShield;
		m_Range = 0;
	}

	inline AuCombatDefender(const AuCombatDefender& c)
	{
		m_Atom = c.m_Atom;
		m_HitLoc = c.m_HitLoc;
		m_HitShield = c.m_HitShield;
		m_Range = c.m_Range;
	}

	inline const AuCombatDefender& operator=(const AuCombatDefender& c)
	{
		m_Atom = c.m_Atom;
		m_HitLoc = c.m_HitLoc;
		m_HitShield = c.m_HitShield;
		m_Range = c.m_Range;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCombatResult
{
public:
	quint32								m_AuxArmor1;			// Aux armor effect vs weapon 1.
	quint32								m_AuxArmor2;			// Aux armor effect vs weapon 2.
	quint32								m_BaseArmor1;			// Base armor effect vs weapon 1.
	quint32								m_BaseArmor2;			// Base armor effect vs weapon 2.
	int									m_CriticalHits;			// Number of critical hits.
	quint32								m_Damage1;				// Net damage from weapon 1.
	quint32								m_Damage2;				// Net damage from weapon 2.
	bool								m_IsDestroyed;			// True if the target is destroyed.

public:
	inline AuCombatResult()
	{
		m_AuxArmor1 = 0;
		m_AuxArmor2 = 0;
		m_BaseArmor1 = 0;
		m_BaseArmor2 = 0;
		m_CriticalHits = 0;
		m_Damage1 = 0;
		m_Damage2 = 0;
		m_IsDestroyed = false;
	}

	inline AuCombatResult(const AuCombatResult& c)
	{
		m_AuxArmor1 = c.m_AuxArmor1;
		m_AuxArmor2 = c.m_AuxArmor2;
		m_BaseArmor1 = c.m_BaseArmor1;
		m_BaseArmor2 = c.m_BaseArmor2;
		m_CriticalHits = c.m_CriticalHits;
		m_Damage1 = c.m_Damage1;
		m_Damage2 = c.m_Damage2;
		m_IsDestroyed = c.m_IsDestroyed;
	}

	inline quint32 Armor1() const
	{
		return m_AuxArmor1 + m_BaseArmor1;
	}

	inline quint32 Armor2() const
	{
		return m_AuxArmor2 + m_BaseArmor2;
	}

	// Returns the gross amount of damage represented by this combat result. That is all net damage and all absorbed by shields.
	inline quint32 GrossDamage() const
	{
		return m_Damage1 + m_BaseArmor1 + m_AuxArmor1 + m_Damage2 + m_BaseArmor2 + m_AuxArmor2;
	}

	inline const AuCombatResult& operator=(const AuCombatResult& c)
	{
		m_AuxArmor1 = c.m_AuxArmor1;
		m_AuxArmor2 = c.m_AuxArmor2;
		m_BaseArmor1 = c.m_BaseArmor1;
		m_BaseArmor2 = c.m_BaseArmor2;
		m_CriticalHits = c.m_CriticalHits;
		m_Damage1 = c.m_Damage1;
		m_Damage2 = c.m_Damage2;
		m_IsDestroyed = c.m_IsDestroyed;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCombatWeaponEffect
{
public:
	float								m_AP;
	int									m_Damage;
	int									m_DamageMod;
	WEAPONEFFECT						m_Effect;
	float								m_Radius;
	float								m_ST;

public:	// Constructors
	inline AuCombatWeaponEffect()
	{
		m_AP = 0;
		m_Damage = 0;
		m_DamageMod = 0;
		m_Effect = WE_None;
		m_Radius = 0;
		m_ST = 0;
	}

	inline AuCombatWeaponEffect(const AuCombatWeaponEffect& c)
	{
		m_AP = c.m_AP;
		m_Damage = c.m_Damage;
		m_DamageMod = c.m_DamageMod;
		m_Effect = c.m_Effect;
		m_Radius = c.m_Radius;
		m_ST = c.m_ST;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_AP;
		out << m_Damage;
		out << m_DamageMod;
		out << (quint8)m_Effect;
		out << m_Radius;
		out << m_ST;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_AP;
		in >> m_Damage;
		in >> m_DamageMod;
		quint8 ui8;
		in >> ui8;
		m_Effect = (WEAPONEFFECT)ui8;
		in >> m_Radius;
		in >> m_ST;
	}

public:	// Methods
	// Multiplies the range and damage of this weapon using 'factor'.
	// Used to increase psionic weapon damage when attacker is wearing a preon ring.
	void Magnify(float factor);

	inline int MaxDamage() const
	{
		return (m_Effect == WE_None) ? 0 : (m_Damage + m_DamageMod);
	}

	inline const AuCombatWeaponEffect& operator=(const AuCombatWeaponEffect& c)
	{
		m_AP = c.m_AP;
		m_Damage = c.m_Damage;
		m_DamageMod = c.m_DamageMod;
		m_Effect = c.m_Effect;
		m_Radius = c.m_Radius;
		m_ST = c.m_ST;
		return *this;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuCombatWeaponEffect& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCombatWeaponEffect& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCombatWeapon
{
public:	// Properties
	COMMODITYID							m_CID;
	WEAPONMODE							m_Mode;
	quint8								m_Quality;
	float								m_Range;
	AuCombatWeaponEffect				m_w1;
	AuCombatWeaponEffect				m_w2;

public:	// Construction
	inline AuCombatWeapon(WEAPONMODE mode)
	{
		m_CID = CID_Invalid;
		m_Mode = mode;
		m_Range = 0;
		m_Quality = 1;
	}

	AuCombatWeapon(const AuItem& weapon, INVENTORYSUB sub);

	// 'explosive' is an item that returned true to DoesExplodeWhenDamaged.
	AuCombatWeapon(const AuItem& explosive);

	// 'ammo' is expected to be an AuItemAmmo or a Bomb.
	AuCombatWeapon(const AuItem* ammo, int count);

	inline AuCombatWeapon(const AuCombatWeapon& c)
	{
		m_CID = c.m_CID;
		m_Mode = c.m_Mode;
		m_Quality = c.m_Quality;
		m_Range = c.m_Range;
		m_w1 = c.m_w1;
		m_w2 = c.m_w2;
	}

public:	// Operators
	inline const AuCombatWeapon& operator=(const AuCombatWeapon& c)
	{
		m_CID = c.m_CID;
		m_Mode = c.m_Mode;
		m_Quality = c.m_Quality;
		m_Range = c.m_Range;
		m_w1 = c.m_w1;
		m_w2 = c.m_w2;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)m_CID;
		out << (quint8)m_Mode;
		out << m_Quality;
		out << m_Range;
		out << m_w1;
		out << m_w2;
	}

	inline void operator<<(QDataStream& in)
	{
		quint16 ui16;
		in >> ui16;
		m_CID = (COMMODITYID)ui16;
		quint8 ui8;
		in >> ui8;
		m_Mode = (WEAPONMODE)ui8;
		in >> m_Quality;
		in >> m_Range;
		in >> m_w1;
		in >> m_w2;
	}

public:	// Methods
	inline COMMODITYID CommodityId() const
	{
		return m_CID;
	}

	// Radius of crater in ground if this weapon explodes at ground level. Don't bother if less than 3m.
	float CraterRadius() const;

	// Returns the SOUND_EFFECT of a hit by weapon 1.
	int HitSound1() const;

	// Returns the SOUND_EFFECT of a hit by weapon 2.
	int HitSound2() const;

	// Returns true if weapon 1 can light things on fire.
	bool IsIncendiary1() const;

	// Returns true if weapon 2 can light things on fire.
	bool IsIncendiary2() const;

	// NOTE: This function is called after BestWeaponAtRange has determined that the weapon is effective at the targetRange.
	inline bool IsOptimumRange(float targetRange) const
	{
		switch(m_Mode)
		{
		case WM_Not:
			break;	// Not a weapon.
		case WM_Area:
		case WM_Detonator:
		case WM_Fuse:
		case WM_Projectile:
		case WM_Psionic:
			// Note it is possible to have an area effect weapon with projectile
			// range, e.g. a grenade launcher or artillery shell.
			if(targetRange < m_Range/2.f)
				return true;
			if(targetRange < m_w1.m_Radius/2.f)
				return true;
			if(m_w2.m_Effect != WE_None && targetRange < m_w2.m_Radius/2.f)
				return true;
			break;
		case WM_Contact:
		case WM_Internal:
			return true;	// Any range is optimum for contact weapons and internal damage.
		}
		return false;
	}

	// Multiplies the range and damage of this weapon using 'factor'.
	// Used to increase psionic weapon damage when attacker is wearing a preon ring.
	void Magnify(float factor);

	inline int MaxDamage1() const
	{
		return m_w1.MaxDamage();
	}

	inline int MaxDamage2() const
	{
		return m_w2.MaxDamage();
	}

	inline float MaxRadius() const
	{
		if(m_w1.m_Effect != WE_None && m_w2.m_Effect != WE_None)
			return m_w1.m_Radius + m_w2.m_Radius;
		else if(m_w1.m_Effect != WE_None)
			return m_w1.m_Radius;
		else if(m_w2.m_Effect != WE_None)
			return m_w2.m_Radius;
		return 0.f;
	}

	// Swaps the stats for weapon 1 with the stats for weapon 2.
	inline void SwapWeapons()
	{
		AuCombatWeaponEffect w(m_w1);
		m_w1 = m_w2;
		m_w2 = w;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuCombatWeapon& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCombatWeapon& o)
{
	o << in;
	return in;
}

#endif // AuCombat_H

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
