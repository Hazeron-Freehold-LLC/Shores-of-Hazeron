//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: danpc.h,v 1.20 2022/05/17 16:09:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaNpc_H
#define DaNpc_H

#include "dacitizen.h"

class AuCommAuction;
class AuCommBid;
class AuCommBidBarter;
class AuCommForSale;

class AUATOM_DLL DaNpc : public DaCitizen  
{
private:
	bool								m_Con;					// True while this Npc thinks it has the con.
	quint8								m_Galaxy;				// Home galaxy of this Npc. GALAXY enum value.
	quint8								m_HomeSickness;			// +1 every day home world no longer has a city of my empire.
	quint8								m_Indestructible;		// True if I am indestructible.
	quint8								m_Role;					// NPC_ROLE
	quint8								m_Say;					// Say index. Not stored.
	quint8								m_WorldBodyType;		// Home planet body type.

private:
	mutable QReadWriteLock				m_NpcLock;
	double								m_AccountBalance;
	AuId								m_Companion;			// Avatar to whom I am assigned.
	AuId								m_GarrisonBd;			// Bd to which a troop is assigned, if any. May be 0.
	AuId								m_PortBd;				// Bd at which the last spacecraft was boarded|debarked.
	AuElapsedTimer						m_SpouseChange;			// When spouse name changed.
	QString								m_SpouseName;			// Blank indicates not married.
	quint32								m_StoryInstance;		// Story instance this character is associated with, if any.

	// Officers need to know their homeworld. Others sometimes benefit from this information.
	AuTime								m_HomeSickTime;			// When m_HomeSickness last changed.
	AuId								m_SectorId;				// Home sector of this Npc.
	QString								m_SectorName;			// Home sector of this Npc.
	AuId								m_SystemId;				// Home solar system of this Npc.
	QString								m_SystemName;			// Home solar system of this Npc.
	AuId								m_WorldId;				// Home planet of this Npc. Null for pirates.
	QString								m_WorldName;			// Home planet of this Npc.

