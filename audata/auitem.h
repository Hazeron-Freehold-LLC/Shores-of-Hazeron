//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitem.h,v 1.214 2022/05/09 16:14:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuItem_H
#define AuItem_H

#include "aubddesignexchange.h"
#include "audnabase.h"
#include "aufloradna.h"
#include "aueventlog.h"
#include "augllight.h"
#include "auhair.h"
#include "auknownpatents.h"
#include "aumission.h"
#include "auscdesignexchange.h"
#include "auspecies.h"
#include "dbindustry.h"
#include "deflora.h"
#include <QDataStream>

#include <typeinfo>						// for std::bad_cast
using namespace std;
#include <unordered_map>

class AuAnimalia;
class AuDNABase;
class AuFolliclePoint;
class AuGearPos;
class AuGearPosList;
class AuGLModelCache;
class AuGLModelInstance;
class AuItemContent;
class AuItemContentId;
class AuItemHash;
class AuItemTEUContainer;
class AuKnownPatents;
class AuParticleSystem;
class AuSound;
class DeConstructionProcess;
class DeManufacturingProcess;
class DpBody;

class AUDATA_DLL QPointHash
{
public:
	inline size_t operator()(const QPoint& o) const
	{
		return hash<unsigned int>()(o.x() << 16 | o.y());
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuBountyOnItem
{
	double								m_Amount;				// Amount of bounty. Cleared when grabbed.
	AuId								m_Id;					// Atom id with bounty to redeem. Cleared when grabbed.
	QString								m_Name;					// Atom name with bounty. Cleared when grabbed.
	AuId								m_PosterId;				// Atom id who posted the bounty.
	QString								m_PosterName;			// Atom name who posted the bounty.
	QString								m_Reason;				// Reason for bounty. Cleared when grabbed.

public:
	inline AuBountyOnItem(const AuId& posterId, const QString& posterName, const AuId& id, const QString& name,
			const QString& reason, double amount)
	{
		m_Amount = amount;
		m_Id = id;
		m_Name = name;
		m_PosterId = posterId;
		m_PosterName = posterName;
		m_Reason = reason;
	}

	inline AuBountyOnItem()
	{
		m_Amount = 0;
	}

	inline AuBountyOnItem(const AuBountyOnItem& c)
	{
		m_Amount = c.m_Amount;
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_PosterId = c.m_PosterId;
		m_PosterName = c.m_PosterName;
		m_Reason = c.m_Reason;
	}

public:
	inline const AuBountyOnItem& operator=(const AuBountyOnItem& c)
	{
		m_Amount = c.m_Amount;
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_PosterId = c.m_PosterId;
		m_PosterName = c.m_PosterName;
		m_Reason = c.m_Reason;
		return *this;
	}

	inline bool operator==(const AuBountyOnItem& c) const
	{
		if(m_Amount != c.m_Amount)
			return false;
		if(m_Id != c.m_Id)
			return false;
		if(m_Name != c.m_Name)
			return false;
		if(m_PosterId != c.m_PosterId)
			return false;
		if(m_PosterName != c.m_PosterName)
			return false;
		if(m_Reason != c.m_Reason)
			return false;
		return true;
	}

	inline bool operator!=(const AuBountyOnItem& c) const
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline double Amount() const
	{
		return m_Amount;
	}

	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline const AuId& PosterId() const
	{
		return m_PosterId;
	}

	inline const QString& PosterName() const
	{
		return m_PosterName;
	}

	const QString& Reason() const
	{
		return m_Reason;
	}
};

Q_DECLARE_METATYPE(AuBountyOnItem);	// Needed to use in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuBountyOnItem& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBountyOnItem& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItem
{
public:
	// These correspond 1:1 to the various subclasses of AuItem.
	enum ITEMTYPE
	{
		IT_Invalid = 0,
		IT_Item = 1,
		IT_DNA = 4,
		IT_Mission = 5,
		IT_State = 6,
		IT_Ammo = 7,
		IT_Weapon = 8,
		IT_WeaponAmmo = 9,
		IT_GalaxyLoc = 10,
		IT_WeaponState = 12,
		IT_OfficerLog = 13,
		IT_FloraDNA = 14,
		IT_Picture = 16,
		IT_StateMessage = 17,
		IT_Container = 18,
		IT_ScDesign = 19,
		IT_TEUContainer = 20,
		IT_StorageMedia = 22,				// Like thumb drives, blank disks, storage crystals.
		IT_BdDesign = 23,
		IT_OwnershipTitle = 24,
	};

private:
	enum ITEM_BITS
	{
		ITEM_Unused						= 0x1f,					// 
		ITEM_DropBits					= 0xc0,					// Top two bits are for drop. Default is both bits off.
		ITEM_NoDrop						= 0x80,					// Item is never dropped when the holder dies.
		ITEM_Slippery					= 0x40,					// Item is always dropped when the holder dies.
	};

protected:
	quint16								m_CID;					// CID_ id of the commodity from the database.
	quint32								m_Count;				// Count in this stack. 0 is invalid.
	quint32								m_EmpireOfOrigin;		// Empire id that manufactured this item. If any, may be 0.
	quint8								m_HitPoints;			// Hit points remaining to this item. At 0 the item is broken.
	QString								m_Inscription;			// Inscription on this item.
	quint8								m_Quality;				// Quality of this commodity. 1-255. 0 is reserved. 255 is perfect.
	quint32								m_StoryInstance;		// Story instance this item is associated with. If any, may be 0.
	quint8								m_Bits;					// ITEM_BITS

private:
	// OpenAL
	unsigned int						m_AudioSource;
	int									m_CurrentSound;			// SOUND_EFFECT

	// OpenGL
	mutable AuGLModelInstance*			m_Model;				// 3D model of this item.
	AuParticleSystem*					m_PSFire;				// Particle system for fire effect.
	GLuint								m_TexFire;

public:
	AuItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItem();
	virtual ~AuItem();
	AuItem(const AuItem& c);

public:	// Operators
	const AuItem& operator=(const AuItem& c);

	virtual bool operator==(const AuItem& item) const
	{
		if(Quality() != item.Quality())
			return false;
		if(CommodityId() != item.CommodityId())
			return false;
		if(EmpireOfOrigin() != item.EmpireOfOrigin())
			return false;
		if(HitPoints() != item.HitPoints())
			return false;
		if(Count() != item.Count())
			return false;
		if(Inscription() != item.Inscription())
			return false;
		return true;
	}

	inline bool operator!=(const AuItem& item) const
	{
		return !operator==(item);
	}

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		// Primary sort criteria is size, for pack contents sorting.
		QSize packSize(PackSize());
		QSize iPackSize(item.PackSize());
		if(packSize.width() < iPackSize.width())
			return true;
		else if(packSize.width() > iPackSize.width())
			return false;

		if(packSize.height() < iPackSize.height())
			return true;
		else if(packSize.height() > iPackSize.height())
			return false;

		if(CommodityId() < item.CommodityId())
			return true;
		else if(CommodityId() > item.CommodityId())
			return false;

		if(Quality() < item.Quality())
			return true;
		else if(Quality() > item.Quality())
			return false;

		if(EmpireOfOrigin() < item.EmpireOfOrigin())
			return true;
		else if(EmpireOfOrigin() > item.EmpireOfOrigin())
			return false;

		return false;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Returns the axis down which the item is aimed. 0=positive X, 1=positive Y
	int AimAxis() const;

	// Returns current ammo quantity loaded at 'sub', always <= AmmoQuantity.
	// This is the current amount of bullets in the weapon.
	virtual int AmmoLoaded(INVENTORYSUB /*sub*/) const
	{
		return 0;
	}

	// Returns maximum ammo quantity that can be loaded at 'sub'.
	// This is the amount of bullets when full.
	virtual int AmmoQuantity(INVENTORYSUB /*sub*/) const
	{
		return 0;
	}

	// Returns the fraction of overall damage absorbed by this armor vs a weapon 'effect', in the range 0 to 1.
	inline float ArmorEffectAbsorb(WEAPONEFFECT effect) const
	{
		return Commodity().ArmorEffectAbsorb(effect);
	}

	// Returns the amount of damage absorbed by this armor vs a weapon 'effect', in the range 0 to n.
	inline float ArmorEffectBlock(WEAPONEFFECT effect) const
	{
		return Commodity().ArmorEffectBlock(effect) * QualityBoostF();
	}

	// Returns true if all body parts are completely covered while this item is worn.
	inline bool BareBodyCovered() const
	{
		return BareNeckCovered() &&
			BareUpperTorsoCovered() &&
			BareLowerTorsoCovered() &&

			BareUpperArmCovered() &&
			BareForeArmCovered() &&
			BareHandCovered() &&

			BareCalfCovered() &&
			BareThighCovered() &&
			BareFootCovered();
	}

	// Returns true if bare calves are covered while this item is worn.
	// When true is returned, the 3D model of the body will not include calves, expecting it to be covered by armor or clothing.
	bool BareCalfCovered() const;

	// Returns true if bare feet are completely covered while this item is worn.
	// When true is returned, the 3D model of the body will not include the arm, expecting it to be covered by armor or clothing.
	bool BareFootCovered() const;

	// Returns true if bare fore arms are covered while this item is worn.
	// When true is returned, the 3D model of the body will not include fore arms, expecting it to be covered by armor or clothing.
	bool BareForeArmCovered() const;

	// Returns true if bare hands are completely covered while this item is worn.
	// When true is returned, the 3D model of the body will not include the arm, expecting it to be covered by armor or clothing.
	bool BareHandCovered() const;

	// Returns true if bare lower torso is completely covered while this item is worn.
	// When true is returned, the 3D model of the body will not include the arm, expecting it to be covered by armor or clothing.
	bool BareLowerTorsoCovered() const;

	// Returns true if the bare neck is completely covered while this item is worn.
	// When true is returned, the 3D model of the body will not include the arm, expecting it to be covered by armor or clothing.
	bool BareNeckCovered() const;

	// Returns true if bare thighs are covered while this item is worn.
	// When true is returned, the 3D model of the body will not include thighs, expecting it to be covered by armor or clothing.
	bool BareThighCovered() const;

	// Returns true if bare upper torso is completely covered while this item is worn.
	// When true is returned, the 3D model of the body will not include the arm, expecting it to be covered by armor or clothing.
	bool BareUpperTorsoCovered() const;

	// Returns true if bare upper arms are covered while this item is worn.
	// When true is returned, the 3D model of the body will not include upper arms, expecting it to be covered by armor or clothing.
	bool BareUpperArmCovered() const;

	virtual AuBountyOnItem Bounty() const
	{
		return AuBountyOnItem();
	}

	// Returns true if this item can be combined with other items in a stack. Returns false for non-empty containers, vehicles,
	// things that store DNA, environmental factors, mission disks, design drawings and disks, storage devices, etc.
	virtual bool CanCombine() const;

	// Returns true if this item can stack with 'item' simply by increasing the count. Things that would lose data by doing this
	// should return false, such as a container of items or a TEU.
	// 'item' is assumed to be the same CID and Q as this item. Caller must check that.
	virtual bool CanStack(const AuItem* item) const;

	virtual AuId ClaimantId() const
	{
		return AuId::NullId();
	}

	virtual QString ClaimantName() const
	{
		return QString();
	}

	virtual AuTime ClaimTime() const
	{
		return AuTime();
	}

	// Tests a hair follicle for coverage by this item when it is worn, presumably an article of clothing, armor or helmet.
	// Returns: 0=not covered, 1=partically covered, 2=fully covered
	// 'wearsBra' is used when making underwear.
	int CoverHair(const AuFolliclePoint& fp, bool wearsBra) const;
protected:
	int CoverHairArmor(const AuFolliclePoint& fp) const;	// Partial body coverage.
	int CoverHairClothes(const AuFolliclePoint& fp) const;	// Complete body coverage. Partial arms.
	int CoverHairHat(const AuFolliclePoint& fp) const;		// Partial head coverage.
	int CoverHairHelmet(const AuFolliclePoint& fp) const;	// Complete head coverage, except for face.
	int CoverHairShorts(const AuFolliclePoint& fp) const;	// Scant body coverage, like shorts and a halter top.
	int CoverHairUnderwear(const AuFolliclePoint& fp, bool wearsBra) const;	// Minimal body coverage, like shorts and optional bra.
	int CoverHairUniform(const AuFolliclePoint& fp) const;	// Complete body coverage, including arms.
	int CoverHairVacSuit(const AuFolliclePoint& fp) const;	// Complete body coverage, including hands and feet.

public:
	virtual bool DeleteStoredBlueprintsBd(const QSet<quint32>& designs)
	{
		Q_UNUSED(designs);
		return false;
	}

	virtual bool DeleteStoredBlueprintsSc(const QSet<quint32>& designs)
	{
		Q_UNUSED(designs);
		return false;
	}

	virtual bool DeleteStoredMissions(const QSet<QString>& missions)
	{
		Q_UNUSED(missions);
		return false;
	}

	virtual bool DeleteStoredPapers(const QSet<QString>& paperTexts)
	{
		Q_UNUSED(paperTexts);
		return false;
	}

	virtual bool DeleteStoredPatents(const QSet<COMMODITYID>& patentCIDs)
	{
		Q_UNUSED(patentCIDs);
		return false;
	}

	virtual bool DoesExplodeWhenDamaged() const
	{
		return IsDetonatorWeapon() || IsFuseWeapon();
	}

	inline float QualityBoostF() const
	{
		return AuDef::QualityBoostF(Quality());
	}

	// Returns the value, modified based on my quality. Entirely integer calculation.
	inline int QualityBoosted(int val) const
	{
		return AuDef::QualityBoosted(val, Quality());
	}

	// Returns the fraction of overall damage absorbed by this armor vs a weapon 'mode', in the range 0 to 1.
	virtual float ArmorModeAbsorb(WEAPONMODE mode) const
	{
		return Commodity().ArmorModeAbsorb(mode);
	}

	// Returns the amount of damage absorbed by this armor vs a weapon 'mode', in the range 0 to n.
	virtual float ArmorModeBlock(WEAPONMODE mode) const
	{
		return Commodity().ArmorModeBlock(mode) * QualityBoostF();
	}

	// Armor can be hardened against a certain weapon effect. Damage from this weapon effect causes 1/2 the normal wear on armor.
	inline WEAPONEFFECT ArmorSpecialty() const
	{
		return Commodity().ArmorSpec();
	}

	// Returns the small square resource bitmap id used to represent this
	// component on signs and things.
	inline const QString BitmapName() const
	{
		return Commodity().BitmapName();
	}

	// Returns the size to display the bitmap, based on the PackSize.
	QSize BitmapSize(int baseSize, int sizeStep) const;

	// Returns the size to display the bitmap, so it will fit into a box of 'fitSize', with aspect ratio according to PackSize.
	QSize BitmapSizeToFit(int fitSize) const;

	// Returns a bitmask that is INVENTORYPOS bits OR'd together, identifying the positions at which the item can be worn.
	inline quint32 BitsWear() const
	{
		return Commodity().m_bitsWear;
	}

	virtual float BlastRadius() const
	{
		return 0;
	}

	// True if this item can be stored in a cargo list such as a person's inventory, the inventory of a city, or the hold of a ship.
	inline bool CanBeCarried() const
	{
		return Commodity().CanBeCargo();
	}

	// Returns true if the item is food or liquor, regardless of whether it has any food or intoxicant value.
	inline bool CanBeEaten() const
	{
		return Commodity().CanBeEaten();
	}

	// Returns true if the item is food or liquor, regardless of whether it has any food or intoxicant value.
	inline bool CanBeEaten(ECOROLE ecoRole) const
	{
		if(IsIntoxicant() || IsFood(ecoRole))
			return true;
		return false;
	}

	// True if this is a weapon that is ready to fire/wield. False if out of ammo.
	virtual bool CanFireWeapon(INVENTORYSUB sub) const
	{
		return IsWeapon(sub);
	}

	// Containers such as backpacks and boxes return a number of pack units of maximum capacity.
	virtual quint16 CapacityInPackUnits() const
	{
		return 0;
	}

	virtual AuItemHash* Cargo()
	{
		return 0;
	}

	virtual const AuItemHash* Cargo() const
	{
		return 0;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	// Apply damage to armor caused by 'effect'.
	// 'damage' is the total amount of damage attributed to hitting this armor item, after CombatArmorAbsorption.
	// The actual damage taken by this item is 1/10th of that, 1/20th when absorbing its specialty effect.
	// Returns 'true' if this item is changed.
	// Armor is never destroyed by taking damage. It looks to weird to blast a dude's armor and clothing off bit by bit.
	bool CombatApplyDamage(WEAPONEFFECT effect, int damage);

	// Returns the amount of damage that this armor could absorb when hit by a weapon.
	// 'absorb' is the fraction of overall damage that is absorbed by this armor, in the range 0 to 1.
	// 'block' is the quantity of damage points that are blocked by this armor, in the range 0 to n.
	inline void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, float& absorb, float& block) const
	{
		absorb = ArmorModeAbsorb(mode) + ArmorEffectAbsorb(effect);
		block = ArmorModeBlock(mode) + ArmorEffectBlock(effect);
	}

	// Only return armor hit points if this armor would absorb any of the 'mode' 'effect' hits of damage.
	inline int CombatArmorHits(WEAPONMODE mode, WEAPONEFFECT effect) const
	{
		// Determine if this item absorbs or blocks the weapon mode effect.
		float itemAbsorb, itemBlock;
		CombatArmorEffectiveness(mode, effect, itemAbsorb, itemBlock);
		if(itemAbsorb == 0 && itemBlock == 0)
			return 0;

		// This armor item absorbs damage from the 'mode' 'effect'.
		return HitPoints() * 10;
	}

	// 'item' has already been checked to insure it is the same cid and q as this item.
	virtual void Combine(const AuItem* item, unsigned int count)
	{
		Q_UNUSED(item);
		unsigned int inucount = Count() + count;
		if(inucount > 0x7fffffff)
			inucount = 0x7fffffff;		// Max value.
		SetCount(inucount);
	}

	// Returns the m_combits value from the corresponding DbCommodity record.
	inline quint32 ComBits() const
	{
		return Commodity().m_combits;
	}

	const DbCommodity& Commodity() const;

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	// Returns qty consumed.
	inline quint32 Consume(quint32 iQuantityToConsume)
	{
		if(!iQuantityToConsume)
			return 0;

		int consumed;
		quint32 count = Count();
		if(count > iQuantityToConsume)
		{
			// Only true for items that use m_iData as a quantity value.
			// Otherwise, Count() always returns 1 or zero.
			consumed = iQuantityToConsume;
			quint32 nuCount = count - iQuantityToConsume;
			ScaleCount(nuCount);
		}
		else
		{
			// Entirely consumed item gets invalidated.
			consumed = count;
			SetCount(0);
		}
		return consumed;
	}

	// 'itemLoc' is the location of the item in this container's contents.
	//void ConsumeContainedItem(const QPoint& itemLoc, unsigned int iMaterialConsumed);

	// 'id' is the id of the item in this container's content.
	void ConsumeContainedItem(const AuItemContentId& id, unsigned int iMaterialConsumed);

	virtual AuItemContent* Content()
	{
		return 0;
	}

	virtual const AuItemContent* Content() const
	{
		return 0;
	}

	virtual quint32 CargoVolume() const
	{
		return Count();
	}

	inline quint32 Count() const
	{
		return m_Count;
	}

	static AuItem* Create(quint8 itemType);

	static AuItem* Create(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);

	// Describe a weapon damage effect in a concise one-line string.
	static QString DamageDescription(WEAPONEFFECT effect, int damage, int damageMod, float radius, float ap, float st)
	{
		QString desc = QObject::tr("%1 %2-%3hp damage")
			.arg(EffectName(effect))
			.arg(1 + damageMod)
			.arg(damage + damageMod);

		if(radius > 0.f)
			desc += QObject::tr(", %4m radius")
				.arg(round(radius / FeetPerMeter()));

		if(ap != 0.f || st != 0.f)
			desc += QObject::tr(", armor piercing %1% %2hp")
				.arg(round(ap*100.f))
				.arg(round(st));

		return desc;
	}

	// Reduce the condition of this item by one. Return false when the condition reaches zero.
	// False is returned if condition is zero on entry to this function.
	inline bool DegradeCondition()
	{
		if(m_HitPoints)
			m_HitPoints--;
		return m_HitPoints ? true : false;
	}

	virtual void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	// Returns demand for an item in the range 0.5 <= demand <= 1.5.
	// Used to modulate the CommodityQualityCost of a commodity based on local supply and demand factors.
	inline float Demand(unsigned int population, unsigned int stock, unsigned int needCount, unsigned int produceCount) const
	{
		// Calculate the local demand in the range of .5 to 2.
		if(!stock && !produceCount)
			return 1.5f;		// No stock and no local production results in maximum demand.

		// Determine percentage of population that uses the commodity on an average daily basis.
		float usage = 0;
		if(IsGroceryItem())
			usage = 6.f/7.f;	// Everyone uses grocery items, but not the same one every day.
		else if(IsRetailItem())
			usage = .5f;		// Most everyone uses retail items.
		else if(IsCantinaItem())
			usage = .15f;		// Not that many use cantina items.

		// Calculate demand factor in the range .5 to 1.5.
		float demand = usage * population + needCount;	// 0 <= demand <= (population + needCount).
		float supply = stock + produceCount;			// supply > 0
		float market = (demand - supply) / supply;		// Sign is + when demand exceeds supply, - when supply exceeds demand.
		float demandFactor = atanf(market);				// -PI/2 < demandFactor < PI/2.
		return demandFactor / PIf + 1.f;				// .5 < demandFactor < 1.5.
	}

	virtual quint32 DesignId() const
	{
		return 0;
	}

	// Returns DNA stored by this item.
	virtual const AuDNABase* DNA() const
	{
		return 0;
	}

	virtual GROWTHSTAGE DNAGrowthStage() const
	{
		return Mature;
	}

	// Return true if the DNA() is sentient.
	virtual bool DNAIsSentient() const
	{
		return false;
	}

	virtual bool DNAIsViable() const
	{
		return false;
	}

	// Render all animal and plant DNA in this item and its contents to be unviable.
	virtual void DNAKillRecursive()
	{
	}

	// Caller is giving me 'animalia' because I returned true to DNAWantsAnimaliaModel.
	// I take ownership of the pointer.
	virtual void DNASetAnimaliaModel(AuAnimalia* animalia)
	{
		Q_UNUSED(animalia);
		Q_ASSERT(false);	// Do not call this on this type of object.
	}

	// Return species of the DNA().
	virtual AuSpecies DNASpecies() const
	{
		return AuSpecies();
	}

	// Return true if I am waiting for the animalia model for rendering. i.e. heads.
	virtual bool DNAWantsAnimaliaModel() const
	{
		return false;
	}

	inline quint32 EmpireOfOrigin() const
	{
		return m_EmpireOfOrigin;
	}

	// Uses DgData::EmpireName to return the name associated with m_EmpireOfOrigin.
	QString EmpireNameOfOrigin() const;

	// Return true if any ammo is expended, up to the number of rounds specified.
	virtual bool ExpendAmmo(INVENTORYSUB /*sub*/, unsigned int /*rounds*/)
	{
		return false;
	}

	// Returns true if any materials were consumed.
	bool FetchConstructionMaterials(DeConstructionProcess& cp);

	// Returns false if this is a container AND it is emptied.
	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	bool FetchManufacturingMaterials(DeManufacturingProcess& mp, quint8 elecSubQ, quint8 waterSubQ);

	virtual bool FitItemIntoContainer(const AuItem& /*cItem*/)
	{
		return false;
	}

	// Called from empire report when items need fixing in the database. See usage.
	// If this item must be replaced, a replacement is created and its pointer is returned.
	// If this item does not need to be replaced, 0 is returned.
	AuItem* FixItem() const;

	// Returns AuFloraDNA stored by this item.
	virtual AuFloraDNA FloraDNA() const
	{
		return AuFloraDNA();
	}

	// Returns food nutrition value points for food.
	// The same 'm_food' property is used for IntoxicantValue so be sure to call IsFood first.
	inline short int FoodNutritionValue() const
	{
		// Water IsFood but it does not have any food nutrition value. Quality should not give it any.
		const DbCommodity& bCom = Commodity();
		int food = QualityBoosted(bCom.m_food);
		if(!food)
			return 0;
		return food + m_Quality/10;
	}

	// Food nutrition value of eating this commodity by a certain ecological role.
	inline short int FoodNutritionValue(ECOROLE ecoRole) const
	{
		if(IsFood(ecoRole))
			return FoodNutritionValue();
		return 0;
	}

	// Returns the labor boost that this item can apply when useful.
	// A person emits a standard 4 units of labor.
	static int GetLaborBoost()
	{
		// By supplying 5 units of labor, a tool slightly more than doubles
		// the possible work put out by a person.
		return 5;
	}

	// Returns the production boost that this item can apply when useful.
	static int GetProductionBoost()
	{
		return 1;
	}

	// Returns the distance between the extents and the origin when oriented according to GroundOrientation.
	float GroundHeightAboveCG() const;

	// Returns the ground orientation that corresponds to my GroundUpVector.
	GROUND_ORIENT GroundOrientation() const;

	// Returns the unit vector that corresponds to GroundOrientation.
	AuPoint GroundUpVector() const;

	inline bool HasRenderEffects() const
	{
		switch(CommodityId())
		{
		default:
			// Most items do not render any special effects.
			return false;

		case CID_BurningBarrel:
		case CID_Candle:
		case CID_StoneFirePit:
		case CID_Torch:
			// These items render a flame when they are "on".
			return State() == 1;

		case CID_PreonRingO:
		case CID_PreonRingB:
		case CID_PreonRingA:
		case CID_PreonRingF:
		case CID_PreonRingG:
		case CID_PreonRingK:
		case CID_PreonRingM:
			// Preon rings render the glowing preon, but it does not emit light.
			return true;
		}
	}

	// Returns true if this item can make a continuous sound all the time, whether or not it is currently making any noise.
	bool HasSound() const;

	// Only applicable to items with no weapon usage in IPS_Item.
	// For items with an IPS_Item weapon effect, this is the maximum number of hit points of damage.
	inline qint8 Health() const
	{
		const DbCommodity& bCom = Commodity();
		return QualityBoosted(bCom.m_health);
	}

	inline quint8 HitPoints() const
	{
		return m_HitPoints;
	}

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const
	{
		return HtmlDesc();
	}

	inline void Inscribe(const QString& inscription)
	{
		m_Inscription = inscription;
	}

	inline const QString& Inscription() const
	{
		return m_Inscription;
	}

	// Returns intoxication points for liquor.
	// The same 'm_food' property is used for FoodNutritionValue so be sure to call IsIntoxicant first.
	inline short int IntoxicantValue() const
	{
		const DbCommodity& bCom = Commodity();
		int food = QualityBoosted(bCom.m_food);
		if(!food)
			return 0;
		return food + m_Quality/10;
	}

	// Returns true if this item is ammunition.
	virtual bool IsAmmo() const
	{
		return false;
	}

	inline bool IsArmorOrHelmet() const
	{
		return Commodity().IsArmorOrHelmet();
	}

	inline bool IsArmorHelmetClothingOrHat() const
	{
		return Commodity().IsArmorHelmetClothingOrHat();
	}

	// Returns true for items found in the armory.
	inline bool IsArmoryItem() const
	{
		if(IsAmmo() || IsArmorOrHelmet())
			return true;
		else if(IsWeaponByDesign())
			return true;	// Many tools can be used as weapons but we don't want the armory to look like someone's shop.
		return false;
	}

	// Returns true if this item is currently making a continuous sound all the time.
	bool IsAudible() const;

	// True if this item has a higher quality than 'item'.
	inline int IsBetter(const AuItem& item) const
	{
		return Quality() > item.Quality();
	}

	inline int IsEqualOrBetter(const AuItem& item) const
	{
		return Quality() >= item.Quality();
	}

	inline bool IsBodyArmor() const
	{
		return Commodity().IsBodyArmor();
	}

	inline bool IsBrokerItem() const
	{
		return (ComBits() & CB_Cargo) ? true : false;
	}

	inline bool IsCantinaItem() const
	{
		return (ComBits() & CB_Cantina) ? true : false;
	}

	inline bool IsCarnivoreFood() const
	{
		return DbCommodity::IsCarnivoreFood(CommodityId());
	}

	inline bool IsClothing() const
	{
		return Commodity().IsClothing();
	}

	inline bool IsConstructionMaterial() const
	{
		return DbCommodity::IsConstructionMaterial(CommodityId());
	}

	inline bool IsDetonatorWeapon() const
	{
		const DbCommodity& bCom = Commodity();
		return bCom.m_wpn1Mode == WM_Detonator || bCom.m_wpn2Mode == WM_Detonator;
	}

	virtual bool IsEnvironmentHelmet() const
	{
		return false;
	}

	virtual bool IsEnvironmentSuit() const
	{
		return false;
	}

	inline bool IsFood() const
	{
		return Commodity().IsFood();
	}

	inline bool IsFood(ECOROLE ecoRole) const
	{
		return Commodity().IsFood(ecoRole);
	}

	inline bool IsFreeToFriendlyGovShip() const
	{
		return DbCommodity::IsFreeToFriendlyGovShip(CommodityId());
	}

	inline bool IsFuel() const
	{
		return Commodity().IsFuel();
	}

	inline bool IsFuseWeapon() const
	{
		const DbCommodity& bCom = Commodity();
		return bCom.m_wpn1Mode == WM_Fuse || bCom.m_wpn2Mode == WM_Fuse;
	}

	bool IsGarageItem() const;

	inline bool IsGroceryItem() const
	{
		return (ComBits() & CB_Grocery) ? true : false;
	}

	inline bool IsHat() const
	{
		return Commodity().IsHat();
	}

	inline bool IsHelmet() const
	{
		return Commodity().IsHelmet();
	}

	inline bool IsHengeKey() const
	{
		switch(CommodityId())
		{
		default:
			return false;

		case CID_TypeOPreons:
		case CID_TypeBPreons:
		case CID_TypeAPreons:
		case CID_TypeFPreons:
		case CID_TypeGPreons:
		case CID_TypeKPreons:
		case CID_TypeMPreons:
		case CID_PreonRingO:
		case CID_PreonRingB:
		case CID_PreonRingA:
		case CID_PreonRingF:
		case CID_PreonRingG:
		case CID_PreonRingK:
		case CID_PreonRingM:
			return true;
		}
	}

	inline bool IsHerbivoreFood() const
	{
		return DbCommodity::IsHerbivoreFood(CommodityId());
	}

	inline bool IsIntoxicant() const
	{
		return Commodity().IsIntoxicant();
	}

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		if(Quality() != item.Quality())
			return false;
		if(CommodityId() != item.CommodityId())
			return false;
		if(EmpireOfOrigin() != item.EmpireOfOrigin())
			return false;
		if(HitPoints() != item.HitPoints())
			return false;
		if(Inscription() != item.Inscription())
			return false;
		return true;
	}

	inline bool IsLight() const
	{
		return Commodity().m_light;
	}

	virtual bool IsMedia() const
	{
		return false;
	}

	inline bool IsMedical() const
	{
		return Commodity().IsMedical();
	}

	// By default, all items are droppable on death.
	inline bool IsDroppableOnDeath() const
	{
		return (m_Bits & ITEM_DropBits) != ITEM_NoDrop;
	}

	inline bool IsOmnivoreFood() const
	{
		return DbCommodity::IsOmnivoreFood(CommodityId());
	}

	// Items avialable in the pantry of a ship's galley.
	// This tends to be all things from a grocery except pharmaceuticals.
	inline bool IsPantryItem() const
	{
		if(IsMedical() && CommodityId() != CID_Water)
			return false;	// Pharmacy items are not kept in the pantry, except water.
		return IsGroceryItem();
	}

	inline bool IsPower() const
	{
		return (CommodityId() == CID_Electricity) ? true : false;
	}

	inline bool IsRetailItem() const
	{
		return (ComBits() & CB_Retail) ? true : false;
	}

	inline bool IsScavengerFood() const
	{
		return DbCommodity::IsScavengerFood(CommodityId());
	}

	inline bool IsSlipperyOnDeath() const
	{
		return (m_Bits & ITEM_DropBits) == ITEM_Slippery;
	}

	// Spare parts are those commodities that are replaced on a spacecraft
	// using a tool to repair damage.
	inline bool IsSparePart() const
	{
		return DbCommodity::IsSparePart(CommodityId());
	}

	virtual bool IsStargateKey() const
	{
		return false;
	}

	// Returns true to non-food items that are used to resupply stations.
	// This includes medical supplies, tools, and spare parts.
	inline bool IsSupplyItem() const
	{
		if(Content())
			return true;
		if(IsMedical())
			return true;
		if(IsTool())
			return true;
		if(IsSparePart())
			return true;
		if(IsEnvironmentHelmet() || IsEnvironmentSuit())
			return true;
		return false;
	}

	// Returns true for titles of ownership that have been voided.
	virtual bool IsTitleVoid() const
	{
		return false;
	}

	inline bool IsTool() const
	{
		return Commodity().IsTool();
	}

	inline bool IsValid() const
	{
		return CommodityId() != CID_Invalid;
	}

	// Return true if this item might be found in the wardrobe of a ship's cabin.
	inline bool IsWardrobeItem() const
	{
		return DbCommodity::IsWardrobeItem(CommodityId());
	}

	// Returns true if this item is inherently a weapon. Returns false for other items, including those that could be used as a
	// weapon.
	virtual bool IsWeapon() const
	{
		return false;
	}

	inline bool IsWeapon(INVENTORYSUB sub) const
	{
		return Commodity().IsWeapon(sub);
	}

	inline bool IsWeaponByDesign() const
	{
		return Commodity().IsWeaponByDesign();
	}

	virtual bool IsWeaponThatUsesAmmo(INVENTORYSUB /*sub*/) const
	{
		return false;
	}

	inline bool IsWorn(INVENTORYPOS wearPos) const
	{
		return Commodity().IsWorn(wearPos);
	}

	inline bool IsWornOnBody() const
	{
		return Commodity().IsWornOnBody();
	}

	inline bool IsWornOnDigit() const
	{
		return Commodity().IsWornOnDigit();
	}

	inline bool IsWornOnFace() const
	{
		return Commodity().IsWornOnFace();
	}

	inline bool IsWornOnHead() const
	{
		return Commodity().IsWornOnHead();
	}

	inline bool IsWornOnLimb() const
	{
		return Commodity().IsWornOnLimb();
	}

	inline bool IsWornOnNeck() const
	{
		return Commodity().IsWornOnNeck();
	}

	static quint8 ItemType()
	{
		return IT_Item;
	}

	// Returns the item type associated with a particular commodity id.
	static quint8 ItemType(COMMODITYID cid);

	inline QColor LightColor() const
	{
		switch(CommodityId())
		{
		case CID_Candle:
			return QColor(AuGLLight::ColorCandle());
		case CID_BurningBarrel:
		case CID_StoneFirePit:
		case CID_Torch:
			return QColor(AuGLLight::ColorTorch());
		case CID_Beacon:
			if(AuTime::Now().SecsSinceEpoch() & 1)
				return QColor(AuGLLight::ColorHalogen());
			else
				return QColor(5, 255, 115);
		default:
			return QColor(AuGLLight::ColorHalogen());
		}
	}

	// Returns a value to modulate light intensity for flickering or oscillating lights, i.e. torches, fires and beacons.
	// 'id' is used to skew the randomness, so everybody does not flicker the same.
	float LightFlicker(const AuId& id) const;

	static float LightFlicker(COMMODITYID cid, const AuId& id);

	// Returns the position of the light. This is the flame position on candles and torches.
	inline AuPoint LightPosition() const
	{
		// These positions were measured off the blender model.
		if(CommodityId() == CID_Torch)
			return AuPoint(0, 0, 1.6);
		else if(CommodityId() == CID_Beacon)
			return AuPoint(0, 0, .3);
		else if(CommodityId() == CID_Candle)
			return AuPoint(0, 0, .53);
		else if(CommodityId() == CID_StoneFirePit)
			return AuPoint(0, 0, 3);
		else if(CommodityId() == CID_BurningBarrel)
			return AuPoint(0, 0, 4);
		else
			return AuPoint(0,0);
	}

	// Returns the radius of this light in feet. 0 is returned if this item is not a light.
	inline int LightRadius() const
	{
		const DbCommodity& bCom = Commodity();
		return QualityBoosted(bCom.m_light) * 5;
	}

	// Returns a detailed one-line name and description suitable for a list control.
	// Simply passes EmpireNameOfOrigin to the other ListDesc.
	QString ListDesc() const;

	// Returns a one-line name and description suitable for a list control when the empireOfOrigin name is known but the empire
	// information may not be available locally. This happens when communications are sent to clients that include a commodity.
	QString ListDesc(const QString& empireOfOrigin) const;

	// Names tend to be formatted as 'Milk, Cow', for organized looking lists, with bits of additional information sometimes.
	// Use SayName for informal name formatting such as 'Good Cow Milk', the way a person might say it when speaking.
	// This base function returns g_Au.CommodityName.
	virtual QString ListName() const;

	// Lists this item if its commodity id is a patent.
	virtual void ListPatents(AuKnownPatents& patents) const;

	virtual bool LoadAmmoIntoWeapon(const AuItem& /*item*/)
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Message() const
	{
		return QString();
	}

	// Sets 'min' and 'max' to the 3D extents of this item's model.
	// Returns true if they are set, false if not.
	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	inline quint8 NewHitPoints() const
	{
		return Commodity().m_newcond;
	}

	// Packs this item's item type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	// Returns the size of this object in grid squares when stored in a container. Not at all related to the Pack/Unpack functions.
	// The term 'Pack' infers a backpack here.
	inline QSize PackSize() const
	{
		return Commodity().m_size;
	}

	// Returns the size of this object in total grid squares when stored in a container.
	inline quint8 VolumeInPackUnits() const
	{
		if(!CanBeCarried())
			return 0;
		QSize packSize(PackSize());
		return packSize.width() * packSize.height();
	}

	virtual bool PlantSeedInPot(const AuItem& /*item*/)
	{
		return false;
	}

	// Returns the id of property identified by this item. e.g. spacecraft title of ownership
	virtual AuId PropertyId() const
	{
		return AuId::NullId();
	}

	// Returns a brief description of property identified by this item. e.g. spacecraft title of ownership
	virtual QString PropertyDesc() const
	{
		return QString();
	}

	// Returns the name of property identified by this item. e.g. spacecraft title of ownership
	virtual QString PropertyName() const
	{
		return QString();
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// 'id' is used by flower pots and picture frames as the user id for the texture that is created.
	// 'windVec' is the wind vector in eye space.
	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// 'scale' is used to scale effects when displaying mini models of items with effects.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0, double effectScale=1.);

	// 'model' matrix transforms this object into the scene atom's ear space.
	void RenderSound(const AuMatrix4x4& model, AuSound& al, float gain, float pitch);

	// 'id' is used by flower pots and picture frames as the user id for the texture that is created.
	// 'windVec' is the wind vector in eye space.
	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// 'globeP' is the position of the globe, in eye space.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			const AuPoint& globeP,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// 'id' is used by flower pots and picture frames as the user id for the texture that is created.
	// 'windVec' is the wind vector in eye space.
	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			float shadowPanelSunAngles[MAX_SUNS],
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Removes a contained item's data from the contents of this item.
	//void RemoveContainedItem(const QPoint& itemLoc);

	inline void ResetCondition()
	{
		m_HitPoints = NewHitPoints();
	}

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const
	{
		return QObject::tr("%1 %2").arg(DbCommodity::QualityDesc(Quality())).arg(ListName());
	}

	// Change count and scale contents accordingly.
	virtual void ScaleCount(quint32 count)
	{
		m_Count = count;
	}

	// Sets a 'bounty' on an item. The bounty can be collected by someone in possession of the item.
	// Pass default constructed 'bounty' to clear the bounty after it has been collected.
	virtual void SetBounty(const AuBountyOnItem& bounty)
	{
		Q_UNUSED(bounty);
	}

	inline void SetCommodity(COMMODITYID lCID, unsigned char byQuality)
	{
		Q_ASSERT(byQuality);
		m_CID = lCID;
		m_Quality = byQuality;
	}

	virtual void SetCount(quint32 count)
	{
		m_Count = count;
	}

	// 'galaxyLoc' is a location in the galaxy, in pc.
	virtual void SetGalaxyLoc(const AuId& /*galaxyId*/, const QString& /*galaxyName*/, const AuPoint& /*galaxyLoc*/)
	{
	}

	inline void SetHitPoints(quint8 hp)
	{
		m_HitPoints = hp;
	}

	// 'drop' 0=default random drop, 1=never drop, 2=always drop
	void SetDropOnDeath(int drop);

	virtual void SetMessage(const QString& /*text*/)
	{
	}

	virtual void SetState(quint8 /*state*/)
	{
	}

	inline void SetStoryInstance(quint32 storyInstance)
	{
		m_StoryInstance = storyInstance;
	}

	// Returns dimensions of this item when it exists as a 3D object in the environment.
	inline AuPoint SingleObjectSize() const
	{
		QSize packSize(PackSize());
		return AuPoint(packSize.width()/2.f, packSize.height()/2.f, 1./12.);
	}

	virtual quint8 State() const
	{
		return 0;
	}

	// Attempts to store the data contained in 'dataItem' into this storage media.
	// Returns:
	// 0 = This is not storage media.
	// 1 = The data item exceeds the space available on this storage media.
	// 2 = The data item cannot be stored onto this storage media.
	// 3 = Source item patent stored successfully.
	// 4 = Source item spacecraft blueprint stored successfully.
	// 5 = Source item building blueprint stored successfully.
	// 6 = Source item data transferred.
	// 7 = Source item patent (or better) already contained on this storage media.
	// 8 = Source item blueprint already contained on this storage media.
	// 9 = Data item cannot be copied due to publisher restrictions.
	// 10 = Copy restrictions on an item in the 'dataItem' storage container prevent the copy operation.
	// 11 = Source item mission stored successfully.
	// 12 = Source item paper is blank.
	// 13 = Source item paper already contained on this storage media.
	// 14 = Source item paper stored successfully.
	virtual int StoreData(const AuItem* dataItem)
	{
		Q_UNUSED(dataItem);
		return 0;	// This is not storage media.
	}

	QString StoryDebug() const;

	inline quint32 StoryInstance() const
	{
		return m_StoryInstance;
	}

	// Calculates the failure rate of a certain quantity of this commodity, based on quality.
	// Returns the number of units of 'count' that do NOT fail due to quality failure rate.
	// The number returned will never exceed 'count'.
	inline quint32 TestFailure(quint32 count) const
	{
		return DbCommodity::TestFailure(m_Quality, count);
	}

	// Performs a random test to determine failure of a single item due to quality.
	// Returns true if the commodity did not fail.
	// Returns false if the commodity failed.
	inline bool TestQuality() const
	{
		return DbCommodity::TestQuality(Quality());
	}

	// Returns the cost of the total count of this commodity based on its quality.
	// Use UnitCost to get the cost of a single item.
	inline float TotalCost() const
	{
		return UnitCost() * Count();
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	// Returns the cost of a single one of this commodity based on its quality.
	// Use TotalCost to get the cost multiplied by the count.
	virtual float UnitCost() const;

	// Restores an item from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static AuItem* Unpack(const QByteArray& ba);

	// "Use" this item once. Return false if this item is broken by this usage.
	inline bool UseItem()
	{
		if(!m_HitPoints)
			return false;

		// Item is not already broken.
		if(TestQuality())
			return true;	// Item did not fail its quality check.

		// Item failed quality check so degrade its condition.
		return DegradeCondition();
	}

	// Voids all ownership titles that refer to 'propertyId'.
	// Returns true if anything changes.
	virtual bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
	{
		Q_UNUSED(propertyId);
		Q_UNUSED(claimantId);
		Q_UNUSED(claimantName);
		return false;
	}

	// Collects information about this items weapon usage.
	virtual void WeaponInfo(INVENTORYSUB sub,
			COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
			WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
			WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const;

	// Recursively determines the weight of this item and all subordinate items if this is a container.
	virtual float Weight() const
	{
		return Commodity().m_wt;
	}

protected:
	void RenderCandleFlame(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale);

	void RenderEffects(const AuMatrix4x4& projections, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& gravityUp,
			double effectScale);

	void RenderFire(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale, double radius);

	void RenderPreon(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale,
			const QColor& color);

	void RenderTorchFlame(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale);
};

inline QDataStream& operator<<(QDataStream& out, const AuItem& item)
{
	item >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItem& item)
{
	item << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Container classes
// Typical item container is hashed by COMMODITYID.

class AuItemHash : public unordered_multimap<COMMODITYID, AuItem*, COMMODITYID_Hash>
{
public:	// Constructors
	AuItemHash()
	{
	}
	AUDATA_DLL AuItemHash(const AuItemHash& c);
	AUDATA_DLL ~AuItemHash()
	{
		Clear();
	}

public:	// Operators
	AUDATA_DLL const AuItemHash& operator=(const AuItemHash& c);

	AUDATA_DLL bool operator==(const AuItemHash& c) const;

	inline bool operator!=(const AuItemHash& c) const
	{
		return !operator==(c);
	}

	AUDATA_DLL void operator>>(QDataStream& out) const;

	AUDATA_DLL void operator<<(QDataStream& in);

	AUDATA_DLL AuItemHash operator-(const AuItemHash& c) const;

public:	// Methods
	// Adds a commodity by increasing quantity of existing entry if possible.
	inline void AddItem(const AuItem& cAdd)
	{
		AddItemCount(cAdd, cAdd.Count());
	}

	// Adds 'count' of a commodity by increasing quantity of existing entry if possible.
	inline iterator AddItemCount(const AuItem& cAdd, unsigned int count)
	{
		if(count > cAdd.Count())
		{
			qWarning() << "AuItemHash::AddItem item" << cAdd.CommodityId() << "count" << count
				<< "exceeds item count" << cAdd.Count();
			return end();
		}

		// Look for existing commodity/quality item.
		iterator addI = Find(cAdd);
		if(addI != end() && cAdd.CanStack((*addI).second))
		{
			(*addI).second->Combine(&cAdd, count);	// An existing commodity was found.
			return addI;
		}
		addI = insert(pair<COMMODITYID, AuItem*>(cAdd.CommodityId(), cAdd.Clone()));
		if((*addI).second->Count() != count)
			(*addI).second->ScaleCount(count);
		return addI;
	}

	// Adds 'count' of an item then insures the total count of that commodity does not exceed storageLimit, regardless of quality.
	inline void AddItemCountLimit(const AuItem& cAdd, int count, int storageLimit)
	{
		// Add 'count' of the nu item.
		AddItemCount(cAdd, count);

		quint32 rc = RawCount(cAdd.CommodityId());
		if(rc <= (quint32)storageLimit)
			return;

		// Trim the commodity to the maximum storage limit of the city by discarding the worst of the excess stuff.
		quint32 excess = rc - storageLimit;			// Quantity of the commodity that exceeds storage.
		while(excess > 0)
		{
			iterator it = FindWorst(cAdd.CommodityId());
			if(it == end())
				break;
			// Consume as much of the excess as possible from the worst commodity.
			excess -= (*it).second->Consume(excess);
			if((*it).second->Count() == 0)
				Erase(it);
		}
	}

	// Adds an item then insures the total count of that commodity does not exceed storageLimit, regardless of quality.
	inline void AddItemLimit(const AuItem& cAdd, int storageLimit)
	{
		// Add the nu item.
		AddItem(cAdd);

		quint32 rc = RawCount(cAdd.CommodityId());
		if(rc <= (quint32)storageLimit)
			return;

		// Trim the commodity to the maximum storage limit of the city by discarding the worst of the excess stuff.
		quint32 excess = rc - storageLimit;			// Quantity of the commodity that exceeds storage.
		while(excess > 0)
		{
			iterator it = FindWorst(cAdd.CommodityId());
			if(it == end())
				break;
			// Consume as much of the excess as possible from the worst commodity.
			excess -= (*it).second->Consume(excess);
			if((*it).second->Count() == 0)
				Erase(it);
		}
	}

	// Adds all 'items' to this hash.
	AUDATA_DLL void AddItems(const AuItemHash& items);

	// Adds unrestricted 'items' to this hash. Restriction is determined by checking commerce restriction on each commodity
	// from 'itemEmpire' to 'thisEmpire'.
	AUDATA_DLL void AddNotRestricted(const AuItemHash& items, quint32 itemEmpire, quint32 thisEmpire);

	// Returns number of different food commodities available to the ecological role.
	inline int AvailableFoodTypes(ECOROLE ecoRole) const
	{
		// Take inventory of the different food types available in this inventory array.
		QSet<COMMODITYID> afoods;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsFood(ecoRole) && (*it).second->FoodNutritionValue())
				afoods.insert((*it).second->CommodityId());
		return afoods.count();
	}

	// Total count of items that have a higher quality than 'item'.
	inline int BetterCount(const AuItem& item) const
	{
		int count = 0;
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
			if((*it).second->IsBetter(item))
				count += (*it).second->Count();
		return count;
	}

	// Total count of items that have a higher quality than 'item'.
	inline int EqualOrBetterCount(const AuItem& item) const
	{
		int count = 0;
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
			if((*it).second->IsEqualOrBetter(item))
				count += (*it).second->Count();
		return count;
	}

	// Test to see if an item can be put into this inventory.
	// This function is only called for city inventory and spacecraft cargo.
	// Returns false if the entire item cannot be put into this inventory.
	inline bool CanAddItem(const AuItem& cItem) const
	{
		if(!cItem.IsValid())
			return false;

		// Insure the stack of these items isn't too big.
		const_iterator itemI = Find(cItem);
		if(itemI != cend())
		{
			if((*itemI).second->Count() + cItem.Count() >= 0x7fffffff)
				return false;	// Count is full. No more of that commodity is allowed in this inventory.
		}
		return true;
	}

	void Clear()
	{
		for(iterator ii = begin(); ii != end(); ii++)
			if((*ii).second)
				delete (*ii).second;
		clear();
	}

	// Consumes a quantity of an item. Returns quantity successfully consumed.
	inline int ConsumeCommodity(COMMODITYID cid, unsigned int iQuantity)
	{
		int consumed = 0;
		iterator it = find(cid);
		while(it != end() && (*it).first == cid && iQuantity)
		{
			// Consume a quantity of the item.
			int consitem = (*it).second->Consume(iQuantity);
			if(consitem)
			{
				// Some of the item was consumed. Track totals.
				iQuantity -= consitem;
				consumed += consitem;
			}

			// Remove items that get fully consumed.
			if((*it).second->Count() == 0)
				it = Erase(it);					// Used up the item.
			else
				it++;							// Skip to the next item.
		}

		// Return the total amount actually consumed.
		return consumed;
	}

	// Returns true if this hash contains any key 'cid'.
	inline bool contains(COMMODITYID cid) const
	{
		return find(cid) != cend();
	}

	// Finds a commodity using operator== test.
	// return true if one is found.
	inline bool ContainsItem(const AuItem& item) const
	{
		return FindItem(item) != cend();
	}

	// Returns the number of pack units of items contained in this list.
	AUDATA_DLL quint16 ContainsPackUnits() const;

	// Discards inventory items that are determined to be obsolete.
	// Air and water are not discarded when 'hasBreathableAtmo' is false
	// Food is not discarded. 'ecoRole' determines what is food.
	// 'storageCap' is the storage capacity of the city, so it doesn't throw away stuff when it has little of the better stuff.
	AUDATA_DLL void DiscardObsoleteItems(const QString& ownerName, bool hasBreathableAtmo, ECOROLE ecoRole,
			quint32 storageCap);

	// Consumes food and drinks from available food resources.
	// 'foodToEat' is the amount of food nutrition value to consume, not the number of food commodities.
	// 'ecoRole' determines the commodities that can be eaten as food
	// return value is the amount of food nutrition value consumed, not the number of food commodities.
	// 'consumedLiquor' is set true if any liquor is consumed while eating food.
	AUDATA_DLL int EatFood(unsigned int foodToEat, ECOROLE ecoRole, bool& consumedLiquor);

	// Consumes food from available food resources.
	// 'foodQty' is the amount of food commodity units to consume.
	// 'ecoRole' determines the commodities that can be eaten as food
	// return value is the amount of food commodities consumed.
	AUDATA_DLL int EatFood(unsigned int foodQty, ECOROLE ecoRole);

	// Consumes food and drinks from available food resources.
	// 'foodToEat' is the amount of food nutrition value to consume, not the number of food commodities.
	// 'ecoRole' determines the commodities that can be eaten as food
	// 'liquorToDrink' is the amount of intoxication value to consume.
	// return value is the amount of food nutrition value consumed, not the number of food commodities.
	// 'liquorDrank' is a return parameter set to the amount of intoxication value consumed.
	AUDATA_DLL int EatMeal(unsigned int foodToEat, ECOROLE ecoRole, unsigned int liquorToDrink, unsigned int& liquorDrank);

	// Delete the item as it is erased.
	iterator Erase(iterator pos)
	{
		delete (*pos).second;
		return erase(pos);
	}

	// Export commodities.
	// Returns true if any exported.
	// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
	// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
	// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
	bool ExportBrokerItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
			POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna);

	// Export food and spirits.
	// Returns true if any exported.
	// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
	// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
	// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
	bool ExportCantinaItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
			POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna);

	// Export food.
	// Returns true if any exported.
	// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
	// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
	// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
	bool ExportGroceryItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
			POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna);

	// Export non-food commodities.
	// Returns true if any exported.
	// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
	// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
	// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
	bool ExportRetailItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
			POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna);

