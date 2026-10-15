//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dgdata.cpp,v 1.600 2022/05/05 16:24:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dgdata.h"
//
#include "aubddesign.h"
#include "aubddesignexchange.h"
#include "aubdreport.h"
#include "aucity.h"
#include "audice.h"
#include "audna.h"
#include "auexchangerating.h"
#include "aufilever.h"
#include "auitdesign.h"
#include "auitem.h"
#include "auknownpatents.h"
#include "auscdesign.h"
#include "auscdesignexchange.h"
#include "auscreport.h"
#include "auzoneresource.h"
#include "deprocesscomponent.h"
#include <QComboBox>
#include <QDir>
#include <QIcon>
#include <QPixmap>
#include <QStringList>

#define GAME_DATA_VER					0x08					// GD file version.
#define SOLO_INIT_VER					0x01					// GS file version.

DgData::DgData()
{
	m_AccountType = ACCT_Active;
	m_ActiveUnit = 0;
	m_confirmationNumber = 0;
	m_DataReceived = false;
	m_idPlayer = 0;		// id of client g_Player
	for(int iu=0; iu<6; iu++)
	{
		m_UnitId[iu] = AuId(0);
		m_UnitDNA[iu] = 0;
	}
	m_Contacts.clear();

	m_BdDesigns = new AuBdDesignHash;
	m_BdExchange = new AuBdDesignExchange;
	m_BdExchangeRatings = new AuExchangeBlueprintRatings;
	m_ItDesigns = new AuItDesignHash;
	m_ScDesigns = new AuScDesignHash;
	m_ScExchange = new AuScDesignExchange;
	m_ScExchangeRatings = new AuExchangeBlueprintRatings;
}

DgData::~DgData()
{
	delete m_BdDesigns;
	delete m_BdExchange;
	delete m_BdExchangeRatings;
	delete m_ItDesigns;
	delete m_ScDesigns;
	delete m_ScExchange;
	delete m_ScExchangeRatings;
	for(int iu=0; iu<6; iu++)
		if(m_UnitDNA[iu])
			delete m_UnitDNA[iu];
}

DgData::DgData(const DgData& c) :
	m_bIndustryArray(c.m_bIndustryArray),
	m_bManufacturingComponentArray(c.m_bManufacturingComponentArray),
	m_bManufacturingProcessArray(c.m_bManufacturingProcessArray),
	m_gEmpires(c.m_gEmpires),
	m_Species(c.m_Species)
{
	m_AccountExpiry = c.m_AccountExpiry;
	m_AccountType = c.m_AccountType;
	m_ActiveUnit = c.m_ActiveUnit;
	m_confirmationNumber = c.m_confirmationNumber;
	m_DataReceived = c.m_DataReceived;
	m_idPlayer = c.m_idPlayer;
	for(int iu=0; iu<6; iu++)
	{
		m_UnitArmor[iu] = c.m_UnitArmor[iu];
		m_UnitClothes[iu] = c.m_UnitClothes[iu];
		if(c.m_UnitDNA[iu])
			m_UnitDNA[iu] = c.m_UnitDNA[iu]->Clone();
		else
			m_UnitDNA[iu] = 0;
		m_UnitChromatid[iu] = c.m_UnitChromatid[iu];
		m_UnitHair[iu] = c.m_UnitHair[iu];
		m_UnitHand[iu] = c.m_UnitHand[iu];
		m_UnitHat[iu] = c.m_UnitHat[iu];
		m_UnitId[iu] = c.m_UnitId[iu];
		m_UnitName[iu] = c.m_UnitName[iu];
		m_UnitShave[iu] = c.m_UnitShave[iu];
		m_UnitVisor[iu] = c.m_UnitVisor[iu];
	}
	m_AcctName = c.m_AcctName;
	m_AcctPassCrypt = c.m_AcctPassCrypt;
	m_Contacts.clear();
	m_Contacts.append(c.m_Contacts);
	m_BdDesigns = new AuBdDesignHash(*c.m_BdDesigns);
	m_BdExchange = new AuBdDesignExchange(*c.m_BdExchange);
	m_ItDesigns = new AuItDesignHash(*c.m_ItDesigns);
	m_ScDesigns = new AuScDesignHash(*c.m_ScDesigns);
	m_ScExchange = new AuScDesignExchange(*c.m_ScExchange);
}

void DgData::Load()
{
	g_Au.m_CommodityArray.Load();
	m_bIndustryArray.Load();
	m_bManufacturingComponentArray.Load();
	m_bManufacturingProcessArray.Load();
	m_gEmpires.Load();
	//m_ItDesigns->Load();
	//m_ScDesigns->Load();
	//m_ScExchange->Load();
	m_Species.Load();

	// Calculate base costs of commodities based on manufacturing processes.
	CalcCommodityBaseCosts();

	// Calculate values used to determine base hit points per level of all industries.
	CalcIndustryHitPointValues();
}

float DgData::ManufacturingCost(COMMODITYID idCommodity) const
{
	float cost = m_bManufacturingProcessArray.Cost(idCommodity, *this);
	return cost;
}

bool DgData::DefaultWorkerEnabled(quint16 mpid) const
{
	for(DbManufacturingProcessArray::const_iterator mpi = m_bManufacturingProcessArray.cbegin();
			mpi != m_bManufacturingProcessArray.cend(); mpi++)
		if((*mpi).m_id == mpid)
		{
			// Found the manufacturing process.
			if((*mpi).m_idi == IID_AirportRepairShop)
				return false;									// Repair shops are off by default; they only make modules.
			// Patent processes are off by default.
			return DbCommodity::IsPatent((COMMODITYID)(*mpi).m_idc) ? false : true;
		}
	return false;	// Unrecognized manufacturing process id is off by default.
}

bool DgData::ManufacturingProcess(quint16 mpid, DbManufacturingProcess& bMan) const
{
	for(DbManufacturingProcessArray::const_iterator it = m_bManufacturingProcessArray.cbegin();
			it != m_bManufacturingProcessArray.cend(); ++it)
		if((*it).m_id == mpid)
		{
			bMan = *it;
			return true;
		}
	return false;
}

INDUSTRYID DgData::ManufacturingProcessIndustry(int mpid) const
{
	if(mpid < 0)
		return IID_SpacecraftFactory;
	for(DbManufacturingProcessArray::const_iterator it = m_bManufacturingProcessArray.cbegin();
			it != m_bManufacturingProcessArray.cend(); ++it)
		if((*it).m_id == mpid)
			return (INDUSTRYID)(*it).m_idi;
	return IID_None;
}

quint8 DgData::ManufacturingProcessPatentQ(quint16 mpid, const AuKnownPatents& patents) const
{
	return m_bManufacturingComponentArray.PatentQ(mpid, patents);
}

bool DgData::IsManufacturingProcessNatural(quint16 mpid) const
{
	DbManufacturingProcessArray::const_iterator it;
	for(it = m_bManufacturingProcessArray.cbegin();
			it != m_bManufacturingProcessArray.cend(); it++)
	{
		if((*it).m_id == mpid)
			return (*it).m_nat;
	}
	return false;
}

INDUSTRYID DgData::IndustryId(const QString& name) const
{
	DbIndustryArray::const_iterator it;
	for(it = m_bIndustryArray.cbegin();
			it != m_bIndustryArray.cend(); ++it)
		if((*it).m_id != IID_None)
			if((*it).m_name.compare(name, Qt::CaseInsensitive) == 0)
				return (INDUSTRYID)(*it).m_id;
	return IID_None;
}

// Return empire id that currently controls a city.
quint32 DgData::CityEmpire(const AuId& cityId) const
{
	for(DgEmpireHash::const_iterator ei = m_gEmpires.cbegin(); ei != m_gEmpires.cend(); ei++)
		if((*ei).second.m_Cities->find(cityId) != (*ei).second.m_Cities->cend())
			return (*ei).second.m_id;
	return 0;
}

bool DgData::RandomEmpireCity(quint32 empireId, AuId& cityId, const QSet<AuId>& exceptIds) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end() && !(*it).second.m_Cities->empty())
	{
		cityId = (*it).second.RandomCity(exceptIds);
		if(!cityId.IsNull())
			return true;
	}
	return false;
}

quint32 DgData::EmpireCommonOverlord(quint32 empireId1, quint32 empireId2) const
{
	// Climb the vassal hierarchy of empire 1 to make a set of all the empires in the branch. We don't care about the order.
	QSet<quint32> overlords1;			// Set of all empire ids in the overlord hierarchy of empire 1.
	quint32 climbId = empireId1;		// Start at empire 1.
	do
	{
		DgEmpireHash::const_iterator it = m_gEmpires.find(climbId);
		if(it != m_gEmpires.cend())
		{
			overlords1.insert(climbId);	// Remember the climb empire id.
			climbId = (*it).second.Overlord();	// Climb up to the empire's overlord, if any; it could be 0.
			if(overlords1.contains(climbId))
				break;					// Circular hierarchy.
		}
		else
			climbId = 0;				// Climb empire record not found. D:
	}
	while(climbId);

	// Now check the vassal hierarchy of empire 2 looking for any of the overlords of empire 1, which includes the id of empire 1.
	QSet<quint32> overlords2;			// Set of all empire ids in the overlord hierarchy of empire 2.
	climbId = empireId2;				// Start at empire 2.
	do
	{
		DgEmpireHash::const_iterator it = m_gEmpires.find(climbId);
		if(it != m_gEmpires.cend())
		{
			if(overlords1.contains(climbId))
				return climbId;			// Climb empire is the first common overlord above (or including) empire 2.
			overlords2.insert(climbId);	// Remember the climb empire id.
			climbId = (*it).second.Overlord();	// Climb up to the empire's overlord, if any; it could be 0.
			if(overlords2.contains(climbId))
				break;					// Circular hierarchy.
		}
		else
			climbId = 0;				// Climb empire record not found. D:
	}
	while(climbId);

	// We get here if there is no common overlord.
	return 0;
}

bool DgData::EmpireCapital(quint32 empireId, AuId& capitolCity) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Get id of capital city.
		capitolCity = (*it).second.m_capitol;
		return !capitolCity.IsNull();
	}
	return false;
}

bool DgData::EmpireCapital(quint32 empireId, AuId& capitolCity, AuId& capitolCityGalaxy) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Get id of capitol city.
		capitolCity = (*it).second.m_capitol;
		if(capitolCity.IsNull())
			return false;

		// Determine galaxy id.
		AuCityHash::const_iterator ci = (*it).second.m_Cities->find(capitolCity);
		if(ci != (*it).second.m_Cities->cend())
			capitolCityGalaxy = (*ci).second.GalaxyId();

		return true;
	}
	return false;
}

QString DgData::EmpireCityName(quint32 empireId, const AuId& cityId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		AuCityHash::const_iterator ci = (*it).second.m_Cities->find(cityId);
		if(ci != (*it).second.m_Cities->cend())
			return (*ci).second.CityName();
	}
	return "";
}

AuId DgData::EmpireCityGalaxy(quint32 empireId, const AuId& cityId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		AuCityHash::const_iterator ci = (*it).second.m_Cities->find(cityId);
		if(ci != (*it).second.m_Cities->cend())
			return (*ci).second.GalaxyId();
	}
	return AuId(0);
}

bool DgData::EmpireHasCityOnWorld(quint32 empireId, const AuId& worldId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		AuId cityId = (*it).second.CityOnWorld(worldId);
		if(!cityId.IsNull())
			return true;
	}
	return false;
}

bool DgData::EmpireHasCityInSystem(quint32 empireId, const AuId& systemId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		for(AuCityHash::const_iterator ci = (*it).second.m_Cities->cbegin(); ci != (*it).second.m_Cities->cend(); ci++)
			if((*ci).second.SystemId() == systemId)
				return true;
	}
	return false;
}

bool DgData::SetEmpireCityWorkerSpecies(quint32 empireId, const AuId& cityId, const AuSpecies& species)
{
	// We do not change the species key of pirate empires this way.
	if(DgEmpire::IsPirate(empireId))
		return false;
	DgEmpireHash::iterator ei = m_gEmpires.find(empireId);
	if(ei != m_gEmpires.end() && (*ei).second.m_capitol == cityId && (*ei).second.m_capitolSpecies != species)
	{
		// The species of the capitol city changed.
		(*ei).second.m_capitolSpecies = species;
		return true;
	}

	// No change.
	return false;
}

void DgData::SetEmpireCapital(quint32 empireId, const AuId& capitolCity, const AuSpecies& species)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// 'capitolCity' is null when a nu emperor is created in the world. The purpose is to initialize the species for people who
		// join the empire before it has a capitol city..
		(*it).second.m_capitol = capitolCity;
		(*it).second.m_capitolSpecies = species;
	}
}

bool DgData::IsEmpireGalaxyCapital(quint32 empireId, const AuId& cityId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Check for the cityId in the list of cities owned by the empire.
		AuCityHash::const_iterator cit = (*it).second.m_Cities->find(cityId);
		if(cit != (*it).second.m_Cities->cend())
			return (*cit).second.IsGalaxyCapital();
	}
	return false;
}

const AuId& DgData::EmpireGalaxyCapital(quint32 empireId, const AuId& galaxyId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Check for the cityId in the list of cities owned by the empire.
		for(AuCityHash::const_iterator cit = (*it).second.m_Cities->cbegin(); cit != (*it).second.m_Cities->cend(); cit++)
			if((*cit).second.IsGalaxyCapital() && (*cit).second.GalaxyId() == galaxyId)
				return (*cit).second.CityId();		// The city is the capital of the galaxy.
	}
	return AuId::NullId();	// Did not find a capital of the galaxy.
}

const AuId& DgData::EmpireGalaxyCapital(quint32 empireId, GALAXY galaxy) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Check for the cityId in the list of cities owned by the empire.
		for(AuCityHash::const_iterator cit = (*it).second.m_Cities->cbegin(); cit != (*it).second.m_Cities->cend(); cit++)
			if((*cit).second.IsGalaxyCapital() && (*cit).second.Galaxy() == galaxy)
				return (*cit).second.CityId();		// The city is the capital of the galaxy.
	}
	return AuId::NullId();	// Did not find a capital of the galaxy.
}

void DgData::SetEmpireGalaxyCapital(quint32 empireId, const AuId& capitolCity, const AuId& galaxy)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		for(AuCityHash::iterator cit = (*it).second.m_Cities->begin(); cit != (*it).second.m_Cities->end(); cit++)
			if((*cit).second.CityId() == capitolCity)
			{
				// Turn on galaxy capital flag of city.
				(*cit).second.SetGalaxyCapital(true);
			}
			else if((*cit).second.IsGalaxyCapital() && (*cit).second.GalaxyId() == galaxy)
			{
				// Turn off galaxy capital flag of a city that is no longer the capital of the galaxy.
				(*cit).second.SetGalaxyCapital(false);
			}
	}
}

bool DgData::IsEmpireSectorCapital(quint32 empireId, const AuId& cityId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Check for the cityId in the list of cities owned by the empire.
		AuCityHash::const_iterator cit = (*it).second.m_Cities->find(cityId);
		if(cit != (*it).second.m_Cities->cend())
			return (*cit).second.IsSectorCapital();
	}
	return false;
}

