//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacitizen.h,v 1.325 2022/05/17 15:49:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaCitizen_H
#define DaCitizen_H

#include "dasentient.h"

class AuGear;
class AuGLEffects;

class AUATOM_DLL AuCitizenSound
{
public:
	AuCitizenSound(const QByteArray& soundData, AuSound& al);
	AuCitizenSound();
	AuCitizenSound(const AuCitizenSound& c);

	inline unsigned int AudioBuffer() const
	{
		return m_AudioBuffer;
	}

	inline bool IsPlaying() const
	{
		return m_IsPlaying;
	}

	inline bool IsValid() const
	{
		return m_AudioBuffer ? true : false;
	}

	void StartPlaying()
	{
		m_IsPlaying = true;
	}

private:
	unsigned int						m_AudioBuffer;			// OpenAL audio buffer created from sound data.
	bool								m_IsPlaying;			// true if the sound has started playing
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaCitizen : public DaSentient  
{
protected:
	quint8								m_Lineage;				// LINEAGE_TYPE
	AuId								m_Ship;					// Ship that this citizen is assigned.
	quint8								m_ShipBerthType;		// AuRosterEntry::TYPE

	// Render
	int									m_Culled;				// 1 if culled during AnimateDeviceObjects, 2 if QueryEnv needed
	int									m_InitStand;

private:
	mutable QReadWriteLock				m_GearLock;
	AuGear								m_Gear;					// clothing and equipment
	AuElapsedTimer						m_tmGearChange;
	float								m_WeightCarried;		// Current weight of inventory, in lbs. Not stored in db.

public:
	// Constructors
	DaCitizen(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage);
	DaCitizen(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name, GROWTHSTAGE stage);
	DaCitizen();
	virtual ~DaCitizen();

protected:
	DaCitizen(const DaCitizen &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	// The item will replace an existing item and, if 'dropIt' is true, will cause the replaced item to be dropped if it is used
	// for the same purpose. i.e. a helmet will cause me to drop my current helmet and use the nu helmet instead.
	// 'useIt' is 0=no, 1=yes, 2=only put in dominant hand if it's a weapon
	// 'useIt' puts weapons in my dominant hand or wear clothing and armor.
	// 'useIt' is 0 to put the item somewhere besides my dominant hand, clothing or armor positions.
	// Returns false if unable to add the item.
	virtual bool AddGearItemUseDrop(const AuItem &cItem, int useIt, bool dropIt);

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	// Return the commodity id of the clothing that is outermost for rendering of appearance.
	virtual COMMODITYID Armor() const
	{
		// Clothing is not depicted while I am in a surgery unit, so I don't really have to take it off.
		if(IsPatientStationPose())
			return CID_Invalid;
		if(IsMating())
			return CID_Invalid;

		COMMODITYID c(CID_Invalid);
		m_GearLock.lockForRead();
		if(IsAirBreathable())
			c = m_Gear.ClothingOrBodyArmorWorn();	// Prefer to show clothing (vs env suit) in breathable atmo.
		else
			c = m_Gear.BodyArmorOrClothingWorn();	// Prefer to show armor (env suit) in breathable atmo.
		m_GearLock.unlock();
		return c;
	}

	virtual AuId AssignedShip() const
	{
		return m_Ship;
	}

	virtual AuRosterEntry::TYPE AssignedShipBerthType() const
	{
		return (AuRosterEntry::TYPE)m_ShipBerthType;
	}

	inline void AssignToShip(const AuId& spacecraft, AuRosterEntry::TYPE berthType)
	{
		m_Ship = spacecraft;
		m_ShipBerthType = berthType;
	}

	// Returns current weight of burden in lbs.
	// Includes burden of riders when mounted.
	virtual float Burden() const
	{
		m_GearLock.lockForRead();
		float v = m_WeightCarried + DaSentient::Burden();
		m_GearLock.unlock();
		return v;
	}

	// Return true if this citizen is able to use an inventory item at a specified 'wearPos' in their inventory.
	// This base class insures that the 'item' can be worn at the 'wearPos' inventory position if it is armor or clothing.
	// Items that are not armor or clothing return true if the wearPos is not an armor or clothing inventory position.
	// Does not check the size of items to see if they will fit at a particular inventory position or if that position is occupied.
	virtual bool CanUseGear(const AuItem& item, AuGearSlot::SLOTTYPE slotType) const
	{
		// Gear must be appropriate for the slot type. e.g. rings on fingers, clothes on body, small enough for belt
		return AuGearSlot::CanHoldItem(slotType, item);
	}

	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	virtual void ClearShipAssignment()
	{
		m_Ship = AuId(0);
	}

	// 'attacker' is the atom that is causing the damage. The attacker id should be NULL when the attacker is not a DaUnit, like
	//   when it is the environment or a sun or something.
	// 'weapon' describes the weapon that was used to inflict the damage.
	// 'wpnLoc' is the location of the weapon that is attacking this atom, relative to this atom.
	// 'hitLoc' is the location of the hit on this atom, relative to this atom.
	// 'result' contains the results of the attack, except the IsDestroyed state; this function call sets that.
	// Sets the IsDestroyed result in 'result', if this atom is killed. This is temporal death, not necessarily atomic death.
	// IsDestroyed result is not modified if this atom is already dead.
	// The only thing this base class function does is sends a log message to 'whoId', if it's not NULL.
	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	// Compares my lineage to 'personLineage'.
	// Returns:
	//   -1 if my lineage is less than 'personLineage'
	//    0 if my lineage is equal to 'personLineage'
	//    1 if my lineage is greater than 'personLineage'
	int ComparePersonLineage(LINEAGE_TYPE personLineage) const;

	void Create();

	// Citizens do not depart the solar system sans vehicle or spacecraft.
	virtual bool DepartingSolarSystem();

	virtual void FindCitizensRecursive(DaAtomList& citizens) const;

	virtual const AuGear* LockGearForRead() const
	{
		m_GearLock.lockForRead();
		return &m_Gear;
	}

	virtual AuGear* LockGearForWrite()
	{
		m_GearLock.lockForWrite();
		return &m_Gear;
	}

	virtual void UnlockGear() const
	{
		m_GearLock.unlock();
	}

	// Return the commodity id of the helmet for rendering of appearance.
	virtual COMMODITYID Helmet() const
	{
		// Clothing is not depicted while I am in a surgery unit, so I don't really have to take it off.
		if(IsPatientStationPose())
			return CID_Invalid;
		if(IsMating())
			return CID_Invalid;

		// Don't show helmet in breathable atmosphere, like players when their visor is open.
		if(IsAirBreathable())
			return CID_Invalid;	// Helmet not needed.

		const AuGear* gear = LockGearForRead();
		COMMODITYID c = gear->HelmetOrHatWorn();
		UnlockGear();
		return c;
	}

	virtual void InitGear(AuItemHash& cinv, AuItemHash& itemsTaken, bool equipFully);

	virtual void InitPirateGear(quint8 maxQ);

	// Calculates the current weight of the inventory in pounds.
	inline float GearWeight() const
	{
		m_GearLock.lockForRead();
		float f = m_Gear.Weight();
		m_GearLock.unlock();
		return f;
	}

	// True if my lineage is 100% citizen, which includes citizens spawned nu into the universe.
	inline bool IsLineageCitizen() const
	{
		return m_Lineage == LINEAGE_None || m_Lineage == LINEAGE_Citizen;
	}

	// True if my lineage contains some avatar.
	inline bool IsLineageDemiavatar() const
	{
		return m_Lineage == LINEAGE_Demiavatar;
	}

	virtual bool IsReadyForMating() const;

	virtual bool IsStoryCharacter() const
	{
		return false;
	}

	virtual bool IsTargetToWildAnimal() const
	{
		return true;
	}

	// Called when this unit is killed by a person.
	virtual void KilledByPerson(DaPerson* killer);

	inline LINEAGE_TYPE Lineage() const
	{
		if(m_Lineage == LINEAGE_None)
			return LINEAGE_Citizen;
		return (LINEAGE_TYPE)m_Lineage;
	}

	virtual QString Name() const;

	// Returns an outcry uttered when this passenger is transferred by pirates to the pirate vessel.
	QString OutcryCapturedByPirates() const;

	// Passengers return the bd id from which they were loaded, i.e. airport terminal.
	virtual AuId PortBd() const
	{
		return AuId();
	}

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

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
	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	void SetAmmo(COMMODITYID ammoCID);

	void SetAmmoCount(int count);

	void SetArmor(const AuItem& item);

	void SetClothing(const AuItem& item);

	void SetGear(const AuGear& gear);

	inline void SetLineage(LINEAGE_TYPE lineage)
	{
		m_Lineage = lineage;
	}

	void SetHelmet(const AuItem& item);

	void SetWeapon(const AuItem& item);

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
			int& ammoFull) const;

	virtual void WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const;

public:
	// Rendering functions.
	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

public:
	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	// Return density in lbs/ft^3
	virtual float Density() const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

protected:
	bool AddGearItemUseDropNoLock(const AuItem &cItem, int useIt, bool dropIt);

	// Initializes my inventory from the commodity arrays provided; any of them can be null.
	// Items are taken from the city's inventory.
	// 'weapon' and 'ammo' arrays must be synchronous, each containing 'weaponTypes' entries. Weapons that don't use ammo should
	//   have a corresponding CID_Invalid entry in the 'ammo' array.
	void InitGear(AuItemHash& cinv, AuItemHash& itemsTaken,
			COMMODITYID* weapon, COMMODITYID* ammo, int weaponTypes,
			COMMODITYID* armor, int armorTypes,
			COMMODITYID* helmet, int helmetTypes,
			COMMODITYID* clothing, int clothingTypes,
			COMMODITYID* items, int itemTypes);

	// Initializes my inventory from the commodity arrays provided; any of them can be null.
	// Items are created out of thin air.
	// 'weapon' and 'ammo' arrays must be synchronous, each containing 'weaponTypes' entries. Weapons that don't use ammo should
	//   have a corresponding CID_Invalid entry in the 'ammo' array.
	void InitPirateGear(quint8 maxQ,
			COMMODITYID* weapon, COMMODITYID* ammo, int weaponTypes,
			COMMODITYID* armor, int armorTypes,
			COMMODITYID* helmet, int helmetTypes,
			COMMODITYID* clothing, int clothingTypes,
			COMMODITYID* items, int itemTypes);

	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	// Update my inventory weight.
	// Expects inventory lock to be locked for write.
	virtual void UpdateGearWeight()
	{
		m_tmGearChange.Set();
		m_WeightCarried = m_Gear.Weight();

		// When drifting in space, a gear change could allow me to begin to be pulled by gravity.
		if(!IsInGravity() && !IsInSpacecraft() && !IsInAtmosphere())
			SetInMotion();

		SetModified();
	}

private:
	// Expects inventory to be locked.
	void SetAmmoCountNoLock(int count);

	void SetAmmoCountNoLock(COMMODITYID ammoCID, int count);

	virtual int UseItem(AuItem& item, INVENTORYSUB sub);
};

#endif // DaCitizen_H
