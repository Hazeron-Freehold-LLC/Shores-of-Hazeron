//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufloradna.h,v 1.9 2022/02/19 18:30:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFloraDNA_H
#define AuFloraDNA_H

#include "augloballib.h"
#include <QColor>
#include <QDataStream>
#include <QString>

class AuDice;

class AUGLOBAL_DLL AuFloraDNA
{
public:
	// DNA Values
	// These are the chemical defenses found naturally in plants.
	// These chemical defenses MUST match the first 8 WEAPONEFFECTs in AuDef.
	// They are duplicated because AuFloraDNA::Chemical is only 3 bits wide.
	enum CHEMICAL
	{
		CHEM_None,			// No chemical defenses.
		CHEM_Acid,
		CHEM_Suffocate,
		CHEM_Stun,
		CHEM_Poison,
		CHEM_Radiation,
		CHEM_Disease,
		CHEM_Electric
	};
	enum INFLORESCENCE
	{
		INFL_Spike,			// Flowers attach directly to the trunk.
		INFL_Raceme,		// Flowers attach to trunk with short stems.
		INFL_Corymb,		// Flattish or convex flower cluster, each on own stalk.
							// Outermost blossoms mature before inner blossoms.
		INFL_Panicle,		// Loosely branched. Gradually simpler toward end.
		INFL_UmbelSimple,	// Flower cluster emanates from a common point.
		INFL_UmbelCompound,	// Each peduncle bears another little umbel, called umbellet
							// or umbellule.
		INFL_Cyme,			// Flattish or convex flower cluster. Differs from corym
							// chiefly in the order of blossoms opening, inner to outer.
		INFL_ScorpioidCyme,	// A form of monochasium in which the lateral branches arise
							// alternately on opposite sides of the false axis.
	};
	enum LEAF
	{
		LEAF_ParallelLonger,	// Parallel style, veins get longer from stem to tip.
		LEAF_ParallelSame,		// Parallel style, vein length stays the same to tip.
		LEAF_ParallelShorter,	// Parallel style, veins get shorter from stem to tip.
		LEAF_RadialLonger,		// Radial style, veins longer as angle off tip grows.
		LEAF_RadialSame,		// Radial style, vein length stays the same radially.
		LEAF_RadialShorter,		// Radial style, veins shorter as angle off tip grows.
		LEAF_BowEnds,			// Bow style, veins start and end at ends.
		LEAF_BowStem,			// Bow style, veins start at stem and end along leaf.
		// Gave up this one to fit leaf styles into three bits rather than come up with
		// seven more.
		//LEAF_BowTip,			// Bow style, veins start along leaf and end at tip.
	};
	enum OVARY
	{
		OVARY_Inferior,
		OVARY_Superior,
	};
	enum ROOT
	{
		Bulb,				// Top of bulb visible.
		Fibrous,			// Wide patch of low thin roots.
		Prop,				// Conical patch of thick roots.
		Tap					// No apparent roots.
	};

	enum ZONE
	{
		Polar,
		Temperate,
		Tropical,
		Zones			// MUST BE LAST ENTRY
	};

public:
	// These bit fields are arranged to insure they break nicely on byte
	// boundaries.
	// They are used to drive a branching algorithm for painting a branch. In
	// that
	// algorithm, successive branches get smaller according to branch taper. The
	// terminating branch segment becomes a leaf cluster or a flower/flower
	// cluster.
	unsigned int	Inflorescence : 3;		// INFLORESCENCE Flower cluster arrangement.
	unsigned int	TrunkLength : 5;		// 1..32 + 3 = Length of first plant segment.

	unsigned int	Multiples : 2;			// 1..4 Multiple for leaves, petals, etc.
	unsigned int	SproutMultiples : 3;	// 0=1 sprout, 1-3=1-3 multiples
	unsigned int	BranchAngle : 2;		// 5-20 degrees * branch index at
											// node
	unsigned int	Unused0 : 1;

	unsigned int	BranchProbability : 2;	// 0..3=100% Test each branch of
											// multiple.
	unsigned int	RootType : 2;			// 0..3 ROOT Style of visible
											// portion of root.
	unsigned int	RootSize : 2;			// 0..3 Relative size of root to
											// trunk.
	unsigned int	LeafProbability : 2;	// 1..4=100% Test at each node.

