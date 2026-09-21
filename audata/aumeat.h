//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumeat.h,v 1.78 2022/02/19 18:19:12 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMeat_H
#define AuMeat_H

#include "augleffects.h"
#include "auhair.h"
#include "ausegment.h"
#include "autriangle.h"
#include "audef.h"
#include <QList>

class AuDNAHair;
class AuGlobeLL;
class AuMeatMesh;
class AuScFaceList;
class QDataStream;

#define MEATPROFILE_FILE_TYPE			0xe0					// .mp meat profile files
#define MEATPROFILE_FILE_VER			0x01					// .mp meat profile files

#define MEATLINE_FILE_TYPE				0xe1					// .ml meat polyline files
#define MEATLINE_FILE_VER				0x01					// .ml meat polyline files

enum MEAT_DECAL
{
	MD_HumanEye1,
	MD_HumanEye2,
	MD_HumanEye3,
	MD_HumanEye4,
	MD_HumanEye5,
	MD_HumanEye6,
	MD_HumanEye7,
	MD_HumanEye8,
	MD_HumanEye9,
	MD_HumanEye10,
	MD_HumanMouth1,
	MD_HumanMouth2,
	MD_HumanMouth3,
	MD_HumanMouth4,
	MD_HumanMouth5,
	MD_HumanMouth6,
	MD_HumanMouth7,
	MD_HumanMouth8,
	MD_HumanMouth9,
	MD_HumanMouth10,
	MD_Teeth,
	MD_Nipple,
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Vertex of meat model.

class AuMeatPoint : public AuPointN
{
public:
	quint8								m_BoneIndex[4];			// Indexes of bone matrices influencing this vertex.
	float								m_BoneWeight[4];		// All bone weights should add up to 1.
	float								m_DecalBlend;			// Blend from base to decal textures: 0=base, 1=decal
	float								m_DecalTx;				// Decal texture X coordinate at this location.
	float								m_DecalTy;				// Decal texture Y coordinate at this location.
	float								m_Reflect;				// Reflectiveness. Modulates specular reflection.
	float								m_Shine;				// Shininess, a.k.a. specular power.
	float								m_SkinBottomTx;			// Bottom texture X coordinate at this location.
	float								m_SkinBottomTy;			// Bottom texture Y coordinate at this location.
	float								m_SkinTopBottomBlend;	// Blend factor from bottom to top skin textures: 0=bottom, 1=top
	float								m_SkinTopTx;			// Top texture X coordinate at this location.
	float								m_SkinTopTy;			// Top texture Y coordinate at this location.

public:	// Constructors
	// Constructor for body mesh.
	inline AuMeatPoint(const AuPoint& loc,
			const AuPoint& surfN,
			float shine,
			float reflect,
			int boneIndex[4],
			float boneWeight[4],
			float skinBottomTx,
			float skinBottomTy,
			float skinTopBottomBlend,
			float skinTopTx,
			float skinTopTy) : AuPointN(loc, surfN)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
		m_Reflect = reflect;
		m_Shine = shine;
		m_SkinBottomTx = skinBottomTx;
		m_SkinBottomTy = skinBottomTy;
		m_SkinTopBottomBlend = skinTopBottomBlend;
		m_SkinTopTx = skinTopTx;
		m_SkinTopTy = skinTopTy;
	}

	// Constructor for eyeball meshes. It could be used for any mesh that only initializes the decal texture.
	inline AuMeatPoint(const AuPoint& loc,
			const AuPoint& surfN,
			float shine,
			float reflect,
			int boneIndex[4],
			float boneWeight[4],
			float hairTx,
			float hairTy) : AuPointN(loc, surfN)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_DecalBlend = 1;
		m_DecalTx = hairTx;
		m_DecalTy = hairTy;
		m_Reflect = reflect;
		m_Shine = shine;
		m_SkinBottomTx = 0;
		m_SkinBottomTy = 0;
		m_SkinTopBottomBlend = 0;
		m_SkinTopTx = 0;
		m_SkinTopTy = 0;
	}

	inline AuMeatPoint()
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = 0;
			m_BoneWeight[ib] = 0;
		}
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
		m_Reflect = 0;
		m_Shine = 0;
		m_SkinBottomTx = 0;
		m_SkinBottomTy = 0;
		m_SkinTopBottomBlend = 0;
		m_SkinTopTx = 0;
		m_SkinTopTy = 0;
	}

	inline AuMeatPoint(const AuMeatPoint& c) : AuPointN(c)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_DecalBlend = c.m_DecalBlend;
		m_DecalTx = c.m_DecalTx;
		m_DecalTy = c.m_DecalTy;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
		m_SkinBottomTx = c.m_SkinBottomTx;
		m_SkinBottomTy = c.m_SkinBottomTy;
		m_SkinTopBottomBlend = c.m_SkinTopBottomBlend;
		m_SkinTopTx = c.m_SkinTopTx;
		m_SkinTopTy = c.m_SkinTopTy;
	}

