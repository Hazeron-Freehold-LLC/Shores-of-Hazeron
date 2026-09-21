//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dhdna.h,v 1.76 2022/02/19 18:19:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DhDNA_H
#define DhDNA_H

#include "audef.h"
#include "audnabase.h"
#include "augloballib.h"
#include "aumath.h"
#include "aupose.h"
#include <QMetaType>
#include <QString>

class Au3dsGLSLDNA;
class AuDice;
class QDataStream;

// This non-virtual version of DhDNA exists to serialize old data.
class DhDNA_
{
public:
	enum ARM				// Manipulation/locomotion
	{
		ARM_Limb,			// Human, most critters
		ARM_Tentacle,		// Octopus, squid
		ARM_WingButterfly,	// Butterfly
		ARM_WingDragonFly,	// DragonFly, mosquito
		ARM_WingFairy,		// Tinker Bell
		ARM_WingFeathered,	// Eagle
		ARM_WingLeathery,	// Bat, Pterodactyl
		ARM_WingMoth,		// Moth, wasp
	};
	enum BRAIN
	{
		BRAIN_Head,			// Brain is located in the head(s). Most critters.
		BRAIN_Torso,		// Brain is located in torso section(s). Earthworm.
	};
	enum EAR
	{
		EAR_Antennae,		// Insect
		EAR_Ear,			// Human, Elephant
		EAR_Fibers,			// Clusters of fibers sticking out of skull.
		EAR_Floppy,			// Cocker spaniel, Jarjar Binks.
		EAR_NotApparent,	// Bird
		EAR_Spot,			// Frog, fish, bird.
		EAR_Stalk,
		EAR_Tab,			// Cat, horse.
	};
	enum EYE
	{
		EYE_ClusterSimple,	// Spider
		EYE_ClusterStalk,	// Beholder
		EYE_Complex,		// Human, squid. Detailed with cornea, iris, pupil.
		EYE_Facet,			// Like insect eye.
		EYE_NotApparent,	// Cousin It, Shai Hulud
		EYE_Simple,			// Like spider or fish or horse. Undetailed dot.
		EYE_Slit,			// Alien, sloth, Jabba the Hut
		EYE_Stalk,			// Snail, slug, lobster
	};
	enum FOOT
	{
		FOOT_Claw,			// Bird
		FOOT_Flipper,		// Duck, platypus
		FOOT_Foot,			// Human
		FOOT_Hand,			// Dual use. Monkey
		FOOT_Hoof,			// Horse, cow
		FOOT_Paw,			// Dog, rabbit
		FOOT_Pincher,		// Spider, ant, lobster feet (not claws)
		FOOT_Sucker,		// Octopus
	};
	enum HAND
	{
		HAND_Claw,			// Bird
		HAND_Flipper,		// Whale, walrus, seal, fish
		HAND_Foot,			// Dual use. Raccoon
		HAND_Hand,			// Human
		HAND_Hoof,			// Horse, cow
		HAND_Paw,			// Dog, bear
		HAND_Pincher,		// Crab Claw as a hand
		HAND_Sucker,		// Octopus
	};
	enum LEG				// Locomotion
	{
		LEG_Cilia,			// Starfish
		LEG_Limb,			// Human, most critters
		LEG_Pseudopod,		// Amoeba, dralasite, starfish
		LEG_Tentacle,		// Octopus, squid
	};
	enum MOUTH
	{
		MOUTH_Beak,			// Eagle
		MOUTH_Cilia,		// Have Spacesuit, Will Travel
		MOUTH_Fangs,		// Lion
		MOUTH_Mandible,		// Predator, ant, spider
		MOUTH_Mouth,		// Salamander, fish
		MOUTH_NotApparent,	// Amoeba, slug
		MOUTH_Teeth,		// Human
		MOUTH_Tusks,		// Elephant
	};
	enum NOSE
	{
		NOSE_Cilia,
		NOSE_Holes,			// Monkey, duck, fish
		NOSE_Nose,			// Human
		NOSE_NotApparent,	// Ant, spider
		NOSE_Proboscis,		// Mosquito
		NOSE_Snorkel,		// Amphibious dinosaur
		NOSE_Snout,			// Horse, dog, cat
		NOSE_Trunk,			// Elephant
	};
	enum SKIN				// Appearance varies with 8 subtypes for each type. This sort of defines the phylum, in our world.
	{						// A top and bottom subtype define appearance.
		// Skins beyond 0..7 are used for indices into the skin texture palette.
		SKIN_DryExoskeleton,			// Bug: Ant.
		SKIN_DryScales,					// Lizard: Lizard.
		SKIN_DrySkin,					// Pig: Pig, human.
		SKIN_Feathers,					// Bird: Sparrow, ostrich, chicken.
		SKIN_Fur,						// Furry: Covered with fur. Beaver, horse.
		SKIN_WetExoskeleton,			// Crab: Black widow, millipede.
		SKIN_WetScales,					// Fish: Shark, cat fish.
		SKIN_WetSkin,					// Frog: Salamander, kardassian
	};
	enum TAIL
	{
		// Tail type is determined by this value.
		// A tail of some sort may be present whether or not there are any tail
		// vertebrae (TailVertebrae).
		// The TailType value determines the tip treatment added to the end of
		// the tail vertebrae skinned portion.
		// The SkinType determines what these mean so look at the code to learn
		// more.
		TAIL_None,
		TAIL_Primary,
		TAIL_Secondary,
		TAIL_Tertiary,
	};

public:
	// These bit fields are arranged to insure they break nicely on byte
	// boundaries.
	// Sizes of flesh on bones are determined as follows:
	//  'Lengths' are increased by 0.5 then multiplied by SizeFactor()
	//  'Thickness' is a function of the corresponding length value.
	//    Thicknesses range in value from 0 to 3 which equals 25%, 50%, 75% and
	//    100% of 1/2 of the corresponding length. e.g. OverallSize 3,
	//    ForeArmLength 4, ForeArmThickness 1 results in length 3*4.5/2=6.75
	//    and forearm thickness of (6.75 * 1/2)*50%=1.69
	//    In other words, thickness is directly proportional to the radius of
	//    the limb.
	//    TorsoThickness is an exception, where the % is multiplied by 3/4 of
	//    the TorsoSegmentLength rather than 1/2.
	unsigned int	Agility : 4;				// 0..15
	unsigned int	Endurance : 4;				// 0..15

