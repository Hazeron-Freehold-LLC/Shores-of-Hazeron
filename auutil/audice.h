//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audice.h,v 1.27 2022/02/19 18:42:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDice_H
#define AuDice_H

#include "auutil.h"
#include "autime.h"

class AUUTIL_DLL AuDice
{
	quint32								m_Next;

public:
	inline AuDice()
	{
		m_Next = 1;
	}

	inline AuDice(quint32 seed)
	{
		m_Next = seed;
	}

	inline AuDice(const AuDice& c)
	{
		m_Next = c.m_Next;
	}

	inline ~AuDice()
	{
	}

public:	// Operators
	inline const AuDice& operator=(const AuDice& c)
	{
		m_Next = c.m_Next;
		return *this;
	}

public:	// Methods
	inline quint32 Random()
	{
		// NOTE: DO NOT CHANGE THESE NUMBERS! ALL "SEED GENERATED" RANDOM DATA IN HAZERON WILL BE AFFECTED.
		m_Next = m_Next * 1103515245 + 12345;
		return m_Next;
	}

	QString RandomBuildingName();

	// Returns a randomly generated name for a city.
	// A city name is a RandomWord with an upper case first letter and possibly a city-esque prefix and/or suffix.
	QString RandomCityName();

	// Returns v1 or v2, chosen randomly.
	inline qint32 RandomChoice(qint32 v1, qint32 v2)
	{
		if(v1 == v2 || Roll(1,2) == 1)
			return v1;
		else
			return v2;
	}

	// Returns a random value from a Normal Gaussian Distribution.
	double RandomGaussianDistribution(double mean, double standardDeviation);

	// Generates a quality value (1 to 255) based on a statistical standard normal distribution.
	quint8 RandomQuality();

	// Range of possible return values is based on a logarithmic progression in size away from 'value', lower and higher.
	// The returned value could fall into a different species increment, higher or lower.
	quint16 RandomLogValue(quint16 value, quint16 maxPossibleValue);

	inline float RandomMinMax(float fMin, float fMax, float fTotalMax=0xffffffff)
	{
		float fRandNum = (float)Random() / fTotalMax;
		return fMin + (fMax - fMin) * fRandNum;
	}

	// A name is a RandomWord with an upper case first letter.
	QString RandomName();

	// Returns a randomly generated password containing only upper and lower case letters and numeric digits.
	// 'length' is the length of the password returned.
	QString RandomPassword(int length);

	// Returns a randomly generated name for a person.
	QString RandomPersonName();

	QString RandomGeoName();

	// Returns a number randomly in the range v1 to v2, inclusive.
	inline qint32 RandomRange(qint32 v1, qint32 v2)
	{
		if(v1 == v2)
			return v1;
		else if(v1 > v2)
		{
			qint32 range = v1 - v2 + 1;	// Inclusive range.
			return Roll(1, range, v2 - 1);
		}
		else	// if(v2 > v1)
		{
			qint32 range = v2 - v1 + 1;	// Inclusive range.
			return Roll(1, range, v1 - 1);
		}
	}

	QString RandomSectorName();

	QString RandomShipName();

	QString RandomSystemName();

	// Returns a randomly generated word. All characters are lower case.
	QString RandomWord();

	QString RandomWorldName();

	// Roll dice and return the sum of their rolls
	// 'lCount' is the number of dice to roll
	// 'lFaces' is the number of faces per die
	inline qint32 Roll(unsigned int lCount, unsigned int lFaces)
	{
		if(!lCount)
			return 0;

		// Must roll the random number generator even with 0 or 1 sided dice because of seeded sequences that must not change.
		// roll them bones
		qint32 lResult = 0;
		for(unsigned int die=0; die<lCount; die++)
		{
			qint32 lRoll = 1 + (qint32)((double)lFaces*Random() / ((quint32)0xffffffff + 1.));
			lResult += lRoll;
		}

		// Insure proper result when faces is illogical or 1.
		if(!lFaces)
			lResult = 0;
		else if(lFaces == 1)
			lResult = lCount;

		return lResult;
	}

	// Roll dice and return the sum of their rolls
	// 'lCount' is the number of dice to roll
	// 'lFaces' is the number of faces per die
	// 'lModifier' is a +/- modifier to the resulting sum of the dice rolls
	inline qint32 Roll(unsigned int lCount, unsigned int lFaces, int lModifier)
	{
		if(!lCount)
			return 0;

		// Must roll the random number generator even with 0 or 1 sided dice because of seeded sequences that must not change.
		// roll them bones
		qint32 lResult = 0;
		for(unsigned int die=0; die<lCount; die++)
		{
			qint32 lRoll = 1 + (qint32)((double)lFaces*Random() / ((quint32)0xffffffff + 1.));
			lResult += lRoll;
		}

		// Insure proper result when faces is illogical or 1.
		if(!lFaces)
			lResult = 0;
		else if(lFaces == 1)
			lResult = lCount;

		// apply the mods
		if(lModifier)
			lResult += lModifier;

		return lResult;
	}

	inline void Seed(quint32 seed)
	{
		m_Next = seed;
	}

	inline void TimeSeed()				// Seeds with current time value.
	{
		quint32 lSeed = (quint32)AuTime::CurrentMSecsSinceEpoch();
		Seed(lSeed);
	}
};

#endif // AuDice_H
