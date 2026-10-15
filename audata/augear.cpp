//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augear.cpp,v 1.36 2022/05/06 17:01:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augear.h"
//
#include "auglobal.h"
#include "auitem.h"
#include "auknownpatents.h"
#include <QDebug>

static int s_AuGearPosMetaType = AuGearPos::RegisterMetaType();

//static
int AuGearPos::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuGearPos>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuGearPos>();

	return metaType;
}

QDebug operator<<(QDebug dbg, const AuGearPos& o)
{
	if(o.IsBody())
		dbg.nospace() << "(IPos " << o.BodyPos() << " " << o.Which() << ")";
	else if(o.IsTopLevel())
		dbg.nospace() << "(" << AuGearSlot::SlotTypeName(o.Slot()) << " " << o.Which() << ")";
	else
		dbg.nospace() << "(" << AuGearSlot::SlotTypeName(o.Slot()) << " " << o.Which() << " path " << o.ContentPath() << ")";
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGearSlot::AuGearSlot(SLOTTYPE slot, quint8 which, const AuItem& item)
{
	m_Item = item.Clone();
	m_Slot = slot;
	m_Which = which;
}

AuGearSlot::AuGearSlot()
{
	m_Item = 0;
	m_Slot = Invalid;
	m_Which = 0;
}

AuGearSlot::AuGearSlot(const AuGearSlot& c)
{
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
	m_Slot = c.m_Slot;
	m_Which = c.m_Which;
}

AuGearSlot::~AuGearSlot()
{
	if(m_Item)
		delete m_Item;
}

const AuGearSlot& AuGearSlot::operator=(const AuGearSlot& c)
{
	if(m_Item)
		delete m_Item;
	if(c.m_Item)
		m_Item = c.m_Item->Clone();
	else
		m_Item = 0;
	m_Slot = c.m_Slot;
	m_Which = c.m_Which;

	return *this;
}

void AuGearSlot::operator>>(QDataStream& out) const
{
	quint8 whichSlot = (m_Which << 4) | m_Slot;	// Top 4 bits is which id, bottom 4 bits is slot type.
	out << whichSlot;

	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
}

void AuGearSlot::operator<<(QDataStream& in)
{
	quint8 whichSlot;
	in >> whichSlot;

	m_Slot = whichSlot & 0x0f;	// Slot type is bottom 4 bits.
	m_Which = (whichSlot >> 4);	// Which slot is top 4 bits.

	quint8 itemType;
	in >> itemType;
	//qDebug() << "AuGearSlot::operator<< slot" << m_Slot << "which" << m_Which << "type" << itemType << "inpos" << in.device()->pos();
	if(itemType)
	{
		m_Item = AuItem::Create(itemType);
		if(m_Item)
		{
			in >> *m_Item;

			// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
			AuItem* replacement = m_Item->FixItem();
			if(replacement)
			{
				delete m_Item;
				m_Item = replacement;
			}
		}
		else
			m_Item = 0;
	}
	else
		m_Item = 0;
}

bool AuGearSlot::operator==(const AuGearSlot& gi) const
{
	if(m_Slot != gi.m_Slot)
		return false;
	if(m_Which != gi.m_Which)
		return false;
	if(!m_Item && !gi.m_Item)
		return true;
	if(!m_Item || !gi.m_Item)
		return false;
	return *m_Item == *gi.m_Item;
}

//static
bool AuGearSlot::CanHoldItem(SLOTTYPE slotType, const AuItem& item)
{
	if(!item.CanBeCarried())
		return false;

	switch(slotType)
	{
	case Invalid:
	case Type_Count:
		break;
	case Hand:
	case Body:
		return true;
	case Head:
		return item.IsWornOnHead();
	case Face:
		return item.IsWornOnFace();
	case Neck:
		return item.IsWornOnNeck();
	case BodyWear:
		return item.IsWornOnBody();
	case Limb:
		return item.IsWornOnLimb();
	case Digit:
		return item.IsWornOnDigit();
	case Waist:
		return item.VolumeInPackUnits() <= 15;	// Small enough items can be carried on the waist.
	}
	return false;
}

quint64 AuGearSlot::MemoryUsage() const
{
	quint64 mem = sizeof(AuGearSlot);
	if(m_Item)
		mem += m_Item->MemoryUsage();
	return mem;
}

bool AuGearSlot::IsVisibleInHand() const
{
	switch(SlotType())
	{
	case Invalid:
	case Type_Count:
		break;

	case BodyWear:
	case Face:
	case Head:
		break;			// Not shown in the hand when selected as the current item.

	case Body:
	case Hand:
	case Neck:
	case Waist:
	case Limb:
	case Digit:
		return true;	// Shown in the hand when selected as the current item.
	}
	return false;
}

void AuGearSlot::FixItem()
{
	AuItem* replacement = m_Item->FixItem();
	if(replacement)
	{
		delete m_Item;
		m_Item = replacement;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGear::AuGear()
{
}

AuGear::~AuGear()
{
}

AuGear::AuGear(const AuGear& c) : QList<AuGearSlot>(c)
{
}

const AuGear& AuGear::operator=(const AuGear& c)
{
	clear();
	for(const_iterator gi = c.cbegin(); gi != c.cend(); gi++)
		append(*gi);
	return *this;
}

void AuGear::operator>>(QDataStream& out) const
{
	out << (quint8)size();
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
		out << *gi;
}

void AuGear::operator<<(QDataStream& in)
{
	clear();
	quint8 c;
	in >> c;
	//qDebug() << "AuGear::operator<< count" << c << in.device()->bytesAvailable() << in.device()->atEnd() << in.device()->pos();
	while(c--)
	{
		AuGearSlot gi;
		in >> gi;
		if(gi.HasItem())
			append(gi);
		else
			qWarning() << "warning: AuGear::operator<< slot item invalid" << c << "size" << size() << in.device()->bytesAvailable();
	}
}

bool AuGear::operator==(const AuGear& c) const
{
	if(size() != c.size())
		return false;

	// Side-by-side comparison does not work because the order can differ though the contents are identical.
	// Make a duplicate of the comparison gear so we can insure 1:1 comparison of items in the presense of multiple identical items.
	AuGear cc(c);
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		iterator ci = cc.FindItemInSlot((*gi).Item(), (*gi).SlotType());	// The slot contains an item.
		if(ci == cc.end())
		{
			//qDebug() << "AuGear::operator== failed, item not found" << (*gi).Item().ListName() << "in" << (*gi).SlotTypeName();
			return false;			// Corresponding identical item not found.
		}
		cc.erase(ci);				// Remove the matching item to insure correct results in case of multiple identical items.
	}

	return true;
}

void AuGear::Refresh(const AuGear& c, AuGLEffects* re, AuSound* al)
{
	// Do nothing if nothing inside this container changed.
	if(operator==(c))
		return;

	// Content changed.
	if(re)
		DeleteRenderObjects(*re, al);

	// Replace myself with c.
	operator=(c);
}

void AuGear::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	for(iterator ci = begin(); ci != end(); ci++)
		(*ci).DeleteRenderObjects(re, al);
}

COMMODITYID AuGear::BodyArmorOrClothingWorn() const
{
	COMMODITYID clothing = CID_Invalid;
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Body armor and clothing are only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsBodyArmor())
			return (*gi).CommodityId();
		else if((*gi).Item().IsClothing())
			clothing = (*gi).CommodityId();
	}
	return clothing;
}

