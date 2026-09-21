//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationnav.cpp,v 1.32 2022/04/14 16:51:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationnav.h"
//
#include "auatomrenderinfo.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausegment.h"
#include "ausound.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "daworldglobe.h"
#include "daworldringsection.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QPainter>
#include <QPixmap>

#define d_Neighbors						SOUND_Bleep1
#define d_OrbitRings					SOUND_Bleep2
#define d_Route							SOUND_Bleep5
#define d_SystemNames					SOUND_Bleep7
#define d_View							SOUND_Bleep6
#define d_Waypoints						SOUND_Bleep3
#define d_Wormholes						SOUND_Bleep8

// Table of control inputs.
DiControlInputArray s_NavConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_NavConsoleDefaults[] =
{
	DiControlInput(QObject::tr("Neighbor Sectors Toggle"), DpStationNav::CI_NeighborsToggle,
				Qt::Key_N, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("System Names Toggle"), DpStationNav::CI_NamesToggle,
				Qt::Key_Y, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Orbit Rings Toggle"), DpStationNav::CI_RingsToggle,
				Qt::Key_O, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Wormholes Toggle"), DpStationNav::CI_WormholeToggle,
				Qt::Key_H, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Pan/Rotate Left"), DpStationNav::CI_PanLeft,
				Qt::Key_A, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Pan/Rotate Right"), DpStationNav::CI_PanRight,
				Qt::Key_D, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Pan/Rotate Up"), DpStationNav::CI_PanUp,
				Qt::Key_W, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Pan/Rotate Down"), DpStationNav::CI_PanDown,
				Qt::Key_S, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Center Solar System/Top Sector View"), DpStationNav::CI_TopViewCenter,
				Qt::Key_K, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Pan to Ship"), DpStationNav::CI_PanToShip,
				Qt::Key_Home, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Front Sector View"), DpStationNav::CI_FrontView,
				Qt::Key_M, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Left Sector View"), DpStationNav::CI_LeftView,
				Qt::Key_J, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Back Sector View"), DpStationNav::CI_BackView,
				Qt::Key_I, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Right Sector View"), DpStationNav::CI_RightView,
				Qt::Key_L, Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Increase Magnification"), DpStationNav::CI_ZoomIn,
				Qt::Key_PageUp, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0, 100, true),
	DiControlInput(QObject::tr("Decrease Magnification"), DpStationNav::CI_ZoomOut,
				Qt::Key_PageDown, Qt::NoModifier, DiControlInput::MT_None,
				Qt::NoModifier, DiControlInput::JT_None, 0, 100, true),
};
int s_NumNavConsoleDefaults = sizeof(s_NavConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationNav::DpStationNav() : DpStation()
{
	m_bMoonRingsVisible = false;
	m_bNeighborSectors = false;
	m_bRingsVisible = false;
	m_bTrackShip = true;
	m_bWormholesVisible = false;
	m_DisplayChanged = false;
	m_eCurrentCommand = CI_Invalid;
	m_eWaypoints = WP_WaypointsAndRoutes;
	m_iNamesVisible = 0;
	m_Zoom = Z_Sector0;

	// Selection
	m_eSelection = SEL_None;
	m_lSelectedWaypoint = 0;

	// Init with bogus values to insure an update.
	m_bLastNeighborSectors = -1;
	m_bLastRings = -1;
	m_bLastWormholes = -1;
	m_eLastCommand = CI_NeighborsToggle;
	m_eLastSel = SEL_Invalid;
	m_eLastWaypoints = WP_Off;
	m_iLastNames = -1;
	m_lLastWaypointCount = -1;
	m_LastZoom = Z_Invalid;
	m_ScreenTex = 0;
}

DpStationNav::~DpStationNav()
{
}

DpStationNav::DpStationNav(const DpStationNav &c) : DpStation(c)
{
	m_bLastNeighborSectors = c.m_bLastNeighborSectors;
	m_bLastRings = c.m_bLastRings;
	m_bLastWormholes = c.m_bLastWormholes;
	m_bMoonRingsVisible = c.m_bMoonRingsVisible;
	m_bNeighborSectors = c.m_bNeighborSectors;
	m_bRingsVisible = c.m_bRingsVisible;
	m_bTrackShip = c.m_bTrackShip;
	m_bWormholesVisible = c.m_bWormholesVisible;
	m_eCurrentCommand = c.m_eCurrentCommand;
	m_eLastCommand = c.m_eLastCommand;
	m_eLastSel = c.m_eLastSel;
	m_eLastWaypoints = c.m_eLastWaypoints;
	m_eSelection = c.m_eSelection;
	m_eWaypoints = c.m_eWaypoints;
	m_fPan = c.m_fPan;
	m_fRot = c.m_fRot;
	m_idSelected = c.m_idSelected;
	m_idSystem = c.m_idSystem;
	m_iLastNames = c.m_iLastNames;
	m_iNamesVisible = c.m_iNamesVisible;
	m_LastZoom = c.m_LastZoom;
	m_lLastWaypointCount = c.m_lLastWaypointCount;
	m_lSelectedWaypoint = c.m_lSelectedWaypoint;
	m_SelectedOffset = c.m_SelectedOffset;
	m_Zoom = c.m_Zoom;

	c.m_WaypointLock.lockForRead();
	m_Waypoints = c.m_Waypoints;
	c.m_WaypointLock.unlock();
	m_ScreenTex = 0;
}

void DpStationNav::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	quint8 bits;
	in >> bits;
	m_bNeighborSectors = (bits & 0x01) ? true : false;
	m_bRingsVisible = (bits & 0x02) ? true : false;
	m_bTrackShip = (bits & 0x04) ? true : false;
	m_bWormholesVisible = (bits & 0x08) ? true : false;
	m_iNamesVisible = (bits & 0x30) >> 4;
	m_bMoonRingsVisible = (bits & 0x40) ? true : false;

	quint8 val8;
	in >> val8;
	m_eCurrentCommand = (CONTROLINPUT)val8;
	in >> val8;
	m_eSelection = (SELECTION)val8;
	in >> val8;
	m_eWaypoints = (WAYPOINTS)val8;
	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fPan = AuPoint(x,y,z);
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	m_idSelected << in;
	m_idSystem << in;
	in >> m_lSelectedWaypoint;
	in >> val8;
	m_Zoom = (ZOOMSETTING)val8;
	in >> x;
	in >> y;
	in >> z;
	m_SelectedOffset = AuPoint(x,y,z);

	m_Waypoints.Rx(in);
}

void DpStationNav::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	quint8 bits = 0;
	if(m_bNeighborSectors)
		bits |= 0x01;
	if(m_bRingsVisible)
		bits |= 0x02;
	if(m_bTrackShip)
		bits |= 0x04;
	if(m_bWormholesVisible)
		bits |= 0x08;
	if(m_iNamesVisible)
		bits |= (m_iNamesVisible << 4);
	if(m_bMoonRingsVisible)
		bits |= 0x40;
	out << bits;

	out << (quint8)m_eCurrentCommand;
	out << (quint8)m_eSelection;
	out << (quint8)m_eWaypoints;
	out << (float)m_fPan.x;
	out << (float)m_fPan.y;
	out << (float)m_fPan.z;
	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	m_idSelected >> out;
	m_idSystem >> out;
	out << m_lSelectedWaypoint;
	out << (quint8)m_Zoom;
	out << (float)m_SelectedOffset.x;
	out << (float)m_SelectedOffset.y;
	out << (float)m_SelectedOffset.z;

	m_WaypointLock.lockForRead();
	m_Waypoints.Tx(out);		// all waypoints
	m_WaypointLock.unlock();
}

// Refresh data from valid counterpart.
void DpStationNav::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationNav* pA = (const DpStationNav*)pCon;
	if(m_Waypoints != pA->m_Waypoints)
	{
		m_Waypoints = pA->m_Waypoints;
		m_DisplayChanged = true;
	}
	if(m_bMoonRingsVisible != pA->m_bMoonRingsVisible)
	{
		m_bMoonRingsVisible = pA->m_bMoonRingsVisible;
		m_DisplayChanged = true;
	}
	if(m_bNeighborSectors != pA->m_bNeighborSectors)
	{
		m_bNeighborSectors = pA->m_bNeighborSectors;
		m_DisplayChanged = true;
	}
	if(m_bRingsVisible != pA->m_bRingsVisible)
	{
		m_bRingsVisible = pA->m_bRingsVisible;
		m_DisplayChanged = true;
	}
	m_bTrackShip = pA->m_bTrackShip;
	if(m_bWormholesVisible != pA->m_bWormholesVisible)
	{
		m_bWormholesVisible = pA->m_bWormholesVisible;
		m_DisplayChanged = true;
	}
	m_eCurrentCommand = pA->m_eCurrentCommand;
	if(m_eSelection != pA->m_eSelection)
	{
		m_eSelection = pA->m_eSelection;
		m_DisplayChanged = true;
	}
	if(m_eWaypoints != pA->m_eWaypoints)
	{
		m_eWaypoints = pA->m_eWaypoints;
		m_DisplayChanged = true;
	}
	if(m_fPan != pA->m_fPan)
	{
		m_fPan = pA->m_fPan;
		m_DisplayChanged = true;
	}
	if(m_fRot != pA->m_fRot)
	{
		m_fRot = pA->m_fRot;
		m_DisplayChanged = true;
	}
	if(m_idSelected != pA->m_idSelected)
	{
		m_idSelected = pA->m_idSelected;
		m_DisplayChanged = true;
	}
	if(m_idSystem != pA->m_idSystem)
	{
		m_idSystem = pA->m_idSystem;
		m_DisplayChanged = true;
	}
	if(m_iNamesVisible != pA->m_iNamesVisible)
	{
		m_iNamesVisible = pA->m_iNamesVisible;
		m_DisplayChanged = true;
	}
	if(m_lSelectedWaypoint != pA->m_lSelectedWaypoint)
	{
		m_lSelectedWaypoint = pA->m_lSelectedWaypoint;
		m_DisplayChanged = true;
	}
	if(m_SelectedOffset != pA->m_SelectedOffset)
	{
		m_SelectedOffset = pA->m_SelectedOffset;
		m_DisplayChanged = true;
	}
	if(m_Zoom != pA->m_Zoom)
	{
		m_Zoom = pA->m_Zoom;
		m_DisplayChanged = true;
	}
}

// Called between rendering scene frames that contains this object.
void DpStationNav::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const AuPoint& location,
		const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);
	AnimateDeviceObjects(re, ship, last);

}

void DpStationNav::AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last, const AuPoint& location,
		const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, bd, last, location, sceneAtom);
	AnimateDeviceObjects(re, bd, last);

}

