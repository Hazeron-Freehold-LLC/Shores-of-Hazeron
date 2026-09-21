//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auapp.h,v 1.92 2022/05/05 16:01:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuApp_H
#define AuApp_H

#include "auagentpower.h"
#include "aubddesign.h"
#include "audossier.h"
#include "auclientlib.h"
#include "auexchangerating.h"
#include "auitdesign.h"
#include "aukeyfilter.h"
#include "auplayeronlinestatus.h"
#include "auscdesign.h"
#include "austory.h"
#include "autransportertarget.h"
#include "dacitizen.h"
#include "deconstructionprocess.h"
#include "dzspecies.h"
#include <QApplication>
#include <QUdpSocket>

class AuClientInterface;
class AuCommPacket;
class AuDesign;
class AuEmojiList;
class AuEther;
class AuFrame;
class AuModel;
class AuOffice;
class AuPlayerOnlineStatusVector;
class AuScPart;
class AuScParts;
class AuSound;
class AuSpecies;
class AuWarCriteria;
class DgData;
class DiControlInputArray;
class DtClient;
class DtClientChat;
class QKeyEvent;

#define CLIENT_SOUNDS 4					// Number of simultaneous client sounds that an play, i.e. not sounds from atoms or effects.

class AUCLIENT_DLL AuApp : public QApplication
{
	Q_OBJECT;
public:
	enum RUNSTATE
	{
		NotLoggedIn,					// Need to query for login name and password. Initital state.
		LoggedIn,						// Logged in and player is ready to run in normal scene mode.
		LoggedInInit,					// Logged in and player needs first-time init.
		LoggedInWait,					// Logged in and player has chosen to either create a new empire, or join a new empire
		InitWait,						// Waiting for reply to first time init user input.
		Solitaire,						// Running in local single-player mode.
		Designer,						// Running in local single-architect design mode.
	};

	enum DEFERTYPE
	{
		DeferNone,						// Not currently deferring.
		DeferScene,						// Defer to scene server (could be master, manager or worker).
		DeferEstFirstEmpire,			// Defer to create first character in empire just created.
	};

protected:
	DgData*								m_Data;
	AuEther*							m_Etherium;				// The global etherium.
	AuClientInterface*					m_I;
	AuGLModelCache*						m_ModelCache;

private:
	AuElapsedTimer						m_AvatarDeactivateTime;	// Time of last query for inactive avatars and deactivation.
	RUNSTATE							m_eState;				// application run state
	AuFrame*							m_Frame;				// main frame widget
	AuKeyFilter							m_KeyFilter;
	AuTime								m_ChatDisconnectTime;	// So we get chat messages missed while disconnected.
	AuElapsedTimer						m_JobOfferLast;
	bool								m_SoloAvatarDeleted;

	// Defer info.
	DEFERTYPE							m_DeferType;
	AuId								m_DeferAtom;			// Atom id in the call to EstDeferred
	bool								m_SceneConnectedOnce;	// True after we have connected to a scene worker server.

	// Comm, data transmit stream and datagram buffers.
	bool								m_InitSyncDone;			// true if the initial sync is complete
	bool								m_SyncDone;				// Set when sync complete rx'd. Cleared when controls tx'd.
	QMutex								m_muChatTcpBuffer;
	QMutex								m_muSceneTcpBuffer;
	QMutex								m_muSceneUdpBuffer;
	QByteArray							m_ChatTcpBuffer;		// Scene outgoing tcp chat stream buffer.
	QByteArray							m_SceneTcpBuffer;		// Scene outgoing tcp stream buffer.
	QUdpSocket 							m_SceneUdpSocket;		// Scene outgoing udp datagram socket.
	quint16								m_UdpSequence;			// Helps the server discard bad packets.
	bool								m_wasConnected;			// true if we are ever connected to the server

	// EstFirstEmpire data that must be resent when deferred.
	QByteArray							m_NewDNA;
	GALAXY								m_NewGalaxy;
	quint8								m_NewProximity;

	// Threads
	DtClient*							m_tSceneChannel;		// Client communication channel with scene server.
	DtClientChat*						m_tChatChannel;			// Client communication channel with scene server.
	AuModel*							m_Model;

	// OpenAL
	AuSound*							m_AL;					// OpenAL sound functions.
	unsigned int						m_Audio[CLIENT_SOUNDS];	// audio sources for simultaneous client sounds
	unsigned int						m_AudioSource;			// audio source for queued sounds
	int									m_Sound[CLIENT_SOUNDS];	// current SOUND_EFFECT for simultaneous client sounds
	QVector<AuCitizenSound>				m_SoundQueue;			// sounds queued for playing

	// Empire requests
	QMutex								m_EmpiresRequestedLock;
	QList<quint32>						m_EmpiresRequested;		// Empires for which we have requested an empire record.

private:
	AuDNA								m_HumanDNA;				// Default human for primeval world and solo designer.
	GENDER								m_HumanGender;			// Default human for primeval world and solo designer.
	HANDEDNESS							m_HumanHand;			// Default human for primeval world and solo designer.
	QString								m_HumanName;			// Default human for primeval world and solo designer.

public:
	AuApp(int& argc, char** argv);
	virtual ~AuApp();

public:
	void AcceptJob(const AuJobOffer& jobOffer);

	void AgentAerialEncounter();

	void AgentClearIndigenousDNA(qint8 floraZone);

	void AgentAnimalEncounter();

	void AgentEncounterAsteroid(quint32 minutesFromNow);

	void AgentEncounterPirateShip();

	void AgentGoToDestination(const AuId& goToId, bool rideToo, bool limboRescue);

	void AgentGoToGalaxy(GALAXY galaxy, const AuPoint& coord, bool rideToo);

	void AgentGoToLongLat(const AuGlobeLLF& ll, bool rideToo);

	void AgentIndigenousEncounter();

	void AgentIndigenousEncounterForceMove();

	void AgentIndigenousEncounterForceMoveResetReputation();

	void AgentKillDomesticAnimals();

	void AgentKillWildAnimals();

	void AgentMakeItem(COMMODITYID cid, quint8 quality, quint32 count, quint8 galaxy, const AuPoint& galaxyCoord,
			AuAgentMakeItem::MAKEWHAT makeWhat);

	void AgentMonsterEncounter();

	void AgentRegenPlantDNA(quint8 plantType, qint8 floraZone);

	void AgentSetBits(quint8 bits);

	void AgentSetCitizenDNA();

	void AgentSetIndigenousDNA(qint8 floraZone);

	void AgentSummonPerson(const AuId& summonId, bool rideToo);

	void ArenaExit();

	void ArenaRecall();

	void ArenaStatus();

	void AssignOffice(const AuId& toWho, const QString& toName, const AuOffice& office);

	// Returns the id of the person atom for whom the scene is generated.
	const AuId& AvatarId() const;

	void AvatarRename(const QString& nuName);

	void BankBorrow(double amount, const AuId& opBd);

	void BankGADeposit(double amount, const AuId& opBd);

