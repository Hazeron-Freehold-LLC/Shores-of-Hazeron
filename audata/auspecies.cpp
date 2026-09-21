//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspecies.cpp,v 1.6 2022/02/19 18:20:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auspecies.h"
//
#include "audna.h"
#include "dgdata.h"
#include "dhdna.h"

static int s_AuSpeciesMetaType = AuSpecies::RegisterMetaType();

//static
int AuSpecies::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuSpecies>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuSpecies>();

	return metaType;
}

AuSpecies::AuSpecies(const QByteArray& ba) : m_Key(ba)
{
}

AuSpecies::AuSpecies()
{
}

AuSpecies::~AuSpecies()
{
}

AuSpecies::AuSpecies(const AuSpecies& c) : m_Key(c.m_Key)
{
}

void AuSpecies::operator>>(QDataStream& out) const
{
	out << m_Key;
}

void AuSpecies::operator<<(QDataStream& in)
{
	in >> m_Key;
}

AuDNABase* AuSpecies::CreateTypicalDNA() const
{
	DhDNA dna3;
	if(dna3.FromSpecies(*this))
		return dna3.Clone();
	AuDNA dna4;
	if(dna4.FromSpecies(*this))
		return dna4.Clone();
	return 0;
}

AuDNABase* AuSpecies::CreateDNA() const
{
	DhDNA dna3;
	if(dna3.FromSpecies(*this))
		return dna3.Clone();
	AuDNA dna4;
	if(dna4.FromSpecies(*this))
	{
		dna4.RandomizeSpecies(g_Au.Dice());
		return dna4.Clone();
	}
	return 0;
}

QString AuSpecies::Name() const
{
	QString n;
	const DgData& gd = g_Au.LockDataForRead();
	n = gd.SpeciesName(*this, false);
	g_Au.UnlockData();
	return n;
}

QString AuSpecies::NameOrDesc() const
{
	QString n;
	const DgData& gd = g_Au.LockDataForRead();
	n = gd.SpeciesName(*this, true);
	g_Au.UnlockData();
	return n;
}

QString AuSpecies::Description() const
{
	DhDNA dna3;
	if(dna3.FromSpecies(*this))
		return dna3.SpeciesDescription();
	AuDNA dna4;
	if(dna4.FromSpecies(*this))
		return dna4.SpeciesDescription();
	return QObject::tr("Prokaryote");	// Invalid organism in our universe.
}

quint64 AuSpecies::MemoryUsage() const
{
	return m_Key.size();
}

ECOROLE AuSpecies::EcoRole() const
{
	DhDNA dna3;
	if(dna3.FromSpecies(*this))
		return dna3.EcoRole();
	AuDNA dna4;
	if(dna4.FromSpecies(*this))
		return dna4.EcoRole();
	return ECO_Omnivore;
}

TOD AuSpecies::TimeActive() const
{
	DhDNA dna3;
	if(dna3.FromSpecies(*this))
		return (TOD)dna3.TimeActive;
	AuDNA dna4;
	if(dna4.FromSpecies(*this))
		return dna4.TimeActive();
	return TOD_Diurnal;
}

bool AuSpecies::IsSexuallyCompatible(const AuSpecies& species) const
{
	AuDNABase* myDNA = CreateTypicalDNA();
	if(!myDNA)
		return false;
	AuDNABase* otherDNA = species.CreateTypicalDNA();
	if(!otherDNA)
	{
		delete myDNA;
		return false;
	}

	bool is = myDNA->IsSexuallyCompatible(*otherDNA);
	delete myDNA;
	delete otherDNA;
	return is;
}

void AuSpecies::RandomVermin(const AuDice& dice)
{
	//DHH animalia
	DhDNA dna;
	dna.RandomVermin(dice);
	*this = dna.Species();
}

