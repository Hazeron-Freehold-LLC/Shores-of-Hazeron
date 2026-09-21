//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclothing.h,v 1.26 2022/05/09 16:13:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClothing_H
#define AuClothing_H

#include "audna.h"
#include "augleffects.h"
#include "auitem.h"
#include "aumeat.h"
#include <QList>

class AuClothingPointMap;

enum CLOTH_DECAL
{
	CD_SoHLogo,
	CD_ParcXLogo,
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingDecal
{
	float								m_Alpha;
	CLOTH_DECAL							m_Decal;
	float								m_Height;
	AuMatrix4x4							m_M;
	float								m_Width;

public:
	AuClothingDecal(CLOTH_DECAL decal, const AuMatrix4x4& m, float width, float height, float alpha) : m_M(m)
	{
		m_Alpha = alpha;
		m_Decal = decal;
		m_Height = height;
		m_Width = width;
	}

	AuClothingDecal()
	{
		m_Alpha = 0;
		m_Decal = CD_SoHLogo;
		m_Height = 0;
		m_Width = 0;
	}

public:
	inline float Alpha() const
	{
		return m_Alpha;
	}

	inline CLOTH_DECAL Decal() const
	{
		return m_Decal;
	}

	inline float Height() const
	{
		return m_Height;
	}

	inline const AuMatrix4x4& M() const
	{
		return m_M;
	}

	inline float Width() const
	{
		return m_Width;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingTexture
{
	QColor								m_Color;				// Color modulates the texture during rendering.
	QString								m_Name;					// Texture name in the texture cache.
	GLuint								m_Tex;					// GL texture id

public:
	inline AuClothingTexture(const QString& name, GLuint tex, const QColor& color)
	{
		m_Color = color;
		m_Name = name;
		m_Tex = tex;
	}

	inline AuClothingTexture()
	{
		m_Tex = 0;
	}

	inline AuClothingTexture(const AuClothingTexture& c)
	{
		m_Color = c.m_Color;
		m_Name = c.m_Name;
		m_Tex = c.m_Tex;
	}

public:
	inline const AuClothingTexture& operator=(const AuClothingTexture& c)
	{
		m_Color = c.m_Color;
		m_Name = c.m_Name;
		m_Tex = c.m_Tex;
		return *this;
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

	inline GLuint Texture() const
	{
		return m_Tex;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingTextureArray : public QVector<AuClothingTexture>
{
public:
	inline AuClothingTextureArray()
	{
	}

	inline AuClothingTextureArray(const AuClothingTextureArray& c) : QVector<AuClothingTexture>(c)
	{
	}

public:
	inline const AuClothingTextureArray& operator=(const AuClothingTextureArray& c)
	{
		clear();
		for(const_iterator i = c.cbegin(); i != c.cend(); i++)
			append(*i);
		return *this;
	}

public:
	// 'name' is the identifying name used when 're' creates it in the texture cache.
	// 'file' is the file from which to load the texture, if it is not already loaded.
	// 'color' modulates the texture during rendering.
	void Create(AuGLEffects& re, const QString& name, const QString& file, const QColor& color);

	// Returns 0 if the 'name' and 'color' combination is not found.
	int IndexOf(const QString& name, const QColor& color) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Vertex of clothing model.

class AuClothingPoint : public AuPointN
{
public:
	quint8								m_BoneIndex[4];			// Indexes of bone matrices influencing this vertex.
	float								m_BoneWeight[4];		// All bone weights should add up to 1.
	float								m_ClothingTx;			// Clothing texture X coordinate at this location.
	float								m_ClothingTy;			// Clothing texture Y coordinate at this location.
	float								m_DecalBlend;			// Blend from base to decal textures: 0=base, 1=decal
	float								m_DecalTx;				// Decal texture X coordinate at this location.
	float								m_DecalTy;				// Decal texture Y coordinate at this location.
	QColor								m_DyeColor;				// Dye color that modulates the color of this vertex, also alpha.
	float								m_Reflect;				// Reflectiveness. Modulates specular reflection.
	float								m_Shine;				// Shininess, a.k.a. specular power.

public:	// Constructors
	// Constructor for clothing mesh.
	inline AuClothingPoint(const AuPoint& loc,
			const AuPoint& surfN,
			float shine,
			float reflect,
			int boneIndex[4],
			float boneWeight[4],
			float clothingTx,
			float clothingTy,
			const QColor& dye) : AuPointN(loc, surfN)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_ClothingTx = clothingTx;
		m_ClothingTy = clothingTy;
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
		m_DyeColor = dye;
		m_Reflect = reflect;
		m_Shine = shine;
	}

	inline AuClothingPoint(const AuPointN& loc, const AuMeatPoint& mp, const QColor& dye) : AuPointN(loc, mp.m_N)
	{
		// The same bones must apply so the clothing moves with the underlying meat mesh.
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = mp.m_BoneIndex[ib];
			m_BoneWeight[ib] = mp.m_BoneWeight[ib];
		}
		m_ClothingTx = mp.m_SkinBottomTx;
		m_ClothingTy = mp.m_SkinBottomTy;
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
		m_DyeColor = dye;
		m_Reflect = 0;
		m_Shine = 0;
	}

	inline AuClothingPoint()
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = 0;
			m_BoneWeight[ib] = 0;
		}
		m_ClothingTx = 0;
		m_ClothingTy = 0;
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
		m_DyeColor = QColor(255,255,255,255);
		m_Reflect = 0;
		m_Shine = 0;
	}

	inline AuClothingPoint(const AuClothingPoint& c) : AuPointN(c)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_ClothingTx = c.m_ClothingTx;
		m_ClothingTy = c.m_ClothingTy;
		m_DecalBlend = c.m_DecalBlend;
		m_DecalTx = c.m_DecalTx;
		m_DecalTy = c.m_DecalTy;
		m_DyeColor = c.m_DyeColor;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
	}

public:	// Operators
	inline const AuClothingPoint& operator=(const AuClothingPoint& c)
	{
		AuPointN::operator=(c);
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_ClothingTx = c.m_ClothingTx;
		m_ClothingTy = c.m_ClothingTy;
		m_DecalBlend = c.m_DecalBlend;
		m_DecalTx = c.m_DecalTx;
		m_DecalTy = c.m_DecalTy;
		m_DyeColor = c.m_DyeColor;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint8 hasWhat = 0;
		if(m_DecalBlend > 0)
			hasWhat |= 0x01;
		if(m_Reflect > 0)
			hasWhat |= 0x02;
		if(m_Shine > 0)
			hasWhat |= 0x04;
		if(m_DyeColor != QColor(255,255,255,255))
			hasWhat |= 0x08;

		for(int ib=0; ib<4; ib++)
		{
			out << m_BoneIndex[ib];
			out << m_BoneWeight[ib];
		}
		out << m_ClothingTx;
		out << m_ClothingTy;

		if(hasWhat & 0x01)
		{
			out << m_DecalBlend;
			out << m_DecalTx;
			out << m_DecalTy;
		}

		if(hasWhat & 0x02)
			out << m_DyeColor;

		if(hasWhat & 0x04)
			out << m_Reflect;

		if(hasWhat & 0x08)
			out << m_Shine;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 hasWhat;
		in >> hasWhat;

		for(int ib=0; ib<4; ib++)
		{
			in >> m_BoneIndex[ib];
			in >> m_BoneWeight[ib];
		}
		in >> m_ClothingTx;
		in >> m_ClothingTy;

		if(hasWhat & 0x01)
		{
			in >> m_DecalBlend;
			in >> m_DecalTx;
			in >> m_DecalTy;
		}

		if(hasWhat & 0x02)
			in >> m_DyeColor;

		if(hasWhat & 0x04)
			in >> m_Reflect;

		if(hasWhat & 0x08)
			in >> m_Shine;
	}

public:	// Methods
	inline float Alpha() const
	{
		return m_DyeColor.alphaF();
	}

	// Sets all bones at this point. All bone weights must add up to 1.
	inline void SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
			float boneWeight2, quint8 boneIndex3, float boneWeight3)
	{
		m_BoneIndex[0] = boneIndex0;
		m_BoneWeight[0] = boneWeight0;

		m_BoneIndex[1] = boneIndex1;
		m_BoneWeight[1] = boneWeight1;

		m_BoneIndex[2] = boneIndex2;
		m_BoneWeight[2] = boneWeight2;

		m_BoneIndex[3] = boneIndex3;
		m_BoneWeight[3] = boneWeight3;
	}

	void Wrinkle(float freqx, float freqy, float maxLift, AuClothingPointMap& done);
};

inline QDataStream& operator<<(QDataStream& out, const AuClothingPoint& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuClothingPoint& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingPointMap : public QMap<AuPoint, AuClothingPoint>
{
public:
	const_iterator FindFuzzy(const AuClothingPoint& p) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle for rendering clothing.

class AuClothingTri
{
public:
	enum PART
	{
		Unknown,
		Ascot,
		Gloves,
		Pants,
		Shirt,
		Shoes,
		Suit,
		Helmet,
		Visor,
	};

public:
	quint8								m_ClothingTexIndex;		// Index of clothing texture to use (0 to 4).
	quint8								m_Part;					// PART of clothing
	AuClothingPoint						m_V0;					// Index into the mesh's vertex array.
	AuClothingPoint						m_V1;					// Index into the mesh's vertex array.
	AuClothingPoint						m_V2;					// Index into the mesh's vertex array.

private:
	mutable AuPoint						m_N;

public:	// Constructors
	inline AuClothingTri(PART part, quint8 clothingTex, const AuClothingPoint& m0, const AuClothingPoint& m1,
			const AuClothingPoint& m2) : m_V0(m0), m_V1(m1), m_V2(m2)
	{
		m_ClothingTexIndex = clothingTex;
		m_Part = part;
	}

	// 'clothingTex' is the clothing texture index, 0 to 4.
	// 'dye0', 'dye1', and 'dye2' are dye colors for each vertex.
	inline AuClothingTri(PART part, quint8 clothingTex, const AuMeatTri& mt, const AuPointN& v0, const QColor& dye0,
			const AuPointN& v1, const QColor& dye1, const AuPointN& v2, const QColor& dye2) :
		m_V0(v0, mt.m_V0, dye0), m_V1(v1, mt.m_V1, dye1), m_V2(v2, mt.m_V2, dye2), m_N(mt.SurfaceNormal())
	{
		m_ClothingTexIndex = clothingTex;
		m_Part = part;
	}

	inline AuClothingTri()
	{
		m_ClothingTexIndex = 0;
		m_Part = Unknown;
	}

	inline AuClothingTri(const AuClothingTri& c)
	{
		m_ClothingTexIndex = c.m_ClothingTexIndex;
		m_N = c.m_N;
		m_Part = c.m_Part;
		m_V0 = c.m_V0;
		m_V1 = c.m_V1;
		m_V2 = c.m_V2;
	}

public:	// Operators
	inline void operator<<(QDataStream& in)
	{
		in >> m_ClothingTexIndex;
		in >> m_Part;
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;

		m_N = AuPoint(0,0);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_ClothingTexIndex;
		out << m_Part;
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}

public:	// Methods
	// Returns the area of this triangle.
	double Area() const;

	// Returns the center point of this triangle.
	AuPoint Center() const;

	// Returns the 3D extents box of this triangle.
	void Extents(AuPoint& min, AuPoint& max) const;

	inline bool IsHead() const
	{
		return m_Part == Helmet || m_Part == Visor;
	}

	// Returns true if any of my vertices have an alpha value that is not zero.
	inline bool NeedsBlend() const
	{
		return m_V0.Alpha() < 1 || m_V1.Alpha() < 1 || m_V2.Alpha() < 1;
	}

	// Returns the unit vector that is perpendicular to this triangle. Will calculate if not calculated yet.
	const AuPoint& SurfaceNormalCalc() const;

	inline void Transform(const AuMatrix4x4& m)
	{
		m_N = m.ProjectVector(m_N);
		m_V0.Transform(m);
		m_V1.Transform(m);
		m_V2.Transform(m);
	}

	void Wrinkle(float freqx, float freqy, float maxLift, AuClothingPointMap& done);
};

inline QDataStream& operator<<(QDataStream& out, const AuClothingTri& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuClothingTri& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingSegment
{
public:
	AuClothingPoint						m_Loc1;
	AuClothingPoint						m_Loc2;

public:
	inline AuClothingSegment(const AuClothingPoint& p1, const AuClothingPoint& p2) : m_Loc1(p1), m_Loc2(p2)
	{
	}

	inline AuClothingSegment()
	{
	}

public:
	inline bool IsSegment(const AuClothingSegment& seg) const
	{
		return (m_Loc1.FuzzyEquals(seg.m_Loc1) && m_Loc2.FuzzyEquals(seg.m_Loc2)) ||
			(m_Loc1.FuzzyEquals(seg.m_Loc2) && m_Loc2.FuzzyEquals(seg.m_Loc1));
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClothingSegmentArray : public QVector<AuClothingSegment>
{
public:
	// Returns index of segment; -1 if not found. Fuzzy test using IsSegment, which fuzzy checks ends both ways.
	inline const_iterator FindSegmentFuzzy(const AuClothingSegment& seg) const
	{
		for(const_iterator si = cbegin(); si != cend(); si++)
			if((*si).IsSegment(seg))
				return si;
		return cend();
	}

	inline iterator FindSegmentFuzzy(const AuClothingSegment& seg)
	{
		for(iterator si = begin(); si != end(); si++)
			if((*si).IsSegment(seg))
				return si;
		return end();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is triangle center point.

class AuClothingMesh : public QList<AuClothingTri>
{
private:
	AuPoint								m_Max;
	AuPoint								m_Min;

public:	// Constructors
	AuClothingMesh()
	{
	}

	AuClothingMesh(const AuClothingMesh& mesh) : QList<AuClothingTri>(mesh)
	{
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
	}

public:
	const AuClothingMesh& operator=(const AuClothingMesh& mesh)
	{
		QList<AuClothingTri>::operator=(mesh);
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
		return *this;
	}

public:	// Methods
	// 'sourceMesh' is the source of the current indexes in 't'.
	// 'combineVerts' can be true when sourceMesh is not this mesh, to use existing vertexes instead of adding duplicates.
	inline void Append(const AuClothingTri& t)
	{
		ExpandExtents(t);
		append(t);
	}

	// 'combineVerts' can be true when sourceMesh is not this mesh, to use existing vertexes instead of adding duplicates.
	inline void Append(const AuClothingMesh& sourceMesh)
	{
		for(const_iterator mi = sourceMesh.cbegin(); mi != sourceMesh.cend(); mi++)
			Append(*mi);
	}

	// Applies a decal texture to this mesh.
	// 'decal' is the decal to apply.
	// 'm' is the transformation matrix to position the decal.
	// 'decalWidth' is the width of the decal rectangle (X axis in m).
	// 'decalHeight' is the height of the decal rectangle (Y axis in m).
	// 'decalAlpha' is the alpha of the decal, for transparency/intensity of application.
	void ApplyDecal(CLOTH_DECAL decal, const AuMatrix4x4& m, float decalWidth, float decalHeight, float decalAlpha); 

	void ApplyDecal(const AuClothingDecal& decal);

	void ApplyDecals(const QList<AuClothingDecal>& decals);

	inline void Clear()
	{
		clear();
	}

	inline int Count() const
	{
		return count();
	}

	inline int CountBlended() const
	{
		int c = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if((*mi).NeedsBlend())
				c++;
		return c;
	}

	inline int CountBlendedHead() const
	{
		int c = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if((*mi).IsHead() && (*mi).NeedsBlend())
				c++;
		return c;
	}

	inline int CountUnblended() const
	{
		int c = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if(!(*mi).NeedsBlend())
				c++;
		return c;
	}

	inline int CountUnblendedHead() const
	{
		int c = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if((*mi).IsHead() && !(*mi).NeedsBlend())
				c++;
		return c;
	}

	// Create VAO(s) for use with the AnimalClothing shader.
	AuGLVAO CreateAnimalClothingVAO(AuOpenGLFunctions& re) const;
	AuGLVAO CreateAnimalClothingBlendVAO(AuOpenGLFunctions& re) const;
	AuGLVAO CreateHeadClothingVAO(AuOpenGLFunctions& re) const;
	AuGLVAO CreateHeadClothingBlendVAO(AuOpenGLFunctions& re) const;

	void ExpandExtents(const AuClothingTri& t);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		min = m_Min;
		max = m_Max;
	}

	void FromFaces(const AuScFaceList& faces, AuClothingTri::PART part, quint8 texIndex, const QColor& dyeColorAlpha);

	// Returns the X, Y and Z dimensions of this mesh.
	inline AuPoint Size() const
	{
		return m_Max - m_Min;
	}

	void Sag(float stiffness);

	// Sets all bones in this mesh. All bone weights must add up to 1.
	void SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2,
			quint8 boneIndex3, float boneWeight3);

	// Recalculates all surface normals to be smooth.
	void SmoothNormals();

	inline void Transform(const AuMatrix4x4& m)
	{
		if(!empty())
		{
			// Transform my triangles and update my extents with the result.
			iterator ti = begin();
			(*ti).Transform(m);
			(*ti).Extents(m_Min, m_Max);
			ti++;
			AuPoint tMin, tMax;
			for(; ti != end(); ti++)
			{
				(*ti).Transform(m);
				(*ti).Extents(tMin, tMax);
				m_Min = m_Min.Min(tMin);
				m_Max = m_Max.Max(tMax);
			}
		}
	}

	void Wrinkle(float freqx, float freqy, float maxLift);

	void WrinkleVertex(float maxLift, AuClothingPoint& v, AuClothingPointMap& done);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Used to model and render current clothing/armor that is worn.
// Each item is the top level item. It could be covering other clothing. e.g. EVA suit covers clothing.

class AuClothing
{
private:	// Properties
	AuItem*								m_Armor;				// Armor or vac suit worn on body.
	AuItem*								m_Clothing;				// Clothing worn on body, usually under armor.
	AuItem*								m_Hat;					// Hat or helmet worn on the head.

public:	// Constructors
	inline AuClothing()
	{
		m_Armor = 0;
		m_Clothing = 0;
		m_Hat = 0;
	}

	inline AuClothing(const AuClothing& c)
	{
		if(c.m_Armor)
			m_Armor = c.m_Armor->Clone();
		else
			m_Armor =  0;

		if(c.m_Clothing)
			m_Clothing = c.m_Clothing->Clone();
		else
			m_Clothing =  0;

		if(c.m_Hat)
			m_Hat = c.m_Hat->Clone();
		else
			m_Hat =  0;
	}

	inline ~AuClothing()
	{
		if(m_Armor)
			delete m_Armor;
		if(m_Clothing)
			delete m_Clothing;
		if(m_Hat)
			delete m_Hat;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		quint8 hasWhat = 0;
		if(m_Armor)
			hasWhat |= 0x01;
		if(m_Clothing)
			hasWhat |= 0x02;
		if(m_Hat)
			hasWhat |= 0x04;
		out << hasWhat;

		if(hasWhat & 0x01)
			out << m_Armor->Pack();

		if(hasWhat & 0x02)
			out << m_Clothing->Pack();

		if(hasWhat & 0x04)
			out << m_Hat->Pack();
	}

	inline void operator<<(QDataStream& in)
	{
		if(m_Armor)
			delete m_Armor;
		if(m_Clothing)
			delete m_Clothing;
		if(m_Hat)
			delete m_Hat;

		quint8 hasWhat;
		in >> hasWhat;

		if(hasWhat & 0x01)
		{
			QByteArray packed;
			in >> packed;
			m_Armor = AuItem::Unpack(packed);
		}
		else
			m_Armor = 0;

		if(hasWhat & 0x02)
		{
			QByteArray packed;
			in >> packed;
			m_Clothing = AuItem::Unpack(packed);
		}
		else
			m_Clothing = 0;

		if(hasWhat & 0x04)
		{
			QByteArray packed;
			in >> packed;
			m_Hat = AuItem::Unpack(packed);
		}
		else
			m_Hat = 0;
	}

	inline const AuClothing& operator=(const AuClothing& c)
	{
		if(m_Armor)
			delete m_Armor;
		if(c.m_Armor)
			m_Armor = c.m_Armor->Clone();
		else
			m_Armor =  0;

		if(m_Clothing)
			delete m_Clothing;
		if(c.m_Clothing)
			m_Clothing = c.m_Clothing->Clone();
		else
			m_Clothing =  0;

		if(m_Hat)
			delete m_Hat;
		if(c.m_Hat)
			m_Hat = c.m_Hat->Clone();
		else
			m_Hat =  0;
		return *this;
	}

	bool operator==(const AuClothing& c) const;

public:	// Methods
	inline const AuItem* Armor() const
	{
		return m_Armor;
	}

	inline COMMODITYID ArmorCID() const
	{
		if(m_Armor)
			return m_Armor->CommodityId();
		return CID_Invalid;
	}

	// If true, the bare body part is not visible due to armor or clothing, to the extent that it can be omitted from the 3D model.
	bool BareBodyCovered() const;	// All body parts, excluding the head.
	bool BareCalfCovered() const;
	bool BareFootCovered() const;
	bool BareForeArmCovered() const;
	bool BareHandCovered() const;
	bool BareLowerTorsoCovered() const;
	bool BareNeckCovered() const;
	bool BareThighCovered() const;
	bool BareUpperArmCovered() const;
	bool BareUpperTorsoCovered() const;

	inline const AuItem* Clothing() const
	{
		return m_Clothing;
	}

	inline COMMODITYID ClothingCID() const
	{
		if(m_Clothing)
			return m_Clothing->CommodityId();
		return CID_Invalid;
	}

	// Test to see if a hair follicle 'fp' is covered by this clothing. Returns true if the follicle should be completely removed,
	//   including the base patch if any.
	// 'basePatch' is true if the hair style calls for a base patch of hair at the follicle.
	// 'lockLen' is an input output parameter. On input, it is the lock length. On output it may be adjusted shorter. If it is
	//   adjusted to 0 and there is no base patch, true is returned so the hair will be removed.
	// 'wearsBra' is needed when making underwear.
	bool CoverHair(const AuFolliclePoint& fp, bool basePatch, float& lockLen, bool wearsBra) const;

	// Create the textures referenced by my AuClothingTri's texture indexes.
	void CreateTextures(AuGLEffects& re, AuClothingTextureArray& ctex) const;

	static void DecalTexture(CLOTH_DECAL tx, float& s, float& t, float& u, float& v);

	// Constructs 'clothing' mesh for this set of clothing, to fit the 'meat' mesh.
	// 'tex' is my clothing textures, previously collected using CreateTextures.
	// 'unique' is used to make clothing that varies slightly between wearers. e.g. wearer's id. So far only affects underwear.
	void GetMesh(AuClothingMesh& clothing, const AuChromatid& dna, const AuMeatMesh& meat,
			const AuClothingTextureArray& ctex, quint32 unique) const;

	inline const AuItem* Hat() const
	{
		return m_Hat;
	}

	inline COMMODITYID HatCID() const
	{
		if(m_Hat)
			return m_Hat->CommodityId();
		return CID_Invalid;
	}

	bool SetArmor(const AuItem* item);
	bool SetArmor(COMMODITYID cid);

	bool SetClothing(const AuItem* item);
	bool SetClothing(COMMODITYID cid);

	bool SetHat(const AuItem* item);
	bool SetHat(COMMODITYID cid);

protected:
	void CreateItemTextures(const AuItem& item, AuGLEffects& re, AuClothingTextureArray& ctex) const;

	// 'cuff' is the proportion of the arm length that is sleeved, from the shoulder to the knuckles, 0 to 1.
	void GetArmsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float cuff, const QColor& dye, float thickness) const;

	// 'thickness' > 0 causes edge walls to be added inward.
	void GetChestMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float neckAlt, float waistAlt, const QColor& dye, float thickness) const;

	// 'cuff' is the proportion of the arm length at which the glove starts, from the shoulder to the knuckles, 0 to 1.
	void GetGlovesMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna, const AuMeatMesh& meat, float skinDistance,
			float cuff, const QColor& dye) const;

	void GetGroinMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float waistAlt, const QColor& dye) const;

	// 'cuff' is the proportion of the arm length at which the mesh starts, from the shoulder to the knuckles, 0 to 1.
	void GetHandsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float cuff, const QColor& dye) const;

	void GetHelmetMesh(AuClothingMesh& clothing, int helmTex, const AuChromatid& dna, const AuMeatMesh& meat,
			const QColor& helmColor, const QList<AuClothingDecal>& decals, COMMODITYID helmCID) const;

	void GetItemMesh(const AuItem& item, AuClothingMesh& clothing, const AuChromatid& dna, const AuMeatMesh& meat,
			const AuClothingTextureArray& ctex, quint32 unique) const;

	void GetLegsMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float waistAlt, float cuffAlt, const QColor& dye, float thickness) const;

	void GetNeckMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float collar, const QColor& dye) const;

