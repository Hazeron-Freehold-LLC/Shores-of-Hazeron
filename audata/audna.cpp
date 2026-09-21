//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audna.cpp,v 1.181 2022/05/09 16:14:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audna.h"
//
#include "auanimalpart.h"
#include "aubox.h"
#include "auclothing.h"
#include "audice.h"
#include "audnawidgets.h"
#include "auelapsedtimer.h"
#include "aufilever.h"
#include "auglobal.h"
#include "auglobell.h"
#include "aunoise.h"
#include "aupose.h"
#include "ausolid.h"
#include <QDataStream>
#include <QDebug>

#define d_ClothingTessFactor			1.f						// Skin triangle size multiplier to make clothing.
#define d_JuvenileHairFactor			.1f						// Fraction of DNA hair length for juveniles.

#define DNA_FILE_VER					0x01					// DNA file
#define DNA_SPECIES_VER					0x01					// Species

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const AuChromoWhich& c)
{
	dbg.nospace() << (c.Index()+1) << "of" << c.HowMany();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Static Data
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

static AuNamedColor s_HairColor[] =
{
	// https://www.collectedwebs.com/art/colors/hair/
	AuNamedColor("Black", QColor("#090806")),
	AuNamedColor("Off Black", QColor("#2C222B")),
	AuNamedColor("Dark Gray", QColor("#71635A")),
	AuNamedColor("Medium Gray", QColor("#B7A69E")),
	AuNamedColor("Light Gray", QColor("#D6C4C2")),
	AuNamedColor("Platinum Blonde", QColor("#CABFB1")),
	AuNamedColor("Bleached Blonde", QColor("#DCD0BA")),
	AuNamedColor("White Blonde", QColor("#FFF5E1")),
	AuNamedColor("Light Blonde", QColor("#E6CEA8")),
	AuNamedColor("Golden Blonde", QColor("#E5C8A8")),
	AuNamedColor("Ash Blonde", QColor("#DEBC99")),
	AuNamedColor("Honey Blonde", QColor("#B89778")),
	AuNamedColor("Strawberry Blonde", QColor("#A56B46")),
	AuNamedColor("Light Red", QColor("#B55239")),
	AuNamedColor("Dark Red", QColor("#8D4A43")),
	AuNamedColor("Light Auburn", QColor("#91553D")),
	AuNamedColor("Dark Auburn", QColor("#533D32")),
	AuNamedColor("Dark Brown", QColor("#3B3024")),
	AuNamedColor("Golden Brown", QColor("#554838")),
	AuNamedColor("Medium Brown", QColor("#4E433F")),
	AuNamedColor("Chestnut Brown", QColor("#504444")),
	AuNamedColor("Brown", QColor("#6A4E42")),
	AuNamedColor("Light Brown", QColor("#A7856A")),
	AuNamedColor("Ash Brown", QColor("#977961")),
};
static int s_HairColors = sizeof(s_HairColor) / sizeof(AuNamedColor);

static AuNamedColor s_SkinColor[] =
{
	AuNamedColor("L'Oréal 1A", QColor(242,212,202)),
	AuNamedColor("L'Oréal 2A", QColor(235,195,187)),
	AuNamedColor("L'Oréal 3A", QColor(224,175,168)),
	AuNamedColor("L'Oréal 4A", QColor(213,159,147)),
	AuNamedColor("L'Oréal 5A", QColor(198,141,130)),
	AuNamedColor("L'Oréal 6A", QColor(182,124,112)),
	AuNamedColor("L'Oréal 7A", QColor(163,106,95)),
	AuNamedColor("L'Oréal 8A", QColor(144,92,79)),
	AuNamedColor("L'Oréal 9A", QColor(123,75,65)),
	AuNamedColor("L'Oréal 10A", QColor(101,61,53)),
	AuNamedColor("L'Oréal 11A", QColor(78,47,42)),
	AuNamedColor("L'Oréal 1B", QColor(241,213,202)),
	AuNamedColor("L'Oréal 2B", QColor(235,195,185)),
	AuNamedColor("L'Oréal 3B", QColor(221,178,161)),
	AuNamedColor("L'Oréal 4B", QColor(209,160,145)),
	AuNamedColor("L'Oréal 5B", QColor(195,143,122)),
	AuNamedColor("L'Oréal 6B", QColor(178,126,105)),
	AuNamedColor("L'Oréal 7B", QColor(162,109,91)),
	AuNamedColor("L'Oréal 8B", QColor(142,93,78)),
	AuNamedColor("L'Oréal 9B", QColor(120,77,61)),
	AuNamedColor("L'Oréal 10B", QColor(100,62,49)),
	AuNamedColor("L'Oréal 11B", QColor(76,48,37)),
	AuNamedColor("L'Oréal 1C", QColor(240,214,201)),
	AuNamedColor("L'Oréal 2C", QColor(231,198,179)),
	AuNamedColor("L'Oréal 3C", QColor(220,179,161)),
	AuNamedColor("L'Oréal 4C", QColor(205,163,139)),
	AuNamedColor("L'Oréal 5C", QColor(192,145,119)),
	AuNamedColor("L'Oréal 6C", QColor(175,128,102)),
	AuNamedColor("L'Oréal 7C", QColor(157,111,85)),
	AuNamedColor("L'Oréal 8C", QColor(138,97,69)),
	AuNamedColor("L'Oréal 9C", QColor(117,79,56)),
	AuNamedColor("L'Oréal 10C", QColor(97,64,45)),
	AuNamedColor("L'Oréal 11C", QColor(75,49,36)),
	AuNamedColor("L'Oréal 1D", QColor(237,216,199)),
	AuNamedColor("L'Oréal 2D", QColor(228,200,178)),
	AuNamedColor("L'Oréal 3D", QColor(218,181,155)),
	AuNamedColor("L'Oréal 4D", QColor(203,164,135)),
	AuNamedColor("L'Oréal 5D", QColor(188,147,117)),
	AuNamedColor("L'Oréal 6D", QColor(170,129,99)),
	AuNamedColor("L'Oréal 7D", QColor(154,113,83)),
	AuNamedColor("L'Oréal 8D", QColor(135,96,67)),
	AuNamedColor("L'Oréal 9D", QColor(117,80,54)),
	AuNamedColor("L'Oréal 10D", QColor(93,65,43)),
	AuNamedColor("L'Oréal 11D", QColor(72,51,34)),
	AuNamedColor("L'Oréal 1E", QColor(237,216,199)),
	AuNamedColor("L'Oréal 2E", QColor(225,199,176)),
	AuNamedColor("L'Oréal 3E", QColor(213,182,151)),
	AuNamedColor("L'Oréal 4E", QColor(201,165,133)),
	AuNamedColor("L'Oréal 5E", QColor(185,149,115)),
	AuNamedColor("L'Oréal 6E", QColor(167,131,97)),
	AuNamedColor("L'Oréal 7E", QColor(149,113,79)),
	AuNamedColor("L'Oréal 8E", QColor(131,98,65)),
	AuNamedColor("L'Oréal 9E", QColor(113,82,51)),
	AuNamedColor("L'Oréal 10E", QColor(92,66,41)),
	AuNamedColor("L'Oréal 11E", QColor(70,52,32)),
	AuNamedColor("L'Oréal 1F", QColor(234,215,198)),
	AuNamedColor("L'Oréal 2F", QColor(224,200,176)),
	AuNamedColor("L'Oréal 3F", QColor(210,184,151)),
	AuNamedColor("L'Oréal 4F", QColor(195,165,129)),
	AuNamedColor("L'Oréal 5F", QColor(181,152,112)),
	AuNamedColor("L'Oréal 6F", QColor(165,133,94)),
	AuNamedColor("L'Oréal 7F", QColor(145,116,76)),
	AuNamedColor("L'Oréal 8F", QColor(126,100,63)),
	AuNamedColor("L'Oréal 9F", QColor(109,85,51)),
	AuNamedColor("L'Oréal 10F", QColor(89,68,39)),
	AuNamedColor("L'Oréal 11F", QColor(69,52,32)),
};
static int s_SkinColors = sizeof(s_SkinColor) / sizeof(AuNamedColor);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Genes
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuGene* AuGene::CreateGene(GTYPE gene)
{
	switch(gene)
	{
	case Hair:
		return new AuGeneHair;
	case Feathers:
		return new AuGeneFeathers;
	case Scales:
		return new AuGeneScales;
	case Spines:
		return new AuGeneSpines;
	case GTYPE_Count:
		qWarning() << "error: AuGene::CreateGene cannot create gene Count.";
		break;
	}

	qWarning() << "error: AuGene::CreateGene unknown gene" << gene
		<< "Did you add a new gene and forget to update CreateGene?";
	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeneHair::AuGeneHair(const QColor& color, quint16 lengthMM, quint64 where)
{
	m_Color = color;
	m_LengthMM = lengthMM;
	m_Where = where;
}

AuGeneHair::AuGeneHair()
{
	m_Color = QColor(255,255,255);
	m_LengthMM = 100;
	m_Where = 0;
}

AuGeneHair::AuGeneHair(const AuGeneHair& c) : AuGene(c)
{
	m_Color = c.m_Color;
	m_LengthMM = c.m_LengthMM;
	m_Where = c.m_Where;
}

void AuGeneHair::operator>>(QDataStream& out) const
{
	AuGene::operator>>(out);
	out << m_Color;
	out << m_LengthMM;
	out << m_Where;
}

void AuGeneHair::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadStream(in, fileVer);
	in >> m_Color;
	in >> m_LengthMM;
	in >> m_Where;
}

void AuGeneHair::WriteSpeciesKey(QDataStream& out) const
{
	AuGene::WriteSpeciesKey(out);
	out << ToColor32(m_Color);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << m_Where;
}

void AuGeneHair::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadSpeciesKey(in, fileVer);
	m_Color = ReadColor32(in);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_Where;
}

void AuGeneHair::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuGene::RandomizeSpecies(dice);
}

bool AuGeneHair::IsHair(AuFolliclePoint::ROLE role) const
{
	return AuFolliclePoint::IsHair(role, m_Where);
}

QWidget* AuGeneHair::MakePropertiesPage(QWidget* parent) const
{
	AuGenePageHair* pp = new AuGenePageHair(parent);
	return pp;
}

QString AuGeneHair::ListWidgetText() const
{
	QString t = QObject::tr("Hair %1mm").arg(m_LengthMM);
	QStringList whereNames = AuFolliclePoint::RoleNames(m_Where);
	if(!whereNames.empty())
	{
		t += ": ";
		QStringList::const_iterator si = whereNames.cbegin();
		t += *si;
		si++;
		while(si != whereNames.cend())
		{
			t += ", ";
			t += *si;
			si++;
		}
	}
	return t;
}

bool AuGeneHair::IsSexuallyCompatible(const AuGene* trait) const
{
	Q_UNUSED(trait);
	//const AuGeneHair* c = (const AuGeneHair*)trait;
	//if(m_Color != c->m_Color)
		//return false;
	//if(m_LengthMM != c->m_LengthMM)
		//return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeneFeathers::AuGeneFeathers(const QColor& color, quint8 shape, quint16 lengthMM)
{
	m_Color = color;
	m_LengthMM = lengthMM;
	m_Shape = shape;
}

AuGeneFeathers::AuGeneFeathers()
{
	m_LengthMM = 100;
	m_Shape = 0;
}

AuGeneFeathers::AuGeneFeathers(const AuGeneFeathers& c) : AuGene(c)
{
	m_Color = c.m_Color;
	m_LengthMM = c.m_LengthMM;
	m_Shape = c.m_Shape;
}

void AuGeneFeathers::operator>>(QDataStream& out) const
{
	AuGene::operator>>(out);
	out << m_Color;
	out << m_LengthMM;
	out << m_Shape;
}

void AuGeneFeathers::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadStream(in, fileVer);
	in >> m_Color;
	in >> m_LengthMM;
	in >> m_Shape;
}

void AuGeneFeathers::WriteSpeciesKey(QDataStream& out) const
{
	AuGene::WriteSpeciesKey(out);
	out << ToColor32(m_Color);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << m_Shape;
}

void AuGeneFeathers::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadSpeciesKey(in, fileVer);
	m_Color = ReadColor32(in);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_Shape;
}

void AuGeneFeathers::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuGene::RandomizeSpecies(dice);
}

QWidget* AuGeneFeathers::MakePropertiesPage(QWidget* parent) const
{
	AuGenePageFeathers* pp = new AuGenePageFeathers(parent);
	return pp;
}

QString AuGeneFeathers::ListWidgetText() const
{
	QString t = QObject::tr("Feathers %1mm").arg(m_LengthMM);
	return t;
}

bool AuGeneFeathers::IsSexuallyCompatible(const AuGene* trait) const
{
	Q_UNUSED(trait);
	//const AuGeneFeathers* c = (const AuGeneFeathers*)trait;
	//if(m_Color != c->m_Color)
		//return false;
	//if(m_Shape != c->m_Shape)
		//return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeneScales::AuGeneScales(const QColor& color, quint16 lengthMM)
{
	m_Color = color;
	m_LengthMM = lengthMM;
}

AuGeneScales::AuGeneScales()
{
	m_LengthMM = 100;
}

AuGeneScales::AuGeneScales(const AuGeneScales& c) : AuGene(c)
{
	m_Color = c.m_Color;
	m_LengthMM = c.m_LengthMM;
}

void AuGeneScales::operator>>(QDataStream& out) const
{
	AuGene::operator>>(out);
	out << m_Color;
	out << m_LengthMM;
}

void AuGeneScales::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadStream(in, fileVer);
	in >> m_Color;
	in >> m_LengthMM;
}

void AuGeneScales::WriteSpeciesKey(QDataStream& out) const
{
	AuGene::WriteSpeciesKey(out);
	out << ToColor32(m_Color);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
}

void AuGeneScales::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadSpeciesKey(in, fileVer);
	m_Color = ReadColor32(in);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuGeneScales::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuGene::RandomizeSpecies(dice);
}

QWidget* AuGeneScales::MakePropertiesPage(QWidget* parent) const
{
	AuGenePageScales* pp = new AuGenePageScales(parent);
	return pp;
}

QString AuGeneScales::ListWidgetText() const
{
	QString t = QObject::tr("Scales %1mm").arg(m_LengthMM);
	return t;
}

bool AuGeneScales::IsSexuallyCompatible(const AuGene* trait) const
{
	Q_UNUSED(trait);
	//const AuGeneScales* c = (const AuGeneScales*)trait;
	//if(m_Color != c->m_Color)
		//return false;
	//if(m_LengthMM != c->m_LengthMM)
		//return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeneSpines::AuGeneSpines(const QColor& color, quint16 lengthMM)
{
	m_Color = color;
	m_LengthMM = lengthMM;
}

AuGeneSpines::AuGeneSpines()
{
	m_LengthMM = 100;
}

AuGeneSpines::AuGeneSpines(const AuGeneSpines& c) : AuGene(c)
{
	m_Color = c.m_Color;
	m_LengthMM = c.m_LengthMM;
}

void AuGeneSpines::operator>>(QDataStream& out) const
{
	AuGene::operator>>(out);
	out << m_Color;
	out << m_LengthMM;
}

void AuGeneSpines::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadStream(in, fileVer);
	in >> m_Color;
	in >> m_LengthMM;
}

void AuGeneSpines::WriteSpeciesKey(QDataStream& out) const
{
	AuGene::WriteSpeciesKey(out);
	out << ToColor32(m_Color);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
}

void AuGeneSpines::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuGene::ReadSpeciesKey(in, fileVer);
	m_Color = ReadColor32(in);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuGeneSpines::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuGene::RandomizeSpecies(dice);
}

QWidget* AuGeneSpines::MakePropertiesPage(QWidget* parent) const
{
	AuGenePageSpines* pp = new AuGenePageSpines(parent);
	return pp;
}

QString AuGeneSpines::ListWidgetText() const
{
	QString t = QObject::tr("Spines %1mm").arg(m_LengthMM);
	return t;
}

bool AuGeneSpines::IsSexuallyCompatible(const AuGene* trait) const
{
	Q_UNUSED(trait);
	//const AuGeneSpines* c = (const AuGeneSpines*)trait;
	//if(m_Color != c->m_Color)
		//return false;
	//if(m_LengthMM != c->m_LengthMM)
		//return false;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGeneChain::AuGeneChain(const AuGeneChain& c) : QList<AuGene*>()
{
	for(const_iterator ti = c.cbegin(); ti != c.cend(); ti++)
		append((*ti)->Clone());
}

const AuGeneChain& AuGeneChain::operator=(const AuGeneChain& dna)
{
	Clear();
	for(const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		append((*ti)->Clone());
	return *this;
}

bool AuGeneChain::operator==(const AuGeneChain& dna) const
{
	// Must have the same number of traits.
	if(count() != dna.count())
		return false;

	for(const_iterator ti = cbegin(), ci = dna.cbegin(); ti != cend() && ci != dna.cend(); ti++, ci++)
		if(*(*ti) != *(*ci))
			return false;
	
	return true;
}

void AuGeneChain::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		out << (quint8)(*ti)->Type();
		out << **ti;
	}
}

void AuGeneChain::ReadStream(QDataStream& in, quint8 fileVer)
{
	Clear();

	quint16 traits;
	in >> traits;
	quint8 type;
	while(traits--)
	{
		in >> type;
		AuGene* trait = AuGene::CreateGene((AuGene::GTYPE)type);
		if(trait)
		{
			trait->ReadStream(in, fileVer);
			append(trait);
		}
		else
			qWarning() << "AuGeneChain::ReadStream received invalid gene type" << type;
	}
}

void AuGeneChain::WriteSpeciesKey(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		out << (quint8)(*ti)->Type();
		(*ti)->WriteSpeciesKey(out);
	}
}

void AuGeneChain::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	Clear();

	quint16 traits;
	in >> traits;
	quint8 type;
	while(traits--)
	{
		in >> type;
		AuGene* trait = AuGene::CreateGene((AuGene::GTYPE)type);
		if(trait)
		{
			trait->ReadSpeciesKey(in, fileVer);
			append(trait);
		}
		else
			qWarning() << "AuGeneChain::ReadSpeciesKey received invalid gene type" << type;
	}
}

void AuGeneChain::RandomizeSpecies(const AuDice& dice)
{
	for(iterator ti = begin(); ti != end(); ti++)
		(*ti)->RandomizeSpecies(dice);	// Same dice for each.
}

void AuGeneChain::AddGene(const AuGene& trait)
{
	append(trait.Clone());
}

const AuGeneHair* AuGeneChain::HairDNA(AuFolliclePoint::ROLE role) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti)->IsHair(role))
			return (const AuGeneHair*)*ti;
	return 0;
}

void AuGeneChain::AddableGenes(QMap<QString, AuGene::GTYPE>& addable) const
{
	// Determine what I already have. Then I can decide what is addable.
	int countFeathers = 0;
	int countHair = 0;
	int countScales = 0;
	int countSpines = 0;
	bool error = false;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		switch((*ti)->Type())
		{
		case AuGene::Feathers:
			countFeathers++;
			if(countFeathers > MaxGeneCount())
				error = true;
			break;
		case AuGene::Hair:
			countHair++;
			if(countHair > MaxGeneCount())
				error = true;
			break;
		case AuGene::Scales:
			countScales++;
			if(countScales > MaxGeneCount())
				error = true;
			break;
		case AuGene::Spines:
			countSpines++;
			if(countSpines > MaxGeneCount())
				error = true;
			break;
		case AuGene::GTYPE_Count:
			error = true;	// This is an error.
			break;
		}
	}
	if(error)
	{
		addable.clear();
		return;
	}

	// Decide what can be added.
	// Feathers, hair, and scales are mutually exclusive of each other.
	if(!countFeathers && !countScales && countHair < MaxGeneCount())
		addable.insert(AuGeneHair::GeneName(), AuGeneHair::GeneType());
	//DHH animalia if(!countHair && !countScales && countFeathers < MaxGeneCount())
		//DHH animalia addable.insert(AuGeneFeathers::GeneName(), AuGeneFeathers::GeneType());
	//DHH animalia if(!countHair && !countFeathers && countScales < MaxGeneCount())
		//DHH animalia addable.insert(AuGeneScales::GeneName(), AuGeneScales::GeneType());

	// Anybody can have spines.
	//DHH animalia if(countSpines < MaxGeneCount())
		//DHH animalia addable.insert(AuGeneSpines::GeneName(), AuGeneSpines::GeneType());
}

void AuGeneChain::AddGene(AuGene::GTYPE type)
{
	AuGene* t = AuGene::CreateGene(type);
	if(!t)
		return;
	if(!t->PrepareToAdd(*this))
		return;
	AddGene(*t);
	delete t;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromosome* AuChromosome::CreateChromosome(CTYPE chromosome)
{
	switch(chromosome)
	{
	case CTYPE_Count:
		return 0;

	// Structure
	case Annelida:
		return new AuChromoAnnelida;
	case Arthropoda:
		return new AuChromoArthropoda;
	case Chordata:
		return new AuChromoChordata;
	case Mollusca:
		return new AuChromoMollusca;

	// Body Covering
	case Feathers:
		return new AuChromoFeathers;
	case Fur:
		return new AuChromoFur;
	case Scales:
		return new AuChromoScales;
	case Skin:
		return new AuChromoSkin;

	// Body Sections
	case Head:
		return new AuChromoHead;
	case Torso:
		return new AuChromoTorso;

	// Limbs
	case Arm:
		return new AuChromoArm;
	case Leg:
		return new AuChromoLeg;
	case Fin:
		return new AuChromoFin;
	case Tentacle:
		return new AuChromoTentacle;
	case Wing:
		return new AuChromoWing;

	// Head Features
	case Antenna:
		return new AuChromoAntenna;
	case Horn:
		return new AuChromoHorn;
	case Ear:
		return new AuChromoEar;
	case Eye:
		return new AuChromoEye;
	case Nose:
		return new AuChromoNose;
	case Mouth:
		return new AuChromoMouth;

	// Internal
	case Lungs:
		return new AuChromoLungs;
	case Gills:
		return new AuChromoGills;
	case Blood:
		return new AuChromoBlood;

	// Young
	case Reproduction:
		return new AuChromoReproduction;
	case Mammary:
		return new AuChromoMammary;

	// Food
	case EatsPlants:
		return new AuChromoEatsPlants;
	case EatsAnimals:
		return new AuChromoEatsAnimals;
	}

	qWarning() << "error: AuChromosome::CreateChromosome unknown chromosome" << chromosome
		<< "Did you add a new chromosome and forget to update CreateChromosome?";
	return 0;
}

AuAnimalPart* AuChromosome::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	Q_UNUSED(dna);
	Q_UNUSED(firstBone);
	return 0;
}

//static
quint64 AuChromosome::HairWhereBits(CTYPE chromosome, const AuChromoWhich& which)
{
	AuChromosome* t = CreateChromosome(chromosome);
	if(!t)
		return 0;
	quint64 b = t->HairWhereBits(which);
	delete t;
	return b;
}

//static
qint16 AuChromosome::CombineAngle(qint16 a1, qint16 a2, const AuDice& Cdice)
{
	if(a1 == a2)
		return a1;
	AuDice dice(Cdice);
	return (dice.Roll(1,100) <= 50) ? a1 : a2;
}

//static
quint64 AuChromosome::CombineBits(quint64 b1, quint64 b2, const AuDice& Cdice)
{
	if(b1 == b2)
		return b1;
	AuDice dice(Cdice);

	quint64 bc = 0;
	for(int b=0; b<64; b++)
	{
		quint64 bval = 1ULL << b;	// 64 bit shift requires the 1 to be cast as a 64 bit value, otherwise it's a 32 bit int.
		if((b1 & bval) && (b2 & bval))
			bc |= bval;											// Both genes have the bit on.
		else if((b1 & bval) || (b2 & bval))
			bc |= (dice.Roll(1,100) <= 50) ? bval : 0;			// 50:50 chance the result has the bit on.
	}
	return bc;
}

//static
bool AuChromosome::CombineBool(bool b1, bool b2, const AuDice& Cdice)
{
	if(b1 == b2)
		return b1;
	AuDice dice(Cdice);
	return (dice.Roll(1,100) <= 50) ? b1 : b2;
}

//static
QColor AuChromosome::CombineColor(const QColor& c1, const QColor& c2, const AuDice& Cdice)
{
	if(c1 == c2)
		return c1;
	AuDice dice(Cdice);
	return (dice.Roll(1,100) <= 50) ? c1 : c2;
}

//static
int AuChromosome::CombineEnum(int e1, int e2, const AuDice& Cdice)
{
	if(e1 == e2)
		return e1;
	AuDice dice(Cdice);
	return (dice.Roll(1,100) <= 50) ? e1 : e2;
}

//static
quint16 AuChromosome::CombineRange(quint16 r1, quint16 r2, const AuDice& Cdice)
{
	if(r1 == r2)
		return r1;
	AuDice dice(Cdice);
	int r = dice.Roll(1,100);
	if(r <= 25)
		return r1;
	else if(r > 75)
		return r2;

	// Return a value from r1 to r2, inclusive.
	if(r1 > r2)
	{
		quint16 range = r1 - r2 + 1;
		return r2 + dice.Roll(1,range,-1);
	}
	else
	{
		quint16 range = r2 - r1 + 1;
		return r1 + dice.Roll(1,range,-1);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromosomeWithGenes::AuChromosomeWithGenes(const AuChromosomeWithGenes& c1, const AuChromosomeWithGenes& c2, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// We don't combine genes when combining DNA. We just grab the entire gene chain from one chromosome or the other.
	// This could be improved.
	if(dice.Roll(1,100) <= 50)
	{
		if(c1.m_Genes)
			m_Genes = new AuGeneChain(*c1.m_Genes);
		else
			m_Genes = 0;
	}
	else
	{
		if(c2.m_Genes)
			m_Genes = new AuGeneChain(*c2.m_Genes);
		else
			m_Genes = 0;
	}
}

void AuChromosomeWithGenes::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(m_Genes && !m_Genes->empty())
		hasWhat |= 0x01;
	out << hasWhat;

	if(hasWhat & 0x01)
		out << *m_Genes;
}

void AuChromosomeWithGenes::ReadStream(QDataStream& in, quint8 fileVer)
{
	Q_UNUSED(fileVer);
	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x01)
	{
		if(!m_Genes)
			m_Genes = new AuGeneChain;
		m_Genes->ReadStream(in, fileVer);
	}
	else if(m_Genes)
	{
		delete m_Genes;
		m_Genes = 0;
	}
}

void AuChromosomeWithGenes::WriteSpeciesKey(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(m_Genes && !m_Genes->empty())
		hasWhat |= 0x01;
	out << hasWhat;

	if(hasWhat & 0x01)
		m_Genes->WriteSpeciesKey(out);
}

void AuChromosomeWithGenes::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	Q_UNUSED(fileVer);
	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x01)
	{
		if(!m_Genes)
			m_Genes = new AuGeneChain;
		m_Genes->ReadSpeciesKey(in, fileVer);
	}
	else if(m_Genes)
	{
		delete m_Genes;
		m_Genes = 0;
	}
}

void AuChromosomeWithGenes::RandomizeSpecies(const AuDice& dice)
{
	if(m_Genes)
		m_Genes->RandomizeSpecies(dice);
}

void AuChromosomeWithGenes::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromosomeWithGenes& c = dynamic_cast<const AuChromosomeWithGenes&>(o);
		if(m_Genes && c.m_Genes)
			*m_Genes = *c.m_Genes;
		else if(m_Genes)
		{
			delete m_Genes;
			m_Genes = 0;
		}
		else if(c.m_Genes)
			m_Genes = new AuGeneChain(*c.m_Genes);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromosomeWithGenes::AddGene(const AuGene& trait)
{
	if(!m_Genes)
		m_Genes = new AuGeneChain;
	m_Genes->AddGene(trait);
}

const AuGeneHair* AuChromosomeWithGenes::HairDNA(AuFolliclePoint::ROLE role) const
{
	if(!m_Genes)
		return 0;
	return m_Genes->HairDNA(role);
}

bool AuChromosomeWithGenes::SetGenes(const AuGeneChain& genes)
{
	if(genes.empty())
	{
		if(m_Genes)
		{
			delete m_Genes;
			m_Genes = 0;
			return true;
		}
	}
	else if(!m_Genes)
	{
		m_Genes = new AuGeneChain(genes);
		return true;
	}
	else if(*m_Genes != genes)
	{
		*m_Genes = genes;
		return true;
	}
	return false;	// no change
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromatid::AuChromatid(const AuChromatid& c) : QList<AuChromosome*>()
{
	for(const_iterator ti = c.cbegin(); ti != c.cend(); ti++)
		append((*ti)->Clone());
}

const AuChromatid& AuChromatid::operator=(const AuChromatid& dna)
{
	Clear();
	for(const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		append((*ti)->Clone());
	return *this;
}

void AuChromatid::AddableChromosomes(QMap<QString, AuChromosome::CTYPE>& addable) const
{
	// Determine what I already have. Then I can decide what is addable.
	bool hasAnnelida = false;
	bool hasArthropoda = false;
	bool hasChordata = false;
	bool hasMollusca = false;
	bool hasFeathers = false;
	bool hasFur = false;
	bool hasSkin = false;
	bool hasScales = false;
	bool hasHead = false;
	bool hasTorso = false;
	unsigned int countArm = 0;
	unsigned int countLeg = 0;
	unsigned int countFin = 0;
	unsigned int countTentacle = 0;
	unsigned int countWing = 0;
	unsigned int countAntenna = 0;
	unsigned int countHorn = 0;
	unsigned int countEar = 0;
	unsigned int countEye = 0;
	bool hasNose = false;
	bool hasMouth = false;
	bool hasLungs = false;
	bool hasGills = false;
	bool hasBlood = false;
	bool hasEatsAnimals = false;
	bool hasEatsPlants = false;
	bool hasReproduction = false;
	unsigned int countMammary = 0;
	bool error = false;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		switch((*ti)->Type())
		{
		case AuChromosome::Annelida:
			if(hasAnnelida)
				error = true;
			else
				hasAnnelida = true;
			break;
		case AuChromosome::Arthropoda:
			if(hasArthropoda)
				error = true;
			else
				hasArthropoda = true;
			break;
		case AuChromosome::Chordata:
			if(hasChordata)
				error = true;
			else
				hasChordata = true;
			break;
		case AuChromosome::Mollusca:
			if(hasMollusca)
				error = true;
			else
				hasMollusca = true;
			break;
		case AuChromosome::Feathers:
			if(hasFeathers)
				error = true;
			else
				hasFeathers = true;
			break;
		case AuChromosome::Fur:
			if(hasFur)
				error = true;
			else
				hasFur = true;
			break;
		case AuChromosome::Skin:
			if(hasSkin)
				error = true;
			else
				hasSkin = true;
			break;
		case AuChromosome::Scales:
			if(hasScales)
				error = true;
			else
				hasScales = true;
			break;
		case AuChromosome::Head:
			if(hasHead)
				error = true;
			else
				hasHead = true;
			break;
		case AuChromosome::Torso:
			if(hasTorso)
				error = true;
			else
				hasTorso = true;
			break;
		case AuChromosome::Arm:
			countArm++;
			if(countArm > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Leg:
			countLeg++;
			if(countLeg > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Fin:
			countFin++;
			if(countFin > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Tentacle:
			countTentacle++;
			if(countTentacle > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Wing:
			countWing++;
			if(countWing > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Antenna:
			countAntenna++;
			if(countAntenna > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Horn:
			countHorn++;
			if(countHorn > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Ear:
			countEar++;
			if(countEar > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Eye:
			countEye++;
			if(countEye > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::Nose:
			if(hasNose)
				error = true;
			else
				hasNose = true;
			break;
		case AuChromosome::Mouth:
			if(hasMouth)
				error = true;
			else
				hasMouth = true;
			break;
		case AuChromosome::Lungs:
			if(hasLungs)
				error = true;
			else
				hasLungs = true;
			break;
		case AuChromosome::Gills:
			if(hasGills)
				error = true;
			else
				hasGills = true;
			break;
		case AuChromosome::Blood:
			if(hasBlood)
				error = true;
			else
				hasBlood = true;
			break;
		case AuChromosome::EatsAnimals:
			if(hasEatsAnimals)
				error = true;
			else
				hasEatsAnimals = true;
			break;
		case AuChromosome::EatsPlants:
			if(hasEatsPlants)
				error = true;
			else
				hasEatsPlants = true;
			break;
		case AuChromosome::Reproduction:
			if(hasReproduction)
				error = true;
			else
				hasReproduction = true;
			break;
		case AuChromosome::Mammary:
			countMammary++;
			if(countMammary > AuChromoWhich::MaxCount())
				error = true;
			break;
		case AuChromosome::CTYPE_Count:
			error = true;	// This is an error.
			break;
		}
	}
	if(error)
	{
		addable.clear();
		return;
	}

	// Decide what can be added.
	// Phylum
	if(!hasChordata && !hasArthropoda && !hasAnnelida && !hasMollusca)
	{
		addable.insert(AuChromoAnnelida::ChromosomeName(), AuChromoAnnelida::ChromosomeType());
		addable.insert(AuChromoArthropoda::ChromosomeName(), AuChromoArthropoda::ChromosomeType());
		addable.insert(AuChromoChordata::ChromosomeName(), AuChromoChordata::ChromosomeType());
		addable.insert(AuChromoMollusca::ChromosomeName(), AuChromoMollusca::ChromosomeType());
		return;	// Must choose phylum before continuing.
	}

	// Body Covering except on exoskeleton.
	if(!hasFeathers && !hasFur && !hasSkin && !hasScales && !hasArthropoda)
	{
		addable.insert(AuChromoFeathers::ChromosomeName(), AuChromoFeathers::ChromosomeType());
		addable.insert(AuChromoFur::ChromosomeName(), AuChromoFur::ChromosomeType());
		addable.insert(AuChromoSkin::ChromosomeName(), AuChromoSkin::ChromosomeType());
		addable.insert(AuChromoScales::ChromosomeName(), AuChromoScales::ChromosomeType());
		return;
	}

	// Internal
	if(!hasBlood)
	{
		addable.insert(AuChromoBlood::ChromosomeName(), AuChromoBlood::ChromosomeType());
		return;
	}
	if(!hasLungs)
		addable.insert(AuChromoLungs::ChromosomeName(), AuChromoLungs::ChromosomeType());
	if(!hasGills)
		addable.insert(AuChromoGills::ChromosomeName(), AuChromoGills::ChromosomeType());
	if(!hasLungs && !hasGills)
		return;

	if(!hasEatsAnimals)
		addable.insert(AuChromoEatsAnimals::ChromosomeName(), AuChromoEatsAnimals::ChromosomeType());
	if(!hasEatsPlants)
		addable.insert(AuChromoEatsPlants::ChromosomeName(), AuChromoEatsPlants::ChromosomeType());
	if(!hasEatsAnimals && !hasEatsPlants)
		return;

	if(!hasReproduction)
		addable.insert(AuChromoReproduction::ChromosomeName(), AuChromoReproduction::ChromosomeType());
	if(!hasReproduction)
		return;

	// Head
	if(!hasHead)
	{
		addable.insert(AuChromoHead::ChromosomeName(), AuChromoHead::ChromosomeType());
		return;	// Must add a head before continuing.
	}

	// Head features
	if(countAntenna < AuChromoWhich::MaxCount())
		addable.insert(AuChromoAntenna::ChromosomeName(), AuChromoAntenna::ChromosomeType());

	if(countHorn < AuChromoWhich::MaxCount())
		addable.insert(AuChromoHorn::ChromosomeName(), AuChromoHorn::ChromosomeType());

	if(countEar < AuChromoWhich::MaxCount())
		addable.insert(AuChromoEar::ChromosomeName(), AuChromoEar::ChromosomeType());

	if(countEye < AuChromoWhich::MaxCount())
		addable.insert(AuChromoEye::ChromosomeName(), AuChromoEye::ChromosomeType());

	if(!hasNose)
		addable.insert(AuChromoNose::ChromosomeName(), AuChromoNose::ChromosomeType());

	if(!hasMouth)
		addable.insert(AuChromoMouth::ChromosomeName(), AuChromoMouth::ChromosomeType());

	// Body
	if(!hasTorso)
	{
		addable.insert(AuChromoTorso::ChromosomeName(hasArthropoda), AuChromoTorso::ChromosomeType());
		return;	// Must add a torso before continuing.
	}

	// Body features
	if(countMammary < AuChromoWhich::MaxCount())
		addable.insert(AuChromoMammary::ChromosomeName(), AuChromoMammary::ChromosomeType());

	if(countArm < AuChromoWhich::MaxCount())
		addable.insert(AuChromoArm::ChromosomeName(), AuChromoArm::ChromosomeType());

	if(countLeg < AuChromoWhich::MaxCount())
		addable.insert(AuChromoLeg::ChromosomeName(), AuChromoLeg::ChromosomeType());

	if(countFin < AuChromoWhich::MaxCount())
		addable.insert(AuChromoFin::ChromosomeName(), AuChromoFin::ChromosomeType());

	if(countTentacle < AuChromoWhich::MaxCount())
		addable.insert(AuChromoTentacle::ChromosomeName(), AuChromoTentacle::ChromosomeType());

	if(countWing < AuChromoWhich::MaxCount())
		addable.insert(AuChromoWing::ChromosomeName(), AuChromoWing::ChromosomeType());
}

bool AuChromatid::AddChromosome(const AuChromosome& chromosome)
{
	/*
	// First check to see if any existing chromosomes are replaced by the new chromosome.
	iterator ti = begin();
	while(ti != end())
	{
		if((*ti)->IsReplacedBy(chromosome))
		{
			delete *ti;
			ti = erase(ti);
		}
		else
			ti++;
	}
	*/

	// Insert the new chromosome into the list according to pose order.
	for(QList<AuChromosome*>::iterator ti = begin(); ti != end(); ti++)
		if(chromosome.PoseOrder() < (*ti)->PoseOrder())
		{
			insert(ti, chromosome.Clone());
			UpdateChromosomeWhichCounts(chromosome.Type());
			return true;
		}

	// If the new chromosome was not inserted, add it to the end.
	append(chromosome.Clone());
	UpdateChromosomeWhichCounts(chromosome.Type());
	return true;
}

bool AuChromatid::AddChromosome(AuChromosome::CTYPE chromosome)
{
	AuChromosome* t = AuChromosome::CreateChromosome(chromosome);
	if(!t)
		return false;
	bool modelChanged = false;
	if(t->PrepareToAdd(*this))
		modelChanged = AddChromosome(*t);
	delete t;
	return modelChanged;
}

void AuChromatid::InitDefaultHuman(GENDER gender, const QColor& skinColor, const QColor& hairColor)
{
	Clear();

	// Torso
	quint16 neckLenMM;
	quint16 upperTorsoLenMM;
	quint16 upperTorsoWidthMM;
	quint16 upperTorsoThickMM;
	quint16 lowerTorsoLenMM;
	quint16 lowerTorsoWidthMM;
	quint16 lowerTorsoThickMM;
	quint16 tailLenMM = 0;

	// Head
	quint16 headAttachOffMM;
	quint16 headWidthMM;
	quint16 headLengthMM;
	quint16 headHeightMM;
	qint16 headEarAngleDeg;
	qint16 headEyeAngleDeg;
	qint16 headMouthAngleDeg;
	qint16 headNoseAngleDeg;

	MEAT_DECAL eyeDecal;
	AuChromoEye::FACING eyeFacing;
	quint16 eyeDiaMM;
	quint16 eyeSocketDiaMM;

	quint16 earDiaMM;
	qint16 earAngleDeg;

	quint16 noseLenMM;
	quint16 noseWidHeadMM;
	quint16 noseWidTipMM;
	quint16 noseAngleDeg;

	quint16 mouthLenMM;
	quint16 mouthWidMM;
	quint16 mouthHtMM;
	quint16 mouthDepMM;
	AuChromoMouth::TEETH teeth;
	quint16 teethHtBottomMM;
	quint16 teethHtTopMM;
	quint16 lipThickMM;

	// Arms
	quint16 shoulderToSpineMM;
	quint16 shoulderRadMM;
	quint16 upperArmLenMM;
	quint16 elbowRadMM;
	quint16 foreArmLenMM;
	quint16 wristRadMM;
	quint16 handLenMM;

	// Legs
	quint16 buttRadMM;
	quint16 hipToSpineMM;
	quint16 hipRadXMM;
	quint16 ankleLenMM;
	quint16 ankleRadMM;
	quint16 footLenMM;
	quint16 footWidMM;
	quint16 thighLenMM;
	quint16 calfLenMM;
	quint16 kneeRadXMM;

	quint16 boobSizeMM;
	AuChromoMammary::ORGAN milkGland;
	bool milkYoung;

	AuChromoReproduction::BIRTHPLACE birthPlace = AuChromoReproduction::OnLand;
	AuChromoReproduction::YOUNG young = AuChromoReproduction::Live;
	quint8 genitalBits;
	quint8 litterMin = 1;
	quint8 litterMax = 2;

	COMMODITYID plantPref;

	quint8 eatBits = 
		//AuChromoEatsAnimals::BloodSucker |
		AuChromoEatsAnimals::Brain |
		AuChromoEatsAnimals::Cannibal |
		AuChromoEatsAnimals::Carne |
		//AuChromoEatsAnimals::Carrion |
		AuChromoEatsAnimals::Organs |
		AuChromoEatsAnimals::Predator;

	if(gender == Male || gender == Hermaphrodite)
	{
		// Human male dimensions.
		neckLenMM = 185;										// Data sheet
		lowerTorsoLenMM = 455 * .2;								// Data sheet
		lowerTorsoWidthMM = 252;
		lowerTorsoThickMM = 180;								// Data sheet
		upperTorsoLenMM = 455 - lowerTorsoLenMM;				// Data sheet
		upperTorsoWidthMM = 285;
		upperTorsoThickMM = 190;

		// Arms
		shoulderToSpineMM = 350 /2;								// Data sheet
		shoulderRadMM = 450 /2 - shoulderToSpineMM;				// Data sheet
		upperArmLenMM = 280;									// Data sheet
		elbowRadMM = (3.f/12 /2) / FeetPerMM();
		foreArmLenMM = 255;										// Data sheet
		wristRadMM = (2.25/12 /2) / FeetPerMM();
		handLenMM = 190;										// Data sheet

		// Legs
		hipToSpineMM = 175 /2;									// Data sheet
		buttRadMM = hipToSpineMM * 1.16f;
		hipRadXMM = 335 /2 - hipToSpineMM;						// Data sheet
		ankleLenMM = 115 - 25;									// Data sheet
		ankleRadMM = (3.f/12 /2) / FeetPerMM();
		footLenMM = 265 - ankleRadMM;							// Data sheet. Foot length in front of ankle bone.
		footWidMM = 100;										// Data sheet
		thighLenMM = 425;										// Data sheet
		calfLenMM = 410;										// Data sheet
		kneeRadXMM = (2.3f/12) / FeetPerMM();

		// Head
		quint16 eyeLevelMM = 1660;								// Data sheet
		headAttachOffMM = eyeLevelMM - ((ankleLenMM + 25) + calfLenMM + thighLenMM + lowerTorsoLenMM + upperTorsoLenMM +
				neckLenMM);										// Data sheet
		headWidthMM = 155;										// Data sheet
		headLengthMM = 195;										// Data sheet
		headHeightMM = 220;									 	// Data sheet
		headEarAngleDeg = 100;
		headEyeAngleDeg = 20;
		headMouthAngleDeg = 120;
		headNoseAngleDeg = 80;

		eyeDecal = MD_HumanEye1;
		eyeFacing = AuChromoEye::Forward;
		eyeDiaMM = 24;
		eyeSocketDiaMM = 40;

		earDiaMM = (quint16)((2.5 / 12) / FeetPerMM());
		earAngleDeg = 25;

		noseLenMM = (quint16)(2. / 12 / FeetPerMM());
		noseWidHeadMM = (quint16)(1.2 / 12 / FeetPerMM());
		noseWidTipMM = (quint16)(1.5 / 12 / FeetPerMM());
		noseAngleDeg = -50;

		mouthLenMM = 37;
		mouthWidMM = 75;
		mouthHtMM = 40;
		mouthDepMM = 50;
		teeth = AuChromoMouth::Fangs;
		teethHtBottomMM = 3;
		teethHtTopMM = 4;
		lipThickMM = 4;

		boobSizeMM = 33;
		milkGland = AuChromoMammary::Nipple;
		milkYoung = false;

		genitalBits = AuChromoReproduction::GEN_Male;

		plantPref = CID_Fruit;
	}
	else
	{
		// Human female dimensions.
		neckLenMM = 175;										// Data sheet
		lowerTorsoLenMM = 425 * .2;								// Data sheet
		lowerTorsoWidthMM = 230;
		lowerTorsoThickMM = 175;								// Data sheet
		upperTorsoLenMM = 425 - lowerTorsoLenMM;				// Data sheet
		upperTorsoWidthMM = 265;
		upperTorsoThickMM = 183;

		// Arms
		shoulderToSpineMM = 315 /2;								// Data sheet
		shoulderRadMM = 405 /2 - shoulderToSpineMM;				// Data sheet
		upperArmLenMM = 265;									// Data sheet
		elbowRadMM = (2.5f/12 /2) / FeetPerMM();
		foreArmLenMM = 235;										// Data sheet
		wristRadMM = (1.75/12 /2) / FeetPerMM();
		handLenMM = 175;										// Data sheet

		// Legs
		hipToSpineMM = 180 /2;									// Data sheet
		buttRadMM = hipToSpineMM * 1.27f;
		hipRadXMM = 350 /2 - hipToSpineMM;						// Data sheet
		ankleLenMM = 120 - 40;									// Data sheet
		ankleRadMM = (2.75f/12 /2) / FeetPerMM();
		footLenMM = 245 - ankleRadMM;							// Data sheet. Foot length in front of ankle bone.
		footWidMM = 90;											// Data sheet
		thighLenMM = 390;										// Data sheet
		calfLenMM = 380;										// Data sheet
		kneeRadXMM = (2.3f/12) / FeetPerMM();

		// Head
		quint16 eyeLevelMM = 1560;
		headAttachOffMM = eyeLevelMM - ((ankleLenMM + 40) + calfLenMM + thighLenMM + lowerTorsoLenMM + upperTorsoLenMM +
				neckLenMM);										// Data sheet
		headWidthMM = 145;										// Data sheet
		headLengthMM = 185;										// Data sheet
		headHeightMM = 205;									 	// Data sheet
		headEarAngleDeg = 100;
		headEyeAngleDeg = 20;
		headMouthAngleDeg = 120;
		headNoseAngleDeg = 80;

		eyeDecal = MD_HumanEye3;
		eyeFacing = AuChromoEye::Forward;
		eyeDiaMM = 24;
		eyeSocketDiaMM = 40;

		earDiaMM = (quint16)((2.4 / 12) / FeetPerMM());
		earAngleDeg = 25;

		noseLenMM = (quint16)(1.9 / 12 / FeetPerMM());
		noseWidHeadMM = (quint16)(1.0 / 12 / FeetPerMM());
		noseWidTipMM = (quint16)(.8 / 12 / FeetPerMM());
		noseAngleDeg = -50;

		mouthLenMM = 35;
		mouthWidMM = 65;
		mouthHtMM = 35;
		mouthDepMM = 45;
		teeth = AuChromoMouth::Fangs;
		teethHtBottomMM = 3;
		teethHtTopMM = 4;
		lipThickMM = 4;

		boobSizeMM = 100;
		milkGland = AuChromoMammary::Breast;
		milkYoung = true;

		genitalBits = AuChromoReproduction::GEN_Female;

		plantPref = CID_Vegetable;
	}

	// Build chromosomes that have genes.
	AuChromoHead head(AuChromoHead(AuChromoHead::AttachBottom, headAttachOffMM, headWidthMM, headLengthMM, headHeightMM,
				headEarAngleDeg, headEyeAngleDeg, headMouthAngleDeg, headNoseAngleDeg));
	AuChromoTorso torso(neckLenMM, upperTorsoLenMM, upperTorsoWidthMM, upperTorsoThickMM, lowerTorsoLenMM, lowerTorsoWidthMM,
			lowerTorsoThickMM, tailLenMM);
	AuChromoMouth mouth(mouthLenMM, mouthWidMM, mouthHtMM, mouthDepMM, lipThickMM, teeth, teethHtBottomMM, teethHtTopMM);
	if(gender == Male || gender == Hermaphrodite)
	{
		// Hair
		quint16 hairLenMM = 300;
		quint64 hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::BackOfHead) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Bangs) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Crown) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Scalp) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideRight);
		head.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));

		hairLenMM = 150;
		hairWhere =
			AuFolliclePoint::RoleBit(AuFolliclePoint::CheekLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::CheekRight) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::ChopLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::ChopRight) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::JowlLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::JowlRight) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::HeadNeckBack) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::HeadNeckFront) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnRight);
		head.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));

		hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::ChinFront) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::ChinLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::ChinRight) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::LipTop);
		mouth.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));

		hairLenMM = 15;
		hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::Groin);
		torso.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));
	}
	else
	{
		// Hair
		quint16 hairLenMM = 350;
		quint64 hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::BackOfHead) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Bangs) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Crown) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::Scalp) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideRight);
		head.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));

		hairLenMM = 200;
		hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::HeadNeckBack) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnLeft) |
			AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnRight);
		head.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));

		hairLenMM = 15;
		hairWhere = AuFolliclePoint::RoleBit(AuFolliclePoint::Groin);
		torso.AddGene(AuGeneHair(hairColor, hairLenMM, hairWhere));
	}

	// Assemble final list of chromosomes.
	AddChromosome(AuChromoChordata(AuChromoChordata::Bones));
	AddChromosome(AuChromoSkin(skinColor, skinColor));
	AddChromosome(head);
	AddChromosome(torso);
	for(int which=0; which<2; which++)
	{
		AddChromosome(AuChromoArm(AuChromoWhich(which, 2), shoulderToSpineMM, shoulderRadMM, upperArmLenMM, elbowRadMM,
					foreArmLenMM, wristRadMM, handLenMM, 0, 0));
		AddChromosome(AuChromoLeg(AuChromoWhich(which, 2), hipToSpineMM, thighLenMM, calfLenMM, buttRadMM, hipRadXMM, kneeRadXMM,
					ankleLenMM, ankleRadMM, footLenMM, footWidMM, 99));
		AddChromosome(AuChromoMammary(AuChromoWhich(which, 2), milkGland, boobSizeMM, milkYoung, true));
		AddChromosome(AuChromoEye(AuChromoWhich(which, 2), eyeFacing, eyeDecal, eyeDiaMM, eyeSocketDiaMM));
		AddChromosome(AuChromoEar(AuChromoWhich(which, 2), AuChromoEar::Circle, earDiaMM, earAngleDeg));
	}
	AddChromosome(AuChromoNose(noseLenMM, noseWidHeadMM, noseWidTipMM, noseAngleDeg));
	AddChromosome(mouth);
	AddChromosome(AuChromoLungs(ATMTYPE_Standard));
	AddChromosome(AuChromoBlood(AuChromoBlood::Warm));
	AddChromosome(AuChromoReproduction(genitalBits, litterMin, litterMax, young, birthPlace));
	AddChromosome(AuChromoEatsAnimals(eatBits));
	AddChromosome(AuChromoEatsPlants(plantPref));

	//Dump();
}

void AuChromatid::Dump() const
{
	QString chromosomes;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if(chromosomes.isEmpty())
			chromosomes += (*ti)->TypeName();
		else
			chromosomes += ", " + (*ti)->TypeName();

	qDebug() << "AuChromatid::Dump" << count() << "chromosomes" << chromosomes;
}

bool AuChromatid::operator==(const AuChromatid& dna) const
{
	// Must have the same number of chromosomes.
	if(count() != dna.count())
		return false;

	for(const_iterator ti = cbegin(), ci = dna.cbegin(); ti != cend() && ci != dna.cend(); ti++, ci++)
		if(*(*ti) != *(*ci))
			return false;
	
	return true;
}

bool AuChromatid::IsSexuallyCompatible(const AuChromatid& dna) const
{
	// Must have the same number of chromosomes.
	if(count() != dna.count())
		return false;

	// Must have the same type of chromosomes in the same order.
	for(const_iterator ti = cbegin(), ci = dna.cbegin(); ti != cend() && ci != dna.cend(); ti++, ci++)
	{
		if((*ti)->Type() != (*ci)->Type())
			return false;
		if(!(*ti)->IsSexuallyCompatible(*ci))
			return false;
	}

	return true;
}

AuChromatid AuChromatid::Combine(const AuChromatid& dna, const AuDice& dice) const
{
	// Caller has already verified that we are sexually compatible. That means we have the same number and type of traits in the
	// same order.
	AuChromatid chroma;
	const_iterator ti = cbegin();
	const_iterator di = dna.cbegin();
	while(ti != cend() && di != dna.cend())
	{
		AuChromosome* c = (*ti)->Combine(*di, dice);	// Same dice for each.
		if(c)
		{
			chroma.AddChromosome(*c);
			delete c;
		}
		ti++;
		di++;
	}
	return chroma;
}

float AuChromatid::NeckLength() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* dt = dynamic_cast<const AuChromoTorso*>(*ti);
		if(dt)
			return dt->NeckLengthMM() * FeetPerMM();
	}
	return 0;
}

int AuChromatid::MammaryCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoMammary* dt = dynamic_cast<const AuChromoMammary*>(*ti);
		if(dt)
			c++;
	}
	return c;
}

int AuChromatid::ArmCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoArm* dt = dynamic_cast<const AuChromoArm*>(*ti);
		if(dt)
			c++;
	}
	return c;
}

int AuChromatid::LegCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoLeg* dt = dynamic_cast<const AuChromoLeg*>(*ti);
		if(dt)
			c++;
	}
	return c;
}

int AuChromatid::EyeCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoEye* dt = dynamic_cast<const AuChromoEye*>(*ti);
		if(dt)
			c++;
	}
	return c;
}

float AuChromatid::FaceRadiusY(const AuChromoHead& head) const
{
	float frady = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		float trady = (*ti)->FaceRadiusY(head);
		if(frady < trady)
			frady = trady;
	}
	return frady;
}

int AuChromatid::EarCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoEar* dt = dynamic_cast<const AuChromoEar*>(*ti);
		if(dt)
			c++;
	}
	return c;
}

float AuChromatid::ArmLength() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoArm* dt = dynamic_cast<const AuChromoArm*>(*ti);
		if(dt)
			return dt->LengthMM() * FeetPerMM();
	}
	return 0;
}

float AuChromatid::LegLength() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoLeg* dt = dynamic_cast<const AuChromoLeg*>(*ti);
		if(dt)
			return dt->LengthMM() * FeetPerMM();
	}
	return 0;
}

float AuChromatid::SpineLength() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(torso)
			return (torso->LowerLengthMM() + torso->UpperLengthMM()) * FeetPerMM();
	}
	return 0;
}

float AuChromatid::TorsoLowerLength() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(torso)
			return torso->LowerLengthMM() * FeetPerMM();
	}
	return 0;
}

float AuChromatid::TorsoLowerWidth() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(torso)
			return torso->LowerWidthMM() * FeetPerMM();
	}
	return 0;
}

float AuChromatid::TorsoUpperWidth() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(torso)
			return torso->UpperWidthMM() * FeetPerMM();
	}
	return 0;
}

float AuChromatid::TorsoBottomThickness() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(torso)
			return torso->BottomThickness();
	}
	return 1;
}

void AuChromatid::GetSkullFeatures(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti)->GetSkullFeatures(*this, mesh, mSkull, head);
}

void AuChromatid::GetSkullFeaturesClothed(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti)->GetSkullFeaturesClothed(*this, mesh, mSkull, head);
}

void AuChromatid::GetSkullJaw(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head, quint8 bone) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti)->GetSkullJaw(*this, mesh, mSkull, head, bone))
			break;	// We only expect one jaw per skull.
}

void AuChromatid::GetSkullEyes(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head, quint8 firstBone) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti)->GetSkullEye(*this, mesh, mSkull, head, firstBone))
			firstBone++;	// There can be multiple eyes per skull.
}

void AuChromatid::GetTorsoFeatures(AuMeatMesh& mesh, const AuMatrix4x4& mTorso, const AuChromoTorso& torso, GROWTHSTAGE stage) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti)->GetTorsoFeatures(*this, mesh, mTorso, torso, stage);
}

void AuChromatid::GetTorsoFeaturesClothed(AuMeatMesh& mesh, const AuMatrix4x4& mTorso, const AuChromoTorso& torso,
		GROWTHSTAGE stage) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti)->GetTorsoFeaturesClothed(*this, mesh, mTorso, torso, stage);
}

quint8 AuChromatid::ButtBone() const
{
	quint8 buttBone = 0;
	quint8 boneIndex = 0;
	AuMeatMesh mesh;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		quint8 bones = (*ti)->BoneCount(*this);
		boneIndex += bones;
		const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(*ti);
		if(!torso)
			continue;
		buttBone = boneIndex - bones;	// First bone of the torso is the butt end.
	}
	return buttBone;
}

void AuChromatid::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		out << (quint8)(*ti)->Type();
		out << **ti;
	}
}

void AuChromatid::ReadStream(QDataStream& in, quint8 fileVer)
{
	Clear();

	quint16 chromosomes;
	in >> chromosomes;
	quint8 type;
	while(chromosomes--)
	{
		in >> type;
		AuChromosome* chromosome = AuChromosome::CreateChromosome((AuChromosome::CTYPE)type);
		if(chromosome)
		{
			chromosome->ReadStream(in, fileVer);
			append(chromosome);
			//qDebug() << "AuChromatid::ReadStream read" << chromosome->TypeName();
		}
		else
			qWarning() << "AuChromatid::ReadStream received invalid chromosome type" << type;
	}
}

void AuChromatid::WriteSpeciesKey(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		out << (quint8)(*ti)->Type();
		(*ti)->WriteSpeciesKey(out);
	}
}

void AuChromatid::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	Clear();

	quint16 chromosomes;
	in >> chromosomes;
	quint8 type;
	while(chromosomes--)
	{
		in >> type;
		AuChromosome* chromosome = AuChromosome::CreateChromosome((AuChromosome::CTYPE)type);
		if(chromosome)
		{
			chromosome->ReadSpeciesKey(in, fileVer);
			append(chromosome);
			//qDebug() << "AuChromatid::ReadSpeciesKey read" << chromosome->TypeName();
		}
		else
			qWarning() << "AuChromatid::ReadSpeciesKey received invalid chromosome type" << type;
	}
}

void AuChromatid::RandomizeSpecies(const AuDice& dice)
{
	for(iterator ti = begin(); ti != end(); ti++)
		(*ti)->RandomizeSpecies(dice);	// Same dice for each.
}

void AuChromatid::MakeParts(QList<AuAnimalPart*>& parts) const
{
	// Model is created facing along the +Y axis, +Z up, +X right, ready for bending.
	quint8 boneIndex = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		AuAnimalPart* part = (*ti)->MakePart(*this, boneIndex);
		if(part)
			parts.append(part);
	}
}

float AuChromatid::ClothingBustAltitude(const AuChromoWhich& which) const
{
	const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(Chromosome(AuChromosome::Torso));
	if(!torso)
		return 0;
	AuPoint breastLoc(torso->BreastLocationNotOnSkin(*this, which));
	return breastLoc.z;
}

float AuChromatid::ClothingGroinAltitude() const
{
	return (-5.f) * FeetPerMM() * 1.2f;
}

float AuChromatid::ClothingWaistAltitude() const
{
	const AuChromoLeg* leg = dynamic_cast<const AuChromoLeg*>(Chromosome(AuChromosome::Leg));
	if(!leg)
		return TorsoLowerLength();
	return (leg->ButtRadiusMM() + 5) * FeetPerMM() * 1.2f;
}

float AuChromatid::AnkleLength() const
{
	const AuChromoLeg* leg = dynamic_cast<const AuChromoLeg*>(Chromosome(AuChromosome::Leg));
	if(!leg)
		return 0;
	return leg->AnkleLengthMM() * FeetPerMM();
}

float AuChromatid::CalfLength() const
{
	const AuChromoLeg* leg = dynamic_cast<const AuChromoLeg*>(Chromosome(AuChromosome::Leg));
	if(!leg)
		return 0;
	return leg->CalfLengthMM() * FeetPerMM();
}

float AuChromatid::ThighLength() const
{
	int legs = LegCount();
	if(!legs)
		return 0;
	AuChromoWhich which(legs-1, legs);
	const AuChromoLeg* leg = dynamic_cast<const AuChromoLeg*>(Chromosome(AuChromosome::Leg, which));
	if(!leg)
		return 0;
	return leg->ThighLengthMM() * FeetPerMM();
}

float AuChromatid::KneeAltitude() const
{
	const AuChromoLeg* leg = dynamic_cast<const AuChromoLeg*>(Chromosome(AuChromosome::Leg));
	if(!leg)
		return 0;
	return leg->KneeAltitude(*this);
}

void AuChromatid::GetSkullHair(float lockWid, AuFolliclePointArray& follicles, QMap<AuPoint, double>& hairEx,
		const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti)->GetSkullHair(*this, lockWid, follicles, hairEx, mSkull, head, stage);
}

bool AuChromatid::HasBreasts() const
{
	const AuChromoMammary* mam = dynamic_cast<const AuChromoMammary*>(Chromosome(AuChromosome::Mammary));
	return mam->Organ() == AuChromoMammary::Breast;
}

bool AuChromatid::HasFemaleGenitals() const
{
	const AuChromoReproduction* repro = dynamic_cast<const AuChromoReproduction*>(Chromosome(AuChromosome::Reproduction));
	if(!repro)
		return false;
	return repro->HasFemaleGenitals();
}

bool AuChromatid::HasMaleGenitals() const
{
	const AuChromoReproduction* repro = dynamic_cast<const AuChromoReproduction*>(Chromosome(AuChromosome::Reproduction));
	if(!repro)
		return false;
	return repro->HasMaleGenitals();
}

void AuChromatid::SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor,
		QString& bottomSkinTexId, QString& bottomSkinTexFile, QColor& bottomSkinColor,
		QString& decalTexId, QString& decalTexFile) const
{
	const AuChromoSkin* skin = dynamic_cast<const AuChromoSkin*>(Chromosome(AuChromosome::Skin));
	if(!skin)
		return;
	skin->SkinRenderParameters(topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile, bottomSkinColor,
			decalTexId, decalTexFile);
}

void AuChromatid::ApplyChanges(const AuChromosome* t)
{
	// Find the corresponding chromosome.
	for(iterator ti = begin(); ti != end(); ti++)
	{
		if((*ti)->Type() != t->Type())
			continue;
		if((*ti)->Which() != t->Which())
			continue;
		// Replace the chromosome, then I don't have to write an operator= override. :P
		delete *ti;
		*ti = t->Clone();
		break;
	}
}

void AuChromatid::Remove(AuChromosome::CTYPE chromosome, const AuChromoWhich& which)
{
	for(iterator ti = begin(); ti != end(); ti++)
	{
		if((*ti)->Type() != chromosome)
			continue;
		if((*ti)->Which() != which)
			continue;
		delete *ti;
		erase(ti);
		break;
	}
	UpdateChromosomeWhichCounts(chromosome);
}

AuChromoWhich AuChromatid::AvailableWhich(AuChromosome::CTYPE chromosome) const
{
	// Let us put an artificial limit on the number of any given chromosome, to conserve bones.
	int chromosomeCount = ChromosomeCount(chromosome);	// Current whiches are based on current chromosome count.
	if(!chromosomeCount)
		return AuChromoWhich(0, 1);		// No chromosomes of that type yet.
	for(unsigned int index=0; index<AuChromoWhich::MaxCount(); index++)
	{
		AuChromoWhich which(index, chromosomeCount);
		if(!Chromosome(chromosome, which))
			return which;				// Return the one we did not find.
	}
	return AuChromoWhich();				// Default constructor makes an invalid which.
}

AuPoint AuChromatid::ArmLocation(const AuChromoWhich& which) const
{
	// Arms start at the top of the spine and get spaced down the spine according to their shoulder radii. The first arm is always
	// at the exact end of the spine.
	int row = which.Index()/2;			// Start at shoulders and work our way down.
	AuChromosome::LATERALITY laterality = AuChromoArm::Laterality(which);
	double shouldersAheadOfMe = 0;
	double shoulderToSpine = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoArm* dt = dynamic_cast<const AuChromoArm*>(*ti);
		if(dt)
		{
			int arow = dt->Which().Index()/2;
			if(arow < row && dt->Laterality() == laterality)
			{
				// Found an arm that is ahead of me on my side of the spine.
				if(arow == 0)
					shouldersAheadOfMe += dt->ShoulderRadiusMM() * FeetPerMM();		// Radius of the first row of arms.
				else
					shouldersAheadOfMe += dt->ShoulderRadiusMM() * FeetPerMM() * 2;	// Diameter of intermediate rows of arms.
			}
			else if(dt->Which() == which)
			{
				if(row)
					shouldersAheadOfMe += dt->ShoulderRadiusMM() * FeetPerMM();		// Radius of my own shoulder.
				shoulderToSpine = dt->ShoulderToSpineMM() * FeetPerMM();
			}
		}
	}
	if(laterality == AuChromosome::Left)
		shoulderToSpine = -shoulderToSpine;
	double spineHt = NeckAltitude() - shouldersAheadOfMe;	// Height above base of spine. NeckAltitude is top of spine.
	return AuPoint(shoulderToSpine, 0, spineHt);
}

AuPoint AuChromatid::LegLocation(const AuChromoWhich& which) const
{
	// Get information about legs.
	int legs = 0;
	AuChromosome::LATERALITY laterality = AuChromoArm::Laterality(which);
	double hipToSpine = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuChromoLeg* dt = dynamic_cast<const AuChromoLeg*>(*ti);
		if(dt)
		{
			legs++;
			if(dt->Which() == which)
			{
				hipToSpine = dt->HipToSpineMM() * FeetPerMM();
			}
		}
	}
	if(laterality == AuChromosome::Left)
		hipToSpine = -hipToSpine;

	// When there's only one or two legs, they go at the bottom of the spine.
	double legHt = 0;
	int rows = (legs+1)/2;
	if(rows <= 1)
		return AuPoint(hipToSpine, 0, legHt);

	// Make pairs of legs down the lower spine. If there is an odd leg, it is the last (hindmost) row, for a Pearson's Puppeteer.
	int row = which.Index()/2;
	double step = TorsoLowerLength() / (rows-1);
	legHt += TorsoLowerLength() - step * row;

	// First leg pair is always at the bottom of the spine, ignoring tails for the moment.
	return AuPoint(hipToSpine, 0, legHt);
}

void AuChromatid::UpdateChromosomeWhichCounts()
{
	QSet<AuChromosome::CTYPE> chromosomes;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		chromosomes.insert((*ti)->Type());
	for(QSet<AuChromosome::CTYPE>::const_iterator di = chromosomes.cbegin(); di != chromosomes.cend(); di++)
		UpdateChromosomeWhichCounts(*di);
}

void AuChromatid::UpdateChromosomeWhichCounts(AuChromosome::CTYPE chromosome)
{
	unsigned int chromosomeCount = ChromosomeCount(chromosome);
	if(!chromosomeCount)
		return;							// Nothing to do.
	for(iterator ti = begin(); ti != end(); ti++)
		if((*ti)->Type() == chromosome)
		{
			AuChromoWhich which = (*ti)->Which();
			which.SetHowMany(chromosomeCount);
			(*ti)->SetWhich(which);
		}
}

AuChromosome::CTYPE AuChromatid::Phylum() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		AuChromosome::CTYPE p = (*ti)->Phylum();
		if(p != AuChromosome::CTYPE_Count)
			return p;
	}
	return AuChromosome::CTYPE_Count;
}

bool AuChromatid::IsPredator() const
{
	const AuChromoEatsAnimals* carnivore = dynamic_cast<const AuChromoEatsAnimals*>(Chromosome(AuChromosome::EatsAnimals));
	if(!carnivore)
		return false;
	return carnivore->IsPredator();
}

GENDER AuChromatid::Gender() const
{
	const AuChromoReproduction* repro = dynamic_cast<const AuChromoReproduction*>(Chromosome(AuChromosome::Reproduction));
	if(!repro)
		return Asexual;
	return repro->Gender();
}

float AuChromatid::HearingRange() const
{
	return 200;	//DHH animalia
}

float AuChromatid::SightRange() const
{
	return 1000;	//DHH animalia
}

float AuChromatid::SmellRange() const
{
	return 100;	//DHH animalia
}

float AuChromatid::FieldOfViewAngle() const
{
	return (90.f + 15.f) * DEG2RADf / 2.f;	//DHH animalia
}

ECOROLE AuChromatid::EcoRole() const
{
	const AuChromoEatsAnimals* eatsA = dynamic_cast<const AuChromoEatsAnimals*>(Chromosome(AuChromosome::EatsAnimals));
	if(eatsA && eatsA->IsScavenger())
		return ECO_Scavenger;

	const AuChromoEatsPlants* eatsP = dynamic_cast<const AuChromoEatsPlants*>(Chromosome(AuChromosome::EatsPlants));
	if(eatsA && eatsP)
		return ECO_Omnivore;
	else if(eatsP)
		return ECO_Herbivore;
	else
		return ECO_Carnivore;
}

ATMTYPE AuChromatid::BreathesAir() const
{
	const AuChromoLungs* lungs = dynamic_cast<const AuChromoLungs*>(Chromosome(AuChromosome::Lungs));
	if(!lungs)
		return ATMTYPE_Count;
	return lungs->AtmosphereType();
}

HYDROGRAPHIC_LIQUID AuChromatid::BreathesWater() const
{
	const AuChromoGills* gills = dynamic_cast<const AuChromoGills*>(Chromosome(AuChromosome::Gills));
	if(!gills)
		return HYDRO_None;
	if(gills->SeaType() == AuChromoGills::SupercooledLiquid)
		return HYDRO_SupercooledLiquid;
	return HYDRO_Water;
}

float AuChromatid::BiteRange() const
{
	const AuChromoMouth* mouth = dynamic_cast<const AuChromoMouth*>(Chromosome(AuChromosome::Mouth));
	if(!mouth)
		return 0;						// Cannot bite with no mouth.
	const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(Chromosome(AuChromosome::Head));
	if(!head)
		return 0;						// No head? :/
	AuPoint mouthLoc(head->MouthLocation());	// Relative to the head.
	float r = mouthLoc.y + mouth->LengthMM() * FeetPerMM();
	const AuChromoTorso* torso = dynamic_cast<const AuChromoTorso*>(Chromosome(AuChromosome::Torso));
	if(torso)
		r += (torso->UpperLengthMM() + torso->NeckLengthMM()) * FeetPerMM();
	return r;
}

float AuChromatid::Volume() const
{
	float v = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		v += (*ti)->Volume();
	return v;
}

float AuChromatid::JumpForce() const
{
	// Jump force is based on leg volume vs body volume.
	float v = 0;
	float lv = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		float tv = (*ti)->Volume();
		if(tv == 0)
			continue;
		v += tv;
		if(dynamic_cast<const AuChromoLeg*>(*ti))
			lv += tv;
	}
	if(lv == 0 || v == 0)
		return 0;

	float legRatio = lv / v;			// Ratio of leg volume to overall body volume.
	float jump = legRatio * 50;			//DHH jump force tuning parm
	return jump;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Structure
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoChordata::AuChromoChordata(AuChromoChordata::SKELETON skeleton)
{
	m_Skeleton = skeleton;
}

AuChromoChordata::AuChromoChordata()
{
	m_Skeleton = Bones;
}

AuChromoChordata::AuChromoChordata(const AuChromoChordata& c) : AuChromosome(c)
{
	m_Skeleton = c.m_Skeleton;
}

AuChromoChordata::AuChromoChordata(const AuChromoChordata& c1, const AuChromoChordata& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_Skeleton = c1.m_Skeleton;
}

void AuChromoChordata::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoChordata& c = dynamic_cast<const AuChromoChordata&>(o);
		m_Skeleton = c.m_Skeleton;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoChordata::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_Skeleton;
}

void AuChromoChordata::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Skeleton = (SKELETON)ui8;
}

void AuChromoChordata::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_Skeleton;
}

void AuChromoChordata::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Skeleton = (SKELETON)ui8;
}

void AuChromoChordata::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoChordata::ListWidgetOrder() const
{
	return 10;
}

QString AuChromoChordata::ListWidgetText() const
{
	return QObject::tr("Chordata<br>"
			"<small>%1</small>").arg(SkeletonName());
}

QWidget* AuChromoChordata::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageChordata* pp = new AuChromoPageChordata(parent);
	return pp;
}

//static
QString AuChromoChordata::SkeletonName(SKELETON skel)
{
	if(skel < 0 || skel > Cartilage)
		return QObject::tr("Invalid Skeleton");
	QString skels[] =
	{
		QObject::tr("Bones"),
		QObject::tr("Cartilage"),
	};
	return skels[skel];
}

bool AuChromoChordata::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoChordata* c = (const AuChromoChordata*)chromosome;
	if(m_Skeleton != c->m_Skeleton)
		return false;
	return true;
}

AuChromosome* AuChromoChordata::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoChordata* c = dynamic_cast<const AuChromoChordata*>(g);
	if(!c)
		return 0;
	AuChromoChordata* comb = new AuChromoChordata(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoAnnelida::AuChromoAnnelida()
{
}

AuChromoAnnelida::AuChromoAnnelida(const AuChromoAnnelida& c) : AuChromosome(c)
{
}

AuChromoAnnelida::AuChromoAnnelida(const AuChromoAnnelida& c1, const AuChromoAnnelida& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
}

void AuChromoAnnelida::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoAnnelida& c = dynamic_cast<const AuChromoAnnelida&>(o);
		Q_UNUSED(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoAnnelida::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
}

void AuChromoAnnelida::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
}

void AuChromoAnnelida::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
}

void AuChromoAnnelida::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
}

void AuChromoAnnelida::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoAnnelida::ListWidgetOrder() const
{
	return 10;
}

QString AuChromoAnnelida::ListWidgetText() const
{
	return QObject::tr("Annelida");
}

QWidget* AuChromoAnnelida::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageAnnelida* pp = new AuChromoPageAnnelida(parent);
	return pp;
}

bool AuChromoAnnelida::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoAnnelida* c = (const AuChromoAnnelida*)chromosome;
	return true;
}

AuChromosome* AuChromoAnnelida::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoAnnelida* c = dynamic_cast<const AuChromoAnnelida*>(g);
	if(!c)
		return 0;
	AuChromoAnnelida* comb = new AuChromoAnnelida(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoArthropoda::AuChromoArthropoda()
{
}

AuChromoArthropoda::AuChromoArthropoda(const AuChromoArthropoda& c) : AuChromosome(c)
{
}

AuChromoArthropoda::AuChromoArthropoda(const AuChromoArthropoda& c1, const AuChromoArthropoda& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
}

void AuChromoArthropoda::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoArthropoda& c = dynamic_cast<const AuChromoArthropoda&>(o);
		Q_UNUSED(c);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoArthropoda::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
}

void AuChromoArthropoda::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
}

void AuChromoArthropoda::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
}

void AuChromoArthropoda::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
}

void AuChromoArthropoda::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoArthropoda::ListWidgetOrder() const
{
	return 10;
}

QString AuChromoArthropoda::ListWidgetText() const
{
	return QObject::tr("Arthropoda");
}

QWidget* AuChromoArthropoda::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageArthropoda* pp = new AuChromoPageArthropoda(parent);
	return pp;
}

bool AuChromoArthropoda::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoArthropoda* c = (const AuChromoArthropoda*)chromosome;
	return true;
}

AuChromosome* AuChromoArthropoda::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoArthropoda* c = dynamic_cast<const AuChromoArthropoda*>(g);
	if(!c)
		return 0;
	AuChromoArthropoda* comb = new AuChromoArthropoda(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoMollusca::AuChromoMollusca(AuChromoMollusca::COVER cover)
{
	m_Cover = cover;
}

AuChromoMollusca::AuChromoMollusca()
{
	m_Cover = Shell;
}

AuChromoMollusca::AuChromoMollusca(const AuChromoMollusca& c) : AuChromosome(c)
{
	m_Cover = c.m_Cover;
}

AuChromoMollusca::AuChromoMollusca(const AuChromoMollusca& c1, const AuChromoMollusca& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_Cover = c1.m_Cover;
}

void AuChromoMollusca::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoMollusca& c = dynamic_cast<const AuChromoMollusca&>(o);
		m_Cover = c.m_Cover;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoMollusca::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_Cover;
}

void AuChromoMollusca::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Cover = (COVER)ui8;
}

void AuChromoMollusca::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_Cover;
}

void AuChromoMollusca::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Cover = (COVER)ui8;
}

void AuChromoMollusca::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoMollusca::ListWidgetOrder() const
{
	return 10;
}

QString AuChromoMollusca::ListWidgetText() const
{
	return QObject::tr("Mollusca<br>"
			"<small>%1</small>").arg(CoverName());
}

QWidget* AuChromoMollusca::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageMollusca* pp = new AuChromoPageMollusca(parent);
	return pp;
}

//static
QString AuChromoMollusca::CoverName(COVER cover)
{
	if(cover < 0 || cover > Shell)
		return QObject::tr("Invalid Cover");
	QString covers[] =
	{
		QObject::tr("Mantle"),
		QObject::tr("Shell"),
	};
	return covers[cover];
}

bool AuChromoMollusca::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoMollusca* c = (const AuChromoMollusca*)chromosome;
	if(m_Cover != c->m_Cover)
		return false;
	return true;
}

AuChromosome* AuChromoMollusca::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoMollusca* c = dynamic_cast<const AuChromoMollusca*>(g);
	if(!c)
		return 0;
	AuChromoMollusca* comb = new AuChromoMollusca(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Body Covering
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoSkin::AuChromoSkin(const QColor& topColor, const QColor& bottomColor)
{
	m_BottomColor = bottomColor;
	m_TopColor = topColor;
}

AuChromoSkin::AuChromoSkin()
{
	m_BottomColor = Color(0);
	m_TopColor = Color(0);
}

AuChromoSkin::AuChromoSkin(const AuChromoSkin& c) : AuChromosome(c)
{
	m_BottomColor = c.m_BottomColor;
	m_TopColor = c.m_TopColor;
}

AuChromoSkin::AuChromoSkin(const AuChromoSkin& c1, const AuChromoSkin& c2, const AuDice& Cdice) :
	AuChromosome(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_BottomColor = CombineColor(c1.m_BottomColor, c2.m_BottomColor, dice);
	dice.Random();
	m_TopColor = CombineColor(c1.m_TopColor, c2.m_TopColor, dice);
}

void AuChromoSkin::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoSkin& c = dynamic_cast<const AuChromoSkin&>(o);
		m_BottomColor = c.m_BottomColor;
		m_TopColor = c.m_TopColor;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoSkin::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_BottomColor;
	out << m_TopColor;
}

void AuChromoSkin::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_BottomColor;
	in >> m_TopColor;
}

void AuChromoSkin::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << ToColor32(m_BottomColor);
	out << ToColor32(m_TopColor);
}

void AuChromoSkin::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	m_BottomColor = ReadColor32(in);
	m_TopColor = ReadColor32(in);
}

void AuChromoSkin::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoSkin::ListWidgetOrder() const
{
	return 20;
}

QString AuChromoSkin::ListWidgetText() const
{
	QString topName, bottomName;
	for(int c=0; c<Colors(); c++)
	{
		if(Color(c) == m_BottomColor)
			bottomName = ColorName(c);
		if(Color(c) == m_TopColor)
			topName = ColorName(c);
	}

	if(bottomName.isEmpty())
		bottomName = m_BottomColor.name();
	if(topName.isEmpty())
		topName = m_TopColor.name();

	if(topName == bottomName)
		return QObject::tr("Skin<br><small>Color %1</small>")
			.arg(topName);
	else
		return QObject::tr("Skin<br><small>Top Color %1<br>Bottom Color %2</small>")
			.arg(topName)
			.arg(bottomName);
}

QWidget* AuChromoSkin::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageSkin* pp = new AuChromoPageSkin(parent);
	return pp;
}

void AuChromoSkin::SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
		QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const
{
	topSkinTexId = QString("DNASkinSmooth");
	topSkinTexFile = QString(":data/SkinSmooth.png");
	topSkinColor = m_TopColor;

	bottomSkinTexId = topSkinTexId;
	bottomSkinTexFile = topSkinTexFile;
	bottomSkinColor = m_BottomColor;

	decalTexId = QString("DNASkinDecal");
	decalTexFile = QString(":data/SkinDecal.png");
}

//static
QString AuChromoSkin::ColorName(int color)
{
	if(color < 0 || color >= s_SkinColors)
		return QString();
	return s_SkinColor[color].Name();
}

//static
QColor AuChromoSkin::Color(int color)
{
	if(color < 0 || color >= s_SkinColors)
		return QColor(1,1,1,1);
	return s_SkinColor[color].Color();
}

//static
int AuChromoSkin::Colors()
{
	return s_SkinColors;
}

bool AuChromoSkin::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoSkin* c = (const AuChromoSkin*)chromosome;
	return true;
}

AuChromosome* AuChromoSkin::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoSkin* c = dynamic_cast<const AuChromoSkin*>(g);
	if(!c)
		return 0;
	AuChromoSkin* comb = new AuChromoSkin(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoFeathers::AuChromoFeathers(const QColor& topColor, const QColor& bottomColor)
{
	m_BottomColor = bottomColor;
	m_TopColor = topColor;
}

AuChromoFeathers::AuChromoFeathers()
{
}

AuChromoFeathers::AuChromoFeathers(const AuChromoFeathers& c) : AuChromosome(c)
{
	m_BottomColor = c.m_BottomColor;
	m_TopColor = c.m_TopColor;
}

AuChromoFeathers::AuChromoFeathers(const AuChromoFeathers& c1, const AuChromoFeathers& c2, const AuDice& Cdice) :
	AuChromosome(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_BottomColor = CombineColor(c1.m_BottomColor, c2.m_BottomColor, dice);
	dice.Random();
	m_TopColor = CombineColor(c1.m_TopColor, c2.m_TopColor, dice);
}

void AuChromoFeathers::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoFeathers& c = dynamic_cast<const AuChromoFeathers&>(o);
		m_BottomColor = c.m_BottomColor;
		m_TopColor = c.m_TopColor;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoFeathers::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_BottomColor;
	out << m_TopColor;
}

void AuChromoFeathers::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_BottomColor;
	in >> m_TopColor;
}

void AuChromoFeathers::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << ToColor32(m_BottomColor);
	out << ToColor32(m_TopColor);
}

void AuChromoFeathers::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	m_BottomColor = ReadColor32(in);
	m_TopColor = ReadColor32(in);
}

void AuChromoFeathers::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoFeathers::ListWidgetOrder() const
{
	return 20;
}

QString AuChromoFeathers::ListWidgetText() const
{
	QString topName, bottomName;
	if(bottomName.isEmpty())
		bottomName = m_BottomColor.name();
	if(topName.isEmpty())
		topName = m_TopColor.name();

	if(topName == bottomName)
		return QObject::tr("Feathers<br><small>Color %1</small>")
			.arg(topName);
	else
		return QObject::tr("Feathers<br><small>Top Color %1<br>Bottom Color %2</small>")
			.arg(topName)
			.arg(bottomName);
}

QWidget* AuChromoFeathers::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageFeathers* pp = new AuChromoPageFeathers(parent);
	return pp;
}

void AuChromoFeathers::SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor,
		QString& bottomSkinTexId, QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId,
		QString& decalTexFile) const
{
	topSkinTexId = QString("DNASkinSmooth");
	topSkinTexFile = QString(":data/SkinSmooth.png");
	topSkinColor = m_TopColor;

	bottomSkinTexId = topSkinTexId;
	bottomSkinTexFile = topSkinTexFile;
	bottomSkinColor = m_BottomColor;

	decalTexId = QString("DNASkinDecal");
	decalTexFile = QString(":data/SkinDecal.png");
}

bool AuChromoFeathers::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoFeathers* c = (const AuChromoFeathers*)chromosome;
	return true;
}

AuChromosome* AuChromoFeathers::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoFeathers* c = dynamic_cast<const AuChromoFeathers*>(g);
	if(!c)
		return 0;
	AuChromoFeathers* comb = new AuChromoFeathers(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoFur::AuChromoFur(const QColor& topColor, const QColor& bottomColor)
{
	m_BottomColor = bottomColor;
	m_TopColor = topColor;
}

AuChromoFur::AuChromoFur()
{
	m_BottomColor = Color(0);
	m_TopColor = Color(0);
}

AuChromoFur::AuChromoFur(const AuChromoFur& c) : AuChromosome(c)
{
	m_BottomColor = c.m_BottomColor;
	m_TopColor = c.m_TopColor;
}

AuChromoFur::AuChromoFur(const AuChromoFur& c1, const AuChromoFur& c2, const AuDice& Cdice) :
	AuChromosome(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_BottomColor = CombineColor(c1.m_BottomColor, c2.m_BottomColor, dice);
	dice.Random();
	m_TopColor = CombineColor(c1.m_TopColor, c2.m_TopColor, dice);
}

void AuChromoFur::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoFur& c = dynamic_cast<const AuChromoFur&>(o);
		m_BottomColor = c.m_BottomColor;
		m_TopColor = c.m_TopColor;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoFur::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_BottomColor;
	out << m_TopColor;
}

void AuChromoFur::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_BottomColor;
	in >> m_TopColor;
}

void AuChromoFur::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << ToColor32(m_BottomColor);
	out << ToColor32(m_TopColor);
}

void AuChromoFur::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	m_BottomColor = ReadColor32(in);
	m_TopColor = ReadColor32(in);
}

void AuChromoFur::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoFur::ListWidgetOrder() const
{
	return 20;
}

QString AuChromoFur::ListWidgetText() const
{
	QString topName, bottomName;
	for(int c=0; c<Colors(); c++)
	{
		if(Color(c) == m_BottomColor)
			bottomName = ColorName(c);
		if(Color(c) == m_TopColor)
			topName = ColorName(c);
	}

	if(bottomName.isEmpty())
		bottomName = m_BottomColor.name();
	if(topName.isEmpty())
		topName = m_TopColor.name();

	if(topName == bottomName)
		return QObject::tr("Fur<br><small>Color %1</small>")
			.arg(topName);
	else
		return QObject::tr("Fur<br><small>Top Color %1<br>Bottom Color %2</small>")
			.arg(topName)
			.arg(bottomName);
}

QWidget* AuChromoFur::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageFur* pp = new AuChromoPageFur(parent);
	return pp;
}

void AuChromoFur::SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
		QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const
{
	topSkinTexId = QString("DNASkinSmooth");
	topSkinTexFile = QString(":data/SkinSmooth.png");
	topSkinColor = m_TopColor;

	bottomSkinTexId = topSkinTexId;
	bottomSkinTexFile = topSkinTexFile;
	bottomSkinColor = m_BottomColor;

	decalTexId = QString("DNASkinDecal");
	decalTexFile = QString(":data/SkinDecal.png");
}

//static
QString AuChromoFur::ColorName(int color)
{
	if(color < 0 || color >= s_HairColors)
		return QString();
	return s_HairColor[color].Name();
}

//static
QColor AuChromoFur::Color(int color)
{
	if(color < 0 || color >= s_HairColors)
		return QColor(1,1,1,1);
	return s_HairColor[color].Color();
}

//static
int AuChromoFur::Colors()
{
	return s_HairColors;
}

bool AuChromoFur::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoFur* c = (const AuChromoFur*)chromosome;
	return true;
}

AuChromosome* AuChromoFur::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoFur* c = dynamic_cast<const AuChromoFur*>(g);
	if(!c)
		return 0;
	AuChromoFur* comb = new AuChromoFur(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoScales::AuChromoScales(const QColor& color, quint8 shape)
{
	m_Color = color;
	m_Shape = shape;
}

AuChromoScales::AuChromoScales()
{
	m_Shape = 0;
}

AuChromoScales::AuChromoScales(const AuChromoScales& c) : AuChromosome(c)
{
	m_Color = c.m_Color;
	m_Shape = c.m_Shape;
}

AuChromoScales::AuChromoScales(const AuChromoScales& c1, const AuChromoScales& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_Color = CombineColor(c1.m_Color, c2.m_Color, dice);
	m_Shape = c1.m_Shape;
}

void AuChromoScales::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoScales& c = dynamic_cast<const AuChromoScales&>(o);
		m_Color = c.m_Color;
		m_Shape = c.m_Shape;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoScales::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_Color;
	out << m_Shape;
}

void AuChromoScales::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_Color;
	in >> m_Shape;
}

void AuChromoScales::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << ToColor32(m_Color);
	out << m_Shape;
}

void AuChromoScales::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	m_Color = ReadColor32(in);
	in >> m_Shape;
}

void AuChromoScales::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoScales::ListWidgetOrder() const
{
	return 20;
}

QString AuChromoScales::ListWidgetText() const
{
	return QObject::tr("Scales");
}

QWidget* AuChromoScales::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageScales* pp = new AuChromoPageScales(parent);
	return pp;
}

bool AuChromoScales::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoScales* c = (const AuChromoScales*)chromosome;
	if(m_Shape != c->m_Shape)
		return false;
	return true;
}

AuChromosome* AuChromoScales::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoScales* c = dynamic_cast<const AuChromoScales*>(g);
	if(!c)
		return 0;
	AuChromoScales* comb = new AuChromoScales(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Body Sections.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoHead::AuChromoHead(ATTACHMENT attach, quint16 attachDistMM, quint16 widthMM, quint16 lengthMM, quint16 heightMM,
		qint16 earAngleDeg, qint16 eyeAngleDeg, qint16 mouthAngleDeg, qint16 noseAngleDeg)
{
	m_Attach = attach;
	m_AttachDistMM = attachDistMM;
	m_EarAngleDeg = earAngleDeg;
	m_EyeAngleDeg = eyeAngleDeg;
	m_MouthAngleDeg = mouthAngleDeg;
	m_NoseAngleDeg = noseAngleDeg;
	m_WidthMM = widthMM;
	m_LengthMM = lengthMM;
	m_HeightMM = heightMM;
}

AuChromoHead::AuChromoHead()
{
	m_Attach = AttachBottom;
	m_AttachDistMM = 40;
	m_EarAngleDeg = 90;
	m_EyeAngleDeg = 30;
	m_MouthAngleDeg = 135;
	m_NoseAngleDeg = 90;
	m_WidthMM = 100;
	m_LengthMM = 100;
	m_HeightMM = 100;
}

AuChromoHead::AuChromoHead(const AuChromoHead& c) : AuChromosomeWithGenes(c)
{
	m_Attach = c.m_Attach;
	m_AttachDistMM = c.m_AttachDistMM;
	m_EarAngleDeg = c.m_EarAngleDeg;
	m_EyeAngleDeg = c.m_EyeAngleDeg;
	m_MouthAngleDeg = c.m_MouthAngleDeg;
	m_NoseAngleDeg = c.m_NoseAngleDeg;
	m_WidthMM = c.m_WidthMM;
	m_LengthMM = c.m_LengthMM;
	m_HeightMM = c.m_HeightMM;
}

AuChromoHead::AuChromoHead(const AuChromoHead& c1, const AuChromoHead& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_Attach = c1.m_Attach;
	m_AttachDistMM = CombineRange(c1.m_AttachDistMM, c2.m_AttachDistMM, dice);
	dice.Random();
	m_EarAngleDeg = CombineAngle(c1.m_EarAngleDeg, c2.m_EarAngleDeg, dice);
	dice.Random();
	m_EyeAngleDeg = CombineAngle(c1.m_EyeAngleDeg, c2.m_EyeAngleDeg, dice);
	dice.Random();
	m_MouthAngleDeg = CombineAngle(c1.m_MouthAngleDeg, c2.m_MouthAngleDeg, dice);
	dice.Random();
	m_NoseAngleDeg = CombineAngle(c1.m_NoseAngleDeg, c2.m_NoseAngleDeg, dice);
	dice.Random();
	m_WidthMM = CombineRange(c1.m_WidthMM, c2.m_WidthMM, dice);
	dice.Random();
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	dice.Random();
	m_HeightMM = CombineRange(c1.m_HeightMM, c2.m_HeightMM, dice);
}

void AuChromoHead::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoHead& c = dynamic_cast<const AuChromoHead&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_Attach = c.m_Attach;
		m_AttachDistMM = c.m_AttachDistMM;
		m_EarAngleDeg = c.m_EarAngleDeg;
		m_EyeAngleDeg = c.m_EyeAngleDeg;
		m_MouthAngleDeg = c.m_MouthAngleDeg;
		m_NoseAngleDeg = c.m_NoseAngleDeg;
		m_WidthMM = c.m_WidthMM;
		m_LengthMM = c.m_LengthMM;
		m_HeightMM = c.m_HeightMM;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoHead::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << (quint8)m_Attach;
	out << m_AttachDistMM;
	out << m_EarAngleDeg;
	out << m_EyeAngleDeg;
	out << m_MouthAngleDeg;
	out << m_NoseAngleDeg;
	out << m_WidthMM;
	out << m_LengthMM;
	out << m_HeightMM;
}

void AuChromoHead::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Attach = (ATTACHMENT)ui8;
	in >> m_AttachDistMM;
	in >> m_EarAngleDeg;
	in >> m_EyeAngleDeg;
	in >> m_MouthAngleDeg;
	in >> m_NoseAngleDeg;
	in >> m_WidthMM;
	in >> m_LengthMM;
	in >> m_HeightMM;
}

void AuChromoHead::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << (quint8)m_Attach;
	out << ToLogByte(m_AttachDistMM, d_DNAMaxMM);
	out << ToAngleByte(m_EarAngleDeg);
	out << ToAngleByte(m_EyeAngleDeg);
	out << ToAngleByte(m_MouthAngleDeg);
	out << ToAngleByte(m_NoseAngleDeg);
	out << ToLogByte(m_WidthMM, d_DNAMaxMM);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << ToLogByte(m_HeightMM, d_DNAMaxMM);
}

void AuChromoHead::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Attach = (ATTACHMENT)ui8;
	m_AttachDistMM = ReadLogByte(in, d_DNAMaxMM);
	m_EarAngleDeg = ReadAngleByte(in);
	m_EyeAngleDeg = ReadAngleByte(in);
	m_MouthAngleDeg = ReadAngleByte(in);
	m_NoseAngleDeg = ReadAngleByte(in);
	m_WidthMM = ReadLogByte(in, d_DNAMaxMM);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_HeightMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoHead::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_AttachDistMM = dice.RandomLogValue(m_AttachDistMM, d_DNAMaxMM);
	m_WidthMM = dice.RandomLogValue(m_WidthMM, d_DNAMaxMM);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	m_HeightMM = dice.RandomLogValue(m_HeightMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoHead::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalHead(*this, dna, firstBone);
}

AuPoint AuChromoHead::CenterOffset() const
{
	AuPoint centerRef;
	if(m_Attach == AttachBottom)
		centerRef.z = m_AttachDistMM * FeetPerMM();
	else
		centerRef.y = -m_AttachDistMM * FeetPerMM();
	return centerRef;
}

void AuChromoHead::GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
		const AuClothing& clothes, GROWTHSTAGE stage) const
{
	// Create the mesh of the head.
	AuMeatMesh mymesh;
	{
		// Build a head.
		// Skull
		AuPoint centerRef(CenterOffset());

		AuSolid solid;

		AuMatrix4x4 mSkull;
		mSkull.Translate(centerRef);
		AuSolidEllipsoid skull(mSkull, RadiusX(), RadiusY(), RadiusZ());
		solid.Add(skull);

		AuScFaceList faces;
		const float triangleSize = .5f/12.f;
		solid.Faces(faces, triangleSize);

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec(0,-1,0);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		mymesh.FromFaces(faces, AuMeatTri::Head);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, RadiusY()*2, topBottomBlend, blendAngle, blendWid);

		// Add skull features.
		dna.GetSkullFeatures(mymesh, mSkull, *this);

		// Hair
		{
			AuFolliclePointArray follicles;
			const float lockWid = .5f/12.f;
			HairFollicles(dna, lockWid, follicles, stage);
			float headSpace = .25f/12;
			float lockLen;
			float hangLength;
			float knotLength;
			float widAng;
			AuPoint combVec;
			bool basePatch;
			bool wearsBra = dna.HasBreasts();
			for(AuFolliclePointArray::const_iterator hi = follicles.cbegin(); hi != follicles.cend(); hi++)
			{
				// Get information about the lock of hair.
				AuPoint knotLoc;
				AuHairStyleLock::STYLE lstyle = hairStyle.Lock(*hi, headSpace, lockLen, hangLength, knotLength, widAng, combVec,
						basePatch);
				if(lstyle == AuHairStyleLock::HeadFlattop)
					lockLen = lockLen + RadiusZ() + centerRef.z - (*hi).z;	// Calculate length needed to reach the desired height.
				else if(lstyle == AuHairStyleLock::HeadKnot)
					knotLoc = AuPoint(centerRef.x, centerRef.y - RadiusY(), centerRef.z);
				if(lockLen == 0)
					continue;

				// Clothes may remove or shorten hair.
				if(clothes.CoverHair(*hi, basePatch, lockLen, wearsBra))
					continue;	// Hair is completely removed including the base patch, or there was no base patch.
				else if(lockLen == 0)
				{
					lstyle = AuHairStyleLock::Shaved;	// So we get only a base patch, no stubble.
					lockLen = 1;
				}

				// Make a lock of hair at the follicle.
				mymesh.Hair().MakeLock(*hi, lstyle, headSpace, lockWid, lockLen, hangLength, knotLoc, knotLength, widAng, combVec,
						basePatch, &skull);
			}
		}

		// Done adding polygons to the head, except for the movable jaw and eye balls. Set the bone to use by it's vertices.
		mymesh.SetBones(firstBone,1, 0,0, 0,0, 0,0);
		mymesh.SetHairBones(1, firstBone+1,1, 0,0, 0,0, 0,0);	// Hair on the movable jaw was added with id 1.

		const AuChromoMouth* mouth = dynamic_cast<const AuChromoMouth*>(dna.Chromosome(AuChromosome::Mouth));
		if(mouth)
		{
			double jawRadX = mouth->WidthMM() * FeetPerMM() / 2;
			mymesh.AddBoneRangeBlendXMouth(0, -(jawRadX+.1), 3, firstBone+2);	// Left side
			mymesh.AddBoneRangeBlendXMouth(0, jawRadX+.1, 3, firstBone+3);		// Right side
		}

		// Add movable portion of jaw, which takes 3 bones.
		dna.GetSkullJaw(mymesh, mSkull, *this, firstBone+1);

		// Add movable portion of eyes last, due to variable number.
		dna.GetSkullEyes(mymesh, mSkull, *this, firstBone+1+3);
	}

	AuMatrix4x4 m;
	m.Translate(BoneLoc(dna, 0));
	mymesh.Transform(m);

	//qDebug() << "AuChromoHead::GetSkinMesh mymesh" << mymesh.Count() << "hair" << mymesh.Hair().Count();
	//AuSegmentArray i;
	//mesh.Join(mymesh, i);
	//intersection.append(i);

	mesh.Append(mymesh);
}

void AuChromoHead::GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const
{
	Q_UNUSED(stage);
	// Create the mesh of the head.
	AuMeatMesh mymesh;
	{
		// Build a head.
		// Skull
		AuPoint centerRef(CenterOffset());

		AuSolid solid;

		AuMatrix4x4 mSkull;
		mSkull.Translate(centerRef);
		AuSolidEllipsoid skull(mSkull, RadiusX(), RadiusY(), RadiusZ());
		solid.Add(skull);

		AuScFaceList faces;
		const float triangleSize = .5f/12.f * d_ClothingTessFactor;
		solid.Faces(faces, triangleSize);

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec(0,-1,0);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		mymesh.FromFaces(faces, AuMeatTri::Head);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, RadiusY()*2, topBottomBlend, blendAngle, blendWid);

		// Add skull features.
		dna.GetSkullFeaturesClothed(mymesh, mSkull, *this);

		// Done adding polygons to the head, except for the eye balls. Set the bone to use by it's vertices.
		mymesh.SetBones(firstBone,1, 0,0, 0,0, 0,0);

		// Add movable portion of jaw, which takes 3 bones.
		dna.GetSkullJaw(mymesh, mSkull, *this, firstBone+1);

		// Add movable portion of eyes last, due to variable number.
		dna.GetSkullEyes(mymesh, mSkull, *this, firstBone+1+3);
	}

	AuMatrix4x4 m;
	m.Translate(BoneLoc(dna, 0));
	mymesh.Transform(m);
	mesh.Append(mymesh);
}

void AuChromoHead::HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles, GROWTHSTAGE stage) const
{
	const float follicleSpacing = lockWid * .5f;

	// Get my hair genes.
	bool hasHair = false;
	const AuGeneHair* hairDNA[AuFolliclePoint::ROLE_Count];
	for(int r=0; r<AuFolliclePoint::ROLE_Count; r++)
	{
		hairDNA[r] = HairDNA((AuFolliclePoint::ROLE)r);
		if(hairDNA[r])
			hasHair = true;
	}

	AuPoint centerRef(CenterOffset());
	AuMatrix4x4 mSkull;
	mSkull.Translate(centerRef);

	// Get follicles and exclusion areas from from face, eyes and ears.
	QMap<AuPoint, double> hairEx;	// Hair exclusion locations with radius.
	dna.GetSkullHair(lockWid, follicles, hairEx, mSkull, *this, stage);
	if(!hasHair)
		return;	// Skull itself has no hair.

	// Skull
	AuPointNArray myfol;
	AuSolidEllipsoid skull(mSkull, RadiusX(), RadiusY(), RadiusZ());
	skull.SurfacePoints(myfol, follicleSpacing);
	for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
	{
		// Get information about the hair follicle.
		double scalpangz = 50*DEG2RAD;			// Z angle above which hair is scalp.
		double crownangz = 25*DEG2RAD;			// Z angle above which hair is crown or bangs, also top of side burns
		double neckangz = -50*DEG2RAD;			// Z angle below which hair is removed for neck.
		double faceang = 20*DEG2RAD;			// XY angle forward of which hair is removed from the face.
		double bangang = 60*DEG2RAD;			// XY angle forward of which scalp hair is considered bangs.
		double cheekang = 50*DEG2RAD;			// XY angle forward of which hair is removed from the cheeks.
		double cheekangz = -10*DEG2RAD;			// Z angle below which face turns into cheeks
		double chinangz = -40*DEG2RAD;			// Z angle below which cheek turns into chin left|right
		double sideburnang = 80*DEG2RAD;		// XY angle forward of which hair below sudeburnangz is sideburns.
		double sideburnangz = -5*DEG2RAD;		// Z angle below which side burn turns into jowls.
		double backang = 110*DEG2RAD;			// XY angle behind which hair is back.
		double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
		double zang = (*fi).m_N.PolarIncline();

		// Decide whether the follicle should stay or go.
		AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
		if(zang > scalpangz)
			role = AuFolliclePoint::Scalp;
		else if(zang > crownangz)
		{
			if(xyang < bangang && xyang > -bangang)
				role = AuFolliclePoint::Bangs;
			else
				role = AuFolliclePoint::Crown;
		}
		else if(zang < neckangz)
		{
			if(xyang > backang || xyang < -backang)
				role = AuFolliclePoint::HeadNeckBack;
			else
				role = AuFolliclePoint::HeadNeckFront;
		}
		else if(fabs(xyang) > backang)
			role = AuFolliclePoint::BackOfHead;
		else if(fabs(xyang) < faceang)
			role = AuFolliclePoint::Face;
		else if(fabs(xyang) < cheekang)
		{
			if(zang > cheekangz)
				role = AuFolliclePoint::Face;
			else if(zang > chinangz)
			{
				if(xyang > 0)
					role = AuFolliclePoint::CheekLeft;
				else
					role = AuFolliclePoint::CheekRight;
			}
			else
			{
				if(xyang > 0)
					role = AuFolliclePoint::ChopLeft;
				else
					role = AuFolliclePoint::ChopRight;
			}
		}
		else if(fabs(xyang) < sideburnang)
		{
			if(xyang > 0)
			{
				if(zang > sideburnangz)
					role = AuFolliclePoint::SideburnLeft;
				else
					role = AuFolliclePoint::JowlLeft;
			}
			else
			{
				if(zang > sideburnangz)
					role = AuFolliclePoint::SideburnRight;
				else
					role = AuFolliclePoint::JowlRight;
			}
		}
		else
		{
			if(xyang > 0)
				role = AuFolliclePoint::SideLeft;
			else
				role = AuFolliclePoint::SideRight;
		}

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		if(!hairDNA[role])
			continue;	// No hair there.

		if(stage == Juvenile)
		{
			// Skip certain roles in juveniles.
			switch(role)
			{
			//case AuFolliclePoint::BackOfHead:
			//case AuFolliclePoint::Bangs:
			case AuFolliclePoint::CheekLeft:
			case AuFolliclePoint::CheekRight:
			case AuFolliclePoint::ChopLeft:
			case AuFolliclePoint::ChopRight:
			//case AuFolliclePoint::Crown:
			case AuFolliclePoint::Face:
			case AuFolliclePoint::JowlLeft:
			case AuFolliclePoint::JowlRight:
			//case AuFolliclePoint::HeadNeckBack:
			case AuFolliclePoint::HeadNeckFront:
			//case AuFolliclePoint::Scalp:
			//case AuFolliclePoint::SideburnLeft:
			//case AuFolliclePoint::SideburnRight:
			//case AuFolliclePoint::SideLeft:
			//case AuFolliclePoint::SideRight:
				continue;	// Hairless on juveniles.
			default:
				break;
			}
		}

		// Check for exclusion by head features such as eyes and ears.
		if(role != AuFolliclePoint::ROLE_Count && !hairEx.empty())
			for(QMap<AuPoint, double>::const_iterator xi = hairEx.cbegin(); xi != hairEx.cend(); xi++)
				if((*fi).Distance2(xi.key()) < (*xi)*(*xi))
				{
					role = AuFolliclePoint::ROLE_Count;
					break;
				}

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		const QColor& hairColor = hairDNA[role]->Color();
		float maxLen = hairDNA[role]->LengthMM() * FeetPerMM();
		AuFolliclePoint fp(0, role, *fi, hairColor, maxLen);
		follicles.append(fp);
	}
}

// Called on a default constructed object.
quint64 AuChromoHead::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::BackOfHead) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Bangs) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::CheekLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::CheekRight) |
		//mouth AuFolliclePoint::RoleBit(AuFolliclePoint::ChinFront) |
		//mouth AuFolliclePoint::RoleBit(AuFolliclePoint::ChinLeft) |
		//mouth AuFolliclePoint::RoleBit(AuFolliclePoint::ChinRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::ChopLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::ChopRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Crown) |
		//ear AuFolliclePoint::RoleBit(AuFolliclePoint::EarLeft) |
		//ear AuFolliclePoint::RoleBit(AuFolliclePoint::EarMiddle) |
		//ear AuFolliclePoint::RoleBit(AuFolliclePoint::EarRight) |
		//eye AuFolliclePoint::RoleBit(AuFolliclePoint::EyeLeft) |
		//eye AuFolliclePoint::RoleBit(AuFolliclePoint::EyeMiddle) |
		//eye AuFolliclePoint::RoleBit(AuFolliclePoint::EyeRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Face) |
		//mouth AuFolliclePoint::RoleBit(AuFolliclePoint::LipTop) |
		//nose AuFolliclePoint::RoleBit(AuFolliclePoint::Nose) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::JowlLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::JowlRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::HeadNeckBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::HeadNeckFront) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Scalp) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::SideburnRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::SideLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::SideRight);
}

quint8 AuChromoHead::BoneCount(const AuChromatid& dna) const
{
	return 1 + 3 + dna.EyeCount();	// Head(1), mouth(3), eyes
}

AuPoint AuChromoHead::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	if(bone == 0)
		return AuPoint(0,0, dna.HeadAltitude());	// Head

	if(bone > 3)
		return AuPoint(0,0, dna.HeadAltitude()) + CenterOffset() + EyeLocation(AuChromoWhich(bone - 4, dna.EyeCount()), dna);// Eye
	else
		return AuPoint(0,0, dna.HeadAltitude()) + CenterOffset() + MouthLocation();	// Mouth
}

AuPoint AuChromoHead::EarLocation(const AuChromoWhich& which) const
{
	LATERALITY laterality = AuChromoEar::Laterality(which);

	double earAngY;						// Angle to right ear from top of head.
	if(laterality == Middle)
		earAngY = 0;					// Top of head for one ear or the centered ear when there is an odd number.
	else
	{
		int earRank = which.Index()/2 + 1;
		earAngY = m_EarAngleDeg*DEG2RAD * earRank;
	}

	AuPoint earLoc = AuPoint(0,0,1).RotateY(earAngY);
	if(laterality == Left)
		earLoc.x = -earLoc.x;	// Flip horizontally for left ear.
	earLoc.Scale(RadiusX(), RadiusY(), RadiusZ());
	return earLoc;
}

AuPoint AuChromoHead::EyeLocation(const AuChromoWhich& which, const AuChromatid& dna) const
{
	float eyeDia;
	const AuChromoEye* eye = dynamic_cast<const AuChromoEye*>(dna.Chromosome(AuChromoEye::ChromosomeType(), which));
	if(eye)
		eyeDia = (float)(eye->EyeDiameterMM() * FeetPerMM());
	else
		eyeDia = 0;
	return EyeLocation(which, eyeDia/2);
}

AuPoint AuChromoHead::EyeLocation(const AuChromoWhich& which, float eyeRad) const
{
	int eyes = which.HowMany();
	LATERALITY laterality = AuChromoEye::Laterality(which);

	double eyeAngZ;						// Z rotation angle to left eye from front of head.
	double eyeAngX;						// X rotation angle to centered eye from front of head.
	if(eyes <= 1)
	{
		eyeAngX = m_EyeAngleDeg*DEG2RAD;
		eyeAngZ = 0;					// Front of head for one eye or the odd eye when there is an odd number.
	}
	else if(eyes == 2)
	{
		eyeAngX = 0;
		eyeAngZ = m_EyeAngleDeg*DEG2RAD;
	}
	else
	{
		if(laterality == Middle)
		{
			// This is the centered eye at the end of the list.
			eyeAngX = m_EyeAngleDeg*DEG2RAD;
			eyeAngZ = 0;
		}
		else
		{
			int eyeRank = which.Index()/2;
			eyeAngX = m_EyeAngleDeg*DEG2RAD * eyeRank;
			eyeAngZ = m_EyeAngleDeg*DEG2RAD;
		}
	}

	AuPoint eyeLoc = AuPoint(0,1,0).RotateZ(eyeAngZ).RotateX(eyeAngX);
	if(laterality == Right)
		eyeLoc.x = -eyeLoc.x;	// Flip horizontally for right eye.
	eyeLoc.Scale(RadiusX(), RadiusY(), RadiusZ());

	// Recess the location into the skull or the eye ball center is exactly on the skull surface, which looks weird.
	if(eyeRad > 0)
	{
		double recessDist = eyeRad * .5f;
		AuPoint recessVec(eyeLoc);
		recessVec.Unit(recessDist);
		eyeLoc -= recessVec;
	}
	return eyeLoc;
}

AuPoint AuChromoHead::MouthLocation() const
{
	double mouthAngX = m_MouthAngleDeg * DEG2RAD;			// Angle to mouth from top of head, which is at 0.
	AuPoint mouthLoc = AuPoint(0,0,1).RotateX(-mouthAngX);
	mouthLoc.Scale(RadiusX(), RadiusY(), RadiusZ());
	return mouthLoc;
}

AuPoint AuChromoHead::NoseLocation() const
{
	double noseAngX = m_NoseAngleDeg * DEG2RAD;			// Angle to nose from top of head, which is at 0.
	AuPoint noseLoc = AuPoint(0,0,1).RotateX(-noseAngX);
	noseLoc.Scale(RadiusX(), RadiusY(), RadiusZ());
	return noseLoc;
}

int AuChromoHead::ListWidgetOrder() const
{
	return 100;
}

QString AuChromoHead::ListWidgetText() const
{
	return QObject::tr("Head");
}

QWidget* AuChromoHead::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageHead* pp = new AuChromoPageHead(parent);
	return pp;
}

void AuChromoHead::SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
{
	for(QList<AuAnimalBone>::const_iterator bi = bones.cbegin(); bi != bones.cend(); bi++)
	{
		AuPoint bloc((*bi).PosedLoc());
		min = min.Min(bloc);
		max = max.Max(bloc);
	}

	// Include a point at the top of the head.
	AuPoint headPt(bones[0].UnposedLoc());
	if(m_Attach == AttachBottom)
		headPt.z += ((double)m_HeightMM/2 + (double)m_AttachDistMM) * FeetPerMM();
	else
		headPt.z += (double)m_HeightMM * FeetPerMM() / 2;
	headPt = bones[0].Matrix().Project(headPt);
	min = min.Min(headPt);
	max = max.Max(headPt);
}

bool AuChromoHead::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
{
	// Skull solid
	AuSolid solid;
	AuPoint centerRef(CenterOffset());
	AuMatrix4x4 mSkull;
	mSkull.Translate(centerRef);
	AuSolidEllipsoid skull(mSkull, RadiusX(), RadiusY(), RadiusZ());
	solid.Add(skull);

	// Use bone matrix to put the solid at its posed location.
	AuMatrix4x4 m(bones[0].Matrix());
	m.Translate(bones[0].UnposedLoc());	// This translate accounts for the translation to bone loc 0 at the end of mesh creation.

	// Transform the solid.
	solid.Transform(m);

	// Test for hit.
	return solid.SightTarget(eyeLoc, sightedLoc);
}

void AuChromoHead::AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
		const AuPose& mouthPose, float currentHeadTilt, float currentHeadTurn, HANDEDNESS hand,
		float& headX, float& headY, float& headZ, float& eyeLeftX, float& eyeLeftZ, float& eyeRightX,
		float& eyeRightZ, float& jawX, float& jawLeftY, float& jawRightY)
{
	Q_UNUSED(id);
	Q_UNUSED(bodyHz);
	headX = 0;
	headY = 0;
	headZ = 0;
	eyeLeftX = 0;
	eyeLeftZ = 0;
	eyeRightX = 0;
	eyeRightZ = 0;
	jawX = 0;
	jawLeftY = 0;
	jawRightY = 0;

	// Eyes share the turn with the head/neck.
	if(currentHeadTurn != 0)
	{
		// Assume I can turn my head just far enough to see 180 straight back in my peripheral vision.
		float peripheralVisionAngle = 60*DEG2RADf;	// Peripheral vision to each side of the center line.
		float turnAngle = fabs(currentHeadTurn);	// So we can work with positive numbers.
		float maxTurn = PIf - peripheralVisionAngle;

		// Turn everything to the max.
		if(currentHeadTurn < 0)
		{
			headZ = -PIf/2;
			eyeRightZ = -(PIf/2 - peripheralVisionAngle);
		}
		else
		{
			headZ = PIf/2;
			eyeRightZ = PIf/2 - peripheralVisionAngle;
		}

		if(turnAngle < maxTurn)
		{
			float frac = turnAngle / maxTurn;
			headZ *= frac;
			eyeRightZ *= frac;
		}

		eyeLeftZ = eyeRightZ;
	}

	switch(pose.Pose())
	{
	// Special
	case AuPose::Init:
	case AuPose::Spawn:
		break;

	// Head Only
	case AuPose::EyesOnly:
		headX = currentHeadTilt;
		break;

	// Horizontal Poses
	case AuPose::Prone:
	case AuPose::ProneCreep:
		// If the torso has a neck, this angle has no effect. In that case, the head is rotated to match the neck, per rotations in
		// AuChromoTorso::AnglesForPose.
		headX = PIf/4 + currentHeadTilt;
		eyeLeftX = 30*DEG2RADf;
		eyeRightX = eyeLeftX;
		break;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		headX = PIf/4 + currentHeadTilt;
		eyeLeftX = 30*DEG2RADf;
		eyeRightX = eyeLeftX;
		break;

	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		headX = currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		headX = currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
	case AuPose::Patient_Prone:			// Prone patient faces up.
		headX = currentHeadTilt;
		break;

	// Angles for dead poses are slightly tighter than writhe poses, and inanimate.
	case AuPose::DeadFaceDown:
		headX = -10*DEG2RADf;
		if(hand == LeftHanded)
		{
			headY = -20*DEG2RADf;
			headZ = 80*DEG2RADf;
		}
		else
		{
			headY = 20*DEG2RADf;
			headZ = -80*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceLeft:
		headX = -30*DEG2RADf;
		headY = 15*DEG2RADf;
		break;
	case AuPose::DeadFaceRight:
		headX = -30*DEG2RADf;
		headY = -15*DEG2RADf;
		break;
	case AuPose::DeadFaceUp:
		if(hand == LeftHanded)
		{
			headY = 15*DEG2RADf;
			headZ = -40*DEG2RADf;
		}
		else
		{
			headY = -15*DEG2RADf;
			headZ = 40*DEG2RADf;
		}
		break;

	// Angles for writhe/dying poses are based on dead poses, only animated to show signs of life.
	case AuPose::WritheFaceDown:
		headX = PIf/4 + currentHeadTilt;
		eyeLeftX = 30*DEG2RADf;
		eyeRightX = eyeLeftX;
		break;
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
		headX = currentHeadTilt;
		break;

	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
		headX = currentHeadTilt + 20*DEG2RADf;
		break;

	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		headX = currentHeadTilt;
		break;

	case AuPose::Swim:
	case AuPose::SwimFast:
		headX = PIf/4 + currentHeadTilt;
		eyeLeftX = 30*DEG2RADf;
		eyeRightX = eyeLeftX;
		break;
	case AuPose::TreadWater:
		headX = currentHeadTilt;
		break;
	
	// Sitting Poses
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Post_Sit:
		headX = currentHeadTilt;
		break;

	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
		headX = currentHeadTilt;
		break;

	case AuPose::Straddle:
		headX = currentHeadTilt;
		break;

	// Crouched Poses
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
		headX = currentHeadTilt;
		break;

	// Standing
	case AuPose::Stand:
	case AuPose::Post_Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
	case AuPose::StandRun:
	case AuPose::StandWalk:
		headX = currentHeadTilt;
		break;

	// Misc
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		headX = currentHeadTilt;
		break;

	// Mouth Poses
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}

	switch(mouthPose.Pose())
	{
	case AuPose::MouthPursed:
		jawX = 0;
		break;
	default:
	case AuPose::MouthClosed:
		jawX = -5*DEG2RADf;
		break;
	case AuPose::MouthOpen:
		jawX = -20*DEG2RADf;
		break;
	case AuPose::MouthAgape:
		jawX = -40*DEG2RADf;
		break;
	case AuPose::MouthOpenWide:
		jawX = -80*DEG2RADf;
		break;
	case AuPose::MouthYap:
		jawX = when.Cycle(mouthPose.Period()) * -40*DEG2RADf;
		break;
	case AuPose::MouthLaugh:
		jawX = -40*DEG2RADf + sinf(when.Cycle(mouthPose.Period()) * PIf*2) * -10*DEG2RADf;
		jawLeftY = 30*DEG2RADf;
		jawRightY = -30*DEG2RADf;
		break;
	case AuPose::MouthBite:
		{
			float c = when.Cycle(mouthPose.Period());
			if(c > .5)
				c = 0;
			else
				c *= 2;
			jawX = c * -85*DEG2RADf - 1*DEG2RADf;
		}
		break;
	case AuPose::MouthWhisper:
		{
			float x = when.Cycle(mouthPose.Period()) * 160;
			jawX = -10*DEG2RADf + AuNoiseF::CoherentNoise(id.GetID() % 12, x) * -10*DEG2RADf;
		}
		break;
	case AuPose::MouthTalk:
		{
			float x = when.Cycle(mouthPose.Period()) * 160;
			jawX = -20*DEG2RADf + AuNoiseF::CoherentNoise(id.GetID() % 12, x) * -20*DEG2RADf;
		}
		break;
	case AuPose::MouthJabber:
		{
			float x = when.Cycle(mouthPose.Period()) * 160;
			jawX = -45*DEG2RADf + AuNoiseF::CoherentNoise(id.GetID() % 12, x) * -40*DEG2RADf;
		}
		break;
	case AuPose::MouthYell:
		{
			float x = when.Cycle(mouthPose.Period()) * 160;
			jawX = -50*DEG2RADf + AuNoiseF::CoherentNoise(id.GetID() % 12, x) * -35*DEG2RADf;
		}
		break;
	case AuPose::MouthYawn:
		{
			float c = when.Cycle(mouthPose.Period());
			float s;	// 0 close to 1 open
			if(c <= .1f)
				s = c*10;	// mouth opening
			else if(c <= .4f)
				s = 1;		// mouth wide open
			else if(c <= .5f)
				s = 1 - (c - .4f) * 10;	// mouth closing
			else
				s = 0;		// mouth closed
			jawX = s * -85*DEG2RADf - 1*DEG2RADf;
		}
		break;
	case AuPose::MouthSmile:
		jawX = -20*DEG2RADf;
		jawLeftY = 30*DEG2RADf;
		jawRightY = -30*DEG2RADf;
		break;
	case AuPose::MouthGrin:
		jawX = -45*DEG2RADf;
		jawLeftY = 40*DEG2RADf;
		jawRightY = -40*DEG2RADf;
		break;
	case AuPose::MouthFrown:
		jawX = -10*DEG2RADf;
		jawLeftY = -30*DEG2RADf;
		jawRightY = 30*DEG2RADf;
		break;
	case AuPose::MouthSad:
		jawX = -30*DEG2RADf;
		jawLeftY = -40*DEG2RADf;
		jawRightY = 40*DEG2RADf;
		break;
	case AuPose::MouthCry:
		jawX = -30*DEG2RADf + sinf(when.Cycle(mouthPose.Period()) * PIf*2) * -10*DEG2RADf;
		jawLeftY = -40*DEG2RADf;
		jawRightY = 40*DEG2RADf;
		break;
	case AuPose::MouthPerplexed:
		jawX = -5*DEG2RADf;
		if(hand == RightHanded)
		{
			jawLeftY = -40*DEG2RADf;
			jawRightY = -40*DEG2RADf;
		}
		else
		{
			jawLeftY = 40*DEG2RADf;
			jawRightY = 40*DEG2RADf;
		}
		break;
	case AuPose::MouthGrim:
		jawX = -5*DEG2RADf;
		if(hand == RightHanded)
			jawRightY = 40*DEG2RADf;
		else
			jawLeftY = -40*DEG2RADf;
		break;
	case AuPose::MouthSardonic:
		jawX = -5*DEG2RADf;
		if(hand == RightHanded)
			jawRightY = -40*DEG2RADf;
		else
			jawLeftY = 40*DEG2RADf;
		break;
	}

	//DHH Testing
	//headX = (45.f - when.Cycle(5000) * 180) * DEG2RADf;
	//eyeLeftZ = (50.f - when.Cycle(3000) * 100) * DEG2RADf;
	//eyeRightZ = eyeLeftZ;
}

float AuChromoHead::Volume() const
{
	// Volume of ellipsoid
	// V = 4/3 PI a b c
	// a b c are the radii
	float a = RadiusX();
	float b = RadiusY();
	float c = RadiusZ();
	float v = 4.f/3.f * PIf * a * b * c;
	return v;
}

bool AuChromoHead::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoHead* c = (const AuChromoHead*)chromosome;
	if(m_Attach != c->m_Attach)
		return false;
	return true;
}

AuChromosome* AuChromoHead::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoHead* c = dynamic_cast<const AuChromoHead*>(g);
	if(!c)
		return 0;
	AuChromoHead* comb = new AuChromoHead(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoTorso::AuChromoTorso(quint16 neckLengthMM, quint16 upperLengthMM, quint16 upperWidthMM, quint16 upperThicknessMM,
		quint16 lowerLengthMM, quint16 lowerWidthMM, quint16 lowerThicknessMM, quint16 tailLengthMM)
{
	m_LowerLengthMM = lowerLengthMM;
	m_LowerThicknessMM = lowerThicknessMM;
	m_LowerWidthMM = lowerWidthMM;
	m_NeckLengthMM = neckLengthMM;
	m_TailLengthMM = tailLengthMM;
	m_UpperLengthMM = upperLengthMM;
	m_UpperThicknessMM = upperThicknessMM;
	m_UpperWidthMM = upperWidthMM;
}

AuChromoTorso::AuChromoTorso()
{
	m_LowerLengthMM = 100;
	m_LowerThicknessMM = 100;
	m_LowerWidthMM = 100;
	m_NeckLengthMM = 100;
	m_TailLengthMM = 100;
	m_UpperLengthMM = 100;
	m_UpperThicknessMM = 100;
	m_UpperWidthMM = 100;
}

AuChromoTorso::AuChromoTorso(const AuChromoTorso& c) : AuChromosomeWithGenes(c)
{
	m_LowerLengthMM = c.m_LowerLengthMM;
	m_LowerThicknessMM = c.m_LowerThicknessMM;
	m_LowerWidthMM = c.m_LowerWidthMM;
	m_NeckLengthMM = c.m_NeckLengthMM;
	m_TailLengthMM = c.m_TailLengthMM;
	m_UpperLengthMM = c.m_UpperLengthMM;
	m_UpperThicknessMM = c.m_UpperThicknessMM;
	m_UpperWidthMM = c.m_UpperWidthMM;
}

AuChromoTorso::AuChromoTorso(const AuChromoTorso& c1, const AuChromoTorso& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_LowerLengthMM = CombineRange(c1.m_LowerLengthMM, c2.m_LowerLengthMM, dice);
	dice.Random();
	m_LowerThicknessMM = CombineRange(c1.m_LowerThicknessMM, c2.m_LowerThicknessMM, dice);
	dice.Random();
	m_LowerWidthMM = CombineRange(c1.m_LowerWidthMM, c2.m_LowerWidthMM, dice);
	dice.Random();
	m_NeckLengthMM = CombineRange(c1.m_NeckLengthMM, c2.m_NeckLengthMM, dice);
	dice.Random();
	m_TailLengthMM = CombineRange(c1.m_TailLengthMM, c2.m_TailLengthMM, dice);
	dice.Random();
	m_UpperLengthMM = CombineRange(c1.m_UpperLengthMM, c2.m_UpperLengthMM, dice);
	dice.Random();
	m_UpperThicknessMM = CombineRange(c1.m_UpperThicknessMM, c2.m_UpperThicknessMM, dice);
	dice.Random();
	m_UpperWidthMM = CombineRange(c1.m_UpperWidthMM, c2.m_UpperWidthMM, dice);
}

void AuChromoTorso::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoTorso& c = dynamic_cast<const AuChromoTorso&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_LowerLengthMM = c.m_LowerLengthMM;
		m_LowerThicknessMM = c.m_LowerThicknessMM;
		m_LowerWidthMM = c.m_LowerWidthMM;
		m_NeckLengthMM = c.m_NeckLengthMM;
		m_TailLengthMM = c.m_TailLengthMM;
		m_UpperLengthMM = c.m_UpperLengthMM;
		m_UpperThicknessMM = c.m_UpperThicknessMM;
		m_UpperWidthMM = c.m_UpperWidthMM;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoTorso::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_LowerLengthMM;
	out << m_LowerThicknessMM;
	out << m_LowerWidthMM;
	out << m_NeckLengthMM;;
	out << m_TailLengthMM;;
	out << m_UpperLengthMM;
	out << m_UpperThicknessMM;
	out << m_UpperWidthMM;
}

void AuChromoTorso::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_LowerLengthMM;
	in >> m_LowerThicknessMM;
	in >> m_LowerWidthMM;
	in >> m_NeckLengthMM;;
	in >> m_TailLengthMM;;
	in >> m_UpperLengthMM;
	in >> m_UpperThicknessMM;
	in >> m_UpperWidthMM;
}

void AuChromoTorso::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_LowerLengthMM, d_DNAMaxMM);
	out << ToLogByte(m_LowerThicknessMM, d_DNAMaxMM);
	out << ToLogByte(m_LowerWidthMM, d_DNAMaxMM);
	out << ToLogByte(m_NeckLengthMM, d_DNAMaxMM);
	out << ToLogByte(m_TailLengthMM, d_DNAMaxMM);
	out << ToLogByte(m_UpperLengthMM, d_DNAMaxMM);
	out << ToLogByte(m_UpperThicknessMM, d_DNAMaxMM);
	out << ToLogByte(m_UpperWidthMM, d_DNAMaxMM);
}

void AuChromoTorso::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_LowerLengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_LowerThicknessMM = ReadLogByte(in, d_DNAMaxMM);
	m_LowerWidthMM = ReadLogByte(in, d_DNAMaxMM);
	m_NeckLengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_TailLengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_UpperLengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_UpperThicknessMM = ReadLogByte(in, d_DNAMaxMM);
	m_UpperWidthMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoTorso::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LowerLengthMM = dice.RandomLogValue(m_LowerLengthMM, d_DNAMaxMM);
	m_LowerThicknessMM = dice.RandomLogValue(m_LowerThicknessMM, d_DNAMaxMM);
	m_LowerWidthMM = dice.RandomLogValue(m_LowerWidthMM, d_DNAMaxMM);
	m_NeckLengthMM = dice.RandomLogValue(m_NeckLengthMM, d_DNAMaxMM);
	m_TailLengthMM = dice.RandomLogValue(m_TailLengthMM, d_DNAMaxMM);
	m_UpperLengthMM = dice.RandomLogValue(m_UpperLengthMM, d_DNAMaxMM);
	m_UpperThicknessMM = dice.RandomLogValue(m_UpperThicknessMM, d_DNAMaxMM);
	m_UpperWidthMM = dice.RandomLogValue(m_UpperWidthMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoTorso::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalTorso(*this, dna, firstBone);
}

void AuChromoTorso::GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
		const AuClothing& clothes, GROWTHSTAGE stage) const
{
	if(clothes.BareUpperTorsoCovered() && clothes.BareLowerTorsoCovered() && clothes.BareNeckCovered())
		return;

	// Create the mesh of the torso.
	AuMeatMesh mymesh;
	{
		double tlen = (UpperLengthMM() + LowerLengthMM()) * FeetPerMM();
		double waistHeight = LowerLengthMM() * FeetPerMM();

		// Get rib cage radius from arm sockets and waist radius from leg sockets.
		float ribRadX = UpperWidthMM() * FeetPerMM() / 2;
		float waistRadX = LowerWidthMM() * FeetPerMM() / 2;
		const double upperRad = UpperThicknessMM() * FeetPerMM() / 2;
		const double lowerRad = LowerThicknessMM() * FeetPerMM() / 2;
		int legs = 0;
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * .9;
				if(spineDist > ribRadX)
					ribRadX = spineDist;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				legs++;
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * 1.1;
				if(spineDist > waistRadX)
					waistRadX = spineDist;
				continue;
			}
		}

		// Build a meat profile of the torso, with bulges at the arm joints and sockets at the leg joints.
		const float triangleSize = .5f/12.f;
		AuMeatProfile torso(triangleSize);

		// Create general profile.
		float topWaist = lowerRad;	// belly
		float bottomWaist = lowerRad;	// front
		float leftWaist = waistRadX;
		float rightWaist = waistRadX;
		AuMeatShape shape(bottomWaist, topWaist, leftWaist, rightWaist);
		torso.Add(waistHeight, shape);

		// Neck end of torso.
		float shoulderBulge = (upperRad + ribRadX)/2;	// Bulge past end of torso for shoulders.
		float neckLen = NeckLengthMM() * FeetPerMM();
		if(neckLen > shoulderBulge)
		{
			// Neck sticks beyond the shoulder meat.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Interpolate neck size at the point it emerges from the torso.
				double radTorsoX = ribRadX / 4;
				double radTorsoY = upperRad / 4;
				float frac = shoulderBulge / neckLen;
				float neckRadX = AuNoiseF::Linear_Interpolate(radTorsoX, radHeadX, frac);
				float neckRadY = AuNoiseF::Linear_Interpolate(radTorsoY, radHeadY, frac);
				float neckOff = tlen + shoulderBulge;
				torso.Add(neckOff, AuMeatShape(neckRadY, neckRadY, neckRadX, neckRadX));

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}
		else
		{
			// No neck between shoulders and head.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}

		// Tail
		double buttBulge = lowerRad/2;
		bool hasTail = false;
		{
			// This set of radii is used to construct the shape added at buttBulge. It is important to making the genitals.
			float bottomRad;
			float topRad;
			float leftRad;
			float rightRad;
			float tailLen = TailLengthMM() * FeetPerMM();
			if(tailLen <= buttBulge)
			{
				// No visible tail.
				if(LowerThicknessMM() > LowerWidthMM())
				{
					bottomRad = 2 * FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				else if(LowerThicknessMM() < LowerWidthMM())
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = 2 * FeetPerMM();
					rightRad = leftRad;
				}
				else
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			}
			else
			{
				// We have a visible tail.
				hasTail = true;
				// Calculate radii at buttRad.
				float tailRad;
				if(LowerThicknessMM() < LowerWidthMM())
					tailRad = LowerThicknessMM() * FeetPerMM() / 4;
				else
					tailRad = LowerWidthMM() * FeetPerMM() / 4;
				bottomRad = tailRad * .5f;
				topRad = topWaist;
				leftRad = tailRad;
				rightRad = tailRad;

				// Add additional shape(s) for the rest of the tail.
				float tailVis = tailLen - buttBulge;
				float tailBase = buttBulge + tailVis * .25f;
				float tailTip = buttBulge + tailVis * .75f;
				shape = AuMeatShape(bottomRad * .5f, topRad * .4f, leftRad * .5f, rightRad * .5f);
				torso.Add(-tailBase, shape);
				shape = AuMeatShape(bottomRad * .25f, topRad * .2f, leftRad * .25f, rightRad * .25f);
				torso.Add(-tailTip, shape);

				shape = AuMeatShape(bottomRad * .2f, topRad * .1f, leftRad * .2f, rightRad * .2f);
				torso.Add(-tailLen, shape);
			}

			// Add the shape that affects the genitals. It is either the end of the torso, or the first shape of the tail.
			shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			torso.Add(-buttBulge, shape);
		}

		// Genitals
		if(dna.HasMaleGenitals())
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = bottomWaist;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}
		else
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = AuNoise::Linear_Interpolate(bottomWaist * .75, shape.BottomRadius(), p) * 1.2;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}

		// Get positions of arms and legs.
		double lowerRadX = LowerWidthMM() * FeetPerMM() / 2;
		double lowestArms = tlen;
		AuPoint mybone(BoneLoc(dna, 0));
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float bottomRad = upperRad;
				float topRad = upperRad;
				float leftRad, rightRad;
				if(tarm->Laterality() == Left)
				{
					leftRad = fabsf((float)bone.x) * 1.2f;
					rightRad = ribRadX;
				}
				else
				{
					leftRad = ribRadX;
					rightRad = fabsf((float)bone.x) * 1.2f;
				}
				double spineHt = bone.z - mybone.z;
				torso.Add(spineHt, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				if(spineHt < lowestArms)
					lowestArms = spineHt;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float bottomRad = bottomWaist * .75;	// Belly
				float topRad = topWaist;				// Back
				float leftRad, rightRad;
				switch(tleg->Laterality())
				{
				case 0:		// Left
					leftRad = fabsf((float)bone.x) * 1.1f;
					rightRad = lowerRadX;
					break;
				case 1:		// Middle
				default:
					continue;
				case 2:		// Right
					leftRad = lowerRadX;
					rightRad = fabsf((float)bone.x) * 1.1f;
					break;
				}
				torso.Add(bone.z - mybone.z, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				continue;
			}
		}

		// Add a rib cage below the lowest arms.
		double ribHt = lowestArms - upperRad;
		if(ribHt > 0)
		{
			float bottomRib = upperRad;
			float topRib = upperRad;
			float leftRib = ribRadX;
			float rightRib = ribRadX;
			AuMeatShape shape(bottomRib, topRib, leftRib, rightRib);
			torso.Add(ribHt, shape);
		}

		int topBottomBlend = 2;	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
		AuPoint blendUpVec(0,0,1);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		torso.GetMesh(mymesh, AuMeatTri::TorsoUpper, topBottomBlend, blendUpVec, blendAngle, blendWid);
		mymesh.SmoothNormals();
		AuMatrix4x4 m;
		m.RotateX(PI/2);
		mymesh.Transform(m);
		if(waistHeight > 0)
			mymesh.SetPartRangeZ(AuMeatTri::TorsoLower, mymesh.Min().z, waistHeight);
		if(neckLen > shoulderBulge)
			mymesh.SetPartRangeZ(AuMeatTri::Neck, tlen + shoulderBulge, mymesh.Max().z);
		if(hasTail)
			mymesh.SetPartRangeZ(AuMeatTri::Tail, mymesh.Min().z, -buttBulge);

		// Add torso features.
		AuMatrix4x4 mTorso;
		dna.GetTorsoFeatures(mymesh, mTorso, *this, stage);

		// Hair
		{
			AuFolliclePointArray follicles;
			const float lockWid = triangleSize * 2;
			HairFollicles(dna, lockWid, follicles, stage);
			float headSpace = .25f/12;
			float lockLen;
			float hangLength;
			float knotLength;
			AuPoint combVec;
			bool basePatch;
			float widAng;
			bool wearsBra = dna.HasBreasts();
			for(AuFolliclePointArray::const_iterator hi = follicles.cbegin(); hi != follicles.cend(); hi++)
			{
				// Get information about the lock of hair.
				AuPoint knotLoc;
				AuHairStyleLock::STYLE lstyle = hairStyle.Lock(*hi, headSpace, lockLen, hangLength, knotLength, widAng, combVec,
						basePatch);
				if(lockLen == 0)
					continue;

				// Clothes may remove or shorten hair.
				if(clothes.CoverHair(*hi, basePatch, lockLen, wearsBra))
					continue;	// Hair is completely removed including the base patch, or there was no base patch.
				else if(lockLen == 0)
				{
					lstyle = AuHairStyleLock::Shaved;	// So we get only a base patch, no stubble.
					lockLen = 1;
				}

				// Make a lock of hair at the follicle.
				mymesh.Hair().MakeLock(*hi, lstyle, headSpace, lockWid, lockLen, hangLength, knotLoc, knotLength, widAng, combVec,
						basePatch, 0);
			}
		}

		// Done adding polygons to the torso. Set the bone to use by it's vertices.
		mymesh.SetBonesRangeBlendZ(mymesh.GrossMinZ(), waistHeight, firstBone, firstBone+1);
		mymesh.SetBonesRangeZ(waistHeight, mymesh.GrossMaxZ(), firstBone+1,1., 0,0., 0,0., 0,0.);
		int neckBones = NeckBoneCount();
		if(neckBones)
			mymesh.SetBonesRangeBlendZ(tlen, mymesh.GrossMaxZ(), firstBone+1, firstBone+1+neckBones);
		int tailBones = TailBoneCount();
		if(hasTail && tailBones)
			mymesh.SetBonesRangeBlendZ(mymesh.GrossMinZ(), -buttBulge, firstBone+1+neckBones+tailBones, firstBone+1+neckBones);

		if(clothes.BareUpperTorsoCovered() && clothes.BareLowerTorsoCovered())
			mymesh.RemoveBelow(tlen + shoulderBulge/2);	// Upper and lower torso are covered.
		else if(clothes.BareUpperTorsoCovered() && clothes.BareNeckCovered())
			mymesh.RemoveAbove(waistHeight * 1.1f);	// Lower torso is not covered.
		else if(clothes.BareLowerTorsoCovered())
			mymesh.RemoveBelow(waistHeight * .9f);	// Upper torso is not covered.
	}

	//qDebug() << "AuChromoTorso::GetSkinMesh mymesh" << mymesh.Count();
	//AuSegmentArray i;
	//mesh.Join(mymesh, i);
	//intersection.append(i);

	mesh.Append(mymesh);
}

void AuChromoTorso::GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const
{
	// Create the mesh of the torso.
	AuMeatMesh mymesh;
	{
		double tlen = (UpperLengthMM() + LowerLengthMM()) * FeetPerMM();
		double waistHeight = LowerLengthMM() * FeetPerMM();

		// Get rib cage radius from arm sockets and waist radius from leg sockets.
		float ribRadX = UpperWidthMM() * FeetPerMM() / 2;
		float waistRadX = LowerWidthMM() * FeetPerMM() / 2;
		const double upperRad = UpperThicknessMM() * FeetPerMM() / 2;
		const double lowerRad = LowerThicknessMM() * FeetPerMM() / 2;
		int legs = 0;
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * .9;
				if(spineDist > ribRadX)
					ribRadX = spineDist;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				legs++;
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * 1.1;
				if(spineDist > waistRadX)
					waistRadX = spineDist;
				continue;
			}
		}

		// Build a meat profile of the torso, with bulges at the arm joints and sockets at the leg joints.
		const float triangleSize = .5f/12.f * d_ClothingTessFactor;
		AuMeatProfile torso(triangleSize);

		// Create general profile.
		float topWaist = lowerRad;	// belly
		float bottomWaist = lowerRad;	// front
		float leftWaist = waistRadX;
		float rightWaist = waistRadX;
		AuMeatShape shape(bottomWaist, topWaist, leftWaist, rightWaist);
		torso.Add(waistHeight, shape);

		// Neck end of torso.
		float shoulderBulge = (upperRad + ribRadX)/2;	// Bulge past end of torso for shoulders.
		float neckLen = NeckLengthMM() * FeetPerMM();
		if(neckLen > shoulderBulge)
		{
			// Neck sticks beyond the shoulder meat.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Interpolate neck size at the point it emerges from the torso.
				double radTorsoX = ribRadX / 4;
				double radTorsoY = upperRad / 4;
				float frac = shoulderBulge / neckLen;
				float neckRadX = AuNoiseF::Linear_Interpolate(radTorsoX, radHeadX, frac);
				float neckRadY = AuNoiseF::Linear_Interpolate(radTorsoY, radHeadY, frac);
				float neckOff = tlen + shoulderBulge;
				torso.Add(neckOff, AuMeatShape(neckRadY, neckRadY, neckRadX, neckRadX));

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}
		else
		{
			// No neck between shoulders and head.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}

		// Tail
		double buttBulge = lowerRad/2;
		bool hasTail = false;
		{
			// This set of radii is used to construct the shape added at buttBulge. It is important to making the genitals.
			float bottomRad;
			float topRad;
			float leftRad;
			float rightRad;
			float tailLen = TailLengthMM() * FeetPerMM();
			if(tailLen <= buttBulge)
			{
				// No visible tail.
				if(LowerThicknessMM() > LowerWidthMM())
				{
					bottomRad = 2 * FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				else if(LowerThicknessMM() < LowerWidthMM())
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = 2 * FeetPerMM();
					rightRad = leftRad;
				}
				else
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			}
			else
			{
				// We have a visible tail.
				hasTail = true;
				// Calculate radii at buttRad.
				float tailRad;
				if(LowerThicknessMM() < LowerWidthMM())
					tailRad = LowerThicknessMM() * FeetPerMM() / 4;
				else
					tailRad = LowerWidthMM() * FeetPerMM() / 4;
				bottomRad = tailRad * .5f;
				topRad = topWaist;
				leftRad = tailRad;
				rightRad = tailRad;

				// Add additional shape(s) for the rest of the tail.
				float tailVis = tailLen - buttBulge;
				float tailBase = buttBulge + tailVis * .25f;
				float tailTip = buttBulge + tailVis * .75f;
				shape = AuMeatShape(bottomRad * .5f, topRad * .4f, leftRad * .5f, rightRad * .5f);
				torso.Add(-tailBase, shape);
				shape = AuMeatShape(bottomRad * .25f, topRad * .2f, leftRad * .25f, rightRad * .25f);
				torso.Add(-tailTip, shape);

				shape = AuMeatShape(bottomRad * .2f, topRad * .1f, leftRad * .2f, rightRad * .2f);
				torso.Add(-tailLen, shape);
			}

			// Add the shape that affects the genitals. It is either the end of the torso, or the first shape of the tail.
			shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			torso.Add(-buttBulge, shape);
		}

		// Genitals
		if(dna.HasMaleGenitals())
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = bottomWaist;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}
		else
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = AuNoise::Linear_Interpolate(bottomWaist * .75, shape.BottomRadius(), p) * 1.2;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}

		// Get positions of arms and legs.
		double lowerRadX = LowerWidthMM() * FeetPerMM() / 2;
		double lowestArms = tlen;
		AuPoint mybone(BoneLoc(dna, 0));
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float bottomRad = upperRad;
				float topRad = upperRad;
				float leftRad, rightRad;
				if(tarm->Laterality() == Left)
				{
					leftRad = fabsf((float)bone.x) * 1.2f;
					rightRad = ribRadX;
				}
				else
				{
					leftRad = ribRadX;
					rightRad = fabsf((float)bone.x) * 1.2f;
				}
				double spineHt = bone.z - mybone.z;
				torso.Add(spineHt, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				if(spineHt < lowestArms)
					lowestArms = spineHt;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float bottomRad = bottomWaist * .75;	// Belly
				float topRad = topWaist;				// Back
				float leftRad, rightRad;
				switch(tleg->Laterality())
				{
				case 0:		// Left
					leftRad = fabsf((float)bone.x) * 1.1f;
					rightRad = lowerRadX;
					break;
				case 1:		// Middle
				default:
					continue;
				case 2:		// Right
					leftRad = lowerRadX;
					rightRad = fabsf((float)bone.x) * 1.1f;
					break;
				}
				torso.Add(bone.z - mybone.z, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				continue;
			}
		}

		// Add a rib cage below the lowest arms.
		double ribHt = lowestArms - upperRad;
		if(ribHt > 0)
		{
			float bottomRib = upperRad;
			float topRib = upperRad;
			float leftRib = ribRadX;
			float rightRib = ribRadX;
			AuMeatShape shape(bottomRib, topRib, leftRib, rightRib);
			torso.Add(ribHt, shape);
		}

		int topBottomBlend = 2;	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
		AuPoint blendUpVec(0,0,1);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		torso.GetMesh(mymesh, AuMeatTri::TorsoUpper, topBottomBlend, blendUpVec, blendAngle, blendWid);
		mymesh.SmoothNormals();
		AuMatrix4x4 m;
		m.RotateX(PI/2);
		mymesh.Transform(m);
		if(waistHeight > 0)
			mymesh.SetPartRangeZ(AuMeatTri::TorsoLower, mymesh.Min().z, waistHeight);
		if(neckLen > shoulderBulge)
			mymesh.SetPartRangeZ(AuMeatTri::Neck, tlen + shoulderBulge, mymesh.Max().z);
		if(hasTail)
			mymesh.SetPartRangeZ(AuMeatTri::Tail, mymesh.Min().z, -buttBulge);

		// Add torso features.
		AuMatrix4x4 mTorso;
		dna.GetTorsoFeaturesClothed(mymesh, mTorso, *this, stage);

		// Done adding polygons to the torso. Set the bone to use by it's vertices.
		mymesh.SetBonesRangeBlendZ(mymesh.Min().z, waistHeight, firstBone, firstBone+1);
		mymesh.SetBonesRangeZ(waistHeight, mymesh.Max().z, firstBone+1,1., 0,0., 0,0., 0,0.);
		int neckBones = NeckBoneCount();
		if(neckBones && neckLen > shoulderBulge)
			mymesh.SetBonesRangeBlendZ(tlen, mymesh.Max().z, firstBone+1, firstBone+neckBones+1);
		int tailBones = TailBoneCount();
		if(hasTail && tailBones)
			mymesh.SetBonesRangeBlendZ(mymesh.GrossMinZ(), -buttBulge, firstBone+1+neckBones+tailBones, firstBone+1+neckBones);
	}

	mesh.Append(mymesh);
}

void AuChromoTorso::HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles, GROWTHSTAGE stage) const
{
	const float follicleSpacing = lockWid * .5f;

	// Get my hair genes.
	bool hasHair = false;
	const AuGeneHair* hairDNA[AuFolliclePoint::ROLE_Count];
	for(int r=0; r<AuFolliclePoint::ROLE_Count; r++)
	{
		hairDNA[r] = HairDNA((AuFolliclePoint::ROLE)r);
		if(hairDNA[r])
			hasHair = true;
	}
	if(!hasHair)
		return;	// I have no hair.

	// Create the mesh of the torso.
	AuMeatMesh mymesh;
	double tlen = (UpperLengthMM() + LowerLengthMM()) * FeetPerMM();
	double waistHeight = LowerLengthMM() * FeetPerMM();
	bool maleGenitals = dna.HasMaleGenitals();
	float shoulderBulge = 0;
	double buttBulge = 0;
	float tailLen = TailLengthMM() * FeetPerMM();
	{
		// Get rib cage radius from arm sockets and waist radius from leg sockets.
		float ribRadX = UpperWidthMM() * FeetPerMM() / 2;
		float waistRadX = LowerWidthMM() * FeetPerMM() / 2;
		const double upperRad = UpperThicknessMM() * FeetPerMM() / 2;
		const double lowerRad = LowerThicknessMM() * FeetPerMM() / 2;
		int legs=0;
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * .9;
				if(spineDist > ribRadX)
					ribRadX = spineDist;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				legs++;
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float spineDist = bone.Length_2D() * 1.1;
				if(spineDist > waistRadX)
					waistRadX = spineDist;
				continue;
			}
		}

		// Build a meat profile of the torso, with bulges at the arm joints and sockets at the leg joints.
		const float triangleSize = follicleSpacing;
		AuMeatProfile torso(triangleSize);

		// Create general profile.
		float topWaist = lowerRad;	// belly
		float bottomWaist = lowerRad;	// front
		float leftWaist = waistRadX;
		float rightWaist = waistRadX;
		AuMeatShape shape(bottomWaist, topWaist, leftWaist, rightWaist);
		torso.Add(waistHeight, shape);

		// Neck end of torso.
		shoulderBulge = (upperRad + ribRadX)/2;	// Bulge past end of torso for shoulders.
		float neckLen = NeckLengthMM() * FeetPerMM();
		if(neckLen > shoulderBulge)
		{
			// Neck sticks beyond the shoulder meat.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Interpolate neck size at the point it emerges from the torso.
				double radTorsoX = ribRadX / 4;
				double radTorsoY = upperRad / 4;
				float frac = shoulderBulge / neckLen;
				float neckRadX = AuNoiseF::Linear_Interpolate(radTorsoX, radHeadX, frac);
				float neckRadY = AuNoiseF::Linear_Interpolate(radTorsoY, radHeadY, frac);
				float neckOff = tlen + shoulderBulge;
				torso.Add(neckOff, AuMeatShape(neckRadY, neckRadY, neckRadX, neckRadX));

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}
		else
		{
			// No neck between shoulders and head.
			const AuChromoHead* head = dynamic_cast<const AuChromoHead*>(dna.Chromosome(AuChromosome::Head));
			if(head)
			{
				// We have a head.
				float radHeadX = head->RadiusX() / 2;
				float radHeadY = head->RadiusY() / 2;

				// Add top end of neck.
				float neckEnd = tlen + neckLen;
				torso.Add(neckEnd, AuMeatShape(radHeadY, radHeadY, radHeadX, radHeadX));
			}
		}

		// Tail
		buttBulge = lowerRad/2;
		bool hasTail = false;
		{
			// This set of radii is used to construct the shape added at buttBulge. It is important to making the genitals.
			float bottomRad;
			float topRad;
			float leftRad;
			float rightRad;
			if(tailLen <= buttBulge)
			{
				// No visible tail.
				if(LowerThicknessMM() > LowerWidthMM())
				{
					bottomRad = 2 * FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				else if(LowerThicknessMM() < LowerWidthMM())
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = 2 * FeetPerMM();
					rightRad = leftRad;
				}
				else
				{
					bottomRad = FeetPerMM();
					topRad = bottomRad;
					leftRad = FeetPerMM();
					rightRad = leftRad;
				}
				shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			}
			else
			{
				// We have a visible tail.
				hasTail = true;
				// Calculate radii at buttRad.
				float tailRad;
				if(LowerThicknessMM() < LowerWidthMM())
					tailRad = LowerThicknessMM() * FeetPerMM() / 4;
				else
					tailRad = LowerWidthMM() * FeetPerMM() / 4;
				bottomRad = tailRad * .5f;
				topRad = topWaist;
				leftRad = tailRad;
				rightRad = tailRad;

				// Add additional shape(s) for the rest of the tail.
				float tailVis = tailLen - buttBulge;
				float tailBase = buttBulge + tailVis * .25f;
				float tailTip = buttBulge + tailVis * .75f;
				shape = AuMeatShape(bottomRad * .5f, topRad * .4f, leftRad * .5f, rightRad * .5f);
				torso.Add(-tailBase, shape);
				shape = AuMeatShape(bottomRad * .25f, topRad * .2f, leftRad * .25f, rightRad * .25f);
				torso.Add(-tailTip, shape);

				shape = AuMeatShape(bottomRad * .2f, topRad * .1f, leftRad * .2f, rightRad * .2f);
				torso.Add(-tailLen, shape);
			}

			// Add the shape that affects the genitals. It is either the end of the torso, or the first shape of the tail.
			shape = AuMeatShape(bottomRad, topRad, leftRad, rightRad);
			torso.Add(-buttBulge, shape);
		}

		// Genitals
		if(maleGenitals)
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = bottomWaist;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}
		else
		{
			double p = .25;					// 0 to 1 from end of spine to bottom of meat
			double pub = -buttBulge * p;
			double bottomPub = AuNoise::Linear_Interpolate(bottomWaist * .75, shape.BottomRadius(), p) * 1.2;
			double topPub = AuNoise::Linear_Interpolate(topWaist, shape.TopRadius(), p);
			double leftPub = AuNoise::Linear_Interpolate(leftWaist, shape.LeftRadius(), p) / 2;
			double rightPub = AuNoise::Linear_Interpolate(rightWaist, shape.RightRadius(), p) / 2;
			torso.Add(pub, AuMeatShape(bottomPub, topPub, leftPub, rightPub));
		}

		// Get positions of arms and legs.
		double lowerRadX = LowerWidthMM() * FeetPerMM() / 2;
		double lowestArms = tlen;
		AuPoint mybone(BoneLoc(dna, 0));
		for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
		{
			const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
			if(tarm)
			{
				AuPoint bone(tarm->BoneLoc(dna, 0));
				float bottomRad = upperRad;
				float topRad = upperRad;
				float leftRad, rightRad;
				if(tarm->Laterality() == Left)
				{
					leftRad = fabsf((float)bone.x) * 1.2f;
					rightRad = ribRadX;
				}
				else
				{
					leftRad = ribRadX;
					rightRad = fabsf((float)bone.x) * 1.2f;
				}
				double spineHt = bone.z - mybone.z;
				torso.Add(spineHt, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				if(spineHt < lowestArms)
					lowestArms = spineHt;
				continue;
			}
			const AuChromoLeg* tleg = dynamic_cast<const AuChromoLeg*>(*ti);
			if(tleg)
			{
				AuPoint bone(tleg->BoneLoc(dna, 0));
				float bottomRad = bottomWaist * .75;	// Belly
				float topRad = topWaist;				// Back
				float leftRad, rightRad;
				switch(tleg->Laterality())
				{
				case 0:		// Left
					leftRad = fabsf((float)bone.x) * 1.1f;
					rightRad = lowerRadX;
					break;
				case 1:		// Middle
				default:
					continue;
				case 2:		// Right
					leftRad = lowerRadX;
					rightRad = fabsf((float)bone.x) * 1.1f;
					break;
				}
				torso.Add(bone.z - mybone.z, AuMeatShape(bottomRad, topRad, leftRad, rightRad));
				continue;
			}
		}

		// Add a rib cage below the lowest arms.
		double ribHt = lowestArms - upperRad;
		if(ribHt > 0)
		{
			float bottomRib = upperRad;
			float topRib = upperRad;
			float leftRib = ribRadX;
			float rightRib = ribRadX;
			AuMeatShape shape(bottomRib, topRib, leftRib, rightRib);
			torso.Add(ribHt, shape);
		}

		int topBottomBlend = 2;	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
		AuPoint blendUpVec(0,0,1);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		torso.GetMesh(mymesh, AuMeatTri::TorsoUpper, topBottomBlend, blendUpVec, blendAngle, blendWid);
		mymesh.SmoothNormals();
		AuMatrix4x4 m;
		m.RotateX(PI/2);
		mymesh.Transform(m);
		if(waistHeight > 0)
			mymesh.SetPartRangeZ(AuMeatTri::TorsoLower, mymesh.Min().z, waistHeight);
		if(neckLen > shoulderBulge)
			mymesh.SetPartRangeZ(AuMeatTri::Neck, tlen + shoulderBulge, mymesh.Max().z);
		if(hasTail)
			mymesh.SetPartRangeZ(AuMeatTri::Tail, mymesh.Min().z, -buttBulge);

		// Add torso features.
		AuMatrix4x4 mTorso;
		dna.GetTorsoFeaturesClothed(mymesh, mTorso, *this, stage);
	}

	AuPointNArray myfol;
	mymesh.SurfacePoints(myfol);

	// Remove follicles from face, eyes and ears.
	QMap<AuPoint, double> hairEx;	// Exclusion locations with radius.
	//DHH dna.TorsoHairExclusions(hairEx, mSkull, *this);

	double topPubHair = waistHeight * .5;
	double chestang = 15*DEG2RAD;			// XY angle forward of which hair is chest.
	double backang = 165*DEG2RAD;			// XY angle behind which hair is back.
	double rearflankang = 45*DEG2RAD;		// XY angle behind which hair is rear flank.
	double tailtip = tailLen * .95;
	double beaverang;
	double slitdist;
	if(maleGenitals)
	{
		beaverang = 45*DEG2RAD;
		slitdist = follicleSpacing;			// pecker peeks out a bit
	}
	else
	{
		beaverang = 45*DEG2RAD;
		slitdist = follicleSpacing/2;		// just barely a slit
	}
	for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
	{
		// Get information about the hair follicle.
		double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.

		// Decide whether the follicle should stay or go.
		AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
		if((*fi).z <= -buttBulge)
		{
			// Tail
			if((*fi).z <= -tailtip)
			{
				if(fabs(xyang) < beaverang)
					role = AuFolliclePoint::TailTipUnder;
				else
					role = AuFolliclePoint::TailTipBack;
			}
			else
			{
				if(fabs(xyang) < beaverang)
					role = AuFolliclePoint::TailUnder;
				else
					role = AuFolliclePoint::TailBack;
			}
		}
		else if((*fi).z <= 0)
		{
			if(fabs(xyang) < beaverang)
			{
				// The slit is bare.
				if(fabs((*fi).x) >= slitdist)
					role = AuFolliclePoint::Groin;	// Outside the bare slit distance.
			}
			else
				role = AuFolliclePoint::Butt;
		}
		else if((*fi).z > tlen + shoulderBulge)
		{
			if(fabs(xyang) > backang)
				role = AuFolliclePoint::NeckBack;
			else if(fabs(xyang) < chestang)
				role = AuFolliclePoint::NeckUnder;
			else
				role = AuFolliclePoint::NeckSides;
		}
		else if(fabs(xyang) < chestang)
		{
			if((*fi).z <= topPubHair)
				role = AuFolliclePoint::Groin;
			else
				role = AuFolliclePoint::Chest;
		}
		else if(fabs(xyang) > backang)
			role = AuFolliclePoint::Back;
		else if((*fi).z >= tlen)
			role = AuFolliclePoint::Shoulders;
		else if(fabs(xyang) > rearflankang)
			role = AuFolliclePoint::FlanksBack;
		else
			role = AuFolliclePoint::FlanksUnder;

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		if(!hairDNA[role])
			continue;	// No hair there.

		if(stage == Juvenile)
		{
			// Skip certain roles in juveniles.
			switch(role)
			{
			//case AuFolliclePoint::Butt:
			case AuFolliclePoint::Chest:
			//case AuFolliclePoint::FlanksBack:
			//case AuFolliclePoint::FlanksUnder:
			case AuFolliclePoint::Groin:
			//case AuFolliclePoint::NeckBack:
			//case AuFolliclePoint::NeckUnder:
			//case AuFolliclePoint::NeckSides:
			//case AuFolliclePoint::Shoulders:
			//case AuFolliclePoint::TailBack:
			//case AuFolliclePoint::TailUnder:
			//case AuFolliclePoint::TailTipBack:
			//case AuFolliclePoint::TailTipUnder:
			//case AuFolliclePoint::Back:
				continue;	// Hairless on juveniles.
			default:
				break;
			}
		}

		/*
		// Check for exclusion by head features such as breasts.
		if(role != AuFolliclePoint::ROLE_Count && !hairEx.empty())
			for(QMap<AuPoint, double>::const_iterator xi = hairEx.cbegin(); xi != hairEx.cend(); xi++)
				if((*fi).Distance2(xi.key()) < (*xi)*(*xi))
				{
					role = AuFolliclePoint::ROLE_Count;
					break;
				}
		*/

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		const QColor& hairColor = hairDNA[role]->Color();
		float maxLen = hairDNA[role]->LengthMM() * FeetPerMM();
		AuFolliclePoint fp(0, role, *fi, hairColor, maxLen);
		follicles.append(fp);
	}
}

// Called on a default constructed object.
quint64 AuChromoTorso::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::Butt) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Chest) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::FlanksBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::FlanksUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Groin) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::NeckBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::NeckUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::NeckSides) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Shoulders) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::TailBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::TailUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::TailTipBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::TailTipUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Back);
}

AuPoint AuChromoTorso::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	Q_UNUSED(dna);
	AuPoint boneLoc(0,0,0);										// Butt bone.
	if(bone == 1)
	{
		double waistHeight = LowerLengthMM() * FeetPerMM();
		boneLoc.z += waistHeight;								// Waist bone.
	}
	else
	{
		bone -= 2;
		quint8 neckBones = NeckBoneCount();
		if(neckBones && bone < neckBones)
		{
			double neckHeight = (LowerLengthMM() + UpperLengthMM()) * FeetPerMM();
			boneLoc.z += neckHeight + NeckLengthMM() * FeetPerMM() * bone / neckBones;		// Neck bones from shoulder to head.
		}
		else
		{
			bone -= neckBones;
			quint8 tailBones = TailBoneCount();
			if(tailBones && bone < tailBones)
				boneLoc.z -= TailLengthMM() * FeetPerMM() * bone / tailBones;	// Tail bones from butt to tip.
		}
	}

	//qDebug() << "torso bone" << bone << "loc" << boneLoc;
	return boneLoc;
}

AuPoint AuChromoTorso::BreastLocationNotOnSkin(const AuChromatid& dna, const AuChromoWhich& which) const
{
	// Breasts go on the upper torso if we have one.
	bool upper;
	if(UpperLengthMM())
		upper = true;
	else
		upper = false;

	double torsoRadius;
	double torsoLen;
	if(upper)
	{
		torsoRadius = UpperThicknessMM() * FeetPerMM() / 2;
		torsoLen = UpperLengthMM() * FeetPerMM();
	}
	else
	{
		torsoRadius = LowerThicknessMM() * FeetPerMM() / 2;
		torsoLen = LowerLengthMM() * FeetPerMM();
	}

	AuPoint breastLoc(0, torsoRadius, 0);
	unsigned int mamms = which.HowMany();
	if(!mamms)
		return breastLoc;

	// Laterality
	LATERALITY laterality = AuChromoMammary::Laterality(which);
	if(laterality != Middle)
	{
		double separationAngle = 45*DEG2RAD;
		if(laterality == Right)
			breastLoc = breastLoc.RotateZ(-separationAngle);	// Right laterality
		else
			breastLoc = breastLoc.RotateZ(separationAngle);		// Left laterality
	}

	// Determine step from the starting location.
	double step = torsoRadius/2;
	if(step > torsoLen/2)
		step = torsoLen/2;

	// Get position based on corresponding arm.
	int arms = 0;
	double lowestArm = 0;
	AuPoint mybone(BoneLoc(dna, 0));
	for(AuChromatid::const_iterator ti = dna.cbegin(); ti != dna.cend(); ti++)
	{
		const AuChromoArm* tarm = dynamic_cast<const AuChromoArm*>(*ti);
		if(!tarm)
			continue;
		arms++;
		AuPoint bone(tarm->BoneLoc(dna, 0));
		double armalt = bone.z - mybone.z - step;
		if(lowestArm == 0 || lowestArm > armalt)
			lowestArm = armalt;
		if(tarm->Which() == which || laterality == Middle)
		{
			breastLoc.z = armalt;
			return breastLoc;
		}
	}

	// Did not find the corresponding arm or didn't find any arms at all.
	// Start at the top.
	if(!arms)
	{
		// No arms so calculate top of spine for first loc.
		if(upper)
			lowestArm = (LowerLengthMM() + UpperLengthMM()) * FeetPerMM();
		else
			lowestArm = LowerLengthMM() * FeetPerMM();
		lowestArm -= step;
		if(laterality == Middle && which.Index() == mamms-1)
		{
			// Odd centered mammary is in the top row.
			breastLoc.z = lowestArm;
			return breastLoc;
		}
	}

	// This is not the odd mammary. Calculate vertical position of its row.
	double base;
	if(upper)
		base = LowerLengthMM() * FeetPerMM();
	else
		base = 0;

	double range = lowestArm - base;
	if(range <= 0)
		qWarning() << "warning: AuChromoTorso::BreastLocationNotOnSkin bad range" << range;

	// Start at the top.
	int rows = mamms/2;
	double spacing = range / rows;
	if(spacing > step*2)
		spacing = step*2;
	int row = which.Index()/2;
	breastLoc.z = lowestArm - spacing * row;
	return breastLoc;
}

AuPoint AuChromoTorso::BreastLocation(const AuChromatid& dna, const AuMeatMesh& torsoMesh, const AuChromoWhich& which) const
{
	AuPoint breastLoc = BreastLocationNotOnSkin(dna, which);

	// Breasts go on the upper torso if we have one.
	AuMeatTri::PART partType;
	if(UpperLengthMM())
		partType = AuMeatTri::TorsoUpper;
	else
		partType = AuMeatTri::TorsoLower;

	// Adjust the breast location to the skin surface.
	AuPoint skinN;
	breastLoc.y = 0;	// Start at the spine.
	breastLoc = torsoMesh.SkinSurface(breastLoc, AuPoint(0,1,0), skinN, partType);	// Look forward for a surface location.
	return breastLoc;
}

int AuChromoTorso::ListWidgetOrder() const
{
	return 200;
}

QString AuChromoTorso::ListWidgetText() const
{
	return QObject::tr("Torso");
}

QWidget* AuChromoTorso::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageTorso* pp = new AuChromoPageTorso(parent);
	return pp;
}

void AuChromoTorso::SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
{
	for(QList<AuAnimalBone>::const_iterator bi = bones.cbegin(); bi != bones.cend(); bi++)
	{
		AuPoint bloc((*bi).PosedLoc());
		min = min.Min(bloc);
		max = max.Max(bloc);
	}
}

bool AuChromoTorso::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
{
	double lowerZ = m_LowerLengthMM * FeetPerMM();
	double lowerX = m_LowerWidthMM * FeetPerMM() / 2;
	double lowerY = m_LowerThicknessMM * FeetPerMM() / 2;
	double upperZ = m_UpperLengthMM * FeetPerMM();
	double upperX = m_UpperWidthMM * FeetPerMM() / 2;
	double upperY = m_UpperThicknessMM * FeetPerMM() / 2;
	double neckZ = m_NeckLengthMM * FeetPerMM();

	// Use bone matrices to put the solids at their posed locations.
	AuMatrix4x4 ml(bones[0].Matrix());
	ml.Translate(bones[0].UnposedLoc());
	AuSolidConoid lower(ml, lowerZ, lowerX, lowerY, lowerX, lowerY, true, false);

	AuMatrix4x4 mu(bones[1].Matrix());
	mu.Translate(bones[1].UnposedLoc());
	AuSolidConoid upper(mu, upperZ, lowerX, lowerY, upperX, upperY, false, true);

	bool sighted = lower.SightTarget(eyeLoc, sightedLoc);
	if(upper.SightTarget(eyeLoc, sightedLoc))
		sighted = true;

	quint8 neckBones = NeckBoneCount();
	if(neckBones)
	{
		double neckSegLen = neckZ / neckBones;
		double neckX = upperX / 4;
		double neckY = upperY / 4;
		for(int nb=0; nb<neckBones; nb++)
		{
			AuMatrix4x4 mu(bones[nb + 2].Matrix());
			mu.Translate(bones[nb + 2].UnposedLoc());
			AuSolidConoid neckSeg(mu, neckSegLen, neckX, neckY, neckX, neckY, false, true);
			if(neckSeg.SightTarget(eyeLoc, sightedLoc))
				sighted = true;
		}
	}

	// tail? DHH animalia

	return sighted;
}

void AuChromoTorso::AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
		float currentHeadTilt, float currentHeadTurn, HANDEDNESS hand, bool hasMaleGenitals,
		float& buttX, float& buttY, float& buttZ, float& chestX, float& chestY, float& chestZ, float& neckX, float& neckY,
		float& neckZ, float& tailX, float& tailY)
{
	buttX = 0;
	buttY = 0;
	buttZ = 0;
	chestX = 0;
	chestY = 0;
	chestZ = 0;
	neckX = 0;
	neckY = 0;
	neckZ = 0;
	tailX = 0;
	tailY = 0;

	// Eyes share the turn with the head/neck.
	if(currentHeadTurn != 0)
	{
		// Assume I can turn my head just far enough to see 180 straight back in my peripheral vision.
		float peripheralVisionAngle = 60*DEG2RADf;	// Peripheral vision to each side of the center line.
		float turnAngle = fabs(currentHeadTurn);	// So we can work with positive numbers.
		float maxTurn = PIf - peripheralVisionAngle;

		// Turn everything to the max.
		if(currentHeadTurn < 0)
		{
			neckZ = -PIf/2;
			//eyeRightZ = -(PIf/2 - peripheralVisionAngle);
		}
		else
		{
			neckZ = PIf/2;
			//eyeRightZ = PIf/2 - peripheralVisionAngle;
		}

		if(turnAngle < maxTurn)
		{
			float frac = turnAngle / maxTurn;
			neckZ *= frac;
			//eyeRightZ *= frac;
		}

		//eyeLeftZ = eyeRightZ;
	}

	//chestX = -when.SecCycle() * PIf/4;	//DHH
	//chestY = when.SecCycle() * PIf/2 - (PIf/4);	//DHH
	//chestZ = when.SecCycle() * PIf/2 - (PIf/4);	//DHH

	// Neck angles.
	// If this torso has a neck, the position of the neck determines the position of the head.
	switch(pose.Pose())
	{
	// Special
	case AuPose::Init:
	case AuPose::Spawn:
		break;

	// Head Only
	case AuPose::EyesOnly:
		neckX = currentHeadTilt;
		break;

	// Horizontal Poses
	case AuPose::Prone:
	case AuPose::ProneCreep:
		buttX = -PIf/2;
		neckX = PIf/4 + currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		buttX = -PIf/2;
		neckX = PIf/4 + currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		buttX = -PIf/2;
		buttZ = PIf/2;
		neckX = currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		buttX = -PIf/2;
		buttZ = -PIf/2;
		neckX = currentHeadTilt;
		break;

	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
	case AuPose::Patient_Prone:			// Prone patient faces up.
		buttX = -PIf/2;
		buttZ = PIf;
		neckX = currentHeadTilt;
		break;

	// Angles for dead poses are slightly tighter than writhe poses, and inanimate.
	case AuPose::DeadFaceDown:
		buttX = -PIf/2;
		neckX = -10*DEG2RADf;
		if(hand == LeftHanded)
		{
			neckY = -20*DEG2RADf;
			neckZ = 80*DEG2RADf;
		}
		else
		{
			neckY = 20*DEG2RADf;
			neckZ = -80*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceLeft:
		buttX = -PIf/2;
		buttZ = PIf/2;
		neckX = -30*DEG2RADf;
		neckY = 15*DEG2RADf;
		break;
	case AuPose::DeadFaceRight:
		buttX = -PIf/2;
		buttZ = -PIf/2;
		neckX = -30*DEG2RADf;
		neckY = -15*DEG2RADf;
		break;
	case AuPose::DeadFaceUp:
		buttX = -PIf/2;
		buttZ = PIf;
		if(hand == LeftHanded)
		{
			neckY = 15*DEG2RADf;
			neckZ = -40*DEG2RADf;
		}
		else
		{
			neckY = -15*DEG2RADf;
			neckZ = 40*DEG2RADf;
		}
		break;

	// Angles for writhe/dying poses are based on dead poses, only animated to show signs of life.
	case AuPose::WritheFaceDown:
		buttX = -PIf/2;
		neckX = PIf/4 + currentHeadTilt;
		break;
	case AuPose::WritheFaceLeft:
		buttX = -PIf/2;
		buttZ = PIf/2;
		neckX = currentHeadTilt;
		break;
	case AuPose::WritheFaceRight:
		buttX = -PIf/2;
		buttZ = -PIf/2;
		neckX = currentHeadTilt;
		break;
	case AuPose::WritheFaceUp:
		buttX = -PIf/2;
		buttZ = PIf;
		neckX = currentHeadTilt;
		break;

	case AuPose::MateOnBottom:
	case AuPose::ReceptiveMate:
		buttX = PIf/2;
		chestX = -5*DEG2RADf;
		neckX = currentHeadTilt - 20*DEG2RADf;
		break;

	case AuPose::MateOnTop:
		if(hasMaleGenitals)
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			buttX = -45*DEG2RADf;
			buttY = 0;
			buttZ = 0;
			chestX = -5*DEG2RADf - perang*10;
			chestY = 0;
			chestZ = 0;
			neckX = currentHeadTilt + 20*DEG2RADf;
			neckY = 0;
			neckZ = 0;
			tailX = -45*DEG2RADf;
			tailY = 0;
		}
		else
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			chestX = -15*DEG2RADf - perang*20;
			neckX = currentHeadTilt + 20*DEG2RADf;
		}
		break;

	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		neckX = currentHeadTilt;
		//DHH animalia
		break;

	case AuPose::Swim:
	case AuPose::SwimFast:
		buttX = -PIf/2;
		neckX = PIf/4 + currentHeadTilt;
		break;
	case AuPose::TreadWater:
		neckX = currentHeadTilt;
		break;

	// Sitting Poses
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Post_Sit:
		buttX = 30*DEG2RADf;
		chestX = -30*DEG2RADf;
		neckX = currentHeadTilt;
		break;

	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
		buttX = 30*DEG2RADf;
		chestX = -30*DEG2RADf;
		neckX = currentHeadTilt;
		break;

	case AuPose::Straddle:
		buttX = 30*DEG2RADf;
		chestX = -30*DEG2RADf;
		neckX = currentHeadTilt;
		break;

	// Crouched Poses
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
		neckX = currentHeadTilt;
		neckY = 0*DEG2RADf;
		chestX = -20*DEG2RADf;
		break;

	// Standing
	case AuPose::Stand:
	case AuPose::Post_Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
	case AuPose::StandRun:
	case AuPose::StandWalk:
		neckX = currentHeadTilt;
		break;

	// Misc
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		neckX = currentHeadTilt;
		neckY = 0*DEG2RADf;
		break;

	// Mouth Poses
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}

	tailX = -90*DEG2RADf;
	tailY = 0;

	// s = sec
	// c = cycle
	// s/c = sec / cycle = 1 / hz
	// ms/c = s/c * 1000ms/s
	qint64 periodMs = 1000.f/bodyHz;
	tailY = when.Cycle(periodMs) * PIf*2;	//DHH
	if(tailY < PIf)
		tailY -= PIf/2;
	else
		tailY = (PIf*2 - tailY) - PIf/2;
}

float AuChromoTorso::Volume() const
{
	float lowerZ = m_LowerLengthMM * FeetPerMM();
	float lowerX = m_LowerWidthMM * FeetPerMM() / 2;
	float lowerY = m_LowerThicknessMM * FeetPerMM() / 2;
	float upperZ = m_UpperLengthMM * FeetPerMM();
	float upperX = m_UpperWidthMM * FeetPerMM() / 2;
	float upperY = m_UpperThicknessMM * FeetPerMM() / 2;
	float neckZ = m_NeckLengthMM * FeetPerMM();
	float tailZ = m_TailLengthMM * FeetPerMM();

	// Area of ellipse
	// A = PI * a * b
	// a and b are radii.
	float al = PIf * lowerX * lowerY;
	float v = al * lowerZ;

	float au = PIf * upperX * upperY;
	v += au * upperZ;

	float an = au / 4;	// Neck cross section area is about 1/4 of upper torso.
	v += an * neckZ;

	float at = al / 4;	// Tail cross section area is about 1/4 of lower torso.
	v += at * tailZ;
	return v;
}

float AuChromoTorso::TailVolume() const
{
	float lowerX = m_LowerWidthMM * FeetPerMM() / 2;
	float lowerY = m_LowerThicknessMM * FeetPerMM() / 2;
	float tailZ = m_TailLengthMM * FeetPerMM();

	// Area of ellipse
	// A = PI * a * b
	// a and b are radii.
	float al = PIf * lowerX * lowerY;

	float at = al / 4;	// Tail cross section area is about 1/4 of lower torso.
	float v = at * tailZ;
	return v;
}

bool AuChromoTorso::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoTorso* c = (const AuChromoTorso*)chromosome;
	//float dif = m_LengthMM / c->m_LengthMM;
	//if(dif < .5 || dif > 1.5)
		//return false;
	//float rdif = m_RadiusMM / c->m_RadiusMM;
	//if(rdif < .5 || rdif > 1.5)
		//return false;
	return true;
}

AuChromosome* AuChromoTorso::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoTorso* c = dynamic_cast<const AuChromoTorso*>(g);
	if(!c)
		return 0;
	AuChromoTorso* comb = new AuChromoTorso(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoArm::AuChromoArm(const AuChromoWhich& which,
		quint16 shoulderToSpineMM,
		quint16 shoulderRadMM,
		quint16 upperArmLengthMM,
		quint16 elbowRadMM,
		quint16 foreArmLenMM,
		quint16 wristRadMM,
		quint16 handLenMM,
		quint8 legBits,
		quint8 standTone)
{
	m_ElbowRadMM = elbowRadMM;
	m_ForeArmLengthMM = foreArmLenMM;
	m_HandLengthMM = handLenMM;
	m_LegBits = legBits;
	m_ShoulderRadMM = shoulderRadMM;
	m_ShoulderToSpineMM = shoulderToSpineMM;
	m_StandTone = standTone;
	m_UpperArmLengthMM = upperArmLengthMM;
	m_Which = which;
	m_WristRadMM = wristRadMM;
}

AuChromoArm::AuChromoArm()
{
	m_ElbowRadMM = 20;
	m_ForeArmLengthMM = 100;
	m_HandLengthMM = 100;
	m_LegBits = 0;
	m_ShoulderRadMM = 20;
	m_ShoulderToSpineMM = 80;
	m_StandTone = 100;
	m_UpperArmLengthMM = 100;
	m_WristRadMM = 20;
}

AuChromoArm::AuChromoArm(const AuChromoArm& c) : AuChromosomeWithGenes(c)
{
	m_ElbowRadMM = c.m_ElbowRadMM;
	m_ForeArmLengthMM = c.m_ForeArmLengthMM;
	m_HandLengthMM = c.m_HandLengthMM;
	m_LegBits = c.m_LegBits;
	m_ShoulderRadMM = c.m_ShoulderRadMM;
	m_ShoulderToSpineMM = c.m_ShoulderToSpineMM;
	m_StandTone = c.m_StandTone;
	m_UpperArmLengthMM = c.m_UpperArmLengthMM;
	m_Which = c.m_Which;
	m_WristRadMM = c.m_WristRadMM;
}

AuChromoArm::AuChromoArm(const AuChromoArm& c1, const AuChromoArm& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoArm which mismatch" << c1.m_Which << c2.m_Which;
	m_ElbowRadMM = CombineRange(c1.m_ElbowRadMM, c2.m_ElbowRadMM, dice);
	dice.Random();
	m_ForeArmLengthMM = CombineRange(c1.m_ForeArmLengthMM, c2.m_ForeArmLengthMM, dice);
	dice.Random();
	m_HandLengthMM = CombineRange(c1.m_HandLengthMM, c2.m_HandLengthMM, dice);
	dice.Random();
	m_LegBits = (quint8)CombineBits(c1.m_LegBits, c2.m_LegBits, dice);
	dice.Random();
	m_ShoulderRadMM = CombineRange(c1.m_ShoulderRadMM, c2.m_ShoulderRadMM, dice);
	dice.Random();
	m_ShoulderToSpineMM = CombineRange(c1.m_ShoulderToSpineMM, c2.m_ShoulderToSpineMM, dice);
	dice.Random();
	m_StandTone = CombineRange(c1.m_StandTone, c2.m_StandTone, dice);
	dice.Random();
	m_UpperArmLengthMM = CombineRange(c1.m_UpperArmLengthMM, c2.m_UpperArmLengthMM, dice);
	dice.Random();
	m_Which = c1.m_Which;
	m_WristRadMM = CombineRange(c1.m_WristRadMM, c2.m_WristRadMM, dice);
}

void AuChromoArm::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoArm& c = dynamic_cast<const AuChromoArm&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_ElbowRadMM = c.m_ElbowRadMM;
		m_ForeArmLengthMM = c.m_ForeArmLengthMM;
		m_HandLengthMM = c.m_HandLengthMM;
		m_LegBits = c.m_LegBits;
		m_ShoulderRadMM = c.m_ShoulderRadMM;
		m_ShoulderToSpineMM = c.m_ShoulderToSpineMM;
		m_StandTone = c.m_StandTone;
		m_UpperArmLengthMM = c.m_UpperArmLengthMM;
		m_Which = c.m_Which;
		m_WristRadMM = c.m_WristRadMM;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoArm::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_ElbowRadMM;
	out << m_ForeArmLengthMM;
	out << m_HandLengthMM;
	out << m_LegBits;
	out << m_ShoulderRadMM;
	out << m_ShoulderToSpineMM;
	out << m_StandTone;
	out << m_UpperArmLengthMM;
	out << m_Which;
	out << m_WristRadMM;
}

void AuChromoArm::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_ElbowRadMM;
	in >> m_ForeArmLengthMM;
	in >> m_HandLengthMM;
	in >> m_LegBits;
	in >> m_ShoulderRadMM;
	in >> m_ShoulderToSpineMM;
	in >> m_StandTone;
	in >> m_UpperArmLengthMM;
	in >> m_Which;
	in >> m_WristRadMM;
}

void AuChromoArm::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_ElbowRadMM, d_DNAMaxMM);
	out << ToLogByte(m_ForeArmLengthMM, d_DNAMaxMM);
	out << ToLogByte(m_HandLengthMM, d_DNAMaxMM);
	out << m_LegBits;
	out << ToLogByte(m_ShoulderRadMM, d_DNAMaxMM);
	out << ToLogByte(m_ShoulderToSpineMM, d_DNAMaxMM);
	out << m_StandTone;
	out << ToLogByte(m_UpperArmLengthMM, d_DNAMaxMM);
	out << m_Which;
	out << ToLogByte(m_WristRadMM, d_DNAMaxMM);
}

void AuChromoArm::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_ElbowRadMM = ReadLogByte(in, d_DNAMaxMM);
	m_ForeArmLengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_HandLengthMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_LegBits;
	m_ShoulderRadMM = ReadLogByte(in, d_DNAMaxMM);
	m_ShoulderToSpineMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_StandTone;
	m_UpperArmLengthMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_Which;
	m_WristRadMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoArm::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_ElbowRadMM = dice.RandomLogValue(m_ElbowRadMM, d_DNAMaxMM);
	m_ForeArmLengthMM = dice.RandomLogValue(m_ForeArmLengthMM, d_DNAMaxMM);
	m_HandLengthMM = dice.RandomLogValue(m_HandLengthMM, d_DNAMaxMM);
	m_ShoulderRadMM = dice.RandomLogValue(m_ShoulderRadMM, d_DNAMaxMM);
	m_ShoulderToSpineMM = dice.RandomLogValue(m_ShoulderToSpineMM, d_DNAMaxMM);
	m_UpperArmLengthMM = dice.RandomLogValue(m_UpperArmLengthMM, d_DNAMaxMM);
	m_WristRadMM = dice.RandomLogValue(m_WristRadMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoArm::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalArm(*this, dna, firstBone);
}

void AuChromoArm::GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
		const AuClothing& clothes, GROWTHSTAGE stage) const
{
	Q_UNUSED(stage);
	if(clothes.BareUpperArmCovered() && clothes.BareForeArmCovered() && clothes.BareHandCovered())
		return;

	// Create the mesh of the arm.
	AuMeatMesh mymesh;
	{
		double foreArmLen = ForeArmLengthMM() * FeetPerMM();
		double handLen = HandLengthMM() * FeetPerMM();
		double shoulderRad = ShoulderRadiusMM() * FeetPerMM();
		double upperArmLen = UpperArmLengthMM() * FeetPerMM();
		double wristRad = WristRadiusMM() * FeetPerMM();
		double wristThick = wristRad * .6;
		double deltoidRad = shoulderRad * 1.1;
		double handWid = wristRad * 1.5;
		double handHt = handWid * .6;
		double palmLen = handLen/2;
		double palmRad = palmLen/2;

		AuMatrix4x4 ms;

		// Arm
		AuSolid solidArm;
		double elbowRad = ElbowRadiusMM() * FeetPerMM();
		double elbowThick = elbowRad * .9;
		double shoulderX = shoulderRad * .9;
		double deltoidX = shoulderX * 1.1;
		double deltoidHt = deltoidRad;

		// Build the arm down to the wrist, excluding the hand.
		LATERALITY laterality = Laterality();
		if(laterality == Left)
			ms.Translate(-(deltoidX-shoulderX),deltoidRad-shoulderRad,0);
		else
			ms.Translate(deltoidX-shoulderX,deltoidRad-shoulderRad,0);
		AuSolidEllipsoid deltoid(ms, shoulderX, deltoidRad, deltoidHt);
		solidArm.Add(deltoid);
		ms.LoadIdentity();
		AuSolidDomoid shoulder(ms, shoulderX, shoulderRad, shoulderX, false);
		solidArm.Add(shoulder);
		ms.RotateX(PI);
		AuSolidConoid upperArm(ms, upperArmLen, shoulderX, shoulderRad, elbowThick, elbowRad, false, false);
		solidArm.Add(upperArm);
		ms.Translate(0,0,upperArmLen);
		AuSolidConoid foreArm(ms, foreArmLen, elbowThick, elbowRad, wristThick, wristRad, false, false);
		solidArm.Add(foreArm);
		ms.Translate(0,0,foreArmLen);
		AuSolidDomoid wrist(ms, wristThick, wristRad, wristThick, false);
		solidArm.Add(wrist);

		// Hand
		AuSolid solidFingers;
		AuSolid solidHand;
		AuSolid solidThumb;
		if(!clothes.BareHandCovered())
		{
			double fingerLenMid = palmLen;
			double fingerLenIndex = fingerLenMid * .9;
			double fingerLenRing = fingerLenMid * .9;
			if(dna.HasMaleGenitals())
				fingerLenRing *= 1.05;
			double fingerLenPinky = fingerLenMid * .75;
			double fingerLenThumb = fingerLenMid * .75;
			double fingerRad = wristRad * .35;
			double fingerRadTip = fingerRad * .7;
			double fingerSpacing = fingerRad * 1.7;

			ms.Translate(0,0,handLen/2 - palmRad);	// Center of palm
			if(laterality == Left)
				ms.RotateZ(-PI/2);
			else
				ms.RotateZ(PI/2);
			ms.RotateX(PI/2);

			AuMatrix4x4 mfb(ms);	// Base transform for fingers.
			mfb.RotateX(-PI/2);		// Orient to palm.

			ms.Translate(0,0,-handHt * .25);
			AuSolidDomoid palm(ms, handWid, palmRad, handHt, true);
			solidHand.Add(palm);

			// Fingers
			AuMatrix4x4 mFmid(mfb);
			mFmid.Translate(0,0,palmRad);
			AuMatrix4x4 mFmidp(mFmid);
			mFmidp.RotateX(PI);
			AuSolidDomoid fmidp(mFmidp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
			solidFingers.Add(fmidp);
			AuSolidConoid fmid(mFmid, fingerLenMid - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
			solidFingers.Add(fmid);
			mFmid.Translate(0,0,fingerLenMid - fingerRadTip);
			AuSolidDomoid fmidt(mFmid, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
			solidFingers.Add(fmidt);

			AuMatrix4x4 mFring(mfb);
			if(laterality == Left)
				mFring.Translate(-fingerSpacing,0,palmRad*.9);
			else
				mFring.Translate(fingerSpacing,0,palmRad*.9);
			AuMatrix4x4 mFringp(mFring);
			mFringp.RotateX(PI);
			AuSolidDomoid fringp(mFringp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
			solidFingers.Add(fringp);
			AuSolidConoid fring(mFring, fingerLenRing - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
			solidFingers.Add(fring);
			mFring.Translate(0,0,fingerLenRing - fingerRadTip);
			AuSolidDomoid fringt(mFring, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
			solidFingers.Add(fringt);

			AuMatrix4x4 mFpinky(mfb);
			if(laterality == Left)
				mFpinky.Translate(-fingerSpacing*2,0,palmRad*.7);
			else
				mFpinky.Translate(fingerSpacing*2,0,palmRad*.7);
			AuMatrix4x4 mFpinkyp(mFpinky);
			mFpinkyp.RotateX(PI);
			AuSolidDomoid fpinkyp(mFpinkyp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
			solidFingers.Add(fpinkyp);
			AuSolidConoid fpinky(mFpinky, fingerLenPinky - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
					false);
			solidFingers.Add(fpinky);
			mFpinky.Translate(0,0,fingerLenPinky - fingerRadTip);
			AuSolidDomoid fpinkyt(mFpinky, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
			solidFingers.Add(fpinkyt);

			AuMatrix4x4 mFindex(mfb);
			if(laterality == Left)
				mFindex.Translate(fingerSpacing,0,palmRad*.9);
			else
				mFindex.Translate(-fingerSpacing,0,palmRad*.9);
			AuMatrix4x4 mFindexp(mFindex);
			mFindexp.RotateX(PI);
			AuSolidDomoid findexp(mFindexp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
			solidFingers.Add(findexp);
			AuSolidConoid findex(mFindex, fingerLenIndex - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
					false);
			solidFingers.Add(findex);
			mFindex.Translate(0,0,fingerLenIndex - fingerRadTip);
			AuSolidDomoid findext(mFindex, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
			solidFingers.Add(findext);

			AuMatrix4x4 mFthumb(mfb);
			if(laterality == Left)
			{
				mFthumb.Translate(fingerSpacing*2,0,-palmRad*.25);
				mFthumb.RotateY(20*DEG2RAD);
				mFthumb.RotateX(-30*DEG2RAD);
			}
			else
			{
				mFthumb.Translate(-fingerSpacing*2,0,-palmRad*.25);
				mFthumb.RotateY(-20*DEG2RAD);
				mFthumb.RotateX(-30*DEG2RAD);
			}
			AuMatrix4x4 mFthumbp(mFthumb);
			mFthumbp.RotateX(PI);
			AuSolidDomoid fthumbp(mFthumbp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
			solidThumb.Add(fthumbp);
			AuSolidConoid fthumb(mFthumb, fingerLenThumb - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
					false);
			solidThumb.Add(fthumb);
			mFthumb.Translate(0,0,fingerLenThumb - fingerRadTip);
			AuSolidDomoid fthumbt(mFthumb, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
			solidThumb.Add(fthumbt);
		}

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec;
		if(laterality == Left)
			blendUpVec = AuPoint(-1,0,0).RotateZ(45*DEG2RAD);
		else
			blendUpVec = AuPoint(1,0,0).RotateZ(-45*DEG2RAD);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;

		// Meshes are added in pieces for part identification.
		// Arm mesh.
		float triangleSize = .75f/12.f;
		AuScFaceList faces;
		solidArm.Faces(faces, triangleSize);
		mymesh.FromFaces(faces, AuMeatTri::Arm);
		if(clothes.BareUpperArmCovered() && clothes.BareForeArmCovered())
			mymesh.RemoveAbove(-(upperArmLen + foreArmLen * .9f));
		else if(clothes.BareUpperArmCovered())
			mymesh.RemoveAbove(-upperArmLen * .9f);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, upperArmLen + foreArmLen, topBottomBlend, blendAngle,
				blendWid);

		// Add arm features.
		//DHH dna.GetArmFeatures(mymesh, mNeck, *this);

		// Hand mesh.
		AuMeatMesh handmesh;
		triangleSize = .6f/12.f;
		faces.Clear();
		solidHand.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Hand);

		// Fingers mesh.
		triangleSize = .5f/12.f;
		faces.Clear();
		solidFingers.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Finger);

		// Thumb mesh.
		faces.Clear();
		solidThumb.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Thumb);
		handmesh.SmoothNormals();
		handmesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, handLen, topBottomBlend, blendAngle, blendWid);
		mymesh.Append(handmesh);

		// Hair
		{
			AuFolliclePointArray follicles;
			const float lockWid = triangleSize * 2;
			HairFollicles(dna, lockWid, follicles);
			float headSpace = .25f/12;
			float lockLen;
			float hangLength;
			float knotLength;
			AuPoint combVec;
			bool basePatch;
			float widAng;
			bool wearsBra = dna.HasBreasts();
			for(AuFolliclePointArray::const_iterator hi = follicles.cbegin(); hi != follicles.cend(); hi++)
			{
				// Get information about the lock of hair.
				AuPoint knotLoc;
				AuHairStyleLock::STYLE lstyle = hairStyle.Lock(*hi, headSpace, lockLen, hangLength, knotLength, widAng, combVec,
						basePatch);
				if(lockLen == 0)
					continue;

				// Clothes may remove or shorten hair.
				if(clothes.CoverHair(*hi, basePatch, lockLen, wearsBra))
					continue;	// Hair is completely removed including the base patch, or there was no base patch.
				else if(lockLen == 0)
				{
					lstyle = AuHairStyleLock::Shaved;	// So we get only a base patch, no stubble.
					lockLen = 1;
				}

				// Make a lock of hair at the follicle.
				mymesh.Hair().MakeLock(*hi, lstyle, headSpace, lockWid, lockLen, hangLength, knotLoc, knotLength, widAng, combVec,
						basePatch, 0);
			}
		}

		// Done adding polygons to the arm. Set the bone to use by it's vertices.
		// Shoulder to elbow is bone 0.
		mymesh.SetBonesRangeZ(-upperArmLen, deltoidRad, firstBone,1., 0,0., 0,0., 0,0.);
		// Elbow to wrist is bone 1.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen), -upperArmLen, firstBone+1,1., 0,0., 0,0., 0,0.);
		// Wrist to middle of hand is bone 2.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen/2), -(upperArmLen+foreArmLen), firstBone+2,1., 0,0., 0,0., 0,0.);
		// Middle of hand to middle of fingers 3.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen+.1), -(upperArmLen+foreArmLen+handLen*.5f),
				firstBone+3,1., 0,0., 0,0., 0,0.);
		// Middle of fingers to tip of fingers is bone 3.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen+handLen*.5f), -(upperArmLen+foreArmLen+handLen*.75f),
				firstBone+4,1., 0,0., 0,0., 0,0.);
	}

	AuMatrix4x4 m;
	m.Translate(dna.ArmLocation(m_Which));
	mymesh.Transform(m);
	//qDebug() << "AuChromoArm::GetSkinMesh mymesh" << mymesh.Count();
	mesh.Append(mymesh);
}

void AuChromoArm::GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const
{
	Q_UNUSED(stage);
	// Create the mesh of the arm.
	AuMeatMesh mymesh;
	{
		double shoulderRad = ShoulderRadiusMM() * FeetPerMM();
		double wristRad = WristRadiusMM() * FeetPerMM();
		double elbowRad = ElbowRadiusMM() * FeetPerMM();
		double foreArmLen = ForeArmLengthMM() * FeetPerMM();
		double upperArmLen = UpperArmLengthMM() * FeetPerMM();
		double handLen = HandLengthMM() * FeetPerMM();

		double wristThick = wristRad * .6;
		double elbowThick = elbowRad * .9;
		double shoulderX = shoulderRad * .9;
		double handWid = wristRad * 1.5;
		double handHt = handWid * .6;
		double palmLen = handLen/2;
		double palmRad = palmLen/2;
		double fingerLenMid = palmLen;
		double fingerLenIndex = fingerLenMid * .9;
		double fingerLenRing = fingerLenMid * .9;
		if(dna.HasMaleGenitals())
			fingerLenRing *= 1.05;
		double fingerLenPinky = fingerLenMid * .75;
		double fingerLenThumb = fingerLenMid * .75;
		double fingerRad = wristRad * .35;
		double fingerRadTip = fingerRad * .7;
		double fingerSpacing = fingerRad * 1.7;
		double deltoidX = shoulderX * 1.1;
		double deltoidRad = shoulderRad * 1.1;
		double deltoidHt = deltoidRad;

		// Build the arm down to the wrist, excluding the hand.
		AuSolid solidArm;
		AuMatrix4x4 ms;
		LATERALITY laterality = Laterality();
		if(laterality == Left)
			ms.Translate(-(deltoidX-shoulderX),deltoidRad-shoulderRad,0);
		else
			ms.Translate(deltoidX-shoulderX,deltoidRad-shoulderRad,0);
		AuSolidEllipsoid deltoid(ms, shoulderX, deltoidRad, deltoidHt);
		solidArm.Add(deltoid);
		ms.LoadIdentity();
		AuSolidDomoid shoulder(ms, shoulderX, shoulderRad, shoulderX, false);
		solidArm.Add(shoulder);
		ms.RotateX(PI);
		AuSolidConoid upperArm(ms, upperArmLen, shoulderX, shoulderRad, elbowThick, elbowRad, false, false);
		solidArm.Add(upperArm);
		ms.Translate(0,0,upperArmLen);
		AuSolidConoid foreArm(ms, foreArmLen, elbowThick, elbowRad, wristThick, wristRad, false, false);
		solidArm.Add(foreArm);
		ms.Translate(0,0,foreArmLen);
		AuSolidDomoid wrist(ms, wristThick, wristRad, wristThick, false);
		solidArm.Add(wrist);

		// Hand
		AuSolid solidHand;
		ms.Translate(0,0,handLen/2 - palmRad);	// Center of palm
		if(laterality == Left)
			ms.RotateZ(-PI/2);
		else
			ms.RotateZ(PI/2);
		ms.RotateX(PI/2);

		AuMatrix4x4 mfb(ms);	// Base transform for fingers.
		mfb.RotateX(-PI/2);		// Orient to palm.

		ms.Translate(0,0,-handHt * .25);
		AuSolidDomoid palm(ms, handWid, palmRad, handHt, true);
		solidHand.Add(palm);

		// Fingers
		AuSolid solidFingers;
		AuMatrix4x4 mFmid(mfb);
		mFmid.Translate(0,0,palmRad);
		AuMatrix4x4 mFmidp(mFmid);
		mFmidp.RotateX(PI);
		AuSolidDomoid fmidp(mFmidp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fmidp);
		AuSolidConoid fmid(mFmid, fingerLenMid - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
		solidFingers.Add(fmid);
		mFmid.Translate(0,0,fingerLenMid - fingerRadTip);
		AuSolidDomoid fmidt(mFmid, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fmidt);

		AuMatrix4x4 mFring(mfb);
		if(laterality == Left)
			mFring.Translate(-fingerSpacing,0,palmRad*.9);
		else
			mFring.Translate(fingerSpacing,0,palmRad*.9);
		AuMatrix4x4 mFringp(mFring);
		mFringp.RotateX(PI);
		AuSolidDomoid fringp(mFringp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fringp);
		AuSolidConoid fring(mFring, fingerLenRing - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
		solidFingers.Add(fring);
		mFring.Translate(0,0,fingerLenRing - fingerRadTip);
		AuSolidDomoid fringt(mFring, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fringt);

		AuMatrix4x4 mFpinky(mfb);
		if(laterality == Left)
			mFpinky.Translate(-fingerSpacing*2,0,palmRad*.7);
		else
			mFpinky.Translate(fingerSpacing*2,0,palmRad*.7);
		AuMatrix4x4 mFpinkyp(mFpinky);
		mFpinkyp.RotateX(PI);
		AuSolidDomoid fpinkyp(mFpinkyp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fpinkyp);
		AuSolidConoid fpinky(mFpinky, fingerLenPinky - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidFingers.Add(fpinky);
		mFpinky.Translate(0,0,fingerLenPinky - fingerRadTip);
		AuSolidDomoid fpinkyt(mFpinky, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fpinkyt);

		AuMatrix4x4 mFindex(mfb);
		if(laterality == Left)
			mFindex.Translate(fingerSpacing,0,palmRad*.9);
		else
			mFindex.Translate(-fingerSpacing,0,palmRad*.9);
		AuMatrix4x4 mFindexp(mFindex);
		mFindexp.RotateX(PI);
		AuSolidDomoid findexp(mFindexp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(findexp);
		AuSolidConoid findex(mFindex, fingerLenIndex - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidFingers.Add(findex);
		mFindex.Translate(0,0,fingerLenIndex - fingerRadTip);
		AuSolidDomoid findext(mFindex, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(findext);

		AuSolid solidThumb;
		AuMatrix4x4 mFthumb(mfb);
		if(laterality == Left)
		{
			mFthumb.Translate(fingerSpacing*2,0,-palmRad*.25);
			mFthumb.RotateY(20*DEG2RAD);
			mFthumb.RotateX(-30*DEG2RAD);
		}
		else
		{
			mFthumb.Translate(-fingerSpacing*2,0,-palmRad*.25);
			mFthumb.RotateY(-20*DEG2RAD);
			mFthumb.RotateX(-30*DEG2RAD);
		}
		AuMatrix4x4 mFthumbp(mFthumb);
		mFthumbp.RotateX(PI);
		AuSolidDomoid fthumbp(mFthumbp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidThumb.Add(fthumbp);
		AuSolidConoid fthumb(mFthumb, fingerLenThumb - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidThumb.Add(fthumb);
		mFthumb.Translate(0,0,fingerLenThumb - fingerRadTip);
		AuSolidDomoid fthumbt(mFthumb, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidThumb.Add(fthumbt);

		// Convert sc faces to a meats meshes.
		int topBottomBlend = 2;
		AuPoint blendUpVec;
		if(laterality == Left)
			blendUpVec = AuPoint(-1,0,0).RotateZ(45*DEG2RAD);
		else
			blendUpVec = AuPoint(1,0,0).RotateZ(-45*DEG2RAD);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;

		// Meshes are added in pieces for part identification.
		// Arm mesh.
		float triangleSize = .75f/12.f * d_ClothingTessFactor;
		AuScFaceList faces;
		solidArm.Faces(faces, triangleSize);
		mymesh.FromFaces(faces, AuMeatTri::Arm);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, upperArmLen + foreArmLen, topBottomBlend, blendAngle,
				blendWid);

		// Add arm features.
		//DHH dna.GetArmFeaturesClothed(mymesh, mNeck, *this);

		// Hand mesh.
		AuMeatMesh handmesh;
		triangleSize = .6f/12.f * d_ClothingTessFactor;
		faces.Clear();
		solidHand.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Hand);

		// Fingers mesh.
		triangleSize = .5f/12.f * d_ClothingTessFactor;
		faces.Clear();
		solidFingers.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Finger);

		// Thumb mesh.
		faces.Clear();
		solidThumb.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Thumb);
		handmesh.SmoothNormals();	// Clothing wants all vertexes smoothed.
		handmesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, handLen, topBottomBlend, blendAngle, blendWid);
		mymesh.Append(handmesh);

		// Done adding polygons to the arm. Set the bone to use by it's vertices.
		// Shoulder to elbow is bone 0.
		mymesh.SetBonesRangeZ(-upperArmLen, deltoidRad, firstBone,1., 0,0., 0,0., 0,0.);
		// Elbow to wrist is bone 1.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen), -upperArmLen, firstBone+1,1., 0,0., 0,0., 0,0.);
		// Wrist to middle of hand is bone 2.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen/2), -(upperArmLen+foreArmLen),
				firstBone+2,1., 0,0., 0,0., 0,0.);
		// Middle of hand to middle of fingers 3.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen+.1), -(upperArmLen+foreArmLen+handLen*.5f),
				firstBone+3,1., 0,0., 0,0., 0,0.);
		// Middle of fingers to tip of fingers is bone 3.
		mymesh.SetBonesRangeZ(-(upperArmLen+foreArmLen+handLen+handLen*.5f), -(upperArmLen+foreArmLen+handLen*.75f),
				firstBone+4,1., 0,0., 0,0., 0,0.);
	}

	AuMatrix4x4 m;
	m.Translate(dna.ArmLocation(m_Which));
	mymesh.Transform(m);
	mesh.Append(mymesh);
}

void AuChromoArm::HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles) const
{
	const float follicleSpacing = lockWid * .5f;

	// Get my hair genes.
	bool hasHair = false;
	const AuGeneHair* hairDNA[AuFolliclePoint::ROLE_Count];
	for(int r=0; r<AuFolliclePoint::ROLE_Count; r++)
	{
		hairDNA[r] = HairDNA((AuFolliclePoint::ROLE)r);
		if(hairDNA[r])
			hasHair = true;
	}
	if(!hasHair)
		return;	// I have no hair.

	// Create the mesh of the arm.
	AuMeatMesh mymesh;
	double shoulderRad = ShoulderRadiusMM() * FeetPerMM();
	double wristRad = WristRadiusMM() * FeetPerMM();
	double elbowRad = ElbowRadiusMM() * FeetPerMM();
	double foreArmLen = ForeArmLengthMM() * FeetPerMM();
	double upperArmLen = UpperArmLengthMM() * FeetPerMM();
	double handLen = HandLengthMM() * FeetPerMM();
	LATERALITY laterality = Laterality();
	{
		double wristThick = wristRad * .6;
		double elbowThick = elbowRad * .9;
		double shoulderX = shoulderRad * .9;
		double handWid = wristRad * 1.5;
		double handHt = handWid * .6;
		double palmLen = handLen/2;
		double palmRad = palmLen/2;
		double fingerLenMid = palmLen;
		double fingerLenIndex = fingerLenMid * .9;
		double fingerLenRing = fingerLenMid * .9;
		if(dna.HasMaleGenitals())
			fingerLenRing *= 1.05;
		double fingerLenPinky = fingerLenMid * .75;
		double fingerLenThumb = fingerLenMid * .75;
		double fingerRad = wristRad * .35;
		double fingerRadTip = fingerRad * .7;
		double fingerSpacing = fingerRad * 1.7;
		double deltoidX = shoulderX * 1.1;
		double deltoidRad = shoulderRad * 1.1;
		double deltoidHt = deltoidRad;

		// Build the arm down to the wrist, excluding the hand.
		AuSolid solidArm;
		AuMatrix4x4 ms;
		if(laterality == Left)
			ms.Translate(-(deltoidX-shoulderX),deltoidRad-shoulderRad,0);
		else
			ms.Translate(deltoidX-shoulderX,deltoidRad-shoulderRad,0);
		AuSolidEllipsoid deltoid(ms, shoulderX, deltoidRad, deltoidHt);
		solidArm.Add(deltoid);
		ms.LoadIdentity();
		AuSolidDomoid shoulder(ms, shoulderX, shoulderRad, shoulderX, false);
		solidArm.Add(shoulder);
		ms.RotateX(PI);
		AuSolidConoid upperArm(ms, upperArmLen, shoulderX, shoulderRad, elbowThick, elbowRad, false, false);
		solidArm.Add(upperArm);
		ms.Translate(0,0,upperArmLen);
		AuSolidConoid foreArm(ms, foreArmLen, elbowThick, elbowRad, wristThick, wristRad, false, false);
		solidArm.Add(foreArm);
		ms.Translate(0,0,foreArmLen);
		AuSolidDomoid wrist(ms, wristThick, wristRad, wristThick, false);
		solidArm.Add(wrist);

		// Hand
		AuSolid solidHand;
		ms.Translate(0,0,handLen/2 - palmRad);	// Center of palm
		if(laterality == Left)
			ms.RotateZ(-PI/2);
		else
			ms.RotateZ(PI/2);
		ms.RotateX(PI/2);

		AuMatrix4x4 mfb(ms);	// Base transform for fingers.
		mfb.RotateX(-PI/2);		// Orient to palm.

		ms.Translate(0,0,-handHt * .25);
		AuSolidDomoid palm(ms, handWid, palmRad, handHt, true);
		solidHand.Add(palm);

		// Fingers
		AuSolid solidFingers;
		AuMatrix4x4 mFmid(mfb);
		mFmid.Translate(0,0,palmRad);
		AuMatrix4x4 mFmidp(mFmid);
		mFmidp.RotateX(PI);
		AuSolidDomoid fmidp(mFmidp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fmidp);
		AuSolidConoid fmid(mFmid, fingerLenMid - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
		solidFingers.Add(fmid);
		mFmid.Translate(0,0,fingerLenMid - fingerRadTip);
		AuSolidDomoid fmidt(mFmid, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fmidt);

		AuMatrix4x4 mFring(mfb);
		if(laterality == Left)
			mFring.Translate(-fingerSpacing,0,palmRad*.9);
		else
			mFring.Translate(fingerSpacing,0,palmRad*.9);
		AuMatrix4x4 mFringp(mFring);
		mFringp.RotateX(PI);
		AuSolidDomoid fringp(mFringp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fringp);
		AuSolidConoid fring(mFring, fingerLenRing - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false, false);
		solidFingers.Add(fring);
		mFring.Translate(0,0,fingerLenRing - fingerRadTip);
		AuSolidDomoid fringt(mFring, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fringt);

		AuMatrix4x4 mFpinky(mfb);
		if(laterality == Left)
			mFpinky.Translate(-fingerSpacing*2,0,palmRad*.7);
		else
			mFpinky.Translate(fingerSpacing*2,0,palmRad*.7);
		AuMatrix4x4 mFpinkyp(mFpinky);
		mFpinkyp.RotateX(PI);
		AuSolidDomoid fpinkyp(mFpinkyp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(fpinkyp);
		AuSolidConoid fpinky(mFpinky, fingerLenPinky - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidFingers.Add(fpinky);
		mFpinky.Translate(0,0,fingerLenPinky - fingerRadTip);
		AuSolidDomoid fpinkyt(mFpinky, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(fpinkyt);

		AuMatrix4x4 mFindex(mfb);
		if(laterality == Left)
			mFindex.Translate(fingerSpacing,0,palmRad*.9);
		else
			mFindex.Translate(-fingerSpacing,0,palmRad*.9);
		AuMatrix4x4 mFindexp(mFindex);
		mFindexp.RotateX(PI);
		AuSolidDomoid findexp(mFindexp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidFingers.Add(findexp);
		AuSolidConoid findex(mFindex, fingerLenIndex - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidFingers.Add(findex);
		mFindex.Translate(0,0,fingerLenIndex - fingerRadTip);
		AuSolidDomoid findext(mFindex, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidFingers.Add(findext);

		AuSolid solidThumb;
		AuMatrix4x4 mFthumb(mfb);
		if(laterality == Left)
		{
			mFthumb.Translate(fingerSpacing*2,0,-palmRad*.25);
			mFthumb.RotateY(20*DEG2RAD);
			mFthumb.RotateX(-30*DEG2RAD);
		}
		else
		{
			mFthumb.Translate(-fingerSpacing*2,0,-palmRad*.25);
			mFthumb.RotateY(-20*DEG2RAD);
			mFthumb.RotateX(-30*DEG2RAD);
		}
		AuMatrix4x4 mFthumbp(mFthumb);
		mFthumbp.RotateX(PI);
		AuSolidDomoid fthumbp(mFthumbp, fingerRad, fingerRad, fingerRad, false);	// Connects finger to palm and caps finger.
		solidThumb.Add(fthumbp);
		AuSolidConoid fthumb(mFthumb, fingerLenThumb - fingerRadTip, fingerRad, fingerRad, fingerRadTip, fingerRadTip, false,
				false);
		solidThumb.Add(fthumb);
		mFthumb.Translate(0,0,fingerLenThumb - fingerRadTip);
		AuSolidDomoid fthumbt(mFthumb, fingerRadTip, fingerRadTip, fingerRadTip, false);	// Finger tip.
		solidThumb.Add(fthumbt);

		// Convert sc faces to a meats meshes.
		// Meshes are added in pieces for part identification.
		// Arm mesh.
		float triangleSize = follicleSpacing;
		AuScFaceList faces;
		solidArm.Faces(faces, triangleSize);
		mymesh.FromFaces(faces, AuMeatTri::Arm);
		mymesh.SmoothNormals();

		// Add arm features.
		//DHH dna.GetArmFeaturesClothed(mymesh, mNeck, *this);

		// Hand mesh.
		AuMeatMesh handmesh;
		triangleSize = .6f/12.f * d_ClothingTessFactor;
		faces.Clear();
		solidHand.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Hand);

		// Fingers mesh.
		triangleSize = .5f/12.f * d_ClothingTessFactor;
		faces.Clear();
		solidFingers.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Finger);

		// Thumb mesh.
		faces.Clear();
		solidThumb.Faces(faces, triangleSize);
		handmesh.FromFaces(faces, AuMeatTri::Thumb);
		handmesh.SmoothNormals();	// Clothing wants all vertexes smoothed.
		mymesh.Append(handmesh);
	}

	AuPointNArray myfol;
	mymesh.SurfacePoints(myfol);

	// Remove follicles from face, eyes and ears.
	QMap<AuPoint, double> hairEx;	// Exclusion locations with radius.
	//DHH dna.ArmHairExclusions(hairEx, mSkull, *this);

	for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
	{
		// Get information about the hair follicle.
		double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
		//double zang = (*fi).m_N.PolarIncline();
		//double chestang = 15*DEG2RAD;			// XY angle forward of which hair is chest.
		//double rearflankang = 45*DEG2RAD;		// XY angle behind which hair is rear flank.
		double backangf = 0*DEG2RAD;			// XY angle behind which hair is back.
		double backangb = 180*DEG2RAD;			// XY angle behind which hair is back.

		// Decide whether the follicle should stay or go.
		AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
		if((*fi).z > -shoulderRad)
		{
			// Shoulder
			role = AuFolliclePoint::Shoulders;
		}
		else if((*fi).z < -(upperArmLen + foreArmLen))
		{
			// Hands
			if(laterality == Left && xyang > backangf && xyang < backangb)
				role = AuFolliclePoint::Hands;
			else if(laterality != Left && xyang < backangf && xyang > -backangb)
				role = AuFolliclePoint::Hands;
			else
				role = AuFolliclePoint::Palms;
		}
		else if((*fi).z < -(upperArmLen + foreArmLen*.75))
		{
			// Wrists
			if(laterality == Left && xyang > backangf && xyang < backangb)
				role = AuFolliclePoint::WristBack;
			else if(laterality != Left && xyang < backangf && xyang > -backangb)
				role = AuFolliclePoint::WristBack;
			else
				role = AuFolliclePoint::WristUnder;
		}
		else
		{
			// Arm
			if(laterality == Left && xyang > backangf && xyang < backangb)
				role = AuFolliclePoint::ArmBack;
			else if(laterality != Left && xyang < backangf && xyang > -backangb)
				role = AuFolliclePoint::ArmBack;
			else
				role = AuFolliclePoint::ArmUnder;
		}

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		if(!hairDNA[role])
			continue;	// No hair there.

		/*
		// Check for exclusion by arm features.
		if(role != AuFolliclePoint::ROLE_Count && !hairEx.empty())
			for(QMap<AuPoint, double>::const_iterator xi = hairEx.cbegin(); xi != hairEx.cend(); xi++)
				if((*fi).Distance2(xi.key()) < (*xi)*(*xi))
				{
					role = AuFolliclePoint::ROLE_Count;
					break;
				}
		*/

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		const QColor& hairColor = hairDNA[role]->Color();
		float maxLen = hairDNA[role]->LengthMM() * FeetPerMM();
		AuFolliclePoint fp(0, role, *fi, hairColor, maxLen);
		follicles.append(fp);
	}
}

quint64 AuChromoArm::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::ArmBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::ArmUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Hands) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Palms) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Shoulders) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::WristBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::WristUnder);
}

AuPoint AuChromoArm::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	AuPoint boneLoc(dna.ArmLocation(m_Which));
	if(bone == 1)
		boneLoc.z -= UpperArmLengthMM() * FeetPerMM();											// Elbow
	else if(bone == 2)
		boneLoc.z -= (UpperArmLengthMM() + ForeArmLengthMM()) * FeetPerMM();					// Hand
	else if(bone == 3)
		boneLoc.z -= (UpperArmLengthMM() + ForeArmLengthMM() + HandLengthMM()*.5f) * FeetPerMM();	// Fingers
	else if(bone == 4)
		boneLoc.z -= (UpperArmLengthMM() + ForeArmLengthMM() + HandLengthMM()*.75f) * FeetPerMM();	// Finger Tips
	return boneLoc;
}

int AuChromoArm::ListWidgetOrder() const
{
	return 400 + m_Which.Index();
}

QString AuChromoArm::ListWidgetText() const
{
	QString note(LateralityName(Laterality()));
	if(Walks())
		note += QObject::tr(", Walks");
	if(Runs())
		note += QObject::tr(", Runs");
	if(Jumps())
		note += QObject::tr(", Jumps");
	return QObject::tr("Arm %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(note);
}

QWidget* AuChromoArm::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageArm* pp = new AuChromoPageArm(parent);
	return pp;
}

bool AuChromoArm::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try next previous of same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);										// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoArm::Laterality(const AuChromoWhich& which)
{
	return which.Index() & 1 ? Right : Left;
}

bool AuChromoArm::StandsFlat() const
{
	return true;	//DHH
}

void AuChromoArm::CalcStandAngles(const AuPose& pose, float strength, float& shoulderX, float& shoulderY, float& shoulderZ,
		float& elbowX, float& elbowZ, float& wristX, float& wristY, float& fingerY, float& fingerTipY) const
{
	// Init return values.
	shoulderX = 0;
	shoulderY = -10*DEG2RADf;
	shoulderZ = 0;
	elbowX = 0;
	elbowZ = 0;
	wristX = 0;
	wristY = 0;
	fingerY = 0;
	fingerTipY = 0;

	const float upperarmlen = UpperArmLengthMM() * FeetPerMM();
	const float forearmlen = ForeArmLengthMM() * FeetPerMM();
	const float handlen = HandLengthMM() * FeetPerMM();
	const float standTone = (float)m_StandTone / 100.f * strength;

	bool touchGround = false;
	switch(pose.Pose())
	{
	case AuPose::CrouchCreep:
	case AuPose::ProneCreep:
	case AuPose::SitOnGroundCreep:
	case AuPose::StandWalk:
		touchGround = Walks();
		break;
	case AuPose::Jump:
		touchGround = Jumps();
		break;
	case AuPose::StandRun:
		touchGround = Runs();
		break;
	case AuPose::Beckon:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::EyesOnly:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Glide:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hover:
	case AuPose::Hug:
	case AuPose::Init:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Prone:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Point:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Prone:
	case AuPose::Psionic:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::SitOnGround:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::Strike:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::Throw:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::TreadWater:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
		break;
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}

	bool standFlat = StandsFlat();
	float shinlen = 0;
	if(standFlat || !touchGround)
		shinlen = forearmlen;
	else
		shinlen = forearmlen + handlen;
	float shoulderHt = upperarmlen + shinlen;
	if(touchGround && standTone > 0)
		shoulderHt *= standTone;			// Arms are mostly extended when standing.
	else
		shoulderHt *= .95f;					// Arms are mostly extended when standing.

	// Using the Law of Cosines...
	float shin2 = shinlen*shinlen;
	float shoulderHt2 = shoulderHt*shoulderHt;
	float upperarm2 = upperarmlen*upperarmlen;
	shoulderX = (shin2 - upperarm2 - shoulderHt2) / (-2.f * upperarmlen * shoulderHt);
	shoulderX = acosf(shoulderX);
	elbowX = (shoulderHt2 - upperarm2 - shin2) / (-2.f * upperarmlen * shinlen);
	elbowX = acosf(elbowX);

	// Calculate wrist bend needed.
	if(standFlat && touchGround)
	{
		//wristX = (elbowX + shoulderX) - PIf/2.f;
		wristY = -PIf/2 - shoulderY;
		fingerY = 10*DEG2RADf;
	}
	else
	{
		wristX = 2.f*DEG2RADf;
		fingerY = 20*DEG2RADf;
		fingerTipY = 20*DEG2RADf;
	}
	shoulderX = -shoulderX;
	elbowX = PIf - elbowX;
}

void AuChromoArm::AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
		float currentHeadTilt, HANDEDNESS hand, bool bendToHoldObject,
		float& shoulderX, float& shoulderY, float& shoulderZ, float& elbowX, float& elbowZ, float& wristX, float& wristY,
		float& wristZ, float& fingerY, float& fingerTipY) const
{
	shoulderX = 0;
	shoulderY = 0;
	shoulderZ = 0;
	elbowX = 0;
	elbowZ = 0;
	wristX = 0;
	wristY = 0;
	wristZ = 0;
	fingerY = 0;
	fingerTipY = 0;

	LATERALITY laterality = Laterality();
	switch(pose.Pose())
	{
	// Special
	case AuPose::Init:
	case AuPose::Spawn:
		break;

	// Head Only
	case AuPose::EyesOnly:
		break;

	// Horizontal Poses
	case AuPose::Prone:
	case AuPose::ProneCreep:
		CalcStandAngles(pose, .5f, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
		shoulderX = 15*DEG2RADf;
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		else
		{
			elbowX = 165*DEG2RADf;
		}
		break;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		shoulderY = -45*DEG2RADf;
		shoulderZ = -90*DEG2RADf;
		elbowX = 125*DEG2RADf;
		wristY = 5*DEG2RADf;
		fingerY = 15*DEG2RADf;
		fingerTipY = 10*DEG2RADf;
		break;

	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		if(laterality == Left)
		{
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 20*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 15*DEG2RADf;
		}
		break;

	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		if(laterality == Right)
		{
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 20*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 15*DEG2RADf;
		}
		break;

	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
	case AuPose::Patient_Prone:			// Prone patient faces up.
		if(laterality == Right)
		{
			shoulderX = -5*DEG2RADf;
			shoulderY = -15*DEG2RADf;
			shoulderZ = 45*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = -5*DEG2RADf;
			shoulderY = -15*DEG2RADf;
			shoulderZ = 35*DEG2RADf;
			elbowX = 10*DEG2RADf;
			elbowZ = -25*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		break;

	// Angles for dead poses are slightly tighter than writhe poses, and inanimate.
	case AuPose::DeadFaceDown:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			shoulderY = -125*DEG2RADf;
			shoulderZ = -90*DEG2RADf;
			elbowX = 25*DEG2RADf;
			wristY = 5*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		else
		{
			shoulderY = -45*DEG2RADf;
			shoulderZ = -90*DEG2RADf;
			elbowX = 100*DEG2RADf;
			wristY = 5*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceLeft:
		if(laterality == Left)
		{
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 15*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceRight:
		if(laterality == Right)
		{
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceUp:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			shoulderX = -15*DEG2RADf;
			shoulderY = -35*DEG2RADf;
			shoulderZ = 45*DEG2RADf;
			elbowX = 15*DEG2RADf;
			elbowZ = -15*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		else
		{
			shoulderX = -15*DEG2RADf;
			shoulderY = -15*DEG2RADf;
			shoulderZ = 35*DEG2RADf;
			elbowX = 15*DEG2RADf;
			elbowZ = -25*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 30*DEG2RADf;
			fingerTipY = 30*DEG2RADf;
		}
		break;

	// Angles for writhe/dying poses are based on dead poses, only animated to show signs of life.
	case AuPose::WritheFaceDown:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			AuDice dice(id.GetID());
			float x = when.CurrentDayCycle() * 25000 + dice.Roll(1,10000);
			float p = AuNoiseF::CoherentNoise(id.GetID() % 12, x);
			if(p < 0)
				p = 0;
			shoulderY = -125*DEG2RADf + p*40*DEG2RADf;
			shoulderZ = -90*DEG2RADf;
			elbowX = 25*DEG2RADf + p*90*DEG2RADf;
			wristY = 5*DEG2RADf;
			fingerY = 10*DEG2RADf;
			fingerTipY = 10*DEG2RADf;
		}
		else
		{
			shoulderY = -45*DEG2RADf;
			shoulderZ = -90*DEG2RADf;
			elbowX = 100*DEG2RADf;
			wristY = 5*DEG2RADf;
			fingerY = 10*DEG2RADf;
			fingerTipY = 10*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceLeft:
		if(laterality == Left)
		{
			AuDice dice(id.GetID());
			float x = when.CurrentDayCycle() * 25000 + dice.Roll(1,10000);
			float p = AuNoiseF::CoherentNoise(id.GetID() % 12, x);
			if(p < 0)
				p = 0;
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf - p*120*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf - p*70*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 20*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 15*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceRight:
		if(laterality == Right)
		{
			AuDice dice(id.GetID());
			float x = when.CurrentDayCycle() * 25000 + dice.Roll(1,10000);
			float p = AuNoiseF::CoherentNoise(id.GetID() % 12, x);
			if(p < 0)
				p = 0;
			shoulderX = 25*DEG2RADf;
			shoulderY = 15*DEG2RADf - p*120*DEG2RADf;
			shoulderZ = 30*DEG2RADf;
			elbowX = 80*DEG2RADf - p*70*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 20*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = 30*DEG2RADf;
			shoulderY = 3*DEG2RADf;
			shoulderZ = 10*DEG2RADf;
			elbowX = 20*DEG2RADf;
			elbowZ = 0;
			wristX = 0;
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 15*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceUp:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			shoulderX = -15*DEG2RADf;
			shoulderY = -35*DEG2RADf;
			shoulderZ = 45*DEG2RADf;
			elbowX = 100*DEG2RADf;
			elbowZ = 15*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		else
		{
			shoulderX = -15*DEG2RADf;
			shoulderY = -10*DEG2RADf;
			shoulderZ = 35*DEG2RADf;
			elbowX = 15*DEG2RADf;
			elbowZ = -25*DEG2RADf;
			wristX = 0;
			wristY = 10*DEG2RADf;
			fingerY = 20*DEG2RADf;
			fingerTipY = 20*DEG2RADf;
		}
		break;

	case AuPose::MateOnBottom:
	case AuPose::ReceptiveMate:
		shoulderX = 5*DEG2RADf;
		shoulderY = -15*DEG2RADf;
		shoulderZ = -25*DEG2RADf;
		elbowX = 30*DEG2RADf;
		elbowZ = -5*DEG2RADf;
		wristX = 0;
		wristY = 10*DEG2RADf;
		fingerY = 20*DEG2RADf;
		fingerTipY = 20*DEG2RADf;
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		break;

	case AuPose::MateOnTop:
		shoulderX = -10*DEG2RADf;
		shoulderY = -15*DEG2RADf;
		shoulderZ = -15*DEG2RADf;
		elbowX = 60*DEG2RADf;
		elbowZ = 20*DEG2RADf;
		wristX = 0;
		wristY = -60*DEG2RADf;
		fingerY = 10*DEG2RADf;
		fingerTipY = 30*DEG2RADf;
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		break;

	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		break;

	case AuPose::Swim:
	case AuPose::SwimFast:
		{
			float cycleOffset = (id.GetID()%360)*DEG2RADf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			shoulderX = 80*DEG2RADf;
			shoulderY = -90*DEG2RADf + (2*perang);
			elbowX = 80*DEG2RADf + (60*perang);
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 10*DEG2RADf;
			if(bendToHoldObject)
			{
				shoulderX = 90*DEG2RADf;
				wristZ = -90*DEG2RADf;
				wristX = 90*DEG2RADf - elbowX;
				fingerY = PIf/2;
				fingerTipY = PIf/2;
			}
		}
		break;
	case AuPose::TreadWater:
		{
			float cycleOffset = (id.GetID()%360)*DEG2RADf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			shoulderX = -10*DEG2RADf;
			shoulderY = -90*DEG2RADf + (2*perang);
			elbowX = 80*DEG2RADf + (60*perang);
			wristY = 5*DEG2RADf;
			fingerY = 15*DEG2RADf;
			fingerTipY = 10*DEG2RADf;
			if(bendToHoldObject)
			{
				shoulderX = 0;
				wristZ = -90*DEG2RADf;
				wristX = 90*DEG2RADf - elbowX;
				fingerY = PIf/2;
				fingerTipY = PIf/2;
			}
		}
		break;
	
	// Sitting Poses
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Post_Sit:
		CalcStandAngles(pose, .5f, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		else
		{
			elbowX += 50*DEG2RADf;
			shoulderX += 10*DEG2RADf;
			shoulderZ -= 5*DEG2RADf;
		}
		break;

	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
		CalcStandAngles(pose, .5f, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		else
		{
			elbowX += 25*DEG2RADf;
			shoulderZ -= 10*DEG2RADf;
		}
		break;

	case AuPose::Straddle:
		CalcStandAngles(pose, .5f, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		break;

	// Crouched Poses
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
		CalcStandAngles(pose, .5f, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
		if(bendToHoldObject)
		{
			elbowX = -shoulderX + currentHeadTilt + PIf/2;
			wristY = -2*DEG2RADf;
			fingerY = PIf/2;
			fingerTipY = PIf/2;
		}
		break;

	// Standing
	case AuPose::Stand:
	case AuPose::Post_Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		{
			CalcStandAngles(pose, 1, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);
			if(bendToHoldObject)
			{
				elbowX = -shoulderX + currentHeadTilt + PIf/2;
				wristY = -2*DEG2RADf;
				fingerY = PIf/2;
				fingerTipY = PIf/2;
			}
		}
		break;

	case AuPose::StandRun:
		{
			CalcStandAngles(pose, 1, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);

			float perang = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality != Left)
				perang += PIf;
			float persin = pose.Rhythm(bodyHz, perang, when);

			shoulderX += (10. - 20.*persin)*DEG2RADf;
			if(bendToHoldObject)
			{
				elbowX = -shoulderX + currentHeadTilt + PIf/2;
				wristY = -2*DEG2RADf;
				fingerY = PIf/2;
				fingerTipY = PIf/2;
			}
			else
				elbowX -= (elbowX + elbowX*persin)/2.;
		}
		break;

	case AuPose::StandWalk:
		{
			CalcStandAngles(pose, 1, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, fingerY, fingerTipY);

			float perang = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality != Left)
				perang += PIf;
			float persin = pose.Rhythm(bodyHz, perang, when);

			shoulderX += (5. - 10.*persin)*DEG2RADf;
			if(bendToHoldObject)
			{
				elbowX = -shoulderX + currentHeadTilt + PIf/2;
				wristY = -2*DEG2RADf;
				fingerY = PIf/2;
				fingerTipY = PIf/2;
			}
			else
				elbowX -= (elbowX + elbowX*persin)/2.;
		}
		break;

	// Misc
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		break;

	// Mouth Poses
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}

	//DHH
	/*
	shoulderX = -20*DEG2RADf;
	shoulderY = -10*DEG2RADf;
	shoulderZ = -5*DEG2RADf;

	elbowX = 40*DEG2RADf;
	//elbowX = when.SecCycle() * (90*DEG2RADf);	//DHH

	wristX = 0*DEG2RADf;
	wristY = 20*DEG2RADf;
	//wristY = when.SecCycle() * (80*DEG2RADf);	//DHH

	fingerY = 30*DEG2RADf;
	//fingerY = when.SecCycle() * (90*DEG2RADf);	//DHH
	*/

	// Reverse Z and Y angles for left side.
	if(laterality == AuChromosome::Left)
	{
		shoulderZ = -shoulderZ;
		shoulderY = -shoulderY;
		elbowZ = -elbowZ;
		wristY = -wristY;
		wristZ = -wristZ;
		fingerY = -fingerY;
		fingerTipY = -fingerTipY;
	}
}

void AuChromoArm::SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
{
	for(QList<AuAnimalBone>::const_iterator bi = bones.cbegin(); bi != bones.cend(); bi++)
	{
		AuPoint bloc((*bi).PosedLoc());
		min = min.Min(bloc);
		max = max.Max(bloc);
	}
}

bool AuChromoArm::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
{
	double shoulderRad = m_ShoulderRadMM * FeetPerMM();
	double upperZ = m_UpperArmLengthMM * FeetPerMM();
	double elbowRad = m_ElbowRadMM * FeetPerMM();
	double foreArmZ = m_ForeArmLengthMM * FeetPerMM();
	double wristRad = m_WristRadMM * FeetPerMM();
	double handZ = m_HandLengthMM * FeetPerMM();
	double handRad = handZ / 4;

	// Use bone matrices to put the solids at their posed locations.
	// Upper arm
	AuMatrix4x4 mu(bones[0].Matrix());
	mu.Translate(bones[0].UnposedLoc());
	mu.Translate(0,0,-upperZ);	// Move down to the elbow to draw the solid.
	AuSolidConoid upperArm(mu, upperZ, elbowRad, elbowRad, shoulderRad, shoulderRad, true, true);

	// Fore arm
	AuMatrix4x4 mf(bones[1].Matrix());
	mf.Translate(bones[1].UnposedLoc());
	mf.Translate(0,0,-foreArmZ);	// Move down to the wrist to draw the solid.
	AuSolidConoid foreArm(mf, foreArmZ, wristRad, wristRad, elbowRad, elbowRad, true, true);

	// Hand
	AuMatrix4x4 mh(bones[2].Matrix());
	mh.Translate(bones[2].UnposedLoc());
	mh.Translate(0,0,-handZ/2);	// Move down to the knuckles to draw the solid.
	AuSolidConoid hand(mh, handZ/2, handRad, handRad, wristRad, wristRad, true, true);

	// Fingers
	AuMatrix4x4 mg(bones[3].Matrix());
	mg.Translate(bones[3].UnposedLoc());
	mg.Translate(0,0,-handZ/4);	// Move down to the fingers to draw the solid.
	AuSolidConoid fingers(mg, handZ/4, handRad, handRad, handRad, handRad, true, true);

	// Finger Tips
	AuMatrix4x4 mt(bones[4].Matrix());
	mt.Translate(bones[4].UnposedLoc());
	mt.Translate(0,0,-handZ/4);	// Move down to the finger tips to draw the solid.
	AuSolidConoid fingerTips(mt, handZ/4, handRad, handRad, handRad, handRad, true, true);

	bool sighted = upperArm.SightTarget(eyeLoc, sightedLoc);
	if(foreArm.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	if(hand.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	if(fingers.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	if(fingerTips.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	return sighted;
}

float AuChromoArm::Volume() const
{
	float shoulderRad = m_ShoulderRadMM * FeetPerMM();
	float upperZ = m_UpperArmLengthMM * FeetPerMM();
	float elbowRad = m_ElbowRadMM * FeetPerMM();
	float foreArmZ = m_ForeArmLengthMM * FeetPerMM();
	float wristRad = m_WristRadMM * FeetPerMM();
	float handZ = m_HandLengthMM * FeetPerMM();

	float v = 0;
	// Shoulder is a sphere
	// V = 4/3 * PI * r^3
	v += 4.f/3.f * PIf * shoulderRad * shoulderRad * shoulderRad;

	// Upper arm.
	if(m_ShoulderRadMM == m_ElbowRadMM)
	{
		// Upper arm is a cylinder.
		// V = PI * r^2 * h
		v += PIf * shoulderRad * shoulderRad * upperZ;
	}
	else
	{
		// Upper arm is a truncated cone.
		float h;	// Height extended to full cone.
		float r;
		if(shoulderRad > elbowRad)
		{
			float s = (shoulderRad - elbowRad) / upperZ;	// Slope of the side
			r = shoulderRad;
			h = r / s;
		}
		else
		{
			float s = (elbowRad - shoulderRad) / upperZ;	// Slope of the side
			r = elbowRad;
			h = r / s;
		}
		// Volume of cone.
		// V = PI * r^2 * h / 3
		float vc = PIf * r * r * h / 3;				// Volume of full cone.
		float vt = PIf * r * r * (h-upperZ) / 3;	// Volume of tip cone.
		v += (vc - vt);								// Volume of truncated cone is full cone minus the tip.
	}

	// Fore arm.
	if(m_WristRadMM == m_ElbowRadMM)
	{
		// Fore arm is a cylinder.
		// V = PI * r^2 * h
		v += PIf * elbowRad * elbowRad * foreArmZ;
	}
	else
	{
		// Fore arm is a truncated cone.
		float h;	// Height extended to full cone.
		float r;
		if(elbowRad > wristRad)
		{
			float s = (elbowRad - wristRad) / foreArmZ;	// Slope of the side
			r = elbowRad;
			h = r / s;
		}
		else
		{
			float s = (wristRad - elbowRad) / foreArmZ;	// Slope of the side
			r = wristRad;
			h = r / s;
		}
		// Volume of cone.
		// V = PI * r^2 * h / 3
		float vc = PIf * r * r * h / 3;				// Volume of full cone.
		float vt = PIf * r * r * (h-foreArmZ) / 3;	// Volume of tip cone.
		v += (vc - vt);								// Volume of truncated cone is full cone minus the tip.
	}

	// Hand is calculated as a half sphere.
	// V = 4/3 * PI * r^3
	float handRad = handZ / 2;
	float vh = 4.f/3.f * PIf * handRad * handRad * handRad / 2;
	v += vh;

	return v;
}

bool AuChromoArm::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoArm* c = (const AuChromoArm*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	return true;
}

AuChromosome* AuChromoArm::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoArm* c = dynamic_cast<const AuChromoArm*>(g);
	if(!c)
		return 0;
	AuChromoArm* comb = new AuChromoArm(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoLeg::AuChromoLeg(const AuChromoWhich& which,
		quint16 hipToSpineMM,
		quint16 thighLenMM,
		quint16 calfLenMM,
		quint16 buttRadMM,
		quint16 hipRadXMM,
		quint16 kneeRadXMM,
		quint16 ankleLenMM,
		quint16 ankleRadMM,
		quint16 footLenMM,
		quint16 footWidMM,
		quint8 standTone)
{
	m_AnkleLenMM = ankleLenMM;
	m_AnkleRadMM = ankleRadMM;
	m_ButtRadMM = buttRadMM;
	m_CalfLenMM = calfLenMM;
	m_FootLenMM = footLenMM;
	m_FootWidMM = footWidMM;
	m_HipRadXMM = hipRadXMM;
	m_HipToSpineMM = hipToSpineMM;
	m_KneeRadXMM = kneeRadXMM;
	m_StandTone = standTone;
	m_ThighLenMM = thighLenMM;
	m_Which = which;
}

AuChromoLeg::AuChromoLeg()
{
	m_AnkleLenMM = 40;
	m_AnkleRadMM = 20;
	m_ButtRadMM = 40;
	m_CalfLenMM = 100;
	m_FootLenMM = 100;
	m_FootWidMM = 40;
	m_HipRadXMM = 30;
	m_HipToSpineMM = 40;
	m_KneeRadXMM = 20;
	m_StandTone = 0;
	m_ThighLenMM = 100;
}

AuChromoLeg::AuChromoLeg(const AuChromoLeg& c) : AuChromosomeWithGenes(c)
{
	m_AnkleLenMM = c.m_AnkleLenMM;
	m_AnkleRadMM = c.m_AnkleRadMM;
	m_ButtRadMM = c.m_ButtRadMM;
	m_CalfLenMM = c.m_CalfLenMM;
	m_FootLenMM = c.m_FootLenMM;
	m_FootWidMM = c.m_FootWidMM;
	m_HipRadXMM = c.m_HipRadXMM;
	m_HipToSpineMM = c.m_HipToSpineMM;
	m_KneeRadXMM = c.m_KneeRadXMM;
	m_StandTone = c.m_StandTone;
	m_ThighLenMM = c.m_ThighLenMM;
	m_Which = c.m_Which;
}

AuChromoLeg::AuChromoLeg(const AuChromoLeg& c1, const AuChromoLeg& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoLeg which mismatch" << c1.m_Which << c2.m_Which;
	m_AnkleLenMM = CombineRange(c1.m_AnkleLenMM, c2.m_AnkleLenMM, dice);
	dice.Random();
	m_AnkleRadMM = CombineRange(c1.m_AnkleRadMM, c2.m_AnkleRadMM, dice);
	dice.Random();
	m_ButtRadMM = CombineRange(c1.m_ButtRadMM, c2.m_ButtRadMM, dice);
	dice.Random();
	m_CalfLenMM = CombineRange(c1.m_CalfLenMM, c2.m_CalfLenMM, dice);
	dice.Random();
	m_FootLenMM = CombineRange(c1.m_FootLenMM, c2.m_FootLenMM, dice);
	dice.Random();
	m_FootWidMM = CombineRange(c1.m_FootWidMM, c2.m_FootWidMM, dice);
	dice.Random();
	m_HipRadXMM = CombineRange(c1.m_HipRadXMM, c2.m_HipRadXMM, dice);
	dice.Random();
	m_HipToSpineMM = CombineRange(c1.m_HipToSpineMM, c2.m_HipToSpineMM, dice);
	dice.Random();
	m_KneeRadXMM = CombineRange(c1.m_KneeRadXMM, c2.m_KneeRadXMM, dice);
	dice.Random();
	m_StandTone = CombineRange(c1.m_StandTone, c2.m_StandTone, dice);
	dice.Random();
	m_ThighLenMM = CombineRange(c1.m_ThighLenMM, c2.m_ThighLenMM, dice);
	m_Which = c1.m_Which;
}

void AuChromoLeg::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoLeg& c = dynamic_cast<const AuChromoLeg&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_AnkleLenMM = c.m_AnkleLenMM;
		m_AnkleRadMM = c.m_AnkleRadMM;
		m_ButtRadMM = c.m_ButtRadMM;
		m_CalfLenMM = c.m_CalfLenMM;
		m_FootLenMM = c.m_FootLenMM;
		m_FootWidMM = c.m_FootWidMM;
		m_HipRadXMM = c.m_HipRadXMM;
		m_HipToSpineMM = c.m_HipToSpineMM;
		m_KneeRadXMM = c.m_KneeRadXMM;
		m_ThighLenMM = c.m_ThighLenMM;
		m_StandTone = c.m_StandTone;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoLeg::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_AnkleLenMM;
	out << m_AnkleRadMM;
	out << m_ButtRadMM;
	out << m_CalfLenMM;
	out << m_FootLenMM;
	out << m_FootWidMM;
	out << m_HipRadXMM;
	out << m_HipToSpineMM;
	out << m_KneeRadXMM;
	out << m_StandTone;
	out << m_ThighLenMM;
	out << m_Which;
}

void AuChromoLeg::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_AnkleLenMM;
	in >> m_AnkleRadMM;
	in >> m_ButtRadMM;
	in >> m_CalfLenMM;
	in >> m_FootLenMM;
	in >> m_FootWidMM;
	in >> m_HipRadXMM;
	in >> m_HipToSpineMM;
	in >> m_KneeRadXMM;
	in >> m_StandTone;
	in >> m_ThighLenMM;
	in >> m_Which;
}

void AuChromoLeg::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_AnkleLenMM, d_DNAMaxMM);
	out << ToLogByte(m_AnkleRadMM, d_DNAMaxMM);
	out << ToLogByte(m_ButtRadMM, d_DNAMaxMM);
	out << ToLogByte(m_CalfLenMM, d_DNAMaxMM);
	out << ToLogByte(m_FootLenMM, d_DNAMaxMM);
	out << ToLogByte(m_FootWidMM, d_DNAMaxMM);
	out << ToLogByte(m_HipRadXMM, d_DNAMaxMM);
	out << ToLogByte(m_HipToSpineMM, d_DNAMaxMM);
	out << ToLogByte(m_KneeRadXMM, d_DNAMaxMM);
	out << m_StandTone;
	out << ToLogByte(m_ThighLenMM, d_DNAMaxMM);
	out << m_Which;
}

void AuChromoLeg::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_AnkleLenMM = ReadLogByte(in, d_DNAMaxMM);
	m_AnkleRadMM = ReadLogByte(in, d_DNAMaxMM);
	m_ButtRadMM = ReadLogByte(in, d_DNAMaxMM);
	m_CalfLenMM = ReadLogByte(in, d_DNAMaxMM);
	m_FootLenMM = ReadLogByte(in, d_DNAMaxMM);
	m_FootWidMM = ReadLogByte(in, d_DNAMaxMM);
	m_HipRadXMM = ReadLogByte(in, d_DNAMaxMM);
	m_HipToSpineMM = ReadLogByte(in, d_DNAMaxMM);
	m_KneeRadXMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_StandTone;
	m_ThighLenMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_Which;
}

void AuChromoLeg::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_AnkleLenMM = dice.RandomLogValue(m_AnkleLenMM, d_DNAMaxMM);
	m_AnkleRadMM = dice.RandomLogValue(m_AnkleRadMM, d_DNAMaxMM);
	m_ButtRadMM = dice.RandomLogValue(m_ButtRadMM, d_DNAMaxMM);
	m_CalfLenMM = dice.RandomLogValue(m_CalfLenMM, d_DNAMaxMM);
	m_FootLenMM = dice.RandomLogValue(m_FootLenMM, d_DNAMaxMM);
	m_FootWidMM = dice.RandomLogValue(m_FootWidMM, d_DNAMaxMM);
	m_HipRadXMM = dice.RandomLogValue(m_HipRadXMM, d_DNAMaxMM);
	m_HipToSpineMM = dice.RandomLogValue(m_HipToSpineMM, d_DNAMaxMM);
	m_KneeRadXMM = dice.RandomLogValue(m_KneeRadXMM, d_DNAMaxMM);
	m_ThighLenMM = dice.RandomLogValue(m_ThighLenMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoLeg::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalLeg(*this, dna, firstBone);
}

quint16 AuChromoLeg::KneeRadiusYMM() const
{
	//if(m_Style == Primate)
		//return KneeRadiusXMM() * .9f;
	//else
		return KneeRadiusXMM();
}

void AuChromoLeg::GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
		const AuClothing& clothes, GROWTHSTAGE stage) const
{
	Q_UNUSED(stage);
	if(clothes.BareThighCovered() && clothes.BareCalfCovered() && clothes.BareFootCovered())
		return;

	// Create the mesh of the leg.
	AuMeatMesh mymesh;
	{
		double ankleRadX = AnkleRadiusMM() * FeetPerMM();
		double ankleRadY = ankleRadX;
		double ankleHt = AnkleLengthMM() * FeetPerMM();
		double buttRad = ButtRadiusMM() * FeetPerMM();
		double calfLen = CalfLengthMM() * FeetPerMM();
		double footHt = FootHeightMM() * FeetPerMM();
		double footLen = FootLengthMM() * FeetPerMM();
		double footWid = FootWidthMM() * FeetPerMM();
		double kneeRadX = KneeRadiusXMM() * FeetPerMM();
		double kneeRadY = KneeRadiusYMM() * FeetPerMM();
		double thighLen = ThighLengthMM() * FeetPerMM();
		double hipRadX = m_HipRadXMM * FeetPerMM();

		double hipHt = buttRad * 1.2;
		double hipY = hipRadX;

		// Leg
		AuSolid solidLeg;
		AuMatrix4x4 ms;
		if(buttRad > hipY || hipHt > hipRadX)
		{
			ms.Translate(0,-buttRad+hipY,0);
			AuSolidEllipsoid butt(ms, hipRadX, buttRad, hipHt);
			solidLeg.Add(butt);
		}
		ms.LoadIdentity();
		AuSolidDomoid hip(ms, hipRadX, hipY, hipRadX, false);
		solidLeg.Add(hip);
		ms.RotateX(PI);
		AuSolidConoid thigh(ms, thighLen, hipRadX, hipY, kneeRadX, kneeRadY, false, false);
		solidLeg.Add(thigh);
		ms.Translate(0,0,thighLen);
		AuSolidConoid calf(ms, calfLen, kneeRadX, kneeRadY, ankleRadX, ankleRadY, false, false);
		solidLeg.Add(calf);
		ms.Translate(0,0,calfLen);
		AuSolidDomoid ankle(ms, ankleRadX, ankleRadY, ankleHt, false);
		solidLeg.Add(ankle);

		LATERALITY laterality = Laterality();

		// Foot
		AuSolid solidFoot;
		if(!clothes.BareFootCovered())
		{
			double toeRad = footWid/6;

			ms.Translate(0,-footLen/2 + ankleRadY + toeRad,ankleHt);
			ms.RotateX(PI);
			AuSolidDomoid foot(ms, footWid/2, footLen/2, footHt, true);
			solidFoot.Add(foot);

			// Toes
			if(laterality == Middle)
			{
				ms.Translate(0,footLen/2,0);
				AuSolidDome toe(ms, toeRad, true);
				solidFoot.Add(toe);
				double toeY = toeRad * .6;
				AuMatrix4x4 msL(ms);
				AuMatrix4x4 msR(ms);
				for(int t=0; t<2; t++)
				{
					toeRad *= .9;
					toeY -= toeRad * .7;

					msL.Translate(-toeRad*1.4,toeY,0);
					AuSolidDome toeL(msL, toeRad, true);
					solidFoot.Add(toeL);

					msR.Translate(toeRad*1.4,toeY,0);
					AuSolidDome toeR(msR, toeRad, true);
					solidFoot.Add(toeR);
				}
			}
			else
			{
				if(laterality == Left)
					ms.Translate(footWid/2 - toeRad*1.15,footLen/2,0);
				else
					ms.Translate(-footWid/2 + toeRad*1.15,footLen/2,0);
				AuSolidDome toe(ms, toeRad, true);
				solidFoot.Add(toe);
				double toeY = toeRad * .6;
				for(int t=0; t<4; t++)
				{
					toeRad *= .9;
					if(laterality == Left)
						ms.Translate(-toeRad*1.4,toeY,0);
					else
						ms.Translate(toeRad*1.4,toeY,0);
					toeY -= toeRad * .7;
					AuSolidDome toeN(ms, toeRad, true);
					solidFoot.Add(toeN);
				}
			}
		}

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec;
		switch(laterality)
		{
		case Left:
			blendUpVec = AuPoint(-1,0,0).RotateZ(45*DEG2RAD);
			break;
		case Middle:
			blendUpVec = AuPoint(0,0,-1);
			break;
		case Right:
			blendUpVec = AuPoint(1,0,0).RotateZ(-45*DEG2RAD);
			break;
		}

		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;

		// Meshes are added in pieces for part identification.
		// Leg mesh.
		const float triangleSize = .75f/12.f;
		AuScFaceList faces;
		if(!solidLeg.IsEmpty())
		{
			solidLeg.Faces(faces, triangleSize);
			mymesh.FromFaces(faces, AuMeatTri::Leg);
			if(clothes.BareThighCovered() && clothes.BareCalfCovered())
				mymesh.RemoveAbove(-(thighLen + calfLen * .9f));
			else if(clothes.BareThighCovered())
				mymesh.RemoveAbove(-thighLen * .9f);
			mymesh.SmoothNormals();
			mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, thighLen + calfLen, topBottomBlend, blendAngle,
					blendWid);
		}

		// Add leg features.
		//DHH dna.GetLegFeatures(mymesh, mNeck, *this);

		// Foot mesh
		if(!solidFoot.IsEmpty())
		{
			blendUpVec = AuPoint(0,0,1);
			faces.Clear();
			solidFoot.Faces(faces, triangleSize);
			AuMeatMesh footmesh;
			footmesh.FromFaces(faces, AuMeatTri::Foot);
			footmesh.SmoothNormals();
			footmesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,1,0), blendUpVec, footLen, topBottomBlend, blendAngle, blendWid);
			mymesh.Append(footmesh);
		}

		// Hair
		{
			AuFolliclePointArray follicles;
			const float lockWid = triangleSize * 2;
			HairFollicles(dna, lockWid, follicles);
			float headSpace = .25f/12;
			float lockLen;
			float hangLength;
			float knotLength;
			AuPoint combVec;
			bool basePatch;
			float widAng;
			bool wearsBra = dna.HasBreasts();
			for(AuFolliclePointArray::const_iterator hi = follicles.cbegin(); hi != follicles.cend(); hi++)
			{
				// Get information about the lock of hair.
				AuPoint knotLoc;
				AuHairStyleLock::STYLE lstyle = hairStyle.Lock(*hi, headSpace, lockLen, hangLength, knotLength, widAng, combVec,
						basePatch);
				if(lockLen == 0)
					continue;

				// Clothes may remove or shorten hair.
				if(clothes.CoverHair(*hi, basePatch, lockLen, wearsBra))
					continue;	// Hair is completely removed including the base patch, or there was no base patch.
				else if(lockLen == 0)
				{
					lstyle = AuHairStyleLock::Shaved;	// So we get only a base patch, no stubble.
					lockLen = 1;
				}

				// Make a lock of hair at the follicle.
				mymesh.Hair().MakeLock(*hi, lstyle, headSpace, lockWid, lockLen, hangLength, knotLoc, knotLength, widAng, combVec,
						basePatch, 0);
			}
		}

		// Done adding polygons. Set the bone to use by it's vertices.
		// Hip to knee is bone 0.
		mymesh.SetBonesRangeZ(-thighLen, mymesh.GrossMaxZ(), firstBone,1., 0,0., 0,0., 0,0.);
		// Knee to ankle is bone 1.
		mymesh.SetBonesRangeZ(-(thighLen+calfLen), -thighLen, firstBone+1,1., 0,0., 0,0., 0,0.);
		// Ankle to bottom of foot is bone 2.
		mymesh.SetBonesRangeZ(mymesh.GrossMinZ(), -(thighLen+calfLen), firstBone+2,1., 0,0., 0,0., 0,0.);
		// Middle of foot forward is bone 3.
		mymesh.SetBonesRangeYZ(footLen/2, mymesh.GrossMaxY(), mymesh.GrossMinZ(), -(thighLen+calfLen),
				firstBone+3,1., 0,0., 0,0., 0,0.);
	}

	AuMatrix4x4 m;
	m.Translate(dna.LegLocation(m_Which));
	mymesh.Transform(m);

	//qDebug() << "AuChromoLeg::GetSkinMesh mymesh" << mymesh.Count() << "which" << m_Which << "loc" << dna.LegLocation(m_Which);
	//AuSegmentArray i;
	//mesh.Join(mymesh, i);
	//intersection.append(i);

	mesh.Append(mymesh);
}

void AuChromoLeg::GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const
{
	Q_UNUSED(stage);
	// Create the mesh of the leg.
	AuMeatMesh mymesh;
	{
		double ankleRadX = AnkleRadiusMM() * FeetPerMM();
		double ankleRadY = ankleRadX;
		double ankleHt = AnkleLengthMM() * FeetPerMM();
		double buttRad = ButtRadiusMM() * FeetPerMM();
		double calfLen = CalfLengthMM() * FeetPerMM();
		double footHt = FootHeightMM() * FeetPerMM();
		double footLen = FootLengthMM() * FeetPerMM();
		double footWid = FootWidthMM() * FeetPerMM();
		double kneeRadX = KneeRadiusXMM() * FeetPerMM();
		double kneeRadY = KneeRadiusYMM() * FeetPerMM();
		double thighLen = ThighLengthMM() * FeetPerMM();
		double hipRadX = m_HipRadXMM * FeetPerMM();

		double hipHt = buttRad * 1.2;
		double hipY = hipRadX;

		// Leg
		AuSolid solidLeg;
		AuMatrix4x4 ms;
		if(buttRad > hipY || hipHt > hipRadX)
		{
			ms.Translate(0,-buttRad+hipY,0);
			AuSolidEllipsoid butt(ms, hipRadX, buttRad, hipHt);
			solidLeg.Add(butt);
		}
		ms.LoadIdentity();
		AuSolidDomoid hip(ms, hipRadX, hipY, hipRadX, false);
		solidLeg.Add(hip);
		ms.RotateX(PI);
		AuSolidConoid thigh(ms, thighLen, hipRadX, hipY, kneeRadX, kneeRadY, false, false);
		solidLeg.Add(thigh);
		ms.Translate(0,0,thighLen);
		AuSolidConoid calf(ms, calfLen, kneeRadX, kneeRadY, ankleRadX, ankleRadY, false, false);
		solidLeg.Add(calf);
		ms.Translate(0,0,calfLen);
		AuSolidDomoid ankle(ms, ankleRadX, ankleRadY, ankleHt, false);
		solidLeg.Add(ankle);

		// Shoe
		AuSolid solidFoot;
		double toeRad = footWid/6;
		double shoeLen = footLen + toeRad*2;
		ms.Translate(0,-footLen/2 + ankleRadY,ankleHt);
		ms.RotateX(PI);
		AuSolidDomoid foot(ms, footWid/2, shoeLen/2, footHt, true);
		solidFoot.Add(foot);

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec;
		LATERALITY laterality = Laterality();
		switch(laterality)
		{
		case Left:		// Left
			blendUpVec = AuPoint(-1,0,0).RotateZ(45*DEG2RAD);
			break;
		case Middle:		// Middle
			blendUpVec = AuPoint(0,0,-1);
			break;
		case Right:		// Right
			blendUpVec = AuPoint(1,0,0).RotateZ(-45*DEG2RAD);
			break;
		}
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;

		// Meshes are added in pieces for part identification.
		// Leg mesh.
		const float triangleSize = .75f/12.f * d_ClothingTessFactor;
		AuScFaceList faces;
		solidLeg.Faces(faces, triangleSize);
		mymesh.FromFaces(faces, AuMeatTri::Leg);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, thighLen + calfLen, topBottomBlend, blendAngle,
				blendWid);

		// Add leg features.
		//DHH dna.GetLegFeaturesClothed(mymesh, mNeck, *this);

		// Shoe mesh
		blendUpVec = AuPoint(0,0,1);
		faces.Clear();
		solidFoot.Faces(faces, triangleSize/2);
		AuMeatMesh footmesh;
		footmesh.FromFaces(faces, AuMeatTri::Foot);
		footmesh.SmoothNormals();
		footmesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,1,0), blendUpVec, footLen, topBottomBlend, blendAngle, blendWid);
		mymesh.Append(footmesh);

		// Done adding polygons. Set the bone to use by it's vertices.
		// Hip to knee is bone 0.
		mymesh.SetBonesRangeZ(-thighLen, mymesh.GrossMaxZ(), firstBone,1., 0,0., 0,0., 0,0.);
		// Knee to ankle is bone 1.
		mymesh.SetBonesRangeZ(-(thighLen+calfLen), -thighLen, firstBone+1,1., 0,0., 0,0., 0,0.);
		// Ankle to bottom of foot is bone 2.
		mymesh.SetBonesRangeZ(mymesh.GrossMinZ(), -(thighLen+calfLen), firstBone+2,1., 0,0., 0,0., 0,0.);
		// Middle of foot forward is bone 3.
		mymesh.SetBonesRangeYZ(footLen/2, mymesh.GrossMaxY(), mymesh.GrossMinZ(), -(thighLen+calfLen),
				firstBone+3,1., 0,0., 0,0., 0,0.);
	}

	AuMatrix4x4 m;
	m.Translate(dna.LegLocation(m_Which));
	mymesh.Transform(m);
	mesh.Append(mymesh);
}

void AuChromoLeg::HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles) const
{
	const float follicleSpacing = lockWid * .5f;

	// Get my hair genes.
	bool hasHair = false;
	const AuGeneHair* hairDNA[AuFolliclePoint::ROLE_Count];
	for(int r=0; r<AuFolliclePoint::ROLE_Count; r++)
	{
		hairDNA[r] = HairDNA((AuFolliclePoint::ROLE)r);
		if(hairDNA[r])
			hasHair = true;
	}
	if(!hasHair)
		return;	// I have no hair.

	// Create the mesh of the leg.
	AuMeatMesh mymesh;
	double buttRad = ButtRadiusMM() * FeetPerMM();
	double ankleHt = AnkleLengthMM() * FeetPerMM();
	double calfLen = CalfLengthMM() * FeetPerMM();
	double thighLen = ThighLengthMM() * FeetPerMM();
	double footHt = FootHeightMM() * FeetPerMM();
	LATERALITY laterality = Laterality();
	{
		double ankleRadX = AnkleRadiusMM() * FeetPerMM();
		double ankleRadY = ankleRadX;
		double footLen = FootLengthMM() * FeetPerMM();
		double footWid = FootWidthMM() * FeetPerMM();
		double kneeRadX = KneeRadiusXMM() * FeetPerMM();
		double kneeRadY = KneeRadiusYMM() * FeetPerMM();
		double hipRadX = m_HipRadXMM * FeetPerMM();

		double hipHt = buttRad * 1.2;
		double hipY = hipRadX;

		// Leg
		AuSolid solidLeg;
		AuMatrix4x4 ms;
		if(buttRad > hipY || hipHt > hipRadX)
		{
			ms.Translate(0,-buttRad+hipY,0);
			AuSolidEllipsoid butt(ms, hipRadX, buttRad, hipHt);
			solidLeg.Add(butt);
		}
		ms.LoadIdentity();
		AuSolidDomoid hip(ms, hipRadX, hipY, hipRadX, false);
		solidLeg.Add(hip);
		ms.RotateX(PI);
		AuSolidConoid thigh(ms, thighLen, hipRadX, hipY, kneeRadX, kneeRadY, false, false);
		solidLeg.Add(thigh);
		ms.Translate(0,0,thighLen);
		AuSolidConoid calf(ms, calfLen, kneeRadX, kneeRadY, ankleRadX, ankleRadY, false, false);
		solidLeg.Add(calf);
		ms.Translate(0,0,calfLen);
		AuSolidDomoid ankle(ms, ankleRadX, ankleRadY, ankleHt, false);
		solidLeg.Add(ankle);

		// Foot
		AuSolid solidFoot;
		double toeRad = footWid/6;
		ms.Translate(0,-footLen/2 + ankleRadY + toeRad,ankleHt);
		ms.RotateX(PI);
		AuSolidDomoid foot(ms, footWid/2, footLen/2, footHt, true);
		solidFoot.Add(foot);

		// Toes
		if(laterality == Middle)
		{
			ms.Translate(0,footLen/2,0);
			AuSolidDome toe(ms, toeRad, true);
			solidFoot.Add(toe);
			double toeY = toeRad * .6;
			AuMatrix4x4 msL(ms);
			AuMatrix4x4 msR(ms);
			for(int t=0; t<2; t++)
			{
				toeRad *= .9;
				toeY -= toeRad * .7;

				msL.Translate(-toeRad*1.4,toeY,0);
				AuSolidDome toeL(msL, toeRad, true);
				solidFoot.Add(toeL);

				msR.Translate(toeRad*1.4,toeY,0);
				AuSolidDome toeR(msR, toeRad, true);
				solidFoot.Add(toeR);
			}
		}
		else
		{
			if(laterality == Left)
				ms.Translate(footWid/2 - toeRad*1.15,footLen/2,0);
			else
				ms.Translate(-footWid/2 + toeRad*1.15,footLen/2,0);
			AuSolidDome toe(ms, toeRad, true);
			solidFoot.Add(toe);
			double toeY = toeRad * .6;
			for(int t=0; t<4; t++)
			{
				toeRad *= .9;
				if(laterality == Left)
					ms.Translate(-toeRad*1.4,toeY,0);
				else
					ms.Translate(toeRad*1.4,toeY,0);
				toeY -= toeRad * .7;
				AuSolidDome toeN(ms, toeRad, true);
				solidFoot.Add(toeN);
			}
		}

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 2;
		AuPoint blendUpVec;
		switch(laterality)
		{
		case Left:
			blendUpVec = AuPoint(-1,0,0).RotateZ(45*DEG2RAD);
			break;
		case Middle:
			blendUpVec = AuPoint(0,0,-1);
			break;
		case Right:
			blendUpVec = AuPoint(1,0,0).RotateZ(-45*DEG2RAD);
			break;
		}
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;

		// Meshes are added in pieces for part identification.
		// Leg mesh.
		float triangleSize = follicleSpacing;
		AuScFaceList faces;
		solidLeg.Faces(faces, triangleSize);
		mymesh.FromFaces(faces, AuMeatTri::Leg);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, thighLen + calfLen, topBottomBlend, blendAngle,
				blendWid);

		// Add leg features.
		//DHH dna.GetLegFeatures(mymesh, mNeck, *this);

		// Foot mesh
		blendUpVec = AuPoint(0,0,1);
		faces.Clear();
		solidFoot.Faces(faces, triangleSize);
		AuMeatMesh footmesh;
		footmesh.FromFaces(faces, AuMeatTri::Foot);
		footmesh.SmoothNormals();
		footmesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,1,0), blendUpVec, footLen, topBottomBlend, blendAngle, blendWid);
		mymesh.Append(footmesh);
	}

	AuPointNArray myfol;
	mymesh.SurfacePoints(myfol);

	// Remove follicles from face, eyes and ears.
	QMap<AuPoint, double> hairEx;	// Exclusion locations with radius.
	//DHH dna.LegHairExclusions(hairEx, mSkull, *this);

	for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
	{
		// Get information about the hair follicle.
		double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
		//double zang = (*fi).m_N.PolarIncline();
		//double chestang = 15*DEG2RAD;			// XY angle forward of which hair is chest.
		//double rearflankang = 45*DEG2RAD;		// XY angle behind which hair is rear flank.
		double backangf = 0*DEG2RAD;			// XY angle behind which hair is back.
		double backangb = 180*DEG2RAD;			// XY angle behind which hair is back.
		double buttang = 100*DEG2RAD;			// XY angle where butt hair starts

		// Decide whether the follicle should stay or go.
		AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
		if((*fi).z <= -(ankleHt + calfLen + thighLen))
			role = AuFolliclePoint::Soles;	// bottom of foot
		else if((*fi).z <= -(calfLen + thighLen + ankleHt - footHt))
			role = AuFolliclePoint::Feet;	// top of foot
		else if((*fi).z > -buttRad && (xyang > buttang || xyang < -buttang))
			role = AuFolliclePoint::Butt;
		else if((*fi).z < -(ankleHt + calfLen*.75 + thighLen))
		{
			// Ankles
			switch(laterality)
			{
			case Left:		// Left
				if(!(xyang > backangf && xyang < backangb))
					role = AuFolliclePoint::AnkleUnder;
				else
					role = AuFolliclePoint::AnkleBack;
				break;
			case Middle:		// Middle
				if(fabs(xyang) < 90*DEG2RAD)
					role = AuFolliclePoint::AnkleUnder;
				else
					role = AuFolliclePoint::AnkleBack;
				break;
			case Right:		// Right
				if(!(xyang < backangf && xyang > -backangb))
					role = AuFolliclePoint::AnkleUnder;
				else
					role = AuFolliclePoint::AnkleBack;
				break;
			}
		}
		else
		{
			// Leg
			switch(laterality)
			{
			case Left:		// Left
				if(!(xyang > backangf && xyang < backangb))
					role = AuFolliclePoint::LegUnder;
				else
					role = AuFolliclePoint::LegBack;
				break;
			case Middle:		// Middle
				if(fabs(xyang) < 90*DEG2RAD)
					role = AuFolliclePoint::LegUnder;
				else
					role = AuFolliclePoint::LegBack;
				break;
			case Right:		// Right
				if(!(xyang < backangf && xyang > -backangb))
					role = AuFolliclePoint::LegUnder;
				else
					role = AuFolliclePoint::LegBack;
				break;
			}
		}

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		if(!hairDNA[role])
			continue;	// No hair there.

		/*
		// Check for exclusion by leg features.
		if(role != AuFolliclePoint::ROLE_Count && !hairEx.empty())
			for(QMap<AuPoint, double>::const_iterator xi = hairEx.cbegin(); xi != hairEx.cend(); xi++)
				if((*fi).Distance2(xi.key()) < (*xi)*(*xi))
				{
					role = AuFolliclePoint::ROLE_Count;
					break;
				}
		*/

		if(role == AuFolliclePoint::ROLE_Count)
			continue;

		const QColor& hairColor = hairDNA[role]->Color();
		float maxLen = hairDNA[role]->LengthMM() * FeetPerMM();
		AuFolliclePoint fp(0, role, *fi, hairColor, maxLen);
		follicles.append(fp);
	}
}

quint64 AuChromoLeg::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::AnkleBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::AnkleUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Butt) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Feet) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::LegBack) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::LegUnder) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Soles);
}

quint16 AuChromoLeg::FootHeightMM() const
{
	//return AnkleLengthMM() / 2;
	return FootWidthMM() / 2;
}

AuPoint AuChromoLeg::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	AuPoint boneLoc(dna.LegLocation(m_Which));
	if(bone == 1)
		boneLoc.z -= ThighLengthMM() * FeetPerMM();							// Knee
	else if(bone == 2)
		boneLoc.z -= (ThighLengthMM() + CalfLengthMM()) * FeetPerMM();		// Ankle
	else if(bone == 3)
	{
		// Ball of the foot, to bend toes.
		boneLoc.y += FootLengthMM() * FeetPerMM() / 2;
		boneLoc.z -= (ThighLengthMM() + CalfLengthMM() + AnkleLengthMM()) * FeetPerMM();		// Toes
	}
	return boneLoc;
}

int AuChromoLeg::ListWidgetOrder() const
{
	return 500 + m_Which.Index();
}

QString AuChromoLeg::ListWidgetText() const
{
	return QObject::tr("Leg %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoLeg::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageLeg* pp = new AuChromoPageLeg(parent);
	return pp;
}

bool AuChromoLeg::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoLeg::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of legs.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

bool AuChromoLeg::StandsFlat() const
{
	return true;	//DHH
}

void AuChromoLeg::CalcStandAngles(const AuPose& pose, float strength, float& hipX, float& hipY, float& hipZ, float& kneeX,
		float& ankleX, float& toeX) const
{
	// Init return values.
	hipX = 0;
	hipY = -1*DEG2RADf;	// Spread the feet apart slightly.
	hipZ = 0;
	kneeX = 0;
	ankleX = 0;
	toeX = 0;

	const float thighlen = ThighLengthMM() * FeetPerMM();
	const float calflen = CalfLengthMM() * FeetPerMM();
	const float ankleHt = AnkleLengthMM() * FeetPerMM();
	const float footlen = FootLengthMM() * FeetPerMM();
	const float standTone = (float)m_StandTone / 100.f * strength;
	const float balllen = footlen / 2;	// Foot bends in half for ball.
	const float archlen = sqrtf(balllen * balllen + ankleHt * ankleHt);		// Distance from ankle to ball of foot.

	// Calculate the lowest height we can smash our legs.
	// Imagine leg folded sitting butt on foot, ankle bent so foot sits flat, regardless of if it bends its ankle or not.
	float minHt;
	if(ButtRadiusMM() > HipRadiusXMM())
		minHt = ButtRadiusMM() * FeetPerMM();
	else
		minHt = HipRadiusXMM() * FeetPerMM();
	minHt += AnkleRadiusMM() * FeetPerMM();
	minHt += ankleHt;

	// Calculate the tallest height we can stretch our legs to stand, considering our ankle bend.
	// Imagine leg straight down, ankle bent so foot is straight down (assumes longer foot than ankle height), bent at toes so ball
	// of foot sits flat.
	bool standFlatFooted = StandsFlat() && pose != AuPose::Crouch && pose != AuPose::StandRun;
	const float footHt = standFlatFooted ? ankleHt : archlen;
	float maxHt = thighlen + calflen + footHt;
	if(maxHt <= minHt)
		return;	// Fat?

	// Knowing my range, select a hip height based on my standing muscle tone.
	float htRange = maxHt - minHt;
	float hipHt = minHt + htRange * standTone;
	if(hipHt < minHt)
		hipHt = minHt;
	else if(hipHt > maxHt)
		hipHt = maxHt;

	// Hip Z spreads the feet nicely when standing tall but it makes the legs cross when squatting.
	hipZ = -15*DEG2RADf * standTone;

	// A non-flat footed leg that does not have room for the tall foot reverts to standing flat footed.
	if(!standFlatFooted)
	{
		hipZ /= 2;
		float minHtBallFoot = minHt - ankleHt + archlen;	// Min height with vertical ankle to ball line.
		if(hipHt <= minHtBallFoot)
			standFlatFooted = true;				// Must stand flat footed because ankle to ball is too tall.
	}

	// Calculate leg angles, knowing our hip height.
	if(standFlatFooted)
	{
		// This leg stands flat footed.
		float range = hipHt - ankleHt;
		float kneeHt = range * calflen / (thighlen + calflen);	// Knee height above the foot.
		if(kneeHt > calflen)
			kneeHt = calflen;
		else if(kneeHt < range - thighlen)
			kneeHt = range - thighlen;

		float sinank = kneeHt / calflen;
		if(sinank > 1)
			sinank = 1;
		else if(sinank < -1)
			sinank = -1;
		float ank = asinf(sinank);
		ankleX = PIf/2 - ank;

		float h = range - kneeHt;				// Distance down from hip to knee.
		float sinhip = h / thighlen;
		if(sinhip > 1)
			sinhip = 1;
		else if(sinhip < -1)
			sinhip = -1;
		float hip = asinf(sinhip);
		hipX = PIf/2 - hip;

		kneeX = ank + hip - PIf;

		toeX = 0;
	}
	else
	{
		// This leg stands on the ball of the foot.
		// Bend the ankle so the ankle bone is directly above the ball of the foot.
		float range = hipHt - archlen;
		float kneeHt = range * calflen / (thighlen + calflen);	// Knee height above the foot.
		if(kneeHt > calflen)
			kneeHt = calflen;
		else if(kneeHt < range - thighlen)
			kneeHt = range - thighlen;

		float h = range - kneeHt;				// Distance down from hip to knee.
		float sinhip = h / thighlen;
		if(sinhip > 1)
			sinhip = 1;
		else if(sinhip < -1)
			sinhip = -1;
		float hip = asinf(sinhip);
		hipX = PIf/2 - hip;

		float cosknee = kneeHt / calflen;
		if(cosknee > 1)
			cosknee = 1;
		else if(cosknee < -1)
			cosknee = -1;
		float knee = acosf(cosknee);
		kneeX = -knee - hipX;

		float sinank = balllen / archlen;
		if(sinank > 1)
			sinank = 1;
		else if(sinank < -1)
			sinank = -1;
		float ank = asinf(sinank);
		ankleX = -(PIf/2 - ank);
		ankleX = -ank - hipX - kneeX;

		float angtoe = PIf/2 - ank;
		toeX = PIf/2 - angtoe;
	}
	//qDebug() << "AuChromoLeg::CalcStandAngles hipHt" << hipHt
		//<< "hipX" << (hipX*RAD2DEGf)
		//<< "kneeX" << (kneeX*RAD2DEGf)
		//<< "ankleX" << (ankleX*RAD2DEGf)
		//<< "toeX" << (toeX*RAD2DEGf);
}

float AuChromoLeg::HipAltitudePosed(const AuChromatid& ctd) const
{
	AuPoint ballLoc = BoneLoc(ctd, 3);		// Bone 3 is the ball of my foot.
	return -ballLoc.z;
}

void AuChromoLeg::AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
		HANDEDNESS hand, bool hasMaleGenitals, float torsoThickness,
		float& hipX, float& hipY, float& hipZ, float& kneeX, float& ankleX, float& toeX) const
{
	hipX = 0;
	hipY = 0;
	hipZ = 0;
	kneeX = 0;
	ankleX = 0;
	toeX = 0;

	LATERALITY laterality = Laterality();
	switch(pose.Pose())
	{
	// Special
	case AuPose::Init:
	case AuPose::Spawn:
		break;

	// Head Only
	case AuPose::EyesOnly:
		break;

	// Horizontal Poses
	case AuPose::Prone:
	case AuPose::ProneCreep:
		hipX = 10*DEG2RADf;
		hipZ = -15*DEG2RADf;
		kneeX = -15*DEG2RADf;
		ankleX = -45*DEG2RADf;
		break;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		hipX = 10*DEG2RADf;
		hipZ = -15*DEG2RADf;
		kneeX = -15*DEG2RADf;
		ankleX = -30*DEG2RADf;
		toeX = -15*DEG2RADf;
		break;

	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		hipX = 30*DEG2RADf;
		hipY = 5*DEG2RADf;
		kneeX = -85*DEG2RADf;
		ankleX = -10*DEG2RADf;
		break;

	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		hipX = 30*DEG2RADf;
		hipY = 5*DEG2RADf;
		kneeX = -85*DEG2RADf;
		ankleX = -10*DEG2RADf;
		break;

	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
	case AuPose::Patient_Prone:			// Prone patient faces up.
		if(laterality == Left)
		{
			hipX = 10*DEG2RADf;
			kneeX = -25*DEG2RADf;
		}
		else
		{
			hipX = 5*DEG2RADf;
			kneeX = -12*DEG2RADf;
		}
		hipY = -2*DEG2RADf;
		hipZ = -15*DEG2RADf;
		ankleX = -15*DEG2RADf;
		toeX = 0;
		break;

	// Angles for dead poses are slightly tighter than writhe poses, and inanimate.
	case AuPose::DeadFaceDown:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			hipX = 5*DEG2RADf;
			hipY = -10*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -10*DEG2RADf;
			ankleX = -40*DEG2RADf;
			toeX = -35*DEG2RADf;
		}
		else
		{
			hipX = 5*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -10*DEG2RADf;
			ankleX = -40*DEG2RADf;
			toeX = -35*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceLeft:
		if(laterality == Left)
		{
			hipX = 20*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -25*DEG2RADf;
			ankleX = -15*DEG2RADf;
		}
		else
		{
			hipX = 30*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -80*DEG2RADf;
			ankleX = -45*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceRight:
		if(laterality == Right)
		{
			hipX = 20*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -25*DEG2RADf;
			ankleX = -15*DEG2RADf;
		}
		else
		{
			hipX = 30*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -80*DEG2RADf;
			ankleX = -45*DEG2RADf;
		}
		break;
	case AuPose::DeadFaceUp:
		hipX = 0;
		hipY = -2*DEG2RADf;
		hipZ = -15*DEG2RADf;
		kneeX = -5*DEG2RADf;
		ankleX = -6*DEG2RADf;
		toeX = 0;
		break;

	// Angles for writhe/dying poses are based on dead poses, only animated to show signs of life.
	case AuPose::WritheFaceDown:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			hipX = 10*DEG2RADf;
			hipY = -10*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -15*DEG2RADf;
			ankleX = -30*DEG2RADf;
			toeX = -15*DEG2RADf;
		}
		else
		{
			hipX = 10*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -15*DEG2RADf;
			ankleX = -30*DEG2RADf;
			toeX = -15*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceLeft:
		if(laterality == Left)
		{
			hipX = 20*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -25*DEG2RADf;
			ankleX = -15*DEG2RADf;
		}
		else
		{
			hipX = 30*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -80*DEG2RADf;
			ankleX = -45*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceRight:
		if(laterality == Right)
		{
			hipX = 20*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -25*DEG2RADf;
			ankleX = -15*DEG2RADf;
		}
		else
		{
			hipX = 30*DEG2RADf;
			hipY = 5*DEG2RADf;
			kneeX = -80*DEG2RADf;
			ankleX = -45*DEG2RADf;
		}
		break;
	case AuPose::WritheFaceUp:
		if((hand == LeftHanded && laterality == Left) ||
			(hand == RightHanded && laterality == Right))
		{
			AuDice dice(id.GetID());
			float x = when.CurrentDayCycle() * 25000 + dice.Roll(1,10000);
			float p = AuNoiseF::CoherentNoise(id.GetID() % 12, x);
			if(p < 0)
				p = 0;
			hipX = 10*DEG2RADf + p*60*DEG2RADf;
			kneeX = -25*DEG2RADf - p*120*DEG2RADf;
			ankleX = -40*DEG2RADf + p*30*DEG2RADf;
		}
		else
		{
			hipX = 5*DEG2RADf;
			kneeX = -12*DEG2RADf;
			ankleX = -15*DEG2RADf;
		}
		hipY = -2*DEG2RADf;
		hipZ = -15*DEG2RADf;
		toeX = 0;
		break;

	case AuPose::MateOnBottom:
	case AuPose::ReceptiveMate:
		hipX = 15*DEG2RADf;
		hipZ = -10*DEG2RADf;
		kneeX = -40*DEG2RADf;
		ankleX = -30*DEG2RADf;
		toeX = 0;
		if(hasMaleGenitals)
			hipY = -5*DEG2RADf;
		else
			hipY = -30*DEG2RADf;
		break;

	case AuPose::MateOnTop:
		// The vertical range of movement of the cg is torsoThickness/4.
		if(hasMaleGenitals)
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			float persin = pose.Rhythm(bodyHz, cycleOffset, when);
			float vertOff = torsoThickness * 1.25 + persin * torsoThickness/4;	// Height from body cg to the ground.
			float thighLen = m_ThighLenMM * FeetPerMM();
			float sa = vertOff / thighLen;
			float anga = asinf(sa);
			hipX = -(PIf/2 - anga) + PIf/4;	// buttX is already at -45
			hipY = -2*DEG2RADf;
			hipZ = 0;
			kneeX = -anga - 10*DEG2RADf;
			ankleX = -20*DEG2RADf - persin*30*DEG2RADf;
			toeX = 60*DEG2RADf;
		}
		else
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			float persin = pose.Rhythm(bodyHz, cycleOffset, when);
			float vertOff = torsoThickness * 1.25 + persin * torsoThickness/4;	// Height from body cg to the ground.
			float thighLen = m_ThighLenMM * FeetPerMM();
			float sa = vertOff / thighLen;
			float anga = asinf(sa);
			hipX = PIf/2 - anga;
			hipY = -30*DEG2RADf;
			hipZ = 20*DEG2RADf;
			kneeX = -(PIf - anga);
			ankleX = -20*DEG2RADf + persin*30*DEG2RADf;
			toeX = 60*DEG2RADf;
		}
		break;

	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		//DHH animalia
		break;

	case AuPose::Swim:
	case AuPose::SwimFast:
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality == Left)
				cycleOffset += PIf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			hipX = 10*DEG2RADf + 5*perang;
			hipY = -2*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -25*DEG2RADf + 10*perang;
			ankleX = -40*DEG2RADf + 15*perang;
			toeX = -15*DEG2RADf;
		}
		break;
	case AuPose::TreadWater:
		{
			float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality == Left)
				cycleOffset += PIf;
			float perang = pose.Rhythm(bodyHz, cycleOffset, when) * DEG2RADf;
			hipX = 10*perang;
			hipY = -3*DEG2RADf;
			hipZ = -15*DEG2RADf;
			kneeX = -25*DEG2RADf + 20*perang;
			ankleX = -40*DEG2RADf + 15*perang;
			toeX = -15*DEG2RADf;
		}
		break;
	
	// Sitting Poses
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Post_Sit:
		if(hasMaleGenitals)
		{
			hipX = 65*DEG2RADf;		// buttX already at 30 degrees
			hipY = -15*DEG2RADf;
			kneeX = -95*DEG2RADf;
		}
		else
		{
			hipX = 60*DEG2RADf;		// buttX already at 30 degrees
			hipY = 2*DEG2RADf;
			kneeX = -90*DEG2RADf;
		}
		break;

	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
		if(hasMaleGenitals)
		{
			// Cross legged.
			hipX = 90*DEG2RADf;	// buttX already at 30 degrees
			hipY = -30*DEG2RADf;
			hipZ = -50*DEG2RADf;
			if(laterality == Left)
				kneeX = -130*DEG2RADf;
			else
				kneeX = -110*DEG2RADf;
		}
		else
		{
			// Legs straight with knees bent.
			hipX = 80*DEG2RADf;	// buttX already at 30 degrees
			hipZ = 1*DEG2RADf;
			kneeX = -50*DEG2RADf;
			ankleX = -30*DEG2RADf;
		}
		break;

	case AuPose::Straddle:
		hipX = 60*DEG2RADf;		// buttX already at 30 degrees
		hipZ = -30*DEG2RADf;
		kneeX = -80*DEG2RADf;
		break;

	// Crouched Poses
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
		CalcStandAngles(pose, .5f, hipX, hipY, hipZ, kneeX, ankleX, toeX);
		break;

	// Standing
	case AuPose::Stand:
	case AuPose::Post_Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		CalcStandAngles(pose, 1, hipX, hipY, hipZ, kneeX, ankleX, toeX);
		break;

	case AuPose::StandRun:
		{
			CalcStandAngles(pose, 1, hipX, hipY, hipZ, kneeX, ankleX, toeX);

			float perang = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality == Left)
				perang += PIf;
			float persin = pose.Rhythm(bodyHz, perang, when);

			hipX += (-3 - 40.*persin)*DEG2RADf;
			float hipZsin = sinf(fabs(hipZ)) * .8;
			float kneesweep = (PIf + kneeX)/6. * (1 - hipZsin);
			kneeX -= (kneesweep - kneesweep*persin);
		}
		break;

	case AuPose::StandWalk:
		{
			CalcStandAngles(pose, 1, hipX, hipY, hipZ, kneeX, ankleX, toeX);

			float perang = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
			if(laterality == Left)
				perang += PIf;
			float persin = pose.Rhythm(bodyHz, perang, when);

			hipX += (-3 - 10.*persin)*DEG2RADf;
			float hipZsin = sinf(fabs(hipZ)) * .8;
			float kneesweep = kneeX/2. * (1 - hipZsin);
			kneeX -= (kneesweep - kneesweep*persin);
		}
		break;

	// Misc
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		break;

	// Mouth Poses
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		break;
	}

	//DHH
	//hipX = 90*DEG2RADf;
	//hipY = 0*DEG2RADf;
	//hipZ = -15*DEG2RADf;

	//kneeX = 0;
	//kneeX = when.SecCycle() * (-90*DEG2RADf);	//DHH

	//ankleX = 0;
	//ankleX = when.SecCycle() * (90*DEG2RADf) - (60*DEG2RADf);	//DHH

	//toeX = 0;
	//toeX = when.SecCycle() * (90*DEG2RADf);	//DHH

	// Reverse Z and Y angles for left side.
	if(laterality == Left)
	{
		hipZ = -hipZ;
		hipY = -hipY;
	}
	else if(laterality == Middle)
		hipZ = 0;
}

void AuChromoLeg::SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
{
	for(QList<AuAnimalBone>::const_iterator bi = bones.cbegin(); bi != bones.cend(); bi++)
	{
		AuPoint bloc((*bi).PosedLoc());
		min = min.Min(bloc);
		max = max.Max(bloc);
	}
}

bool AuChromoLeg::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
{
	double ankleZ = m_AnkleLenMM * FeetPerMM();			// Length from ankle joint to base of foot.
	double ankleRad = m_AnkleRadMM * FeetPerMM();		// Radius of ankle.
	double buttRad = m_ButtRadMM * FeetPerMM();			// Distance from hip joint to back of butt cheek.
	double calfZ = m_CalfLenMM * FeetPerMM();
	double footLen = m_FootLenMM * FeetPerMM();			// From ankle joint to tip of foot.
	double footWid = m_FootWidMM * FeetPerMM();
	double footRad = footWid/2;
	double hipRad = m_HipRadXMM * FeetPerMM();			// From hip joint to side of hip meat.
	double kneeRad = m_KneeRadXMM * FeetPerMM();		// X radius of knee. Y radius depends on style.
	double thighZ = m_ThighLenMM * FeetPerMM();

	// Use bone matrices to put the solids at their posed locations.
	// Thigh
	AuMatrix4x4 mt(bones[0].Matrix());
	mt.Translate(bones[0].UnposedLoc());
	mt.Translate(0,0,-thighZ);	// Move down to the knee to draw the solid.
	AuSolidConoid thigh(mt, thighZ, kneeRad, kneeRad, hipRad, buttRad, true, true);

	// Calf
	AuMatrix4x4 mc(bones[1].Matrix());
	mc.Translate(bones[1].UnposedLoc());
	mc.Translate(0,0,-calfZ);	// Move down to the ankle to draw the solid.
	AuSolidConoid calf(mc, calfZ, ankleRad, ankleRad, kneeRad, kneeRad, true, true);

	// Foot
	AuMatrix4x4 mf;
	mf *= bones[2].Matrix();
	mf.Translate(bones[2].UnposedLoc());
	mf.Translate(0,footLen/2,-ankleZ);	// Move down and forward to the ball to draw the solid.
	mf.RotateX(PI/2);
	AuSolidConoid foot(mf, footLen/2, footRad, footRad, ankleRad, ankleRad, true, true);

	// Toes
	AuMatrix4x4 mo(bones[3].Matrix());
	mo.Translate(bones[3].UnposedLoc());
	mo.Translate(0,footLen/2,0);	// Move forward to the tip of the toes to draw the solid.
	mo.RotateX(PI/2);
	AuSolidConoid toes(mo, footLen/2, footRad, footRad, footRad, footRad, true, true);

	bool sighted = thigh.SightTarget(eyeLoc, sightedLoc);
	if(calf.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	if(foot.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	if(toes.SightTarget(eyeLoc, sightedLoc))
		sighted = true;
	return sighted;
}

float AuChromoLeg::Volume() const
{
	float ankleZ = m_AnkleLenMM * FeetPerMM();			// Length from ankle joint to base of foot.
	float ankleRad = m_AnkleRadMM * FeetPerMM();		// Radius of ankle.
	float buttRad = m_ButtRadMM * FeetPerMM();			// Distance from hip joint to back of butt cheek.
	float calfZ = m_CalfLenMM * FeetPerMM();
	float footLen = m_FootLenMM * FeetPerMM();			// From ankle joint to tip of foot.
	float footWid = m_FootWidMM * FeetPerMM();
	float footRad = footWid/2;
	float hipRad = m_HipRadXMM * FeetPerMM();			// From hip joint to side of hip meat.
	float kneeRad = m_KneeRadXMM * FeetPerMM();		// X radius of knee. Y radius depends on style.
	float thighZ = m_ThighLenMM * FeetPerMM();

	float hipHt = buttRad * 1.2f;
	float hipY = hipRad;

	float v = 0;

	// Butt
	if(buttRad > hipY || hipHt > hipRad)
	{
		// Volume of ellipsoid
		// V = 4/3 PI a b c
		// a b c are the radii
		v += 4.f/3.f * PIf * hipRad * buttRad * hipHt / 2;	// 1/2 because it is mostly embedded with the hip of the thigh
	}

	// Thigh
	if(m_HipRadXMM == m_KneeRadXMM)
	{
		// Thigh is a cylinder.
		// V = PI * r^2 * h
		v += PIf * hipRad * hipRad * thighZ;
	}
	else
	{
		// Thigh is a truncated cone.
		float h;	// Height extended to full cone.
		float r;
		if(hipRad > kneeRad)
		{
			float s = (hipRad - kneeRad) / thighZ;	// Slope of the side
			r = hipRad;
			h = r / s;
		}
		else
		{
			float s = (kneeRad - hipRad) / thighZ;	// Slope of the side
			r = kneeRad;
			h = r / s;
		}
		// Volume of cone.
		// V = PI * r^2 * h / 3
		float vc = PIf * r * r * h / 3;				// Volume of full cone.
		float vt = PIf * r * r * (h-thighZ) / 3;	// Volume of tip cone.
		v += (vc - vt);								// Volume of truncated cone is full cone minus the tip.
	}

	// Calf
	if(m_AnkleRadMM == m_KneeRadXMM)
	{
		// Calf is a cylinder.
		// V = PI * r^2 * h
		v += PIf * kneeRad * kneeRad * calfZ;
	}
	else
	{
		// Calf is a truncated cone.
		float h;	// Height extended to full cone.
		float r;
		if(kneeRad > ankleRad)
		{
			float s = (kneeRad - ankleRad) / calfZ;	// Slope of the side
			r = kneeRad;
			h = r / s;
		}
		else
		{
			float s = (ankleRad - kneeRad) / calfZ;	// Slope of the side
			r = ankleRad;
			h = r / s;
		}
		// Volume of cone.
		// V = PI * r^2 * h / 3
		float vc = PIf * r * r * h / 3;				// Volume of full cone.
		float vt = PIf * r * r * (h-calfZ) / 3;		// Volume of tip cone.
		v += (vc - vt);								// Volume of truncated cone is full cone minus the tip.
	}

	// Foot is calculated as a half ellipsoid.
	// Volume of ellipsoid
	// V = 4/3 PI a b c
	// a b c are the radii
	float a = footRad;
	float b = footLen / 2;
	float c = ankleZ;
	float vf = 4.f/3.f * PIf * a * b * c / 2;
	v += vf;

	return v;
}

bool AuChromoLeg::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoLeg* c = (const AuChromoLeg*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	return true;
}

AuChromosome* AuChromoLeg::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoLeg* c = dynamic_cast<const AuChromoLeg*>(g);
	if(!c)
		return 0;
	AuChromoLeg* comb = new AuChromoLeg(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoFin::AuChromoFin(const AuChromoWhich& which, AuChromoFin::STYLE style, quint16 lengthMM, quint8 where)
{
	m_LengthMM = lengthMM;
	m_Style = style;
	m_Where = where;
	m_Which = which;
}

AuChromoFin::AuChromoFin()
{
	m_LengthMM = 100;
	m_Style = Side;
	m_Where = 0;
}

AuChromoFin::AuChromoFin(const AuChromoFin& c) : AuChromosomeWithGenes(c)
{
	m_LengthMM = c.m_LengthMM;
	m_Style = c.m_Style;
	m_Where = c.m_Where;
	m_Which = c.m_Which;
}

AuChromoFin::AuChromoFin(const AuChromoFin& c1, const AuChromoFin& c2, const AuDice& dice) :
	AuChromosomeWithGenes(c1, c2, dice)
{
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoFin which mismatch" << c1.m_Which << c2.m_Which;
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	m_Style = c1.m_Style;
	m_Where = c1.m_Where;
	m_Which = c1.m_Which;
}

void AuChromoFin::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoFin& c = dynamic_cast<const AuChromoFin&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_LengthMM = c.m_LengthMM;
		m_Style = c.m_Style;
		m_Where = c.m_Where;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoFin::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_LengthMM;
	out << (quint8)m_Style;
	out << m_Where;
	out << m_Which;
}

void AuChromoFin::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_LengthMM;
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Where;
	in >> m_Which;
}

void AuChromoFin::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << (quint8)m_Style;
	out << m_Where;
	out << m_Which;
}

void AuChromoFin::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Where;
	in >> m_Which;
}

void AuChromoFin::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoFin::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalFin(*this, dna, firstBone);
}

AuPoint AuChromoFin::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	//DHH
	return AuPoint(0,0);
}

int AuChromoFin::ListWidgetOrder() const
{
	return 600 + m_Which.Index();
}

QString AuChromoFin::ListWidgetText() const
{
	return QObject::tr("Fin %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoFin::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageFin* pp = new AuChromoPageFin(parent);
	return pp;
}

bool AuChromoFin::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoFin::Laterality(const AuChromoWhich& which)
{
	//if(m_Where)
		//return Middle;	// Top or bottom
	return which.Index() & 1 ? Right : Left;
}

bool AuChromoFin::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoFin* c = (const AuChromoFin*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Style != c->m_Style)
		return false;
	if(m_Where != c->m_Where)
		return false;
	return true;
}

AuChromosome* AuChromoFin::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoFin* c = dynamic_cast<const AuChromoFin*>(g);
	if(!c)
		return 0;
	AuChromoFin* comb = new AuChromoFin(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoTentacle::AuChromoTentacle(const AuChromoWhich& which, AuChromoTentacle::STYLE style)
{
	m_Style = style;
	m_Which = which;
}

AuChromoTentacle::AuChromoTentacle()
{
	m_Style = Octopus;
}

AuChromoTentacle::AuChromoTentacle(const AuChromoTentacle& c) : AuChromosomeWithGenes(c)
{
	m_Style = c.m_Style;
	m_Which = c.m_Which;
}

AuChromoTentacle::AuChromoTentacle(const AuChromoTentacle& c1, const AuChromoTentacle& c2, const AuDice& dice) :
	AuChromosomeWithGenes(c1, c2, dice)
{
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoTentacle which mismatch" << c1.m_Which << c2.m_Which;
	m_Style = c1.m_Style;
	m_Which = c1.m_Which;
}

void AuChromoTentacle::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoTentacle& c = dynamic_cast<const AuChromoTentacle&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_Style = c.m_Style;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoTentacle::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoTentacle::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoTentacle::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoTentacle::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoTentacle::RandomizeSpecies(const AuDice& dice)
{
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoTentacle::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalTentacle(*this, dna, firstBone);
}

AuPoint AuChromoTentacle::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	//DHH
	return AuPoint(0,0);
}

int AuChromoTentacle::ListWidgetOrder() const
{
	return 700 + m_Which.Index();
}

QString AuChromoTentacle::ListWidgetText() const
{
	return QObject::tr("Tentacle %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoTentacle::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageTentacle* pp = new AuChromoPageTentacle(parent);
	return pp;
}

bool AuChromoTentacle::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoTentacle::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of tentacles.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

bool AuChromoTentacle::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoTentacle* c = (const AuChromoTentacle*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Style != c->m_Style)
		return false;
	return true;
}

AuChromosome* AuChromoTentacle::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoTentacle* c = dynamic_cast<const AuChromoTentacle*>(g);
	if(!c)
		return 0;
	AuChromoTentacle* comb = new AuChromoTentacle(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoWing::AuChromoWing(const AuChromoWhich& which, AuChromoWing::STYLE style, quint16 lengthMM)
{
	m_LengthMM = lengthMM;
	m_Style = style;
	m_Which = which;
}

AuChromoWing::AuChromoWing()
{
	m_LengthMM = 100;
	m_Style = Bird;
}

AuChromoWing::AuChromoWing(const AuChromoWing& c) : AuChromosomeWithGenes(c)
{
	m_LengthMM = c.m_LengthMM;
	m_Style = c.m_Style;
	m_Which = c.m_Which;
}

AuChromoWing::AuChromoWing(const AuChromoWing& c1, const AuChromoWing& c2, const AuDice& dice) :
	AuChromosomeWithGenes(c1, c2, dice)
{
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoWing which mismatch" << c1.m_Which << c2.m_Which;
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	m_Style = c1.m_Style;
	m_Which = c1.m_Which;
}

void AuChromoWing::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoWing& c = dynamic_cast<const AuChromoWing&>(o);
		AuChromosomeWithGenes::Copy(o);
		m_LengthMM = c.m_LengthMM;
		m_Style = c.m_Style;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoWing::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_LengthMM;
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoWing::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_LengthMM;
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoWing::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoWing::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoWing::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

AuAnimalPart* AuChromoWing::MakePart(const AuChromatid& dna, quint8& firstBone) const
{
	return new AuAnimalWing(*this, dna, firstBone);
}

AuPoint AuChromoWing::BoneLoc(const AuChromatid& dna, quint8 bone) const
{
	//DHH
	return AuPoint(0,0);
}

int AuChromoWing::ListWidgetOrder() const
{
	return 800 + m_Which.Index();
}

QString AuChromoWing::ListWidgetText() const
{
	return QObject::tr("Wing %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoWing::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageWing* pp = new AuChromoPageWing(parent);
	return pp;
}

bool AuChromoWing::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoWing::Laterality(const AuChromoWhich& which)
{
	return which.Index() & 1 ? Right : Left;
}

bool AuChromoWing::CanFly() const
{
	return true;	//DHH animalia
}

bool AuChromoWing::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoWing* c = (const AuChromoWing*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Style != c->m_Style)
		return false;
	return true;
}

AuChromosome* AuChromoWing::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoWing* c = dynamic_cast<const AuChromoWing*>(g);
	if(!c)
		return 0;
	AuChromoWing* comb = new AuChromoWing(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoAntenna::AuChromoAntenna(const AuChromoWhich& which, AuChromoAntenna::STYLE style, quint16 lengthMM)
{
	m_LengthMM = lengthMM;
	m_Style = style;
	m_Which = which;
}

AuChromoAntenna::AuChromoAntenna()
{
	m_LengthMM = 100;
	m_Style = Simple;
}

AuChromoAntenna::AuChromoAntenna(const AuChromoAntenna& c) : AuChromosome(c)
{
	m_LengthMM = c.m_LengthMM;
	m_Style = c.m_Style;
	m_Which = c.m_Which;
}

AuChromoAntenna::AuChromoAntenna(const AuChromoAntenna& c1, const AuChromoAntenna& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoAntenna which mismatch" << c1.m_Which << c2.m_Which;
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	m_Style = c1.m_Style;
	m_Which = c1.m_Which;
}

void AuChromoAntenna::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoAntenna& c = dynamic_cast<const AuChromoAntenna&>(o);
		m_LengthMM = c.m_LengthMM;
		m_Style = c.m_Style;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoAntenna::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_LengthMM;
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoAntenna::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_LengthMM;
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoAntenna::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoAntenna::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoAntenna::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoAntenna::ListWidgetOrder() const
{
	return 110 + m_Which.Index();	// Head is 100
}

QString AuChromoAntenna::ListWidgetText() const
{
	return QObject::tr("Antenna %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoAntenna::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageAntenna* pp = new AuChromoPageAntenna(parent);
	return pp;
}

bool AuChromoAntenna::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoAntenna::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of horns.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

bool AuChromoAntenna::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoAntenna* c = (const AuChromoAntenna*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Style != c->m_Style)
		return false;
	return true;
}

AuChromosome* AuChromoAntenna::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoAntenna* c = dynamic_cast<const AuChromoAntenna*>(g);
	if(!c)
		return 0;
	AuChromoAntenna* comb = new AuChromoAntenna(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoHorn::AuChromoHorn(const AuChromoWhich& which, AuChromoHorn::STYLE style)
{
	m_Style = style;
	m_Which = which;
}

AuChromoHorn::AuChromoHorn()
{
	m_Style = Cow;
}

AuChromoHorn::AuChromoHorn(const AuChromoHorn& c) : AuChromosome(c)
{
	m_Style = c.m_Style;
	m_Which = c.m_Which;
}

AuChromoHorn::AuChromoHorn(const AuChromoHorn& c1, const AuChromoHorn& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoHorn which mismatch" << c1.m_Which << c2.m_Which;
	m_Style = c1.m_Style;
	m_Which = c1.m_Which;
}

void AuChromoHorn::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoHorn& c = dynamic_cast<const AuChromoHorn&>(o);
		m_Style = c.m_Style;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoHorn::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoHorn::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoHorn::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_Style;
	out << m_Which;
}

void AuChromoHorn::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
}

void AuChromoHorn::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoHorn::ListWidgetOrder() const
{
	return 120 + m_Which.Index();
}

QString AuChromoHorn::ListWidgetText() const
{
	return QObject::tr("Horn %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoHorn::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageHorn* pp = new AuChromoPageHorn(parent);
	return pp;
}

bool AuChromoHorn::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoHorn::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of horns.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

bool AuChromoHorn::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoHorn* c = (const AuChromoHorn*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Style != c->m_Style)
		return false;
	return true;
}

AuChromosome* AuChromoHorn::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoHorn* c = dynamic_cast<const AuChromoHorn*>(g);
	if(!c)
		return 0;
	AuChromoHorn* comb = new AuChromoHorn(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEar::AuChromoEar(const AuChromoWhich& which, AuChromoEar::STYLE style, quint16 diameterMM, qint16 angleDeg)
{
	m_AngleDeg = angleDeg;
	m_DiameterMM = diameterMM;
	m_Style = style;
	m_Which = which;
}

AuChromoEar::AuChromoEar()
{
	m_AngleDeg = 90;
	m_DiameterMM = 100;
	m_Style = Circle;
}

AuChromoEar::AuChromoEar(const AuChromoEar& c) : AuChromosomeWithGenes(c)
{
	m_AngleDeg = c.m_AngleDeg;
	m_DiameterMM = c.m_DiameterMM;
	m_Style = c.m_Style;
	m_Which = c.m_Which;
}

AuChromoEar::AuChromoEar(const AuChromoEar& c1, const AuChromoEar& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoEar which mismatch" << c1.m_Which << c2.m_Which;
	m_AngleDeg = CombineAngle(c1.m_AngleDeg, c2.m_AngleDeg, dice);
	dice.Random();
	m_DiameterMM = CombineRange(c1.m_DiameterMM, c2.m_DiameterMM, dice);
	m_Style = c1.m_Style;
	m_Which = c1.m_Which;
}

void AuChromoEar::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoEar& c = dynamic_cast<const AuChromoEar&>(o);
		AuChromosomeWithGenes::Copy(c);
		m_AngleDeg = c.m_AngleDeg;
		m_DiameterMM = c.m_DiameterMM;
		m_Style = c.m_Style;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoEar::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << (quint8)m_Style;
	out << m_Which;
	out << m_AngleDeg;
	out << m_DiameterMM;
}

void AuChromoEar::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
	in >> m_AngleDeg;
	in >> m_DiameterMM;
}

void AuChromoEar::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << (quint8)m_Style;
	out << m_Which;
	out << ToAngleByte(m_AngleDeg);
	out << ToLogByte(m_DiameterMM, d_DNAMaxMM);
}

void AuChromoEar::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
	in >> m_Which;
	m_AngleDeg = ReadAngleByte(in);
	m_DiameterMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoEar::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_DiameterMM = dice.RandomLogValue(m_DiameterMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

void AuChromoEar::GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	AuPoint locOnSkull(head.EarLocation(m_Which));
	LATERALITY laterality = Laterality();

	double size = DiameterMM() * FeetPerMM();
	double radius = size / 2;
	double thickness = radius / 5;
	AuPointArray eprofile;
	eprofile.append(AuPoint(-thickness,0));
	eprofile.append(AuPoint(-thickness,-radius));
	eprofile.append(AuPoint(0,-radius));
	eprofile.append(AuPoint(0,-radius+thickness));
	eprofile.append(AuPoint(-thickness*3/4,0));

	const float triangleSize = .3f/12.f;
	double circ = size * PI;
	int steps = ceil(circ / triangleSize);
	if(steps < 3)
		steps = 3;
	AuScFaceList ear;
	ear.MakeSpindle(eprofile, steps, AuMatrix4x4());

	AuMatrix4x4 mEar(mSkull);
	mEar.Translate(locOnSkull);
	double ang = (double)m_AngleDeg * DEG2RAD;
	if(laterality == Left)
		mEar.RotateZ(PI - ang);
	else
		mEar.RotateZ(ang);
	ear.Transform(mEar);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 2;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(ear, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, size, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

void AuChromoEar::GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	AuPoint locOnSkull(head.EarLocation(m_Which));
	LATERALITY laterality = Laterality();

	double size = DiameterMM() * FeetPerMM();
	double radius = size / 2;
	double thickness = radius / 5;
	AuPointArray eprofile;
	eprofile.append(AuPoint(-thickness,0));
	eprofile.append(AuPoint(-thickness,-radius));
	eprofile.append(AuPoint(0,-radius));
	eprofile.append(AuPoint(0,-radius+thickness));
	eprofile.append(AuPoint(-thickness*3/4,0));

	const float triangleSize = .3f/12.f * d_ClothingTessFactor;
	double circ = size * PI;
	int steps = ceil(circ / triangleSize);
	if(steps < 3)
		steps = 3;
	AuScFaceList ear;
	ear.MakeSpindle(eprofile, steps, AuMatrix4x4());

	AuMatrix4x4 mEar(mSkull);
	mEar.Translate(locOnSkull);
	double ang = (double)m_AngleDeg * DEG2RAD;
	if(laterality == Left)
		mEar.RotateZ(PI - ang);
	else
		mEar.RotateZ(ang);
	ear.Transform(mEar);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 2;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(ear, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, size, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

void AuChromoEar::GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles,
		QMap<AuPoint, double>& hairEx, const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
{
	if(stage == Juvenile)
		return;
	Q_UNUSED(lockWid);
	AuFolliclePoint::ROLE role;
	switch(Laterality())
	{
	case 0:	// Left
		role = AuFolliclePoint::EarLeft;
		break;
	case 1:	// Middle
	default:
		role = AuFolliclePoint::EarMiddle;
		break;
	case 2:	// Right
		role = AuFolliclePoint::EarRight;
		break;
	}
	const AuGeneHair* hdna = HairDNA(role);
	if(hdna)
	{
		AuMeatMesh mesh;
		GetSkullFeatures(dna, mesh, mSkull, head);
		AuPointNArray hairpts;
		mesh.SurfacePoints(hairpts);
		const QColor& hairColor = hdna->Color();
		float maxLen = hdna->LengthMM() * FeetPerMM();
		for(AuPointNArray::const_iterator fi = hairpts.cbegin(); fi != hairpts.cend(); fi++)
			follicles.append(AuFolliclePoint(0, role, *fi, hairColor, maxLen));
	}
	else
	{
		// No hair so exclude my area.
		AuPoint locOnSkull(head.EarLocation(m_Which));
		double exRadius = DiameterMM() * FeetPerMM() * 1.1 / 2;
		hairEx.insert(mSkull.Project(locOnSkull), exRadius);
	}
}

// Called on a default constructed object.
quint64 AuChromoEar::HairWhereBits(const AuChromoWhich& which) const
{
	switch(Laterality(which))
	{
	case 0:	// Left
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EarLeft);
	case 1:	// Middle
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EarMiddle);
	case 2:	// Right
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EarRight);
	}
	return AuFolliclePoint::RoleBit(AuFolliclePoint::EarLeft);
}

int AuChromoEar::ListWidgetOrder() const
{
	return 140 + m_Which.Index();
}

QString AuChromoEar::ListWidgetText() const
{
	return QObject::tr("Ear %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoEar::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageEar* pp = new AuChromoPageEar(parent);
	return pp;
}

bool AuChromoEar::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

//static
AuChromosome::LATERALITY AuChromoEar::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of ears.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

float AuChromoEar::Volume() const
{
	float r = Radius();
	float thickness = r / 5;
	// Volume of cylinder
	// V = PI * r^2 * h
	float v = PIf * r * r * thickness;
	return v;
}

bool AuChromoEar::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoEar* c = (const AuChromoEar*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	return true;
}

AuChromosome* AuChromoEar::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoEar* c = dynamic_cast<const AuChromoEar*>(g);
	if(!c)
		return 0;
	AuChromoEar* comb = new AuChromoEar(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEye::AuChromoEye(const AuChromoWhich& which, FACING facing, MEAT_DECAL decal, quint16 eyeDiaMM, quint16 socketDiaMM)
{
	m_Decal = decal;
	m_EyeDiaMM = eyeDiaMM;
	m_Facing = facing;
	m_SocketDiaMM = socketDiaMM;
	m_Which = which;
}

AuChromoEye::AuChromoEye()
{
	m_Decal = MD_HumanEye1;
	m_EyeDiaMM = 24;
	m_Facing = Forward;
	m_SocketDiaMM = 40;
}

AuChromoEye::AuChromoEye(const AuChromoEye& c) : AuChromosomeWithGenes(c)
{
	m_Decal = c.m_Decal;
	m_EyeDiaMM = c.m_EyeDiaMM;
	m_Facing = c.m_Facing;
	m_SocketDiaMM = c.m_SocketDiaMM;
	m_Which = c.m_Which;
}

AuChromoEye::AuChromoEye(const AuChromoEye& c1, const AuChromoEye& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoEye which mismatch" << c1.m_Which << c2.m_Which;
	m_Decal = (MEAT_DECAL)CombineEnum(c1.m_Decal, c2.m_Decal, dice);
	dice.Random();
	m_EyeDiaMM = CombineRange(c1.m_EyeDiaMM, c2.m_EyeDiaMM, dice);
	dice.Random();
	m_Facing = (FACING)CombineEnum(c1.m_Facing, c2.m_Facing, dice);
	dice.Random();
	m_SocketDiaMM = CombineRange(c1.m_SocketDiaMM, c2.m_SocketDiaMM, dice);
	m_Which = c1.m_Which;
}

void AuChromoEye::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoEye& c = dynamic_cast<const AuChromoEye&>(o);
		AuChromosomeWithGenes::Copy(c);
		m_Decal = c.m_Decal;
		m_EyeDiaMM = c.m_EyeDiaMM;
		m_Facing = c.m_Facing;
		m_SocketDiaMM = c.m_SocketDiaMM;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoEye::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << (quint16)m_Decal;
	out << m_EyeDiaMM;
	out << (quint8)m_Facing;
	out << m_SocketDiaMM;
	out << m_Which;
}

void AuChromoEye::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	quint16 ui16;
	in >> ui16;
	m_Decal = (MEAT_DECAL)ui16;
	in >> m_EyeDiaMM;
	quint8 ui8;
	in >> ui8;
	m_Facing = (FACING)ui8;
	in >> m_SocketDiaMM;
	in >> m_Which;
}

void AuChromoEye::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << (quint16)m_Decal;
	out << ToLogByte(m_EyeDiaMM, d_DNAMaxMM);
	out << (quint8)m_Facing;
	out << ToLogByte(m_SocketDiaMM, d_DNAMaxMM);
	out << m_Which;
}

void AuChromoEye::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	quint16 ui16;
	in >> ui16;
	m_Decal = (MEAT_DECAL)ui16;
	m_EyeDiaMM = ReadLogByte(in, d_DNAMaxMM);
	quint8 ui8;
	in >> ui8;
	m_Facing = (FACING)ui8;
	m_SocketDiaMM = ReadLogByte(in, d_DNAMaxMM);
	in >> m_Which;
}

void AuChromoEye::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_EyeDiaMM = dice.RandomLogValue(m_EyeDiaMM, d_DNAMaxMM);
	m_SocketDiaMM = dice.RandomLogValue(m_SocketDiaMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

void AuChromoEye::GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	double socketRad = SocketDiameterMM() * FeetPerMM() / 2;
	double socketLen = socketRad;
	double eyeRad = EyeDiameterMM() * FeetPerMM() / 2;
	AuPoint locOnSkull(head.EyeLocation(m_Which, eyeRad));

	AuSolid solid;

	AuMatrix4x4 mEye(mSkull);
	mEye.Translate(locOnSkull);
	switch(m_Facing)
	{
	case Forward:
		mEye.RotateX(PI/2);
		break;
	case Side:
		switch(Laterality())
		{
		case 0:	// Left
			mEye.RotateZ(PI/2);	// Left
			break;
		case 1:	// Middle
			break;
		case 2:	// Right
			mEye.RotateZ(-PI/2);
			break;
		}
		mEye.RotateX(PI/2);
		break;
	case Away:
		mEye *= AuPoint(0,0,-1).GetAlignmentTransform(locOnSkull);
		break;
	}
	mEye.Translate(0,0,-eyeRad/2);		// Eye ball sunken into socket.
	AuSolidCylindomoid socket(mEye, eyeRad/2, socketRad, socketRad, socketLen, true);
	solid.Add(socket);

	AuScFaceList faces;
	const float triangleSize = .25f/12.f;
	solid.Faces(faces, triangleSize);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, socketRad, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

void AuChromoEye::GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	double socketRad = SocketDiameterMM() * FeetPerMM() / 2;
	double socketLen = socketRad;
	double eyeRad = EyeDiameterMM() * FeetPerMM() / 2;
	AuPoint locOnSkull(head.EyeLocation(m_Which, eyeRad));

	AuSolid solid;

	AuMatrix4x4 mEye(mSkull);
	mEye.Translate(locOnSkull);
	switch(m_Facing)
	{
	case Forward:
		mEye.RotateX(PI/2);
		break;
	case Side:
		switch(Laterality())
		{
		case 0:	// Left
			mEye.RotateZ(PI/2);	// Left
			break;
		case 1:	// Middle
			break;
		case 2:	// Right
			mEye.RotateZ(-PI/2);
			break;
		}
		mEye.RotateX(PI/2);
		break;
	case Away:
		mEye *= AuPoint(0,0,-1).GetAlignmentTransform(locOnSkull);
		break;
	}
	mEye.Translate(0,0,-eyeRad/2);		// Eye ball sunken into socket.
	AuSolidCylindomoid socket(mEye, eyeRad/2, socketRad, socketRad, socketLen, true);
	solid.Add(socket);

	AuScFaceList faces;
	const float triangleSize = .25f/12.f * d_ClothingTessFactor;
	solid.Faces(faces, triangleSize);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, socketRad, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

bool AuChromoEye::GetSkullEye(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
		quint8 firstBone) const
{
	Q_UNUSED(dna);
	double eyeRad = EyeDiameterMM() * FeetPerMM() / 2;
	AuPoint locOnSkull(head.EyeLocation(m_Which, eyeRad));

	AuMatrix4x4 mEye(mSkull);
	mEye.Translate(locOnSkull);
	switch(m_Facing)
	{
	case Forward:
		mEye.RotateZ(PI/2);
		break;
	case Side:
		switch(Laterality())
		{
		case 0:	// Left
			mEye.RotateZ(PI);	// Left
			break;
		case 1:	// Middle
			mEye.RotateZ(PI/2);	// Left
			break;
		case 2:	// Right
			break;
		}
		break;
	case Away:
		mEye *= AuPoint(1,0,0).GetAlignmentTransform(locOnSkull);
		break;
	}
	//mEye.Translate(-eyeRad/2,0,0);		// Eye ball sunken into socket.

	const float triangleSize = .2f/12.f;

	float s, t, u, v;
	AuMeat::DecalTexture(m_Decal, s, t, u, v);

	AuMeatMesh mymesh;
	mymesh.Sphere(AuMeatTri::EyeBall, triangleSize, eyeRad, s,t,u,v, firstBone);
	mymesh.Transform(mEye);
	mesh.Append(mymesh);
	return true;
}

void AuChromoEye::GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles,
		QMap<AuPoint, double>& hairEx, const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
{
	if(stage == Juvenile)
		return;
	Q_UNUSED(lockWid);
	AuFolliclePoint::ROLE role;
	switch(Laterality())
	{
	case 0:	// Left
		role = AuFolliclePoint::EyeLeft;
		break;
	case 1:	// Middle
	default:
		role = AuFolliclePoint::EyeMiddle;
		break;
	case 2:	// Right
		role = AuFolliclePoint::EyeRight;
		break;
	}
	const AuGeneHair* hdna = HairDNA(role);
	if(hdna)
	{
		AuMeatMesh mesh;
		GetSkullFeatures(dna, mesh, mSkull, head);
		AuPointNArray hairpts;
		mesh.SurfacePoints(hairpts);
		const QColor& hairColor = hdna->Color();
		float maxLen = hdna->LengthMM() * FeetPerMM();
		for(AuPointNArray::const_iterator fi = hairpts.cbegin(); fi != hairpts.cend(); fi++)
			follicles.append(AuFolliclePoint(0, role, *fi, hairColor, maxLen));
	}
	else
	{
		// No hair so exclude my area.
		double exRadius = EyeDiameterMM() * FeetPerMM();
		AuPoint locOnSkull(head.EyeLocation(m_Which, exRadius/2));
		hairEx.insert(mSkull.Project(locOnSkull), exRadius);
	}
}

// Called on a default constructed object.
quint64 AuChromoEye::HairWhereBits(const AuChromoWhich& which) const
{
	switch(Laterality(which))
	{
	case 0:	// Left
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EyeLeft);
	case 1:	// Middle
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EyeMiddle);
	case 2:	// Right
		return AuFolliclePoint::RoleBit(AuFolliclePoint::EyeRight);
	}
	return AuFolliclePoint::RoleBit(AuFolliclePoint::EyeLeft);
}

int AuChromoEye::ListWidgetOrder() const
{
	return 130 + m_Which.Index();
}

QString AuChromoEye::ListWidgetText() const
{
	return QObject::tr("Eye %1<br><small>%3</small>")
		.arg(m_Which.Index()+1)
		.arg(LateralityName(Laterality()));
}

QWidget* AuChromoEye::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageEye* pp = new AuChromoPageEye(parent);
	return pp;
}

bool AuChromoEye::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try the next previous of the same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

float AuChromoEye::FaceRadiusY(const AuChromoHead& head) const
{
	double eyeRad = EyeDiameterMM() * FeetPerMM() / 2;
	AuPoint locOnSkull(head.EyeLocation(m_Which, eyeRad));
	return locOnSkull.y + eyeRad;
}

//static
AuChromosome::LATERALITY AuChromoEye::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of eyes.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

bool AuChromoEye::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoEye* c = (const AuChromoEye*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Facing != c->m_Facing)
		return false;
	return true;
}

AuChromosome* AuChromoEye::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoEye* c = dynamic_cast<const AuChromoEye*>(g);
	if(!c)
		return 0;
	AuChromoEye* comb = new AuChromoEye(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoNose::AuChromoNose(quint16 lengthMM, quint16 widthHeadMM, quint16 widthTipMM, qint16 angleDeg)
{
	m_AngleDeg = angleDeg;
	m_LengthMM = lengthMM;
	m_WidthHeadMM = widthHeadMM;
	m_WidthTipMM = widthTipMM;
}

AuChromoNose::AuChromoNose()
{
	m_AngleDeg = 0;
	m_LengthMM = 40;
	m_WidthHeadMM = 20;
	m_WidthTipMM = 20;
}

AuChromoNose::AuChromoNose(const AuChromoNose& c) : AuChromosomeWithGenes(c)
{
	m_AngleDeg = c.m_AngleDeg;
	m_LengthMM = c.m_LengthMM;
	m_WidthHeadMM = c.m_WidthHeadMM;
	m_WidthTipMM = c.m_WidthTipMM;
}

AuChromoNose::AuChromoNose(const AuChromoNose& c1, const AuChromoNose& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_AngleDeg = CombineAngle(c1.m_AngleDeg, c2.m_AngleDeg, dice);
	dice.Random();
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	dice.Random();
	m_WidthHeadMM = CombineRange(c1.m_WidthHeadMM, c2.m_WidthHeadMM, dice);
	dice.Random();
	m_WidthTipMM = CombineRange(c1.m_WidthTipMM, c2.m_WidthTipMM, dice);
}

void AuChromoNose::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoNose& c = dynamic_cast<const AuChromoNose&>(o);
		AuChromosomeWithGenes::Copy(c);
		m_AngleDeg = c.m_AngleDeg;
		m_LengthMM = c.m_LengthMM;
		m_WidthHeadMM = c.m_WidthHeadMM;
		m_WidthTipMM = c.m_WidthTipMM;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoNose::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_AngleDeg;
	out << m_LengthMM;
	out << m_WidthHeadMM;
	out << m_WidthTipMM;
}

void AuChromoNose::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_AngleDeg;
	in >> m_LengthMM;
	in >> m_WidthHeadMM;
	in >> m_WidthTipMM;
}

void AuChromoNose::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToAngleByte(m_AngleDeg);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << ToLogByte(m_WidthHeadMM, d_DNAMaxMM);
	out << ToLogByte(m_WidthTipMM, d_DNAMaxMM);
}

void AuChromoNose::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_AngleDeg = ReadAngleByte(in);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_WidthHeadMM = ReadLogByte(in, d_DNAMaxMM);
	m_WidthTipMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoNose::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	m_WidthHeadMM = dice.RandomLogValue(m_WidthHeadMM, d_DNAMaxMM);
	m_WidthTipMM = dice.RandomLogValue(m_WidthTipMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

void AuChromoNose::GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	double noseLength = LengthMM() * FeetPerMM();
	double radHead = WidthHeadMM() * FeetPerMM() / 2;
	double radTip = WidthTipMM() * FeetPerMM() / 2;
	double angle = AngleDeg() * DEG2RAD;

	AuPoint noseLoc(head.NoseLocation());
	noseLoc.y -= radHead * 2 * fabs(sin(angle));

	AuSolid solid;

	AuMatrix4x4 tnose(mSkull);
	tnose.Translate(noseLoc);
	tnose.RotateX(-PI/2 + angle);
	AuSolidCone solNose(tnose, noseLength, radHead, radTip, false, false);
	solid.Add(solNose);
	AuMatrix4x4 tnosetip(tnose);
	tnosetip.Translate(0,0,noseLength);
	AuSolidDome solNoseTip(tnosetip, radTip, false);
	solid.Add(solNoseTip);

	AuScFaceList faces;
	const float triangleSize = .3f/12.f;
	solid.Faces(faces, triangleSize);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	float texSize = (radHead > radTip) ? radHead : radTip;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, texSize, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

void AuChromoNose::GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	double noseLength = LengthMM() * FeetPerMM();
	double radHead = WidthHeadMM() * FeetPerMM() / 2;
	double radTip = WidthTipMM() * FeetPerMM() / 2;
	double angle = AngleDeg() * DEG2RAD;

	AuPoint noseLoc(head.NoseLocation());
	noseLoc.y -= radHead * 2 * fabs(sin(angle));

	AuSolid solid;

	AuMatrix4x4 tnose(mSkull);
	tnose.Translate(noseLoc);
	tnose.RotateX(-PI/2 + angle);
	AuSolidCone solNose(tnose, noseLength, radHead, radTip, false, false);
	solid.Add(solNose);
	AuMatrix4x4 tnosetip(tnose);
	tnosetip.Translate(0,0,noseLength);
	AuSolidDome solNoseTip(tnosetip, radTip, false);
	solid.Add(solNoseTip);

	AuScFaceList faces;
	const float triangleSize = .3f/12.f * d_ClothingTessFactor;
	solid.Faces(faces, triangleSize);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	float texSize = (radHead > radTip) ? radHead : radTip;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, texSize, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

void AuChromoNose::GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles,
		QMap<AuPoint, double>& hairEx, const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
{
	if(stage == Juvenile)
		return;
	Q_UNUSED(dna);
	const float follicleSpacing = lockWid * .5f;

	Q_UNUSED(hairEx);
	AuFolliclePoint::ROLE role = AuFolliclePoint::Nose;
	const AuGeneHair* hdna = HairDNA(role);
	if(!hdna)
		return;

	double noseLength = LengthMM() * FeetPerMM();
	double radHead = WidthHeadMM() * FeetPerMM() / 2;
	double radTip = WidthTipMM() * FeetPerMM() / 2;
	double angle = AngleDeg() * DEG2RAD;

	AuPoint noseLoc(head.NoseLocation());
	noseLoc.y -= radHead * 2 * fabs(sin(angle));

	AuSolid solid;

	AuMatrix4x4 tnose(mSkull);
	tnose.Translate(noseLoc);
	tnose.RotateX(-PI/2 + angle);
	AuSolidCone solNose(tnose, noseLength, radHead, radTip, false, false);
	solid.Add(solNose);
	AuMatrix4x4 tnosetip(tnose);
	tnosetip.Translate(0,0,noseLength);
	AuSolidDome solNoseTip(tnosetip, radTip, false);
	solid.Add(solNoseTip);

	AuPointNArray myfol;
	solid.SurfacePoints(myfol, follicleSpacing);

	const QColor& hairColor = hdna->Color();
	float maxLen = hdna->LengthMM() * FeetPerMM();
	AuPointNArray::iterator fi = myfol.begin();
	while(fi != myfol.end())
	{
		follicles.append(AuFolliclePoint(0, role, *fi, hairColor, maxLen));
		fi++;
	}
}

// Called on a default constructed object.
quint64 AuChromoNose::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::Nose);
}

int AuChromoNose::ListWidgetOrder() const
{
	return 150;
}

QString AuChromoNose::ListWidgetText() const
{
	return QObject::tr("Nose");
}

QWidget* AuChromoNose::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageNose* pp = new AuChromoPageNose(parent);
	return pp;
}

float AuChromoNose::FaceRadiusY(const AuChromoHead& head) const
{
	double noseLength = LengthMM() * FeetPerMM();
	double radHead = WidthHeadMM() * FeetPerMM() / 2;
	double radTip = WidthTipMM() * FeetPerMM() / 2;
	double angle = AngleDeg() * DEG2RAD;
	AuPoint locOnSkull(head.NoseLocation());
	locOnSkull.y -= radHead * 2 * fabs(sin(angle));
	return locOnSkull.y + noseLength + radTip;
}

float AuChromoNose::Volume() const
{
	double noseLen = LengthMM() * FeetPerMM();
	double headRad = WidthHeadMM() * FeetPerMM() / 2;
	float v;
	if(m_WidthHeadMM == m_WidthTipMM)
	{
		// Nose is a cylinder.
		// Volume of cylinder
		// V = PI * r^2 * h
		v = PIf * headRad * headRad * noseLen;
	}
	else
	{
		// Nose is a truncated cone.
		// Volume of truncated cone.
		double tipRad = WidthTipMM() * FeetPerMM() / 2;
		float h;	// Height extended to full cone.
		float r;
		if(headRad > tipRad)
		{
			float s = (headRad - tipRad) / noseLen;	// Slope of the side
			r = headRad;
			h = r / s;
		}
		else
		{
			float s = (tipRad - headRad) / noseLen;	// Slope of the side
			r = tipRad;
			h = r / s;
		}
		// Volume of cone.
		// V = PI * r^2 * h / 3
		float vc = PIf * r * r * h / 3;				// Volume of full cone.
		float vt = PIf * r * r * (h-noseLen) / 3;	// Volume of tip cone.
		v = (vc - vt);								// Volume of truncated cone is full cone minus the tip.
	}
	return v;
}

bool AuChromoNose::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoNose* c = (const AuChromoNose*)chromosome;
	return true;
}

AuChromosome* AuChromoNose::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoNose* c = dynamic_cast<const AuChromoNose*>(g);
	if(!c)
		return 0;
	AuChromoNose* comb = new AuChromoNose(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoMouth::AuChromoMouth(quint16 lengthMM, quint16 widthMM, quint16 heightMM, quint16 depthMM, quint16 lipThicknessMM,
		TEETH teeth, quint16 teethHtBottomMM, quint16 teethHtTopMM)
{
	m_DepthMM = depthMM;
	m_HeightMM = heightMM;
	m_LengthMM = lengthMM;
	m_LipThicknessMM = lipThicknessMM;
	m_Teeth = teeth;
	m_TeethHeightBottomMM = teethHtBottomMM;
	m_TeethHeightTopMM = teethHtTopMM;
	m_WidthMM = widthMM;
}

AuChromoMouth::AuChromoMouth()
{
	m_DepthMM = 30;
	m_HeightMM = 30;
	m_LengthMM = 40;
	m_LipThicknessMM = 1;
	m_Teeth = Fangs;
	m_TeethHeightBottomMM = 10;
	m_TeethHeightTopMM = 10;
	m_WidthMM = 40;
}

AuChromoMouth::AuChromoMouth(const AuChromoMouth& c) : AuChromosomeWithGenes(c)
{
	m_DepthMM = c.m_DepthMM;
	m_HeightMM = c.m_HeightMM;
	m_LengthMM = c.m_LengthMM;
	m_LipThicknessMM = c.m_LipThicknessMM;
	m_Teeth = c.m_Teeth;
	m_TeethHeightBottomMM = c.m_TeethHeightBottomMM;
	m_TeethHeightTopMM = c.m_TeethHeightTopMM;
	m_WidthMM = c.m_WidthMM;
}

AuChromoMouth::AuChromoMouth(const AuChromoMouth& c1, const AuChromoMouth& c2, const AuDice& Cdice) :
	AuChromosomeWithGenes(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_DepthMM = CombineRange(c1.m_DepthMM, c2.m_DepthMM, dice);
	dice.Random();
	m_HeightMM = CombineRange(c1.m_HeightMM, c2.m_HeightMM, dice);
	dice.Random();
	m_LengthMM = CombineRange(c1.m_LengthMM, c2.m_LengthMM, dice);
	dice.Random();
	m_LipThicknessMM = CombineRange(c1.m_LipThicknessMM, c2.m_LipThicknessMM, dice);
	dice.Random();
	m_Teeth = (TEETH)CombineEnum(c1.m_Teeth, c2.m_Teeth, dice);
	dice.Random();
	m_TeethHeightBottomMM = CombineRange(c1.m_TeethHeightBottomMM, c2.m_TeethHeightBottomMM, dice);
	dice.Random();
	m_TeethHeightTopMM = CombineRange(c1.m_TeethHeightTopMM, c2.m_TeethHeightTopMM, dice);
	dice.Random();
	m_WidthMM = CombineRange(c1.m_WidthMM, c2.m_WidthMM, dice);
}

void AuChromoMouth::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoMouth& c = dynamic_cast<const AuChromoMouth&>(o);
		AuChromosomeWithGenes::Copy(c);
		m_DepthMM = c.m_DepthMM;
		m_HeightMM = c.m_HeightMM;
		m_LengthMM = c.m_LengthMM;
		m_LipThicknessMM = c.m_LipThicknessMM;
		m_Teeth = c.m_Teeth;
		m_TeethHeightBottomMM = c.m_TeethHeightBottomMM;
		m_TeethHeightTopMM = c.m_TeethHeightTopMM;
		m_WidthMM = c.m_WidthMM;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoMouth::operator>>(QDataStream& out) const
{
	AuChromosomeWithGenes::operator>>(out);
	out << m_DepthMM;
	out << m_HeightMM;
	out << m_LengthMM;
	out << m_LipThicknessMM;
	out << (quint8)m_Teeth;
	out << m_TeethHeightBottomMM;
	out << m_TeethHeightTopMM;
	out << m_WidthMM;
}

void AuChromoMouth::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadStream(in, fileVer);
	in >> m_DepthMM;
	in >> m_HeightMM;
	in >> m_LengthMM;
	in >> m_LipThicknessMM;
	quint8 ui8;
	in >> ui8;
	m_Teeth = (TEETH)ui8;
	in >> m_TeethHeightBottomMM;
	in >> m_TeethHeightTopMM;
	in >> m_WidthMM;
}

void AuChromoMouth::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosomeWithGenes::WriteSpeciesKey(out);
	out << ToLogByte(m_DepthMM, d_DNAMaxMM);
	out << ToLogByte(m_HeightMM, d_DNAMaxMM);
	out << ToLogByte(m_LengthMM, d_DNAMaxMM);
	out << ToLogByte(m_LipThicknessMM, d_DNAMaxMM);
	out << (quint8)m_Teeth;
	out << ToLogByte(m_TeethHeightBottomMM, d_DNAMaxMM);
	out << ToLogByte(m_TeethHeightTopMM, d_DNAMaxMM);
	out << ToLogByte(m_WidthMM, d_DNAMaxMM);
}

void AuChromoMouth::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosomeWithGenes::ReadSpeciesKey(in, fileVer);
	m_DepthMM = ReadLogByte(in, d_DNAMaxMM);
	m_HeightMM = ReadLogByte(in, d_DNAMaxMM);
	m_LengthMM = ReadLogByte(in, d_DNAMaxMM);
	m_LipThicknessMM = ReadLogByte(in, d_DNAMaxMM);
	quint8 ui8;
	in >> ui8;
	m_Teeth = (TEETH)ui8;
	m_TeethHeightBottomMM = ReadLogByte(in, d_DNAMaxMM);
	m_TeethHeightTopMM = ReadLogByte(in, d_DNAMaxMM);
	m_WidthMM = ReadLogByte(in, d_DNAMaxMM);
}

void AuChromoMouth::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_DepthMM = dice.RandomLogValue(m_DepthMM, d_DNAMaxMM);
	m_HeightMM = dice.RandomLogValue(m_HeightMM, d_DNAMaxMM);
	m_LengthMM = dice.RandomLogValue(m_LengthMM, d_DNAMaxMM);
	m_LipThicknessMM = dice.RandomLogValue(m_LipThicknessMM, d_DNAMaxMM);
	m_TeethHeightBottomMM = dice.RandomLogValue(m_TeethHeightBottomMM, d_DNAMaxMM);
	m_TeethHeightTopMM = dice.RandomLogValue(m_TeethHeightTopMM, d_DNAMaxMM);
	AuChromosomeWithGenes::RandomizeSpecies(dice);
}

void AuChromoMouth::GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	AuPoint mouthLoc(head.MouthLocation());
	double headHt = head.HeightMM() * FeetPerMM();
	double jawRadX = m_WidthMM * FeetPerMM() / 2;
	double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out. / 2;
	double jawHt = m_HeightMM * FeetPerMM();
	double lipThickness = m_LipThicknessMM * FeetPerMM();

	AuSolid solid;

	AuMatrix4x4 tj(mSkull);
	tj.Translate(mouthLoc);

	AuSolidDomoid upperJaw(AuMatrix4x4(), jawRadX, jawRadX, jawHt, true);
	solid.Add(upperJaw);

	AuScFaceList faces;
	const float triangleSize = .5f/12.f;
	solid.Faces(faces, triangleSize);
	faces.ScalePositiveY(jawRadY/jawRadX);	// Stretch only the front part of the domoid.
	faces.Transform(tj);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::JawUpper);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, headHt, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);

	// Teeth in upper jaw
	if(m_Teeth)
	{
		double molarHt = 2 * FeetPerMM();
		if(m_Teeth == Flat)
			molarHt = m_TeethHeightTopMM * FeetPerMM();
		else
			molarHt = 2 * FeetPerMM();
		double outerRadius = jawRadX - lipThickness;
		int teethInCircle = 32;
		double circ = PI * outerRadius * 2;
		double toothWid = circ / teethInCircle;
		double teethThickness = toothWid;
		double innerRadius = outerRadius - teethThickness;
		double bevel = teethThickness/4;
		AuPointArray tprofile;
		tprofile.append(AuPoint(0,-innerRadius));
		tprofile.append(AuPoint(0,-outerRadius));
		tprofile.append(AuPoint(molarHt,-outerRadius + bevel));
		tprofile.append(AuPoint(molarHt,-innerRadius - bevel));
		tprofile.append(AuPoint(0,-innerRadius));

		AuScFaceList teeth;
		teeth.MakeSpindle(tprofile, teethInCircle, AuMatrix4x4());
		teeth.ScalePositiveY(jawRadY/jawRadX);
		tj.RotateY(PI/2);
		teeth.Transform(tj);
		teeth.FlattenVertexNormals();

		if(m_Teeth == Fangs)
		{
			// Upper fangs.
			double yxscale = jawRadY/jawRadX;
			double fangHt = m_TeethHeightTopMM * FeetPerMM();
			double fangAtRad = jawRadX - lipThickness - teethThickness/2;
			double fangRadBaseX = teethThickness * .6;
			double fangRadBaseY = fangRadBaseX * yxscale;
			double fangRadTipX = fangRadBaseX / 4;
			double fangRadTipY = fangRadBaseY / 4;
			double fangang = 60 * DEG2RAD;	// Angle to fang from front of face.
			double fx = cos(fangang) * fangAtRad;
			double fy = sin(fangang) * fangAtRad * yxscale;

			AuSolid fangs;
			AuMatrix4x4 mfl(mSkull);
			mfl.Translate(mouthLoc + AuPoint(-fx, fy, 0));
			mfl.RotateX(PI);
			AuSolidConoid fleft(mfl, fangHt, fangRadBaseX, fangRadBaseY, fangRadTipX, fangRadTipY, false, true);
			fangs.Add(fleft);
			AuMatrix4x4 mfr(mSkull);
			mfr.Translate(mouthLoc + AuPoint(fx, fy, 0));
			mfr.RotateX(PI);
			AuSolidConoid fright(mfr, fangHt, fangRadBaseX, fangRadBaseY, fangRadTipX, fangRadTipY, false, true);
			fangs.Add(fright);

			AuScFaceList faces;
			const float triangleSize = fangRadTipY;
			fangs.Faces(faces, triangleSize);
			faces.SmoothVertices();
			teeth.AddFaces(faces);
		}

		// Teeth decal matrix. ApplyDecal wants the matrix centered at the decal location.
		AuPoint tmin, tmax;
		teeth.Extents(tmin, tmax);
		AuMatrix4x4 mTeethDecal;
		mTeethDecal.Translate((tmin.x + tmax.x)/2, (tmin.y + tmax.y)/2, tmax.z + 1);
		mTeethDecal.RotateX(PI);
		float decalWidth = (tmax.x - tmin.x) * 1.1f;
		float decalHeight = (tmax.y - tmin.y) * 1.1f;
		AuMeatMesh teethMesh(teeth, AuMeatTri::Teeth);
		teethMesh.ApplyDecal(MD_Teeth, mTeethDecal, decalWidth, decalHeight, 1); 

		mesh.Append(teethMesh);
	}
}

void AuChromoMouth::GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
		const AuChromoHead& head) const
{
	Q_UNUSED(dna);
	AuPoint mouthLoc(head.MouthLocation());
	double headHt = head.HeightMM() * FeetPerMM();
	double jawRadX = m_WidthMM * FeetPerMM() / 2;
	double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out. / 2;
	double jawHt = m_HeightMM * FeetPerMM();

	AuSolid solid;

	AuMatrix4x4 tj(mSkull);
	tj.Translate(mouthLoc);

	AuSolidDomoid upperJaw(AuMatrix4x4(), jawRadX, jawRadY, jawHt, true);
	solid.Add(upperJaw);

	AuScFaceList faces;
	const float triangleSize = .5f/12.f * d_ClothingTessFactor;
	solid.Faces(faces, triangleSize);
	faces.ScalePositiveY(jawRadY/jawRadX);	// Stretch only the front part of the domoid.
	faces.Transform(tj);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, headHt, topBottomBlend, blendAngle, blendWid);
	mesh.Append(mymesh);
}

bool AuChromoMouth::GetSkullJaw(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
		quint8 bone) const
{
	Q_UNUSED(dna);
	AuPoint mouthLoc(head.MouthLocation());
	double headHt = head.HeightMM() * FeetPerMM();
	double jawRadX = m_WidthMM * FeetPerMM() / 2;
	double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out. / 2;
	double jawDepth = m_DepthMM * FeetPerMM();
	double lipThickness = m_LipThicknessMM * FeetPerMM();

	AuSolid solid;

	AuMatrix4x4 tj(mSkull);
	tj.Translate(mouthLoc);

	AuMatrix4x4 tflip;
	tflip.RotateX(PI);
	AuSolidDomoid lowerJaw(tflip, jawRadX, jawRadX, jawDepth, true);
	solid.Add(lowerJaw);

	AuScFaceList faces;
	const float triangleSize = .5f/12.f;
	solid.Faces(faces, triangleSize);
	faces.ScalePositiveY(jawRadY/jawRadX);	// Stretch only the front part of the domoid.
	faces.Transform(tj);

	// Convert sc faces to a meat mesh.
	int topBottomBlend = 0;
	AuPoint blendUpVec(0,-1,0);
	float blendAngle = -10*DEG2RADf;
	float blendWid = 30*DEG2RADf;
	AuMeatMesh mymesh;
	mymesh.FromFaces(faces, AuMeatTri::Head);
	mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,1), blendUpVec, headHt, topBottomBlend, blendAngle, blendWid);

	// Teeth in lower jaw
	if(m_Teeth)
	{
		double molarHt = 2 * FeetPerMM();
		if(m_Teeth == Flat)
			molarHt = m_TeethHeightBottomMM * FeetPerMM();
		else
			molarHt = 2 * FeetPerMM();
		double outerRadius = jawRadX - lipThickness;
		int teethInCircle = 32;
		double circ = PI * outerRadius * 2;
		double toothWid = circ / teethInCircle;
		double teethThickness = toothWid;
		double innerRadius = outerRadius - teethThickness;
		double bevel = teethThickness/4;
		AuPointArray tprofile;
		tprofile.append(AuPoint(0,-innerRadius));
		tprofile.append(AuPoint(0,-outerRadius));
		tprofile.append(AuPoint(molarHt,-outerRadius + bevel));
		tprofile.append(AuPoint(molarHt,-innerRadius - bevel));
		tprofile.append(AuPoint(0,-innerRadius));

		AuScFaceList teeth;
		teeth.MakeSpindle(tprofile, teethInCircle, AuMatrix4x4());
		tflip.RotateY(PI/2);
		teeth.Transform(tflip);
		teeth.ScalePositiveY(jawRadY/jawRadX);
		teeth.Transform(tj);
		teeth.FlattenVertexNormals();

		if(m_Teeth == Fangs)
		{
			// Lower fangs.
			double yxscale = jawRadY/jawRadX;
			double fangHt = m_TeethHeightBottomMM * FeetPerMM();
			double fangAtRad = jawRadX - lipThickness - teethThickness/2;
			double fangRadBaseX = teethThickness * .6;
			double fangRadBaseY = fangRadBaseX * yxscale;
			double fangRadTipX = fangRadBaseX / 4;
			double fangRadTipY = fangRadBaseY / 4;
			double fangang = 65 * DEG2RAD;	// Angle to fang from front of face.
			double fx = cos(fangang) * fangAtRad;
			double fy = sin(fangang) * fangAtRad * yxscale;

			AuSolid fangs;
			AuMatrix4x4 mfl(mSkull);
			mfl.Translate(mouthLoc + AuPoint(-fx, fy, 0));
			AuSolidConoid fleft(mfl, fangHt, fangRadBaseX, fangRadBaseY, fangRadTipX, fangRadTipY, false, true);
			fangs.Add(fleft);
			AuMatrix4x4 mfr(mSkull);
			mfr.Translate(mouthLoc + AuPoint(fx, fy, 0));
			AuSolidConoid fright(mfr, fangHt, fangRadBaseX, fangRadBaseY, fangRadTipX, fangRadTipY, false, true);
			fangs.Add(fright);

			AuScFaceList faces;
			const float triangleSize = fangRadTipY;
			fangs.Faces(faces, triangleSize);
			faces.SmoothVertices();
			teeth.AddFaces(faces);
		}

		// Teeth decal matrix. ApplyDecal wants the matrix centered at the decal location.
		AuPoint tmin, tmax;
		teeth.Extents(tmin, tmax);
		AuMatrix4x4 mTeethDecal;
		mTeethDecal.Translate((tmin.x + tmax.x)/2, (tmin.y + tmax.y)/2, tmin.z - 1);
		float decalWidth = (tmax.x - tmin.x) * 1.1f;
		float decalHeight = (tmax.y - tmin.y) * 1.1f;
		AuMeatMesh teethMesh(teeth, AuMeatTri::Teeth);
		teethMesh.ApplyDecal(MD_Teeth, mTeethDecal, decalWidth, decalHeight, 1); 

		mymesh.Append(teethMesh);
	}
	mymesh.SetBonesRangeBlendY(mouthLoc.y-jawRadX-.1, mouthLoc.y+jawRadY+.1, bone-1, bone);
	mymesh.AddBoneRangeBlendX(0, -(jawRadX+.1), 3, bone+1);	// Left side
	mymesh.AddBoneRangeBlendX(0, jawRadX+.1, 3, bone+2);	// Right side
	mesh.Append(mymesh);
	return true;
}

void AuChromoMouth::GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles,
		QMap<AuPoint, double>& hairEx, const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
{
	if(stage == Juvenile)
		return;
	Q_UNUSED(dna);
	Q_UNUSED(hairEx);
	const float follicleSpacing = lockWid * .5f;

	// Get my hair genes.
	AuFolliclePoint::ROLE roleChinFront = AuFolliclePoint::ChinFront;
	AuFolliclePoint::ROLE roleChinLeft = AuFolliclePoint::ChinLeft;
	AuFolliclePoint::ROLE roleChinRight = AuFolliclePoint::ChinRight;
	AuFolliclePoint::ROLE roleLip = AuFolliclePoint::LipTop;
	AuFolliclePoint::ROLE roleFace = AuFolliclePoint::Face;
	const AuGeneHair* hdnaChinFront = HairDNA(roleChinFront);
	const AuGeneHair* hdnaChinLeft = HairDNA(roleChinLeft);
	const AuGeneHair* hdnaChinRight = HairDNA(roleChinRight);
	const AuGeneHair* hdnaLip = HairDNA(roleLip);
	const AuGeneHair* hdnaFace = HairDNA(roleFace);
	if( ! (hdnaLip || hdnaChinFront || hdnaChinLeft || hdnaChinRight || hdnaFace))
		return;

	if(hdnaLip || hdnaFace)
	{
		// Upper Jaw
		AuPoint mouthLoc(head.MouthLocation());
		double jawRadX = m_WidthMM * FeetPerMM() / 2;
		double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out.
		double jawHt = m_HeightMM * FeetPerMM();

		AuMatrix4x4 tj(mSkull);
		tj.Translate(mouthLoc);
		AuSolidDomoid upperJaw(tj, jawRadX, jawRadY, jawHt, true);
		AuPointNArray myfol;
		upperJaw.SurfacePoints(myfol, follicleSpacing);

		for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
		{
			if((*fi).m_N.z < 0)
				continue;		// Normal points downward.
			double zang = (*fi).m_N.PolarIncline();
			double lipzb = 1*DEG2RAD;
			if(zang < lipzb)
				continue;	// No hair ON the lip.
			double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
			double lip = 100*DEG2RAD;
			double lipz = 30*DEG2RAD;

			AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
			if(zang > lipz)
				role = AuFolliclePoint::Face;
			else if(xyang > -lip && xyang < lip)
			{
				if(zang < lipz)
					role = AuFolliclePoint::LipTop;
				else
					role = AuFolliclePoint::Face;
			}
			else
				role = AuFolliclePoint::Face;

			float maxLen;
			switch(role)
			{
			case AuFolliclePoint::Face:
				if(!hdnaFace)
					continue;	// No hair there.
				maxLen = hdnaFace->LengthMM() * FeetPerMM();
				follicles.append(AuFolliclePoint(0, role, *fi, hdnaFace->Color(), maxLen));
				break;
			case AuFolliclePoint::LipTop:
				if(!hdnaLip)
					continue;	// No hair there.
				maxLen = hdnaLip->LengthMM() * FeetPerMM();
				follicles.append(AuFolliclePoint(0, role, *fi, hdnaLip->Color(), maxLen));
				break;
			default:
				continue;	// Other roles don't belong on this mouth part.
			}
		}
	}

	if(hdnaChinFront || hdnaChinLeft || hdnaChinRight || hdnaFace)
	{
		// Lower Jaw
		AuPoint mouthLoc(head.MouthLocation());
		double jawRadX = m_WidthMM * FeetPerMM() / 2;
		double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out.
		double jawDepth = m_DepthMM * FeetPerMM();

		AuMatrix4x4 tj(mSkull);
		tj.Translate(mouthLoc);
		tj.RotateX(PI);
		AuSolidDomoid lowerJaw(tj, jawRadX, jawRadY, jawDepth, true);
		AuPointNArray myfol;
		lowerJaw.SurfacePoints(myfol, follicleSpacing);

		for(AuPointNArray::const_iterator fi = myfol.cbegin(); fi != myfol.cend(); fi++)
		{
			if((*fi).m_N.z > 0)
				continue;	// Normal points up
			double zang = (*fi).m_N.PolarIncline();
			double lipzt = -1*DEG2RAD;
			if(zang > lipzt)
				continue;	// No hair ON the lip.
			double xyang = (*fi).m_N.RotateZ(-PI/2).ZRotation();	// xyangle as if facing +X, so nose angle is 0.
			double front = 15*DEG2RAD;
			double lip = 100*DEG2RAD;
			double lipz = -20*DEG2RAD;

			AuFolliclePoint::ROLE role = AuFolliclePoint::ROLE_Count;
			if(xyang > -front && xyang < front)
				role = AuFolliclePoint::ChinFront;
			else if(zang > lipz)
				role = AuFolliclePoint::Face;
			else if(xyang > -lip && xyang < lip)
			{
				if(xyang < 0)
					role = AuFolliclePoint::ChinRight;
				else
					role = AuFolliclePoint::ChinLeft;
			}
			else
				role = AuFolliclePoint::Face;

			switch(role)
			{
			case AuFolliclePoint::Face:
				if(!hdnaFace)
					continue;	// No hair there.
				follicles.append(AuFolliclePoint(1, role, *fi, hdnaFace->Color(), hdnaFace->LengthMM() * FeetPerMM()));
				break;
			case AuFolliclePoint::ChinLeft:
				if(!hdnaChinLeft)
					continue;	// No hair there.
				follicles.append(AuFolliclePoint(1, role, *fi, hdnaChinLeft->Color(), hdnaChinLeft->LengthMM() * FeetPerMM()));
				break;
			case AuFolliclePoint::ChinRight:
				if(!hdnaChinRight)
					continue;	// No hair there.
				follicles.append(AuFolliclePoint(1, role, *fi, hdnaChinRight->Color(), hdnaChinRight->LengthMM() * FeetPerMM()));
				break;
			case AuFolliclePoint::ChinFront:
				if(!hdnaChinFront)
					continue;	// No hair there.
				follicles.append(AuFolliclePoint(1, role, *fi, hdnaChinFront->Color(), hdnaChinFront->LengthMM() * FeetPerMM()));
				break;
			default:
				continue;	// Other roles don't belong on this mouth part.
			}
		}
	}
}

// Called on a default constructed object.
quint64 AuChromoMouth::HairWhereBits(const AuChromoWhich& which) const
{
	Q_UNUSED(which);
	return 
		AuFolliclePoint::RoleBit(AuFolliclePoint::ChinFront) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::ChinLeft) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::ChinRight) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::Face) |
		AuFolliclePoint::RoleBit(AuFolliclePoint::LipTop);
}

int AuChromoMouth::ListWidgetOrder() const
{
	return 160;
}

QString AuChromoMouth::ListWidgetText() const
{
	return QObject::tr("Mouth");
}

QWidget* AuChromoMouth::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageMouth* pp = new AuChromoPageMouth(parent);
	return pp;
}

float AuChromoMouth::FaceRadiusY(const AuChromoHead& head) const
{
	double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out. / 2;
	AuPoint locOnSkull(head.MouthLocation());
	return locOnSkull.y + jawRadY;
}

float AuChromoMouth::Volume() const
{
	double jawRadX = m_WidthMM * FeetPerMM() / 2;
	double jawRadY = m_LengthMM * FeetPerMM();	// Not divided because we consider the length to be the part that sticks out. / 2;
	double jawDepth = m_DepthMM * FeetPerMM();

	// Volume of ellipsoid
	// V = 4/3 PI a b c
	// a b c are the radii
	float v = 4.f/3.f * PIf * jawRadX * jawRadX * jawRadY / 4;	// Upper jaw is 1/4 of an ellipsoid.
	v += 4.f/3.f * PIf * jawRadX * jawDepth * jawRadY / 4;		// Lower jaw is 1/4 of an ellipsoid.
	return v;
}

bool AuChromoMouth::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoMouth* c = (const AuChromoMouth*)chromosome;
	return true;
}

AuChromosome* AuChromoMouth::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoMouth* c = dynamic_cast<const AuChromoMouth*>(g);
	if(!c)
		return 0;
	AuChromoMouth* comb = new AuChromoMouth(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Internal
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoLungs::AuChromoLungs(ATMTYPE atmoType)
{
	m_AtmoType = atmoType;
}

AuChromoLungs::AuChromoLungs()
{
	m_AtmoType = ATMTYPE_Standard;
}

AuChromoLungs::AuChromoLungs(const AuChromoLungs& c) : AuChromosome(c)
{
	m_AtmoType = c.m_AtmoType;
}

AuChromoLungs::AuChromoLungs(const AuChromoLungs& c1, const AuChromoLungs& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_AtmoType = c1.m_AtmoType;
}

void AuChromoLungs::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoLungs& c = dynamic_cast<const AuChromoLungs&>(o);
		m_AtmoType = c.m_AtmoType;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoLungs::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_AtmoType;
}

void AuChromoLungs::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_AtmoType;
}

void AuChromoLungs::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << m_AtmoType;
}

void AuChromoLungs::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	in >> m_AtmoType;
}

void AuChromoLungs::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoLungs::ListWidgetOrder() const
{
	return 40;
}

QString AuChromoLungs::ListWidgetText() const
{
	return QObject::tr("Lungs<br><small>%1 Atmosphere</small>").arg(AtmoTypeName(AtmosphereType()));
}

QWidget* AuChromoLungs::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageLungs* pp = new AuChromoPageLungs(parent);
	return pp;
}

bool AuChromoLungs::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoLungs* c = (const AuChromoLungs*)chromosome;
	if(m_AtmoType != c->m_AtmoType)
		return false;
	return true;
}

AuChromosome* AuChromoLungs::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoLungs* c = dynamic_cast<const AuChromoLungs*>(g);
	if(!c)
		return 0;
	AuChromoLungs* comb = new AuChromoLungs(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoGills::AuChromoGills(AuChromoGills::STYLE style, AuChromoGills::SEATYPE seaType)
{
	m_SeaType = seaType;
	m_Style = style;
}

AuChromoGills::AuChromoGills()
{
	m_SeaType = Water;
	m_Style = Side;
}

AuChromoGills::AuChromoGills(const AuChromoGills& c) : AuChromosome(c)
{
	m_SeaType = c.m_SeaType;
	m_Style = c.m_Style;
}

AuChromoGills::AuChromoGills(const AuChromoGills& c1, const AuChromoGills& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_SeaType = c1.m_SeaType;
	m_Style = c1.m_Style;
}

void AuChromoGills::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoGills& c = dynamic_cast<const AuChromoGills&>(o);
		m_SeaType = c.m_SeaType;
		m_Style = c.m_Style;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoGills::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_SeaType;
	out << (quint8)m_Style;
}

void AuChromoGills::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_SeaType = (SEATYPE)ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
}

void AuChromoGills::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_SeaType;
	out << (quint8)m_Style;
}

void AuChromoGills::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_SeaType = (SEATYPE)ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
}

void AuChromoGills::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoGills::ListWidgetOrder() const
{
	return 40;
}

QString AuChromoGills::ListWidgetText() const
{
	switch(m_SeaType)
	{
	case Water:
		return QObject::tr("Gills<br><small>Water</small>");
	case SupercooledLiquid:
		return QObject::tr("Gills<br><small>Supercooled Liquid</small>");
	}
	return QObject::tr("Invalid Gills");
}

QWidget* AuChromoGills::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageGills* pp = new AuChromoPageGills(parent);
	return pp;
}

bool AuChromoGills::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoGills* c = (const AuChromoGills*)chromosome;
	if(m_Style != c->m_Style)
		return false;
	if(m_SeaType != c->m_SeaType)
		return false;
	return true;
}

AuChromosome* AuChromoGills::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoGills* c = dynamic_cast<const AuChromoGills*>(g);
	if(!c)
		return 0;
	AuChromoGills* comb = new AuChromoGills(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoBlood::AuChromoBlood(AuChromoBlood::STYLE style)
{
	m_Style = style;
}

AuChromoBlood::AuChromoBlood()
{
	m_Style = Cold;
}

AuChromoBlood::AuChromoBlood(const AuChromoBlood& c) : AuChromosome(c)
{
	m_Style = c.m_Style;
}

AuChromoBlood::AuChromoBlood(const AuChromoBlood& c1, const AuChromoBlood& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_Style = c1.m_Style;
}

void AuChromoBlood::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoBlood& c = dynamic_cast<const AuChromoBlood&>(o);
		m_Style = c.m_Style;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoBlood::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_Style;
}

void AuChromoBlood::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
}

void AuChromoBlood::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_Style;
}

void AuChromoBlood::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Style = (STYLE)ui8;
}

void AuChromoBlood::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoBlood::ListWidgetOrder() const
{
	return 30;
}

QString AuChromoBlood::ListWidgetText() const
{
	QString text;
	switch(m_Style)
	{
	case Cold:
		text = QObject::tr("Cold Blooded");
		break;
	case Warm:
		text = QObject::tr("Warm Blooded");
		break;
	}

	if(text.isEmpty())
		return QObject::tr("Invalid Blood");
	return text;
}

QWidget* AuChromoBlood::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageBlood* pp = new AuChromoPageBlood(parent);
	return pp;
}

bool AuChromoBlood::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoBlood* c = (const AuChromoBlood*)chromosome;
	if(m_Style != c->m_Style)
		return false;
	return true;
}

AuChromosome* AuChromoBlood::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoBlood* c = dynamic_cast<const AuChromoBlood*>(g);
	if(!c)
		return 0;
	AuChromoBlood* comb = new AuChromoBlood(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Young
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoReproduction::AuChromoReproduction(quint8 genitalBits, quint8 litterMin, quint8 litterMax, YOUNG young,
		BIRTHPLACE birthPlace)
{
	m_BirthPlace = birthPlace;
	m_GenitalBits = genitalBits;
	SetLitterRange(litterMin, litterMax);
	m_Young = young;
}

AuChromoReproduction::AuChromoReproduction()
{
	m_BirthPlace = OnLand;
	m_GenitalBits = 0;
	m_LitterMax = 1;
	m_LitterMin = 1;
	m_Young = Eggs;
}

AuChromoReproduction::AuChromoReproduction(const AuChromoReproduction& c) : AuChromosome(c)
{
	m_BirthPlace = c.m_BirthPlace;
	m_GenitalBits = c.m_GenitalBits;
	m_LitterMax = c.m_LitterMax;
	m_LitterMin = c.m_LitterMin;
	m_Young = c.m_Young;
}

AuChromoReproduction::AuChromoReproduction(const AuChromoReproduction& c1, const AuChromoReproduction& c2, const AuDice& Cdice) :
	AuChromosome(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	m_BirthPlace = c1.m_BirthPlace;
	m_GenitalBits = c1.m_GenitalBits;
	m_LitterMax = CombineRange(c1.m_LitterMax, c2.m_LitterMax, dice);
	dice.Random();
	m_LitterMin = CombineRange(c1.m_LitterMin, c2.m_LitterMin, dice);
	if(m_LitterMin > m_LitterMax)
		m_LitterMin = m_LitterMax;
	m_Young = c1.m_Young;
}

void AuChromoReproduction::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoReproduction& c = dynamic_cast<const AuChromoReproduction&>(o);
		m_BirthPlace = c.m_BirthPlace;
		m_GenitalBits = c.m_GenitalBits;
		m_LitterMax = c.m_LitterMax;
		m_LitterMin = c.m_LitterMin;
		m_Young = c.m_Young;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoReproduction::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint8)m_BirthPlace;
	out << m_GenitalBits;
	quint8 ui8 = (m_LitterMin << 4) | m_LitterMax;
	out << ui8;
	out << (quint8)m_Young;
}

void AuChromoReproduction::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_BirthPlace = (BIRTHPLACE)ui8;
	in >> m_GenitalBits;
	in >> ui8;
	m_LitterMax = ui8 & 0x0f;	// Bottom nibble.
	m_LitterMin = ui8 >> 4;		// Top nibble.
	in >> ui8;
	m_Young = (YOUNG)ui8;
}

void AuChromoReproduction::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << (quint8)m_BirthPlace;
	out << m_GenitalBits;
	quint8 ui8 = (m_LitterMin << 4) | m_LitterMax;
	out << ui8;
	out << (quint8)m_Young;
}

void AuChromoReproduction::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_BirthPlace = (BIRTHPLACE)ui8;
	in >> m_GenitalBits;
	in >> ui8;
	m_LitterMax = ui8 & 0x0f;	// Bottom nibble.
	m_LitterMin = ui8 >> 4;		// Top nibble.
	in >> ui8;
	m_Young = (YOUNG)ui8;
}

void AuChromoReproduction::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoReproduction::ListWidgetOrder() const
{
	return 60;
}

QString AuChromoReproduction::ListWidgetText() const
{
	bool canGestate = false;
	QString startWith;
	QString text;
	switch(Gender())
	{
	case Female:
		canGestate = true;
		startWith = ", ";
		text = QObject::tr("Sexual Reproduction<br><small>Female</small>");
		break;
	case Male:
		canGestate = false;
		startWith = ", ";
		text = QObject::tr("Sexual Reproduction<br><small>Male</small>");
		break;
	case Asexual:
		canGestate = true;
		startWith = "<br>";
		text = QObject::tr("Asexual Reproduction");
		break;
	case Hermaphrodite:
		canGestate = true;
		startWith = ", ";
		text = QObject::tr("Sexual Reproduction<br><small>Hermaphrodite</small>");
		break;
	}

	if(canGestate)
	{
		if(m_LitterMin != m_LitterMax)
		{
			if(m_Young == Eggs)
				text += QObject::tr("%1<small>Lays %2 to %3 Eggs").arg(startWith).arg(m_LitterMin).arg(m_LitterMax);
			else// if(m_Young == Live)
				text += QObject::tr("%1<small>Births %2 to %3 Live Young").arg(startWith).arg(m_LitterMin).arg(m_LitterMax);
		}
		else if(m_LitterMin > 1)
		{
			if(m_Young == Eggs)
				text += QObject::tr("%1<small>Lays %2 Eggs").arg(startWith).arg(m_LitterMin);
			else// if(m_Young == Live)
				text += QObject::tr("%1<small>Births %2 Live Young").arg(startWith).arg(m_LitterMin);
		}
		else
		{
			if(m_Young == Eggs)
				text += QObject::tr("%1<small>Lays One Egg").arg(startWith);
			else// if(m_Young == Live)
				text += QObject::tr("%1<small>Births One Live Young").arg(startWith);
		}

		switch(m_BirthPlace)
		{
		case OnLand:
			text += QObject::tr(" on Land");
			break;
		case InWater:
			text += QObject::tr(" in Water");
			break;
		case DeadHost:
			text += QObject::tr(" into a Dead Host");
			break;
		case LiveHost:
			text += QObject::tr(" into a Live Host");
			break;
		}
		text += "</small>";
	}
	return text;
}

QWidget* AuChromoReproduction::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageReproduction* pp = new AuChromoPageReproduction(parent);
	return pp;
}

GENDER AuChromoReproduction::Gender() const
{
	bool hasM = HasMaleGenitals();
	bool hasF = HasFemaleGenitals();
	if(hasM && hasF)
		return Hermaphrodite;
	else if(hasM)
		return Male;
	else if(hasF)
		return Female;
	else
		return Asexual;
}

bool AuChromoReproduction::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	// When comparing both chromatids of a DNA, we want them to match. This isn't the place to test for gender compatibility.
	const AuChromoReproduction* c = (const AuChromoReproduction*)chromosome;
	if(m_BirthPlace != c->m_BirthPlace)
		return false;
	if(m_GenitalBits != c->m_GenitalBits)
		return false;
	if(m_Young != c->m_Young)
		return false;
	return true;
}

AuChromosome* AuChromoReproduction::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoReproduction* c = dynamic_cast<const AuChromoReproduction*>(g);
	if(!c)
		return 0;
	AuChromoReproduction* comb = new AuChromoReproduction(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoMammary::AuChromoMammary(const AuChromoWhich& which, AuChromoMammary::ORGAN organ, quint16 sizeMM, bool feed, bool show)
{
	m_Feed = feed;
	m_Organ = organ;
	m_Show = show;
	m_SizeMM = sizeMM;
	m_Which = which;
}

AuChromoMammary::AuChromoMammary()
{
	m_Feed = false;
	m_Organ = Breast;
	m_Show = true;
	m_SizeMM = 40;
}

AuChromoMammary::AuChromoMammary(const AuChromoMammary& c) : AuChromosome(c)
{
	m_Feed = c.m_Feed;
	m_Organ = c.m_Organ;
	m_Show = c.m_Show;
	m_SizeMM = c.m_SizeMM;
	m_Which = c.m_Which;
}

AuChromoMammary::AuChromoMammary(const AuChromoMammary& c1, const AuChromoMammary& c2, const AuDice& Cdice) :
	AuChromosome(c1, c2, Cdice)
{
	AuDice dice(Cdice);
	if(c1.m_Which != c2.m_Which)
		qWarning() << "warning: AuChromoMammary which mismatch" << c1.m_Which << c2.m_Which;
	m_Feed = c1.m_Feed;
	m_Organ = c1.m_Organ;
	m_Show = CombineBool(c1.m_Show, c2.m_Show, dice);
	dice.Random();
	m_SizeMM = CombineRange(c1.m_SizeMM, c2.m_SizeMM, dice);
	m_Which = c1.m_Which;
}

void AuChromoMammary::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoMammary& c = dynamic_cast<const AuChromoMammary&>(o);
		m_Feed = c.m_Feed;
		m_Organ = c.m_Organ;
		m_Show = c.m_Show;
		m_SizeMM = c.m_SizeMM;
		m_Which = c.m_Which;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoMammary::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	quint8 fs = 0;
	if(m_Feed)
		fs |= 0x01;
	if(m_Show)
		fs |= 0x02;
	out << fs;
	out << m_SizeMM;
	out << (quint8)m_Organ;
	out << m_Which;
}

void AuChromoMammary::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Feed = ui8 & 0x01;
	m_Show = ui8 & 0x02;
	in >> m_SizeMM;
	in >> ui8;
	m_Organ = (ORGAN)ui8;
	in >> m_Which;
}

void AuChromoMammary::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	quint8 fs = 0;
	if(m_Feed)
		fs |= 0x01;
	if(m_Show)
		fs |= 0x02;
	out << fs;
	out << ToLogByte(m_SizeMM, d_DNAMaxMM);
	out << (quint8)m_Organ;
	out << m_Which;
}

void AuChromoMammary::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	quint8 ui8;
	in >> ui8;
	m_Feed = ui8 & 0x01;
	m_Show = ui8 & 0x02;
	m_SizeMM = ReadLogByte(in, d_DNAMaxMM);
	in >> ui8;
	m_Organ = (ORGAN)ui8;
	in >> m_Which;
}

void AuChromoMammary::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_SizeMM = dice.RandomLogValue(m_SizeMM, d_DNAMaxMM);
	AuChromosome::RandomizeSpecies(dice);
}

//static
AuChromosome::LATERALITY AuChromoMammary::Laterality(const AuChromoWhich& which)
{
	if(which.Null())
		return Left;		// wut?
	else if(which.HowMany() == 1)
		return Middle;		// Middle
	else if(which.HowMany() & 1)
	{
		// I am one of an odd number of mammaries.
		if(which.Index() == which.HowMany()-1)
			return Middle;	// Last one is centered.
	}
	if(which.Index() & 1)
		return Right;		// Odd Right
	else
		return Left;		// Even Left
}

void AuChromoMammary::GetTorsoFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
		const AuChromoTorso& torso, GROWTHSTAGE stage) const
{
	if(!m_Show)
		return;
	if(m_SizeMM == 0)
		return;

	float nippleAlpha = .2f;
	AuPoint boobLoc = torso.BreastLocation(dna, mesh, m_Which);
	float boobSize = SizeMM() * FeetPerMM();
	float boobRad = boobSize / 2;

	if(m_Organ == Nipple || stage == Juvenile)
	{
		if(stage == Juvenile)
			boobSize /= 2;

		// Nipple decal matrix. ApplyDecal wants the matrix centered at the decal location.
		AuMatrix4x4 mDecal;
		mDecal.Translate(boobLoc.x, 0, boobLoc.z);
		mDecal.RotateX(-PI/2);
		mesh.ApplyDecal(MD_Nipple, mDecal, boobSize, boobSize, nippleAlpha); 
		return;
	}

	AuMeatMesh mymesh;
	if(m_Organ == Breast)
	{
		// Breasts go on the upper torso if we have one.
		AuMeatTri::PART partType;
		if(torso.UpperLengthMM())
			partType = AuMeatTri::MammaryUpper;
		else
			partType = AuMeatTri::MammaryLower;

		// Build a boob.
		boobLoc.z -= boobRad / 2;
		float nippleTipRad = boobRad / 10;
		float nippleTipLen = nippleTipRad;

		AuMatrix4x4 ms;
		AuSolid solid;
		AuSolidSphere boob(ms, boobRad);
		solid.Add(boob);

		ms.Translate(0,0,boobRad + nippleTipLen);
		ms.RotateX(PI);
		AuSolidCylinder nipple(ms, nippleTipLen*2, nippleTipRad, true, false);
		solid.Add(nipple);

		AuScFaceList faces;
		const float triangleSize = .4f/12.f;
		solid.Faces(faces, triangleSize);

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 0;
		AuPoint blendUpVec(0,-1,0);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		double torsoLen = (torso.UpperLengthMM() + torso.LowerLengthMM()) * FeetPerMM();
		mymesh.FromFaces(faces, partType);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, torsoLen, topBottomBlend, blendAngle, blendWid);

		// Nipple decal matrix. ApplyDecal wants the matrix centered at the decal location.
		AuMatrix4x4 mDecal;
		mDecal.RotateZ((m_Which.Index() * 100) * DEG2RAD);
		mymesh.ApplyDecal(MD_Nipple, mDecal, boobRad, boobRad, nippleAlpha); 

		float swayAngle = 15*DEG2RADf;
		LATERALITY laterality = Laterality();
		AuMatrix4x4 mB;
		mB.Translate(boobLoc);
		if(laterality == Left)
			mB.RotateZ(swayAngle);	// Left
		else if(laterality == Right)
			mB.RotateZ(-swayAngle);	// Right
		mB.RotateX(-PI/2 - swayAngle);
		if(laterality == Right)
			mB.RotateZ(PI);
		else if(laterality == Middle)
			mB.RotateZ(PI/2);
		mymesh.Transform(mB);
	}
	mymesh.Transform(mTorso);
	mesh.Append(mymesh);
}

void AuChromoMammary::GetTorsoFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
		const AuChromoTorso& torso, GROWTHSTAGE stage) const
{
	if(!m_Show)
		return;
	if(m_SizeMM == 0)
		return;
	if(m_Organ == Nipple || stage == Juvenile)
		return;

	AuPoint boobLoc = torso.BreastLocation(dna, mesh, m_Which);
	float boobRad = SizeMM() * FeetPerMM() / 2;

	AuMeatMesh mymesh;
	if(m_Organ == Breast)
	{
		// Breasts go on the upper torso if we have one.
		AuMeatTri::PART partType;
		if(torso.UpperLengthMM())
			partType = AuMeatTri::MammaryUpper;
		else
			partType = AuMeatTri::MammaryLower;

		// Build a boob.
		boobLoc.z -= boobRad / 2;

		AuMatrix4x4 ms;
		AuSolid solid;
		AuSolidSphere boob(ms, boobRad);
		solid.Add(boob);

		AuScFaceList faces;
		const float triangleSize = .4f/12.f;
		solid.Faces(faces, triangleSize);

		// Convert sc faces to a meat mesh.
		int topBottomBlend = 0;
		AuPoint blendUpVec(0,-1,0);
		float blendAngle = -10*DEG2RADf;
		float blendWid = 30*DEG2RADf;
		double torsoLen = (torso.UpperLengthMM() + torso.LowerLengthMM()) * FeetPerMM();
		mymesh.FromFaces(faces, partType);
		mymesh.SmoothNormals();
		mymesh.ApplySkinTexture(AuPoint(0,0), AuPoint(0,0,-1), blendUpVec, torsoLen, topBottomBlend, blendAngle, blendWid);

		float swayAngle = 15*DEG2RADf;
		LATERALITY laterality = Laterality();
		AuMatrix4x4 mB;
		mB.Translate(boobLoc);
		if(laterality == Left)
			mB.RotateZ(swayAngle);	// Left
		else if(laterality == Right)
			mB.RotateZ(-swayAngle);	// Right
		mB.RotateX(-PI/2 - swayAngle);
		if(laterality == Right)
			mB.RotateZ(PI);
		else if(laterality == Middle)
			mB.RotateZ(PI/2);
		mymesh.Transform(mB);
	}
	mymesh.Transform(mTorso);
	mesh.Append(mymesh);
}

int AuChromoMammary::ListWidgetOrder() const
{
	return 210 + m_Which.Index();	// Torso is 200
}

QString AuChromoMammary::ListWidgetText() const
{
	QString text;
	if(m_Feed && m_Show)
	{
		switch(m_Organ)
		{
		case Breast:
			text = QObject::tr("Milks Young from Breast %1").arg(m_Which.Index()+1);
			break;
		case Nipple:
			text = QObject::tr("Milks Young from Nipple %1").arg(m_Which.Index()+1);
			break;
		case Teat:
			text = QObject::tr("Milks Young from Teat %1").arg(m_Which.Index()+1);
			break;
		case Udder:
			text = QObject::tr("Milks Young from Udder %1").arg(m_Which.Index()+1);
			break;
		}
	}
	else if(m_Feed)
	{
		switch(m_Organ)
		{
		case Breast:
			text = QObject::tr("Milks Young from Hidden Breast %1").arg(m_Which.Index()+1);
			break;
		case Nipple:
			text = QObject::tr("Milks Young from Hidden Nipple %1").arg(m_Which.Index()+1);
			break;
		case Teat:
			text = QObject::tr("Milks Young from Hidden Teat %1").arg(m_Which.Index()+1);
			break;
		case Udder:
			text = QObject::tr("Milks Young from Hidden Udder %1").arg(m_Which.Index()+1);
			break;
		}
	}
	else if(m_Show)
	{
		switch(m_Organ)
		{
		case Breast:
			text = QObject::tr("Vestigial Breast %1").arg(m_Which.Index()+1);
			break;
		case Nipple:
			text = QObject::tr("Vestigial Nipple %1").arg(m_Which.Index()+1);
			break;
		case Teat:
			text = QObject::tr("Vestigial Teat %1").arg(m_Which.Index()+1);
			break;
		case Udder:
			text = QObject::tr("Vestigial Udder %1").arg(m_Which.Index()+1);
			break;
		}
	}
	else
	{
		switch(m_Organ)
		{
		case Breast:
			text = QObject::tr("Hidden Vestigial Breast %1").arg(m_Which.Index()+1);
			break;
		case Nipple:
			text = QObject::tr("Hidden Vestigial Nipple %1").arg(m_Which.Index()+1);
			break;
		case Teat:
			text = QObject::tr("Hidden Vestigial Teat %1").arg(m_Which.Index()+1);
			break;
		case Udder:
			text = QObject::tr("Hidden Vestigial Udder %1").arg(m_Which.Index()+1);
			break;
		}
	}
	if(text.isEmpty())
		text = QObject::tr("Invalid Mammary");

	text += QObject::tr("<br><small>%1</small>").arg(LateralityName(Laterality()));
	return text;
}

QWidget* AuChromoMammary::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageMammary* pp = new AuChromoPageMammary(parent);
	return pp;
}

bool AuChromoMammary::PrepareToAdd(const AuChromatid& chain)
{
	// Pick up my properties from any existing chromosome of the same type.
	AuChromoWhich which = chain.AvailableWhich(ChromosomeType());
	if(which.Null())
		return false;
	const AuChromosome* x = 0;
	if(which.Index())
		x = chain.Chromosome(ChromosomeType(), AuChromoWhich(which.Index()-1, which.HowMany()));	// Try next prev of same type.
	if(!x)
		x = chain.Chromosome(ChromosomeType());			// Try any of the same type.
	if(x)
		Copy(*x);								// Copy properties from existing chromosome.
	m_Which = which;
	return true;
}

void AuChromoMammary::SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
{
	//DHH animalia
}

bool AuChromoMammary::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
{
	return false;	//DHH animalia
}

float AuChromoMammary::Volume() const
{
	if(m_SizeMM == 0)
		return 0;
	if(m_Organ == Nipple)
		return 0;
	// Volume of sphere
	// V = 4/3 * PI * r^3
	float r = m_SizeMM * FeetPerMM() / 2;
	float v = 4.f/3.f * PIf * r * r * r * .7f;	// Fraction because it is partly embedded in the chest.
	return v;
}

bool AuChromoMammary::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	const AuChromoMammary* c = (const AuChromoMammary*)chromosome;
	if(m_Which != c->m_Which)
		return false;
	if(m_Feed != c->m_Feed)
		return false;
	if(m_Organ != c->m_Organ)
		return false;
	return true;
}

AuChromosome* AuChromoMammary::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoMammary* c = dynamic_cast<const AuChromoMammary*>(g);
	if(!c)
		return 0;
	AuChromoMammary* comb = new AuChromoMammary(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Food
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEatsPlants::AuChromoEatsPlants(COMMODITYID prefers)
{
	m_Prefers = prefers;
}

AuChromoEatsPlants::AuChromoEatsPlants()
{
	m_Prefers = CID_Invalid;
}

AuChromoEatsPlants::AuChromoEatsPlants(const AuChromoEatsPlants& c) : AuChromosome(c)
{
	m_Prefers = c.m_Prefers;
}

AuChromoEatsPlants::AuChromoEatsPlants(const AuChromoEatsPlants& c1, const AuChromoEatsPlants& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_Prefers = (COMMODITYID)CombineEnum(c1.m_Prefers, c2.m_Prefers, dice);
}

void AuChromoEatsPlants::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoEatsPlants& c = dynamic_cast<const AuChromoEatsPlants&>(o);
		m_Prefers = c.m_Prefers;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoEatsPlants::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << (quint16)m_Prefers;
}

void AuChromoEatsPlants::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	quint16 ui16;
	in >> ui16;
	m_Prefers = (COMMODITYID)ui16;
}

void AuChromoEatsPlants::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
}

void AuChromoEatsPlants::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
}

void AuChromoEatsPlants::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoEatsPlants::ListWidgetOrder() const
{
	return 51;
}

QString AuChromoEatsPlants::ListWidgetText() const
{
	if(m_Prefers != CID_Invalid)
		return QObject::tr("Eats Plants<br><small>Prefers %1</small>")
			.arg(g_Au.CommodityName(m_Prefers));
	else
		return QObject::tr("Eats Plants");
}

QWidget* AuChromoEatsPlants::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageEatsPlants* pp = new AuChromoPageEatsPlants(parent);
	return pp;
}

bool AuChromoEatsPlants::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoEatsPlants* c = (const AuChromoEatsPlants*)chromosome;
	return true;
}

AuChromosome* AuChromoEatsPlants::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoEatsPlants* c = dynamic_cast<const AuChromoEatsPlants*>(g);
	if(!c)
		return 0;
	AuChromoEatsPlants* comb = new AuChromoEatsPlants(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuChromoEatsAnimals::AuChromoEatsAnimals(quint8 eatBits)
{
	m_EatBits = eatBits;
}

AuChromoEatsAnimals::AuChromoEatsAnimals()
{
	m_EatBits = 0xff;
}

AuChromoEatsAnimals::AuChromoEatsAnimals(const AuChromoEatsAnimals& c) : AuChromosome(c)
{
	m_EatBits = c.m_EatBits;
}

AuChromoEatsAnimals::AuChromoEatsAnimals(const AuChromoEatsAnimals& c1, const AuChromoEatsAnimals& c2, const AuDice& dice) :
	AuChromosome(c1, c2, dice)
{
	m_EatBits = (quint8)CombineBits(c1.m_EatBits, c2.m_EatBits, dice);
}

void AuChromoEatsAnimals::Copy(const AuChromosome& o)
{
	try
	{
		const AuChromoEatsAnimals& c = dynamic_cast<const AuChromoEatsAnimals&>(o);
		m_EatBits = c.m_EatBits;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
	}
}

void AuChromoEatsAnimals::operator>>(QDataStream& out) const
{
	AuChromosome::operator>>(out);
	out << m_EatBits;
}

void AuChromoEatsAnimals::ReadStream(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadStream(in, fileVer);
	in >> m_EatBits;
}

void AuChromoEatsAnimals::WriteSpeciesKey(QDataStream& out) const
{
	AuChromosome::WriteSpeciesKey(out);
	out << m_EatBits;
}

void AuChromoEatsAnimals::ReadSpeciesKey(QDataStream& in, quint8 fileVer)
{
	AuChromosome::ReadSpeciesKey(in, fileVer);
	in >> m_EatBits;
}

void AuChromoEatsAnimals::RandomizeSpecies(const AuDice& dice)
{
	AuChromosome::RandomizeSpecies(dice);
}

int AuChromoEatsAnimals::ListWidgetOrder() const
{
	return 50;
}

QString AuChromoEatsAnimals::ListWidgetText() const
{
	QString text = QObject::tr("Eats Animals");
	if(m_EatBits != 0)
	{
		text += "<br><small>";

		QStringList eats;
		if(m_EatBits & BloodSucker)
			eats << QObject::tr("Blood Sucker");
		if(m_EatBits & Brain)
			eats << QObject::tr("Brain");
		if(m_EatBits & Cannibal)
			eats << QObject::tr("Cannibal");
		if(m_EatBits & Carne)
			eats << QObject::tr("Carne");
		if(m_EatBits & Carrion)
			eats << QObject::tr("Carrion");
		if(m_EatBits & Organs)
			eats << QObject::tr("Organs");
		if(m_EatBits & Predator)
			eats << QObject::tr("Predator");

		if(eats.size() == 1)
			text += eats.first();
		else
		{
			QStringList::const_iterator ei = eats.cbegin();
			text += *ei;
			ei++;
			for(; ei != eats.cend(); ei++)
				text += ", " + *ei;
		}

		text += "</small>";
	}
	return text;
}

QWidget* AuChromoEatsAnimals::MakePropertiesPage(QWidget* parent) const
{
	AuChromoPageEatsAnimals* pp = new AuChromoPageEatsAnimals(parent);
	return pp;
}

bool AuChromoEatsAnimals::IsSexuallyCompatible(const AuChromosome* chromosome) const
{
	Q_UNUSED(chromosome);
	//const AuChromoEatsAnimals* c = (const AuChromoEatsAnimals*)chromosome;
	return true;
}

AuChromosome* AuChromoEatsAnimals::Combine(const AuChromosome* g, const AuDice& dice) const
{
	const AuChromoEatsAnimals* c = dynamic_cast<const AuChromoEatsAnimals*>(g);
	if(!c)
		return 0;
	AuChromoEatsAnimals* comb = new AuChromoEatsAnimals(*this, *c, dice);
	return comb;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNA::AuDNA(const AuDNA& dna)
{
	m_Chromatid[Alpha] = dna.m_Chromatid[Alpha];
	m_Chromatid[Beta] = dna.m_Chromatid[Beta];
}

AuDNA::AuDNA(const AuDNA& dna1, const AuDNA& dna2, const AuDice& dice)
{
	if(dna1.m_Chromatid[Alpha].IsSexuallyCompatible(dna2.m_Chromatid[Alpha]) &&
			dna1.m_Chromatid[Beta].IsSexuallyCompatible(dna2.m_Chromatid[Beta]))
	{
		// Chromatids can combine 1:1.
		m_Chromatid[Alpha] = dna1.m_Chromatid[Alpha].Combine(dna2.m_Chromatid[Alpha], dice);	// Same dice for each.
		m_Chromatid[Beta] = dna1.m_Chromatid[Beta].Combine(dna2.m_Chromatid[Beta], dice);	// Same dice for each.
	}
	else
	{
		// Chromatids must be reversed to combine them.
		m_Chromatid[Alpha] = dna1.m_Chromatid[Alpha].Combine(dna2.m_Chromatid[Beta], dice);	// Same dice for each.
		m_Chromatid[Beta] = dna1.m_Chromatid[Beta].Combine(dna2.m_Chromatid[Alpha], dice);	// Same dice for each.
	}
}

const AuDNA& AuDNA::operator=(const AuDNA& dna)
{
	m_Chromatid[Alpha] = dna.m_Chromatid[Alpha];
	m_Chromatid[Beta] = dna.m_Chromatid[Beta];
	return *this;
}

bool AuDNA::operator==(const AuDNABase& b) const
{
	try
	{
		const AuDNA& c = dynamic_cast<const AuDNA&>(b);
		if(m_Chromatid[Alpha] != c.m_Chromatid[Alpha])
			return false;
		if(m_Chromatid[Beta] != c.m_Chromatid[Beta])
			return false;
		return true;
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

void AuDNA::InitDefaultHuman()
{
	InitDefaultHuman(AuChromoSkin::Color(0), AuChromoFur::Color(20), AuChromoSkin::Color(0), AuChromoFur::Color(8));
}

void AuDNA::InitDefaultHuman(const QColor& alphaSkinColor, const QColor& alphaHairColor, const QColor& betaSkinColor,
		const QColor& betaHairColor)
{
	m_Chromatid[Alpha].InitDefaultHuman(Male, alphaSkinColor, alphaHairColor);
	m_Chromatid[Beta].InitDefaultHuman(Female, betaSkinColor, betaHairColor);
}

void AuDNA::MakeParts(QList<AuAnimalPart*>& parts, CHROMATID chain) const
{
	m_Chromatid[chain].MakeParts(parts);
}

void AuDNA::ApplyChanges(const AuChromosome* alpha, const AuChromosome* beta)
{
	m_Chromatid[Alpha].ApplyChanges(alpha);
	m_Chromatid[Beta].ApplyChanges(beta);
}

void AuDNA::SkinRenderParameters(CHROMATID chain, QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor,
		QString& bottomSkinTexId, QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId,
		QString& decalTexFile) const
{
	m_Chromatid[chain].SkinRenderParameters(topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
			bottomSkinColor, decalTexId, decalTexFile);
}

void AuDNA::Remove(AuChromosome::CTYPE chromosome, const AuChromoWhich& which) 
{
	m_Chromatid[Alpha].Remove(chromosome, which);
	m_Chromatid[Beta].Remove(chromosome, which);
}

void AuDNA::AddableChromosomes(QMap<QString, AuChromosome::CTYPE>& addable) const
{
	m_Chromatid[Alpha].AddableChromosomes(addable);
}

bool AuDNA::AddChromosome(AuChromosome::CTYPE type)
{
	bool a = m_Chromatid[Alpha].AddChromosome(type);
	bool b = m_Chromatid[Beta].AddChromosome(type);
	return a || b;
}

void AuDNA::AddChromosome(const AuChromosome& chromosome)
{
	m_Chromatid[Alpha].AddChromosome(chromosome);
	m_Chromatid[Beta].AddChromosome(chromosome);
}

void AuDNA::operator>>(QDataStream& out) const
{
	out << (quint8)DNA_FILE_VER;
	out << m_Chromatid[Alpha];
	out << m_Chromatid[Beta];
}

void AuDNA::operator<<(QDataStream& in)
{
	quint8 fileVer;
	in >> fileVer;
	m_Chromatid[Alpha].ReadStream(in, fileVer);
	m_Chromatid[Beta].ReadStream(in, fileVer);
}

int AuDNA::SaveFile(const QString& fileName) const
{
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly))
		return 1;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_7);

	ds << (quint8)DNA_FILE_TYPE;
	ds << *this;

	file.close();
	return 0;
}

int AuDNA::LoadFile(const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return 1;

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_7);

	quint8 fileType;
	ds >> fileType;
	if(fileType != DNA_FILE_TYPE)
	{
		file.close();
		return 2;	// bad file type
	}

	ds >> *this;

	file.close();
	return 0;
}

bool AuDNA::IsValid() const
{
	return true;	//DHH
}

AuChromosome::CTYPE AuDNA::Phylum() const
{
	return m_Chromatid[Alpha].Phylum();	// Phylum is supposed to be the same in each chain.
}

QString AuDNA::PhylumName() const
{
	QString phylumName;
	switch(Phylum())
	{
	case AuChromosome::Annelida:
		phylumName = QObject::tr("Annelid");
		break;
	case AuChromosome::Arthropoda:
		phylumName = QObject::tr("Arthropod");
		break;
	case AuChromosome::Chordata:
		phylumName = QObject::tr("Chordate");
		break;
	case AuChromosome::Mollusca:
		phylumName = QObject::tr("Mollusc");
		break;
	default:
		phylumName = QObject::tr("Unknown Phylum");
		break;
	}
	return phylumName;
}

bool AuDNA::IsFlyer() const
{
	return false;	//DHH
}

QString AuDNA::SpeciesDescription() const
{
	QString env;
	if(IsDigger())
		env = QObject::tr("Subterranean ");
	if(BreathesWater() != HYDRO_None && IsFlyer() && BreathesAir() != ATMTYPE_Count)
		env += QObject::tr("Triphibious");
	else if(BreathesWater() != HYDRO_None && BreathesAir() != ATMTYPE_Count)
		env += QObject::tr("Amphibious");
	else if(BreathesWater() != HYDRO_None && IsFlyer())
		env += QObject::tr("Aquaerial");
	else if(IsFlyer())
		env += QObject::tr("Aerial");
	else if(BreathesWater() != HYDRO_None)
		env += QObject::tr("Aquatic");

	QString role;
	switch(EcoRole())
	{
	case ECO_Carnivore:		// Creature only eats meat that it kills.
		role = QObject::tr("Carnivorous");
		break;
	case ECO_Herbivore:		// Creature only eats plants.
		role = QObject::tr("Herbivorous");
		break;
	case ECO_Omnivore:		// Creature eats plants or meat that it kills.
		role = QObject::tr("Omnivorous");
		break;
	case ECO_Scavenger:		// Creature only eats meat killed by others.
		role = QObject::tr("Scavenging");
		break;
	}

	if(!env.isEmpty())
		return env + " " + role + " " + PhylumName();
	else
		return role + " " + PhylumName();
}

ECOROLE AuDNA::EcoRole() const
{
	return m_Chromatid[Alpha].EcoRole();
}

ATMTYPE AuDNA::BreathesAir() const
{
	return m_Chromatid[Alpha].BreathesAir();
}

HYDROGRAPHIC_LIQUID AuDNA::BreathesWater() const
{
	return m_Chromatid[Alpha].BreathesWater();
}

bool AuDNA::IsDigger() const
{
	return false;	//DHH animalia
}

TOD AuDNA::TimeActive() const
{
	return TOD_Diurnal;	//DHH animalia
}

QString AuDNA::TimeActiveName() const
{
	return ::TimeActiveName(TimeActive());
}

bool AuDNA::IsSexuallyCompatible(const AuDNABase& dna) const
{
	try
	{
		const AuDNA& c = dynamic_cast<const AuDNA&>(dna);
		return (m_Chromatid[Alpha].IsSexuallyCompatible(c.m_Chromatid[Alpha]) &&
			m_Chromatid[Beta].IsSexuallyCompatible(c.m_Chromatid[Beta])) ||
			(m_Chromatid[Alpha].IsSexuallyCompatible(c.m_Chromatid[Beta]) &&
			 m_Chromatid[Beta].IsSexuallyCompatible(c.m_Chromatid[Alpha]));
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return false;
	}
}

AuDNABase* AuDNA::Combine(const AuDNABase& dnaBase, const AuDice& dice) const
{
	// Verify we have the same kind of chromosomes in the same order.
	if(!IsSexuallyCompatible(dnaBase))
		return 0;

	try
	{
		const AuDNA& dna = dynamic_cast<const AuDNA&>(dnaBase);
		return new AuDNA(*this, dna, dice);
	}
	catch(const bad_cast&)
	{
		// dynamic_cast failed
		return 0;
	}
};

AuSpecies AuDNA::Species() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_7);
	out << (quint8)DNA_SPECIES_TYPE;
	out << (quint8)DNA_SPECIES_VER;
	m_Chromatid[Alpha].WriteSpeciesKey(out);
	m_Chromatid[Beta].WriteSpeciesKey(out);
	return AuSpecies(ba);
}

bool AuDNA::FromSpecies(const AuSpecies& species)
{
	QDataStream in(species.Key());
	in.setVersion(QDataStream::Qt_5_7);
	quint8 speciesType;
	in >> speciesType;
	if(speciesType != DNA_SPECIES_TYPE)
		return false;
	quint8 speciesVer;
	in >> speciesVer;
	m_Chromatid[Alpha].ReadSpeciesKey(in, speciesVer);
	m_Chromatid[Beta].ReadSpeciesKey(in, speciesVer);
	return true;
}

void AuDNA::RandomizeSpecies(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_Chromatid[Alpha].RandomizeSpecies(dice);
	dice.Random();
	m_Chromatid[Beta].RandomizeSpecies(dice);
}

int AuDNA::GestateBabyDays() const
{
	return 9;	//DHH animalia
}

int AuDNA::GestateImplantHours() const
{
	return 1;	//DHH animalia
}

int AuDNA::MinutesToMature() const
{
	return 10;	//DHH animalia
}

GENDER AuDNA::Gender(CHROMATID chroma) const
{
	return m_Chromatid[chroma].Gender();
}

bool AuDNA::HasBone(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	if(butcheringWhat != CID_Animal && butcheringWhat != CID_Fish && butcheringWhat != CID_Head)
		return false;

	const AuChromoChordata* ch = (const AuChromoChordata*)Chain(chroma).Chromosome(AuChromosome::Chordata);
	if(!ch)
		return false;
	return ch->Skeleton() == AuChromoChordata::Bones;
}

bool AuDNA::HasBrain(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	Q_UNUSED(chroma);
	return butcheringWhat == CID_Head;
}

bool AuDNA::HasGills(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	return (butcheringWhat == CID_Animal || butcheringWhat == CID_Fish) && Chain(chroma).Chromosome(AuChromosome::Gills);
}

bool AuDNA::HasHeart(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	return (butcheringWhat == CID_Animal || butcheringWhat == CID_Fish) && Chain(chroma).Chromosome(AuChromosome::Blood);
}

bool AuDNA::HasLeather(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	return (butcheringWhat == CID_Animal || butcheringWhat == CID_Fish) && Chain(chroma).Chromosome(AuChromosome::Skin);
}

bool AuDNA::HasLungs(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	return (butcheringWhat == CID_Animal || butcheringWhat == CID_Fish) && Chain(chroma).Chromosome(AuChromosome::Lungs);
}

bool AuDNA::HasStomach(CHROMATID chroma, COMMODITYID butcheringWhat) const
{
	if(butcheringWhat != CID_Animal && butcheringWhat != CID_Fish)
		return false;
	return Chain(chroma).Chromosome(AuChromosome::EatsPlants) ||
		Chain(chroma).Chromosome(AuChromosome::EatsAnimals);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