const AuId& DgData::EmpireSectorCapital(quint32 empireId, const AuId& sectorId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Check for the cityId in the list of cities owned by the empire.
		for(AuCityHash::const_iterator cit = (*it).second.m_Cities->cbegin(); cit != (*it).second.m_Cities->cend(); cit++)
			if((*cit).second.IsSectorCapital() && (*cit).second.SectorId() == sectorId)
				return (*cit).second.CityId();		// The city is the capital of the sector.
	}
	return AuId::NullId();	// Did not find a capital of the sector.
}

void DgData::SetEmpireSectorCapital(quint32 empireId, const AuId& capitolCity, const AuId& sector)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		for(AuCityHash::iterator cit = (*it).second.m_Cities->begin(); cit != (*it).second.m_Cities->end(); cit++)
			if((*cit).second.CityId() == capitolCity)
			{
				// Turn on sector capital flag of city.
				(*cit).second.SetSectorCapital(true);
			}
			else if((*cit).second.IsSectorCapital() && (*cit).second.SectorId() == sector)
			{
				// Turn off sector capital flag of a city that is no longer the capital of the sector.
				(*cit).second.SetSectorCapital(false);
			}
	}
}

void DgData::SetEmpireCommerce(quint32 empireId, const QByteArray& commerce)
{
	if(!empireId)
		return;
	// Extract the Vector from the byte array.
	QVector<quint8> inCommerce;
	QDataStream in(commerce);
	in.setVersion(QDataStream::Qt_4_1);
	quint32 commerceCount;
	in >> commerceCount;
	for(; commerceCount; commerceCount--)
	{
		quint8 iVal;
		in >> iVal;
		inCommerce.append(iVal);
	}
	while(inCommerce.count() < CID_Count)
		inCommerce.append(EXPOL_ToNeutral);	// Grow commerce array to include nu commodities that have been added.

	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.m_Commerce = inCommerce;
}

QString DgData::EmpireJoinOverlordStatement(quint32 empireId) const
{
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei == m_gEmpires.cend())
		return "";

	return (*ei).second.m_JoinMessage;
}

void DgData::SetEmpireFlag(quint32 empireId, const QImage& flag)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.m_flag = flag;
}

void DgData::SetEmpireName(quint32 empireId, const QString& name, QList<quint32>& chgEmpireIds)
{
	if(EmpireId(name))
	{
		qWarning() << "DgData::SetEmpireName empire named %s already exists. Change ignored!" << name;
		return;
	}

	QString oldName;
	if(!EmpireName(empireId, oldName))
		return;

	chgEmpireIds.append(empireId);

	// Must notify all empire records because of the names stored in their known empires data.
	for(DgEmpireHash::iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
		if((*it).second.ChangeEmpireName(oldName, name))
			chgEmpireIds.append((*it).second.m_id);
}

bool DgData::EmpireFlag(quint32 empireId, QImage& imFlag) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		imFlag = (*it).second.m_flag;
		return true;
	}
	return false;
}

// Returns 0 on failure
quint32 DgData::EmpireId(const QString& name) const
{
	// Look for the empire with that name.
	QString ename = name.trimmed();
	for(DgEmpireHash::const_iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
		if((*it).second.Name() == ename)
			return (*it).second.m_id;													// Found the empire by name.

	// Check for 'Forgotten Empire NN' format.
	QString unkPrefix("Forgotten Empire ");
	if(ename.left(unkPrefix.length()) == unkPrefix)
		return QVariant(ename.mid(unkPrefix.length())).toUInt();				// Found what may be Forgotten Empire NN format.

	// Empire name was not found and it does not look like an 'Forgotten Empire NN' format.
	return 0;
}

bool DgData::EmpireName(quint32 empireId, QString& name) const
{
	// Empire id 0 results in blank return with failure code.
	if(!empireId)
	{
		name = "";
		return false;
	}

	// Locate the empire record using its id.
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		name = (*it).second.Name();
		return true;
	}

	// Return 'Forgotten Empire NN' for empire id that does not have a corresponding empire record.
	name = QString("Forgotten Empire %1").arg(empireId);
	return false;
}

EMPIRE_ROLE DgData::EmpireRole(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return (*it).second.Role();
	return EMPIRE_Player;
}

EXPORT_POLICY DgData::EmpireCommerce(quint32 empireId, COMMODITYID cid) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.CommodityCommerce(cid);

	// Should only get here if the empire id is bogus, or empire 0.
	return EXPOL_ToEnemy;
}

// 'true' means the commodity cannot be exported to the political stance.
// cid = commodity to test restriction. CID_Invalid = Colonists, CID_Sunlight = Spacecraft Repairs,
bool DgData::EmpireCommerceRestricted(quint32 empireId, COMMODITYID cid, POLITICAL_STANCE stance) const
{
	// Commerce is never restricted to members of the same empire.
	if(stance == POL_Empire)
		return false;

	// Determine commerce restriction established for the particular commodity.
	EXPORT_POLICY commerce = EmpireCommerce(empireId, cid);
	switch(stance)
	{
	case POL_Empire:
	case POL_Vassal:
		break;
	case POL_Friend:
		if(commerce == EXPOL_ToNobody)
			return true;	// Commodity cannot be exported to anybody, only to my own empire.
		break;
	case POL_Neutral:
		if(commerce == EXPOL_ToNobody || commerce == EXPOL_ToFriend)
			return true;	// Commodity cannot be exported to neutrals, only to friends and my own empire.
		break;
	case POL_Enemy:
		if(commerce != EXPOL_ToEnemy)
			return true;	// Commodity cannot be exported to enemies.
		break;
	}
	return false;
}

POLITICAL_STANCE DgData::EmpireDefaultPoliticalStance(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.DefaultPoliticalStance();
	return POL_Neutral;	// Should only get here if the empire id is bogus.
}

// Returns true if 'testEmpireId' is known to 'toEmpireId'.
// Returns true if 'toEmpireId' is 0 or invalid.
// Returns true if 'testEmpireId' is 0.
bool DgData::IsEmpireKnown(quint32 toEmpireId, quint32 testEmpireId) const
{
	// Empire always knows itself.
	// Non-empire is always 'known'. It is the empire of non-aligned units such as wild animals.
	if(toEmpireId == testEmpireId || !toEmpireId || !testEmpireId)
		return true;

	// Locate the toEmpireId record.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(toEmpireId);
	if(ei == m_gEmpires.end())
		return true;					// Nonexistent toEmpireId knows about everybody.

	// The testEmpireId record may not exist due to decay, but stances toward them remain in other empire records.
	return (*ei).second.IsEmpireKnown(testEmpireId);
}

// Determines the political stance of 'empireId' toward 'otherEmpireId', whether or not 'otherEmpireId's record still exists.
// Returns POL_Empire if 'empireId' equals 'otherEmpireId'.
// Returns POL_Neutral if 'otherEmpireId' or 'empireId' is 0.
// Returns POL_Neutral if 'empireId's record does not exist.
// Returns 'empireId's stance toward 'otherEmpireId', 'empireId's default stance if 'otherEmpireId' is not known to them.
POLITICAL_STANCE DgData::EmpirePoliticalStance(quint32 empireId, quint32 otherEmpireId) const
{
	// POL_Empire when querying about myself.
	if(empireId == otherEmpireId)
		return POL_Empire;

	// POL_Neutral when querying about a non-empire, such as a wild animal.
	if(!otherEmpireId || !empireId)
		return POL_Neutral;

	// Locate the empireId record to determine its stance toward otherEmpireId.
	// The otherEmpireId record may not exist due to decay, but stances toward them remain in other empire records.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei == m_gEmpires.end())
		return POL_Neutral;				// The empireId record does not exist.

	// Return empireId's stance toward otherEmpireId.
	// Returns empireId's default political stance if otherEmpireId is not known to them.
	return (*ei).second.PoliticalStance(otherEmpireId);
}

float DgData::EmpireIncomeTax(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.IncomeTax();
	return 0;
}

float DgData::EmpireSalesTax(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.SalesTax();
	return 0;
}

float DgData::EmpireSubsidyTax(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.SubsidyTax();
	return 0;
}

QColor DgData::EmpireContrailColor(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		return(*it).second.ContrailColor();

	// Return the default blue.
	QColor color;
	color.setHsvF(240.f / 360.f, .5f, 1);
	return color;
}

void DgData::SetEmpireDefaultPoliticalStance(quint32 empireId, POLITICAL_STANCE stance)
{
	Q_ASSERT(stance != POL_Empire);	// This cannot be a stance toward anothe empire.
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.SetDefaultPoliticalStance(stance);
}

void DgData::SetEmpireRole(quint32 empireId, EMPIRE_ROLE gameRole)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.SetRole(gameRole);
}

// Hue value range is the full range of an unsigned byte.
// Saturation value is limited to the range 64-191.
void DgData::SetEmpireContrailColor(quint32 empireId, quint8 hue, quint8 sat)
{
	if(sat < 64)
		sat = 64;
	else if(sat > 191)
		sat = 191;

	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		(*it).second.m_ContrailHue = hue;
		(*it).second.m_ContrailSat = sat;
	}
}

int DgData::EmpireDossierAddDocument(quint32 empireId, quint32 dosEmpire, const AuDocument& doc)
{
	bool anyChange = false;
	bool declaredWar = false;
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		anyChange = (*it).second.DossierAddDocument(dosEmpire, doc);

		POLITICAL_STANCE es = (*it).second.PoliticalStance(dosEmpire);
		if(es == POL_Neutral)
		{
			// Empire getting the document is neutral toward the empire that caused it.
			// Check war criteria.
			DgEmpireHash::iterator di = m_gEmpires.find(dosEmpire);
			if(di != m_gEmpires.end())
			{
				// Found the other empire.
				// Check to see if we should change our stance toward them to enemy.
				if((*it).second.DossierCheckWarCriteria(dosEmpire))
				{
					// We are declaring war on them.
					POLITICAL_STANCE ds = (*di).second.PoliticalStance(empireId);
					declaredWar = true;

					(*it).second.SetPoliticalStance(dosEmpire, (*di).second.Name(), POL_Enemy, 0, AuDocWar::DocumentType(),
							AuId(), QString(), ds, AuId(), QString(), GALAXY_Count, QString(), QString(), QString());

					if(ds != POL_Enemy)
						(*di).second.SetPoliticalStance(empireId, (*it).second.Name(), POL_Enemy, 0,
								AuDocReciprocalStance::DocumentType(), AuId(), QString(), POL_Enemy, AuId(), QString(),
								GALAXY_Count, QString(), QString(), QString());
				}
			}
		}
	}
	if(declaredWar)
		return 2;
	else if(anyChange)
		return 1;
	return 0;
}

void DgData::SetEmpireDossierChangeState(quint32 empireId, quint32 dosEmpire, const AuTime& docTime, quint8 docType,
		quint16 docIndex, AuDocument::DOC_STATE docState)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.DossierChangeState(dosEmpire, docTime, docType, docIndex, docState);
}

void DgData::SetEmpireAnnouncements(quint32 empireId, const QString& announcements)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.m_Announcements = announcements;
}

void DgData::SetEmpireWarCriteria(quint32 empireId, const AuWarCriteria& crit)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		(*it).second.m_WarCriteria = crit;
}

void DgData::SetEmpireTaxes(quint32 empireId, quint8 income, quint8 sales, quint8 subsidy)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		(*it).second.m_TaxIncome = income;
		(*it).second.m_TaxSales = sales;
		(*it).second.m_TaxSubsidy = subsidy;
	}
}

