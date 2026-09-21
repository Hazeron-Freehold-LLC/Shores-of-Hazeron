//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daarena.cpp,v 1.121 2022/05/17 15:16:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daarena.h"
//
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausynccontroller.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QSqlError>
#include <QSqlQuery>

static GLuint s_TexRingworld = 0;		// Ringworld hull and shadow square texture.

DaArena::DaArena(const QString& name)
{
	m_Name = name;
	m_MatchState = Join;
	m_MatchType = SpacecraftDuel;
	m_MaximizeShipQ = true;
	m_MaxShipQ = 1;
	m_OpponentDesignsOk = true;
	m_ShipDesign = 0;
	m_SpawnCrew = Initial;
	m_SpawnOfficer = Initial;
	m_SpawnPassengers = Initial;
	m_SpawnTroops = Initial;
	m_SpawnVehicles = Initial;
	m_TeamShips = 0;
	m_Team1Ships = 0;
	m_Team2Ships = 0;
}

DaArena::DaArena()
{
	m_MatchState = Join;
	m_MatchType = SpacecraftDuel;
	m_MaximizeShipQ = true;
	m_MaxShipQ = 1;
	m_OpponentDesignsOk = true;
	m_ShipDesign = 0;
	m_SpawnCrew = Initial;
	m_SpawnOfficer = Initial;
	m_SpawnPassengers = Initial;
	m_SpawnTroops = Initial;
	m_SpawnVehicles = Initial;
	m_TeamShips = 0;
	m_Team1Ships = 0;
	m_Team2Ships = 0;
}

DaArena::~DaArena()
{
}

DaArena::DaArena(const DaArena &rhs) : DaAtom(rhs)
{
	m_MatchState = rhs.m_MatchState;
	m_MatchType = rhs.m_MatchType;
	m_MaximizeShipQ = rhs.m_MaximizeShipQ;
	m_MaxShipQ = rhs.m_MaxShipQ;
	m_Name = rhs.m_Name;
	m_OpponentDesignsOk = rhs.m_OpponentDesignsOk;

	rhs.m_PasswordLock.lock();
	m_Password = rhs.m_Password;
	rhs.m_PasswordLock.unlock();

	m_ShellRing = AuGLVAO();
	m_ShipDesign = rhs.m_ShipDesign;

	rhs.m_CommLock.lockForRead();
	m_ShipsHailed = rhs.m_ShipsHailed;
	m_tmChannelsMonitored = rhs.m_tmChannelsMonitored;
	rhs.m_CommLock.unlock();

	m_SpawnCrew = rhs.m_SpawnCrew;
	m_SpawnOfficer = rhs.m_SpawnOfficer;
	m_SpawnPassengers = rhs.m_SpawnPassengers;
	m_SpawnTroops = rhs.m_SpawnTroops;
	m_SpawnVehicles = rhs.m_SpawnVehicles;
	m_TeamShips = rhs.m_TeamShips;
	m_Team1Ships = rhs.m_Team1Ships;
	m_Team2Ships = rhs.m_Team2Ships;
}

DaAtom *DaArena::Clone() const
{
	return new DaArena(*this);
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaArena::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat |= 0x01;

	m_PasswordLock.lock();
	if(!m_Password.isEmpty())
		hasWhat |= 0x02;		// We only send a bit so the client knows there is a password.
	m_PasswordLock.unlock();

	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		toClient << m_MatchType;
		toClient << m_MaximizeShipQ;
		toClient << m_MaxShipQ;
		toClient << m_Name;
		toClient << m_OpponentDesignsOk;
		// m_Password
		toClient << m_ShipDesign;
		toClient << m_SpawnCrew;
		toClient << m_SpawnOfficer;
		toClient << m_SpawnPassengers;
		toClient << m_SpawnTroops;
		toClient << m_SpawnVehicles;
		toClient << m_TeamShips;
		toClient << m_Team1Ships;
		toClient << m_Team2Ships;
	}

	toClient << m_MatchState;
}

// Receive updates to this atom from a server.
void DaArena::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;

	fromServer >> hasWhat;

	if(hasWhat & 0x01)
	{
		fromServer >> m_MatchType;
		fromServer >> m_MaximizeShipQ;
		fromServer >> m_MaxShipQ;
		fromServer >> m_Name;
		fromServer >> m_OpponentDesignsOk;
		// m_Password
		fromServer >> m_ShipDesign;
		fromServer >> m_SpawnCrew;
		fromServer >> m_SpawnOfficer;
		fromServer >> m_SpawnPassengers;
		fromServer >> m_SpawnTroops;
		fromServer >> m_SpawnVehicles;
		fromServer >> m_TeamShips;
		fromServer >> m_Team1Ships;
		fromServer >> m_Team2Ships;
	}

	fromServer >> m_MatchState;

	if(hasWhat & 0x02)
		m_Password = "*";
	else
		m_Password = "";
}

// Refresh data from valid counterpart.
bool DaArena::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaArena* pA = (const DaArena*)pAtom;
	m_MatchState = pA->m_MatchState;
	m_MatchType = pA->m_MatchType;
	m_MaximizeShipQ = pA->m_MaximizeShipQ;
	m_MaxShipQ = pA->m_MaxShipQ;
	m_Name = pA->m_Name;
	m_OpponentDesignsOk = pA->m_OpponentDesignsOk;
	m_ShipDesign = pA->m_ShipDesign;
	m_SpawnCrew = pA->m_SpawnCrew;
	m_SpawnOfficer = pA->m_SpawnOfficer;
	m_SpawnPassengers = pA->m_SpawnPassengers;
	m_SpawnTroops = pA->m_SpawnTroops;
	m_SpawnVehicles = pA->m_SpawnVehicles;
	m_TeamShips = pA->m_TeamShips;
	m_Team1Ships = pA->m_Team1Ships;
	m_Team2Ships = pA->m_Team2Ships;

	pA->m_PasswordLock.lock();
	m_PasswordLock.lock();
	m_Password = pA->m_Password;
	m_PasswordLock.unlock();
	pA->m_PasswordLock.unlock();

	return false;
}

QString DaArena::Name() const
{
	return tr("%1 Arena %2").arg(m_Name).arg(Id().StringId());
}

QString DaArena::Description() const
{
	QString desc = MatchTypeName();

	desc += QString(", Q%1").arg(m_MaxShipQ);

	return desc;
}

void DaArena::ListSceneSubatoms(const DaPerson* pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	const DaAtom* sa = FindIdRecursive(pSceneAtom->Id());
	if(!sa)
		return;	// Do not get subatoms if the scene atom is not in this arena.

	DaAtom::ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
}

