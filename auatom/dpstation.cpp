//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstation.cpp,v 1.24 2022/04/14 16:43:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstation.h"
//
#include "auatomrenderinfo.h"
#include "ausensorblip.h"
#include "dasector.h"
#include "davessel.h"
#include "dgdata.h"

DpStation::DpStation()
{
	m_BeamStartTex = 0;
	m_NavFixTex = 0;
	m_WaypointTex = 0;
}

DpStation::~DpStation()
{
}

DpStation::DpStation(const DpStation &rhs)
{
	Q_UNUSED(rhs);
	m_BeamStartTex = 0;
	m_NavFixTex = 0;
	m_WaypointTex = 0;
}

DpStation::STATION_TYPE DpStation::ClientRestoreType() const
{
	return STATION_Count;	// This station type does not restore any settings.
}

void DpStation::LoadClientSettings(const AuId& planetId)
{
	Q_UNUSED(planetId);
	Q_ASSERT(ClientRestoreType() != STATION_Count);
}

AuId DpStation::DisplayedSystem() const
{
	return AuId(0);
}

void DpStation::Render(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& tModel,
		const DaPerson& sceneAtom, const DaCreature* seatedCreature, AuAtomRenderInfo& ari)
{
	Q_UNUSED(projection);
	Q_UNUSED(farProjection);
	Q_UNUSED(tModel);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(seatedCreature);
	Q_UNUSED(ari);
}

void DpStation::RenderMini(const AuMatrix4x4& projection, const AuMatrix4x4& tModel, const DaCreature* seatedCreature,
		AuAtomRenderInfo& ari)
{
	Q_UNUSED(projection);
	Q_UNUSED(tModel);
	Q_UNUSED(seatedCreature);
	Q_UNUSED(ari);
}

// Drawn facing the negative Y direction with origin at 0,0.
void DpStation::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	Q_UNUSED(ari);
	Q_UNUSED(parent);
	Q_UNUSED(inBuilding);
}

void DpStation::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	Q_UNUSED(re);
	Q_UNUSED(ship);
	Q_UNUSED(last);
	Q_UNUSED(location);
	Q_UNUSED(sceneAtom);
}

void DpStation::AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	Q_UNUSED(re);
	Q_UNUSED(bd);
	Q_UNUSED(last);
	Q_UNUSED(location);
	Q_UNUSED(sceneAtom);
}

void DpStation::DeleteDeviceObjects(AuGLEffects& re)
{
	Q_UNUSED(re);
	// DaAtom deletes all textures used by the owner's atom id.
	m_BeamStartTex = 0;
	m_NavFixTex = 0;
	m_WaypointTex = 0;
}

void DpStation::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaCreature& seatedCreature) const
{
	Q_UNUSED(aControlPanels);
	Q_UNUSED(tModel);
	Q_UNUSED(idPanels);
	Q_UNUSED(seatedCreature);
}

void DpStation::SampleControlPanelInput(qint16 idPanel, const AuPoint& fPanelLoc, DiControlInputArray& aActiveControls,
		bool bContinuous)
{
	Q_UNUSED(idPanel);
	Q_UNUSED(fPanelLoc);
	Q_UNUSED(aActiveControls);
	Q_UNUSED(bContinuous);
}

void DpStation::Tx(QDataStream& out) const
{
	Q_UNUSED(out);
}

void DpStation::Rx(QDataStream& in, DaCapsule& capsule)
{
	Q_UNUSED(in);
	Q_UNUSED(capsule);
}

// Refresh data from counterpart.
void DpStation::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	Q_UNUSED(pCon);
	Q_UNUSED(parent);
}

