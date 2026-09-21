//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbcommodity.cpp,v 1.150 2022/05/09 15:49:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dbcommodity.h"
//
#include "auglobal.h"
#include <QDebug>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <QVariant>

QDebug operator<<(QDebug dbg, const AuSysId& sysId)
{
	dbg.nospace() << ScSystemName(sysId.System()) << ":" << sysId.Which();
	return dbg.space();
}

COMMODITYID AuSysId::RandomPart(COMMODITYID systemModule) const
{
	switch(m_System)
	{
	// Structural
	case SYS_Hull:
	case SYS_Fuel:
	case SYS_Hold:
		return HullModuleMaterial(systemModule);
	default:
		return systemModule;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DbCommodity::DbCommodity()
{
	m_id = CID_Invalid;
	m_ammo1Type = AMMO_None;
	m_ammo1Qty = 0;				// Round count for weapons that use ammo.
	m_ammo2Type = AMMO_None;
	m_ammo2Qty = 0;				// Round count for weapons that use ammo.
	m_ArmorEffectAbsorbAcid = 0;
	m_ArmorEffectAbsorbBludgeon = 0;
	m_ArmorEffectAbsorbCold = 0;
	m_ArmorEffectAbsorbCrush = 0;
	m_ArmorEffectAbsorbDisease = 0;
	m_ArmorEffectAbsorbElectric = 0;
	m_ArmorEffectAbsorbHeat = 0;
	m_ArmorEffectAbsorbImplant = 0;
	m_ArmorEffectAbsorbPierce = 0;
	m_ArmorEffectAbsorbPoison = 0;
	m_ArmorEffectAbsorbRadiation = 0;
	m_ArmorEffectAbsorbSlash = 0;
	m_ArmorEffectAbsorbSonic = 0;
	m_ArmorEffectAbsorbStun = 0;
	m_ArmorEffectAbsorbSuffocate = 0;
	m_ArmorEffectBlockAcid = 0;
	m_ArmorEffectBlockBludgeon = 0;
	m_ArmorEffectBlockCold = 0;
	m_ArmorEffectBlockCrush = 0;
	m_ArmorEffectBlockDisease = 0;
	m_ArmorEffectBlockElectric = 0;
	m_ArmorEffectBlockHeat = 0;
	m_ArmorEffectBlockImplant = 0;
	m_ArmorEffectBlockPierce = 0;
	m_ArmorEffectBlockPoison = 0;
	m_ArmorEffectBlockRadiation = 0;
	m_ArmorEffectBlockSlash = 0;
	m_ArmorEffectBlockSonic = 0;
	m_ArmorEffectBlockStun = 0;
	m_ArmorEffectBlockSuffocate = 0;
	m_ArmorModeAbsorbArea = 0;
	m_ArmorModeAbsorbContact = 0;
	m_ArmorModeAbsorbProjectile = 0;
	m_ArmorModeAbsorbPsionic = 0;
	m_ArmorModeBlockArea = 0;
	m_ArmorModeBlockContact = 0;
	m_ArmorModeBlockProjectile = 0;
	m_ArmorModeBlockPsionic = 0;
	m_armorSpec = WE_None;
	m_baseCost = -1;
	m_bitsWear = 0;				// OR'd INVENTORYPOS bits
	m_combits = 0;				// OR'd COMMODITYBITS from AuDef.
	m_food = 0;
	m_health = 0;				// Health effect of usage.
	m_insertHeight = 0; 
	m_UpVector = AuPoint(0,0,1);
	m_light = 0;				// Light radius in 10s of feet
	m_newcond = 0;				// Condition points when nu.
	m_stackLimit = 0;
	m_wpn1Mode = WM_Not;
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
	m_wpn2Mode = WM_Not;
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
	m_wt = 0;					// in pounds for each unit
}

DbCommodity::~DbCommodity()
{
}

DbCommodity::DbCommodity(const DbCommodity &c)
{
	m_id = c.m_id;
	m_ammo1Qty = c.m_ammo1Qty;
	m_ammo1Type = c.m_ammo1Type;
	m_ammo2Qty = c.m_ammo2Qty;
	m_ammo2Type = c.m_ammo2Type;
	m_ArmorEffectAbsorbAcid = c.m_ArmorEffectAbsorbAcid;
	m_ArmorEffectAbsorbBludgeon = c.m_ArmorEffectAbsorbBludgeon;
	m_ArmorEffectAbsorbCold = c.m_ArmorEffectAbsorbCold;
	m_ArmorEffectAbsorbCrush = c.m_ArmorEffectAbsorbCrush;
	m_ArmorEffectAbsorbDisease = c.m_ArmorEffectAbsorbDisease;
	m_ArmorEffectAbsorbElectric = c.m_ArmorEffectAbsorbElectric;
	m_ArmorEffectAbsorbHeat = c.m_ArmorEffectAbsorbHeat;
	m_ArmorEffectAbsorbImplant = c.m_ArmorEffectAbsorbImplant;
	m_ArmorEffectAbsorbPierce = c.m_ArmorEffectAbsorbPierce;
	m_ArmorEffectAbsorbPoison = c.m_ArmorEffectAbsorbPoison;
	m_ArmorEffectAbsorbRadiation = c.m_ArmorEffectAbsorbRadiation;
	m_ArmorEffectAbsorbSlash = c.m_ArmorEffectAbsorbSlash;
	m_ArmorEffectAbsorbSonic = c.m_ArmorEffectAbsorbSonic;
	m_ArmorEffectAbsorbStun = c.m_ArmorEffectAbsorbStun;
	m_ArmorEffectAbsorbSuffocate = c.m_ArmorEffectAbsorbSuffocate;
	m_ArmorEffectBlockAcid = c.m_ArmorEffectBlockAcid;
	m_ArmorEffectBlockBludgeon = c.m_ArmorEffectBlockBludgeon;
	m_ArmorEffectBlockCold = c.m_ArmorEffectBlockCold;
	m_ArmorEffectBlockCrush = c.m_ArmorEffectBlockCrush;
	m_ArmorEffectBlockDisease = c.m_ArmorEffectBlockDisease;
	m_ArmorEffectBlockElectric = c.m_ArmorEffectBlockElectric;
	m_ArmorEffectBlockHeat = c.m_ArmorEffectBlockHeat;
	m_ArmorEffectBlockImplant = c.m_ArmorEffectBlockImplant;
	m_ArmorEffectBlockPierce = c.m_ArmorEffectBlockPierce;
	m_ArmorEffectBlockPoison = c.m_ArmorEffectBlockPoison;
	m_ArmorEffectBlockRadiation = c.m_ArmorEffectBlockRadiation;
	m_ArmorEffectBlockSlash = c.m_ArmorEffectBlockSlash;
	m_ArmorEffectBlockSonic = c.m_ArmorEffectBlockSonic;
	m_ArmorEffectBlockStun = c.m_ArmorEffectBlockStun;
	m_ArmorEffectBlockSuffocate = c.m_ArmorEffectBlockSuffocate;
	m_ArmorModeAbsorbArea = c.m_ArmorModeAbsorbArea;
	m_ArmorModeAbsorbContact = c.m_ArmorModeAbsorbContact;
	m_ArmorModeAbsorbProjectile = c.m_ArmorModeAbsorbProjectile;
	m_ArmorModeAbsorbPsionic = c.m_ArmorModeAbsorbPsionic;
	m_ArmorModeBlockArea = c.m_ArmorModeBlockArea;
	m_ArmorModeBlockContact = c.m_ArmorModeBlockContact;
	m_ArmorModeBlockProjectile = c.m_ArmorModeBlockProjectile;
	m_ArmorModeBlockPsionic = c.m_ArmorModeBlockPsionic;
	m_armorSpec = c.m_armorSpec;
	m_baseCost = c.m_baseCost;
	m_bitsWear = c.m_bitsWear;
	m_capacity = c.m_capacity;
	m_combits = c.m_combits;
	m_food = c.m_food;
	m_health = c.m_health;
	m_iconPath = c.m_iconPath;
	m_modelPath = c.m_modelPath;
	m_insertHeight = c.m_insertHeight;
	m_UpVector = c.m_UpVector;
	m_light = c.m_light;
	m_name = c.m_name;
	m_newcond = c.m_newcond;
	m_size = c.m_size;
	m_stackLimit = c.m_stackLimit;
	m_wpn1Mode = c.m_wpn1Mode;
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
	m_wpn2Mode = c.m_wpn2Mode;
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
	m_wt = c.m_wt;
}

DbCommodity& DbCommodity::operator=(const DbCommodity &c)
{
	m_id = c.m_id;
	m_ammo1Qty = c.m_ammo1Qty;
	m_ammo1Type = c.m_ammo1Type;
	m_ammo2Qty = c.m_ammo2Qty;
	m_ammo2Type = c.m_ammo2Type;
	m_ArmorEffectAbsorbAcid = c.m_ArmorEffectAbsorbAcid;
	m_ArmorEffectAbsorbBludgeon = c.m_ArmorEffectAbsorbBludgeon;
	m_ArmorEffectAbsorbCold = c.m_ArmorEffectAbsorbCold;
	m_ArmorEffectAbsorbCrush = c.m_ArmorEffectAbsorbCrush;
	m_ArmorEffectAbsorbDisease = c.m_ArmorEffectAbsorbDisease;
	m_ArmorEffectAbsorbElectric = c.m_ArmorEffectAbsorbElectric;
	m_ArmorEffectAbsorbHeat = c.m_ArmorEffectAbsorbHeat;
	m_ArmorEffectAbsorbImplant = c.m_ArmorEffectAbsorbImplant;
	m_ArmorEffectAbsorbPierce = c.m_ArmorEffectAbsorbPierce;
	m_ArmorEffectAbsorbPoison = c.m_ArmorEffectAbsorbPoison;
	m_ArmorEffectAbsorbRadiation = c.m_ArmorEffectAbsorbRadiation;
	m_ArmorEffectAbsorbSlash = c.m_ArmorEffectAbsorbSlash;
	m_ArmorEffectAbsorbSonic = c.m_ArmorEffectAbsorbSonic;
	m_ArmorEffectAbsorbStun = c.m_ArmorEffectAbsorbStun;
	m_ArmorEffectAbsorbSuffocate = c.m_ArmorEffectAbsorbSuffocate;
	m_ArmorEffectBlockAcid = c.m_ArmorEffectBlockAcid;
	m_ArmorEffectBlockBludgeon = c.m_ArmorEffectBlockBludgeon;
	m_ArmorEffectBlockCold = c.m_ArmorEffectBlockCold;
	m_ArmorEffectBlockCrush = c.m_ArmorEffectBlockCrush;
	m_ArmorEffectBlockDisease = c.m_ArmorEffectBlockDisease;
	m_ArmorEffectBlockElectric = c.m_ArmorEffectBlockElectric;
	m_ArmorEffectBlockHeat = c.m_ArmorEffectBlockHeat;
	m_ArmorEffectBlockImplant = c.m_ArmorEffectBlockImplant;
	m_ArmorEffectBlockPierce = c.m_ArmorEffectBlockPierce;
	m_ArmorEffectBlockPoison = c.m_ArmorEffectBlockPoison;
	m_ArmorEffectBlockRadiation = c.m_ArmorEffectBlockRadiation;
	m_ArmorEffectBlockSlash = c.m_ArmorEffectBlockSlash;
	m_ArmorEffectBlockSonic = c.m_ArmorEffectBlockSonic;
	m_ArmorEffectBlockStun = c.m_ArmorEffectBlockStun;
	m_ArmorEffectBlockSuffocate = c.m_ArmorEffectBlockSuffocate;
	m_ArmorModeAbsorbArea = c.m_ArmorModeAbsorbArea;
	m_ArmorModeAbsorbContact = c.m_ArmorModeAbsorbContact;
	m_ArmorModeAbsorbProjectile = c.m_ArmorModeAbsorbProjectile;
	m_ArmorModeAbsorbPsionic = c.m_ArmorModeAbsorbPsionic;
	m_ArmorModeBlockArea = c.m_ArmorModeBlockArea;
	m_ArmorModeBlockContact = c.m_ArmorModeBlockContact;
	m_ArmorModeBlockProjectile = c.m_ArmorModeBlockProjectile;
	m_ArmorModeBlockPsionic = c.m_ArmorModeBlockPsionic;
	m_armorSpec = c.m_armorSpec;
	m_baseCost = c.m_baseCost;
	m_bitsWear = c.m_bitsWear;
	m_capacity = c.m_capacity;
	m_combits = c.m_combits;
	m_food = c.m_food;
	m_health = c.m_health;
	m_iconPath = c.m_iconPath;
	m_modelPath = c.m_modelPath;
	m_insertHeight = c.m_insertHeight;
	m_UpVector = c.m_UpVector;
	m_light = c.m_light;
	m_name = c.m_name;
	m_newcond = c.m_newcond;
	m_size = c.m_size;
	m_stackLimit = c.m_stackLimit;
	m_wpn1Mode = c.m_wpn1Mode;
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
	m_wpn2Mode = c.m_wpn2Mode;
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
	m_wt = c.m_wt;
	return *this;
}

void DbCommodity::Tx(QDataStream& out) const
{
	out << (quint16)m_id;
	out << m_ammo1Qty;
	out << (quint8)m_ammo1Type;
	out << m_ammo2Qty;
	out << (quint8)m_ammo2Type;
	out << (quint8)(m_ArmorEffectAbsorbAcid * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbBludgeon * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbCold * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbCrush * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbDisease * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbElectric * 200.f);
	out << (quint8)0;	//(m_ArmorEffectAbsorbEnergy * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbHeat * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbImplant * 200.f);
	out << (quint8)0;	//(m_ArmorEffectAbsorbKinetic * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbPierce * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbPoison * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbRadiation * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSlash * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSonic * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbStun * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSuffocate * 200.f);
	out << (quint8)m_ArmorEffectBlockAcid;
	out << (quint8)m_ArmorEffectBlockBludgeon;
	out << (quint8)m_ArmorEffectBlockCold;
	out << (quint8)m_ArmorEffectBlockCrush;
	out << (quint8)m_ArmorEffectBlockDisease;
	out << (quint8)m_ArmorEffectBlockElectric;
	out << (quint8)0;	//m_ArmorEffectBlockEnergy;
	out << (quint8)m_ArmorEffectBlockHeat;
	out << (quint8)m_ArmorEffectBlockImplant;
	out << (quint8)0;	//m_ArmorEffectBlockKinetic;
	out << (quint8)m_ArmorEffectBlockPierce;
	out << (quint8)m_ArmorEffectBlockPoison;
	out << (quint8)m_ArmorEffectBlockRadiation;
	out << (quint8)m_ArmorEffectBlockSlash;
	out << (quint8)m_ArmorEffectBlockSonic;
	out << (quint8)m_ArmorEffectBlockStun;
	out << (quint8)m_ArmorEffectBlockSuffocate;
	out << (quint8)(m_ArmorModeAbsorbArea * 200.f);
	out << (quint8)(m_ArmorModeAbsorbContact * 200.f);
	out << (quint8)(m_ArmorModeAbsorbProjectile * 200.f);
	out << (quint8)(m_ArmorModeAbsorbPsionic * 200.f);
	out << (quint8)m_ArmorModeBlockArea;
	out << (quint8)m_ArmorModeBlockContact;
	out << (quint8)m_ArmorModeBlockProjectile;
	out << (quint8)m_ArmorModeBlockPsionic;
	out << (quint8)m_armorSpec;
	out << m_bitsWear;
	out << (quint8)m_capacity.width();
	out << (quint8)m_capacity.height();
	out << m_combits;
	out << m_food;
	out << m_health;
	out << m_iconPath;
	out << m_modelPath;
	out << m_insertHeight;
	out << m_UpVector;
	out << m_light;
	out << m_name;
	out << m_newcond;
	out << (quint8)m_size.width();
	out << (quint8)m_size.height();
	out << m_stackLimit;
	out << (quint8)m_wpn1Mode;
	out << m_wpn1Range;
	out << (quint8)m_wpn1Effect1;
	out << m_wpn1Damage1;
	out << m_wpn1DamageMod1;
	out << m_wpn1Radius1;
	out << m_wpn1AP1;
	out << m_wpn1ST1;
	out << (quint8)m_wpn1Effect2;
	out << m_wpn1Damage2;
	out << m_wpn1DamageMod2;
	out << m_wpn1Radius2;
	out << m_wpn1AP2;
	out << m_wpn1ST2;
	out << (quint8)m_wpn2Mode;
	out << m_wpn2Range;
	out << (quint8)m_wpn2Effect1;
	out << m_wpn2Damage1;
	out << m_wpn2DamageMod1;
	out << m_wpn2Radius1;
	out << m_wpn2AP1;
	out << m_wpn2ST1;
	out << (quint8)m_wpn2Effect2;
	out << m_wpn2Damage2;
	out << m_wpn2DamageMod2;
	out << m_wpn2Radius2;
	out << m_wpn2AP2;
	out << m_wpn2ST2;
	out << m_wt;
}

void DbCommodity::Rx(QDataStream& in)
{
	quint16 ui16;
	in >> ui16;
	m_id = (COMMODITYID)ui16;
	in >> m_ammo1Qty;
	quint8 ui8;
	in >> ui8;
	m_ammo1Type = (AMMOTYPE)ui8;
	in >> m_ammo2Qty;
	in >> ui8;
	m_ammo2Type = (AMMOTYPE)ui8;
	in >> ui8;
	m_ArmorEffectAbsorbAcid = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbBludgeon = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbCold = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbCrush = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbDisease = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbElectric = (float)ui8 / 200.f;
	in >> ui8;
	//m_ArmorEffectAbsorbEnergy = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbHeat = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbImplant = (float)ui8 / 200.f;
	in >> ui8;
	//m_ArmorEffectAbsorbKinetic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbPierce = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbPoison = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbRadiation = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSlash = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSonic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbStun = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSuffocate = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectBlockAcid = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockBludgeon = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockCold = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockCrush = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockDisease = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockElectric = (float)ui8;
	in >> ui8;
	//m_ArmorEffectBlockEnergy = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockHeat = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockImplant = (float)ui8;
	in >> ui8;
	//m_ArmorEffectBlockKinetic = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockPierce = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockPoison = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockRadiation = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSlash = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSonic = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockStun = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSuffocate = (float)ui8;
	in >> ui8;
	m_ArmorModeAbsorbArea = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbContact = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbProjectile = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbPsionic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeBlockArea = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockContact = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockProjectile = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockPsionic = (float)ui8;
	in >> ui8;
	m_armorSpec = (WEAPONEFFECT)ui8;
	in >> m_bitsWear;
	quint8 x,y;
	in >> x;
	in >> y;
	m_capacity = QSize(x,y);
	in >> m_combits;
	in >> m_food;
	in >> m_health;
	in >> m_iconPath;
	in >> m_modelPath;
	in >> m_insertHeight;
	in >> m_UpVector;
	in >> m_light;
	in >> m_name;
	in >> m_newcond;
	in >> x;
	in >> y;
	m_size = QSize(x,y);
	in >> m_stackLimit;
	in >> ui8;
	m_wpn1Mode = (WEAPONMODE)ui8;
	in >> m_wpn1Range;
	in >> ui8;
	m_wpn1Effect1 = (WEAPONEFFECT)ui8;
	in >> m_wpn1Damage1;
	in >> m_wpn1DamageMod1;
	in >> m_wpn1Radius1;
	in >> m_wpn1AP1;
	in >> m_wpn1ST1;
	in >> ui8;
	m_wpn1Effect2 = (WEAPONEFFECT)ui8;
	in >> m_wpn1Damage2;
	in >> m_wpn1DamageMod2;
	in >> m_wpn1Radius2;
	in >> m_wpn1AP2;
	in >> m_wpn1ST2;
	in >> ui8;
	m_wpn2Mode = (WEAPONMODE)ui8;
	in >> m_wpn2Range;
	in >> ui8;
	m_wpn2Effect1 = (WEAPONEFFECT)ui8;
	in >> m_wpn2Damage1;
	in >> m_wpn2DamageMod1;
	in >> m_wpn2Radius1;
	in >> m_wpn2AP1;
	in >> m_wpn2ST1;
	in >> ui8;
	m_wpn2Effect2 = (WEAPONEFFECT)ui8;
	in >> m_wpn2Damage2;
	in >> m_wpn2DamageMod2;
	in >> m_wpn2Radius2;
	in >> m_wpn2AP2;
	in >> m_wpn2ST2;
	in >> m_wt;

	// Client calculates m_baseCost now, so server doesn't have to send it.
	m_baseCost = -1;

	//qDebug("DbCommodity::Rx cid %d", m_id);
}

void DbCommodity::operator>>(QDataStream& out) const
{
	out << (quint16)m_id;
	out << m_ammo1Qty;
	out << (quint8)m_ammo1Type;
	out << m_ammo2Qty;
	out << (quint8)m_ammo2Type;
	out << (quint8)(m_ArmorEffectAbsorbAcid * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbBludgeon * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbCold * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbCrush * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbDisease * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbElectric * 200.f);
	out << (quint8)0;	//(m_ArmorEffectAbsorbEnergy * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbHeat * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbImplant * 200.f);
	out << (quint8)0;	//(m_ArmorEffectAbsorbKinetic * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbPierce * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbPoison * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbRadiation * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSlash * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSonic * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbStun * 200.f);
	out << (quint8)(m_ArmorEffectAbsorbSuffocate * 200.f);
	out << (quint8)m_ArmorEffectBlockAcid;
	out << (quint8)m_ArmorEffectBlockBludgeon;
	out << (quint8)m_ArmorEffectBlockCold;
	out << (quint8)m_ArmorEffectBlockCrush;
	out << (quint8)m_ArmorEffectBlockDisease;
	out << (quint8)m_ArmorEffectBlockElectric;
	out << (quint8)0;	//m_ArmorEffectBlockEnergy;
	out << (quint8)m_ArmorEffectBlockHeat;
	out << (quint8)m_ArmorEffectBlockImplant;
	out << (quint8)0;	//m_ArmorEffectBlockKinetic;
	out << (quint8)m_ArmorEffectBlockPierce;
	out << (quint8)m_ArmorEffectBlockPoison;
	out << (quint8)m_ArmorEffectBlockRadiation;
	out << (quint8)m_ArmorEffectBlockSlash;
	out << (quint8)m_ArmorEffectBlockSonic;
	out << (quint8)m_ArmorEffectBlockStun;
	out << (quint8)m_ArmorEffectBlockSuffocate;
	out << (quint8)(m_ArmorModeAbsorbArea * 200.f);
	out << (quint8)(m_ArmorModeAbsorbContact * 200.f);
	out << (quint8)(m_ArmorModeAbsorbProjectile * 200.f);
	out << (quint8)(m_ArmorModeAbsorbPsionic * 200.f);
	out << (quint8)m_ArmorModeBlockArea;
	out << (quint8)m_ArmorModeBlockContact;
	out << (quint8)m_ArmorModeBlockProjectile;
	out << (quint8)m_ArmorModeBlockPsionic;
	out << (quint8)m_armorSpec;
	out << m_bitsWear;
	out << (quint8)m_capacity.width();
	out << (quint8)m_capacity.height();
	out << m_combits;
	QString desc;
	out << desc;
	out << m_food;
	out << m_health;
	out << m_iconPath;
	out << m_modelPath;
	out << m_insertHeight;
	out << m_UpVector;
	out << m_light;
	out << m_name;
	out << m_newcond;
	out << (quint8)m_size.width();
	out << (quint8)m_size.height();
	out << m_stackLimit;
	out << (quint8)m_wpn1Mode;
	out << m_wpn1Range;
	out << (quint8)m_wpn1Effect1;
	out << m_wpn1Damage1;
	out << m_wpn1DamageMod1;
	out << m_wpn1Radius1;
	out << m_wpn1AP1;
	out << m_wpn1ST1;
	out << (quint8)m_wpn1Effect2;
	out << m_wpn1Damage2;
	out << m_wpn1DamageMod2;
	out << m_wpn1Radius2;
	out << m_wpn1AP2;
	out << m_wpn1ST2;
	out << (quint8)m_wpn2Mode;
	out << m_wpn2Range;
	out << (quint8)m_wpn2Effect1;
	out << m_wpn2Damage1;
	out << m_wpn2DamageMod1;
	out << m_wpn2Radius1;
	out << m_wpn2AP1;
	out << m_wpn2ST1;
	out << (quint8)m_wpn2Effect2;
	out << m_wpn2Damage2;
	out << m_wpn2DamageMod2;
	out << m_wpn2Radius2;
	out << m_wpn2AP2;
	out << m_wpn2ST2;
	out << m_wt;
}

void DbCommodity::operator<<(QDataStream& in)
{
	quint16 ui16;
	in >> ui16;
	m_id = (COMMODITYID)ui16;
	in >> m_ammo1Qty;
	quint8 ui8;
	in >> ui8;
	m_ammo1Type = (AMMOTYPE)ui8;
	in >> m_ammo2Qty;
	in >> ui8;
	m_ammo2Type = (AMMOTYPE)ui8;
	in >> ui8;
	m_ArmorEffectAbsorbAcid = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbBludgeon = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbCold = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbCrush = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbDisease = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbElectric = (float)ui8 / 200.f;
	in >> ui8;
	//m_ArmorEffectAbsorbEnergy = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbHeat = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbImplant = (float)ui8 / 200.f;
	in >> ui8;
	//m_ArmorEffectAbsorbKinetic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbPierce = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbPoison = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbRadiation = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSlash = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSonic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbStun = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectAbsorbSuffocate = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorEffectBlockAcid = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockBludgeon = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockCold = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockCrush = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockDisease = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockElectric = (float)ui8;
	in >> ui8;
	//m_ArmorEffectBlockEnergy = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockHeat = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockImplant = (float)ui8;
	in >> ui8;
	//m_ArmorEffectBlockKinetic = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockPierce = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockPoison = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockRadiation = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSlash = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSonic = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockStun = (float)ui8;
	in >> ui8;
	m_ArmorEffectBlockSuffocate = (float)ui8;
	in >> ui8;
	m_ArmorModeAbsorbArea = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbContact = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbProjectile = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeAbsorbPsionic = (float)ui8 / 200.f;
	in >> ui8;
	m_ArmorModeBlockArea = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockContact = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockProjectile = (float)ui8;
	in >> ui8;
	m_ArmorModeBlockPsionic = (float)ui8;
	in >> ui8;
	m_armorSpec = (WEAPONEFFECT)ui8;
	in >> m_bitsWear;
	quint8 x,y;
	in >> x;
	in >> y;
	m_capacity = QSize(x,y);
	in >> m_combits;
	QString desc;
	in >> desc;
	in >> m_food;
	in >> m_health;
	in >> m_iconPath;
	in >> m_modelPath;
	in >> m_insertHeight;
	in >> m_UpVector;
	in >> m_light;
	in >> m_name;
	in >> m_newcond;
	in >> x;
	in >> y;
	m_size = QSize(x,y);
	in >> m_stackLimit;
	in >> ui8;
	m_wpn1Mode = (WEAPONMODE)ui8;
	in >> m_wpn1Range;
	in >> ui8;
	m_wpn1Effect1 = (WEAPONEFFECT)ui8;
	in >> m_wpn1Damage1;
	in >> m_wpn1DamageMod1;
	in >> m_wpn1Radius1;
	in >> m_wpn1AP1;
	in >> m_wpn1ST1;
	in >> ui8;
	m_wpn1Effect2 = (WEAPONEFFECT)ui8;
	in >> m_wpn1Damage2;
	in >> m_wpn1DamageMod2;
	in >> m_wpn1Radius2;
	in >> m_wpn1AP2;
	in >> m_wpn1ST2;
	in >> ui8;
	m_wpn2Mode = (WEAPONMODE)ui8;
	in >> m_wpn2Range;
	in >> ui8;
	m_wpn2Effect1 = (WEAPONEFFECT)ui8;
	in >> m_wpn2Damage1;
	in >> m_wpn2DamageMod1;
	in >> m_wpn2Radius1;
	in >> m_wpn2AP1;
	in >> m_wpn2ST1;
	in >> ui8;
	m_wpn2Effect2 = (WEAPONEFFECT)ui8;
	in >> m_wpn2Damage2;
	in >> m_wpn2DamageMod2;
	in >> m_wpn2Radius2;
	in >> m_wpn2AP2;
	in >> m_wpn2ST2;
	in >> m_wt;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DbCommodityArray::Load()
{
	clear();

	// Fill the list out to the max.
	while(count() < CID_Count)
		append(DbCommodity());

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	DbCommodity ob;
	QString sq("SELECT id, name, ammo1Qty, ammo1Type, ammo2Qty, ammo2Type, "
			"armorEffectAbsorbAcid, "
			"armorEffectAbsorbBludgeon, "
			"armorEffectAbsorbCold, "
			"armorEffectAbsorbCrush, "
			"armorEffectAbsorbDisease, "
			"armorEffectAbsorbElectric, "
			//"armorEffectAbsorbEnergy, "
			"armorEffectAbsorbHeat, "
			"armorEffectAbsorbImplant, "
			//"armorEffectAbsorbKinetic, "
			"armorEffectAbsorbPierce, "
			"armorEffectAbsorbPoison, "
			"armorEffectAbsorbRadiation, "
			"armorEffectAbsorbSlash, "
			"armorEffectAbsorbSonic, "
			"armorEffectAbsorbStun, "
			"armorEffectAbsorbSuffocate, "
			"armorEffectBlockAcid, "
			"armorEffectBlockBludgeon, "
			"armorEffectBlockCold, "
			"armorEffectBlockCrush, "
			"armorEffectBlockDisease, "
			"armorEffectBlockElectric, "
			//"armorEffectBlockEnergy, "
			"armorEffectBlockHeat, "
			"armorEffectBlockImplant, "
			//"armorEffectBlockKinetic, "
			"armorEffectBlockPierce, "
			"armorEffectBlockPoison, "
			"armorEffectBlockRadiation, "
			"armorEffectBlockSlash, "
			"armorEffectBlockSonic, "
			"armorEffectBlockStun, "
			"armorEffectBlockSuffocate, "
			"armorModeAbsorbArea, "
			"armorModeAbsorbContact, "
			"armorModeAbsorbProjectile, "
			"armorModeAbsorbPsionic, "
			"armorModeBlockArea, "
			"armorModeBlockContact, "
			"armorModeBlockProjectile, "
			"armorModeBlockPsionic, "
			"armorSpec, "
			"bitsWear, capht, capwid, combits, food, health, "
			"iconPath, modelPath, insertHeight, upVectorX, upVectorY, upVectorZ, "
			"light, newcond, sizeht, sizewid, stackLimit, "
			"wpn1Mode, wpn1Range, "
			"wpn1Effect1, wpn1Damage1, wpn1DamageMod1, wpn1Radius1, wpn1AP1, wpn1ST1, "
			"wpn1Effect2, wpn1Damage2, wpn1DamageMod2, wpn1Radius2, wpn1AP2, wpn1ST2, "
			"wpn2Mode, wpn2Range, "
			"wpn2Effect1, wpn2Damage1, wpn2DamageMod1, wpn2Radius1, wpn2AP1, wpn2ST1, "
			"wpn2Effect2, wpn2Damage2, wpn2DamageMod2, wpn2Radius2, wpn2AP2, wpn2ST2, "
			"wt FROM b_Commodity ORDER BY id;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv = 0;	// query value index
		ob.m_id = (COMMODITYID)qry.value(qv++).toInt();
		ob.m_name = qry.value(qv++).toString();
		if(ob.m_name.isEmpty())
			continue;	// Skip commodities with blank names.
		ob.m_ammo1Qty = qry.value(qv++).toInt();
		ob.m_ammo1Type = (AMMOTYPE)qry.value(qv++).toInt();
		ob.m_ammo2Qty = qry.value(qv++).toInt();
		ob.m_ammo2Type = (AMMOTYPE)qry.value(qv++).toInt();
		ob.m_ArmorEffectAbsorbAcid = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbBludgeon = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbCold = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbCrush = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbDisease = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbElectric = qry.value(qv++).toFloat() / 100.f;
		//ob.m_ArmorEffectAbsorbEnergy = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbHeat = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbImplant = qry.value(qv++).toFloat() / 100.f;
		//ob.m_ArmorEffectAbsorbKinetic = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbPierce = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbPoison = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbRadiation = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbSlash = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbSonic = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbStun = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectAbsorbSuffocate = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorEffectBlockAcid = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockBludgeon = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockCold = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockCrush = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockDisease = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockElectric = qry.value(qv++).toFloat();
		//ob.m_ArmorEffectBlockEnergy = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockHeat = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockImplant = qry.value(qv++).toFloat();
		//ob.m_ArmorEffectBlockKinetic = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockPierce = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockPoison = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockRadiation = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockSlash = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockSonic = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockStun = qry.value(qv++).toFloat();
		ob.m_ArmorEffectBlockSuffocate = qry.value(qv++).toFloat();
		ob.m_ArmorModeAbsorbArea = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorModeAbsorbContact = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorModeAbsorbProjectile = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorModeAbsorbPsionic = qry.value(qv++).toFloat() / 100.f;
		ob.m_ArmorModeBlockArea = qry.value(qv++).toFloat();
		ob.m_ArmorModeBlockContact = qry.value(qv++).toFloat();
		ob.m_ArmorModeBlockProjectile = qry.value(qv++).toFloat();
		ob.m_ArmorModeBlockPsionic = qry.value(qv++).toFloat();
		ob.m_armorSpec = (WEAPONEFFECT)qry.value(qv++).toUInt();
		ob.m_bitsWear = qry.value(qv++).toInt();
		short int capht = qry.value(qv++).toInt();
		short int capwid = qry.value(qv++).toInt();
		ob.m_capacity = QSize(capwid,capht);
		ob.m_combits = qry.value(qv++).toInt();
		ob.m_food = qry.value(qv++).toInt();
		ob.m_health = qry.value(qv++).toInt();
		ob.m_iconPath = qry.value(qv++).toString();
		ob.m_modelPath = qry.value(qv++).toString();
		ob.m_insertHeight = qry.value(qv++).toDouble();
		float x = qry.value(qv++).toDouble();
		float y = qry.value(qv++).toDouble();
		float z = qry.value(qv++).toDouble();
		ob.m_UpVector = AuPoint(x,y,z);
		ob.m_UpVector.Unit();

		ob.m_light = qry.value(qv++).toInt();
		ob.m_newcond = qry.value(qv++).toInt();
		short int ht = qry.value(qv++).toInt();
		short int wid = qry.value(qv++).toInt();
		ob.m_size = QSize(wid,ht);
		ob.m_stackLimit = qry.value(qv++).toInt();

		ob.m_wpn1Mode = (WEAPONMODE)qry.value(qv++).toInt();
		ob.m_wpn1Range = qry.value(qv++).toInt();
		ob.m_wpn1Effect1 = (WEAPONEFFECT)qry.value(qv++).toInt();
		ob.m_wpn1Damage1 = qry.value(qv++).toInt();
		ob.m_wpn1DamageMod1 = qry.value(qv++).toInt();
		ob.m_wpn1Radius1 = qry.value(qv++).toInt();
		ob.m_wpn1AP1 = qry.value(qv++).toInt();
		ob.m_wpn1ST1 = qry.value(qv++).toInt();
		ob.m_wpn1Effect2 = (WEAPONEFFECT)qry.value(qv++).toInt();
		ob.m_wpn1Damage2 = qry.value(qv++).toInt();
		ob.m_wpn1DamageMod2 = qry.value(qv++).toInt();
		ob.m_wpn1Radius2 = qry.value(qv++).toInt();
		ob.m_wpn1AP2 = qry.value(qv++).toInt();
		ob.m_wpn1ST2 = qry.value(qv++).toInt();

		ob.m_wpn2Mode = (WEAPONMODE)qry.value(qv++).toInt();
		ob.m_wpn2Range = qry.value(qv++).toInt();
		ob.m_wpn2Effect1 = (WEAPONEFFECT)qry.value(qv++).toInt();
		ob.m_wpn2Damage1 = qry.value(qv++).toInt();
		ob.m_wpn2DamageMod1 = qry.value(qv++).toInt();
		ob.m_wpn2Radius1 = qry.value(qv++).toInt();
		ob.m_wpn2AP1 = qry.value(qv++).toInt();
		ob.m_wpn2ST1 = qry.value(qv++).toInt();
		ob.m_wpn2Effect2 = (WEAPONEFFECT)qry.value(qv++).toInt();
		ob.m_wpn2Damage2 = qry.value(qv++).toInt();
		ob.m_wpn2DamageMod2 = qry.value(qv++).toInt();
		ob.m_wpn2Radius2 = qry.value(qv++).toInt();
		ob.m_wpn2AP2 = qry.value(qv++).toInt();
		ob.m_wpn2ST2 = qry.value(qv++).toInt();

		ob.m_wt = qry.value(qv++).toDouble();

		Add(ob);
	}
}

// Store record using its id as the index for fast lookup performance.
void DbCommodityArray::Add(const DbCommodity& rec)
{
	// We expect this array to be indexable by COMMODITYID. Fill out gaps.
	while(count() <= rec.m_id)
		append(DbCommodity());
	replace(rec.m_id, rec);
}

void DbCommodityArray::operator<<(QDataStream& in)
{
	clear();
	quint16 count;
	in >> count;
	for(;count; count--)
	{
		DbCommodity commodity;
		commodity << in;
		append(commodity);
	}
}

void DbCommodityArray::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it) >> out;
}

// Returns base cost of a commodity modified by quality.
float DbCommodityArray::CommodityQualityCost(COMMODITYID idCommodity, unsigned int quality) const
{
	if(idCommodity == CID_Money)
		return 1;		// Money has a standard cost of 1.
	float qcost = BaseCost(idCommodity);
	if(qcost < 0)
		return qcost;	// undetermined

	// Quality does not affect electricity because it is always perfect.
	if(idCommodity != CID_Electricity)
	{
		// Modify by quality.
		float qmod = QualityBoostF(quality);
		qcost *= qmod;
	}

	// Round up to at least 1.
	if(qcost < 1)
		qcost = 1;

	return qcost;
}

void DbCommodityArray::ListFilterCommodities(QList<COMMODITYID>& commodities, COMMODITYFILTER filter)
{
	for(const_iterator cit = constBegin(); cit != constEnd(); cit++)
	{
		if((*cit).m_id == CID_Invalid)
			continue;											// Skip the invalid commodity.
		switch(filter)
		{
		case FILTER_All:
		case FILTER_Count:
			break;
		case FILTER_Ammo:
			if(!(*cit).IsAmmo())
				continue;
			break;
		case FILTER_ArmorAndClothes:
			if(!(*cit).IsArmorHelmetClothingOrHat())
				continue;
			break;
		case FILTER_BuildingDesigns:
			if(!(*cit).IsBdDesign())
				continue;
			break;
		case FILTER_ConstructionMaterials:
			if(!(*cit).IsConstructionMaterial())
				continue;
			break;
		case FILTER_Food:
			if(!(*cit).CanBeEaten())
				continue;
			break;
		case FILTER_Fuel:
			if(!(*cit).IsFuel())
				continue;
			break;
		case FILTER_MedicalSupplies:
			if(!(*cit).IsMedical())
				continue;
			break;
		case FILTER_NaturalResources:
			if(!(*cit).IsNaturalResource())
				continue;
			break;
		case FILTER_Patents:
			if(!(*cit).IsPatent())
				continue;
			break;
		case FILTER_SpacecraftDesigns:
			if(!(*cit).IsScDesign())
				continue;
			break;
		case FILTER_SpareParts:
			if(!(*cit).IsSparePart())
				continue;
			break;
		case FILTER_Tools:
			if(!(*cit).IsTool())
				continue;
			break;
		case FILTER_Weapons:
			if(!(*cit).IsWeaponByDesign())
				continue;
			break;
		case FILTER_Misc:
			if((*cit).IsAmmo() ||												// FILTER_Ammo
					(*cit).IsArmorHelmetClothingOrHat() ||						// FILTER_ArmorAndClothes
					(*cit).IsBdDesign() ||										// FILTER_BuildingDesigns
					(*cit).IsConstructionMaterial() ||							// FILTER_ConstructionMaterials
					(*cit).CanBeEaten() ||										// FILTER_Food
					(*cit).IsFuel() ||											// FILTER_Fuel
					(*cit).IsMedical() ||										// FILTER_MedicalSupplies
					(*cit).IsNaturalResource() ||								// FILTER_NaturalResources
					(*cit).IsPatent() ||										// FILTER_Patent
					(*cit).IsScDesign() ||										// FILTER_SpacecraftDesigns
					(*cit).IsSparePart() ||										// FILTER_SpareParts
					(*cit).IsTool() ||											// FILTER_Tools
					(*cit).IsWeaponByDesign() ||								// FILTER_Weapons
					(*cit).IsObsolete())										// Obsolete commodities are not misc.
				continue;
			break;
		}

		// Made it through the filter so add the commodity id to the array.
		if(!commodities.contains((COMMODITYID)(*cit).m_id))
			commodities.append((COMMODITYID)(*cit).m_id);
	}
}

//static Used for setting up pirates.
COMMODITYID DbCommodity::RandomFood(ECOROLE ecoRole)
{
	switch(ecoRole)
	{
	case ECO_Carnivore:
		{
			COMMODITYID cidFood[] =
			{
				CID_Animal,
				CID_AnimalMeat,
				CID_Brain,
				CID_Cheese,
				CID_Eggs,
				CID_Fish,
				CID_FishMeat,
				CID_Gills,
				CID_Heart,
				CID_IceCream,
				CID_Lungs,
				CID_Milk,
				CID_Stomach,
			};
			int cidCount = sizeof(cidFood) / sizeof(COMMODITYID);
			return cidFood[g_Au.Dice().Roll(1,cidCount,-1)];
		}
		break;
	case ECO_Herbivore:
		{
			COMMODITYID cidFood[] =
			{
				CID_Beans,
				CID_Bread,
				CID_Candy,
				CID_Flour,
				CID_Fruit,
				CID_Grain,
				CID_Grapes,
				CID_Hay,
				CID_Herbs,
				CID_Hops,
				CID_Milk,
				CID_IceCream,
				CID_Nuts,
				CID_Pastry,
				CID_Silage,
				CID_Spices,
				CID_Sugar,
				CID_Vegetable,
			};
			int cidCount = sizeof(cidFood) / sizeof(COMMODITYID);
			return cidFood[g_Au.Dice().Roll(1,cidCount,-1)];
		}
		break;
	case ECO_Omnivore:
		{
			COMMODITYID cidFood[] =
			{
				CID_Animal,
				CID_AnimalMeat,
				CID_Beans,
				CID_Brain,
				CID_Bread,
				CID_Candy,
				CID_Cheese,
				CID_Eggs,
				CID_Fish,
				CID_FishMeat,
				CID_Flour,
				CID_Fruit,
				CID_Gills,
				CID_Grain,
				CID_Grapes,
				CID_Heart,
				CID_Herbs,
				CID_IceCream,
				CID_Lungs,
				CID_Milk,
				CID_Nuts,
				CID_Pastry,
				CID_Spices,
				CID_Stomach,
				CID_Sugar,
				CID_Vegetable,
			};
			int cidCount = sizeof(cidFood) / sizeof(COMMODITYID);
			return cidFood[g_Au.Dice().Roll(1,cidCount,-1)];
		}
		break;
	case ECO_Scavenger:
		{
			COMMODITYID cidFood[] =
			{
				CID_Animal,
				CID_AnimalMeat,
				CID_Bone,
				CID_Brain,
				CID_Cheese,
				CID_Eggs,
				CID_Fish,
				CID_FishMeat,
				CID_Gills,
				CID_Heart,
				CID_IceCream,
				CID_Leather,
				CID_Lungs,
				CID_Milk,
				CID_Stomach,
			};
			int cidCount = sizeof(cidFood) / sizeof(COMMODITYID);
			return cidFood[g_Au.Dice().Roll(1,cidCount,-1)];
		}
		break;
	}
	return CID_Invalid;
}

// Performs a random test to determine failure of a single item due to quality.
// Returns true if the commodity did not fail.
// Returns false if the commodity failed.
//static
bool DbCommodity::TestQuality(quint8 quality)
{
	int lSuccess = 3500 + quality*2;							// Success rate out of 4010.
	int lRoll = g_Au.Dice().Roll(1,4010);
	return lRoll < lSuccess;
}

// Calculates the failure rate of a certain quantity of this commodity, based on quality.
// Returns the number of units of 'count' that do NOT fail due to quality failure rate.
// The number returned will never exceed 'count'.
//static
quint32 DbCommodity::TestFailure(quint8 quality, quint32 count)
{
	// Perfect quality items never fail.
	if(quality >= 255)
		return count;

	quint32 lSuccess = 3500 + quality*2;		// Success rate out of 4010.
	quint32 lRoll = g_Au.Dice().Roll(1,4010);

	// When testing only one item, use the random TestQuality check.
	if(count == 1)
		return (lRoll < lSuccess)?1:0;		// Same as TestQuality.

	// Multiple items are calculated according to expected failure rate.
	// Avoid 32 bit rollover when count is very large. It is about to be multipled by 4010 (or less), which is the max lSuccess.
	quint32 successCount;
	if(count <= 0xffffffff / 4010)
		successCount = count * lSuccess / 4010;	// count would not roll over 0xffffffff if multipled by max lSuccess.
	else
		successCount = count / 4010 * lSuccess;	// count would roll over 0xffffffff if multipled by max lSuccess.

	// We add one random test at the end, for the fractional cases.
	// This prevents low quality materials in low amounts from always producing 0.
	// As quantity increases, the impact of the random +1 is reduced, allowing a closer approximation of the statistical
	// failure rate.
	if(successCount < count && lRoll <= lSuccess)
		successCount++;
	return (quint32)successCount;
}

bool DbCommodity::IsInvalid(COMMODITYID cid)
{
	return cid == CID_Invalid;
}

//static
bool DbCommodity::IsPatent(COMMODITYID cid)
{
	switch(cid)
	{
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
		return true;
	default:
		break;
	}
	return false;
}

//static
bool DbCommodity::IsScPatent(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_PatentMakingAtmosphereCondenserBayModule:
	case CID_PatentMakingBallisticWeaponBayModule:
	case CID_PatentMakingBallisticWeaponTurretModule:
	case CID_PatentMakingCapacitorModule:
	case CID_PatentMakingCloakingShieldModule:
	case CID_PatentMakingCommandAndControlModule:
	case CID_PatentMakingCyberneticsMedicalModule:
	case CID_PatentMakingEnergyShieldModule:
	case CID_PatentMakingEnergyWeaponBayModule:
	case CID_PatentMakingEnergyWeaponTurretModule:
	case CID_PatentMakingGeneticsMedicalModule:
	case CID_PatentMakingGravityDriveModule:
	case CID_PatentMakingKineticShieldModule:
	case CID_PatentMakingLifeSupportGravityModule:
	case CID_PatentMakingLifeSupportModule:
	case CID_PatentMakingParticleCollectorBayModule:
	case CID_PatentMakingPersonnelTransporterModule:
	case CID_PatentMakingHydrogenPowerPlantModule:
	case CID_PatentMakingProspectorBayModule:
	case CID_PatentMakingRocketDriveModule:
	case CID_PatentMakingSensorModule:
	case CID_PatentMakingSiphonBayModule:
	case CID_PatentMakingSurgeryMedicalModule:
	case CID_PatentMakingTractorBeamBayModule:
	case CID_PatentMakingVehicleTransporterModule:
	case CID_PatentMakingWarp1FTLDriveModule:
	case CID_PatentMakingWarp2FTLDriveModule:
	case CID_PatentMakingWarp3FTLDriveModule:
	case CID_PatentMakingWarp4FTLDriveModule:
	case CID_PatentMakingWarp5FTLDriveModule:
	case CID_PatentMakingWarp6FTLDriveModule:
	case CID_PatentMakingWarp7FTLDriveModule:
	case CID_PatentMakingWarp8FTLDriveModule:
	case CID_PatentMakingWarp9FTLDriveModule:
	case CID_PatentMakingWeaponDeploymentBayModule:
	case CID_PatentMakingWeaponLauncherBayModule:
	case CID_PatentMakingWeaponLauncherTurretModule:
	case CID_PatentMakingWildcatterBayModule:
	case CID_PatentMakingWormholePFTLDriveModule:
	case CID_PatentMakingWormholePNFTLDriveModule:
	case CID_PatentMakingWormholePNNFTLDriveModule:
		return true;
	default:
		break;
	}
	return false;
}

