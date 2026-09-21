//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapwidget.cpp,v 1.42 2022/04/11 20:26:41 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapwidget.h"
//
#include "auapp.h"
#include "auatomrenderinfo.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auglmodelmaker.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include <QComboBox>
#include <QMenu>
#include <QPainter>
#include <QTimer>
#include <QWheelEvent>

static QColor	s_AvatarColor(32,255,32);
static QColor	s_AvatarColorDim(32/4,255/4,32/4);
static QColor	s_AvatarColorDots(128,255,128);
static QColor	s_AxisColor(200,0,200);
static QColor	s_AxisColorDim(100,0,100);
static QColor	s_BlackColor(0,0,0);
static QColor	s_DetectedColor(128,227,255);
static QColor	s_StarColor(255,255,255);
static QColor	s_UnkColor(255,0,255);

class AuGalaxySpot
{
public:
	QColor								m_Color;				// color of spot
	QPoint								m_Loc;					// pixel location of spot
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStarMapWidget::AuStarMapWidget(QWidget* parent) : QWidget(parent), m_Cursor(QPixmap(":data/CrossHairCursor.png"), 32,32)
{
	m_CountSelected = 0;
	m_Density = 100;
	m_FilterExplored = false;
	m_FilterRange = false;
	m_FilterSelection = false;
	m_Galaxy = GALAXY_Count;
	m_InGalaxy = false;
	m_LeftButtonMode = 0;
	m_LookDist = 1;
	m_MagnifierScale = 1;
	m_PreviewCityEmpire = 0;
	m_PreviewGalaxy = ShoresOfHazeron;
	m_PreviewGenHab = 0;
	m_PreviewGenSeed = 0;
	m_PreviewIsCoreBlackHole = false;
	m_PreviewIsEmptySystem = false;
	m_PreviewIsExplored = false;
	m_PreviewIsScanned = false;
	m_PreviewHasSpacecraft = false;
	m_PreviewType = DaAtom::ATOM_Invalid;	// AtomType of preview atom.
	m_Range = 0;
	m_ShowGalaxy = true;
	m_ShowScanned = true;
	m_ShowSignals = true;
	m_ShowShips = true;
	m_ShowStations = true;
	m_ShowWormholes = true;
	m_TargetCityEmpire = 0;
	m_TargetGalaxy = ShoresOfHazeron;
	m_TargetGenHab = 0;
	m_TargetGenSeed = 0;
	m_TargetHasSpacecraft = false;
	m_TargetIsCoreBlackHole = false;
	m_TargetIsEmptySystem = false;
	m_TargetIsExplored = false;
	m_TargetIsScanned = false;
	m_TargetType = DaAtom::ATOM_Invalid;	// AtomType of target atom.
	m_timer = 0;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setCursor(m_Cursor);
	setFocusPolicy(Qt::StrongFocus);
	setMouseTracking(true);
}

AuStarMapWidget::~AuStarMapWidget()
{
}

void AuStarMapWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(250);
}

void AuStarMapWidget::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	update();
}

void AuStarMapWidget::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);

	// Capture information about the closest atom to the mouse press event. This is the target atom.
	m_Target = m_Preview;				// Atom that is at the mouse press location.
	m_TargetLoc = m_PreviewLoc;			// Galaxy coordinate of target atom
	m_TargetName = m_PreviewName;		// Target atom name.
	m_TargetType = m_PreviewType;		// AtomType of preview atom.
	m_TargetGalaxy = m_PreviewGalaxy;
	m_TargetGenHab = m_PreviewGenHab;
	m_TargetGenSeed = m_PreviewGenSeed;
	m_TargetHasSpacecraft = m_PreviewHasSpacecraft;
	m_TargetCityEmpire = m_PreviewCityEmpire;
	m_TargetIsCoreBlackHole = m_PreviewIsCoreBlackHole;
	m_TargetIsEmptySystem = m_PreviewIsEmptySystem;
	m_TargetIsExplored = m_PreviewIsExplored;
	m_TargetIsScanned = m_PreviewIsScanned;
	m_TargetSector = m_PreviewSector;

	m_LeftButtonMode = 0;
	m_MouseDownPt = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		if(m_Magnifier.contains(e->pos()))
		{
			setCursor(QCursor(Qt::SizeVerCursor));
			m_LeftButtonMode = 2;								// zoom
			m_MagnifierScale = m_LookDist;						// Capture current scale before we start messing with it.
		}
		else
		{
			setCursor(QCursor(Qt::ClosedHandCursor));
			m_LeftButtonMode = 1;								// rotate
		}
	}
}

void AuStarMapWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);
	if(e->button() == Qt::LeftButton)
	{
		// Ignore double-click in the magnifier.
		if(m_Magnifier.contains(e->pos()))
			return;

		// Center view on preview atom, which is the closest atom to the cursor.
		if(!m_Target.IsNull())
		{
			emit PlaceDoubleClicked(m_Target);
			m_Target = AuId(0);		// Prevent the mouse release event from sending a clicked signal on this guy.
		}
	}
}

void AuStarMapWidget::PreviewClosestPlace(const QPoint& pos)
{
	// Calculate the combined model view projection matrix.
	AuMatrix4x4 mv, mvp;
	GetMVP(mv, mvp);

	AuPoint curpos(pos);
	m_Preview = AuId(0);
	double closestDist2 = -1;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.Galaxy() != Galaxy())
			continue;

		if(m_CountSelected && m_FilterSelection)
		{
			// There are selected items and we are filtering only on selected items.
			bool skipIt = !(*pi).second.m_Selected;
			if(skipIt && !(*pi).second.m_SectorId.IsNull())
			{
				// The place is not selected and it's not a sector.
				// Don't skip if the parent sector is selected.
				unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.find((*pi).second.m_SectorId);
				if(spi != m_Places.end() && (*spi).second.m_Selected)
					skipIt = false;								// Parent sector is selected.
			}
			if(skipIt)
				continue;
		}

		switch((*pi).second.m_AtomType)
		{
		case DaAtom::ATOM_Sector:
			if(!m_FilterRange || m_SectorIds.contains((*pi).first))
			{
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < -120)
				{
					// Sector location is in front of the eye plane and farther than the distance where we see its stars.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					double dist2 = pxloc.Distance2_2D(curpos);
					if(dist2 > 20*20)
						continue;	// more than 100 pixels away
					if(closestDist2 < 0 || dist2 < closestDist2)
					{
						closestDist2 = dist2;
						m_Preview = (*pi).first;
						m_PreviewLoc = (*pi).second.m_Loc;
						m_PreviewName = (*pi).second.m_Name;
						m_PreviewSector = (*pi).first;
						m_PreviewType = (*pi).second.m_AtomType;
					}
				}
			}
			break;
		case DaAtom::ATOM_SolarSystem:
			if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
				continue;	// Filtering by range and the sector is out of range.
			if((*pi).second.m_SystemExplored || !m_FilterExplored)
			{
				// System is explored or we are not filtering out unexplored systems.
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < 0 && eyeLoc.z > -120)
				{
					// Solar system location is in front of the eye plane and close enough to see.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					double dist2 = pxloc.Distance2_2D(curpos);
					if(dist2 > 100*100)
						continue;	// more than 100 pixels away
					if(closestDist2 < 0 || dist2 < closestDist2)
					{
						closestDist2 = dist2;
						m_Preview = (*pi).first;
						m_PreviewGalaxy = (*pi).second.m_Galaxy;
						m_PreviewGenHab = (*pi).second.m_SystemGenHab;
						m_PreviewGenSeed = (*pi).second.m_SystemGenSeed;
						m_PreviewHasSpacecraft = (*pi).second.HasShip() || (*pi).second.HasStation();
						m_PreviewIsCoreBlackHole = (*pi).second.m_SystemCoreBlackHole;
						m_PreviewIsEmptySystem = (*pi).second.m_EmptySystem;
						m_PreviewIsExplored = (*pi).second.m_SystemExplored;
						m_PreviewIsScanned = (*pi).second.m_SystemScanned;
						m_PreviewLoc = (*pi).second.m_Loc;
						m_PreviewName = (*pi).second.m_Name;
						m_PreviewSector = (*pi).second.m_SectorId;
						m_PreviewType = (*pi).second.m_AtomType;
						m_PreviewCityEmpire = (*pi).second.m_CityEmpire;
					}
				}
			}
			break;
		default:
			continue;
		}
	}
}

// NOTE: We have enabled mouse tracking so this gets called whenever the mouse moves, even if no button is depressed.
void AuStarMapWidget::mouseMoveEvent(QMouseEvent* e)
{
	QWidget::mouseMoveEvent(e);
	if(m_LeftButtonMode == 0)
	{
		// Show nearby object.
		PreviewClosestPlace(e->pos());
		//qDebug() << "AuStarMapWidget::mouseMoveEvent preview" << m_Preview << m_PreviewName
			//<< "target" << m_Target << m_TargetName;
	}
	else
	{
		m_Preview = AuId(0);									// Turn off preview while mouse is down.
		m_Target = AuId(0);										// Do nothing to any target items when dragging view.
		//qDebug() << "AuStarMapWidget::mouseMoveEvent preview cleared" << "target" << m_Target << m_TargetName;
		if(m_LeftButtonMode == 1)
		{
			// Rotate view around look point.
			int dx = e->pos().x() - m_MouseDownPt.x();
			int dy = m_MouseDownPt.y() - e->pos().y();
			//qDebug() << "dx" << dx << "dy" << dy;
			m_MouseDownPt = e->pos();

			AuMatrix4x4 t;
			t.RotateY(DEG2RAD*dx);
			t.RotateX(-DEG2RAD*dy);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();

			InvalidateBackground();
			update();
		}
		else if(m_LeftButtonMode == 2)
		{
			// Zoom closer/farther from look point.
			float oldLookDist = m_LookDist;

			int dy = m_MouseDownPt.y() - e->pos().y();
			//m_Scale += .01f * dy;
			if(dy > 0)
			{
				// Zooming in
				if(m_LookDist > 1)
				{
					// Max zoom is at top of widget.
					// Interpolate between current zoom at center and max at top.
					float sclRange = 10.f - m_MagnifierScale;
					float sclPerPixel = sclRange / m_MouseDownPt.y();
					m_LookDist = m_MagnifierScale + sclPerPixel * dy;
				}
			}
			else
			{
				// Zooming out
				if(m_LookDist < 15000)
				{
					// Min zoom is at bottom of widget.
					// Interpolate between current zoom at center and min at bottom.
					float sclRange = 15000.f - m_MagnifierScale;
					float sclPerPixel = sclRange / (height() - m_MouseDownPt.y());
					m_LookDist = m_MagnifierScale - sclPerPixel * dy;
				}
			}

			// Restrict range of scale.
			if(m_LookDist > 15000)
				m_LookDist = 15000;
			else if(m_LookDist < 1)
				m_LookDist = 1;
			
			if(m_LookDist != oldLookDist)
			{
				InvalidateBackground();
				update();
			}
		}
	}
}

void AuStarMapWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(m_Cursor);
	m_LeftButtonMode = 0;
	QWidget::mouseReleaseEvent(e);

	if(e->button() == Qt::LeftButton)
	{
		//qDebug() << "AuStarMapWidget::mouseReleaseEvent preview" << m_Preview << m_PreviewName
			//<< "target" << m_Target << m_TargetName;
		if(!m_Target.IsNull())
			emit PlaceClicked(m_Target);
	}
	else if(e->button() == Qt::RightButton)
	{
		// Build a right-click context menu.
		QMenu menu(this);

		QAction* lookAt = 0;
		QAction* selectSystem = 0;
		QAction* selectSector = 0;
		QAction* search = 0;
		QAction* reportExplored = 0;
		QAction* reportObserved = 0;
		QAction* reportSurveyed = 0;
		QAction* reportSpacecraft = 0;
		QAction* domainAdd = 0;
		QAction* goDirect = 0;
		QAction* goDirectToCoord = 0;
		QAction* goViaWormholes = 0;
		QAction* doScanSystem = 0;
		QAction* doScanTactical = 0;
		unordered_map<AuId, QAction*, AuIdHash> orbitCities;
		unordered_map<AuId, QAction*, AuIdHash> landAtCities;
		unordered_map<AuId, QAction*, AuIdHash> reportCitiesLast;
		unordered_map<AuId, QAction*, AuIdHash> reportCitiesHistory;
		if(!m_Target.IsNull())
		{
			if(m_TargetIsEmptySystem)
			{
				selectSector = menu.addAction(QIcon(":data/RangeSector.png"), tr("Select Sector"));
			}
			else
			{
				lookAt = menu.addAction(QIcon(":data/MsgSpot.png"), tr("Look At %1").arg(m_TargetName));
				selectSystem = menu.addAction(QIcon(":data/RangeSystem.png"), tr("Select %1").arg(m_TargetName));
				selectSector = menu.addAction(QIcon(":data/RangeSector.png"), tr("Select Sector Containing %1").arg(m_TargetName));
				search = menu.addAction(QIcon(":data/bObservatory.png"), tr("Search for Solar Systems Near %1").arg(m_TargetName));
			}

			if(m_TargetType == DaAtom::ATOM_SolarSystem)
			{
				reportSpacecraft = menu.addAction(QIcon(":data/Spacecraft.png"), tr("%1 Spacecraft Report...").arg(m_TargetName));
				if(!m_TargetHasSpacecraft)
					reportSpacecraft->setEnabled(false);

				if(!m_TargetIsEmptySystem)
				{
					if(m_TargetIsScanned)
						reportSurveyed = menu.addAction(QIcon(":data/SystemSurvey.png"),
								tr("%1 Survey Report...").arg(m_TargetName));
					else if(m_TargetIsExplored)
						reportExplored = menu.addAction(QIcon(":data/SystemWorlds.png"),
								tr("%1 Exploration Report...").arg(m_TargetName));
					else
						reportObserved = menu.addAction(QIcon(":data/bObservatory.png"),
								tr("%1 Observatory Report...").arg(m_TargetName));

					if(!auApp->IsSingleUserDesignerMode())
						domainAdd = menu.addAction(QIcon(":data/AvatarDomain.png"), tr("Add %1 to Land...").arg(m_TargetName));
				}
			}

			if(!auApp->IsSingleUserDesignerMode())
			{
				menu.addSeparator();

				// Travel orders
				if(m_TargetType == DaAtom::ATOM_Sector)
				{
					AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, m_TargetLoc);
					goDirectToCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
				else if(m_TargetType == DaAtom::ATOM_SolarSystem)
				{
					if(!m_TargetIsEmptySystem)
					{
						AuMissionOrderGoToSystemDirect order1(m_Target, m_TargetName, m_TargetLoc);
						goDirect = menu.addAction(QIcon(order1.ListNameIcon()), order1.ListName());

						AuMissionOrderGoToSystemViaWormholes order2(m_Target, m_TargetName);
						goViaWormholes = menu.addAction(QIcon(order2.ListNameIcon()), order2.ListName());

						AuMissionOrderScanSurveySystem order3(false);
						doScanSystem = menu.addAction(QIcon(order3.ListNameIcon()), order3.ListName());
						if(m_TargetIsScanned)
							doScanSystem->setEnabled(false);
					}

					AuMissionOrderScanTactical order4(false);
					doScanTactical = menu.addAction(QIcon(order4.ListNameIcon()), order4.ListName());

					if(!m_TargetIsEmptySystem)
					{
						// Look for cities.
						AuIdList cityIds;
						QStringList cityNames;
						QList<quint32> cityEmpires;
						g_AuClient.StarMapListCitiesInSystem(m_Target, cityIds, cityNames, cityEmpires);
						if(!cityIds.isEmpty())
						{
							QMenu* reportCitiesLastMenu = menu.addMenu(QIcon(":data/MailCityStatus.png"), tr("Recent Report"));

							QMenu* reportCitiesHistoryMenu = menu.addMenu(QIcon(":data/MailCityStatus.png"), tr("History Report"));

							AuMissionOrderGoToOrbitCity oGoToOrbitCity;
							QMenu* orbitMenu = menu.addMenu(QIcon(oGoToOrbitCity.ListNameIcon()), oGoToOrbitCity.ListName());

							AuMissionOrderGoToCityAnywhere oGoToCityAnywhere;
							QMenu* landAtMenu = menu.addMenu(QIcon(oGoToCityAnywhere.ListNameIcon()), oGoToCityAnywhere.ListName());

							AuIdList::const_iterator ii = cityIds.cbegin();
							QStringList::const_iterator ni = cityNames.cbegin();
							QList<quint32>::const_iterator ei = cityEmpires.cbegin();
							while(ii != cityIds.cend())
							{
								// Get the city's empire data record, if it's available.
								DgEmpire cityEmpire;
								cityEmpire.m_id = *ei;
								const DgData& gd = g_Au.LockDataForRead();
								bool hasEmpire = gd.Empire(cityEmpire);
								g_Au.UnlockData();

								AuView* pView = auApp->Frame()->View();
								quint32 sceneEmpire = pView->SceneAtomEmpire();

								if(hasEmpire)
								{
									QString nameWithEmp = cityEmpire.Name() + " " + *ni;

									QPixmap empireFlag;
									empireFlag.convertFromImage(cityEmpire.m_flag);
									QIcon flagIcon(empireFlag);

									QAction* landAtCity = landAtMenu->addAction(flagIcon, nameWithEmp);
									landAtCities.insert(pair<AuId, QAction*>(*ii, landAtCity));

									QAction* orbitCity = orbitMenu->addAction(flagIcon, nameWithEmp);
									orbitCities.insert(pair<AuId, QAction*>(*ii, orbitCity));

									QAction* reportCityLast = reportCitiesLastMenu->addAction(flagIcon, nameWithEmp + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityLast->setEnabled(false);
									reportCitiesLast.insert(pair<AuId, QAction*>(*ii, reportCityLast));

									QAction* reportCityHistory = reportCitiesHistoryMenu->addAction(flagIcon, nameWithEmp + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityHistory->setEnabled(false);
									reportCitiesHistory.insert(pair<AuId, QAction*>(*ii, reportCityHistory));
								}
								else
								{
									QAction* landAtCity = landAtMenu->addAction(QIcon(":data/City.png"), *ni);
									landAtCities.insert(pair<AuId, QAction*>(*ii, landAtCity));

									QAction* orbitCity = orbitMenu->addAction(QIcon(":data/City.png"), *ni);
									orbitCities.insert(pair<AuId, QAction*>(*ii, orbitCity));

									QAction* reportCityLast = reportCitiesLastMenu->addAction(QIcon(":data/MailCityStatus.png"),
											*ni + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityLast->setEnabled(false);
									reportCitiesLast.insert(pair<AuId, QAction*>(*ii, reportCityLast));

									QAction* reportCityHistory =
										reportCitiesHistoryMenu->addAction(QIcon(":data/MailCityStatus.png"), *ni + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityHistory->setEnabled(false);
									reportCitiesHistory.insert(pair<AuId, QAction*>(*ii, reportCityHistory));
								}

								ii++;
								ni++;
								ei++;
							}
						}
					}
				}
			}

			// Separator before the go to coordinates orders.
			menu.addSeparator();
		}

		// Determine the point in the plane of the galaxy.
		QAction* goDirectToZ0Coord = 0;	// Coord at Z=0 plane of the galaxy.
		AuPoint z0Loc;					// Resulting point in the z=0 plane.
		QAction* goDirectToZSCoord = 0;	// Coord at Z=sceneAtomLoc.z plane of the galaxy.
		AuPoint zSLoc;					// Resulting point in the z=sceneAtomLoc.z plane.
		{
			// Calculate the combined model view projection matrix.
			AuMatrix4x4 mvp;
			GetMVP(mvp);
			AuMatrix4x4 twp;				// Plane into which the point is projected.
			if(mvp.UnProjectIntoPlane(z0Loc, e->pos(), twp))
			{
				z0Loc.Round(.0001);

				AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, z0Loc);
				goDirectToZ0Coord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
			}

			if(m_InGalaxy)
			{
				twp.Translate(m_SceneAtomLoc);
				if(mvp.UnProjectIntoPlane(zSLoc, e->pos(), twp))
				{
					zSLoc.Round(.0001);

					AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, zSLoc);
					goDirectToZSCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
			}
		}

		QAction* action = menu.exec(QCursor::pos());
		if(!action)
			return;

		if(action == lookAt)
		{
			// Center view on preview atom, which is the closest atom to the cursor.
			emit PlaceDoubleClicked(m_Target);
		}
		else if(action == search)
		{
			emit SystemSearchAt(m_TargetGalaxy, m_TargetLoc);
		}
		else if(action == selectSystem)
		{
			emit PlaceClicked(m_Target);
		}
		else if(action == selectSector)
		{
			emit PlaceClicked(m_TargetSector);
		}
		else if(action == reportSurveyed)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 2);
		}
		else if(action == reportExplored)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 1);
		}
		else if(action == reportObserved)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 0);
		}
		else if(action == domainAdd)
		{
			emit DomainAdd(m_Target, m_TargetName);
		}
		else if(action == reportSpacecraft)
		{
			AuView* pView = auApp->Frame()->View();
			quint32 sceneEmpire = pView->SceneAtomEmpire();
			emit ReportSpacecraft(sceneEmpire, m_TargetSector, m_Target, m_TargetName);
		}
		else if(action == goDirect)
		{
			emit GoDirectToSystem(m_Target, m_TargetName, m_TargetLoc);
		}
		else if(action == goDirectToCoord)
		{
			emit GoDirectToCoord(m_TargetLoc);
		}
		else if(action == goDirectToZ0Coord)
		{
			emit GoDirectToCoord(z0Loc);
		}
		else if(action == goDirectToZSCoord)
		{
			emit GoDirectToCoord(zSLoc);
		}
		else if(action == goViaWormholes)
		{
			emit GoViaWormholes(m_Target, m_TargetName);
		}
		else if(action == doScanSystem)
		{
			emit ScanSystem();
		}
		else if(action == doScanTactical)
		{
			emit ScanTactical();
		}
		else
		{
			// Check cities
			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = orbitCities.begin(); ci != orbitCities.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit OrbitCity((*ci).first, (*ci).second->text());
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = landAtCities.begin(); ci != landAtCities.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit LandAtCity((*ci).first, (*ci).second->text());
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = reportCitiesLast.begin();
					ci != reportCitiesLast.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit ReportCityLast((*ci).first);
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = reportCitiesHistory.begin();
					ci != reportCitiesHistory.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit ReportCityHistory((*ci).first);
					break;
				}
			}
		}
	}
}

