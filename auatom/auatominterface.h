//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatominterface.h,v 1.11 2022/05/06 15:05:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomInterface_H
#define AuAtomInterface_H

#include "auatom.h"
#include "audossier.h"
#include "auexchangerating.h"
#include "augear.h"
#include "auitem.h"
#include "aujoboffer.h"
#include "auoffice.h"
#include "auroster.h"
#include "aushipment.h"
#include "autransportertarget.h"
#include "deconstructionprocess.h"
#include <QObject>

class AuArenaReady;
class AuCityReport;
class AuCityReportHistory;
class AuDesign;
class AuDesignPosition;
class AuDesignTex;
class AuDNA;
class AuGearPos;
class AuId;
class AuItDesign;
class AuJob;
class AuMatrix4x4;
class AuPoint;
class AuPointArray;
class AuScDesign;
class AuScFaceList;
class AuScIdSet;
class AuScPart;
class AuScParts;
class AuScript;
class AuScSelection;
class AuSensorBlipList;
class AuShipment;
class AuSpecies;
class AuStarMapData;
class AuTime;
class DaCitizen;
class DaPerson;
class DaVessel;

class AuAtomInterface : public QObject
{
	Q_OBJECT;
public:	// Construction
	AuAtomInterface();
	virtual ~AuAtomInterface();
	AuAtomInterface(const AuAtomInterface& c);

public:	// Virtual Methods
	// Could return 0. Must call UnlockAvatar regardless of the return value.
	virtual DaPerson* LockAvatar() = 0;

	virtual void SetAvatarId(const AuId& avatarId) = 0;

	virtual void SetSystemId(const AuId& systemId) = 0;

	virtual void UnlockAvatar() = 0;

public:	// Methods
	void AgentGoToAtom(const AuId& destAtomId, bool rideAlso, bool limboRescue);

	void AgentGoToGalaxyLoc(quint8 galaxyEnum, const AuPoint& galaxyLoc, bool rideAlso);

	void AgentGoToLongLat(const AuGlobeLLF& ll, bool rideAlso);

	// 'type' 0=animal encounter, 1=pirate station, 2=pirate ship, 3=load officer, 4=load crew, 5=load troops, 6=load passengers
	void AgentEncounter(quint8 type, quint32 designId);

	void AgentEncounterAsteroid(quint32 minutesFromNow);

	void AgentRecoverShip(const AuId& shipId);

	void AgentResetStory(const AuId& avatarId, quint32 scriptId);

	void AgentScuttleShip(const QString& reason);

	void AgentSetBits(quint8 bits);

	void AgentSetDNA(const AuId& chgWho, const AuDNABase& nuDNA);

	// Sets alert condition for current ship.
	void AlertCondition(int condition);

	void ArenaCreateNew(const AuArenaReady& arena);

	void ArenaExit();

	void ArenaJoin(const AuId& arenaId, const QString& password, quint32 designId, quint8 team);

	void ArenaList(unordered_map<AuId, AuArenaReady, AuIdHash>& arenas);

	void ArenaRecall();

	void ArenaSetPassword(const QString& pass);

	void ArenaStatus();

	// Retrieves the 'personId' and 'personName' of the current avatar.
	// Returns false if no avatar is found.
	bool Avatar(AuId& personId, QString& personName);

	void BankLoan(double amount, const AuId& opBd);

	void BankPay(double amount, const AuId& opBd);

