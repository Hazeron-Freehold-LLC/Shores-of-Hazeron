//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausynccontroller.h,v 1.5 2022/04/21 21:23:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSyncController_H
#define AuSyncController_H

#include "auatompos.h"
#include "auid.h"
#include "aumatrix4x4.h"
#include "aupose.h"

class QDataStream;
class QDebug;

class AuSyncController
{
public:
	enum TYPE
	{
		SYNC_AtomPos,
		SYNC_Vector,
		SYNC_Motion,
		SYNC_AtomPosVec,
		SYNC_Pose,
	};

public:
	inline AuSyncController()
	{
	}

	inline AuSyncController(const AuSyncController& c)
	{
		Q_UNUSED(c);
	}

	virtual ~AuSyncController()
	{
	}

public:
	virtual void operator>>(QDataStream& out) const = 0;
	virtual void operator<<(QDataStream& in) = 0;

public:
	virtual bool AtRest() const
	{
		return false;
	}

	virtual AuSyncController* Clone() const = 0;

	static AuSyncController* Create(TYPE type);

	virtual float HeadTilt() const
	{
		return 0;
	}

	// Return true if this sync is replaced by 'c'.
	virtual bool IsReplacedBy(const AuSyncController& c) const = 0;

	virtual AuAtomPos Pos() const
	{
		return AuAtomPos();
	}

	virtual AuPose Pose() const
	{
		return AuPose();
	}

	virtual quint8 SeatIndex() const
	{
		return 0;
	}

	virtual quint16 StationId() const
	{
		return 0;
	}

	virtual TYPE Type() const = 0;

	virtual QString TypeName() const = 0;

	virtual const AuPoint& Vector() const
	{
		return AuPoint::Invalid();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSyncController& d)
{
	d >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSyncController& d)
{
	d << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuSyncController& d);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncControllerList : public QList<AuSyncController*>
{
public:
	inline AuSyncControllerList()
	{
	}

	inline AuSyncControllerList(const AuSyncControllerList& c) : QList<AuSyncController*>()
	{
		for(const_iterator di = c.cbegin(); di != c.cend(); di++)
			append((*di)->Clone());
	}

	inline ~AuSyncControllerList()
	{
		Clear();
	}

public:
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	// Appends demands read from a data stream.
	void Append(QDataStream& in);

	void Append(const AuSyncControllerList& list);

	// Adds 'd' after removing any prior syncs that it replaces. Uses IsReplacedBy test.
	void AppendReplace(const AuSyncController& d);

	inline void Clear()
	{
		while(!isEmpty())
			delete takeFirst();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSyncControllerList& d)
{
	d >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSyncControllerList& d)
{
	d << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuSyncControllerList& d);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncAtomPos : public AuSyncController
{
	AuAtomPos							m_Pos;

public:
	inline AuSyncAtomPos(const AuAtomPos& pos) : m_Pos(pos)
	{
	}

	inline AuSyncAtomPos()
	{
	}

	inline AuSyncAtomPos(const AuSyncAtomPos& c) : AuSyncController(c), m_Pos(c.m_Pos)
	{
	}

public:
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual AuSyncController* Clone() const
	{
		return new AuSyncAtomPos(*this);
	}

	virtual bool IsReplacedBy(const AuSyncController& c) const;

	virtual AuAtomPos Pos() const
	{
		return m_Pos;
	}

	virtual TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("AtomPos");
	}

	inline static TYPE TypeStatic()
	{
		return SYNC_AtomPos;
	}
};
 
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncVector : public AuSyncController
{
	AuPoint								m_Vector;

public:
	inline AuSyncVector(const AuPoint& v) : m_Vector(v)
	{
	}

	inline AuSyncVector()
	{
	}

	inline AuSyncVector(const AuSyncVector& c) : AuSyncController(c), m_Vector(c.m_Vector)
	{
	}

public:
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual AuSyncController* Clone() const
	{
		return new AuSyncVector(*this);
	}

	virtual bool IsReplacedBy(const AuSyncController& c) const;

	virtual TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Vector");
	}

	inline static TYPE TypeStatic()
	{
		return SYNC_Vector;
	}

	virtual const AuPoint& Vector() const
	{
		return m_Vector;
	}
};
 
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncMotion : public AuSyncController
{
public:
	inline AuSyncMotion()
	{
	}

	inline AuSyncMotion(const AuSyncMotion& c) : AuSyncController(c)
	{
	}

public:
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual AuSyncController* Clone() const
	{
		return new AuSyncMotion(*this);
	}

	virtual bool IsReplacedBy(const AuSyncController& c) const;

	virtual TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Motion");
	}

	inline static TYPE TypeStatic()
	{
		return SYNC_Motion;
	}
};
 
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncAtomPosVec : public AuSyncAtomPos
{
	bool								m_AtRest;
	AuPoint								m_Vector;

public:
	inline AuSyncAtomPosVec(const AuAtomPos& pos, bool atRest, const AuPoint& vector) : AuSyncAtomPos(pos), m_Vector(vector)
	{
		m_AtRest = atRest;
	}

	inline AuSyncAtomPosVec()
	{
		m_AtRest = false;
	}

	inline AuSyncAtomPosVec(const AuSyncAtomPosVec& c) : AuSyncAtomPos(c), m_Vector(c.m_Vector)
	{
		m_AtRest = c.m_AtRest;
	}

public:
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual bool AtRest() const
	{
		return m_AtRest;
	}

	virtual AuSyncController* Clone() const
	{
		return new AuSyncAtomPosVec(*this);
	}

	virtual bool IsReplacedBy(const AuSyncController& c) const;

	virtual TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("AtomPos w/Vector");
	}

	inline static TYPE TypeStatic()
	{
		return SYNC_AtomPosVec;
	}

	virtual const AuPoint& Vector() const
	{
		return m_Vector;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSyncPose : public AuSyncController
{
	float								m_HeadTilt;
	AuPose								m_Pose;
	quint8								m_SeatIndex;
	quint16								m_StationId;

public:
	inline AuSyncPose(const AuPose& v, float headTilt, quint16 stationId, quint8 seatIndex) : m_Pose(v)
	{
		m_HeadTilt = headTilt;
		m_SeatIndex = seatIndex;
		m_StationId = stationId;
	}

	inline AuSyncPose()
	{
		m_HeadTilt = 0;
		m_SeatIndex = 0;
		m_StationId = 0;
	}

	inline AuSyncPose(const AuSyncPose& c) : AuSyncController(c), m_Pose(c.m_Pose)
	{
		m_HeadTilt = c.m_HeadTilt;
		m_SeatIndex = c.m_SeatIndex;
		m_StationId = c.m_StationId;
	}

public:
	virtual void operator>>(QDataStream& out) const;
	virtual void operator<<(QDataStream& in);

public:
	virtual AuSyncController* Clone() const
	{
		return new AuSyncPose(*this);
	}

	virtual float HeadTilt() const
	{
		return m_HeadTilt;
	}

	virtual bool IsReplacedBy(const AuSyncController& c) const;

	virtual AuPose Pose() const
	{
		return m_Pose;
	}

	virtual quint8 SeatIndex() const
	{
		return m_SeatIndex;
	}

	virtual quint16 StationId() const
	{
		return m_StationId;
	}

	virtual TYPE Type() const
	{
		return TypeStatic();
	}

	virtual QString TypeName() const
	{
		return QObject::tr("Pose");
	}

	inline static TYPE TypeStatic()
	{
		return SYNC_Pose;
	}
};
 
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuSyncController_H
