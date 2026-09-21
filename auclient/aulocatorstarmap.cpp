//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatorstarmap.cpp,v 1.58 2022/04/08 19:02:24 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulocatorstarmap.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "austarmaphighlightdialog.h"
#include "austarmapplaces.h"
#include "austarmapquery.h"
#include "austarmapsearch.h"
#include "austarmapsearchsystems.h"
#include "austarmapwidget.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dgdata.h"
#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QSpinBox>
#include <QSplitter>
#include <QTcpSocket>
#include <QVBoxLayout>

static QColor	s_AvatarColor(32,255,32);
static QColor	s_BlackColor(0,0,0);
static QColor	s_DetectedColor(128,227,255);
static QColor	s_ExColor(0,100,100);
static QColor	s_GalaxyColor(255,255,255);
static QColor	s_UnexColor(100,60,80);
static QColor	s_UnkColor(255,0,255);

AuLocatorStarMapDock::AuLocatorStarMapDock(QWidget* parent) :
	AuDockWidget(":data/RangeGalaxy.png", tr("<b>Star Map</b>"), tr("Star Map"), parent)
{
	m_Content = new AuLocatorStarMap(this);
	setWidget(m_Content);

	if(auApp->IsHazeronCommander())
	{
		// Hazeron Commander uses a QPainter drawn starmap. It docks ok.
		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorStarMapAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}
	else
		setAllowedAreas(Qt::NoDockWidgetArea);		// No docking for this widget. It breaks the OpenGL render.
}

void AuLocatorStarMapDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorStarMapUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocatorStarMap::AuLocatorStarMap(QWidget* parent) : QWidget(parent)
{
	AuFrame* frame = auApp->Frame();
	int range = g_Au.Settings().readNumEntry("Widget/StarMap/Range", 1);

	m_PlacesValid = false;
	m_SearchDlg = 0;
	m_SearchSystemsDlg = 0;
	m_ShowDetectedPlaces = true;
	m_ShowEmpirePlaces = true;
	m_ShowEnemyPlaces = true;
	m_ShowExploredPlaces = true;
	m_ShowFriendPlaces = true;
	m_ShowNeutralPlaces = true;
	m_ShowUnknownPlaces = true;
	m_UnknownPrecedesEnemy = true;

	if(!auApp->IsHazeronCommander())
	{
		// OpenGL star map in full version.
		m_StarMapW = 0;
		m_StarMapGL = new AuStarMapGL(this);
		m_StarMapGL->setMinimumWidth(100);
		connect(m_StarMapGL, SIGNAL(DomainAdd(AuId, QString)), SLOT(DomainAdd(AuId, QString)));
		connect(m_StarMapGL, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)),
				auApp, SLOT(GoDirectToSystem(AuId, QString, AuPoint)));
		connect(m_StarMapGL, SIGNAL(GoDirectToCoord(AuPoint)), auApp, SLOT(GoDirectToCoord(AuPoint)));
		connect(m_StarMapGL, SIGNAL(GoViaWormholes(AuId, QString)), auApp, SLOT(GoViaWormholes(AuId, QString)));
		connect(m_StarMapGL, SIGNAL(ScanSystem()), SLOT(ScanSystem()));
		connect(m_StarMapGL, SIGNAL(ScanTactical()), SLOT(ScanTactical()));
		connect(m_StarMapGL, SIGNAL(SystemSearchAt(int, const AuPoint&)), SLOT(SearchSystems(int, const AuPoint&)));
		connect(m_StarMapGL, SIGNAL(LandAtCity(AuId, QString)), auApp, SLOT(GoLandAtCity(AuId, QString)));
		connect(m_StarMapGL, SIGNAL(OrbitCity(AuId, QString)), auApp, SLOT(GoOrbitCity(AuId, QString)));
		connect(m_StarMapGL, SIGNAL(PlaceClicked(const AuId&)), SLOT(PlaceClicked(const AuId&)));
		connect(m_StarMapGL, SIGNAL(PlaceDoubleClicked(const AuId&)), SLOT(PlaceActivated(const AuId&)));
		connect(m_StarMapGL, SIGNAL(ReportCityLast(AuId)), SLOT(ReportCityLast(AuId)));
		connect(m_StarMapGL, SIGNAL(ReportCityHistory(AuId)), SLOT(ReportCityHistory(AuId)));
		connect(m_StarMapGL, SIGNAL(ReportSpacecraft(quint32, AuId, AuId, QString)),
				frame, SLOT(ReportSystemSpacecraft(quint32, AuId, AuId, QString)));
		connect(m_StarMapGL, SIGNAL(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)),
				frame, SLOT(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)));
	}
	else
	{
		// Widget star map in commander version.
		m_StarMapGL = 0;
		m_StarMapW = new AuStarMapWidget(this);
		m_StarMapW->setMinimumWidth(100);
		connect(m_StarMapW, SIGNAL(DomainAdd(AuId, QString)), SLOT(DomainAdd(AuId, QString)));
		connect(m_StarMapW, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)),
				auApp, SLOT(GoDirectToSystem(AuId, QString, AuPoint)));
		connect(m_StarMapW, SIGNAL(GoDirectToCoord(AuPoint)), auApp, SLOT(GoDirectToCoord(AuPoint)));
		connect(m_StarMapW, SIGNAL(GoViaWormholes(AuId, QString)), auApp, SLOT(GoViaWormholes(AuId, QString)));
		connect(m_StarMapW, SIGNAL(ScanSystem()), SLOT(ScanSystem()));
		connect(m_StarMapW, SIGNAL(ScanTactical()), SLOT(ScanTactical()));
		connect(m_StarMapW, SIGNAL(SystemSearchAt(int, const AuPoint&)), SLOT(SearchSystems(int, const AuPoint&)));
		connect(m_StarMapW, SIGNAL(LandAtCity(AuId, QString)), auApp, SLOT(GoLandAtCity(AuId, QString)));
		connect(m_StarMapW, SIGNAL(OrbitCity(AuId, QString)), auApp, SLOT(GoOrbitCity(AuId, QString)));
		connect(m_StarMapW, SIGNAL(PlaceClicked(const AuId&)), SLOT(PlaceClicked(const AuId&)));
		connect(m_StarMapW, SIGNAL(PlaceDoubleClicked(const AuId&)), SLOT(PlaceActivated(const AuId&)));
		connect(m_StarMapW, SIGNAL(ReportCityLast(AuId)), SLOT(ReportCityLast(AuId)));
		connect(m_StarMapW, SIGNAL(ReportCityHistory(AuId)), SLOT(ReportCityHistory(AuId)));
		connect(m_StarMapW, SIGNAL(ReportSpacecraft(quint32, AuId, AuId, QString)),
				frame, SLOT(ReportSystemSpacecraft(quint32, AuId, AuId, QString)));
		connect(m_StarMapW, SIGNAL(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)),
				frame, SLOT(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)));
	}

	m_PlacesModel.insertColumns(0,3);
	m_PlacesModel.setHeaderData(0, Qt::Horizontal, "Place");
	m_PlacesModel.setHeaderData(1, Qt::Horizontal, "Location");
	m_PlacesModel.setHeaderData(2, Qt::Horizontal, "Status");

	m_PlacesView = new AuStarMapPlaces;
	m_PlacesView->setAllColumnsShowFocus(true);
	m_PlacesView->setModel(&m_PlacesModel);
	m_PlacesView->setSelectionMode(QAbstractItemView::MultiSelection);
	connect(m_PlacesView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(PlaceActivated(const QModelIndex&)));
	connect(m_PlacesView, SIGNAL(DomainAdd(AuId, QString)), SLOT(DomainAdd(AuId, QString)));
	connect(m_PlacesView, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)),
			auApp, SLOT(GoDirectToSystem(AuId, QString, AuPoint)));
	connect(m_PlacesView, SIGNAL(GoDirectToCoord(AuPoint)), auApp, SLOT(GoDirectToCoord(AuPoint)));
	connect(m_PlacesView, SIGNAL(GoViaWormholes(AuId, QString)), auApp, SLOT(GoViaWormholes(AuId, QString)));
	connect(m_PlacesView, SIGNAL(ScanSystem()), SLOT(ScanSystem()));
	connect(m_PlacesView, SIGNAL(ScanTactical()), SLOT(ScanTactical()));
	connect(m_PlacesView, SIGNAL(LandAtCity(AuId, QString)), auApp, SLOT(GoLandAtCity(AuId, QString)));
	connect(m_PlacesView, SIGNAL(OrbitCity(AuId, QString)), auApp, SLOT(GoOrbitCity(AuId, QString)));
	connect(m_PlacesView, SIGNAL(PlaceClicked(const AuId&)), SLOT(PlaceClicked(const AuId&)));
	connect(m_PlacesView, SIGNAL(PlaceDoubleClicked(const AuId&)), SLOT(PlaceActivated(const AuId&)));
	connect(m_PlacesView, SIGNAL(QuerySector(const AuId&)), SLOT(QuerySector(const AuId&)));
	connect(m_PlacesView, SIGNAL(ReportCityLast(AuId)), SLOT(ReportCityLast(AuId)));
	connect(m_PlacesView, SIGNAL(ReportCityHistory(AuId)), SLOT(ReportCityHistory(AuId)));
	connect(m_PlacesView, SIGNAL(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)),
			frame, SLOT(ReportSystemResources(AuId, QString, quint8, bool, quint8, quint32, AuPoint, float, int)));
	connect(m_PlacesView, SIGNAL(SelectionChanged(const QItemSelection&, const QItemSelection&)),
			SLOT(PlacesSelected(const QItemSelection&, const QItemSelection&)));

	m_ShowGalaxy = new AuIconButton(":data/RangeGalaxy.png", this);
	m_ShowGalaxy->setCheckable(true);
	m_ShowGalaxy->setToolTip(tr("<html><b>Galaxy</b><br>"
				"Toggles display of galaxy.</html>"));
	connect(m_ShowGalaxy, SIGNAL(clicked()), SLOT(ToggleShowGalaxy()));

	m_ShowWormholes = new AuIconButton(":data/ac_WormholesOn.bmp", this);
	m_ShowWormholes->setCheckable(true);
	m_ShowWormholes->setToolTip(tr("<html><b>Wormholes</b><br>"
				"Toggles display of wormhole lines.</html>"));
	connect(m_ShowWormholes, SIGNAL(clicked()), SLOT(ToggleShowWormholes()));

	m_ShowStations = new AuIconButton(":data/SpaceStation.png", this);
	m_ShowStations->setCheckable(true);
	m_ShowStations->setToolTip(tr("<html><b>Space Stations</b><br>"
				"Toggles display of space stations.</html>"));
	connect(m_ShowStations, SIGNAL(clicked()), SLOT(ToggleShowStations()));

	m_ShowShips = new AuIconButton(":data/Spacecraft.png", this);
	m_ShowShips->setCheckable(true);
	m_ShowShips->setToolTip(tr("<html><b>Space Ships</b><br>"
				"Toggles display of space ships.</html>"));
	connect(m_ShowShips, SIGNAL(clicked()), SLOT(ToggleShowShips()));

	m_ShowScanned = new AuIconButton(":data/SpotScanned.png", this);
	m_ShowScanned->setCheckable(true);
	m_ShowScanned->setToolTip(tr("<html><b>Surveyed</b><br>"
				"Solar systems will show a marker if they have been survey scanned by your empire. "
				"<br>"
				"%1 Right click a scanned system to request a survey report. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_ShowScanned, SIGNAL(clicked()), SLOT(ToggleShowScanned()));

	m_ShowSignals = new AuIconButton(":data/SignalStrength.png", this);
	m_ShowSignals->setCheckable(true);
	m_ShowSignals->setToolTip(tr("<html><b>Warp Signature Bars</b><br>"
				"Solar systems will show the strength of the warp signatures detected from there, if any. "
				"Warp signatures are detected by radars at cities and military bases of your empire, "
				"if your empire has warp technology, and by spacecraft of your empire, if they are equipped with sensors and warp "
				"drive.<br>"
				"%1 Solar systems emit a signal hourly, that is a measure of the warp drives engaged in that system."
				"<br>"
				"%1 Each airport terminal and military air base, belonging to an empire with warp technology, "
				"accounts for one warp drive engaged in that hour, at the highest warp factor available to the empire."
				"<br>"
				"%1 Cities in other solar systems detect this signal when in range."
				"<br>"
				"%1 Detection range is based on the strength of the signal, about one sector per warp factor engaged, "
				"and the <i>warp signature detection range</i> of the detecting city or spacecraft. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_ShowSignals, SIGNAL(clicked()), SLOT(ToggleShowSignals()));

	m_FilterSelection = new AuIconButton(":data/Sweep.png", this);
	m_FilterSelection->setCheckable(true);
	m_FilterSelection->setToolTip(tr("<html><b>Selection Filter</b><br>"
				"Toggles display of selected places only.<br>"
				"%1 When not filtered, all places are shown in the star map.<br>"
				"%1 When filtered, only selected places are shown in the star map.</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_FilterSelection, SIGNAL(clicked()), SLOT(ToggleFilterSelection()));

	m_FilterRange = new AuIconButton(":data/RangeSector.png", this);
	m_FilterRange->setCheckable(true);
	m_FilterRange->setToolTip(tr("<html><b>Range Filter</b><br>"
				"Toggles display of sectors that are within range.<br>"
				"%1 When not filtered, all sectors are shown in the star map.<br>"
				"%1 When filtered, only sectors within range are shown in the star map.</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_FilterRange, SIGNAL(clicked()), SLOT(ToggleFilterRange()));

	m_FilterExplored = new AuIconButton(":data/c_Spacecraft.png", this);
	m_FilterExplored->setCheckable(true);
	m_FilterExplored->setToolTip(tr("<html><b>Explored Systems Filter</b><br>"
				"Toggles display of explored systems only.<br>"
				"%1 When not filtered, all solar systems are shown in the star map.<br>"
				"%1 When filtered, only explored solar systems are shown in the star map.</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_FilterExplored, SIGNAL(clicked()), SLOT(ToggleFilterExplored()));

	m_ClearSelection = new AuIconButton(":data/PostureHoldFire.png", this);
	m_ClearSelection->setToolTip(tr("<html><b>Clear Selection</b><br>"
				"Deselects all selected places."
				"</html>"));
	connect(m_ClearSelection, SIGNAL(clicked()), SLOT(ClearSelection()));

	m_LookAtSelection = new AuIconButton(":data/MsgSolicitSales.png", this);
	m_LookAtSelection->setToolTip(tr("<html><b>Look At Selection</b><br>"
				"Moves the view point to center on the selected places."
				"</html>"));
	if(m_StarMapGL)
		connect(m_LookAtSelection, SIGNAL(clicked()), m_StarMapGL, SLOT(LookAtSelection()));
	else if(m_StarMapW)
		connect(m_LookAtSelection, SIGNAL(clicked()), m_StarMapW, SLOT(LookAtSelection()));

	m_LookAtLastSelected = new AuIconButton(":data/MsgSpot.png", this);
	m_LookAtLastSelected->setToolTip(tr("<html><b>Look At Last Selected</b><br>"
				"Moves the view point to the last place that was selected."
				"</html>"));
	if(m_StarMapGL)
		connect(m_LookAtLastSelected, SIGNAL(clicked()), m_StarMapGL, SLOT(LookAtLastSelected()));
	else if(m_StarMapW)
		connect(m_LookAtLastSelected, SIGNAL(clicked()), m_StarMapW, SLOT(LookAtLastSelected()));

	m_Search = new AuIconButton(":data/MsgChristen.png", this);
	m_Search->setToolTip(tr("<html><b>Name Search...</b><br>"
				"Searches the star map for names."
				"</html>"));
	connect(m_Search, SIGNAL(clicked()), SLOT(Search()));

	m_SearchSystems = new AuIconButton(":data/bObservatory.png", this);
	m_SearchSystems->setToolTip(tr("<html><b>System Search...</b><br>"
				"Searches the star map for solar systems that match certain criteria."
				"</html>"));
	connect(m_SearchSystems, SIGNAL(clicked()), SLOT(SearchSystems()));

	if(!auApp->IsSingleUserDesignerMode())
	{
		m_Settings = new AuIconButton(":data/Settings.png", this);
		m_Settings->setToolTip(tr("<html><b>Highlighting...</b><br>"
					"Shows a dialog to configure highlighting in the star map and places list. "
					"Tooltips and controls on the dialog help to understand the information displayed on the starmap.</html>"));
		connect(m_Settings, SIGNAL(clicked()), SLOT(StarMapViewerSettings()));
	}
	else
		m_Settings = 0;

	m_RequestData = new AuIconButton(":data/dp_NavConsole.png", this);
	m_RequestData->setToolTip(tr("<html><b>Query Star Map...</b><br>"
				"Shows a dialog to request information from the master star map. "
				"Gets information about sectors and solar systems that were explored, scanned or detected by your empire."
				"</html>"));
	connect(m_RequestData, SIGNAL(clicked()), SLOT(RequestData()));

	m_SaveStarMap = 0;
	/* XML export nerfed
	m_SaveStarMap = new AuIconButton(":data/c_BlankDisk.png", this);
	m_SaveStarMap->setToolTip(tr("<html><b>Save Star Map...</b><br>"
				"Saves the current starmap data to an XML file."
				"</html>"));
	connect(m_SaveStarMap, SIGNAL(clicked()), SLOT(SaveStarMapXml()));
	*/

	m_RangeSpin = new QSpinBox(this);
	m_RangeSpin->setRange(0,99);
	m_RangeSpin->setValue(range);
	m_RangeSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_RangeSpin->setToolTip(tr("<html><b>Sector Range</b><br>"
				"When filtering by range, this controls the range of the sectors displayed in the galaxy image, "
				"from your current sector. "
				"The default range is 1, which restricts the display to your current sector plus all sectors up to 1 sector away. "
				"A range of 0 shows only your current sector. "
				"The accepted range is 0 to 99 sectors. "
				"</html>"));
	m_RangeSpin->setEnabled(false);	// until the filter is enabled.
	connect(m_RangeSpin, SIGNAL(valueChanged(int)), SLOT(RangeChanged(int)));

	QHBoxLayout* buttonRow = new QHBoxLayout;
	buttonRow->addWidget(m_ShowGalaxy);
	buttonRow->addWidget(m_ShowWormholes);
	buttonRow->addWidget(m_ShowScanned);
	buttonRow->addWidget(m_ShowSignals);
	buttonRow->addWidget(m_ShowStations);
	buttonRow->addWidget(m_ShowShips);
	if(m_Settings)
		buttonRow->addWidget(m_Settings);
	buttonRow->addStretch(4);
	buttonRow->addWidget(m_FilterSelection);
	buttonRow->addWidget(m_FilterExplored);
	buttonRow->addWidget(m_FilterRange);
	buttonRow->addWidget(m_RangeSpin);
	buttonRow->addStretch(1);
	buttonRow->addWidget(m_ClearSelection);
	buttonRow->addWidget(m_LookAtSelection);
	buttonRow->addWidget(m_LookAtLastSelected);
	buttonRow->addStretch(1);
	buttonRow->addWidget(m_Search);
	buttonRow->addWidget(m_SearchSystems);
	if(m_RequestData || m_SaveStarMap)
	{
		buttonRow->addStretch(1);
		if(m_RequestData)
			buttonRow->addWidget(m_RequestData);
		if(m_SaveStarMap)
			buttonRow->addWidget(m_SaveStarMap);
	}

	// Assemble the widgets into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Horizontal);
	if(m_StarMapGL)
		m_Splitter->addWidget(m_StarMapGL);
	else if(m_StarMapW)
		m_Splitter->addWidget(m_StarMapW);
	m_Splitter->addWidget(m_PlacesView);
	m_Splitter->setCollapsible(0, false);						// Prevent starmap from collapsing to 0. People lose it.
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addLayout(buttonRow, row,0);
	row++;
	lay->addWidget(m_Splitter, row,0);
	lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	// Init places model.
	InitPlaces();

	// Restore splitter state.
	QByteArray splitterState;
	g_Au.Settings().readEntry("Widget/Locator/StarMap/Splitter", splitterState);
	if(!splitterState.isEmpty())
		m_Splitter->restoreState(splitterState);

	connect(this, SIGNAL(StarMapAllRequested()), frame, SLOT(StarMapUpdateAllStarted()));
	connect(this, SIGNAL(StarMapGalaxyRequested(int)), frame, SLOT(StarMapUpdateGalaxyStarted(int)));
	connect(this, SIGNAL(StarMapSectorRequested(AuId, unsigned int)), frame, SLOT(StarMapUpdateSectorStarted(AuId, unsigned int)));
	connect(this, SIGNAL(StarMapAllReceived()), frame, SLOT(StarMapUpdateAllComplete()));
	connect(this, SIGNAL(StarMapGalaxyReceived(int)), frame, SLOT(StarMapUpdateGalaxyComplete(int)));
	connect(this, SIGNAL(StarMapSectorReceived(int, AuIdList)), frame, SLOT(StarMapUpdateSectorComplete(int, AuIdList)));
}

AuLocatorStarMap::~AuLocatorStarMap()
{
	if(m_SearchDlg)
		m_SearchDlg->close();
	if(m_SearchSystemsDlg)
		m_SearchSystemsDlg->close();
}

QSize AuLocatorStarMap::sizeHint() const
{
	return QSize(1000,800);
}

void AuLocatorStarMap::PlaceActivated(const AuId& id)
{
	ActivatePlaceRecursive(id, QModelIndex());
}

bool AuLocatorStarMap::ActivatePlaceRecursive(const AuId& id, const QModelIndex& parentIdx)
{
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
		if(itemId == id)
		{
			// Found the place to activate.
			PlaceActivated(itemIdx);
			m_PlacesView->scrollTo(itemIdx);
			return true;
		}
		else
		{
			// Recurse.
			if(ActivatePlaceRecursive(id, itemIdx))
				return true;
		}
	}
	return false;
}

void AuLocatorStarMap::PlaceActivated(const QModelIndex& sel)
{
	if(sel.isValid())
	{
		QModelIndex col0 = sel.sibling(sel.row(), 0);

		// Get the atom id from the selected item.
		// Get a copy of the job offer from the selected item.
		AuId atomId = AuId(col0.data(Qt::UserRole).toUInt());
		if(m_StarMapGL)
			m_StarMapGL->LookAtPlace(atomId);
		else if(m_StarMapW)
			m_StarMapW->LookAtPlace(atomId);
	}
}

void AuLocatorStarMap::PlacesSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Deselect places.
	QModelIndexList deselectem = deselected.indexes();
	for(QModelIndexList::const_iterator si = deselectem.cbegin(); si != deselectem.cend(); si++)
	{
		// Get the atom id from the selected item.
		QModelIndex col0 = (*si).sibling((*si).row(), 0);
		AuId atomId = AuId(col0.data(Qt::UserRole).toUInt());
		if(m_StarMapGL)
			m_StarMapGL->DeselectPlace(atomId);
		else if(m_StarMapW)
			m_StarMapW->DeselectPlace(atomId);
	}

	// Select places.
	QModelIndexList selectem = selected.indexes();
	for(QModelIndexList::const_iterator si = selectem.cbegin(); si != selectem.cend(); si++)
	{
		// Get the atom id from the selected item.
		QModelIndex col0 = (*si).sibling((*si).row(), 0);
		AuId atomId = AuId(col0.data(Qt::UserRole).toUInt());
		if(m_StarMapGL)
			m_StarMapGL->SelectPlace(atomId);
		else if(m_StarMapW)
			m_StarMapW->SelectPlace(atomId);
	}
}

void AuLocatorStarMap::ToggleFilterRange()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsRangeFiltered())
		{
			m_StarMapGL->FilterRange(false);
			m_FilterRange->setChecked(false);
			m_RangeSpin->setEnabled(false);
		}
		else
		{
			m_StarMapGL->FilterRange(true);
			m_FilterRange->setChecked(true);
			m_RangeSpin->setEnabled(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsRangeFiltered())
		{
			m_StarMapW->FilterRange(false);
			m_FilterRange->setChecked(false);
			m_RangeSpin->setEnabled(false);
		}
		else
		{
			m_StarMapW->FilterRange(true);
			m_FilterRange->setChecked(true);
			m_RangeSpin->setEnabled(true);
		}
	}
}

void AuLocatorStarMap::ToggleFilterExplored()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsExploredFiltered())
		{
			m_StarMapGL->FilterExplored(false);
			m_FilterExplored->setChecked(false);
		}
		else
		{
			m_StarMapGL->FilterExplored(true);
			m_FilterExplored->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsExploredFiltered())
		{
			m_StarMapW->FilterExplored(false);
			m_FilterExplored->setChecked(false);
		}
		else
		{
			m_StarMapW->FilterExplored(true);
			m_FilterExplored->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleFilterSelection()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsSelectionFiltered())
		{
			m_StarMapGL->FilterSelection(false);
			m_FilterSelection->setChecked(false);
		}
		else
		{
			m_StarMapGL->FilterSelection(true);
			m_FilterSelection->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsSelectionFiltered())
		{
			m_StarMapW->FilterSelection(false);
			m_FilterSelection->setChecked(false);
		}
		else
		{
			m_StarMapW->FilterSelection(true);
			m_FilterSelection->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowGalaxy()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsGalaxyVisible())
		{
			m_StarMapGL->HideGalaxy();
			m_ShowGalaxy->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowGalaxy();
			m_ShowGalaxy->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsGalaxyVisible())
		{
			m_StarMapW->HideGalaxy();
			m_ShowGalaxy->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowGalaxy();
			m_ShowGalaxy->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowScanned()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsScannedVisible())
		{
			m_StarMapGL->HideScanned();
			m_ShowScanned->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowScanned();
			m_ShowScanned->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsScannedVisible())
		{
			m_StarMapW->HideScanned();
			m_ShowScanned->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowScanned();
			m_ShowScanned->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowShips()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsShipsVisible())
		{
			m_StarMapGL->HideShips();
			m_ShowShips->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowShips();
			m_ShowShips->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsShipsVisible())
		{
			m_StarMapW->HideShips();
			m_ShowShips->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowShips();
			m_ShowShips->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowStations()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsStationsVisible())
		{
			m_StarMapGL->HideStations();
			m_ShowStations->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowStations();
			m_ShowStations->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsStationsVisible())
		{
			m_StarMapW->HideStations();
			m_ShowStations->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowStations();
			m_ShowStations->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowSignals()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsSignalsVisible())
		{
			m_StarMapGL->HideSignals();
			m_ShowSignals->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowSignals();
			m_ShowSignals->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsSignalsVisible())
		{
			m_StarMapW->HideSignals();
			m_ShowSignals->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowSignals();
			m_ShowSignals->setChecked(true);
		}
	}
}

void AuLocatorStarMap::ToggleShowWormholes()
{
	if(m_StarMapGL)
	{
		if(m_StarMapGL->IsWormholesVisible())
		{
			m_StarMapGL->HideWormholes();
			m_ShowWormholes->setChecked(false);
		}
		else
		{
			m_StarMapGL->ShowWormholes();
			m_ShowWormholes->setChecked(true);
		}
	}
	else if(m_StarMapW)
	{
		if(m_StarMapW->IsWormholesVisible())
		{
			m_StarMapW->HideWormholes();
			m_ShowWormholes->setChecked(false);
		}
		else
		{
			m_StarMapW->ShowWormholes();
			m_ShowWormholes->setChecked(true);
		}
	}
}

void AuLocatorStarMap::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	RestoreSettings();

	if(!m_PlacesValid)
	{
		ListPlaces();
		if(m_StarMapGL)
			m_StarMapGL->LookAtPlace(AuId(GALAXY_Count));					// Look at location of scene atom.
		else if(m_StarMapW)
			m_StarMapW->LookAtPlace(AuId(GALAXY_Count));					// Look at location of scene atom.
	}
}

void AuLocatorStarMap::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);
	if(m_SearchDlg)
		m_SearchDlg->hide();
	if(m_SearchSystemsDlg)
		m_SearchSystemsDlg->hide();
}

