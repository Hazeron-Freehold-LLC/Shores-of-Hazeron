//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitem.cpp,v 1.335 2022/05/09 17:02:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitem.h"
//
#include "auanimalia.h"
#include "aufireparticles.h"
#include "augear.h"
#include "auglmodel.h"
#include "auglobal.h"
#include "auknownpatents.h"
#include "auscdesign.h"
#include "ausound.h"
#include "deconstructionprocess.h"
#include "demanufacturingprocess.h"
#include "dgdata.h"
#include "dpbody.h"
#include "psentity.h"
#include <QDebug>

static int s_MT_AuBountyOnItem = qRegisterMetaType<AuBountyOnItem>();
static int s_MT_AuItemTEUContainer = AuItemTEUContainer::RegisterMetaType();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuBountyOnItem::operator>>(QDataStream& out) const
{
	quint8 version = 1;
	out << -m_Amount;					// Negative amount indicates version byte follows.
	out << version;
	out << m_Id;
	out << m_Name;
	out << m_PosterId;
	out << m_PosterName;
	out << m_Reason;
}

void AuBountyOnItem::operator<<(QDataStream& in)
{
	in >> m_Amount;
	if(m_Amount < 0)
	{
		m_Amount = -m_Amount;
		quint8 version;
		in >> version;
		in >> m_Id;
		in >> m_Name;
		in >> m_PosterId;
		in >> m_PosterName;
		in >> m_Reason;
	}
	else
	{
		in >> m_Id;
		in >> m_Name;
		QString reason;
		in >> reason;	// Old version used middle dot to separate fields :/
		QStringList fields = reason.split(AuGlobal::MiddleDotSymbol());
		m_Reason = fields[0];
		if(fields.size() > 1)
			m_PosterName = fields[1];
		if(fields.size() > 2)
			m_PosterId = AuId(fields[2]);
		if(m_PosterId.IsNull() && m_PosterName == InsurerName())
			m_PosterId = AuId::InsurerId();
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Helper class for consuming food.
class AuFood
{
public:
	COMMODITYID		cid;
	unsigned char	qual;
	int				qty;
	int				foodValue;
	quint32			empireOfOrigin;
};
typedef QList<AuFood> FoodArray;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const AuItemContentId& id)
{
	dbg.nospace() << id.Id();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItem::AuItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/)
{
	Q_ASSERT(quality);
	m_Bits = 0;
	m_CID = cid;
	m_Count = count;
	m_EmpireOfOrigin = empireOfOrigin;

	// Don't crash if the commodity array is invalid. e.g. CID_Underwear.
	if(g_Au.m_CommodityArray[cid].m_id == cid)
		m_HitPoints = NewHitPoints();
	else
		m_HitPoints = 1;

	m_Quality = quality;
	m_StoryInstance = 0;

	m_AudioSource = 0;
	m_CurrentSound = SOUND_None;

	m_Model = 0;
	m_PSFire = 0;
	m_TexFire = 0;
}

AuItem::AuItem()
{
	m_Bits = 0;
	m_CID = CID_Invalid;
	m_Count = 1;
	m_EmpireOfOrigin = 0;
	m_HitPoints = 0;
	m_Quality = 1;
	m_StoryInstance = 0;

	m_AudioSource = 0;
	m_CurrentSound = SOUND_None;

	m_Model = 0;
	m_PSFire = 0;
	m_TexFire = 0;
}

AuItem::~AuItem()
{
	if(m_PSFire)
		delete m_PSFire;
}

AuItem::AuItem(const AuItem& c)
{
	m_Bits = c.m_Bits;
	m_CID = c.m_CID;
	m_Count = c.m_Count;
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_HitPoints = c.m_HitPoints;
	m_Inscription = c.m_Inscription;
	m_Quality = c.m_Quality;
	m_StoryInstance = c.m_StoryInstance;

	m_AudioSource = c.m_AudioSource;
	m_CurrentSound = c.m_CurrentSound;

	m_Model = 0;
	m_PSFire = 0;
	m_TexFire = 0;
}

const AuItem& AuItem::operator=(const AuItem& c)
{
	m_Bits = c.m_Bits;
	m_CID = c.m_CID;
	m_Count = c.m_Count;
	m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	m_HitPoints = c.m_HitPoints;
	m_Inscription = c.m_Inscription;
	m_Quality = c.m_Quality;
	m_StoryInstance = c.m_StoryInstance;

	m_AudioSource = c.m_AudioSource;
	m_CurrentSound = c.m_CurrentSound;

	m_Model = 0;
	m_PSFire = 0;
	m_TexFire = 0;
	return *this;
}

AuItem* AuItem::Clone() const
{
	return new AuItem(*this);
}

QString AuItem::HtmlDesc() const
{
	QString empireNameOfOrigin = EmpireNameOfOrigin();
	QString madeIn;
	if(!empireNameOfOrigin.isEmpty())
		madeIn = QObject::tr("Made in <b>%1</b><br>").arg(empireNameOfOrigin);

	QString itemDesc;
	if(Count() == 1)
		itemDesc = QString("<b>%1</b><br>"
				"Quality <b>%3 Q%4</b><br>"
				"%5"
				"Condition <b>%6/%7hp</b><br>"
				"Value <b>%L9%10</b><br>"
				"Weight <b>%L8kg</b>")
			.arg(ListName())
			.arg(DbCommodity::QualityDesc(Quality()))
			.arg(Quality())
			.arg(madeIn)
			.arg(HitPoints())
			.arg(NewHitPoints())
			.arg(Weight() * KilogramsPerPound(), 0, 'f', 2)
			.arg(UnitCost(),0,'f',0)
			.arg(g_Au.CurrencySymbol());
	else
		itemDesc = QString("<b>%1</b><br>"
				"Quality <b>%3 Q%4</b><br>"
				"%5"
				"Condition <b>%6/%7hp</b><br>"
				"Units <b>%L13</b><br>"
				"Value <b>%L9%10</b> Each, <b>%L11%10</b> Total<br>"
				"Weight <b>%L8kg</b> Each, <b>%L12kg</b> Total")
			.arg(ListName())
			.arg(DbCommodity::QualityDesc(Quality()))
			.arg(Quality())
			.arg(madeIn)
			.arg(HitPoints())
			.arg(NewHitPoints())
			.arg(Weight() * KilogramsPerPound(), 0, 'f', 2)
			.arg(UnitCost(),0,'f',0)
			.arg(g_Au.CurrencySymbol())
			.arg(UnitCost() * Count(),0,'f',0)
			.arg(Weight() * Count() * KilogramsPerPound(), 0, 'f', 2)
			.arg(Count());

	quint8 cu = VolumeInPackUnits();
	if(cu)
	{
		if(Count() == 1)
			itemDesc += QString("<br>Size <b>%L11cu</b>").arg(cu);
		else
			itemDesc += QString("<br>Size <b>%L14cu</b> Each, <b>%L15cu</b> Total").arg(cu).arg(cu * Count());
	}

	// Describe food and liquor. They are mutually exclusive of each other since they use the food nutrition value to mean
	// different things.
	if(IsFood())
	{
		QString eaters;
		if(IsCarnivoreFood())
			eaters += QObject::tr("Carnivore, ");
		if(IsHerbivoreFood())
			eaters += QObject::tr("Herbivore, ");
		if(IsOmnivoreFood())
			eaters += QObject::tr("Omnivore, ");
		if(IsScavengerFood())
			eaters += QObject::tr("Scavenger, ");
		if(!eaters.isEmpty())
		{
			eaters = eaters.left(eaters.length()-2);	// drop trailing 'comma space'
			itemDesc += QObject::tr("<br>Food <b>%1</b><div style=\"margin-left: 10px;\">Edible To <b>%2</b></div>")
				.arg(FoodNutritionValue()).arg(eaters);
		}
		else
			itemDesc += QObject::tr("<br>Food <b>%1</b>").arg(FoodNutritionValue());
	}
	else if(IsIntoxicant())
		itemDesc += QObject::tr("<br>Intoxicant <b>%1</b>").arg(IntoxicantValue());

	// Add descriptions for specific commodities.
	switch(CommodityId())
	{
	case CID_Antidote:
		itemDesc += QObject::tr("<br><b>Cures Poison</b>");
		break;
	case CID_Antitoxin:
		itemDesc += QObject::tr("<br><b>Cures Disease</b>");
		break;
		/*
	case CID_Defibrilator:
		itemDesc += QObject::tr("<br><b>Revives Dying Lifeforms</b>");
		break;
		*/
	case CID_DNASampleKit:
		itemDesc += QObject::tr("<br><b>Collects a DNA Sample</b>");
		break;
	case CID_DNASample:
		itemDesc += QObject::tr("<br><b>Contains a DNA Sample</b>");
		break;
	case CID_FittingWrench:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>System Module</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Space Rocket</b>");
		break;
	case CID_GlueGun:
		itemDesc += QObject::tr("<br>Repairs <b>Sail Boat, Motor Bike, Submarine, SUV, APC, Farm Machinery and Helicopter</b>");
		break;
	case CID_PreonRingO:
	case CID_PreonRingB:
	case CID_PreonRingA:
	case CID_PreonRingF:
	case CID_PreonRingG:
	case CID_PreonRingK:
	case CID_PreonRingM:
		{
			itemDesc += QObject::tr("<br>Boosts Psionic Attack <b>×%1</b>.")
				.arg(QualityBoostF() + 1, 0,'f',2);

			float absorbK, blockK;
			CombatArmorEffectiveness(WM_Psionic, WE_Generic, absorbK, blockK);
			if(absorbK || blockK)
			{
				if(ArmorSpecialty() != WE_None)
					itemDesc += QObject::tr("<br>Psionic Armor<div style=\"margin-left: 10px;\">"
							"<b>Absorbs %1 Psionic Damage</b><br>"
							"<b>Hardened vs %3</b>"
							"</div>")
						.arg(g_Au.FormatArmorFactors(absorbK, blockK))
						.arg(EffectName(ArmorSpecialty()));
				else
					itemDesc += QObject::tr("<br>Psionic Armor<div style=\"margin-left: 10px;\">"
							"<b>Absorbs %1 Psionic Damage</b><br>"
							"</div>")
						.arg(g_Au.FormatArmorFactors(absorbK, blockK));
			}
		}
		break;
	case CID_Hammer:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>Adamantine, Metal, Magmium, Vulcium</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Sail Boat, Submarine, SUV, APC, Farm Machinery and Helicopter</b>");
		break;
	case CID_Knife:
	case CID_MagmiumKnife:
	case CID_VulciumKnife:
		itemDesc += QObject::tr("<br>Butchers <b>Animal Carcass, Fish Carcass and Head</b>");
		break;
	case CID_MedicalKit:
		itemDesc += QObject::tr("<br>Heals <b>%1hp</b>").arg(Health());
		break;
	case CID_Pliers:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>System Module</b>");
		break;
	case CID_ScrewDriver:
		itemDesc += QObject::tr("<br>Disassembles <b>System Module</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Space Fighter F1, Space Fighter F2, Space Transport and Space Rocket</b>");
		break;
	case CID_SewingNeedle:
		itemDesc += QObject::tr("<br>Repairs <b>Sail Boat</b>");
		break;
	case CID_Shovel:
		itemDesc += QObject::tr("<br><b>Digs</b>");
		break;
	case CID_SolderIron:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>Electronic Part</b> on <b>Space Vehicle Recovery System</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Submarine, Space Fighter F1, Space Fighter F2 and Space Transport</b>");
		break;
	case CID_TorqueWrench:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>Mechanical Part</b> on <b>Space Vehicle Launcher</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Motor Bike, SUV, APC, Farm Machinery, Helicopter, Space Fighter F1, Space Fighter F2, Space Transport and Space Rocket</b>");
		break;
	case CID_Water:
		itemDesc += QObject::tr("<br><b>Decontaminates Acid</b>");
		break;
	case CID_Wrench:
		itemDesc += QObject::tr("<br>Removes and Replaces <b>System Module</b>");
		itemDesc += QObject::tr("<br>Repairs <b>Motor Bike, Submarine, SUV, APC, Farm Machinery, Helicopter, Space Fighter F1, Space Fighter F2, Space Transport and Space Rocket</b>");
		break;
	default:
		break;
	}

	// Describe armor.
	float absorbK, blockK;
	CombatArmorEffectiveness(WM_Contact, WE_Generic, absorbK, blockK);
	if(absorbK || blockK)
	{
		if(ArmorSpecialty() != WE_None)
			itemDesc += QObject::tr("<br>Armor<div style=\"margin-left: 10px;\">"
					"<b>Absorbs %1 Damage</b><br>"
					"<b>Hardened vs %3</b>"
					"</div>")
				.arg(g_Au.FormatArmorFactors(absorbK, blockK))
				.arg(EffectName(ArmorSpecialty()));
		else
			itemDesc += QObject::tr("<br>Armor<div style=\"margin-left: 10px;\">"
					"<b>Absorbs %1 Damage</b><br>"
					"</div>")
				.arg(g_Au.FormatArmorFactors(absorbK, blockK));
	}

	// Inscription.
	if(!m_Inscription.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";
		if(m_Inscription.indexOf('<') >= 0)
			itemDesc += QObject::tr("Inscription<p>%1<p>").arg(m_Inscription);
		else
			itemDesc += QObject::tr("Inscription<div style=\"margin-left: 10px;\"><b>%1</b></div>").arg(m_Inscription);
	}

	// Describe weapon characteristics last.
	// Item mode might provide a crude weapon, like a bayonette.
	quint32 combits = ComBits();
	if((combits & CB_Slash) == CB_Slash)
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";
		itemDesc += QObject::tr("Contact Weapon 1<div style=\"margin-left: 10px;\"><b>Slash 1-%1hp Damage</b></div>")
			.arg(Health());
	}
	else if(combits & CB_Pierce)
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";
		itemDesc += QObject::tr("Contact Weapon 1<div style=\"margin-left: 10px;\"><b>Pierce 1-%1hp Damage</b></div>")
			.arg(Health());
	}
	else if(combits & CB_Bludgeon)
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";
		itemDesc += QObject::tr("Contact Weapon 1<div style=\"margin-left: 10px;\"><b>Bludgeon 1-%1hp Damage</b></div>")
			.arg(Health());
	}

	return itemDesc;
}

QString AuItem::StoryDebug() const
{
	QString storyDesc;
	if(m_StoryInstance)
		storyDesc += QString("%1 ").arg(m_StoryInstance);

	if(IsDroppableOnDeath())
		storyDesc += "can drop ";
	else
		storyDesc += "no drop ";

	if(IsSlipperyOnDeath())
		storyDesc += "slippery ";
	else
		storyDesc += "not slippery ";

	return storyDesc.trimmed();
}

// Packs this item's item type and data into a byte array using operator>>. Use Unpack to restore.
QByteArray AuItem::Pack() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)Type();
	out << *this;
	return ba;
}

// Restores an item from a byte array that was stored using Pack.
// The returned pointer is allocated on the heap and becomes the property of the caller.
//static
AuItem* AuItem::Unpack(const QByteArray& ba)
{
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 itemType;
	in >> itemType;
	if(!itemType)
	{
		qWarning() << "error: AuItem::Unpack read itemType 0";
		return 0;
	}
	AuItem* item = Create(itemType);
	if(!item)
		return 0;
	in >> *item;

	// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
	AuItem* replacement = item->FixItem();
	if(replacement)
	{
		delete item;
		item = replacement;
	}

	return item;
}

void AuItem::operator>>(QDataStream& out) const
{
	// Decide what to send.
	quint8 hasWhat = 0;
	if(m_Count != 1)
		hasWhat |= 0x01;
	if(m_EmpireOfOrigin)
		hasWhat |= 0x02;
	if(m_HitPoints != NewHitPoints())
		hasWhat |= 0x04;
	if(!m_Inscription.isEmpty())
		hasWhat |= 0x08;
	if(m_StoryInstance)
		hasWhat |= 0x10;
	if(m_Bits)
		hasWhat |= 0x20;

	// Send the data.
	out << hasWhat;

	out << (quint16)m_CID;

	if(hasWhat & 0x01)
		out << m_Count;

	if(hasWhat & 0x02)
		out << m_EmpireOfOrigin;

	if(hasWhat & 0x04)
		out << m_HitPoints;

	if(hasWhat & 0x08)
		out << m_Inscription;

	out << m_Quality;

	if(hasWhat & 0x10)
		out << m_StoryInstance;

	if(hasWhat & 0x20)
		out << m_Bits;
}

void AuItem::operator<<(QDataStream& in)
{
	// Determine what was sent.
	quint8 hasWhat = 0;
	in >> hasWhat;

	// Receive the data.
	in >> m_CID;

	if(hasWhat & 0x01)
		in >> m_Count;
	else
		m_Count = 1;

	if(hasWhat & 0x02)
		in >> m_EmpireOfOrigin;
	else
		m_EmpireOfOrigin = 0;

	if(hasWhat & 0x04)
		in >> m_HitPoints;
	else if(!DbCommodity::IsObsolete(CommodityId()))
		m_HitPoints = NewHitPoints();	// This will assert if we try to reference an obsolete item.

	if(hasWhat & 0x08)
		in >> m_Inscription;
	else
		m_Inscription = "";

	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;

	if(hasWhat & 0x10)
		in >> m_StoryInstance;
	else
		m_StoryInstance = 0;

	if(hasWhat & 0x20)
		in >> m_Bits;
	else
		m_Bits = 0;
}

void AuItem::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	if(re && al && m_CID != item->m_CID)
		DeleteRenderObjects(*re, al);

	m_Bits = item->m_Bits;
	m_CID = item->m_CID;
	m_Count = item->m_Count;
	m_EmpireOfOrigin = item->m_EmpireOfOrigin;
	m_HitPoints = item->m_HitPoints;
	m_Inscription = item->m_Inscription;
	m_Quality = item->m_Quality;
	m_StoryInstance = item->m_StoryInstance;
}

//static
AuItem* AuItem::Create(quint8 itemType)
{
	if(itemType == AuItem::ItemType())
		return new AuItem;
	else if(itemType == AuItemAmmo::ItemType())
		return new AuItemAmmo;
	else if(itemType == AuItemBdDesign::ItemType())
		return new AuItemBdDesign;
	else if(itemType == AuItemContainer::ItemType())
		return new AuItemContainer;
	else if(itemType == AuItemDNA::ItemType())
		return new AuItemDNA;
	else if(itemType == AuItemOfficerLog::ItemType())
		return new AuItemOfficerLog;
	else if(itemType == AuItemGalaxyLoc::ItemType())
		return new AuItemGalaxyLoc;
	else if(itemType == AuItemMission::ItemType())
		return new AuItemMission;
	else if(itemType == AuItemState::ItemType())
		return new AuItemState;
	else if(itemType == AuItemStateMessage::ItemType())
		return new AuItemStateMessage;
	else if(itemType == AuItemWeapon::ItemType())
		return new AuItemWeapon;
	else if(itemType == AuItemWeaponAmmo::ItemType())
		return new AuItemWeaponAmmo;
	else if(itemType == AuItemWeaponState::ItemType())
		return new AuItemWeaponState;
	else if(itemType == AuItemFloraDNA::ItemType())
		return new AuItemFloraDNA;
	else if(itemType == AuItemPicture::ItemType())
		return new AuItemPicture;
	else if(itemType == AuItemScDesign::ItemType())
		return new AuItemScDesign;
	else if(itemType == AuItemStorageMedia::ItemType())
		return new AuItemStorageMedia;
	else if(itemType == AuItemTEUContainer::ItemType())
		return new AuItemTEUContainer;
	else if(itemType == AuItemOwnershipTitle::ItemType())
		return new AuItemOwnershipTitle;

	qWarning() << "error: AuItem::Create invalid itemType" << itemType;
	//Q_ASSERT(false);
	return 0;
}

//static
AuItem* AuItem::Create(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/)
{
	quint8 itemType = ItemType(cid);
	if(itemType == AuItem::ItemType())
		return new AuItem(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemAmmo::ItemType())
		return new AuItemAmmo(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemBdDesign::ItemType())
		return new AuItemBdDesign(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemContainer::ItemType())
		return new AuItemContainer(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemDNA::ItemType())
		return new AuItemDNA(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemOfficerLog::ItemType())
		return new AuItemOfficerLog(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemGalaxyLoc::ItemType())
		return new AuItemGalaxyLoc(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemMission::ItemType())
		return new AuItemMission(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemState::ItemType())
		return new AuItemState(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemStateMessage::ItemType())
		return new AuItemStateMessage(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemWeapon::ItemType())
		return new AuItemWeapon(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemWeaponAmmo::ItemType())
		return new AuItemWeaponAmmo(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemWeaponState::ItemType())
		return new AuItemWeaponState(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemFloraDNA::ItemType())
		return new AuItemFloraDNA(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemPicture::ItemType())
		return new AuItemPicture(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemScDesign::ItemType())
		return new AuItemScDesign(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemStorageMedia::ItemType())
		return new AuItemStorageMedia(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemTEUContainer::ItemType())
		return new AuItemTEUContainer(cid, quality, empireOfOrigin, count);
	else if(itemType == AuItemOwnershipTitle::ItemType())
		return new AuItemOwnershipTitle(cid, quality, empireOfOrigin, count);
	qWarning() << "error: AuItem::Create invalid itemType" << itemType << "commodity" << cid;
	return 0;
}

//static
quint8 AuItem::ItemType(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_FIRST:
	case CID_Acid:
	case CID_Collision:
	case CID_Fire:
	case CID_MedicalProcedure:
	case CID_GivingBirth:
	case CID_ImplantBirth:
	case CID_Poison:
	case CID_Disease:
	case CID_Eaten:
	case CID_ShipCapture:
	case CID_Wormhole:
	case CID_Starvation:
	case CID_InternalInjuries:
	case CID_BaseWeapon:
	case CID_BaseTurret:
	case CID_Spaghettification:
	case CID_SunBurn:
	case CID_ShipTurret:
	case CID_BayWeapon:
	case CID_Mount_Anus:
	case CID_Mount_Eyes:
	case CID_Mount_Feet:
	case CID_Mount_Hands:
	case CID_Mount_Head:
	case CID_Mount_Tail:
	case CID_Mount_Torso:
	case CID_My_Anus:
	case CID_My_Eyes:
	case CID_My_Feet:
	case CID_My_Hands:
	case CID_My_Head:
	case CID_My_Tail:
	case CID_My_Torso:
	case CID_Count:
	case CID_Atmosphere:
	case CID_Hydrographics:
	case CID_Vegetation:
	case CID_Sunlight:
	case CID_Magma:
	case CID_SupercooledLiquid:
	case CID_InfernoAtmosphere:
	case CID_FrigidAtmosphere:
		// These special commodity id's are not represent as AuItems.
		break;

	case CID_Helicopter:
	case CID_SpaceRocket:
	case CID_SpaceFighter:
	case CID_SpaceTransport:
	case CID_FarmMachinery:
	case CID_MineMachinery:
	case CID_CommercialShip:
	case CID_NavalShip:
	case CID_Submarine:
	case CID_SUV:
	case CID_Tank:
	case CID_APC:
	case CID_SailBoat:
	case CID_DirtBike:
	case CID_AdvSpaceFighter:
		// Vehicles do not exist as AuItems.
		break;

	case CID_Invalid:
		break;

	case CID_Adamantine:
	case CID_AdamantineHullModule:
	case CID_Adamantite:
	case CID_Air:
	case CID_Antidote:
	case CID_AntifluxParticles:
	case CID_Antitoxin:
	case CID_AtmosphereCondenserBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_BallisticWeaponTurretModule:
	case CID_Beans:
	case CID_Beer:
	case CID_BlankDisk:
	case CID_Bolite:
	case CID_BorexinoPrecipitate:
	case CID_Bread:
	case CID_Candy:
	case CID_CapriHalterOutfit:
	case CID_CapacitorModule:
	case CID_CloakingShieldModule:
	case CID_Coal:
	case CID_CoalPowerPlantModule:
	case CID_CommandAndControlModule:
	case CID_ComputerObsolete:
	case CID_CrudeOil:
	case CID_CryoHeatSink:
	case CID_Cryozine:
	case CID_CrystalJewelry:
	case CID_Crystals:
	case CID_CyberneticsMedicalModule:
	case CID_DNASampleKit:
	case CID_Electricity:
	case CID_ElectronicPart:
	case CID_Eludium:
	case CID_EnergyShieldModule:
	case CID_EnergyWeaponBayModule:
	case CID_EnergyWeaponTurretModule:
	case CID_Fertilizer:
	case CID_Fiber:
	case CID_Flag:
	case CID_Flomentum:
	case CID_Flour:
	case CID_Fruit:
	case CID_Gasoline:
	case CID_GasolinePowerPlantModule:
	case CID_GemRing:
	case CID_Gems:
	case CID_GeneticsMedicalModule:
	case CID_Gin:
	case CID_Gold:
	case CID_GoldOre:
	case CID_GoldRing:
	case CID_Grain:
	case CID_Grapes:
	case CID_GravCoupling:
	case CID_GravityDriveModule:
	case CID_GunPowder:
	case CID_Hay:
	case CID_HeavyWeapon:
	case CID_Herbs:
	case CID_Hops:
	case CID_HydroelectricPowerPlantModule:
	case CID_HydrogenPowerPlantModule:
	case CID_Ice:
	case CID_IceCream:
	case CID_IoTube:
	case CID_Ioplasma:
	case CID_KineticShieldModule:
	case CID_LargeRocketMotor:
	case CID_LifeSupportGravityModule:
	case CID_LifeSupportModule:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_Lumber:
	case CID_LumberHullModule:
	case CID_Lumenite:
	case CID_Magmex:
	case CID_Magmium:
	case CID_MagmiumHeatSink:
	case CID_MagmiumHullModule:
	case CID_MechanicalPart:
	case CID_MedicalKit:
	case CID_Metal:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalHullModule:
	case CID_GoldBracelet:
	case CID_Microscope:
	case CID_Minerals:
	case CID_Money:
	case CID_Myrasplicer:
	case CID_Myrathane:
	case CID_NanopathCalibrator:
	case CID_NaturalGasPowerPlantModule:
	case CID_NuclearFuelRod:
	case CID_NuclearPowerPlantModule:
	case CID_Nuts:
	case CID_OScope:
	case CID_Ore:
	case CID_Paper:
	case CID_ParcXUniform:
	case CID_ParticleCollectorBayModule:
	case CID_Pastry:
	case CID_PatentAPCApparatus:
	case CID_PatentBattleShipApparatus:
	case CID_PatentCommercialShipApparatus:
	case CID_PatentDistillingGin:
	case CID_PatentDistillingRum:
	case CID_PatentDistillingTequila:
	case CID_PatentDistillingVodka:
	case CID_PatentDistillingWhiskey:
	case CID_PatentDrillingPhlogiston:
	case CID_PatentDrillingPolytaride:
	case CID_PatentDrillingViathol:
	case CID_PatentExtractingFlomentum:
	case CID_PatentExtractingMagmex:
	case CID_PatentExtractingMyrathane:
	case CID_PatentFarmMachineryApparatus:
	case CID_PatentGeneratingElectricityusingCoal:
	case CID_PatentGeneratingElectricityusingGasoline:
	case CID_PatentGeneratingElectricityusingHydrogen:
	case CID_PatentGeneratingElectricityusingNaturalGas:
	case CID_PatentGeneratingElectricityusingNuclearFuelRod:
	case CID_PatentGeneratingElectricityusingRadioactives:
	case CID_PatentGeneratingElectricityusingSunlight:
	case CID_PatentGeneratingElectricityusingWaterinEnvironment:
	case CID_PatentGeneratingElectricityusingWind:
	case CID_PatentHelicopterApparatus:
	case CID_PatentMakingAK:
	case CID_PatentMakingAntidoteusingHerbs:
	case CID_PatentMakingAntidoteusingMinerals:
	case CID_PatentMakingAntidoteusingPetrochemicals:
	case CID_PatentMakingAntitoxinusingAnimalCarcass:
	case CID_PatentMakingAntitoxinusingEggs:
	case CID_PatentMakingAtmosphereCondenserBayModule:
	case CID_PatentMakingBallisticWeaponBayModule:
	case CID_PatentMakingBallisticWeaponTurretModule:
	case CID_PatentMakingBeacon:
	case CID_PatentMakingBlankDisk:
	case CID_PatentMakingBombBlBlackCat:
	case CID_PatentMakingBombBlPiPorcupine:
	case CID_PatentMakingBombBlRaLosAlamos:
	case CID_PatentMakingBombElStTesla:
	case CID_PatentMakingBombSlCrMeatSlicer:
	case CID_PatentMakingBombSuPoMuffler:
	case CID_PatentMakingCandleusingPetrochemicals:
	case CID_PatentMakingCandy:
	case CID_PatentMakingCapacitorModule:
	case CID_PatentMakingCloakingShieldModule:
	case CID_PatentMakingCommandAndControlModule:
	case CID_PatentMakingCompressorBeamLens:
	case CID_PatentMakingComputer:
	case CID_PatentMakingCryoHeatSink:
	case CID_PatentMakingCyberneticsMedicalModule:
	case CID_PatentMakingDNASampleKit:
	case CID_PatentMakingDisintegratorRayLens:
	case CID_PatentMakingDisruptorBeamLens:
	case CID_PatentMakingDynamite:
	case CID_PatentMakingElectricLantern:
	case CID_PatentMakingElectronBeamLens:
	case CID_PatentMakingElectronicPart:
	case CID_PatentMakingEnergyShieldModule:
	case CID_PatentMakingEnergyWeaponBayModule:
	case CID_PatentMakingEnergyWeaponTurretModule:
	case CID_PatentMakingEnvironmentSuit:
	case CID_PatentMakingEnvironmentSuitHelmet:
	case CID_PatentMakingErgRayLens:
	case CID_PatentMakingFreezeRayLens:
	case CID_PatentMakingGeneticsMedicalModule:
	case CID_PatentMakingGigacell:
	case CID_PatentMakingGlueGun:
	case CID_PatentMakingGravCoupling:
	case CID_PatentMakingGravPack:
	case CID_PatentMakingGravityDriveModule:
	case CID_PatentMakingGunPowder:
	case CID_PatentMakingHeatRayLens:
	case CID_PatentMakingHeavyArmsAmmoPi:
	case CID_PatentMakingHeavyArmsAmmoPiAc:
	case CID_PatentMakingHeavyArmsAmmoPiBl:
	case CID_PatentMakingHeavyArmsAmmoPiRa:
	case CID_PatentMakingHeavyWeapon:
	case CID_PatentMakingHydrogenPowerPlantModule:
	case CID_PatentMakingHydrogenfromNaturalGas:
	case CID_PatentMakingIceCream:
	case CID_PatentMakingIoTube:
	case CID_PatentMakingJewelryStorage:
	case CID_PatentMakingKineticShieldModule:
	case CID_PatentMakingKnifeusingMetalandPlastic:
	case CID_PatentMakingLargeMissileBlBuFireball:
	case CID_PatentMakingLargeMissileBlThumper:
	case CID_PatentMakingLargeMissileSlPoSideviper:
	case CID_PatentMakingLargeRocketBlBuMolotov:
	case CID_PatentMakingLargeRocketBlThud:
	case CID_PatentMakingLargeRocketBuDiScudpox:
	case CID_PatentMakingLargeRocketMotor:
	case CID_PatentMakingLaserPistol:
	case CID_PatentMakingLaserRifle:
	case CID_PatentMakingLifeSupportGravityModule:
	case CID_PatentMakingLifeSupportModule:
	case CID_PatentMakingMagmiumHeatSink:
	case CID_PatentMakingMagmiumKnife:
	case CID_PatentMakingMagmiumPowerArmorHelmet:
	case CID_PatentMakingMagmiumPowerArmorSuit:
	case CID_PatentMakingMetalPowerArmorHelmet:
	case CID_PatentMakingMetalPowerArmorSuit:
	case CID_PatentMakingMicroscope:
	case CID_PatentMakingMyrasplicer:
	case CID_PatentMakingNanopathCalibrator:
	case CID_PatentMakingNeutronBeamLens:
	case CID_PatentMakingNitrateExplosive:
	case CID_PatentMakingNuclearFuelRod:
	case CID_PatentMakingOscilloscope:
	case CID_PatentMakingParticleCollectorBayModule:
	case CID_PatentMakingPastry:
	case CID_PatentMakingPersonnelTransporterModule:
	case CID_PatentMakingPharmaceuticalsusingPetrochemicals:
	case CID_PatentMakingPistol:
	case CID_PatentMakingPistolAmmunition:
	case CID_PatentMakingPlastic:
	case CID_PatentMakingPlasticExplosive:
	case CID_PatentMakingPlutonium:
	case CID_PatentMakingPreonRing:
	case CID_PatentMakingPressureTent:
	case CID_PatentMakingProspectorBayModule:
	case CID_PatentMakingProtonBeamLens:
	case CID_PatentMakingRifle:
	case CID_PatentMakingRifleAmmunition:
	case CID_PatentMakingRocketDriveModule:
	case CID_PatentMakingSensorModule:
	case CID_PatentMakingShotgun:
	case CID_PatentMakingShotgunAmmunition:
	case CID_PatentMakingSiphonBayModule:
	case CID_PatentMakingSmallMissileBlBuBarnburner:
	case CID_PatentMakingSmallMissileBlZipper:
	case CID_PatentMakingSmallMissileCrFrSnowball:
	case CID_PatentMakingSmallRocketBlBuFirecracker:
	case CID_PatentMakingSmallRocketBlPartypopper:
	case CID_PatentMakingSmallRocketMotor:
	case CID_PatentMakingSmallRocketSlPiMeathook:
	case CID_PatentMakingSolderingIron:
	case CID_PatentMakingStasisBeamLens:
	case CID_PatentMakingStorageCrystal:
	case CID_PatentMakingSurgeryMedicalModule:
	case CID_PatentMakingTesloidDissipator:
	case CID_PatentMakingThumbDrive:
	case CID_PatentMakingTractorBeamBayModule:
	case CID_PatentMakingVehicleTransporterModule:
	case CID_PatentMakingVulciumHeatSink:
	case CID_PatentMakingVulciumKnife:
	case CID_PatentMakingVulciumPowerArmorHelmet:
	case CID_PatentMakingVulciumPowerArmorSuit:
	case CID_PatentMakingWarp1FTLDriveModule:
	case CID_PatentMakingWarp2FTLDriveModule:
	case CID_PatentMakingWarp3FTLDriveModule:
	case CID_PatentMakingWarp4FTLDriveModule:
	case CID_PatentMakingWarp5FTLDriveModule:
	case CID_PatentMakingWarp6FTLDriveModule:
	case CID_PatentMakingWarp7FTLDriveModule:
	case CID_PatentMakingWarp8FTLDriveModule:
	case CID_PatentMakingWarp9FTLDriveModule:
	case CID_PatentMakingWaterusingAirandHydrogen:
	case CID_PatentMakingWeaponDeploymentBayModule:
	case CID_PatentMakingWeaponLauncherBayModule:
	case CID_PatentMakingWeaponLauncherTurretModule:
	case CID_PatentMakingWildcatterBayModule:
	case CID_PatentMakingWormholePFTLDriveModule:
	case CID_PatentMakingWormholePNFTLDriveModule:
	case CID_PatentMakingWormholePNNFTLDriveModule:
	case CID_PatentMineMachineryApparatus:
	case CID_PatentMiningAdamantite:
	case CID_PatentMiningBolite:
	case CID_PatentMiningEludium:
	case CID_PatentMiningLumenite:
	case CID_PatentMiningRadioactives:
	case CID_PatentMiningVulcanite:
	case CID_PatentMotorBikeApparatus:
	case CID_PatentPlasticSailboatApparatus:
	case CID_PatentRefiningAirfromAtmosphere:
	case CID_PatentRefiningAirfromWater:
	case CID_PatentRefiningAntifluxParticlesfromAtmosphere:
	case CID_PatentRefiningBorexinoPrecipitatefromAtmosphere:
	case CID_PatentRefiningCryozine:
	case CID_PatentRefiningFertilizerfromAir:
	case CID_PatentRefiningFertilizerfromAtmosphere:
	case CID_PatentRefiningGasoline:
	case CID_PatentRefiningHydrogenfromAtmosphere:
	case CID_PatentRefiningHydrogenfromWater:
	case CID_PatentRefiningIoplasma:
	case CID_PatentRefiningPetrochemicals:
	case CID_PatentSUVApparatus:
	case CID_PatentSmeltingAdamantine:
	case CID_PatentSmeltingMagmium:
	case CID_PatentSmeltingVulcium:
	case CID_PatentSpaceFighterF1Apparatus:
	case CID_PatentSpaceFighterF2Apparatus:
	case CID_PatentSpaceRocketApparatus:
	case CID_PatentSpaceTransportApparatus:
	case CID_PatentSubmarineApparatus:
	case CID_PatentTankApparatus:
	case CID_PatentWeavingTextilesusingPlastic:
	case CID_PersonnelTransporterModule:
	case CID_Pharmaceuticals:
	case CID_Plastic:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_Pliers:
	case CID_Plutonium:
	case CID_Polytaride:
	case CID_PreonRingA:
	case CID_PreonRingB:
	case CID_PreonRingF:
	case CID_PreonRingG:
	case CID_PreonRingK:
	case CID_PreonRingM:
	case CID_PreonRingO:
	case CID_PressureTent:
	case CID_ProspectorBayModule:
	case CID_Radioactives:
	case CID_RocketDriveModule:
	case CID_Rum:
	case CID_SensorModule:
	case CID_Silage:
	case CID_SiphonBayModule:
	case CID_SmallRocketMotor:
	case CID_SolarPowerPlantModule:
	case CID_Spices:
	case CID_Stone:
	case CID_StoneHullModule:
	case CID_Sugar:
	case CID_SurgeryMedicalModule:
	case CID_Tepee:
	case CID_Tequila:
	case CID_TesloidDissipator:
	case CID_Textiles:
	case CID_TractorBeamBayModule:
	case CID_Underwear:					// Doesn't actually exist.
	case CID_Vegetable:
	case CID_VehicleTransporterModule:
	case CID_Vodka:
	case CID_Vulcanite:
	case CID_Vulcium:
	case CID_VulciumHeatSink:
	case CID_VulciumHullModule:
	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
	case CID_Water:
	case CID_WeaponDeploymentBayModule:
	case CID_WeaponLauncherBayModule:
	case CID_WeaponLauncherTurretModule:
	case CID_Whiskey:
	case CID_WildcatterBayModule:
	case CID_WindPowerPlantModule:
	case CID_Wine:
	case CID_Wood:
	case CID_WoodHullModule:
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
		// Typical commodity.
		return AuItem::ItemType();

	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
	case CID_Clamp:
	case CID_Dynamite:
	case CID_FittingWrench:
	case CID_GlueGun:
	case CID_Hammer:
	case CID_Knife:
	case CID_MagmiumKnife:
	case CID_NitrateExplosive:
	case CID_PlasticExplosive:
	case CID_ScrewDriver:
	case CID_SewingNeedle:
	case CID_Shovel:
	case CID_SolderIron:
	case CID_Tongs:
	case CID_TorqueWrench:
	case CID_VulciumKnife:
	case CID_Wrench:
		// Weapons that do not use ammo.
		return AuItemWeapon::ItemType();

	case CID_Candle:
	case CID_Torch:
		// Weapon that has a state.
		return AuItemWeaponState::ItemType();

	case CID_AK47:
	case CID_Pistol:
	case CID_LaserPistol:
	case CID_Rifle:
	case CID_Shotgun:
	case CID_LaserRifle:
		// Weapons that use ammo.
		return AuItemWeaponAmmo::ItemType();

	case CID_PistolAmmo:
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiAc:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiRa:
	case CID_RifleAmmo:
	case CID_ShotgunAmmo:
	case CID_Gigacell:
	case CID_SmallMissileBl:
	case CID_LargeMissileBl:
	case CID_SmallRocketBl:
	case CID_LargeRocketBl:
	case CID_SmallMissileBlBu:
	case CID_LargeMissileBlBu:
	case CID_SmallRocketBlBu:
	case CID_LargeRocketBlBu:
	case CID_SmallRocketSlPi:
	case CID_LargeRocketBuDi:
	case CID_SmallMissileCrFr:
	case CID_LargeMissileSlPo:
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
	case CID_Hydrogen:
	case CID_NaturalGas:
	case CID_Petrochemicals:
	case CID_Phlogiston:
	case CID_Viathol:
		// Ammo and things that explode when shot.
		return AuItemAmmo::ItemType();

	case CID_BurningBarrel:
	case CID_Drum:
	case CID_Lantern:
	case CID_MagmiumPowerArmor:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalPowerArmor:
	case CID_MetalPowerArmorHelmet:
	case CID_StoneFirePit:
	case CID_VacSuit:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmor:
	case CID_VulciumPowerArmorHelmet:
		// Items that retain a state.
		return AuItemState::ItemType();

	case CID_Beacon:
		// Items that retain a state and have a text string.
		return AuItemStateMessage::ItemType();

	case CID_Backpack:
	case CID_RedBinder:
	case CID_GreenBinder:
	case CID_BlueBinder:
	case CID_Crate:
	case CID_GravPack:
	case CID_Knapsack:
	case CID_UtilityPouch:
	case CID_ParcX:
		// Items that are containers for other items.
		return AuItemContainer::ItemType();

	case CID_ContainerTEU:
		return AuItemTEUContainer::ItemType();

	case CID_MissionDisk:
		// Items that store a mission.
		return AuItemMission::ItemType();

	case CID_OfficerLog:
		// Items that store an event log.
		return AuItemOfficerLog::ItemType();

	case CID_Animal:
	case CID_AnimalMeat:
	case CID_Bone:
	case CID_Brain:
	case CID_Cheese:
	case CID_DNASample:
	case CID_Eggs:
	case CID_Fish:
	case CID_FishMeat:
	case CID_Gills:
	case CID_Head:
	case CID_Heart:
	case CID_Leather:
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
	case CID_Lungs:
	case CID_Milk:
	case CID_Stomach:
		// Items that store DNA.
		return AuItemDNA::ItemType();

	case CID_TypeOPreons:
	case CID_TypeBPreons:
	case CID_TypeAPreons:
	case CID_TypeFPreons:
	case CID_TypeGPreons:
	case CID_TypeKPreons:
	case CID_TypeMPreons:
		// Items that store galactic coordinates.
		return AuItemGalaxyLoc::ItemType();

	case CID_Seed:
	case CID_FlowerPotSmall:
	case CID_FlowerPotLarge:
		return AuItemFloraDNA::ItemType();

	case CID_PictureFrame120x68:
	case CID_PictureFrame22x28:
	case CID_PictureFrame60x34:
		return AuItemPicture::ItemType();

	case CID_BdDesignDisk:
	case CID_BdDesignDrawing:
		// Items that store a spacecraft design.
		return AuItemBdDesign::ItemType();

	case CID_ScDesignDisk:
	case CID_ScDesignDrawing:
		// Items that store a spacecraft design.
		return AuItemScDesign::ItemType();

	case CID_Computer:
	case CID_StorageCrystal:
	case CID_ThumbDrive:
	case CID_BraceletThumbDrive:
	case CID_BraceletStorageCrystal:
	case CID_NecklaceThumbDrive:
	case CID_NecklaceStorageCrystal:
	case CID_RingThumbDrive:
	case CID_RingStorageCrystal:
		return AuItemStorageMedia::ItemType();

	case CID_SpacecraftTitle:
		return AuItemOwnershipTitle::ItemType();
	}
	return 0;	// Invalid
}

