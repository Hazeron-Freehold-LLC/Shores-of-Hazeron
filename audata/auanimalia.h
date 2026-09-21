//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auanimalia.h,v 1.71 2022/05/09 16:21:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAnimalia_H
#define AuAnimalia_H

#include "auanimalpart.h"
#include "auclothing.h"
#include "aupose.h"

class AuAnimalPart;
class AuCombatWeapon;
class AuId;

class AuAnimalia : private QList<AuAnimalPart*>
{
private:
	// DaCreature uses these to store its gender, DNA, handedness and special weapon use time.
	AuDNA::CHROMATID					m_Chain;
	AuDNA								m_DNA;
	GROWTHSTAGE							m_GrowthStage;
	HANDEDNESS							m_Handedness;
	qint16								m_HitPoints;
	AuTime								m_LastSpecUse;			// Time special attack was last used.

	// Render states.
	bool								m_BendDominantArm;
	AuClothing							m_Clothing;
	AuHairStyle::STYLE					m_Haircut;
	float								m_HeadTilt;				// Head tilt angle. 0 = look ahead. + look up. - look down.
	float								m_HeadTurn;				// Head turn angle. 0.0 = none = straight ahead.
	AuPose								m_Pose[AuAnimalPart::PS_Count];		// Current body pose.
	float								m_PoseHz;				// Pose animation frequency in cycles per second.
	AuHairStyle::SHAVE					m_Shave;

	// Render objects.
	AuClothingTextureArray				m_AnimalClothingTex;
	// Full body VAOs
	AuGLVAO								m_AnimalVAO;
	AuGLVAO								m_AnimalClothingVAO;
	AuGLVAO								m_AnimalClothingBlendVAO;
	AuGLVAO								m_AnimalHelmetBlendVAO;	// Just the visor of the helmet.
	AuGLVAO								m_AnimalHairBottomVAO;	// Hair is always blended.
	AuGLVAO								m_AnimalHairTopVAO;		// Hair is always blended.
	// Head only VAOs
	AuGLVAO								m_HeadVAO;
	AuGLVAO								m_HeadClothingVAO;
	AuGLVAO								m_HeadClothingBlendVAO;
	AuGLVAO								m_HeadHelmetBlendVAO;	// Just the visor of the helmet.
	AuGLVAO								m_HeadHairBottomVAO;	// Hair is always blended.
	AuGLVAO								m_HeadHairTopVAO;		// Hair is always blended.

public:	// Constructors/Destructors
	AuAnimalia(const AuDNA& dna, AuDNA::CHROMATID chain, HANDEDNESS handedness, GROWTHSTAGE stage);
	AuAnimalia();
	~AuAnimalia();
	AuAnimalia(const AuAnimalia& c);

public:	// Operators
	const AuAnimalia& operator=(const AuAnimalia& c);

	//void operator>>(QDataStream& out) const;
	//void operator<<(QDataStream& in);

public:	// Methods
	int AbilityPointsUsed() const;

	// 'id' is used to offset things so all creatures don't move in sync.
	void AnimatePose(const AuId& id, const AuElapsedTimer& when);

	// Dying pose returns the corresponding dead pose within 3 seconds of actual death. So the body looks really dead briefly.
	AuPose ApparentSectionPose(AuAnimalPart::SECTION ps) const;

	// Checks to see if this animal is valid for use as an avatar.
	// Returns a bitmask of reasons why not. 0 indicates ok, no problem.
	// 0x0001 = ability points invalid
	// 0x0002 = hit points invalid
	// 0x0004 = camera height invalid when standing
	// 0x0008 = cg height invalid when standing
	// 0x0010 = head height invalid when standing
	// 0x0020 = camera height invalid when sitting
	// 0x0040 = head height invalid when sitting
	// 0x0080 = atmosphere breathability
	int AvatarCheck();

	inline bool BendDominantArm() const
	{
		return m_BendDominantArm;
	}

