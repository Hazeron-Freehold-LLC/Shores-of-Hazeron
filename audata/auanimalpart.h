//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auanimalpart.h,v 1.43 2022/05/09 16:10:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAnimalPart_H
#define AuAnimalPart_H

#include "audna.h"

class AuAnimalia;
class AuClothing;
class AuCombatWeapon;
class AuTime;

// Base class used to represent individual body parts.
class AuAnimalPart
{
public:
	// General body pose affects most of the positioning of a body's sections.
	// Individual sections can be posed differently. E.g. generally standing, left arm blocking, right arm striking, tail wagging.
	// Not all body sections exist in all DNA combinations.
	enum SECTION
	{
		PS_General,			// General overall body pose.
		PS_ArmLeft,			// All left arms, if more than one pair. Might not have any arms.
		PS_ArmRight,		// All right arms, if more than one pair. Might not have any arms.
		PS_HeadNeck,		// Head and neck. There is always a head but not always a neck.
		PS_LegsFront,		// The first pair of legs only, if any.
		PS_LegsRear,		// All but the first pair of legs, if more than one pair. Might not have any legs.
		PS_Tail,			// Could be an abdomen, which is a form of tail. Might not have a tail.
		PS_Torso,			// Torso above the waist, if any.
		PS_Wings,			// All wings, if any.
		PS_Mouth,
		PS_Count			// LAST ENTRY
	};

protected:
	QList<AuAnimalBone>					m_Bones;
	AuMeatMesh							m_ClothedMesh;			// Mesh for generating clothing.
	quint8								m_FirstBone;			// Index of first bone.
	AuMeatMesh							m_SkinMesh;				// Mesh of my naked skin.

public:	// Constructors
	inline AuAnimalPart()
	{
		m_FirstBone = 0;
	}

	inline AuAnimalPart(quint8 firstBone)
	{
		m_FirstBone = firstBone;
	}

	virtual ~AuAnimalPart()
	{
	}

	AuAnimalPart(const AuAnimalPart& c) : m_Bones(c.m_Bones), m_ClothedMesh(c.m_ClothedMesh), m_SkinMesh(c.m_SkinMesh)
	{
		m_FirstBone = c.m_FirstBone;
	}

public:	// Operators

public:	// Methods
	// 'id' is used to offset things so all creatures don't move in sync.
	// Returns true when a do-once pose has completed its cycle.
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
	{
		Q_UNUSED(id);
		Q_UNUSED(when);
		Q_UNUSED(body);
		return false;
	}

	inline const QList<AuAnimalBone>& Bones() const
	{
		return m_Bones;
	}

	virtual AuAnimalPart* Clone() const = 0;

