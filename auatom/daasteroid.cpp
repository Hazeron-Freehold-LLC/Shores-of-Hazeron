//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daasteroid.cpp,v 1.70 2022/05/17 15:20:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daasteroid.h"
//
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "ausmokeparticles.h"
#include "ausound.h"
#include "ausphere.h"
#include "daatomlist.h"
#include "dabd.h"
#include "daeffect.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "daworld.h"
#include "dgdata.h"
#include <QSqlError>
#include <QSqlQuery>

static GLuint							s_TexSmoke = 0;

DaAsteroid::DaAsteroid(quint8 size)
{
	m_Model = 0;
	m_PSSmoke = 0;
	m_Size = size;
	m_Hits = m_Size * 100;
	m_SpinAxis1 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed1 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
	m_SpinAxis2 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed2 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
}

DaAsteroid::DaAsteroid(const AuId& target, const AuTime& timeOnTarget, const AuTime& timeOfRelease)
{
	m_Model = 0;
	m_PSSmoke = 0;
	m_Size = 39 + g_Au.Dice().Roll(1,6);	// 40 to 45
	m_Hits = m_Size * 100;
	m_SpinAxis1 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed1 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
	m_SpinAxis2 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed2 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
	m_Target = target;
	m_TargetTime = timeOnTarget;
	m_TargetReleaseTime = timeOfRelease;
}

DaAsteroid::DaAsteroid()
{
	m_Model = 0;
	m_PSSmoke = 0;
	m_Size = 40;
	m_Hits = m_Size * 100;
	m_SpinAxis1 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed1 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
	m_SpinAxis2 = AuParticleSystem::GetRandomVector();
	m_SpinSpeed2 = g_Au.Dice().Roll(1,91,-46) * DEG2RADf;	// -45 to 45 degrees per second
}

DaAsteroid::~DaAsteroid()
{
	if(m_PSSmoke)
	{
		delete m_PSSmoke;
		m_PSSmoke = 0;
	}
}

DaAsteroid::DaAsteroid(const DaAsteroid &c) : DaAtom(c)
{
	m_Hits = c.m_Hits;
	m_HitsChange = c.m_HitsChange;
	m_Model = 0;
	m_PSSmoke = 0;	// The will be created when needed.
	m_Size = c.m_Size;
	m_Spin = c.m_Spin;
	m_SpinAxis1 = c.m_SpinAxis1;
	m_SpinSpeed1 = c.m_SpinSpeed1;
	m_SpinAxis2 = c.m_SpinAxis2;
	m_SpinSpeed2 = c.m_SpinSpeed2;
	m_Target = c.m_Target;
	m_TargetTime = c.m_TargetTime;
	m_TargetReleaseTime = c.m_TargetReleaseTime;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaAsteroid::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull())
		hasWhat = 0x01;					// Size and spin do not change so we only have to send them the first time.
	m_AsteroidLock.lockForRead();
	if(lastTx.IsNull() || m_HitsChange >= lastTx)
		hasWhat |= 0x02;				// Hit points
	m_AsteroidLock.unlock();
	toClient << hasWhat;

	toClient << Vector();

	if(hasWhat & 0x01)
	{
		m_AsteroidLock.lockForRead();
		toClient << m_Size;
		toClient << m_SpinAxis1;
		toClient << m_SpinSpeed1;
		toClient << m_SpinAxis2;
		toClient << m_SpinSpeed2;
		m_AsteroidLock.unlock();
	}

	if(hasWhat & 0x02)
	{
		m_AsteroidLock.lockForRead();
		toClient << m_Hits;
		m_AsteroidLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaAsteroid::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;
	fromServer >> hasWhat;

	AuPoint v;
	fromServer >> v;
	SetVector(v);

	if(hasWhat & 0x01)
	{
		m_AsteroidLock.lockForWrite();
		fromServer >> m_Size;
		fromServer >> m_SpinAxis1;
		fromServer >> m_SpinSpeed1;
		fromServer >> m_SpinAxis2;
		fromServer >> m_SpinSpeed2;
		m_AsteroidLock.unlock();
	}

	if(hasWhat & 0x02)
	{
		m_AsteroidLock.lockForWrite();
		fromServer >> m_Hits;
		m_AsteroidLock.unlock();
	}
}

// Refresh data from valid counterpart.
bool DaAsteroid::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaAsteroid* pA = (const DaAsteroid*)pAtom;
	pA->m_AsteroidLock.lockForRead();
	m_AsteroidLock.lockForWrite();
	m_Hits = pA->m_Hits;
	m_Size = pA->m_Size;
	m_SpinAxis1 = pA->m_SpinAxis1;
	m_SpinSpeed1 = pA->m_SpinSpeed1;
	m_SpinAxis2 = pA->m_SpinAxis2;
	m_SpinSpeed2 = pA->m_SpinSpeed2;
	m_AsteroidLock.unlock();
	pA->m_AsteroidLock.unlock();

	SetVector(pA->Vector());

	return false;
}

DaAtom* DaAsteroid::Clone() const
{
	return new DaAsteroid(*this);
}

