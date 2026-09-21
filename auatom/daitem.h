//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daitem.h,v 1.270 2022/05/17 16:07:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaItem_H
#define DaItem_H

#include "audnabase.h"
#include "daatom.h"

class Au3dsModelInstance;
class AuItem;
class DeConstructionProcess;
class DeManufacturingProcess;
class DpBody;

class AUATOM_DLL DaItem : public DaAtom  
{
protected:
	enum CONTROLINPUT
	{
		CI_PickUp,
		CI_PickProduce,
		CI_Grow1,
		CI_Grow2,
		CI_Grow3,
		CI_Shrink1,
		CI_Shrink2,
		CI_Shrink3,
		CI_PickProduceCID,
		CI_PickProduceSeed,
	};

private:
	bool								m_StaticDisplay;		// True if this item is on static display.

private:
	mutable QReadWriteLock				m_ItemLock;
	AuItem*								m_Item;					// The commodity that this object represents.
	AuElapsedTimer						m_ItemChangeTime;		// For triggering data updates. Not serialized.

private:
	mutable QReadWriteLock				m_OwnerLock;
	AuElapsedTimer						m_OwnerChangeTime;		// For triggering data updates. Not serialized.
	quint32								m_OwnerEmpire;
	AuId								m_OwnerId;				// Character who 'set' this object, for detonated and fused items.
	QString								m_OwnerName;			// Character who 'set' this object, for detonated and fused items.
	QString								m_StaticRoom;			// Static display room name in sc or bd, for lighting. Can be blank.

private:
	AuElapsedTimer						m_LastForage;			// Not stored in db.

public:
	DaItem(const AuItem& item);
	DaItem();
	virtual ~DaItem();

protected:
	DaItem(const DaItem &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	inline quint32 ArmedByEmpire() const
	{
		m_OwnerLock.lockForRead();
		quint32 v = m_OwnerEmpire;
		m_OwnerLock.unlock();
		return v;
	}

	// Causes detonator and fuse weapons to become 'armed' by the 'id' and 'name' passed.
	// Fuse weapons will explode shortly.
	// Detonator weapons will explode when the detonate signal is received from the owner.
	// 'id' and 'name' will get credit for kills resulting from the explostion.
	inline void ArmWeapon(const AuId& id, const QString& name, quint32 empireId)
	{
		Q_ASSERT(!IsStaticDisplay());
		m_OwnerLock.lockForWrite();
		m_OwnerChangeTime.Set();
		m_OwnerEmpire = empireId;
		m_OwnerId = id;
		m_OwnerName = name;
		m_OwnerLock.unlock();
	}

	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire);

	virtual bool CanSetInMotion() const
	{
		if(!DaAtom::CanSetInMotion())
			return false;
		return !IsStaticDisplay();	// Static displayed items cannot be set in motion.
	}

	virtual QString CapsuleRoomName() const;

	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	virtual DaAtom *Clone() const;

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

	void Create();

	virtual bool DepartingSolarSystem();

	// Begins detonation of an explosive. Detonation completes in the next Tick cycle by calling DetonateComplete.
	// That way chain reactions of lots of bombs don't freeze the server while they all explode simultaneously.
	void DetonateInitiate();

	AuDNABase::CHROMATID DNAChromatid() const;

	inline GROWTHSTAGE DNAGrowthStage() const
	{
		m_ItemLock.lockForRead();
		GROWTHSTAGE stage = m_Item ? m_Item->DNAGrowthStage() : Mature;
		m_ItemLock.unlock();
		return stage;
	}

	inline bool DNAIsSentient() const
	{
		m_ItemLock.lockForRead();
		bool is = m_Item ? m_Item->DNAIsSentient() : false;
		m_ItemLock.unlock();
		return is;
	}

	inline bool DNAIsViable() const
	{
		m_ItemLock.lockForRead();
		bool is = m_Item ? m_Item->DNAIsViable() : false;
		m_ItemLock.unlock();
		return is;
	}

	// Caller must delete the pointer.
	inline AuDNABase* DNAnew() const
	{
		AuDNABase* dna;

		m_ItemLock.lockForRead();
		if(m_Item && m_Item->DNA())
			dna = m_Item->DNA()->Clone();
		else
			dna = 0;
		m_ItemLock.unlock();

		return dna;
	}

	bool DoesExplodeWhenDamaged() const;

	// Return true for atoms that require QueryEnvironment information.
	// Atoms that can have a movement vector must return true.
	virtual bool DoesQueryEnvironment() const
	{
		return true;
	}