void AuLocatorStarMap::RestoreSettings()
{
	m_PlacesView->RestoreSettings();
	if(m_StarMapGL)
	{
		m_StarMapGL->RestoreSettings();

		m_FilterExplored->setChecked(m_StarMapGL->IsExploredFiltered());
		m_FilterRange->setChecked(m_StarMapGL->IsRangeFiltered());
		m_FilterSelection->setChecked(m_StarMapGL->IsSelectionFiltered());
		m_RangeSpin->setEnabled(m_StarMapGL->IsRangeFiltered());
		//qDebug() << "AuLocatorStarMap::RestoreSettings galaxyVis" << m_StarMapGL->IsGalaxyVisible();

		m_ShowGalaxy->setChecked(m_StarMapGL->IsGalaxyVisible());
		m_ShowScanned->setChecked(m_StarMapGL->IsScannedVisible());
		m_ShowShips->setChecked(m_StarMapGL->IsShipsVisible());
		m_ShowSignals->setChecked(m_StarMapGL->IsSignalsVisible());
		m_ShowStations->setChecked(m_StarMapGL->IsStationsVisible());
		m_ShowWormholes->setChecked(m_StarMapGL->IsWormholesVisible());
	}
	else if(m_StarMapW)
	{
		m_StarMapW->RestoreSettings();

		m_FilterExplored->setChecked(m_StarMapW->IsExploredFiltered());
		m_FilterRange->setChecked(m_StarMapW->IsRangeFiltered());
		m_FilterSelection->setChecked(m_StarMapW->IsSelectionFiltered());
		m_RangeSpin->setEnabled(m_StarMapW->IsRangeFiltered());
		//qDebug() << "AuLocatorStarMap::RestoreSettings galaxyVis" << m_StarMapW->IsGalaxyVisible();

		m_ShowGalaxy->setChecked(m_StarMapW->IsGalaxyVisible());
		m_ShowScanned->setChecked(m_StarMapW->IsScannedVisible());
		m_ShowShips->setChecked(m_StarMapW->IsShipsVisible());
		m_ShowSignals->setChecked(m_StarMapW->IsSignalsVisible());
		m_ShowStations->setChecked(m_StarMapW->IsStationsVisible());
		m_ShowWormholes->setChecked(m_StarMapW->IsWormholesVisible());
	}

	// Filter
	QByteArray ba;
	g_Au.Settings().readEntry("Widget/Locator/StarMap/EmpiresIgnored", ba);
	if(!ba.isEmpty())
	{
		QDataStream in(ba);
		in >> m_EmpiresIgnored;
	}
	m_ShowDetectedPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowDetectedPlaces", true);
	m_ShowEmpirePlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowEmpirePlaces", true);
	m_ShowEnemyPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowEnemyPlaces", true);
	m_ShowExploredPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowExploredPlaces", true);
	m_ShowFriendPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowFriendPlaces", true);
	m_ShowNeutralPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowNeutralPlaces", true);
	m_ShowUnknownPlaces = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/ShowUnknownPlaces", true);
	m_UnknownPrecedesEnemy = g_Au.Settings().readBoolEntry("Widget/Locator/StarMap/UnknownPrecedesEnemy", true);

	if(m_Settings)
	{
		if(m_EmpiresIgnored.isEmpty() &&
				m_ShowEmpirePlaces &&
				m_ShowFriendPlaces &&
				m_ShowNeutralPlaces &&
				m_ShowEnemyPlaces &&
				m_ShowUnknownPlaces &&
				m_ShowExploredPlaces &&
				m_ShowDetectedPlaces)
			m_Settings->setChecked(false);							// Filter is clear so raise the button.
		else
			m_Settings->setChecked(true);							// Filter is active so depress the button.
	}
}

void AuLocatorStarMap::SplitterMoved(int /*pos*/, int /*index*/)
{
	// Save splitter state.
	QByteArray splitterState = m_Splitter->saveState();
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/Splitter", splitterState);
}

void AuLocatorStarMap::SaveSettings()
{
	// Filter
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out << m_EmpiresIgnored;
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/EmpiresIgnored", ba);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowDetectedPlaces", m_ShowDetectedPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowEmpirePlaces", m_ShowEmpirePlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowEnemyPlaces", m_ShowEnemyPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowExploredPlaces", m_ShowExploredPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowFriendPlaces", m_ShowFriendPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowNeutralPlaces", m_ShowNeutralPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/ShowUnknownPlaces", m_ShowUnknownPlaces);
	g_Au.Settings().writeEntry("Widget/Locator/StarMap/UnknownPrecedesEnemy", m_UnknownPrecedesEnemy);
}

