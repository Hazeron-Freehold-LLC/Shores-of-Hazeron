//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpbody.h,v 1.195 2022/04/15 21:01:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpBody_H
#define DpBody_H

#include "audata.h"
#include "autime.h"
#include "dbcommodity.h"
#include "dhdna.h"

class Au3dsGLSLModelInstance;
class AuCombatWeapon;
class AuGLEffects;
class AuGLModelCache;
class AuPoint;
class DpBody;
class SkinVertex;

#define SkinTexLim 4
#define ArmorTexLim 4
#define HelmetTexLim 8

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpBodyPart
{
/*
friend class DpHeadAndNeck;
friend class DpLeftArmAndHand;
friend class DpLeftLegAndFoot;
friend class DpRightArmAndHand;
friend class DpRightLegAndFoot;
friend class DpTail;
friend class DpTorso;
*/
protected:
	DpBody*		m_Body;			// Body parts often need access to their parent body.
	DhDNA		m_DNA;			// Store a copy of the DNA so it can be tweaked.
	// Keep some calculated values that never change
	AuPoint		m_ConnectLoc;

public:
	DpBodyPart(DpBody* body);

	virtual ~DpBodyPart()
	{
	}

public:
	inline DpBodyPart(const DpBodyPart& c)
	{
		m_Body = c.m_Body;
		m_DNA = c.m_DNA;
		m_ConnectLoc = c.m_ConnectLoc;
	}

public:
	virtual void OneTimeCalcs() = 0;
	virtual DpBodyPart* Clone(DpBody* body) const = 0;
	virtual AuPoint ConnectLoc() const = 0;	// Returns connect location to body.
	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc) = 0;
	virtual AuPoint CurrentConnectLoc() const = 0;	// Returns connect loc*bone.
	virtual bool IsArmAndHand() const
	{
		return false;
	}
	virtual bool IsHeadAndNeck() const
	{
		return false;
	}
	virtual bool IsLegAndFoot() const
	{
		return false;
	}
	virtual bool IsTail() const
	{
		return false;
	}
	virtual bool IsTorso() const
	{
		return false;
	}

	virtual quint64 MemoryUsage() const;

	virtual void MoveBy(const AuPoint& vMove) = 0;

	virtual void RenderHitBox(AuGLEffects& re) = 0;

	virtual void RenderHitBoxHeadOnly(AuGLEffects& re)
	{
		Q_UNUSED(re);
	}

	virtual void RoughExtentLocs(AuPointArray& locs) const = 0;

	void SetBody(DpBody* body)
	{
		m_Body = body;
	}

	// Returns true if this part is hit.
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const = 0;

	// Returns true if this part is hit.
	virtual bool SightTargetDetailsHead(const AuPoint& /*eyeLoc*/, AuPoint& /*sightedLoc*/, unsigned int& /*sightedPart*/) const
	{
		return false;
	}

	virtual float Volume() const = 0;		// Approximate volume in cubic feet.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpTorso : public DpBodyPart
{
public:
	DpTorso(DpBody* body) : DpBodyPart(body)
	{
	}

	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpTorso(*this);
		bp->SetBody(body);
		return bp;
	}

	void OneTimeCalcs();
	virtual AuPoint ConnectLoc() const
	{
		return AuPoint(0,0);
	}
	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);
	virtual AuPoint CurrentConnectLoc() const;	// Returns connect loc * bone.
	virtual bool IsTorso() const
	{
		return true;
	}
	virtual void MoveBy(const AuPoint& vMove);

	virtual void RenderHitBox(AuGLEffects& re);

	void RotateWaist(float angle);

	virtual void RoughExtentLocs(AuPointArray& locs) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual float Volume() const;		// Approximate volume in cubic feet.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpTail : public DpBodyPart
{
public:
	DpTail(DpBody* body) : DpBodyPart(body)
	{
	}

	void OneTimeCalcs();
	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpTail(*this);
		bp->SetBody(body);
		return bp;
	}
	bool CanBendLaterally() const;	// return true if this tail bends side-to-side

	bool CanBendVertically() const;	// return true if this tail bends up-and-down

	virtual AuPoint ConnectLoc() const;
	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);
	virtual AuPoint CurrentConnectLoc() const;	// Returns connect loc * bone.

	void DeadFacingDown();

	void DeadFacingLeft();

	void DeadFacingRight();

	void DeadFacingUp();

	void Fold();

	virtual bool IsTail() const
	{
		return true;
	}

	void Lash(float hipHt, float speedEnc);

	virtual void MoveBy(const AuPoint& vMove);

	void ReceptiveFacingUp();

	void Relax(float hipHt);

	virtual void RenderHitBox(AuGLEffects& re);

	virtual void RoughExtentLocs(AuPointArray& locs) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	void Slither(const AuPose& pose, float speedEnc);

	void Strike(float hipHt);

	void Swim(const AuPose& poseRhythm);

	virtual float Volume() const;		// Approximate volume in cubic feet.

	void Wag(float hipHt);

	void Wave(float hipHt, float speedEnc);

	void WritheFacingDown();

	void WritheFacingLeft();

	void WritheFacingRight();

	void WritheFacingUp();