void AuStarMapWidget::wheelEvent(QWheelEvent *e)
{
	double moveDist;
	if(e->modifiers() & Qt::ShiftModifier)
		moveDist = 10;
	else
		moveDist = 1000;

	float oldLookDist = m_LookDist;
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() > 0)
	{
		// Moving closer.
		if(m_LookDist > moveDist)
			m_LookDist -= moveDist;
		else
			m_LookDist /= 2;
	}
	else if(e->angleDelta().y() < 0)
	{
		// Moving farther away.
		if(m_LookDist > moveDist || m_LookDist*2 > moveDist)
			m_LookDist += moveDist;
		else
			m_LookDist *= 2;
	}
#else
	if(e->delta() > 0)
	{
		// Moving closer.
		if(m_LookDist > moveDist)
			m_LookDist -= moveDist;
		else
			m_LookDist /= 2;
	}
	else if(e->delta() < 0)
	{
		// Moving farther away.
		if(m_LookDist > moveDist || m_LookDist*2 > moveDist)
			m_LookDist += moveDist;
		else
			m_LookDist *= 2;
	}
#endif

	if(m_LookDist > 15000)
		m_LookDist = 15000;
	else if(m_LookDist < 1)
		m_LookDist = 1;

	QWidget::wheelEvent(e);
	
	if(m_LookDist != oldLookDist)
	{
		InvalidateBackground();
		update();
	}
}

void AuStarMapWidget::InvalidateBackground()
{
	// Release the current background image.
	if(!m_Background.isNull())
		m_Background = QImage();
}

void AuStarMapWidget::PaintBackground(const AuMatrix4x4& mvp, const AuMatrix4x4& mv)
{
	//QCursor oldCursor = cursor();
	//setCursor(QCursor(Qt::BusyCursor));

	// Create a background image, discarding any previous background image.
	m_Background = QImage(width(), height(), QImage::Format_RGB32);

	// Prepare to paint into the background image.
	QPainter p(&m_Background);

	// Fill background with a dark color.
	QBrush brBack(s_BlackColor);
	p.fillRect(0,0, width(),height(), brBack);
	if(Galaxy() >= GALAXY_Count)
		return;

	// Calculate axis points of the galaxy.
	const double galaxyRadius = DaGalaxy::Radius(Galaxy());		// in parsecs
	AuPoint originE = mv.Project(AuPoint(0,0));
	AuPoint negXE = mv.Project(AuPoint(-galaxyRadius,0));
	AuPoint posXE = mv.Project(AuPoint(galaxyRadius,0));
	AuPoint negYE = mv.Project(AuPoint(0,-galaxyRadius));
	AuPoint posYE = mv.Project(AuPoint(0,galaxyRadius));
	AuPoint negZE = mv.Project(AuPoint(0,0,-galaxyRadius/4));
	AuPoint posZE = mv.Project(AuPoint(0,0,galaxyRadius/4));
	if(originE.z > 0 && negXE.z > 0 && posXE.z > 0 && negYE.z > 0 && posYE.z > 0 && negZE.z > 0 && posZE.z > 0)
		return;							// The galaxy is entirely behind the eye plane.

	//AuPoint origin = mvp.Project(AuPoint(0,0));
	AuPoint negX = mvp.Project(AuPoint(-galaxyRadius,0));
	AuPoint posX = mvp.Project(AuPoint(galaxyRadius,0));
	AuPoint negY = mvp.Project(AuPoint(0,-galaxyRadius));
	AuPoint posY = mvp.Project(AuPoint(0,galaxyRadius));
	//AuPoint negZ = mvp.Project(AuPoint(0,0,-galaxyRadius/4));
	//AuPoint posZ = mvp.Project(AuPoint(0,0,galaxyRadius/4));

	// Draw axes of the galaxy.
	// Draw these as multiple segements so we may see part of them even if one end is really far off the screen.
	p.setPen(QPen(s_AxisColorDim, 0.f));
	PaintLine(p, mvp, mv, AuPoint(-galaxyRadius,0), AuPoint(galaxyRadius,0), 250);
	PaintLine(p, mvp, mv, AuPoint(0,-galaxyRadius), AuPoint(0,galaxyRadius), 250);
	PaintLine(p, mvp, mv, AuPoint(0,0,-galaxyRadius/5), AuPoint(0,0,galaxyRadius/5), 250/5);

	// Label the axes
	if(negXE.z < 0 || posXE.z < 0 || negYE.z < 0 || posYE.z < 0)
	{
		p.setFont(QFont(g_Au.TypefaceSans(),12));
		p.setPen(QPen(s_AxisColor, 0.f));
		if(negXE.z < 0)
			p.drawText(negX.x, negX.y, "W");
		if(posXE.z < 0)
			p.drawText(posX.x, posX.y, "E");
		if(negYE.z < 0)
			p.drawText(negY.x, negY.y, "S");
		if(posYE.z < 0)
			p.drawText(posY.x, posY.y, "N");
	}

	if(m_ShowGalaxy)
	{
		/*
		// Determine near and far z depths in the current view.
		double zmin;
		//if(originE.z < 0)
			zmin = origin.z;
		//else
			//zmin = 1;
		double zmax = zmin;
		//if(negXE.z < 0)
		{
			if(negX.z < zmin)
				zmin = negX.z;
			if(negX.z > zmax)
				zmax = negX.z;
		}
		//if(negYE.z < 0)
		{
			if(negY.z < zmin)
				zmin = negY.z;
			if(negY.z > zmax)
				zmax = negY.z;
		}
		//if(negZE.z < 0)
		{
			if(negZ.z < zmin)
				zmin = negZ.z;
			if(negZ.z > zmax)
				zmax = negZ.z;
		}
		//if(posXE.z < 0)
		{
			if(posX.z < zmin)
				zmin = posX.z;
			if(posX.z > zmax)
				zmax = posX.z;
		}
		//if(posYE.z < 0)
		{
			if(posY.z < zmin)
				zmin = posY.z;
			if(posY.z > zmax)
				zmax = posY.z;
		}
		//if(posZE.z < 0)
		{
			if(posZ.z < zmin)
				zmin = posZ.z;
			if(posZ.z > zmax)
				zmax = posZ.z;
		}
		if(zmin < 1)
			zmin = 1;
		if(zmax < 1)
			zmax = 1;
		double zrange = zmax - zmin;
		//qDebug() << "zmin" << zmin << "zmax" << zmax << "zrange" << zrange;

		if(zrange != 0)
		{
			// Draw the galaxy through the lens of this widget.
			DaGalaxy galaxy(Galaxy());							// For star density calculations.
			const int step = (width()/100) | 0x01;				// Odd number brush size seems to fill better.
			const int zsteps = 15;
			double curdensity = 0;
			const double sectorRadius = galaxyRadius / 10;
			for(int ix=1; ix<width(); ix+=step)
				for(int iy=1; iy<height(); iy+=step)
				{
					// Sample at multiple screen z-depths.
					double density = 0;
					float sectorRed = 0;
					float sectorGreen = 0;
					float sectorBlue = 0;
					for(int iz=0; iz<=zsteps; iz++)
					{
						double zdepth = zmin + zrange*iz/zsteps;
						AuPoint sectorLoc = mvp.UnProject(ix, iy, zdepth);
						sectorLoc /= 10;						// Convert from parsecs to whole sectors.
						sectorLoc.Round(10);					// Sector locations are in increments of 10.
						sectorLoc += AuPoint(5,5,5);			// Sector locations always fall on the 5's.
						if(fabs(sectorLoc.x) > sectorRadius)
							continue;
						if(fabs(sectorLoc.y) > sectorRadius)
							continue;
						if(fabs(sectorLoc.z) > sectorRadius/4)
							continue;
						if(sectorLoc.Length2() > sectorRadius*sectorRadius)
							continue;
						AuPoint sectorLocE = mv.Project(sectorLoc);
						if(sectorLocE.z > 0)
							continue;							// The sector loc is behind the eye plane.
						sectorLoc *= 10;						// Convert back to parsecs.
						QColor starColor = galaxy.Color(sectorLoc);
						sectorRed += starColor.redF();
						sectorGreen += starColor.greenF();
						sectorBlue += starColor.blueF();
						density += starColor.alphaF();
					}
					sectorRed /= zsteps;
					sectorGreen /= zsteps;
					sectorBlue /= zsteps;
					density /= zsteps;
					density *= 10;

					if(density <= .001)
						continue;
					else if(density > 1)
						density = 1;

					if(density != curdensity)
					{
						curdensity = density;

						QColor sectorColor;
						sectorColor.setRedF(sectorRed);
						sectorColor.setGreenF(sectorGreen);
						sectorColor.setBlueF(sectorBlue);
						sectorColor.setAlphaF(density);
						QPen pen(sectorColor);
						pen.setWidth(step);
						p.setPen(pen);
					}
					p.drawPoint(ix, iy);
				}
		}
		*/
		if(m_Density)
		{
			if(m_SpotLocs.isEmpty())
			{
				// Update spot locations cache.
				DaGalaxy galaxy(Galaxy());							// For star density calculations.
				AuDice dice;										// For consistent random noise.
				int pixelStep = galaxy.Radius()/m_Density;
				for(int ix=-galaxy.Radius(); ix<=galaxy.Radius(); ix+=pixelStep)
					for(int iy=-galaxy.Radius(); iy<=galaxy.Radius(); iy+=pixelStep)
						for(int iz=-galaxy.Radius()/10; iz<=galaxy.Radius()/10; iz+=pixelStep)
						{
							// Apply some noise to the sector location so we don't make a perfectly regular grid of dots.
							AuPoint sectorLoc(ix + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)),
									iy + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)),
									iz + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)));

							// Determine color of spot.
							AuGalaxySpot spot;
							spot.m_Color = galaxy.Color(sectorLoc);
							if(spot.m_Color.alphaF() == 0)
								continue;

							// Append non-zero sector location to my spot list.
							m_SpotLocs.insert(sectorLoc, spot.m_Color);
						}
				//qDebug() << "spotLocs" << m_SpotLocs.size();
			}

			// Z-sort spot locations using spot location cache.
			QMultiMap<double, AuGalaxySpot> spots;				// key is z depth, value is final spot
			for(QMap<AuPoint, QColor>::const_iterator sli = m_SpotLocs.cbegin(); sli != m_SpotLocs.cend(); sli++)
			{
				// Determine color of spot.
				AuGalaxySpot spot;
				spot.m_Color = *sli;
				if(spot.m_Color.alphaF() == 0)
					continue;

				// Determine Z depth of spot.
				AuPoint pxLocE = mv.Project(sli.key());
				if(pxLocE.z > 0)
					continue;							// Positive Z depth is behind eye plane.

				// Determine 2D pixel location of spot.
				AuPoint pxLoc = mvp.Project(sli.key());
				spot.m_Loc = QPoint(pxLoc.x, pxLoc.y);
				if(!rect().contains(spot.m_Loc))
					continue;

				// Save spot in the map so it will be Z sorted.
				spots.insert(pxLocE.z, spot);			// Key with positive Z depth for sorting near to far.
			}

			// Draw spots.
			QColor curColor(s_BlackColor);
			p.setPen(QPen(curColor, 0.f));
			for(QMultiMap<double, AuGalaxySpot>::const_iterator si = spots.cbegin(); si != spots.cend(); si++)
			{
				if((*si).m_Color != curColor)
				{
					curColor = (*si).m_Color;
					p.setPen(QPen(curColor, 0.f));
				}
				p.drawPoint((*si).m_Loc);
			}
		}
	}

	p.end();

	//setCursor(oldCursor);
}

/*
void AuStarMapWidget::PaintSector(QPainter& p, const AuMatrix4x4& mvp, const AuMatrix4x4& mv, const DaSector* sector)
{
	// Calculate corner points of sector box.
	AuPoint loc = sector->Location();

	AuPoint llbe = mv.Project(loc + AuPoint(-5,-5,-5));
	AuPoint ulbe = mv.Project(loc + AuPoint(-5,5,-5));
	AuPoint urbe = mv.Project(loc + AuPoint(5,5,-5));
	AuPoint lrbe = mv.Project(loc + AuPoint(5,-5,-5));
	AuPoint llte = mv.Project(loc + AuPoint(-5,-5,5));
	AuPoint ulte = mv.Project(loc + AuPoint(-5,5,5));
	AuPoint urte = mv.Project(loc + AuPoint(5,5,5));
	AuPoint lrte = mv.Project(loc + AuPoint(5,-5,5));

	AuPoint llb = mvp.Project(loc + AuPoint(-5,-5,-5));
	AuPoint ulb = mvp.Project(loc + AuPoint(-5,5,-5));
	AuPoint urb = mvp.Project(loc + AuPoint(5,5,-5));
	AuPoint lrb = mvp.Project(loc + AuPoint(5,-5,-5));
	AuPoint llt = mvp.Project(loc + AuPoint(-5,-5,5));
	AuPoint ult = mvp.Project(loc + AuPoint(-5,5,5));
	AuPoint urt = mvp.Project(loc + AuPoint(5,5,5));
	AuPoint lrt = mvp.Project(loc + AuPoint(5,-5,5));

	// Draw a box of lines at the edges of the sector.
	// Don't draw any edge line where the end is behind the viewer.
	p.setPen(QPen(QColor(0,0,255, 64), 0.f));
	if(llbe.z < 0 && ulbe.z < 0)
		p.drawLine(QPointF(llb.x, llb.y), QPointF(ulb.x, ulb.y));
	if(ulbe.z < 0 && urbe.z < 0)
		p.drawLine(QPointF(ulb.x, ulb.y), QPointF(urb.x, urb.y));
	if(urbe.z < 0 && lrbe.z < 0)
		p.drawLine(QPointF(urb.x, urb.y), QPointF(lrb.x, lrb.y));
	if(lrbe.z < 0 && llbe.z < 0)
		p.drawLine(QPointF(lrb.x, lrb.y), QPointF(llb.x, llb.y));

	if(llte.z < 0 && ulte.z < 0)
		p.drawLine(QPointF(llt.x, llt.y), QPointF(ult.x, ult.y));
	if(ulte.z < 0 && urte.z < 0)
		p.drawLine(QPointF(ult.x, ult.y), QPointF(urt.x, urt.y));
	if(urte.z < 0 && lrte.z < 0)
		p.drawLine(QPointF(urt.x, urt.y), QPointF(lrt.x, lrt.y));
	if(lrte.z < 0 && llte.z < 0)
		p.drawLine(QPointF(lrt.x, lrt.y), QPointF(llt.x, llt.y));

	if(llbe.z < 0 && llte.z < 0)
		p.drawLine(QPointF(llb.x, llb.y), QPointF(llt.x, llt.y));
	if(ulbe.z < 0 && ulte.z < 0)
		p.drawLine(QPointF(ulb.x, ulb.y), QPointF(ult.x, ult.y));
	if(urbe.z < 0 && urte.z < 0)
		p.drawLine(QPointF(urb.x, urb.y), QPointF(urt.x, urt.y));
	if(lrbe.z < 0 && lrte.z < 0)
		p.drawLine(QPointF(lrb.x, lrb.y), QPointF(lrt.x, lrt.y));

	// Draw the solar systems in the sector.
	DaAtomList* subatoms = sector->LockSubatoms();
	if(subatoms)
	{
		p.setPen(QPen(QColor(255,255,255), 0.f));
		for(DaAtomList::const_iterator ai = subatoms->begin(); ai != subatoms->end(); ai++)
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*ai);
			if(system)
			{
				AuPoint sloc = loc + system->Location();
				AuPoint eyeLoc = mv.Project(sloc);
				if(eyeLoc.z > 0)
					continue;			// System is behind the eye plane.

				sloc = mvp.Project(sloc);
				p.drawPoint(sloc.x, sloc.y);
			}
		}
	}
	sector->UnlockSubatoms();
}
*/