void AuLocatorStarMap::RequestData()
{
	AuStarMapQuery dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	quint32 empireId = g_AuClient.StarMapEmpire();
	if(dlg.ClearMap())
	{
		g_AuClient.StarMapInit(empireId);
		InitPlaces();
	}
	AuId sceneSector(auApp->Frame()->View()->SceneSector());
	PostQuery(dlg.Query(), dlg.Galaxy(), sceneSector, dlg.Range());
	if(dlg.Include3xSectors())
	{
		// Add sectors in the 3x3x3 box around the scene atom.
		GALAXY gal = auApp->Frame()->View()->SceneGalaxy();
		AuEther& ether = g_Au.LockEtheriumDelete();
		const DaGalaxy* galaxy = ether.FindGalaxy(gal);
		if(galaxy)
		{
			DaAtomList subs;
			galaxy->ListSupersector(sceneSector, subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaSector* sector = dynamic_cast<const DaSector*>(*it);
				if(sector)
					auApp->Frame()->StarMapAddSector(sector, empireId);
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuLocatorStarMap::QuerySector(const AuId& sectorId)
{
	PostQuery(0, GALAXY_Count, sectorId, 0);
}

void AuLocatorStarMap::PostQuery(int what, GALAXY galaxy, const AuId& sectorId, quint16 range)
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId personId = gd.ClientUnitId();
	g_Au.UnlockData();
	if(personId.IsNull())
		return;
	quint32 personEmpireId = g_AuClient.StarMapEmpire();
	if(!personEmpireId)
		return;
	AuId personSystemId(auApp->Frame()->View()->SceneSolarSystem());

	quint8 query;	// query value wanted with the PG_StarMap packet.
	switch(what)
	{
	case 0:	// sector
		emit StarMapSectorRequested(sectorId, range);
		query = 2;
		break;
	case 1:	// galaxy
		emit StarMapGalaxyRequested(galaxy);
		query = 1;
		break;
	case 2:	// universe
		emit StarMapAllRequested();
		query = 0;
		break;
	default:
		return;
	}

	// This query must be made to the master scene server. By now, the client is talking to a worker.
	AuIdList sectorIds;					// returned sector ids when query is sector
	GALAXY sectorGalaxy = GALAXY_Count;	// galaxy of returned sectors when query is sector
	auApp->QueryStarmap(query, galaxy, sectorId, range, personId, personEmpireId, personSystemId, sectorIds, sectorGalaxy);

	//qDebug() << "AuLocatorStarMap::RequestData sectors received" << sectorIds.size() << "query what" << what;
	switch(what)
	{
	case 0:	// sector
		emit StarMapSectorReceived(sectorGalaxy, sectorIds);
		break;
	case 1:	// galaxy
		emit StarMapGalaxyReceived(galaxy);
		break;
	case 2:	// all
		emit StarMapAllReceived();
		break;
	}
}

void AuLocatorStarMap::StarMapUpdateAllStarted()
{
	m_RequestData->setEnabled(false);
}

void AuLocatorStarMap::StarMapUpdateGalaxyStarted(GALAXY galaxy)
{
	Q_UNUSED(galaxy);
	m_RequestData->setEnabled(false);
}

void AuLocatorStarMap::StarMapUpdateSectorStarted(const AuId& sectorId, unsigned int range)
{
	Q_UNUSED(sectorId);
	Q_UNUSED(range);
	m_RequestData->setEnabled(false);
}

void AuLocatorStarMap::StarMapUpdateAllComplete()
{
	m_RequestData->setEnabled(true);

	// Get current selection so we can restore it.
	AuIdList selection;
	if(m_StarMapGL)
		selection = m_StarMapGL->SelectedPlaces();
	else if(m_StarMapW)
		selection = m_StarMapW->SelectedPlaces();
	AuIdList expanded = ListExpanded();
	InitPlaces();
	if(!isVisible())
		m_PlacesValid = false;
	else
	{
		ListPlaces();

		// Restore selection and expanded branches.
		SelectPlaces(selection);
		ExpandPlaces(expanded);
	}
}

void AuLocatorStarMap::StarMapUpdateGalaxyComplete(GALAXY galaxy)
{
	m_RequestData->setEnabled(true);

	// Get current selection so we can restore it.
	AuIdList selection;
	if(m_StarMapGL)
		selection = m_StarMapGL->SelectedPlaces();
	else if(m_StarMapW)
		selection = m_StarMapW->SelectedPlaces();
	AuIdList expanded = ListExpanded();
	ClearGalaxy(galaxy);
	if(!isVisible())
		m_PlacesValid = false;
	else
	{
		ListPlaces();

		// Restore selection and expanded branches.
		SelectPlaces(selection);
		ExpandPlaces(expanded);
	}
}

void AuLocatorStarMap::StarMapUpdateSectorComplete(GALAXY galaxy, const AuIdList& sectorIds)
{
	m_RequestData->setEnabled(true);

	// Get current selection so we can restore it.
	AuIdList selection;
	if(m_StarMapGL)
		selection = m_StarMapGL->SelectedPlaces();
	else if(m_StarMapW)
		selection = m_StarMapW->SelectedPlaces();
	AuIdList expanded = ListExpanded();
	ClearSectors(galaxy, sectorIds);
	if(!isVisible())
		m_PlacesValid = false;
	else
	{
		ListPlaces();

		// Restore selection and expanded branches.
		SelectPlaces(selection);
		ExpandPlaces(expanded);
	}
}

void AuLocatorStarMap::InitPlaces()
{
	// Remove all existing places.
	m_PlacesModel.removeRows(0, m_PlacesModel.rowCount());
	if(m_StarMapGL)
		m_StarMapGL->ClearUniverse();
	else if(m_StarMapW)
		m_StarMapW->ClearUniverse();
	m_SectorsPresent.clear();

	const DgData& gd = g_Au.LockDataForRead();
	QString personName = gd.ClientUnitName(gd.ClientUnitId());
	g_Au.UnlockData();

	// Galaxy location of the "Current Location" item is calculated when it is hit.
	// It is identified with atom id = GALAXY_Count.
	AddPlace(QModelIndex(), AuId(GALAXY_Count), DaAtom::ATOM_Avatar, QIcon(":data/RangeClose.png"), personName,
			s_AvatarColor, AuPoint(0,0), "", AuId(0), GALAXY_Count, false,false,0,0,false,false, 0, 0);
}

void AuLocatorStarMap::ClearGalaxy(GALAXY galaxyToClear)
{
	QModelIndex galIdx;
	int galRow = 0;
	for(; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
	{
		// GALAXY enum stored on column 0.
		QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
		GALAXY galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
		if(galaxy == galaxyToClear)
		{
			galIdx = itemIdx;
			break;
		}
	}
	if(galIdx == QModelIndex())
		return;
	m_PlacesModel.removeRows(galRow, 1);
	if(m_StarMapGL)
		m_StarMapGL->ClearGalaxy(galaxyToClear);
	else if(m_StarMapW)
		m_StarMapW->ClearGalaxy(galaxyToClear);
	QHash<AuId, GALAXY>::const_iterator si = m_SectorsPresent.cbegin();
	while(si != m_SectorsPresent.cend())
	{
		if(*si == galaxyToClear)
			si = m_SectorsPresent.erase(si);
		else
			si++;
	}
}

void AuLocatorStarMap::ClearSectors(GALAXY galaxyToClear, const AuIdList& sectorIds)
{
	QModelIndex galIdx;
	for(int galRow=0; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
	{
		// GALAXY enum stored on column 0.
		QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
		GALAXY galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
		if(galaxy == galaxyToClear)
		{
			galIdx = itemIdx;
			break;
		}
	}
	if(galIdx == QModelIndex())
		return;
	// Delete from bottom to top so indexes don't change as we go.
	for(int sectorRow=m_PlacesModel.rowCount(galIdx)-1; sectorRow>=0; sectorRow--)
	{
		// Atom id stored on column 0.
		QModelIndex itemIdx = m_PlacesModel.index(sectorRow,0, galIdx);
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
		if(sectorIds.contains(itemId))
			m_PlacesModel.removeRows(sectorRow, 1, galIdx);
	}
	if(m_StarMapGL)
		m_StarMapGL->ClearSectors(sectorIds);
	else if(m_StarMapW)
		m_StarMapW->ClearSectors(sectorIds);
	for(AuIdList::const_iterator si = sectorIds.cbegin(); si != sectorIds.cend(); si++)
		m_SectorsPresent.remove(*si);
}

void AuLocatorStarMap::ListPlaces()
{
	//qDebug() << "AuLocatorStarMap::ListPlaces" << showProgress;
	// We could create the same icon over and over again from the same empire's flag. Cache them.
	QIcon sectorIcon(":data/RangeSector.png");
	QIcon systemIcon(":data/RangeSystem.png");
	QIcon scannedSystemIcon(":data/SolarSystemInfo.png");
	QIcon cityIcon(":data/City.png");
	QHash<quint32, QIcon> empireFlagIcons;

	// That's as much setup as we can do before locking the star map. Let's get it over with quickly.
	AuStarMapData starMap = g_AuClient.LockStarMapForRead();
	g_AuClient.UnlockStarMap();

	QProgressDialog* prog;
	prog = new QProgressDialog(this);
	prog->setWindowTitle(tr("Updating Places"));
	prog->setWindowModality(Qt::WindowModal);
	prog->setValue(0);
	prog->setRange(0, (int)starMap.m_Sectors.size());
	prog->setLabelText(tr("Adding %L1 known sectors to the list of places...").arg(starMap.m_Sectors.size()));

	// Get my empire data record.
	DgEmpire personEmpire;
	personEmpire.m_id = g_AuClient.m_StarMapEmpire;
	//qDebug() << "starmap empire" << g_AuClient.m_StarMapEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	if(!gd.Empire(personEmpire))
		personEmpire.ChangeEmpireName(personEmpire.Name(), tr("Empire"));
	g_Au.UnlockData();

	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		prog->setValue(prog->value()+1);
		if(m_SectorsPresent.contains((*sectI).first))
			continue;	// We already have this sector.

		// Locate the model index for the galaxy at the root.
		QModelIndex galIdx;
		GALAXY galaxy = GALAXY_Count;
		for(int galRow=0; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
		{
			// GALAXY enum stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
			galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
			if(galaxy == (*sectI).second.Galaxy())
			{
				galIdx = itemIdx;
				break;
			}
		}
		if(galIdx == QModelIndex())
		{
			// Galaxy not found. Create an entry.
			galIdx = AddPlace(QModelIndex(), AuId((*sectI).second.Galaxy()), DaAtom::ATOM_Galaxy,
					QIcon(":data/RangeGalaxy.png"), GalaxyName((*sectI).second.Galaxy()), s_GalaxyColor, AuPoint(0,0), "",
					AuId(0), (*sectI).second.Galaxy(), false,false,0,0,false,false, 0, 0);
		}

		//qDebug() << "sector" << sectI.key() << (*sectI).second.m_Name << "loc" << (*sectI).second.m_Loc;
		// Locate the model index for the sector, if it already exists.
		QModelIndex sectIdx;
		for(int sectorRow=0; sectorRow<m_PlacesModel.rowCount(galIdx); sectorRow++)
		{
			// Atom id stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(sectorRow,0, galIdx);
			AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
			if(itemId == (*sectI).first)
			{
				sectIdx = itemIdx;
				break;
			}
		}
		if(sectIdx == QModelIndex())
		{
			// Sector not found.
			m_SectorsPresent.insert((*sectI).first, galaxy);
			// Convert from parsec coord to sector coord.
			AuPoint fLoc = (*sectI).second.m_Loc/10.0;

			QString sectorName((*sectI).second.m_Name);
			if(sectorName == "Sector")
				sectorName += QString(" (%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);

			sectIdx = AddPlace(galIdx, (*sectI).first, DaAtom::ATOM_Sector, sectorIcon, sectorName,
					QColor(255,255,255), (*sectI).second.m_Loc, "", AuId(0), (*sectI).second.Galaxy(), false,false,0,0,false,false,
					0, 0);

			// Process solar systems.
			for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
					sysI != (*sectI).second.m_Systems.end(); sysI++)
			{
				//qDebug() << " system" << (*sysI).second.m_Name << "loc" << (*sysI).second.m_Loc;
				AuPoint systemLoc = (*sectI).second.m_Loc + (*sysI).second.m_Loc;

				quint8 genHab = 0;
				quint32 genSeed = 0;
				QIcon sysIcon(systemIcon);
				bool isScanned = (*sysI).second.IsScanned();
				if(isScanned)
					sysIcon = scannedSystemIcon;
				genHab = (*sysI).second.m_GenHab;
				genSeed = (*sysI).second.m_GenSeed;

				quint8 signalStr = (*sysI).second.DetectedSignalStrength();

				QModelIndex sysIdx;
				if((*sysI).second.IsExplored())
				{
					// System is explored.
					quint16 empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips,
							enemyStations;
					SpacecraftPresent((*sysI).second, personEmpire.m_id, empireShips, empireStations, friendShips, friendStations,
							neutralShips, neutralStations, enemyShips, enemyStations);

					sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
							(*sysI).second.m_Name, s_ExColor, systemLoc, "", (*sectI).first, (*sectI).second.Galaxy(),
							(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, true, isScanned, 0,
							signalStr,
							empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips,
							enemyStations);

					// Explored systems show their wormholes, both explored and unexplored wormholes.
					for(QList<AuStarMapWormhole>::const_iterator wi = (*sysI).second.m_Wormholes.cbegin();
							wi != (*sysI).second.m_Wormholes.cend(); wi++)
						if((*wi).Polarity() != AuWormhole::Intergalactic &&
								(*wi).Polarity() != AuWormhole::IntergalacticOneWay)
						{
							if(m_StarMapGL)
								m_StarMapGL->AddPlaceWormhole((*sysI).first, (*wi).VecToDest(), (*wi).IsExplored());
							else if(m_StarMapW)
								m_StarMapW->AddPlaceWormhole((*sysI).first, (*wi).VecToDest(), (*wi).IsExplored());
						}
				}
				else if(signalStr && m_ShowDetectedPlaces)
				{
					// Warp signature detected from unexplored system.
					sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
							(*sysI).second.m_Name, s_DetectedColor, systemLoc, tr(""), (*sectI).first, (*sectI).second.Galaxy(),
							(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, false, false, 0,
							signalStr);
				}
				else
				{
					// Unexplored and undetected.
					sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
							(*sysI).second.m_Name, s_BlackColor, systemLoc, tr(""), (*sectI).first, (*sectI).second.Galaxy(),
							(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, false,false, 0, 0);
				}

				// Cities are only reported for systems that have been explored, btw.
				if((*sysI).second.m_Cities.size() && sysIdx != QModelIndex())
				{
					// Process cities.
					for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
							cityI != (*sysI).second.m_Cities.end(); cityI++)
					{
						// What do we know about the empire who owns the city.
						QString empireName;
						POLITICAL_STANCE stance;
						QColor stanceColor;
						if(personEmpire.m_id == (*cityI).second.m_Empire)
						{
							// It is our empire.
							empireName = personEmpire.Name();
							stance = POL_Empire;
							stanceColor = PoliticalStanceColor(stance, false);
						}
						else
						{
							// It is not our empire.
							stance = personEmpire.PoliticalStance((*cityI).second.m_Empire);	// Will get default vs unknown.
							if(personEmpire.IsEmpireKnown((*cityI).second.m_Empire))
							{
								// It is an empire we have encountered so we know its name.
								empireName = personEmpire.NameOfKnownEmpire((*cityI).second.m_Empire);
								stanceColor = PoliticalStanceColor(stance, false);
							}
							else
							{
								// It is an unknown empire.
								empireName = tr("Unknown Empire");
								stanceColor = s_UnkColor;
							}
						}

						QHash<quint32, QIcon>::const_iterator ii = empireFlagIcons.constFind((*cityI).second.m_Empire);
						if(ii == empireFlagIcons.cend())
						{
							// Flag for that empire is not in our cache yet.
							DgEmpire empire;
							empire.m_id = (*cityI).second.m_Empire;
							const DgData& gd = g_Au.LockDataForRead();
							bool hasEmpire = gd.Empire(empire);
							g_Au.UnlockData();
							if(hasEmpire)
							{
								// We have the empire record locally so we can make an icon from its flag.
								QPixmap pxflag;
								pxflag.convertFromImage(empire.m_flag);
								ii = empireFlagIcons.insert((*cityI).second.m_Empire, QIcon(pxflag));
							}
							else
							{
								// We do not have the empire record so use the generic city icon instead of a flag.
								ii = empireFlagIcons.insert((*cityI).second.m_Empire, cityIcon);
							}
						}
						AddPlace(sysIdx, (*cityI).first, DaAtom::ATOM_Bd, *ii, (*cityI).second.m_Name,
								stanceColor, systemLoc, empireName, (*sectI).first, (*sectI).second.Galaxy(),
								false,false, 0,0,false,false, (*cityI).second.m_Empire, 0);
					}
				}
			}
		}

		HighlightSector(sectI, sectIdx, personEmpire);

		// Test for cancel button on progress dialog.
		auApp->processEvents();
		if(prog->wasCanceled())
			break;
	}

	//qDebug() << "AuLocatorStarMap::ListPlaces done";

	delete prog;
	m_PlacesValid = true;
}

// Change place color highlighting of sectors and solar systems based on the current filter.
// Cities are not changed because they always reflect our stance toward their owner empire.
void AuLocatorStarMap::HighlightPlaces()
{
	// That's as much setup as we can do before locking the star map. Let's get it over with quickly.
	AuStarMapData starMap = g_AuClient.LockStarMapForRead();
	g_AuClient.UnlockStarMap();

	QProgressDialog prog(this);
	prog.setWindowTitle(tr("Highlighting Places"));
	prog.setWindowModality(Qt::WindowModal);
	prog.setValue(0);
	prog.setRange(0, (int)starMap.m_Sectors.size());
	prog.setLabelText(tr("Highlighting %L1 known sectors...").arg(starMap.m_Sectors.size()));

	// Get my empire data record.
	DgEmpire personEmpire;
	personEmpire.m_id = g_AuClient.m_StarMapEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	if(!gd.Empire(personEmpire))
		personEmpire.ChangeEmpireName(personEmpire.Name(), tr("Empire"));
	g_Au.UnlockData();

	// Process sectors.
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		prog.setValue(prog.value()+1);

		// Locate the model index for the galaxy at the root.
		QModelIndex galIdx;
		for(int galRow=0; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
		{
			// GALAXY enum stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
			GALAXY galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
			if(galaxy == (*sectI).second.Galaxy())
			{
				galIdx = itemIdx;
				break;
			}
		}
		if(galIdx == QModelIndex())
			continue;

		// Locate the model index for the sector.
		QModelIndex sectIdx;
		for(int sectorRow=0; sectorRow<m_PlacesModel.rowCount(galIdx); sectorRow++)
		{
			// Atom id stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(sectorRow,0, galIdx);
			AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
			if(itemId == (*sectI).first)
			{
				sectIdx = itemIdx;
				break;
			}
		}
		if(sectIdx == QModelIndex())
			continue;	// Sector not found?

		HighlightSector(sectI, sectIdx, personEmpire);

		// Test for cancel button on progress dialog.
		auApp->processEvents();
		if(prog.wasCanceled())
			break;
	}
}

void AuLocatorStarMap::HighlightSector(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI,
		const QModelIndex& sectIdx, const DgEmpire& personEmpire)
{
	QColor exploredColor;
	if(m_ShowExploredPlaces)
		exploredColor = s_ExColor;
	else
		exploredColor = s_BlackColor;

	QString sectorStatus = tr("Unexplored");
	POLITICAL_STANCE sectorStance = POL_Vassal;	// Only set if we find a city with a locally known empire.
	bool anyExploredInSector = false;
	bool anyUnexDetectedInSector = false;
	bool anyUnknownEmpiresInSector = false;

	// Process solar systems.
	for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
			sysI != (*sectI).second.m_Systems.end(); sysI++)
	{
		QString systemStatus;
		POLITICAL_STANCE systemStance = POL_Vassal;
		bool anyUnknownEmpiresInSystem = false;

		// Locate the model index for the solar system.
		QModelIndex sysIdx;
		for(int sysRow=0; sysRow<m_PlacesModel.rowCount(sectIdx); sysRow++)
		{
			// Atom id stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(sysRow,0, sectIdx);
			AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
			if(itemId == (*sysI).first)
			{
				sysIdx = itemIdx;
				break;
			}
		}

		quint8 signalStr = (*sysI).second.DetectedSignalStrength();

		if((*sysI).second.IsExplored())
		{
			// System is explored.
			systemStatus = tr("Explored");
			anyExploredInSector = true;
		}
		else if(signalStr && m_ShowDetectedPlaces)
		{
			systemStatus = tr("Detected");
			anyUnexDetectedInSector = true;
		}
		else
			systemStatus = tr("Unexplored");

		// Cities are only reported for systems that have been explored, btw.
		if((*sysI).second.m_Cities.size())
		{
			// Process cities.
			for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
					cityI != (*sysI).second.m_Cities.end(); cityI++)
			{
				// Filter ignored empires.
				if(m_EmpiresIgnored.contains((*cityI).second.m_Empire))
					continue;

				// What do we know about the empire who owns the city.
				POLITICAL_STANCE stance;
				QColor stanceColor;
				if(personEmpire.m_id == (*cityI).second.m_Empire)
				{
					// It is our empire.
					if(!m_ShowEmpirePlaces)
						continue;

					stance = POL_Empire;
					stanceColor = PoliticalStanceColor(stance, false);
				}
				else
				{
					// It is not our empire.
					stance = personEmpire.PoliticalStance((*cityI).second.m_Empire);	// Will get default vs unknown.
					if(personEmpire.IsEmpireKnown((*cityI).second.m_Empire))
					{
						// It is an empire we have encountered so we know its name.
						switch(stance)
						{
						case POL_Empire:
						case POL_Vassal:
							if(!m_ShowEmpirePlaces)
								continue;
							break;
						case POL_Friend:
							if(!m_ShowFriendPlaces)
								continue;
							break;
						case POL_Neutral:
							if(!m_ShowNeutralPlaces)
								continue;
							break;
						case POL_Enemy:
							if(!m_ShowEnemyPlaces)
								continue;
							break;
						}
						stanceColor = PoliticalStanceColor(stance, false);
					}
					else
					{
						// It is an unknown empire.
						if(!m_ShowUnknownPlaces)
							continue;
						stanceColor = s_UnkColor;
						anyUnknownEmpiresInSector = true;
						anyUnknownEmpiresInSystem = true;
					}
				}

				if(m_UnknownPrecedesEnemy)
				{
					// Update stance for solar system.
					if(anyUnknownEmpiresInSystem)
					{
						systemStance = POL_Empire;
						systemStatus = tr("Unknown Empire City Present");
					}
					else if(stance == POL_Enemy)
					{
						systemStance = POL_Enemy;
						systemStatus = tr("Enemy City Present");
					}
					else if(stance == POL_Neutral && systemStance != POL_Enemy)
					{
						systemStance = POL_Neutral;
						systemStatus = tr("Neutral City Present");
					}
					else if(stance == POL_Friend && systemStance != POL_Enemy && systemStance != POL_Neutral)
					{
						systemStance = POL_Friend;
						systemStatus = tr("Friend City Present");
					}
					else if(systemStance != POL_Enemy && systemStance != POL_Neutral && systemStance != POL_Friend)
					{
						systemStance = POL_Empire;
						systemStatus = tr("%1 City Present").arg(personEmpire.Name());
					}

					// Update stance for sector.
					if(anyUnknownEmpiresInSector)
					{
						sectorStance = POL_Empire;
						sectorStatus = tr("Unknown Empire City Present");
					}
					else if(stance == POL_Enemy)
					{
						sectorStance = POL_Enemy;
						sectorStatus = tr("Enemy City Present");
					}
					else if(stance == POL_Neutral && sectorStance != POL_Enemy)
					{
						sectorStance = POL_Neutral;
						sectorStatus = tr("Neutral City Present");
					}
					else if(stance == POL_Friend && sectorStance != POL_Enemy && sectorStance != POL_Neutral)
					{
						sectorStance = POL_Friend;
						sectorStatus = tr("Friend City Present");
					}
					else if(sectorStance != POL_Enemy && sectorStance != POL_Neutral && sectorStance != POL_Friend)
					{
						sectorStance = POL_Empire;
						sectorStatus = tr("%1 City Present").arg(personEmpire.Name());
					}
				}
				else
				{
					// Update stance for solar system.
					if(stance == POL_Enemy)
					{
						systemStance = POL_Enemy;
						systemStatus = tr("Enemy City Present");
					}
					else if(anyUnknownEmpiresInSystem)
					{
						if(systemStance != POL_Enemy)
						{
							systemStance = POL_Empire;
							systemStatus = tr("Unknown Empire City Present");
						}
					}
					else if(stance == POL_Neutral && systemStance != POL_Enemy)
					{
						systemStance = POL_Neutral;
						systemStatus = tr("Neutral City Present");
					}
					else if(stance == POL_Friend && systemStance != POL_Enemy && systemStance != POL_Neutral)
					{
						systemStance = POL_Friend;
						systemStatus = tr("Friend City Present");
					}
					else if(systemStance != POL_Enemy && systemStance != POL_Neutral && systemStance != POL_Friend)
					{
						systemStance = POL_Empire;
						systemStatus = tr("%1 City Present").arg(personEmpire.Name());
					}

					// Update stance for sector.
					if(stance == POL_Enemy)
					{
						sectorStance = POL_Enemy;
						sectorStatus = tr("Enemy City Present");
					}
					else if(anyUnknownEmpiresInSector)
					{
						if(sectorStance != POL_Enemy)
						{
							sectorStance = POL_Empire;
							sectorStatus = tr("Unknown Empire City Present");
						}
					}
					else if(stance == POL_Neutral && sectorStance != POL_Enemy)
					{
						sectorStance = POL_Neutral;
						sectorStatus = tr("Neutral City Present");
					}
					else if(stance == POL_Friend && sectorStance != POL_Enemy && sectorStance != POL_Neutral)
					{
						sectorStance = POL_Friend;
						sectorStatus = tr("Friend City Present");
					}
					else if(sectorStance != POL_Enemy && sectorStance != POL_Neutral && sectorStance != POL_Friend)
					{
						sectorStance = POL_Empire;
						sectorStatus = tr("%1 City Present").arg(personEmpire.Name());
					}
				}
			}
		}

		// Set system name color based on cities in the system.
		QColor systemStanceColor;
		if(m_UnknownPrecedesEnemy)
		{
			if(!(*sysI).second.IsExplored() && signalStr && m_ShowDetectedPlaces)
				systemStanceColor = s_DetectedColor;				// Power emission detected in unexplored system.
			else if(!(*sysI).second.IsExplored())
				systemStanceColor = s_UnexColor;					// Unexplored/undetected system.
			else if(systemStance == POL_Vassal)
				systemStanceColor = exploredColor;					// Explored but uninhabited system.
			else if(anyUnknownEmpiresInSystem)
				systemStanceColor = s_UnkColor;						// Unknown empire present is precedent over stance.
			else
				systemStanceColor = PoliticalStanceColor(systemStance, false);	// Explored and inhabited.
		}
		else
		{
			if(!(*sysI).second.IsExplored() && signalStr && m_ShowDetectedPlaces)
				systemStanceColor = s_DetectedColor;				// Power emission detected in unexplored system.
			else if(!(*sysI).second.IsExplored())
				systemStanceColor = s_UnexColor;					// Unexplored system.
			else if(systemStance == POL_Vassal)
				systemStanceColor = exploredColor;					// Explored but uninhabited system.
			else
			{
				systemStanceColor = PoliticalStanceColor(systemStance, false);	// Explored and inhabited.
				if(anyUnknownEmpiresInSystem && systemStanceColor != PoliticalStanceColor(POL_Enemy, false))
					systemStanceColor = s_UnkColor;					// Unknown empire precedent over all but enemy.
			}
		}

		if(signalStr && m_ShowDetectedPlaces)
			systemStatus += tr(", Signal %1").arg(signalStr);

		if(sysIdx != QModelIndex())
		{
			QStandardItem* systemItem = m_PlacesModel.itemFromIndex(sysIdx);
			if(systemItem)
				systemItem->setForeground(QBrush(systemStanceColor));
			QModelIndex sysIdx2 = sysIdx.sibling(sysIdx.row(), 1);
			systemItem = m_PlacesModel.itemFromIndex(sysIdx2);
			if(systemItem)
				systemItem->setForeground(QBrush(systemStanceColor));
			QModelIndex sysIdx3 = sysIdx.sibling(sysIdx.row(), 2);
			m_PlacesModel.setData(sysIdx3, systemStatus);					// Set system status text.
			systemItem = m_PlacesModel.itemFromIndex(sysIdx3);
			if(systemItem)
				systemItem->setForeground(QBrush(systemStanceColor));
		}

		// Update color of star map item.
		if(m_StarMapGL)
		{
			if(systemStanceColor != s_UnexColor)
				m_StarMapGL->SetPlaceColor((*sysI).first, systemStanceColor);
			else
				m_StarMapGL->SetPlaceColor((*sysI).first, s_BlackColor);
		}
		else if(m_StarMapW)
		{
			if(systemStanceColor != s_UnexColor)
				m_StarMapW->SetPlaceColor((*sysI).first, systemStanceColor);
			else
				m_StarMapW->SetPlaceColor((*sysI).first, s_BlackColor);
		}
	}

	// Set sector name color based on cities in the sector.
	QColor sectorStanceColor;
	if(m_UnknownPrecedesEnemy)
	{
		if(anyUnexDetectedInSector)
		{
			sectorStanceColor = s_DetectedColor;						// Emission detected in unexplored sector.
			sectorStatus = tr("Detected");
		}
		else if(!anyExploredInSector)
		{
			sectorStanceColor = s_UnexColor;							// Unexplored sector.
			sectorStatus = tr("Unexplored");
		}
		else if(sectorStance == POL_Vassal)
			sectorStanceColor = exploredColor;							// Explored but uninhabited sector.
		else if(anyUnknownEmpiresInSector)
			sectorStanceColor = s_UnkColor;						// Unknown empire present is precedent over stance.
		else
			sectorStanceColor = PoliticalStanceColor(sectorStance, false);
	}
	else
	{
		if(anyUnexDetectedInSector)
		{
			sectorStanceColor = s_DetectedColor;						// Emission detected in unexplored sector.
			sectorStatus = tr("Detected");
		}
		else if(!anyExploredInSector)
		{
			sectorStanceColor = s_UnexColor;							// Unexplored sector.
			sectorStatus = tr("Unexplored");
		}
		else if(sectorStance == POL_Vassal)
			sectorStanceColor = exploredColor;							// Explored but uninhabited sector.
		else
		{
			sectorStanceColor = PoliticalStanceColor(sectorStance, false);
			if(anyUnknownEmpiresInSector && sectorStanceColor != PoliticalStanceColor(POL_Enemy, false))
				sectorStanceColor = s_UnkColor;					// Unknown empire precedent over all but enemy.
		}
	}

	QStandardItem* sectorItem = m_PlacesModel.itemFromIndex(sectIdx);
	if(sectorItem)
		sectorItem->setForeground(QBrush(sectorStanceColor));
	QModelIndex sectIdx2 = sectIdx.sibling(sectIdx.row(), 1);
	sectorItem = m_PlacesModel.itemFromIndex(sectIdx2);
	if(sectorItem)
		sectorItem->setForeground(QBrush(sectorStanceColor));
	QModelIndex sectIdx3 = sectIdx.sibling(sectIdx.row(), 2);
	m_PlacesModel.setData(sectIdx3, sectorStatus);					// Set sector status text.
	sectorItem = m_PlacesModel.itemFromIndex(sectIdx3);
	if(sectorItem)
		sectorItem->setForeground(QBrush(sectorStanceColor));

	// Update color of star map item.
	if(m_StarMapGL)
	{
		if(sectorStanceColor != s_UnexColor)
			m_StarMapGL->SetPlaceColor((*sectI).first, sectorStanceColor);
		else
			m_StarMapGL->SetPlaceColor((*sectI).first, s_BlackColor);
	}
	else if(m_StarMapW)
	{
		if(sectorStanceColor != s_UnexColor)
			m_StarMapW->SetPlaceColor((*sectI).first, sectorStanceColor);
		else
			m_StarMapW->SetPlaceColor((*sectI).first, s_BlackColor);
	}
}

void AuLocatorStarMap::UpdatePlaceFlags()
{
	QIcon cityIcon(":data/City.png");
	QHash<quint32, QIcon> empireFlagIcons;

	// That's as much setup as we can do before locking the star map. Let's get it over with quickly.
	const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();

	// Process sectors.
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		// Locate the model index for the galaxy at the root.
		QModelIndex galIdx;
		for(int galRow=0; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
		{
			// GALAXY enum stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
			GALAXY galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
			if(galaxy == (*sectI).second.Galaxy())
			{
				galIdx = itemIdx;
				break;
			}
		}
		if(galIdx == QModelIndex())
			continue;

		// Locate the model index for the sector.
		QModelIndex sectIdx;
		for(int sectorRow=0; sectorRow<m_PlacesModel.rowCount(galIdx); sectorRow++)
		{
			// Atom id stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(sectorRow,0, galIdx);
			AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
			if(itemId == (*sectI).first)
			{
				sectIdx = itemIdx;
				break;
			}
		}
		if(sectIdx == QModelIndex())
			continue;	// Sector not found?

		// Process solar systems.
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
				sysI != (*sectI).second.m_Systems.end(); sysI++)
		{
			// Locate the model index for the solar system.
			QModelIndex sysIdx;
			for(int sysRow=0; sysRow<m_PlacesModel.rowCount(sectIdx); sysRow++)
			{
				// Atom id stored on column 0.
				QModelIndex itemIdx = m_PlacesModel.index(sysRow,0, sectIdx);
				AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
				if(itemId == (*sysI).first)
				{
					sysIdx = itemIdx;
					break;
				}
			}
			if(sysIdx == QModelIndex())
				continue;	// System not found?

			// Cities are only reported for systems that have been explored, btw.
			if((*sysI).second.m_Cities.size())
			{
				// Process cities.
				for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
						cityI != (*sysI).second.m_Cities.end(); cityI++)
				{
					// Locate the model index for the city.
					QModelIndex cityIdx;
					for(int cityRow=0; cityRow<m_PlacesModel.rowCount(sysIdx); cityRow++)
					{
						// Atom id stored on column 0.
						QModelIndex itemIdx = m_PlacesModel.index(cityRow,0, sysIdx);
						AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
						if(itemId == (*cityI).first)
						{
							cityIdx = itemIdx;
							break;
						}
					}
					if(cityIdx == QModelIndex())
						continue;	// City not found?

					QHash<quint32, QIcon>::const_iterator ii = empireFlagIcons.constFind((*cityI).second.m_Empire);
					if(ii == empireFlagIcons.cend())
					{
						// Flag for that empire is not in our cache yet.
						DgEmpire empire;
						empire.m_id = (*cityI).second.m_Empire;
						const DgData& gd = g_Au.LockDataForRead();
						bool hasEmpire = gd.Empire(empire);
						g_Au.UnlockData();
						if(hasEmpire)
						{
							// We have the empire record locally so we can make an icon from its flag.
							QPixmap pxflag;
							pxflag.convertFromImage(empire.m_flag);
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, QIcon(pxflag));
						}
						else
						{
							// We do not have the empire record so use the generic city icon instead of a flag.
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, cityIcon);
						}
					}

					// Update the flag icon.
					m_PlacesModel.setData(cityIdx, *ii, Qt::DecorationRole);
					if(m_StarMapGL)
						m_StarMapGL->SetPlaceIcon((*cityI).first, *ii);
					else if(m_StarMapW)
						m_StarMapW->SetPlaceIcon((*cityI).first, *ii);
				}
			}
		}
	}

	g_AuClient.UnlockStarMap();
}