// Renders an entire scene from the perspective of this object.
// View transformation has already been set up.
void DaArena::RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, DaPerson* sceneAtom)
{
	Q_UNUSED(farProjection);
	//qDebug() << "DaArena::RenderEnvironment" << Id() << Name();
	// Multiply by inverse of subatom transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Set up lighting for the scene.
	AuMatrix4x4 tmodel = model * tsub;
	SetupLights(sceneAtom, tmodel, *ari.re);

	// Render the skydome.
	const DaSolarSystem* system = sceneAtom->FindParentSolarSystem();
	if(system)
	{
		DaAtom* enviro = system->FindEnvironmentAtom();
		enviro->RenderEnvironment(projection, farProjection, model, ari, sceneAtom);
	}

	RenderShell(projection, tmodel, *ari.re, .2f);

	RenderFieldMarkers(projection, tmodel, ari.textBrightness, *ari.re);

	AuModelMatrixHash rmod;
	CollectModelMatrices(tmodel, rmod);

	DaAtomList subs;
	ListSubatoms(subs);
	if(!subs.empty())
	{
		DaAtomList shadSc;
		DaAtomList shadScFog;
		DaAtomList shadScGlobe;
		DaAtomList shadScGlobeFog;
		DaAtomList shadScRingworld;
		DaAtomList shadScRingworldFog;
		// The renderLast map accumulates info about atoms that must be rendered in a final pass, usually due to blending effects.
		// The map is keyed by z-depth so the resulting contents are z-sorted, farthest to nearest.
		{
			AuMatrix4x4 farProjection(projection);
			AuRenderLastMap renderLast;
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				(*it)->RenderEnvironmentObject(farProjection, ari, rmod, sceneAtom, renderLast, PASS_Far, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_Far);

			// Render atoms that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
			for(AuRenderLastMap::iterator lri = renderLast.begin(); lri != renderLast.end(); lri++)
				(*lri).second->RenderLast(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
						shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_FarBlend);
		}

		AuRenderLastMap renderLast;
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_Near);

		// Render atoms that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
		for(AuRenderLastMap::iterator lri = renderLast.begin(); lri != renderLast.end(); lri++)
			(*lri).second->RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
					shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_NearBlend);
	}
}

void DaArena::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::CreateDeviceObjects(ari);

	if(!s_TexRingworld)
		s_TexRingworld = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Ringworld", ":data/Ringworld.png", GL_LINEAR,
				GL_LINEAR, true, 1);

}

void DaArena::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::DeleteDeviceObjects(ari);

	if(m_ShellRing.VAO())
		m_ShellRing.DeleteGL();
}

// Setup light sources for lighting calculations to render a given object.
void DaArena::SetupLights(DaPerson* pSceneAtom, const AuMatrix4x4& model, AuGLEffects& re)
{
	QList<AuGLSun> suns;
	QList<AuGLLight> lights;
	GetRenderLightsRecursive(*pSceneAtom, model, suns, lights);
	SetRenderLights(re, suns, lights, AuGLLight::MaxAmbientStarlight());	// Bright starlight gives good ambient lighting.
}

void DaArena::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	// Set up some suns.
	const double ltdist = 10000.0;		// Light distance from model.
	double atten = 1.0/(ltdist*ltdist);
	AuPoint vl(0,0,ltdist);
	vl.Unit(ltdist);
	vl = model.Project(vl);
	suns.append(AuGLSun(atten, QColor(220,220,220), vl, 1.0));

	vl = AuPoint(-669,-743,-1);
	vl.Unit(ltdist);
	vl = model.Project(vl);
	suns.append(AuGLSun(atten, QColor(192,192,192), vl, 1.0));

	vl = AuPoint(697,627,-348);
	vl.Unit(ltdist);
	vl = model.Project(vl);
	suns.append(AuGLSun(atten, QColor(160,160,160), vl, 1.0));

	DaAtom::GetRenderLightsRecursive(person, model, suns, lights);
}

// Insert first time database record.
void DaArena::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Arena (id,idp, locX,locY,locZ, rotX,rotY,rotZ, timeCreate,timeModified,timeTock,timeDeath, "
			"matchState,matchType,maximizeShipTL,maxShipTL,name,opponentDesignsOk,password,shipDesign,spawnCrew,"
			"spawnOfficer,spawnPassengers,spawnTroops,spawnVehicles,teamShips,team1Ships,team2ships"
			") VALUES ("
			"?,?, ?,?,?, ?,?,?, ?,?,?,?, "
			"?,?,?,?,?,?,?,?,?,"
			"?,?,?,?,?,?,?);");
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

	// DaArena values
	qa.addBindValue(m_MatchState);
	qa.addBindValue(m_MatchType);
	qa.addBindValue(m_MaximizeShipQ);
	qa.addBindValue(m_MaxShipQ);
	qa.addBindValue(m_Name);
	qa.addBindValue(m_OpponentDesignsOk);
	qa.addBindValue(m_Password);
	qa.addBindValue(m_ShipDesign);
	qa.addBindValue(m_SpawnCrew);
	qa.addBindValue(m_SpawnOfficer);
	qa.addBindValue(m_SpawnPassengers);
	qa.addBindValue(m_SpawnTroops);
	qa.addBindValue(m_SpawnVehicles);
	qa.addBindValue(m_TeamShips);
	qa.addBindValue(m_Team1Ships);
	qa.addBindValue(m_Team2Ships);

	if(!qa.exec())
		qWarning("DaArena::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaArena::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Arena SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, "
			"timeModified=?, timeTock=?, timeDeath=?, "
			"matchState=?, matchType=?, maximizeShipTL=?, maxShipTL=?, name=?, opponentDesignsOk=?, password=?, shipDesign=?, "
			"spawnCrew=?, spawnOfficer=?, spawnPassengers=?, spawnTroops=?, spawnVehicles=?, teamShips=?, "
			"team1Ships=?, team2ships=? "
			"WHERE id=%2;").arg(Id().GetID());
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

	// DaArena values
	qa.addBindValue(m_MatchState);
	qa.addBindValue(m_MatchType);
	qa.addBindValue(m_MaximizeShipQ);
	qa.addBindValue(m_MaxShipQ);
	qa.addBindValue(m_Name);
	qa.addBindValue(m_OpponentDesignsOk);
	qa.addBindValue(m_Password);
	qa.addBindValue(m_ShipDesign);
	qa.addBindValue(m_SpawnCrew);
	qa.addBindValue(m_SpawnOfficer);
	qa.addBindValue(m_SpawnPassengers);
	qa.addBindValue(m_SpawnTroops);
	qa.addBindValue(m_SpawnVehicles);
	qa.addBindValue(m_TeamShips);
	qa.addBindValue(m_Team1Ships);
	qa.addBindValue(m_Team2Ships);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaArena::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaArena::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaArena* DaArena::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,timeDeath, "
			"matchState,matchType,maximizeShipTL,maxShipTL,name,opponentDesignsOk,password,shipDesign,spawnCrew,"
			"spawnOfficer,spawnPassengers,spawnTroops,spawnVehicles,teamShips,team1Ships,team2ships "
			"FROM a_Arena WHERE ");
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
		DaArena *pnu = new DaArena;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;	// query value index
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

			// DaArena values.
			pnu->m_MatchState = qa.value(qv++).toUInt();
			pnu->m_MatchType = qa.value(qv++).toUInt();
			pnu->m_MaximizeShipQ = qa.value(qv++).toBool();
			pnu->m_MaxShipQ = qa.value(qv++).toUInt();
			if(!pnu->m_MaxShipQ)
				pnu->m_MaxShipQ = 1;
			pnu->m_Name = qa.value(qv++).toString();
			pnu->m_OpponentDesignsOk = qa.value(qv++).toBool();
			pnu->m_Password = qa.value(qv++).toString();
			pnu->m_ShipDesign = qa.value(qv++).toUInt();
			pnu->m_SpawnCrew = qa.value(qv++).toUInt();
			pnu->m_SpawnOfficer = qa.value(qv++).toUInt();
			pnu->m_SpawnPassengers = qa.value(qv++).toUInt();
			pnu->m_SpawnTroops = qa.value(qv++).toUInt();
			pnu->m_SpawnVehicles = qa.value(qv++).toUInt();
			pnu->m_TeamShips = qa.value(qv++).toUInt();
			pnu->m_Team1Ships = qa.value(qv++).toUInt();
			pnu->m_Team2Ships = qa.value(qv++).toUInt();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			//qDebug("DaArena - Loading Subatoms ");
			DaSc::LoadSubatoms(ether, db, pnu->Id());
			DaAnimal::LoadSubatoms(ether, db, pnu->Id());
			DaVehicle::LoadSubatoms(ether, db, pnu->Id());
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaItem::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

