//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapwidget.h,v 1.15 2022/02/19 18:01:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapWidget_H
#define AuStarMapWidget_H

#include "auclientlib.h"
#include "auglsl.h"
#include "auhash.h"
#include "aumatrix4x4.h"
#include "aupoint.h"
#include "daatom.h"
#include <QIcon>
#include <QOpenGLWidget>
#include <QWidget>

using namespace std;
#include <unordered_map>

class AuAtomRenderInfo;
class AuGLModelCache;
class AuGLModelMaker;
class QKeyEvent;
class QTimer;

class AUCLIENT_DLL AuStarMapWidgetPlace
{
public:
	DaAtom::ATOM_TYPE					m_AtomType;				// Type, as returned by DaAtom::Type().
	quint32								m_CityEmpire;			// Empire id of city, if atom is a city.
	QColor								m_Color;				// Color
	bool								m_EmptySystem;			// True for solar system that has no sun or worlds.
	quint8								m_Galaxy;				// GALAXY enum value.
	QIcon								m_Icon;					// Icon
	AuPoint								m_Loc;					// Relative to the galaxy.
	QString								m_Name;					// Place name
	AuId								m_SectorId;				// Sector that contains this place. Null if this place IS a sector.
	bool								m_Selected;				// True if selected.
	quint8								m_SignalStrength;		// Signal strength detected from a system, 0-15
	bool								m_SystemCoreBlackHole;	// Only for solar systems.
	bool								m_SystemExplored;		// True if this is an explored system. Only for solar systems.
	quint8								m_SystemGenHab;			// Only for solar systems.
	quint32								m_SystemGenSeed;		// Only for solar systems.
	bool								m_SystemScanned;		// True if this is a scanned system. Only for solar systems.
	QList<AuPoint>						m_WormDestEx;			// Destination location of explored wormholes.
	QList<AuPoint>						m_WormDestUnex;			// Destination location of unexplored wormholes.

	quint16								m_EmpireShips;
	quint16								m_EmpireStations;
	quint16								m_FriendShips;
	quint16								m_FriendStations;
	quint16								m_NeutralShips;
	quint16								m_NeutralStations;
	quint16								m_EnemyShips;
	quint16								m_EnemyStations;

public:
	inline AuStarMapWidgetPlace(DaAtom::ATOM_TYPE type, const QIcon& icon, const QString& name, const QColor& color,
			const AuPoint& loc, const AuId& sectorId, GALAXY galaxy, bool systemCoreBlackHole, bool isEmptySystem,
			quint8 systemGenHab, quint32 systemGenSeed, bool systemExplored, bool systemScanned, quint32 cityEmpire,
			quint8 signalStrength,
			quint16 empireShips, quint16 empireStations,
			quint16 friendShips, quint16 friendStations,
			quint16 neutralShips, quint16 neutralStations,
			quint16 enemyShips, quint16 enemyStations)
	{
		m_AtomType = type;
		m_CityEmpire = cityEmpire;
		m_Color = color;
		m_EmptySystem = isEmptySystem;
		m_Galaxy = galaxy;
		m_Icon = icon;
		m_Loc = loc;
		m_Name = name;
		m_SectorId = sectorId;
		m_Selected = false;
		m_SignalStrength = signalStrength;
		m_SystemCoreBlackHole = systemCoreBlackHole;
		m_SystemExplored = systemExplored;
		m_SystemGenHab = systemGenHab;
		m_SystemGenSeed = systemGenSeed;
		m_SystemScanned = systemScanned;

		m_EmpireShips = empireShips;
		m_EmpireStations = empireStations;
		m_FriendShips = friendShips;
		m_FriendStations = friendStations;
		m_NeutralShips = neutralShips;
		m_NeutralStations = neutralStations;
		m_EnemyShips = enemyShips;
		m_EnemyStations = enemyStations;
	}

