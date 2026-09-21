//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorstarmap.h,v 1.21 2022/04/08 16:41:44 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLocatorStarMap_H
#define AuLocatorStarMap_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "aumatrix4x4.h"
#include "austarmapdata.h"
#include "daatom.h"
#include <QStandardItemModel>
#include <QWidget>

class AuStarMapGL;
class AuStarMapPlaces;
class AuStarMapViewerData;
class AuStarMapWidget;
class DaSector;
class DgEmpire;
class AuIconButton;
class AuStarMapSearch;
class AuStarMapSearchSystems;
class QCheckBox;
class QItemSelection;
class QSpinBox;
class QSplitter;

class AUCLIENT_DLL AuLocatorStarMap : public QWidget
{
	Q_OBJECT;
private:
	QStandardItemModel					m_PlacesModel;			// Places model
	QHash<AuId, GALAXY>					m_SectorsPresent;		// Key is sector id, value is GALAXY.

	// Highlighting Filter
	QList<quint32>						m_EmpiresIgnored;
	bool								m_PlacesValid;
	bool								m_ShowDetectedPlaces;
	bool								m_ShowEmpirePlaces;
	bool								m_ShowEnemyPlaces;
	bool								m_ShowExploredPlaces;
	bool								m_ShowFriendPlaces;
	bool								m_ShowNeutralPlaces;
	bool								m_ShowUnknownPlaces;
	bool								m_UnknownPrecedesEnemy;

	// Widgets
	AuIconButton*						m_LookAtLastSelected;
	AuIconButton*						m_LookAtSelection;
	AuIconButton*						m_ClearSelection;
	AuIconButton*						m_FilterExplored;
	AuIconButton*						m_FilterRange;
	AuIconButton*						m_FilterSelection;
	AuStarMapPlaces*					m_PlacesView;			// Places view
	QSpinBox*							m_RangeSpin;
	AuIconButton*						m_RequestData;
	AuIconButton*						m_SaveStarMap;
	AuIconButton*						m_Search;
	AuStarMapSearch*					m_SearchDlg;
	AuIconButton*						m_SearchSystems;
	AuStarMapSearchSystems*				m_SearchSystemsDlg;
	AuIconButton*						m_Settings;
	AuIconButton*						m_ShowGalaxy;
	AuIconButton*						m_ShowScanned;
	AuIconButton*						m_ShowShips;
	AuIconButton*						m_ShowSignals;
	AuIconButton*						m_ShowStations;
	AuIconButton*						m_ShowWormholes;
	QSplitter*							m_Splitter;
	AuStarMapGL*						m_StarMapGL;							// GL version is used in full game.
	AuStarMapWidget*					m_StarMapW;								// Widget version is used in commander.

public:
	AuLocatorStarMap(QWidget* parent=0);
	virtual ~AuLocatorStarMap();

	virtual QSize sizeHint() const;

	void StarMapErased();

	void StarMapSectorAdded(const AuId& sectorId);

	void StarMapShowCities(const AuIdList& cityIds);

	void StarMapShowGalaxyLocation(const QString& galaxyName, const AuPoint& loc);

	void StarMapUpdateAllComplete();

	void StarMapUpdateGalaxyComplete(GALAXY galaxy);

	void StarMapUpdateSectorComplete(GALAXY galaxy, const AuIdList& sectorIds);

	void StarMapUpdateAllStarted();

	void StarMapUpdateGalaxyStarted(GALAXY galaxy);

