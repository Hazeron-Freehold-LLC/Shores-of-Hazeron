//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dateleporter.cpp,v 1.171 2022/05/17 16:45:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dateleporter.h"
//
#include "auatomrenderinfo.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "dgdata.h"
#include <qgl.h>
#include <QSqlError>
#include <qsqlquery.h>

static GLuint s_TexOrigin = 0;
#define c_LifeSpanOrigin				15						// Life span of an origin teleporter, in seconds.
#define c_LifeSpanDestination			18						// Life span of a destination teleporter, in seconds.
#define d_PersonSizeLat					1.3						// Lateral distance from teleporter center, for people.
#define d_PersonSizeVert				3.9						// Vertical height above/below teleporter center, for people.
#define d_VehicleSizeLat				9						// Lateral distance from teleporter center, for vehicles.
#define d_VehicleSizeVert				5						// Vertical height above/below teleporter center, for vehicles.

DaTeleporter::DaTeleporter(const AuId& dest, double maxRange)
{
	m_idDest = dest;
	m_MaxRange2 = maxRange*maxRange;
	m_PSTeleporter = NULL;
	m_VehicleSpotType = SPOT_Invalid;
}

DaTeleporter::DaTeleporter()
{
	m_MaxRange2 = 0;
	m_PSTeleporter = NULL;
	m_VehicleSpotType = SPOT_Invalid;
}

DaTeleporter::~DaTeleporter()
{
	if(m_PSTeleporter)
	{
		delete m_PSTeleporter;
		m_PSTeleporter = 0;
	}
}

DaTeleporter::DaTeleporter(const DaTeleporter &rhs) : DaAtom(rhs)
{
	rhs.m_TeleporterLock.lockForRead();
	m_idDest = rhs.m_idDest;
	m_MaxRange2 = rhs.m_MaxRange2;
	m_Teleporters = rhs.m_Teleporters;
	m_VehicleSpotType = rhs.m_VehicleSpotType;
	rhs.m_TeleporterLock.unlock();

	m_PSTeleporter = 0;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaTeleporter::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	quint8 hasWhat = 0;
	if(IsOrigin())
		hasWhat |= 0x01;	// client only needs to know yes or no
	if(m_VehicleSpotType != SPOT_Invalid)
		hasWhat |= 0x02;
	toClient << hasWhat;

	// Teleporter Locations
	m_TeleporterLock.lockForRead();
	Q_ASSERT(m_Teleporters.count() <= 255);
	toClient << (quint8)m_Teleporters.count();
	for(AuPointArray::const_iterator ti = m_Teleporters.constBegin(); ti != m_Teleporters.constEnd(); ti++)
		toClient << *ti;
	m_TeleporterLock.unlock();

	if(hasWhat & 0x02)
		toClient << (quint8)m_VehicleSpotType;

	// Client doesn't need to know the max range value.
}

// Receive updates to this atom from a server.
void DaTeleporter::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
		m_idDest = AuId(1);	// client only needs to know 1 for origin, 0 for dest

	// Teleporter Locations
	m_Teleporters.clear();
	quint8 teleporterCount;
	fromServer >> teleporterCount;
	AuPoint teleporterLoc;
	while(teleporterCount--)
	{
		fromServer >> teleporterLoc;
		m_Teleporters.append(teleporterLoc);
	}

	if(hasWhat & 0x02)
	{
		quint8 ui8;
		fromServer >> ui8;
		m_VehicleSpotType = (VEHICLE_SPOT_TYPE)ui8;
	}

	// Client doesn't need to know the max range value.
}

// Refresh data from valid counterpart.
bool DaTeleporter::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaTeleporter* pA = (const DaTeleporter*)pAtom;
	pA->m_TeleporterLock.lockForRead();
	m_TeleporterLock.lockForWrite();
	m_idDest = pA->m_idDest;
	m_Teleporters = pA->m_Teleporters;
	m_VehicleSpotType = pA->m_VehicleSpotType;
	m_TeleporterLock.unlock();
	pA->m_TeleporterLock.unlock();

	// Client doesn't need to know the max range value.

	return false;
}

DaAtom *DaTeleporter::Clone() const
{
	return new DaTeleporter(*this);
}