//static
bool DbCommodity::IsObsolete(COMMODITYID cid)
{
	return
		cid == CID_WoodHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_LumberHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_StoneHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_MetalHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_MagmiumHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_VulciumHullModule ||					// Hull modules are conceptual. They do not actually exist.
		cid == CID_AdamantineHullModule ||				// Hull modules are conceptual. They do not actually exist.
		cid == CID_CommercialShip ||	// Never implemented
		cid == CID_MineMachinery ||		// Never implemented
		cid == CID_NavalShip ||			// Never implemented
		cid == CID_Tank;				// Never implemented
}

//static
bool DbCommodity::IsStorageLiquid(COMMODITYID cid)
{
	switch(cid)
	{
	default:
		return false;
	case CID_Air:
	case CID_Beer:
	case CID_CrudeOil:
	case CID_Gasoline:
	case CID_Hydrogen:
	case CID_Milk:
	case CID_NaturalGas:
	case CID_Petrochemicals:
	case CID_Water:
	case CID_Whiskey:
	case CID_Wine:
	case CID_NitrateExplosive:
	case CID_Vodka:
	case CID_Gin:
	case CID_Antitoxin:
	case CID_Rum:
	case CID_Tequila:
	case CID_Ioplasma:
	case CID_Phlogiston:
	case CID_Cryozine:
	case CID_Myrathane:
	case CID_Polytaride:
	case CID_Myrasplicer:
	case CID_BorexinoPrecipitate:
	case CID_Viathol:
	case CID_Flomentum:
		return true;
	}
}