QModelIndex AuLocatorStarMap::AddPlace(const QModelIndex& parentIdx, const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon,
		const QString& name, const QColor& textColor, const AuPoint& galaxyLoc, const QString& status, const AuId& sectorId,
		GALAXY galaxy, bool isCoreBlackHole, bool isEmptySystem, quint8 systemGenHab, quint32 systemGenSeed, bool systemExplored,
		bool systemScanned,
		quint32 cityEmpire, quint8 signalStrength,
		quint16 empireShips, quint16 empireStations,
		quint16 friendShips, quint16 friendStations,
		quint16 neutralShips, quint16 neutralStations,
		quint16 enemyShips, quint16 enemyStations)
{
	//if(systemGenHab || systemGenSeed)
		//qDebug() << "AuLocatorStarMap::AddPlace" << id << name << status << "sector" << sectorId << "type" << type
			//<< "sysGen" << systemGenHab << systemGenSeed;
	if(isEmptySystem)
	{
		// Add an item to the star map.
		if(m_StarMapGL)
		{
			m_StarMapGL->AddPlace(id, type, icon, name, textColor, galaxyLoc, sectorId, galaxy, isCoreBlackHole, isEmptySystem,
					systemGenHab, systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength, empireShips,
					empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
		}
		else if(m_StarMapW)
		{
			m_StarMapW->AddPlace(id, type, icon, name, textColor, galaxyLoc, sectorId, galaxy, isCoreBlackHole, isEmptySystem,
					systemGenHab, systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength, empireShips,
					empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
		}
		return QModelIndex();
	}

	int insertAt;
	if(type == DaAtom::ATOM_Avatar || type == DaAtom::ATOM_Galaxy)
		insertAt = m_PlacesModel.rowCount(parentIdx);	// Person and galaxy places are added as they come in.
	else
	{
		// Determine where to insert the new place alphabetically.
		QString upperName = name.toUpper();
		for(insertAt = 0; insertAt < m_PlacesModel.rowCount(parentIdx); insertAt++)
		{
			QModelIndex itemIdx = m_PlacesModel.index(insertAt,0, parentIdx);
			AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
			if(itemId.GetID() <= GALAXY_Count)
				continue;				// Do not insert before the person and galaxy places.
			if(upperName < m_PlacesModel.data(itemIdx).toString().toUpper())
				break;					// Break out with insertAt pointing at the item we must be inserted before.
		}
	}

	// Add a row to the model data. If we did not find one to insert before, the insertAt index will be rowCount, which gives us
	// a new row at the end.
	m_PlacesModel.insertRow(insertAt, parentIdx);

	QModelIndex itemIdx = m_PlacesModel.index(insertAt,0, parentIdx);
	m_PlacesModel.insertColumns(0,3, itemIdx);

	if(type == DaAtom::ATOM_SolarSystem)
	{
		QString cname = DaSolarSystem::CatalogName(galaxyLoc);
		if(cname != name)
			m_PlacesModel.setData(itemIdx, QString("%1\n%2").arg(name).arg(cname));
		else
			m_PlacesModel.setData(itemIdx, name);
	}
	else
		m_PlacesModel.setData(itemIdx, name);
	m_PlacesModel.setData(itemIdx, icon, Qt::DecorationRole);
	m_PlacesModel.setData(itemIdx, id.GetID(), Qt::UserRole);	// Atom id stored on column 0. GALAXY enum for galaxy entries.
																// GALAXY_Count for ATOM_Avatar entries.
	m_PlacesModel.setData(itemIdx, systemGenHab, Qt::UserRole+1);
	m_PlacesModel.setData(itemIdx, systemGenSeed, Qt::UserRole+2);
	m_PlacesModel.setData(itemIdx, galaxy, Qt::UserRole+3);
	m_PlacesModel.setData(itemIdx, isCoreBlackHole, Qt::UserRole+4);
	m_PlacesModel.setData(itemIdx, cityEmpire, Qt::UserRole+5);
	m_PlacesModel.setData(itemIdx, signalStrength, Qt::UserRole+6);
	m_PlacesModel.setData(itemIdx, systemExplored, Qt::UserRole+7);
	m_PlacesModel.setData(itemIdx, systemScanned, Qt::UserRole+8);

	QModelIndex itemIdx2 = m_PlacesModel.index(insertAt,1, parentIdx);
	QString gloc;
	if(type != DaAtom::ATOM_Avatar && type != DaAtom::ATOM_Galaxy)
		gloc = QString("%1, %2, %3").arg(galaxyLoc.x).arg(galaxyLoc.y).arg(galaxyLoc.z);
	m_PlacesModel.setData(itemIdx2, gloc);
	m_PlacesModel.setData(itemIdx2, type, Qt::UserRole);	// Atom type stored on column 1.

	QModelIndex itemIdx3 = m_PlacesModel.index(insertAt,2, parentIdx);
	m_PlacesModel.setData(itemIdx3, status);

	// Configure color of item.
	QStandardItem* item = m_PlacesModel.itemFromIndex(itemIdx);
	if(item)
		item->setForeground(QBrush(textColor));
	item = m_PlacesModel.itemFromIndex(itemIdx2);
	if(item)
		item->setForeground(QBrush(textColor));
	item = m_PlacesModel.itemFromIndex(itemIdx3);
	if(item)
		item->setForeground(QBrush(textColor));

	// Add an item to the star map.
	if(m_StarMapGL)
	{
		m_StarMapGL->AddPlace(id, type, icon, name, textColor, galaxyLoc, sectorId, galaxy, isCoreBlackHole, isEmptySystem,
				systemGenHab, systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength, empireShips, empireStations,
				friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
	}
	else if(m_StarMapW)
	{
		m_StarMapW->AddPlace(id, type, icon, name, textColor, galaxyLoc, sectorId, galaxy, isCoreBlackHole, isEmptySystem,
				systemGenHab, systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength, empireShips, empireStations,
				friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
	}

	return itemIdx;
}

void AuLocatorStarMap::EmpireDataChanged()
{
	UpdatePlaceFlags();

	if(m_RequestData)
		m_RequestData->setEnabled(true);
}

void AuLocatorStarMap::PlaceClicked(const AuId& id)
{
	// Select the appropriate place item in my tree view control.
	//qDebug() << "PlaceClicked" << id;
	if(id.IsNull())
		ClearSelection();
	else
	{
		SelectPlaceRecursive(id, QModelIndex(), QItemSelectionModel::Toggle);
		if(m_StarMapGL)
			m_StarMapGL->SetLastSelected(id);
		else if(m_StarMapW)
			m_StarMapW->SetLastSelected(id);
	}
}

bool AuLocatorStarMap::SelectPlaceRecursive(const AuId& id, const QModelIndex& parentIdx, QItemSelectionModel::SelectionFlag cmd)
{
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
		if(itemId == id)
		{
			// Found the place to select.
			//qDebug() << "SelectPlaceRecursive found it";
			m_PlacesView->selectionModel()->select(itemIdx, cmd);
			return true;
		}
		else
		{
			// Recurse.
			if(SelectPlaceRecursive(id, itemIdx, cmd))
				return true;
		}
	}
	return false;
}

void AuLocatorStarMap::ClearSelection()
{
	m_PlacesView->selectionModel()->clearSelection();
	if(m_StarMapGL)
		m_StarMapGL->ClearSelection();
	else if(m_StarMapW)
		m_StarMapW->ClearSelection();
}

void AuLocatorStarMap::RangeChanged(int range)
{
	if(isVisible())
		SetRange(range);
}

void AuLocatorStarMap::SetRange(int range)
{
	if(m_StarMapGL)
		m_StarMapGL->SetRange(range);
	else if(m_StarMapW)
		m_StarMapW->SetRange(range);
}

void AuLocatorStarMap::StarMapViewerSettings()
{
	quint32 personEmpireId = g_AuClient.StarMapEmpire();

	AuStarMapHighlightDialog dlg(personEmpireId,
			m_ShowEmpirePlaces,
			m_ShowFriendPlaces,
			m_ShowNeutralPlaces,
			m_ShowEnemyPlaces,
			m_ShowUnknownPlaces,
			m_ShowExploredPlaces,
			m_ShowDetectedPlaces,
			m_UnknownPrecedesEnemy,
			m_EmpiresIgnored,
			this);
	if(dlg.exec() == QDialog::Accepted)
	{
		m_EmpiresIgnored = dlg.IgnoredEmpires();
		m_ShowEmpirePlaces = dlg.ShowEmpire();
		m_ShowFriendPlaces = dlg.ShowFriend();
		m_ShowNeutralPlaces = dlg.ShowNeutral();
		m_ShowEnemyPlaces = dlg.ShowEnemy();
		m_ShowUnknownPlaces = dlg.ShowUnknown();
		m_ShowExploredPlaces = dlg.ShowExplored();
		m_ShowDetectedPlaces = dlg.ShowDetected();
		m_UnknownPrecedesEnemy = dlg.UnknownPrecedesEnemy();
		SaveSettings();

		if(m_Settings)
		{
			if(m_EmpiresIgnored.isEmpty() &&
					m_ShowEmpirePlaces &&
					m_ShowFriendPlaces &&
					m_ShowNeutralPlaces &&
					m_ShowEnemyPlaces &&
					m_ShowUnknownPlaces &&
					m_ShowExploredPlaces &&
					m_ShowDetectedPlaces)
				m_Settings->setChecked(false);						// Filter is clear so raise the button.
			else
				m_Settings->setChecked(true);						// Filter is active so depress the button.
		}

		HighlightPlaces();
	}
}

void AuLocatorStarMap::StarMapSectorAdded(const AuId& sectorId)
{
	//qDebug() << "AuLocatorStarMap::StarMapSectorAdded" << sectorId;
	// We could create the same icon over and over again from the same empire's flag. Cache them.
	QIcon sectorIcon(":data/RangeSector.png");
	QIcon systemIcon(":data/RangeSystem.png");
	QIcon scannedSystemIcon(":data/SolarSystemInfo.png");
	QIcon cityIcon(":data/City.png");
	QHash<quint32, QIcon> empireFlagIcons;

	// That's as much setup as we can do before locking the star map. Let's get it over with quickly.
	const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();

	// Get my empire data record.
	DgEmpire personEmpire;
	personEmpire.m_id = g_AuClient.m_StarMapEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	if(!gd.Empire(personEmpire))
		personEmpire.ChangeEmpireName(personEmpire.Name(), tr("Empire"));
	g_Au.UnlockData();

	unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.find(sectorId);
	if(sectI != starMap.m_Sectors.end())
	{
		// Locate the model index for the galaxy at the root.
		QModelIndex galIdx;
		for(int galRow=0; galRow<m_PlacesModel.rowCount(QModelIndex()); galRow++)
		{
			// GALAXY enum stored on column 0.
			QModelIndex itemIdx = m_PlacesModel.index(galRow,0, QModelIndex());
			GALAXY galaxy = (GALAXY)m_PlacesModel.data(itemIdx, Qt::UserRole).toInt();
			if(galaxy == (*sectI).second.Galaxy())
			{
				galIdx = itemIdx;
				break;
			}
		}
		if(galIdx == QModelIndex())
		{
			// Galaxy not found. Create an entry.
			galIdx = AddPlace(QModelIndex(), AuId((*sectI).second.Galaxy()), DaAtom::ATOM_Galaxy,
					QIcon(":data/RangeGalaxy.png"), GalaxyName((*sectI).second.Galaxy()), s_GalaxyColor, AuPoint(0,0), "",
					AuId(0), (*sectI).second.Galaxy(), false,false, 0,0,false,false, 0, 0);
		}

		//qDebug() << "sector" << sectI.key() << (*sectI).second.m_Name << "loc" << (*sectI).second.m_Loc;
		// Convert from parsec coord to sector coord.
		AuPoint fLoc = (*sectI).second.m_Loc/10.0;

		QString sectorName((*sectI).second.m_Name);
		if(sectorName == "Sector")
			sectorName += QString(" (%1, %2, %3)").arg(fLoc.x).arg(fLoc.y).arg(fLoc.z);

		QModelIndex sectIdx = AddPlace(galIdx, (*sectI).first, DaAtom::ATOM_Sector, sectorIcon, sectorName,
				QColor(255,255,255), (*sectI).second.m_Loc, "", AuId(0), (*sectI).second.Galaxy(), false,false, 0,0,false,false,
				0, 0);

		// Process solar systems.
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
				sysI != (*sectI).second.m_Systems.end(); sysI++)
		{
			//qDebug() << " system" << (*sysI).second.m_Name << "loc" << (*sysI).second.m_Loc;
			AuPoint systemLoc = (*sectI).second.m_Loc + (*sysI).second.m_Loc;

			quint8 genHab = 0;
			quint32 genSeed = 0;
			QIcon sysIcon(systemIcon);
			bool isScanned = (*sysI).second.IsScanned();
			if(isScanned)
				sysIcon = scannedSystemIcon;
			genHab = (*sysI).second.m_GenHab;
			genSeed = (*sysI).second.m_GenSeed;

			quint8 signalStr = (*sysI).second.DetectedSignalStrength();

			QModelIndex sysIdx;
			if((*sysI).second.IsExplored())
			{
				// System is explored.
				quint16 empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips,
						enemyStations;
				SpacecraftPresent((*sysI).second, personEmpire.m_id, empireShips, empireStations, friendShips, friendStations,
						neutralShips, neutralStations, enemyShips, enemyStations);

				sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
						(*sysI).second.m_Name, s_ExColor, systemLoc, "", (*sectI).first, (*sectI).second.Galaxy(),
						(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, true, isScanned, 0,
						signalStr, empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations,
						enemyShips, enemyStations);

				// Explored systems show their wormholes, both explored and unexplored wormholes.
				for(QList<AuStarMapWormhole>::const_iterator wi = (*sysI).second.m_Wormholes.cbegin();
						wi != (*sysI).second.m_Wormholes.cend(); wi++)
					if((*wi).Polarity() != AuWormhole::Intergalactic &&
							(*wi).Polarity() != AuWormhole::IntergalacticOneWay)
					{
						if(m_StarMapGL)
							m_StarMapGL->AddPlaceWormhole((*sysI).first, (*wi).VecToDest(), (*wi).IsExplored());
						else if(m_StarMapW)
							m_StarMapW->AddPlaceWormhole((*sysI).first, (*wi).VecToDest(), (*wi).IsExplored());
					}
			}
			else if(signalStr && m_ShowDetectedPlaces)
			{
				// Emission detected from unexplored system.
				sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
						(*sysI).second.m_Name, s_DetectedColor, systemLoc, tr(""), (*sectI).first, (*sectI).second.Galaxy(),
						(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, false, false, 0,
						signalStr);
			}
			else
			{
				// Unexplored and undetected.
				sysIdx = AddPlace(sectIdx, (*sysI).first, DaAtom::ATOM_SolarSystem, sysIcon,
						(*sysI).second.m_Name, s_BlackColor, systemLoc, tr(""), (*sectI).first, (*sectI).second.Galaxy(),
						(*sysI).second.IsCoreBlackHole(), (*sysI).second.IsEmptySystem(), genHab, genSeed, false, false, 0, 0);
			}

			// Cities are only reported for systems that have been explored, btw.
			if((*sysI).second.m_Cities.size())
			{
				// Process cities.
				for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
						cityI != (*sysI).second.m_Cities.end(); cityI++)
				{
					// What do we know about the empire who owns the city.
					QString empireName;
					POLITICAL_STANCE stance;
					QColor stanceColor;
					if(personEmpire.m_id == (*cityI).second.m_Empire)
					{
						// It is our empire.
						empireName = personEmpire.Name();
						stance = POL_Empire;
						stanceColor = PoliticalStanceColor(stance, false);
					}
					else
					{
						// It is not our empire.
						stance = personEmpire.PoliticalStance((*cityI).second.m_Empire);	// Will get default vs unknown.
						if(personEmpire.IsEmpireKnown((*cityI).second.m_Empire))
						{
							// It is an empire we have encountered so we know its name.
							empireName = personEmpire.NameOfKnownEmpire((*cityI).second.m_Empire);
							stanceColor = PoliticalStanceColor(stance, false);
						}
						else
						{
							// It is an unknown empire.
							empireName = tr("Unknown Empire");
							stanceColor = s_UnkColor;
						}
					}

					if((*cityI).second.HasAirport())
						empireName += tr(" Airport");
					else if((*cityI).second.HasAirBase())
						empireName += tr(" Air Base");

					QHash<quint32, QIcon>::const_iterator ii = empireFlagIcons.constFind((*cityI).second.m_Empire);
					if(ii == empireFlagIcons.cend())
					{
						// Flag for that empire is not in our cache yet.
						DgEmpire empire;
						empire.m_id = (*cityI).second.m_Empire;
						const DgData& gd = g_Au.LockDataForRead();
						bool hasEmpire = gd.Empire(empire);
						g_Au.UnlockData();
						if(hasEmpire)
						{
							// We have the empire record locally so we can make an icon from its flag.
							QPixmap pxflag;
							pxflag.convertFromImage(empire.m_flag);
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, QIcon(pxflag));
						}
						else
						{
							// We do not have the empire record so use the generic city icon instead of a flag.
							ii = empireFlagIcons.insert((*cityI).second.m_Empire, cityIcon);
						}
					}
					AddPlace(sysIdx, (*cityI).first, DaAtom::ATOM_Bd, *ii, (*cityI).second.m_Name,
							stanceColor, systemLoc, empireName, (*sectI).first, (*sectI).second.Galaxy(),
							false,false, 0,0,false,false, (*cityI).second.m_Empire, 0);
				}
			}
		}

		HighlightSector(sectI, sectIdx, personEmpire);
	}

	g_AuClient.UnlockStarMap();
}

