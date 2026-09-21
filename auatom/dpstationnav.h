//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationnav.h,v 1.15 2022/04/14 16:46:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpStationNav_H
#define DpStationNav_H

#include "auatom.h"
#include "dewaypoint.h"
#include "dpstation.h"

class DaSolarSystem;
class DaStar;

class AUATOM_DLL DpStationNav : public DpStation  
{
public:
	enum CONTROLINPUT
	{
		CI_Invalid = 0,

		CI_NeighborsToggle,
		CI_RingsToggle,
		CI_WormholeToggle,
		CI_WaypointsToggle,
		CI_ZoomSector0,
		CI_ZoomSector1,
		CI_ZoomSector2,
		CI_ZoomSector3,
		CI_ZoomSector4,
		CI_ZoomSector5,
		CI_ZoomSolarSystem0,
		CI_ZoomSolarSystem1,
		CI_ZoomSolarSystem2,
		CI_ZoomSolarSystem3,
		CI_ZoomSolarSystem4,
		CI_ZoomSolarSystem5,
		CI_ZoomIn,
		CI_ZoomOut,
		CI_TopViewCenter,
		CI_PanToSelection,
		CI_PanToShip,
		CI_PanDown,
		CI_PanLeft,
		CI_PanLeftDown,
		CI_PanLeftUp,
		CI_PanRight,
		CI_PanRightDown,
		CI_PanRightUp,
		CI_PanUp,
		CI_FrontView,
		CI_LeftView,
		CI_RightView,
		CI_BackView,
		
		CI_AddRouteToSelected,
		CI_AddWaypointAtSelected,
		CI_AddWaypoint,
		CI_InsertWaypoint,
		CI_DeleteWaypointAtSelected,
		CI_DeleteAllWaypoints,
		CI_DisplayScreen,
		CI_NamesToggle,
	};

	enum SELECTION
	{
		SEL_Invalid=-1,
		SEL_None,				// Nothing is selected.
		SEL_SectorAtom,			// An atom at sector scale is selected.
		SEL_SolarSystemAtom,	// An atom at solar system scale is selected.
		SEL_Waypoint,			// A wormhole is selected (only at solar system scale).
	};

	enum WAYPOINTS
	{
		WP_Off,
		WP_Waypoints,
		WP_Routes,
		WP_WaypointsAndRoutes,
	};

	enum ZOOMSETTING
	{
		Z_Invalid = -1,
		Z_System0,
		Z_System1,
		Z_System2,
		Z_System3,
		Z_System4,
		Z_System5,
		Z_Sector0,
		Z_Sector1,
		Z_Sector2,
		Z_Sector3,
		Z_Sector4,
		Z_Sector5,
		Z_Levels		// Last entry for number of zoom levels.
	};

private:
	bool			m_bMoonRingsVisible;	// true if the orbit rings of moons are on
	bool			m_bNeighborSectors;		// true to show neighboring sectors.
	bool			m_bRingsVisible;		// true if the orbit rings are on
	bool			m_bTrackShip;			// true to follow ship.
	bool			m_bWormholesVisible;	// true if the wormhole lines are on
	CONTROLINPUT	m_eCurrentCommand;		// For controls that need point input from the chart.
	WAYPOINTS		m_eWaypoints;			// Visibility state of waypoints and routes.
	AuPoint			m_fPan;					// Pan offset from center. (only X and Y are serialized)
	AuPoint			m_fRot;					// Rotation around center.
	AuId			m_idSystem;				// Solar system selected for display
	char			m_iNamesVisible;		// 0=no names, 1=names systems, 2=all systems
	ZOOMSETTING		m_Zoom;					// Display zoom level.