unsigned int DaArena::IsTocker() const
{
	unsigned int ms = DaAtom::IsTocker();
	if(!IsNormal())
		return ms;
	if(ms == 0 || ms > 10*d_MSecPerSec)
		ms = 10*d_MSecPerSec;
	return ms;
}

// Return false if this atom is to be deleted upon return.
void DaArena::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaAtom::Tock(last, now, scripts, tockTrans);

	// Kill the arena if there are no other people present.
	if(!FindPersonRecursive())
	{
		// Nobody here.
		if(m_MatchState == Join)
			m_MatchState = Play;		// Someone could be enroute. Switch to Play state and go away later.
		else if(m_MatchState == Play)
			m_MatchState = Aftermath;	// Someone could be enroute. Switch to Aftermath state and go away next time through Tock.
		else
		{
			qDebug() << "DaArena::Tock" << Id() << Name() << "empty in Aftermath, going away.";
			Kill();
		}
	}

	// We must monitor channels before spawning ships or we miss the initial hail from target drones.
	m_CommLock.lockForRead();
	bool c = m_tmChannelsMonitored.ElapsedMin() > 15 || !m_tmChannelsMonitored.isValid();
	m_CommLock.unlock();
	if(c)
	{
		// I will refresh my comm channel so it does not decay every 15 minutes.
		// The scene server needs to know what channels are being monitored by atoms.
		//qDebug() << "DaArena::Tock" << Id() << "monitoring hail channel.";
		g_Au.MonitorChannel(AuCommStdChannel::HailChannelName(), Id());			// Monitor Hail channel.
		m_CommLock.lockForWrite();
		m_tmChannelsMonitored.restart();
		m_CommLock.unlock();
	}

	if(!m_Position.isValid() || m_Position.elapsed() >= 2000)
	{
		m_Position.restart();

		// Position waiting people aboard empty ships before spawning nu ships.
		PositionTeamMembers(DgEmpire::ArenaTeam1Id(), true);
		PositionTeamMembers(DgEmpire::ArenaTeam2Id(), true);

		// Spawn a ship for team 1 if someone needs one.
		if(m_Team1Ships && MatchState() != Aftermath)
		{
			if(SpawnTeamShip(DgEmpire::ArenaTeam1Id()))
				m_Team1Ships--;
		}
		else
			PositionTeamMembers(DgEmpire::ArenaTeam1Id(), false);

		// Spawn a ship for team 2 if someone needs one.
		if(m_Team2Ships && MatchState() != Aftermath)
		{
			if(SpawnTeamShip(DgEmpire::ArenaTeam2Id()))
				m_Team2Ships--;
		}
		else
			PositionTeamMembers(DgEmpire::ArenaTeam2Id(), false);
	}
	else
	{
		// Position waiting people aboard empty ships before spawning nu ships.
		PositionTeamMembers(DgEmpire::ArenaTeam2Id(), true);
		PositionTeamMembers(DgEmpire::ArenaTeam1Id(), true);

		// Spawn a ship for team 2 if someone needs one.
		if(m_Team2Ships && MatchState() != Aftermath)
		{
			if(SpawnTeamShip(DgEmpire::ArenaTeam2Id()))
				m_Team2Ships--;
		}
		else
			PositionTeamMembers(DgEmpire::ArenaTeam2Id(), false);

		// Spawn a ship for team 1 if someone needs one.
		if(m_Team1Ships && MatchState() != Aftermath)
		{
			if(SpawnTeamShip(DgEmpire::ArenaTeam1Id()))
				m_Team1Ships--;
		}
		else
			PositionTeamMembers(DgEmpire::ArenaTeam1Id(), false);
	}

	DaAtomList spaceships;
	switch(MatchState())
	{
	case Join:
		if(m_MatchType == SpacecraftDuel)
		{
			bool hasDesign = false;
			QString designDesc;
			// This call to ScDesignReport triggers load of design if it is not already loaded.
			AuScReport report;
			const DgData& sdd = g_Au.LockScDesignsForRead();
			hasDesign = sdd.ScDesignReport(m_ShipDesign, report);
			g_Au.UnlockScDesigns();
			if(hasDesign)
			{
				designDesc = report.DesignName() + ' ' + report.DescLine();
				//qDebug() << "DaArena::Tock" << Id() << Name() << "emitting ArenaReady" << m_Name;
				emit ArenaReady(AuArenaReady(Id(), m_Name, EntryPassword(), m_MatchType, m_MatchState, m_MaxShipQ,
							m_MaximizeShipQ, m_OpponentDesignsOk, m_ShipDesign, designDesc, m_SpawnCrew,
							m_SpawnOfficer, m_SpawnPassengers, m_SpawnTroops, m_SpawnVehicles, m_TeamShips, m_Team1Ships,
							m_Team2Ships, AuTime::Now()));
			}
		}
		break;

	case Play:
		{
			// Check for win condition.
			if(!m_Team1Ships && !m_Team2Ships)
			{
				// Neither team has ships left to spawn.
				// If one team has no ships, the other team wins.
				int team1Ships = 0;
				int team2Ships = 0;
				FindVesselsRecursive(spaceships);
				for(DaAtomList::const_iterator si = spaceships.begin(); si != spaceships.end(); si++)
				{
					if((*si)->EmpireId() == DgEmpire::ArenaTeam1Id())
						team1Ships++;
					else if((*si)->EmpireId() == DgEmpire::ArenaTeam2Id())
						team2Ships++;
				}
				if(!team1Ships && !team2Ships)
				{
					// Announce draw.
					m_MatchState = Aftermath;

					QString text = tr("Match ended in a draw. All spacecraft destroyed.");
					AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
							AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(mymsg);
				}
				else if(!team1Ships)
				{
					// Announce team 2 winner.
					m_MatchState = Aftermath;

					QString empire1, empire2;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName(DgEmpire::ArenaTeam1Id(), empire1);
					gd.EmpireName(DgEmpire::ArenaTeam2Id(), empire2);
					g_Au.UnlockData();

					QString text = tr("Match ended. %1 is the winner. All %2 spacecraft destroyed.").arg(empire2).arg(empire1);
					AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
							AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(mymsg);
				}
				else if(!team2Ships)
				{
					// Announce team 1 winner.
					m_MatchState = Aftermath;

					QString empire1, empire2;
					const DgData& gd = g_Au.LockDataForRead();
					gd.EmpireName(DgEmpire::ArenaTeam1Id(), empire1);
					gd.EmpireName(DgEmpire::ArenaTeam2Id(), empire2);
					g_Au.UnlockData();

					QString text = tr("Match ended. %1 is the winner. All %2 spacecraft destroyed.").arg(empire1).arg(empire2);
					AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
							AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(mymsg);
				}
			}
		}
		break;

	case Aftermath:
		break;
	}

	if(m_MatchState != Aftermath)
	{
		// Check for crew and vehicle respawn.
		if(m_SpawnCrew == Respawn || m_SpawnOfficer == Respawn || m_SpawnPassengers == Respawn || m_SpawnTroops == Respawn ||
				m_SpawnVehicles == Respawn)
		{
			if(spaceships.isEmpty())
				FindVesselsRecursive(spaceships);
			for(DaAtomList::const_iterator si = spaceships.begin(); si != spaceships.end(); si++)
			{
				DaVessel* ship = (DaVessel*)*si;

				// Officers, crew, etc. that need DNA.
				if(m_SpawnCrew == Respawn || m_SpawnOfficer == Respawn || m_SpawnPassengers == Respawn || m_SpawnTroops == Respawn)
					ship->SpawnArenaCrewman(m_SpawnOfficer == Respawn, m_SpawnCrew == Respawn, m_SpawnTroops == Respawn,
							m_SpawnPassengers == Respawn);

				// Vehicles
				if(m_SpawnVehicles == Respawn)
					ship->SpawnArenaVehicle();
			}
		}

		// Duel continues announcing so observers can join.
		/*
		if(m_MatchType == SpacecraftDuel)
		{
			QString designDesc;
			const DgData& sdd = g_Au.LockShipDesignDataForRead();
			const DnSpaceShip* shipDesign = sdd.SpaceShipDesign(m_ShipDesign);
			if(shipDesign)
				designDesc = shipDesign->m_name + ' ' + shipDesign->DescriptionOneLine();
			g_Au.UnlockShipDesignData();
			emit ArenaReady(AuArenaReady(Id(), m_Name, EntryPassword(), m_MatchType, m_MatchState, m_MaxShipQ, m_MaximizeShipQ,
						m_OpponentDesignsOk, m_ShipDesign, designDesc, m_SpawnCrew, m_SpawnOfficer,
						m_SpawnPassengers, m_SpawnTroops, m_SpawnVehicles, m_TeamShips, m_Team1Ships, m_Team2Ships));
		}
		*/
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

// Render circular nav beacon for stations.
void DaArena::RenderFieldMarkers(const AuMatrix4x4& projection, const AuMatrix4x4& model, float textBrightness, AuGLEffects& re)
{
	Q_UNUSED(textBrightness);

	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * model);

	const int iarcstrokes = 100;
	AuPoint fa;
	//DHH glsl re.glLineWidth(1);

	re.GLSL().LineShader().SetColor(1,1,1,.5f);

	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	const float playRadius = c_ArenaRingRadius;
	/*DHH glsl
	re.glBegin(GL_LINE_STRIP);
		// Draw XY plane circle.
		for(int iarc=0; iarc<=iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * playRadius;
			fa.y = sinf(ang) * playRadius;
			re.glVertex3f(fa.x, fa.y, 0);	// iarc=0 -> 1,0,0
		}

		/+
		// Draw XZ plane circle.
		for(int iarc=0; iarc<=iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * playRadius;
			fa.y = sinf(ang) * playRadius;
			re.glVertex3f(fa.x, 0, fa.y);	// iarc=0 -> 1,0,0
		}
		+/
	re.glEnd();
	*/

	// Compile a list of vertex locations.
	// Draw XY plane circle.
	AuPointArray verts;
	for(int iarc=0; iarc<iarcstrokes; iarc++)
	{
		float ang = PIf*2.f * iarc / iarcstrokes;
		fa.x = cosf(ang) * playRadius;
		fa.y = sinf(ang) * playRadius;
		verts.append(fa);	// iarc=0 -> 1,0,0
	}

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_LINE_STRIP);
	vao.DeselectGL();
	vao.DeleteGL();

	AuMatrix4x4 cm(model);
	double ang = 360./12. * DEG2RAD;
	for(int c=0; c<6; c++)
	{
		cm.RotateZ(ang);
		re.GLSL().LineShader().SetProjectionModelView(projection * cm);

		// Compile a list of vertex locations.
		// Draw YZ plane circle.
		AuPointArray verts;
		for(int iarc=0; iarc<iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * playRadius;
			fa.y = sinf(ang) * playRadius;
			verts.append(fa);	// iarc=0 -> 0,1,0
		}

		// Make a VAO of the vertex locations.
		AuGLVAO vao;
		vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);

		// Display the VAO once and clean it up.
		vao.SelectGL();
		vao.DrawGL(GL_LINE_STRIP);
		vao.DeselectGL();
		vao.DeleteGL();
	}

	/*
	// Out of bounds marker.
	re.GLSL().LineShader().SetColor(1,1,0);
	const float boundsRadius = c_ArenaBoundsRadius;
	re.glBegin(GL_LINE_STRIP);
		// Draw XY plane circle.
		for(int iarc=0; iarc<=iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * boundsRadius;
			fa.y = sinf(ang) * boundsRadius;
			re.glVertex3f(fa.x, fa.y, 0);	// iarc=0 -> 1,0,0
		}

		// Draw XZ plane circle.
		for(int iarc=0; iarc<=iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * boundsRadius;
			fa.y = sinf(ang) * boundsRadius;
			re.glVertex3f(fa.x, 0, fa.y);	// iarc=0 -> 1,0,0
		}
	re.glEnd();

	re.glBegin(GL_LINE_STRIP);
		// Draw YZ plane circle.
		for(int iarc=0; iarc<=iarcstrokes; iarc++)
		{
			float ang = PIf*2.f * iarc / iarcstrokes;
			fa.x = cosf(ang) * boundsRadius;
			fa.y = sinf(ang) * boundsRadius;
			re.glVertex3f(0, fa.x, fa.y);	// iarc=0 -> 1,0,0
		}
	re.glEnd();
	*/
	re.glDisable(GL_BLEND);

	re.GLSL().LineShader().Deselect();
}

