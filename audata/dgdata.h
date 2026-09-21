//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dgdata.h,v 1.350 2022/05/05 16:26:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DGDATA_H
#define DGDATA_H

#include "aucontact.h"
#include "audata.h"
#include "audnabase.h"
#include "auglobal.h"
#include "auhair.h"
#include "dbindustry.h"
#include "dbmanufacturingcomponent.h"
#include "dbmanufacturingprocess.h"
#include "dgempire.h"
#include "dzspecies.h"
#include <QIcon>
#include <QVariant>

using namespace std;
#include <unordered_map>

class AuBdDesign;
class AuBdDesignHash;
class AuBdDesignExchange;
class AuBdDesignExchangeEntry;
class AuBdReport;
class AuCity;
class AuCityIndustryHash;
class AuExchangeBlueprintRatings;
class AuItDesign;
class AuItDesignHash;
class AuItemHash;
class AuKnownPatents;
class AuPointArray;
class AuScDesign;
class AuScDesignHash;
class AuScDesignExchange;
class AuScDesignExchangeEntry;
class AuScReport;
class DaPerson;
class QComboBox;
class QDataStream;
class QSqlDatabase;

class AUDATA_DLL DgData
{
public:
	DgData();
	~DgData();
	DgData(const DgData& c);

	// Only used on scene client to know when basic game data has been received
	// from the scene server. During startup, the app waits until this data has
	// been received.
	inline bool BasicGameDataReceived() const
	{
		return m_DataReceived;
	}

public:
	// Loads default empires, designs and exchanges.
	// Returns:
	// 0 = ok
	// 1 = file not found
	// 2 = file could not be opened
	// 3 = bad file type or version
	int InitSolitaire();

	// Removes inhabited systems from 'systemIds'.
	// Removes solar system ids from 'systemIds' that contain a city of any empire, according to empire records.
	void RemoveInhabitedSystems(AuIdList& systemIds) const;

	// Loads basic game data from a local cache file for use by solo designer and primeval world.
	// Returns 0 if the data was successfully loaded from a local file.
	// Returns 1 if the GD file was not found.
	// Returns 2 if the GD file could not be opened.
	// Returns 3 if the GD file data is incorrect.
	int LoadBasicGameData();

	// Saves basic game data to a local cache file for use by stand-alone spacecraft designer.
	void SaveBasicGameData();

	// Saves the file that is loaded to initialize a solitaire game. It contains default empires, designs and exchanges.
	// NOTE: Strips all data that is not wanted in the solitaire init from this DgData object.
	void SaveSolitaireInit(QSqlDatabase& db);

	// Called on client after the basic game data termination packet is received.
	void SetBasicGameDataReceived();

	void Load();

	// Gathers a complete set of all city ids from all empire records.
	void CityIds(QSet<AuId>& cityIds) const;

	// Industries
	inline const DbIndustry& Industry(INDUSTRYID iid) const
	{
		Q_ASSERT(iid >= IID_None);
		Q_ASSERT(iid < IID_Count);
		Q_ASSERT(iid < m_bIndustryArray.count());
#if !defined(QT_NO_DEBUG)
		if((m_bIndustryArray[iid].m_id != iid))
			qDebug("DgData::Industry iid %d != array value of %d", iid, m_bIndustryArray[iid].m_id);
#endif
		return m_bIndustryArray[iid];
	}

	inline int IndustryBuildRule(INDUSTRYID iid) const
	{
		Q_ASSERT(iid >= IID_None);
		Q_ASSERT(iid < IID_Count);
		Q_ASSERT(iid < m_bIndustryArray.count());
#if !defined(QT_NO_DEBUG)
		if((m_bIndustryArray[iid].m_id != iid))
			qDebug("DgData::IndustryBuildRule iid %d != array value of %d", iid, m_bIndustryArray[iid].m_id);
#endif
		return Industry(iid).BuildRule();
	}

	INDUSTRYID IndustryId(const QString& name) const;

	// Returns the id of the default manufacturing process for an industry, if any.
	quint16 IndustryManufacturingDefProc(INDUSTRYID iid) const;

	// Fills 'mfgProcs' with all possible manufacturing processes that can be run by industry 'iid'.
	// Processes that require natural resources, such as mines and wells, must be culled by the caller based on the environment.
	// Returns the id of the default process, if any.
	quint16 IndustryManufacturingProcs(INDUSTRYID iid, QMap<QString, quint16>& mfgProcs) const;

	inline QString IndustryName(INDUSTRYID iid) const
	{
		Q_ASSERT(iid >= IID_None);
		Q_ASSERT(iid < IID_Count);
		Q_ASSERT(iid < m_bIndustryArray.count());
#if !defined(QT_NO_DEBUG)
		if((m_bIndustryArray[iid].m_id != iid))
			qDebug("DgData::IndustryName iid %d != array value of %d", iid, m_bIndustryArray[iid].m_id);
#endif
		return Industry(iid).m_name;
	}

	// Returns true if 'industry' has any manufacturing process to make 'cid'.
	bool DoesIndustryMake(INDUSTRYID industry, COMMODITYID cid) const;

	// Returns true if 'industry' has any manufacturing process that uses 'cid'.
	bool DoesIndustryUse(INDUSTRYID industry, COMMODITYID cid) const;

	// Manufacturing Processes
	// 'mpid' is the manufacturing process id.
	bool DefaultWorkerEnabled(quint16 mpid) const;	// True if worker is enabled by default.

	float ManufacturingCost(COMMODITYID idCommodity) const;

	// 'mpid' is the manufacturing process id.
	bool ManufacturingProcess(quint16 mpid, DbManufacturingProcess& bMan) const;

	// 'mpid' is the manufacturing process id. It will be negative for spacecraft designs.
	INDUSTRYID ManufacturingProcessIndustry(int mpid) const;

	// Returns the commodity made by 'mpid'.
	COMMODITYID ManufacturingProcessMakes(int mpid) const;

	// 'mpid' is a manufacturing process id.
	// Returns the maximum Q of a process, based on the available patents.
	// Returns 0 if a required patent is not present in 'patents'.
	quint8 ManufacturingProcessPatentQ(quint16 mpid, const AuKnownPatents& patents) const;

	// 'mpid' is the manufacturing process id.
	bool IsManufacturingProcessNatural(quint16 mpid) const;

	bool IsIndustryPatent(COMMODITYID patent, INDUSTRYID iid) const;

