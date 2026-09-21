//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucryoberths.cpp,v 1.4 2022/02/19 17:39:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucryoberths.h"
//

AuCryoBerths::AuCryoBerths()
{
}

AuCryoBerths::~AuCryoBerths()
{
}

AuCryoBerths::AuCryoBerths(const AuCryoBerths& c) : QMap<AuSpecies, quint32>(c)
{
}

const AuCryoBerths& AuCryoBerths::operator=(const AuCryoBerths& c)
{
	QMap<AuSpecies, quint32>::operator=(c);
	return *this;
}

void AuCryoBerths::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator bi = cbegin(); bi != cend(); bi++)
	{
		out << bi.key();
		out << *bi;
	}
}

void AuCryoBerths::operator<<(QDataStream& in)
{
	clear();

	quint16 count;
	in >> count;
	while(count--)
	{
		AuSpecies key;
		in >> key;
		quint32 pop;
		in >> pop;

		insert(key, pop);
	}
}

quint32 AuCryoBerths::RemoveOccupants(const AuSpecies& species, quint32 howMany)
{
	// Find a match.
	iterator bi = find(species);
	if(bi == end())
		return 0;
	if(*bi <= howMany)
	{
		// Remove the entire entry.
		howMany = *bi;
		erase(bi);
	}
	else
	{
		// Remove a portion of the entry.
		*bi -= howMany;
	}
	return howMany;
}

void AuCryoBerths::AddOccupant(const AuSpecies& species)
{
	// Find a match.
	iterator bi = find(species);
	if(bi != end())
		(*bi)++;
	else
		insert(species, 1);
}

bool AuCryoBerths::RemoveOccupant(const AuSpecies& species)
{
	// Find a match.
	iterator bi = find(species);
	if(bi == end())
		return false;
	if(*bi <= 1)
		erase(bi);
	else
		(*bi)--;
	return true;
}

AuSpecies AuCryoBerths::RemoveAnyOccupant()
{
	iterator bi = begin();
	if(bi == end())
		return AuSpecies();
	AuSpecies species = bi.key();
	if(*bi <= 1)
		erase(bi);
	else
		(*bi)--;
	return species;
}