void AuItem::WeaponInfo(INVENTORYSUB sub,
		COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
		WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
		WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const
{
	const DbCommodity& bCom = Commodity();
	if(sub == IPS_Weapon1 || sub == IPS_Weapon2 || !bCom.IsWeapon(sub))
	{
		cid = CID_Invalid;
		quality = 0;
		mode = WM_Not;
		range = 0;
		ammoCur = 0;
		ammoFull = 0;
		effect1 = WE_None;
		damage1 = 0;
		damageMod1 = 0;
		radius1 = 0;
		ap1 = 0;
		st1 = 0;
		effect2 = WE_None;
		damage2 = 0;
		damageMod2 = 0;
		radius2 = 0;
		ap2 = 0;
		st2 = 0;
		return;
	}

	cid = CommodityId();
	quality = Quality();
	mode = WM_Contact;
	range = 1;
	ammoCur = 0;
	ammoFull = 0;
	if((bCom.m_combits & CB_Slash) == CB_Slash)
		effect1 = WE_Slash;
	else if(bCom.m_combits & CB_Pierce)
		effect1 = WE_Pierce;
	else
		effect1 = WE_Bludgeon;
	damage1 = QualityBoosted(bCom.m_health);
	damageMod1 = 0;
	radius1 = 0;
	ap1 = 0;
	st1 = 0;
	effect2 = WE_None;
	damage2 = 0;
	damageMod2 = 0;
	radius2 = 0;
	ap2 = 0;
	st2 = 0;
}

// Returns the name of this item.
QString AuItem::ListDesc() const
{
	return ListDesc(EmpireNameOfOrigin());
}

// Returns the name of this item.
QString AuItem::ListDesc(const QString& empireOfOrigin) const
{
	QString name = ListName();
	if(CommodityId() == CID_MissionDisk ||
			CommodityId() == CID_OfficerLog ||
			CommodityId() == CID_BdDesignDisk ||
			CommodityId() == CID_ScDesignDisk)
		name += QObject::tr(", disk");	// Descriptive text is about the disk, not the design|mission.
	else if(CommodityId() == CID_BdDesignDrawing ||
			CommodityId() == CID_ScDesignDrawing)
		name += QObject::tr(", drawing");	// Descriptive text is about the disk, not the design|mission.

	QString madeIn;
	if(!empireOfOrigin.isEmpty())
		madeIn = QObject::tr(" Made in %1").arg(empireOfOrigin);	// Note leading space is important.

	QString condition;
	if(HitPoints() != NewHitPoints())
		condition = QObject::tr(" %1/%2hp").arg(HitPoints()).arg(NewHitPoints());

	// Format the final result.
	if(Count() > 1)
		return QObject::tr("%1 %2 Q%3%6%7, %L4 units")
			.arg(name)
			.arg(DbCommodity::QualityDesc(Quality()))
			.arg(Quality())
			.arg(Count())
			.arg(madeIn)
			.arg(condition);
	else
		return QObject::tr("%1 %2 Q%3%6%7")
			.arg(name)
			.arg(DbCommodity::QualityDesc(Quality()))
			.arg(Quality())
			.arg(madeIn)
			.arg(condition);
}

// Returns false if this is a container AND it is emptied.
bool AuItem::FetchConstructionMaterials(DeConstructionProcess& cp)
{
	AuItemContent* contents = Content();
	if(contents)
	{
		contents->FetchConstructionMaterials(cp);
		if(contents->empty())
			return false;
	}
	return true;
}

// Returns false if this is a container AND it is emptied.
// If elecSubQ is non-zero, electricity can be substituted for sunlight
// but the elecSubQ is used.
bool AuItem::FetchManufacturingMaterials(DeManufacturingProcess &mp, quint8 elecSubQ, quint8 waterSubQ)
{
	AuItemContent* contents = Content();
	if(!contents)
		return true;

	contents->FetchManufacturingMaterials(mp, elecSubQ, waterSubQ);
	if(contents->empty())
		return false;

	return true;
}

void AuItem::ConsumeContainedItem(const AuItemContentId& id, unsigned int iMaterialConsumed)
{
	AuItemContent* contents = Content();
	if(!contents)
		return;

	AuItemContent::iterator it = contents->find(id);
	if(it == contents->end())
		return;

	(*it).second->Consume(iMaterialConsumed);
	if((*it).second->Count() == 0)
		contents->Erase(it);
}

QString AuItem::EmpireNameOfOrigin() const
{
	QString empireNameOfOrigin;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(EmpireOfOrigin(), empireNameOfOrigin);
	g_Au.UnlockData();
	return empireNameOfOrigin;
}

// Returns true if there is any change to this item's condition.
bool AuItem::CombatApplyDamage(WEAPONEFFECT effect, int damage)
{
	Q_ASSERT(damage);
	if(m_HitPoints <= 1)
		return false;											// Do not destroy armor due to damage. Only damage it.

	// Calculate the damage points caused to this armor due to absorbing damage.
	int hitsDamage = (damage + 9) / 10;
	if(ArmorSpecialty() == effect)
		hitsDamage /= 2;										// 1/2 damage to armor vs its specialty damage.
	if(!hitsDamage)
		return false;

	// Degrade the condition of this armor item.
	// Degrade this armor as low as 1 point but not zero. It looks weird to shoot someone's clothes off piece by piece.
	if(m_HitPoints > hitsDamage)
	{
		// There is room to degrade this armor by the full amount without dropping below 1.
		m_HitPoints -= hitsDamage;
		return true;
	}
	else if(m_HitPoints > 1)
	{
		// Current condition must be reduced to 1.
		m_HitPoints = 1;
		return true;
	}
	return false;
}

void AuItem::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	Q_UNUSED(re);
	m_Model = 0;

	if(m_PSFire)
	{
		delete m_PSFire;
		m_PSFire = 0;
	}

	if(al && m_AudioSource)
	{
		al->StopSound(m_AudioSource);
		m_CurrentSound = SOUND_None;
	}

	m_TexFire = 0;
}

// Returns true if this item can make a continuous sound all the time, whether or not it is currently making any noise.
bool AuItem::HasSound() const
{
	switch(CommodityId())
	{
	default:
		return false;

	case CID_BurningBarrel:
	case CID_Drum:
	case CID_Lantern:
	case CID_StoneFirePit:
	case CID_Torch:
		return true;
	}
}

// Returns true if this item is currently making a continuous sound all the time.
bool AuItem::IsAudible() const
{
	switch(CommodityId())
	{
	default:
		return false;

	case CID_BurningBarrel:
	case CID_Drum:
	case CID_Lantern:
	case CID_StoneFirePit:
	case CID_Torch:
		return State() == 1;
	}
}

bool AuItem::IsGarageItem() const
{
	if(DbCommodity::IsVehicleAmmunition(CommodityId()))
		return true;
	if(IsFuel())
		return true;
	if(IsTool())
		return true;
	if(CommodityId() == CID_Beacon)
		return true;
	return false;
}

void AuItem::RenderSound(const AuMatrix4x4& model, AuSound& al, float gain, float pitch)
{
	if(State() == 1)
	{
		SOUND_EFFECT sound;
		bool loopSound;
		switch(CommodityId())
		{
		default:
			return;
		case CID_Torch:
			loopSound = true;
			sound = SOUND_Torch;
			break;
		case CID_Drum:
			loopSound = true;
			sound = SOUND_NativeDrums;
			break;
		case CID_Lantern:
			loopSound = true;
			sound = SOUND_GasLamp;
			break;
		case CID_BurningBarrel:
			gain *= .8f;					// Not as loud as the fire pit.
			loopSound = true;
			sound = SOUND_CracklingFire;
			break;
		case CID_StoneFirePit:
			loopSound = true;
			sound = SOUND_CracklingFire;
			break;
		}

		AuPoint sndPos = model.Translation();
		SOUND_EFFECT cur = (SOUND_EFFECT)m_CurrentSound;
		al.Play(m_AudioSource, cur, sound, sndPos.x, sndPos.y, sndPos.z, gain, pitch, loopSound);
		m_CurrentSound = cur;
	}
	else
	{
		SOUND_EFFECT cur = (SOUND_EFFECT)m_CurrentSound;
		al.Play(m_AudioSource, cur, SOUND_None, 0,0,0, 0,0, false);
		m_CurrentSound = cur;
	}
}

double AuItem::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc, const AuId& id,
		float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		const AuPoint& windVec, float scale, int pass, double effectScale)
{
	Q_UNUSED(id);
	Q_UNUSED(windVec);
	// Create device objects.
	if(!m_Model)
	{
		// Start loading the 3ds model of the commodity using a worker thread.
		QString modelFile = g_Au.CommodityModel(CommodityId());
		if(modelFile.isEmpty() || !QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		m_Model = mc.LoadModelThreaded(modelFile, true, 1.);	// This does not create the open gl display list.
	}
	else if(!m_Model->HasGLModel())
		m_Model->GLModel(re);							// GL display list isn't done yet. Attempt to create the GL display list.

	// Model loading is multi threaded so it may take a few tries before we succeed here.
	if(!m_Model || !m_Model->HasGLModel())
		return 0;

	// Render the 'lit' version of the model, if there is one. It tends to look more interesting.
	// The lantern model switches from bright to dim when light is on or off.
	// All other models prefer the lit version for glowy things.
	bool lightOn = (CommodityId() != CID_Lantern && CommodityId() != CID_Beacon) || State() == 1;

	// Render
	if(pass < 2)
	{
		// Multiply by current transformation (Translation, rotation, and scale.)
		AuPoint pv = model.Project(AuPoint(0,0));
		if(pv.z > m_Model->RoughRadius()*scale)
			return 0;	// this item is behind the camera plane

		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		// Not inside atmosphere range of a world.
		// Note that 3ds models always return false.
		bool hasTransparency = m_Model->Render(projection, tThis, re, ambientR, ambientG, ambientB, ambientUp, lightOn, 1, pass);

		if(pass == 1)
		{
			if(hasTransparency || HasRenderEffects())
				return pv.Length2();
		}
		else
		{
			// Render final pass blended things, like fire on torches.
			RenderEffects(projection, model, re, gravityUp, effectScale);
		}
	}
	else
	{
		// Render final pass blended things, like fire on torches.
		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		m_Model->Render(projection, tThis, re, ambientR, ambientG, ambientB, ambientUp, lightOn, 1, pass);

		RenderEffects(projection, model, re, gravityUp, effectScale);
	}
	return 0;
}

double AuItem::RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		const AuPoint& globeP,
		const AuPoint& windVec, float scale, int pass)
{
	Q_UNUSED(id);
	Q_UNUSED(windVec);
	// Create device objects.
	{
		if(!m_Model)
		{
			// Start loading the 3ds model of the commodity using a worker thread.
			QString modelFile = g_Au.CommodityModel(CommodityId());
			if(modelFile.isEmpty() || !QFile::exists(modelFile))
				modelFile = ":data/BoltCube.3dsX";				// The model file does not exist so substitute with the bold cube.
			m_Model = mc.LoadModelThreaded(modelFile, true, 1.);	// This does not create the open gl display list.
		}
		else if(!m_Model->HasGLModel())
			m_Model->GLModel(re);						// GL display list isn't done yet. Attempt to create the GL display list.

		// Model loading is multi threaded so it may take a few tries before we succeed here.
		if(!m_Model || !m_Model->HasGLModel())
			return 0;
	}

	// Render the 'lit' version of the model, if there is one. It tends to look more interesting.
	// The lantern model switches from bright to dim when light is on or off.
	// All other models prefer the lit version for glowy things.
	bool lightOn = (CommodityId() != CID_Lantern && CommodityId() != CID_Beacon) || State() == 1;

	// Render
	if(pass < 2)
	{
		// Multiply by current transformation (Translation, rotation, and scale.)
		AuPoint pv = model.Project(AuPoint(0,0));
		if(pv.z > m_Model->RoughRadius()*scale)
			return 0;	// this item is behind the camera plane

		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		// Inside atmosphere of a globe.
		// Note that 3ds models always return false.
		bool hasTransparency = m_Model->RenderFogGlobe(projection, tThis, re, globeP, ambientR, ambientG, ambientB, ambientUp,
				lightOn, fogDensity, fogR, fogG, fogB, 1, pass);
		if(pass == 1)
		{
			if(hasTransparency || HasRenderEffects())
				return pv.Length2();
		}
		else
		{
			// Render final pass blended things, like fire on torches.
			RenderEffects(projection, model, re, gravityUp, 1.);
		}
	}
	else
	{
		// Render final pass blended things, like fire on torches.
		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		m_Model->RenderFogGlobe(projection, tThis, re, globeP, ambientR, ambientG, ambientB, ambientUp,
				lightOn, fogDensity, fogR, fogG, fogB, 1, pass);
		RenderEffects(projection, model, re, gravityUp, 1.);
	}
	return 0;
}

double AuItem::RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		float shadowPanelSunAngles[MAX_SUNS],
		const AuPoint& windVec, float scale, int pass)
{
	Q_UNUSED(id);
	Q_UNUSED(windVec);
	// Create device objects.
	{
		if(!m_Model)
		{
			// Start loading the 3ds model of the commodity using a worker thread.
			QString modelFile = g_Au.CommodityModel(CommodityId());
			if(modelFile.isEmpty() || !QFile::exists(modelFile))
				modelFile = ":data/BoltCube.3dsX";				// The model file does not exist so substitute with the bold cube.
			m_Model = mc.LoadModelThreaded(modelFile, true, 1.);	// This does not create the open gl display list.
		}
		else if(!m_Model->HasGLModel())
			m_Model->GLModel(re);						// GL display list isn't done yet. Attempt to create the GL display list.

		// Model loading is multi threaded so it may take a few tries before we succeed here.
		if(!m_Model || !m_Model->HasGLModel())
			return 0;
	}

	// Render the 'lit' version of the model, if there is one. It tends to look more interesting.
	// The lantern model switches from bright to dim when light is on or off.
	// All other models prefer the lit version for glowy things.
	bool lightOn = (CommodityId() != CID_Lantern && CommodityId() != CID_Beacon) || State() == 1;

	// Render
	if(pass < 2)
	{
		// Multiply by current transformation (Translation, rotation, and scale.)
		AuPoint pv = model.Project(AuPoint(0,0));
		if(pv.z > m_Model->RoughRadius()*scale)
			return 0;	// this item is behind the camera plane

		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		// Inside atmosphere of a ringworld.
		// Note that 3ds models always return false.
		bool hasTransparency = m_Model->RenderFogRingworld(projection, tThis, re, shadowPanelSunAngles, ambientR, ambientG,
				ambientB, ambientUp, lightOn, fogDensity, fogR, fogG, fogB, 1, pass);
		if(pass == 1)
		{
			if(hasTransparency || HasRenderEffects())
				return pv.Length2();
		}
		else
		{
			// Render final pass blended things, like fire on torches and preons on rings.
			RenderEffects(projection, model, re, gravityUp, 1.);
		}
	}
	else
	{
		// Render final pass blended things, like fire on torches and preons on rings.
		AuMatrix4x4 tThis(model);
		if(scale != 1 && scale != 0)
			tThis.Scale(scale, scale, scale);

		m_Model->RenderFogRingworld(projection, tThis, re, shadowPanelSunAngles, ambientR, ambientG,
				ambientB, ambientUp, lightOn, fogDensity, fogR, fogG, fogB, 1, pass);
		RenderEffects(projection, model, re, gravityUp, 1.);
	}
	return 0;
}

void AuItem::RenderEffects(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& gravityUp,
		double effectScale)
{
	// Render final pass blended things, like fire on torches.
	switch(CommodityId())
	{
	default:
		// Most items do not render any special effects.
		return;

	case CID_BurningBarrel:
		if(State() == 1)
		{
			AuMatrix4x4 firePos(model);
			firePos.Translate(0,0,3);

			if(gravityUp != AuPoint(0,0,1))
			{
				AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(gravityUp);
				firePos *= trot;
			}

			RenderFire(projection, firePos, re, effectScale, 1.);
		}
		break;

	case CID_StoneFirePit:
		if(State() == 1)
		{
			AuMatrix4x4 firePos(model);
			firePos.Translate(0,0,1);

			if(gravityUp != AuPoint(0,0,1))
			{
				AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(gravityUp);
				firePos *= trot;
			}

			RenderFire(projection, firePos, re, effectScale, 2.);
		}
		break;

	case CID_Torch:
		if(State() == 1)
		{
			AuMatrix4x4 firePos(model);
			firePos.Translate(LightPosition());

			if(gravityUp != AuPoint(0,0,1))
			{
				AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(gravityUp);
				firePos *= trot;
			}

			RenderTorchFlame(projection, firePos, re, effectScale);
		}
		break;

	case CID_Candle:
		if(State() == 1)
		{
			AuMatrix4x4 firePos(model);
			firePos.Translate(LightPosition());

			if(gravityUp != AuPoint(0,0,1))
			{
				AuMatrix4x4 trot = AuPoint(0,0,1).GetAlignmentTransform(gravityUp);
				firePos *= trot;
			}

			RenderCandleFlame(projection, firePos, re, effectScale);
		}
		break;

	case CID_PreonRingO:
	case CID_PreonRingB:
	case CID_PreonRingA:
	case CID_PreonRingF:
	case CID_PreonRingG:
	case CID_PreonRingK:
	case CID_PreonRingM:
		{
			AuMatrix4x4 preonPos(model);
			preonPos.Translate(0, .125, 0);

			// Copied from DaStar::SunColor
			QColor starColors[] =
			{
				QColor(56, 98, 255),		// Light Blue
				QColor(87, 125, 255),		// Lighter Blue
				QColor(148, 174, 255),		// Even Lighter Blue
				QColor(242, 240, 255),		// Lightest Blue, Almost White
				QColor(255, 234, 214),		// Yellow
				QColor(255, 165, 66),		// Pale Orange
				QColor(255, 166, 3),		// Red Orange
				QColor(0, 0, 12)			// Black Hole
			};

			RenderPreon(projection, preonPos, re, effectScale, starColors[CommodityId() - CID_PreonRingO]);
		}
		break;
	}
}