	// Export items from this inventory to a station's inventory 'stationInv'.
	// Returns true if any exported.
	// 'percent' is the maximum percent of each kind of commodity in this inventory to export to the station.
	// 'foodSpace' is empty cargo space available for food on station.
	// 'supplySpace' is empty cargo space available for supplies on station.
	// 'munitionSpace' is empty cargo space available for ammunition on station.
	// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
	// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
	AUDATA_DLL bool ExportToStation(qint32 exporterPopulation, AuItemHash& stationInv, quint32 percent, quint32 foodSpace,
			quint32 supplySpace, quint32 munitionSpace, bool planetCanHaveFlora, bool planetCanHaveFauna);

	// Finds a commodity using IsItem test, vs true equality test.
	// return 'end()' on failure.
	inline iterator Find(const AuItem& item)
	{
		for(iterator it = find(item.CommodityId()); it != end() && (*it).first == item.CommodityId(); it++)
			if((*it).second->IsItem(item))
				return it;
		return end();
	}

	// Finds a commodity using IsItem test, vs true equality test.
	// return 'cend()' on failure.
	inline const_iterator Find(const AuItem& item) const
	{
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
			if((*it).second->IsItem(item))
				return it;
		return cend();
	}

	inline iterator FindBest(COMMODITYID cid)
	{
#if !defined(QT_NO_DEBUG)
		if(cid == CID_Invalid)
		{
			qDebug("AuItemHash::FindBest called with CID_Invalid");
			return end();
		}
#endif
		quint8 bestQ = 0;
		iterator bestIt = end();
		for(iterator it = find(cid); it != end() && (*it).first == cid; it++)
		{
			if((*it).second->Quality() > bestQ)
			{
				bestQ = (*it).second->Quality();
				bestIt = it;
			}
		}
		return bestIt;
	}