	void GetPantsMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna, const AuMeatMesh& meat, float skinDistance,
			float waistAlt, float cuffAlt, const QColor& dye, float noiseFreqX, float noiseFreqY, float noiseMaxLift) const;

	void GetPowerArmorHelmetMesh(AuClothingMesh& clothing, int surfaceTex, int earphoneTex, const AuChromatid& dna,
			const AuMeatMesh& meat, const QColor& dye, int visorTex, const QColor& visorColor, const QList<AuClothingDecal>& decals,
			COMMODITYID helmCID) const;

	// 'thickness' > 0 causes edge walls to be added inward.
	void GetShirtMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna, const AuMeatMesh& meat, float skinDistance,
			float neckAlt, float waistAlt, float cuff, const QColor& dye, const QList<AuClothingDecal>& decals, float noiseFreqX,
			float noiseFreqY, float noiseMaxLift, float thickness) const;

	void GetShoesMesh(AuClothingMesh& clothing, int texIndex, const AuChromatid& dna, const AuMeatMesh& meat, float skinDistance,
			float bootStrapAlt, const QColor& dye) const;

	void GetShoesMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, const QColor& dye) const;

	void GetTailMesh(AuClothingMesh& clothing, AuClothingTri::PART part, int texIndex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, const QColor& dye) const;

	void GetVacSuitHelmetMesh(AuClothingMesh& clothing, int surfaceTex, int earphoneTex, const AuChromatid& dna,
			const AuMeatMesh& meat, const QColor& dye, int visorTex, const QColor& visorColor,
			const QList<AuClothingDecal>& decals) const;

	void GetVacSuitMesh(AuClothingMesh& clothing, int torsoTex, int armTex, int legTex, const AuChromatid& dna,
			const AuMeatMesh& meat, float skinDistance, float wrist, float ankleAlt, const QColor& dye,
			const QList<AuClothingDecal>& decals, float maxLift) const;
};

#endif // AuClothing_H