void AuItem::RenderPreon(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale,
		const QColor& color)
{
	if(!m_TexFire)
		m_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_PSFire)
	{
		m_PSFire = new PsEntity();
		m_PSFire->SetMaxParticles(50);
		m_PSFire->SetNumToRelease(2);
		m_PSFire->SetReleaseInterval(0.025f);
		m_PSFire->SetLifeCycle(5.f);
		m_PSFire->SetSize(.2f);
		m_PSFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSFire->SetVelocityVar(.01f);
		m_PSFire->SetTextureID(m_TexFire);
		m_PSFire->SetColor(color);
		m_PSFire->Init(re);
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(m_TexFire);
	re.GLSL().SpriteSizeShader().SetColor(m_PSFire->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSFire->GetSize()*100*effectScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSFire->Update())
		m_PSFire->Render(projection, model, re, effectScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void AuItem::RenderCandleFlame(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale)
{
	if(!m_TexFire)
		m_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_PSFire)
	{
		m_PSFire = new AuFireParticles(.025f);
		m_PSFire->SetMaxParticles(1000);
		m_PSFire->SetLifeCycle(2.5f);
		m_PSFire->SetVelocityVar(.01f);
		m_PSFire->SetNumToRelease(3);
		m_PSFire->SetReleaseInterval(0.05f);
		QColor colFire;
		colFire.setRgbF(1, .25, .15);
		m_PSFire->SetColor(colFire);
		m_PSFire->SetSize(.2f);
		m_PSFire->SetTextureID(m_TexFire);
		m_PSFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSFire->Init(re);
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(m_TexFire);
	re.GLSL().SpriteSizeShader().SetColor(m_PSFire->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSFire->GetSize()*100*effectScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSFire->Update())
		m_PSFire->Render(projection, model, re, effectScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void AuItem::RenderFire(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale,
		double radius)
{
	if(!m_TexFire)
		m_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_PSFire)
	{
		m_PSFire = new AuFireParticles(radius);
		m_PSFire->SetMaxParticles(1000);
		m_PSFire->SetLifeCycle(1.5f);
		m_PSFire->SetVelocityVar(radius * .67f);
		m_PSFire->SetNumToRelease(20);
		m_PSFire->SetReleaseInterval(0.05f);
		QColor colFire;
		colFire.setRgbF(1, .25, .15);
		m_PSFire->SetColor(colFire);
		m_PSFire->SetSize(radius);
		m_PSFire->SetTextureID(m_TexFire);
		m_PSFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSFire->Init(re);
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(m_TexFire);
	re.GLSL().SpriteSizeShader().SetColor(m_PSFire->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSFire->GetSize()*100*effectScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSFire->Update())
		m_PSFire->Render(projection, model, re, effectScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

void AuItem::RenderTorchFlame(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, double effectScale)
{
	if(!m_TexFire)
		m_TexFire = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");

	if(!m_PSFire)
	{
		m_PSFire = new AuFireParticles(.3f);
		m_PSFire->SetMaxParticles(1000);
		m_PSFire->SetLifeCycle(1.5f);
		m_PSFire->SetVelocityVar(.25f);
		m_PSFire->SetNumToRelease(20);
		m_PSFire->SetReleaseInterval(0.05f);
		QColor colFire;
		colFire.setRgbF(1, .25, .15);
		m_PSFire->SetColor(colFire);
		m_PSFire->SetSize(.75f);
		m_PSFire->SetTextureID(m_TexFire);
		m_PSFire->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSFire->Init(re);
	}

	re.GLSL().SpriteSizeShader().Select();
	re.GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	re.GLSL().SpriteSizeShader().SetTexture(m_TexFire);
	re.GLSL().SpriteSizeShader().SetColor(m_PSFire->GetColor());
	re.GLSL().SpriteSizeShader().SetSize(m_PSFire->GetSize()*100*effectScale);

	// Render the model of this atom.
	re.glEnable(GL_DEPTH_TEST);
	re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	if(m_PSFire->Update())
		m_PSFire->Render(projection, model, re, effectScale);
	re.glDepthMask(GL_TRUE);
	re.glDisable(GL_BLEND);

	re.GLSL().SpriteSizeShader().Deselect();
}

bool AuItem::ModelExtents(AuPoint& min, AuPoint& max) const
{
	COMMODITYID cid = CommodityId();
	if(cid == CID_Invalid)
		return false;

	if(!m_Model)
	{
		QString modelFile = g_Au.CommodityModel(cid);
		if(modelFile.isEmpty() || !QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";				// The model file does not exist so substitute with the bold cube.

		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		AuGLModelInstance* model = models.CreateGeometry(modelFile);
		g_Au.UnlockGLModels();
		if(!model)
			return false;
		model->GetExtents(min, max);
		if(g_Au.IsSceneWorker())
			m_Model = model;	// Model is not good enough for the client.
	}
	else
		m_Model->GetExtents(min, max);
	return true;
}

// 'drop' 0=default random drop, 1=never drop, 2=always drop
void AuItem::SetDropOnDeath(int drop)
{
	m_Bits &= ~ITEM_DropBits;		// Turn off all drop bits.
	if(drop == 1)
		m_Bits |= ITEM_NoDrop;	// Turn on 'no drop' bit.
	else if(drop == 2)
		m_Bits |= ITEM_Slippery;	// Turn on 'slippery' bit.
}

const DbCommodity& AuItem::Commodity() const
{
	return g_Au.Commodity(CommodityId());
}

QString AuItem::ListName() const
{
	if(CommodityId() == CID_Paper && Inscription().isEmpty())
		return QObject::tr("Blank Paper");
	return g_Au.CommodityName(CommodityId());
}

float AuItem::UnitCost() const
{
	// Get the commodity/quality cost of the item.
	float unitCost = g_Au.CommodityQualityCost(CommodityId(), Quality());

	// Modulate the cost by the condition of the item.
	if(HitPoints() >= NewHitPoints() || NewHitPoints() == 0)
		return unitCost;									// The item is in 'like nu' condition.

	float conditionMod = (float)HitPoints() / (float)NewHitPoints();
	conditionMod *= conditionMod;							// Price drops off exponentially due to worn condition.
	return unitCost * conditionMod;
}

void AuItem::ListPatents(AuKnownPatents& patents) const
{
	if(DbCommodity::IsPatent(CommodityId()))
		patents.RecordPatent(CommodityId(), Quality());
}

float AuItem::LightFlicker(const AuId& id) const
{
	return LightFlicker(CommodityId(), id);
}

//static
float AuItem::LightFlicker(COMMODITYID cid, const AuId& id)
{
	switch(cid)
	{
	case CID_My_Head:	// Entity
		{
			quint32 msec = (quint32)((AuTime::CurrentMSecsSinceEpoch() + (id.GetID()*100)) / 100);
			AuDice dice;
			dice.Seed(msec);
			float fact = dice.Roll(1,21,-11);   // +/- 10
			return (1.f + fact/100.f);
		}
	case CID_BurningBarrel:
	case CID_StoneFirePit:
	case CID_Torch:
		{
			quint32 msec = (quint32)((AuTime::CurrentMSecsSinceEpoch() + (id.GetID()*100)) / 100);
			AuDice dice;
			dice.Seed(msec);
			float fact = dice.Roll(1,31,-16);   // +/- 15
			return (1.f + fact/100.f);
		}
	case CID_Beacon:
		return AuTime::CurrentSecCycle();
	default:
		return 1.f;
	}
}

bool AuItem::CanCombine() const
{
	if(Content() && !Content()->empty())
		return false;
	return DbCommodity::CanCombine(CommodityId());
}

bool AuItem::CanStack(const AuItem* item) const
{
	if(!CanCombine())
		return false;
	if(CommodityId() != item->CommodityId())
		return false;
	if(Inscription() != item->Inscription())
		return false;
	if(m_HitPoints != item->m_HitPoints)
		return false;
	if(m_StoryInstance != item->m_StoryInstance)
		return false;
	return true;
}

quint64 AuItem::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_CID);
	mem += sizeof(m_Count);
	mem += sizeof(m_EmpireOfOrigin);
	mem += sizeof(m_HitPoints);
	mem += StringMemoryUsage(m_Inscription);
	mem += sizeof(m_Quality);
	mem += sizeof(m_StoryInstance);
	mem += sizeof(m_Bits);

	mem += sizeof(m_AudioSource);
	mem += sizeof(m_CurrentSound);

	mem += sizeof(m_Model);
	mem += sizeof(m_PSFire);
	mem += sizeof(m_TexFire);
	return mem;
}

QSize AuItem::BitmapSize(int baseSize, int sizeStep) const
{
	int width = baseSize;
	int height = baseSize;
	QSize packSize(PackSize());
	if(packSize.width() > 0 && packSize.height() > 0)
	{
		width += (packSize.width()-1) * sizeStep;
		height += (packSize.height()-1) * sizeStep;
	}
	return QSize(width, height);
}

QSize AuItem::BitmapSizeToFit(int fitSize) const
{
    int width = fitSize;
    int height = fitSize;
    QSize packSize(PackSize());
    if(packSize.width() > 0 && packSize.height() > 0)
    {
        if(packSize.width() < packSize.height())
        {
            // Pack size width is less that pack size height. Reduce width according to aspect ratio.
            width = width * packSize.width() / packSize.height();
        }
        else if(packSize.width() > packSize.height())
        {
            // Pack size width is more that pack size height. Reduce height according to aspect ratio.
            height = height * packSize.height() / packSize.width();
        }
    }
	return QSize(width, height);
}

int AuItem::CoverHair(const AuFolliclePoint& fp, bool wearsBra) const
{
	switch(CommodityId())
	{
	case CID_Underwear:
		return CoverHairUnderwear(fp, wearsBra);	// Scant body coverage.
	case CID_CapriHalterOutfit:
		return CoverHairShorts(fp);		// Scant body coverage.
	case CID_LeatherBodyArmor:
	case CID_MetalBodyArmor:
	case CID_PlasticBodyArmor:
	case CID_WovenBodyArmor:
		return CoverHairArmor(fp);		// Scant body coverage.
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
		return CoverHairClothes(fp);	// Partial body coverage.
	case CID_ParcXUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
		return CoverHairUniform(fp);	// Partial body coverage.
	case CID_LeatherHelmet:
	case CID_MetalHelmet:
	case CID_PlasticHelmet:
	case CID_WovenHelmet:
		return CoverHairHat(fp);		// Partial head coverage.
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
		return CoverHairVacSuit(fp);	// Complete body coverage, including hands and feet.
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
		return CoverHairHelmet(fp);		// Complete head coverage, except face.
	default:
		break;
	}
	return 0;
}

int AuItem::CoverHairArmor(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
		break;	// No coverage on head or neck.
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
		return 2;
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
		return 1;
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairUnderwear(const AuFolliclePoint& fp, bool wearsBra) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
	case AuFolliclePoint::Shoulders:
		break;
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
		if(wearsBra)
			return 2;
		break;
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
		return 2;
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairShorts(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
		break;
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
		return 2;
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
		return 1;
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
		break;
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
		return 2;
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairClothes(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
		break;
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
		return 2;
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
		return 1;
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
		break;
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
		return 2;
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairUniform(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
		break;
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
		return 2;
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
		break;
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
		return 2;
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairHat(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
		return 1;
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
		break;
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
		return 1;
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairHelmet(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
		return 1;
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
	case AuFolliclePoint::ROLE_Count:
		break;
	}
	return 0;
}

int AuItem::CoverHairVacSuit(const AuFolliclePoint& fp) const
{
	switch(fp.Role())
	{
	case AuFolliclePoint::BackOfHead:
	case AuFolliclePoint::Bangs:
	case AuFolliclePoint::Crown:
	case AuFolliclePoint::Scalp:
	case AuFolliclePoint::SideLeft:
	case AuFolliclePoint::SideRight:
	case AuFolliclePoint::SideburnLeft:
	case AuFolliclePoint::SideburnRight:
	case AuFolliclePoint::HeadNeckBack:
	case AuFolliclePoint::HeadNeckFront:
	case AuFolliclePoint::CheekLeft:
	case AuFolliclePoint::CheekRight:
	case AuFolliclePoint::ChinFront:
	case AuFolliclePoint::ChinLeft:
	case AuFolliclePoint::ChinRight:
	case AuFolliclePoint::EarLeft:
	case AuFolliclePoint::EarMiddle:
	case AuFolliclePoint::EarRight:
	case AuFolliclePoint::EyeLeft:
	case AuFolliclePoint::EyeMiddle:
	case AuFolliclePoint::EyeRight:
	case AuFolliclePoint::Face:
	case AuFolliclePoint::LipTop:
	case AuFolliclePoint::ChopLeft:
	case AuFolliclePoint::ChopRight:
	case AuFolliclePoint::JowlLeft:
	case AuFolliclePoint::JowlRight:
	case AuFolliclePoint::Nose:
		break;
	case AuFolliclePoint::NeckBack:
	case AuFolliclePoint::NeckSides:
	case AuFolliclePoint::NeckUnder:
		return 1;
	case AuFolliclePoint::Shoulders:
	case AuFolliclePoint::Back:
	case AuFolliclePoint::Chest:
	case AuFolliclePoint::FlanksBack:
	case AuFolliclePoint::FlanksUnder:
	case AuFolliclePoint::Butt:
	case AuFolliclePoint::Groin:
	case AuFolliclePoint::ArmBack:
	case AuFolliclePoint::ArmUnder:
	case AuFolliclePoint::WristBack:
	case AuFolliclePoint::WristUnder:
	case AuFolliclePoint::Hands:
	case AuFolliclePoint::Palms:
	case AuFolliclePoint::LegBack:
	case AuFolliclePoint::LegUnder:
	case AuFolliclePoint::AnkleBack:
	case AuFolliclePoint::AnkleUnder:
	case AuFolliclePoint::Feet:
	case AuFolliclePoint::Soles:
	case AuFolliclePoint::ROLE_Count:
	case AuFolliclePoint::TailBack:
	case AuFolliclePoint::TailUnder:
	case AuFolliclePoint::TailTipBack:
	case AuFolliclePoint::TailTipUnder:
		return 2;
	}
	return 0;
}

bool AuItem::BareNeckCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
		return true;
	case CID_CapriHalterOutfit:
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_ParcXUniform:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareUpperTorsoCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_ParcXUniform:
	case CID_LeatherBodyArmor:
	case CID_WovenBodyArmor:
	case CID_PlasticBodyArmor:
	case CID_MetalBodyArmor:
		return true;
	case CID_CapriHalterOutfit:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareLowerTorsoCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_ParcXUniform:
	case CID_CapriHalterOutfit:
		return true;
	case CID_LeatherBodyArmor:
	case CID_WovenBodyArmor:
	case CID_PlasticBodyArmor:
	case CID_MetalBodyArmor:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareUpperArmCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_ParcXUniform:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_PlasticBodyArmor:
		return true;
	case CID_CapriHalterOutfit:
	case CID_LeatherBodyArmor:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareForeArmCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_ParcXUniform:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
		return true;
	case CID_PlasticBodyArmor:
	case CID_CapriHalterOutfit:
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
	case CID_JeansAndShirt:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareHandCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
		return true;
	case CID_CapriHalterOutfit:
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_ParcXUniform:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareThighCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_ParcXUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_CapriHalterOutfit:
		return true;
	case CID_LeatherBodyArmor:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareCalfCovered() const
{
	switch(CommodityId())
	{
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_ParcXUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
		return true;
	case CID_CapriHalterOutfit:
	case CID_LeatherBodyArmor:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

bool AuItem::BareFootCovered() const
{
	switch(CommodityId())
	{
	case CID_CapriHalterOutfit:
	case CID_LeatherClothes:
	case CID_JeansAndShirt:
	case CID_BlackUniform:
	case CID_BlueUniform:
	case CID_BrownUniform:
	case CID_GoldUniform:
	case CID_GreenUniform:
	case CID_GreyUniform:
	case CID_OrangeUniform:
	case CID_PinkUniform:
	case CID_RedUniform:
	case CID_TealUniform:
	case CID_WhiteUniform:
	case CID_GreyBDU:
	case CID_KhakiBDU:
	case CID_OliveDrabBDU:
	case CID_MagmiumPowerArmor:
	case CID_MetalPowerArmor:
	case CID_ParcXUniform:
	case CID_VacSuit:
	case CID_VulciumPowerArmor:
		return true;
	case CID_LeatherBodyArmor:
	case CID_LeatherHelmet:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmorHelmet:
	case CID_PlasticBodyArmor:
	case CID_PlasticHelmet:
	case CID_VacSuitHelmet:
	case CID_VulciumPowerArmorHelmet:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
	default:
		break;
	}
	return false;
}

int AuItem::AimAxis() const
{
	COMMODITYID cid = CommodityId();
	if(cid == CID_Invalid)
		return 0;
	int aimAxis = -1;
	if(!m_Model)
	{
		QString modelFile = g_Au.CommodityModel(cid);
		if(!modelFile.isEmpty() && QFile::exists(modelFile))
		{
			AuGLModelCache& models = g_Au.LockGLModelsForWrite();
			AuGLModelInstance* model = models.CreateGeometry(modelFile);
			g_Au.UnlockGLModels();
			if(model)
			{
				aimAxis = model->AimAxis();
				if(g_Au.IsSceneWorker())
					m_Model = model;	// Model is not good enough for the client.
			}
		}
	}
	else
		aimAxis = m_Model->AimAxis();

	if(aimAxis < 0)
		aimAxis = Commodity().AimAxis();	// Get aim axis from the commodity record.
	return aimAxis;
}

GROUND_ORIENT AuItem::GroundOrientation() const
{
	COMMODITYID cid = CommodityId();
	if(cid == CID_Invalid)
		return GORI_ZPosUp;
	GROUND_ORIENT gori = GORI_Count;
	if(!m_Model)
	{
		QString modelFile = g_Au.CommodityModel(cid);
		if(!modelFile.isEmpty() && QFile::exists(modelFile))
		{
			AuGLModelCache& models = g_Au.LockGLModelsForWrite();
			AuGLModelInstance* model = models.CreateGeometry(modelFile);
			g_Au.UnlockGLModels();
			if(model)
			{
				gori = model->GroundOrientation();
				if(g_Au.IsSceneWorker())
					m_Model = model;	// Model is not good enough for the client.
			}
		}
	}
	else
		gori = m_Model->GroundOrientation();
	if(gori != GORI_Count)
		return gori;	// Model gave me a valid ground orientation.

	// Get ground up vector from the DbCommodity record.
	AuPoint upv(Commodity().m_UpVector);
	if(upv.z == 1)
		return GORI_ZPosUp;
	else if(upv.z == -1)
		return GORI_ZNegUp;
	else if(upv.y == 1)
		return GORI_YPosUp;
	else if(upv.y == -1)
		return GORI_YNegUp;
	else if(upv.x == 1)
		return GORI_XPosUp;
	else// if(upv.x == -1)
		return GORI_XNegUp;
}

AuPoint AuItem::GroundUpVector() const
{
	COMMODITYID cid = CommodityId();
	if(cid == CID_Invalid)
		return AuPoint(0,0,1);
	GROUND_ORIENT gori = GORI_Count;
	if(!m_Model)
	{
		QString modelFile = g_Au.CommodityModel(cid);
		if(!modelFile.isEmpty() && QFile::exists(modelFile))
		{
			AuGLModelCache& models = g_Au.LockGLModelsForWrite();
			AuGLModelInstance* model = models.CreateGeometry(modelFile);
			g_Au.UnlockGLModels();
			if(model)
			{
				gori = model->GroundOrientation();
				if(g_Au.IsSceneWorker())
					m_Model = model;	// Model is not good enough for the client.
			}
		}
	}
	else
		gori = m_Model->GroundOrientation();

	// .SoH models report their ground orientation.
	switch(gori)
	{
	case GORI_ZPosUp:
		return AuPoint(0,0,1);
	case GORI_ZNegUp:
		return AuPoint(0,0,-1);
	case GORI_YPosUp:
		return AuPoint(0,1);
	case GORI_YNegUp:
		return AuPoint(0,-1);
	case GORI_XPosUp:
		return AuPoint(1,0);
	case GORI_XNegUp:
		return AuPoint(-1,0);
	case GORI_Count:
		break;
	}

	// Get ground up vector from the DbCommodity record.
	return Commodity().m_UpVector;
}

float AuItem::GroundHeightAboveCG() const
{
	switch(CommodityId())
	{
	case CID_PressureTent:
		return 0;
	default:
		break;
	}

	AuPoint min,max;
	if(ModelExtents(min, max))
	{
		switch(GroundOrientation())
		{
		case GORI_ZPosUp:
			return -min.z;
		case GORI_ZNegUp:
			return max.z;
		case GORI_YPosUp:
			return -min.y;
		case GORI_YNegUp:
			return max.y;
		case GORI_XPosUp:
			return -min.x;
		case GORI_XNegUp:
			return max.x;
		case GORI_Count:
			break;
		}
	}
	return 0;
}

AuItem* AuItem::FixItem() const
{
	if(CommodityId() == CID_ComputerObsolete)
	{
		// Computers were upgraded to be storage devices.
		return Create(CID_Computer, Quality(), EmpireOfOrigin(), Count());
	}
	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemDNA::AuItemDNA(const AuDNABase* dna, AuDNABase::CHROMATID chromatid, bool dnaViable, bool sentient, HANDEDNESS hand,
		GROWTHSTAGE stage, AuHairStyle::STYLE hair, AuHairStyle::SHAVE shave, COMMODITYID cid, quint8 quality,
		quint32 empireOfOrigin, quint32 count) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Bounty = 0;
	if(dna)
		m_DNA = dna->Clone();
	else
		m_DNA = 0;
	m_DNADead = !dnaViable;
	m_DNAChromatid = chromatid;
	m_DNAHair = hair;
	m_DNAHand = hand;
	m_DNAShave = shave;
	m_DNAStage = stage;
	m_HeadAnimalia = 0;
	m_HeadBody = 0;
	m_Sentient = sentient;
}

AuItemDNA::AuItemDNA(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Bounty = 0;
	m_DNA = 0;
	m_DNADead = false;
	m_DNAChromatid = AuDNABase::Alpha;
	m_DNAHair = AuHairStyle::Unkempt;
	m_DNAHand = RightHanded;
	m_DNAShave = AuHairStyle::Unshaven;
	m_DNAStage = Mature;
	m_HeadAnimalia = 0;
	m_HeadBody = 0;
	m_Sentient = false;
}

AuItemDNA::AuItemDNA()
{
	m_Bounty = 0;
	m_DNA = 0;
	m_DNADead = false;
	m_DNAChromatid = AuDNABase::Alpha;
	m_DNAHair = AuHairStyle::Unkempt;
	m_DNAHand = RightHanded;
	m_DNAShave = AuHairStyle::Unshaven;
	m_DNAStage = Mature;
	m_HeadAnimalia = 0;
	m_HeadBody = 0;
	m_Sentient = false;
}

AuItemDNA::~AuItemDNA()
{
	if(m_Bounty)
		delete m_Bounty;
	if(m_DNA)
		delete m_DNA;
	if(m_HeadAnimalia)
		delete m_HeadAnimalia;
	if(m_HeadBody)
		delete m_HeadBody;
}

AuItemDNA::AuItemDNA(const AuItemDNA& c) : AuItem(c)
{
	if(c.m_Bounty)
		m_Bounty = new AuBountyOnItem(*c.m_Bounty);
	else
		m_Bounty = 0;
	if(c.m_DNA)
		m_DNA = c.m_DNA->Clone();
	else
		m_DNA = 0;
	m_DNADead = c.m_DNADead;
	m_DNAChromatid = c.m_DNAChromatid;
	m_DNAHair = c.m_DNAHair;
	m_DNAHand = c.m_DNAHand;
	m_DNAShave = c.m_DNAShave;
	m_DNAStage = c.m_DNAStage;
	m_HeadAnimalia = 0;
	m_HeadBody = 0;
	m_Sentient = c.m_Sentient;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemDNA::Clone() const
{
	return new AuItemDNA(*this);
}

QString AuItemDNA::ListName() const
{
	if(!HasDNA())
	{
		return AuItem::ListName();
	}
	else
	{
		if(CommodityId() == CID_Head && !Inscription().isEmpty())
		{
			QString name = Inscription();
			int idx = name.indexOf(" killed by");
			if(idx > 1)
				name = name.left(idx);
			double bounty = Bounty().Amount();
			if(bounty > 0)
				return QObject::tr("%2, %1 %L3%4")
					.arg(name)
					.arg(AuItem::ListName())
					.arg(bounty, 0,'f',0)
					.arg(AuGlobal::CurrencySymbol());
			else
				return QObject::tr("%2, %1")
					.arg(name)
					.arg(AuItem::ListName());
		}
		else
		{
			// Get the name of the species.
			const DgData& gd = g_Au.LockDataForRead();
			QString speciesName(gd.SpeciesName(m_DNA->Species(), true));
			g_Au.UnlockData();
			return QObject::tr("%2, %1").arg(speciesName).arg(AuItem::ListName());
		}
	}
}

QString AuItemDNA::SayName() const
{
	if(!HasDNA())
	{
		return AuItem::SayName();
	}
	else
	{
		// Get the name of the specie.
		QString speciesName;
		const DgData& gd = g_Au.LockDataForRead();
		speciesName = gd.SpeciesName(m_DNA->Species(), true);
		g_Au.UnlockData();
		return QObject::tr("%1 %2 %3").arg(DbCommodity::QualityDesc(Quality())).arg(speciesName).arg(AuItem::ListName());
	}
}

GENDER AuItemDNA::DNAGender() const
{
	if(!m_DNA)
		return Male;
	return m_DNA->Gender(m_DNAChromatid);
}

QString AuItemDNA::DNAGenderName() const
{
	return GenderName(DNAGender());
}

QString AuItemDNA::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// DNA Description
	if(HasDNA())
	{
		// Get the name of the specie, or a blank name when the specie is not named.
		const DgData& gd = g_Au.LockDataForRead();
		QString speciesName = gd.SpeciesName(m_DNA->Species(), false);
		g_Au.UnlockData();

		if(speciesName.isEmpty())
			itemDesc += QObject::tr("DNA<div style=\"margin-left: 10px;\"><b>%1<br>%2<br>%6 %3 %4<br>%5</b></div>")
				.arg(m_DNA->SpeciesDescription())
				.arg(m_DNA->TimeActiveName())
				.arg(DNAIsSentient()?QObject::tr("Sentient"):QObject::tr("Animal"))
				.arg(DNAGenderName())
				.arg(m_DNADead?QObject::tr("DNA Not Viable"):QObject::tr("DNA Viable"))
				.arg(DNAGrowthStage() == Juvenile ? QObject::tr("Juvenile"):QObject::tr("Mature"));
		else
			itemDesc += QObject::tr("DNA<div style=\"margin-left: 10px;\"><b>%1<br>%2<br>%3<br>%7 %4 %5<br>%6</b></div>")
				.arg(speciesName)
				.arg(m_DNA->SpeciesDescription())
				.arg(m_DNA->TimeActiveName())
				.arg(DNAIsSentient()?QObject::tr("Sentient"):QObject::tr("Animal"))
				.arg(DNAGenderName())
				.arg(m_DNADead?QObject::tr("DNA Not Viable"):QObject::tr("DNA Viable"))
				.arg(DNAGrowthStage() == Juvenile ? QObject::tr("Juvenile"):QObject::tr("Mature"));
	}
	else
		itemDesc += QObject::tr("DNA<div style=\"margin-left: 10px;\"><b>Unrecognizable</b></div>");

	if(m_Bounty)
	{
		itemDesc += QObject::tr("Bounty<div style=\"margin-left: 10px;\">"
				//"On Who <b>%1 <i>%2</i></b><br>"
				"Amount <b>%L3%4</b><br>"
				"Reason <b>%5</b><br>"
				"By Who <b>%6 <i>%7</i></b>"
				"</div>")
			//.arg(m_Bounty->Name())
			//.arg(m_Bounty->Id().StringId())
			.arg(m_Bounty->Amount(), 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(m_Bounty->Reason())
			.arg(m_Bounty->PosterName())
			.arg(m_Bounty->PosterId().StringId());
	}

	return itemDesc;
}

void AuItemDNA::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);

	quint8 ui8 = 0;
	if(m_Sentient)
		ui8 |= 0x01;
	if(m_DNADead)
		ui8 |= 0x02;
	if(m_Bounty)
		ui8 |= 0x04;
	if(m_DNA)
		ui8 |= 0x08;	// bit says we are using AuDNABase now.
	if(m_DNAStage == Juvenile)
		ui8 |= 0x10;

	out << QByteArray();	// Old version serialized a byte array before the ui8 byte. :/
	out << ui8;

	if(m_DNA)
	{
		m_DNA->WriteDynamic(out);
		// Reverse chromatid so the value matches male/female gender, which is what used to be serialized here.
		quint8 gc = (m_DNAChromatid == AuDNABase::Alpha) ? AuDNABase::Beta : AuDNABase::Alpha;
		quint8 genderHand = (gc << 4) | m_DNAHand;
		out << genderHand;
		out << (quint8)m_DNAHair;
		out << (quint8)m_DNAShave;
	}

	if(ui8 & 0x04)
		out << *m_Bounty;
}

void AuItemDNA::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);

	if(m_DNA)
	{
		delete m_DNA;
		m_DNA = 0;
	}

	QByteArray ba;
	quint8 ui8;

	// Old version serialized a DhDNA then the ui8 byte. A DhDNA is serialized as a QByteArray in its own operator>>.
	in >> ba;			// This will be non-empty if we read an old style DhDNA.
	in >> ui8;
	if(ui8 & 0x08)
	{
		m_DNA = AuDNABase::ReadDynamic(in);
		quint8 ui8dna;
		in >> ui8dna;
		quint8 gc = ui8dna >> 4;											// top nibble used to be gender
		m_DNAChromatid = (gc == AuDNA::Alpha) ? AuDNA::Beta : AuDNA::Alpha;	// unswap the chromatids or change gender to chromatid
		m_DNAHand = (HANDEDNESS)(ui8dna & 0x0f);							// bottom nibble is handedness
		in >> ui8dna;
		m_DNAHair = (AuHairStyle::STYLE)ui8dna;
		in >> ui8dna;
		m_DNAShave = (AuHairStyle::SHAVE)ui8dna;
	}
	else if(!ba.isEmpty())
	{
		m_DNA = new DhDNA(ba);
		if(*m_DNA == DhDNA())
		{
			delete m_DNA;
			m_DNA = 0;
		}
	}

	m_Sentient = ui8 & 0x01;
	m_DNADead = ui8 & 0x02;
	if(ui8 & 0x10)
		m_DNAStage = Juvenile;
	else
		m_DNAStage = Mature;

	if(ui8 & 0x04)
	{
		if(!m_Bounty)
			m_Bounty = new AuBountyOnItem();
		in >> *m_Bounty;
	}
	else if(m_Bounty)
	{
		delete m_Bounty;
		m_Bounty = 0;
	}
	//qDebug() << "AuItemDNA::operator<<" << ListName();
}

void AuItemDNA::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemDNA* c = dynamic_cast<const AuItemDNA*>(item);
	if(!c)
		return;

	// This test is close enough. DNA doesn't really come and go from items nor does it change on a rendered item.
	if(re && m_DNA && c->m_DNA && *m_DNA != *c->m_DNA)
		DeleteRenderObjects(*re, al);

	if(c->m_Bounty)
	{
		if(!m_Bounty)
			m_Bounty = new AuBountyOnItem(*c->m_Bounty);
		else
			*m_Bounty = *c->m_Bounty;
	}
	else if(m_Bounty)
	{
		delete m_Bounty;
		m_Bounty = 0;
	}

	if(m_DNA)
		delete m_DNA;
	if(c->m_DNA)
		m_DNA = c->m_DNA->Clone();
	else
		m_DNA = 0;
		
	m_DNADead = c->m_DNADead;
	m_DNAChromatid = c->m_DNAChromatid;
	m_DNAHair = c->m_DNAHair;
	m_DNAHand = c->m_DNAHand;
	m_DNAShave = c->m_DNAShave;
	m_DNAStage = c->m_DNAStage;
	m_Sentient = c->m_Sentient;
	//qDebug() << "AuItemDNA::Refresh" << ListName();
}

bool AuItemDNA::DNAWantsAnimaliaModel() const
{
	if(m_HeadAnimalia)
		return false;	// We already have the model.
	if(!m_DNA)
		return false;	// No DNA.
	if(m_DNA->Version() != 4)
		return false;	// Not animalia DNA.
	return true;
}

void AuItemDNA::DNASetAnimaliaModel(AuAnimalia* animalia)
{
	Q_ASSERT(!m_HeadAnimalia);	// Don't do this twice.
	m_HeadAnimalia = animalia;
}

void AuItemDNA::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	AuItem::DeleteRenderObjects(re, al);
	if(m_HeadAnimalia)
	{
		m_HeadAnimalia->DeleteGL();
		delete m_HeadAnimalia;
		m_HeadAnimalia = 0;
	}
	if(m_HeadBody)
	{
		m_HeadBody->DeleteDeviceObjects(re);
		delete m_HeadBody;
		m_HeadBody = 0;
	}
}

double AuItemDNA::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		const AuPoint& windVec, float scale, int pass, double effectScale)
{
	if(CommodityId() != CID_Head)
		return AuItem::Render(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, windVec, scale,
				pass, effectScale);

	if(!HasDNA())
		return 0;

	// Determine if this creature is in front of the camera plane.
	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z < 0 || pv.Length2() < 10*10)
	{
		// Create device objects.
		if(DNA()->Version() == 3)
		{
			if(pass > 1)
				return 0;
			if(!m_HeadBody)
			{
				const DhDNA* dna3 = (const DhDNA*)DNA();
				m_HeadBody = new DpBody(0, 0, *dna3);
				m_HeadBody->Pose(AuPose::EyesOnly, false, 1);
				m_HeadBody->CreateDeviceObjects(re, mc, CID_Invalid, CID_Invalid, 1);
			}

			// Render the head.
			re.SetAmbientLight(ambientR, ambientG, ambientB, 1);
			m_HeadBody->RenderSkin(projection, model, re, ambientUp, false);
		}
		else if(DNA()->Version() == 4)
		{
			if(!m_HeadAnimalia)
			{
				const AuDNA* dna4 = (const AuDNA*)DNA();
				m_HeadAnimalia = new AuAnimalia(*dna4, m_DNAChromatid, m_DNAHand, m_DNAStage);	// Handedness affects hair style. :/
				m_HeadAnimalia->SetHairStyle(m_DNAHair, m_DNAShave);
				if(!m_HeadAnimalia->HasVAOs())
				{
					m_HeadAnimalia->Pose(id, AuPose::EyesOnly, false, 1);
					m_HeadAnimalia->CreateClothingTex(re);
					m_HeadAnimalia->CreateAnimalVAO(re, 0);
				}
			}
			if(pass == 1)
			{
				m_HeadAnimalia->RenderHead(projection, model, re, false, 0, ambientUp);
				return pv.Length2();
			}
			else if(pass == 2)
			{
				m_HeadAnimalia->RenderHead(projection, model, re, true, 0, ambientUp);
			}
			else
			{
				m_HeadAnimalia->RenderHead(projection, model, re, false, 0, ambientUp);
				m_HeadAnimalia->RenderHead(projection, model, re, true, 0, ambientUp);
			}
		}
	}
	return 0;
}

double AuItemDNA::RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		const AuPoint& globeP,
		const AuPoint& windVec, float scale, int pass)
{
	if(CommodityId() != CID_Head)
		return AuItem::RenderOnGlobe(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp,
				fogDensity, fogR, fogG, fogB, globeP, windVec, scale, pass);

	if(!HasDNA())
		return 0;

	// Determine if this creature is in front of the camera plane.
	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z < 0 || pv.Length2() < 10*10)
	{
		// Create device objects.
		if(DNA()->Version() == 3)
		{
			if(pass > 1)
				return 0;
			if(!m_HeadBody)
			{
				const DhDNA* dna3 = (const DhDNA*)DNA();
				m_HeadBody = new DpBody(0, 0, *dna3);
				m_HeadBody->Pose(AuPose::EyesOnly, false, 1);
				m_HeadBody->CreateDeviceObjects(re, mc, CID_Invalid, CID_Invalid, 1);
			}

			// Render the head.
			re.SetAmbientLight(ambientR, ambientG, ambientB, 1);
			re.SetFog(fogR, fogG, fogB, fogDensity);
			m_HeadBody->RenderSkinFogGlobe(projection, model, re, ambientUp, globeP, false);
		}
		else if(DNA()->Version() == 4)
		{
			if(!m_HeadAnimalia)
			{
				const AuDNA* dna4 = (const AuDNA*)DNA();
				m_HeadAnimalia = new AuAnimalia(*dna4, m_DNAChromatid, m_DNAHand, m_DNAStage);	// Handedness affects hair style. :/
				m_HeadAnimalia->SetHairStyle(m_DNAHair, m_DNAShave);
				if(!m_HeadAnimalia->HasVAOs())
				{
					m_HeadAnimalia->Pose(id, AuPose::EyesOnly, false, 1);
					m_HeadAnimalia->CreateClothingTex(re);
					m_HeadAnimalia->CreateAnimalVAO(re, 0);
				}
			}
			if(pass == 1)
			{
				m_HeadAnimalia->RenderHeadGlobeFog(projection, model, re, false, 0, ambientUp, globeP);
				return pv.Length2();
			}
			else if(pass == 2)
			{
				m_HeadAnimalia->RenderHeadGlobeFog(projection, model, re, true, 0, ambientUp, globeP);
			}
			else
			{
				m_HeadAnimalia->RenderHeadGlobeFog(projection, model, re, false, 0, ambientUp, globeP);
				m_HeadAnimalia->RenderHeadGlobeFog(projection, model, re, true, 0, ambientUp, globeP);
			}
		}
	}
	return 0;
}

double AuItemDNA::RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		float shadowPanelSunAngles[MAX_SUNS],
		const AuPoint& windVec, float scale, int pass)
{
	if(CommodityId() != CID_Head)
		return AuItem::RenderOnRingworld(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp,
				fogDensity, fogR, fogG, fogB, shadowPanelSunAngles, windVec, scale, pass);

	if(!HasDNA())
		return 0;

	// Determine if this creature is in front of the camera plane.
	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z < 0 || pv.Length2() < 10*10)
	{
		// Create device objects.
		if(DNA()->Version() == 3)
		{
			if(pass > 1)
				return 0;
			if(!m_HeadBody)
			{
				const DhDNA* dna3 = (const DhDNA*)DNA();
				m_HeadBody = new DpBody(0, 0, *dna3);
				m_HeadBody->Pose(AuPose::EyesOnly, false, 1);
				m_HeadBody->CreateDeviceObjects(re, mc, CID_Invalid, CID_Invalid, 1);
			}

			// Render the head.
			re.SetAmbientLight(ambientR, ambientG, ambientB, 1);
			re.SetFog(fogR, fogG, fogB, fogDensity);
			m_HeadBody->RenderSkinFogRingworld(projection, model, re, ambientUp, shadowPanelSunAngles, false);
		}
		else if(DNA()->Version() == 4)
		{
			if(!m_HeadAnimalia)
			{
				const AuDNA* dna4 = (const AuDNA*)DNA();
				m_HeadAnimalia = new AuAnimalia(*dna4, m_DNAChromatid, m_DNAHand, m_DNAStage);	// Handedness affects hair style. :/
				m_HeadAnimalia->SetHairStyle(m_DNAHair, m_DNAShave);
				if(!m_HeadAnimalia->HasVAOs())
				{
					m_HeadAnimalia->Pose(id, AuPose::EyesOnly, false, 1);
					m_HeadAnimalia->CreateClothingTex(re);
					m_HeadAnimalia->CreateAnimalVAO(re, 0);
				}
			}
			if(pass == 1)
			{
				m_HeadAnimalia->RenderHeadRingworldFog(projection, model, re, false, 0, ambientUp, shadowPanelSunAngles);
				return pv.Length2();
			}
			else if(pass == 2)
			{
				m_HeadAnimalia->RenderHeadRingworldFog(projection, model, re, true, 0, ambientUp, shadowPanelSunAngles);
			}
			else
			{
				m_HeadAnimalia->RenderHeadRingworldFog(projection, model, re, false, 0, ambientUp, shadowPanelSunAngles);
				m_HeadAnimalia->RenderHeadRingworldFog(projection, model, re, true, 0, ambientUp, shadowPanelSunAngles);
			}
		}
	}
	return 0;
}

bool AuItemDNA::ModelExtents(AuPoint& min, AuPoint& max) const
{
	if(CommodityId() != CID_Head || !HasDNA())
		return AuItem::ModelExtents(min, max);

	float hl2 = 1;
	float ht2 = 1;
	float hw2 = 1;
	if(DNA()->Version() == 3)
	{
		const DhDNA* dna3 = (const DhDNA*)DNA();

		// This is just a rough wag, close enough.
		float sz = (dna3->OverallSize + 1) * .1f;
		hl2 = sz;
		ht2 = sz;
		hw2 = sz;

		min.x = -hw2;
		min.y = ht2;
		min.z = -hl2;

		max.x = hw2;
		max.y = 0;			// Head origin seems to be right in front of the eyes.
		max.z = hl2;
	}
	else if(DNA()->Version() == 4)
	{
		const AuDNA* dna4 = (const AuDNA*)DNA();
		const AuChromatid& chromatid = dna4->Chain(m_DNAChromatid);
		const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(chromatid.Chromosome(AuChromoHead::ChromosomeType()));
		if(head)
		{
			hw2 = head->RadiusX();
			ht2 = head->RadiusY();
			hl2 = head->RadiusZ();
		}
		min.x = -hw2;
		min.y = -ht2;
		min.z = -hl2;

		max.x = hw2;
		max.y = ht2;
		max.z = hl2;
	}
	else
	{
		min.x = -hw2;
		min.y = -ht2;
		min.z = -hl2;

		max.x = hw2;
		max.y = ht2;
		max.z = hl2;
	}
	return true;
}

quint64 AuItemDNA::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_Bounty);
	mem += sizeof(m_DNA);
	mem += sizeof(m_DNADead);
	mem += sizeof(m_DNAStage);
	mem += sizeof(m_Sentient);
	mem += sizeof(m_HeadBody);
	if(m_HeadBody)
		mem += m_HeadBody->MemoryUsage();
	return mem;
}

bool AuItemDNA::CanStack(const AuItem* item) const
{
	if(CommodityId() != item->CommodityId())
		return false;
	if(m_Sentient != item->DNAIsSentient())
		return false;
	if(m_DNAStage != item->DNAGrowthStage())
		return false;
	if(DNAIsViable() != item->DNAIsViable())
		return false;
	if(!m_DNA && !item->DNA())
		return true;
	else if(!m_DNA || !item->DNA())
		return false;
	if(Bounty() != item->Bounty())
		return false;
	return *m_DNA == *item->DNA();
}

void AuItemDNA::SetDNA(const AuDNABase* dna, bool dnaViable, bool isSentient, GROWTHSTAGE stage)
{
	if(m_DNA)
		delete m_DNA;
	if(dna)
		m_DNA = dna->Clone();
	else
		m_DNA = 0;
	m_DNADead = !dnaViable;
	m_DNAStage = stage;
	m_Sentient = isSentient;
}

float AuItemDNA::ArmorModeAbsorb(WEAPONMODE mode) const
{
	float armorAbsorb = Commodity().ArmorModeAbsorb(mode);
	if(armorAbsorb)
	{
		// DNA affects armor value for items that already have an armor value.
		const DhDNA* dna = dynamic_cast<const DhDNA*>(m_DNA);	//DHH animalia
		if(dna)
			armorAbsorb += (float)(dna->TopSkinThickness + dna->BottomSkinThickness) / 100.f;	// Add 0 to .06 for hide thickness.
	}
	return armorAbsorb;
}

float AuItemDNA::ArmorModeBlock(WEAPONMODE mode) const
{
	float armorBlock = Commodity().ArmorModeBlock(mode);
	if(armorBlock)
	{
		// DNA affects armor value for items that already have an armor value.
		const DhDNA* dna = dynamic_cast<const DhDNA*>(m_DNA);	//DHH animalia
		if(dna)
			armorBlock += (float)(dna->TopSkinThickness + dna->BottomSkinThickness) / 2.f;	// Add 0 to 3 for hide thickness.
		armorBlock *= QualityBoostF();
	}
	return armorBlock;
}

float AuItemDNA::UnitCost() const
{
	float unitCost = AuItem::UnitCost();

	// Boost cost based on armor increase above stock.
	float armorK = Commodity().ArmorModeBlock(WM_Contact);					// Base armor of the commodity vs contact damage.
	if(armorK)
	{
		// DNA affects armor value for items that already have an armor value.
		const DhDNA* dna = dynamic_cast<const DhDNA*>(m_DNA);	//DHH animalia
		if(dna)
		{
			float armorBlock = (float)(dna->TopSkinThickness + dna->BottomSkinThickness) / 2.f;	// Add 0 to 3 for hide thickness.
			float increase = (armorK + armorBlock) / armorK;
			unitCost *= increase;
		}
	}

	return unitCost;
}

AuSpecies AuItemDNA::DNASpecies() const
{
	if(!m_DNA)
		return AuSpecies();
	return m_DNA->Species();
}

bool AuItemDNA::operator<(const AuItem& item) const
{
	try
	{
		const AuItemDNA& c = dynamic_cast<const AuItemDNA&>(item);
		if(m_Sentient < c.m_Sentient)
			return true;
		else if(m_Sentient > c.m_Sentient)
			return false;

		// Reversed for more desirable sorting.
		if(m_DNADead > c.m_DNADead)
			return true;
		else if(m_DNADead < c.m_DNADead)
			return false;

		if(m_DNAStage > c.m_DNAStage)
			return true;
		else if(m_DNAStage < c.m_DNAStage)
			return false;

		if(m_DNA && c.m_DNA)
		{
			AuSpecies s(m_DNA->Species());
			AuSpecies cs(c.m_DNA->Species());
			if(s < cs)
				return true;
			else if(s > cs)
				return false;
		}
		else if(m_DNA)
			return true;
		else if(c.m_DNA)
			return false;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}

	return AuItem::operator<(item);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemMission::AuItemMission(const QString& missionName, const AuMission& mission, COMMODITYID cid, quint8 quality,
		quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Mission = mission;
	m_MissionName = missionName;
}

AuItemMission::AuItemMission(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemMission::AuItemMission()
{
}

AuItemMission::AuItemMission(const AuItemMission& c) : AuItem(c)
{
	m_Mission = c.m_Mission;
	m_MissionName = c.m_MissionName;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemMission::Clone() const
{
	return new AuItemMission(*this);
}

QString AuItemMission::ListName() const
{
	if(!m_MissionName.isEmpty())
		return QObject::tr("Mission: %1").arg(m_MissionName);
	else
		return QObject::tr("%1 With Invalid Data").arg(AuItem::ListName());
}

QString AuItemMission::SayName() const
{
	if(!m_MissionName.isEmpty())
		return QObject::tr("%1 mission").arg(m_MissionName);
	else
		return QObject::tr("%1 with bad data").arg(AuItem::SayName());
}

QString AuItemMission::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(!m_MissionName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += QObject::tr("Mission <b>%1</b><br>%2")
			.arg(MissionName())
			.arg(Mission().HtmlDesc());
	}

	return itemDesc;
}

QString AuItemMission::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(!m_MissionName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += QObject::tr("Mission <b>%1</b><br>%2")
			.arg(MissionName())
			.arg(Mission().HtmlToolTip());
	}

	return itemDesc;
}

void AuItemMission::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_MissionName;
	out << m_Mission;
}

void AuItemMission::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_MissionName;
	in >> m_Mission;
}

void AuItemMission::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemMission* c = dynamic_cast<const AuItemMission*>(item);
	if(!c)
		return;

	m_Mission = c->m_Mission;
	m_MissionName = c->m_MissionName;
}

quint64 AuItemMission::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += m_Mission.MemoryUsage();
	mem += StringMemoryUsage(m_MissionName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemOfficerLog::AuItemOfficerLog(const QString& officerName, const QString& officerHome, const QString& shipName,
		const QString& shipDesc, bool shipPrivate, const AuEventLog& eventLog,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_EventLog = eventLog;
	m_OfficerHome = officerHome;
	m_OfficerName = officerName;
	m_ShipDesc = shipDesc;
	m_ShipName = shipName;
	m_ShipPrivate = shipPrivate;
}

AuItemOfficerLog::AuItemOfficerLog(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_ShipPrivate = false;
}

AuItemOfficerLog::AuItemOfficerLog()
{
	m_ShipPrivate = false;
}

AuItemOfficerLog::AuItemOfficerLog(const AuItemOfficerLog& c) : AuItem(c)
{
	m_EventLog = c.m_EventLog;
	m_OfficerHome = c.m_OfficerHome;
	m_OfficerName = c.m_OfficerName;
	m_ShipDesc = c.m_ShipDesc;
	m_ShipName = c.m_ShipName;
	m_ShipPrivate = c.m_ShipPrivate;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemOfficerLog::Clone() const
{
	return new AuItemOfficerLog(*this);
}

QString AuItemOfficerLog::ListName() const
{
	if(!m_OfficerName.isEmpty() && !m_ShipName.isEmpty())
		return QObject::tr("Officer Log, %1, %2").arg(m_OfficerName).arg(m_ShipName);
	else if(!m_OfficerName.isEmpty())
		return QObject::tr("Officer Log, %1").arg(m_OfficerName);
	else if(!m_ShipName.isEmpty())
		return QObject::tr("Ship Log, %1").arg(m_ShipName);
	else
		return QObject::tr("Officer Log, Blank");
}

QString AuItemOfficerLog::SayName() const
{
	if(!m_OfficerName.isEmpty() && !m_ShipName.isEmpty())
		return QObject::tr("%1 log of %2").arg(m_OfficerName).arg(m_ShipName);
	else if(!m_OfficerName.isEmpty())
		return QObject::tr("%1 log").arg(m_OfficerName);
	else if(!m_ShipName.isEmpty())
		return QObject::tr("%1 log").arg(m_ShipName);
	else
		return QObject::tr("Blank officer log");
}

QString AuItemOfficerLog::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();

	// Officer
	if(!m_OfficerName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		if(!m_OfficerHome.isEmpty())
			itemDesc += QObject::tr("Officer <b>%1 from %2</b>").arg(m_OfficerName).arg(m_OfficerHome);
		else
			itemDesc += QObject::tr("Officer <b>%1</b>").arg(m_OfficerName);
	}

	// Ship
	if(!m_ShipName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		if(m_ShipPrivate)
			itemDesc += QObject::tr("Company Spacecraft <b>%1, %2</b>")
				.arg(m_ShipName)
				.arg(m_ShipDesc);
		else
			itemDesc += QObject::tr("Spacecraft <b>%1, %2</b>")
				.arg(m_ShipName)
				.arg(m_ShipDesc);
	}

	// Event Log
	QString eventRpt = m_EventLog.ReportHtmlToolTip("Event Log");
	if(!eventRpt.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += g_Au.DecodeUTCStrings(eventRpt);	// Decode UTC date/time strings to user's locale.
	}

	return itemDesc;
}

QString AuItemOfficerLog::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();

	// Officer
	if(!m_OfficerName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		if(!m_OfficerHome.isEmpty())
			itemDesc += QObject::tr("Officer <b>%1 from %2</b>").arg(m_OfficerName).arg(m_OfficerHome);
		else
			itemDesc += QObject::tr("Officer <b>%1</b>").arg(m_OfficerName);
	}

	// Ship
	if(!m_ShipName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		if(m_ShipPrivate)
			itemDesc += QObject::tr("Company Spacecraft <b>%1, %2</b>")
				.arg(m_ShipName)
				.arg(m_ShipDesc);
		else
			itemDesc += QObject::tr("Spacecraft <b>%1, %2</b>")
				.arg(m_ShipName)
				.arg(m_ShipDesc);
	}

	return itemDesc;
}

