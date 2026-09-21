//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhair.h,v 1.53 2022/04/08 14:42:11 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuHair_H
#define AuHair_H

#include "augleffects.h"
#include <QList>

class AuDNAHair;
class AuSolidEllipsoid;
class QDataStream;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFolliclePoint : public AuPointN
{
public:
	// NOTE: If you add a ROLE, you must make changes in the Roles and RoleNames functions.
	enum ROLE
	{
		// Order of the values in this enum is not significant. Add new ones at the end.
		// The first group are specifically head follicle roles, NOTHING ELSE. Don't use them to mean something else or hair styles
		// will affect those other body parts.
		BackOfHead,						// Hair styled
		Bangs,							// Hair styled
		Crown,							// Hair styled
		Scalp,							// Hair styled
		SideLeft,						// Hair styled
		SideRight,						// Hair styled
		SideburnLeft,					// Hair styled
		SideburnRight,					// Hair styled
		HeadNeckBack,					// Hair styled

		HeadNeckFront,					// Whisker styled
		CheekLeft,						// Whisker styled
		CheekRight,						// Whisker styled
		ChinFront,						// Whisker styled
		ChinLeft,						// Whisker styled
		ChinRight,						// Whisker styled
		EarLeft,						// Whisker styled
		EarMiddle,						// Whisker styled
		EarRight,						// Whisker styled
		EyeLeft,						// Whisker styled
		EyeMiddle,						// Whisker styled
		EyeRight,						// Whisker styled
		Face,							// Whisker styled
		LipTop,							// Whisker styled
		ChopLeft,						// Whisker styled
		ChopRight,						// Whisker styled
		JowlLeft,						// Whisker styled
		JowlRight,						// Whisker styled
		Nose,							// Whisker styled

		NeckBack,						// Remaining roles are for body parts that are not the head.
		NeckUnder,
		NeckSides,

		Shoulders,
		Back,
		Chest,
		FlanksBack,
		FlanksUnder,
		Butt,
		Groin,

		ArmBack,						// Remaining roles are for body parts that are not the head.
		ArmUnder,
		WristBack,
		WristUnder,
		Hands,
		Palms,

		LegBack,						// Remaining roles are for body parts that are not the head.
		LegUnder,
		AnkleBack,						// Remaining roles are for body parts that are not the head.
		AnkleUnder,
		Feet,
		Soles,

		TailBack,
		TailUnder,
		TailTipBack,
		TailTipUnder,
		ROLE_Count						// ROLE_Count must not exceed 64.
	};

public:
	QColor								m_Color;
	quint8								m_Id;					// Identifier for bone application. See usage.
	float								m_MaxLength;			// Maximum length of hair from this follicle. Not styled length.
	ROLE								m_Role;

public:	// Constructors
	inline AuFolliclePoint(quint8 id, ROLE role, const AuPointN& loc, const QColor& color, float maxLen) : AuPointN(loc)
	{
		m_Color = color;
		m_Id = id;
		m_MaxLength = maxLen;
		m_Role = role;
	}

	inline AuFolliclePoint(quint8 id, ROLE role, const AuPoint& loc, const AuPoint& n, const QColor& color, float maxLen) :
		AuPointN(loc, n)
	{
		m_Color = color;
		m_Id = id;
		m_MaxLength = maxLen;
		m_Role = role;
	}

	inline AuFolliclePoint()
	{
		m_Id = 0;
		m_MaxLength = 0;
		m_Role = BackOfHead;
	}

	inline AuFolliclePoint(const AuFolliclePoint& c) : AuPointN(c)
	{
		m_Color = c.m_Color;
		m_Id = c.m_Id;
		m_MaxLength = c.m_MaxLength;
		m_Role = c.m_Role;
	}

public:	// Operators
	inline const AuFolliclePoint& operator=(const AuFolliclePoint& c)
	{
		AuPointN::operator=(c);
		m_Color = c.m_Color;
		m_Id = c.m_Id;
		m_MaxLength = c.m_MaxLength;
		m_Role = c.m_Role;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointN::operator>>(out);
		out << m_Color;
		out << m_Id;
		out << m_MaxLength;
		out << (quint8)m_Role;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointN::operator<<(in);
		in >> m_Color;
		in >> m_MaxLength;
		in >> m_Id;
		quint8 ui8;
		in >> ui8;
		m_Role = (ROLE)ui8;
	}

	inline bool operator==(const AuFolliclePoint& p) const
	{
		if(m_Id != p.m_Id)
			return false;
		if(m_Role != p.m_Role)
			return false;
		if(!AuPointN::operator==(p))
			return false;
		if(m_Color != p.m_Color)
			return false;
		if(m_MaxLength != p.m_MaxLength)
			return false;
		return true;
	}