// Render course lines in space.
void DpStation::RenderCourse(const DeWaypointRoute& route, const AuMatrix4x4& projection, const AuMatrix4x4& tModel,
		const DaAtom& sceneAtom, AuAtomRenderInfo& ari, bool lines, bool reticles)
{
	AuEther* ether = sceneAtom.Ether();
	if(!ether)
		return;
	if(route.isEmpty())
		return;
	if(sceneAtom.IsMini())
		return;
	if(!ari.m_ShowHUD)
		return;
	const DaVessel* pship = sceneAtom.FindParentVesselInEnv();
	if(!pship)
		return;
	const DaAtom* pss = pship->FindEnvironmentAtom();
	if(!pss)
		return;
	const DaSector* psect = pss->FindParentSector();
	if(!psect)
		return;

	if(!m_BeamStartTex)
		m_BeamStartTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "BeamStart", ":data/txBeamStart.png");
	if(!m_NavFixTex)
		m_NavFixTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NavFix", ":data/txNavFix.png");
	if(!m_WaypointTex)
		m_WaypointTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "NavReticle", ":data/txWaypoint.png");

	ari.re->glDisable(GL_DEPTH_TEST);
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

	// Multiply by inverse of subatom transformation
	// (Translation, rotation, and scale.)
	AuMatrix4x4 sysRot;
	sysRot.Rotate(pss->Rotation());
	sysRot = sysRot * sceneAtom.EnvTransform();
	AuPoint floc = sysRot.Project(AuPoint(0,0));	// vector from solar system in coordinates relative to the sector

	AuMatrix4x4 tstar;
	tstar.Translate(-floc);
	tstar = tstar * sysRot;
	tstar = tstar.Inverse();

	AuMatrix4x4 tsub = sceneAtom.EnvTransform().Inverse();
	AuMatrix4x4 tmodel = tModel * tsub;

	// Determine the center point, in parsecs, about which the star field is offset.
	AuPoint fSceneLoc;
	sceneAtom.GetLocationRelativeToParent(fSceneLoc, psect);

	// Get a list of the star locations relative to the current scene location (in parsecs).
	DqAtomLocArray aStars;

	// Draw course line between waypoints in model space.
	if(lines && route.count() > 1)
	{
		// Prepare shaders.
		ari.re->GLSL().LineShader().Select();
		ari.re->GLSL().LineShader().SetProjectionModelView(projection * tModel);
		ari.re->GLSL().LineShader().SetColor(0, 0.25f, 1, 1);

		// Compile a list of vertex locations.
		AuPointArray verts;
		for(DeWaypointRoute::const_iterator wi = route.constBegin(); wi != route.constEnd(); wi++)
		{
			AuPoint fwp;
			if((*wi).Location(pss, fwp))
			{
				// Now we have the location of the waypoint relative to the parent solar system.
				fwp = tsub.Project(fwp);
				verts.append(fwp);
			}
			else
			{
				// Determine waypoint location in star field.
				const DaSolarSystem *pdss = (const DaSolarSystem*)ether->FindNormalAtom((*wi).m_idSystem);

				// Find the destination system in the list.
				if(aStars.count() == 0)
					psect->ListSolarSystemLocations(aStars);
				for(int il=0; il<aStars.count(); il++)
				{
					DqAtomLoc &al = aStars[il];
					if(al.m_pA == (const DaAtom*)pdss)
					{
						// Found the destination star.
						AuPoint fstarloc = al.m_fLocation - fSceneLoc;
						double rlen = fstarloc.Length();	// Distance to the star, in parsecs (up to about 25).
						double rnulen = rlen / 50.0 + 1.0;
						fstarloc.Unit(rnulen);
						fstarloc = tstar.Project(fstarloc);
						double rStarShellMinRadius = FeetPerSolarSystem() * .5;
						fstarloc *= rStarShellMinRadius;
						verts.append(fstarloc);
						break;
					}
				}
			}
		}

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->GLSL().LineShader().Deselect();
	}

	QRect rcClient = ari.glw->rect();
	double windowAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double sceneAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double fovy = AuGLEffects::WindowFieldOfViewY(sceneAspect, g_Au.FieldOfViewY());

	// Create perspective camera transformations the way AuView does it.
	AuMatrix4x4 tP;
	tP.Perspective(fovy, 1.0, 3.0, FeetPerSector());

	AuMatrix4x4 projAspect;
	projAspect.Scale(windowAspect, 1, 1);

	const double ymax = .98;
	const double xmax = ymax/windowAspect;

	// First segment of course line is drawn in screen space to keep the reticle on the screen.
	if(lines)
	{
		// Prepare shaders.
		ari.re->GLSL().LineShader().Select();
		ari.re->GLSL().LineShader().SetProjectionModelView(projAspect);
		ari.re->GLSL().LineShader().SetColor(0, 0.25f, 1, 1);

		// Compile a list of vertex locations.
		AuPointArray verts;
		verts.append(AuPoint(0,.90,0));	// Start at top center of screen.

		AuPoint fwp;
		if(route[0].Location(pss, fwp))
		{
			// Now we have the location of the waypoint relative to the
			// parent solar system.
			fwp = tmodel.Project(fwp);
			bool behind = false;
			if(fwp.z > 0.)
				behind = true;

			// Keep first waypoint on the screen.
			fwp = tP.Project(fwp);
			fwp.x *= sceneAspect / windowAspect;
			if(behind)
			{
				// Behind viewer.
				// X is reversed.
				if(fwp.x > 0)
					fwp.x = -xmax;
				else
					fwp.x = xmax;
				fwp.y = 0.;
				fwp.z = .5;
			}
			if(fwp.x < -xmax)
				fwp.x = -xmax;
			else if(fwp.x > xmax)
				fwp.x = xmax;
			if(fwp.y < -ymax)
				fwp.y = -ymax;
			else if(fwp.y > ymax)
				fwp.y = ymax;
			verts.append(fwp);
		}
		else
		{
			// Determine waypoint location in star field.
			const DaSolarSystem *pdss = (DaSolarSystem*)ether->FindNormalAtom(route[0].m_idSystem);

			// Find the destination system in the list.
			if(aStars.count() == 0)
				psect->ListSolarSystemLocations(aStars);
			for(int il=0; il<aStars.count(); il++)
			{
				DqAtomLoc &al = aStars[il];
				if(al.m_pA == (const DaAtom*)pdss)
				{
					// Found the destination star.
					AuPoint fstarloc = al.m_fLocation - fSceneLoc;
					double rlen = fstarloc.Length();	// Distance to the star, in parsecs (up to about 25).
					double rnulen = rlen / 50.0 + 1.0;
					fstarloc.Unit(rnulen);
					fstarloc = tstar.Project(fstarloc);
					double rStarShellMinRadius = FeetPerSolarSystem() * .5;
					fstarloc *= rStarShellMinRadius;
					fstarloc = tModel.Project(fstarloc);
					bool behind = false;
					if(fstarloc.z > 0.)
						behind = true;

					// Keep first waypoint on the screen.
					fstarloc = tP.Project(fstarloc);
					fstarloc.x *= sceneAspect / windowAspect;
					if(behind)
					{
						// Behind viewer.
						if(fstarloc.x > 0)
							fstarloc.x = -xmax;
						else
							fstarloc.x = xmax;
						fstarloc.y = 0.;
						fstarloc.z = .5;
					}
					if(fstarloc.x < -xmax)
						fstarloc.x = -xmax;
					else if(fstarloc.x > xmax)
						fstarloc.x = xmax;
					if(fstarloc.y < -ymax)
						fstarloc.y = -ymax;
					else if(fstarloc.y > ymax)
						fstarloc.y = ymax;
					verts.append(fstarloc);
					break;
				}
			}
		}

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, verts, false, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		if(vao.VAO())
		{
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();
		}

		ari.re->GLSL().LineShader().Deselect();

		// Draw the beam start point. The little gold dot at the top of the screen.
		ari.re->RenderSprite(rcClient, 0,.9f, .1f,.1f, m_BeamStartTex, 1,1,1,1);
	}

	// Draw waypoint markers in screen space.
	if(reticles)
	{
		for(DeWaypointRoute::const_iterator wi = route.constBegin(); wi != route.constEnd(); wi++)
		{
			AuPoint fwp;
			if((*wi).Location(pss, fwp))
			{
				// Now we have the location of the waypoint relative to the parent
				// solar system.
				fwp = tmodel.Project(fwp);
				bool behind = false;
				if(fwp.z > 0.)
					behind = true;
				//qDebug("nav at %g,%g,%g", fwp.x, fwp.y, fwp.z);

				fwp = tP.Project(fwp);
				fwp.x *= sceneAspect / windowAspect;
				if(wi != route.constBegin())
				{
					// Not the first waypoint.
					if(!behind)
						ari.re->RenderSprite(rcClient, fwp.x,fwp.y, .1f,.1f, m_NavFixTex, 1,1,1,1);
				}
				else
				{
					// Keep first waypoint on the screen.
					if(behind)
					{
						// Behind viewer.
						if(fwp.x > 0)
							fwp.x = -xmax;
						else
							fwp.x = xmax;
						fwp.y = 0.;
					}
					if(fwp.x < -xmax)
						fwp.x = -xmax;
					else if(fwp.x > xmax)
						fwp.x = xmax;
					if(fwp.y < -ymax)
						fwp.y = -ymax;
					else if(fwp.y > ymax)
						fwp.y = ymax;
					ari.re->RenderSprite(rcClient, fwp.x,fwp.y, .1f,.1f, m_WaypointTex, 1,1,1,1);
				}
			}
			else
			{
				// Determine waypoint location in star field.
				const DaSolarSystem *pdss = (DaSolarSystem*)ether->FindNormalAtom((*wi).m_idSystem);

				// Find the destination system in the list.
				if (aStars.count() == 0)
					psect->ListSolarSystemLocations(aStars);
				for (int il=0; il<aStars.count(); il++)
				{
					DqAtomLoc &al = aStars[il];
					if (al.m_pA == (const DaAtom*)pdss)
					{
						// Found the destination star.
						AuPoint fstarloc = al.m_fLocation - fSceneLoc;
						double rlen = fstarloc.Length();	// Distance to the star, in parsecs (up to about 25).
						double rnulen = rlen / 50.0 + 1.0;
						fstarloc.Unit(rnulen);
						fstarloc = tstar.Project(fstarloc);
						double rStarShellMinRadius = FeetPerSolarSystem() * .5;
						fstarloc *= rStarShellMinRadius;
						fstarloc = tModel.Project(fstarloc);
						bool behind = false;
						if(fstarloc.z > 0.)
							behind = true;
						fstarloc = tP.Project(fstarloc);
						fstarloc.x *= sceneAspect / windowAspect;
						if(wi != route.constBegin())
						{
							// Not the first waypoint.
							if(!behind)
								ari.re->RenderSprite(rcClient, fstarloc.x,fstarloc.y, .1f,.1f, m_NavFixTex, 1,1,1,1);
						}
						else
						{
							// Keep first waypoint on the screen.
							if(behind)
							{
								// Behind viewer.
								if(fstarloc.x > 0)
									fstarloc.x = -xmax;
								else
									fstarloc.x = xmax;
								fstarloc.y = 0.;
							}
							if(fstarloc.x < -xmax)
								fstarloc.x = -xmax;
							else if(fstarloc.x > xmax)
								fstarloc.x = xmax;
							if(fstarloc.y < -ymax)
								fstarloc.y = -ymax;
							else if(fstarloc.y > ymax)
								fstarloc.y = ymax;
							ari.re->RenderSprite(rcClient, fstarloc.x,fstarloc.y, .1f,.1f, m_WaypointTex, 1,1,1,1);
						}
						break;
					}
				}
			}
		}
	}

	ari.re->glDisable(GL_BLEND);
	ari.re->glEnable(GL_DEPTH_TEST);
}