bool DaArena::SpawnTeamShip(quint8 team)
{
	if(m_MatchType == SpacecraftTrial && team == DgEmpire::ArenaTeam2Id())
	{
		// Spawn a ship for the AI team.
		quint32 shipDesign = m_ShipDesign;
		/*
		if(m_OpponentDesignsOk)
		{
			const DgData& sdd = g_Au.LockShipDesignDataForRead();
			quint32 pirateDesign = sdd.SelectPirateShipDesign((m_MaxShipQ+1)/2, m_MaxShipQ);
			g_Au.UnlockShipDesignData();
			if(pirateDesign)
				shipDesign = pirateDesign;
		}
		*/
		quint8 errcode = 0;
		DaVessel* ship = SpawnSpacecraft(team, shipDesign, errcode);
		if(ship)
		{
			// Officer needs to do something to get started on his thinking processes.
			// The completion of an order triggers him to decide what to do next.
			if(m_SpawnOfficer != No)
			{
				ship->MissionAppendOrder(AuMissionOrderPosture(POSTURE_Offensive));
				ship->MissionSetType(AuMission::Single);
				ship->MissionSetActive(true);
			}

			if(m_SpawnOfficer == No || m_SpawnCrew == No)
			{
				// Hail the nu AI controlled ship ready right away, since conditions will never permit an officer to do so.
				AuCommHailRequest msg(EmpireId(), tr("Target drone ready."),
						AuCommStdChannel::HailChannelName(), ship->Id(), ship->Name(), ship->FindParentSector()->Id(),
						ship->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
			}
			return true;
		}
	}
	else
	{
		// Look for a floating person on the team.
		DaAtomList people;
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaPerson* person = dynamic_cast<DaPerson*>(*it);
			if(!person)
				continue;
			if(person->ArenaTeam() != team)
				continue;
			if(person->IsInvisibleOffLine() || !person->InArenaOutsideRing())
				continue;

			// Found a person on line on the team who is floating in space.
			people.append(person);
		}

		for(DaAtomList::const_iterator it = people.begin(); it != people.end(); it++)
		{
			DaPerson* person = dynamic_cast<DaPerson*>(*it);
			if(!person)
				continue;

			// Spawn a ship for them.
			quint32 shipDesign = m_ShipDesign;
			if(m_OpponentDesignsOk && person->ArenaShipDesign())
				shipDesign = person->ArenaShipDesign();

			quint8 errcode = 0;
			DaVessel* ship = SpawnSpacecraft(team, shipDesign, errcode);
			if(ship)
			{
				// Place the person aboard the nu ship, near the helm.
				person->Stand();
				SyncController(AuSyncPose(AuPose::Stand, 0, 0, 0));
				AuMatrix4x4 tPerson;
				tPerson.Translate(ship->HelmLocation() + AuPoint(1,-1,person->CGHeight()));
				person->ForcePosStop(AuAtomPos(ship->Id(), tPerson));
				person->SetModified();
				if(MatchState() == Join)
				{
					QString text = tr("The match begins when each fleet has a spacecraft and all spacecraft present have "
							"hailed on this channel.");
					AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
							AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(mymsg);
				}
				return true;
			}
		}
	}
	return false;
}