void AuStarMapWidget::PaintLine(QPainter& p, const AuMatrix4x4& mvp, const AuMatrix4x4& mv, const AuPoint& p0, const AuPoint& p1,
		int segments)
{
	AuPoint segvec = AuPoint(p1 - p0)/segments;					// Vector for each segment of the line.

	// Init first end point data.
	AuPoint pseg0(p0);
	AuPoint pxseg0E = mv.Project(pseg0);
	AuPoint pxseg0 = mvp.Project(pseg0);

	// Second end point data will be calculated at each segment along the way.
	AuPoint pseg1;
	AuPoint pxseg1E;
	AuPoint pxseg1;

	// Draw each segment where both ends of the segment are in front of the eye plane.
	for(int seg=0; seg<segments; seg++)
	{
		// Calculate segment end location.
		pseg1 = pseg0 + segvec;
		pxseg1E = mv.Project(pseg1);
		pxseg1 = mvp.Project(pseg1);

		// Draw the segment when both ends of the segment are in front of the eye plane.
		if(pxseg0E.z < 0 && pxseg1E.z < 0)
			p.drawLine(QLineF(pxseg0.x, pxseg0.y, pxseg1.x, pxseg1.y));

		// Rotate the segment end points for the next segment.
		pseg0 = pseg1;
		pxseg0E = pxseg1E;
		pxseg0 = pxseg1;
	}
}

void AuStarMapWidget::GetMVP(AuMatrix4x4& mvp)
{
	AuMatrix4x4 mv;
	GetMVP(mv, mvp);
}

void AuStarMapWidget::GetMVP(AuMatrix4x4& mv, AuMatrix4x4& mvp)
{
	// Apply perspective transformation that accounts for aspect ratio of screen.
	double nearClip = 4;
	AuMatrix4x4 perspT;					// Perspective matrix.
	if(width() > 0)
		perspT.Perspective(45.0*DEG2RAD, (double)height()/(double)width(), nearClip, 100.0);
	else
		perspT.Perspective(45.0*DEG2RAD, 1, nearClip, 100.0);

	// Calculate a projection matrix that gives us results like OpenGL.
	AuMatrix4x4 pT;						// Projection matrix.
	pT.Scale(width()/2,-height()/2,1);	// Invert Y. Scale to pixels.
	pT.Translate(1,-1,0);				// Offset to upper left origin.
	pT *= perspT;

	// Calculate the model view matrix that places the camera where we want it.
	mv.LoadIdentity();
	mv.Translate(0,0,-(m_LookDist + nearClip));
	mv.Rotate(m_LookRot);
	mv.Translate(-m_LookPoint);

	// Calculate the combined model view projection matrix.
	mvp = pT * mv;
}

void AuStarMapWidget::paintEvent(QPaintEvent* pe)
{
	// Calculate the combined model view projection matrix.
	AuMatrix4x4 mv, mvp;
	GetMVP(mv, mvp);

	// Acquire current galaxy location of scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		m_InGalaxy = false;
		const DaGalaxy* galaxy = person->FindParentGalaxy();
		if(galaxy && (galaxy->Galaxy() == Galaxy() || Galaxy() == GALAXY_Count))
		{
			if(Galaxy() != galaxy->Galaxy())
			{
				m_Galaxy = galaxy->Galaxy();
				m_SpotLocs.clear();
				InvalidateBackground();
			}

			person->GetLocationRelativeToParent(m_SceneAtomLoc, galaxy);
			m_InGalaxy = true;
		}
		const DaSector* sector = person->FindParentSector();
		if(sector)
			m_SceneAtomSectorLoc = sector->Location();	// in pc
	}
	pView->UnlockSceneAtom();

	// When filtering by range, make a list of sectors that are going to be displayed.
	m_SectorIds.clear();
	if(m_FilterRange)
	{
		for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
		{
			if((*pi).second.Galaxy() != Galaxy())
				continue;	// Not in the current galaxy.
			if((*pi).second.m_AtomType != DaAtom::ATOM_Sector)
				continue;	// Not a sector.
			double secrange = m_SceneAtomSectorLoc.Distance((*pi).second.m_Loc);	// in pc
			secrange /= 10;		// in sectors
			if((int)floor(secrange) > m_Range)
				continue;
			m_SectorIds.append((*pi).first);
		}
	}

	QPainter p(this);

	// Update the background image cache. It's a lot of work to paint.
	if(m_Background.isNull() || m_Background.width() != width() || m_Background.height() != height())
		PaintBackground(mvp, mv);

	// Draw the background image into the widget.
	p.drawImage(0,0, m_Background);

	// Show all blended backgrounds of places, namely the ownership bubble around solar systems.
	QImage spotEmpire(":data/SpotEmpire.png");
	QImage spotEnemy(":data/SpotEnemy.png");
	QImage spotExplored(":data/SpotExplored.png");
	QImage spotFriend(":data/SpotFriend.png");
	QImage spotNeutral(":data/SpotNeutral.png");
	QImage spotDetected(":data/SpotDetected.png");
	QImage spotUnk(":data/SpotUnknown.png");
	QImage spotScanned(":data/SpotScanned.png");
	QImage spotStation(":data/SpaceStation.png");
	QImage spotShipM(":data/clMShip.png");	// empire
	QImage spotShipF(":data/clFShip.png");	// friend
	QImage spotShipN(":data/clNShip.png");	// neutral
	QImage spotShipE(":data/clEShip.png");	// enemy
	QImage spotStationM(":data/clMStation.png");	// empire
	QImage spotStationF(":data/clFStation.png");	// friend
	QImage spotStationN(":data/clNStation.png");	// neutral
	QImage spotStationE(":data/clEStation.png");	// enemy
	AuPoint eyeLoc;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.Galaxy() != Galaxy())
			continue;

		if(m_CountSelected && m_FilterSelection)
		{
			// There are selected items and we are filtering only on selected items.
			bool skipIt = !(*pi).second.m_Selected;
			if(skipIt && !(*pi).second.m_SectorId.IsNull())
			{
				// The place is not selected and it's not a sector.
				// Don't skip if the parent sector is selected.
				unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.find((*pi).second.m_SectorId);
				if(spi != m_Places.end() && (*spi).second.m_Selected)
					skipIt = false;								// Parent sector is selected.
			}
			if(skipIt)
				continue;
		}

		switch((*pi).second.m_AtomType)
		{
		case DaAtom::ATOM_SolarSystem:
			if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
				continue;	// Filtering by range and the sector is out of range.
			// Draw a spot at the system location.
			if(((*pi).second.m_SystemExplored || !m_FilterExplored) && (*pi).second.m_Color != s_BlackColor)
			{
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < 0 && eyeLoc.z > -200)
				{
					// Solar system location is in front of the eye plane.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					p.setPen(QPen(QColor((*pi).second.m_Color.red(), (*pi).second.m_Color.green(), (*pi).second.m_Color.blue(), 64),
								0.f));
					p.setBrush(QColor((*pi).second.m_Color.red(), (*pi).second.m_Color.green(), (*pi).second.m_Color.blue(), 64));
					double rad = (200. + eyeLoc.z) / 10.;
					QRectF rcspot(pxloc.x - rad, pxloc.y - rad, rad*2, rad*2);
					//p.drawChord(rcspot, 0, 16*360);
					if((*pi).second.m_Color == PoliticalStanceColor(POL_Empire, false))
						p.drawImage(rcspot, spotEmpire);
					else if((*pi).second.m_Color == PoliticalStanceColor(POL_Enemy, false))
						p.drawImage(rcspot, spotEnemy);
					else if((*pi).second.m_Color == PoliticalStanceColor(POL_Friend, false))
						p.drawImage(rcspot, spotFriend);
					else if((*pi).second.m_Color == PoliticalStanceColor(POL_Neutral, false))
						p.drawImage(rcspot, spotNeutral);
					else if((*pi).second.m_Color == s_DetectedColor)
						p.drawImage(rcspot, spotDetected);
					else if((*pi).second.m_Color == s_UnkColor)
						p.drawImage(rcspot, spotUnk);
					else
						p.drawImage(rcspot, spotExplored);
				}
			}
			break;
		default:
			break;
		}
	}

	// Show all places.
	p.setFont(QFont(g_Au.TypefaceSans(),10));
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.Galaxy() != Galaxy())
			continue;

		if(m_CountSelected && m_FilterSelection)
		{
			// There are selected items and we are filtering only on selected items.
			bool skipIt = !(*pi).second.m_Selected;
			if(skipIt && !(*pi).second.m_SectorId.IsNull())
			{
				// The place is not selected and it's not a sector.
				// Don't skip if the parent sector is selected.
				unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.find((*pi).second.m_SectorId);
				if(spi != m_Places.end() && (*spi).second.m_Selected)
					skipIt = false;								// Parent sector is selected.
			}
			if(skipIt)
				continue;
		}

		switch((*pi).second.m_AtomType)
		{
		case DaAtom::ATOM_Sector:
			if(m_FilterRange && !m_SectorIds.contains((*pi).first))
				continue;	// Filtering by range and the sector is out of range.
			// Draw a box around the sector.
			if((*pi).second.m_Selected || (*pi).first == m_Preview)
			{
				if((*pi).second.m_Color != s_BlackColor)
					p.setPen(QPen(QColor((*pi).second.m_Color.red(), (*pi).second.m_Color.green(), (*pi).second.m_Color.blue(),
									128), 0.f));
				else
					p.setPen(QPen(QColor(64,64,64, 128), 0.f));

				// Calculate corner points of sector box.
				AuPoint llbe = mv.Project((*pi).second.m_Loc + AuPoint(-5,-5,-5));
				AuPoint ulbe = mv.Project((*pi).second.m_Loc + AuPoint(-5,5,-5));
				AuPoint urbe = mv.Project((*pi).second.m_Loc + AuPoint(5,5,-5));
				AuPoint lrbe = mv.Project((*pi).second.m_Loc + AuPoint(5,-5,-5));
				AuPoint llte = mv.Project((*pi).second.m_Loc + AuPoint(-5,-5,5));
				AuPoint ulte = mv.Project((*pi).second.m_Loc + AuPoint(-5,5,5));
				AuPoint urte = mv.Project((*pi).second.m_Loc + AuPoint(5,5,5));
				AuPoint lrte = mv.Project((*pi).second.m_Loc + AuPoint(5,-5,5));

				AuPoint llb = mvp.Project((*pi).second.m_Loc + AuPoint(-5,-5,-5));
				AuPoint ulb = mvp.Project((*pi).second.m_Loc + AuPoint(-5,5,-5));
				AuPoint urb = mvp.Project((*pi).second.m_Loc + AuPoint(5,5,-5));
				AuPoint lrb = mvp.Project((*pi).second.m_Loc + AuPoint(5,-5,-5));
				AuPoint llt = mvp.Project((*pi).second.m_Loc + AuPoint(-5,-5,5));
				AuPoint ult = mvp.Project((*pi).second.m_Loc + AuPoint(-5,5,5));
				AuPoint urt = mvp.Project((*pi).second.m_Loc + AuPoint(5,5,5));
				AuPoint lrt = mvp.Project((*pi).second.m_Loc + AuPoint(5,-5,5));

				// Draw a box of lines at the edges of the sector.
				// Don't draw any edge line where the end is behind the viewer.
				if(llbe.z < 0 && ulbe.z < 0)
					p.drawLine(QPointF(llb.x, llb.y), QPointF(ulb.x, ulb.y));
				if(ulbe.z < 0 && urbe.z < 0)
					p.drawLine(QPointF(ulb.x, ulb.y), QPointF(urb.x, urb.y));
				if(urbe.z < 0 && lrbe.z < 0)
					p.drawLine(QPointF(urb.x, urb.y), QPointF(lrb.x, lrb.y));
				if(lrbe.z < 0 && llbe.z < 0)
					p.drawLine(QPointF(lrb.x, lrb.y), QPointF(llb.x, llb.y));

				if(llte.z < 0 && ulte.z < 0)
					p.drawLine(QPointF(llt.x, llt.y), QPointF(ult.x, ult.y));
				if(ulte.z < 0 && urte.z < 0)
					p.drawLine(QPointF(ult.x, ult.y), QPointF(urt.x, urt.y));
				if(urte.z < 0 && lrte.z < 0)
					p.drawLine(QPointF(urt.x, urt.y), QPointF(lrt.x, lrt.y));
				if(lrte.z < 0 && llte.z < 0)
					p.drawLine(QPointF(lrt.x, lrt.y), QPointF(llt.x, llt.y));

				if(llbe.z < 0 && llte.z < 0)
					p.drawLine(QPointF(llb.x, llb.y), QPointF(llt.x, llt.y));
				if(ulbe.z < 0 && ulte.z < 0)
					p.drawLine(QPointF(ulb.x, ulb.y), QPointF(ult.x, ult.y));
				if(urbe.z < 0 && urte.z < 0)
					p.drawLine(QPointF(urb.x, urb.y), QPointF(urt.x, urt.y));
				if(lrbe.z < 0 && lrte.z < 0)
					p.drawLine(QPointF(lrb.x, lrb.y), QPointF(lrt.x, lrt.y));

				AuPoint cenlocE = mv.Project((*pi).second.m_Loc);
				if(cenlocE.z < 0)
				{
					AuPoint cenloc = mvp.Project((*pi).second.m_Loc);

					if((*pi).second.m_Color != s_BlackColor)
						p.setPen(QPen((*pi).second.m_Color, 0.f));
					else
						p.setPen(QPen(QColor(64,64,64), 0.f));

					p.setFont(QFont(g_Au.TypefaceSans(),10, QFont::Bold));
					p.drawText(QPointF(cenloc.x, cenloc.y), (*pi).second.m_Name);
					p.setFont(QFont(g_Au.TypefaceSans(),10));
				}
			}
			break;
		case DaAtom::ATOM_SolarSystem:
			if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
				continue;	// Filtering by range and the sector is out of range.
			// Draw a pixel at the system location.
			if((*pi).second.m_SystemExplored || !m_FilterExplored)
			{
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < 0)
				{
					// Solar system location is in front of the eye plane.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					// Wormhole lines
					if(m_ShowWormholes && eyeLoc.z > -120)
					{
						// Within solar system view depth so show wormhole lines.
						p.setPen(QPen(QColor(192, 64, 32, 200), 0.f));
						for(QList<AuPoint>::const_iterator wi = (*pi).second.m_WormDestEx.cbegin();
								wi != (*pi).second.m_WormDestEx.cend(); wi++)
						{
							AuPoint px = mv.Project(*wi);
							if(px.z > 0)
								continue;
							AuPoint pxw = mvp.Project(*wi);
							p.drawLine(QPointF(pxloc.x,pxloc.y), QPointF(pxw.x,pxw.y));
						}

						p.setPen(QPen(QColor(32, 64, 192, 200), 0.f));
						for(QList<AuPoint>::const_iterator wi = (*pi).second.m_WormDestUnex.cbegin();
								wi != (*pi).second.m_WormDestUnex.cend(); wi++)
						{
							AuPoint px = mv.Project(*wi);
							if(px.z > 0)
								continue;
							AuPoint pxw = mvp.Project(*wi);
							p.drawLine(QPointF(pxloc.x,pxloc.y), QPointF(pxw.x,pxw.y));
						}
					}

					double rad = 10;
					QRectF rcspot(pxloc.x - rad, pxloc.y - rad, rad*2, rad*2);

					// Star dot
					if(!(*pi).second.IsEmptySystem())
					{
						if((*pi).second.m_Selected || (*pi).first == m_Preview || eyeLoc.z > -120)
						{
							QPen penStar(s_StarColor, 0.f);
							float penWidth = (120. + eyeLoc.z) / 50.;
							if(penWidth < 1)
								penWidth = 1;
							penStar.setWidthF(penWidth);
							p.setPen(penStar);
							p.drawPoint(QPointF(pxloc.x, pxloc.y));
						}

						//qDebug() << (*pi).second.m_Name << "z" << eyeLoc.z << "lookDist" << m_LookDist;
						// Scanned indicator
						if(m_ShowScanned && (*pi).second.IsScanned() && eyeLoc.z > -120)
							p.drawImage(rcspot, spotScanned);
					}

					// Signal strength
					if(m_ShowSignals && (*pi).second.m_SignalStrength && m_LookDist <= 16 && eyeLoc.z > -120 &&
							(*pi).second.m_Color != s_BlackColor)
					{
						int ht = 6;	// pixels height of each bar
						int px = rcspot.left();
						int py = (rcspot.bottom() + rcspot.top())/2 - ht/2;

						// Fill background of signal strength bar.
						//QColor barcol((*pi).second.m_Color.red(), (*pi).second.m_Color.green(), (*pi).second.m_Color.blue(), 64);
						QColor barcol(s_DetectedColor.red(), s_DetectedColor.green(), s_DetectedColor.blue(), 64);
						QRect bgrect(px - 15*2 + 1, py-1, 15*2 + 1, ht+3);
						p.fillRect(bgrect, barcol);

						// Draw signal strength indicator bars.
						p.setPen(QPen(barcol, 0.f));
						int pxb = px;
						for(int bar=0; bar < (*pi).second.m_SignalStrength; bar++, pxb-=2)
						{
							p.drawLine(pxb,py, pxb,py+ht);
							p.drawLine(pxb,py+1, pxb,py+ht-1);
						}
						p.setPen(QPen(s_DetectedColor, 0.f));
						pxb = px;
						for(int bar=0; bar < (*pi).second.m_SignalStrength; bar++, pxb-=2)
							p.drawLine(pxb,py+2, pxb,py+ht-2);
					}

					// Spacecraft
					if(m_LookDist <= 32 && eyeLoc.z > -45)
					{
						if(m_ShowStations && ((*pi).second.HasStation()))
						{
							// Draw station indicators.
							int l = rcspot.left() - spotStationM.width()/2;
							int t = rcspot.bottom();
							QRect rcsc(l, t, spotStationM.width(), spotStationM.height());
							if((*pi).second.m_EmpireStations)
							{
								p.drawImage(rcsc, spotStationM);
								if((*pi).second.m_EmpireStations > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_EmpireStations);
									p.setPen(QPen(PoliticalStanceColor(POL_Empire, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter,n);
								}
								rcsc.translate(0, spotStationM.height());
							}
							if((*pi).second.m_FriendStations)
							{
								p.drawImage(rcsc, spotStationF);
								if((*pi).second.m_FriendStations > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_FriendStations);
									p.setPen(QPen(PoliticalStanceColor(POL_Friend, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter,n);
								}
								rcsc.translate(0, spotStationF.height());
							}
							if((*pi).second.m_NeutralStations)
							{
								p.drawImage(rcsc, spotStationN);
								if((*pi).second.m_NeutralStations > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_NeutralStations);
									p.setPen(QPen(PoliticalStanceColor(POL_Neutral, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter,n);
								}
								rcsc.translate(0, spotStationN.height());
							}
							if((*pi).second.m_EnemyStations)
							{
								p.drawImage(rcsc, spotStationE);
								if((*pi).second.m_EnemyStations > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_EnemyStations);
									p.setPen(QPen(PoliticalStanceColor(POL_Enemy, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter,n);
								}
								rcsc.translate(0, spotStationE.height());
							}
						}

						if(m_ShowShips && (*pi).second.HasShip())
						{
							// Draw space ship indicators.
							int l = rcspot.left() - spotShipM.width()/2;
							int t = rcspot.top() - spotShipM.height();
							QRect rcsc(l, t, spotShipM.width(), spotShipM.height());
							if((*pi).second.m_EmpireShips)
							{
								p.drawImage(rcsc, spotShipM);
								if((*pi).second.m_EmpireShips > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_EmpireShips);
									p.setPen(QPen(PoliticalStanceColor(POL_Empire, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter, n);
								}
								rcsc.translate(8, -spotShipM.height());
							}
							if((*pi).second.m_FriendShips)
							{
								p.drawImage(rcsc, spotShipF);
								if((*pi).second.m_FriendShips > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_FriendShips);
									p.setPen(QPen(PoliticalStanceColor(POL_Friend, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter, n);
								}
								rcsc.translate(8, -spotShipF.height());
							}
							if((*pi).second.m_NeutralShips)
							{
								p.drawImage(rcsc, spotShipN);
								if((*pi).second.m_NeutralShips > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_NeutralShips);
									p.setPen(QPen(PoliticalStanceColor(POL_Neutral, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter, n);
								}
								rcsc.translate(8, -spotShipN.height());
							}
							if((*pi).second.m_EnemyShips)
							{
								p.drawImage(rcsc, spotShipE);
								if((*pi).second.m_EnemyShips > 1)
								{
									QString n = QString("%L1").arg((*pi).second.m_EnemyShips);
									p.setPen(QPen(PoliticalStanceColor(POL_Enemy, false), 0.f));
									p.drawText(rcsc.left() - 204, rcsc.top(), 200,rcsc.height(), Qt::AlignRight|Qt::AlignVCenter, n);
								}
								rcsc.translate(8, -spotShipE.height());
							}
						}
					}

					// Selected/hover extra info.
					if(((*pi).second.m_Selected || (*pi).first == m_Preview) && !(*pi).second.IsEmptySystem())
					{
						if((*pi).second.m_Color != s_BlackColor)
							p.setPen(QPen((*pi).second.m_Color, 0.f));
						else
							p.setPen(QPen(QColor(64,64,64), 0.f));

						// Show the system name.
						p.drawText(QPointF(pxloc.x+rad, pxloc.y-4), (*pi).second.m_Name);

						if((*pi).first == m_Preview)
						{
							// Preview system shows more information.
							double distToLookPt = (*pi).second.m_Loc.Distance(m_LookPoint);
							QString gloc;
							if(distToLookPt < .1)
								gloc = QString("(%2, %3, %4)")
									.arg((*pi).second.m_Loc.x)
									.arg((*pi).second.m_Loc.y)
									.arg((*pi).second.m_Loc.z);
							else
								gloc = QString("(%2, %3, %4) %L5pc")
									.arg((*pi).second.m_Loc.x)
									.arg((*pi).second.m_Loc.y)
									.arg((*pi).second.m_Loc.z)
									.arg(distToLookPt, 0,'f',2);
							pxloc.y += 16;
							p.drawText(QPointF(pxloc.x+rad, pxloc.y-4), gloc);

							// List cities under the system name.
							for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.begin();
									spi != m_Places.end(); spi++)
							{
								if((*spi).second.m_AtomType != DaAtom::ATOM_Bd)
									continue;
								if((*spi).second.m_Loc != (*pi).second.m_Loc)
									continue;
								pxloc.y += 16;
								p.drawPixmap(QPointF(pxloc.x+rad, pxloc.y-4-11), (*spi).second.m_Icon.pixmap(15,15));
								if((*spi).second.m_Color != s_BlackColor)
									p.setPen(QPen((*spi).second.m_Color, 0.f));
								else
									p.setPen(QPen(QColor(64,64,64), 0.f));
								p.drawText(QPointF(pxloc.x+rad+18, pxloc.y-4), (*spi).second.m_Name);
							}
						}
					}
				}
			}
			break;
		case DaAtom::ATOM_Avatar:
			if(m_InGalaxy)
			{
				// This is the scene atom. Update their location.
				(*pi).second.m_Loc = m_SceneAtomLoc;

				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < 0)
				{
					// Person location is in front of the eye plane.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					if((*pi).second.m_Selected)
					{
						// Show the person's name.
						p.setPen(QPen((*pi).second.m_Color, 0.f));
						p.drawText(QPointF(pxloc.x+4, pxloc.y-4), (*pi).second.m_Name);
					}
				}
			}
			break;
		default:
			break;
		}
	}

	// Show where I am.
	if(m_InGalaxy)
	{
		AuPoint eyeLoc = mv.Project(m_SceneAtomLoc);
		if(eyeLoc.z < 0)
		{
			QColor collines;
			if(AuTime::CurrentSecIsOdd())
				collines = s_AvatarColor;
			else
				collines = s_AvatarColorDim;

			// My location is in front of the eye plane.
			AuPoint pxloc = mvp.Project(m_SceneAtomLoc);
			p.setPen(QPen(collines, 0.f));
			p.drawLine(pxloc.x+2,pxloc.y, pxloc.x+12,pxloc.y);
			p.drawLine(pxloc.x-2,pxloc.y, pxloc.x-12,pxloc.y);
			p.drawLine(pxloc.x,pxloc.y+2, pxloc.x,pxloc.y+12);
			p.drawLine(pxloc.x,pxloc.y-2, pxloc.x,pxloc.y-12);
			p.setPen(QPen(s_AvatarColorDots, 0.f));
			p.drawPoint(pxloc.x+13,pxloc.y);
			p.drawPoint(pxloc.x-13,pxloc.y);
			p.drawPoint(pxloc.x,pxloc.y+13);
			p.drawPoint(pxloc.x,pxloc.y-13);
		}
	}

	// Magnification control
	{
		// Pick up the dimensions of the magnifier button.
		QImage magnifier(":data/Magnifier.png");
		m_Magnifier = QRect(0,height()/2-magnifier.height()/2,magnifier.width(),magnifier.height());
		p.setTransform(QTransform());	// Reset to identity matrix.
		p.drawImage(m_Magnifier, magnifier);
	}

	p.end();
	
	QWidget::paintEvent(pe);
}

void AuStarMapWidget::LookAtPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		bool inThisGalaxy = false;
		if(id == AuId(GALAXY_Count))// && (*pi).second.m_Loc == AuPoint(0,0))
		{
			// We do not know the location of the scene atom yet.
			// Acquire current galaxy location of scene atom.
			AuView* pView = auApp->Frame()->View();
			DaPerson* person;
			pView->LockSceneAtom(&person);
			if(person)
			{
				const DaGalaxy* galaxy = person->FindParentGalaxy();
				if(galaxy && galaxy->Galaxy() == Galaxy())
				{
					person->GetLocationRelativeToParent((*pi).second.m_Loc, galaxy);
					inThisGalaxy = true;
				}
			}
			pView->UnlockSceneAtom();
		}
		else
			inThisGalaxy = true;

		if(inThisGalaxy && m_LookPoint != (*pi).second.m_Loc)
		{
			m_LookPoint = (*pi).second.m_Loc;
			if(Galaxy() != (*pi).second.Galaxy())
			{
				m_Galaxy = (*pi).second.Galaxy();
				m_SpotLocs.clear();
			}
			InvalidateBackground();
			update();
		}
	}
}

