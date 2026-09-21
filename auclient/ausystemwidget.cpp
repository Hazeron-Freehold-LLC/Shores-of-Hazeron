//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemwidget.cpp,v 1.34 2022/04/13 23:25:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemwidget.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>

#define d_CursorMargin					10						// Pixels added around selected body for visible cursor and select.
#define d_MinZoom						100

AuSystemWidget::AuSystemWidget(QWidget* parent) :
	QWidget(parent), m_Cursor(QPixmap(":data/CrossHairCursor.png"), 32,32)
{
	m_LeftButtonMode = 0;
	m_MinQToShow = 200;
	m_RightButtonMode = 0;
	m_ScanEx = 0;
	m_System = 0;
	m_timer = 0;
	m_ViewerBlip = AuSensorBlip::Invalid;
	m_ViewerStance = POL_Neutral;
	m_Zoom = 0;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setCursor(m_Cursor);
	//setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
}

AuSystemWidget::~AuSystemWidget()
{
	ClearAsteroids();
}

void AuSystemWidget::ClearAsteroids()
{
	for(QHash<DaAsteroid*, AuPoint>::iterator ai = m_Asteroids.begin(); ai != m_Asteroids.end(); ai++)
		delete ai.key();
	m_Asteroids.clear();
}

void AuSystemWidget::SetSystem(DaSolarSystem* system, int scanEx)
{
	m_ScanEx = scanEx;
	m_System = system;
	m_Selected = AuId::NullId();
	update();
}

void AuSystemWidget::SetViewerLoc(AuSensorBlip::TYPE viewerBlip, POLITICAL_STANCE viewerStance, const AuPoint& viewerLoc,
		const DaAtomList& asteroids)
{
	m_ViewerBlip = viewerBlip;
	m_ViewerLoc = viewerLoc;
	m_ViewerStance = viewerStance;

	ClearAsteroids();
	for(DaAtomList::const_iterator ai = asteroids.cbegin(); ai != asteroids.cend(); ai++)
	{
		DaAsteroid* roid = dynamic_cast<DaAsteroid*>(*ai);
		if(!roid)
			continue;
		m_Asteroids.insert((DaAsteroid*)(roid->Clone()), roid->EnvLocation());
	}
	update();
}

void AuSystemWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(Update()));
	}
	if(!m_timer->isActive())
		m_timer->start(250);
}

void AuSystemWidget::Update()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	if(m_System)
	{
		DaAtomList atoms;
		m_System->ListSubatomsRecursive(atoms);
		for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
			(*ai)->TickOnClient(m_LastTick);
		m_LastTick.restart();
	}

	update();
}

void AuSystemWidget::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);

	m_LeftButtonMode = 0;
	m_RightButtonMode = 0;
	m_MouseDownPt = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		setCursor(QCursor(Qt::ClosedHandCursor));
		m_LeftButtonMode = 1;								// pan
	}
	else if(e->button() == Qt::RightButton)
	{
		setCursor(QCursor(QPixmap(":data/SpinCursor.png")));
		m_RightButtonMode = 1;								// rotate
	}
}

void AuSystemWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);
}

