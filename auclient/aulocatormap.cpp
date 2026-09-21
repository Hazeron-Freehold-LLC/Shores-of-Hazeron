//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulocatormap.cpp,v 1.32 2022/04/13 15:19:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulocatormap.h"
//
#include "aubeamoptions.h"
#include "aubeamvehicleoptions.h"
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auterraininfo.h"
#include "auview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QSplitter>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>

void AuLocatorWorldMapDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleLocatorWorldMapUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocatorMapWidget::AuLocatorMapWidget(QWidget* parent) : QWidget(parent), m_Cursor(QPixmap(":data/CrossHairCursor.png"), 32,32)
{
	m_CanBeamOver = false;
	m_CanBeamVehicle = false;
	m_CenterLat = 0;
	m_CenterLong = 0;
	m_CompassLoc = QPoint(-1,-1);
	m_CompassSize = 0;
	m_FollowPerson = true;
	m_IsCrew = false;
	m_IsNewbieEmperor = false;
	m_IsRingSection = false;
	m_LeftButtonMode = 0;
	m_MagnifierScale = 1;
	m_PlanetLatDivs = 0;
	m_PlanetLonDivs = 0;
	m_Scale = 1;
	m_SunlightEnable = g_Au.Settings().readBoolEntry("Widget/LocatorMap/ShowSunlight", true);
	m_timer = 0;

	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setCursor(m_Cursor);
}

AuLocatorMapWidget::~AuLocatorMapWidget()
{
}

void AuLocatorMapWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(500);
}

QSize AuLocatorMapWidget::sizeHint() const
{
	return QSize(800,400);
}

void AuLocatorMapWidget::UpdateContents()
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