	void BankGARequestBalance(const AuId& opBd);

	void BankGAWithdraw(double amount, const AuId& opBd);

	void BankPay(double amount, const AuId& opBd);

	void BdExchangeBuyDesign(quint32 designId, const AuGearPos& mediaPos);

	void BdExchangeDesignsNeeded(const QList<quint32>& designsNeeded);

	void BdExchangePublish(AuBdDesignExchangeEntry& libEntry);

	void BdExchangeRate(quint32 designId, quint8 rating, const QString& comment);

	void BdExchangeSaveToGear(const AuBdDesignExchangeEntry& libEntry, const AuGearPos& mediaPos);

	void BdExchangeUnpublish(quint32 designId);

	void BdExchangeViewRatings(quint32 designId);

	void BdLibraryRequestDesigns();

	void BrokerAcceptTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd);

	void BrokerCounterTradeAgreement(const AuTradeAgreement& ta, const AuTradeAgreement& tac, const AuId& opBd);

	void BrokerCreateShipment(const AuShipment& shipment, const AuId& opBd);

	void BrokerCreateTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd);

	void BrokerDeleteShipment(const AuShipment& shipment, const AuId& opBd);

	void BrokerDeleteTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd);

	void BrokerRequestShipments(bool sayResult, const AuId& opBd);

	void BuildingDemolish(const AuId& bldId);

	// 'name' may be blank.
	void BuildingRename(const QString& name, const AuId& opBd);

	// Sets the storage commodity for warehouses and storage tanks.
	void BuildingStoreCommodity(COMMODITYID storeCID, const AuId& opBd);

	void BuildingTakeItem(const AuItem& item, const AuId& opBd);

	void BuildingTrashItem(const AuItem& item, const AuId& opBd);

	void CallTurboLift(const AuId& shipId, quint16 liftId, const QString& stop);

	void ChangeMyDNA(const AuDNABase& dna);

	// 'cityId' may be the world id to change geo roads.
	void ChangeRoadConstruction(const AuId& cityId, const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid);

	void CityRename(const QString& nuName);

	// When operated from a capitol building, 'opBd' is the atom id of the DaBd that is being operated.
	// 'opBd' is null when the scene atom is at the DaBd being operated.
	void ClaimReward(const AuId& opBd);

	void ClearBd(const AuId& bdId);

	void ClearGeo(const AuGlobeLLF& geoLL);

	// Deletes all current saved game files. Does not delete the current game in progress.
	void ClearSavedGames();

	// Clears the Etherium and deletes the current solo game and its cache files. Does not touch saved game files.
	void ClearSolitaire();

	void CommAcceptTeleport(DaPerson* fromPerson, const AuCommPacket& msg, AuEther& ether);

	void CommCloseChannel(const QString& channelName);

	void CommChannelOpened(const QString& channelName);

	void CommOpenChannelTo(const QString& channelName, const AuId& toId);

	void CreateNewEmpire(const QString& name, const QImage& flag);

	void DeletePerson(const AuId& personId);

	// Deletes the solitaire universe when the last solitaire avatar is deleted.
	void DeleteSolitaireAvatar(const AuId& avatarId);

	void DesignerAddPartObjects(quint16 partId, const AuScParts& partObjects);

	// Adds 'parts' to the design.
	void DesignerAddParts(const AuScParts& parts);

	void DesignerAssociateParts(const AuScSelection& partIds);

	void DesignerCheckDoors(const AuScSelection& partIds);

	void DesignerCopyPartObjects(const AuScSelection& partIds, const AuMatrix4x4& t);

	// Adds 'partObjects' to part 'partId'.
	void DesignerCopyParts(const AuScSelection& partIds, const AuMatrix4x4& t);

	void DesignerCopyPartsSymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane);

	void DesignerCutFaces(const AuScSelection& sel, const AuPointArray& cutLine);

	void DesignerDeletePartObjects(const AuScSelection& partIds);

	void DesignerDeleteParts(const AuScSelection& partIds);

	void DesignerDissociateParts(const AuScSelection& partIds);

	void DesignerDivideFaces(const AuScSelection& sel, quint8 divisions);

	void DesignerEffectIntensityVertices(const AuScSelection& sel, float intensity);

	void DesignerFlatten(const AuScSelection& sel);

	void DesignerFractureFaces(const AuScSelection& sel);

	void DesignerFuseFaces(const AuScSelection& sel);

	// Opens a design from media at a gear position 'pos'.
	void DesignerGearOpen(quint32 designId, const AuGearPos& pos);

	void DesignerGlowVertices(const AuScSelection& sel, const QColor& glow);

	// Teleports the avatar to the selected part(s).
	void DesignerGoToSelection(const AuScSelection& sel);

	void DesignerGridPositionSave(quint8 number, const AuMatrix4x4& gridM);

	void DesignerJigCut(const AuScSelection& sel);

	// Joins 'sel' parts to part 'partId'.
	void DesignerJoinParts(quint16 partId, const AuScSelection& sel);

	void DesignerLibraryDeleteBd(quint32 designId);

	void DesignerLibraryDeleteIt(quint32 designId);

	void DesignerLibraryDeleteSc(quint32 designId);

	bool DesignerLibraryFinalizeAssembly(AuDesign* design);

	// 'designId' is only provided by agents when a design must be assigned a specific design id.
	// An existing design with the same id is replaced. 'designId' is ignored in solo play.
	bool DesignerLibraryFinalizeDesign(AuDesign* design, quint32 designId);

	// 'design.Id()' can be 0 to save as a new design, otherwise it overwrites an existing design.
	// 'design.Name() is the name that appears in the library, akint to a file name. It does not have to be unique.
	bool DesignerLibrarySave(AuDesign* design);

	bool DesignerLoadedFromFile(AuDesign& design);

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

	// 'theme' is a AuScFace::COLOR_THEME
	void DesignerPaintFaces(const AuScSelection& sel, quint8 theme, quint8 opacity);

	void DesignerPartAdd(const AuScPart& part);

	void DesignerPartAddFaces(quint16 partId, const AuScFaceList& faces);

	void DesignerPartAddState(quint16 partId, quint8 state);

	void DesignerPartAddStop(quint16 partId, const QString& stop);

	void DesignerPartDeleteState(quint16 partId, quint8 state);

	void DesignerPartDeleteStop(quint16 partId, const QString& stop);

	void DesignerPartFillFace(const AuScSelection& partIds, const AuPointArray& boundary, const AuMatrix4x4& gridPlane);

	void DesignerPartFillQuad(const AuScSelection& partIds, const AuPointArray& boundary);

	void DesignerPartFillTriangle(const AuScSelection& partIds, const AuPointArray& boundary);

	void DesignerPartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName);

	// Selected faces are removed from 'partId'.
	// 'faces' are added to 'partId'.
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

	// 'alignAxis' is 0=x, 1=y, or 2=z but only matters is 'gridAlign' is true.
	void DesignerPartsEffectUnwrap(AuScSelection sel, SURFACE_EFFECT effect, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

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

	// 'alignAxis' is 0=x, 1=y, or 2=z but only matters is 'gridAlign' is true.
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

	// Changes the DNA of the designer avatar. Only for single user mode.
	void DesignerSetDNA(const AuDNABase* dna);

	void DesignerSetGravity(bool gravOn);

	bool DesignerSetPreview(quint8 previewType, quint32 genSeed);

	void DesignerSetProperties(const AuDesign& design);

	void DesignerSetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count]);

	void DesignerSmooth(const AuScSelection& sel);

	void DesignerSplitFaces(const AuScSelection& sel);

	void DesignerTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis);

	void DesignerTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis, const AuPoint& gunEmitLoc,
			const AuPoint& gunEmitVec, const AuPoint& gunSightLoc);

	// 'gunnerPos' is a AuScTurret::GUNNER_POSITION
	void DesignerTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc, const AuPoint& gunnerFacing,
			const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane, COMMODITYID module);

	void DesignerUndo();

	void DesignerViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos);

	void DesignerViewPositionLoad(const AuId& avatarId, quint8 number);

	void DesignerViewPositionOrigin(const AuId& avatarId);

	void DesignerViewPositionSave(const AuId& avatarId, quint8 number);

	void DesignExit();

	bool DesignIsGravityOn() const;

	bool DesignIsRenderShaded() const;

	// Return value is a DaDesignBase::DESIGNMODE.
	// SpaceShip and Building are returned when the design environment is valid.
	// Returns DaDesignBase::InvalidMode if not in any design studio.
	int DesignMode() const;

	QString DesignName() const;

	// 'designMode' is a DaDesignBase::DESIGNMODE
	void DesignOpen(quint32 designId, int designMode);

	void EmpireChangeEmperor(quint32 empireId, const AuId& avatarId, const QString& avatarName);

	void EmpireChangeFlag(const QImage& imFlag);

	void EmpireChangeFlag(quint32 empireId, const QImage& imFlag);

	void EmpireChangeName(quint32 empireId, const QString& name);

	void FarmSetLivestock(const AuSpecies& species, const AuId& opBd);

	inline AuFrame* Frame()
	{
		return m_Frame;
	}

	void GameSelect(quint8 gameType);

	// The local player has started a game at a game table.
	void GameStart(quint8 gameType, QByteArray gameSettings);

	// The local player has finished their turn at a game table.
	void GameTurn(quint8 gameType, QByteArray gameSettings);

	// The local player has quit the current game.
	void GameQuit();

	void GearDeleteContents(const AuGearPos& gearPos, const QSet<COMMODITYID>& patentCIDs, const QSet<quint32>& scDesigns,
			const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers);

	void GetFromCargo(const AuItem& item);

	void GovQuitClaim();

	void GovSetBoss(const AuId& governorId, const AuId& newBossId);

	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(int capitalType);

	void GovSetContrailColor(quint8 hue, quint8 sat);

	void GovSetEmpireCapital();

	void GovSetGalaxyCapital();

	void GovSetJoinPassword(const QString& password);

	void GovSetSectorCapital();

	// 'theme' is a AuScFace::COLOR_THEME
	void GovSetThemeColor(quint8 theme, const QColor& color);

	void HandleSignal(int value);   // Called to handle a signal caught in main.cpp.

	void HangarLaunch(bool takeCommand);

	void HangarMothball(const AuId& shipId);

	void HangarQuery();

	virtual QString Icon128() const = 0;

	virtual QString Icon32() const = 0;

	// Configure model for running single-user designer mode.
	void InitDesignerMode();

	// Creates the AuSound object. This should be done after global settings exist so it can't be done in my constructor.
	void InitializeSound();

	// 'sound' is a SOUND_EFFECT
	bool IsClientSoundPlaying(int sound) const;

	virtual bool IsHazeronCommander() const = 0;

	virtual bool IsHazeronStarship() const = 0;

	// Transmit buffers are used by the client to send data to the server. They are not
	// used by the server, which allocates buffers in each DtServer thread.
	inline bool IsInitSyncComplete() const
	{
		return m_InitSyncDone;
	}

	bool IsSceneConnected() const;

	inline bool IsSingleUserDesignerMode() const
	{
		return RunState() == Designer;
	}

	inline bool IsSingleUserMode() const
	{
		return RunState() == Solitaire || IsSingleUserDesignerMode();
	}

	void ItemMoveBackward(const AuId& itemId, double amount, bool relativeToItem);

	void ItemMoveDown(const AuId& itemId, double amount, bool relativeToItem);

	void ItemMoveForward(const AuId& itemId, double amount, bool relativeToItem);

	void ItemMoveLeft(const AuId& itemId, double amount, bool relativeToItem);

	void ItemMoveRight(const AuId& itemId, double amount, bool relativeToItem);

	void ItemMoveUp(const AuId& itemId, double amount, bool relativeToItem);

	void ItemReset(const AuId& itemId, const AuMatrix4x4& t, const QString& roomName);

	void ItemRotateXNeg(const AuId& itemId, double amount, bool relativeToItem);

	void ItemRotateXPos(const AuId& itemId, double amount, bool relativeToItem);

	void ItemRotateYNeg(const AuId& itemId, double amount, bool relativeToItem);

	void ItemRotateYPos(const AuId& itemId, double amount, bool relativeToItem);

	void ItemRotateZNeg(const AuId& itemId, double amount, bool relativeToItem);

	void ItemRotateZPos(const AuId& itemId, double amount, bool relativeToItem);

	void ItemTake(const AuId& itemId);

	void ItLibraryRequestDesigns();

	void JoinArena(AuId arenaId, QString password, quint32 shipDesign, quint8 team);

	inline AuKeyFilter& KeyFilter()
	{
		return m_KeyFilter;
	}

	inline const AuKeyFilter& KeyFilter() const
	{
		return m_KeyFilter;
	}

	void KickOrLeaveEmpire(const AuId& personId, const QString& personName, quint32 idOldEmpire);

	void LandAddCurrentSystem();

	void LandAddSystem(const AuId& systemId, const QString& systemName);

	void LandRemoveSystem(const AuId& systemId, const QString& systemName);

	// 'names' is filled with human readable localized names.
	// 'files' is filled with corresonding actual file names.
	void ListSolitaireSaves(QStringList& names, QStringList& files);

	void MissionAppendMission(const AuMission& mission);

	void MissionAppendOrder(const AuMissionOrder& order);

	void MissionBurnCD(const QString& name, const AuGearPos& mediaPos);

	void MissionClear();

	void MissionInsertOrder(const AuMissionOrder& order);

	void MissionInsertOrder(quint8 beforeOrderId, const AuMissionOrder& order);

	void MissionLoad(const AuMission& mission);

	void MissionNew(const AuMission& mission);

	void MissionNew(const AuMissionOrder& order);

	void MissionOrderComplete();

	void MissionOrderDelete(quint8 orderId);

	void MissionOrderReplace(quint8 orderId, const AuMissionOrder& order);

	void MissionRequestSensorContacts();

	void MissionSetActive(bool active);

	void MissionSetCycle(bool cycle);

	inline AuModel* Model()
	{
		return m_Model;
	}

	// Set the name of an unnamed specie.
	void NameSpecies(const AuSpecies& species, const QString& speciesName, bool isWildAnimal);

	void NameTheBaby(const QString& name);

	inline quint16 NextUdpSequence()
	{
		m_muSceneUdpBuffer.lock();
		quint16 v = m_UdpSequence++;
		m_muSceneUdpBuffer.unlock();
		return v;
	}

	void OrdnanceLoad(quint8 rack, const AuItem& item);

	void OrdnanceRequest();

	void PayInsurer(double amountToPay);

	void PirateSetIndustryBuilding(quint32 empireId, INDUSTRYID industry, quint32 blueprint);

	void PirateSetSpacecraft(quint32 empireId, int scrole, quint32 designId);

	void PlanetRename(const QString& nuName);

	// 'sound' is a SOUND_EFFECT
	// 'gain' is further modulated by g_Au.SoundEffectsGain and sound effect specific gain setting.
	void PlayClientSound(int sound, float gain=1.f, float pitch=1.f);

	void PlayQueuedSounds(float gain);

	void PostBounty(const AuId& victimId, const QString& victimName, const QString& reason, double amount);

	void PostSelfie(const QImage& image);

	void ProcessControlInput(const AuId& toAtom, DiControlInputArray& aActiveControls, const AuGearPos& pos, qint8 chSelValue,
			int msElapsed);

	void PutIntoCargo(const AuItem& item);

	void QueryArenas();

	// Insure we have the empire and specie data loaded for all items in a gear, including items in containers.
	bool QueryEmpireAndSpecie(const AuGear& gear);

	// Insure we have the empire and specie data loaded for all items in a cargo inventory, not including items in containers.
	bool QueryEmpireAndSpecie(const AuItemHash& cargo);

	// Insure we have the empire and specie records for this item in the local client data.
	bool QueryEmpireAndSpecie(const AuItem& item);

	void QueryJobs();

	void QueryJobs(quint16 maxSectorsAway, bool typeFilter[AuJob::JobTypes]);

	void QueryPlayersOnline(AuPlayerOnlineStatusVector& players);

	// 'query' is 0=universe, 1=galaxy, 2=sector
	// 'sectorIds' is filled with sector ids when query is sector
	// 'sectorGalaxy' set to galaxy of returned sectors when query is sector
	void QueryStarmap(quint8 query, GALAXY galaxy, const AuId& sectorId, quint16 range, const AuId& personId,
			quint32 personEmpireId, const AuId& personSystemId, AuIdList& sectorIds, GALAXY& sectorGalaxy);

	void ReleaseBerth();

	void RepairShopRecover();

	void RequestBasicGameData();

	// Requests the DgEmpire record for 'empireId', regardless if we already requested it.
	void RequestEmpire(quint32 empireId);

	void RequestEmpiresAll();

	void RequestEmpiresJoinable();

	void RequestPlayersOnline();

	// To resign all offices from a person, call ResignOffice with AuId(0) as the placeId.
	void ResignOffice();

	// Revokes all offices from a person and leaves them as a Citizen.
	void RevokeOffice(const AuId& playerId, const QString& playerName);

	inline RUNSTATE RunState() const
	{
		return m_eState;
	}

	virtual void SceneLogin() = 0;	// Send login packet to scene server.

	void SceneLogout();	// Send logout packet to scene server.

	void SceneSyncComplete();

	// Sends the SyncComplete packet to the server then calls SceneSyncComplete().
	void SceneSyncComplete(const AuTime& serverUTC);

	// Return true if scene sync flag is reset, false if it was not set on entry.
	bool SceneSyncReset();

	void ScExchangeBuyDesign(quint32 designId, const AuGearPos& mediaPos);

	void ScExchangeDesignsNeeded(const QList<quint32>& designsNeeded);

	void ScExchangePublish(AuScDesignExchangeEntry& libEntry);

	void ScExchangeRate(quint32 designId, quint8 rating, const QString& comment);

	void ScExchangeSaveToGear(const AuScDesignExchangeEntry& libEntry, const AuGearPos& mediaPos);

	void ScExchangeUnpublish(quint32 designId);

	void ScExchangeViewRatings(quint32 designId);

	void ScLibraryRequestDesigns();

	void SectorRename(const QString& nuName);

	void SendAddSquelch(const AuId& idSquelch);

	void SendChatMessage(const AuCommPacket& msg);

	void SendDeleteSquelch(AuId &idSquelch);

	// Sends funds from the scene atom's personal account to another atom.
	// The recipient atom can be a person, city, or spacecraft.
	// If the amount exceeds the senders account balance, the entire account balance is transferred.
	// If the amount exceeds a receiving city's bank storage capacity, the excess is lost. Oops! Sorry.
	// A receipt is mailed to the sender and to the receiver even if no money is exchanged.
	// Returns false if toId is null or amount is <= 0.
	bool SendFunds(const QString& toName, const AuId& toId, double amount);

	void SendPosition(quint16 hasWhat, float headTilt, const AuAtomPos& pos, const AuPose pose[DpBody::PS_Count],
			quint16 whichConsole, quint16 stationId, const AuPoint& vec, quint8 seatIndex, const AuAtomPos& parentPos,
			const AuPose parentPose[DpBody::PS_Count], quint16 parentWhichConsole, quint16 parentStationId,
			const AuPoint& parentVec);

	void SetAFK(bool afk);

	void SetAutoEat(int autoEat);

	void SetAvatarId(const AuId& personId);

	void SetDefaultHuman(const QString& name, HANDEDNESS hand, GENDER gender, const AuDNA& dna);

	void SetFrame(AuFrame* frame);

	void SetEmojis(const AuEmojiList& emojis);

	void SetHairStyle(AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave);

	void SetRunState(RUNSTATE state);

	void SetUnitAmmo(const AuId& unitId, COMMODITYID ammoCID);

	void SetUnitAmmoCount(const AuId& unitId, quint8 count);

	void SetUnitArmor(const AuId& unitId, const AuItem& item);

	void SetUnitClothing(const AuId& unitId, const AuItem& item);

	void SetUnitHelmet(const AuId& unitId, const AuItem& item);

	void SetUnitWeapon(const AuId& unitId, const AuItem& item);

	void ShipChristen(const QString& name);

	void ShipClaimTitle();

	void ShipClearRatDNA();

	void ShipCopyTitle(const AuGearPos& mediaPos);

	void ShipCreateShipment(const AuShipment& shipment);

	void ShipDeleteShipment(const AuShipment& shipment);

	void ShipDrainCapacitor(quint8 percentCap);

	void ShipFillCapacitor(quint8 percentCap);

	void ShipLoadCrew();

	void ShipLoadOfficer();

	void ShipLoadPassengers();

	void ShipLoadTroops();

	void ShipPirateEquip();

	void ShipRecover(const AuId& shipId);

	void ShipRefuel(quint8 fuelQ);

	void ShipReleaseTitle();

	void ShipRepair();

	void ShipRequestShipments(bool sayResult);

	void ShipScuttle(const QString& reason);

	void ShipToggleOwnership();

	virtual QString Slogan() const = 0;

	// Processes atom chat messages that have accumulated.
	void SoloAtomChat();

	// Activates and deactivates avatars.
	void SoloDeactivateAvatars();

	inline AuSound* Sound()
	{
		return m_AL;
	}

	// Sends StartNewAvatar request to server.
	// Server eventually responds.
	void StartNewAvatar(const QString& charname, const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand,
			quint8 proximity, GALAXY galaxy, SCENARIO scenario, AuTime asteroidImpact, quint32 joinEmpire,
			const QByteArray& joinPassCrypt, const AuId& joinSectorId);

	// 'avatarIndex' must be 0 to 5.
	void StartNewAvatarSolo(int avatarIndex, const QString& charname, const AuDNABase& dna, AuDNABase::CHROMATID chroma,
			HANDEDNESS hand, quint8 proximity, GALAXY galaxy, SCENARIO scenario, AuTime asteroidImpact, quint32 joinEmpire,
			const AuId& joinSectorId, const QString& newEmpireName, const QImage& newEmpireFlag);

	// Called when player wants to sell stuff to the building they are in.
	void StartSalesDialog();

	// 'sound' is a SOUND_EFFECT
	void StopClientSound(int sound);

	// Stops all client sounds.
	void StopClientSounds();

	void StoreOfferToBuy(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd);

	void SwitchToAvatar(const AuId& personId);

	void SystemRename(const QString& nuName);

	// Destroys the OpenAL context. Deletes the sound object. No more sounds can be made after this call, without InitializeSound.
	void TerminateSound();

	void TestClientConnection();	// Test client thread for alive. Notify on failure.

	void TransferRequestCitizens(const AuId& shipId);

	void TransferSendCargo(const AuId& otherShipId, const AuItemHash& cargo);

	void TransferSendCargoItem(const AuId& otherShipId, const AuItem& item);

	void TransferSendCitizen(const AuId& otherShipId, const AuId& citizenId, const AuId& swapId);

	void TransferSendCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds);

	void TransferSendItemsAll(const AuId& otherShipId);

	void TransferSendItemsLoose(const AuId& otherShipId);

	void TransferSendItemsStatic(const AuId& otherShipId);

	void TransferSendMoney(const AuId& otherShipId, double amountToTransfer);

	void TransferTakeCargo(const AuId& otherShipId, const AuItemHash& cargo);

	void TransferTakeCargoItem(const AuId& otherShipId, const AuItem& item);

	void TransferTakeCitizen(const AuId& otherShipId, const AuId& citizenId, const AuId& swapId);

	void TransferTakeCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds);

	void TransferTakeItemsAll(const AuId& otherShipId);

	void TransferTakeItemsLoose(const AuId& otherShipId);

	void TransferTakeItemsStatic(const AuId& otherShipId);

	void TransferTakeMoney(const AuId& otherShipId, double amountToTransfer);

	// Returns the number of bytes transmitted.
	int TransmitChatBuffer(QTcpSocket& tcpSocket);

	// Returns the number of bytes transmitted.
	int TransmitSceneBuffer(QTcpSocket& tcpSocket);

	void TransporterCoord(const AuPoint& personLoc);

	void TransporterLongLat(const AuGlobeLLF& ll);

	void TransporterPlaces();

	void TransporterSpacecraft();

	void TransporterTransporters();

	void UpgradeDNA(const AuDNA& dna);