	inline const_iterator FindBest(COMMODITYID cid) const
	{
#if !defined(QT_NO_DEBUG)
		if(cid == CID_Invalid)
		{
			qDebug("AuItemHash::FindBest const called with CID_Invalid");
			return cend();
		}
#endif
		quint8 bestQ = 0;
		const_iterator bestIt = cend();
		for(const_iterator it = find(cid); it != cend() && (*it).first == cid; it++)
		{
			if((*it).second->Quality() > bestQ)
			{
				bestQ = (*it).second->Quality();
				bestIt = it;
			}
		}
		return bestIt;
	}

	// Finds the best commodity of a given type manufactured by a specific empire.
	// return 'end()' on failure.
	inline const_iterator FindBest(COMMODITYID cid, quint32 empireOfOrigin) const
	{
#if !defined(QT_NO_DEBUG)
		if(cid == CID_Invalid)
		{
			qDebug() << "AuItemHash::FindBest const called with CID_Invalid, empireOfOrigin" << empireOfOrigin;
			return cend();
		}
#endif
		quint8 bestQ = 0;
		const_iterator bestIt = cend();
		for(const_iterator it = find(cid); it != cend() && (*it).first == cid; it++)
		{
			if((*it).second->EmpireOfOrigin() != empireOfOrigin)
				continue;
			if((*it).second->Quality() > bestQ)
			{
				bestQ = (*it).second->Quality();
				bestIt = it;
			}
		}
		return bestIt;
	}