public:	// Operators
	inline const AuMeatPoint& operator=(const AuMeatPoint& c)
	{
		AuPointN::operator=(c);
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_DecalBlend = c.m_DecalBlend;
		m_DecalTx = c.m_DecalTx;
		m_DecalTy = c.m_DecalTy;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
		m_SkinBottomTx = c.m_SkinBottomTx;
		m_SkinBottomTy = c.m_SkinBottomTy;
		m_SkinTopBottomBlend = c.m_SkinTopBottomBlend;
		m_SkinTopTx = c.m_SkinTopTx;
		m_SkinTopTy = c.m_SkinTopTy;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointN::operator>>(out);
		quint8 hasWhat = 0;
		if(m_DecalBlend > 0)
			hasWhat |= 0x02;
		if(m_Reflect > 0)
			hasWhat |= 0x04;
		if(m_Shine > 0)
			hasWhat |= 0x08;
		if(m_SkinTopBottomBlend > 0)
			hasWhat |= 0x10;

		out << hasWhat;

		for(int ib=0; ib<4; ib++)
		{
			out << m_BoneIndex[ib];
			out << m_BoneWeight[ib];
		}

		if(hasWhat & 0x02)
		{
			out << m_DecalBlend;
			out << m_DecalTx;
			out << m_DecalTy;
		}

		if(hasWhat & 0x04)
			out << m_Reflect;

		if(hasWhat & 0x08)
			out << m_Shine;

		out << m_SkinBottomTx;
		out << m_SkinBottomTy;

		if(hasWhat & 0x10)
			out << m_SkinTopBottomBlend;

		out << m_SkinTopTx;
		out << m_SkinTopTy;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointN::operator<<(in);
		quint8 hasWhat;
		in >> hasWhat;

		for(int ib=0; ib<4; ib++)
		{
			in >> m_BoneIndex[ib];
			in >> m_BoneWeight[ib];
		}

		if(hasWhat & 0x02)
		{
			in >> m_DecalBlend;
			in >> m_DecalTx;
			in >> m_DecalTy;
		}

		if(hasWhat & 0x04)
			in >> m_Reflect;

		if(hasWhat & 0x08)
			in >> m_Shine;

		in >> m_SkinBottomTx;
		in >> m_SkinBottomTy;

		if(hasWhat & 0x10)
			in >> m_SkinTopBottomBlend;

		in >> m_SkinTopTx;
		in >> m_SkinTopTy;
	}

public:	// Methods
	// 'bone' is which of my bones to set, 0 to 3.
	// 'boneIndex' is the index of the bone matrix among all bone matrices.
	// 'boneWeight' is the weight to assign, which is averaged into the existing weights.
	void AddBone(int bone, quint8 boneIndex, float boneWeight);

	inline void RemoveDecal()
	{
		m_DecalBlend = 0;
		m_DecalTx = 0;
		m_DecalTy = 0;
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
};

