//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augear.h,v 1.43 2022/05/06 15:56:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGear_H
#define AuGear_H

#include "audata.h"
#include "auitem.h"
#include <QList>

class AuGLEffects;
class AuSound;
class QDataStream;

class AUDATA_DLL AuGearSlot
{
public:
	enum SLOTTYPE
	{
		Invalid,						// The official invalid slot type.
		Hand,							// Almost anything can be held in hand.
		Head,							// Helmet, env helmet, hat
		Face,							// Night vision goggles, eye glasses, filter mask, goggles
		Neck,							// Necklace
		Body,							// Backpack, knapsack
		BodyWear,						// Env suit, bulletproof vest, clothing
		Limb,							// Arms and legs. Bracelet, anklet
		Digit,							// Fingers and toes. Ring
		Waist,							// Small enough items can be attached to the waist using a belt or something.
		Type_Count						// LAST ITEM MUST NOT EXCEED 16
	};

private:	// Properties
	AuItem*								m_Item;				// Gear slots only exist while they contain an item.
	quint8								m_Slot;				// SLOTTYPE
	quint8								m_Which;			// Id of slot when more than one of the same type exist. Limit 15.

public:	// Constructors
	AuGearSlot(SLOTTYPE slot, quint8 which, const AuItem& item);

	AuGearSlot();

	AuGearSlot(const AuGearSlot& c);

	~AuGearSlot();

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuGearSlot& gi) const;

	const AuGearSlot& operator=(const AuGearSlot& c);

public:	// Methods
	inline bool CanHoldItem(const AuItem& item) const
	{
		return CanHoldItem(SlotType(), item);
	}

	static bool CanHoldItem(SLOTTYPE slotType, const AuItem& item);

	inline void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, float& absorb, float& block) const
	{
		switch(SlotType())
		{
		case Invalid:
		case Type_Count:
			absorb = 0;
			block = 0;
			break;

		case Hand:
		case Body:
		case Waist:
			// Armor, helmets, clothing and hats are not effective unless worn.
			absorb = 0;
			block = 0;
			break;

		case Head:
		case Face:
		case Neck:
		case BodyWear:
		case Limb:
		case Digit:
			// Anything that can be put in these slots is effective there.
			Item().CombatArmorEffectiveness(mode, effect, absorb, block);
			break;
		}
	}

	inline int CombatArmorHits(WEAPONMODE mode, WEAPONEFFECT effect) const
	{
		switch(SlotType())
		{
		case Invalid:
		case Type_Count:
			break;

		case Hand:
		case Body:
		case Waist:
			// Armor, helmets, clothing and hats are not effective unless worn.
			break;

		case Head:
		case Face:
		case Neck:
		case BodyWear:
		case Limb:
		case Digit:
			// Anything that can be put in these slots is effective there.
			return Item().CombatArmorHits(mode, effect);
		}
		return 0;
	}

	inline COMMODITYID CommodityId() const
	{
		return Item().CommodityId();
	}

	inline quint32 Count() const
	{
		return Item().Count();
	}

	inline void DeleteRenderObjects(AuGLEffects& re, AuSound* al)
	{
		Item().DeleteRenderObjects(re, al);
	}

	// Called from empire report when items need fixing in the database. See usage.
	void FixItem();

	inline bool HasItem() const
	{
		return m_Item;
	}

	inline quint8 HitPoints() const
	{
		return Item().HitPoints();
	}

	inline bool IsItem(const AuItem& gi) const
	{
		return Item() == gi;
	}

	// Returns false for items that are not visible when selected as the current item in hand, such as environment suit or helmet.
	bool IsVisibleInHand() const;

	inline AuItem& Item()
	{
		Q_ASSERT(m_Item);		// We don't make these things without an item.
		return *m_Item;
	}

	inline const AuItem& Item() const
	{
		Q_ASSERT(m_Item);		// We don't make these things without an item.
		return *m_Item;
	}

	// Compares the factors that affect the appearance of items in the Gear window.
	inline bool ItemLooksLike(const AuItem& item) const
	{
		// Compare factors that are visible in the Gear window.
		// Icon column.
		if(Item().BitmapName() != item.BitmapName())
			return false;		// Icons differ.

		// Name column.
		if(Item().ListName() != item.ListName())
			return false;		// List name differs.

		// Size column
		if(Item().VolumeInPackUnits() != item.VolumeInPackUnits())
			return false;		// Pack size differs.

		if(Item().Content() && item.Content())
		{
			// Containers show the size of their contents.
			if(Item().Content()->ContainsPackUnits() != item.Content()->ContainsPackUnits())
				return false;	// Content size differs.
		}
		else if(Item().Content() || item.Content())
			return false;		// Only one of the items has contents.

		// Weight column.
		if(Item().Weight() != item.Weight())
			return false;		// Weight differs.

		// State does not actually appear in the tree but it affects the content of the right-click context menu.
		if(Item().State() != item.State())
			return false;		// State differs.

		// All things visible in the gear item tree match.
		return true;
	}

	quint64 MemoryUsage() const;

	inline void ReplaceItem(AuItem* item)
	{
		if(m_Item)
			delete m_Item;
		m_Item = item;
	}

	inline SLOTTYPE SlotType() const
	{
		return (SLOTTYPE)m_Slot;
	}

	inline QString SlotTypeIcon() const
	{
		return SlotTypeIcon(SlotType());
	}

	inline static QString SlotTypeIcon(SLOTTYPE slotType)
	{
		QString typeNames[] =
		{
			QString(":data/c_ParcX.png"),
			QString(":data/SlotHand.png"),			// Hand
			QString(":data/SlotHead.png"),			// Head
			QString(":data/SlotFace.png"),			// Face
			QString(":data/SlotNeck.png"),			// Neck
			QString(":data/SlotBody.png"),			// Body
			QString(":data/SlotBodyWear.png"),		// BodyWear
			QString(":data/SlotLimb.png"),			// Limb
			QString(":data/SlotDigit.png"),			// Digit
			QString(":data/SlotWaist.png"),			// Waist
		};
		return typeNames[slotType];
	}

	inline QString SlotTypeName() const
	{
		return SlotTypeName(SlotType());
	}

	inline static QString SlotTypeName(SLOTTYPE slotType)
	{
		QString typeNames[] =
		{
			QObject::tr("Invalid"),
			QObject::tr("Hand"),
			QObject::tr("Head"),
			QObject::tr("Face"),
			QObject::tr("Neck"),
			QObject::tr("Body"),
			QObject::tr("Wear"),
			QObject::tr("Limb"),
			QObject::tr("Digit"),
			QObject::tr("Waist"),
		};
		return typeNames[slotType];
	}

	// Returns a name that is used for sorting the slots in the gear tree. This text is never actually seen by anyone.
	inline static QString SlotTypeSortName(SLOTTYPE slotType)
	{
		QString typeNames[] =
		{
			QString("Z.Invalid"),
			QString("H.Hand"),
			QString("A.Head"),
			QString("B.Face"),
			QString("C.Neck"),
			QString("I.Body"),
			QString("D.Wear"),
			QString("E.Limb"),
			QString("F.Digit"),
			QString("G.Waist"),
		};
		return typeNames[slotType];
	}

	inline quint8 Which() const
	{
		return m_Which;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuGearSlot& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGearSlot& o)
{
	o << in;
	return in;
}