	inline AuStarMapWidgetPlace(const AuStarMapWidgetPlace& c) : m_WormDestEx(c.m_WormDestEx), m_WormDestUnex(c.m_WormDestUnex)
	{
		m_AtomType = c.m_AtomType;
		m_CityEmpire = c.m_CityEmpire;
		m_Color = c.m_Color;
		m_EmptySystem = c.m_EmptySystem;
		m_Galaxy = c.m_Galaxy;
		m_Icon = c.m_Icon;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_SectorId = c.m_SectorId;
		m_Selected = c.m_Selected;
		m_SignalStrength = c.m_SignalStrength;
		m_SystemCoreBlackHole = c.m_SystemCoreBlackHole;
		m_SystemExplored = c.m_SystemExplored;
		m_SystemGenHab = c.m_SystemGenHab;
		m_SystemGenSeed = c.m_SystemGenSeed;
		m_SystemScanned = c.m_SystemScanned;

		m_EmpireShips = c.m_EmpireShips;
		m_EmpireStations = c.m_EmpireStations;
		m_FriendShips = c.m_FriendShips;
		m_FriendStations = c.m_FriendStations;
		m_NeutralShips = c.m_NeutralShips;
		m_NeutralStations = c.m_NeutralStations;
		m_EnemyShips = c.m_EnemyShips;
		m_EnemyStations = c.m_EnemyStations;
	}

public:	// Operators
	inline const AuStarMapWidgetPlace& operator=(const AuStarMapWidgetPlace& c)
	{
		m_AtomType = c.m_AtomType;
		m_CityEmpire = c.m_CityEmpire;
		m_Color = c.m_Color;
		m_EmptySystem = c.m_EmptySystem;
		m_Galaxy = c.m_Galaxy;
		m_Icon = c.m_Icon;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_SectorId = c.m_SectorId;
		m_Selected = c.m_Selected;
		m_SignalStrength = c.m_SignalStrength;
		m_SystemCoreBlackHole = c.m_SystemCoreBlackHole;
		m_SystemExplored = c.m_SystemExplored;
		m_SystemGenHab = c.m_SystemGenHab;
		m_SystemGenSeed = c.m_SystemGenSeed;
		m_SystemScanned = c.m_SystemScanned;

		m_EmpireShips = c.m_EmpireShips;
		m_EmpireStations = c.m_EmpireStations;
		m_FriendShips = c.m_FriendShips;
		m_FriendStations = c.m_FriendStations;
		m_NeutralShips = c.m_NeutralShips;
		m_NeutralStations = c.m_NeutralStations;
		m_EnemyShips = c.m_EnemyShips;
		m_EnemyStations = c.m_EnemyStations;

		m_WormDestEx = c.m_WormDestEx;
		m_WormDestUnex = c.m_WormDestUnex;
		return *this;
	}

public:	// Methods
	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline bool HasShip() const
	{
		return m_EmpireShips || m_FriendShips || m_NeutralShips || m_EnemyShips;
	}

	inline bool HasStation() const
	{
		return m_EmpireStations || m_FriendStations || m_NeutralStations || m_EnemyStations;
	}

	inline bool IsEmptySystem() const
	{
		return m_EmptySystem;
	}