	// Story characters sometimes want to pick up things.
	COMMODITYID							m_WantCID;				// Item commodity id, 0 = don't want anything
	quint8								m_WantQ;				// Item quality, 0 = don't care
	quint32								m_WantEoO;				// Item empire of origin, 0 = don't care
	QString								m_WantInscription;		// Item inscription, blank = don't care

private:
	mutable QReadWriteLock				m_LogLock;
	AuEventLog							m_Log;					// For a summary of noteworthy events in each report.
	AuTime								m_LogTime;				// Time the log was last sent to my chain of command.

private:
	// OpenAL
	unsigned int						m_AudioSource;			// audio source
	QVector<AuCitizenSound>				m_Sounds;				// sounds queued for playing

public:
	// Construction
	DaNpc(NPC_ROLE role, const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name);
	DaNpc(NPC_ROLE role, const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name);
	DaNpc();
	virtual ~DaNpc();

protected:
	DaNpc(const DaNpc &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual double AccountBalance() const;

	virtual bool AddGearItemUseDrop(const AuItem &cItem, int useIt, bool dropIt);

	virtual COMMODITYID Armor() const;

	virtual ATOM_TYPE AtomType() const;

	// Roles are restricted to what weapons and armor they can wear.
	virtual bool CanUseGear(const AuItem& item, AuGearSlot::SLOTTYPE slotType) const;

	virtual void ChooseHairStyle();

	virtual DaAtom* Clone() const;

	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	inline AuId Companion() const
	{
		m_NpcLock.lockForRead();
		AuId id(m_Companion);
		m_NpcLock.unlock();
		return id;
	}

	virtual void ConversationChoicePicked(DaPerson& person, qint8 pickValue);

	// The name used when addressed by a superior.
	virtual QString CrewName() const;

	virtual void CreditAccount(double amount);

	virtual void DebitAccount(double amount);

	virtual QString DescribeActivity() const;

	virtual DaNpc* FindCharacterRecursive() const;

	virtual DaNpc* FindCharacterRecursive(const QString& charName, quint32 storyInstance) const;

	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual void FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const;

	virtual const DaNpc* FindOfficerRecursive() const;

	virtual void FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const;

	virtual void FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const;

	virtual void FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const;

	virtual void FindTroopsRecursive(quint32 empireId, DaAtomList& troops);

	// Returns the id of the bd this troop is currently assigned to.
	inline AuId GarrisonBd() const
	{
		m_NpcLock.lockForRead();
		AuId v(m_GarrisonBd);
		m_NpcLock.unlock();
		return v;
	}

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	// Called after placement/spawn at nu job site, such as aboard a nuly assigned ship.
	virtual void GoToWork();

	virtual bool GrowNaturally() const;

	inline bool HasCompanion() const
	{
		m_NpcLock.lockForRead();
		bool v = m_Companion.IsNull();
		m_NpcLock.unlock();
		return !v;
	}

	virtual bool HasTheCon() const;

	virtual COMMODITYID Helmet() const;

	virtual GALAXY HomeGalaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	AuId HomeGalaxyId() const;

	QString HomeGalaxyName() const;

	inline AuId HomeSectorId() const
	{
		m_NpcLock.lockForRead();
		AuId v(m_SectorId);
		m_NpcLock.unlock();
		return v;
	}

	virtual QString HomeSectorName() const
	{
		m_NpcLock.lockForRead();
		QString v(m_SectorName);
		m_NpcLock.unlock();
		return v;
	}

	inline AuId HomeSolarSystemId() const
	{
		m_NpcLock.lockForRead();
		AuId v(m_SystemId);
		m_NpcLock.unlock();
		return v;
	}

	virtual QString HomeSolarSystemName() const
	{
		m_NpcLock.lockForRead();
		QString v(m_SystemName);
		m_NpcLock.unlock();
		return v;
	}

	inline BODY_TYPE HomeWorldBodyType() const
	{
		return (BODY_TYPE)m_WorldBodyType;
	}

	inline AuId HomeWorldId() const
	{
		m_NpcLock.lockForRead();
		AuId v(m_WorldId);
		m_NpcLock.unlock();
		return v;
	}

	virtual QString HomeWorldName() const
	{
		m_NpcLock.lockForRead();
		QString v(m_WorldName);
		m_NpcLock.unlock();
		return v;
	}

	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const;

	virtual void InitGear(AuItemHash& cinv, AuItemHash& itemsTaken, bool equipFully);

	void InitIndigenousGear(const AuZoneResource& res);

	virtual void InitPirateGear(quint8 maxQ);

	// Sets initial skills representing basic training, to the first rank level for officers, crew and troops.
	// Also initializes marital status.
	void InitTraining();

	virtual void IncreaseSkill(AuSkill::SKILL skill, quint32 xp);

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	virtual bool IsCivilian() const;

	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	inline bool IsIndestructible() const
	{
		return m_Indestructible;
	}

	virtual bool IsInStoryInstance(quint32 storyInstance) const;

	virtual bool IsMarried() const;

	virtual bool IsReadyForMating() const;

	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual bool IsStoryCharacter() const
	{
		return StoryInstance();
	}

	virtual bool IsTargetToWildAnimal() const;

	virtual unsigned int IsTocker() const;

	virtual quint32 JobExperiencePoints() const;

	virtual void Kill();	// Kills this atom and all subatoms recursively.

	// Called when this unit is killed by another unit.
	virtual void KilledByUnit(const DaUnit* killer);

	static DaNpc* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	inline const AuEventLog& LockEventLog() const
	{
		m_LogLock.lockForRead();
		return m_Log;
	}

	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId=AuId())
	{
		AuEventLogEntry logEntry(type, desc, atomId);
		m_LogLock.lockForWrite();
		m_Log.AddEventTrim(logEntry);
		m_LogLock.unlock();
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	virtual NPC_ROLE NpcRole() const
	{
		return Role();
	}

	virtual void OccupyBerth(DaVessel* ship);

	virtual AuId PortBd() const
	{
		m_NpcLock.lockForRead();
		AuId id(m_PortBd);
		m_NpcLock.unlock();
		return id;
	}

	// Returns the total number of experience points needed to get promoted to the next rank.
	// This is the total experience points needed for the rank, not the experience points remaining to get there.
	// 0 is returned if currently at the maximum rank.
	virtual quint32 PromotionGoal() const;

	// Returns the total number of experience points needed to get promoted to the next rank above 'rank'.
	// This is the total experience points needed for the rank, not the experience points remaining to get there.
	// 0 is returned if 'rank' is the maximum rank.
	quint32 PromotionGoal(quint8 rank) const;

	// This is only called on the scene client.
	void QueueSound(const QByteArray& soundData, AuSound& al);

	virtual quint8 Rank() const;

	virtual QString RankAbbr() const;

	// Rank when addressed by a superior. e.g. Sergeant instead of MSGT.
	virtual QString RankCrew() const;

	virtual QString RankIcon() const;

	virtual QString RankName() const;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void ReleaseTheCon();

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	inline NPC_ROLE Role() const
	{
		return (NPC_ROLE)m_Role;
	}

	virtual void RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup=true);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Calls SayIndigenousXXXX according to my role.
	void SayIndigenous(const DaWorldLL& llworld, DaPerson& person);

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

	void SetCompanion(const AuId& companion);

	virtual void SetGarrisonBd(const AuId& bdId)
	{
		m_NpcLock.lockForWrite();
		m_GarrisonBd = bdId;
		m_NpcLock.unlock();
		SetModified(NOTIFY_Database);
	}

	// Sets the posture of this unit to be the default posture when told to garrison
	virtual void SetGarrisonPosture();

	inline bool SetHomeSectorName(const QString& sectorName)
	{
		m_NpcLock.lockForRead();
		bool changeIt = m_SectorName != sectorName;
		m_NpcLock.unlock();
		if(!changeIt)
			return false;

		m_NpcLock.lockForWrite();
		m_SectorName = sectorName;
		m_NpcLock.unlock();
		return true;
	}

	// 'homeWorld' can be null to clear home world values.
	void SetHomeWorld(const DaWorld* homeWorld);