void AuLocatorStarMap::StarMapErased()
{
	//qDebug() << "AuLocatorStarMap::StarMapErased";
	InitPlaces();
}

void AuLocatorStarMap::ScanSystem()
{
	AuMissionOrderScanSurveySystem order(false);
	if(!order.IsValid())
		return;
	auApp->MissionAppendOrder(order);
}

void AuLocatorStarMap::ScanTactical()
{
	AuMissionOrderScanTactical order(false);
	if(!order.IsValid())
		return;
	auApp->MissionAppendOrder(order);
}

void AuLocatorStarMap::SaveStarMapXml()
{
	return;	// XML export nerfed
	// Get my empire data record.
	DgEmpire personEmpire;
	personEmpire.m_id = g_AuClient.StarMapEmpire();
	if(!personEmpire.m_id)
		return;
	const DgData& gd = g_Au.LockDataForRead();
	bool hasEmpire = gd.Empire(personEmpire);
	g_Au.UnlockData();
	if(!hasEmpire)
		return;

	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastStarMapFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString saveFile = QFileDialog::getSaveFileName(this,
			tr("Star Map XML File"),
			lastFile + "/" + tr("%1 Star Map.xml").arg(personEmpire.Name()),
			"Star Map (*.xml);;All Files (*.*)",
			0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Store last known file folder.
	g_Au.Settings().writeEntry("/Settings/LastStarMapFile", saveFile);

	QFile file(saveFile);
	if(file.open(QFile::WriteOnly))
	{
		QXmlStreamWriter stream(&file);
		stream.setAutoFormatting(true);
		stream.writeStartDocument();

		stream.writeStartElement("starmap");
		stream.writeAttribute("empireId", QString("%1").arg(personEmpire.m_id));
		stream.writeAttribute("empire", personEmpire.Name());
		stream.writeAttribute("createDate", QDateTime::currentDateTime().toUTC().toString());

		AuStarMapData starMap = g_AuClient.LockStarMapForRead();
		g_AuClient.UnlockStarMap();

		// Make a list of the galaxies present so we can make a tier for them.
		QMap<GALAXY, bool> galaxies;
		for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
				sectI != starMap.m_Sectors.end(); sectI++)
			if(!galaxies.contains((*sectI).second.Galaxy()))
				galaxies.insert((*sectI).second.Galaxy(), true);

		QProgressDialog* prog;
		prog = new QProgressDialog(this);
		prog->setWindowTitle(tr("Exporting Star Map XML"));
		prog->setWindowModality(Qt::WindowModal);
		prog->setValue(0);
		prog->setRange(0, (int)starMap.m_Sectors.size());
		prog->setLabelText(tr("Exporting %L1 sectors to the XML file...").arg(starMap.m_Sectors.size()));

		// Process galaxies.
		bool canx = false;
		QSet<AuId> sectorsDone;
		for(QMap<GALAXY, bool>::const_iterator gi = galaxies.cbegin(); gi != galaxies.cend() && !canx; gi++)
		{
			stream.writeStartElement("galaxy");
			stream.writeAttribute("name", GalaxyName(gi.key()));

			// Process sectors.
			for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
					sectI != starMap.m_Sectors.end(); sectI++)
			{
				// Skip sectors that are not in the current galaxy tier.
				if(sectorsDone.contains((*sectI).first))
					continue;
				if((*sectI).second.Galaxy() != gi.key())
					continue;
				sectorsDone.insert((*sectI).first);
				prog->setValue(sectorsDone.size());

				// Convert from parsec coord to sector coord.
				AuPoint sectorLoc = (*sectI).second.m_Loc/10.0;

				stream.writeStartElement("sector");
				stream.writeAttribute("sectorId", (*sectI).first.StringId());
				stream.writeAttribute("name", (*sectI).second.m_Name);
				stream.writeAttribute("x", QVariant(sectorLoc.x).toString());
				stream.writeAttribute("y", QVariant(sectorLoc.y).toString());
				stream.writeAttribute("z", QVariant(sectorLoc.z).toString());
				//if(!(*sectI).second.m_Systems.isEmpty())
					//stream.writeAttribute("systemCount", QVariant((*sectI).second.m_Systems.size()).toString());

				// Process solar systems.
				for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.begin();
						sysI != (*sectI).second.m_Systems.end(); sysI++)
				{
					AuPoint systemLoc = (*sectI).second.m_Loc + (*sysI).second.m_Loc;
					//systemLoc.Round(.1);

					QString eod;
					int scanEx;
					if((*sysI).second.IsScanned())
					{
						eod = "Surveyed";
						scanEx = 2;
					}
					else if((*sysI).second.IsExplored())
					{
						eod = "Explored";
						scanEx = 1;
					//Ceruleas else if((*sysI).second.DetectedSignalStrength())
						//Ceruleas eod = "Detected";
					}
					else
					{
						scanEx = 0;
						eod = "Unexplored";
					}
					//Ceruleas QString sig;
					//Ceruleas if((*sysI).second.DetectedSignalStrength())
						//Ceruleas sig = QString("%1").arg((*sysI).second.DetectedSignalStrength());	// 1-15

					QString catname(DaSolarSystem::CatalogName(systemLoc));

					stream.writeStartElement("system");
					stream.writeAttribute("systemId", (*sysI).first.StringId());
					if((*sysI).second.IsEmptySystem())
					{
						stream.writeAttribute("name", "Deep Space");
					}
					else
					{
						stream.writeAttribute("name", (*sysI).second.m_Name);
						if(catname != (*sysI).second.m_Name)
							stream.writeAttribute("catalog", catname);
						stream.writeAttribute("x", QString("%1").arg(systemLoc.x, 0,'f',1));
						stream.writeAttribute("y", QString("%1").arg(systemLoc.y, 0,'f',1));
						stream.writeAttribute("z", QString("%1").arg(systemLoc.z, 0,'f',1));
					}
					stream.writeAttribute("eod", eod);
					//Ceruleas if(!sig.isEmpty())
						//Ceruleas stream.writeAttribute("signal", sig);

					if(scanEx)
					{
						//if(!(*sysI).second.m_Wormholes.isEmpty())
							//stream.writeAttribute("wormholeCount", QVariant((*sysI).second.m_Wormholes.count()).toString());
						//if(!(*sysI).second.m_Cities.isEmpty())
							//stream.writeAttribute("cityCount", QVariant((*sysI).second.m_Cities.size()).toString());

						if(!(*sysI).second.IsEmptySystem())
						{
							// Process wormholes.
							for(QList<AuStarMapWormhole>::const_iterator wi = (*sysI).second.m_Wormholes.cbegin();
									wi != (*sysI).second.m_Wormholes.cend(); wi++)
							{
								AuPoint destLoc = systemLoc + (*wi).VecToDest();

								// Try to find the destination system id.
								AuId destId;
								for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator subSectI =
										starMap.m_Sectors.begin();
										subSectI != starMap.m_Sectors.end(); subSectI++)
								{
									// Skip sectors that are not in the current galaxy tier.
									if((*subSectI).second.Galaxy() != gi.key())
										continue;
									for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator subSysI =
											(*subSectI).second.m_Systems.begin();
											subSysI != (*subSectI).second.m_Systems.end(); subSysI++)
									{
										AuPoint systemLoc = (*subSectI).second.m_Loc + (*subSysI).second.m_Loc;
										if(systemLoc.RoundedFuzzyEquals(destLoc, .001))
										{
											destId = (*subSysI).first;
											break;
										}
									}
									if(!destId.IsNull())
										break;
								}
								/*
								if(destId.IsNull())
								{
									// Look harder. Find the closest destination system.
									double closestDist2 = 0;
									for(QHash<AuId, AuStarMapSector>::const_iterator subSectI = starMap.m_Sectors.cbegin();
											subSectI != starMap.m_Sectors.cend(); subSectI++)
									{
										// Skip sectors that are not in the current galaxy tier.
										if((*subSectI).Galaxy() != gi.key())
											continue;
										for(QHash<AuId, AuStarMapSystem>::const_iterator subSysI = (*subSectI).m_Systems.cbegin();
												subSysI != (*subSectI).m_Systems.cend(); subSysI++)
										{
											AuPoint systemLoc = (*subSectI).m_Loc + (*subSysI).m_Loc;
											double dist2 = systemLoc.Distance2(destLoc);
											if(dist2 >= 4)
												continue;
											if(destId.IsNull() || dist2 < closestDist2)
											{
												destId = subSysI.key();
												closestDist2 = dist2;
											}
										}
									}
								}
								*/

								stream.writeStartElement("wormhole");
								stream.writeAttribute("destX", QString("%1").arg(destLoc.x, 0,'f',1));
								stream.writeAttribute("destY", QString("%1").arg(destLoc.y, 0,'f',1));
								stream.writeAttribute("destZ", QString("%1").arg(destLoc.z, 0,'f',1));
								if(!destId.IsNull())
									stream.writeAttribute("destSystemId", destId.StringId());
								if((*wi).IsExplored())
									stream.writeAttribute("explored", tr("Yes"));
								switch((*wi).Polarity())
								{
								case AuWormhole::Invalid:
								default:
									break;
								case AuWormhole::Positive:
									stream.writeAttribute("polarity", tr("positive"));
									break;
								case AuWormhole::Negative:
									stream.writeAttribute("polarity", tr("negative"));
									break;
								case AuWormhole::Intergalactic:
								case AuWormhole::IntergalacticOneWay:
									stream.writeAttribute("polarity", tr("neutral"));
									break;
								}
								stream.writeEndElement(); // wormhole
							}
						}

						// Process spacecraft.
						for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator scI =
								(*sysI).second.m_Spacecraft.begin(); scI != (*sysI).second.m_Spacecraft.end(); scI++)
						{
							// What do we know about the empire who owns the city.
							QString empireName;
							POLITICAL_STANCE stance;
							QString stanceName;
							if(personEmpire.m_id == (*scI).second.m_Empire)
							{
								// It is our empire.
								empireName = personEmpire.Name();
								stanceName = tr("Empire");
							}
							else
							{
								// It is not our empire.
								stance = personEmpire.PoliticalStance((*scI).second.m_Empire);	// Will get default vs unknown.
								if(personEmpire.IsEmpireKnown((*scI).second.m_Empire))
								{
									// It is an empire we have encountered so we know its name.
									empireName = personEmpire.NameOfKnownEmpire((*scI).second.m_Empire);
									switch(stance)
									{
									case POL_Friend:
										stanceName = tr("Friend");
										break;
									case POL_Neutral:
										stanceName = tr("Neutral");
										break;
									case POL_Enemy:
										stanceName = tr("Enemy");
										break;
									case POL_Empire:
										stanceName = tr("Empire");
										break;
									case POL_Vassal:
										stanceName = tr("Vassal");
										break;
									}
								}
							}

							QString fc;
							if((*scI).second.IsCompany())
								fc = tr("Company");
							else
								fc = tr("Fleet");

							QString ss;
							if((*scI).second.IsSpaceStation())
								ss = SpacecraftStructureName(SC_Station);
							else if((*scI).second.HasWarpDrive() || (*scI).second.HasWormholeDrive())
								ss = SpacecraftStructureName(SC_Starship);
							else
								ss = SpacecraftStructureName(SC_Spaceship);

							QString alert;
							switch((*scI).second.AlertCondition())
							{
							case ConditionGreen:
								alert = "Green";
								break;
							case ConditionYellow:
								alert = "Yellow";
								break;
							case ConditionRed:
								alert = "Red";
								break;
							}

							QString space;
							if((*scI).second.IsInAtmosphere())
								space = tr("atmosphere");
							else if((*scI).second.IsOrbiting())
								space = tr("orbit");
							else if((*scI).second.IsInSystem())
								space = tr("system");
							else
								space = tr("deep");

							QString warpDrive;
							if((*scI).second.HasWarpDrive())
							{
								if((*scI).second.IsWarpFTLHot())
									warpDrive = "hot";
								else
									warpDrive = "yes";
							}
							else
								warpDrive = "none";

							QString wormholeDrive;
							if((*scI).second.HasWormholeDrive())
							{
								if((*scI).second.IsWormholeFTLHot())
									wormholeDrive = "hot";
								else
									wormholeDrive = "yes";
							}
							else
								wormholeDrive = "none";

							QString health = QString("%1").arg((*scI).second.Health());	// 0 to 255

							stream.writeStartElement("spacecraft");
							stream.writeAttribute("spacecraftId", (*scI).first.StringId());
							stream.writeAttribute("name", (*scI).second.Name());
							stream.writeAttribute("ownership", fc);
							stream.writeAttribute("structure", ss);
							if(!empireName.isEmpty())
							{
								stream.writeAttribute("empire", empireName);
								stream.writeAttribute("empireId", QString("%1").arg((*scI).second.m_Empire));
							}
							if(!stanceName.isEmpty())
								stream.writeAttribute("stance", stanceName);
							stream.writeAttribute("condition", alert);
							stream.writeAttribute("health", health);
							stream.writeAttribute("moving", (*scI).second.IsMoving() ? "yes" : "no");
							stream.writeAttribute("space", space);
							stream.writeAttribute("sensors", (*scI).second.HasSensors() ? "yes" : "no");
							stream.writeAttribute("warpDrive", warpDrive);
							stream.writeAttribute("wormholeDrive", wormholeDrive);
							stream.writeAttribute("captain", (*scI).second.HasCaptain() ? "yes" : "no");
							stream.writeAttribute("officer", (*scI).second.HasOfficer() ? "yes" : "no");
							stream.writeAttribute("new", (*scI).second.IsNew() ? "yes" : "no");
							stream.writeEndElement(); // spacecraft
						}
					}

					if(!(*sysI).second.IsEmptySystem())
					{
						if(scanEx)
						{
							// Process cities.
							for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI =(*sysI).second.m_Cities.begin();
									cityI != (*sysI).second.m_Cities.end(); cityI++)
							{
								// What do we know about the empire who owns the city.
								QString empireName;
								POLITICAL_STANCE stance;
								QString stanceName;
								if(personEmpire.m_id == (*cityI).second.m_Empire)
								{
									// It is our empire.
									empireName = personEmpire.Name();
									stanceName = tr("Empire");
								}
								else
								{
									// It is not our empire.
									stance = personEmpire.PoliticalStance((*cityI).second.m_Empire);	// Will get default vs unk.
									if(personEmpire.IsEmpireKnown((*cityI).second.m_Empire))
									{
										// It is an empire we have encountered so we know its name.
										empireName = personEmpire.NameOfKnownEmpire((*cityI).second.m_Empire);
										switch(stance)
										{
										case POL_Friend:
											stanceName = tr("Friend");
											break;
										case POL_Neutral:
											stanceName = tr("Neutral");
											break;
										case POL_Enemy:
											stanceName = tr("Enemy");
											break;
										case POL_Empire:
											stanceName = tr("Empire");
											break;
										case POL_Vassal:
											stanceName = tr("Vassal");
											break;
										}
									}
								}

								QString capital;
								if((*cityI).second.HasEmpireCapital() &&
										(*cityI).second.HasGalaxyCapital() &&
										(*cityI).second.HasSectorCapital())
									capital = tr("Empire, Galaxy, Sector");
								else if((*cityI).second.HasEmpireCapital() && (*cityI).second.HasGalaxyCapital())
									capital = tr("Empire, Galaxy");
								else if((*cityI).second.HasEmpireCapital() && (*cityI).second.HasSectorCapital())
									capital = tr("Empire, Sector");
								else if((*cityI).second.HasGalaxyCapital() && (*cityI).second.HasSectorCapital())
									capital = tr("Galaxy, Sector");
								else if((*cityI).second.HasEmpireCapital())
									capital = tr("Empire");
								else if((*cityI).second.HasGalaxyCapital())
									capital = tr("Galaxy");
								else if((*cityI).second.HasSectorCapital())
									capital = tr("Sector");

								QString airport;
								if((*cityI).second.HasAirport())
									airport = tr("Airport Terminal");
								else if((*cityI).second.HasAirBase())
									airport = tr("Military Airport Terminal");

								QString admin;
								if((*cityI).second.HasCapitolBuilding())
									admin = tr("Capitol");
								else if((*cityI).second.HasBaseHeadquarters())
									admin = tr("Military Headquarters");

								stream.writeStartElement("city");
								stream.writeAttribute("cityId", (*cityI).first.StringId());
								stream.writeAttribute("name", (*cityI).second.m_Name);
								if(!empireName.isEmpty())
								{
									stream.writeAttribute("empire", empireName);
									stream.writeAttribute("empireId", QString("%1").arg((*cityI).second.m_Empire));
								}
								if(!stanceName.isEmpty())
									stream.writeAttribute("stance", stanceName);
								if(!capital.isEmpty())
									stream.writeAttribute("capital", capital);
								if(!admin.isEmpty())
									stream.writeAttribute("admin", admin);
								if(!airport.isEmpty())
									stream.writeAttribute("airport", airport);

								stream.writeEndElement(); // city
							}
						}

						// Export scan data.
						AuEther ether("AuLocatorStarMap::SaveStarMapXml");
						DaSolarSystem* system = new DaSolarSystem;
						system->SetId((*sysI).first);
						ether.AddAtom(system);

						QString sysname;
						int nl = (*sysI).second.m_Name.indexOf("\n");
						if(nl >= 0)
							sysname = (*sysI).second.m_Name.left(nl);
						else
							sysname = (*sysI).second.m_Name;
						AuMatrix4x4 systemT;
						systemT.Translate(systemLoc);
						system->PrepareForResourceScan(catname, sysname, gi.key(), (HABITABILITY)(*sysI).second.m_GenHab,
								(*sysI).second.m_GenSeed, (*sysI).second.IsCoreBlackHole(), systemT, -1);
						DaStar* primary = system->FindPrimaryStar();
						if(primary)
						{
							DaAtomList stars;
							stars.append(primary);
							primary->ListStars(stars);
							for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
							{
								// Report the star and its resources.
								(*si)->SystemScanReportXML(stream, scanEx);
								if(!scanEx)
									continue;		// Skip planets when not explored or scanned.

								DaAtomList planets;
								(*si)->FindPlanetsRecursive(planets);
								for(DaAtomList::const_iterator pi = planets.begin(); pi != planets.end(); pi++)
								{
									if((*pi)->FindParentStarInEnv() != (DaStar*)(*si))
										continue;	// Skip planets around companion stars.

									// Report the planet and its resources.
									(*pi)->SystemScanReportXML(stream, scanEx);
								}
							}
						}
					}
					stream.writeEndElement(); // system
				}
				stream.writeEndElement(); // sector

				// Test for cancel button on progress dialog.
				auApp->processEvents();
				if(prog->wasCanceled())
				{
					canx = true;
					break;
				}
			}
			stream.writeEndElement(); // galaxy
		}
		stream.writeEndElement(); // empire
		stream.writeEndDocument();
		delete prog;
	}
}