void DpStationNav::AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& ship, const AuElapsedTimer& last)
{
	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("NCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if (!pTex)
		return;

	bool zoomChanged = (m_Zoom != m_LastZoom) ? true : false;

	// Update the texture map with the current control states.
	// Neighbor Sectors Indicator
	if(zoomChanged || (m_bNeighborSectors?1:0) != m_bLastNeighborSectors)
	{
		m_bLastNeighborSectors = m_bNeighborSectors;

		QImage bml;
		if(m_Zoom < Z_Sector0)
		{
			bml.load(":/data/ac_Blank16x16.bmp");
		}
		else
		{
			if (m_bNeighborSectors)
				bml.load(":/data/ac_NeighborsOn.bmp");
			else
				bml.load(":/data/ac_NeighborsOff.bmp");
		}
		pTex->ReplaceSubImage(re, bml, 236,55);
	}

	// System names toggle.
	if(zoomChanged || m_iNamesVisible != m_iLastNames)
	{
		m_iLastNames = m_iNamesVisible;

		QImage bml;
		if(m_Zoom < Z_Sector0)
		{
			bml.load(":/data/ac_Blank9x9.bmp");
		}
		else
		{
			if(m_iNamesVisible == 1)
				bml.load(":/data/ac_NamesOn.bmp");
			else if(m_iNamesVisible == 2)
				bml.load(":/data/ac_NamesFull.bmp");
			else
				bml.load(":/data/ac_NamesOff.bmp");
		}
		pTex->ReplaceSubImage(re, bml, 224,55);
	}

	// Wormholes Indicator
	if((m_bWormholesVisible?1:0) != m_bLastWormholes)
	{
		m_bLastWormholes = m_bWormholesVisible;

		QImage bml;
		if (m_bWormholesVisible)
			bml.load(":/data/ac_WormholesOn.bmp");
		else
			bml.load(":/data/ac_WormholesOff.bmp");
		pTex->ReplaceSubImage(re, bml, 236,73);
	}

	// Orbit Rings Indicator
	char rings = ((int)m_bMoonRingsVisible << 1) | (int)m_bRingsVisible;
	if(rings != m_bLastRings)
	{
		m_bLastRings = rings;
		QImage bml;
		if(m_bRingsVisible && m_bMoonRingsVisible)
			bml.load(":data/pc_RingsBoth.bmp");
		else if(m_bRingsVisible)
			bml.load(":data/pc_RingsOn.bmp");
		else
			bml.load(":data/pc_RingsOff.bmp");
		pTex->ReplaceSubImage(re, bml, 236,91);
	}

	// Waypoints Visibility Indicator
	bool routechanged = false;
	if(m_eWaypoints != m_eLastWaypoints || m_Waypoints.count() != m_lLastWaypointCount)
	{
		m_eLastWaypoints = m_eWaypoints;
		m_lLastWaypointCount = m_Waypoints.count();
		routechanged = true;

		QImage bml;
		if(m_lLastWaypointCount == 0)
		{
			bml.load(":/data/ac_Blank16x16.bmp");
		}
		else
		{
			switch(m_eWaypoints)
			{
			case WP_Off:
				bml.load(":/data/ac_WaypointsOff.bmp");
				break;
			case WP_Waypoints:
				bml.load(":/data/ac_WaypointsOnly.bmp");
				break;
			case WP_Routes:
				bml.load(":/data/ac_RoutesOnly.bmp");
				break;
			default:
			case WP_WaypointsAndRoutes:
				bml.load(":/data/ac_WaypointsAndRoutes.bmp");
				break;
			}
		}
		pTex->ReplaceSubImage(re, bml, 236,37);
	}

	// Primary display.
	if(m_DisplayChanged || m_bTrackShip || last.ElapsedSecTicks() > 0)
	{
		const QSize szdisp(218,120);
		QPixmap bmd(szdisp.width(),szdisp.height());
		if(!bmd.isNull())
		{
			bmd.fill(Qt::black);
			QPainter p(&bmd);
			if(m_Zoom >= Z_Sector0)
				DrawSectorDisplay(p, szdisp, ship, ship.EmpireId());
			else
				DrawSolarSystemDisplay(p, szdisp, ship, ship.EmpireId());

			QImage imd = bmd.toImage();
			pTex->ReplaceSubImage(re, imd, 4,4);
		}
	}

	// Zoom scale indicator.
	if(zoomChanged)
	{
		m_LastZoom = m_Zoom;

		// Zoom bug.
		QImage bmbug(":/data/ac_ZoomBug.png");
		if (!bmbug.isNull())
		{
			QPixmap bmz(":/data/ac_Zoom.bmp");
			if (!bmz.isNull())
			{
				QPainter p(&bmz);

				// Vertical positions of zoom indicator by zoom level within
				// the sub image. We add one in case the zoom is set to
				// Z_Invalid, which is -1.
				int iy[Z_Levels+1] = {0, 0,3,6,9,12,15, 21,24,27,30,33,36};
				p.drawImage(0,iy[m_Zoom+1], bmbug);

				QImage imz = bmz.toImage();
				pTex->ReplaceSubImage(re, imz, 225,68);
			}
		}
	}

	// Pan to select cursor.
	if(m_eSelection != m_eLastSel)
	{
		m_eLastSel = m_eSelection;

		QImage bm;
		if(m_eSelection == SEL_None)
			bm.load(":/data/ac_Blank9x9.bmp");
		else
			bm.load(":/data/ac_PanToSelect.bmp");
		pTex->ReplaceSubImage(re, bm, 241,112);
	}

	// Nav buttons.
	if(m_DisplayChanged || m_eCurrentCommand != m_eLastCommand || routechanged)
	{
		m_eLastCommand = m_eCurrentCommand;

		// Add route to select cursor.
		QImage bm;
		if(m_eSelection == SEL_None)	// || m_Zoom < 6)
			bm.load(":/data/ac_Blank9x9.bmp");
		else
			bm.load(":/data/ac_AddRouteToSelect.bmp");
		pTex->ReplaceSubImage(re, bm, 224,23);

		// Add waypoint at select cursor.
		if(m_eSelection == SEL_None)
			bm.load(":/data/ac_Blank9x9.bmp");
		else
			bm.load(":/data/ac_AddWaypointAtSelect.bmp");
		pTex->ReplaceSubImage(re, bm, 224,13);

		// Add Waypoint
		if(m_Zoom < Z_Sector0)
		{
			if (m_eCurrentCommand == CI_AddWaypoint)
				bm.load(":/data/ac_AddWaypointLit.bmp");
			else
				bm.load(":/data/ac_AddWaypoint.bmp");
		}
		else
			bm.load(":/data/ac_Blank9x9.bmp");
		pTex->ReplaceSubImage(re, bm, 234,13);

		// Insert Waypoint
		if(m_Zoom < Z_Sector0)
		{
			if(!m_Waypoints.isEmpty() && m_eSelection == SEL_Waypoint)
			{
				if (m_eCurrentCommand == CI_InsertWaypoint)
					bm.load(":/data/ac_InsertWaypointLit.bmp");
				else
					bm.load(":/data/ac_InsertWaypoint.bmp");
			}
			else
				bm.load(":/data/ac_Blank9x9.bmp");
		}
		else
			bm.load(":/data/ac_Blank9x9.bmp");
		pTex->ReplaceSubImage(re, bm, 244,13);

		// Delete Waypoint
		if(m_eSelection != SEL_Waypoint || m_Waypoints.isEmpty())
			bm.load(":/data/ac_Blank9x9.bmp");
		else
			bm.load(":/data/ac_DeleteWaypointAtSelect.bmp");
		pTex->ReplaceSubImage(re, bm, 234,3);

		// Delete All Waypoints
		if(m_Waypoints.isEmpty())
			bm.load(":/data/ac_Blank9x9.bmp");
		else
			bm.load(":/data/ac_DeleteAllWaypoints.bmp");
		pTex->ReplaceSubImage(re, bm, 244,3);
	}

	// Remember last animate time.
	m_DisplayChanged = false;	// Display is good now.
}

void DpStationNav::DrawSolarSystemDisplay(QPainter& p, const QSize &sz, const DaAtom& ref, quint32 refEmpire)
{
	AuEther* ether = ref.Ether();
	if(!ether)
		return;
	//qDebug() << "DpStationNav::DrawSolarSystemDisplay" << m_idSystem << "ref" << ref.Id() << ref.Name();
	const DaSolarSystem* pmyss = dynamic_cast<const DaSolarSystem*>(ref.FindEnvironmentAtom());
	if(!pmyss)
		return;	// No system display in arena.

	// Find the solar system that is the subject of the display.
	DaSolarSystem* pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
	if(!pssv)
	{
		//qDebug() << "DpStationNav::DrawSolarSystemDisplay - Solar system not found" << m_idSystem;
		return;
	}
	else if(!pssv->IsExploredBy(refEmpire))
	{
		// The ship's owning empire has never explored the system.
		p.setFont(QFont(g_Au.TypefaceSans(), 8));
		p.setPen(QPen(QColor(0,200,100), 0.f));
		QRect rc(2,0,sz.width()-2,sz.height());
		QString sText = pssv->Name();
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, sText);
		p.drawText(rc, Qt::AlignHCenter|Qt::AlignVCenter,
				QObject::tr("Uncharted Solar System"));
		return;
	}

	DaStar* pstar = pssv->FindStar();
	if(!pstar)
	{
		// There is no data available on the system.
		// The system has never been explored
		//  or
		// the scene server only has the system loaded for starfield purposes.
		p.setFont(QFont(g_Au.TypefaceSans(), 8));
		p.setPen(QPen(QColor(200,100,0), 0.f));
		QRect rc(2,0,sz.width()-2,sz.height());
		QString sText = pssv->Name();
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, sText);
		p.drawText(rc, Qt::AlignHCenter|Qt::AlignVCenter,
				QObject::tr("Deep Space"));
		return;
	}

	// Calculate some values for converting the real world into this pixel
	// rectangle.
	QPoint pCenter(sz.width()/2, sz.height()/2);
	const int iMaxStarOrbits = 16;
	// In case orbit 15 is a star with its max of 7 orbits (1/2 the star's
	// orbit number (0-15)).
	const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
	const int iPixelsPerOrbit = pCenter.y()/iMaxSystemOrbits *
		(1<<(Z_System5 - m_Zoom));

	// Offset pCenter for pan.
	AuPoint fpxpan(m_fPan);
	fpxpan /= FeetPerOrbit();
	fpxpan *= iPixelsPerOrbit;
	pCenter.setX(pCenter.x() - (int)fpxpan.x);
	pCenter.setY(pCenter.y() + (int)fpxpan.y);

	// Draw primary and subordinate stars and their planets.
	bool bOrbit[6] = {0,0,0,0,0,0};	// Orbit zones set this true as they're used
	DrawStarAndPlanets(p, pCenter, pstar, pssv, iPixelsPerOrbit, bOrbit);
	DaAtomList stars;
	pstar->ListStars(stars);
	while(!stars.isEmpty())
	{
		pstar = (DaStar*)stars.takeFirst();
		DrawStarAndPlanets(p, pCenter, pstar, pssv, iPixelsPerOrbit, bOrbit);
	}

	// Calculate my location.
	bool bInSystem = false;
	AuPoint fcons;
	QPoint pxcons;
	if(pmyss->Is(m_idSystem))
	{
		bInSystem = true;
		fcons = ref.EnvLocation();
		fcons /= FeetPerOrbit();		// Convert to "orbits".
		fcons *= iPixelsPerOrbit;		// Convert to "pixels".
		pxcons.setX((int)(pCenter.x() + fcons.x));
		pxcons.setY((int)(pCenter.y() - fcons.y));
	}

	// Draw waypoints.
	if(!m_Waypoints.isEmpty() && m_eWaypoints != WP_Off)
	{
		// Draw lines first.
		if (m_eWaypoints != WP_Waypoints)
		{
			p.setPen(QPen(QColor(0,64,128), 0.f));
			bool bMove = true;
			QPoint pxmove;
			if (bInSystem)
			{
				pxmove = pxcons;
				bMove = false;		// First move is done. Next will be a LineTo.
			}
			for(DeWaypointRoute::const_iterator wi = m_Waypoints.constBegin(); wi != m_Waypoints.constEnd(); wi++)
			{
				AuPoint floc;
				if((*wi).Location(pssv, floc))
				{
					// The destination of the waypoint is within the viewed solar system.
					floc /= FeetPerOrbit();		// Convert to "orbits".
					floc *= iPixelsPerOrbit;		// Convert to "pixels".
					QPoint pxw;
					pxw.setX((int)(pCenter.x() + floc.x));
					pxw.setY((int)(pCenter.y() - floc.y));
					if (bMove)
					{
						// Need to move to this waypoint location.
						pxmove = pxw;
						bMove = false;
					}
					else
					{
						// Draw a line to this waypoint location.
						p.drawLine(pxmove, pxw);
						pxmove = pxw;
					}
				}
				else
				{
					// The destination of the waypoint is not within the viewed solar system.
					// The next destination that IS within the viewed solar system must be a Move rather than a Line.
					bMove = true;
				}
			}
		}

		// Draw icons and numbers after lines.
		if (m_eWaypoints != WP_Routes)
		{
			p.setFont(QFont(g_Au.TypefaceSans(), 6));	// This font is pretty small but works ok
											// for just numbers.
			const int iwaysz = 3;
			int iw = 1;
			for(DeWaypointRoute::const_iterator wi = m_Waypoints.constBegin(); wi != m_Waypoints.constEnd(); wi++, iw++)
			{
				AuPoint floc;
				if((*wi).Location(pssv, floc))
				{
					// The destination of the waypoint is within the viewed system.
					floc /= FeetPerOrbit();			// Convert to "orbits".
					floc *= iPixelsPerOrbit;		// Convert to "pixels".
					QPoint pxw;
					pxw.setX((int)(pCenter.x() + floc.x));
					pxw.setY((int)(pCenter.y() - floc.y));

					p.setPen(QPen(QColor(192,192,192), 0.f));
					p.drawLine(pxw.x(), pxw.y() - iwaysz, pxw.x() + iwaysz, pxw.y());
					p.drawLine(pxw.x() + iwaysz, pxw.y(), pxw.x(), pxw.y() + iwaysz);
					p.drawLine(pxw.x(), pxw.y() + iwaysz, pxw.x() - iwaysz, pxw.y());
					p.drawLine(pxw.x() - iwaysz, pxw.y(), pxw.x(), pxw.y() - iwaysz);

					p.setPen(QPen(QColor(128,128,128), 0.f));
					QString sid;
					sid = QString("%1").arg(iw);
					p.drawText(pxw.x() + iwaysz, pxw.y() + iwaysz/2, sid);
				}
				else
				{
					// The destination of the waypoint is not within the viewed system.
					// Don't draw anything.
				}
			}
		}
	}

	// Mark the location of this console.
	if(bInSystem)
	{
		p.setPen(QPen(QColor(0,255,0), 0.f));
		if (fcons.z >= 0)
		{
			p.drawPoint(pxcons.x(), pxcons.y());
			p.drawPoint(pxcons.x()+1, pxcons.y());
			p.drawPoint(pxcons.x(), pxcons.y()-1);
			p.drawPoint(pxcons.x()-1, pxcons.y());
		}
		else
		{
			p.drawPoint(pxcons.x(), pxcons.y());
			p.drawPoint(pxcons.x()+1, pxcons.y());
			p.drawPoint(pxcons.x(), pxcons.y()+1);
			p.drawPoint(pxcons.x()-1, pxcons.y());
		}
	}

	// Draw wormholes.
	// Flash every second.
	QColor colWorm[8];
	colWorm[0] = QColor(255,32,0);		// Known Outie High
	colWorm[1] = QColor(255,0,255);		// Known Innie High
	colWorm[2] = QColor(128,16,0);		// Known Outie Low
	colWorm[3] = QColor(128,0,128);		// Known Innie Low
	colWorm[4] = QColor(255,255,0);		// "Known" marker High
	colWorm[5] = QColor(128,128,0);		// "Known" marker Low
	colWorm[6] = QColor(255,0,0);		// Known intergalactic High
	colWorm[7] = QColor(128,0,0);		// Known intergalactic low
	const AuWormholeArray& wha = pssv->LockWormholes();
	bool oddSec = AuElapsedTimer::CurrentSecIsOdd();
	for(AuWormholeArray::const_iterator it = wha.constBegin(); it != wha.constEnd(); it++)
	{
		AuPoint fworm((*it).Location(pssv));	// Location relative to solar system center, in feet.
		fworm /= FeetPerOrbit();		// Convert to "orbits".
		fworm *= iPixelsPerOrbit;		// Convert to "pixels".

		int icol = -1;
		int iknow = -1;
		if((*it).Polarity() == AuWormhole::Positive)
		{
			if(oddSec)
			{
				if(fworm.z >= fcons.z)	// When not in the system, fcons.z is 0.0, which is the XY plane.
				{
					icol = 1;	// Above me.
					iknow = 4;
				}
				else
				{
					icol = 3;	// Below me.
					iknow = 5;
				}
			}
		}
		else if((*it).Polarity() == AuWormhole::Negative)
		{
			if(!oddSec)
			{
				if(fworm.z >= fcons.z)	// When not in the system, fcons.z is 0.0, which is the XY plane.
				{
					icol = 0;	// Above me.
					iknow = 4;
				}
				else
				{
					icol = 2;	// Below me.
					iknow = 5;
				}
			}
		}
		else
		{
			// Intergalactic wormhole.
			if(fworm.z >= fcons.z)	// When not in the system, fcons.z is 0.0, which is the XY plane.
			{
				if(oddSec)
				{
					icol = 0;	// Above me.
					iknow = 6;
				}
			}
			else
			{
				if(!oddSec)
				{
					icol = 2;	// Below me.
					iknow = 7;
				}
			}
		}

		if(icol >= 0)
		{
			p.setPen(QPen(colWorm[icol], 0.f));
			QPoint pxw;
			pxw.setX((int)(pCenter.x() + fworm.x));
			pxw.setY((int)(pCenter.y() - fworm.y));
			if(m_Zoom <= Z_System2)
			{
				if((*it).IsIntergalactic())
				{
					p.drawPoint(pxw.x()-1, pxw.y()-1);
					p.drawPoint(pxw.x()-1, pxw.y()+1);
					p.drawPoint(pxw.x()+1, pxw.y()+1);
					p.drawPoint(pxw.x()+1, pxw.y()-1);
				}
				else
				{
					p.drawPoint(pxw.x()-1, pxw.y());
					p.drawPoint(pxw.x()+1, pxw.y());
					if((*it).Polarity() == AuWormhole::Positive)
					{
						p.drawPoint(pxw.x(), pxw.y()-1);
						p.drawPoint(pxw.x(), pxw.y()+1);
					}
				}

				if((*it).IsExploredBy(refEmpire))
					p.setPen(QPen(colWorm[iknow], 0.f));
				else
					p.setPen(QPen(Qt::black, 0.f));
				p.drawPoint(pxw);
			}
			else
			{
				p.drawPoint(pxw);
				if((*it).IsIntergalactic())
					p.drawPoint(pxw.x()-1,pxw.y()+1);
				else if((*it).Polarity() == AuWormhole::Positive)
					p.drawPoint(pxw.x()+1,pxw.y());

				if((*it).IsExploredBy(refEmpire))
					p.setPen(QPen(colWorm[iknow], 0.f));
				else
					p.setPen(QPen(Qt::black, 0.f));
				p.drawPoint(pxw.x()-1, pxw.y());
			}
		}
	}
	pssv->UnlockWormholes();

	// Display selection reticle.
	bool bSelCursorLoc = false;
	AuPoint fSelCursorLoc;
	const DaAtom *psel = NULL;
	if(m_eSelection == SEL_SolarSystemAtom)
	{
		psel = pssv->FindIdRecursive(m_idSelected);
		if (psel)
		{
			fSelCursorLoc = psel->EnvLocation();
			bSelCursorLoc = true;
		}
	}
	else if (m_eSelection == SEL_Waypoint && m_lSelectedWaypoint < m_Waypoints.count())
	{
		psel = m_Waypoints[m_lSelectedWaypoint].Location(pssv, fSelCursorLoc);
		if(psel)
			bSelCursorLoc = true;
	}
	if (bSelCursorLoc)
	{
		fSelCursorLoc += m_SelectedOffset;
		fSelCursorLoc /= FeetPerOrbit();		// Convert to "orbits".
		fSelCursorLoc *= iPixelsPerOrbit;		// Convert to "pixels".
		QPoint pxs;
		pxs.setX((int)(pCenter.x() + fSelCursorLoc.x));
		pxs.setY((int)(pCenter.y() - fSelCursorLoc.y));

		p.setPen(QPen(QColor(0,255,255), 0.f));

		p.drawLine(pxs.x() + 2, pxs.y() - 3, pxs.x() + 4, pxs.y() - 3);
		p.drawLine(pxs.x() + 4, pxs.y() - 3, pxs.x() + 4, pxs.y() + 3);
		p.drawLine(pxs.x() + 4, pxs.y() + 3, pxs.x() + 2, pxs.y() + 3);

		p.drawLine(pxs.x() - 2, pxs.y() - 3, pxs.x() - 4, pxs.y() - 3);
		p.drawLine(pxs.x() - 4, pxs.y() - 3, pxs.x() - 4, pxs.y() + 3);
		p.drawLine(pxs.x() - 4, pxs.y() + 3, pxs.x() - 2, pxs.y() + 3);
	}

	// Display star data and other text.
	QString sText = pssv->Name();
	if(!sText.isEmpty())
	{
		p.setFont(QFont(g_Au.TypefaceSans(), 8));
		p.setPen(QPen(QColor(0,200,100), 0.f));
		QRect rc(2,0,sz.width()-2,sz.height());
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, sText);

		const int ikeyht = 8;
		QColor colorbit[6];
		colorbit[0] = QColor(96,0,0);		// Inferno
		colorbit[1] = QColor(72,72,0);		// Inner
		colorbit[2] = QColor(0,96,0);		// Habitable
		colorbit[3] = QColor(0,72,72);		// Outer
		colorbit[4] = QColor(48,48,48);		// Frigid
		colorbit[5] = QColor(96,96,192);	// Companion Star
		p.setFont(QFont(g_Au.TypefaceSans(), ikeyht-1));
		QRect rckey(rc);
		rckey.setLeft(rckey.left() + 3);
		rckey.setBottom(rckey.bottom() - ikeyht);
		for(int iorb=5; iorb>=0; iorb--)
		{
			if(bOrbit[iorb])
			{
				p.fillRect(rckey.left()-4, rckey.bottom() - ikeyht*4/5, 4, 1, colorbit[iorb]);
				p.setPen(QPen(colorbit[iorb], 0.f));
				if(iorb == 5)
					p.drawText(rckey, Qt::AlignLeft|Qt::AlignBottom|Qt::TextDontClip, QObject::tr("Companion"));
				else
					p.drawText(rckey, Qt::AlignLeft|Qt::AlignBottom|Qt::TextDontClip, OrbitZoneName((ORBITZONE)iorb));
				rckey.setBottom(rckey.bottom() - ikeyht);
			}
		}

		//qDebug("Info about selected %d psel %c seloff %g,%g,%g",
				//m_eSelection, psel?'Y':'N',
				//m_SelectedOffset.x, m_SelectedOffset.y, m_SelectedOffset.z);
		// Information about the selection.
		p.setPen(QPen(QColor(0,200,200), 0.f));
		if(m_eSelection == SEL_Waypoint && psel)
		{
			sText = QString("Nav %1, %2").arg(m_lSelectedWaypoint+1).arg(psel->Name());
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sText);
		}
		else if(m_eSelection == SEL_SolarSystemAtom && psel)
		{
			if(m_SelectedOffset.IsZero())
				sText = psel->Name();
			else
				sText = ((DaSolarSystem*)psel)->WormholeNameAt(m_SelectedOffset);
			//qDebug("Text about selected '%s'", (const char*)sText);
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sText);
		}
	}
}