COMMODITYID AuGear::ClothingOrBodyArmorWorn() const
{
	COMMODITYID armor = CID_Invalid;
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Body armor and clothing are only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsClothing())
			return (*gi).CommodityId();
		else if((*gi).Item().IsBodyArmor())
			armor = (*gi).CommodityId();
	}
	return armor;
}

COMMODITYID AuGear::HelmetOrHatWorn() const
{
	COMMODITYID hat = CID_Invalid;
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Helmets and hats are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::Head)
			continue;

		if((*gi).Item().IsHelmet())
			return (*gi).CommodityId();
		else if((*gi).Item().IsHat())
			hat = (*gi).CommodityId();
	}
	return hat;
}

AuGearPos AuGear::FindArmorWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Hats are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsBodyArmor())
			return AuGearPos(*gi);
	}
	return AuGearPos();
}

AuGearPos AuGear::FindClothingWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Hats are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsClothing())
			return AuGearPos(*gi);
	}
	return AuGearPos();
}

AuGearPos AuGear::FindHatWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Hats are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::Head)
			continue;

		if((*gi).Item().IsHat())
			return AuGearPos(*gi);
	}
	return AuGearPos();
}

AuGearPos AuGear::FindHelmetWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Helmets are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::Head)
			continue;

		if((*gi).Item().IsHelmet())
			return AuGearPos(*gi);
	}
	return AuGearPos();
}