	inline bool IsScanned() const
	{
		return m_SystemScanned;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuStarMapWidget : public QWidget
{
	Q_OBJECT;
private:
	QImage								m_Background;
	int									m_CountSelected;		// count of selected places
	QCursor								m_Cursor;				// Crosshair cursor
	int									m_Density;				// Density of galaxy stars.
	bool								m_FilterExplored;		// true to only display systems that are explored
	bool								m_FilterRange;			// true to only display systems that are withing m_Range
	bool								m_FilterSelection;		// true to only display places that are selected
	GALAXY								m_Galaxy;				// current galaxy shown
	bool								m_InGalaxy;				// True if scene atom is in m_Galaxy.
	AuId								m_LastSelected;			// Id of last selected place.
	int									m_LeftButtonMode;		// 0=none, 1=rotate, 2=zoom
	double								m_LookDist;				// Distance back from look point to place camera.
	AuPoint								m_LookPoint;			// Focus of our camera.
	AuPoint								m_LookRot;				// Rotation of our camera.
	QRect								m_Magnifier;
	float								m_MagnifierScale;		// Scale when magnifier started being dragged.
	QPoint								m_MouseDownPt;
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>	m_Places;				// Key is atom id.
	AuId								m_Preview;				// Atom that is closest to the moving cursor, for preview.
	quint8								m_PreviewCityEmpire;
	quint8								m_PreviewGalaxy;
	quint8								m_PreviewGenHab;
	quint32								m_PreviewGenSeed;
	bool								m_PreviewIsCoreBlackHole;
	bool								m_PreviewIsEmptySystem;
	bool								m_PreviewIsExplored;
	bool								m_PreviewIsScanned;
	AuPoint								m_PreviewLoc;			// Galaxy coordinate of preview atom
	QString								m_PreviewName;			// Atom name that is closest to the moving cursor, for orders.
	AuId								m_PreviewSector;		// Sector that contains m_Preview. Is m_Preview if a sector.
	bool								m_PreviewHasSpacecraft;	// true if spacecraft info is available at a solar system
	DaAtom::ATOM_TYPE					m_PreviewType;			// AtomType of preview atom.
	int									m_Range;				// Range in sectors
	AuPoint								m_SceneAtomLoc;			// Last known loc relative to parent m_Galaxy, in pc.
	AuPoint								m_SceneAtomSectorLoc;	// Last known Location() of scene atom's sector.
	AuIdList							m_SectorIds;			// Sectors in range when m_Range is not zero.
	bool								m_ShowGalaxy;
	bool								m_ShowScanned;
	bool								m_ShowSignals;
	bool								m_ShowShips;
	bool								m_ShowStations;
	bool								m_ShowWormholes;
	QMap<AuPoint, QColor>				m_SpotLocs;				// Spot locations for galaxy display.
	AuId								m_Target;				// Atom that is at the mouse press location.
	quint8								m_TargetCityEmpire;
	quint8								m_TargetGalaxy;
	quint8								m_TargetGenHab;
	quint32								m_TargetGenSeed;
	bool								m_TargetIsCoreBlackHole;
	bool								m_TargetIsEmptySystem;
	bool								m_TargetIsExplored;
	bool								m_TargetIsScanned;
	AuPoint								m_TargetLoc;			// Galaxy coordinate of target atom
	QString								m_TargetName;			// Target atom name.
	AuId								m_TargetSector;			// Sector that contains m_Target. Is m_Target if a sector.
	bool								m_TargetHasSpacecraft;	// true if spacecraft info is available at a solar system
	DaAtom::ATOM_TYPE					m_TargetType;			// AtomType of preview atom.
	QTimer*								m_timer;

public:
	AuStarMapWidget(QWidget* parent=0);
	virtual ~AuStarMapWidget();

	// 'loc' is location relative to the galaxy, in parsecs.
	// 'signalStrength' is the strength of the detected emission from systems that are only emitting, not explored. 0-15
	void AddPlace(const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon, const QString& name, const QColor& color,
			const AuPoint& loc, const AuId& sectorId, GALAXY galaxy, bool systemCoreBlackHole, bool isEmptySystem,
			quint32 systemGenHab, quint32 systemGenSeed, bool systemExplored, bool systemScanned, quint32 cityEmpire,
			quint8 signalStrength,
			quint16 empireShips, quint16 empireStations,
			quint16 friendShips, quint16 friendStations,
			quint16 neutralShips, quint16 neutralStations,
			quint16 enemyShips, quint16 enemyStations);

	void AddPlaceWormhole(const AuId& id, const AuPoint& wormVec, bool isExplored);

	void ClearGalaxy(GALAXY galaxy);
	void ClearSectors(const AuIdList& sectorIds);
	void ClearUniverse();

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline bool IsGalaxyVisible() const
	{
		return m_ShowGalaxy;
	}

	inline bool IsShipsVisible() const
	{
		return m_ShowShips;
	}

	inline bool IsExploredFiltered() const
	{
		return m_FilterExplored;
	}

	inline bool IsRangeFiltered() const
	{
		return m_FilterRange;
	}

	inline bool IsScannedVisible() const
	{
		return m_ShowScanned;
	}

	inline bool IsSelectionFiltered() const
	{
		return m_FilterSelection;
	}

	inline bool IsSignalsVisible() const
	{
		return m_ShowSignals;
	}

	inline bool IsStationsVisible() const
	{
		return m_ShowStations;
	}

	inline bool IsWormholesVisible() const
	{
		return m_ShowWormholes;
	}

	AuIdList SelectedPlaces() const;

	inline void SetLastSelected(const AuId& id)
	{
		m_LastSelected = id;
	}

	void SetPlaceColor(const AuId& id, const QColor& color);

	void SetPlaceIcon(const AuId& id, const QIcon& icon);

	void SetLookPoint(const AuPoint& look);

signals:
	void DomainAdd(AuId systemId, QString systemName);
	void GoDirectToCoord(AuPoint galaxyLoc);
	void GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc);
	void GoViaWormholes(AuId systemId, QString systemName);
	void LandAtCity(AuId cityId, QString cityName);
	void OrbitCity(AuId cityId, QString cityName);
	void PlaceClicked(const AuId& id);
	void PlaceDoubleClicked(const AuId& id);
	void ReportCityHistory(const AuId& cityId);
	void ReportCityLast(const AuId& cityId);
	void ReportSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName);
	// 'scanEx' 2=scanned, 1=explored, 0=observed
	void ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
			quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx);
	void ScanSystem();
	void ScanTactical();
	// 'galaxy' is a GALAXY enum. 'galaxyLoc' is relative to 'galaxy', in pc.
	void SystemSearchAt(int galaxy, const AuPoint& galaxyLoc);