void AuItemOfficerLog::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_OfficerName;
	out << m_OfficerHome;
	out << m_ShipName;
	out << m_ShipDesc;
	out << m_ShipPrivate;
	m_EventLog >> out;
}

void AuItemOfficerLog::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_OfficerName;
	in >> m_OfficerHome;
	in >> m_ShipName;
	in >> m_ShipDesc;
	in >> m_ShipPrivate;
	m_EventLog << in;
}

void AuItemOfficerLog::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemOfficerLog* c = dynamic_cast<const AuItemOfficerLog*>(item);
	if(!c)
		return;

	m_EventLog = c->m_EventLog;
	m_OfficerHome = c->m_OfficerHome;
	m_OfficerName = c->m_OfficerName;
	m_ShipDesc = c->m_ShipDesc;
	m_ShipName = c->m_ShipName;
	m_ShipPrivate = c->m_ShipPrivate;
}

quint64 AuItemOfficerLog::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += m_EventLog.MemoryUsage();
	mem += StringMemoryUsage(m_OfficerHome);
	mem += StringMemoryUsage(m_OfficerName);
	mem += StringMemoryUsage(m_ShipDesc);
	mem += StringMemoryUsage(m_ShipName);
	mem += sizeof(m_ShipPrivate);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemState::AuItemState(quint8 state, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_State = state;
}

AuItemState::AuItemState(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_State = 0;
}

AuItemState::AuItemState()
{
	m_State = 0;
}

AuItemState::AuItemState(const AuItemState& c) : AuItem(c)
{
	m_State = c.m_State;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemState::Clone() const
{
	return new AuItemState(*this);
}

QString AuItemState::ListName() const
{
	return AuItem::ListName();
}

QString AuItemState::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Environment helmets store their visor open/closed state.
	if(IsEnvironmentHelmet())
	{
		itemDesc += QObject::tr("Environment Seal <b>%1</b><br>").arg((HitPoints()>1)?"Intact":"Broken");
		itemDesc += QObject::tr("Helmet Visor <b>%1</b><br>").arg(State()==1?"Closed":"Open");
	}
	else if(IsEnvironmentSuit())
	{
		itemDesc += QObject::tr("Environment Seal <b>%1</b><br>").arg((HitPoints()>1)?"Intact":"Broken");
	}

	// Describe lights, which includes environment suit.
	if(IsLight())
	{
		float radius = round((float)LightRadius() / FeetPerMeter());
		itemDesc += QObject::tr("Light <b>%1</b><div style=\"margin-left: 10px;\"><b>"
				"%2m radius</b></div>")
			.arg(State()==1?"On":"Off")
			.arg(radius);
	}

	return itemDesc;
}

void AuItemState::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_State;
}

void AuItemState::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_State;
}

void AuItemState::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemState* c = dynamic_cast<const AuItemState*>(item);
	if(!c)
		return;

	m_State = c->m_State;
}

quint64 AuItemState::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_State);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemAmmo::AuItemAmmo(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemAmmo::AuItemAmmo()
{
}

AuItemAmmo::AuItemAmmo(const AuItemAmmo& c) : AuItem(c)
{
}

const AuItemAmmo& AuItemAmmo::operator=(const AuItemAmmo& c)
{
	AuItem::operator=(c);
	return *this;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemAmmo::Clone() const
{
	return new AuItemAmmo(*this);
}

QString AuItemAmmo::ListName() const
{
	return AuItem::ListName();
}

QString AuItemAmmo::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	QString ammotypes[] =
	{
		QObject::tr("None"),
		QObject::tr("Energy Ammunition"),
		QObject::tr("Cannon Ammunition"),
		QObject::tr("Missile Launcher Ammunition"),
		QObject::tr("Pistol Ammunition"),
		QObject::tr("Rifle Ammunition"),
		QObject::tr("Shotgun Ammunition"),
		QObject::tr("Rocket Launcher Ammunition"),
		QObject::tr("Energy Ammunition"),
		QObject::tr("Explosive"),
	};

	itemDesc += QObject::tr("<b>%1</b>").arg(ammotypes[AmmoType()]);

	itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
		.arg(DamageDescription(Effect1(), Damage1(), DamageMod1(), Radius1(), ArmorPierce1F(), StrikeThrough1()));

	if(Effect2() != WE_None)
		itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
			.arg(DamageDescription(Effect2(), Damage2(), DamageMod2(), Radius2(), ArmorPierce2F(), StrikeThrough2()));

	return itemDesc;
}

void AuItemAmmo::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
}

void AuItemAmmo::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
}

void AuItemAmmo::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	//const AuItemAmmo* c = dynamic_cast<const AuItemAmmo*>(item);
	//if(!c)
		//return;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemWeapon::AuItemWeapon(quint16 w1Range,
		WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1ap1, quint16 w1st1,
		WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1ap2, quint16 w1st2,
		quint16 w2Range,
		WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2ap1, quint16 w2st1,
		WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2ap2, quint16 w2st2,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_wpn1Range = w1Range;
	m_wpn1Effect1 = w1Effect1;
	m_wpn1Damage1 = w1Damage1;
	m_wpn1DamageMod1 = w1DamageMod1;
	m_wpn1Radius1 = w1Radius1;
	m_wpn1AP1 = w1ap1;
	m_wpn1ST1 = w1st1;
	m_wpn1Effect2 = w1Effect2;
	m_wpn1Damage2 = w1Damage2;
	m_wpn1DamageMod2 = w1DamageMod2;
	m_wpn1Radius2 = w1Radius2;
	m_wpn1AP2 = w1ap2;
	m_wpn1ST2 = w1st2;
	m_wpn2Range = w2Range;
	m_wpn2Effect1 = w2Effect1;
	m_wpn2Damage1 = w2Damage1;
	m_wpn2DamageMod1 = w2DamageMod1;
	m_wpn2Radius1 = w2Radius1;
	m_wpn2AP1 = w2ap1;
	m_wpn2ST1 = w2st1;
	m_wpn2Effect2 = w2Effect2;
	m_wpn2Damage2 = w2Damage2;
	m_wpn2DamageMod2 = w2DamageMod2;
	m_wpn2Radius2 = w2Radius2;
	m_wpn2AP2 = w2ap2;
	m_wpn2ST2 = w2st2;
}

AuItemWeapon::AuItemWeapon(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_wpn1Range = 0;
	m_wpn1Effect1 = WE_None;
	m_wpn1Damage1 = 0;
	m_wpn1DamageMod1 = 0;
	m_wpn1Radius1 = 0;
	m_wpn1AP1 = 0;
	m_wpn1ST1 = 0;
	m_wpn1Effect2 = WE_None;
	m_wpn1Damage2 = 0;
	m_wpn1DamageMod2 = 0;
	m_wpn1Radius2 = 0;
	m_wpn1AP2 = 0;
	m_wpn1ST2 = 0;
	m_wpn2Range = 0;
	m_wpn2Effect1 = WE_None;
	m_wpn2Damage1 = 0;
	m_wpn2DamageMod1 = 0;
	m_wpn2Radius1 = 0;
	m_wpn2AP1 = 0;
	m_wpn2ST1 = 0;
	m_wpn2Effect2 = WE_None;
	m_wpn2Damage2 = 0;
	m_wpn2DamageMod2 = 0;
	m_wpn2Radius2 = 0;
	m_wpn2AP2 = 0;
	m_wpn2ST2 = 0;

	// Get weapon values from commodity record.
	const DbCommodity& bCom = Commodity();

	// AuItemWeaponAmmo Notes:
	//  - wpnRange is determined by rockets and missiles when loaded into this weapon as ammo.
	//  - wpnEffect, wpnDamage, wpnDamageMod, wpnRadius are determined by ammo loaded into this weapon, including
	//    rockets/missiles.
	if(bCom.m_wpn1Effect1 != WE_None ||
			(bCom.m_wpn1Mode == WM_Projectile && bCom.m_ammo1Type != AMMO_Tube && bCom.m_ammo1Type != AMMO_Launcher))
		m_wpn1Range = QualityBoosted(bCom.m_wpn1Range);

	// Weapon 1 Effect 1
	m_wpn1Effect1 = bCom.m_wpn1Effect1;
	if(m_wpn1Effect1 != WE_None)
	{
		int wpn1Damage1 = QualityBoosted(bCom.m_wpn1Damage1);
		if(wpn1Damage1 < 1)
			m_wpn1Damage1 = 1;
		else if(wpn1Damage1 > 0xffff)
			m_wpn1Damage1 = 0xffff;
		else
			m_wpn1Damage1 = (quint16)wpn1Damage1;

		int wpn1DamageMod1 = QualityBoosted(bCom.m_wpn1DamageMod1);
		if(wpn1DamageMod1 < 0)
			m_wpn1DamageMod1 = 0;
		else if(wpn1DamageMod1 > 0xffff)
			m_wpn1DamageMod1 = 0xffff;
		else
			m_wpn1DamageMod1 = (quint16)wpn1DamageMod1;

		m_wpn1Radius1 = QualityBoosted(bCom.m_wpn1Radius1);

		m_wpn1AP1 = bCom.m_wpn1AP1;

		m_wpn1ST1 = QualityBoosted(bCom.m_wpn1ST1);
	}

	// Weapon 1 Effect 2
	m_wpn1Effect2 = bCom.m_wpn1Effect2;
	if(m_wpn1Effect2 != WE_None)
	{
		int wpn1Damage2 = QualityBoosted(bCom.m_wpn1Damage2);
		if(wpn1Damage2 < 1)
			m_wpn1Damage2 = 1;
		else if(wpn1Damage2 > 0xffff)
			m_wpn1Damage2 = 0xffff;
		else
			m_wpn1Damage2 = (quint16)wpn1Damage2;

		int wpn1DamageMod2 = QualityBoosted(bCom.m_wpn1DamageMod2);
		if(wpn1DamageMod2 < 0)
			m_wpn1DamageMod2 = 0;
		else if(wpn1DamageMod2 > 0xffff)
			m_wpn1DamageMod2 = 0xffff;
		else
			m_wpn1DamageMod2 = (quint16)wpn1DamageMod2;

		m_wpn1Radius2 = QualityBoosted(bCom.m_wpn1Radius2);

		m_wpn1AP2 = bCom.m_wpn1AP2;

		m_wpn1ST2 = QualityBoosted(bCom.m_wpn1ST2);
	}

	// AuItemWeaponAmmo Notes:
	//  - wpnRange is determined by rockets and missiles when loaded into this weapon as ammo.
	//  - wpnEffect, wpnDamage, wpnDamageMod, wpnRadius are determined by ammo loaded into this weapon, including
	//    rockets/missiles.
	if(bCom.m_wpn2Effect1 != WE_None ||
			(bCom.m_wpn2Mode == WM_Projectile && bCom.m_ammo2Type != AMMO_Tube && bCom.m_ammo2Type != AMMO_Launcher))
		m_wpn2Range = QualityBoosted(bCom.m_wpn2Range);

	// Weapon 2 Effect 1
	m_wpn2Effect1 = bCom.m_wpn2Effect1;
	if(m_wpn2Effect1 != WE_None)
	{
		int wpn2Damage1 = QualityBoosted(bCom.m_wpn2Damage1);
		if(wpn2Damage1 < 1)
			m_wpn2Damage1 = 1;
		else if(wpn2Damage1 > 0xffff)
			m_wpn2Damage1 = 0xffff;
		else
			m_wpn2Damage1 = (quint16)wpn2Damage1;

		int wpn2DamageMod1 = QualityBoosted(bCom.m_wpn2DamageMod1);
		if(wpn2DamageMod1 < 0)
			m_wpn2DamageMod1 = 0;
		else if(wpn2DamageMod1 > 0xffff)
			m_wpn2DamageMod1 = 0xffff;
		else
			m_wpn2DamageMod1 = (quint16)wpn2DamageMod1;

		m_wpn2Radius1 = QualityBoosted(bCom.m_wpn2Radius1);

		m_wpn2AP1 = bCom.m_wpn2AP1;

		m_wpn2ST1 = QualityBoosted(bCom.m_wpn2ST1);
	}

	// Weapon 2 Effect 2
	m_wpn2Effect2 = bCom.m_wpn2Effect2;
	if(m_wpn2Effect2 != WE_None)
	{
		int wpn2Damage2 = QualityBoosted(bCom.m_wpn2Damage2);
		if(wpn2Damage2 < 1)
			m_wpn2Damage2 = 1;
		else if(wpn2Damage2 > 0xffff)
			m_wpn2Damage2 = 0xffff;
		else
			m_wpn2Damage2 = (quint16)wpn2Damage2;

		int wpn2DamageMod2 = QualityBoosted(bCom.m_wpn2DamageMod2);
		if(wpn2DamageMod2 < 0)
			m_wpn2DamageMod2 = 0;
		else if(wpn2DamageMod2 > 0xffff)
			m_wpn2DamageMod2 = 0xffff;
		else
			m_wpn2DamageMod2 = (quint16)wpn2DamageMod2;

		m_wpn2Radius2 = QualityBoosted(bCom.m_wpn2Radius2);

		m_wpn2AP2 = bCom.m_wpn2AP2;

		m_wpn2ST2 = QualityBoosted(bCom.m_wpn2ST2);
	}
}

AuItemWeapon::AuItemWeapon()
{
	m_wpn1Range = 0;
	m_wpn1Effect1 = WE_None;
	m_wpn1Damage1 = 0;
	m_wpn1DamageMod1 = 0;
	m_wpn1Radius1 = 0;
	m_wpn1AP1 = 0;
	m_wpn1ST1 = 0;
	m_wpn1Effect2 = WE_None;
	m_wpn1Damage2 = 0;
	m_wpn1DamageMod2 = 0;
	m_wpn1Radius2 = 0;
	m_wpn1AP2 = 0;
	m_wpn1ST2 = 0;
	m_wpn2Range = 0;
	m_wpn2Effect1 = WE_None;
	m_wpn2Damage1 = 0;
	m_wpn2DamageMod1 = 0;
	m_wpn2Radius1 = 0;
	m_wpn2AP1 = 0;
	m_wpn2ST1 = 0;
	m_wpn2Effect2 = WE_None;
	m_wpn2Damage2 = 0;
	m_wpn2DamageMod2 = 0;
	m_wpn2Radius2 = 0;
	m_wpn2AP2 = 0;
	m_wpn2ST2 = 0;
}

AuItemWeapon::AuItemWeapon(const AuItemWeapon& c) : AuItem(c)
{
	m_wpn1Range = c.m_wpn1Range;
	m_wpn1Effect1 = c.m_wpn1Effect1;
	m_wpn1Damage1 = c.m_wpn1Damage1;
	m_wpn1DamageMod1 = c.m_wpn1DamageMod1;
	m_wpn1Radius1 = c.m_wpn1Radius1;
	m_wpn1AP1 = c.m_wpn1AP1;
	m_wpn1ST1 = c.m_wpn1ST1;
	m_wpn1Effect2 = c.m_wpn1Effect2;
	m_wpn1Damage2 = c.m_wpn1Damage2;
	m_wpn1DamageMod2 = c.m_wpn1DamageMod2;
	m_wpn1Radius2 = c.m_wpn1Radius2;
	m_wpn1AP2 = c.m_wpn1AP2;
	m_wpn1ST2 = c.m_wpn1ST2;
	m_wpn2Range = c.m_wpn2Range;
	m_wpn2Effect1 = c.m_wpn2Effect1;
	m_wpn2Damage1 = c.m_wpn2Damage1;
	m_wpn2DamageMod1 = c.m_wpn2DamageMod1;
	m_wpn2Radius1 = c.m_wpn2Radius1;
	m_wpn2AP1 = c.m_wpn2AP1;
	m_wpn2ST1 = c.m_wpn2ST1;
	m_wpn2Effect2 = c.m_wpn2Effect2;
	m_wpn2Damage2 = c.m_wpn2Damage2;
	m_wpn2DamageMod2 = c.m_wpn2DamageMod2;
	m_wpn2Radius2 = c.m_wpn2Radius2;
	m_wpn2AP2 = c.m_wpn2AP2;
	m_wpn2ST2 = c.m_wpn2ST2;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemWeapon::Clone() const
{
	return new AuItemWeapon(*this);
}

QString AuItemWeapon::ListName() const
{
	return AuItem::ListName();
}

QString AuItemWeapon::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Describe all the weapon modes of this item.
	QString modes[] =
	{
		QObject::tr("Not"),
		QObject::tr("Area"),
		QObject::tr("Contact"),
		QObject::tr("Detonator"),
		QObject::tr("Fuse"),
		QObject::tr("Projectile"),
		QObject::tr("Psionic"),
	};

	// Primary weapon is IPS_Weapon1.
	COMMODITYID cid;
	quint8 quality;
	WEAPONMODE mode;
	int ammoCur, ammoFull;
	float range;
	WEAPONEFFECT effect1, effect2;
	int damage1, damage2;
	int damageMod1, damageMod2;
	float radius1, radius2;
	float ap1, st1, ap2, st2;
	WeaponInfo(IPS_Weapon1, cid, quality, mode, range, ammoCur, ammoFull,
			effect1, damage1, damageMod1, radius1, ap1, st1, effect2, damage2, damageMod2, radius2, ap2, st2);
	if(mode != WM_Not)
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Area:
		case WM_Contact:
		case WM_Detonator:
		case WM_Fuse:
		case WM_Internal:
			itemDesc += QObject::tr("%1 Weapon 2").arg(modes[mode]);
			break;
		case WM_Projectile:
		case WM_Psionic:
			itemDesc += QObject::tr("%1 Weapon 2<div style=\"margin-left: 10px;\"><b>%2m range</b></div>")
				.arg(modes[mode])
				.arg(round(range / FeetPerMeter()));
			break;
		}

		if(effect1 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect1, damage1, damageMod1, radius1, ap1, st1));

		if(effect2 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect2, damage2, damageMod2, radius2, ap2, st2));
	}

	// Secondary weapon is IPS_Weapon2.
	WeaponInfo(IPS_Weapon2, cid, quality, mode, range, ammoCur, ammoFull,
			effect1, damage1, damageMod1, radius1, ap1, st1, effect2, damage2, damageMod2, radius2, ap2, st2);
	if(mode != WM_Not)
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Area:
		case WM_Contact:
		case WM_Detonator:
		case WM_Fuse:
		case WM_Internal:
			itemDesc += QObject::tr("%1 Weapon 3").arg(modes[mode]);
			break;
		case WM_Projectile:
		case WM_Psionic:
			itemDesc += QObject::tr("%1 Weapon 3<div style=\"margin-left: 10px;\"><b>%2m range</b></div>")
				.arg(modes[mode])
				.arg(round(range / FeetPerMeter()));
			break;
		}

		if(effect1 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect1, damage1, damageMod1, radius1, ap1, st1));

		if(effect2 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect2, damage2, damageMod2, radius2, ap2, st2));
	}

	return itemDesc;
}

void AuItemWeapon::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);

	quint8 hasWhat = 0;
	if(m_wpn1Effect1 != WE_None)
		hasWhat |= 0x01;
	if(m_wpn1Effect2 != WE_None)
		hasWhat |= 0x02;
	if(m_wpn2Effect1 != WE_None)
		hasWhat |= 0x04;
	if(m_wpn2Effect2 != WE_None)
		hasWhat |= 0x08;
	if(m_wpn2Range)
		hasWhat |= 0x10;
	hasWhat |= 0x20;		// This is our version indicator for when weapon damage mods were only stored as a byte.

	out << hasWhat;

	out << m_wpn1Range;

	if(hasWhat & 0x01)
	{
		out << m_wpn1Effect1;
		out << m_wpn1Damage1;
		out << m_wpn1DamageMod1;
		out << m_wpn1Radius1;
		out << m_wpn1AP1;
		out << m_wpn1ST1;
	}

	if(hasWhat & 0x02)
	{
		out << m_wpn1Effect2;
		out << m_wpn1Damage2;
		out << m_wpn1DamageMod2;
		out << m_wpn1Radius2;
		out << m_wpn1AP2;
		out << m_wpn1ST2;
	}

	if(hasWhat & 0x1c)
	{
		out << m_wpn2Range;

		if(hasWhat & 0x04)
		{
			out << m_wpn2Effect1;
			out << m_wpn2Damage1;
			out << m_wpn2DamageMod1;
			out << m_wpn2Radius1;
			out << m_wpn2AP1;
			out << m_wpn2ST1;
		}

		if(hasWhat & 0x08)
		{
			out << m_wpn2Effect2;
			out << m_wpn2Damage2;
			out << m_wpn2DamageMod2;
			out << m_wpn2Radius2;
			out << m_wpn2AP2;
			out << m_wpn2ST2;
		}
	}
}

void AuItemWeapon::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);

	quint8 hasWhat = 0;
	in >> hasWhat;

	in >> m_wpn1Range;

	if(hasWhat & 0x01)
	{
		in >> m_wpn1Effect1;
		in >> m_wpn1Damage1;
		if(hasWhat & 0x20)
			in >> m_wpn1DamageMod1;
		else
		{
			// Old version stored damage mod in a byte.
			quint8 ui8;
			in >> ui8;
			m_wpn1DamageMod1 = ui8;
		}
		in >> m_wpn1Radius1;
		in >> m_wpn1AP1;
		in >> m_wpn1ST1;
	}
	else
	{
		m_wpn1Effect1 = WE_None;
		m_wpn1Damage1 = 0;
		m_wpn1DamageMod1 = 0;
		m_wpn1Radius1 = 0;
		m_wpn1AP1 = 0;
		m_wpn1ST1 = 0;
	}

	if(hasWhat & 0x02)
	{
		in >> m_wpn1Effect2;
		in >> m_wpn1Damage2;
		if(hasWhat & 0x20)
			in >> m_wpn1DamageMod2;
		else
		{
			// Old version stored damage mod in a byte.
			quint8 ui8;
			in >> ui8;
			m_wpn1DamageMod2 = ui8;
		}
		in >> m_wpn1Radius2;
		in >> m_wpn1AP2;
		in >> m_wpn1ST2;
	}
	else
	{
		m_wpn1Effect2 = WE_None;
		m_wpn1Damage2 = 0;
		m_wpn1DamageMod2 = 0;
		m_wpn1Radius2 = 0;
		m_wpn1AP2 = 0;
		m_wpn1ST2 = 0;
	}

	if(hasWhat & 0x1c)
	{
		in >> m_wpn2Range;

		if(hasWhat & 0x04)
		{
			in >> m_wpn2Effect1;
			in >> m_wpn2Damage1;
			if(hasWhat & 0x20)
				in >> m_wpn2DamageMod1;
			else
			{
				// Old version stored damage mod in a byte.
				quint8 ui8;
				in >> ui8;
				m_wpn2DamageMod1 = ui8;
			}
			in >> m_wpn2Radius1;
			in >> m_wpn2AP1;
			in >> m_wpn2ST1;
		}
		else
		{
			m_wpn2Effect1 = WE_None;
			m_wpn2Damage1 = 0;
			m_wpn2DamageMod1 = 0;
			m_wpn2Radius1 = 0;
			m_wpn2AP1 = 0;
			m_wpn2ST1 = 0;
		}

		if(hasWhat & 0x08)
		{
			in >> m_wpn2Effect2;
			in >> m_wpn2Damage2;
			if(hasWhat & 0x20)
				in >> m_wpn2DamageMod1;
			else
			{
				// Old version stored damage mod in a byte.
				quint8 ui8;
				in >> ui8;
				m_wpn2DamageMod1 = ui8;
			}
			in >> m_wpn2Radius2;
			in >> m_wpn2AP2;
			in >> m_wpn2ST2;
		}
		else
		{
			m_wpn2Effect2 = WE_None;
			m_wpn2Damage2 = 0;
			m_wpn2DamageMod2 = 0;
			m_wpn2Radius2 = 0;
			m_wpn2AP2 = 0;
			m_wpn2ST2 = 0;
		}
	}
	else
	{
		m_wpn2Range = 0;
		m_wpn2Effect1 = WE_None;
		m_wpn2Damage1 = 0;
		m_wpn2DamageMod1 = 0;
		m_wpn2Radius1 = 0;
		m_wpn2AP1 = 0;
		m_wpn2ST1 = 0;
		m_wpn2Effect2 = WE_None;
		m_wpn2Damage2 = 0;
		m_wpn2DamageMod2 = 0;
		m_wpn2Radius2 = 0;
		m_wpn2AP2 = 0;
		m_wpn2ST2 = 0;
	}
}

void AuItemWeapon::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemWeapon* c = dynamic_cast<const AuItemWeapon*>(item);
	if(!c)
		return;

	m_wpn1Range = c->m_wpn1Range;
	m_wpn1Effect1 = c->m_wpn1Effect1;
	m_wpn1Damage1 = c->m_wpn1Damage1;
	m_wpn1DamageMod1 = c->m_wpn1DamageMod1;
	m_wpn1Radius1 = c->m_wpn1Radius1;
	m_wpn1AP1 = c->m_wpn1AP1;
	m_wpn1ST1 = c->m_wpn1ST1;
	m_wpn1Effect2 = c->m_wpn1Effect2;
	m_wpn1Damage2 = c->m_wpn1Damage2;
	m_wpn1DamageMod2 = c->m_wpn1DamageMod2;
	m_wpn1Radius2 = c->m_wpn1Radius2;
	m_wpn1AP2 = c->m_wpn1AP2;
	m_wpn1ST2 = c->m_wpn1ST2;
	m_wpn2Range = c->m_wpn2Range;
	m_wpn2Effect1 = c->m_wpn2Effect1;
	m_wpn2Damage1 = c->m_wpn2Damage1;
	m_wpn2DamageMod1 = c->m_wpn2DamageMod1;
	m_wpn2Radius1 = c->m_wpn2Radius1;
	m_wpn2AP1 = c->m_wpn2AP1;
	m_wpn2ST1 = c->m_wpn2ST1;
	m_wpn2Effect2 = c->m_wpn2Effect2;
	m_wpn2Damage2 = c->m_wpn2Damage2;
	m_wpn2DamageMod2 = c->m_wpn2DamageMod2;
	m_wpn2Radius2 = c->m_wpn2Radius2;
	m_wpn2AP2 = c->m_wpn2AP2;
	m_wpn2ST2 = c->m_wpn2ST2;
}

void AuItemWeapon::WeaponInfo(INVENTORYSUB sub,
		COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
		WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
		WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const
{
	const DbCommodity& bCom = Commodity();
	if(!bCom.IsWeapon(sub))
	{
		// 'sub' mode is not a weapon.
		cid = CID_Invalid;
		quality = 0;
		mode = WM_Not;
		range = 0;
		ammoCur = 0;
		ammoFull = 0;
		effect1 = WE_None;
		damage1 = 0;
		damageMod1 = 0;
		radius1 = 0;
		ap1 = 0;
		st1 = 0;
		effect2 = WE_None;
		damage2 = 0;
		damageMod2 = 0;
		radius2 = 0;
		ap2 = 0;
		st2 = 0;
	}
	else
	{
		// 'sub' mode is a weapon.
		cid = CommodityId();
		quality = Quality();
		switch(sub)
		{
		case IPS_Item:
			mode = WM_Contact;
			range = 1;
			ammoCur = 0;
			ammoFull = 0;
			if((bCom.m_combits & CB_Slash) == CB_Slash)
				effect1 = WE_Slash;
			else if(bCom.m_combits & CB_Pierce)
				effect1 = WE_Pierce;
			else
				effect1 = WE_Bludgeon;
			damage1 = QualityBoosted(bCom.m_health);
			damageMod1 = 0;
			radius1 = 0;
			ap1 = 0;
			st1 = 0;
			effect2 = WE_None;
			damage2 = 0;
			damageMod2 = 0;
			radius2 = 0;
			ap2 = 0;
			st2 = 0;
			break;
		case IPS_Weapon1:
			mode = bCom.m_wpn1Mode;
			range = m_wpn1Range;
			ammoCur = 0;
			ammoFull = 0;
			effect1 = (WEAPONEFFECT)m_wpn1Effect1;
			damage1 = m_wpn1Damage1;
			damageMod1 = m_wpn1DamageMod1;
			radius1 = m_wpn1Radius1;
			ap1 = m_wpn1AP1;
			st1 = m_wpn1ST1;
			effect2 = (WEAPONEFFECT)m_wpn1Effect2;
			damage2 = m_wpn1Damage2;
			damageMod2 = m_wpn1DamageMod2;
			radius2 = m_wpn1Radius2;
			ap2 = m_wpn1AP2;
			st2 = m_wpn1ST2;
			break;
		case IPS_Weapon2:
			mode = bCom.m_wpn2Mode;
			range = m_wpn2Range;
			ammoCur = 0;
			ammoFull = 0;
			effect1 = (WEAPONEFFECT)m_wpn2Effect1;
			damage1 = m_wpn2Damage1;
			damageMod1 = m_wpn2DamageMod1;
			radius1 = m_wpn2Radius1;
			ap1 = m_wpn2AP1;
			st1 = m_wpn2ST1;
			effect2 = (WEAPONEFFECT)m_wpn2Effect2;
			damage2 = m_wpn2Damage2;
			damageMod2 = m_wpn2DamageMod2;
			radius2 = m_wpn2Radius2;
			ap2 = m_wpn2AP2;
			st2 = m_wpn2ST2;
			break;
		}
	}
}

quint64 AuItemWeapon::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_wpn1Range);
	mem += sizeof(m_wpn1Effect1);
	mem += sizeof(m_wpn1Damage1);
	mem += sizeof(m_wpn1DamageMod1);
	mem += sizeof(m_wpn1Radius1);
	mem += sizeof(m_wpn1AP1);
	mem += sizeof(m_wpn1ST1);
	mem += sizeof(m_wpn1Effect2);
	mem += sizeof(m_wpn1Damage2);
	mem += sizeof(m_wpn1DamageMod2);
	mem += sizeof(m_wpn1Radius2);
	mem += sizeof(m_wpn1AP2);
	mem += sizeof(m_wpn1ST2);
	mem += sizeof(m_wpn2Range);
	mem += sizeof(m_wpn2Effect1);
	mem += sizeof(m_wpn2Damage1);
	mem += sizeof(m_wpn2DamageMod1);
	mem += sizeof(m_wpn2Radius1);
	mem += sizeof(m_wpn2AP1);
	mem += sizeof(m_wpn2ST1);
	mem += sizeof(m_wpn2Effect2);
	mem += sizeof(m_wpn2Damage2);
	mem += sizeof(m_wpn2DamageMod2);
	mem += sizeof(m_wpn2Radius2);
	mem += sizeof(m_wpn2AP2);
	mem += sizeof(m_wpn2ST2);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemWeaponAmmo::AuItemWeaponAmmo(quint16 w1Range,
		WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1AP1, quint16 w1ST1,
		WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1AP2, quint16 w1ST2,
		quint16 w2Range,
		WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2AP1, quint16 w2ST1,
		WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2AP2, quint16 w2ST2,
		quint8 ammo1Loaded, quint8 ammo1Quality,
		quint8 ammo2Loaded, quint8 ammo2Quality,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItemWeapon(w1Range,
			w1Effect1, w1Damage1, w1DamageMod1, w1Radius1, w1AP1, w1ST1,
			w1Effect2, w1Damage2, w1DamageMod2, w1Radius2, w1AP2, w1ST2,
			w2Range,
			w2Effect1, w2Damage1, w2DamageMod1, w2Radius1, w2AP1, w2ST1,
			w2Effect2, w2Damage2, w2DamageMod2, w2Radius2, w2AP2, w2ST2,
			cid, quality, empireOfOrigin, count)
{
	m_Ammo1Loaded = ammo1Loaded;
	m_Ammo1Quality = ammo1Quality;
	m_Ammo2Loaded = ammo2Loaded;
	m_Ammo2Quality = ammo2Quality;
}

AuItemWeaponAmmo::AuItemWeaponAmmo(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItemWeapon(cid, quality, empireOfOrigin, count)
{
	m_Ammo1Loaded = 0;
	m_Ammo1Quality = 0;
	m_Ammo2Loaded = 0;
	m_Ammo2Quality = 0;
}

AuItemWeaponAmmo::AuItemWeaponAmmo()
{
	m_Ammo1Loaded = 0;
	m_Ammo1Quality = 0;
	m_Ammo2Loaded = 0;
	m_Ammo2Quality = 0;
}

AuItemWeaponAmmo::AuItemWeaponAmmo(const AuItemWeaponAmmo& c) : AuItemWeapon(c)
{
	m_Ammo1Loaded = c.m_Ammo1Loaded;
	m_Ammo1Quality = c.m_Ammo1Quality;
	m_Ammo2Loaded = c.m_Ammo2Loaded;
	m_Ammo2Quality = c.m_Ammo2Quality;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemWeaponAmmo::Clone() const
{
	return new AuItemWeaponAmmo(*this);
}

QString AuItemWeaponAmmo::ListName() const
{
	return AuItemWeapon::ListName();
}

QString AuItemWeaponAmmo::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Describe all the weapon modes of this item.
	QString modes[] =
	{
		QObject::tr("Not"),
		QObject::tr("Area"),
		QObject::tr("Contact"),
		QObject::tr("Detonator"),
		QObject::tr("Fuse"),
		QObject::tr("Projectile"),
		QObject::tr("Psionic"),
	};

	// Primary weapon is IPS_Weapon1.
	COMMODITYID cid;
	quint8 quality;
	WEAPONMODE mode;
	int ammoCur, ammoFull;
	float range;
	WEAPONEFFECT effect1, effect2;
	int damage1, damage2;
	int damageMod1, damageMod2;
	float radius1, radius2;
	float ap1, st1, ap2, st2;
	WeaponInfo(IPS_Weapon1, cid, quality, mode, range, ammoCur, ammoFull,
			effect1, damage1, damageMod1, radius1, ap1, st1, effect2, damage2, damageMod2, radius2, ap2, st2);
	if(mode != WM_Not)
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Area:
		case WM_Contact:
		case WM_Detonator:
		case WM_Fuse:
		case WM_Internal:
			itemDesc += QObject::tr("%1 Weapon 2").arg(modes[mode]);
			break;
		case WM_Projectile:
		case WM_Psionic:
			itemDesc += QObject::tr("%1 Weapon 2<div style=\"margin-left: 10px;\"><b>%2m range</b></div>")
				.arg(modes[mode])
				.arg(round(range / FeetPerMeter()));
			break;
		}

		if(effect1 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect1, damage1, damageMod1, radius1, ap1, st1));

		if(effect2 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect2, damage2, damageMod2, radius2, ap2, st2));

		if(Ammo1Qty())
		{
			if(m_Ammo1Loaded)
				itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1 of %2 %3 Q%4 rounds</b></div>")
					.arg(m_Ammo1Loaded)
					.arg(Ammo1Qty())
					.arg(DbCommodity::QualityDesc(m_Ammo1Quality))
					.arg(m_Ammo1Quality);
			else
				itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>0 of %1 rounds</b></div>")
					.arg(Ammo1Qty());
		}
	}

	// Secondary weapon is IPS_Weapon2.
	WeaponInfo(IPS_Weapon2, cid, quality, mode, range, ammoCur, ammoFull,
			effect1, damage1, damageMod1, radius1, ap1, st1, effect2, damage2, damageMod2, radius2, ap2, st2);
	if(mode != WM_Not)
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Area:
		case WM_Contact:
		case WM_Detonator:
		case WM_Fuse:
		case WM_Internal:
			itemDesc += QObject::tr("%1 Weapon 3").arg(modes[mode]);
			break;
		case WM_Projectile:
		case WM_Psionic:
			itemDesc += QObject::tr("%1 Weapon 3<div style=\"margin-left: 10px;\"><b>%2m range</b></div>")
				.arg(modes[mode])
				.arg(round(range / FeetPerMeter()));
			break;
		}

		if(effect1 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect1, damage1, damageMod1, radius1, ap1, st1));

		if(effect2 != WE_None)
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1</b></div>")
				.arg(DamageDescription(effect2, damage2, damageMod2, radius2, ap2, st2));

		if(Ammo2Type() == AMMO_Energy1)
		{
			itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>Firing mode uses Weapon 2 ammo.</b></div>");
		}
		else if(Ammo2Qty())
		{
			if(Ammo2Loaded())
				itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>%1 of %2 %3 Q%4 rounds</b></div>")
					.arg(Ammo2Loaded())
					.arg(Ammo2Qty())
					.arg(DbCommodity::QualityDesc(m_Ammo2Quality))
					.arg(m_Ammo2Quality);
			else
				itemDesc += QObject::tr("<div style=\"margin-left: 10px;\"><b>0 of %1 rounds</b></div>")
					.arg(Ammo2Qty());
		}
	}

	return itemDesc;
}

void AuItemWeaponAmmo::operator>>(QDataStream& out) const
{
	AuItemWeapon::operator>>(out);
	out << m_Ammo1Loaded;
	out << m_Ammo1Quality;
	out << m_Ammo2Loaded;
	out << m_Ammo2Quality;
}

void AuItemWeaponAmmo::operator<<(QDataStream& in)
{
	AuItemWeapon::operator<<(in);
	in >> m_Ammo1Loaded;
	in >> m_Ammo1Quality;
	in >> m_Ammo2Loaded;
	in >> m_Ammo2Quality;
}