	unsigned int	ArmPairs : 2;				// 0..3
	unsigned int	BodyCarriage : 2;			// 0..3
	unsigned int	WeaponType : 4;				// 0..15 WEAPONTYPE

	unsigned int	ArmType : 3;				// 0..7 ARM
	unsigned int	CalfLength : 5;				// 0..31

	unsigned int	BottomSkinSubtype : 3;		// 0..7
	unsigned int	CalfThickness : 5;			// 0..31

	unsigned int	BottomSkinThickness : 2;	// 0..3
	unsigned int	CrestSubtype : 2;			// 0..3
	unsigned int	EarSize : 2;				// 0..3
	unsigned int	EcologicalRole : 2;			// 0..3 ECOROLE

	unsigned int	BrainLocation : 1;			// 0..1 BRAIN
	unsigned int	Heads : 1;					// 0..1 Interpreted as 1..2
	unsigned int	LivesInAir : 1;				// 0..1
	unsigned int	LivesInWater : 1;			// 0..1
	unsigned int	LivesOnLand : 1;			// 0..1
	unsigned int	LivesUnderGround : 1;		// 0..1
	unsigned int	WeaponAltLocation : 1;		// 0..1 0=Head, 1=Mode dependent alt loc
	unsigned int    UNUSED0 : 1;				// 0..1 UNUSED BIT HERE IF YOU NEED ONE

	unsigned int	EarType : 3;				// 0..7 EAR
	unsigned int	ForeArmLength : 5;			// 0..31

	unsigned int	EcologicalStrategy : 2;		// 0..3 ECOSTRATEGY
	unsigned int	EyeSize : 2;				// 0..3
	unsigned int	HeadCrestSize : 2;			// 0..3. On skin, 1=human hair.
	unsigned int	HornBranches : 2;			// 0..3. 0=unicorn, 1=bull

	unsigned int	EyeType : 3;				// 0..7 EYE
	unsigned int	ForeArmThickness : 5;		// 0..31

	unsigned int	FightReflex : 4;			// 0..15
	unsigned int	FlightReflex : 4;			// 0..15

	unsigned int	FootLength : 5;				// 0..31
	unsigned int	FootType : 3;				// 0..7 FOOT