public slots:
	void LookAtLastSelected();
	void LookAtSelection();
	void ClearSelection();
	void DeselectPlace(const AuId& id);
	void FilterExplored(bool filterExplored);
	void FilterRange(bool filterRange);
	void FilterSelection(bool filterSelection);
	void LookAtPlace(const AuId& id);
	void HideGalaxy();
	void HideScanned();
	void HideShips();
	void HideSignals();
	void HideStations();
	void HideWormholes();
	void RestoreSettings();
	void SelectPlace(const AuId& id);
	void SetDensity(int density);
	void SetRange(int range);
	void ShowGalaxy();
	void ShowScanned();
	void ShowShips();
	void ShowSignals();
	void ShowStations();
	void ShowWormholes();

protected:
	void GetMVP(AuMatrix4x4& mvp);
	void GetMVP(AuMatrix4x4& mv, AuMatrix4x4& mvp);
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	void PaintBackground(const AuMatrix4x4& mvp, const AuMatrix4x4& mv);
	void PaintLine(QPainter& p, const AuMatrix4x4& mvp, const AuMatrix4x4& mv, const AuPoint& p0, const AuPoint& p1, int segments);
	virtual void paintEvent(QPaintEvent* event);
	virtual void showEvent(QShowEvent* event);
	virtual void wheelEvent(QWheelEvent* e);

protected slots:
	void InvalidateBackground();
	void PreviewClosestPlace(const QPoint& pos);
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStarMapGL : public QOpenGLWidget
{
	Q_OBJECT;
private:
	QFont								m_FontBig;
	QFont								m_FontMedium;
	QFont								m_FontSmall;
	AuGLModelCache*						m_ModelCache;
	Qt::MouseButtons					m_MouseButtons;			// Buttons during last mouse press event.
	QPoint								m_MouseLast;
	AuGLModelMaker*						m_MM;					// Model maker.
	AuAtomRenderInfo*					m_RenderInfo;			// Values needed by atoms when rendering them.

	// Things to render.
	AuGLVAO								m_AxesVAO;
	AuGLVAO								m_GalaxyVAO;
	GLuint								m_GalaxySpriteTex;
	GLuint								m_MyLoc1Tex;
	GLuint								m_MyLoc2Tex;
	GLuint								m_ScannedSpriteTex;
	GLuint								m_ShipTex[4];
	GLuint								m_SignalTex[16];
	GLuint								m_StarSpriteTex;
	GLuint								m_StationTex[4];
	GLuint								m_SystemSpriteTex;

public:	// Constructors
	AuStarMapGL(QWidget *parent=0);

	~AuStarMapGL();

public:
	virtual void initializeGL();

	virtual void paintGL();

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);
	virtual void mousePressEvent(QMouseEvent* e);
	virtual void wheelEvent(QWheelEvent* e);

	// From AuStarMapWidget