// 'errcode' is an output parameter. It will be set to one of the following values:
// 0 = success
// 1 = an unlikely error occurred
// 2 = design not loaded, try again
// 3 = design not ok to manufacture
DaVessel* DaArena::SpawnSpacecraft(quint8 team, quint32 designId, quint8& errcode)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Insure the design is allowed to be manufactured.
	bool designLoaded;
	bool okToMake;
	AuScReport r;	// For sc designs.
	const DgData& sdd = g_Au.LockScDesignsForRead();
	okToMake = sdd.ScDesignAllowedToManufacture(designId, r);	// Trigger load of design, if it's not loaded.
	designLoaded = sdd.ScDesignIsLoaded(designId);
	g_Au.UnlockScDesigns();

	if(!designLoaded)
	{
		// Design not loaded yet.
		errcode = 2;
		return 0;
	}
	if(!okToMake)
	{
		// Not ok to make the design.
		errcode = 3;
		return 0;
	}

	// If the design quality is too high, and it's not the default design, create the default design instead.
	quint8 designQ = r.Quality();
	if(designId != m_ShipDesign && designQ > m_MaxShipQ)
		return SpawnSpacecraft(team, m_ShipDesign, errcode);	// Create default design instead.

	// Create a spacecraft.
	DaSc* ship = new DaSc;
	if(!ship)
	{
		errcode = 1;	// not likely
		return 0;
	}
	ship->ConnectSignals();
	ship->Create();
	ship->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
	ether->AddAtom(ship);

	ship->SetDesign(r);

	DaVessel* vessel = ship;

	// Find a location at which to spawn the nu ship, before we make it so it doesn't block itself.
	AuPoint spawnAt;
	float spawnShipRot = 0;
	bool blocked;
	double spawnRad;
	if(team == DgEmpire::ArenaTeam1Id() && m_MatchState == Join)
		spawnRad = 0;										// Spawn near the center at first, rim after the game starts.
	else
		spawnRad = c_ArenaRingRadius - FeetPerMoonOrbit();	// Spawn near the rim.
	do
	{
		float spawnPosRot = AuTime::CurrentHourCycle() * PIf*2.f * 5.f;	// x5 to raise the frequency
		if(team == DgEmpire::ArenaTeam1Id())
			spawnRad += 100*FeetPerMeter();	// Work our way outward.
		else
		{
			spawnRad -= 100*FeetPerMeter();	// Work our way inward.
			spawnShipRot = spawnPosRot + PIf/2;	// Face the center.
		}

		// Check for existing ships blocking that spawn location.
		spawnAt = AuPoint(spawnRad, 0, 0).RotateZ(spawnPosRot);
		blocked = IsSpawnLocationBlocked(spawnAt, vessel->Id(), vessel->ObstacleRadius());
	}
	while(blocked);

	// Position the atom in the atmosphere.
	AuMatrix4x4 t;
	t.Translate(spawnAt);
	t.RotateZ(spawnShipRot);
	vessel->ChangePos(AuAtomPos(Id(), t));

	g_Au.AtomAdded(vessel->Id());

	// Start up vessel systems.
	quint8 shipQ;
	if(m_MaximizeShipQ && designId != m_ShipDesign && m_MaxShipQ > designQ)
		shipQ = m_MaxShipQ;
	else
		shipQ = designQ;
	vessel->SetupArena(Name(), team, shipQ, m_SpawnOfficer != No, m_SpawnCrew != No, m_SpawnTroops != No, m_SpawnPassengers != No,
			m_SpawnVehicles != No);

	vessel->SetNormal();	// New atom is ready for normal thread processing.
	vessel->TickTockRecursive();

	// Return the shiny nu ship atom.
	errcode = 0;	// success
	//qDebug() << "DaArena::SpawnSpacecraft" << Id() << Name() << "team" << team << "designId" << designId
		//<< "ship" << vessel->Id() << vessel->Name();
	return vessel;
}