	inline void SetHomeWorld(const AuId& worldId, const QString& worldName, BODY_TYPE worldBodyType, const AuId& systemId,
			const QString& systemName, const AuId& sectorId, const QString& sectorName, GALAXY galaxy)
	{
		m_NpcLock.lockForWrite();
		m_WorldId = worldId;
		m_WorldName = worldName;
		m_SystemId = systemId;
		m_SystemName = systemName;
		m_SectorId = sectorId;
		m_SectorName = sectorName;
		m_NpcLock.unlock();
		m_Galaxy = galaxy;
		m_WorldBodyType = worldBodyType;
	}

	void SetIndestructible(bool isIndestructible);

	inline void SetPortBd(const AuId& bdId)
	{
		m_NpcLock.lockForWrite();
		m_PortBd = bdId;
		m_NpcLock.unlock();
	}

	// Changes my Npc role.
	// Fails if I am holding a berth.
	// Returns true on success, false on failure.
	bool SetRole(NPC_ROLE role);

	void SetSpouse(const QString& name);

	inline void SetStoryInstance(quint32 storyInstance)
	{
		m_StoryInstance = storyInstance;
	}

	// 'notified' is an output parameter, filled with atom id's who are sent notifications.
	virtual void ShipLogFinal(const DaVessel* ship, const QString& reasonFinal, const QString& killerName, COMMODITYID weaponCID,
			quint8 weaponQ, AuIdList& notified);

	virtual QString SpouseName() const
	{
		m_NpcLock.lockForRead();
		QString v(m_SpouseName);
		m_NpcLock.unlock();
		return v;
	}

	virtual void StartVoiceConversationRecursive(const DaPerson& talker) const;

	inline quint32 StoryInstance() const
	{
		return m_StoryInstance;
	}

	quint32 StoryScript() const;

	virtual void TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
			bool firstInGroup, const QString& orderText=QString(""));

	virtual void TakeTheCon();

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual TRANSPORTER_TARGET TransporterTarget() const;

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	// Returns true if this unit can be selected by the scene atom to be given orders.
	virtual bool UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const;

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* victim);

	inline void UnlockEventLock()
	{
		m_LogLock.unlock();
	}

	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	// Makes this character desire 'cid'. If someone offers one for sale, I will make a bid.
	void WantItem(COMMODITYID cid);

	// Makes this character desire 'cid', with specific 'quality' and 'empireOfOrigin'. 'inscription' is only compared if it is
	// not blank.
	// If someone offers this exact commodity for sale, I will make a bid.
	void WantItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& inscription);

	virtual COMMODITYID WornArmor() const;

	virtual COMMODITYID WornClothing() const;

	virtual COMMODITYID WornHat() const;

protected:
	virtual void BodyHasDied(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
			const QString& killerName);

	void CharacterRxAuction(const AuCommAuction* msg);
	void CharacterRxBid(const AuCommBid* msg);
	void CharacterRxBidBarter(const AuCommBidBarter* msg);
	void CharacterRxForSale(const AuCommForSale* msg);

	// Notifies chain-of-command of my location when I am not in command or I am lost.
	virtual void ContactLog();

	void DeathLog(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2, const QString& whoName);

	virtual void DoBattleStation();
	virtual void DoGarrison();
	virtual void DoScan();

	virtual bool GainExperienceForWorking();

	virtual bool GetACheckup();

	void IndigenousRxBid(const AuCommBid* msg);
	void IndigenousRxBidBarter(const AuCommBidBarter* msg);

	// Returns true if a terse reply is given, indicating that no further conversation is wanted.
	bool IndigenousTerseReply(const AuCommPacket* msg, const DaWorldLL* llworld);

	void LookForClosestPerson(DaPerson*& person, AuPoint& personLoc, double& personRange) const;

	void MinuteTock();

	// Render OpenAL audio sources
	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	virtual void RxHailReply(const AuCommPacket* msg);

	virtual void RxHailRequest(const AuCommPacket* msg);

	// Adult indigenous says something to 'person'.
	void SayIndigenousAdult(const DaWorldLL& llworld, const DaPerson& person);

	// Child indigenous says something to 'person'.
	void SayIndigenousChild(const DaWorldLL& llworld, const DaPerson& person);

	// Elder indigenous says something to 'person'.
	void SayIndigenousElder(const DaWorldLL& llworld, const DaPerson& person);

	// Doctor indigenous says something to 'person'.
	void SayIndigenousHealer(const DaWorldLL& llworld, DaPerson& person);

	void SayIndigenousSomething(const QString statements[], int statementCount, const DaWorldLL& llworld, const DaPerson& person);

	// Star Gazer indigenous says something to 'person'.
	void SayIndigenousStargazer(const DaWorldLL& llworld, const DaPerson& person);

	void SayWorkerOpinion(const DaPerson& person);

	// May destroy the ship.
	virtual void ShipLog(DaVessel* ship);

	void SolicitSales(COMMODITYFILTER filter, COMMODITYID cid, const AuCommPacket* msg);

	virtual void StayIdle();

	virtual void TakePosture(const AuCommPacket& msg, CITIZEN_POSTURE order, bool firstInGroup);

	void UpgradeEquipmentFromBd();
};

#endif // DaNpc_H
