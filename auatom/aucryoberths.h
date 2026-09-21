//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucryoberths.h,v 1.5 2022/02/19 17:30:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCryoBerths_H
#define AuCryoBerths_H

#include "auatom.h"
#include "auspecies.h"

// Key is SpeciesKey of cryo traveller(s). Value is count of cryo travellers.
class AuCryoBerths : public QMap<AuSpecies, quint32>
{
public:	// Construction
	AuCryoBerths();
	virtual ~AuCryoBerths();
	AuCryoBerths(const AuCryoBerths& c);

public:	// Operators
	const AuCryoBerths& operator=(const AuCryoBerths& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	void AddOccupant(const AuSpecies& species);

	inline quint32 CountOccupants() const
	{
		quint32 o = 0;
		for(const_iterator bi = cbegin(); bi != cend(); bi++)
			o += *bi;
		return o;
	}

	// Returns species key if one is removed.
	AuSpecies RemoveAnyOccupant();

	// Removes one occupant with 'dna'.
	// Returns true if one is removed.
	bool RemoveOccupant(const AuSpecies& species);

	// Removes 'howMany' occupants with 'dna'.
	// Returns the number actually removed.
	quint32 RemoveOccupants(const AuSpecies& species, quint32 howMany);
};

inline QDataStream& operator<<(QDataStream& out, const AuCryoBerths& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCryoBerths& o)
{
	o << in;
	return in;
}

#endif // AuCryoBerths_H