inline QDataStream& operator<<(QDataStream& out, const AuMeatPoint& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMeatPoint& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle for rendering meat.

class AuMeatTri
{
public:
	enum PART
	{
		Unknown,
		Arm,
		EyeBall,
		Finger,
		Foot,
		Head,
		Hand,
		JawLower,
		JawUpper,
		Leg,
		MammaryLower,
		MammaryUpper,
		Neck,
		Tail,							// Could also be abdominal section of insect or spider.
		Teeth,
		Thumb,
		TorsoLower,
		TorsoUpper,
	};

public:
	quint8								m_Part;					// PART of the meat, for clothing generation.
	AuMeatPoint							m_V0;					// Index into the mesh's vertex array.
	AuMeatPoint							m_V1;					// Index into the mesh's vertex array.
	AuMeatPoint							m_V2;					// Index into the mesh's vertex array.

private:
	mutable AuPoint						m_N;

public:
	inline AuMeatTri(PART part, const AuMeatPoint& m0, const AuMeatPoint& m1, const AuMeatPoint& m2) :
		m_V0(m0), m_V1(m1), m_V2(m2)
	{
		m_Part = part;
	}

	AuMeatTri(PART part, const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, double radius,
			float s, float t, float u, float v, quint8 bone);

	inline AuMeatTri()
	{
		m_Part = Unknown;
	}

	inline AuMeatTri(const AuMeatTri& c)
	{
		m_N = c.m_N;
		m_Part = c.m_Part;
		m_V0 = c.m_V0;
		m_V1 = c.m_V1;
		m_V2 = c.m_V2;
	}

public:	// Operators
	inline const AuMeatTri& operator=(const AuMeatTri& c)
	{
		m_N = c.m_N;
		m_Part = c.m_Part;
		m_V0 = c.m_V0;
		m_V1 = c.m_V1;
		m_V2 = c.m_V2;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Part;
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Part;
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;
		m_N = AuPoint(0,0);
	}

public:	// Methods
	// Returns true if any vertex of this triangle is above 'z'.
	bool Above(double z) const;

	// 'mTex' orients my vertices to the texture and determines up direction.
	// 'length' is the length of the texture in Y. It also applies to X equally until something different is needed.
	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
	// 'blendAngle' is the angle at which the texture changes from top to bottom. See TopBottomBlend function.
	// 'blendWid' is the angle width of the blend zone at the transition.
	void ApplySkinTexture(const AuMatrix4x4& mTex, float length, int topBottomBlend, float blendAngle, float blendWid);

	// Returns the area of this triangle.
	double Area() const;

	// Returns true if any vertex of this triangle is below 'z'.
	bool Below(double z) const;

	// Returns the center point of this triangle.
	AuPoint Center() const;

	// Returns true if any triangles are added to result.
	//bool Clip(const AuPoint& clipLoc, AuMeatMesh& result, const AuMeatMesh& mesh, double tol=FUZZ) const;

	// Returns true if any triangles are added to result.
	//bool Clip(const AuSegment& clipSeg, AuMeatMesh& result, const AuMeatMesh& mesh) const;

	// Returns true if 'loc' is inside of this triangle in 3D.
	// The triangle can be clockwise or counterclockwise.
	// 'loc' must be IN the plane of this triangle to return true.
	int Contains(const AuPoint& loc) const;

	int ContainsFuzzy(const AuPoint& loc) const;

	// Returns the 3D extents box of this triangle.
	void Extents(AuPoint& min, AuPoint& max) const;

	inline bool IsHead() const
	{
		switch(Part())
		{
		case EyeBall:
		case Head:
		case JawLower:
		case JawUpper:
		case Teeth:
			return true;
		case Unknown:
		case Arm:
		case Finger:
		case Foot:
		case Hand:
		case Leg:
		case MammaryLower:
		case MammaryUpper:
		case Neck:
		case Tail:
		case Thumb:
		case TorsoLower:
		case TorsoUpper:
			break;
		}
		return false;
	}

	inline bool IsMouth() const
	{
		switch(Part())
		{
		case JawLower:
		case JawUpper:
		case Teeth:
			return true;
		case Unknown:
		case Arm:
		case EyeBall:
		case Finger:
		case Foot:
		case Hand:
		case Head:
		case Leg:
		case MammaryLower:
		case MammaryUpper:
		case Neck:
		case Tail:
		case Thumb:
		case TorsoLower:
		case TorsoUpper:
			break;
		}
		return false;
	}

	inline PART Part() const
	{
		return (PART)m_Part;
	}

	void RemoveDecal();

	// Tests a segment for intersection with this triangle.
	// 'intersect' is set to the intersection of the line through seg1~seg2 to the plane of this triangle.
	// 1 is returned if 'intersect' is in this triangle.
	// 2 is returned if 'intersect' and 'intesect2' are in this triangle, if the segment is parallel to the plane of this triangle.
	// 0 is returnd if there is no intersection or if 'intersect' is not in this triangle.
	int SegmentIntersection(AuPoint& intersect, AuPoint& intersect2, const AuPoint& seg1, const AuPoint& seg2,
			const AuMeatMesh& mesh) const;

	// Tests a segment for intersection with this triangle.
	// 'intersect' is set to the intersection of the line through seg1~seg2 to the plane of this triangle.
	// Returns true if the intersection falls within this triangle.
	// Returns false if the intersection does not fall within this triangle.
	// Returns false if the segment is parallel to the plane of this triangle.
	bool SegmentIntersection(AuPoint& intersect, const AuPoint& seg1, const AuPoint& seg2) const;

	inline void SetPart(PART part)
	{
		m_Part = part;
	}

	// Checks for intersection between this triangle and 'tri', specifically looking for a segment intersection.
	// Triangles whose planes are parallel do not intersect, even if they are coplanar.
	// Triangles that touch each other at only one location do not intersect.
	bool TriangleIntersection(const AuMeatTri& tri, const AuMeatMesh& trimesh, AuPoint& seg1, AuPoint& seg2,
			const AuMeatMesh& mesh) const;

	// Returns the unit vector that is perpendicular to this triangle. Will be 0,0 if it hasn't been calculated yet.
	const AuPoint& SurfaceNormal() const
	{
		return m_N;
	}

	// Returns the unit vector that is perpendicular to this triangle. Will calculate if not calculated yet.
	const AuPoint& SurfaceNormalCalc() const;

	// 'midAngle' is the angle at which the texture changes from top to bottom. PI/2 at the top, 0 at the side, -PI/2 at the bottom.
	// Returns a number from 0 to 1. 0=bottom, 1=top.
	inline static float TopBottomBlend(const AuPoint& skinUp, const AuPoint& updir, float midAngle, float widAngle)
	{
		double d = skinUp.Dot(updir);
		float ang = asinf(d);

		float angmin = midAngle - widAngle/2;
		if(ang <= angmin)
			return 0;

		float angmax = midAngle + widAngle/2;
		if(ang >= angmax)
			return 1;

		float frac = (ang - angmin) / widAngle;
		if(frac > 1)
			return 1;

		return frac;
	}

	inline void Transform(const AuMatrix4x4& m)
	{
		m_N = m.ProjectVector(m_N);
		m_V0.Transform(m);
		m_V1.Transform(m);
		m_V2.Transform(m);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuMeatTri& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMeatTri& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is triangle center point.

class AuMeatMesh : public QList<AuMeatTri>
{
private:
	AuHairMesh							m_HairMesh;
	AuPoint								m_Max;
	AuPoint								m_Min;

public:	// Constructors
	inline AuMeatMesh()
	{
	}

	AuMeatMesh(const AuScFaceList& faces, AuMeatTri::PART part);

	AuMeatMesh(const AuScFaceList& faces, AuMeatTri::PART part, int topBottomBlend, const AuPoint& upVec, float blendAngle,
			float blendWid);

	inline AuMeatMesh(const AuMeatMesh& mesh) : QList<AuMeatTri>(mesh), m_HairMesh(mesh.m_HairMesh)
	{
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
	}

public:
	const AuMeatMesh& operator=(const AuMeatMesh& mesh)
	{
		QList<AuMeatTri>::operator=(mesh);
		m_HairMesh = mesh.m_HairMesh;
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
		return *this;
	}

public:	// Methods
	inline void Append(const AuMeatTri& t)
	{
		ExpandExtents(t);
		append(t);
	}

	inline void Append(const AuMeatMesh& sourceMesh)
	{
		for(const_iterator mi = sourceMesh.cbegin(); mi != sourceMesh.cend(); mi++)
			Append(*mi);
		m_HairMesh.Append(sourceMesh.m_HairMesh);
	}

	// Applies a decal texture to this mesh.
	// 'decal' is the decal to apply.
	// 'm' is the transformation matrix to position the decal.
	// 'decalWidth' is the width of the decal rectangle (X axis in m).
	// 'decalHeight' is the height of the decal rectangle (Y axis in m).
	// 'decalAlpha' is the alpha of the decal, for transparency/intensity of application.
	void ApplyDecal(MEAT_DECAL decal, const AuMatrix4x4& m, float decalWidth, float decalHeight, float decalAlpha); 

	// 'origin' is the point where the texture coordinates Y will be 0.
	// 'yAxis' specifies the direction in which the texture will be stretched from 0 at the origin, to 1 at 'length'.
	// 'upVec' is used for determining top-to-bottom texture coordinates for the blend. It points toward the sun, to tan the top.
	// 'length' is the length of the texture in Y. It also applies to X equally until something different is needed.
	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
	// 'blendAngle' is the angle at which the texture changes from top to bottom.
	// 'blendWid' is the angle width of the blend zone at the transition.
	void ApplySkinTexture(const AuPoint& origin, const AuPoint& yAxis, const AuPoint& upVec, float length, int topBottomBlend,
			float blendAngle, float blendWid);

	// Looks for the first triangle of type 'part'. Returns false if none is found.
	// Returns true if a triangle is found and sets 'boneIndexN' and 'boneWeightN' values from the triangle.
	bool Bones(AuMeatTri::PART part, quint8 boneIndex[4], float boneWeight[4]) const;

	// Any triangle with an edge length greater than 'maxEdgeLength' is broken into three triangles, sharing a new common vertex
	// at the center of the old triangle.
	void BreakBigTriangles(double maxEdgeLength);

	// 'mesh' is used to expand this mesh outward.
	// 'vector' is the direction to push my vertices.
	// 'tensionRadius' is the distance from the affected vertices that other vertices are stretched along with the bulge.
	// 'isOut' is true to expand this mesh outward to encompass 'mesh'.
	// 'isOut' is false to expand this mesh inward to encompass 'mesh'.
	void Bulge(const AuMeatMesh& mesh, const AuPoint& vector, double tensionRadius, bool isOut);

	// 'poly' is used to expand this mesh outward.
	// 'vector' is the direction to push my vertices.
	// 'tensionRadius' is the distance from the affected vertices that other vertices are stretched along with the bulge.
	// 'isOut' is true to expand this mesh outward to encompass 'poly'.
	// 'isOut' is false to expand this mesh inward to encompass 'poly'.
	void Bulge(const AuPointArray& poly, const AuPoint& vector, double tensionRadius, bool isOut, bool fill=false);

	inline void Clear()
	{
		clear();
		m_HairMesh.Clear();
	}

	// Clips the triangles of this mesh using 'intersection'.
	// Triangles that end up inside 'mesh' are discarded.
	//void ClipTriangles(const AuMeatMesh& mesh, const AuSegmentArray& intersection);

	// Combines separate chunks into one contiguous skin.
	void CombineChunks();

	inline int Count() const
	{
		return count();
	}

	// Create VAO(s) for use with the Animal shader.
	inline AuGLVAO CreateAnimalHairVAO(AuOpenGLFunctions& re, bool isTop) const
	{
		return m_HairMesh.CreateAnimalHairVAO(re, isTop);
	}

	AuGLVAO CreateAnimalVAO(AuOpenGLFunctions& re) const;

	inline AuGLVAO CreateHeadHairVAO(AuOpenGLFunctions& re, bool isTop) const
	{
		return m_HairMesh.CreateHeadHairVAO(re, isTop);
	}

	AuGLVAO CreateHeadVAO(AuOpenGLFunctions& re) const;

	// Creates a VAO for use with the Lines shader.
	AuGLVAO CreateLinesVAO(AuOpenGLFunctions& re) const;

	void ExpandExtents(const AuMeatTri& t);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		min = m_Min;
		max = m_Max;
	}

	void FromFaces(const AuScFaceList& faces, AuMeatTri::PART part);

	// 'upVec' is used for determining top-to-bottom/side-to-side texture coordinates.
	// 'topBottomBlend' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
	void FromFaces(const AuScFaceList& faces, AuMeatTri::PART part, int topBottomBlend, const AuPoint& upVec, float blendAngle,
			float blendWid);

	// Returns the lowest minimum Y, either hair or meat.
	inline double GrossMinY() const
	{
		if(m_HairMesh.Min().y < m_Min.y)
			return m_HairMesh.Min().y;
		return m_Min.y;
	}

	// Returns the lowest minimum Z, either hair or meat.
	inline double GrossMinZ() const
	{
		if(m_HairMesh.Min().z < m_Min.z)
			return m_HairMesh.Min().z;
		return m_Min.z;
	}

	// Returns the highest maximum Y, either hair or meat.
	inline double GrossMaxY() const
	{
		if(m_HairMesh.Max().y > m_Max.y)
			return m_HairMesh.Max().y;
		return m_Max.y;
	}

	// Returns the highest maximum Z, either hair or meat.
	inline double GrossMaxZ() const
	{
		if(m_HairMesh.Max().z > m_Max.z)
			return m_HairMesh.Max().z;
		return m_Max.z;
	}

	inline AuHairMesh& Hair()
	{
		return m_HairMesh;
	}

	inline const AuHairMesh& Hair() const
	{
		return m_HairMesh;
	}

	inline const AuPoint& HairMax() const
	{
		return m_HairMesh.Max();
	}

	inline const AuPoint& HairMin() const
	{
		return m_HairMesh.Min();
	}

	// Finds a point on the inside of my skin from point 'p' along the vector 'vec'.
	// 'skinN' is set to the surface normal of the skin triangle that is hit.
	// CAVEAT: The ray out of 'p' used to find the skin is 10000000 units long.
	// Returns AuPoint(0,0) if 'p' is not inside my skin.
	//DHH AuPoint InsideSkin(const AuPoint& p, const AuPoint& vec, AuPoint& skinN) const;

	// Finds the intersection between each triangle in 'mesh' with my triangles. Adds the segments to 'segs'.
	void IntersectionSegments(AuSegmentArray& segs, const AuMeatMesh& mesh) const;

	inline const AuPoint& Max() const
	{
		return m_Max;
	}

	inline const AuPoint& Min() const
	{
		return m_Min;
	}

	// 'mesh' is combined with this mesh to produce one continuous shell.
	// The only change to 'mesh' is the inside mesh flags are updated by a call to TestInside using this mesh.
	// 'intersection' is an output parameter. It is the intersection(s) between the two meshes.
	//void Join(AuMeatMesh& mesh, AuSegmentArray& intersection);

	// Removes all of this mesh with any vertex above 'z' altitude.
	void RemoveAbove(double z);

	// Removes all of this mesh with any vertex below 'z' altitude.
	void RemoveBelow(double z);

	// Removes all decals from this meat mesh.
	void RemoveDecals();

	// Assigns 'boneIndex' to 'bone', from a weight of 0 at 'fromX' to a weight of 1 at 'toX'. 'fromX' and 'toX' can be greater
	// or less than each other in either order.
	// 'bone' is which of the vertex bones to set, 0 to 3.
	// 'boneIndex' is the index of the bone matrix among all bone matrices.
	void AddBoneRangeBlendX(double fromX, double toX, int bone, quint8 boneIndex);
	void AddBoneRangeBlendXMouth(double fromX, double toX, int bone, quint8 boneIndex);

	// Sets all bones in this mesh. All bone weights must add up to 1.
	void SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2,
			quint8 boneIndex3, float boneWeight3);

	// Sets bones in this mesh whose Y falls between minY and maxY. All bone weights must add up to 1.
	//void SetBonesRangeY(double minY, double maxY, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1,
			//quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3);

	// Sets bones in this mesh whose Z falls between minZ and maxZ. All bone weights must add up to 1.
	void SetBonesRangeZ(double minZ, double maxZ, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1,
			quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3);
	void SetBonesRangeYZ(double minY, double maxY, double minZ, double maxZ, quint8 boneIndex0, float boneWeight0,
			quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3);

	// Assigns 'boneMin' and 'boneMax' to mesh vertices between minY and maxY, inclusive.
	// Bone weight is linearly interpolated between vertices. 'boneMin' is 1 at minY; 'boneMax' is 1 at maxY.
	void SetBonesRangeBlendY(double minY, double maxY, quint8 boneMin, quint8 boneMax);

	// Assigns 'boneMin' and 'boneMax' to mesh vertices between minZ and maxZ, inclusive.
	// Bone weight is linearly interpolated between vertices. 'boneMin' is 1 at minZ; 'boneMax' is 1 at maxZ.
	void SetBonesRangeBlendZ(double minZ, double maxZ, quint8 boneMin, quint8 boneMax);

	inline void SetHairBones(quint16 id, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1,
			quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3)
	{
		m_HairMesh.SetBones(id, boneIndex0, boneWeight0, boneIndex1, boneWeight1, boneIndex2, boneWeight2, boneIndex3, boneWeight3);
	}

	// Sets the 'part' type on all triangles whose center.z falls between 'minZ' and 'maxZ'.
	void SetPartRangeZ(AuMeatTri::PART part, double minZ, double maxZ);

	// Returns the X, Y and Z dimensions of this mesh.
	inline AuPoint Size() const
	{
		return m_Max - m_Min;
	}

	// Finds a point on my skin from point 'p' along the vector 'vec'.
	// 'skinN' is set to the surface normal of the skin triangle that is hit.
	// CAVEAT: The ray out of 'p' used to find the skin is 10000000 units long.
	// Returns AuPoint(0,0) if the ray from p does not hit my skin at all.
	AuPoint SkinSurface(const AuPoint& p, const AuPoint& vec, AuPoint& skinN) const;

	// Restricts the search to finding only triangles of 'partType'.
	AuPoint SkinSurface(const AuPoint& p, const AuPoint& vec, AuPoint& skinN, AuMeatTri::PART partType) const;

	// Recalculates all surface normals to be smooth.
	void SmoothNormals();

	// Recalculates all surface normals to be smooth. Weighting is based on surface area of each triangle.
	void SmoothNormalsArea();

	// Creates a sphere of triangles in this mesh.
	void Sphere(AuMeatTri::PART part, float triangleSize, float radius, float s, float t, float u, float v, quint8 bone);

	// Splits triangles to eliminate T intersections.
	void SplitTees(double tol);

	void SurfacePoints(AuPointNArray& points) const;

	// Checks to see if 'p' is contained within this meat mesh.
	// NOTE: The ray out of 'p' used to find the skin is 10000000 units long.
	// Returns 0 if 'p' is outside this mesh.
	// Returns 1 if 'p' is on the surface of this mesh.
	// Returns 2 if 'p' is inside this mesh.
	int TestInside(const AuPoint& p) const;

	// The only change to 'mesh' is the inside mesh flags are updated.
	// Returns 0, 1, 2 or 3. The bits are as follows.
	//  bit 0 = a vertex of 'mesh' is outside this mesh.
	//  bit 1 = a vertex of 'mesh' is on the surface of this mesh.
	//  bit 2 = a vertex of 'mesh' is inside this mesh.
	int TestInside(AuMeatMesh& mesh) const;

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
		m_HairMesh.Transform(m);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Meat chunks vary in thickness relative to their center line segment to create muscles, tentacles, ankles, etc.
// For reference, think of your outstretched arm, palm down, when considering the meaning of bottom, top, left and right.
// Torso considers bottom as the front and top as the back, as if lying on your belly.

class AuMeatShape
{
private:
	float								m_BottomRadius;			// Radius below the center line.
	float								m_LeftRadius;			// Radius above the center line.
	float								m_RightRadius;			// Radius to the left of the center line.
	float								m_TopRadius;			// Radius to the right of the center line.

public:	// Constructors/Destructors
	inline AuMeatShape(float bottomRad, float topRad, float leftRad, float rightRad)
	{
		m_BottomRadius = bottomRad;
		m_LeftRadius = leftRad;
		m_RightRadius = rightRad;
		m_TopRadius = topRad;
	}

	inline AuMeatShape()
	{
		m_BottomRadius = 0;
		m_LeftRadius = 0;
		m_RightRadius = 0;
		m_TopRadius = 0;
	}

public:	// Operators
	inline bool operator==(const AuMeatShape& c) const
	{
		if(m_BottomRadius != c.m_BottomRadius)
			return false;
		if(m_LeftRadius != c.m_LeftRadius)
			return false;
		if(m_RightRadius != c.m_RightRadius)
			return false;
		if(m_TopRadius != c.m_TopRadius)
			return false;
		return true;
	}

	inline bool operator!=(const AuMeatShape& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_BottomRadius;
		out << m_LeftRadius;
		out << m_RightRadius;
		out << m_TopRadius;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_BottomRadius;
		in >> m_LeftRadius;
		in >> m_RightRadius;
		in >> m_TopRadius;
	}

	inline AuMeatShape operator*(float v) const
	{
		AuMeatShape m;
		m.m_BottomRadius = m_BottomRadius * v;
		m.m_TopRadius = m_TopRadius * v;
		m.m_LeftRadius = m_LeftRadius * v;
		m.m_RightRadius = m_RightRadius * v;
		return m;
	}

	inline AuMeatShape operator/(float v) const
	{
		AuMeatShape m;
		m.m_BottomRadius = m_BottomRadius / v;
		m.m_TopRadius = m_TopRadius / v;
		m.m_LeftRadius = m_LeftRadius / v;
		m.m_RightRadius = m_RightRadius / v;
		return m;
	}

public:	// Methods
	inline float BottomRadius() const
	{
		return m_BottomRadius;
	}

	inline void Expand(const AuMeatShape& s)
	{
		if(m_BottomRadius < s.m_BottomRadius)
			m_BottomRadius = s.m_BottomRadius;
		if(m_LeftRadius < s.m_LeftRadius)
			m_LeftRadius = s.m_LeftRadius;
		if(m_RightRadius < s.m_RightRadius)
			m_RightRadius = s.m_RightRadius;
		if(m_TopRadius < s.m_TopRadius)
			m_TopRadius = s.m_TopRadius;
	}

	inline float Height() const
	{
		return m_BottomRadius + m_TopRadius;
	}

	inline float LeftRadius() const
	{
		return m_LeftRadius;
	}

	inline void ReverseLeftRight()
	{
		float leftRad = m_LeftRadius;
		m_LeftRadius = m_RightRadius;
		m_RightRadius = leftRad;
	}

	inline void Scale(float scaleWidth, float scaleHeight)
	{
		m_BottomRadius *= scaleHeight;
		m_TopRadius *= scaleHeight;

		m_LeftRadius *= scaleWidth;
		m_RightRadius *= scaleWidth;
	}

	inline float RightRadius() const
	{
		return m_RightRadius;
	}

	inline float TopRadius() const
	{
		return m_TopRadius;
	}

	inline float Width() const
	{
		return m_LeftRadius + m_RightRadius;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuMeatShape& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMeatShape& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Meat profile describes the shape of the meat along the length of a body section, such as a limb or torso.
// The key is distance along the body section. The key of the last entry is the length of the body secton.

class AuMeatProfile : public QMap<float, AuMeatShape>
{
private:
	QList<AuPoint>						m_Joints;				// Distance from origin to each joint. First is always 0.
	float								m_TriangleSize;

public:	// Constructors
	inline AuMeatProfile()
	{
		m_TriangleSize = 25.f * FeetPerMM();
	}

	inline AuMeatProfile(float triangleSize)
	{
		m_TriangleSize = triangleSize;
	}

public:	// Operators
	inline bool operator==(const AuMeatProfile& c) const
	{
		if(count() != c.count())
			return false;

		if(m_Joints.size() != c.m_Joints.size())
			return false;

		if(m_TriangleSize != c.m_TriangleSize)
			return false;

		for(const_iterator mi = cbegin(), ci = c.cbegin(); mi != cend() && ci != c.end(); mi++, ci++)
		{
			if(mi.key() != ci.key())
				return false;
			if(*mi != *ci)
				return false;
		}

		for(QList<AuPoint>::const_iterator ji = m_Joints.cbegin(), ci = c.m_Joints.cbegin();
				ji != m_Joints.cend() && ci != c.m_Joints.end(); ji++, ci++)
			if(*ji != *ci)
				return false;

		return true;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_TriangleSize;

		out << (quint16)count();
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			out << mi.key();
			out << mi.value();
		}

		out << (quint8)m_Joints.count();
		for(QList<AuPoint>::const_iterator ji = m_Joints.cbegin(); ji != m_Joints.cend(); ji++)
			out << *ji;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_TriangleSize;

		quint16 n;
		in >> n;
		float d;
		AuMeatShape m;
		while(n--)
		{
			in >> d;
			in >> m;
			insert(d, m);
		}

		quint8 j;
		in >> j;
		AuPoint l;
		while(j--)
		{
			in >> l;
			m_Joints.append(l);
		}
	}

public:	// Methods
	// Adds 'shape' to this meat profile, using 'distAlongBody' as the key.
	// If another shape is already at (fuzzy) the key location, the shape there is expanded to encompass the new shape.
	void Add(float distAlongBody, const AuMeatShape& shape);

	inline void GetJoints(QList<AuPoint>& joints) const
	{
		joints = m_Joints;
	}

	// 'upVec' is used for determining top-to-bottom/side-to-side texture coordinates.
	// 'tex' is: 0 = bottom texture, 1 = top texture, 2 = blend bottom to top texture,
	void GetMesh(AuMeatMesh& mesh, AuMeatTri::PART part, int tex, const AuPoint& upVec, float blendAngle, float blendWid) const;

	void GetSticks(AuSegmentArray& sticks) const;

	inline float Length() const
	{
		return (end()-1).key() - begin().key();
	}

	// Constructs this meat profile from the contents of a meat profile (.mp) file.
	// When 'lenBetweenJoints' is true, 'length' expresses the desired length between the first joint and the last joint.
	// When 'lenBetweenJoints' is false, 'length' expresses the desired length between the first joint and the tip of the meat.
	// 'width' and 'height' can be specified, otherwise those dimensions are scaled proportionally with length.
	void Load(const QString& fileName, float length, bool lenBetweenJoints, float triSize, float width=0, float height=0);

	inline float MaxBottomRadius() const
	{
		float max = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if((*mi).BottomRadius() > max)
				max = (*mi).BottomRadius();
		return max;
	}

	inline float MaxHeight() const
	{
		float maxt = 0;
		float maxb = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			if((*mi).TopRadius() > maxt)
				maxt = (*mi).TopRadius();
			if((*mi).BottomRadius() > maxb)
				maxb = (*mi).BottomRadius();
		}
		return maxt + maxb;
	}

	inline float MaxWidth() const
	{
		float maxl = 0;
		float maxr = 0;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
		{
			if((*mi).LeftRadius() > maxl)
				maxl = (*mi).LeftRadius();
			if((*mi).RightRadius() > maxr)
				maxr = (*mi).RightRadius();
		}
		return maxl + maxr;
	}

	// Interpolates between meat shapes to return the radii at a particular offset along its length.
	// Offsets less than 0 or greater than the length will return a rounded off interpolation to 0 at the RoundOffDistance() beyond
	// the end.
	void RadiiAt(float offset, float& bottom, float& top, float& left, float& right) const;

	inline void ReverseLeftRight()
	{
		for(iterator mi = begin(); mi != end(); mi++)
			(*mi).ReverseLeftRight();
	}

	// Length beyond either end at which the meat radii round off to zero.
	float RoundOffDistance() const;

	// 'st' is the scaling to be applied to the triangle size.
	inline void Scale(float sx, float sy, float sz, float st)
	{
		m_TriangleSize *= st;

		// Must build a new copy of myself that is scaled due to scaling the keys, which is the Y dimension.
		AuMeatProfile nump;
		for(iterator mi = begin(); mi != end(); mi++)
		{
			AuMeatShape ms = *mi;
			ms.Scale(sx, sz);
			nump.insert(mi.key() * sy, ms);
		}

		clear();
		for(const_iterator ni = nump.cbegin(); ni != nump.cend(); ni++)
			insert(ni.key(), *ni);
	}

	void TriangleBand(AuMeatMesh& mesh, AuMeatTri::PART part, const AuPointArray& pring, const AuPointArray& nring, int tex,
			const AuPoint& upVec, float blendAngle, float blendWid, float texYp, float texYc) const;

	void TriangleFan(AuMeatMesh& mesh, AuMeatTri::PART part, const AuPointArray& ring, const AuPoint& apex, bool reverse, int tex,
			const AuPoint& upVec, float blendAngle, float blendWid, float texY) const;

	void VertexRingAt(float offset, int verts, AuPointArray& ring) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuMeatProfile& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuMeatProfile& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMeat
{
public:
	static void DecalTexture(MEAT_DECAL tx, float& s, float& t, float& u, float& v);

	static void LoadPoly(const QString& fileName, AuPointArray& poly, float wid, float ht);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuMeat_H