	inline quint32 EmpireOfOrigin() const
	{
		m_ItemLock.lockForRead();
		quint32 e = m_Item ? m_Item->EmpireOfOrigin() : 0;
		m_ItemLock.unlock();
		return e;
	}

	virtual double FallSafeSpeed() const;

	virtual void FilterFinalMoveVector(AuPoint& vector) const;

	virtual void FilterPostMoveVector(AuPoint& vector) const;

	virtual void FindBeaconsRecursive(DaAtomList& cities) const;

	virtual void FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const;

	inline short int FoodNutritionValue(ECOROLE ecoRole) const
	{
		short int f;

		m_ItemLock.lockForRead();
		if(m_Item)
			f = m_Item->FoodNutritionValue(ecoRole);	// Return food nutrition value of food, which could be zero.
		else
			f = 0;
		m_ItemLock.unlock();

		return f;
	}

	// Returns the vector that corresponds to 'up' when this item lies on the ground.
	AuPoint GroundUpVector() const;

	inline bool HasItem() const
	{
		return m_Item;
	}

	inline QString HtmlDesc() const
	{
		m_ItemLock.lockForRead();
		QString i = m_Item ? m_Item->HtmlDesc() : "";
		m_ItemLock.unlock();
		return i;
	}

	inline QString HtmlToolTip() const
	{
		m_ItemLock.lockForRead();
		QString i = m_Item ? m_Item->HtmlToolTip() : "";
		m_ItemLock.unlock();
		return i;
	}

	inline void Inscribe(const QString& inscription)
	{
		m_ItemLock.lockForWrite();
		if(m_Item)
			m_Item->Inscribe(inscription);
		m_ItemLock.unlock();
	}

	inline QString Inscription() const
	{
		m_ItemLock.lockForRead();
		QString i = m_Item ? m_Item->Inscription() : "";
		m_ItemLock.unlock();
		return i;
	}

	virtual bool IsEthereal() const;

	inline QString SayName() const
	{
		m_ItemLock.lockForRead();
		QString i = m_Item ? m_Item->SayName() : "";
		m_ItemLock.unlock();
		return i;
	}

	virtual bool IsActiveMissile() const
	{
		return false;
	}

	inline bool IsAvoidedByWildAnimals() const
	{
		return DbCommodity::IsAvoidedByWildAnimals(CommodityId());
	}

	inline bool IsDetonatorOrFuseWeapon() const
	{
		m_ItemLock.lockForRead();
		bool is  = m_Item && (m_Item->IsDetonatorWeapon() || m_Item->IsFuseWeapon());
		m_ItemLock.unlock();
		return is;
	}

	inline bool IsDNA(const AuDNABase* dna) const
	{
		m_ItemLock.lockForRead();
		bool is  = m_Item && m_Item->DNA() && *m_Item->DNA() == *dna;
		m_ItemLock.unlock();
		return is;
	}

	inline bool IsFlowerPot() const
	{
		return DbCommodity::IsFlowerPot(CommodityId());
	}

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsStaticDisplay() const
	{
		return m_StaticDisplay;
	}

	// Returns true if this is a static display item that can be controlled by 'personId'.
	bool IsStaticDisplayControllableBy(const AuId& personId, quint32 personEmpireId) const;

	inline bool IsPicture() const
	{
		return DbCommodity::IsPicture(CommodityId());
	}

	inline bool IsThreatToWildAnimals() const
	{
		return DbCommodity::IsThreatToWildAnimals(CommodityId());
	}

	inline quint32 ItemCount() const
	{
		m_ItemLock.lockForRead();
		quint32 v = m_Item ? m_Item->Count() : 0;
		m_ItemLock.unlock();
		return v;
	}

	inline QString ItemMessage() const
	{
		m_ItemLock.lockForRead();
		QString v = m_Item ? m_Item->Message() : 0;
		m_ItemLock.unlock();
		return v;
	}

	inline quint8 ItemState() const
	{
		m_ItemLock.lockForRead();
		quint8 v = m_Item ? m_Item->State() : 0;
		m_ItemLock.unlock();
		return v;
	}

	inline AuItem::ITEMTYPE ItemType() const
	{
		m_ItemLock.lockForRead();
		AuItem::ITEMTYPE v = m_Item ? (AuItem::ITEMTYPE)m_Item->Type() : AuItem::IT_Invalid;
		m_ItemLock.unlock();
		return v;
	}

