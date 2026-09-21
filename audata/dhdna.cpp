//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dhdna.cpp,v 1.75 2022/04/14 17:04:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dhdna.h"
//
#include "au3dsglsldna.h"
#include "audef.h"
#include "audice.h"
#include "auelapsedtimer.h"
#include <QFile>
#include <QTextStream>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_DhDNAMetaType = qRegisterMetaType<DhDNA>();

DhDNA_::DhDNA_()
{
	memset(this, 0, sizeof(DhDNA_));
}

DhDNA_::DhDNA_(const QByteArray& ba)
{
	memcpy(this, ba.constData(), sizeof(DhDNA_));

	// Unsure the unused bits are 0 so we don't get random differences between otherwise identical DNA.
	UNUSED0 = 0;
	UNUSED1 = 0;
}

DhDNA_::DhDNA_(const DhDNA_& c)
{
	memcpy(this, &c, sizeof(DhDNA_));

	// Unsure the unused bits are 0 so we don't get random differences between otherwise identical DNA.
	UNUSED0 = 0;
	UNUSED1 = 0;
}

const DhDNA_& DhDNA_::operator=(const DhDNA_& c)
{
	memcpy(this, &c, sizeof(DhDNA_));

	// Unsure the unused bits are 0 so we don't get random differences between otherwise identical DNA.
	UNUSED0 = 0;
	UNUSED1 = 0;
	return *this;
}

void DhDNA_::operator>>(QDataStream& out) const
{
	QByteArray ba((const char*)this, sizeof(DhDNA_));
	out << ba;
}

void DhDNA_::operator<<(QDataStream& in)
{
	QByteArray ba;
	in >> ba;
	if(ba.size() != sizeof(DhDNA_))
		qWarning() << "error: DhDNA_::operator<< read" << ba.size() << "bytes, should be" << sizeof(DhDNA_);
	memcpy(this, ba.constData(), sizeof(DhDNA_));

	// Unsure the unused bits are 0 so we don't get random differences between otherwise identical DNA.
	UNUSED0 = 0;
	UNUSED1 = 0;
}

bool DhDNA_::operator==(const DhDNA_& c) const
{
	return memcmp(this, &c, sizeof(DhDNA_)) == 0;
}