QString DaAsteroid::Name() const
{
	return tr("Asteroid %1").arg(Id().StringId());
}

QString DaAsteroid::SpotDescription(quint32 spotterEmpire) const
{
	Q_UNUSED(spotterEmpire);
	double diaM = ObstacleRadius() / FeetPerMeter() * 2;	// Diameter in meters.
	int maxHits = m_Size * 100;
	int hp = Hits();
	if(hp == maxHits)
		return tr("%1 %L2m Diameter %L3hp")
			.arg(Name())
			.arg(diaM, 0,'f',0)
			.arg(hp);
	else
		return tr("%1 %L2m Diameter %L3/%L4hp")
			.arg(Name())
			.arg(diaM, 0,'f',0)
			.arg(hp)
			.arg(maxHits);
}

void DaAsteroid::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
		DaAtomList& shadSc,
		DaAtomList& shadScFog,
		DaAtomList& shadScGlobe,
		DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld,
		DaAtomList& shadScRingworldFog)
{
	if(!m_Model || !m_Model->HasGLModel())
		return;

	// Determine where the viewer is with respect to this planet.
	const AuPoint& pv = rmod.ViewLoc(Id());
	if(pv.z >= 0)
		return;	// this item is behind the camera plane
	double viewDistance = pv.Length();
	bool bViewerNearThisPlanet = viewDistance <= FeetPerOrbit()/2;	// true if all detail must be shown.
	if(bViewerNearThisPlanet)
	{
		// Render in the near pass when viewer is near me.
		if(pass != PASS_Near)
			return;
	}
	else
	{
		// Render in the far pass when viewer is far from me.
		if(pass != PASS_Far)
			return;
	}

	// Multiply by current transformation (Translation, rotation, and scale.)
	const AuMatrix4x4& model = rmod.Matrix(Id());
	AuMatrix4x4 tOb = model * m_Spin;

	// Get ambient lighting and fog values.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);
	AuPoint ambientUp = model.ProjectVector(GravityNormal());

	// Disable suns when they are blocked at my location. Prepare to restore them.
	QList<int> sunsDisabled;
	if(sunsBlocked)
		ari.re->DisableSuns(sunsDisabled);

	m_Model->RenderGLSLModelTex(projection, tOb, *ari.re, ambientR, ambientG, ambientB, ambientUp, false, 1, false);

	// Restore suns that I disabled.
	ari.re->EnableSuns(sunsDisabled);

	// Render smoke trail in RenderLast.
	renderLast.Insert(rmod, this);

	// Base atom class needs to render special effects.
	DaAtom::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass,
			shadSc,
			shadScFog,
			shadScGlobe,
			shadScGlobeFog,
			shadScRingworld,
			shadScRingworldFog);
}

void DaAsteroid::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom,
		DaAtomList& shadSc,
		DaAtomList& shadScFog,
		DaAtomList& shadScGlobe,
		DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld,
		DaAtomList& shadScRingworldFog)
{
	DaAtom::RenderLast(projection, ari, rmod, sceneAtom,
			shadSc,
			shadScFog,
			shadScGlobe,
			shadScGlobeFog,
			shadScRingworld,
			shadScRingworldFog);

	if(!s_TexSmoke)
		s_TexSmoke = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SmokeT", ":data/smoketransparent.png");

	if(!m_PSSmoke)
	{
		float size = ObstacleRadius() / 2;
		m_PSSmoke = new AuSmokeParticles();
		m_PSSmoke->SetMaxParticles(1000);
		m_PSSmoke->SetLifeCycle(7.5f);
		m_PSSmoke->SetVelocityVar(size / 2.f);
		m_PSSmoke->SetNumToRelease(1);
		m_PSSmoke->SetReleaseInterval(0.25f);
		QColor colSmoke(193,169,121);
		m_PSSmoke->SetColor(colSmoke);
		m_PSSmoke->SetSize(size);
		m_PSSmoke->SetTextureID(s_TexSmoke);
		m_PSSmoke->SetPosition(AuPoint(0.0f, 0.0f, 0.0f));
		m_PSSmoke->Init(*ari.re);
	}
	if(Vector().IsZero())
	{
		m_PSSmoke->SetReleaseInterval(0.25f);
		m_PSSmoke->SetVelocity(AuPoint(0,0));
	}
	else
	{
		m_PSSmoke->SetReleaseInterval(0.05f);
		m_PSSmoke->SetVelocity(-Vector());
	}

	const AuMatrix4x4& model = rmod.Matrix(Id());
	ari.re->GLSL().SpriteSizeShader().Select();
	ari.re->GLSL().SpriteSizeShader().SetProjectionModelView(projection * model);
	ari.re->GLSL().SpriteSizeShader().SetTexture(s_TexSmoke);
	ari.re->GLSL().SpriteSizeShader().SetColor(m_PSSmoke->GetColor());
	ari.re->GLSL().SpriteSizeShader().SetSize(m_PSSmoke->GetSize()*500);

	// Render the smoke.
	ari.re->glEnable(GL_DEPTH_TEST);
	ari.re->glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if(m_PSSmoke->Update())
		m_PSSmoke->Render(projection, model, *ari.re);
	ari.re->glDepthMask(GL_TRUE);
	ari.re->glDisable(GL_BLEND);

	ari.re->GLSL().SpriteSizeShader().Deselect();
}