// Report the name of this Atom for listing purposes.
QString DaTeleporter::Name() const
{
	QString sName;
	switch(m_VehicleSpotType)
	{
	case SPOT_GroundVehicle:
		if(IsOrigin())
			return tr("Origin Ground Vehicle Teleporter");
		else
			return tr("Destination Ground Vehicle Teleporter");
	case SPOT_SpaceVehicle:
		if(IsOrigin())
			return tr("Origin Space Vehicle Teleporter");
		else
			return tr("Destination Space Vehicle Teleporter");
	case SPOT_WaterVehicle:
		if(IsOrigin())
			return tr("Origin Water Vehicle Teleporter");
		else
			return tr("Destination Water Vehicle Teleporter");
	case SPOT_Rocket:
		if(IsOrigin())
			return tr("Origin Space Rocket Teleporter");
		else
			return tr("Destination Space Rocket Teleporter");
	case SPOT_Invalid:
	case SPOT_Count:
	default:
		if(IsOrigin())
			return tr("Origin Teleporter");
		else
			return tr("Destination Teleporter");
	}
}

void DaTeleporter::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	// Do not animate subatoms.
	Q_UNUSED(ari);
	Q_UNUSED(sceneAtom);
	Q_UNUSED(lastAnim);
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaTeleporter::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(projection);
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	// Do not render distant teleporters.
	if(pass != PASS_Near)
		return;

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, rmod.Matrix(Id()), ari.al, ari.fade);

	renderLast.Insert(rmod, this);
}

void DaTeleporter::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	// Adjust gain based on number of portals and relative location to scene atom.
	// Gain fades off at the end of the portal's life.
	float gain = (.125f + (m_Teleporters.count() * .075f)) * RelativeAudioGain(sceneAtom) * fade;
	if(m_VehicleSpotType != SPOT_Invalid)
		gain *= 2.f;
	if(IsOrigin())
	{
		// This time comparison doesn't have to be super precise.
		if(CreatedTime().SecsTo(g_Au.CoordinatedTime()) >= c_LifeSpanOrigin)
		{
			float msover = CreatedTime().MSecsTo(g_Au.CoordinatedTime()) - c_LifeSpanOrigin*1000;
			gain -= msover * .001f * gain;	// 1/1000th of gain for each ms, so it takes one sec to fade to 0 wherever it started.
			if(gain < 0)
				gain = 0;
		}
	}
	else
	{
		// This time comparison doesn't have to be super precise.
		if(CreatedTime().SecsTo(g_Au.CoordinatedTime()) >= c_LifeSpanDestination)
		{
			float msover = CreatedTime().MSecsTo(g_Au.CoordinatedTime()) - c_LifeSpanDestination*1000;
			gain -= msover * .001f * gain;	// 1/1000th of gain for each ms, so it takes one sec to fade to 0 wherever it started.
			if(gain < 0)
				gain = 0;
		}
	}

	SOUND_EFFECT sound = SOUND_Transporter;
	AuPoint epos = model.Translation();					// Sound emission location relative to this atom.
	float refDist = (float)AuSound::SoundReferenceDistance(sound);
	float rolloff = AuSound::SoundRolloffFactor(sound);
	//qDebug() << "DaTeleporter::RenderAudioSources" << Id() << "gain" << gain;

	// Play my sound if it's not playing yet. Otherwise just updates the sound parameters, such as position and gain.
	SOUND_EFFECT cur = (SOUND_EFFECT)m_Sound[0];
	al->Play(m_Audio[0], cur, sound, epos.x, epos.y, epos.z, gain, 1.f, true, refDist, rolloff, 0);
	m_Sound[0] = cur;
}