void DgData::ListAllAvatars(QComboBox& cb) const
{
	cb.clear();
	for(DgEmpireHash::const_iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
		(*it).second.ListAllAvatars(cb);
}

void DgData::CityIds(QSet<AuId>& cityIds) const
{
	for(DgEmpireHash::const_iterator it = m_gEmpires.cbegin(); it != m_gEmpires.cend(); it++)
		for(AuCityHash::const_iterator ci = (*it).second.m_Cities->cbegin(); ci != (*it).second.m_Cities->cend(); ci++)
			cityIds.insert((*ci).first);
}

void DgData::ListAllCities(QComboBox& cb) const
{
	cb.clear();
	for(DgEmpireHash::const_iterator it = m_gEmpires.cbegin(); it != m_gEmpires.cend(); it++)
	{
		for(AuCityHash::const_iterator ci = (*it).second.m_Cities->cbegin(); ci != (*it).second.m_Cities->cend(); ci++)
		{
			QString cname = QString("%1 on %2, %3")
				.arg((*ci).second.CityName())
				.arg((*ci).second.PlanetName())
				.arg((*ci).second.CityId().StringId());
			if(cb.findText(cname) >= 0)
				continue;	// Skip repeats.
			bool inserted = false;
			for(int idx=0; idx<cb.count(); idx++)
			{
				int comp = cname.compare(cb.itemText(idx), Qt::CaseInsensitive);
				if(comp == 0)
					comp = cname.compare(cb.itemText(idx), Qt::CaseSensitive);
				if(comp < 0)
				{
					// Insert the item before the idx item.
					inserted = true;
					QIcon icon((*ci).second.CityIcon());
					cb.insertItem(idx, icon, cname, QVariant((*ci).second.CityId().GetID()));	// Add item with record id.
					break;
				}
			}
			if(!inserted)
			{
				// Append the item at the end.
				QIcon icon((*ci).second.CityIcon());
				cb.addItem(icon, cname, QVariant((*ci).second.CityId().GetID()));	// Add item with record id.
			}
		}
	}
}

void DgData::ListAllEmpires(QComboBox& cb) const
{
	cb.clear();
	for(DgEmpireHash::const_iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
	{
		QString ename = (*it).second.Name();

		bool inserted = false;
		for(int idx=0; idx<cb.count(); idx++)
		{
			int comp = ename.compare(cb.itemText(idx), Qt::CaseInsensitive);
			if(comp == 0)
				comp = ename.compare(cb.itemText(idx), Qt::CaseSensitive);
			if(comp < 0)
			{
				// Insert the item before the idx item.
				inserted = true;
				QPixmap pix = QPixmap::fromImage((*it).second.m_flag);
				QIcon icon = QIcon(pix);
				cb.insertItem(idx, icon, ename, QVariant((*it).second.m_id));	// Add item with record id.
				break;
			}
		}
		if(!inserted)
		{
			// Append the item at the end.
			QPixmap pix = QPixmap::fromImage((*it).second.m_flag);
			QIcon icon = QIcon(pix);
			cb.addItem(icon, ename, QVariant((*it).second.m_id));	// Add item with record id.
		}
	}
}

void DgData::ListEmpiresToJoin(DgEmpireHash& joinableEmpires) const
{
	joinableEmpires.clear();
	DgEmpireHash::const_iterator it;
	for(it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
	{
		// Only list empires that can be joined.
		if(!(*it).second.IsJoinableByNewAvatar())
			continue;

		joinableEmpires.insert(pair<quint32, DgEmpire>((*it).first, (*it).second));
	}
}

void DgData::ListEmpiresToJoin(QComboBox& cb) const
{
	cb.clear();
	DgEmpireHash::const_iterator it;
	for(it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
	{
		// Only list empires that can be joined.
		if(!(*it).second.IsJoinableByNewAvatar())
			continue;

		QString ename = (*it).second.Name();

		bool inserted = false;
		for(int idx=0; idx<cb.count(); idx++)
		{
			int comp = ename.compare(cb.itemText(idx), Qt::CaseInsensitive);
			if(comp == 0)
				comp = ename.compare(cb.itemText(idx), Qt::CaseSensitive);
			if(comp < 0)
			{
				// Insert the item before the idx item.
				inserted = true;
				QPixmap pix = QPixmap::fromImage((*it).second.m_flag);
				QIcon icon = QIcon(pix);
				cb.insertItem(idx, icon, ename, QVariant((*it).second.m_id));	// Add item with record id.
				break;
			}
		}
		if(!inserted)
		{
			// Append the item at the end.
			QPixmap pix = QPixmap::fromImage((*it).second.m_flag);
			QIcon icon = QIcon(pix);
			cb.addItem(icon, ename, QVariant((*it).second.m_id));		// Add item with record id.
		}
	}
}

quint32 DgData::EmpireOverlordJoinCount(const AuId& galaxy) const
{
	// Make a map of empires, sorted by city count in galaxy.
	QMultiMap<quint32, quint32> empiresBySystems;
	for(DgEmpireHash::const_iterator ei = m_gEmpires.cbegin(); ei != m_gEmpires.cend(); ei++)
		if((*ei).second.IsJoinableAsOverlord(galaxy))
		{
			quint32 systems = (*ei).second.SolarSystemsInGalaxy(galaxy);
			if(systems > 1)
				empiresBySystems.insert(systems, (*ei).second.m_id);
		}

	return empiresBySystems.size();
}

void DgData::ListOverlordEmpiresToJoin(QComboBox& cb, const AuId& galaxy) const
{
	// Make a map of empires, sorted by solar system count in galaxy.
	QMultiMap<quint32, quint32> empiresBySystems;
	for(DgEmpireHash::const_iterator ei = m_gEmpires.cbegin(); ei != m_gEmpires.cend(); ei++)
		if((*ei).second.IsJoinableAsOverlord(galaxy))
		{
			quint32 systems = (*ei).second.SolarSystemsInGalaxy(galaxy);
			if(systems > 1)
				empiresBySystems.insert(systems, (*ei).second.m_id);
		}

	// Remove the first items until the map only contains the top ten.
	while(empiresBySystems.size() > 10)
		empiresBySystems.erase(empiresBySystems.begin());

	cb.clear();

	for(QMultiMap<quint32, quint32>::const_iterator oi = empiresBySystems.cbegin(); oi != empiresBySystems.cend(); oi++)
	{
		DgEmpireHash::const_iterator ei = m_gEmpires.find(*oi);
		if(ei == m_gEmpires.cend())
			continue;

		QString ename = QObject::tr("%1 spans %L2 systems.").arg((*ei).second.Name()).arg(oi.key());

		QPixmap pix = QPixmap::fromImage((*ei).second.m_flag);
		QIcon icon = QIcon(pix);
		cb.insertItem(0, icon, ename, QVariant((*ei).second.m_id));
	}
}

void DgData::FindEmpireForPlayerId(const AuId& idPlayer, DgEmpire &empire) const
{
	if(!idPlayer.IsNull())
	{
		bool bFound = false;
		DgEmpireHash::const_iterator it;
		for(it = m_gEmpires.begin(); it != m_gEmpires.end() && !bFound; it++)
		{
			if((*it).second.HasCitizen(idPlayer))
			{
				empire = (*it).second;
				bFound = true;
				break;
			}
		}
	}
}

// Return random empire with open immig pol
quint32 DgData::JoinRandomEmpire(quint32 exceptEmpireId) const
{
	// Count empires that are open to join that have a capitol city.
	QList<quint32> openids;
	DgEmpireHash::const_iterator it;
	for(it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
	{
		if((*it).second.m_id == exceptEmpireId)
			continue;	// skip this empire
		if(!(*it).second.IsPlayerEmpire())
			continue;	// not a player empire
		if((*it).second.JoinRestricted())
			continue;	// join is restricted
		if((*it).second.m_capitol.IsNull())
			continue;	// no capitol city
		if(!(*it).second.HasCity())
			continue;	// no city at all
		openids.append((*it).second.m_id);
	}
	if(openids.empty())
		return 0;	// No empires open to join.
	if(openids.size() == 1)
		return openids.first();
	int randidx = g_Au.Dice().Roll(1, openids.size(), -1);
	return openids[randidx];
}

// Get cEmp rec using id set in cEmp.
bool DgData::Empire(DgEmpire& cEmp) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(cEmp.m_id);
	if(it != m_gEmpires.end())
	{
		cEmp = (*it).second;
		return true;
	}
	return false;
}

void DgData::EmpireCityDecayed(quint32 /*empireId*/, const AuId& idCity)
{
	// Remove the city from all empires, in case it somehow lingers in the data of more than one of them.
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		// Remove the information about the city.
		// This will forget the city was ever a sector capital.
		AuCityHash::iterator cit = (*ei).second.m_Cities->find(idCity);
		if(cit != (*ei).second.m_Cities->end())
		{
			if((*ei).second.IsPirate())
				qDebug() << "DgData::EmpireCityDecayed" << (*ei).first << (*ei).second.Name() << "city" << (*cit).second.CityId()
					<< (*cit).second.CityName();
			// Forget the city was the empire capital.
			if((*cit).second.CityId() == (*ei).second.m_capitol)
				(*ei).second.m_capitol = AuId(0);						// Empire no longer has a capitol city
			(*ei).second.m_Cities->erase(cit);
		}
	}
}

void DgData::EmpireCityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, const AuId& idCity)
{
	// Look for the empire who lost the city.
	DgEmpireHash::iterator it = m_gEmpires.find(oldEmpireId);
	if(it != m_gEmpires.end())
	{
		// Found the empire who lost the city.
		if(nuEmpireId)
		{
			AuCityHash::const_iterator cit = (*it).second.m_Cities->find(idCity);
			if(cit != (*it).second.m_Cities->cend())
				AddCityToEmpire(nuEmpireId,
						(*cit).second.CityType(), (*cit).second.CityId(), (*cit).second.CityName(),
						(*cit).second.FounderId(), (*cit).second.FounderName(),
						(*cit).second.PlanetId(), (*cit).second.PlanetName(),
						(*cit).second.SystemId(), (*cit).second.SystemName(), (*cit).second.SystemLoc(),
						(*cit).second.SectorId(), (*cit).second.SectorName(),
						(*cit).second.GalaxyId(), (*cit).second.Galaxy(), false, false);	// Captured city is not a capitol to me.
		}

		// Remove the city from the old empire.
		// This will forget the city was ever a sector capitol.
		AuCityHash::iterator cit = (*it).second.m_Cities->find(idCity);
		if(cit != (*it).second.m_Cities->end())
		{
			// Forget the city was the empire capitol.
			if((*cit).second.CityId() == (*it).second.m_capitol)
				(*it).second.m_capitol = AuId(0);				// Empire no longer has a capitol city
			(*it).second.m_Cities->erase(cit);
		}
	}
}

void DgData::AddCityToEmpire(quint32 empireId,
		CITYTYPE cityType, const AuId& cityId, const QString& cityName,
		const AuId& founderId, const QString& founderName,
		const AuId& planetId, const QString& planetName,
		const AuId& systemId, const QString& systemName, const AuPoint& systemLoc,
		const AuId& sectorId, const QString& sectorName,
		const AuId& galaxyId, GALAXY galaxy, bool isSectorCapitol, bool isGalaxyCapitol)
{
	AddCityToEmpire(empireId, AuCity(cityType, cityId, cityName, founderId, founderName, planetId, planetName, systemId, systemName,
				systemLoc, sectorId, sectorName, galaxyId, galaxy, isSectorCapitol, isGalaxyCapitol));
}

void DgData::AddCityToEmpire(quint32 empireId, const AuCity& city)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end() && !(*it).second.IsArenaEmpire())
	{
		// Replace the city if it already exists. Sometimes we get called with reconciled/updated data.
		AuCityHash::iterator ci = (*it).second.m_Cities->find(city.CityId());
		if(ci == (*it).second.m_Cities->end())
			(*it).second.m_Cities->insert(pair<AuId, AuCity>(city.CityId(), city));
		else
			(*ci).second = city;
	}
}

void DgData::AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, const AuId& personId, const QString& personName,
		quint8 personGender)
{
	DgEmpireHash::iterator it = m_gEmpires.find(nuEmpireId);
	if(it != m_gEmpires.end() && !(*it).second.IsArenaEmpire())
	{
		if(!(*it).second.HasCitizen(personId))
		{
			// Create a nu citizen entry, with no office.
			AuCitizen citizen;
			citizen.m_id = personId;
			citizen.m_name = personName;
			citizen.m_Gender = personGender;
			(*it).second.m_Citizens.insert(pair<AuId, AuCitizen>(personId, citizen));
			if(g_Au.m_LogMessages & LOG_Governance)
				qDebug("DgData::AddCitizenToEmpire Added player %d to empire %d count=%ld",
						personId.GetID(), (*it).second.m_id, (*it).second.m_Citizens.size());

			// Now remove this person from the their current empire
			if(oldEmpireId && oldEmpireId != nuEmpireId)
				RemoveCitizenFromEmpire(oldEmpireId, personId);
		}
		else if(g_Au.m_LogMessages & LOG_Governance)
			qWarning("DgData::AddCitizenToEmpire Player %d already exists.", personId.GetID());
	}
}

void DgData::RemoveCitizenFromEmpire(quint32 empireId, const AuId& personId)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
	{
		// Only remove the citizen if they exist
		if((*it).second.RemoveCitizen(personId))
		{
			if(g_Au.m_LogMessages & LOG_Governance)
				qDebug("DgData::RemoveCitizenFromEmpire Person=%d removed", personId.GetID());
		}
		else if(g_Au.m_LogMessages & LOG_Error)
			qWarning("DgData::RemoveCitizenFromEmpire Person=%d not found", personId.GetID());
	}
}

void DgData::EmpireAssignOffice(quint32 empireId, const AuId& avatarId, const AuOffice& office)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	// Watch for empire change.
	// Due to the sequence of communications between servers, we usually get the nu empire office assignment before the add
	// citizen to empire notification.
	if(!(*it).second.HasCitizen(avatarId))
	{
		// Locate the empire that currently holds 'avatarId'.
		quint32 oldEmpireId = 0;
		GENDER gender = Male;
		QString name;
		for(DgEmpireHash::const_iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
			if((*ei).second.HasCitizen(avatarId))
			{
				oldEmpireId = (*ei).second.m_id;
				name = (*ei).second.CitizenName(avatarId);
				gender = (*ei).second.CitizenGender(avatarId);
				break;
			}
		if(!oldEmpireId)
			return;

		AddCitizenToEmpire(oldEmpireId, empireId, avatarId, name, gender);
		it = m_gEmpires.find(empireId);
	}

	(*it).second.CitizenAssignOffice(avatarId, office);
}

void DgData::EmpireRevokeOffice(quint32 empireId, const AuId& avatarId)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.CitizenRevokeOffice(avatarId);
}

quint32 DgData::HighestEmpireId() const
{
	quint32 idHighest = 0;
	for(DgEmpireHash::const_iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); ++it)
		if((*it).second.m_id > idHighest)
			idHighest = (*it).second.m_id;
	return idHighest;
}

bool DgData::HasEmpire(quint32 empireId) const
{
	return m_gEmpires.find(empireId) != m_gEmpires.end();
}

// Add/replace an empire record.
void DgData::ReplaceEmpire(const DgEmpire& empire)
{
	// Try to replace existing record, if we've got it.
	DgEmpireHash::iterator it = m_gEmpires.find(empire.m_id);
	if(it != m_gEmpires.end())
	{
		(*it).second = empire;
		return;
	}

	// We get here if the empire record doesn't already exist.
	m_gEmpires.insert(pair<quint32, DgEmpire>(empire.m_id, empire));
}

// Deletes empire record and clears other empire's associations.
void DgData::DeleteEmpire(quint32 empireId, QList<quint32>& chgEmpireIds)
{
	// Remove the empire record from the empires hash.
	DgEmpireHash::iterator rei = m_gEmpires.find(empireId);
	if(rei != m_gEmpires.end())
		m_gEmpires.erase(rei);

	// Remove political stance info from empires who know about empireId.
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator si = (*ei).second.m_Stance.find(empireId);
		if(si != (*ei).second.m_Stance.end())
		{
			// The empire has a stance toward the empire that was deleted.
			(*ei).second.m_Stance.erase(si);
			if(!chgEmpireIds.contains((*ei).second.m_id))
				chgEmpireIds.append((*ei).second.m_id);
		}
	}
}

// Export 'percent' of exporter city's commodities to an importer city.
// Returns true if any commodities were exported.
bool DgData::ExportBrokerItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
		quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
		bool planetCanHaveFlora, bool planetCanHaveFauna) const
{
	// Determine political stance of the exporter to the importer.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(exportEmpire);
	if(ei != m_gEmpires.end())
	{
		// Found the exporting empire record.
		POLITICAL_STANCE exporterStance = EmpirePoliticalStance(exportEmpire, importEmpire);

		// We have the stance toward the importing empire. Export commodities that are permitted.
		return exportInventory.ExportBrokerItemsTo(exporterPopulation, importInventory, percent, storageLimit, exporterStance,
				(*ei).second.m_Commerce, planetCanHaveFlora, planetCanHaveFauna);
	}
	else
	{
		// Did not find the exporting empire record.
		POLITICAL_STANCE exporterStance = POL_Neutral;
		QVector<quint8> exporterCommerce;				// Empty allows export of everything to neutrals.
		return exportInventory.ExportBrokerItemsTo(exporterPopulation, importInventory, percent, storageLimit, exporterStance,
				exporterCommerce, planetCanHaveFlora, planetCanHaveFauna);
	}
}

// Export 'percent' of exporter city's commodities to an importer city.
// Returns true if any commodities were exported.
bool DgData::ExportGroceryItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
		quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
		bool planetCanHaveFlora, bool planetCanHaveFauna) const
{
	// Empire 0 exports nothing.
	if(!exportEmpire)
		return false;
	// Determine political stance of the exporter to the importer.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(exportEmpire);
	if(ei != m_gEmpires.end())
	{
		// Found the exporting empire record.
		POLITICAL_STANCE exporterStance = EmpirePoliticalStance(exportEmpire, importEmpire);

		// We have the stance toward the importing empire. Export commodities that are permitted.
		return exportInventory.ExportGroceryItemsTo(exporterPopulation, importInventory, percent, storageLimit, exporterStance,
				(*ei).second.m_Commerce, planetCanHaveFlora, planetCanHaveFauna);
	}

	// The other empire was not found. This should never happen.
	qWarning("DgData::ExportGroceryItems importEmpire %d exportEmpire %d NOT FOUND!", importEmpire, exportEmpire);
	return false;
}