const AuItem* AuGear::ArmorItemWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Clothing is only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsBodyArmor())
			return &(*gi).Item();
	}
	return 0;
}

COMMODITYID AuGear::ArmorWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Clothing is only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsBodyArmor())
			return (*gi).CommodityId();
	}
	return CID_Invalid;
}

const AuItem* AuGear::ClothingItemWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Clothing is only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsClothing())
			return &(*gi).Item();
	}
	return 0;
}

COMMODITYID AuGear::ClothingWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Clothing is only worn in the BodyWear slot.
		if((*gi).SlotType() != AuGearSlot::BodyWear)
			continue;

		if((*gi).Item().IsClothing())
			return (*gi).CommodityId();
	}
	return CID_Invalid;
}

const AuItem* AuGear::HatItemWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Helmets are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::Head)
			continue;

		if((*gi).Item().IsHat())
			return &(*gi).Item();
	}
	return 0;
}

const AuItem* AuGear::HelmetItemWorn() const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Helmets are only worn in the Head slot.
		if((*gi).SlotType() != AuGearSlot::Head)
			continue;

		if((*gi).Item().IsHelmet())
			return &(*gi).Item();
	}
	return 0;
}

AuGearPos AuGear::FindReadyItem(COMMODITYID cid) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		if((*ii).CommodityId() == cid)
			return AuGearPos(*ii);	// Found an item of the type we are looking for.

	// Look inside waist containers, e.g. utility pouch.
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		if((*ii).SlotType() != AuGearSlot::Waist)
			continue;

		const AuItemContent* contents = (*ii).Item().Content();
		if(contents)
		{
			// Look inside belt container.
			for(AuItemContent::const_iterator ci = contents->cbegin(); ci != contents->cend(); ci++)
				if((*ci).second->CommodityId() == cid)
				{
					// Found an item of the type we are looking for inside the container.
					return AuGearPos(*ii, (*ci).first);
				}
		}
	}

	// Failure if we get here.
	return AuGearPos();
}

float AuGear::Weight() const
{
	float weight = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		weight += (*it).Item().Weight();
	return weight;
}

bool AuGear::ReloadItem(const AuGearPos& pos, INVENTORYSUB itemSub)
{
	// Check the item to see if it is valid and that it is a weapon that uses ammo.
	// We don't currently reload anything but weapons.
	AuItemWeaponAmmo* item = dynamic_cast<AuItemWeaponAmmo*>(ItemAt(pos));
	if(!item)
		return false;	// Item is not a weapon that uses ammo.

	if((itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1)) &&
			item->Ammo1Loaded() >= item->Ammo1Qty())
		return false;	// Weapon 1 is already fully loaded. Ignore the reload command.
	if(itemSub == IPS_Weapon2 && item->Ammo2Loaded() >= item->Ammo2Qty())
		return false;	// Weapon 2 is already fully loaded. Ignore the reload command.

	// Look for the first ammo found in this person's ready inventory, without digging into containers.
	for(iterator gi = begin(); gi != end(); gi++)
	{
		AuItemAmmo* ammo = dynamic_cast<AuItemAmmo*>(&(*gi).Item());
		if(!ammo)
			continue;	// Inventory item is not ammo.

		if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
			continue;

		AuGearPos ammopos(*gi);
		if(ammopos == pos)
			continue;	// Inventory item cannot reload itself.

		if(itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1))
		{
			// Current item usage is set to use weapon 1.
			if(ammo->AmmoType() == item->Ammo1Type())
			{
				// The inventory item is the appropriate kind of ammo for weapon 1.
				// MoveItem handles reloading of weapons.
				if(MoveItem(ammopos, pos))
					return true;	// Stop trying to load ammo from the ready inventory.
			}
		}
		else if(itemSub == IPS_Weapon2)
		{
			// Current item usage is set to use weapon 2.
			if(ammo->AmmoType() == item->Ammo2Type())
			{
				// The inventory item is the appropriate kind of ammo for weapon 2.
				// MoveItem handles reloading of weapons.
				if(MoveItem(ammopos, pos))
					return true;	// Stop trying to load ammo from the ready inventory.
			}
		}
	}

	// If no ammo was found in my ready inventory, snuffle through backpacks and other containers.
	for(iterator gi = begin(); gi != end(); gi++)
	{
		AuItemContent* contents = (*gi).Item().Content();
		if(!contents)
			continue;	// Inventory item is not a container.

		// Look for appropriate ammo in the container.
		for(AuItemContent::const_iterator ci = contents->begin(); ci != contents->end(); ci++)
		{
			AuItemAmmo* ammo = dynamic_cast<AuItemAmmo*>((*ci).second);
			if(!ammo)
				continue;	// Inventory item is not ammo.

			if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
				continue;

			if(itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1))
			{
				// Current item usage is set to use weapon 1.
				if(ammo->AmmoType() == item->Ammo1Type())
				{
					// The inventory item is the appropriate kind of ammo for weapon 1.
					// MoveItem handles reloading of weapons.
					if(MoveItem(AuGearPos(*gi, (*ci).first), pos))
						return true;	// Stop trying to load ammo from the container.
				}
			}
			else if(itemSub == IPS_Weapon2)
			{
				// Current item usage is set to use weapon 2.
				// Note that the ammo always offers m_ammo1Type. Ammo only represents one type. A weapon may
				// accept two types, as in the case of a rifle with a grenade launcher.
				if(ammo->AmmoType() == item->Ammo2Type())
				{
					// The inventory item is the appropriate kind of ammo for weapon 2.
					// MoveItem handles reloading of weapons.
					if(MoveItem(AuGearPos(*gi, (*ci).first), pos))
						return true;	// Stop trying to load ammo from the container.
				}
			}
		}
	}
	return false;
}