//static
bool DbCommodity::HitMakesExplosionSound(COMMODITYID cid)
{
	return IsFiredFromHeavyWeapon(cid) ||
		IsEnergyWeapon(cid) ||
		IsDeployableWeapon(cid) ||
		IsRocket(cid) ||
		cid == CID_Dynamite ||
		cid == CID_Hydrogen ||
		IsMissile(cid);// ||
		//IsVehicle(cid);
}

//static
bool DbCommodity::CanCombine(COMMODITYID cid)
{
	switch(cid)
	{
	default:
		return true;
	case CID_Invalid:
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
	case CID_Atmosphere:
	case CID_Hydrographics:
	case CID_Vegetation:
	case CID_Sunlight:
	case CID_Tank:
	case CID_APC:
	case CID_SailBoat:
	case CID_DirtBike:
	case CID_SupercooledLiquid:
	case CID_InfernoAtmosphere:
	case CID_FrigidAtmosphere:
	case CID_AdvSpaceFighter:
	case CID_MissionDisk:
	case CID_DNASample:
	case CID_OfficerLog:
	case CID_Seed:
	case CID_Head:
	case CID_StoneFirePit:
	case CID_BurningBarrel:
	case CID_ScDesignDrawing:
	case CID_ScDesignDisk:
	case CID_ContainerTEU:
	case CID_StorageCrystal:
	case CID_ThumbDrive:
	case CID_BraceletThumbDrive:
	case CID_BraceletStorageCrystal:
	case CID_NecklaceThumbDrive:
	case CID_NecklaceStorageCrystal:
	case CID_RingThumbDrive:
	case CID_RingStorageCrystal:
	case CID_BdDesignDrawing:
	case CID_BdDesignDisk:
	case CID_SpacecraftTitle:
	case CID_Count:
		return false;
	}
}

