//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnabase.cpp,v 1.3 2022/02/19 18:20:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audnabase.h"
//
#include "audna.h"
#include "dhdna.h"

AuDNABase::AuDNABase()
{
}

AuDNABase::~AuDNABase()
{
}

//static
AuDNABase* AuDNABase::CreateDNA(quint8 dnaVersion)
{
	switch(dnaVersion)
	{
	case 3:
		return new DhDNA();
	case 4:
		return new AuDNA();
	}
	return 0;
}

void AuDNABase::WriteDynamic(QDataStream& out) const
{
	out << (quint8)Version();
	out << *this;
}

//static
AuDNABase* AuDNABase::ReadDynamic(QDataStream& in)
{
	quint8 dnaVersion;
	in >> dnaVersion;
	AuDNABase* dna = CreateDNA(dnaVersion);
	if(!dna)
		return 0;
	in >> *dna;
	return dna;
}