bool AuGear::ReloadItem(const AuGearPos& pos, INVENTORYSUB itemSub, QString& ammoDesc)
{
	// Check the item to see if it is valid and that it is a weapon that uses ammo.
	// We don't currently reload anything but weapons.
	AuItemWeaponAmmo* item = dynamic_cast<AuItemWeaponAmmo*>(ItemAt(pos));
	if(!item)
		return false;	// Item is not a weapon that uses ammo.

	if((itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1)) &&
			item->Ammo1Loaded() >= item->Ammo1Qty())
		return false;	// Weapon 1 is already fully loaded. Ignore the reload command.
	if(itemSub == IPS_Weapon2 && item->Ammo2Loaded() >= item->Ammo2Qty())
		return false;	// Weapon 2 is already fully loaded. Ignore the reload command.

	// Look for the first ammo found in this person's ready inventory, without digging into containers.
	for(iterator gi = begin(); gi != end(); gi++)
	{
		AuItemAmmo* ammo = dynamic_cast<AuItemAmmo*>(&(*gi).Item());
		if(!ammo)
			continue;	// Inventory item is not ammo.

		if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
			continue;

		AuGearPos ammopos(*gi);
		if(ammopos == pos)
			continue;	// Inventory item cannot reload itself.

		if(itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1))
		{
			// Current item usage is set to use weapon 1.
			if(ammo->AmmoType() == item->Ammo1Type())
			{
				// The inventory item is the appropriate kind of ammo for weapon 1.
				ammoDesc = ammo->ListDesc();

				// MoveItem handles reloading of weapons.
				if(MoveItem(ammopos, pos))
					return true;	// Stop trying to load ammo from the ready inventory.
			}
		}
		else if(itemSub == IPS_Weapon2)
		{
			// Current item usage is set to use weapon 2.
			if(ammo->AmmoType() == item->Ammo2Type())
			{
				// The inventory item is the appropriate kind of ammo for weapon 2.
				ammoDesc = ammo->ListDesc();

				// MoveItem handles reloading of weapons.
				if(MoveItem(ammopos, pos))
					return true;	// Stop trying to load ammo from the ready inventory.
			}
		}
	}

	// If no ammo was found in my ready inventory, snuffle through backpacks and other containers.
	for(iterator gi = begin(); gi != end(); gi++)
	{
		AuItemContent* contents = (*gi).Item().Content();
		if(!contents)
			continue;	// Inventory item is not a container.

		// Look for appropriate ammo in the container.
		for(AuItemContent::const_iterator ci = contents->begin(); ci != contents->end(); ci++)
		{
			AuItemAmmo* ammo = dynamic_cast<AuItemAmmo*>((*ci).second);
			if(!ammo)
				continue;	// Inventory item is not ammo.

			if(DbCommodity::IsEnergyWeapon(ammo->CommodityId()))
				continue;

			if(itemSub == IPS_Weapon1 || (itemSub == IPS_Weapon2 && item->Ammo2Type() == AMMO_Energy1))
			{
				// Current item usage is set to use weapon 1.
				if(ammo->AmmoType() == item->Ammo1Type())
				{
					// The inventory item is the appropriate kind of ammo for weapon 1.
					ammoDesc = ammo->ListDesc();

					// MoveItem handles reloading of weapons.
					if(MoveItem(AuGearPos(*gi, (*ci).first), pos))
						return true;	// Stop trying to load ammo from the container.
				}
			}
			else if(itemSub == IPS_Weapon2)
			{
				// Current item usage is set to use weapon 2.
				// Note that the ammo always offers m_ammo1Type. Ammo only represents one type. A weapon may
				// accept two types, as in the case of a rifle with a grenade launcher.
				if(ammo->AmmoType() == item->Ammo2Type())
				{
					// The inventory item is the appropriate kind of ammo for weapon 2.
					ammoDesc = ammo->ListDesc();

					// MoveItem handles reloading of weapons.
					if(MoveItem(AuGearPos(*gi, (*ci).first), pos))
						return true;	// Stop trying to load ammo from the container.
				}
			}
		}
	}
	return false;
}