bool DbCommodity::IsMilitaryHQItem() const
{
	switch(m_id)
	{
	default:
		break;	// Not a military hq item.
	// Construction materials are needed because base repairs work from community supplies.
	case CID_Adamantine:
	case CID_Lumber:
	case CID_Magmium:
	case CID_Metal:
	case CID_Stone:
	case CID_Vulcium:
	case CID_Wood:
		return true;
	// The philosophy behind these choices was to import things needed by personnel. Weapon systems import their own ammo. Air is
	// imported when needed, separately.
	case CID_AK47:
	case CID_AnimalMeat:
	case CID_Antidote:
	case CID_Antitoxin:
	case CID_Backpack:
	case CID_Beans:
	case CID_Beer:
	case CID_RedBinder:
	case CID_GreenBinder:
	case CID_BlueBinder:
	case CID_BlackUniform:
	case CID_BlankDisk:
	case CID_BlueUniform:
	case CID_Bread:
	case CID_BrownUniform:
	case CID_Candy:
	case CID_CapriHalterOutfit:
	case CID_Cheese:
	case CID_Dynamite:
	case CID_Eggs:
	case CID_FishMeat:
	case CID_Flag:
	case CID_Flour:
	case CID_Fruit:
	case CID_Gigacell:
	case CID_Gin:
	case CID_GoldUniform:
	case CID_Grain:
	case CID_Grapes:
	case CID_GravPack:
	case CID_GreenUniform:
	case CID_GreyBDU:
	case CID_GreyUniform:
	case CID_Herbs:
	case CID_IceCream:
	case CID_JeansAndShirt:
	case CID_KhakiBDU:
	case CID_Knapsack:
	case CID_Knife:
	case CID_Lantern:
	case CID_LaserPistol:
	case CID_LaserRifle:
	case CID_LeatherBodyArmor:
	case CID_LeatherClothes:
	case CID_LeatherHelmet:
	case CID_MagmiumKnife:
	case CID_MagmiumPowerArmor:
	case CID_MagmiumPowerArmorHelmet:
	case CID_MedicalKit:
	case CID_MetalBodyArmor:
	case CID_MetalHelmet:
	case CID_MetalPowerArmor:
	case CID_MetalPowerArmorHelmet:
	case CID_Milk:
	case CID_Nuts:
	case CID_OliveDrabBDU:
	case CID_OrangeUniform:
	case CID_Paper:
	case CID_Pastry:
	case CID_Pharmaceuticals:
	case CID_PinkUniform:
	case CID_Pistol:
	case CID_PistolAmmo:
	case CID_PlasticBodyArmor:
	case CID_PlasticExplosive:
	case CID_PlasticHelmet:
	case CID_RedUniform:
	case CID_Rifle:
	case CID_RifleAmmo:
	case CID_Rum:
	case CID_Shotgun:
	case CID_ShotgunAmmo:
	case CID_Spices:
	case CID_Sugar:
	case CID_TealUniform:
	case CID_Tequila:
	case CID_UtilityPouch:
	case CID_VacSuit:
	case CID_VacSuitHelmet:
	case CID_Vegetable:
	case CID_Vodka:
	case CID_VulciumKnife:
	case CID_VulciumPowerArmor:
	case CID_VulciumPowerArmorHelmet:
	case CID_Water:
	case CID_Whiskey:
	case CID_WhiteUniform:
	case CID_Wine:
	case CID_WovenBodyArmor:
	case CID_WovenHelmet:
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
		return true;
	}
	return false;
}