	// Expects the bones to already be generated.
	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage) = 0;
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes,
			GROWTHSTAGE stage) = 0;

	inline void GetClothedMesh(AuMeatMesh& clothedMesh)
	{
		clothedMesh.Append(m_ClothedMesh);
	}

	inline void GetSkinMesh(AuMeatMesh& skinMesh)
	{
		skinMesh.Append(m_SkinMesh);
	}

	inline bool HasClothedMesh() const
	{
		return !m_ClothedMesh.isEmpty();
	}

	inline bool HasSkinMesh() const
	{
		return !m_SkinMesh.isEmpty();
	}

	virtual SECTION PoseSection() const = 0;

	// Sight extents does not depend on the skin mesh.
	virtual void SightExtents(AuPoint& min, AuPoint& max) const
	{
		Q_UNUSED(min);
		Q_UNUSED(max);
	}

	// Tests for intersection by segment from 'eyeLoc' to 'sightedLoc'.
	// Returns false if not hit.
	// Returns true if hit and...
	// - Adjusts 'sightedLoc' to the hit location when hit, presumably closer to 'eyeLoc'.
	// - Sets 'sightedPart' to (AuChromoWhich << 8) | AuAnimalPart::SECTION;
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
	{
		Q_UNUSED(eyeLoc);
		Q_UNUSED(sightedLoc);
		Q_UNUSED(sightedPart);
		return false;
	}

	// Returns the extents of the skin mesh.
	inline void SkinExtents(AuPoint& min, AuPoint& max) const
	{
		m_SkinMesh.Extents(min, max);
	}

	// Returns the volume of this part in cubic feet.
	// Beware that the volume of the entire body is not the sum of the volumes of its AuAnimalParts. Things like ears, noses,
	// mouths and mammaries add to body volume.
	virtual float Volume() const
	{
		return 0;
	}

	virtual bool Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(loc);
		Q_UNUSED(weapon);
		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalArm : public AuAnimalPart
{
protected:
	AuChromoArm*							m_Chromosome;
	float								m_Lift;					// Rotation about Y axis when standing.
	float								m_Splay;				// Rotation about Z axis when standing.

public:	// Constructors
	AuAnimalArm(const AuChromoArm& armDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoArm*)armDNA.Clone();
		m_Lift = 0;
		m_Splay = 0;
		GenBones(dna, firstBone);
	}

	AuAnimalArm(const AuAnimalArm& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoArm*)c.m_Chromosome->Clone();
		m_Lift = c.m_Lift;
		m_Splay = c.m_Splay;
	}

	virtual ~AuAnimalArm()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalArm(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	virtual SECTION PoseSection() const
	{
		if(m_Chromosome->Laterality() == AuChromosome::Left)
			return PS_ArmLeft;
		else
			return PS_ArmRight;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual float Volume() const;

	virtual bool Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const;

	inline AuChromoWhich Which() const
	{
		return m_Chromosome->Which();
	}

protected:
	// Returns joint angles for the current pose.
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
			float& shoulderX, float& shoulderY, float& shoulderZ, float& elbowX, float& elbowZ, float& wristX, float& wristY,
			float& wristZ, float& fingerY, float& fingerTipY);

	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalFin : public AuAnimalPart
{
protected:
	AuChromoFin*							m_Chromosome;
	float								m_Lift;					// Rotation about Y axis when standing.
	float								m_Splay;				// Rotation about Z axis when standing.

public:	// Constructors
	AuAnimalFin(const AuChromoFin& finDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoFin*)finDNA.Clone();
		m_Lift = 0;
		m_Splay = 0;
		GenBones(dna, firstBone);
	}

	AuAnimalFin(const AuAnimalFin& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoFin*)c.m_Chromosome->Clone();
		m_Lift = c.m_Lift;
		m_Splay = c.m_Splay;
	}

	virtual ~AuAnimalFin()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalFin(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	virtual SECTION PoseSection() const
	{
		switch(m_Chromosome->Laterality())
		{
		case AuChromosome::Left:
			return PS_ArmLeft;
		case AuChromosome::Middle:
			break;
		case AuChromosome::Right:
			return PS_ArmRight;
		}
		return PS_General;
	}

protected:
	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalHead : public AuAnimalPart
{
protected:
	AuChromoHead*							m_Chromosome;

public:	// Constructors
	AuAnimalHead(const AuChromoHead& headDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoHead*)headDNA.Clone();
		GenBones(dna, firstBone);
	}

	AuAnimalHead(const AuAnimalHead& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoHead*)c.m_Chromosome->Clone();
	}

	virtual ~AuAnimalHead()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalHead(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	virtual SECTION PoseSection() const
	{
		return PS_HeadNeck;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual float Volume() const;

	virtual bool Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const;

protected:
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
			float& headX, float& headY, float& headZ, float& eyeLeftX, float& eyeLeftZ, float& eyeRightX, float& eyeRightZ,
			float& jawX, float& jawLeftY, float& jawRightY);

	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalLeg : public AuAnimalPart
{
protected:
	AuChromoLeg*						m_Chromosome;
	float								m_Splay;				// Rotation about Z axis when standing.

public:	// Constructors
	AuAnimalLeg(const AuChromoLeg& legDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoLeg*)legDNA.Clone();
		m_Splay = 0;
		GenBones(dna, firstBone);
	}

	AuAnimalLeg(const AuAnimalLeg& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoLeg*)c.m_Chromosome->Clone();
		m_Splay = c.m_Splay;
	}

	virtual ~AuAnimalLeg()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalLeg(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	float HipAltitudePosed(const AuChromatid& dna) const;

	virtual SECTION PoseSection() const
	{
		if(m_Chromosome->IsFront())
			return PS_LegsFront;
		else
			return PS_LegsRear;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual bool Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const;

	virtual float Volume() const;

protected:
	// Returns joint angles for the current pose.
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
			float& hipX, float& hipY, float& hipZ, float& kneeX, float& ankleX, float& toeX);

	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalTentacle : public AuAnimalPart
{
protected:
	AuChromoTentacle*						m_Chromosome;
	float								m_Lift;					// Rotation about Y axis when standing.
	float								m_Splay;				// Rotation about Z axis when standing.

public:	// Constructors
	AuAnimalTentacle(const AuChromoTentacle& tentacleDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoTentacle*)tentacleDNA.Clone();
		m_Lift = 0;
		m_Splay = 0;
		GenBones(dna, firstBone);
	}

	AuAnimalTentacle(const AuAnimalTentacle& c) : AuAnimalPart(c)
	{
		m_Lift = c.m_Lift;
		m_Splay = c.m_Splay;
	}

	virtual ~AuAnimalTentacle()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalTentacle(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	virtual SECTION PoseSection() const
	{
		if(m_Chromosome->Laterality() == AuChromosome::Left)
			return PS_ArmLeft;
		else
			return PS_ArmRight;
	}

protected:
	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalTorso : public AuAnimalPart
{
protected:
	AuChromoTorso*							m_Chromosome;

public:	// Constructors
	AuAnimalTorso(const AuChromoTorso& torsoDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoTorso*)torsoDNA.Clone();
		GenBones(dna, firstBone);
	}

	AuAnimalTorso(const AuAnimalTorso& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoTorso*)c.m_Chromosome->Clone();
	}

	virtual ~AuAnimalTorso()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	const AuMatrix4x4& BaseMatrix() const;

	// Thickness (front to back in Y) of unposed torso meat. Lower torso is preferred if any, otherwise upper torso.
	float BottomThickness() const;

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalTorso(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	bool HasNeck() const;

	bool HasTail() const;

	const AuMatrix4x4& HeadBaseMatrix() const;

	const AuMatrix4x4& NeckBaseMatrix() const;

	virtual SECTION PoseSection() const
	{
		if(m_Chromosome->Orientation() == AuChromoTorso::Vertical)
			return PS_Torso;
		else
			return PS_General;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const;

	virtual float Volume() const;

	virtual bool Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const;

	float Width() const;

protected:
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
			float& buttX, float& buttY, float& buttZ, float& chestX, float& chestY, float& chestZ, float& neckX, float& neckY,
			float& neckZ, float& tailX, float& tailY);

	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalWing : public AuAnimalPart
{
protected:
	AuChromoWing*						m_Chromosome;
	float								m_Lift;					// Rotation about Y axis when standing.
	float								m_Splay;				// Rotation about Z axis when standing.

public:	// Constructors
	AuAnimalWing(const AuChromoWing& wingDNA, const AuChromatid& dna, quint8& firstBone)
		: AuAnimalPart(firstBone)
	{
		m_Chromosome = (AuChromoWing*)wingDNA.Clone();
		m_Lift = 0;
		m_Splay = 0;
		GenBones(dna, firstBone);
	}

	AuAnimalWing(const AuAnimalWing& c) : AuAnimalPart(c)
	{
		m_Chromosome = (AuChromoWing*)c.m_Chromosome->Clone();
		m_Lift = c.m_Lift;
		m_Splay = c.m_Splay;
	}

	virtual ~AuAnimalWing()
	{
		delete m_Chromosome;
	}

public:	// Methods
	virtual bool AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body);

	bool CanFly() const;

	virtual AuAnimalPart* Clone() const
	{
		return new AuAnimalWing(*this);
	}

	virtual void GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage);
	virtual void GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage);

	virtual SECTION PoseSection() const
	{
		return PS_Wings;
	}

protected:
	// 'boneIndex' on entry establishes my first bone index. I must increment it by the number of bones I need.
	void GenBones(const AuChromatid& dna, quint8& boneIndex);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuAnimalPart_H
