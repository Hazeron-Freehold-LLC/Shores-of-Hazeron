//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpconsole.cpp,v 1.150 2022/04/14 16:42:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpconsole.h"
//
#include "auatomrenderinfo.h"
#include "auglobal.h"
#include "auquad.h"
#include "dacreature.h"
#include "dadesignbase.h"
#include "dasolarsystem.h"
#include "davessel.h"
#include "daworld.h"

// These GL models can be reused by every instance so the value is stored statically.
static AuGLVAO							g_BigScreenVAO;			// Console large primary display screen quad.
static GLuint							g_glSeatTexture = 0;	// OpenGL texture id.
static float							g_PanelDistAdj = 0;		// Distance from viewer adjustment, relative to base screen model.
static float							g_PanelHtAdj = 0;		// Height above floor adjustment, relative to base screen model.
static Au3dsGLSLModelInstance*			g_PanelModel = 0;
static AuPoint							g_PanelOff;				// Offset for placement of the screen model.
static float							g_PanelRotAdj = 0;		// Rotation adjustment, relative to the base screen model.
static Au3dsGLSLModelInstance*			g_SeatModel = 0;
static AuPoint							g_SeatOff;				// Offset for placement of the seat model.
static AuGLVAO							g_SmallScreenVAO;		// Console small secondary display screen quad.

DpConsole::DpConsole()
{
	m_glLowerConsole = 0;
	m_LastETESecs = -1;
}

DpConsole::~DpConsole()
{
}

DpConsole::DpConsole(const DpConsole& rhs)
{
	m_glLowerConsole = rhs.m_glLowerConsole;
	m_LastETESecs = rhs.m_LastETESecs;
}

void DpConsole::RenderChair(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari)
{
	if(!g_SeatModel || !g_PanelModel)
		return;
	if(!g_SeatModel->HasGLModel())
		return;

	AuMatrix4x4 tThis(tModel);
	tThis.RotateZ(90*DEG2RAD);
	AuPoint seatOff(g_SeatOff);
	seatOff.x += 1;				// Move the seat back a bit.
	tThis.Translate(-seatOff);

	// Lighting
	AuPoint ambientUp = tModel.ProjectVector(0,0,1);

	// Seat model
	g_SeatModel->SetTexture(g_glSeatTexture);
	g_SeatModel->RenderGLSLModel(projection, tThis, *ari.re, ambientUp, false);
}

void DpConsole::RenderConsole(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, AuAtomRenderInfo& ari,
		GLuint texScreen) const
{
	if(!g_SeatModel || !g_PanelModel)
		return;
	if(!g_SeatModel->HasGLModel())
		return;
	if(!g_PanelModel->HasGLModel())
		return;

	AuMatrix4x4 tThis(tModel);
	tThis.RotateZ(90*DEG2RAD);
	AuPoint seatOff(g_SeatOff);
	seatOff.x += 1;				// Move the seat back a bit.
	tThis.Translate(-seatOff);

	// Lighting
	AuPoint ambientUp = tModel.ProjectVector(0,0,1);

	// Seat model
	g_SeatModel->SetTexture(g_glSeatTexture);
	g_SeatModel->RenderGLSLModel(projection, tThis, *ari.re, ambientUp, false);

	// Screen Model
	{
		AuMatrix4x4 tScreen(tModel);
		tScreen.RotateZ(90*DEG2RAD);
		AuPoint screenOff(g_SeatOff);
		screenOff.x -= g_PanelDistAdj;	// User can adjust screen distance.
		screenOff.z -= g_PanelHtAdj;	// User can adjust screen height.
		tScreen.Translate(-screenOff);

		tScreen.Translate(g_PanelOff);
		tScreen.RotateY(g_PanelRotAdj);	// User can adjust screen rotation.
		tScreen.Translate(-g_PanelOff);

		g_PanelModel->SetTexture(g_glSeatTexture);
		g_PanelModel->RenderGLSLModel(projection, tScreen, *ari.re, ambientUp, false);

		// Display Screens
		if(g_BigScreenVAO.VAO() && g_SmallScreenVAO.VAO())
		{
			// Select texture shader and configure its parameters.
			ari.re->GLSL().TextureShader().Select();
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * tScreen);
			ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

			// Render the primary display screen.
			ari.re->GLSL().TextureShader().SetTexture(texScreen);
			g_BigScreenVAO.SelectGL();
			g_BigScreenVAO.DrawGL();
			g_BigScreenVAO.DeselectGL();

			// Render the secondary display screen.
			ari.re->GLSL().TextureShader().SetTexture(m_glLowerConsole);
			g_SmallScreenVAO.SelectGL();
			g_SmallScreenVAO.DrawGL();
			g_SmallScreenVAO.DeselectGL();

			ari.re->GLSL().TextureShader().Deselect();
		}
	}
}