void DaTeleporter::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(sceneAtom);
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	if(!m_PSTeleporter)
	{
		m_PSTeleporter = new PsTeleport;
		if(m_VehicleSpotType != SPOT_Invalid)
		{
			// Vehicle teleporter.
			m_PSTeleporter->SetMaxParticles(1000);
			m_PSTeleporter->SetNumToRelease(60);
			m_PSTeleporter->SetReleaseInterval(0.1f);
			m_PSTeleporter->SetLifeCycle(3.0f);
			m_PSTeleporter->SetSize(120.0f);
			QColor partCol;
			if(!IsOrigin())
			{
				partCol.setRgbF(0.9f, 0.2f, 0.5f);
			}
			else
			{
				partCol.setRgbF(0.2f, 0.5f, 0.9f);
				m_PSTeleporter->SetOrigin(true);
			}
			m_PSTeleporter->SetColor(partCol);
			m_PSTeleporter->SetEndReleaseTime(13);
			m_PSTeleporter->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
			m_PSTeleporter->SetVelocityVar(2.5f);
			m_PSTeleporter->SetSpread(d_VehicleSizeVert, d_VehicleSizeLat);
			m_PSTeleporter->Init(*ari.re);
		}
		else
		{
			// Personnel teleporter.
			m_PSTeleporter->SetMaxParticles(1000);
			m_PSTeleporter->SetNumToRelease(75);
			m_PSTeleporter->SetReleaseInterval(0.05f);
			m_PSTeleporter->SetLifeCycle(3.0f);
			m_PSTeleporter->SetSize(30.0f);
			QColor partCol;
			if(!IsOrigin())
			{
				partCol.setRgbF(0.9f, 0.2f, 0.5f);
			}
			else
			{
				partCol.setRgbF(0.2f, 0.5f, 0.9f);
				m_PSTeleporter->SetOrigin(true);
			}
			m_PSTeleporter->SetColor(partCol);
			m_PSTeleporter->SetEndReleaseTime(13);
			m_PSTeleporter->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
			m_PSTeleporter->SetVelocityVar(1.5f);
			m_PSTeleporter->SetSpread((float)d_PersonSizeVert, (float)d_PersonSizeLat);
			m_PSTeleporter->Init(*ari.re);
		}
	}

	ari.re->GLSL().SpriteSizeShader().Select();
	ari.re->GLSL().SpriteSizeShader().SetTexture(s_TexOrigin);
	ari.re->GLSL().SpriteSizeShader().SetColor(m_PSTeleporter->GetColor());
	ari.re->GLSL().SpriteSizeShader().SetSize(m_PSTeleporter->GetSize());

	// Render the teleporters relative to this atom.
	const AuMatrix4x4& model = rmod.Matrix(Id());
	ari.re->glEnable(GL_DEPTH_TEST);
	ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend
	for(AuPointArray::const_iterator pi = m_Teleporters.constBegin(); pi != m_Teleporters.constEnd(); pi++)
		if(m_PSTeleporter->Update())
		{
			AuMatrix4x4 tp(model);
			tp.Translate(*pi);
			ari.re->GLSL().SpriteSizeShader().SetProjectionModelView(projection * tp);

			m_PSTeleporter->Render(projection, tp, *ari.re);
		}
	ari.re->glDepthMask(GL_TRUE);
	ari.re->glDisable(GL_BLEND);

	ari.re->GLSL().SpriteSizeShader().Deselect();
}

void DaTeleporter::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::CreateDeviceObjects(ari);

	// Create texture.
	if(!s_TexOrigin)
		s_TexOrigin = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "tBCM", ":data/particle.bmp");
}

void DaTeleporter::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::DeleteDeviceObjects(ari);
}

// Returns true if movement from foldloc to fnuloc would enter the area of influence of this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaTeleporter::CheckPeerJoin(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom* who)
{
	// Cannot enter the destination end of a teleporter.
	if(!IsOrigin())
		return false;

	double checkDist;		// Z distance in atmosphere or on ground, 3D distance otherwise.
	double checkDistLat2;	// Squared lateral distance to check when in an atmosphere or on the ground.
	if(m_VehicleSpotType != SPOT_Invalid)
	{
		// Check for correct type of vehicle.
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle *>(who);
		if(!vehicle)
			return false;
		if(vehicle->ParkingSpotType() != VehicleSpotType())
			return false;
		checkDist = d_VehicleSizeVert;
		checkDistLat2 = d_VehicleSizeLat*d_VehicleSizeLat;
	}
	else
	{
		// Only creatures may use teleporters.
		const DaCreature* creature = dynamic_cast<const DaCreature *>(who);
		if(!creature)
			return false;
		//if(creature->MaxHitPoints() > 250)
			//return false;
		checkDist = d_PersonSizeVert;
		checkDistLat2 = d_PersonSizeLat*d_PersonSizeLat;
	}

	// Check distance from every teleporter.
	AuPoint testLoc = Transform().UnProject(fnuloc);			// Calculate nu location in my coordinate space.
	m_TeleporterLock.lockForRead();
	for(AuPointArray::const_iterator pi = m_Teleporters.constBegin(); pi != m_Teleporters.constEnd(); pi++)
	{
		AuPoint teleporterLoc = testLoc - *pi;					// Vector to a teleporter, in my space.
		if(who->IsOnGround() || who->IsInAtmosphere())
		{
			// When on the ground, like at a planet or in a ship, the person is usually oriented to the teleporter.
			if(fabs(teleporterLoc.z) > checkDist)
				continue;											// Too far above or below.
			if(teleporterLoc.Length2_2D() > checkDistLat2)
				continue;											// Too far away laterally.
		}
		else
		{
			// When not on the ground, like when EVA, the person could be at any rotation relative to the teleporter.
			if(teleporterLoc.Length() > checkDist)
				continue;
		}

		// Close enough, the atom has joined this teleporter.
		m_TeleporterLock.unlock();
		return true;
	}
	m_TeleporterLock.unlock();

	// No teleporters close enough to fnuloc.
	return false;
}