void DhDNA_::Randomize(int sizemod, bool canHaveWings, TOD tod, const AuDice& Cdice, bool canChangePhylum)
{
	AuDice dice(Cdice);
	int size = abs(dice.Roll(2,31,-31+sizemod));		// 0..31.
	if(size > 31)
		size = 31;
	int sizeBase = dice.Roll(1,25,-1);		// 0..24

	Agility = dice.Roll(1,16,-1);			// 0..15
	Endurance = dice.Roll(1,16,-1);			// 0..15

	ArmPairs = dice.Roll(1,4,-1);			// 0..3
	int carrmod = sizemod / 2;					// Body carriage affected by planet size
	int carriage = dice.Roll(1,4,-1 + carrmod);
	if(carriage < 0)
		carriage = 0;
	else if(carriage > 3)
		carriage = 3;
	BodyCarriage = carriage;					// 0..3
	WeaponType = (AuDNABase::WEAPONTYPE)dice.Roll(1,16,-1);	// 0..15

	if(canHaveWings)
		ArmType = (ARM)dice.Roll(1,8,-1);	// 0..7, possibly wings
	else
		ArmType = (ARM)dice.Roll(1,2,-1);	// 0..1. No wings.
	CalfLength = dice.Roll(1,8,sizeBase-1);			// 0..31

	BottomSkinSubtype = dice.Roll(1,8,-1);	// 0..7
	CalfThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	BottomSkinThickness = dice.Roll(1,4,-1);	// 0..3
	CrestSubtype = dice.Roll(1,4,-1);		// 0..3
	EarSize = dice.Roll(1,4,-1);				// 0..3
	EcologicalRole = dice.Roll(1,4,-1);		// 0..3

	BrainLocation = (BRAIN)dice.Roll(1,2,-1);	// 0..1
	// Multiple heads are extremely rare.
	int hd = dice.Roll(1,100);				// Roll percentage
	if(hd <= 95)
		Heads = 0;	// Only one head
	else
		Heads = 1;	// Two heads
	LivesInAir = dice.Roll(1,5) == 1 ? 1 : 0;	// 0..1. 1=Flies or Leaps
	LivesOnLand = dice.Roll(1,2,-1);
	LivesInWater = dice.Roll(1,2,-1);
	LivesUnderGround = dice.Roll(1,6) == 1 ? 1 : 0;	// 0..1
	WeaponAltLocation = dice.Roll(1,2,-1);	// 0..1
	UNUSED0 = 0;									// 0..1 UNUSED BIT HERE IF YOU NEED ONE

	EarType = (EAR)dice.Roll(1,8,-1);		// 0..7
	ForeArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	EcologicalStrategy = (AuDNABase::ECOSTRATEGY)dice.Roll(1,4,-1);	// 0..3
	EyeSize = dice.Roll(1,4,-1);				// 0..3
	HeadCrestSize = dice.Roll(1,4,-1);		// 0..3
	HornBranches = dice.Roll(1,4,-1);		// 0..3

	EyeType = (EYE)dice.Roll(1,8,-1);		// 0..7
	ForeArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31

	FightReflex = dice.Roll(1,16,-1);		// 0..15
	FlightReflex = dice.Roll(1,16,-1);		// 0..15

	FootLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	FootType = (FOOT)dice.Roll(1,8,-1);		// 0..7

	HandLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	HandType = (HAND)dice.Roll(1,8,-1);		// 0..7

	HornSize = dice.Roll(1,4,-1);			// 0..3
	LegPairs = dice.Roll(1,4,-1);			// 0..3
	LegType = (LEG)dice.Roll(1,4,-1);		// 0..3
	MouthSize = dice.Roll(1,4,-1);			// 0..3

	MouthType = (MOUTH)dice.Roll(1,8,-1);	// 0..7
	OverallSize = size;

	NeckVertebrae = dice.Roll(1,4,-1);		// 0..3
	NoseSize = dice.Roll(1,4,-1);			// 0..3
	TAIL tails[10] = {TAIL_None, TAIL_None, TAIL_None, TAIL_None, 	// 4:10 = None
		TAIL_Primary, TAIL_Primary, TAIL_Primary,		// 3:10 = Primary
		TAIL_Secondary, TAIL_Secondary,					// 2:10 = Secondary
		TAIL_Tertiary};									// 1:10 = Tertiary
	TailType = tails[dice.Roll(1,10,-1)];	// 0..3
	TailVertebrae = dice.Roll(1,4,-1);		// 0..3

	NoseType = (NOSE)dice.Roll(1,8,-1);		// 0..7
	ThighLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	if(canChangePhylum)
		SkinType = (SKIN)dice.Roll(1,8,-1);		// 0..7
	ThighThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	Speed = dice.Roll(1,16,-1);				// 0..15
	Strength = dice.Roll(1,16,-1);			// 0..15

	TimeActive = tod;
	TopSkinThickness = dice.Roll(1,4,-1);	// 0..3
	TorsoCrestSize = dice.Roll(1,4,-1);		// 0..3
	WeaponMode = (AuDNABase::DNAWEAPONMODE)dice.Roll(1,4,-1);	// 0..3

	TopSkinSubtype = dice.Roll(1,8,-1);		// 0..7
	TorsoSegmentLength = dice.Roll(1,8,sizeBase-1);	// 0..31

	TorsoThickness = dice.Roll(1,8,sizeBase-1);		// 0..31
	TorsoVertebrae = dice.Roll(1,8,-1); 		// 0..7

	UpperArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31
	WeaponSize = dice.Roll(1,8,-1);			// 0..7

	UpperArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31
	WingSubType = dice.Roll(1,8,-1);			// 0..7

	UNUSED1 = 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DhDNA::DhDNA()
{
	if(!s_DhDNAMetaType)
		s_DhDNAMetaType = qRegisterMetaType<DhDNA>();
}

DhDNA::DhDNA(const QByteArray& ba) : DhDNA_(ba)
{
}

DhDNA::DhDNA(const DhDNA_& dna)
{
	operator=(dna);
}

const DhDNA& DhDNA::operator=(const DhDNA_& dna)
{
	DhDNA_::operator=(dna);
	return *this;
}

bool DhDNA::operator==(const AuDNABase& b) const
{
	try
	{
		const DhDNA& c = dynamic_cast<const DhDNA&>(b);
		return DhDNA_::operator==(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

// static
QStringList DhDNA::DNAStringDescription()
{
	QStringList out;
	out << "  Index   Trait                 Valid Values";
	out << "  -----   -------------------   --------------------------------";
	out << "   0      Agility               0123456789ABCDEF";
	out << "   1      Endurance             0123456789ABCDEF";
	out << "   2      Arm Pairs             0123";
	out << "   3      Body Carriage         0123";
	out << "                                  0 Crawl";
	out << "                                  1 Squat";
	out << "                                  2 Crouch";
	out << "                                  3 Stand";
	out << "   4      Weapon Type           0123456789ABCDEF";
	out << "                                  0 None";
	out << "                                  1 Acid";
	out << "                                  2 Suffocate";
	out << "                                  3 Stun";
	out << "                                  4 Poison";
	out << "                                  5 Radiation";
	out << "                                  6 Disease";
	out << "                                  7 Electric";
	out << "                                  8 Bludgeon";
	out << "                                  9 Cold";
	out << "                                  A Crush";
	out << "                                  B Sonic";
	out << "                                  C Heat";
	out << "                                  D Pierce";
	out << "                                  E Implant";
	out << "                                  F Slash";
	out << "   5      Arm Type              01234567. Only the 1st arm pair is Wings; extra arm pairs are always Limbs.";
	out << "                                  0 Limb";
	out << "                                  1 Tentacle";
	out << "                                  2 Wing Butterfly, glides";
	out << "                                  3 Wing DragonFly, hovers";
	out << "                                  4 Wing Fairy, hovers";
	out << "                                  5 Wing Feathered, glides";
	out << "                                  6 Wing Leathery, glides";
	out << "                                  7 Wing Moth, glides";
	out << "   6      Calf Length           0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "   7      Bottom Skin Subtype   01234567";
	out << "   8      Calf Thickness        0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "   9      Bottom Skin Thickness 0123";
	out << "  10      Crest Subtype         0123";
	out << "  11      Ear Size              0123";
	out << "  12      Ecological Role       0123";
	out << "                                  0 Carnivore";
	out << "                                  1 Herbivore";
	out << "                                  2 Omnivore";
	out << "                                  3 Scavenger";
	out << "  13      Brain Location        01";
	out << "                                  0 Head";
	out << "                                  1 Torso";
	out << "  14      Heads                 01";
	out << "                                  0 One Head";
	out << "                                  1 Two Heads";
	out << "  15      Lives In Air          01";
	out << "                                  0 No";
	out << "                                  1 Yes, flyer with wings, jumper without wings.";
	out << "  16      Lives In Water        01";
	out << "                                  0 No";
	out << "                                  1 Yes, water dweller.";
	out << "  17      Lives On Land         01";
	out << "                                  0 No";
	out << "                                  1 Yes, land dweller.";
	out << "  18      Lives Under Ground    01";
	out << "                                  0 No";
	out << "                                  1 Yes, burrower.";
	out << "  19      Weapon Alt Location   01";
	out << "                                  0 No";
	out << "                                  1 Yes";
	out << "  20      Unused                01";
	out << "  21      Ear Type              01234567";
	out << "                                  0 Antennae";
	out << "                                  1 Ear";
	out << "                                  2 Fibers";
	out << "                                  3 Floppy";
	out << "                                  4 Not Apparent";
	out << "                                  5 Spot";
	out << "                                  6 Stalk";
	out << "                                  7 Tab";
	out << "  22      Fore Arm Length       0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  23      Ecological Strategy   0123";
	out << "                                  0 Waiter";
	out << "                                  1 Grazer";
	out << "                                  2 Hunter";
	out << "                                  3 Sneaker";
	out << "  24      Eye Size              0123";
	out << "  25      Head Crest Size       0123";
	out << "  26      Horn Branches         0123";
	out << "  27      Eye Type              01234567";
	out << "                                  0 Cluster Simple";
	out << "                                  1 Cluster Stalk";
	out << "                                  2 Complex";
	out << "                                  3 Facet";
	out << "                                  4 Not Apparent";
	out << "                                  5 Simple";
	out << "                                  6 Slit";
	out << "                                  7 Stalk";
	out << "  28      Fore Arm Thickness    0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  29      Fight Reflex          0123456789ABCDEF";
	out << "  30      Flight Reflx          0123456789ABCDEF";
	out << "  31      Foot Length           0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  32      Foot Type             01234567";
	out << "                                  0 Claw";
	out << "                                  1 Flipper";
	out << "                                  2 Foot";
	out << "                                  3 Hand";
	out << "                                  4 Hoof";
	out << "                                  5 Paw";
	out << "                                  6 Pincher";
	out << "                                  7 Sucker";
	out << "  33      Hand Length           0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  34      Hand Type             01234567";
	out << "                                  0 Claw";
	out << "                                  1 Flipper";
	out << "                                  2 Foot";
	out << "                                  3 Hand";
	out << "                                  4 Hoof";
	out << "                                  5 Paw";
	out << "                                  6 Pincher";
	out << "                                  7 Sucker";
	out << "  35      Horn Size             0123";
	out << "  36      Leg Pairs             0123";
	out << "  37      Leg Type              0123";
	out << "                                  0 Cilia";
	out << "                                  1 Limb";
	out << "                                  2 Pseudopod";
	out << "                                  3 Tentacle";
	out << "  38      Mouth Size            0123";
	out << "  39      Mouth Type            01234567";
	out << "                                  0 Beak";
	out << "                                  1 Cilia";
	out << "                                  2 Fangs";
	out << "                                  3 Mandible";
	out << "                                  4 Mouth";
	out << "                                  5 Not Apparent";
	out << "                                  6 Teeth";
	out << "                                  7 Tusks";
	out << "  40      Overall Size          0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  41      Neck Vertebrae        0123";
	out << "  42      Nose Size             0123";
	out << "  43      Tail Type             0123";
	out << "                                  0 None";
	out << "                                  0 Primary";
	out << "                                  0 Secondary";
	out << "                                  0 Tertiary";
	out << "  44      Tail Vertebrae        0123";
	out << "  45      Nose Type             01234567";
	out << "                                  0 Cilia";
	out << "                                  1 Holes";
	out << "                                  2 Nose";
	out << "                                  3 Not Apparent";
	out << "                                  4 Proboscis";
	out << "                                  5 Snorkel";
	out << "                                  6 Snout";
	out << "                                  7 Trunk";
	out << "  46      Thigh Length          0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  47      Skin Type             01234567";
	out << "                                  0" << PhylumName(0);
	out << "                                  1" << PhylumName(1);
	out << "                                  2" << PhylumName(2);
	out << "                                  3" << PhylumName(3);
	out << "                                  4" << PhylumName(4);
	out << "                                  5" << PhylumName(5);
	out << "                                  6" << PhylumName(6);
	out << "                                  7" << PhylumName(7);
	out << "  48      Thigh Thickness       0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  49      Speed                 0123456789ABCDEF";
	out << "  50      Strength              0123456789ABCDEF";
	out << "  51      Time Active           0123";
	out << "                                  0 Dark";
	out << "                                  1 Dim";
	out << "                                  2 Bright";
	out << "                                  3 Any";
	out << "  52      Top Skin Thickness    0123";
	out << "  53      Torso Crest Size      0123";
	out << "  54      Weapon Mode           0123";
	out << "                                  0 Area";
	out << "                                  1 Contact";
	out << "                                  2 Projectile";
	out << "                                  3 Psionic";
	out << "  55      Top Skin Subtype      01234567";
	out << "  56      Torso Segment Length  0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  57      Torso Thickness       0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  58      Torso Vertebrae       01234567";
	out << "  59      Upper Arm Length      0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  60      Weapon Size           01234567";
	out << "  61      Upper Arm Thickness   0123456789ABCDEFGHIJKLMNOPQRSTUV";
	out << "  62      Wing SubType          01234567";
	return out;
}

void DhDNA::operator>>(QDataStream& out) const
{
	DhDNA_::operator>>(out);
}

void DhDNA::operator<<(QDataStream& in)
{
	DhDNA_::operator<<(in);
}

// Returns number of vertices in back allocated to arms.
// Returns zero if none.
unsigned int DhDNA::ArmVertices() const
{
	// The creature must have arms to need vertices.
	if(!ArmPairs)
		return 0;

	// The creature must have a spine to use more than one vertex for arms.
	if(!TorsoVertebrae)
		return 1;

	// This creature has arms and at least two vertices to sprout limbs.
	unsigned int verts = (unsigned int)TorsoVertebrae + 1;	// Use each end of
	// spine segments.
	unsigned int armverts = verts;
	if(LegPairs)
	{
		// This creature has legs and arms and at least two vertices to sprout
		// limbs.
		// Arms and Legs are split among the available body vertebrae.
		// Arms and legs get their own vertices in this situation; they don't
		// share.
		armverts = verts * ArmPairs / (ArmPairs + LegPairs);
		if(armverts >= verts)
			armverts = verts-1;			// Leave at least one vertex for legs.
		if(armverts < 1)
			armverts = 1;				// Need at least one vertex for arms.
	}
	if(armverts > ArmPairs)
		armverts = ArmPairs;	// Don't take more vertices than needed.
	//qDebug("DhDNA::ArmVertices %d", armverts);
	return armverts;
}

// Returns number of vertices in back allocated to legs.
// Returns zero if none.
unsigned int DhDNA::LegVertices() const
{
	// The creature must have legs to need vertices.
	if(!LegPairs)
		return 0;

	// The creature must have a spine to use more than one vertex for legs.
	if(!TorsoVertebrae)
		return 1;

	// This creature has legs and at least two vertices to sprout limbs.
	unsigned int verts = (unsigned int)TorsoVertebrae + 1;	// Use each end of spine segments.
	unsigned int armverts = ArmVertices();
	unsigned int legverts = verts - armverts;
	if(legverts < 1)
		legverts = 1;				// Need at least one vertex for legs.
	else if(legverts > LegPairs)
		legverts = LegPairs;	// Don't take more vertices than needed.
	//qDebug("DhDNA::LegVertices %d", legverts);
	return legverts;
}

bool DhDNA::HasAnkleBend() const
{
	if(FootType == FOOT_Hoof || FootType == FOOT_Pincher)
		return false;
	return true;
}

bool DhDNA::HasWristBend() const
{
	if(HandType == HAND_Pincher || HandType == HAND_Hoof)
		return false;
	return true;
}

bool DhDNA::IsExoskeletal() const
{
	if(SkinType == SKIN_DryExoskeleton || SkinType == SKIN_WetExoskeleton)
		return true;
	else
		return false;
}

// Weapon location determines creature posture when using the weapon.
AuDNABase::WEAPONLOC DhDNA::WeaponLocation() const
{
	if(WeaponType == WT_None)
	{
		//qDebug("DhDNA::WeaponLocation no weapon = None");
		return WL_None;
	}

	WEAPONLOC loc = WL_Head;	// Default return value unless changed.
	if(WeaponAltLocation)
	{
		// Select alternate weapon location based on weapon mode and physical
		// construction of this critter.
		switch(WeaponMode)
		{
		case DWM_Contact:	// Touch. Loc 0=Head, 1=Tail, claws, then body
			if(IsExoskeletal())
			{
				// Exoskeletal creatures prefer contact poison in the tail.
				// e.g. Wasps, bees, scorpions.
				if(EyeSize > 1)
					loc = WL_Eyes;	// Prominent eyes.
				else if(TailVertebrae > 1)
					loc = WL_Tail;	// Flexible tail.
				else if(HasArms() && HandLength > 1)
					loc = WL_Hands;	// Prominent hands.
				else if(LegPairs && FootLength > 1)
					loc = WL_Feet;	// Prominent feet.
				else if(TorsoVertebrae > 1)
					loc = WL_Anus;	// Flexible body.
				else if(TorsoVertebrae)
					loc = WL_Torso;	// Body.
				//qDebug("DhDNA::WeaponLocation exoskeletal contact weapon in %d", loc);
			}
			else
			{
				// Non-exoskeletal creatures prefer contact poison in hands/claws.
				// Kimodo dragon
				if(EyeSize > 1)
					loc = WL_Eyes;	// Prominent eyes.
				else if(HasArms() && HandLength > 1)
					loc = WL_Hands;	// Prominent hands.
				else if(LegPairs && FootLength > 1)
					loc = WL_Feet;	// Prominent feet.
				else if(TailVertebrae > 1)
					loc = WL_Tail;	// Flexible tail.
				else if(TorsoVertebrae > 1)
					loc = WL_Anus;	// Flexible body.
				else if(TorsoVertebrae)
					loc = WL_Torso;	// Body.
				//qDebug("DhDNA::WeaponLocation endoskeletal contact weapon in %d", loc);
			}
			break;
		case DWM_Area:	// Gas, spore, mist. Breathed by victim.
			loc = WL_Anus;		// Stinky butt.
			//qDebug("DhDNA::WeaponLocation area weapon in Anus %d", loc);
			break;
		case DWM_Projectile:	// Hurled, spit, flung. Loc 0=Head, 1=Anus
			if(TailVertebrae > 1)
				loc = WL_Tail;	// Flings with tail. Chimera
			else if(EyeSize > 1)
				loc = WL_Eyes;	// Prominent eyes.
			else if(TorsoVertebrae)
				loc = WL_Anus;	// Most unpleasant. Skunk
			//qDebug("DhDNA::WeaponLocation projectile weapon in %d", loc);
			break;
		case DWM_Psionic:	// Telepathic. Loc 0=Head, 1=Hands, then body
			if(HasArms())
				loc = WL_Hands;	// Waves arms
			else if(TorsoVertebrae)
				loc = WL_Torso;	// Emanates from body.
			//qDebug("DhDNA::WeaponLocation psionic weapon in %d", loc);
			break;
		}
	}
	//else
	//qDebug("DhDNA::WeaponLocation weapon in Head %d", loc);

	return loc;
}

// Return a string pointing to the alternate location for this weapon type
QString DhDNA::AltWeaponLocation()
{
	int iPrevLoc = WeaponAltLocation;
	WeaponAltLocation = 1;
	WEAPONLOC loc = WeaponLocation();
	WeaponAltLocation = iPrevLoc;
	return WeaponLocationName(loc);
}

// Returns vertex in back at which spine bends, even if it has no legs.
// Returns zero if no arms.
// Return value is always the first vertex at which legs sprout if it has legs.
unsigned int DhDNA::WaistVertex() const
{
	if(LegPairs == 1)
		return TorsoVertebrae;		// Last vertex when only one pair of legs.

	int armverts = ArmVertices();
	if(!armverts)
		return 0;	// First vertex when no arms, whether or not there are legs.

	unsigned int waist = 0;
	if(LegPairs == 0)
	{
		// Arms but no legs.
		// Does it walk on its hands?
		if(WalksOnHands())
		{
			// Walks on its hands so bend at last arm vertex.
			waist = armverts - 1;
		}
		else
		{
			// Does not walk on its hands so bend at next vertex after arms if
			// possible.
			waist = armverts;
		}
	}
	else
	{
		// Multiple sets of legs so waist is at first set of legs along spine,
		// which are at the next vertex after the last arm vertex.
		waist = armverts;
	}
	if(waist > TorsoVertebrae)
		waist = TorsoVertebrae;
	return waist;
}

bool DhDNA::WalksOnHands() const
{
	/*
	if(!ArmPairs)
	return false;	// Can't walk on hands because it doesn't have any arms.
	if(HandType == HAND_Hand)
	return false;	// These hand types are not used as feet.
	else if(ArmPairs == 1 && HasWings())
	return false;	// Its only set of arms is wings; don't stand on them.
	// When ArmPairs > 1, only one set is wings; others are arms.
	else
	return true;	// All other hand types are used as feet.
	*/
	if(HandType == HAND_Hand || HandType == HAND_Pincher)
		return false;	// These hand types are not used as feet.
	if(!HasArms())
		return false;	// Can't walk on hands because it doesn't have any arms.
	return true;	// All other hand types are used as feet.
}

// Generates a random 'rat' dna.
// This will always be a small scavenger. It may fly but it will not be subterranean or aquatic.
void DhDNA::RandomVermin(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	//int size = abs(dice.Roll(2,2,-2));		// 0, 1, 1, 2
	int size = 0;							// These were as big as dogs at size 0.
	int sizeBase = dice.Roll(1,25,-1);		// 0..24
	sizeBase /= 3;							// Keep rats small.

	Agility = dice.Roll(1,16,-1);			// 0..15
	Endurance = dice.Roll(1,16,-1);			// 0..15

	ArmPairs = dice.Roll(1,4,-1);			// 0..3
	BodyCarriage = dice.Roll(1,4,-1);		// 0..3
	WeaponType = (WEAPONTYPE)dice.Roll(1,16,-1);	// 0..15

	bool canHaveWings = (dice.Roll(1,100) <= 20);	// 20% probability that the rat MIGHT have wings.
	if(canHaveWings)
		ArmType = (ARM)dice.Roll(1,8,-1);	// 0..7, possibly wings
	else
		ArmType = (ARM)dice.Roll(1,2,-1);	// 0..1. No wings.
	CalfLength = dice.Roll(1,8,sizeBase-1);			// 0..31

	BottomSkinSubtype = dice.Roll(1,8,-1);	// 0..7
	CalfThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	BottomSkinThickness = dice.Roll(1,4,-1);	// 0..3
	CrestSubtype = dice.Roll(1,4,-1);		// 0..3
	EarSize = dice.Roll(1,4,-1);				// 0..3
	EcologicalRole = ECO_Scavenger;			// This IS a rat, after all.

	BrainLocation = (BRAIN)dice.Roll(1,2,-1);	// 0..1
	// Multiple heads are extremely rare.
	int hd = dice.Roll(1,100);				// Roll percentage
	if(hd <= 95)
		Heads = 0;	// Only one head
	else
		Heads = 1;	// Two heads
	LivesInAir = dice.Roll(1,5) == 1 ? 1 : 0;	// 0..1. 1=Flies or Leaps
	LivesOnLand = 1;							// NOTE: This bit is also used as an indication of whether the rat DNA is valid.
	LivesInWater = dice.Roll(1,2,-1);
	LivesUnderGround = dice.Roll(1,6) == 1 ? 1 : 0;	// 0..1
	WeaponAltLocation = dice.Roll(1,2,-1);	// 0..1
	UNUSED0 = 0;									// 0..1 UNUSED BIT HERE IF YOU NEED ONE

	EarType = (EAR)dice.Roll(1,8,-1);		// 0..7
	ForeArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	EcologicalStrategy = (ECOSTRATEGY)dice.Roll(1,4,-1);	// 0..3
	EyeSize = dice.Roll(1,4,-1);				// 0..3
	HeadCrestSize = dice.Roll(1,4,-1);		// 0..3
	HornBranches = dice.Roll(1,4,-1);		// 0..3

	EyeType = (EYE)dice.Roll(1,8,-1);		// 0..7
	ForeArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31

	FightReflex = dice.Roll(1,16,-1);		// 0..15
	FlightReflex = dice.Roll(1,16,-1);		// 0..15

	FootLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	FootType = (FOOT)dice.Roll(1,8,-1);		// 0..7

	HandLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	HandType = (HAND)dice.Roll(1,8,-1);		// 0..7

	HornSize = dice.Roll(1,4,-1);			// 0..3
	LegPairs = dice.Roll(1,4,-1);			// 0..3
	LegType = (LEG)dice.Roll(1,4,-1);		// 0..3
	MouthSize = dice.Roll(1,4,-1);			// 0..3

	MouthType = (MOUTH)dice.Roll(1,8,-1);	// 0..7
	OverallSize = size;

	NeckVertebrae = dice.Roll(1,4,-1);		// 0..3
	NoseSize = dice.Roll(1,4,-1);			// 0..3
	TAIL tails[10] = {TAIL_None, 									// 1:10 = None
		TAIL_Primary, TAIL_Primary, TAIL_Primary, TAIL_Primary,		// 4:10 = Primary
		TAIL_Secondary, TAIL_Secondary, TAIL_Secondary,				// 3:10 = Secondary
		TAIL_Tertiary, TAIL_Tertiary};								// 2:10 = Tertiary
	TailType = tails[dice.Roll(1,10,-1)];	// 0..3
	TailVertebrae = dice.Roll(1,4,-1);		// 0..3

	NoseType = (NOSE)dice.Roll(1,8,-1);		// 0..7
	ThighLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	SkinType = (SKIN)dice.Roll(1,8,-1);		// 0..7
	ThighThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	Speed = dice.Roll(1,16,-1);				// 0..15
	Strength = dice.Roll(1,16,-1);			// 0..15

	TimeActive = dice.Roll(1,4,-1);			// 0..3
	TopSkinThickness = dice.Roll(1,4,-1);	// 0..3
	TorsoCrestSize = dice.Roll(1,4,-1);		// 0..3
	WeaponMode = (DNAWEAPONMODE)dice.Roll(1,4,-1);	// 0..3

	TopSkinSubtype = dice.Roll(1,8,-1);		// 0..7
	TorsoSegmentLength = dice.Roll(1,8,sizeBase-1);	// 0..31

	TorsoThickness = dice.Roll(1,8,sizeBase-1);		// 0..31
	TorsoVertebrae = dice.Roll(1,8,-1); 		// 0..7

	UpperArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31
	WeaponSize = dice.Roll(1,8,-1);			// 0..7

	UpperArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31
	WingSubType = dice.Roll(1,8,-1);			// 0..7

	UNUSED1 = 0;
}

void DhDNA::RandomMonster(unsigned int maxSize, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	int size = dice.Roll(1, maxSize);
	if(size > 31)
		size = 31;
	int sizeBase = dice.Roll(1,25,-1);		// 0..24

	Agility = dice.Roll(1,16,-1);			// 0..15
	Endurance = dice.Roll(1,16,-1);			// 0..15

	ArmPairs = dice.Roll(1,4,-1);			// 0..3
	int carriage = dice.Roll(1,4,-1);
	if(carriage < 0)
		carriage = 0;
	else if(carriage > 3)
		carriage = 3;
	BodyCarriage = carriage;					// 0..3
	WeaponType = (WEAPONTYPE)dice.Roll(1,16,-1);	// 0..15

	//if(canHaveWings)
		//ArmType = (ARM)dice.Roll(1,8,-1);	// 0..7, possibly wings
	//else
		ArmType = (ARM)dice.Roll(1,2,-1);	// 0..1. No wings.
	CalfLength = dice.Roll(1,8,sizeBase-1);			// 0..31

	BottomSkinSubtype = dice.Roll(1,8,-1);	// 0..7
	CalfThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	BottomSkinThickness = dice.Roll(1,4,-1);	// 0..3
	CrestSubtype = dice.Roll(1,4,-1);		// 0..3
	EarSize = dice.Roll(1,4,-1);				// 0..3
	if(dice.Roll(1,4) == 1)
		EcologicalRole = ECO_Omnivore;
	else
		EcologicalRole = ECO_Carnivore;

	BrainLocation = (BRAIN)dice.Roll(1,2,-1);	// 0..1
	// Multiple heads are extremely rare.
	int hd = dice.Roll(1,100);				// Roll percentage
	if(hd <= 95)
		Heads = 0;	// Only one head
	else
		Heads = 1;	// Two heads
	LivesInAir = 0;	//dice.Roll(1,5) == 1 ? 1 : 0;	// 0..1. 1=Flies or Leaps
	LivesOnLand = 1;
	LivesInWater = dice.Roll(1,2,-1);
	LivesUnderGround = dice.Roll(1,6) == 1 ? 1 : 0;	// 0..1
	WeaponAltLocation = dice.Roll(1,2,-1);	// 0..1
	UNUSED0 = 0;									// 0..1 UNUSED BIT HERE IF YOU NEED ONE

	EarType = (EAR)dice.Roll(1,8,-1);		// 0..7
	ForeArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	EcologicalStrategy = (ECOSTRATEGY)dice.Roll(1,4,-1);	// 0..3
	EyeSize = dice.Roll(1,4,-1);				// 0..3
	HeadCrestSize = dice.Roll(1,4,-1);		// 0..3
	HornBranches = dice.Roll(1,4,-1);		// 0..3

	EyeType = (EYE)dice.Roll(1,8,-1);		// 0..7
	ForeArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31

	FightReflex = 15;	//dice.Roll(1,16,-1);		// 0..15
	FlightReflex = 0;	//dice.Roll(1,16,-1);		// 0..15

	FootLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	FootType = (FOOT)dice.Roll(1,8,-1);		// 0..7

	HandLength = dice.Roll(1,8,sizeBase-1);			// 0..31
	HandType = (HAND)dice.Roll(1,8,-1);		// 0..7

	HornSize = dice.Roll(1,4,-1);			// 0..3
	LegPairs = dice.Roll(1,4,-1);			// 0..3
	LegType = (LEG)dice.Roll(1,4,-1);		// 0..3
	MouthSize = dice.Roll(1,4,-1);			// 0..3

	MouthType = (MOUTH)dice.Roll(1,8,-1);	// 0..7
	OverallSize = size;

	NeckVertebrae = dice.Roll(1,4,-1);		// 0..3
	NoseSize = dice.Roll(1,4,-1);			// 0..3
	TAIL tails[10] = {TAIL_None, TAIL_None, TAIL_None, TAIL_None, 	// 4:10 = None
		TAIL_Primary, TAIL_Primary, TAIL_Primary,		// 3:10 = Primary
		TAIL_Secondary, TAIL_Secondary,					// 2:10 = Secondary
		TAIL_Tertiary};									// 1:10 = Tertiary
	TailType = tails[dice.Roll(1,10,-1)];	// 0..3
	TailVertebrae = dice.Roll(1,4,-1);		// 0..3

	NoseType = (NOSE)dice.Roll(1,8,-1);		// 0..7
	ThighLength = dice.Roll(1,8,sizeBase-1);		// 0..31

	SkinType = (SKIN)dice.Roll(1,8,-1);		// 0..7
	ThighThickness = dice.Roll(1,8,sizeBase-1);		// 0..31

	Speed = dice.Roll(1,16,-1);				// 0..15
	Strength = dice.Roll(1,16,-1);			// 0..15

	TimeActive = dice.Roll(1,3,-1);
	TopSkinThickness = dice.Roll(1,4,-1);	// 0..3
	TorsoCrestSize = dice.Roll(1,4,-1);		// 0..3
	WeaponMode = (DNAWEAPONMODE)dice.Roll(1,4,-1);	// 0..3

	TopSkinSubtype = dice.Roll(1,8,-1);		// 0..7
	TorsoSegmentLength = dice.Roll(1,8,sizeBase-1);	// 0..31

	TorsoThickness = dice.Roll(1,8,sizeBase-1);		// 0..31
	TorsoVertebrae = dice.Roll(1,8,-1); 		// 0..7

	UpperArmLength = dice.Roll(1,8,sizeBase-1);		// 0..31
	WeaponSize = dice.Roll(1,8,-1);			// 0..7

	UpperArmThickness = dice.Roll(1,8,sizeBase-1);	// 0..31
	WingSubType = dice.Roll(1,8,-1);			// 0..7

	UNUSED1 = 0;
}

void DhDNA::InitDefaultHuman()
{
	Agility = 5;
	Endurance = 6;
	Speed = 4;			// Speed+1 = walk speed in fps, i.e. 0=1fps, 1=2fps, 15=16fps. 4=5fps=5.48km/hr.
	Strength = 5;

	ArmPairs = 1;
	BodyCarriage = 3;
	WeaponType = DhDNA::WT_None;
	ArmType = DhDNA::ARM_Limb;
	CalfLength = 16;
	BottomSkinSubtype = 2;
	CalfThickness = 18;
	BottomSkinThickness = 0;
	CrestSubtype = 0;
	EarSize = 0;
	EcologicalRole = ECO_Omnivore;
	BrainLocation = DhDNA::BRAIN_Head;
	Heads = 0;
	LivesInAir = 0;
	LivesInWater = 0;
	LivesOnLand = 1;
	LivesUnderGround = 0;
	WeaponAltLocation = 0;
	UNUSED0 = 0;
	EarType = DhDNA::EAR_Ear;
	ForeArmLength = 14;
	EcologicalStrategy = ECO_Hunter;
	EyeSize = 1;
	HeadCrestSize = 1;
	HornBranches = 0;
	EyeType = DhDNA::EYE_Complex;
	ForeArmThickness = 16;
	FightReflex = 7;
	FlightReflex = 7;
	FootLength = 24;
	FootType = DhDNA::FOOT_Foot;
	HandLength = 10;
	HandType = DhDNA::HAND_Hand;
	HornSize = 0;
	LegPairs = 1;
	LegType = DhDNA::LEG_Limb;
	MouthSize = 1;
	MouthType = DhDNA::MOUTH_Teeth;
	OverallSize = 8;
	NeckVertebrae = 1;
	NoseSize = 1;
	TailType = DhDNA::TAIL_None;
	TailVertebrae = 0;
	NoseType = DhDNA::NOSE_Nose;
	ThighLength = 16;
	SkinType = DhDNA::SKIN_DrySkin;
	ThighThickness = 20;
	TimeActive = TOD_Diurnal;
	TopSkinThickness = 0;
	TorsoCrestSize = 0;
	WeaponMode = DhDNA::DWM_Psionic;
	TopSkinSubtype = 2;
	TorsoSegmentLength = 14;
	TorsoThickness = 22;
	TorsoVertebrae = 2;
	UpperArmLength = 20;
	WeaponSize = 0;
	UpperArmThickness = 20;
	WingSubType = 0;
	UNUSED1 = 0;
}

/*
bool DhDNA::IsValid() const
{
int iPointsUsed;
iPointsUsed = OverallSize/3;
iPointsUsed += Speed;
iPointsUsed += Agility;
iPointsUsed += Strength;
iPointsUsed += Endurance;
iPointsUsed += TopSkinThickness;
iPointsUsed += BottomSkinThickness;
if(WeaponType != 0)
{
iPointsUsed += 2;
if(WeaponMode == 0)			// Area
iPointsUsed += WeaponSize*4;
else if(WeaponMode == 1)	// Contact
iPointsUsed += WeaponSize*1;
else if(WeaponMode == 2)	// Projectile
iPointsUsed += WeaponSize*3;
else if(WeaponMode == 3)	// Psionic
iPointsUsed += WeaponSize*2;
}
DpBody body;
body.SetDNA(*this);
AuDNABase::WEAPONLOC wLoc;
WEAPONMODE wMode;
int wToHitMod;
float wRange;
WEAPONEFFECT wEffect1 = WE_None;
WEAPONEFFECT wEffect2 = WE_None;
int wDamage1,wDamage2;
int wDamageMod1,wDamageMod2;
float wRadius1,wRadius2;

// Physical weapon

body.UseSpecialWeapon(now);
body.WeaponInfo(0, wLoc, wMode, wToHitMod, wRange, wEffect1, wDamage1, wDamageMod1, wRadius1, wEffect2,
wDamage2, wDamageMod2, wRadius2, now);
iPointsUsed += wDamage1/2;		// 2 points of physical weapon damage = 1 ability point
int hitPoints = body.MaxHitPoints();
float cgHeight = 0;
float cameraLoc = 0;
if(BodyCarriage == 0 || BodyCarriage == 1)
{
body.Pose(Crouch);
cgHeight = body.CGHeight();
cameraLoc = body.CameraLocation().z;
}
else
{
body.Pose(Stand);
cgHeight = body.CGHeight();
cameraLoc = body.CameraLocation().z;
}
if(iPointsUsed > 60)
{
//qDebug("DpBody::IsValid - Too many points used = %d", iPointsUsed);
return false;
}
else if(cgHeight < 1.5f || 8 < cgHeight)
{
//qDebug("DpBody::IsValid - cgHeight not valid = %f", cgHeight);
return false;
}
else if(cgHeight+cameraLoc < 1.5 || 7.25 < cgHeight+cameraLoc)
{
//qDebug("DpBody::IsValid - Head height not valid. cgHeight:%f cameraLoc:%f", cgHeight, cameraLoc);
return false;
}
else if(CanFly() && (hitPoints < 25 || 40 < hitPoints))
{
//qDebug("DpBody::IsValid - Hit points not valid for a flier = %d", hitPoints);
return false;
}
else if(!CanFly() && LivesInAir && (hitPoints < 25 || 53.33f < hitPoints))
{
//qDebug("DpBody::IsValid - Hit points not valid for a jumper = %d", hitPoints);
return false;
}
else if(!CanFly() && !LivesInAir && (hitPoints < 25 || hitPoints > 80))
{
//qDebug("DpBody::IsValid - Hit points not valid = %d", hitPoints);
return false;
}

//qDebug("DpBody::IsValid - DNA is valid");
return true;
}
*/

QString DhDNA::Description() const
{
	QString env;
	if(LivesUnderGround)
		env = "Subterranean ";
	if(LivesInWater && CanFly() && LivesOnLand)
		env += "Triphibious ";
	else if(LivesInWater && LivesOnLand)
		env += "Amphibious ";
	else if(LivesInWater && CanFly())
		env += "Aquaerial ";
	else if(CanFly())
		env += "Aerial ";
	else if(LivesInWater)
		env += "Aquatic ";

	QString strategy;
	switch(EcologicalStrategy)
	{
	case ECO_Waiter:		// Waits for food to come within range. Rarely moves.
		strategy = "Waiting ";
		break;
	case ECO_Grazer:		// Eats continuously while moving.
		strategy = "Grazing ";
		break;
	case ECO_Hunter:		// Moves continuously. Stops to eat.
		strategy = "Hunting ";
		break;
	case ECO_Sneaker:		// Moves intermittently. Stops to eat.
		strategy = "Sneaking ";
		break;
	}

	QString role;
	switch(EcologicalRole)
	{
	case ECO_Carnivore:		// Creature only eats meat that it kills.
		role = "Carnivorous ";
		break;
	case ECO_Herbivore:		// Creature only eats plants.
		role = "Herbivorous ";
		break;
	case ECO_Omnivore:		// Creature eats plants or meat that it kills.
		role = "Omnivorous ";
		break;
	case ECO_Scavenger:		// Creature only eats meat killed by others.
		role = "Scavenging ";
		break;
	}

	QString desc;
	if(!env.isEmpty())
		desc = env + strategy + role + PhylumName();
	else
		desc = strategy + role + PhylumName();
	return desc;
}

// Short description used when spotted by a person using their comm interface.
QString DhDNA::SpeciesDescription() const
{
	QString env;
	if(LivesUnderGround)
		env = "Subterranean ";
	if(LivesInWater && CanFly() && LivesOnLand)
		env += "Triphibious ";
	else if(LivesInWater && LivesOnLand)
		env += "Amphibious ";
	else if(LivesInWater && CanFly())
		env += "Aquaerial ";
	else if(CanFly())
		env += "Aerial ";
	else if(LivesInWater)
		env += "Aquatic ";

	QString role;
	switch(EcologicalRole)
	{
	case ECO_Carnivore:		// Creature only eats meat that it kills.
		role = "Carnivorous ";
		break;
	case ECO_Herbivore:		// Creature only eats plants.
		role = "Herbivorous ";
		break;
	case ECO_Omnivore:		// Creature eats plants or meat that it kills.
		role = "Omnivorous ";
		break;
	case ECO_Scavenger:		// Creature only eats meat killed by others.
		role = "Scavenging ";
		break;
	}

	if(!env.isEmpty())
		return env + role + PhylumName();
	else
		return role + PhylumName();
}

void DhDNA::Dump() const
{
	QFile file("dna.txt");
	if(file.open(QIODevice::Text |
		QIODevice::Append |
		QIODevice::WriteOnly))
	{
		QTextStream out(&file);
		out << "--------------------------------------------------\n";
		out << "Description " << Description() << "\n";
		out << "OverallSize " << OverallSize << "\n";
		out << "TorsoVertebrae " << TorsoVertebrae << "\n";
		out << "NeckVertebrae " << NeckVertebrae << "\n";
		out << "TailVertebrae " << TailVertebrae << "\n";
		out << "Heads " << Heads << "\n";
		out << "ArmPairs " << ArmPairs << "\n";
		out << "WeaponAltLocation " << WeaponAltLocation << "\n";
		out << "LegPairs " << LegPairs << "\n";
		out << "ArmType " << ArmType << "\n";
		out << "LegType " << LegType << "\n";
		out << "BrainLocation " << BrainLocation << "\n";
		out << "TorsoSegmentLength " << TorsoSegmentLength << "\n";
		out << "TorsoThickness " << TorsoThickness << "\n";
		out << "TailType " << TailType << "\n";
		out << "TopSkinThickness " << TopSkinThickness << "\n";
		out << "TopSkinSubtype " << TopSkinSubtype << "\n";
		out << "SkinType " << SkinType << "\n";
		out << "HandLength " << HandLength << "\n";
		out << "HandType " << HandType << "\n";
		out << "FootLength " << FootLength << "\n";
		out << "FootType " << FootType << "\n";
		out << "UpperArmLength " << UpperArmLength << "\n";
		out << "UpperArmThickness " << UpperArmThickness << "\n";
		out << "ForeArmLength " << ForeArmLength << "\n";
		out << "ForeArmThickness " << ForeArmThickness << "\n";
		out << "ThighLength " << ThighLength << "\n";
		out << "ThighThickness " << ThighThickness << "\n";
		out << "CalfLength " << CalfLength << "\n";
		out << "CalfThickness " << CalfThickness << "\n";
		out << "MouthType " << MouthType << "\n";
		out << "MouthSize " << MouthSize << "\n";
		out << "NoseType " << NoseType << "\n";
		out << "NoseSize " << NoseSize << "\n";
		out << "HornBranches " << HornBranches << "\n";
		out << "HornSize " << HornSize << "\n";
		out << "EyeSize " << EyeSize << "\n";
		out << "EyeType " << EyeType << "\n";
		out << "EarSize " << EarSize << "\n";
		out << "EarType " << EarType << "\n";
		out << "HeadCrestSize " << HeadCrestSize << "\n";
		out << "TorsoCrestSize " << TorsoCrestSize << "\n";
		out << "EcologicalRole " << EcologicalRole << "\n";
		out << "EcologicalStrategy " << EcologicalStrategy << "\n";
		out << "TimeActive " << TimeActive << "\n";
		out << "LivesInAir " << LivesInAir << "\n";
		out << "LivesOnLand " << LivesOnLand << "\n";
		out << "LivesInWater " << LivesInWater << "\n";
		out << "LivesUnderGround " << LivesUnderGround << "\n";
		out << "BodyCarriage " << BodyCarriage << "\n";
		out << "CrestSubtype " << CrestSubtype << "\n";
		out << "WeaponType " << WeaponType << "\n";
		out << "WeaponMode " << WeaponMode << "\n";
		out << "WeaponSize " << WeaponSize << "\n";
		out << "BottomSkinSubtype " << BottomSkinSubtype << "\n";
		out << "BottomSkinThickness " << BottomSkinThickness << "\n";
		out << "Speed " << Speed << "\n";
		out << "Agility " << Agility << "\n";
		out << "Strength " << Strength << "\n";
		out << "Endurance " << Endurance << "\n";
		out << "FightReflex " << FightReflex << "\n";
		out << "FlightReflex " << FlightReflex << "\n";
		file.close();
	}
}

// Returns a number from -1 to 1 that represents the current moment in the
// body's rhythm cycle, which is based on its speed.
// 'speedEnc' is speed modifier due to encumberance. It is multiplied by the natural pose speed.
float DhDNA::Rhythm(const AuPose& pose, float cycleOffset, float speedEnc) const
{
	float bodyHz = speedEnc/CycleSpeed();
	return pose.Rhythm(bodyHz, cycleOffset, AuElapsedTimer::Now());
}

bool DhDNA::FromString(const QString& dnaString)
{
	if(dnaString.length() != 63)
	{
		qWarning("DhDNA::FromString dnaString length %d is incorrect.", dnaString.length());
		return false;
	}
	bool isOk = true;	// Will be set false if a call to ValueChar fails.
	int chr = 0;		// Character index in the dnaString.
	Agility = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	Endurance = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	ArmPairs = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	BodyCarriage = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	WeaponType = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	ArmType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	CalfLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	BottomSkinSubtype = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	CalfThickness = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	BottomSkinThickness = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	CrestSubtype = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	EarSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	EcologicalRole = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	BrainLocation = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	Heads = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	LivesInAir = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	LivesInWater = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	LivesOnLand = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	LivesUnderGround = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	WeaponAltLocation = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	UNUSED0 = ValueChar(dnaString[chr++], 1, isOk); if(!isOk) return false;
	EarType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	ForeArmLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	EcologicalStrategy = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	EyeSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	HeadCrestSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	HornBranches = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	EyeType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	ForeArmThickness = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	FightReflex = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	FlightReflex = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	FootLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	FootType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	HandLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	HandType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	HornSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	LegPairs = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	LegType = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	MouthSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	MouthType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	OverallSize = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	NeckVertebrae = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	NoseSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	TailType = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	TailVertebrae = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	NoseType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	ThighLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	SkinType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	ThighThickness = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	Speed = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	Strength = ValueChar(dnaString[chr++], 15, isOk); if(!isOk) return false;
	TimeActive = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	TopSkinThickness = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	TorsoCrestSize = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	WeaponMode = ValueChar(dnaString[chr++], 3, isOk); if(!isOk) return false;
	TopSkinSubtype = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	TorsoSegmentLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	TorsoThickness = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	TorsoVertebrae = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	UpperArmLength = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	WeaponSize = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	UpperArmThickness = ValueChar(dnaString[chr++], 31, isOk); if(!isOk) return false;
	WingSubType = ValueChar(dnaString[chr++], 7, isOk); if(!isOk) return false;
	UNUSED1 = 0;
	return true;
}

QString DhDNA::ToString() const
{
	QString dnaString;
	dnaString.append(CharValue(Agility));
	dnaString.append(CharValue(Endurance));
	dnaString.append(CharValue(ArmPairs));
	dnaString.append(CharValue(BodyCarriage));
	dnaString.append(CharValue(WeaponType));
	dnaString.append(CharValue(ArmType));
	dnaString.append(CharValue(CalfLength));
	dnaString.append(CharValue(BottomSkinSubtype));
	dnaString.append(CharValue(CalfThickness));
	dnaString.append(CharValue(BottomSkinThickness));
	dnaString.append(CharValue(CrestSubtype));
	dnaString.append(CharValue(EarSize));
	dnaString.append(CharValue(EcologicalRole));
	dnaString.append(CharValue(BrainLocation));
	dnaString.append(CharValue(Heads));
	dnaString.append(CharValue(LivesInAir));
	dnaString.append(CharValue(LivesInWater));
	dnaString.append(CharValue(LivesOnLand));
	dnaString.append(CharValue(LivesUnderGround));
	dnaString.append(CharValue(WeaponAltLocation));
	dnaString.append(CharValue(UNUSED0));
	dnaString.append(CharValue(EarType));
	dnaString.append(CharValue(ForeArmLength));
	dnaString.append(CharValue(EcologicalStrategy));
	dnaString.append(CharValue(EyeSize));
	dnaString.append(CharValue(HeadCrestSize));
	dnaString.append(CharValue(HornBranches));
	dnaString.append(CharValue(EyeType));
	dnaString.append(CharValue(ForeArmThickness));
	dnaString.append(CharValue(FightReflex));
	dnaString.append(CharValue(FlightReflex));
	dnaString.append(CharValue(FootLength));
	dnaString.append(CharValue(FootType));
	dnaString.append(CharValue(HandLength));
	dnaString.append(CharValue(HandType));
	dnaString.append(CharValue(HornSize));
	dnaString.append(CharValue(LegPairs));
	dnaString.append(CharValue(LegType));
	dnaString.append(CharValue(MouthSize));
	dnaString.append(CharValue(MouthType));
	dnaString.append(CharValue(OverallSize));
	dnaString.append(CharValue(NeckVertebrae));
	dnaString.append(CharValue(NoseSize));
	dnaString.append(CharValue(TailType));
	dnaString.append(CharValue(TailVertebrae));
	dnaString.append(CharValue(NoseType));
	dnaString.append(CharValue(ThighLength));
	dnaString.append(CharValue(SkinType));
	dnaString.append(CharValue(ThighThickness));
	dnaString.append(CharValue(Speed));
	dnaString.append(CharValue(Strength));
	dnaString.append(CharValue(TimeActive));
	dnaString.append(CharValue(TopSkinThickness));
	dnaString.append(CharValue(TorsoCrestSize));
	dnaString.append(CharValue(WeaponMode));
	dnaString.append(CharValue(TopSkinSubtype));
	dnaString.append(CharValue(TorsoSegmentLength));
	dnaString.append(CharValue(TorsoThickness));
	dnaString.append(CharValue(TorsoVertebrae));
	dnaString.append(CharValue(UpperArmLength));
	dnaString.append(CharValue(WeaponSize));
	dnaString.append(CharValue(UpperArmThickness));
	dnaString.append(CharValue(WingSubType));
	Q_ASSERT(dnaString.length() == 63);	// If this fails, FromString might be broken also.
	return dnaString;
}

AuSpecies DhDNA::Species() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);
	operator>>(out);
	return AuSpecies(ba);
}

bool DhDNA::FromSpecies(const AuSpecies& species)
{
	if(species.Key().size() != sizeof(DhDNA_)+4)	// +4 because of the QByteArray wrapper.
		return false;
	QDataStream in(species.Key());
	in.setVersion(QDataStream::Qt_5_7);
	operator<<(in);
	return true;
}

void DhDNA::CreateModelDNA(Au3dsGLSLDNA& dna) const
{
	dna.OverallSize = OverallSize;
	dna.TorsoVertebrae = TorsoVertebrae;
	dna.NeckVertebrae = NeckVertebrae;
	dna.TailVertebrae = TailVertebrae;
	dna.Heads = Heads;
	dna.ArmPairs = ArmPairs;
	dna.LegPairs = LegPairs;
	dna.ArmType = ArmType;
	dna.LegType = LegType;
	dna.TailType = TailType;
	dna.TopSkinThickness = TopSkinThickness;
	dna.TopSkinSubtype = TopSkinSubtype;
	dna.SkinType = SkinType;
	dna.HandType = HandType;
	dna.FootType = FootType;
	dna.MouthType = MouthType;
	dna.MouthSize = MouthSize;
	dna.NoseType = NoseType;
	dna.NoseSize = NoseSize;
	dna.HornBranches = HornBranches;
	dna.HornSize = HornSize;
	dna.EyeSize = EyeSize;
	dna.EyeType = EyeType;
	dna.EarSize = EarSize;
	dna.EarType = EarType;
	dna.HeadCrestSize = HeadCrestSize;
	dna.TorsoCrestSize = TorsoCrestSize;
	dna.BodyCarriage = BodyCarriage;
	dna.CrestSubtype = CrestSubtype;
	dna.WeaponType = WeaponType;
	dna.WeaponMode = WeaponMode;
	dna.WeaponSize = WeaponSize;
	dna.BottomSkinSubtype = BottomSkinSubtype;
	dna.BottomSkinThickness = BottomSkinThickness;
	dna.WingSubType = WingSubType;
	dna.HandLength = HandLength;
	dna.FootLength = FootLength;
	dna.TorsoSegmentLength = TorsoSegmentLength;
	dna.TorsoThickness = TorsoThickness;
	dna.UpperArmLength = UpperArmLength;
	dna.UpperArmThickness = UpperArmThickness;
	dna.ForeArmLength = ForeArmLength;
	dna.ForeArmThickness = ForeArmThickness;
	dna.ThighLength = ThighLength;
	dna.ThighThickness = ThighThickness;
	dna.CalfLength = CalfLength;
	dna.CalfThickness = CalfThickness;
}

//static
const char* DhDNA::BoneName(BONES bone)
{
	const char* boneNames[] =
	{
		"BONE_Torso0", "BONE_Torso1", "BONE_Torso2", "BONE_Torso3", "BONE_Torso4",
		"BONE_Torso5", "BONE_Torso6",
		"BONE_Head0", "BONE_Neck00", "BONE_Neck01", "BONE_Neck02", "BONE_Neck03",
		"BONE_Head1", "BONE_Neck10", "BONE_Neck11", "BONE_Neck12", "BONE_Neck13",
		"BONE_Tail0", "BONE_Tail1", "BONE_Tail2", "BONE_Tail3", "BONE_Tail4", "BONE_Tail5",
		"BONE_Hand0L", "BONE_ForeArm0L", "BONE_UpperArm0L",
		"BONE_Hand0R", "BONE_ForeArm0R", "BONE_UpperArm0R",
		"BONE_Hand1L", "BONE_ForeArm1L", "BONE_UpperArm1L",
		"BONE_Hand1R", "BONE_ForeArm1R", "BONE_UpperArm1R",
		"BONE_Hand2L", "BONE_ForeArm2L", "BONE_UpperArm2L",
		"BONE_Hand2R", "BONE_ForeArm2R", "BONE_UpperArm2R",
		"BONE_Foot0L", "BONE_Calf0L", "BONE_Thigh0L",
		"BONE_Foot0R", "BONE_Calf0R", "BONE_Thigh0R",
		"BONE_Foot1L", "BONE_Calf1L", "BONE_Thigh1L",
		"BONE_Foot1R", "BONE_Calf1R", "BONE_Thigh1R",
		"BONE_Foot2L", "BONE_Calf2L", "BONE_Thigh2L",
		"BONE_Foot2R", "BONE_Calf2R", "BONE_Thigh2R",
		"BONE_Jaw0",
		"BONE_Jaw1",
		"BONE_Max"	// vsolarskin4m.cg shader limit is 61
	};
	return boneNames[bone];
}

AuDNABase* DhDNA::Combine(const AuDNABase& dnaBase, const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	try
	{
		const DhDNA& dna = dynamic_cast<const DhDNA&>(dnaBase);
		if(SkinType != dna.SkinType)
			return 0;				// Wrong phylum. Ew.
		if(EcologicalRole != dna.EcologicalRole)
			return 0;				// No comment.

		DhDNA_ comb;

		comb.Agility = dice.RandomRange(Agility, dna.Agility);
		comb.Endurance = dice.RandomRange(Endurance, dna.Endurance);

		comb.ArmPairs = dice.RandomRange(ArmPairs, dna.ArmPairs);
		comb.BodyCarriage = dice.RandomRange(BodyCarriage, dna.BodyCarriage);
		comb.WeaponType = dice.RandomChoice(WeaponType, dna.WeaponType);

		comb.ArmType = dice.RandomChoice(ArmType, dna.ArmType);
		comb.CalfLength = dice.RandomRange(CalfLength, dna.CalfLength);

		comb.BottomSkinSubtype = dice.RandomChoice(BottomSkinSubtype, dna.BottomSkinSubtype);
		comb.CalfThickness = dice.RandomRange(CalfThickness, dna.CalfThickness);

		comb.BottomSkinThickness = dice.RandomRange(BottomSkinThickness, dna.BottomSkinThickness);
		comb.CrestSubtype = dice.RandomChoice(CrestSubtype, dna.CrestSubtype);
		comb.EarSize = dice.RandomRange(EarSize, dna.EarSize);
		comb.EcologicalRole = EcologicalRole;

		comb.BrainLocation = dice.RandomChoice(BrainLocation, dna.BrainLocation);
		comb.Heads = dice.RandomChoice(Heads, dna.Heads);
		comb.LivesInAir = dice.RandomChoice(LivesInAir, dna.LivesInAir);
		comb.LivesInWater = dice.RandomChoice(LivesInWater, dna.LivesInWater);
		comb.LivesOnLand = dice.RandomChoice(LivesOnLand, dna.LivesOnLand);
		comb.LivesUnderGround = dice.RandomChoice(LivesUnderGround, dna.LivesUnderGround);
		comb.WeaponAltLocation = dice.RandomChoice(WeaponAltLocation, dna.WeaponAltLocation);

		comb.EarType = dice.RandomChoice(EarType, dna.EarType);
		comb.ForeArmLength = dice.RandomRange(ForeArmLength, dna.ForeArmLength);

		comb.EcologicalStrategy = dice.RandomChoice(EcologicalStrategy, dna.EcologicalStrategy);
		comb.EyeSize = dice.RandomRange(EyeSize, dna.EyeSize);
		comb.HeadCrestSize = dice.RandomRange(HeadCrestSize, dna.HeadCrestSize);
		comb.HornBranches = dice.RandomChoice(HornBranches, dna.HornBranches);

		comb.EyeType = dice.RandomChoice(EyeType, dna.EyeType);
		comb.ForeArmThickness = dice.RandomRange(ForeArmThickness, dna.ForeArmThickness);

		comb.FightReflex = dice.RandomChoice(FightReflex, dna.FightReflex);
		comb.FlightReflex = dice.RandomChoice(FlightReflex, dna.FlightReflex);

		comb.FootLength = dice.RandomRange(FootLength, dna.FootLength);
		comb.FootType = dice.RandomChoice(FootType, dna.FootType);

		comb.HandLength = dice.RandomRange(HandLength, dna.HandLength);
		comb.HandType = dice.RandomChoice(HandType, dna.HandType);

		comb.HornSize = dice.RandomRange(HornSize, dna.HornSize);
		comb.LegPairs = dice.RandomRange(LegPairs, dna.LegPairs);
		comb.LegType = dice.RandomChoice(LegType, dna.LegType);
		comb.MouthSize = dice.RandomRange(MouthSize, dna.MouthSize);

		comb.MouthType = dice.RandomChoice(MouthType, dna.MouthType);
		comb.OverallSize = dice.RandomRange(OverallSize, dna.OverallSize);

		comb.NeckVertebrae = dice.RandomRange(NeckVertebrae, dna.NeckVertebrae);
		comb.NoseSize = dice.RandomRange(NoseSize, dna.NoseSize);
		comb.TailType = dice.RandomChoice(TailType, dna.TailType);
		comb.TailVertebrae = dice.RandomRange(TailVertebrae, dna.TailVertebrae);

		comb.NoseType = dice.RandomChoice(NoseType, dna.NoseType);
		comb.ThighLength = dice.RandomRange(ThighLength, dna.ThighLength);

		comb.SkinType = SkinType;
		comb.ThighThickness = dice.RandomRange(ThighThickness, dna.ThighThickness);

		comb.Speed = dice.RandomRange(Speed, dna.Speed);
		comb.Strength = dice.RandomRange(Strength, dna.Strength);

		comb.TimeActive = dice.RandomChoice(TimeActive, dna.TimeActive);
		comb.TopSkinThickness = dice.RandomRange(TopSkinThickness, dna.TopSkinThickness);
		comb.TorsoCrestSize = dice.RandomRange(TorsoCrestSize, dna.TorsoCrestSize);
		comb.WeaponMode = dice.RandomChoice(WeaponMode, dna.WeaponMode);

		comb.TopSkinSubtype = dice.RandomChoice(TopSkinSubtype, dna.TopSkinSubtype);
		comb.TorsoSegmentLength = dice.RandomRange(TorsoSegmentLength, dna.TorsoSegmentLength);

		comb.TorsoThickness = dice.RandomRange(TorsoThickness, dna.TorsoThickness);
		comb.TorsoVertebrae = dice.RandomRange(TorsoVertebrae, dna.TorsoVertebrae);

		comb.UpperArmLength = dice.RandomRange(UpperArmLength, dna.UpperArmLength);
		comb.WeaponSize = dice.RandomRange(WeaponSize, dna.WeaponSize);

		comb.UpperArmThickness = dice.RandomRange(UpperArmThickness, dna.UpperArmThickness);
		comb.WingSubType = dice.RandomChoice(WingSubType, dna.WingSubType);

		return new DhDNA(comb);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return 0;
	}
};

float DhDNA::HearingRange() const
{
	return 25*FeetPerMeter() + EarSize*10*FeetPerMeter();
}

QString DhDNA::WeaponImageName() const
{
	QString names[] =
	{
		"",
		":data/wl_Anus.png",
		":data/wl_Eye.png",
		":data/wl_Foot.png",
		":data/wl_Hand.png",
		":data/wl_Horns.png",
		":data/wl_Mouth.png",
		":data/wl_Tail.png",
		":data/wl_Torso.png",
	};
	int imageIdx;
	switch(WeaponLocation())
	{
	case WL_None:
	default:
		imageIdx = 0;
		break;
	case WL_Anus:
		imageIdx = 1;
		break;
	case WL_Eyes:
		imageIdx = 2;
		break;
	case WL_Feet:
		imageIdx = 3;
		break;
	case WL_Hands:
		imageIdx = 4;
		break;
	case WL_Head:
		if(HornSize > MouthSize)
			imageIdx = 5;	// Horns
		else
			imageIdx = 6;	// Mouth
		break;
	case WL_Tail:
		imageIdx = 7;
		break;
	case WL_Torso:
		imageIdx = 8;
		break;
	}
	return names[imageIdx];
}

QString DhDNA::WeaponLocationName() const
{
	QString names[] =
	{
		QObject::tr("None"),
		QObject::tr("Anus"),
		QObject::tr("Eyes"),
		QObject::tr("Feet"),
		QObject::tr("Hands"),
		QObject::tr("Horns"),
		QObject::tr("Mouth"),
		QObject::tr("Tail"),
		QObject::tr("Torso"),
	};
	int imageIdx;
	switch(WeaponLocation())
	{
	case WL_None:
	default:
		imageIdx = 0;
		break;
	case WL_Anus:
		imageIdx = 1;
		break;
	case WL_Eyes:
		imageIdx = 2;
		break;
	case WL_Feet:
		imageIdx = 3;
		break;
	case WL_Hands:
		imageIdx = 4;
		break;
	case WL_Head:
		if(HornSize > MouthSize)
			imageIdx = 5;	// Horns
		else
			imageIdx = 6;	// Mouth
		break;
	case WL_Tail:
		imageIdx = 7;
		break;
	case WL_Torso:
		imageIdx = 8;
		break;
	}
	return names[imageIdx];
}

bool DhDNA::IsSexuallyCompatible(const AuDNABase& dna) const
{
	try
	{
		const DhDNA& c = dynamic_cast<const DhDNA&>(dna);
		if(SkinType != c.SkinType)
			return false;				// Wrong phylum. Ew.
		if(EcologicalRole != c.EcologicalRole)
			return false;				// No comment.
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

bool DhDNA::HasBone(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	return butcheringWhat == CID_Animal || butcheringWhat == CID_Head;
}

bool DhDNA::HasBrain(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	if(butcheringWhat == CID_Animal || butcheringWhat == CID_Fish)
		return BrainLocation == BRAIN_Torso;
	else if(butcheringWhat == CID_Head)
		return BrainLocation == BRAIN_Head;
	return false;
}

bool DhDNA::HasGills(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	Q_UNUSED(butcheringWhat);
	if(butcheringWhat != CID_Animal && butcheringWhat != CID_Fish)
		return false;
	return LivesInWater;
}

bool DhDNA::HasHeart(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	return butcheringWhat == CID_Animal || butcheringWhat == CID_Fish;
}

bool DhDNA::HasLeather(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	return butcheringWhat == CID_Animal;
}

bool DhDNA::HasLungs(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	if(butcheringWhat != CID_Animal && butcheringWhat != CID_Fish)
		return false;
	return LivesInAir || LivesOnLand || LivesUnderGround;
}

bool DhDNA::HasStomach(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	if(butcheringWhat != CID_Animal && butcheringWhat != CID_Fish)
		return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const DhDNA& dna)
{
	dbg.nospace() << dna.ToString() << "~" << dna.Description();
	return dbg.space();
}
