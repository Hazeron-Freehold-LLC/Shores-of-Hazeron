//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatompos.h,v 1.4 2022/02/19 17:30:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAtomPos_H
#define AuAtomPos_H

#include "auatom.h"
#include "auid.h"
#include "aumatrix4x4.h"

class QDebug;

class AuAtomPos
{
	AuId								m_ParentId;				// Universally unique identifier of an atom's parent atom.
	AuMatrix4x4							m_Transform;			// Transformation matrix of an atom relative to its parent atom.

public:	// Construction
	inline AuAtomPos(const AuId& parentId, const AuMatrix4x4& m) : m_ParentId(parentId), m_Transform(m)
	{
	}

	inline AuAtomPos(const AuId& parentId, const AuPoint& location, const AuPoint& rotation) : m_ParentId(parentId)
	{
		m_Transform.LoadIdentity();
		m_Transform.Translate(location);
		m_Transform.Rotate(rotation);
	}

	inline AuAtomPos(const AuId& parentId, double locx, double locy, double locz, double rotx, double roty, double rotz) :
		m_ParentId(parentId)
	{
		m_Transform.LoadIdentity();
		m_Transform.Translate(locx,locy,locz);
		m_Transform.Rotate(AuPoint(rotx,roty,rotz));
	}

	inline AuAtomPos()
	{
	}

	inline AuAtomPos(const AuAtomPos& c) : m_ParentId(c.m_ParentId), m_Transform(c.m_Transform)
	{
	}

public:	// Operators
	inline const AuAtomPos& operator=(const AuAtomPos& c)
	{
		m_ParentId = c.m_ParentId;
		m_Transform = c.m_Transform;
		return *this;
	}

	inline bool operator==(const AuAtomPos& c)
	{
		if(m_ParentId != c.m_ParentId)
			return false;
		if(m_Transform != c.m_Transform)
			return false;
		return true;
	}

	inline bool operator!=(const AuAtomPos& c)
	{
		return !operator==(c);
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	inline const AuId& ParentId() const
	{
		return m_ParentId;
	}

	inline AuPoint ProjectVector(const AuPoint& v) const
	{
		return m_Transform.ProjectVector(v);
	}

	inline void RotateAxis(double ang, const AuPoint& vec)
	{
		m_Transform.RotateAxis(ang, vec);
	}

	inline void RotateX(double ang)
	{
		m_Transform.RotateX(ang);
	}

	inline void RotateY(double ang)
	{
		m_Transform.RotateY(ang);
	}

	inline void RotateZ(double ang)
	{
		m_Transform.RotateZ(ang);
	}

	inline AuPoint Rotation() const
	{
		return m_Transform.GetRotation();
	}

	void Rx(QDataStream& in);

	inline void SetTransform(const AuMatrix4x4& t)
	{
		m_Transform = t;
	}

	inline const AuMatrix4x4& Transform() const
	{
		return m_Transform;
	}

	inline void TransformBy(const AuMatrix4x4& t)
	{
		m_Transform *= t;
	}

	inline void Translate(const AuPoint& v)
	{
		m_Transform.Translate(v);
	}

	inline void Translate(double x, double y, double z)
	{
		m_Transform.Translate(x, y, z);
	}

	inline AuPoint Translation() const
	{
		return m_Transform.Translation();
	}

	void Tx(QDataStream& out) const;

	inline AuPoint UnProjectVector(const AuPoint& v) const
	{
		return m_Transform.UnProjectVector(v);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuAtomPos& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuAtomPos& o)
{
	o << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuAtomPos& o);

#endif // AuAtomPos_H