// Export 'percent' of exporter city's commodities to an importer city.
// Returns true if any commodities were exported.
bool DgData::ExportRetailItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
		quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
		bool planetCanHaveFlora, bool planetCanHaveFauna) const
{
	// Empire 0 exports nothing.
	if(!exportEmpire)
		return false;
	// Determine political stance of the exporter to the importer.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(exportEmpire);
	if(ei != m_gEmpires.end())
	{
		// Found the exporting empire record.
		POLITICAL_STANCE exporterStance = EmpirePoliticalStance(exportEmpire, importEmpire);

		// We have the stance toward the importing empire. Export commodities that are permitted.
		return exportInventory.ExportRetailItemsTo(exporterPopulation, importInventory, percent, storageLimit, exporterStance,
				(*ei).second.m_Commerce, planetCanHaveFlora, planetCanHaveFauna);
	}

	// The other empire was not found. This should never happen.
	qWarning("DgData::ExportRetailItems importEmpire %d exportEmpire %d NOT FOUND!", importEmpire, exportEmpire);
	return false;
}

// Export 'percent' of exporter city's commodities to an importer city.
// Returns true if any commodities were exported.
bool DgData::ExportCantinaItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
		quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
		bool planetCanHaveFlora, bool planetCanHaveFauna) const
{
	// Empire 0 exports nothing.
	if(!exportEmpire)
		return false;
	// Determine political stance of the exporter to the importer.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(exportEmpire);
	if(ei != m_gEmpires.end())
	{
		// Found the exporting empire record.
		POLITICAL_STANCE exporterStance = EmpirePoliticalStance(exportEmpire, importEmpire);

		// We have the stance toward the importing empire. Export commodities that are permitted.
		return exportInventory.ExportCantinaItemsTo(exporterPopulation, importInventory, percent, storageLimit, exporterStance,
				(*ei).second.m_Commerce, planetCanHaveFlora, planetCanHaveFauna);
	}

	// The other empire was not found. This should never happen.
	qWarning("DgData::ExportCantinaItems importEmpire %d exportEmpire %d NOT FOUND!", importEmpire, exportEmpire);
	return false;
}

// Calculate base cost of this commodity.
void DgData::CalcCommodityBaseCost(DbCommodity& bCom)
{
	if(bCom.m_id == CID_Invalid)
		bCom.SetBaseCost(0);			// skip invalid items
	// Base cost of an item is based on the average cost of the manufacturing
	// processes available to produce it.
	else if(AuZoneResource::IsFetchable(bCom.m_id))
		bCom.SetBaseCost(DeProcessComponent::GetLaborNeeded(10) * LABOR_RATE);	// only labor is needed
	else
		bCom.SetBaseCost(ManufacturingCost(bCom.m_id));
}

// Called after loading to calculate costs.
void DgData::CalcCommodityBaseCosts()
{
	int unresolved = 0;
	do
	{
		unresolved = 0;
		for(DbCommodityArray::iterator it = g_Au.m_CommodityArray.begin(); it != g_Au.m_CommodityArray.end(); it++)
		{
			if((*it).BaseCost() < 0)
			{
				CalcCommodityBaseCost(*it);
				if((*it).BaseCost() < 0)
				{
					//qDebug(" %d. %s cost unresolved", (*it).m_id, (const char*)((*it).m_name));
					unresolved++;
				}
			}
		}
		//qDebug("%d unresolved this pass", unresolved);
	}
	while(unresolved);

	/*
	qDebug("---------- Commodity Base Costs ----------");
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); ++it)
		if((*it).m_id != CID_Invalid)
			qDebug(" %3d. %12.2f %s", (*it).m_id, (*it).BaseCost(), (const char*)(*it).m_name.toAscii());
	qDebug("------------------------------------------");
	*/
}

// Calculate values used to determine base hit points per level of all industries.
void DgData::CalcIndustryHitPointValues()
{
	Q_ASSERT(!m_bManufacturingProcessArray.isEmpty());
	Q_ASSERT(!m_bManufacturingComponentArray.isEmpty());
	m_bIndustryArray.CalcPatentsRequired(m_bManufacturingProcessArray, m_bManufacturingComponentArray);
}

AuId DgData::EmpireEmperor(quint32 empireId) const
{
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei == m_gEmpires.end())
		return AuId::NullId();

	return (*ei).second.EmperorId();
}

// Returns true if 'industry' has any manufacturing process to make 'cid'.
bool DgData::DoesIndustryMake(INDUSTRYID industry, COMMODITYID cid) const
{
	return m_bManufacturingProcessArray.DoesIndustryMake(industry, cid);
}

// Returns true if 'industry' has any manufacturing process that uses 'cid'.
bool DgData::DoesIndustryUse(INDUSTRYID industry, COMMODITYID cid) const
{
	return m_bManufacturingProcessArray.DoesIndustryUse(industry, cid, m_bManufacturingComponentArray);
}

void DgData::NameSpecies(const AuSpecies& species, const QString& name, const AuId& whoNamed)
{
	DzSpecies spec(species, name, whoNamed);
	DzSpeciesHash::iterator si = m_Species.find(species);
	if(si != m_Species.end())
		(*si).second = spec;
	else
		m_Species.insert(pair<AuSpecies, DzSpecies>(species, spec));
}

QString DgData::SpeciesName(const AuSpecies& species, bool descIfUnnamed) const
{
	// Check to see if the specie has been named.
	// Note that on the client the name may be blank. This occurs when we have requested a specie from the server. The server
	// responds with an empty specie record if it has not been named, so I won't keep asking for the same one over again. If that
	// specie record changes, the server knows I have it and sends it to me again.
	DzSpeciesHash::const_iterator si = m_Species.find(species);
	if(si != m_Species.end() && !(*si).second.Name().isEmpty())
		return (*si).second.Name();

	// Return a blank string when specie is not named.
	if(descIfUnnamed)
		return species.Description();	// Caller wants the species desription instead of a blank return.
	return "";
}

const AuId& DgData::SpeciesNamedBy(const AuSpecies& species) const
{
	// Check to see if the specie has been named.
	// Note that on the client the id may be null. This occurs when we have requested a specie from the server. The server
	// responds with an empty specie record if it has not been named, so I won't keep asking for the same one over again. If that
	// specie record changes, the server knows I have it and sends it to me again.
	DzSpeciesHash::const_iterator si = m_Species.find(species);
	if(si != m_Species.end())
		return (*si).second.WhoNamed();

	// Return a null id when specie is not named.
	return AuId::NullId();
}

// Add/replace a specie record.
void DgData::ReplaceSpecies(const DzSpecies& species)
{
	// Try to replace existing record, if we've got it.
	DzSpeciesHash::iterator it = m_Species.find(species.Species());
	if(it != m_Species.end())
	{
		(*it).second = species;
		return;
	}

	// We get here if the specie record doesn't already exist.
	m_Species.insert(pair<AuSpecies, DzSpecies>(species.Species(), species));
}

bool DgData::HasSpecies(const AuSpecies& species) const
{
	return m_Species.find(species) != m_Species.end();
}

bool DgData::IsPirateEmpireReady(quint32 empireId) const
{
	if(!DgEmpire::IsPirate(empireId))
		return false;
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei != m_gEmpires.end())
		return (*ei).second.IsPirateReady();
	return false;
}

quint32 DgData::RandomPirateEmpire(GALAXY galaxy) const
{
	return m_gEmpires.RandomPirateEmpire(galaxy);
}

// Returns sentient race species from a random empire.
AuSpecies DgData::RandomEmpireSpecies() const
{
	return m_gEmpires.RandomSpecies();
}

void DgData::SetBasicGameDataReceived()
{
	// Called after the basic game data termination packet is received.
	m_DataReceived = true;

	// Calculate base costs of commodities based on manufacturing processes.
	CalcCommodityBaseCosts();

	// Calculate values used to determine base hit points per level of all industries.
	CalcIndustryHitPointValues();

	SaveBasicGameData();
}

// Saves basic game data to a local cache file for use by stand-alone spacecraft designer.
void DgData::SaveBasicGameData()
{
	// Store basic game data into a local cache file for use by stand-alone spacecraft designer.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath("GD1");
	QFile::remove(cacheFileName);		// Remove the old file, if it already exists.

	// Insure the cache folder exists.
	if(g_Au.CreateCacheGame())
	{
		// The cache folder exists.
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QFile::WriteOnly))
		{
			// Prepare to write to the file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Write a header to the file.
			const quint8 fileType = GAME_DATA_CACHE;
			const quint8 fileVersion = GAME_DATA_VER;
			dsfile << fileType;
			dsfile << fileVersion;

			// Commodities.
			g_Au.m_CommodityArray >> dsfile;

			// Industries
			dsfile << (quint16)m_bIndustryArray.count();
			for(DbIndustryArray::const_iterator ii = m_bIndustryArray.cbegin(); ii != m_bIndustryArray.cend(); ++ii)
				(*ii).Tx(dsfile);

			// Manufacturing processes.
			dsfile << (quint16)m_bManufacturingProcessArray.count();
			for(DbManufacturingProcessArray::const_iterator mpit = m_bManufacturingProcessArray.cbegin();
					mpit != m_bManufacturingProcessArray.cend(); mpit++)
				(*mpit).Tx(dsfile);

			// Manufacturing process components.
			dsfile << (quint32)m_bManufacturingComponentArray.count();
			for(DbManufacturingComponentArray::const_iterator mcit = m_bManufacturingComponentArray.cbegin();
					mcit != m_bManufacturingComponentArray.cend(); mcit++)
				(*mcit).Tx(dsfile);

			// Close the file.
			cacheFile.close();
		}
	}
}

// Loads basic game data from a local cache file for use by stand-alone spacecraft designer.
int DgData::LoadBasicGameData()
{
	// Load basic game data from a local cache file for use by stand-alone spacecraft designer.
	// We prefer to load basic game data from the temporary folder since it could be more up-to-date than the distro file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath("GD1");
	if(g_Au.m_LogMessages & LOG_Database)
		qDebug() << "DgData::LoadBasicGameData on entry cacheFileName" << cacheFileName;
	if(QFile::exists(cacheFileName))
	{
		// Try to open the basic game data file. On failure, the client will query the server for the data and then the client will
		// save the file when SetBasicGameDataReceived is called.
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QIODevice::ReadOnly))
		{
			// Load from cache file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Read the header from the file.
			quint8 fileType;
			quint8 fileVersion;
			dsfile >> fileType;
			dsfile >> fileVersion;
			if(fileType == GAME_DATA_CACHE && fileVersion == GAME_DATA_VER)
			{
				//DHH This code is duplicated below. I did it this way to avoid a lot of building today. It can be tidied up later.
				// Commodities.
				DbCommodityArray commodities;
				commodities << dsfile;
				if(g_Au.m_CommodityArray.isEmpty() || g_Au.m_CommodityArray.count() != commodities.count())
				{
					if(g_Au.m_LogMessages & LOG_Database)
						qDebug() << "DgData::LoadBasicGameData commodities loaded from GD1" << commodities.count();
					g_Au.m_CommodityArray = commodities;
				}

				// Industries
				{
					m_bIndustryArray.clear();
					quint16 cnt;
					dsfile >> cnt;
					while(cnt--)
					{
						DbIndustry itype;
						itype.Rx(dsfile);
						m_bIndustryArray.append(itype);
					}
				}

				// Manufacturing processes.
				{
					m_bManufacturingProcessArray.clear();
					quint16 cnt;
					dsfile >> cnt;
					while(cnt--)
					{
						DbManufacturingProcess mproc;
						mproc.Rx(dsfile);
						m_bManufacturingProcessArray.append(mproc);
					}
				}

				// Manufacturing components.
				{
					m_bManufacturingComponentArray.clear();
					quint32 cnt;
					dsfile >> cnt;
					while(cnt--)
					{
						DbManufacturingComponent mcomp;
						mcomp.Rx(dsfile);
						m_bManufacturingComponentArray.append(mcomp);
					}
				}

				// Close the file.
				cacheFile.close();

				// Calculate base costs of commodities based on manufacturing processes.
				CalcCommodityBaseCosts();

				// Calculate values used to determine base hit points per level of all industries.
				CalcIndustryHitPointValues();

				// Called after the basic game data termination packet is received.
				m_DataReceived = true;
				if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
					g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Data: GD1 loaded"));
				return 0;	// Good return.
			}
			else
			{
				// Close the file.
				cacheFile.close();
			}
		}
	}

	// Load basic game data from distro file.
	cacheFileName = "GD";
	if(g_Au.m_LogMessages & LOG_Database)
		qDebug() << "DgData::LoadBasicGameData trying cacheFileName" << cacheFileName;
	if(!QFile::exists(cacheFileName))
		return 1;

	// Try to open the basic game data file. On failure, the client will query the server for the data and then the client will
	// save the file when SetBasicGameDataReceived is called.
	QFile cacheFile(cacheFileName);
	if(!cacheFile.open(QIODevice::ReadOnly))
		return 2;											// Failed to open cache data file.

	// Load from cache file.
	QDataStream dsfile(&cacheFile);
	dsfile.setVersion(QDataStream::Qt_4_1);

	// Read the header from the file.
	quint8 fileType;
	quint8 fileVersion;
	dsfile >> fileType;
	dsfile >> fileVersion;
	if(fileType != GAME_DATA_CACHE || fileVersion != GAME_DATA_VER)
	{
		cacheFile.close();
		return 3;
	}

	// Commodities.
	DbCommodityArray commodities;
	commodities << dsfile;
	if(g_Au.m_CommodityArray.isEmpty() || g_Au.m_CommodityArray.count() != commodities.count())
	{
		if(g_Au.m_LogMessages & LOG_Database)
			qDebug() << "DgData::LoadBasicGameData commodities loaded from GD" << commodities.count();
		g_Au.m_CommodityArray = commodities;
	}

	// Industries
	{
		m_bIndustryArray.clear();
		quint16 cnt;
		dsfile >> cnt;
		while(cnt--)
		{
			DbIndustry itype;
			itype.Rx(dsfile);
			m_bIndustryArray.append(itype);
		}
	}

	// Manufacturing processes.
	{
		m_bManufacturingProcessArray.clear();
		quint16 cnt;
		dsfile >> cnt;
		while(cnt--)
		{
			DbManufacturingProcess mproc;
			mproc.Rx(dsfile);
			m_bManufacturingProcessArray.append(mproc);
		}
	}

	// Manufacturing components.
	{
		m_bManufacturingComponentArray.clear();
		quint32 cnt;
		dsfile >> cnt;
		while(cnt--)
		{
			DbManufacturingComponent mcomp;
			mcomp.Rx(dsfile);
			m_bManufacturingComponentArray.append(mcomp);
		}
	}

	// Close the file.
	cacheFile.close();

	// Calculate base costs of commodities based on manufacturing processes.
	CalcCommodityBaseCosts();

	// Calculate values used to determine base hit points per level of all industries.
	CalcIndustryHitPointValues();

	// Called after the basic game data termination packet is received.
	m_DataReceived = true;
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Data: GD loaded"));
	return 0;	// Good return
}

// Returns the id of the affected empire.
quint32 DgData::SetEmpireAvatarName(AuId& avatarId, const QString& nuName)
{
	// Locate the empire that currently claims the avatar.
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = (*ei).second.m_Citizens.find(avatarId);
		if(ci != (*ei).second.m_Citizens.end())
		{
			// Found the empire that claims the avatar as a citizen.
			(*ci).second.m_name = nuName;
			return (*ei).second.m_id;
		}
	}
	return 0;
}