void AuLocatorStarMap::ReportCityHistory(const AuId& id)
{
	auApp->ReportCityHistory(id);
}

void AuLocatorStarMap::ReportCityLast(const AuId& id)
{
	auApp->ReportCityLast(id);
}

void AuLocatorStarMap::SelectPlaces(const AuIdList& ids)
{
	for(AuIdList::const_iterator pi = ids.cbegin(); pi != ids.cend(); pi++)
		SelectPlace(*pi);
}

void AuLocatorStarMap::SelectPlace(const AuId& id)
{
	SelectPlaceRecursive(id, QModelIndex(), QItemSelectionModel::Select);
}

AuIdList AuLocatorStarMap::ListExpanded() const
{
	AuIdList expanded;
	ListExpandedRecursive(expanded, QModelIndex());
	return expanded;
}

void AuLocatorStarMap::ListExpandedRecursive(AuIdList& ids, const QModelIndex& parentIdx) const
{
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		if(m_PlacesView->isExpanded(itemIdx))
			ids.append(AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt()));

		// Recurse.
		ListExpandedRecursive(ids, itemIdx);
	}
}

void AuLocatorStarMap::ExpandPlaces(const AuIdList& ids) const
{
	for(AuIdList::const_iterator pi = ids.cbegin(); pi != ids.cend(); pi++)
		ExpandPlaceRecursive(*pi, QModelIndex());
}