void DaAsteroid::Create()
{
	DaAtom::Create();

	// Grab the model data right away so whoever created this item can
	// figure out how to place it in the environment.
	QString modelFile = ModelName();
	if(!modelFile.isEmpty())
	{
		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(ModelName(), ObstacleRadius()));
		g_Au.UnlockGLModels();
	}
}

// Drawn facing the negative Y direction with origin at 0,0.
void DaAsteroid::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::CreateDeviceObjects(ari);
	if(!m_Model)
	{
		// Start loading the 3ds model of the commodity using a worker thread.
		// This does not create the open gl display list.
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(ModelName(), true, ObstacleRadius()));
	}
	else if(!m_Model->HasGLModel())
		m_Model->GLModel(*ari.re);						// GL display list isn't done yet. Attempt to create the GL display list.
}

bool DaAsteroid::RoughExtents(AuBox &fExt) const
{
	if(m_Model)
	{
		// Use 3D extents of the 3ds model.
		AuPoint min,max;
		m_Model->GetExtents(min, max);
		fExt.SetFromMinMax(min, max);
	}
	return true;
}

void DaAsteroid::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::DeleteDeviceObjects(ari);
}

void DaAsteroid::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();
	AuPoint vVec = Vector();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_Asteroid (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
			"timeCreate,timeModified,timeTock,timeDeath, "
			"atRest,vecX,vecY,vecZ, "
			"hits,size,targetId,targetTime,releaseTime, "
			"spinAxis1X,spinAxis1Y,spinAxis1Z,spinSpeed1, spinAxis2X,spinAxis2Y,spinAxis2Z,spinSpeed2"
			") VALUES (?,?,?,?,?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,?,"
			"?,?,?,?, ?,?,?,?);");
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

	// DaAsteroid values.
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(vVec.x));
	qa.addBindValue(FuzzyClamp(vVec.y));
	qa.addBindValue(FuzzyClamp(vVec.z));
	qa.addBindValue(m_Hits);
	qa.addBindValue(m_Size);
	qa.addBindValue(m_Target.GetID());
	qa.addBindValue(m_TargetTime.UTC());
	qa.addBindValue(m_TargetReleaseTime.UTC());
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.x));
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.y));
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.z));
	qa.addBindValue(m_SpinSpeed1);
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.x));
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.y));
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.z));
	qa.addBindValue(m_SpinSpeed2);

	if(!qa.exec())
		qWarning("DaAsteroid::Insert %s FAILED.", (const char*)Name().toLatin1());
}

void DaAsteroid::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();
	AuPoint vVec = Vector();

	QString sq;
	sq = QString("UPDATE a_Asteroid SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, "
			"timeModified=?, timeTock=?, timeDeath=?, "
			"atRest=?, vecX=?, vecY=?, vecZ=?,  "
			"hits=?, size=?, targetId=?, targetTime=?, releaseTime=?, "
			"spinAxis1X=?, spinAxis1Y=?, spinAxis1Z=?, spinSpeed1=?,  spinAxis2X=?, spinAxis2Y=?, spinAxis2Z=?, spinSpeed2=? "
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

	// DaAsteroid values.
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(vVec.x));
	qa.addBindValue(FuzzyClamp(vVec.y));
	qa.addBindValue(FuzzyClamp(vVec.z));
	qa.addBindValue(m_Hits);
	qa.addBindValue(m_Size);
	qa.addBindValue(m_Target.GetID());
	qa.addBindValue(m_TargetTime.UTC());
	qa.addBindValue(m_TargetReleaseTime.UTC());
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.x));
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.y));
	qa.addBindValue(FuzzyClamp(m_SpinAxis1.z));
	qa.addBindValue(m_SpinSpeed1);
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.x));
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.y));
	qa.addBindValue(FuzzyClamp(m_SpinAxis2.z));
	qa.addBindValue(m_SpinSpeed2);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaAsteroid::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaAsteroid::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