void AuStarMapWidget::DeselectPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		if((*pi).second.m_Selected)
		{
			// We don't count the 'Current Location' and 'Galaxy' places in our selection count.
			// Otherwise, they cause wierd behavior when the selection filter is active.
			if((*pi).first.GetID() > GALAXY_Count)
				m_CountSelected--;

			(*pi).second.m_Selected = false;
			update();
		}
	}
}

void AuStarMapWidget::SelectPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		if(!(*pi).second.m_Selected)
		{
			// We don't count the 'Current Location' and 'Galaxy' places in our selection count.
			// Otherwise, they cause wierd behavior when the selection filter is active.
			if((*pi).first.GetID() > GALAXY_Count)
				m_CountSelected++;

			if(Galaxy() != (*pi).second.Galaxy())
			{
				m_Galaxy = (*pi).second.Galaxy();
				m_SpotLocs.clear();
				InvalidateBackground();
			}

			m_LastSelected = id;
			(*pi).second.m_Selected = true;
			update();
		}
	}
}

void AuStarMapWidget::HideGalaxy()
{
	if(m_ShowGalaxy)
	{
		m_ShowGalaxy = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowGalaxy", m_ShowGalaxy);
		InvalidateBackground();
		update();
	}
}

void AuStarMapWidget::ShowGalaxy()
{
	if(!m_ShowGalaxy)
	{
		m_ShowGalaxy = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowGalaxy", m_ShowGalaxy);
		InvalidateBackground();
		update();
	}
}

void AuStarMapWidget::HideScanned()
{
	if(m_ShowScanned)
	{
		m_ShowScanned = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowScanned", m_ShowScanned);
		update();
	}
}

void AuStarMapWidget::ShowScanned()
{
	if(!m_ShowScanned)
	{
		m_ShowScanned = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowScanned", m_ShowScanned);
		update();
	}
}

void AuStarMapWidget::HideShips()
{
	if(m_ShowShips)
	{
		m_ShowShips = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowShips", m_ShowShips);
		update();
	}
}

void AuStarMapWidget::ShowShips()
{
	if(!m_ShowShips)
	{
		m_ShowShips = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowShips", m_ShowShips);
		update();
	}
}

void AuStarMapWidget::HideSignals()
{
	if(m_ShowSignals)
	{
		m_ShowSignals = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowSignals", m_ShowSignals);
		update();
	}
}

void AuStarMapWidget::ShowSignals()
{
	if(!m_ShowSignals)
	{
		m_ShowSignals = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowSignals", m_ShowSignals);
		update();
	}
}

void AuStarMapWidget::HideStations()
{
	if(m_ShowStations)
	{
		m_ShowStations = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowStations", m_ShowStations);
		update();
	}
}

void AuStarMapWidget::ShowStations()
{
	if(!m_ShowStations)
	{
		m_ShowStations = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowStations", m_ShowStations);
		update();
	}
}

void AuStarMapWidget::HideWormholes()
{
	if(m_ShowWormholes)
	{
		m_ShowWormholes = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowWormholes", m_ShowWormholes);
		update();
	}
}

void AuStarMapWidget::ShowWormholes()
{
	if(!m_ShowWormholes)
	{
		m_ShowWormholes = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowWormholes", m_ShowWormholes);
		update();
	}
}

void AuStarMapWidget::FilterExplored(bool filterExplored)
{
	if(m_FilterExplored != filterExplored)
	{
		m_FilterExplored = filterExplored;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterExplored", m_FilterExplored);
		update();
	}
}

void AuStarMapWidget::FilterRange(bool filterRange)
{
	if(m_FilterRange != filterRange)
	{
		m_FilterRange = filterRange;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterRange", m_FilterRange);
		update();
	}
}

void AuStarMapWidget::FilterSelection(bool filterSelection)
{
	if(m_FilterSelection != filterSelection)
	{
		m_FilterSelection = filterSelection;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterSelection", m_FilterSelection);
		update();
	}
}

void AuStarMapWidget::RestoreSettings()
{
	//qDebug() << "AuStarMapWidget::RestoreSettings";
	/*
	double lookDist = g_Au.Settings().readNumEntry("Widget/StarMap/LookDist", m_LookDist);
	if(lookDist < 1)
		lookDist = 1;
	else if(lookDist > 15000)
		lookDist = 15000;
	if(m_LookDist != lookDist)
	{
		m_LookDist = lookDist;
		InvalidateBackground();
	}

	AuPoint lookPoint;
	lookPoint.x = g_Au.Settings().readNumEntry("Widget/StarMap/LookAtX", m_LookPoint.x);
	lookPoint.y = g_Au.Settings().readNumEntry("Widget/StarMap/LookAtY", m_LookPoint.y);
	lookPoint.z = g_Au.Settings().readNumEntry("Widget/StarMap/LookAtZ", m_LookPoint.z);
	if(fabs(lookPoint.x) > DaGalaxy::Radius(Galaxy()) ||
			fabs(lookPoint.y) > DaGalaxy::Radius(Galaxy()) ||
			fabs(lookPoint.z) > DaGalaxy::Radius(Galaxy())/4)
		lookPoint = AuPoint(0,0);
	if(m_LookPoint != lookPoint)
	{
		m_LookPoint = lookPoint;
		InvalidateBackground();
	}

	AuPoint lookRot;
	lookRot.x = g_Au.Settings().readNumEntry("Widget/StarMap/LookRotX", m_LookRot.x*RAD2DEG);
	lookRot.y = g_Au.Settings().readNumEntry("Widget/StarMap/LookRotY", m_LookRot.y*RAD2DEG);
	lookRot.z = g_Au.Settings().readNumEntry("Widget/StarMap/LookRotZ", m_LookRot.z*RAD2DEG);
	lookRot *= DEG2RAD;
	if(fabs(lookRot.x) > PI*2 ||
			fabs(lookRot.y) > PI*2 ||
			fabs(lookRot.z) > PI*2)
		lookRot = AuPoint(0,0);
	if(m_LookRot != lookRot)
	{
		m_LookRot = lookRot;
		InvalidateBackground();
	}
	*/

	int density;
	density = g_Au.Settings().readNumEntry("Widget/StarMap/Density", 100);
	if(density < 0)
		density = 0;
	else if(density > 100)
		density = 100;		// Don't load higher than 100 from settings or it can be frustrating when left at 500.
	int range;
	range = g_Au.Settings().readNumEntry("Widget/StarMap/Range", 1);
	if(range < 0)
		range = 0;
	else if(range > 1000)
		range = 1000;
	if(m_Density != density || m_Range != range)
	{
		m_Density = density;
		m_Range = range;
		InvalidateBackground();
	}

	m_FilterExplored = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterExplored", false);
	m_FilterRange = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterRange", false);
	m_FilterSelection = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterSelection", false);

	bool showGalaxy = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowGalaxy", true);
	if(m_ShowGalaxy != showGalaxy)
	{
		m_ShowGalaxy = showGalaxy;
		InvalidateBackground();
	}

	m_ShowScanned = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowScanned", true);
	m_ShowSignals = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowSignals", true);
	m_ShowShips = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowShips", true);
	m_ShowStations = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowStations", true);
	m_ShowWormholes = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowWormholes", true);
}

// 'loc' is location relative to the galaxy, in parsecs.
void AuStarMapWidget::AddPlace(const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon, const QString& name, const QColor& color,
		const AuPoint& loc, const AuId& sectorId, GALAXY galaxy, bool systemCoreBlackHole, bool isEmptySystem, quint32 systemGenHab,
		quint32 systemGenSeed, bool systemExplored, bool systemScanned, quint32 cityEmpire, quint8 signalStrength,
		quint16 empireShips, quint16 empireStations,
		quint16 friendShips, quint16 friendStations,
		quint16 neutralShips, quint16 neutralStations,
		quint16 enemyShips, quint16 enemyStations)
{
	// Grab the galaxy from the first place we receive that has a galaxy.
	if(Galaxy() == GALAXY_Count && Galaxy() != galaxy)
	{
		m_Galaxy = galaxy;
		m_SpotLocs.clear();
		InvalidateBackground();
	}

	AuStarMapWidgetPlace place(type, icon, name, color, loc, sectorId, galaxy, systemCoreBlackHole, isEmptySystem, systemGenHab,
			systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength,
			empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second = place;	// replace existing place
	else
		m_Places.insert(pair<AuId, AuStarMapWidgetPlace>(id, place));
}

void AuStarMapWidget::SetPlaceColor(const AuId& id, const QColor& color)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second.m_Color = color;
}

void AuStarMapWidget::SetPlaceIcon(const AuId& id, const QIcon& icon)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second.m_Icon = icon;
}

void AuStarMapWidget::AddPlaceWormhole(const AuId& id, const AuPoint& wormVec, bool isExplored)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		// When we have both an explored and an unexplored wormhole going to the same destination, we prefer to only show the
		// explored wormhole.
		AuPoint wormDest = (*pi).second.m_Loc + wormVec;
		if(isExplored)
		{
			if(!(*pi).second.m_WormDestEx.contains(wormDest))
			{
				(*pi).second.m_WormDestEx.append(wormDest);			// Add the destination to the explored wormholes list.
				(*pi).second.m_WormDestUnex.removeAll(wormDest);	// Remove the destination from the unexplored wormholes list.
			}
		}
		else if(!(*pi).second.m_WormDestEx.contains(wormDest))
		{
			if(!(*pi).second.m_WormDestUnex.contains(wormDest))
				(*pi).second.m_WormDestUnex.append(wormDest);		// Add to the unexplored wormholes list if not in explored list.
		}
	}
}

void AuStarMapWidget::ClearUniverse()
{
	m_Places.clear();
	m_CountSelected = 0;
}

void AuStarMapWidget::ClearGalaxy(GALAXY galaxy)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin();
	while(pi != m_Places.end())
	{
		if((*pi).second.Galaxy() == galaxy)
			pi = m_Places.erase(pi);
		else
			pi++;
	}
}

void AuStarMapWidget::ClearSectors(const AuIdList& sectorIds)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin();
	while(pi != m_Places.end())
	{
		if(sectorIds.contains((*pi).first) || sectorIds.contains((*pi).second.m_SectorId))
			pi = m_Places.erase(pi);	// Found the sector or a place that is in it.
		else
			pi++;
	}
}

void AuStarMapWidget::ClearSelection()
{
	bool anyChange = false;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.m_Selected)
		{
			(*pi).second.m_Selected = false;
			anyChange = true;
		}
	}
	m_CountSelected = 0;

	if(anyChange)
		update();
}

void AuStarMapWidget::SetDensity(int density)
{
	int newDensity;
	if(density < 0)
		newDensity = 0;
	else if(density > 500)
		newDensity = 500;
	else
		newDensity = density;
	if(m_Density != newDensity)
	{
		m_Density = newDensity;
		m_SpotLocs.clear();
		InvalidateBackground();
		g_Au.Settings().writeEntry("Widget/StarMap/Density", m_Density);
		update();
	}
}

void AuStarMapWidget::SetRange(int range)
{
	int newRange;
	if(range < 0)
		newRange = 0;
	else if(range > 1000)
		newRange = 1000;
	else
		newRange = range;
	if(m_Range != newRange)
	{
		m_Range = newRange;
		m_SpotLocs.clear();
		InvalidateBackground();
		g_Au.Settings().writeEntry("Widget/StarMap/Range", m_Range);
		update();
	}
}

void AuStarMapWidget::LookAtLastSelected()
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(m_LastSelected);
	if(pi != m_Places.end() && m_LookPoint != (*pi).second.m_Loc)
	{
		m_LookPoint = (*pi).second.m_Loc;

		if(Galaxy() != (*pi).second.Galaxy())
		{
			m_Galaxy = (*pi).second.Galaxy();
			m_SpotLocs.clear();
		}

		InvalidateBackground();
		update();
	}
}

