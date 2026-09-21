//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufloradna.cpp,v 1.5 2022/02/19 18:33:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aufloradna.h"
//
#include "audice.h"
#include "auglobal.h"

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuFloraDNAMetaType = qRegisterMetaType<AuFloraDNA>();

AuFloraDNA::AuFloraDNA()
{
	memset(this, 0, sizeof(AuFloraDNA));
}

QString AuFloraDNA::ListName() const
{
	QString inflo[] =
	{
		QObject::tr("Spike"),
		QObject::tr("Raceme"),
		QObject::tr("Corymb"),
		QObject::tr("Panicle"),
		QObject::tr("Simple umbel"),
		QObject::tr("Compound umbel"),
		QObject::tr("Cyme"),
		QObject::tr("Scorpioid cyme"),
	};

	QString desc = QObject::tr("%1 inflorescence.")
		.arg(inflo[Inflorescence]);

	if(FlowerProbability)
	{
		QString petalColorName;
		QStringList colorNames = QColor::colorNames();
		QColor petalCol(PetalColor());
		for(QStringList::const_iterator si = colorNames.cbegin(); si != colorNames.cend(); si++)
		{
			QColor pcol(*si);
			pcol.setRed(pcol.red() & 0xe0);		// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			pcol.setGreen(pcol.green() & 0xe0);	// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			pcol.setBlue(pcol.blue() & 0xe0);	// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			if(pcol == petalCol)
			{
				petalColorName = *si;
				break;
			}
		}

		if(petalColorName.isEmpty())
			petalColorName = QObject::tr("%1 colored").arg(petalCol.name());

		desc += QObject::tr(" Flowers have %1 petals.")
			.arg(petalColorName);
	}

	return desc;
}

QString AuFloraDNA::Description() const
{
	QString inflo[] =
	{
		QObject::tr("Spike"),
		QObject::tr("Raceme"),
		QObject::tr("Corymb"),
		QObject::tr("Panicle"),
		QObject::tr("Simple umbel"),
		QObject::tr("Compound umbel"),
		QObject::tr("Cyme"),
		QObject::tr("Scorpioid cyme"),
	};

	QString leafTypes[] =
	{
		QObject::tr("Parallel broadening"),
		QObject::tr("Parallel uniform"),
		QObject::tr("Parallel narrowing"),
		QObject::tr("Radial lengthening"),
		QObject::tr("Radial uniform"),
		QObject::tr("Radial shortening"),
		QObject::tr("Bow"),
		QObject::tr("Bow"),
	};

	QString stiffness[] =
	{
		QObject::tr("Flimsy"),
		QObject::tr("Soft"),
		QObject::tr("Pliable"),
		QObject::tr("Flexible"),
		QObject::tr("Inflexible"),
		QObject::tr("Firm"),
		QObject::tr("Hard"),
		QObject::tr("Rigid"),
	};

	QString rootTypes[] =
	{
		QObject::tr("Bulb"),
		QObject::tr("Fibrous"),
		QObject::tr("Prop"),
		QObject::tr("Tap"),
	};

	QString desc = QObject::tr("%1 inflorescence. %2 leaves in multiples of %3. %4 stalk. %5 root.")
		.arg(inflo[Inflorescence])
		.arg(leafTypes[LeafType])
		.arg(Multiples+1)
		.arg(stiffness[Stiffness])
		.arg(rootTypes[RootType]);

	if(ThornSize)
	{
		QString thornFreq[] =
		{
			QObject::tr("Sparse"),
			QObject::tr("Few"),
			QObject::tr("Moderate"),
			QObject::tr("Many"),
		};

		QString thorns[] =
		{
			QObject::tr("No"),
			QObject::tr("small"),
			QObject::tr("medium"),
			QObject::tr("large"),
		};

		desc += QObject::tr(" %1 %2 thorns.")
			.arg(thornFreq[ThornFreq])
			.arg(thorns[ThornSize]);
	}

	if(Chemical != CHEM_None)
	{
		QString chemStr[] =
		{
			QObject::tr("faint"),
			QObject::tr("weak"),
			QObject::tr("medium"),
			QObject::tr("strong"),
		};

		QString chem[] =
		{
			QObject::tr(""),
			QObject::tr("acid"),
			QObject::tr("asphyxiant"),
			QObject::tr("paralysis"),
			QObject::tr("poison"),
			QObject::tr("radiation"),
			QObject::tr("disease"),
			QObject::tr("electricity"),
		};

		desc += QObject::tr(" Exudes %1 %2.")
			.arg(chemStr[ChemicalStrength])
			.arg(chem[Chemical]);
	}

	if(FlowerProbability)
	{
		QString petalColorName;
		QString ovaryColorName;
		QStringList colorNames = QColor::colorNames();
		QColor petalCol(PetalColor());
		QColor ovaryCol(OvaryColor());
		for(QStringList::const_iterator si = colorNames.cbegin(); si != colorNames.cend(); si++)
		{
			QColor pcol(*si);
			pcol.setRed(pcol.red() & 0xe0);		// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			pcol.setGreen(pcol.green() & 0xe0);	// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			pcol.setBlue(pcol.blue() & 0xe0);	// Strip bottom 5 bits. Our colors are all zero in the bottom five bits.
			if(pcol == petalCol)
				petalColorName = *si;

			if(pcol == ovaryCol)
				ovaryColorName = *si;

			if(!petalColorName.isEmpty() && !ovaryColorName.isEmpty())
				break;
		}

		if(petalColorName.isEmpty())
			petalColorName = QObject::tr("%1 colored").arg(petalCol.name());

		if(ovaryColorName.isEmpty())
			ovaryColorName = QObject::tr("%1 colored").arg(ovaryCol.name());

		QString ova[] =
		{
			QObject::tr("Inferior"),
			QObject::tr("Superior"),
		};

		desc += QObject::tr(" Flowers have %1 petals in multiples of %2. %3 %4 ovary.")
			.arg(petalColorName)
			.arg(PetalMultiples+1)
			.arg(ova[FlowerOvary])
			.arg(ovaryColorName);
	}

	return desc;
}