	unsigned int	LeafType : 3;			// LEAF Vein pattern.
	unsigned int	LeafSize : 3;			// Leaf length = Branch
											// length/(LeafSize+.7)
	unsigned int	LeafVeinCount : 2;		// 0..3 Veins on each side of
											// primary vein.

	unsigned int	LeafVeinWidth : 3;		// Width of plant flesh along veins.
	unsigned int	Chemical : 3;			// CHEMICAL Chemical defense of
											// plant. May be none.
	unsigned int	ChemicalStrength : 2;	// 1..4 Strength of chemical defense

	unsigned int	InternodeSizeRatio : 4;	// width = trunk length/(InternodeSizeRatio+2)
	unsigned int	BarkTexture : 3;		// Texture to use for woody parts.
	unsigned int	PetalMultiples : 1;		// 2..3 Multiples of Petals.

	unsigned int	PetalLength : 3;		// 0..7 Length of petal.
	unsigned int	PetalTaper : 2;			// 0..3 Stem to taper-in distance.
	unsigned int	PetalWidth : 3;			// 0..7 Width of petal. 0=.5

	unsigned int	FlowerProbability : 2;	// 0..3 Test for flower at a tip.
											// 0=No flwrs
	unsigned int	FlowerPetalRed : 3;		// 1..8 Red color intensity.
	unsigned int	FlowerPetalGreen : 3;	// 1..8 Green color intensity.

	unsigned int	FlowerPetalBlue : 3;	// 1..8 Blue color intensity.
	unsigned int	FlowerOvarySize : 2;	// 1..4 Fraction of petal length
	unsigned int	FlowerOvaryRed : 3;		// 0..7 Red color intensity.

	unsigned int	FlowerOvaryGreen : 3;	// 0..7 Green color intensity.
	unsigned int	FlowerOvaryBlue : 3;	// 0..7 Blue color intensity.
	unsigned int	LeafStemLength : 2;		// 1..4 Fraction of the vein width.

	unsigned int	FlowerOvary : 1;		// OVARY 0..1 0=Inferior, 1=Superior
	unsigned int	ThornSize : 2;			// 0..3 0=None, 1=Hairs, 2,3=Harmful
	unsigned int	ThornFreq : 2;			// 1..4 Interval along branches.
	unsigned int	Stiffness : 3;			// Stiffness factor of entire plant.
											// Stiffness determines branch
											// taper, 15-50%

public:
	AuFloraDNA();

	inline bool operator==(const AuFloraDNA& c) const
	{
		if(Inflorescence != c.Inflorescence)
			return false;
		if(TrunkLength != c.TrunkLength)
			return false;
		if(Multiples != c.Multiples)
			return false;
		if(SproutMultiples != c.SproutMultiples)
			return false;
		if(BranchAngle != c.BranchAngle)
			return false;
		if(BranchProbability != c.BranchProbability)
			return false;
		if(RootType != c.RootType)
			return false;
		if(RootSize != c.RootSize)
			return false;
		if(LeafProbability != c.LeafProbability)
			return false;
		if(LeafType != c.LeafType)
			return false;
		if(LeafSize != c.LeafSize)
			return false;
		if(LeafVeinCount != c.LeafVeinCount)
			return false;
		if(LeafVeinWidth != c.LeafVeinWidth)
			return false;
		if(Chemical != c.Chemical)
			return false;
		if(ChemicalStrength != c.ChemicalStrength)
			return false;
		if(InternodeSizeRatio != c.InternodeSizeRatio)
			return false;
		if(BarkTexture != c.BarkTexture)
			return false;
		if(PetalMultiples != c.PetalMultiples)
			return false;
		if(PetalLength != c.PetalLength)
			return false;
		if(PetalTaper != c.PetalTaper)
			return false;
		if(PetalWidth != c.PetalWidth)
			return false;
		if(FlowerProbability != c.FlowerProbability)
			return false;
		if(FlowerPetalRed != c.FlowerPetalRed)
			return false;
		if(FlowerPetalGreen != c.FlowerPetalGreen)
			return false;
		if(FlowerPetalBlue != c.FlowerPetalBlue)
			return false;
		if(FlowerOvarySize != c.FlowerOvarySize)
			return false;
		if(FlowerOvaryRed != c.FlowerOvaryRed)
			return false;
		if(FlowerOvaryGreen != c.FlowerOvaryGreen)
			return false;
		if(FlowerOvaryBlue != c.FlowerOvaryBlue)
			return false;
		if(LeafStemLength != c.LeafStemLength)
			return false;
		if(FlowerOvary != c.FlowerOvary)
			return false;
		if(ThornSize != c.ThornSize)
			return false;
		if(ThornFreq != c.ThornFreq)
			return false;
		if(Stiffness != c.Stiffness)
			return false;
		return true;
	}