	// Selection
	SELECTION		m_eSelection;			// Current selection type.
	AuId			m_idSelected;			// Selected atom (Not valid when selection is SEL_Waypoint).
	quint8			m_lSelectedWaypoint;	// Index of selected waypoint.
	AuPoint			m_SelectedOffset;		// Offset for wormholes.

private:
	mutable QReadWriteLock				m_WaypointLock;
	DeWaypointRoute						m_Waypoints;			// Route of waypoints.

private:
	// Values used for device animation...NOT serialized.
	char			m_bLastNeighborSectors;
	char			m_bLastRings;
	char			m_bLastWormholes;
	bool			m_DisplayChanged;
	CONTROLINPUT	m_eLastCommand;			// For controls that need point input from the chart.
	WAYPOINTS		m_eLastWaypoints;		// Last Visibility state of waypoints and routes.
	SELECTION		m_eLastSel;
	char			m_iLastNames;
	ZOOMSETTING		m_LastZoom;
	int				m_lLastWaypointCount;

protected:
	// OpenGL
	GLuint								m_ScreenTex;			// OpenGL texture id for display screen.

public:
	DpStationNav();
	virtual ~DpStationNav();
	DpStationNav(const DpStationNav &c);

public:
	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
			const AuPoint& location, const DaCreature& sceneAtom);

	virtual void EnumControlInputs(DiControlInputArray &aAllControls) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding);

	virtual void DeleteDeviceObjects(AuGLEffects& re);

	void LoadControls() const;

	virtual quint64 MemoryUsage() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaBd& bd,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson* pSceneAtom, DaVessel& ship,
			CONTROL_CONTEXT cc, const AuPoint& location);

	virtual void Refresh(const DpStation* pCon, const DaAtom& parent);

	// 'sceneAtom' is the atom for whom the scene is being constructed.
	// 'seatedCreature' is the creature seated at this console, if any; may be 0.
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
			const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

	virtual bool Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person);

	virtual void Rx(QDataStream& in, DaCapsule& capsule);
	virtual void RxSettings(QDataStream& in);
	virtual void Tx(QDataStream& out) const;
	virtual void TxSettings(QDataStream& out) const;
														
	virtual void LoadClientSettings(const AuId& planetId);
	virtual void StoreClientSettings(const AuId& planetId) const;
	virtual STATION_TYPE ClientRestoreType() const;

public:
	virtual AuId DisplayedSystem() const
	{
		return m_idSystem;
	}

	inline bool IsAllNamesVisible() const
	{
		return (m_iNamesVisible == 2) ? true : false;
	}

	virtual bool IsMoonRingsVisible() const
	{
		return m_bMoonRingsVisible;
	}

	virtual bool IsRingsVisible() const
	{
		return m_bRingsVisible;
	}

	inline bool IsSystemNamesVisible() const
	{
		return (m_iNamesVisible == 1) ? true : false;
	}

	virtual bool IsWormholesVisible() const
	{
		return m_bWormholesVisible;
	}

	virtual int SystemNamesVisible() const
	{
		return m_iNamesVisible;
	}

protected:
	void AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& capsule, const AuElapsedTimer& last);
	static GLuint CreateDisplayTexture(AuGLEffects& re, const AuId& parentId);
	void DrawSolarSystemDisplay(QPainter& p, const QSize &sz, const DaAtom& ref, quint32 refEmpire);
	void DrawSectorDisplay(QPainter& p, const QSize &sz, const DaAtom& ref, quint32 refEmpire);
	void DrawStarAndPlanets(QPainter& p, const QPoint &pCenter, DaStar *pstar, const DaSolarSystem *pss, int iPixelsPerOrbit,
			bool bOrbit[6]);
	void AddWaypoint(const QPoint &pxHit, DaVessel& ship);
	AuPoint ConvertDisplayPixelToLoc(const QPoint &pxHit) const;
	const DaAtom *FindClosestDisplayAtom(const QPoint &pxHit, double &rDisplayDistance, AuPoint& vOffset, const DaAtom& ref) const;
	const DaAtom *FindClosestSectorDisplayAtom(const QPoint &pxHit, double &rDisplayDistance, const DaAtom& ref) const;
	const DaAtom *FindClosestSolarSystemDisplayAtom(const QPoint &pxHit, double &rDisplayDistance, AuPoint& vOffset,
			const DaAtom& ref) const;
	const DaAtom *FindClosestStarOrPlanet(const QPoint &pCenter, DaStar *pstar, const DaSolarSystem *pss, int iPixelsPerOrbit,
			const QPoint &pxHit, double &rDisplayDistance, const DaAtom *pBest) const;
	int FindClosestDisplayWaypoint(const QPoint &pxHit, double &rDisplayDistance, const DaAtom& ref) const;
	void InsertWaypoint(const QPoint &pxHit, DaVessel& ship);
	virtual void PanelDimensions(double& width, double& height) const = 0;
	void PanToSelection(DaAtom& ref);
	void ProcessDisplayHit(const QPoint &pxHit, DaBd& bd);
	void ProcessDisplayHit(const QPoint &pxHit, DaVessel& ship);
	void SelectNearest(const QPoint &pxHit, DaAtom& ref);
};

#endif // DpStationNav_H