// Returns an available which id for a slot type.
quint8 AuGear::NextWhich(AuGearSlot::SLOTTYPE slotType) const
{
	quint8 nextWhich;
	ItemsInSlot(slotType, &nextWhich);
	return nextWhich;
}

bool AuGear::MoveItem(const AuGearPos& srcPos, AuGearSlot::SLOTTYPE destSlotType)
{
	const AuItem* srcItem = ItemAt(srcPos);
	if(!srcItem || !AuGearSlot::CanHoldItem(destSlotType, *srcItem))
		return false;

	// We have a valid source item and it is allowed to move to the destination slot type.
	if(srcPos.IsTopLevel() && srcPos.Slot() == destSlotType)
		return false;	// The top level item is already in the destination slot type.

	// Are there any empty destination slots of the desired type.
	quint8 destWhich;	// Will be set to an available id in the slot type.
	int itemsInSlot = ItemsInSlot(destSlotType, &destWhich);
	bool hasEmptySlot(false);
	switch(destSlotType)
	{
	case AuGearSlot::Invalid:
	case AuGearSlot::Type_Count:
		break;
	case AuGearSlot::Hand:
		hasEmptySlot = itemsInSlot < 2;
		break;
	case AuGearSlot::Head:
	case AuGearSlot::Face:
	case AuGearSlot::Neck:
	case AuGearSlot::Body:
		hasEmptySlot = !itemsInSlot;
		break;
	case AuGearSlot::BodyWear:
		if(!itemsInSlot)
			hasEmptySlot = true;
		else if(itemsInSlot < 2)
		{
			// Body can hold one armor and one clothing.
			const AuItem* item = ItemInSlot(destSlotType);
			if((srcItem->IsBodyArmor() && !item->IsBodyArmor()) || (srcItem->IsClothing() && !item->IsClothing()))
				hasEmptySlot = true;
		}
		else
			hasEmptySlot = false;
		break;
	case AuGearSlot::Limb:
		hasEmptySlot = itemsInSlot < 2;
		break;
	case AuGearSlot::Digit:
	case AuGearSlot::Waist:
		hasEmptySlot = itemsInSlot < 4;
		break;
	}

	if(!hasEmptySlot)
		return false;

	// The item can be moved to a nu top level slot.
	append(AuGearSlot(destSlotType, destWhich, *srcItem));
	Erase(srcPos);
	return true;
}