protected:
	void Bend(float angIncX, float angIncZ, float hipHt);
	void Slither(float angIncX, float angIncZ);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpHeadAndNeck : public DpBodyPart
{
private:
	int		m_Which;	// Which one among multiples possible (0..3)
	AuPoint	m_HeadLoc;

public:
	DpHeadAndNeck(DpBody* body, int which) : DpBodyPart(body)
	{
		m_Which = which;
	}

	virtual quint64 MemoryUsage() const;

	void OneTimeCalcs();

	void BendNeck(float angle);	// Rotate each neck bone around X.
									// Positive moves head front to back.
	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpHeadAndNeck(*this);
		bp->SetBody(body);
		return bp;
	}
	virtual AuPoint ConnectLoc() const;
	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);
	virtual AuPoint CurrentConnectLoc() const;	// Returns connect loc * bone.

	AuPoint CurrentHeadLoc() const;

	AuPoint CurrentCameraLoc() const;

	AuPoint HeadLoc() const;

	virtual bool IsHeadAndNeck() const
	{
		return true;
	}
	void LeanNeck(float angle);	// Rotate neck around Y at base of neck.
									// Positive moves head to right.
	virtual void MoveBy(const AuPoint& vMove);

	virtual void RenderHitBox(AuGLEffects& re);

	virtual void RenderHitBoxHeadOnly(AuGLEffects& re);

	void RotateNeck(float angle);	// Rotates around X at base of neck.

	virtual void RoughExtentLocs(AuPointArray& locs) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual bool SightTargetDetailsHead(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	void Straighten();				// Straighten out all bones.

	// Tilt head and bend neck to 'tiltAngle', 0=level.
	// Turn head to 'turnAngle', 0=straight ahead.
	void TiltHeadTo(float tiltAngle, float turnAngle);

	virtual float Volume() const;		// Approximate volume in cubic feet.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpArmAndHand : public DpBodyPart
{
protected:
	float	m_Lift;		// Rotation about Y axis when standing. See DpBody::SpreadArms.
	float	m_Splay;	// Rotation about Z axis when standing. See DpBody::SpreadArms.
	int		m_Which;	// Which one among multiples possible (0..2)
	AuPoint	m_ElbowLoc;
	AuPoint	m_WristLoc;
	AuPoint	m_HandLoc;

public:
	DpArmAndHand(DpBody* body, int which) : DpBodyPart(body)
	{
		m_Lift = 0;
		m_Splay = 0;
		m_Which = which;
	}

	virtual quint64 MemoryUsage() const;

	void OneTimeCalcs();
	void Beckon(float headTilt);

	void Block(float headTilt);

	virtual void Bones(DhDNA::BONES& upper, DhDNA::BONES& fore, DhDNA::BONES& hand) const = 0;

	void BowWing(float angle);			// Rotates around Z at each joint.

	void ChangeArmType(DhDNA::ARM type)
	{
		m_DNA.ArmType = type;
	}

	void Climb(bool bendToHoldObject, float speedEnc);

	virtual AuPoint ConnectLoc() const;	// Shoulder loc relative to body origin.

	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);

	virtual AuPoint CurrentConnectLoc() const;	// Returns connect loc * bone.

	AuPoint CurrentElbowLoc() const;		// Return elbow location * bone matrix.

	AuPoint CurrentHandLoc() const;		// Return center of hand location * bone matrix.

	AuPoint CurrentWristLoc() const;		// Return wrist location * bone matrix.

	void Dig();

	bool DoesTouchGroundBelowJoint(float shoulderHt, float upperarmlen,
			float forearmlen, float handlen) const;

	void Eat(float hipHt);	// Pick something up off ground.

	AuPoint ElbowLoc() const;			// Calculate loc relative to body origin.

	void FlapWing(float angle);		// Rotates around Z at shoulder.

	void FlyWing(int speed);	// speed 0=glide, 1=normal, 2=fast

	void Fold(bool bendToHoldObject);

	AuPoint HandLoc() const;				// Calculate center of hand rel to body origin.

	virtual bool IsArmAndHand() const
	{
		return true;
	}
	virtual bool IsLeft() const
	{
		return false;
	}
	bool IsWing() const
	{
		return m_DNA.HasWings();
	}
	bool IsWingLobed() const			// Butterfly, Dragonfly, Moth, Fairy
	{
		return !IsWingSkeletal();
		/*
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly ||
			m_DNA.ArmType == DhDNA::ARM_WingDragonFly ||
			m_DNA.ArmType == DhDNA::ARM_WingFairy ||
			m_DNA.ArmType == DhDNA::ARM_WingMoth)
			return true;
		else
			return false;
			*/
	}
	bool IsWingSkeletal() const			// Feathered and Leathery
	{
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered ||
			m_DNA.ArmType == DhDNA::ARM_WingLeathery)
			return true;
		else
			return false;
	}

	void Jump(float hipHt, bool bendToHoldObject, float speedEnc);

	void Ladder(bool bendToHoldObject);

	virtual void MoveBy(const AuPoint& vMove);

	void Point(float headTilt);

	void ProneCreep(float hipHt, bool bendToHoldObject, float speedEnc);

	void PsionicWave(float headTilt);

	void Pummel(float headTilt);

	void DeadFacingDown();

	void DeadFacingLeft();

	void DeadFacingRight();

	void DeadFacingUp();

	void DeadWingFacingDown();

	void DeadWingFacingLeft();

	void DeadWingFacingRight();

	void DeadWingFacingUp();

	void ReceptiveFacingUp();

	void Relax(float hipHt, bool bendToHoldObject);

	virtual void RenderHitBox(AuGLEffects& re);

	void RotateElbow(float angle);		// Rotates around X axis

	void RotateShoulder(float angle);	// YZ plane rotation

	void RotateWrist(float angle);		// Rotates around X axis

	virtual void RoughExtentLocs(AuPointArray& locs) const;

	void Run(float hipHt, bool bendToHoldObject, float speedEnc);

	void SetSpread(float liftAngle, float splayAngle)
	{
		m_Lift = liftAngle;
		m_Splay = splayAngle;
	}

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	void StandWing();

	void Straddle(float hipHt, bool bendToHoldObject);

	void Swim(const AuPose& poseRhythm, bool bendToHoldObject);

	void Throw(float headTilt);

	void TreadWater(bool bendToHoldObject);

	virtual float Volume() const;		// Approximate volume in cubic feet.

	void Walk(float hipHt, bool bendToHoldObject, float speedEnc);

	void Wave(float headTilt);

	AuPoint WristLoc() const;			// Calculate loc relative to body origin.

	void WritheFacingDown();

	void WritheFacingLeft();

	void WritheFacingRight();

	void WritheFacingUp();

	void WritheWingFacingDown();

	void WritheWingFacingLeft();

	void WritheWingFacingRight();

	void WritheWingFacingUp();