	// Returns true if the info describes the special weapon.
	// 'targetRange' is distance to the target from my body pivot point.
	// 'range' returned is range of body weapon relative to my body pivot point.
	bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon,
		bool okToUseSpecialWeapon=true) const;

	float BiteRange() const;

	// Returns maximum burden this animal can carry in lbs.
	float BurdenMax() const;

	// Current eye location.
	AuPoint CameraLocation() const;

	bool CanHover() const;

	// Returns the CG height above the ground in my current pose.
	float CGHeight() const;

	inline AuDNA::CHROMATID Chromatid() const
	{
		return m_Chain;
	}

	// Sets my VAOs to 0 without calling DeleteGL on them. OpenGL context not required.
	void ClearVAOs();

	// Copies render objects and render states from 'c'.
	void CopyVAOs(const AuAnimalia& c);

	// Creates VAOs needed for render using animal shader.
	// 'unique' is used to make clothing that varies slightly between wearers. e.g. wearer's id
	void CreateAnimalVAO(AuOpenGLFunctions& re, quint32 unique=0);

	// Call this BEFORE CALLING CreateMeshes.
	// An OpenGL context must be current.
	void CreateClothingTex(AuGLEffects& re);

	// Be sure to call CreateClothingTex before calling this function.
	// This guy does not require an OpenGL context to be current.
	// 'unique' is used to make clothing that varies slightly between wearers. e.g. wearer's id
	void CreateMeshes(AuMeatMesh& skinMesh, AuClothingMesh& clothingMesh, quint32 unique) const;

	// Creates/recreates my VAOs using skin and clothing meshes.
	// An OpenGL context must be current.
	void CreateVAOs(AuOpenGLFunctions& re, const AuMeatMesh& skinMesh, const AuClothingMesh& clothingMesh);

	inline const AuPose& CurrentGeneralPose() const
	{
		return CurrentSectionPose(AuAnimalPart::PS_General);
	}

	// For positioning a carried item. Result is based on current posed position of the hand.
	// +X is the aim direction of the palm.
	// +Z goes in the direction of the thumb.
	// On the left hand, -Y comes out of the palm.
	AuMatrix4x4 LeftHandMatrix() const;

	inline float CurrentHeadTilt() const
	{
		return m_HeadTilt;
	}

	inline float CurrentHeadTurn() const
	{
		return m_HeadTurn;
	}

	inline qint16 CurrentHitPoints() const
	{
		return m_HitPoints;
	}

	inline const AuPose& CurrentMouthPose() const
	{
		return CurrentSectionPose(AuAnimalPart::PS_Mouth);
	}

	inline const AuPose* CurrentPoses() const
	{
		return m_Pose;
	}

	// For positioning a carried item. Result is based on current posed position of the hand.
	// +X is the aim direction of the palm.
	// +Z goes in the direction of the thumb.
	// On the right hand, +Y comes out of the palm.
	AuMatrix4x4 RightHandMatrix() const;

	inline const AuPose& CurrentSectionPose(AuAnimalPart::SECTION ps) const
	{
		return m_Pose[ps];
	}

	// Returns the number of seconds per standard body cycle, which is based its the Speed.
	float CycleSpeed() const;

	inline static qint16 DeadHitPoints()
	{
		// At zero, the body starts dying.
		// At this number of hit points, the body is dead.
		return -30;
	}

	// OpenGL context must be current.
	void DeleteGL();

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	// Return density in lbs/ft^3
	float Density() const;

	QString Description() const;

	inline const AuDNA& DNA() const
	{
		return m_DNA;
	}

	inline const AuChromatid& DNAChain() const
	{
		return m_DNA.Chain(m_Chain);
	}

	ECOROLE EcoRole() const;

	AuDNABase::ECOSTRATEGY EcoStrategy() const;

	inline bool Empty() const
	{
		return empty();
	}

	float FieldOfViewAngle() const;

	GENDER Gender() const;

	// Returns the number of minutes this creature needs to grow when it has emerged from an implant or been born and is in its
    // "growth" state.
	quint8 GrowthTime() const;

	inline AuHairStyle::STYLE Haircut() const
	{
		return m_Haircut;
	}

	inline HANDEDNESS Handedness() const
	{
		return m_Handedness;
	}

	bool HasArms() const;

	bool HasLegs() const;

	bool HasMaleGenitals() const;

	bool HasNeck() const;

	bool HasTail() const;

	bool HasVAOs() const;

	bool HasWings() const;

	// Returns the matrix that will place the head at the end of the neck, ready for posing from there.
	const AuMatrix4x4& HeadBaseMatrix() const;

	AuPoint HeadLocation() const;

	// Y distance to the eye plane, for camera positioning.
	float HeadThickness() const;

	float HearingRange() const;

	void Init();					// Initialize all body parts.

	static void InitPoses(const AuPose& pose, AuPose poses[AuAnimalPart::PS_Count]);

	bool Is(const AuDNA& dna, AuDNA::CHROMATID chain, HANDEDNESS handedness) const;

	bool IsAbilityPointsValidForPerson() const;

	inline bool IsBerthPose() const
	{
		return CurrentGeneralPose().IsBerth();
	}

	inline bool IsBitingPose() const
	{
		return m_Pose[AuAnimalPart::PS_HeadNeck].IsBiting();
	}

	bool IsCameraHeightValidForPerson() const;

	bool IsCgHeightValidForPerson() const;

	// True if in a console pose that requires a DpConsole.
	// Like IsLockedPose except these poses require a DpConsole.
	inline bool IsConsolePose() const
	{
		return CurrentGeneralPose().IsConsole();
	}

	inline bool IsCrouchedPose() const
	{
		return CurrentGeneralPose().IsCrouched();
	}

	inline bool IsDead() const
	{
		return CurrentGeneralPose().IsDead();
	}

	inline bool IsDying() const
	{
		return CurrentGeneralPose().IsDying();
	}

	inline bool IsEatingPose() const
	{
		return m_Pose[AuAnimalPart::PS_HeadNeck].IsEating();
	}

	bool IsExoskeletal() const;

	bool IsFlyer() const;

	inline bool IsFlyingPose() const
	{
		return CurrentGeneralPose().IsFlying();
	}

	inline bool IsGameTablePose() const
	{
		return CurrentGeneralPose().IsGameTable();
	}

	bool IsHeadHeightValidForPerson() const;

	bool IsHitPointsValidForPerson() const;

	inline bool IsJumpingPose() const
	{
		return CurrentGeneralPose().IsJumping();
	}

	inline bool IsJuvenile() const
	{
		return m_GrowthStage == Juvenile;
	}

	inline bool IsLadderPose() const
	{
		return CurrentGeneralPose().IsLadder();
	}

	bool IsLeaper() const;

	bool IsLeftHanded() const;

	inline bool IsLockedPose() const
	{
		return CurrentGeneralPose().IsLocked();
	}

	inline bool IsMature() const
	{
		return m_GrowthStage == Mature;
	}

	inline bool IsMovingPose() const
	{
		return CurrentGeneralPose().IsMoving();
	}

	inline bool IsPostPose() const
	{
		return CurrentGeneralPose().IsPost();
	}

	inline bool IsPronePose() const
	{
		return CurrentGeneralPose().IsProne();
	}

	inline bool IsSwimmingPose() const
	{
		return CurrentGeneralPose().IsSwimming();
	}

	inline bool IsTroopStationPose() const
	{
		return CurrentGeneralPose().IsTroopStation();
	}

	inline bool IsTurretIntegratedPose() const
	{
		return CurrentGeneralPose().IsTurretIntegrated();
	}

	inline bool IsTurretPose() const
	{
		return CurrentGeneralPose().IsTurret();
	}

	float JumpForce() const;

	AuPoint JumpVector() const;

	int LegCount() const;

	bool LivesInAir() const;

	bool LivesInWater() const;

	bool LivesOnLand() const;

	bool LivesUnderGround() const;

	bool LooksLikeVermin() const;

	qint16 MaxHitPoints() const;

	// Returns the maximum speed this body can be moving to still be able to initiate a jump.
	// Slightly faster than run speed works well for lunar bunny hopping.
	float MaxSpeedToJump() const;

	// Returns true if my current mouth pose has expired.
	// 'durationMs' may specify a length of time. If 0, the cycle time of the pose is used. If that is zero, false is returned.
	bool MouthPoseExpired(const AuElapsedTimer& poseStarted, unsigned int durationMs) const;

	// Movement rate based on DNA and a pose.
	// Return value is universe feet per real world second.
	//
	// http://wiki.answers.com/Q/What_is_the_average_running_speed_of_a_man
	// The average running speed over a medium distance for a normal person would be 10 to 18 km/hr (6.25 to 11.2 mph).
	// The average walking speed for a healthy adult is 4 to 5.5 km/hr (2.5 to 3.5 mph).
	// Skilled runners can run around 13 to 21 km/hr (8-13 mph) for long distances (i.e. a marathon). For very short distances, 
	// the current world record for the 100m dash is 9.58 seconds. This translates to 23.4 mph. Instantaneous speeds off the 
	// line can hit up to 28 mph for a short distance.
	// Cruising speed of a dolphin is 8km per hour.
	// Fast cruising speed of a dolphin is 11-12.5km per hour.
	// 
	// 1 kilometer per hour = 0.911344415 feet per second
	//
	// Slowest DNA is 0, which is 1fps = 1.1km per hour
	// Default human DNA is 4, which is 5fps = 5.48km per hour.
	// Fastest DNA is 15, which is 16fps = 17.56km per hour
	float MovementSpeed() const;

	// Returns the matrix that will place the neck at the end of the torso, ready for posing from there.
	const AuMatrix4x4& NeckBaseMatrix() const;

	inline bool NeedsInit() const
	{
		return m_Pose[AuAnimalPart::PS_General] == AuPose::Init;
	}

	inline bool NeedsSpawn() const
	{
		return m_Pose[AuAnimalPart::PS_General] == AuPose::Spawn;
	}

	inline float NoiseLevel() const	// 0..1
	{
		return CurrentGeneralPose().NoiseLevel();
	}

	// "Pose" functions return true if the pose value of any section is changed.
	inline bool Pose(const AuId& id, const AuPose& pose, bool bendDominantArm, float poseFrequency)
	{
		Q_ASSERT(pose.IsGeneral());
		AuPose poses[AuAnimalPart::PS_Count];
		InitPoses(pose, poses);
		return Pose(id, poses, bendDominantArm, poseFrequency);
	}

	bool Pose(const AuId& id, const AuPose pose[AuAnimalPart::PS_Count], bool bendDominantArm, float poseFrequency);

	bool PoseAttackFlying(const AuId& id);

	bool PoseAttackHovering(const AuId& id);

	bool PoseAttackStanding(const AuId& id);

	bool PoseAttackUnderGround(const AuId& id);

	bool PoseAttackUnderWater(const AuId& id);

	bool PoseBurrow(const AuId& id, float poseFrequency);

	bool PoseEatOffGround(const AuId& id);

	void PoseMouth(const AuId& id, const AuPose& pose);

	void PositionArms(float hipHt);

	void PositionHeads();

	void PositionLegs(float hipHt);

	void PositionTail(float hipHt);

	// Returns waist rot
	float PositionTorso(float hipHt);

	void PositionWings(float waistRot);

	void Refresh(const AuAnimalia& c);

	// Render using Animal shader.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	// 'visorState' is the AuItem::State of the helmet. It is 0 when the visor on my helmet is open, 1 when visor is closed.
	void RenderAnimal(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp);
	void RenderAnimalGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderAnimalRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	// Render animal hair using AnimalClothing shader.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	void RenderAnimalClothing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderAnimalClothingGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderAnimalClothingRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	void RenderAnimalHelmetVisor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderAnimalHelmetVisorGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderAnimalHelmetVisorRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	// Render animal hair using AnimalHair shader.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	void RenderAnimalHair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderAnimalHairGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderAnimalHairRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	// Render only my head and hair using Animal shader. No clothing or helmet is rendered.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	void RenderHead(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp);
	void RenderHeadGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderHeadRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			quint8 visorState, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	// Render head hair using AnimalClothing shader.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	void RenderHeadClothing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderHeadClothingGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderHeadClothingRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	void RenderHeadHelmetVisor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderHeadHelmetVisorGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderHeadHelmetVisorRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	// Render head hair using AnimalHair shader.
	// 'blended' is true when rendering the blended parts, in a separate pass after rendering the solid parts.
	void RenderHeadHair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp);
	void RenderHeadHairGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, const AuPoint& globeP);
	void RenderHeadHairRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
			const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS]);

	void ResetHitPoints();

	// 'item' can be 0 to remove the appearance of clothing.
	inline bool SetArmor(const AuItem* item)
	{
		return m_Clothing.SetArmor(item);
	}

	// 'cid' can be CID_Invalid to remove the appearance of clothing.
	inline bool SetArmor(COMMODITYID cid)
	{
		return m_Clothing.SetArmor(cid);
	}

	inline bool SetClothing(const AuItem* item)
	{
		return m_Clothing.SetClothing(item);
	}

	inline bool SetClothing(COMMODITYID cid)
	{
		return m_Clothing.SetClothing(cid);
	}

	void SetDNA(const AuDNA& dna, AuDNA::CHROMATID chromatid);

	inline bool SetGrowthStage(GROWTHSTAGE stage)
	{
		if(m_GrowthStage == stage)
			return false;
		m_GrowthStage = stage;
		return true;
	}

	// Render VAOs must be recreated after calling this.
	inline bool SetHairStyle(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave)
	{
		if(m_Haircut == cut && m_Shave == shave)
			return false;
		m_Haircut = cut;
		m_Shave = shave;
		return true;
	}

	inline bool SetHandedness(HANDEDNESS hand)
	{
		if(m_Handedness == hand)
			return false;
		m_Handedness = hand;
		return true;
	}

	// 'item' can be 0 to remove the appearance of clothing.
	inline bool SetHat(const AuItem* item)
	{
		return m_Clothing.SetHat(item);
	}

	inline bool SetHat(COMMODITYID cid)
	{
		return m_Clothing.SetHat(cid);
	}

	void SetHitPoints(qint16 hitpoints);

	void SetLeftHanded(bool isLefty);

	inline bool SpecialWeaponReady() const
	{
		return m_LastSpecUse.IsNull() || m_LastSpecUse.ElapsedSec() >= 10;
	}

	inline const AuTime& SpecialWeaponUseTime() const
	{
		return m_LastSpecUse;
	}

	inline AuHairStyle::SHAVE Shave() const
	{
		return m_Shave;
	}

	// Returns current body pose frequency, in cycles per second.
	inline float PoseFrequency() const
	{
		return m_PoseHz;
	}

	// Returns the extents of the posed body, without referencing the skin mesh. Used by the server, who does not make a skin mesh.
	void SightExtents(AuPoint& min, AuPoint& max) const;

	float SightRange() const;

	// Tests for intersection by segment from 'eyeLoc' to 'sightedLoc'.
	// Returns false if not hit.
	// Returns true if hit and...
	// - Adjusts 'sightedLoc' to the hit location when hit, presumably closer to 'eyeLoc'.
	// - Sets 'sightedPart' to (AuChromaWhich << 8) | AuAnimalPart::SECTION;
	bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	float SmellRange() const;

	quint8 Stamina() const;

	// Returns the amount of stamina needed by this body to jump.
	quint8 StaminaToJump() const;

	// Returns stamina trend based on body pose in stamina points per second.
	// Positive represents a rejuvenation of stamina.
	// Negative represents a drain of stamina.
	inline int StaminaTrend() const
	{
		return CurrentGeneralPose().StaminaTrend();
	}

	// Returns the location that is best for targeting this body with a weapon.
	AuPoint TargetLocation() const;

	// Tilt head and bend neck to 'tiltAngle', 0=level.
	// Turn head to 'turnAngle', 0=straight ahead.
	inline void TiltHeadsTo(float tiltAngle, float turnAngle)
	{
		float tilt = tiltAngle;
		if(tilt > PIf/2.f)
			tilt = PIf/2.f;
		else if(tilt < -PIf/2.f)
			tilt = -PIf/2.f;
		m_HeadTilt = tilt;
		m_HeadTurn = turnAngle;
	}

	// Returns the matrix at the base (bone 0) of the torso.
	const AuMatrix4x4& TorsoBaseMatrix() const;

	// Thickness (chest to back if upright) of torso, even if none.
	float TorsoThickness() const;

	// Width (side to side) of torso, even if none.
	float TorsoWidth() const;

	float TurnRate() const;

	inline void UseSpecialWeapon(const AuTime& when)
	{
		m_LastSpecUse = when;
	}

	// Compares this animalia to 'c'. Returns a measure of visual alikeness in the range 0 to 100.
	// Used when selecting a GL model to display an animal when the 100 match might not be done yet.
	// DNA is already known to be the same; just needs to check hair and clothing.
	int VisualCompare(const AuAnimalia& c) const;

	// Approximate volume in cubic feet.
	float Volume() const;

	// 'range' returned is range of body weapon relative to my body pivot point.
	void WeaponAtPos(AuDNABase::WEAPONLOC loc, bool useSpecialWeapon, AuCombatWeapon& weapon) const;

	COMMODITYID WornArmor() const;

	COMMODITYID WornClothing() const;

	COMMODITYID WornHat() const;
};

/*
inline QDataStream& operator<<(QDataStream& out, const AuAnimalia& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuAnimalia& c)
{
	c << in;
	return in;
}
*/

AUUTIL_DLL QDebug operator<<(QDebug dbg, const AuAnimalia& c);

#endif // AuAnimalia_H