	inline const AuItem* LockItem() const
	{
		m_ItemLock.lockForRead();
		return m_Item;
	}

	inline AuItem* LockItemForWrite() const
	{
		m_ItemLock.lockForWrite();
		return m_Item;
	}

	inline void UnlockItem() const
	{
		m_ItemLock.unlock();
	}

	virtual quint64 MemoryUsage() const;

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	virtual QString Name() const;

	QString OwnerName() const
	{
		m_OwnerLock.lockForRead();
		QString v(m_OwnerName);
		m_OwnerLock.unlock();
		return v;
	}

	// Returns true if the 'seedItem' was viable and I am a flower pot that is not on static display.
	bool PlantSeed(const AuItem& seedItem);

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

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	void SetItemMessage(const QString& text);

	void SetItemState(quint8 state);

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	// Clears the 'storyInstance' of an item that is currently associated with a story instance.
	// This prevents the item from self destructing after the story instance ends.
	void StoryMakePermanent(quint32 storyInstance);

	// Sets the 'drop' state of an item associated with a story instance.
	// 'drop' 0=default random drop, 1=never drop, 2=always drop
	void StorySetDropOnDeath(quint32 storyInstance, int drop);

	virtual void TestHullStress();

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual TRANSPORTER_TARGET TransporterTarget() const;