bool DbCommodity::IsMilitaryAirBaseItem() const
{
	switch(m_id)
	{
	default:
		return false;
	// Construction materials are needed because base repairs work from community supplies.
	case CID_Adamantine:
	case CID_Lumber:
	case CID_Magmium:
	case CID_Metal:
	case CID_Stone:
	case CID_Vulcium:
	case CID_Wood:
		return true;
	// EV suits.
	case CID_VacSuit:
	case CID_VacSuitHelmet:
	case CID_MetalPowerArmor:
	case CID_MetalPowerArmorHelmet:
	case CID_MagmiumPowerArmor:
	case CID_MagmiumPowerArmorHelmet:
	case CID_VulciumPowerArmor:
	case CID_VulciumPowerArmorHelmet:
		return true;
	// Fuel and weapons.
	case CID_Hydrogen:
	case CID_Gasoline:
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
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
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
		return true;
	}
}

bool DbCommodity::IsMilitaryDockItem() const
{
	switch(m_id)
	{
	default:
		return false;
	// Construction materials are needed because base repairs work from community supplies.
	case CID_Adamantine:
	case CID_Lumber:
	case CID_Magmium:
	case CID_Metal:
	case CID_Stone:
	case CID_Vulcium:
	case CID_Wood:
		return true;
	case CID_Gasoline:
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
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
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		return true;
	}
}