DaAsteroid* DaAsteroid::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, "
			"atRest,vecX,vecY,vecZ, "
			"hits,size,targetId,targetTime,releaseTime, "
			"spinAxis1X,spinAxis1Y,spinAxis1Z,spinSpeed1, spinAxis2X,spinAxis2Y,spinAxis2Z,spinSpeed2 "
			"FROM a_Asteroid WHERE ");
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
		DaAsteroid *pnu = new DaAsteroid;
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

			// DaAsteroid values.
			bool atRest = qa.value(qv++).toBool();
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			if(!atRest)
			{
				pnu->SetVector(AuPoint(vx, vy, vz));
				pnu->SetInMotion();
			}

			pnu->m_Hits = qa.value(qv++).toUInt();
			pnu->m_HitsChange.Set();
			pnu->m_Size = qa.value(qv++).toUInt();
			if(pnu->m_Size < 5)
				pnu->m_Size *= 10;	// Scale old 1-4 size range up to 10-40. (valid range is 6 to 45).
			pnu->m_Target = AuId(qa.value(qv++).toUInt());
			QDateTime tmTarg = qa.value(qv++).toDateTime();
			tmTarg.setTimeSpec(Qt::UTC);
			pnu->m_TargetTime = tmTarg;
			QDateTime tmRel = qa.value(qv++).toDateTime();
			tmRel.setTimeSpec(Qt::UTC);
			pnu->m_TargetReleaseTime = tmRel;

			double s1x = qa.value(qv++).toDouble();
			double s1y = qa.value(qv++).toDouble();
			double s1z = qa.value(qv++).toDouble();
			pnu->m_SpinAxis1 = AuPoint(s1x, s1y, s1z);
			pnu->m_SpinSpeed1 = qa.value(qv++).toFloat();

			double s2x = qa.value(qv++).toDouble();
			double s2y = qa.value(qv++).toDouble();
			double s2z = qa.value(qv++).toDouble();
			pnu->m_SpinAxis2 = AuPoint(s2x, s2y, s2z);
			pnu->m_SpinSpeed2 = qa.value(qv++).toFloat();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

void DaAsteroid::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaAtom::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	// Rotate about the spin axes.
	qint64 msElapsed = lastAnim.elapsed();
	if(msElapsed > 0)
	{
		m_AsteroidLock.lockForWrite();
		if(!m_SpinAxis1.IsZero() && m_SpinSpeed1 != 0)
		{
			double angle = m_SpinSpeed1 * (float)msElapsed / 1000.f;
			m_Spin.RotateAxis(angle, m_SpinAxis1);
		}
		if(!m_SpinAxis2.IsZero() && m_SpinSpeed2 != 0)
		{
			double angle = m_SpinSpeed2 * (float)msElapsed / 1000.f;
			m_Spin.RotateAxis(angle, m_SpinAxis2);
		}
		m_AsteroidLock.unlock();
	}
}

bool DaAsteroid::IsTicker() const
{
	if(IsDead())
		return false;
	if(IsAwaitingRelease())
		return false;	// Not ready for release. Don't waste time ticking.
	return true;
}

// Return false if this atom is to be deleted upon return.
void DaAsteroid::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	if(!m_Model)
	{
		AuGLModelCache& models = g_Au.LockGLModelsForWrite();
		m_Model = dynamic_cast<Au3dsGLSLModelInstance*>(models.CreateGeometry(ModelName(), ObstacleRadius()));
		g_Au.UnlockGLModels();
	}

	m_AsteroidLock.lockForRead();
	AuId targetId(m_Target);
	AuTime timeOnTarget(m_TargetTime);
	AuTime timeRelease(m_TargetReleaseTime);
	m_AsteroidLock.unlock();

	if(!targetId.IsNull())
	{
		// I am going after a specific target.
		AuTime tnow(AuTime::Now());
		if(timeRelease > tnow)
		{
			// Not ready for launch.
			if(!IsAtRest())
				SetAtRest();
		}
		else
		{
			// Set my movement vector to hit the target at the appropriate time.
			DaSolarSystem* system = FindParentSolarSystem();
			if(system)
			{
				bool forgetTarget = false;
				DaAtom* target = system->FindIdRecursive(targetId);
				if(target)
				{
					AuPoint vecToTarget = target->EnvLocation() - EnvLocation();
					vecToTarget = EnvTransform().UnProjectVector(vecToTarget);	// Convert the vector to be relative to me.
					double dist = vecToTarget.Length();							// Distance to the target in feet.
					double secs = tnow.MSecsTo(timeOnTarget) / 1000.;			// Time to scheduled impact.
					if(secs <= 0)
					{
						forgetTarget = true;									// Too late? Forget the target.
						vecToTarget.Unit(TargetPursueSpeed());					// Standard asteroid speed when things aren't right.
					}
					else
						vecToTarget.Unit(dist / secs);							// Velocity needed to hit the target on time.
					if(vecToTarget != Vector())
					{
						SetVector(vecToTarget);									// Fly like a guided missile.
						SetModified();
					}
					if(IsAtRest())
						SetInMotion();
				}
				else
					forgetTarget = true;										// Target not found in the system.

				if(forgetTarget)
				{
					m_AsteroidLock.lockForWrite();
					m_Target = AuId(0);
					m_AsteroidLock.unlock();
				}
			}
		}
	}
	else if(IsAtRest())
		SetInMotion();	// Untargetted asteroid keeps moving with gravity.

	DaAtom::Tick(last, now);
	if(IsDead())
		return;

	if(IsOnGround() || IsInWater() || (IsInAtmosphere() && AltitudeAGL() < GroundHeightAboveCG()))
	{
		// Grab my world's name and type before it gets destroyed.
		BODY_TYPE bodyType;
		const DaWorld* parentWorld = dynamic_cast<const DaWorld*>(Parent());
		QString name;
		if(parentWorld)
		{
			bodyType = parentWorld->BodyType();
			name = parentWorld->Name();
		}
		else
			bodyType = BODY_Planet;

		HitPlanet();
		if(parentWorld && parentWorld->IsDead())
			NotifyObservers(ASTEROID_DestroyedWorld, bodyType, name);
		else
			NotifyObservers(ASTEROID_HitWorld, bodyType, name);
	}
	else
	{
		// Test for solar radiation damage if my parent is a sun.
		const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
		if(parentStar)
		{
			ApplySolarRadiation(parentStar, 1000);
			if(IsDead())
			{
				// Destroyed by collision with sun.
				BODY_TYPE bodyType = parentStar->BodyType();
				if(parentStar->IsAnyBlackHole())
					NotifyObservers(ASTEROID_HitBlackHole, bodyType, parentStar->Name());
				else
					NotifyObservers(ASTEROID_HitSun, bodyType, parentStar->Name());
			}
		}
	}
}