	unsigned int	HandLength : 5;				// 0..31
	unsigned int	HandType : 3;				// 0..7 HAND also wing subtype/texture/tip sz

	unsigned int	HornSize : 2;				// 0..3
	unsigned int	LegPairs : 2;				// 0..3
	unsigned int	LegType : 2;				// 0..3 LEG
	unsigned int	MouthSize : 2;				// 0..3

	unsigned int	MouthType : 3;				// 0..7 MOUTH
	unsigned int	OverallSize : 5;			// 0..31 Length multiplier

	unsigned int	NeckVertebrae : 2;			// 0..3 (Actual is NeckVertebrae+Heads)
	unsigned int	NoseSize : 2;				// 0..3
	unsigned int	TailType : 2;				// 0..3 TAIL
	unsigned int	TailVertebrae : 2;			// 0..3

	unsigned int	NoseType : 3;				// 0..7 NOSE
	unsigned int	ThighLength : 5;			// 0..31

	unsigned int	SkinType : 3;				// 0..7 SKIN
	unsigned int	ThighThickness : 5;			// 0..31

	unsigned int	Speed : 4;					// 0..15
	unsigned int	Strength : 4;				// 0..15

	unsigned int	TimeActive : 2;				// 0..3 TOD
	unsigned int	TopSkinThickness : 2;		// 0..3
	unsigned int	TorsoCrestSize : 2;			// 0..3
	unsigned int	WeaponMode : 2;				// 0..3 DNAWEAPONMODE

	unsigned int	TopSkinSubtype : 3;			// 0..7
	unsigned int	TorsoSegmentLength : 5;		// 0..31

	unsigned int	TorsoThickness : 5;			// 0..31
	unsigned int	TorsoVertebrae : 3;			// 0..7

	unsigned int	UpperArmLength : 5;			// 0..31
	unsigned int	WeaponSize : 3;				// 0..7

	unsigned int	UpperArmThickness : 5;		// 0..31
	unsigned int	WingSubType : 3;			// 0..7

	unsigned int	UNUSED1 : 8;				// Fill out to the next 32bit boundary. Otherwise we get random data here.

public:	// Construction
	DhDNA_();
	DhDNA_(const QByteArray& ba);
	DhDNA_(const DhDNA_& c);

public:	// Operators
	const DhDNA_& operator=(const DhDNA_& c);
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);
	bool operator==(const DhDNA_& c) const;

public:	// Methods
	// 'sizemod' is a size modifier. This is usually calculated based on world size. The smallest possible habitable world has a
	// sizemod of 6, increasing the size of its creatures. The largest possible world has a sizemod of -4, decreasing the size of
	// its creatures.
	// 'canHaveWings' is true if the creature can have wings as an arm type. This is false when atmosphere density is less than 50%.
	// 'tod' is the time-of-day this creature is normally active.
	// 'canChangePhylum' is true if the SkinType can be changed.
	void Randomize(int sizemod, bool canHaveWings, TOD tod, const AuDice& dice, bool canChangePhylum);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Physical construction of a critter.
class DhDNA : public AuDNABase, public DhDNA_
{
public:
	// All possible bones resulting from DNA.
	// Lines of bones that form limbs are numerically continuous. They can be
	// reliably iterated by incrementing/decrementing the bone enumeration
	// value.
	// There is a resource limitation in the vertex skinning shader that
	// imposes a limit on the total amount of data that can be passed as
	// uniform parameters. This number of bones comes very close to exceeding
	// that limit. If the limit is exceeded, the likely symptom will be nothing
	// on the screen at all.
	enum BONES
	{
		BONE_Torso0, BONE_Torso1, BONE_Torso2, BONE_Torso3, BONE_Torso4,
		BONE_Torso5, BONE_Torso6,
		BONE_Head0, BONE_Neck00, BONE_Neck01, BONE_Neck02, BONE_Neck03,
		BONE_Head1, BONE_Neck10, BONE_Neck11, BONE_Neck12, BONE_Neck13,
		BONE_Tail0, BONE_Tail1, BONE_Tail2, BONE_Tail3, BONE_Tail4, BONE_Tail5,
		BONE_Hand0L, BONE_ForeArm0L, BONE_UpperArm0L,
		BONE_Hand0R, BONE_ForeArm0R, BONE_UpperArm0R,
		BONE_Hand1L, BONE_ForeArm1L, BONE_UpperArm1L,
		BONE_Hand1R, BONE_ForeArm1R, BONE_UpperArm1R,
		BONE_Hand2L, BONE_ForeArm2L, BONE_UpperArm2L,
		BONE_Hand2R, BONE_ForeArm2R, BONE_UpperArm2R,
		BONE_Foot0L, BONE_Calf0L, BONE_Thigh0L,
		BONE_Foot0R, BONE_Calf0R, BONE_Thigh0R,
		BONE_Foot1L, BONE_Calf1L, BONE_Thigh1L,
		BONE_Foot1R, BONE_Calf1R, BONE_Thigh1R,
		BONE_Foot2L, BONE_Calf2L, BONE_Thigh2L,
		BONE_Foot2R, BONE_Calf2R, BONE_Thigh2R,
		BONE_Jaw0,
		BONE_Jaw1,
		BONE_Max	// vsolarskin4m.cg shader limit is 61
	};

public:	// Construction
	DhDNA();