inline unsigned int qHash(const AuGearSlot& key)
{
	return (unsigned int)key.SlotType();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuGearPos
{
private:	// Properties
	QList<AuItemContentId>				m_ContentPath;			// Id path to item inside container(s). Last id is the item.
	qint8								m_Slot;					// SLOTTYPE of the gear slot. < 0 for body weapon positions.
	quint8								m_Which;				// Id of slot when more than one of the same type exist. Limit 15.

public:	// Constructors
	// Constructor for gear position referencing an item in inventory.
	inline AuGearPos(const AuGearSlot& slot, const AuItemContentId& contentId=AuItemContentId())
	{
		if(contentId.IsValid())
			m_ContentPath.append(contentId);
		m_Slot = slot.SlotType();
		m_Which = slot.Which();
	}

	// Constructor for gear position referencing an item in inventory.
	inline AuGearPos(AuGearSlot::SLOTTYPE slot, quint8 which, const AuItemContentId& contentId=AuItemContentId())
	{
		if(contentId.IsValid())
			m_ContentPath.append(contentId);
		m_Slot = slot;
		m_Which = which;
	}

	// Handy constructor for building pos of an item inside a container.
	// Adds to sub id list for nested items.
	inline AuGearPos(const AuGearPos& c, const AuItemContentId& contentId) : m_ContentPath(c.m_ContentPath)
	{
		if(contentId.IsValid())
		{
			Q_ASSERT(m_ContentPath.size() < 255);	// Size must fit into a byte for streamer.
			m_ContentPath.append(contentId);
		}
		m_Slot = c.m_Slot;
		m_Which = c.m_Which;
	}

	// Constructor for gear position referencing person body weapon.
	inline AuGearPos(INVENTORYPOS bodyPos)
	{
		m_Slot = bodyPos;
		m_Which = 0;
		Q_ASSERT(IsBody());
	}

	inline AuGearPos()
	{
		m_Slot = AuGearSlot::Invalid;
		m_Which = 0;
	}

	inline AuGearPos(const AuGearPos& c) : m_ContentPath(c.m_ContentPath)
	{
		m_Slot = c.m_Slot;
		m_Which = c.m_Which;
	}

public:	// Operators
	inline const AuGearPos& operator=(const AuGearPos& c)
	{
		m_ContentPath = c.m_ContentPath;
		m_Slot = c.m_Slot;
		m_Which = c.m_Which;
		return *this;
	}

	inline bool operator==(const AuGearPos& c) const
	{
		if(m_Slot != c.m_Slot)
			return false;
		if(m_Which != c.m_Which)
			return false;
		if(m_ContentPath != c.m_ContentPath)
			return false;
		return true;
	}

	inline bool operator!=(const AuGearPos& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Slot;

		quint8 w = m_Which;
		if(!m_ContentPath.isEmpty())
			w |= 0x80;

		out << w;
		if(w & 0x80)
		{
			out << (quint8)m_ContentPath.size();
			for(QList<AuItemContentId>::const_iterator si = m_ContentPath.cbegin(); si != m_ContentPath.cend(); si++)
				out << *si;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Slot;

		quint8 w;
		in >> w;

		m_Which = w & 0x0f;

		m_ContentPath.clear();
		if(w & 0x80)
		{
			quint8 c;
			in >> c;

			AuItemContentId iid; 
			while(c--)
			{
				in >> iid;
				m_ContentPath.append(iid);
			}
		}
	}

public:	// Methods
	// The negative INVENTORYPOS values correspond 1:1 with some negative CID_ values for body parts.
	inline INVENTORYPOS BodyPos() const
	{
		if(!IsBody())
			return IP_NumInventoryPos;
		return (INVENTORYPOS)m_Slot;
	}

	// Encode slot and which for person's current gear item field in db. Does not incorporate sub id.
	inline quint16 EncodeU16() const
	{
		return (quint16)((m_Which << 8) | (quint8)m_Slot);
	}

	static AuGearPos DecodeU16(quint16 encodedU16)
	{
		AuGearPos decPos;
		decPos.m_Slot = (qint8)(encodedU16 & 0xff);
		decPos.m_Which = (quint8)((encodedU16 >> 8) & 0x000f);
		return decPos;
	}

	// True if this pos references a personal body position or a mount's body position.
	// Body positions show up as gear positions with a specific range of negative slot values. They are never inside containers.
	inline bool IsBody() const
	{
		return m_Slot >= IP_Mount_Anus && m_Slot <= IP_My_Torso && IsTopLevel();
	}

	// True if this is a body weapon on a mount.
	// Gear position is used to select body weapons of an avatar and their mount, in addition to equipment in a gear vector.
	inline bool IsMountBody() const
	{
		return m_Slot >= IP_Mount_Anus && m_Slot <= IP_Mount_Torso && IsTopLevel();
	}

	// True if this is a body weapon on the avatar.
	// Gear position is used to select body weapons of an avatar and their mount, in addition to equipment in a gear vector.
	inline bool IsPersonalBody() const
	{
		return m_Slot >= IP_My_Anus && m_Slot <= IP_My_Torso && IsTopLevel();
	}

	// A top level item is one that is not inside a container.
	inline bool IsTopLevel() const
	{
		return m_ContentPath.isEmpty();
	}

	// Returns true if this pos value could be used as a reference into a gear object.
	// Certain negative slot values are used for body weapons. See IsBodyPos.
	inline bool IsGear() const
	{
		return m_Slot > AuGearSlot::Invalid && m_Slot < AuGearSlot::Type_Count;
	}

	inline bool IsValid() const
	{
		return IsGear() || IsBody();
	}

	// The negative INVENTORYPOS values correspond 1:1 with some negative CID_ values for body parts.
	// Returns the mount's body weapon CID, translated to be in the My body weapon range of CIDs.
	inline INVENTORYPOS MountBodyWeaponPos() const
	{
		if(!IsMountBody())
			return IP_NumInventoryPos;
		return (INVENTORYPOS)(m_Slot - (IP_Mount_Torso - IP_My_Torso));
	}

	// The negative INVENTORYPOS values correspond 1:1 with some negative CID_ values for body parts.
	inline INVENTORYPOS PersonalBodyWeaponPos() const
	{
		if(!IsPersonalBody())
			return IP_NumInventoryPos;
		return (INVENTORYPOS)m_Slot;
	}

	static int RegisterMetaType();

	inline AuGearSlot::SLOTTYPE Slot() const
	{
		if(!IsGear())
			return AuGearSlot::Invalid;
		return (AuGearSlot::SLOTTYPE)m_Slot;
	}

	// The id path when inside a container. List is empty for top level items.
	// First item in the list is the beginning of the path in the container at Slot/Which.
	// Last item in the list is the item referenced by this gear pos.
	inline const QList<AuItemContentId>& ContentPath() const
	{
		return m_ContentPath;
	}

	inline quint8 Which() const
	{
		return m_Which;
	}
};

Q_DECLARE_METATYPE(AuGearPos)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuGearPos& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGearPos& o)
{
	o << in;
	return in;
}

inline unsigned int qHash(const AuGearPos& key)
{
	Q_ASSERT(false);	// This is only needed to make Windows happy. I don't think it is used.
	return (key.Slot() << 16) | (key.Which() << 8);
}

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuGearPos& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuGearPosList : public QList<AuGearPos>
{
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuGear : public QList<AuGearSlot>
{
private:	// Properties

public:	// Constructors
	AuGear();
	~AuGear();
	AuGear(const AuGear& c);

public:	// Operators
	const AuGear& operator=(const AuGear& gear);

	bool operator==(const AuGear& gear) const;

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	const AuItem* ArmorItemWorn() const;

	// Returns CID of armor that is worn on the body, CID_Invalid if none is worn.
	COMMODITYID ArmorWorn() const;

	// Returns CID of armor that is worn on the body, clothing if no armor is worn, CID_Invalid if neither is worn.
	COMMODITYID BodyArmorOrClothingWorn() const;

	inline void Clear()
	{
		clear();
	}

	const AuItem* ClothingItemWorn() const;

	// Returns CID of clothing that is worn on the body, armor if no clothing is worn, CID_Invalid if neither is worn.
	COMMODITYID ClothingOrBodyArmorWorn() const;

	// Returns CID of clothing that is worn on the body, CID_Invalid if none is worn.
	COMMODITYID ClothingWorn() const;

	// Apply damage to armor caused by 'effect'.
	// 'damage' is the total amount of damage attributed to hitting the armor in this gear, after CombatArmorAbsorption.
	// The actual damage taken by armor items is 1/10th of that, 1/20th when absorbing their specialty effect.
	// Returns 'true' if any item is changed.
	// Armor is never destroyed by taking damage. It looks to weird to blast a dude's armor and clothing off bit by bit.
	inline bool CombatApplyDamage(WEAPONMODE mode, WEAPONEFFECT effect, int damage)
	{
		//qDebug() << "AuGear::CombatApplyDamage effect" << effect << "k" << kineticDamage << "e" << energyDamage;
		// Tally hit points of armor that is effective against 'mode' 'effect' hits of damage.
		int armorHits = CombatArmorHits(mode, effect);
		if(!armorHits)
			return false;

		// Distribute damage across all armor items.
		bool anyChange = false;
		int ipos = 0;
		for(iterator it = begin(); it != end(); it++, ipos++)
		{
			int itemHits = (*it).CombatArmorHits(mode, effect);
			if(itemHits)
			{
				// The item would absorb some or all of the mode effect damage.
				// Calculate the armor item's fraction of the damage.
				int hitsDamage = damage * itemHits / armorHits; 

				// Apply damage to the item.
				if(hitsDamage && (*it).Item().CombatApplyDamage(effect, hitsDamage))
					anyChange = true;						// The armor item's condition was degraded.
			}
		}
		
		return anyChange;
	}

	// Return total combat armor factor values due to items in this gear.
	// 'absorb' is the fraction of overall damage that is absorbed by armor, in the range 0 to 1.
	// 'block' is the quantity of damage points that are blocked by armor, in the range 0 to n.
	inline void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, float& absorb, float& block) const
	{
		absorb = 0;
		block = 0;
		float itemAbsorb = 0;
		float itemBlock = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
		{
			(*it).CombatArmorEffectiveness(mode, effect, itemAbsorb, itemBlock);
			absorb += itemAbsorb;
			block += itemBlock;
		}
	}

	// Return total hit points of combat armor remaining in this gear that can absorb 'mode' 'effect' hits of damage.
	inline int CombatArmorHits(WEAPONMODE mode, WEAPONEFFECT effect) const
	{
		int armorHits = 0;
		for(const_iterator it = cbegin(); it != cend(); it++)
			armorHits += (*it).CombatArmorHits(mode, effect);			// The armor is in a valid usage position.
		return armorHits;
	}

	// Combine items into stacks where possible.
	// Looks in the first level of containers.
	// Does not list containers unless they are empty.
	void CombineItemsForSale(AuItemHash& combined) const;

	// Returns the number of 'item's that are available. i.e. Non-empty containers are not available.
	// This is used when bartering and is specifically followed by RemoveItems, which must use the same criteria.
	unsigned int CountAvailableItems(const AuItem& item) const;

	bool DeleteStoredBlueprintsBd(const AuGearPos& pos, const QSet<quint32>& designs);

	bool DeleteStoredBlueprintsSc(const AuGearPos& pos, const QSet<quint32>& designs);

	bool DeleteStoredMissions(const AuGearPos& pos, const QSet<QString>& missions);

	bool DeleteStoredPapers(const AuGearPos& pos, const QSet<QString>& papers);

	bool DeleteStoredPatents(const AuGearPos& pos, const QSet<COMMODITYID>& patentCIDs);

	void DeleteRenderObjects(AuGLEffects& re, AuSound* al);

	// Destructs items associated with stories that are no longer active. Looks in containers, recursively.
	// Returns true if any items are destroyed.
	bool DestructStoryItems();

	inline void Erase(const AuGearPos& pos)
	{
		iterator ii = FindPos(pos);
		if(ii == end())
			return;

		if(pos.IsTopLevel())
			erase(ii);
		else
		{
			// Erase a contained item.
			AuItemContent* srcContents = (*ii).Item().Content();
			if(srcContents)
				srcContents->Erase(pos.ContentPath());
		}
	}

	AuGearPos FindArmorWorn() const;

	// Locates the best item of a particular commodity type 'cid'.
	// Looks inside containers.
	// Sets 'pos' and 'subPos' to the item that is found. 'subPos' is QPoint(255,255) when 'pos' is the top level item.
	// true returned if any item is found of type 'cid'.
	inline AuGearPos FindBest(COMMODITYID cid) const
	{
		AuGearPos pos;
		quint8 bestQ = 0;
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).Item().CommodityId() == cid)
			{
				// Found an item of the type we are looking for.
				if(bestQ == 0 || (*ii).Item().Quality() > bestQ)
				{
					pos = AuGearPos(*ii);								// top level item.
					bestQ = (*ii).Item().Quality();
				}
			}

			const AuItemContent* contents = (*ii).Item().Content();
			if(contents)
			{
				AuGearPos spos = contents->FindBest(AuGearPos(*ii), cid, bestQ);
				if(spos.IsValid())
					pos = spos;
			}
		}

		return pos;
	}

	// Like FindBest but only checks top level items.
	AuGearPos FindBestTop(COMMODITYID cid) const;

	AuGearPos FindClothingWorn() const;

	// Find food for consumption by a specific ecological role.
	AuGearPos FindFood(ECOROLE ecoRole) const;

	AuGearPos FindHatWorn() const;

	AuGearPos FindHelmetWorn() const;

	// Finds a commodity using operator== test.
	// return 'end()' on failure.
	inline iterator FindItem(const AuItem& item)
	{
		for(iterator gi = begin(); gi != end(); gi++)
			if((*gi).IsItem(item))
				return gi;
		return end();
	}

	// Finds a commodity using operator== test.
	// return 'cend()' on failure.
	inline const_iterator FindItem(const AuItem& item) const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if((*gi).IsItem(item))
				return gi;
		return cend();
	}

	// Finds a commodity that is in a specific slot type.
	// return 'end()' on failure.
	inline iterator FindItemInSlot(const AuItem& item, AuGearSlot::SLOTTYPE slotType)
	{
		for(iterator gi = begin(); gi != end(); gi++)
			if((*gi).SlotType() == slotType && (*gi).IsItem(item))
				return gi;
		return end();
	}

	// Finds a commodity that is in a specific slot type.
	// return 'cend()' on failure.
	inline const_iterator FindItemInSlot(COMMODITYID cid, AuGearSlot::SLOTTYPE slotType, quint8 slotWhich) const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if((*gi).SlotType() == slotType && (*gi).Which() == slotWhich && (*gi).CommodityId() == cid)
				return gi;
		return cend();
	}

	// Finds a commodity using IsItem test.
	inline AuGearPos FindItemPos(const AuItem& item) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).Item().IsItem(item))
				return AuGearPos(*ii);						// Found item at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents)
				{
					AuGearPos spos = contents->FindItemPos(AuGearPos(*ii), item);
					if(spos.IsValid())
						return spos;
				}
			}
		}
		return AuGearPos();
	}

	// Finds a commodity that is in a specific slot type.
	// Compares the factors that affect the appearance of items in the Gear window.
	// operator== doesn't work due to lazy resolution of specie names.
	// return 'end()' on failure.
	inline iterator FindLooksLikeItemInSlot(const AuItem& item, AuGearSlot::SLOTTYPE slotType)
	{
		for(iterator gi = begin(); gi != end(); gi++)
			if((*gi).SlotType() == slotType)
				if((*gi).ItemLooksLike(item))
					return gi;	// All things visible in the gear item tree match.
		return end();
	}

	// Locates the best item of a particular commodity type 'cid'.
	// Looks inside containers.
	// Sets 'pos' and 'subPos' to the item that is found. 'subPos' is QPoint(255,255) when 'pos' is the top level item.
	// true returned if any item is found of type 'cid'.
	AuGearPos FindReadyItem(COMMODITYID cid) const;

	inline iterator FindPos(const AuGearPos& pos)
	{
		for(iterator gi = begin(); gi != end(); gi++)
			if((*gi).SlotType() == pos.Slot() && (*gi).Which() == pos.Which())
				return gi;
		return end();
	}

	inline const_iterator FindPos(const AuGearPos& pos) const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if((*gi).SlotType() == pos.Slot() && (*gi).Which() == pos.Which())
				return gi;
		return cend();
	}

	// Called from empire report when items need fixing in the database. See usage.
	void FixItems();

	// Looks at top-level items and into containers to determine if any food is in this gear.
	inline bool HasFood(ECOROLE ecoRole) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).Item().IsFood(ecoRole) && (*ii).Item().FoodNutritionValue())
				return true;			// Found food at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents && contents->HasFood(ecoRole))
					return true;	// Found food in a container.
			}
		}
		return false;
	}

	// Looks at top-level items and into containers recursively to determine if a commodity is in this gear.
	inline bool HasItem(COMMODITYID cid) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).CommodityId() == cid)
				return true;			// Found item at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents && contents->HasItem(cid))
					return true;
			}
		}
		return false;
	}

	// Looks at top-level items and into containers to determine if a commodity is in this gear.
	inline bool HasItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).Item().CommodityId() == cid &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin &&
					(*ii).Item().Quality() == quality)
				return true;			// Found item at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents && contents->HasItem(cid, quality, empireOfOrigin))
					return true;	// Found item in a container.
			}
		}
		return false;
	}

	// Looks at top-level items and into containers to determine if a commodity is in this gear.
	inline bool HasItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
			const QString& inscription) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).Item().CommodityId() == cid &&
					(*ii).Item().Quality() == quality &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin &&
					(*ii).Item().Inscription() == inscription)
				return true;			// Found item at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents && contents->HasItem(cid, quality, empireOfOrigin, inscription))
					return true;	// Found item in a container.
			}
		}
		return false;
	}

	// Looks at top-level items and into containers to determine if a commodity is in this gear.
	// Returns the total count of the commodity in this gear, including in containers.
	inline int HasItemCount(COMMODITYID cid) const
	{
		int count = 0;
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).CommodityId() == cid)
				count += (*ii).Item().Count();						// Found item at top level.

			const AuItemContent* contents = (*ii).Item().Content();
			if(contents)
				count += contents->HasItemCount(cid);
		}
		return count;
	}

	inline bool HasItemInSlot(AuGearSlot::SLOTTYPE slotType, COMMODITYID cid) const
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			if((*ii).SlotType() != slotType)
				continue;

			if((*ii).CommodityId() == cid)
				return true;			// Found item at top level.
			else
			{
				const AuItemContent* contents = (*ii).Item().Content();
				if(contents)
				{
					// Look inside container.
					for(AuItemContent::const_iterator ci = contents->cbegin(); ci != contents->cend(); ci++)
						if((*ci).second->CommodityId() == cid)
							return true;	// Found item in a container.
				}
			}
		}
		return false;
	}

	const AuItem* HatItemWorn() const;

	const AuItem* HelmetItemWorn() const;

	// Returns CID of helmet that is worn on the body, hat if no helmet is worn, CID_Invalid if neither is worn.
	COMMODITYID HelmetOrHatWorn() const;

	inline void Insert(AuGearSlot::SLOTTYPE slotType, quint8 which, const AuItem& item)
	{
		if(slotType == AuGearSlot::Invalid)
			return;
		push_back(AuGearSlot(slotType, which, item));
	}

	inline bool IsPosValid(const AuGearPos& pos) const
	{
		// Locate the top level item at pos.
		const_iterator ii = FindPos(pos);
		if(ii == cend())
			return false;
		if(pos.IsTopLevel())
			return true;

		// Test sub item id for valid range.
		const AuItem& item = (*ii).Item();
		if(!item.Content() || !item.Content()->IsValid(pos.ContentPath()))
			return false;

		return true;
	}

	inline const AuItem* ItemAt(const AuGearPos& pos) const
	{
		const_iterator ii = FindPos(pos);
		if(ii == cend())
			return 0;

		const AuItem& item = (*ii).Item();
		if(pos.IsTopLevel())
			return &item;

		if(!item.Content())
			return 0;

		return item.Content()->ItemAt(pos.ContentPath());
	}

	inline AuItem* ItemAt(const AuGearPos& pos)
	{
		iterator ii = FindPos(pos);
		if(ii == end())
			return 0;

		AuItem& item = (*ii).Item();
		if(pos.IsTopLevel())
			return &item;

		if(!item.Content())
			return 0;

		return item.Content()->ItemAt(pos.ContentPath());
	}

	// Returns the first item found to be in the slot type.
	const AuItem* ItemInSlot(AuGearSlot::SLOTTYPE slotType) const;

	AuItem* ItemInSlot(AuGearSlot::SLOTTYPE slotType);

	// Returns the number of items occupying the slot type.
	// 'nextWhich' is set to the next available which id, when it is not null.
	quint8 ItemsInSlot(AuGearSlot::SLOTTYPE slotType, quint8* nextWhich) const;

	// Searches for an item in the slot type.
	// Returns the pos of the first item found.
	inline AuGearPos ItemSlotPos(AuGearSlot::SLOTTYPE slotType) const
	{
		if(slotType == AuGearSlot::Invalid)
			return AuGearPos();

		// Search for a slot of the type requested.
		for(const_iterator ii = cbegin(); ii < cend(); ii++)
			if((*ii).SlotType() == slotType)
				return AuGearPos(*ii);

		// No item was found in the slot type.
		return AuGearPos();
	}

	inline const AuItem* ItemVisibleInHandAt(const AuGearPos& pos) const
	{
		if(!IsPosValid(pos))
			return 0;

		if(pos.IsTopLevel())
		{
			// Top level items might not be visible in hand when selected as current item.
			const_iterator ii = FindPos(pos);
			if(!(*ii).IsVisibleInHand())
				return 0;
			const AuItem& item = (*ii).Item();
			return &item;
		}
		else
		{
			// Sub items are not worn so they are always visible in hand when selected as current item. Not sure if they can be
			// selected as current item in hand, though.
			return ItemAt(pos);
		}
	}

	// Locates a top level light that is on.
	AuGearPos LightOnPos() const;

	// Recursive
	void ListAll(AuGearPosList& gearPos) const;

	// Recursive
	void ListItems(COMMODITYID cid, AuGearPosList& gearPos) const;

	// Recursive
	void ListPatents(AuKnownPatents& patents) const;

	// Compares the factors that affect the appearance of items in the Gear window.
	// operator== doesn't work due to lazy resolution of specie names.
	bool LooksLike(const AuGear& gear) const;

	quint64 MemoryUsage() const;

	// Returns true if the item at 'srcPos' is succefully moved to a new empty top level slot of 'destSlotType'.
	bool MoveItem(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType);

	// Return value:
	// 0 = Item could not be moved.
	// 1 = Item moved.
	// 2 = Source item is ammo that reloaded a destination weapon.
	// 3 = Source item is a seed that was planted in a flower pot.
	// 10+1 = The data item exceeds the space available on this storage media.
	// 10+2 = The data item cannot be stored onto this storage media.
	// 10+3 = Source item patent stored successfully.
	// 10+4 = Source item spacecraft blueprint stored successfully.
	// 10+5 = Source item building blueprint stored successfully.
	// 10+6 = Source item data transferred.
	// 10+7 = Source item patent (or better) already contained on this storage media.
	// 10+8 = Source item blueprint already contained on this storage media.
	// 10+9 = Data item cannot be copied due to publisher restrictions.
	// 10+10 = Data on one of the items in the 'dataItem' storage container cannot be copied due to publisher restrictions.
	// 10+11 = Source item mission stored successfully.
	int MoveItem(const AuGearPos& srcPos, const AuGearPos& destPos);

	// Returns an available which id for a slot type.
	quint8 NextWhich(AuGearSlot::SLOTTYPE slotType) const;

	// 're' is not null when the OpenGL context is current.
	void Refresh(const AuGear& gear, AuGLEffects* re, AuSound* al);

	// Tries to reload an item using another object found in this gear.
	// Returns true if the item is reloaded from another item in this gear.
	bool ReloadItem(const AuGearPos& pos, INVENTORYSUB itemSub);

	// Tries to reload an item using another object found in this gear.
	// Returns true if the item is reloaded from another item in this gear.
	bool ReloadItem(const AuGearPos& pos, INVENTORYSUB itemSub, QString& ammoDesc);

	// Removes item.Count() items from this gear.
	// This is used when bartering and is specifically validated using CountAvailableItems, which must use the same criteria.
	void RemoveItems(const AuItem& item);

	// Takes ownership of 'item'.
	// Does nothing if there is not already an item at pos.
	void ReplaceItem(const AuGearPos& pos, AuItem* item);

	inline void StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).Item().StoryInstance() == storyInstance &&
					(*ii).Item().CommodityId() == cid &&
					(*ii).Item().Quality() == quality &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin)
				(*ii).Item().SetStoryInstance(0);

			// Recursive search into all containers.
			if((*ii).Item().Content())
				(*ii).Item().Content()->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin);
		}
	}

	inline void StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, const QString& inscription)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).Item().StoryInstance() == storyInstance &&
					(*ii).Item().CommodityId() == cid &&
					(*ii).Item().Quality() == quality &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin &&
					(*ii).Item().Inscription() == inscription)
				(*ii).Item().SetStoryInstance(0);

			// Recursive search into all containers.
			if((*ii).Item().Content())
				(*ii).Item().Content()->StoryItemMakePermanent(storyInstance, cid, quality, empireOfOrigin,
						inscription);
		}
	}

	// 'drop' 0=default random drop, 1=never drop, 2=always drop
	inline void StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).Item().StoryInstance() == storyInstance &&
					(*ii).Item().CommodityId() == cid &&
					(*ii).Item().Quality() == quality &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin)
				(*ii).Item().SetDropOnDeath(drop);

			// Recursive search into all containers.
			if((*ii).Item().Content())
				(*ii).Item().Content()->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin);
		}
	}

	inline void StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality,
			quint32 empireOfOrigin, const QString& inscription)
	{
		for(iterator ii = begin(); ii != end(); ii++)
		{
			if((*ii).Item().StoryInstance() == storyInstance &&
					(*ii).Item().CommodityId() == cid &&
					(*ii).Item().Quality() == quality &&
					(*ii).Item().EmpireOfOrigin() == empireOfOrigin &&
					(*ii).Item().Inscription() == inscription)
				(*ii).Item().SetDropOnDeath(drop);

			// Recursive search into all containers.
			if((*ii).Item().Content())
				(*ii).Item().Content()->StoryItemSetDropOnDeath(drop, storyInstance, cid, quality, empireOfOrigin,
						inscription);
		}
	}

	// Voids all ownership titles that refer to 'propertyId'.
	// Returns true if anything changes.
	bool VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName);

	// Searches for a weapon in the slot type.
	// Returns the pos of the first weapon found.
	inline AuGearPos WeaponSlotPos(AuGearSlot::SLOTTYPE slotType) const
	{
		if(slotType == AuGearSlot::Invalid)
			return AuGearPos();

		// Search for a slot of the type requested.
		for(const_iterator ii = cbegin(); ii < cend(); ii++)
			if((*ii).SlotType() == slotType && (*ii).Item().IsWeaponByDesign())
				return AuGearPos(*ii);

		// Search for a slot of the type requested.
		for(const_iterator ii = cbegin(); ii < cend(); ii++)
			if((*ii).SlotType() == slotType && (*ii).Item().IsWeapon())
				return AuGearPos(*ii);

		// No weapon was found in the slot type.
		return AuGearPos();
	}

	float Weight() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuGear& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGear& o)
{
	o << in;
	return in;
}

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuGear& o);

#endif // AuGear_H