// Returns the id of the affected empire.
quint32 DgData::SetEmpireCityName(AuId& cityId, const QString& nuName)
{
	// Locate the empire that currently claims the city.
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		AuCityHash::iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci != (*ei).second.m_Cities->end())
		{
			// Found the empire that claims the city.
			(*ci).second.SetCityName(nuName);
			return (*ei).second.m_id;
		}
	}
	return 0;
}

// Caller MUST insure all cities on the planet are controlled by the same empire.
// Returns the id of the affected empire.
quint32 DgData::SetEmpirePlanetName(AuId& planetId, const QString& nuName)
{
	// Locate the empire that currently claims the planet by having a city there.
	quint32 empireId = 0;
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		// Check all cities of the empire to see if any of them is on the planet.
		for(AuCityHash::iterator ci = (*ei).second.m_Cities->begin(); ci != (*ei).second.m_Cities->end(); ci++)
			if((*ci).second.PlanetId() == planetId)
			{
				// Found the empire that has a city on the planet.
				(*ci).second.SetPlanetName(nuName);
				empireId = (*ei).second.m_id;
			}

		// Caller has insured there are no other empires that have cities on the planet.
		if(empireId)
			return empireId;
	}
	return 0;
}

// 'empireIds' is filled with the list of affected empires.
void DgData::SetEmpireSectorName(AuId& sectorId, const QString& nuName, QList<quint32>& empireIds)
{
	// Locate all empires that currently claim the sector by having a city there.
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		// Check all cities of the empire to see if any of them is in the sector.
		quint32 empireId = 0;
		for(AuCityHash::iterator ci = (*ei).second.m_Cities->begin(); ci != (*ei).second.m_Cities->end(); ci++)
			if((*ci).second.SectorId() == sectorId)
			{
				// Found an empire that has a city in the sector.
				(*ci).second.SetSectorName(nuName);
				empireId = (*ei).second.m_id;
			}

		// Accumulate a list of the empires that are affected by the change.
		if(empireId)
			empireIds.append(empireId);
	}
}

// Caller MUST insure all cities in the system are controlled by the same empire.
// Returns the id of the affected empire.
quint32 DgData::SetEmpireSystemName(AuId& systemId, const QString& nuName)
{
	// Locate the empire that currently claims the system by having a city there.
	quint32 empireId = 0;
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		// Check all cities of the empire to see if any of them is in the system.
		for(AuCityHash::iterator ci = (*ei).second.m_Cities->begin(); ci != (*ei).second.m_Cities->end(); ci++)
			if((*ci).second.SystemId() == systemId)
			{
				// Found the empire that has a city in the system.
				(*ci).second.SetSystemName(nuName);
				empireId = (*ei).second.m_id;
			}

		// Caller has insured there are no other empires that have cities in the system.
		if(empireId)
			return empireId;
	}
	return 0;
}

GENDER DgData::EmpireCitizenGender(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei != m_gEmpires.end())
		return (*ei).second.CitizenGender(avatarId);
	return Male;
}

// Returns true if 'avatarId' holds any government office in 'empireId'.
bool DgData::IsEmpireGovernor(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.IsGovernor(avatarId);
}

// Returns true if 'cityId' is in the governance domain of 'avatarId', in the empire 'empireId'.
bool DgData::IsEmpireCityInDomain(quint32 empireId, const AuId& cityId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;	// Empire does not exist.

	// Check the city.
	if((*it).second.IsPlaceInOfficeDomain(cityId, avatarId))
		return true;

	// Check the chain of places up to the universe.
	AuCityHash::const_iterator ci = (*it).second.m_Cities->find(cityId);
	if(ci == (*it).second.m_Cities->cend())
		return false;	// City does not exist in empire.

	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ai = (*it).second.m_Citizens.find(avatarId);
	if(ai == (*it).second.m_Citizens.end())
		return false;	// Citizen not found in the empire.

	return (*ai).second.IsEmperor() ||
		(*ai).second.IsPlaceInOfficeDomain((*ci).second.PlanetId()) ||
		(*ai).second.IsPlaceInOfficeDomain((*ci).second.SystemId()) ||
		(*ai).second.IsPlaceInOfficeDomain((*ci).second.SectorId()) ||
		(*ai).second.IsPlaceInOfficeDomain((*ci).second.GalaxyId()) ||
		(*ai).second.IsPlaceInOfficeDomain(AuId::Universe());
}

bool DgData::IsEmpirePlaceInDomain(quint32 empireId, const AuIdList& places, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.IsPlaceHierarchyInDomain(places, avatarId);
}

bool DgData::EmpireCitizenCanAcceptCitySurrender(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanAcceptCitySurrender(avatarId);
}

bool DgData::EmpireCitizenCanAccessGovtAccount(quint32 empireId, const AuId& avatarId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanAccessGovtAccount(avatarId, places);
}

// true if a citizen of an empire is a governor who has the power to delegate their power to others.
bool DgData::EmpireCitizenCanDelegate(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanDelegate(avatarId);
}

bool DgData::EmpireCitizenCanInviteCitizens(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanInviteCitizens(avatarId);
}

bool DgData::EmpireCitizenCanKickCitizens(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanKickCitizens(avatarId);
}

bool DgData::EmpireCitizenCanLoadOfficer(quint32 empireId, const AuId& avatarId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanLoadOfficer(avatarId, places);
}

bool DgData::EmpireCitizenCanReceiveFleetMessages(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanReceiveFleetMessages(avatarId);
}

bool DgData::EmpireCitizenCanSendEmpireMail(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSendEmpireMail(avatarId);
}

bool DgData::EmpireCitizenCanManageDossier(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanManageDossier(avatarId);
}

bool DgData::EmpireCitizenCanManageTrade(quint32 empireId, const AuId& avatarId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanManageTrade(avatarId, places);
}

bool DgData::EmpireCitizenCanSetJoinPassword(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetJoinPassword(avatarId);
}

bool DgData::EmpireCitizenCanSetAnnouncements(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetAnnouncements(avatarId);
}

bool DgData::EmpireCitizenCanSetWarCriteria(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetWarCriteria(avatarId);
}

bool DgData::EmpireCitizenCanSetCapital(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetCapital(avatarId);
}

bool DgData::EmpireCitizenCanSetCommerce(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetCommerce(avatarId);
}

bool DgData::EmpireCitizenCanSetContrailColor(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetContrailColor(avatarId);
}

bool DgData::EmpireCitizenCanSetEmpireFlag(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetEmpireFlag(avatarId);
}

bool DgData::EmpireCitizenCanSetColorTheme(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetColorTheme(avatarId);
}

bool DgData::EmpireCitizenCanSetPlaceNames(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetPlaceNames(avatarId);
}

bool DgData::EmpireCitizenCanSetPoliticalStance(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetPoliticalStance(avatarId);
}

bool DgData::EmpireCitizenCanSetGalaxyCapital(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetGalaxyCapital(avatarId);
}

bool DgData::EmpireCitizenCanSetGalaxyCapital(quint32 empireId, const AuId& avatarId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetGalaxyCapital(avatarId, places);
}

bool DgData::EmpireCitizenCanSetSectorCapital(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetSectorCapital(avatarId);
}

bool DgData::EmpireCitizenCanSetSectorCapital(quint32 empireId, const AuId& avatarId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetSectorCapital(avatarId, places);
}

bool DgData::EmpireCitizenCanSetTax(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanSetTax(avatarId);
}

bool DgData::EmpireCitizenCanUseDiplomacyChannel(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenCanUseDiplomacyChannel(avatarId);
}

QString DgData::EmpireHtmlName(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return "";
	return (*it).second.HtmlName();
}

AuId DgData::EmpireCityFounder(quint32 empireId, const AuId& cityId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return AuId::NullId();
	return (*it).second.CityFounder(cityId);
}

AuId DgData::EmpireCityFounderBoss(quint32 empireId, const AuIdList& places) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return AuId::NullId();
	return (*it).second.CityFounderBoss(places);
}

void DgData::EmpireDomainGovernors(quint32 empireId, const AuIdList& places, AuIdList& commanders) const
{
	commanders.clear();

	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.DomainGovernors(places, commanders);
}

void DgData::EmpireDomainOfficerLoaders(quint32 empireId, const AuIdList& places, AuIdList& commanders) const
{
	commanders.clear();

	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.DomainOfficerLoaders(places, commanders);
}

QString DgData::EmpireCitizenTitle(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return "";

	return (*it).second.CitizenTitle(avatarId);
}

void DgData::EmpireReceivesSensorScans(quint32 empireId, const AuIdList& places, AuIdList& avatarIds) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.ReceivesSensorScans(places, avatarIds);
}

// Fills 'avatarIds' with the ids of empire governors who are to receive ship messages.
void DgData::EmpireReceivesFleetMessages(quint32 empireId, const AuIdList& places, AuIdList& avatarIds) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.ReceivesFleetMessages(places, avatarIds);
}

QString DgData::EmpireCitizenName(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return "";

	return (*it).second.CitizenName(avatarId);
}

QString DgData::EmpireCitizenNameWithTitle(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return "";

	return (*it).second.CitizenNameWithTitle(avatarId);
}

AuOffice DgData::EmpireCitizenOffice(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return AuOffice();

	return (*it).second.CitizenOffice(avatarId);
}

// Returns a number that is multipled by an empire's reward for killing enemy units.
// Returns 0 if the citizen does not exist in the empire.
// Returns 1 for ordinary citizens.
// Returns higher number for governors.
quint32 DgData::EmpireCitizenTargetValue(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return 0;

	return (*it).second.CitizenTargetValue(avatarId);
}

// Returns true if 'citizenId' is subordinate to 'avatarId' in the chain-of-command.
// Citizens who are not governors are not subordinate to anybody so return false.
bool DgData::EmpireCitizenInChainOfCommand(quint32 empireId, const AuId& avatarId, const AuId& citizenId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return 0;

	return (*it).second.CitizenInChainOfCommand(avatarId, citizenId);
}

// Filters the settings in 'office' against the permissions of 'avatarId', before assigning those permissions to 'assignee'.
// Returns false if 'avatarId' is not allowed to assign anything.
bool DgData::EmpireCitizenFilterAssignment(quint32 empireId, const AuId& avatarId, const AuId& assigneeId, AuOffice& office) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenFilterAssignment(avatarId, assigneeId, office);
}

// Verifies that no other empire has a city in sectorId and that empireId has at least one city there.
bool DgData::EmpireOkToRenameSector(quint32 empireId, const AuId& sectorId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	// Verify that empireId has a city in the sector.
	if(!(*it).second.HasCityInSector(sectorId))
		return false;

	// Verify that no other empire has a city in the sector.
	for(DgEmpireHash::const_iterator it = m_gEmpires.begin(); it != m_gEmpires.end(); it++)
		if((*it).second.m_id != empireId && (*it).second.HasCityInSector(sectorId))
			return false;

	return true;
}

bool DgData::EmpireChangeBoss(quint32 empireId, const AuId& governorId, const AuId& bossId)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.CitizenChangeBoss(governorId, bossId);
}

AuId DgData::CityOnWorld(const AuId& worldId) const
{
	for(DgEmpireHash::const_iterator it = m_gEmpires.cbegin(); it != m_gEmpires.cend(); it++)
	{
		AuId cityId = (*it).second.CityOnWorld(worldId);
		if(!cityId.IsNull())
			return cityId;
	}
	return AuId();
}

bool DgData::PirateEmperor(quint32 empireId, AuId& emperorId, QString& emperorName) const
{
	if(!DgEmpire::IsPirate(empireId))
		return false;
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	// Pirate empires are assigned a bogus emperor by name.
	if((*it).second.Name() == "Akson")
	{
		emperorId = AuId::AksonEmperor();
		emperorName = "Detria";
	}
	else if((*it).second.Name() == "Balorite")
	{
		emperorId = AuId::BaloriteEmperor();
		emperorName = "Atrakhan";
	}
	else if((*it).second.Name() == "Dendrae")
	{
		emperorId = AuId::DendraeEmperor();
		emperorName = "Martess";
	}
	else if((*it).second.Name() == "Haxu")
	{
		emperorId = AuId::HaxuEmperor();
		emperorName = "Tenracca";
	}
	else if((*it).second.Name() == "Kla'tra")
	{
		emperorId = AuId::KlatraEmperor();
		emperorName = "Putres'p";
	}
	else if((*it).second.Name() == "Malacon")
	{
		emperorId = AuId::MalaconEmperor();
		emperorName = "Ilth";
	}
	else if((*it).second.Name() == "Myntaka")
	{
		emperorId = AuId::MyntakaEmperor();
		emperorName = "Tiver";
	}
	else if((*it).second.Name() == "Ogar")
	{
		emperorId = AuId::OgarEmperor();
		emperorName = "Xidius";
	}
	else if((*it).second.Name() == "Otari")
	{
		emperorId = AuId::OtariEmperor();
		emperorName = "Boanon";
	}
	else if((*it).second.Name() == "Seledon")
	{
		emperorId = AuId::SeledonEmperor();
		emperorName = "Nahoth";
	}
	else if((*it).second.Name() == "Syth")
	{
		emperorId = AuId::SythEmperor();
		emperorName = "Decavit";
	}
	else if((*it).second.Name() == "Tassad")
	{
		emperorId = AuId::TassadEmperor();
		emperorName = "Si'darr";
	}
	else if((*it).second.Name() == "Vilmorti")
	{
		emperorId = AuId::VilmortiEmperor();
		emperorName = "Levalle";
	}
	else if((*it).second.Name() == "Vreen")
	{
		emperorId = AuId::VreenEmperor();
		emperorName = "Ortax";
	}
	else if((*it).second.Name() == "Zuul")
	{
		emperorId = AuId::ZuulEmperor();
		emperorName = "Kang";
	}
	else
		return false;

	return true;
}

AuTime DgData::EmpireNewestNewDossierDoc(quint32 empireId) const
{
	// Find the empire.
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return AuTime();

	// Return time of newest nu dossier document from the empire.
	return (*it).second.NewestNewDossierDoc();
}

bool DgData::EmpireThemeColors(quint32 empireId, QColor themeColors[AuScFace::THEME_Count]) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
	{
		// Make a default empire to get the default constructor theme colors.
		DgEmpire emp;
		emp.ThemeColors(themeColors);
		return empireId == 0;	// Default theme colors are ok if there is no empire at all.
	}

	(*it).second.ThemeColors(themeColors);
	return true;
}

void DgData::SetEmpireThemeColor(quint32 empireId, AuScFace::COLOR_THEME theme, const QColor& color)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.SetThemeColor(theme, color);
}

void DgData::SetEmpireThemeColors(quint32 empireId, const QColor themeColors[AuScFace::THEME_Count])
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return;

	(*it).second.SetThemeColors(themeColors);
}