int AuGear::MoveItem(const AuGearPos& srcPos, const AuGearPos& destPos)
{
	const AuItem* srcItem = ItemAt(srcPos);
	AuItem* destItem = ItemAt(destPos);
	if(!srcItem || !destItem)
		return 0;

	if(destItem->LoadAmmoIntoWeapon(*srcItem))
	{
		// Remove source item now that the ammo was loaded successfully.
		Erase(srcPos);
		return 2;
	}
	else if(destItem->PlantSeedInPot(*srcItem))
	{
		// Remove source item now that the ammo was loaded successfully.
		Erase(srcPos);
		return 3;
	}
	else if(destItem->FitItemIntoContainer(*srcItem))
	{
		// Remove source item now that it fit successfully.
		Erase(srcPos);
		return 1;
	}
	else if(srcItem->CommodityId() == CID_DNASampleKit && destItem->DNAIsViable())
	{
		// Take a DNA sample of the destination item.
		AuItem* replacementItem = AuItem::Create(CID_DNASample, srcItem->Quality(), srcItem->EmpireOfOrigin());
		if(replacementItem)
		{
			replacementItem->SetHitPoints(srcItem->HitPoints());
			AuItemDNA* repDNA = dynamic_cast<AuItemDNA*>(replacementItem);
			Q_ASSERT(repDNA);
			repDNA->SetDNA(destItem->DNA(), true, destItem->DNAIsSentient(), srcItem->DNAGrowthStage());        // Store DNA
			ReplaceItem(srcPos, replacementItem);
		}
	}
	else if(destItem->CommodityId() == CID_DNASampleKit && srcItem->DNAIsViable())
	{
		// Take a DNA sample of the source item. This enables a leather backpack to be sampled, without simply storing the sample
		// kit in the pack.
		AuItem* replacementItem = AuItem::Create(CID_DNASample, destItem->Quality(), destItem->EmpireOfOrigin());
		if(replacementItem)
		{
			replacementItem->SetHitPoints(destItem->HitPoints());
			AuItemDNA* repDNA = dynamic_cast<AuItemDNA*>(replacementItem);
			Q_ASSERT(repDNA);
			repDNA->SetDNA(srcItem->DNA(), true, srcItem->DNAIsSentient(), srcItem->DNAGrowthStage());        // Store DNA
			ReplaceItem(destPos, replacementItem);
		}
	}
	else
	{
		// Attempt to store data into destItem that is stored in the srcItem.
		int result = destItem->StoreData(srcItem);
		if(result)
			return 10+result;	// We got a response.
	}
	return 0;
}

AuGearPos AuGear::FindFood(ECOROLE ecoRole) const
{
	// Look for the first food found in this person's ready inventory, without digging into containers.
	bool anyContainers = false;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		if((*ii).Item().Content())
		{
			anyContainers = true;
			continue;										// Let's assume no containers have food nutrition value.
		}
		if(!(*ii).Item().IsFood(ecoRole) || !(*ii).Item().FoodNutritionValue() || (*ii).Item().CommodityId() == CID_Head)
			continue;									// Inventory item is not food or has no food nutrition value, or it's a head.

		return AuGearPos(*ii);
	}

	// If no food was found in my ready inventory, snuffle through backpacks and other containers.
	if(anyContainers)
	{
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			const AuItemContent* contents = (*ii).Item().Content();
			if(!contents)
				continue;									// Inventory item is not a container.

			// Look for appropriate food in the container.
			for(AuItemContent::const_iterator ci = contents->begin(); ci != contents->end(); ci++)
			{
				if(!(*ci).second->IsFood(ecoRole) || !(*ci).second->FoodNutritionValue() || (*ci).second->CommodityId() == CID_Head)
					continue;						// Inventory item is not food or has no food nutrition value, or it's a head.
				return AuGearPos(*ii, (*ci).first);
			}
		}
	}

	return AuGearPos();
}

AuGearPos AuGear::LightOnPos() const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		if((*ii).Item().IsLight() && (*ii).Item().State() == 1)
			return AuGearPos(*ii);
	return AuGearPos();
}

const AuItem* AuGear::ItemInSlot(AuGearSlot::SLOTTYPE slotType) const
{
	if(slotType == AuGearSlot::Invalid)
		return 0;

	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		if((*ii).SlotType() == slotType)
			return &(*ii).Item();
	return 0;
}

AuItem* AuGear::ItemInSlot(AuGearSlot::SLOTTYPE slotType)
{
	if(slotType == AuGearSlot::Invalid)
		return 0;

	for(iterator ii = begin(); ii != end(); ii++)
		if((*ii).SlotType() == slotType)
			return &(*ii).Item();
	return 0;
}

