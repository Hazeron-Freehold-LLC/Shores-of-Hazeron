//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerbar2.cpp,v 1.7 2022/02/19 18:25:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerbar2.h"
//
#include "auclientglobal.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "dadesignbase.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>

AuDesignerBar2::AuDesignerBar2(QWidget* parent) : QToolBar(parent)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = pFrame->ViewWidget();

	setObjectName("DesignerHelperBar");
	setWindowTitle(tr("Designer Helper Bar"));

	int dockSides = g_Au.Settings().readNumEntry("Dock/DesignerHelperBarAreas", (int)allowedAreas());
	setAllowedAreas((Qt::ToolBarAreas)dockSides);

	m_AnchorDesc = new QLabel(this);
	m_AnchorDesc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_FindEdges = new QAction(QIcon(":data/DesignerSelEdges.png"), tr("Find Edges"), this);
	m_FindEdges->setToolTip(tr("<html>"
				"<b>Find Edges</b><br>"
				"When checked, edges can be selected in face mode.<br>"
				"When unchecked, edges cannot be selected in face mode. "
				"</html>"));
	m_FindEdges->setCheckable(true);
	m_FindEdges->setChecked(g_AuClient.DesignerFindEdges());
	connect(m_FindEdges, SIGNAL(triggered()), SLOT(FindEdges()));

	m_FindFaces = new QAction(QIcon(":data/DesignerSelFaces.png"), tr("Find Faces"), this);
	m_FindFaces->setToolTip(tr("<html>"
				"<b>Find Faces</b><br>"
				"When checked, faces can be selected in face mode.<br>"
				"When unchecked, faces cannot be selected in face mode. "
				"</html>"));
	m_FindFaces->setCheckable(true);
	m_FindFaces->setChecked(g_AuClient.DesignerFindFaces());
	connect(m_FindFaces, SIGNAL(triggered()), SLOT(FindFaces()));

	m_FindVertices = new QAction(QIcon(":data/DesignerSelVertices.png"), tr("Find Vertices"), this);
	m_FindVertices->setToolTip(tr("<html>"
				"<b>Find Vertices</b><br>"
				"When checked, vertices can be selected in face mode.<br>"
				"When unchecked, vertices cannot be selected in face mode. "
				"</html>"));
	m_FindVertices->setCheckable(true);
	m_FindVertices->setChecked(g_AuClient.DesignerFindVertices());
	connect(m_FindVertices, SIGNAL(triggered()), SLOT(FindVertices()));

	m_ShowGrid = new QAction(QIcon(":data/dp_ShowDesignPlane.png"), tr("Show Grid"), this);
	m_ShowGrid->setToolTip(tr("<html>"
				"<b>Show Grid</b><br>"
				"When checked, the grid is visible.<br>"
				"When unchecked, the grid is not visible. "
				"Rounding grid continues to function when not visible."
				"</html>"));
	m_ShowGrid->setCheckable(true);
	m_ShowGrid->setChecked(g_AuClient.DesignerShowGrid());
	connect(m_ShowGrid, SIGNAL(triggered()), SLOT(ShowGrid()));

	m_ShowBacks = new QAction(QIcon(":data/dp_ShowBacks.png"), tr("Show Back Faces"), this);
	m_ShowBacks->setToolTip(tr("<html>"
				"<b>Show Back Faces</b><br>"
				"When checked, back sides of solid faces of the model are visible. "
				"In the game, back sides of solid faces are not visible.<br>"
				"When unchecked, back faces of the model are not visible."
				"</html>"));
	m_ShowBacks->setCheckable(true);
	m_ShowBacks->setChecked(g_AuClient.DesignerShowBacks());
	connect(m_ShowBacks, SIGNAL(triggered()), SLOT(ShowBacks()));

	m_ShowEdges = new QAction(QIcon(":data/dp_ShowEdges.png"), tr("Show Edges"), this);
	m_ShowEdges->setToolTip(tr("<html>"
				"<b>Show Edges</b><br>"
				"When checked, edge lines of the model are visible.<br>"
				"When unchecked, edge lines of the model are not visible. "
				"In the game, edge lines are not visible."
				"</html>"));
	m_ShowEdges->setCheckable(true);
	m_ShowEdges->setChecked(g_AuClient.DesignerShowEdges());
	connect(m_ShowEdges, SIGNAL(triggered()), SLOT(ShowEdges()));

	m_ShowFaces = new QAction(QIcon(":data/dp_ShowFaces.png"), tr("Show Faces"), this);
	m_ShowFaces->setToolTip(tr("<html>"
				"<b>Show Faces</b><br>"
				"When checked, solid faces of the model are visible. "
				"In the game, only solid faces are visible.<br>"
				"When unchecked, solid faces of the model are not visible."
				"</html>"));
	m_ShowFaces->setCheckable(true);
	m_ShowFaces->setChecked(g_AuClient.DesignerShowFaces());
	connect(m_ShowFaces, SIGNAL(triggered()), SLOT(ShowFaces()));

	m_GridFollowsWork = new QAction(QIcon(":data/DesignPlaneFollow.png"), tr("Grid Follows Work"), this);
	m_GridFollowsWork->setToolTip(tr("<html>"
				"<b>Grid Follows Work</b><br>"
				"When checked, the grid moves to each new coordinate entered.<br>"
				"When unchecked, the grid does not move when coordinates are entered."
				"</html>"));
	m_GridFollowsWork->setCheckable(true);
	m_GridFollowsWork->setChecked(g_AuClient.DesignerGridFollowsWork());
	connect(m_GridFollowsWork, SIGNAL(triggered()), SLOT(GridFollowsWork()));

	m_GridIncs = new QComboBox(this);
	m_GridIncs->addItem("1mm", .001);
	m_GridIncs->addItem("1cm", .01);
	m_GridIncs->addItem(".1m", .1);
	m_GridIncs->addItem("1m", 1.);
	m_GridIncs->addItem("10m", 10.);
	m_GridIncs->addItem("1/16\"", (1./(12. * 16.)) / FeetPerMeter());
	m_GridIncs->addItem("1/8\"", (1./(12. * 8.)) / FeetPerMeter());
	m_GridIncs->addItem("1/4\"", (1./(12. * 4.)) / FeetPerMeter());
	m_GridIncs->addItem("1/2\"", (1./(12. * 2.)) / FeetPerMeter());
	m_GridIncs->addItem("1\"", (1./12.) / FeetPerMeter());
	m_GridIncs->addItem("1'", (1.) / FeetPerMeter());
	m_GridIncs->addItem("1yd", (1. * 3.) / FeetPerMeter());
	m_GridIncs->addItem("10yd", (10. * 3.) / FeetPerMeter());
	int inc = m_GridIncs->findData(g_AuClient.DesignerGridSpacing());
	m_GridIncs->setCurrentIndex(inc);
	m_GridIncs->setToolTip(tr("<html>"
				"<b>Grid Spacing</b><br>"
				"Adjusts the spacing of the grid lines."
				"</html>"));
	connect(m_GridIncs, SIGNAL(currentIndexChanged(int)), SLOT(GridSpacingChanged(int)));

	m_SnapToEdges = new QAction(QIcon(":data/DesignerSnapToEdges.png"), tr("Snap to Edges"), this);
	m_SnapToEdges->setToolTip(tr("<html>"
				"<b>Snap to Edges</b><br>"
				"Edge snapping helps find edges of existing parts. "
				"Snapping is used when drawing or dragging parts.<br>"
				"%1 When checked, the cursor finds edges of parts.<br>"
				"%1 When unchecked, the cursor does not find edges of parts."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_SnapToEdges->setCheckable(true);
	m_SnapToEdges->setChecked(g_AuClient.DesignerSnapToEdges());
	connect(m_SnapToEdges, SIGNAL(triggered()), SLOT(SnapToEdges()));

	m_SnapToVertices = new QAction(QIcon(":data/DesignerSnapToVertices.png"), tr("Snap to Vertices"), this);
	m_SnapToVertices->setToolTip(tr("<html>"
				"<b>Snap to Vertices</b><br>"
				"Vertex snapping helps find vertices of existing parts. "
				"Snapping is used when drawing or dragging parts.<br>"
				"%1 When checked, the cursor finds vertices of parts.<br>"
				"%1 When unchecked, the cursor does not find vertices of parts.<br>"
				"%1 <b>Affects the grab point when dragging parts.</b>"
				"</html>").arg(g_Au.MiddleDotSymbol()));
	m_SnapToVertices->setCheckable(true);
	m_SnapToVertices->setChecked(g_AuClient.DesignerSnapToVertices());
	connect(m_SnapToVertices, SIGNAL(triggered()), SLOT(SnapToVertices()));

	m_SnapToGrid = new QAction(QIcon(":data/DesignerSnapToGrid.png"), tr("Snap to Grid"), this);
	m_SnapToGrid->setToolTip(tr("<html>"
				"<b>Snap to Grid</b><br>"
				"When checked, the cursor snaps to the grid.<br>"
				"When unchecked, the cursor does not snap to the grid.<br>"
				"Grid snapping does not depend upon the visibility of the grid."
				"</html>"));
	m_SnapToGrid->setCheckable(true);
	m_SnapToGrid->setChecked(g_AuClient.DesignerSnapToGrid());
	connect(m_SnapToGrid, SIGNAL(triggered()), SLOT(SnapToGrid()));

	m_BlipCoord = new QLineEdit();
	m_BlipCoord->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BlipCoord->setMinimumWidth(100);
	m_BlipCoord->setMaximumWidth(200);
	m_BlipCoord->setReadOnly(true);
	m_BlipCoord->setToolTip(tr("<html>"
				"When designing, shows the offset of the cursor from the center of the grid.<br>"
				"When dragging, shows the offset of the drag."
				"</html>"));

	m_MoveGrid = new QAction(QIcon(":data/dp_PivotPoint.png"), tr("Move Grid"), this);
	m_MoveGrid->setToolTip(tr("<html>"
				"<b>Move Grid</b><br>"
				"Moves the center of the grid to a new location. "
				"</html>"));
	connect(m_MoveGrid, SIGNAL(triggered()), view, SLOT(DesignerGridMove()));

	m_GridXY = new QAction(QIcon(":data/ScGridXY.png"), tr("XY Grid"), this);
	m_GridXY->setToolTip(tr("<html>"
				"<b>XY Grid</b><br>"
				"Rotates the grid so the positive Z axis goes straight up. "
				"</html>"));
	connect(m_GridXY, SIGNAL(triggered()), view, SLOT(DesignerGridXY()));

	m_GridXZ = new QAction(QIcon(":data/ScGridXZ.png"), tr("XZ Grid"), this);
	m_GridXZ->setToolTip(tr("<html>"
				"<b>XZ Grid</b><br>"
				"Rotates the grid so the positive Y axis goes straight up. "
				"</html>"));
	connect(m_GridXZ, SIGNAL(triggered()), view, SLOT(DesignerGridXZ()));

	m_GridYZ = new QAction(QIcon(":data/ScGridYZ.png"), tr("YZ Grid"), this);
	m_GridYZ->setToolTip(tr("<html>"
				"<b>YZ Grid</b><br>"
				"Rotates the grid so the positive X axis goes straight up. "
				"</html>"));
	connect(m_GridYZ, SIGNAL(triggered()), view, SLOT(DesignerGridYZ()));

	m_PositionGrid = new QAction(QIcon(":data/DesignGrid3pt.png"), tr("Position Grid"), this);
	m_PositionGrid->setToolTip(tr("<html>"
				"<b>Position Grid</b><br>"
				"Positions the grid by entering the center point, X axis point, and a point in the plane. "
				"</html>"));
	connect(m_PositionGrid, SIGNAL(triggered()), view, SLOT(DesignerGrid3pt()));

	m_ObjectMode = new QAction(QIcon(":data/ObjectMode.png"), tr("Face Mode"), this);
	m_ObjectMode->setToolTip(tr("<html>"
				"<b>Face Mode</b><br>"
				"When checked, editing operations affect parts inside of the selected mesh.<br>"
				"When unchecked, editing operations affect whole meshes that are selected."
				"<br>"
				"<b>Face mode is unavailable if any part in the selection is sealed and you are not its architect.</b>"
				"</html>"));
	m_ObjectMode->setCheckable(true);
	m_ObjectMode->setChecked(false);
	m_ObjectMode->setEnabled(false);
	connect(m_ObjectMode, SIGNAL(triggered()), SLOT(ObjectMode()));

	m_ObjectIsolate = new QAction(QIcon(":data/ScIsolate.png"), tr("Isolate Parts"), this);
	m_ObjectIsolate->setToolTip(tr("<html>"
				"<b>Isolate Parts</b><br>"
				"When checked, face mode only displays the parts that are being edited.<br>"
				"When unchecked, all parts are displayed while working in face mode."
				"</html>"));
	m_ObjectIsolate->setCheckable(true);
	m_ObjectIsolate->setChecked(false);
	m_ObjectIsolate->setEnabled(false);
	connect(m_ObjectIsolate, SIGNAL(triggered()), SLOT(ObjectIsolate()));

	addAction(m_ObjectMode);
	addAction(m_ObjectIsolate);
	addAction(m_FindVertices);
	addAction(m_FindEdges);
	addAction(m_FindFaces);
	addSeparator();
	addAction(m_ShowEdges);
	addAction(m_ShowFaces);
	addAction(m_ShowBacks);
	addSeparator();
	addAction(m_MoveGrid);
	addAction(m_GridXY);
	addAction(m_GridXZ);
	addAction(m_GridYZ);
	addAction(m_PositionGrid);
	addWidget(m_GridIncs);
	addAction(m_ShowGrid);
	addAction(m_GridFollowsWork);
	addAction(m_SnapToGrid);
	addSeparator();
	addAction(m_SnapToEdges);
	addAction(m_SnapToVertices);
	addWidget(m_BlipCoord);
	addSeparator();
	addWidget(m_AnchorDesc);
	setIconSize(QSize(16,16));
}

AuDesignerBar2::~AuDesignerBar2()
{
}

void AuDesignerBar2::changeEvent(QEvent* event)
{
	QToolBar::changeEvent(event);
	if(event->type() == QEvent::ActivationChange)
	{
		// Configure transparency state of this widget.
		if(isFloating())
		{
			float opacity = (float)g_Au.WindowOpacity(isActiveWindow())/100.f;
			setWindowOpacity(opacity);
		}
		else
		{
			// This appears to have no effect when docked but here goes anyway.
			setWindowOpacity(1);
		}
	}
}

void AuDesignerBar2::SetAnchorDesc(QString desc)
{
	if(m_AnchorDesc->text() != desc)
		m_AnchorDesc->setText(desc);
}

void AuDesignerBar2::SetBlipCoordinate(AuPoint coord)
{
	AuPoint cround(coord);
	cround /= g_AuClient.DesignerUnitScale();								// Convert output to desired units.
	cround = cround.Rounded(.001);

	QString scoord;
	if(cround.z == 0)
		scoord = QString("%1, %2").arg(cround.x, 0,'g',3).arg(cround.y, 0,'g',3);
	else
		scoord = QString("%1, %2, %3").arg(cround.x, 0,'g',3).arg(cround.y, 0,'g',3).arg(cround.z, 0,'g',3);
	m_BlipCoord->setText(scoord);
}

void AuDesignerBar2::ShowGrid()
{
	auDesignerApp->DesignerFrame()->DesignerSetShowGrid(m_ShowGrid->isChecked());
}

void AuDesignerBar2::SetShowGrid(bool showEm)
{
	if(m_ShowGrid->isChecked() != showEm)
	{
		m_ShowGrid->blockSignals(true);
		m_ShowGrid->setChecked(showEm);
		m_ShowGrid->blockSignals(false);
	}
}

void AuDesignerBar2::ShowBacks()
{
	auDesignerApp->DesignerFrame()->DesignerSetShowBacks(m_ShowBacks->isChecked());
}

void AuDesignerBar2::SetShowBacks(bool showEm)
{
	if(m_ShowBacks->isChecked() != showEm)
	{
		m_ShowBacks->blockSignals(true);
		m_ShowBacks->setChecked(showEm);
		m_ShowBacks->blockSignals(false);
	}
}

void AuDesignerBar2::ShowEdges()
{
	auDesignerApp->DesignerFrame()->DesignerSetShowEdges(m_ShowEdges->isChecked());
}

void AuDesignerBar2::SetShowEdges(bool showEm)
{
	if(m_ShowEdges->isChecked() != showEm)
	{
		m_ShowEdges->blockSignals(true);
		m_ShowEdges->setChecked(showEm);
		m_ShowEdges->blockSignals(false);
	}
}

void AuDesignerBar2::ShowFaces()
{
	auDesignerApp->DesignerFrame()->DesignerSetShowFaces(m_ShowFaces->isChecked());
}

void AuDesignerBar2::SetShowFaces(bool showEm)
{
	if(m_ShowFaces->isChecked() != showEm)
	{
		m_ShowFaces->blockSignals(true);
		m_ShowFaces->setChecked(showEm);
		m_ShowFaces->blockSignals(false);
	}
}

void AuDesignerBar2::SnapToEdges()
{
	auDesignerApp->DesignerFrame()->DesignerSetSnapToEdges(m_SnapToEdges->isChecked());
}

void AuDesignerBar2::SetSnapToEdges(bool snap)
{
	if(m_SnapToEdges->isChecked() != snap)
	{
		m_SnapToEdges->blockSignals(true);
		m_SnapToEdges->setChecked(snap);
		m_SnapToEdges->blockSignals(false);
	}
}

void AuDesignerBar2::SnapToVertices()
{
	auDesignerApp->DesignerFrame()->DesignerSetSnapToVertices(m_SnapToVertices->isChecked());
}

void AuDesignerBar2::SetSnapToVertices(bool snap)
{
	if(m_SnapToVertices->isChecked() != snap)
	{
		m_SnapToVertices->blockSignals(true);
		m_SnapToVertices->setChecked(snap);
		m_SnapToVertices->blockSignals(false);
	}
}

void AuDesignerBar2::GridFollowsWork()
{
	auDesignerApp->DesignerFrame()->DesignerSetGridFollowsWork(m_GridFollowsWork->isChecked());
}

void AuDesignerBar2::SetGridFollowsWork(bool follow)
{
	if(m_GridFollowsWork->isChecked() != follow)
	{
		m_GridFollowsWork->blockSignals(true);
		m_GridFollowsWork->setChecked(follow);
		m_GridFollowsWork->blockSignals(false);
	}
}

void AuDesignerBar2::SnapToGrid()
{
	auDesignerApp->DesignerFrame()->DesignerSetSnapToGrid(m_SnapToGrid->isChecked());
}

void AuDesignerBar2::SetSnapToGrid(bool snap)
{
	if(m_SnapToGrid->isChecked() != snap)
	{
		m_SnapToGrid->blockSignals(true);
		m_SnapToGrid->setChecked(snap);
		m_SnapToGrid->blockSignals(false);
	}
}

void AuDesignerBar2::GridSpacingChanged(int idx)
{
	double gridSpacing = m_GridIncs->itemData(idx).toDouble();
	quint8 gridOm;
	quint8 gridNoM;
	if(m_GridIncs->itemText(idx) == "1/16\"")
	{
		gridOm = 16;
		gridNoM = 12;
	}
	else if(m_GridIncs->itemText(idx) == "1/8\"")
	{
		gridOm = 8;
		gridNoM = 12;
	}
	else if(m_GridIncs->itemText(idx) == "1/4\"")
	{
		gridOm = 4;
		gridNoM = 12;
	}
	else if(m_GridIncs->itemText(idx) == "1/2\"")
	{
		gridOm = 2;
		gridNoM = 12;
	}
	else if(m_GridIncs->itemText(idx) == "1\"")
	{
		gridOm = 12;
		gridNoM = 3;
	}
	else if(m_GridIncs->itemText(idx) == "1\'")
	{
		gridOm = 3;
		gridNoM = 10;
	}
	else
	{
		gridOm = 10;
		gridNoM = 10;
	}
	g_AuClient.DesignerSetGridSpacing(gridSpacing, gridOm, gridNoM);
}

void AuDesignerBar2::ObjectMode()
{
	auDesignerApp->DesignerFrame()->DesignerSetObjectMode(m_ObjectMode->isChecked());

	EnableControls();
}

void AuDesignerBar2::SetObjectMode(bool objectMode)
{
	if(m_ObjectMode->isChecked() != objectMode)
	{
		m_ObjectMode->blockSignals(true);
		m_ObjectMode->setChecked(objectMode);
		m_ObjectMode->blockSignals(false);
	}
}

void AuDesignerBar2::ObjectIsolate()
{
	auDesignerApp->DesignerFrame()->DesignerSetObjectIsolate(m_ObjectIsolate->isChecked());

	EnableControls();
}

void AuDesignerBar2::SetObjectIsolate(bool objectIsolate)
{
	if(m_ObjectIsolate->isChecked() != objectIsolate)
	{
		m_ObjectIsolate->blockSignals(true);
		m_ObjectIsolate->setChecked(objectIsolate);
		m_ObjectIsolate->blockSignals(false);
	}
}

void AuDesignerBar2::EnableControls()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = pFrame->DesignerView();
	bool preview = view->IsDesignPreview();
	DaDesignBase::DESIGNMODE designMode = (DaDesignBase::DESIGNMODE)view->DesignMode();
	bool dok = !preview && designMode > DaDesignBase::DESIGN_Unknown;			// True if mode is valid for designing things.
	m_BlipCoord->setEnabled(dok);
	m_FindEdges->setEnabled(dok && view->DesignerObjectMode());
	m_FindFaces->setEnabled(dok && view->DesignerObjectMode());
	m_FindVertices->setEnabled(dok && view->DesignerObjectMode());
	m_GridFollowsWork->setEnabled(dok);
	m_GridIncs->setEnabled(dok);
	m_GridXY->setEnabled(dok);
	m_GridXZ->setEnabled(dok);
	m_GridYZ->setEnabled(dok);
	m_MoveGrid->setEnabled(dok);
	m_ObjectIsolate->setEnabled(dok && auDesignerApp->DesignerFrame()->DesignerMenuObjectModeEnabled() && view->DesignerObjectMode());
	m_ObjectMode->setEnabled(dok && auDesignerApp->DesignerFrame()->DesignerMenuObjectModeEnabled());
	m_PositionGrid->setEnabled(dok);
	m_ShowGrid->setEnabled(dok);
	m_ShowBacks->setEnabled(dok);
	m_ShowEdges->setEnabled(dok);
	m_ShowFaces->setEnabled(dok);
	m_SnapToEdges->setEnabled(dok);
	m_SnapToGrid->setEnabled(dok);
	m_SnapToVertices->setEnabled(dok);
}