bool DaArena::IsSpawnLocationBlocked(const AuPoint& loc, const AuId& ignoreId, double spawnRad) const
{
	double blockRad = 500*FeetPerMeter() + spawnRad;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if((*it)->Is(ignoreId))
			continue;
		if((*it)->Location().Distance(loc) < blockRad + (*it)->ObstacleRadius())
			return true;
	}
	return false;
}

// This function is called when a chat message is received on a channel that is being monitored by this atom.
// This function is also called when orders are issued to units via chat message.
// 'firstInGroup' is true when this unit is the first in a group of similar units to receive this order message, so only one
// unit acknowledges the order instead of a large group of them.
// 'countSupplied' is used when boarding crew and passengers so multiple cities can take a crack at supplying some citizens
// until the request is filled.
void DaArena::RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup)
{
	// I only respond to Hail messages.
	if(msg->Type() != AuCommPacket::HailRequest)
		return;
	//qDebug() << "DaArena::RxChatMessage" << Id() << Name() << "msg" << msg->Text();

	// The sender must be inside this environment.
	Q_UNUSED(countSupplied);
	Q_UNUSED(firstInGroup);
	const DaAtom* sender = FindIdRecursive(msg->FromId());
	if(!sender)
	{
		//qDebug() << "DaArena::RxChatMessage" << Id() << Name() << "sender not found.";
		return;
	}

	const DaVessel* senderShip = sender->FindParentVesselInEnv();	// Returns sender if it IS a spaceship.
	if(!senderShip)
	{
		//qDebug() << "DaArena::RxChatMessage" << Id() << Name() << "sender" << sender->Id() << sender->Name()
			//<< "parent vessel not found.";
		return;
	}

	switch(m_MatchState)
	{
	case Join:
		{
			m_CommLock.lockForWrite();
			m_ShipsHailed.insert(senderShip->Id());
			m_CommLock.unlock();

			// If all ships present have hailed, we can start the match.
			bool allReady = true;	// Unless we find a ship that is not ready.
			bool team1here = false;
			bool team2here = false;
			DaAtomList spaceships;
			FindVesselsRecursive(spaceships);

			m_CommLock.lockForRead();
			for(DaAtomList::const_iterator si = spaceships.begin(); si != spaceships.end(); si++)
			{
				const DaVessel* ship = (const DaVessel*)*si;
				if(ship->ArenaTeam() == DgEmpire::ArenaTeam1Id())
				{
					//qDebug() << "DaArena::RxChatMessage" << Id() << "team1 ship" << ship->Id() << ship->Name();
					team1here = true;
				}
				else if(ship->ArenaTeam() == DgEmpire::ArenaTeam2Id())
				{
					//qDebug() << "DaArena::RxChatMessage" << Id() << "team2 ship" << ship->Id() << ship->Name();
					team2here = true;
				}

				if(!m_ShipsHailed.contains(ship->Id()))
					allReady = false;
			}
			m_CommLock.unlock();

			if(!team1here || !team2here)
			{
				// The other team doesn't have a ship in the match yet.
				QString text = tr("Opposing fleet needs a spacecraft to start the match. "
						"Match will begin when all spacecraft have hailed.");
				AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(mymsg);
			}
			else if(!allReady)
			{
				// At least one ship has not checked in.
				QString text = tr("Both fleets have spacecraft. Match will begin when all spacecraft have hailed.");
				AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(mymsg);
			}
			else
			{
				// Ready to go.
				m_MatchState = Play;
				SetModified();

				QString text = tr("Match begins. All spacecraft have hailed from both fleets.");
				AuCommChat mymsg(text, AuCommStdChannel::HailChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Hail, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(mymsg);
			}
		}
		break;

	case Play:
		{
			//DHH Hail response during play could state current ships/team states.
		}
		break;

	case Aftermath:
		{
			//DHH Hail response during aftermath could state final ships/team states.
		}
		break;
	}
}

void DaArena::PositionTeamMembers(quint8 team, bool emptyShipsOnly)
{
	DaAtomList people;
	DaAtomList destships;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(*it);
		if(!person)
			continue;
		if(person->ArenaTeam() != team)
			continue;
		if(!person->IsInvisibleOffLine() && person->InArenaOutsideRing())
		{
			// Found a person on line on the team who is floating in space outside the arena ring.
			// Position them aboard the closest ship.
			DaAtomList spaceships;
			for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(*si);
				if(!ship)
					continue;
				if(ship->ArenaTeam() != team)
					continue;
				if(emptyShipsOnly && ship->FindPersonOnLineRecursive())
					continue;	// Skip non-empty ship.
				spaceships.append(ship);
			}

			if(!spaceships.isEmpty())
			{
				// Choose a ship at random, prefering those with no people aboard.
				DaVessel* ship = (DaVessel*)spaceships[g_Au.Dice().Roll(1, spaceships.count(), -1)];
				if(ship)
				{
					// Found a ship on which to place the person.
					people.append(person);
					destships.append(ship);
				}
			}
			break;
		}
	}

	DaAtomList::const_iterator pi = people.begin();
	DaAtomList::const_iterator si = destships.begin();
	while(pi != people.end() && si != destships.end())
	{
		DaPerson* person = (DaPerson*)(*pi);
		DaVessel* ship = (DaVessel*)(*si);
		pi++;
		si++;

		// Place the person aboard the ship, near the helm.
		person->Stand();
		SyncController(AuSyncPose(AuPose::Stand, 0, 0, 0));
		AuMatrix4x4 tPerson;
		tPerson.Translate(ship->HelmLocation() + AuPoint(1,-1,person->CGHeight()));
		person->ForcePosStop(AuAtomPos(ship->Id(), tPerson));
		person->SetModified();
	}
}