// The only time a teleporter has a subatom is when an atom has entered it.
// The atom is either moved to the destination, or removed. It can't stay.
bool DaTeleporter::MoveSubatom(DaAtom* pMove, const AuPoint &/*vMove*/, int /*elapsedMs*/)
{
	// Subatoms of a teleporter are not moved on the client.
	if(g_Au.IsSceneClient())
		return true;
	Q_ASSERT(pMove);
	Q_ASSERT(pMove->IsNormal());

	// Attempt to move the atom to the destination teleporter.
	DaAtom* system = FindEnvironmentAtom();
	if(system)
	{
		// Get my location relative to the parent solar system.
		AuPoint mySysLoc = EnvLocation();

		DaTeleporter* telDest = (DaTeleporter*)system->FindIdRecursive(DestinationId());
		if(telDest)
		{
			// The destination teleporter was found.
			// Insure they are within range of each other.
			AuPoint destSysLoc = telDest->EnvLocation();
			if(mySysLoc.Distance2(destSysLoc) < MaxRange2())
			{
				bool wasInAtm = pMove->IsInAtmosphere();		// in atmosphere range of a planet
				bool wasInShip = pMove->IsInSpacecraft();
				bool wasInWater = pMove->IsEyeSubmerged();		// eye under water and wet, for player stats
				bool wasEVA = (!wasInAtm && !wasInShip) ? true:false;
				const DaWorld* wasOnPlanet;
				if(wasInAtm && !wasInShip)
					wasOnPlanet = pMove->FindParentPlanetInEnv();
				else
					wasOnPlanet = 0;

				// Calculate a transform to place them perfectly at the destination.
				int takeIndex = TakeTeleporterContainingLoc(pMove->Location());	// Take the portal that is at their location.
				AuMatrix4x4 destT(telDest->Transform());
				AuPoint tloc = telDest->TakeTeleporterLoc(takeIndex);			// Take the corresponding destination portal.
				tloc.z += pMove->GroundHeightAboveCG() - DeckHeight()/2;	// Adjust Z so their feet are on the floor.
				destT.Translate(tloc);

				// Remove the atom and put it at the destination teleporter's location.
				pMove->ForcePosMove(AuAtomPos(telDest->ParentId(), destT), AuPoint(0,0));
				pMove->SetModified();
				pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.

				// When the last atom is gone from this teleporter it is destroyed.
				m_TeleporterLock.lockForRead();
				bool v = m_Teleporters.isEmpty();
				m_TeleporterLock.unlock();
				if(v && !HasSubatoms())
				{
					// All my teleporters are used up and I have no more subatoms. The dest teleporter and I can go away now,
					// without waiting for our time to die in Tick.
					Kill();										// I need to go away immediately.
					telDest->Kill();							// Destination teleporter needs to go away immediately.
				}
				else
				{
					SetModified();							// One origin teleporter location was used up.
					telDest->SetModified();					// One destination teleporter location was used up.
				}

				// Player Statistics
				DaPerson* person = dynamic_cast<DaPerson*>(pMove);
				if(person)
				{
					bool isInAtm = pMove->IsInAtmosphere();		// in atmosphere range of a planet
					bool isInShip = pMove->IsInsideSpacecraft();
					bool isInWater = pMove->IsEyeSubmerged();		// eye under water and wet, for player stats
					bool isEVA = (!isInAtm && !isInShip) ? true:false;
					const DaWorld* isOnPlanet;
					if(isInAtm && !isInShip)
						isOnPlanet = pMove->FindParentPlanetInEnv();
					else
						isOnPlanet = 0;

					// Player Statistics
					if(wasEVA)
						person->Stats().EVABeamedFrom();							// Beamed from EVA.
					else if(isEVA)
						person->Stats().EVABeamedTo();								// Beamed to EVA.
					else if(wasOnPlanet && !isOnPlanet)
						person->Stats().PlanetBeamedUp(wasOnPlanet, wasInWater);	// Beamed up from planet surface.
					else if(isOnPlanet && !wasOnPlanet)
						person->Stats().PlanetBeamedDown(isOnPlanet, isInWater);	// Beamed down to planet surface.
				}
				return true;
			}
			else
				qDebug() << "DaTeleporter::MoveSubatom dest teleporter out of range, pMove" << pMove->Id() << pMove->Name();
		}
		else
			qDebug() << "DaTeleporter::MoveSubatom dest teleporter not found, pMove" << pMove->Id() << pMove->Name();
	}
	else
		qDebug() << "DaTeleporter::MoveSubatom parent solar system not found, pMove" << pMove->Id() << pMove->Name();

	// If we get here, the destination teleporter could not be located or it is out of range.
	pMove->ForcePosMove(Pos(), AuPoint(0,0));
	pMove->SetModified();
	pMove->InvalidateEnvironmentRecursive();	// Refresh environment on atom when it moves, in this thread.

	if(!HasSubatoms())
		Kill();	// I need to go away immediately. The dest teleporter will be killed in Tick, if it still exists.
	return true;
}