// Drawn facing the negative Y direction with origin at 0,0.
void DpConsole::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	if(!m_glLowerConsole)
	{
		// Create a texture map from a resource bitmap.
		// Identify it uniquely by parent so all lower consoles on the same vessel show the same data.
		QString name = QString("LCon%1").arg(parent.Id().GetID());
		if(inBuilding)
			m_glLowerConsole = ari.re->CreateTextureFromBitmap(parent.Id().GetID(), name, ":data/BldConsole_Lower.bmp");
		else
			m_glLowerConsole = ari.re->CreateTextureFromBitmap(parent.Id().GetID(), name, ":data/txConsole_Lower.bmp");
	}

	if(!g_SeatModel)
	{
		// Start loading the 3ds model of the seat using a worker thread.
		QString modelFile(":data/mConsoleSeat.3dsN");
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		g_SeatModel = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
		if(g_SeatModel->HasGLModel())
		{
			AuPoint min,max;
			g_SeatModel->GetExtents(min, max);
			g_SeatOff = (min + max)/2;
			g_SeatOff.z = min.z;
		}
	}
	else if(!g_SeatModel->HasGLModel())
	{
		// The 3ds model load was started but the GL model hasn't been created yet.
		// When the model is ready, I can get its extents.
		// GLModel actually creates the open GL models after the worker thread has loaded the necessary data.
		// GLModel returns true when the open GL model is ready.
		if(g_SeatModel->GLModel(*ari.re))
		{
			AuPoint min,max;
			g_SeatModel->GetExtents(min, max);
			g_SeatOff = (min + max)/2;
			g_SeatOff.z = min.z;
		}
	}

	if(!g_PanelModel)
	{
		// Start loading the 3ds model of the seat using a worker thread.
		QString modelFile(":data/mConsoleScreen.3dsN");
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		g_PanelModel = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
		if(g_PanelModel->HasGLModel())
		{
			AuPoint min,max;
			g_PanelModel->GetExtents(min, max);
			g_PanelOff = (min + max)/2;
			g_PanelOff.z = min.z;
		}
	}
	else if(!g_PanelModel->HasGLModel())
	{
		// The 3ds model load was started but the GL model hasn't been created yet.
		// When the model is ready, I can get its extents.
		// GLModel actually creates the open GL models after the worker thread has loaded the necessary data.
		// GLModel returns true when the open GL model is ready.
		if(g_PanelModel->GLModel(*ari.re))
		{
			AuPoint min,max;
			g_PanelModel->GetExtents(min, max);
			g_PanelOff = (min + max)/2;
			g_PanelOff.z = min.z;
		}
	}

	if(g_PanelModel->HasGLModel())
	{
		if(!g_BigScreenVAO.VAO())
		{
			// Primary display screen model.
			AuPoint fmpll, fmpul, fmpur, fmplr;
			g_PanelModel->ScreenVertices(0, fmpll, fmpul, fmpur, fmplr);

			AuTriTex tri0;
			tri0.m_V0 = AuPointTex(fmpll, 0,0);
			tri0.m_V1 = AuPointTex(fmpul, 0,1);
			tri0.m_V2 = AuPointTex(fmpur, 1,1);
			AuTriTex tri1;
			tri1.m_V0 = AuPointTex(fmpur, 1,1);
			tri1.m_V1 = AuPointTex(fmplr, 1,0);
			tri1.m_V2 = AuPointTex(fmpll, 0,0);
			QList<AuTriTex> tris;
			tris << tri0 << tri1;

			g_BigScreenVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
		}

		if(!g_SmallScreenVAO.VAO())
		{
			AuPoint fmpll, fmpul, fmpur, fmplr;
			g_PanelModel->ScreenVertices(1, fmpll, fmpul, fmpur, fmplr);

			AuTriTex tri0;
			tri0.m_V0 = AuPointTex(fmpll, 0,0);
			tri0.m_V1 = AuPointTex(fmpul, 0,1);
			tri0.m_V2 = AuPointTex(fmpur, 1,1);
			AuTriTex tri1;
			tri1.m_V0 = AuPointTex(fmpur, 1,1);
			tri1.m_V1 = AuPointTex(fmplr, 1,0);
			tri1.m_V2 = AuPointTex(fmpll, 0,0);
			QList<AuTriTex> tris;
			tris << tri0 << tri1;

			g_SmallScreenVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
		}
	}

	if(!g_glSeatTexture)
		g_glSeatTexture = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ConTex", ":data/mConsoleTex.png");
}