private:
	int									m_CountSelected;		// count of selected places
	QCursor								m_Cursor;				// Crosshair cursor
	bool								m_FilterExplored;		// true to only display systems that are explored
	bool								m_FilterRange;			// true to only display systems that are withing m_Range
	bool								m_FilterSelection;		// true to only display places that are selected
	GALAXY								m_Galaxy;				// current galaxy shown
	bool								m_InGalaxy;				// True if scene atom is in m_Galaxy.
	AuId								m_LastSelected;			// Id of last selected place.
	int									m_LeftButtonMode;		// 0=none, 1=rotate, 2=zoom
	double								m_LookDist;				// Distance back from look point to place camera.
	AuPoint								m_LookPoint;			// Focus of our camera.
	AuPoint								m_LookRot;				// Rotation of our camera.
	QRect								m_Magnifier;
	float								m_MagnifierScale;		// Scale when magnifier started being dragged.
	QPoint								m_MouseDownPt;
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>	m_Places;				// Key is atom id.
	AuId								m_Preview;				// Atom that is closest to the moving cursor, for preview.
	quint8								m_PreviewCityEmpire;
	quint8								m_PreviewGalaxy;
	quint8								m_PreviewGenHab;
	quint32								m_PreviewGenSeed;
	bool								m_PreviewIsCoreBlackHole;
	bool								m_PreviewIsEmptySystem;
	bool								m_PreviewIsExplored;
	bool								m_PreviewIsScanned;
	AuPoint								m_PreviewLoc;			// Galaxy coordinate of preview atom
	QString								m_PreviewName;			// Atom name that is closest to the moving cursor, for orders.
	AuId								m_PreviewSector;		// Sector that contains m_Preview. Is m_Preview if a sector.
	bool								m_PreviewHasSpacecraft;	// true if spacecraft info is available at a solar system
	quint8								m_PreviewType;			// AtomType of preview atom.
	int									m_Range;				// Range in sectors
	AuPoint								m_SceneAtomLoc;			// Last known loc relative to parent m_Galaxy, in pc.
	AuPoint								m_SceneAtomSectorLoc;	// Last known Location() of scene atom's sector.
	AuIdList							m_SectorIds;			// Sectors in range when m_Range is not zero.
	bool								m_ShowGalaxy;
	bool								m_ShowScanned;
	bool								m_ShowSignals;
	bool								m_ShowShips;
	bool								m_ShowStations;
	bool								m_ShowWormholes;
	QList<AuScPoint>					m_SpotLocs;				// Spot locations, size and color for galaxy display.
	AuId								m_Target;				// Atom that is at the mouse press location.
	quint8								m_TargetCityEmpire;
	quint8								m_TargetGalaxy;
	quint8								m_TargetGenHab;
	quint32								m_TargetGenSeed;
	bool								m_TargetIsCoreBlackHole;
	bool								m_TargetIsEmptySystem;
	bool								m_TargetIsExplored;
	bool								m_TargetIsScanned;
	AuPoint								m_TargetLoc;			// Galaxy coordinate of target atom
	QString								m_TargetName;			// Target atom name.
	AuId								m_TargetSector;			// Sector that contains m_Target. Is m_Target if a sector.
	bool								m_TargetHasSpacecraft;	// true if spacecraft info is available at a solar system
	quint8								m_TargetType;			// AtomType of preview atom.