int DgData::ScDesignSave(AuScDesign& design, quint32 designId)
{
	if(designId)
	{
		// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
		design.SetId(designId);
		AuScDesignHash::iterator di = m_ScDesigns->find(design.Id());
		if(di != m_ScDesigns->end())
		{
			// Replace existing design, regardless of its read/write status.
			(*di).second = design;
			g_Au.ScDesignChanged(design.Id());
			return 1;
		}
		else
		{
			// Add new design.
			m_ScDesigns->insert(pair<quint32, AuScDesign>(design.Id(), design));
			g_Au.ScDesignAdded(design.Id());
			return 0;
		}
	}
	else if(!design.Id())
	{
		// Saving a nu design.
		design.SetId(g_Au.NextSpaceShipDesignId());
		AuScDesignHash::iterator di = m_ScDesigns->find(design.Id());
		if(di != m_ScDesigns->end())
			return 4;					// New design id already exists. wtf?

		m_ScDesigns->insert(pair<quint32, AuScDesign>(design.Id(), design));
		g_Au.ScDesignAdded(design.Id());
		return 0;						// New design added.
	}
	else
	{
		// Overwriting an existing design.
		AuScDesignHash::iterator di = m_ScDesigns->find(design.Id());
		if(di == m_ScDesigns->end())
			return 3;					// Existing design was not found.

		if(!(*di).second.IsWritable())
			return 2;					// Existing design is read only.

		(*di).second = design;
		g_Au.ScDesignChanged(design.Id());
		return 1;						// Existing design updated.
	}
}

bool DgData::ScDesignIsLoaded(quint32 designId) const
{
	return m_ScDesigns->find(designId) != m_ScDesigns->cend();
}

quint8 DgData::ScDesignState(quint32 designId) const
{
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it == m_ScDesigns->cend())
		return 0;
	return (*it).second.State();
}

/*DHH waiting for design compatibility backport from Hazeron Starship
void DgData::ScDesignItemSpots(quint32 designId, QHash<COMMODITYID, QList<AuItemSpot> >& spotsByCID) const
{
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it != m_ScDesigns->cend())
		(*it).second.ItemSpots(spotsByCID);
}

void DgData::ScDesignVehicleParkingSpots(quint32 designId, QHash<COMMODITYID, QList<AuVehicleSpot> >& parkingByCID) const
{
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it != m_ScDesigns->cend())
		(*it).second.VehicleParkingSpots(parkingByCID);
}
*/

AuId DgData::ScDesignArchitect(quint32 designId) const
{
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it == m_ScDesigns->cend())
		return AuId();
	return (*it).second.ArchitectId();
}

// Signals db thread to load the design if it is not already loaded.
bool DgData::ScDesign(quint32 designId, AuScDesign& design) const
{
	// Locate the design record.
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it == m_ScDesigns->end())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.ScDesignNeeded(designId);
		return false;
	}

	// Found the design.
	design = (*it).second;
	return true;
}

// Signals db thread to load the design if it is not already loaded.
bool DgData::ScDesignReport(quint32 designId, AuScReport& report) const
{
	// Locate the design record.
	AuScDesignHash::const_iterator it = m_ScDesigns->find(designId);
	if(it == m_ScDesigns->end())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.ScDesignNeeded(designId);
		return false;
	}

	(*it).second.ReportDesign(report);
	return true;
}

// Called when this server has been notified that a design has been deactivated.
// Checks to see if a design is loaded and insures that it is marked as Deleted.
void DgData::ScDesignDeactivated(quint32 designId)
{
	AuScDesignHash::iterator it = m_ScDesigns->find(designId);
	if(it != m_ScDesigns->end() && !(*it).second.IsDeleted())
		(*it).second.SetState(AuScDesign::Deleted);	// Design is loaded and does not show as being deleted; mark it as deleted.
}

void DgData::ScDesignAddDesigns(const AuScDesignHash& designs)
{
	for(AuScDesignHash::const_iterator di = designs.begin(); di != designs.end(); di++)
		ScDesignAdd((*di).second);
}

void DgData::ScDesignAdd(const AuScDesign& design)
{
	AuScDesignHash::iterator it = m_ScDesigns->find(design.Id());
	if(it == m_ScDesigns->end())
		m_ScDesigns->insert(pair<quint32, AuScDesign>(design.Id(), design));
	else if(!(*it).second.IsDeleted())	// Don't overwrite a deleted design; the incoming design might not be deleted.
		(*it).second = design;
}

void DgData::ScDesignRemove(quint32 designId)
{
	AuScDesignHash::iterator di = m_ScDesigns->find(designId);
	if(di != m_ScDesigns->end())
		m_ScDesigns->erase(di);
}

// Deletes a design.
// Designs that are not final are actually deleted.
// Designs that are final are only marked as Deleted so they no longer appear in lists.
// Return result 0=no such design, 1=design deleted, 2=design marked as deleted
int DgData::ScDesignDelete(quint32 designId)
{
	AuScDesignHash::iterator it = m_ScDesigns->find(designId);
	if(it == m_ScDesigns->end())
		return 0;												// The design does not exist.

	// We do not delete designs even if unfinished because they can be sold via the published design library. Someone could have
	// a disk that references one of them, even if unfinished.
	/*
	// Cannot delete a design that is final. Existing ships and manufacturing processes could suffer.
	if((*it).second.IsFinal())
	{
	*/
		// Mark the design as 'Deleted' instead of actually deleting it.
		(*it).second.SetState(AuDesign::Deleted);
		g_Au.ScDesignChanged(designId);							// Update/add the database record.
		g_Au.ScDesignDeactivated((*it).second.Id());			// Notify other servers in the farm.
		return 2;
		/*
	}
	else
	{
		// The design is writable so it can really be deleted.
		g_Au.ScDesignDeleted(designId);
		m_ScDesigns->erase(it);
		return 1;
	}
	*/
}

int DgData::ScExchangeAdd(const AuScDesignExchangeEntry& libraryEntry)
{
	int result = m_ScExchange->Add(libraryEntry);
	if(result == 1)
		g_Au.ScExchangeEntryAdded(libraryEntry.DesignId());		// Added a nu entry.
	else if(result == 2)
		g_Au.ScExchangeEntryChanged(libraryEntry.DesignId());	// Replaced an existing entry.
	return result;
}

bool DgData::ScExchangeRemove(quint32 designId)
{
	if(!m_ScExchange->Remove(designId))
		return false;
	g_Au.ScExchangeEntryDeleted(designId);
	return true;
}

bool DgData::ScExchangeGet(quint32 designId, AuScDesignExchangeEntry& libraryEntry) const
{
	return m_ScExchange->Get(designId, libraryEntry);
}

bool DgData::ScDesignAllowedToManufacture(quint32 designId, AuScReport& report) const
{
	AuScDesignHash::const_iterator di = m_ScDesigns->find(designId);
	if(di == m_ScDesigns->cend())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.ScDesignNeeded(designId);
		return false;
	}

	// Report knows how to determine if a design is ok to manufacture.
	(*di).second.ReportDesign(report);
	//QStringList msgs;
	//return report.OkToManufacture(msgs, (*di).second);
	// OkToManufacture is a lot of work. Trust the state of the design; it had to pass the OkToManufacture test to be finalized.
	return (*di).second.State() == AuDesign::Final;
}

quint32 DgData::CityFacilitiesChanged(const AuId& cityId, const AuCityIndustryHash& facil)
{
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		AuCityHash::iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci == (*ei).second.m_Cities->end())
			continue;

		// Found the empire with the city.
		// Update the city's facilities.
		(*ci).second.SetFacilities(facil);
		return (*ei).first;
	}
	return 0;
}

quint32 DgData::CityStatusChanged(const AuId& cityId, ALERT_CONDITION ac, qint8 morale)
{
	for(DgEmpireHash::iterator ei = m_gEmpires.begin(); ei != m_gEmpires.end(); ei++)
	{
		AuCityHash::iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci == (*ei).second.m_Cities->end())
			continue;

		// Found the empire with the city.
		// Update the city's status.
		(*ci).second.SetAlertCondition(ac);
		(*ci).second.SetMorale(morale);
		return (*ei).first;
	}
	return 0;
}

AuPoint DgData::CitySystemLoc(const AuId& cityId) const
{
	for(DgEmpireHash::const_iterator ei = m_gEmpires.cbegin(); ei != m_gEmpires.cend(); ei++)
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci == (*ei).second.m_Cities->cend())
			continue;

		// Found the empire with the city.
		return (*ci).second.SystemLoc();
	}
	return AuPoint(0,0);
}

// Returns result code: 0=added, 1=updated, 2=existing design read only, 3=existing design not found, 4=nu id already exists.
int DgData::BdDesignSave(AuBdDesign& design, quint32 designId)
{
	if(designId)
	{
		// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
		design.SetId(designId);
		AuBdDesignHash::iterator di = m_BdDesigns->find(design.Id());
		if(di != m_BdDesigns->end())
		{
			// Replace existing design, regardless of its read/write status.
			(*di).second = design;
			g_Au.BdDesignChanged(design.Id());
			return 1;
		}
		else
		{
			// Add new design.
			m_BdDesigns->insert(pair<quint32, AuBdDesign>(design.Id(), design));
			g_Au.BdDesignAdded(design.Id());
			return 0;
		}
	}
	else if(!design.Id())
	{
		// Saving a nu design.
		design.SetId(g_Au.NextBuildingDesignId());
		AuBdDesignHash::iterator di = m_BdDesigns->find(design.Id());
		if(di != m_BdDesigns->end())
			return 4;					// New design id already exists. wtf?

		m_BdDesigns->insert(pair<quint32, AuBdDesign>(design.Id(), design));
		g_Au.BdDesignAdded(design.Id());
		return 0;						// New design added.
	}
	else
	{
		// Overwriting an existing design.
		AuBdDesignHash::iterator di = m_BdDesigns->find(design.Id());
		if(di == m_BdDesigns->end())
			return 3;					// Existing design was not found.

		if(!(*di).second.IsWritable())
			return 2;					// Existing design is read only.

		(*di).second = design;
		g_Au.BdDesignChanged(design.Id());
		return 1;						// Existing design updated.
	}
}

bool DgData::BdDesignIsLoaded(quint32 designId) const
{
	return m_BdDesigns->find(designId) != m_BdDesigns->cend();
}

bool DgData::BdDesignIsLoaded(quint32 designId, QSet<quint32>& xrefsNeeded) const
{
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->cend())
		return false;	// Design is not loaded.
	QSet<quint32> xrefs;
	(*it).second.ExternalReferences(xrefs);
	for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
		if(!BdDesignIsLoaded(*xi, xrefsNeeded))
			xrefsNeeded.insert(*xi);
	return true;
}

quint8 DgData::BdDesignState(quint32 designId) const
{
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->cend())
		return 0;
	return (*it).second.State();
}

AuId DgData::BdDesignArchitect(quint32 designId) const
{
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->cend())
		return AuId();
	return (*it).second.ArchitectId();
}

// Signals db thread to load the design if it is not already loaded.
bool DgData::BdDesign(quint32 designId, AuBdDesign& design) const
{
	// Locate the design record.
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->end())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.BdDesignNeeded(designId);
		return false;
	}

	// Found the design.
	design = (*it).second;

	// Insure we have any designs it references externally.
	QSet<quint32> xrefs;
	design.ExternalReferences(xrefs);
	for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
	{
		AuBdDesignHash::const_iterator it = m_BdDesigns->find(*xi);
		if(it == m_BdDesigns->end())
			g_Au.BdDesignNeeded(designId);	// Signal db thread to try loading the xref.
	}
	return true;
}

// Signals db thread to load the design if it is not already loaded.
bool DgData::BdDesignReport(quint32 designId, AuBdReport& report) const
{
	// Locate the design record.
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->end())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.BdDesignNeeded(designId);
		return false;
	}

	(*it).second.ReportDesign(report);
	return true;
}

// Called when this server has been notified that a design has been deactivated.
// Checks to see if a design is loaded and insures that it is marked as Deleted.
void DgData::BdDesignDeactivated(quint32 designId)
{
	AuBdDesignHash::iterator it = m_BdDesigns->find(designId);
	if(it != m_BdDesigns->end() && !(*it).second.IsDeleted())
		(*it).second.SetState(AuBdDesign::Deleted);	// Design is loaded and does not show as being deleted; mark it as deleted.
}

void DgData::BdDesignAddDesigns(const AuBdDesignHash& designs)
{
	for(AuBdDesignHash::const_iterator di = designs.begin(); di != designs.end(); di++)
		BdDesignAdd((*di).second);
}

void DgData::BdDesignAdd(const AuBdDesign& design)
{
	AuBdDesignHash::iterator it = m_BdDesigns->find(design.Id());
	if(it == m_BdDesigns->end())
		m_BdDesigns->insert(pair<quint32, AuBdDesign>(design.Id(), design));
	else if(!(*it).second.IsDeleted())	// Don't overwrite a deleted design; the incoming design might not be deleted.
		(*it).second = design;
}

void DgData::BdDesignRemove(quint32 designId)
{
	AuBdDesignHash::iterator di = m_BdDesigns->find(designId);
	if(di != m_BdDesigns->end())
		m_BdDesigns->erase(di);
}

// Deletes a design.
// Designs that are not final are actually deleted.
// Designs that are final are only marked as Deleted so they no longer appear in lists.
// Return result 0=no such design, 1=design deleted, 2=design marked as deleted
int DgData::BdDesignDelete(quint32 designId)
{
	AuBdDesignHash::iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->end())
		return 0;												// The design does not exist.

	// We do not delete designs even if unfinished because they can be sold via the published design library. Someone could have
	// a disk that references one of them, even if unfinished.
	/*
	// Cannot delete a design that is final. Existing ships and manufacturing processes could suffer.
	if((*it).second.IsFinal())
	{
	*/
		// Mark the design as 'Deleted' instead of actually deleting it.
		(*it).second.SetState(AuDesign::Deleted);
		g_Au.BdDesignChanged(designId);							// Update/add the database record.
		g_Au.BdDesignDeactivated((*it).second.Id());			// Notify other servers in the farm.
		return 2;
		/*
	}
	else
	{
		// The design is writable so it can really be deleted.
		g_Au.BdDesignDeleted(designId);
		m_BdDesigns->erase(it);
		return 1;
	}
	*/
}

int DgData::BdExchangeAdd(const AuBdDesignExchangeEntry& libraryEntry)
{
	int result = m_BdExchange->Add(libraryEntry);
	if(result == 1)
		g_Au.BdExchangeEntryAdded(libraryEntry.DesignId());		// Added a nu entry.
	else if(result == 2)
		g_Au.BdExchangeEntryChanged(libraryEntry.DesignId());	// Replaced an existing entry.
	return result;
}

bool DgData::BdExchangeRemove(quint32 designId)
{
	if(!m_BdExchange->Remove(designId))
		return false;
	g_Au.BdExchangeEntryDeleted(designId);
	return true;
}

bool DgData::BdExchangeGet(quint32 designId, AuBdDesignExchangeEntry& libraryEntry) const
{
	return m_BdExchange->Get(designId, libraryEntry);
}

bool DgData::BdDesignAllowedToConstruct(quint32 designId, AuBdReport& report) const
{
	AuBdDesignHash::const_iterator di = m_BdDesigns->find(designId);
	if(di == m_BdDesigns->cend())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.BdDesignNeeded(designId);
		return false;
	}

	// Report knows how to determine if a design is ok to construct.
	(*di).second.ReportDesign(report);
	//QStringList msgs;
	//return report.OkToConstruct(msgs, (*di).second);
	// OkToConstruct is a lot of work. Trust the state of the design; it had to pass the OkToConstruct test to be finalized.
	return (*di).second.State() == AuDesign::Final;
}