//static
void DpStation::RenderSensorLock(const AuId& lockedId, POLITICAL_STANCE lockedStance, bool isCivilian, const AuPoint& lockedSysLoc,
		const AuMatrix4x4& tModel, const DaAtom& sceneAtom, AuAtomRenderInfo& ari)
{
	if(lockedId.IsNull())
		return;
	if(sceneAtom.IsMini())
		return;
	if(!ari.m_ShowHUD)
		return;
	const DaCapsule* pship = sceneAtom.FindParentCapsuleInEnv();
	if(!pship)
		return;
	const DaAtom* system = pship->FindEnvironmentAtom();
	if(!system)
		return;

	GLuint sensorLockTex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SensorLock", ":data/txLockOn.png");

	ari.re->glDisable(GL_DEPTH_TEST);

	// Due to network latency, the blip appears to jitter if we simply use the location received.
	// If we can locate the locked atom, we can be really precise when it really matters, as in when the atom is in the scene.
	AuPoint fLockLoc;
	const DaAtom* lockedAtom = system->FindIdRecursive(lockedId);
	if(lockedAtom)
		fLockLoc = lockedAtom->EnvLocation();
	else
		fLockLoc = lockedSysLoc;
	fLockLoc = sceneAtom.EnvTransform().UnProject(fLockLoc);					// blip loc in scene atom space
	fLockLoc = tModel.Project(fLockLoc);										// into eye space
	bool behind = false;
	if(fLockLoc.z > 0.)
		behind = true;
	double targDist = fLockLoc.Length();	// Distance from viewers eye.

	QRect rcClient = ari.glw->rect();
	double windowAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double sceneAspect = AuGLEffects::WindowAspectRatio(rcClient);
	double fovy = AuGLEffects::WindowFieldOfViewY(sceneAspect, g_Au.FieldOfViewY());

	// Create perspective camera transformations the way AuView does it.
	AuMatrix4x4 tP;
	tP.Perspective(fovy, 1.0, 3.0, FeetPerSector());

	// sensor lock reticle
	fLockLoc = tP.Project(fLockLoc);
	fLockLoc.x *= sceneAspect / windowAspect;
	const double ymax = 0.98;
	const double xmax = ymax/windowAspect;
	if(behind)
	{
		// Behind viewer.
		// X is reversed.
		if(fLockLoc.x > 0)
			fLockLoc.x = -xmax;
		else
			fLockLoc.x = xmax;
		fLockLoc.y = 0.;
	}
	if(fLockLoc.x < -xmax)
		fLockLoc.x = -xmax;
	else if(fLockLoc.x > xmax)
		fLockLoc.x = xmax;
	if(fLockLoc.y < -ymax)
		fLockLoc.y = -ymax;
	else if(fLockLoc.y > ymax)
		fLockLoc.y = ymax;

	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

	QColor blipCol = PoliticalStanceColor(lockedStance, isCivilian);
	if(behind)
		ari.RenderSprite(fLockLoc.x, fLockLoc.y, .06f, .06f, sensorLockTex, blipCol.redF(), blipCol.greenF(), blipCol.blueF(), 1);
	else
		ari.RenderSprite(fLockLoc.x, fLockLoc.y, .1f, .1f, sensorLockTex, blipCol.redF(), blipCol.greenF(), blipCol.blueF(), 1);

	// Show target distance on crosshair.
	int meters = (int)ceil(targDist / FeetPerMeter());
	ari.RenderNumber(meters, fLockLoc.x + .02f, fLockLoc.y + .02f, .025f, 0, false, blipCol.redF(), blipCol.greenF(),
			blipCol.blueF(), 1);

	ari.re->glDisable(GL_BLEND);
	ari.re->glEnable(GL_DEPTH_TEST);
}

