//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dauniverse.cpp,v 1.70 2022/05/17 16:48:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dauniverse.h"
//
#include <auglobal.h>
#include "daatomlist.h"
#include "dagalaxy.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

DaUniverse::DaUniverse()
{
}

DaUniverse::~DaUniverse()
{
}

DaUniverse::DaUniverse(const DaUniverse &rhs) : DaAtom(rhs)
{
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaUniverse::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
		const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
}

// Receive updates to this atom from a server.
void DaUniverse::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
}

// Refresh data from valid counterpart.
bool DaUniverse::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	return DaAtom::Refresh(pAtom, ari);
}

DaAtom *DaUniverse::Clone() const
{
	return new DaUniverse(*this);
}

void DaUniverse::Create()
{
	DaAtom::Create(AuId::Universe());
}

void DaUniverse::InitGalaxies()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Create all galaxies.
	for(int galaxyIdx=0; galaxyIdx < GALAXY_Count; galaxyIdx++)
	{
		// Create a galaxy.
		DaGalaxy *pGalaxy = new DaGalaxy((GALAXY)galaxyIdx);
		pGalaxy->ConnectSignals();
		pGalaxy->Create();
		pGalaxy->InitPos(Id());
		ether->AddAtom(pGalaxy);

		// Galaxy must emit the signal because this Universe has not been added to its parent yet and so has no
		// signal/slot connections.
		g_Au.AtomAdded(pGalaxy->Id());
		pGalaxy->SetNormal();	// New atom is ready for normal thread processing.
	}
}

void DaUniverse::CreateMissingGalaxies()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	// Create all galaxies that are missing.
	for(int galaxyIdx=0; galaxyIdx < GALAXY_Count; galaxyIdx++)
	{
		DaGalaxy* galaxy = FindGalaxy((GALAXY)galaxyIdx);
		if(galaxy)
		{
			// Insure the galaxy is positioned properly.
			galaxy->InitPos(Id());
		}
		else
		{
			// Create a galaxy.
			DaGalaxy *pGalaxy = new DaGalaxy((GALAXY)galaxyIdx);
			pGalaxy->ConnectSignals();
			pGalaxy->Create();
			pGalaxy->InitPos(Id());
			ether->AddAtom(pGalaxy);

			// Galaxy must emit the signal because this Universe has not been added to its parent yet and so has no
			// signal/slot connections.
			g_Au.AtomAdded(pGalaxy->Id());
			pGalaxy->SetNormal();	// New atom is ready for normal thread processing.
			qDebug() << "DaUniverse::CreateMissingGalaxies created galaxy" << *pGalaxy;
		}
	}
}

// Insert first time database record.
void DaUniverse::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq;
	sq = QString("INSERT INTO a_Universe (id,idp, locX,locY,locZ, rotX,rotY,rotZ, "
			"timeCreate,timeModified,timeTock,timeDeath) "
			"VALUES (?,?, ?,?,?, ?,?,?, ?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(0);	// Universe is never subordinate to anyone.
	//qa.addBindValue(ParentId().GetID());
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

	if(!qa.exec())
		qWarning() << "error: DaUniverse::Insert failed." << sq;
}

// Update existing database record.
void DaUniverse::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq = QString("UPDATE a_Universe SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=? WHERE id=%1;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(0);	// Universe is never subordinate to anyone.
	//qa.addBindValue(ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaUniverse::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaUniverse::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaUniverse* DaUniverse::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,timeTock,timeDeath FROM a_Universe");
	if(!idToLoad.IsNull())
		sq += QString(" WHERE id=%1").arg(idToLoad.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaUniverse *pnu = new DaUniverse;
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

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

void DaUniverse::RemoveGamePiecesFromStarField()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaGalaxy* pAtom = dynamic_cast<DaGalaxy*>(*it);
		if(!pAtom)
			continue;
		pAtom->RemoveGamePiecesFromStarField();
	}
}

DaUniverse* DaUniverse::FindParentUniverse()
{
	return this;
}

const DaUniverse* DaUniverse::FindParentUniverse() const
{
	return this;
}

DaAtom::ATOM_TYPE DaUniverse::AtomType() const
{
	return ATOM_Universe;
}

void DaUniverse::ListSceneSubatoms(const DaPerson*, const AuPoint& cameraEnvLoc, DaAtomList&)
{
	Q_UNUSED(cameraEnvLoc);
	// Universe does not add all subordinate galaxies to the scene. This would be
	// the result if the base DaAtom::ListSceneSubatoms was called.
}

void DaUniverse::ListGalaxies(DaAtomList& galaxies) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		if(dynamic_cast<DaGalaxy*>(*it))
			galaxies.AddIfUnique(*it);
}

void DaUniverse::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
}

void DaUniverse::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
}

void DaUniverse::DecaySolo()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(*it);
		if(!galaxy)
			continue;	// wut?
		if(!galaxy->SubatomCount())
			continue;	// No sectors

		// Delete sectors that are not needed.
		AuIdList sectorsToDecay;
		galaxy->ListSectorsReadyToDecay(sectorsToDecay);
		//qDebug() << "DaUniverse::DecaySolo" << *galaxy << "sectorsToDecay" << sectorsToDecay.size() << "of" << galaxy->SubatomCount() << "sectors";
		if(!sectorsToDecay.isEmpty())
		{
			AuIdList deletedSectors;	// We don't care about these.
			galaxy->DecaySectors(sectorsToDecay, deletedSectors);
		}
	}
}