bool DgData::EmpireHasPatent(quint32 empireId, COMMODITYID patentCID, quint8 minQ) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.cend())
		if((*it).second.HasPatent(patentCID, minQ))
			return true;
	return false;
}

bool DgData::EmpireRecordPatent(quint32 empireId, COMMODITYID patentCID, quint8 patentQ)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.end())
		if((*it).second.RecordPatent(patentCID, patentQ))
			return true;
	return false;
}

quint16 DgData::IndustryManufacturingDefProc(INDUSTRYID iid) const
{
#if !defined(QT_NO_DEBUG)
	if((m_bIndustryArray[iid].m_id != iid))
		qWarning() << "error: DgData::IndustryManufacturingDefProc" << iid << m_bIndustryArray[iid].m_id;
#endif
	for(DbManufacturingProcessArray::const_iterator mpi = m_bManufacturingProcessArray.cbegin();
			mpi != m_bManufacturingProcessArray.cend(); mpi++)
		if((*mpi).m_isDefault && (*mpi).IndustryId() == iid)
			return (*mpi).Id();
	return 0;
}

quint16 DgData::IndustryManufacturingProcs(INDUSTRYID iid, QMap<QString, quint16>& mfgProcs) const
{
#if !defined(QT_NO_DEBUG)
	if((m_bIndustryArray[iid].m_id != iid))
		qWarning() << "error: DgData::IndustryManufacturingProcs" << iid << m_bIndustryArray[iid].m_id;
#endif
	quint16 defProc = 0;
	for(DbManufacturingProcessArray::const_iterator mpi = m_bManufacturingProcessArray.cbegin();
			mpi != m_bManufacturingProcessArray.cend(); mpi++)
	{
		if((*mpi).IndustryId() != iid)
			continue;
		mfgProcs.insert((*mpi).m_name, (*mpi).m_id);
		if((*mpi).m_isDefault)
			defProc = (*mpi).m_id;
	}
	return defProc;
}

quint64 DgData::MemoryUsage(QStringList& details) const
{
	quint64 mem = 0;
	mem += sizeof(DbIndustry) * m_bIndustryArray.capacity();
	mem += sizeof(DbManufacturingComponent) * m_bManufacturingComponentArray.capacity();
	mem += sizeof(DbManufacturingProcess) * m_bManufacturingProcessArray.capacity();
	quint64 emp = m_gEmpires.MemoryUsage();
	mem += emp;
	mem += m_Species.MemoryUsage();

	quint64 bd = m_BdDesigns->MemoryUsage();
	mem += bd;

	quint64 bdx = m_BdExchange->MemoryUsage();
	mem += bdx;

	quint64 sc = m_ScDesigns->MemoryUsage();
	mem += sc;

	quint64 scx = m_ScExchange->MemoryUsage();
	mem += scx;

	mem += sizeof(m_AccountExpiry);
	mem += sizeof(m_AccountType);
	mem += sizeof(m_ActiveUnit);
	mem += sizeof(m_confirmationNumber);
	mem += sizeof(m_idPlayer);
	mem += sizeof(m_UnitArmor);
	mem += sizeof(m_UnitClothes);
	mem += sizeof(m_UnitChromatid);
	mem += sizeof(m_UnitHair);
	mem += sizeof(m_UnitHand);
	mem += sizeof(m_UnitHat);
	mem += sizeof(m_UnitId);
	mem += sizeof(m_UnitName);
	mem += sizeof(m_UnitShave);
	mem += sizeof(m_UnitVisor);

	mem += sizeof(m_AcctName);
	mem += m_AcctPassCrypt.capacity();

	mem += sizeof(AuContact) * m_Contacts.size();

	mem += sizeof(m_DataReceived);
	details << QString("  Empires %L1 bytes").arg(emp);
	details << QString("  Bd designs %L1 bytes, exchange %L2 bytes").arg(bd).arg(bdx);
	details << QString("  Sc designs %L1 bytes, exchange %L2 bytes").arg(sc).arg(scx);
	details << QString("  Other %L1 bytes").arg(mem - (bd + bdx + sc + scx));
	details << QString("  Total %L1 bytes").arg(mem);
	return mem;
}

bool DgData::IsIndustryPatent(COMMODITYID patent, INDUSTRYID iid) const
{
	if(iid == IID_SpacecraftFactory)
		return DbCommodity::IsScPatent(patent);
	if(!DbCommodity::IsPatent(patent))
		return false;	// The cid is not a patent.
	if(iid == IID_School)
		return true;
	return m_bIndustryArray[iid].IsPatentRequired(patent);
}

AuId DgData::PirateBaseInSystem(const AuId& systemId, quint32& empireId) const
{
	return m_gEmpires.PirateBaseInSystem(systemId, empireId);
}

AuId DgData::PirateBaseSystemInSector(const AuId& sectorId, quint32& empireId) const
{
	return m_gEmpires.PirateBaseSystemInSector(sectorId, empireId);
}

void DgData::RemoveInhabitedSystems(AuIdList& systemIds) const
{
	return m_gEmpires.RemoveInhabitedSystems(systemIds);
}

AuSpecies DgData::EmpireCapitolSpecies(quint32 empireId) const
{
	return m_gEmpires.CapitolSpecies(empireId);
}

int DgData::PirateUbiquity(GALAXY galaxy) const
{
	return m_gEmpires.PirateUbiquity(galaxy);
}

int DgData::PirateEmpireRank(quint32 empireId) const
{
	return m_gEmpires.PirateEmpireRank(empireId);
}

AuBdPref DgData::EmpirePreferredBlueprints(quint32 empireId, INDUSTRYID industryId) const
{
	return m_gEmpires.PreferredBlueprints(empireId, industryId);
}

AuScPref DgData::EmpirePreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role) const
{
	return m_gEmpires.PreferredBlueprints(empireId, role);
}

void DgData::SetEmpirePreferredBlueprints(quint32 empireId, INDUSTRYID industryId, const AuBdPref& pref)
{
	m_gEmpires.SetPreferredBlueprints(empireId, industryId, pref);
}

void DgData::SetEmpirePreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role, const AuScPref& pref)
{
	m_gEmpires.SetPreferredBlueprints(empireId, role, pref);
}

void DgData::PirateBdBlueprints(QList<quint32>& blueprints) const
{
	m_gEmpires.PirateBdBlueprints(blueprints);
}

void DgData::PirateScBlueprints(QList<quint32>& blueprints) const
{
	m_gEmpires.PirateScBlueprints(blueprints);
}

quint32 DgData::PirateEncounterSpacecraft(quint32 empireId, GALAXY galaxy, AuScPrefs::SCROLE& scrole) const
{
	return m_gEmpires.PirateEncounterSpacecraft(empireId, galaxy, scrole);
}

void DgData::EmpireCity(quint32 empireId, const AuId& cityId, AuCity& city) const
{
	m_gEmpires.EmpireCity(empireId, cityId, city);
}

void DgData::PirateBaseBuildings(quint32 empireId, QList<INDUSTRYID>& buildings) const
{
	m_gEmpires.PirateBaseBuildings(empireId, buildings);
}

bool DgData::BdDesignBuildZoneCheckLocs(quint32 designId, AuPointArray& siteLocs) const
{
	AuBdDesignHash::const_iterator it = m_BdDesigns->find(designId);
	if(it == m_BdDesigns->cend())
		return false;
	(*it).second.BuildZoneCheckLocs(siteLocs);
	return true;
}

bool DgData::EmpireHasWarpTechnology(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.cend())
		if((*it).second.HasWarpTechnology())
			return true;
	return false;
}

void DgData::EmpirePatents(quint32 empireId, AuKnownPatents& patents) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.cend())
		(*it).second.ListPatents(patents);
}

unsigned int DgData::EmpireWarpTechnology(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it != m_gEmpires.cend())
		return (*it).second.WarpTechnology();
	return 0;
}

void DgData::ClientUnitDeleted(const AuId& unitId)
{
	for(int iu=0; iu<6; iu++)
		if(m_UnitId[iu] == unitId)
		{
			m_UnitId[iu] = AuId(0);
			if(m_UnitDNA[iu])
			{
				delete m_UnitDNA[iu];
				m_UnitDNA[iu] = 0;
			}
		}
}

void DgData::SetClientAvatars(const AuId unitIds[6], const QString unitNames[6])
{
	for(int iu=0; iu<6; iu++)
	{
		m_UnitId[iu] = unitIds[iu];
		m_UnitName[iu] = unitNames[iu];
		if(m_UnitDNA[iu])
		{
			delete m_UnitDNA[iu];
			m_UnitDNA[iu] = 0;
		}
		m_UnitChromatid[iu] = AuDNABase::Alpha;
		m_UnitHand[iu] = RightHanded;
	}
}

void DgData::SetClientInfo(int confirmNum, unsigned int idPlayer, const AuId idUnits[6], const QString names[6],
		const AuDNABase* dna[6], const AuDNABase::CHROMATID chroma[6], const HANDEDNESS hand[6], COMMODITYID clothes[6],
		COMMODITYID armor[6], COMMODITYID hat[6], quint8 visor[6], const AuHairStyle::STYLE hair[6],
		const AuHairStyle::SHAVE shave[6], ACCOUNT acctType, const AuTime& acctExpiry)
{
	m_AccountExpiry = acctExpiry;
	m_AccountType = acctType;
	m_ActiveUnit = 0;
	m_confirmationNumber = confirmNum;
	m_idPlayer = idPlayer;
	for(int iu=0; iu<6; iu++)
	{
		m_UnitId[iu] = idUnits[iu];
		m_UnitName[iu] = names[iu];
		if(m_UnitDNA[iu])
			delete m_UnitDNA[iu];
		if(dna[iu])
			m_UnitDNA[iu] = dna[iu]->Clone();
		else
			m_UnitDNA[iu] = 0;
		m_UnitChromatid[iu] = chroma[iu];
		m_UnitHand[iu] = hand[iu];
		m_UnitClothes[iu] = clothes[iu];
		m_UnitArmor[iu] = armor[iu];
		m_UnitHat[iu] = hat[iu];
		m_UnitVisor[iu] = visor[iu];
		m_UnitHair[iu] = hair[iu];
		m_UnitShave[iu] = shave[iu];
	}
}

COMMODITYID DgData::ManufacturingProcessMakes(int mpid) const
{
	if(mpid < 0)
		return CID_Invalid;
	for(DbManufacturingProcessArray::const_iterator it = m_bManufacturingProcessArray.cbegin();
			it != m_bManufacturingProcessArray.cend(); ++it)
		if((*it).m_id == mpid)
			return (*it).CommodityId();
	return CID_Invalid;
}

bool DgData::EmpireHasCitizen(quint32 empireId, const AuId& avatarId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;

	return (*it).second.HasCitizen(avatarId);
}

FOUNDER_STATUS DgData::EmpireCityFounderStatus(quint32 empireId, const AuId& cityId) const
{
	// Find the empire.
	DgEmpireHash::const_iterator ei = m_gEmpires.find(empireId);
	if(ei == m_gEmpires.end())
		return FOUNDER_NoEmpire;	// no empire

	// Find the city.
	AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
	if(ci == (*ei).second.m_Cities->cend())
		return FOUNDER_NoCity;		// no city

	// Determine the founder.
	AuId founderId = (*ci).second.FounderId();
	if(founderId.IsNull())
		return FOUNDER_NoFounder;	// no founder

	// Determine empire of founder.
	if((*ei).second.HasCitizen(founderId))
		return FOUNDER_Empire;		// founder belongs to city empire
	DgEmpireHash::const_iterator fei = m_gEmpires.FindCitizenEmpire(founderId);
	if(fei == m_gEmpires.cend())
		return FOUNDER_FreeOrDead;	// no empire, founder is free or dead

	// Determine political stance of founder empire to the city's empire.
	POLITICAL_STANCE stance = EmpirePoliticalStance((*fei).first, (*ei).first);
	switch(stance)
	{
	case POL_Friend:
		return FOUNDER_Friend;
	case POL_Neutral:
		return FOUNDER_Neutral;
	case POL_Enemy:
		return FOUNDER_Enemy;
	case POL_Empire:
		return FOUNDER_Empire;
	case POL_Vassal:
		return FOUNDER_Vassal;
	}
	return FOUNDER_BadStance;		// invalid stance? not really possible
}

// Signals db thread to load the design if it is not already loaded.
bool DgData::ItDesign(quint32 designId, AuItDesign& design) const
{
	// Locate the design record.
	AuItDesignHash::const_iterator it = m_ItDesigns->find(designId);
	if(it == m_ItDesigns->end())
	{
		// Signal db thread to try loading the design that someone wants.
		g_Au.ItDesignNeeded(designId);
		return false;
	}

	// Found the design.
	design = (*it).second;

	// Insure we have any designs it references externally.
	QSet<quint32> xrefs;
	design.ExternalReferences(xrefs);
	for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
	{
		AuItDesignHash::const_iterator it = m_ItDesigns->find(*xi);
		if(it == m_ItDesigns->end())
			g_Au.ItDesignNeeded(designId);	// Signal db thread to try loading the xref.
	}
	return true;
}

int DgData::ItDesignSave(AuItDesign& design, quint32 designId)
{
	if(designId)
	{
		// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
		design.SetId(designId);
		AuItDesignHash::iterator di = m_ItDesigns->find(design.Id());
		if(di != m_ItDesigns->end())
		{
			// Replace existing design, regardless of its read/write status.
			(*di).second = design;
			g_Au.ItDesignChanged(design.Id());
			return 1;
		}
		else
		{
			// Add new design.
			m_ItDesigns->insert(pair<quint32, AuItDesign>(design.Id(), design));
			g_Au.ItDesignAdded(design.Id());
			return 0;
		}
	}
	else if(!design.Id())
	{
		// Saving a nu design.
		design.SetId(g_Au.NextItemDesignId());
		AuItDesignHash::iterator di = m_ItDesigns->find(design.Id());
		if(di != m_ItDesigns->end())
			return 4;					// New design id already exists. wtf?

		m_ItDesigns->insert(pair<quint32, AuItDesign>(design.Id(), design));
		g_Au.ItDesignAdded(design.Id());
		return 0;						// New design added.
	}
	else
	{
		// Overwriting an existing design.
		AuItDesignHash::iterator di = m_ItDesigns->find(design.Id());
		if(di == m_ItDesigns->end())
			return 3;					// Existing design was not found.

		if(!(*di).second.IsWritable())
			return 2;					// Existing design is read only.

		(*di).second = design;
		g_Au.ItDesignChanged(design.Id());
		return 1;						// Existing design updated.
	}
}

AuId DgData::ItDesignArchitect(quint32 designId) const
{
	AuItDesignHash::const_iterator it = m_ItDesigns->find(designId);
	if(it == m_ItDesigns->cend())
		return AuId();
	return (*it).second.ArchitectId();
}

