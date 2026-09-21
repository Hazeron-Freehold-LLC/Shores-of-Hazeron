//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: defauna.h,v 1.26 2022/02/19 17:35:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DePlanetFauna - Describes the various creatures found on a habitable planet.
// DeTerrainFauna - Describes the various creatures found in a terrain type.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#ifndef DeFauna_H
#define DeFauna_H

#include "dhdna.h"

class DeFaunaEncounter
{
private:
	DhDNA_			m_DNA;			// The DNA of the creature.
	char			m_GroupSize;	// Max number of creatures in group.

public:
	DeFaunaEncounter();
	~DeFaunaEncounter();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	const DhDNA_& DNA() const
	{
		return m_DNA;
	}

	int GroupSize() const
	{
		return m_GroupSize;
	}

	void Init(bool isRingworld, bool isGasGiant, quint16 radius100km, quint8 waterPercent, ORBITZONE orbitZone, quint8 atmDensity,
			TERRAIN tertype, TOD tod, ECOROLE role, const AuDice& dice);

	bool IsValid() const
	{
		return (m_GroupSize >= 1 && m_GroupSize <= 6) ? true : false;
	}

	void NewGroupSize();			// Init size of nu group of this encounter.

	void RxUpdate(QDataStream& fromServer);

	void TxUpdate(QDataStream& toClient) const;
};

inline QDataStream& operator<<(QDataStream& out, const DeFaunaEncounter& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeFaunaEncounter& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DeTerrainFauna
{
public:
	DeFaunaEncounter	m_Encounters[7];	// Each position of a 2D4-1 encounter table.

public:
	DeTerrainFauna();
	~DeTerrainFauna();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	bool HasAerialCreatures() const;

	void Init(bool isRingworld, bool isGasGiant, quint16 radius100km, quint8 waterPercent, ORBITZONE orbitZone, quint8 atmDensity,
			TERRAIN tertype, TOD tod, const AuDice& dice);

	DeFaunaEncounter NewAerialEncounter() const;

	DeFaunaEncounter NewEncounter() const;

	DeFaunaEncounter NewFarmAnimal() const;

	void RxUpdate(QDataStream& fromServer);
	void TxUpdate(QDataStream& toClient) const;
};

inline QDataStream& operator<<(QDataStream& out, const DeTerrainFauna& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeTerrainFauna& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DePlanetFauna
{
private:
	DeTerrainFauna	m_Fauna[TerrainTypes][TOD_Any];

public:
	DePlanetFauna();
	~DePlanetFauna();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	const unsigned char* Data() const
	{
		return (const unsigned char*)m_Fauna;
	}
	unsigned int DataSize() const
	{
		return sizeof(m_Fauna);
	}

	bool HasAerialCreatures(float sunlight, TERRAIN terrainType) const;

	void InitGasGiant(ORBITZONE orbitZone, quint8 atmDensity, const AuDice& dice);

	void InitGlobe(ORBITZONE orbitZone, quint8 atmDensity, quint8 waterPercent, quint16 radius100km, const AuDice& dice);

	void InitRingworld(ORBITZONE orbitZone, quint8 atmDensity, quint8 waterPercent, const AuDice& dice);

	// Test HasAerialCreatures before calling this.
	DeFaunaEncounter NewAerialEncounter(float sunlight, TERRAIN terrainType) const;

	DeFaunaEncounter NewEncounter(float sunlight, TERRAIN terrainType) const;

	DeFaunaEncounter NewFarmAnimal(float sunlight, TERRAIN terrainType) const;

	void SetData(const char* data)
	{
		memcpy(m_Fauna, data, DataSize());
	}

	void RxUpdate(QDataStream& fromServer);
	void TxUpdate(QDataStream& toClient) const;
};

inline QDataStream& operator<<(QDataStream& out, const DePlanetFauna& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DePlanetFauna& o)
{
	o << in;
	return in;
}

#endif // DeFauna_H