void DpConsole::DeleteGL()
{
	// DaAtom deletes all textures used by the ship's id
	m_glLowerConsole = 0;
}

void DpConsole::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last, const DaCreature& sceneAtom)
{
	AuEther* ether = ship.Ether();
	if(!ether)
		return;
	// Lower console only needs to be updated once per second.
	if(last.ElapsedSecTicks() == 0)
		return;

	// The lower ship console is the same on every ship console.
	// Only ship is required. Planet and solar system are optional.
	DaAtom* mysys = ship.FindEnvironmentAtom();
	DaWorld* planet;
	if(dynamic_cast<const DaSolarSystem*>(mysys) || dynamic_cast<const DaDesignBase*>(mysys))
		planet = (DaWorld *)ship.FindParentPlanetInEnv();
	else
		planet = 0;

	QString name;
	name = QString("LCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(pTex)
	{
		QRect rc(0,0,122,16);
		QPainter p;

		// System Status
		QPixmap pane(122,16);		// Most sub-panes are 122x16 pixels.
		if(ship.RepairSecondsRemaining())
		{
			// Ship is getting repaired.
			if(last.SecIsOdd())
				pane.load(":data/conRepairs2.png");
			else
				pane.load(":data/conRepairs3.png");
			QRect rc2(0,0,122,16);
			p.begin(&pane);
			p.setFont(QFont(g_Au.TypefaceSans(),8));
			p.setPen(QPen(QColor(0,0,0), 0.f));
			QString statText = QObject::tr("Repairing %1 sec.").arg(ship.RepairSecondsRemaining());
			p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
			rc2 = rc2.translated(-1,-1);
			p.setPen(QPen(QColor(255,255,255), 0.f));
			p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
			p.end();
		}
		else
		{
			// Get the name of the system with the worst damage. This is usually the cause of the alert condition.
			QString worstSystem = ship.WorstDamagedSystem();
			int pctWorstSys;
			if(worstSystem.isEmpty())
			{
				worstSystem = QObject::tr("Systems");
				pctWorstSys = 100;
			}
			else
				pctWorstSys = 100 - ship.WorstSystemDamage();

			// Condition of Worst System
			p.begin(&pane);
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			switch(ship.AlertConditionOfSystems())
			{
			case ConditionGreen:
				p.fillRect(rc, QColor(0,255,0));
				p.setPen(QPen(QColor(0,32,0), 0.f));
				p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
				break;
			case ConditionYellow:
				if(last.SecIsOdd())
					p.fillRect(rc, QColor(255,192,32));
				else
					p.fillRect(rc, QColor(128,96,16));
				p.setPen(QPen(QColor(32,24,3), 0.f));
				p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
				break;
			case ConditionRed:
				if(last.SecIsOdd())
					p.fillRect(rc, QColor(255,0,0));
				else
					p.fillRect(rc, QColor(128,0,0));
				p.setPen(QPen(QColor(32,0,0), 0.f));
				p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, worstSystem);
				break;
			}
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, QString("%1%").arg(pctWorstSys));
			p.end();
		}
		QImage imp = pane.toImage();
		pTex->ReplaceSubImage(re, imp, 131,3);

		// Solar System
		p.begin(&pane);
		p.fillRect(rc, QColor(0,0,0));
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		p.setPen(QPen(QColor(128,128,128), 0.f));
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("System"));
		if(mysys)
		{
			p.setPen(QPen(QColor(200,200,200), 0.f));
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, mysys->Name());
		}
		p.end();
		imp = pane.toImage();
		pTex->ReplaceSubImage(re, imp, 3,45);

		// Planet
		p.begin(&pane);
		p.fillRect(rc, QColor(0,0,0));
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		p.setPen(QPen(QColor(128,128,128), 0.f));
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("World"));
		if(planet)
		{
			p.setPen(QPen(QColor(200,200,200), 0.f));
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, planet->Name());
		}
		p.end();
		imp = pane.toImage();
		pTex->ReplaceSubImage(re, imp, 3,24);

		// Destination
		p.begin(&pane);
		p.fillRect(rc, QColor(0,0,0));
		p.setFont(QFont(g_Au.TypefaceSans(),7));
		p.setPen(QPen(QColor(128,128,128), 0.f));
		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("Dest"));
		if(ship.HasWaypoint())
		{
			const DeWaypoint* wp = ship.LockWaypoint(0);
			if(wp)
			{
				const DaAtom* pwaydest = ether->FindNormalAtom(wp->m_idSystem);
				const DaSolarSystem* pwayss = dynamic_cast<const DaSolarSystem*>(pwaydest);
				if(pwaydest)
				{
					//qDebug("Found waypoint destination solarsystem, destid %s ssid %s", 
							//(const char*)wp->Destination().StringId(),
							//(const char*)pwaydest->Id().StringId());
					AuPoint fwp;
					const DaAtom* pdest = wp->Location(pwaydest, fwp);
					if(pdest)
					{
						if(mysys && mysys->Is(pwaydest->Id()))
						{
							// The next waypoint is in this system.
							if(wp->DestinationOffset().IsZero())
							{
								// The atom is the destination.
								p.setPen(QPen(QColor(200,200,200), 0.f));
								p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, pdest->Name());
							}
							else
							{
								// The atom is a nav point.
								QString navname;
								if(pwayss)
									navname = pwayss->WormholeNameAt(wp->DestinationOffset());
								if(navname.isEmpty())
									navname = QObject::tr("Nav Point");
								p.setPen(QPen(QColor(200,200,200), 0.f));
								p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, navname);
							}
						}
						else if(pwayss)
						{
							//qDebug("Found waypoint destination atom is in another system");
							// The next waypoint is in another system.
							p.setPen(QPen(QColor(100,100,200), 0.f));
							p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, pdest->Name());
						}
					}
				}
			}
			ship.UnlockWaypoint();
		}
		p.end();
		imp = pane.toImage();
		pTex->ReplaceSubImage(re, imp, 131,45);

		int etesecs = ship.ETE();
		if(etesecs != m_LastETESecs && etesecs >= 0)
		{
			m_LastETESecs = etesecs;

			// Break down the ete into hours minutes seconds.
			int hours = (int)(etesecs/(60*60));
			etesecs -= hours*(60*60);
			int mins = (int)(etesecs/60);
			etesecs -= mins*60;
			int secs = round(etesecs);
			if(secs == 60)
			{
				secs = 0;
				mins++;
			}

			QString sete;
			if(hours)
				sete = QString("%1:%2:%3").arg(hours).arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
			else
				sete = QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));

			p.begin(&pane);
			p.fillRect(rc, QColor(0,0,0));
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			p.setPen(QPen(QColor(128,128,128), 0.f));
			p.drawText(rc, Qt::AlignLeft|Qt::AlignTop, QObject::tr("ETE"));
			p.setPen(QPen(QColor(200,200,200), 0.f));
			p.drawText(rc, Qt::AlignRight|Qt::AlignBottom, sete);
			p.end();

			imp = pane.toImage();
			pTex->ReplaceSubImage(re, imp, 131,24);
		}

		// Structure Status
		QPixmap pane2(112,16);		// Structural health sub-pane is 112x16 pixels.
		QRect rc2(0,0,112,16);
		if(ship.RepairSecondsRemaining())
		{
			// Ship is getting repaired.
			if(last.SecIsOdd())
				pane2.load(":data/conRepairs0.png");
			else
				pane2.load(":data/conRepairs1.png");
			p.begin(&pane2);
			p.setFont(QFont(g_Au.TypefaceSans(),8));
			p.setPen(QPen(QColor(0,0,0), 0.f));
			QString statText = QObject::tr("Repairing %1 sec.").arg(ship.RepairSecondsRemaining());
			p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
			rc2 = rc2.translated(-1,-1);
			p.setPen(QPen(QColor(255,255,255), 0.f));
			p.drawText(rc2, Qt::AlignCenter|Qt::AlignVCenter, statText);
			p.end();
		}
		else
		{
			// The ship is not getting repaired.
			unsigned int curHits = ship.HitsDamageStructural();
			unsigned int maxHits = ship.DesignHitsStructural();
			int pctRemaining;
			if(maxHits && curHits < maxHits)
				pctRemaining = Percent((maxHits - curHits), maxHits);	// percent of remaining hit points
			else
				pctRemaining = 0;
			if(!pctRemaining && curHits < maxHits)
				pctRemaining = 1;	// Slightly more than zero percent remaining.
			else if(curHits && pctRemaining == 100)
				pctRemaining = 99;	// Slightly less than 100 percent remaining.

			// These rgb color values came from DaVehicle::RenderBodyStats. These are the colors a vehicle
			// uses to draw its health bar when you spot it with your cursor.
			QColor bgcol, fgcol;
			ALERT_CONDITION ac;
			if(ship.IsAnyConditionVisible())
				ac = ConditionRed;
			else
				ac = ship.AlertConditionOfStructure();
			switch(ac)
			{
			case ConditionGreen:
				bgcol.setRgbF(.04f,.04f,.2f);
				fgcol.setRgbF(.2f,.2f,1.f);
				break;
			case ConditionYellow:
				if(last.SecIsOdd())
				{
					bgcol.setRgbF(.2f,.14f,.04f);
					fgcol.setRgbF(1.f,.7f,.2f);
				}
				else
				{
					bgcol.setRgbF(.1f,.07f,.02f);
					fgcol.setRgbF(.5f,.35f,.1f);
				}
				break;
			case ConditionRed:
				if(last.SecIsOdd())
				{
					bgcol.setRgbF(.2f,.04f,.04f);
					fgcol.setRgbF(1.f,.2f,.2f);
				}
				else
				{
					bgcol.setRgbF(.1f,.02f,.02f);
					fgcol.setRgbF(.5f,.1f,.1f);
				}
				break;
			}

			// Structural Health as a percentage.
			p.begin(&pane2);
			p.fillRect(rc2, bgcol);	// Background is filled with unhealthy color.
			QRect rcstat(rc2);
			int right = rcstat.width() * pctRemaining / 100;
			if(!right && pctRemaining)
				right = 1;
			else if(curHits && right == rcstat.width())
				right = rcstat.width() - 1;
			rcstat.setRight(right);
			p.fillRect(rcstat, fgcol);	// Healthy part of the bar.
			p.setFont(QFont(g_Au.TypefaceSans(),7));
			p.setPen(QPen(QColor(128,128,128), 0.f));
			p.drawText(rc2, Qt::AlignLeft|Qt::AlignTop, QObject::tr("Structure"));
			p.setPen(QPen(QColor(200,200,200), 0.f));
			QString statText = QString("%1%").arg(pctRemaining);
			p.drawText(rc2, Qt::AlignRight|Qt::AlignBottom, statText);
			p.end();
		}
		imp = pane2.toImage();
		pTex->ReplaceSubImage(re, imp, 13,3);

		// Air Pressure Pane
		QPixmap pane3(6,16);	// Air Pressure sub-pane is 6x16 pixels.
		QRect rc3(0,0,6,16);
		QColor bgcol, fgcol;
		int airP = (int)(ship.AirPressure(sceneAtom.ShipRoomName()) * 200.f);
		if(sceneAtom.IsAirBreathable())
		{
			if(airP < 50 || airP > 150)
			{
				// Within 10% of the limits so show yellow.
				if(last.SecIsOdd())
				{
					bgcol.setRgbF(.2f,.14f,.04f);
					fgcol.setRgbF(1.f,.7f,.2f);
				}
				else
				{
					bgcol.setRgbF(.1f,.07f,.02f);
					fgcol.setRgbF(.5f,.35f,.1f);
				}
			}
			else
			{
				// Well inside the comfort zone.
				bgcol.setRgbF(.04f,.2f,.04f);
				fgcol.setRgbF(.2f,1.f,.2f);
			}
		}
		else
		{
			// Air is not breathable.
			if(last.SecIsOdd())
			{
				bgcol.setRgbF(.2f,.04f,.04f);
				fgcol.setRgbF(1.f,.2f,.2f);
			}
			else
			{
				bgcol.setRgbF(.1f,.02f,.02f);
				fgcol.setRgbF(.5f,.1f,.1f);
			}
		}
		p.begin(&pane3);
		p.fillRect(rc3, bgcol);
		QRect rcair(rc3);
		int top = rcair.height() - rcair.height() * airP / 200;
		if(top > 0)
			rcair.setTop(top);
		p.fillRect(rcair, fgcol);	// Healthy part of the bar.
		p.end();
		imp = pane3.toImage();
		pTex->ReplaceSubImage(re, imp, 3,3);
	}
}