bool DbCommodity::IsMilitaryMotorPoolItem() const
{
	switch(m_id)
	{
	default:
		return false;
	case CID_Gasoline:
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
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
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		return true;
	}
}

//static
bool DbCommodity::IsPreon(COMMODITYID cid)
{
	switch(cid)
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
		return true;
	}
}

bool DbCommodity::IsHospitalItem() const
{
	switch(m_id)
	{
	default:
		return false;
	// Surgery Station Supplies
	case CID_Antidote:			// poison
	case CID_Antitoxin:			// disease
	case CID_MedicalKit:		// heal
	case CID_Water:				// decontaminate or extinguish
		return true;
	// Pharmacy?
	case CID_Pharmaceuticals:
		return true;
	// Four Food Groups
		// Milk Group
	case CID_Milk:
	case CID_Cheese:
	case CID_IceCream:
		return true;
		// Meat Group
	case CID_AnimalMeat:
	case CID_FishMeat:
	case CID_Eggs:
		return true;
		// Fruits and Vegetables Group
	case CID_Fruit:
	case CID_Vegetable:
	case CID_Grapes:
		return true;
		// Breads and Cereals Group
	case CID_Bread:
	case CID_Pastry:
		return true;
	}
}

//static
int DbCommodity::AimAxis(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_Head:
	case CID_LaserRifle:
	case CID_LaserPistol:
		// These models are created aiming down the positive Y axis, with Z up.
		return 1;

	default:
		// Older models were created with positive Z up, aiming down the positive X axis.
		break;
	}
	return 0;
}