void AuStarMapWidget::LookAtSelection()
{
	// Calculate the average center point of the selected places.
	AuPoint locavg;						// Sum of all selected locations for average.
	GALAXY galaxy = Galaxy();
	int selected = 0;					// Count of selected items for average.
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.m_Selected)
		{
			galaxy = (*pi).second.Galaxy();
			locavg += (*pi).second.m_Loc;
			selected++;
		}
	}

	if(selected)
	{
		// Move look point to the average of the selected place locations.
		if(selected > 1)
			locavg /= selected;			// Calculate the average of the selected place locations.
		if(m_LookPoint != locavg)
		{
			if(Galaxy() != galaxy)
			{
				m_Galaxy = galaxy;
				m_SpotLocs.clear();
			}

			m_LookPoint = locavg;
			InvalidateBackground();
			update();
		}
	}
}

void AuStarMapWidget::keyPressEvent(QKeyEvent *e)
{
	switch(e->key())
	{
	case Qt::Key_W:
		{
			// move forward
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(0,0,-1));
			m_LookPoint += moveVec;
			InvalidateBackground();
		}
		break;
	case Qt::Key_S:
		{
			// back up
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(0,0,1));
			m_LookPoint += moveVec;
			InvalidateBackground();
		}
		break;
	case Qt::Key_A:
		{
			// strafe left
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(-1,0,0));
			m_LookPoint += moveVec;
			InvalidateBackground();
		}
		break;
	case Qt::Key_D:
		{
			// strafe right
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(1,0,0));
			m_LookPoint += moveVec;
			InvalidateBackground();
		}
		break;
	case Qt::Key_C:
		{
			AuMatrix4x4 t;
			t.RotateZ(DEG2RAD*2);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();
			InvalidateBackground();
		}
		break;
	case Qt::Key_Z:
		{
			AuMatrix4x4 t;
			t.RotateZ(-DEG2RAD*2);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();
			InvalidateBackground();
		}
		break;
	}
	QWidget::keyPressEvent(e);
}

AuIdList AuStarMapWidget::SelectedPlaces() const
{
	AuIdList selected;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
		if((*pi).second.m_Selected)
			selected.append((*pi).first);
	return selected;
}