void DpStationNav::DrawStarAndPlanets(QPainter& p, const QPoint &pCenter, DaStar *pstar, const DaSolarSystem* /*pss*/,
		int iPixelsPerOrbit, bool bOrbit[6])
{
	// Draw the orbit ring of the star if it's not the primary.
	if(m_bRingsVisible && !pstar->IsPrimary())
	{
		double radorbit = (pstar->Orbit() + 1.0);	// Orbit radius in orbits.
		int iradorbit = (int)(radorbit * iPixelsPerOrbit);	// Orbit radius in
													// pixels.
		QRect rcOrbit(pCenter.x()-iradorbit, pCenter.y()-iradorbit,
				iradorbit*2+1, iradorbit*2+1);
		p.setPen(QPen(QColor(96,96,192), 0.f));
		p.drawArc(rcOrbit, 0, 5760);
		bOrbit[5] = true;
	}

	// Draw a filled circle to represent the star.
	AuPoint fstar = pstar->EnvLocation();
	fstar /= FeetPerOrbit();		// Convert to "orbits".
	fstar *= iPixelsPerOrbit;		// Convert to "pixels".
	QPoint pxstar((int)(pCenter.x() + fstar.x), (int)(pCenter.y() - fstar.y));	// Center of star in bitmap.
	int iradstar = (int)(pstar->Radius()/FeetPerOrbit() * iPixelsPerOrbit);	// Pixel radius of star.
	if (iradstar < 1)
		iradstar = 1;
	p.setBrush(pstar->ChartColor());
	p.setPen(QPen(pstar->ChartColor(), 0.f));
	QRect rcStar(pxstar.x()-iradstar, pxstar.y()-iradstar, iradstar*2+1,
			iradstar*2+1);
	p.drawEllipse(rcStar);

	// Draw orbit rings.
	QColor penorbit[5] =
	{
		QColor(96,0,0),		// Inferno
		QColor(72,72,0),	// Inner
		QColor(0,96,0),		// Habitable
		QColor(0,72,72),	// Outer
		QColor(48,48,48)	// Frigid
	};
	DaAtomList planets;
	pstar->ListPlanets(planets);
	for(DaAtomList::iterator it = planets.begin(); it != planets.end(); it++)
	{
		DaWorld *pb = (DaWorld*)(*it);

		// Draw the orbit ring.
		double radorbit = pb->OrbitRadius()/FeetPerOrbit();	// Orbit radius in orbits.
		int iradorbit = (int)(radorbit * iPixelsPerOrbit);	// Orbit radius in pixels.
		if (m_bRingsVisible)
		{
			QRect rcOrbit(pxstar.x()-iradorbit, pxstar.y()-iradorbit, iradorbit*2+1, iradorbit*2+1);
			p.setPen(QPen(penorbit[pb->OrbitZone()], 0.f));
			p.drawArc(rcOrbit, 0, 5760);
			bOrbit[pb->OrbitZone()] = true;
		}
	}

	// Draw planets.
	for(DaAtomList::iterator it = planets.begin(); it != planets.end(); it++)
	{
		const DaWorld *pb = (const DaWorld*)(*it);
		QColor colPlanet = pb->AtmosphereColor();			// This is the color the planet appears from a distance.
		const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(pb);
		if(rsworld)
		{
			if(rsworld->SectionIndex())
				continue;

			// Ringworld
			// Draw the ring as a thick line, so it overlaps the orbit ring.
			double radorbit = pb->OrbitRadius()/FeetPerOrbit();	// Orbit radius in orbits.
			int iradorbit = (int)(radorbit * iPixelsPerOrbit);	// Orbit radius in pixels.
			QRect rcRingworld(pxstar.x()-iradorbit, pxstar.y()-iradorbit, iradorbit*2+1, iradorbit*2+1);
			p.setPen(QPen(colPlanet, 0.f));
			p.drawArc(rcRingworld, 0, 5760);
		}
		else
		{
			// Draw the planet.
			AuPoint ploc = pb->EnvLocation();
			ploc /= FeetPerOrbit();												// Convert to "orbits".
			ploc *= iPixelsPerOrbit;											// Convert to "pixels".
			QPoint pxc((int)(pCenter.x() + ploc.x), (int)(pCenter.y() - ploc.y));	// Center of planet in bitmap.

			// Spherical planet radius in orbits.
			double radplanet = pb->SeaLevelAltitude()/FeetPerOrbit();			// Planet radius in orbits.
			int iradplanet = (int)(radplanet * iPixelsPerOrbit);				// Planet radius in pixels.
			if (iradplanet < 1)
				iradplanet = 1;
			QRect rcPlanet(pxc.x()-iradplanet, pxc.y()-iradplanet, iradplanet*2+1, iradplanet*2+1);
			p.setBrush(colPlanet);
			p.setPen(QPen(colPlanet, 0.f));
			p.drawEllipse(rcPlanet);

			DaAtomList moons;
			pb->ListPlanets(moons);
			while(!moons.isEmpty())
			{
				DaWorld* satellite = dynamic_cast<DaWorld*>(moons.takeFirst());
				DaWorldGlobe* glmoon = dynamic_cast<DaWorldGlobe*>(satellite);
				if(glmoon)
				{
					// Draw a moon.
					AuPoint mloc = satellite->EnvLocation();
					mloc /= FeetPerOrbit();										// Convert to "orbits".
					mloc *= iPixelsPerOrbit;									// Convert to "pixels".
					QPoint pxm((int)(pCenter.x() + mloc.x), (int)(pCenter.y() - mloc.y));	// Center of planet in bitmap.

					QColor colMoon = satellite->AtmosphereColor();			// This is the color the planet appears from a distance.
					p.setPen(QPen(colMoon, 0.f));
					p.drawPoint(pxm);
				}
				else
				{
					/*
					DaWorldRing* ring = dynamic_cast<DaWorldRing*>(satellite);
					if(ring)
					{
						// Draw a ring.

						// This old code could never logically have executed due to the way it was structured. It may provide the
						// roots of what is needed to draw a ring.
						//DHH what does this do?
						double radplanet = pb->Radius()/FeetPerOrbit();			// Planet radius in orbits.
						int iradplanet = (int)(radplanet * iPixelsPerOrbit);	// Planet radius in pixels.
						if (iradplanet < 1)
							iradplanet = 1;
						QRect rcPlanet(pxm.x()-iradplanet, pxm.y()-iradplanet, iradplanet*2+1, iradplanet*2+1);
						p.setBrush(colMoon);
						p.setPen(QPen(colMoon, 0.f));
						p.drawEllipse(rcPlanet);
					}
					*/
				}
			}
		}
	}
}