//static
AuPoint DbCommodity::WeaponMuzzleLoc(COMMODITYID cid)
{
	switch(cid)
	{
	default:
		break;
	case CID_HeavyWeapon:
		// The CID_HeavyWeapon model is 7.00207' from the pivot point to the end of the barrel.
		return AuPoint(6.5,0);  // start effect just inside barrel end
	case CID_AK47:
		return AuPoint(2.18,0,.335);
	case CID_Rifle:
		return AuPoint(2.18,0,.335);
	case CID_Shotgun:
		return AuPoint(2.61,0,.37);
	case CID_Pistol:
	case CID_LaserPistol:
		return AuPoint(.464,0,.251);
	case CID_LaserRifle:
		return AuPoint(1.8388,0,.128);
	}
	return AuPoint(0,0);
}

//static
bool DbCommodity::IsSparePart(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_Adamantine:
	case CID_AtmosphereCondenserBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_BallisticWeaponTurretModule:
	case CID_CapacitorModule:
	case CID_CloakingShieldModule:
	case CID_CommandAndControlModule:
	case CID_CyberneticsMedicalModule:
	case CID_ElectronicPart:	// vehicle landing system
	case CID_EnergyShieldModule:
	case CID_EnergyWeaponBayModule:
	case CID_EnergyWeaponTurretModule:
	case CID_GeneticsMedicalModule:
	case CID_GravityDriveModule:
	case CID_HydrogenPowerPlantModule:
	//case CID_IoTube:
	case CID_KineticShieldModule:
	case CID_LifeSupportGravityModule:
	case CID_LifeSupportModule:
	case CID_Magmium:
	case CID_MechanicalPart:	// vehicle launcher
	case CID_Metal:
	case CID_ParticleCollectorBayModule:
	case CID_PersonnelTransporterModule:
	//case CID_Plastic:
	case CID_ProspectorBayModule:
	case CID_RocketDriveModule:
	case CID_SensorModule:
	case CID_SiphonBayModule:
	case CID_SurgeryMedicalModule:
	case CID_TractorBeamBayModule:
	case CID_VehicleTransporterModule:
	case CID_Vulcium:
	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
	case CID_WeaponDeploymentBayModule:
	case CID_WeaponLauncherBayModule:
	case CID_WeaponLauncherTurretModule:
	case CID_WildcatterBayModule:
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
		return true;
	default:
		return false;
	}
}