	virtual QString TransporterTargetName() const;

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	// "Uses" this item so it might decay.
	// Animal carcass turns into a bone when used up.
	// This item is 'Kill'ed when completely used up.
	void UseItem();

public:
	virtual float GroundHeightAboveCG() const;
	virtual bool RoughExtents(AuBox &fExt) const;

	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	virtual bool OpensAutomaticDoors() const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal);

	virtual double OrientationSpeed() const;

	inline bool IsArmedDetonatorBomb() const
	{
		if(IsStaticDisplay())
			return false;

		m_ItemLock.lockForRead();
		bool is = m_Item && !m_OwnerId.IsNull() && m_Item->IsDetonatorWeapon();
		m_ItemLock.unlock();
		return is;
	}

	inline bool IsArmedFuseBomb() const
	{
		if(IsStaticDisplay())
			return false;

		m_ItemLock.lockForRead();
		bool is = m_Item && !m_OwnerId.IsNull() && m_Item->IsFuseWeapon();
		m_ItemLock.unlock();
		return is;
	}

	inline AuId ArmedBy() const
	{
		m_OwnerLock.lockForRead();
		AuId v(m_OwnerId);
		m_OwnerLock.unlock();
		return v;
	}

	inline bool IsArmedBy(const AuId& id) const
	{
		m_OwnerLock.lockForRead();
		bool v = m_OwnerId == id && !IsStaticDisplay();
		m_OwnerLock.unlock();
		return v;
	}

	inline bool IsDetonatorWeapon() const
	{
		if(IsStaticDisplay())
			return false;

		m_ItemLock.lockForRead();
		bool is = m_Item && m_Item->IsDetonatorWeapon();
		m_ItemLock.unlock();
		return is;
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	bool FetchConstructionMaterials(DeConstructionProcess &cp);

	// If 'elecSubQ' is non-zero, electricity can be substituted for sunlight but the elecSubQ is used.
	// If 'waterSubQ' is non-zero, bottled water can be substituted for hydrographics but the waterSubQ is used.
	void FetchManufacturingMaterials(DeManufacturingProcess &mp, quint8 elecSubQ, quint8 waterSubQ);

	// Reduces the condition of the item by one point.
	// When the item reaches zero condition, this item is Kill()'d or turned into a bone, in the case of an animal carcass.
	// Returns 'true' when killed or converted to a bone.
	// This function is only known to be used by animals who are eating a carcass. They get the food nutrition value when the item
	// is gone.
	inline bool DegradeCondition()
	{
		if(IsStaticDisplay())
			return false;

		m_ItemLock.lockForWrite();
		if(!m_Item)
		{
			m_ItemLock.unlock();
			Kill();				// Kill this broken item.
			return true;
		}

		if(m_Item->DegradeCondition())
		{
			// Item was not fully consumed.
			m_ItemLock.unlock();
			SetModified();
			return false;
		}
		else if(m_Item->CommodityId() == CID_Animal || m_Item->CommodityId() == CID_Head)
		{
			// Animal carcass and head turns into a bone. Since both are AuItemDNA objects, the DNA is preserved.
			m_Item->SetCommodity(CID_Bone, m_Item->Quality());
			m_Item->ResetCondition();
			m_ItemChangeTime.Set();
			m_ItemLock.unlock();

			SetInMotion();
			TickTock();
			SetModified();

			Orient(Location(), Transform().ProjectVector(TerrainNormal()));
			return true;		// Animal was consumed. Bone remains.
		}
		else
		{
			m_ItemLock.unlock();
			Kill();				// Item was consumed.
			return true;
		}
	}

	/*
	inline void SetCapacity(const QSize &szCap)
	{
		m_cItem.SetCapacity(szCap);
	}

	inline void SetCommodity(COMMODITYID lCID, unsigned char byQuality, unsigned char techLevel, quint32 empireOfOrigin)
	{
		m_cItem.SetCommodityOrigin(lCID, byQuality, techLevel, empireOfOrigin);
		m_cItem.InitDbData();

		// Set the count to 1 for items that don't store data.
		// Data-storing items implicitly have a count of 1. Calling SetCount may mess them up.
		if(!m_cItem.CanStoreData())
			m_cItem.SetCount(1);
	}

	inline void SetItemData(int intData, const QString& stringData)
	{
		m_cItem.SetData(intData, stringData);
	}
	*/

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	inline AuPoint SingleObjectSize() const
	{
		AuPoint p;

		m_ItemLock.lockForRead();
		if(m_Item)
			p = m_Item->SingleObjectSize();
		m_ItemLock.unlock();

		return p;
	}

	inline COMMODITYID CommodityId() const
	{
		COMMODITYID c;

		m_ItemLock.lockForRead();
		if(m_Item)
			c = m_Item->CommodityId();
		else
			c = CID_Invalid;
		m_ItemLock.unlock();

		return c;
	}

	inline quint8 Quality() const
	{
		quint8 q;

		m_ItemLock.lockForRead();
		if(m_Item)
			q = m_Item->Quality();
		else
			q = 0;
		m_ItemLock.unlock();

		return q;
	}

	// In usage this function is looking to discard a container after fetching materials from inside it.
	// Empty crates, boxes and containers can be taken in that case.
	inline bool IsCrate() const
	{
		return CommodityId() == CID_Crate ||
			CommodityId() == CID_ParcX ||
			CommodityId() == CID_ContainerTEU;
	}

	// Physics
	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	virtual float Density() const
	{
		return (float)DensityOfWater();	//DHH
	}

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	// Returns the mass of this atom, in pounds.
	virtual float Mass() const
	{
		float m;

		m_ItemLock.lockForRead();
		if(m_Item)
			m = m_Item->Weight();
		else
			m = 0.f;
		m_ItemLock.unlock();

		return m;
	}

	virtual void SetAtRest();

	inline void SetStaticDisplay(const AuId& ownerId, const QString& ownerName, const QString& inRoomName)
	{
		m_OwnerLock.lockForWrite();
		Q_ASSERT(m_OwnerId.IsNull());
		m_OwnerChangeTime.Set();
		m_OwnerId = ownerId;
		m_OwnerName = ownerName;
		m_StaticRoom = inRoomName;
		m_OwnerLock.unlock();

		m_StaticDisplay = true;

		m_Item->DNAKillRecursive();
	}

	inline void SetStaticRoom(const QString& inRoomName)
	{
		m_OwnerLock.lockForWrite();
		m_OwnerChangeTime.Set();
		m_StaticRoom = inRoomName;
		m_OwnerLock.unlock();
	}

	// User interface functions.
	virtual bool IsControl(const DaAtom* sceneAtom) const;

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);
	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
			DiControlInputArray &aActiveControls, bool bContinuous);

	inline QString StaticRoom() const
	{
		m_OwnerLock.lockForRead();
		QString v(m_StaticRoom);
		m_OwnerLock.unlock();
		return v;
	}

public:
	static DaItem* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

protected:
	virtual AuPoint Acceleration() const;

	QString BeaconMessage() const;

	void DetonateTerminate();

	void EnumSightSurfaces(AuQuadArray &solidSurfs, const AuMatrix4x4 &tModel) const;

	// 'loc' is relative to me. (0,0) is my location.
	void MakeCrater(const AuPoint& loc, const AuCombatWeapon& weapon);

	void RenderItem(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, double scale);

	// 'sightedPart' will be the index into the array of surfaces returned by EnumSightSurfaces.
	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc, unsigned int& sightedPart,
			bool& sightedShield) const;
};

#endif // DaItem_H