unsigned int DaAsteroid::IsTocker() const
{
	unsigned int ms = DaAtom::IsTocker();
	if(!IsNormal())
		return ms;
	if(ms < d_MSecPerMin*10)
		ms = d_MSecPerMin*10;	// Tock me every ten minutes.
	return ms;
}

// Return false if this atom is to be deleted upon return.
void DaAsteroid::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaAtom::Tock(last, now, scripts, tockTrans);
	if(now.IsNull())
		return;

	if(!IsInAnyScene() &&
			(TargetId().IsNull() || CreatedTime().HoursTo(now) > 24*7) &&
			dynamic_cast<const DaSolarSystem*>(Parent()))
	{
		// Asteroids are started one week from the target. This puts them in deep space.
		// After a week, they get killed if they return to deep space.
		// They also get killed in deep space if they have no target.
		//if(TargetId().IsNull())
			//qDebug() << "DaAsteroid::Tock killing" << *this << "deep space and no target";
		//else
			//qDebug() << "DaAsteroid::Tock killing" << *this << "deep space and too old" << CreatedTime() << now;
		NotifyObservers(ASTEROID_DepartedDeepSpace, BODY_TYPE_Count, QString()/*na*/);
		Kill();	// Remove asteroids that reach deep space.
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

void DaAsteroid::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	Q_UNUSED(mode);
	Q_UNUSED(effect);
	Q_UNUSED(hitShield);
	Q_UNUSED(wpnLoc);
	Q_UNUSED(hitLoc);
	// Get base armor values from the vehicle.
	float armorModeAbsorb = 20.f/100.f;
	float armorModeBlock = 20.f;
	float armorEffectAbsorb = 0.f;
	float armorEffectBlock = 0.f;
	baseAbsorb = armorModeAbsorb + armorEffectAbsorb;
	baseBlock = armorModeBlock + armorEffectBlock;
	auxAbsorb = 0;
	auxBlock = 0;
}

void DaAsteroid::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	DaAtom::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);
	if(weapon.CommodityId() == CID_SunBurn || weapon.CommodityId() == CID_Spaghettification)
	{
		//qDebug() << "DaAsteroid::CombatApplyDamage" << *this << attacker.m_Name;
		Kill();
		return;
	}

	if(!attacker.m_Id.IsNull())
	{
		DaSolarSystem* system = Parent()->FindParentSolarSystem();
		if(!system)
			return;
		DaUnit* unit = dynamic_cast<DaUnit*>(system->FindIdRecursive(attacker.m_Id));
		if(unit)
		{
			if(weapon.CommodityId() == CID_Collision)
			{
				// Units of all kinds are destroyed by asteroids on contact.
				AuCombatAttacker attacker(tr("asteroid collision"));

				AuCombatWeapon weapon(WM_Contact);
				weapon.m_CID = CID_Collision;
				weapon.m_w1.m_Effect = WE_Bludgeon;
				
				AuCombatResult result;
				result.m_Damage1 = AsteroidSize() * 100000000UL;

				unit->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			}
		}
	}

	if(!result.m_Damage1 && !result.m_Damage2)
		return;	// No damage.

	m_AsteroidLock.lockForWrite();
	if(result.m_Damage1)
	{
		if(result.m_Damage1 >= m_Hits)
			m_Hits = 0;
		else
			m_Hits -= result.m_Damage1;
		m_HitsChange.Set();
	}
	if(result.m_Damage2)
	{
		if(result.m_Damage2 >= m_Hits)
			m_Hits = 0;
		else
			m_Hits -= result.m_Damage2;
		m_HitsChange.Set();
	}
	int finalHits = m_Hits;
	m_AsteroidLock.unlock();

	if(!finalHits)
		Explode(Location());
}

void DaAsteroid::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	m_AsteroidLock.lockForRead();
	out << m_Hits;
	out << AuTime();	//m_HitsChange;
	out << m_Size;
	out << m_SpinAxis1;
	out << m_SpinSpeed1;
	out << m_SpinAxis2;
	out << m_SpinSpeed2;
	out << m_Target;
	out << m_TargetTime;
	out << m_TargetReleaseTime;
	m_AsteroidLock.unlock();
}