	void StarMapUpdateSectorStarted(const AuId& sector, unsigned int range);

signals:
	void StarMapAllReceived();
	void StarMapAllRequested();
	void StarMapGalaxyReceived(int galaxy);
	void StarMapGalaxyRequested(int galaxy);
	void StarMapSectorReceived(int galaxy, AuIdList sectorIds);
	void StarMapSectorRequested(AuId sector, unsigned int range);

public slots:
	void ClearSelection();
	void DomainAdd(AuId systemId, QString systemName);
	void EmpireDataChanged();
	void PlaceActivated(const AuId& id);
	void PlaceActivated(const QModelIndex& sel);
	void PlaceClicked(const AuId& id);
	void PlacesSelected(const QItemSelection& selected, const QItemSelection& deselected);
	void QuerySector(const AuId& sectorId);
	void RangeChanged(int range);
	void ReportCityHistory(const AuId& id);
	void ReportCityLast(const AuId& id);
	void RequestData();
	void RestoreSettings();
	void SaveSettings();
	void SaveStarMapXml();
	void ScanSystem();
	void ScanTactical();
	void Search();
	void SearchPlace(AuId id);
	void SearchSystems();
	// 'galaxy' is a GALAXY enum.
	void SearchSystems(int galaxy, const AuPoint& galaxyLoc);
	void SetRange(int range);
	void SplitterMoved(int pos, int index);
	void StarMapViewerSettings();
	void ToggleFilterExplored();
	void ToggleFilterRange();
	void ToggleFilterSelection();
	void ToggleShowGalaxy();
	void ToggleShowScanned();
	void ToggleShowShips();
	void ToggleShowSignals();
	void ToggleShowStations();
	void ToggleShowWormholes();

protected:
	bool ActivatePlaceRecursive(const AuId& id, const QModelIndex& parentIdx);
	// 'signalStrength' is the strength of the detected emission from systems that are only emitting, not explored. 0-15
	QModelIndex AddPlace(const QModelIndex& parentIdx, const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon,
			const QString& name, const QColor& textColor, const AuPoint& galaxyLoc, const QString& status, const AuId& sectorId,
			GALAXY galaxy, bool isCoreBlackHole, bool isEmptySystem, quint8 systemGenHab, quint32 systemGenSeed,
			bool systemExplored, bool systemScanned,
			quint32 cityEmpire, quint8 signalStrength,
			quint16 empireShips=0, quint16 empireStations=0,	// only for systems
			quint16 friendShips=0, quint16 friendStations=0,	// only for systems
			quint16 neutralShips=0, quint16 neutralStations=0,	// only for systems
			quint16 enemyShips=0, quint16 enemyStations=0);		// only for systems
	void ClearGalaxy(GALAXY galaxy);
	void ClearSectors(GALAXY galaxy, const AuIdList& sectorIds);
	virtual void hideEvent(QHideEvent* event);
	void HighlightPlaces();
	void HighlightSector(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI, const QModelIndex& sectIdx,
			const DgEmpire& personEmpire);
	void InitPlaces();
	void ListPlaces();
	void PostQuery(int what, GALAXY galaxy, const AuId& sectorId, quint16 range);
	void SelectPlace(const AuId& id);
	void SelectPlaces(const AuIdList& ids);
	bool SelectPlaceRecursive(const AuId& id, const QModelIndex& parentIdx, QItemSelectionModel::SelectionFlag cmd);
	void SelectPlacesTree(const AuIdList& ids);
	void SelectPlaceTree(const AuId& id);
	bool SelectPlaceTreeRecursive(const AuId& id, const QModelIndex& parentIdx, QItemSelectionModel::SelectionFlag cmd);
	virtual void showEvent(QShowEvent* event);
	void SpacecraftPresent(const AuStarMapSystem& sys, quint32 empireId, quint16& empireShips, quint16& empireStations,
			quint16& friendShips, quint16& friendStations, quint16& neutralShips, quint16& neutralStations, quint16& enemyShips,
			quint16& enemyStations);
	void UpdatePlaceFlags();
	AuIdList ListExpanded() const;
	void ListExpandedRecursive(AuIdList& expanded, const QModelIndex& parentIdx) const;
	void ExpandPlaces(const AuIdList& ids) const;
	bool ExpandPlaceRecursive(const AuId& id, const QModelIndex& parentIdx) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorStarMapDock : public AuDockWidget
{
public:
	AuLocatorStarMap*						m_Content;

public:
	AuLocatorStarMapDock(QWidget* parent);

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorStarMapAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuLocatorStarMap_H