	DhDNA(const QByteArray& ba);

	DhDNA(const DhDNA_& dna);

	virtual ~DhDNA()
	{
	}

public:	// Operators
	const DhDNA& operator=(const DhDNA_& dna);

	virtual bool operator==(const AuDNABase& c) const;

	inline bool operator<(const DhDNA& c) const
	{
		return BitSum() < c.BitSum();
	}

	inline bool operator>(const DhDNA& c) const
	{
		return BitSum() > c.BitSum();
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	unsigned int ArmVertices() const;

	inline unsigned int BitSum() const
	{
		return Agility +
			ArmPairs +
			ArmType +
			BodyCarriage +
			BottomSkinSubtype +
			BottomSkinThickness +
			BrainLocation +
			CalfLength +
			CalfThickness +
			CrestSubtype  +
			EarSize +
			EarType +
			EcologicalRole +
			EcologicalStrategy +
			Endurance +
			EyeSize +
			EyeType +
			FightReflex +
			FlightReflex +
			FootLength +
			FootType +
			ForeArmLength +
			ForeArmThickness +
			HandLength +
			HandType +
			HeadCrestSize +
			Heads +
			HornBranches +
			HornSize +
			LegPairs +
			LegType +
			LivesInAir +
			LivesInWater +
			LivesOnLand +
			LivesUnderGround +
			MouthSize +
			MouthType +
			NeckVertebrae +
			NoseSize +
			NoseType +
			OverallSize +
			SkinType +
			Speed +
			Strength +
			TailType +
			TailVertebrae +
			ThighLength +
			ThighThickness +
			TimeActive +
			TopSkinSubtype +
			TopSkinThickness +
			TorsoCrestSize +
			TorsoSegmentLength +
			TorsoThickness +
			TorsoVertebrae +
			UpperArmLength +
			UpperArmThickness +
			WeaponAltLocation +
			WeaponMode +
			WeaponSize +
			WeaponType +
			WingSubType;
	}

	// Returns a bone name suitable for debugging.
	static const char* BoneName(BONES bone);

	inline bool CanFly() const
	{
		// Air dwellers with wings can really fly.
		return (LivesInAir && HasWings()) ? true : false;
	}
	inline bool CanJump() const
	{
		// Air dwellers without wings are considered to be jumpers/leapers.
		return (LivesInAir && !HasWings()) ? true : false;
	}

	virtual AuDNABase* Clone() const
	{
		return new DhDNA(*this);
	}

	AuDNABase* Combine(const AuDNABase& dna, const AuDice& dice) const;

	void CreateModelDNA(Au3dsGLSLDNA& dna) const;

	// Returns the number of seconds per standard body cycle, which is based its the Speed.
	inline float CycleSpeed() const
	{
		return 1.f + 4.f/((float)Speed+1.f);	// seconds per standard cycle
	}

	QString Description() const;

	void Dump() const;

	virtual ECOROLE EcoRole() const
	{
		return (ECOROLE)EcologicalRole;
	}

	inline ECOSTRATEGY EcoStrategy() const
	{
		return (ECOSTRATEGY)EcologicalStrategy;
	}

	// Field of view angle is 1/2 field of view. We only test half the angle off the center line of the eye vector.
	inline float FieldOfViewAngle() const
	{
		if(Heads > 0)
		{
			// Broaden field of view when two heads.
			switch(EcoRole())
			{
			case ECO_Carnivore:
				return (90.f + 15.f) * DEG2RADf / 2.f;
			case ECO_Herbivore:
				return (344.f + 15.f) * DEG2RADf / 2.f;
			case ECO_Omnivore:
			default:
				return (135.f + 15.f) * DEG2RADf / 2.f;
			case ECO_Scavenger:
				return (125.f + 15.f) * DEG2RADf / 2.f;
			}
		}
		else
		{
			switch(EcoRole())
			{
			case ECO_Carnivore:
				return 90.f * DEG2RADf / 2.f;
			case ECO_Herbivore:
				return 344.f * DEG2RADf / 2.f;
			case ECO_Omnivore:
			default:
				return 135.f * DEG2RADf / 2.f;
			case ECO_Scavenger:
				return 125.f * DEG2RADf / 2.f;
			}
		}
	}

	// Returns a length value in feet given an integer DNA length value from 0 to 31.
	static inline float Length(int length)
	{
		return ((float)length/16.f + .5f);
	}

	virtual bool FromSpecies(const AuSpecies& species);

	bool FromString(const QString& dnaString);

	virtual GENDER Gender(CHROMATID chroma) const
	{
		return (chroma == Alpha) ? Male : Female;
	}

	virtual int GestateBabyDays() const
	{
		return OverallSize + 1;
	}

	virtual int GestateImplantHours() const
	{
		return OverallSize + 1;
	}

	bool HasAnkleBend() const;

	inline bool HasArms() const
	{
		if(HasWings())
		{
			// Only first arm pair is wings, rest are arms.
			if(ArmPairs > 1)
				return true;
		}
		else
		{
			// No wings so any arm pairs will do.
			if(ArmPairs)
				return true;
		}
		return false;
	}

	virtual bool HasBone(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasBrain(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasGills(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasHeart(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasLeather(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasLungs(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasStomach(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	inline bool HasWings() const
	{
		if(!ArmPairs)
			return false;

		if(ArmType == ARM_Limb || ArmType == ARM_Tentacle)
			return false;

		// All other arm types are wings.
		return true;
	}

	bool HasWristBend() const;

	// answers.com says normal range for chatting is 20m, 100m for shouting, 10m for whispering.
	// DtChatChannel uses this range to determine audibility of messages. Messages can be heard but are not audible in the
	// last 20% of a creature's hearing range.
	float HearingRange() const;

	// Inits this DNA to the default human.
	void InitDefaultHuman();

	bool IsExoskeletal() const;

	virtual bool IsSexuallyCompatible(const AuDNABase& dna) const;

	// Returns true if this DNA could be vermin.
	inline bool LooksLikeVermin() const
	{
		if(OverallSize)
			return false;										// Vermin are only size 0.
		if(!LivesOnLand)
			return false;										// Vermin always live on land.
		if(EcologicalRole != ECO_Scavenger)
			return false;										// Vermin are always scavengers.
		return true;
	}

	// Returns true if this DNA appears to be NULL, at least for a player character DNA.
	inline bool LooksNull() const
	{
		return OverallSize == 0 && Speed == 0 && Agility == 0 && Strength == 0 && Endurance == 0;
	}

	inline unsigned int JumpForce() const
	{
		if(CanJump())
			return (Speed + 1) * 4 + 10;
		else
			return (Speed + 1) + 10;
	}

	unsigned int LegVertices() const;

	virtual int MinutesToMature() const
	{
		return OverallSize + 1;
	}

	inline unsigned int NeckVertebraeCount() const
	{
		return NeckVertebrae + Heads;
	}

	QString PhylumName() const
	{
		return PhylumName(SkinType);
	}

	static QString PhylumName(int phylum)
	{
		Q_ASSERT(phylum >= 0 && phylum < 8);
		QString phylums[8] =
		{
			QObject::tr("Insectoid"),
			QObject::tr("Reptilian"),
			QObject::tr("Glabrian"),
			QObject::tr("Avian"),
			QObject::tr("Furrian"),
			QObject::tr("Crustacean"),
			QObject::tr("Aquarian"),
			QObject::tr("Amphibian")
		};
		return phylums[phylum];
	}

	// Generates a random 'monster' dna.
	// 'maxSize' should be 0 to 31.
	void RandomMonster(unsigned int maxSize, const AuDice& dice);

	// Generates a random 'rat' dna.
	// This will always be a small scavenger. It may fly but it will not be subterranean or aquatic.
	void RandomVermin(const AuDice& dice);

	// Returns a number from -1 to 1 that represents the current moment in the pose's rhythm cycle, which is based on its speed.
	// 'speedEnc' is speed modifier due to encumberance. It is multiplied by the natural pose speed. 1=natural speed, .5=half speed
	float Rhythm(const AuPose& pose, float cycleOffset, float speedEnc) const;

	float SightRange() const
	{
		float sightRange = (EyeSize+1) * 90;	// Sight range of eyes, in feet.
		if(CanFly())
			sightRange *= 2;	// Flyers get enhanced vision in exchange for smell.
		return sightRange;
	}

	float SmellRange() const
	{
		float smellRange = (NoseSize+1) * 30;	// Smell range of nose, in feet.
		if(EcoRole() == ECO_Scavenger)
			smellRange *= 4;	// Scavengers get enhanced smell, even flyers.
		else if(CanFly())
			smellRange /= 4;	// Flyers sacrifice smell for enhanced vision.
		return smellRange;
	}

	virtual QString SpeciesDescription() const;

	virtual AuSpecies Species() const;

	// Provides a description of the custom DNA string encoding.
	static QStringList DNAStringDescription();

	// Returns a thickness value in feet given an integer DNA thickness value from 0 to 31.
	static inline float Thickness(int thickness)
	{
		return (((float)thickness/16.f + 1) * .25f);
	}

	virtual QString TimeActiveName() const
	{
		return ::TimeActiveName((TOD)TimeActive);
	}

	// Returns the torso thickness value given a torso segment length in feet.
	inline float TorsoThicknessCalc(float torsoSegLength)
	{
		return torsoSegLength/3.f + ((float)TorsoThickness/16.f * .25f * torsoSegLength);
	}

	QString ToString() const;

	virtual quint8 Version() const
	{
		return 3;
	}

	unsigned int WaistVertex() const;

	bool WalksOnHands() const;

	WEAPONLOC WeaponLocation() const;

	QString AltWeaponLocation();

	// Returns horns or mouth for head, depending on which is bigger.
	QString WeaponImageName() const;

	// Returns horns or mouth for head, depending on which is bigger.
	QString WeaponLocationName() const;

	// Names that correspond to each WEAPONEFFECT.
	static inline QString WeaponLocationName(WEAPONLOC loc)
	{
		QString locNames[] =
		{
			QObject::tr("None"),
			QObject::tr("Anus"),
			QObject::tr("Eyes"),
			QObject::tr("Feet"),
			QObject::tr("Hands"),
			QObject::tr("Head"),
			QObject::tr("Tail"),
			QObject::tr("Torso"),
		};
		return locNames[loc];
	}

protected:
	// Converts an integer 'dnaValue' to a character. Used by SpeciesKey.
	inline QChar CharValue(unsigned int dnaValue) const
	{
		if(dnaValue > 31)
		{
			qWarning("DhDNA::CharValue dnaValue %d is invalid.", dnaValue);
			return '?';
		}
		const char* charValues = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
		return charValues[dnaValue];
	}

	// Converts 'dnaChar' to an integer value. Used by FromSpeciesKey.
	// 'maxValue' is the maximum expected value for the conversion result.
	// 'isOk' is set false on error, true if all goes well
	inline unsigned int ValueChar(const QChar& dnaChar, int maxValue, bool& isOk) const
	{
		const QString charValues("0123456789ABCDEFGHIJKLMNOPQRSTUV");
		int value = charValues.indexOf(dnaChar);
		if(value < 0)
		{
			qWarning() << "DhDNA::ValueChar dnaChar" << dnaChar << "is invalid.";
			isOk = false;
			return 0;
		}
		else if(value > maxValue)
		{
			qWarning() << "DhDNA::ValueChar dnaChar value" << value << "exceeds max" << maxValue;
			isOk = false;
			return 0;
		}
		isOk = true;
		return value;
	}
};

Q_DECLARE_METATYPE(DhDNA)	// Needed to this use class in queued signal/slot connections. Must also call qRegisterMetaType.

QDebug operator<<(QDebug dbg, const DhDNA& dna);

#endif // DhDNA_H