void AuItemWeaponAmmo::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItemWeapon::Refresh(item, re, al);

	const AuItemWeaponAmmo* c = dynamic_cast<const AuItemWeaponAmmo*>(item);
	if(!c)
		return;

	m_Ammo1Loaded = c->m_Ammo1Loaded;
	m_Ammo1Quality = c->m_Ammo1Quality;
	m_Ammo2Loaded = c->m_Ammo2Loaded;
	m_Ammo2Quality = c->m_Ammo2Quality;
}

void AuItemWeaponAmmo::WeaponInfo(INVENTORYSUB sub,
		COMMODITYID& cid, quint8& quality, WEAPONMODE& mode, float& range, int& ammoCur, int& ammoFull,
		WEAPONEFFECT& effect1, int& damage1, int& damageMod1, float& radius1, float& ap1, float& st1,
		WEAPONEFFECT& effect2, int& damage2, int& damageMod2, float& radius2, float& ap2, float& st2) const
{
	const DbCommodity& bCom = Commodity();
	if(!bCom.IsWeapon(sub))
	{
		// 'sub' mode is not a weapon.
		cid = CID_Invalid;
		quality = 0;
		mode = WM_Not;
		range = 0;
		ammoCur = 0;
		ammoFull = 0;
		effect1 = WE_None;
		damage1 = 0;
		damageMod1 = 0;
		radius1 = 0;
		ap1 = 0;
		st1 = 0;
		effect2 = WE_None;
		damage2 = 0;
		damageMod2 = 0;
		radius2 = 0;
		ap2 = 0;
		st2 = 0;
	}
	else
	{
		// 'sub' mode is a weapon.
		cid = CommodityId();
		quality = Quality();
		switch(sub)
		{
		case IPS_Item:
			mode = WM_Contact;
			range = 1;
			ammoCur = 0;
			ammoFull = 0;
			if((bCom.m_combits & CB_Slash) == CB_Slash)
				effect1 = WE_Slash;
			else if(bCom.m_combits & CB_Pierce)
				effect1 = WE_Pierce;
			else
				effect1 = WE_Bludgeon;
			damage1 = QualityBoosted(bCom.m_health);
			damageMod1 = 0;
			radius1 = 0;
			ap1 = 0;
			st1 = 0;
			effect2 = WE_None;
			damage2 = 0;
			damageMod2 = 0;
			radius2 = 0;
			ap2 = 0;
			st2 = 0;
			break;
		case IPS_Weapon1:
			mode = bCom.m_wpn1Mode;
			range = m_wpn1Range;
			ammoCur = m_Ammo1Loaded;
			ammoFull = bCom.m_ammo1Qty;
			effect1 = (WEAPONEFFECT)m_wpn1Effect1;
			damage1 = m_wpn1Damage1;
			damageMod1 = m_wpn1DamageMod1;
			radius1 = m_wpn1Radius1;
			ap1 = m_wpn1AP1;
			st1 = m_wpn1ST1;
			effect2 = (WEAPONEFFECT)m_wpn1Effect2;
			damage2 = m_wpn1Damage2;
			damageMod2 = m_wpn1DamageMod2;
			radius2 = m_wpn1Radius2;
			ap2 = m_wpn1AP2;
			st2 = m_wpn1ST2;
			break;
		case IPS_Weapon2:
			mode = bCom.m_wpn2Mode;
			range = m_wpn2Range;
			if(Ammo2Type() == AMMO_Energy1)
			{
				ammoCur= m_Ammo1Loaded;
				ammoFull = bCom.m_ammo1Qty;
			}
			else
			{
				ammoCur = m_Ammo2Loaded;
				ammoFull = bCom.m_ammo2Qty;
			}
			effect1 = (WEAPONEFFECT)m_wpn2Effect1;
			damage1 = m_wpn2Damage1;
			damageMod1 = m_wpn2DamageMod1;
			radius1 = m_wpn2Radius1;
			ap1 = m_wpn2AP1;
			st1 = m_wpn2ST1;
			effect2 = (WEAPONEFFECT)m_wpn2Effect2;
			damage2 = m_wpn2Damage2;
			damageMod2 = m_wpn2DamageMod2;
			radius2 = m_wpn2Radius2;
			ap2 = m_wpn2AP2;
			st2 = m_wpn2ST2;
			break;
		}
	}
}

bool AuItemWeaponAmmo::LoadAmmoIntoWeapon(const AuItem& item)
{
	// Energy weapon modules for turrets appear as energy ammo but they can't be loaded into a laser rifle or laser pistol.
	if(DbCommodity::IsEnergyWeapon(item.CommodityId()))
		return false;

	try
	{
		const AuItemAmmo& ammo = dynamic_cast<const AuItemAmmo&>(item);

		bool loaded = false;

		AMMOTYPE ammo1Type = Ammo1Type();
		quint8 ammo1Qty = Ammo1Qty();
		AMMOTYPE ammo2Type = Ammo2Type();
		if(ammo.AmmoType() == ammo1Type && m_Ammo1Loaded < ammo1Qty)
		{
			m_Ammo1Loaded = ammo1Qty;			// Fully load weapon.
			m_Ammo1Quality = ammo.Quality();	// Quality of last ammo loaded.

			// Missiles and rockets provide the range mod.
			if(ammo1Type == AMMO_Tube || ammo1Type == AMMO_Launcher)
				m_wpn1Range = ammo.Range();

			m_wpn1Effect1 = ammo.Effect1();
			m_wpn1Damage1 = ammo.Damage1();
			m_wpn1DamageMod1 = ammo.DamageMod1();
			m_wpn1Radius1 = ammo.Radius1();
			m_wpn1AP1 = ammo.ArmorPierce1();
			m_wpn1ST1 = ammo.StrikeThrough1();

			m_wpn1Effect2 = ammo.Effect2();
			m_wpn1Damage2 = ammo.Damage2();
			m_wpn1DamageMod2 = ammo.DamageMod2();
			m_wpn1Radius2 = ammo.Radius2();
			m_wpn1AP2 = ammo.ArmorPierce2();
			m_wpn1ST2 = ammo.StrikeThrough2();

			if(ammo1Type == AMMO_Energy)
			{
				// Energy weapons increase their damage by their own quality.
				m_wpn1Damage1 = QualityBoosted(m_wpn1Damage1);
				m_wpn1DamageMod1 = QualityBoosted(m_wpn1Damage1);
				m_wpn1Radius1 = QualityBoosted(m_wpn1Radius1);
				m_wpn1ST1 = QualityBoosted(m_wpn1ST1);

				m_wpn1Damage2 = QualityBoosted(m_wpn1Damage2);
				m_wpn1DamageMod2 = QualityBoosted(m_wpn1Damage2);
				m_wpn1Radius2 = QualityBoosted(m_wpn1Radius2);
				m_wpn1ST2 = QualityBoosted(m_wpn1ST2);

				// Reduce damage inflicted by laser pistol. It gets more shots instead. Rounds count is already in the data.
				if(CommodityId() == CID_LaserPistol)
				{
					m_wpn1Damage1 = m_wpn1Damage1 * 3 / 4;
					m_wpn1DamageMod1 = m_wpn1DamageMod1 * 3 / 4;
					m_wpn1Radius1 = m_wpn1Radius1 * 3 / 4;
					m_wpn1AP1 = m_wpn1AP1 * 3 / 4;
					m_wpn1ST1 = m_wpn1ST1 * 3 / 4;

					m_wpn1Damage2 = m_wpn1Damage2 * 3 / 4;
					m_wpn1DamageMod2 = m_wpn1DamageMod2 * 3 / 4;
					m_wpn1Radius2 = m_wpn1Radius2 * 3 / 4;
					m_wpn1AP2 = m_wpn1AP2 * 3 / 4;
					m_wpn1ST2 = m_wpn1ST2 * 3 / 4;
				}
			}

			if(ammo2Type == AMMO_Energy1)
			{
				// Weapon 2 is a firing mode that uses ammo from weapon 1.
				m_Ammo2Quality = m_Ammo1Quality;

				m_wpn2Effect1 = m_wpn1Effect1;
				m_wpn2Damage1 = m_wpn1Damage1;
				m_wpn2DamageMod1 = m_wpn1DamageMod1;
				m_wpn2Radius1 = m_wpn1Radius1;
				m_wpn2AP1 = m_wpn1AP1;
				m_wpn2ST1 = m_wpn1ST1;

				m_wpn2Effect2 = m_wpn1Effect2;
				m_wpn2Damage2 = m_wpn1Damage2;
				m_wpn2DamageMod2 = m_wpn1DamageMod2;
				m_wpn2Radius2 = m_wpn1Radius2;
				m_wpn2AP2 = m_wpn1AP2;
				m_wpn2ST2 = m_wpn1ST2;
			}

			loaded = true;
		}

		quint8 ammo2Qty = Ammo2Qty();
		if(ammo.AmmoType() == ammo2Type && m_Ammo2Loaded < ammo2Qty)
		{
			m_Ammo2Loaded = ammo2Qty;			// Fully load weapon.
			m_Ammo2Quality = ammo.Quality();	// Quality of last ammo loaded.

			// Missiles and rockets provide the range mod.
			if(ammo2Type == AMMO_Tube || ammo2Type == AMMO_Launcher)
				m_wpn2Range = ammo.Range();

			m_wpn2Effect1 = ammo.Effect1();
			m_wpn2Damage1 = ammo.Damage1();
			m_wpn2DamageMod1 = ammo.DamageMod1();
			m_wpn2Radius1 = ammo.Radius1();
			m_wpn2AP1 = ammo.ArmorPierce1();
			m_wpn2ST1 = ammo.StrikeThrough1();

			m_wpn2Effect2 = ammo.Effect2();
			m_wpn2Damage2 = ammo.Damage2();
			m_wpn2DamageMod2 = ammo.DamageMod2();
			m_wpn2Radius2 = ammo.Radius2();
			m_wpn2AP2 = ammo.ArmorPierce2();
			m_wpn2ST2 = ammo.StrikeThrough2();

			if(ammo2Type == AMMO_Energy)
			{
				// Energy weapons increase their damage by their own quality.
				m_wpn2Damage1 = QualityBoosted(m_wpn2Damage1);
				m_wpn2DamageMod1 = QualityBoosted(m_wpn2Damage1);
				m_wpn2Radius1 = QualityBoosted(m_wpn2Radius1);
				m_wpn2ST1 = QualityBoosted(m_wpn2ST1);

				m_wpn2Damage2 = QualityBoosted(m_wpn2Damage2);
				m_wpn2DamageMod2 = QualityBoosted(m_wpn2Damage2);
				m_wpn2Radius2 = QualityBoosted(m_wpn2Radius2);
				m_wpn2ST2 = QualityBoosted(m_wpn2ST2);
			}

			loaded = true;
		}

		return loaded;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

quint64 AuItemWeaponAmmo::MemoryUsage() const
{
	quint64 mem = AuItemWeapon::MemoryUsage();
	mem += sizeof(m_Ammo1Loaded);
	mem += sizeof(m_Ammo1Quality);
	mem += sizeof(m_Ammo2Loaded);
	mem += sizeof(m_Ammo2Quality);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemWeaponState::AuItemWeaponState(quint16 w1Range,
		WEAPONEFFECT w1Effect1, quint16 w1Damage1, quint16 w1DamageMod1, quint16 w1Radius1, quint8 w1AP1, quint16 w1ST1,
		WEAPONEFFECT w1Effect2, quint16 w1Damage2, quint16 w1DamageMod2, quint16 w1Radius2, quint8 w1AP2, quint16 w1ST2,
		quint16 w2Range,
		WEAPONEFFECT w2Effect1, quint16 w2Damage1, quint16 w2DamageMod1, quint16 w2Radius1, quint8 w2AP1, quint16 w2ST1,
		WEAPONEFFECT w2Effect2, quint16 w2Damage2, quint16 w2DamageMod2, quint16 w2Radius2, quint8 w2AP2, quint16 w2ST2,
		quint8 state,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItemWeapon(w1Range,
			w1Effect1, w1Damage1, w1DamageMod1, w1Radius1, w1AP1, w1ST1,
			w1Effect2, w1Damage2, w1DamageMod2, w1Radius2, w1AP2, w1ST2,
			w2Range,
			w2Effect1, w2Damage1, w2DamageMod1, w2Radius1, w2AP1, w2ST1,
			w2Effect2, w2Damage2, w2DamageMod2, w2Radius2, w2AP2, w2ST2,
			cid, quality, empireOfOrigin, count)
{
	m_State = state;
}

AuItemWeaponState::AuItemWeaponState(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) : AuItemWeapon(cid, quality, empireOfOrigin, count)
{
	m_State = 0;
}

AuItemWeaponState::AuItemWeaponState()
{
	m_State = 0;
}

AuItemWeaponState::AuItemWeaponState(const AuItemWeaponState& c) : AuItemWeapon(c)
{
	m_State = c.m_State;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemWeaponState::Clone() const
{
	return new AuItemWeaponState(*this);
}

QString AuItemWeaponState::ListName() const
{
	return AuItemWeapon::ListName();
}

QString AuItemWeaponState::HtmlDesc() const
{
	QString itemDesc = AuItemWeapon::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Describe lights.
	if(IsLight())
	{
		float radius = round((float)LightRadius() / FeetPerMeter());
		itemDesc += QObject::tr("Light <b>%1</b><div style=\"margin-left: 10px;\"><b>"
				"%2m radius</b></div>")
			.arg(State()==1?"On":"Off")
			.arg(radius);
	}

	return itemDesc;
}

void AuItemWeaponState::operator>>(QDataStream& out) const
{
	AuItemWeapon::operator>>(out);
	out << m_State;
}

void AuItemWeaponState::operator<<(QDataStream& in)
{
	AuItemWeapon::operator<<(in);
	in >> m_State;
}

void AuItemWeaponState::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemWeaponState* c = dynamic_cast<const AuItemWeaponState*>(item);
	if(!c)
		return;

	m_State = c->m_State;
}

quint64 AuItemWeaponState::MemoryUsage() const
{
	quint64 mem = AuItemWeapon::MemoryUsage();
	mem += sizeof(m_State);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemGalaxyLoc::AuItemGalaxyLoc(const AuId& galaxyId, const QString& galaxyName, const AuPoint& galaxyLoc,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_GalaxyId = galaxyId;
	m_GalaxyLoc = galaxyLoc;
	m_GalaxyName = galaxyName;
}

AuItemGalaxyLoc::AuItemGalaxyLoc(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemGalaxyLoc::AuItemGalaxyLoc()
{
}

AuItemGalaxyLoc::AuItemGalaxyLoc(const AuItemGalaxyLoc& c) : AuItem(c)
{
	m_GalaxyId = c.m_GalaxyId;
	m_GalaxyLoc = c.m_GalaxyLoc;
	m_GalaxyName = c.m_GalaxyName;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemGalaxyLoc::Clone() const
{
	return new AuItemGalaxyLoc(*this);
}

QString AuItemGalaxyLoc::ListName() const
{
	if(!m_GalaxyName.isEmpty())
		return QObject::tr("%1, %5 (%2, %3, %4)")
			.arg(AuItem::ListName())
			.arg(m_GalaxyLoc.x, 0,'f',1)
			.arg(m_GalaxyLoc.y, 0,'f',1)
			.arg(m_GalaxyLoc.z, 0,'f',1)
			.arg(m_GalaxyName);
	else
		return AuItem::ListName();
}

QString AuItemGalaxyLoc::SayName() const
{
	if(!m_GalaxyName.isEmpty())
		return QObject::tr("%1 from %2 %3 %4 in %5")
			.arg(AuItem::SayName())
			.arg(m_GalaxyLoc.x, 0,'f',1)
			.arg(m_GalaxyLoc.y, 0,'f',1)
			.arg(m_GalaxyLoc.z, 0,'f',1)
			.arg(m_GalaxyName);
	else
		return AuItem::SayName();
}

QString AuItemGalaxyLoc::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(!m_GalaxyName.isEmpty())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += QObject::tr("Origin <b>%4 (%1, %2, %3)pc</b>")
			.arg(m_GalaxyLoc.x, 0,'f',1)
			.arg(m_GalaxyLoc.y, 0,'f',1)
			.arg(m_GalaxyLoc.z, 0,'f',1)
			.arg(m_GalaxyName);
	}

	return itemDesc;
}

void AuItemGalaxyLoc::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_GalaxyId;
	out << m_GalaxyLoc;
	out << m_GalaxyName;
}

void AuItemGalaxyLoc::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_GalaxyId;
	in >> m_GalaxyLoc;
	in >> m_GalaxyName;
}

void AuItemGalaxyLoc::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemGalaxyLoc* c = dynamic_cast<const AuItemGalaxyLoc*>(item);
	if(!c)
		return;

	m_GalaxyId = c->m_GalaxyId;
	m_GalaxyLoc = c->m_GalaxyLoc;
	m_GalaxyName = c->m_GalaxyName;
}

quint64 AuItemGalaxyLoc::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_GalaxyId);
	mem += sizeof(m_GalaxyLoc);
	mem += StringMemoryUsage(m_GalaxyName);
	return mem;
}

bool AuItemGalaxyLoc::CanStack(const AuItem* item) const
{
	if(CommodityId() != item->CommodityId())
		return false;
	const AuItemGalaxyLoc* o = dynamic_cast<const AuItemGalaxyLoc*>(item);
	if(!o)
		return false;
	if(m_GalaxyId != o->m_GalaxyId)
		return false;
	if(m_GalaxyLoc != o->m_GalaxyLoc)
		return false;
	if(m_GalaxyName != o->m_GalaxyName)
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Container classes

AuItemHash::AuItemHash(const AuItemHash& c) : unordered_multimap<COMMODITYID, AuItem*, COMMODITYID_Hash>()
{
	for(const_iterator ii = c.cbegin(); ii != c.cend(); ii++)
		insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));
}

const AuItemHash& AuItemHash::operator=(const AuItemHash& c)
{
	Clear();
	for(const_iterator ii = c.cbegin(); ii != c.cend(); ii++)
		insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));
	return *this;
}

AuItemHash AuItemHash::operator-(const AuItemHash& c) const
{
	AuItemHash result(*this);
	for(const_iterator ci = c.cbegin(); ci != c.cend(); ci++)
		result.TakeItem(*(*ci).second);
	return result;
}

void AuItemHash::operator>>(QDataStream& out) const
{
	// Store the count.
	out << (quint32)size();

	// Store the items.
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		out << (*ii).second->Type();									// Store the item's class type.
		out << *(*ii).second;											// Store the item's data.
	}
}

void AuItemHash::operator<<(QDataStream& in)
{
	// Remove current contents.
	Clear();

	// Read the count.
	quint32 cnt;
	in >> cnt;

	quint8 itemType;
	AuItem* item;
	while(cnt--)
	{
		in >> itemType;											// Read the item's class type.

		item = AuItem::Create(itemType);						// Create an item of the appropriate class type.
		if(item)
		{
			in >> *item;										// Read the item's data.

			Q_ASSERT(item->IsValid());
			if(DbCommodity::IsObsolete(item->CommodityId()))
			{
				delete item;
				continue;
			}

			// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
			AuItem* replacement = item->FixItem();
			if(replacement)
			{
				delete item;
				item = replacement;
			}

			insert(pair<COMMODITYID, AuItem*>(item->CommodityId(), item));					// Insert the item into my hash.
		}
	}
}

// Export commodities.
// Returns true if any exported.
// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
bool AuItemHash::ExportBrokerItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
		POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna)
{
	if(percent <= 0 || percent > 100 || !storageLimit)
		return false;

	bool any = false;
	iterator it = begin();
	while(it != end())
	{
		// Check commerce restrictions.
		switch(exporterStance)
		{
		case POL_Empire:
		case POL_Vassal:
			break;			// Always export to members of the same empire.
		case POL_Friend:
			if(!commerce.empty() && commerce[(*it).second->CommodityId()] == EXPOL_ToNobody)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to anybody.
			}
			break;
		case POL_Neutral:
			if(!commerce.empty() && (commerce[(*it).second->CommodityId()] == EXPOL_ToNobody ||
					commerce[(*it).second->CommodityId()] == EXPOL_ToFriend))
			{
				it++;
				continue;	// Skip commodity that cannot be exported to neutrals.
			}
			break;
		case POL_Enemy:
			if(commerce.empty() || commerce[(*it).second->CommodityId()] != EXPOL_ToEnemy)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to enemies.
			}
			break;
		}

		// Broker sells pretty much everything.
		if(!(*it).second->IsBrokerItem())
		{
			it++;
			continue;
		}

		// Limit export of essential supplies in harsh environments.
		quint32 minStock;									// Minimum stock that the exporter wants to hold.
		if((*it).second->CommodityId() == CID_Air || (*it).second->CommodityId() == CID_Water)
		{
			// Air and water, because water can be used to make air at a refinery.
			if(!planetCanHaveFauna)
				minStock = exporterPopulation*13;			// Keep enough air for one city report cycle.
			else
				minStock = 0;								// Planet has abundant air.
		}
		else if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
		{
			// Food with real food nutrition value, i.e. not "diet".
			if(!planetCanHaveFlora)
				minStock = exporterPopulation*10;			// Planet cannot produce food easily.
			else
				minStock = exporterPopulation;				// Don't starve the city even in good conditions.
			minStock /= (*it).second->FoodNutritionValue();
		}
		else
			minStock = 0;

		// Determine the maximum amount of the commodity we are allowed to take.
		quint32 cntAvail = (*it).second->Count();
		if(cntAvail > minStock)
			cntAvail -= minStock;
		else
			cntAvail = 0;

		// Calculate the desired percentage of the commodity that is available for export.
		quint32 cnt = cntAvail * percent/100;
		if(!cnt && cntAvail)
			cnt = 1;	// Export at least 1.

		if(cnt)
		{
			// Limit export to max storage available.
			// Count the number in stock that have equal or better quality.
			// Those will not be replaced.
			int betterStock = a.EqualOrBetterCount(*(*it).second);
			if(betterStock >= storageLimit)
			{
				it++;
				continue;	// Space filled with equal or better qual stock.
			}
			quint32 spaceAvailable = storageLimit - betterStock;
			if(cnt > spaceAvailable)
				cnt = spaceAvailable;

			if((*it).second->Count() == cnt)
			{
				// Export the entire item.
				a.AddItemLimit(*(*it).second, storageLimit);
				any = true;
				it = Erase(it);
			}
			else
			{
				// Export a fraction of the item.
				a.AddItemCountLimit(*(*it).second, cnt, storageLimit);	// Add count of exported item.
				(*it).second->SetCount((*it).second->Count() - cnt);	// Reduce item count
				any = true;
				it++;
			}
		}
		else
			it++;
	}
	return any;
}

// Export food.
// Returns true if any exported.
// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
bool AuItemHash::ExportGroceryItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
		POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna)
{
	if(percent <= 0 || percent > 100 || !storageLimit)
		return false;

	bool any = false;
	iterator it = begin();
	while(it != end())
	{
		// Check commerce restrictions.
		switch(exporterStance)
		{
		case POL_Empire:
		case POL_Vassal:
			break;			// Always export to members of the same empire.
		case POL_Friend:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to anybody.
			}
			break;
		case POL_Neutral:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody ||
					commerce[(*it).second->CommodityId()] == EXPOL_ToFriend)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to neutrals.
			}
			break;
		case POL_Enemy:
			if(commerce[(*it).second->CommodityId()] != EXPOL_ToEnemy)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to enemies.
			}
			break;
		}

		// Grocery store sells anything that can be eaten, but no liquor.
		if(!(*it).second->IsGroceryItem())
		{
			it++;
			continue;
		}

		// Limit export of essential supplies in harsh environments.
		quint32 minStock;									// Minimum stock that the exporter wants to hold.
		if((*it).second->CommodityId() == CID_Air || (*it).second->CommodityId() == CID_Water)
		{
			// Air and water, because water can be used to make air at a refinery.
			if(!planetCanHaveFauna)
				minStock = exporterPopulation*13;			// Keep enough air for one city report cycle.
			else
				minStock = 0;								// Planet has abundant air.
		}
		else if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
		{
			// Food with real food nutrition value, i.e. not "diet".
			if(!planetCanHaveFlora)
				minStock = exporterPopulation*10;			// Planet cannot produce food easily.
			else
				minStock = exporterPopulation;				// Don't starve the city even in good conditions.
			minStock /= (*it).second->FoodNutritionValue();
		}
		else
			minStock = 0;

		// Determine the maximum amount of the commodity we are allowed to take.
		quint32 cntAvail = (*it).second->Count();
		if(cntAvail > minStock)
			cntAvail -= minStock;
		else
			cntAvail = 0;

		// Calculate the desired percentage of the commodity that is available for export.
		quint32 cnt = cntAvail * percent/100;
		if(!cnt && cntAvail)
			cnt = 1;	// Export at least 1.

		if(cnt)
		{
			// Limit export to max storage available.
			// Count the number in stock that have equal or better quality.
			// Those will not be replaced.
			int betterStock = a.EqualOrBetterCount(*(*it).second);
			if(betterStock >= storageLimit)
			{
				it++;
				continue;	// Space filled with equal or better qual stock.
			}
			quint32 spaceAvailable = storageLimit - betterStock;
			if(cnt > spaceAvailable)
				cnt = spaceAvailable;

			if((*it).second->Count() == cnt)
			{
				// Export the entire item.
				a.AddItemLimit(*(*it).second, storageLimit);
				any = true;
				it = Erase(it);
			}
			else
			{
				// Export a fraction of the item.
				a.AddItemCountLimit(*(*it).second, cnt, storageLimit);	// Add count of exported item.
				(*it).second->SetCount((*it).second->Count() - cnt);	// Reduce item count
				any = true;
				it++;
			}
		}
		else
			it++;
	}
	return any;
}

// Export non-food commodities.
// Returns true if any exported.
// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
bool AuItemHash::ExportRetailItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
		POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna)
{
	if(percent <= 0 || percent > 100 || !storageLimit)
		return false;

	bool any = false;
	iterator it = begin();
	while(it != end())
	{
		// Check commerce restrictions.
		switch(exporterStance)
		{
		case POL_Empire:
		case POL_Vassal:
			break;			// Always export to members of the same empire.
		case POL_Friend:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to anybody.
			}
			break;
		case POL_Neutral:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody ||
					commerce[(*it).second->CommodityId()] == EXPOL_ToFriend)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to neutrals.
			}
			break;
		case POL_Enemy:
			if(commerce[(*it).second->CommodityId()] != EXPOL_ToEnemy)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to enemies.
			}
			break;
		}

		// Retail store sells end-user stuff.
		if(!(*it).second->IsRetailItem())
		{
			it++;
			continue;
		}

		// Limit export of essential supplies in harsh environments.
		quint32 minStock;									// Minimum stock that the exporter wants to hold.
		if((*it).second->CommodityId() == CID_Air || (*it).second->CommodityId() == CID_Water)
		{
			// Air and water, because water can be used to make air at a refinery.
			if(!planetCanHaveFauna)
				minStock = exporterPopulation*13;			// Keep enough air for one city report cycle.
			else
				minStock = 0;								// Planet has abundant air.
		}
		else if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
		{
			// Food with real food nutrition value, i.e. not "diet".
			if(!planetCanHaveFlora)
				minStock = exporterPopulation*10;			// Planet cannot produce food easily.
			else
				minStock = exporterPopulation;				// Don't starve the city even in good conditions.
			minStock /= (*it).second->FoodNutritionValue();
		}
		else
			minStock = 0;

		// Determine the maximum amount of the commodity we are allowed to take.
		quint32 cntAvail = (*it).second->Count();
		if(cntAvail > minStock)
			cntAvail -= minStock;
		else
			cntAvail = 0;

		// Calculate the desired percentage of the commodity that is available for export.
		quint32 cnt = cntAvail * percent/100;
		if(!cnt && cntAvail)
			cnt = 1;	// Export at least 1.

		if(cnt)
		{
			// Limit export to max storage available.
			// Count the number in stock that have equal or better quality.
			// Those will not be replaced.
			int betterStock = a.EqualOrBetterCount(*(*it).second);
			if(betterStock >= storageLimit)
			{
				it++;
				continue;	// Space filled with equal or better qual stock.
			}
			quint32 spaceAvailable = storageLimit - betterStock;
			if(cnt > spaceAvailable)
				cnt = spaceAvailable;

			if((*it).second->Count() == cnt)
			{
				// Export the entire item.
				a.AddItemLimit(*(*it).second, storageLimit);
				any = true;
				it = Erase(it);
			}
			else
			{
				// Export a fraction of the item.
				a.AddItemCountLimit(*(*it).second, cnt, storageLimit);	// Add count of exported item.
				(*it).second->SetCount((*it).second->Count() - cnt);	// Reduce item count
				any = true;
				it++;
			}
		}
		else
			it++;
	}
	return any;
}

// Export food and spirits.
// Returns true if any exported.
// 'storageLimit' is total space available in receiving inventory 'a' PER COMMODITY
// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
bool AuItemHash::ExportCantinaItemsTo(qint32 exporterPopulation, AuItemHash& a, int percent, int storageLimit,
		POLITICAL_STANCE exporterStance, const QVector<quint8>& commerce, bool planetCanHaveFlora, bool planetCanHaveFauna)
{
	if(percent <= 0 || percent > 100 || !storageLimit)
		return false;

	bool any = false;
	iterator it = begin();
	while(it != end())
	{
		// Check commerce restrictions.
		switch(exporterStance)
		{
		case POL_Empire:
		case POL_Vassal:
			break;			// Always export to members of the same empire.
		case POL_Friend:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to anybody.
			}
			break;
		case POL_Neutral:
			if(commerce[(*it).second->CommodityId()] == EXPOL_ToNobody ||
					commerce[(*it).second->CommodityId()] == EXPOL_ToFriend)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to neutrals.
			}
			break;
		case POL_Enemy:
			if(commerce[(*it).second->CommodityId()] != EXPOL_ToEnemy)
			{
				it++;
				continue;	// Skip commodity that cannot be exported to enemies.
			}
			break;
		}

		// Cantinas sell food and liquor.
		if(!(*it).second->IsCantinaItem())
		{
			it++;
			continue;
		}

		// Limit export of essential supplies in harsh environments.
		quint32 minStock;									// Minimum stock that the exporter wants to hold.
		if((*it).second->CommodityId() == CID_Air || (*it).second->CommodityId() == CID_Water)
		{
			// Air and water, because water can be used to make air at a refinery.
			if(!planetCanHaveFauna)
				minStock = exporterPopulation*13;			// Keep enough air for one city report cycle.
			else
				minStock = 0;								// Planet has abundant air.
		}
		else if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
		{
			// Food with real food nutrition value, i.e. not "diet".
			if(!planetCanHaveFlora)
				minStock = exporterPopulation*10;			// Planet cannot produce food easily.
			else
				minStock = exporterPopulation;				// Don't starve the city even in good conditions.
			minStock /= (*it).second->FoodNutritionValue();
		}
		else
			minStock = 0;

		// Determine the maximum amount of the commodity we are allowed to take.
		quint32 cntAvail = (*it).second->Count();
		if(cntAvail > minStock)
			cntAvail -= minStock;
		else
			cntAvail = 0;

		// Calculate the desired percentage of the commodity that is available for export.
		quint32 cnt = cntAvail * percent/100;
		if(!cnt && cntAvail)
			cnt = 1;	// Export at least 1.

		if(cnt)
		{
			// Limit export to max storage available.
			// Count the number in stock that have equal or better quality.
			// Those will not be replaced.
			int betterStock = a.EqualOrBetterCount(*(*it).second);
			if(betterStock >= storageLimit)
			{
				it++;
				continue;	// Space filled with equal or better qual stock.
			}
			quint32 spaceAvailable = storageLimit - betterStock;
			if(cnt > spaceAvailable)
				cnt = spaceAvailable;

			if((*it).second->Count() == cnt)
			{
				// Export the entire item.
				a.AddItemLimit(*(*it).second, storageLimit);
				any = true;
				it = Erase(it);
			}
			else
			{
				// Export a fraction of the item.
				a.AddItemCountLimit(*(*it).second, cnt, storageLimit);	// Add count of exported item.
				(*it).second->SetCount((*it).second->Count() - cnt);	// Reduce item count
				any = true;
				it++;
			}
		}
		else
			it++;
	}
	return any;
}

// Export items from this inventory to a station's inventory 'stationInv'.
// Returns true if any exported.
// 'percent' is the maximum percent of each kind of commodity in this inventory to export to the station.
// 'foodSpace' is empty cargo space available for food on station.
// 'supplySpace' is empty cargo space available for supplies on station.
// 'munitionSpace' is empty cargo space available for ammunition on station.
// 'exporterPopulation' is used to decide quantity minimums in harsh environments.
// 'planetCanHaveFlora' and 'planetCanHaveFauna' describe the environment of the exporting city.
bool AuItemHash::ExportToStation(qint32 exporterPopulation, AuItemHash& stationInv, quint32 percent, quint32 foodSpace,
		quint32 supplySpace, quint32 munitionSpace, bool planetCanHaveFlora, bool planetCanHaveFauna)
{
	Q_ASSERT(percent > 0 && percent <= 100);

	// Test to see if my inventory contains any of the things that the station's inventory wants.
	quint32 foodKinds = TotalFoodUnitKinds();
	quint32 supplyKinds = TotalSupplyUnitKinds();
	quint32 munitionKinds = TotalMunitionUnitKinds();
	if( ! ((foodSpace && foodKinds) || (supplySpace && supplyKinds) || (munitionSpace && munitionKinds)))
		return false;	// This inventory has nothing the station wants.

	// Try to take a variety of food.
	quint32 foodPerKind;
	if(foodKinds)
	{
		if(foodSpace > foodKinds)
			foodPerKind = foodSpace/foodKinds;
		else
			foodPerKind = 1;
	}
	else
		foodPerKind = 0;

	// Try to take a variety of supplies.
	quint32 suppliesPerKind;
	if(supplyKinds)
	{
		if(supplySpace > supplyKinds)
			suppliesPerKind = supplySpace/supplyKinds;
		else
			suppliesPerKind = 1;
	}
	else
		suppliesPerKind = 0;

	// Try to take a variety of munitions.
	quint32 munitionsPerKind;
	if(munitionKinds)
	{
		if(munitionSpace > munitionKinds)
			munitionsPerKind = munitionSpace/munitionKinds;
		else
			munitionsPerKind = 1;
	}
	else
		munitionsPerKind = 0;

	bool any = false;
	COMMODITYID previousKind = CID_Invalid;
	iterator it = begin();
	while(it != end() && (foodSpace || supplySpace || munitionSpace))
	{
		// Only export once from each kind of commodity.
		// There may be several of different qualities but the station doesn't care.
		if((*it).second->CommodityId() == previousKind)
		{
			// Skip the item and don't forget to bump the iterator. ;)
			it++;
			continue;
		}
		previousKind = (*it).second->CommodityId();

		// Stations stock food, supplies, and munitions.
		bool skipitem = false;
		if((*it).second->IsCantinaItem() || (*it).second->IsPantryItem())
		{
			// Only export if the station has food space.
			if(!foodSpace)
				skipitem = true;
		}
		else if((*it).second->IsSupplyItem())
		{
			// Only export if the station has supply space.
			if(!supplySpace)
				skipitem = true;
		}
		else if((*it).second->IsWeaponByDesign() || (*it).second->IsAmmo())
		{
			// Only export if the station has munitions space.
			if(!munitionSpace)
				skipitem = true;
		}
		else
		{
			// Don't export this commodity to stations.
			skipitem = true;
		}

		if(skipitem)
		{
			// Skip the item and don't forget to bump the iterator. ;)
			it++;
			continue;
		}

		// Limit export of essential supplies in harsh environments.
		quint32 minStock;									// Minimum stock that the exporter wants to hold.
		if((*it).second->CommodityId() == CID_Air || (*it).second->CommodityId() == CID_Water)
		{
			// Air and water, because water can be used to make air at a refinery.
			if(!planetCanHaveFauna)
				minStock = exporterPopulation*13;			// Keep enough air for one city report cycle.
			else
				minStock = 0;								// Planet has abundant air.
		}
		else if((*it).second->IsFood() && (*it).second->FoodNutritionValue())
		{
			// Food with real food nutrition value, i.e. not "diet".
			if(!planetCanHaveFlora)
				minStock = exporterPopulation*10;			// Planet cannot produce food easily.
			else
				minStock = exporterPopulation;				// Don't starve the city even in good conditions.
			minStock /= (*it).second->FoodNutritionValue();
		}
		else
			minStock = 0;

		// Determine the maximum amount of the commodity we are allowed to take.
		quint32 cntAvail = (*it).second->Count();
		if(cntAvail > minStock)
			cntAvail -= minStock;
		else
			cntAvail = 0;

		// Calculate the desired percentage of the commodity that is available for export.
		quint32 cnt = cntAvail * percent/100;

		if(cnt)
		{
			// There is enough to take some.
			// Limit export to max count desired for variety.
			if((*it).second->IsCantinaItem() || (*it).second->IsPantryItem())
			{
				if(cnt > foodPerKind)
					cnt = foodPerKind;
				if(cnt > foodSpace)
				{
					cnt = foodSpace;
					foodSpace = 0;
				}
				else
					foodSpace -= cnt;
			}
			else if((*it).second->IsSupplyItem())
			{
				if(cnt > suppliesPerKind)
					cnt = suppliesPerKind;
				if(cnt > supplySpace)
				{
					cnt = supplySpace;
					supplySpace = 0;
				}
				else
					supplySpace -= cnt;
			}
			else
			{
				if(cnt > munitionsPerKind)
					cnt = munitionsPerKind;
				if(cnt > munitionSpace)
				{
					cnt = munitionSpace;
					munitionSpace = 0;
				}
				else
					munitionSpace -= cnt;
			}

			if((*it).second->Count() == cnt)
			{
				// Export the entire item.
				stationInv.AddItem(*(*it).second);
				any = true;
				it = Erase(it);
			}
			else if(cnt)
			{
				// Export a fraction of the item.
				(*it).second->SetCount((*it).second->Count() - cnt);	// Reduce item count

				AuItem* expIt = (*it).second->Clone();
				expIt->SetCount(cnt);
				stationInv.AddItem(*expIt);
				delete expIt;
				expIt = 0;
				any = true;
				it++;
			}
			else
				it++;
		}
		else
			it++;
	}
	return any;
}

