//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: danpc.cpp,v 1.46 2022/05/17 22:52:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "danpc.h"
//
#include "auatomchat.h"
#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausensorblip.h"
#include "auskill.h"
#include "ausound.h"
#include "austory.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "daworldll.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "daworldsphere.h"
#include "dgdata.h"
#include "dpbody.h"
#include <QSqlError>
#include <QSqlQuery>

#define d_HomeSickDeath					7

DaNpc::DaNpc(NPC_ROLE role, const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name) :
	DaCitizen(dna, chroma, hand, name, Mature)
{
	m_Con = false;
	m_Galaxy = GALAXY_Count;
	m_HomeSickness = 0;
	m_Indestructible = false;
	m_Role = role;
	m_Say = 0;
	m_WorldBodyType = BODY_TYPE_Count;

	m_AccountBalance = 0;
	m_StoryInstance = 0;

	m_WantCID = CID_Invalid;
	m_WantQ = 0;
	m_WantEoO = 0;

	m_AudioSource = 0;
}

DaNpc::DaNpc(NPC_ROLE role, const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name) :
	DaCitizen(species, chroma, hand, name, Mature)
{
	m_Con = false;
	m_Galaxy = GALAXY_Count;
	m_HomeSickness = 0;
	m_Indestructible = false;
	m_Role = role;
	m_Say = 0;
	m_WorldBodyType = BODY_TYPE_Count;

	m_AccountBalance = 0;
	m_StoryInstance = 0;

	m_WantCID = CID_Invalid;
	m_WantQ = 0;
	m_WantEoO = 0;

	m_AudioSource = 0;
}

DaNpc::DaNpc()
{
	m_Con = false;
	m_Galaxy = GALAXY_Count;
	m_HomeSickness = 0;
	m_Indestructible = false;
	m_Role = ROLE_Citizen;
	m_Say = 0;
	m_WorldBodyType = BODY_TYPE_Count;

	m_AccountBalance = 0;
	m_StoryInstance = 0;

	m_WantCID = CID_Invalid;
	m_WantQ = 0;
	m_WantEoO = 0;

	m_AudioSource = 0;
}

DaNpc::~DaNpc()
{
}

DaNpc::DaNpc(const DaNpc &rhs) : DaCitizen(rhs)
{
	m_Con = rhs.m_Con;
	m_Galaxy = rhs.m_Galaxy;
	m_HomeSickness = rhs.m_HomeSickness;
	m_Indestructible = rhs.m_Indestructible;
	m_Role = rhs.m_Role;
	m_Say = rhs.m_Say;
	m_WorldBodyType = rhs.m_WorldBodyType;

	rhs.m_NpcLock.lockForRead();
	m_AccountBalance = rhs.m_AccountBalance;
	m_Companion = rhs.m_Companion;
	m_GarrisonBd = rhs.m_GarrisonBd;
	m_PortBd = rhs.m_PortBd;
	m_SpouseChange = rhs.m_SpouseChange;
	m_SpouseName = rhs.m_SpouseName;
	m_StoryInstance = rhs.m_StoryInstance;
	m_HomeSickTime = rhs.m_HomeSickTime;
	m_SectorId = rhs.m_SectorId;
	m_SectorName = rhs.m_SectorName;
	m_SystemId = rhs.m_SystemId;
	m_SystemName = rhs.m_SystemName;
	m_WorldId = rhs.m_WorldId;
	m_WorldName = rhs.m_WorldName;
	m_WantCID = rhs.m_WantCID;
	m_WantQ = rhs.m_WantQ;
	m_WantEoO = rhs.m_WantEoO;
	m_WantInscription = rhs.m_WantInscription;
	rhs.m_NpcLock.unlock();

	rhs.m_LogLock.lockForRead();
	m_Log = rhs.m_Log;
	m_LogTime = rhs.m_LogTime;
	rhs.m_LogLock.unlock();

	m_AudioSource = 0;
}

void DaNpc::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaCitizen::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(lastTx.IsNull() || m_SpouseChange >= lastTx)
		hasWhat |= 0x11;	// The 0x10 tells us we added m_WorldBodyType
	if(m_Con)
		hasWhat |= 0x02;
	AuId companionId(Companion());
	if(!companionId.IsNull())
		hasWhat |= 0x04;
	if(m_Indestructible)
		hasWhat |= 0x08;

	toClient << hasWhat;
	toClient << m_Role;

	if(hasWhat & 0x01)
	{
		m_NpcLock.lockForRead();
		toClient << m_SectorId;
        toClient << m_SectorName;
        toClient << m_SystemId;
        toClient << m_SystemName;
        toClient << m_WorldBodyType;
        toClient << m_WorldId;
        toClient << m_WorldName;
        toClient << m_Galaxy;
		toClient << m_SpouseName;
		m_NpcLock.unlock();
	}

	if(hasWhat & 0x04)
		toClient << companionId;
}

void DaNpc::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaCitizen::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;
	fromServer >> hasWhat;
	m_Con = hasWhat & 0x02;
	m_Indestructible = hasWhat & 0x08;

	fromServer >> m_Role;

	if(hasWhat & 0x01)
	{
		m_NpcLock.lockForWrite();
        fromServer >> m_SectorId;
        fromServer >> m_SectorName;
        fromServer >> m_SystemId;
        fromServer >> m_SystemName;
		if(hasWhat & 0x10)
			fromServer >> m_WorldBodyType;
        fromServer >> m_WorldId;
        fromServer >> m_WorldName;
        fromServer >> m_Galaxy;
		fromServer >> m_SpouseName;
		m_NpcLock.unlock();
	}

	if(hasWhat & 0x04)
		fromServer >> m_Companion;
	else
		m_Companion = AuId();
}

// Refresh data from valid counterpart.
bool DaNpc::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaCitizen::Refresh(pAtom, ari);
	const DaNpc* pA = (const DaNpc*)pAtom;

	m_Con = pA->m_Con;
	m_Galaxy = pA->m_Galaxy;
	m_Indestructible = pA->m_Indestructible;
	m_Role = pA->m_Role;
	m_WorldBodyType = pA->m_WorldBodyType;

	pA->m_NpcLock.lockForRead();
	m_NpcLock.lockForWrite();
	m_Companion = pA->m_Companion;
	m_SpouseName = pA->m_SpouseName;
	m_SectorId = pA->m_SectorId;
	m_SectorName = pA->m_SectorName;
	m_SystemId = pA->m_SystemId;
	m_SystemName = pA->m_SystemName;
	m_WorldId = pA->m_WorldId;
	m_WorldName = pA->m_WorldName;
	m_NpcLock.unlock();
	pA->m_NpcLock.unlock();
	return false;
}

DaAtom *DaNpc::Clone() const
{
	return new DaNpc(*this);
}

// Insert first time database record.
void DaNpc::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Npc (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
			"timeCreate,timeModified,timeTock,timeDeath,"
			"name,allegiance,arenaTeam,conditions,damageHistory, "
			"hunger,seatIndex,hp,sex,dna4,hair,shave,islefty,pose,whichConsole,stationId,atRest,vecX,vecY,vecZ,stamina, "
			"skills,lineage,ship,berthType,bodyQ,orders,posture,inv, "
			"bank,companion,spouseName,indestructible,interlocutor,role, "
			"con,log,logtime,sectorid,sectorname,systemid,systemname,worldid,worldname,worldBodyType,galaxy,homesick,homesickTime, "
			"portBd, garrisonBd, story,wantCID,wantQ,wantEoO,wantIns) "
			"VALUES (?,?,?,?,?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,?,"
			"?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,"
			"?,?,?,?,?,?,?,?,"
			"?,?,?,?,?,?,"
			"?,?,?,?,?,?,?,?,?,?,?,?,?,"
			"?, ?, ?,?,?,?,?);");
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

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}
	{
		// Unit Damage History
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		const AuUnitDamageHash& ud = LockUnitDamage();
		out << (quint32)ud.size();
		for(AuUnitDamageHash::const_iterator di = ud.begin(); di != ud.end(); di++)
		{
			out << (*di).first;
			out << (*di).second;
		}
		UnlockUnitDamage();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(CurrentBodyHitPoints());
	qa.addBindValue((Chromatid() == AuDNABase::Alpha) ? AuDNABase::Beta : AuDNABase::Alpha);	// reversed for old gender values
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		AuDNABase* dna = DNAnew();
		dna->WriteDynamic(out);
		delete dna;
		qa.addBindValue(ba);
	}
	qa.addBindValue(Haircut());
	qa.addBindValue(Shave());
	qa.addBindValue(IsLeftHanded());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(StationId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);

	// DaCitizen values
	{
		// Skills
		QByteArray ba;
		const AuSkillSet& skills = LockSkillsForRead();
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		out << skills;
		UnlockSkills();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_Lineage);
	qa.addBindValue(m_Ship.GetID());
	qa.addBindValue(m_ShipBerthType);
	qa.addBindValue(m_BodyQ);
	{
		// Orders
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockOrdersForRead() >> out;
		UnlockOrders();
		qa.addBindValue(ba);
	}
	qa.addBindValue(Posture());

	// Gear
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << *LockGearForRead();
		UnlockGear();
		qa.addBindValue(ba);
	}

	// DaNpc
	qa.addBindValue(m_AccountBalance);
	qa.addBindValue(m_Companion.GetID());
	qa.addBindValue(m_SpouseName);
	qa.addBindValue((bool)m_Indestructible);
	qa.addBindValue(m_Interlocutor.GetID());
	qa.addBindValue(m_Role);
	qa.addBindValue(m_Con);
	{
		// Captain's Log
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_LogLock.lockForRead();
		m_Log >> out;
		m_LogLock.unlock();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_LogTime.UTC());
	qa.addBindValue(m_SectorId.GetID());
	qa.addBindValue(m_SectorName);
	qa.addBindValue(m_SystemId.GetID());
	qa.addBindValue(m_SystemName);
	qa.addBindValue(m_WorldId.GetID());
	qa.addBindValue(m_WorldName);
	qa.addBindValue(m_WorldBodyType);
	qa.addBindValue(m_Galaxy);
	qa.addBindValue(m_HomeSickness);
	qa.addBindValue(m_HomeSickTime.UTC());
	qa.addBindValue(m_PortBd.GetID());
	qa.addBindValue(m_GarrisonBd.GetID());
	qa.addBindValue(m_StoryInstance);
	qa.addBindValue(m_WantCID);
    qa.addBindValue(m_WantQ);
    qa.addBindValue(m_WantEoO);
    qa.addBindValue(m_WantInscription);

	if(!qa.exec())
		qWarning() << "warning: INSERT failed: " << *this << qa.lastError();
}

// Update existing database record.
void DaNpc::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Npc SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
			"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, name=?, "
			"allegiance=?, arenaTeam=?, conditions=?, damageHistory=?, "
			"dna4=?, hair=?, shave=?, "
			"hunger=?, seatIndex=?, hp=?, pose=?, whichConsole=?, "
			"stationId=?, atRest=?, vecX=?, vecY=?, vecZ=?, "
			"stamina=?, skills=?, lineage=?, ship=?, berthType=?, bodyQ=?, orders=?, posture=?, inv=?, "
			"bank=?, companion=?, spouseName=?, indestructible=?, interlocutor=?, role=?, con=?, log=?, logtime=?, sectorid=?, "
			"sectorname=?, systemid=?, systemname=?, worldid=?, worldname=?, worldBodyType=?, galaxy=?, homesick=?, homesickTime=?, "
			"portBd=?, garrisonBd=?, story=?, wantCID=?, wantQ=?, wantEoO=?, wantIns=? "
			"WHERE id=%1;").arg(Id().GetID());
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

	// DaUnit values.
	qa.addBindValue(AssignedName());
	qa.addBindValue(Allegiance());
	qa.addBindValue(ArenaTeam());
	{
		// Conditions
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << LockConditions();
		UnlockConditions();
		qa.addBindValue(ba);
	}
	{
		// Unit Damage History
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		const AuUnitDamageHash& ud = LockUnitDamage();
		out << (quint32)ud.size();
		for(AuUnitDamageHash::const_iterator di = ud.begin(); di != ud.end(); di++)
		{
			out << (*di).first;
			out << (*di).second;
		}
		UnlockUnitDamage();
		qa.addBindValue(ba);
	}

	// DaCreature values.
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		AuDNABase* dna = DNAnew();
		dna->WriteDynamic(out);
		delete dna;
		qa.addBindValue(ba);
	}
	qa.addBindValue(Haircut());
	qa.addBindValue(Shave());
	qa.addBindValue(CurrentHunger());
	qa.addBindValue(m_SeatIndex);
	qa.addBindValue(CurrentBodyHitPoints());
	qa.addBindValue(CurrentGeneralPose().Pose());
	qa.addBindValue(ControlId());
	qa.addBindValue(StationId());
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(m_Stamina);	

	// DaCitizen values
	{
		// Skills
		QByteArray ba;
		const AuSkillSet& skills = LockSkillsForRead();
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_5_7);
		out << skills;
		UnlockSkills();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_Lineage);
	qa.addBindValue(m_Ship.GetID());
	qa.addBindValue(m_ShipBerthType);
	qa.addBindValue(m_BodyQ);
	{
		// Orders
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		LockOrdersForRead() >> out;
		UnlockOrders();
		qa.addBindValue(ba);
	}
	qa.addBindValue(Posture());

	// Gear
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << *LockGearForRead();
		UnlockGear();
		qa.addBindValue(ba);
	}

	// DaNpc
	qa.addBindValue(m_AccountBalance);
	qa.addBindValue(m_Companion.GetID());
	qa.addBindValue(m_SpouseName);
	qa.addBindValue((bool)m_Indestructible);
	qa.addBindValue(m_Interlocutor.GetID());
	qa.addBindValue(m_Role);
	qa.addBindValue(m_Con);
	{
		// Captain's Log
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		m_LogLock.lockForRead();
		m_Log >> out;
		m_LogLock.unlock();
		qa.addBindValue(ba);
	}
	qa.addBindValue(m_LogTime.UTC());
	qa.addBindValue(m_SectorId.GetID());
	qa.addBindValue(m_SectorName);
	qa.addBindValue(m_SystemId.GetID());
	qa.addBindValue(m_SystemName);
	qa.addBindValue(m_WorldId.GetID());
	qa.addBindValue(m_WorldName);
	qa.addBindValue(m_WorldBodyType);
	qa.addBindValue(m_Galaxy);
	qa.addBindValue(m_HomeSickness);
	qa.addBindValue(m_HomeSickTime.UTC());
	qa.addBindValue(m_PortBd.GetID());
	qa.addBindValue(m_GarrisonBd.GetID());
	qa.addBindValue(m_StoryInstance);
	qa.addBindValue(m_WantCID);
    qa.addBindValue(m_WantQ);
    qa.addBindValue(m_WantEoO);
    qa.addBindValue(m_WantInscription);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaNpc::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaNpc::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaNpc* DaNpc::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
			"timeModified,timeTock,timeDeath, name,allegiance,arenaTeam,conditions,damageHistory, "
			"hunger,seatIndex,hp,sex,dna4,hair,shave,"
			"islefty,pose,whichConsole,stationId,atRest,vecX,vecY,vecZ,stamina, "
			"skills,lineage,ship,berthType,bodyQ,orders,posture,inv, "
			"bank,companion,spouseName,indestructible,interlocutor,role, "
			"con,log,logtime,sectorid,sectorname,systemid,systemname,worldid,worldname,worldBodyType,galaxy,homesick,homesickTime, "
			"portBd, garrisonBd, story,wantCID,wantQ,wantEoO,wantIns "
			"FROM a_Npc WHERE ");
	if(!idToLoad.IsNull())
		sq += " id=" + QVariant(idToLoad.GetID()).toString();
	else
		sq += " idp=" + QVariant(parentId.GetID()).toString();
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaNpc *pnu = new DaNpc;
		if(pnu)
		{
			int qv = 0;	// query value index
			// DaAtom values.
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

			// DaUnit values.
			pnu->SetName(qa.value(qv++).toString());
			pnu->SetAllegiance(qa.value(qv++).toUInt());
			pnu->SetArenaTeam(qa.value(qv++).toUInt());
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Conditions
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> pnu->LockConditionsForWrite();
				pnu->UnlockConditions();
			}
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Unit Damage History
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				quint32 c;
				AuUnitDamageHash& ud = pnu->LockUnitDamageForWrite();
				in >> c;
				while(c--)
				{
					AuId i;
					AuUnitDamage d;

					in >> i;
					in >> d;

					ud.insert(pair<AuId, AuUnitDamage>(i, d));
				}
				pnu->UnlockUnitDamage();
			}

			// DaCreature values.
			pnu->SetHunger(qa.value(qv++).toInt());
			pnu->m_SeatIndex = qa.value(qv++).toInt();
			pnu->SetHitPoints(qa.value(qv++).toInt());
			pnu->SetChromatid((AuDNABase::CHROMATID)qa.value(qv++).toInt() ? AuDNABase::Alpha : AuDNABase::Beta);	// Was gender
			{
				QByteArray banu = qa.value(qv++).toByteArray();
				QDataStream in(banu);
				in.setVersion(QDataStream::Qt_5_7);
				AuDNABase* dna = AuDNABase::ReadDynamic(in);
				if(dna)
				{
					pnu->SetDNA(*dna);
					delete dna;
				}
			}
			AuHairStyle::STYLE hair = (AuHairStyle::STYLE)qa.value(qv++).toInt();
			AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)qa.value(qv++).toInt();
			pnu->SetHairStyle(hair, shave);
			pnu->SetLeftHanded(qa.value(qv++).toBool());
			AuPose curPose((AuPose::POSE)qa.value(qv++).toInt());
			pnu->m_ControlId = qa.value(qv++).toUInt();
			quint16 stationId = qa.value(qv++).toUInt();
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_Stamina = qa.value(qv++).toInt();

			// DaCitizen values.
			QByteArray ba = qa.value(qv++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);
				in >> pnu->m_Skills;
			}
			pnu->m_Lineage = (LINEAGE_TYPE)qa.value(qv++).toUInt();
			pnu->m_Ship = AuId(qa.value(qv++).toUInt());
			pnu->m_ShipBerthType = (AuRosterEntry::TYPE)(qa.value(qv++).toInt());
			pnu->m_BodyQ = qa.value(qv++).toUInt();
			if(pnu->m_BodyQ == 0)
				pnu->m_BodyQ = 128;
			qvi = qv++;
			AuAIOrderArray& orders = pnu->LockOrdersForWrite();
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				orders << in;
			}
			if(orders.isEmpty())
			{
				// Index 0 is always my fallback Stay order.
				orders.append(AuAIOrder(ORDER_Stay, parentId, pnu->Location(), 0, ""));
				pnu->UnlockOrders();
			}
			else
			{
				pnu->UnlockOrders();
				pnu->ResetOrderTime();
			}
			pnu->SetPosture((CITIZEN_POSTURE)qa.value(qv++).toInt());

			// Gear
			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> *pnu->LockGearForWrite();
				pnu->UnlockGear();
			}

			// DaNpc
			pnu->m_AccountBalance = qa.value(qv++).toDouble();
			pnu->m_Companion = AuId(qa.value(qv++).toUInt());
			pnu->m_SpouseName = qa.value(qv++).toString();
			pnu->m_Indestructible = qa.value(qv++).toBool();
			pnu->m_Interlocutor = AuId(qa.value(qv++).toUInt());
			pnu->m_Role = (NPC_ROLE)qa.value(qv++).toUInt();
			pnu->m_Con = qa.value(qv++).toBool();

			qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Captain's Log
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				pnu->m_Log << in;
			}

			QDateTime logt = qa.value(qv++).toDateTime();
			logt.setTimeSpec(Qt::UTC);
			pnu->m_LogTime = logt;

			pnu->m_SectorId = AuId(qa.value(qv++).toUInt());
			pnu->m_SectorName = qa.value(qv++).toString();
			pnu->m_SystemId = AuId(qa.value(qv++).toUInt());
			pnu->m_SystemName = qa.value(qv++).toString();
			pnu->m_WorldId = AuId(qa.value(qv++).toUInt());
			pnu->m_WorldName = qa.value(qv++).toString();
			pnu->m_WorldBodyType = qa.value(qv++).toUInt();
			pnu->m_Galaxy = qa.value(qv++).toUInt();
			pnu->m_HomeSickness = qa.value(qv++).toUInt();
			QDateTime tm = qa.value(qv++).toDateTime();
			if(tm.isValid())
			{
				tm.setTimeSpec(Qt::UTC);
				pnu->m_HomeSickTime = tm;
			}

			pnu->m_PortBd = AuId(qa.value(qv++).toUInt());
			pnu->m_GarrisonBd = AuId(qa.value(qv++).toUInt());

			pnu->m_StoryInstance = qa.value(qv++).toUInt();
			pnu->m_WantCID = (COMMODITYID)qa.value(qv++).toInt();
			pnu->m_WantQ = qa.value(qv++).toUInt();
			pnu->m_WantEoO = qa.value(qv++).toUInt();
			pnu->m_WantInscription = qa.value(qv++).toString();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->RestorePose(curPose, stationId);				// Restore console after adding to my parent atom.
			pnu->StartThinking();
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaAnimal::LoadSubatoms(ether, db, pnu->Id());
			DaNpc::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

// The name used when addressed by a superior.
QString DaNpc::CrewName() const
{
	return tr("%1 %2").arg(RankCrew()).arg(AssignedName());
}

QString DaNpc::Name() const
{
	QString rankAbbr(RankAbbr());
	if(rankAbbr.isEmpty())
		return AssignedName();
	return tr("%1 %2").arg(rankAbbr).arg(AssignedName());
}

void DaNpc::SensorScan(AuSensorBlipList& blips, const AuPoint& /*scannerLoc*/, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int scanContent, const AuId& /*lockedBlip*/,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	// I am only scanned for AI turret gunners and transporter consoles.
	if(!(scanContent & 0x0024))
		return;

	// Only my own ship, that I hold a berth aboard, can detect me on transporter sensors.
	if(AssignedShip().IsNull() || AssignedShip() != scannerId)
		return;

	// Dying creatures do not show up on sensors.
	if(IsBodyDyingOrDead())
		return;

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	// I hold a berth aboard the scanning ship. Return a blip that corresponds to my berth type.
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		blips.Add(AuBlipPassenger(Name(), Species(), EmpireId(), PoliticalStance(scannerEmpire, EmpireId(), stanceCache), Id(),
					EnvLocation()));
		break;
	case ROLE_Crew:
		blips.Add(AuBlipCrew(Name(), Species(), EmpireId(), PoliticalStance(scannerEmpire, EmpireId(), stanceCache), Id(),
					EnvLocation()));
		break;
	case ROLE_Troop:
		blips.Add(AuBlipTroop(Name(), Species(), EmpireId(), PoliticalStance(scannerEmpire, EmpireId(), stanceCache), Id(),
					EnvLocation()));
		break;
	case ROLE_Officer:
		blips.Add(AuBlipOfficer(Name(), Species(), EmpireId(), PoliticalStance(scannerEmpire, EmpireId(), stanceCache), Id(),
					EnvLocation()));
		break;
	}
}

unsigned int DaNpc::IsTocker() const
{
	unsigned int ms = DaCitizen::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is not zero.

	if(IsBodyDyingOrDead())
		return ms;
	if(ms > d_MSecPerMin)
		ms = d_MSecPerMin;
	if(ms > d_MSecPerSec)
		if((!IsAirBreathable() && !IsMouthSubmerged()) || IsInVisibleWater())
			ms = d_MSecPerSec;
	return ms;
}

void DaNpc::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	//qDebug("DaNpc::Tock");
	DaCitizen::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;	// This atom (not body) is dead or awaiting transfer.
	if(IsBodyDyingOrDead())
		return;

	// Check for officer homesickness.
	// Home world id 1 is set when an agent spawns an officer.
	if(IsNpcOfficer() && !last.IsNull() && last.ElapsedDayTicks(now) > 0 && !HomeWorldId().IsNull() && HomeWorldId() != AuId(1))
	{
		// Day changed.
		if(m_HomeSickTime.ElapsedHourTicks() > 1 || m_HomeSickTime.IsNull())
		{
			// Date is later than the last time we did this by more than an hour. We have to check this because officers warping at
			// high speed during the day change can hit this code a lot, with slightly different server times.
			const DgData& gd = g_Au.LockDataForRead();
			bool homeHasEmpireCity = gd.EmpireHasCityOnWorld(EmpireId(), HomeWorldId());
			g_Au.UnlockData();
			if(homeHasEmpireCity)
			{
				if(m_HomeSickness)
				{
					m_HomeSickness = 0;	// Home sickness goes away when there is a city.
					m_HomeSickTime = AuTime::Now();
					SetModified(NOTIFY_Database);
				}
			}
			else
			{
				m_HomeSickness++;		// Home sickness increases each day.
				m_HomeSickTime = AuTime::Now();
				SetModified(NOTIFY_Database);
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else if(m_StoryInstance)
	{
		if(g_Au.IsStoryActive(m_StoryInstance))
		{
			// My story is active.
			if(StoryScript() == 8)
			{
				if(Name() == "Dr. Millford")
				{
					// Dr. Millford gets pulled through a portal to the ringworld in this story segment. We need to remove him when
					// that happens.
					if(!IsInAtmosphere())
					{
#if !defined(QT_NO_DEBUG)
						qDebug() << "D DaNpc::Tock" << Id() << Name() << "not in atmosphere, killed";
#endif
						Kill();
					}
					else
					{
						const DaWorldSphere* globe = dynamic_cast<const DaWorldSphere*>(FindParentPlanetInEnv());
						if(!globe)
						{
#if !defined(QT_NO_DEBUG)
							qDebug() << "D DaNpc::Tock" << Id() << Name() << "not on globe, killed";
#endif
							Kill();
						}
					}
				}
				else if(Name() == "Shaman Lotar")
				{
					// Keep Shaman Lotar from running off when he gets to the ringworld.
					const DaWorldRingSection* ringw = dynamic_cast<const DaWorldRingSection*>(FindParentPlanetInEnv());
					if(ringw)
						SetOrder(ORDER_Stay, AuId(0), AuPoint());
				}
			}
		}
		else
		{
			// Stop following people and doing other things when my story is not active.
			if(HasOrder() && !IsNapping())
				SetOrder(ORDER_Stay, AuId(0), AuPoint());

			// When not in any scene and my story is no longer active, I can be removed.
			if(!IsInAnyScene())
			{
				Kill();
				return;
			}

			if(last.ElapsedSecTicks(now) > 0)
			{
				// Turn off indestructibility when character is no longer in a story.
				if(IsIndestructible())
					SetIndestructible(false);
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(g_Au.m_EnableTroopResupply && IsAtBd() && last.IsNewPeriod(now, d_MSecPerHr/3, Id().GetID()))
	{
		UpgradeEquipmentFromBd();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.IsNewPeriod(now, d_MSecPerMin, Id().GetID()))
	{
		MinuteTock();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.ElapsedSecTicks(now) > 0)
	{
		// Take damage from the atmosphere if it is not breathable and I am not wearing an environment suit.
		if(!IsMouthSubmerged())
		{
			if(IsAirBreathable() || IsInBerth() || IsPatientStationPose() || IsProtectedByEnvironmentSuit())
				Inhale();
			else if(!HoldBreath())
				ApplyAtmosphericDamage();					// I am not wearing an environment suit or my suit leaks.
		}

		// Take damage from the surface liquid if it is not water.
		// "Water" in the inferno/frigid zone is not water.
		// Environment suit does not protect me from this.
		if(IsInVisibleWater())
			ApplyLiquidDamage();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

void DaNpc::TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
		bool firstInGroup, const QString& orderText)
{
	if(StoryInstance())
	{
		DaCitizen::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);
		return;
	}

	QString myresponse;
	AuVisage::VISAGE myvisage = AuVisage::Closed;
	QString channelName = msg.ChannelName();
	AuCommStdChannel::SCOPE scope = msg.Scope();

	// Garrison order requires some checking to see if it can be done.
	bool acceptOrder = true;
	if(order == ORDER_Garrison)
	{
		if(!IsInsideSpacecraft())
			return;
		const DaVessel* ship = FindParentVesselInEnv();
		if(!ship)
			return;

		if(ship->PoliticalStanceMakeContact(this) == POL_Enemy)
		{
			if(!firstInGroup)
				return;	// Only the first in the group responds to the order.
			acceptOrder = false;
			if(msg.RecipientCount() > 1)
				myresponse = tr("Sorry sir, this ship flies the flag of an enemy empire. We can't take a berth.");
			else
				myresponse = tr("Sorry sir, this ship flies the flag of an enemy empire. I can't take a berth.");
		}
		else
		{
			// Try taking a berth aboard the ship.
			bool berthAvailable = false;
			if(CommandExperiencePoints())
			{
				// Try to take an officer berth.
				if(ship->EmpireId() == EmpireId() && ship->OfficerBerthsAvailable())
					berthAvailable = true;
			}

			if(!berthAvailable && CrewExperiencePoints())
			{
				if(ship->CanAcceptBerthType(AuRosterEntry::Crew) && ship->CrewBerthsAvailable())
					berthAvailable = true;
			}

			if(!berthAvailable && TroopExperiencePoints())
			{
				if(ship->EmpireId() == EmpireId() && ship->CanAcceptBerthType(AuRosterEntry::Troop) && ship->TroopBerthsAvailable())
					berthAvailable = true;
			}

			if(!berthAvailable)
			{
				if(ship->CanAcceptBerthType(AuRosterEntry::PassengerAI) && ship->PassengerBerthsAvailable())
					berthAvailable = true;
			}

			if(!berthAvailable)
			{
				if(!firstInGroup)
					return;	// Only the first in the group responds to the order.
				acceptOrder = false;
				myresponse = tr("Sorry sir, no berths are available aboard this ship.");
			}
		}
	}

	if(acceptOrder)
	{
		switch(Role())
		{
		case ROLE_Citizen:
		case ROLE_IndigenousPerson:
		case ROLE_IndigenousElder:
		case ROLE_IndigenousHealer:
		case ROLE_IndigenousStargazer:
		case ROLE_Worker:
			// Only the first in the group responds to the order.
			if(firstInGroup)
			{
				// Send a response to the person who gave me the order.
				QString responses[] =
				{
					"I'm not staying here.",	// ORDER_Stay
					"I'm not attacking anything.",	// ORDER_AttackTarget
					"I'm not moving to where you are.",	// ORDER_ComeHere
					"I'm not moving right to where you are.",	// ORDER_ComeHereExactly
					"I'm not following you.",	// ORDER_FollowMe
					"I'm not following anything.",	// ORDER_FollowTarget
					"I'm not getting in with you.",	// ORDER_GetIn
					"I'm not getting out.",	// ORDER_GetOut
					"I'm not moving there.",	// ORDER_GoToTarget
					"I'm not moving right there.",	// ORDER_GoToTargetExactly
					"I'm not guarding this place.",	// ORDER_Garrison
					"I'm not closing the door.",	// ORDER_CloseDoor
					"I'm not opening the door.",	// ORDER_OpenDoor
					"I'm not getting that.",	// ORDER_GetItem
					"I'm not manning any station.",	// ORDER_BattleStation
					"I'm not sleeping here.",	// ORDER_Sleep
					"I'm not daydreaming.",	// ORDER_SleepDayDream
					"I'm not eating.",	// ORDER_SleepEat
					"I'm not exercising.",	// ORDER_SleepExercise
					QString("I'm not playing %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"I'm not writing a letter.",	// ORDER_SleepLetter
					"I'm not watching a movie.",	// ORDER_SleepMovie
					"I'm not listening to music.",	// ORDER_SleepMusic
					"I'm not pining for my love.",	// ORDER_SleepPine
					"I'm not talking on the phone.",	// ORDER_SleepPhone
					"I'm not reading.",	// ORDER_SleepRead
					"I'm not talking.",	// ORDER_SleepTalk
					"I'm not texting.",	// ORDER_SleepText
					"I'm not watching TV.",	// ORDER_SleepTV
					"I'm not surfing the web.",	// ORDER_SleepWeb
					"I'm not sending a damage report.",	// ORDER_DamageReport
					"I'm not scanning the planet.",	// ORDER_SurveyScanPlanet
					"I'm not scanning the system.",	// ORDER_SurveyScanSystem
					"I'm not tactical scanning.",	// ORDER_TacticalScan
					"I'm not getting any medical procedures.",	// ORDER_EnterSurgery
					"I'm not taking anything.",	// ORDER_TakeCommand
					"I'm not releasing anything.",	// ORDER_ReleaseCommand
					"I'm not dropping my rider.",							// ORDER_DropRider
					"I'm not mounting you.",								// ORDER_MountMe
					"I'm not mounting that target.",						// ORDER_MountTarget
					"I'm not dancing.",						// ORDER_HengeCeremony
					"I'm not moving to %1.",	// ORDER_GoToRoom
				};

				myresponse = responses[order];
				myvisage = AuVisage::Frown;
			}
			break;
		case ROLE_Passenger:
			// Base class quietly accepts the order.
			// Passengers can't be ordered to attack.
			if(order == ORDER_AttackTarget || order == ORDER_BattleStation)
				DaCitizen::TakeOrder(msg, ORDER_Stay, orderTarget, orderLoc, firstInGroup, orderText);
			else
				DaCitizen::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);

			// Only the first in the group responds to the order.
			if(firstInGroup)
			{
				QString responses[] =
				{
					"I'll stay here.",	// ORDER_Stay
					"I'll attack that.",	// ORDER_AttackTarget
					"I'll move to where you are.",	// ORDER_ComeHere
					"I'll move precisely to where you are.",	// ORDER_ComeHereExactly
					"I'll follow you.",	// ORDER_FollowMe
					"I'll follow that.",	// ORDER_FollowTarget
					"I'll get in with you.",	// ORDER_GetIn
					"I'll get out.",	// ORDER_GetOut
					"I'll move there.",	// ORDER_GoToTarget
					"I'll move precisely there.",	// ORDER_GoToTargetExactly
					"I'll join the local population.",	// ORDER_Garrison
					"I'll close the door.",	// ORDER_CloseDoor
					"I'll open the door.",	// ORDER_OpenDoor
					"I'll get that.",	// ORDER_GetItem
					"I'll man a station.",	// ORDER_BattleStation
					"I'll sleep here.",	// ORDER_Sleep
					"I'll daydream.",	// ORDER_SleepDayDream
					"I'll eat.",	// ORDER_SleepEat
					"I'll exercise.",	// ORDER_SleepExercise
					QString("I'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"I'll write a letter.",	// ORDER_SleepLetter
					"I'll watch a movie.",	// ORDER_SleepMovie
					"I'll listen to music.",	// ORDER_SleepMusic
					"I'll pine for my love.",	// ORDER_SleepPine
					"I'll talk on the phone.",	// ORDER_SleepPhone
					"I'll read.",	// ORDER_SleepRead
					"I'll talk.",	// ORDER_SleepTalk
					"I'll text.",	// ORDER_SleepText
					"I'll watch TV.",	// ORDER_SleepTV
					"I'll surf the web.",	// ORDER_SleepWeb
					"I'll send a damage report.",	// ORDER_DamageReport
					"I'll survey scan the planet.",	// ORDER_SurveyScanPlanet
					"I'll survey scan the system.",	// ORDER_SurveyScanSystem
					"I'll tactical scan.",	// ORDER_TacticalScan
					"I'll get a checkup in the sick bay.",	// ORDER_EnterSurgery
					"I don't have the con. It was that other dude who looks just like me.",	// ORDER_TakeCommand
					"I certainly hope you don't think I had it.",	// ORDER_ReleaseCommand
					"I'll drop my rider.",									// ORDER_DropRider
					"I'll mount you.",										// ORDER_MountMe
					"I'll mount that target.",								// ORDER_MountTarget
					"I'll do the curly shuffle.",								// ORDER_HengeCeremony
					"I'll move to %1, sounds fun.",	// ORDER_GoToRoom
				};

				if(Order() == ORDER_Stay)
				{
					if(order == ORDER_AttackTarget)
						myresponse = tr("I'm not attacking anything.");
					else if(order == ORDER_BattleStation)
						myresponse = tr("I'm not manning any station.");
					else
						myresponse = responses[Order()];
				}
				else
					myresponse = responses[Order()];
			}
			break;
		case ROLE_Crew:
			// Base class quietly accepts the order.
			DaCitizen::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);
			if(firstInGroup)
				break;	// Only the first in the group responds to the order.

			if(msg.RecipientCount() > 1)
			{
				QString responses[] =
				{
					"Aye sir. We'll stay here.",	// ORDER_Stay
					"Aye sir. We'll attack that target.",	// ORDER_AttackTarget
					"Aye sir. We'll move to your location.",	// ORDER_ComeHere
					"Aye sir. We'll move to your exact location.",	// ORDER_ComeHereExactly
					"Aye sir. We'll follow you.",	// ORDER_FollowMe
					"Aye sir. We'll follow that target.",	// ORDER_FollowTarget
					"Aye sir. We'll get in with you.",	// ORDER_GetIn
					"Aye sir. We'll get out.",	// ORDER_GetOut
					"Aye sir. We'll move to that location.",	// ORDER_GoToTarget
					"Aye sir. We'll move exactly to that location.",	// ORDER_GoToTargetExactly
					"Aye sir. We'll fortify our positions.",	// ORDER_Garrison
					"Aye sir. We'll close the doors.",	// ORDER_CloseDoor
					"Aye sir. We'll open the doors.",	// ORDER_OpenDoor
					"Aye sir. We'll get that.",	// ORDER_GetItem
					"Aye sir. We'll man stations.",	// ORDER_BattleStation
					"Aye sir. We'll sleep here.",	// ORDER_Sleep
					"Aye sir. We'll daydream.",	// ORDER_SleepDayDream
					"Aye sir. We'll eat.",	// ORDER_SleepEat
					"Aye sir. We'll exercise.",	// ORDER_SleepExercise
					QString("Aye sir. We'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Aye sir. We'll write a letter.",	// ORDER_SleepLetter
					"Aye sir. We'll watch a movie.",	// ORDER_SleepMovie
					"Aye sir. We'll listen to music.",	// ORDER_SleepMusic
					"Aye sir. We'll pine for our loves.",	// ORDER_SleepPine
					"Aye sir. We'll talk on the phone.",	// ORDER_SleepPhone
					"Aye sir. We'll read.",	// ORDER_SleepRead
					"Aye sir. We'll talk.",	// ORDER_SleepTalk
					"Aye sir. We'll text.",	// ORDER_SleepText
					"Aye sir. We'll watch TV.",	// ORDER_SleepTV
					"Aye sir. We'll surf the web.",	// ORDER_SleepWeb
					"Aye sir. We'll send a damage report.",	// ORDER_DamageReport
					"Aye sir. We'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
					"Aye sir. We'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
					"Aye sir. We'll conduct a tactical scan.",	// ORDER_TacticalScan
					"Aye sir. We'll report to sick bay for a checkup.",	// ORDER_EnterSurgery
					"Sir, we're not officer material.",	// ORDER_TakeCommand
					"Sir, we're not in command. Aren't you in command?",	// ORDER_ReleaseCommand
					"Aye sir. We'll drop our riders.",				// ORDER_DropRider
					"Aye sir. We'll mount you.",					// ORDER_MountMe
					"Aye sir. We'll mount that target.",			// ORDER_MountTarget
					"Aye sir. We'll dance.",			// ORDER_HengeCeremony
					"Aye sir. We'll move to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			else
			{
				QString responses[] =
				{
					"Aye sir. I'll stay here.",	// ORDER_Stay
					"Aye sir. I'll attack that target.",	// ORDER_AttackTarget
					"Aye sir. I'll move to your location.",	// ORDER_ComeHere
					"Aye sir. I'll move to your exact location.",	// ORDER_ComeHereExactly
					"Aye sir. I'll follow you.",	// ORDER_FollowMe
					"Aye sir. I'll follow that target.",	// ORDER_FollowTarget
					"Aye sir. I'll get in with you.",	// ORDER_GetIn
					"Aye sir. I'll get out.",	// ORDER_GetOut
					"Aye sir. I'll move to that location.",	// ORDER_GoToTarget
					"Aye sir. I'll move exactly to that location.",	// ORDER_GoToTargetExactly
					"Aye sir. I'll fortify my position.",	// ORDER_Garrison
					"Aye sir. I'll close the door.",	// ORDER_CloseDoor
					"Aye sir. I'll open the door.",	// ORDER_OpenDoor
					"Aye sir. I'll get that.",	// ORDER_GetItem
					"Aye sir. I'll man a station.",	// ORDER_BattleStation
					"Aye sir. I'll sleep here.",	// ORDER_Sleep
					"Aye sir. I'll daydream.",	// ORDER_SleepDayDream
					"Aye sir. I'll eat.",	// ORDER_SleepEat
					"Aye sir. I'll exercise.",	// ORDER_SleepExercise
					QString("Aye sir. I'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Aye sir. I'll write a letter.",	// ORDER_SleepLetter
					"Aye sir. I'll watch a movie.",	// ORDER_SleepMovie
					"Aye sir. I'll listen to music.",	// ORDER_SleepMusic
					"Aye sir. I'll pine for my love.",	// ORDER_SleepPine
					"Aye sir. I'll talk on the phone.",	// ORDER_SleepPhone
					"Aye sir. I'll read.",	// ORDER_SleepRead
					"Aye sir. I'll talk.",	// ORDER_SleepTalk
					"Aye sir. I'll text.",	// ORDER_SleepText
					"Aye sir. I'll watch TV.",	// ORDER_SleepTV
					"Aye sir. I'll surf the web.",	// ORDER_SleepWeb
					"Aye sir. I'll send a damage report.",	// ORDER_DamageReport
					"Aye sir. I'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
					"Aye sir. I'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
					"Aye sir. I'll conduct a tactical scan.",	// ORDER_TacticalScan
					"Aye sir. I'll report to sick bay for a checkup.",	// ORDER_EnterSurgery
					"Sir, I'm not officer grade.",	// ORDER_TakeCommand
					"Sir, I'm not in command. Is anyone in command?",	// ORDER_ReleaseCommand
					"Aye sir. I'll drop my rider.",					// ORDER_DropRider
					"Aye sir. I'll mount you.",						// ORDER_MountMe
					"Aye sir. I'll mount that target.",				// ORDER_MountTarget
					"Aye sir. I'll dance.",						// ORDER_HengeCeremony
					"Aye sir. I'll move to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			break;
		case ROLE_Troop:
			// Base class quietly accepts the order.
			DaCitizen::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);
			if(firstInGroup)
				break;	// Only the first in the group responds to the order.

			if(msg.RecipientCount() > 1)
			{
				QString responses[] =
				{
					"Yes sir. We're staying here.",	// ORDER_Stay
					"Yes sir. We're attacking that target.",	// ORDER_AttackTarget
					"Yes sir. We're moving to your location.",	// ORDER_ComeHere
					"Yes sir. We're moving to your exact location.",	// ORDER_ComeHereExactly
					"Yes sir. We're following you.",	// ORDER_FollowMe
					"Yes sir. We're following that target.",	// ORDER_FollowTarget
					"Yes sir. We're getting in with you.",	// ORDER_GetIn
					"Yes sir. We're getting out.",	// ORDER_GetOut
					"Yes sir. We're moving to that location.",	// ORDER_GoToTarget
					"Yes sir. We're moving exactly to that location.",	// ORDER_GoToTargetExactly
					"Yes sir. We're fortifying our positions.",	// ORDER_Garrison
					"Yes sir. We're closing the doors.",	// ORDER_CloseDoor
					"Yes sir. We're opening the doors.",	// ORDER_OpenDoor
					"Yes sir. We're getting that.",	// ORDER_GetItem
					"Yes sir. We're manning the guns.",	// ORDER_BattleStation
					"Yes sir. We're sleeping here.",	// ORDER_Sleep
					"Yes sir. We're daydreaming.",	// ORDER_SleepDayDream
					"Yes sir. We're eating.",	// ORDER_SleepEat
					"Yes sir. We're exercising.",	// ORDER_SleepExercise
					QString("Yes sir. We're playing %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Yes sir. We're writing a letter.",	// ORDER_SleepLetter
					"Yes sir. We're watching a movie.",	// ORDER_SleepMovie
					"Yes sir. We're listening to music.",	// ORDER_SleepMusic
					"Yes sir. We're pining for our loves.",	// ORDER_SleepPine
					"Yes sir. We're talking on the phone.",	// ORDER_SleepPhone
					"Yes sir. We're reading.",	// ORDER_SleepRead
					"Yes sir. We're talking.",	// ORDER_SleepTalk
					"Yes sir. We're texting.",	// ORDER_SleepText
					"Yes sir. We're watching TV.",	// ORDER_SleepTV
					"Yes sir. We're surfing the web.",	// ORDER_SleepWeb
					"Yes sir. We're sending a damage report.",	// ORDER_DamageReport
					"Yes sir. We're survey scanning the planet.",	// ORDER_SurveyScanPlanet
					"Yes sir. We're survey scanning the system.",	// ORDER_SurveyScanSystem
					"Yes sir. We're tactical scanning.",	// ORDER_TacticalScan
					"Yes sir. We're reporting to the medic for a checkup.",	// ORDER_EnterSurgery
					"Sir, we're just GIs. Maybe you'd rather we kill something for you.",	// ORDER_TakeCommand
					"Sir, we're not in charge.",	// ORDER_ReleaseCommand
					"Yes sir. We'll drop our riders.",				// ORDER_DropRider
					"Yes sir. We'll mount you.",					// ORDER_MountMe
					"Yes sir. We'll mount that target.",			// ORDER_MountTarget
					"Yes sir. We'll dance.",			// ORDER_HengeCeremony
					"Yes sir. We're moving to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			else
			{
				QString responses[] =
				{
					"Yes sir. I'm staying here.",	// ORDER_Stay
					"Yes sir. I'm attacking that target.",	// ORDER_AttackTarget
					"Yes sir. I'm moving to your location.",	// ORDER_ComeHere
					"Yes sir. I'm moving to your exact location.",	// ORDER_ComeHereExactly
					"Yes sir. I'm following you.",	// ORDER_FollowMe
					"Yes sir. I'm following that target.",	// ORDER_FollowTarget
					"Yes sir. I'm getting in with you.",	// ORDER_GetIn
					"Yes sir. I'm getting out.",	// ORDER_GetOut
					"Yes sir. I'm moving to that location.",	// ORDER_GoToTarget
					"Yes sir. I'm moving exactly to that location.",	// ORDER_GoToTargetExactly
					"Yes sir. I'm fortifying my position.",	// ORDER_Garrison
					"Yes sir. I'm closing the door.",	// ORDER_CloseDoor
					"Yes sir. I'm opening the door.",	// ORDER_OpenDoor
					"Yes sir. I'm getting that.",	// ORDER_GetItem
					"Yes sir. I'm manning the guns.",	// ORDER_BattleStation
					"Yes sir. I'm sleeping here.",	// ORDER_Sleep
					"Yes sir. I'm daydreaming.",	// ORDER_SleepDayDream
					"Yes sir. I'm eating.",	// ORDER_SleepEat
					"Yes sir. I'm exercising.",	// ORDER_SleepExercise
					QString("Yes sir. I'm playing %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Yes sir. I'm writing a letter.",	// ORDER_SleepLetter
					"Yes sir. I'm watching a movie.",	// ORDER_SleepMovie
					"Yes sir. I'm listening to music.",	// ORDER_SleepMusic
					"Yes sir. I'm pining for my love.",	// ORDER_SleepPine
					"Yes sir. I'm talking on the phone.",	// ORDER_SleepPhone
					"Yes sir. I'm reading.",	// ORDER_SleepRead
					"Yes sir. I'm talking.",	// ORDER_SleepTalk
					"Yes sir. I'm texting.",	// ORDER_SleepText
					"Yes sir. I'm watching TV.",	// ORDER_SleepTV
					"Yes sir. I'm surfing the web.",	// ORDER_SleepWeb
					"Yes sir. I'm sending a damage report.",	// ORDER_DamageReport
					"Yes sir. I'm survey scanning the planet.",	// ORDER_SurveyScanPlanet
					"Yes sir. I'm survey scanning the system.",	// ORDER_SurveyScanSystem
					"Yes sir. I'm tactical scanning.",	// ORDER_TacticalScan
					"Yes sir. I'm reporting to the medic for a checkup.",	// ORDER_EnterSurgery
					"Sir, I failed the Navy entrance exam.",	// ORDER_TakeCommand
					"Sir, I'm not in charge.",	// ORDER_ReleaseCommand
					"Yes sir. I'll drop my rider.",					// ORDER_DropRider
					"Yes sir. I'll mount you.",						// ORDER_MountMe
					"Yes sir. I'll mount that target.",				// ORDER_MountTarget
					"Yes sir. I'll dance.",			// ORDER_HengeCeremony
					"Yes sir. I'll move to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			break;
		case ROLE_Officer:
			// Quietly accept the order.
			if(order == ORDER_TakeCommand || order == ORDER_ReleaseCommand)
			{
				// Taking and releasing the con is done as an amendment so it does not change what I am already doing.
				AmendOrder(order, orderTarget, orderLoc, 0);
				SetModified();
			}
			else
				DaCitizen::TakeOrder(msg, order, orderTarget, orderLoc, firstInGroup, orderText);
			if(firstInGroup)
				break;	// Only the first in the group responds to the order.

			if(msg.RecipientCount() > 1)
			{
				QString responses[] =
				{
					"Aye sir. We'll stay here.",	// ORDER_Stay
					"Aye sir. We'll attack that target.",	// ORDER_AttackTarget
					"Aye sir. We'll move to your location.",	// ORDER_ComeHere
					"Aye sir. We'll move to your exact location.",	// ORDER_ComeHereExactly
					"Aye sir. We'll follow you.",	// ORDER_FollowMe
					"Aye sir. We'll follow that target.",	// ORDER_FollowTarget
					"Aye sir. We'll get in with you.",	// ORDER_GetIn
					"Aye sir. We'll get out.",	// ORDER_GetOut
					"Aye sir. We'll move to that location.",	// ORDER_GoToTarget
					"Aye sir. We'll move exactly to that location.",	// ORDER_GoToTargetExactly
					"Aye sir. We'll fortify our positions.",	// ORDER_Garrison
					"Aye sir. We'll close the doors.",	// ORDER_CloseDoor
					"Aye sir. We'll open the doors.",	// ORDER_OpenDoor
					"Aye sir. We'll get that.",	// ORDER_GetItem
					"Aye sir. We'll man stations.",	// ORDER_BattleStation
					"Aye sir. We'll sleep here.",	// ORDER_Sleep
					"Aye sir. We'll daydream.",	// ORDER_SleepDayDream
					"Aye sir. We'll eat.",	// ORDER_SleepEat
					"Aye sir. We'll exercise.",	// ORDER_SleepExercise
					QString("Aye sir. We'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Aye sir. We'll write a letter.",	// ORDER_SleepLetter
					"Aye sir. We'll watch a movie.",	// ORDER_SleepMovie
					"Aye sir. We'll listen to music.",	// ORDER_SleepMusic
					"Aye sir. We'll pine for our loves.",	// ORDER_SleepPine
					"Aye sir. We'll talk on the phone.",	// ORDER_SleepPhone
					"Aye sir. We'll read.",	// ORDER_SleepRead
					"Aye sir. We'll talk.",	// ORDER_SleepTalk
					"Aye sir. We'll text.",	// ORDER_SleepText
					"Aye sir. We'll watch TV.",	// ORDER_SleepTV
					"Aye sir. We'll surf the web.",	// ORDER_SleepWeb
					"Aye sir. We'll send a damage report.",	// ORDER_DamageReport
					"Aye sir. We'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
					"Aye sir. We'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
					"Aye sir. We'll conduct a tactical scan.",	// ORDER_TacticalScan
					"Aye sir. We'll report to the doc for a checkup.",	// ORDER_EnterSurgery
					"Aye sir. We have the con.",	// ORDER_TakeCommand
					"Aye sir. You have the con.",	// ORDER_ReleaseCommand
					"Aye sir. We'll drop our riders.",				// ORDER_DropRider
					"Aye sir. We'll mount you.",					// ORDER_MountMe
					"Aye sir. We'll mount that target.",			// ORDER_MountTarget
					"Aye sir. We'll dance.",					// ORDER_HengeCeremony
					"Aye sir. We'll move to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			else
			{
				QString responses[] =
				{
					"Aye sir. I'll stay here.",	// ORDER_Stay
					"Aye sir. I'll attack that target.",	// ORDER_AttackTarget
					"Aye sir. I'll move to your location.",	// ORDER_ComeHere
					"Aye sir. I'll move to your exact location.",	// ORDER_ComeHereExactly
					"Aye sir. I'll follow you.",	// ORDER_FollowMe
					"Aye sir. I'll follow that target.",	// ORDER_FollowTarget
					"Aye sir. I'll get in with you.",	// ORDER_GetIn
					"Aye sir. I'll get out.",	// ORDER_GetOut
					"Aye sir. I'll move to that location.",	// ORDER_GoToTarget
					"Aye sir. I'll move exactly to that location.",	// ORDER_GoToTargetExactly
					"Aye sir. I'll fortify my position.",	// ORDER_Garrison
					"Aye sir. I'll close the door.",	// ORDER_CloseDoor
					"Aye sir. I'll open the door.",	// ORDER_OpenDoor
					"Aye sir. I'll get that.",	// ORDER_GetItem
					"Aye sir. I'll man a station.",	// ORDER_BattleStation
					"Aye sir. I'll sleep here.",	// ORDER_Sleep
					"Aye sir. I'll daydream.",	// ORDER_SleepDayDream
					"Aye sir. I'll eat.",	// ORDER_SleepEat
					"Aye sir. I'll exercise.",	// ORDER_SleepExercise
					QString("Aye sir. I'll play %1.").arg(g_Au.GameName()),	// ORDER_SleepGame
					"Aye sir. I'll write a letter.",	// ORDER_SleepLetter
					"Aye sir. I'll watch a movie.",	// ORDER_SleepMovie
					"Aye sir. I'll listen to music.",	// ORDER_SleepMusic
					"Aye sir. I'll pine for my love.",	// ORDER_SleepPine
					"Aye sir. I'll talk on the phone.",	// ORDER_SleepPhone
					"Aye sir. I'll read.",	// ORDER_SleepRead
					"Aye sir. I'll talk.",	// ORDER_SleepTalk
					"Aye sir. I'll text.",	// ORDER_SleepText
					"Aye sir. I'll watch TV.",	// ORDER_SleepTV
					"Aye sir. I'll surf the web.",	// ORDER_SleepWeb
					"Aye sir. I'll send a damage report.",	// ORDER_DamageReport
					"Aye sir. I'll conduct a planet survey scan.",	// ORDER_SurveyScanPlanet
					"Aye sir. I'll conduct a system survey scan.",	// ORDER_SurveyScanSystem
					"Aye sir. I'll conduct a tactical scan.",	// ORDER_TacticalScan
					"Aye sir. I'll report to the doc for a checkup.",	// ORDER_EnterSurgery
					"Aye sir. I have the con.",	// ORDER_TakeCommand
					"Aye sir. You have the con.",	// ORDER_ReleaseCommand
					"Aye sir. I'll drop my rider.",					// ORDER_DropRider
					"Aye sir. I'll mount you.",						// ORDER_MountMe
					"Aye sir. I'll mount that target.",				// ORDER_MountTarget
					"Aye sir. I'll dance.",					// ORDER_HengeCeremony
					"Aye sir. I'll move to %1.",	// ORDER_GoToRoom
				};
				myresponse = responses[Order()];
			}
			break;
		case ROLE_Count:
			break;
		}
	}

	// Send a response to the person who gave me the order.
	if(myresponse.contains("%"))
	{
		AuCommChat myreply(myresponse.arg(orderText), channelName, Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), scope);
		Say(myreply, myvisage);
	}
	else if(!myresponse.isEmpty())
	{
		AuCommChat myreply(myresponse, channelName, Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), scope);
		Say(myreply, myvisage);
	}
}

void DaNpc::TakePosture(const AuCommPacket& msg, CITIZEN_POSTURE posture, bool firstInGroup)
{
	if(StoryInstance())
	{
		DaCitizen::TakePosture(msg, posture, firstInGroup);
		return;
	}

	QString myresponse;
	AuVisage::VISAGE myvisage = AuVisage::Closed;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		// Workers and passengers don't take posture changes from anybody.
		if(firstInGroup)
			break;	// Only the first in the group responds to the posture change.

		// Send a response to the person who gave me the posture change.
		switch(posture)
		{
		case POSTURE_Berserk:
			myresponse = tr("I'm not going berserk.");
			break;
		case POSTURE_DefendSelf:
			myresponse = tr("I'm not holding back.");
			break;
		case POSTURE_DefendArea:
			myresponse = tr("I'm not attacking anybody that comes near me.");
			break;
		case POSTURE_Offensive:
			myresponse = tr("I'm not attacking anybody.");
			break;
		case POSTURE_HoldFire:
			myresponse = tr("I'm not holding my fire.");
			break;
		}

		myvisage = AuVisage::Sardonic;
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		// Don't permit posture change orders to unassigned Crew.
		if(AssignedShip().IsNull())
			break;

		// Base class quietly accepts the posture change.
		DaCitizen::TakePosture(msg, posture, firstInGroup);
		if(firstInGroup)
			break;	// Only the first in the group responds to the posture change.

		// Send a response to the person who gave me the posture change.
		switch(Posture())
		{
		case POSTURE_Berserk:
			if(msg.RecipientCount() > 1)
				myresponse = tr("Aye sir. We'll kill everything we see.");
			else
				myresponse = tr("Aye sir. I'll kill everything I see.");
			break;
		case POSTURE_DefendSelf:
			if(msg.RecipientCount() > 1)
				myresponse = tr("Aye sir. We'll hold our fire unless attacked.");
			else
				myresponse = tr("Aye sir. I'll hold my fire unless attacked.");
			break;
		case POSTURE_DefendArea:
			if(msg.RecipientCount() > 1)
				myresponse = tr("Aye sir. We'll watch for threats that get close.");
			else
				myresponse = tr("Aye sir. I'll watch for threats that get close.");
			break;
		case POSTURE_Offensive:
			if(msg.RecipientCount() > 1)
				myresponse = tr("Aye sir. We'll watch for threats.");
			else
				myresponse = tr("Aye sir. I'll watch for threats.");
			break;
		case POSTURE_HoldFire:
			if(msg.RecipientCount() > 1)
				myresponse = tr("Aye sir. We'll hold our fire.");
			else
				myresponse = tr("Aye sir. I'll hold my fire.");
			break;
		}
		break;
	case ROLE_Count:
		break;
	}

	if(myresponse.isEmpty())
		return;

	// Send a response to the person who gave me the posture change.
	AuCommChat myreply(myresponse, msg.ChannelName(), Id(), Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
			msg.Scope());
	Say(myreply, myvisage);
}

bool DaNpc::UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const
{
	if(InDesignPreview())
		return true;
	if(IsNpcOfficer())
	{
		if(!InChainOfCommand(sceneAtom->Id(), sceneAtom->EmpireId()))
			return false;
	}
	else
	{
		if(sceneAtom->InArena())
		{
			const DaVessel* ship = sceneAtom->FindParentVesselInEnv();
			if(!ship)
				return false;
			if(!ship->Is(AssignedShip()))
				return false;
			if(ship->EmpireId() != sceneAtom->EmpireId())
				return false;
		}
		else
		{
			if(sceneAtom->SpawnShip().IsNull())
				return false;	// Scene atom is not assigned to any ship.
			if(sceneAtom->SpawnShip() != AssignedShip())
				return false;	// I am not assigned to same ship as the scene atom.

			// Any human on the roster of my own ship can give me orders, except passengers unless I am a passenger.
			switch(Role())
			{
			case ROLE_Citizen:
			case ROLE_IndigenousPerson:
			case ROLE_IndigenousElder:
			case ROLE_IndigenousHealer:
			case ROLE_IndigenousStargazer:
			case ROLE_Worker:
			case ROLE_Count:
				return false;
			case ROLE_Passenger:
				// Any human on the roster of my own ship can give me orders.
				break;
			case ROLE_Crew:
				// Any human on the roster of my own ship can give me orders, except passengers and troops.
				if(sceneAtom->SpawnShipBerthType() == AuRosterEntry::PassengerHuman ||
						sceneAtom->SpawnShipBerthType() == AuRosterEntry::Troop)
					return false;
				break;
			case ROLE_Troop:
				// Any human on the roster of my own ship can give me orders, except passengers and crew.
				if(sceneAtom->SpawnShipBerthType() == AuRosterEntry::PassengerHuman ||
						sceneAtom->SpawnShipBerthType() == AuRosterEntry::Crew)
					return false;
				break;
			case ROLE_Officer:
				break;
			}
		}
	}

	if(topDownView && IsInSpacecraft())
	{
		if(!sceneAtom->IsInSpacecraft())
			return false;
		if(FindParentVesselInEnv() != sceneAtom->FindParentVesselInEnv())
			return false;
	}
	return true;
}

void DaNpc::InitGear(AuItemHash& cinv, AuItemHash& itemsTaken, bool equipFully)
{
	switch(Role())
	{
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		DaCitizen::InitGear(cinv, itemsTaken, equipFully);
		break;
	case ROLE_Citizen:
	case ROLE_Worker:
		{
			// Clothing in the order of preference, by gender.
			COMMODITYID clothingM[] =
			{
				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesM = sizeof(clothingM)/sizeof(COMMODITYID);

			COMMODITYID clothingF[] =
			{
				CID_CapriHalterOutfit,
				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesF = sizeof(clothingF)/sizeof(COMMODITYID);

			if(equipFully)
			{
				// Choose a random item from the city's inventory that any citizen might have.
				// Choose an item that can be carried. Do not choose clothing, weapons, ammunition or armor.
				COMMODITYID items[] =
				{
					//CID_Invalid = 0,
					CID_Flag,
					CID_Computer,
					//CID_Electricity,
					CID_Hammer,
					CID_Air,				// breathable compressed air
					//CID_Helicopter,
					//CID_SpaceRocket,
					//CID_SpaceFighter,
					//CID_SpaceTransport,
					//CID_PistolAmmo,
					//CID_HeavyArmsAmmo,
					//CID_PlasticExplosive,
					//CID_Animal = 14,
					CID_Beans,
					CID_Beer,
					//CID_LeatherBodyArmor,
					CID_Bread,
					CID_Candy,
					CID_Cheese,
					CID_Coal,
					CID_CrudeOil,
					CID_Crystals,
					//CID_Dynamite,
					CID_Eggs,
					CID_ElectronicPart,
					//CID_FarmMachinery,
					CID_Fertilizer,
					CID_Fiber,
					//CID_Pistol = 31,
					//CID_Rifle,
					CID_Fish,
					CID_Flour,
					CID_Fruit,
					CID_Gasoline,
					CID_Gems,
					CID_Grain,
					CID_Grapes,
					CID_GunPowder,
					//CID_WovenHelmet,
					CID_Herbs,
					CID_Hops,
					CID_Hydrogen,
					CID_Ice,
					CID_CrystalJewelry,
					CID_Leather,
					CID_Lumber,
					CID_AnimalMeat,
					CID_MechanicalPart,
					CID_Metal,
					CID_IceCream,
					CID_Milk,
					CID_Minerals,
					//CID_MineMachinery,
					CID_NaturalGas,
					CID_Nuts,
					CID_Ore,
					CID_Pastry,
					CID_Petrochemicals,
					CID_Pharmaceuticals,
					CID_Plastic,
					CID_Radioactives,
					//CID_CommercialShip,
					//CID_NavalShip,
					//CID_VacSuitHelmet,
					CID_MedicalKit,
					CID_Spices,
					//CID_RifleAmmo,
					CID_Stone,
					//CID_Submarine,
					CID_Sugar,
					CID_Textiles,
					//CID_VacSuit,
					CID_Vegetable,
					//CID_SUV,
					CID_Water,
					CID_Whiskey,
					CID_Wine,
					CID_Wood,
					//CID_Atmosphere,
					//CID_Hydrographics,
					//CID_Vegetation,
					//CID_PlasticBodyArmor,
					//CID_WovenBodyArmor,
					//CID_NitrateExplosive,
					//CID_Sunlight,
					//CID_LeatherClothes,
					//CID_LeatherHelmet,
					//CID_PlasticHelmet,
					CID_GoldBracelet,
					CID_GoldRing,
					CID_GemRing,
					CID_FishMeat,
					CID_Vodka,
					//CID_HeavyWeapon,
					//CID_Tank,
					//CID_APC,
					CID_Crate,
					CID_Beacon,
					CID_Lantern,
					CID_Knapsack,
					CID_Backpack,
					CID_Lumenite,
					CID_BlankDisk,
					CID_Shovel,
					CID_Gin,
					//CID_Knife,
					//CID_Shotgun,
					//CID_ShotgunAmmo,
					CID_Antidote,
					CID_Antitoxin,
					CID_Rum,
					CID_Tequila,
					CID_FittingWrench,
					CID_GlueGun,
					CID_ScrewDriver,
					CID_SewingNeedle,
					CID_SolderIron,
					CID_Tongs,
					CID_Wrench,
					CID_Hay,
					CID_Candle,
					CID_Torch,
					//CID_SailBoat,
					//CID_DirtBike,
					CID_Eludium,
					CID_Pliers,
					CID_TorqueWrench,
					CID_Vulcanite,
					CID_Ioplasma,
					CID_Magmex,
					CID_Phlogiston,
					CID_Bolite,
					CID_Cryozine,
					CID_Myrathane,
					CID_Polytaride,
					CID_Vulcium,
					CID_Magmium,
					//CID_Magma,
					//CID_SupercooledLiquid,
					//CID_InfernoAtmosphere,
					//CID_FrigidAtmosphere,
					//CID_MagmiumKnife,
					//CID_VulciumKnife,
					//CID_MagmiumPowerArmor,
					//CID_MagmiumPowerArmorHelmet,
					//CID_VulciumPowerArmor,
					//CID_VulciumPowerArmorHelmet,
					//CID_Money,
					CID_GravCoupling,
					CID_CryoHeatSink,
					CID_IoTube,
					CID_Clamp,
					CID_OScope,
					//CID_AdvSpaceFighter,
					//CID_MissionDisk,
					//CID_Bone,
					//CID_CommandAndControlModule,
					//CID_GravityDriveModule,
					//CID_RocketDriveModule,
					//CID_LifeSupportModule,
					//CID_PowerPlanetModule,
					//CID_EnergyShieldModule,
					//CID_KineticShieldModule,
					//CID_CloakingShieldModule,
					//CID_SurgeryMedicalModule,
					//CID_GeneticsMedicalModule,
					//CID_CyberneticsMedicalModule,
					//CID_PersonnelTransporterModule,
					//CID_VehicleTransporterModule,
					//CID_AtmosphereCondenserBayModule,
					//CID_ProspectorBayModule,
					//CID_TractorBeamBayModule,
					//CID_WormholeFTLDriveModule,
					//CID_WarpFTLDriveModule,
					//CID_WeaponBayModule,
					//CID_GunTurretModule,
					//CID_SensorModule,
					CID_MagmiumHeatSink,
					CID_VulciumHeatSink,
					CID_Myrasplicer,
					CID_Microscope,
					CID_Gigacell,
					CID_TesloidDissipator,
					CID_NanopathCalibrator,
					CID_AntifluxParticles,
					CID_BorexinoPrecipitate,
					//CID_TypeOPreons,
					//CID_TypeBPreons,
					//CID_TypeAPreons,
					//CID_TypeFPreons,
					//CID_TypeGPreons,
					//CID_TypeKPreons,
					//CID_TypeMPreons,
					//CID_MagmiumHullModule,
					//CID_VulciumHullModule,
					CID_Paper,
					CID_RedBinder,
					CID_GreenBinder,
					CID_BlueBinder,
					//CID_ParticleCollectorBayModule,
					CID_DNASampleKit,
					//CID_DNASample,
					CID_UtilityPouch,
					CID_GravPack,
					CID_Silage,
					//CID_Count				// LAST ITEM
				};
				int itemTypes = sizeof(items)/sizeof(COMMODITYID);

				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingF, clothingTypesF,						// Clothing
							items, itemTypes);								// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingM, clothingTypesM,						// Clothing
							items, itemTypes);								// Items
			}
			else
			{
				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingF, clothingTypesF,						// Clothing
							0, 0);											// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingM, clothingTypesM,						// Clothing
							0, 0);											// Items
			}
		}
		break;
	case ROLE_Passenger:
		{
			// Base class gets me clothing and sometimes a random item.
			DaCitizen::InitGear(cinv, itemsTaken, equipFully);

			// As a prepared space traveller, I'll need a vac suit and helmet.
			COMMODITYID armor[] =
			{
				CID_VacSuit,
			};
			int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

			COMMODITYID helmet[] =
			{
				CID_VacSuitHelmet,
			};
			int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

			DaCitizen::InitGear(cinv, itemsTaken,
					0, 0, 0,											// Weapon/ammo
					armor, armorTypes,									// Armor
					helmet, helmetTypes,								// Helmet
					0, 0,												// Clothing
					0, 0);												// Items
		}
		break;
	case ROLE_Crew:
		{
			// Clothing in the order of preference, by gender.
			COMMODITYID clothingM[] =
			{
				CID_BlueUniform,
				CID_RedUniform,
				CID_TealUniform,
				CID_PinkUniform,

				CID_BlackUniform,
				CID_GreyUniform,
				CID_GoldUniform,
				CID_WhiteUniform,

				CID_OrangeUniform,
				CID_GreenUniform,
				CID_BrownUniform,

				CID_KhakiBDU,
				CID_OliveDrabBDU,
				CID_GreyBDU,

				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesM = sizeof(clothingM)/sizeof(COMMODITYID);

			COMMODITYID clothingF[] =
			{
				CID_BlueUniform,
				CID_RedUniform,
				CID_TealUniform,
				CID_PinkUniform,

				CID_BlackUniform,
				CID_GreyUniform,
				CID_GoldUniform,
				CID_WhiteUniform,

				CID_OrangeUniform,
				CID_GreenUniform,
				CID_BrownUniform,

				CID_KhakiBDU,
				CID_OliveDrabBDU,
				CID_GreyBDU,

				CID_CapriHalterOutfit,
				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesF = sizeof(clothingF)/sizeof(COMMODITYID);

			if(equipFully)
			{
				// Choose a weapon.
				COMMODITYID weapon[] =
				{
					CID_LaserPistol,
					CID_Pistol,
					CID_Wrench,
					CID_Hammer,
				};
				// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
				COMMODITYID ammo[] =
				{
					CID_Gigacell,
					CID_PistolAmmo,
					CID_Invalid,
					CID_Invalid,
				};
				int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

				// Choose armor.
				COMMODITYID armor[] =
				{
					CID_VacSuit,
				};
				int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

				// Choose a helmet.
				COMMODITYID helmet[] =
				{
					CID_VacSuitHelmet,
				};
				int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothingF, clothingTypesF,						// Clothing
							0, 0);											// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothingM, clothingTypesM,						// Clothing
							0, 0);											// Items
			}
			else
			{
				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingF, clothingTypesF,						// Clothing
							0, 0);											// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingM, clothingTypesM,						// Clothing
							0, 0);											// Items
			}
		}
		break;
	case ROLE_Troop:
		{
			// Choose clothing.
			COMMODITYID clothing[] =
			{
				CID_OliveDrabBDU,
				CID_GreyBDU,
				CID_KhakiBDU,

				CID_BrownUniform,
				CID_GreenUniform,
				CID_OrangeUniform,

				CID_RedUniform,
				CID_BlueUniform,
				CID_TealUniform,
				CID_PinkUniform,

				CID_BlackUniform,
				CID_GreyUniform,
				CID_GoldUniform,
				CID_WhiteUniform,

				CID_LeatherClothes,
				CID_JeansAndShirt,
			};
			int clothingTypes = sizeof(clothing)/sizeof(COMMODITYID);

			if(equipFully)
			{
				// Choose a weapon.
				COMMODITYID weapon[] =
				{
					CID_LaserRifle,
					CID_AK47,
					CID_Rifle,
					CID_LaserPistol,
					CID_Shotgun,
					CID_Pistol,
					CID_VulciumKnife,
					CID_MagmiumKnife,
					CID_Knife,
					CID_Hammer,
					CID_Stone,
				};
				// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
				COMMODITYID ammo[] =
				{
					CID_Gigacell,
					CID_RifleAmmo,
					CID_RifleAmmo,
					CID_Gigacell,
					CID_ShotgunAmmo,
					CID_PistolAmmo,
					CID_Invalid,
					CID_Invalid,
					CID_Invalid,
					CID_Invalid,
					CID_Invalid,
				};
				int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

				const DaWorld* world = FindParentPlanetInEnv();
				if(world && world->HasBreathableAtmosphere())
				{
					// Choose armor.
					COMMODITYID armor[] =
					{
						CID_VulciumPowerArmor,
						CID_MagmiumPowerArmor,
						CID_MetalPowerArmor,
						CID_MetalBodyArmor,
						CID_PlasticBodyArmor,
						CID_WovenBodyArmor,
						CID_VacSuit,
						CID_LeatherBodyArmor,
					};
					int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

					// Choose a helmet.
					COMMODITYID helmet[] =
					{
						CID_VulciumPowerArmorHelmet,
						CID_MagmiumPowerArmorHelmet,
						CID_MetalPowerArmorHelmet,
						CID_MetalHelmet,
						CID_PlasticHelmet,
						CID_WovenHelmet,
						CID_VacSuitHelmet,
						CID_LeatherHelmet,
					};
					int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothing, clothingTypes,						// Clothing
							0, 0);											// Items
				}
				else
				{
					// Choose armor suitable for a harsh environment.
					COMMODITYID armor[] =
					{
						CID_VulciumPowerArmor,
						CID_MagmiumPowerArmor,
						CID_MetalPowerArmor,
						CID_VacSuit,
						CID_MetalBodyArmor,
						CID_PlasticBodyArmor,
						CID_WovenBodyArmor,
						CID_LeatherBodyArmor,
					};
					int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

					// Choose a helmet.
					COMMODITYID helmet[] =
					{
						CID_VulciumPowerArmorHelmet,
						CID_MagmiumPowerArmorHelmet,
						CID_MetalPowerArmorHelmet,
						CID_VacSuitHelmet,
						CID_MetalHelmet,
						CID_PlasticHelmet,
						CID_WovenHelmet,
						CID_LeatherHelmet,
					};
					int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothing, clothingTypes,						// Clothing
							0, 0);											// Items
				}
			}
			else
			{
				DaCitizen::InitGear(cinv, itemsTaken,
						0, 0, 0,										// Weapon/ammo
						0, 0,											// Armor
						0, 0,											// Helmet
						clothing, clothingTypes,						// Clothing
						0, 0);											// Items
			}
		}
		break;
	case ROLE_Officer:
		{
			// Clothing in the order of preference, by gender.
			COMMODITYID clothingM[] =
			{
				CID_WhiteUniform,
				CID_GoldUniform,
				CID_GreyUniform,
				CID_BlackUniform,

				CID_RedUniform,
				CID_BlueUniform,
				CID_TealUniform,
				CID_PinkUniform,

				CID_OrangeUniform,
				CID_GreenUniform,
				CID_BrownUniform,

				CID_GreyBDU,
				CID_KhakiBDU,
				CID_OliveDrabBDU,

				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesM = sizeof(clothingM)/sizeof(COMMODITYID);

			COMMODITYID clothingF[] =
			{
				CID_WhiteUniform,
				CID_GoldUniform,
				CID_GreyUniform,
				CID_BlackUniform,

				CID_RedUniform,
				CID_BlueUniform,
				CID_TealUniform,
				CID_PinkUniform,

				CID_OrangeUniform,
				CID_GreenUniform,
				CID_BrownUniform,

				CID_GreyBDU,
				CID_KhakiBDU,
				CID_OliveDrabBDU,

				CID_CapriHalterOutfit,
				CID_JeansAndShirt,
				CID_LeatherClothes,
			};
			int clothingTypesF = sizeof(clothingF)/sizeof(COMMODITYID);

			if(equipFully)
			{
				// Choose a weapon.
				COMMODITYID weapon[] =
				{
					CID_Shotgun,
					CID_LaserPistol,
					CID_Pistol,
				};
				// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
				COMMODITYID ammo[] =
				{
					CID_ShotgunAmmo,
					CID_Gigacell,
					CID_PistolAmmo,
				};
				int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

				// Choose armor.
				COMMODITYID armor[] =
				{
					CID_VulciumPowerArmor,
					CID_MagmiumPowerArmor,
					CID_MetalPowerArmor,
					CID_VacSuit,
				};
				int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

				// Choose a helmet.
				COMMODITYID helmet[] =
				{
					CID_VulciumPowerArmorHelmet,
					CID_MagmiumPowerArmorHelmet,
					CID_MetalPowerArmorHelmet,
					CID_VacSuitHelmet,
				};
				int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothingF, clothingTypesF,						// Clothing
							0, 0);											// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							weapon, ammo, weaponTypes,						// Weapon/ammo
							armor, armorTypes,								// Armor
							helmet, helmetTypes,							// Helmet
							clothingM, clothingTypesM,						// Clothing
							0, 0);											// Items
			}
			else
			{
				if(Gender() == Female)
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingF, clothingTypesF,						// Clothing
							0, 0);											// Items
				else
					DaCitizen::InitGear(cinv, itemsTaken,
							0, 0, 0,										// Weapon/ammo
							0, 0,											// Armor
							0, 0,											// Helmet
							clothingM, clothingTypesM,						// Clothing
							0, 0);											// Items
			}
		}
		break;
	}
}

void DaNpc::InitPirateGear(quint8 maxQ)
{
	switch(Role())
	{
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
	case ROLE_Citizen:
		DaCitizen::InitPirateGear(maxQ);
		break;
	case ROLE_Crew:
		{
			// Choose a weapon.
			COMMODITYID weapon[] =
			{
				CID_LaserPistol,
				CID_Pistol,
				CID_Pistol,
				CID_Shotgun,
			};
			// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
			COMMODITYID ammo[] =
			{
				CID_Gigacell,
				CID_PistolAmmo,
				CID_PistolAmmo,
				CID_ShotgunAmmo,
			};
			int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

			// Choose armor.
			COMMODITYID armor[] =
			{
				CID_VacSuit,
			};
			int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

			// Choose a helmet.
			COMMODITYID helmet[] =
			{
				CID_VacSuitHelmet,
			};
			int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

			DaCitizen::InitPirateGear(maxQ,
					weapon, ammo, weaponTypes,	// Weapon/ammo
					armor, armorTypes,									// Armor
					helmet, helmetTypes,								// Helmet
					0, 0,												// Clothing
					0, 0);												// Items

			// Base class grabs clothing and an item.
			DaCitizen::InitPirateGear(maxQ);
		}
		break;
	case ROLE_Troop:
		{
			// Choose a weapon.
			COMMODITYID weapon[] =
			{
				CID_LaserRifle,
				CID_AK47,
				CID_AK47,
				CID_Rifle,
				CID_Shotgun,
				CID_Shotgun,
				CID_Pistol,
				CID_LaserPistol,
			};
			// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
			COMMODITYID ammo[] =
			{
				CID_Gigacell,
				CID_RifleAmmo,
				CID_RifleAmmo,
				CID_RifleAmmo,
				CID_ShotgunAmmo,
				CID_ShotgunAmmo,
				CID_PistolAmmo,
				CID_Gigacell,
			};
			int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

			// Choose armor.
			COMMODITYID armor[] =
			{
				CID_VulciumPowerArmor,
				CID_MagmiumPowerArmor,
				CID_MetalPowerArmor,
				CID_MetalPowerArmor,
				//CID_MetalBodyArmor,
				//CID_MetalBodyArmor,
				//CID_PlasticBodyArmor,
				//CID_PlasticBodyArmor,
				//CID_WovenBodyArmor,
				//CID_WovenBodyArmor,
				CID_VacSuit,
				CID_VacSuit,
				CID_VacSuit,
				//CID_LeatherBodyArmor,
				//CID_LeatherBodyArmor,
			};
			int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

			// Choose a helmet.
			COMMODITYID helmet[] =
			{
				CID_VulciumPowerArmorHelmet,
				CID_MagmiumPowerArmorHelmet,
				CID_MetalPowerArmorHelmet,
				CID_MetalPowerArmorHelmet,
				//CID_MetalHelmet,
				//CID_MetalHelmet,
				//CID_PlasticHelmet,
				//CID_PlasticHelmet,
				//CID_WovenHelmet,
				//CID_WovenHelmet,
				CID_VacSuitHelmet,
				CID_VacSuitHelmet,
				CID_VacSuitHelmet,
				//CID_LeatherHelmet,
				//CID_LeatherHelmet,
			};
			int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

			DaCitizen::InitPirateGear(maxQ,
					weapon, ammo, weaponTypes,	// Weapon/ammo
					armor, armorTypes,									// Armor
					helmet, helmetTypes,								// Helmet
					0, 0,												// Clothing
					0, 0);												// Items

			// Base class grabs clothing and an item.
			DaCitizen::InitPirateGear(maxQ);
		}
		break;
	case ROLE_Officer:
		{
			// Choose a weapon.
			COMMODITYID weapon[] =
			{
				CID_Shotgun,
				CID_LaserPistol,
				CID_Shotgun,
				CID_Pistol,
			};
			// This table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
			COMMODITYID ammo[] =
			{
				CID_ShotgunAmmo,
				CID_Gigacell,
				CID_ShotgunAmmo,
				CID_PistolAmmo,
			};
			int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

			// Choose armor.
			COMMODITYID armor[] =
			{
				CID_VulciumPowerArmor,
				CID_MagmiumPowerArmor,
				CID_MetalPowerArmor,
				CID_MetalPowerArmor,
				CID_MetalPowerArmor,
				CID_VacSuit,
				CID_VacSuit,
			};
			int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

			// Choose a helmet.
			COMMODITYID helmet[] =
			{
				CID_VulciumPowerArmorHelmet,
				CID_MagmiumPowerArmorHelmet,
				CID_MetalPowerArmorHelmet,
				CID_MetalPowerArmorHelmet,
				CID_MetalPowerArmorHelmet,
				CID_VacSuitHelmet,
				CID_VacSuitHelmet,
			};
			int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

			DaCitizen::InitPirateGear(maxQ,
					weapon, ammo, weaponTypes,	// Weapon/ammo
					armor, armorTypes,									// Armor
					helmet, helmetTypes,								// Helmet
					0, 0,												// Clothing
					0, 0);												// Items

			// Base class grabs clothing and an item.
			DaCitizen::InitPirateGear(maxQ);
		}
		break;
	}
}

void DaNpc::InitIndigenousGear(const AuZoneResource& res)
{
	// Choose a weapon.
	COMMODITYID weapon[] =
	{
		CID_Invalid,
		CID_Knife,
		CID_Wood,
		CID_Wood,
		CID_Hammer,
		CID_Hammer,
		CID_Hammer,
	};
	int weaponTypes = sizeof(weapon)/sizeof(COMMODITYID);

	// Natural resource that determines the quality of the weapon.
	COMMODITYID weaponQ[] =
	{
		CID_Invalid,
		CID_Ore,
		CID_Wood,
		CID_Wood,
		CID_Wood,
		CID_Stone,
		CID_Stone,
	};

	// The ammo table matches 1:1 the weapons table. Be sure to put the appropriate ammo here.
	COMMODITYID ammo[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
	};

	// Natural resource that determines the quality of the ammo.
	COMMODITYID ammoQ[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
	};

	// Armor.
	COMMODITYID armor[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_LeatherBodyArmor,
		CID_LeatherBodyArmor,
	};
	int armorTypes = sizeof(armor)/sizeof(COMMODITYID);

	// Natural resource that determines the quality of the armor.
	COMMODITYID armorQ[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Fiber,
		CID_Fiber,
		CID_Animal,
		CID_Animal,
	};

	// Helmet.
	COMMODITYID helmet[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_LeatherHelmet,
		CID_LeatherHelmet,
	};
	int helmetTypes = sizeof(helmet)/sizeof(COMMODITYID);

	// Natural resource that determines the quality of the helmet.
	COMMODITYID helmetQ[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Fiber,
		CID_Fiber,
		CID_Animal,
		CID_Animal,
	};

	// Choose clothing.
	COMMODITYID clothing[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_LeatherClothes,
		CID_LeatherClothes,
	};
	int clothingTypes = sizeof(clothing)/sizeof(COMMODITYID);

	// Natural resource that determines the quality of the clothing.
	COMMODITYID clothingQ[] =
	{
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Invalid,
		CID_Animal,
		CID_Animal,
	};

	// Items.
	COMMODITYID items[] =
	{
		CID_Animal,
		CID_Beans,
		CID_Beer,
		CID_Bread,
		CID_Cheese,
		CID_Coal,
		CID_CrudeOil,
		CID_Crystals,
		CID_Eggs,
		CID_Fiber,
		CID_Fish,
		CID_Flour,
		CID_Fruit,
		CID_Gems,
		CID_Grain,
		CID_Grapes,
		CID_Herbs,
		CID_Hops,
		CID_CrystalJewelry,
		CID_Leather,
		CID_AnimalMeat,
		CID_Milk,
		CID_Minerals,
		CID_Nuts,
		CID_Ore,
		CID_Pharmaceuticals,
		CID_Radioactives,
		CID_MedicalKit,
		CID_Spices,
		CID_Sugar,
		CID_Textiles,
		CID_Vegetable,
		CID_Water,
		CID_Wine,
		CID_GemRing,
		CID_FishMeat,
		CID_Knapsack,
		CID_Backpack,
		CID_Antidote,
		CID_SewingNeedle,
		CID_Torch,
		CID_Bone,
		CID_Paper,
		CID_FlowerPotSmall,
		CID_FlowerPotLarge,
		CID_Candle,
	};
	int itemTypes = sizeof(items)/sizeof(COMMODITYID);

	// Natural resource that determines the quality of the items.
	COMMODITYID itemQ[] =
	{
		CID_Animal,
		CID_Beans,
		CID_Hops,
		CID_Grain,
		CID_Cheese,
		CID_Coal,
		CID_CrudeOil,
		CID_Crystals,
		CID_Eggs,
		CID_Fiber,
		CID_Fish,
		CID_Grain,
		CID_Fruit,
		CID_Gems,
		CID_Grain,
		CID_Grapes,
		CID_Herbs,
		CID_Hops,
		CID_Crystals,
		CID_Animal,
		CID_AnimalMeat,
		CID_Milk,
		CID_Minerals,
		CID_Nuts,
		CID_Ore,
		CID_Herbs,
		CID_Radioactives,
		CID_Herbs,
		CID_Spices,
		CID_Fruit,
		CID_Fiber,
		CID_Vegetable,
		CID_Water,
		CID_Grapes,
		CID_Fish,
		CID_Animal,
		CID_Animal,
		CID_Minerals,
		CID_Animal,
		CID_CrudeOil,
		CID_Animal,
		CID_Wood,
		CID_Stone,
		CID_Stone,
		CID_Animal,
	};

	// Create items in my inventory.
	AuGearPos pos = CurrentItemPos();
	AuGear* gear = LockGearForWrite();

	// Choose a weapon.
	int ii = g_Au.Dice().Roll(1,weaponTypes,-1);
	if(weapon[ii] != CID_Invalid)
	{
		// Make the weapon if I don't already have one in hand.
		const AuItem* gearItem = gear->ItemAt(pos);
		if(!gearItem || gearItem->CommodityId() != weapon[ii])
		{
			// Make weapon.
			quint8 q = res.Quality(weaponQ[ii]); 
			if(!q)
				q = 1;
			AuItem* item = AuItem::Create(weapon[ii], q, 0, 1);
			if(item)
			{
				AddGearItemUseDropNoLock(*item, 1, false);
				delete item;
				item = 0;
			}
		}

		// Does the weapon need ammo?
		if(ammo[ii] != CID_Invalid)
		{
			// The weapon uses ammo.
			// Count units of ammunition in my inventory.
			int ammoCount = 0;
			for(AuGear::const_iterator ai = gear->cbegin(); ai != gear->cend(); ai++)
			{
				const AuItemAmmo* ammoItem = dynamic_cast<const AuItemAmmo*>(&(*ai).Item());
				if(!ammoItem)
					continue;	// The item is not ammo.
				if(ammoItem->CommodityId() == ammo[ii])
					ammoCount++;
			}

			if(ammoCount < 5)
			{
				// Make ammo.
				quint8 q = res.Quality(ammoQ[ii]); 
				if(!q)
					q = 1;
				AuItem* item = AuItem::Create(ammo[ii], q, 0, 1);
				if(item)
				{
					int ammoToMake = 5 - ammoCount;				// Enough to fill free hand and 4 waist positions.
					while(ammoToMake--)
						AddGearItemUseDropNoLock(*item, 0, false);
					delete item;
					item = 0;
				}
			}
		}
	}

	// Choose armor.
	if(!gear->ArmorItemWorn())
	{
		// I do not currently have armor.
		int ii = g_Au.Dice().Roll(1,armorTypes,-1);
		if(armor[ii] != CID_Invalid)
		{
			// Make armor.
			quint8 q = res.Quality(armorQ[ii]); 
			if(!q)
				q = 1;
			AuItem* item = AuItem::Create(armor[ii], q, 0, 1);
			if(item)
			{
				AddGearItemUseDropNoLock(*item, 1, false);
				delete item;
				item = 0;
			}
		}
	}

	// Choose helmet.
	if(!gear->HelmetItemWorn())
	{
		// I do not currently have a helmet.
		int ii = g_Au.Dice().Roll(1,helmetTypes,-1);
		if(helmet[ii] != CID_Invalid)
		{
			// Make helmet.
			quint8 q = res.Quality(helmetQ[ii]); 
			if(!q)
				q = 1;
			AuItem* item = AuItem::Create(helmet[ii], q, 0, 1);
			if(item)
			{
				AddGearItemUseDropNoLock(*item, 1, false);
				delete item;
				item = 0;
			}
		}
	}

	// Choose clothing.
	if(!gear->ClothingItemWorn())
	{
		// I do not currently have a clothing.
		int ii = g_Au.Dice().Roll(1,clothingTypes,-1);
		if(clothing[ii] != CID_Invalid)
		{
			// Make clothing.
			quint8 q = res.Quality(clothingQ[ii]); 
			if(!q)
				q = 1;
			AuItem* item = AuItem::Create(clothing[ii], q, 0, 1);
			if(item)
			{
				AddGearItemUseDropNoLock(*item, 1, false);
				delete item;
				item = 0;
			}
		}
	}

	// Special items.
	if(Role() == ROLE_IndigenousHealer)
	{
		quint8 q = res.Quality(CID_Minerals);
		if(!q)
			q = 1;
		AuItem* antidote = AuItem::Create(CID_Antidote, q, 0, 1);
		if(antidote)
		{
			AddGearItemUseDropNoLock(*antidote, 1, false);
			delete antidote;
			antidote = 0;
		}

		q = res.Quality(CID_Animal);
		if(!q)
			q = 1;
		AuItem* knapsack = AuItem::Create(CID_Knapsack, q, 0, 1);
		if(knapsack)
		{
			AddGearItemUseDropNoLock(*knapsack, 1, false);
			delete knapsack;
			knapsack = 0;
		}

		q = res.Quality(CID_Herbs);
		if(!q)
			q = 1;
		AuItem* medkit = AuItem::Create(CID_MedicalKit, q, 0, 1);
		if(medkit)
		{
			AddGearItemUseDropNoLock(*medkit, 0, false);
			delete medkit;
			medkit = 0;
		}
	}

	// Choose item.
	ii = g_Au.Dice().Roll(1,itemTypes,-1);
	if(items[ii] != CID_Invalid)
	{
		// Make item.
		quint8 q = res.Quality(itemQ[ii]); 
		if(!q)
			q = 1;
		AuItem* item = AuItem::Create(items[ii], q, 0, 1);
		if(item)
		{
			AddGearItemUseDropNoLock(*item, 0, false);
			delete item;
			item = 0;
		}
	}

	UnlockGear();
}

bool DaNpc::GainExperienceForWorking()
{
	if(StoryInstance())
		return false;
	if(!DaCitizen::GainExperienceForWorking())
		return false;	// Not time to gain experience.

	// Gain 1 experience point for doing my job.
	if(m_Con && Role() == ROLE_Officer)
	{
		IncreaseSkill(AuSkill::Command, 1);
		return true;
	}

	// Generic spaceship skill.
	switch(ConsoleType())
	{
	case AuStation::Console_Command:
	case AuStation::Console_Patient:
	case AuStation::Integrated_Command:
	case AuStation::Integrated_Patient:
	case AuStation::Table_Game:
	case AuStation::Table_Game_0:
	case AuStation::Table_Game_45:
	case AuStation::Berth_Captain:
	case AuStation::Berth_Crew:
	case AuStation::Berth_Officer:
	case AuStation::Berth_Passenger:
	case AuStation::Berth_Troop:
	case AuStation::Berth_1_0:
	case AuStation::Berth_1_45:
	case AuStation::Berth_1_90:
	case AuStation::Berth_1_135:
	case AuStation::Berth_1_180:
	case AuStation::Berth_1_225:
	case AuStation::Berth_1_270:
	case AuStation::Berth_1_315:
	case AuStation::Berth_2_0:
	case AuStation::Berth_2_45:
	case AuStation::Berth_2_90:
	case AuStation::Berth_2_135:
	case AuStation::Berth_2_180:
	case AuStation::Berth_2_225:
	case AuStation::Berth_2_270:
	case AuStation::Berth_2_315:
	case AuStation::Citizen:
	case AuStation::Livestock:
	case AuStation::Integrated_Designer:
	case AuStation::TYPE_Count:
		// No specific station skill increase.
		switch(Role())
		{
		case ROLE_Citizen:
		case ROLE_IndigenousPerson:
		case ROLE_IndigenousElder:
		case ROLE_IndigenousHealer:
		case ROLE_IndigenousStargazer:
		case ROLE_Worker:
		case ROLE_Passenger:
		case ROLE_Count:
			break;
		case ROLE_Crew:
		case ROLE_Officer:
			IncreaseSkill(AuSkill::SpacecraftOperations, 1);	// Generic spaceship skill.
			break;
		case ROLE_Troop:
			IncreaseSkill(AuSkill::Guard, 1);
			break;
		}
		break;
	case AuStation::Console_Engine:
	case AuStation::Integrated_Engine:
		IncreaseSkill(AuSkill::Engineer, 1);
		break;
	case AuStation::Console_FireControl:
	case AuStation::Integrated_FireControl:
		IncreaseSkill(AuSkill::FireControl, 1);
		break;
	case AuStation::Console_Helm:
	case AuStation::Integrated_Helm:
		IncreaseSkill(AuSkill::Helm, 1);
		break;
	case AuStation::Console_Medic:
	case AuStation::Integrated_Medic:
		IncreaseSkill(AuSkill::Medic, 1);
		break;
	case AuStation::Console_Navigation:
	case AuStation::Integrated_Navigation:
		IncreaseSkill(AuSkill::Navigator, 1);
		break;
	case AuStation::Console_Power:
	case AuStation::Integrated_Power:
		IncreaseSkill(AuSkill::Power, 1);
		break;
	case AuStation::Console_Sensor:
	case AuStation::Integrated_Sensor:
		IncreaseSkill(AuSkill::Sensor, 1);
		break;
	case AuStation::Console_Shield:
	case AuStation::Integrated_Shield:
		IncreaseSkill(AuSkill::Shield, 1);
		break;
	case AuStation::Console_Transporter:
	case AuStation::Integrated_Transporter:
		IncreaseSkill(AuSkill::Transporter, 1);
		break;
	case AuStation::Turret_Integrated:
	case AuStation::Turret_Bottom:
	case AuStation::Turret_Top:
	case AuStation::Turret_0:
	case AuStation::Turret_45:
	case AuStation::Turret_90:
	case AuStation::Turret_135:
	case AuStation::Turret_180:
	case AuStation::Turret_225:
	case AuStation::Turret_270:
	case AuStation::Turret_315:
		IncreaseSkill(AuSkill::Turret, 1);
		break;
	case AuStation::Troop:
		IncreaseSkill(AuSkill::Guard, 1);
		break;
	}
	return true;
}

void DaNpc::DoBattleStation()
{
	if(StoryInstance())
		return;
	if(IsRider() || IsMounted())
	{
		// I am on a mount or I am mounted; I am at my station.
		StayIdle();
		return;
	}

	if(IsAboardVehicle())
	{
		// I am in a vehicle; I am at my station.
		// Do random turning and looking behavior.
		StayIdle();
		return;
	}

	if(!IsInSpacecraft())
	{
		// I'm not in a vehicle or a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Check to see if I need to run to the sick bay for surgery.
	if(GetACheckup())
		return;													// I have decided to get medical attention.

	// I am aboard a spacecraft.
	DaVessel* ship = dynamic_cast<DaVessel*>(Parent());
	if(!ship)
	{
		// I'm not in a vehicle or a spacecraft. Cancel this order and move on to the next order.
		RequestOrder();	// Ask for my next order.
		return;	// Parent ship not found or not ready for analysis.
	}
	if(!ship->IsNormal())
		return;				// Wait for ship to be normal. It is probably waiting to wormhole or something.
	if(!ship->IsDesignDataCacheValid())
		return;
	if(PoliticalStance(EmpireId(), ship->EmpireId()) == POL_Enemy)
	{
		RequestOrder();	// Ask for my next order.
		return;	// Ship is not of my empire.
	}

	// Do station AI based on the berth I hold.
	if(IsNpcOfficer())
	{
		// Officer AI cobbled from DaCitOfficer::DoBattleStation.
		if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 2)
		{
			// Troops are always watching for threats.
			if(LookForThreats(false))
				return;						// Threat spotted; pursue and destroy.
		}

		// Command the ship to accomplish its mission.
		bool imInCommand = false;
		if(HasTheCon() && Is(ship->AIFirstOfficer()))
		{
			// I am in command because I am the first officer and I have the con.
			imInCommand = true;
			CommandShip(ship);
		}

		if(IsCommandStationPose())
		{
			// Sit idly in my Captain's chair.
			StayIdle();

			// Take a nap if the mission is inactive or has no orders.
			if(!imInCommand || !ship->IsOnAMission())
				TakeANap();										// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a command console yet.
			// Locate the closest unmanned command console to my location.
			// If all consoles are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			if(ship->ClosestUnoccupiedCommandStation(Location(), stationId, station))
			{
				// Found an unmanned console. Try to sit at the console if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the console or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, 0);
				}
				else
				{
					// Move close to the unmanned console.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->BerthDetailIdOccupied(Id()))
			{
				// No unmanned console and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();		// Wait for an empty console.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					//qDebug("DaPerson::DoBattleStation %d %s taking a nap in my bunk",
						//Id().GetID(), (const char*)Name().toAscii());
					TakeANap();
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his bunk.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned console and I don't have a berth aboard this ship.
				StayIdle();		// Wait for an empty console.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				//qDebug("DaPerson::DoBattleStation %d %s taking a nap", Id().GetID(), (const char*)Name().toAscii());
				TakeANap();
			}
		}
	}
	else if(IsNpcCrew())
	{
		// Crew AI copied from DaCitCrew::DoBattleStation.
		if(IsEngineStationPose())
		{
			// I am operating a sensor console.
			// Operate engineer console even when not in a scene because it is used to turn on the power plant and drives.
			OperateEngineerConsole(ship);
		}
		else if(IsFireControlStationPose())
		{
			// I am operating a fire control console.
			// Operate fire control consoles even when not in a scene because they can reach a long way, beyond the scene.
			OperateFireControlConsole(ship);
		}
		else if(IsNavStationPose())
		{
			// I am operating a nav console.
			// Operate nav console even when not in a scene because it is used to execute mission orders.
			if(Is(ship->AINavigator()))
				OperateNavigatorConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsHelmStationPose())
		{
			// I am operating a helm.
			// Operate pilot console even when not in a scene because it is used to turn on the power plant and gravity drive.
			if(Is(ship->AIPilot()))
				OperatePilotConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsPowerStationPose())
		{
			// I am operating a power relay console.
			// Operate power relay console even when not in a scene because changes may be needed to fix the ship.
			if(Is(ship->AIPowerRelay()))
				OperatePowerRelayConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsSensorStationPose())
		{
			// I am operating a sensor console.
			// Operate sensor console even when not in a scene because it is used to target the weapon bay.
			if(Is(ship->AISensor()))
				OperateSensorConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsShieldStationPose())
		{
			// I am operating a sensor console.
			// Operate shield console even when not in a scene because it is used to turn on the shields.
			if(Is(ship->AIShield()))
				OperateShieldConsole(ship);
			else
			{
				if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
				{
					// Off duty crew watches for threats.
					if(LookForThreats(false))
						return;						// Threat spotted; pursue and destroy.
				}
				TakeANap();							// Sleep when I am not on duty.
			}
		}
		else if(IsMedicStationPose())
		{
			// I am operating a surgery console.
			// Only operate surgery console if I'm in somebody's scene.
			// WHY? Don't sick dudes go to the sick bay when they need help?
			//if(IsInAnyScene())
				OperateSurgeryConsole(ship);
			//else
				//TakeANap();							// Sleep when I am not on duty.
		}
		else if(IsTransporterStationPose())
		{
			// I am operating a sensor console.
			// Operate transporter console even when not in a scene because it is used to turn off the power when not in use.
			OperateTransporterConsole(ship);
		}
		else if(IsLockedPose())
		{
			// I am manning some other(?) console.
			if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 5)
			{
				// Off duty crew watches for threats.
				if(LookForThreats(false))
					return;						// Threat spotted; pursue and destroy.
			}
			TakeANap();							// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a console yet.
			if(IsInAnyScene() && LastLookForThreatsTime().ElapsedSec() > 3)
			{
				// Off duty crew watches for threats.
				if(LookForThreats(false))
					return;						// Threat spotted; pursue and destroy.
			}

			// Locate the closest unmanned console to my location.
			// If all consoles are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			quint16 which;
			if(ship->ClosestUnoccupiedCrewStation(Location(), stationId, station, which, BestCrewStationSkill()))
			{
				// Found an unmanned console. Try to sit at the console if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the console or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, which);
				}
				else
				{
					// Move close to the unmanned console.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->IsBerthedHere(Id()))
			{
				// No unmanned console and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();							// Wait for an empty turret.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					TakeANap();							// Sleep when I am not on duty.
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his station.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned console and I don't have a berth aboard this ship.
				StayIdle();							// Wait for an empty console.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				TakeANap();							// Sleep when I am not on duty.
			}
		}
	}
	else if(IsNpcTroop())
	{
		// Troop AI cobbled from DaCitTroop::DoBattleStation.
		// Aboard a spacecraft, troops only operate turrets.
		// Turrets are not operated when not in any scene.
		// This also prevents this dude from moving to a turret when not in any scene.
		if(IsInAnyScene())
		{
			// Troops are always watching for threats.
			if(IsTurretPose())
			{
				// Troops watch for threats less often when their head is in a turret.
				if(LastLookForThreatsTime().ElapsedSec() > 5 && LookForThreats(false))
					return;					// Threat spotted; pursue and destroy.
			}
			else
			{
				// Troops are alert for threats when not operating a turret.
				if(LastLookForThreatsTime().ElapsedSec() > 1 && LookForThreats(false))
					return;					// Threat spotted; pursue and destroy.
			}
		}
		else
		{
			// I am not in any scene.
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
			if(system && !system->IsEnemyEmpirePresent(ship->EmpireId()))
				return;	// Do nothing when the system contains no enemy units.
		}

		// Close jammed doors if the atmosphere inside is bad.
		AuPoint doorLoc;
		if(!IsAirBreathable() && ship->AnyDoorJammed(doorLoc))
		{
			doorLoc = ship->FloorLocInsideDoor(doorLoc);
			AmendOrder(ORDER_CloseDoor, ship->Id(), doorLoc, 0);				// Close the door when I get there.
			AmendOrder(ORDER_GoToTargetExactly, ship->Id(), doorLoc, 0);		// First, go to the door.
		}
		else if(IsTurretPose())
		{
			// I am operating a turret.
			// We only operate turrets when we are in a scene. Turrets don't reach far enough to need this work outside of a scene.
			// This test also insures that my turret is not currently turning; when turning we let the turret keep turning until it
			// reaches the angle I selected.
			OperateTurret(ship);
		}
		else if(IsLockedPose())
		{
			// I am manning some other console, probably a guard post.
			TakeANap();							// Sleep when I am not on duty.
		}
		else
		{
			// I am not manning a turret or at a guard post yet.
			// Locate the closest unmanned turret or troop station to my location.
			// If all turrets and troop stations are manned, I'll go to my bunk and wait.
			// If I don't have a bunk, I'll wait where I am standing.
			quint16 stationId;
			AuStation station;
			quint16 which;
			if(ship->ClosestUnoccupiedTroopStation(Location(), stationId, station, which, Id()))
			{
				// Found an unmanned turret or troop station. Try to sit at the turret if it is close enough.
				AuPoint destLoc = station.OpM().Translation();
				//if(Location().Distance2_2D(destLoc) < 5*5 || ship->OkToTeleportToStation(this))
				AuPoint conLoc = station.OpM().UnProject(Location());	// My location in the space of the console.
				if(conLoc.Length2_2D() < 4*4 || ship->OkToTeleportToStation(this))
				{
					// I am close enough to the turret/post or it's ok to just teleport this dude to his station.
					// Adjust for pose.
					// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
					station.OffsetOpM(ConsolePoseOffset(station.Type(), station.OccPos(), CGHeightStanding(), TorsoThickness(),
								station.SeatIndex()));
					DoSitAtConsole(stationId, station, which);
				}
				else
				{
					// Move close to the unmanned turret station or guard post loc.
					// Amend my orders with a movement order that will get me there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), destLoc, stationId);
				}
			}
			else if(ship->BerthDetailIdOccupied(Id()))
			{
				// No unmanned turret and I have a berth aboard this ship.
				quint16 roomId;
				AuPoint berthLoc = ship->BerthLocation(Id(), roomId);

				// When going to my berth, don't be too precise about it or my bunkmate and I stand on top of each other.
				if(roomId == ShipRoomId())
				{
					// I am in my bunk room. That's close enough.
					StayIdle();		// Wait for an empty turret.

					// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
					TakeANap();
				}
				else
				{
					// Move to my bunk room.
					// Adjust the destination location to be in the room and not exactly on the floor, ceiling, or wall line.
					AuPoint destLoc = berthLoc - ship->DesignCG();
					destLoc.z += CGHeightStanding();
					if(ship->OkToTeleportToStation(this))
					{
						// It's ok to just teleport this dude to his station.
						DoTeleport(destLoc);
					}
					else
					{
						// Amend my orders with a movement order that will get me there.
						AmendOrder(ORDER_GoToTarget, ship->Id(), destLoc, 0);
					}
				}
			}
			else
			{
				// No unmanned turret and I don't have a berth aboard this ship.
				StayIdle();		// Wait for an empty turret.

				// Amend my orders with a sleep order so I won't be a load on the server looking for an empty console.
				TakeANap();
			}
		}
	}
	else
	{
		// Passengers don't do anything.
		RequestOrder();	// Ask for my next order.
		return;
	}
}

void DaNpc::DoGarrison()
{
	if(StoryInstance())
		return;
	// Get the name of the ship I am deploying from, if possible.
	if(!AssignedShip().IsNull() && !IsInsideSpacecraft() && !BdId().IsNull())
	{
		// I am assigned to a ship.
		DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			DaVessel* ship = (DaVessel*)system->FindIdRecursive(AssignedShip());
			if(ship)
				SetLastShipName(ship->NameWithEmpire());
		}
	}

	// Base class DoGarrison releases my berth, if I have any.
	DaCitizen::DoGarrison();

	if(!DecisionCount())
	{
		// Base class is finished releasing my berth, etc.
		if(AssignedShip().IsNull())
		{
			const DaVessel* ship = FindParentVesselInEnv();
			if(!ship)
				return;
			if(ship->PoliticalStanceMakeContact(this) == POL_Enemy)
				return;
			bool berthAvailable = false;
			if(CommandExperiencePoints())
			{
				// Try to take an officer berth.
				if(ship->EmpireId() == EmpireId() && ship->OfficerBerthsAvailable())
				{
					berthAvailable = true;
					DoMoveInOfficer();
				}
			}

			if(!berthAvailable && CrewExperiencePoints())
			{
				if(ship->CanAcceptBerthType(AuRosterEntry::Crew) && ship->CrewBerthsAvailable())
				{
					berthAvailable = true;
					DoMoveInCrew();
				}
			}

			if(!berthAvailable && TroopExperiencePoints())
			{
				if(ship->EmpireId() == EmpireId() && ship->CanAcceptBerthType(AuRosterEntry::Troop) && ship->TroopBerthsAvailable())
				{
					berthAvailable = true;
					DoMoveInTroop();
				}
			}

			if(!berthAvailable)
			{
				if(ship->CanAcceptBerthType(AuRosterEntry::PassengerAI) && ship->PassengerBerthsAvailable())
				{
					berthAvailable = true;
					DoMoveInPassenger();
				}
			}
		}
		RequestOrder();											// Ask for my next order.
	}
}

// Crew are restricted to what weapons and armor they can wear.
bool DaNpc::CanUseGear(const AuItem& item, AuGearSlot::SLOTTYPE slotType) const
{
	// Check base class.
	return DaCitizen::CanUseGear(item, slotType);
}

QString DaNpc::DescribeActivity() const
{
	if(IsMating())
		return DaCitizen::DescribeActivity();
	QString order;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		switch(Order())
		{
		default:
			return DaCitizen::DescribeActivity();

		case ORDER_Stay:
			order = tr("Loitering");
			break;
		case ORDER_SleepDayDream:
			order = tr("Imagining My Destination");
			break;
		case ORDER_SleepExercise:
			order = tr("Aerobics");
			break;
		case ORDER_SleepPine:
			order = tr("Missing My Family");
			break;
		case ORDER_SleepRead:
			order = tr("Reading a Book");
			break;
		case ORDER_TakeCommand:
			order = tr("Humming tunelessly.");
			break;
		case ORDER_ReleaseCommand:
			order = tr("Whistling tunelessly.");
			break;
		}
		break;
	case ROLE_Crew:
		switch(Order())
		{
		default:
			return DaCitizen::DescribeActivity();

		case ORDER_SleepDayDream:
			order = tr("Daydreaming About Spacecraft");
			break;
		case ORDER_SleepPine:
			order = tr("Yearning for That Dancer at the Last Port of Call");
			break;
		case ORDER_SleepRead:
			order = tr("Reading a Technical Manual");
			break;
		case ORDER_TakeCommand:
			order = tr("Fidgeting");
			break;
		case ORDER_ReleaseCommand:
			order = tr("Sighing With Relief");
			break;
		}
		break;
	case ROLE_Troop:
		switch(Order())
		{
		default:
			return DaCitizen::DescribeActivity();

		case ORDER_Stay:
			order = tr("Guarding");
			break;
		case ORDER_Garrison:
			order = tr("Fortifying");
			break;
		case ORDER_SleepDayDream:
			order = tr("Fantasizing About a Bigger Gun");
			break;
		case ORDER_SleepExercise:
			order = tr("Pumping Iron");
			break;
		case ORDER_SleepPine:
			order = tr("Longing for Love at the Next World");
			break;
		case ORDER_SleepRead:
			order = tr("Paging Through a Magazine");
			break;
		case ORDER_SleepTalk:
			order = tr("Bragging");
			break;
		case ORDER_TakeCommand:
		case ORDER_ReleaseCommand:
			order = tr("Waiting");
			break;
		}
		break;
	case ROLE_Officer:
		switch(Order())
		{
		default:
			order = DaCitizen::DescribeActivity();
			break;
		case ORDER_SleepDayDream:
			order = tr("Wondering at the Vast Depths of Space");
			break;
		case ORDER_SleepExercise:
			order = tr("Calisthenics");
			break;
		case ORDER_SleepPine:
			if(m_WorldId == AuId(1))
				order = tr("Pining for My Love Back Wherever %1 Conjured Me From").arg(HomeWorldName());	// Agent spawn
			else
				order = tr("Pining for My Love Back on %1").arg(HomeWorldName());
			break;
		case ORDER_SleepRead:
			order = tr("Reading Military Regulations");
			break;
		}

		if(m_Con)
		{
			if(dynamic_cast<const DaVessel*>(Parent()))
				order = tr("Commanding %1, %2").arg(Parent()->Name()).arg(order);
			else
				order = tr("Commanding Spacecraft, %1").arg(order);
		}
		break;
	}
	return order;
}

void DaNpc::operator>>(QDataStream& out) const
{
	DaCitizen::operator>>(out);
	quint8 hasWhat = 0;
	if(m_Con)
		hasWhat |= 0x01;
	if(m_WorldBodyType != BODY_TYPE_Count)
		hasWhat |= 0x02;
	hasWhat |= 0x04;	// contact time no longer included

	out << hasWhat;
	out << m_Galaxy;
	out << m_HomeSickness;
	out << m_Indestructible;
	out << m_Role;
	out << m_Say;
	if(hasWhat & 0x02)
		out << m_WorldBodyType;

	m_NpcLock.lockForRead();
	out << m_AccountBalance;
	out << m_Companion;
	out << m_GarrisonBd;
	out << m_PortBd;
	out << m_SpouseName;
	out << m_StoryInstance;
	out << m_HomeSickTime;
	out << m_SectorId;
	out << m_SectorName;
	out << m_SystemId;
	out << m_SystemName;
	out << m_WorldId;
	out << m_WorldName;
	out << (quint16)m_WantCID;
	out << m_WantQ;
	out << m_WantEoO;
	out << m_WantInscription;
	m_NpcLock.unlock();

	m_LogLock.lockForRead();
	out << m_Log;
	out << m_LogTime;
	m_LogLock.unlock();
}

void DaNpc::operator<<(QDataStream& in)
{
	DaCitizen::operator<<(in);
	quint8 hasWhat;
	in >> hasWhat;

	m_Con = hasWhat & 0x01;

	in >> m_Galaxy;
	in >> m_HomeSickness;
	in >> m_Indestructible;
	in >> m_Role;
	in >> m_Say;
	if(hasWhat & 0x02)
		in >> m_WorldBodyType;

	m_NpcLock.lockForWrite();
	in >> m_AccountBalance;
	in >> m_Companion;
	in >> m_GarrisonBd;
	in >> m_PortBd;
	in >> m_SpouseName;
	in >> m_StoryInstance;
	if(!(hasWhat & 0x04))
	{
		AuTime contact;
		in >> contact;
	}
	in >> m_HomeSickTime;
	in >> m_SectorId;
	in >> m_SectorName;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_WorldId;
	in >> m_WorldName;
	quint16 ui16;
	in >> ui16;
	m_WantCID = (COMMODITYID)ui16;
	in >> m_WantQ;
	in >> m_WantEoO;
	in >> m_WantInscription;
	m_NpcLock.unlock();

	m_LogLock.lockForWrite();
	in >> m_Log;
	in >> m_LogTime;
	m_LogLock.unlock();
}

void DaNpc::DoScan()
{
	if(!IsSensorStationPose())
	{
		// Can only do a scan while seated at a sensor console.
		RequestOrder();	// Ask for my next order.
		return;
	}

	if(InArena() || InDesignPreview())
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	DaVessel* ship = dynamic_cast<DaVessel*>(Parent());
	if(!ship)
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	// Consoles cannot function when there is no power to the ship.
	if(ship->CapacitorLevel() <= 0)
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	if(!ship->IsConsoleOperational(StationSysId()))
	{
		ExitConsole();
		return;
	}

	// Don't do anything if I'm not the "current" AI sensor operator.
	if(!Is(ship->AISensor()))
	{
		RequestOrder();	// Ask for my next order.
		return;
	}

	DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return;	// Not likely.

	AuId systemId = system->Id();
	AuId worldId;
	QString worldName;
	const DaGalaxy* pg = system->FindParentGalaxy();
	if(!pg)
		return;
	GALAXY galaxy = pg->Galaxy();
	AuPoint galaxyLoc;
	system->GetLocationRelativeToParent(galaxyLoc, pg);

	const DaSector* sector = system->FindParentSector();

	AuIdList places;
	places.append(AuId::Universe());
	places.append(pg->Id());
	places.append(sector->Id());
	places.append(system->Id());

	// Prepare a report.
	QString title;
	QString rpt;
	if(Order() == ORDER_SurveyScanPlanet)
	{
		QString planetName;
		const DaWorld* planet = FindParentPlanetInEnv();
		if(planet)
		{
			// If the world has rings, I could be in one of them. If so, report the ring.
			AuPoint planetLoc;
			GetLocationRelativeToParent(planetLoc, planet);
			const DaWorldRing* ring = planet->FindRing(planetLoc);
			if(ring)
			{
				worldId = ring->Id();
				worldName = ring->Name();
				planetName = ring->Name();
			}
			else
			{
				worldId = planet->Id();
				worldName = planet->Name();
				planetName = planet->Name();
			}
			title = tr("Planet Survey of %1").arg(planetName);
			rpt = MailHeader(ship, title, "");

			places.append(worldId);

			double sensorRange = ship->SensorRange();
			if(sensorRange == 0)
			{
				rpt += QObject::tr("Sensors are not operational. No planet survey scan data is available.");
			}
			else
			{
				// Do the hard work of the sensor scan when needed.
				AuSensorBlipList sensorBlips;
				ship->PerformSensorConsoleScan(sensorBlips);

				AuSensorBlipList::const_iterator sbi = sensorBlips.Find(worldId);
				if(sbi != sensorBlips.end())
				{
					// First line of info is the name of the the planet.
					QStringList infoLines = (*sbi)->InfoLines(system);
					rpt += "<big><b>" + infoLines[0] + "</b></big><div style=\"margin-left: 10px;\">";
					for(QStringList::const_iterator ili = infoLines.cbegin()+1; ili != infoLines.cend(); ili++)
					{
						if((*ili).left(1) == " ")
							rpt += "&nbsp;" + *ili + "<br>";
						else if((*ili).left(12) == "Data Limited")
							rpt += "<span style=\"color: rgb(255, 255, 32);\"><b>" + *ili + "</b></span><br>";
						else if((*ili).indexOf("sphere") >= 0 || *ili == "Primary" || (*ili).left(9) == "Orbiting ")
							rpt += "<b>" + *ili + "</b><br>";
						else
							rpt += *ili + "<br>";
					}
					rpt += "</div>";
				}
				else
				{
					rpt += QObject::tr("Planet %1 is not showing up on sensors. How odd.").arg(planetName);
				}
			}
		}
		else
		{
			// Mission has brought us to a location with no planet to scan.
			planetName = tr("Planet");
			title = tr("Planet Survey of %1").arg(planetName);
			rpt = MailHeader(ship, title, "");

			rpt += QObject::tr("There is no planet here to scan with sensors.");
		}
	}
	else if(Order() == ORDER_SurveyScanSystem)
	{
		if(!system->IsEmptySystem())
		{
			if(system->IsScannedBy(ship->EmpireId()))
			{
				// Do it anyway in case it's out of sync.
				QString text = tr("Sir, this system has already been scanned by our empire. "
						"I notified command to verify their records. "
						"Check your star map; you may need fresh map info.");
				AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
						sector->Id(), galaxy, AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
				if(SayNoRepeat(msg, AuVisage::Closed))
				{
					// This will send a signal to the farm that updates everybody.
					system->ScannedBy(ship->EmpireId(), system->GenHab(), system->GenSeed());
					if(ship->MissionOrderReportToCommand())
					{
						// Survey scan report only sent if chain-of-command selected.
						title = tr("System Survey of %1 Canceled").arg(system->Name());
						rpt = MailHeader(ship, title, "");
						rpt += QObject::tr("%1 has already been scanned by our empire. I did not conduct a system survey scan.")
							.arg(system->Name());
					}
				}
			}
			else
			{
				double sensorRange = ship->SensorRange();
				if(sensorRange <= 0)
				{
					QString text = tr("Sir, sensors are not operational. No system survey scan data is available.");
					AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
							sector->Id(), galaxy, AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
					if(SayNoRepeat(msg, AuVisage::Perplexed))
					{
						if(ship->MissionOrderReportToCommand())
						{
							// Survey scan report only sent if chain-of-command selected.
							title = tr("System Survey of %1 Failed").arg(system->Name());
							rpt = MailHeader(ship, title, "");
							rpt += QObject::tr("Sensors are not operational. I was unable to conduct a system survey scan.");
						}
					}
				}
				else
				{
					// Dilly dally until I have had time to finish the report.
					DaAtomList planets;
					system->FindPlanetsRecursive(planets);
					int scantime = (planets.count() + 1) * (20 - Rank()*5);	// 20 sec per planet/star reduced 5 sec per body by rank
					QString scanRange;
					if(sensorRange > FeetPerOrbit()*15)
					{
						// System range could scan the entire system from one spot. Allow 30 sec to get to that spot.
						scanRange = tr("system");
						scantime += 30;
					}
					else if(sensorRange > FeetPerOrbit()/2)
					{
						// Planetary range could scan each world and its moons from the parent world's orbit.
						// Estimating 1 in 4 bodies is a planet, allow 30 seconds to travel to each planet.
						scanRange = tr("planetary");
						scantime += (planets.count()+3)/4 * 60;
					}
					else if(sensorRange > FeetPerMoonOrbit())
					{
						// Orbital range sensors could scan each body by orbiting it.
						// Allow 60 seconds to travel to each planet and moon.
						scanRange = tr("orbital");
						scantime += planets.count() * 60;
					}
					else
					{
						// Short range sensors might require landing on every planet.
						// Allow 120 seconds to land on each planet and moon.
						scanRange = tr("short");
						scantime += planets.count() * 120;
					}

					int secsRemaining = scantime - ship->MissionOrderStart().ElapsedSec();
					if(secsRemaining > 0 && !ship->CaptainId().IsNull())
					{
						// Check for agent aboard in command of the ship.
						const DaPerson* captain = dynamic_cast<const DaPerson*>(ship->FindIdRecursive(ship->CaptainId()));
						if(captain && captain->HasAgentPower())
							secsRemaining = 0;
					}
					if(secsRemaining > 0)
					{
						QString text;
						if(secsRemaining >= 120)
						{
							if(!SpokeRecently(60))
							{
								text = tr("Sir, %2 minutes until I finish surveying all %3 worlds of %1 using %4 range sensors.")
									.arg(system->Name())
									.arg((int)secsRemaining/60)
									.arg(planets.count())
									.arg(scanRange);
							}
						}
						else if(secsRemaining > 5)
						{
							if(!SpokeRecently(20))
							{
								text = tr("Sir, %2 seconds until I finish surveying all %3 worlds of %1 using %4 range sensors.")
									.arg(system->Name())
									.arg(secsRemaining)
									.arg(planets.count())
									.arg(scanRange);
							}
						}
						if(!text.isEmpty())
						{
							AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
									sector->Id(), galaxy, AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
							SayNoRepeat(msg, AuVisage::Closed);
						}
						StayIdle();
						return;
					}

					system->ScannedBy(ship->EmpireId(), system->GenHab(), system->GenSeed());
					IncreaseSkill(AuSkill::Sensor, 1);

					QString text = tr("Sir, system survey of %1 is complete. "
							"The data can be accessed using the star map; you may need fresh map info.")
						.arg(system->Name());
					AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(),
							sector->Id(), galaxy, AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
					if(SayNoRepeat(msg, AuVisage::Closed))
					{
						if(ship->MissionOrderReportToCommand())
						{
							// Survey scan report only sent if chain-of-command selected.
							title = tr("System Survey of %1 Complete").arg(system->Name());
							rpt = MailHeader(ship, title, "");
							rpt += QObject::tr("I have completed a thorough system survey scan of %1. "
									"The data was transmitted to headquarters for inclusion in the master star map archive.")
								.arg(system->Name());
						}
					}
				}
			}
		}
		else
		{
			// Mission has brought us to a location with no planet to scan.
			title = tr("System Survey Failure");
			rpt = MailHeader(ship, title, "");

			rpt += QObject::tr("There is no solar system here to scan with sensors.");
		}
	}
	else
	{
		if(!system->IsEmptySystem())
			title = tr("Tactical Scan in %1").arg(system->Name());
		else
			title = tr("Tactical Scan in %1").arg(sector->SectorNameCoord());
		rpt = MailHeader(ship, title, "");

		double sensorRange = ship->SensorRange();
		if(sensorRange == 0)
		{
			rpt += QObject::tr("Sensors are not operational. No tactical scan data is available.");
		}
		else
		{
			// Do the hard work of the sensor scan when needed.
			AuSensorBlipList sensorBlips;
			ship->PerformSensorConsoleScan(sensorBlips);

			double sensorRange2 = sensorRange * sensorRange;
			AuPoint shipSysLoc = ship->EnvLocation();
			multimap<int, QString> sensorContacts;	// Map so they are sorted by type.
			for(AuSensorBlipList::const_iterator sbi = sensorBlips.begin(); sbi != sensorBlips.end(); sbi++)
			{
				if(!(*sbi)->IsSensorConsoleBlip())
					continue;	// Skip blips that don't show up on sensor consoles, such as life forms.
				if((*sbi)->IsStarOrPlanet())
					continue;	// Skip stars and planets.
				if((*sbi)->IsRangeLimited() && (*sbi)->Range2(shipSysLoc) > sensorRange2)
					continue;	// Skip blips that are out of range.

				// NOTE: If the number of fields in this call changes, update the header line below.
				sensorContacts.insert(pair<int, QString>((*sbi)->Type(), (*sbi)->HtmlTableRow(system, shipSysLoc, 11)));
			}

			rpt += "<big><b>Sensor Contacts</b></big><div style=\"margin-left: 10px;\">";

			if(sensorContacts.empty())
				rpt += QObject::tr("Sensors are clear of tactical contacts.");
			else
			{
				// Format this data into a table.
				rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">"
					"<tbody>";

				// Put a header on the table.
				rpt += QString("<tr>"
						"<td></td>"
						"<td><b>Contact</b></td>"
						"<td></td>"
						"<td></td>"
						"<td></td>"
						"<td></td>"
						"<td></td>"
						"<td></td>"
						"<td></td>"
						"<td><b>Empire</b></td>"
						"<td><b>Range</b></td>"
						"</tr>");

				// Spit out the rows in order.
				for(multimap<int, QString>::const_iterator sci = sensorContacts.begin(); sci != sensorContacts.end(); sci++)
					rpt += (*sci).second;

				// Finalize the table.
				rpt += "</tbody>"
					"</table>";
			}

			rpt += "<br>";
			rpt += "</div>";
		}
	}

	if(!rpt.isEmpty())
	{
		// Send the sensor scan report by mail to everyone in the chain of command of the officer.
		AuIdList recipients;
		const DgData& gd = g_Au.LockDataForRead();
		if(ship->MissionOrderReportToCommand())
			gd.EmpireReceivesSensorScans(ship->EmpireId(), places, recipients);
		unsigned int idPlayer = gd.PlayerId();					// The local server must be "logged in" as someone to send mail.
		unsigned int confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();

		// List captain and human officers as recipients.
		bool anyHumans = false;
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			if((*ri).IsHumanOfficer())	// Also includes captain.
			{
				recipients.append((*ri).m_OccId);
				anyHumans = true;
			}
		ship->UnlockRoster();

		if(recipients.count())
			g_Au.SendBulkMail(idPlayer, confirmNum, ship->Id(), ship->Name(), recipients, AuMailMsg::MSG_ShipReport, title, rpt,
					systemId, system->Name(), worldId, worldName, galaxy, galaxyLoc);

		if(anyHumans)
		{
			// Announce the completion of the scan report on the crew channel.
			QString text;
			if(ship->MissionOrderReportToCommand())
				text = tr("Sir, %1 complete. I mailed the report to you and CC'd the chain-of-command.").arg(title);
			else if(recipients.count() > 1)
				text = tr("Sir, %1 complete. I mailed the report to you and CC'd the officers.").arg(title);
			else
				text = tr("Sir, %1 complete. I mailed the report to you.").arg(title);
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), sector->Id(),
					galaxy, AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			SayNoRepeat(msg, AuVisage::Closed);
		}
	}

	// We only get one shot at doing this.
	RequestOrder();	// Ask for my next order.
}

// Check to see if I need to run to the sick bay for surgery.
// Insures the ship I am aboard has a functioning surgery unit and medical kits in the hold.
// Returns true if an order is amended to get medical attention.
bool DaNpc::GetACheckup()
{
	// Different kinds of characters have different thresholds for pain.
	switch(Role())
	{
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_Worker:
		if(CurrentHitPoints() >= MaxHitPoints()/2)
			return false;										// I am not hurt enough to want medical attention.
		break;
	case ROLE_Citizen:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
		if(CurrentHitPoints() >= MaxHitPoints()*3/4)
			return false;										// I am not hurt enough to want medical attention.
		break;
	case ROLE_Troop:
		if(CurrentHitPoints() >= MaxHitPoints()/4)
			return false;										// I am not hurt enough to want medical attention.
		break;
	case ROLE_Officer:
		if(CurrentHitPoints() >= MaxHitPoints()*2/3)
			return false;										// I am not hurt enough to want medical attention.
		break;
	}
	return DaCitizen::GetACheckup();
}

// Reward this atom appropriately for killing a unit.
void DaNpc::UnitKilled(DaUnit* victim)
{
	// This Crew killed a unit.
	IncreaseSkill(AuSkill::Combat, victim->ExperiencePointsForKilling());

	// A crewman who is under the command of a person gives credit to the person for the kill. That person may be executing a story
	// or job that involves killing the victim.
	if(!AssignedShip().IsNull())
	{
		// I am assigned to a ship.
		DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			// Locate my assigned ship in the solar system.
			DaVessel* ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(AssignedShip()));
			if(ship)
			{
				// Found my assigned ship in the solar system.
				// Notify all people online in the solar system who are in the chain-of-command of my ship.
				DaAtomList people;
				system->FindPeopleAwakeRecursive(people);
				for(DaAtomList::iterator pi = people.begin(); pi != people.end(); pi++)
				{
					DaPerson* person = (DaPerson*)(*pi);
					if(!ship->InChainOfCommand(person))
						continue;
					victim->KilledByPerson(person);
				}
			}
		}
	}

	// Base class declares mutual enemies appropriately.
	DaCitizen::UnitKilled(victim);
}

void DaNpc::OccupyBerth(DaVessel* ship)
{
	AuRosterEntry::TYPE berthType = AuRosterEntry::Invalid;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Count:
		break;
	case ROLE_Passenger:
		berthType = AuRosterEntry::PassengerAI;
		break;
	case ROLE_Crew:
		berthType = AuRosterEntry::Crew;
		break;
	case ROLE_Troop:
		berthType = AuRosterEntry::Troop;
		break;
	case ROLE_Officer:
		berthType = AuRosterEntry::OfficerAI;
		break;
	}
	if(berthType == AuRosterEntry::Invalid)
		return;

	ship->OccupyBerth(berthType, Id(), Name());
	AssignToShip(ship->Id(), berthType);
	if(berthType == AuRosterEntry::OfficerAI && !ship->HasHumanOfficer())
	{
		TakeTheCon();											// Take the con.
		ship->SetCrewPosture(Posture());						// Set crew posture to match my current posture.
	}
	SetGarrisonBd(AuId());										// I am no longer associated with a city.
	SetModified();
}

void DaNpc::GoToWork()
{
	if(StoryInstance())
		return;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		{
			DaVessel* ship = FindParentVesselInEnv();
			if(ship)
			{
				// Choose a place to go that is ok for a passenger aboard a ship.
				AuPoint voidLoc;
				if(ship->ColonistVoidLocation(voidLoc))
				{
					AmendOrder(ORDER_GoToTarget, ship->Id(), voidLoc, 0);
					return;
				}
			}
		}
		break;
	case ROLE_Crew:
		if(IsInSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship && ship->Is(AssignedShip()))
			{
				// I am aboard my assigned spacecraft. Man a station.
				SetOrder(ORDER_BattleStation, ship->Id(), AuPoint(0,0));
				PoseVisage(AuVisage::Closed, AuVisage::Closed);

				AuPoint jammedDoor;
				if(ship->AnyDoorJammed(jammedDoor))
				{
					// Go close that jammed door.
					jammedDoor = ship->FloorLocInsideDoor(jammedDoor);
					AmendOrder(ORDER_CloseDoor, ship->Id(), jammedDoor, 0);			// Close the door when I get there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), jammedDoor, 0);	// First, go to the location of the door.
				}
			}
		}
		break;
	case ROLE_Troop:
		if(IsInSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship && ship->Is(AssignedShip()))
			{
				// I am aboard my assigned spacecraft. Man a station.
				SetOrder(ORDER_BattleStation, ship->Id(), AuPoint(0,0));
				PoseVisage(AuVisage::Closed, AuVisage::Closed);

				AuPoint jammedDoor;
				if(ship->AnyDoorJammed(jammedDoor))
				{
					// Go close that jammed door.
					jammedDoor = ship->FloorLocInsideDoor(jammedDoor);
					AmendOrder(ORDER_CloseDoor, ship->Id(), jammedDoor, 0);			// Close the door when I get there.
					AmendOrder(ORDER_GoToTargetExactly, ship->Id(), jammedDoor, 0);	// First, go to the location of the door.
				}
			}
		}
		else if(IsInAtmosphere())
		{
			// BattleStation order will figure out what to do and how to get there.
			SetOrder(ORDER_BattleStation, FindParentPlanetInEnv()->Id(), AuPoint(0,0));
			PoseVisage(AuVisage::Closed, AuVisage::Closed);
		}
		break;
	case ROLE_Officer:
		if(IsInSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship && ship->Is(AssignedShip()))
			{
				// I am aboard my assigned spacecraft. Man a station.
				SetOrder(ORDER_BattleStation, ship->Id(), AuPoint(0,0));
				PoseVisage(AuVisage::Closed, AuVisage::Closed);
			}
		}
		break;
	}
}

void DaNpc::KilledByUnit(const DaUnit* killer)
{
	if(!killer->EmpireId())
		return;							// Killer is probably a wild animal.
	if(!EmpireId())
		return;

	if(InArena() || InDesignPreview())
		return;

	if(EmpireId() == killer->EmpireId())
		return;
	// Victim is from a different empire as the killer.

	// Victim's empire logs this as a diplomatic incident.
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(!system)
		return;	// No parent solar system?

	GALAXY galaxy = system->Galaxy();
	const DaSector* sector = system->FindParentSector();
	const DaWorld* world = FindParentPlanetInEnv();
	QString sectorName = sector ? sector->Name() : "";
	QString systemName = system ? system->Name() : "";
	QString worldName = world ? world->Name() : "";

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Count:
		break;
	case ROLE_Worker:
		{
			AuDocWorkerKilled doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case ROLE_Passenger:
		{
			AuDocPassengerKilled doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case ROLE_Crew:
		{
			AuDocCrewKilled doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case ROLE_Troop:
		{
			AuDocTroopKilled doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case ROLE_Officer:
		{
			AuDocOfficerKilled doc(Name(), HomeWorldName(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	}
}

quint64 DaNpc::MemoryUsage() const
{
	quint64 mem = DaCitizen::MemoryUsage();
	mem += sizeof(m_Con);
	mem += sizeof(m_GarrisonBd);
	mem += sizeof(m_PortBd);
	mem += sizeof(m_Indestructible);
	mem += sizeof(m_Role);
	mem += sizeof(m_Say);
	mem += sizeof(m_Companion);
	return mem;
}

void DaNpc::FindCrewRecursive(const AuId& shipAssigned, DaAtomList& crewMembers) const
{
	//if(!IsNormal())
		//return;
	DaCitizen::FindCrewRecursive(shipAssigned, crewMembers);
	if(Role() == ROLE_Crew && AssignedShip() == shipAssigned)
		crewMembers.append((DaAtom*)this);	// Cast away constness.
}

const DaNpc* DaNpc::FindOfficerRecursive() const
{
	//if(!IsNormal())
		//return 0;
	if(IsNpcOfficer())
		return this;
	return DaCitizen::FindOfficerRecursive();
}

void DaNpc::FindOfficersRecursive(const AuId& shipAssigned, DaAtomList& officers) const
{
	//if(!IsNormal())
		//return;
	DaCitizen::FindOfficersRecursive(shipAssigned, officers);
	if(Role() == ROLE_Officer && AssignedShip() == shipAssigned)
		officers.append((DaAtom*)this);	// Cast away constness.
}

void DaNpc::FindPassengersRecursive(const AuId& shipAssigned, DaAtomList& passengers) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindPassengersRecursive(shipAssigned, passengers);
	if(Role() == ROLE_Passenger && AssignedShip() == shipAssigned)
		passengers.append((DaAtom*)this);	// Cast away constness.
}

void DaNpc::FindTroopsRecursive(const AuId& shipAssigned, DaAtomList& troops) const
{
	//if(!IsNormal())
		//return;
	DaSentient::FindTroopsRecursive(shipAssigned, troops);
	if(Role() == ROLE_Troop && (AssignedShip() == shipAssigned || GarrisonBd() == shipAssigned))
		troops.append((DaAtom*)this);	// Cast away constness.
}

void DaNpc::FindTroopsRecursive(quint32 empireId, DaAtomList& troops)
{
	//if(!IsNormal())
		//return;
	DaSentient::FindTroopsRecursive(empireId, troops);
	if(Role() == ROLE_Troop && EmpireId() == empireId)
		troops.append(this);
}

DaAtom::ATOM_TYPE DaNpc::AtomType() const
{
	return ATOM_Npc;
}

void DaNpc::ChooseHairStyle()
{
	AuHairStyle::STYLE style;
	AuHairStyle::SHAVE shave;
	if(Gender() == Male)
	{
		// Choose from among certain masculine hair and shave styles.
		AuHairStyle::STYLE styles[] =
		{
			AuHairStyle::Bald,							// All follicles removed.
			AuHairStyle::Buzz,							// Every follicle as a stub.
			AuHairStyle::Close,							// Hair patch only at every follicle.
			AuHairStyle::Short,
			AuHairStyle::Baltar,
			AuHairStyle::Mandinka,						// Mr. T
			AuHairStyle::Mohawk,
			AuHairStyle::Regulation,						// Parted on the right
			AuHairStyle::HighAndTight,
			AuHairStyle::Crop,
			AuHairStyle::Manager,
			AuHairStyle::Egghead,
			AuHairStyle::Spacely,
			AuHairStyle::Carrot,
			AuHairStyle::Crown,
			AuHairStyle::Flattop,
			AuHairStyle::Manchu,
			AuHairStyle::MohawkPony,
		};
		unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
		style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];

		AuHairStyle::SHAVE shaves[] =
		{
			AuHairStyle::Clean,							// All follicles removed.
			AuHairStyle::Stubble,						// Every follicle as a stub.
			AuHairStyle::CloseWhiskers,					// Hair patch only at every follicle.
			AuHairStyle::ShapedStubble,
			AuHairStyle::Hollywoodian,
			AuHairStyle::FuManchu,
			AuHairStyle::VanDyke,
			AuHairStyle::ExtendedVanDyke,
			AuHairStyle::Balbo,
			AuHairStyle::Klingon,
			AuHairStyle::ShortBoxedBeard,
			AuHairStyle::Goatee,
			AuHairStyle::ExtendedGoatee,
			AuHairStyle::ChinStrap,
			AuHairStyle::Zappa,
			AuHairStyle::SoulPatch,
			AuHairStyle::Winnfield,
			AuHairStyle::PetitGoatee,
			AuHairStyle::FranzJosef,
			AuHairStyle::Anchor,
			AuHairStyle::Napoleon,
			AuHairStyle::Copstash,
			AuHairStyle::Confucius,
			AuHairStyle::Boris,
			AuHairStyle::Toothbrush,
			AuHairStyle::ChinPuff,
			AuHairStyle::Stripe,
			AuHairStyle::RapStandard,
			AuHairStyle::Pencil,
			AuHairStyle::Handlebar,
			AuHairStyle::HandlebarAndChinPuff,
			AuHairStyle::HandlebarAndGoatee,
			AuHairStyle::PencilPatch,
		};
		unsigned int shavesCount = sizeof(shaves)/sizeof(AuHairStyle::SHAVE);
		shave = shaves[g_Au.Dice().Roll(1, shavesCount, -1)];
	}
	else if(Gender() == Female)
	{
		// Choose from among certain feminine hair and shave styles.
		AuHairStyle::STYLE styles[] =
		{
			AuHairStyle::Close,							// Hair patch only at every follicle.
			AuHairStyle::Short,
			AuHairStyle::Baltar,
			AuHairStyle::PonyBangs,
			AuHairStyle::PonyTail,
			AuHairStyle::Waterfall,
			AuHairStyle::Crop,
			AuHairStyle::Carrot,
			AuHairStyle::Bun,
			AuHairStyle::BunBangs,
			AuHairStyle::Bullet,
		};
		unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
		style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];

		shave = AuHairStyle::Clean;
	}
	else
	{
		// Choose from among certain hair and shave styles.
		AuHairStyle::STYLE styles[] =
		{
			AuHairStyle::Bald,							// All follicles removed.
			AuHairStyle::Buzz,							// Every follicle as a stub.
			AuHairStyle::Close,							// Hair patch only at every follicle.
			AuHairStyle::Short,
			AuHairStyle::Baltar,
			AuHairStyle::Mandinka,						// Mr. T
			AuHairStyle::Mohawk,
			AuHairStyle::PonyBangs,
			AuHairStyle::PonyTail,
			AuHairStyle::Regulation,						// Parted on the right
			AuHairStyle::Waterfall,
			AuHairStyle::HighAndTight,
			AuHairStyle::Crop,
			AuHairStyle::Manager,
			AuHairStyle::Egghead,
			AuHairStyle::Spacely,
			AuHairStyle::Carrot,
			AuHairStyle::Crown,
			AuHairStyle::Flattop,
			AuHairStyle::Bun,
			AuHairStyle::BunBangs,
			AuHairStyle::Bullet,
			AuHairStyle::Manchu,
			AuHairStyle::MohawkPony,
		};
		unsigned int stylesCount = sizeof(styles)/sizeof(AuHairStyle::STYLE);
		style = styles[g_Au.Dice().Roll(1, stylesCount, -1)];

		AuHairStyle::SHAVE shaves[] =
		{
			AuHairStyle::Clean,							// All follicles removed.
			AuHairStyle::Stubble,						// Every follicle as a stub.
			AuHairStyle::CloseWhiskers,					// Hair patch only at every follicle.
			AuHairStyle::ShapedStubble,
			AuHairStyle::Hollywoodian,
			AuHairStyle::FuManchu,
			AuHairStyle::VanDyke,
			AuHairStyle::ExtendedVanDyke,
			AuHairStyle::Balbo,
			AuHairStyle::Klingon,
			AuHairStyle::ShortBoxedBeard,
			AuHairStyle::Goatee,
			AuHairStyle::ExtendedGoatee,
			AuHairStyle::ChinStrap,
			AuHairStyle::Zappa,
			AuHairStyle::SoulPatch,
			AuHairStyle::Winnfield,
			AuHairStyle::PetitGoatee,
			AuHairStyle::FranzJosef,
			AuHairStyle::Anchor,
			AuHairStyle::Napoleon,
			AuHairStyle::Copstash,
			AuHairStyle::Confucius,
			AuHairStyle::Boris,
			AuHairStyle::Toothbrush,
			AuHairStyle::ChinPuff,
			AuHairStyle::Stripe,
			AuHairStyle::RapStandard,
			AuHairStyle::Pencil,
			AuHairStyle::Handlebar,
			AuHairStyle::HandlebarAndChinPuff,
			AuHairStyle::HandlebarAndGoatee,
			AuHairStyle::PencilPatch,
		};
		unsigned int shavesCount = sizeof(shaves)/sizeof(AuHairStyle::SHAVE);
		shave = shaves[g_Au.Dice().Roll(1, shavesCount, -1)];
	}

	SetHairStyle(style, shave);
}

COMMODITYID DaNpc::WornArmor() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Troop:
	case ROLE_Worker:
	case ROLE_Count:
		break;
	case ROLE_Passenger:
		// If I can breathe and I'm not naked underneath, don't show the armor.
		if(IsAirBreathable() && WornClothing() != CID_Invalid)
			return CID_Invalid;
		break;
	case ROLE_Crew:
	case ROLE_Officer:
		if(IsAirBreathable() && WornClothing() != CID_Invalid)
		{
			// Air is breathable and I'm not naked.
			if(IsInsideBd() || IsInsideEnclosure())
				return CID_Invalid;			// Armor not needed inside a building or enclosure.
			else if(IsInsideSpacecraft())
			{
				const DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->ApparentAlertCondition() == ConditionGreen)
					return CID_Invalid;		// Armor not needed inside a spacecraft, unless it is on red or yellow alert.
			}
		}
		break;
	}
	return DaCitizen::WornArmor();
}

COMMODITYID DaNpc::WornClothing() const
{
	return DaCitizen::WornClothing();
}

COMMODITYID DaNpc::WornHat() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Count:
		break;
	case ROLE_Worker:
	case ROLE_Passenger:
		// If I can breathe, don't show the helmet.
		if(IsAirBreathable())
			return CID_Invalid;
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		if(IsAirBreathable())
		{
			// Air is breathable.
			if(IsInsideBd() || IsInsideEnclosure())
				return CID_Invalid;			// Helmet not needed inside a building or enclosure.
			else if(IsInsideSpacecraft())
			{
				const DaVessel* ship = FindParentVesselInEnv();
				if(ship && ship->ApparentAlertCondition() != ConditionRed)
					return CID_Invalid;		// Helmet not needed inside a spacecraft, unless it is on red alert.
			}
		}
		break;
	}
	return DaCitizen::WornHat();
}

void DaNpc::IncreaseSkill(AuSkill::SKILL skill, quint32 xp)
{
	quint8 oldRank = Rank();
	DaCitizen::IncreaseSkill(skill, xp);
	if(!AssignedShip().IsNull() && oldRank != Rank())
	{
		// I am assigned to a ship and my rank changed; I should update my name in the ship's roster if possible.
		// It's not a big deal if I don't find my ship in the solar system since I'll be killed as a deserter anyway.
		DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			DaVessel* ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(AssignedShip()));
			if(ship)
			{
				// Remove me from the roster and add me again to update my name due to the rank change.
				AuRosterEntry::TYPE berthType = AssignedShipBerthType();
				ship->RemoveBerthOccupant(Id());
				ship->OccupyBerth(berthType, Id(), Name());
			}
		}
	}
}

quint32 DaNpc::JobExperiencePoints() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
	case ROLE_Count:
		break;
	case ROLE_Crew:
		return CrewExperiencePoints();
	case ROLE_Troop:
		return TroopExperiencePoints();
	case ROLE_Officer:
		return CommandExperiencePoints();
	}
	return 0;
}

bool DaNpc::IsMarried() const
{
	m_NpcLock.lockForRead();
	bool isSingle = m_SpouseName.isEmpty();
	m_NpcLock.unlock();
	return !isSingle;
}

void DaNpc::RxHailReply(const AuCommPacket* msg)
{
	// Do not talk when I am unconscious.
	if(IsBodyDyingOrDead())
		return;

	if(!AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
		return;

	// Plain old chat text has empire id 0.
	if(EmpireId() && msg->FromEmpireId() && PoliticalStance(EmpireId(), msg->FromEmpireId()) == POL_Enemy)
	{
		WakeUp();
		return;
	}

	if(msg->FromId() != Interlocutor())
		return;

	DaAtom* env = FindEnvironmentAtom();
	DaPerson* person = env ? dynamic_cast<DaPerson*>(env->FindIdRecursive(msg->FromId())) : 0;
	if(!person || !InConeOfAddress(*person))
		return;

	// Wake up from my nap, if I am sleeping.
	WakeUp();

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		// Sometimes I don't want to talk to them.
		{
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
			if(IndigenousTerseReply(msg, llworld))
				return;
			if(ContainsEuphemismForSex(msg->Text()))
			{
				// Person wants to have sex?
				if(GrowthStage() == Juvenile)
				{
					QString text = tr("That's some bad language %1 %2. Don't let my mom hear you talk like that.")
						.arg(person->MsMr()).arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsPregnant())
				{
					QString text;
					if(person->Is(FatherOfBaby()))
						text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
							.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
					else
						text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(person->IsPregnant())
				{
					QString text;
					if(Is(person->FatherOfBaby()))
						text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
					else
						text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(TryBecomingReceptiveToMating(*person))
				{
					QString text;
					if(msg->Text().indexOf(QChar('!')) >= 0)
						text = tr("Ooh? So demanding. Be forceful with me %1!").arg(person->Name());
					else if(msg->Text().indexOf(QChar('?')) >= 0)
						text = tr("You are cute %1! I hoped you would ask.").arg(person->Name());
					else if(msg->Text().indexOf(QChar('.')) >= 0)
						text = tr("Have your way with me %1.").arg(person->Name());
					else
						text = tr("That's a great idea! Come to me %1.").arg(person->Name());
					switch(Role())
					{
					case ROLE_IndigenousPerson:	// children won't pass the TryBecomingReceptiveToMating test.
						text = tr("lol %1").arg(text);
						break;
					case ROLE_IndigenousElder:
						text = tr("%1 Elder doesn't mean dead.").arg(text);
						break;
					case ROLE_IndigenousHealer:
						text = tr("%1 Let's see what you've got there.").arg(text);
						break;
					case ROLE_IndigenousStargazer:
						text = tr("Oh dear. %1 For science.").arg(text);
						break;
					case ROLE_Citizen:
					case ROLE_Count:
					case ROLE_Worker:
					case ROLE_Passenger:
					case ROLE_Crew:
					case ROLE_Troop:
					case ROLE_Officer:
						break;
					}
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					SetGeneralPose(AuPose::ReceptiveMate);
					SetInMotion();
					TickTock();
					return;
				}
			}

			SayIndigenous(*llworld, *person);
		}
		break;
	case ROLE_Worker:
		if(ContainsEuphemismForSex(msg->Text()) && IsFullGrown())
		{
			// Person wants to have sex?
			if(IsPregnant())
			{
				QString text;
				if(person->Is(FatherOfBaby()))
					text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
						.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
				else
					text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(person->IsPregnant())
			{
				QString text;
				if(Is(person->FatherOfBaby()))
					text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
				else
					text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(TryBecomingReceptiveToMating(*person))
			{
				QString text;
				if(msg->Text().indexOf(QChar('!')) >= 0)
					text = tr("So foreward %1. I usually get dinner and a movie first.").arg(person->TitleName());
				else if(msg->Text().indexOf(QChar('?')) >= 0)
					text = tr("You are pretty cute %1. Be gentle; I'm a virgin.").arg(person->TitleName());
				else if(msg->Text().indexOf(QChar('.')) >= 0)
				{
					switch(person->Gender())
					{
					case Female:
						text = tr("%1? I was hoping for Seven of Nine. I'll just close my eyes.")
							.arg(person->TitleName());
						break;
					case Male:
						text = tr("%1? I was hoping for Captain Picard. I'll just close my eyes.")
							.arg(person->TitleName());
						break;
					case Asexual:
						text = tr("That is illogical %1.").arg(person->TitleName());
						break;
					case Hermaphrodite:
						text = tr("%1? I was hoping for Jabba the Hutt. I'll just close my eyes.")
							.arg(person->TitleName());
						break;
					}
				}
				else
					text = tr("So %1, my friends call me Afternoon Delight. Wanna find out why?").arg(person->TitleName());
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				SetGeneralPose(AuPose::ReceptiveMate);
				SetInMotion();
				TickTock();
				return;
			}
		}

		// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
		SayJobOffer(*person);
		break;
	case ROLE_Passenger:
		if(ContainsEuphemismForSex(msg->Text()) && IsFullGrown())
		{
			// Person wants to have sex?
			if(IsPregnant())
			{
				QString text;
				if(person->Is(FatherOfBaby()))
					text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
						.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
				else
					text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(person->IsPregnant())
			{
				QString text;
				if(Is(person->FatherOfBaby()))
					text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
				else
					text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(TryBecomingReceptiveToMating(*person))
			{
				if(IsLockedPose())
				{
					// Not while on duty.
					QString text = tr("I can't right now %1, not while I am at this station. Maybe later, somewhere private.")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsMarried())
				{
					// Some are married.
					QString text = tr("I'm sorry %1 but I can't. My spouse and I made a vow to be true to each other.")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}

				QString text;
				if(msg->Text().indexOf(QChar('!')) >= 0)
					text = tr("Well, well %1! This trip is getting better already.")
						.arg(person->Name());
				else if(msg->Text().indexOf(QChar('?')) >= 0)
					text = tr("Tell you what %1; I'll show you mine if you show me yours.").arg(person->Name());
				else if(msg->Text().indexOf(QChar('.')) >= 0)
					text = tr("Are you kidding? I don't take these trips for the food. "
							"What are you waiting for %1?").arg(person->Name());
				else
					text = tr("Oh %1, I was hoping this would be one of those kind of cruises, where you meet someone special, "
							"share a romantic moment, and fuck.")
						.arg(person->Name());
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				SetGeneralPose(AuPose::ReceptiveMate);
				SetInMotion();
				TickTock();
				return;
			}
		}

		// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
		SayJobOffer(*person);
		break;
	case ROLE_Crew:
		if(ContainsEuphemismForSex(msg->Text()) && IsFullGrown())
		{
			// Person wants to have sex?
			if(IsPregnant())
			{
				QString text;
				if(person->Is(FatherOfBaby()))
					text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
						.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
				else
					text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(person->IsPregnant())
			{
				QString text;
				if(Is(person->FatherOfBaby()))
					text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
				else
					text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(TryBecomingReceptiveToMating(*person))
			{
				if(IsLockedPose())
				{
					// Not while on duty.
					QString text = tr("I really can't right now %1, not while I am at my station. Maybe later, somewhere private.")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsMarried())
				{
					// Some are married.
					QString text = tr("I am betrothed to another %1. It is my responsibility to keep my vow. Please understand.")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}

				QString text;
				if(msg->Text().indexOf(QChar('!')) >= 0)
					text = tr("Yes %2 %1! What are your orders? I will comply. lol")
						.arg(person->Name())
						.arg(person->MaamSir(false));
				else if(msg->Text().indexOf(QChar('?')) >= 0)
					text = tr("Getting lonely %1? I've got a cure for that.").arg(person->Name());
				else if(msg->Text().indexOf(QChar('.')) >= 0)
					text = tr("Space gets lonely %1. Take me away, even if its only for little awhile.").arg(person->Name());
				else
					text = tr("Don't talk like that unless you mean it, %1. I haven't taken shore leave in a long time.")
						.arg(person->Name());
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				SetGeneralPose(AuPose::ReceptiveMate);
				SetInMotion();
				TickTock();
				return;
			}
		}

		// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
		SayJobOffer(*person);
		break;
	case ROLE_Troop:
		if(ContainsEuphemismForSex(msg->Text()) && IsFullGrown())
		{
			// Person wants to have sex?
			if(IsPregnant())
			{
				QString text;
				if(person->Is(FatherOfBaby()))
					text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
						.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
				else
					text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(person->IsPregnant())
			{
				QString text;
				if(Is(person->FatherOfBaby()))
					text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
				else
					text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(TryBecomingReceptiveToMating(*person))
			{
				if(IsLockedPose())
				{
					// Not while on duty.
					QString text = tr("I really can't right now %1; I'm at my post. How about later, after my shift ends?")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsMarried())
				{
					// Some are married.
					QString text = tr("Sorry %1, I'm married. We don't cheat on each other.")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}

				QString text;
				if(msg->Text().indexOf(QChar('!')) >= 0)
					text = tr("Fuck yeah! Give me what you've got %1!")
						.arg(person->Name());
				else if(msg->Text().indexOf(QChar('?')) >= 0)
					text = tr("Is this a trick question %1? I'd fuck a rat right now; we been out here so long.")
						.arg(person->Name());
				else if(msg->Text().indexOf(QChar('.')) >= 0)
					text = tr("That's a great idea %1. Let's fuck!").arg(person->Name());
				else
					text = tr("Do you want to play with the box the baby came in, %1?").arg(person->Name());
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				SetGeneralPose(AuPose::ReceptiveMate);
				SetInMotion();
				TickTock();
				return;
			}
		}

		// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
		SayJobOffer(*person);
		break;
	case ROLE_Officer:
		if(ContainsEuphemismForSex(msg->Text()) && IsFullGrown())
		{
			// Person wants to have sex?
			if(IsPregnant())
			{
				QString text;
				if(person->Is(FatherOfBaby()))
					text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
						.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
				else
					text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(person->IsPregnant())
			{
				QString text;
				if(Is(person->FatherOfBaby()))
					text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
				else
					text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				return;
			}
			else if(TryBecomingReceptiveToMating(*person))
			{
				if(IsLockedPose())
				{
					// Not while on duty.
					QString text = tr("Not now %1; I am busy. Maybe I could come by your cabin later?")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsMarried())
				{
					// Some are married.
					QString text = tr("I can't %1. My spouse would kill me. Is this going to affect my performance report?")
						.arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}

				QString text;
				if(msg->Text().indexOf(QChar('!')) >= 0)
					text = tr("%2! Your wish is my command. lol")
						.arg(person->MaamSir(true));
				else if(msg->Text().indexOf(QChar('?')) >= 0)
					text = tr("Yes, even officers get lonely. Come with me %1.").arg(person->Name());
				else if(msg->Text().indexOf(QChar('.')) >= 0)
					text = tr("I know you want it! Give me what we both need %1.").arg(person->Name());
				else
					text = tr("Its been a long time since our last port-of-call, %1. That kind of talk will get you everywhere.")
						.arg(person->Name());
				AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Open, person->Id());
				SetGeneralPose(AuPose::ReceptiveMate);
				SetInMotion();
				TickTock();
				return;
			}
		}

		// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
		SayJobOffer(*person);
		break;
	}
}

void DaNpc::RxHailRequest(const AuCommPacket* msg)
{
	// Do not talk when I am unconscious.
	if(IsBodyDyingOrDead())
		return;

	if(!AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
		return;

	DaAtom* env = FindEnvironmentAtom();
	DaPerson* person = env ? dynamic_cast<DaPerson*>(env->FindIdRecursive(msg->FromId())) : 0;
	if(!person || !InConeOfAddress(*person))
		return;

	// Wake up from my nap, if I am sleeping.
	WakeUp();

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		{
			AuPose personPose = person->CurrentGeneralPose();
			if(CurrentGeneralPose().IsSittingOnGround())
			{
				// I am sitting. Do I get up?
				if(personPose != AuPose::SitOnGround && personPose != AuPose::SitOnGroundCreep &&
						personPose != AuPose::Prone && personPose != AuPose::ProneCreep &&
						personPose != AuPose::Crouch && personPose != AuPose::CrouchCreep)
					Stand();
			}
			else if(CurrentGeneralPose() == AuPose::Stand)
			{
				// I am standing. Do I sit?
				if(personPose == AuPose::SitOnGround && personPose == AuPose::SitOnGroundCreep &&
						personPose == AuPose::Prone && personPose == AuPose::ProneCreep &&
						personPose == AuPose::Crouch && personPose == AuPose::CrouchCreep)
					SetGeneralPose(AuPose::SitOnGround);
			}

			// Sometimes I don't want to talk to them.
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
			if(IndigenousTerseReply(msg, llworld))
				return;

			// Say hello, if this is the beginning of our conversation.
			if(msg->FromId() != Interlocutor())
			{
				// Reset my say counter.
				m_Say = 0;

				QString text;
				if(Role() == ROLE_IndigenousElder)
				{
					double b = person->Bounty();
					if(b > 0)
						text = tr("Greetings avatar %1. I hear there is a price on your head.").arg(msg->FromName());
					else
						text = tr("Greetings avatar %1.").arg(msg->FromName());
				}
				else if(Role() == ROLE_IndigenousStargazer)
					text = tr("Listen to me %1 %2.").arg(person->MsMr()).arg(msg->FromName());
				else if(GrowthStage() == Juvenile)
					text = tr("Hi %1 %2.").arg(person->MsMr()).arg(msg->FromName());
				else
					text = tr("Greetings %1.").arg(msg->FromName());

				AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Closed, msg->FromId());
			}

			if(ContainsEuphemismForSex(msg->Text()))
			{
				// Person wants to have sex right away?
				if(GrowthStage() == Juvenile)
				{
					QString text = tr("Watch your language %1 %2! If my mom hears you, she'll wash your mouth out with soap.")
						.arg(person->MsMr()).arg(person->Name());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(IsPregnant())
				{
					QString text;
					if(person->Is(FatherOfBaby()))
						text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
							.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
					else
						text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(person->IsPregnant())
				{
					QString text;
					if(Is(person->FatherOfBaby()))
						text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
					else
						text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(TryBecomingReceptiveToMating(*person))
				{
					QString text;
					if(msg->Text().indexOf(QChar('!')) >= 0)
						text = tr("I am your slave %1!").arg(person->Name());
					else if(msg->Text().indexOf(QChar('?')) >= 0)
						text = tr("%1, you seem pretty eager! Let's do it.").arg(person->Name());
					else if(msg->Text().indexOf(QChar('.')) >= 0)
						text = tr("I am for you %1.").arg(person->Name());
					else
						text = tr("Don't be bashful %1. Let's get to know each other.").arg(person->Name());
					switch(Role())
					{
					case ROLE_IndigenousPerson:	// children won't pass the TryBecomingReceptiveToMating test.
						text = tr("lol %1").arg(text);
						break;
					case ROLE_IndigenousElder:
						text = tr("%1 Elder doesn't mean fast; give me a moment.").arg(text);
						break;
					case ROLE_IndigenousHealer:
						text = tr("%1 Let's see what's got you in such a hurry.").arg(text);
						break;
					case ROLE_IndigenousStargazer:
						text = tr("Oh my! %1 For science.").arg(text);
						break;
					case ROLE_Citizen:
					case ROLE_Count:
					case ROLE_Worker:
					case ROLE_Passenger:
					case ROLE_Crew:
					case ROLE_Troop:
					case ROLE_Officer:
						break;
					}
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					SetGeneralPose(AuPose::ReceptiveMate);
					SetInMotion();
					TickTock();
					return;
				}
			}

			// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
			SayIndigenous(*llworld, *person);
		}
		break;
	case ROLE_Worker:
		{
			if(PoliticalStance(EmpireId(), msg->FromEmpireId()) == POL_Enemy)
			{
				QString text = tr("Please %1, leave me alone. I am not permitted to speak to you.").arg(msg->FromName());
				AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Frown, msg->FromId());
				return;
			}

			// Select a job offer for the person before I say anything.
			SelectJobOffer(*person);

			// Say hello, if this is the beginning of our conversation.
			if(msg->FromId() != Interlocutor())
			{
				QString text = tr("Hello %1").arg(msg->FromName());

				QDateTime dt = g_Au.CurrentUTC();
				if(dt.date().month() == 12 && (dt.date().day() == 24 || dt.date().day() == 25))
					text += tr(" Merry Christmas.");
				else if(dt.date().month() == 1 && dt.date().day() == 1)
					text += tr(" Happy New Year.");
				AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Closed, msg->FromId());

				if(g_Au.Dice().Roll(1,100) <= 40)
					SayWorkerOpinion(*person);	// Say something about my town.
			}

			if(ContainsEuphemismForSex(msg->Text()))
			{
				if(IsPregnant())
				{
					QString text;
					if(person->Is(FatherOfBaby()))
						text = tr("I am already pregnant with our baby, my love. The baby is due %1.")
							.arg(DueDate().UTC().toString("MMMM d yyyy hh:mm t"));
					else
						text = tr("Your advances are flattering but I am already pregnant. I am not interested.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(person->IsPregnant())
				{
					QString text;
					if(Is(person->FatherOfBaby()))
						text = tr("You are already pregnant my love. I wouldn't want to hurt our baby.");
					else
						text = tr("No. You are pregnant. I wouldn't want to hurt your baby.");
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					return;
				}
				else if(TryBecomingReceptiveToMating(*person))
				{
					// Person wants to have sex right away?
					QString text;
					if(msg->Text().indexOf(QChar('!')) >= 0)
						text = tr("So foreward %1. I usually get dinner and a movie first.").arg(person->TitleName());
					else if(msg->Text().indexOf(QChar('?')) >= 0)
						text = tr("You are pretty cute %1. Be gentle; I'm a virgin.").arg(person->TitleName());
					else if(msg->Text().indexOf(QChar('.')) >= 0)
					{
						switch(person->Gender())
						{
						case Female:
							text = tr("%1? I was hoping for Seven of Nine. I'll just close my eyes.")
								.arg(person->TitleName());
							break;
						case Male:
							text = tr("%1? I was hoping for Captain Picard. I'll just close my eyes.")
								.arg(person->TitleName());
							break;
						case Asexual:
							text = tr("That is illogical %1.").arg(person->TitleName());
							break;
						case Hermaphrodite:
							text = tr("%1? I was hoping for Jabba the Hutt. I'll just close my eyes.")
								.arg(person->TitleName());
							break;
						}
					}
					else
						text = tr("So %1, my friends call me Afternoon Delight. Wanna find out why?").arg(person->TitleName());
					AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(rmsg, AuVisage::Open, person->Id());
					SetGeneralPose(AuPose::ReceptiveMate);
					SetInMotion();
					TickTock();
					return;
				}
			}

			// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
			SayJobOffer(*person);
		}
		break;
	case ROLE_Passenger:
	case ROLE_Crew:
	case ROLE_Troop:
		break;
	case ROLE_Officer:
		{
			if(PoliticalStance(EmpireId(), msg->FromEmpireId()) == POL_Enemy)
				return;

			// Select a job offer for the person before I say anything.
			SelectJobOffer(*person);

			// Say hello, if this is the beginning of our conversation.
			if(msg->FromId() != Interlocutor())
			{
				QString text = tr("Hello %1.").arg(msg->FromName());

				QDateTime dt = g_Au.CurrentUTC();
				if(dt.date().month() == 12 && (dt.date().day() == 24 || dt.date().day() == 25))
					text += tr(" Merry Christmas.");
				else if(dt.date().month() == 1 && dt.date().day() == 1)
					text += tr(" Happy New Year.");

				if(!InArena() && !InDesignPreview())
				{
					// Not in arena so tell them about my homeworld.
					if(!m_WorldName.isEmpty() && !m_SystemName.isEmpty() && !m_SectorName.isEmpty())
					{
						AuId sectorId = FindParentSector()->Id();
						AuId systemId = FindParentSolarSystem()->Id();
						AuId worldId;
						const DaWorld* world = FindParentPlanetInEnv();
						if(world)
							worldId = world->Id();

						if(worldId == m_WorldId)
							text += tr(" %1 is my home world. It feels good to be here.")
								.arg(m_WorldName);
						else if(systemId == m_SystemId)
							text += tr(" I am from %1, in this system.")
								.arg(m_WorldName);
						else if(sectorId == m_SectorId)
							text += tr(" I am from %1, in the %2 system. It's in this sector.")
								.arg(m_WorldName)
								.arg(m_SystemName);
						else if(Galaxy() == HomeGalaxy())
							text += tr(" I am from %1, in the %2 system, %3. It's in this galaxy.")
								.arg(m_WorldName)
								.arg(m_SystemName)
								.arg(m_SectorName);
						else
							text += tr(" I am from %1, in the %2 system, %3. It's in a galaxy far far away called %4.")
								.arg(m_WorldName)
								.arg(m_SystemName)
								.arg(m_SectorName)
								.arg(HomeGalaxyName());
					}
				}

				AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, AuVisage::Closed, msg->FromId());

				// Finish with an opportunity to keep the conversation going.
				SayPleasantry(*person);
			}
			else
			{
				// Do not offer the job the first time because I already said enough by chatting about my home world.
				// Tell the person of the job offer I have for them, if any. Otherwise, give them a pleasantry.
				SayJobOffer(*person);
			}
		}
		break;
	}
}

void DaNpc::SetCompanion(const AuId& companion)
{
	m_NpcLock.lockForWrite();
	m_Companion = companion;
	m_NpcLock.unlock();
	SetModified();
}

bool DaNpc::HasTheCon() const
{
	return m_Con;
}

void DaNpc::ReleaseTheCon()
{
	if(!m_Con)
		return;
	m_Con = false;
	SetModified();
}

void DaNpc::TakeTheCon()
{
	if(m_Con)
		return;
	m_Con = true;
	SetModified();
}

void DaNpc::SetSpouse(const QString& name)
{
	m_NpcLock.lockForWrite();
	m_SpouseName = name;
	m_SpouseChange.Set();
	m_NpcLock.unlock();
	SetModified();
}

void DaNpc::SetHomeWorld(const DaWorld* homeWorld)
{
	AuId worldId;
	QString worldName;
	BODY_TYPE worldBodyType = BODY_TYPE_Count;
	AuId systemId;
	QString systemName;
	AuId sectorId;
	QString sectorName;
	GALAXY galaxy = GALAXY_Count;
	if(homeWorld)
	{
		// Home world
		worldId = homeWorld->Id();
		worldName = homeWorld->Name();
		worldBodyType = homeWorld->BodyType();
		const DaSolarSystem* system = homeWorld->FindParentSolarSystem();
		if(system)
		{
			// Home solar system.
			systemId = system->Id();
			systemName = system->Name();
			const DaSector* sector = system->FindParentSector();
			if(sector)
			{
				// Home sector and galaxy.
				sectorId = sector->Id();
				sectorName = sector->Name();
				galaxy = sector->Galaxy();
			}
		}
	}

	SetHomeWorld(worldId, worldName, worldBodyType, systemId, systemName, sectorId, sectorName, galaxy);
}

void DaNpc::UpgradeEquipmentFromBd()
{
	if(StoryInstance())
		return;			// Story characters have specific inventories.
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_Worker:
	case ROLE_Troop:
	case ROLE_Passenger:
	case ROLE_Crew:
	case ROLE_Officer:
		if(g_Au.m_EnableTroopResupply && IsAtBd())
		{
			// Citizens at a building will try to upgrade their equipment once per hour from the local supplies. Otherwise they
			// stand around naked forever.
			DaWorld* planet = (DaWorld*)FindParentPlanetInEnv();
			if(planet)
			{
				DaBd* bd = planet->FindBd(BdId());
				if(bd && bd->EmpireId() == EmpireId())
				{
					AuItemHash items, itemsTaken;
					bd->CommunityInventoryAvailable(items, EmpireId());
					InitGear(items, itemsTaken, true);
					bd->CommunityRemoveInventory(itemsTaken);
				}
			}
		}
		break;
	case ROLE_Count:
		break;
	}
}

void DaNpc::MinuteTock()
{
	if(StoryInstance())
		return;
	switch(Role())
	{
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		if(HasCompanion())
			break;
		if(IsInAnyScene())
			break;
		if(!IsInAtmosphere())
			Kill();
		else if(!BdId().IsNull())
			Kill();
		else
		{
			const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
			if(!llworld || !llworld->Is(HomeWorldId()))
				Kill();
		}
		break;
	case ROLE_Worker:
		if(HasCompanion())
			break;
		if(IsInAnyScene())
			break;
		// Kill workers when they aren't in any scene, unless they are pregnant.
		if(!IsPregnant())
			Kill();
		break;
	case ROLE_Passenger:
		if(IsInsideSpacecraft() && AssignedShip().IsNull())
		{
			// I am aboard a spacecraft and I am not assigned to a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship && ship->CanAcceptBerthType(AuRosterEntry::PassengerAI) && ship->PassengerBerthsAvailable())
			{
				// The ship can accept a passenger berth request from me.
				// There is an empty passenger berth so take it.
				OccupyBerth(ship);
			}
		}

		if(HasCompanion())
			break;
		if(IsInAnyScene())
			break;

		if(!IsMounted())
		{
			// Kill castaways.
			bool isCastaway = false;
			if(!AssignedShip().IsNull())
			{
				// I am assigned to a ship; try to find my ship.
				DaVessel* ship = FindParentVesselInEnv();
				if(!ship || !ship->Is(AssignedShip()))
				{
					// I am not aboard my assigned ship.
					DaWorld* planet = FindParentPlanetInEnv();
					if(!planet || !planet->FindIdRecursive(AssignedShip()))
						isCastaway = true;	// My ship is not at this planet.
				}
			}
			else if(!IsAboardVehicle())
			{
				// I am not assigned to a ship and I am not aboard a vehicle.
				isCastaway = true;
			}
			if(isCastaway)
			{
				// I am a castaway.
				if(!BdId().IsNull())
				{
					// Join the local population.
					DaWorld* planet = FindParentPlanetInEnv();
					if(planet)
					{
						DaAtom* atom = planet->FindSubatomId(BdId());
						DaBd* bd = dynamic_cast<DaBd*>(atom);
						if(bd)
						{
							if(AssignedShip().IsNull())
								bd->CommunityIncreasePopulation(this, tr("Homeless drifter"));
							else
								bd->CommunityIncreasePopulation(this, tr("Castaway from spacecraft"));
						}
					}
				}

				// This atom must be killed.
				ReleaseBerth();
				Kill();
				return;
			}
		}
		break;
	case ROLE_Crew:
		if(IsInsideSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship)
			{
				if(AssignedShip().IsNull())
				{
					// I am not assigned to a spacecraft.
					if(PoliticalStanceMakeContact(ship) == POL_Enemy ||
							ship->PoliticalStanceMakeContact(this) == POL_Enemy)
					{
						// Stop manning a battle station aboard a ship that has become an enemy.
						if(PrimaryOrder() == ORDER_BattleStation)
							RequestOrder();
					}
					else if(ship->CanAcceptBerthType(AuRosterEntry::Crew) && ship->CrewBerthsAvailable())
					{
						// The ship can accept a crew berth request from me.
						// There is an empty Crew berth so take it.
						OccupyBerth(ship);
					}
				}
				else if(ship->Is(AssignedShip()))
				{
					// I am aboard my assigned ship.
					if(PoliticalStanceMakeContact(ship) == POL_Enemy)
					{
						// The empire's stance toward the owner of the ship has changed to enemy so release my berth.
						ReleaseBerth(ship);
						if(IsLockedPose())
							Stand();							// Stop manning a console.
						SetModified();
					}
					else
					{
						// When there is an AI capt or no human officers, man a station if I am not doing anything else.
						if(PrimaryOrder() == ORDER_Stay && (!ship->HasHumanOfficer() || ship->IsCommandedByAI()))
							GoToWork();						// Find a station to man.
					}
				}
			}
		}

		if(HasCompanion())
			break;
		if(IsInAnyScene())
			break;

		if(!IsMounted())
		{
			// Kill deserters.
			bool isDeserter = false;
			DaVessel* ship = FindParentVesselInEnv();
			if(!AssignedShip().IsNull())
			{
				// I am assigned to a ship; try to find my ship.
				if(!ship || !ship->Is(AssignedShip()))
				{
					// I am not aboard my assigned ship.
					DaAtom* system = FindEnvironmentAtom();
					if(!system)
						isDeserter = true;						// My solar system is gone?
					else
					{
						// Locate my ship in this solar system.
						ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(AssignedShip()));
						if(!ship)
							isDeserter = true;					// My ship is not in the solar system.
						else if(PoliticalStanceMakeContact(ship) == POL_Enemy)
							isDeserter = true;					// My ship has become an enemy to my empire.
					}
				}
			}
			else
			{
				// I am not assigned to a ship.
				if(ship)
				{
					// I am aboard a ship. If it was just captured, it will be my enemy. Stick around to defend/capture the ship.
					if(PoliticalStance(EmpireId(), ship->EmpireId()) != POL_Enemy)
						isDeserter = true;
				}
				else
					isDeserter = true;
			}
			if(isDeserter)
			{
				// I am a deserter.
				if(!BdId().IsNull())
				{
					// Join the local population.
					DaWorld* planet = FindParentPlanetInEnv();
					if(planet)
					{
						DaAtom* atom = planet->FindSubatomId(BdId());
						DaBd* bd = dynamic_cast<DaBd*>(atom);
						if(bd)
							bd->CommunityIncreaseCrew(this, tr("Deserted crew"));
					}
				}

				// This atom must be killed.
				ReleaseBerth();
				Kill();
				return;
			}
		}
		break;
	case ROLE_Troop:
		// People in cities notice the presense of enemy troops. This causes workers to stop spawning which relieves a genocide
		// exploit. They are safer if they stay indoors.
		if(!BdId().IsNull() && InCityEmpire() != EmpireId() && !IsInSpacecraft() && !IsInVehicle())
		{
			// This unit is in a city and not inside a spacecraft or vehicle.
			DaWorld* planet = FindParentPlanetInEnv();
			if(planet)
			{
				DaBd* bd = planet->FindBd(BdId());
				if(bd)
				{
					if(bd->Population() || bd->Workers())
					{
						// Buildings with people might see something.
						if(bd->PoliticalStanceMakeContact(this) == POL_Enemy)
						{
							QString empireName;
							const DgData& gd = g_Au.LockDataForRead();
							gd.EmpireName(EmpireId(), empireName);
							g_Au.UnlockData();
							bd->LogEvent(AuEventLogEntry::EnemyTroopSpotted, empireName);
						}
					}
				}
			}
		}

		if(IsInsideSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship)
			{
				if(AssignedShip().IsNull())
				{
					// I am not assigned to a spacecraft.
					if(ship->EmpireId() != EmpireId())
					{
						// Stop manning a battle station aboard a ship that has become an enemy.
						if(PrimaryOrder() == ORDER_BattleStation)
							RequestOrder();
					}
					else if(ship->CanAcceptBerthType(AuRosterEntry::Troop) && ship->TroopBerthsAvailable())
					{
						// The ship can accept a troop berth request from me.
						// There is an empty troop berth so take it.
						OccupyBerth(ship);
					}
				}
				else if(ship->Is(AssignedShip()))
				{
					// I am aboard my assigned ship.
					if(ship->EmpireId() != EmpireId())
					{
						// The ship's allegiance has changed so release my berth.
						ReleaseBerth(ship);
						if(IsLockedPose())
							Stand();							// Stop manning a console.
						SetModified();
					}
					else
					{
						// When there is an AI capt or no human officers, man a station if I am not doing anything else.
						if(PrimaryOrder() == ORDER_Stay && (!ship->HasHumanOfficer() || ship->IsCommandedByAI()))
							GoToWork();
					}
				}
			}
		}
		else if(IsInAtmosphere())
		{
			// I am within the atmosphere range of a planet.
			if(AssignedShip().IsNull() && PrimaryOrder() == ORDER_Stay)
			{
				const DaWorld* world = dynamic_cast<const DaWorld*>(Parent());
				if(world)
				{
					// I am immediately subordinate to a world, not a ship or vehicle.
					GoToWork();	// Primary order is a stay order. Go to battle stations if I am in a base of my empire.
				}
			}
		}

		if(HasCompanion())
			break;
		if(IsInAnyScene())
			break;

		if(!IsMounted())
		{
			// Kill excess troop atoms.
			bool killMe = false;
			DaVessel* ship = FindParentVesselInEnv();
			if(!AssignedShip().IsNull())
			{
				// I am assigned to a ship; try to find my ship.
				if(!ship || !ship->Is(AssignedShip()))
				{
					// I am not aboard my assigned ship.
					DaAtom* system = FindEnvironmentAtom();
					if(!system)
						killMe = true;						// My solar system is gone?
					else
					{
						// Locate my ship in this solar system.
						ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(AssignedShip()));
						if(!ship)
							killMe = true;					// My ship is not in the solar system.
						else if(ship->EmpireId() != EmpireId())
							killMe = true;					// My ship no longer belongs to my empire.
					}
				}
			}
			else if(ship)
			{
				// I am aboard a ship but I am not assigned to a ship. If it was just captured, it will be my enemy. Stick
				// around to defend/capture the ship.
				// This also prevents an exploit where troops are loaded and then kicked from their berths so more troops can be
				// loaded.
				if(PoliticalStance(EmpireId(), ship->EmpireId()) != POL_Enemy)
					killMe = true;
			}
			else
			{
				// I am not assigned to a ship and I am not in any scene and I am not aboard a spacecraft.
				if(BdId() != GarrisonBd())
				{
					// I am not in my assigned garrison building or I am not in any building at all.
					if(GarrisonBd().IsNull())
					{
						qWarning() << "warning: DaNpc::Tock" << *this << "died abandoned";
						killMe = true;		// I have no garrison building. How did I get here.
					}
					else
					{
						DaWorldLL* world = FindParentWorldInEnv();
						if(!world || !world->FindSubatomId(GarrisonBd()))
							killMe = true;	// I am not at the world of my garrison bd.
					}
				}
			}

			if(killMe)
			{
				// I am a deserter.
				ExitConsole();	// In case I am at a console, to decrement console count cache.
				ReleaseBerth();

				// This atom must be killed.
				Kill();
				return;
			}
		}
		break;
	case ROLE_Officer:
		if(IsInsideSpacecraft())
		{
			// I am aboard a spacecraft.
			DaVessel* ship = FindParentVesselInEnv();
			if(ship)
			{
				if(AssignedShip().IsNull())
				{
					// I am not assigned to a spacecraft.
					if(ship->EmpireId() != EmpireId())
                    {
                        // Stop manning a battle station aboard a ship that has become an enemy.
                        if(PrimaryOrder() == ORDER_BattleStation)
                            RequestOrder();
                    }
                    else if(ship->OfficerBerthsAvailable())
                        OccupyBerth(ship);  // There is an empty Officer berth so take it.
				}
				else if(ship->Is(AssignedShip()))
				{
					// I am aboard my assigned ship.
					// Calculate time interval for daily report just like city report.
					// Pirates don't send log reports unless they have a human captain.
					m_LogLock.lockForWrite();
					int realSecs = m_LogTime.ElapsedSec();
					if(realSecs < 0 || (IsPirate() && ship->IsPirateWithFreeWill()) || m_LogTime.IsNull())
					{
						m_LogTime.Set();
						m_LogLock.unlock();
					}
					else
					{
						m_LogLock.unlock();

						const float realSecsPerUDay = (float)RealDaysPerUYear() * (float)SecondsPerDay() / 365.f;
						float uDays = realSecs / realSecsPerUDay;	// Days since last meal.
						float uReports = uDays * 4;					// Four times per day.
						if(uReports >= 1)
						{
							if(Is(ship->AIFirstOfficer()))
								ShipLog(ship);					// Send log report, which includes my location and mission.
							else
							{
								ContactLog();					// Notify where I am.

								// ContactLog may result in my suicide due to abandonment.
								if(IsBodyDyingOrDead())
									return;
							}
						}
					}

					// I am aboard my assigned ship.
					if(ship->EmpireId() != EmpireId())
					{
						// The empire's stance toward the owner of the ship has changed to enemy so release my berth.
						ReleaseBerth(ship);
						if(IsLockedPose())
							Stand();							// Stop manning a console.
						SetModified();
					}
					else
					{
						// When there is no AI capt and no human officers, take command.
						if(!ship->IsCommandedByAI() && !ship->HasHumanOfficer() && !ship->InArena())
						{
							// Take command.
							TakeTheCon();
							ship->SetCrewPosture(Posture());			// Set crew posture to match mine.
						}

						if(HasTheCon())
						{
							// Manage the alert condition. It triggers upward automatically. Lower it when appropriate.
							ALERT_CONDITION alert = ship->AlertCondition();	// Alert based on ship conditions.
							if(alert < ship->ApparentAlertCondition())
								ship->SetAlertCondition(alert);			// Reduce alert condition.

							// Man a station if I am not doing anything else.
							if(PrimaryOrder() == ORDER_Stay)
								GoToWork();
						}

						DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ship->FindEnvironmentAtom());
						if(system)
						{
							if(IsPirate() && !ship->IsSpaceStation())
							{
								// Notify the solar system of the presense of pirates to prevent more encounters while I am here.
								system->SetLastPirateShip();
							}

							if(!IsPirate() || !ship->IsPirateWithFreeWill())
							{
								// Mark the solar system as visited so it does not decay. Pirates do not prevent decay.
								// This is particularly important if I now decide to scuttle the ship. This gives the owner up to a
								// week to retrieve their private ship before the system decays.
								system->SetLastOfficerVisited();
							}
						}
					}
				}
				else
				{
					// I am aboard a spacecraft that is not my assigned ship.
					// Notify chain-of-command of my location periodically.
					// Pirates don't send log reports. I'm a pirate if my homeworld id is null.
					m_LogLock.lockForWrite();
					int realSecs = m_LogTime.ElapsedSec();
					if(realSecs < 0 || IsPirate() || m_LogTime.IsNull())
					{
						m_LogTime.Set();
						m_LogLock.unlock();
					}
					else
					{
						m_LogLock.unlock();

						const float realSecsPerUDay = (float)RealDaysPerUYear() * (float)SecondsPerDay() / 365.f;
						float uDays = realSecs / realSecsPerUDay;	// Days since last meal.
						float uReports = uDays * 4;					// Four times per day.
						if(uReports >= 1)
						{
							ContactLog();

							// ContactLog may result in my suicide due to abandonment.
							if(IsBodyDyingOrDead())
								return;
						}
					}
				}
			}
		}
		else
		{
			// Not aboard ship so send location info periodically.
			// Calculate time interval for daily report just like city report.
			// Pirates don't send log reports. I'm a pirate if my homeworld id is null.
			m_LogLock.lockForWrite();
			int realSecs = m_LogTime.ElapsedSec();
			if(realSecs < 0 || IsPirate() || m_LogTime.IsNull())
			{
				m_LogTime.Set();
				m_LogLock.unlock();
			}
			else
			{
				m_LogLock.unlock();

				const float realSecsPerUDay = (float)RealDaysPerUYear() * (float)SecondsPerDay() / 365.f;
				float uDays = realSecs / realSecsPerUDay;	// Days since last meal.
				float uReports = uDays * 4;					// Four times per day.
				if(uReports >= 1)
				{
					ContactLog();

					// ContactLog may result in my suicide due to abandonment.
					if(IsBodyDyingOrDead())
						return;
				}
			}
		}

		if(!IsMounted() && !IsInAnyScene())
		{
			// Kill deserters.
			bool isDeserter = false;
			DaVessel* ship = FindParentVesselInEnv();
			if(!AssignedShip().IsNull())
			{
				// I am assigned to a ship; try to find my ship.
				if(!ship || !ship->Is(AssignedShip()))
				{
					// I am not aboard my assigned ship.
					DaAtom* system = FindEnvironmentAtom();
					if(!system)
						isDeserter = true;						// My solar system is gone?
					else
					{
						// Locate my ship in this solar system.
						ship = dynamic_cast<DaVessel*>(system->FindIdRecursive(AssignedShip()));
						if(!ship)
							isDeserter = true;					// My ship is not in the solar system.
						else if(ship->EmpireId() != EmpireId())
							isDeserter = true;					// My ship no longer belongs to my empire.
					}
				}
			}
			else
			{
				// I am not assigned to a ship.
				if(ship)
				{
					// I am aboard a ship. If it was just captured, it will be my enemy. Stick around to defend/capture the ship.
					if(PoliticalStance(EmpireId(), ship->EmpireId()) != POL_Enemy)
						isDeserter = true;
				}
				else
					isDeserter = true;
			}
			if(isDeserter)
			{
				// I am a deserter.
				if(!BdId().IsNull())
				{
					// Join the local population.
					DaWorld* planet = FindParentPlanetInEnv();
					if(planet)
					{
						DaAtom* atom = planet->FindSubatomId(BdId());
						DaBd* bd = dynamic_cast<DaBd*>(atom);
						if(bd)
							bd->CommunityIncreaseCrew(this, tr("Deserted command"));
					}
				}

				// This atom must be killed.
				ReleaseBerth();
				Kill();
				return;
			}
		}
		break;
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	}
}

void DaNpc::ShipLog(DaVessel* ship)
{
	// Do not send ship's log while in arena.
	if(InArena() || InDesignPreview())
		return;

	//qDebug() << "DaNpc::ShipLog" << Id() << Name() << "aboard" << ship->Id() << ship->Name() << "when" << now;
	QString title = tr("Captain's Log");
	QString rpt = MailHeader(ship, title, tr("Commander,"));

	AuMailMsg::MSGTYPE rptMessageType = AuMailMsg::MSG_ShipLog;	// Types are defined in aumailmsg.h

	// Check for unexplored destination.
	if(ship->IsOnAMission() && !DecideWhetherToTravel(ship))
		rpt += tr("<p>Our mission takes us into unexplored space. "
				"I ordered the crew to hold our position until you are present.");

	// Comments
	if(m_HomeSickness > d_HomeSickDeath)
	{
		if(ship->CaptainsBerthOccupied() || ship->OfficerBerthsOccupied() > 1)
		{
			// There is a human captain or at least one other officer. Just commit suicide. Don't scuttle the ship.
			qDebug() << "DaNpc::ShipLog" << Id() << Name() << "suicide, home sick" << m_HomeSickness;
			AuCombatAttacker attacker(tr("Suicide due to home sickness"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_InternalInjuries;
			weapon.m_w1.m_Effect = WE_Pierce;

			AuCombatResult result;
			result.m_Damage1 = CurrentHitPoints() + g_Au.Dice().Roll(1,-DpBody::DeadHitPoints()-5,-1);

			CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
			return;
		}
		else
		{
			// Configure the ship to self-destruct.
			qDebug() << "DaNpc::ShipLog" << Id() << Name() << "scuttling" << *ship << "home sick" << m_HomeSickness;
			ship->MissionScuttle(3 * 60);
			rpt += tr("<p>What have you done!<p>My home world is gone. "
					"Family, friends, all gone, I have no reason to live. "
					"You fools have lost it; that beautiful gem.<p>"
					"The crew feels as I do. "
					"We are returning to the green hills of home. "
					"After a final meal with the crew, "
					"I will initiate the self destruct sequence to scuttle the ship.<p>"
					"Forgive me.");
		}
		rptMessageType = AuMailMsg::MSG_ShipLogDistress;	// Types are defined in aumailmsg.h
	}

	// Where am I from?
	if(m_HomeSickness > 1)
		rpt += tr("<p>I was deployed from %1 in %2, %3. "
				"I miss my home; it's been %4 days since I last heard from my family.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()))
			.arg(m_HomeSickness);
	else
		rpt += tr("<p>I was deployed from %1 in %2, %3.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()));

	// Closing
	rpt += tr("<p>"
		"Yours in service,<br>"
		"<i>%1</i><p>").arg(Name());

	// Event Log
	m_LogLock.lockForWrite();
	QString eventRpt = m_Log.ReportHtml("EVENT LOG", m_LogTime);
	if(!eventRpt.isEmpty())
	{
		rpt += eventRpt;
		if(rptMessageType != AuMailMsg::MSG_ShipLogDistress && m_Log.ContainsAlert(m_LogTime))
			rptMessageType = AuMailMsg::MSG_ShipLogAlert;		// Types are defined in aumailmsg.h
	}
	m_LogTime.Set();
	m_LogLock.unlock();

	// Damage Report
	rpt += ship->DamageReportHtml();

	// Account
	rpt += "<b>ACCOUNT</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Ship's Fund<br>")
		.arg(g_Au.CommodityBitmapName(CID_Money))
		.arg((int)ship->AccountBalance())
		.arg(g_Au.CurrencySymbol());
	if(!ship->IsPrivatelyOwned())
	{
		rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Fleet Tax Collected<br>")
			.arg(g_Au.CommodityBitmapName(CID_Money))
			.arg((int)ship->AccountBalanceSubsidyTax())
			.arg(g_Au.CurrencySymbol());
	}
	rpt += "</div>";

	// Fuel
	rpt += "<b>FUEL</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Hydrogen %2 Q%3 in %4 Fuel Cells<br>")
		.arg(g_Au.CommodityBitmapName(CID_Hydrogen))
		.arg((int)ship->FuelQuantity())
		.arg(ship->FuelQuality())
		.arg(ship->FuelCapacity());
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Electricity %2 in %3 Capacitor<br>")
		.arg(g_Au.CommodityBitmapName(CID_Electricity))
		.arg((int)ship->CapacitorLevel())
		.arg((int)ship->CapacitorMax());
	if(ship->FuelQuantity() < 1 && ship->CapacitorPercent() < 1)
		rptMessageType = AuMailMsg::MSG_ShipLogDistress;	// Types are defined in aumailmsg.h
	rpt += "</div>";

	// Cargo in Hold
	if(ship->CargoCapacityVolume())
	{
		rpt += tr("<b>CARGO</b><div style=\"margin-left: 10px;\">");
		rpt += tr("Hold Space %1<br>").arg(ship->CargoCapacityVolume());

		// Make a map of the inventory table contents so it will be alphabetized.
		QString qual;
		QMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
		QMap<QString, QString> bitmap;		// Key is commodity name.
		const AuItemHash& sinv = ship->LockCargoForRead();
		for(AuItemHash::const_iterator it = sinv.begin(); it != sinv.end(); it++)
		{
			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = inven.find(commod);
			if(iit == inven.end())
				iit = inven.insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != inven.end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId())
					{
						// Item was created in an empire different from mine.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, tr("%1 Q%2 Made in %3")
								.arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, tr("%1 Q%2")
								.arg((*it).second->Count())
								.arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, tr("<i>%1 Q%2</i>")
							.arg((*it).second->Count())
							.arg((*it).second->Quality()));
				}
			}
		}
		ship->UnlockCargo();

		// Create a table of commodities.
		if(!inven.isEmpty())
		{
			rpt += tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			for(QMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.begin();
					iit != inven.end(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).begin(); sci != (*iit).end(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += tr("<tr><td><img height=\"16\" src=\"qrc%3\">%1</td><td>%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		rpt += "</div>";
	}

	// Mission
	rpt += "<b>MISSION</b><div style=\"margin-left: 10px;\">";
	if(ship->MissionIsEmpty())
	{
		rpt += QString("%1, %2, No Orders<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");
	}
	else
	{
		rpt += QString("%1, %2<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");

		rpt += ship->MissionHtmlReportTable();
	}
	rpt += "</div>";

	// Roster
	rpt += "<b>ROSTER</b><div style=\"margin-left: 10px;\">";
	{
		if(ship->CaptainsBerthType() == AuRosterEntry::CaptainSlot)
			rpt += QString("No Berths, Captain Slot Only<br>");
		else
		{
			if(ship->OfficerBerths())
				rpt += QString("%L1 Officer Berths<br>").arg(ship->OfficerBerths());
			if(ship->CrewBerths())
				rpt += QString("%L1 Crew Berths<br>").arg(ship->CrewBerths());
			if(ship->TroopBerths())
				rpt += QString("%L1 Troop Berths<br>").arg(ship->TroopBerths());
			if(ship->PassengerBerths())
			{
				if(ship->IsFirstClass())
					rpt += QString("%L1 1st Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else if(ship->IsSecondClass())
					rpt += QString("%L1 2nd Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else
					rpt += QString("%L1 3rd Class Passenger Berths<br>").arg(ship->PassengerBerths());
			}
			if(ship->CryoBerths())
				rpt += QString("%L1 Cryo Berths<br>").arg(ship->CryoBerths());
		}

		// Create a map of the roster so it will be alphabetized.
		QMap<QString, const char*> occs;		// Key is occupant name, value is icon resource name.
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			occs.insert((*ri).OccupantName(), (*ri).OccupantIcon());
		ship->UnlockRoster();

		rpt += tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
		for(QMap<QString, const char*>::const_iterator oi = occs.cbegin(); oi != occs.cend(); oi++)
			rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
		rpt += "</tbody></table><br>";

		// List occupied cryo berths.
		QMap<QString, const char*> cryos;		// Key is occupant name, value is icon resource name.
		const AuCryoBerths& cryo = ship->LockCryoBerths();
		for(AuCryoBerths::const_iterator it = cryo.constBegin(); it != cryo.constEnd(); it++)
		{
			const DgData& gd = g_Au.LockDataForRead();
			QString specieName = gd.SpeciesName(it.key(), true);
			g_Au.UnlockData();
			QString c = QString("%1, %L2").arg(specieName).arg(*it);
			cryos.insert(c, ":data/CryoPassenger.png");
		}
		ship->UnlockCryoBerths();

		if(!cryos.empty())
		{
			rpt += tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
			for(QMap<QString, const char*>::const_iterator oi = cryos.cbegin(); oi != cryos.cend(); oi++)
				rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
			rpt += "</tbody></table><br>";
		}
	}
	rpt += "</div>";

	// Send the report by mail to everyone in the chain of command of this officer.
	AuIdList recipients;
	AuIdList placeChain;
	const DgData& gd = g_Au.LockDataForRead();
	placeChain.append(AuId::Universe());
	placeChain.append(HomeGalaxyId());
	placeChain.append(HomeSectorId());
	placeChain.append(HomeSolarSystemId());
	placeChain.append(HomeWorldId());
	gd.EmpireReceivesFleetMessages(EmpireId(), placeChain, recipients);
	if(!ship->OwnerId().IsNull())
		recipients.AddIfUnique(ship->OwnerId());
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// List captain and human officers as recipients.
	const AuRoster& roster = ship->LockRoster();
	for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
		if((*ri).IsHumanOfficer())	// Also includes captain.
			recipients.AddIfUnique((*ri).m_OccId);
	ship->UnlockRoster();

	if(!recipients.isEmpty())
	{
		// Collect current system and world information for mail message. It helps people navigate to the mail origin.
		AuId systemId, worldId;
		QString systemName, worldName;
		AuPoint galaxyLoc;
		GALAXY galaxy = GALAXY_Count;
		const DaSolarSystem* system = FindParentSolarSystem();
		if(system)
		{
			systemId = system->Id();
			systemName = system->Name();
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				galaxy = pg->Galaxy();
				system->GetLocationRelativeToParent(galaxyLoc, pg);
			}
		}
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			worldId = world->Id();
			worldName = world->Name();
		}

		g_Au.SendBulkMail(idPlayer, confirmNum, ship->Id(), ship->Name(), recipients, rptMessageType, title, rpt,
				systemId, systemName, worldId, worldName, galaxy, galaxyLoc);
	}
	else
	{
		qDebug() << "DaNpc::ShipLog" << Id() << Name() << "has no commander, committing suicide.";
		ReleaseBerth(ship);
		Kill();
	}
	SetModified(NOTIFY_Database);
}

void DaNpc::ContactLog()
{
	// Do not send contact report while in arena.
	if(InArena() || InDesignPreview())
		return;

	QString title = tr("Contact");
	QString rpt = MailHeader(tr("Re: %1").arg(title), tr("Commander,"));

	// Comments
	DaVessel* ship = FindParentVesselInEnv();
	if(m_HomeSickness > d_HomeSickDeath)
	{
		// Time to commit suicide.
		qDebug() << "DaNpc::ContactLog" << Id() << Name() << "suicide, home sick" << m_HomeSickness;
		AuCombatAttacker attacker(tr("Suicide due to home sickness. There are no cities on my homeworld."));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_CID = CID_InternalInjuries;
		weapon.m_w1.m_Effect = WE_Pierce;

		AuCombatResult result;
		result.m_Damage1 = CurrentHitPoints() + g_Au.Dice().Roll(1,-DpBody::DeadHitPoints()-5,-1);

		CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		return;
	}
	else if(ship && ship->Is(AssignedShip()))
	{
		// I am assigned to a ship but I am not in command.
		// Mission
		if(ship->IsOnAMission())
		{
			rpt += tr("We are on an active mission. "
					"The current mission order is: %1 ").arg(ship->MissionOrderListDescription());

			// Check for unexplored destination.
			if(!DecideWhetherToTravel(ship))
				rpt += tr("Our mission takes us into unexplored space. "
						"The crew is holding our position until you arrive.");
		}
		else if(ship->MissionIsEmpty())
			rpt += tr("We currently have no mission orders.");
		else
			rpt += tr("We have a mission but we are holding, waiting for the mission to be activated.");
	}

	// Where am I from?
	if(m_HomeSickness > 1)
		rpt += tr("<p>I was deployed from %1 in %2, %3. "
				"I miss my home; it's been %4 days since I last heard from my family.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()))
			.arg(m_HomeSickness);
	else
		rpt += tr("<p>I was deployed from %1 in %2, %3.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()));

	// Closing
	rpt += tr("<p>"
		"Awaiting orders,<br>"
		"<i>%1</i><p>").arg(Name());

	// Event Log
	m_LogLock.lockForWrite();
	m_LogTime.Set();
	m_LogLock.unlock();

	// Send the report by mail to everyone in the chain of command of this officer.
	AuIdList recipients;
	AuIdList placeChain;
	const DgData& gd = g_Au.LockDataForRead();
	placeChain.append(AuId::Universe());
	placeChain.append(HomeGalaxyId());
	placeChain.append(HomeSectorId());
	placeChain.append(HomeSolarSystemId());
	placeChain.append(HomeWorldId());
	gd.EmpireReceivesFleetMessages(EmpireId(), placeChain, recipients);
	if(ship && !ship->OwnerId().IsNull())
		recipients.AddIfUnique(ship->OwnerId());
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	if(recipients.isEmpty())
	{
		qDebug("DaNpc::ContactLog %d %s has no commander, committing suicide.",
				Id().GetID(), (const char*)Name().toLatin1());
		ReleaseBerth();
		Kill();
	}
	else
	{
		// Collect current system and world information for mail message. It helps people navigate to the mail origin.
		AuId systemId, worldId;
		QString systemName, worldName;
		AuPoint galaxyLoc;
		GALAXY galaxy = GALAXY_Count;
		const DaSolarSystem* system = FindParentSolarSystem();
		if(system)
		{
			systemId = system->Id();
			systemName = system->Name();
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				galaxy = pg->Galaxy();
				system->GetLocationRelativeToParent(galaxyLoc, pg);
			}
		}
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			worldId = world->Id();
			worldName = world->Name();
		}

		g_Au.SendBulkMail(idPlayer, confirmNum, Id(), Name(), recipients, AuMailMsg::MSG_OfficerContact, title, rpt,
				systemId, systemName, worldId, worldName, galaxy, galaxyLoc);
	}

	SetModified(NOTIFY_Database);
}

void DaNpc::ShipLogFinal(const DaVessel* ship, const QString& reasonFinal, const QString& killerName, COMMODITYID weaponCID,
		quint8 weaponQ, AuIdList& notified)
{
	// Do not send final ship log while in arena.
	if(InArena() || InDesignPreview())
		return;

	QString title = tr("Captain's Log, Final Entry");
	QString rpt = MailHeader(ship, title, "Commander,");

	// Body
	rpt += tr("My ship, %1, has been destroyed. All hands were lost.").arg(ship->NameWithEmpire());

	// Where am I from?
	if(m_HomeSickness > 1)
		rpt += tr("<p>I was deployed from %1 in %2, %3. "
				"I miss my home; it's been %4 days since I last heard from my family.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()))
			.arg(m_HomeSickness);
	else
		rpt += tr("<p>I was deployed from %1 in %2, %3.")
			.arg(HtmlName(HomeWorldName()))
			.arg(HtmlName(HomeSectorName()))
			.arg(HtmlName(HomeGalaxyName()));

	// Closing
	rpt += tr("<p>"
		"In memory,<br>"
		"<i>%1</i><p>").arg(Name());

	// Reason for Destruction
	rpt += tr("<b>SPACECRAFT DESTROYED</b><div style=\"margin-left: 10px;\">");
	rpt += reasonFinal;

	// I'm not really sure if we ever get here with an invalid weaponCID.
	if(weaponCID != CID_Invalid)
	{
		if(weaponQ > 1)
		{
			if(!killerName.isEmpty())
				rpt += tr("<p>Destroyed by %1 using %2 Q%3.")
					.arg(killerName)
					.arg(g_Au.CommodityName(weaponCID))
					.arg(weaponQ);
			else
				rpt += tr("<p>Destroyed by %1 Q%2.")
					.arg(g_Au.CommodityName(weaponCID))
					.arg(weaponQ);
		}
		else
		{
			if(!killerName.isEmpty())
				rpt += tr("<p>Destroyed by %1 using %2.")
					.arg(killerName)
					.arg(g_Au.CommodityName(weaponCID));
			else
				rpt += tr("<p>Destroyed by %1.")
					.arg(g_Au.CommodityName(weaponCID));
		}
	}
	else if(!killerName.isEmpty())
		rpt += tr("<p>Destroyed by %1.").arg(killerName);

	rpt += "<br></div>";

	// Event Log
	m_LogLock.lockForWrite();
	QString eventRpt = m_Log.ReportHtml("EVENT LOG", m_LogTime);
	if(!eventRpt.isEmpty())
		rpt += eventRpt;
	m_LogTime.Set();
	m_LogLock.unlock();

	// Damage Report
	rpt += ship->DamageReportHtml();

	// Account
	rpt += "<b>ACCOUNT</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Ship's Fund<br>")
		.arg(g_Au.CommodityBitmapName(CID_Money))
		.arg((int)ship->AccountBalance())
		.arg(g_Au.CurrencySymbol());
	if(!ship->IsPrivatelyOwned())
	{
		rpt += QString("<img height=\"16\" src=\"qrc%1\"> %L2%3 Fleet Tax Collected<br>")
			.arg(g_Au.CommodityBitmapName(CID_Money))
			.arg((int)ship->AccountBalanceSubsidyTax())
			.arg(g_Au.CurrencySymbol());
	}
	rpt += "</div>";

	// Fuel
	rpt += "<b>FUEL</b><div style=\"margin-left: 10px;\">";
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Hydrogen %2 Q%3 in %4 Fuel Cells<br>")
		.arg(g_Au.CommodityBitmapName(CID_Hydrogen))
		.arg((int)ship->FuelQuantity())
		.arg(ship->FuelQuality())
		.arg(ship->FuelCapacity());
	rpt += QString("<img height=\"16\" src=\"qrc%1\"> Electricity %2 in %3 Capacitor<br>")
		.arg(g_Au.CommodityBitmapName(CID_Electricity))
		.arg((int)ship->CapacitorLevel())
		.arg((int)ship->CapacitorMax());
	rpt += "</div>";

	// Cargo in Hold
	if(ship->CargoCapacityVolume())
	{
		rpt += tr("<b>CARGO</b><div style=\"margin-left: 10px;\">");
		rpt += tr("Hold Space %1<br>").arg(ship->CargoCapacityVolume());

		// Make a map of the inventory table contents so it will be alphabetized.
		QString qual;
		QMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
		QMap<QString, QString> bitmap;		// Key is commodity name.
		const AuItemHash& sinv = ship->LockCargoForRead();
		for(AuItemHash::const_iterator it = sinv.begin(); it != sinv.end(); it++)
		{
			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = inven.find(commod);
			if(iit == inven.end())
				iit = inven.insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != inven.end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId())
					{
						// Item was created in an empire different from mine.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, tr("%1 Q%2 Made in %3")
								.arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, tr("%1 Q%2")
								.arg((*it).second->Count())
								.arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, tr("<i>%1 Q%2</i>")
							.arg((*it).second->Count())
							.arg((*it).second->Quality()));
				}
			}
		}
		ship->UnlockCargo();

		// Create a table of commodities.
		if(!inven.isEmpty())
		{
			rpt += tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			for(QMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.begin();
					iit != inven.end(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).begin(); sci != (*iit).end(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += tr("<tr><td><img height=\"16\" src=\"qrc%3\">%1</td><td>%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		rpt += "</div>";
	}

	// Mission
	rpt += "<b>MISSION</b><div style=\"margin-left: 10px;\">";
	if(ship->MissionIsEmpty())
	{
		rpt += QString("%1, %2, No Orders<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");
	}
	else
	{
		rpt += QString("%1, %2<br>")
			.arg(ship->IsMissionActive()?"Active":"On Hold")
			.arg((ship->MissionType() == AuMission::Repeat)?"Cyclic":"Do Once");

		rpt += ship->MissionHtmlReportTable();
	}
	rpt += "</div>";

	// Roster
	rpt += "<b>ROSTER</b><div style=\"margin-left: 10px;\">";
	{
		if(ship->CaptainsBerthType() == AuRosterEntry::CaptainSlot)
			rpt += QString("No Berths, Captain Slot Only<br>");
		else
		{
			if(ship->OfficerBerths())
				rpt += QString("%L1 Officer Berths<br>").arg(ship->OfficerBerths());
			if(ship->CrewBerths())
				rpt += QString("%L1 Crew Berths<br>").arg(ship->CrewBerths());
			if(ship->TroopBerths())
				rpt += QString("%L1 Troop Berths<br>").arg(ship->TroopBerths());
			if(ship->PassengerBerths())
			{
				if(ship->IsFirstClass())
					rpt += QString("%1 1st Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else if(ship->IsSecondClass())
					rpt += QString("%1 2nd Class Passenger Berths<br>").arg(ship->PassengerBerths());
				else
					rpt += QString("%1 3rd Class Passenger Berths<br>").arg(ship->PassengerBerths());
			}
		}

		// Create a map of the roster so it will be alphabetized.
		QMap<QString, const char*> occs;		// Key is occupant name, value is icon resource name.
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			occs.insert((*ri).OccupantName(), (*ri).OccupantIcon());
		ship->UnlockRoster();

		rpt += tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(32, 32, 32);\"><tbody>");
		for(QMap<QString, const char*>::const_iterator oi = occs.cbegin(); oi != occs.cend(); oi++)
			rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td></tr>") .arg(*oi) .arg(oi.key());
		rpt += "</tbody></table><br>";
	}
	rpt += "</div>";


	// Send a final report by mail to everyone in the chain of command of the officer.
	AuIdList placeChain;
	const DgData& gd = g_Au.LockDataForRead();
	placeChain.append(AuId::Universe());
	placeChain.append(HomeGalaxyId());
	placeChain.append(HomeSectorId());
	placeChain.append(HomeSolarSystemId());
	placeChain.append(HomeWorldId());
	gd.EmpireReceivesFleetMessages(EmpireId(), placeChain, notified);
	if(!ship->OwnerId().IsNull())
		notified.AddIfUnique(ship->OwnerId());
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// List captain and human officers as recipients.
	const AuRoster& roster = ship->LockRoster();
	for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
		if((*ri).IsHumanOfficer())	// Also includes captain.
			notified.AddIfUnique((*ri).m_OccId);
	ship->UnlockRoster();

	if(!notified.isEmpty())
	{
		// Collect current system and world information for mail message. It helps people navigate to the mail origin.
		AuId systemId, worldId;
		QString systemName, worldName;
		AuPoint galaxyLoc;
		GALAXY galaxy = GALAXY_Count;
		const DaSolarSystem* system = FindParentSolarSystem();
		if(system)
		{
			systemId = system->Id();
			systemName = system->Name();
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				galaxy = pg->Galaxy();
				system->GetLocationRelativeToParent(galaxyLoc, pg);
			}
		}
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			worldId = world->Id();
			worldName = world->Name();
		}

		g_Au.SendBulkMail(idPlayer, confirmNum, ship->Id(), ship->Name(), notified, AuMailMsg::MSG_ShipLogFinal, title, rpt,
				systemId, systemName, worldId, worldName, galaxy, galaxyLoc);
	}
}

double DaNpc::AccountBalance() const
{
	m_NpcLock.lockForRead();
	double v = m_AccountBalance;
	m_NpcLock.unlock();
	return v;
}

bool DaNpc::IsInStoryInstance(quint32 storyInstance) const
{
	return StoryInstance() == storyInstance;
}

bool DaNpc::SetRole(NPC_ROLE role)
{
	if(AssignedShipBerthType() != AuRosterEntry::Invalid)
		return false;	// I can't change role while holding a berth.
	m_Role = role;
	return true;
}

void DaNpc::SetIndestructible(bool isIndestructible)
{
	m_Indestructible = isIndestructible;
}

quint32 DaNpc::StoryScript() const
{
	const AuStories* stories = g_Au.LockStoriesForRead();
	quint32 scriptId = stories->ScriptId(m_StoryInstance);
	g_Au.UnlockStories();
	return scriptId;
}

AuId DaNpc::HomeGalaxyId() const
{
	return AuId::GalaxyId(HomeGalaxy());
}

QString DaNpc::HomeGalaxyName() const
{
	return GalaxyName(Galaxy());	// This is a global function in audef.h
}

bool DaNpc::IsCivilian() const
{
	if(StoryInstance())
		return true;	// Story characters are always civilians, so far.
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		return false;
	}
	return true;
}

bool DaNpc::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_Worker:
	case ROLE_Passenger:
		if(gunnerPosture != POSTURE_Berserk)
            break;   // Gunners don't shoot civilians unless they are berserk.
		return DaCitizen::IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc);
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		return DaCitizen::IsGuardTowerTarget(gunnerEmpire, gunnerPosture, gunLoc, gunRange, targetLoc);
	}
	return false;
}

bool DaNpc::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		if(gunnerPosture != POSTURE_Berserk)
			break;	// Gunners don't shoot civilians unless they are berserk.
		return DaCitizen::IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
				gunSysVec, targetSysLoc);
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		return DaCitizen::IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange,
				gunSysVec, targetSysLoc);
	}
	return false;
}

TRANSPORTER_TARGET DaNpc::TransporterTarget() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		return TT_Passenger;
	case ROLE_Crew:
		return TT_Crew;
	case ROLE_Troop:
		return TT_Troop;
	case ROLE_Officer:
		return TT_Officer;
	}
	return TT_Invalid;
}

void DaNpc::RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup)
{
	// Do not talk when I am unconscious.
	if(IsBodyDyingOrDead())
		return;

	if(StoryInstance())
	{
		switch(msg->Type())
		{
		default:
			// Story characters do not currently respond to messages on the comm. Sentient base class gives out jobs.
			//DaCitizen::RxChatMessage(msg, countSupplied, firstInGroup);
			break;

		case AuCommPacket::Auction:
			CharacterRxAuction(dynamic_cast<const AuCommAuction*>(msg));
			break;

		case AuCommPacket::ForSale:
			CharacterRxForSale(dynamic_cast<const AuCommForSale*>(msg));
			break;

		case AuCommPacket::Bid:
			CharacterRxBid(dynamic_cast<const AuCommBid*>(msg));
			break;

		case AuCommPacket::BidBarter:
			CharacterRxBidBarter(dynamic_cast<const AuCommBidBarter*>(msg));
			break;
		}
	}
	else
	{
		switch(Role())
		{
		case ROLE_Citizen:
		case ROLE_Count:
			break;
		case ROLE_IndigenousPerson:
		case ROLE_IndigenousElder:
		case ROLE_IndigenousHealer:
		case ROLE_IndigenousStargazer:
			{
				// Only communicate on the voice channel.
				if(!AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
					break;

				switch(msg->Type())
				{
				default:
					DaCitizen::RxChatMessage(msg, countSupplied, firstInGroup);
					break;

				case AuCommPacket::SolicitAllSales:
					SolicitSales(FILTER_All, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitAmmoSales:
					SolicitSales(FILTER_Ammo, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitArmorSales:
					SolicitSales(FILTER_ArmorAndClothes, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitPatentSales:
					SolicitSales(FILTER_Patents, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitConstructionMaterialSales:
					SolicitSales(FILTER_ConstructionMaterials, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitFoodSales:
					SolicitSales(FILTER_Food, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitFuelSales:
					SolicitSales(FILTER_Fuel, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitMedSales:
					SolicitSales(FILTER_MedicalSupplies, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitMiscSales:
					SolicitSales(FILTER_Misc, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitPartsSales:
					SolicitSales(FILTER_SpareParts, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitToolSales:
					SolicitSales(FILTER_Tools, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitNaturalResourceSales:
					SolicitSales(FILTER_NaturalResources, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitWeaponSales:
					SolicitSales(FILTER_Weapons, CID_Invalid, msg);
					break;

				case AuCommPacket::SolicitItemSales:
					SolicitSales(FILTER_All, msg->CommodityId(), msg);
					break;

				case AuCommPacket::Bid:
					IndigenousRxBid(dynamic_cast<const AuCommBid*>(msg));
					break;

				case AuCommPacket::BidBarter:
					IndigenousRxBidBarter(dynamic_cast<const AuCommBidBarter*>(msg));
					break;
				}
			}
			break;
		case ROLE_Worker:
			// Only communicate on the voice channel.
			if(AuCommStdChannel::IsVoiceChannel(msg->ChannelName()))
				DaCitizen::RxChatMessage(msg, countSupplied, firstInGroup);
			break;
		case ROLE_Passenger:
		case ROLE_Crew:
		case ROLE_Troop:
		case ROLE_Officer:
			DaCitizen::RxChatMessage(msg, countSupplied, firstInGroup);
			break;
		}
	}
}

// Returns true if a terse reply is given, indicating that no further conversation is wanted.
bool DaNpc::IndigenousTerseReply(const AuCommPacket* msg, const DaWorldLL* llworld)
{
	if(!llworld || !llworld->Is(HomeWorldId()))
	{
		QString text = tr("I want to return to the green hills of home.");
		AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
				msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Sad, msg->FromId());
		return true;
	}

	// Give an appropriate response to people who are not welcome.
	qint8 rep = llworld->VillageReputation(msg->FromId());
	if(Role() == ROLE_IndigenousElder)
	{
		if(rep <= -10)
		{
			QString text = tr("You have deeply alienated the people %1. "
					"A great deal of time must pass before their hearts will soften toward you.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Sad, msg->FromId());
			return true;
		}
		else if(rep <= -5)
		{
			QString text = tr("Leave the village %1. Go away from here; reflect upon your deeds. "
					"You are banished for a time.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Frown, msg->FromId());
			return true;
		}
		else if(rep < 0)
		{
			QString text = tr("The people are troubled by your actions %1. "
					"You must show good behavior and compassion for a while.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grim, msg->FromId());
			return true;
		}
	}
	else if(Role() == ROLE_IndigenousHealer)
	{
		if(rep <= -10)
		{
			QString text = tr("You have cut the people deeply %1. Only time can heal this wound.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Sad, msg->FromId());
			return true;
		}
		else if(rep <= -5)
		{
			QString text = tr("Leave here %1. The people need time to heal.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Frown, msg->FromId());
			return true;
		}
		else if(rep < 0)
		{
			QString text = tr("You have hurt the people %1. "
					"Be good for a while; let time be the cure.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grim, msg->FromId());
			return true;
		}
	}
	else if(Role() == ROLE_IndigenousStargazer)
	{
		if(rep <= -10)
		{
			QString text = tr("Be gone foul wretch! I'll have nothing to do with the likes of you.");
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Sad, msg->FromId());
			return true;
		}
		else if(rep <= -5)
		{
			QString text = tr("Go away %1. Your kind is unwelcome here.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Frown, msg->FromId());
			return true;
		}
		else if(rep < 0)
		{
			QString text = tr("I am busy. You are bothering me.");
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grim, msg->FromId());
			return true;
		}
	}
	else if(GrowthStage() == Juvenile)
	{
		if(rep <= -10)
		{
			QString text = tr("Aiee! I am begging you %1. Spare me; I am too young to die.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Sad, msg->FromId());
			return true;
		}
		else if(rep <= -5)
		{
			QString text = tr("Don't hurt me %1, please. I am only a child.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Frown, msg->FromId());
			return true;
		}
		else if(rep < 0)
		{
			QString text = tr("No %1. You are bad. Go away.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grim, msg->FromId());
			return true;
		}
	}
	else
	{
		// Adult
		if(rep <= -10)
		{
			QString text = tr("Die %1.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Sad, msg->FromId());
			return true;
		}
		else if(rep <= -5)
		{
			QString text = tr("Go away %1. You are shunned from the village.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Frown, msg->FromId());
			return true;
		}
		else if(rep < 0)
		{
			QString text = tr("Stop it %1. I will not speak to you.").arg(msg->FromName());
			AuCommHailReply rmsg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
					msg->FromSectorId(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grim, msg->FromId());
			return true;
		}
	}
	return false;
}

void DaNpc::SayIndigenousStargazer(const DaWorldLL& llworld, const DaPerson& person)
{
	m_Say++;
	if(m_Say > 2)
	{
		QString text = tr("Leave me alone. I am working on important calculations.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Grim, person.Id());
		return;
	}

	if(m_Say == 1 || m_Say == 2)
	{
		const DaSolarSystem* system = llworld.FindParentSolarSystemInEnv();
		if(system)
		{
			AuPoint asteroidLoc;
			AuTime asteroidTime = system->AsteroidImpactTime(llworld.Id(), asteroidLoc);
			if(asteroidTime != AuTime() && asteroidTime > AuTime::Now())
			{
				if(m_Say == 1)
				{
					// An asteroid is heading for this world.
					QString text = tr(
							"We are doomed. "
							"An ancient evil has awakened. "
							"It grows closer day by day. "
							);
					AuCommChat amsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(amsg, AuVisage::Grim, person.Id());

					text = tr(
							"I watch the sky, always. "
							"The signs are there now, as foretold through generations of stargazers. "
							);
					AuCommChat bmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(bmsg, AuVisage::Grim, person.Id());

					text = tr(
							"Prophecy teaches that a burning mountain will fall from the sky. "
							"Our world will be utterly destroyed. "
							"Everyone is going to die! "
							);
					AuCommChat dmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(dmsg, AuVisage::Grim, person.Id());

					text = tr("Tell me more. I want to live.");
					AuCommChatChoice emsg(person.Id(), text, 1, 0, AuCommStdChannel::VoiceChannelName(),
							Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(emsg, person.Id());
				}
				else
				{
					// Tell them when and where.
					QString dateString = asteroidTime.UTC().date().toString("dddd MMMM d");
					QString timeString = asteroidTime.UTC().time().toString("h:mm ap GMT");
					QString text = tr("According to my calculations, judgement day is %1, about %2.")
						.arg(dateString)
						.arg(timeString);
					AuCommChat amsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(amsg, AuVisage::Grim, person.Id());

					text = tr(
							"Those found worthy will be caught up into heaven and spared from this wrath. "
							"The rest will be thrown into the lake of fire. "
							);
					AuCommChat dmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(dmsg, AuVisage::Grim, person.Id());

					// The only useful location info we can give them is which hemisphere.
					double inc = asteroidLoc.PolarIncline();
					bool inPlane = false;
					if(inc > 30*DEG2RAD)
						text = tr("Stargazer lore says this evil will first be seen in the night sky of the northern hemisphere.");
					else if(inc < -30*DEG2RAD)
						text = tr("Stargazer lore says this evil will first be seen in the night sky of the southern hemisphere.");
					else
					{
						text = tr(
								"Stargazer lore says this evil approaches in the plane of the ecliptic "
								"so it may be hidden by the sun, depending on the season. "
								);
						inPlane = true;
					}
					AuCommChat bmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(bmsg, AuVisage::Grim, person.Id());

					// An asteroid is heading for this world.
					if(AuTime::Now().SecsTo(asteroidTime) < 60*15)
					{
						// At 15 minutes away, it an be seen with the naked eye.
						text = tr(
								"It is there! You can see it if you look, day or night. "
								"If only I were wrong. "
								);
						AuCommChat cmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
								Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
						Say(cmsg, AuVisage::Grim, person.Id());

						text = tr(
								"People remain surprisingly calm in the face of certain death. "
								"We are all subjects of a shared destiny. "
								"Who can resist the hand of fate? "
								);
						AuCommChat emsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
								Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
						Say(emsg, AuVisage::Grim, person.Id());
					}
					else
					{
						// The asteroid is still far away.
						if(inPlane)
							text = tr(
									"If our great and benevolent sun fails to shield us, "
										"this terror from the sky will become visible to everyone, day and night. "
									);
						else
							text = tr(
									"In the end, this terror from the sky will become visible to everyone, day and night. "
									);
						AuCommChat cmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
								Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
						Say(cmsg, AuVisage::Grim, person.Id());

						text = tr(
								"People will cower at sight of their destiny. "
								"Oh, the tragedy! "
								"What choice does any of us have? "
								);
						AuCommChat emsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
								Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
						Say(emsg, AuVisage::Grim, person.Id());
					}
				}
				return;
			}
		}
	}

	QString adultStatements[] =
	{
		tr("Tell Matt Maeson, I don't have a straight razor but I do have a joint paper. He is welcome to take it."),
		tr("Some rocks that fall from the sky are melted, but they will not melt or even glow in the hottest fire."),
		tr("Legends say there are beings who flit from place to place across the sky, in vehicles beyond our comprehension."),
		tr("I have seen lights moving across the night sky, in a different way than shooting stars or comets, "
				"as if they had a will of their own."),
		tr("When I was a child, I tried to find the treasure at the end of a rainbow."),
		tr("Birds fly as effortlessly as I can walk. How high can they go? Do they fly to the moon?"),
		tr("Daedalus made a set of wings for himself and his son Icarus. "
				"He told Icarus not to fly too close to the sun or the wax that binds the wings together would melt. "
				"Icarus didn't listen so he became the first sun diver."),
		tr("Lode stones contain a mysterious force that interacts with metal."),
		tr("Circular orbits and a common ecliptic plane for the entire solar system sure makes the calculations easier."),
		tr("The stars are the land-marks of the universe."),
		tr("New stars offer to the mind a phenomenon more surprising, and less explicable, than almost any other in the science of astronomy."),
		tr("It shows you exactly how a star is formed; nothing else can be so pretty! A cluster of vapor, the cream of the milky way, a sort of celestial cheese, churned into light."),
		tr("For my part I know nothing with any certainty but the sight of the stars makes me dream."),
		tr("When I consider how, after sunset, the stars come out gradually in troops from behind the hills and woods, I confess that I could not have contrived a more curious and inspiring sight."),
		tr("It is reasonable to hope that in the not too distant future we shall be competent to understand so simple a thing as a star."),
		tr("There wouldn't be a sky full of stars if we were all meant to wish on the same one."),
		tr("It is not in the stars to hold our destiny but in ourselves."),
		tr("The stars are the jewels of the night, and perchance surpass anything which day has to show."),
		tr("When it is darkest, men see the stars."),
		tr("God's promises are like the stars; the darker the night the brighter they shine."),
		tr("The stars are the street lights of eternity."),
		tr("If people sat outside and looked at the stars each night, I'll bet they'd live a lot differently."),
		tr("If the stars should appear but one night every thousand years how man would marvel and stare."),
		tr("Look at the stars! Look, look up at the skies! Oh look at all the fire-folk sitting in the air! The bright boroughs, the circle-citadels there!"),
		tr("Beyond the stars there are fields of wildflowers where Gods eternal presence inspires the hungry spirit as it nourishes the soul."),
		tr("The moon is quite a show off given the chance. The stars make a sound when they shine so bright. Water so blue and so black."),
		tr("He counts the number of the stars; He gives names to all of them. Great is our Lord and abundant in strength; His understanding is infinite."),
		tr("There are too many stars in some places and not enough in others."),
		tr("How is it that the sky feeds the stars?"),
		tr("The Sun, the stars and the seasons as they pass, some can gaze upon these with no strain of fear."),
		tr("Never judge the moon by gazing the stars."),
		tr("The sun is but a morning star."),
		tr("The stars are the apexes of what triangles!"),
		tr("You can feel the stars and the infinity of the sky since life, in spite of everything, is like a dream."),
		tr("Keep your eyes on the stars, and your feet on the ground."),
		tr("The sky is filled with stars, invisible by day."),
		tr("The sky was clear - remarkably clear - and the twinkling of all the stars seemed to be but throbs of one body, timed by a common pulse."),
		tr("I've loved the stars too fondly to be fearful of the night."),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
	};
	int statementCount = sizeof(adultStatements)/sizeof(QString);
	SayIndigenousSomething(adultStatements, statementCount, llworld, person);
}

void DaNpc::SayIndigenousAdult(const DaWorldLL& llworld, const DaPerson& person)
{
	m_Say++;
	if(m_Say > 4)
	{
		QString text = tr("I have work to do. Maybe we can chat another time.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
		return;
	}

	if(m_Say == 1 && llworld.BdHere())
	{
		QString text;
		switch(Id().GetID()%6)
		{
		case 0:
			// Don't mention any cities.
			break;
		case 1:
			text = llworld.IndigenousSayClosestCity(GlobeLL());
			break;
		case 2:
			text = llworld.IndigenousSayOldestCity(GlobeLL());
			break;
		case 3:
			text = llworld.IndigenousSayNewestCity(GlobeLL());
			break;
		case 4:
			text = llworld.IndigenousSayLargestCity(GlobeLL());
			break;
		case 5:
			text = llworld.IndigenousSaySmallestCity(GlobeLL());
			break;
		}

		if(!text.isEmpty())
		{
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Closed, person.Id());
			return;
		}
	}

	if(m_Say == 2)
	{
		const DaSolarSystem* system = llworld.FindParentSolarSystemInEnv();
		if(system)
		{
			AuPoint asteroidLoc;
			AuTime asteroidTime = system->AsteroidImpactTime(llworld.Id(), asteroidLoc);
			if(asteroidTime != AuTime() && asteroidTime > AuTime::Now())
			{
				// An asteroid is heading for this world.
				if(AuTime::Now().SecsTo(asteroidTime) < 60*15)
				{
					QString text = tr("I have seen the doom in the sky foretold by the Stargazer.");
					AuCommChat amsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(amsg, AuVisage::Grim, person.Id());

					text = tr("What are we to do, hide under the ground like badgers and moles? I choose to face the enemy.");
					AuCommChat bmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(bmsg, AuVisage::Grim, person.Id());
				}
				else
				{
					QString text = tr("Prophecy. Phooey! Can you believe that Stargazer?");
					AuCommChat amsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(amsg, AuVisage::Grim, person.Id());

					text = tr("The hysterics about the end of the world are scaring the women and children. "
							"They are the only ones who listen to the old fool.");
					AuCommChat bmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
					Say(bmsg, AuVisage::Grim, person.Id());
				}
			}
		}
	}

	QString adultStatements[] =
	{
		tr("All things are made up of four basic elements: earth, water, fire, air."),
		tr("I love the wind."),
		tr("I like to take long walks through the tall grass. It is very peaceful."),
		tr("It is good to store lots of food before winter starts."),
		tr("The land is our home."),
		tr("When you are on the trail, conserve sweat not water. Drink your water; "
				"your body uses it efficiently and it's easier to carry that way."),
		tr("I caught a trout in a stream. His colors flashed in the sun. My brother spoke to me without words."),
		tr("Yesterday we rode our horses fast across the plains. Their hooves barely touched the ground. We became the wind."),
		tr("I enjoy the sound of fish jumping in calm shallows on a warm summer evening."),
		tr("When butchering an animal, be careful not to cut into the gut sack."),
		tr("Every animal has enough tannin in its brain to tan its own hide."),
		tr("River fish are best in the spring. In late summer they taste like mud."),
		tr("Be careful setting traps. They are dangerous."),
		tr("Nothing beats the thrill of skimming fast across smooth water in a sailboat."),
		tr("Seal blubber is oily enough to light on fire. It makes black sooty smoke that gets into everything but it will not "
				"harm you."),
		tr("Dead fall traps are easy to make and they can bring down large animals."),
		tr("Meat lasts longer in storage if it is salted and dried."),
		tr("Many things live by the sea, in and out of the water. It is the best place to find food all year around."),
		tr("Beware of plants with milky white sap. They are usually toxic."),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
		tr("G"),
	};
	int statementCount = sizeof(adultStatements)/sizeof(QString);
	SayIndigenousSomething(adultStatements, statementCount, llworld, person);
}

void DaNpc::SayIndigenousChild(const DaWorldLL& llworld, const DaPerson& person)
{
	m_Say++;
	if(m_Say > 8)
	{
		QString text = tr("MOM!");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Sad, person.Id());
		return;
	}
	else if(m_Say > 7)
	{
		QString text = tr("Get away from me creep.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Frown, person.Id());
		return;
	}
	else if(m_Say > 6)
	{
		QString text = tr("Really, I have to go.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Grim, person.Id());
		return;
	}
	else if(m_Say > 5)
	{
		QString text = tr("My mom is calling me. I have to go.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
		return;
	}

	QString childStatements[] =
	{
		tr("When you climb way up in the tall trees, the swaying of the trunk feels like you are flying."),
		tr("I can hold my breath for a long time. Do you think I can be a space man?"),
		tr("Dad wants me to develop strict discipline and a high regard for sharing."),
		tr("When I picked my first berries and dug my first roots, they were given away to an elder. Mom said it was so I would "
				"learn to share my future successes."),
		tr("I like carrying water for the home. The elders always say the water tastes better, as if it held meat or berries."),
		tr("Everyone encourages me to not be lazy and to grow straight like a sapling."),
		tr("I am not supposed to speak to strangers."),
		tr("I want to be a space traveller some day."),
		tr("Some day I will be the elder of the village. Then I get to make the rules."),
		tr("I heard a scary story about a space monster once."),
		tr("It is safer near the village."),
		tr("If I had a space suit, I would be willing to travel."),
		tr("Mom is always worrying that I will get lost."),
		tr("Mom frets when we go swimming. How silly."),
		tr("Mom says not to swallow watermellon seeds or they will grow in my tummy."),
		tr("The fire is nice and warm but it can burn you."),
		tr("I sleep near the middle of the tent. That way the monsters cannot reach under the edge and grab me."),
		tr("Sometimes at night I stare up at the stars through the smoke hole in the tent."),
		tr("Do people really travel to other stars?"),
	};
	int statementCount = sizeof(childStatements)/sizeof(QString);
	SayIndigenousSomething(childStatements, statementCount, llworld, person);
}

void DaNpc::SayIndigenousHealer(const DaWorldLL& llworld, DaPerson& person)
{
	m_Say++;

	if(person.CurrentHitPoints() < 0)
	{
		AuCommChat msg(tr("Hold on to your life, even if it is easier to let go."),
				AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(msg, AuVisage::Closed, person.Id());
		return;
	}

	// Examine the patient.
	if(m_Say == 1 && person.IsPregnant())
	{
		AuCommChat msg(tr("I see you are pregnant. Children are the jewels of your crown when you grow old."),
				AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(msg, AuVisage::Smile, person.Id());
	}

	bool treatedCondition = false;
	bool mentionedDisease = m_Say > 1;		// I would have mentioned this the first time through.
	bool mentionedImplant = m_Say > 1;		// I would have mentioned this the first time through.
	bool mentionedParalysis = m_Say > 1;	// I would have mentioned this the first time through.
	AuConditionList& conds = person.LockConditionsForWrite();
	for(AuConditionList::iterator ci = conds.begin(); ci != conds.end(); ci++)
	{
		switch((*ci)->CType())
		{
		case AuCondition::Poison:
			// Poison can be treated.
			if((*ci)->Quality() > 0)
			{
				AuCommChat msg(tr("You are poisoned. These herbs will help."), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
						FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim, person.Id());

				(*ci)->ReduceQuality(1);
			}
			else
			{
				AuCommChat msg(tr("You are poisoned. These herbs will cure it."), AuCommStdChannel::VoiceChannelName(),
						Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim, person.Id());

				delete *ci;
				conds.erase(ci);
			}
			treatedCondition = true;
			break;
		case AuCondition::Disease:
			// Disease cannot be treated.
			if(!mentionedDisease)
			{
				AuCommChat msg(tr("You have a disease. It could be contageous. Stay comfortable and keep your health up."),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Frown, person.Id());
				mentionedDisease = true;
			}
			break;
		case AuCondition::Paralysis:
			// Paralysis is mentioned but ignored.
			if(!mentionedParalysis)
			{
				AuCommChat msg(tr("You have a paralysis of the muscles. It is not serious. It will pass."),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Closed, person.Id());
				mentionedParalysis = true;
			}
			break;
		case AuCondition::ImplantDNA:
		case AuCondition::ImplantSpecies:
			// Implant is mentioned but cannot be treated.
			if(!mentionedImplant)
			{
				AuCommChat msg(tr("A young %1 lives inside your body. It will feed upon you and grow. "
							"When it is ready, it will emerge. Keep your health up; it will be painful.").arg((*ci)->Name()),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Frown, person.Id());
				mentionedImplant = true;
			}
			break;
		case AuCondition::Acid:
			// Acid can be washed off.
			if((*ci)->Quality() > 0)
			{
				AuCommChat msg(tr("Acid is melting your flesh. It looks bad; this water will remove some of it."),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim, person.Id());
				(*ci)->ReduceQuality(1);
			}
			else
			{
				AuCommChat msg(tr("Acid is melting your flesh. This water will remove it. Hold still."),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim, person.Id());
				delete *ci;
				conds.erase(ci);
			}
			treatedCondition = true;
			break;
		case AuCondition::Fire:
			// Fire can be doused.
			{
				AuCommChat msg(tr("You are on fire. This water will douse it. Hold still."),
						AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(msg, AuVisage::Grim, person.Id());
				delete *ci;
				conds.erase(ci);
				treatedCondition = true;
			}
			break;
		case AuCondition::Antitoxin:
		case AuCondition::Bandage:
		case AuCondition::Bounty:
		case AuCondition::UNUSED:
		case AuCondition::Invalid:
		case AuCondition::Pregnant:
		case AuCondition::Thought:
			break;
		}
		if(treatedCondition)
			break;	// Do only one thing per pass.
	}
	if(treatedCondition)
	{
		person.ConditionsChanged();
		person.UnlockConditions();
		return;
	}
	person.UnlockConditions();

	if(person.CurrentHitPoints() < person.MaxHitPoints())
	{
		// Person is injured.
		if(person.HasCondition(AuCondition::Bandage))
		{
			AuCommChat msg(tr("The dressing on your wound looks clean. There is nothing more I can do."),
					AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(msg, AuVisage::Closed, person.Id());
		}
		else
		{
			AuCommChat msg(tr("You are injured. This poultice will fix you up. It is made from local minerals and herbs."),
					AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(msg, AuVisage::Grim, person.Id());

			AuZoneResource res;
			llworld.QueryNaturalResources(res, llworld.ResourceZone(GlobeLL().Longitude()));
			quint8 qh = res.Quality(CID_Herbs);
			quint8 qm = res.Quality(CID_Minerals);
			quint8 medQ;
			if(qh > qm)
				medQ = qh;
			else
				medQ = qm;

			// Add a new bandage.
			AuConBandage* bandage = new AuConBandage(medQ, AuTime::Now());
			person.AddCondition(bandage);
			person.SetModified();
		}
	}
	else
	{
		AuCommChat msg(tr("You look healthy to me. If you get hurt, I can treat your wounds using local herbs and minerals."),
				AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(msg, AuVisage::Closed, person.Id());
	}

	// Say something doctor-ish if I didn't say anything.
	QString doctorStatements[] =
	{
		tr("I have studied the healing effects of many minerals and wild herbs."),
		tr("Do not put anything into your ear except your elbow."),
		tr("You are as healthy as a horse today."),
		tr("Eat an apple a day to keep me away."),
		tr("My nurse told me the man I just treated collapsed in the entrance to my tent. "
				"I told her to turn him around so it looks like he was just arriving."),
		tr("A mother complained that her daughter lies in bed all day and eats yeast and bees wax. I told her not to worry, "
				"the daughter would eventually rise and shine."),
		tr("I told a patient he had two weeks to live. He asked if he could have them in autumn."),
		tr("One out of four people is mentally unbalanced. Think of your 3 closest friends... If they seem okay, then you're "
				"the one."),
		tr("I became insane with long intervals of horrible sanity."),
		tr("Reality is the leading cause of stress amongst those in touch with it."),
		tr("Neurotics build castles in the air. Psychotics live in them. Psychiatrists are the people who collect the rent."),
		tr("Truly great madness cannot be achieved without significant intelligence."),
		tr("Most men are within a fingers breadth of going mad."),
		tr("The allergist voted to scratch it."),
		tr("The dermatologist prefers no rash moves."),
		tr("The psychiatrist thought it was madness."),
		tr("The radiologist could see right through it."),
		tr("The gastro enterologist had a gut feeling about it."),
		tr("The neurologist thought the administration had a lot of nerve."),
		tr("The obstetrician stated they were labouring under a misconception."),
		tr("The ophthalmologist considered the idea short sighted."),
		tr("The pathologist yelled, 'Over my dead body!'"),
		tr("The paediatrician said, 'Grow up.'"),
		tr("The plastic surgeon said, 'This puts a whole new face on the matter.'"),
		tr("The podiatrist thought it was a step forward."),
		tr("The urologist felt the scheme wouldn't hold water."),
		tr("The surgeon decided to wash his hands of the whole thing."),
		tr("The anaesthesialogist thought the whole idea was a gas."),
		tr("The cardiologist didn't have the heart to say no."),
		tr("Damn! Page 47 of the manual is missing!"),
		tr("Better save that. We will need it for the autopsy."),
		tr("Wait a minute, if this is his spleen, then what's that?"),
		tr("...and could you stop that thing from beating; it's throwing my concentration off."),
		tr("Anyone see where I left that scalpel?"),
		tr("Orthodox medicine has not found an answer to your complaint. However, luckily for you, I happen to be a quack."),
		tr("After a year in therapy my psychiatrist said to me, 'Maybe life isn't for everyone.'"),
		tr("She got her looks from her father. He's a plastic surgeon."),
		tr("I'm not feeling very well, I need a doctor immediately. Ring the nearest golf course."),
		tr("The art of medicine is in amusing the patient while nature affects the cure."),
		tr("A woman went to a plastic surgeon and asked him to make her like a super model. He gave her a lobotomy."),
		tr("Whiskey is by far the most popular of all remedies that won't cure a cold."),
		tr("Anyone who goes to a psychiatrist should have his head examined."),
		tr("Never go to a doctor whose office plants have died."),
		tr("I'm always amazed to hear of crash victims so badly mutilated they have to be identified by their dental records. "
				"What I can't understand is, if they don't know who you are, how do they know who your dentist is?"),
		tr("Do you know what you call a medical student who graduates at the bottom of their class? Doctor."),
		tr("When we remember that we are all mad, the mysteries disappear and life stands explained."),
		tr("Most people are born with genius, but most people only keep it a few minutes."),
		tr("When dealing with the insane, the best method is to pretend to be sane."),
		tr("To conquer others requires force; to conquer oneself requires enlightenment."),
	};
	int statementCount = sizeof(doctorStatements)/sizeof(QString);
	SayIndigenousSomething(doctorStatements, statementCount, llworld, person);
}

void DaNpc::SayIndigenousElder(const DaWorldLL& llworld, const DaPerson& person)
{
	m_Say++;
	if(m_Say > 2)
	{
		QString text = tr("I am tired. I must rest now.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Frown, person.Id());
		return;
	}
	else if(m_Say > 1)
	{
		QString text = tr("I grow tired. I must rest.");
		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
				AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Grim, person.Id());

		text = llworld.IndigenousSayGeologicalFeature(person.GlobeLL(), true);
		if(!text.isEmpty())
		{
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Closed, person.Id());
		}

		if(person.HasAgentPower())
			llworld.DumpGeo();
		return;
	}

	QString elderStatements[] =
	{
		tr("The old Lakota was wise. "
				"He knew that man’s heart, away from nature, becomes hard; "
				"he knew that lack of respect for growing, living things soon led to a lack of respect for humans, too. "
				"So he kept his children close to nature’s softening influence."),
		tr("Inside us all, a battle rages between two wolves. "
				"One is evil and all things bad. The other is good and all things virtuous. "
				"The one that wins is the one you feed."),
		tr("Hold on to what is good, even if it is a handful of earth."),
		tr("Hold on to what you believe, even if it is a tree that stands by itself."),
		tr("Hold on to what you must do, even if it is a long way from here."),
		tr("Hold on to your life, even if it is easier to let go."),
		tr("Hold on to my hand, even if someday it is gone away from you."),
		tr("I salute the light within your eyes where the whole Universe dwells. "
				"For when you are at that center within you and I am at that place within me, we shall be one."),
		tr("You have to look deeper, way below the anger, the hurt, the hate, the jealosy, the self-pity, "
				"way down deeper where the dreams lie, son. Find your dream. It's the pursuit of the dream that heals you."),
		tr("I thank you, Wakantanka, for what you have given me."),
		tr("I am poor and naked, but I am the chief of the nation."),
		tr("We do not want riches but we do want to train our children right."),
		tr("Riches would do us no good. We could not take them with us to the other world."),
		tr("We do not want riches. We want peace and love."),
		tr("Hear me, my chiefs. I am tired. My heart is sick and sad. From where the sun now stands, "
				"I will fight no more forever."),
		tr("In our every deliberation, we must consider the impact of our decisions on the next seven generations."),
		tr("Children learn from what they see. We need to set an example of truth and action."),
		tr("When all the trees have been cut down, when all the animals have been hunted, when all the waters are polluted, "
				"when all the air is unsafe to breathe, only then will you discover you cannot eat money."),
		tr("Humankind has not woven the web of life. We are only one thread within it. Whatever we do to the web, "
				"we do to ourselves. All things are bound together. All things connect."),
		tr("When you know who you are; when your mission is clear and you burn with the inner fire of unbreakable will; "
				"no cold can touch your heart; no deluge can dampen your purpose. You know that you are alive."),
		tr("Eventually one gets to the Medicine Wheel to fulfill one's life."),
		tr("We live, we die, and like the grass and trees, renew ourselves from the soft earth of the grave. "
				"Stones crumble and decay, faiths grow old and they are forgotten, but new beliefs are born. "
				"The faith of the villages is dust now...but it will grow again...like the trees."),
		tr("The ground on which we stand is sacred ground. It is the dust and blood of our ancestors."),
		tr("Sometimes I go about pitying myself, and all the while I am being carried across the sky by beautiful clouds."),
		tr("A frog does not drink up the pond in which it lives."),
		tr("One does not sell the land people walk on."),
		tr("Between individuals, as between nations, peace means respect for the rights of others."),
		tr("When the white man discovered this country Indians were running it. No taxes no debt, women did all the work. "
				"White man thought he could improve on a system like this."),
		tr("Out of the Indian approach to life there came a great freedom, an intense and absorbing respect for life, "
				"enriching faith in a Supreme Power, and principles of truth, honesty, generosity, equity, "
				"and brotherhood as a guilde to mundane relations."),
		tr("Only to the white man was nature a wilderness and only to him was the land 'infested' with 'wild' animals and "
				"'savage' people. To us it was tame, Earth was bountiful and we were surrounded with the blessings of the "
				"Great Mystery."),
		tr("Grown men can learn from the very little children for the hearts of the little children are pure. Therefore, the "
				"Great Spirit may show to them many things which older people miss."),
		tr("I am going to venture that the man who sat on the ground in his tipi meditating on life and its meaning, accepting "
				"the kinship of all creatures, and acknowledging unity with the universe of things was infusing into his being "
				"the true essence of civilization."),
		tr("It does not require many words to speak the truth."),
		tr("Is it wrong for me to love my own? Is it wicked for me because my skin is red? Because I am Sioux? Because I was "
				"born where my father lived? Because I would die for my people and my country? God made me an Indian."),
		tr("He who would do great things should not attempt them all alone."),
		tr("A very great vision is needed and the man who has it must follow it as the eagle seeks the deepest blue of the sky."),
		tr("I am a red man. If the Great Spirit had desired me to be a white man he would have made me so in the first place. "
				"He put in your heart certain wishes and plans, in my heart he put other and different desires. Each man is good "
				"in his sight. It is not necessary for Eagles to be Crows. We are poor...but we are free. No white man controls "
				"our footsteps. If we must die...we die defending our rights."),
		tr("When a white army battles Indians and wins, it is called a great victory, but if they lose it is called a massacre."),
		tr("Our land is more valuable than your money. It will last forever. It will not even perish by the flames of fire. "
				"As long as the sun shines and the waters flow, this land will be here to give life to men and animals."),
		tr("One thing to remember is to talk to the animals. If you do, they will talk back to you. But if you don't talk to the "
				"animals, they won't talk back to you, then you won't understand, and when you don't understand you will fear, "
				"and when you fear you will destroy the animals, and if you destroy the animals, you will destroy yourself."),
		tr("The strength of the fire, the taste of the salmon, the trail of the sun, and the life that never goes away, they "
				"speak to me. And my heart soars."),
	};
	int statementCount = sizeof(elderStatements)/sizeof(QString);
	SayIndigenousSomething(elderStatements, statementCount, llworld, person);
}

void DaNpc::SayIndigenousSomething(const QString statements[], int statementCount, const DaWorldLL& llworld, const DaPerson& person)
{
	// Decide what to say.
	int tries = 0;
	QString text, saidKey;
	m_TalkLock.lockForRead();
	do
	{
		int pi = g_Au.Dice().Roll(1, statementCount, -1);
		text = statements[pi];
		if(text == "G")
			text = llworld.IndigenousSayGeologicalFeature(person.GlobeLL(), false);
		saidKey = text.left(10);
		tries++;
	}
	while(m_LastSaid.contains(saidKey) && tries < 20);	// Try up to 20 times to come up with something new to say.
	m_TalkLock.unlock();

	// Remember what I decided to say so I can avoid repeating myself.
	if(tries < 20 && !saidKey.isEmpty())
	{
		m_TalkLock.lockForWrite();
		m_LastSaid.append(saidKey);
		while(m_LastSaid.size() > 100)
			m_LastSaid.takeFirst();		// Remember the last 100 things I said.
		m_TalkLock.unlock();
	}

	// Say something.
	if(tries < 20 && !text.isEmpty())
	{
		if(text.length() < AuCommChat::MaxChatText())
		{
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Closed, person.Id());
		}
		else
		{
			// Cheat and use this one so I can say more than what a normal chat message allows.
			AuCommChatContinued rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Closed, person.Id());
		}

		QString continuations[] =
		{
			tr("What else is new?"),
			tr("That is so interesting."),
			tr("You don't say."),
			tr("Hmm."),
			tr("Uh huh."),
			tr("Oh."),
			tr("Golly."),
			tr("Go on."),
			tr("Ooh."),
			tr("Gosh."),
			tr("Groovy."),
			tr("Cool."),
			tr("Awesome."),
			tr("Astounding."),
			tr("Excellent."),
			tr("I wondered."),
			tr("That's worth thinking about."),
			tr("It happens."),
			tr("It's been known to happen."),
			tr("Really?"),
			tr("That is so profound."),
			tr("Pure wisdom."),
			tr("That sounds familiar."),
			tr("I heard that."),
			tr("Then what?"),
			tr("Do you always feel this way?"),
		};
		int ccount = sizeof(continuations)/sizeof(QString);
		int pi = g_Au.Dice().Roll(1, ccount, -1);
		text = continuations[pi];

		AuCommChatChoice nomsg(person.Id(), text, 1, 0, AuCommStdChannel::VoiceChannelName(),
				Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(nomsg, person.Id());
	}
	else
	{
		AuCommChat rmsg(tr("Enough talk. I have no more to say."), AuCommStdChannel::VoiceChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
	}
}

void DaNpc::CharacterRxBid(const AuCommBid* msg)
{
	if(!msg)
		return;

	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;

	DaSentient* sent = dynamic_cast<DaSentient*>(env->FindIdRecursive(msg->FromId()));
	if(!sent || !sent->IsInStoryInstance(m_StoryInstance))
		return;

	// Attempt to accept the bid.
	if(!g_Au.HasPendingBid(msg))
		return;

	// Extract barter offer info.
	const AuItem* item = msg->Item();							// The item I offered for sale.
	if(!item)
		return;
	double offerprice = msg->Price();
	QString empireNameOfOrigin = msg->EmpireOfOrigin();

	// Locate the item for sale.
	AuItemHash myCombinedInv;
	LockGearForRead()->CombineItemsForSale(myCombinedInv);
	UnlockGear();
	AuItemHash::iterator itemI = myCombinedInv.Find(*item);
	if(itemI == myCombinedInv.end())
	{
		// The item is no longer in stock.
		// Tell the buyer why.
		//QString text = tr("I do not have %1.")
			//.arg(item->ListDesc(empireNameOfOrigin));
		//AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				//Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		//g_Au.SendAtomChat(mymsg);
		return;
	}
	else if((*itemI).second->Count() < item->Count())
	{
		// We no longer have that many in stock.
		// Tell the buyer why.
		//QString text = tr("I only have %1 of them.")
			//.arg((*itemI).second->ListDesc(empireNameOfOrigin));
		//AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				//Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		//g_Au.SendAtomChat(mymsg);
		return;
	}

	/* Character always accepts the trade.
	// Check my selling price.
	double myprice = item->UnitCost() * item->Count() * .8;
	if(offerprice < myprice)
	{
		// Tell the buyer why.
		QString text = tr("Your offer is not good enough. I won't accept less value than %L1%2 for %L3.")
			.arg(myprice, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(item->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore the offer; it's too low.
	}
	*/

	// Insure the buyer can afford the cost.
	double fAccountBalance = sent->AccountBalance();
	if(offerprice > fAccountBalance)
	{
		// Tell the buyer why.
		QString text(tr("You don't have %L1%2.").arg(offerprice).arg(AuGlobal::CurrencySymbol()));
		AuCommCityChat mymsg(text, msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore the offer; insufficient funds.
	}

	// Insure the buyer can store the merchandise.
	AuItem* iNew = item->Clone();	// So we can mess with the count.
	DaPerson* person = dynamic_cast<DaPerson*>(sent);
	{
		// Add the item(s) to the person's inventory one at a time to insure they fit.
		iNew->SetCount(1);
		quint32 added = 0;	// Count of the number actually added.
		for(quint32 iadd=0; iadd<item->Count(); iadd++)
		{
			if(person)
			{
				if(person->AddGearItem(*iNew))
					added++;	// Count the ones that fit.
				else
					break;
			}
			else
			{
				if(sent->AddGearItemUseDrop(*iNew, 1, false))
					added++;	// Count the ones that fit.
				else
					break;
			}
		}
		if(added < item->Count())
		{
			// Some of the items fit into the buyer's inventory but not all of them.
			// Drop the remaining items at their feet.
			for(;added < item->Count(); added++)
				sent->DropItem(*iNew);

			// Tell the buyer what happened.
			QString text;
			if(added)
				text = tr("You only had room to carry %1 of the items. I dropped the rest at your feet.").arg(added);
			else
				text = tr("You had no room to carry the items. I dropped them at your feet.");
			AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(mymsg);
		}

		// Restore full count in iNew.
		iNew->SetCount(item->Count());
	}

	// Perform the transaction.

	// Player Statistics
	if(person)
	{
		person->Stats().ItemBoughtAsInventory(*iNew);
		person->IncreaseSkill(AuSkill::Trade, 1);
	}

	sent->DebitAccount(offerprice);					// Take the money from the person.

	// Remove the offered item from my inventory.
	RemoveGearItems(*iNew);

	CreditAccount(offerprice);

	// Revoke the bid so it can't be accepted again.
	g_Au.BidRevoked(msg->FromId(), AuCommStdChannel::VoiceChannelName());

	// Send an Accept message so the person knows the deal is done.
	AuCommAccept mymsg(msg->FromId(), msg->FromName(), *iNew, empireNameOfOrigin, offerprice, AuCommStdChannel::VoiceChannelName(),
			Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
	delete iNew;
	// Transaction is completed.
}

void DaNpc::CharacterRxBidBarter(const AuCommBidBarter* msg)
{
	if(!msg)
		return;

	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;

	DaSentient* sent = dynamic_cast<DaSentient*>(env->FindIdRecursive(msg->FromId()));
	if(!sent || !sent->IsInStoryInstance(m_StoryInstance))
		return;

	// Attempt to accept the bid.
	if(!g_Au.HasPendingBidBarter(msg))
		return;

	// Extract barter offer info.
	const AuItem* item = msg->Item();							// The item I offered for sale.
	if(!item)
		return;
	const AuItem* offerItem = msg->OfferItem();					// The item offered in trade by a buyer.
	if(!offerItem)
		return;
	QString offerEmpireNameOfOrigin = msg->OfferEmpireOfOrigin();
	//double offerprice = offerItem->TotalCost();
	QString empireNameOfOrigin = msg->EmpireOfOrigin();

	// Locate the item for sale.
	AuItemHash myCombinedInv;
	LockGearForRead()->CombineItemsForSale(myCombinedInv);
	UnlockGear();
	AuItemHash::iterator itemI = myCombinedInv.Find(*item);
	if(itemI == myCombinedInv.end())
	{
		// The item is no longer in stock.
		// Tell the buyer why.
		//QString text = tr("I do not have %1.")
			//.arg(item->ListDesc(empireNameOfOrigin));
		//AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				//Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		//g_Au.SendAtomChat(mymsg);
		return;
	}
	else if((*itemI).second->Count() < item->Count())
	{
		// We no longer have that many in stock.
		// Tell the buyer why.
		//QString text = tr("I only have %1 of them.")
			//.arg((*itemI).second->ListDesc(empireNameOfOrigin));
		//AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				//Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		//g_Au.SendAtomChat(mymsg);
		return;
	}

	/* Character always accepts the trade.
	// Check my selling price.
	double myprice = item->UnitCost() * item->Count() * .8;
	if(offerprice < myprice)
	{
		// Tell the buyer why.
		QString text = tr("Your offer is not good enough. I won't accept less value than %L1%2 for %L3.")
			.arg(myprice, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(item->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore the offer; it's too low.
	}
	*/

	// Insure the buyer has the item they offered.
	// Check the person's inventory.
	// List the person's personal items they might have to trade.
	// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
	// Lists items in top-level containers.
	// Does not list containers that are not empty.
	AuItemHash combinedInv;
	sent->LockGearForRead()->CombineItemsForSale(combinedInv);
	sent->UnlockGear();
	AuItemHash::const_iterator offerItemI = combinedInv.Find(*offerItem);	// Find match using IsItem test.
	if(offerItemI == combinedInv.end())
	{
		// The item is no longer in their inventory.
		// Tell the buyer why.
		QString text = tr("You do not have %1.")
			.arg(offerItem->ListDesc(offerEmpireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}
	else if((*offerItemI).second->Count() < offerItem->Count())
	{
		// Their inventory no longer contains that many.
		// Tell the buyer why.
		QString text = tr("You have only %1.")
			.arg(offerItem->ListDesc(offerEmpireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure the buyer can store the merchandise.
	AuItem* iNew = item->Clone();	// So we can mess with the count.
	DaPerson* person = dynamic_cast<DaPerson*>(sent);
	{
		// Add the item(s) to the person's inventory one at a time to insure they fit.
		iNew->SetCount(1);
		quint32 added = 0;	// Count of the number actually added.
		for(quint32 iadd=0; iadd<item->Count(); iadd++)
		{
			if(person)
			{
				if(person->AddGearItem(*iNew))
					added++;	// Count the ones that fit.
				else
					break;
			}
			else
			{
				if(sent->AddGearItemUseDrop(*iNew, 1, false))
					added++;	// Count the ones that fit.
				else
					break;
			}
		}
		if(added < item->Count())
		{
			// Some of the items fit into the buyer's inventory but not all of them.
			// Drop the remaining items at their feet.
			for(;added < item->Count(); added++)
				sent->DropItem(*iNew);

			// Tell the buyer what happened.
			QString text;
			if(added)
				text = tr("You only had room to carry %1 of the items. I dropped the rest at your feet.").arg(added);
			else
				text = tr("You had no room to carry the items. I dropped them at your feet.");
			AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(mymsg);
		}

		// Restore full count in iNew.
		iNew->SetCount(item->Count());
	}

	// Perform the transaction.

	// Player Statistics
	if(person)
	{
		person->Stats().ItemBoughtAsInventory(*iNew);
		person->IncreaseSkill(AuSkill::Trade, 1);
	}

	// The item has already been added to the person's inventory, or dropped on the ground.
	// Take the offer item from the person.
	sent->ConsumeItem(*offerItem);

	// Remove the offered item from my inventory.
	RemoveGearItems(*iNew);

	// Add the offer item to my inventory.
	if(offerItem->Count() > 1)
	{
		// Add them to my inventory one at a time.
		// Note that if the item is not ammo or a weapon, DaCitizen::AddGearItemUseDrop will only end up adding one of them.
		AuItem* offer1 = offerItem->Clone();
		offer1->SetCount(1);
		for(unsigned int oi=0; oi<offerItem->Count(); oi++)
			if(!AddGearItemUseDrop(*offer1, 2, false))
				break;	// If that one didn't fit, the next one won't either.
		delete offer1;
	}
	else
		AddGearItemUseDrop(*offerItem, 2, false);

	// Revoke the bid so it can't be accepted again.
	g_Au.BidRevoked(msg->FromId(), AuCommStdChannel::VoiceChannelName());

	// Send an Accept message so the person knows the deal is done. The count may have changed from the bid when not
	// all of the items fit in the buyer's inventory.
	AuCommAcceptBarter mymsg(msg->FromId(), msg->FromName(), *iNew, empireNameOfOrigin, *offerItem, offerEmpireNameOfOrigin,
			AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
			AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
	delete iNew;
	// Transaction is completed.
}

void DaNpc::CharacterRxAuction(const AuCommAuction* msg)
{
	if(!msg)
		return;

	m_NpcLock.lockForRead();
	COMMODITYID wantCID = m_WantCID;
	quint8 wantQ = m_WantQ;
	quint32 wantEoO = m_WantEoO;
	QString wantInscription = m_WantInscription;
	m_NpcLock.unlock();
	if(wantCID == CID_Invalid)
		return;	// I don't want anything.

	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;

	DaSentient* sent = dynamic_cast<DaSentient*>(env->FindIdRecursive(msg->FromId()));
	if(!sent || !sent->IsInStoryInstance(m_StoryInstance))
		return;

	// Determine what is for sale.
	const AuItem* item = msg->Item();
	if(!item)
		return;
	if(item->Count() != 1)
		return;

	if(item->StoryInstance() && item->StoryInstance() != m_StoryInstance)
	{
		//AuCommCityMessage mymsg(AuCommCityMessage::UnwantedStoryItem, msg->ChannelName(), Id(), CityBuildingName(), sectorId,
				//Galaxy(), range, msg->Scope());
		//g_Au.SendAtomChat(mymsg);
		return;
	}

	QString empireNameOfOrigin = msg->EmpireOfOrigin();
	AuItem* offer = item->Clone();						// So we can mess with the count.
	quint32 price = item->UnitCost();
	double totalForSalePrice = price * offer->Count();	// Price at which the commodities are offered for sale.
	// Do I want it?
	if(wantCID != item->CommodityId())
	{
		delete offer;
		return;
	}
	if(wantQ && wantQ != item->Quality())
	{
		delete offer;
		return;
	}
	if(wantEoO && wantEoO != item->EmpireOfOrigin())
	{
		delete offer;
		return;
	}
	if(!wantInscription.isEmpty() && wantInscription != item->Inscription())
	{
		delete offer;
		return;
	}

	// Can I afford their price?
	if(totalForSalePrice > AccountBalance())
		totalForSalePrice = AccountBalance();

	// Do I have room to carry the item?
	//const AuGear* gear = LockGearForRead();
	bool v = true;	//DHH test fit the item to see if I can take it.
	//UnlockGear();
	if(!v)
	{
		delete offer;
		return;
	}

	AuCommBid mymsg(*offer, empireNameOfOrigin, totalForSalePrice, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
			FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
	delete offer;
}

void DaNpc::CharacterRxForSale(const AuCommForSale* msg)
{
	if(!msg)
		return;

	m_NpcLock.lockForRead();
	COMMODITYID wantCID = m_WantCID;
	quint8 wantQ = m_WantQ;
	quint32 wantEoO = m_WantEoO;
	QString wantInscription = m_WantInscription;
	m_NpcLock.unlock();
	if(wantCID == CID_Invalid)
		return;	// I don't want anything.

	DaAtom* env = FindEnvironmentAtom();
	if(!env)
		return;

	DaSentient* sent = dynamic_cast<DaSentient*>(env->FindIdRecursive(msg->FromId()));
	if(!sent || !sent->IsInStoryInstance(m_StoryInstance))
		return;

	// Determine what is for sale.
	const AuItem* item = msg->Item();
	if(!item)
		return;
	if(item->Count() != 1)
		return;

	if(item->StoryInstance() && item->StoryInstance() != m_StoryInstance)
	{
		//AuCommCityMessage mymsg(AuCommCityMessage::UnwantedStoryItem, msg->ChannelName(), Id(), CityBuildingName(), sectorId,
				//Galaxy(), range, msg->Scope());
		//g_Au.SendAtomChat(mymsg);
		return;
	}

	QString empireNameOfOrigin = msg->EmpireOfOrigin();
	AuItem* offer = item->Clone();						// So we can mess with the count.
	quint32 price = msg->Price();
	double totalForSalePrice = price * offer->Count();	// Price at which the commodities are offered for sale.
	// Do I want it?
	if(wantCID != item->CommodityId())
	{
		delete offer;
		return;
	}
	if(wantQ && wantQ != item->Quality())
	{
		delete offer;
		return;
	}
	if(wantEoO && wantEoO != item->EmpireOfOrigin())
	{
		delete offer;
		return;
	}
	if(!wantInscription.isEmpty() && wantInscription != item->Inscription())
	{
		delete offer;
		return;
	}

	// Can I afford their price?
	if(totalForSalePrice > AccountBalance())
	{
		delete offer;
		return;
	}

	// Do I have room to carry the item?
	//const AuGear* gear = LockGearForRead();
	bool v = true;	//DHH test fit the item to see if I can take it.
	//UnlockGear();
	if(!v)
	{
		delete offer;
		return;
	}

	AuCommBid mymsg(*offer, empireNameOfOrigin, totalForSalePrice, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
			FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
	delete offer;
}

void DaNpc::SolicitSales(COMMODITYFILTER filter, COMMODITYID cid, const AuCommPacket* msg)
{
	if(!msg)
		return;
	if(msg->FromId() != Interlocutor())
		return;

	const DaWorldLL* planet = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
	if(!planet)
		return;

	// Get a list of the commodities that fit into the current filter.
	QList<COMMODITYID> cidFilter;
	if(cid != CID_Invalid)
		cidFilter.append(cid);											// Caller wants a specific commodity.
	else
		g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, filter);	// Caller wants a category of commodities.

	//DHH Build my inventory of things to sell.
	AuItemHash inventory;
	LockGearForRead()->CombineItemsForSale(inventory);
	UnlockGear();

	// Send the best of each commodity as a For Sale message.
	// We'll accumulate an alphabetically sorted list so they look nice on the clients screen.
	QString cname;				// only set when cid != CID_Invalid and we find one
	QMultiMap<QString, SellOffer> offers;	// List of offers sorted alphabetically.
	for(AuItemHash::const_iterator it = inventory.begin(); it != inventory.end(); it++)
	{
		// Determine if the item is available based on commodity filter.
		if(!cidFilter.contains((*it).second->CommodityId()))
			continue;

		// Prepare a sell offer.
		SellOffer offer(*(*it).second);
		offer.m_Price = (*it).second->UnitCost();

		// Remember commodity name for message text later.
		QString name = (*it).second->ListName();
		if(cid != CID_Invalid)
			cname = name;

		// Insert the sell offer into our list in alphabetical order.
		offers.insert(name, offer);
	}

	AuId sectorId = FindParentSector()->Id();

	if(!offers.isEmpty())
	{
		// Lock atom chat queue mutex so we can access both the open
		// channels queue and the chat messages queue simultaneously.
		AuAtomChatList* chatqueue = g_Au.LockAtomChat();

		// Construct a chat message to send on the voice channel.
		QString text;
		if(cid != CID_Invalid)
			text = tr("%1 for sale.").arg(cname);
		else
		{
			text = tr("This is what I have to trade.");
			if(IsNpcIndigenous())
				text += tr(" Use the price as a value guide; I do not want money.");
		}
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), sectorId, Galaxy(), AuCommStdChannel::Close,
				AuCommStdChannel::Audible);
		chatqueue->append(AuAtomChat(mymsg));

		// Now transmit the sorted list of offers as ForSale messages.
		for(QMultiMap<QString, SellOffer>::const_iterator it = offers.cbegin(); it != offers.cend(); it++)
		{
			// Format the ForSale message text.
			AuCommForSale mymsg(*(*it).m_Item, (*it).m_EmpireOfOrigin, (*it).m_Price, AuCommStdChannel::VoiceChannelName(), Id(),
					Name(), sectorId, Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			chatqueue->append(AuAtomChat(mymsg));
		}

		g_Au.UnlockAtomChat();
	}
	else if(cid != CID_Invalid)
	{
		// We had nothing to offer them. At least give them a response.
		AuCommChat mymsg(tr("I have none of those to trade."), AuCommStdChannel::VoiceChannelName(), Id(), Name(), sectorId,
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
	}
	else
	{
		// We had nothing to offer them. At least give them a response.
		AuCommChat mymsg(tr("I have nothing to trade."), AuCommStdChannel::VoiceChannelName(), Id(), Name(), sectorId,
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
	}
}

void DaNpc::IndigenousRxBid(const AuCommBid* msg)
{
	if(!msg)
		return;
	if(msg->FromId() != Interlocutor())
		return;

	// Don't talk when not on my home world.
	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
	if(!llworld || !llworld->Is(HomeWorldId()))
		return;

	// Don't trade with enemies.
	if(llworld->VillageReputation(msg->FromId()) < 0)
		return;

	AuCommChat mymsg(tr("I have no use for money. Do you have something to offer in trade?"), AuCommStdChannel::VoiceChannelName(),
			Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
			AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
}

void DaNpc::IndigenousRxBidBarter(const AuCommBidBarter* msg)
{
	if(!msg)
		return;
	if(msg->FromId() != Interlocutor())
		return;

	// Don't talk when not on my home world.
	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
	if(!llworld || !llworld->Is(HomeWorldId()))
		return;

	DaPerson* person = dynamic_cast<DaPerson*>(llworld->FindIdRecursive(msg->FromId()));
	if(!person)
		return;

	// Don't trade with enemies.
	if(llworld->VillageReputation(msg->FromId()) < 0)
		return;

	// Indigenous will attempt to accept the bid.
	if(!g_Au.HasPendingBidBarter(msg))
		return;

	// Extract barter offer info.
	const AuItem* item = msg->Item();							// The item I offered for sale.
	if(!item)
		return;
	const AuItem* offerItem = msg->OfferItem();					// The item offered in trade by a buyer.
	if(!offerItem)
		return;
	QString offerEmpireNameOfOrigin = msg->OfferEmpireOfOrigin();
	double offerprice = offerItem->TotalCost();
	QString empireNameOfOrigin = msg->EmpireOfOrigin();

	// Certain items are completely useless to indigenous.
	if(DbCommodity::IsPatent(offerItem->CommodityId()) ||
			offerItem->IsEnvironmentSuit() ||
			g_Au.Commodity(offerItem->CommodityId()).IsScDesign() ||
			DbCommodity::IsFiredFromHeavyWeapon(offerItem->CommodityId()))
	{
		QString text = tr("I have no use for %1.")
			.arg(item->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Locate the item for sale.
	AuItemHash myCombinedInv;
	LockGearForRead()->CombineItemsForSale(myCombinedInv);
	UnlockGear();
	AuItemHash::iterator itemI = myCombinedInv.Find(*item);
	if(itemI == myCombinedInv.end())
	{
		// The item is no longer in stock.
		// Tell the buyer why.
		QString text = tr("I do not have %1.")
			.arg(item->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}
	else if((*itemI).second->Count() < item->Count())
	{
		// We no longer have that many in stock.
		// Tell the buyer why.
		QString text = tr("I only have %1 left.")
			.arg((*itemI).second->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Check my selling price.
	double myprice = item->UnitCost() * item->Count() * .8;
	if(offerprice < myprice)
	{
		// Tell the buyer why.
		QString text = tr("Your offer is not good enough. I won't accept less value than %L1%2 for %L3.")
			.arg(myprice, 0,'f',0)
			.arg(AuGlobal::CurrencySymbol())
			.arg(item->ListDesc(empireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore the offer; it's too low.
	}

	// Insure the buyer has the item they offered.
	// Check the person's inventory.
	// List the person's personal items they might have to trade.
	// Consolidate the inventory into a list that stacks items instead of listing them one-by-one.
	// Lists items in top-level containers.
	// Does not list containers that are not empty.
	AuItemHash combinedInv;
	person->LockGearForRead()->CombineItemsForSale(combinedInv);
	person->UnlockGear();
	AuItemHash::const_iterator offerItemI = combinedInv.Find(*offerItem);	// Find match using IsItem test.
	if(offerItemI == combinedInv.end())
	{
		// The item is no longer in their inventory.
		// Tell the buyer why.
		QString text = tr("You do not have %1.")
			.arg(offerItem->ListDesc(offerEmpireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}
	else if((*offerItemI).second->Count() < offerItem->Count())
	{
		// Their inventory no longer contains that many.
		// Tell the buyer why.
		QString text = tr("You have only %1.")
			.arg(offerItem->ListDesc(offerEmpireNameOfOrigin));
		AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure the buyer can store the merchandise.
	AuItem* iNew = item->Clone();	// So we can mess with the count.
	{
		// Add the item(s) to the person's inventory one at a time to insure they fit.
		iNew->SetCount(1);
		quint32 added = 0;	// Count of the number actually added.
		for(quint32 iadd=0; iadd<item->Count(); iadd++)
		{
			if(person->AddGearItem(*iNew))
				added++;	// Count the ones that fit.
			else
				break;
		}
		if(added < item->Count())
		{
			// Some of the items fit into the buyer's inventory but not all of them.
			// Drop the remaining items at their feet.
			for(;added < item->Count(); added++)
				person->DropItem(*iNew);

			// Tell the buyer what happened.
			QString text;
			if(added)
				text = tr("You only had room to carry %1 of the items. I dropped the rest at your feet.").arg(added);
			else
				text = tr("You had no room to carry the items. I dropped them at your feet.");
			AuCommChat mymsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
			g_Au.SendAtomChat(mymsg);
		}

		// Restore full count in iNew.
		iNew->SetCount(item->Count());
	}

	// Perform the transaction.

	// Player Statistics
	person->Stats().ItemBoughtAsInventory(*iNew);
	person->IncreaseSkill(AuSkill::Trade, 1);

	// The item has already been added to the person's inventory, or dropped on the ground.
	// Take the offer item from the person.
	person->ConsumeItem(*offerItem);

	// Remove the offered item from my inventory.
	RemoveGearItems(*iNew);

	// Add the offer item to my inventory.
	if(offerItem->Count() > 1)
	{
		// Add them to my inventory one at a time.
		// Note that if the item is not ammo or a weapon, DaCitizen::AddGearItem will only end up adding one of them.
		AuItem* offer1 = offerItem->Clone();
		offer1->SetCount(1);
		for(unsigned int oi=0; oi<offerItem->Count(); oi++)
			if(!AddGearItemUseDrop(*offer1, 2, false))
				break;	// If that one didn't fit, the next one won't either.
		delete offer1;
	}
	else
		AddGearItemUseDrop(*offerItem, 2, false);

	// Revoke the bid so it can't be accepted again.
	g_Au.BidRevoked(msg->FromId(), AuCommStdChannel::VoiceChannelName());

	// Send an Accept message so the person knows the deal is done. The count may have changed from the bid when not
	// all of the items fit in the buyer's inventory.
	AuCommAcceptBarter mymsg(msg->FromId(), msg->FromName(), *iNew, empireNameOfOrigin, *offerItem, offerEmpireNameOfOrigin,
			AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close,
			AuCommStdChannel::Audible);
	g_Au.SendAtomChat(mymsg);
	delete iNew;
	// Transaction is completed.
}

// Kills this atom and all subatoms recursively.
void DaNpc::Kill()
{
	// When an officer is killed, its origin planet must be notified.
	// Pirates do not have a planet of origin.
	if(IsNpcOfficer() && !IsPirate())
	{
		AuId homeWorldId(HomeWorldId());
		if(!homeWorldId.IsNull())
			emit OfficerKilled(homeWorldId);
	}
	DaCitizen::Kill();
}

void DaNpc::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		if(gunnerPosture != POSTURE_Berserk)
			return;	// Gunners don't shoot civilians unless they are berserk.
		DaCitizen::FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc,
				gunRange, gunSysVec, targetAtom, targetRange, targetSysLoc);
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		DaCitizen::FindClosestShipTurretTargetRecursive(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc,
				gunRange, gunSysVec, targetAtom, targetRange, targetSysLoc);
		break;
	}
}

void DaNpc::StartVoiceConversationRecursive(const DaPerson& talker) const
{
	// Story characters do not respond to casual attempts to strike up a conversation.
	if(StoryInstance())
		return;
	DaCitizen::StartVoiceConversationRecursive(talker);
}

void DaNpc::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Do not accept additional damage if my body is already dead.
	if(IsBodyDead() || !IsNormal())
		return;

	if(!m_Indestructible)
		DaCitizen::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	if(IsNpcIndigenous() && !result.m_IsDestroyed && !attacker.m_Id.IsNull() && !attacker.m_IsPirate)
	{
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
		if(llworld && llworld->Is(HomeWorldId()))
		{
			// I am on my homeworld. Damage to me affects the attacker's reputation in my village.
			if(result.m_Damage1 || result.m_Damage2)
				llworld->IndigenousInjuredBy(attacker.m_Id);
			else
				llworld->IndigenousAttackedBy(attacker.m_Id);
		}
	}
}

void DaNpc::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	// Get armor afforded by body.
	DaCreature::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// Armor is not tallied from inventory when in a surgery unit.
	if(IsPatientStationPose())
		return;

	if(IsNpcPassenger())
	{
		// Calculate armor from items that are in valid "use" locations.
		// Passengers only tally armor when they are visibly wearing it. They don't always show space suit depending on environ.
		float invAbsorb, invBlock;

		// Tally clothes in all conditions.
		const AuGear* gear = LockGearForRead();
		const AuItem* gearItem = gear->ClothingItemWorn();
		if(gearItem)
		{
			// Clothes
			gearItem->CombatArmorEffectiveness(mode, effect, invAbsorb, invBlock);
			auxAbsorb += invAbsorb;
			auxBlock += invBlock;
		}

		if(!IsAirBreathable())
		{
			// Tally space suit in airless conditions. It's the only armor passengers ever wear.
			gearItem = gear->HelmetItemWorn();
			if(gearItem)
			{
				// Helmet
				gearItem->CombatArmorEffectiveness(mode, effect, invAbsorb, invBlock);
				auxAbsorb += invAbsorb;
				auxBlock += invBlock;
			}
			gearItem = gear->ArmorItemWorn();
			if(gearItem)
			{
				// Suit
				gearItem->CombatArmorEffectiveness(mode, effect, invAbsorb, invBlock);
				auxAbsorb += invAbsorb;
				auxBlock += invBlock;
			}
		}
		UnlockGear();
	}
}

// Render OpenAL audio sources
void DaNpc::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	Q_UNUSED(sceneAtom);
	if(!m_Sounds.isEmpty())
	{
		// The sound queue has a sound to play.
		AuPoint soundPos = model.Project(CameraOffset());	// Offset to my head.
		if(m_Sounds.first().IsPlaying())
		{
			// The first sound in the queue has been started already.
			if(al->IsSoundPlaying(m_AudioSource))
			{
				// The sound source is still playing the sound.
				// Update the sound source parameters for the playing sound, such as position and fade.
				al->Play(m_AudioSource, m_Sounds.first().AudioBuffer(), soundPos.x, soundPos.y, soundPos.z, fade, 1, 1.f,.75f);
			}
			else
			{
				// The current sound source has finished playing. Clean up.
				al->DeleteSound(m_Sounds.first().AudioBuffer());	// Delete the OpenAL sound buffer.
				m_Sounds.pop_front();							// Erase the sound entry from the array.
			}
		}
		else
		{
			// Start playing the next sound in the sound source.
			m_Sounds.first().StartPlaying();
			al->Play(m_AudioSource, m_Sounds.first().AudioBuffer(), soundPos.x, soundPos.y, soundPos.z, fade, 1, 1.f,.75f);
		}
	}
}

void DaNpc::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	// Do not render distant creatures.
	if(pass != PASS_Near)
		return;

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, rmod.Matrix(Id()), ari.al, ari.fade);

	// Base atom class needs to render special effects.
	DaCitizen::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaNpc::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(IsInVehicle())
		return;	// Don't add lights for myself or any subatoms when inside an enclosed vehicle.
	AuGearPos pos = CurrentItemPos();
	const AuGear* gear = LockGearForRead();
	const AuItem* item = gear->ItemAt(pos);
	if(item && item->IsLight() && item->State() == 1)
	{
		// I am equipped with a light and it is on.
		float lightHt = .5f;			// 6" above my eye camera.
		AuPoint lightPos = model.Project(CameraOffset() + AuPoint(0,0,lightHt));
		QColor colLight = item->LightColor();
		bool useCone = false;
		float cosInnerCone = .939692621f;		// 20 degrees
		float cosOuterCone = .766f;				// 40 degrees
		AuPoint lightVec;
		if(item->IsArmorHelmetClothingOrHat())
		{
			useCone = true;
			lightVec = model.ProjectVector(AuPoint(0,1,0).RotateX(CurrentHeadTilt()));
		}

		float lightFlicker = item->LightFlicker(Id());
		if(IsInsideCapsule())
			lightFlicker /= 4.f;		// Tone down lights a lot when aboard a spacecraft or inside a building.

		// Emit light from above the person's head.
		float lightRadius = item->LightRadius() * lightFlicker;
		float kQ = 1.f/(lightRadius * lightRadius);
		float kL = .0065f;
		if(useCone)
			lights.append(AuGLLight(Id(), AuGLLight::Person, kL,kQ, colLight, lightPos, cosOuterCone, cosInnerCone, lightVec));
		else
			lights.append(AuGLLight(Id(), AuGLLight::Person, kL,kQ, colLight, lightPos));
	}
	UnlockGear();
	DaCitizen::GetRenderLightsRecursive(person, model, suns, lights);
}

bool DaNpc::IsTargetToWildAnimal() const
{
	if(StoryInstance())
		return g_Au.IsStoryActive(m_StoryInstance) ? false : true;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		return true;
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		return true;
	}
	return false;
}

void DaNpc::StayIdle()
{
	// Don't do idle animation when not in anyone's scene.
	if(!IsInAnyScene())
	{
		ClearInterlocutor();
		return;
	}

	// Don't do this in a vehicle, at a station, or riding a mount; it messes up the vehicle seat pose.
	if(IsAboardVehicle() || IsLockedPose() || IsRider())
	{
		// Keep current interlocutor state. Don't initiate a conversation.
		DaCitizen::StayIdle();
		return;
	}

	AuId intid(Interlocutor());
	if(!intid.IsNull())
	{
		// I am talking to someone. Turn to face them.
		DaCitizen::StayIdle();
		return;
	}

	// I am not talking to someone.
	if(StoryInstance())
	{
		// I am a story character. Don't do self-motivated things.
		DaCitizen::StayIdle();
		return;
	}

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_IndigenousPerson:
		if(GrowthStage() == Mature)
		{
			// Adults watch for threats. They will attack people whose reputation is too low in the village.
			// Find the closest person who is on line.
			// Does not find people aboard vehicles or spacecraft.
			DaPerson* person;
			AuPoint personLoc;
			double personRange;
			LookForClosestPerson(person, personLoc, personRange);
			if(person)
			{
				// Found a person who is on line.
				// Can I perceive them? This code was cobbled from DaAnimal.
				bool perceivedThem = false;

				if(!IsUnderGround() && !person->IsUnderGround())
				{
					// Neither of us is underground.
					// I may be able to see them.
					float eyeRange = SightRange();				// Range of eyes.
					if(IsEyeSubmerged())
						eyeRange /= 3.f;
					if(personRange < eyeRange)
						perceivedThem = true;

					if(!perceivedThem && AtmosphereDensity() > 0)
					{
						// I cannot see them and there is atmosphere density present.
						// I may be able to smell them.
						float noseRange = SmellRange();			// Range of nose.
						if(IsEyeSubmerged())
							noseRange /= 2.f;
						if(personRange < noseRange)
						{
							if(person->IsCGSubmerged() && IsEyeSubmerged())
								perceivedThem = true;					// Both of us are under water.
							else if(!person->IsEyeSubmerged() && !IsEyeSubmerged())
								perceivedThem = true;					// Neigher of us is under water.
						}
					}
				}

				if(!perceivedThem && (IsUnderGround() || AtmosphereDensity()))
				{
					// I cannot see or smell them and I am underground or there is atmosphere density present.
					// I may be able to hear them.
					float earRange = HearingRange();				// Range of ears.
					if(IsEyeSubmerged())
						earRange *= 2.f;

					// Noise level of creature affects perception range with ears.
					float noise = person->NoiseLevel();
					if(personRange < earRange * noise)
						perceivedThem = true;
				}

				if(perceivedThem)
				{
					// Found a person in my area who is on line and their lineage makes them superior to me.
					// What is their reputation in the village?
					DaWorld* planet = FindParentPlanetInEnv();
					DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
					if(llworld)
					{
						qint8 rep = llworld->VillageReputation(person->Id());
						if(rep <= -10)
						{
							SetOrder(ORDER_AttackTarget, person->Id(), AuPoint(0,0));	// Attack them.
							PoseVisage(AuVisage::Frown, AuVisage::Frown);
						}
					}
				}
			}
		}
		break;
	case ROLE_Worker:
		if(g_Au.Dice().Roll(1,100) <= 20)
		{
			// Find the closest person who is on line.
			// Does not find people aboard vehicles or spacecraft.
			DaPerson* person;
			AuPoint personLoc;
			double personRange;
			LookForClosestPerson(person, personLoc, personRange);
			if(person && ComparePersonLineage(person->Lineage()) <= 0)
			{
				// Found a person who is on line. Their lineage makes them superior to me.
				// Can I perceive them? This code was cobbled from DaAnimal.
				bool perceivedThem = false;

				if(!IsUnderGround() && !person->IsUnderGround())
				{
					// Neither of us is underground.
					// I may be able to see them.
					float eyeRange = SightRange();				// Range of eyes.
					if(IsEyeSubmerged())
						eyeRange /= 3.f;
					if(personRange < eyeRange)
						perceivedThem = true;

					if(!perceivedThem && AtmosphereDensity() > 0)
					{
						// I cannot see them and there is atmosphere density present.
						// I may be able to smell them.
						float noseRange = SmellRange();			// Range of nose.
						if(IsEyeSubmerged())
							noseRange /= 2.f;
						if(personRange < noseRange)
						{
							if(person->IsCGSubmerged() && IsEyeSubmerged())
								perceivedThem = true;					// Both of us are under water.
							else if(!person->IsEyeSubmerged() && !IsEyeSubmerged())
								perceivedThem = true;					// Neigher of us is under water.
						}
					}
				}

				if(!perceivedThem && (IsUnderGround() || AtmosphereDensity()))
				{
					// I cannot see or smell them and I am underground or there is atmosphere density present.
					// I may be able to hear them.
					float earRange = HearingRange();				// Range of ears.
					if(IsEyeSubmerged())
						earRange *= 2.f;

					// Noise level of creature affects perception range with ears.
					float noise = person->NoiseLevel();
					if(personRange < earRange * noise)
						perceivedThem = true;
				}

				if(perceivedThem && personRange < 15)
				{
					// Found a person in my area who is on line and their lineage makes them superior to me.
					if(SelectJobOffer(*person))
					{
						// I have a job offer for the person.
						if(CurrentGeneralPose().Pose() != AuPose::Stand)
							DoPose(AuPose::Stand);

						// Speak to the person. This causes me to turn to face them.
						AuCommHailRequest msg(EmpireId(), QString(), AuCommStdChannel::VoiceChannelName(), Id(), Name(), 
								FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
						Say(msg, AuVisage::Closed, person->Id());

						if(g_Au.Dice().Roll(1,100) <= 20)
							SayWorkerOpinion(*person);	// Say something about my town.
					}
				}
			}
		}
		break;
	case ROLE_Passenger:
		if(IsInsideSpacecraft())
		{
			// Do nothing until my ship's design is analyzed.
			DaVessel* ship = FindParentVesselInEnv();
			if(!ship || !ship->IsDesignDataCacheValid())
				return;

			// Sometimes I decide to go to another place on the ship.
			// I come through here about every 3-5 seconds.
			// If I decide to move 1 in 100 times, I'll move every 300 to 500 seconds, which is about every 5 to 8 minutes.
			// If I decide to move 1 in 200 times, I'll move every 600 to 1000 seconds, which is about every 10 to 16 minutes.
			// Always decide to get out of the sick bay or the room fills with recently healed patients.
			if((ShipRoomType() == ROOM_SickBay || g_Au.Dice().Roll(1,200) == 1) && !IsMounted())
			{
				GoToWork();				// Go somewhere on the ship.
				return;
			}

			if(!IsAboardVehicle())
			{
				// Look for a person in my room who is on line. Ignore people who are invisible.
				DaPerson* person = ship->FindPersonOnLine(ShipRoomId());
				if(person && !person->IsEthereal() && ComparePersonLineage(person->Lineage()) <= 0)
				{
					// Found a person in my room who is on line and their lineage makes them superior to me.
					if(person->EnvLocation().Distance2(EnvLocation()) < 10*10)
					{
						if(SelectJobOffer(*person))
						{
							// I have a job offer for the person.
							if(CurrentGeneralPose().Pose() != AuPose::Stand)
								DoPose(AuPose::Stand);

							// Speak to the person. This causes me to turn to face them.
							AuCommHailRequest msg(EmpireId(), QString(), AuCommStdChannel::VoiceChannelName(), Id(), Name(), 
									FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
							Say(msg, AuVisage::Closed, person->Id());
						}
					}
				}
			}
		}
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		break;
	}

	// Base class does random turning and head tilt behavior.
	// Turns to face interlocutor.
	DaCitizen::StayIdle();
}

// Returns the closest person on line who is in this worker's terrain patch or a neighboring terrain patch.
// 'personLoc' is relative to this atom.
void DaNpc::LookForClosestPerson(DaPerson*& person, AuPoint& personLoc, double& personRange) const
{
	// Init return values.
	person = 0;
	personLoc = AuPoint(0,0);
	personRange = 0;
	const DaWorldLL* llworld = FindParentWorldInEnv();
	if(!llworld)
		return;

	// Search my own globe.
	AuMatrix4x4 tme;
	GetTransformRelativeToParent(tme, llworld);
	AuPoint myLoc = tme.Translation();							// My location relative to my parent globe.
	llworld->FindClosestPersonOnLineRecursive(myLoc, person, personRange);
	if(!person)
		return;

	// Calculate the location and distance of the person relative to this atom.
	AuPoint personPlanetLoc;
	person->GetLocationRelativeToParent(personPlanetLoc, llworld);				// Person's loc relative to the planet.
	personLoc = tme.UnProject(personPlanetLoc);									// Person's loc relative to me.
	personRange = sqrt(personRange);							// FindClosestPersonOnLineRecursive returns a squared value.
}

void DaNpc::SayWorkerOpinion(const DaPerson& person)
{
	// Say something about my town.
	if(InBdCity().IsNull())
		return;

	const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
	const DaAtom* atom = llworld->FindSubatomId(InBdCity());
	const DaBd* bd = dynamic_cast<const DaBd*>(atom);
	if(bd)
	{
		// In a bd. Are they returning here or is this their first visit?
		AuTime firstVisit = person.FirstMemoryOfCity(InBdCity());
		QString text;
		if(firstVisit.IsNull() || !firstVisit.ElapsedDay())
			text = tr("Welcome to %1, %2. ").arg(bd->CityName()).arg(person.TitleName());
		else
			text = tr("Welcome back to %1, %2. ").arg(bd->CityName()).arg(person.TitleName());

		QString empname;	// Empire who owns the city, could be occupier.
		//QString allegname;	// Empire of allegiance.
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(bd->EmpireId(), empname);
		//gd.EmpireName(bd->AllegEmpire(), allegname);
		g_Au.UnlockData();
		float loyalty = 1;	// Loyalty toward empire of allegiance, in the range 0 to 1.
		/*
		if(city->IsOccupied())
		{
			// Empire of allegiance is not the same as empire who owns the city.
			if(loyalty > .9f)
				text += tr("Almost everyone hates the %1 for occupying our city. "
						"They are eager for %2 to return and crush them.").arg(empname).arg(allegname);
			else if(loyalty > .5f)
				text += tr("Most of the people here hate the %1 for occupying our city. "
						"They wait expectantly for the day when %2 returns to liberate us.").arg(empname).arg(allegname);
			else if(loyalty > .1f)
				text += tr("Many people dislike the %1 for occupying our city. "
						"They pine for the day when %2 returns to save us. "
						"I don't think it's going to happen.").arg(empname).arg(allegname);
			else
				text += tr("A few people remember the days of %2, before the %1 occupied our city. "
						"They just can't let go. "
						"I don't see that it matters much any more.").arg(empname).arg(allegname);
		}
		else
		*/
		{
			text += tr("Our community is an important part of the %1. ").arg(empname);
			if(loyalty > .9f)
				text += tr("Almost everyone works hard for the team, doing their best to make the empire stronger.");
			else if(loyalty > .5f)
				text += tr("Most people root for team empire but there's still a lot of them who aren't so committed.");
			else if(loyalty > .1f)
				text += tr("Many people swear allegiance to the empire but the majority of them have not come "
						"around yet.");
			else
				text += tr("A few people have sworn allegiance to the empire. They are an inspiration to others.");
		}

		if(text.isEmpty())
			return;

		AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(),
				FindParentSector()->Id(), Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Audible);
		Say(rmsg, AuVisage::Closed, person.Id());
	}
}

void DaNpc::BodyHasDied(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
		const QString& killerName)
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		{
			AuId killerId = KillerId();	// The person/unit who got the most hits on me.
			if(!killerId.IsNull())
			{
				// On my homeworld, death to me affects the attacker's reputation in my village.
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
				if(llworld && llworld->Is(HomeWorldId()))
					llworld->IndigenousKilledBy(killerId);
			}
		}
		break;
	case ROLE_Worker:
		if(!InDesignPreview())
		{
			// Worker died so reduce the population of the city by one.
			DaWorld* planet = FindParentPlanetInEnv();
			if(planet)
			{
				DaAtom* atom = planet->FindSubatomId(BdId());
				DaBd* bd = dynamic_cast<DaBd*>(atom);
				if(bd)
					bd->WorkerDied(planet, 1);
			}
		}
		break;
	case ROLE_Passenger:
	case ROLE_Crew:
	case ROLE_Troop:
		break;
	case ROLE_Officer:
		if(!IsPirate() && !InDesignPreview())
		{
			// Officer assigned to a ship drops event log disk even if the event log is empty.
			// Pirates do not do this because they do not maintain an event log.
			if(!AssignedShip().IsNull())
			{
				// I am assigned to a ship and I am not a pirate.
				// Home sector and world name.
				QString home = QObject::tr("%1 in %2, %3").arg(HomeWorldName()).arg(HomeSectorName()).arg(HomeGalaxyName());

				// Assigned ship and description. Could be blank.
				QString shipName, shipDesc;
				bool shipPrivate = false;
				{
					const DaVessel* ship = FindParentVesselInEnv();
					if(!ship || !ship->Is(AssignedShip()))
					{
						// Not aboard a ship or it's not my assigned ship.
						const DaAtom* system = FindEnvironmentAtom();
						if(system)
							ship = dynamic_cast<const DaVessel*>(system->FindIdRecursive(AssignedShip()));
						else
							ship = 0;
					}
					if(ship)
					{
						shipName = ship->NameWithEmpire();
						shipDesc = ship->Description();
						shipPrivate = ship->IsPrivatelyOwned();
					}
				}

				// Drop the item. The disk is created out of thin air so I will assign it a random quality value.
				m_LogLock.lockForRead();
				AuItemOfficerLog itemLog(Name(), home, shipName, shipDesc, shipPrivate, m_Log, CID_OfficerLog,
						g_Au.Dice().RandomQuality(), EmpireId());
				m_LogLock.unlock();

				DropItem(itemLog);
			}

			// Officer reports death. Pirates do not send mail
			DeathLog(weaponCID, weaponQ, effect1, effect2, killerName);	// Officer notifies chain-of-command when dying.
		}
		break;
	}

	// Base class drops a random item and meat. It also releases my berth.
	DaCitizen::BodyHasDied(weaponCID, weaponQ, effect1, effect2, killerName);
}

// Officer notifies chain-of-command when killed.
void DaNpc::DeathLog(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
		const QString& whoName)
{
	// Do not send death log while in arena.
	if(InArena() || InDesignPreview())
		return;

	QString title = tr("Defeat");
	QString rpt = MailHeader(tr("Re: %1").arg(title), "Commander,");

	// Reason for Destruction
	// I'm not really sure if we ever get here with an invalid weaponCID.
	if(weaponCID != CID_Invalid)
	{
		// Report weapon TL only if it's more than 1.
		QString tlgotme;
		if(weaponQ)
			tlgotme = QString(" Q%1").arg(weaponQ);

		if(!whoName.isEmpty())
		{
			if(effect2 != WE_None)		// Always implies effect1 != WE_None.
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%3 got me using a %4%5. "
						"These %6 and %7 wounds are fatal; I am slipping away."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect2)).arg(EffectName(effect1));
			else if(effect1 != WE_None)
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%3 got me using a %4%5. "
						"This %6 wound is fatal; I am slipping away."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect1));
			else
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%3 got me using a %4%5."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(whoName).arg(g_Au.CommodityName(weaponCID)).arg(tlgotme);
		}
		else
		{
			if(effect2 != WE_None)		// Always implies effect1 != WE_None.
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%4%5 got me. "
						"These %6 and %7 wounds are fatal; I am slipping away."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect2)).arg(EffectName(effect1));
			else if(effect1 != WE_None)
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%4%5 got me. "
						"This %6 wound is fatal; I am slipping away."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme)
					.arg(EffectName(effect1));
			else
				rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
						"%4%5 got me."
						"<p>"
						"Unconsiously,<br>"
						"<i>%1</i>")
					.arg(Name())
					.arg(g_Au.CommodityName(weaponCID)).arg(tlgotme);
		}
	}
	else
	{
		if(effect2 != WE_None)		// Always implies effect1 != WE_None.
			rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
					"These %6 and %7 wounds are fatal; I am slipping away."
					"<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name())
				.arg(EffectName(effect2)).arg(EffectName(effect1));
		else if(effect1 != WE_None)
			rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
					"This %6 wound is fatal; I am slipping away."
					"<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name())
				.arg(EffectName(effect1));
		else
			rpt += tr("I have fallen in the line of duty. Please notify my widow if I don't make it.<p>"
					"Unconsiously,<br>"
					"<i>%1</i>")
				.arg(Name());
	}

	const DaVessel* ship = FindParentVesselInEnv();

	// Send the report by mail to everyone in the chain of command of this officer.
	AuIdList recipients;
	AuIdList placeChain;
	const DgData& gd = g_Au.LockDataForRead();
	placeChain.append(AuId::Universe());
	placeChain.append(HomeGalaxyId());
	placeChain.append(HomeSectorId());
	placeChain.append(HomeSolarSystemId());
	placeChain.append(HomeWorldId());
	gd.EmpireReceivesFleetMessages(EmpireId(), placeChain, recipients);
	if(ship && !ship->OwnerId().IsNull())
		recipients.AddIfUnique(ship->OwnerId());
	unsigned int idPlayer = gd.PlayerId();				// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// List captain and human officers as recipients.
	if(ship && ship->Is(AssignedShip()))
	{
		const AuRoster& roster = ship->LockRoster();
		for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
			if((*ri).IsHumanOfficer())	// Also includes captain.
				recipients.AddIfUnique((*ri).m_OccId);
		ship->UnlockRoster();
	}

	if(!recipients.isEmpty())
	{
		// Collect current system and world information for mail message. It helps people navigate to the mail origin.
		AuId systemId, worldId;
		QString systemName, worldName;
		AuPoint galaxyLoc;
		GALAXY galaxy = GALAXY_Count;
		const DaSolarSystem* system = FindParentSolarSystem();
		if(system)
		{
			systemId = system->Id();
			systemName = system->Name();
			const DaGalaxy* pg = system->FindParentGalaxy();
			if(pg)
			{
				galaxy = pg->Galaxy();
				system->GetLocationRelativeToParent(galaxyLoc, pg);
			}
		}
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			worldId = world->Id();
			worldName = world->Name();
		}

		g_Au.SendBulkMail(idPlayer, confirmNum, Id(), Name(), recipients, AuMailMsg::MSG_OfficerDeath, title, rpt,
				systemId, systemName, worldId, worldName, galaxy, galaxyLoc);
	}
	SetModified(NOTIFY_Database);
}

void DaNpc::SetGarrisonPosture()
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		SetPosture(POSTURE_Offensive);
		return;
	}
	DaCitizen::SetGarrisonPosture();
}

QString DaNpc::RankName() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		return MissMister(true);
	case ROLE_Crew:
		{
			QString rankNames[4] =
			{
				tr("Starman"),
				tr("Petty Officer"),
				tr("Chief Petty Officer"),
				tr("Master Chief Petty Officer"),
			};
			return rankNames[Rank()];
		}
	case ROLE_Troop:
		{
			QString rankNames[4] =
			{
				tr("Private"),
				tr("Corporal"),
				tr("Sergeant"),
				tr("Master Sergeant"),
			};
			return rankNames[Rank()];
		}
	case ROLE_Officer:
		{
			QString rankNames[6] =
			{
				tr("Ensign"),
				tr("Lieutenant Junior Grade"),
				tr("Lieutenant"),
				tr("Commander"),
				tr("Captain"),
				tr("Admiral"),
			};
			return rankNames[Rank()];
		}
	}
	return DaCitizen::RankName();
}

QString DaNpc::RankIcon() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		return QString(":data/Passenger.png");
	case ROLE_Crew:
		{
			QString rankIcons[4] =
			{
				tr(":data/RankSN.png"),
				tr(":data/RankPO.png"),
				tr(":data/RankCPO.png"),
				tr(":data/RankMCPO.png"),
			};
			return rankIcons[Rank()];
		}
	case ROLE_Troop:
		{
			QString rankIcons[4] =
			{
				tr(":data/RankPrivate.png"),
				tr(":data/RankCorporal.png"),
				tr(":data/RankSergeant.png"),
				tr(":data/RankMasterSergeant.png"),
			};
			return rankIcons[Rank()];
		}
	case ROLE_Officer:
		{
			QString rankIcons[6] =
			{
				tr(":data/RankO1.png"),
				tr(":data/RankO2.png"),
				tr(":data/RankO3.png"),
				tr(":data/RankO4.png"),
				tr(":data/RankO5.png"),
				tr(":data/RankO6.png"),
			};
			return rankIcons[Rank()];
		}
	}
	return DaCitizen::RankIcon();
}

QString DaNpc::RankAbbr() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
		break;
	case ROLE_IndigenousElder:
		return tr("Elder");
	case ROLE_IndigenousHealer:
		return tr("Healer");
	case ROLE_IndigenousStargazer:
		return tr("Stargazer");
	case ROLE_Worker:
		break;
	case ROLE_Passenger:
		return MsMr();
	case ROLE_Crew:
		{
			QString rankAbbrs[4] =
			{
				tr("SN"),
				tr("PO"),
				tr("CPO"),
				tr("MCPO"),
			};
			return rankAbbrs[Rank()];
		}
	case ROLE_Troop:
		{
			QString rankAbbrs[4] =
			{
				tr("Pvt"),
				tr("Cpl"),
				tr("Sgt"),
				tr("MSgt"),
			};
			return rankAbbrs[Rank()];
		}
	case ROLE_Officer:
		{
			QString rankAbbrs[6] =
			{
				tr("ENS"),
				tr("LTJG"),
				tr("LT"),
				tr("CDR"),
				tr("CAPT"),
				tr("ADM"),
			};
			return rankAbbrs[Rank()];
		}
	}
	return DaCitizen::RankAbbr();
}

QString DaNpc::RankCrew() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
		{
			QString rankAbbrs[4] =
			{
				tr("Starman"),
				tr("Petty Officer"),
				tr("Chief"),
				tr("Master Chief"),
			};
			return rankAbbrs[Rank()];
		}
	case ROLE_Troop:
		{
			QString rankAbbrs[4] =
			{
				tr("Private"),
				tr("Corporal"),
				tr("Sergeant"),
				tr("Master Sergeant"),
			};
			return rankAbbrs[Rank()];
		}
	case ROLE_Officer:
		{
			QString rankAbbrs[6] =
			{
				tr("Ensign"),
				tr("Lieutenant JG"),
				tr("Lieutenant"),
				tr("Commander"),
				tr("Captain"),
				tr("Admiral"),
			};
			return rankAbbrs[Rank()];
		}
	}
	return DaCitizen::RankCrew();
}

quint8 DaNpc::Rank() const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
		{
			quint32 xp = CrewExperiencePoints();
			if(xp < PromotionGoalRank(0))
				return 0;
			else if(xp < PromotionGoalRank(1))
				return 1;
			else if(xp < PromotionGoalRank(2))
				return 2;
			else
				return 3;
		}
	case ROLE_Troop:
		{
			quint32 xp = TroopExperiencePoints();
			if(xp < PromotionGoalRank(0))
				return 0;
			else if(xp < PromotionGoalRank(1))
				return 1;
			else if(xp < PromotionGoalRank(2))
				return 2;
			else
				return 3;
		}
	case ROLE_Officer:
		{
			quint32 xp = CommandExperiencePoints();
			if(xp < PromotionGoalRank(0))
				return 0;
			else if(xp < PromotionGoalRank(1))
				return 1;
			else if(xp < PromotionGoalRank(2))
				return 2;
			else if(xp < PromotionGoalRank(3))
				return 3;
			else if(xp < PromotionGoalRank(4))
				return 4;
			else
				return 5;
		}
	}
	return DaCitizen::Rank();
}

quint32 DaNpc::PromotionGoalRank(quint8 rank) const
{
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Worker:
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
		{
			quint32 promotions[4] =
			{
				4000,							// Points needed to be promoted to Petty Officer.
				20000,							// Points needed to be promoted to Chief Petty Officer.
				100000,							// Points needed to be promoted to Master Chief Petty Officer.
				0								// No promotion beyond Master Chief Petty Officer.
			};
			return promotions[rank];
		}
	case ROLE_Troop:
		{
			quint32 promotions[4] =
			{
				3000,					// Points needed to be promoted to Corporal.
				18000,					// Points needed to be promoted to Sergeant.
				108000,					// Points needed to be promoted to Master Sergeant.
				0						// No promotion beyond MasterSergeant.
			};
			return promotions[rank];
		}
	case ROLE_Officer:
		{
			quint32 promotions[6] =
			{
				5000,							// Points needed to be promoted to Lieutenant Junior Grade.
				20000,							// Points needed to be promoted to Lieutenant
				80000,							// Points needed to be promoted to Commander.
				320000,							// Points needed to be promoted to Captain.
				640000,							// Points needed to be promoted to Admiral.
				0								// No promotion beyond Admiral.
			};
			return promotions[rank];
		}
	}
	return 0;
}

quint32 DaNpc::PromotionGoal() const
{
	return PromotionGoalRank(Rank());
}

bool DaNpc::InChainOfCommand(const AuId& personId, quint32 personEmpire) const
{
	if(!IsNpcOfficer())
		return false;
	Q_UNUSED(personId);
	if(InDesignPreview())
		return true;
	if(EmpireId() != personEmpire)
		return false;		// The person is not in my empire.
	if(InArena())
		return true;

	bool inChain = false;
	const DgData& gd = g_Au.LockDataForRead();
	if(gd.IsEmpireGovernor(personEmpire, personId))
	{
		// Check my home to see if it is in the chain of command of the person.
		AuIdList placeChain;
		placeChain.append(AuId::Universe());
		placeChain.append(HomeGalaxyId());
		placeChain.append(HomeSectorId());
		placeChain.append(HomeSolarSystemId());
		placeChain.append(HomeWorldId());
		inChain = gd.IsEmpirePlaceInDomain(personEmpire, placeChain, personId);
	}
	g_Au.UnlockData();
	return inChain;
}

void DaNpc::CreditAccount(double amount)
{
	if(amount <= 0)
		return;
	m_NpcLock.lockForWrite();
	m_AccountBalance += amount;
	if(m_AccountBalance < 0)	// Could happen when 'amount' is negative.
		m_AccountBalance = 0;
#if !defined(Q_OS_WIN)
	else
	{
		m_AccountBalance = round(m_AccountBalance * 100.) / 100.;	// Round to the nearest .01.
		if(m_AccountBalance > 1000000000000000)
			m_AccountBalance = 1000000000000000;
	}
#endif
	m_NpcLock.unlock();
	SetModified();
}

void DaNpc::DebitAccount(double amount)
{
	if(amount == 0)
		return;
	m_NpcLock.lockForWrite();
	m_AccountBalance -= amount;
	if(m_AccountBalance < 0)
		m_AccountBalance = 0;
#if !defined(Q_OS_WIN)
	else
		m_AccountBalance = round(m_AccountBalance * 100.) / 100.;	// Round to the nearest .01.
#endif
	m_NpcLock.unlock();
	SetModified();
}

void DaNpc::ConversationChoicePicked(DaPerson& person, qint8 pickValue)
{
	if(IsNpcIndigenous())
	{
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(FindParentPlanetInEnv());
		if(!llworld || !llworld->Is(HomeWorldId()))
			return;

		if(TryMating(person))
		{
			// They spoke to me and they are in the mating pose and ready to have sex. I am ready and compatible.
			QString text;
			if(Gender() == Male)
				text = tr("I will give you a child, %1.").arg(person.Name());
			else if(Gender() == Hermaphrodite)
				text = tr("We will bear each other's child, %1.").arg(person.Name());
			else
				text = tr("I will bear your child, %1.").arg(person.Name());
			AuCommChat rmsg(text, AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
					AuCommStdChannel::Close, AuCommStdChannel::Audible);
			Say(rmsg, AuVisage::Grin, person.Id());
			DaCitizen::TakeOrder(rmsg, ORDER_MountTarget, person.Id(), AuPoint(0,0), true,
					tr("Have sex with %1.").arg(person.Name()));
			return;
		}

		switch(pickValue)
		{
		case 0:
			SayIndigenous(*llworld, person);
			break;

		default:
			{
				AuCommChat rmsg(tr("What?"), AuCommStdChannel::VoiceChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
						AuCommStdChannel::Close, AuCommStdChannel::Audible);
				Say(rmsg, person.Id());
			}
			break;
		}
		return;
	}
	else
		DaCitizen::ConversationChoicePicked(person, pickValue);
}

bool DaNpc::AddGearItemUseDrop(const AuItem &cItem, int useIt, bool dropIt)
{
	bool v = DaCitizen::AddGearItemUseDrop(cItem, useIt, dropIt);
	if(v)
	{
		// Did I want the item?
		bool wantSatis = false;
		m_NpcLock.lockForWrite();
		if(m_WantCID == cItem.CommodityId() &&
				(m_WantQ == 0 || m_WantQ == cItem.Quality()) &&
				(m_WantEoO == 0 || m_WantEoO == cItem.EmpireOfOrigin()) &&
				(m_WantInscription.isEmpty() || m_WantInscription == cItem.Inscription()))
		{
			// The item satisfied my want.
			m_WantCID = CID_Invalid;
			m_WantQ = 0;
			m_WantEoO = 0;
			m_WantInscription = QString();
			wantSatis = true;
		}
		m_NpcLock.unlock();
		if(wantSatis)
			SetModified(NOTIFY_Database);
	}
	return v;
}

bool DaNpc::IsReadyForMating() const
{
	if(StoryInstance())
		return false;		// Not yet.
	return DaCitizen::IsReadyForMating();
}

bool DaNpc::GrowNaturally() const
{
	if(StoryInstance())
		return false;
	if(IsNpcIndigenous() && MinutesToFullGrown() <= (GrowthTime() + 1)/2)
		return false;	// Indigenous children stop growing at 1/2 size.
	return DaCitizen::GrowNaturally();
}

COMMODITYID DaNpc::Armor() const
{
	// Clothing is not depicted while I am in a surgery unit, so I don't really have to take it off.
	if(IsPatientStationPose())
		return CID_Invalid;
	if(IsMating())
		return CID_Invalid;

	if(StoryInstance())
	{
		const AuGear* gear = LockGearForRead();
		COMMODITYID c = gear->BodyArmorOrClothingWorn();
		UnlockGear();
		return c;
	}

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_Worker:
		// Workers always have an environment suit handy in harsh environments.
		if(!IsAirBreathable())
			return CID_VacSuit;
		break;
	case ROLE_Passenger:
		break;
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		{
			// Troops always show armor when they have it, regardless of whether atmosphere is breathable.
			const AuGear* gear = LockGearForRead();
			COMMODITYID c = gear->BodyArmorOrClothingWorn();
			UnlockGear();
			return c;
		}
	}
	return DaCitizen::Armor();
}

COMMODITYID DaNpc::Helmet() const
{
	// Clothing is not depicted while I am in a surgery unit, so I don't really have to take it off.
	if(IsPatientStationPose())
		return CID_Invalid;
	if(IsMating())
		return CID_Invalid;

	if(StoryInstance())
	{
		COMMODITYID c = CID_Invalid;
		const AuGear* gear = LockGearForRead();
		const AuItem* helmet = gear->HelmetItemWorn();
		if(helmet && helmet->State() == 1)
			c = helmet->CommodityId();
		else
		{
			const AuItem* hat = gear->HatItemWorn();
			if(hat)
				c = hat->CommodityId();
		}
		UnlockGear();
		return c;
	}

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
		break;
	case ROLE_Worker:
		// Workers always have an environment suit helmet handy in harsh environments.
		if(!IsAirBreathable())
			return CID_VacSuitHelmet;
		break;
	case ROLE_Passenger:
	case ROLE_Crew:
		break;
	case ROLE_Troop:
		{
			// Troops always wear a helmet when they have one, regardless of whether atmosphere is breathable.
			const AuGear* gear = LockGearForRead();
			COMMODITYID c = gear->HelmetOrHatWorn();
			UnlockGear();
			return c;
		}
	case ROLE_Officer:
		break;
	}
	return DaCitizen::Helmet();
}

DaNpc* DaNpc::FindNpcRecursive() const
{
    if(!IsNormal())
        return 0;
    if(g_Au.IsStoryActive(StoryInstance()))
        return (DaNpc*)this;    // Cast away constness.
    return DaCitizen::FindNpcRecursive();
}

DaNpc* DaNpc::FindNpcRecursive(const QString& charName, quint32 storyInstance) const
{
    if(Name() == charName && StoryInstance() == storyInstance)
        return (DaNpc*)this;    // Cast away constness.
    return DaCitizen::FindNpcRecursive(charName, storyInstance);
}

void DaNpc::WantItem(COMMODITYID cid)
{
	WantItem(cid, 0, 0, QString());
}

void DaNpc::WantItem(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& inscription)
{
	m_NpcLock.lockForWrite();
	m_WantCID = cid;
	m_WantQ = quality;
	m_WantEoO = empireOfOrigin;
	m_WantInscription = inscription;
	m_NpcLock.unlock();

	// The sentient must monitor the voice channel for offers.
	g_Au.MonitorChannel(AuCommStdChannel::VoiceChannelName(), Id());
}

void DaNpc::QueueSound(const QByteArray& soundData, AuSound& al)
{
	if(!soundData.isEmpty())
	{
		// It looks like we got sound data. Create a sound buffer from the data and add it to my sound queue.
		AuCitizenSound sound(soundData, al);
		if(sound.IsValid())
			m_Sounds.append(AuCitizenSound(soundData, al));
		else
		{
			// Sometimes the sound buffer creation fails with ALUT_ERROR_INVALID_OPERATION.
			// ALUT doc says this means, "ALUT has not been initialised." However, I know alutInit has been called by now.
			// The buffer creation usually works on the second try.
			AuCitizenSound sound(soundData, al);
			if(sound.IsValid())
				m_Sounds.append(AuCitizenSound(soundData, al));
			else
			{
				// I've never seen the buffer creation fail on the second try but it we'll try a third time just to be sure.
				AuCitizenSound sound(soundData, al);
				if(sound.IsValid())
					m_Sounds.append(AuCitizenSound(soundData, al));
			}
		}
	}
}

void DaNpc::SayIndigenous(const DaWorldLL& llworld, DaPerson& person)
{
	if(GrowthStage() == Juvenile)
	{
		SayIndigenousChild(llworld, person);
		return;
	}

	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_Count:
		break;
	case ROLE_IndigenousPerson:
		SayIndigenousAdult(llworld, person);
		break;
	case ROLE_IndigenousElder:
		SayIndigenousElder(llworld, person);
		break;
	case ROLE_IndigenousHealer:
		SayIndigenousHealer(llworld, person);
		break;
	case ROLE_IndigenousStargazer:
		SayIndigenousStargazer(llworld, person);
		break;
	case ROLE_Worker:
	case ROLE_Passenger:
	case ROLE_Crew:
	case ROLE_Troop:
	case ROLE_Officer:
		break;
	}
}

void DaNpc::InitTraining()
{
	if(GrowthStage() == Juvenile)
		return;	// Juveniles do not get training.

	quint32 promotionGoal = PromotionGoalRank(0);
	quint32 xp;
	switch(Role())
	{
	case ROLE_Citizen:
	case ROLE_IndigenousPerson:
	case ROLE_IndigenousElder:
	case ROLE_IndigenousHealer:
	case ROLE_IndigenousStargazer:
	case ROLE_Count:
		break;
	case ROLE_Worker:
	case ROLE_Passenger:
		if(g_Au.Dice().Roll(1,100) < 50)
			SetSpouse(g_Au.Dice().RandomPersonName());
		break;
	case ROLE_Crew:
		if(g_Au.Dice().Roll(1,100) < 35)
			SetSpouse(g_Au.Dice().RandomPersonName());
		// Crew first promotion goal is 4000.
		// Give them up to 200xp in each command skill, for a potential maximum of 3800.
		IncreaseSkill(AuSkill::Combat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Engineer, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::EVSuit, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::FireControl, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Helm, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Medic, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Navigator, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Power, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Sensor, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Shield, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpaceExploration, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftOperations, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Transporter, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleDriver, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehiclePilot, g_Au.Dice().Roll(2,100));
		// Top them off with Command skill.
		xp = CrewExperiencePoints();
		if(xp < promotionGoal)
			IncreaseSkill(AuSkill::SpacecraftOperations, promotionGoal - xp);	// Just enough to get promoted.
		break;
	case ROLE_Troop:
		if(g_Au.Dice().Roll(1,100) < 20)
			SetSpouse(g_Au.Dice().RandomPersonName());
		// Troop first promotion goal is 3000.
		// Give them up to 200xp in each command skill, for a potential maximum of 2600.
		IncreaseSkill(AuSkill::Combat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::EVSuit, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Guard, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpaceExploration, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftOperations, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SquadLeader, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Turret, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleDriver, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehiclePilot, g_Au.Dice().Roll(2,100));
		xp = TroopExperiencePoints();
		if(xp < promotionGoal)
			IncreaseSkill(AuSkill::Combat, promotionGoal - xp);	// Just enough to get promoted.
		break;
	case ROLE_Officer:
		if(g_Au.Dice().Roll(1,100) < 60)
			SetSpouse(g_Au.Dice().RandomPersonName());
		// Officer first promotion goal is 5000.
		// Give them up to 200xp in each command skill, for a potential maximum of 4600.
		IncreaseSkill(AuSkill::Combat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Command, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Engineer, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::EVSuit, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::FireControl, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Helm, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Medic, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Navigator, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Power, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Sensor, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Shield, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpaceExploration, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SpacecraftOperations, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::SquadLeader, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Trade, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Transporter, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::Turret, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCapture, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleCombat, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehicleDriver, g_Au.Dice().Roll(2,100));
		IncreaseSkill(AuSkill::VehiclePilot, g_Au.Dice().Roll(2,100));
		// Top them off with Command skill.
		xp = CommandExperiencePoints();
		if(xp < promotionGoal)
			IncreaseSkill(AuSkill::Command, promotionGoal - xp);	// Just enough to get promoted.
		break;
	}
}

void DaNpc::FindNpcsRecursive(DaAtomList& npcs) const
{
    //if(!IsNormal())
        //return;
    npcs.append((DaNpc*)this);  // Cast away constness.
    DaAtom::FindNpcsRecursive(npcs);
}