void DpStationNav::DrawSectorDisplay(QPainter& p, const QSize &sz, const DaAtom& ref, quint32 refEmpire)
{
	// Find the solar system that is the subject of the display.
	AuEther* ether = ref.Ether();
	if(!ether)
		return;
	DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
	if (!pssv)
		return;
	DaSector *psect = pssv->FindParentSector();
	if (!psect)
		return;
	DaSolarSystem* pmyss = dynamic_cast<DaSolarSystem*>(ref.FindEnvironmentAtom());
	if(!pmyss)
		return;	// No sector display in arena.
	DaSector* pshipsect = (DaSector*)ref.FindParentSector();
	if(!pshipsect)
		return;

	// Get location that will be the center of the star field.
	AuPoint fRelLoc;
	pssv->GetLocationRelativeToParent(fRelLoc, psect);

	// Determine ship location relative to the center sector of the display, in parsecs.
	AuPoint shipSectorLoc;
	ref.GetLocationRelativeToParent(shipSectorLoc, pshipsect);
	if(pshipsect != psect)
	{
		// Ship is in a different sector than the center sector.
		shipSectorLoc += pshipsect->Location() - psect->Location();
	}

	// Get the stars and wormholes in this sector and neighboring sectors, relative to fRelLoc.
	DqAtomLocArray aStars;
	AuSegmentArray exploredWormholes;
	AuSegmentArray knownWormholes;
	if(m_bNeighborSectors)
		psect->ListStarAndWormholeLocations(aStars, exploredWormholes, knownWormholes, refEmpire, false);	// Neighbor sectors too
	else
		psect->ListSolarSystemLocations(aStars, AuPoint(0,0), exploredWormholes, knownWormholes, refEmpire);	// Only this sector
	//qDebug("DpStationNav::DrawSectorDisplay - %d stars", aStars.count());

	// Make transforms to projection the locations into the bitmap.
	AuMatrix4x4 tdev;
	tdev.Translate(sz.width()/2.0, sz.height()/2.0, 0.0);
	double rpxscl = sz.width()/2.0;
	rpxscl *= Z_Levels - m_Zoom;
	rpxscl *= 11;

	tdev.Scale(-rpxscl, rpxscl, rpxscl);
	tdev.Perspective(60.0, 1.0, 0.5, 1000.0);
	tdev.Translate(0.0, 0.0, -10.0);
	double rscl = 1.0/5.0;
	tdev.Scale(rscl, rscl, rscl);

	AuMatrix4x4 trot;
	trot.Rotate(m_fRot);
	trot.Translate(-fRelLoc);
	AuMatrix4x4 tcomb = tdev * trot;

	// Draw the sector box frame.
	AuPoint fbll(-5,-5,-5);
	AuPoint fbul(-5, 5,-5);
	AuPoint fbur( 5, 5,-5);
	AuPoint fblr( 5,-5,-5);
	AuPoint ftll(-5,-5, 5);
	AuPoint ftul(-5, 5, 5);
	AuPoint ftur( 5, 5, 5);
	AuPoint ftlr( 5,-5, 5);
	fbll = tcomb.Project(fbll);
	fbul = tcomb.Project(fbul);
	fbur = tcomb.Project(fbur);
	fblr = tcomb.Project(fblr);
	ftll = tcomb.Project(ftll);
	ftul = tcomb.Project(ftul);
	ftur = tcomb.Project(ftur);
	ftlr = tcomb.Project(ftlr);

	if (m_bNeighborSectors)
	{
		// Show edge lines of neighboring sectors.
		AuPoint fltt(-15, 5, 5);
		AuPoint fltb(-15,-5, 5);
		AuPoint flbb(-15,-5,-5);
		AuPoint flbt(-15, 5,-5);
		fltt = tcomb.Project(fltt);
		fltb = tcomb.Project(fltb);
		flbb = tcomb.Project(flbb);
		flbt = tcomb.Project(flbt);
		AuPoint frtt(15, 5, 5);
		AuPoint frtb(15,-5, 5);
		AuPoint frbb(15,-5,-5);
		AuPoint frbt(15, 5,-5);
		frtt = tcomb.Project(frtt);
		frtb = tcomb.Project(frtb);
		frbb = tcomb.Project(frbb);
		frbt = tcomb.Project(frbt);
		AuPoint fblt(-5, 15, 5);
		AuPoint fbrt( 5, 15, 5);
		AuPoint fblb(-5, 15,-5);
		AuPoint fbrb( 5, 15,-5);
		fblt = tcomb.Project(fblt);
		fbrt = tcomb.Project(fbrt);
		fblb = tcomb.Project(fblb);
		fbrb = tcomb.Project(fbrb);
		AuPoint fflt(-5, -15, 5);
		AuPoint ffrt( 5, -15, 5);
		AuPoint fflb(-5, -15,-5);
		AuPoint ffrb( 5, -15,-5);
		fflt = tcomb.Project(fflt);
		ffrt = tcomb.Project(ffrt);
		fflb = tcomb.Project(fflb);
		ffrb = tcomb.Project(ffrb);
		AuPoint fTul(-5, 5, 15);
		AuPoint fTur( 5, 5, 15);
		AuPoint fTlr( 5,-5, 15);
		AuPoint fTll(-5,-5, 15);
		fTul = tcomb.Project(fTul);
		fTur = tcomb.Project(fTur);
		fTlr = tcomb.Project(fTlr);
		fTll = tcomb.Project(fTll);
		AuPoint fBul(-5, 5,-15);
		AuPoint fBur( 5, 5,-15);
		AuPoint fBlr( 5,-5,-15);
		AuPoint fBll(-5,-5,-15);
		fBul = tcomb.Project(fBul);
		fBur = tcomb.Project(fBur);
		fBlr = tcomb.Project(fBlr);
		fBll = tcomb.Project(fBll);

		p.setPen(QPen(QColor(0, 16, 64), 0.f));

		p.drawLine((int)ftul.x, (int)ftul.y, (int)fltt.x, (int)fltt.y);
		p.drawLine((int)ftll.x, (int)ftll.y, (int)fltb.x, (int)fltb.y);
		p.drawLine((int)fbll.x, (int)fbll.y, (int)flbb.x, (int)flbb.y);
		p.drawLine((int)fbul.x, (int)fbul.y, (int)flbt.x, (int)flbt.y);

		p.drawLine((int)ftur.x, (int)ftur.y, (int)frtt.x, (int)frtt.y);
		p.drawLine((int)ftlr.x, (int)ftlr.y, (int)frtb.x, (int)frtb.y);
		p.drawLine((int)fblr.x, (int)fblr.y, (int)frbb.x, (int)frbb.y);
		p.drawLine((int)fbur.x, (int)fbur.y, (int)frbt.x, (int)frbt.y);

		p.drawLine((int)ftul.x, (int)ftul.y, (int)fblt.x, (int)fblt.y);
		p.drawLine((int)ftur.x, (int)ftur.y, (int)fbrt.x, (int)fbrt.y);
		p.drawLine((int)fbul.x, (int)fbul.y, (int)fblb.x, (int)fblb.y);
		p.drawLine((int)fbur.x, (int)fbur.y, (int)fbrb.x, (int)fbrb.y);

		p.drawLine((int)ftll.x, (int)ftll.y, (int)fflt.x, (int)fflt.y);
		p.drawLine((int)ftlr.x, (int)ftlr.y, (int)ffrt.x, (int)ffrt.y);
		p.drawLine((int)fbll.x, (int)fbll.y, (int)fflb.x, (int)fflb.y);
		p.drawLine((int)fblr.x, (int)fblr.y, (int)ffrb.x, (int)ffrb.y);

		p.drawLine((int)ftll.x, (int)ftll.y, (int)fTll.x, (int)fTll.y);
		p.drawLine((int)ftul.x, (int)ftul.y, (int)fTul.x, (int)fTul.y);
		p.drawLine((int)ftur.x, (int)ftur.y, (int)fTur.x, (int)fTur.y);
		p.drawLine((int)ftlr.x, (int)ftlr.y, (int)fTlr.x, (int)fTlr.y);

		p.drawLine((int)fbll.x, (int)fbll.y, (int)fBll.x, (int)fBll.y);
		p.drawLine((int)fbul.x, (int)fbul.y, (int)fBul.x, (int)fBul.y);
		p.drawLine((int)fbur.x, (int)fbur.y, (int)fBur.x, (int)fBur.y);
		p.drawLine((int)fblr.x, (int)fblr.y, (int)fBlr.x, (int)fBlr.y);
	}

	// Draw the edge frame around the current sector.
	p.setPen(QPen(QColor(0, 64, 255), 0.f));
	p.drawLine((int)ftll.x, (int)ftll.y, (int)ftul.x, (int)ftul.y);
	p.drawLine((int)ftul.x, (int)ftul.y, (int)ftur.x, (int)ftur.y);
	p.drawLine((int)ftur.x, (int)ftur.y, (int)ftlr.x, (int)ftlr.y);
	p.drawLine((int)ftlr.x, (int)ftlr.y, (int)ftll.x, (int)ftll.y);
	p.drawLine((int)ftll.x, (int)ftll.y, (int)fbll.x, (int)fbll.y);

	p.setPen(QPen(QColor(0, 32, 128), 0.f));

	p.drawLine((int)fbll.x, (int)fbll.y, (int)fbul.x, (int)fbul.y);
	p.drawLine((int)fbul.x, (int)fbul.y, (int)fbur.x, (int)fbur.y);
	p.drawLine((int)fbur.x, (int)fbur.y, (int)fblr.x, (int)fblr.y);
	p.drawLine((int)fblr.x, (int)fblr.y, (int)fbll.x, (int)fbll.y);

	p.drawLine((int)ftul.x, (int)ftul.y, (int)fbul.x, (int)fbul.y);

	p.drawLine((int)ftur.x, (int)ftur.y, (int)fbur.x, (int)fbur.y);

	p.drawLine((int)ftlr.x, (int)ftlr.y, (int)fblr.x, (int)fblr.y);

	// Draw waypoints.
	if(!m_Waypoints.isEmpty() && m_eWaypoints != WP_Off)
	{
		// The waypoint route starts at my location.
		DaSolarSystem *plast = pmyss;

		// Determine the location of the solar system that contains the waypoint.
		bool bMove = true;
		QPoint pxmove;
		for(DqAtomLocArray::const_iterator ali = aStars.constBegin(); ali != aStars.constEnd(); ali++)
		{
			if((*ali).m_pA == plast)
			{
				AuPoint fpx = tcomb.Project((*ali).m_fLocation);
				QPoint pxw((int)fpx.x, (int)fpx.y);
				pxmove = pxw;
				bMove = false;
				break;
			}
		}

		// Draw course lines and numbers.
		p.setFont(QFont(g_Au.TypefaceSans(), 6));	// This font is pretty small but works ok for just numbers.
		const int iwaysz = 3;
		int iw=1;
		for(DeWaypointRoute::const_iterator wi = m_Waypoints.constBegin(); wi != m_Waypoints.constEnd(); wi++, iw++)
		{
			if(wi != m_Waypoints.constBegin() && (*wi).m_idSystem == plast->Id())
				continue;	// Don't pile them on top of each other.

			// Find the waypoint destination in the galaxy.
			DaSolarSystem *pss = (DaSolarSystem*)ether->FindNormalAtom((*wi).m_idSystem);
			if (!pss)
				continue;
			plast = pss;

			// Determine the location of the solar system that contains the waypoint.
			for(DqAtomLocArray::const_iterator ali = aStars.constBegin(); ali != aStars.constEnd(); ali++)
			{
				if((*ali).m_pA == pss)
				{
					AuPoint fpx = tcomb.Project((*ali).m_fLocation);
					QPoint pxw((int)fpx.x, (int)fpx.y);

					if (m_eWaypoints != WP_Waypoints)
					{
						if (!bMove)
						{
							p.setPen(QPen(QColor(0,64,128), 0.f));
							p.drawLine(pxmove, pxw);
						}
						pxmove = pxw;
						bMove = false;
					}

					if (m_eWaypoints != WP_Routes)
					{
						QString sid;
						sid = QString("%1").arg(iw);
						p.setPen(QPen(QColor(128,128,128), 0.f));
						p.drawText(pxw.x() + iwaysz, pxw.y() + iwaysz/2, sid);
					}
					break;
				}
			}
		}
	}

	// Draw the wormholes.
	if(m_bWormholesVisible)
	{
		// Explored wormholes
		p.setPen(QPen(QColor(192, 64, 32), 0.f));
		AuSegment *pw = exploredWormholes.begin();
		for (int iw=exploredWormholes.count(); iw; iw--, pw++)
		{
			// Don't overwrite explored wormhole segments with known wormhole segments.
			int segIndex = knownWormholes.FindSegment(*pw);
			if(segIndex >= 0)
				knownWormholes.remove(segIndex);

			pw->Transform(tcomb);
			QPoint px1((int)pw->m_fLoc1.x, (int)pw->m_fLoc1.y);
			QPoint px2((int)pw->m_fLoc2.x, (int)pw->m_fLoc2.y);
			p.drawLine(px1,px2);
		}

		// Known wormholes
		p.setPen(QPen(QColor(32, 64, 192), 0.f));
		pw = knownWormholes.begin();
		for (int iw=knownWormholes.count(); iw; iw--, pw++)
		{
			pw->Transform(tcomb);
			QPoint px1((int)pw->m_fLoc1.x, (int)pw->m_fLoc1.y);
			QPoint px2((int)pw->m_fLoc2.x, (int)pw->m_fLoc2.y);
			p.drawLine(px1,px2);
		}
	}

	// Draw the stars.
	p.setFont(QFont(g_Au.TypefaceSans(),6));
	bool bSelCursorLoc = false;
	QString sSelName;
	AuPoint fSelCursorLoc;
	for(DqAtomLocArray::const_iterator ali = aStars.constBegin(); ali != aStars.constEnd(); ali++)
	{
		AuPoint fpx = trot.Project((*ali).m_fLocation);
		double rz = fpx.z;		// The relative z depth is used to establish brightness.
		fpx = tdev.Project(fpx);
		QPoint px((int)fpx.x, (int)fpx.y);

		if (m_eSelection == SEL_SectorAtom && (*ali).m_pA && (*ali).m_pA->Is(m_idSelected))
		{
			fSelCursorLoc = (*ali).m_fLocation;
			sSelName = (*ali).m_pA->Name();
			bSelCursorLoc = true;
		}

		// Determine the star pixel color.
		double rint = rz/15.;	// -1 to 1 range
		//rint = rint*128.0 + (255-128);	// 0 - 255 range
		//rint = (rint+1) * 128;			// 0 - 256 range
		//rint = (rint+1)*120 + 16;			// 16 - 256 range
		rint = (rint+1)*118 + 20;			// 20 - 256 range
		if (rint < 20)
			rint = 20;
		else if (rint > 255)
			rint = 255;
		int colint = (int)rint;

		p.setPen(QPen(QColor(colint,colint,colint), 0.f));
		p.drawPoint(px.x(), px.y());
		if(m_Zoom <= Z_Sector1)
		{
			p.setPen(QPen(QColor(colint/2, colint/2, colint/2), 0.f));
			p.drawPoint(px.x()+1, px.y());
			p.drawPoint(px.x()-1, px.y());
			p.drawPoint(px.x(), px.y()+1);
			p.drawPoint(px.x(), px.y()-1);
		}

		// Display solar system names.
		if(m_iNamesVisible == 1)
		{
			// Display named system names.
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>((*ali).m_pA);
			if(system && system->HasName())
			{
				p.setPen(QPen(QColor(colint, colint/2, colint/4), 0.f));
				p.drawText(px, system->Name());
			}
		}
		else if(m_iNamesVisible)
		{
			// Display ALL system names.
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>((*ali).m_pA);
			if(system)
			{
				if(system->HasName())
					p.setPen(QPen(QColor(colint, colint/2, colint/4), 0.f));
				else
					p.setPen(QPen(QColor(colint/2, colint/2, colint), 0.f));
				p.drawText(px, system->Name());
			}
		}
	}

	// Display text.
	p.setFont(QFont(g_Au.TypefaceSans(),7));
	QRect rc(2,0,sz.width()-2,sz.height());

	// System name.
	p.setPen(QPen(QColor(0,200,100), 0.f));
	p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, pssv->Name());

	// Sector coordinates.
	p.setPen(QPen(QColor(255,200,0), 0.f));
	QString sText = QString("%1 %2").arg(psect->Name()).arg(psect->Description());
	//QString sText = psect->Name() + ' ' + psect->Description();
	p.drawText(rc, Qt::AlignLeft|Qt::AlignBottom, sText);

	if (bSelCursorLoc)
	{
		// Information about the selected system.
		double rdist = (fSelCursorLoc - fRelLoc).Length();
		QString spc = AuSensorBlip::FormatBigNumber(rdist*FeetPerParsec());
		sText = QString("%1").arg(spc);
		p.setPen(QPen(QColor(0,200,200), 0.f));
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sText);
		rc.setBottom(rc.bottom() - 9);
		p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sSelName);
	}

	// Draw the location of the viewer.
	if(AuElapsedTimer::CurrentSecIsOdd())
	{
		AuPoint fpx = tcomb.Project(shipSectorLoc);
		QPoint pxMyLoc((int)fpx.x, (int)fpx.y);

		p.setPen(QPen(QColor(0,255,0), 0.f));
		p.drawPoint(pxMyLoc.x(), pxMyLoc.y());
		p.drawPoint(pxMyLoc.x()+1, pxMyLoc.y());
		p.drawPoint(pxMyLoc.x()-1, pxMyLoc.y());
		p.drawPoint(pxMyLoc.x(), pxMyLoc.y()-1);
	}

	if (bSelCursorLoc)
	{
		AuPoint fpx = tcomb.Project(fSelCursorLoc);
		QPoint pxs((int)fpx.x, (int)fpx.y);

		p.setPen(QPen(QColor(0,255,255), 0.f));

		p.drawLine(pxs.x() + 2, pxs.y() - 3, pxs.x() + 4, pxs.y() - 3);
		p.drawLine(pxs.x() + 4, pxs.y() - 3, pxs.x() + 4, pxs.y() + 3);
		p.drawLine(pxs.x() + 4, pxs.y() + 3, pxs.x() + 2, pxs.y() + 3);

		p.drawLine(pxs.x() - 2, pxs.y() - 3, pxs.x() - 4, pxs.y() - 3);
		p.drawLine(pxs.x() - 4, pxs.y() - 3, pxs.x() - 4, pxs.y() + 3);
		p.drawLine(pxs.x() - 4, pxs.y() + 3, pxs.x() - 2, pxs.y() + 3);
	}
}