quint8 AuGear::ItemsInSlot(AuGearSlot::SLOTTYPE slotType, quint8* nextWhich) const
{
	if(slotType == AuGearSlot::Invalid || slotType >= AuGearSlot::Type_Count)
	{
		// Invalid slots do not hold items.
		if(nextWhich)
			*nextWhich = 0;
		return 0;
	}

	if(nextWhich)
	{
		// Get a list of the which ids in use by the slot type.
		QList<quint8> slotWhiches;
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			if((*ii).SlotType() == slotType)
				slotWhiches.append((*ii).Which());

		// Select a which id that is not already in use.
		*nextWhich = 0;
		while(slotWhiches.contains(*nextWhich))
			(*nextWhich)++;

		return slotWhiches.size();
	}
	else
	{
		// It's simpler when all they want is the total number of items in that kind of slot.
		quint8 c = 0;
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
			if((*ii).SlotType() == slotType)
				c++;
		return c;
	}
}

void AuGear::ReplaceItem(const AuGearPos& pos, AuItem* item)
{
	iterator gi = FindPos(pos);
	if(gi == end())
		return;

	if(pos.IsTopLevel())
		(*gi).ReplaceItem(item);
	else if((*gi).Item().Content())
		(*gi).Item().Content()->ReplaceItem(pos.ContentPath(), item);
}

// Returns the number of 'item's that are available. i.e. Non-empty containers are not available.
unsigned int AuGear::CountAvailableItems(const AuItem& item) const
{
	unsigned int countAvailable = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		const AuItemContent* contents = (*ii).Item().Content();
		if((*ii).IsItem(item) && (!contents || contents->empty()))
			countAvailable++;	// Found item at top level and it is not a container or it is an empty container.
		else if(contents)
		{
			// Look inside container.
			for(AuItemContent::const_iterator ci = contents->begin(); ci != contents->end(); ci++)
			{
				if((*ci).second->IsItem(item) && (!(*ci).second->Content() || (*ci).second->Content()->empty()))
					countAvailable++;	// Found item in a container. It is not a container or it is an empty container.
			}
		}
	}
	return countAvailable;
}

void AuGear::RemoveItems(const AuItem& item)
{
	unsigned int countToRemove = item.Count();
	if(!countToRemove)
		return;

	iterator ii = begin();
	while(ii != end())
	{
		AuItemContent* contents = (*ii).Item().Content();
		if((*ii).IsItem(item) && (!contents || contents->empty()))
		{
			// Found item at top level and it is not a container or it is an empty container.
			ii = erase(ii);
			countToRemove--;
			if(!countToRemove)
				return;
			continue;
		}
		else if(contents)
		{
			// Look inside container.
			AuItemContent::iterator ci = contents->begin();
			while(ci != contents->end())
			{
				if((*ci).second->IsItem(item) && (!(*ci).second->Content() || (*ci).second->Content()->empty()))
				{
					// Found item in a container. It is not a container or it is an empty container.
					ci = contents->Erase(ci);
					countToRemove--;
					if(!countToRemove)
						return;
					continue;
				}
				ci++;
			}
		}
		ii++;
	}
}

// Returns true if any items are destroyed.
bool AuGear::DestructStoryItems()
{
	//qDebug() << "AuGear::DestructStoryItems";
	bool anyDestroyed = false;
	iterator ii = begin();
	while(ii != end())
	{
		AuItem& invit = (*ii).Item();

		// Destroy contents first in case the container becomes empty and it can be destructed.
		AuItemContent* contents = invit.Content();
		if(contents && contents->DestructStoryItems())
			anyDestroyed = true;

		if(!invit.Content() || invit.Content()->empty())
		{
			// Top level item is not a container or it is an empty container.
			if(invit.StoryInstance())
			{
				// The item is associated with a story instance.
				//qDebug() << "AuGear::DestructStoryItems item" << invit->ListName() << "inStory" << invit->StoryInstance()
					//<< "active" << g_Au.IsStoryActive(invit->StoryInstance());
				if(!g_Au.IsStoryActive(invit.StoryInstance()))
				{
					ii = erase(ii);		// Delete the item because the story instance is no longer active.
					anyDestroyed = true;
					continue;
				}
			}
		}

		ii++;
	}
	//qDebug() << "AuGear::DestructStoryItems anyDestroyed" << anyDestroyed;
	return anyDestroyed;
}

void AuGear::CombineItemsForSale(AuItemHash& combined) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		const AuItemContent* contents = (*ii).Item().Content();
		if(contents && !contents->empty())
		{
			// Add the items in the container but not the container itself.
			for(AuItemContent::const_iterator ci = contents->cbegin(); ci != contents->cend(); ci++)
			{
				const AuItemContent* subcontents = (*ci).second->Content();
				if(!subcontents || subcontents->empty())
					combined.AddItem(*(*ci).second);			// Add the contained item or empty container. Combine if possible.
			}
		}
		else
			combined.AddItem((*ii).Item());						// Add the top-level item or empty container. Combine if possible.
	}
}