	inline bool operator!=(const AuFolliclePoint& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline const QColor& HairColor() const
	{
		return m_Color;
	}

	inline quint8 Id() const
	{
		return m_Id;
	}

	// Returns true if the corresponding bit for 'roleWanted' is on in 'roleBits'.
	inline static bool IsHair(ROLE roleWanted, quint64 roleBits)
	{
		return roleBits & RoleBit(roleWanted);
	}

	inline bool IsHead() const
	{
		return IsHead(m_Role);
	}

	inline static bool IsHead(ROLE role)
	{
		return role <= Nose;
	}

	inline bool IsMouth() const
	{
		return IsMouth(m_Role);
	}

	inline static bool IsMouth(ROLE role)
	{
		switch(role)
		{
		case AuFolliclePoint::ChinFront:
		case AuFolliclePoint::ChinLeft:
		case AuFolliclePoint::ChinRight:
		//case AuFolliclePoint::Face:
		case AuFolliclePoint::LipTop:
			return true;
		default:
			break;
		}
		return false;
	}

	inline float MaxLength() const
	{
		return m_MaxLength;
	}

	inline ROLE Role() const
	{
		return m_Role;
	}

	inline quint64 RoleBit() const
	{
		return RoleBit(m_Role);
	}

	static inline quint64 RoleBit(ROLE role)
	{
		return 1ULL << role;	// 64 bit shift requires the 1 to be cast as a 64 bit value, otherwise it's a 32 bit int.
	}

	// Fills 'roleMap' with all known role names and their corresponding bit.
	static void Roles(QMap<QString, ROLE>& roleMap);

	static QStringList RoleNames(quint64 roleBits);
};

inline QDataStream& operator<<(QDataStream& out, const AuFolliclePoint& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFolliclePoint& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFolliclePointArray : public QVector<AuFolliclePoint>
{
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Vertex of hair model.

class AuHairPoint : public AuPointN
{
public:
	quint8								m_BoneIndex[4];			// Indexes of bone matrices influencing this vertex.
	float								m_BoneWeight[4];		// All bone weights should add up to 1.
	QColor								m_Color;				// Hair color, alpha ignored.
	float								m_HairTx;				// Hair texture X coordinate at this location.
	float								m_HairTy;				// Hair texture Y coordinate at this location.
	float								m_Reflect;				// Reflectiveness. Modulates specular reflection.
	float								m_Shine;				// Shininess, a.k.a. specular power.

public:	// Constructors
	inline AuHairPoint(const AuPoint& loc,
			const AuPoint& surfN,
			float shine,
			float reflect,
			int boneIndex[4],
			float boneWeight[4],
			float hairTx,
			float hairTy,
			const QColor& color) : AuPointN(loc, surfN)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_Color = color;
		m_HairTx = hairTx;
		m_HairTy = hairTy;
		m_Reflect = reflect;
		m_Shine = shine;
	}

	inline AuHairPoint()
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = 0;
			m_BoneWeight[ib] = 0;
		}
		m_HairTx = 0;
		m_HairTy = 0;
		m_Reflect = 0;
		m_Shine = 0;
	}

	inline AuHairPoint(const AuHairPoint& c) : AuPointN(c)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_Color = c.m_Color;
		m_HairTx = c.m_HairTx;
		m_HairTy = c.m_HairTy;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
	}

public:	// Operators
	inline const AuHairPoint& operator=(const AuHairPoint& c)
	{
		AuPointN::operator=(c);
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_Color = c.m_Color;
		m_HairTx = c.m_HairTx;
		m_HairTy = c.m_HairTy;
		m_Reflect = c.m_Reflect;
		m_Shine = c.m_Shine;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointN::operator>>(out);
		for(int ib=0; ib<4; ib++)
		{
			out << m_BoneIndex[ib];
			out << m_BoneWeight[ib];
		}
		out << m_Color;
		out << m_HairTx;
		out << m_HairTy;
		out << m_Reflect;
		out << m_Shine;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointN::operator<<(in);
		for(int ib=0; ib<4; ib++)
		{
			in >> m_BoneIndex[ib];
			in >> m_BoneWeight[ib];
		}
		in >> m_Color;
		in >> m_HairTx;
		in >> m_HairTy;
		in >> m_Reflect;
		in >> m_Shine;
	}

public:	// Methods
	// 'bone' is which of my bones to set, 0 to 3.
	// 'boneIndex' is the index of the bone matrix among all bone matrices.
	// 'boneWeight' is the weight to assign, which is averaged into the existing weights.
	void AddBone(int bone, quint8 boneIndex, float boneWeight);

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