void DaAsteroid::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	m_AsteroidLock.lockForWrite();
	in >> m_Hits;
	AuTime hitsChange;
	in >> hitsChange;
	in >> m_Size;
	in >> m_SpinAxis1;
	in >> m_SpinSpeed1;
	in >> m_SpinAxis2;
	in >> m_SpinSpeed2;
	in >> m_Target;
	in >> m_TargetTime;
	in >> m_TargetReleaseTime;
	m_AsteroidLock.unlock();
}

AuPoint DaAsteroid::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	return DaAtom::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

bool DaAsteroid::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(sceneAtom);
	if(IsAwaitingRelease())
		return false;
	// Calculate scene atom location in my coordinate space.
	AuPoint sceneLoc = EnvTransform().UnProject(cameraEnvLoc);
	return sceneLoc.Length2() < (FeetPerSolarSystem() * FeetPerSolarSystem());
}

// Acceleration/deceleration in ft/sec/sec due to gravity and other factors.
AuPoint DaAsteroid::Acceleration() const
{
	return DaAtom::Acceleration();
}

// Returns friction in ft/sec/sec, typically due to surface contact.
AuPoint DaAsteroid::Friction() const
{
	double fricFwd = 0, fricLat = 0, fricVrt = 0;	// forward is +Z. lateral is X and vertical is Y.
	return AuPoint(fricLat, fricVrt, fricFwd);
}

void DaAsteroid::HitPlanet()
{
	DaWorld* world = dynamic_cast<DaWorld*>(Parent());
	if(!world)
		return;
	if(!world->IsNormal())
		return;
	if(world->HitByAsteroid(AsteroidSize(), Location()))
		Kill();
	else
		Explode(Location());
}

void DaAsteroid::Explode(const AuPoint& explodeLoc)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	DaAtom* parent = Parent();
	if(!parent)
		return;
	// Mark me dormant immediately so my own explosion does not hit me again.
	SetDormant();		// So I am ignored by my explosion.

	// Break into smaller rocks.
	int asize = AsteroidSize();
	quint8 newSize = asize - g_Au.Dice().Roll(1,4,2);	// subtract 3-6
	while(newSize >= 6)
	{
		// Pick a random direction to go. Smaller rocks move faster.
		AuPoint godir = AuParticleSystem::GetRandomVector();
		AuPoint roidVec = Vector() + godir * (g_Au.Dice().Roll(1,900 - newSize*20,200) * FeetPerMeter());

		AuMatrix4x4 troid(Transform());
		troid.Translate(godir * ObstacleRadius()/3);

		DaAsteroid* roid = new DaAsteroid(newSize);
		roid->ConnectSignals();
		roid->Create();
		roid->SetPosAndOrient(AuAtomPos(ParentId(), troid));
		ether->AddAtom(roid);

		roid->SetVector(roidVec);
		roid->SetInMotion();
		g_Au.AtomAdded(roid->Id());
		roid->SetNormal();					// New atom is ready for normal thread processing.
		if(IsInAnyScene())
			roid->SetInScene();				// So it doesn't get deleted right away in deep space.
		roid->TickTock();

		newSize -= g_Au.Dice().Roll(1,4,2);	// subtract 3-6
	}

	// Create an effect atom to carry the explosion effect.
	DaEffect* explode = parent->CreateEffect();
	if(explode)
	{
		WEAPONEFFECT ex2s[] =	// Secondary explosion effect because I can't decide.
		{
			WE_Radiation,
			WE_Electric,
			WE_Bludgeon,
			WE_Crush,
			WE_Heat,
			WE_Pierce,
			WE_Slash,
		};
		int ex2cnt = sizeof(ex2s) / sizeof(WEAPONEFFECT);
		WEAPONEFFECT we2 = ex2s[g_Au.Dice().Roll(1,ex2cnt,-1)];
		float radm2 = g_Au.Dice().Roll(1,60,20) / 10.f;
		float gain = (float)ObstacleRadius() / 10.f;
		float pitch = .5f + (5.f - asize/10.f)/15.f + (float)g_Au.Dice().Roll(1,100,-1)/1000.f;
		AuEffect effect(explodeLoc, AuPoint(0,0), AuPoint(0,0), SOUND_ExplodeVehicle, gain, pitch, VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Heat, (float)ObstacleRadius() * 1.5f);
		effect.SetExplosionEffect2(we2, (float)ObstacleRadius() * radm2);
		explode->AddEffect(effect);
	}
	Kill();	// It's ok to trash me now.
}

QString DaAsteroid::ModelName() const
{
	// AsteroindN.3ds files are all unit-sized asteroids. They get scaled to their final size.
	QString models[] =
	{
		":data/Asteroid1.3ds",
		":data/Asteroid2.3ds",
		":data/Asteroid3.3ds",
		":data/Asteroid4.3ds",
	};
	int modelIndex = Id().GetID() % 4;
	return models[modelIndex];
}