int AuItemHash::EatMeal(unsigned int foodToEat, ECOROLE ecoRole, unsigned int liquorToDrink, unsigned int& liquorDrank)
{
	if(!foodToEat && !liquorToDrink)
	{
		liquorDrank = 0;
		return 0;
	}

	// Take inventory of the food in this inventory array that is edible by the ecological role.
	// We'll accumulate this in two arrays so we prefer to eat the "good"
	// food before the food with less nutrition value.
	AuItemHash food, secondfood, liquor;
	int totalFood = 0;		// Total food nutrition value of all food units.
	int totalLiquor = 0;	// Total intoxicant value of all liquor units.
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).second->Count() <= 0)
			continue;

		short int foodValue = (*ci).second->FoodNutritionValue(ecoRole);
		if(foodValue)
		{
			// Item is edible by the ecological role.
			totalFood += (*ci).second->Count() * foodValue;			// Accumulate total value.
			if((*ci).second->IsCantinaItem())
				food.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
			else
				secondfood.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
		}
		else if((*ci).second->IsIntoxicant())
		{
			// Item is liquor.
			totalLiquor += (*ci).second->Count() * (*ci).second->IntoxicantValue();			// Accumulate total value.
			liquor.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
		}
	}

	int consumed = 0;					// Accumulate total food nutrition value consumed for return.
	if(foodToEat && totalFood)
	{
		float fooddist = (float)foodToEat / (float)totalFood;	// Food nutrition value consumption distribution.
		if(fooddist >= 1.0)
		{
			// All food is consumed.
			for(AuItemHash::iterator fit = food.begin(); fit != food.end(); fit++)
				consumed += TakeItem(*(*fit).second) * (*fit).second->FoodNutritionValue(ecoRole);
			for(AuItemHash::iterator fit = secondfood.begin(); fit != secondfood.end(); fit++)
				consumed += TakeItem(*(*fit).second) * (*fit).second->FoodNutritionValue(ecoRole);
		}
		else
		{
			// Consume a fraction of each food type until we've reached the quantity of food nutrition value desired.
			int qtyToEat = foodToEat;
			for(AuItemHash::iterator fit = food.begin(); fit != food.end() && qtyToEat; fit++)
			{
				short int foodValue = (*fit).second->FoodNutritionValue(ecoRole);
				float totalFoodValue = (*fit).second->Count() * foodValue;
				int eat = (int)ceil(totalFoodValue * fooddist);	// food nutrition value units to eat based on distribution
				if(eat > qtyToEat)		// Do not exceed quantity desired.
					eat = qtyToEat;
				else if(eat < 1)		// At least 1.
					eat = 1;

				// calc units needed of this food to equal or exceed 'eat' food nutrition value points
				int units = (int)ceil((float)eat / (float)foodValue);

				// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
				// consumed is returned.
				int actualEaten = TakeItem(*(*fit).second, units) * foodValue;
				consumed += actualEaten;
				if(consumed > (int)foodToEat)	// Could exceed due to foodValue multiple.
					consumed = foodToEat;
				qtyToEat -= actualEaten;
				if(qtyToEat < 0)		// Could exceed due to foodValue multiple.
					qtyToEat = 0;
			}
			for(AuItemHash::iterator fit = secondfood.begin(); fit != secondfood.end() && qtyToEat; fit++)
			{
				short int foodValue = (*fit).second->FoodNutritionValue(ecoRole);
				float totalFoodValue = (*fit).second->Count() * foodValue;
				int eat = (int)ceil(totalFoodValue * fooddist);	// food nutrition value units to eat based on distribution
				if(eat > qtyToEat)		// Do not exceed quantity desired.
					eat = qtyToEat;
				else if(eat < 1)		// At least 1.
					eat = 1;

				// calc units needed of this food to equal or exceed 'eat' food nutrition value points
				int units = (int)ceil((float)eat / (float)foodValue);

				// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
				// consumed is returned.
				int actualEaten = TakeItem(*(*fit).second, units) * foodValue;
				consumed += actualEaten;
				if(consumed > (int)foodToEat)	// Could exceed due to foodValue multiple.
					consumed = foodToEat;
				qtyToEat -= actualEaten;
				if(qtyToEat < 0)		// Could exceed due to foodValue multiple.
					qtyToEat = 0;
			}
		}
	}

	int drank = 0;					// Accumulate total intoxicant value drank.
	if(liquorToDrink && totalLiquor)
	{
		float liquordist = (float)foodToEat / (float)totalLiquor;	// Intoxicant value consumption distribution.
		if(liquordist >= 1.0)
		{
			// All liquor is drank.
			for(AuItemHash::iterator fit = liquor.begin(); fit != liquor.end(); fit++)
				drank += TakeItem(*(*fit).second) * (*fit).second->IntoxicantValue();
		}
		else
		{
			// Consume a fraction of each liquor type until we've reached the quantity of intoxicant value desired.
			int qtyToDrink = liquorToDrink;
			for(AuItemHash::iterator fit = liquor.begin(); fit != liquor.end() && qtyToDrink; fit++)
			{
				short int intoxValue = (*fit).second->IntoxicantValue();
				float totalIntoxValue = (*fit).second->Count() * intoxValue;
				int drink = (int)ceil(totalIntoxValue * liquordist);	// intox value units to drink based on distribution
				if(drink > qtyToDrink)		// Do not exceed quantity desired.
					drink = qtyToDrink;
				else if(drink < 1)		// At least 1.
					drink = 1;

				// calc units needed of this liquor to equal or exceed 'drink' intox value points
				int units = (int)ceil((float)drink / (float)intoxValue);

				// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
				// consumed is returned.
				int actualDrank = TakeItem(*(*fit).second, units) * intoxValue;
				drank += actualDrank;
				if(drank > (int)liquorToDrink)	// Could exceed due to foodValue multiple.
					drank = liquorToDrink;
				qtyToDrink -= actualDrank;
				if(qtyToDrink < 0)		// Could exceed due to foodValue multiple.
					qtyToDrink = 0;
			}
		}
	}
	liquorDrank = drank;
	return consumed;
}

// Consumes food from available food resources.
// foodToEat is the amount of food nutrition value that needs to be consumed
// Returns the number of food nutrition value actually consumed.
int AuItemHash::EatFood(unsigned int foodToEat, ECOROLE ecoRole, bool& consumedLiquor)
{
	// Take inventory of the food in this inventory array that is edible by the ecological role.
	// We'll accumulate this in two arrays so we prefer to eat the "good"
	// food before the food with less food nutrition value.
	AuItemHash food, secondfood, liquor;
	int totalFood = 0;		// Total food nutrition value of all food units.
	int totalLiquor = 0;	// Total intoxicant value of all liquor units.
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).second->Count() <= 0)
			continue;

		short int foodValue = (*ci).second->FoodNutritionValue(ecoRole);
		if(foodValue)
		{
			// Item is edible by the ecological role.
			totalFood += (*ci).second->Count() * foodValue;			// Accumulate total value.
			if((*ci).second->IsCantinaItem())
				food.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
			else
				secondfood.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
		}
		else if((*ci).second->IsIntoxicant())
		{
			// Item is liquor.
			totalLiquor += (*ci).second->Count() * (*ci).second->IntoxicantValue();			// Accumulate total value.
			liquor.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
		}
	}

	int consumed = 0;					// Accumulate total food nutrition value consumed for return.
	float fooddist = (float)foodToEat / (float)totalFood;	// Food nutrition value consumption distribution.
	if(fooddist >= 1.0)
	{
		// All food is consumed.
		for(AuItemHash::iterator fit = food.begin(); fit != food.end(); fit++)
			consumed += TakeItem(*(*fit).second) * (*fit).second->FoodNutritionValue(ecoRole);
		for(AuItemHash::iterator fit = secondfood.begin(); fit != secondfood.end(); fit++)
			consumed += TakeItem(*(*fit).second) * (*fit).second->FoodNutritionValue(ecoRole);
	}
	else
	{
		// Consume a fraction of each food type until we've reached the quantity of food nutrition value desired.
		int qtyToEat = foodToEat;
		for(AuItemHash::iterator fit = food.begin(); fit != food.end() && qtyToEat; fit++)
		{
			short int foodValue = (*fit).second->FoodNutritionValue(ecoRole);
			float totalFoodValue = (*fit).second->Count() * foodValue;
			int eat = (int)ceil(totalFoodValue * fooddist);	// food nutrition value units to eat based on distribution
			if(eat > qtyToEat)		// Do not exceed quantity desired.
				eat = qtyToEat;
			else if(eat < 1)		// At least 1.
				eat = 1;

			// calc units needed of this food to equal or exceed 'eat' food nutrition value points
			int units = (int)ceil((float)eat / (float)foodValue);

			// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
			// consumed is returned.
			int actualEaten = TakeItem(*(*fit).second, units) * foodValue;
			consumed += actualEaten;
			if(consumed > (int)foodToEat)	// Could exceed due to foodValue multiple.
				consumed = foodToEat;
			qtyToEat -= actualEaten;
			if(qtyToEat < 0)		// Could exceed due to foodValue multiple.
				qtyToEat = 0;
		}
		for(AuItemHash::iterator fit = secondfood.begin(); fit != secondfood.end() && qtyToEat; fit++)
		{
			short int foodValue = (*fit).second->FoodNutritionValue(ecoRole);
			float totalFoodValue = (*fit).second->Count() * foodValue;
			int eat = (int)ceil(totalFoodValue * fooddist);	// food nutrition value units to eat based on distribution
			if(eat > qtyToEat)		// Do not exceed quantity desired.
				eat = qtyToEat;
			else if(eat < 1)		// At least 1.
				eat = 1;

			// calc units needed of this food to equal or exceed 'eat' food nutrition value points
			int units = (int)ceil((float)eat / (float)foodValue);

			// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
			// consumed is returned.
			int actualEaten = TakeItem(*(*fit).second, units) * foodValue;
			consumed += actualEaten;
			if(consumed > (int)foodToEat)	// Could exceed due to foodValue multiple.
				consumed = foodToEat;
			qtyToEat -= actualEaten;
			if(qtyToEat < 0)		// Could exceed due to foodValue multiple.
				qtyToEat = 0;
		}
	}

	int drank = 0;					// Accumulate total intoxicant value drank.
	float liquordist = (float)foodToEat / (float)totalLiquor;	// Intoxicant value consumption distribution.
	if(liquordist >= 1.0)
	{
		// All liquor is drank.
		for(AuItemHash::iterator fit = liquor.begin(); fit != liquor.end(); fit++)
			drank += TakeItem(*(*fit).second) * (*fit).second->IntoxicantValue();
	}
	else
	{
		// Consume a fraction of each liquor type until we've reached the quantity of intoxicant value desired.
		int qtyToDrink = foodToEat;
		for(AuItemHash::iterator fit = liquor.begin(); fit != liquor.end() && qtyToDrink; fit++)
		{
			short int intoxValue = (*fit).second->IntoxicantValue();
			float totalIntoxValue = (*fit).second->Count() * intoxValue;
			int drink = (int)ceil(totalIntoxValue * liquordist);	// intox value units to drink based on distribution
			if(drink > qtyToDrink)		// Do not exceed quantity desired.
				drink = qtyToDrink;
			else if(drink < 1)		// At least 1.
				drink = 1;

			// calc units needed of this liquor to equal or exceed 'drink' intox value points
			int units = (int)ceil((float)drink / (float)intoxValue);

			// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
			// consumed is returned.
			int actualDrank = TakeItem(*(*fit).second, units) * intoxValue;
			drank += actualDrank;
			if(drank > (int)foodToEat)	// Could exceed due to foodValue multiple.
				drank = foodToEat;
			qtyToDrink -= actualDrank;
			if(qtyToDrink < 0)		// Could exceed due to foodValue multiple.
				qtyToDrink = 0;
		}
	}
	if(drank)
		consumedLiquor = true;
	return consumed;
}

// Consumes food from available food resources.
// 'foodQty' is the amount of food commodity units to consume.
// 'ecoRole' determines the commodities that can be eaten as food
// return value is the amount of food commodities consumed.
int AuItemHash::EatFood(unsigned int foodQty, ECOROLE ecoRole)
{
	// Take inventory of the food in this inventory array that is edible by the ecological role.
	AuItemHash food;
	int totalFood = 0;		// Total food units.
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).second->FoodNutritionValue(ecoRole) && (*ci).second->Count() > 0)
		{
			// Item is edible by the ecological role.
			totalFood += (*ci).second->Count();						// Accumulate total units.
			food.insert(pair<COMMODITYID, AuItem*>((*ci).second->CommodityId(), (*ci).second->Clone()));
		}
	}

	int consumed = 0;					// Accumulate total food nutrition value consumed for return.
	float fooddist = (float)foodQty / (float)totalFood;	// Food nutrition value consumption distribution.
	if(fooddist >= 1.0)
	{
		// All food is consumed.
		for(AuItemHash::iterator fit = food.begin(); fit != food.end(); fit++)
			consumed += TakeItem(*(*fit).second);
	}
	else
	{
		// Consume a fraction of each food type until we've reached the quantity of food nutrition value desired.
		int qtyToEat = foodQty;
		for(AuItemHash::iterator fit = food.begin(); fit != food.end() && qtyToEat; fit++)
		{
			float totalFoodValue = (*fit).second->Count();
			int units = (int)ceil(totalFoodValue * fooddist);	// food units to eat based on distribution
			if(units > qtyToEat)		// Do not exceed quantity desired.
				units = qtyToEat;
			else if(units < 1)		// At least 1.
				units = 1;

			// Note, if units exceeds the quantity in *fit, only the quantity available will be consumed. The actual amount
			// consumed is returned.
			int actualEaten = TakeItem(*(*fit).second, units);
			consumed += actualEaten;
			if(consumed > (int)foodQty)	// Could exceed due to foodValue multiple.
				consumed = foodQty;
			qtyToEat -= actualEaten;
			if(qtyToEat < 0)		// Could exceed due to foodValue multiple.
				qtyToEat = 0;
		}
	}

	return consumed;
}

bool AuItemHash::operator==(const AuItemHash& c) const
{
	if(size() != c.size())
		return false;

	/* Side-by-side comparison does not work with multi hash because the order can differ though the contents are identical.
	for(const_iterator ii = cbegin(), ci = c.cbegin(); ii != cend() && ci != c.constEnd(); ii++, ci++)
		if(*(*ii).second != *(*ci).second)
			return false;
			*/

	// Make a duplicate of the comparison hash so we can insure 1:1 comparison of items in the presense of multiple identical items.
	AuItemHash cc(c);
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		iterator ci = cc.FindItem(*(*ii).second);
		if(ci == cc.end())
			return false;				// Corresponding identical item not found.
		cc.Erase(ci);					// Remove the matching item to insure correct results in case of multiple identical items.
	}

	return true;
}

void AuItemHash::DiscardObsoleteItems(const QString& ownerName, bool hasBreathableAtmo, ECOROLE ecoRole, quint32 storageCap)
{
	Q_UNUSED(ownerName);
	// Release build emits no messages.
	iterator ti = begin();
	while(ti != end())
		if(IsObsoleteItem(*(*ti).second, hasBreathableAtmo, ecoRole, storageCap))
		{
			// Discard 10% at every city cycle.
			quint32 itemCount = (*ti).second->Count();
			quint32 obsoleteItems = (itemCount+9)/10;
			if(itemCount <= obsoleteItems)
				ti = Erase(ti);
			else
			{
				(*ti).second->SetCount(itemCount - obsoleteItems);
				ti++;
			}
		}
		else
			ti++;
}

bool AuItemHash::IsObsoleteItem(const AuItem& item, bool hasBreathableAtmo, ECOROLE ecoRole, quint32 storageCap) const
{
	if(!hasBreathableAtmo && (item.CommodityId() == CID_Air || item.CommodityId() == CID_Water))
		return false;
	else if(item.IsFood(ecoRole))
		return false;
	/*
	else if(item.IsUpgradeModule())
	{
		// Upgrade modules are a little more stringent about obsolescence or we end up with lots of little stacks of them. Tally
		// total items of that type that are good enough to keep in the presence of the item.
		quint32 totalBetter = 0;		// Total count of inventory items that are better than the item.
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
		{
			// Inventory items of the same quality or less do not cause the item to be obsolete.
			// This skips the item itself in inv.
			if((*it).second->Quality() <= item.Quality())
				continue;				// Inventory item quality is the same or lower than item. It could be the same item.

			// The inventory item has a higher quality than the item.
			// Tally inventory items that are better than the item.
			// If the total better items equals or exceeds that of item, item is obsolete.
			totalBetter += (*it).second->Count();
			if(totalBetter > item.Count())
				return true;			// We have plenty of stuff that's better than item; it's obsolete.
		}
	}
	*/
	else
	{
		// Tally total items of that type that are good enough to keep in the presense of 'item'.
		quint32 totalBetter = 0;		// Total count of inventory items that are better than the item.
		for(const_iterator it = find(item.CommodityId()); it != cend() && (*it).first == item.CommodityId(); it++)
		{
			// Inventory items of one higher quality or less do not cause the item to be obsolete.
			// This skips the item itself in inv.
			int tldiff = (int)item.Quality() - (int)(*it).second->Quality();	// Neg. = item Q < inv Q, Pos. = item Q > inv Q.
			//  2 = item Q > inv Q by 2
			//  1 = item Q > inv Q by 1
			//  0 = item Q = inv Q
			// -1 = item Q < inv Q by 1
			// -2 = item Q < inv Q by 2
			if(tldiff >= -1)
				continue;				// Allow one quality older items to be ok.

			// The inventory item has a higher quality than the item by at least 2.
			// Tally inventory items that are better than the item.
			// If the total better items equals or exceeds half the city storage available, item is obsolete.
			totalBetter += (*it).second->Count();
			if(totalBetter > storageCap/2)
				return true;			// We have plenty of stuff that's better than item; it's obsolete.
		}
	}
	return false;
}

quint16 AuItemHash::ContainsPackUnits() const
{
	int totalUnits = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		totalUnits += (*ii).second->VolumeInPackUnits();
	if(totalUnits >= 0xffff)
		return 0xffff;
	return (quint16)totalUnits;
}

void AuItemHash::AddItems(const AuItemHash& items)
{
	for(AuItemHash::const_iterator ii = items.cbegin(); ii != items.cend(); ii++)
		AddItem(*(*ii).second);
}

void AuItemHash::AddNotRestricted(const AuItemHash& items, quint32 itemEmpire, quint32 thisEmpire)
{
	POLITICAL_STANCE stance = PoliticalStance(itemEmpire, thisEmpire);
	const DgData& gd = g_Au.LockDataForRead();
	for(AuItemHash::const_iterator ii = items.cbegin(); ii != items.cend(); ii++)
		if(!gd.EmpireCommerceRestricted(itemEmpire, (*ii).second->CommodityId(), stance))
			AddItem(*(*ii).second);
	g_Au.UnlockData();
}

quint64 AuItemHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuItemHash);
	for(AuItemHash::const_iterator ii = cbegin(); ii != cend(); ii++)
		mem += sizeof(COMMODITYID) + (*ii).second->MemoryUsage();
	return mem;
}

bool AuItemHash::HasLiquor() const
{
	for(AuItemHash::const_iterator ii = cbegin(); ii != cend(); ii++)
		if((*ii).second->IsIntoxicant())
			return true;
	return false;
}

bool AuItemHash::HasEnvironmentHelmetAndSuit() const
{
	bool hasHelm = false;
	bool hasSuit = false;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		if((*ii).second->IsEnvironmentHelmet())
		{
			if(hasSuit)
				return true;
			hasHelm = true;
		}
		else if((*ii).second->IsEnvironmentSuit())
		{
			if(hasHelm)
				return true;
			hasSuit = true;
		}
	}
	return false;
}

bool AuItemHash::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	bool anyChange = false;
	for(iterator ci = begin(); ci != end(); ci++)
		if((*ci).second->VoidOwnershipTitle(propertyId, claimantId, claimantName))
			anyChange = true;
	return anyChange;
}

double AuItemHash::Value() const
{
	double value = 0;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		value += (*ci).second->TotalCost();
	return value;
}

bool AuItemHash::ShipmentDefaulted(const AuItemTEUContainer& container)
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if(*(*ci).second == container)
		{
			((AuItemTEUContainer*)(*ci).second)->SetReceiverDefaulted();
			return true;
		}
	return false;
}

void AuItemHash::FixItems()
{
	for(iterator ci = begin(); ci != end(); ci++)
	{
		AuItem* replacement = (*ci).second->FixItem();
		if(replacement)
		{
			delete (*ci).second;
			(*ci).second = replacement;
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemContent::AuItemContent(const AuItemContent& c) : map<AuItemContentId, AuItem*>()
{
	for(const_iterator ii = c.cbegin(); ii != c.cend(); ii++)
		insert(pair<AuItemContentId, AuItem*>((*ii).first, (*ii).second->Clone()));
}

bool AuItemContent::operator==(const AuItemContent& c) const
{
	if(size() != c.size())
		return false;

	for(const_iterator ii = cbegin(), ci = c.cbegin(); ii != cend() && ci != c.cend(); ii++, ci++)
	{
		if((*ii).first != (*ci).first)
			return false;
		if(*(*ii).second != *(*ci).second)
			return false;
	}

	return true;
}

void AuItemContent::operator>>(QDataStream& out) const
{
	// Store the version.
	// Second version is 241 since the original version didn't store a version value, just the count. 241 exceeds the maximum
	// number of items in a crate, the largest container. That way we know it is our new version and not a count.
	out << (quint8)241;

	// Store the count.
	out << (quint8)size();

	// Store the items.
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		out << (*ii).first;										// Store the item's id in this container.
		out << (*ii).second->Type();							// Store the item's class type.
		out << *(*ii).second;									// Store the item's data.
	}
}

void AuItemContent::operator<<(QDataStream& in)
{
	// Remove current contents.
	Clear();

	// Read the version. It did not exist in the first go so it could be the count.
	quint8 version;
	in >> version;

	// Read the count for versions greater than 240, which is the maximum count of items in a crate.
	// The version is the count for versions less than or equal 240, which is the maximum count of items in a crate.
	quint8 cnt;
	if(version >= 241)
		in >> cnt;												// Second version (or later) sent a version and a count.
	else
		cnt = version;											// Original version is the count.

	AuItemContentId itemId;
	quint8 itemType;
	while(cnt--)
	{
		// Newer version sends the item id. Older version must generate a unique id for each item.
		if(version >= 241)
			in >> itemId;										// Read the item's id in this container.
		else
			itemId = UnusedId();								// Find an unused id for the item.

		in >> itemType;											// Read the item's class type.
		AuItem* item = AuItem::Create(itemType);				// Create an item of the appropriate class type.
		if(item)
		{
			in >> *item;										// Read the item's data.

			if(DbCommodity::IsObsolete(item->CommodityId()))
			{
				delete item;
				continue;
			}

			// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
			AuItem* replacement = item->FixItem();
			if(replacement)
			{
				delete item;
				item = replacement;
			}

			insert(pair<AuItemContentId, AuItem*>(itemId, item));	// Store the item into this container.
		}
	}
}

void AuItemContent::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	for(iterator ci = begin(); ci != end(); ci++)
		(*ci).second->DeleteRenderObjects(re, al);
}

void AuItemContent::Refresh(const AuItemContent& c, AuGLEffects* re, AuSound* al)
{
	// Do nothing if nothing inside this container changed.
	if(operator==(c))
		return;

	// Contents changed.
	if(re)
		DeleteRenderObjects(*re, al);

	// Replace myself with c.
	Clear();
	for(const_iterator ii = c.cbegin(); ii != c.cend(); ii++)
		insert(pair<AuItemContentId, AuItem*>((*ii).first, (*ii).second->Clone()));
}

void AuItemContent::ReplaceItem(const QList<AuItemContentId>& contentPath, AuItem* item)
{
	if(contentPath.isEmpty())
		return;

	iterator ii = find(contentPath.first());
	if(ii == end())
		return;

	if(contentPath.size() == 1)
	{
		delete (*ii).second;
		(*ii).second = item;
	}
	else
	{
		// The next item should be a container.
		AuItemContent* c = dynamic_cast<AuItemContent*>((*ii).second);
		if(!c)
			return;	// Error; this should be a container.

		// Remove the first item from the path and recurse into that container.
		QList<AuItemContentId> subPath(contentPath);
		subPath.pop_front();
		c->ReplaceItem(subPath, item);
	}
}

quint8 AuItemContent::ContainsPackUnits() const
{
	int totalUnits = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		totalUnits += (*ii).second->VolumeInPackUnits();
	if(totalUnits >= 255)
		return 255;
	return (quint8)totalUnits;
}

// Recursively destructs items associated with stories that are no longer active.
// Returns true if any items are destroyed.
bool AuItemContent::DestructStoryItems()
{
	//qDebug() << "AuItemContent::DestructStoryItems";
	bool anyDestroyed = false;
	iterator ci = begin();
	while(ci != end())
	{
		AuItem* invit = (*ci).second;

		// Destroy contents first in case the container becomes empty and it can be destructed.
		AuItemContent* contents = invit->Content();
		if(contents && contents->DestructStoryItems())
			anyDestroyed = true;

		if(!invit->Content() || invit->Content()->empty())
		{
			// Item in this container is not a container or it is an empty container.
			if(invit->StoryInstance())
			{
				// The item is associated with a story instance.
				//qDebug() << "AuItemContent::DestructStoryItems item" << invit->ListName() << "inStory" << invit->StoryInstance()
					//<< "active" << g_Au.IsStoryActive(invit->StoryInstance());
				if(!g_Au.IsStoryActive(invit->StoryInstance()))
				{
					ci = Erase(ci);		// Delete the item because the story instance is no longer active.
					anyDestroyed = true;
					continue;
				}
			}
		}

		ci++;
	}
	//qDebug() << "AuItemContent::DestructStoryItems anyDestroyed" << anyDestroyed;
	return anyDestroyed;
}

void AuItemContent::ListAll(const AuGearPos& parentPos, AuGearPosList& gearPos) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		// Get contained item gear pos.
		AuGearPos gpos(parentPos, (*ii).first);
		gearPos.append(gpos);

		// Get gear pos of all items in containers recursively.
		const AuItemContent* contents = (*ii).second->Content();
		if(contents)
			contents->ListAll(gpos, gearPos);
	}
}

void AuItemContent::ListItems(COMMODITYID cid, const AuGearPos& parentPos, AuGearPosList& gearPos) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		// Get contained item gear pos.
		AuGearPos gpos(parentPos, (*ii).first);
		if((*ii).second->CommodityId() == cid)
			gearPos.append(gpos);

		// Get gear pos of all items in containers recursively.
		const AuItemContent* contents = (*ii).second->Content();
		if(contents)
			contents->ListItems(cid, gpos, gearPos);
	}
}

void AuItemContent::ListPatents(AuKnownPatents& patents) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		(*ii).second->ListPatents(patents);	// Get contained item patents recursively.
}

AuGearPos AuItemContent::FindItemPos(const AuGearPos& parentPos, const AuItem& item) const
{
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		if((*ii).second->IsItem(item))
			return AuGearPos(parentPos, (*ii).first);

		const AuItemContent* contents = (*ii).second->Content();
		if(contents)
		{
			AuGearPos spos = contents->FindItemPos(AuGearPos(parentPos, (*ii).first), item);
			if(spos.IsValid())
				return spos;
		}
	}
	return AuGearPos();
}

AuGearPos AuItemContent::FindBest(const AuGearPos& parentPos, COMMODITYID cid, quint8& bestQ) const
{
	AuGearPos pos;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
	{
		if((*ii).second->CommodityId() == cid)
		{
			if(bestQ == 0 || (*ii).second->Quality() > bestQ)
			{
				pos = AuGearPos(parentPos, (*ii).first);
				bestQ = (*ii).second->Quality();
			}
		}

		const AuItemContent* contents = (*ii).second->Content();
		if(contents)
		{
			AuGearPos spos = contents->FindBest(AuGearPos(parentPos, (*ii).first), cid, bestQ);
			if(spos.IsValid())
				pos = spos;
		}
	}
	return pos;
}

bool AuItemContent::FetchConstructionMaterials(DeConstructionProcess& cp)
{
	bool anyConsumed = false;
	iterator ii = begin();
	while(ii != end())
	{
		quint32 oldcount = (*ii).second->Count();
		cp.ApplyFetchedMaterial(*(*ii).second);
		if(oldcount != (*ii).second->Count())
			anyConsumed = true;

		if((*ii).second->Count() == 0)
			ii = Erase(ii);	// The inventory item at 'index' was completely consumed.
		else
			ii++;
	}
	return anyConsumed;
}

bool AuItemContent::FetchManufacturingMaterials(DeManufacturingProcess& mp, quint8 elecSubQ, quint8 waterSubQ)
{
	bool anyConsumed = false;
	iterator ii = begin();
	while(ii != end())
	{
		quint32 oldcount = (*ii).second->Count();
		mp.ApplyFetchedMaterial(*(*ii).second, elecSubQ, waterSubQ);
		if(oldcount != (*ii).second->Count())
			anyConsumed = true;

		if((*ii).second->Count() == 0)
			ii = Erase(ii);	// The inventory item at 'index' was completely consumed.
		else
			ii++;
	}
	return anyConsumed;
}

quint64 AuItemContent::MemoryUsage() const
{
	quint64 mem = sizeof(AuItemContent);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += (*ci).second->MemoryUsage();
	return mem;
}

bool AuItemContent::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	bool anyChange = false;
	for(iterator ci = begin(); ci != end(); ci++)
		if((*ci).second->VoidOwnershipTitle(propertyId, claimantId, claimantName))
			anyChange = true;
	return anyChange;
}

QDebug operator<<(QDebug dbg, const AuItemContent& o)
{
	dbg.nospace() << "AuItemContent " << o.size() << " items\n";
	for(AuItemContent::const_iterator ci = o.cbegin(); ci != o.cend(); ci++)
	{
		dbg.nospace() << " " << (*ci).first << " " << (*ci).second->ListName()
			<< "canDrop" << (*ci).second->IsDroppableOnDeath() << "\n";
		if((*ci).second->Content())
			qDebug() << *(*ci).second->Content();
	}
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuItem subclasses
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemFloraDNA::AuItemFloraDNA(const AuFloraDNA& dna, bool dnaViable, quint32 worldGenSeed, quint8 worldResourceZone,
		quint8 plantType, COMMODITYID produceCID, AuFloraDNA::ZONE latitudeRegion,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_DNA = dna;
	m_DNADead = !dnaViable;
	m_LatitudeRegion = latitudeRegion;
	m_PlantType = plantType;
	m_PottedGenSeed = 0;
	if(CommodityId() == CID_FlowerPotLarge)
		m_PottedSize = 128;		// 50%
	else if(CommodityId() == CID_FlowerPotSmall)
		m_PottedSize = 64;		// 25%
	else
		m_PottedSize = 0;
	m_ProduceCID = produceCID;
	m_WorldResourceZone = worldResourceZone;
	m_WorldGenSeed = worldGenSeed;

	m_FloraTexture = 0;
	m_FloraVAO = 0;
}

AuItemFloraDNA::AuItemFloraDNA(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_DNADead = false;
	m_LatitudeRegion = AuFloraDNA::Polar;
	m_PlantType = 0;
	m_PottedGenSeed = 0;
	if(CommodityId() == CID_FlowerPotLarge)
		m_PottedSize = 128;		// 50%
	else if(CommodityId() == CID_FlowerPotSmall)
		m_PottedSize = 64;		// 25%
	else
		m_PottedSize = 0;
	m_ProduceCID = CID_Invalid;
	m_WorldResourceZone = 0;
	m_WorldGenSeed = 0;

	m_FloraTexture = 0;
	m_FloraVAO = 0;
}

AuItemFloraDNA::AuItemFloraDNA()
{
	m_DNADead = false;
	m_LatitudeRegion = AuFloraDNA::Polar;
	m_PlantType = 0;
	m_PottedGenSeed = 0;
	m_PottedSize = 0;
	m_ProduceCID = CID_Invalid;
	m_WorldResourceZone = 0;
	m_WorldGenSeed = 0;

	m_FloraTexture = 0;
	m_FloraVAO = 0;
}

AuItemFloraDNA::AuItemFloraDNA(const AuItemFloraDNA& c) : AuItem(c)
{
	m_DNA = c.m_DNA;
	m_DNADead = c.m_DNADead;
	m_LatitudeRegion = c.m_LatitudeRegion;
	m_PlantType = c.m_PlantType;
	m_PottedGenSeed = c.m_PottedGenSeed;
	m_PottedSize = c.m_PottedSize;
	m_ProduceCID = c.m_ProduceCID;
	m_WorldResourceZone = c.m_WorldResourceZone;
	m_WorldGenSeed = c.m_WorldGenSeed;

	m_FloraTexture = 0;
	m_FloraVAO = 0;
}

AuItemFloraDNA::~AuItemFloraDNA()
{
	if(m_FloraVAO)
		delete m_FloraVAO;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemFloraDNA::Clone() const
{
	return new AuItemFloraDNA(*this);
}

QString AuItemFloraDNA::ListName() const
{
	if(!HasDNA())
	{
		return AuItem::ListName();
	}
	else
	{
		QString plantType;
		if(m_PlantType == 0)
			plantType = QObject::tr("Ground Cover");
		else if(m_PlantType == 1)
			plantType = QObject::tr("Shrub");
		else if(m_PlantType == 2)
			plantType = QObject::tr("Tree");
		else
			plantType = QObject::tr("Moss");

		if(m_PlantType == 0)
			return QObject::tr("%1, %2")
				.arg(AuItem::ListName())							// e.g. Seed
				.arg(plantType);									// e.g. Ground Cover
		else
			return QObject::tr("%1, %2 %3")
				.arg(AuItem::ListName())							// e.g. Seed
				.arg(g_Au.CommodityName(ProduceCommodityId()))		// e.g. Apple
				.arg(plantType);									// e.g. Tree
	}
}

QString AuItemFloraDNA::SayName() const
{
	if(!HasDNA())
	{
		return AuItem::SayName();
	}
	else
	{
		QString plantType;
		if(m_PlantType == 0)
			plantType = QObject::tr("ground cover");
		else if(m_PlantType == 1)
			plantType = QObject::tr("shrub");
		else if(m_PlantType == 2)
			plantType = QObject::tr("tree");
		else
			plantType = QObject::tr("moss");

		if(m_PlantType == 0)
			return QObject::tr("%1 %2 %3")
				.arg(DbCommodity::QualityDesc(Quality()))
				.arg(plantType)
				.arg(AuItem::ListName().toLower());
		else
			return QObject::tr("%1 %2 %3 %4")
				.arg(DbCommodity::QualityDesc(Quality()))
				.arg(g_Au.CommodityName(ProduceCommodityId()).toLower())
				.arg(plantType)
				.arg(AuItem::ListName().toLower());
	}
}

QString AuItemFloraDNA::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// DNA Description
	if(HasDNA())
	{
		QString plantType;
		if(m_PlantType == 0)
			plantType = QObject::tr("Ground cover");
		else if(m_PlantType == 1)
			plantType = QObject::tr("shrub");
		else if(m_PlantType == 2)
			plantType = QObject::tr("tree");
		else
			plantType = QObject::tr("moss");

		QString dnadesc = m_DNA.Description().replace(".", ".<br>");

		QString latRegion;
		if(m_LatitudeRegion == AuFloraDNA::Polar)
			latRegion = QObject::tr("Polar");
		else if(m_LatitudeRegion == AuFloraDNA::Temperate)
			latRegion = QObject::tr("Temperate");
		else
			latRegion = QObject::tr("Tropical");

		if(m_PlantType == 0)
			itemDesc += QObject::tr("Plant<div style=\"margin-left: 10px;\"><b>%1.<br>%3%4 hardiness zone.<br>%5</b></div>")
				.arg(plantType)
				.arg(dnadesc)
				.arg(latRegion)
				.arg(m_DNADead?QObject::tr("Seed Not Viable"):QObject::tr("Seed Viable"));
		else
			itemDesc += QObject::tr("Plant<div style=\"margin-left: 10px;\"><b>%1 %2.<br>%3%4 hardiness zone.<br>%5</b></div>")
				.arg(g_Au.CommodityName(ProduceCommodityId()))
				.arg(plantType)
				.arg(dnadesc)
				.arg(latRegion)
				.arg(m_DNADead?QObject::tr("Seed Not Viable"):QObject::tr("Seed Viable"));
	}
	else
		itemDesc += QObject::tr("Plant<div style=\"margin-left: 10px;\"><b>Not planted.</b></div>");

	return itemDesc;
}

void AuItemFloraDNA::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);

	quint8 plantTypeVer = 0x80 | m_PlantType;	// Top bit is version
	if(m_DNADead)
		plantTypeVer |= 0x08;

	out << m_DNA;
	out << m_LatitudeRegion;
	out << plantTypeVer;
	out << m_PottedGenSeed;
	out << m_PottedSize;
	out << m_ProduceCID;
	out << m_WorldResourceZone;
	out << m_WorldGenSeed;
}