void DpConsole::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4& tModel, QVector<qint16>& idPanels) const
{
	if(!g_PanelModel)
		return;

	//qDebug("DpConsole::EnumControlPanels");
	AuMatrix4x4 tScreen(tModel);
	tScreen.RotateZ(90*DEG2RAD);
	AuPoint screenOff(g_SeatOff);
	screenOff.x -= g_PanelDistAdj;	// User can adjust screen distance.
	screenOff.z -= g_PanelHtAdj;	// User can adjust screen height.
	tScreen.Translate(-screenOff);

	tScreen.Translate(g_PanelOff);
	tScreen.RotateY(g_PanelRotAdj);	// User can adjust screen rotation.
	tScreen.Translate(-g_PanelOff);

	// Upper panel.
	AuPoint fmpll, fmpul, fmpur, fmplr;
	g_PanelModel->ScreenVertices(0, fmpll, fmpul, fmpur, fmplr);
	AuQuad qupper(fmpll, fmpul, fmpur, fmplr);
	qupper.Transform(tScreen);
	aControlPanels.append(qupper);
	idPanels.append(0);

	// Lower panel.
	AuPoint flpll, flpul, flpur, flplr;
	g_PanelModel->ScreenVertices(1, flpll, flpul, flpur, flplr);
	AuQuad qlower(flpll, flpul, flpur, flplr);
	qlower.Transform(tScreen);
	aControlPanels.append(qlower);
	idPanels.append(1);
}