DaAtom* DaAsteroid::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	Q_UNUSED(sightingAtom);
	Q_UNUSED(sightedShield);
	if(IsAwaitingRelease())
		return pBest;
	// Spherical bodies can be tested readily to see if the segment passes within their volume.
	AuSphere sphere(tModel.Project(AuPoint(0,0)), ObstacleRadius());
	AuPoint hit1, hit2;
	if(!sphere.Intersect(eyeLoc, sightedLoc, hit1, hit2))
		return pBest;									// The segment does not enter the sphere of this asteroid.

	sightedLoc = hit1;
	if(sightedSurfaceNormal)
	{
		hit1.Unit();
		*sightedSurfaceNormal = hit1;
	}
	sightedPart = 0;
	return (DaAtom*)this;								// Must cast me to non-const.
}

DaAtom::ATOM_TYPE DaAsteroid::AtomType() const
{
	return ATOM_Asteroid;
}

const DaAsteroid* DaAsteroid::FindAsteroidRecursive(const AuId& targetId) const
{
	// Asteroids never have subatoms.
	if(TargetId() != targetId)
		return 0;
	return this;
}

void DaAsteroid::FindAsteroidsRecursive(DaAtomList& asteroids) const
{
	//if(!IsNormal())
		//return;
	asteroids.append((DaAtom*)this);	// Cast away constness.
}

//static
double DaAsteroid::TargetPursueSpeed()
{
	return 1000.f * FeetPerMeter();
}

bool DaAsteroid::IsSightTarget(const DaAtom* toWho) const
{
	Q_UNUSED(toWho);
	if(IsAwaitingRelease())
		return false;
	return true;
}

bool DaAsteroid::IsObstacle(const DaAtom* toWho) const
{
	if(IsAwaitingRelease())
		return false;
	if(dynamic_cast<const DaAsteroid*>(toWho))
		return false;
	return true;
}

bool DaAsteroid::IsAwaitingRelease() const
{
	m_AsteroidLock.lockForRead();
	if(m_Target.IsNull())
	{
		m_AsteroidLock.unlock();
		return false;	// Not targetted on an atom.
	}
	else if(m_TargetReleaseTime < AuTime::Now())
	{
		m_AsteroidLock.unlock();
		return false;	// Target release time has already arrived or there was no specified release time.
	}
	m_AsteroidLock.unlock();
	return true;		// I am waiting to be released to approach my target atom.
}

void DaAsteroid::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld, float scannerSensorRange, float scannerWeaponRange, int scanContent,
		const AuId& lockedBlip, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	Q_UNUSED(scannerEmpire);
	Q_UNUSED(scannerId);
	Q_UNUSED(scannerLoc);
	Q_UNUSED(scannerSensorRange);
	Q_UNUSED(scannerWeaponRange);
	Q_UNUSED(stanceCache);
	// Do not detect asteroids that are waiting to release.
	if(IsAwaitingRelease())
		return;

	// Must scan me if looking for locked-on blip and I'm it.
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles, weapon consoles, AI pilots, and AI turret gunners.
		//   b0 = sensor console blips
		//   b1 = fire control console blips
		//   b2 = transporter console blips
		//   b3 = locked-on blip
		//   b4 = AI pilot obstacle blips
		//   b5 = AI turret gunner target blips
		//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
		if(!(scanContent & 0x0033))
			return;
	}

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	blips.Add(AuBlipAsteroid(AsteroidSize(), Hits(), Id(), EnvLocation()));
}

bool DaAsteroid::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerEmpire);
	Q_UNUSED(gunnerPosture);
	if(IsAwaitingRelease())
		return false;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
	if(targetVec.Length2() > gunRange*gunRange)
		return false;

	if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
		return false;

	targetSysLoc = EnvTransform().Project(TargetLocation());
	return true;
}

bool DaAsteroid::IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerEmpire);
	Q_UNUSED(gunnerPosture);
	if(IsAwaitingRelease())
		return false;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;

	// Perform range test.
	AuPoint mysysloc = EnvTransform().Project(TargetLocation());
	AuPoint targetVec = mysysloc - shipSysLoc;					// Vector to target from ship's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return false;

	targetSysLoc = mysysloc;
	return true;
}

void DaAsteroid::FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc)
{
	Q_UNUSED(gunnerEmpire);
	if(IsAwaitingRelease())
		return;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, AuId()))
		return;
	AuPoint mysysloc = EnvTransform().Project(TargetLocation());
	double range2 = gunSysLoc.Distance2(mysysloc);
	if(range2 > gunRange*gunRange)
		return;		// I am out of range of the weapon.

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = mysysloc;
}

const DaAtom* DaAsteroid::FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
		quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
		double& bestDistance) const
{
	Q_UNUSED(launcherId);
	Q_UNUSED(missileEmpireId);
	if(IsAwaitingRelease())
		return bestAtom;
	if(!IsGunnerDomainTarget(missileDomain, missileWorldId, launcherId))
		return bestAtom;
	double dist = EnvLocation().Distance(missileSysLoc);
	if(bestDistance < 0. || dist < bestDistance)
	{
		bestDistance = dist;
		bestSysLoc = EnvLocation();
		return this;
	}

	// Don't check my subatoms; they can't be targetted.
	return bestAtom;
}