bool AuLocatorStarMap::ExpandPlaceRecursive(const AuId& id, const QModelIndex& parentIdx) const
{
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
		if(itemId == id)
		{
			// Found the place to select.
			m_PlacesView->setExpanded(itemIdx, true);
			return true;
		}
		else
		{
			// Recurse.
			if(ExpandPlaceRecursive(id, itemIdx))
				return true;
		}
	}
	return false;
}

void AuLocatorStarMap::StarMapShowCities(const AuIdList& cityIds)
{
	ClearSelection();
	SelectPlacesTree(cityIds);
	if(m_StarMapGL)
		m_StarMapGL->LookAtSelection();
	else if(m_StarMapW)
		m_StarMapW->LookAtSelection();
}

void AuLocatorStarMap::SelectPlacesTree(const AuIdList& ids)
{
	for(AuIdList::const_iterator pi = ids.cbegin(); pi != ids.cend(); pi++)
		SelectPlaceTree(*pi);
}

void AuLocatorStarMap::SelectPlaceTree(const AuId& id)
{
	SelectPlaceTreeRecursive(id, QModelIndex(), QItemSelectionModel::Select);
}

bool AuLocatorStarMap::SelectPlaceTreeRecursive(const AuId& id, const QModelIndex& parentIdx,
		QItemSelectionModel::SelectionFlag cmd)
{
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());
		if(itemId == id)
		{
			// Found the place to select.
			//qDebug() << "SelectPlaceTreeRecursive found it";
			m_PlacesView->selectionModel()->select(itemIdx, cmd);
			m_PlacesView->setExpanded(itemIdx, true);
			return true;
		}
		else
		{
			// Recurse.
			if(SelectPlaceTreeRecursive(id, itemIdx, cmd))
			{
				// Expand parent of selected item.
				m_PlacesView->setExpanded(itemIdx, true);

				// Select parent solar system of selected item.
				QModelIndex itemIdx2 = m_PlacesModel.index(row, 1, parentIdx);
				quint8 type = m_PlacesModel.data(itemIdx2, Qt::UserRole).toUInt();
				if(type == DaAtom::ATOM_SolarSystem)
					m_PlacesView->selectionModel()->select(itemIdx, cmd);
				return true;
			}
			//else
				//m_PlacesView->setExpanded(itemIdx, false);	// This does not work when multiple cities are selected.
		}
	}
	return false;
}

void AuLocatorStarMap::StarMapShowGalaxyLocation(const QString& galaxyName, const AuPoint& loc)
{
	ClearSelection();

	// Instead of an atom id, this tree actually stores the GALAXY enum on galaxies. We have to find it by name.
	AuId galaxyId;
	QModelIndex parentIdx;	// Null is the root.
	for(int row=0; row<m_PlacesModel.rowCount(parentIdx); row++)
	{
		QModelIndex itemIdx = m_PlacesModel.index(row, 0, parentIdx);
		QString itemName = m_PlacesModel.data(itemIdx).toString();
		AuId itemId = AuId(m_PlacesModel.data(itemIdx, Qt::UserRole).toUInt());	// Item id is GALAXY for galaxy items, not AtomId.

		// item type
		QModelIndex itemIdx2 = m_PlacesModel.index(row,1, parentIdx);
		quint8 type = m_PlacesModel.data(itemIdx2, Qt::UserRole).toUInt();

		if(type == DaAtom::ATOM_Galaxy && itemName == galaxyName)
		{
			// Found the galaxy.
			galaxyId = itemId;
			break;
		}
	}

	SelectPlace(galaxyId);
	if(m_StarMapGL)
	{
		m_StarMapGL->SetLastSelected(galaxyId);
		m_StarMapGL->SetLookPoint(loc);
	}
	else if(m_StarMapW)
	{
		m_StarMapW->SetLastSelected(galaxyId);
		m_StarMapW->SetLookPoint(loc);
	}
}

void AuLocatorStarMap::SpacecraftPresent(const AuStarMapSystem& sys, quint32 empireId, quint16& empireShips,
		quint16& empireStations, quint16& friendShips, quint16& friendStations, quint16& neutralShips, quint16& neutralStations,
		quint16& enemyShips, quint16& enemyStations)
{
	empireShips = 0;
	empireStations = 0;
	friendShips = 0;
	friendStations = 0;
	neutralShips = 0;
	neutralStations = 0;
	enemyShips = 0;
	enemyStations = 0;
	for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator si = sys.m_Spacecraft.begin();
			si != sys.m_Spacecraft.end(); si++)
	{
		POLITICAL_STANCE stance = PoliticalStance(empireId, (*si).second.EmpireId());
		switch(stance)
		{
		case POL_Friend:
			if((*si).second.IsSpaceStation())
				friendStations++;
			else
				friendShips++;
			break;
		case POL_Neutral:
			if((*si).second.IsSpaceStation())
				neutralStations++;
			else
				neutralShips++;
			break;
		case POL_Enemy:
			if((*si).second.IsSpaceStation())
				enemyStations++;
			else
				enemyShips++;
			break;
		case POL_Empire:
		case POL_Vassal:
			if((*si).second.IsSpaceStation())
				empireStations++;
			else
				empireShips++;
			break;
		}
	}
}

void AuLocatorStarMap::Search()
{
	if(!m_SearchDlg)
	{
		m_SearchDlg = new AuStarMapSearch(this);
		connect(m_SearchDlg, SIGNAL(PlaceClicked(AuId)), SLOT(SearchPlace(AuId)));
	}
	m_SearchDlg->show();
}

void AuLocatorStarMap::SearchSystems()
{
	if(!m_SearchSystemsDlg)
	{
		m_SearchSystemsDlg = new AuStarMapSearchSystems(this);
		connect(m_SearchSystemsDlg, SIGNAL(SystemClicked(AuId)), SLOT(SearchPlace(AuId)));
	}
	m_SearchSystemsDlg->ClearLocationReference();
	m_SearchSystemsDlg->show();
}

void AuLocatorStarMap::SearchSystems(int galaxy, const AuPoint& galaxyLoc)
{
	if(!m_SearchSystemsDlg)
	{
		m_SearchSystemsDlg = new AuStarMapSearchSystems(this);
		connect(m_SearchSystemsDlg, SIGNAL(SystemClicked(AuId)), SLOT(SearchPlace(AuId)));
	}
	m_SearchSystemsDlg->SetLocationReference(galaxy, galaxyLoc);
	m_SearchSystemsDlg->show();
}

void AuLocatorStarMap::SearchPlace(AuId placeId)
{
	//qDebug() << "AuLocatorStarMap::SearchPlace" << placeId;
	SelectPlaceTree(placeId);
	if(m_StarMapGL)
	{
		m_StarMapGL->SetLastSelected(placeId);
		m_StarMapGL->LookAtLastSelected();
	}
	else if(m_StarMapW)
	{
		m_StarMapW->SetLastSelected(placeId);
		m_StarMapW->LookAtLastSelected();
	}
}

void AuLocatorStarMap::DomainAdd(AuId systemId, QString systemName)
{
	auApp->LandAddSystem(systemId, systemName);
}

