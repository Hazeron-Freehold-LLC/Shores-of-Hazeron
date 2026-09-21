//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnabase.h,v 1.8 2022/02/19 18:19:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNABase_H
#define AuDNABase_H

#include "audata.h"
#include "audef.h"
#include "auspecies.h"
#include "dbcommodity.h"

class AuDNABase
{
public:
	enum CHROMATID
	{
		Alpha,
		Beta
	};

	enum ECOSTRATEGY
	{
		ECO_Waiter,						// Waits for food to get within range. Rarely moves. spiders, ant lion, anemone, crocodile
		ECO_Grazer,						// Eats continuously while moving. Army ant, cow, slug, snail
		ECO_Hunter,						// Moves continuously. Stops to eat.
		ECO_Sneaker,					// Moves intermittently. Stops to eat.
	};

	enum DNAWEAPONMODE
	{
		DWM_Area,			// Weapon has an area affect when discharged.
		DWM_Contact,		// Weapon must physically contact the target.
		DWM_Projectile,		// Weapon fires some form of shot.
		DWM_Psionic,		// Weapon inflicts damage telepathically.
	};

	enum WEAPONLOC
	{
		WL_None,
		WL_Anus,
		WL_Eyes,
		WL_Feet,
		WL_Hands,
		WL_Head,
		WL_Tail,
		WL_Torso,
	};

	// These are the weapon types found naturally in creatures.
	// These weapon types MUST match the first 16 WEAPONEFFECTs in AuDef...and ITEMEFFECTS
	// They are duplicated because DhDNA::WeaponType is only 4 bits wide.
	enum WEAPONTYPE
	{
		WT_None,
		WT_Acid,
		WT_Suffocate,
		WT_Stun,
		WT_Poison,
		WT_Radiation,
		WT_Disease,
		WT_Electric,
		WT_Bludgeon,
		WT_Cold,
		WT_Crush,
		WT_Sonic,
		WT_Heat,
		WT_Pierce,
		WT_Implant,
		WT_Slash,
	};

public:	// Construction
	AuDNABase();
	virtual ~AuDNABase();

public:	// Operators
	virtual bool operator==(const AuDNABase& c) const = 0;

	inline bool operator!=(const AuDNABase& c) const
	{
		return !operator==(c);
	}

	virtual void operator>>(QDataStream& out) const = 0;

	virtual void operator<<(QDataStream& in) = 0;

public:	// Methods
	virtual AuDNABase* Clone() const = 0;

	// Returns the result of combining this DNA with 'dna'. Caller takes ownership of the pointer.
	// Returns 0 if they cannot be combined.
	virtual AuDNABase* Combine(const AuDNABase& dna, const AuDice& dice) const = 0;

	// Creates a new DNA.
	// Caller must delete the pointer.
	static AuDNABase* CreateDNA(quint8 dnaVersion);

	virtual ECOROLE EcoRole() const = 0;

	// Constructs this DNA from 'speciesKey'. The result is randomized between calls to produce variations within a species.
	// Returns false if the 'speciesKey' is invalid.
	virtual bool FromSpecies(const AuSpecies& species) = 0;

	virtual GENDER Gender(CHROMATID chroma) const = 0;

	virtual int GestateBabyDays() const = 0;

	virtual int GestateImplantHours() const = 0;

	virtual bool HasBone(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasBrain(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasGills(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasHeart(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasLeather(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasLungs(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool HasStomach(CHROMATID chroma, COMMODITYID butcheringWhat) const = 0;

	virtual bool IsSexuallyCompatible(const AuDNABase& c) const = 0;

	virtual int MinutesToMature() const = 0;

	// Loads a versioned DNA from a stream and returns its pointer.
	// Use in conjunction with WriteDynamic.
	// Caller must delete the pointer.
	static AuDNABase* ReadDynamic(QDataStream& in);

	virtual QString SpeciesDescription() const = 0;

	// Returns a string that can uniquely identify the species. This is a compact encoded string that is not humanly readable.
	virtual AuSpecies Species() const = 0;

	virtual QString TimeActiveName() const = 0;

	// 3=DhDNA, 4=AuDNA
	virtual quint8 Version() const = 0;

	// Writes a versioned DNA to a stream.
	// Use in conjunction with ReadDynamic.
	void WriteDynamic(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuDNABase& dna)
{
	dna >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDNABase& dna)
{
	dna << in;
	return in;
}

#endif // AuDNABase_H