void DaArena::SetupArena(const AuArenaReady& arena)
{
	m_MatchState = Join;
	m_MatchType = arena.m_MatchType;
	m_MaximizeShipQ = arena.m_MaximizeShipQ;
	m_MaxShipQ = arena.m_MaxShipQ;
	//m_Name = arena.m_Name;
	m_OpponentDesignsOk = arena.m_OpponentDesignsOk;
	m_Password = arena.m_Password;
	m_ShipDesign = arena.m_ShipDesign;
	m_SpawnCrew = arena.m_SpawnCrew;
	m_SpawnOfficer = arena.m_SpawnOfficer;
	m_SpawnPassengers = arena.m_SpawnPassengers;
	m_SpawnTroops = arena.m_SpawnTroops;
	m_SpawnVehicles = arena.m_SpawnVehicles;
	m_TeamShips = arena.m_TeamShips;
	m_Team1Ships = arena.m_Team1Ships;
	m_Team2Ships = arena.m_Team2Ships;
}

QString DaArena::MatchStateDesc(MATCH_STATE state)
{
	QString states[] =
	{
		//tr("Match is preparing to start. Players and observers are able to join the match."),
		//tr("Match has started. Players may not join the match. Observers are able to join the match."),
		tr("Match is preparing to start. Players may join the match. "
				"Match will start when both teams have a spacecraft and all spacecraft have hailed on the Hail channel."),
		tr("Match has started. Players may not join the match."),
		tr("Match has ended, waiting for players to depart. No one may join."),
	};
	if(state < 0 || state >= sizeof(states)/sizeof(QString))
		return tr("Invalid Match Type %1").arg(state);
	return states[state];
}

QString DaArena::MatchTypeDesc(MATCH_TYPE type)
{
	QString types[] =
	{
		tr("Two fleets of spacecraft fight against each other. The match is announced universally."),
		tr("One fleet of spacecraft fights against an AI commanded fleet. This is a solo match."),
	};
	if(type < 0 || (unsigned int)type >= sizeof(types)/sizeof(QString))
		return tr("Invalid Match Type %1").arg(type);
	return types[type];
}

void DaArena::CreateShellRingVAO(AuOpenGLFunctions& GLf)
{
	Q_ASSERT(!m_ShellRing.VAO());
	// Create a cylinder at the arena bounds.
	QList<AuGlobeTriTexN> tris;												// Triangles that make up the shell ring model.

	double insideRad = c_ArenaBoundsRadius;									// Dist from center to inside face of shell ring.
	double width = FeetPerMoonOrbit()*2;									// Width of shell ring.
	double thickness = 100*FeetPerMeter();									// Thickness of the band.
	int shadowSections = 100;												// Number of sections in the shell ring.
	AuPoint botInPrev;
	AuPoint topInPrev;
	AuPoint botOutPrev;
	AuPoint topOutPrev;
	AuPoint nOutPrev;
	float texXPrev = 0;
	for(int sect=0; sect<=shadowSections; sect++)
	{
		double ang = PI*2. * sect / shadowSections;

		AuPoint nOut = AuPoint(1,0).RotateZ(ang);
		AuPoint botIn = nOut * insideRad + AuPoint(0,0,-width/2);
		AuPoint botOut = botIn + nOut * thickness;
		AuPoint topIn = botIn + AuPoint(0,0,width);
		AuPoint topOut = botOut + AuPoint(0,0,width);
		float texX = (float)sect / (float)shadowSections;

		if(sect != 0)
		{
			// Create a section of the shell ring.
			// Outside face
			AuGlobeTriTexN triOut1;
			triOut1.m_Vert0 = AuGlobeVertTexN(botOutPrev, QPointF(texXPrev,0), nOutPrev);
			triOut1.m_Vert1 = AuGlobeVertTexN(topOutPrev, QPointF(texXPrev,1), nOutPrev);
			triOut1.m_Vert2 = AuGlobeVertTexN(topOut, QPointF(texX,1), nOut);
			tris.append(triOut1);

			AuGlobeTriTexN triOut2;
			triOut2.m_Vert0 = triOut1.m_Vert2;
			triOut2.m_Vert1 = AuGlobeVertTexN(botOut, QPointF(texX,0), nOut);
			triOut2.m_Vert2 = triOut1.m_Vert0;
			tris.append(triOut2);

			// Inside face
			AuGlobeTriTexN triIn1;
			triIn1.m_Vert0 = AuGlobeVertTexN(topInPrev, QPointF(texXPrev,1), -nOutPrev);
			triIn1.m_Vert1 = AuGlobeVertTexN(botInPrev, QPointF(texXPrev,0), -nOutPrev);
			triIn1.m_Vert2 = AuGlobeVertTexN(botIn, QPointF(texX,0), -nOut);
			tris.append(triIn1);

			AuGlobeTriTexN triIn2;
			triIn2.m_Vert0 = triIn1.m_Vert2;
			triIn2.m_Vert1 = AuGlobeVertTexN(topIn, QPointF(texX,1), -nOut);
			triIn2.m_Vert2 = triIn1.m_Vert0;
			tris.append(triIn2);

			// Top face
			AuGlobeTriTexN triTop1;
			triTop1.m_Vert0 = AuGlobeVertTexN(topOutPrev, QPointF(texXPrev,0), AuPoint(0,1));
			triTop1.m_Vert1 = AuGlobeVertTexN(topInPrev, QPointF(texXPrev,1), AuPoint(0,1));
			triTop1.m_Vert2 = AuGlobeVertTexN(topIn, QPointF(texX,1), AuPoint(0,1));
			tris.append(triTop1);

			AuGlobeTriTexN triTop2;
			triTop2.m_Vert0 = triTop1.m_Vert2;
			triTop2.m_Vert1 = AuGlobeVertTexN(topOut, QPointF(texX,0), AuPoint(0,1));
			triTop2.m_Vert2 = triTop1.m_Vert0;
			tris.append(triTop2);

			// Bottom face
			AuGlobeTriTexN triBot1;
			triBot1.m_Vert0 = AuGlobeVertTexN(botInPrev, QPointF(texXPrev,0), AuPoint(0,-1));
			triBot1.m_Vert1 = AuGlobeVertTexN(botOutPrev, QPointF(texXPrev,1), AuPoint(0,-1));
			triBot1.m_Vert2 = AuGlobeVertTexN(botOut, QPointF(texX,1), AuPoint(0,-1));
			tris.append(triBot1);

			AuGlobeTriTexN triBot2;
			triBot2.m_Vert0 = triBot1.m_Vert2;
			triBot2.m_Vert1 = AuGlobeVertTexN(botIn, QPointF(texX,0), AuPoint(0,-1));
			triBot2.m_Vert2 = triBot1.m_Vert0;
			tris.append(triBot2);
		}

		botInPrev = botIn;
		topInPrev = topIn;
		botOutPrev = botOut;
		topOutPrev = topOut;
		nOutPrev = nOut;
		texXPrev = texX;
	}

	// Store count of triangles in the vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeN = new GLfloat[vertices*3];				// normals for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeNIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTexN>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vert 0
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert0.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

		// Vert 1
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert1.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

		// Vert 2
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.x;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.y;
		globeN[globeNIdx++] = (float)(*vi).m_Vert2.m_N.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_ShellRing.CreateGL(GLf, vertices, 3);

	// Select vertex array to construct.
	m_ShellRing.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShellRing.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShellRing.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeNIdx*sizeof(GLfloat), globeN, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_ShellRing.Buffer(2));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_ShellRing.DeselectGL();

	delete [] globePos;
	delete [] globeN;
	delete [] globeTex;
}