void AuStarMapWidget::SetLookPoint(const AuPoint& look)
{
	m_LookPoint = look;
	InvalidateBackground();
	update();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStarMapGL::AuStarMapGL(QWidget* parent) : QOpenGLWidget(parent),
	m_FontBig(g_Au.TypefaceSans(), 14),
	m_FontMedium(g_Au.TypefaceSans(), 12),
	m_FontSmall(g_Au.TypefaceSans(), 11),
	m_Cursor(QPixmap(":data/CrossHairCursor.png"), 32,32)
{
	m_MM = new AuGLModelMaker(false),
	m_ModelCache = new AuGLModelCache();
	m_RenderInfo = 0;

	m_CountSelected = 0;
	m_FilterExplored = false;
	m_FilterRange = false;
	m_FilterSelection = false;
	m_Galaxy = GALAXY_Count;
	m_GalaxySpriteTex = 0;
	m_MyLoc1Tex = 0;
	m_MyLoc2Tex = 0;
	m_StarSpriteTex = 0;
	m_ScannedSpriteTex = 0;
	for(int i=0; i<4; i++)
	{
		m_ShipTex[i] = 0;
		m_StationTex[i] = 0;
	}
	for(int i=0; i<16; i++)
		m_SignalTex[i] = 0;
	m_SystemSpriteTex = 0;
	m_InGalaxy = false;
	m_LeftButtonMode = 0;
	m_LookDist = 1;
	m_MagnifierScale = 1;
	m_PreviewCityEmpire = 0;
	m_PreviewGalaxy = ShoresOfHazeron;
	m_PreviewGenHab = 0;
	m_PreviewGenSeed = 0;
	m_PreviewIsCoreBlackHole = false;
	m_PreviewIsEmptySystem = false;
	m_PreviewIsExplored = false;
	m_PreviewIsScanned = false;
	m_PreviewHasSpacecraft = false;
	m_PreviewType = DaAtom::ATOM_Invalid;	// AtomType of preview atom.
	m_Range = 0;
	m_ShowGalaxy = true;
	m_ShowScanned = true;
	m_ShowSignals = true;
	m_ShowShips = true;
	m_ShowStations = true;
	m_ShowWormholes = true;
	m_TargetCityEmpire = 0;
	m_TargetGalaxy = ShoresOfHazeron;
	m_TargetGenHab = 0;
	m_TargetGenSeed = 0;
	m_TargetHasSpacecraft = false;
	m_TargetIsCoreBlackHole = false;
	m_TargetIsEmptySystem = false;
	m_TargetIsExplored = false;
	m_TargetIsScanned = false;
	m_TargetType = DaAtom::ATOM_Invalid;	// AtomType of target atom.

	setFocusPolicy(Qt::StrongFocus);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	setCursor(m_Cursor);
	setMouseTracking(true);
}

AuStarMapGL::~AuStarMapGL()
{
	delete m_RenderInfo;
	delete m_ModelCache;
	delete m_MM;
}

void AuStarMapGL::initializeGL()
{
	//qDebug() << "AuStarMapGL::initializeGL"
	if(m_RenderInfo)
	{
		//qWarning() << "error: AuStarMapGL::initializeGL called after it was already called";
		delete m_RenderInfo;
	}
	m_RenderInfo = new AuAtomRenderInfo(this, 0, *m_MM, *m_ModelCache, m_FontMedium);
	m_RenderInfo->InitializeGL();

	// Configure for solid rendered display.
	m_RenderInfo->re->glFrontFace(GL_CW);						// Front faces are those drawn clockwise.
	m_RenderInfo->re->glCullFace(GL_BACK);						// Cull back-facing faces.
	m_RenderInfo->re->glEnable(GL_CULL_FACE);					// Enable face culling.
	m_RenderInfo->re->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	m_RenderInfo->re->glEnable(GL_DEPTH_TEST);					// Enable depth calculations
	m_RenderInfo->re->glClearColor(0,0,0,1);
}

void AuStarMapGL::paintGL()
{
	//qDebug() << "AuStarMapGL::paintGL";
	m_RenderInfo->re->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(Galaxy() >= GALAXY_Count)
		return;

	AuMatrix4x4 p, mv, mvp;
	GetMVP(p, mv, mvp);

	// Render background axes and galaxy density blobs.
	{
		// Calculate axis points of the galaxy.
		const double galaxyRadius = DaGalaxy::Radius(Galaxy());		// in parsecs
		AuPointArray verts;
		verts.append(AuPoint(-galaxyRadius,0));
		verts.append(AuPoint(galaxyRadius,0));
		verts.append(AuPoint(0,-galaxyRadius));
		verts.append(AuPoint(0,galaxyRadius));
		verts.append(AuPoint(0,0,-galaxyRadius/4));
		verts.append(AuPoint(0,0,galaxyRadius/4));

		// Create render objects.
		if(!m_AxesVAO.VAO())
			m_AxesVAO.CreateForLines(*m_RenderInfo->re, verts, false, GL_STATIC_DRAW);

		if(m_ShowGalaxy)
		{
			if(!m_GalaxySpriteTex)
				m_GalaxySpriteTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SunGlow", ":data/txSunGlow.bmp",
						GL_LINEAR, GL_LINEAR, false, 2);

			if(m_SpotLocs.empty())
			{
				if(m_GalaxyVAO.VAO())
					m_GalaxyVAO.DeleteGL();

				// Update spot locations cache.
				int density = 250;	// Time to make the scene goes up geometrically with the increase of this value.
				float blendFact = density / 100.f;
				if(blendFact < 1)
					blendFact = 1;
				else
					blendFact *= blendFact;
				blendFact *= 9;				// Bigger is fainter
				float spriteSize = 5000;
				DaGalaxy galaxy(Galaxy());							// For star density calculations.
				AuDice dice;										// For consistent random noise.
				int pixelStep = galaxy.Radius()/density;
				for(int ix=-galaxy.Radius(); ix<=galaxy.Radius(); ix+=pixelStep)
					for(int iy=-galaxy.Radius(); iy<=galaxy.Radius(); iy+=pixelStep)
						for(int iz=-galaxy.Radius()/10; iz<=galaxy.Radius()/10; iz+=pixelStep)
						{
							// Apply some noise to the sector location so we don't make a perfectly regular grid of dots.
							AuPoint sectorLoc(ix + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)),
									iy + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)),
									iz + dice.Roll(1,pixelStep*2+1,-(pixelStep+1)));

							// Determine color of spot.
							AuGalaxySpot spot;
							spot.m_Color = galaxy.Color(sectorLoc);
							if(spot.m_Color.alphaF() == 0)
								continue;

							// Reduce blend of spots when there are lots of them.
							spot.m_Color.setAlphaF(spot.m_Color.alphaF() / blendFact);

							// Append non-zero sector location to my spot list.
							AuScPoint sl(AuPointTexN(sectorLoc,0,0,AuPoint(0,0,1)), 0, 0, spriteSize, spot.m_Color);
							m_SpotLocs.append(sl);
						}
				//qDebug() << "spotLocs" << m_SpotLocs.size();
			}

			if(!m_GalaxyVAO.VAO() && !m_SpotLocs.empty())
				m_GalaxyVAO.CreateForSpriteSizeColor(*m_RenderInfo->re, m_SpotLocs, GL_STATIC_DRAW);
		}

		// Render
		if(m_AxesVAO.VAO())
		{
			// Use line shader so lines will be super bright, even at night.
			m_RenderInfo->re->GLSL().LineShader().Select();
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
			m_RenderInfo->re->GLSL().LineShader().SetColor(s_AxisColorDim);

			// Display the VAO once.
			m_AxesVAO.SelectGL();
			m_AxesVAO.DrawGL(GL_LINES);
			m_AxesVAO.DeselectGL();

			// Restore rendering states.
			m_RenderInfo->re->GLSL().LineShader().Deselect();

			// Label axes.
			m_RenderInfo->RenderText(p, mv, verts[0], tr("W"), s_AxisColor);
			m_RenderInfo->RenderText(p, mv, verts[1], tr("E"), s_AxisColor);
			m_RenderInfo->RenderText(p, mv, verts[2], tr("S"), s_AxisColor);
			m_RenderInfo->RenderText(p, mv, verts[3], tr("N"), s_AxisColor);
		}

		if(m_ShowGalaxy && m_GalaxyVAO.VAO() && m_GalaxySpriteTex)
		{
			m_RenderInfo->re->GLSL().SpriteSizeColorShader().Select();
			m_RenderInfo->re->GLSL().SpriteSizeColorShader().SetProjectionModelView(mvp);
			m_RenderInfo->re->GLSL().SpriteSizeColorShader().SetTexture(m_GalaxySpriteTex);

			m_RenderInfo->re->glDepthMask(GL_FALSE);				// This way the particles do not have to be sorted
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
			m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);

			// Display the VAO once.
			m_GalaxyVAO.SelectGL();
			m_GalaxyVAO.DrawGL(GL_POINTS);
			m_GalaxyVAO.DeselectGL();

			m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
			m_RenderInfo->re->glDepthMask(GL_TRUE);
			m_RenderInfo->re->glDisable(GL_BLEND);

			m_RenderInfo->re->GLSL().SpriteSizeColorShader().Deselect();
		}
	}

	// Render the good stuff.
	{
		// Acquire current galaxy location of scene atom.
		AuView* pView = auApp->Frame()->View();
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
		{
			m_InGalaxy = false;
			const DaGalaxy* galaxy = person->FindParentGalaxy();
			if(galaxy && (galaxy->Galaxy() == Galaxy() || Galaxy() == GALAXY_Count))
			{
				if(Galaxy() != galaxy->Galaxy())
				{
					m_Galaxy = galaxy->Galaxy();
					m_SpotLocs.clear();
				}

				person->GetLocationRelativeToParent(m_SceneAtomLoc, galaxy);
				m_InGalaxy = true;
			}
			const DaSector* sector = person->FindParentSector();
			if(sector)
				m_SceneAtomSectorLoc = sector->Location();	// in pc
		}
		pView->UnlockSceneAtom();

		// When filtering by range, make a list of sectors that are going to be displayed.
		m_SectorIds.clear();
		if(m_FilterRange)
		{
			for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin();
					pi != m_Places.end(); pi++)
			{
				if((*pi).second.Galaxy() != Galaxy())
					continue;	// Not in the current galaxy.
				if((*pi).second.m_AtomType != DaAtom::ATOM_Sector)
					continue;	// Not a sector.
				double secrange = m_SceneAtomSectorLoc.Distance((*pi).second.m_Loc);	// in pc
				secrange /= 10;		// in sectors
				if((int)floor(secrange) > m_Range)
					continue;
				m_SectorIds.append((*pi).first);
			}
		}

		// Show all blended backgrounds of places, namely the ownership bubble around solar systems.
		QList<AuScPoint> syslocs;				// Solar system locations, size and color.
		{
			AuPoint eyeLoc;
			for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin();
					pi != m_Places.end(); pi++)
			{
				if((*pi).second.Galaxy() != Galaxy())
					continue;

				if(m_CountSelected && m_FilterSelection)
				{
					// There are selected items and we are filtering only on selected items.
					bool skipIt = !(*pi).second.m_Selected;
					if(skipIt && !(*pi).second.m_SectorId.IsNull())
					{
						// The place is not selected and it's not a sector.
						// Don't skip if the parent sector is selected.
						unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi =
							m_Places.find((*pi).second.m_SectorId);
						if(spi != m_Places.end() && (*spi).second.m_Selected)
							skipIt = false;								// Parent sector is selected.
					}
					if(skipIt)
						continue;
				}

				float spriteSize = 1000;
				switch((*pi).second.m_AtomType)
				{
				case DaAtom::ATOM_SolarSystem:
					if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
						continue;	// Filtering by range and the sector is out of range.
					// Draw a spot at the system location.
					if(((*pi).second.m_SystemExplored || !m_FilterExplored) && (*pi).second.m_Color != s_BlackColor)
					{
						if((*pi).second.IsEmptySystem())
						{
							AuScPoint sl(AuPointTexN((*pi).second.m_Loc,0,0,AuPoint(0,0,1)), 0, 0, spriteSize*2,
									(*pi).second.m_Color);
							syslocs.append(sl);
						}
						else
						{
							AuScPoint sl(AuPointTexN((*pi).second.m_Loc,0,0,AuPoint(0,0,1)), 0, 0, spriteSize,
									(*pi).second.m_Color);
							syslocs.append(sl);
						}
					}
					break;
				default:
					break;
				}
			}

			if(!syslocs.empty())
			{
				if(!m_SystemSpriteTex)
					m_SystemSpriteTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "LOmni",
							":data/txLightOmni.png");

				AuGLVAO vao;
				vao.CreateForSpriteSizeColor(*m_RenderInfo->re, syslocs, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					m_RenderInfo->re->GLSL().SpriteSizeColorShader().Select();
					m_RenderInfo->re->GLSL().SpriteSizeColorShader().SetProjectionModelView(mvp);
					m_RenderInfo->re->GLSL().SpriteSizeColorShader().SetTexture(m_SystemSpriteTex);

					m_RenderInfo->re->glDepthMask(GL_FALSE);				// This way the particles do not have to be sorted
					m_RenderInfo->re->glEnable(GL_BLEND);
					m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);

					// Display the VAO once and clean it up.
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();

					m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
					m_RenderInfo->re->glDepthMask(GL_TRUE);
					m_RenderInfo->re->glDisable(GL_BLEND);

					m_RenderInfo->re->GLSL().SpriteSizeColorShader().Deselect();
				}
			}
		}

		// Show all places.
		QList<AuPointAlpha> starVerts;
		QList<AuPointAlpha> scannedVerts;
		AuPointArray wormExSegs;		// Pairs of locations at explored wormhole start and end points.
		AuPointArray wormUnexSegs;		// Pairs of locations at unexplored wormhole start and end points.
		QList<AuPointAlpha> shipsM, shipsF, shipsN, shipsE;
		QList<AuPointAlpha> stationsM, stationsF, stationsN, stationsE;
		for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
		{
			if((*pi).second.Galaxy() != Galaxy() && (*pi).second.m_AtomType != DaAtom::ATOM_Avatar)
				continue;

			if(m_CountSelected && m_FilterSelection)
			{
				// There are selected items and we are filtering only on selected items.
				bool skipIt = !(*pi).second.m_Selected;
				if(skipIt && !(*pi).second.m_SectorId.IsNull())
				{
					// The place is not selected and it's not a sector.
					// Don't skip if the parent sector is selected.
					unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi =m_Places.find((*pi).second.m_SectorId);
					if(spi != m_Places.end() && (*spi).second.m_Selected)
						skipIt = false;								// Parent sector is selected.
				}
				if(skipIt)
					continue;
			}

			switch((*pi).second.m_AtomType)
			{
			case DaAtom::ATOM_Sector:
				if(m_FilterRange && !m_SectorIds.contains((*pi).first))
					continue;	// Filtering by range and the sector is out of range.
				// Draw a box around the sector.
				if((*pi).second.m_Selected || (*pi).first == m_Preview)
				{
					// Calculate corner points of sector box.
					AuPoint llb = (*pi).second.m_Loc + AuPoint(-5,-5,-5);
					AuPoint ulb = (*pi).second.m_Loc + AuPoint(-5,5,-5);
					AuPoint urb = (*pi).second.m_Loc + AuPoint(5,5,-5);
					AuPoint lrb = (*pi).second.m_Loc + AuPoint(5,-5,-5);
					AuPoint llt = (*pi).second.m_Loc + AuPoint(-5,-5,5);
					AuPoint ult = (*pi).second.m_Loc + AuPoint(-5,5,5);
					AuPoint urt = (*pi).second.m_Loc + AuPoint(5,5,5);
					AuPoint lrt = (*pi).second.m_Loc + AuPoint(5,-5,5);
					AuPointArray segs;
					//Bottom
					segs.append(llb);
					segs.append(ulb);
					segs.append(ulb);
					segs.append(urb);
					segs.append(urb);
					segs.append(lrb);
					segs.append(lrb);
					segs.append(llb);
					//Top
					segs.append(llt);
					segs.append(ult);
					segs.append(ult);
					segs.append(urt);
					segs.append(urt);
					segs.append(lrt);
					segs.append(lrt);
					segs.append(llt);
					//Legs
					segs.append(llb);
					segs.append(llt);
					segs.append(ulb);
					segs.append(ult);
					segs.append(urb);
					segs.append(urt);
					segs.append(lrb);
					segs.append(lrt);

					m_RenderInfo->re->GLSL().LineShader().Select();
					m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);

					AuGLVAO vaoLines;
					vaoLines.CreateForLines(*m_RenderInfo->re, segs, false, GL_STREAM_DRAW);
					if(vaoLines.VAO())
					{
						if((*pi).second.m_Color != s_BlackColor)
							m_RenderInfo->re->GLSL().LineShader().SetColor((*pi).second.m_Color.redF(),
									(*pi).second.m_Color.greenF(), (*pi).second.m_Color.blueF(), .5f);
						else
							m_RenderInfo->re->GLSL().LineShader().SetColor(.25f,.25f,.25f, .5f);

						vaoLines.SelectGL();
						vaoLines.DrawGL(GL_LINES);
						vaoLines.DeselectGL();
						vaoLines.DeleteGL();

					}

					// Text is rendered using the lines shader.
					QColor col;
					if((*pi).second.m_Color != s_BlackColor)
					{
						col = (*pi).second.m_Color;
						float hue = col.hueF();
						float sat = col.saturationF();
						float val = col.valueF();
						val += .1f;
						if(val > 1)
							val = 1;
						col.setHsvF(hue, sat, val);
					}
					else
						col = QColor(128,128,128);
					m_RenderInfo->re->glEnable(GL_BLEND);
					m_RenderInfo->re->glDepthMask(GL_FALSE);
					m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					m_RenderInfo->re->RenderText(p, mv, (*pi).second.m_Loc, (*pi).second.m_Name, m_FontMedium, col,
							m_RenderInfo->glw->height(), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
					m_RenderInfo->re->glDepthMask(GL_TRUE);
					m_RenderInfo->re->glDisable(GL_BLEND);

					m_RenderInfo->re->GLSL().LineShader().Deselect();
				}
				break;
			case DaAtom::ATOM_SolarSystem:
				if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
					continue;	// Filtering by range and the sector is out of range.
				// Draw a pixel at the system location.
				if((*pi).second.m_SystemExplored || !m_FilterExplored)
				{
					if(m_ShowWormholes)
					{
						for(QList<AuPoint>::const_iterator wi = (*pi).second.m_WormDestEx.cbegin();
								wi != (*pi).second.m_WormDestEx.cend(); wi++)
						{
							wormExSegs.append((*pi).second.m_Loc);
							wormExSegs.append(*wi);
						}

						for(QList<AuPoint>::const_iterator wi = (*pi).second.m_WormDestUnex.cbegin();
								wi != (*pi).second.m_WormDestUnex.cend(); wi++)
						{
							wormUnexSegs.append((*pi).second.m_Loc);
							wormUnexSegs.append(*wi);
						}
					}

					// Star dot
					if(!(*pi).second.IsEmptySystem())
					{
						starVerts.append((*pi).second.m_Loc);

						// Scanned indicator
						if(m_ShowScanned && (*pi).second.IsScanned())
							scannedVerts.append((*pi).second.m_Loc);
					}

					// Signal strength
					if(m_ShowSignals && (*pi).second.m_SignalStrength && (*pi).second.m_Color != s_BlackColor)
					{
						if(!m_SignalTex[0])
						{
							for(int i=0; i<16; i++)
							{
								QString sid = QString("Sig%1").arg(i);
								QString file = QString(":data/Signal_%1.png").arg(i);
								m_SignalTex[i] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, file,
										GL_LINEAR, GL_LINEAR, true, 2);
							}
						}

						m_RenderInfo->re->GLSL().SpriteSizeShader().Select();
						m_RenderInfo->re->GLSL().SpriteSizeShader().SetProjectionModelView(mvp);
						m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_SignalTex[(*pi).second.m_SignalStrength]);
						m_RenderInfo->re->GLSL().SpriteSizeShader().SetColor(1,1,1,1);
						m_RenderInfo->re->GLSL().SpriteSizeShader().SetSize(150);

						m_RenderInfo->re->glDepthMask(GL_FALSE);				// This way the particles do not have to be sorted
						m_RenderInfo->re->glEnable(GL_BLEND);
						m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);

						// Make a VAO of the vertex location.
						QList<AuPointAlpha> verts;
						verts << (*pi).second.m_Loc;
						AuGLVAO vao;
						vao.CreateForSprites(*m_RenderInfo->re, verts, GL_STREAM_DRAW);

						// Display the VAO once and clean it up.
						if(vao.VAO())
						{
							vao.SelectGL();
							vao.DrawGL(GL_POINTS);
							vao.DeselectGL();
							vao.DeleteGL();
						}

						m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
						m_RenderInfo->re->glDepthMask(GL_TRUE);
						m_RenderInfo->re->glDisable(GL_BLEND);

						m_RenderInfo->re->GLSL().SpriteSizeShader().Deselect();
					}

					// Spacecraft
					{
						if(m_ShowStations && (*pi).second.HasStation())
						{
							// Draw space station indicators.
							AuPoint iconLoc((*pi).second.m_Loc);
							double spacing = .5;
							iconLoc.x += spacing;
							if((*pi).second.m_EmpireStations)
							{
								iconLoc.y -= spacing;
								stationsM.append(iconLoc);

								int stations = (*pi).second.m_EmpireStations;
								if(stations > 1)
								{
									QString n = QString("%L1").arg(stations);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_FriendStations)
							{
								iconLoc.y -= spacing;
								stationsF.append(iconLoc);

								int stations = (*pi).second.m_FriendStations;
								if(stations > 1)
								{
									QString n = QString("%L1").arg(stations);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_NeutralStations)
							{
								iconLoc.y -= spacing;
								stationsN.append(iconLoc);

								int stations = (*pi).second.m_NeutralStations;
								if(stations > 1)
								{
									QString n = QString("%L1").arg(stations);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_EnemyStations)
							{
								iconLoc.y -= spacing;
								stationsE.append(iconLoc);

								int stations = (*pi).second.m_EnemyStations;
								if(stations > 1)
								{
									QString n = QString("%L1").arg(stations);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
						}

						if(m_ShowShips && (*pi).second.HasShip())
						{
							// Draw space ship indicators.
							AuPoint iconLoc((*pi).second.m_Loc);
							double spacing = .5;
							iconLoc.x += spacing;
							if((*pi).second.m_EmpireShips)
							{
								iconLoc.y += spacing;
								shipsM.append(iconLoc);

								int ships = (*pi).second.m_EmpireShips;
								if(ships > 1)
								{
									QString n = QString("%L1").arg(ships);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_FriendShips)
							{
								iconLoc.y += spacing;
								shipsF.append(iconLoc);

								int ships = (*pi).second.m_FriendShips;
								if(ships > 1)
								{
									QString n = QString("%L1").arg(ships);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_NeutralShips)
							{
								iconLoc.y += spacing;
								shipsN.append(iconLoc);

								int ships = (*pi).second.m_NeutralShips;
								if(ships > 1)
								{
									QString n = QString("%L1").arg(ships);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
							if((*pi).second.m_EnemyShips)
							{
								iconLoc.y += spacing;
								shipsE.append(iconLoc);

								int ships = (*pi).second.m_EnemyShips;
								if(ships > 1)
								{
									QString n = QString("%L1").arg(ships);

									// Text is rendered using the lines shader.
									m_RenderInfo->re->GLSL().LineShader().Select();
									m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
									m_RenderInfo->re->glEnable(GL_BLEND);
									m_RenderInfo->re->glDepthMask(GL_FALSE);
									m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
									m_RenderInfo->re->RenderText(p, mv, iconLoc + AuPoint(spacing,0), n, m_FontMedium,
											PoliticalStanceColor(POL_Empire, false), m_RenderInfo->glw->height(),
											Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine);
									m_RenderInfo->re->glDepthMask(GL_TRUE);
									m_RenderInfo->re->glDisable(GL_BLEND);
									m_RenderInfo->re->GLSL().LineShader().Deselect();
								}
							}
						}
					}

					// Selected/hover extra info.
					if(((*pi).second.m_Selected || (*pi).first == m_Preview) && !(*pi).second.IsEmptySystem())
					{
						QColor col;
						if((*pi).second.m_Color != s_BlackColor)
						{
							col = (*pi).second.m_Color;
							float hue = col.hueF();
							float sat = col.saturationF();
							float val = col.valueF();
							val += .1f;
							if(val > 1)
								val = 1;
							col.setHsvF(hue, sat, val);
						}
						else
							col = QColor(128,128,128);

						// Show the system name.
						AuPoint nameLoc((*pi).second.m_Loc + AuPoint(.1,0,0));
						nameLoc = mvp.Project(nameLoc);
						QPoint px = ViewportToPixel(m_RenderInfo->glw, QPointF(nameLoc.x, nameLoc.y));
						px.setX(px.x() + 10);

						m_RenderInfo->re->glEnable(GL_BLEND);
						m_RenderInfo->re->glDepthMask(GL_FALSE);
						m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						m_RenderInfo->re->RenderText(px.x(), px.y(), (*pi).second.m_Name, m_FontBig, col,
								m_RenderInfo->glw->width(), m_RenderInfo->glw->height());

						bool cities = false;
						double spacing = 17;	// So 16 pixel flags are 1 pixel apart.
						if((*pi).first == m_Preview)
						{
							// Preview system shows more information.
							double distToLookPt = (*pi).second.m_Loc.Distance(m_LookPoint);
							QString gloc;
							if(distToLookPt < .1)
								gloc = QString("(%2, %3, %4)")
									.arg((*pi).second.m_Loc.x)
									.arg((*pi).second.m_Loc.y)
									.arg((*pi).second.m_Loc.z);
							else
								gloc = QString("(%2, %3, %4) %L5pc")
									.arg((*pi).second.m_Loc.x)
									.arg((*pi).second.m_Loc.y)
									.arg((*pi).second.m_Loc.z)
									.arg(distToLookPt, 0,'f',2);
							px.setY(px.y() + spacing);
							m_RenderInfo->re->RenderText(px.x(), px.y(), gloc, m_FontMedium, col,
									m_RenderInfo->glw->width(), m_RenderInfo->glw->height());

							// List cities under the system name.
							QPoint pxc;
							pxc.setX(px.x() + spacing);
							pxc.setY(px.y());
							for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.begin();
									spi != m_Places.end(); spi++)
							{
								if((*spi).second.m_AtomType != DaAtom::ATOM_Bd)
									continue;
								if((*spi).second.m_Loc != (*pi).second.m_Loc)
									continue;
								cities = true;
								pxc.setY(pxc.y() + spacing);
								QColor col;
								if((*spi).second.m_Color != s_BlackColor)
								{
									col = (*pi).second.m_Color;
									float hue = col.hueF();
									float sat = col.saturationF();
									float val = col.valueF();
									val += .1f;
									if(val > 1)
										val = 1;
									col.setHsvF(hue, sat, val);
								}
								else
									col = QColor(128,128,128);
								m_RenderInfo->re->RenderText(pxc.x(), pxc.y(), (*spi).second.m_Name, m_FontSmall, col,
										m_RenderInfo->glw->width(), m_RenderInfo->glw->height());
							}
						}
						m_RenderInfo->re->glDisable(GL_BLEND);

						// Empire flag icons next to cities.
						if(cities)
						{
							QPoint pxc(px);
							QRect rcWindow(m_RenderInfo->glw->rect());
							double windowAspect = AuGLEffects::WindowAspectRatio(rcWindow);
							float size = 16.f / rcWindow.height() * 2;
							for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.begin();
									spi != m_Places.end(); spi++)
							{
								if((*spi).second.m_AtomType != DaAtom::ATOM_Bd)
									continue;
								if((*spi).second.m_Loc != (*pi).second.m_Loc)
									continue;

								QImage texImg;
								const DgData& gd = g_Au.LockDataForRead();
								gd.EmpireFlag((*spi).second.m_CityEmpire, texImg);
								g_Au.UnlockData();
								if(texImg.isNull())
									continue;
								float flagAspect = (float)texImg.width() / (float)texImg.height();

								QString stid = QString("Flag:%1").arg((*spi).second.m_CityEmpire);
								GLuint flagTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, stid, texImg,
										GL_LINEAR, GL_LINEAR, false, 1);
								if(!flagTex)
									continue;

								pxc.setY(pxc.y() + spacing);
								QPointF vpx = PixelToViewport(m_RenderInfo->glw, pxc);
								vpx.setX(vpx.x() / windowAspect);
								vpx.setY(vpx.y() + size/2);

								m_RenderInfo->re->glEnable(GL_BLEND);
								m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
								m_RenderInfo->re->RenderSprite(rcWindow, vpx.x(), vpx.y(), size*flagAspect,size, flagTex, 1,1,1,1);
								m_RenderInfo->re->glDisable(GL_BLEND);
							}
						}
					}
				}
				break;
			case DaAtom::ATOM_Avatar:
				if(m_InGalaxy)
				{
					// This is the scene atom. Update their location.
					(*pi).second.m_Loc = m_SceneAtomLoc;
					if((*pi).second.m_Selected)
					{
						// Text is rendered using the lines shader.
						m_RenderInfo->re->GLSL().LineShader().Select();
						m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);
						m_RenderInfo->re->glEnable(GL_BLEND);
						m_RenderInfo->re->glDepthMask(GL_FALSE);
						m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						m_RenderInfo->re->RenderText(p, mv, (*pi).second.m_Loc, QString("   ") + (*pi).second.m_Name, m_FontSmall,
								(*pi).second.m_Color, m_RenderInfo->glw->height(),
								Qt::AlignLeft|Qt::AlignBottom|Qt::TextSingleLine);
						m_RenderInfo->re->glDepthMask(GL_TRUE);
						m_RenderInfo->re->glDisable(GL_BLEND);
						m_RenderInfo->re->GLSL().LineShader().Deselect();
					}
				}
				break;
			default:
				break;
			}
		}

		if(!shipsM.empty() || !shipsF.empty() || !shipsN.empty() || !shipsE.empty() ||
				!stationsM.empty() || !stationsF.empty() || !stationsN.empty() || !stationsE.empty())
		{
			// Draw space ship indicators.
			if(!m_ShipTex[0])
			{
				m_ShipTex[0] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "MShip",
						":data/clMShip.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_ShipTex[1] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FShip",
						":data/clFShip.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_ShipTex[2] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NShip",
						":data/clNShip.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_ShipTex[3] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EShip",
						":data/clEShip.png", GL_LINEAR, GL_LINEAR, true, 2);

				m_StationTex[0] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "MStation",
						":data/clMStation.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_StationTex[1] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "FStation",
						":data/clFStation.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_StationTex[2] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NStation",
						":data/clNStation.png", GL_LINEAR, GL_LINEAR, true, 2);
				m_StationTex[3] = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "EStation",
						":data/clEStation.png", GL_LINEAR, GL_LINEAR, true, 2);
			}

			m_RenderInfo->re->GLSL().SpriteSizeShader().Select();
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetProjectionModelView(mvp);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetColor(1,1,1,1);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetSize(80);
			m_RenderInfo->re->glDepthMask(GL_FALSE);
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);
			m_RenderInfo->re->glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

			// Ships
			if(!shipsM.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_ShipTex[0]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, shipsM, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!shipsF.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_ShipTex[1]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, shipsF, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!shipsN.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_ShipTex[2]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, shipsN, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!shipsE.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_ShipTex[3]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, shipsE, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}

			// Stations
			if(!stationsM.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_StationTex[0]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, stationsM, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!stationsF.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_StationTex[1]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, stationsF, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!stationsN.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_StationTex[2]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, stationsN, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}
			if(!stationsE.empty())
			{
				m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_StationTex[3]);

				// Make a VAO of the vertex locations.
				AuGLVAO vao;
				vao.CreateForSprites(*m_RenderInfo->re, stationsE, GL_STREAM_DRAW);

				// Display the VAO once and clean it up.
				if(vao.VAO())
				{
					vao.SelectGL();
					vao.DrawGL(GL_POINTS);
					vao.DeselectGL();
					vao.DeleteGL();
				}
			}

			m_RenderInfo->re->glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
			m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
			m_RenderInfo->re->glDepthMask(GL_TRUE);
			m_RenderInfo->re->glDisable(GL_BLEND);
			m_RenderInfo->re->GLSL().SpriteSizeShader().Deselect();
		}

		// Wormhole Lines
		if(!wormExSegs.empty() || !wormUnexSegs.empty())
		{
			m_RenderInfo->re->GLSL().LineShader().Select();
			m_RenderInfo->re->GLSL().LineShader().SetProjectionModelView(mvp);

			if(!wormExSegs.empty())
			{
				AuGLVAO vao;
				vao.CreateForLines(*m_RenderInfo->re, wormExSegs, false, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					QColor col(192, 64, 32, 200);
					m_RenderInfo->re->GLSL().LineShader().SetColor(col);
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();

				}
			}

			if(!wormUnexSegs.empty())
			{
				AuGLVAO vao;
				vao.CreateForLines(*m_RenderInfo->re, wormUnexSegs, false, GL_STREAM_DRAW);
				if(vao.VAO())
				{
					QColor col(32, 64, 192, 200);
					m_RenderInfo->re->GLSL().LineShader().SetColor(col);
					vao.SelectGL();
					vao.DrawGL(GL_LINES);
					vao.DeselectGL();
					vao.DeleteGL();

				}
			}

			m_RenderInfo->re->GLSL().LineShader().Deselect();
		}

		if(!starVerts.empty())
		{
			if(!m_StarSpriteTex)
				m_StarSpriteTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "StarSprite", ":data/Star.png",
						GL_LINEAR, GL_LINEAR, true, 2);

			m_RenderInfo->re->GLSL().SpriteSizeShader().Select();
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetProjectionModelView(mvp);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_StarSpriteTex);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetColor(1,1,1,1);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetSize(50);

			m_RenderInfo->re->glDepthMask(GL_FALSE);				// This way the particles do not have to be sorted
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(*m_RenderInfo->re, starVerts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
			m_RenderInfo->re->glDepthMask(GL_TRUE);
			m_RenderInfo->re->glDisable(GL_BLEND);

			m_RenderInfo->re->GLSL().SpriteSizeShader().Deselect();
		}

		if(!scannedVerts.empty())
		{
			if(!m_ScannedSpriteTex)
				m_ScannedSpriteTex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SpotScanned",
						":data/SpotScanned.png", GL_LINEAR, GL_LINEAR, true, 2);

			m_RenderInfo->re->GLSL().SpriteSizeShader().Select();
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetProjectionModelView(mvp);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetTexture(m_ScannedSpriteTex);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetColor(1,1,1,1);
			m_RenderInfo->re->GLSL().SpriteSizeShader().SetSize(150);

			m_RenderInfo->re->glDepthMask(GL_FALSE);				// This way the particles do not have to be sorted
			m_RenderInfo->re->glEnable(GL_BLEND);
			m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			m_RenderInfo->re->glEnable(GL_PROGRAM_POINT_SIZE);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForSprites(*m_RenderInfo->re, scannedVerts, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			if(vao.VAO())
			{
				vao.SelectGL();
				vao.DrawGL(GL_POINTS);
				vao.DeselectGL();
				vao.DeleteGL();
			}

			m_RenderInfo->re->glDisable(GL_PROGRAM_POINT_SIZE);
			m_RenderInfo->re->glDepthMask(GL_TRUE);
			m_RenderInfo->re->glDisable(GL_BLEND);

			m_RenderInfo->re->GLSL().SpriteSizeShader().Deselect();
		}

		// Show where I am.
		if(m_InGalaxy)
		{
			AuPoint eyeLoc = mv.Project(m_SceneAtomLoc);
			if(eyeLoc.z < 0)
			{
				if(!m_MyLoc1Tex)
				{
					m_MyLoc1Tex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "MyLoc1",
							":data/MyLocBright.png", GL_LINEAR, GL_LINEAR, true, 2);
					m_MyLoc2Tex = m_RenderInfo->re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "MyLoc2",
							":data/MyLocDim.png", GL_LINEAR, GL_LINEAR, true, 2);
				}

				GLuint tex;
				if(AuTime::CurrentSecIsOdd())
					tex = m_MyLoc1Tex;
				else
					tex = m_MyLoc2Tex;

				float size = .1f;

				QRect rcWindow = m_RenderInfo->glw->rect();
				double windowAspect = AuGLEffects::WindowAspectRatio(rcWindow);
				AuPoint pxloc = p.Project(eyeLoc);
				pxloc.x /= windowAspect;
				m_RenderInfo->re->glEnable(GL_BLEND);
				m_RenderInfo->re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				m_RenderInfo->re->RenderSprite(rcWindow, pxloc.x, pxloc.y, size,size, tex, 1,1,1,1);
				m_RenderInfo->re->glDisable(GL_BLEND);
			}
		}

		// Magnification control
		/*DHH
		{
			// Pick up the dimensions of the magnifier button.
			QImage magnifier(":data/Magnifier.png");
			m_Magnifier = QRect(0,height()/2-magnifier.height()/2,magnifier.width(),magnifier.height());
			p.setTransform(QTransform());	// Reset to identity matrix.
			p.drawImage(m_Magnifier, magnifier);
		}
		*/
	}

	update();
}