void AuItemFloraDNA::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);

	quint8 plantTypeVer;

	in >> m_DNA;
	in >> m_LatitudeRegion;
	in >> plantTypeVer;
	m_PlantType = plantTypeVer & 0x03;
	m_DNADead = plantTypeVer & 0x08;
	in >> m_PottedGenSeed;

	// Set default potted size.
	if(CommodityId() == CID_FlowerPotLarge)
		m_PottedSize = 128;		// 50%
	else if(CommodityId() == CID_FlowerPotSmall)
		m_PottedSize = 64;		// 25%
	else
		m_PottedSize = 0;

	if(plantTypeVer & 0x80)			// Top bit is version.
		in >> m_PottedSize;

	in >> m_ProduceCID;
	in >> m_WorldResourceZone;
	in >> m_WorldGenSeed;
}

void AuItemFloraDNA::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemFloraDNA* c = dynamic_cast<const AuItemFloraDNA*>(item);
	if(!c)
		return;

	// We don't change anything on an existing flora DNA object that would change the OpenGL data. The sizing is done with scaling.
	m_DNA = c->m_DNA;
	m_DNADead = c->m_DNADead;
	m_LatitudeRegion = c->m_LatitudeRegion;
	m_PlantType = c->m_PlantType;
	m_PottedGenSeed = c->m_PottedGenSeed;
	m_PottedSize = c->m_PottedSize;
	m_ProduceCID = c->m_ProduceCID;
	m_WorldResourceZone = c->m_WorldResourceZone;
	m_WorldGenSeed = c->m_WorldGenSeed;
}

bool AuItemFloraDNA::PlantSeedInPot(const AuItem& item)
{
	// Energy weapon modules for turrets appear as energy ammo but they can't be loaded into a laser rifle or laser pistol.
	if(item.CommodityId() != CID_Seed)
		return false;		// The incoming item is not a seed.
	if(CommodityId() == CID_Seed)
		return false;		// This item is a seed.

	try
	{
		const AuItemFloraDNA& seed = dynamic_cast<const AuItemFloraDNA&>(item);
		m_PottedGenSeed = seed.WorldGenSeed() + g_Au.Dice().Roll(1,100000000);
		if(CommodityId() == CID_FlowerPotLarge)
			m_PottedSize = 128;		// 50%
		else
			m_PottedSize = 64;		// 25%
		SetDNA(seed.FloraDNA(), seed.DNAIsViable(), seed.WorldGenSeed(), seed.WorldResourceZone(), seed.PlantType(),
				seed.ProduceCommodityId(), seed.LatitudeRegion());
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuItemFloraDNA::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	AuItem::DeleteRenderObjects(re, al);
	if(m_FloraVAO)
	{
		if(m_FloraVAO->VAO())
			m_FloraVAO->DeleteGL();
		delete m_FloraVAO;
		m_FloraVAO = 0;
	}
}

double AuItemFloraDNA::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		const AuPoint& windVec, float scale, int pass, double effectScale)
{
	// Render the base flower pot.
	AuItem::Render(projection, model, re, mc, id, ambientR,ambientG,ambientB, ambientUp, gravityUp, windVec, scale, pass,
			effectScale);

	if(!DbCommodity::IsFlowerPot(CommodityId()))
		return 0;

	if(pass > 1)
		return 0;

	if(!HasDNA())
		return 0;

	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z >= 0)
	{
		// Flower pot checks a point above the pot so the plant doesn't disappear when the pot is not in the scene.
		AuPoint pv = model.Project(AuPoint(0,0,4));
		if(pv.z >= 0)
			return 0;
	}

	if(!m_FloraVAO)
		CreateFloraVAO(re, id);

	if(m_FloraVAO)
	{
		// Render the plant inside the pot.
		AuMatrix4x4 plantM;
		if(CommodityId() == CID_FlowerPotLarge)
			plantM.Translate(0,0,1.25);	// Measured off the model.
		else
			plantM.Translate(0,0,.64);	// Measured off the model.
		float pottedScale = (float)PottedSize() / 255.f;
		plantM.Scale(pottedScale, pottedScale, pottedScale);

		// Render the plant with spacecraft lighting.
		re.GLSL().FloraShader().Select(projection, re);
		re.GLSL().FloraShader().SetModelView(model);
		re.GLSL().FloraShader().SetFog(0, 1, 1, 1);
		re.GLSL().FloraShader().SetTexture(m_FloraTexture);
		re.GLSL().FloraShader().SetPlantMatrix(plantM);
		AuPoint pottedWindVec = windVec * (pottedScale * 2);
		re.GLSL().FloraShader().SetForceVector(pottedWindVec.x, pottedWindVec.y, pottedWindVec.z);	// In eye space.

		// Globe position in eye space is needed to calculate sun over-the-horizon effect.
		re.GLSL().FloraShader().SetGlobeLocation(0, 0, 0);

		// Ambient light
		re.GLSL().FloraShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().FloraShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Configure glow.
		QColor glowColor = FloraDNA().Glow();
		re.GLSL().FloraShader().SetGlowAlpha(glowColor.redF(), glowColor.greenF(), glowColor.blueF(), 1.f);

		Q_ASSERT(m_FloraVAO->VAO());

		m_FloraVAO->SelectGL();
		m_FloraVAO->DrawGL();
		m_FloraVAO->DeselectGL();

		re.GLSL().FloraShader().Deselect();
	}

	return 0;
}

double AuItemFloraDNA::RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		const AuPoint& globeP,
		const AuPoint& windVec, float scale, int pass)
{
	// Render the base flower pot.
	AuItem::RenderOnGlobe(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, fogDensity, fogR, fogG,
			fogB, globeP, windVec, scale, pass);

	if(!DbCommodity::IsFlowerPot(CommodityId()))
		return 0;

	if(pass > 1)
		return 0;

	if(!HasDNA())
		return 0;

	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z >= 0)
	{
		// Flower pot checks a point above the pot so the plant doesn't disappear when the pot is not in the scene.
		AuPoint pv = model.Project(AuPoint(0,0,4));
		if(pv.z >= 0)
			return 0;
	}

	if(!m_FloraVAO)
		CreateFloraVAO(re, id);

	if(m_FloraVAO)
	{
		// Render the plant inside the pot.
		AuMatrix4x4 plantM;
		if(CommodityId() == CID_FlowerPotLarge)
			plantM.Translate(0,0,1.25);	// Measured off the model.
		else
			plantM.Translate(0,0,.64);	// Measured off the model.
		float pottedScale = (float)PottedSize() / 255.f;
		plantM.Scale(pottedScale, pottedScale, pottedScale);

		// Render the plant with globe lighting.
		re.GLSL().FloraShader().Select(projection, re);
		re.GLSL().FloraShader().SetModelView(model);
		re.GLSL().FloraShader().SetFog(fogDensity, fogR, fogG, fogB);
		re.GLSL().FloraShader().SetTexture(m_FloraTexture);
		re.GLSL().FloraShader().SetPlantMatrix(plantM);
		AuPoint pottedWindVec = windVec * (pottedScale * 2);
		re.GLSL().FloraShader().SetForceVector(pottedWindVec.x, pottedWindVec.y, pottedWindVec.z);	// In eye space.

		// Globe position in eye space is needed to calculate sun over-the-horizon effect.
		re.GLSL().FloraShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

		// Ambient light
		re.GLSL().FloraShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().FloraShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Configure glow.
		QColor glowColor = FloraDNA().Glow();
		re.GLSL().FloraShader().SetGlowAlpha(glowColor.redF(), glowColor.greenF(), glowColor.blueF(), 1.f);

		Q_ASSERT(m_FloraVAO->VAO());

		m_FloraVAO->SelectGL();
		m_FloraVAO->DrawGL();
		m_FloraVAO->DeselectGL();

		re.GLSL().FloraShader().Deselect();
	}

	return 0;
}

double AuItemFloraDNA::RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		AuGLModelCache& mc, const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp,
		const AuPoint& gravityUp, float fogDensity, float fogR, float fogG, float fogB,
		float shadowPanelSunAngles[MAX_SUNS],
		const AuPoint& windVec, float scale, int pass)
{
	// Render the base flower pot.
	AuItem::RenderOnRingworld(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, fogDensity, fogR,
			fogG, fogB, shadowPanelSunAngles, windVec, scale, pass);

	if(!DbCommodity::IsFlowerPot(CommodityId()))
		return 0;

	if(pass > 1)
		return 0;

	if(!HasDNA())
		return 0;

	AuPoint pv = model.Project(AuPoint(0,0));
	if(pv.z >= 0)
	{
		// Flower pot checks a point above the pot so the plant doesn't disappear when the pot is not in the scene.
		AuPoint pv = model.Project(AuPoint(0,0,4));
		if(pv.z >= 0)
			return 0;
	}

	if(!m_FloraVAO)
		CreateFloraVAO(re, id);

	if(m_FloraVAO)
	{
		// Render the plant inside the pot.
		AuMatrix4x4 plantM;
		if(CommodityId() == CID_FlowerPotLarge)
			plantM.Translate(0,0,1.25);	// Measured off the model.
		else
			plantM.Translate(0,0,.64);	// Measured off the model.
		float pottedScale = (float)PottedSize() / 255.f;
		plantM.Scale(pottedScale, pottedScale, pottedScale);

		// Render the plant with ringworld lighting.
		re.GLSL().FloraRingworldShader().Select(projection, re);
		re.GLSL().FloraRingworldShader().SetModelView(model);
		re.GLSL().FloraRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);
		re.GLSL().FloraRingworldShader().SetTexture(m_FloraTexture);
		re.GLSL().FloraRingworldShader().SetPlantMatrix(plantM);
		AuPoint pottedWindVec = windVec * (pottedScale * 2);
		re.GLSL().FloraRingworldShader().SetForceVector(pottedWindVec.x, pottedWindVec.y, pottedWindVec.z);	// In eye space.

		// Ambient light
		re.GLSL().FloraRingworldShader().SetAmbientLight(ambientR, ambientG, ambientB);
		re.GLSL().FloraRingworldShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Suns, eye space lighting.
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			re.GLSL().FloraRingworldShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

		// Configure glow.
		QColor glowColor = FloraDNA().Glow();
		re.GLSL().FloraRingworldShader().SetGlowAlpha(glowColor.redF(), glowColor.greenF(),
				glowColor.blueF(), 1.);

		Q_ASSERT(m_FloraVAO->VAO());

		m_FloraVAO->SelectGL();
		m_FloraVAO->DrawGL();
		m_FloraVAO->DeselectGL();

		re.GLSL().FloraRingworldShader().Deselect();
	}

	return 0;
}

void AuItemFloraDNA::CreateFloraVAO(AuGLEffects& re, const AuId& id)
{
	// Render the plant inside the pot.
	// Create the VAO and texture needed to render the plant.
	switch(PlantType())
	{
	case 0:	// Ground Cover
		{
			// Prepare to create a model of the ground cover.
			m_FloraVAO = new AuGLVAO;
			DePlanetFlora flora;
			flora.SetFloraDNA(LatitudeRegion(), PlantType(), ProduceCommodityId(), FloraDNA());

			// Create the texture. This must be done before the model is created.
			flora.CreateDeviceObjects(re, id, WorldGenSeed(), WorldResourceZone(), (AuFloraDNA::ZONE)LatitudeRegion(), PlantType(),
					ProduceCommodityId());
			m_FloraTexture = flora.Texture();

			// Create the VAO.
			float maxRadius;
			float pottedScale = (float)PottedSize() / 255.f;
			if(CommodityId() == CID_FlowerPotLarge)
				maxRadius = (1.f - .083) / pottedScale;	// Large pot is 1' radius.
			else
				maxRadius = (.5f - .083) / pottedScale;	// Small pot is 6" radius.
			AuDice dice;
			dice.Seed(PottedGenSeed());
			flora.CreateGroundCoverVAO(re, *m_FloraVAO, LatitudeRegion(), dice, 2, maxRadius);
		}
		break;

	case 1:	// Shrub
		{
			// Prepare to create a model of the shrub.
			m_FloraVAO = new AuGLVAO;
			DePlanetFlora flora;
			flora.SetFloraDNA(LatitudeRegion(), PlantType(), ProduceCommodityId(), FloraDNA());

			// Create the texture. This must be done before the model is created.
			flora.CreateDeviceObjects(re, id, WorldGenSeed(), WorldResourceZone(), (AuFloraDNA::ZONE)LatitudeRegion(), PlantType(),
					ProduceCommodityId());
			m_FloraTexture = flora.Texture();

			// Create the VAO.
			float maxRadius;
			float pottedScale = (float)PottedSize() / 255.f;
			if(CommodityId() == CID_FlowerPotLarge)
				maxRadius = (1.f - .083) / pottedScale;	// Large pot is 1' radius.
			else
				maxRadius = (.5f - .083) / pottedScale;	// Small pot is 6" radius.
			AuDice dice;
			dice.Seed(PottedGenSeed());
			flora.CreateShrubVAO(re, *m_FloraVAO, LatitudeRegion(), ProduceCommodityId(), dice, 2, maxRadius);
		}
		break;

	case 2:	// Tree
		{
			// Prepare to create a model of the tree.
			m_FloraVAO = new AuGLVAO;
			DePlanetFlora flora;
			flora.SetFloraDNA(LatitudeRegion(), PlantType(), ProduceCommodityId(), FloraDNA());

			// Create the texture. This must be done before the model is created.
			flora.CreateDeviceObjects(re, id, WorldGenSeed(), WorldResourceZone(), (AuFloraDNA::ZONE)LatitudeRegion(), PlantType(),
					ProduceCommodityId());
			m_FloraTexture = flora.Texture();

			// Create the VAO.
			float maxRadius;
			float pottedScale = (float)PottedSize() / 255.f;
			if(CommodityId() == CID_FlowerPotLarge)
				maxRadius = (1.f - .083) / pottedScale;	// Large pot is 1' radius.
			else
				maxRadius = (.5f - .083) / pottedScale;	// Small pot is 6" radius.
			AuDice dice;
			dice.Seed(PottedGenSeed());
			flora.CreateTreeVAO(re, *m_FloraVAO, LatitudeRegion(), ProduceCommodityId(), dice, 2, maxRadius);
		}
		break;
	}
}

quint64 AuItemFloraDNA::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_DNA);
	mem += sizeof(m_DNADead);
	mem += sizeof(m_LatitudeRegion);
	mem += sizeof(m_PlantType);
	mem += sizeof(m_PottedGenSeed);
	mem += sizeof(m_PottedSize);
	mem += sizeof(m_ProduceCID);
	mem += sizeof(m_WorldResourceZone);
	mem += sizeof(m_WorldGenSeed);

	mem += sizeof(m_FloraTexture);
	mem += sizeof(m_FloraVAO);
	return mem;
}

bool AuItemFloraDNA::CanStack(const AuItem* item) const
{
	if(CommodityId() != item->CommodityId())
		return false;
	const AuItemFloraDNA* o = dynamic_cast<const AuItemFloraDNA*>(item);
	if(!o)
		return false;
	if(m_DNA != o->m_DNA)
		return false;
	if(m_DNADead != o->m_DNADead)
		return false;
	if(m_LatitudeRegion != o->m_LatitudeRegion)
		return false;
	if(m_PlantType != o->m_PlantType)
		return false;
	if(m_PottedGenSeed != o->m_PottedGenSeed)
		return false;
	if(m_PottedSize != o->m_PottedSize)
		return false;
	if(m_ProduceCID != o->m_ProduceCID)
		return false;
	if(m_WorldResourceZone != o->m_WorldResourceZone)
		return false;
	if(m_WorldGenSeed != o->m_WorldGenSeed)
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemPicture::AuItemPicture(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_HideURL = false;
	m_RotatePicture = false;

	m_PictureTexture = 0;
	m_PictureVAO = 0;
}

AuItemPicture::AuItemPicture()
{
	m_HideURL = false;
	m_RotatePicture = false;

	m_PictureTexture = 0;
	m_PictureVAO = 0;
}

AuItemPicture::~AuItemPicture()
{
	if(m_PictureVAO)
		delete m_PictureVAO;
}

AuItemPicture::AuItemPicture(const AuItemPicture& c) : AuItem(c)
{
	m_HideURL = c.m_HideURL;
	m_PictureURL = c.m_PictureURL;
	m_RotatePicture = c.m_RotatePicture;

	m_PictureTexture = 0;
	m_PictureVAO = 0;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemPicture::Clone() const
{
	return new AuItemPicture(*this);
}

QString AuItemPicture::ListName() const
{
	return AuItem::ListName();
}

QString AuItemPicture::SayName() const
{
	return AuItem::SayName();
}

QString AuItemPicture::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

#if defined(QT_NO_DEBUG)
	if(!m_HideURL)
		itemDesc += QObject::tr("Picture URL <b>%1</b>")
			.arg(m_PictureURL);
	else
		itemDesc += QObject::tr("Picture URL <b>Hidden</b>");
#else
	if(!m_HideURL)
		itemDesc += QObject::tr("Picture URL <b>%1</b>")
			.arg(m_PictureURL);
	else
		itemDesc += QObject::tr("Picture URL <b>Hidden (shh! %1)</b>")
			.arg(m_PictureURL);
#endif

	return itemDesc;
}

void AuItemPicture::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);

	out << m_PictureURL;

	quint8 ui8 = 0;
	if(m_RotatePicture)
		ui8 |= 0x01;
	if(m_HideURL)
		ui8 |= 0x02;
	out << ui8;
}

void AuItemPicture::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);

	quint8 ui8;

	in >> m_PictureURL;
	in >> ui8;
	m_RotatePicture = ui8 & 0x01;
	m_HideURL = ui8 & 0x02;
}

void AuItemPicture::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemPicture* c = dynamic_cast<const AuItemPicture*>(item);
	if(!c)
		return;

	if(re && m_PictureURL != c->m_PictureURL)
		DeleteRenderObjects(*re, al);

	m_HideURL = c->m_HideURL;
	m_PictureURL = c->m_PictureURL;
	m_RotatePicture = c->m_RotatePicture;
}

void AuItemPicture::DeleteRenderObjects(AuGLEffects& re, AuSound* al)
{
	AuItem::DeleteRenderObjects(re, al);
	if(m_PictureVAO)
	{
		if(m_PictureVAO->VAO())
			m_PictureVAO->DeleteGL();
		delete m_PictureVAO;
		m_PictureVAO = 0;
	}

	m_PictureTexture = 0;
}

double AuItemPicture::Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		const AuPoint& windVec, float scale, int pass, double effectScale)
{
	// Render the base picture frame model.
	AuItem::Render(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, windVec, scale, pass,
			effectScale);

	if(pass > 1)
		return 0;

	RenderPicture(projection, model, re, id);

	return 0;
}

double AuItemPicture::RenderOnGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelCache& mc,
		const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp, const AuPoint& gravityUp,
		float fogDensity, float fogR, float fogG, float fogB,
		const AuPoint& globeP,
		const AuPoint& windVec, float scale, int pass)
{
	// Render the base picture frame model.
	AuItem::RenderOnGlobe(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, fogDensity, fogR, fogG,
			fogB, globeP, windVec, scale, pass);

	if(pass > 1)
		return 0;

	RenderPicture(projection, model, re, id);

	return 0;
}

double AuItemPicture::RenderOnRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		AuGLModelCache& mc, const AuId& id, float ambientR, float ambientG, float ambientB, const AuPoint& ambientUp,
		const AuPoint& gravityUp, float fogDensity, float fogR, float fogG, float fogB,
		float shadowPanelSunAngles[MAX_SUNS],
		const AuPoint& windVec, float scale, int pass)
{
	// Render the base picture frame model.
	AuItem::RenderOnRingworld(projection, model, re, mc, id, ambientR, ambientG, ambientB, ambientUp, gravityUp, fogDensity, fogR,
			fogG, fogB, shadowPanelSunAngles, windVec, scale, pass);

	if(pass > 1)
		return 0;

	RenderPicture(projection, model, re, id);

	return 0;
}

void AuItemPicture::RenderPicture(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuId& id)
{
	// Create device objects.
	{
		if(!m_PictureVAO)
		{
			// Create a simple rectangle VAO on which to display the picture. The coordinates were taken off the
			// model of the picture frame.
			QList<AuTriTex> tris;
			double w2, h;		// 1/2 the width and the height of the picture area
			switch(CommodityId())
			{
			case CID_PictureFrame120x68:
				w2 = 2;
				h = 2.25;
				break;
			case CID_PictureFrame22x28:
			default:
				w2 = .354;
				h = .917;
				break;
			case CID_PictureFrame60x34:
				w2 = 1;
				h = 1.125;
				break;
			}
			if(RotatePicture())
			{
				AuTriTex tri0;
				tri0.m_V0 = AuPointTex(-w2, -.01, -h, 1,0);
				tri0.m_V1 = AuPointTex(-w2, -.01, 0, 0,0);
				tri0.m_V2 = AuPointTex(w2, -.01, 0, 0,1);
				AuTriTex tri1;
				tri1.m_V0 = AuPointTex(w2, -.01, 0, 0,1);
				tri1.m_V1 = AuPointTex(w2, -.01, -h, 1,1);
				tri1.m_V2 = AuPointTex(-w2, -.01, -h, 1,0);
				tris << tri0 << tri1;
			}
			else
			{
				AuTriTex tri0;
				tri0.m_V0 = AuPointTex(-w2, -.01, -h, 0,0);
				tri0.m_V1 = AuPointTex(-w2, -.01, 0, 0,1);
				tri0.m_V2 = AuPointTex(w2, -.01, 0, 1,1);
				AuTriTex tri1;
				tri1.m_V0 = AuPointTex(w2, -.01, 0, 1,1);
				tri1.m_V1 = AuPointTex(w2, -.01, -h, 1,0);
				tri1.m_V2 = AuPointTex(-w2, -.01, -h, 0,0);
				tris << tri0 << tri1;
			}

			m_PictureVAO = new AuGLVAO;
			m_PictureVAO->CreateForTexture(re, tris, GL_STATIC_DRAW);
		}

		if(g_Au.DrawFramedPictures() && !m_PictureTexture && !PictureURL().isEmpty())
		{
			// Load the picture that was downloaded, once it finally arrives.
			QImage imtex(g_Au.DownloadImage(PictureURL()));
			if(!imtex.isNull())
			{
				// Resize image to an acceptable maximum?
				int maxDim = g_Au.FramedPictureSize();
				if(imtex.height() > maxDim || imtex.width() > maxDim)
				{
					if(imtex.height() > imtex.width())
						imtex = imtex.scaledToHeight(maxDim, Qt::SmoothTransformation);
					else
						imtex = imtex.scaledToWidth(maxDim, Qt::SmoothTransformation);
				}

				// Create a texture from the image.
				//qDebug() << "AuItemPicture::RenderPicture" << id << "URL" << PictureURL() << "img" << imtex.size();
				m_PictureTexture = re.CreateTextureFromBitmap(id.GetID(), PictureURL(), imtex, GL_LINEAR, GL_LINEAR, true);
			}
		}
	}

	// Render the picture.
	if(m_PictureVAO)
	{
		re.GLSL().TextureShader().Select();
		re.GLSL().TextureShader().SetProjectionModelView(projection * model);
		if(g_Au.DrawFramedPictures() && m_PictureTexture)
			re.GLSL().TextureShader().SetTexture(m_PictureTexture);
		else
		{
			GLuint tex = re.CreateTextureFromBitmap(id.GetID(), "EmptyPictureFrame", ":data/hazBlackbutton.png");
			re.GLSL().TextureShader().SetTexture(tex);
		}
		re.GLSL().TextureShader().SetColor(1,1,1,1);

		re.glDisable(GL_CULL_FACE);

		// Render the model.
		m_PictureVAO->SelectGL();
		m_PictureVAO->DrawGL();
		m_PictureVAO->DeselectGL();

		re.GLSL().TextureShader().Deselect();

		re.glEnable(GL_CULL_FACE);
	}
}

quint64 AuItemPicture::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_HideURL);
	mem += StringMemoryUsage(m_PictureURL);
	mem += sizeof(m_RotatePicture);
	mem += sizeof(m_PictureTexture);
	mem += sizeof(m_PictureVAO);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemStateMessage::AuItemStateMessage(const QString& text, quint8 state, COMMODITYID cid, quint8 quality, 
		quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItemState(state, cid, quality, empireOfOrigin, count)
{
	m_Message = text;
}

AuItemStateMessage::AuItemStateMessage(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) : AuItemState(cid, quality, empireOfOrigin, count)
{
}

AuItemStateMessage::AuItemStateMessage()
{
}

AuItemStateMessage::AuItemStateMessage(const AuItemStateMessage& c) : AuItemState(c)
{
	m_Message = c.m_Message;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemStateMessage::Clone() const
{
	return new AuItemStateMessage(*this);
}

QString AuItemStateMessage::ListName() const
{
	return AuItemState::ListName();
}

QString AuItemStateMessage::HtmlDesc() const
{
	QString itemDesc = AuItemState::HtmlDesc();

	if(!m_Message.isEmpty())
		itemDesc += QObject::tr("Message <b>%1</b>").arg(m_Message);

	return itemDesc;
}

void AuItemStateMessage::operator>>(QDataStream& out) const
{
	AuItemState::operator>>(out);
	out << m_Message;
}

void AuItemStateMessage::operator<<(QDataStream& in)
{
	AuItemState::operator<<(in);
	in >> m_Message;
}

void AuItemStateMessage::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItemState::Refresh(item, re, al);

	const AuItemStateMessage* c = dynamic_cast<const AuItemStateMessage*>(item);
	if(!c)
		return;

	m_Message = c->m_Message;
}

quint64 AuItemStateMessage::MemoryUsage() const
{
	quint64 mem = AuItemState::MemoryUsage();
	mem += StringMemoryUsage(m_Message);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemContainer::AuItemContainer(quint8 capacity, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Capacity = capacity;
}

AuItemContainer::AuItemContainer(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Capacity = Commodity().m_capacity.width() * Commodity().m_capacity.height();
	Q_ASSERT(m_Capacity);
}

AuItemContainer::AuItemContainer()
{
	m_Capacity = 0;
}

AuItemContainer::AuItemContainer(const AuItemContainer& c) : AuItem(c), m_Contents(c.m_Contents)
{
	m_Capacity = c.m_Capacity;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemContainer::Clone() const
{
	return new AuItemContainer(*this);
}

QString AuItemContainer::ListName() const
{
	return AuItem::ListName();
}

QString AuItemContainer::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	if(CommodityId() == CID_RedBinder ||
		CommodityId() == CID_GreenBinder ||
		CommodityId() == CID_BlueBinder)
		itemDesc += QObject::tr("Capacity <b>%2 documents</b><br>"
				"Contents <b>%1 documents</b>")
			.arg(m_Contents.size())
			.arg(CapacityInPackUnits());
	else
		itemDesc += QObject::tr("Capacity <b>%3 container units (cu)</b><br>"
				"Contents <b>%1 items totaling %2cu</b>")
			.arg(m_Contents.size())
			.arg(m_Contents.ContainsPackUnits())
			.arg(CapacityInPackUnits());

	return itemDesc;
}

void AuItemContainer::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_Capacity;
	out << m_Contents;
}

void AuItemContainer::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_Capacity;
	in >> m_Contents;
}

void AuItemContainer::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemContainer* c = dynamic_cast<const AuItemContainer*>(item);
	if(!c)
		return;

	m_Capacity = c->m_Capacity;
	m_Contents.Refresh(c->m_Contents, re, al);
}

bool AuItemContainer::FitItemIntoContainer(const AuItem& cItem)
{
	// Can't put an item inside an item of the same exact type.
	if(CommodityId() == cItem.CommodityId())
		return false;

	// Can't exceed the volume of this container.
	if(m_Contents.ContainsPackUnits() + cItem.VolumeInPackUnits() > CapacityInPackUnits())
		return false;

	// Binders only store paper and discs.
	if(CommodityId() == CID_RedBinder ||
			CommodityId() == CID_GreenBinder ||
			CommodityId() == CID_BlueBinder)
	{
		switch(cItem.CommodityId())
		{
		default:
			return false;
		case CID_BdDesignDisk:
		case CID_BdDesignDrawing:
		case CID_BlankDisk:
		case CID_MissionDisk:
		case CID_OfficerLog:
		case CID_Paper:
		case CID_PatentAPCApparatus:
		case CID_PatentBattleShipApparatus:
		case CID_PatentCommercialShipApparatus:
		case CID_PatentDistillingGin:
		case CID_PatentDistillingRum:
		case CID_PatentDistillingTequila:
		case CID_PatentDistillingVodka:
		case CID_PatentDistillingWhiskey:
		case CID_PatentDrillingPhlogiston:
		case CID_PatentDrillingPolytaride:
		case CID_PatentDrillingViathol:
		case CID_PatentExtractingFlomentum:
		case CID_PatentExtractingMagmex:
		case CID_PatentExtractingMyrathane:
		case CID_PatentFarmMachineryApparatus:
		case CID_PatentGeneratingElectricityusingCoal:
		case CID_PatentGeneratingElectricityusingGasoline:
		case CID_PatentGeneratingElectricityusingHydrogen:
		case CID_PatentGeneratingElectricityusingNaturalGas:
		case CID_PatentGeneratingElectricityusingNuclearFuelRod:
		case CID_PatentGeneratingElectricityusingRadioactives:
		case CID_PatentGeneratingElectricityusingSunlight:
		case CID_PatentGeneratingElectricityusingWaterinEnvironment:
		case CID_PatentGeneratingElectricityusingWind:
		case CID_PatentHelicopterApparatus:
		case CID_PatentMakingAK:
		case CID_PatentMakingAntidoteusingHerbs:
		case CID_PatentMakingAntidoteusingMinerals:
		case CID_PatentMakingAntidoteusingPetrochemicals:
		case CID_PatentMakingAntitoxinusingAnimalCarcass:
		case CID_PatentMakingAntitoxinusingEggs:
		case CID_PatentMakingAtmosphereCondenserBayModule:
		case CID_PatentMakingBallisticWeaponBayModule:
		case CID_PatentMakingBallisticWeaponTurretModule:
		case CID_PatentMakingBeacon:
		case CID_PatentMakingBlankDisk:
		case CID_PatentMakingBombBlBlackCat:
		case CID_PatentMakingBombBlPiPorcupine:
		case CID_PatentMakingBombBlRaLosAlamos:
		case CID_PatentMakingBombElStTesla:
		case CID_PatentMakingBombSlCrMeatSlicer:
		case CID_PatentMakingBombSuPoMuffler:
		case CID_PatentMakingCandleusingPetrochemicals:
		case CID_PatentMakingCandy:
		case CID_PatentMakingCapacitorModule:
		case CID_PatentMakingCloakingShieldModule:
		case CID_PatentMakingCommandAndControlModule:
		case CID_PatentMakingCompressorBeamLens:
		case CID_PatentMakingComputer:
		case CID_PatentMakingCryoHeatSink:
		case CID_PatentMakingCyberneticsMedicalModule:
		case CID_PatentMakingDNASampleKit:
		case CID_PatentMakingDisintegratorRayLens:
		case CID_PatentMakingDisruptorBeamLens:
		case CID_PatentMakingDynamite:
		case CID_PatentMakingElectricLantern:
		case CID_PatentMakingElectronBeamLens:
		case CID_PatentMakingElectronicPart:
		case CID_PatentMakingEnergyShieldModule:
		case CID_PatentMakingEnergyWeaponBayModule:
		case CID_PatentMakingEnergyWeaponTurretModule:
		case CID_PatentMakingEnvironmentSuit:
		case CID_PatentMakingEnvironmentSuitHelmet:
		case CID_PatentMakingErgRayLens:
		case CID_PatentMakingFreezeRayLens:
		case CID_PatentMakingGeneticsMedicalModule:
		case CID_PatentMakingGigacell:
		case CID_PatentMakingGlueGun:
		case CID_PatentMakingGravCoupling:
		case CID_PatentMakingGravPack:
		case CID_PatentMakingGravityDriveModule:
		case CID_PatentMakingGunPowder:
		case CID_PatentMakingHeatRayLens:
		case CID_PatentMakingHeavyArmsAmmoPi:
		case CID_PatentMakingHeavyArmsAmmoPiAc:
		case CID_PatentMakingHeavyArmsAmmoPiBl:
		case CID_PatentMakingHeavyArmsAmmoPiRa:
		case CID_PatentMakingHeavyWeapon:
		case CID_PatentMakingHydrogenPowerPlantModule:
		case CID_PatentMakingHydrogenfromNaturalGas:
		case CID_PatentMakingIceCream:
		case CID_PatentMakingIoTube:
		case CID_PatentMakingJewelryStorage:
		case CID_PatentMakingKineticShieldModule:
		case CID_PatentMakingKnifeusingMetalandPlastic:
		case CID_PatentMakingLargeMissileBlBuFireball:
		case CID_PatentMakingLargeMissileBlThumper:
		case CID_PatentMakingLargeMissileSlPoSideviper:
		case CID_PatentMakingLargeRocketBlBuMolotov:
		case CID_PatentMakingLargeRocketBlThud:
		case CID_PatentMakingLargeRocketBuDiScudpox:
		case CID_PatentMakingLargeRocketMotor:
		case CID_PatentMakingLaserPistol:
		case CID_PatentMakingLaserRifle:
		case CID_PatentMakingLifeSupportGravityModule:
		case CID_PatentMakingLifeSupportModule:
		case CID_PatentMakingMagmiumHeatSink:
		case CID_PatentMakingMagmiumKnife:
		case CID_PatentMakingMagmiumPowerArmorHelmet:
		case CID_PatentMakingMagmiumPowerArmorSuit:
		case CID_PatentMakingMetalPowerArmorHelmet:
		case CID_PatentMakingMetalPowerArmorSuit:
		case CID_PatentMakingMicroscope:
		case CID_PatentMakingMyrasplicer:
		case CID_PatentMakingNanopathCalibrator:
		case CID_PatentMakingNeutronBeamLens:
		case CID_PatentMakingNitrateExplosive:
		case CID_PatentMakingNuclearFuelRod:
		case CID_PatentMakingOscilloscope:
		case CID_PatentMakingParticleCollectorBayModule:
		case CID_PatentMakingPastry:
		case CID_PatentMakingPersonnelTransporterModule:
		case CID_PatentMakingPharmaceuticalsusingPetrochemicals:
		case CID_PatentMakingPistol:
		case CID_PatentMakingPistolAmmunition:
		case CID_PatentMakingPlastic:
		case CID_PatentMakingPlasticExplosive:
		case CID_PatentMakingPlutonium:
		case CID_PatentMakingPreonRing:
		case CID_PatentMakingPressureTent:
		case CID_PatentMakingProspectorBayModule:
		case CID_PatentMakingProtonBeamLens:
		case CID_PatentMakingRifle:
		case CID_PatentMakingRifleAmmunition:
		case CID_PatentMakingRocketDriveModule:
		case CID_PatentMakingSensorModule:
		case CID_PatentMakingShotgun:
		case CID_PatentMakingShotgunAmmunition:
		case CID_PatentMakingSiphonBayModule:
		case CID_PatentMakingSmallMissileBlBuBarnburner:
		case CID_PatentMakingSmallMissileBlZipper:
		case CID_PatentMakingSmallMissileCrFrSnowball:
		case CID_PatentMakingSmallRocketBlBuFirecracker:
		case CID_PatentMakingSmallRocketBlPartypopper:
		case CID_PatentMakingSmallRocketMotor:
		case CID_PatentMakingSmallRocketSlPiMeathook:
		case CID_PatentMakingSolderingIron:
		case CID_PatentMakingStasisBeamLens:
		case CID_PatentMakingStorageCrystal:
		case CID_PatentMakingSurgeryMedicalModule:
		case CID_PatentMakingTesloidDissipator:
		case CID_PatentMakingThumbDrive:
		case CID_PatentMakingTractorBeamBayModule:
		case CID_PatentMakingVehicleTransporterModule:
		case CID_PatentMakingVulciumHeatSink:
		case CID_PatentMakingVulciumKnife:
		case CID_PatentMakingVulciumPowerArmorHelmet:
		case CID_PatentMakingVulciumPowerArmorSuit:
		case CID_PatentMakingWarp1FTLDriveModule:
		case CID_PatentMakingWarp2FTLDriveModule:
		case CID_PatentMakingWarp3FTLDriveModule:
		case CID_PatentMakingWarp4FTLDriveModule:
		case CID_PatentMakingWarp5FTLDriveModule:
		case CID_PatentMakingWarp6FTLDriveModule:
		case CID_PatentMakingWarp7FTLDriveModule:
		case CID_PatentMakingWarp8FTLDriveModule:
		case CID_PatentMakingWarp9FTLDriveModule:
		case CID_PatentMakingWaterusingAirandHydrogen:
		case CID_PatentMakingWeaponDeploymentBayModule:
		case CID_PatentMakingWeaponLauncherBayModule:
		case CID_PatentMakingWeaponLauncherTurretModule:
		case CID_PatentMakingWildcatterBayModule:
		case CID_PatentMakingWormholePFTLDriveModule:
		case CID_PatentMakingWormholePNFTLDriveModule:
		case CID_PatentMakingWormholePNNFTLDriveModule:
		case CID_PatentMineMachineryApparatus:
		case CID_PatentMiningAdamantite:
		case CID_PatentMiningBolite:
		case CID_PatentMiningEludium:
		case CID_PatentMiningLumenite:
		case CID_PatentMiningRadioactives:
		case CID_PatentMiningVulcanite:
		case CID_PatentMotorBikeApparatus:
		case CID_PatentPlasticSailboatApparatus:
		case CID_PatentRefiningAirfromAtmosphere:
		case CID_PatentRefiningAirfromWater:
		case CID_PatentRefiningAntifluxParticlesfromAtmosphere:
		case CID_PatentRefiningBorexinoPrecipitatefromAtmosphere:
		case CID_PatentRefiningCryozine:
		case CID_PatentRefiningFertilizerfromAir:
		case CID_PatentRefiningFertilizerfromAtmosphere:
		case CID_PatentRefiningGasoline:
		case CID_PatentRefiningHydrogenfromAtmosphere:
		case CID_PatentRefiningHydrogenfromWater:
		case CID_PatentRefiningIoplasma:
		case CID_PatentRefiningPetrochemicals:
		case CID_PatentSUVApparatus:
		case CID_PatentSmeltingAdamantine:
		case CID_PatentSmeltingMagmium:
		case CID_PatentSmeltingVulcium:
		case CID_PatentSpaceFighterF1Apparatus:
		case CID_PatentSpaceFighterF2Apparatus:
		case CID_PatentSpaceRocketApparatus:
		case CID_PatentSpaceTransportApparatus:
		case CID_PatentSubmarineApparatus:
		case CID_PatentTankApparatus:
		case CID_PatentWeavingTextilesusingPlastic:
		case CID_ScDesignDisk:
		case CID_ScDesignDrawing:
		case CID_SpacecraftTitle:
			// This item can be stored in a binder.
			break;
		}
	}

	// Store a clone of the item into my contents vector.
	m_Contents.Insert(cItem.Clone());
	return true;
}