// Draw extra information about this blip into a rectangle provided for this purpose.
// Updates 'scroll' value if it exceeds limit.
void DpStation::DrawInfo(const DaAtom* system, QPixmap& pix, const QSize &sz, quint8 fontSize, quint8& scroll,
		const AuSensorBlip* blip) const
{
	QPainter p(&pix);

	// A two-pixel margin is allowed.
	QRect rc(QPoint(0,0),sz);
	rc.setLeft(rc.left()+2);
	rc.setTop(rc.top()+2);
	rc.setRight(rc.right() - (2 + 8 + 1));	// Extra 8 pixels to allow for controls at the right.
	rc.setBottom(rc.bottom()-2);

	// Create a font for text display.
	p.setFont(QFont(g_Au.TypefaceSans(), fontSize));
	p.setPen(QPen(QColor(192,192,192), 0.f));

	QStringList infoLines = blip->InfoLines(system);
	if(!infoLines.isEmpty())
	{
		QString sInfo;
		if(scroll > infoLines.count() - 1)
			scroll = infoLines.count() - 1;		// Show only the last line rather than nothing.
		for(QStringList::const_iterator li = infoLines.constBegin() + scroll; li != infoLines.constEnd(); li++)
			if(sInfo.isEmpty())
				sInfo = *li;
			else
				sInfo += QChar('\n') + *li;

		p.drawText(rc, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, sInfo);
	}
	else
		scroll = 0;
}

quint64 DpStation::MemoryUsage() const
{
	quint64 mem = sizeof(DpStation);
	return mem;
}