inline QDataStream& operator<<(QDataStream& out, const AuHairPoint& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHairPoint& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle for rendering meat.

class AuHairTri
{
public:
	AuPoint								m_Follicle;				// Follicle location from which this hair tri emerged.
	quint8								m_Id;					// Follicle id that produced this hair tri.
	AuFolliclePoint::ROLE				m_Role;					// Role of this hair tri.
	bool								m_Top;					// True if this is the top facing hair, false for bottom facing.
	AuHairPoint							m_V0;					// Index into the mesh's vertex array.
	AuHairPoint							m_V1;					// Index into the mesh's vertex array.
	AuHairPoint							m_V2;					// Index into the mesh's vertex array.

private:
	mutable AuPoint						m_N;

public:
	inline AuHairTri(const AuFolliclePoint& follicle, const AuHairPoint& m0, const AuHairPoint& m1, const AuHairPoint& m2,
			bool isTop) :
		m_Follicle(follicle), m_V0(m0), m_V1(m1), m_V2(m2)
	{
		m_Id = follicle.Id();
		m_Role = follicle.Role();
		m_Top = isTop;
	}

	inline AuHairTri()
	{
		m_Id = 0;
		m_Role = AuFolliclePoint::BackOfHead;
		m_Top = false;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Follicle;
		out << m_Id;
		out << (quint8)m_Role;
		out << m_Top;
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Follicle;
		in >> m_Id;
		quint8 ui8;
		in >> ui8;
		m_Role = (AuFolliclePoint::ROLE)ui8;
		in >> m_Top;
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;

		m_N = AuPoint(0,0);
	}

public:	// Methods
	// Returns true if any vertex of this triangle is above 'z'.
	bool Above(double z) const;

	// Returns the area of this triangle.
	double Area() const;

	// Returns true if any vertex of this triangle is below 'z'.
	bool Below(double z) const;

	// Returns the center point of this triangle.
	AuPoint Center() const;

	// Returns the 3D extents box of this triangle.
	void Extents(AuPoint& min, AuPoint& max) const;

	inline const AuPoint& Follicle() const
	{
		return m_Follicle;
	}

	inline quint8 Id() const
	{
		return m_Id;
	}

	inline bool IsHead() const
	{
		return AuFolliclePoint::IsHead(m_Role);
	}

	// True if this is a top facing hair triangle.
	inline bool IsTop() const
	{
		return m_Top;
	}

	inline AuFolliclePoint::ROLE Role() const
	{
		return m_Role;
	}

	// Returns the unit vector that is perpendicular to this triangle.
	const AuPoint& SurfaceNormalCalc() const;

	inline void Transform(const AuMatrix4x4& m)
	{
		m_N = m.ProjectVector(m_N);
		m_V0.Transform(m);
		m_V1.Transform(m);
		m_V2.Transform(m);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHairTri& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHairTri& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuHairStyleLock
{
public:
	enum STYLE
	{
		Shaved,							// Hair lock is completely removed.
		Comb,							// Straight out from the follicles. Combs in a direction. Does not hang with gravity.
		CombHang,						// Comb that hangs with gravity.
		HeadComb,						// CombHang that follows the head closely.
		HeadFlattop,					// Needs skull top height. Limits hair to a specific height above the skull.
		HeadKnot,						// Combs close to the head to a knot location then hangs with gravity.
		Mohawk,							// Comb limited to narrow band.
		MohawkHang,						// Limited to narrow band. Hangs with gravity.
		MoustacheComb,					// Ignores head. Limits hair to inside/outside facial angles. Falls down w/o hair spring.
		MoustacheCombHang,				// MoustacheComb with hair spring.
		Rough,							// CombHang with extra waviness for a rough unkempt look.
		SoulPatch,						// Ignores head. Hangs with gravity. Part angle divides above from below.
		HandlebarCombHang,				// MoustacheCombHang with upcurl at the edges.
	};

private:
	bool								m_Base;					// True to put a patch at the base for solid hair density.
	AuPoint								m_CombLeft;				// Comb vector of this lock left of the part.
	AuPoint								m_CombRight;			// Comb vector of this lock right of the part.
	float								m_HangLeft;				// Hang length of this lock left of the part.
	float								m_HangRight;			// Hang length of this lock right of the part.
	float								m_HeadSpaceLeft;		// Head space of this lock left of the part.
	float								m_HeadSpaceRight;		// Head space of this lock right of the part.
	float								m_KnotLeft;				// Knot length of this lock left of the part.
	float								m_KnotRight;			// Knot length of this lock right of the part.
	float								m_LengthLeft;			// Length of this lock left of the part.
	float								m_LengthRight;			// Length of this lock right of the part.
	float								m_PartAngle;			// Angle left or right of head center to part hair.
	bool								m_PartHair;				// True if we are parting the hair.
	STYLE								m_StyleLeft;			// Left of part
	STYLE								m_StyleRight;			// Right of part
	float								m_WidAngLeft;
	float								m_WidAngRight;

public:
	inline AuHairStyleLock()
	{
		m_Base = false;
		m_HangLeft = 0;
		m_HangRight = 0;
		m_HeadSpaceLeft = 0;
		m_HeadSpaceRight = 0;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = 0;
		m_LengthRight = 0;
		m_PartAngle = 0;
		m_PartHair = false;
		m_StyleLeft = Shaved;
		m_StyleRight = Shaved;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}

	inline AuHairStyleLock(const AuHairStyleLock& c)
	{
		m_Base = c.m_Base;
		m_CombLeft = c.m_CombLeft;
		m_CombRight = c.m_CombRight;
		m_HangLeft = c.m_HangLeft;
		m_HangRight = c.m_HangRight;
		m_HeadSpaceLeft = c.m_HeadSpaceLeft;
		m_HeadSpaceRight = c.m_HeadSpaceRight;
		m_KnotLeft = c.m_KnotLeft;
		m_KnotRight = c.m_KnotRight;
		m_LengthLeft = c.m_LengthLeft;
		m_LengthRight = c.m_LengthRight;
		m_PartAngle = c.m_PartAngle;
		m_PartHair = c.m_PartHair;
		m_StyleLeft = c.m_StyleLeft;
		m_StyleRight = c.m_StyleRight;
		m_WidAngLeft = c.m_WidAngLeft;
		m_WidAngRight = c.m_WidAngRight;
	}

public:	// Operators
	inline const AuHairStyleLock& operator=(const AuHairStyleLock& c)
	{
		m_Base = c.m_Base;
		m_CombLeft = c.m_CombLeft;
		m_CombRight = c.m_CombRight;
		m_HangLeft = c.m_HangLeft;
		m_HangRight = c.m_HangRight;
		m_HeadSpaceLeft = c.m_HeadSpaceLeft;
		m_HeadSpaceRight = c.m_HeadSpaceRight;
		m_KnotLeft = c.m_KnotLeft;
		m_KnotRight = c.m_KnotRight;
		m_LengthLeft = c.m_LengthLeft;
		m_LengthRight = c.m_LengthRight;
		m_PartAngle = c.m_PartAngle;
		m_PartHair = c.m_PartHair;
		m_StyleLeft = c.m_StyleLeft;
		m_StyleRight = c.m_StyleRight;
		m_WidAngLeft = c.m_WidAngLeft;
		m_WidAngRight = c.m_WidAngRight;
		return *this;
	}

public:	// Methods
	inline bool BasePatch() const
	{
		return m_Base;
	}

	// Returns 0=no, 1=yes, 2=after knotting, 3=yes no springiness to hair
	inline static int FallWithGravity(STYLE style)
	{
		switch(style)
		{
		case AuHairStyleLock::Comb:
		case AuHairStyleLock::HeadFlattop:
		case AuHairStyleLock::Mohawk:
		case AuHairStyleLock::Shaved:
			break;
		case AuHairStyleLock::CombHang:
		case AuHairStyleLock::HandlebarCombHang:
		case AuHairStyleLock::HeadComb:
		case AuHairStyleLock::MohawkHang:
		case AuHairStyleLock::MoustacheCombHang:
		case AuHairStyleLock::Rough:
		case AuHairStyleLock::SoulPatch:
			return 1;
		case AuHairStyleLock::HeadKnot:
			return 2;	// Does not fall until after the hair reaches the knot.
		case AuHairStyleLock::MoustacheComb:
			return 3;	// Falls straight downward, no spring outward.
		}
		return 0;
	}

	// Returns true for styles that ignore the geometry of the head.
	inline static bool IgnoreHead(STYLE style)
	{
		switch(style)
		{
		case AuHairStyleLock::HandlebarCombHang:
		case AuHairStyleLock::MoustacheComb:
		case AuHairStyleLock::MoustacheCombHang:
		case AuHairStyleLock::Shaved:
		case AuHairStyleLock::SoulPatch:
			return true;
		case AuHairStyleLock::Comb:
		case AuHairStyleLock::HeadComb:
		case AuHairStyleLock::HeadFlattop:
		case AuHairStyleLock::HeadKnot:
		case AuHairStyleLock::Mohawk:
		case AuHairStyleLock::MohawkHang:
		case AuHairStyleLock::CombHang:
		case AuHairStyleLock::Rough:
			break;
		}
		return false;
	}

	// Close styles do not stray far from the skull, instead remaining within a narrow range of altitude above the skull.
	// Once combed hair is knotted, it is allowed to fall.
	inline static bool IsKeptClose(STYLE style)
	{
		switch(style)
		{
		case AuHairStyleLock::HeadComb:
		case AuHairStyleLock::HeadKnot:
			return true;
		case AuHairStyleLock::Comb:
		case AuHairStyleLock::HandlebarCombHang:
		case AuHairStyleLock::HeadFlattop:
		case AuHairStyleLock::Mohawk:
		case AuHairStyleLock::MohawkHang:
		case AuHairStyleLock::MoustacheComb:
		case AuHairStyleLock::MoustacheCombHang:
		case AuHairStyleLock::Shaved:
		case AuHairStyleLock::SoulPatch:
		case AuHairStyleLock::CombHang:
		case AuHairStyleLock::Rough:
			break;
		}
		return false;
	}

	// 'f' is an input parameter describing the hair follicle.
	// 'headSpace' is an input/output parameter which specifies the distance to maintain between hair and skull. On input, it is
	//   the minimum distance.
	// 'lockLen' is an output parameter specifying the maximum length of the lock of hair.
	// 'hangLength' is an output parameter specifying the length the lock of hair is cut to after crowning the skull and heading
	//   downward with gravity.
	// 'knotLen' is an output parameter specifying how much hair is used up in the knot at the back of the head, if any.
	// 'widAng' is an output parameter specifying the width angle for knots and mohawks.
	// 'combVec' is an output parameter specifying the direction to comb the lock until it crowns the head and falls.
	// 'basePatch' is an output parameter specifying if the lock should start with a base patch of hair to cover the skin there.
	STYLE Lock(const AuFolliclePoint& f, float& headSpace, float& lockLen, float& hangLen, float& knotLen, float& widAng,
			AuPoint& combVec, bool& basePatch) const;

	static float NoiseFrequency(STYLE style);

	inline void Setup(STYLE style, float len, float headSpace, float hang, const AuPoint& combVec, bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combVec;
		m_CombRight = combVec;
		m_HangLeft = hang;
		m_HangRight = hang;
		m_HeadSpaceLeft = headSpace;
		m_HeadSpaceRight = headSpace;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = len;
		m_LengthRight = len;
		m_PartAngle = 0;
		m_PartHair = false;
		m_StyleLeft = style;
		m_StyleRight = style;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}

	// Handlebar moustache
	inline void SetupHandlebar(float len, float headSpace)
	{
		m_Base = false;
		m_CombLeft = AuPoint(0,0,-1).RotateY(30*DEG2RAD) * .5;
		m_CombRight = AuPoint(0,0,-1).RotateY(-30*DEG2RAD) * .5;
		m_HangLeft = -1;
		m_HangRight = -1;
		m_HeadSpaceLeft = headSpace;
		m_HeadSpaceRight = headSpace;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = len;
		m_LengthRight = len;
		m_PartAngle = 0;
		m_PartHair = true;
		m_StyleLeft = AuHairStyleLock::HandlebarCombHang;
		m_StyleRight = AuHairStyleLock::HandlebarCombHang;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}

	// When 'style' is a HeadFlattop, 'len' is the height above the skull to cut the hair.
	inline void SetupKnotted(STYLE style, float len, float headSpace, float knot, float hang, float widAng, const AuPoint& combVec,
			bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combVec;
		m_CombRight = combVec;
		m_HangLeft = hang;
		m_HangRight = hang;
		m_HeadSpaceLeft = headSpace;
		m_HeadSpaceRight = headSpace;
		m_KnotLeft = knot;
		m_KnotRight = knot;
		m_LengthLeft = len;
		m_LengthRight = len;
		m_PartAngle = 0;
		m_PartHair = false;
		m_StyleLeft = style;
		m_StyleRight = style;
		m_WidAngLeft = widAng;
		m_WidAngRight = widAng;
	}

	inline void SetupKnottedParted(float partAngle, STYLE styleL, STYLE styleR, float lenL, float lenR, float headSpaceL,
			float headSpaceR, float knotL, float knotR, float hangL, float hangR, float widAngL, float widAngR,
			const AuPoint& combL, const AuPoint& combR, bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combL;
		m_CombRight = combR;
		m_HangLeft = hangL;
		m_HangRight = hangR;
		m_HeadSpaceLeft = headSpaceL;
		m_HeadSpaceRight = headSpaceR;
		m_KnotLeft = knotL;
		m_KnotRight = knotR;
		m_LengthLeft = lenL;
		m_LengthRight = lenR;
		m_PartAngle = partAngle;
		m_PartHair = true;
		m_StyleLeft = styleL;
		m_StyleRight = styleR;
		m_WidAngLeft = widAngL;
		m_WidAngRight = widAngR;
	}

	// 'F' is for the front of the moustache, e.g. the short front part of a Fu Manchu.
	// 'S' is for the sides of the moustache, e.g. the long side parts of a Fu Manchu.
	// 'angleF' specifies the front angle of the moustache. Follicles whose normal is within 'angleF' degrees of the front facing
	//   will use the front parameters, which are stored in my left values.
	// 'angleS' specifies the side angle of the moustache. Follicles whose normal is within 'angleS' degrees of the front facing
	//   will use the side parameters, which are stored in my right values.
	//   Follicles outside 'angleS' are shaved.
	inline void SetupMoustache(STYLE styleF, STYLE styleS,
			float angleF, float angleS,
			float lenF, float lenS,
			float headSpaceF, float headSpaceS,
			float hangF, float hangS,
			const AuPoint& combF, const AuPoint& combS,
			bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combF;
		m_CombRight = combS;
		m_HangLeft = hangF;
		m_HangRight = hangS;
		m_HeadSpaceLeft = headSpaceF;
		m_HeadSpaceRight = headSpaceS;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = lenF;
		m_LengthRight = lenS;
		m_PartAngle = angleS;
		m_PartHair = true;
		m_StyleLeft = styleF;
		m_StyleRight = styleS;
		m_WidAngLeft = angleF;
		m_WidAngRight = angleF;
	}

	inline void SetupParted(float partAngle, STYLE styleL, STYLE styleR, float lenL, float lenR, float headSpaceL,
			float headSpaceR, float hangL, float hangR, const AuPoint& combL, const AuPoint& combR, bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combL;
		m_CombRight = combR;
		m_HangLeft = hangL;
		m_HangRight = hangR;
		m_HeadSpaceLeft = headSpaceL;
		m_HeadSpaceRight = headSpaceR;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = lenL;
		m_LengthRight = lenR;
		m_PartAngle = partAngle;
		m_PartHair = true;
		m_StyleLeft = styleL;
		m_StyleRight = styleR;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}

	// Shaved areas do not create any hair lock polygons. They may make a base patch, which appears at 1/2 the 'headSpace' above
	// the skin.
	inline void SetupShaved(float headSpace, bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = AuPoint(0,0);
		m_CombRight = AuPoint(0,0);
		m_HangLeft = false;
		m_HangRight = false;
		m_HeadSpaceLeft = headSpace;
		m_HeadSpaceRight = headSpace;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = 1;				// MakeLock is happier if this is not zero, when the base patch is being drawn.
		m_LengthRight = 1;				// MakeLock is happier if this is not zero, when the base patch is being drawn.
		m_PartAngle = 0;
		m_PartHair = false;
		m_StyleLeft = Shaved;
		m_StyleRight = Shaved;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}

	// 'angleL' specifies the z angle of follicle normal where the transition from upper 'U'
	// parameters (left) to lower 'L' parameters (right) begins.
	// 'U' is above the angle, e.g. the patch part of the SoulPatch.
	// 'L' is below the angle, e.g. the bare chin part of the SoulPatch.
	inline void SetupSoulPatch(float angleL,
			float lenU, float lenL,
			float headSpaceU, float headSpaceL,
			float hangU, float hangL,
			const AuPoint& combU, const AuPoint& combL,
			bool basePatch)
	{
		m_Base = basePatch;
		m_CombLeft = combU;
		m_CombRight = combL;
		m_HangLeft = hangU;
		m_HangRight = hangL;
		m_HeadSpaceLeft = headSpaceU;
		m_HeadSpaceRight = headSpaceL;
		m_KnotLeft = 0;
		m_KnotRight = 0;
		m_LengthLeft = lenU;
		m_LengthRight = lenL;
		m_PartAngle = angleL;
		m_PartHair = true;
		m_StyleLeft = SoulPatch;
		m_StyleRight = SoulPatch;
		m_WidAngLeft = 0;
		m_WidAngRight = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuHairMesh : public QList<AuHairTri>
{
private:
	AuPoint								m_Max;
	AuPoint								m_Min;

public:	// Constructors
	AuHairMesh()
	{
	}

	AuHairMesh(const AuHairMesh& mesh) : QList<AuHairTri>(mesh)
	{
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
	}

public:
	const AuHairMesh& operator=(const AuHairMesh& mesh)
	{
		QList<AuHairTri>::operator=(mesh);
		m_Max = mesh.m_Max;
		m_Min = mesh.m_Min;
		return *this;
	}

public:	// Methods
	// 'sourceMesh' is the source of the current indexes in 't'.
	// 'combineVerts' can be true when sourceMesh is not this mesh, to use existing vertexes instead of adding duplicates.
	inline void Append(const AuHairTri& t)
	{
		ExpandExtents(t);
		append(t);
	}

	// 'combineVerts' can be true when sourceMesh is not this mesh, to use existing vertexes instead of adding duplicates.
	inline void Append(const AuHairMesh& sourceMesh)
	{
		for(const_iterator mi = sourceMesh.cbegin(); mi != sourceMesh.cend(); mi++)
			Append(*mi);
	}

	void Clear()
	{
		clear();
	}

	inline int Count() const
	{
		return size();
	}

	// Create VAO for use with the AnimalHair shader.
	AuGLVAO CreateAnimalHairVAO(AuOpenGLFunctions& re, bool isTop) const;

	AuGLVAO CreateHeadHairVAO(AuOpenGLFunctions& re, bool isTop) const;

	void ExpandExtents(const AuHairTri& t);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		min = m_Min;
		max = m_Max;
	}

	// 'sourceMesh' is the source of the vertex indexes in 't'.
	// 'combineVerts' can be true when sourceMesh is not this mesh, to use existing vertexes instead of adding duplicates.
	void Insert(const AuHairTri& t, const AuHairMesh& sourceMesh, bool combineVerts);

	// Creats a lock of hair emerging from 'follicle'.
	// 'style' is the style of the lock.
	// 'headSpace' is the margin distance between the lock of hair and the skin as it traverses the 'skull'.
	// 'lockWidth' and 'lockLength' specify the dimensions of the lock.
	// 'hangLength' limits the length of hair that hangs straight downward after it crests the head.
	// 'knotLoc' is the location on the skull to form the knot.
	// 'knotLength' is the distance out from the skull the hair goes at the 'knotLoc' before falling downward. This length is taken
	//   from the 'hangLength' so there will be no hanging hair when the two are equal.
	// 'widAng' is the sweep angle (in radians) of the knot on either side of the 'knotLoc'. e.g. 10 degrees is 20 degrees wide.
	//   For a mohawk, 'widAng' is the width of the strip on either side of the center line.
	// 'combDir' is a vector to push the hair. Its length is the strength of the comb vector.
	// 'basePatch' is true to make a patch of hair flat to the skin at the follicle location, to appear more hairy.
	// 'skull' provides the geometry of the head. It can be null to ignore conflicts.
	void MakeLock(const AuFolliclePoint& follicle, AuHairStyleLock::STYLE style, float headSpace, float lockWidth, float lockLength,
			float hangLength, const AuPoint& knotLoc, float knotLength, float widAng, const AuPoint& combDir, bool basePatch,
			const AuSolidEllipsoid* skull);

	inline const AuPoint& Max() const
	{
		return m_Max;
	}

	inline const AuPoint& Min() const
	{
		return m_Min;
	}

	// Removes all of this mesh with any vertex above 'z' altitude.
	void RemoveAbove(double z);

	// Removes all of this mesh with any vertex below 'z' altitude.
	void RemoveBelow(double z);

	// Assigns 'bone' to bone index 3, from a weight of 0 at 'fromX' to a weight of 1 at 'toX'. 'fromX' and 'toX' can be greater
	// or less than each other in either order.
	// 'bone' is which of the vertex bones to set, 0 to 3.
	// 'boneIndex' is the index of the bone matrix among all bone matrices.
	void AddBoneRangeBlendX(double fromX, double toX, int bone, quint8 boneIndex);
	void AddBoneRangeBlendXMouth(double fromX, double toX, int bone, quint8 boneIndex);

	// Sets all bones in this mesh. All bone weights must add up to 1.
	void SetBones(quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2,
			quint8 boneIndex3, float boneWeight3);

	// Sets all 'role' bones in this mesh. All bone weights must add up to 1.
	void SetBones(quint8 id, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1, quint8 boneIndex2,
			float boneWeight2, quint8 boneIndex3, float boneWeight3);

	// Assigns 'boneMin' and 'boneMax' to mesh vertices between minY and maxY, inclusive.
	// Bone weight is linearly interpolated between vertices. 'boneMin' is 1 at minY; 'boneMax' is 1 at maxY.
	void SetBonesRangeBlendY(double minY, double maxY, quint8 boneMin, quint8 boneMax);

	// Assigns 'boneMin' and 'boneMax' to mesh vertices between minZ and maxZ, inclusive.
	// Bone weight is linearly interpolated between vertices. 'boneMin' is 1 at minZ; 'boneMax' is 1 at maxZ.
	void SetBonesRangeBlendZ(double minZ, double maxZ, quint8 boneMin, quint8 boneMax);

	// Sets bones in this mesh whose follicle Z falls between minZ and maxZ. All bone weights must add up to 1.
	void SetBonesRangeZ(double minZ, double maxZ, quint8 boneIndex0, float boneWeight0, quint8 boneIndex1, float boneWeight1,
			quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3);
	void SetBonesRangeYZ(double minY, double maxY, double minZ, double maxZ, quint8 boneIndex0, float boneWeight0,
			quint8 boneIndex1, float boneWeight1, quint8 boneIndex2, float boneWeight2, quint8 boneIndex3, float boneWeight3);

	// Returns the X, Y and Z dimensions of this mesh.
	inline AuPoint Size() const
	{
		return m_Max - m_Min;
	}

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
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuHairStyle
{
public:
	enum STYLE
	{
		Bald,							// All follicles removed.
		Buzz,							// Every follicle as a stub.
		Close,							// Hair patch only at every follicle.
		Straight,						// Straight out of every follicle.
		Unkempt,						// Straight out, wavy, falling with gravity, to maximum hair length.

		CousinIt,
		Short,
		Baltar,
		Eowyn,
		Arwen,
		Elrond,
		Mandinka,						// Mr. T
		Mohawk,
		PonyBangs,
		PonyTail,
		Regulation,						// Parted on the right
		Waterfall,
		HighAndTight,
		Crop,
		Manager,
		FrTuck,
		Egghead,
		Spacely,
		Carrot,
		Crown,
		Mentat,
		Flattop,
		Bun,
		BunBangs,
		Bullet,
		Fallout,
		Curtain,
		Manchu,
		MohawkPony,
		STYLE_Count
	};

	enum SHAVE
	{
		Clean,							// All follicles removed.
		Stubble,						// Every follicle as a stub.
		CloseWhiskers,					// Hair patch only at every follicle.
		StraightWhiskers,				// Straight out of every follicle.
		Unshaven,						// Straight out, wavy, falling with gravity, to maximum hair length.

		ShapedStubble,
		Hollywoodian,
		MuttonChops,
		FrenchFork,
		FuManchu,
		OldDutch,
		VanDyke,
		ExtendedVanDyke,
		FriendlyMuttonChops,
		Balbo,
		Klingon,
		ShortBoxedBeard,
		FullBeard,
		Goatee,
		ExtendedGoatee,
		ChinStrap,
		ChinCurtain,
		Zappa,
		SoulPatch,
		Winnfield,
		PetitGoatee,
		FranzJosef,
		Anchor,
		Napoleon,
		Copstash,
		Gandalf,
		Confucius,
		Boris,
		Toothbrush,
		ChinPuff,
		Stripe,
		RapStandard,
		Pencil,
		Handlebar,
		HandlebarAndChinPuff,
		HandlebarAndGoatee,
		Wally,
		PencilPatch,
		SHAVE_Count
	};

private:
	AuHairStyleLock						m_Locks[AuFolliclePoint::ROLE_Count];

public:	// Construction
	inline AuHairStyle()
	{
	}

	inline AuHairStyle(const AuHairStyle& o)
	{
		for(int role=0; role<AuFolliclePoint::ROLE_Count; role++)
			m_Locks[role] = o.m_Locks[role];
	}

public:	// Operators

public:	// Methods
	// 'f' is an input parameter describing the hair follicle.
	// 'headSpace' is an input/output parameter which specifies the distance to maintain between hair and skull. On input, it is
	//   the minimum distance.
	// 'lockLen' is an output parameter specifying the maximum length of the lock of hair.
	// 'hangLength' is an output parameter specifying the length the lock of hair is cut to after crowning the skull and heading
	//   downward with gravity.
	// 'knotLength' is an output parameter specifying how much hair is used up in the knot at the back of the head, if any.
	// 'widAng' is an output parameter specifying the width angle for knots and mohawks.
	// 'combVec' is an output parameter specifying the direction to comb the lock until it crowns the head and falls.
	// 'basePatch' is an output parameter specifying if the lock should start with a base patch of hair to cover the skin there.
	AuHairStyleLock::STYLE Lock(const AuFolliclePoint& f, float& headSpace, float& lockLen, float& hangLen, float& knotLen,
			float& widAng, AuPoint& combVec, bool& basePatch) const;

	// Configures this hair style using canned styles.
	// 'isRightHanded' is true if right handed, false if left handed. This affects the part side for some hair styles.
	void Init(STYLE style, SHAVE shave, float headSpace, bool isRightHanded);

	inline static QString Name(STYLE style)
	{
		if(style < 0 || style > STYLE_Count)
			style = STYLE_Count;
		QString names[STYLE_Count+1] =
		{
			QObject::tr("Bald"),
			QObject::tr("Buzz"),
			QObject::tr("Close"),
			QObject::tr("Straight Out"),
			QObject::tr("Unkempt"),

			QObject::tr("Cousin It"),
			QObject::tr("Short"),
			QObject::tr("Baltar"),
			QObject::tr("Eowyn"),
			QObject::tr("Arwen"),
			QObject::tr("Elrond"),
			QObject::tr("Mandinka"),
			QObject::tr("Mohawk"),
			QObject::tr("Pony with Bangs"),
			QObject::tr("Pony Tail"),
			QObject::tr("Regulation"),
			QObject::tr("Waterfall"),
			QObject::tr("High and Tight"),
			QObject::tr("Crop"),
			QObject::tr("Manager"),
			QObject::tr("Friar Tuck"),
			QObject::tr("Egghead"),
			QObject::tr("Spacely"),
			QObject::tr("Carrot"),
			QObject::tr("Crown"),
			QObject::tr("Mentat"),
			QObject::tr("Flattop"),
			QObject::tr("Bun"),
			QObject::tr("Bun with Bangs"),
			QObject::tr("Bullet"),
			QObject::tr("Fallout"),
			QObject::tr("Curtain"),
			QObject::tr("Manchu"),
			QObject::tr("Mohawk Tail"),
			QObject::tr("Error Bad Hair Style")
		};
		return names[style];
	}

	inline static QString ShaveName(SHAVE style)
	{
		if(style < 0 || style > SHAVE_Count)
			style = SHAVE_Count;
		QString names[SHAVE_Count+1] =
		{
			QObject::tr("Clean Shaven"),
			QObject::tr("Stubble"),
			QObject::tr("Close"),
			QObject::tr("Straight Out"),
			QObject::tr("Unshaven"),

			QObject::tr("Stubble Shaped"),
			QObject::tr("Hollywoodian"),
			QObject::tr("Mutton Chops"),
			QObject::tr("French Fork"),
			QObject::tr("Fu Manchu"),
			QObject::tr("Old Dutch"),
			QObject::tr("Van Dyke"),
			QObject::tr("Van Dyke Extended"),
			QObject::tr("Friendly Mutton Chops"),
			QObject::tr("Balbo"),
			QObject::tr("Klingon"),
			QObject::tr("Short Boxed Beard"),
			QObject::tr("Full Beard"),
			QObject::tr("Goatee"),
			QObject::tr("Goatee Extended"),
			QObject::tr("Chin Strap"),
			QObject::tr("Chin Curtain"),
			QObject::tr("Zappa"),
			QObject::tr("Soul Patch"),
			QObject::tr("Winnfield"),
			QObject::tr("Petit Goatee"),
			QObject::tr("Franz Josef"),
			QObject::tr("Anchor"),
			QObject::tr("Napoleon III Imperial"),
			QObject::tr("Copstash Standard"),
			QObject::tr("Gandalf"),
			QObject::tr("Confucius"),
			QObject::tr("Boris"),
			QObject::tr("Toothbrush"),
			QObject::tr("Chin Puff"),
			QObject::tr("Stripe"),
			QObject::tr("Stubble Rap"),
			QObject::tr("Pencil"),
			QObject::tr("Handlebar"),
			QObject::tr("Handlebar and Chin Puff"),
			QObject::tr("Handlebar and Goatee"),
			QObject::tr("Wally"),
			QObject::tr("Pencil Patch"),
			QObject::tr("Error Bad Shave Style")
		};
		return names[style];
	}

protected:
	void StyleFacialHair(AuHairStyleLock::STYLE style, float moustacheLen, float chinLen, float chinSidesLen, float chopLen,
			float jowlLen, float cheekLen, float neckLen, float headSpace, const AuPoint& combVec);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuHair_H