	void BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk);

	// Building a warehouse or storage tank.
	void BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk, COMMODITYID storeCID);

	void BdConstructionEnabled(bool enabled, const AuId& opBd);

	void BdConstructionFetch(const AuId& opBd);

	void BdConstructionGo(const AuId& opBd);

	void BdDesignExchangePublish(AuBdDesignExchangeEntry& exchangeItem);

	void BdDesignExchangePurchase(quint32 designId, const AuGearPos& mediaPos);

	void BdDesignExchangeRate(const AuExchangeRatingRec& rating);

	// Unpublishes a design made by this architect.
    // 'deleting' is true when the design is being unpublished in the process of being deleted.
    void BdDesignExchangeUnpublish(bool isArchitect, quint32 designId, bool deleting);

	void BdDiscardItem(const AuItem& buyItem, const AuId& opBd);

	void BdFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& onPlatform, quint32 customDesignId,
			int mat);

	void BdFoundCity(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk, const QString& cityName, const QString& solarSystemName,
			const QString& sectorNameGiven);

	void BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd);

	void BdManufactureCitizenEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd);

	void BdManufactureClear(const AuId& opBd);

	void BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureFetch(quint16 panelIndex, const AuId& opBd);

	void BdManufactureFetchMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos, const AuId& opBd);

	void BdManufactureGo(quint16 panelIndex, const AuId& opBd);

	void BdManufactureGoMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureLowerMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureMaxMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureMinMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessSelected(quint16 panelIndex, qint32 mpid, const AuId& opBd);

	void BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd);

	void BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd);

	void BdManufactureRaiseMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureRecordPatents(const AuKnownPatents& patents, const AuId& opBd);

	void BdManufactureSetLivestock(const AuSpecies& species, const AuId& opBd);

	void BdManufactureSetMinQ(quint8 minQ, const AuId& opBd);

	void BdManufactureSetWarehouseStorage(COMMODITYID cid, const AuId& opBd);

	void BdManufactureSpacecraftBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd);

	void BdManufactureSpacecraftCitizenEnabled(quint32 enabled, const AuId& opBd);

	void BdManufactureSpacecraftCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd);

	void BdManufactureSpacecraftOrderDelete(quint32 designId, const AuId& opBd);

	void BdManufactureSpacecraftProcessCanceled(const AuId& opBd);

	void BdManufactureSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ,
			const AuId& opBd);

	void BdManufactureSpacecraftSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd);

	void BdManufactureSpacecraftSetMinQ(quint8 minQ, const AuId& opBd);

	void BdPreview(const AuPoint& fBuildLoc, float zRotation);

	void BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc);

	void BdStoreBuyItem(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd);

	void BdTakeItem(const AuItem& buyItem, const AuId& opBd);

	// Returns:
	// 0 = ok
	// 1 = person not found
	// 2 = bd not found
	// 3 = person not in chain of command of bd
	int BrokerShipments(AuShipmentList& shipments, AuTradeAgreementList& tas, const AuId& opBd);

	void CallTurboLift(const AuId& shipId, quint16 liftId, const QString& stop);

	void ChangeRoadConstruction(const AuId& cityId, const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid);

	void Christen(const QString& name);

	void ClaimSpacecraftTitle();

	void ClearGeo(const AuGlobeLLF& geoLL);

	void CloseJob(const AuJob* jobToClose);

	void CloseStory(quint32 storyInstance);

	void CollectBounty(const AuGearPos& pos);

	// 'mode' 0=universe, 1='galaxy', 2=sector centered at 'sectorId' for 'range' radius in sectors
	// 'personSystemId' is the solar system containing the scene atom when the request for the starmap is made
	static void CollectStarMapData(quint32 empireId, AuStarMapData& starMap, int query, GALAXY gal, const AuId& sectorId,
			unsigned int range, const AuId& personSystemId);

	void CopySpacecraftTitle(const AuGearPos& mediaPos);

	void CreateNewEmpire(const QString& ename, const QImage& imFlag);

	void DemolishBd(const AuId& bdId);

	void DepositShipFunds(double amount, const AuId& shipId);

	void DesignerAddPartObjects(quint16 partId, const AuScParts& parts);

	void DesignerAddParts(const AuScParts& parts);

	// 'isArchitect' is true if the person's account is an architect.
	void DesignerArchitectDeleteBd(bool isArchitect, quint32 designId);

	// 'isArchitect' is true if the person's account is an architect.
	void DesignerArchitectDeleteIt(bool isArchitect, quint32 designId);

	// 'isArchitect' is true if the person's account is an architect.
	void DesignerArchitectDeleteSc(bool isArchitect, quint32 designId);

	void DesignerArchitectFinalizeBdAssembly(AuBdDesign& design, QSet<quint32>& xrefs);

	// 'designId' may be specified by an agent to assign a specific id. Existing design is overwritten, if any.
	void DesignerArchitectFinalizeBdDesign(AuBdDesign& design, QSet<quint32>& xrefs, quint32 designId=0);

	void DesignerArchitectFinalizeItAssembly(AuItDesign& design);

	void DesignerArchitectFinalizeItDesign(AuItDesign& design, quint32 designId=0);

	void DesignerArchitectFinalizeScAssembly(AuScDesign& design);

	// 'designId' may be specified by an agent to assign a specific id. Existing design is overwritten, if any.
	void DesignerArchitectFinalizeScDesign(AuScDesign& design, quint32 designId=0);

	void DesignerArchitectSaveBd(AuBdDesign& design);

	void DesignerArchitectSaveIt(AuItDesign& design);

	void DesignerArchitectSaveSc(AuScDesign& design);

	void DesignerAssociateParts(const AuScSelection& partIds);

	void DesignerCheckDoors(const AuScSelection& partIds);

	void DesignerCopyPartObjects(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerCopyParts(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerCopyPartsSymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane);

	void DesignerCutFaces(const AuScSelection& sel, const AuPointArray& cutLine);

	void DesignerDeletePartObjects(const AuScSelection& partIds);

	void DesignerDeleteParts(const AuScSelection& partIds);

	void DesignerDissociateParts(const AuScSelection& partIds);

	void DesignerDivideFaces(const AuScSelection& sel, quint8 divisions);

	void DesignerEffectIntensityVertices(const AuScSelection& sel, float intensity);

	void DesignerFlattenVertices(const AuScSelection& sel);

	void DesignerFractureFaces(const AuScSelection& sel);

	void DesignerFuseFaces(const AuScSelection& sel);

	// Returns design studio mode.
	quint8 DesignerGearOpen(quint32 designId, const AuGearPos& pos);

	void DesignerGearSaveBd(const AuBdDesignExchangeEntry& e, const AuGearPos& pos);

	void DesignerGearSaveSc(const AuScDesignExchangeEntry& e, const AuGearPos& pos);

	void DesignerGlowVertices(const AuScSelection& sel, const QColor& glow);

	void DesignerGoToSelection(const AuScSelection& sel);

	void DesignerGridPositionSave(quint8 number, const AuMatrix4x4& gridM);

	void DesignerJigCut(const AuScSelection& sel);

	void DesignerJoinParts(quint16 partId, const AuScSelection& sel);

	void DesignerLoadedFromFile(AuDesign& design);

	void DesignerMakeDecal(const AuScSelection& partIds);

	void DesignerMakeDoor(const AuScSelection& partIds);

	void DesignerMakeHull(const AuScSelection& partIds);

	void DesignerMakeLandingGear(const AuScSelection& partIds);

	void DesignerMakeLightLens(const AuScSelection& partIds, const QColor& lightColor);

	void DesignerMakeMesh(const AuScSelection& partIds);

	void DesignerMakeNavLightLens(const AuScSelection& partIds, const QColor& color);

	void DesignerMakeRoom(const AuScSelection& partIds);

	void DesignerMakeRotatingBeaconLens(const AuScSelection& partIds, const QColor& lightColor, const AuPoint& rotationAxis);

	void DesignerMakeStrobeLens(const AuScSelection& partIds, const QColor& lightColor);

	void DesignerMakeTurboLift(const AuScSelection& partIds);

	void DesignerMakeTurret(const AuScSelection& partIds, const AuPoint& basePivot, const AuPoint& baseAxis);

	void DesignerMakeWindow(const AuScSelection& partIds, float opacity);

	void DesignerMoveLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerMovePartObjects(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerMoveParts(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerNew(const AuDesign& design);

	void DesignerPaintFaces(const AuScSelection& sel, const QColor& paint);

	void DesignerPaintFaces(const AuScSelection& sel, quint8 theme, quint8 opacity);

	void DesignerPartAdd(const AuScPart& part);

	void DesignerPartAddFaces(quint16 partId, const AuScFaceList& faces);

	void DesignerPartAddState(quint16 partId, quint8 state);

	void DesignerPartAddStop(quint16 partId, const QString& stop);

	void DesignerPartDeleteState(quint16 partId, quint8 state);

	void DesignerPartDeleteStop(quint16 partId, const QString& stop);

	void DesignerPartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane);

	void DesignerPartFillQuad(const AuScSelection& sel, const AuPointArray& boundary);

	void DesignerPartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
        const AuPoint& v4);

	void DesignerPartFillTriangle(const AuScSelection& sel, const AuPointArray& boundary);

	void DesignerPartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3);

	void DesignerPartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName);

	void DesignerPartReplaceFaces(quint16 partId, const AuScSelection& sel, const AuScFaceList& faces);

	void DesignerPartsEffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect);

	void DesignerPartsEffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	void DesignerPartsEffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
			const AuMatrix4x4& gridPlane);

	void DesignerPartsEffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	void DesignerPartsEffectMove(AuScSelection sel, QPointF vec, quint8 undoSeq);

	void DesignerPartsEffectRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq);

	void DesignerPartsEffectStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq);

	void DesignerPartsEffectUnwrap(AuScSelection sel, SURFACE_EFFECT effect, double texSize, AuScFaceList::UNWRAP_METHOD unwrap,
			float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void DesignerPartSetOpacity(quint16 partId, float opacity);

	void DesignerPartSetState(quint16 partId, quint8 partState);

	void DesignerPartSetStop(quint16 partId, const QString& stop);

	void DesignerPartSetStopName(quint16 partId, const QString& name);

	void DesignerPartsMoveDoorSwitches(const AuScSelection& partIds, const AuPoint& switch1, const AuPoint& switch2);

	void DesignerPartsSetDoorProperties(const AuScSelection& partIds, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
			Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle);

	void DesignerPartsSetFaceOpacity(const AuScSelection& sel, float opacity);

	void DesignerPartsSetFaceSelfIllumination(const AuScSelection& sel, float selfIllum);

	void DesignerPartsSetGroup(const AuScIdSet& parts, const QString& groupName);

	void DesignerPartsSetHidden(const AuScIdSet& parts, bool hide);

	void DesignerPartsSetLightBulbProperties(const AuScSelection& partIds, const QColor& color, float kL, float kQ,
			float cosOuterCone, float cosInnerCone);

	void DesignerPartsSetLightLensProperties(const AuScSelection& partIds, const QColor& color);

	void DesignerPartsSetLocked(const AuScIdSet& parts, bool lock);

	void DesignerPartsSetOmitted(const AuScIdSet& parts, bool omit);

	void DesignerPartsSetRoomProperties(const AuScSelection& partIds, bool isSunlit, INTERIOR_LIGHT initLight);

	void DesignerPartsSetSeal(const AuScIdSet& parts, bool seal);

	void DesignerPartsSetSiteProperties(const AuScSelection& partIds, quint8 grade, quint8 grass, quint8 pavement, quint8 rocks,
			quint8 shrubs, quint8 terBlend, quint8 trees);

	void DesignerPartsSetVoidProperties(const AuScSelection& partIds, GRAVITY gravityType, const AuPoint& gravityUp,
			HOLD_ACCESS holdAccess, quint8 personnelAccess);

	void DesignerPartsTextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex);

	void DesignerPartsTextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			double texSize, const AuMatrix4x4& gridPlane);

	void DesignerPartsTextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
			const AuMatrix4x4& gridPlane);

	void DesignerPartsTextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	void DesignerPartsTextureMove(AuScSelection sel, QPointF vec, quint8 undoSeq);

	void DesignerPartsTextureRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq);

	void DesignerPartsTextureStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq);

	void DesignerPartsTextureUnwrap(AuScSelection sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void DesignerPartsTransporterPadLocs(const AuScSelection& partIds, const AuPointArray& padLocs);

	void DesignerPartsWeaponEmitLoc(const AuScSelection& partIds, const AuPoint& emitLoc);

	void DesignerPartUpdate(quint16 partId, const AuScPart& part);

	void DesignerPolishFaces(const AuScSelection& sel, float shine, float reflectivity);

	void DesignerPreviewPeople(const AuSpecies& species);

	void DesignerPunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd);

	void DesignerRedo();

	void DesignerReverseFaces(const AuScSelection& sel);

	void DesignerSetGravity(bool gravOn);

	void DesignerSetPreview(quint8 previewType, const AuDesign* design, quint32 genSeed);

	void DesignerSetProperties(const AuDesign& design);

	void DesignerSetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count]);

	void DesignerSmooth(const AuScSelection& sel);

	void DesignerSplitFaces(const AuScSelection& sel);

	void DesignerTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis);

	void DesignerTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
			const AuPoint& gunEmitVec, const AuPoint& gunSightLoc);

	void DesignerTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc, const AuPoint& gunnerFacing,
			const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane, COMMODITYID module);

	void DesignerUndo();

	void DesignerViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos);

	void DesignerViewPositionLoad(const AuId& avatarId, quint8 number);

	void DesignerViewPositionOrigin(const AuId& avatarId);

	void DesignerViewPositionSave(const AuId& avatarId, quint8 number);

	void DesignExit();

	bool DesignIsGravityOn();

	bool DesignIsRenderShaded();

	int DesignMode();

	QString DesignName();

	// Returns design studio mode.
	quint8 DesignOpen(quint32 designId, int designMode);

	void DesignSetPassword(const QString& pass);

	// Returns design studio mode.
	quint8 DesignStudioEnter(quint16 panelIndex, const QString& password);

	void DomainAdd(const AuId& systemId, const QString& systemName);

	void DomainAddCurrent();

	void DomainRemove(const AuId& systemId, const QString& systemName);

	void DropGearItem(const AuGearPos& pos);

	void EmpireChangeBuildingPref(quint32 empireId, int industry, quint32 designId);

	void EmpireChangeSpacecraftPref(quint32 empireId, int scrole, quint32 designId);

	void EmpireGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos);

	void EmpireRecordPatents(const AuKnownPatents& patents);

	void GameQuit();

	bool GameSelected(quint8 gameType);

	void GameStart(quint8 gameType, const QByteArray& gameSettings);

	void GameTurn(quint8 gameType, const QByteArray& gameTurn);

	void GearDeleteStoredData(const AuGearPos& srcPos, const QSet<COMMODITYID>& patentCIDs, const QSet<quint32>& scDesigns,
			const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers);

	void GeoLandscaper(const AuPoint& startLoc, const AuPoint& endLoc, float roadWidth, bool hideRocks, int hideGrass,
			bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement);

	void GeoRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float roadWidth, quint8 geoType);

	void GetFromCargo(const AuItem& item);

	void GovAssignOffice(const AuId& toWho, const QString& toName, const AuOffice& office);

	void GovDossierAddNote(quint32 dosEmpire, const QString& docNote);

	void GovDossierDispatchCourier(quint32 dosEmpire, const QString& message);

	void GovDossierDocState(quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex, quint8 docState);

	void GovKick(const AuId& personId, const QString& personName, quint32 idOldEmpire);

	void GovLeaveEmpire(const AuId& personId, const QString& personName, quint32 idOldEmpire);

	void GovQuitClaim();

	void GovResignOffice();

	// Revokes all offices held by 'fromWho' and leaves them as a Citizen.
	void GovRevokeOffice(const AuId& fromWho, const QString& fromName);

	void GovSetAnnouncements(const QString& announcements);

	void GovSetBoss(const AuId& governorId, const AuId& bossId);

	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(int capitalType);

	void GovSetCommerce(QByteArray commerce);

	void GovSetContrailColor(quint8 hue, quint8 sat);

	void GovSetDefaultPoliticalStance(quint8 stance);

	void GovSetEmpireFlag(const QImage& imflag);

	void GovSetJoinPassword(const QByteArray& passCrypt);

	void GovSetPilotPolicy(quint8 whichPolicy, quint8 policyValue);

	void GovSetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent);

	void GovSetShieldPolicy(quint8 whichPolicy, quint8 policyValue);

	// tax values are 0-200 representing percentages from 0-100 in .5 percent incs.
	void GovSetTax(quint8 income, quint8 sales, quint8 subsidy);

	// 'theme' is a AuScFace::COLOR_THEME value.
	void GovSetThemeColor(quint8 theme, const QColor& color);

	void GovSetWarCriteria(const AuWarCriteria& crit);

	void GovtAcctDepositFunds(double amount, const AuId& opBd);

	void GovtAcctRequestBalance(const AuId& opBd);

	void GovtAcctWithdrawFunds(double amount, const AuId& opBd);

	void HailStore();

	void HandcraftGo(int mpid);

	bool HangarLaunch(bool takeCaptBerth);

	bool HangarMothball(const AuId& shipId);

	void HangarQuery(QString& shipDesc, quint32& shipDesign, quint32& shipsInHangar);

	void InscribeGearItem(const AuGearPos& pos, const QString& inscription);

	void JobAccepted(const AuJobOffer& jobOffer);

	void KickBerthOccupant(const AuId& kickId);

	void KickCryoBerthOccupants(const AuSpecies& species, int count);

	void KickStowaways();

	void MailCityLast(const AuId& cityId);

	void MissionAppend(const AuMission& mission);

	void MissionAppendOrder(const AuMissionOrder& order);

	void MissionBurnCD(const AuGearPos& mediaPos, const QString& missionName);

	void MissionClear();

	void MissionDeleteCurrentOrder();

	void MissionDeleteOrder(quint8 orderId);

	void MissionInsertOrder(quint8 beforeId, const AuMissionOrder& order);

	void MissionNew(const AuMission& mission);

	void MissionOrderComplete();

	void MissionReplaceOrder(quint8 orderId, const AuMissionOrder& order);

	void MissionSetActive(bool isActive);

	void MissionSetType(quint8 missionType);

	void MoveGearItemToItem(const AuGearPos& fromPos, const AuGearPos& toPos);

	void MoveGearItemToSlot(const AuGearPos& fromPos, quint8 toSlotType);

	void MoveItemRight(const AuId& itemId, double amount, bool relativeToItem);
	void MoveItemLeft(const AuId& itemId, double amount, bool relativeToItem);
	void MoveItemForward(const AuId& itemId, double amount, bool relativeToItem);
	void MoveItemBackward(const AuId& itemId, double amount, bool relativeToItem);
	void MoveItemUp(const AuId& itemId, double amount, bool relativeToItem);
	void MoveItemDown(const AuId& itemId, double amount, bool relativeToItem);

	void NameTheBaby(const QString& babyName);

	// 'loaderId' is the id of the atom providing the ordnance.
	void OrdnanceList(AuId& loaderId, AuItemHash& ordnance);

	// 'rack' is a DaVehicle::WEAPON_RACK.
	// 'ord' is the ordnance wanted to load.
	void OrdnanceLoad(int rack, const AuItem& ord);

	void PayInsurer(double amount);

	void PostBounty(const AuId& victimId, const QString& victimName, const QString& reason, double reward);

	void PutIntoCargo(const AuItem& item);

	// Key is sector range so they are sent closest to farthest.
	void QueryJobs(QMultiMap<int, AuJobOffer>& jobsToStart);

	void QueryJobs(QMultiMap<int, AuJobOffer>& jobsToStart, const QList<AuJob::TYPE>& filterTypes, int maxSectorsAway);

	void ReconcileRoster();

	void RedeemGovtDebt(const AuId& opBd);

	void ReleaseBerth();

	// 'personId' is the person who has released their berth.
	// 'shipId' is the ship that the person is berthing with instead of their old one. This could be 0.
	static void ReleaseBerthDown(const AuId& personId, const AuId& shipId);

	void ReleaseSpacecraftTitle();

	// 'renameWhat' 0=city, 1=planet, 2=solar system, 3=sector, 4=young avatar, 5=building.
	// Returns true on success.
	bool Rename(quint8 renameWhat, const QString& nuName, const AuId& capitolOpId);

	void RepairShopRecover();

	bool ReportCityHistory(const AuId& cityId, bool& haveReport, AuCityReportHistory& report, bool& personHasAgentPower,
			quint32& personEmpire);

	bool ReportCityLast(const AuId& cityId, bool& haveReport, AuCityReport& report, bool& havePrevReport, AuCityReport& prevReport,
			bool& personHasAgentPower, quint32& personEmpire);

	void ResetItemTransform(const AuId& itemId, const AuMatrix4x4& t, const QString& room);

	// 'role' is 0=none, 1=crew, 2=troop, 3=passenger
	void ReviveCryoBerthOccupant(const AuSpecies& species, quint8 role);

	void RotateItemXPos(const AuId& itemId, double amount, bool relativeToItem);
	void RotateItemXNeg(const AuId& itemId, double amount, bool relativeToItem);
	void RotateItemYPos(const AuId& itemId, double amount, bool relativeToItem);
	void RotateItemYNeg(const AuId& itemId, double amount, bool relativeToItem);
	void RotateItemZPos(const AuId& itemId, double amount, bool relativeToItem);
	void RotateItemZNeg(const AuId& itemId, double amount, bool relativeToItem);

	void ScDesignExchangePublish(AuScDesignExchangeEntry& exchangeItem);

	void ScDesignExchangePurchase(quint32 designId, const AuGearPos& mediaPos);

	void ScDesignExchangeRate(const AuExchangeRatingRec& rating);

	// Unpublishes a design made by this architect.
    // 'deleting' is true when the design is being unpublished in the process of being deleted.
	void ScDesignExchangeUnpublish(bool isArchitect, quint32 designId, bool deleting);

	// Returns sensor range.
	double SensorContacts(AuSensorBlipList& sensorBlips, AuPoint& sensorEnvLoc);

	void SetDNA(const AuDNA& dna);

	void SetHairStyle(AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave);

	void StorySendCurrentBlock(quint32 storyInstance);

	void StorySendHint(quint32 storyInstance);

	void Selfie(const QImage& img);

	void SendFunds(const AuId& toId, const QString& toName, double amount);

	void SetAcceptCrew(bool accept);

	void SetAcceptPassengers(bool accept);

	void SetAcceptTroops(bool accept);

	void SetAllowPassengerSeaChests(bool allow);

	void SetAutoEat(bool autoEat);

	void SetFlowerPot(const AuGearPos& pos, quint8 plantSize);

	void SetMessage(const AuGearPos& pos, const QString& message);

	void SetPictureFrame(const AuGearPos& pos, const QString& url, bool hideURL, bool rotate);

	void SetSpeciesName(const AuSpecies& species, const QString& speciesName, bool isWildAnimal);

	void SetUnitAmmo(const AuId& unitId, COMMODITYID ammoCID);

	void SetUnitAmmoCount(const AuId& unitId, quint8 count);

	void SetUnitArmor(const AuId& unitId, const AuItem& item);

	void SetUnitClothing(const AuId& unitId, const AuItem& item);

	void SetUnitHelmet(const AuId& unitId, const AuItem& item);

	void SetUnitWeapon(const AuId& unitId, const AuItem& item);

	bool ShipmentCreate(const AuShipment& shipment);

	bool ShipmentCreate(const AuShipment& shipment, const AuId& opBd);

	bool ShipmentDelete(const AuShipment& shipment);

	bool ShipmentDelete(const AuShipment& shipment, const AuId& opBd);

	// Returns:
	// 0 = ok
	// 1 = person not found
	// 2 = vessel not found
	// 3 = person not in chain of command of vessel
	int Shipments(AuShipmentList& shipments, bool& isStation);

	void StoryChoicePicked(quint32 storyInstance, qint8 value, const AuId& offeredBy);

	void StoryClearScript(const AuId& forWho);

	void StoryScriptSubmitted(const AuId& forWho, const AuScript& script);

	void TakeItem(const AuId& itemId);

	void ToggleShipOwnership();

	bool TradeAgreementAccept(const AuTradeAgreement& ta, const AuId& opBd);

	bool TradeAgreementCounter(const AuTradeAgreement& proposal, const AuTradeAgreement& counter, const AuId& opBd);

	bool TradeAgreementCreate(const AuTradeAgreement& ta, const AuId& opBd);

	bool TradeAgreementDelete(const AuTradeAgreement& ta, const AuId& opBd);

	void TransferCitizen(DaCitizen* citizen, DaVessel* fromShip, DaVessel* toShip, AuRosterEntry::TYPE berthType);

	void TransferCitizens(DaVessel* fromShip, DaVessel* toShip, const AuIdList& citizenIds, const AuIdList& swapIds);

	void TransferListCitizens(const AuId& shipId, QByteArray& bacit);

	void TransferSendCargo(const AuId& otherShipId, const AuItem& item);

	void TransferSendCargo(const AuId& otherShipId, const AuItemHash& items);

	void TransferSendCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds);

	void TransferSendItemsLoose(const AuId& otherShipId);

	void TransferSendItemsStatic(const AuId& otherShipId);

	void TransferSendMoney(const AuId& otherShipId, double amount);

	bool TransferSetup(const AuId& otherShipId, DaVessel** ship, DaVessel** otherShip);

	void TransferSwapCitizens(DaCitizen* citizen, DaCitizen* swap, DaVessel* fromShip, DaVessel* toShip,
			AuRosterEntry::TYPE berthType);

	void TransferTakeCargo(const AuId& otherShipId, const AuItem& item);

	void TransferTakeCargo(const AuId& otherShipId, const AuItemHash& items);

	void TransferTakeCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds);

	void TransferTakeItemsLoose(const AuId& otherShipId);

	void TransferTakeItemsStatic(const AuId& otherShipId);

	void TransferTakeMoney(const AuId& otherShipId, double amount);

	void TransporterCoord(const AuPoint& coord);

	void TransporterLongLat(const AuGlobeLLF& ll);

	void TransporterPlaces(QList<AuTransporterTarget>& ttplaces);

	void TransporterSpacecraft(QList<AuTransporterShip>& ttships);

	void TransporterTransporters(QList<AuTransporterSystem>& tttrans);

	void UseGearItem(const AuGearPos& pos);

	void WithdrawShipFunds(double amount, const AuId& shipId);