void AuDesignerBar2::FindEdges()
{
	auDesignerApp->DesignerFrame()->DesignerSetFindEdges(m_FindEdges->isChecked());
}

void AuDesignerBar2::SetFindEdges(bool findEm)
{
	if(m_FindEdges->isChecked() != findEm)
	{
		m_FindEdges->blockSignals(true);
		m_FindEdges->setChecked(findEm);
		m_FindEdges->blockSignals(false);
	}
}

void AuDesignerBar2::FindFaces()
{
	auDesignerApp->DesignerFrame()->DesignerSetFindFaces(m_FindFaces->isChecked());
}

void AuDesignerBar2::SetFindFaces(bool findEm)
{
	if(m_FindFaces->isChecked() != findEm)
	{
		m_FindFaces->blockSignals(true);
		m_FindFaces->setChecked(findEm);
		m_FindFaces->blockSignals(false);
	}
}

void AuDesignerBar2::FindVertices()
{
	auDesignerApp->DesignerFrame()->DesignerSetFindVertices(m_FindVertices->isChecked());
}

void AuDesignerBar2::SetFindVertices(bool findEm)
{
	if(m_FindVertices->isChecked() != findEm)
	{
		m_FindVertices->blockSignals(true);
		m_FindVertices->setChecked(findEm);
		m_FindVertices->blockSignals(false);
	}
}