	inline iterator FindBest(COMMODITYID cid, quint32 empireOfOrigin)
	{
#if !defined(QT_NO_DEBUG)
		if(cid == CID_Invalid)
		{
			qDebug() << "AuItemHash::FindBest called with CID_Invalid, empireOfOrigin" << empireOfOrigin;
			return end();
		}
#endif
		quint8 bestQ = 0;
		iterator bestIt = end();
		for(iterator it = find(cid); it != end() && (*it).first == cid; it++)
		{
			if((*it).second->EmpireOfOrigin() != empireOfOrigin)
				continue;
			if((*it).second->Quality() > bestQ)
			{
				bestQ = (*it).second->Quality();
				bestIt = it;
			}
		}
		return bestIt;
	}

	// Finds a commodity using operator== test.
	// return 'end()' on failure.
	inline iterator FindItem(const AuItem& item)
	{
		for(iterator it = find(item.CommodityId()); it != end() && (*it).first == item.CommodityId(); it++)
			if((*it).second->operator==(item))
				return it;
		return end();
	}

	// Finds a commodity using operator== test.
	// return 'cend()' on failure.
	inline const_iterator FindItem(const AuItem& item) const
	{
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
			if((*it).second->operator==(item))
				return it;
		return cend();
	}

	// Finds the worst commodity of a given type.
	// return 'end()' on failure.
	inline iterator FindWorst(COMMODITYID cid)
	{
#if !defined(QT_NO_DEBUG)
		if(cid == CID_Invalid)
		{
			qDebug("AuItemHash::FindWorst called with CID_Invalid");
			return end();
		}
#endif
		unsigned int worstQ = 255+1;
		iterator worstIt = end();
		for(iterator it = find(cid); it != end() && (*it).first == cid; it++)
		{
			if((*it).second->Quality() < worstQ)
			{
				worstQ = (*it).second->Quality();
				worstIt = it;
			}
		}
		return worstIt;
	}

	// Called from empire report when items need fixing in the database. See usage.
	void FixItems();

	// Returns true if inventory contains at least one item that IsEnvironmentHelmet and one item that IsEnvironmentSuit.
	bool HasEnvironmentHelmetAndSuit() const;

	inline bool HasFood(ECOROLE ecoRole) const
	{
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsFood(ecoRole) && (*it).second->FoodNutritionValue())
				return true;
		return false;
	}

	AUDATA_DLL bool HasLiquor() const;

	// Returns true if item is obsolete.
	// Air and water are never obsolete when 'hasBreathableAtmo' is false
	// Food is never obsolete. 'ecoRole' determines what is food.
	// 'storageCap' is the storage capacity of the city, so it doesn't throw away stuff when it has little of the better stuff.
	AUDATA_DLL bool IsObsoleteItem(const AuItem& item, bool hasBreathableAtmo, ECOROLE ecoRole, quint32 storageCap) const;

	AUDATA_DLL quint64 MemoryUsage() const;

	// Raw total of a commodity, ignoring different qualities.
	inline quint32 RawCount(COMMODITYID cid) const
	{
		quint32 count = 0;
		for(const_iterator it = find(cid); it != cend() && (*it).first == cid; it++)
			count += (*it).second->Count();
		return count;
	}

	// Raw total of a commodity of a minimum quality.
	inline quint32 RawCount(COMMODITYID cid, quint8 minQ) const
	{
		quint32 count = 0;
		for(const_iterator it = find(cid); it != cend() && (*it).first == cid; it++)
			if((*it).second->Quality() >= minQ)
				count += (*it).second->Count();
		return count;
	}

	// Removes the first 'count' quantity of a commodity that is in this array, ignoring quality.
	// Returns the number of items that were actually removed.
	inline int RawRemove(COMMODITYID cid, quint32 count)
	{
		int removed = 0;
		if(count)
		{
			iterator it = find(cid);
			while(it != end() && (*it).first == cid)
			{
				// Iterator is pointing at the kind of commodity we are to remove.
				if((*it).second->Count() > count)
				{
					// This entry exceeds the quantity left to remove.
					removed += count;
					(*it).second->Consume(count);
					break;	// That's all.
				}
				else
				{
					// Reduce the count left to remove.
					removed += (*it).second->Count();;
					count -= (*it).second->Count();

					// Remove the used-up commodity.
					it = Erase(it);

					// This could be all of them.
					if(!count)
						break;	// That's all.
				}
			}
		}
		return removed;
	}

	// Removes the first 'count' quantity of a commodity that is in this array of a minimum quality.
	// Returns the number of items that were actually removed.
	inline int RawRemove(COMMODITYID cid, quint8 minQ, quint32 count)
	{
		int removed = 0;
		if(count)
		{
			iterator it = find(cid);
			while(it != end() && (*it).first == cid)
			{
				// Iterator is pointing at the kind of commodity we are to remove.
				if((*it).second->Quality() >= minQ)
				{
					// This is the quality of commodity we are to remove.
					if((*it).second->Count() > count)
					{
						// This entry exceeds the quantity left to remove.
						removed += count;
						(*it).second->Consume(count);
						break;	// That's all.
					}
					else
					{
						// Reduce the count left to remove.
						removed += (*it).second->Count();;
						count -= (*it).second->Count();

						// Remove the used-up commodity.
						it = Erase(it);

						// This could be all of them.
						if(!count)
							break;	// That's all.
					}
				}
				else
				{
					// Skip other qualities of the commodity.
					it++;
				}
			}
		}
		return removed;
	}

	// Removes an item by exact match of the item.
	inline void RemoveItem(const AuItem& item)
	{
		iterator ii = FindItem(item);
		if(ii != end())
			Erase(ii);
	}

	// A recipient of 'container' default on their ability to pay when it was delivered. The carrier is free to keep the shipment.
	// Returns true if the shipment that matches 'container' is found and released.
	bool ShipmentDefaulted(const AuItemTEUContainer& container);

	// Returns the count that is actually taken.
	inline quint32 TakeItem(const AuItem& iTake)
	{
		return TakeItem(iTake, iTake.Count());
	}

	// Returns the count that is actually taken.
	inline quint32 TakeItem(const AuItem& iTake, quint32 howMany)
	{
		quint32 taken = 0;
		iterator it = find(iTake.CommodityId());
		while(it != end() && (*it).first == iTake.CommodityId() && howMany)
		{
			if((*it).second->IsItem(iTake))
			{
				if((*it).second->Count() <= howMany)
				{
					// The inventory item at 'index' is completely removed.
					taken += (*it).second->Count();
					howMany -= (*it).second->Count();
					it = Erase(it);
				}
				else
				{
					// The inventory item at 'index' is reduced.
					(*it).second->ScaleCount((*it).second->Count() - howMany);
					taken += howMany;
					howMany = 0;
					it++;
				}
			}
			else
				it++;
		}
		return taken;
	}

	// The number returned is roughly in cubic feet.
	inline quint32 TotalCargoVolume() const
	{
		quint32 cargoVol = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			cargoVol += (*it).second->CargoVolume();
		return cargoVol;
	}

	inline quint32 TotalFoodUnitKinds() const
	{
		// Tally the different supply units available in this inventory array.
		// This is for stations, which want all cantina and pantry items, regardless of ecological role.
		// Use AvailableFoodTypes for an IsFood test on each commodity.
		QSet<COMMODITYID> unitKinds;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsCantinaItem() || (*it).second->IsPantryItem())
				unitKinds.insert((*it).second->CommodityId());
		return unitKinds.count();
	}

	// Tally the different food units available in this inventory array, edible to ANY ecological role.
	inline quint32 TotalFoodUnits() const
	{
		quint32 available = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
				available += (*it).second->Count();
		return available;
	}

	inline int TotalFoodUnits(ECOROLE ecoRole) const
	{
		// Tally the different food units available in this inventory array, edible to the ecological role.
		int available = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsFood(ecoRole) && (*it).second->FoodNutritionValue())
				available += (*it).second->Count();
		return available;
	}

	inline quint32 TotalFoodNutritionValue(ECOROLE ecoRole) const
	{
		// Tally the different food units available in this inventory array, edible to the ecological role.
		quint32 available = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
		{
			if(!(*it).second->IsFood(ecoRole))
				continue;
			short int FoodNutritionValue = (*it).second->FoodNutritionValue();
			if(!FoodNutritionValue)
				continue;
			available += (*it).second->Count() * FoodNutritionValue;
		}
		return available;
	}

	inline quint32 TotalMunitionUnitKinds() const
	{
		// Tally the different supply units available in this inventory array.
		QSet<COMMODITYID> unitKinds;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsWeapon() || (*it).second->IsAmmo())
				unitKinds.insert((*it).second->CommodityId());
		return unitKinds.count();
	}

	inline quint32 TotalMunitionUnits() const
	{
		// Tally the different weapon and ammo units available in this inventory array.
		quint32 available = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsWeapon() || (*it).second->IsAmmo())
				available += (*it).second->Count();
		return available;
	}

	inline quint32 TotalSupplyUnitKinds() const
	{
		// Tally the different supply units available in this inventory array.
		QSet<COMMODITYID> unitKinds;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsSupplyItem())
				unitKinds.insert((*it).second->CommodityId());
		return unitKinds.count();
	}

	inline quint32 TotalSupplyUnits() const
	{
		// Tally the different supply units available in this inventory array.
		quint32 available = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			if((*it).second->IsSupplyItem())
				available += (*it).second->Count();
		return available;
	}

	// Total units stored at the top level in this inventory.
	inline quint32 TotalUnits() const
	{
		quint32 units = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			units += (*it).second->Count();
		return units;
	}

	// Returns the sum of the TotalCost of all my items.
	double Value() const;

	bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);

	inline float Weight() const
	{
		float weight = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			weight += (*it).second->Weight();
		return weight;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuItemHash& itemHash)
{
	itemHash >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItemHash& itemHash)
{
	itemHash << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemContentId
{
private:	// Properties
	quint8								m_Id;					// 0 is considered to be invalid.

public:	// Constructors
	inline AuItemContentId(quint8 id)
	{
		m_Id = id;
	}

	inline AuItemContentId()
	{
		m_Id = 0;
	}

	inline AuItemContentId(const AuItemContentId& c)
	{
		m_Id = c.m_Id;
	}

public:	// Operators
	inline const AuItemContentId& operator=(const AuItemContentId& c)
	{
		m_Id = c.m_Id;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Id;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Id;
	}

	inline bool operator==(const AuItemContentId& o) const
	{
		return m_Id == o.m_Id;
	}

	inline bool operator!=(const AuItemContentId& o) const
	{
		return !operator==(o);
	}

	inline bool operator<(const AuItemContentId& o) const
	{
		return m_Id < o.m_Id;
	}

	inline bool operator>(const AuItemContentId& o) const
	{
		return m_Id > o.m_Id;
	}

public:	// Methods
	inline quint8 Id() const
	{
		return m_Id;
	}

	inline bool IsValid() const
	{
		return m_Id;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuItemContentId& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItemContentId& o)
{
	o << in;
	return in;
}

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuItemContentId& id);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemContentIdHash
{
public:
	inline size_t operator()(const AuItemContentId& o) const
	{
		return hash<unsigned int>()(o.Id());
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuItemContent : public map<AuItemContentId, AuItem*>
{
public:	// Constructors
	AuItemContent()
	{
	}

	AUDATA_DLL AuItemContent(const AuItemContent& c);

	~AuItemContent()
	{
		Clear();
	}

public:	// Operators
	AUDATA_DLL void operator>>(QDataStream& out) const;

	AUDATA_DLL void operator<<(QDataStream& in);

	bool operator==(const AuItemContent& c) const;

	inline bool operator!=(const AuItemContent& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline void Clear()
	{
		for(iterator ii = begin(); ii != end(); ii++)
			if((*ii).second)
				delete (*ii).second;
		clear();
	}

	// Returns the number of pack units of items contained in this list.
	AUDATA_DLL quint8 ContainsPackUnits() const;

	void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	// Recursively destructs items associated with stories that are no longer active.
	// Returns true if any items are destroyed.
	AUDATA_DLL bool DestructStoryItems();

	// Delete the item as it is erased.
	/*
	inline void Erase(quint8 index)
	{
		iterator ii = begin() + index;
		Erase(ii);
	}
	*/

	// The first item in the contentPath is found in this list. If it is the last/only item in contentPath, the item is removed.
	inline void Erase(const QList<AuItemContentId>& contentPath)
	{
		if(contentPath.isEmpty())
			return;

		// Locate the first item in the contentPath. It should be in this container.
		iterator ii = find(contentPath.first());
		if(ii == end())
			return;

		if(contentPath.size() == 1)
		{
			// The last item in the path is erased.
			Erase(ii);
		}
		else
		{
			// The next item should be a container.
			AuItemContent* c = (*ii).second->Content();
			if(!c)
				return;	// Error; this should be a container.

			// Remove the first item from the path and recurse into that container.
			QList<AuItemContentId> subPath(contentPath);
			subPath.pop_front();
			c->Erase(subPath);
		}
	}

	// Recursive.
	AuGearPos FindBest(const AuGearPos& parentPos, COMMODITYID cid, quint8& bestQ) const;

	// Recursive.
	AuGearPos FindItemPos(const AuGearPos& parentPos, const AuItem& item) const;

	// Recursive
	inline bool HasFood(ECOROLE ecoRole) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).second->IsFood(ecoRole) && (*ii).second->FoodNutritionValue())
				return true;

			const AuItemContent* c = (*ii).second->Content();
			if(c && c->HasFood(ecoRole))
				return true;
		}
		return false;
	}

	// Recursive.
	inline bool HasItem(COMMODITYID cid) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).second->CommodityId() == cid)
				return true;

			const AuItemContent* c = (*ii).second->Content();
			if(c && c->HasItem(cid))
				return true;
		}
		return false;
	}

	// Recursive.
	inline bool HasItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).second->CommodityId() == cid &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin &&
					(*ii).second->Quality() == quality)
				return true;

			const AuItemContent* c = (*ii).second->Content();
			if(c && c->HasItem(cid, quality, empireOfOrigin))
				return true;
		}
		return false;
	}

	// Recursive.
	inline bool HasItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& inscription) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).second->CommodityId() == cid &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin &&
					(*ii).second->Quality() == quality &&
					(*ii).second->Inscription() == inscription)
				return true;

			const AuItemContent* c = (*ii).second->Content();
			if(c && c->HasItem(cid, quality, empireOfOrigin, inscription))
				return true;
		}
		return false;
	}

	inline int HasItemCount(COMMODITYID cid) const
	{
		int itemCount = 0;
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).second->CommodityId() == cid)
				itemCount += (*ii).second->Count();

			const AuItemContent* c = (*ii).second->Content();
			if(c)
				itemCount += c->HasItemCount(cid);
		}
		return itemCount;
	}

	inline bool IsValid(const QList<AuItemContentId>& contentPath) const
	{
		if(contentPath.isEmpty())
			return false;

		// Locate the first item in the contentPath. It should be in this container.
		const_iterator ii = find(contentPath.first());
		if(ii == cend())
			return false;

		if(contentPath.size() == 1)
			return true;
		else
		{
			// The next item should be a container.
			const AuItemContent* c = (*ii).second->Content();
			if(!c)
				return false;	// Error; this should be a container.

			// Remove the first item from the path and recurse into that container.
			QList<AuItemContentId> subPath(contentPath);
			subPath.pop_front();
			return c->IsValid(subPath);
		}
	}

	inline const AuItem* ItemAt(const QList<AuItemContentId>& contentPath) const
	{
		if(contentPath.isEmpty())
			return 0;

		// Locate the first item in the contentPath. It should be in this container.
		const_iterator ii = find(contentPath.first());
		if(ii == cend())
			return 0;

		if(contentPath.size() == 1)
			return (*ii).second;
		else
		{
			// The top item should be a container.
			const AuItemContent* c = (*ii).second->Content();
			if(!c)
				return 0;	// Error; this should be a container.

			// Remove the top item from the path and recurse into that container.
			QList<AuItemContentId> subPath(contentPath);
			subPath.pop_front();
			return c->ItemAt(subPath);
		}
	}

	inline AuItem* ItemAt(const QList<AuItemContentId>& contentPath)
	{
		if(contentPath.isEmpty())
			return 0;

		// Locate the first item in the contentPath. It should be in this container.
		iterator ii = find(contentPath.first());
		if(ii == end())
			return 0;

		if(contentPath.size() == 1)
			return (*ii).second;
		else
		{
			// The top item should be a container.
			AuItemContent* c = (*ii).second->Content();
			if(!c)
				return 0;	// Error; this should be a container.

			// Remove the top item from the path and recurse into that container.
			QList<AuItemContentId> subPath(contentPath);
			subPath.pop_front();
			return c->ItemAt(subPath);
		}
	}

	// Delete the item as it is erased.
	inline iterator Erase(iterator pos)
	{
		delete (*pos).second;
		return erase(pos);
	}

	// Returns true if any materials were consumed.
	// This function is only ever called on the contents of DaItem objects lying about, not on city inventories.
	bool FetchConstructionMaterials(DeConstructionProcess& cp);

	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	// This function is only ever called on the contents of DaItem objects lying about, not on city inventories.
	bool FetchManufacturingMaterials(DeManufacturingProcess& mp, quint8 elecSubQ, quint8 waterSubQ);

	// Takes ownership of 'item'.
	// Allocates a unique item id for the new item.
	// Returns false on failure due to no unused ids available.
	inline bool Insert(AuItem* item)
	{
		AuItemContentId iid = UnusedId();
		if(!iid.IsValid())
			return false;

		insert(pair<AuItemContentId, AuItem*>(iid, item));
		return true;
	}

	// Recursive
	void ListAll(const AuGearPos& parentPos, AuGearPosList& gearPos) const;

	// Recursive
	void ListItems(COMMODITYID cid, const AuGearPos& parentPos, AuGearPosList& gearPos) const;

	// Recursive
	void ListPatents(AuKnownPatents& patents) const;

	quint64 MemoryUsage() const;

	// 're' is not null when the OpenGL context is current.
	void Refresh(const AuItemContent& c, AuGLEffects* re, AuSound* al);

	// Takes ownership of item.
	// Does nothing if 'id' is invalid.
	void ReplaceItem(const QList<AuItemContentId>& contentPath, AuItem* item);

	inline void StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).second->StoryInstance() == storyInstance &&
					(*ii).second->CommodityId() == cid &&
					(*ii).second->Quality() == quality &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin)
				(*ii).second->SetStoryInstance(0);

			// Recursive search into all containers.
			if((*ii).second->Content())
				(*ii).second->Content()->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin);
		}
	}

	inline void StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, const QString& inscription)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).second->StoryInstance() == storyInstance &&
					(*ii).second->CommodityId() == cid &&
					(*ii).second->Quality() == quality &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin &&
					(*ii).second->Inscription() == inscription)
				(*ii).second->SetStoryInstance(0);

			// Recursive search into all containers.
			if((*ii).second->Content())
				(*ii).second->Content()->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin,
						inscription);
		}
	}

	// 'drop' 0=default random drop, 1=never drop, 2=always drop
	inline void StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).second->StoryInstance() == storyInstance &&
					(*ii).second->CommodityId() == cid &&
					(*ii).second->Quality() == quality &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin)
				(*ii).second->SetDropOnDeath(drop);

			// Recursive search into all containers.
			if((*ii).second->Content())
				(*ii).second->Content()->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin);
		}
	}

	inline void StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, const QString& inscription)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).second->StoryInstance() == storyInstance &&
					(*ii).second->CommodityId() == cid &&
					(*ii).second->Quality() == quality &&
					(*ii).second->EmpireOfOrigin() == empireOfOrigin &&
					(*ii).second->Inscription() == inscription)
				(*ii).second->SetDropOnDeath(drop);

			// Recursive search into all containers.
			if((*ii).second->Content())
				(*ii).second->Content()->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin,
						inscription);
		}
	}

	inline float Weight() const
	{
		float weight = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			weight += (*it).second->Weight();
		return weight;
	}

	// Returns invalid AuItemContentId on failure.
	inline AuItemContentId UnusedId() const
	{
		if(empty())
			return AuItemContentId(1);

		// Find the highest and lowest ids in use.
		const_iterator it = cbegin();
		AuItemContentId lowId = (*it).first;
		AuItemContentId highId = (*it).first;
		it++;
		for(; it != cend(); it++)
		{
			if((*it).first < lowId)
				lowId = (*it).first;
			if((*it).first > highId)
				highId = (*it).first;
		}

		if(lowId.Id() > 1)
			return AuItemContentId(lowId.Id()-1);
		if(highId.Id() < 255)
			return AuItemContentId(highId.Id()+1);

		// Find an unused id.
		for(quint8 id=lowId.Id()+1; id < highId.Id()-1; id++)
		{
			AuItemContentId iid(id);
			if(find(iid) == cend())
				return iid;
		}

		// This is a failed return.
		qWarning() << "error: AuItemContent::UnusedId failed to find an unused id" << size();
		return AuItemContentId();
	}

	bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);
};