void AuStarMapGL::wheelEvent(QWheelEvent *e)
{
	double moveDist;
	if(e->modifiers() & Qt::ShiftModifier)
		moveDist = 10;
	else
		moveDist = 1000;

	float oldLookDist = m_LookDist;
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() > 0)
	{
		// Moving closer.
		if(m_LookDist > moveDist)
			m_LookDist -= moveDist;
		else
			m_LookDist /= 2;
	}
	else if(e->angleDelta().y() < 0)
	{
		// Moving farther away.
		if(m_LookDist > moveDist || m_LookDist*2 > moveDist)
			m_LookDist += moveDist;
		else
			m_LookDist *= 2;
	}
#else
	if(e->delta() > 0)
	{
		// Moving closer.
		if(m_LookDist > moveDist)
			m_LookDist -= moveDist;
		else
			m_LookDist /= 2;
	}
	else if(e->delta() < 0)
	{
		// Moving farther away.
		if(m_LookDist > moveDist || m_LookDist*2 > moveDist)
			m_LookDist += moveDist;
		else
			m_LookDist *= 2;
	}
#endif

	if(m_LookDist > 15000)
		m_LookDist = 15000;
	else if(m_LookDist < 1)
		m_LookDist = 1;

	QOpenGLWidget::wheelEvent(e);
	
	if(m_LookDist != oldLookDist)
	{
		if(isVisible())
			update();
	}
}

void AuStarMapGL::mousePressEvent(QMouseEvent* e)
{
	QOpenGLWidget::mousePressEvent(e);

	// Capture information about the closest atom to the mouse press event. This is the target atom.
	m_Target = m_Preview;				// Atom that is at the mouse press location.
	m_TargetLoc = m_PreviewLoc;			// Galaxy coordinate of target atom
	m_TargetName = m_PreviewName;		// Target atom name.
	m_TargetType = m_PreviewType;		// AtomType of preview atom.
	m_TargetGalaxy = m_PreviewGalaxy;
	m_TargetGenHab = m_PreviewGenHab;
	m_TargetGenSeed = m_PreviewGenSeed;
	m_TargetHasSpacecraft = m_PreviewHasSpacecraft;
	m_TargetCityEmpire = m_PreviewCityEmpire;
	m_TargetIsCoreBlackHole = m_PreviewIsCoreBlackHole;
	m_TargetIsEmptySystem = m_PreviewIsEmptySystem;
	m_TargetIsExplored = m_PreviewIsExplored;
	m_TargetIsScanned = m_PreviewIsScanned;
	m_TargetSector = m_PreviewSector;

	m_LeftButtonMode = 0;
	m_MouseDownPt = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		if(m_Magnifier.contains(e->pos()))
		{
			setCursor(QCursor(Qt::SizeVerCursor));
			m_LeftButtonMode = 2;								// zoom
			m_MagnifierScale = m_LookDist;						// Capture current scale before we start messing with it.
		}
		else
		{
			setCursor(QCursor(Qt::ClosedHandCursor));
			m_LeftButtonMode = 1;								// rotate
		}
	}
}

// NOTE: We have enabled mouse tracking so this gets called whenever the mouse moves, even if no button is depressed.
void AuStarMapGL::mouseMoveEvent(QMouseEvent* e)
{
	QOpenGLWidget::mouseMoveEvent(e);
	if(m_LeftButtonMode == 0)
	{
		// Show nearby object.
		PreviewClosestPlace(e->pos());
		//qDebug() << "AuStarMapGL::mouseMoveEvent preview" << m_Preview << m_PreviewName
			//<< "target" << m_Target << m_TargetName;
	}
	else
	{
		m_Preview = AuId(0);									// Turn off preview while mouse is down.
		m_Target = AuId(0);										// Do nothing to any target items when dragging view.
		//qDebug() << "AuStarMapGL::mouseMoveEvent preview cleared" << "target" << m_Target << m_TargetName;
		if(m_LeftButtonMode == 1)
		{
			// Rotate view around look point.
			int dx = e->pos().x() - m_MouseDownPt.x();
			int dy = m_MouseDownPt.y() - e->pos().y();
			//qDebug() << "dx" << dx << "dy" << dy;
			m_MouseDownPt = e->pos();

			AuMatrix4x4 t;
			t.RotateY(DEG2RAD*dx);
			t.RotateX(-DEG2RAD*dy);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();

			if(isVisible())
				update();
		}
		else if(m_LeftButtonMode == 2)
		{
			// Zoom closer/farther from look point.
			float oldLookDist = m_LookDist;

			int dy = m_MouseDownPt.y() - e->pos().y();
			//m_Scale += .01f * dy;
			if(dy > 0)
			{
				// Zooming in
				if(m_LookDist > 1)
				{
					// Max zoom is at top of widget.
					// Interpolate between current zoom at center and max at top.
					float sclRange = 10.f - m_MagnifierScale;
					float sclPerPixel = sclRange / m_MouseDownPt.y();
					m_LookDist = m_MagnifierScale + sclPerPixel * dy;
				}
			}
			else
			{
				// Zooming out
				if(m_LookDist < 15000)
				{
					// Min zoom is at bottom of widget.
					// Interpolate between current zoom at center and min at bottom.
					float sclRange = 15000.f - m_MagnifierScale;
					float sclPerPixel = sclRange / (height() - m_MouseDownPt.y());
					m_LookDist = m_MagnifierScale - sclPerPixel * dy;
				}
			}

			// Restrict range of scale.
			if(m_LookDist > 15000)
				m_LookDist = 15000;
			else if(m_LookDist < 1)
				m_LookDist = 1;
			
			if(m_LookDist != oldLookDist)
			{
				if(isVisible())
					update();
			}
		}
	}
}

void AuStarMapGL::mouseDoubleClickEvent(QMouseEvent* e)
{
	QOpenGLWidget::mouseDoubleClickEvent(e);
	if(e->button() == Qt::LeftButton)
	{
		// Ignore double-click in the magnifier.
		if(m_Magnifier.contains(e->pos()))
			return;

		// Center view on preview atom, which is the closest atom to the cursor.
		if(!m_Target.IsNull())
		{
			emit PlaceDoubleClicked(m_Target);
			m_Target = AuId(0);		// Prevent the mouse release event from sending a clicked signal on this guy.
		}
	}
}

void AuStarMapGL::PreviewClosestPlace(const QPoint& pos)
{
	// Calculate the combined model view projection matrix.
	AuMatrix4x4 mv, mvp;
	GetMVP(mv, mvp);

	AuPoint curpos(pos);
	m_Preview = AuId(0);
	double closestDist2 = -1;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.Galaxy() != Galaxy())
			continue;

		if(m_CountSelected && m_FilterSelection)
		{
			// There are selected items and we are filtering only on selected items.
			bool skipIt = !(*pi).second.m_Selected;
			if(skipIt && !(*pi).second.m_SectorId.IsNull())
			{
				// The place is not selected and it's not a sector.
				// Don't skip if the parent sector is selected.
				unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator spi = m_Places.find((*pi).second.m_SectorId);
				if(spi != m_Places.end() && (*spi).second.m_Selected)
					skipIt = false;								// Parent sector is selected.
			}
			if(skipIt)
				continue;
		}

		switch((*pi).second.m_AtomType)
		{
		case DaAtom::ATOM_Sector:
			if(!m_FilterRange || m_SectorIds.contains((*pi).first))
			{
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < -120)
				{
					// Sector location is in front of the eye plane and farther than the distance where we see its stars.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					double dist2 = pxloc.Distance2_2D(curpos);
					if(dist2 > 20*20)
						continue;	// more than 100 pixels away
					if(closestDist2 < 0 || dist2 < closestDist2)
					{
						closestDist2 = dist2;
						m_Preview = (*pi).first;
						m_PreviewLoc = (*pi).second.m_Loc;
						m_PreviewName = (*pi).second.m_Name;
						m_PreviewSector = (*pi).first;
						m_PreviewType = (*pi).second.m_AtomType;
					}
				}
			}
			break;
		case DaAtom::ATOM_SolarSystem:
			if(m_FilterRange && !m_SectorIds.contains((*pi).second.m_SectorId))
				continue;	// Filtering by range and the sector is out of range.
			if((*pi).second.m_SystemExplored || !m_FilterExplored)
			{
				// System is explored or we are not filtering out unexplored systems.
				AuPoint eyeLoc = mv.Project((*pi).second.m_Loc);
				if(eyeLoc.z < 0 && eyeLoc.z > -120)
				{
					// Solar system location is in front of the eye plane and close enough to see.
					AuPoint pxloc = mvp.Project((*pi).second.m_Loc);
					double dist2 = pxloc.Distance2_2D(curpos);
					if(dist2 > 100*100)
						continue;	// more than 100 pixels away
					if(closestDist2 < 0 || dist2 < closestDist2)
					{
						closestDist2 = dist2;
						m_Preview = (*pi).first;
						m_PreviewGalaxy = (*pi).second.m_Galaxy;
						m_PreviewGenHab = (*pi).second.m_SystemGenHab;
						m_PreviewGenSeed = (*pi).second.m_SystemGenSeed;
						m_PreviewHasSpacecraft = (*pi).second.HasShip() || (*pi).second.HasStation();
						m_PreviewIsCoreBlackHole = (*pi).second.m_SystemCoreBlackHole;
						m_PreviewIsEmptySystem = (*pi).second.m_EmptySystem;
						m_PreviewIsExplored = (*pi).second.m_SystemExplored;
						m_PreviewIsScanned = (*pi).second.m_SystemScanned;
						m_PreviewLoc = (*pi).second.m_Loc;
						m_PreviewName = (*pi).second.m_Name;
						m_PreviewSector = (*pi).second.m_SectorId;
						m_PreviewType = (*pi).second.m_AtomType;
						m_PreviewCityEmpire = (*pi).second.m_CityEmpire;
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

void AuStarMapGL::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(m_Cursor);
	m_LeftButtonMode = 0;
	m_MouseButtons = Qt::NoButton;
	QOpenGLWidget::mouseReleaseEvent(e);

	if(e->button() == Qt::LeftButton)
	{
		//qDebug() << "AuStarMapGL::mouseReleaseEvent preview" << m_Preview << m_PreviewName
			//<< "target" << m_Target << m_TargetName;
		if(!m_Target.IsNull())
			emit PlaceClicked(m_Target);
	}
	else if(e->button() == Qt::RightButton)
	{
		// Build a right-click context menu.
		QMenu menu(this);

		QAction* lookAt = 0;
		QAction* selectSystem = 0;
		QAction* selectSector = 0;
		QAction* search = 0;
		QAction* reportExplored = 0;
		QAction* reportObserved = 0;
		QAction* reportSurveyed = 0;
		QAction* reportSpacecraft = 0;
		QAction* domainAdd = 0;
		QAction* goDirect = 0;
		QAction* goDirectToCoord = 0;
		QAction* goViaWormholes = 0;
		QAction* doScanSystem = 0;
		QAction* doScanTactical = 0;
		unordered_map<AuId, QAction*, AuIdHash> orbitCities;
		unordered_map<AuId, QAction*, AuIdHash> landAtCities;
		unordered_map<AuId, QAction*, AuIdHash> reportCitiesLast;
		unordered_map<AuId, QAction*, AuIdHash> reportCitiesHistory;
		if(!m_Target.IsNull())
		{
			if(m_TargetIsEmptySystem)
			{
				selectSector = menu.addAction(QIcon(":data/RangeSector.png"), tr("Select Sector"));
			}
			else
			{
				lookAt = menu.addAction(QIcon(":data/MsgSpot.png"), tr("Look At %1").arg(m_TargetName));
				selectSystem = menu.addAction(QIcon(":data/RangeSystem.png"), tr("Select %1").arg(m_TargetName));
				selectSector = menu.addAction(QIcon(":data/RangeSector.png"), tr("Select Sector Containing %1").arg(m_TargetName));
				search = menu.addAction(QIcon(":data/bObservatory.png"), tr("Search for Solar Systems Near %1").arg(m_TargetName));
			}

			if(m_TargetType == DaAtom::ATOM_SolarSystem)
			{
				reportSpacecraft = menu.addAction(QIcon(":data/Spacecraft.png"), tr("%1 Spacecraft Report...").arg(m_TargetName));
				if(!m_TargetHasSpacecraft)
					reportSpacecraft->setEnabled(false);

				if(!m_TargetIsEmptySystem)
				{
					if(m_TargetIsScanned)
						reportSurveyed = menu.addAction(QIcon(":data/SystemSurvey.png"),
								tr("%1 Survey Report...").arg(m_TargetName));
					else if(m_TargetIsExplored)
						reportExplored = menu.addAction(QIcon(":data/SystemWorlds.png"),
								tr("%1 Exploration Report...").arg(m_TargetName));
					else
						reportObserved = menu.addAction(QIcon(":data/bObservatory.png"),
								tr("%1 Observatory Report...").arg(m_TargetName));

					if(!auApp->IsSingleUserDesignerMode())
						domainAdd = menu.addAction(QIcon(":data/AvatarDomain.png"), tr("Add %1 to Land...").arg(m_TargetName));
				}
			}

			if(!auApp->IsSingleUserDesignerMode())
			{
				menu.addSeparator();

				// Travel orders
				if(m_TargetType == DaAtom::ATOM_Sector)
				{
					AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, m_TargetLoc);
					goDirectToCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
				else if(m_TargetType == DaAtom::ATOM_SolarSystem)
				{
					if(!m_TargetIsEmptySystem)
					{
						AuMissionOrderGoToSystemDirect order1(m_Target, m_TargetName, m_TargetLoc);
						goDirect = menu.addAction(QIcon(order1.ListNameIcon()), order1.ListName());

						AuMissionOrderGoToSystemViaWormholes order2(m_Target, m_TargetName);
						goViaWormholes = menu.addAction(QIcon(order2.ListNameIcon()), order2.ListName());

						AuMissionOrderScanSurveySystem order3(false);
						doScanSystem = menu.addAction(QIcon(order3.ListNameIcon()), order3.ListName());
						if(m_TargetIsScanned)
							doScanSystem->setEnabled(false);
					}

					AuMissionOrderScanTactical order4(false);
					doScanTactical = menu.addAction(QIcon(order4.ListNameIcon()), order4.ListName());

					if(!m_TargetIsEmptySystem)
					{
						// Look for cities.
						AuIdList cityIds;
						QStringList cityNames;
						QList<quint32> cityEmpires;
						g_AuClient.StarMapListCitiesInSystem(m_Target, cityIds, cityNames, cityEmpires);
						if(!cityIds.isEmpty())
						{
							QMenu* reportCitiesLastMenu = menu.addMenu(QIcon(":data/MailCityStatus.png"), tr("Recent Report"));

							QMenu* reportCitiesHistoryMenu = menu.addMenu(QIcon(":data/MailCityStatus.png"), tr("History Report"));

							AuMissionOrderGoToOrbitCity oGoToOrbitCity;
							QMenu* orbitMenu = menu.addMenu(QIcon(oGoToOrbitCity.ListNameIcon()), oGoToOrbitCity.ListName());

							AuMissionOrderGoToCityAnywhere oGoToCityAnywhere;
							QMenu* landAtMenu = menu.addMenu(QIcon(oGoToCityAnywhere.ListNameIcon()), oGoToCityAnywhere.ListName());

							AuIdList::const_iterator ii = cityIds.cbegin();
							QStringList::const_iterator ni = cityNames.cbegin();
							QList<quint32>::const_iterator ei = cityEmpires.cbegin();
							while(ii != cityIds.cend())
							{
								// Get the city's empire data record, if it's available.
								DgEmpire cityEmpire;
								cityEmpire.m_id = *ei;
								const DgData& gd = g_Au.LockDataForRead();
								bool hasEmpire = gd.Empire(cityEmpire);
								g_Au.UnlockData();

								AuView* pView = auApp->Frame()->View();
								quint32 sceneEmpire = pView->SceneAtomEmpire();

								if(hasEmpire)
								{
									QString nameWithEmp = cityEmpire.Name() + " " + *ni;

									QPixmap empireFlag;
									empireFlag.convertFromImage(cityEmpire.m_flag);
									QIcon flagIcon(empireFlag);

									QAction* landAtCity = landAtMenu->addAction(flagIcon, nameWithEmp);
									landAtCities.insert(pair<AuId, QAction*>(*ii, landAtCity));

									QAction* orbitCity = orbitMenu->addAction(flagIcon, nameWithEmp);
									orbitCities.insert(pair<AuId, QAction*>(*ii, orbitCity));

									QAction* reportCityLast = reportCitiesLastMenu->addAction(flagIcon, nameWithEmp + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityLast->setEnabled(false);
									reportCitiesLast.insert(pair<AuId, QAction*>(*ii, reportCityLast));

									QAction* reportCityHistory = reportCitiesHistoryMenu->addAction(flagIcon, nameWithEmp + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityHistory->setEnabled(false);
									reportCitiesHistory.insert(pair<AuId, QAction*>(*ii, reportCityHistory));
								}
								else
								{
									QAction* landAtCity = landAtMenu->addAction(QIcon(":data/City.png"), *ni);
									landAtCities.insert(pair<AuId, QAction*>(*ii, landAtCity));

									QAction* orbitCity = orbitMenu->addAction(QIcon(":data/City.png"), *ni);
									orbitCities.insert(pair<AuId, QAction*>(*ii, orbitCity));

									QAction* reportCityLast = reportCitiesLastMenu->addAction(QIcon(":data/MailCityStatus.png"),
											*ni + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityLast->setEnabled(false);
									reportCitiesLast.insert(pair<AuId, QAction*>(*ii, reportCityLast));

									QAction* reportCityHistory =
										reportCitiesHistoryMenu->addAction(QIcon(":data/MailCityStatus.png"), *ni + "...");
									if(cityEmpire.m_id != sceneEmpire)
										reportCityHistory->setEnabled(false);
									reportCitiesHistory.insert(pair<AuId, QAction*>(*ii, reportCityHistory));
								}

								ii++;
								ni++;
								ei++;
							}
						}
					}
				}
			}

			// Separator before the go to coordinates orders.
			menu.addSeparator();
		}

		// Determine the point in the plane of the galaxy.
		QAction* goDirectToZ0Coord = 0;	// Coord at Z=0 plane of the galaxy.
		AuPoint z0Loc;					// Resulting point in the z=0 plane.
		QAction* goDirectToZSCoord = 0;	// Coord at Z=sceneAtomLoc.z plane of the galaxy.
		AuPoint zSLoc;					// Resulting point in the z=sceneAtomLoc.z plane.
		{
			// Calculate the combined model view projection matrix.
			AuMatrix4x4 mvp;
			GetMVP(mvp);
			AuMatrix4x4 twp;				// Plane into which the point is projected.
			if(mvp.UnProjectIntoPlane(z0Loc, e->pos(), twp))
			{
				z0Loc.Round(.0001);

				AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, z0Loc);
				goDirectToZ0Coord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
			}

			if(m_InGalaxy)
			{
				twp.Translate(m_SceneAtomLoc);
				if(mvp.UnProjectIntoPlane(zSLoc, e->pos(), twp))
				{
					zSLoc.Round(.0001);

					AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, zSLoc);
					goDirectToZSCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
				}
			}
		}

		QAction* action = menu.exec(QCursor::pos());
		if(!action)
			return;

		if(action == lookAt)
		{
			// Center view on preview atom, which is the closest atom to the cursor.
			emit PlaceDoubleClicked(m_Target);
		}
		else if(action == search)
		{
			emit SystemSearchAt(m_TargetGalaxy, m_TargetLoc);
		}
		else if(action == selectSystem)
		{
			emit PlaceClicked(m_Target);
		}
		else if(action == selectSector)
		{
			emit PlaceClicked(m_TargetSector);
		}
		else if(action == reportSurveyed)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 2);
		}
		else if(action == reportExplored)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 1);
		}
		else if(action == reportObserved)
		{
			emit ReportSystemResources(m_Target, m_TargetName, m_TargetGalaxy, m_TargetIsCoreBlackHole, m_TargetGenHab,
					m_TargetGenSeed, m_TargetLoc, -1.f, 0);
		}
		else if(action == domainAdd)
		{
			emit DomainAdd(m_Target, m_TargetName);
		}
		else if(action == reportSpacecraft)
		{
			AuView* pView = auApp->Frame()->View();
			quint32 sceneEmpire = pView->SceneAtomEmpire();
			emit ReportSpacecraft(sceneEmpire, m_TargetSector, m_Target, m_TargetName);
		}
		else if(action == goDirect)
		{
			emit GoDirectToSystem(m_Target, m_TargetName, m_TargetLoc);
		}
		else if(action == goDirectToCoord)
		{
			emit GoDirectToCoord(m_TargetLoc);
		}
		else if(action == goDirectToZ0Coord)
		{
			emit GoDirectToCoord(z0Loc);
		}
		else if(action == goDirectToZSCoord)
		{
			emit GoDirectToCoord(zSLoc);
		}
		else if(action == goViaWormholes)
		{
			emit GoViaWormholes(m_Target, m_TargetName);
		}
		else if(action == doScanSystem)
		{
			emit ScanSystem();
		}
		else if(action == doScanTactical)
		{
			emit ScanTactical();
		}
		else
		{
			// Check cities
			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = orbitCities.begin(); ci != orbitCities.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit OrbitCity((*ci).first, (*ci).second->text());
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = landAtCities.begin(); ci != landAtCities.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit LandAtCity((*ci).first, (*ci).second->text());
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = reportCitiesLast.begin();
					ci != reportCitiesLast.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit ReportCityLast((*ci).first);
					break;
				}
			}

			for(unordered_map<AuId, QAction*, AuIdHash>::const_iterator ci = reportCitiesHistory.begin();
					ci != reportCitiesHistory.end(); ci++)
			{
				if(action == (*ci).second)
				{
					emit ReportCityHistory((*ci).first);
					break;
				}
			}
		}
	}
}