signals:
	void AcceptTrade(AuTradeAgreement ta);

	void AdminGoTo(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail);

	void AdminGoToGalaxyLoc(AuId personOrRide, AuId origSystem, AuId destGalaxyId, AuPoint destGalaxyLoc);

	void BdDesignExchangePublished(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublished(quint32 designId);

	void BlueprintRatedBd(AuExchangeRatingRec rating);

	void BlueprintRatedSc(AuExchangeRatingRec rating);

	void BountyPosted(AuId posterId, QString posterName, AuId victim, QString reason, double reward, int why);

	// 'cityType' is a CITYTYPE enum value.
	// 'galaxy' is a GALAXY enum value.
	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	// 'industry' is an INDUSTRYID
	void EmpireChgBdPref(quint32 empireId, int industry, quint32 designId);

	void EmpireChgFlag(quint32 empireId, QImage flag);

	// 'scrole' is an AuScPrefs::SCROLE
	void EmpireChgScPref(quint32 empireId, int scrole, quint32 designId);

	void EstablishNewEmpire(AuId personId, QString personName, quint8 personGender, quint32 idOldEmpire, QString name,
			QImage imFlag);

	void GovBossChanged(quint32 empire, AuId governorId, AuId bossId);

	void GovDossierAddDocument(quint32 empire, quint32 dosEmpire, QByteArray packedDoc);

	// 'docTime' and 'docType' together are used to identify the exact document.
	void GovDossierChangeState(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
			quint8 docState);

	void GovSetAnnouncements(quint32 empire, QString announcements);

	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(quint32 idEmpire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);

	void GovSetCommerce(quint32 idEmpire, QByteArray commerce);

	void GovSetContrailColor(quint32 idEmpire, quint8 hue, quint8 sat);

	void GovSetDefaultPoliticalStance(quint32 empire, quint8 stance);

	void GovSetJoinPassword(quint32 empire, const QByteArray& passCrypt);

	void GovSetPoliticalStance(quint32 idToEmpire, quint32 idFromEmpire, quint8 stance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

	void GovSetTax(quint32 empire, quint8 income, quint8 sales, quint8 subsidy);

	void GovSetThemeColor(quint32 empire, quint8 theme, QColor color);

	void GovSetWarCriteria(quint32 empire, AuWarCriteria crit);

	void LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire);

	void NameSpecies(AuSpecies species, QString speciesName, AuId discoverer);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OfficeRevoked(AuId person, quint32 empire);

	void ProposeTrade(AuTradeAgreement ta);

	void ReleaseBerth(AuId personId, AuId shipId);

	void RenameAvatar(AuId avatarId, QString nuName);

	void RenameCity(AuId cityId, QString nuName);

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSector(AuId sectorId, QString nuName);

	void RenameSystem(AuId systemId, QString nuName);

	void ScDesignExchangePublished(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublished(quint32 designId);

	// 'fromGalaxy' is a GALAXY value.
	void SendFunds(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);
};

#endif // AuAtomInterface_H