AuPoint DaTeleporter::TakeTeleporterLoc(unsigned int takeIndex)
{
	m_TeleporterLock.lockForWrite();
	if(takeIndex < (unsigned int)m_Teleporters.count())
	{
		// 'takeIndex' is in my valid range of teleporters.
		AuPoint tloc(m_Teleporters[takeIndex]);
		m_Teleporters.remove(takeIndex);
		m_TeleporterLock.unlock();
		return tloc;
	}
	else if(!m_Teleporters.isEmpty())
	{
		// 'takeIndex' is NOT in my valid range of teleporters but I have at least one teleporter.
		// Remove a teleporter and return its location.
		AuPoint tloc(m_Teleporters[0]);
		m_Teleporters.remove(0);
		m_TeleporterLock.unlock();
		return tloc;
	}
	m_TeleporterLock.unlock();

	qDebug() << "DaTeleporter::TakeTeleporterLoc" << Id() << Name() << "no teleporter locs to take";
	return AuPoint(0,0);
}

unsigned int DaTeleporter::TakeTeleporterContainingLoc(const AuPoint& loc)
{
	// Remove the teleporter that contains loc.
	unsigned int takeIndex = 0;
	m_TeleporterLock.lockForWrite();
	for(AuPointArray::iterator pi = m_Teleporters.begin(); pi != m_Teleporters.end(); pi++, takeIndex++)
	{
		AuPoint teleporterLoc = loc - *pi;						// Vector to a teleporter, in my space.
		if(fabs(teleporterLoc.z) > 3.9)
			continue;											// Too far above or below.
		if(teleporterLoc.Length2_2D() > 1.3*1.3)
			continue;											// Too far away laterally.

		// Close enough, the loc is inside a teleporter.
		m_Teleporters.erase(pi);
		m_TeleporterLock.unlock();
		return takeIndex;
	}

	// We get here if no teleporter was found to contain loc.
	if(!m_Teleporters.isEmpty())
	{
		// The caller wants us to take a teleporter so do it.
		int telCount = m_Teleporters.count();
		m_Teleporters.remove(0);
		m_TeleporterLock.unlock();

		qDebug() << "DaTeleporter::TakeTeleporterContainingLoc" << Id() << Name() << "none of my" << telCount
			<< "teleporters contains" << loc;
	}
	else
		m_TeleporterLock.unlock();

	return 0;
}

// Insert first time database record.
void DaTeleporter::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Teleporter (id,idp,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,"
			"timeDeath, node,maxRange,spotType,teleporters) VALUES (?,?,?,?,?,?,?,?,?,?,?,?, ?,?,?,?);");
	qa.prepare(sq);

	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaTeleporter values.
	qa.addBindValue(m_idDest.GetID());
	qa.addBindValue(sqrt(m_MaxRange2));
	qa.addBindValue(m_VehicleSpotType);

	{
		// Teleporter Locations
		Q_ASSERT(m_Teleporters.count() <= 255);
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_TeleporterLock.lockForRead();
		out << (quint8)m_Teleporters.count();
		for(AuPointArray::const_iterator ti = m_Teleporters.constBegin(); ti != m_Teleporters.constEnd(); ti++)
			out << *ti;
		m_TeleporterLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
	{
		QSqlError err = qa.lastError();
		qWarning("DaTeleporter::Insert %s FAILED '%s' err '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1(),
				(const char*)err.text().toLatin1());
	}
}