	// Exports that consider political stance and commerce restrictions.
	bool ExportBrokerItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
			quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
			bool planetCanHaveFlora, bool planetCanHaveFauna) const;
	bool ExportCantinaItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
			quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
			bool planetCanHaveFlora, bool planetCanHaveFauna) const;
	bool ExportGroceryItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
			quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
			bool planetCanHaveFlora, bool planetCanHaveFauna) const;
	bool ExportRetailItems(qint32 exporterPopulation, quint32 exportEmpire, AuItemHash& exportInventory,
			quint32 importEmpire, AuItemHash& importInventory, int percent, int storageLimit,
			bool planetCanHaveFlora, bool planetCanHaveFauna) const;

	// Return empire id that currently controls a city.
	quint32 CityEmpire(const AuId& cityId) const;

	// Updates the city facilities, alert condition, and morale information in the empire record that owns 'cityId'.
	// Returns the empire id that owns 'cityId', 0 if none.
	quint32 CityFacilitiesChanged(const AuId& cityId, const AuCityIndustryHash& facil);

	quint32 CityStatusChanged(const AuId& cityId, ALERT_CONDITION ac, qint8 morale);

	// Returns any city id that is found on worldId.
	AuId CityOnWorld(const AuId& worldId) const;

	AuPoint CitySystemLoc(const AuId& cityId) const;

	void DeleteEmpire(quint32 empireId, QList<quint32>& chgEmpireIds);	// Delete empire record and clear other empire associations.
	bool Empire(DgEmpire& cEmp) const;	// Get cEmp rec using id set in cEmp.

	void EmpireAssignOffice(quint32 empireId, const AuId& avatarId, const AuOffice& office);

	void EmpireRevokeOffice(quint32 empireId, const AuId& avatarId);

	// True is returned if 'empireId' has a capital city.
	// 'capitolCity' is only set on true return.
	bool EmpireCapital(quint32 empireId, AuId& capitolCity) const;

	// True is returned if 'empireId' has a capital city.
	// 'capitolCity' and 'capitolCityGalaxy' are only set on true return.
	bool EmpireCapital(quint32 empireId, AuId& capitolCity, AuId& capitolCityGalaxy) const;

	AuSpecies EmpireCapitolSpecies(quint32 empireId) const;

	// Called when a citizen on 'empireId' signals that it is on line. Avatars do this once per minute.
	// Returns true if the empire record changes.
	bool EmpireCitizenOnLine(quint32 empireId);

	// Returns true if any avatar of 'empireId' was online recently, like within the past fifteen minutes.
	bool EmpireIsCitizenOnLine(quint32 empireId) const;

	// Gets information about a 'cityId' owned by an 'empireId'. City information is returned in 'city'.
	// If the city is owned by a different empire, 'city' is set to the constructor default.
	void EmpireCity(quint32 empireId, const AuId& cityId, AuCity& city) const;

	void EmpireCityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, const AuId& idCity);

	void EmpireCityDecayed(quint32 empireId, const AuId& idCity);

	AuId EmpireCityFounder(quint32 empireId, const AuId& cityId) const;

	// Returns the boss of a citizen who builds a city in the 'placeIds' domain hierarchy of 'empireId'.
	// Order is important here. Starts from the beginning of the placeIds list (world) and works its way to the end (universe).
	AuId EmpireCityFounderBoss(quint32 empireId, const AuIdList& places) const;

	FOUNDER_STATUS EmpireCityFounderStatus(quint32 empireId, const AuId& cityId) const;

	QString EmpireCityName(quint32 empireId, const AuId& cityId) const;

	AuId EmpireCityGalaxy(quint32 empireId, const AuId& cityId) const;

	// Finds the best city for a new avatar who starts by joining an empire.
	// Returns true on success, false on failure.
	bool EmpireCityToJoin(quint32 empireId, AuId& city) const;

	// Returns empire id of a common overlord empire if there is one.
	// Returns 0 if the empires do not share a common overlord.
	quint32 EmpireCommonOverlord(quint32 empireId1, quint32 empireId2) const;

	// Appends to 'friendlies' all the empires that consider 'empireId' to be a friend. Does not append 'empireId'.
	void EmpireFriendlies(quint32 empireId, QList<quint32>& friendlies) const;

	// Appends to 'friends' all the empires that 'empireId' considers to be a friend. Does not append 'empireId'.
	void EmpireFriends(quint32 empireId, QList<quint32>& friends) const;

	AuTime EmpireNewestNewDossierDoc(quint32 empireId) const;

	QString EmpireJoinOverlordStatement(quint32 empireId) const;

	// Returns the number of empires that can be joined by new empires created in 'galaxy' as a vassal at the start.
	quint32 EmpireOverlordJoinCount(const AuId& galaxy) const;

	EXPORT_POLICY EmpireCommerce(quint32 empireId, COMMODITYID cid) const;
	bool EmpireCommerceRestricted(quint32 empireId, COMMODITYID cid, POLITICAL_STANCE stance) const;	// restricted = no export
	QColor EmpireContrailColor(quint32 empireId) const;
	POLITICAL_STANCE EmpireDefaultPoliticalStance(quint32 empireId) const;
	bool EmpireFlag(quint32 empireId, QImage& imFlag) const;

	// true if a citizen of an empire is a governor who has the power to accept surrendered cities
	bool EmpireCitizenCanAcceptCitySurrender(quint32 empireId, const AuId& avatarId) const;

	// 'places' is typically the chain of place ids up to the universe from the city.
	bool EmpireCitizenCanAccessGovtAccount(quint32 empireId, const AuId& avatarId, const AuIdList& places) const;

	// true if a citizen of an empire is a governor who has the power to delegate their power to others.
	bool EmpireCitizenCanDelegate(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanInviteCitizens(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanKickCitizens(quint32 empireId, const AuId& avatarId) const;

	// 'places' is typically the chain of place ids up to the universe from the city.
	bool EmpireCitizenCanLoadOfficer(quint32 empireId, const AuId& avatarId, const AuIdList& places) const;

	bool EmpireCitizenCanManageDossier(quint32 empireId, const AuId& avatarId) const;

	// 'places' is typically the chain of place ids up to the universe from the city.
	bool EmpireCitizenCanManageTrade(quint32 empireId, const AuId& avatarId, const AuIdList& places) const;

	bool EmpireCitizenCanReceiveFleetMessages(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSendEmpireMail(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetAnnouncements(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetCapital(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetColorTheme(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetCommerce(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetContrailColor(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetEmpireFlag(quint32 empireId, const AuId& avatarId) const;

	// Returns true for emperor and anyone with permission to set sector capitals.
	bool EmpireCitizenCanSetGalaxyCapital(quint32 empireId, const AuId& avatarId) const;

	// 'places' is typically the chain of place ids up to the universe from the sector, including the sector.
	bool EmpireCitizenCanSetGalaxyCapital(quint32 empireId, const AuId& avatarId, const AuIdList& places) const;

	bool EmpireCitizenCanSetJoinPassword(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetPlaceNames(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetPoliticalStance(quint32 empireId, const AuId& avatarId) const;

	// Returns true for emperor and anyone with permission to set sector capitals.
	bool EmpireCitizenCanSetSectorCapital(quint32 empireId, const AuId& avatarId) const;

	// 'places' is typically the chain of place ids up to the universe from the sector, including the sector.
	bool EmpireCitizenCanSetSectorCapital(quint32 empireId, const AuId& avatarId, const AuIdList& places) const;

	bool EmpireCitizenCanSetTax(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanSetWarCriteria(quint32 empireId, const AuId& avatarId) const;

	bool EmpireCitizenCanUseDiplomacyChannel(quint32 empireId, const AuId& avatarId) const;

	// Filters the settings in 'office' against the permissions of 'avatarId', before assigning those permissions to someone.
	// Returns false if 'avatarId' is not allowed to assign anything.
	bool EmpireCitizenFilterAssignment(quint32 empireId, const AuId& avatarId, const AuId& assigneeId, AuOffice& office) const;

	QString EmpireCitizenName(quint32 empireId, const AuId& avatarId) const;

	QString EmpireCitizenNameWithTitle(quint32 empireId, const AuId& avatarId) const;

	AuOffice EmpireCitizenOffice(quint32 empireId, const AuId& avatarId) const;

	// Returns true if 'citizenId' is subordinate to 'avatarId' in the chain-of-command of 'empireId'.
	// Citizens who are not governors are not subordinate to anybody so return false.
	bool EmpireCitizenInChainOfCommand(quint32 empireId, const AuId& avatarId, const AuId& citizenId) const;

	// Returns a number that is multipled by an empire's reward for killing enemy units.
	// Returns 0 if the empire does not exist or the citizen does not exist in the empire.
	// Returns 1 for ordinary citizens.
	// Returns > 1 for governors, higher numbers based on domain of all subordinates recursively.
	quint32 EmpireCitizenTargetValue(quint32 empireId, const AuId& avatarId) const;

	QString EmpireCitizenTitle(quint32 empireId, const AuId& avatarId) const;

	// Returns the gender of the citizen 'avatarId' in empire 'empireId'.
	GENDER EmpireCitizenGender(quint32 empireId, const AuId& avatarId) const;

	// Return id of emperor.
	AuId EmpireEmperor(quint32 empireId) const;

	bool EmpireHasCitizen(quint32 empireId, const AuId& avatarId) const;

	// Returns true if 'empireId' has a city in the solar system 'systemId'.
	bool EmpireHasCityInSystem(quint32 empireId, const AuId& systemId) const;

	// Returns true if 'empireId' has a city on the world 'worldId'.
	bool EmpireHasCityOnWorld(quint32 empireId, const AuId& worldId) const;

	// true if the empire has a non-blank join password
	bool EmpireHasJoinPassword(quint32 empireId) const;

	// true if the empire has the patent at minQ or higher.
	bool EmpireHasPatent(quint32 empireId, COMMODITYID patentCID, quint8 minQ) const;

	// true if the empire has any warp drive patent
	bool EmpireHasWarpTechnology(quint32 empireId) const;

	// true if the empire has a restrictions on join, i.e. password or invite only
	bool EmpireJoinRestricted(quint32 empireId) const;

	// Returns the patents known to an empire.
	void EmpirePatents(quint32 empireId, AuKnownPatents& patents) const;

	bool EmpireSetJoinPassCrypt(quint32 empireId, const QByteArray& passCrypt);

	// true if 'passCrypt' matches the empire's encrypted join password.
	bool EmpireTestJoinPassCrypt(quint32 empireId, const QByteArray& passCrypt) const;

	// true if 'password' encrypts to match the empire's encrypted join password.
	bool EmpireTestJoinPassword(quint32 empireId, const QString& password) const;

	// Returns the highest warp factor available to an empire, based on its patents.
	unsigned int EmpireWarpTechnology(quint32 empireId) const;

	// Returns the id associated with the empire named 'name'. Empire names are required to be unique.
	// If the empire record is not found, checks for 'Forgotten Empire NN' format and returns NN.
	quint32 EmpireId(const QString& name) const;

	// Returns the name of the empire with 'empireId'.
	// Returns blank if and only if empire id is 0.
	// Returns 'Forgotten Empire NN' (where NN is empireId) if empireId is non-zero but the empire record cannot be found.
	bool EmpireName(quint32 empireId, QString& name) const;

	// Verifies that no other empire has a city in sectorId and that empireId has at least one city there.
	bool EmpireOkToRenameSector(quint32 empireId, const AuId& sectorId) const;

	QString EmpireHtmlName(quint32 empireId) const;

	// Changes the boss of 'governorId' to 'bossId' in the empire 'empireId'.
	bool EmpireChangeBoss(quint32 empireId, const AuId& governorId, const AuId& bossId);

	// Fills 'commanders' with all the people who are in the chain of command of the places listed in 'places'.
	void EmpireDomainGovernors(quint32 empireId, const AuIdList& places, AuIdList& commanders) const;

	// Fills 'commanders' with all the people who are in the chain of command of the places listed in 'places' that have permission
	// to load officers.
	void EmpireDomainOfficerLoaders(quint32 empireId, const AuIdList& places, AuIdList& commanders) const;

	// 'empireId' is the empire to change.
	// 'dosEmpire' is the empire in their dossier.
	// 'doc' is the document to be updated/added in the dossier.
	// returns 2 if the new document causes war to break out due to war criteria, which also changes 'dosEmpire's record.
	// returns 1 if the new document changes my dossier and should trigger a db update of 'empireId's record.
	// returns 0 if the new document does not change any empire records.
	int EmpireDossierAddDocument(quint32 empireId, quint32 dosEmpire, const AuDocument& doc);

	const AuId& EmpireGalaxyCapital(quint32 empireId, const AuId& galaxyId) const;	// Returns city id of galaxy capitol.
	const AuId& EmpireGalaxyCapital(quint32 empireId, GALAXY galaxy) const;	// Returns city id of galaxy capitol.

	// Determines the political stance of 'empireId' toward 'otherEmpireId', whether or not 'otherEmpireId's record still exists.
	// Returns POL_Empire if 'empireId' equals 'otherEmpireId'.
	// Returns POL_Neutral if 'otherEmpireId' or 'empireId' is 0.
	// Returns POL_Neutral if 'empireId's record does not exist.
	// Returns 'empireId's stance toward 'otherEmpireId', 'empireId's default stance if 'otherEmpireId' is not known to them.
	POLITICAL_STANCE EmpirePoliticalStance(quint32 empireId, quint32 otherEmpireId) const;

	// Returns preferred blueprints to use when constructing a building for 'industryId'.
	// Defaults are the AuBdPrefs static defaults.
	AuBdPref EmpirePreferredBlueprints(quint32 empireId, INDUSTRYID industryId) const;

	// Returns preferred blueprints to use when constructing a spacecraft for 'role'.
	// Defaults are the AuScPrefs static defaults.
	AuScPref EmpirePreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role) const;

	// Fills 'avatarIds' with the ids of empire governors who are to receive ship messages.
	void EmpireReceivesFleetMessages(quint32 empireId, const AuIdList& places, AuIdList& avatarIds) const;

	// Fills 'avatarIds' with the ids of empire governors who are to receive sensor scan reports.
	void EmpireReceivesSensorScans(quint32 empireId, const AuIdList& places, AuIdList& avatarIds) const;

	// Returns true if the empire record is changed.
	bool EmpireRecordPatent(quint32 empireId, COMMODITYID patentCID, quint8 patentQ);

	EMPIRE_ROLE EmpireRole(quint32 empireId) const;

	const AuId& EmpireSectorCapital(quint32 empireId, const AuId& sectorId) const;	// Returns city id of sector capitol.

	bool EmpireThemeColors(quint32 empireId, QColor themeColors[AuScFace::THEME_Count]) const;

	float EmpireIncomeTax(quint32 empireId) const;
	float EmpireSalesTax(quint32 empireId) const;
	float EmpireSubsidyTax(quint32 empireId) const;

	// Return random empire with open join policy
	// Empire must have a city.
	quint32 JoinRandomEmpire(quint32 exceptEmpireId) const;

	bool HasEmpire(quint32 empireId) const;

	// Checks the known chain up to the universe to see if 'avatarId' is in the domain of 'cityId' or any of its parent domains.
	bool IsEmpireCityInDomain(quint32 empireId, const AuId& cityId, const AuId& avatarId) const;

	bool IsEmpireGalaxyCapital(quint32 empireId, const AuId& cityId) const;	// true if cityId is a galaxy capitol

	// Returns true if 'avatarId' holds any government office in 'empireId'.
	bool IsEmpireGovernor(quint32 empireId, const AuId& avatarId) const;

	bool IsEmpireKnown(quint32 toEmpireId, quint32 testEmpireId) const;	// true if 'testEmpireId' is known to empire 'toEmpireId'.

	// True if any place in 'places' is in the domain of 'avatarId'.
	bool IsEmpirePlaceInDomain(quint32 empireId, const AuIdList& places, const AuId& avatarId) const;

	bool IsEmpireSectorCapital(quint32 empireId, const AuId& cityId) const;	// true if cityId is a sector capitol

	// Returns true for pirate empires that are ready for encounters. They will have preferred spacecraft and building designs that
	// are ready for generating encounters.
	bool IsPirateEmpireReady(quint32 empireId) const;

	void ListAllAvatars(QComboBox& cb) const;		// Using data in empire records.
	void ListAllCities(QComboBox& cb) const;		// Using data in empire records.
	void ListAllEmpires(QComboBox& cb) const;		// Using empire records.

	// List empires that can be joined by a new avatar.
	void ListEmpiresToJoin(DgEmpireHash& joinableEmpires) const;

	// List empires that can be joined by a new avatar.
	// Item data is the empire id.
	void ListEmpiresToJoin(QComboBox& cb) const;

	// List empires into the combo box that can be joined by a newly created vassal empire in 'galaxy'.
	// The result will be limited to the to ten empires.
	void ListOverlordEmpiresToJoin(QComboBox& cb, const AuId& galaxy) const;

	void FindEmpireForPlayerId(const AuId& idPlayer, DgEmpire& empire) const;

	// Returns the list of buildings a pirate empire likes to build.
	void PirateBaseBuildings(quint32 empireId, QList<INDUSTRYID>& buildings) const;

	// Returns the Bd id of the pirate base HQ in 'systemId', or AuId() if none.
	// 'empireId' is an output parameter, set to the pirate empire whose base was found.
	AuId PirateBaseInSystem(const AuId& systemId, quint32& empireId) const;

	// Returns the id of a solar system contained in 'sectorId' that has a pirate base, or AuId() if none.
	// 'empireId' is an output parameter, set to the pirate empire whose base was found.
	AuId PirateBaseSystemInSector(const AuId& sectorId, quint32& empireId) const;

	// Fills 'blueprints' with all preferred bd blueprint ids from pirate empires that are ready for encounters.
	// Ids are appended to 'blueprints'; it is not cleared.
	void PirateBdBlueprints(QList<quint32>& blueprints) const;

	// Returns false if 'empireId' is not a pirate empire.
	bool PirateEmperor(quint32 empireId, AuId& emperorId, QString& emperorName) const;

	// Returns ranking/difficulty level of pirate 'empireId', 1 to 15. Returns 0 if 'empireId' is not a pirate empire.
	int PirateEmpireRank(quint32 empireId) const;

	// Returns an sc spacecraft blueprint id to use for an encounter with a pirate ship.
	// 'empireId' is the pirate empire that has been encountered.
	// 'galaxy' is the galaxy in which the encounter occurred.
	// 'scrole' is an output parameter, which is the role of the returned blueprint id.
	quint32 PirateEncounterSpacecraft(quint32 empireId, GALAXY galaxy, AuScPrefs::SCROLE& scrole) const;

	// Fills 'blueprints' with all preferred sc blueprint ids from pirate empires that are ready for encounters.
	// Ids are appended to 'blueprints'; it is not cleared.
	void PirateScBlueprints(QList<quint32>& blueprints) const;

	// Returns the total ubiquity value of pirate empires in 'galaxy' that are ready for encounters.
	int PirateUbiquity(GALAXY galaxy) const;

	// City id must not be in 'exceptIds'.
	bool RandomEmpireCity(quint32 empireId, AuId& cityId, const QSet<AuId>& exceptIds) const;

	// Returns sentient species key from a random empire.
	AuSpecies RandomEmpireSpecies() const;

	// Randomly select a pirate empire id for an encounter in 'galaxy'. The returned pirate empire will have sufficient bd and sc
	// blueprint preferences to encounter their ships, stations and bases in all possible environments.
	quint32 RandomPirateEmpire(GALAXY galaxy) const;

	void ReplaceEmpire(const DgEmpire& empire);	// Add/replace an empire record.

	// No global locks are locked when this function is called.
	void RestoreSolitaire(QDataStream& in);

	// No global locks are locked when this function is called.
	void SaveSolitaire(QDataStream& out) const;

	void SetEmpireAnnouncements(quint32 empireId, const QString& announcements);

	void SetEmpireCapital(quint32 empireId, const AuId& capitolCity, const AuSpecies& species);

	// 'docTime', 'docType' and 'docIndex' together are used to identify the exact document.
	void SetEmpireDossierChangeState(quint32 empireId, quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex,
			AuDocument::DOC_STATE docState);

	// Returns the id of the affected empire.
	quint32 SetEmpireAvatarName(AuId& avatarId, const QString& nuName);

	// Returns the id of the affected empire.
	quint32 SetEmpireCityName(AuId& cityId, const QString& nuName);

	// Caller MUST insure all cities on the planet are controlled by the same empire.
	// Returns the id of the affected empire.
	quint32 SetEmpirePlanetName(AuId& planetId, const QString& nuName);

	// 'empireIds' is filled with the list of affected empires.
	void SetEmpireSectorName(AuId& sectorId, const QString& nuName, QList<quint32>& empireIds);

	// Caller MUST insure all cities in the system are controlled by the same empire.
	// Returns the id of the affected empire.
	quint32 SetEmpireSystemName(AuId& systemId, const QString& nuName);

	// Returns true if the empire record is changed.
	bool SetEmpireCityWorkerSpecies(quint32 empireId, const AuId& cityId, const AuSpecies& species);

	void SetEmpireCommerce(quint32 empireId, const QByteArray& commerce);

	// Hue value range is the full range of an unsigned byte.
	// Saturation value is clamped to the range 64-191.
	void SetEmpireContrailColor(quint32 empireId, quint8 hue, quint8 sat);

	void SetEmpireDefaultPoliticalStance(quint32 empireId, POLITICAL_STANCE stance);
	void SetEmpireFlag(quint32 empireId, const QImage& flag);

	void SetEmpireGalaxyCapital(quint32 empireId, const AuId& capitolCity, const AuId& galaxy);

	void SetEmpireName(quint32 empireId, const QString& name, QList<quint32>& chgEmpireIds);

	void SetEmpirePreferredBlueprints(quint32 empireId, INDUSTRYID industryId, const AuBdPref& pref);

	void SetEmpirePreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role, const AuScPref& pref);

	void SetEmpireRole(quint32 empireId, EMPIRE_ROLE gameRole);

	void SetEmpireSectorCapital(quint32 empireId, const AuId& capitolCity, const AuId& sector);

	void SetEmpireTaxes(quint32 empireId, quint8 income, quint8 sales, quint8 subsidy);

	void SetEmpireThemeColor(quint32 empireId, AuScFace::COLOR_THEME theme, const QColor& color);

	void SetEmpireThemeColors(quint32 empireId, const QColor themeColors[AuScFace::THEME_Count]);

	void SetEmpireWarCriteria(quint32 empireId, const AuWarCriteria& crit);

	void AddCityToEmpire(quint32 empireId, const AuCity& city);

	// 'systemLoc' is the galaxy coordinates of the city's solar system.
	void AddCityToEmpire(quint32 empireId,
			CITYTYPE cityType, const AuId& cityId, const QString& cityName,
			const AuId& founderId, const QString& founderName,
			const AuId& planetId, const QString& planetName,
			const AuId& systemId, const QString& systemName, const AuPoint& systemLoc,
			const AuId& sectorId, const QString& sectorName,
			const AuId& galaxyId, GALAXY galaxy, bool isSectorCapital, bool isGalaxyCapital);

	void RemoveCitizenFromEmpire(quint32 empireId, const AuId& idPlayer);

	// 'personGender' 0=female, 1=male
	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, const AuId& personId, const QString& personName,
			quint8 personGender);

public:
	//-------------------
	// Bd Designs

	// Triggers design load request if it's not loaded.
	// Signals db thread to load the design if it is not already loaded.
	bool BdDesign(quint32 designId, AuBdDesign& design) const;

	// Replaces existing record with the same id.
	void BdDesignAdd(const AuBdDesign& design);

	void BdDesignAddDesigns(const AuBdDesignHash& designs);

	// Signals db thread to load the design if it is not already loaded.
	// 'report' is an output parameter on true return.
	bool BdDesignAllowedToConstruct(quint32 designId, AuBdReport& report) const;

	AuId BdDesignArchitect(quint32 designId) const;

	// Retrieves the build zone check locations for a building design.
	// Returns false if the design is not found/loaded. Does not trigger a design load.
	// Returns true if the design is found, even if it has no build zone locs.
	bool BdDesignBuildZoneCheckLocs(quint32 designId, AuPointArray& siteLocs) const;

	void BdDesignDeactivated(quint32 designId);

	// Deletes a design.
	// Designs that are not final are actually deleted.
	// Designs that are final are only marked as Deleted so they no longer appear in lists.
	// Return result 0=no such design, 1=design deleted, 2=design marked as deleted
	int BdDesignDelete(quint32 designId);

	bool BdDesignIsLoaded(quint32 designId) const;

	// If the design is loaded, 'xrefsNeeded' is filled with any externally referenced designs that are not loaded.
	bool BdDesignIsLoaded(quint32 designId, QSet<quint32>& xrefsNeeded) const;

	// doesn't update database
	void BdDesignRemove(quint32 designId);

	// Triggers design load request if it's not loaded.
	// Returns true if the design is loaded and 'report' is returned.
	bool BdDesignReport(quint32 designId, AuBdReport& report) const;

	// Caller must LockBdDesignsForWrite.
	// If id of 'design' is 0, it is updated with the new id.
	// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
	// Returns result code: 0=added, 1=updated, 2=existing design read only, 3=existing design not found, 4=new id already exists.
	int BdDesignSave(AuBdDesign& design, quint32 designId=0);

	// Returns AuDesign::STATE of 'designId'.
	quint8 BdDesignState(quint32 designId) const;

	//-------------------

public:
	//-------------------
	// Bd Library

	// Replaces existing record with the same id.
	// Returns 1=added new entry, 2=replaced entry, 3=rejected duplicate entry.
	int BdExchangeAdd(const AuBdDesignExchangeEntry& libraryEntry);

	// Removes record 'designId' from the library.
	// Returns true if 'designId' is removed, false if it is not in the published library.
	bool BdExchangeRemove(quint32 designId);

	// Gets the library entry for a 'designId'.
	// Returns true on success, false if no entry found for 'designId'.
	bool BdExchangeGet(quint32 designId, AuBdDesignExchangeEntry& libraryEntry) const;

	//-------------------

public:
	//-------------------
	// It Designs

	// Triggers design load request if it's not loaded.
	// Signals db thread to load the design if it is not already loaded.
	bool ItDesign(quint32 designId, AuItDesign& design) const;

	// Replaces existing record with the same id.
	void ItDesignAdd(const AuItDesign& design);

	void ItDesignAddDesigns(const AuItDesignHash& designs);

	AuId ItDesignArchitect(quint32 designId) const;

	// Called when this server has been notified that a design has been deactivated.
	// Checks to see if a design is loaded and insures that it is marked as Deleted.
	void ItDesignDeactivated(quint32 designId);

	// Deletes a design.
	// Designs that are not final are actually deleted.
	// Designs that are final are only marked as Deleted so they no longer appear in lists.
	// Return result 0=no such design, 1=design deleted, 2=design marked as deleted
	int ItDesignDelete(quint32 designId);

	bool ItDesignIsLoaded(quint32 designId) const;

	// doesn't update database
	void ItDesignRemove(quint32 designId);

	// Caller must LockItDesignsForWrite.
	// If id of 'design' is 0, it is updated with the new id.
	// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
	// Returns result code: 0=added, 1=updated, 2=existing design read only, 3=existing design not found, 4=new id already exists.
	int ItDesignSave(AuItDesign& design, quint32 designId=0);

	// Returns AuDesign::STATE of 'designId'.
	quint8 ItDesignState(quint32 designId) const;

	//-------------------

public:
	//-------------------
	// Sc Designs

	// Triggers design load request if it's not loaded.
	// Signals db thread to load the design if it is not already loaded.
	bool ScDesign(quint32 designId, AuScDesign& design) const;

	// Replaces existing record with the same id.
	void ScDesignAdd(const AuScDesign& design);

	void ScDesignAddDesigns(const AuScDesignHash& designs);

	// Signals db thread to load the design if it is not already loaded.
	// 'report' is an output parameter on true return.
	bool ScDesignAllowedToManufacture(quint32 designId, AuScReport& report) const;

	AuId ScDesignArchitect(quint32 designId) const;

	// Called when this server has been notified that a design has been deactivated.
	// Checks to see if a design is loaded and insures that it is marked as Deleted.
	void ScDesignDeactivated(quint32 designId);

	// Deletes a design.
	// Designs that are not final are actually deleted.
	// Designs that are final are only marked as Deleted so they no longer appear in lists.
	// Return result 0=no such design, 1=design deleted, 2=design marked as deleted
	int ScDesignDelete(quint32 designId);

	bool ScDesignIsLoaded(quint32 designId) const;

	// doesn't update database
	void ScDesignRemove(quint32 designId);

	// Triggers design load request if it's not loaded.
	// Returns true if the design is loaded and 'report' is returned.
	bool ScDesignReport(quint32 designId, AuScReport& report) const;

	// Caller must LockScDesignsForWrite.
	// If id of 'design' is 0, it is updated with the new id.
	// 'designId' can be specified by an agent to assign a specific id. Existing design is replaced, if any.
	// Returns result code: 0=added, 1=updated, 2=existing design read only, 3=existing design not found, 4=new id already exists.
	int ScDesignSave(AuScDesign& design, quint32 designId=0);

	// Returns AuDesign::STATE of 'designId'.
	quint8 ScDesignState(quint32 designId) const;

	//-------------------

public:
	//-------------------
	// Sc Library

	// Replaces existing record with the same id.
	// Returns 1=added new entry, 2=replaced entry, 3=rejected duplicate entry.
	int ScExchangeAdd(const AuScDesignExchangeEntry& libraryEntry);

	// Removes record 'designId' from the library.
	// Returns true if 'designId' is removed, false if it is not in the published library.
	bool ScExchangeRemove(quint32 designId);

	// Gets the library entry for a 'designId'.
	// Returns true on success, false if no entry found for 'designId'.
	bool ScExchangeGet(quint32 designId, AuScDesignExchangeEntry& libraryEntry) const;

	//-------------------

public:
	quint64 MemoryUsage(QStringList& details) const;

	// Returns true if 'species' has been named.
	bool HasSpecies(const AuSpecies& species) const;

	// Assigns a name to a specie.
	void NameSpecies(const AuSpecies& species, const QString& name, const AuId& whoNamed);

	void ReplaceSpecies(const DzSpecies& species);	// Add/replace a specie record.

	// Returns the name of a species, if it has been named.
	// 'descIfUnnamed' returns the AuSpecies::Description for unnamed species, otherwise a blank string is returned.
	QString SpeciesName(const AuSpecies& species, bool descIfUnnamed) const;

	// Returns the id of the person who named a specie.
	// Returns null id if the specie has not been named.
	const AuId& SpeciesNamedBy(const AuSpecies& species) const;

	// Server specific functions.
	quint32 HighestEmpireId() const;

	// Client specific functions.
	inline AuTime ClientAccountExpiry() const
	{
		return m_AccountExpiry;
	}

	inline ACCOUNT ClientAccountType() const
	{
		return m_AccountType;
	}

	inline bool ClientPlayerIsAgent() const
	{
		return m_idPlayer && AcctIsAgent(m_AccountType);
	}

	inline bool ClientPlayerIsArchitect() const
	{
		return m_idPlayer && AcctIsArchitect(m_AccountType);
	}

	inline bool ClientPlayerIsExpired() const
	{
		return m_idPlayer && AcctIsExpired(m_AccountType);
	}

	inline bool ClientPlayerIsMinion() const
	{
		return m_idPlayer && AcctIsMinion(m_AccountType);
	}

	inline bool ClientPlayerIsProgrammer() const
	{
		return m_idPlayer && AcctIsProgrammer(m_AccountType);
	}

	inline COMMODITYID ClientUnitArmor(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitArmor[iu];
		return CID_Invalid;
	}

	// Returns an available client index from 0 to 5 if one is available.
	// Returns -1 if no slots are available.
	inline int ClientUnitAvailableIndex() const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu].IsNull())
				return iu;
		return -1;
	}

	inline AuDNABase::CHROMATID ClientUnitChromatid(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitChromatid[iu];
		return AuDNABase::Alpha;
	}

	inline COMMODITYID ClientUnitClothes(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitClothes[iu];
		return CID_Invalid;
	}

	void ClientUnitDeleted(const AuId& unitId);

	// Returns 0 if no DNA.
	inline const AuDNABase* ClientUnitDNA(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitDNA[iu];
		return 0;
	}

	inline AuHairStyle::STYLE ClientUnitHaircut(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitHair[iu];
		return AuHairStyle::Bald;
	}

	inline HANDEDNESS ClientUnitHand(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitHand[iu];
		return RightHanded;
	}

	inline COMMODITYID ClientUnitHat(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitHat[iu];
		return CID_Invalid;
	}

	inline int ClientUnitIndex(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return iu;
		return -1;
	}

	// Returns the state of the helmet, if any.
	inline quint8 ClientUnitVisor(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitVisor[iu];
		return 0;
	}

	inline const AuId& ClientUnitId(unsigned int unitIndex) const
	{
		Q_ASSERT(unitIndex < 6);
		return m_UnitId[unitIndex];
	}

	inline const AuId& ClientUnitId() const
	{
		return ClientUnitId(m_ActiveUnit);
	}

	inline QString ClientUnitName(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitName[iu];
		return "";
	}

	inline AuHairStyle::SHAVE ClientUnitShave(const AuId& unitId) const
	{
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
				return m_UnitShave[iu];
		return AuHairStyle::Clean;
	}

	inline unsigned int ConfirmationNumber() const
	{
		return m_confirmationNumber;
	}

	inline bool HasClientUnit() const
	{
		for(int iu=0; iu<6; iu++)
			if(!m_UnitId[iu].IsNull())
				return true;	// Found a non-null unit id.
		return false;
	}

	inline bool IsClientLoggedIn() const
	{
		return m_idPlayer != 0 && m_confirmationNumber != 0;
	}

	inline unsigned int PlayerId() const
	{
		return m_idPlayer;
	}

	// This function establishes the person id that the client will be controlling during the gaming session.
	inline void SetActiveUnit(const AuId& unitId)
	{
		// First, check to see if the unit id is already in our list of units.
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu] == unitId)
			{
				m_ActiveUnit = iu;
				return;
			}

		// The unit id was not in our list. It must be a nuly created unit. Stick it into a null entry.
		for(int iu=0; iu<6; iu++)
			if(m_UnitId[iu].IsNull())
			{
				m_ActiveUnit = iu;
				m_UnitId[iu] = unitId;
				return;
			}

		// The unit id was not in our list and we don't have any null entries to reassign. This should never happen but we'll deal
		// with it sanely.
		m_ActiveUnit = 0;
		m_UnitId[0] = unitId;
	}

	// Only sets the ids and names of the current player's avatars.
	void SetClientAvatars(const AuId unitIds[6], const QString unitNames[6]);

	void SetClientInfo(int confirmNum, unsigned int idPlayer,
			const AuId idUnits[6],
			const QString names[6],
			const AuDNABase* dna[6],
			const AuDNABase::CHROMATID chroma[6],
			const HANDEDNESS hand[6],
			COMMODITYID clothes[6],
			COMMODITYID armor[6],
			COMMODITYID hat[6],
			quint8 visor[6],
			const AuHairStyle::STYLE hair[6],
			const AuHairStyle::SHAVE shave[6],
			ACCOUNT acctType,
			const AuTime& acctExpiry);

	// Used in solo play to change the name of an avatar.
	void SetClientUnitName(const AuId& unitId, const QString& unitName);

	// Used in solo play to add a newly created avatar to the solo "account".
	// Returns false if no slots are available.
	bool AddClientUnit(const AuId& idUnit,
			const QString& name,
			const AuDNABase* dna,
			AuDNABase::CHROMATID chroma,
			HANDEDNESS hand,
			COMMODITYID clothes,
			COMMODITYID armor,
			COMMODITYID hat,
			quint8 visor,
			AuHairStyle::STYLE hair,
			AuHairStyle::SHAVE shave);

	inline void SetAcctName(const QString& acctName)
	{
		m_AcctName = acctName;
	}

	inline const QString& GetAcctName() const
	{
		return m_AcctName;
	}

	// Sets the SHA-1 hash key for the account password.
	inline void SetAcctPassCrypt(const QByteArray& acctPassword)
	{
		m_AcctPassCrypt = acctPassword;
	}

	// Gets the SHA-1 hash key for the account password.
	inline const QByteArray& GetAcctPassCrypt() const
	{
		return m_AcctPassCrypt;
	}

	// set the buddy/squelch list
	inline void SetContacts(QList<AuContact>& contacts)
	{
		m_Contacts.clear();
		m_Contacts.append(contacts);
	}

	inline void AddContact(AuContact& contact)
	{
		m_Contacts.append(contact);
	}

	inline void DeleteContact(AuContact& contact)
	{
		bool bDeleteOne=false;
		QList<AuContact>::iterator it = m_Contacts.begin();
		while(it != m_Contacts.end() && !bDeleteOne)
		{
			if(contact.m_id == (*it).m_id)
			{
				m_Contacts.erase(it);
				bDeleteOne = true;
			}
			else
				it++;
		}
	}

	inline void DeleteContacts(QList<AuContact>& contacts)
	{
		for(QList<AuContact>::iterator it = contacts.begin(); it != contacts.end(); it++)
			DeleteContact((*it));
	}

	// get the list of contacts
	inline const QList<AuContact>& GetContacts() const
	{
		return m_Contacts;
	}

	// Adjusts some mfg process times for solo play, so they don't take so long.
	void SoloAdjustGameData();

protected:
	void CalcCommodityBaseCost(DbCommodity& bCom);

	void CalcCommodityBaseCosts();

	void CalcIndustryHitPointValues();

public:
	// Guarded. Use AuGlobal::LockData functions.
	DbIndustryArray						m_bIndustryArray;				// From GD
	DbManufacturingComponentArray		m_bManufacturingComponentArray;	// From GD
	DbManufacturingProcessArray			m_bManufacturingProcessArray;	// From GD
	DgEmpireHash						m_gEmpires;
	DzSpeciesHash						m_Species;				// Names assigned to various DNA.

public:
	// Guarded. Use AuGlobal::LockBdDesigns functions.
	AuBdDesignHash*						m_BdDesigns;

public:
	// Guarded. Use AuGlobal::LockBdExchange functions.
	AuBdDesignExchange*					m_BdExchange;			// Library of published designs.
	AuExchangeBlueprintRatings*			m_BdExchangeRatings;

public:
	// Guarded. Use AuGlobal::LockItDesigns functions.
	AuItDesignHash*						m_ItDesigns;

public:
	// Guarded. Use AuGlobal::LockScDesigns functions.
	AuScDesignHash*						m_ScDesigns;

public:
	// Guarded. Use AuGlobal::LockScExchange functions.
	AuScDesignExchange*					m_ScExchange;			// Library of published designs.
	AuExchangeBlueprintRatings*			m_ScExchangeRatings;

private:
	// REMAINING VALUES ARE ONLY VALID AFTER SUCCESSFUL LOGIN TO THE LOGIN SERVER
	AuTime								m_AccountExpiry;
	ACCOUNT								m_AccountType;
	unsigned int						m_ActiveUnit;			// index of active unit in m_idUnits array.
	unsigned int						m_confirmationNumber;
	unsigned int						m_idPlayer;				// id of client g_Player record. NOT AN ATOM ID.
	COMMODITYID							m_UnitArmor[6];
	AuDNABase::CHROMATID				m_UnitChromatid[6];		// Chromatid of all units controlled by the player.
	COMMODITYID							m_UnitClothes[6];
	AuDNABase*							m_UnitDNA[6];			// dna of all units controlled by the player, or 0.
	AuHairStyle::STYLE					m_UnitHair[6];
	HANDEDNESS							m_UnitHand[6];			// handedness of all units controlled by the player
	COMMODITYID							m_UnitHat[6];
	AuId								m_UnitId[6];			// ids of all units controlled by the player.
	QString								m_UnitName[6];			// names of all units controlled by the player.
	AuHairStyle::SHAVE					m_UnitShave[6];
	quint8								m_UnitVisor[6];			// Item State() of the helmet, if any.

	QString								m_AcctName;				// Used for requerying and updating of Contacts
	QByteArray							m_AcctPassCrypt;		// Used for requerying and updating of Contacts. Is a SHA-1 key.

	// CONTACTS REQUIRE SUBSEQUENT QUERY	
	QList<AuContact>					m_Contacts;

private:
	bool								m_DataReceived;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Political stance can sometimes be determined without locking a mutex.
// This global function is helpful to get the political stance most efficiently.
inline POLITICAL_STANCE PoliticalStance(quint32 ofEmpireId, quint32 towardEmpireId)
{
	POLITICAL_STANCE stance;
	if(!ofEmpireId)
	{
		// 'of' unit has no empire.
		if(towardEmpireId)
			stance = POL_Enemy;	// Non empire units view empire units as enemy.
		else
			stance = POL_Neutral;	// Non empire units view non empire units as neutral.
	}
	else
	{
		// 'of' unit has an empire.
		if(!towardEmpireId)
			stance = POL_Enemy;	// Empire units view non empire units as enemy.
		else if(ofEmpireId == towardEmpireId)
			stance = POL_Empire;	// Empire units view fellow empire units as comrades.
		else
		{
			const DgData& gd = g_Au.LockDataForRead();
			stance = gd.EmpirePoliticalStance(ofEmpireId, towardEmpireId);
			g_Au.UnlockData();
		}
	}
	return stance;
}

// Determines a political stance incorporating a cache.
// This method is used by DaAtom::SensorScan functions to reduce the amount of mutex locks per sensor scan.
inline POLITICAL_STANCE PoliticalStance(quint32 ofEmpireId, quint32 towardEmpireId,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache)
{
	// Check the stanceCache.
	unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>::const_iterator sci = stanceCache.find(towardEmpireId);
	if(sci != stanceCache.end())
		return (*sci).second;

	// Empire not found in stanceCache. Determine the stance the hard way and cache the result.
	POLITICAL_STANCE stance = PoliticalStance(ofEmpireId, towardEmpireId);
	stanceCache.insert(pair<quint32, POLITICAL_STANCE>(towardEmpireId, stance));
	return stance;
}

// Returns a color based on a political stance.
//  Blue if political stance is POL_Empire.
//  Green if political stance is POL_Friend.
//  Grey if political stance is POL_Neutral.
//  Red if political stance is POL_Enemy.
inline QColor PoliticalStanceColor(POLITICAL_STANCE stance, bool isCivilian)
{
	if(isCivilian)
	{
		switch(stance)
		{
		case POL_Empire:
		case POL_Vassal:
			return QColor(192,32,255);	// Purple
		case POL_Enemy:
			return QColor(255,128,32);	// Orange
		case POL_Friend:
			return QColor(255,255,32);	// Yellow
		case POL_Neutral:
		default:
			return QColor(192,192,128);	// Dull Yellow
		}
	}
	else
	{
		switch(stance)
		{
		case POL_Empire:
		case POL_Vassal:	// Vassals treat their stance like empire.
			return QColor(96,96,255);	// Blue
		case POL_Enemy:
			return QColor(255,32,32);	// Red
		case POL_Friend:
			return QColor(64,255,64);	// Green
		case POL_Neutral:
		default:
			return QColor(192,192,192);	// Grey
		}
	}
}

// Returns a color based on the political stance of an empire toward some other empire.
//  Blue if political stance is POL_Empire.
//  Green if political stance is POL_Friend.
//  Grey if political stance is POL_Neutral.
//  Red if political stance is POL_Enemy.
inline QColor PoliticalStanceColor(quint32 ofEmpireId, quint32 towardEmpireId, bool isCivilian)
{
	return PoliticalStanceColor(PoliticalStance(ofEmpireId, towardEmpireId), isCivilian);
}

#endif // DGDATA_H
