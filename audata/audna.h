//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audna.h,v 1.106 2022/05/09 16:11:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDNA_H
#define AuDNA_H

#include "audnabase.h"
#include "aumeat.h"
#include "dbcommodity.h"
#include <QList>

class AuAnimalPart;
class AuClothing;
class AuChromatid;
class AuChromoHead;
class AuChromoTorso;
class AuElapsedTimer;
class AuGeneChain;
class AuMeatMesh;
class AuPose;
class AuTime;
class QDataStream;
class QDebug;

#define d_DNAMaxMM						5000					// Max mm for most dna mm lengths. This is the basis of the data
																// AuDNA stores in AuSpecies. DO NOT CHANGE THIS VALUE!

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAnimalBone
{
private:
	AuPoint								m_Loc;					// Location of bone in unposed model.
	AuMatrix4x4							m_M;					// Projection matrix applied to achieve posed location.

public:	// Constructors
	inline AuAnimalBone()
	{
	}

	inline AuAnimalBone(const AuPoint& loc)
	{
		m_Loc = loc;
	}

	inline AuAnimalBone(const AuAnimalBone& c)
	{
		m_Loc = c.m_Loc;
		m_M = c.m_M;
	}

public:	// Operators
	inline const AuAnimalBone& operator=(const AuAnimalBone& c)
	{
		m_Loc = c.m_Loc;
		m_M = c.m_M;
		return *this;
	}

public:	// Methods
	inline const AuPoint& UnposedLoc() const
	{
		return m_Loc;
	}

	inline const AuMatrix4x4& Matrix() const
	{
		return m_M;
	}

	inline AuPoint PosedLoc() const
	{
		return m_M.Project(m_Loc);
	}

	inline void SetMatrix(const AuMatrix4x4& m)
	{
		m_M = m;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuNamedColor
{
	QColor								m_Color;
	QString								m_Name;

public:
	inline AuNamedColor(const QString& name, const QColor& color) : m_Color(color), m_Name(name)
	{
	}

	inline AuNamedColor(const QString& name, const QString& colorName) : m_Color(colorName), m_Name(name)
	{
	}

	inline AuNamedColor(const AuNamedColor& c) : m_Color(c.m_Color), m_Name(c.m_Name)
	{
	}

public:
	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoWhich
{
	quint8								m_Which;				// Top nibble is count; bottom nibble is index. 0 is invalid

public:
	inline AuChromoWhich(unsigned int index, unsigned int howMany)
	{
		Q_ASSERT(index <= 0x0f);
		Q_ASSERT(howMany <= 0x0f);
		m_Which = (howMany << 4) | index;
	}

	inline AuChromoWhich()
	{
		m_Which = 0;	// 0 of 0 is invalid
	}

	inline AuChromoWhich(const AuChromoWhich& c)
	{
		m_Which = c.m_Which;
	}

public:
	inline const AuChromoWhich& operator=(const AuChromoWhich& c)
	{
		m_Which = c.m_Which;
		return *this;
	}

	inline bool operator==(const AuChromoWhich& c) const
	{
		if(m_Which != c.m_Which)
			return false;
		return true;
	}

	inline bool operator!=(const AuChromoWhich& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Which;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Which;
	}

public:
	inline unsigned int HowMany() const
	{
		return m_Which >> 4;	// Top nibble
	}

	inline unsigned int Index() const
	{
		return m_Which & 0x0f;	// Bottom nibble
	}

	// Test validity of data. Use Null() to simply test for the 0 case.
	// Null() is considered invalid.
	inline bool Invalid() const
	{
		if(Null())
			return false;				// Null is not valid.
		unsigned int howMany = HowMany();
		if(howMany > MaxCount())
			return false;				// Too many is not valid.
		return Index() >= howMany;		// Bad index is not valid.
	}

	// Returns the maximum number of each kind of 'which' referenced chromosome: e.g. arms, legs, eyes, ears, tits.
	// The highest index value is MaxCount()-1.
	inline static unsigned int MaxCount()
	{
		return 10;	// The real limit is 16 due to our nibble storage. 10 is just a nice game limit.
	}

	inline bool Null() const
	{
		return !m_Which;
	}

	inline quint8 Raw() const
	{
		return m_Which;
	}

	inline void SetHowMany(unsigned int howMany)
	{
		Q_ASSERT(howMany <= 0x0f);
		m_Which &= 0x0f;			// Turn off top nibble.
		m_Which |= (howMany << 4);	// Put the value into the top nibble.
	}

	inline void SetIndex(unsigned int index)
	{
		Q_ASSERT(index <= 0x0f);
		m_Which &= 0xf0;			// Turn off bottom nibble.
		m_Which |= index;			// Put the value into the bottom nibble.
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuChromoWhich& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuChromoWhich& o)
{
	o << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuChromoWhich& id);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Genes are attached to some chromosomes but genes never exist alone.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGene
{
public:
	enum GTYPE
	{
		Feathers,
		Hair,
		Scales,
		Spines,
		GTYPE_Count
	};

public:	// Construction
	inline AuGene()
	{
	}

	virtual ~AuGene()
	{
	}

	inline AuGene(const AuGene& g)
	{
		Q_UNUSED(g);
	}

public:	// Operators
	virtual void operator>>(QDataStream& out) const
	{
		Q_UNUSED(out);
	}

	virtual void ReadStream(QDataStream& in, quint8 fileVer)
	{
		Q_UNUSED(in);
		Q_UNUSED(fileVer);
	}

	virtual void WriteSpeciesKey(QDataStream& out) const
	{
		Q_UNUSED(out);
	}

	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer)
	{
		Q_UNUSED(in);
		Q_UNUSED(fileVer);
	}

	virtual bool operator==(const AuGene& gene) const
	{
		Q_UNUSED(gene);
		return true;
	}

	inline bool operator!=(const AuGene& gene) const
	{
		return !operator==(gene);
	}

public:	// Methods
	virtual AuGene* Clone() const = 0;

	static AuGene* CreateGene(GTYPE gene);

	virtual bool IsHair(AuFolliclePoint::ROLE role) const
	{
		Q_UNUSED(role);
		return false;
	}
	virtual bool IsSexuallyCompatible(const AuGene* trait) const = 0;

	virtual QString ListWidgetText() const = 0;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const = 0;

	// Prepares this gene to be added to 'genes'.
	// Returns false if this gene cannot be added to 'genes'.
	virtual bool PrepareToAdd(const AuGeneChain& genes)
	{
		Q_UNUSED(genes);
		return true;
	}

	virtual void RandomizeSpecies(const AuDice& dice)
	{
		Q_UNUSED(dice);
	}

	// Returns a number that is unique to each derived gene class.
	virtual GTYPE Type() const = 0;

	virtual QString TypeName() const = 0;
};

inline QDataStream& operator<<(QDataStream& out, const AuGene& t)
{
	t >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeneFeathers : public AuGene
{
private:
	QColor								m_Color;
	quint16								m_LengthMM;
	quint8								m_Shape;

public:	// Construction/destruction
	AuGeneFeathers(const QColor& color, quint8 shape, quint16 lengthMM);
	AuGeneFeathers();
	AuGeneFeathers(const AuGeneFeathers& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuGene& dna) const
	{
		try
		{
			const AuGeneFeathers& c = dynamic_cast<const AuGeneFeathers&>(dna);
			if(!AuGene::operator==(c))
				return false;
			if(m_Color != c.m_Color)
				return false;
			if(m_Shape != c.m_Shape)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuGene* Clone() const
	{
		return new AuGeneFeathers(*this);
	}

	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline static QString GeneName()
	{
		return QObject::tr("Feathers");
	}

	inline static GTYPE GeneType()
	{
		return Feathers;
	}

	// NOTE: When this is called, the Type() of this trait has already been compared with the Type() of 'trait'.
	virtual bool IsSexuallyCompatible(const AuGene* trait) const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetColor(const QColor& c)
	{
		if(m_Color == c)
			return false;
		m_Color = c;
		return true;
	}

	virtual GTYPE Type() const
	{
		return GeneType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Feathers");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A chromosome can have multiple hair genes, to define hair of different lengths on different areas. For example, a head might
// have long hair on top and shorter hair on the face.

class AuGeneHair : public AuGene
{
private:
	QColor								m_Color;
	quint16								m_LengthMM;				// Length of the natural uncut hair.
	quint64								m_Where;				// AuFolliclePoint::ROLE bits OR'd together.

public:	// Construction/destruction
	AuGeneHair(const QColor& color, quint16 lengthMM, quint64 where);
	AuGeneHair();
	AuGeneHair(const AuGeneHair& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuGene& dna) const
	{
		try
		{
			const AuGeneHair& c = dynamic_cast<const AuGeneHair&>(dna);
			if(!AuGene::operator==(c))
				return false;
			if(m_Color != c.m_Color)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			if(m_Where != c.m_Where)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuGene* Clone() const
	{
		return new AuGeneHair(*this);
	}

	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline static QString GeneName()
	{
		return QObject::tr("Hair");
	}

	inline static GTYPE GeneType()
	{
		return Hair;
	}

	virtual bool IsHair(AuFolliclePoint::ROLE role) const;

	// NOTE: When this is called, the Type() of this trait has already been compared with the Type() of 'trait'.
	virtual bool IsSexuallyCompatible(const AuGene* trait) const;

	inline quint16 LengthMM() const
	{
		return m_LengthMM;
	}

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetColor(const QColor& c)
	{
		if(m_Color == c)
			return false;
		m_Color = c;
		return true;
	}

	inline bool SetLengthMM(quint16 lengthMM)
	{
		if(m_LengthMM == lengthMM)
			return false;
		m_LengthMM = lengthMM;
		return true;
	}

	inline bool SetWhere(quint64 whereBits)
	{
		if(m_Where == whereBits)
			return false;
		m_Where = whereBits;
		return true;
	}

	virtual GTYPE Type() const
	{
		return GeneType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Hair");
	}

	inline quint64 Where() const
	{
		return m_Where;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// This trait adds additional visible scales. It is not the same as scaly skin, which is simply a texture.

class AuGeneScales : public AuGene
{
private:
	QColor								m_Color;				// Scales texture index. See usage.
	quint16								m_LengthMM;

public:	// Construction/destruction
	AuGeneScales(const QColor& color, quint16 lengthMM);
	AuGeneScales();
	AuGeneScales(const AuGeneScales& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuGene& dna) const
	{
		try
		{
			const AuGeneScales& c = dynamic_cast<const AuGeneScales&>(dna);
			if(!AuGene::operator==(c))
				return false;
			if(m_Color != c.m_Color)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuGene* Clone() const
	{
		return new AuGeneScales(*this);
	}

	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline static QString GeneName()
	{
		return QObject::tr("Scales");
	}

	inline static GTYPE GeneType()
	{
		return Scales;
	}

	// NOTE: When this is called, the Type() of this trait has already been compared with the Type() of 'trait'.
	virtual bool IsSexuallyCompatible(const AuGene* trait) const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetColor(const QColor& c)
	{
		if(m_Color == c)
			return false;
		m_Color = c;
		return true;
	}

	virtual GTYPE Type() const
	{
		return GeneType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Scales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGeneSpines : public AuGene
{
private:
	QColor								m_Color;				// Spines texture index. See usage.
	quint16								m_LengthMM;

public:	// Construction/destruction
	AuGeneSpines(const QColor& color, quint16 lengthMM);
	AuGeneSpines();
	AuGeneSpines(const AuGeneSpines& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuGene& dna) const
	{
		try
		{
			const AuGeneSpines& c = dynamic_cast<const AuGeneSpines&>(dna);
			if(!AuGene::operator==(c))
				return false;
			if(m_Color != c.m_Color)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuGene* Clone() const
	{
		return new AuGeneSpines(*this);
	}

	inline const QColor& Color() const
	{
		return m_Color;
	}

	inline static QString GeneName()
	{
		return QObject::tr("Spines");
	}

	inline static GTYPE GeneType()
	{
		return Spines;
	}

	// NOTE: When this is called, the Type() of this trait has already been compared with the Type() of 'trait'.
	virtual bool IsSexuallyCompatible(const AuGene* trait) const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetColor(const QColor& c)
	{
		if(m_Color == c)
			return false;
		m_Color = c;
		return true;
	}

	virtual GTYPE Type() const
	{
		return GeneType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Spines");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Chain of genes.

class AuGeneChain : public QList<AuGene*>
{
public:	// Constructors/Destructors
	inline AuGeneChain()
	{
	}

	inline ~AuGeneChain()
	{
		Clear();
	}

	AuGeneChain(const AuGeneChain& c);

public:	// Operators
	const AuGeneChain& operator=(const AuGeneChain& dna);

	bool operator==(const AuGeneChain& dna) const;

	inline bool operator!=(const AuGeneChain& dna) const
	{
		return !operator==(dna);
	}

	void operator>>(QDataStream& out) const;
	void ReadStream(QDataStream& in, quint8 fileVer);

	void WriteSpeciesKey(QDataStream& out) const;
	void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

public:	// Methods
	void AddableGenes(QMap<QString, AuGene::GTYPE>& addable) const;

	void AddGene(const AuGene& trait);

	void AddGene(AuGene::GTYPE type);

	inline void Clear()
	{
		while(!empty())
			delete takeFirst();
	}

	// Returns the first hair gene dna that fits the role. Returns 0 if none.
	const AuGeneHair* HairDNA(AuFolliclePoint::ROLE role) const;

	// Returns the maximum number of each kind of gene that can be added.
	inline static quint8 MaxGeneCount()
	{
		return 10;
	}

	void RandomizeSpecies(const AuDice& dice);
};

inline QDataStream& operator<<(QDataStream& out, const AuGeneChain& dna)
{
	dna >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Chromosomes are always paired.

class AuChromosome
{
public:
	enum CTYPE
	{
		// Phylum
		Annelida,
		Arthropoda,
		Chordata,
		Mollusca,

		// Body Covering
		Feathers,					// Body covered by feathers.
		Fur,						// Body covered by fur.
		Scales,						// Body covered by scales.
		Skin,						// Body covered by skin.

		// Body Sections
		// Body must have a torso or thorax to attach limbs.
		Head,						// All animals must have a head.
		Torso,						// Most mammals. Thorax for insects.

		// Limbs
		Arm,
		Leg,
		Fin,
		Tentacle,
		Wing,

		// Head Features
		Antenna,
		Horn,
		Ear,
		Eye,
		Nose,
		Mouth,

		// Internal
		Lungs,						// Breathes in air.
		Gills,						// Breathes in water.
		Blood,						// Warm or cold blooded.

		// Food
		EatsAnimals,
		EatsPlants,

		// Young
		Reproduction,				// Asexual, sexual.
		Mammary,					// Produces milk.

		CTYPE_Count
	};

	enum LATERALITY
	{
		Left,
		Middle,
		Right,
	};

public:	// Constructors/Destructors
	inline AuChromosome()
	{
	}

	virtual ~AuChromosome()
	{
	}

	inline AuChromosome(const AuChromosome& c)
	{
		Q_UNUSED(c);
	}

	// Used when combining during mating.
	inline AuChromosome(const AuChromosome& c1, const AuChromosome& c2, const AuDice& dice)
	{
		Q_UNUSED(c1);
		Q_UNUSED(c2);
		Q_UNUSED(dice);
	}

public:	// Operators
	virtual void Copy(const AuChromosome& o)
	{
		Q_UNUSED(o);
	}

	virtual void operator>>(QDataStream& out) const
	{
		Q_UNUSED(out);
	}

	virtual void ReadStream(QDataStream& in, quint8 fileVer)
	{
		Q_UNUSED(in);
		Q_UNUSED(fileVer);
	}

	virtual void WriteSpeciesKey(QDataStream& out) const
	{
		Q_UNUSED(out);
	}

	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer)
	{
		Q_UNUSED(in);
		Q_UNUSED(fileVer);
	}

	virtual bool operator==(const AuChromosome& /*dna*/) const
	{
		return true;
	}

	inline bool operator!=(const AuChromosome& dna) const
	{
		return !operator==(dna);
	}

public:	// Methods
	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 0;
	}

	// Returns the location of the bone in the unposed model.
	// 'dna' is my DNA.
	// 'bone' is my bone index, among my BoneCount() bones. 0 <= bone < BoneCount().
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(bone);
		return AuPoint(0,0);
	}

	// Returns a duplicate of this chromosome, allocated on the heap.
	virtual AuChromosome* Clone() const = 0;

	// Returns mated combination of this chromosome and 'c'. Caller takes ownership of the pointer.
	// Returns 0 if 'c' is not the same dynamic cast as this chromosome.
	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const = 0;

	static qint16 CombineAngle(qint16 a1, qint16 a2, const AuDice& dice);

	static quint64 CombineBits(quint64 b1, quint64 b2, const AuDice& dice);

	static bool CombineBool(bool b1, bool b2, const AuDice& dice);

	static QColor CombineColor(const QColor& c1, const QColor& c2, const AuDice& dice);

	static int CombineEnum(int e1, int e2, const AuDice& dice);

	// Combines 'v1' and 'v2'. Can return a number anywhere from v1 to v2, inclusive.
	static quint16 CombineRange(quint16 r1, quint16 r2, const AuDice& dice);

	static AuChromosome* CreateChromosome(CTYPE chromosome);

	// Returns the farthest distance in +Y on the face of 'head', for construction of a helmet.
	virtual float FaceRadiusY(const AuChromoHead& head) const
	{
		Q_UNUSED(head);
		return 0;
	}

	// Gets the mesh used to make clothing to cover the naked body model.
	// 'mesh' is the mesh of the final model.
	// 'dna' is my DNA.
	// 'firstBone' is the first of my BoneCount() bones.
	virtual void GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const
	{
		Q_UNUSED(mesh);
		Q_UNUSED(dna);
		Q_UNUSED(firstBone);
		Q_UNUSED(stage);
	}

	// Gets the mesh used to make the naked body model.
	// 'mesh' is the mesh of the final model.
	// 'dna' is my DNA.
	// 'firstBone' is the first of my BoneCount() bones.
	// 'hairStyle' describes hair styling on the head.
	virtual void GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
			const AuClothing& clothes, GROWTHSTAGE stage) const
	{
		Q_UNUSED(mesh);
		Q_UNUSED(dna);
		Q_UNUSED(firstBone);
		Q_UNUSED(hairStyle);
		Q_UNUSED(clothes);
		Q_UNUSED(stage);
	}

	// Each eye for the skull adds its mesh and returns true.
	virtual bool GetSkullEye(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
			quint8 bone) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mSkull);
		Q_UNUSED(head);
		Q_UNUSED(bone);
		return false;
	}

	// Adds to 'mesh' any parts this chromosome adds to the skull of 'head', not including the skull itself.
	// 'dna' is mine.
	virtual void GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mSkull);
		Q_UNUSED(head);
	}

	// Adds to 'mesh' any parts this chromosome adds to the skull of 'head', not including the skull itself.
	// 'dna' is mine.
	virtual void GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mSkull);
		Q_UNUSED(head);
	}

	// Eye and ear locations with radius, for excluding head hair.
	virtual void GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx,
			const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(lockWid);
		Q_UNUSED(hair);
		Q_UNUSED(hairEx);
		Q_UNUSED(mSkull);
		Q_UNUSED(head);
		Q_UNUSED(stage);
	}

	// The one and only jaw for the skull returns true.
	virtual bool GetSkullJaw(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
			quint8 bone) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mSkull);
		Q_UNUSED(head);
		Q_UNUSED(bone);
		return false;
	}

	// Adds to 'mesh' any parts this chromosome adds to the 'torso', not including the skull itself.
	// 'dna' is mine.
	virtual void GetTorsoFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
			const AuChromoTorso& torso, GROWTHSTAGE stage) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mTorso);
		Q_UNUSED(torso);
		Q_UNUSED(stage);
	}

	// Adds to 'mesh' any parts this chromosome adds to the 'torso' when clothed, not including the skull itself.
	// 'dna' is mine.
	virtual void GetTorsoFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
			const AuChromoTorso& torso, GROWTHSTAGE stage) const
	{
		Q_UNUSED(dna);
		Q_UNUSED(mesh);
		Q_UNUSED(mTorso);
		Q_UNUSED(torso);
		Q_UNUSED(stage);
	}

	// Returns a mask of AuFolliclePoint::ROLE bits to indicate where hair can be placed on this chromosome, if any.
	// You must override this function to be permitted to add hair as a gene.
	// This is called from a default constructed instance of this object. It can't really depend on my properties.
	virtual quint64 HairWhereBits(const AuChromoWhich& which) const
	{
		Q_UNUSED(which);
		return 0;
	}

	static quint64 HairWhereBits(CTYPE chromosome, const AuChromoWhich& which);

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const = 0;

	inline static QString LateralityName(LATERALITY laterality)
	{
		QString lnames[] =
		{
			QObject::tr("Left"),
			QObject::tr("Middle"),
			QObject::tr("Right"),
		};
		return lnames[laterality];
	}

	// Returns a number used for sorting chromosome in a list ui.
	virtual int ListWidgetOrder() const = 0;

	// Returns the text name of this chromosome when listed in a list ui. The text can contain no HTML formatting.
	virtual QString ListWidgetText() const = 0;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const = 0;

	// Returns my CTYPE, if I am a chromosome that defines the phylum of the creature.
	virtual CTYPE Phylum() const
	{
		return CTYPE_Count;
	}

	// Returns a number that is used for sorting parts so bones on base parts are posed before subordinate parts are posed.
	// The list of chromosome is sorted based on pose order, lowest to highest.
	// That way torso is posed before neck, which must be posed before head. i.e. torso order < neck < head.
	// Parts that do not have bones should return the default value.
	virtual quint8 PoseOrder() const
	{
		return 0xff;
	}

	// Prepares this chromosome to be added to 'chroma', mainly to configure my which id, if any.
	// Returns false if this part cannot be added to 'chroma', most likely due to MaxWhichCount reached.
	virtual bool PrepareToAdd(const AuChromatid& chroma)
	{
		Q_UNUSED(chroma);
		return true;
	}

	virtual void RandomizeSpecies(const AuDice& dice)
	{
		Q_UNUSED(dice);
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		Q_UNUSED(which);
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const
	{
		Q_UNUSED(min);
		Q_UNUSED(max);
		Q_UNUSED(bones);
	}

	// Tests for intersection by segment from 'eyeLoc' to 'sightedLoc'. 'bones' contains my current bones.
	// Returns false if not hit.
	// Returns true if hit and...
	// - Adjusts 'sightedLoc' to the hit location when hit, presumably closer to 'eyeLoc'.
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const
	{
		Q_UNUSED(eyeLoc);
		Q_UNUSED(sightedLoc);
		Q_UNUSED(bones);
		return false;
	}

	// Returns a number that is unique to each derived chromosome class.
	virtual CTYPE Type() const = 0;

	virtual QString TypeName() const = 0;

	// Return the volume of this part in cubic feet. Used for hit points assessment.
	virtual float Volume() const
	{
		return 0;
	}

	// For chromosome that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return AuChromoWhich();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuChromosome& t)
{
	t >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromosomeWithGenes : public AuChromosome
{
	AuGeneChain*						m_Genes;				// Not allocated until needed.

public:	// Constructors/Destructors
	inline AuChromosomeWithGenes()
	{
		m_Genes = 0;
	}

	virtual ~AuChromosomeWithGenes()
	{
		if(m_Genes)
			delete m_Genes;
	}

	inline AuChromosomeWithGenes(const AuChromosomeWithGenes& c) : AuChromosome(c)
	{
		if(c.m_Genes)
			m_Genes = new AuGeneChain(*c.m_Genes);
		else
			m_Genes = 0;
	}

	AuChromosomeWithGenes(const AuChromosomeWithGenes& c1, const AuChromosomeWithGenes& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromosomeWithGenes& c = dynamic_cast<const AuChromosomeWithGenes&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Genes && c.m_Genes)
			{
				// Both have genes.
				if(*m_Genes != *c.m_Genes)
					return false;	// Genes don't match.
			}
			else if(m_Genes || c.m_Genes)
				return false;		// One of the sides has Genes but not the other.
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
		return true;
	}

	inline bool operator!=(const AuChromosome& dna) const
	{
		return !operator==(dna);
	}

public:	// Methods
	void AddGene(const AuGene& trait);

	// Finds my hair gene for a specific hair follicle role. Returns 0 if there is no hair for that role.
	const AuGeneHair* HairDNA(AuFolliclePoint::ROLE role) const;

	bool SetGenes(const AuGeneChain& genes);

	// This can return 0. The genes are not allocated until used.
	inline const AuGeneChain* Genes() const
	{
		return m_Genes;
	}

	// This can return 0. The genes are not allocated until used.
	inline AuGeneChain* Genes()
	{
		return m_Genes;
	}

	virtual void RandomizeSpecies(const AuDice& dice);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Phylum
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoAnnelida : public AuChromosome
{
public:	// Construction/destruction
	AuChromoAnnelida();
	AuChromoAnnelida(const AuChromoAnnelida& c);
	AuChromoAnnelida(const AuChromoAnnelida& c1, const AuChromoAnnelida& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoAnnelida& c = dynamic_cast<const AuChromoAnnelida&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoAnnelida(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Annelida");
	}

	inline static CTYPE ChromosomeType()
	{
		return Annelida;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual CTYPE Phylum() const
	{
		return ChromosomeType();
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Annelida");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoArthropoda : public AuChromosome
{
public:	// Construction/destruction
	AuChromoArthropoda();
	AuChromoArthropoda(const AuChromoArthropoda& c);
	AuChromoArthropoda(const AuChromoArthropoda& c1, const AuChromoArthropoda& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoArthropoda& c = dynamic_cast<const AuChromoArthropoda&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoArthropoda(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Arthropoda");
	}

	inline static CTYPE ChromosomeType()
	{
		return Arthropoda;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual CTYPE Phylum() const
	{
		return ChromosomeType();
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Arthropoda");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoChordata : public AuChromosome
{
public:
	enum SKELETON
	{
		Bones,
		Cartilage,
	};

private:
	SKELETON							m_Skeleton;

public:	// Construction/destruction
	AuChromoChordata(SKELETON skeleton);
	AuChromoChordata();
	AuChromoChordata(const AuChromoChordata& c);
	AuChromoChordata(const AuChromoChordata& c1, const AuChromoChordata& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoChordata& c = dynamic_cast<const AuChromoChordata&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Skeleton != c.m_Skeleton)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoChordata(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Chordata");
	}

	inline static CTYPE ChromosomeType()
	{
		return Chordata;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual CTYPE Phylum() const
	{
		return ChromosomeType();
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetSkeleton(SKELETON skeleton)
	{
		if(m_Skeleton == skeleton)
			return false;
		m_Skeleton = skeleton;
		return true;
	}

	inline SKELETON Skeleton() const
	{
		return m_Skeleton;
	}

	inline QString SkeletonName() const
	{
		return SkeletonName(m_Skeleton);
	}

	static QString SkeletonName(SKELETON skel);

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Chordata");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoMollusca : public AuChromosome
{
public:
	enum COVER
	{
		Mantle,
		Shell,
	};

private:
	COVER								m_Cover;

public:	// Construction/destruction
	AuChromoMollusca(COVER cover);
	AuChromoMollusca();
	AuChromoMollusca(const AuChromoMollusca& c);
	AuChromoMollusca(const AuChromoMollusca& c1, const AuChromoMollusca& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoMollusca& c = dynamic_cast<const AuChromoMollusca&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Cover != c.m_Cover)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoMollusca(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Mollusca");
	}

	inline static CTYPE ChromosomeType()
	{
		return Mollusca;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline COVER Cover() const
	{
		return m_Cover;
	}

	inline QString CoverName() const
	{
		return CoverName(m_Cover);
	}

	static QString CoverName(COVER skel);

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual CTYPE Phylum() const
	{
		return ChromosomeType();
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetCover(COVER cover)
	{
		if(m_Cover == cover)
			return false;
		m_Cover = cover;
		return true;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Mollusca");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Body covering.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoSkin : public AuChromosome
{
private:
	QColor								m_BottomColor;			// Color modulation for bottom skin.
	QColor								m_TopColor;				// Color modulation for top skin.

public:	// Construction/destruction
	AuChromoSkin(const QColor& topColor, const QColor& bottomColor);
	AuChromoSkin();
	AuChromoSkin(const AuChromoSkin& c);
	AuChromoSkin(const AuChromoSkin& c1, const AuChromoSkin& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoSkin& c = dynamic_cast<const AuChromoSkin&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_BottomColor != c.m_BottomColor)
				return false;
			if(m_TopColor != c.m_TopColor)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline const QColor& BottomColor() const
	{
		return m_BottomColor;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoSkin(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Skin");
	}

	inline static CTYPE ChromosomeType()
	{
		return Skin;
	}

	// Returns a skin color from the table of known skin colors.
	static QColor Color(int color);

	// Returns a skin color name from the table of known skin colors.
	static QString ColorName(int color);

	// Returns the number of skin colors in the table of known skin colors.
	static int Colors();

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetBottomColor(const QColor& color)
	{
		if(m_BottomColor == color)
			return false;
		m_BottomColor = color;
		return true;
	}

	inline bool SetTopColor(const QColor& color)
	{
		if(m_TopColor == color)
			return false;
		m_TopColor = color;
		return true;
	}

	// 'topSkinTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinColor' is used to modulate the skin color looked up from the texture.
	// 'bottomSkinTexId' is the string id of the bottom skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinTexFile' is the file that contains the bottom skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinColor' is used to modulate the skin color looked up from the texture.
	void SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
			QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const;

	inline const QColor& TopColor() const
	{
		return m_TopColor;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Skin");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoFeathers : public AuChromosome
{
private:
	QColor								m_BottomColor;			// Color modulation for bottom skin.
	QColor								m_TopColor;				// Color modulation for top skin.

public:	// Construction/destruction
	AuChromoFeathers(const QColor& topColor, const QColor& bottomColor);
	AuChromoFeathers();
	AuChromoFeathers(const AuChromoFeathers& c);
	AuChromoFeathers(const AuChromoFeathers& c1, const AuChromoFeathers& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoFeathers& c = dynamic_cast<const AuChromoFeathers&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_BottomColor != c.m_BottomColor)
				return false;
			if(m_TopColor != c.m_TopColor)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline const QColor& BottomColor() const
	{
		return m_BottomColor;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoFeathers(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Feathers");
	}

	inline static CTYPE ChromosomeType()
	{
		return Feathers;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetBottomColor(const QColor& color)
	{
		if(m_BottomColor == color)
			return false;
		m_BottomColor = color;
		return true;
	}

	inline bool SetTopColor(const QColor& color)
	{
		if(m_TopColor == color)
			return false;
		m_TopColor = color;
		return true;
	}

	// 'topSkinTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinColor' is used to modulate the skin color looked up from the texture.
	// 'bottomSkinTexId' is the string id of the bottom skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinTexFile' is the file that contains the bottom skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinColor' is used to modulate the skin color looked up from the texture.
	void SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
			QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const;

	inline const QColor& TopColor() const
	{
		return m_TopColor;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Feathers");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoFur : public AuChromosome
{
private:
	QColor								m_BottomColor;			// Color modulation for bottom skin.
	QColor								m_TopColor;				// Color modulation for top skin.

public:	// Construction/destruction
	AuChromoFur(const QColor& topColor, const QColor& bottomColor);
	AuChromoFur();
	AuChromoFur(const AuChromoFur& c);
	AuChromoFur(const AuChromoFur& c1, const AuChromoFur& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoFur& c = dynamic_cast<const AuChromoFur&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_BottomColor != c.m_BottomColor)
				return false;
			if(m_TopColor != c.m_TopColor)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline const QColor& BottomColor() const
	{
		return m_BottomColor;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoFur(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Fur");
	}

	inline static CTYPE ChromosomeType()
	{
		return Fur;
	}

	// Returns a hair color from the table of known hair colors.
	static QColor Color(int color);

	// Returns a hair color name from the table of known hair colors.
	static QString ColorName(int color);

	// Returns the number of hair colors in the table of known hair colors.
	static int Colors();

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetBottomColor(const QColor& color)
	{
		if(m_BottomColor == color)
			return false;
		m_BottomColor = color;
		return true;
	}

	inline bool SetTopColor(const QColor& color)
	{
		if(m_TopColor == color)
			return false;
		m_TopColor = color;
		return true;
	}

	// 'topSkinTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinColor' is used to modulate the skin color looked up from the texture.
	// 'bottomSkinTexId' is the string id of the bottom skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinTexFile' is the file that contains the bottom skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinColor' is used to modulate the skin color looked up from the texture.
	void SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
			QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const;

	inline const QColor& TopColor() const
	{
		return m_TopColor;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Fur");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoScales : public AuChromosome
{
private:
	QColor								m_Color;				// Scales texture index. See usage.
	quint8								m_Shape;

public:	// Construction/destruction
	AuChromoScales(const QColor& color, quint8 shape);
	AuChromoScales();
	AuChromoScales(const AuChromoScales& c);
	AuChromoScales(const AuChromoScales& c1, const AuChromoScales& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoScales& c = dynamic_cast<const AuChromoScales&>(dna);
			if(!AuChromosome::operator==(c))
				return false;

			if(m_Color != c.m_Color)
				return false;
			if(m_Shape != c.m_Shape)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoScales(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Scales");
	}

	inline static CTYPE ChromosomeType()
	{
		return Scales;
	}

	inline const QColor& Color() const
	{
		return m_Color;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetColor(const QColor& c)
	{
		if(m_Color == c)
			return false;
		m_Color = c;
		return true;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Scales");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Body Sections.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoHead : public AuChromosomeWithGenes
{
public:
	enum ATTACHMENT
	{
		AttachBottom,					// Neck/torso is attached at the bottom of the head, like a human or a dog.
		AttachBack,						// Neck/torso is attached at the back of the head, like an alligator or snake.
	};

private:
	ATTACHMENT							m_Attach;
	quint16								m_AttachDistMM;			// Distance from center for attachment, in m_Attach direction.
	qint16								m_EarAngleDeg;			// Angle from top of head down to ear(s), in degrees.
	qint16								m_EyeAngleDeg;			// Angle from front of head side-to-side to eye(s), in degrees.
	qint16								m_MouthAngleDeg;		// Angle from top of head down to mouth, in degrees.
	qint16								m_NoseAngleDeg;			// Angle from top of head down to nose, in degrees.
	quint16								m_WidthMM;				// Side-to-side
	quint16								m_LengthMM;				// Back-to-front
	quint16								m_HeightMM;				// Bottom-to-top

public:	// Construction/destruction
	AuChromoHead(ATTACHMENT attach, quint16 attachOffMM, quint16 widthMM, quint16 lengthMM, quint16 heightMM, qint16 earAngleDeg,
			qint16 eyeAngleDeg, qint16 mouthAngleDeg, qint16 noseAngleDeg);
	AuChromoHead();
	AuChromoHead(const AuChromoHead& c);
	AuChromoHead(const AuChromoHead& c1, const AuChromoHead& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoHead& c = dynamic_cast<const AuChromoHead&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Attach != c.m_Attach)
				return false;
			if(m_AttachDistMM != c.m_AttachDistMM)
				return false;
			if(m_EarAngleDeg != c.m_EarAngleDeg)
				return false;
			if(m_EyeAngleDeg != c.m_EyeAngleDeg)
				return false;
			if(m_MouthAngleDeg != c.m_MouthAngleDeg)
				return false;
			if(m_NoseAngleDeg != c.m_NoseAngleDeg)
				return false;
			if(m_WidthMM != c.m_WidthMM)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			if(m_HeightMM != c.m_HeightMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose, const AuPose& mouthPose,
			float currentHeadTilt, float currentHeadTurn, HANDEDNESS hand,
			float& headX, float& headY, float& headZ, float& eyeLeftX, float& eyeLeftZ, float& eyeRightX,
			float& eyeRightZ, float& jawX, float& jawLeftY, float& jawRightY);

	inline quint16 AttachDistanceMM() const
	{
		return m_AttachDistMM;
	}

	inline ATTACHMENT Attachment() const
	{
		return (ATTACHMENT)m_Attach;
	}

	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	// When I build my mesh, I add the eyes and jaw myself so I must account for their bones too.
	virtual quint8 BoneCount(const AuChromatid& dna) const;

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	// Returns vector from neck attachment (bone 0) to center of head ellipsoid, based on m_Attach and m_AttachDist.
	AuPoint CenterOffset() const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoHead(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Head");
	}

	inline static CTYPE ChromosomeType()
	{
		return Head;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline qint16 EarAngleDeg() const
	{
		return m_EarAngleDeg;
	}

	inline qint16 EyeAngleDeg() const
	{
		return m_EyeAngleDeg;
	}

	// Returns the location of 'whichEar' in the unrotated head construction model. i.e. +X right, +Z up, facing down the +Y axis.
	AuPoint EarLocation(const AuChromoWhich& which) const;

	// Returns the location of 'whichEye' in the unrotated head construction model. i.e. +X right, +Z up, facing down the +Y axis.
	// Uses 'dna' to find the eye, to determine its diameter. If you already know that, call the other version instead.
	AuPoint EyeLocation(const AuChromoWhich& which, const AuChromatid& dna) const;

	AuPoint EyeLocation(const AuChromoWhich& which, float eyeRad) const;

	virtual float FaceRadiusY(const AuChromoHead& head) const
	{
		Q_UNUSED(head);
		return RadiusY();
	}

	virtual void GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const;

	virtual void GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
			const AuClothing& clothes, GROWTHSTAGE stage) const;

	void HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	inline qint16 MouthAngleDeg() const
	{
		return m_MouthAngleDeg;
	}

	// Returns the location of the mouth in the unrotated head construction model. i.e. +X right, +Z up, facing down the +Y axis.
	AuPoint MouthLocation() const;

	inline qint16 NoseAngleDeg() const
	{
		return m_NoseAngleDeg;
	}

	// Returns the location of the nose in the unrotated head construction model. i.e. +X right, +Z up, facing down the +Y axis.
	AuPoint NoseLocation() const;

	virtual quint8 PoseOrder() const
	{
		return 3;
	}

	inline float RadiusX() const
	{
		return (float)m_WidthMM * FeetPerMM() / 2.f;
	}

	inline float RadiusY() const
	{
		return (float)m_LengthMM * FeetPerMM() / 2.f;
	}

	inline float RadiusZ() const
	{
		return (float)m_HeightMM * FeetPerMM() / 2.f;
	}

	inline quint16 WidthMM() const
	{
		return m_WidthMM;
	}

	inline quint16 LengthMM() const
	{
		return m_LengthMM;
	}

	inline quint16 HeightMM() const
	{
		return m_HeightMM;
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetAttachDistanceMM(quint16 dist)
	{
		if(m_AttachDistMM == dist)
			return false;
		m_AttachDistMM = dist;
		return true;
	}

	inline bool SetAttachment(ATTACHMENT attach)
	{
		if(m_Attach == attach)
			return false;
		m_Attach = attach;
		return true;
	}

	inline bool SetEarAngleDeg(qint16 deg)
	{
		if(m_EarAngleDeg == deg)
			return false;
		m_EarAngleDeg = deg;
		return true;
	}

	inline bool SetEyeAngleDeg(qint16 deg)
	{
		if(m_EyeAngleDeg == deg)
			return false;
		m_EyeAngleDeg = deg;
		return true;
	}

	inline bool SetHeightMM(quint16 heightMM)
	{
		if(m_HeightMM == heightMM)
			return false;
		m_HeightMM = heightMM;
		return true;
	}

	inline bool SetMouthAngleDeg(qint16 deg)
	{
		if(m_MouthAngleDeg == deg)
			return false;
		m_MouthAngleDeg = deg;
		return true;
	}

	inline bool SetNoseAngleDeg(qint16 deg)
	{
		if(m_NoseAngleDeg == deg)
			return false;
		m_NoseAngleDeg = deg;
		return true;
	}

	inline bool SetLengthMM(quint16 lengthMM)
	{
		if(m_LengthMM == lengthMM)
			return false;
		m_LengthMM = lengthMM;
		return true;
	}

	inline bool SetWidthMM(quint16 widthMM)
	{
		if(m_WidthMM == widthMM)
			return false;
		m_WidthMM = widthMM;
		return true;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const;

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Head");
	}

	virtual float Volume() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoTorso : public AuChromosomeWithGenes
{
public:
	enum ORIENTATION
	{
		Horizontal,						// Horse
		Vertical,						// Human
		Bent,							// Centaur
	};

private:
	quint16								m_LowerLengthMM;		// Length of lower spine. Waist down on human, waist back on centaur
	quint16								m_LowerThicknessMM;		// Back to front dimension.
	quint16								m_LowerWidthMM;			// Side to side dimension.
	quint16								m_NeckLengthMM;
	quint16								m_TailLengthMM;
	quint16								m_UpperLengthMM;		// Length of upper spine. Waist up on human or centaur.
	quint16								m_UpperThicknessMM;		// Back to front dimension.
	quint16								m_UpperWidthMM;			// Side to side dimension.

public:	// Construction/destruction
	AuChromoTorso(quint16 neckLengthMM, quint16 upperLengthMM, quint16 upperWidthMM, quint16 upperThicknessMM,
			quint16 lowerLengthMM, quint16 lowerWidthMM, quint16 lowerThicknessMM, quint16 tailLengthMM);
	AuChromoTorso();
	AuChromoTorso(const AuChromoTorso& c);
	AuChromoTorso(const AuChromoTorso& c1, const AuChromoTorso& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoTorso& c = dynamic_cast<const AuChromoTorso&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_LowerLengthMM != c.m_LowerLengthMM)
				return false;
			if(m_LowerThicknessMM != c.m_LowerThicknessMM)
				return false;
			if(m_LowerWidthMM != c.m_LowerWidthMM)
				return false;
			if(m_NeckLengthMM != c.m_NeckLengthMM)
				return false;
			if(m_TailLengthMM != c.m_TailLengthMM)
				return false;
			if(m_UpperLengthMM != c.m_UpperLengthMM)
				return false;
			if(m_UpperThicknessMM != c.m_UpperThicknessMM)
				return false;
			if(m_UpperWidthMM != c.m_UpperWidthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
			float currentHeadTilt, float currentHeadTurn, HANDEDNESS hand, bool hasMaleGenitals,
			float& buttX, float& buttY, float& buttZ, float& chestX, float& chestY, float& chestZ, float& neckX, float& neckY,
			float& neckZ, float& tailX, float& tailY);

	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 2 + NeckBoneCount() + TailBoneCount();
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	// Thickness (front to back in Y) of unposed torso meat. Lower torso is preferred if any, otherwise upper torso.
	inline float BottomThickness() const
	{
		if(m_LowerLengthMM)
			return m_LowerThicknessMM * FeetPerMM();
		else
			return m_UpperThicknessMM * FeetPerMM();
	}

	// Returns the calculated location of a breast, adjusted forward/backward to hit the torso mesh.
	AuPoint BreastLocation(const AuChromatid& dna, const AuMeatMesh& torsoMesh, const AuChromoWhich& which) const;

	// Returns the calculated location of a breast, without adjusting it forward/backward to hit the torso mesh.
	AuPoint BreastLocationNotOnSkin(const AuChromatid& dna, const AuChromoWhich& which) const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoTorso(*this);
	}

	inline static QString ChromosomeName(bool hasArthropoda)
	{
		if(hasArthropoda)
			return QObject::tr("Thorax");
		else
			return QObject::tr("Torso");
	}

	inline static CTYPE ChromosomeType()
	{
		return Torso;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	virtual void GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const;

	virtual void GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
			const AuClothing& clothes, GROWTHSTAGE stage) const;

	void HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	inline quint16 LowerLengthMM() const
	{
		return m_LowerLengthMM;
	}

	inline quint16 LowerThicknessMM() const
	{
		return m_LowerThicknessMM;
	}

	inline quint16 LowerWidthMM() const
	{
		return m_LowerWidthMM;
	}

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	inline quint8 NeckBoneCount() const
	{
		return 2;
	}

	inline quint16 NeckLengthMM() const
	{
		return m_NeckLengthMM;
	}

	inline ORIENTATION Orientation() const
	{
		if(m_LowerLengthMM && m_UpperLengthMM)
			return Bent;
		else if(m_LowerLengthMM)
			return Horizontal;
		else
			return Vertical;
	}

	virtual quint8 PoseOrder() const
	{
		return 1;
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetLowerLengthMM(quint16 mm)
	{
		if(m_LowerLengthMM == mm)
			return false;
		m_LowerLengthMM = mm;
		return true;
	}

	inline bool SetLowerThicknessMM(quint16 mm)
	{
		if(m_LowerThicknessMM == mm)
			return false;
		m_LowerThicknessMM = mm;
		return true;
	}

	inline bool SetLowerWidthMM(quint16 mm)
	{
		if(m_LowerWidthMM == mm)
			return false;
		m_LowerWidthMM = mm;
		return true;
	}

	inline bool SetNeckLengthMM(quint16 mm)
	{
		if(m_NeckLengthMM == mm)
			return false;
		m_NeckLengthMM = mm;
		return true;
	}

	inline bool SetTailLengthMM(quint16 mm)
	{
		if(m_TailLengthMM == mm)
			return false;
		m_TailLengthMM = mm;
		return true;
	}

	inline bool SetUpperLengthMM(quint16 mm)
	{
		if(m_UpperLengthMM == mm)
			return false;
		m_UpperLengthMM = mm;
		return true;
	}

	inline bool SetUpperThicknessMM(quint16 mm)
	{
		if(m_UpperThicknessMM == mm)
			return false;
		m_UpperThicknessMM = mm;
		return true;
	}

	inline bool SetUpperWidthMM(quint16 mm)
	{
		if(m_UpperWidthMM == mm)
			return false;
		m_UpperWidthMM = mm;
		return true;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const;

	inline quint8 TailBoneCount() const
	{
		return m_TailLengthMM ? 2 : 0;
	}

	inline quint16 TailLengthMM() const
	{
		return m_TailLengthMM;
	}

	// Returns the volume of the tail in cubic feet, for weapon calculations.
	float TailVolume() const;

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Torso");
	}

	inline quint16 UpperLengthMM() const
	{
		return m_UpperLengthMM;
	}

	inline quint16 UpperThicknessMM() const
	{
		return m_UpperThicknessMM;
	}

	inline quint16 UpperWidthMM() const
	{
		return m_UpperWidthMM;
	}

	virtual float Volume() const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Limbs.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoArm : public AuChromosomeWithGenes
{
public:
	enum LEGBITS
	{
		LEG_Walk						= 0x01,					// Arm is used as a leg when walking.
		LEG_Run							= 0x02,					// Arm is used as a leg when running.
		LEG_Jump						= 0x04,					// Arm is used as a leg when jumping.
	};

private:
	quint16								m_ElbowRadMM;
	quint16								m_ForeArmLengthMM;		// From elbow to wrist.
	quint16								m_HandLengthMM;			// From wrist to tip of hand.
	quint8								m_LegBits;				// OR'd LEGBITS
	quint16								m_ShoulderRadMM;
	quint16								m_ShoulderToSpineMM;	// Distance from center line to joint.
	quint8								m_StandTone;			// 0 to 100 muscle tone when standing. 100 = straight, 0 = limp
	quint16								m_UpperArmLengthMM;		// From joint to elbow.
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.
	quint16								m_WristRadMM;

public:	// Construction/destruction
	AuChromoArm(const AuChromoWhich& which, quint16 shoulderToSpineMM, quint16 shoulderRadMM, quint16 upperArmLengthMM,
			quint16 elbowRadMM, quint16 foreArmLenMM, quint16 wristRadMM, quint16 handLenMM, quint8 legBits, quint8 standTone);
	AuChromoArm();
	AuChromoArm(const AuChromoArm& c);
	AuChromoArm(const AuChromoArm& c1, const AuChromoArm& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoArm& c = dynamic_cast<const AuChromoArm&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_ElbowRadMM != c.m_ElbowRadMM)
				return false;
			if(m_ForeArmLengthMM != c.m_ForeArmLengthMM)
				return false;
			if(m_HandLengthMM != c.m_HandLengthMM)
				return false;
			if(m_LegBits != c.m_LegBits)
				return false;
			if(m_ShoulderRadMM != c.m_ShoulderRadMM)
				return false;
			if(m_ShoulderToSpineMM != c.m_ShoulderToSpineMM)
				return false;
			if(m_StandTone != c.m_StandTone)
				return false;
			if(m_UpperArmLengthMM != c.m_UpperArmLengthMM)
				return false;
			if(m_WristRadMM != c.m_WristRadMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
			float currentHeadTilt, HANDEDNESS hand, bool bendToHoldObject,
			float& shoulderX, float& shoulderY, float& shoulderZ, float& elbowX, float& elbowZ, float& wristX, float& wristY,
			float& wristZ, float& fingerY, float& fingerTipY) const;

	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 5;
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	void CalcStandAngles(const AuPose& pose, float strength, float& shoulderX, float& shoulderY, float& shoulderZ, float& elbowX,
			float& elbowZ, float& wristX, float& wristY, float& fingerY, float& fingerTipY) const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoArm(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Arm");
	}

	inline static CTYPE ChromosomeType()
	{
		return Arm;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline quint16 ElbowRadiusMM() const
	{
		return m_ElbowRadMM;
	}

	inline quint16 ForeArmLengthMM() const
	{
		return m_ForeArmLengthMM;
	}

	virtual void GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const;

	virtual void GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
			const AuClothing& clothes, GROWTHSTAGE stage) const;

	void HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	inline quint16 HandLengthMM() const
	{
		return m_HandLengthMM;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	inline bool Jumps() const
	{
		return m_LegBits & LEG_Jump;
	}

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	inline quint16 LengthMM() const
	{
		return m_UpperArmLengthMM + m_ForeArmLengthMM + m_HandLengthMM;
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual quint8 PoseOrder() const
	{
		return 2;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool Runs() const
	{
		return m_LegBits & LEG_Run;
	}

	inline bool SetElbowRadiusMM(quint16 mm)
	{
		if(m_ElbowRadMM == mm)
			return false;
		m_ElbowRadMM = mm;
		return true;
	}

	inline bool SetForeArmLengthMM(quint16 mm)
	{
		if(m_ForeArmLengthMM == mm)
			return false;
		m_ForeArmLengthMM = mm;
		return true;
	}

	inline bool SetHandLengthMM(quint16 mm)
	{
		if(m_HandLengthMM == mm)
			return false;
		m_HandLengthMM = mm;
		return true;
	}

	inline bool SetLegBits(quint8 bits)
	{
		if(m_LegBits == bits)
			return false;
		m_LegBits = bits;
		return true;
	}

	inline bool SetShoulderToSpineMM(quint16 mm)
	{
		if(m_ShoulderToSpineMM == mm)
			return false;
		m_ShoulderToSpineMM = mm;
		return true;
	}

	inline bool SetShoulderRadiusMM(quint16 mm)
	{
		if(m_ShoulderRadMM == mm)
			return false;
		m_ShoulderRadMM = mm;
		return true;
	}

	inline bool SetStandTone(quint8 tone)
	{
		if(m_StandTone == tone)
			return false;
		m_StandTone = tone;
		return true;
	}

	inline bool SetUpperArmLengthMM(quint16 mm)
	{
		if(m_UpperArmLengthMM == mm)
			return false;
		m_UpperArmLengthMM = mm;
		return true;
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	inline bool SetWristRadiusMM(quint16 mm)
	{
		if(m_WristRadMM == mm)
			return false;
		m_WristRadMM = mm;
		return true;
	}

	inline quint16 ShoulderRadiusMM() const
	{
		return m_ShoulderRadMM;
	}

	inline quint16 ShoulderToSpineMM() const
	{
		return m_ShoulderToSpineMM;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const;

	// Returns true if the arm stands flat footed on the hand. True for human hands. False for paws, hooves, etc.
	bool StandsFlat() const;

	inline quint8 StandTone() const
	{
		return m_StandTone;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Arm");
	}

	inline quint16 UpperArmLengthMM() const
	{
		return m_UpperArmLengthMM;
	}

	virtual float Volume() const;

	inline bool Walks() const
	{
		return m_LegBits & LEG_Walk;
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}

	inline quint16 WristRadiusMM() const
	{
		return m_WristRadMM;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoLeg : public AuChromosomeWithGenes
{
private:
	quint16								m_AnkleLenMM;			// Length from ankle joint to base of foot.
	quint16								m_AnkleRadMM;			// Radius of ankle.
	quint16								m_ButtRadMM;			// Distance from hip joint to back of butt cheek.
	quint16								m_CalfLenMM;
	quint16								m_FootLenMM;			// From ankle joint to tip of foot.
	quint16								m_FootWidMM;
	quint16								m_HipRadXMM;			// From hip joint to side of hip meat.
	quint16								m_HipToSpineMM;			// Distance from center line to joint.
	quint16								m_KneeRadXMM;			// X radius of knee. Y radius depends on style.
	quint8								m_StandTone;			// 0 to 100 muscle tone when standing. 100 = straight, 0 = limp
	quint16								m_ThighLenMM;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoLeg(const AuChromoWhich& which,
			quint16 hipToSpineMM,
			quint16 thighLenMM,
			quint16 calfLenMM,
			quint16 buttRadMM,
			quint16 hipRadXMM,
			quint16 kneeRadXMM,
			quint16 ankleLenMM,
			quint16 ankleRadYMM,
			quint16 footLenMM,
			quint16 footWidMM,
			quint8 standTone);
	AuChromoLeg();
	AuChromoLeg(const AuChromoLeg& c);
	AuChromoLeg(const AuChromoLeg& c1, const AuChromoLeg& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoLeg& c = dynamic_cast<const AuChromoLeg&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_AnkleLenMM != c.m_AnkleLenMM)
				return false;
			if(m_AnkleRadMM != c.m_AnkleRadMM)
				return false;
			if(m_ButtRadMM != c.m_ButtRadMM)
				return false;
			if(m_CalfLenMM != c.m_CalfLenMM)
				return false;
			if(m_HipToSpineMM != c.m_HipToSpineMM)
				return false;
			if(m_FootLenMM != c.m_FootLenMM)
				return false;
			if(m_FootWidMM != c.m_FootWidMM)
				return false;
			if(m_HipRadXMM != c.m_HipRadXMM)
				return false;
			if(m_StandTone != c.m_StandTone)
				return false;
			if(m_ThighLenMM != c.m_ThighLenMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	// 'pose' is the current pose
	// 'bodyHz' is body cycles per second
	void AnglesForPose(const AuId& id, const AuElapsedTimer& when, float bodyHz, const AuPose& pose,
			HANDEDNESS hand, bool hasMaleGenitals, float torsoThickness,
			float& hipX, float& hipY, float& hipZ, float& kneeX, float& ankleX, float& toeX) const;

	// Height of the ankle bone above the ground.
	inline quint16 AnkleLengthMM() const
	{
		return m_AnkleLenMM;
	}

	// Ankle radius.
	inline quint16 AnkleRadiusMM() const
	{
		return m_AnkleRadMM;
	}

	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 4;
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	inline quint16 ButtRadiusMM() const
	{
		return m_ButtRadMM;
	}

	// These are optimum angles, not adjusted for body position or terrain.
	// 'strength' is 0 to 1, to modulate standing muscle tone.
	void CalcStandAngles(const AuPose& pose, float strength, float& hipX, float& hipY, float& hipZ, float& kneeX, float& ankleX,
			float& toeX) const;

	inline quint16 CalfLengthMM() const
	{
		return m_CalfLenMM;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoLeg(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Leg");
	}

	inline static CTYPE ChromosomeType()
	{
		return Leg;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	quint16 FootHeightMM() const;

	inline quint16 FootLengthMM() const
	{
		return m_FootLenMM;
	}

	inline quint16 FootWidthMM() const
	{
		return m_FootWidMM;
	}

	virtual void GetClothedMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, GROWTHSTAGE stage) const;

	virtual void GetSkinMesh(AuMeatMesh& mesh, const AuChromatid& dna, quint8 firstBone, const AuHairStyle& hairStyle,
			const AuClothing& clothes, GROWTHSTAGE stage) const;

	void HairFollicles(const AuChromatid& dna, float lockWid, AuFolliclePointArray& follicles) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// Returns my hip altitude above the ground, based on my current bone positions.
	float HipAltitudePosed(const AuChromatid& ctd) const;

	inline quint16 HipRadiusXMM() const
	{
		return m_HipRadXMM;
	}

	inline quint16 HipToSpineMM() const
	{
		return m_HipToSpineMM;
	}

	inline bool IsFront() const
	{
		return m_Which.Index() <= 1;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	inline float KneeAltitude(const AuChromatid& dna) const
	{
		return BoneLoc(dna, 1).z;
	}

	// Knee radius to the sides.
	inline quint16 KneeRadiusXMM() const
	{
		return m_KneeRadXMM;
	}

	// Knee radius to the front/back.
	quint16 KneeRadiusYMM() const;

	// Returns 0=left, 1=middle, 2=right
	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	// Length of this leg from hip joint to bottom of foot.
	inline quint16 LengthMM() const
	{
		return m_ThighLenMM + m_CalfLenMM + m_AnkleLenMM;
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual quint8 PoseOrder() const
	{
		return 2;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetAnkleLengthMM(quint16 mm)
	{
		if(m_AnkleLenMM == mm)
			return false;
		m_AnkleLenMM = mm;
		return true;
	}

	inline bool SetAnkleRadiusMM(quint16 mm)
	{
		if(m_AnkleRadMM == mm)
			return false;
		m_AnkleRadMM = mm;
		return true;
	}

	inline bool SetButtRadiusMM(quint16 mm)
	{
		if(m_ButtRadMM == mm)
			return false;
		m_ButtRadMM = mm;
		return true;
	}

	inline bool SetCalfLengthMM(quint16 mm)
	{
		if(m_CalfLenMM == mm)
			return false;
		m_CalfLenMM = mm;
		return true;
	}

	inline bool SetFootLengthMM(quint16 mm)
	{
		if(m_FootLenMM == mm)
			return false;
		m_FootLenMM = mm;
		return true;
	}

	inline bool SetFootWidthMM(quint16 mm)
	{
		if(m_FootWidMM == mm)
			return false;
		m_FootWidMM = mm;
		return true;
	}

	inline bool SetHipRadiusXMM(quint16 mm)
	{
		if(m_HipRadXMM == mm)
			return false;
		m_HipRadXMM = mm;
		return true;
	}

	inline bool SetHipToSpineMM(quint16 mm)
	{
		if(m_HipToSpineMM == mm)
			return false;
		m_HipToSpineMM = mm;
		return true;
	}

	inline bool SetKneeRadiusXMM(quint16 mm)
	{
		if(m_KneeRadXMM == mm)
			return false;
		m_KneeRadXMM = mm;
		return true;
	}

	inline bool SetStandTone(quint8 tone)
	{
		if(m_StandTone == tone)
			return false;
		m_StandTone = tone;
		return true;
	}

	inline bool SetThighLengthMM(quint16 mm)
	{
		if(m_ThighLenMM == mm)
			return false;
		m_ThighLenMM = mm;
		return true;
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const;

	// Returns true if the leg stands flat footed on the foot. True for human feet. False for paws, hooves, etc.
	bool StandsFlat() const;

	inline quint8 StandTone() const
	{
		return m_StandTone;
	}

	inline quint16 ThighLengthMM() const
	{
		return m_ThighLenMM;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Leg");
	}

	virtual float Volume() const;

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoFin : public AuChromosomeWithGenes
{
public:
	enum STYLE
	{
		Side,
	};

private:
	quint16								m_LengthMM;
	STYLE								m_Style;
	quint8								m_Where;				// 0=sides, 1=top/back, 2=bottom/belly
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoFin(const AuChromoWhich& which, STYLE style, quint16 lengthMM, quint8 where);
	AuChromoFin();
	AuChromoFin(const AuChromoFin& c);
	AuChromoFin(const AuChromoFin& c1, const AuChromoFin& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoFin& c = dynamic_cast<const AuChromoFin&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Where != c.m_Where)
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 1;
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoFin(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Fin");
	}

	inline static CTYPE ChromosomeType()
	{
		return Fin;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline bool IsBottom() const
	{
		return m_Where == 2;
	}

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	inline bool IsTop() const
	{
		return m_Where == 1;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual quint8 PoseOrder() const
	{
		return 2;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Fin");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoTentacle : public AuChromosomeWithGenes
{
public:
	enum STYLE
	{
		Octopus,
	};

private:
	STYLE								m_Style;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoTentacle(const AuChromoWhich& which, STYLE style);
	AuChromoTentacle();
	AuChromoTentacle(const AuChromoTentacle& c);
	AuChromoTentacle(const AuChromoTentacle& c1, const AuChromoTentacle& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoTentacle& c = dynamic_cast<const AuChromoTentacle&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 4;
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoTentacle(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Tentacle");
	}

	inline static CTYPE ChromosomeType()
	{
		return Tentacle;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual quint8 PoseOrder() const
	{
		return 2;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Tentacle");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoWing : public AuChromosomeWithGenes
{
public:
	enum STYLE
	{
		Bird,
	};

private:
	quint16								m_LengthMM;
	STYLE								m_Style;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoWing(const AuChromoWhich& which, STYLE style, quint16 lengthMM);
	AuChromoWing();
	AuChromoWing(const AuChromoWing& c);
	AuChromoWing(const AuChromoWing& c1, const AuChromoWing& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoWing& c = dynamic_cast<const AuChromoWing&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	// Returns the number of bone matrixes needed for the mesh created by this chromosome.
	virtual quint8 BoneCount(const AuChromatid& dna) const
	{
		Q_UNUSED(dna);
		return 3;
	}

	// Returns the location of the bone in the unposed model.
	virtual AuPoint BoneLoc(const AuChromatid& dna, quint8 bone) const;

	bool CanFly() const;

	virtual AuChromosome* Clone() const
	{
		return new AuChromoWing(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Wing");
	}

	inline static CTYPE ChromosomeType()
	{
		return Wing;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual AuAnimalPart* MakePart(const AuChromatid& dna, quint8& firstBone) const;

	virtual quint8 PoseOrder() const
	{
		return 2;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Wing");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Head Features
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoAntenna : public AuChromosome
{
public:
	enum STYLE
	{
		Simple,
	};

private:
	quint16								m_LengthMM;
	STYLE								m_Style;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoAntenna(const AuChromoWhich& which, STYLE style, quint16 lengthMM);
	AuChromoAntenna();
	AuChromoAntenna(const AuChromoAntenna& c);
	AuChromoAntenna(const AuChromoAntenna& c1, const AuChromoAntenna& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoAntenna& c = dynamic_cast<const AuChromoAntenna&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoAntenna(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Antenna");
	}

	inline static CTYPE ChromosomeType()
	{
		return Antenna;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Antenna");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoHorn : public AuChromosome
{
public:
	enum STYLE
	{
		Cow,
	};

private:
	STYLE								m_Style;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoHorn(const AuChromoWhich& which, STYLE style);
	AuChromoHorn();
	AuChromoHorn(const AuChromoHorn& c);
	AuChromoHorn(const AuChromoHorn& c1, const AuChromoHorn& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoHorn& c = dynamic_cast<const AuChromoHorn&>(dna);
			if(!AuChromosome::operator==(c))
				return false;

			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;

			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoHorn(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Horn");
	}

	inline static CTYPE ChromosomeType()
	{
		return Horn;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Horn");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEar : public AuChromosomeWithGenes
{
public:
	enum STYLE
	{
		Circle,
	};

private:
	qint16								m_AngleDeg;			// rotation angle in degrees
	quint16								m_DiameterMM;
	STYLE								m_Style;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoEar(const AuChromoWhich& which, STYLE style, quint16 diameterMM, qint16 angleDeg);
	AuChromoEar();
	AuChromoEar(const AuChromoEar& c);
	AuChromoEar(const AuChromoEar& c1, const AuChromoEar& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoEar& c = dynamic_cast<const AuChromoEar&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_Which != c.m_Which)
				return false;
			if(m_Style != c.m_Style)
				return false;
			if(m_AngleDeg != c.m_AngleDeg)
				return false;
			if(m_DiameterMM != c.m_DiameterMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	// Returns ear angle in degrees.
	inline qint16 AngleDeg() const
	{
		return m_AngleDeg;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoEar(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Ear");
	}

	inline static CTYPE ChromosomeType()
	{
		return Ear;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline quint16 DiameterMM() const
	{
		return m_DiameterMM;
	}

	virtual void GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx,
			const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	inline float Radius() const
	{
		return m_DiameterMM * FeetPerMM() / 2; 
	}

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetAngleDeg(qint16 deg)
	{
		if(m_AngleDeg == deg)
			return false;
		m_AngleDeg = deg;
		return true;
	}

	inline bool SetDiameterMM(quint16 mm)
	{
		if(m_DiameterMM == mm)
			return false;
		m_DiameterMM = mm;
		return true;
	}

	inline bool SetStyle(STYLE s)
	{
		if(m_Style == s)
			return false;
		m_Style = s;
		return true;
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	inline STYLE Style() const
	{
		return m_Style;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return ChromosomeName();
	}

	virtual float Volume() const;

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEye : public AuChromosomeWithGenes
{
public:
	enum FACING
	{
		Forward,
		Side,
		Away,
	};

private:
	MEAT_DECAL							m_Decal;				// Decal to use for the eye. default MD_HumanEye1.
	quint16								m_EyeDiaMM;
	FACING								m_Facing;
	quint16								m_SocketDiaMM;
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoEye(const AuChromoWhich& which, FACING facing, MEAT_DECAL decal, quint16 eyeDiaMM, quint16 socketDiaMM);
	AuChromoEye();
	AuChromoEye(const AuChromoEye& c);
	AuChromoEye(const AuChromoEye& c1, const AuChromoEye& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoEye& c = dynamic_cast<const AuChromoEye&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;

			if(m_Which != c.m_Which)
				return false;
			if(m_Decal != c.m_Decal)
				return false;
			if(m_EyeDiaMM != c.m_EyeDiaMM)
				return false;
			if(m_Facing != c.m_Facing)
				return false;
			if(m_SocketDiaMM != c.m_SocketDiaMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoEye(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Eye");
	}

	inline static CTYPE ChromosomeType()
	{
		return Eye;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline MEAT_DECAL Decal() const
	{
		return m_Decal;
	}

	inline quint16 EyeDiameterMM() const
	{
		return m_EyeDiaMM;
	}

	inline FACING Facing() const
	{
		return m_Facing;
	}

	virtual float FaceRadiusY(const AuChromoHead& head) const;

	virtual bool GetSkullEye(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
			quint8 firstBone) const;

	virtual void GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx,
			const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// Returns 0=left, 1=middle, 2=right
	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	inline quint16 SocketDiameterMM() const
	{
		return m_SocketDiaMM;
	}

	inline bool SetDecal(MEAT_DECAL decal)
	{
		if(m_Decal == decal)
			return false;
		m_Decal = decal;
		return true;
	}

	inline bool SetEyeDiameterMM(quint16 mm)
	{
		if(m_EyeDiaMM == mm)
			return false;
		m_EyeDiaMM = mm;
		return true;
	}

	inline bool SetFacing(FACING facing)
	{
		if(m_Facing == facing)
			return false;
		m_Facing = facing;
		return true;
	}

	inline bool SetSocketDiameterMM(quint16 mm)
	{
		if(m_SocketDiaMM == mm)
			return false;
		m_SocketDiaMM = mm;
		return true;
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Eye");
	}

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoNose : public AuChromosomeWithGenes
{
private:
	qint16								m_AngleDeg;				// Degrees
	quint16								m_LengthMM;
	quint16								m_WidthHeadMM;			// Width at the head end.
	quint16								m_WidthTipMM;			// Width at the tip.

public:	// Construction/destruction
	AuChromoNose(quint16 lengthMM, quint16 widthHeadMM, quint16 widthTipMM, qint16 angleDeg);
	AuChromoNose();
	AuChromoNose(const AuChromoNose& c);
	AuChromoNose(const AuChromoNose& c1, const AuChromoNose& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoNose& c = dynamic_cast<const AuChromoNose&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_AngleDeg != c.m_AngleDeg)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			if(m_WidthHeadMM != c.m_WidthHeadMM)
				return false;
			if(m_WidthTipMM != c.m_WidthTipMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline qint16 AngleDeg() const
	{
		return m_AngleDeg;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoNose(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Nose");
	}

	inline static CTYPE ChromosomeType()
	{
		return Nose;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	virtual float FaceRadiusY(const AuChromoHead& head) const;

	virtual void GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx,
			const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	inline quint16 LengthMM() const
	{
		return m_LengthMM;
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetAngleDeg(qint16 deg)
	{
		if(m_AngleDeg == deg)
			return false;
		m_AngleDeg = deg;
		return true;
	}

	inline bool SetLengthMM(quint16 mm)
	{
		if(m_LengthMM == mm)
			return false;
		m_LengthMM = mm;
		return true;
	}

	inline bool SetWidthHeadMM(quint16 mm)
	{
		if(m_WidthHeadMM == mm)
			return false;
		m_WidthHeadMM = mm;
		return true;
	}

	inline bool SetWidthTipMM(quint16 mm)
	{
		if(m_WidthTipMM == mm)
			return false;
		m_WidthTipMM = mm;
		return true;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return ChromosomeName();
	}

	virtual float Volume() const;

	inline quint16 WidthHeadMM() const
	{
		return m_WidthHeadMM;
	}

	inline quint16 WidthTipMM() const
	{
		return m_WidthTipMM;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoMouth : public AuChromosomeWithGenes
{
public:
	enum TEETH
	{
		NoTeeth,
		Flat,
		Fangs,
	};

private:
	quint16								m_DepthMM;				// Lips-to-bottom
	quint16								m_HeightMM;				// Lips-to-top
	quint16								m_LengthMM;				// Back-to-front
	quint16								m_LipThicknessMM;		// Margin between teeth and face skin.
	TEETH								m_Teeth;
	quint16								m_TeethHeightBottomMM;	// Height of tallest teeth in the lower jaw.
	quint16								m_TeethHeightTopMM;		// Height of tallest teeth in the upper jaw.
	quint16								m_WidthMM;				// Side-to-side

public:	// Construction/destruction
	AuChromoMouth(quint16 lengthMM, quint16 widthMM, quint16 heightMM, quint16 depthMM, quint16 lipThicknessMM,
			TEETH teeth, quint16 teethHtBottomMM, quint16 teethHtTopMM);
	AuChromoMouth();
	AuChromoMouth(const AuChromoMouth& c);
	AuChromoMouth(const AuChromoMouth& c1, const AuChromoMouth& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoMouth& c = dynamic_cast<const AuChromoMouth&>(dna);
			if(!AuChromosomeWithGenes::operator==(c))
				return false;
			if(m_DepthMM != c.m_DepthMM)
				return false;
			if(m_WidthMM != c.m_WidthMM)
				return false;
			if(m_LengthMM != c.m_LengthMM)
				return false;
			if(m_LipThicknessMM != c.m_LipThicknessMM)
				return false;
			if(m_HeightMM != c.m_HeightMM)
				return false;
			if(m_Teeth != c.m_Teeth)
				return false;
			if(m_TeethHeightBottomMM != c.m_TeethHeightBottomMM)
				return false;
			if(m_TeethHeightTopMM != c.m_TeethHeightTopMM)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoMouth(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Mouth");
	}

	inline static CTYPE ChromosomeType()
	{
		return Mouth;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline quint16 DepthMM() const
	{
		return m_DepthMM;
	}

	virtual float FaceRadiusY(const AuChromoHead& head) const;

	virtual void GetSkullFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual void GetSkullFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull,
			const AuChromoHead& head) const;

	virtual bool GetSkullJaw(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head,
			quint8 bone) const;

	virtual void GetSkullHair(const AuChromatid& dna, float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx,
			const AuMatrix4x4& mSkull, const AuChromoHead& head, GROWTHSTAGE stage) const;

	virtual quint64 HairWhereBits(const AuChromoWhich& which) const;

	inline quint16 HeightMM() const
	{
		return m_HeightMM;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	inline quint16 LengthMM() const
	{
		return m_LengthMM;
	}

	inline quint16 LipThicknessMM() const
	{
		return m_LipThicknessMM;
	}

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetDepthMM(quint16 mm)
	{
		if(m_DepthMM == mm)
			return false;
		m_DepthMM = mm;
		return true;
	}

	inline bool SetHeightMM(quint16 mm)
	{
		if(m_HeightMM == mm)
			return false;
		m_HeightMM = mm;
		return true;
	}

	inline bool SetLengthMM(quint16 mm)
	{
		if(m_LengthMM == mm)
			return false;
		m_LengthMM = mm;
		return true;
	}

	inline bool SetLipThicknessMM(quint16 mm)
	{
		if(m_LipThicknessMM == mm)
			return false;
		m_LipThicknessMM = mm;
		return true;
	}

	inline bool SetTeeth(TEETH t)
	{
		if(m_Teeth == t)
			return false;
		m_Teeth = t;
		return true;
	}

	inline bool SetTeethHeightBottomMM(quint16 mm)
	{
		if(m_TeethHeightBottomMM == mm)
			return false;
		m_TeethHeightBottomMM = mm;
		return true;
	}

	inline bool SetTeethHeightTopMM(quint16 mm)
	{
		if(m_TeethHeightTopMM == mm)
			return false;
		m_TeethHeightTopMM = mm;
		return true;
	}

	inline bool SetWidthMM(quint16 mm)
	{
		if(m_WidthMM == mm)
			return false;
		m_WidthMM = mm;
		return true;
	}

	inline TEETH Teeth() const
	{
		return m_Teeth;
	}

	inline quint16 TeethHeightBottomMM() const
	{
		return m_TeethHeightBottomMM;
	}

	inline quint16 TeethHeightTopMM() const
	{
		return m_TeethHeightTopMM;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return ChromosomeName();
	}

	virtual float Volume() const;

	inline quint16 WidthMM() const
	{
		return m_WidthMM;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Breathing Mechanism
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoLungs : public AuChromosome
{
private:
	quint8								m_AtmoType;				// ATMTYPE enum

public:	// Construction/destruction
	AuChromoLungs(ATMTYPE atmoType);
	AuChromoLungs();
	AuChromoLungs(const AuChromoLungs& c);
	AuChromoLungs(const AuChromoLungs& c1, const AuChromoLungs& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoLungs& c = dynamic_cast<const AuChromoLungs&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_AtmoType != c.m_AtmoType)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline ATMTYPE AtmosphereType() const
	{
		return (ATMTYPE)m_AtmoType;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoLungs(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Lungs");
	}

	inline static CTYPE ChromosomeType()
	{
		return Lungs;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline void SetAtmosphereType(ATMTYPE atmoType)
	{
		m_AtmoType = atmoType;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Lungs");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoGills : public AuChromosome
{
public:
	enum STYLE
	{
		Side,
	};

	enum SEATYPE
	{
		Water,
		SupercooledLiquid,
	};

private:
	SEATYPE								m_SeaType;				// 0=water, 1=supercooled liquid
	STYLE								m_Style;

public:	// Construction/destruction
	AuChromoGills(STYLE style, SEATYPE seaType);
	AuChromoGills();
	AuChromoGills(const AuChromoGills& c);
	AuChromoGills(const AuChromoGills& c1, const AuChromoGills& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoGills& c = dynamic_cast<const AuChromoGills&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_SeaType != c.m_SeaType)
				return false;
			if(m_Style != c.m_Style)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoGills(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Gills");
	}

	inline static CTYPE ChromosomeType()
	{
		return Gills;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline unsigned int SeaType() const
	{
		return m_SeaType;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Gills");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoBlood : public AuChromosome
{
public:
	enum STYLE
	{
		Cold,
		Warm,
	};

private:
	STYLE								m_Style;				// e.g. warm or cold

public:	// Construction/destruction
	AuChromoBlood(STYLE style);
	AuChromoBlood();
	AuChromoBlood(const AuChromoBlood& c);
	AuChromoBlood(const AuChromoBlood& c1, const AuChromoBlood& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoBlood& c = dynamic_cast<const AuChromoBlood&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Style != c.m_Style)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoBlood(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Blood");
	}

	inline static CTYPE ChromosomeType()
	{
		return Blood;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline void SetStyle(STYLE style)
	{
		m_Style = style;
	}

	inline STYLE Style() const
	{
		return m_Style;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Blood");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Young
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoReproduction : public AuChromosome
{
public:
	enum BIRTHPLACE
	{
		OnLand,
		InWater,
		DeadHost,
		LiveHost,
	};

	enum GENITALBITS
	{
		GEN_None						= 0x00,					// No genitals is an asexual animal.
		GEN_Female						= 0x01,
		GEN_Male						= 0x02,
	};

	enum YOUNG
	{
		Eggs,
		Live,
	};

private:
	BIRTHPLACE							m_BirthPlace;
	quint8								m_GenitalBits;			// OR'd GENITALBITS
	quint8								m_LitterMin;			// Minimum number of babies/eggs in a litter. 1 to 15.
	quint8								m_LitterMax;			// Maximum number of babies/eggs in a litter. 1 to 15.
	YOUNG								m_Young;

public:	// Construction/destruction
	AuChromoReproduction(quint8 genitalBits, quint8 litterMin, quint8 litterMax, YOUNG young, BIRTHPLACE birthPlace);
	AuChromoReproduction();
	AuChromoReproduction(const AuChromoReproduction& c);
	AuChromoReproduction(const AuChromoReproduction& c1, const AuChromoReproduction& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoReproduction& c = dynamic_cast<const AuChromoReproduction&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_BirthPlace != c.m_BirthPlace)
				return false;
			if(m_GenitalBits != c.m_GenitalBits)
				return false;
			if(m_LitterMax != c.m_LitterMax)
				return false;
			if(m_LitterMin != c.m_LitterMin)
				return false;
			if(m_Young != c.m_Young)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	inline BIRTHPLACE BirthingPlace() const
	{
		return m_BirthPlace;
	}

	virtual AuChromosome* Clone() const
	{
		return new AuChromoReproduction(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Reproduction");
	}

	inline static CTYPE ChromosomeType()
	{
		return Reproduction;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	GENDER Gender() const;

	inline bool HasFemaleGenitals() const
	{
		return m_GenitalBits & GEN_Female;
	}

	inline bool HasMaleGenitals() const
	{
		return m_GenitalBits & GEN_Male;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	inline quint8 LitterMax() const
	{
		return m_LitterMax;
	}

	inline quint8 LitterMin() const
	{
		return m_LitterMin;
	}

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline void SetBirthingPlace(BIRTHPLACE bp)
	{
		m_BirthPlace = bp;
	}

	inline bool SetHasFemaleGenitals(bool hasEm)
	{
		if(HasFemaleGenitals() == hasEm)
			return false;
		if(hasEm)
			m_GenitalBits |= GEN_Female;
		else
			m_GenitalBits &= ~GEN_Female;
		return true;
	}

	inline bool SetHasMaleGenitals(bool hasEm)
	{
		if(HasMaleGenitals() == hasEm)
			return false;
		if(hasEm)
			m_GenitalBits |= GEN_Male;
		else
			m_GenitalBits &= ~GEN_Male;
		return true;
	}

	inline void SetLitterRange(quint8 litterMin, quint8 litterMax)
	{
		Q_ASSERT(litterMin > 0 && litterMin < 16);
		Q_ASSERT(litterMax > 0 && litterMax < 16);
		Q_ASSERT(litterMax >= litterMin);
		m_LitterMin = litterMin;
		m_LitterMax = litterMax;
	}

	inline void SetYoung(YOUNG young)
	{
		m_Young = young;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Reproduction");
	}

	inline YOUNG Young() const
	{
		return m_Young;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoMammary : public AuChromosome
{
public:
	enum ORGAN
	{
		Breast,
		Nipple,
		Teat,
		Udder,
	};

private:
	bool								m_Feed;					// Feeds milk to offspring.
	ORGAN								m_Organ;				// e.g. breast, teat, udder
	bool								m_Show;					// Shows tits.
	quint16								m_SizeMM;				// Diameter of boob.
	AuChromoWhich						m_Which;				// Identity among similar chromosomes.

public:	// Construction/destruction
	AuChromoMammary(const AuChromoWhich& which, ORGAN organ, quint16 sizeMM, bool feed, bool show);
	AuChromoMammary();
	AuChromoMammary(const AuChromoMammary& c);
	AuChromoMammary(const AuChromoMammary& c1, const AuChromoMammary& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoMammary& c = dynamic_cast<const AuChromoMammary&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Feed != c.m_Feed)
				return false;
			if(m_Organ != c.m_Organ)
				return false;
			if(m_Show != c.m_Show)
				return false;
			if(m_SizeMM != c.m_SizeMM)
				return false;
			if(m_Which != c.m_Which)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoMammary(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Mammary");
	}

	inline static CTYPE ChromosomeType()
	{
		return Mammary;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline bool Feed() const
	{
		return m_Feed;
	}

	virtual void GetTorsoFeatures(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
			const AuChromoTorso& torso, GROWTHSTAGE stage) const;

	virtual void GetTorsoFeaturesClothed(const AuChromatid& dna, AuMeatMesh& mesh, const AuMatrix4x4& mTorso,
			const AuChromoTorso& torso, GROWTHSTAGE stage) const;

	// Returns 0=left, 1=middle, 2=right
	static LATERALITY Laterality(const AuChromoWhich& which);

	inline LATERALITY Laterality() const
	{
		return Laterality(m_Which);
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	inline ORGAN Organ() const
	{
		return m_Organ;
	}

	virtual bool PrepareToAdd(const AuChromatid& chroma);

	virtual void RandomizeSpecies(const AuDice& dice);

	inline bool SetFeed(bool feed)
	{
		if(m_Feed == feed)
			return false;
		m_Feed = feed;
		return true;
	}

	inline bool SetOrgan(ORGAN organ)
	{
		if(m_Organ == organ)
			return false;
		m_Organ = organ;
		return true;
	}

	inline bool SetSizeMM(quint16 mm)
	{
		if(m_SizeMM == mm)
			return false;
		m_SizeMM = mm;
		return true;
	}

	inline bool SetShow(bool show)
	{
		if(m_Show == show)
			return false;
		m_Show = show;
		return true;
	}

	virtual void SetWhich(AuChromoWhich& which)
	{
		m_Which = which;
	}

	inline bool Show() const
	{
		return m_Show;
	}

	virtual void SightExtents(AuPoint& min, AuPoint& max, const QList<AuAnimalBone>& bones) const;

	virtual bool SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, const QList<AuAnimalBone>& bones) const;

	inline quint16 SizeMM() const
	{
		return m_SizeMM;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Mammary");
	}

	virtual float Volume() const;

	// For chromosomes that can be in multiples, such as arms and legs.
	virtual AuChromoWhich Which() const
	{
		return m_Which;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Food
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEatsPlants : public AuChromosome
{
private:
	COMMODITYID							m_Prefers;				// Favorite herbivore food. CID_Invalid is no preference.

public:	// Construction/destruction
	AuChromoEatsPlants(COMMODITYID prefers);
	AuChromoEatsPlants();
	AuChromoEatsPlants(const AuChromoEatsPlants& c);
	AuChromoEatsPlants(const AuChromoEatsPlants& c1, const AuChromoEatsPlants& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoEatsPlants& c = dynamic_cast<const AuChromoEatsPlants&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_Prefers != c.m_Prefers)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoEatsPlants(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Eats Plants");
	}

	inline static CTYPE ChromosomeType()
	{
		return EatsPlants;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline void SetPrefers(COMMODITYID cid)
	{
		m_Prefers = cid;
	}

	inline COMMODITYID Prefers() const
	{
		return m_Prefers;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Eats Plants");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuChromoEatsAnimals : public AuChromosome
{
public:
	enum EATBITS
	{
		BloodSucker						= 0x01,					// Drains blood from live animals for food.
		Brain							= 0x02,					// Eats brains.
		Cannibal						= 0x04,					// Eats its own kind.
		Carne							= 0x08,					// Eats fresh meat and bones.
		Carrion							= 0x10,					// Eats rotting meat and bones.
		Organs							= 0x20,					// Eats internal organs.
		Predator						= 0x40,					// Kills live animals for food.
	};

private:
	quint8								m_EatBits;				// EATBITS or'd together

public:	// Construction/destruction
	AuChromoEatsAnimals(quint8 eatBits);
	AuChromoEatsAnimals();
	AuChromoEatsAnimals(const AuChromoEatsAnimals& c);
	AuChromoEatsAnimals(const AuChromoEatsAnimals& c1, const AuChromoEatsAnimals& c2, const AuDice& dice);

public:	// Operators
	virtual void Copy(const AuChromosome& o);

	virtual void operator>>(QDataStream& out) const;
	virtual void ReadStream(QDataStream& in, quint8 fileVer);

	virtual void WriteSpeciesKey(QDataStream& out) const;
	virtual void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

	virtual bool operator==(const AuChromosome& dna) const
	{
		try
		{
			const AuChromoEatsAnimals& c = dynamic_cast<const AuChromoEatsAnimals&>(dna);
			if(!AuChromosome::operator==(c))
				return false;
			if(m_EatBits != c.m_EatBits)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:
	virtual AuChromosome* Clone() const
	{
		return new AuChromoEatsAnimals(*this);
	}

	inline static QString ChromosomeName()
	{
		return QObject::tr("Eats Animals");
	}

	inline static CTYPE ChromosomeType()
	{
		return EatsAnimals;
	}

	virtual AuChromosome* Combine(const AuChromosome* c, const AuDice& dice) const;

	inline quint8 EatBits() const
	{
		return m_EatBits;
	}

	inline bool IsPredator() const
	{
		return m_EatBits & Predator;
	}

	inline bool IsScavenger() const
	{
		return m_EatBits & Carrion;
	}

	// NOTE: When this is called, the Type() of this chromosome has already been compared with the Type() of 'chromosome'.
	virtual bool IsSexuallyCompatible(const AuChromosome* chromosome) const;

	virtual int ListWidgetOrder() const;

	virtual QString ListWidgetText() const;

	virtual QWidget* MakePropertiesPage(QWidget* parent) const;

	virtual void RandomizeSpecies(const AuDice& dice);

	inline void SetEatBits(quint8 eatBits)
	{
		m_EatBits = eatBits;
	}

	virtual CTYPE Type() const
	{
		return ChromosomeType();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Eats Animals");
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Chain of chromosomes.

class AuChromatid : public QList<AuChromosome*>
{
public:	// Constructors/Destructors
	inline AuChromatid()
	{
	}

	inline ~AuChromatid()
	{
		Clear();
	}

	AuChromatid(const AuChromatid& c);

public:	// Operators
	const AuChromatid& operator=(const AuChromatid& dna);

	bool operator==(const AuChromatid& dna) const;

	inline bool operator!=(const AuChromatid& dna) const
	{
		return !operator==(dna);
	}

	void operator>>(QDataStream& out) const;
	void ReadStream(QDataStream& in, quint8 fileVer);

	void WriteSpeciesKey(QDataStream& out) const;
	void ReadSpeciesKey(QDataStream& in, quint8 fileVer);

public:	// Methods
	void AddableChromosomes(QMap<QString, AuChromosome::CTYPE>& addable) const;

	// Returns true if adding the chromosome causes the 3D model to change.
	bool AddChromosome(const AuChromosome& chromosome);

	// Returns true if adding the chromosome causes the 3D model to change.
	bool AddChromosome(AuChromosome::CTYPE type);

	// Ankle length of the first leg is returned.
	// 0 is returned if I have no legs.
	float AnkleLength() const;

	void ApplyChanges(const AuChromosome* t);

	int ArmCount() const;

	float ArmLength() const;

	AuPoint ArmLocation(const AuChromoWhich& which) const;

	// Returns an available which id for 'chromosome'.
	// Limited to 0 through 10. Shader can only handle MAX_BONES total; multiple arms and legs use them up quickly.
	// Returns AuChromoWhich::Null() if no 'which' is available for the chromosome.
	AuChromoWhich AvailableWhich(AuChromosome::CTYPE chromosome) const;

	float BiteRange() const;

	// Returns ATMTYPE breathed using lungs.
	// Returns ATMTYPE_Count if it does not breathe aire.
	ATMTYPE BreathesAir() const;

	// Returns HYDROGRAPHIC_LIQUID breathed using gills.
	HYDROGRAPHIC_LIQUID BreathesWater() const;

	// Returns the bone index that blends with the hip joint of the legs.
	quint8 ButtBone() const;

	// Calf length of the first leg is returned, in feet.
	// 0 is returned if I have no legs.
	float CalfLength() const;

	inline const AuChromosome* Chromosome(AuChromosome::CTYPE chromosome) const
	{
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			if((*ti)->Type() == chromosome)
				return *ti;
		return 0;
	}

	inline const AuChromosome* Chromosome(AuChromosome::CTYPE chromosome, const AuChromoWhich& which) const
	{
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			if((*ti)->Type() == chromosome && (*ti)->Which() == which)
				return *ti;
		return 0;
	}

	inline unsigned int ChromosomeCount(AuChromosome::CTYPE chromosome) const
	{
		unsigned int c = 0;
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			if((*ti)->Type() == chromosome)
				c++;
		return c;
	}

	inline void Clear()
	{
		while(!empty())
			delete takeFirst();
	}

	float ClothingBustAltitude(const AuChromoWhich& which) const;

	float ClothingGroinAltitude() const;

	// Waist altitude is just ahead of the first set of hips.
	float ClothingWaistAltitude() const;

	AuChromatid Combine(const AuChromatid& dna, const AuDice& dice) const;

	static void DecalTexture(MEAT_DECAL tx, float& s, float& t, float& u, float& v);

	// Dumps chromosomes and other info to debug output.
	void Dump() const;

	int EarCount() const;

	ECOROLE EcoRole() const;

	int EyeCount() const;

	// Returns the farthest distance in +Y on the face of 'head', for construction of a helmet.
	float FaceRadiusY(const AuChromoHead& head) const;

	// Field of view angle is 1/2 field of view. We only test half the angle off the center line of the eye vector.
	float FieldOfViewAngle() const;

	GENDER Gender() const;

	// Gets the movable portion of the eyes into 'mesh'.
	// 'mSkull' is the transform to the skull mesh, created by 'head'.
	// Eye bones start at 'firstBone' and increment for EyeCount() eyes.
	void GetSkullEyes(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head, quint8 firstBone) const;

	void GetSkullFeatures(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head) const;

	void GetSkullFeaturesClothed(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head) const;

	// Eye and ear locations with radius, for excluding head hair.
	void GetSkullHair(float lockWid, AuFolliclePointArray& hair, QMap<AuPoint, double>& hairEx, const AuMatrix4x4& mSkull,
			const AuChromoHead& head, GROWTHSTAGE stage) const;

	void GetSkullJaw(AuMeatMesh& mesh, const AuMatrix4x4& mSkull, const AuChromoHead& head, quint8 bone) const;

	void GetTorsoFeatures(AuMeatMesh& mesh, const AuMatrix4x4& mTorso, const AuChromoTorso& torso, GROWTHSTAGE stage) const;

	void GetTorsoFeaturesClothed(AuMeatMesh& mesh, const AuMatrix4x4& mTorso, const AuChromoTorso& torso, GROWTHSTAGE stage) const;

	bool HasBreasts() const;

	bool HasFemaleGenitals() const;

	bool HasMaleGenitals() const;

	// Altitude from base to head connection point.
	inline float HeadAltitude() const
	{
		return NeckAltitude() + NeckLength();
	}

	float HearingRange() const;

	void InitDefaultHuman(GENDER gender, const QColor& skinColor, const QColor& hairColor);

	bool IsPredator() const;

	bool IsSexuallyCompatible(const AuChromatid& dna) const;

	// Jump force is based on leg volume vs body volume.
	float JumpForce() const;

	// Altitude from base to knee connection point. Knee altitude of the first leg is found.
	// 0 is returned if I have no legs.
	float KneeAltitude() const;

	int LegCount() const;

	// Length of standing legs, if any. In the case of multi legged animals, the length of the fore legs is returned.
	// Leg length is to the bottom of the feet where it stands on the ground.
	float LegLength() const;

	AuPoint LegLocation(const AuChromoWhich& which) const;

	// Creates all the parts that are needed to render an animal with this dna.
	// The resulting 'parts' list is sorted according to pose sort order.
	// Does not create meshes.
	void MakeParts(QList<AuAnimalPart*>& parts) const;

	int MammaryCount() const;

	// Altitude of neck joint on unposed body.
	inline float NeckAltitude() const
	{
		return SpineLength();
	}

	// Length of neck, if any.
	// Neck length is the distance from the top of the torso to the head connection altitude.
	float NeckLength() const;

	// Returns the chromosome that defines my phylum.
	AuChromosome::CTYPE Phylum() const;

	void RandomizeSpecies(const AuDice& dice);

	void Remove(AuChromosome::CTYPE chromosome, const AuChromoWhich& which);

	// Seat for ceiling turret is dropped below ceiling by this amount.
	inline static float SeatDrop()
	{
		return 2.5;
	}

	inline static float SeatHeight()
	{
		return 20.f/12.f;		// 20" is our standard seat height.
	}

	float SightRange() const;

	float SmellRange() const;

	// Collects skin render information.
	// 'topSkinTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinColor' is used to modulate the skin color looked up from the texture.
	// 'bottomSkinTexId' is the string id of the bottom skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinTexFile' is the file that contains the bottom skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinColor' is used to modulate the skin color looked up from the texture.
	void SkinRenderParameters(QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor, QString& bottomSkinTexId,
			QString& bottomSkinTexFile, QColor& bottomSkinColor, QString& decalTexId, QString& decalTexFile) const;

	// Altitude from base to bottom of neck.
	float SpineLength() const;

	// Thigh length of the last leg is returned, in feet.
	// 0 is returned if I have no legs.
	float ThighLength() const;

	// Thickness (front to back in Y) of unposed torso meat. Lower torso is preferred if any, otherwise upper torso.
	float TorsoBottomThickness() const;

	// Length of lower portion of torso, if any.
	float TorsoLowerLength() const;

	// Side to side width of torso.
	float TorsoLowerWidth() const;

	// Side to side width of torso.
	float TorsoUpperWidth() const;

	// Return the volume of all my parts in cubic feet. Used for hit points assessment.
	float Volume() const;

protected:
	void UpdateChromosomeWhichCounts();

	void UpdateChromosomeWhichCounts(AuChromosome::CTYPE chromosome);
};

inline QDataStream& operator<<(QDataStream& out, const AuChromatid& dna)
{
	dna >> out;
	return out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Final DNA construct, a pair of DNA chromatids.

class AuDNA : public AuDNABase
{
private:
	AuChromatid							m_Chromatid[2];

public:	// Constructors/Destructors
	inline AuDNA()
	{
	}

	inline ~AuDNA()
	{
	}

	AuDNA(const AuDNA& c);

	// Creates this DNA by combining 'dna1' and 'dna2'. This is used to produce the result of mating.
	AuDNA(const AuDNA& dna1, const AuDNA& dna2, const AuDice& dice);

public:	// Operators
	const AuDNA& operator=(const AuDNA& dna);

	virtual bool operator==(const AuDNABase& dna) const;

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	void AddableChromosomes(QMap<QString, AuChromosome::CTYPE>& addable) const;

	// Returns true if adding the chromosome causes the 3D model to change.
	bool AddChromosome(AuChromosome::CTYPE type);

	void AddChromosome(const AuChromosome& chromosome);

	void ApplyChanges(const AuChromosome* alpha, const AuChromosome* beta);

	// Returns ATMTYPE breathed using lungs.
	// Returns ATMTYPE_Count if it does not breathe aire.
	ATMTYPE BreathesAir() const;

	// Returns HYDROGRAPHIC_LIQUID breathed using gills.
	HYDROGRAPHIC_LIQUID BreathesWater() const;

	AuChromatid& Chain(CHROMATID chroma)
	{
		return m_Chromatid[chroma];
	}

	const AuChromatid& Chain(CHROMATID chroma) const
	{
		return m_Chromatid[chroma];
	}

	virtual AuDNABase* Clone() const
	{
		return new AuDNA(*this);
	}

	virtual AuDNABase* Combine(const AuDNABase& dna, const AuDice& dice) const;

	virtual ECOROLE EcoRole() const;

	// Creates this DNA from a species.
	virtual bool FromSpecies(const AuSpecies& species);

	virtual GENDER Gender(CHROMATID chroma) const;

	virtual int GestateBabyDays() const;

	virtual int GestateImplantHours() const;

	virtual bool HasBone(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasBrain(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasGills(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasHeart(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasLeather(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasLungs(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual bool HasStomach(CHROMATID chroma, COMMODITYID butcheringWhat) const;

	virtual int MinutesToMature() const;

	void InitDefaultHuman();

	void InitDefaultHuman(const QColor& alphaSkinColor, const QColor& alphaHairColor, const QColor& betaSkinColor,
			const QColor& betaHairColor);

	// Diggers are 'subterranean' animals.
	bool IsDigger() const;

	// True if I am equipped to fly, regardless of current environmental conditions.
	bool IsFlyer() const;

	virtual bool IsSexuallyCompatible(const AuDNABase& dna) const;

	// Returns true if this DNA is valid for use in the game universe.
	bool IsValid() const;

	// Returns:
	// 0 = ok
	// 1 = cannot open file
	// 2 = bad file type
	int LoadFile(const QString& fileName);

	// Creates all the parts that are needed to render an animal with this dna.
	// The resulting 'parts' list is sorted according to pose sort order.
	// Does not create meshes.
	void MakeParts(QList<AuAnimalPart*>& parts, CHROMATID chroma) const;

	// Returns the chromosome that defines my phylum.
	AuChromosome::CTYPE Phylum() const;

	QString PhylumName() const;

	// DNA values are randomized such that they do not depart from their current species ranges.
	void RandomizeSpecies(const AuDice& dice);

	void Remove(AuChromosome::CTYPE chromosome, const AuChromoWhich& which);

	// Returns:
	// 0 = ok
	// 1 = cannot open file
	int SaveFile(const QString& fileName) const;

	// Collects skin render information.
	// 'chroma' is the chromatid to get.
	// 'topSkinTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'topSkinColor' is used to modulate the skin color looked up from the texture.
	// 'bottomSkinTexId' is the string id of the bottom skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinTexFile' is the file that contains the bottom skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	// 'bottomSkinColor' is used to modulate the skin color looked up from the texture.
	// 'decalTexId' is the string id of the top skin texture, for AuGLEffects::CreateTextureFromBitmap.
	// 'decalTexFile' is the file that contains the top skin texture image, for AuGLEffects::CreateTextureFromBitmap.
	void SkinRenderParameters(CHROMATID chroma, QString& topSkinTexId, QString& topSkinTexFile, QColor& topSkinColor,
			QString& bottomSkinTexId, QString& bottomSkinTexFile, QColor& bottomSkinColor,
			QString& decalTexId, QString& decalTexFile) const;

	virtual QString SpeciesDescription() const;

	virtual AuSpecies Species() const;

	TOD TimeActive() const;

	virtual QString TimeActiveName() const;

	virtual quint8 Version() const
	{
		return 4;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuDNA_H