// Update existing database record.
void DaTeleporter::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// This atom is known to the server so I need to update my existing record.
	QString sq;
	sq = QString("UPDATE a_Teleporter SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=?, spotType=?, teleporters=? WHERE id=%1;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaTeleporter values.
	qa.addBindValue(m_VehicleSpotType);
	{
		// Teleporter Locations
		Q_ASSERT(m_Teleporters.count() <= 255);
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_TeleporterLock.lockForRead();
		out << (quint8)m_Teleporters.count();
		for(AuPointArray::const_iterator ti = m_Teleporters.constBegin(); ti != m_Teleporters.constEnd(); ti++)
			out << *ti;
		m_TeleporterLock.unlock();
		qa.addBindValue(ba);
	}

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaTeleporter::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaTeleporter::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
DaTeleporter* DaTeleporter::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,timeDeath, "
			"node,maxRange,spotType,teleporters FROM a_Teleporter WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaTeleporter *pnu = new DaTeleporter;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			// DaTeleporter values.
			pnu->m_idDest = AuId(qa.value(qv++).toUInt());
			double mr = qa.value(qv++).toDouble();
			pnu->m_MaxRange2 = mr * mr;
			pnu->m_VehicleSpotType = (VEHICLE_SPOT_TYPE)qa.value(qv++).toUInt();

			// Teleporter Locations
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				quint8 teleporterCount;
				in >> teleporterCount;
				AuPoint teleporterLoc;
				while(teleporterCount--)
				{
					in >> teleporterLoc;
					pnu->m_Teleporters.append(teleporterLoc);
				}
			}

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			DaAnimal::LoadSubatoms(ether, db, pnu->Id());
			DaVehicle::LoadSubatoms(ether, db, pnu->Id());
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

bool DaTeleporter::IsTicker() const
{
	if(IsDead())
		return false;
	return true;
}

void DaTeleporter::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Perform base-level tick processing, including the possibility of dying.
	DaAtom::Tick(last, now);
	if(IsDead())
		return;

	// Origin teleporters disappear c_LifeSpanOrigin seconds after they are opened.
	// Destination teleporters live a few seconds longer.
	if(IsOrigin())
	{
		// Origin teleporters sometimes have subatoms.
		if(!HasSubatoms())
		{
			// This origin teleporter has no subatoms.
			if(CreatedTime().ElapsedSec() > c_LifeSpanOrigin)
				Kill();
		}
	}
	else
	{
		// Destination teleporters never have subatoms.
		if(CreatedTime().ElapsedSec() > c_LifeSpanDestination)
			Kill();
	}
}

void DaTeleporter::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	m_TeleporterLock.lockForRead();
	out << m_idDest;
	out << m_MaxRange2;
	out << m_Teleporters;
	out << (quint8)m_VehicleSpotType;
	m_TeleporterLock.unlock();
}

void DaTeleporter::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	m_TeleporterLock.lockForWrite();
	in >> m_idDest;
	in >> m_MaxRange2;
	in >> m_Teleporters;
	quint8 ui8;
	in >> ui8;
	m_VehicleSpotType = (VEHICLE_SPOT_TYPE)ui8;
	m_TeleporterLock.unlock();
}

DaAtom* DaTeleporter::SightTarget(const DaAtom* /*sightingAtom*/, const AuPoint& /*eyeLoc*/, const AuMatrix4x4 &/*tModel*/,
		DaAtom* pBest, AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	//qDebug() << "DaTeleporter::SightTarget" << Id() << Name();
	Q_UNUSED(sightedLoc);
	Q_UNUSED(sightedPart);
	Q_UNUSED(sightedShield);
	Q_UNUSED(sightedSurfaceNormal);
	// Do not recurse to my subatoms.
	return pBest;
}

bool DaTeleporter::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(cameraEnvLoc);
	return DistanceToAtom2(sceneAtom, 0) < (200*FeetPerMeter() * 200*FeetPerMeter());
}

quint64 DaTeleporter::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaTeleporter) - sizeof(DaAtom);
	mem += m_Teleporters.MemoryUsage();
	return mem;
}

void DaTeleporter::FindTeleportersRecursive(DaAtomList& teleporters) const
{
	//if(!IsNormal())
		//return;
	teleporters.append((DaTeleporter*)this);	// Cast away constness.
}

DaAtom::ATOM_TYPE DaTeleporter::AtomType() const
{
	return ATOM_Teleporter;
}