void DaAsteroid::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	Q_UNUSED(shipId);
	Q_UNUSED(gunnerEmpire);
	Q_UNUSED(gunnerPosture);
	if(IsAwaitingRelease())
		return;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - gunSysLoc;		// Vector to target from gun's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return;

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	if(targetVec.Dot(gunSysVec) <= 0)						// Test field of fire.
		return;

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = EnvTransform().Project(TargetLocation());
}

void DaAsteroid::FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, const DaAtom*& targetAtom,
		double& targetRange, AuPoint& targetSysLoc) const
{
	AuPoint mysysloc;
	if(!IsShipWeaponTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, shipSysLoc, gunRange, mysysloc))
		return;

	double range2 = mysysloc.Distance2(shipSysLoc);		// Vector to target from ship's location.
	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	//qDebug() << "DaVessel::FindClosestShipWeaponTarget" << Id() << Name() << "in range";
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = mysysloc;
}

double DaAsteroid::PilotAvoidRadius() const
{
	return ObstacleRadius() + 1000. * FeetPerMeter();
}

void DaAsteroid::NotifyObservers(ASTEROID_OBSERVATION obs, BODY_TYPE bodyType, const QString& name) const
{
	AuEventLogEntry le;
	switch(obs)
	{
	case ASTEROID_DepartedDeepSpace:
		le = AuEventLogEntry(AuEventLogEntry::AsteroidDepartedDeepSpace);
		break;
	case ASTEROID_HitBlackHole:
		le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionBlackHole, name);
		break;
	case ASTEROID_HitSun:
		le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionSun, name);
		break;
	case ASTEROID_HitWorld:
		switch(bodyType)
		{
		case BODY_GasGiant:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionGasGiant, name);
			break;
		case BODY_Titan:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionTitan, name);
			break;
		case BODY_Moon:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionMoon, name);
			break;
		case BODY_Planet:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionPlanet, name);
			break;
		case BODY_Ringworld:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionRingworld, name);
			break;
		case BODY_Ring:
		case BODY_TypeOStar:
		case BODY_TypeBStar:
		case BODY_TypeAStar:
		case BODY_TypeFStar:
		case BODY_TypeGStar:
		case BODY_TypeKStar:
		case BODY_TypeMStar:
		case BODY_SupermassiveBlackHole:
		case BODY_StellarBlackHole:
		case BODY_NeutronStar:
		case BODY_SolarSystemSpace:
		case BODY_SectorSpace:
		case BODY_TYPE_Count:
			// wut?
			break;
		case BODY_Planetoid:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionPlanetoid, name);
			break;
		}
		break;
	case ASTEROID_DestroyedWorld:
		switch(bodyType)
		{
		case BODY_Titan:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionTitanDestroyed, name);
			break;
		case BODY_Moon:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionMoonDestroyed, name);
			break;
		case BODY_Planet:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionPlanetDestroyed, name);
			break;
		case BODY_GasGiant:
		case BODY_Ringworld:
		case BODY_Ring:
		case BODY_TypeOStar:
		case BODY_TypeBStar:
		case BODY_TypeAStar:
		case BODY_TypeFStar:
		case BODY_TypeGStar:
		case BODY_TypeKStar:
		case BODY_TypeMStar:
		case BODY_SupermassiveBlackHole:
		case BODY_StellarBlackHole:
		case BODY_NeutronStar:
		case BODY_SolarSystemSpace:
		case BODY_SectorSpace:
		case BODY_TYPE_Count:
			// wut?
			break;
		case BODY_Planetoid:
			le = AuEventLogEntry(AuEventLogEntry::AsteroidCollisionPlanetoidDestroyed, name);
			break;
		}
		break;
	}
	if(le.m_Type == AuEventLogEntry::Invalid)
		return;
	const DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(!system)
		return;	// wut?

	DaAtomList bds;
	system->FindBdRecursive(bds);
	for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*bi);
		if(!bd)
			continue;
		if(!bd->IsNormal())
			continue;
		if(bd->IndustryId() != IID_Observatory)
			bd->LogEvent(le);
		else if(bd->SensorsWorking())
		{
			double range = bd->EnvLocation().Distance(EnvLocation());
			if(range <= bd->SensorRange())
				bd->LogEvent(le);
		}
	}
}

bool DaAsteroid::IsEthereal() const
{
	return IsAwaitingRelease();	// Ethereal while awaiting release.
}

QString DaAsteroid::SystemScanReport(int scanEx) const
{
	Q_UNUSED(scanEx);
	double rdia = ObstacleRadius() * 2.;	// Diameter in feet.
	rdia /= FeetPerMeter();			// Convert to meters.
	rdia = round(rdia/100) * 10;	// Round to nearest 10 meters.
	QString sSize = QObject::tr("%L1m Diameter").arg(rdia);

	// Put it all together.
	QString rpt = QString("<html><big><b>%1</b></big><p>").arg(Name());

	rpt += QObject::tr("Eccentric Orbit");

	rpt += QObject::tr("<hr><b>Geosphere</b><br>");
	rpt += QObject::tr("Asteroid") + ", " + sSize;

	rpt += "</html>";
	return rpt;
}