bool AuGear::LooksLike(const AuGear& gear) const
{
	if(size() != gear.size())
		return false;

	// Side-by-side comparison does not work because the order can differ though the contents are identical.
	// Make a duplicate of the comparison gear so we can insure 1:1 comparison of items in the presense of multiple identical items.
	AuGear cc(gear);
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		iterator ci = cc.FindLooksLikeItemInSlot((*gi).Item(), (*gi).SlotType());	// The slot contains an item.
		if(ci == cc.end())
			return false;			// Corresponding identical item not found.
		cc.erase(ci);				// Remove the matching item to insure correct results in case of multiple identical items.
	}

	return true;
}

void AuGear::ListAll(AuGearPosList& gearPos) const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Get top level gear pos.
		AuGearPos gpos(*gi);
		gearPos.append(gpos);

		// Get gear pos of all items in containers recursively.
		const AuItemContent* contents = (*gi).Item().Content();
		if(contents)
			contents->ListAll(gpos, gearPos);
	}
}

void AuGear::ListItems(COMMODITYID cid, AuGearPosList& gearPos) const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
	{
		// Get top level gear pos.
		AuGearPos gpos(*gi);
		if((*gi).CommodityId() == cid)
			gearPos.append(gpos);

		// Get gear pos of all items in containers recursively.
		const AuItemContent* contents = (*gi).Item().Content();
		if(contents)
			contents->ListItems(cid, gpos, gearPos);
	}
}

void AuGear::ListPatents(AuKnownPatents& patents) const
{
	for(const_iterator gi = cbegin(); gi != cend(); gi++)
		(*gi).Item().ListPatents(patents);
}

QDebug operator<<(QDebug dbg, const AuGear& o)
{
	dbg.nospace() << "AuGear " << o.size() << " slots\n";
	for(AuGear::const_iterator gi = o.cbegin(); gi != o.cend(); gi++)
	{
		dbg.nospace() << " " << AuGearPos(*gi) << " " << (*gi).Item().ListName() << "\n";
		if((*gi).Item().Content())
			qDebug() << *(*gi).Item().Content();
	}
	return dbg.space();
}

bool AuGear::DeleteStoredBlueprintsBd(const AuGearPos& pos, const QSet<quint32>& designs)
{
	AuItem* item = ItemAt(pos);
	if(!item)
		return false;
	return item->DeleteStoredBlueprintsBd(designs);
}

bool AuGear::DeleteStoredBlueprintsSc(const AuGearPos& pos, const QSet<quint32>& designs)
{
	AuItem* item = ItemAt(pos);
	if(!item)
		return false;
	return item->DeleteStoredBlueprintsSc(designs);
}

bool AuGear::DeleteStoredPapers(const AuGearPos& pos, const QSet<QString>& papers)
{
	AuItem* item = ItemAt(pos);
	if(!item)
		return false;
	return item->DeleteStoredPapers(papers);
}

bool AuGear::DeleteStoredPatents(const AuGearPos& pos, const QSet<COMMODITYID>& patentCIDs)
{
	AuItem* item = ItemAt(pos);
	if(!item)
		return false;
	return item->DeleteStoredPatents(patentCIDs);
}

bool AuGear::DeleteStoredMissions(const AuGearPos& pos, const QSet<QString>& missions)
{
	AuItem* item = ItemAt(pos);
	if(!item)
		return false;
	return item->DeleteStoredMissions(missions);
}

quint64 AuGear::MemoryUsage() const
{
	quint64 mem = sizeof(AuGear);
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		mem += (*ii).MemoryUsage();
	return mem;
}

AuGearPos AuGear::FindBestTop(COMMODITYID cid) const
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
	}
	return pos;
}

bool AuGear::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	bool anyChange = false;
	for(iterator ii = begin(); ii != end(); ii++)
		if((*ii).Item().VoidOwnershipTitle(propertyId, claimantId, claimantName))
			anyChange = true;
	return anyChange;
}

void AuGear::FixItems()
{
	for(iterator ii = begin(); ii != end(); ii++)
		(*ii).FixItem();
}