public:
	//AuStarMapWidget(QWidget* parent=0);
	//virtual ~AuStarMapWidget();

	// 'loc' is location relative to the galaxy, in parsecs.
	// 'signalStrength' is the strength of the detected emission from systems that are only emitting, not explored. 0-15
	void AddPlace(const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon, const QString& name, const QColor& color,
			const AuPoint& loc, const AuId& sectorId, GALAXY galaxy, bool systemCoreBlackHole, bool isEmptySystem,
			quint32 systemGenHab, quint32 systemGenSeed, bool systemExplored, bool systemScanned, quint32 cityEmpire,
			quint8 signalStrength,
			quint16 empireShips, quint16 empireStations,
			quint16 friendShips, quint16 friendStations,
			quint16 neutralShips, quint16 neutralStations,
			quint16 enemyShips, quint16 enemyStations);

	void AddPlaceWormhole(const AuId& id, const AuPoint& wormVec, bool isExplored);

	void ClearGalaxy(GALAXY galaxy);
	void ClearSectors(const AuIdList& sectorIds);
	void ClearUniverse();

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline bool IsGalaxyVisible() const
	{
		return m_ShowGalaxy;
	}

	inline bool IsShipsVisible() const
	{
		return m_ShowShips;
	}

	inline bool IsExploredFiltered() const
	{
		return m_FilterExplored;
	}

	inline bool IsRangeFiltered() const
	{
		return m_FilterRange;
	}

	inline bool IsScannedVisible() const
	{
		return m_ShowScanned;
	}

	inline bool IsSelectionFiltered() const
	{
		return m_FilterSelection;
	}

	inline bool IsSignalsVisible() const
	{
		return m_ShowSignals;
	}

	inline bool IsStationsVisible() const
	{
		return m_ShowStations;
	}

	inline bool IsWormholesVisible() const
	{
		return m_ShowWormholes;
	}

	AuIdList SelectedPlaces() const;

	inline void SetLastSelected(const AuId& id)
	{
		m_LastSelected = id;
	}

	void SetPlaceColor(const AuId& id, const QColor& color);

	void SetPlaceIcon(const AuId& id, const QIcon& icon);

	void SetLookPoint(const AuPoint& look);

signals:
	void DomainAdd(AuId systemId, QString systemName);
	void GoDirectToCoord(AuPoint galaxyLoc);
	void GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc);
	void GoViaWormholes(AuId systemId, QString systemName);
	void LandAtCity(AuId cityId, QString cityName);
	void OrbitCity(AuId cityId, QString cityName);
	void PlaceClicked(const AuId& id);
	void PlaceDoubleClicked(const AuId& id);
	void ReportCityHistory(const AuId& cityId);
	void ReportCityLast(const AuId& cityId);
	void ReportSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName);
	// 'scanEx' 2=scanned, 1=explored, 0=observed
	void ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
			quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx);
	void ScanSystem();
	void ScanTactical();
	// 'galaxy' is a GALAXY enum. 'galaxyLoc' is relative to 'galaxy', in pc.
	void SystemSearchAt(int galaxy, const AuPoint& galaxyLoc);

public slots:
	void LookAtLastSelected();
	void LookAtSelection();
	void ClearSelection();
	void DeselectPlace(const AuId& id);
	void FilterExplored(bool filterExplored);
	void FilterRange(bool filterRange);
	void FilterSelection(bool filterSelection);
	void LookAtPlace(const AuId& id);
	void HideGalaxy();
	void HideScanned();
	void HideShips();
	void HideSignals();
	void HideStations();
	void HideWormholes();
	void RestoreSettings();
	void SelectPlace(const AuId& id);
	void SetRange(int range);
	void ShowGalaxy();
	void ShowScanned();
	void ShowShips();
	void ShowSignals();
	void ShowStations();
	void ShowWormholes();

protected:
	void GetMVP(AuMatrix4x4& mvp);
	void GetMVP(AuMatrix4x4& mv, AuMatrix4x4& mvp);
	void GetMVP(AuMatrix4x4& p, AuMatrix4x4& mv, AuMatrix4x4& mvp);	// This one does not invert Y, for OpenGL rendering.
	virtual void keyPressEvent(QKeyEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent* e);

protected slots:
	void PreviewClosestPlace(const QPoint& pos);
};

#endif // AuStarMapWidget_H