inline QDataStream& operator<<(QDataStream& out, const AuItemContent& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuItemContent& o)
{
	o << in;
	return in;
}

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuItemContent& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuItem subclasses begin here.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemScDesign : public AuItem
{
protected:
	AuScDesignExchangeEntry				m_Exchange;

public:
	AuItemScDesign(const AuScDesignExchangeEntry& exchange,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemScDesign(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemScDesign();
	AuItemScDesign(const AuItemScDesign& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemScDesign& c = dynamic_cast<const AuItemScDesign&>(item);
			if(m_Exchange.DesignId() < c.m_Exchange.DesignId())
				return true;
			else if(m_Exchange.DesignId() > c.m_Exchange.DesignId())
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual quint32 DesignId() const
	{
		return m_Exchange.DesignId();
	}

	const AuScDesignExchangeEntry& Exchange() const
	{
		return m_Exchange;
	}

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemScDesign& c = dynamic_cast<const AuItemScDesign&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_Exchange != c.m_Exchange)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_ScDesign;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemScDesign& c = dynamic_cast<const AuItemScDesign&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_Exchange != c.m_Exchange)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	void SetExchange(const AuScDesignExchangeEntry& exchange)
	{
		m_Exchange = exchange;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items with DNA associated with them.

class AUDATA_DLL AuItemDNA : public AuItem
{
protected:
	AuBountyOnItem*						m_Bounty;				// Bounty if any, for heads of avatars who die with a bounty.
	AuDNABase*							m_DNA;					// DNA associated with this item.
	bool								m_DNADead;				// True if the DNA is dead.
	AuDNABase::CHROMATID				m_DNAChromatid;
	AuHairStyle::STYLE					m_DNAHair;
	HANDEDNESS							m_DNAHand;
	AuHairStyle::SHAVE					m_DNAShave;
	GROWTHSTAGE							m_DNAStage;
	bool								m_Sentient;				// True if the DNA is from a sentient creature.

private:
	mutable AuAnimalia*					m_HeadAnimalia;			// When I am a head.
	mutable DpBody*						m_HeadBody;				// When I am a head.

public:
	AuItemDNA(const AuDNABase* dna, AuDNABase::CHROMATID chromatid, bool dnaViable, bool sentient, HANDEDNESS hand,
			GROWTHSTAGE stage, AuHairStyle::STYLE hair, AuHairStyle::SHAVE shave, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, quint32 count=1);
	AuItemDNA(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemDNA();
	virtual ~AuItemDNA();
	AuItemDNA(const AuItemDNA& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const;

public:
	// Returns the fraction of overall damage absorbed by this armor vs a weapon 'mode', in the range 0 to 1.
	virtual float ArmorModeAbsorb(WEAPONMODE mode) const;

	// Returns the amount of damage absorbed by this armor vs a weapon 'mode', in the range 0 to n.
	virtual float ArmorModeBlock(WEAPONMODE mode) const;

	virtual AuBountyOnItem Bounty() const
	{
		if(m_Bounty)
			return *m_Bounty;
		return AuItem::Bounty();
	}

	virtual bool CanCombine() const
	{
		return false;
	}

	virtual bool CanStack(const AuItem* item) const;

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	virtual const AuDNABase* DNA() const
	{
		return m_DNA;
	}

	inline AuDNABase::CHROMATID DNAChromatid() const
	{
		return m_DNAChromatid;
	}

	GENDER DNAGender() const;

	QString DNAGenderName() const;

	virtual GROWTHSTAGE DNAGrowthStage() const
	{
		return m_DNAStage;
	}

	inline AuHairStyle::STYLE DNAHair() const
	{
		return m_DNAHair;
	}

	inline HANDEDNESS DNAHandedness() const
	{
		return m_DNAHand;
	}

	virtual bool DNAIsSentient() const
	{
		return m_Sentient;
	}

	// Returns true if this item has DNA that is not dead.
	virtual bool DNAIsViable() const
	{
		return !m_DNADead && HasDNA();
	}

	inline AuHairStyle::SHAVE DNAShave() const
	{
		return m_DNAShave;
	}

	// Return true if I am waiting for the animalia model for rendering. i.e. heads.
	// Use of this function allows the animalia model to be constructed by the model maker thread before rendering this item. It is
	// optional. Simply calling the appropriate render function will create the model on the fly. For gen4 heads, this causes a huge
	// lag spike.
	virtual bool DNAWantsAnimaliaModel() const;

	// Caller is giving me 'animalia' because I returned true to DNAWantsAnimaliaModel.
	// I take ownership of the pointer.
	virtual void DNASetAnimaliaModel(AuAnimalia* animalia);

	virtual AuSpecies DNASpecies() const;

	// Render all animal and plant DNA in this item and its contents to be unviable.
	virtual void DNAKillRecursive()
	{
		m_DNADead = true;
	}

	inline bool HasDNA() const
	{
		return m_DNA;
	}

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemDNA& c = dynamic_cast<const AuItemDNA&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_Bounty && c.m_Bounty)
			{
				if(*m_Bounty != *c.m_Bounty)
					return false;
			}
			else if(m_Bounty || c.m_Bounty)
				return false;
			if(m_Sentient != c.m_Sentient)
				return false;
			if(m_DNADead != c.m_DNADead)
				return false;
			if((m_DNA && !c.m_DNA) ||
					(!m_DNA && c.m_DNA))
				return false;
			if((m_DNA && c.m_DNA) && (*m_DNA != *c.m_DNA))
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_DNA;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	// Sets 'min' and 'max' to the 3D extents of this item's model.
	// Returns true if they are set, false if not.
	virtual bool ModelExtents(AuPoint& min, AuPoint& max) const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemDNA& c = dynamic_cast<const AuItemDNA&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_Bounty && c.m_Bounty)
			{
				if(*m_Bounty != *c.m_Bounty)
					return false;
			}
			else if(m_Bounty || c.m_Bounty)
				return false;
			if(m_Sentient != c.m_Sentient)
				return false;
			if(m_DNADead != c.m_DNADead)
				return false;
			if((m_DNA && !c.m_DNA) ||
					(!m_DNA && c.m_DNA))
				return false;
			if((m_DNA && c.m_DNA) && (*m_DNA != *c.m_DNA))
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0, double effectScale=1.);

	// 'globeP' is the position of the globe, in eye space.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			const AuPoint& globeP,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			float shadowPanelSunAngles[MAX_SUNS],
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	virtual void SetBounty(const AuBountyOnItem& bounty)
	{
		if(bounty.Id().IsNull())
		{
			if(m_Bounty)
			{
				delete m_Bounty;
				m_Bounty = 0;	// Clear current bounty.
			}
		}
		else
		{
			if(m_Bounty)
				*m_Bounty = bounty;
			else
				m_Bounty = new AuBountyOnItem(bounty);
		}
	}

	void SetDNA(const AuDNABase* dna, bool dnaViable, bool isSentient, GROWTHSTAGE stage);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	virtual float UnitCost() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items with a mission associated with them.

class AUDATA_DLL AuItemMission : public AuItem
{
protected:
	AuMission							m_Mission;				// Mission associated with this item.
	QString								m_MissionName;			// Name assigned to the mission; could be blank.

public:
	AuItemMission(const QString& missionName, const AuMission& mission, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, quint32 count=1);
	AuItemMission(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemMission();
	AuItemMission(const AuItemMission& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemMission& c = dynamic_cast<const AuItemMission&>(item);
			if(m_MissionName < c.m_MissionName)
				return true;
			else if(m_MissionName > c.m_MissionName)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemMission& c = dynamic_cast<const AuItemMission&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(MissionName() != c.MissionName())
				return false;
			if(Mission() != c.Mission())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_Mission;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	inline const AuMission& Mission() const
	{
		return m_Mission;
	}

	inline const QString& MissionName() const
	{
		return m_MissionName;
	}

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemMission& c = dynamic_cast<const AuItemMission&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(MissionName() != c.MissionName())
				return false;
			if(Mission() != c.Mission())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items that can have an associated state value, 0 to 255, e.g. lantern on/off state, helmet open/closed state.

class AUDATA_DLL AuItemState : public AuItem
{
protected:
	quint8								m_State;				// State value for this item.

public:
	AuItemState(quint8 state, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemState(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemState();
	AuItemState(const AuItemState& c);

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual bool IsEnvironmentHelmet() const
	{
		return DbCommodity::IsEnvironmentHelmet(CommodityId());
	}

	virtual bool IsEnvironmentSuit() const
	{
		return DbCommodity::IsEnvironmentSuit(CommodityId());
	}

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemState& c = dynamic_cast<const AuItemState&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(State() != c.State())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_State;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemState& c = dynamic_cast<const AuItemState&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(State() != c.State())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	virtual void SetState(quint8 state)
	{
		m_State = state;
	}

	virtual quint8 State() const
	{
		return m_State;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items that are ammo for weapons.

class AUDATA_DLL AuItemAmmo : public AuItem
{
public:
	AuItemAmmo(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemAmmo();
	AuItemAmmo(const AuItemAmmo& c);

public:	// Operators
	const AuItemAmmo& operator=(const AuItemAmmo& c);

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemAmmo& c = dynamic_cast<const AuItemAmmo&>(item);
			if(!AuItem::operator==(c))
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);


public:	// Methods
	// Value returned is a percentage number in the range 0 to 100.
	// Armor pierce negates armor absorb.
	inline quint8 ArmorPierce1() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();
		return bCom.m_wpn1AP1;
	}

	// Returns the armor pierce value as a fraction of 100.
	inline float ArmorPierce1F() const
	{
		return (float)ArmorPierce1()/100.f;
	}

	// Value returned is a percentage number in the range 0 to 100.
	// Armor pierce negates armor absorb.
	inline quint8 ArmorPierce2() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();
		return bCom.m_wpn1AP2;
	}

	// Returns the armor pierce value as a fraction of 100.
	inline float ArmorPierce2F() const
	{
		return (float)ArmorPierce2()/100.f;
	}

	inline AMMOTYPE AmmoType() const
	{
		return Commodity().m_ammo1Type;
	}

	virtual float BlastRadius() const
	{
		if(Effect2() == WE_None)
			return Radius1();
		float blastrad1 = Radius1();
		float blastrad2 = Radius2();
		if(blastrad1 > blastrad2)
			return blastrad1;
		return blastrad2;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	quint16 Damage1() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();
		if(!bCom.m_wpn1Damage1)
			return 0;

		// Increase damage value due to quality.
		int damage = QualityBoosted(bCom.m_wpn1Damage1);

		// Return value within limits.
		if(damage < 1)
			return 1;
		else if(damage > 0xffff)
			return 0xffff;
		else
			return (quint16)damage;
	}

	quint16 Damage2() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();
		if(!bCom.m_wpn1Damage2)
			return 0;

		// Increase damage value due to quality.
		int damage = QualityBoosted(bCom.m_wpn1Damage2);

		// Return value within limits.
		if(damage < 1)
			return 1;
		else if(damage > 0xffff)
			return 0xffff;
		else
			return (quint16)damage;
	}

	quint16 DamageMod1() const
	{
		// Get base damage modifier from commodity record.
		const DbCommodity& bCom = Commodity();

		// Increase damage modifier due to quality.
		int damageMod = QualityBoosted(bCom.m_wpn1DamageMod1);

		// Return value within limits.
		if(damageMod < 0)
			return 0;
		else if(damageMod > 0xffff)
			return 0xffff;
		else
			return (quint16)damageMod;
	}

	quint16 DamageMod2() const
	{
		// Get base damage modifier from commodity record.
		const DbCommodity& bCom = Commodity();

		// Increase damage modifier due to quality.
		int damageMod = QualityBoosted(bCom.m_wpn1DamageMod2);

		// Return value within limits.
		if(damageMod < 0)
			return 0;
		else if(damageMod > 0xffff)
			return 0xffff;
		else 
			return (quint16)damageMod;
	}

	virtual bool DoesExplodeWhenDamaged() const
	{
		return Radius1() > 0 || Radius2() > 0 || CommodityId() == CID_Gigacell;
	}

	WEAPONEFFECT Effect1() const
	{
		return Commodity().m_wpn1Effect1;
	}

	WEAPONEFFECT Effect2() const
	{
		return Commodity().m_wpn1Effect2;
	}

	float Radius1() const
	{
		const DbCommodity& bCom = Commodity();
		return QualityBoosted(bCom.m_wpn1Radius1);
	}

	float Radius2() const
	{
		const DbCommodity& bCom = Commodity();
		return QualityBoosted(bCom.m_wpn1Radius2);
	}

	float Range() const
	{
		const DbCommodity& bCom = Commodity();
		return QualityBoosted(bCom.m_wpn1Range);
	}

	// Returns true if this item is ammunition.
	virtual bool IsAmmo() const
	{
		return DbCommodity::IsAmmo(AmmoType(), 0);
	}

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemAmmo& c = dynamic_cast<const AuItemAmmo&>(item);
			if(!AuItem::IsItem(c))
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_Ammo;
	}

	virtual QString ListName() const;

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Value is a number of hit points.
	// Strike through negates armor block.
	quint16 StrikeThrough1() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();

		// Increase damage value due to quality.
		int st = QualityBoosted(bCom.m_wpn1ST1);

		// Return value within limits.
		if(st < 0)
			return 0;
		else if(st > 0xffff)
			return 0xffff;
		else
			return (quint16)st;
	}

	// Value is a number of hit points.
	// Strike through negates armor block.
	quint16 StrikeThrough2() const
	{
		// Get base damage value from commodity record.
		const DbCommodity& bCom = Commodity();

		// Increase damage value due to quality.
		int st = QualityBoosted(bCom.m_wpn1ST2);

		// Return value within limits.
		if(st < 0)
			return 0;
		else if(st > 0xffff)
			return 0xffff;
		else
			return (quint16)st;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items that can be used as a weapon.

class AUDATA_DLL AuItemWeapon : public AuItem
{
public:
	// Weapon values are set by the ammo that is loaded or by the specialty of the weapon smith when manufactured.
	quint16								m_wpn1Range;
	quint8								m_wpn1Effect1;
	quint16								m_wpn1Damage1;
	quint16								m_wpn1DamageMod1;
	quint16								m_wpn1Radius1;
	quint8								m_wpn1AP1;
	quint16								m_wpn1ST1;
	quint8								m_wpn1Effect2;
	quint16								m_wpn1Damage2;
	quint16								m_wpn1DamageMod2;
	quint16								m_wpn1Radius2;
	quint8								m_wpn1AP2;
	quint16								m_wpn1ST2;
	quint16								m_wpn2Range;
	quint8								m_wpn2Effect1;
	quint16								m_wpn2Damage1;
	quint16								m_wpn2DamageMod1;
	quint16								m_wpn2Radius1;
	quint8								m_wpn2AP1;
	quint16								m_wpn2ST1;
	quint8								m_wpn2Effect2;
	quint16								m_wpn2Damage2;
	quint16								m_wpn2DamageMod2;
	quint16								m_wpn2Radius2;
	quint8								m_wpn2AP2;
	quint16								m_wpn2ST2;

public:
	AuItemWeapon(quint16 w1Range,
			WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1ap1, quint16 w1st1,
			WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1ap2, quint16 w1st2,
			quint16 w2Range,
			WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2ap1, quint16 w2st1,
			WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2ap2, quint16 w2st2,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeapon(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeapon();
	AuItemWeapon(const AuItemWeapon& c);

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemWeapon& c = dynamic_cast<const AuItemWeapon&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_wpn1Range != c.m_wpn1Range)
				return false;
			if(m_wpn1Effect1 != c.m_wpn1Effect1)
				return false;
			if(m_wpn1Damage1 != c.m_wpn1Damage1)
				return false;
			if(m_wpn1DamageMod1 != c.m_wpn1DamageMod1)
				return false;
			if(m_wpn1Radius1 != c.m_wpn1Radius1)
				return false;
			if(m_wpn1AP1 != c.m_wpn1AP1)
				return false;
			if(m_wpn1ST1 != c.m_wpn1ST1)
				return false;
			if(m_wpn1Effect2 != c.m_wpn1Effect2)
				return false;
			if(m_wpn1Damage2 != c.m_wpn1Damage2)
				return false;
			if(m_wpn1DamageMod2 != c.m_wpn1DamageMod2)
				return false;
			if(m_wpn1Radius2 != c.m_wpn1Radius2)
				return false;
			if(m_wpn1AP2 != c.m_wpn1AP2)
				return false;
			if(m_wpn1ST2 != c.m_wpn1ST2)
				return false;
			if(m_wpn2Range != c.m_wpn2Range)
				return false;
			if(m_wpn2Effect1 != c.m_wpn2Effect1)
				return false;
			if(m_wpn2Damage1 != c.m_wpn2Damage1)
				return false;
			if(m_wpn2DamageMod1 != c.m_wpn2DamageMod1)
				return false;
			if(m_wpn2Radius1 != c.m_wpn2Radius1)
				return false;
			if(m_wpn2AP1 != c.m_wpn2AP1)
				return false;
			if(m_wpn2ST1 != c.m_wpn2ST1)
				return false;
			if(m_wpn2Effect2 != c.m_wpn2Effect2)
				return false;
			if(m_wpn2Damage2 != c.m_wpn2Damage2)
				return false;
			if(m_wpn2DamageMod2 != c.m_wpn2DamageMod2)
				return false;
			if(m_wpn2Radius2 != c.m_wpn2Radius2)
				return false;
			if(m_wpn2AP2 != c.m_wpn2AP2)
				return false;
			if(m_wpn2ST2 != c.m_wpn2ST2)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}

		return true;
	}

	// Returns true if this item is inherently a weapon. Returns false for other items, including those that could be used as a
	// weapon.
	virtual bool IsWeapon() const
	{
		return true;
	}

	static quint8 ItemType()
	{
		return IT_Weapon;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemWeapon& c = dynamic_cast<const AuItemWeapon&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_wpn1Range != c.m_wpn1Range)
				return false;
			if(m_wpn1Effect1 != c.m_wpn1Effect1)
				return false;
			if(m_wpn1Damage1 != c.m_wpn1Damage1)
				return false;
			if(m_wpn1DamageMod1 != c.m_wpn1DamageMod1)
				return false;
			if(m_wpn1Radius1 != c.m_wpn1Radius1)
				return false;
			if(m_wpn1AP1 != c.m_wpn1AP1)
				return false;
			if(m_wpn1ST1 != c.m_wpn1ST1)
				return false;
			if(m_wpn1Effect2 != c.m_wpn1Effect2)
				return false;
			if(m_wpn1Damage2 != c.m_wpn1Damage2)
				return false;
			if(m_wpn1DamageMod2 != c.m_wpn1DamageMod2)
				return false;
			if(m_wpn1Radius2 != c.m_wpn1Radius2)
				return false;
			if(m_wpn1AP2 != c.m_wpn1AP2)
				return false;
			if(m_wpn1ST2 != c.m_wpn1ST2)
				return false;
			if(m_wpn2Range != c.m_wpn2Range)
				return false;
			if(m_wpn2Effect1 != c.m_wpn2Effect1)
				return false;
			if(m_wpn2Damage1 != c.m_wpn2Damage1)
				return false;
			if(m_wpn2DamageMod1 != c.m_wpn2DamageMod1)
				return false;
			if(m_wpn2Radius1 != c.m_wpn2Radius1)
				return false;
			if(m_wpn2AP1 != c.m_wpn2AP1)
				return false;
			if(m_wpn2ST1 != c.m_wpn2ST1)
				return false;
			if(m_wpn2Effect2 != c.m_wpn2Effect2)
				return false;
			if(m_wpn2Damage2 != c.m_wpn2Damage2)
				return false;
			if(m_wpn2DamageMod2 != c.m_wpn2DamageMod2)
				return false;
			if(m_wpn2Radius2 != c.m_wpn2Radius2)
				return false;
			if(m_wpn2AP2 != c.m_wpn2AP2)
				return false;
			if(m_wpn2ST2 != c.m_wpn2ST2)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}

		return true;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	// Collects information about this items weapon usage.
	virtual void WeaponInfo(INVENTORYSUB sub,
			COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
			WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
			WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Weapons that require ammo or any sort of charge level.

class AUDATA_DLL AuItemWeaponAmmo : public AuItemWeapon
{
protected:
	quint8								m_Ammo1Loaded;			// Quantity of ammo or other charge loaded now.
	quint8								m_Ammo1Quality;			// Quality of last ammo loaded.
	quint8								m_Ammo2Loaded;			// Quantity of ammo or other charge loaded now.
	quint8								m_Ammo2Quality;			// Quality of last ammo loaded.

public:
	AuItemWeaponAmmo(quint16 w1Range,
			WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1ap1, quint16 w1st1,
			WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1ap2, quint16 w1st2,
			quint16 w2Range,
			WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2ap1, quint16 w2st1,
			WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2ap2, quint16 w2st2,
			quint8 ammo1Loaded, quint8 ammo1Quality,
			quint8 ammo2Loaded, quint8 ammo2Quality,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeaponAmmo(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeaponAmmo();
	AuItemWeaponAmmo(const AuItemWeaponAmmo& c);

	inline quint8 Ammo1Loaded() const
	{
		return m_Ammo1Loaded;
	}

	inline quint8 Ammo1Qty() const
	{
		return Commodity().m_ammo1Qty;
	}

	inline quint8 Ammo1Quality() const
	{
		return m_Ammo1Quality;
	}

	AMMOTYPE Ammo1Type() const
	{
		return Commodity().m_ammo1Type;
	}

	inline quint8 Ammo2Loaded() const
	{
		// Energy1 ammo indicates that weapon two uses the ammo of weapon 1. It is treated as a firing mode. e.g. laser weapons.
		if(Ammo2Type() == AMMO_Energy1)
			return Ammo1Loaded();
		return m_Ammo2Loaded;
	}

	inline quint8 Ammo2Qty() const
	{
		// Energy1 ammo indicates that weapon two uses the ammo of weapon 1. It is treated as a firing mode. e.g. laser weapons.
		if(Ammo2Type() == AMMO_Energy1)
			return Ammo1Qty();
		return Commodity().m_ammo2Qty;
	}

	inline quint8 Ammo2Quality() const
	{
		return m_Ammo2Quality;
	}

	AMMOTYPE Ammo2Type() const
	{
		return Commodity().m_ammo2Type;
	}

	virtual int AmmoLoaded(INVENTORYSUB sub) const
	{
		switch(sub)
		{
		case IPS_Item:
			break;					// Physical weapon ability never uses ammo.
		case IPS_Weapon1:
			return Ammo1Loaded();
		case IPS_Weapon2:
			return Ammo2Loaded();
		}
		return 0;
	}

	virtual int AmmoQuantity(INVENTORYSUB sub) const
	{
		switch(sub)
		{
		case IPS_Item:
			break;					// Physical weapon ability never uses ammo.
		case IPS_Weapon1:
			return Ammo1Qty();
		case IPS_Weapon2:
			return Ammo2Qty();
		}
		return 0;
	}

	// True if this is a weapon that is ready to fire/wield. False if out of ammo.
	virtual bool CanFireWeapon(INVENTORYSUB sub) const
	{
		if(!AuItemWeapon::CanFireWeapon(sub))
			return false;	// This item is not a weapon.

		switch(sub)
		{
		case IPS_Item:
			break;						// Physical weapon usage of item is always ready.
		case IPS_Weapon1:
			if(Ammo1Qty() && !Ammo1Loaded())
				return false;			// Primary weapon requires ammo but doesn't have any.
			break;
		case IPS_Weapon2:
			if(Ammo2Qty() && !Ammo2Loaded())
				return false;			// Secondary weapon requires ammo but doesn't have any.
			break;
		}
		return true;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	// Return true if any ammo is expended, up to the number of rounds specified.
	virtual bool ExpendAmmo(INVENTORYSUB sub, unsigned int rounds)
	{
		// Energy1 ammo indicates that weapon two uses the ammo of weapon 1. It is treated as a firing mode. e.g. laser weapons.
		if(Ammo2Type() == AMMO_Energy1 && sub == IPS_Weapon2)
			sub = IPS_Weapon1;

		switch(sub)
		{
		case IPS_Item:
			break;					// Physical weapon ability never uses ammo.
		case IPS_Weapon1:
			if(m_Ammo1Loaded)
			{
				// Weapon has ammo loaded.
				if(rounds >= m_Ammo1Loaded)
					m_Ammo1Loaded = 0;
				else
					m_Ammo1Loaded -= rounds;
				return true;
			}
			break;
		case IPS_Weapon2:
			if(m_Ammo2Loaded)
			{
				// Weapon has ammo loaded.
				if(rounds >= m_Ammo2Loaded)
					m_Ammo2Loaded = 0;
				else
					m_Ammo2Loaded -= rounds;
				return true;
			}
		}
		return false;
	}

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemWeaponAmmo& c = dynamic_cast<const AuItemWeaponAmmo&>(item);

			// Note: We intentionally skip AuItemWeapon so we can test fields that are only relevant when ammo is loaded.
			if(!AuItem::IsItem(c))
				return false;

			if(m_Ammo1Loaded != c.m_Ammo1Loaded)
				return false;

			// Original manufacture provides the range mod, unless it's a missile or rocket launcher..
			AMMOTYPE ammo1Type = Ammo1Type();
			if(m_wpn1Range != c.m_wpn1Range && (ammo1Type != AMMO_Tube && ammo1Type != AMMO_Launcher))
				return false;

			if(m_Ammo1Loaded)
			{
				// Test factors that are only relevant when ammo is loaded.
				if(m_Ammo1Quality != c.m_Ammo1Quality)
					return false;

				// Missiles and rockets provide the range mod.
				if((ammo1Type == AMMO_Tube || ammo1Type == AMMO_Launcher) && m_wpn1Range != c.m_wpn1Range)
					return false;
				if(m_wpn1Effect1 != c.m_wpn1Effect1)
					return false;
				if(m_wpn1Damage1 != c.m_wpn1Damage1)
					return false;
				if(m_wpn1DamageMod1 != c.m_wpn1DamageMod1)
					return false;
				if(m_wpn1Radius1 != c.m_wpn1Radius1)
					return false;
				if(m_wpn1AP1 != c.m_wpn1AP1)
					return false;
				if(m_wpn1ST1 != c.m_wpn1ST1)
					return false;
				if(m_wpn1Effect2 != c.m_wpn1Effect2)
					return false;
				if(m_wpn1Damage2 != c.m_wpn1Damage2)
					return false;
				if(m_wpn1DamageMod2 != c.m_wpn1DamageMod2)
					return false;
				if(m_wpn1Radius2 != c.m_wpn1Radius2)
					return false;
				if(m_wpn1AP2 != c.m_wpn1AP2)
					return false;
				if(m_wpn1ST2 != c.m_wpn1ST2)
					return false;
			}

			if(m_Ammo2Loaded != c.m_Ammo2Loaded)
				return false;

			// Original manufacture provides the range mod, unless it's a missile or rocket launcher..
			AMMOTYPE ammo2Type = Ammo2Type();
			if(m_wpn2Range != c.m_wpn2Range && (ammo2Type != AMMO_Tube && ammo2Type != AMMO_Launcher))
				return false;

			if(Ammo2Loaded())
			{
				// Test factors that are only relevant when ammo is loaded.
				if(m_Ammo2Quality != c.m_Ammo2Quality)
					return false;

				// Missiles and rockets provide the range mod.
				if((ammo2Type == AMMO_Tube || ammo2Type == AMMO_Launcher) && m_wpn2Range != c.m_wpn2Range)
					return false;
				if(m_wpn2Effect1 != c.m_wpn2Effect1)
					return false;
				if(m_wpn2Damage1 != c.m_wpn2Damage1)
					return false;
				if(m_wpn2DamageMod1 != c.m_wpn2DamageMod1)
					return false;
				if(m_wpn2Radius1 != c.m_wpn2Radius1)
					return false;
				if(m_wpn2AP1 != c.m_wpn2AP1)
					return false;
				if(m_wpn2ST1 != c.m_wpn2ST1)
					return false;
				if(m_wpn2Effect2 != c.m_wpn2Effect2)
					return false;
				if(m_wpn2Damage2 != c.m_wpn2Damage2)
					return false;
				if(m_wpn2DamageMod2 != c.m_wpn2DamageMod2)
					return false;
				if(m_wpn2Radius2 != c.m_wpn2Radius2)
					return false;
				if(m_wpn2AP2 != c.m_wpn2AP2)
					return false;
				if(m_wpn2ST2 != c.m_wpn2ST2)
					return false;
			}

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual bool IsWeaponThatUsesAmmo(INVENTORYSUB sub) const
	{
		return Commodity().IsWeaponThatUsesAmmo(sub);
	}

	static quint8 ItemType()
	{
		return IT_WeaponAmmo;
	}

	virtual QString ListName() const;

	virtual bool LoadAmmoIntoWeapon(const AuItem& item);

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemWeaponAmmo& c = dynamic_cast<const AuItemWeaponAmmo&>(item);
			if(!AuItemWeapon::operator==(c))
				return false;

			if(m_Ammo1Loaded != c.m_Ammo1Loaded)
				return false;
			if(m_Ammo1Quality != c.m_Ammo1Quality)
				return false;
			if(m_Ammo2Loaded != c.m_Ammo2Loaded)
				return false;
			if(m_Ammo2Quality != c.m_Ammo2Quality)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	// Collects information about this items weapon usage.
	virtual void WeaponInfo(INVENTORYSUB sub,
			COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
			WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
			WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items used to activate ringworld stargates.

class AUDATA_DLL AuItemGalaxyLoc : public AuItem
{
protected:
	AuId								m_GalaxyId;				// Atom id of the galaxy.
	AuPoint								m_GalaxyLoc;			// Location in the galaxy, in pc.
	QString								m_GalaxyName;			// Name of the galaxy.

public:
	AuItemGalaxyLoc(const AuId& galaxyId, const QString& galaxyName, const AuPoint& galaxyLoc,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemGalaxyLoc(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemGalaxyLoc();
	AuItemGalaxyLoc(const AuItemGalaxyLoc& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemGalaxyLoc& c = dynamic_cast<const AuItemGalaxyLoc&>(item);
			if(m_GalaxyId < c.m_GalaxyId)
				return true;
			else if(m_GalaxyId > c.m_GalaxyId)
				return false;

			if(m_GalaxyLoc < c.m_GalaxyLoc)
				return true;
			else if(m_GalaxyLoc != c.m_GalaxyLoc)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	virtual bool CanStack(const AuItem* item) const;

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	inline const AuId& GalaxyId() const
	{
		return m_GalaxyId;
	}

	inline const AuPoint& GalaxyLoc() const
	{
		return m_GalaxyLoc;
	}

	inline const QString& GalaxyName() const
	{
		return m_GalaxyName;
	}

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemGalaxyLoc& c = dynamic_cast<const AuItemGalaxyLoc&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_GalaxyId != c.m_GalaxyId)
				return false;
			if(m_GalaxyLoc != c.m_GalaxyLoc)
				return false;
			if(m_GalaxyName != c.m_GalaxyName)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual bool IsStargateKey() const
	{
		return true;
	}

	static quint8 ItemType()
	{
		return IT_GalaxyLoc;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemGalaxyLoc& c = dynamic_cast<const AuItemGalaxyLoc&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_GalaxyId != c.m_GalaxyId)
				return false;
			if(m_GalaxyLoc != c.m_GalaxyLoc)
				return false;
			if(m_GalaxyName != c.m_GalaxyName)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	// 'galaxyLoc' is a location in the galaxy, in pc.
	virtual void SetGalaxyLoc(const AuId& galaxyId, const QString& galaxyName, const AuPoint& galaxyLoc)
	{
		m_GalaxyId = galaxyId;
		m_GalaxyLoc = galaxyLoc;
		m_GalaxyName = galaxyName;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items that are containers for other items.

class AUDATA_DLL AuItemContainer : public AuItem
{
protected:
	quint8								m_Capacity;				// Number of box units this thing can hold. 1 to 254.
	AuItemContent						m_Contents;				// Contents of this container.

public:
	AuItemContainer(quint8 capacity, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
			quint32 count=1);
	AuItemContainer(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemContainer();
	AuItemContainer(const AuItemContainer& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemContainer& c = dynamic_cast<const AuItemContainer&>(item);
			if(m_Capacity < c.m_Capacity)
				return true;
			else if(m_Contents.size() > c.m_Contents.size())
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const;

	virtual quint16 CapacityInPackUnits() const
	{
		return m_Capacity;
	}

	virtual quint32 CargoVolume() const
	{
		return Count();
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual AuItemContent* Content()
	{
		return &m_Contents;
	}

	virtual const AuItemContent* Content() const
	{
		return &m_Contents;
	}

	// Render all animal and plant DNA in this item and its contents to be unviable.
	virtual void DNAKillRecursive()
	{
		for(AuItemContent::iterator ci = m_Contents.begin(); ci != m_Contents.end(); ci++)
			(*ci).second->DNAKillRecursive();
	}

	virtual bool FitItemIntoContainer(const AuItem& cItem);

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemContainer& c = dynamic_cast<const AuItemContainer&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_Contents != c.m_Contents)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_Container;
	}

	virtual QString ListName() const;

	virtual void ListPatents(AuKnownPatents& patents) const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemContainer& c = dynamic_cast<const AuItemContainer&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_Contents != c.m_Contents)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	virtual bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);

	// Recursively determines the weight of this item and all subordinate items in this container.
	// Antigravity backpack returns only the weight of the backpack itself, not including the contents.
	virtual float Weight() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Weapons that have a state associated with them. i.e. Torch.

class AUDATA_DLL AuItemWeaponState : public AuItemWeapon
{
protected:
	quint8								m_State;				// State value for this weapon.

public:
	AuItemWeaponState(quint16 w1Range,
			WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1AP1, quint16 w1ST1,
			WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1AP2, quint16 w1ST2,
			quint16 w2Range,
			WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2AP1, quint16 w2ST1,
			WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2AP2, quint16 w2ST2,
			quint8 state,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeaponState(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemWeaponState();
	AuItemWeaponState(const AuItemWeaponState& c);

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemWeaponState& c = dynamic_cast<const AuItemWeaponState&>(item);
			if(!AuItemWeapon::IsItem(c))
				return false;

			if(m_State != c.m_State)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_WeaponState;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemWeaponState& c = dynamic_cast<const AuItemWeaponState&>(item);
			if(!AuItemWeapon::operator==(c))
				return false;

			if(m_State != c.m_State)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	virtual void SetState(quint8 state)
	{
		m_State = state;
	}

	virtual quint8 State() const
	{
		return m_State;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items with an event log associated with them.

class AUDATA_DLL AuItemOfficerLog : public AuItem
{
protected:
	AuEventLog							m_EventLog;				// Event log associated with this item.
	QString								m_OfficerHome;			// Home of officer; could be blank.
	QString								m_OfficerName;			// Name of officer; could be blank.
	QString								m_ShipDesc;				// Description of ship; could be blank.
	QString								m_ShipName;				// Preferably DaVessel::NameWithEmpire; could be blank.
	quint8								m_ShipPrivate;			// true if the ship is privately owned, otherwise subsidized

public:
	AuItemOfficerLog(const QString& officerName, const QString& officerHome, const QString& shipName, const QString& shipDesc,
			bool shipPrivate, const AuEventLog& log,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemOfficerLog(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemOfficerLog();
	AuItemOfficerLog(const AuItemOfficerLog& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemOfficerLog& c = dynamic_cast<const AuItemOfficerLog&>(item);
			if(m_OfficerName < c.m_OfficerName)
				return true;
			else if(m_OfficerName > c.m_OfficerName)
				return false;

			if(m_ShipName < c.m_ShipName)
				return true;
			else if(m_ShipName > c.m_ShipName)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemOfficerLog& c = dynamic_cast<const AuItemOfficerLog&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_ShipPrivate != c.m_ShipPrivate)
				return false;
			if(OfficerHome() != c.OfficerHome())
				return false;
			if(OfficerName() != c.OfficerName())
				return false;
			if(ShipDesc() != c.ShipDesc())
				return false;
			if(ShipName() != c.ShipName())
				return false;
			if(EventLog() != c.EventLog())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_OfficerLog;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	inline const AuEventLog& EventLog() const
	{
		return m_EventLog;
	}

	inline const QString& OfficerHome() const
	{
		return m_OfficerHome;
	}

	inline const QString& OfficerName() const
	{
		return m_OfficerName;
	}

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemOfficerLog& c = dynamic_cast<const AuItemOfficerLog&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_ShipPrivate != c.m_ShipPrivate)
				return false;
			if(OfficerHome() != c.OfficerHome())
				return false;
			if(OfficerName() != c.OfficerName())
				return false;
			if(ShipDesc() != c.ShipDesc())
				return false;
			if(ShipName() != c.ShipName())
				return false;
			if(EventLog() != c.EventLog())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	inline const QString& ShipDesc() const
	{
		return m_ShipDesc;
	}

	inline const QString& ShipName() const
	{
		return m_ShipName;
	}

	inline bool ShipPrivate() const
	{
		return m_ShipPrivate;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items with flora DNA associated with them.

class AUDATA_DLL AuItemFloraDNA : public AuItem
{
protected:
	AuFloraDNA							m_DNA;					// Flora DNA associated with this item.
	bool								m_DNADead;				// True if the DNA is dead.
	quint8								m_LatitudeRegion;		// AuFloraDNA::ZONE from which the DNA was collected.
	quint8								m_PlantType;			// 0=ground cover, 1=shrub, 2=tree. Matches DeFlora::TYPE enum.
	quint32								m_PottedGenSeed;		// Gen seed when potted, so all pots are different.
	quint8								m_PottedSize;			// Size in pot.
	quint16								m_ProduceCID;			// Commodity id this plant produces when grown.
	quint32								m_WorldResourceZone;	// Resource zone from original world.
	quint32								m_WorldGenSeed;			// Gen seed from original world.

private:
	GLuint								m_FloraTexture;			// For plants in pots.
	AuGLVAO*							m_FloraVAO;				// For plants in pots.

public:
	AuItemFloraDNA(const AuFloraDNA& dna, bool dnaViable, quint32 worldGenSeed, quint8 worldResourceZone, quint8 plantType,
			COMMODITYID produceCID, AuFloraDNA::ZONE latitudeRegion,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemFloraDNA(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemFloraDNA();
	virtual ~AuItemFloraDNA();
	AuItemFloraDNA(const AuItemFloraDNA& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemFloraDNA& c = dynamic_cast<const AuItemFloraDNA&>(item);
			if(m_PlantType < c.m_PlantType)
				return true;
			else if(m_PlantType > c.m_PlantType)
				return false;

			if(m_LatitudeRegion < c.m_LatitudeRegion)
				return true;
			else if(m_LatitudeRegion > c.m_LatitudeRegion)
				return false;

			if(m_ProduceCID < c.m_ProduceCID)
				return true;
			else if(m_ProduceCID > c.m_ProduceCID)
				return false;

			// Reversed for more desirable sorting.
			if(m_DNADead > c.m_DNADead)
				return true;
			else if(m_DNADead < c.m_DNADead)
				return false;

			if(m_DNADead < c.m_DNADead)
				return true;
			else if(m_DNADead > c.m_DNADead)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	virtual bool CanStack(const AuItem* item) const;

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	// Decreases size down to minimum of 1.
	// Returns true if size changes.
	inline bool DecreaseSize(int howMuch)
	{
		int nuSize = m_PottedSize - howMuch;
		if(nuSize < 1)
			nuSize = 1;
		else if(nuSize > 255)
			nuSize = 255;

		if(m_PottedSize == nuSize)
			return false;

		m_PottedSize = nuSize;
		return true;
	}

	virtual void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	// Returns true if this item has DNA that is not dead.
	virtual bool DNAIsViable() const
	{
		return !m_DNADead && HasDNA();
	}

	// Render all animal and plant DNA in this item and its contents to be unviable.
	virtual void DNAKillRecursive()
	{
		m_DNADead = true;
	}

	virtual AuFloraDNA FloraDNA() const
	{
		return m_DNA;
	}

	inline bool HasDNA() const
	{
		return m_DNA != AuFloraDNA();
	}

	virtual QString HtmlDesc() const;

	// Increases size to maximum of 255.
	// Returns true if size changes.
	inline bool IncreaseSize(int howMuch)
	{
		int nuSize = m_PottedSize + howMuch;
		if(nuSize < 1)
			nuSize = 1;
		else if(nuSize > 255)
			nuSize = 255;

		if(m_PottedSize == nuSize)
			return false;

		m_PottedSize = nuSize;
		return true;
	}

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemFloraDNA& c = dynamic_cast<const AuItemFloraDNA&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_WorldGenSeed != c.m_WorldGenSeed)
				return false;
			if(m_PottedGenSeed != c.m_PottedGenSeed)
				return false;
			if(m_PottedSize != c.m_PottedSize)
				return false;
			if(m_WorldResourceZone != c.m_WorldResourceZone)
				return false;
			if(m_LatitudeRegion != c.m_LatitudeRegion)
				return false;
			if(m_ProduceCID != c.m_ProduceCID)
				return false;
			if(m_PlantType != c.m_PlantType)
				return false;
			if(m_DNADead != c.m_DNADead)
				return false;
			if(m_DNA != c.m_DNA)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_FloraDNA;
	}

	inline AuFloraDNA::ZONE LatitudeRegion() const
	{
		return (AuFloraDNA::ZONE)m_LatitudeRegion;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemFloraDNA& c = dynamic_cast<const AuItemFloraDNA&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_WorldGenSeed != c.m_WorldGenSeed)
				return false;
			if(m_PottedGenSeed != c.m_PottedGenSeed)
				return false;
			if(m_PottedSize != c.m_PottedSize)
				return false;
			if(m_WorldResourceZone != c.m_WorldResourceZone)
				return false;
			if(m_LatitudeRegion != c.m_LatitudeRegion)
				return false;
			if(m_ProduceCID != c.m_ProduceCID)
				return false;
			if(m_PlantType != c.m_PlantType)
				return false;
			if(m_DNADead != c.m_DNADead)
				return false;
			if(m_DNA != c.m_DNA)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool PlantSeedInPot(const AuItem& item);

	inline quint8 PlantType() const
	{
		return m_PlantType;
	}

	inline quint32 PottedGenSeed() const
	{
		return m_PottedGenSeed;
	}

	inline quint8 PottedSize() const
	{
		return m_PottedSize;
	}

	inline COMMODITYID ProduceCommodityId() const
	{
		return (COMMODITYID)m_ProduceCID;
	}

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0, double effectScale=1.);

	// 'globeP' is the position of the globe, in eye space.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			const AuPoint& globeP,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			float shadowPanelSunAngles[MAX_SUNS],
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	inline void SetDNA(const AuFloraDNA& dna, bool dnaViable, quint32 worldGenSeed, quint8 worldResourceZone, quint8 plantType,
			COMMODITYID produceCID, AuFloraDNA::ZONE latitudeRegion)
	{
		m_DNA = dna;
		m_DNADead = !dnaViable;
		m_LatitudeRegion = latitudeRegion;
		m_PlantType = plantType;
		m_ProduceCID = produceCID;
		m_WorldResourceZone = worldResourceZone;
		m_WorldGenSeed = worldGenSeed;
	}

	inline void SetPottedSize(quint8 size)
	{
		m_PottedSize = size;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	inline quint32 WorldGenSeed() const
	{
		return m_WorldGenSeed;
	}

	inline quint32 WorldResourceZone() const
	{
		return m_WorldResourceZone;
	}

protected:
	void CreateFloraVAO(AuGLEffects& re, const AuId& id);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemPicture : public AuItem
{
protected:
	bool								m_HideURL;
	QString								m_PictureURL;
	bool								m_RotatePicture;

private:
	GLuint								m_PictureTexture;		// For plants in pots.
	AuGLVAO*							m_PictureVAO;			// For plants in pots.

public:
	AuItemPicture(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemPicture();
	virtual ~AuItemPicture();
	AuItemPicture(const AuItemPicture& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemPicture& c = dynamic_cast<const AuItemPicture&>(item);
			if(m_PictureURL < c.m_PictureURL)
				return true;
			else if(m_PictureURL > c.m_PictureURL)
				return false;

			if(m_RotatePicture < c.m_RotatePicture)
				return true;
			else if(m_RotatePicture > c.m_RotatePicture)
				return false;

			if(m_HideURL < c.m_HideURL)
				return true;
			else if(m_HideURL > c.m_HideURL)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemPicture& c = dynamic_cast<const AuItemPicture&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_PictureURL != c.m_PictureURL)
				return false;
			if(m_RotatePicture != c.m_RotatePicture)
				return false;
			if(m_HideURL != c.m_HideURL)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	inline bool IsURLHidden() const
	{
		return m_HideURL;
	}

	static quint8 ItemType()
	{
		return IT_Picture;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemPicture& c = dynamic_cast<const AuItemPicture&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_PictureURL != c.m_PictureURL)
				return false;
			if(m_RotatePicture != c.m_RotatePicture)
				return false;
			if(m_HideURL != c.m_HideURL)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline const QString& PictureURL() const
	{
		return m_PictureURL;
	}

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// 'pass' is 0 to render everything in a single pass.
	// 'pass' is 1 to render first pass, returns scene depth squared if another pass is needed.
	// 'pass' is 2 to render final pass.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0, double effectScale=1.);

	// 'globeP' is the position of the globe, in eye space.
	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			const AuPoint& globeP,
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	// Be sure to call DeleteRenderObjects() when finished.
	virtual double RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
			const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
			float fogDensity, float fogR, float fogG, float fogB,
			float shadowPanelSunAngles[MAX_SUNS],
			const AuPoint& windVec=AuPoint(0,0), float scale=1.f, int pass=0);

	inline bool RotatePicture() const
	{
		return m_RotatePicture;
	}

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	inline void SetPictureURL(const QString& pictureURL, bool hideURL)
	{
		m_PictureURL = pictureURL;
		m_HideURL = hideURL;
	}

	inline void SetRotatePicture(bool rotatePicture)
	{
		m_RotatePicture = rotatePicture;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

private:
	void RenderPicture(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuId& id);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemStateMessage : public AuItemState
{
protected:
	QString								m_Message;				// Message value for this item.

public:
	AuItemStateMessage(const QString& text, quint8 state, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
			quint32 count=1);
	AuItemStateMessage(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemStateMessage();
	AuItemStateMessage(const AuItemStateMessage& c);

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemStateMessage& c = dynamic_cast<const AuItemStateMessage&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(Message() != c.Message())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_StateMessage;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemStateMessage& c = dynamic_cast<const AuItemStateMessage&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(Message() != c.Message())
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	virtual void SetMessage(const QString& text)
	{
		m_Message = text;
	}

	virtual QString Message() const
	{
		return m_Message;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	// 0 is never a valid item type.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemTEUContainer : public AuItem
{
protected:
	AuId								m_CarrierId;			// Id of responsible avatar, if any.
	QString								m_CarrierName;			// Name of responsible avatar, if any.
	AuItemHash							m_Contents;				// Contents of this container.
	AuId								m_DestCityId;			// Destination city id.
	QString								m_DestDesc;				// Descriptive text of destination.
	AuId								m_DestSectorId;			// Destination sector id.
	AuId								m_DestSysId;			// Destination system id.
	float								m_Distance;				// Distance to destination in pc.
	AuTime								m_Pickup;				// Time of pickup.

public:
	AuItemTEUContainer(const AuId& destCityId, const QString& destDesc, float destDistPc, const AuId& destSysId,
			const AuId& destSectorId, quint32 empireOfOrigin);
	AuItemTEUContainer(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemTEUContainer();
	AuItemTEUContainer(const AuItemTEUContainer& c);

public:	// Operators
	const AuItemTEUContainer& operator=(const AuItemTEUContainer& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemTEUContainer& c = dynamic_cast<const AuItemTEUContainer&>(item);
			if(m_DestCityId > c.m_DestCityId)
				return false;
			if(m_DestDesc > c.m_DestDesc)
				return false;
			if(m_Contents.size() > c.m_Contents.size())
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	virtual AuItemHash* Cargo()
	{
		return &m_Contents;
	}

	virtual const AuItemHash* Cargo() const
	{
		return &m_Contents;
	}

	// Returns the volume of TEUCount() whole TEUs.
	virtual quint32 CargoVolume() const;

	inline const AuId& CarrierId() const
	{
		return m_CarrierId;
	}

	inline const QString& CarrierName() const
	{
		return m_CarrierName;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	inline const AuId& DestinationCity() const
	{
		return m_DestCityId;
	}

	inline const AuId& DestinationSector() const
	{
		return m_DestSectorId;
	}

	inline const AuId& DestinationSystem() const
	{
		return m_DestSysId;
	}

	inline const QString& DestinationDesc() const
	{
		return m_DestDesc;
	}

	inline float DestinationDistancePc() const
	{
		return m_Distance;
	}

	// Render all animal and plant DNA in this item and its contents to be unviable.
	virtual void DNAKillRecursive()
	{
		for(AuItemHash::iterator ci = m_Contents.begin(); ci != m_Contents.end(); ci++)
			(*ci).second->DNAKillRecursive();
	}

	AuTime EstimatedDeliveryTime() const;

	// Estimates payment once this shipment is filled with 'teuCount' TEUs of cargo.
	float EstimatePayment(quint32 teuCount) const;

	virtual QString HtmlDesc() const;

	// If the receiver defaults on payment at delivery, the shipment belongs to the carrier and it is no longer insured.
	inline bool IsInsured() const
	{
		return !m_DestCityId.IsNull();
	}

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemTEUContainer& c = dynamic_cast<const AuItemTEUContainer&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_CarrierId != c.m_CarrierId)
				return false;
			if(m_CarrierName != c.m_CarrierName)
				return false;
			if(m_DestCityId != c.m_DestCityId)
				return false;
			if(m_Distance != c.m_Distance)
				return false;
			if(m_Pickup != c.m_Pickup)
				return false;
			if(m_DestSectorId != c.m_DestSectorId)
				return false;
			if(m_DestSysId != c.m_DestSysId)
				return false;
			if(m_DestDesc != c.m_DestDesc)
				return false;
			if(m_Contents != c.m_Contents)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_TEUContainer;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemTEUContainer& c = dynamic_cast<const AuItemTEUContainer&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_CarrierId != c.m_CarrierId)
				return false;
			if(m_CarrierName != c.m_CarrierName)
				return false;
			if(m_DestCityId != c.m_DestCityId)
				return false;
			if(m_Distance != c.m_Distance)
				return false;
			if(m_Pickup != c.m_Pickup)
				return false;
			if(m_DestSectorId != c.m_DestSectorId)
				return false;
			if(m_DestSysId != c.m_DestSysId)
				return false;
			if(m_DestDesc != c.m_DestDesc)
				return false;
			if(m_Contents != c.m_Contents)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// Returns payment based on actual delivery now. Current time is the basis for bonus payments.
	float PaymentNow() const;

	// Standard price for hauling 1 TEU a distance of 1 parsec.
	static float PriceTEUPc();

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	static int RegisterMetaType();

	// 'carrierId' 'carrierName' is the id of the responsible avatar. May be null.
	// 'when' is the pick up time.
	void SetPickedUp(const AuId& carrierId, const QString& carrierName, const AuTime& when);

	void SetReceiverDefaulted();

	// Returns the delivery time in seconds, based on 2x lightspeed.
	quint32 StandardDeliveryTime() const;

	// Returns the delivery time in seconds, based on c_SpeedOfFreight adjusted for broker quality.
	float StandardPayment() const;

	// Returns the number of TEU containers needed to hold the cargo.
	inline quint32 TEUCount() const
	{
		return TEUCount(m_Contents.TotalCargoVolume());
	}

	// Returns the number of TEU containers needed to hold the 'cargo'.
	static quint32 TEUCount(quint32 cargo);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	virtual bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);

	// Recursively determines the weight of this item and all subordinate items in this container.
	// Antigravity backpack returns only the weight of the backpack itself, not including the contents.
	virtual float Weight() const;
};

Q_DECLARE_METATYPE(AuItemTEUContainer);	// Needed to use in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemStorageMedia : public AuItem
{
protected:
	AuBdDesignExchange					m_BlueprintsBd;			// Building blueprints stored on this media.
	AuScDesignExchange					m_BlueprintsSc;			// Spacecraft bBlueprints stored on this media.
	AuMissionList						m_Missions;				// Map of missions stored on this media. Key is mission name.
	QStringList							m_Papers;				// Papers stored on this media.
	AuKnownPatents						m_Patents;				// Patents stored on this media.

public:
	AuItemStorageMedia(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemStorageMedia();
	AuItemStorageMedia(const AuItemStorageMedia& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemStorageMedia& c = dynamic_cast<const AuItemStorageMedia&>(item);
			if(m_BlueprintsBd.size() < c.m_BlueprintsBd.size())
				return true;
			else if(m_BlueprintsBd.size() > c.m_BlueprintsBd.size())
				return false;
			if(m_BlueprintsSc.size() < c.m_BlueprintsSc.size())
				return true;
			else if(m_BlueprintsSc.size() > c.m_BlueprintsSc.size())
				return false;
			if(m_Missions.size() < c.m_Missions.size())
				return true;
			else if(m_Missions.size() > c.m_Missions.size())
				return false;
			if(m_Papers.size() < c.m_Papers.size())
				return true;
			else if(m_Papers.size() > c.m_Papers.size())
				return false;
			if(m_Patents.size() < c.m_Patents.size())
				return true;
			else if(m_Patents.size() > c.m_Patents.size())
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	inline const AuBdDesignExchange& BlueprintsBd() const
	{
		return m_BlueprintsBd;
	}

	inline const AuScDesignExchange& BlueprintsSc() const
	{
		return m_BlueprintsSc;
	}

	virtual bool CanCombine() const
	{
		return false;
	}

	virtual bool CanStack(const AuItem* item) const;

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual bool DeleteStoredBlueprintsBd(const QSet<quint32>& blueprints);

	virtual bool DeleteStoredBlueprintsSc(const QSet<quint32>& blueprints);

	virtual bool DeleteStoredMissions(const QSet<QString>& missions);

	virtual bool DeleteStoredPapers(const QSet<QString>& patentCIDs);

	virtual bool DeleteStoredPatents(const QSet<COMMODITYID>& patentCIDs);

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemStorageMedia& c = dynamic_cast<const AuItemStorageMedia&>(item);
			if(!AuItem::IsItem(c))
				return false;
			if(m_BlueprintsBd != c.m_BlueprintsBd)
				return false;
			if(m_BlueprintsSc != c.m_BlueprintsSc)
				return false;
			if(m_Missions != c.m_Missions)
				return false;
			if(m_Papers != c.m_Papers)
				return false;
			if(m_Patents != c.m_Patents)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	inline static quint8 ItemType()
	{
		return IT_StorageMedia;
	}

	virtual QString ListName() const;

	virtual void ListPatents(AuKnownPatents& patents) const;

	virtual quint64 MemoryUsage() const;

	const AuMissionList& Missions() const
	{
		return m_Missions;
	}

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemStorageMedia& c = dynamic_cast<const AuItemStorageMedia&>(item);
			if(!AuItem::operator==(c))
				return false;
			if(m_BlueprintsBd != c.m_BlueprintsBd)
				return false;
			if(m_BlueprintsSc != c.m_BlueprintsSc)
				return false;
			if(m_Missions != c.m_Missions)
				return false;
			if(m_Papers != c.m_Papers)
				return false;
			if(m_Patents != c.m_Patents)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	inline const QStringList& Papers() const
	{
		return m_Papers;
	}

	inline const AuKnownPatents& Patents() const
	{
		return m_Patents;
	}

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	quint8 StorageSpaceAvailable() const;

	quint8 StorageSpaceTotal() const;

	quint8 StorageSpaceUsed() const;

	virtual int StoreData(const AuItem* dataItem);

	// Returns:
	//  0=Added nu design
	//  1=no space
	//  2=blueprint already stored
	//  3=replaced existing design
	//  4=rejected duplicate named design by same architect
	//  5=unknown error
	int StoreDesign(const AuBdDesignExchangeEntry& design);
	int StoreDesign(const AuScDesignExchangeEntry& design);

	bool StoreMission(const QString& missionName, const AuMission& mission);

	// Returns false if insufficient free storage space remains.
	bool StorePaper(const QString& text);

	// Returns false if insufficient free storage space remains.
	bool StorePatent(COMMODITYID patentCID, quint8 patentQ);

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemBdDesign : public AuItem
{
protected:
	AuBdDesignExchangeEntry				m_Exchange;

public:
	AuItemBdDesign(const AuBdDesignExchangeEntry& exchange,
			COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemBdDesign(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemBdDesign();
	AuItemBdDesign(const AuItemBdDesign& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemBdDesign& c = dynamic_cast<const AuItemBdDesign&>(item);
			if(m_Exchange.DesignId() < c.m_Exchange.DesignId())
				return true;
			else if(m_Exchange.DesignId() > c.m_Exchange.DesignId())
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual quint32 DesignId() const
	{
		return m_Exchange.DesignId();
	}

	const AuBdDesignExchangeEntry& Exchange() const
	{
		return m_Exchange;
	}

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemBdDesign& c = dynamic_cast<const AuItemBdDesign&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_Exchange != c.m_Exchange)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	static quint8 ItemType()
	{
		return IT_BdDesign;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemBdDesign& c = dynamic_cast<const AuItemBdDesign&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_Exchange != c.m_Exchange)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	void SetExchange(const AuBdDesignExchangeEntry& exchange)
	{
		m_Exchange = exchange;
	}

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuItemOwnershipTitle : public AuItem
{
protected:
	AuId								m_ClaimantId;			// Id of person who claimed the title that voided this one.
	QString								m_ClaimantName;			// Name of claimant.
	AuTime								m_ClaimTime;			// When this title was claimed.
	AuId								m_PropertyId;
	QString								m_PropertyDesc;
	QString								m_PropertyName;

public:
	AuItemOwnershipTitle(const AuId& propertyId, const QString& propertyName, const QString& propertyDesc, COMMODITYID cid,
			quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemOwnershipTitle(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count=1);
	AuItemOwnershipTitle();
	AuItemOwnershipTitle(const AuItemOwnershipTitle& c);

	// Used for sorting pack contents.
	virtual bool operator<(const AuItem& item) const
	{
		try
		{
			const AuItemOwnershipTitle& c = dynamic_cast<const AuItemOwnershipTitle&>(item);
			if(m_PropertyName < c.m_PropertyName)
				return true;
			else if(m_PropertyName > c.m_PropertyName)
				return false;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}

		return AuItem::operator<(item);
	}

public:
	virtual bool CanCombine() const
	{
		return false;
	}

	virtual AuId ClaimantId() const
	{
		return m_ClaimantId;
	}

	virtual QString ClaimantName() const
	{
		return m_ClaimantName;
	}

	virtual AuTime ClaimTime() const
	{
		return m_ClaimTime;
	}

	// Returns a duplicate of this item, allocated on the heap.
	virtual AuItem* Clone() const;

	virtual QString HtmlDesc() const;

	virtual QString HtmlToolTip() const;

	// Compares fields necessary to distinquish items in a collection. Does not compare Count.
	virtual bool IsItem(const AuItem& item) const
	{
		try
		{
			const AuItemOwnershipTitle& c = dynamic_cast<const AuItemOwnershipTitle&>(item);
			if(!AuItem::IsItem(c))
				return false;

			if(m_ClaimantId != c.m_ClaimantId)
				return false;
			//if(m_ClaimantName != c.m_ClaimantName)
				//return false;
			if(m_ClaimTime != c.m_ClaimTime)
				return false;
			if(m_PropertyId != c.m_PropertyId)
				return false;
			//if(m_PropertyDesc != c.m_PropertyDesc)
				//return false;
			//if(m_PropertyName != c.m_PropertyName)
				//return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual bool IsTitleVoid() const
	{
		return !m_ClaimantId.IsNull();
	}

	static quint8 ItemType()
	{
		return IT_OwnershipTitle;
	}

	virtual QString ListName() const;

	virtual quint64 MemoryUsage() const;

	virtual bool operator==(const AuItem& item) const
	{
		try
		{
			const AuItemOwnershipTitle& c = dynamic_cast<const AuItemOwnershipTitle&>(item);
			if(!AuItem::operator==(c))
				return false;

			if(m_ClaimantId != c.m_ClaimantId)
				return false;
			if(m_ClaimantName != c.m_ClaimantName)
				return false;
			if(m_ClaimTime != c.m_ClaimTime)
				return false;
			if(m_PropertyId != c.m_PropertyId)
				return false;
			if(m_PropertyDesc != c.m_PropertyDesc)
				return false;
			if(m_PropertyName != c.m_PropertyName)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual QString PropertyDesc() const
	{
		return m_PropertyDesc;
	}

	virtual AuId PropertyId() const
	{
		return m_PropertyId;
	}

	virtual QString PropertyName() const
	{
		return m_PropertyName;
	}

	// 're' is not null when the OpenGL context is current.
	virtual void Refresh(const AuItem* item, AuGLEffects* re, AuSound* al);

	// Names tend to be formatted as 'Good Cow Milk', the way a person might say it when speaking.
	// Use ListName for formal name formatting such as 'Milk, Cow'.
	virtual QString SayName() const;

	// Returns a number that is unique to each derived AuItem class. This is NOT the commodity id.
	virtual quint8 Type() const
	{
		return ItemType();
	}

	virtual bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuItem_H