	inline bool operator!=(const AuFloraDNA& c) const
	{
		return !operator==(c);
	}

	inline bool operator<(const AuFloraDNA& c) const
	{
		return BitSum() < c.BitSum();
	}

	inline bool operator>(const AuFloraDNA& c) const
	{
		return BitSum() > c.BitSum();
	}

public:
	inline unsigned int BitSum() const
	{
		return BarkTexture +
			BranchAngle +
			BranchProbability +
			Chemical +
			ChemicalStrength +
			FlowerOvary +
			FlowerOvaryBlue +
			FlowerOvaryGreen +
			FlowerOvaryRed +
			FlowerOvarySize +
			FlowerPetalBlue +
			FlowerPetalGreen +
			FlowerPetalRed +
			FlowerProbability +
			Inflorescence +
			InternodeSizeRatio +
			LeafProbability +
			LeafSize +
			LeafStemLength +
			LeafType +
			LeafVeinCount +
			LeafVeinWidth +
			Multiples +
			PetalLength +
			PetalMultiples +
			PetalTaper +
			PetalWidth +
			RootSize +
			RootType +
			SproutMultiples +
			Stiffness +
			ThornFreq +
			ThornSize +
			TrunkLength;
	}

	QString Description() const;

	void Init(ZONE zone, const AuDice& dice, int sizemod);

	inline float BranchLength(float baseDiameter) const
	{
		float brlen = baseDiameter * ((float)InternodeSizeRatio + 1.5f);
		return brlen;
	}

	inline float BranchBendFactor() const
	{
		// Returns a value to taper each branch segment from
		// starting diameter to ending diameter.
		// i.e. end dia = start dia * taper factor.
		// Stiffness determines branch taper, 15-50%
		// Higher stiffness results in a steeper taper, which makes the plant
		// stockier. A low taper factor results in long slender limbs.
		// Range is .05 to .26
		float taperFactor = .05f + (float)Stiffness * .03f;
		return taperFactor;
	}

	inline float BranchTaperFactor() const
	{
		// Returns a value to taper each branch segment from
		// starting diameter to ending diameter.
		// i.e. end dia = start dia * taper factor.
		// Stiffness determines branch taper, 15-50%
		// Higher stiffness results in a steeper taper, which makes the plant
		// stockier. A low taper factor results in long slender limbs.
		// Range is .055 to .3
		float taperFactor = .055f + (float)Stiffness * .035f;
		return taperFactor;
	}

	// Radioactive plants glow.
	inline QColor Glow() const
	{
		return Glow(0);
	}

	// All plants glow on worlds with a background flora glow factor.
	inline QColor Glow(int addedStrength) const
	{
		// Plants with a radioactive defense will glow.
		float glowR;
		float glowG;
		float glowB;
		if(Chemical == CHEM_Radiation)
		{
			// Radioactive plants always glow.
			if(FlowerPetalRed || FlowerPetalGreen || FlowerPetalBlue)
			{
				// Glow in the flower petal color as long as it is not black.
				glowR = ((float)ChemicalStrength + 1 + addedStrength) * .1f * FlowerPetalRed / 7;
				glowG = ((float)ChemicalStrength + 1 + addedStrength) * .1f * FlowerPetalGreen / 7;
				glowB = ((float)ChemicalStrength + 1 + addedStrength) * .1f * FlowerPetalBlue / 7;
			}
			else
			{
				// Green glow.
				glowG = ((float)ChemicalStrength + 1 + addedStrength) * .1f;
				glowR = glowG * .05f/.3f;
				glowB = glowR;
			}
		}
		else if(addedStrength)
		{
			// All plants glow when added strength is provided.
			if(FlowerPetalRed || FlowerPetalGreen || FlowerPetalBlue)
			{
				// Glow in the flower petal color as long as it is not black.
				glowR = ((float)addedStrength) * .1f * FlowerPetalRed / 7.f;
				glowG = ((float)addedStrength) * .1f * FlowerPetalGreen / 7.f;
				glowB = ((float)addedStrength) * .1f * FlowerPetalBlue / 7.f;
			}
			else
			{
				// Green glow.
				glowG = ((float)addedStrength) * .1f;
				glowR = glowG * .05f/.3f;
				glowB = glowR;
			}
		}
		else
		{
			// No glow.
			glowR = 0;
			glowG = 0;
			glowB = 0;
		}

		// Return the result.
		QColor glowColor;
		glowColor.setRgbF(glowR, glowG, glowB);
		return glowColor;
	}

