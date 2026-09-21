//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesignplane.h,v 1.69 2022/05/17 16:04:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaDesignPlane_H
#define DaDesignPlane_H

#include "daatom.h"

class AUATOM_DLL DaDesignPlane : public DaAtom  
{
public:
	enum GRIDBITS
	{
		GRIDBIT_Horizontal	= 0x01,	// 0 degree horizontal lines
		GRIDBIT_Diagonal45	= 0x02,	// 45 degree diagonal lines
		GRIDBIT_Vertical	= 0x04,	// 90 degree vertical lines
		GRIDBIT_Diagonal135	= 0x08,	// 135 degree diagonal lines
	};

	bool			m_GravityOn;	// true for gravity effects to be enabled.
	double			m_GridInc;		// Grid increment.
	quint8			m_GridIncNoM;	// Grid medium bright increments per brightest line increment.
	quint8			m_GridIncOm;	// Grid increments per medium bright line increment.
	quint8			m_GridOnBits;	// Bits of grid lines that are "on".
	double			m_GridRad;		// Grid radius.
	AuId			m_OwnerId;		// person atom who owns this design plane
	bool			m_RenderSolid;	// true for shaded render, false for lines
	quint8			m_RoomMode;		// 0=draw rooms, 1=change existing room type
	quint8			m_RoomId;		// Id to use for next room added by owner.

public:
	DaDesignPlane(const AuId& owner);
	DaDesignPlane();
	virtual ~DaDesignPlane();

protected:
	DaDesignPlane(const DaDesignPlane &rhs);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	virtual DaAtom *Clone() const;

	inline double GridRadius() const
	{
		return m_GridRad;
	}

	inline double GridSpacing() const
	{
		return m_GridInc;
	}

	virtual bool IsEthereal() const
	{
		return true;
	}

	virtual bool IsTicker() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const			// Report the name of this Atom for listing purposes.
	{
		return("Design Plane");
	}

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	inline void SetGridRadius(double r)
	{
		m_GridRad = r;
	}

	// Visible grid is depicted as a dim grid of lines at the grid increment spacing. At some interval the lines are brighter.
	// At a larger interval the lines are brightest.
	// 's' is the grid increment.
	// 'gridOm' is the number of grid increments for each medium bright line on the visible grid.
	// 'gridNoM' is the number of medium bright increments per brightest increment.
	inline void SetGridSpacing(double s, quint8 gridOm, quint8 gridNoM)
	{
		m_GridInc = s;
		m_GridIncOm = gridOm;
		m_GridIncNoM = gridNoM;
	}

	virtual ATOM_TYPE AtomType() const;

	inline void ToggleRoomMode()
	{
		if(m_RoomMode)
			m_RoomMode = 0;
		else
			m_RoomMode = 1;
	}

public:
	// User interface functions.
	void DecrementRoomId();

	inline void GravityToggle()
	{
		m_GravityOn = !m_GravityOn;
	}

	inline bool IsGravityOn() const
	{
		return m_GravityOn;
	}

	void IncrementRoomId();

	void MoveDown();

	void MoveUp();

	inline const AuId& Owner() const
	{
		return m_OwnerId;
	}

	inline void RenderModeToggle()
	{
		m_RenderSolid = !m_RenderSolid;
	}

	inline bool IsRenderSolid() const
	{
		return m_RenderSolid;
	}

	inline quint8 RoomId() const
	{
		return m_RoomId;
	}

	// Returns current room command mode: 0=draw, 1=change room type
	inline quint8 RoomMode() const
	{
		return m_RoomMode;
	}

	void EnableHorizontalLines();
	void EnableDiagonal45Lines();
	void EnableVerticalLines();
	void EnableDiagonal135Lines();
	void DisableHorizontalLines();
	void DisableDiagonal45Lines();
	void DisableVerticalLines();
	void DisableDiagonal135Lines();
	inline short int GridOnBits() const
	{
		return m_GridOnBits;
	}
	inline bool DoesGridFormBoundaries() const
	{
		// No grid lines or only one grid line cannot form boundaries.
		if(m_GridOnBits == 0 ||
				m_GridOnBits == GRIDBIT_Horizontal ||
				m_GridOnBits == GRIDBIT_Vertical ||
				m_GridOnBits == GRIDBIT_Diagonal45 ||
				m_GridOnBits == GRIDBIT_Diagonal135)
			return false;
		else
			return true;	// All other combinations form boundaries.
	}
	inline bool IsHorizontalEnabled() const
	{
		return (m_GridOnBits&GRIDBIT_Horizontal) ? true : false;
	}
	inline bool IsDiagonal45Enabled() const
	{
		return (m_GridOnBits&GRIDBIT_Diagonal45) ? true : false;
	}
	inline bool IsVerticalEnabled() const
	{
		return (m_GridOnBits&GRIDBIT_Vertical) ? true : false;
	}
	inline bool IsDiagonal135Enabled() const
	{
		return (m_GridOnBits&GRIDBIT_Diagonal135) ? true : false;
	}

public:
	static DaDesignPlane* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

private:
	void RenderBdGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
			float textBrightness, const QFont& font, BUILDING_STRUCTURE bdStruct);
	void RenderItGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
			float textBrightness, const QFont& font, COMMODITYID cid);
	void RenderScGrid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, QOpenGLWidget* glw,
			float textBrightness, const QFont& font);
};

#endif // DaDesignPlane_H