protected:
	void CalcStandAngles(float hipHt, float& shoulderang, float& elbowbend,
			float& wristbend) const;
	void LiftShoulder(float angle);	// Rotate at shoulder around Y.
										// Positive moves elbow away from body.
	void SplayShoulder(float angle);	// Rotate at shoulder around Z.
										// Positive moves elbow away from body.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpLeftArmAndHand : public DpArmAndHand
{
public:
	DpLeftArmAndHand(DpBody* body, int which) : DpArmAndHand(body, which)
	{
	}

	virtual void Bones(DhDNA::BONES& upper, DhDNA::BONES& fore,
			DhDNA::BONES& hand) const;
	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpLeftArmAndHand(*this);
		bp->SetBody(body);
		return bp;
	}
	virtual bool IsLeft() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpRightArmAndHand : public DpArmAndHand
{
public:
	DpRightArmAndHand(DpBody* body, int which) : DpArmAndHand(body, which)
	{
	}

	virtual void Bones(DhDNA::BONES& upper, DhDNA::BONES& fore,
			DhDNA::BONES& hand) const;
	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpRightArmAndHand(*this);
		bp->SetBody(body);
		return bp;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpLegAndFoot : public DpBodyPart
{
protected:
	float	m_Splay;	// Rotation about Z axis when standing. See DpBody::SpreadLegs.
	int		m_Which;	// Which one among multiples possible (0..2)
	AuPoint	m_KneeLoc;
	AuPoint	m_AnkleLoc;

public:
	DpLegAndFoot(DpBody* body, int which) : DpBodyPart(body)
	{
		m_Splay = 0;
		m_Which = which;
	}

	DpLegAndFoot(const DpLegAndFoot &rhs);					// Copy constructor.

	virtual quint64 MemoryUsage() const;

	void OneTimeCalcs();
	AuPoint AnkleLoc() const;			// Calculate loc relative to body origin.
	virtual void Bones(DhDNA::BONES& thigh, DhDNA::BONES& calf,
			DhDNA::BONES& foot) const = 0;

	void Climb(float hipHt, float speedEnc);

	virtual AuPoint ConnectLoc() const;	// Hip loc relative to body origin.
	AuPoint CurrentAnkleLoc() const;		// Return ankle location * bone matrix.
	virtual AuPoint CurrentConnectLoc() const;	// Returns connect loc * bone.
	AuPoint CurrentKneeLoc() const;		// Return knee location * bone matrix.
	virtual void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);

	void Dig(float hipHt, float speedEnc);

	bool DoesTouchGroundBelowJoint(float hipHt, float thighlen, float calflen,
			float footlen) const;

	void Fold();

	bool IsFront() const				// true if this is one of the front legs.
	{
		return m_Which == 0;
	}

	virtual bool IsLeft() const
	{
		return false;
	}

	virtual bool IsLegAndFoot() const
	{
		return true;
	}

	void Jump(float hipHt, float speedEnc);

	void Kick(float hipHt);

	AuPoint KneeLoc() const;				// Calculate loc relative to body origin.
	virtual void MoveBy(const AuPoint& vMove);

	void DeadFacingDown();

	void DeadFacingLeft();

	void DeadFacingRight();

	void DeadFacingUp();

	void Prone();

	void ProneCreep(float speedEnc);

	void ReceptiveFacingUp();

	virtual void RenderHitBox(AuGLEffects& re);

	void RotateAnkle(float angle);		// YZ plane rotation

	void RotateHip(float angle);		// YZ plane rotation

	void RotateKnee(float angle);		// YZ plane rotation

	virtual void RoughExtentLocs(AuPointArray& locs) const;

	void Run(float hipHt, float speedEnc);

	void SetSpread(float splayAngle)
	{
		m_Splay = splayAngle;
	}

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	void Stand(float hipHt);

	void Stomp(float hipHt);

	void Straddle();

	void Swim(const AuPose& poseRhythm);

	void TreadWater(float hipHt);

	virtual float Volume() const;		// Approximate volume in cubic feet.

	void Walk(float hipHt, float speedEnc);

	void WritheFacingDown();

	void WritheFacingLeft();

	void WritheFacingRight();

	void WritheFacingUp();

protected:
	void CalcStandAngles(float hipHt, float& hipang, float& kneebend,
		float& anklebend) const;
	void SplayHip(float angle);		// Rotate at hip around Z.
										// Positive moves knee away from body.
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpLeftLegAndFoot : public DpLegAndFoot
{
public:
	DpLeftLegAndFoot(DpBody* body, int which) : DpLegAndFoot(body, which)
	{
	}

	virtual void Bones(DhDNA::BONES& thigh, DhDNA::BONES& calf,
			DhDNA::BONES& foot) const;
	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpLeftLegAndFoot(*this);
		bp->SetBody(body);
		return bp;
	}
	virtual bool IsLeft() const
	{
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpRightLegAndFoot : public DpLegAndFoot
{
public:
	DpRightLegAndFoot(DpBody* body, int which) : DpLegAndFoot(body, which)
	{
	}

	virtual void Bones(DhDNA::BONES& thigh, DhDNA::BONES& calf, DhDNA::BONES& foot) const;

	virtual DpBodyPart* Clone(DpBody* body) const
	{
		DpBodyPart* bp = new DpRightLegAndFoot(*this);
		bp->SetBody(body);
		return bp;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AUDATA_DLL DpBody : public QVector<DpBodyPart*>
{
public:
	// Each section of the body can be assigned a different AuPose.
	// Not all body sections exist in all DNA combinations.
	enum SECTION
	{
		PS_FrontLegs,		// The first pair of legs only.
		PS_LeftHeadNeck,	// THE head and neck, if there's only one.
		PS_LeftArm,			// All left arms, if more than one pair.
		PS_LowerTorso,		// Torso below the waist vertex. General body
							// position.
		PS_RearLegs,		// All but the first pair of legs, if more than one
							// pair.
		PS_RightArm,		// All right arms, if more than one pair.
		PS_RightHeadNeck,	// Secondary head and neck.
		PS_Tail,			// May be an abdomen.
		PS_UpperTorso,		// Torso above the waist vertex.
		PS_Wings,			// There is only ever one set of wings.
		PS_Count			// NOTE: LAST ENTRY FOR COUNT OF SECTIONS
	};

	// Value used to describe all possible torso segment combinations when building the 3D model.
	// Prefix is the node closest to the head. It may contain any of these combinations.
	//       neck, arms & neck, arms & torso, torso, legs & neck, legs & torso
	// Suffix is the node closest to the tail. It may contain any of these combinations.
	//       butt, arms & tail, legs & butt, arms & torso, torso, legs & tail, legs & torso, tail, arms & butt
	enum TORSO_SEGMENT
	{
		TS_None,
		TS_NeckToButt,
		TS_NeckToShouldersAndTail,
		TS_NeckToHipsAndButt,
		TS_NeckToShouldersAndTorso,
		TS_NeckToTorso,
		TS_NeckToHipsAndTail,
		TS_NeckToHipsAndTorso,
		TS_NeckToTail,
		TS_NeckToShouldersAndButt,
		TS_ShouldersAndNeckToButt,
		TS_ShouldersAndNeckToShouldersAndTail,
		TS_ShouldersAndNeckToHipsAndButt,
		TS_ShouldersAndNeckToShouldersAndTorso,
		TS_ShouldersAndNeckToTorso,
		TS_ShouldersAndNeckToHipsAndTail,
		TS_ShouldersAndNeckToHipsAndTorso,
		TS_ShouldersAndNeckToTail,
		TS_ShouldersAndNeckToShouldersAndButt,
		TS_ShouldersAndTorsoToButt,
		TS_ShouldersAndTorsoToShouldersAndTail,
		TS_ShouldersAndTorsoToHipsAndButt,
		TS_ShouldersAndTorsoToShouldersAndTorso,
		TS_ShouldersAndTorsoToTorso,
		TS_ShouldersAndTorsoToHipsAndTail,
		TS_ShouldersAndTorsoToHipsAndTorso,
		TS_ShouldersAndTorsoToTail,
		TS_ShouldersAndTorsoToShouldersAndButt,
		TS_TorsoToButt,
		TS_TorsoToShouldersAndTail,
		TS_TorsoToHipsAndButt,
		TS_TorsoToShouldersAndTorso,
		TS_TorsoToTorso,
		TS_TorsoToHipsAndTail,
		TS_TorsoToHipsAndTorso,
		TS_TorsoToTail,
		TS_TorsoToShouldersAndButt,
		TS_HipsAndNeckToButt,
		TS_HipsAndNeckToShouldersAndTail,
		TS_HipsAndNeckToHipsAndButt,
		TS_HipsAndNeckToShouldersAndTorso,
		TS_HipsAndNeckToTorso,
		TS_HipsAndNeckToHipsAndTail,
		TS_HipsAndNeckToHipsAndTorso,
		TS_HipsAndNeckToTail,
		TS_HipsAndNeckToShouldersAndButt,
		TS_HipsAndTorsoToButt,
		TS_HipsAndTorsoToShouldersAndTail,
		TS_HipsAndTorsoToHipsAndButt,
		TS_HipsAndTorsoToShouldersAndTorso,
		TS_HipsAndTorsoToTorso,
		TS_HipsAndTorsoToHipsAndTail,
		TS_HipsAndTorsoToHipsAndTorso,
		TS_HipsAndTorsoToTail,
		TS_HipsAndTorsoToShouldersAndButt,
	};

public:
	DpBody();
	DpBody(char sex, bool isLefty);
	DpBody(char sex, bool isLefty, const DhDNA& dna);
	DpBody(const DpBody& c);
	~DpBody();

public:
	const DpBody& operator=(const DpBody& c);

	int AbilityPointsUsed() const;

	// Returns true if 'pose' can be restored when positioning a live avatar at a new place. Used when restoring position in the
	// designer.
	// Returns true for general poses that do not harm a living avatar.
	// Returns false for such poses as consoles, dead, dying, berths, etc.
	static bool CanRestorePositionPose(const AuPose& pose);

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	// Return density in lbs/ft^3
	inline float Density() const
	{
		// Creatures that live in the water are essentially the same density as water, so they can swim up and down readily.
		// Minor variations in density to control bouyancy are outside our current scope.
		if(m_DNA.LivesInWater)
			return (float)DensityOfWater();

		// All other creatures like to float a bit.
		// Wikipedia says the body has a relative density of .98 to water.
		return (float)DensityOfWater() * .98f;
	}

	void OneTimeCalcs();

	QString GetBodyModelPrefix() const;

	QString GetHelmetModelPrefix() const;

	bool HasBodyPowerArmorOn() const;

	bool HasHelmetPowerArmorOn() const;

	// Returns true if the DNA variations of this body do not cost too many ability points for creation of a character.
	// Independent of current body pose.
	bool IsAbilityPointsValidForPerson() const;

	// Returns true if the eye height is valid for a person.
	// Person must be in their natural standing/crouching pose based on body carriage, or they must be in AuPose::ConsoleCommand.
	bool IsCameraHeightValidForPerson() const;

	// Returns true if the CG height is valid for a person.
	// Person must be in their natural standing/crouching pose based on body carriage.
	// n/a for person in console pose because CG height is prescribed by the console.
	bool IsCgHeightValidForPerson() const;

	// Returns true if the head height is valid for a person.
	// Person must be in their natural standing/crouching pose based on body carriage, or they must be in AuPose::ConsoleCommand.
	bool IsHeadHeightValidForPerson() const;

	// Returns true if the maximum hit points of this body are valid for a player character.
	// Independent of current body pose.
	bool IsHitPointsValidForPerson() const;

	// Returns true if person's body size works when standing or seated at a console.
	// Independent of current body pose.
	inline bool IsValidForPerson()
	{
		// Test factors that are independent of the body's pose.
		if(!IsAbilityPointsValidForPerson())
			return false;										// Body uses too many ability points.
		if(!IsHitPointsValidForPerson())
			return false;										// Body has too many hit points.

		// Test body in its normal standing pose, which is either AuPose::Crouch or AuPose::Stand, depending on body carriage.
		if(DNA().BodyCarriage == 0 || DNA().BodyCarriage == 1)
			Pose(AuPose::Crouch, false, 1);
		else
			Pose(AuPose::Stand, false, 1);
		if(!IsCameraHeightValidForPerson() ||
				!IsCgHeightValidForPerson() ||
				!IsHeadHeightValidForPerson())
			return false;

		// Test body when seated at a console.
		Pose(AuPose::ConsoleCommand, false, 1);
		if(!IsCameraHeightValidForPerson() ||
				!IsHeadHeightValidForPerson())
			return false;

		return true;
	}

	quint64 MemoryUsage() const;

	// Seat for ceiling turret is dropped below ceiling by this amount.
	inline static float SeatDrop()
	{
		return 2.5;
	}

	void SelectTextures(AuGLEffects &re);

	inline void SetClothesType(COMMODITYID clothes)
	{
		m_eBodyClothesType = clothes;
	}

	inline void SetHelmetType(COMMODITYID helmet)
	{
		m_eHelmetClothesType = helmet;
	}

	// 'bendDominantArm' is true to bend the dominant arm as if holding something, like a gun. Does not affect wings.
	void AnimatePose(bool bendDominantArm, float speedEnc);

	// Distance from body pivot point to food for eating.
	float BiteRange() const;

	inline AuMatrix4x4& Bone(unsigned int bone)
	{
		Q_ASSERT(bone < DhDNA::BONE_Max);
		return m_Bones[bone];
	}

	bool CanHover() const;

	// Clears this body of its body parts.
	void DeleteBodyParts();

	void ResetCache();

	void SetupSkinMaterial();

	// Number of segments in torso is m_DNA.TorsoVertebrae.
	// 'segmentIndex' is
	//   0 = neck end torso segment
	//   m_DNA.TorsoVertebrae-1 = tail end torso segment.
	// Return value indicates the structure of the torso segment, from neck end to tail end. e.g. TS_ShouldersAndNeckToGroin.
	TORSO_SEGMENT TorsoSegment(unsigned int segmentIndex) const;

	float CalfLength() const
	{
		return m_CalfLength;
	}

	float CalfThickness() const
	{
		return m_CalfThickness;
	}

	// Current eye location relative to the pivot point of my body.
	AuPoint CameraLocation() const;

	// 'toBottomOfFoot' is true to include the height of the foot when appropriate, otherwise the height returned is to the bone.
	float CGHeight(bool toBottomOfFoot) const;			// Current height to center of gravity, based on pose.

	// 'toBottomOfFoot' is true to include the height of the foot when appropriate, otherwise the height returned is to the bone.
	float CGHeightStanding(bool toBottomOfFoot) const;	// Height to center of gravity when standing.

	AuDNABase::CHROMATID Chromatid() const;

	void CreateDeviceObjects(AuGLEffects& re, AuGLModelCache& mc, COMMODITYID clothing, COMMODITYID helmet, float speedEnc);

	void CreateSkin(AuGLEffects& re, AuGLModelCache& mc);		// Creates 3ds models of skin chunks.

	AuPose CurrentBonePose(DhDNA::BONES bone) const;

	inline const AuPose& CurrentGeneralPose() const
	{
		return m_Pose[PS_LowerTorso];
	}

	// Position returned is relative to the body waist/pivot point.
	AuPoint CurrentLeftHandLoc() const;		// For positioning carried item.

	inline float CurrentHeadTilt() const
	{
		return m_HeadTilt;
	}

	inline float CurrentHeadTurn() const
	{
		return m_HeadTurn;
	}

	inline const AuPose* CurrentPoses() const
	{
		return m_Pose;
	}

	inline short int CurrentHitPoints() const
	{
		return m_HitPoints;
	}

	inline static int DeadHitPoints()
	{
		// At zero, the body starts dying.
		// At this number of hit points, the body is dead.
		return -30;
	}

	// Current head location relative to the pivot point of my body.
	AuPoint HeadLocation() const;

	inline bool IsBerthPose() const
	{
		return IsBerthPose(CurrentGeneralPose());
	}

	inline static bool IsBerthPose(const AuPose& pose)
	{
		return pose.IsBerth();
	}

	inline bool IsGameTablePose() const
	{
		return CurrentGeneralPose().IsGameTable();
	}

	inline bool IsPatientPronePose() const
	{
		return CurrentGeneralPose().IsPatientProne();
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	// stationary version of a pose
	inline AuPose StationaryPose(const AuPose& pose) const
	{
		return StationaryPose(pose, m_DNA);
	}

	// stationary version of a pose
	static AuPose StationaryPose(const AuPose& pose, const DhDNA& dna);

	// Position returned is relative to the body waist/pivot point.
	AuPoint CurrentRightHandLoc() const;		// For positioning carried item.

	void DeleteDeviceObjects(AuGLEffects& re);

	inline const DhDNA& DNA() const
	{
		return m_DNA;
	}

	float FootLength(bool bHp=false) const;

	float FootHeight() const;

	inline float ForeArmLength() const
	{
		return m_ForeArmLength;
	}

	inline float ForeArmThickness() const
	{
		return m_ForeArmThickness;
	}

	float HandLength(bool bHp=false) const;		// Length of hand.

	float HeadLength() const;		// Length of head.

	float HeadThickness() const;	// Thickness of head (back to front)

	float HeadWidth() const;		// Width of head (left to right)

	void Init();					// Initialize all body parts.

	inline static void InitPoses(const AuPose& pose, AuPose poses[PS_Count])
	{
		for(int ps=0; ps<PS_Count; ps++)
			poses[ps] = pose;
	}

	inline bool IsBitingPose() const
	{
		if(m_Pose[PS_LeftHeadNeck].IsBiting() ||
				m_Pose[PS_RightHeadNeck].IsBiting())
			return true;
		return false;
	}

	// True if in a console-like pose that may or may not require a DpConsole.
	// Like IsConsolePose except no requirement for a DpConsole.
	inline bool IsLockedPose() const
	{
		return CurrentGeneralPose().IsLocked();
	}

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
		if(m_Pose[PS_LeftHeadNeck].IsEating() ||
				m_Pose[PS_RightHeadNeck].IsEating() ||
				m_Pose[PS_UpperTorso].IsEating())
			return true;
		return false;
	}

	inline bool IsFlyingPose() const
	{
		return CurrentGeneralPose().IsFlying();
	}

	inline bool IsLadderPose() const
	{
		return CurrentGeneralPose().IsLadder();
	}

	inline bool IsLeftHanded() const
	{
		return m_IsLefty;
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

	inline bool IsRightHanded() const
	{
		return !m_IsLefty;
	}

	inline bool IsSwimmingPose() const
	{
		return CurrentGeneralPose().IsSwimming();
	}

	// True if a troop in the current pose is manning a station.
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

	inline AuPoint JumpVector() const
	{
		return CurrentGeneralPose().JumpVector(DNA().JumpForce());
	}

	unsigned short int MaxHitPoints() const;

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
	float MovementSpeed(const AuPose& pose) const;

	// Returns the maximum speed this body can be moving to still be able to
	// initiate a jump. Slightly faster than run speed works well for lunar
	// bunny hopping.
	inline float MaxSpeedToJump() const
	{
		const float maxSpeed = 3.0f;
		return maxSpeed * ((float)m_DNA.Speed + 1);
	}

	// Movement rate based on DNA and pose.
	// Universe feet per real world second.
	inline float MovementSpeed() const
	{
		return MovementSpeed(CurrentGeneralPose());
	}

	inline bool NeedsInit() const
	{
		return m_Pose[0] == AuPose::Init;
	}

	inline bool NeedsSpawn() const
	{
		return m_Pose[0] == AuPose::Spawn;
	}

	inline float NoiseLevel() const	// 0..1
	{
		return CurrentGeneralPose().NoiseLevel();
	}

	// Current height needed based on pose to have room to move, from bottom of feet to highest point on body.
	// Does not consider horns or wings.
	float PhysicalHeight() const;

	// "Pose" functions return true if the pose value of any section is changed.
	inline bool Pose(const AuPose& pose, bool bendDominantArm, float speedEnc)
	{
		Q_ASSERT(pose.IsGeneral());
		AuPose poses[PS_Count];
		InitPoses(pose, poses);
		return Pose(poses, bendDominantArm, speedEnc);
	}

	bool Pose(const AuPose pose[PS_Count], bool bendDominantArm, float speedEnc);

	bool PoseAttackFlying();

	bool PoseAttackHovering();

	bool PoseAttackStanding();

	bool PoseAttackUnderGround();

	bool PoseAttackUnderWater();

	bool PoseBurrow(float speedEnc);

	bool PoseEatOffGround();

	void RenderHitBox(AuGLEffects& re);

	void RenderHitBoxChunk(AuGLEffects& re, DhDNA::BONES bone, const AuPoint& loc, float len, float avgrad) const;

	void RenderSightRay(AuGLEffects& re);

	// Render body centered on the body waist/pivot point.
	void RenderSkin(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
			bool headless);

	void RenderSkinGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
			const AuPoint& globeP, bool headless);

	void RenderSkinRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& ambientLightUp, float shadowPanelSunAngles[MAX_SUNS], bool headless);

	void RenderSkinFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
			bool headless);

	void RenderSkinFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
			const AuPoint& globeP, bool headless);

	void RenderSkinFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			const AuPoint& ambientLightUp, float shadowPanelSunAngles[MAX_SUNS], bool headless);

	inline void ResetHitPoints()
	{
		m_HitPoints = MaxHitPoints();
	}

	void RoughExtents(AuPoint& min, AuPoint& max) const;

	void RoughExtentChunk(AuPointArray& locs, DhDNA::BONES bone, const AuPoint& loc, float len) const;

	void SetDNA(const DhDNA& dna, float speedEnc);

	inline void SetHitPoints(short int hitpoints)
	{
		// Prevent negative hit points below "dead" level.
		// 0...dead is the "dying" zone where the body loses one hit point per
		// second until "dead".
		if(hitpoints >= DeadHitPoints())
			m_HitPoints = hitpoints;
		else
			m_HitPoints = DeadHitPoints();
	}

	inline void SetLeftHanded(bool isLefty)
	{
		m_IsLefty = isLefty;
	}

	inline void SetSex(char sex)
	{
		m_Sex = sex;
	}

	inline quint8 Sex() const
	{
		return m_Sex;
	}

	bool SightChunk(DhDNA::BONES bone, const AuPoint& start, const AuPoint& end,
			float avgrad, const AuPoint& eyeLoc, AuPoint& sightedLoc) const;

	// 'sightedPart' will be the BONE_ bone that was sighted.
	bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	// Sighting when in eyes-only pose, such as in the cockpit of a fighter.
	// 'sightedPart' will be the BONE_ bone that was sighted.
	bool SightTargetDetailsHead(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	// Overall scaling factor for skeleton. Not zero.
	// Nominal human size 8 body returns .7181 resulting in camera height at 5' 6".
	float SizeFactor() const;

	inline bool SpecialWeaponReady() const
	{
		return m_LastSpecUse.IsNull() || m_LastSpecUse.ElapsedSec() >= 10;
	}

	inline const AuTime& SpecialWeaponUseTime() const
	{
		return m_LastSpecUse;
	}

	// Returns maximum stamina of this body.
	// Range is 15 to 255.
	// A person with an endurance of 7 has 127 stamina points.
	inline quint8 Stamina() const
	{
		return DNA().Endurance * 16 + 15;
	}

	// To be "Starving" you must have no satiation
	inline unsigned short int StarvationThreshold() const
	{
		return 0;
	}

	// Returns the location that is best for targeting this body with a weapon, relative to the body pivot point.
	AuPoint TargetLocation() const;

	// Return scaling factor for model pieces at the body size of this creature.
	float OverallScale() const;

	// Heads depend on upper torso position.
	void PositionHeads();

	// Returns the amount of stamina needed by this body to jump.
	inline quint8 StaminaToJump() const
	{
		if(DNA().CanJump())
		{
			// Jumpers use less stamina to jump.
			return Stamina() / 15;
		}
		else
		{
			// Non-jumpers use a lot of stamina to jump.
			return Stamina() / 3;
		}
	}

	// Returns stamina trend based on body pose in stamina points per second.
	// Positive represents a rejuvenation of stamina.
	// Negative represents a drain of stamina.
	int StaminaTrend() const;

	inline float ThighLength() const
	{
		return m_ThighLength;
	}

	inline float ThighThickness() const
	{
		return m_ThighThickness;
	}

	// Heads tilt at base of each head. 0 = level
	void TiltHeadsTo(float tiltAngle, float turnAngle);

	// Length of torso from neck attach point to waist.
	// Returns zero if there is no torso.
	inline float TorsoLength() const
	{
		return m_TorsoLength;
	}

	inline float TorsoSegmentLength() const	// Length of a body torso segment, even if none.
	{
		return m_TorsoSegmentLength;
	}

	inline float TorsoThickness() const	// Thickness (chest to back if upright) of torso, even if none.
	{
		return m_TorsoThickness;
	}

	inline float TorsoWidth() const		// Width (left to right) of torso, even if none.
	{
		return m_TorsoWidth;
	}

	float TurnRate() const;

	inline float UpperArmLength() const
	{
		return m_UpperArmLength;
	}

	inline float UpperArmThickness() const
	{
		return m_UpperArmThickness;
	}

	inline void UseSpecialWeapon(const AuTime& when)
	{
		m_LastSpecUse = when;
	}

	float Volume() const;			// Approximate volume of body in cubic feet.

	// Returns true if the info describes the special weapon.
	// 'targetRange' is distance to the target from my body pivot point.
	// 'range' returned is range of body weapon relative to my body pivot point.
	bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon,
		bool okToUseSpecialWeapon=true) const;

	// 'range' returned is range of body weapon relative to my body pivot point.
	void WeaponAtPos(AuDNABase::WEAPONLOC loc, bool useSpecialWeapon, AuCombatWeapon& weapon) const;

private:
	void BeckonArms(int which);	// 0=left, 1=right, 2=both

	void BendNecks(float angle);	// Bends all necks around X at each vertex.

	void BlockArms(int which);	// 0=left, 1=right

	void ClimbArms(int which, bool bendFirstArm, float speedEnc);	// 0=left, 1=right

	void ClimbLegs(float hipHt, float speedEnc);

	AuPoint CurrentWaistLoc() const;	// Return waist location * bone matrix.

	void DeadArmsFacingDown(int which);
	void DeadArmsFacingLeft(int which);
	void DeadArmsFacingRight(int which);
	void DeadArmsFacingUp(int which);

	void DeadLegsFacingDown(int which);
	void DeadLegsFacingLeft(int which);
	void DeadLegsFacingRight(int which);
	void DeadLegsFacingUp(int which);

	void DeadTailFacingDown();
	void DeadTailFacingLeft();
	void DeadTailFacingRight();
	void DeadTailFacingUp();

	void DeadWingsFacingDown(float waistRot);
	void DeadWingsFacingLeft(float waistRot);
	void DeadWingsFacingRight(float waistRot);
	void DeadWingsFacingUp(float waistRot);

	void DigArms(int which);

	void DigLegs(float hipHt, float speedEnc);

	void EatWithArms(float hipHt, int which);	// Feed myself.

	void FlyWings(float waistRot, bool fast);

	void FoldArms(int which, bool bendFirstArm);

	void FoldLegs(int which);

	void FoldTail();

	void GlideWings(float waistRot);

	/*
	void HangTail();				// Hang tail with gravity.
	bool HasWings() const;			// True if this skeleton actually has wings.
	*/
	void InitAllBoneMatrices();		// Initialize all bone matrices to identity.

	void InitBoneMatrices();		// Initialize bone matrices to identity.

	void JumpArms(float hipHt, int which, bool bendFirstArm, float speedEnc); // which 0=left, 1=right, 2=both

	void JumpLegs(float hipHt, int which, float speedEnc); // which 0=back, 1=front, 2=both

	void KickLegs(float hipHt, int which);
									// which 0=back, 1=front, 2=both

	// Stationary clinging to ladder pose.
	void LadderArms(int which, bool bendFirstArm);		// 0=left, 1=right

	void LashTail(float hipHt, float speedEnc);

	void PointArms(int which);

	void PositionArms(float hipHt, float speedEnc);

	void PositionLegs(float hipHt, float speedEnc);

	void PositionTail(float hipHt, float speedEnc);

	float PositionTorso(float hipHt, float speedEnc);	// Returns waist rot

	void PositionWings(float waistRot);

	void ProneArms(int which, bool bendFirstArm);

	void ProneCreepArms(float hipHt, int which, bool bendFirstArm, float speedEnc);

	void ProneCreepLegs(int which, float speedEnc);

	void ProneLegs(int which);

	void PsionicArms(int which);	// which 0=left, 1=right, 2=both

	void ReceptiveArmsFacingUp(int which);	// which 0=left, 1=right, 2=both

	void ReceptiveLegsFacingUp(int which);

	void ReceptiveTailFacingUp();

	void RelaxTail(float hipHt);

	bool RotateWaist(float angle);	// Rotates body, head and arms around X at
									// waist.

	void RunArms(float hipHt, int which, bool bendFirstArm, float speedEnc);

	void RunLegs(float hipHt, int which, float speedEnc);
									// which 0=back, 1=front, 2=both

	void SlitherTail(const AuPose& tailPose, float speedEnc);

	void SpreadArms(float splayAng, float liftAng, int which);	// Sets spread
									// angles of arms. Handles multiple limbs at node.
									// which 0=left, 1=right, 2=both

	void SpreadLegs(float splayAng, int which);	// Sets spread angles
									// of legs. Handles multiple limbs at node.
									// which 0=back, 1=front, 2=all

	void StandArms(float hipHt, int which, bool bendFirstArm);	// Position arms for standing.
									// which 0=left, 1=right, 2=both

	float StandingArmWaistRotation(float hipHt) const;	// Returns standing waist rot
									// based on arm geometry and dna.

	void StandLegs(float hipHt, int which);	// Position legs for standing.

	void StandWings(float waistRot);	// Position wings for standing.

	void StompLegs(float hipHt, int which);

	void StraddleArms(float hipHt, int which, bool bendFirstArm);		// Arms only.

	void StraddleLegs();		// All legs only.

	void StrikeArms(int which);	// which 0=left, 1=right, 2=both

	void StrikeTail(float hipHt);

	void SwimArms(int which, bool bendFirstArm);

	void SwimLegs(int which);

	void SwimTail();

	void ThrowArms(int which);

	void TreadWaterArms(int which, bool bendFirstArm);

	void TreadWaterLegs(float hipHt, int which);

	void WagTail(float hipHt);

	void WalkArms(float hipHt, int which, bool bendFirstArm, float speedEnc);

	void WalkLegs(float hipHt, int which, float speedEnc);

	void WaveArms(int which);	// which 0=left,
														// 1=right, 2=both

	void WaveTail(float hipHt, float speedEnc);

	void WritheArmsFacingDown(int which);
	void WritheArmsFacingLeft(int which);
	void WritheArmsFacingRight(int which);
	void WritheArmsFacingUp(int which);

	void WritheLegsFacingDown(int which);
	void WritheLegsFacingLeft(int which);
	void WritheLegsFacingRight(int which);
	void WritheLegsFacingUp(int which);

	void WritheTailFacingDown();
	void WritheTailFacingLeft();
	void WritheTailFacingRight();
	void WritheTailFacingUp();

	void WritheWingsFacingDown(float waistRot);
	void WritheWingsFacingLeft(float waistRot);
	void WritheWingsFacingRight(float waistRot);
	void WritheWingsFacingUp(float waistRot);

private:
	mutable float	m_BiteRange;	// Body bite range cached to save work.
	DhDNA			m_DNA;			// Body needs the DNA.
	float			m_HeadTilt;		// Head tilt angle. 0.0 = none = straight ahead.
	float			m_HeadTurn;		// Head turn angle. 0.0 = none = straight ahead.
	short int		m_HitPoints;	// Hit points remaining for this creature.
	bool			m_IsLefty;		// true = left-handed, false = right-handed
	AuPose			m_Pose[PS_Count];	// Current position of each body section
	char 			m_Sex;			// 1 if male, 0 if female
	mutable float	m_Volume;		// Body volume cached to save work.

	// Remaining members are not serialized.
	bool			m_BendArm;		// true to bend the dominant arm
	AuMatrix4x4		m_Bones[DhDNA::BONE_Max];	// Bone matrices for vertex
									// skinning.
									// Calculated based on poses, not serialized
	int				m_glTexSkin[SkinTexLim];	// Skin texture
	int				m_glTexWings;	// Wings texture
	int				m_glTexClothes[SkinTexLim];	// Clothes texture
	int				m_glTexArmor[ArmorTexLim];	// Body Armor texture
	int				m_glTexHelmet[HelmetTexLim][ArmorTexLim];	// Helmet texture
	AuTime			m_LastSpecUse;	// Time special attack was last used.

public:
	AuPoint				m_Min, m_Max;	// 3D extents of the 3ds model.
	QVector<Au3dsGLSLModelInstance*>		m_Models;
	AuPoint			m_eyePos;
	COMMODITYID		m_eBodyClothesType;
	COMMODITYID		m_eHelmetClothesType;
	GLfloat			m_skinOffset[4];

	// Keep some calculated values that never change
	float			m_CalfLength;
	float			m_CalfThickness;
	float			m_ForeArmLength;
	float			m_ForeArmThickness;
	float			m_HeadLength;
	float			m_HeadThickness;
	float			m_HeadWidth;
	float			m_ThighLength;
	float			m_ThighThickness;
	float			m_TorsoLength;
	float			m_TorsoSegmentLength;
	float			m_TorsoThickness;
	float			m_TorsoWidth;
	float			m_UpperArmLength;
	float			m_UpperArmThickness;

public:
	QColor			m_TopColor[2];			// Two types, one for skin and one for clothes
	float			m_TopShininess[2];		// Two types, one for skin and one for clothes
	QColor			m_BottomColor[2];		// Two types, one for skin and one for clothes
	float			m_BottomShininess[2];	// Two types, one for skin and one for clothes
	float			m_Glow[2];				// Two types, one for skin and one for clothes
	float			m_Glowdim[2];			// Two types, one for skin and one for clothes
	double			m_uTOffset;
	double			m_vTOffset;
	double			m_uBOffset;
	double			m_vBOffset;
	bool			m_bDisplayHelmet;
};

#endif // DpBody_H