	inline bool IsUmbel() const
	{
		// Umbel expansion sometimes takes a long time.
		//if(Inflorescence == INFL_UmbelSimple ||
				//Inflorescence == INFL_UmbelCompound)
			//return true;
		return false;
	}

	inline float LeafLength(float branchLength) const
	{
		// .75x to 1.75x branch length
		float szfactor = (float)LeafSize/7.f + .75;
		float leafLen = branchLength * szfactor;
		return leafLen;
	}

	QString ListName() const;

	inline float MinBaseDiameter() const
	{
		// Return minimum base diameter so taper to next node will end with enough diameter for another branch.
		float minBase = MinBranchDiameter() / BranchTaperFactor() + 0.05f;
		if(minBase < 2)
			return 2;		// Minimum spread so we don't make tiny plants.
		return minBase;
 	}

	float MinBranchDiameter() const;

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline QColor OvaryColor() const
	{
		// Shift 3-bit values up to 8 bits for bright colors.
		int r = FlowerOvaryRed << 5;
		int g = FlowerOvaryGreen << 5;
		int b = FlowerOvaryBlue << 5;
		return QColor(r,g,b);
	}

	inline QColor PetalColor() const
	{
		// Shift 3-bit values up to 8 bits for bright colors.
		int r = FlowerPetalRed << 5;
		int g = FlowerPetalGreen << 5;
		int b = FlowerPetalBlue << 5;
		return QColor(r,g,b);
	}

	inline float PetalSizeIncrement(float branchDia, int branchNodes) const
	{
		// Return 0 if flowers are not possible.
		if(FlowerProbability == 0)
			return 0.f;

		// PetalLength equates to 0 to 1.4x branch length.
		float petalInc = branchDia/2.;

		// On Corymb, flowers get larger as node count increases.
		if(Inflorescence == INFL_Corymb)
		{
			// branchNodes=0 is passed for the trunk, which results in the
			// smallest flower size on Corymb.
			// Note that branchDia diminishes as node count increases. The
			// multiplier here needs to overcome that reduction and then some.
			petalInc *= 1.f + (float)branchNodes * .025f;
		}

		return petalInc;
	}

	inline float PetalWidthCalc(float petalSizeInc) const
	{
		if(PetalWidth == 0)
			return petalSizeInc * .5f;
		else
			return petalSizeInc * PetalWidth;
	}

	void RxUpdate(QDataStream& fromServer);

	inline float TrunkBaseDiameter() const
	{
		// Calculate diameter based on trunk length to internode size ratio.
		float dia = (float)(TrunkLength + 1) / (float)(InternodeSizeRatio + 2);

		// Trunk must not taper to less than the min branch diameter or it won't have any foliage at the tip.
		float minbasedia = MinBaseDiameter();
		if(minbasedia > dia)
			return minbasedia;
		return dia;
	}

	void TxUpdate(QDataStream& toClient) const;
};

Q_DECLARE_METATYPE(AuFloraDNA)	// Needed to this use class in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuFloraDNA& dna)
{
	dna >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFloraDNA& dna)
{
	dna << in;
	return in;
}

AUGLOBAL_DLL QDebug operator<<(QDebug dbg, const AuFloraDNA& dna);

#endif // AuFloraDNA_H