void AuFloraDNA::Init(ZONE zone, const AuDice& Cdice, int sizemod)
{
	AuDice dice(Cdice);
	Inflorescence = (INFLORESCENCE)dice.Roll(1,8,-1);	// 0..7
	int size = dice.Roll(2, 30, sizemod);	// Bell curve from [2..60] + sizemod.
	size /= 2;									// Result is [1..30] + sizemod
	if(size < 0)
		size = 0;
	else if(size > 31)
		size = 31;
	TrunkLength = size;

	Multiples = dice.Roll(1,4,-1); 			// 0..3
	SproutMultiples = dice.Roll(1,8,-1);		// 0..7
	BranchAngle = dice.Roll(1,4,-1); 		// 0..3
	Unused0 = dice.Roll(1,2,-1); 			// 0..1

	BranchProbability = dice.Roll(1,4,-1);	// 0..3
	RootType = (ROOT)dice.Roll(1,4,-1);		// 0..3
	RootSize = dice.Roll(1,4,-1);			// 0..3
	LeafProbability = dice.Roll(1,4,-1);		// 0..3

	int leafMod;
	if(zone == Polar)
		leafMod = -1;	// Narrower in polar regions.
	else if(zone == Temperate)
		leafMod = 0;
	else
		leafMod = 1;		// Wider in tropics.

	LeafType = (LEAF)dice.Roll(1,8,-1);		// 0..7
	int leafSize = dice.Roll(1,8,-1+leafMod);	// 0..7
	if(leafSize < 0)
		LeafSize = 0;
	else if(leafSize > 7)
		LeafSize = 7;
	else
		LeafSize = leafSize;
	LeafVeinCount = dice.Roll(1,4,-1);		// 0..3

	int leafWid = dice.Roll(1,8,-1+leafMod);	// 0..7
	if(leafWid < 0)
		LeafVeinWidth = 0;
	else if(leafWid > 7)
		LeafVeinWidth = 7;
	else
		LeafVeinWidth = leafWid;
	Chemical = (CHEMICAL)dice.Roll(1,8,-1);	// 0..7
	ChemicalStrength = dice.Roll(1,4,-1);	// 0..3

	InternodeSizeRatio = dice.Roll(1,16,-1);	// 0..15
	BarkTexture = dice.Roll(1,8,-1);			// 0..7
	PetalMultiples = dice.Roll(1,2,-1);		// 0..1

	PetalLength = dice.Roll(1,8,-1);			// 0..7
	PetalTaper = dice.Roll(1,4,-1);			// 0..3
	PetalWidth = dice.Roll(1,8,-1);			// 0..7

	FlowerProbability = dice.Roll(1,4,-1);	// 0..3
	FlowerPetalRed = dice.Roll(1,8,-1);		// 0..7
	FlowerPetalGreen = dice.Roll(1,8,-1);	// 0..7

	FlowerPetalBlue = dice.Roll(1,8,-1);		// 0..7
	FlowerOvarySize = dice.Roll(1,4,-1);		// 0..3
	FlowerOvaryRed = dice.Roll(1,8,-1);		// 0..7

	FlowerOvaryGreen = dice.Roll(1,8,-1);	// 0..7
	FlowerOvaryBlue = dice.Roll(1,8,-1);		// 0..7
	LeafStemLength = dice.Roll(1,4,-1);		// 0..3

	FlowerOvary = (OVARY)dice.Roll(1,2,-1);	// 0..1
	ThornSize = dice.Roll(1,4,-1);			// 0..3
	ThornFreq = dice.Roll(1,4,-1);			// 0..3
	Stiffness = dice.Roll(1,8,-1);			// 0..7
	//qDebug("AuFloraDNA::Init finished desc %s", (const char*)Description());
}

void AuFloraDNA::operator<<(QDataStream& in)
{
	QByteArray ba;
	in >> ba;
	memcpy(this, ba.constData(), sizeof(AuFloraDNA));
}

void AuFloraDNA::operator>>(QDataStream& out) const
{
	QByteArray ba((const char*)this, sizeof(AuFloraDNA));
	out << ba;
}

void AuFloraDNA::TxUpdate(QDataStream& toClient) const
{
	QByteArray ba((const char*)this, sizeof(AuFloraDNA));
	toClient << ba;
}

void AuFloraDNA::RxUpdate(QDataStream& fromServer)
{
	QByteArray ba;
	fromServer >> ba;
	memcpy(this, ba.constData(), sizeof(AuFloraDNA));
}

float AuFloraDNA::MinBranchDiameter() const
{
	return g_Au.TreeTrunkMinDia();
}