void AuLocatorMapWidget::paintEvent(QPaintEvent* pe)
{
	QPainter p(this);

	// Fill background with a dark color.
	QBrush brBack(QColor(0,0,0));
	p.fillRect(0,0, width(),height(), brBack);

	QString slonglat, scity, sdev, shipname, shipinfo, secDesc, secName, sysNameDesc, oat, wx, wind;
	QString sysPos, secPos, galPos, galName;
	AuView* pView = auApp->Frame()->View();
	INDUSTRYID iid;
	SPACECRAFT_STRUCTURE shipStruct;
	BODY_TYPE bodyType;
	double rheading = pView->GetLocationInfo(m_PlanetName, bodyType, slonglat, scity, sdev, iid, shipname, shipinfo, shipStruct,
			sysNameDesc, secDesc, secName, oat, wx, wind, sysPos, secPos, galPos, galName);

	// Draw planet map, if we have one.
	// Lock the root and find the scene atom.
	bool isExplored = false;
	int resourceZone = -1;
	int rotdir = 0;
	QString latZone;
	QString terType;
	QString devType;
	QColor devColor;
	m_PlanetLatDivs = 0;	// Unless we find a planet.
	m_PlanetLonDivs = 0;	// Unless we find a planet.
	bool isPlanetoid = false;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		m_IsCrew = !person->SpawnShip().IsNull();
		m_IsNewbieEmperor = person->IsNewbieEmperor();
		const DaWorldLL* llworld = person->FindParentWorldInEnv();
		if(llworld)
		{
			isPlanetoid = llworld->IsPlanetoidSize();
			if(llworld->SurfaceImage())
				m_PlanetImage = *llworld->SurfaceImage();
			if(!m_PlanetImage.isNull())
			{
				// Globe world with a surface image.
				rotdir = llworld->DirectionOfRotation();
				if(!llworld->Is(m_PlanetId))
				{
					m_FollowPerson = true;							// Planet changed so resume following person.
					m_FogOfWarImg = QImage();
					m_SunlightMap = QImage();						// Sunlight map is no longer valid.
				}
				m_PlanetId = llworld->Id();
				if(dynamic_cast<const DaWorldRingSection*>(llworld))
					m_IsRingSection = true;
				else
					m_IsRingSection = false;
				m_SystemId = llworld->FindParentSolarSystem()->Id();

				// Determine if the person can request advanced beam-over options on the voice channel.
				m_CanBeamOver = false;
				m_CanBeamVehicle = false;
				if(person->InVehicleTransporter() != SPOT_Invalid)
				{
					// Person is aboard a ship in a transporter room.
					DaCapsule* ship = person->FindParentCapsuleInEnv();
					if(ship)
						m_CanBeamVehicle = true;
				}
				else if(person->InTransporterRoom())
				{
					// Person is aboard a ship in a transporter room.
					DaCapsule* ship = person->FindParentCapsuleInEnv();
					if(ship)
						m_CanBeamOver = true;
				}

				// Determine the person's location on the map, in map pixel coordinates.
				AuGlobeLLF personLL = person->GlobeLL();
				if(m_FollowPerson)
				{
					m_CenterLong = personLL.Longitude();
					m_CenterLat = personLL.Latitude();
				}

				// On globe type worlds, we will store the pixel dimensions of the surface image.
				m_PlanetLonDivs = m_PlanetImage.width();
				m_PlanetLatDivs = m_PlanetImage.height();

				// Fog of war
				AuFogOfWarData fow;
				if(llworld->FogOfWar(person->Id(), person->EmpireId(), fow))
				{
					bool updateFog = false;
					if(m_FogOfWarImg.isNull() || m_FogOfWarImg.size() != m_PlanetImage.size())
					{
						m_FogOfWarImg = QImage(m_PlanetImage.size(), QImage::Format_ARGB32);
						updateFog = true;
					}
					else if(m_FogOfWarData != fow)
						updateFog = true;

					if(updateFog)
					{
						//qDebug() << "AuLocatorMapWidget::paintEvent updateFog" << llworld->Id() << m_FogOfWarImg.size();
						m_FogOfWarData = fow;
						m_FogOfWarImg.fill(qRgba(0,0,0,0));

						double pixelsPerFoot = m_FogOfWarImg.height() / llworld->SurfaceImageHeight();
						float spotRadY = (float)(fow.Radius() * pixelsPerFoot) + 2;
						int eqspots = llworld->SurfaceImageHeight() / fow.Radius();	// number of spots around the equator
						QImage imgShroud(":data/MapShroud.png");
						QPainter p(&m_FogOfWarImg);

						AuGlobeLLF ll;
						for(QList<AuPoint>::const_iterator fi = m_FogOfWarData.Fog().cbegin();
								fi != m_FogOfWarData.Fog().cend(); fi++)
						{
							if(!llworld->LLOfLoc(ll, *fi))
								continue;
							// Calculate location of fog spot long lat.
							int x = (int)((ll.Longitude() + PIf) * m_FogOfWarImg.width() / (PIf*2.f));
							int y = (int)(((PIf/2.f) - ll.Latitude()) * m_FogOfWarImg.height() / PIf);

							// Calculate spot size based on latitude.
							// The Mercator projection means the spots must get wider as they approach the poles.
							int spotsAtLat = cosf(ll.Latitude()) * eqspots;
							if(!spotsAtLat)
								spotsAtLat = 1;
							float llradx = ((float)m_FogOfWarImg.width() / (float)spotsAtLat) / 2.f + 2;
							float llrady = spotRadY;
							float imgl = x - llradx;
							float imgt = y - llrady;
							QRectF imgrc(imgl,imgt, llradx*2,llrady*2);
							p.drawImage(imgrc, imgShroud);

							// Wrap at the sides when the rect goes off the edge.
							if(imgl < 0)
							{
								QRectF imgrc(imgl + m_FogOfWarImg.width(),imgt, llradx*2,llrady*2);
								p.drawImage(imgrc, imgShroud);
							}
							if(imgl + llradx*2 >= m_FogOfWarImg.width())
							{
								QRectF imgrc(imgl - m_FogOfWarImg.width(),imgt, llradx*2,llrady*2);
								p.drawImage(imgrc, imgShroud);
							}
						}
					}
				}
				else
					m_FogOfWarImg = QImage();

				// Draw night/day into a bitmap that corresponds to the surface image of the planet.
				if(m_SunlightEnable)
				{
					bool updateSunlight = false;
					if(m_SunlightMap.isNull() || m_SunlightMap.size() != m_PlanetImage.size())
					{
						m_SunlightMap = QImage(m_PlanetImage.size(), QImage::Format_ARGB32);
						updateSunlight = true;
					}
					else if(!m_SunlightTime.isValid() || m_SunlightTime.elapsed()/d_MSecPerSec > 60)
						updateSunlight = true;

					if(updateSunlight)
					{
						//qDebug() << "AuLocatorMapWidget::paintEvent updateSunlight" << llworld->Id() << m_SunlightMap.size();
						m_SunlightMap.fill(qRgba(0,0,0,0));
						for(int ix=0; ix<m_SunlightMap.width(); ix++)
						{
							// Determine longitude of the pixel at (ix,iy).
							float lon = PIf*2.f * (float)ix / (float)m_PlanetLonDivs - PIf;

							for(int iy=0; iy<m_SunlightMap.height(); iy++)
							{
								// Determine latitude of the pixel at (ix,iy).
								float lat = PIf/2.f - PIf * ((float)iy / (float)m_PlanetLatDivs);

								// Calculate light level at the location.
								AuPoint surfLoc = llworld->LocOfLL(AuGlobeLLF(lon, lat));
								float sunlight = llworld->QuerySunlight(surfLoc);

								// Calculate darkness factor for sunlight overlay.
								quint8 darkness;
								if(sunlight >= 1.f)
									darkness = 0;
								else
									darkness = (255 - (quint8)(sunlight * 255.f)) * 9 / 10;

								m_SunlightMap.setPixel(QPoint(ix,iy), qRgba(0,0,0,darkness));
							}
						}
						m_SunlightTime.restart();
					}
				}

				int sxpos = (personLL.Longitude() + PIf) / (PIf*2) * m_PlanetLonDivs;	// map pixel location of person
				int sypos = (1.f - (personLL.Latitude() + PIf/2) / PIf) * m_PlanetLatDivs;		// map pixel location of person

				// Center point of map display.
				AuGlobeLL mapLL(m_CenterLong, m_CenterLat);
				slonglat = mapLL.ToString();
				int mxpos = (mapLL.Longitude() + PIf) / (PIf*2) * m_PlanetLonDivs;	// map pixel location of center point
				int mypos = (1.f - (mapLL.Latitude() + PIf/2) / PIf) * m_PlanetLatDivs;	// map pixel location of center point

				// Draw the map.
				int szx = round(m_PlanetLonDivs * m_Scale);
				int szy = round(m_PlanetLatDivs * m_Scale);
				int mapx = width()/2 - szx/2;										// Centered in display area.
				int mapy = height()/2 - round(mypos * m_Scale);						// Center of display area.
				QRect rcmap(mapx,mapy, szx,szy);		// Destination rectangle for the map.
				int dispwid = m_PlanetImage.width();
				QImage img(dispwid,m_PlanetImage.height(), QImage::Format_ARGB32);
				QPainter p2(&img);
				if(!m_IsRingSection && mxpos < dispwid/2)
				{
					// The xpos is far enough left in the map strip that we must draw it in two pieces.
					int endWid = dispwid/2 - mxpos;
					int begWid = dispwid - endWid;
					QRect rcsrc1(0,0, begWid,m_PlanetImage.height());
					QRect rcsrc2(m_PlanetImage.width() - endWid,0, endWid,m_PlanetImage.height());

					// Draw the two pieces into a single image that we will stretch into the zoom rectangle.
					p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_PlanetImage, rcsrc2);
					p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_PlanetImage, rcsrc1);

					if(!m_FogOfWarImg.isNull())
					{
						// Overlay fog of war.
						p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_FogOfWarImg, rcsrc2);
						p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_FogOfWarImg, rcsrc1);
					}

					if(m_SunlightEnable)
					{
						// Overlay pieces of the sunlight image into a single image that we will stretch into the zoom rectangle.
						p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_SunlightMap, rcsrc2);
						p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_SunlightMap, rcsrc1);
					}
				}
				else if(!m_IsRingSection && mxpos > m_PlanetImage.width() - dispwid)
				{
					// The xpos is far enough right in the map strip that we must draw it in two pieces.
					int begWid = mxpos - (m_PlanetImage.width() - dispwid/2);
					int endWid = dispwid - begWid; 
					QRect rcsrc1(0,0, begWid,m_PlanetImage.height());
					QRect rcsrc2(m_PlanetImage.width() - endWid,0, endWid,m_PlanetImage.height());
					
					// Draw the two pieces into a single image that we will stretch into the zoom rectangle.
					p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_PlanetImage, rcsrc2);
					p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_PlanetImage, rcsrc1);

					if(!m_FogOfWarImg.isNull())
					{
						// Overlay fog of war.
						p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_FogOfWarImg, rcsrc2);
						p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_FogOfWarImg, rcsrc1);
					}

					if(m_SunlightEnable)
					{
						// Overlay pieces of the sunlight image into a single image that we will stretch into the zoom rectangle.
						p2.drawImage(QRect(0,0,endWid,rcsrc2.height()), m_SunlightMap, rcsrc2);
						p2.drawImage(QRect(endWid,0,begWid,rcsrc1.height()), m_SunlightMap, rcsrc1);
					}
				}
				else
				{
					// The xpos is centered in the map strip enough that we can just grab the chunk we need.
					QRect rcsrc(mxpos - dispwid/2,0, dispwid,m_PlanetImage.height());
					p2.drawImage(QRect(0,0,dispwid,m_PlanetImage.height()), m_PlanetImage, rcsrc);

					if(!m_FogOfWarImg.isNull())
					{
						// Overlay fog of war.
						p2.drawImage(QRect(0,0,dispwid,m_FogOfWarImg.height()), m_FogOfWarImg, rcsrc);
					}

					if(m_SunlightEnable)
					{
						// Overlay the sunlight image.
						p2.drawImage(QRect(0,0,dispwid,m_PlanetImage.height()), m_SunlightMap, rcsrc);
					}
				}

				p2.end();
				p.drawImage(rcmap, img);

				// Calculate person's current location, which could be outside the visible map area on a ringworld.
				int scx = round((float)mapx + (dispwid/2+(sxpos-mxpos)) * m_Scale + m_Scale/2);
				int mapWidth = round(m_PlanetImage.width() * m_Scale);
				if(scx < mapx)
					scx += mapWidth;
				if(scx >= mapx + szx)
					scx -= mapWidth;	// On ringworld, could end up in nonvisible area, i.e. less than mapx.
				if(scx >= mapx)
				{
					// Draw a blip at the person's current location.
					int scy = round((float)mapy + (mypos+(sypos-mypos))*m_Scale + m_Scale/2);	// Scaled/centered in its map cell.

					// Select the appropriate image
					QImage img;
					if(person->IsInSpacecraft())
					{
						const DaVessel* ship = person->FindParentVesselInEnv();
						if(ship)
						{
							POLITICAL_STANCE stance = PoliticalStance(person->EmpireId(), ship->EmpireId());
							bool shields = ship->ShieldsWorkingAndUp();
							SPACECRAFT_STRUCTURE structure = ship->Structure();
							if(shields)
							{
								switch(structure)
								{
								case SC_Count:
								case SC_Deprecated:
									break;
								case SC_Hull:
									{
										const char* icons[] =
										{
											":data/BlipSpacecraftHullFriend.png",
											":data/BlipSpacecraftHullNeutral.png",
											":data/BlipSpacecraftHullEnemy.png",
											":data/BlipSpacecraftHullEmpire.png",
											":data/BlipSpacecraftHullEmpire.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Starship:
									{
										const char* icons[] =
										{
											":data/BlipStarshipFriendS.png",
											":data/BlipStarshipNeutralS.png",
											":data/BlipStarshipEnemyS.png",
											":data/BlipStarshipEmpireS.png",
											":data/BlipStarshipEmpireS.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Spaceship:
									{
										const char* icons[] =
										{
											":data/BlipSpaceshipFriendS.png",
											":data/BlipSpaceshipNeutralS.png",
											":data/BlipSpaceshipEnemyS.png",
											":data/BlipSpaceshipEmpireS.png",
											":data/BlipSpaceshipEmpireS.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Station:
									{
										const char* icons[] =
										{
											":data/BlipSpaceportFriendS.png",
											":data/BlipSpaceportNeutralS.png",
											":data/BlipSpaceportEnemyS.png",
											":data/BlipSpaceportEmpireS.png",
											":data/BlipSpaceportEmpireS.png",
										};
										img.load(icons[stance]);
									}
									break;
								}
							}
							else
							{
								switch(structure)
								{
								case SC_Count:
								case SC_Deprecated:
									break;
								case SC_Hull:
									{
										const char* icons[] =
										{
											":data/BlipSpacecraftHullFriend.png",
											":data/BlipSpacecraftHullNeutral.png",
											":data/BlipSpacecraftHullEnemy.png",
											":data/BlipSpacecraftHullEmpire.png",
											":data/BlipSpacecraftHullEmpire.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Starship:
									{
										const char* icons[] =
										{
											":data/BlipStarshipFriend.png",
											":data/BlipStarshipNeutral.png",
											":data/BlipStarshipEnemy.png",
											":data/BlipStarshipEmpire.png",
											":data/BlipStarshipEmpire.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Spaceship:
									{
										const char* icons[] =
										{
											":data/BlipSpaceshipFriend.png",
											":data/BlipSpaceshipNeutral.png",
											":data/BlipSpaceshipEnemy.png",
											":data/BlipSpaceshipEmpire.png",
											":data/BlipSpaceshipEmpire.png",
										};
										img.load(icons[stance]);
									}
									break;
								case SC_Station:
									{
										const char* icons[] =
										{
											":data/BlipSpaceportFriend.png",
											":data/BlipSpaceportNeutral.png",
											":data/BlipSpaceportEnemy.png",
											":data/BlipSpaceportEmpire.png",
											":data/BlipSpaceportEmpire.png",
										};
										img.load(icons[stance]);
									}
									break;
								}
							}
						}
					}
					else if(person->IsInBd())
					{
						const DaBd* bd = person->FindParentBdInEnv();
						if(bd)
						{
							POLITICAL_STANCE stance = PoliticalStance(person->EmpireId(), bd->EmpireId());
							const char* icons[] =
							{
								":data/BlipCityFriend.png",
								":data/BlipCityNeutral.png",
								":data/BlipCityEnemy.png",
								":data/BlipCityEmpire.png",
								":data/BlipCityEmpire.png",
							};
							img.load(icons[stance]);
						}
					}
					else if(person->IsInVehicle())
					{
						const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
						if(vehicle)
						{
							POLITICAL_STANCE stance = PoliticalStance(person->EmpireId(), vehicle->EmpireId());
							switch(vehicle->CommodityId())
							{
							case CID_DirtBike:
							case CID_SUV:
							case CID_APC:
							case CID_FarmMachinery:
								{
									const char* icons[] =
									{
										":data/BlipVehicleFriend.png",
										":data/BlipVehicleNeutral.png",
										":data/BlipVehicleEnemy.png",
										":data/BlipVehicleEmpire.png",
										":data/BlipVehicleEmpire.png",
									};
									img.load(icons[stance]);
								}
								break;

							case CID_SailBoat:
							case CID_Submarine:
								{
									const char* icons[] =
									{
										":data/BlipBoatFriend.png",
										":data/BlipBoatNeutral.png",
										":data/BlipBoatEnemy.png",
										":data/BlipBoatEmpire.png",
										":data/BlipBoatEmpire.png",
									};
									img.load(icons[stance]);
								}
								break;

							case CID_AdvSpaceFighter:
							case CID_SpaceFighter:
							case CID_SpaceTransport:
							case CID_SpaceRocket:
							case CID_Helicopter:
								{
									const char* icons[] =
									{
										":data/BlipAircraftFriend.png",
										":data/BlipAircraftNeutral.png",
										":data/BlipAircraftEnemy.png",
										":data/BlipAircraftEmpire.png",
										":data/BlipAircraftEmpire.png",
									};
									img.load(icons[stance]);
								}
								break;

							default:
								break;
							}
						}
					}
					else
					{
						// Just the person.
						const char* icons[] =
						{
							":data/BlipPersonEmpire.png",			// Not tied to a ship.
							":data/BlipCrewMateEmpire.png",
							":data/BlipCrewMateEmpire.png",
							":data/BlipCrewMateEmpire.png",
							":data/BlipCrewMateEmpire.png",
							":data/BlipCrewMateNeutral.png",		// Not valid for a person
							":data/BlipPassengerNeutral.png",		// Not valid for a person
							":data/BlipTroopEmpire.png",
							":data/BlipPassengerEmpire.png",
						};
						img.load(icons[person->SpawnShipBerthType()]);
					}

					if(img.isNull())
						img.load(":data/BlipPersonEmpire.png");

					if(AuTime::CurrentSecIsOdd())
					{
						int targSize = m_Scale * 8;
						QRect targ(scx - targSize/2, scy - targSize/2, targSize, targSize);
						QRect src(0,0,img.width(),img.height());

						// Draw lighted spot to help icon stand out.
						QImage imgSpot(":data/BrightSpot.png");
						QRect targSpot(scx - targSize*3/2, scy - targSize*3/2, targSize*3, targSize*3);
						QRect srcSpot(0,0,imgSpot.width(),imgSpot.height());
						p.drawImage(targSpot, imgSpot, srcSpot);

						// Draw icon.
						p.drawImage(targ, img, src);
					}
				}

				if(!m_FollowPerson)
				{
					// Draw crosshair at map center location when we're not following the person's location.
					int scx = round((float)mapx + dispwid/2 * m_Scale + m_Scale/2);
					int scy = round((float)mapy + mypos*m_Scale + m_Scale/2);	// Scaled and centered in its map cell.
					p.setPen(QPen(QColor(32,255,32,255), 0.f));
					p.drawLine(mapx,scy, scx-m_Scale,scy);
					p.drawLine(mapx+szx,scy, scx+m_Scale,scy);
					p.drawLine(scx,mapy, scx,scy-m_Scale);
					p.drawLine(scx,mapy+szy, scx,scy+m_Scale);
				}

				resourceZone = llworld->ResourceZone(m_CenterLong);
				isExplored = llworld->FogOfWarTest(person->Id(), person->EmpireId(), mapLL);

				AuTerrainInfo ter;
				llworld->QueryTerrain(ter, mapLL);
				if(ter.IsGasGiant())
				{
					// Determine terrain type.
					QString terTypes[TerrainTypes] =
					{
						QString("Crust"),						// HighlandForest
						QString("Crust"),						// HighlandSavanna
						QString("Crust"),						// HighlandDesert
						QString("Lithosphere"),					// LowlandForest
						QString("Lithosphere"),					// LowlandSavanna
						QString("Lithosphere"),					// LowlandDesert
						QString("Mantle"),						// SeaLevelForest
						QString("Mantle"),						// SeaLevelSavanna
						QString("Mantle"),						// SeaLevelDesert
						QString("Core"),						// Underwater
					};
					terType = terTypes[ter.TerrainType()];
				}
				else
				{
					// Determine if the terrain is polar.
					float iPolarLevel = llworld->PolarLatitude();
					if(iPolarLevel < PIf/2 && fabs(m_CenterLat) > iPolarLevel)
						latZone = tr("Polar");
					else
					{
						float iTropicLevel = llworld->TropicalLatitude();
						if(fabs(m_CenterLat) <= iTropicLevel)
							latZone = tr("Tropical");
						else
							latZone = tr("Temperate");
					}

					// Determine terrain type.
					QString terTypes[TerrainTypes] =
					{
						QString("Highland"),					// HighlandForest
						QString("Highland"),					// HighlandSavanna
						QString("Highland"),					// HighlandDesert
						QString("Lowland"),						// LowlandForest
						QString("Lowland"),						// LowlandSavanna
						QString("Lowland"),						// LowlandDesert
						QString("Sea Level"),					// SeaLevelForest
						QString("Sea Level"),					// SeaLevelSavanna
						QString("Sea Level"),					// SeaLevelDesert
						QString("Below Sea Level"),				// Underwater
					};
					terType = terTypes[ter.TerrainType()];
				}
				double alt = llworld->TerrainAltitudeMSL(mapLL) / FeetPerMeter();
				terType += QString(", %L1m MSL, Foliage %2")
					.arg(alt, 0,'f',2)
					.arg(ter.m_byFoliage);

				// Development present
				DaBd* bd = llworld->FindBdContaining(llworld->LocOfLL(mapLL), FIND_SkipLandscapers);
				if(bd)
				{
					// Start with city name and assigned name, if any.
					devType = bd->CityName();
					QString name = bd->AssignedName();

					// Empire name
					QString empName;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName(bd->EmpireId(), empName);
					QString indName = gd.IndustryName(bd->IndustryId());
					g_Au.UnlockData();

					if(!devType.isEmpty() && !name.isEmpty())
						devType = QString("%1 %2, %3, %4")
							.arg(devType)	// city name
							.arg(indName)
							.arg(empName)
							.arg(name);
					else if(!devType.isEmpty())
						devType = QString("%1, %2, %3")
							.arg(devType)	// city name
							.arg(indName)
							.arg(empName);
					else if(!name.isEmpty())
						devType = QString("%1, %2, %3")
							.arg(indName)
							.arg(empName)
							.arg(name);
					else 
						devType = QString("%1, %2")
							.arg(indName)
							.arg(empName);
					devColor = bd->MapColor();
				}
			}
		}
		else
		{
			m_PlanetId = AuId(0);								// Not at a planet with a surface image. Could be gas giant or ring.
			m_SunlightMap = QImage();							// Sunlight map is no longer valid.
		}
	}
	else
	{
		m_IsCrew = false;
		m_IsNewbieEmperor = false;
	}
	pView->UnlockSceneAtom();

	// Draw compass image with a drop shadow.
	QImage imcompassshad(":data/CompassShadow.png");
	QImage imcompass(":data/Compass.png");
	imcompassshad = imcompassshad.scaledToWidth(m_CompassSize, Qt::SmoothTransformation);
	imcompass = imcompass.scaledToWidth(m_CompassSize, Qt::SmoothTransformation);
	int compx = m_CompassLoc.x();
	int compy = m_CompassLoc.y();
	if(compx < 0 || compy < 0)
	{
		compx = width() - imcompass.width()/2 - 4;
		compy = imcompass.height()/2 + 4;
	}
	p.translate(compx+5, compy+5);
	p.rotate(-rheading*RAD2DEG);
	p.drawImage(-imcompassshad.width()/2,-imcompassshad.height()/2, imcompassshad);
	p.setTransform(QTransform());	// Reset to identity matrix.
	p.translate(compx, compy);
	p.rotate(-rheading*RAD2DEG);
	p.drawImage(-imcompass.width()/2,-imcompass.height()/2, imcompass);
	p.setWorldMatrixEnabled(false);

	// Location name and optional long/lat coordinates.
	p.setFont(QFont(g_Au.TypefaceSans(),12));
	const int lineht = p.fontMetrics().height();
	int ypos = p.fontMetrics().ascent();
	QColor textColor;
	if(m_FollowPerson)
		textColor = QColor(255,255,0);
	else
		textColor = QColor(32,255,32);

	// System or Planet name
	p.setPen(QPen(Qt::black, 0.f));
	p.drawText(5,ypos+1, m_PlanetName);
	p.setPen(QPen(textColor, 0.f));
	p.drawText(4,ypos, m_PlanetName);

	// Planet location.
	if(!slonglat.isEmpty())
	{
		ypos += lineht;
		p.setPen(QPen(Qt::black, 0.f));
		p.drawText(5,ypos+1, slonglat);
		p.setPen(QPen(textColor, 0.f));
		p.drawText(4,ypos, slonglat);
	}

	// Start adding text lines up from the bottom.
	ypos = height() - 5;

	if(isExplored)
	{
		// City development
		devColor.setAlpha(255);
		if(!devType.isEmpty())
		{
			p.setPen(QPen(Qt::black, 0.f));
			p.drawText(5,ypos+1, devType);
			p.setPen(QPen(devColor, 0.f));
			p.drawText(4,ypos, devType);
			ypos -= lineht;
		}

		// Terrain Type
		if(!terType.isEmpty())
		{
			p.setPen(QPen(Qt::black, 0.f));
			p.drawText(5,ypos+1, terType);
			p.setPen(QPen(textColor, 0.f));
			p.drawText(4,ypos, terType);
			ypos -= lineht;
		}
	}
	else
	{
		// Unexplored terrain
		QString unex(tr("Unexplored"));

		p.setPen(QPen(Qt::black, 0.f));
		p.drawText(5,ypos+1, unex);
		p.setPen(QPen(textColor, 0.f));
		p.drawText(4,ypos, unex);
		ypos -= lineht;
	}

	// Resource zone
	if(resourceZone >= 0 && !latZone.isEmpty())
	{
		QString rzone = tr("Zone %1, %2").arg(resourceZone+1).arg(latZone);
		p.setPen(QPen(Qt::black, 0.f));
		p.drawText(5,ypos+1, rzone);
		p.setPen(QPen(textColor, 0.f));
		p.drawText(4,ypos, rzone);
		ypos -= lineht;
	}

	// Planet Rotation Direction
	if(!isPlanetoid)
	{
		p.setFont(QFont(g_Au.TypefaceSans(),8));
		QString rdir = tr("Direction of Rotation");
		QRect textrect = p.fontMetrics().boundingRect(0,0,1,1, Qt::TextSingleLine|Qt::TextDontClip, rdir);

		int cenx = width()/2;
		int xpos = cenx - textrect.width()/2;
		ypos = lineht;
		p.setPen(QPen(Qt::black, 0.f));
		p.drawText(xpos+1,ypos+1, rdir);
		p.setPen(QPen(QColor(32,255,32), 0.f));
		p.drawText(xpos,ypos, rdir);

		ypos += 1;	// drawImage wants top coord, not bottom like drawText.
		if(rotdir == 1)
		{
			QImage arrow(":data/LeftToRight.png");
			QRect arect(cenx - arrow.width()/4, ypos, arrow.width()/2, arrow.height()/2);
			p.drawImage(arect, arrow);
		}
		else
		{
			QImage arrow(":data/RightToLeft.png");
			QRect arect(cenx - arrow.width()/4, ypos, arrow.width()/2, arrow.height()/2);
			p.drawImage(arect, arrow);
		}
	}

	// Magnification control
	QImage magnifier(":data/Magnifier.png");
	{
		// Pick up the dimensions of the magnifier button.
		m_Magnifier = QRect(0, height()/2-magnifier.height()/2, magnifier.width(), magnifier.height());
		p.drawImage(m_Magnifier, magnifier);
	}

	// Compass resizer control
	{
		// Pick up the dimensions of the magnifier button.
		QImage compass(":data/Compass.png");
		m_CompMagnifier = QRect(width()-magnifier.width(), height()/2-magnifier.height()/2, magnifier.width(), magnifier.height());
		p.drawImage(m_CompMagnifier, compass);
	}

	p.end();
	
	QWidget::paintEvent(pe);
}

void AuLocatorMapWidget::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	m_LeftButtonMode = 0;
	m_MouseDownPt = e->pos();
	if(e->button() == Qt::LeftButton)
	{
		if(m_Magnifier.contains(e->pos()))
		{
			setCursor(QCursor(Qt::SizeVerCursor));
			m_LeftButtonMode = 2;
			m_MagnifierScale = m_Scale;							// Capture current scale before we start messing with it.
		}
		else if(m_CompMagnifier.contains(e->pos()))
		{
			setCursor(QCursor(Qt::SizeVerCursor));
			m_LeftButtonMode = 3;
		}
		else
		{
			setCursor(QCursor(Qt::ClosedHandCursor));
			m_LeftButtonMode = 1;
		}
	}
}

void AuLocatorMapWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
	QWidget::mouseDoubleClickEvent(e);
	if(e->button() == Qt::LeftButton)
	{
		// Ignore double-click in the magnifier.
		if(m_Magnifier.contains(e->pos()))
			return;

		// Ignore double-click in the compass resizer.
		if(m_CompMagnifier.contains(e->pos()))
			return;

		if(IsAtPlanet())
		{
			// Stop following the person. We could change our mind on this.
			m_FollowPerson = false;

			// Calculate lat of mouse click location.
			double pxperlly = (double)m_PlanetLatDivs / PI;	// Surface image pixels per radian.
			double dy = ((m_MouseDownPt.y() - height()/2) / m_Scale) /pxperlly;
			if(fabs(dy) >= PIf/2)
				m_FollowPerson = true;
			else
				LookAt(m_MouseDownPt);
		}
	}
}

// NOTE: We only get this event while a button is held down.
void AuLocatorMapWidget::mouseMoveEvent(QMouseEvent* e)
{
	QWidget::mouseMoveEvent(e);
	if(m_LeftButtonMode == 1)
	{
		if(IsAtPlanet())
		{
			// Stop following the person.
			m_FollowPerson = false;

			// Calculate lat long of mouse click location.
			double posLong, posLat;
			double pxperllx = (double)m_PlanetLonDivs / (PI*2);	// Surface image pixels per radian.
			double pxperlly = (double)m_PlanetLatDivs / PI;		// Surface image pixels per radian.
			double dx = ((m_MouseDownPt.x() - e->pos().x()) / m_Scale) /pxperllx;
			double dy = ((m_MouseDownPt.y() - e->pos().y()) / m_Scale) /pxperlly;
			//qDebug() << "mouseMove" << dx << dy << (dx*RAD2DEG) << (dy*RAD2DEG)
				//<< "center" << (m_CenterLong*RAD2DEG) << (m_CenterLat*RAD2DEG) << "scale" << m_Scale
				//<< "pxperll" << pxperll;

			posLong = m_CenterLong + dx;
			if(posLong < -PI)
				posLong += PI*2;
			else if(posLong >= PI)
				posLong -= PI*2;

			posLat = m_CenterLat - dy;
			if(posLat >= PI/2)
				posLat = PI/2;
			else if(posLat < -PI/2)
				posLat = -PI/2;

			if(posLong != m_CenterLong || posLat != m_CenterLat)
			{
				m_CenterLong = posLong;
				m_CenterLat = posLat;
				update();
			}

			m_MouseDownPt = e->pos();
		}
	}
	else if(m_LeftButtonMode == 2)
	{
		float oldScale = m_Scale;

		int dy = m_MouseDownPt.y() - e->pos().y();
		//m_Scale += .01f * dy;
		if(dy > 0)
		{
			// Zooming in
			if(m_Scale <= 10)
			{
				// Max zoom is at top of widget.
				// Interpolate between current zoom at center and max at top.
				float sclRange = 10.f - m_MagnifierScale;
				float sclPerPixel = sclRange / m_MouseDownPt.y();
				m_Scale = m_MagnifierScale + sclPerPixel * dy;
			}
		}
		else
		{
			// Zooming out
			if(m_Scale >= .5f)
			{
				// Min zoom is at bottom of widget.
				// Interpolate between current zoom at center and min at bottom.
				float sclRange = m_MagnifierScale - .5f;
				float sclPerPixel = sclRange / (height() - m_MouseDownPt.y());
				m_Scale = m_MagnifierScale + sclPerPixel * dy;
			}
		}

		// Restrict range of scale.
		if(m_Scale > 10)
			m_Scale = 10;
		else if(m_Scale < .5f)
			m_Scale = .5f;
		
		if(m_Scale != oldScale)
			update();
	}
	else if(m_LeftButtonMode == 3)
	{
		int newSize = m_CompassSize;

		newSize += m_MouseDownPt.y() - e->pos().y();
		if(newSize < 0)
			newSize = 0;
		else if(newSize > 200)
			newSize = 200;
		
		if(m_CompassSize != newSize)
			CompassSizeChanged(newSize);
	}
}

void AuLocatorMapWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(m_Cursor);
	m_LeftButtonMode = 0;
	QWidget::mouseReleaseEvent(e);

	if(e->button() == Qt::RightButton)
	{
		const DgData& gd = g_Au.LockDataForRead();
		bool isAgent = gd.ClientPlayerIsAgent();
		g_Au.UnlockData();

		QMenu menu(this);

		QAction* teleport;
		QAction* teleportWithRide;
		if(isAgent)
		{
			teleport = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport There"));
			teleportWithRide = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport There With My Ride"));
		}
		/*
		else if(m_IsNewbieEmperor)
		{
			teleport = 0;
			teleportWithRide = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport There"));
		}
		*/
		else
		{
			teleport = 0;
			teleportWithRide = 0;
		}

		QAction* followMe = menu.addAction(QIcon(":data/c_VacSuitHelmet.png"), tr("Track My Location"));

		QAction* lookAt = menu.addAction(QIcon(":data/MsgSpot.png"), tr("Look At"));

		QAction* moveCompass = menu.addAction(QIcon(":data/Locator.png"), tr("Move Compass"));

		QAction* toggleSunlight = menu.addAction(QIcon(":data/c_Sunlight.png"), tr("Toggle Sunlight"));

		QAction* hoverAt = 0;
		QAction* landAt = 0;
		QAction* orbitAt = 0;
		QAction* beamOver = 0;
		QHash<AuId, QAction*> landAtCities;
		AuMissionOrderGoToHoverSite hoverOrder;
		AuMissionOrderGoToLandingSite landOrder;
		AuMissionOrderGoToOrbitCoords orbitOrder;
		AuGlobeLLF globeLL;
		if(!auApp->IsSingleUserDesignerMode() && IsAtPlanet())
		{
			// Determine the longitude and latitude of the click point.
			globeLL = PlanetLongLat(e->pos());

			if(m_IsCrew)
			{
				// Travel to coordinates orders
				menu.addSeparator();

				orbitOrder = AuMissionOrderGoToOrbitCoords(m_PlanetId, m_PlanetName, globeLL);
				orbitAt = menu.addAction(QIcon(orbitOrder.ListNameIcon()), orbitOrder.ListDescription());
				//orbitAt->setToolTip(AuMissionOrder::ListToolTip(AuMissionOrder::GoToOrbitCoords));

				hoverOrder = AuMissionOrderGoToHoverSite(m_PlanetId, m_PlanetName, globeLL, 100);
				hoverAt = menu.addAction(QIcon(hoverOrder.ListNameIcon()), hoverOrder.ListDescription());
				//hoverAt->setToolTip(AuMissionOrder::ListToolTip(AuMissionOrder::GoToHoverSite));

				landOrder = AuMissionOrderGoToLandingSite(m_PlanetId, m_PlanetName, globeLL);
				landAt = menu.addAction(QIcon(landOrder.ListNameIcon()), landOrder.ListDescription());
				//landAt->setToolTip(AuMissionOrder::ListToolTip(AuMissionOrder::GoToLandingSite));
			}

			menu.addSeparator();

			// Look for cities.
			AuIdList cityIds;
			QStringList cityNames;
			QList<quint32> cityEmpires;
			g_AuClient.StarMapListCitiesAtPlanet(m_SystemId, m_PlanetId, cityIds, cityNames, cityEmpires);
			if(!cityIds.isEmpty())
			{
				AuMissionOrderGoToCityAnywhere goToCityAny;
				QMenu* landAtMenu = menu.addMenu(QIcon(goToCityAny.ListNameIcon()), goToCityAny.ListName());
				//landAtMenu->setToolTip(AuMissionOrder::ListToolTip(AuMissionOrder::GoToCityAnywhere));

				AuIdList::const_iterator ii = cityIds.constBegin();
				QStringList::const_iterator ni = cityNames.constBegin();
				QList<quint32>::const_iterator ei = cityEmpires.constBegin();
				while(ii != cityIds.constEnd())
				{
					// Get the city's empire data record, if it's available.
					DgEmpire cityEmpire;
					cityEmpire.m_id = *ei;
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.Empire(cityEmpire);
					g_Au.UnlockData();

					if(hasEmpire)
					{
						QString nameWithEmp = cityEmpire.Name() + " " + *ni;

						QPixmap empireFlag;
						empireFlag.convertFromImage(cityEmpire.m_flag);
						QIcon flagIcon(empireFlag);

						QAction* landAtCity = landAtMenu->addAction(flagIcon, nameWithEmp);
						landAtCities.insert(*ii, landAtCity);
					}
					else
					{
						QAction* landAtCity = landAtMenu->addAction(QIcon(":data/City.png"), *ni);
						landAtCities.insert(*ii, landAtCity);
					}

					ii++;
					ni++;
					ei++;
				}
			}

			if(m_CanBeamVehicle)
			{
				menu.addSeparator();

				beamOver = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Vehicle Transporter Options..."));
			}
			else if(m_CanBeamOver)
			{
				menu.addSeparator();

				beamOver = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Transporter Options..."));
			}
		}

		QAction* action = menu.exec(QCursor::pos());
		if(!action)
			return;

		if(action == teleport)
		{
			TeleportTo(globeLL, false);
		}
		else if(action == teleportWithRide)
		{
			TeleportTo(globeLL, true);
		}
		else if(action == followMe)
		{
			m_FollowPerson = true;
			update();
		}
		else if(action == lookAt)
		{
			LookAt(e->pos());
		}
		else if(action == moveCompass)
		{
			m_CompassLoc = e->pos();
			update();
		}
		else if(action == toggleSunlight)
		{
			m_SunlightEnable = !m_SunlightEnable;
			g_Au.Settings().writeEntry("Widget/LocatorMap/ShowSunlight", m_SunlightEnable);
			update();
		}
		else if(action == hoverAt)
		{
			auApp->MissionAppendOrder(hoverOrder);
		}
		else if(action == landAt)
		{
			auApp->MissionAppendOrder(landOrder);
		}
		else if(action == orbitAt)
		{
			auApp->MissionAppendOrder(orbitOrder);
		}
		else if(action == beamOver)
		{
			BeamOver(globeLL);
		}
		else
		{
			// Check cities
			for(QHash<AuId, QAction*>::const_iterator ci = landAtCities.constBegin(); ci != landAtCities.constEnd(); ci++)
			{
				if(action == *ci)
				{
					AuMissionOrderGoToCityAnywhere order(ci.key(), (*ci)->text());
					auApp->MissionAppendOrder(order);
					break;
				}
			}
		}
	}
}

void AuLocatorMapWidget::LookAtLL(const AuGlobeLLF& ll)
{
	QPoint pos = PlanetLongLatPos(ll);
	LookAt(pos);
}

void AuLocatorMapWidget::LookAt(const QPoint& pos)
{
	if(IsAtPlanet())
	{
		// Stop following the person. We could change our mind on this.
		m_FollowPerson = false;

		// Calculate lat long of mouse click location.
		double posLong, posLat;
		double pxperllx = (double)m_PlanetLonDivs / (PI*2);	// Surface image pixels per radian.
		double pxperlly = (double)m_PlanetLatDivs / PI;		// Surface image pixels per radian.
		double dx = ((pos.x() - width()/2) / m_Scale) /pxperllx;
		double dy = ((pos.y() - height()/2) / m_Scale) /pxperlly;

		posLong = m_CenterLong + dx;
		if(posLong < -PI)
			posLong += PI*2;
		else if(posLong >= PI)
			posLong -= PI*2;

		posLat = m_CenterLat - dy;
		if(posLat >= PI/2)
			posLat = PI/2;
		else if(posLat < -PI/2)
			posLat = -PI/2;

		if(posLong != m_CenterLong || posLat != m_CenterLat)
		{
			m_CenterLong = posLong;
			m_CenterLat = posLat;
			update();
		}
	}
}

// Calculate lat long of pos.
AuGlobeLLF AuLocatorMapWidget::PlanetLongLat(const QPoint& pos) const
{
	if(!IsAtPlanet())
		return AuGlobeLLF(0,0);

	float posLong, posLat;
	float pxperllx = (float)m_PlanetLonDivs / (PIf*2.f);	// Surface image pixels per radian.
	float pxperlly = (float)m_PlanetLatDivs / PIf;		// Surface image pixels per radian.
	float dx = ((pos.x() - width()/2.f) / m_Scale) /pxperllx;
	float dy = ((pos.y() - height()/2.f) / m_Scale) /pxperlly;

	posLong = m_CenterLong + dx;
	if(posLong < -PIf)
		posLong += PIf*2.f;
	else if(posLong >= PIf)
		posLong -= PIf*2.f;

	posLat = m_CenterLat - dy;
	if(posLat >= PIf/2.f)
		posLat = PIf/2.f;
	else if(posLat < -PIf/2.f)
		posLat = -PIf/2.f;

	//qDebug() << "AuLocatorMapWidget::PlanetLongLat" << "dxy" << dx << dy << "latlong" << posLat*RAD2DEGf << posLong*RAD2DEGf;
	return AuGlobeLLF(posLong, posLat);
}

// Calculate pos of lat long.
QPoint AuLocatorMapWidget::PlanetLongLatPos(const AuGlobeLLF& ll) const
{
	if(!IsAtPlanet())
		return QPoint(0,0);

	float dx = ll.Longitude() - m_CenterLong;
	float dy = m_CenterLat - ll.Latitude();
	float pxperllx = (float)m_PlanetLonDivs / (PIf*2.f);	// Surface image pixels per radian.
	float pxperlly = (float)m_PlanetLatDivs / PIf;		// Surface image pixels per radian.
	int px = round(dx * pxperllx * m_Scale + width()/2.f);
	int py = round(dy * pxperlly * m_Scale + height()/2.f);
	return QPoint(px,py);
}

void AuLocatorMapWidget::wheelEvent(QWheelEvent *e)
{
	float oldScale = m_Scale;
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	if(e->angleDelta().y() > 0)
		m_Scale += .5f;
	else if(e->angleDelta().y() < 0)
		m_Scale -= .5f;
#else
	if(e->delta() > 0)
		m_Scale += .5f;
	else if(e->delta() < 0)
		m_Scale -= .5f;
#endif
	if(m_Scale > 10)
		m_Scale = 10;
	else if(m_Scale < .5f)
		m_Scale = .5f;

	QWidget::wheelEvent(e);
	
	if(m_Scale != oldScale)
		update();
}

void AuLocatorMapWidget::CompassSizeChanged(int size)
{
	if(isVisible())
	{
		SetCompassSize(size);
		update();
		g_Au.Settings().writeEntry("Widget/LocatorMap/CompassSize", m_CompassSize);
	}
}

void AuLocatorMapWidget::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	if(visible)
	{
		SetCompassSize(g_Au.Settings().readNumEntry("Widget/LocatorMap/CompassSize", 64));
		emit CompassSizeRestored(m_CompassSize);
	}
}

void AuLocatorMapWidget::BeamOver(const AuGlobeLLF& ll)
{
	//qDebug() << "AuCommChannel::BeamOver" << ll.ToString();
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	// In a transporter room, the player may be eligible for advanced transporter destination options.
	bool inTransporterRoom = false;
	bool onVehicle = false;
	VEHICLE_SPOT_TYPE spotType = SPOT_Invalid;	// Vehicle parking spot type for vehicle transport.
	AuId opId;	// Id of AI crewman manning the transporter console in the room, if any. Can be null. Ship id for vehicle trans.
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		onVehicle = person->IsAboardVehicle();
		const DaCapsule* ship = person->FindParentCapsuleInEnv();
		if(ship)
		{
			if(onVehicle)
			{
				// Check for vehicle that can be transported.
				spotType = person->InVehicleTransporter();
				if(spotType != SPOT_Invalid)
				{
					// Person is in a vehicle that can be transported from the ship.
					opId = ship->Id();					// Ship handles vehicle beam down messages.
				}
			}
			else if(person->InTransporterRoom())
			{
				// Person is aboard a ship in a transporter room.
				inTransporterRoom = true;
				DaSentient* crew = ship->FindTransporterOperator(person->CapsuleRoomId());
				if(crew)
					opId = crew->Id();					// Need AI id to route message specifically to them, like an order.
			}
		}
	}
	view->UnlockSceneAtom();

	if(onVehicle && spotType != SPOT_Invalid)
	{
		// Offer vehicle transport options. It's ok if the opId is null; a human could process this command also.
		AuBeamVehicleOptions dlg(pFrame, ll);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			switch(dlg.BeamDestination())
			{
			case 0:	// Atom
				{
					AuCommBeamVehicleAtom msg(opId, spotType, 0, dlg.CityId(), dlg.CityName(), dlg.Target(),
							AuCommStdChannel::HailChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					auApp->SendChatMessage(msg);
				}
				break;
			case 1:	// Coordinate
				{
					AuCommBeamVehicleCoordinate msg(opId, spotType, 0, dlg.CoordX(), dlg.CoordY(), dlg.CoordZ(),
							AuCommStdChannel::HailChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					auApp->SendChatMessage(msg);
				}
				break;
			case 2:	// Planet
				{
					AuCommBeamVehiclePlanet msg(opId, spotType, 0, dlg.Longitude(), dlg.Latitude(),
							AuCommStdChannel::HailChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					auApp->SendChatMessage(msg);
				}
				break;
			case 3:	// Ship
				{
					AuCommBeamVehicleShip msg(opId, spotType, 0, dlg.ShipId(), dlg.ShipName(),
							AuCommStdChannel::HailChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					auApp->SendChatMessage(msg);
				}
				break;
			case 4:	// Transporter
				{
					AuCommBeamVehicleTransporter msg(opId, spotType, 0, dlg.TransporterId(),
							dlg.TransporterName(), dlg.TransporterWhich(),
							AuCommStdChannel::HailChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					auApp->SendChatMessage(msg);
				}
				break;
			default:
				break;
			}
		}
	}
	else if(inTransporterRoom)
	{
		// Offer transport options. It's ok if the opId is null; a human could process this command also.
		AuBeamOptions dlg(pFrame, ll);
		int result = auApp->Frame()->ExecDialog(dlg);
		if(result == QDialog::Accepted)
		{
			switch(dlg.BeamDestination())
			{
			case 0:	// Atom
				{
					AuCommBeamAtom msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.CityId(), dlg.CityName(), dlg.Target(),
							AuCommStdChannel::VoiceChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					auApp->SendChatMessage(msg);
				}
				break;
			case 1:	// Coordinate
				{
					AuCommBeamCoordinate msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.CoordX(), dlg.CoordY(), dlg.CoordZ(),
							AuCommStdChannel::VoiceChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					auApp->SendChatMessage(msg);
				}
				break;
			case 2:	// Planet
				{
					AuCommBeamPlanet msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.Longitude(), dlg.Latitude(),
							AuCommStdChannel::VoiceChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					auApp->SendChatMessage(msg);
				}
				break;
			case 3:	// Ship
				{
					AuCommBeamShip msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.ShipId(), dlg.ShipName(),
							AuCommStdChannel::VoiceChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					auApp->SendChatMessage(msg);
				}
				break;
			case 4:	// Transporter
				{
					AuCommBeamTransporter msg(opId, dlg.BeamCount(), dlg.BeamDirection(), dlg.TransporterId(),
							dlg.TransporterName(), dlg.TransporterWhich(),
							AuCommStdChannel::VoiceChannelName(),
							view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
							view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					auApp->SendChatMessage(msg);
				}
				break;
			default:
				break;
			}
		}
	}
}

void AuLocatorMapWidget::TeleportTo(const AuGlobeLLF& ll, bool rideAlso)
{
	auApp->AgentGoToLongLat(ll, rideAlso);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLocatorMap::AuLocatorMap(QWidget* parent) : QWidget(parent)
{
	m_Map = new AuLocatorMapWidget(this);

	m_Places = new QTableWidget(this);
	m_Places->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_Places->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_Places->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Places->setShowGrid(false);
	QStringList hls;
	hls << tr("Place") << tr("Location");
	m_Places->setColumnCount(hls.count());
	m_Places->setHorizontalHeaderLabels(hls);
	m_Places->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	m_Places->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	m_Places->setColumnWidth(0, 150);
	m_Places->setColumnWidth(2, 200);
	QHeaderView* vhdr = m_Places->verticalHeader();
	vhdr->hide();
	connect(m_Places, SIGNAL(itemSelectionChanged()), SLOT(PlaceSelectionChanged()));

	// Assemble the widgets into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->addWidget(m_Map);
	m_Splitter->addWidget(m_Places);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/LocatorMap/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);
}

AuLocatorMap::~AuLocatorMap()
{
}

void AuLocatorMap::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/LocatorMap/Splitter", m_Splitter->saveState());
}

void AuLocatorMap::PlaceSelectionChanged()
{
	QItemSelectionModel* ism = m_Places->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* item = m_Places->item((*si).row(), 0);
				if(item)
				{
					AuGlobeLLF ll = item->data(Qt::UserRole).value<AuGlobeLLF>();
					m_Map->LookAtLL(ll);
				}
				break;
			}
	}
}

void AuLocatorMap::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);
	// Update places list.
	m_Places->clearContents();
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaWorldLL* world = person->FindParentWorldInEnv();
		if(world)
		{
			m_Places->setSortingEnabled(false);
			// Add the world claim.
			if(world->ClaimedByEmpire())
			{
				QString claimE;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(world->ClaimedByEmpire(), claimE);
				g_Au.UnlockData();
				QString claim = tr("Claimed by %1").arg(claimE);
				AddPlace(TTIcon(TT_ClaimFlag), claim, world->ClaimedAtLL());
			}

			// Add cities and key buildings on the world.
			DaAtomList subs;
			world->ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaAtom* atom = *it;
				QString name;
				AuGlobeLLF ll;
				const DaBd* bd = dynamic_cast<const DaBd*>(atom);
				if(bd)
				{
					switch(bd->IndustryId())
					{
					case IID_AirportTerminal:
					case IID_BaseAirTerminal:
					case IID_BaseHeadquarters:
					case IID_Capitol:
						AddPlace(TTIcon(TT_Bd), bd->CommName(), bd->GlobeLL());
						break;
					default:
						continue;
					}
				}
			}

			// Add places of interest.
			const AuGeologicalFeatures& geof = world->LockGeologicalFeatures();
			for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
			{
				if(!(*gi)->IsTransporterTarget())
					continue;
				AddPlace(TTIcon(TT_GeologicalFeatureFirst), (*gi)->TransporterName(world->HydrographicLiquid()), (*gi)->LL());
			}
			world->UnlockGeologicalFeatures();

			// Ringworld control center.
			const DaWorldRingSection* rw = dynamic_cast<const DaWorldRingSection*>(world);
			if(rw && rw->SectionIndex() == 0)
				AddPlace(TTIcon(TT_RingworldControlCenter), tr("Ringworld Control Center"), AuGlobeLLF(0,0));
			m_Places->setSortingEnabled(true);
		}
	}
	pView->UnlockSceneAtom();
}

void AuLocatorMap::AddPlace(const QString& iconName, const QString& placeName, const AuGlobeLLF& placeLL)
{
	m_Places->insertRow(0);

	int col=0;
	QTableWidgetItem* twi;
	twi = new QTableWidgetItem(QIcon(iconName), placeName);
	twi->setData(Qt::UserRole, QVariant::fromValue(placeLL));
	m_Places->setItem(0, col++, twi);

	twi = new QTableWidgetItem(placeLL.ToString());
	m_Places->setItem(0, col++, twi);
}