// NOTE: We have enabled mouse tracking so this gets called whenever the mouse moves, even if no button is depressed.
void AuSystemWidget::mouseMoveEvent(QMouseEvent* e)
{
	QWidget::mouseMoveEvent(e);

	if(m_LeftButtonMode == 0)
	{
	}
	else if(m_LeftButtonMode == 1)
	{
		// Test for move far enough to start drag.
		QPoint vec = e->pos() - m_MouseDownPt;
		if(vec.manhattanLength() > 8)
			m_LeftButtonMode = 2;	// Dragging
	}
	else
	{
		// Pan view around look point.
		int dx = e->pos().x() - m_MouseDownPt.x();
		int dy = e->pos().y() - m_MouseDownPt.y();
		m_MouseDownPt = e->pos();

		// m_fPan is in feet.
		AuPoint panVec(dx, dy);
		AuMatrix4x4 mvp;
		ModelViewProjection(mvp);
		panVec = mvp.UnProjectVector(panVec);
		m_fPan -= panVec;

		const int iMaxStarOrbits = 16;
		const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
		if(m_fPan.x > iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.x = iMaxSystemOrbits*FeetPerOrbit();
		else if(m_fPan.x < -iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.x = -iMaxSystemOrbits*FeetPerOrbit();
		if(m_fPan.y > iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.y = iMaxSystemOrbits*FeetPerOrbit();
		else if(m_fPan.y < -iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.y = -iMaxSystemOrbits*FeetPerOrbit();

		update();
	}

	if(m_RightButtonMode == 0)
	{
	}
	else if(m_RightButtonMode == 1)
	{
		// Test for move far enough to start drag.
		QPoint vec = e->pos() - m_MouseDownPt;
		if(vec.manhattanLength() > 8)
			m_RightButtonMode = 2;	// Rotating
	}
	else
	{
		// Rotate view around look point.
		int dx = e->pos().x() - m_MouseDownPt.x();
		int dy = m_MouseDownPt.y() - e->pos().y();
		m_MouseDownPt = e->pos();

		m_fRot.x += dy * DEG2RAD;
		while(m_fRot.x < -PI)
			m_fRot.x += PI*2;
		while(m_fRot.x > PI)
			m_fRot.x -= PI*2;

		m_fRot.z += dx * DEG2RAD;
		while(m_fRot.z < -PI)
			m_fRot.z += PI*2;
		while(m_fRot.z > PI)
			m_fRot.z -= PI*2;

		update();
	}
}

void AuSystemWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(m_Cursor);
	QWidget::mouseReleaseEvent(e);

	if(m_LeftButtonMode == 1)
	{
		emit BodyClicked(BodyAt(e->pos()));
	}
	else if(m_RightButtonMode == 1 && m_System && !auApp->IsSingleUserDesignerMode())
	{
		// Build a right-click context menu.
		AuId bodyId(BodyAt(e->pos()));

		QAction* attack = 0;
		QAction* follow = 0;
		QAction* goToCoord = 0;
		QAction* goToCoordSys = 0;
		QAction* goToLand = 0;
		QAction* goToOrbit = 0;
		AuPoint targetLoc;
		QString targetName;
		QMenu menu(this);

		bool isRoid = false;
		AuPoint roidLoc;
		QString roidName;
		for(QHash<DaAsteroid*, AuPoint>::const_iterator ai = m_Asteroids.cbegin(); ai != m_Asteroids.cend(); ai++)
		{
			if(ai.key()->Is(bodyId))
			{
				isRoid = true;
				roidLoc = *ai;
				roidName = ai.key()->Name();
				break;
			}
		}

		if(isRoid)
		{
			AuMissionOrderAttack orderA(bodyId, roidName);
			attack = menu.addAction(QIcon(orderA.ListNameIcon()), orderA.ListName());

			AuMissionOrderFollow orderF(bodyId, roidName);
			follow = menu.addAction(QIcon(orderF.ListNameIcon()), orderF.ListName());

			targetLoc = roidLoc;
			targetLoc /= FeetPerAu();	// Order wants the number in au.
			AuMissionOrderGoToCoordinate orderG(AuMissionOrder::SolarSystem, targetLoc);
			goToCoord = menu.addAction(QIcon(orderG.ListNameIcon()), orderG.ListName());
		}
		else
		{
			const DaAtom* atom = m_System->FindIdRecursive(bodyId);
			if(atom)
			{
				targetLoc = atom->EnvLocation();
				targetLoc /= FeetPerAu();	// Order wants the number in au.
				AuMissionOrderGoToCoordinate order(AuMissionOrder::SolarSystem, targetLoc);
				goToCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());

				const DaWorld* world = dynamic_cast<const DaWorld*>(atom);
				const DaStar* star = dynamic_cast<const DaStar*>(atom);
				if(world)
				{
					targetName = world->DefaultName(m_System->CatalogName());
					AuMissionOrderGoToOrbit orderO(AuId(1), targetName);
					goToOrbit = menu.addAction(QIcon(orderO.ListNameIcon()), orderO.ListName());

					AuMissionOrderGoToLand orderL(AuId(1), targetName);
					goToLand = menu.addAction(QIcon(orderL.ListNameIcon()), orderL.ListName());
				}
				else if(star)
				{
					targetName = star->Name();
					AuMissionOrderGoToOrbit order(AuId(1), targetName);
					goToOrbit = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
			}
		}

		// Calculate the solar system point under the cursor.
		AuMatrix4x4 mvp;
		ModelViewProjection(mvp);
		AuPoint systemLoc = mvp.UnProject(AuPoint(e->pos()));	// in feet
		systemLoc /= FeetPerAu();	// Order wants the number in au.
		AuMissionOrderGoToCoordinate order(AuMissionOrder::SolarSystem, systemLoc);
		goToCoordSys = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());

		QAction* action = menu.exec(QCursor::pos());
		if(action)
		{
			if(action == attack)
				emit Attack(bodyId, roidName);
			else if(action == follow)
				emit Follow(bodyId, roidName);
			else if(action == goToCoord)
				emit GoToCoord(targetLoc);
			else if(action == goToLand)
				emit GoLand(AuId(1), targetName);
			else if(action == goToOrbit)
				emit GoOrbit(AuId(1), targetName);
			else if(action == goToCoordSys)
				emit GoToCoord(systemLoc);
		}
	}
	m_LeftButtonMode = 0;
	m_RightButtonMode = 0;
}

void AuSystemWidget::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);

	// Calculate the solar system point under the cursor.
	AuMatrix4x4 mvp;
	ModelViewProjection(mvp);
#if(QT_VERSION >= QT_VERSION_CHECK(5,12,9))
	AuPoint sloc = mvp.UnProject(AuPoint(e->position()));	// in feet
#else
	AuPoint sloc = mvp.UnProject(AuPoint(e->pos()));	// in feet
#endif
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	int zDelta = e->angleDelta().y();
#else
	int zDelta = e->delta();
#endif

	float amt = zDelta;
	m_Zoom += amt;
	if(m_Zoom < d_MinZoom)
		m_Zoom = d_MinZoom;

	// Pan the view so the solar system point that was under the cursor is still under the cursor.
	{
		// Calculate new pixel location of system loc, to see where it moved to.
		ModelViewProjection(mvp);
		AuPoint pxloc = mvp * sloc;										// nu pixel location of sloc
#if(QT_VERSION >= QT_VERSION_CHECK(5,12,9))
		AuPoint panVec(e->position().x() - pxloc.x, e->position().y() - pxloc.y, -pxloc.z);	// Pan vector required to keep sloc in place.
#else
		AuPoint panVec(e->pos().x() - pxloc.x, e->pos().y() - pxloc.y, -pxloc.z);	// Pan vector required to keep sloc in place.
#endif
		panVec = mvp.UnProjectVector(panVec);
		m_fPan -= panVec;

		const int iMaxStarOrbits = 16;
		const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
		if(m_fPan.x > iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.x = iMaxSystemOrbits*FeetPerOrbit();
		else if(m_fPan.x < -iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.x = -iMaxSystemOrbits*FeetPerOrbit();
		if(m_fPan.y > iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.y = iMaxSystemOrbits*FeetPerOrbit();
		else if(m_fPan.y < -iMaxSystemOrbits*FeetPerOrbit())
			m_fPan.y = -iMaxSystemOrbits*FeetPerOrbit();
	}

	//qDebug() << m_Zoom;
	update();
}

void AuSystemWidget::keyPressEvent(QKeyEvent *e)
{
	switch(e->key())
	{
	case Qt::Key_W:
		break;
	}
	QWidget::keyPressEvent(e);
}

void AuSystemWidget::paintEvent(QPaintEvent* pe)
{
	// Initialize zoom.
	if(m_Zoom == 0)
		m_Zoom = width()/2;

	QPainter p(this);
	p.fillRect(rect(), Qt::black);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);
	DrawSolarSystemDisplay(p);
	p.end();
	
	QWidget::paintEvent(pe);
}

void AuSystemWidget::DrawSolarSystemDisplay(QPainter& p)
{
	if(!m_System)
		return;
	DaStar* pstar = m_System->FindStar();
	if(!pstar)
		return;

	AuMatrix4x4 mvp;
	double pixelsPerOrbit = ModelViewProjection(mvp);

	const DaAtom* selected = 0;
	QRect rcSel;

	// Draw primary and subordinate stars and their planets.
	m_Bodies.clear();				// Filled during the call.
	m_Rings.clear();				// Filled during the call.
	bool bOrbit[6] = {0,0,0,0,0,0};	// Orbit zones set this true as they're used
	DrawStarAndPlanets(p, mvp, pstar, pixelsPerOrbit, bOrbit, selected, rcSel);
	DaAtomList stars;
	pstar->ListStars(stars);
	while(!stars.isEmpty())
	{
		pstar = (DaStar*)stars.takeFirst();
		DrawStarAndPlanets(p, mvp, pstar, pixelsPerOrbit, bOrbit, selected, rcSel);
	}

	// Show viewer location and asteroids.
	if(m_ViewerBlip != AuSensorBlip::Invalid)
	{
		// Asteroids are only shown in the system that contains the viewer.
		DrawAsteroids(p, mvp, pixelsPerOrbit, selected, rcSel);
		if(m_ViewerBlip != AuSensorBlip::Invalid && AuTime::CurrentSecsSinceEpoch() & 1)
		{
			// Draw an icon to represent the viewer.
			AuPoint pxloc = mvp * m_ViewerLoc;
			int iradloc = 8;
			QRect rcloc(pxloc.x-iradloc, pxloc.y-iradloc, iradloc*2+1, iradloc*2+1);
			AuSensorBlip* blip = AuSensorBlip::Create(m_ViewerBlip);
			if(blip)
			{
				blip->SetPoliticalStance(m_ViewerStance);
				QImage imgLoc(blip->BlipImage());
				p.drawImage(rcloc, imgLoc);
				delete blip;
			}
		}
	}

	// Show selected last.
	if(selected)
	{
		QMultiMap<quint8, COMMODITYID> atmRes, hydRes, geoRes, bioRes;
		if(m_ScanEx == 2)
			selected->SystemScanResources(atmRes, hydRes, geoRes, bioRes);
		ShowSelected(p, selected->Name(), rcSel, atmRes, hydRes, geoRes, bioRes);
	}
}

void AuSystemWidget::DrawStarAndPlanets(QPainter& p, const AuMatrix4x4& mvp, DaStar *pstar, double pixelsPerOrbit, bool bOrbit[6],
		const DaAtom*& selected, QRect& rcSel)
{
	// Draw the orbit ring of the star if it's not the primary.
	if(!pstar->IsPrimary())
	{
		double radorbit = (pstar->Orbit() + 1.0);	// Orbit radius in orbits.
		p.setPen(QPen(QColor(96,96,192), 0.f));

		AuPointArray circle;
		circle.MakeCircle(AuPoint(0,0), AuPoint(radorbit * FeetPerOrbit(), 0), 60);
		circle.Transform(pstar->Parent()->EnvTransform());
		circle.append(circle.first());

		QVector<QPointF> cline;
		AuPointArray::const_iterator ci = circle.cbegin();
		AuPoint prev = mvp * *ci;
		for(ci++; ci != circle.cend(); ci++)
		{
			AuPoint cur = mvp * *ci;
			cline.append(QPointF(prev.x, prev.y));
			cline.append(QPointF(cur.x, cur.y));
			prev = cur;
		}
		p.drawLines(cline);

		bOrbit[5] = true;
	}

	// Draw a filled circle to represent the star.
	AuPoint pxstar = mvp * pstar->EnvLocation();
	int iradstar = (int)(pstar->Radius()/FeetPerOrbit() * pixelsPerOrbit);	// Pixel radius of star.
	if(iradstar < 1)
		iradstar = 1;
	QRect rcStar(pxstar.x-iradstar, pxstar.y-iradstar, iradstar*2+1, iradstar*2+1);
	//p.setBrush(pstar->ChartColor());
	//p.setPen(QPen(pstar->ChartColor(), 0.f));
	//p.drawEllipse(rcStar);
	QImage imgstar;
	switch(pstar->StarType())
	{
	case DaStar::TYPE_O:
		imgstar.load(":data/c_TypeOPreons.png");
		break;
	case DaStar::TYPE_B:
		imgstar.load(":data/c_TypeBPreons.png");
		break;
	case DaStar::TYPE_A:
		imgstar.load(":data/c_TypeAPreons.png");
		break;
	case DaStar::TYPE_F:
		imgstar.load(":data/c_TypeFPreons.png");
		break;
	case DaStar::TYPE_G:
		imgstar.load(":data/c_TypeGPreons.png");
		break;
	case DaStar::TYPE_K:
		imgstar.load(":data/c_TypeKPreons.png");
		break;
	case DaStar::TYPE_M:
		imgstar.load(":data/c_TypeMPreons.png");
		break;
	case DaStar::TYPE_StellarBlackHole:
	case DaStar::TYPE_SupermassiveBlackHole:
		imgstar.load(":data/StarBlackHole.png");
		break;
	case DaStar::TYPE_NeutronStar:
		imgstar.load(":data/StarNeutron.png");
		break;
	}
	p.drawImage(rcStar, imgstar);

	// Label the star.
	p.drawText(rcStar.right()+d_CursorMargin+2, rcStar.bottom()+d_CursorMargin-2, pstar->Name());

	// Show selected item indicator.
	if(pstar->Is(m_Selected))
	{
		selected = pstar;
		rcSel = rcStar;
	}

	m_Bodies.insert(pstar->Id(), rcStar);

	if(!m_ScanEx)
		return;

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
		QPen ringpen(penorbit[pb->OrbitZone()], 0.f);
		p.setPen(ringpen);
		DrawCircle(p, mvp, AuPoint(0,0), pb->OrbitRadius(), pb->Parent()->EnvTransform(), 60);
		bOrbit[pb->OrbitZone()] = true;
	}

	// Draw planets.
	for(DaAtomList::iterator it = planets.begin(); it != planets.end(); it++)
	{
		const DaWorld *pb = (const DaWorld*)(*it);
		QColor colPlanet = pb->AtmosphereColor();			// This is the color the planet appears from a distance.
		p.setBrush(colPlanet);
		p.setPen(QPen(colPlanet, 0.f));

		const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(pb);
		if(rsworld)
		{
			if(!rsworld->SectionIndex())
			{
				// Ringworld
				// Draw the ring as a double line, so it looks like a ringworld.
				double radY = rsworld->TerrainYSize()/2;
				DrawCircle(p, mvp, AuPoint(0,0,radY), pb->OrbitRadius(), pb->Parent()->EnvTransform(), 60);
				DrawCircle(p, mvp, AuPoint(0,0,-radY), pb->OrbitRadius(), pb->Parent()->EnvTransform(), 60);
			}

			AuPoint ploc = mvp * pb->EnvLocation();
			QPoint pxc(ploc.x, ploc.y);

			// Label the ring arc.
			if(m_Zoom > 500 || pb->Is(m_Selected))
				p.drawText(pxc.x()+d_CursorMargin+2, pxc.y()+d_CursorMargin-2, pb->Name());

			QRect rcRing = QRect(pxc, QSize(1,1)).adjusted(-10,-10,10,10);	// Just the selection area.

			// Show selected item indicator.
			if(pb->Is(m_Selected))
			{
				selected = pb;
				rcSel = rcRing;
			}

			m_Rings.insert(pb->Id(), rcRing);
		}
		else
		{
			// Draw the planet.
			AuPoint ploc = mvp * pb->EnvLocation();
			QPoint pxc(ploc.x, ploc.y);

			// Spherical planet radius in orbits.
			double radplanet = pb->SeaLevelAltitude()/FeetPerOrbit();			// Planet radius in orbits.
			int iradplanet = (int)(radplanet * pixelsPerOrbit);				// Planet radius in pixels.
			if (iradplanet < 1)
				iradplanet = 1;
			QRect rcPlanet(pxc.x()-iradplanet, pxc.y()-iradplanet, iradplanet*2+1, iradplanet*2+1);
			p.drawEllipse(rcPlanet);

			if(pb->IsAsteroidBeltPrimary())
			{
				QColor colRing = QColor(176,140,31);
				QColor colRingDim = QColor(176/2,140/2,31/2);
				QPen ringpen(colRing, 0.f);
				ringpen.setStyle(Qt::DotLine);
				QPen ringpenCen(colPlanet, 0.f);
				ringpenCen.setStyle(Qt::DashLine);
				QPen ringpenDim(colRingDim, 0.f);
				ringpenDim.setStyle(Qt::DotLine);
				p.setPen(ringpenDim);			// For first ring.
				for(int r=-5; r<=5; r++)
				{
					double rad = pb->OrbitRadius() + r * FeetPerMoonOrbit();
					DrawCircle(p, mvp, AuPoint(0,0), rad, pb->Parent()->EnvTransform(), 60);

					if(r == -4 || r == 0)
						p.setPen(ringpen);		// For inner rings.
					else if(r == -1)
						p.setPen(ringpenCen);	// For center rings.
					else if(r == 3)
						p.setPen(ringpenDim);	// For last ring.
				}
				p.setPen(QPen(colPlanet, 0.f));	// Restore
			}

			// Label the planet.
			if(m_Zoom > 500 || pb->Is(m_Selected))
				p.drawText(rcPlanet.right()+d_CursorMargin+2, rcPlanet.bottom()+d_CursorMargin-2, pb->Name());

			// Show selected item indicator.
			if(pb->Is(m_Selected))
			{
				selected = pb;
				rcSel = rcPlanet;
			}

			m_Bodies.insert(pb->Id(), rcPlanet);

			DaAtomList moons;
			pb->ListPlanets(moons);
			while(!moons.isEmpty())
			{
				DaWorld* satellite = dynamic_cast<DaWorld*>(moons.takeFirst());
				DaWorldGlobe* glmoon = dynamic_cast<DaWorldGlobe*>(satellite);
				if(glmoon)
				{
					// Draw a moon.
					QColor colMoon = satellite->AtmosphereColor();			// This is the color the planet appears from a distance.
					p.setBrush(colMoon);
					p.setPen(QPen(colMoon, 0.f));

					AuPoint mloc = mvp * satellite->EnvLocation();
					QPoint pxm(mloc.x, mloc.y);	// Center of planet in bitmap.

					// Spherical planet radius in orbits.
					double radplanet = glmoon->SeaLevelAltitude()/FeetPerOrbit();			// Planet radius in orbits.
					int iradplanet = (int)(radplanet * pixelsPerOrbit);				// Planet radius in pixels.
					if (iradplanet < 1)
						iradplanet = 1;
					QRect rcMoon(pxm.x()-iradplanet, pxm.y()-iradplanet, iradplanet*2+1, iradplanet*2+1);
					// Moons look bad at low scale.
					if(m_Zoom > 500)
					{
						p.drawEllipse(rcMoon);

						// Big enough to see is big enough to select.
						m_Bodies.insert(satellite->Id(), rcMoon);
					}

					// Lunar orbit ring.
					if(m_Zoom > 5000 || satellite->Is(m_Selected))
					{
						double ringRad = glmoon->Location().Length();
						DrawCircle(p, mvp, AuPoint(0,0), ringRad, glmoon->Parent()->EnvTransform(), 40);
					}

					// Label the moon.
					if(m_Zoom > 2000 || satellite->Is(m_Selected))
						p.drawText(rcMoon.right()+d_CursorMargin+2, rcMoon.bottom()+d_CursorMargin-2, satellite->Name());

					// Show selected item indicator.
					if(satellite->Is(m_Selected))
					{
						selected = satellite;
						rcSel = rcMoon;
					}
				}
				else
				{
					DaWorldRing* ring = dynamic_cast<DaWorldRing*>(satellite);
					if(ring)
					{
						// Draw a ring.
						QColor colRing = QColor(176,140,31);
						QPen ringpen(colRing, 0.f);
						ringpen.setStyle(Qt::DashLine);
						p.setBrush(colRing);
						p.setPen(ringpen);

						AuPoint mloc = mvp * satellite->Parent()->EnvLocation();
						QPoint pxm(mloc.x, mloc.y);	// Center of ring in bitmap.

						double ringRad = satellite->Location().Length();
						int iradorbit = (int)(ringRad / FeetPerOrbit() * pixelsPerOrbit);		// Orbit radius in pixels.
						QRect rcRing(pxm.x()-iradorbit, pxm.y()-iradorbit, iradorbit*2+1, iradorbit*2+1);

						DrawCircle(p, mvp, AuPoint(0,0), ringRad, satellite->Parent()->EnvTransform(), 40);

						// Big enough to see is big enough to select.
						m_Rings.insert(satellite->Id(), rcRing);

						if(m_Zoom > 500)
						{
							// Draw inner and outer rings.
							p.setBrush(colRing);
							ringpen.setStyle(Qt::DotLine);
							p.setPen(ringpen);
							double inner, outer;
							ring->Radii(inner, outer);
							DrawCircle(p, mvp, AuPoint(0,0), ringRad - inner, satellite->Parent()->EnvTransform(), 40);
							DrawCircle(p, mvp, AuPoint(0,0), ringRad + inner, satellite->Parent()->EnvTransform(), 40);
						}

						// Label the ring.
						if(m_Zoom > 2000 || satellite->Is(m_Selected))
							p.drawText(rcRing.right()+d_CursorMargin+2, rcRing.bottom()+d_CursorMargin-2, satellite->Name());

						// Show selected item indicator.
						if(satellite->Is(m_Selected))
						{
							selected = satellite;
							rcSel = rcRing;
						}
					}
				}
			}
		}
	}
}

void AuSystemWidget::DrawAsteroids(QPainter& p, const AuMatrix4x4& mvp, double pixelsPerOrbit, const DaAtom*& selected,
		QRect& rcSel)
{
	if(m_Asteroids.isEmpty())
		return;
	QImage imgroidblip(":data/BlipAsteroid.png");
	QImage imgroid(":data/Asteroid.png");
	p.setPen(QPen(QColor(255,128,64), 0.f));
	for(QHash<DaAsteroid*, AuPoint>::const_iterator ai = m_Asteroids.cbegin(); ai != m_Asteroids.cend(); ai++)
	{
		DaAsteroid* roid = ai.key();

		// Draw a filled circle to represent the star.
		AuPoint pxroid = mvp * *ai;
		int iradroid = (int)(roid->ObstacleRadius()/FeetPerOrbit() * pixelsPerOrbit);	// Pixel radius of star.
		if(iradroid < 1)
			iradroid = 1;
		int bliprad = iradroid * 3 / 2 + 6;

		QRect rcBlip(pxroid.x-bliprad, pxroid.y-bliprad, bliprad*2+1, bliprad*2+1);
		p.drawImage(rcBlip, imgroidblip);

		QRect rcAster(pxroid.x-iradroid, pxroid.y-iradroid, iradroid*2+1, iradroid*2+1);
		p.drawImage(rcAster, imgroid);

		// Big enough to see is big enough to select.
		m_Bodies.insert(roid->Id(), rcBlip);

		// Label the roid.
		if(m_Zoom > 1000 || roid->Is(m_Selected))
			p.drawText(rcBlip.right()+d_CursorMargin+2, rcBlip.bottom()+d_CursorMargin-2, roid->Name());

		// Show selected item indicator.
		if(roid->Is(m_Selected))
		{
			selected = roid;
			rcSel = rcBlip;
		}
	}
}

void AuSystemWidget::SetMinQToShow(int minQ)
{
	m_MinQToShow = (quint8)minQ;
	update();
}

void AuSystemWidget::ShowSelected(QPainter& p, const QString& itemName, const QRect& itemRect,
		const QMultiMap<quint8, COMMODITYID>& atmRes, const QMultiMap<quint8, COMMODITYID>& hydRes,
		const QMultiMap<quint8, COMMODITYID>& geoRes, const QMultiMap<quint8, COMMODITYID>& bioRes)
{
	// We will show the best from each category.
	QMultiMap<QString, COMMODITYID> showMe;
	const int showEach = 100;			// Max number of commodities to show in each category.
	int shown=0;
	for(QMultiMap<quint8, COMMODITYID>::const_iterator ri = geoRes.cend()-1; ri != geoRes.cbegin()-1 && shown < showEach; ri--)
	{
		if(ri.key() < m_MinQToShow)
			break;
		shown++;
		showMe.insert(QString("%1 Q%2").arg(g_Au.CommodityName(*ri)).arg(ri.key()), *ri);
	}
	shown=0;
	for(QMultiMap<quint8, COMMODITYID>::const_iterator ri = hydRes.cend()-1; ri != hydRes.cbegin()-1 && shown < showEach; ri--)
	{
		if(ri.key() < m_MinQToShow)
			break;
		shown++;
		showMe.insert(QString("%1 Q%2").arg(g_Au.CommodityName(*ri)).arg(ri.key()), *ri);
	}
	shown=0;
	for(QMultiMap<quint8, COMMODITYID>::const_iterator ri = atmRes.cend()-1; ri != atmRes.cbegin()-1 && shown < showEach; ri--)
	{
		if(ri.key() < m_MinQToShow)
			break;
		shown++;
		showMe.insert(QString("%1 Q%2").arg(g_Au.CommodityName(*ri)).arg(ri.key()), *ri);
	}
	shown=0;
	for(QMultiMap<quint8, COMMODITYID>::const_iterator ri = bioRes.cend()-1; ri != bioRes.cbegin()-1 && shown < showEach; ri--)
	{
		if(ri.key() < m_MinQToShow)
			break;
		shown++;
		showMe.insert(QString("%1 Q%2").arg(g_Au.CommodityName(*ri)).arg(ri.key()), *ri);
	}

	// Calculate sizes and positions.
	QRect rcsel(itemRect.adjusted(-d_CursorMargin,-d_CursorMargin,d_CursorMargin,d_CursorMargin));
	const int cidwidth = 250;
	const int cidleft = (itemRect.right() + rcsel.right())/2;
	const int rowht = 20;
	const int cidheight = showMe.size() * rowht;
	QRect cidbg(cidleft, rcsel.bottom() - rowht, cidwidth, cidheight + rowht + 8);

	// Fill background of text.
	p.fillRect(cidbg, QBrush(QColor(0,32,32,224)));

	// Draw reticle around body.
	if(rcsel.width() < 32)
	{
		QImage imgsel(":data/ScanSelect16.png");
		p.drawImage(rcsel, imgsel);
	}
	else if(rcsel.width() < 64)
	{
		QImage imgsel(":data/ScanSelect32.png");
		p.drawImage(rcsel, imgsel);
	}
	else if(rcsel.width() < 128)
	{
		QImage imgsel(":data/ScanSelect64.png");
		p.drawImage(rcsel, imgsel);
	}
	else if(rcsel.width() < 256)
	{
		QImage imgsel(":data/ScanSelect128.png");
		p.drawImage(rcsel, imgsel);
	}
	else
	{
		QImage imgsel(":data/ScanSelect256.png");
		p.drawImage(rcsel, imgsel);
	}

	// Draw lines.
	p.setPen(QPen(QColor(0,255,255,128), 1.5f));
	p.drawLine(QPoint(cidleft-2, rcsel.bottom()+2), QPoint(cidleft + cidwidth, rcsel.bottom()+2));	// Horizontal line
	p.drawLine(QPoint(cidleft+cidwidth-3, rcsel.bottom()+2), QPoint(cidleft + cidwidth + 1, rcsel.bottom()+2));	// Highlight the tip
	p.drawLine(QPoint(cidleft, itemRect.center().y()), QPoint(cidleft, cidbg.bottom() + 2));		// Vertical line
	p.drawLine(QPoint(cidleft, itemRect.center().y()-1), QPoint(cidleft, itemRect.center().y() + 2));	// Highlight the tip
	p.drawLine(QPoint(cidleft, cidbg.bottom() - 3), QPoint(cidleft, cidbg.bottom() + 1));		// Highlight the other tip

	// Display the name of the selected body.
	p.setPen(QColor(128,255,255));
	p.drawText(rcsel.right()+2, rcsel.bottom()-2, itemName);

	// Display icons and names of top 5 commodities.
	p.setPen(QColor(0,192,192));
	int xpos = cidleft + 4;
	int ypos = rcsel.bottom();
	for(QMultiMap<QString, COMMODITYID>::const_iterator si = showMe.cbegin(); si != showMe.cend(); si++)
	{
		QImage img = QImage(g_Au.CommodityBitmapName(*si)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		p.drawImage(xpos, ypos+8, img);
		ypos += rowht;
		p.drawText(xpos + 20, ypos, si.key());
	}
}

AuId AuSystemWidget::BodyAt(const QPoint& px) const
{
	AuId closestId;
	float closestDist2 = 0;
	for(QHash<AuId, QRect>::const_iterator bi = m_Bodies.cbegin(); bi != m_Bodies.cend(); bi++)
		if((*bi).adjusted(-d_CursorMargin,-d_CursorMargin,d_CursorMargin,d_CursorMargin).contains(px))
		{
			QPoint vec = px - (*bi).center();
			float dist2 = vec.x() * vec.x() + vec.y() * vec.y();	// squared distance from the pixel to the center of the rect
			if(closestId.IsNull() || dist2 < closestDist2)
			{
				closestId = bi.key();
				closestDist2 = dist2;
			}
		}
	if(closestId.IsNull())
	{
		// Did not find a body. Look in the rings list. They tend to overlap the bodies.
		for(QHash<AuId, QRect>::const_iterator bi = m_Rings.cbegin(); bi != m_Rings.cend(); bi++)
			if((*bi).adjusted(-d_CursorMargin,-d_CursorMargin,d_CursorMargin,d_CursorMargin).contains(px))
			{
				QPoint vec = px - (*bi).center();
				float dist2 = vec.x() * vec.x() + vec.y() * vec.y();	// squared distance from the pixel to the center of the rect
				if(closestId.IsNull() || dist2 < closestDist2)
				{
					closestId = bi.key();
					closestDist2 = dist2;
				}
			}
	}
	return closestId;
}

double AuSystemWidget::ModelViewProjection(AuMatrix4x4& mvp) const
{
	// Create orthographic model view projection.
	QSize sz(size());
	AuMatrix4x4 p;
	p.Translate(sz.width()/2, sz.height()/2, 0);
	p.Scale(1,-1,1);

	// Calculate some values for converting the real world into this pixel rectangle.
	// In case orbit 15 is a star with its max of 7 orbits (1/2 the star's orbit number (0-15)).
	const int iMaxStarOrbits = 16;
	const int iMaxSystemOrbits = iMaxStarOrbits + (iMaxStarOrbits - 1)/2;
	const double pixelsPerOrbit = m_Zoom/(double)iMaxSystemOrbits;
	const double pixelsPerFoot = pixelsPerOrbit / FeetPerOrbit();

	AuMatrix4x4 mv;
	mv.Scale(pixelsPerFoot, pixelsPerFoot, pixelsPerFoot);
	mv.Rotate(m_fRot);
	mv.Translate(-m_fPan);

	mvp = p * mv;
	return pixelsPerOrbit;
}

void AuSystemWidget::ViewReset()
{
	m_fPan = AuPoint(0,0);
	m_fRot = AuPoint(0,0,0);
	m_Zoom = 0;
	update();
}


void AuSystemWidget::DrawCircle(QPainter& p, const AuMatrix4x4& mvp, const AuPoint& center, double radius, const AuMatrix4x4& model,
		int sides)
{
	AuPointArray circle;
	circle.MakeCircle(center, AuPoint(radius,0), sides);
	circle.Transform(model);
	circle.append(circle.first());

	QVector<QPointF> cline;
	AuPointArray::const_iterator ci = circle.cbegin();
	AuPoint prev = mvp * *ci;
	for(ci++; ci != circle.cend(); ci++)
	{
		AuPoint cur = mvp * *ci;
		cline.append(QPointF(prev.x, prev.y));
		cline.append(QPointF(cur.x, cur.y));
		prev = cur;
	}
	p.drawLines(cline);
}