//static
bool DbCommodity::IsModule(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_AtmosphereCondenserBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_BallisticWeaponTurretModule:
	case CID_CapacitorModule:
	case CID_CloakingShieldModule:
	case CID_CommandAndControlModule:
	case CID_CyberneticsMedicalModule:
	case CID_EnergyShieldModule:
	case CID_EnergyWeaponBayModule:
	case CID_EnergyWeaponTurretModule:
	case CID_GeneticsMedicalModule:
	case CID_GravityDriveModule:
	case CID_HydrogenPowerPlantModule:
	case CID_KineticShieldModule:
	case CID_LifeSupportGravityModule:
	case CID_LifeSupportModule:
	case CID_ParticleCollectorBayModule:
	case CID_PersonnelTransporterModule:
	case CID_ProspectorBayModule:
	case CID_RocketDriveModule:
	case CID_SensorModule:
	case CID_SiphonBayModule:
	case CID_SurgeryMedicalModule:
	case CID_TractorBeamBayModule:
	case CID_VehicleTransporterModule:
	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
	case CID_WeaponDeploymentBayModule:
	case CID_WeaponLauncherBayModule:
	case CID_WeaponLauncherTurretModule:
	case CID_WildcatterBayModule:
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
		return true;
	default:
		return false;
	}
}

//static
quint8 DbCommodity::DataStorageSpace(COMMODITYID storageMediaCID, quint8 quality)
{
	switch(storageMediaCID)
	{
	// Computer is the most primitive storage device.
	case CID_Computer:
		return 1 + quality / 8;	// 1..33
	
	// Thumb Drives
	// Min is based on prior fixed capacity that was not modulated by quality.
	case CID_RingThumbDrive:
		return 10 + quality / 8;	// 10 to 42
	case CID_NecklaceThumbDrive:
		return 16 + quality / 8;	// 16 to 48
	case CID_ThumbDrive:
	case CID_BraceletThumbDrive:
		return 20 + quality / 8;	// 20 to 52
	
	// Storage Crystals
	// Min is based on prior fixed capacity that was not modulated by quality.
	case CID_RingStorageCrystal:
		return 25 + quality / 4;	// 25 to 89
	case CID_NecklaceStorageCrystal:
		return 40 + quality / 4;	// 40 to 104
	case CID_BraceletStorageCrystal:
	case CID_StorageCrystal:
		return 50 + quality / 4;	// 50 to 114

	default:
		break;
	}
	return 0;
}