void DaArena::RenderShell(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight)
{
	re.glGetError();	//DHH Clear any pending error state.

	if(!m_ShellRing.VAO())
		CreateShellRingVAO(re);

	// Position the panel according to its current rotation around the sun.
	AuMatrix4x4 panelM(model);

	// Configure far globe shader. All planet details are blended into a single surface texture.
	re.GLSL().ShadowSquareShader().Select();
	re.GLSL().ShadowSquareShader().SetProjectionModelView(projection * panelM);
	re.GLSL().ShadowSquareShader().SetTexture(s_TexRingworld);

	// Ambient light
	re.GLSL().ShadowSquareShader().SetAmbientLight(ambientLight);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
	{
		// Sun location was stored in eye space. Shaders want it in the space of this object for lighting calculations.
		const AuGLSun& sun = re.Sun(sunIndex);
		AuPoint sunLoc = panelM.UnProject(sun.m_LocX, sun.m_LocY, sun.m_LocZ);	// Sun location relative to me.

		// Configure shader for object space lighting.
		re.GLSL().ShadowSquareShader().SetSun(sunIndex, sun.m_Enabled, sun.m_Attenuation, sun.m_ColorR, sun.m_ColorG,
				sun.m_ColorB, sunLoc.x, sunLoc.y, sunLoc.z, sun.m_DiskRadius);
	}

	// Render the shadow square.
	m_ShellRing.SelectGL();
	m_ShellRing.DrawGL();

	panelM.RotateX(PI/2);
	re.GLSL().ShadowSquareShader().SetProjectionModelView(projection * panelM);
	m_ShellRing.DrawGL();

	panelM.RotateY(PI/2);
	re.GLSL().ShadowSquareShader().SetProjectionModelView(projection * panelM);
	m_ShellRing.DrawGL();

	m_ShellRing.DeselectGL();

	re.GLSL().ShadowSquareShader().Deselect();
}

quint64 DaArena::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(m_MatchState);
	mem += sizeof(m_MatchType);
	mem += sizeof(m_MaximizeShipQ);
	mem += sizeof(m_MaxShipQ);
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_OpponentDesignsOk);
	mem += sizeof(m_Position);
	mem += sizeof(m_ShipDesign);
	mem += sizeof(m_SpawnCrew);
	mem += sizeof(m_SpawnOfficer);
	mem += sizeof(m_SpawnPassengers);
	mem += sizeof(m_SpawnTroops);
	mem += sizeof(m_SpawnVehicles);
	mem += sizeof(m_TeamShips);
	mem += sizeof(m_Team1Ships);
	mem += sizeof(m_Team2Ships);

	mem += sizeof(m_PasswordLock);
	mem += StringMemoryUsage(m_Password);

	mem += sizeof(m_CommLock);
	mem += sizeof(AuId) * m_ShipsHailed.capacity();
	mem += sizeof(m_tmChannelsMonitored);

	mem += sizeof(m_ShellRing);
	return mem;
}

DaBd* DaArena::FindParentBdInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaBd* DaArena::FindParentBdInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaCapsule* DaArena::FindParentCapsuleInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaCapsule* DaArena::FindParentCapsuleInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaDesignBase* DaArena::FindParentDesignBaseInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaDesignBase* DaArena::FindParentDesignBaseInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaDesigner* DaArena::FindParentDesignerInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaDesigner* DaArena::FindParentDesignerInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorld* DaArena::FindParentPlanetInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorld* DaArena::FindParentPlanetInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaSolarSystem* DaArena::FindParentSolarSystemInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaSolarSystem* DaArena::FindParentSolarSystemInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaStar* DaArena::FindParentStarInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaStar* DaArena::FindParentStarInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaVessel* DaArena::FindParentVesselInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaVessel* DaArena::FindParentVesselInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaWorldLL* DaArena::FindParentWorldInEnv()
{
	return 0;	// I am the environment. The search stops here.
}

const DaWorldLL* DaArena::FindParentWorldInEnv() const
{
	return 0;	// I am the environment. The search stops here.
}

DaArena* DaArena::FindParentArena()
{
	return this;
}

const DaArena* DaArena::FindParentArena() const
{
	return this;
}

DaAtom::ATOM_TYPE DaArena::AtomType() const
{
	return ATOM_Arena;
}

void DaArena::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_MatchState;
	out << m_MatchType;
	out << m_MaximizeShipQ;
	out << m_MaxShipQ;
	out << m_Name;
	out << m_OpponentDesignsOk;
	QTime position;
	out << position;
	out << m_ShipDesign;
	out << m_SpawnCrew;
	out << m_SpawnOfficer;
	out << m_SpawnPassengers;
	out << m_SpawnTroops;
	out << m_SpawnVehicles;
	out << m_TeamShips;
	out << m_Team1Ships;
	out << m_Team2Ships;

	m_PasswordLock.lock();
	out << m_Password;
	m_PasswordLock.unlock();
}

void DaArena::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_MatchState;
	in >> m_MatchType;
	in >> m_MaximizeShipQ;
	in >> m_MaxShipQ;
	in >> m_Name;
	in >> m_OpponentDesignsOk;
	QTime position;
	in >> position;
	in >> m_ShipDesign;
	in >> m_SpawnCrew;
	in >> m_SpawnOfficer;
	in >> m_SpawnPassengers;
	in >> m_SpawnTroops;
	in >> m_SpawnVehicles;
	in >> m_TeamShips;
	in >> m_Team1Ships;
	in >> m_Team2Ships;

	m_PasswordLock.lock();
	in >> m_Password;
	m_PasswordLock.unlock();
}