public slots:
	void AcceptCrew(bool accept);

	void AcceptPassengers(bool accept);

	// Configures access policy for the ship the person is currently aboard.
	void AccessPolicy(int policy);

	void AcceptTrade(AuTradeAgreement ta);

	void AcceptTroops(bool accept);

	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender);

	void AdminGoTo(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail);

	// Configures alert condition for the ship the person is currently aboard. Only applies to DaSc ships.
	void AlertCondition(int condition);

	void AllowPassengerSeaChests(bool allow);

	void ArenaReady(AuArenaReady arena);

	void AtomCollision(AuId myId, AuPoint myChange, AuPoint myHitLoc, AuId hitId, AuPoint hitChange, AuPoint hitFromLoc,
				AuPoint hitResultVec, bool hitShield);

	void AtomDeadheaded(AuId departingSystem, AuId travId, AuPoint travLoc);

	void AtomEnteredWormhole(AuId travId, AuId systemId, AuWormhole wh, float entryAngle);

	void AtomFell(AuId fellId, quint32 hitsDamage, int stopReason);

	// A local atom is travelling from one solar system to another.
	// This is where the trail begins when an atom departs a solar system, either through a wormhole or by dead-head through space.
	// This is not where spawning happens.
	// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void AtomTraveled(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector);

	void AtomTraveledGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTraveledStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	// Called daily by avatars whose account is active, to stave off decay.
	// 'land' is a list of the solar systems they have declared to be their land.
	void AvatarAccountActive(AuIdList land);

	// Emitted when a nu baby avatar is born. Adds the nu baby to the mother's login account, if there is an empty slot.
	void AvatarBorn(AuId motherAvatar, AuId babyAvatar);

	void AvatarOnLineUp(AuId personId, quint32 empireId);

	void Attack(AuId targetId, QString targetName);

	void AutoFetchVehicles(bool allow);

	void BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& fBuildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk);

	// 'fenceStyle' is an AuScFence::STYLE
	// 'mat' is the construction material, for custom designs. n/a for built in designs.
	void BdBuildFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& onPlatform,
			quint32 customDesignId, COMMODITYID mat);

	void BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& fBuildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk, COMMODITYID storeCID);

	void BdConstructionEnabled(bool enabled, const AuId& opBd);

	void BdConstructionFetch(const AuId& opBd);

	void BdConstructionGo(const AuId& opBd);

	void BdDesign(AuBdDesign rec);

	void BdDesignDelete(quint32 designId);

	void BdDesignExchangePublished(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublished(quint32 designId);

	void BdDesignNeeded(quint32 designId);						// Scene says it needs an AuBdDesign.

	void BdDesignNeeded(quint32 designId, quint8);				// Server says I need an AuBdDesign with a specific state.

	void BdExchangeRequestDesigns();

	void BdFoundCity(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& fBuildLoc, float zRotation,
			const AuId& onPlatform, quint16 mpid, bool workerOk, const QString& cityName, const QString& systemName,
			const QString& sectorName);

	void BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd);

	void BdManufactureCitizenEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd);

	void BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureFetch(quint16 panelIndex, const AuId& opBd);

	void BdManufactureFetchMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureGo(quint16 panelIndex, const AuId& opBd);

	void BdManufactureGoMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessesClear(const AuId& opBd);					// Removes all processes.

	void BdManufactureProcessLowerMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessMaxMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessMinMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessRaiseMPID(quint16 mpid, const AuId& opBd);

	void BdManufactureProcessesSetMinQ(quint8 minQ, const AuId& opBd);	// Sets minQ for all processes.

	void BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd);

	void BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd);

	void BdManufactureSpacecraftCitizenEnabled(quint32 enabled, const AuId& opBd);

	// When operated from a capitol building, 'opBd' is the atom id of the DaBd that is being operated.
	// 'opBd' is null when the scene atom is at the DaBd being operated.
	void BdPatentsRecord(const AuKnownPatents& patents, const AuId& opBd);

	// 'mediaPos' is the media on which to copy the patents.
	void BdPatentsRequested(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos, const AuId& opBd);

	void BdPreview(const AuPoint& fBuildLoc, float zRotation);

	void BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc);

	void BdSpacecraftOrderDelete(quint32 designId, const AuId& opBd);

	void BdSpacecraftProcessBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd);

	void BdSpacecraftProcessCanceled(const AuId& opBd);

	void BdSpacecraftProcessCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd);

	void BdSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ, const AuId& opBd);

	void BdSpacecraftProcessSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd);

	void BdSpacecraftProcessSetMinQ(quint8 minQ, const AuId& opBd);

	void BlueprintRatedBd(AuExchangeRatingRec rating);

	void BlueprintRatedSc(AuExchangeRatingRec rating);

	void BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void ChatReceived(QByteArray msg);

	void ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy);

	void CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId);

	// 'cityType' is a CITYTYPE enum value.
	// 'galaxy' is a GALAXY enum value.
	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	void CityDecayed(quint32 empireId, AuId cityId);

	// Propagate city industries up to master server to modify master empire record.
	void CityFacilitiesChanged(AuId cityId, AuCityIndustryHash facil);

	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CityStatusChanged(AuId cityId, int alertCondition, qint8 morale);

	void ClaimTitleUp(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void CloseStory(AuJobOffer jobToClose);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void ConnectChat();

	void ConnectUdp();

	void CreateArena(AuArenaReady arena);

	void CreateEmpireUI();

	void CreditCityBank(float fAmount, AuId cityId);

	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	void DeleteSolitaireSave(const QString& fileName);

	// Emitted when a demiavatar is killed. Removes the avatar from the owner's login account.
	void DemiavatarDied(AuId avatar);

	void DesignerRefreshObstructions();

	void DropGearItem(AuGearPos pos);

	// 'industry' is an INDUSTRYID
	void EmpireChgBdPref(quint32 empireId, int industry, quint32 designId);

	void EmpireChgFlag(quint32 empireId, QImage flag);

	// 'scrole' is an AuScPrefs::SCROLE
	void EmpireChgScPref(quint32 empireId, int scrole, quint32 designId);

	void EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies);

	void EmpirePatentsRecord(const AuKnownPatents& patents);

	// 'mediaPos' is the media on which to copy the patents.
	void EmpirePatentsRequested(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos);

	// Records a patent in an empire.
	void EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ);

	void EmpireRequested(quint32 empireId);

	void EnteredDesigner(AuId personId, int designMode);

	void EnterStudio(int panelIndex, QString password);

	void EstablishNewEmpire(AuId personId, QString personName, quint8 personGender, quint32 idOldEmpire, QString name,
			QImage imFlag);

	void Follow(AuId targetId, QString targetName);

	void GameOver(AuId ship, quint16 whichTable, quint8 gameType, QByteArray gameResult);

	void GameTurn(AuId ship, quint16 whichTable, AuId personsTurn);

	void GeoLandscape(const AuPoint& roadStart, const AuPoint& roadEnd, float roadWidth, bool hideRocks, int hideGrass,
			bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement);

	// 'geoType' is the AuGeologicalFeature::Type of the road.
	void GeoRoad(const AuPoint& roadStart, const AuPoint& roadEnd, bool isSlab, float roadWidth, quint8 geoType);

	void GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc);

	void GoDirectToCoord(AuPoint galaxyLoc);

	void GoLandAtCity(AuId cityId, QString cityName);

	void GoLand(AuId worldId, QString worldName);

	void GoOrbit(AuId worldId, QString worldName);

	void GoOrbitCity(AuId cityId, QString cityName);

	void GoToCoord(AuPoint systemLoc);

	void GovBossChanged(quint32 empire, AuId governorId, AuId bossId);

	// 'empireId' is the empire storing the document in their dossier.
	// 'dosEmpire' is the empire in the dossier.
	// 'packedDoc' is the document to store in the dossier, created using the Pack function of AuDocument.
	void GovDossierAddDocument(quint32 empireId, quint32 dosEmpire, QByteArray packedDoc);

	void GovDossierAddNote(quint32 dosEmpire, const QString& docNote);

	// 'docTime' and 'docType' together are used to identify the exact document.
	void GovDossierChangeState(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
			quint8 docState);

	void GovDossierDispatchCourier(quint32 toEmpire, const QString& message);

	void GovDossierDocState(quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex, quint8 docState);

	void GoViaWormholes(AuId systemId, QString systemName);

	void GovSetAnnouncements(const QString& announcements);

	void GovSetAnnouncements(quint32 empire, QString announcements);

	// 'sector' is NULL is setting empire capital
	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapitalUp(quint32 empire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);

	void GovSetCommerce(quint32 idEmpire, QByteArray commerce);

	void GovSetCommerce(QByteArray commerce);

	void GovSetContrailColor(quint32 idEmpire, quint8 hue, quint8 sat);

	void GovSetDefaultPoliticalStance(quint32 empire, quint8 stance);

	void GovSetDefaultPoliticalStance(int stance);

	void GovSetJoinPasswordUp(quint32 empire, QByteArray passCrypt);

	void GovSetTax(quint32 empire, quint8 income, quint8 sales, quint8 subsidy);

	void GovSetTaxes(double income, double sales, double subsidy);

	void GovSetThemeColor(quint32 empire, quint8 theme, QColor color);

	void GovSetWarCriteria(quint32 empire, AuWarCriteria crit);

	void GovSetWarCriteria(const AuWarCriteria& crit);

	void HandCraftGo(int mpid);

	void HengeActivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName,
			QString worldName, quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void HengeTeleporterEntered(AuId teleportId, AuId fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
			AuId destSysId, quint8 destSun, quint8 destArc);

	void InscribeGearItem(AuGearPos pos, QString inscription);

	void ItDesign(AuItDesign rec);

	void ItDesignDelete(quint32 designId);

	void ItDesignNeeded(quint32 designId);						// Scene says it needs an AuItDesign.

	void ItDesignNeeded(quint32 designId, quint8);				// Server says I need an AuItDesign with a specific state.

	void KickBerthOccupant(const AuId& occupant);

	void KickCryoBerthOccupants(const AuSpecies& species, int count);

	void KickStowaways();

	void LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire);

	void LoginSucceeded();

	// Moves a gear item at 'fromPos' to an existing gear item at 'toPos'. This could put an item into a container, plant a seed,
	// reload a weapon, etc.
	void MoveGearItemToItem(AuGearPos fromPos, AuGearPos toPos);

	// Moves a gear item at 'fromPos' to a new empty top level slot of 'toSlotType'.
	// 'toSlotType' is a AuGearSlot::SLOTTYPE.
	void MoveGearItemToSlot(AuGearPos fromPos, quint8 toSlotType);

	void NameSpecies(AuSpecies species, QString speciesName, AuId discoverer);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OfficeRevoked(AuId person, quint32 empire);

	void OfficerKilledUp(const AuId homePlanetId);

	// 'propagateUp' is true to allow the channel to span servers in its quest for the target atom.
	void OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope, bool propagateUp);

	void OperateTurboLiftUI(AuId shipId, AuId personId, quint16 liftId);

	// Configures pilot up policy for the ship the person is currently aboard.
	void PilotUpPolicy(int policy);

	void PostJobOffer(AuJobOffer jobOffer);

	void ProposeTrade(AuTradeAgreement ta);

	void QueueSound(const QByteArray& soundData);

	void ReceiveShipFunds(double amount, AuId idShip);

	void ReconcileRoster();

	void RedeemGearItem(AuGearPos pos);

	// ReleaseBerthUp gets the id of the nu ship to which the person is
	// assigned, which could be 0 if they are simply releasing their berth.
	// This helps insure the berth/roster data doesn't get out of sync.
	// The logic causes all ships that are not the ship id to remove the
	// person's id from their roster, if it's there. That way a person can't
	// accidentally end up consuming berths on multiple different ships.
	// This slot is used when propagating the ReleaseBerth up to the master
	// server.
	void ReleaseBerthUp(AuId personId, AuId shipId);

	void RenameAvatar(AuId avatarId, QString nuName);

	void RenameCity(AuId cityId, QString nuName);

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSector(AuId sectorId, QString nuName);

	void RenameSystem(AuId systemId, QString nuName);

	void ReplaceSpecies(DzSpecies species);

	void ReportMemoryUsage();

	void ReportCitiesByMail(const AuIdList& cityIds);

	void ReportCitiesHistoryByMail(const AuIdList& cityIds);

	void ReportCitiesLastByMail(const AuIdList& cityIds);

	void ReportCityHistory(const AuId& cityId);

	void ReportCityLast(const AuId& cityId);

	void RequestAvatars();

	// Atom on client emits a signal when a new console is manned to send locally stored settings to the server.
	// 'stationType' is a DpStation::STATION_TYPE
	void RestoreConsoleSettings(quint8 stationType, QByteArray consoleSettings);

	// Restores the last saved solitaire avatars.
	// Caller must delete the pointers.
	// Return values:
	// 0 = Success
	// 1 = Unable to open Av file.
	// 2 = Bad Av file type or version.
	// 3 = File opened ok but no avatars were contained there.
	// 4 = Avatar count in the file exceeds 6. This is not valid so no avatars were read at all.
	// 5 = Atom type in file is not ATOM_Avatar.
	int RestoreSolitaireAvatar(DaPerson* avatars[6]);

	void RestoreSolitaireSave(const QString& fileName);

	// Restores the last saved solitaire universe.
	// Returns false on failure.
	bool RestoreSolitaireUniverse();

	// 'role' is 0=none, 1=crew, 2=troop, 3=passenger
	void ReviveCryoBerthOccupant(const AuSpecies& species, quint8 role);

	// 'name' is a suffix added to the default check point save name.
	void SaveCheckPoint(const QString& name) const;

	// Saves a check point only if there is not already a check point dated today.
	void SaveCheckPointToday() const;

	// Saves the current solitaire universe.
	void SaveSolitaire() const;

	// Saves the solo player account avatars to a file.
	// Returns false on failure for any reason, including no avatars to save.
	bool SaveSolitaireAvatars(const QString& fileName) const;
	bool SaveSolitaireAvatarsFile(QFile& file) const;

	void SaveSolitaireUniverse(const QString& fileName) const;
	void SaveSolitaireUniverseFile(QFile& file) const;

	void ScDesign(AuScDesign rec);

	void ScDesignDelete(quint32 designId);

	void ScDesignExchangePublished(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublished(quint32 designId);

	void ScDesignNeeded(quint32 designId);						// Scene says it needs an AuScDesign.

	void ScDesignNeeded(quint32 designId, quint8);				// Server says I need an AuScDesign with a specific state.

	void SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId);

	void SendControlPackets();

	// 'fromGalaxy' is the GALAXY of the sender.
	void SendFundsUp(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void SendShipFunds(double amount, AuId idShip);

	void ScExchangeRequestDesigns();

	void SetFlowerPot(AuGearPos pos, quint8 plantSize);

	void SetMessage(AuGearPos pos, QString inscription);

	void SetPictureFrame(AuGearPos pos, QString url, bool hideURL, bool rotate);

	void SetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent);

	// Sets political stance of 'fromEmpire' toward 'toEmpire' as a result of initial contact between units of two empires.
	// 'fromStance' is the stance taken by the fromEmpire.
	// 'vassalTributePercent' is relevant when becoming a vassal.
	// 'fromUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'toStance' is the stance taken by the toEmpire.
	// 'toUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'reason' is an AuDocument::Type() value, for the dossier.
	// 'galaxy', 'sector', 'system' and 'world' identify where this incident occurred. System and world may be blank.
	void SetPoliticalStance(quint32 toEmpire, quint32 fromEmpire, quint8 fromStance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

	// Configures shield fuel policy for the ship the person is currently aboard.
	void ShieldFuelPolicy(int policy);

	// Configures shield up policy for the ship the person is currently aboard.
	void ShieldUpPolicy(int policy);

	void ShipIt(AuItemTEUContainer container);

	void SolarSystemExplored(const AuId systemId, quint32 empireId);		// On the way up the server chain.

	void SolarSystemFinal(AuId systemId);

	void SolarSystemReadyToDecay(AuId systemId);

	// 'genHab' is a HABITABILITY enum
	void SolarSystemScanned(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed);		// up the server chain

	void SoundGainChanged(int sound);

	void SoundOverrideChanged(int sound);

	void SpacecraftNameChanged(AuId shipId, QString nuName);

	void SpacecraftPositionReport(AuId shipId, AuId systemId, AuStarMapSpacecraft pos);

	void SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtom(AuId person, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	// Request a specie record from the server. It will send a reply, even if the specie has never been named.
	void SpeciesRequested(const AuSpecies& species);

	void StargateClosed(AuId fromGalaxy, AuPoint fromGalaxyLoc);

	void StargateOpened(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuTime expire);

	// Call this function with the unit id chosen for the online scene. The id can be null when establishing a new empire.
	// This starts the process of logging in to the scene servers, starting with the master server. The master will
	// defer us to a manager who will then defer us to the worker who is hosting our active unit.
	void StartSceneCommunications(AuId personId);

	// Call this function with the unit id chosen for the solitaire scene. The id must not be null.
	// This completes the process of starting or resuming a solitaire game.
	void StartSolitaire(AuId personId);

	void SteamCreateStarship();

	void SteamCreateStation();

	void SteamEquipAvatar();

	void StoryCancelSubmission(const AuId& avatarId);

	void StoryHint(quint32 storyInstance);

	void StoryResendBlock(quint32 storyInstance);

	void StoryReset(const AuId& avatarId, quint32 scriptId);

	void StorySubmitScript(const AuId& avatarId, const AuScript& script);

	// This one is on the way up the chain to the master.
	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

	// Scene atom wants to enter a cartesian coordinate at a transporter console.
	void TransporterCoordUI(AuPoint personLoc);

	// Scene atom wants to enter a longitude latitude coordinate at a transporter console.
	void TransporterLongLatUI(AuGlobeLLF ll);

	// Squelch Management - notify the server of changes
	void UpdateSquelch(AuId idContact, bool bDelete);

	// Squelch Management - notify the server of changes
	void UpdateSquelches(AuIdList idContacts, bool bDelete);

	void UseGearItem(AuGearPos pos);

	void WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId);

	void WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

protected:
	QByteArray& LockChatTcpBuffer(const char* /*who*/)
	{
		//qDebug("LockChatTcpBuffer - %s", who);
		m_muChatTcpBuffer.lock();
		//qDebug("LockChatTcpBuffer - %s acquired", who);
		return m_ChatTcpBuffer;
	}

	QByteArray& LockSceneTcpBuffer(const char* /*who*/)
	{
		//qDebug("LockSceneTcpBuffer - %s", who);
		m_muSceneTcpBuffer.lock();
		//qDebug("LockSceneTcpBuffer - %s acquired", who);
		return m_SceneTcpBuffer;
	}

	void Logout();

	virtual bool notify(QObject* receiver, QEvent* event);

	void ReportMemoryUsage(bool noLocks);

	void QueueChatPacket(const QByteArray& ba);		// Adds a packet to the chat stream buffer.

	void QueueScenePacket(const QByteArray& ba);	// Adds a packet to the scene stream buffer.

	inline bool SceneConnectedOnce() const
	{
		return m_SceneConnectedOnce;
	}

	void SendChatConnect();

	void SendMail(AuMailMsg::MSGTYPE msgType, const AuId& fromId, const QString& fromName, const AuId& toId,
			const QString& subject, const QString& bodyText) const;

	void SendUdpPacket(const QByteArray& ba);		// Sends a packet on the datagram socket.

	void StartSceneClientThread();

	void StopThreads();

	void UnlockChatTcpBuffer(const char* /*who*/)
	{
		//qDebug("UnlockChatTcpBuffer - %s", who);
		m_muChatTcpBuffer.unlock();
	}
	
	void UnlockSceneTcpBuffer(const char* /*who*/)
	{
		//qDebug("UnlockSceneTcpBuffer - %s", who);
		m_muSceneTcpBuffer.unlock();
	}

protected slots:
	void ChannelOpened(QString channelName, quint8 scope, QString narrDesc);

	void ConnectionEstablished();

	// 'idAtom' is the player's new sector when deferred to a scene manager.
	// 'idAtom' is the player's new homeworld when deferred to a scene worker.
	void EstDeferred(AuId idAtom);

	void InitSucceeded(AuId idUnit);

	void LoggedOut();

	void LoginFailed(quint8 errCode);

	virtual void PrepareToQuit();

	void ReadPendingDatagrams();

	void RestartNewAvatar();

	void SceneDeferred(int deferType);

	void SceneDisconnected();

	void SceneLoggedIn();

	void StartNewAvatarError(unsigned char reason);

private:
	// 'avatarIndex' must be 0 to 5. Should index an empty avatar slot.
	// Inits a new universe for solitaire game play, if needed. Returns the id of the nu avatar.
	AuId CreateSoloAvatar(int avatarIndex, unsigned char proximity, GALAXY galaxy, const QString& avatarName, const AuDNABase* dna,
			AuDNABase::CHROMATID chroma, HANDEDNESS hand, SCENARIO scenario, const AuTime& asteroidImpact, quint32 joinEmpire,
			const AuId& joinSectorId, const QString& newEmpireName, const QImage& newEmpireFlag);

	void ForgetEmpireRequests();

	bool InitCitizenOfTheGalaxy(AuEther& ether, DaPerson* person, quint32 joinEmpire, GALAXY galaxy, const AuId& joinSectorId);

	bool InitLostAtlantis(AuEther& ether, DaPerson* person, unsigned char proximity, GALAXY galaxy, const QString& empireName,
			const QImage& empireFlag);

	bool InitReachForTheStars(AuEther& ether, DaPerson* person, char proximity, GALAXY galaxy, const AuTime& asteroidImpact);

	bool InitSavageState(AuEther& ether, DaPerson* person, unsigned char proximity, GALAXY galaxy);

	void InsertSquelchIds(QDataStream& out);

	void ProcessControlInput(CONTROL_CONTEXT localMotion, const AuId& toAtom, const DiControlInputArray& aActiveControls,
			const AuGearPos& pos, qint8 selValue, int msElapsed);

	void SoloChat(const AuCommPacket& msg);

	void SoloChatToClient(AuCommPacket* msg, AuEther& ether);

	void SummonDormantShip(const AuId& shipId, const AuId& crewId, GALAXY toGalaxy);

signals:
	void ArenaReadySignal(AuArenaReady arena);

	void AvatarsUpdated();

	void BdExchangeBlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating);

	void BdExchangeUpdated();

	void BdLibraryUpdated();

	void BrokerShipment(AuShipment shipment);

	void BrokerShipmentsClear();

	void BrokerTradeAgreement(AuTradeAgreement ta);

	void BrokerTradeAgreementsClear();

	void CityHistoryReportReady(AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId cityId, AuCityReport report);

	void HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar);

	void JobAvailable(AuJobOffer job);

	void KeyPressed(QKeyEvent* e);

	void KeyReleased(QKeyEvent* e);

	void MissionSensorContactsReceived(QByteArray msg);

	void OrdnanceReceived(AuId loaderId, QByteArray ord);

	void PlayersOnlineStatus(AuPlayerOnlineStatusVector players);

	void ScExchangeBlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating);

	void ScExchangeUpdated();

	void ScLibraryUpdated();

	void ServerConnected();

	void SoloChannelOpened(QString channelName, quint8 scope, QString narrDesc);

	void SoloChatReceived(QByteArray msg);

	void TransferCitizensAvailable(QByteArray ba);

	void TransporterPlace(AuTransporterTarget tt);

	void TransporterShip(AuId id, QString name, QString desc);

	void TransporterTransporter(AuId id, QString name, QString desc, quint16 whichTransporter);
};

#define auApp	((AuApp*)qApp)

#endif // AuApp_H