// Recursively determines the weight of this item and all subordinate items in this container.
float AuItemContainer::Weight() const
{
	// Antigravity backpack only returns the weight of the backpack itself, not the contents.
	if(CommodityId() == CID_GravPack)
		return Commodity().m_wt;

	return Commodity().m_wt + m_Contents.Weight();
}

void AuItemContainer::ListPatents(AuKnownPatents& patents) const
{
	AuItem::ListPatents(patents);
	m_Contents.ListPatents(patents);
}

bool AuItemContainer::CanCombine() const
{
	if(!m_Contents.empty())
		return false;
	return AuItem::CanCombine();
}

quint64 AuItemContainer::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_Capacity);
	mem += m_Contents.MemoryUsage();
	return mem;
}

bool AuItemContainer::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	return m_Contents.VoidOwnershipTitle(propertyId, claimantId, claimantName);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemBdDesign::AuItemBdDesign(const AuBdDesignExchangeEntry& exchange,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count), m_Exchange(exchange)
{
}

AuItemBdDesign::AuItemBdDesign(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) : AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemBdDesign::AuItemBdDesign()
{
}

AuItemBdDesign::AuItemBdDesign(const AuItemBdDesign& c) : AuItem(c), m_Exchange(c.m_Exchange)
{
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemBdDesign::Clone() const
{
	return new AuItemBdDesign(*this);
}

QString AuItemBdDesign::ListName() const
{
	if(m_Exchange.DesignId())
		return QObject::tr("%4 Blueprint %1 %2 by %3")
			.arg(m_Exchange.DesignId())
			.arg(m_Exchange.DesignName())
			.arg(m_Exchange.DesignArchitectName())
			.arg(BuildingStructureName(m_Exchange.DesignStructure()));
	else
		return QObject::tr("%1 With Invalid Data").arg(AuItem::ListName());
}

QString AuItemBdDesign::SayName() const
{
	return QObject::tr("%4 blueprint %1 by %3")
		.arg(m_Exchange.DesignName())
		.arg(m_Exchange.DesignArchitectName())
		.arg(BuildingStructureName(m_Exchange.DesignStructure()));
}

QString AuItemBdDesign::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(m_Exchange.DesignId())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += m_Exchange.ReportText();
	}

	return itemDesc;
}

QString AuItemBdDesign::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(m_Exchange.DesignId())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += m_Exchange.ReportToolTip();
	}

	return itemDesc;
}

void AuItemBdDesign::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_Exchange;
}

void AuItemBdDesign::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_Exchange;
}

void AuItemBdDesign::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemBdDesign* c = dynamic_cast<const AuItemBdDesign*>(item);
	if(!c)
		return;

	m_Exchange = c->m_Exchange;
}

quint64 AuItemBdDesign::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += m_Exchange.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemScDesign::AuItemScDesign(const AuScDesignExchangeEntry& exchange,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count), m_Exchange(exchange)
{
}

AuItemScDesign::AuItemScDesign(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
		quint32 count/*=1*/) : AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemScDesign::AuItemScDesign()
{
}

AuItemScDesign::AuItemScDesign(const AuItemScDesign& c) : AuItem(c), m_Exchange(c.m_Exchange)
{
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemScDesign::Clone() const
{
	return new AuItemScDesign(*this);
}

QString AuItemScDesign::ListName() const
{
	if(m_Exchange.DesignId())
		return QObject::tr("%4 Blueprint %1 %2 by %3")
			.arg(m_Exchange.DesignId())
			.arg(m_Exchange.DesignName())
			.arg(m_Exchange.DesignArchitectName())
			.arg(SpacecraftStructureName(m_Exchange.DesignStructure()));
	else
		return QObject::tr("%1 With Invalid Data").arg(AuItem::ListName());
}

QString AuItemScDesign::SayName() const
{
	return QObject::tr("%4 blueprint %1 by %3")
		.arg(m_Exchange.DesignName())
		.arg(m_Exchange.DesignArchitectName())
		.arg(SpacecraftStructureName(m_Exchange.DesignStructure()));
}

QString AuItemScDesign::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(m_Exchange.DesignId())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += m_Exchange.ReportText();
	}

	return itemDesc;
}

QString AuItemScDesign::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(m_Exchange.DesignId())
	{
		if(itemDesc.right(5) != "/div>")
			itemDesc += "<br>";

		itemDesc += m_Exchange.ReportToolTip();
	}

	return itemDesc;
}

void AuItemScDesign::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	out << m_Exchange;
}

void AuItemScDesign::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_Exchange;
}

void AuItemScDesign::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemScDesign* c = dynamic_cast<const AuItemScDesign*>(item);
	if(!c)
		return;

	m_Exchange = c->m_Exchange;
}

quint64 AuItemScDesign::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += m_Exchange.MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemTEUContainer::AuItemTEUContainer(const AuId& destCityId, const QString& destDesc, float destDistPc,
		const AuId& destSysId, const AuId& destSectorId, quint32 empireOfOrigin) :
	AuItem(CID_ContainerTEU, 128, empireOfOrigin, 1)
{
	m_DestCityId = destCityId;
	m_DestDesc = destDesc;
	m_DestSectorId = destSectorId;
	m_DestSysId = destSysId;
	m_Distance = destDistPc;
}

AuItemTEUContainer::AuItemTEUContainer(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_Distance = 0;
}

AuItemTEUContainer::AuItemTEUContainer()
{
	m_Distance = 0;
}

AuItemTEUContainer::AuItemTEUContainer(const AuItemTEUContainer& c) : AuItem(c), m_Contents(c.m_Contents)
{
	m_CarrierId = c.m_CarrierId;
	m_CarrierName = c.m_CarrierName;
	m_DestCityId = c.m_DestCityId;
	m_DestDesc = c.m_DestDesc;
	m_DestSectorId = c.m_DestSectorId;
	m_DestSysId = c.m_DestSysId;
	m_Distance = c.m_Distance;
	m_Pickup = c.m_Pickup;
}

const AuItemTEUContainer& AuItemTEUContainer::operator=(const AuItemTEUContainer& c)
{
	AuItem::operator=(c);
	m_Contents = c.m_Contents;
	m_CarrierId = c.m_CarrierId;
	m_CarrierName = c.m_CarrierName;
	m_DestCityId = c.m_DestCityId;
	m_DestDesc = c.m_DestDesc;
	m_DestSectorId = c.m_DestSectorId;
	m_DestSysId = c.m_DestSysId;
	m_Distance = c.m_Distance;
	m_Pickup = c.m_Pickup;
	return *this;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemTEUContainer::Clone() const
{
	return new AuItemTEUContainer(*this);
}

QString AuItemTEUContainer::ListName() const
{
	if(m_Pickup == AuTime())
		return AuItem::ListName();
	// Use the string encoded pickup time as the TEU identifier.
	return QString("%1 %2")
		.arg(AuItem::ListName())
		.arg(StringBE26(m_Pickup.MSecsSinceEpoch()));
}

AuTime AuItemTEUContainer::EstimatedDeliveryTime() const
{
	return m_Pickup.AddSecs(StandardDeliveryTime());
}

QString AuItemTEUContainer::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	quint32 teus = TEUCount();
	if(teus == 1)
		itemDesc += QObject::tr("Hold Volume: <b>1 TEU at %L1cu</b>").arg(CargoVolume());
	else
		itemDesc += QObject::tr("Hold Volume: <b>%L1 TEUs at %L2cu</b>").arg(teus).arg(CargoVolume());

	if(!m_CarrierId.IsNull())
	{
		itemDesc += QObject::tr("<br>Carrier: <b>%1 <i>%2</i></b>").arg(m_CarrierName).arg(m_CarrierId.StringId());
		itemDesc += QObject::tr("<br>Pickup Date: <b>%1</b>").arg(m_Pickup.UTC().toLocalTime().toString());
		if(!m_DestCityId.IsNull())
		{
			AuTime deliverTime(EstimatedDeliveryTime());
			itemDesc += QObject::tr("<br>Delivery Date: <b>%1</b>").arg(deliverTime.UTC().toLocalTime().toString());
			float payment = PaymentNow();
			itemDesc += QObject::tr("<br>Payment: <b>%L1%2 if delivered now.</b>")
				.arg(payment, 0,'f',0)
				.arg(AuGlobal::CurrencySymbol());
		}
	}

	if(!m_DestCityId.IsNull())
	{
		itemDesc += QObject::tr("<br>Destination: <b>%1</b>").arg(m_DestDesc);
		itemDesc += QObject::tr("<br>Trip Distance: <b>%L1pc</b>").arg(m_Distance, 0,'f',1);
	}

	itemDesc += QObject::tr("<br>Contents: <b>%L1 packages totaling %L2cu</b>")
		.arg(m_Contents.size())
		.arg(m_Contents.TotalCargoVolume());
	QMultiMap<QString, AuItem*> sorted;
	for(AuItemHash::const_iterator ci = m_Contents.cbegin(); ci != m_Contents.cend(); ci++)
		sorted.insert((*ci).second->ListName(), (*ci).second);
	for(QMultiMap<QString, AuItem*>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
		itemDesc += QString("<br><img height=\"16\" src=\"qrc%1\"> %2")
			.arg((*si)->BitmapName())
			.arg((*si)->ListDesc());

	return itemDesc;
}

void AuItemTEUContainer::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);

	out << m_CarrierId;
	if(!m_CarrierId.IsNull())
		out << m_CarrierName;
	out << m_Contents;
	out << m_DestCityId;
	out << m_DestDesc;
	out << m_DestSectorId;
	out << m_DestSysId;
	out << m_Distance;
	out << m_Pickup;
}

void AuItemTEUContainer::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	in >> m_CarrierId;
	if(!m_CarrierId.IsNull())
		in >> m_CarrierName;
	in >> m_Contents;
	in >> m_DestCityId;
	in >> m_DestDesc;
	bool hasBrokerQ = m_DestDesc.right(1) == "*";
	bool hasBrokenBrokerQ = m_DestDesc.right(1) == "#";
	in >> m_DestSectorId;
	in >> m_DestSysId;
	in >> m_Distance;
	in >> m_Pickup;

	if(hasBrokerQ)
	{
		quint8 brokerQ;
		in >> brokerQ;
		m_DestDesc = m_DestDesc.left(m_DestDesc.length()-1);	// Strip off the hoky version indicator.
	}
	else if(hasBrokenBrokerQ)
		m_DestDesc = m_DestDesc.left(m_DestDesc.length()-1);	// Strip off the hoky version indicator that was broken.
}

void AuItemTEUContainer::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemTEUContainer* c = dynamic_cast<const AuItemTEUContainer*>(item);
	if(!c)
		return;

	m_CarrierId = c->m_CarrierId;
	m_CarrierName = c->m_CarrierName;
	m_Contents = c->m_Contents;
	m_DestCityId = c->m_DestCityId;
	m_DestDesc = c->m_DestDesc;
	m_DestSectorId = c->m_DestSectorId;
	m_DestSysId = c->m_DestSysId;
	m_Distance = c->m_Distance;
	m_Pickup = c->m_Pickup;
}

quint32 AuItemTEUContainer::CargoVolume() const
{
	//return m_Contents.TotalCargoVolume();
	return TEUCount() * 1360;
}

// Recursively determines the weight of this item and all subordinate items in this container.
float AuItemTEUContainer::Weight() const
{
	return Commodity().m_wt + m_Contents.Weight();
}

void AuItemTEUContainer::SetPickedUp(const AuId& carrierId, const QString& carrierName, const AuTime& when)
{
	m_CarrierId = carrierId;
	m_CarrierName = carrierName;
	m_Pickup = when;
}

//static
quint32 AuItemTEUContainer::TEUCount(quint32 cargo)
{
	if(cargo > 0xffffffff - 1359)
		return cargo / 1360 + 1;
	return (cargo + 1359) / 1360;
}

//static
float AuItemTEUContainer::PriceTEUPc()
{
	return 1000;
}

quint32 AuItemTEUContainer::StandardDeliveryTime() const
{
	return (quint32)((m_Distance * FeetPerParsec()) / SpeedOfFreight());	// result in real world sec
}

float AuItemTEUContainer::StandardPayment() const
{
	return EstimatePayment(TEUCount());
}

float AuItemTEUContainer::EstimatePayment(quint32 teuCount) const
{
	return teuCount * m_Distance * PriceTEUPc();
}

float AuItemTEUContainer::PaymentNow() const
{
	float elapsedSec = m_Pickup.ElapsedSec();
	if(elapsedSec <= 0)
		elapsedSec = 1;
	float payment = (float)StandardDeliveryTime() / elapsedSec * StandardPayment();
	if(payment < 1000)
		payment = 1000;
	return payment;
}

//static
int AuItemTEUContainer::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuItemTEUContainer>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuItemTEUContainer>();

	return metaType;
}

quint64 AuItemTEUContainer::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_CarrierId);
	mem += StringMemoryUsage(m_CarrierName);
	mem += m_Contents.MemoryUsage();
	mem += sizeof(m_DestCityId);
	mem += StringMemoryUsage(m_DestDesc);
	mem += sizeof(m_DestSectorId);
	mem += sizeof(m_DestSysId);
	mem += sizeof(m_Distance);
	mem += sizeof(m_Pickup);
	return mem;
}

bool AuItemTEUContainer::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	return m_Contents.VoidOwnershipTitle(propertyId, claimantId, claimantName);
}

void AuItemTEUContainer::SetReceiverDefaulted()
{
	m_DestCityId = AuId();
	m_DestDesc = "";
	m_DestSectorId = AuId();
	m_DestSysId = AuId();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemStorageMedia::AuItemStorageMedia(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemStorageMedia::AuItemStorageMedia()
{
}

AuItemStorageMedia::AuItemStorageMedia(const AuItemStorageMedia& c) : AuItem(c), m_BlueprintsBd(c.m_BlueprintsBd),
	m_BlueprintsSc(c.m_BlueprintsSc), m_Missions(c.m_Missions), m_Papers(c.m_Papers), m_Patents(c.m_Patents)
{
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemStorageMedia::Clone() const
{
	return new AuItemStorageMedia(*this);
}

QString AuItemStorageMedia::ListName() const
{
	if(Inscription().isEmpty())
		return QObject::tr("%1, %2/%3 free")
			.arg(AuItem::ListName())
			.arg(StorageSpaceAvailable())
			.arg(StorageSpaceTotal());
	else
		return QObject::tr("%1, %2/%3 free, %4")
			.arg(AuItem::ListName())
			.arg(StorageSpaceAvailable())
			.arg(StorageSpaceTotal())
			.arg(Inscription());
}

QString AuItemStorageMedia::SayName() const
{
	return QObject::tr("%1 %2").arg(DbCommodity::QualityDesc(Quality())).arg(AuItem::ListName());
}

QString AuItemStorageMedia::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Volume summary.
	itemDesc += QObject::tr("Storage Space Total <b>%1</b>").arg(StorageSpaceTotal());
	itemDesc += QObject::tr("<br>Storage Space Free <b>%1</b>").arg(StorageSpaceAvailable());
	itemDesc += QObject::tr("<br>Storage Space Used <b>%1</b>").arg(StorageSpaceUsed());

	// Contents.
	if(!m_Missions.empty())
		itemDesc += m_Missions.HtmlListContents();
	if(!m_Papers.empty())
	{
		// Alphabetize
		QMap<QString, int> papers;
		for(QStringList::const_iterator pi = m_Papers.cbegin(); pi != m_Papers.cend(); pi++)
			papers.insert(*pi, 0);

		// Format
		QString cont = QObject::tr("<hr><img height=\"16\" src=\"qrc:data/Folder.png\"> Papers <b>%1</b>").arg(m_Papers.size());
		for(QMap<QString, int>::const_iterator pi = papers.cbegin(); pi != papers.cend(); pi++)
		{
			QString s(*pi);
			if(s.length() > 40)
				s = s.left(40) + "...";
			cont += QString("<br>%2 <img height=\"16\" src=\"qrc:data/c_Paper.png\"> %1")
				.arg(s)
				.arg(g_Au.MiddleDotSymbol());
		}
		return cont;
	}
	if(!m_Patents.empty())
		itemDesc += m_Patents.HtmlListContents();
	if(!m_BlueprintsBd.empty())
		itemDesc += m_BlueprintsBd.HtmlListContents();
	if(!m_BlueprintsSc.empty())
		itemDesc += m_BlueprintsSc.HtmlListContents();
	return itemDesc;
}

QString AuItemStorageMedia::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	// Volume summary.
	itemDesc += QObject::tr("Storage Space Total <b>%1</b>").arg(StorageSpaceTotal());
	itemDesc += QObject::tr("<br>Storage Space Free <b>%1</b>").arg(StorageSpaceAvailable());
	itemDesc += QObject::tr("<br>Storage Space Used <b>%1</b>").arg(StorageSpaceUsed());

	// Contents summary.
	if(!m_Missions.empty())
		itemDesc += QObject::tr("<br>Missions <b>%1</b>").arg(m_Missions.size());
	if(!m_Papers.empty())
		itemDesc += QObject::tr("<br>Papers <b>%1</b>").arg(m_Papers.size());
	if(!m_Patents.empty())
		itemDesc += QObject::tr("<br>Patents <b>%1</b>").arg(m_Patents.size());
	if(!m_BlueprintsBd.empty())
		itemDesc += QObject::tr("<br>Building Blueprints <b>%1</b>").arg(m_BlueprintsBd.size());
	if(!m_BlueprintsSc.empty())
		itemDesc += QObject::tr("<br>Spacecraft Blueprints <b>%1</b>").arg(m_BlueprintsSc.size());
	return itemDesc;
}

void AuItemStorageMedia::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	quint8 hasWhat = 0;
	if(!m_BlueprintsSc.empty())
		hasWhat |= 0x01;
	if(!m_Patents.empty())
		hasWhat |= 0x02;
	if(!m_Missions.empty())
		hasWhat |= 0x04;
	if(!m_BlueprintsBd.empty())
		hasWhat |= 0x08;
	if(!m_Papers.empty())
		hasWhat |= 0x10;
	out << hasWhat;
	if(hasWhat & 0x01)
		out << m_BlueprintsSc;
	if(hasWhat & 0x02)
		out << m_Patents;
	if(hasWhat & 0x04)
		out << m_Missions;
	if(hasWhat & 0x08)
		out << m_BlueprintsBd;
	if(hasWhat & 0x10)
		out << m_Papers;
}

void AuItemStorageMedia::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	if(hasWhat & 0x01)
		in >> m_BlueprintsSc;
	if(hasWhat & 0x02)
		in >> m_Patents;
	if(hasWhat & 0x04)
		in >> m_Missions;
	if(hasWhat & 0x08)
		in >> m_BlueprintsBd;
	if(hasWhat & 0x10)
		in >> m_Papers;
}

void AuItemStorageMedia::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);
	const AuItemStorageMedia* c = dynamic_cast<const AuItemStorageMedia*>(item);
	if(!c)
		return;
	m_BlueprintsBd = c->m_BlueprintsBd;
	m_BlueprintsSc = c->m_BlueprintsSc;
	m_Missions = c->m_Missions;
	m_Papers = c->m_Papers;
	m_Patents = c->m_Patents;
}

quint8 AuItemStorageMedia::StorageSpaceAvailable() const
{
	return StorageSpaceTotal() - StorageSpaceUsed();
}

quint8 AuItemStorageMedia::StorageSpaceTotal() const
{
	return DbCommodity::DataStorageSpace(CommodityId(), Quality());
}

quint8 AuItemStorageMedia::StorageSpaceUsed() const
{
	return m_BlueprintsBd.size() + m_BlueprintsSc.size() + m_Missions.size() + m_Papers.size() + m_Patents.size();
}

void AuItemStorageMedia::ListPatents(AuKnownPatents& patents) const
{
	for(QHash<COMMODITYID, quint8>::const_iterator pi = m_Patents.cbegin(); pi != m_Patents.cend(); pi++)
		patents.RecordPatent(pi.key(), *pi);
}

int AuItemStorageMedia::StoreData(const AuItem* dataItem)
{
	// Try to store patent.
	if(DbCommodity::IsPatent(dataItem->CommodityId()))
	{
		if(m_Patents.HasPatent(dataItem->CommodityId(), dataItem->Quality()))
			return 7;					// Patent already stored here.
		if(!StorePatent(dataItem->CommodityId(), dataItem->Quality()))
			return 1;					// No space to store a patent.
		return 3;						// Patent stored successfully.
	}

	// Try to store building blueprint.
	const AuItemBdDesign* bdi = dynamic_cast<const AuItemBdDesign*>(dataItem);
	if(bdi)
	{
		// Some blueprints cannot be copied.
		if(m_BlueprintsBd.contains(bdi->DesignId()))
			return 8;					// Blueprint already stored here.
		if(!StorageSpaceAvailable())
			return 1;					// No space to store a blueprint.
		if(!bdi->Exchange().CanCopy())
			return 9;					// dataItem cannot be stored due to copy restrictions.
		m_BlueprintsBd.Add(bdi->Exchange());
		return 5;						// Spacecraft blueprint stored successfully.
	}

	// Try to store spacecraft blueprint.
	const AuItemScDesign* sci = dynamic_cast<const AuItemScDesign*>(dataItem);
	if(sci)
	{
		// Some blueprints cannot be copied.
		if(m_BlueprintsSc.contains(sci->DesignId()))
			return 8;					// Blueprint already stored here.
		if(!StorageSpaceAvailable())
			return 1;					// No space to store a blueprint.
		if(!sci->Exchange().CanCopy())
			return 9;					// dataItem cannot be stored due to copy restrictions.
		m_BlueprintsSc.Add(sci->Exchange());
		return 4;						// Spacecraft blueprint stored successfully.
	}

	// Try to store mission.
	const AuItemMission* mi = dynamic_cast<const AuItemMission*>(dataItem);
	if(mi)
	{
		if(!StoreMission(mi->MissionName(), mi->Mission()))
			return 1;					// No space to store a mission.
		return 11;						// Mission stored successfully.
	}

	// Try to store paper.
	if(dataItem->CommodityId() == CID_Paper)
	{
		if(dataItem->Inscription().isEmpty())
			return 12;
		if(m_Papers.contains(dataItem->Inscription()))
			return 13;					// Paper already stored here.
		if(!StorePaper(dataItem->Inscription()))
			return 1;					// No space to store a paper.
		return 14;						// Paper stored successfully.
	}

	// Try to store contents of storage media.
	const AuItemStorageMedia* smi = dynamic_cast<const AuItemStorageMedia*>(dataItem);
	if(smi)
	{
		if(StorageSpaceAvailable() < smi->StorageSpaceUsed())
			return 1;					// Insufficient space to store the contents of the storage media.
		// Check for copyrighted blueprints.
		for(AuBdDesignExchange::const_iterator di = smi->BlueprintsBd().cbegin(); di != smi->BlueprintsBd().cend(); di++)
			if(!m_BlueprintsBd.contains(di.key()))
				if(!(*di).CanCopy())
					return 10;			// dataItem cannot be stored due to copy restrictions on a contained item.
		for(AuScDesignExchange::const_iterator di = smi->BlueprintsSc().cbegin(); di != smi->BlueprintsSc().cend(); di++)
			if(!m_BlueprintsSc.contains(di.key()))
				if(!(*di).CanCopy())
					return 10;			// dataItem cannot be stored due to copy restrictions on a contained item.
		// Copy blueprints from the storage media.
		for(AuBdDesignExchange::const_iterator di = smi->BlueprintsBd().cbegin(); di != smi->BlueprintsBd().cend(); di++)
			if(!m_BlueprintsBd.contains(di.key()))
				m_BlueprintsBd.Add(*di);
		for(AuScDesignExchange::const_iterator di = smi->BlueprintsSc().cbegin(); di != smi->BlueprintsSc().cend(); di++)
			if(!m_BlueprintsSc.contains(di.key()))
				m_BlueprintsSc.Add(*di);
		// Copy papers from the storage media.
		m_Papers.append(smi->Papers());
		m_Papers.removeDuplicates();
		// Copy patents from the storage media.
		smi->ListPatents(m_Patents);
		// Copy missions from the storage media.
		for(AuMissionList::const_iterator mi = smi->Missions().cbegin(); mi != smi->Missions().cend(); mi++)
			StoreMission(mi.key(), *mi);
		return 6;
	}

	// The item could not be stored.
	return 2;
}

int AuItemStorageMedia::StoreDesign(const AuBdDesignExchangeEntry& design)
{
	if(!StorageSpaceAvailable())
		return 1;			// No space.
	if(m_BlueprintsBd.contains(design.DesignId()))
		return 2;			// Id already stored.
	int result = m_BlueprintsBd.Add(design);
	if(result == 1)
		return 0;			// added nu entry
	else if(result == 2)
		return 3;			// replaced existing entry
	else if(result == 3)
		return 4;			// rejected duplicate entry
	return 5;				// Unknown error.
}

int AuItemStorageMedia::StoreDesign(const AuScDesignExchangeEntry& design)
{
	if(!StorageSpaceAvailable())
		return 1;			// No space.
	if(m_BlueprintsSc.contains(design.DesignId()))
		return 2;			// Id already stored.
	int result = m_BlueprintsSc.Add(design);
	if(result == 1)
		return 0;			// added nu entry
	else if(result == 2)
		return 3;			// replaced existing entry
	else if(result == 3)
		return 4;			// rejected duplicate entry
	return 5;				// Unknown error.
}

bool AuItemStorageMedia::DeleteStoredBlueprintsBd(const QSet<quint32>& blueprints)
{
	bool anyDeleted = false;
	for(QSet<quint32>::const_iterator di = blueprints.cbegin(); di != blueprints.cend(); di++)
		if(m_BlueprintsBd.Remove(*di))
			anyDeleted = true;
	return anyDeleted;
}

bool AuItemStorageMedia::DeleteStoredBlueprintsSc(const QSet<quint32>& blueprints)
{
	bool anyDeleted = false;
	for(QSet<quint32>::const_iterator di = blueprints.cbegin(); di != blueprints.cend(); di++)
		if(m_BlueprintsSc.Remove(*di))
			anyDeleted = true;
	return anyDeleted;
}

bool AuItemStorageMedia::StorePaper(const QString& text)
{
	if(!StorageSpaceAvailable())
		return false;
	m_Papers.append(text);
	return true;
}

bool AuItemStorageMedia::DeleteStoredPapers(const QSet<QString>& paperTexts)
{
	bool anyDeleted = false;
	for(QSet<QString>::const_iterator pi = paperTexts.cbegin(); pi != paperTexts.cend(); pi++)
		if(m_Papers.removeAll(*pi))
			anyDeleted = true;
	return anyDeleted;
}

bool AuItemStorageMedia::StorePatent(COMMODITYID patentCID, quint8 patentQ)
{
	if(!StorageSpaceAvailable())
		return false;
	m_Patents.RecordPatent(patentCID, patentQ);
	return true;
}

bool AuItemStorageMedia::DeleteStoredPatents(const QSet<COMMODITYID>& patentCIDs)
{
	bool anyDeleted = false;
	for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
		if(m_Patents.RemovePatent(*pi))
			anyDeleted = true;
	return anyDeleted;
}

bool AuItemStorageMedia::StoreMission(const QString& missionName, const AuMission& mission)
{
	if(!StorageSpaceAvailable())
		return false;
	m_Missions.insert(missionName, mission);
	return true;
}

bool AuItemStorageMedia::DeleteStoredMissions(const QSet<QString>& missions)
{
	bool anyDeleted = false;
	for(QSet<QString>::const_iterator mi = missions.cbegin(); mi != missions.cend(); mi++)
		if(m_Missions.Remove(*mi))
			anyDeleted = true;
	return anyDeleted;
}

quint64 AuItemStorageMedia::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += m_BlueprintsBd.MemoryUsage();
	mem += m_BlueprintsSc.MemoryUsage();
	mem += m_Missions.MemoryUsage();
	for(QStringList::const_iterator pi = m_Papers.cbegin(); pi != m_Papers.cend(); pi++)
		mem += StringMemoryUsage((*pi));
	mem += m_Patents.MemoryUsage();
	return mem;
}

bool AuItemStorageMedia::CanStack(const AuItem* item) const
{
	if(CommodityId() != item->CommodityId())
		return false;
	if(StorageSpaceUsed())
		return false;
	const AuItemStorageMedia* o = dynamic_cast<const AuItemStorageMedia*>(item);
	if(!o)
		return false;
	if(o->StorageSpaceUsed())
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItemOwnershipTitle::AuItemOwnershipTitle(const AuId& propertyId, const QString& propertyName, const QString& propertyDesc,
		COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
	m_PropertyId = propertyId;
	m_PropertyDesc = propertyDesc;
	m_PropertyName = propertyName;
}

AuItemOwnershipTitle::AuItemOwnershipTitle(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, quint32 count/*=1*/) :
	AuItem(cid, quality, empireOfOrigin, count)
{
}

AuItemOwnershipTitle::AuItemOwnershipTitle()
{
}

AuItemOwnershipTitle::AuItemOwnershipTitle(const AuItemOwnershipTitle& c) : AuItem(c)
{
	m_ClaimantId = c.m_ClaimantId;
	m_ClaimantName = c.m_ClaimantName;
	m_ClaimTime = c.m_ClaimTime;
	m_PropertyId = c.m_PropertyId;
	m_PropertyDesc = c.m_PropertyDesc;
	m_PropertyName = c.m_PropertyName;
}

// Returns a duplicate of this item, allocated on the heap.
AuItem* AuItemOwnershipTitle::Clone() const
{
	return new AuItemOwnershipTitle(*this);
}

QString AuItemOwnershipTitle::ListName() const
{
	if(m_ClaimantId.IsNull())
		return QObject::tr("Title to %1 %2").arg(m_PropertyName).arg(m_PropertyId.StringId());
	else
		return QObject::tr("Void Title to %1 %2").arg(m_PropertyName).arg(m_PropertyId.StringId());
}

QString AuItemOwnershipTitle::SayName() const
{
	if(m_ClaimantId.IsNull())
		return QObject::tr("title to %1").arg(m_PropertyName);
	else
		return QObject::tr("void title to %1").arg(m_PropertyName);
}

QString AuItemOwnershipTitle::HtmlDesc() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	if(CommodityId() == CID_SpacecraftTitle)
	{
		if(m_ClaimantId.IsNull())
			itemDesc += QObject::tr("Title of Spacecraft Ownership <b>%1 <i>%2</i></b>"
					"<p>"
					"Described as %3."
					"</p>"
					"<p>"
					"Use this title to claim ownership of the company spacecraft identified. "
					"All existing titles to the spacecraft will become null and void. "
					"</p>"
					"<p>"
					"Spacecraft announce on the <i>Company</i> comm channel before and after ownership is claimed "
					"so both the old and new owners are notified. "
					"The channel does not open automatically. "
					"Open the channel before claiming the title, if you want to receive the response. "
					"</p>"
					"<p>"
					"The flag of the spacecraft changes to the new owner's empire. "
					"People aboard the spacecraft do not change their empire of allegiance. "
					"This could cause some people to abandon their posts or mutiny. "
					"</p>")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_PropertyDesc);
		else
			itemDesc += QObject::tr("Void Title of Spacecraft Ownership <b>%1 <i>%2</i></b>"
					"<p>"
					"Described as %3."
					"</p>"
					"<p>"
					"%4 <i>%5</i> claimed ownership %6. "
					"This spacecraft title is null and void. "
					"It cannot be used to claim ownership of any spacecraft."
					"</p>")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_PropertyDesc)
				.arg(m_ClaimantName)
				.arg(m_ClaimantId.StringId())
				.arg(m_ClaimTime.UTC().toLocalTime().toString());
	}
	else
	{
		if(m_ClaimantId.IsNull())
			itemDesc += QObject::tr("Title of Property Ownership <b>%1 <i>%2</i></b>"
					"<p>"
					"Described as %3."
					"</p>"
					"<p>"
					"Use this title to claim ownership of the property identified. "
					"All existing titles to the property will become null and void. "
					"</p>")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_PropertyDesc);
		else
			itemDesc += QObject::tr("Void Title of Property Ownership <b>%1 <i>%2</i></b>"
					"<p>"
					"Described as %3."
					"</p>"
					"<p>"
					"%4 <i>%5</i> claimed ownership %6. "
					"This title is null and void. "
					"It cannot be used to claim ownership of any property."
					"</p>")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_PropertyDesc)
				.arg(m_ClaimantName)
				.arg(m_ClaimantId.StringId())
				.arg(m_ClaimTime.UTC().toLocalTime().toString());
	}
	return itemDesc;
}

QString AuItemOwnershipTitle::HtmlToolTip() const
{
	QString itemDesc = AuItem::HtmlDesc();
	if(itemDesc.right(5) != "/div>")
		itemDesc += "<br>";

	if(CommodityId() == CID_SpacecraftTitle)
	{
		if(m_ClaimantId.IsNull())
			itemDesc += QObject::tr("Title of Spacecraft Ownership <b>%1 <i>%2</i></b><br>"
					"Use this title to claim ownership of the company spacecraft identified.")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId());
		else
			itemDesc += QObject::tr("Void Title of Spacecraft Ownership <b>%1 <i>%2</i></b><br>"
					"%3 <i>%4</i> claimed ownership %5. "
					"This spacecraft title is null and void. ")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_ClaimantName)
				.arg(m_ClaimantId.StringId())
				.arg(m_ClaimTime.UTC().toLocalTime().toString());
	}
	else
	{
		if(m_ClaimantId.IsNull())
			itemDesc += QObject::tr("Title of Property Ownership <b>%1 <i>%2</i></b><br>"
					"Use this title to claim ownership of the property identified.")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId());
		else
			itemDesc += QObject::tr("Void Title of Property Ownership <b>%1 <i>%2</i></b><br>"
					"%3 <i>%4</i> claimed ownership %5. "
					"This property title is null and void. ")
				.arg(m_PropertyName)
				.arg(m_PropertyId.StringId())
				.arg(m_ClaimantName)
				.arg(m_ClaimantId.StringId())
				.arg(m_ClaimTime.UTC().toLocalTime().toString());
	}
	return itemDesc;
}

void AuItemOwnershipTitle::operator>>(QDataStream& out) const
{
	AuItem::operator>>(out);
	quint8 hasWhat = 0;
	if(!m_ClaimantId.IsNull())
		hasWhat |= 0x01;
	if(!m_PropertyDesc.isEmpty())
		hasWhat |= 0x02;
	out << hasWhat;
	out << m_PropertyId;
	out << m_PropertyName;
	if(hasWhat & 0x02)
		out << m_PropertyDesc;
	if(hasWhat & 0x01)
	{
		out << m_ClaimantId;
		out << m_ClaimantName;
		out << m_ClaimTime;
	}
}

void AuItemOwnershipTitle::operator<<(QDataStream& in)
{
	AuItem::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;
	in >> m_PropertyId;
	in >> m_PropertyName;
	if(hasWhat & 0x02)
		in >> m_PropertyDesc;
	if(hasWhat & 0x01)
	{
		in >> m_ClaimantId;
		in >> m_ClaimantName;
		in >> m_ClaimTime;
	}
}

void AuItemOwnershipTitle::Refresh(const AuItem* item, AuGLEffects* re, AuSound* al)
{
	AuItem::Refresh(item, re, al);

	const AuItemOwnershipTitle* c = dynamic_cast<const AuItemOwnershipTitle*>(item);
	if(!c)
		return;

	m_ClaimantId = c->m_ClaimantId;
	m_ClaimantName = c->m_ClaimantName;
	m_ClaimTime = c->m_ClaimTime;
	m_PropertyDesc = c->m_PropertyDesc;
	m_PropertyId = c->m_PropertyId;
	m_PropertyName = c->m_PropertyName;
}

quint64 AuItemOwnershipTitle::MemoryUsage() const
{
	quint64 mem = AuItem::MemoryUsage();
	mem += sizeof(m_ClaimantId);
	mem += StringMemoryUsage(m_ClaimantName);
	mem += sizeof(m_ClaimTime);
	mem += StringMemoryUsage(m_PropertyDesc);
	mem += sizeof(m_PropertyId);
	mem += StringMemoryUsage(m_PropertyName);
	return mem;
}

bool AuItemOwnershipTitle::VoidOwnershipTitle(const AuId& propertyId, const AuId& claimantId, const QString& claimantName)
{
	if(m_PropertyId != propertyId)
		return false;	// Different property.
	if(!m_ClaimantId.IsNull())
		return false;	// Already claimed.
	m_ClaimantId = claimantId;
	m_ClaimantName = claimantName;
	m_ClaimTime.Set();
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