void DpStationNav::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_NavConsoleControls.begin(); myci != s_NavConsoleControls.end(); myci++)
		{
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the nu data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void DpStationNav::RestoreDefaultControls()
{
	s_NavConsoleControls.clear();
	for(int ic=0; ic<s_NumNavConsoleDefaults; ic++)
		s_NavConsoleControls.append(s_NavConsoleDefaults[ic]);
	SaveControls();
}

void DpStationNav::LoadControls() const
{
	// Init with my default controls.
	s_NavConsoleControls.clear();
	for(int ic=0; ic<s_NumNavConsoleDefaults; ic++)
		s_NavConsoleControls.append(s_NavConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_NavConsoleControls.LoadSettings("Controls/Nav Console");
}

void DpStationNav::SaveControls() const
{
	// Store my controls in the settings file.
	s_NavConsoleControls.SaveSettings("Controls/Nav Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationNav::EnumControlInputs(DiControlInputArray &aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_NavConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_NavConsoleControls.constBegin(); ci != s_NavConsoleControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationNav::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaBd& bd,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	if(bd.InArena())
		return;	// No nav in arena.

	const double rPanDist = FeetPerOrbit() * (1<<m_Zoom) / 4.0;
	const double rRotInc = 5.0 * DEG2RAD;	// Rotation increment.
	for (int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_NeighborsToggle:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
					m_bNeighborSectors = !m_bNeighborSectors;
			}
			if(cc != CONTROL_Server)
			{
				if(m_Zoom >= Z_Sector0)
				{
					float buttonPitch = m_bNeighborSectors ? .8f : .9f;
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Neighbors, 1, buttonPitch);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_NamesToggle:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					m_iNamesVisible++;
					if(m_iNamesVisible < 0 || m_iNamesVisible > 2)
						m_iNamesVisible = 0;
					bd.SetSystemNamesVisible(sceneAtom->BdRoomName(), m_iNamesVisible);
				}
			}
			if(cc != CONTROL_Server)
			{
				if(m_Zoom >= Z_Sector0)
				{
					int vis = m_iNamesVisible;
					vis++;
					if(vis > 2)
						vis = 0;
					float bp = .8f + vis/10.f;	// button pitch
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SystemNames, 1,bp);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_RingsToggle:
			if(cc != CONTROL_Client)
			{
				// Cycle between three combinations here.
				if(m_bRingsVisible && m_bMoonRingsVisible)
				{
					// Both are on so turn them off.
					m_bMoonRingsVisible = false;
					m_bRingsVisible = false;
				}
				else if(m_bRingsVisible)
				{
					// Primary orbit rings are on so turn both of them on.
					m_bMoonRingsVisible = true;
					m_bRingsVisible = true;
				}
				else
				{
					// Nothing is on so turn on primary orbit rings.
					m_bMoonRingsVisible = false;
					m_bRingsVisible = true;
				}
				bd.SetMoonOrbitRingsVisible(sceneAtom->BdRoomName(), m_bMoonRingsVisible);
				bd.SetOrbitRingsVisible(sceneAtom->BdRoomName(), m_bRingsVisible);
			}
			if(cc != CONTROL_Server)
			{
				// Cycle between three combinations here.
				if(m_bRingsVisible && m_bMoonRingsVisible)
				{
					// Both are on so turn them off.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.8f);
					sceneAtom->AddEffect(buttonClick);
				}
				else if(m_bRingsVisible)
				{
					// Primary orbit rings are on so turn both of them on.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,1);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					// Nothing is on so turn on primary orbit rings.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.9f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_WormholeToggle:
			if(cc != CONTROL_Client)
			{
				m_bWormholesVisible = !m_bWormholesVisible;
				bd.SetWormholeLinesVisible(sceneAtom->BdRoomName(), m_bWormholesVisible);
			}
			if(cc != CONTROL_Server)
			{
				bool vis = m_bWormholesVisible;
				vis = !vis;
				float bp = vis ? 1 : .9f;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Wormholes, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_WaypointsToggle:
			if(cc != CONTROL_Client)
			{
				m_eWaypoints = (WAYPOINTS)(m_eWaypoints + 1);
				if (m_eWaypoints > WP_WaypointsAndRoutes)
					m_eWaypoints = WP_Off;
			}
			if(cc != CONTROL_Server)
			{
				int w = m_eWaypoints + 1;
				if(w > WP_WaypointsAndRoutes)
					w = WP_Off;
				float bp = .75f + w/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Waypoints, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ZoomSolarSystem0:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System0;
			break;
		case CI_ZoomSolarSystem1:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System1;
			break;
		case CI_ZoomSolarSystem2:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System2;
			break;
		case CI_ZoomSolarSystem3:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System3;
			break;
		case CI_ZoomSolarSystem4:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System4;
			break;
		case CI_ZoomSolarSystem5:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System5;
			break;
		case CI_ZoomSector0:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector0;
			break;
		case CI_ZoomSector1:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector1;
			break;
		case CI_ZoomSector2:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector2;
			break;
		case CI_ZoomSector3:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector3;
			break;
		case CI_ZoomSector4:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector4;
			break;
		case CI_ZoomSector5:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector5;
			break;
		case CI_ZoomIn:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom > Z_System0)
					m_Zoom = (ZOOMSETTING)(m_Zoom - 1);
			}
			break;
		case CI_ZoomOut:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < (Z_Levels-1))
					m_Zoom = (ZOOMSETTING)(m_Zoom + 1);
			}
			break;
		case CI_TopViewCenter:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					// Solar system zoom level.
					// Center view on the solar system.
					m_fPan = AuPoint(0,0);
					m_bTrackShip = false;
				}
				else
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0,0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PanToShip:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					// Solar system zoom level.
					DaAtom* pmyss = bd.FindEnvironmentAtom();
					m_idSystem = pmyss->Id();
					m_fPan = bd.EnvLocation();
					m_bTrackShip = true;
				}
				else
				{
					// Sector scale.
					DaAtom* pmyss = bd.FindEnvironmentAtom();
					m_idSystem = pmyss->Id();
				}
			}
			break;
		case CI_PanToSelection:
			if(cc != CONTROL_Client)
				PanToSelection(bd);
			break;
		case CI_PanDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(0,-rPanDist,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeft:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(-rPanDist,0,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeftDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(-PI*3.0/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeftUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(PI*3.0/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRight:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(rPanDist,0,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRightDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(-PI/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRightUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(PI/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(0,rPanDist,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_FrontView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(PI/2.0, 0.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0.0, -PI/2.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(-PI/2.0, 0.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0.0, PI/2.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_AddRouteToSelected:
		case CI_AddWaypointAtSelected:
		case CI_AddWaypoint:
		case CI_InsertWaypoint:
		case CI_DeleteWaypointAtSelected:
		case CI_DeleteAllWaypoints:
			break;
		case CI_DisplayScreen:
			if(cc != CONTROL_Client)
				ProcessDisplayHit(ci.HitLocation(), bd);
			break;
		}
	}
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationNav::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	if(ship.InArena())
		return;	// No nav in arena.
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".

	const double rPanDist = FeetPerOrbit() * (1<<m_Zoom) / 4.0;
	const double rRotInc = 5.0 * DEG2RAD;	// Rotation increment.
	for (int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_NeighborsToggle:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
					m_bNeighborSectors = !m_bNeighborSectors;
			}
			if(cc != CONTROL_Server)
			{
				if(m_Zoom >= Z_Sector0)
				{
					float buttonPitch = m_bNeighborSectors ? .8f : .9f;
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Neighbors, 1, buttonPitch);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_NamesToggle:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					m_iNamesVisible++;
					if(m_iNamesVisible < 0 || m_iNamesVisible > 2)
						m_iNamesVisible = 0;
					ship.SetSystemNamesVisible(sceneAtom->ShipRoomName(), m_iNamesVisible);
				}
			}
			if(cc != CONTROL_Server)
			{
				if(m_Zoom >= Z_Sector0)
				{
					int vis = m_iNamesVisible;
					vis++;
					if(vis > 2)
						vis = 0;
					float bp = .8f + vis/10.f;	// button pitch
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_SystemNames, 1,bp);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_RingsToggle:
			if(cc != CONTROL_Client)
			{
				// Cycle between three combinations here.
				if(m_bRingsVisible && m_bMoonRingsVisible)
				{
					// Both are on so turn them off.
					m_bMoonRingsVisible = false;
					m_bRingsVisible = false;
				}
				else if(m_bRingsVisible)
				{
					// Primary orbit rings are on so turn both of them on.
					m_bMoonRingsVisible = true;
					m_bRingsVisible = true;
				}
				else
				{
					// Nothing is on so turn on primary orbit rings.
					m_bMoonRingsVisible = false;
					m_bRingsVisible = true;
				}
				ship.SetMoonOrbitRingsVisible(sceneAtom->ShipRoomName(), m_bMoonRingsVisible);
				ship.SetOrbitRingsVisible(sceneAtom->ShipRoomName(), m_bRingsVisible);
			}
			if(cc != CONTROL_Server)
			{
				// Cycle between three combinations here.
				if(m_bRingsVisible && m_bMoonRingsVisible)
				{
					// Both are on so turn them off.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.8f);
					sceneAtom->AddEffect(buttonClick);
				}
				else if(m_bRingsVisible)
				{
					// Primary orbit rings are on so turn both of them on.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,1);
					sceneAtom->AddEffect(buttonClick);
				}
				else
				{
					// Nothing is on so turn on primary orbit rings.
					AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_OrbitRings, 1,.9f);
					sceneAtom->AddEffect(buttonClick);
				}
			}
			break;
		case CI_WormholeToggle:
			if(cc != CONTROL_Client)
			{
				m_bWormholesVisible = !m_bWormholesVisible;
				ship.SetWormholeLinesVisible(sceneAtom->ShipRoomName(), m_bWormholesVisible);
			}
			if(cc != CONTROL_Server)
			{
				bool vis = m_bWormholesVisible;
				vis = !vis;
				float bp = vis ? 1 : .9f;	// button pitch
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Wormholes, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_WaypointsToggle:
			if(cc != CONTROL_Client)
			{
				m_eWaypoints = (WAYPOINTS)(m_eWaypoints + 1);
				if (m_eWaypoints > WP_WaypointsAndRoutes)
					m_eWaypoints = WP_Off;
			}
			if(cc != CONTROL_Server)
			{
				int w = m_eWaypoints + 1;
				if(w > WP_WaypointsAndRoutes)
					w = WP_Off;
				float bp = .75f + w/10.f;
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Waypoints, 1,bp);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_ZoomSolarSystem0:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System0;
			break;
		case CI_ZoomSolarSystem1:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System1;
			break;
		case CI_ZoomSolarSystem2:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System2;
			break;
		case CI_ZoomSolarSystem3:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System3;
			break;
		case CI_ZoomSolarSystem4:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System4;
			break;
		case CI_ZoomSolarSystem5:
			if(cc != CONTROL_Client)
				m_Zoom = Z_System5;
			break;
		case CI_ZoomSector0:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector0;
			break;
		case CI_ZoomSector1:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector1;
			break;
		case CI_ZoomSector2:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector2;
			break;
		case CI_ZoomSector3:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector3;
			break;
		case CI_ZoomSector4:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector4;
			break;
		case CI_ZoomSector5:
			if(cc != CONTROL_Client)
				m_Zoom = Z_Sector5;
			break;
		case CI_ZoomIn:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom > Z_System0)
					m_Zoom = (ZOOMSETTING)(m_Zoom - 1);
			}
			break;
		case CI_ZoomOut:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < (Z_Levels-1))
					m_Zoom = (ZOOMSETTING)(m_Zoom + 1);
			}
			break;
		case CI_TopViewCenter:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					// Solar system zoom level.
					// Center view on the solar system.
					m_fPan = AuPoint(0,0);
					m_bTrackShip = false;
				}
				else
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0,0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PanToShip:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					// Solar system zoom level.
					DaAtom* pmyss = ship.FindEnvironmentAtom();
					m_idSystem = pmyss->Id();
					m_fPan = ship.EnvLocation();
					m_bTrackShip = true;
				}
				else
				{
					// Sector scale.
					DaAtom* pmyss = ship.FindEnvironmentAtom();
					m_idSystem = pmyss->Id();
				}
			}
			break;
		case CI_PanToSelection:
			if(cc != CONTROL_Client)
				PanToSelection(ship);
			break;
		case CI_PanDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(0,-rPanDist,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeft:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(-rPanDist,0,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeftDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(-PI*3.0/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanLeftUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(PI*3.0/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.RotateY(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRight:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(rPanDist,0,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRightDown:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(-PI/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(rRotInc);
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanRightUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					AuMatrix4x4 t;
					t.RotateZ(PI/4.0);
					m_fPan -= t.Project(AuPoint(rPanDist,0,0));
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.RotateY(rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_PanUp:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					m_fPan -= AuPoint(0,rPanDist,0);
					m_bTrackShip = false;
				}
				else
				{
					AuMatrix4x4 t;
					t.RotateX(-rRotInc);
					t.Rotate(m_fRot);
					m_fRot = t.GetRotation();
				}
			}
			break;
		case CI_FrontView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(PI/2.0, 0.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_LeftView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0.0, -PI/2.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_BackView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(-PI/2.0, 0.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_RightView:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom >= Z_Sector0)
				{
					// Sector scale, zero rotation for top view.
					m_fRot = AuPoint(0.0, PI/2.0, 0.0);
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_View, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		case CI_AddRouteToSelected:
			if(cc != CONTROL_Client)
			{
				if(m_eSelection != SEL_None)
				{
					m_eCurrentCommand = CI_Invalid;
					if(m_eSelection == SEL_SolarSystemAtom)
						ship.AddWaypointRouteTo(m_idSelected, m_SelectedOffset);
					else
						ship.AddWaypointRouteTo(m_idSelected, AuPoint(0,0));
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AddWaypointAtSelected:
			if(cc != CONTROL_Client)
			{
				if(m_eSelection != SEL_None)
				{
					m_eCurrentCommand = CI_Invalid;
					if(m_eSelection == SEL_SectorAtom)
					{
						ship.AddWaypoint(m_idSelected, m_idSelected);
					}
					else if(m_eSelection == SEL_SolarSystemAtom)
					{
						if(m_SelectedOffset.IsZero())
							ship.AddWaypoint(m_idSystem, m_idSelected);
						else
							ship.AddWaypoint(m_idSystem, m_idSelected, m_SelectedOffset);
					}
					else if(m_eSelection == SEL_Waypoint)
					{
						const DeWaypoint* pw = ship.LockWaypoint(m_lSelectedWaypoint);
						if(pw)
						{
							DeWaypoint wp(*pw);
							ship.UnlockWaypoint();

							ship.AddWaypoint(&wp);
						}
						else
							ship.UnlockWaypoint();
					}
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AddWaypoint:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0)
				{
					if (m_eCurrentCommand == CI_AddWaypoint)
						m_eCurrentCommand = CI_Invalid;
					else
						m_eCurrentCommand = CI_AddWaypoint;
					m_eSelection = SEL_None;
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_InsertWaypoint:
			if(cc != CONTROL_Client)
			{
				if(m_Zoom < Z_Sector0 && m_eSelection == SEL_Waypoint)
				{
					// Inserts a waypoint before the selected waypoint.
					if (m_eCurrentCommand == CI_InsertWaypoint)
						m_eCurrentCommand = CI_Invalid;
					else
						m_eCurrentCommand = CI_InsertWaypoint;
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,.95f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_DeleteWaypointAtSelected:
			if(cc != CONTROL_Client)
			{
				if(m_eSelection == SEL_Waypoint && ship.HasWaypoint(m_lSelectedWaypoint))
				{
					m_eCurrentCommand = CI_Invalid;
					ship.RemoveWaypoint(m_lSelectedWaypoint);
					m_eSelection = SEL_None;
					m_lSelectedWaypoint = 0;
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_DeleteAllWaypoints:
			if(cc != CONTROL_Client)
			{
				if(ship.HasWaypoint())
				{
					m_eCurrentCommand = CI_Invalid;
					if (m_eSelection == SEL_Waypoint)
					{
						m_eSelection = SEL_None;
						m_lSelectedWaypoint = 0;
					}
					ship.RemoveAllWaypoints();
				}
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Route, 1,.7f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_DisplayScreen:
			if(cc != CONTROL_Client)
				ProcessDisplayHit(ci.HitLocation(), ship);
			break;
		}
	}
}

void DpStationNav::ProcessDisplayHit(const QPoint &pxHit, DaBd& bd)
{
	//qDebug("Display Hit at %d,%d\n", pxHit.x(), pxHit.y());
	SelectNearest(pxHit, bd);
	m_eCurrentCommand = CI_Invalid;
}

void DpStationNav::ProcessDisplayHit(const QPoint &pxHit, DaVessel& ship)
{
	//qDebug("Display Hit at %d,%d\n", pxHit.x(), pxHit.y());
	switch(m_eCurrentCommand)
	{
	default:
		SelectNearest(pxHit, ship);
		break;
	case CI_AddWaypoint:
		AddWaypoint(pxHit, ship);
		break;
	case CI_InsertWaypoint:
		InsertWaypoint(pxHit, ship);
		break;
	}
	m_eCurrentCommand = CI_Invalid;
}

AuPoint DpStationNav::ConvertDisplayPixelToLoc(const QPoint &pxHit) const
{
	// Calculate some values for converting the real world location into pixels.
	const QSize szdisp(218,120);
	QPoint pCenter(szdisp.width()/2, szdisp.height()/2);
	const int iMaxStarOrbits = 16;
	// In case orbit 15 is a star with its max of 7 orbits (1/2 the star's
	// orbit number (0-15)).
	const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
	const int iPixelsPerOrbit = pCenter.y()/iMaxSystemOrbits * (1<<(Z_System5 - m_Zoom));

	// Offset pCenter for pan.
	AuPoint fpxpan(m_fPan);
	fpxpan /= FeetPerOrbit();
	fpxpan *= iPixelsPerOrbit;
	pCenter.setX(pCenter.x() - (int)fpxpan.x);
	pCenter.setY(pCenter.y() + (int)fpxpan.y);

	// Calculate the point in the solar system's XY plane.
	AuPoint floc;
	floc.x = pxHit.x() - pCenter.x();
	floc.y = pCenter.y() - pxHit.y();
	if(iPixelsPerOrbit)
		floc /= iPixelsPerOrbit;			// Convert from pixels to orbits.
	floc *= FeetPerOrbit();				// Convert from orbits to feet.

	return floc;
}

void DpStationNav::AddWaypoint(const QPoint &pxHit, DaVessel& ship)
{
	if(m_Zoom >= Z_Sector0)
		return;
	if (m_idSystem.IsNull())
		return;

	AuPoint floc = ConvertDisplayPixelToLoc(pxHit);

	// Create a waypoint at the nu location.
	ship.AddWaypoint(m_idSystem, m_idSystem, floc);
}

void DpStationNav::InsertWaypoint(const QPoint &pxHit, DaVessel& ship)
{
	if (m_eSelection != SEL_Waypoint)
		return;
	if (m_idSystem.IsNull())
		return;
	if (!ship.HasWaypoint(m_lSelectedWaypoint))
		return;

	const double rseldist = 5.0;	// Pixel distance to select.

	double radist = -1.0;	// Distance to the nearest atom in display pixels.
	AuPoint voff;			// Offset from selected atom, for wormholes.
	const DaAtom *pa = FindClosestDisplayAtom(pxHit, radist, voff, ship);
	if(pa && radist <= rseldist)
	{
		if(voff.IsZero())
			ship.InsertWaypointAt(m_lSelectedWaypoint, m_idSystem, pa->Id());
		else
		{
			// Must be a wormhole.
			ship.InsertWaypointAt(m_lSelectedWaypoint, m_idSystem, m_idSystem, voff);
		}
	}
	else
	{
		// Insert a waypoint at the nu location.
		AuPoint floc = ConvertDisplayPixelToLoc(pxHit);
		ship.InsertWaypointAt(m_lSelectedWaypoint, m_idSystem, m_idSystem, floc);
	}
}

void DpStationNav::SelectNearest(const QPoint &pxHit, DaAtom& ref)
{
	const double rseldist = 5.0;	// Pixel distance to select.

	// Find the closest atom in the display.
	double radist = -1.0;	// Distance to the nearest atom in display pixels.
	AuPoint voff;			// Offset from selected atom, for wormholes.
	const DaAtom *pa = FindClosestDisplayAtom(pxHit, radist, voff, ref);
	if(pa && radist > rseldist)
		pa = NULL;

	SELECTION eSel = SEL_None;
	int lSelWay = -1;
	AuId idSel;

	if(m_Zoom < Z_Sector0)
	{
		// Solar system zoom so try to select a waypoint before anything else.
		double rwdist = -1.0;
		int lwp = FindClosestDisplayWaypoint(pxHit, rwdist, ref);
		if(lwp >= 0)
		{
			if(rwdist >= 0.0 && rwdist <= rseldist && (!pa || rwdist <= radist))
			{
				eSel = SEL_Waypoint;
				lSelWay = lwp;
			}
		}
	}

	AuPoint selOff;
	if(eSel == SEL_None)
	{
		// Haven't selected a waypoint so try another object.
		if(pa)
		{
			// We did find an atom close enough to select.
			if(m_Zoom < Z_Sector0)
				eSel = SEL_SolarSystemAtom;
			else
				eSel = SEL_SectorAtom;
			idSel = pa->Id();
			selOff = voff;
		}
		else if (m_eSelection != SEL_None)
		{
			eSel = SEL_None;
		}
	}

	if(eSel != m_eSelection || lSelWay != m_lSelectedWaypoint ||
			idSel != m_idSelected || selOff != m_SelectedOffset)
	{
		m_eSelection = eSel;
		m_lSelectedWaypoint = lSelWay;
		m_idSelected = idSel;
		m_SelectedOffset = selOff;
	}
	else if(m_eSelection != SEL_None)
	{
		// The selection didn't change so the same object must have been
		// clicked again.
		PanToSelection(ref);
	}
}

const DaAtom *DpStationNav::FindClosestDisplayAtom(const QPoint &pxHit, double &rDisplayDistance, AuPoint& vOffset,
		const DaAtom& ref) const
{
	//qDebug("DpStationNav::FindClosestDisplayAtom");
	if(m_Zoom < Z_Sector0)
	{
		// Solar system scale.
		return FindClosestSolarSystemDisplayAtom(pxHit, rDisplayDistance, vOffset, ref);
	}
	else
	{
		// Sector scale.
		vOffset = AuPoint(0,0);
		return FindClosestSectorDisplayAtom(pxHit, rDisplayDistance, ref);
	}
}

const DaAtom *DpStationNav::FindClosestSolarSystemDisplayAtom(const QPoint &pxHit, double &rDisplayDistance, AuPoint& vOffset,
		const DaAtom& ref) const
{
	//qDebug("DpStationNav::FindClosestSolarSystemDisplayAtom");
	AuEther* ether = ref.Ether();
	if(!ether)
		return 0;
	// Find the solar system that is the subject of the display.
	DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
	if (!pssv)
		return NULL;

	DaStar *pstar = (DaStar *)pssv->FindStar();
	if (!pstar)
		return NULL;

	// Calculate some values for converting the real world into this pixel
	// rectangle.
	const QSize szdisp(218,120);
	QPoint pCenter(szdisp.width()/2, szdisp.height()/2);
	const int iMaxStarOrbits = 16;
	// In case orbit 15 is a star with its max of 7 orbits (1/2 the star's
	// orbit number (0-15)).
	const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
	const int iPixelsPerOrbit = pCenter.y()/iMaxSystemOrbits *
		(1<<(Z_System5 - m_Zoom));

	// Offset pCenter for pan.
	AuPoint fpxpan(m_fPan);
	fpxpan /= FeetPerOrbit();
	fpxpan *= iPixelsPerOrbit;
	pCenter.setX(pCenter.x() - (int)fpxpan.x);
	pCenter.setY(pCenter.y() + (int)fpxpan.y);

	// Draw primary and subordinate stars and their planets.
	rDisplayDistance = -1.0;
	const DaAtom *pBest = FindClosestStarOrPlanet(pCenter, pstar, pssv, iPixelsPerOrbit, pxHit, rDisplayDistance, NULL);
	DaAtomList stars;
	pstar->ListStars(stars);
	while (!stars.isEmpty())
	{
		pstar = (DaStar*)stars.last();
		stars.removeLast();
		pBest = FindClosestStarOrPlanet(pCenter, pstar, pssv, iPixelsPerOrbit, pxHit, rDisplayDistance, pBest);
	}

	// Calc distance to wormholes.
	vOffset = AuPoint(0,0);
	const AuWormholeArray& wha = pssv->LockWormholes();
	for(AuWormholeArray::const_iterator it = wha.constBegin(); it != wha.constEnd(); it++)
	{
		// Calc distance to the wormhole.
		AuPoint fwhloc((*it).Location(pssv));	// Location relative to solar system center, in feet.
		AuPoint fworm(fwhloc);	// Location relative to solar system center, in feet.
		//qDebug("Checking a wormhole at %g,%g,%g", fworm.x, fworm.y, fworm.z);
		fworm /= FeetPerOrbit();		// Convert to "orbits".
		fworm *= iPixelsPerOrbit;		// Convert to "pixels".
		QPoint pxw;
		pxw.setX((int)(pCenter.x() + fworm.x));
		pxw.setY((int)(pCenter.y() - fworm.y));
		double rdist = AuPoint(pxHit).Distance_2D(AuPoint(pxw));
		if(!pBest || rdist < rDisplayDistance)
		{
			//qDebug("Near a wormhole, %g pixels", rdist);
			pBest = pssv;
			vOffset = fwhloc;
			rDisplayDistance = rdist;
		}
	}
	pssv->UnlockWormholes();

	return pBest;
}

const DaAtom *DpStationNav::FindClosestStarOrPlanet(const QPoint &pCenter, DaStar *pstar, const DaSolarSystem* /*pss*/,
		int iPixelsPerOrbit, const QPoint &pxHit, double &rDisplayDistance, const DaAtom *pBest) const
{
	//qDebug("DpStationNav::FindClosestStarOrPlanet");
	// Calc distance to the star.
	AuPoint fstar = pstar->EnvLocation();
	fstar /= FeetPerOrbit();		// Convert to "orbits".
	fstar *= iPixelsPerOrbit;		// Convert to "pixels".
	QPoint pxstar((int)(pCenter.x() + fstar.x), (int)(pCenter.y() - fstar.y));	// Center of star in bitmap.
	double rdist = AuPoint(pxHit).Distance_2D(AuPoint(pxstar));
	if (!pBest || rdist < rDisplayDistance)
	{
		pBest = pstar;
		rDisplayDistance = rdist;
	}

	// Calc distance to planets.
	DaAtomList planets;
	pstar->ListPlanets(planets);
	for(DaAtomList::iterator it = planets.begin(); it != planets.end(); it++)
	{
		DaWorld *pb = (DaWorld*)(*it);

		// Calc distance to the planet.
		AuPoint ploc = pb->EnvLocation();
		ploc /= FeetPerOrbit();													// Convert to "orbits".
		ploc *= iPixelsPerOrbit;												// Convert to "pixels".
		QPoint pxc((int)(pCenter.x() + ploc.x), (int)(pCenter.y() - ploc.y));	// Center of planet in bitmap.

		double rdist = AuPoint(pxHit).Distance_2D(AuPoint(pxc));
		if (!pBest || rdist < rDisplayDistance)
		{
			pBest = pb;
			rDisplayDistance = rdist;
		}

		DaAtomList moons;
		pb->ListPlanets(moons);
		while(!moons.isEmpty())
		{
			DaWorld* satellite = dynamic_cast<DaWorld*>(moons.takeFirst());
			DaWorldGlobe* glmoon = dynamic_cast<DaWorldGlobe*>(satellite);
			if(glmoon)
			{
				// Check distance to moon.
				AuPoint mloc = satellite->EnvLocation();
				mloc /= FeetPerOrbit();											// Convert to "orbits".
				mloc *= iPixelsPerOrbit;										// Convert to "pixels".
				QPoint pxm((int)(pCenter.x() + mloc.x), (int)(pCenter.y() - mloc.y));	// Center of planet in bitmap.

				double rdist = AuPoint(pxHit).Distance_2D(AuPoint(pxm));
				if(!pBest || rdist < rDisplayDistance)
				{
					pBest = satellite;
					rDisplayDistance = rdist;
				}
			}
			else
			{
				/*
				DaWorldRing* ring = dynamic_cast<DaWorldRing*>(satellite);
				if(ring)
				{
				}
				*/
			}
		}
	}

	return pBest;
}

const DaAtom *DpStationNav::FindClosestSectorDisplayAtom(const QPoint &pxHit, double &rDisplayDistance,
		const DaAtom& ref) const
{
	// Find the solar system that is the subject of the display.
	AuEther* ether = ref.Ether();
	if(!ether)
		return 0;
	DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
	if (!pssv)
		return NULL;
	DaSector *psect = pssv->FindParentSector();
	if (!psect)
		return NULL;

	// Get location that will be the center of the star field.
	AuPoint fRelLoc;
	pssv->GetLocationRelativeToParent(fRelLoc, psect);

	// Get the stars and wormholes in this sector and neighboring sectors, relative to fRelLoc.
	DqAtomLocArray aStars;
	if(m_bNeighborSectors)
		psect->ListSolarSystemLocations(aStars);								// List all neighbors also.
	else
		psect->ListSolarSystemLocations(aStars, AuPoint(0,0));					// List only this sector.

	// Make transforms to projection the locations into pixels.
	const QSize szdisp(218,120);
	AuMatrix4x4 tdev;
	tdev.Translate(szdisp.width()/2.0, szdisp.height()/2.0, 0.0);
	double rpxscl = szdisp.width()/2.0;
	rpxscl *= Z_Levels - m_Zoom;
	rpxscl *= 11;

	tdev.Scale(-rpxscl, rpxscl, rpxscl);
	tdev.Perspective(60.0, 1.0, 0.5, 1000.0);
	tdev.Translate(0.0, 0.0, -10.0);
	double rscl = 1.0/5.0;
	tdev.Scale(rscl, rscl, rscl);

	AuMatrix4x4 trot;
	trot.Rotate(m_fRot);
	trot.Translate(-fRelLoc);
	AuMatrix4x4 tcomb = tdev * trot;

	// Search the stars.
	int lBest = -1;
	DqAtomLocArray::iterator ps;
	int is=0;
	for(ps = aStars.begin(); ps != aStars.end(); ++ps, is++)
	{
		AuPoint fpx = tcomb.Project(ps->m_fLocation);
		QPoint px((int)fpx.x, (int)fpx.y);

		double rdist = AuPoint(pxHit).Distance_2D(AuPoint(px));
		if (lBest < 0 || rdist < rDisplayDistance)
		{
			lBest = is;
			rDisplayDistance = rdist;
		}
	}

	if (lBest >= 0 && lBest < (int)aStars.count())
		return aStars[lBest].m_pA;

	return NULL;
}

int DpStationNav::FindClosestDisplayWaypoint(const QPoint &pxHit, double &rDisplayDistance, const DaAtom& ref) const
{
	if(m_Zoom >= Z_Sector0)
		return -1;

	AuEther* ether = ref.Ether();
	if(!ether)
		return -1;
	const DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
	if (!pssv)
		return -1;

	// Calculate some values for converting the real world into this pixel
	// rectangle.
	const QSize szdisp(218,120);
	QPoint pCenter(szdisp.width()/2, szdisp.height()/2);
	const int iMaxStarOrbits = 16;
	// In case orbit 15 is a star with its max of 7 orbits (1/2 the star's
	// orbit number (0-15)).
	const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
	const int iPixelsPerOrbit = pCenter.y()/iMaxSystemOrbits *
		(1<<(Z_System5 - m_Zoom));

	// Offset pCenter for pan.
	AuPoint fpxpan(m_fPan);
	fpxpan /= FeetPerOrbit();
	fpxpan *= iPixelsPerOrbit;
	pCenter.setX(pCenter.x() - (int)fpxpan.x);
	pCenter.setY(pCenter.y() + (int)fpxpan.y);

	int lBest = -1;
	int iwp=0;
	m_WaypointLock.lockForRead();
	for(DeWaypointRoute::const_iterator wi = m_Waypoints.constBegin(); wi != m_Waypoints.constEnd(); wi++, iwp++)
	{
		AuPoint floc;
		if ((*wi).Location(pssv, floc))
		{
			floc /= FeetPerOrbit();			// Convert to "orbits".
			floc *= iPixelsPerOrbit;		// Convert to "pixels".
			QPoint px((int)(pCenter.x() + floc.x), (int)(pCenter.y() - floc.y));	// Location in bitmap.
			double rdist = AuPoint(pxHit).Distance_2D(AuPoint(px));
			if (lBest < 0 || rdist < rDisplayDistance)
			{
				lBest = iwp;
				rDisplayDistance = rdist;
			}
		}
	}
	m_WaypointLock.unlock();
	return lBest;
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationNav::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Note that the top/bottom is reversed in the rectangles. This works with
	// the iy we've calculated that is also measured from the bottom, not the
	// top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		DiPanelControl(CI_RingsToggle, QRect(236,91,16,16)),
		DiPanelControl(CI_WormholeToggle, QRect(236,73,16,16)),
		DiPanelControl(CI_NeighborsToggle, QRect(236,55,16,16)),
		DiPanelControl(CI_NamesToggle, QRect(224,55,9,9)),
		DiPanelControl(CI_WaypointsToggle, QRect(236,37,16,16)),
		DiPanelControl(CI_ZoomSolarSystem0, QRect(225,104,9,3)),
		DiPanelControl(CI_ZoomSolarSystem1, QRect(225,101,9,3)),
		DiPanelControl(CI_ZoomSolarSystem2, QRect(225,98,9,3)),
		DiPanelControl(CI_ZoomSolarSystem3, QRect(225,95,9,3)),
		DiPanelControl(CI_ZoomSolarSystem4, QRect(225,92,9,3)),
		DiPanelControl(CI_ZoomSolarSystem5, QRect(225,89,9,3)),
		DiPanelControl(CI_ZoomSector0, QRect(225,83,9,3)),
		DiPanelControl(CI_ZoomSector1, QRect(225,80,9,3)),
		DiPanelControl(CI_ZoomSector2, QRect(225,77,9,3)),
		DiPanelControl(CI_ZoomSector3, QRect(225,74,9,3)),
		DiPanelControl(CI_ZoomSector4, QRect(225,71,9,3)),
		DiPanelControl(CI_ZoomSector5, QRect(225,68,9,3)),
		DiPanelControl(CI_TopViewCenter, QRect(231,115,3,3)),
		DiPanelControl(CI_PanDown, QRect(231,109,3,3), true),
		DiPanelControl(CI_PanLeft, QRect(225,115,3,3), true),
		DiPanelControl(CI_PanLeftDown, QRect(227,111,3,3), true),
		DiPanelControl(CI_PanLeftUp, QRect(227,119,3,3), true),
		DiPanelControl(CI_PanRight, QRect(237,115,3,3), true),
		DiPanelControl(CI_PanRightDown, QRect(235,111,3,3), true),
		DiPanelControl(CI_PanRightUp, QRect(235,119,3,3), true),
		DiPanelControl(CI_PanUp, QRect(231,121,3,3), true),
		DiPanelControl(CI_FrontView, QRect(231,112,3,3)),
		DiPanelControl(CI_BackView, QRect(231,118,3,3)),
		DiPanelControl(CI_LeftView, QRect(228,115,3,3)),
		DiPanelControl(CI_RightView, QRect(234,115,3,3)),
		DiPanelControl(CI_AddRouteToSelected, QRect(224,23,9,9)),
		DiPanelControl(CI_AddWaypointAtSelected, QRect(224,13,9,9)),
		DiPanelControl(CI_AddWaypoint, QRect(234,13,9,9)),
		DiPanelControl(CI_InsertWaypoint, QRect(244,13,9,9)),
		DiPanelControl(CI_DeleteWaypointAtSelected, QRect(234,3,9,9)),
		DiPanelControl(CI_DeleteAllWaypoints, QRect(244,3,9,9)),
		DiPanelControl(CI_DisplayScreen, QRect(4,4,218,120), false, true),
		DiPanelControl(CI_PanToSelection, QRect(241,112,9,9)),
		DiPanelControl(CI_PanToShip, QRect(243,121,5,4)),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationNav::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	DpStation::Render(projection, farProjection, tModel, sceneAtom, seatedCreature, ari);
	if(seatedCreature && seatedCreature->Is(sceneAtom.Id()))
	{
		if(m_eWaypoints != WP_Off && !m_Waypoints.isEmpty())
			RenderCourse(m_Waypoints, farProjection, tModel, sceneAtom, ari, m_eWaypoints != WP_Waypoints,
					m_eWaypoints != WP_Routes);

		const DaVessel* ship = sceneAtom.FindParentVesselInEnv();
		if(ship)
		{
			// Draw the lock-on reticle.
			if(!ship->LockedBlip().IsNull())
				RenderSensorLock(ship->LockedBlip(), ship->LockedStance(), ship->LockedIsCivilian(), ship->LockedSysLoc(), tModel,
						sceneAtom, ari);
		}
	}
}

void DpStationNav::PanToSelection(DaAtom& ref)
{
	AuEther* ether = ref.Ether();
	if(!ether)
		return;
	if(m_eSelection == SEL_None)
		return;	// Nothing is selected.
	// Something is selected.
	if(m_Zoom < Z_Sector0)
	{
		// Solar system zoom level.
		DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSystem);
		if (pssv)
		{
			if (m_eSelection == SEL_Waypoint)
			{
				// Pan to selected waypoint in currently viewed system.
				m_WaypointLock.lockForRead();
				if(m_lSelectedWaypoint < m_Waypoints.count())
					m_Waypoints[m_lSelectedWaypoint].Location(pssv, m_fPan);
				m_WaypointLock.unlock();
			}
			else
			{
				// Pan to selected atom in currently viewed system.
				DaAtom *psel = pssv->FindIdRecursive(m_idSelected);
				if(psel)
					m_fPan = psel->EnvLocation();
			}
		}
		else
		{
			m_fPan = AuPoint(0,0);
		}
		m_bTrackShip = false;
	}
	else if (m_eSelection == SEL_SectorAtom)
	{
		// Sector scale, center view on selected solar system.
		// The selected solar system must be in the same sector as my current solar system.
		if (m_idSelected != m_idSystem)
		{
			/*
			// The selected solar system is not the same as the currently viewed solar system.
			DaGalaxy *pg = (DaGalaxy *)FindParentGalaxy();
			DaSolarSystem *pmyss = (DaSolarSystem *)FindParentSolarSystem();
			if (pg && pmyss)
			{
				// We have a galaxy and I have found my solar system.
				DaSector *pmysect = pmyss->FindParentSector();
				DaSolarSystem *pssv = (DaSolarSystem*)ether->FindNormalAtom(m_idSelected);
				if (pmysect && pssv)
				{
					// I have found my sector and the selected solar system.
					DaSector *psect = pssv->FindParentSector();
					if (psect && psect->Is(pmysect->Id()))
					{
						// The selected solar system is in the same sector as I am.
						*/
						m_idSystem = m_idSelected;
						m_fPan = AuPoint(0,0);
						m_bTrackShip = false;
						/*
					}
				}
			}
			*/
		}
	}
}

bool DpStationNav::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(ship)
	{
		if(!ship->IsDesignDataCacheValid() || ship->InArena())
			return true;

		if(!last.SameStartSec(now))
		{
			DaSolarSystem *pss = ship->FindParentSolarSystem();
			if(pss)
			{
				// Current solar system is always considered explored by the empire who owns the ship.
				if(m_idSystem.IsNull())
					m_idSystem = pss->Id();

				if(m_bTrackShip)
				{
					// I am in the solar system that is on the display panel and I'm tracking my location.
					m_idSystem = pss->Id();
					m_fPan = ship->EnvLocation();
				}
			}

			// Get the current waypoint route from the ship.
			m_WaypointLock.lockForWrite();
			m_Waypoints.clear();
			for(int iwp=0; iwp<ship->WaypointCount(); iwp++)
			{
				m_Waypoints.append(*ship->LockWaypoint(iwp));
				ship->UnlockWaypoint();
			}
			m_WaypointLock.unlock();
		}
	}
	else
	{
		// Only the solar system id is needed for the nav console on the ground.
		if(m_idSystem.IsNull())
		{
			DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(person->FindEnvironmentAtom());
			if(pss)
			{
				// Current solar system is always considered explored by the operator of the console.
				m_idSystem = pss->Id();
			}
		}
	}
	return true;
}

void DpStationNav::LoadClientSettings(const AuId& /*planetId*/)
{
	m_bNeighborSectors = g_Au.Settings().readBoolEntry("/NavigatorStation/NeighborSectors", m_bNeighborSectors);
	int rings = g_Au.Settings().readNumEntry("/NavigatorStation/RingsVisible", 0);
	m_bRingsVisible = rings & 0x01;
	m_bMoonRingsVisible = rings & 0x02;
	m_bTrackShip = g_Au.Settings().readBoolEntry("/NavigatorStation/TrackShip", m_bTrackShip);
	m_bWormholesVisible = g_Au.Settings().readBoolEntry("/NavigatorStation/WormholesVisible", m_bWormholesVisible);
	m_eWaypoints = (WAYPOINTS)g_Au.Settings().readNumEntry("/NavigatorStation/WaypointVisibility", m_eWaypoints);
	m_fPan.x = g_Au.Settings().readNumEntry("/NavigatorStation/XPan", m_fPan.x);
	m_fPan.y = g_Au.Settings().readNumEntry("/NavigatorStation/YPan", m_fPan.y);
	m_fPan.z = g_Au.Settings().readNumEntry("/NavigatorStation/ZPan", m_fPan.z);
	m_fRot.x = g_Au.Settings().readNumEntry("/NavigatorStation/XRotation", m_fRot.x);
	m_fRot.y = g_Au.Settings().readNumEntry("/NavigatorStation/YRotation", m_fRot.y);
	m_fRot.z = g_Au.Settings().readNumEntry("/NavigatorStation/ZRotation", m_fRot.z);
	m_Zoom = (ZOOMSETTING)g_Au.Settings().readNumEntry("/NavigatorStation/Zoom", m_Zoom);

	m_idSystem = AuId(g_Au.Settings().readNumEntry("/NavigatorStation/SelectedSystem", 0));
	m_iNamesVisible = g_Au.Settings().readNumEntry("/NavigatorStation/NamesVisible", m_iNamesVisible);
	m_eSelection = (SELECTION)g_Au.Settings().readNumEntry("/NavigatorStation/SelectionType", m_eSelection);
	m_idSelected = AuId(g_Au.Settings().readNumEntry("/NavigatorStation/SelectedId", 0));
	m_lSelectedWaypoint = g_Au.Settings().readNumEntry("/NavigatorStation/SelectedWaypoint", m_lSelectedWaypoint);
	m_SelectedOffset.x = g_Au.Settings().readNumEntry("/NavigatorStation/SelectedOffsetX", m_SelectedOffset.x);
	m_SelectedOffset.y = g_Au.Settings().readNumEntry("/NavigatorStation/SelectedOffsetY", m_SelectedOffset.y);
	m_SelectedOffset.z = g_Au.Settings().readNumEntry("/NavigatorStation/SelectedOffsetZ", m_SelectedOffset.z);
}

void DpStationNav::StoreClientSettings(const AuId& /*planetId*/) const
{
	g_Au.Settings().writeEntry("/NavigatorStation/NeighborSectors", m_bNeighborSectors);
	int rings = ((int)m_bMoonRingsVisible << 1) | (int)m_bRingsVisible;
	g_Au.Settings().writeEntry("/NavigatorStation/RingsVisible", rings);
	g_Au.Settings().writeEntry("/NavigatorStation/TrackShip", m_bTrackShip);
	g_Au.Settings().writeEntry("/NavigatorStation/WormholesVisible", m_bWormholesVisible);
	g_Au.Settings().writeEntry("/NavigatorStation/WaypointVisibility", m_eWaypoints);
	g_Au.Settings().writeEntry("/NavigatorStation/XPan", m_fPan.x);
	g_Au.Settings().writeEntry("/NavigatorStation/YPan", m_fPan.y);
	g_Au.Settings().writeEntry("/NavigatorStation/ZPan", m_fPan.z);
	g_Au.Settings().writeEntry("/NavigatorStation/XRotation", m_fRot.x);
	g_Au.Settings().writeEntry("/NavigatorStation/YRotation", m_fRot.y);
	g_Au.Settings().writeEntry("/NavigatorStation/ZRotation", m_fRot.z);
	g_Au.Settings().writeEntry("/NavigatorStation/Zoom", m_Zoom);

	g_Au.Settings().writeEntry("/NavigatorStation/SelectedSystem", (int)m_idSystem.GetID());
	g_Au.Settings().writeEntry("/NavigatorStation/NamesVisible", m_iNamesVisible);
	g_Au.Settings().writeEntry("/NavigatorStation/SelectionType", m_eSelection);
	g_Au.Settings().writeEntry("/NavigatorStation/SelectedId", (int)m_idSelected.GetID());
	g_Au.Settings().writeEntry("/NavigatorStation/SelectedWaypoint", m_lSelectedWaypoint);
	g_Au.Settings().writeEntry("/NavigatorStation/SelectedOffsetX", m_SelectedOffset.x);
	g_Au.Settings().writeEntry("/NavigatorStation/SelectedOffsetY", m_SelectedOffset.y);
	g_Au.Settings().writeEntry("/NavigatorStation/SelectedOffsetZ", m_SelectedOffset.z);
}

void DpStationNav::TxSettings(QDataStream& out) const
{
	quint8 bits = 0;
	if(m_bNeighborSectors)
		bits |= 0x01;
	if(m_bRingsVisible)
		bits |= 0x02;
	if(m_bTrackShip)
		bits |= 0x04;
	if(m_bWormholesVisible)
		bits |= 0x08;
	if(m_iNamesVisible)
		bits |= (m_iNamesVisible << 4);
	if(m_bMoonRingsVisible)
		bits |= 0x40;
	out << bits;

	out << (quint8)m_eSelection;
	out << (quint8)m_eWaypoints;
	out << (float)m_fPan.x;
	out << (float)m_fPan.y;
	out << (float)m_fPan.z;
	out << (float)m_fRot.x;
	out << (float)m_fRot.y;
	out << (float)m_fRot.z;
	m_idSelected >> out;
	out << m_lSelectedWaypoint;
	out << (quint8)m_Zoom;
	out << (float)m_SelectedOffset.x;
	out << (float)m_SelectedOffset.y;
	out << (float)m_SelectedOffset.z;
}

void DpStationNav::RxSettings(QDataStream& in)
{
	quint8 bits;
	in >> bits;
	m_bNeighborSectors = (bits & 0x01) ? true : false;
	m_bRingsVisible = (bits & 0x02) ? true : false;
	m_bTrackShip = (bits & 0x04) ? true : false;
	m_bWormholesVisible = (bits & 0x08) ? true : false;
	m_iNamesVisible = (bits & 0x30) >> 4;
	m_bMoonRingsVisible = (bits & 0x40) ? true : false;

	quint8 val8;
	in >> val8;
	m_eSelection = (SELECTION)val8;
	in >> val8;
	m_eWaypoints = (WAYPOINTS)val8;
	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fPan = AuPoint(x,y,z);
	in >> x;
	in >> y;
	in >> z;
	m_fRot = AuPoint(x,y,z);
	m_idSelected << in;
	in >> m_lSelectedWaypoint;
	in >> val8;
	m_Zoom = (ZOOMSETTING)val8;
	in >> x;
	in >> y;
	in >> z;
	m_SelectedOffset = AuPoint(x,y,z);
}

DpStation::STATION_TYPE DpStationNav::ClientRestoreType() const
{
	return STATION_Nav;
}

void DpStationNav::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationNav::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("NCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txConsoleNav.bmp");
}

void DpStationNav::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationNav::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_bMoonRingsVisible);
	mem += sizeof(m_bNeighborSectors);
	mem += sizeof(m_bRingsVisible);
	mem += sizeof(m_bTrackShip);
	mem += sizeof(m_bWormholesVisible);
	mem += sizeof(m_eCurrentCommand);
	mem += sizeof(m_eWaypoints);
	mem += sizeof(m_fPan);
	mem += sizeof(m_fRot);
	mem += sizeof(m_idSystem);
	mem += sizeof(m_iNamesVisible);
	mem += sizeof(m_Zoom);
	mem += sizeof(m_eSelection);
	mem += sizeof(m_idSelected);
	mem += sizeof(m_lSelectedWaypoint);
	mem += sizeof(m_SelectedOffset);
	mem += sizeof(m_WaypointLock);
	mem += sizeof(DeWaypoint) * m_Waypoints.capacity();
	mem += sizeof(m_bLastNeighborSectors);
	mem += sizeof(m_bLastRings);
	mem += sizeof(m_bLastWormholes);
	mem += sizeof(m_DisplayChanged);
	mem += sizeof(m_eLastCommand);
	mem += sizeof(m_eLastWaypoints);
	mem += sizeof(m_eLastSel);
	mem += sizeof(m_iLastNames);
	mem += sizeof(m_LastZoom);
	mem += sizeof(m_lLastWaypointCount);
	mem += sizeof(m_ScreenTex);
	return mem;
}