void DpConsole::PanelDimensions(quint16 idPanel, double& width, double& height) const
{
	if(!g_PanelModel)
		return;

	if(idPanel == 0)
	{
		// Upper panel.
		AuPoint fmpll, fmpul, fmpur, fmplr;
		g_PanelModel->ScreenVertices(0, fmpll, fmpul, fmpur, fmplr);
		height = fmpll.Distance(fmpul);
		width = fmpll.Distance(fmplr);
	}
	else
	{
		// Lower panel.
		AuPoint flpll, flpul, flpur, flplr;
		g_PanelModel->ScreenVertices(1, flpll, flpul, flpur, flplr);
		height = flpll.Distance(flpul);
		width = flpll.Distance(flplr);
	}
}

//static
void DpConsole::Raise(const AuId& who)
{
	g_PanelHtAdj += .25f/12.f;
	if(g_PanelHtAdj > 3)
		g_PanelHtAdj = 3;

	// Store last change.
	QString section = QString("/PanelAdj/Ht%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelHtAdj);
}

//static
void DpConsole::Lower(const AuId& who)
{
	g_PanelHtAdj -= .25f/12.f;
	if(g_PanelHtAdj < -3)
		g_PanelHtAdj = -3;

	// Store last change.
	QString section = QString("/PanelAdj/Ht%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelHtAdj);
}

//static
void DpConsole::MoveFarther(const AuId& who)
{
	g_PanelDistAdj += .25f/12.f;
	if(g_PanelDistAdj > 3)
		g_PanelDistAdj = 3;

	// Store last change.
	QString section = QString("/PanelAdj/Dist%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelDistAdj);
}

//static
void DpConsole::MoveCloser(const AuId& who)
{
	g_PanelDistAdj -= .25f/12.f;
	if(g_PanelDistAdj < -3)
		g_PanelDistAdj = -3;

	// Store last change.
	QString section = QString("/PanelAdj/Dist%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelDistAdj);
}

//static
void DpConsole::RotateAway(const AuId& who)
{
	g_PanelRotAdj += DEG2RADf;	// 1 degree
	if(g_PanelRotAdj > 90*DEG2RADf)
		g_PanelRotAdj = 90*DEG2RADf;

	// Store last change.
	QString section = QString("/PanelAdj/Rot%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelRotAdj*RAD2DEGf);
}

//static
void DpConsole::RotateToward(const AuId& who)
{
	g_PanelRotAdj -= DEG2RADf;	// 1 degree
	if(g_PanelRotAdj < -90*DEG2RADf)
		g_PanelRotAdj = -90*DEG2RADf;

	// Store last change.
	QString section = QString("/PanelAdj/Rot%1").arg(who.StringId());
	g_Au.Settings().writeEntry(section, g_PanelRotAdj*RAD2DEGf);
}

//static
void DpConsole::RestoreAdjustments(const AuId& who)
{
	QString section = QString("/PanelAdj/Dist%1").arg(who.StringId());
	g_PanelDistAdj = g_Au.Settings().readNumEntry(section, -1.8f);
	//qDebug() << "DpConsole::RestoreAdjustments dist" << g_PanelDistAdj;
	if(fabs(g_PanelDistAdj) > 3.f)
		g_PanelDistAdj = -1.8f;

	section = QString("/PanelAdj/Ht%1").arg(who.StringId());
	g_PanelHtAdj = g_Au.Settings().readNumEntry(section, -1.1f);
	//qDebug() << "DpConsole::RestoreAdjustments ht" << g_PanelHtAdj;
	if(fabs(g_PanelHtAdj) > 3.f)
		g_PanelHtAdj = -1.1f;

	section = QString("/PanelAdj/Rot%1").arg(who.StringId());
	g_PanelRotAdj = g_Au.Settings().readNumEntry(section, 37.f);
	//qDebug() << "DpConsole::RestoreAdjustments rot" << g_PanelRotAdj;
	if(fabs(g_PanelRotAdj) > 90.f)
		g_PanelRotAdj = 37.f * DEG2RADf;
	else
		g_PanelRotAdj *= DEG2RADf;			// convert to radians
}

quint64 DpConsole::MemoryUsage() const
{
	quint64 mem = sizeof(DpConsole);
	return mem;
}