int DgData::ItDesignDelete(quint32 designId)
{
	AuItDesignHash::iterator it = m_ItDesigns->find(designId);
	if(it == m_ItDesigns->end())
		return 0;												// The design does not exist.

	// We do not delete designs even if unfinished because they can be sold via the published design library. Someone could have
	// a disk that references one of them, even if unfinished.
	/*
	// Cannot delete a design that is final. Existing ships and manufacturing processes could suffer.
	if((*it).second.IsFinal())
	{
	*/
		// Mark the design as 'Deleted' instead of actually deleting it.
		(*it).second.SetState(AuDesign::Deleted);
		g_Au.ItDesignChanged(designId);							// Update/add the database record.
		g_Au.ItDesignDeactivated((*it).second.Id());			// Notify other servers in the farm.
		return 2;
		/*
	}
	else
	{
		// The design is writable so it can really be deleted.
		g_Au.ItDesignDeleted(designId);
		m_ItDesigns->erase(it);
		return 1;
	}
	*/
}

void DgData::ItDesignAddDesigns(const AuItDesignHash& designs)
{
	for(AuItDesignHash::const_iterator di = designs.begin(); di != designs.end(); di++)
		ItDesignAdd((*di).second);
}

void DgData::ItDesignAdd(const AuItDesign& design)
{
	AuItDesignHash::iterator it = m_ItDesigns->find(design.Id());
	if(it == m_ItDesigns->end())
		m_ItDesigns->insert(pair<quint32, AuItDesign>(design.Id(), design));
	else if(!(*it).second.IsDeleted())	// Don't overwrite a deleted design; the incoming design might not be deleted.
		(*it).second = design;
}

void DgData::ItDesignDeactivated(quint32 designId)
{
	AuItDesignHash::iterator it = m_ItDesigns->find(designId);
	if(it != m_ItDesigns->end() && !(*it).second.IsDeleted())
		(*it).second.SetState(AuItDesign::Deleted);	// Design is loaded and does not show as being deleted; mark it as deleted.
}

void DgData::ItDesignRemove(quint32 designId)
{
	AuItDesignHash::iterator di = m_ItDesigns->find(designId);
	if(di != m_ItDesigns->end())
		m_ItDesigns->erase(di);
}

quint8 DgData::ItDesignState(quint32 designId) const
{
	AuItDesignHash::const_iterator it = m_ItDesigns->find(designId);
	if(it == m_ItDesigns->cend())
		return 0;
	return (*it).second.State();
}

bool DgData::ItDesignIsLoaded(quint32 designId) const
{
	return m_ItDesigns->find(designId) != m_ItDesigns->cend();
}

void DgData::SaveSolitaire(QDataStream& out) const
{
	quint8 ver = 1;
	out << ver;

	g_Au.LockDataForRead();
	out << m_Contacts;			// ver 1
	out << m_gEmpires;
	out << m_Species;
	g_Au.UnlockData();

	g_Au.LockBdDesignsForRead();
	out << *m_BdDesigns;
	g_Au.UnlockBdDesigns();

	g_Au.LockBdExchangeForRead();
	out << *m_BdExchange;
	out << *m_BdExchangeRatings;
	g_Au.UnlockBdExchange();

	g_Au.LockItDesignsForRead();
	out << *m_ItDesigns;
	g_Au.UnlockItDesigns();

	g_Au.LockScDesignsForRead();
	out << *m_ScDesigns;
	g_Au.UnlockScDesigns();

	g_Au.LockScExchangeForRead();
	out << *m_ScExchange;
	out << *m_ScExchangeRatings;
	g_Au.UnlockScExchange();
}

void DgData::RestoreSolitaire(QDataStream& in)
{
	quint8 ver;
	in >> ver;

	g_Au.LockDataForWrite();
	if(ver >= 1)
		in >> m_Contacts;
	in >> m_gEmpires;
	SetEmpireRole(105, EMPIRE_Agent);	// This prevents Imperium from being joinable by new avatars.
	//qDebug() << "Empires" << m_gEmpires.size();
	in >> m_Species;
	//qDebug() << "Species" << m_Species.size();
	g_Au.UnlockData();

	AuBdDesignHash bdDesigns;
	g_Au.LockBdDesignsForWrite();
	in >> bdDesigns;
	//qDebug() << "Building Designs" << bdDesigns.size();
	//for(AuBdDesignHash::const_iterator it = bdDesigns.cbegin(); it != bdDesigns.cend(); it++)
		//qDebug() << "id" << (*it).second.Id() << "name" << (*it).second.Name() << "state" << (*it).second.StateName();
	m_BdDesigns->Merge(bdDesigns);
	g_Au.UnlockBdDesigns();

	AuBdDesignExchange bdExchange;
	g_Au.LockBdExchangeForWrite();
	in >> bdExchange;
	m_BdExchange->Merge(bdExchange);
	//qDebug() << "Building Exchange" << m_BdExchange->size();
	in >> *m_BdExchangeRatings;
	//qDebug() << "Building Exchange Ratings" << m_BdExchangeRatings->size();
	g_Au.UnlockBdExchange();

	AuItDesignHash itDesigns;
	g_Au.LockItDesignsForWrite();
	in >> itDesigns;
	m_ItDesigns->Merge(itDesigns);
	//qDebug() << "Item Designs" << m_ItDesigns->size();
	g_Au.UnlockItDesigns();

	AuScDesignHash scDesigns;
	g_Au.LockScDesignsForWrite();
	in >> scDesigns;
	//qDebug() << "Spacecraft Designs" << scDesigns.size();
	//for(AuScDesignHash::const_iterator it = scDesigns.cbegin(); it != scDesigns.cend(); it++)
		//qDebug() << "id" << (*it).second.Id() << "name" << (*it).second.Name() << "state" << (*it).second.StateName();
	m_ScDesigns->Merge(scDesigns);
	g_Au.UnlockScDesigns();

	AuScDesignExchange scExchange;
	g_Au.LockScExchangeForWrite();
	in >> scExchange;
	m_ScExchange->Merge(scExchange);
	//qDebug() << "Spacecraft Exchange" << m_ScExchange->size();
	in >> *m_ScExchangeRatings;
	//qDebug() << "Spacecraft Exchange Ratings" << m_ScExchangeRatings->size();
	g_Au.UnlockScExchange();
}

void DgData::SaveSolitaireInit(QSqlDatabase& db)
{
	// Strip data that is unwanted in the solitaire init.
	g_Au.LockDataForWrite();

	// Clear all known species.
	m_Species.clear();
	//qDebug() << "Species" << m_Species.size();
	g_Au.UnlockData();

	QSet<quint32> empiresNeeded;	// Empires of exchange publishers.

	// Load Bd design records that were designed by Haxus.
	g_Au.LockBdDesignsForWrite();
	m_BdDesigns->LoadSoloInit();
	//qDebug() << "Building Designs" << m_BdDesigns->size();
	g_Au.UnlockBdDesigns();

	g_Au.LockBdExchangeForWrite();
	m_BdExchange->SqlSelectAll(db);
	// Remove Bd exchange records that were not designed by Haxus.
	AuBdDesignExchange::iterator bx = m_BdExchange->begin();
	while(bx != m_BdExchange->end())
	{
		if((*bx).DesignArchitectId() != AuId::Haxus())
			bx = m_BdExchange->erase(bx);
		else
		{
			empiresNeeded.insert((*bx).PublisherEmpire());
			bx++;
		}
	}
	//qDebug() << "Building Exchange" << m_BdExchange->size();

	// Clear all bd exchange ratings.
	delete m_BdExchangeRatings;
	m_BdExchangeRatings = new AuExchangeBlueprintRatings;
	//qDebug() << "Building Exchange Ratings" << m_BdExchangeRatings->size();
	g_Au.UnlockBdExchange();

	g_Au.LockItDesignsForWrite();
	m_ItDesigns->LoadSoloInit();
	//qDebug() << "Item Designs" << m_ItDesigns->size();
	g_Au.UnlockItDesigns();

	// Load Sc design records that were designed by Haxus.
	g_Au.LockScDesignsForWrite();
	m_ScDesigns->LoadSoloInit();
	//qDebug() << "Spacecraft Designs" << m_ScDesigns->size();
	g_Au.UnlockScDesigns();

	g_Au.LockScExchangeForWrite();
	m_ScExchange->SqlSelectAll(db);
	// Remove Sc exchange records that were not designed by Haxus.
	AuScDesignExchange::iterator sx = m_ScExchange->begin();
	while(sx != m_ScExchange->end())
	{
		if((*sx).DesignArchitectId() != AuId::Haxus())
			sx = m_ScExchange->erase(sx);
		else
		{
			empiresNeeded.insert((*sx).PublisherEmpire());
			sx++;
		}
	}
	//qDebug() << "Spacecraft Exchange" << m_ScExchange->size();

	// Clear all sc exchange ratings.
	delete m_ScExchangeRatings;
	m_ScExchangeRatings = new AuExchangeBlueprintRatings;
	//qDebug() << "Spacecraft Exchange Ratings" << m_ScExchangeRatings->size();
	g_Au.UnlockScExchange();

	// Strip non-pirate empires.
	unordered_map<quint32, DgEmpire, AuEmpireIdHash>::iterator ei = m_gEmpires.begin();
	while(ei != m_gEmpires.end())
	{
		if(!(*ei).second.IsPirate() &&
				!(*ei).second.IsArenaEmpire() &&
				!(*ei).second.IsAgentEmpire() &&
				!empiresNeeded.contains((*ei).first))
			ei = m_gEmpires.erase(ei);
		else
			ei++;
	}
	//qDebug() << "Empires" << m_gEmpires.size();

	// Data is prepped.

	// Store solitaire init data into a local cache file for use by stand-alone spacecraft designer.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath("GS");
	QFile::remove(cacheFileName);		// Remove the old file, if it already exists.

	// Insure the cache folder exists.
	if(g_Au.CreateCacheGame())
	{
		// The cache folder exists.
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QFile::WriteOnly))
		{
			// Prepare to write to the file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_5_0);

			// Write a header to the file.
			const quint8 fileType = SOLO_INIT_TYPE;
			const quint8 fileVersion = SOLO_INIT_VER;
			dsfile << fileType;
			dsfile << fileVersion;

			SaveSolitaire(dsfile);

			// Close the file.
			cacheFile.close();
		}
	}
}

int DgData::InitSolitaire()
{
	QString cacheFileName = "GS";
	if(!QFile::exists(cacheFileName))
		return 1;

	// Try to open the basic game data file. On failure, the client will query the server for the data and then the client will
	// save the file when SetBasicGameDataReceived is called.
	QFile cacheFile(cacheFileName);
	if(!cacheFile.open(QIODevice::ReadOnly))
		return 2;											// Failed to open cache data file.

	// Load from cache file.
	QDataStream dsfile(&cacheFile);
	dsfile.setVersion(QDataStream::Qt_5_0);

	// Read the header from the file.
	quint8 fileType;
	quint8 fileVersion;
	dsfile >> fileType;
	dsfile >> fileVersion;
	if(fileType != SOLO_INIT_TYPE || fileVersion != SOLO_INIT_VER)
	{
		cacheFile.close();
		return 3;
	}

	RestoreSolitaire(dsfile);

	// Close the file.
	cacheFile.close();
	return 0;	// Good return
}

bool DgData::AddClientUnit(const AuId& idUnit,
		const QString& name,
		const AuDNABase* dna,
		AuDNABase::CHROMATID chroma,
		HANDEDNESS hand,
		COMMODITYID clothes,
		COMMODITYID armor,
		COMMODITYID hat,
		quint8 visor,
		AuHairStyle::STYLE hair,
		AuHairStyle::SHAVE shave)
{
	int iu = ClientUnitAvailableIndex();
	if(iu < 0)
		return false;

	m_UnitId[iu] = idUnit;
	m_UnitName[iu] = name;
	if(m_UnitDNA[iu])
		delete m_UnitDNA[iu];
	if(dna)
		m_UnitDNA[iu] = dna->Clone();
	else
		m_UnitDNA[iu] = 0;
	m_UnitChromatid[iu] = chroma;
	m_UnitHand[iu] = hand;
	m_UnitClothes[iu] = clothes;
	m_UnitArmor[iu] = armor;
	m_UnitHat[iu] = hat;
	m_UnitVisor[iu] = visor;
	m_UnitHair[iu] = hair;
	m_UnitShave[iu] = shave;
	return true;
}

void DgData::SetClientUnitName(const AuId& unitId, const QString& unitName)
{
	int iu = ClientUnitIndex(unitId);
	if(iu < 0)
		return;
	m_UnitName[iu] = unitName;
}

void DgData::SoloAdjustGameData()
{
	for(DbManufacturingProcessArray::iterator pi = m_bManufacturingProcessArray.begin();
			pi != m_bManufacturingProcessArray.end(); pi++)
	{
		switch((*pi).CommodityId())
		{
		default:
			continue;
		case CID_PatentMakingWarp1FTLDriveModule:
		case CID_PatentMakingWarp2FTLDriveModule:
		case CID_PatentMakingWarp3FTLDriveModule:
		case CID_PatentMakingWarp4FTLDriveModule:
		case CID_PatentMakingWarp5FTLDriveModule:
		case CID_PatentMakingWarp6FTLDriveModule:
		case CID_PatentMakingWarp7FTLDriveModule:
		case CID_PatentMakingWarp8FTLDriveModule:
		case CID_PatentMakingWarp9FTLDriveModule:
		case CID_PatentMakingWormholePFTLDriveModule:
		case CID_PatentMakingWormholePNFTLDriveModule:
		case CID_PatentMakingWormholePNNFTLDriveModule:
			// FTL drive patents are not a time sink in solo play.
			if((*pi).m_idi == IID_School)
				(*pi).m_labor = 2400;	// At University
			else
				(*pi).m_labor = 2880;	// At the industry.
			break;
		}
	}
}

bool DgData::EmpireCityToJoin(quint32 empireId, AuId& city) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	city = (*it).second.JoinableCity();
	return !city.IsNull();
}

void DgData::EmpireFriendlies(quint32 empireId, QList<quint32>& friendlies) const
{
	m_gEmpires.ListFriendlies(empireId, friendlies);
}

void DgData::EmpireFriends(quint32 empireId, QList<quint32>& friends) const
{
	m_gEmpires.ListFriends(empireId, friends);
}

bool DgData::EmpireCitizenOnLine(quint32 empireId)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;
	return (*it).second.CitizenOnLine();
}

bool DgData::EmpireIsCitizenOnLine(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	return (*it).second.IsCitizenOnLine();
}

bool DgData::EmpireJoinRestricted(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	return (*it).second.JoinRestricted();
}

bool DgData::EmpireHasJoinPassword(quint32 empireId) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	return (*it).second.HasJoinPassword();
}

bool DgData::EmpireTestJoinPassCrypt(quint32 empireId, const QByteArray& passCrypt) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	return (*it).second.JoinRestricted() &&
		(*it).second.HasJoinPassword() &&
		(*it).second.TestJoinPassCrypt(passCrypt);
}

bool DgData::EmpireTestJoinPassword(quint32 empireId, const QString& password) const
{
	DgEmpireHash::const_iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.cend())
		return false;
	return (*it).second.JoinRestricted() &&
		(*it).second.HasJoinPassword() &&
		(*it).second.TestJoinPassword(password);
}

bool DgData::EmpireSetJoinPassCrypt(quint32 empireId, const QByteArray& passCrypt)
{
	DgEmpireHash::iterator it = m_gEmpires.find(empireId);
	if(it == m_gEmpires.end())
		return false;
	(*it).second.SetJoinPassCrypt(passCrypt);
	return true;
}