void AuStarMapGL::GetMVP(AuMatrix4x4& mvp)
{
	AuMatrix4x4 mv;
	GetMVP(mv, mvp);
}

void AuStarMapGL::GetMVP(AuMatrix4x4& mv, AuMatrix4x4& mvp)
{
	// Apply perspective transformation that accounts for aspect ratio of screen.
	double nearClip = 4;
	AuMatrix4x4 perspT;					// Perspective matrix.
	if(width() > 0)
		perspT.Perspective(45.0*DEG2RAD, (double)height()/(double)width(), nearClip, 1000000.0);
	else
		perspT.Perspective(45.0*DEG2RAD, 1, nearClip, 1000000.0);

	// Calculate a projection matrix that gives us results like OpenGL.
	AuMatrix4x4 pT;						// Projection matrix.
	pT.Scale(width()/2,-height()/2,1);	// Invert Y. Scale to pixels.
	pT.Translate(1,-1,0);				// Offset to upper left origin.
	pT *= perspT;

	// Calculate the model view matrix that places the camera where we want it.
	mv.LoadIdentity();
	mv.Translate(0,0,-(m_LookDist + nearClip));
	mv.Rotate(m_LookRot);
	mv.Translate(-m_LookPoint);

	// Calculate the combined model view projection matrix.
	mvp = pT * mv;
}

void AuStarMapGL::GetMVP(AuMatrix4x4& p, AuMatrix4x4& mv, AuMatrix4x4& mvp)
{
	// Apply perspective transformation that accounts for aspect ratio of screen.
	double nearClip = 4;
	p.LoadIdentity();
	if(width() > 0)
		p.Perspective(45.0*DEG2RAD, (double)height()/(double)width(), nearClip, 1000000.0);
	else
		p.Perspective(45.0*DEG2RAD, 1, nearClip, 1000000.0);

	// Calculate the model view matrix that places the camera where we want it.
	mv.LoadIdentity();
	mv.Translate(0,0,-(m_LookDist + nearClip));
	mv.Rotate(m_LookRot);
	mv.Translate(-m_LookPoint);

	// Calculate the combined model view projection matrix.
	mvp = p * mv;
}

void AuStarMapGL::LookAtPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		bool inThisGalaxy = false;
		GALAXY lookGalaxy = (*pi).second.Galaxy();
		if(id == AuId(GALAXY_Count))// && (*pi).second.m_Loc == AuPoint(0,0))
		{
			// We do not know the location of the scene atom yet.
			// Acquire current galaxy location of scene atom.
			AuView* pView = auApp->Frame()->View();
			DaPerson* person;
			pView->LockSceneAtom(&person);
			if(person)
			{
				const DaGalaxy* galaxy = person->FindParentGalaxy();
				if(galaxy && galaxy->Galaxy() == Galaxy())
				{
					person->GetLocationRelativeToParent((*pi).second.m_Loc, galaxy);
					inThisGalaxy = true;
					lookGalaxy = galaxy->Galaxy();
				}
			}
			pView->UnlockSceneAtom();
		}
		else
			inThisGalaxy = true;

		if(inThisGalaxy && m_LookPoint != (*pi).second.m_Loc)
		{
			m_LookPoint = (*pi).second.m_Loc;
			if(Galaxy() != lookGalaxy)
			{
				m_Galaxy = lookGalaxy;
				m_SpotLocs.clear();
			}
			if(isVisible())
				update();
		}
	}
}

void AuStarMapGL::DeselectPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		if((*pi).second.m_Selected)
		{
			// We don't count the 'Current Location' and 'Galaxy' places in our selection count.
			// Otherwise, they cause wierd behavior when the selection filter is active.
			if((*pi).first.GetID() > GALAXY_Count)
				m_CountSelected--;

			(*pi).second.m_Selected = false;
			if(isVisible())
				update();
		}
	}
}

void AuStarMapGL::SelectPlace(const AuId& id)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		if(!(*pi).second.m_Selected)
		{
			// We don't count the 'Current Location' and 'Galaxy' places in our selection count.
			// Otherwise, they cause wierd behavior when the selection filter is active.
			if((*pi).first.GetID() > GALAXY_Count)
				m_CountSelected++;

			if((*pi).second.m_AtomType != DaAtom::ATOM_Avatar && Galaxy() != (*pi).second.Galaxy())
			{
				m_Galaxy = (*pi).second.Galaxy();
				m_SpotLocs.clear();
			}

			m_LastSelected = id;
			(*pi).second.m_Selected = true;
			if(isVisible())
				update();
		}
	}
}

void AuStarMapGL::HideGalaxy()
{
	if(m_ShowGalaxy)
	{
		m_ShowGalaxy = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowGalaxy", m_ShowGalaxy);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowGalaxy()
{
	if(!m_ShowGalaxy)
	{
		m_ShowGalaxy = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowGalaxy", m_ShowGalaxy);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::HideScanned()
{
	if(m_ShowScanned)
	{
		m_ShowScanned = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowScanned", m_ShowScanned);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowScanned()
{
	if(!m_ShowScanned)
	{
		m_ShowScanned = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowScanned", m_ShowScanned);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::HideShips()
{
	if(m_ShowShips)
	{
		m_ShowShips = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowShips", m_ShowShips);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowShips()
{
	if(!m_ShowShips)
	{
		m_ShowShips = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowShips", m_ShowShips);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::HideSignals()
{
	if(m_ShowSignals)
	{
		m_ShowSignals = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowSignals", m_ShowSignals);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowSignals()
{
	if(!m_ShowSignals)
	{
		m_ShowSignals = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowSignals", m_ShowSignals);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::HideStations()
{
	if(m_ShowStations)
	{
		m_ShowStations = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowStations", m_ShowStations);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowStations()
{
	if(!m_ShowStations)
	{
		m_ShowStations = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowStations", m_ShowStations);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::HideWormholes()
{
	if(m_ShowWormholes)
	{
		m_ShowWormholes = false;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowWormholes", m_ShowWormholes);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::ShowWormholes()
{
	if(!m_ShowWormholes)
	{
		m_ShowWormholes = true;
		g_Au.Settings().writeEntry("Widget/StarMap/ShowWormholes", m_ShowWormholes);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::FilterExplored(bool filterExplored)
{
	if(m_FilterExplored != filterExplored)
	{
		m_FilterExplored = filterExplored;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterExplored", m_FilterExplored);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::FilterRange(bool filterRange)
{
	if(m_FilterRange != filterRange)
	{
		m_FilterRange = filterRange;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterRange", m_FilterRange);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::FilterSelection(bool filterSelection)
{
	if(m_FilterSelection != filterSelection)
	{
		m_FilterSelection = filterSelection;
		g_Au.Settings().writeEntry("Widget/StarMap/FilterSelection", m_FilterSelection);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::RestoreSettings()
{
	//qDebug() << "AuStarMapGL::RestoreSettings";
	int range;
	range = g_Au.Settings().readNumEntry("Widget/StarMap/Range", 1);
	if(range < 0)
		range = 0;
	else if(range > 1000)
		range = 1000;
	if(m_Range != range)
		m_Range = range;

	m_FilterExplored = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterExplored", false);
	m_FilterRange = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterRange", false);
	m_FilterSelection = g_Au.Settings().readBoolEntry("Widget/StarMap/FilterSelection", false);

	m_ShowGalaxy = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowGalaxy", true);
	m_ShowScanned = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowScanned", true);
	m_ShowShips = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowShips", true);
	m_ShowSignals = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowSignals", true);
	m_ShowStations = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowStations", true);
	m_ShowWormholes = g_Au.Settings().readBoolEntry("Widget/StarMap/ShowWormholes", true);
}

// 'loc' is location relative to the galaxy, in parsecs.
void AuStarMapGL::AddPlace(const AuId& id, DaAtom::ATOM_TYPE type, const QIcon& icon, const QString& name, const QColor& color,
		const AuPoint& loc, const AuId& sectorId, GALAXY galaxy, bool systemCoreBlackHole, bool isEmptySystem, quint32 systemGenHab,
		quint32 systemGenSeed, bool systemExplored, bool systemScanned, quint32 cityEmpire, quint8 signalStrength,
		quint16 empireShips, quint16 empireStations,
		quint16 friendShips, quint16 friendStations,
		quint16 neutralShips, quint16 neutralStations,
		quint16 enemyShips, quint16 enemyStations)
{
	// Grab the galaxy from the first place we receive that has a galaxy.
	if(Galaxy() == GALAXY_Count && Galaxy() != galaxy)
	{
		m_Galaxy = galaxy;
		m_SpotLocs.clear();
	}

	AuStarMapWidgetPlace place(type, icon, name, color, loc, sectorId, galaxy, systemCoreBlackHole, isEmptySystem, systemGenHab,
			systemGenSeed, systemExplored, systemScanned, cityEmpire, signalStrength,
			empireShips, empireStations, friendShips, friendStations, neutralShips, neutralStations, enemyShips, enemyStations);
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second = place;	// replace existing place
	else
		m_Places.insert(pair<AuId, AuStarMapWidgetPlace>(id, place));
}

void AuStarMapGL::SetPlaceColor(const AuId& id, const QColor& color)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second.m_Color = color;
}

void AuStarMapGL::SetPlaceIcon(const AuId& id, const QIcon& icon)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
		(*pi).second.m_Icon = icon;
}

void AuStarMapGL::AddPlaceWormhole(const AuId& id, const AuPoint& wormVec, bool isExplored)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(id);
	if(pi != m_Places.end())
	{
		// When we have both an explored and an unexplored wormhole going to the same destination, we prefer to only show the
		// explored wormhole.
		AuPoint wormDest = (*pi).second.m_Loc + wormVec;
		if(isExplored)
		{
			if(!(*pi).second.m_WormDestEx.contains(wormDest))
			{
				(*pi).second.m_WormDestEx.append(wormDest);			// Add the destination to the explored wormholes list.
				(*pi).second.m_WormDestUnex.removeAll(wormDest);	// Remove the destination from the unexplored wormholes list.
			}
		}
		else if(!(*pi).second.m_WormDestEx.contains(wormDest))
		{
			if(!(*pi).second.m_WormDestUnex.contains(wormDest))
				(*pi).second.m_WormDestUnex.append(wormDest);		// Add to the unexplored wormholes list if not in explored list.
		}
	}
}

void AuStarMapGL::ClearUniverse()
{
	m_Places.clear();
	m_CountSelected = 0;
}

void AuStarMapGL::ClearGalaxy(GALAXY galaxy)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin();
	while(pi != m_Places.end())
	{
		if((*pi).second.Galaxy() == galaxy)
			pi = m_Places.erase(pi);
		else
			pi++;
	}
}

void AuStarMapGL::ClearSectors(const AuIdList& sectorIds)
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin();
	while(pi != m_Places.end())
	{
		if(sectorIds.contains((*pi).first) || sectorIds.contains((*pi).second.m_SectorId))
			pi = m_Places.erase(pi);	// Found the sector or a place that is in it.
		else
			pi++;
	}
}

void AuStarMapGL::ClearSelection()
{
	bool anyChange = false;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.m_Selected)
		{
			(*pi).second.m_Selected = false;
			anyChange = true;
		}
	}
	m_CountSelected = 0;

	if(anyChange)
		update();
}

void AuStarMapGL::SetRange(int range)
{
	int newRange;
	if(range < 0)
		newRange = 0;
	else if(range > 1000)
		newRange = 1000;
	else
		newRange = range;
	if(m_Range != newRange)
	{
		m_Range = newRange;
		m_SpotLocs.clear();
		g_Au.Settings().writeEntry("Widget/StarMap/Range", m_Range);
		if(isVisible())
			update();
	}
}

void AuStarMapGL::LookAtLastSelected()
{
	unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.find(m_LastSelected);
	if(pi != m_Places.end() && m_LookPoint != (*pi).second.m_Loc)
	{
		m_LookPoint = (*pi).second.m_Loc;

		if(Galaxy() != (*pi).second.Galaxy())
		{
			m_Galaxy = (*pi).second.Galaxy();
			m_SpotLocs.clear();
		}

		if(isVisible())
			update();
	}
}

void AuStarMapGL::LookAtSelection()
{
	// Calculate the average center point of the selected places.
	AuPoint locavg;						// Sum of all selected locations for average.
	GALAXY galaxy = Galaxy();
	int selected = 0;					// Count of selected items for average.
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
	{
		if((*pi).second.m_Selected)
		{
			galaxy = (*pi).second.Galaxy();
			locavg += (*pi).second.m_Loc;
			selected++;
		}
	}

	if(selected)
	{
		// Move look point to the average of the selected place locations.
		if(selected > 1)
			locavg /= selected;			// Calculate the average of the selected place locations.
		if(m_LookPoint != locavg)
		{
			if(Galaxy() != galaxy)
			{
				m_Galaxy = galaxy;
				m_SpotLocs.clear();
			}

			m_LookPoint = locavg;
			if(isVisible())
				update();
		}
	}
}

void AuStarMapGL::keyPressEvent(QKeyEvent *e)
{
	switch(e->key())
	{
	case Qt::Key_W:
		{
			// move forward
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(0,0,-1));
			m_LookPoint += moveVec;
		}
		break;
	case Qt::Key_S:
		{
			// back up
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(0,0,1));
			m_LookPoint += moveVec;
		}
		break;
	case Qt::Key_A:
		{
			// strafe left
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(-1,0,0));
			m_LookPoint += moveVec;
		}
		break;
	case Qt::Key_D:
		{
			// strafe right
			AuMatrix4x4 t;
			t.Rotate(m_LookRot);
			AuPoint moveVec = t.UnProject(AuPoint(1,0,0));
			m_LookPoint += moveVec;
		}
		break;
	case Qt::Key_C:
		{
			AuMatrix4x4 t;
			t.RotateZ(DEG2RAD*2);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();
		}
		break;
	case Qt::Key_Z:
		{
			AuMatrix4x4 t;
			t.RotateZ(-DEG2RAD*2);
			t.Rotate(m_LookRot);
			m_LookRot = t.GetRotation();
		}
		break;
	}
	QWidget::keyPressEvent(e);
}

AuIdList AuStarMapGL::SelectedPlaces() const
{
	AuIdList selected;
	for(unordered_map<AuId, AuStarMapWidgetPlace, AuIdHash>::const_iterator pi = m_Places.begin(); pi != m_Places.end(); pi++)
		if((*pi).second.m_Selected)
			selected.append((*pi).first);
	return selected;
}

void AuStarMapGL::SetLookPoint(const AuPoint& look)
{
	m_LookPoint = look;
	if(isVisible())
		update();
}

